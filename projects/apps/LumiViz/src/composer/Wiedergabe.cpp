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

void Wiedergabe::setKomposition(Komposition komposition)
{
    entladeAlles();
    m_bericht.clear();
    m_anlaeufe.clear();
    komposition.richte(&m_bericht);
    m_komposition = std::move(komposition);
}

void Wiedergabe::wirfWeg(Lauf& lauf)
{
    if (lauf.renderer != nullptr) lauf.renderer->cleanup();
    lauf.renderer.reset();
}

void Wiedergabe::entladeAlles()
{
    for (auto& [schluessel, lauf] : m_laeufe) wirfWeg(lauf);
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
    auto renderer = std::make_unique<MultiEffectVisualizer>();
    if (clip.datei.isEmpty() || !QFileInfo::exists(clip.datei) || !ladePreset(*renderer, clip.datei))
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

bool Wiedergabe::rendere(std::int64_t stelleMs, float deltaSekunden, QSize groesse)
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
        wirfWeg(it->second);
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
        rechne(lauf, deltaSekunden, groesse);
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
