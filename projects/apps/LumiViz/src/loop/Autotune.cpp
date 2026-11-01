/**
 * @file Autotune.cpp
 * @brief Loop: die Spur Autotune rechnen (Stufe 15i, S170)
 */

#include "loop/Autotune.hpp"

#include <algorithm>
#include <array>
#include <cmath>

namespace lumi::loop
{

namespace
{

/// Die Stufen von Dur und natuerlichem Moll ab dem Grundton.
constexpr std::array<int, 7> kDur{0, 2, 4, 5, 7, 9, 11};
constexpr std::array<int, 7> kMoll{0, 2, 3, 5, 7, 8, 10};

/// Der Abschnitt, in dem Sekunde `t` liegt (Abschnitte nach Anfang sortiert, ohne Ueberlappung);
/// nullptr = keiner.
const AutotuneAbschnitt* abschnittBei(const std::vector<AutotuneAbschnitt>& abschnitte, double t)
{
    const auto nach = std::upper_bound(abschnitte.begin(), abschnitte.end(), t,
                                       [](double s, const AutotuneAbschnitt& a) { return s < a.anfang; });
    if (nach == abschnitte.begin()) return nullptr;
    const AutotuneAbschnitt& a = *(nach - 1);
    return t < a.ende ? &a : nullptr;
}

/// Wie weit ein Pol in `dt` Sekunden zum Ziel geht, mit `speedMs` (95 % nach Speed).
double schrittAnteil(double speedMs, double dt)
{
    if (speedMs <= 0.0) return 1.0;
    return 1.0 - std::exp(-dt / (speedMs / 3000.0));
}

}  // namespace

double midiVonHz(double hz)
{
    return 69.0 + 12.0 * std::log2(hz / 440.0);
}

bool inTonart(int ton, int grundton, Skala skala)
{
    if (skala == Skala::Chromatisch) return true;
    const int stufe = ((ton - grundton) % 12 + 12) % 12;
    const auto& stufen = skala == Skala::Dur ? kDur : kMoll;
    return std::find(stufen.begin(), stufen.end(), stufe) != stufen.end();
}

int naechsterTon(double midi, int grundton, Skala skala)
{
    const auto mitte = static_cast<int>(std::lround(midi));
    int beste = mitte;
    double abstand = 1e9;
    for (int ton = mitte - 2; ton <= mitte + 2; ++ton)
        if (inTonart(ton, grundton, skala) && std::abs(midi - ton) < abstand)
        {
            abstand = std::abs(midi - ton);
            beste = ton;
        }
    return std::clamp(beste, 0, 127);
}

std::vector<double> autotuneKorrektur(const std::vector<AutotuneAbschnitt>& abschnitte, const Tonhoehenverlauf& verlauf)
{
    std::vector<double> korrektur(verlauf.hz.size(), 0.0);
    if (abschnitte.empty() || verlauf.rate <= 0 || verlauf.schritt <= 0) return korrektur;
    const double dt = static_cast<double>(verlauf.schritt) / verlauf.rate;
    double wert = 0.0;     // die Korrektur, wie sie dem Ziel folgt
    double gehalten = 0.0; // der letzte Zielwert im Abschnitt (fuer stumme Rahmen)
    const AutotuneAbschnitt* voriger = nullptr;
    double ausklingMs = 0.0;
    for (std::size_t k = 0; k < verlauf.hz.size(); ++k)
    {
        const double t = verlauf.zeitS(k);
        const AutotuneAbschnitt* a = abschnittBei(abschnitte, t);
        double ziel = 0.0;
        double speedMs = ausklingMs;
        if (a != nullptr)
        {
            if (a != voriger) gehalten = 0.0;  // ein neuer Abschnitt haelt nichts vom alten
            if (verlauf.hz[k] > 0.0f)
                gehalten = a->staerke * (a->zielHoehe + a->zielCent / 100.0 - midiVonHz(verlauf.hz[k]));
            ziel = gehalten;
            speedMs = a->speedMs;
            ausklingMs = a->speedMs;
        }
        voriger = a;
        wert += (ziel - wert) * schrittAnteil(speedMs, dt);
        korrektur[k] = wert;
    }
    return korrektur;
}

double autotuneBei(const std::vector<double>& korrektur, const Tonhoehenverlauf& verlauf, double sekunde)
{
    if (korrektur.empty() || verlauf.rate <= 0 || verlauf.schritt <= 0) return 0.0;
    const double j = sekunde * verlauf.rate / verlauf.schritt;
    if (j < 0.0 || j > static_cast<double>(korrektur.size() - 1)) return 0.0;
    const auto unten = static_cast<std::size_t>(j);
    const std::size_t oben = std::min(unten + 1, korrektur.size() - 1);
    const double f = j - static_cast<double>(unten);
    return korrektur[unten] + (korrektur[oben] - korrektur[unten]) * f;
}

NotenErkennung erkenneNoten(const Tonhoehenverlauf& verlauf, const Autotune& autotune, double von, double bis)
{
    NotenErkennung ergebnis;
    if (verlauf.rate <= 0 || verlauf.schritt <= 0 || verlauf.hz.empty() || !(bis > von)) return ergebnis;
    const double dt = static_cast<double>(verlauf.schritt) / verlauf.rate;

    // die laufende Note: ihr Ton, der erste und letzte gesungene Rahmen
    int ton = -1;
    std::size_t erster = 0;
    std::size_t letzter = 0;
    const auto schliesse = [&] {
        if (ton < 0) return;
        const double anfang = std::max(von, verlauf.zeitS(erster));
        const double ende = std::min(bis, verlauf.zeitS(letzter) + dt);
        if (ende - anfang >= kNotenMinS)
            ergebnis.abschnitte.push_back({anfang, ende, ton, 0, autotune.staerke, autotune.speedMs});
        ton = -1;
    };

    std::size_t rahmen = 0;
    std::size_t gesungen = 0;
    std::vector<float> sicher;
    for (std::size_t k = 0; k < verlauf.hz.size(); ++k)
    {
        const double t = verlauf.zeitS(k);
        if (t < von || t >= bis) continue;
        ++rahmen;
        if (verlauf.hz[k] <= 0.0f)
        {
            if (ton >= 0 && t - (verlauf.zeitS(letzter) + dt) > kNotenLueckeS) schliesse();
            continue;
        }
        ++gesungen;
        if (k < verlauf.stimmhaft.size()) sicher.push_back(verlauf.stimmhaft[k]);
        const double midi = midiVonHz(verlauf.hz[k]);
        const int naechster = naechsterTon(midi, autotune.grundton, autotune.skala);
        const bool bleibt = ton >= 0 && std::abs(midi - ton) < std::abs(midi - naechster) + kNotenHysterese &&
                            t - (verlauf.zeitS(letzter) + dt) <= kNotenLueckeS;
        if (!bleibt)
        {
            schliesse();
            ton = naechster;
            erster = k;
        }
        letzter = k;
    }
    schliesse();

    if (rahmen > 0 && !sicher.empty())
    {
        std::nth_element(sicher.begin(), sicher.begin() + static_cast<std::ptrdiff_t>(sicher.size() / 2), sicher.end());
        ergebnis.eineStimme = static_cast<double>(gesungen) >= 0.3 * static_cast<double>(rahmen) && sicher[sicher.size() / 2] >= 0.6f;
    }
    return ergebnis;
}

void ersetzeAbschnitte(Autotune& autotune, const std::vector<AutotuneAbschnitt>& neue, double von, double bis)
{
    std::vector<AutotuneAbschnitt> bleiben;
    for (const AutotuneAbschnitt& a : autotune.abschnitte)
    {
        if (a.ende <= von || a.anfang >= bis)
        {
            bleiben.push_back(a);
            continue;
        }
        // ragt hinein: die Teile ausserhalb bleiben
        if (a.anfang < von)
        {
            AutotuneAbschnitt vorn = a;
            vorn.ende = von;
            bleiben.push_back(vorn);
        }
        if (a.ende > bis)
        {
            AutotuneAbschnitt hinten = a;
            hinten.anfang = bis;
            bleiben.push_back(hinten);
        }
    }
    for (const AutotuneAbschnitt& a : neue) bleiben.push_back(a);
    autotune.abschnitte = std::move(bleiben);
    richteAutotune(autotune);
}

bool teileAbschnitt(Autotune& autotune, double sekunde)
{
    for (std::size_t i = 0; i < autotune.abschnitte.size(); ++i)
    {
        AutotuneAbschnitt& a = autotune.abschnitte[i];
        if (sekunde <= a.anfang || sekunde >= a.ende) continue;
        if (sekunde - a.anfang < kNotenMinS / 2.0 || a.ende - sekunde < kNotenMinS / 2.0) return false;
        AutotuneAbschnitt hinten = a;
        hinten.anfang = sekunde;
        a.ende = sekunde;
        autotune.abschnitte.insert(autotune.abschnitte.begin() + static_cast<std::ptrdiff_t>(i + 1), hinten);
        return true;
    }
    return false;
}

bool verbindeAbschnitte(Autotune& autotune, std::size_t i)
{
    if (i + 1 >= autotune.abschnitte.size()) return false;
    autotune.abschnitte[i].ende = autotune.abschnitte[i + 1].ende;
    autotune.abschnitte.erase(autotune.abschnitte.begin() + static_cast<std::ptrdiff_t>(i + 1));
    return true;
}

}  // namespace lumi::loop
