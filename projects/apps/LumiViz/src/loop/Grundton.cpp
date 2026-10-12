/**
 * @file Grundton.cpp
 * @brief Loop: den Grundton einer Aufnahme messen (Stufe 11a, Schritt 3)
 */

#include "loop/Grundton.hpp"

#include "loop/Instrument.hpp"

#include <algorithm>
#include <cmath>

namespace lumi::loop
{

namespace
{

constexpr std::int64_t kFensterMax = 2048;  ///< so viele Abtastwerte werden je Stelle verglichen
constexpr std::int64_t kFramesMin = 1024;   ///< darunter wird nicht gemessen
constexpr int kStellen = 5;                 ///< an so vielen Stellen der Aufnahme
constexpr double kSchwelle = 0.15;          ///< darunter gilt eine Verschiebung als Periode
constexpr double kSchwelleWeit = 0.30;      ///< ohne eine solche: das kleinste, wenn es darunter liegt
constexpr double kLeise = 1e-3;             ///< eine Stelle unter diesem Pegel zaehlt nicht
constexpr double kEinigHalbtoene = 0.5;     ///< so nah muessen die Stellen beieinander liegen

/// Die Periode an einer Stelle, in Abtastwerten (mit Bruchteil); 0 = keine.
double periodeBei(const std::vector<double>& mono, std::int64_t ab, std::int64_t fenster, std::int64_t tauVon,
                  std::int64_t tauBis)
{
    // die Abweichung von sich selbst je Verschiebung, bezogen auf ihren laufenden Mittelwert
    std::vector<double> d(static_cast<std::size_t>(tauBis + 1), 1.0);
    double summe = 0.0;
    for (std::int64_t tau = 1; tau <= tauBis; ++tau)
    {
        double abweichung = 0.0;
        for (std::int64_t j = 0; j < fenster; ++j)
        {
            const double delta = mono[static_cast<std::size_t>(ab + j)] - mono[static_cast<std::size_t>(ab + j + tau)];
            abweichung += delta * delta;
        }
        summe += abweichung;
        d[static_cast<std::size_t>(tau)] = summe > 0.0 ? abweichung * static_cast<double>(tau) / summe : 1.0;
    }
    // die erste Verschiebung unter der Schwelle, bis in ihr Tal — so gewinnt die Periode
    // und nicht ihr Doppeltes
    std::int64_t beste = 0;
    for (std::int64_t tau = tauVon; tau <= tauBis; ++tau)
    {
        if (d[static_cast<std::size_t>(tau)] >= kSchwelle) continue;
        while (tau + 1 <= tauBis && d[static_cast<std::size_t>(tau + 1)] < d[static_cast<std::size_t>(tau)]) ++tau;
        beste = tau;
        break;
    }
    if (beste == 0)
    {
        std::int64_t kleinste = tauVon;
        for (std::int64_t tau = tauVon; tau <= tauBis; ++tau)
            if (d[static_cast<std::size_t>(tau)] < d[static_cast<std::size_t>(kleinste)]) kleinste = tau;
        if (d[static_cast<std::size_t>(kleinste)] >= kSchwelleWeit) return 0.0;
        beste = kleinste;
    }
    // zwischen den Abtastwerten: der Scheitel einer Parabel durch drei Punkte
    if (beste > tauVon && beste < tauBis)
    {
        const double a = d[static_cast<std::size_t>(beste - 1)];
        const double b = d[static_cast<std::size_t>(beste)];
        const double c = d[static_cast<std::size_t>(beste + 1)];
        const double nenner = a - 2.0 * b + c;
        if (std::abs(nenner) > 1e-12) return static_cast<double>(beste) + 0.5 * (a - c) / nenner;
    }
    return static_cast<double>(beste);
}

}  // namespace

Grundton messeGrundton(const std::vector<float>& stereo, int rate, std::int64_t abFrame)
{
    Grundton ergebnis;
    const auto alle = static_cast<std::int64_t>(stereo.size() / 2);
    const std::int64_t ab = std::clamp<std::int64_t>(abFrame, 0, alle);
    const std::int64_t frames = alle - ab;
    if (rate <= 0 || frames < kFramesMin) return ergebnis;

    std::vector<double> mono(static_cast<std::size_t>(frames));
    double spitze = 0.0;
    for (std::int64_t t = 0; t < frames; ++t)
    {
        const auto i = static_cast<std::size_t>(2 * (ab + t));
        mono[static_cast<std::size_t>(t)] = 0.5 * (static_cast<double>(stereo[i]) + stereo[i + 1]);
        spitze = std::max(spitze, std::abs(mono[static_cast<std::size_t>(t)]));
    }
    if (spitze < kLeise) return ergebnis;

    // ein Fenster und dahinter Platz fuer die laengste Verschiebung
    const std::int64_t fenster = std::min(kFensterMax, frames / 2);
    const auto tauVon = std::max<std::int64_t>(2, static_cast<std::int64_t>(rate / kGrundtonBisHz));
    const std::int64_t tauBis = std::min(static_cast<std::int64_t>(rate / kGrundtonVonHz), frames - fenster - 1);
    if (tauBis <= tauVon + 2) return ergebnis;

    // die Stellen: hinter dem Anschlag, ueber die erste Haelfte verteilt — dort klingt ein Ton am klarsten
    const std::int64_t spiel = frames - fenster - tauBis - 1;
    std::vector<double> halbtoene;
    int gemessen = 0;
    for (int stelle = 0; stelle < kStellen; ++stelle)
    {
        const std::int64_t von = spiel <= 0 ? 0 : static_cast<std::int64_t>(static_cast<double>(spiel) * (0.10 + 0.125 * stelle));
        double energie = 0.0;
        for (std::int64_t j = 0; j < fenster; ++j) energie += mono[static_cast<std::size_t>(von + j)] * mono[static_cast<std::size_t>(von + j)];
        if (std::sqrt(energie / static_cast<double>(fenster)) < kLeise) continue;  // hier ist nichts zu hoeren
        ++gemessen;
        const double periode = periodeBei(mono, von, fenster, tauVon, tauBis);
        if (periode > 0.0) halbtoene.push_back(69.0 + 12.0 * std::log2(rate / periode / 440.0));
        if (spiel <= 0) break;  // die Aufnahme reicht nur fuer eine Stelle
    }
    if (halbtoene.empty() || 2 * static_cast<int>(halbtoene.size()) < gemessen) return ergebnis;

    std::sort(halbtoene.begin(), halbtoene.end());
    const double mitte = halbtoene[halbtoene.size() / 2];
    // einig: wenigstens die Haelfte der gemessenen Stellen liegt beim mittleren Wert
    double summe = 0.0;
    int einig = 0;
    for (const double wert : halbtoene)
        if (std::abs(wert - mitte) <= kEinigHalbtoene)
        {
            summe += wert;
            ++einig;
        }
    if (2 * einig < gemessen) return ergebnis;

    const double ton = summe / einig;
    ergebnis.gefunden = true;
    ergebnis.hz = 440.0 * std::exp2((ton - 69.0) / 12.0);
    ergebnis.hoehe = static_cast<int>(std::lround(ton));
    ergebnis.cent = (ton - ergebnis.hoehe) * 100.0;
    return ergebnis;
}

QString grundtonText(const Grundton& grundton)
{
    if (!grundton.gefunden) return {};
    const auto cent = static_cast<int>(std::lround(grundton.cent));
    if (cent == 0) return tonName(grundton.hoehe);
    return QStringLiteral("%1 %2%3 ct").arg(tonName(grundton.hoehe), cent > 0 ? QStringLiteral("+") : QString()).arg(cent);
}

}  // namespace lumi::loop
