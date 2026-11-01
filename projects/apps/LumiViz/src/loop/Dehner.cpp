/**
 * @file Dehner.cpp
 * @brief Loop: Dauer und Tonhoehe mit Signalsmith Stretch (Stufe 15f, S169)
 */

#include "loop/Dehner.hpp"

#include <signalsmith-stretch.h>

#include <algorithm>
#include <cmath>
#include <random>

namespace lumi::loop
{

namespace
{

/// Der Samen der Zufallszahlen (von Claude gesetzt): Stretch verteilt die Zeit je Band ein
/// wenig zufaellig. Fest, damit derselbe Klang immer gleich gerechnet wird (Tonspeicher, Freeze).
constexpr long kSamen = 0x4C756D69;  // „Lumi"

/// Zwei Kanaele als Zeiger, ab einer Stelle — so liest und schreibt Stretch (`kanal[c][i]`).
struct Kanaele
{
    float* links = nullptr;
    float* rechts = nullptr;
    float* operator[](int c) const { return c == 0 ? links : rechts; }
};

/// Signalsmith Stretch (externals/signalsmith-stretch, WV6 a): fuer jedes Material, *General*.
/// Fuer ein Stueck fester Laenge wie im README der Bibliothek: am Anfang `seek` mit
/// `inputLatency()` Abtastwerten, am Ende `inputLatency()` Stille und `flush`; das Ergebnis traegt
/// `outputLatency()` Vorlauf, der wegfaellt.
class DehnerSignalsmith final : public Dehner
{
public:
    [[nodiscard]] std::string name() const override { return "Signalsmith Stretch 1.4.0"; }

    [[nodiscard]] std::vector<float> rechne(const std::vector<float>& stereo, int rate, std::int64_t ausFrames,
                                            const std::function<double(std::int64_t)>& halbtoene) const override
    {
        if (ausFrames <= 0 || rate <= 0) return {};
        std::vector<float> ergebnis(static_cast<std::size_t>(2 * ausFrames), 0.0f);
        const auto n = static_cast<std::int64_t>(stereo.size() / 2);
        if (n == 0) return ergebnis;

        signalsmith::stretch::SignalsmithStretch<float, std::mt19937> stretch(kSamen);
        stretch.presetDefault(2, static_cast<float>(rate));
        const std::int64_t ein = stretch.inputLatency();
        const std::int64_t aus = stretch.outputLatency();

        // getrennt nach Kanaelen, hinten Stille: `ein` Abtastwerte, damit die Rechnung das Ende
        // erreicht — und bei einem Klang kuerzer als `ein` so viel, dass `seek` `ein` lesen kann
        const std::int64_t alle = std::max(n, ein) + ein;
        std::vector<float> links(static_cast<std::size_t>(alle), 0.0f);
        std::vector<float> rechts(static_cast<std::size_t>(alle), 0.0f);
        for (std::int64_t i = 0; i < n; ++i)
        {
            links[static_cast<std::size_t>(i)] = stereo[static_cast<std::size_t>(2 * i)];
            rechts[static_cast<std::size_t>(i)] = stereo[static_cast<std::size_t>(2 * i + 1)];
        }
        std::vector<float> ausLinks(static_cast<std::size_t>(ausFrames + aus), 0.0f);
        std::vector<float> ausRechts(static_cast<std::size_t>(ausFrames + aus), 0.0f);

        const double verhaeltnis = static_cast<double>(n) / static_cast<double>(ausFrames);
        stretch.seek(Kanaele{links.data(), rechts.data()}, static_cast<int>(ein), verhaeltnis);
        // die Stelle der Eingabe, bis zu der gelesen ist: nach `seek` steht die Rechnung am Anfang
        // des Klangs, gelesen sind seine ersten `ein` Abtastwerte; `process` liest danach genau n
        const std::int64_t anfang = ein;
        std::int64_t gelesen = anfang;
        double tonhoehe = std::nan("");
        for (std::int64_t k = 0; k < ausFrames; k += kDehnerBlock)
        {
            const std::int64_t block = std::min<std::int64_t>(kDehnerBlock, ausFrames - k);
            // die Tonhoehe gilt an der Stelle, die gerade gerechnet wird — `aus` vor der Ausgabe,
            // also genau bei Abtastwert k des Ergebnisses
            const double ht = halbtoene ? halbtoene(k + block / 2) : 0.0;
            if (!(ht == tonhoehe))
            {
                stretch.setTransposeSemitones(static_cast<float>(ht));
                tonhoehe = ht;
            }
            const std::int64_t bis = anfang + std::llround(static_cast<double>(k + block) * verhaeltnis);
            const std::int64_t lies = std::max<std::int64_t>(0, bis - gelesen);
            stretch.process(Kanaele{links.data() + gelesen, rechts.data() + gelesen}, static_cast<int>(lies),
                            Kanaele{ausLinks.data() + k, ausRechts.data() + k}, static_cast<int>(block));
            gelesen += lies;
        }
        stretch.flush(Kanaele{ausLinks.data() + ausFrames, ausRechts.data() + ausFrames}, static_cast<int>(aus));

        for (std::int64_t k = 0; k < ausFrames; ++k)
        {
            ergebnis[static_cast<std::size_t>(2 * k)] = ausLinks[static_cast<std::size_t>(k + aus)];
            ergebnis[static_cast<std::size_t>(2 * k + 1)] = ausRechts[static_cast<std::size_t>(k + aus)];
        }
        return ergebnis;
    }
};

}  // namespace

std::vector<std::unique_ptr<Dehner>> dehnerUmsetzungen()
{
    std::vector<std::unique_ptr<Dehner>> alle;
    alle.push_back(std::make_unique<DehnerSignalsmith>());
    return alle;
}

const Dehner& dehner()
{
    static const DehnerSignalsmith vorgabe;
    return vorgabe;
}

}  // namespace lumi::loop
