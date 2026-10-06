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

void Wiedergabe::festerClipHeisstJetzt(const QString& datei)
{
    if (m_fest.first < 0 || m_fest.first >= static_cast<int>(m_komposition.kanaele.size())) return;
    auto& clips = m_komposition.kanaele[static_cast<size_t>(m_fest.first)].clips;
    if (m_fest.second < 0 || m_fest.second >= static_cast<int>(clips.size())) return;
    clips[static_cast<size_t>(m_fest.second)].datei = datei;
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
        const auto& clips = k.kanaele[static_cast<size_t>(s.first)].clips;
        if (s.second < 0 || s.second >= static_cast<int>(clips.size())) return nullptr;
        return &clips[static_cast<size_t>(s.second)];
    };

    // Zweite Exemplare (Schleife, S99) ueberstehen das Bearbeiten nicht: ihr
    // Clip kann an anderem Platz liegen — im Fenster entsteht ein neues.
    for (auto& [schluessel, lauf] : m_frische) wirfWeg(lauf, false);
    m_frische.clear();

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
            const auto& clips = komposition.kanaele[static_cast<size_t>(k)].clips;
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
    for (auto& [schluessel, lauf] : m_frische) wirfWeg(lauf, false);
    m_frische.clear();
    if (m_karaokeTraeger != nullptr) m_karaokeTraeger->cleanup();
    m_karaokeTraeger.reset();
    m_gemischt = 0;
    m_karaokeAufgelegt = false;
}

Wiedergabe::Lauf& Wiedergabe::sichere(const Schluessel& s, const VisualClip& clip, QSize groesse,
                                      std::int64_t stelleMs, bool vorgewaermt, bool frisch)
{
    Lauf& lauf = frisch ? m_frische[s] : m_laeufe[s];
    if (lauf.renderer != nullptr || lauf.defekt) return lauf;

    QElapsedTimer uhr;
    uhr.start();
    std::unique_ptr<MultiEffectVisualizer> renderer;
    // Der festgehaltene Clip bekommt seinen geparkten Renderer wieder — samt
    // der Kette, wie sie zuletzt war (auch ungespeicherte Aenderungen).
    const bool adoptiert = !frisch && s == m_fest && m_park != nullptr && m_park->geparkt != nullptr &&
                           m_park->adoptierbar;
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
                         bool clipsStehen, Schleifenlage schleife)
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
        double pegel;  ///< nur fuer laufende
        bool frisch = false;            ///< das zweite Exemplar des Clips (`m_frische`)
        std::int64_t imClipMs = -1;     ///< die Stelle im Clip, an der er gemischt wird
    };
    std::vector<Dran> dran;
    const std::int64_t zweiteMs = schleife.zweiteMs;
    for (int k = 0; k < static_cast<int>(m_komposition.kanaele.size()); ++k)
    {
        const Kanal& kanal = m_komposition.kanaele[static_cast<size_t>(k)];
        if (kanal.art != KanalArt::Visual || !kanal.an) continue;
        // Im Fenster der Schleife liegen in einem Kanal zwei Clips zugleich:
        // der des Endes zuerst, der des Anfangs darueber — der Neue kommt
        // obenauf, wie ein hoeherer Kanal. Ausserhalb gibt es nur einen Durchgang.
        for (const bool nurHinten : {true, false})
        {
            if (nurHinten && zweiteMs < 0) continue;
            for (int c = 0; c < static_cast<int>(kanal.clips.size()); ++c)
            {
                const VisualClip& clip = kanal.clips[static_cast<size_t>(c)];
                const bool vorn = stelleMs >= clip.startMs && stelleMs < clip.endeMs;
                const bool hinten = zweiteMs >= clip.startMs && zweiteMs < clip.endeMs;
                if (nurHinten != (hinten && !vorn)) continue;
                if (vorn || hinten)
                {
                    // Ein Clip in beiden Fenstern laeuft einmal durch; es gilt
                    // der hoehere der zwei Pegel (Entscheid Patrik S99).
                    // Dazu die Blende der GANZEN Komposition an der jeweiligen
                    // Stelle: am Anfang herauf, am Ende herunter — im Fenster
                    // ergibt das eine Ueberblendung.
                    const std::int64_t vornIm = stelleMs - clip.startMs;
                    const std::int64_t hintenIm = zweiteMs - clip.startMs;
                    const double pegelVorn =
                        vorn ? clip.pegelBei(vornIm) * m_komposition.blendeBei(stelleMs) : 0.0;
                    const double pegelHinten =
                        hinten ? clip.pegelBei(hintenIm) * m_komposition.blendeBei(zweiteMs) : 0.0;
                    // Visuals overlap themselves (Haken der Komposition): im
                    // Fenster zwei Exemplare — das alte spielt das Ende, ein
                    // frisches darueber den Anfang, jedes mit seinem Pegel.
                    // Der festgehaltene Clip (Effect Chain haengt an ihm)
                    // laeuft immer nur einmal.
                    const bool darfDoppelt = m_komposition.bilderDoppelt && Schluessel{k, c} != m_fest;
                    if (vorn && hinten && darfDoppelt)
                    {
                        dran.push_back({{k, c}, &clip, true, pegelHinten, false, hintenIm});
                        dran.push_back({{k, c}, &clip, true, pegelVorn, true, vornIm});
                        continue;
                    }
                    dran.push_back({{k, c}, &clip, true, std::max(pegelVorn, pegelHinten), false,
                                    hinten ? hintenIm : vornIm});
                    // ... und kurz vor dem Umschlag wird das frische Exemplar
                    // vorgewaermt: der Clip laeuft ueber den Umschlag hinweg
                    // und beginnt in der naechsten Runde innerhalb des Vorlaufs
                    if (vorn && !hinten && darfDoppelt && m_vorlaufMs > 0 && m_komposition.ueberlappung() > 0 &&
                        schleife.umschlagInMs >= 0 && schleife.umschlagInMs + clip.startMs <= m_vorlaufMs &&
                        clip.endeMs > stelleMs + schleife.umschlagInMs)
                        dran.push_back({{k, c}, &clip, false, 0.0, true, -1});
                    continue;
                }
                bool bald = m_vorlaufMs > 0 && stelleMs < clip.startMs &&
                            stelleMs >= clip.startMs - m_vorlaufMs;
                // ueber den Umschlag der Schleife hinweg: der Clip beginnt in
                // der naechsten Runde innerhalb des Vorlaufs
                if (!bald && m_vorlaufMs > 0 && schleife.umschlagInMs >= 0)
                    bald = schleife.umschlagInMs + clip.startMs <= m_vorlaufMs;
                if (bald) dran.push_back({{k, c}, &clip, false, 0.0});
            }
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
            m_fest.second < static_cast<int>(kanal.clips.size()))
            dran.push_back({m_fest, &kanal.clips[static_cast<size_t>(m_fest.second)], false, 0.0});
    }
    // Zweite Exemplare, die niemand mehr braucht. Hinter dem Fenster laeuft
    // EINES weiter: das, dessen letzte Stelle im Clip der jetzigen am
    // naechsten liegt — nach einer Runde das frische (es rueckt nach
    // `m_laeufe`), nach „Loop aus" im Fenster das alte.
    for (auto it = m_frische.begin(); it != m_frische.end();)
    {
        const Schluessel s = it->first;
        const bool bleibt = std::any_of(dran.begin(), dran.end(), [&](const Dran& d) {
            return d.frisch && d.schluessel == s;
        });
        if (bleibt)
        {
            ++it;
            continue;
        }
        const auto einzeln = std::find_if(dran.begin(), dran.end(), [&](const Dran& d) {
            return !d.frisch && d.laeuft && d.schluessel == s;
        });
        const auto alt = m_laeufe.find(s);
        bool rueckt = false;
        if (einzeln != dran.end() && it->second.renderer != nullptr && it->second.imClipMs >= 0)
        {
            const std::int64_t zumFrischen = std::llabs(it->second.imClipMs - einzeln->imClipMs);
            rueckt = alt == m_laeufe.end() || alt->second.imClipMs < 0 ||
                     zumFrischen < std::llabs(alt->second.imClipMs - einzeln->imClipMs);
        }
        if (rueckt)
        {
            if (alt != m_laeufe.end())
            {
                wirfWeg(alt->second, false);
                m_laeufe.erase(alt);
            }
            // den Knoten umhaengen (die Leinwand laesst sich nicht bewegen)
            auto knoten = m_frische.extract(it++);
            m_laeufe.insert(std::move(knoten));
        }
        else
        {
            wirfWeg(it->second, false);
            it = m_frische.erase(it);
        }
    }
    // Entladen: was weder laeuft noch bald dran ist
    for (auto it = m_laeufe.begin(); it != m_laeufe.end();)
    {
        const bool bleibt = std::any_of(dran.begin(), dran.end(), [&](const Dran& d) {
            return !d.frisch && d.schluessel == it->first;
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
        const bool neu = (d.frisch ? m_frische : m_laeufe).count(d.schluessel) == 0;
        Lauf& lauf = sichere(d.schluessel, *d.clip, groesse, stelleMs, true, d.frisch);
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
        Lauf& lauf = sichere(d.schluessel, *d.clip, groesse, stelleMs, false, d.frisch);
        if (lauf.renderer == nullptr) continue;
        lauf.imClipMs = d.imClipMs;
        // Auch bei Pegel 0 weiterrechnen: das Visual lebt, nur sieht man es nicht.
        // Steht die Uhr, bleibt sein letztes Bild stehen — ausser es hat noch
        // keins oder die Groesse hat sich geaendert.
        if (!clipsStehen || lauf.bilder == 0 || lauf.leinwand.groesse() != groesse)
            rechne(lauf, clipsStehen ? 0.0f : deltaSekunden, groesse);
        m_mischer.lege(lauf.leinwand, d.pegel, d.clip->mischart);
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
    for (auto& [schluessel, lauf] : m_frische)
        if (lauf.renderer != nullptr) tu(*lauf.renderer);
    if (m_karaokeTraeger != nullptr) tu(*m_karaokeTraeger);
}

}  // namespace lumi::composer
