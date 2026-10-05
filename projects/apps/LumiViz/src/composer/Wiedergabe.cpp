/**
 * @file Wiedergabe.cpp
 * @brief Composer: Wiedergabe einer Komposition (Stufe C1b)
 */

#include "composer/Wiedergabe.hpp"

#include "visualizers/MultiEffectVisualizer.hpp"

#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>

#include <algorithm>
#include <cstdlib>
#include <map>

namespace lumi::composer
{

namespace
{

/// Preset ueber den Ladeweg seiner Endung.
bool ladePreset(MultiEffectVisualizer& renderer, const QString& datei)
{
    const QString endung = QFileInfo(datei).suffix().toLower();
    if (endung == QLatin1String("avs")) return renderer.loadAvsFile(datei);
    if (endung == QLatin1String("milk")) return renderer.loadMilkFile(datei);
    return renderer.loadChainFile(datei);
}

}  // namespace

Wiedergabe::Wiedergabe() = default;

Wiedergabe::~Wiedergabe()
{
    entladeAlles();
}

void Wiedergabe::setFest(int kanal, int clip, bool andererClip)
{
    const Schluessel neu{kanal, clip};
    // Ein geparkter Renderer gehoert dem Clip, der festgehalten WAR.
    if (andererClip && neu != m_fest && m_park != nullptr) m_park->adoptierbar = false;
    m_fest = neu;
}

void Wiedergabe::setKomposition(Komposition komposition)
{
    entladeAlles();
    // eine andere Komposition: das Festhalten galt der alten
    m_fest = {-1, -1};
    if (m_park != nullptr) m_park->adoptierbar = false;
    m_bericht.clear();
    m_anlaeufe.clear();
    komposition.richte(&m_bericht);
    m_komposition = std::move(komposition);
}

void Wiedergabe::ersetzeKomposition(Komposition komposition)
{
    komposition.richte(&m_bericht);
    const auto clipVon = [](const Komposition& k, const Schluessel& s) -> const VisualClip* {
        if (s.first < 0 || s.first >= static_cast<int>(k.kanaele.size())) return nullptr;
        const auto& clips = k.kanaele[static_cast<size_t>(s.first)].visual;
        if (s.second < 0 || s.second >= static_cast<int>(clips.size())) return nullptr;
        return &clips[static_cast<size_t>(s.second)];
    };

    const MultiEffectVisualizer* const festVorher = rendererVon(m_fest.first, m_fest.second);
    std::map<Schluessel, Lauf> alt = std::move(m_laeufe);
    m_laeufe.clear();
    // Welcher alte Renderer gehoert zu welchem neuen Clip? Ein Platz
    // (Kanal, Nummer) sagt es nicht — verschwindet ein Nachbar, ruecken die
    // Nummern nach. Darum: alle Paare „alter Clip — neuer Clip mit derselben
    // Datei", das mit dem kleinsten Abstand der Startzeiten zuerst (bei
    // Gleichstand der gleiche Platz), jeder Renderer und jeder Clip nur einmal.
    struct Paar
    {
        std::int64_t abstand;
        bool andererPlatz;
        Schluessel von;
        Schluessel nach;
    };
    std::vector<Paar> paare;
    for (const auto& [schluessel, lauf] : alt)
    {
        const VisualClip* vorher = clipVon(m_komposition, schluessel);
        if (lauf.renderer == nullptr || vorher == nullptr) continue;
        for (int k = 0; k < static_cast<int>(komposition.kanaele.size()); ++k)
        {
            const auto& clips = komposition.kanaele[static_cast<size_t>(k)].visual;
            for (int c = 0; c < static_cast<int>(clips.size()); ++c)
            {
                const VisualClip& neu = clips[static_cast<size_t>(c)];
                if (neu.datei != vorher->datei) continue;
                paare.push_back({std::llabs(neu.startMs - vorher->startMs),
                                 Schluessel{k, c} != schluessel, schluessel, {k, c}});
            }
        }
    }
    std::sort(paare.begin(), paare.end(), [](const Paar& a, const Paar& b) {
        return a.abstand != b.abstand ? a.abstand < b.abstand : a.andererPlatz < b.andererPlatz;
    });
    for (const Paar& p : paare)
    {
        const auto quelle = alt.find(p.von);
        if (quelle == alt.end() || m_laeufe.count(p.nach) != 0) continue;
        // den Knoten umhaengen statt den Lauf zu verschieben (die Leinwand laesst sich nicht bewegen)
        auto knoten = alt.extract(quelle);
        knoten.key() = p.nach;
        m_laeufe.insert(std::move(knoten));
    }
    // ohne Clip: entladen. Auch der festgehaltene bekommt seinen nicht wieder —
    // seinen Clip gibt es nicht mehr.
    for (auto& [schluessel, lauf] : alt) wirfWeg(lauf, false);
    m_komposition = std::move(komposition);

    // Das Festhalten folgt seinem Renderer an den neuen Platz.
    if (festVorher != nullptr)
    {
        m_fest = {-1, -1};
        for (const auto& [schluessel, lauf] : m_laeufe)
            if (lauf.renderer.get() == festVorher) m_fest = schluessel;
        if (m_fest.first < 0 && m_park != nullptr) m_park->adoptierbar = false;
    }
}

void Wiedergabe::wirfWeg(Lauf& lauf, bool warFest)
{
    if (lauf.renderer == nullptr) return;
    lauf.renderer->cleanup();
    // An diesem Renderer haengt die Oberflaeche: parken statt loeschen (C3c).
    if (m_park != nullptr && lauf.renderer.get() == m_park->geschuetzt)
    {
        m_park->geparkt = std::move(lauf.renderer);
        m_park->adoptierbar = warFest;
        return;
    }
    lauf.renderer.reset();
}

void Wiedergabe::entladeAlles()
{
    for (auto& [schluessel, lauf] : m_laeufe) wirfWeg(lauf, schluessel == m_fest);
    m_laeufe.clear();
    if (m_karaokeTraeger != nullptr) m_karaokeTraeger->cleanup();
    m_karaokeTraeger.reset();
    m_gemischt = 0;
    m_karaokeAufgelegt = false;
}

Wiedergabe::Lauf& Wiedergabe::sichere(const Schluessel& s, const VisualClip& clip, QSize groesse,
                                      std::int64_t stelleMs, bool vorgewaermt)
{
    Lauf& lauf = m_laeufe[s];
    if (lauf.renderer != nullptr || lauf.defekt) return lauf;

    QElapsedTimer uhr;
    uhr.start();
    std::unique_ptr<MultiEffectVisualizer> renderer;
    // Der festgehaltene Clip bekommt seinen geparkten Renderer wieder — samt
    // der Kette, wie sie zuletzt war (auch ungespeicherte Aenderungen).
    const bool adoptiert =
        s == m_fest && m_park != nullptr && m_park->geparkt != nullptr && m_park->adoptierbar;
    if (adoptiert)
    {
        renderer = std::move(m_park->geparkt);
        m_park->adoptierbar = false;
    }
    else
        renderer = std::make_unique<MultiEffectVisualizer>();
    if (!adoptiert &&
        (clip.datei.isEmpty() || !QFileInfo::exists(clip.datei) || !ladePreset(*renderer, clip.datei)))
    {
        m_bericht.append(QStringLiteral("Clip could not be loaded: %1")
                             .arg(QDir::toNativeSeparators(clip.datei)));
        lauf.defekt = true;
        return lauf;
    }
    const double ladenMs = static_cast<double>(uhr.nsecsElapsed()) / 1.0e6;
    uhr.restart();
    // Die Ebene legt der Mischer einmal ueber alles (Composer-Naht R2)
    renderer->setKaraokeAuslassen(true);
    renderer->initialize();
    renderer->resize(groesse);
    lauf.renderer = std::move(renderer);

    // --- Messung (C2a): was der Anlauf kostet
    Anlauf a;
    a.kanal = s.first;
    a.clip = s.second;
    a.datei = clip.datei;
    a.stelleMs = stelleMs;
    a.vorgewaermt = vorgewaermt;
    a.ladenMs = ladenMs;
    a.anlegenMs = static_cast<double>(uhr.nsecsElapsed()) / 1.0e6;
    lauf.anlauf = static_cast<int>(m_anlaeufe.size());
    lauf.bilder = 0;
    m_anlaeufe.push_back(std::move(a));
    return lauf;
}

void Wiedergabe::rechne(Lauf& lauf, float deltaSekunden, QSize groesse)
{
    if (lauf.bilder >= 2 || lauf.anlauf < 0)
    {
        lauf.leinwand.rendere(*lauf.renderer, deltaSekunden, groesse);
        return;
    }
    QElapsedTimer uhr;
    uhr.start();
    lauf.leinwand.rendere(*lauf.renderer, deltaSekunden, groesse);
    const double ms = static_cast<double>(uhr.nsecsElapsed()) / 1.0e6;
    Anlauf& a = m_anlaeufe[static_cast<size_t>(lauf.anlauf)];
    (lauf.bilder == 0 ? a.erstesBildMs : a.zweitesBildMs) = ms;
    ++lauf.bilder;
}

bool Wiedergabe::rendere(std::int64_t stelleMs, float deltaSekunden, QSize groesse,
                         bool clipsStehen)
{
    m_gemischt = 0;
    m_karaokeAufgelegt = false;
    if (groesse.isEmpty()) return false;

    // --- 1. Wer laeuft, wer wird vorgewaermt, wer geht?
    struct Dran
    {
        Schluessel schluessel;
        const VisualClip* clip;
        bool laeuft;
    };
    std::vector<Dran> dran;
    for (int k = 0; k < static_cast<int>(m_komposition.kanaele.size()); ++k)
    {
        const Kanal& kanal = m_komposition.kanaele[static_cast<size_t>(k)];
        if (kanal.art != KanalArt::Visual || !kanal.an) continue;
        for (int c = 0; c < static_cast<int>(kanal.visual.size()); ++c)
        {
            const VisualClip& clip = kanal.visual[static_cast<size_t>(c)];
            const bool laeuft = stelleMs >= clip.startMs && stelleMs < clip.endeMs;
            const bool bald = m_vorlaufMs > 0 && stelleMs < clip.startMs &&
                              stelleMs >= clip.startMs - m_vorlaufMs;
            if (laeuft || bald) dran.push_back({{k, c}, &clip, laeuft});
        }
    }
    // Der festgehaltene Clip (C3c) ist immer dran — wie ein vorgewaermter,
    // auch weit weg von seiner Zeit und in einem ausgeschalteten Kanal.
    if (m_fest.first >= 0 && m_fest.first < static_cast<int>(m_komposition.kanaele.size()))
    {
        const Kanal& kanal = m_komposition.kanaele[static_cast<size_t>(m_fest.first)];
        const bool schon = std::any_of(dran.begin(), dran.end(),
                                       [&](const Dran& d) { return d.schluessel == m_fest; });
        if (!schon && kanal.art == KanalArt::Visual && m_fest.second >= 0 &&
            m_fest.second < static_cast<int>(kanal.visual.size()))
            dran.push_back({m_fest, &kanal.visual[static_cast<size_t>(m_fest.second)], false});
    }
    // Entladen: was weder laeuft noch bald dran ist
    for (auto it = m_laeufe.begin(); it != m_laeufe.end();)
    {
        const bool bleibt = std::any_of(dran.begin(), dran.end(), [&](const Dran& d) {
            return d.schluessel == it->first;
        });
        if (bleibt)
        {
            ++it;
            continue;
        }
        wirfWeg(it->second, false);
        it = m_laeufe.erase(it);
    }

    // --- 2. Vorwaermen: anlegen und EIN Bild rechnen (kompiliert das Preset).
    //     Mit Delta 0, damit die Zeit des Visuals nicht vorlaeuft; ganz frisch
    //     ist der Clip an seinem Start dann nicht mehr — er hat ein Bild hinter sich.
    for (const Dran& d : dran)
    {
        if (d.laeuft) continue;
        const bool neu = m_laeufe.count(d.schluessel) == 0;
        Lauf& lauf = sichere(d.schluessel, *d.clip, groesse, stelleMs, true);
        // Baut der Renderer noch an einer Movement-Tabelle (Scheiben-Betrieb),
        // rechnet er weiter Bilder, bis sie steht — sonst kaeme der Rest des
        // Baus erst, wenn der Clip schon zu sehen ist.
        if (lauf.renderer != nullptr && (neu || lauf.renderer->movementTabelleImBau()))
            rechne(lauf, 0.0f, groesse);
    }

    // --- 3. Laufende Clips rechnen und mischen (dran ist in Kanalreihenfolge)
    if (!m_mischer.beginne(groesse)) return false;
    for (const Dran& d : dran)
    {
        if (!d.laeuft) continue;
        Lauf& lauf = sichere(d.schluessel, *d.clip, groesse, stelleMs, false);
        if (lauf.renderer == nullptr) continue;
        // Auch bei Pegel 0 weiterrechnen: das Visual lebt, nur sieht man es nicht.
        // Steht die Uhr, bleibt sein letztes Bild stehen — ausser es hat noch
        // keins oder die Groesse hat sich geaendert.
        if (!clipsStehen || lauf.bilder == 0 || lauf.leinwand.groesse() != groesse)
            rechne(lauf, clipsStehen ? 0.0f : deltaSekunden, groesse);
        const double pegel = d.clip->pegelBei(stelleMs - d.clip->startMs);
        m_mischer.lege(lauf.leinwand, pegel, d.clip->mischart);
        ++m_gemischt;
    }

    // --- 4. Karaoke-Ebene einmal ueber das Gemischte
    if (MultiEffectVisualizer::karaokeEbene())
    {
        if (m_karaokeTraeger == nullptr)
        {
            m_karaokeTraeger = std::make_unique<MultiEffectVisualizer>();
            m_karaokeTraeger->initialize();
            m_karaokeTraeger->resize(groesse);
        }
        // Der Traeger rechnet nur die Ebene; sein eigenes Bild wird verworfen.
        m_karaokeLeinwand.rendere(*m_karaokeTraeger, deltaSekunden, groesse);
        if (m_karaokeTraeger->debugKaraokeGezeichnet())
        {
            m_mischer.binde();
            m_karaokeTraeger->legeKaraokeEbeneAuf();
            m_karaokeAufgelegt = true;
        }
    }
    else if (m_karaokeTraeger != nullptr)
    {
        m_karaokeTraeger->cleanup();
        m_karaokeTraeger.reset();
    }
    return true;
}

void Wiedergabe::fuerJedenRenderer(const std::function<void(MultiEffectVisualizer&)>& tu)
{
    for (auto& [schluessel, lauf] : m_laeufe)
        if (lauf.renderer != nullptr) tu(*lauf.renderer);
    if (m_karaokeTraeger != nullptr) tu(*m_karaokeTraeger);
}

}  // namespace lumi::composer
