/**
 * @file Tonauftrag.cpp
 * @brief Loop: die Klaenge eines Loops laden — im Hintergrund auf mehreren Kernen (Stufe 29c)
 */

#include "loop/Tonauftrag.hpp"

#include "loop/Instrument.hpp"
#include "loop/Kit.hpp"
#include "loop/Loop.hpp"
#include "loop/Sample.hpp"
#include "loop/Tonspeicher.hpp"

#include <QDir>
#include <QFileInfo>

#include <algorithm>
#include <chrono>
#include <map>
#include <mutex>
#include <optional>
#include <utility>

namespace lumi::loop
{

namespace
{

using Uhr = std::chrono::steady_clock;

double msSeit(Uhr::time_point anfang)
{
    return std::chrono::duration<double, std::milli>(Uhr::now() - anfang).count();
}

}  // namespace

/// Je Instrument einmal geholt — mit seinen Samples und seinem Fingerabdruck (TV2 a).
/// Wer ein Instrument zuerst braucht, holt es; wer es zugleich braucht, wartet an seiner
/// Sperre. Verschiedene Instrumente werden nebeneinander geholt.
struct Tonauftrag::Gemeinsam
{
    struct Geholt
    {
        std::mutex sperre;
        bool fertig = false;
        std::optional<Instrument> instrument;
        QByteArray abdruck;
        QStringList zeilen;  ///< was beim Holen auffiel: Samples, nicht gefunden, zu neu
    };

    std::mutex sperre;
    std::map<QString, std::unique_ptr<Geholt>> instrumente;

    Geholt& eintrag(const QString& nennung)
    {
        const std::lock_guard<std::mutex> halt(sperre);
        std::unique_ptr<Geholt>& ort = instrumente[nennung];
        if (ort == nullptr) ort = std::make_unique<Geholt>();
        return *ort;
    }

    /// nullptr = noch nie gebraucht
    const Geholt* finde(const QString& nennung)
    {
        const std::lock_guard<std::mutex> halt(sperre);
        const auto ort = instrumente.find(nennung);
        return ort == instrumente.end() ? nullptr : ort->second.get();
    }
};

Tonauftrag::Tonauftrag(QStringList namen, int rate, Dekodierer dekodierer)
    : m_namen(std::move(namen)), m_rate(rate), m_dekodierer(std::move(dekodierer)), m_gemeinsam(std::make_unique<Gemeinsam>())
{
    // jeder Name einmal: wer zweimal vorkommt, wird einmal geladen
    std::map<QString, std::size_t> gesehen;
    m_platz.reserve(static_cast<std::size_t>(m_namen.size()));
    for (const QString& name : m_namen)
    {
        const auto [ort, neu] = gesehen.emplace(name, m_einzeln.size());
        if (neu) m_einzeln.push_back(name);
        m_platz.push_back(ort->second);
    }
    m_ergebnis.resize(m_einzeln.size());
    m_zeilen.resize(m_einzeln.size());
}

Tonauftrag::~Tonauftrag()
{
    brichAb();
    warte();
}

void Tonauftrag::rechneHier()
{
    arbeite();
}

void Tonauftrag::starte(int threads)
{
    if (!m_threads.empty()) return;
    int zahl = threads > 0 ? threads : static_cast<int>(std::thread::hardware_concurrency()) - 1;
    zahl = std::min(std::max(zahl, 1), gesamt());
    for (int t = 0; t < zahl; ++t) m_threads.emplace_back([this] { arbeite(); });
}

void Tonauftrag::brichAb()
{
    m_abbruch = true;
}

void Tonauftrag::warte()
{
    for (std::thread& thread : m_threads)
        if (thread.joinable()) thread.join();
}

void Tonauftrag::arbeite()
{
    while (!m_abbruch.load())
    {
        const std::size_t einer = m_naechster++;
        if (einer >= m_einzeln.size()) return;
        lade(einer);
        ++m_erledigt;
    }
}

void Tonauftrag::lade(std::size_t einer)
{
    const QString& klang = m_einzeln[einer];
    Tonspeicher& speicher = tonspeicher();
    if (istTon(klang))
    {
        // der Ton eines gerechneten Instruments: aus dem Tonspeicher oder hier gerechnet
        TonAngabe ton;  // seit Stufe 23b mit Saite und Ende
        if (!zerlegeTon(klang, ton))
        {
            m_zeilen[einer] << QStringLiteral("Instrument not found: %1").arg(klang);
            return;
        }
        Gemeinsam::Geholt& geholt = m_gemeinsam->eintrag(ton.instrument);
        {
            const std::lock_guard<std::mutex> halt(geholt.sperre);
            if (!geholt.fertig)
            {
                // je Instrument einmal holen — eine eigene Datei wird dabei gelesen
                Instrument neu;
                bool zuNeu = false;
                const bool da = holeInstrument(ton.instrument, neu, &zuNeu);
                if (da)
                {
                    // die Aufnahmen seiner Sample-Schichten: einmal dekodiert und gemerkt (Stufe 11a)
                    for (const Schicht& schicht : neu.schichten)
                    {
                        if (schicht.schwingung != Schwingung::Sample || !schicht.an) continue;
                        if (sampleDaten(schicht.sample) != nullptr) continue;
                        const QString datei = sampleDatei(schicht.sample);
                        if (!datei.isEmpty()) merkeSampleDaten(schicht.sample, kSampleRate, m_dekodierer(datei, kSampleRate, kKlangMaxSekunden));
                        if (sampleDaten(schicht.sample) == nullptr)
                        {
                            const QString zeile = QStringLiteral("Sample not loaded: %1")
                                                      .arg(schicht.sample.isEmpty() ? QStringLiteral("(none)")
                                                                                    : QDir::toNativeSeparators(schicht.sample));
                            if (!geholt.zeilen.contains(zeile)) geholt.zeilen << zeile;
                        }
                    }
                    geholt.abdruck = instrumentAbdruck(neu);
                    geholt.instrument = std::move(neu);
                }
                else
                {
                    geholt.zeilen << (zuNeu ? QStringLiteral("Instrument needs a newer LumiViz: %1")
                                            : QStringLiteral("Instrument not found: %1"))
                                         .arg(ton.instrument.isEmpty() ? klang : QDir::toNativeSeparators(ton.instrument));
                }
                geholt.fertig = true;
            }
        }
        if (!geholt.instrument.has_value()) return;
        const QString schluessel = tonSchluessel(klang, m_rate, geholt.abdruck);
        m_ergebnis[einer] = speicher.hole(schluessel);
        if (m_ergebnis[einer] == nullptr)
        {
            const auto anfang = Uhr::now();
            auto neu = std::make_shared<const Klang>(rendereTon(*geholt.instrument, ton, m_rate));
            speicher.lege(schluessel, neu, msSeit(anfang));
            m_ergebnis[einer] = std::move(neu);
        }
        return;
    }
    // ein dekodierter Klang: ebenso aus dem Tonspeicher (TV3 a), sein Schluessel traegt die Datei —
    // nur eine Datei, die es gibt: an einer anderen liesse sich keine Aenderung erkennen
    const QString datei = klangDatei(klang);
    if (!datei.isEmpty())
    {
        const bool merkbar = QFileInfo(datei).isFile();
        const QString schluessel = merkbar ? klangSchluessel(klang, datei, m_rate) : QString();
        if (merkbar) m_ergebnis[einer] = speicher.hole(schluessel);
        if (m_ergebnis[einer] == nullptr)
        {
            const auto anfang = Uhr::now();
            Klang neu;
            neu.daten = m_dekodierer(datei, m_rate, kKlangMaxSekunden);
            if (!neu.daten.empty())
            {
                auto geteilt = std::make_shared<const Klang>(std::move(neu));
                if (merkbar) speicher.lege(schluessel, geteilt, msSeit(anfang));
                m_ergebnis[einer] = std::move(geteilt);
            }
        }
    }
    if (m_ergebnis[einer] == nullptr) m_zeilen[einer] << QStringLiteral("Sound not loaded: %1").arg(QDir::toNativeSeparators(klang));
}

Klaenge Tonauftrag::klaenge() const
{
    Klaenge klaenge;
    klaenge.reserve(m_platz.size());
    for (const std::size_t einer : m_platz) klaenge.push_back(m_ergebnis[einer]);
    return klaenge;
}

QStringList Tonauftrag::bericht() const
{
    // in der Reihenfolge der Namen: ein Ton bringt die Zeilen seines Instruments mit
    QStringList alle;
    const auto nimm = [&alle](const QStringList& zeilen) {
        for (const QString& zeile : zeilen)
            if (!alle.contains(zeile)) alle << zeile;
    };
    for (const std::size_t einer : m_platz)
    {
        nimm(m_zeilen[einer]);
        TonAngabe ton;
        if (!istTon(m_einzeln[einer]) || !zerlegeTon(m_einzeln[einer], ton)) continue;
        if (const Gemeinsam::Geholt* geholt = m_gemeinsam->finde(ton.instrument)) nimm(geholt->zeilen);
    }
    return alle;
}

}  // namespace lumi::loop
