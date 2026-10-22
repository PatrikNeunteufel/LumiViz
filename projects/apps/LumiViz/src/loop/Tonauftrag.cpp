/**
 * @file Tonauftrag.cpp
 * @brief Loop: die Klaenge eines Loops laden — im Hintergrund auf mehreren Kernen (Stufe 29c)
 */

#include "loop/Tonauftrag.hpp"

#include "loop/Freeze.hpp"
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

QString sampleZeile(const QString& sample)
{
    return QStringLiteral("Sample not loaded: %1").arg(sample.isEmpty() ? QStringLiteral("(none)") : QDir::toNativeSeparators(sample));
}

}  // namespace

/// Ein Instrument, einmal geholt — mit seinem Fingerabdruck (TV2 a). Wer es zuerst braucht,
/// holt es; wer es zugleich braucht, wartet an seiner Sperre. Die Aufnahmen seiner
/// Sample-Schichten werden erst dekodiert, wenn ein Ton wirklich gerechnet wird.
struct Tonauftrag::Geholt
{
    std::mutex sperre;
    bool geholt = false;
    bool samples = false;  ///< die Aufnahmen sind dekodiert (oder fehlen)
    std::optional<Instrument> instrument;
    QByteArray abdruck;
    QStringList zeilen;  ///< was auffiel: Samples, nicht gefunden, zu neu
};

/// Je Instrument ein `Geholt`; verschiedene Instrumente werden nebeneinander geholt.
struct Tonauftrag::Gemeinsam
{
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

bool Tonauftrag::ausSpeicher()
{
    if (!m_threads.empty() || m_erledigt.load() > 0) return fertig();
    bool alle = true;
    for (std::size_t einer = 0; einer < m_einzeln.size(); ++einer)
    {
        QString schluessel;
        Geholt* geholt = nullptr;
        if (!ausSpeicher(einer, schluessel, geholt)) alle = false;
    }
    if (alle) m_erledigt = gesamt();
    return alle;
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
    m_laufend = zahl;
    for (int t = 0; t < zahl; ++t)
        m_threads.emplace_back([this] {
            arbeite();
            --m_laufend;
        });
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

bool Tonauftrag::ausSpeicher(std::size_t einer, QString& schluessel, Geholt*& geholt)
{
    if (m_ergebnis[einer] != nullptr) return true;
    const QString& klang = m_einzeln[einer];
    if (istTon(klang))
    {
        TonAngabe ton;  // seit Stufe 23b mit Saite und Ende
        if (!zerlegeTon(klang, ton))
        {
            if (m_zeilen[einer].isEmpty()) m_zeilen[einer] << QStringLiteral("Instrument not found: %1").arg(klang);
            return true;  // nichts zu laden
        }
        geholt = &m_gemeinsam->eintrag(ton.instrument);
        {
            const std::lock_guard<std::mutex> halt(geholt->sperre);
            if (!geholt->geholt)
            {
                // je Instrument einmal holen — eine eigene Datei wird dabei gelesen
                Instrument neu;
                bool zuNeu = false;
                if (holeInstrument(ton.instrument, neu, &zuNeu))
                {
                    // eine Aufnahme, deren Datei fehlt, faellt schon hier auf — auch wenn
                    // der Ton aus dem Speicher kommt (dort liegt er dann still)
                    for (const Schicht& schicht : neu.schichten)
                    {
                        if (schicht.schwingung != Schwingung::Sample || !schicht.an) continue;
                        const QString datei = sampleDatei(schicht.sample);
                        if ((datei.isEmpty() || !QFileInfo(datei).isFile()) && !geholt->zeilen.contains(sampleZeile(schicht.sample)))
                            geholt->zeilen << sampleZeile(schicht.sample);
                    }
                    geholt->abdruck = instrumentAbdruck(neu);
                    geholt->instrument = std::move(neu);
                }
                else
                {
                    geholt->zeilen << (zuNeu ? QStringLiteral("Instrument needs a newer LumiViz: %1")
                                             : QStringLiteral("Instrument not found: %1"))
                                          .arg(ton.instrument.isEmpty() ? klang : QDir::toNativeSeparators(ton.instrument));
                }
                geholt->geholt = true;
            }
        }
        if (!geholt->instrument.has_value()) return true;  // nicht zu laden: still
        schluessel = tonSchluessel(klang, m_rate, geholt->abdruck);
        m_ergebnis[einer] = tonspeicher().hole(schluessel);
        return m_ergebnis[einer] != nullptr;
    }
    // eine eingefrorene Wellenform (Stufe 30f): ihr Name ist der Abdruck — eine Datei mit
    // diesem Namen aendert sich nie, Rate und Name genuegen als Schluessel
    if (!abdruckVonKlang(klang).isEmpty())
    {
        schluessel = klang + QLatin1Char('|') + QString::number(m_rate);
        m_ergebnis[einer] = tonspeicher().hole(schluessel);
        return m_ergebnis[einer] != nullptr;
    }
    // eine Klangdatei: nur eine, die es gibt, steht im Speicher — an einer anderen liesse
    // sich keine Aenderung erkennen
    const QString datei = klangDatei(klang);
    if (datei.isEmpty())
    {
        if (m_zeilen[einer].isEmpty()) m_zeilen[einer] << QStringLiteral("Sound not loaded: %1").arg(QDir::toNativeSeparators(klang));
        return true;
    }
    if (!QFileInfo(datei).isFile()) return false;  // dekodieren: die Engine weiss, was daraus wird
    schluessel = klangSchluessel(klang, datei, m_rate);
    m_ergebnis[einer] = tonspeicher().hole(schluessel);
    return m_ergebnis[einer] != nullptr;
}

void Tonauftrag::lade(std::size_t einer)
{
    QString schluessel;
    Geholt* geholt = nullptr;
    if (ausSpeicher(einer, schluessel, geholt)) return;
    const QString& klang = m_einzeln[einer];
    if (geholt != nullptr)
    {
        // ein Ton: die Aufnahmen seiner Sample-Schichten einmal dekodieren und merken (Stufe 11a)
        {
            const std::lock_guard<std::mutex> halt(geholt->sperre);
            if (!geholt->samples)
            {
                for (const Schicht& schicht : geholt->instrument->schichten)
                {
                    if (schicht.schwingung != Schwingung::Sample || !schicht.an) continue;
                    if (sampleDaten(schicht.sample) != nullptr) continue;
                    const QString datei = sampleDatei(schicht.sample);
                    if (!datei.isEmpty()) merkeSampleDaten(schicht.sample, kSampleRate, m_dekodierer(datei, kSampleRate, kKlangMaxSekunden));
                    if (sampleDaten(schicht.sample) == nullptr && !geholt->zeilen.contains(sampleZeile(schicht.sample)))
                        geholt->zeilen << sampleZeile(schicht.sample);
                }
                geholt->samples = true;
            }
        }
        TonAngabe ton;
        (void)zerlegeTon(klang, ton);
        const auto anfang = Uhr::now();
        auto neu = std::make_shared<const Klang>(rendereTon(*geholt->instrument, ton, m_rate));
        tonspeicher().lege(schluessel, neu, msSeit(anfang));
        m_ergebnis[einer] = std::move(neu);
        return;
    }
    // eine eingefrorene Wellenform (Stufe 30f): aus dem Ordner `frozen`. Laesst sie sich nicht
    // lesen, geht sie — wer plant, nimmt den Clip dann live (`frozenFehlt`, `eingefrorenVon`)
    if (const QString abdruck = abdruckVonKlang(klang); !abdruck.isEmpty())
    {
        const auto anfang = Uhr::now();
        std::optional<std::vector<float>> welle = holeFrozen(abdruck.toLatin1(), m_rate);
        if (welle && !welle->empty())
        {
            Klang neu;
            neu.daten = std::move(*welle);
            auto geteilt = std::make_shared<const Klang>(std::move(neu));
            tonspeicher().lege(schluessel, geteilt, msSeit(anfang));
            m_ergebnis[einer] = std::move(geteilt);
            return;
        }
        vergissFrozen(abdruck.toLatin1());
        m_zeilen[einer] << QStringLiteral("Frozen clip not readable, playing live: %1").arg(abdruck);
        return;
    }
    // ein dekodierter Klang (TV3 a)
    const QString datei = klangDatei(klang);
    const auto anfang = Uhr::now();
    Klang neu;
    neu.daten = m_dekodierer(datei, m_rate, kKlangMaxSekunden);
    if (!neu.daten.empty())
    {
        auto geteilt = std::make_shared<const Klang>(std::move(neu));
        if (!schluessel.isEmpty()) tonspeicher().lege(schluessel, geteilt, msSeit(anfang));
        m_ergebnis[einer] = std::move(geteilt);
    }
    else
        m_zeilen[einer] << QStringLiteral("Sound not loaded: %1").arg(QDir::toNativeSeparators(klang));
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
        if (const Geholt* geholt = m_gemeinsam->finde(ton.instrument)) nimm(geholt->zeilen);
    }
    return alle;
}

}  // namespace lumi::loop
