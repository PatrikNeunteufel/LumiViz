/**
 ****************************************************************************************
 * @file   Modellton.hpp
 * @brief  7.5.9 (SM4 b): der Ton des Modells fuer die Suche — je Taste die drei Ebenen v6, v10, v16 gerechnet wie im
 *         Programm, an der Struktur des Ziels gemessen und gegen Ziel und Toleranz verglichen
 *         (docs/audio/Spielweisen_Konzept.md §4.1; Entscheide Patrik S155/S156/S161: K4 a, K5 a, D1 a, SG1 a, ZF5 a)
 *
 * @details
 * Eine Auswertung ist ein Satz Kurven (eine `HammerSaite`) an allen Tasten: je Taste drei Toene an den Stufen der Ebenen
 * v6, v10, v16 (−20, −12, 0 dB; SG1 a), gehalten so lang, wie die Struktur von *Salamander* an der Taste reicht, und
 * 1,5 s mehr (K5 a, mindestens 2 s), gemessen mit `messeTon` (Kanalmittel, `huellen`) und an der Struktur
 * (`modellGroessen`), verglichen mit `vergleiche` (`Zielsumme.hpp`). `werteAus` rechnet viele Auswertungen zugleich:
 * alle Toene aller Saiten auf einem Pool, die laengsten zuerst — jeder Ton rechnet fuer sich, das Ergebnis ist dasselbe
 * wie nacheinander. Hoechstens `kPoolHoechstens` Threads: der Prozessor steht an der Waermegrenze, mehr machen die
 * Rechnung langsamer (S156, `LUMIVIZ_75_ZEIT`).
 ****************************************************************************************
 */

#pragma once

#include "Fluegelmessung.hpp"
#include "ModellMessung.hpp"
#include "Strukturmessung.hpp"
#include "Zielsumme.hpp"

#include "loop/Fft.hpp"
#include "loop/HammerSaite.hpp"
#include "loop/HammerTon.hpp"
#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"

#include <QString>

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <functional>
#include <numeric>
#include <thread>
#include <vector>

namespace modelltest
{

/// So viele Threads rechnen hoechstens (S156: am schnellsten, mehr werden langsamer — Waermegrenze).
inline constexpr unsigned kPoolHoechstens = 11;

inline unsigned poolThreads()
{
    return std::max(1u, std::min(kPoolHoechstens, std::thread::hardware_concurrency()));
}

/// `einer(i)` fuer i = 0 … zahl − 1 auf `threads` Threads; jede Aufgabe rechnet fuer sich.
inline void parallelJe(int zahl, unsigned threads, const std::function<void(int)>& einer)
{
    std::atomic<int> naechste{0};
    std::vector<std::thread> arbeiter;
    for (unsigned a = 0; a < std::max(1u, threads); ++a)
        arbeiter.emplace_back([&] {
            for (int i = naechste++; i < zahl; i = naechste++) einer(i);
        });
    for (std::thread& t : arbeiter) t.join();
}

/// Der Fluegel der Probe und der Suche mit der Saite `saite` (Boden `steinway_d`, ungedaempft ab 89 nach D1 a).
inline lumi::loop::Instrument fluegelMit(const lumi::loop::HammerSaite& saite)
{
    lumi::loop::Instrument instrument = fluegel(QStringLiteral("lib:steinway_d"), &saite);
    instrument.tastatur->ungedaempftAb = 89;  // D1 a
    return instrument;
}

/// Die Ebenen der Suche (SG1 a): v6, v10, v16.
inline constexpr std::array<int, 3> kSuchEbenen{6, 10, 16};

/// So lange wird ein Ton der Suche gehalten (K5 a): das groesste Ende der Teilton-Strukturen und 1,5 s, mindestens 2 s.
inline int dauerMsAus(const lumi::messung::TonStruktur& struktur)
{
    std::size_t bis = 0;
    for (const lumi::messung::TeiltonStruktur& s : struktur)
        if (s.da) bis = std::max(bis, s.bis);
    const int ms = static_cast<int>(static_cast<double>(bis) * lumi::messung::Einstellungen{}.schrittS * 1000.0) + 1500;
    return std::max(ms, 2000);
}

/// Ein Ton des Modells gemessen wie *Salamander* (Kanalmittel und Huellen, ohne Phantome).
struct EbenenMessung
{
    lumi::messung::TonHuellen huellen;
    std::array<lumi::messung::KanalMessung, 2> messung;
};

inline EbenenMessung messeEbene(const lumi::loop::Instrument& instrument, int midi, int ebene, int dauerMs)
{
    lumi::loop::TonAngabe a;
    a.hoehe = midi;
    a.dauerMs = dauerMs;
    a.anschlagStufe = stufeVonEbene(ebene);
    const lumi::loop::Klang klang = lumi::loop::rendereTon(instrument, a, kRate);
    EbenenMessung m;
    m.messung = lumi::messung::messeTon(klang.daten, kRate, lumi::messung::tastenFrequenz(midi), lumi::loop::fft(), false, nullptr, nullptr,
                                        &m.huellen);
    return m;
}

/// Die Groessen des Modells an einer Taste aus ihren drei Ebenen.
inline std::vector<lumi::messung::GroesseJeEbene> groessenAus(const std::vector<EbenenMessung>& ebenen, const lumi::messung::TonStruktur& struktur)
{
    std::vector<lumi::messung::TonHuellen> h;
    std::vector<std::array<lumi::messung::KanalMessung, 2>> m;
    for (const EbenenMessung& e : ebenen)
    {
        h.push_back(e.huellen);
        m.push_back(e.messung);
    }
    return lumi::messung::modellGroessen(h, m, struktur);
}

/// Die Groessen des Modells mit der Saite `saite` an einer Taste (drei Ebenen auf dem Pool).
inline std::vector<lumi::messung::GroesseJeEbene> modellAn(const lumi::loop::HammerSaite& saite, int midi, const lumi::messung::TonStruktur& struktur)
{
    const lumi::loop::Instrument instrument = fluegelMit(saite);
    const int dauerMs = dauerMsAus(struktur);
    std::vector<EbenenMessung> ebenen(kSuchEbenen.size());
    parallelJe(static_cast<int>(kSuchEbenen.size()), poolThreads(),
               [&](int i) { ebenen[static_cast<std::size_t>(i)] = messeEbene(instrument, midi, kSuchEbenen[static_cast<std::size_t>(i)], dauerMs); });
    return groessenAus(ebenen, struktur);
}

/// Eine Auswertung: je Taste die Summe, dazu Σ r² ueber alle Tasten.
struct Auswertung
{
    std::vector<lumi::messung::TastenSumme> tasten;
    double summe = 0.0;
};

/// Viele Auswertungen zugleich: je Saite alle `tasten` an den Ebenen der Suche, alle Toene auf einem Pool (die laengsten
/// zuerst). `sekunden` (wenn gegeben) bekommt die Rechenzeit des ganzen Aufrufs.
inline std::vector<Auswertung> werteAus(const std::vector<lumi::loop::HammerSaite>& saiten, const std::vector<int>& tasten,
                                        const lumi::messung::Strukturen& strukturen, const lumi::messung::Ziele& ziele, double* sekunden = nullptr)
{
    const auto start = std::chrono::steady_clock::now();
    std::vector<lumi::loop::Instrument> instrumente;
    for (const lumi::loop::HammerSaite& s : saiten) instrumente.push_back(fluegelMit(s));
    const auto strukturAn = [&strukturen](int midi) {
        const auto it = strukturen.find(midi);
        return it != strukturen.end() ? it->second : lumi::messung::TonStruktur{};
    };
    std::vector<int> dauer;
    for (const int m : tasten) dauer.push_back(dauerMsAus(strukturAn(m)));

    // je Ton (Saite, Taste, Ebene) ein Auftrag; die laengsten zuerst, damit der Pool am Ende nicht auf einen wartet
    const std::size_t je = kSuchEbenen.size();
    const std::size_t zahl = saiten.size() * tasten.size() * je;
    std::vector<std::size_t> reihe(zahl);
    std::iota(reihe.begin(), reihe.end(), std::size_t{0});
    std::stable_sort(reihe.begin(), reihe.end(), [&](std::size_t a, std::size_t b) { return dauer[(a / je) % tasten.size()] > dauer[(b / je) % tasten.size()]; });
    std::vector<EbenenMessung> messungen(zahl);
    parallelJe(static_cast<int>(zahl), poolThreads(), [&](int i) {
        const std::size_t auftrag = reihe[static_cast<std::size_t>(i)];
        const std::size_t taste = (auftrag / je) % tasten.size();
        const std::size_t saite = auftrag / (je * tasten.size());
        messungen[auftrag] = messeEbene(instrumente[saite], tasten[taste], kSuchEbenen[auftrag % je], dauer[taste]);
    });

    std::vector<Auswertung> alle(saiten.size());
    for (std::size_t s = 0; s < saiten.size(); ++s)
        for (std::size_t t = 0; t < tasten.size(); ++t)
        {
            const auto ab = messungen.begin() + static_cast<std::ptrdiff_t>((s * tasten.size() + t) * je);
            const std::vector<EbenenMessung> ebenen(ab, ab + static_cast<std::ptrdiff_t>(je));
            const auto z = ziele.find(tasten[t]);
            const lumi::messung::TastenZiele leer;
            lumi::messung::TastenSumme summe =
                lumi::messung::vergleiche(tasten[t], z != ziele.end() ? z->second : leer, groessenAus(ebenen, strukturAn(tasten[t])));
            alle[s].summe += summe.summe;
            alle[s].tasten.push_back(std::move(summe));
        }
    if (sekunden != nullptr) *sekunden = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    return alle;
}

/// Die 30 Tasten von *Salamander* als MIDI-Nummern.
inline std::vector<int> salamanderMidi()
{
    std::vector<int> m;
    for (const lumi::messung::SalamanderTaste& t : lumi::messung::salamanderTasten()) m.push_back(t.midi);
    return m;
}

}  // namespace modelltest
