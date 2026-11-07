/**
 * @file HammerSaite.cpp
 * @brief InstrumentModels, Kern `hammerString`: Kurven, Grenzen und Vorgaben der Hammer-Saite (bis S175
 *        `src/loop/HammerSaite.cpp` der App; die Zahlen wortgleich, dazu Belegkraft und Herkunft je Kurve)
 */
#include "InstrumentModels/hammer/HammerSaite.hpp"

#include "InstrumentCore/Anschlag.hpp"

#include <algorithm>
#include <cmath>
#include <string>

namespace lumi::kern::hammer
{

namespace
{

/// Die Tasten der Vorgabe: A0, A1, … A7 und C8.
constexpr int kVorgabeTasten[] = {21, 33, 45, 57, 69, 81, 93, 105, 108};

/// Die Tasten der Hammer-Vorgabe: alle sechs Tasten und C8 — Q0 ist bei RT-0425 in ln quadratisch in der Taste; an
/// den A-Tasten allein laege die Kurve dazwischen bis 4 % daneben.
constexpr int kHammerTasten[] = {21, 27, 33, 39, 45, 51, 57, 63, 69, 75, 81, 87, 93, 99, 105, 108};

template <class Formel, std::size_t N>
Kurve kurveAus(const int (&tasten)[N], Formel formel, Belegkraft belegkraft, std::string herkunft)
{
    Kurve kurve;
    for (const int taste : tasten) kurve.punkte.push_back(NotenWert{taste, formel(taste)});
    kurve.belegkraft = belegkraft;
    kurve.herkunft = std::move(herkunft);
    return kurve;
}

constexpr std::string_view kSalamander = "measured at Salamander (Yamaha C5) through the model, 31d-4";

HammerSaite baueVorgabe()
{
    HammerSaite s;
    // B: im Bass bis E2 gemessen an Salamander (Yamaha C5), gesetzt durch das Modell hindurch (31d-4, S2 a);
    // darueber Rigaud Gl. 5 an den Tasten der Vorgabe (UF5 a: B im Diskant bleibt bei Rigaud). Die Belegkraft ist
    // die schwaechere der beiden (Quelle), die Herkunft nennt die Bereiche.
    s.inharmonizitaet = kurveAusPunkten({{21, 2.177e-4}, {24, 1.593e-4}, {27, 1.278e-4}, {30, 8.824e-5}, {33, 8.795e-5},
                                         {36, 8.776e-5}, {39, 7.436e-5}},
                                        Belegkraft::Quelle,
                                        "to E2 measured at Salamander through the model (31d-4, S2 a); above Rigaud, "
                                        "David & Daudet 2011, eq. 5");
    for (const int m : kVorgabeTasten)
        if (m > 40) s.inharmonizitaet.punkte.push_back(NotenWert{m, rigaudB(m, kRigaudBassSteigung, kRigaudBassAchse)});
    // belegt (HU3-1 c, S159; davor HT2 a nach RT-0425 Gl. 2–4): der Hammer nach Stulov & Kartofelev 2009 Gl. 9
    // (Taste n = MIDI − 20) — die Helligkeit je Staerke an Salamander trifft Stulov, RT-0425 kaum (Befund S159)
    const std::string stulov = "Stulov & Kartofelev 2009, eq. 9 (HU3-1 c)";
    s.hammerMasseG = kurveAus(kHammerTasten, [](int m) { return stulovHammerMasseG(m - 20); }, Belegkraft::Quelle, stulov);
    s.hammerExponent = kurveAus(kHammerTasten, [](int m) { return stulovHammerExponent(m - 20); }, Belegkraft::Quelle, stulov);
    s.hammerSteife = kurveAus(kHammerTasten, [](int m) { return stulovHammerSteife(m - 20); }, Belegkraft::Quelle, stulov);
    s.hammerHysterese = kurveAus(kHammerTasten, [](int m) { return stulovHammerHysterese(m - 20); }, Belegkraft::Quelle, stulov);
    s.hammerRelaxationUs =
        kurveAus(kHammerTasten, [](int m) { return stulovHammerRelaxationUs(m - 20); }, Belegkraft::Quelle, stulov);
    // belegt (HT1 a): RT-0425 Anhang A, C1 … B7 — µ = ρπd²/4, f_l0/f0 = √(EA/T0) mit E = 2,02·10¹¹ Pa; die Tasten so
    // gewaehlt, dass die Kurve jede Taste der Tabelle auf 2 % trifft (E2 | F2: der Sprung am Bassteg)
    const std::string rt = "Chabassier & Durufle, INRIA RT-0425 (2012), appendix A (HT1 a)";
    s.saitenLaengeM = kurveAusPunkten({{24, 2.007}, {31, 1.842}, {37, 1.548}, {39, 1.442}, {40, 1.378}, {41, 1.837},
                                       {42, 1.757}, {45, 1.482}, {52, 1.013}, {74, 0.308}, {107, 0.052}},
                                      Belegkraft::Quelle, rt);
    s.saitenMasseGm = kurveAusPunkten({{24, 99.41}, {28, 66.53}, {33, 31.57}, {35, 23.63}, {39, 15.32}, {40, 14.30},
                                       {41, 7.729}, {54, 6.733}, {70, 5.459}, {82, 4.972}, {94, 4.392}, {107, 3.404}},
                                      Belegkraft::Quelle, rt);
    s.laengsVerhaeltnis =
        kurveAusPunkten({{24, 14.21}, {40, 13.71}, {41, 15.74}, {55, 14.98}, {81, 13.59}, {107, 12.20}}, Belegkraft::Quelle, rt);
    // gemessen an Salamander (Yamaha C5) an den Phantomteiltoenen im Bass bis A2, darueber der letzte Wert (31d-4, S2 a;
    // MIDI 33 an der Grenze 10°) — die Fortsetzung ist nicht gemessen, also offen
    s.stegWinkelGrad = kurveAusPunkten({{21, 1.02}, {24, 0.6763}, {27, 0.6648}, {30, 0.9756}, {33, 10.0}, {36, 1.263},
                                        {39, 2.0}, {42, 3.532}, {45, 4.432}, {108, 4.432}},
                                       Belegkraft::Offen,
                                       "to A2 measured at Salamander at the phantom partials (31d-4, S2 a); above the last "
                                       "value held (open)");
    // x_H/L liegt dort zwischen 0,113 und 0,127, fast ueberall bei 0,120
    s.anschlagStelle = kurveAusPunkten({{21, 0.12}, {108, 0.12}}, Belegkraft::Quelle,
                                       "RT-0425 appendix A: x_H/L between 0.113 and 0.127, nearly everywhere 0.120");
    // gemessen an Salamander (Yamaha C5): T60 bei f1 aus dem Ausgleich ueber die Teiltoene, gesetzt durch das Modell
    // hindurch (31d-4, S1 a, S2 a, S3 a; drei Durchgaenge, 18 Runden); 105 ohne Treffer, vermerkt
    s.abklingS = kurveAusPunkten({{21, 15.99}, {24, 13.71}, {27, 18.04}, {30, 18.77}, {33, 11.99}, {36, 14.86}, {39, 12.02},
                                  {42, 7.494}, {45, 8.793}, {48, 5.843}, {51, 4.726}, {54, 3.92}, {57, 3.77}, {60, 5.294},
                                  {63, 2.053}, {66, 3.761}, {69, 3.713}, {72, 2.06}, {75, 7.973}, {78, 2.934}, {81, 1.621},
                                  {84, 1.075}, {87, 11.47}, {90, 4.109}, {93, 2.199}, {96, 0.5089}, {99, 1.507}, {102, 1.837}},
                                 Belegkraft::Gemessen, std::string(kSalamander));
    // vorlaeufig dreimal Decay (HT3 a) — das Modell zeigt keine zweite Stufe wie Salamander (S3 a)
    s.abklingSpaetS = kurveAusPunkten({{21, 47.96}, {24, 41.14}, {27, 54.11}, {30, 56.32}, {33, 35.98}, {36, 44.57},
                                       {39, 36.07}, {42, 22.48}, {45, 26.38}, {48, 17.53}, {51, 14.18}, {54, 11.76},
                                       {57, 11.31}, {60, 15.88}, {63, 6.16}, {66, 11.28}, {69, 11.14}, {72, 6.181},
                                       {75, 23.92}, {78, 8.801}, {81, 4.862}, {84, 3.225}, {87, 34.4}, {90, 12.33},
                                       {93, 6.596}, {96, 1.527}, {99, 4.522}, {102, 5.512}},
                                      Belegkraft::Offen, "provisional: three times Decay (HT3 a)");
    // gemessen an Salamander (Yamaha C5) aus demselben Ausgleich, durch das Modell hindurch (31d-4); 96 und 105 vermerkt
    s.abklingHoch = kurveAusPunkten({{21, 0.9791}, {24, 0.9888}, {27, 0.9689}, {30, 0.9549}, {33, 0.9718}, {36, 0.9409},
                                     {39, 0.9154}, {42, 0.9577}, {45, 0.9504}, {48, 0.9342}, {51, 0.8951}, {54, 0.7855},
                                     {57, 0.8781}, {60, 0.6443}, {63, 0.904}, {66, 0.6689}, {69, 0.4688}, {72, 0.524},
                                     {75, 0.1524}, {78, 0.2841}, {81, 0.3365}, {84, 0.3355}, {87, 0.01508}, {90, 0.03498},
                                     {93, 0.02882}, {99, 0.02764}, {102, 0.01849}},
                                    Belegkraft::Gemessen, std::string(kSalamander));
    s.chor = kurveAusPunkten({{21, 1.0}, {30, 1.0}, {31, 2.0}, {48, 2.0}, {49, 3.0}, {108, 3.0}}, Belegkraft::Offen,
                             "default of the grand piano (one string to F#1, two to C3, then three)");
    // gemessen an Salamander (Yamaha C5) an der Schwebung der Teiltoene 1–3, durch das Modell hindurch, nur bei
    // mindestens zwei Saiten (31d-4, S3 a); 84, 87, 93, 96, 105 vermerkt — dort schwebt das Modell nicht
    s.verstimmungCent = kurveAusPunkten({{33, 3.014}, {36, 1.926}, {39, 0.9654}, {42, 4.268}, {45, 3.919}, {48, 2.254},
                                         {51, 1.228}, {54, 2.851}, {57, 1.084}, {60, 0.8048}, {63, 0.8484}, {66, 1.394},
                                         {69, 1.422}, {72, 1.789}, {75, 0.858}, {78, 5.052}, {81, 3.76}, {90, 1.635},
                                         {99, 0.5929}, {102, 1.479}},
                                        Belegkraft::Gemessen, std::string(kSalamander));
    s.daempferS = kurveAusPunkten({{21, 0.8}, {90, 0.2}}, Belegkraft::Offen, "provisional (31a)");
    // vorlaeufig ohne zweite Ebene (31d-7 A): der Ton rechnet wie vorher, bis 31d-7 B γ und δ an Salamander setzt
    // (P4 a); h bleibt 0 bis UF8 b (P3 a, P3-1 a)
    s.polarisationWinkelGrad = kurveAusPunkten({{21, 0.0}, {108, 0.0}}, Belegkraft::Offen, "provisional: no second plane (31d-7 A)");
    s.polarisationCent = kurveAusPunkten({{21, 0.0}, {108, 0.0}}, Belegkraft::Offen, "provisional: no second plane (31d-7 A)");
    s.waagrechtVerhaeltnis = kurveAusPunkten({{21, 0.0}, {108, 0.0}}, Belegkraft::Offen, "provisional: 0 until UF8 b (P3-1 a)");
    // vorlaeufig gleicher Stoss (US4 a): der Ton rechnet wie vorher, bis 31d-7 B den Stoss setzt (US1 a)
    s.stossLeise = kurveAusPunkten({{21, 0.0}, {108, 0.0}}, Belegkraft::Offen, "provisional: even strike (US4 a)");
    s.stossLaut = kurveAusPunkten({{21, 0.0}, {108, 0.0}}, Belegkraft::Offen, "provisional: even strike (US4 a)");
    return s;
}

}  // namespace

const std::vector<KurvenAngabe>& kurvenAngaben()
{
    using L = Skala;
    static const std::vector<KurvenAngabe> angaben = {
        {&HammerSaite::inharmonizitaet, "inharmonicity", "Inharmonicity B", "String", "", L::Logarithmisch, 1e-6, 0.1},
        {&HammerSaite::saitenLaengeM, "stringLength", "String length", "String", "m", L::Logarithmisch, 0.02, 8.0},
        {&HammerSaite::saitenMasseGm, "stringMass", "String mass", "String", "g/m", L::Logarithmisch, 0.5, 500.0},
        {&HammerSaite::laengsVerhaeltnis, "longitudinalRatio", "Longitudinal ratio", "String", "", L::Linear, 5.0, 40.0},
        {&HammerSaite::chor, "strings", "Strings", "Choir", "", L::Linear, static_cast<double>(kChorMin), static_cast<double>(kChorMax)},
        {&HammerSaite::verstimmungCent, "detune", "Detune", "Choir", "cent", L::Linear, 0.0, 20.0},
        {&HammerSaite::polarisationWinkelGrad, "polarizationAngle", "Polarization angle", "Choir", "deg", L::Linear, 0.0, 90.0},
        {&HammerSaite::polarisationCent, "polarizationDetune", "Polarization detune", "Choir", "cent", L::Linear, 0.0, 20.0},
        {&HammerSaite::waagrechtVerhaeltnis, "horizontalRatio", "Horizontal ratio", "Choir", "", L::Linear, 0.0, 1.0},
        {&HammerSaite::abklingS, "decay", "Decay", "Decay", "s", L::Logarithmisch, 0.05, 120.0},
        {&HammerSaite::abklingSpaetS, "lateDecay", "Late decay", "Decay", "s", L::Logarithmisch, 0.05, 120.0},
        {&HammerSaite::abklingHoch, "highDecay", "High decay", "Decay", "", L::Logarithmisch, 0.001, 1.0},
        {&HammerSaite::stegWinkelGrad, "bridgeAngle", "Bridge angle", "Decay", "deg", L::Linear, 0.0, 10.0},
        {&HammerSaite::hammerMasseG, "hammerMass", "Hammer mass", "Hammer", "g", L::Linear, 1.0, 30.0},
        {&HammerSaite::hammerSteife, "hammerStiffness", "Hammer stiffness Q0", "Hammer", "N/mm^p", L::Logarithmisch, 1.0, 1e6},
        {&HammerSaite::hammerExponent, "hammerExponent", "Hammer exponent p", "Hammer", "", L::Linear, 1.0, 6.0},
        {&HammerSaite::hammerHysterese, "hammerHysteresis", "Hammer hysteresis", "Hammer", "", L::Linear, 0.0, 0.999},
        {&HammerSaite::hammerRelaxationUs, "hammerRelaxation", "Hammer relaxation", "Hammer", "us", L::Logarithmisch, 0.01, 1000.0},
        {&HammerSaite::anschlagStelle, "strikePosition", "Strike position", "Hammer", "", L::Linear, 0.02, 0.5},
        {&HammerSaite::stossLeise, "strikeUnevennessSoft", "Strike unevenness, soft", "Hammer", "", L::Linear, 0.0, 1.0},
        {&HammerSaite::stossLaut, "strikeUnevennessLoud", "Strike unevenness, loud", "Hammer", "", L::Linear, 0.0, 1.0},
        {&HammerSaite::daempferS, "damper", "Damper", "Damper", "s", L::Logarithmisch, 0.01, 10.0},
    };
    return angaben;
}

int chorAn(const HammerSaite& saite, int taste)
{
    const double wert = wertAn(saite.chor, taste, Skala::Linear);
    return std::clamp(static_cast<int>(std::lround(wert)), kChorMin, kChorMax);
}

const HammerSaite& hammerSaiteVorgabe()
{
    static const HammerSaite vorgabe = baueVorgabe();
    return vorgabe;
}

void richteHammerSaite(HammerSaite& saite)
{
    const HammerSaite& v = hammerSaiteVorgabe();
    for (const KurvenAngabe& a : kurvenAngaben()) richteKurve(saite.*a.feld, a.von, a.bis, v.*a.feld, kTasteMin, kTasteMax);
}

double rtHammerMasseG(int i)
{
    return (-6.2348e-5 * i + 0.0112) * 1000.0;
}

double rtHammerExponent(int i)
{
    return 2.4295e-4 * i * i - 0.007703 * i + 2.337;
}

double rtHammerSteifeNm(int i)
{
    return std::pow(10.0, 5.3097e-2 * i + 7.6425);
}

double rtHammerSteife(int i)
{
    // K_H·u^p mit u in m = K_H·10⁻³ᵖ·u^p mit u in mm
    return rtHammerSteifeNm(i) * std::pow(10.0, -3.0 * rtHammerExponent(i));
}

double stulovHammerMasseG(int n)
{
    return 11.074 - 0.074 * n + 1e-4 * n * n;
}

double stulovHammerExponent(int n)
{
    return 3.7 + 0.015 * n;
}

double stulovHammerSteife(int n)
{
    return 183.0 * std::exp(0.045 * n);
}

double stulovHammerHysterese(int n)
{
    return 0.9894 - 8.8e-5 * n * n;
}

double stulovHammerRelaxationUs(int n)
{
    return 2.72 - 0.02 * n + 9e-5 * n * n;
}

double rigaudB(int m, double sB, double yB)
{
    constexpr double kSt = 9.26e-2;
    constexpr double kYt = -13.64;
    return std::exp(sB * m + yB) + std::exp(kSt * m + kYt);
}

double hammerTempo(int stufe)
{
    return kHammerTempoMax * anschlagFaktor(stufe);
}

}  // namespace lumi::kern::hammer
