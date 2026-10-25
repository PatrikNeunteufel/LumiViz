/**
 * @file HammerSaite.cpp
 * @brief Loop: Kurven ueber die Tastatur, Vorgaben und Datei der Hammer-Saite,
 *        der Block *Keyboard* und die Stufen der Anschlagstaerke (Stufe 31a)
 */
#include "loop/HammerSaite.hpp"

#include <QJsonArray>

#include <algorithm>
#include <cmath>
#include <iterator>

namespace lumi::loop
{

namespace
{

/// Monoton kubisch zwischen den Punkten (HK1 a): die Steigungen nach Fritsch und
/// Butland, wie PCHIP — innen das gewichtete harmonische Mittel der Nachbarsehnen
/// (0 bei einem Wechsel der Richtung), am Rand die Formel aus drei Punkten, auf
/// die Richtung der ersten Sehne begrenzt. Mit zwei Punkten: die Gerade.
std::vector<double> steigungen(const std::vector<double>& x, const std::vector<double>& y)
{
    const std::size_t n = x.size();
    std::vector<double> m(n, 0.0);
    if (n < 2) return m;
    std::vector<double> h(n - 1);
    std::vector<double> d(n - 1);
    for (std::size_t k = 0; k + 1 < n; ++k)
    {
        h[k] = x[k + 1] - x[k];
        d[k] = (y[k + 1] - y[k]) / h[k];
    }
    if (n == 2)
    {
        m[0] = m[1] = d[0];
        return m;
    }
    for (std::size_t k = 1; k + 1 < n; ++k)
    {
        if (d[k - 1] * d[k] <= 0.0) continue;
        const double w1 = 2.0 * h[k] + h[k - 1];
        const double w2 = h[k] + 2.0 * h[k - 1];
        m[k] = (w1 + w2) / (w1 / d[k - 1] + w2 / d[k]);
    }
    const auto rand = [](double h0, double h1, double d0, double d1) {
        double s = ((2.0 * h0 + h1) * d0 - h0 * d1) / (h0 + h1);
        if (s * d0 <= 0.0)
            s = 0.0;
        else if (d0 * d1 < 0.0 && std::abs(s) > 3.0 * std::abs(d0))
            s = 3.0 * d0;
        return s;
    };
    m[0] = rand(h[0], h[1], d[0], d[1]);
    m[n - 1] = rand(h[n - 2], h[n - 3], d[n - 2], d[n - 3]);
    return m;
}

double inSkala(double wert, KurvenSkala skala)
{
    return skala == KurvenSkala::Logarithmisch ? std::log(std::max(wert, 1e-300)) : wert;
}

double ausSkala(double wert, KurvenSkala skala)
{
    return skala == KurvenSkala::Logarithmisch ? std::exp(wert) : wert;
}

/// Die Paare einer Liste `[[taste, wert], …]`.
std::vector<TastenWert> paareAus(const QJsonValue& wert)
{
    std::vector<TastenWert> paare;
    for (const QJsonValue& eintrag : wert.toArray())
    {
        const QJsonArray paar = eintrag.toArray();
        if (paar.size() != 2 || !paar[0].isDouble() || !paar[1].isDouble()) continue;
        paare.push_back(TastenWert{paar[0].toInt(), paar[1].toDouble()});
    }
    return paare;
}

QJsonArray paareAlsJson(const std::vector<TastenWert>& paare)
{
    QJsonArray liste;
    for (const TastenWert& p : paare) liste.append(QJsonArray{p.taste, p.wert});
    return liste;
}

void richtePaare(std::vector<TastenWert>& paare, double von, double bis)
{
    std::vector<TastenWert> gut;
    for (TastenWert p : paare)
    {
        if (!std::isfinite(p.wert)) continue;
        p.taste = std::clamp(p.taste, kTasteMin, kTasteMax);
        p.wert = std::clamp(p.wert, von, bis);
        gut.push_back(p);
    }
    // je Taste der erste Eintrag
    std::stable_sort(gut.begin(), gut.end(), [](const TastenWert& a, const TastenWert& b) { return a.taste < b.taste; });
    gut.erase(std::unique(gut.begin(), gut.end(), [](const TastenWert& a, const TastenWert& b) { return a.taste == b.taste; }),
              gut.end());
    paare = std::move(gut);
}

/// Die Grenzen je Groesse (wie in HammerSaite.hpp beschrieben).
struct Grenzen
{
    double von;
    double bis;
};
constexpr Grenzen kGrenzeB{1e-6, 0.1};
constexpr Grenzen kGrenzeAbkling{0.05, 120.0};
constexpr Grenzen kGrenzeHoch{0.001, 1.0};
constexpr Grenzen kGrenzeChor{static_cast<double>(kChorMin), static_cast<double>(kChorMax)};
constexpr Grenzen kGrenzeCent{0.0, 20.0};
constexpr Grenzen kGrenzeMasse{1.0, 30.0};
constexpr Grenzen kGrenzeSteife{1.0, 1e6};
constexpr Grenzen kGrenzeExponent{1.0, 6.0};
constexpr Grenzen kGrenzeStelle{0.02, 0.5};
constexpr Grenzen kGrenzeDaempfer{0.01, 10.0};
constexpr Grenzen kGrenzeLaenge{0.02, 8.0};
constexpr Grenzen kGrenzeMasseJeM{0.5, 500.0};
constexpr Grenzen kGrenzeLaengs{5.0, 40.0};
constexpr Grenzen kGrenzeWinkel{0.0, 10.0};
constexpr Grenzen kGrenzePolarisationWinkel{0.0, 90.0};
constexpr Grenzen kGrenzePolarisationCent{0.0, 20.0};
constexpr Grenzen kGrenzeVerhaeltnis{0.0, 1.0};

/// Die Tasten der Vorgabe: A0, A1, … A7 und C8.
constexpr int kVorgabeTasten[] = {21, 33, 45, 57, 69, 81, 93, 105, 108};

/// Die Tasten der Hammer-Vorgabe: alle sechs Tasten und C8 — Q0 ist bei RT-0425 in ln
/// quadratisch in der Taste; an den A-Tasten allein laege die Kurve dazwischen bis 4 % daneben.
constexpr int kHammerTasten[] = {21, 27, 33, 39, 45, 51, 57, 63, 69, 75, 81, 87, 93, 99, 105, 108};

template <class Formel, std::size_t N>
Tastenkurve kurveAus(const int (&tasten)[N], Formel formel)
{
    Tastenkurve kurve;
    for (const int taste : tasten) kurve.punkte.push_back(TastenWert{taste, formel(taste)});
    return kurve;
}

Tastenkurve kurveAusPunkten(std::initializer_list<TastenWert> punkte)
{
    Tastenkurve kurve;
    kurve.punkte.assign(punkte.begin(), punkte.end());
    return kurve;
}

HammerSaite baueVorgabe()
{
    HammerSaite s;
    // B: im Bass bis E2 gemessen an Salamander (Yamaha C5), gesetzt durch das Modell hindurch (31d-4, S2 a);
    // darueber Rigaud Gl. 5 an den Tasten der Vorgabe (UF5 a: B im Diskant bleibt bei Rigaud)
    s.inharmonizitaet = kurveAusPunkten({{21, 2.177e-4}, {24, 1.593e-4}, {27, 1.278e-4}, {30, 8.824e-5}, {33, 8.795e-5},
                                         {36, 8.776e-5}, {39, 7.436e-5}});
    for (const int m : kVorgabeTasten)
        if (m > 40) s.inharmonizitaet.punkte.push_back(TastenWert{m, rigaudB(m, kRigaudBassSteigung, kRigaudBassAchse)});
    // belegt (HT2 a): der Hammer nach RT-0425 Gl. 2–4 (Taste i = MIDI − 20)
    s.hammerMasseG = kurveAus(kHammerTasten, [](int m) { return rtHammerMasseG(m - 20); });
    s.hammerExponent = kurveAus(kHammerTasten, [](int m) { return rtHammerExponent(m - 20); });
    s.hammerSteife = kurveAus(kHammerTasten, [](int m) { return rtHammerSteife(m - 20); });
    // belegt (HT1 a): RT-0425 Anhang A, C1 … B7 — µ = ρπd²/4, f_l0/f0 = √(EA/T0) mit
    // E = 2,02·10¹¹ Pa; die Tasten so gewaehlt, dass die Kurve jede Taste der Tabelle auf
    // 2 % trifft (E2 | F2: der Sprung am Bassteg)
    s.saitenLaengeM = kurveAusPunkten({{24, 2.007}, {31, 1.842}, {37, 1.548}, {39, 1.442}, {40, 1.378}, {41, 1.837},
                                       {42, 1.757}, {45, 1.482}, {52, 1.013}, {74, 0.308}, {107, 0.052}});
    s.saitenMasseGm = kurveAusPunkten({{24, 99.41}, {28, 66.53}, {33, 31.57}, {35, 23.63}, {39, 15.32}, {40, 14.30},
                                       {41, 7.729}, {54, 6.733}, {70, 5.459}, {82, 4.972}, {94, 4.392}, {107, 3.404}});
    s.laengsVerhaeltnis =
        kurveAusPunkten({{24, 14.21}, {40, 13.71}, {41, 15.74}, {55, 14.98}, {81, 13.59}, {107, 12.20}});
    // gemessen an Salamander (Yamaha C5) an den Phantomteiltoenen im Bass bis A2, darueber der letzte Wert
    // (31d-4, S2 a; MIDI 33 an der Grenze 10°, trifft knapp — Auffaelligkeit fuer 31d Nacharbeit)
    s.stegWinkelGrad = kurveAusPunkten({{21, 1.02}, {24, 0.6763}, {27, 0.6648}, {30, 0.9756}, {33, 10.0}, {36, 1.263},
                                        {39, 2.0}, {42, 3.532}, {45, 4.432}, {108, 4.432}});
    // x_H/L liegt dort zwischen 0,113 und 0,127, fast ueberall bei 0,120
    s.anschlagStelle = kurveAusPunkten({{21, 0.12}, {108, 0.12}});
    // gemessen an Salamander (Yamaha C5): T60 bei f1 aus dem Ausgleich ueber die Teiltoene, gesetzt durch das
    // Modell hindurch (31d-4, S1 a, S2 a, S3 a; drei Durchgaenge, 18 Runden, so gemessen); 105 ohne Treffer, vermerkt
    s.abklingS = kurveAusPunkten({{21, 15.99}, {24, 13.71}, {27, 18.04}, {30, 18.77}, {33, 11.99}, {36, 14.86}, {39, 12.02},
                                  {42, 7.494}, {45, 8.793}, {48, 5.843}, {51, 4.726}, {54, 3.92}, {57, 3.77}, {60, 5.294},
                                  {63, 2.053}, {66, 3.761}, {69, 3.713}, {72, 2.06}, {75, 7.973}, {78, 2.934}, {81, 1.621},
                                  {84, 1.075}, {87, 11.47}, {90, 4.109}, {93, 2.199}, {96, 0.5089}, {99, 1.507}, {102, 1.837}});
    // vorlaeufig dreimal Decay (HT3 a) — das Modell zeigt keine zweite Stufe wie Salamander (S3 a: Kipppunkt fuer
    // 31d Nacharbeit)
    s.abklingSpaetS = kurveAusPunkten({{21, 47.96}, {24, 41.14}, {27, 54.11}, {30, 56.32}, {33, 35.98}, {36, 44.57},
                                       {39, 36.07}, {42, 22.48}, {45, 26.38}, {48, 17.53}, {51, 14.18}, {54, 11.76},
                                       {57, 11.31}, {60, 15.88}, {63, 6.16}, {66, 11.28}, {69, 11.14}, {72, 6.181},
                                       {75, 23.92}, {78, 8.801}, {81, 4.862}, {84, 3.225}, {87, 34.4}, {90, 12.33},
                                       {93, 6.596}, {96, 1.527}, {99, 4.522}, {102, 5.512}});
    // gemessen an Salamander (Yamaha C5) aus demselben Ausgleich, durch das Modell hindurch (31d-4); 96 und 105 vermerkt
    s.abklingHoch = kurveAusPunkten({{21, 0.9791}, {24, 0.9888}, {27, 0.9689}, {30, 0.9549}, {33, 0.9718}, {36, 0.9409},
                                     {39, 0.9154}, {42, 0.9577}, {45, 0.9504}, {48, 0.9342}, {51, 0.8951}, {54, 0.7855},
                                     {57, 0.8781}, {60, 0.6443}, {63, 0.904}, {66, 0.6689}, {69, 0.4688}, {72, 0.524},
                                     {75, 0.1524}, {78, 0.2841}, {81, 0.3365}, {84, 0.3355}, {87, 0.01508}, {90, 0.03498},
                                     {93, 0.02882}, {99, 0.02764}, {102, 0.01849}});
    s.chor = kurveAusPunkten({{21, 1.0}, {30, 1.0}, {31, 2.0}, {48, 2.0}, {49, 3.0}, {108, 3.0}});
    // gemessen an Salamander (Yamaha C5) an der Schwebung der Teiltoene 1–3, durch das Modell hindurch, nur bei
    // mindestens zwei Saiten (31d-4, S3 a); 84, 87, 93, 96, 105 vermerkt — dort schwebt das Modell nicht
    s.verstimmungCent = kurveAusPunkten({{33, 3.014}, {36, 1.926}, {39, 0.9654}, {42, 4.268}, {45, 3.919}, {48, 2.254},
                                         {51, 1.228}, {54, 2.851}, {57, 1.084}, {60, 0.8048}, {63, 0.8484}, {66, 1.394},
                                         {69, 1.422}, {72, 1.789}, {75, 0.858}, {78, 5.052}, {81, 3.76}, {90, 1.635},
                                         {99, 0.5929}, {102, 1.479}});
    s.daempferS = kurveAusPunkten({{21, 0.8}, {90, 0.2}});
    // vorlaeufig ohne zweite Ebene (31d-7 A): der Ton rechnet wie vorher, bis 31d-7 B γ und δ an
    // Salamander setzt (P4 a); h bleibt 0 bis UF8 b (P3 a, P3-1 a)
    s.polarisationWinkelGrad = kurveAusPunkten({{21, 0.0}, {108, 0.0}});
    s.polarisationCent = kurveAusPunkten({{21, 0.0}, {108, 0.0}});
    s.waagrechtVerhaeltnis = kurveAusPunkten({{21, 0.0}, {108, 0.0}});
    return s;
}

}  // namespace

double wertAn(const Tastenkurve& kurve, int taste, KurvenSkala skala)
{
    for (const TastenWert& a : kurve.abweichungen)
        if (a.taste == taste) return a.wert;
    const auto& p = kurve.punkte;
    if (p.empty()) return 0.0;
    if (taste <= p.front().taste) return p.front().wert;
    if (taste >= p.back().taste) return p.back().wert;
    std::vector<double> x;
    std::vector<double> y;
    x.reserve(p.size());
    y.reserve(p.size());
    for (const TastenWert& w : p)
    {
        x.push_back(w.taste);
        y.push_back(inSkala(w.wert, skala));
    }
    const std::vector<double> m = steigungen(x, y);
    const auto oben = std::upper_bound(x.begin(), x.end(), static_cast<double>(taste));
    const auto k = static_cast<std::size_t>(std::distance(x.begin(), oben)) - 1;
    const double h = x[k + 1] - x[k];
    const double t = (taste - x[k]) / h;
    const double t2 = t * t;
    const double t3 = t2 * t;
    const double wert = (2 * t3 - 3 * t2 + 1) * y[k] + (t3 - 2 * t2 + t) * h * m[k] + (-2 * t3 + 3 * t2) * y[k + 1] +
                        (t3 - t2) * h * m[k + 1];
    return ausSkala(wert, skala);
}

void richteKurve(Tastenkurve& kurve, double von, double bis, const Tastenkurve& ersatz)
{
    richtePaare(kurve.punkte, von, bis);
    richtePaare(kurve.abweichungen, von, bis);
    if (kurve.punkte.empty()) kurve.punkte = ersatz.punkte;
}

QJsonObject kurveAlsJson(const Tastenkurve& kurve)
{
    return QJsonObject{{QStringLiteral("points"), paareAlsJson(kurve.punkte)},
                       {QStringLiteral("overrides"), paareAlsJson(kurve.abweichungen)}};
}

Tastenkurve kurveAusJson(const QJsonValue& wert)
{
    const QJsonObject o = wert.toObject();
    Tastenkurve kurve;
    kurve.punkte = paareAus(o.value(QStringLiteral("points")));
    kurve.abweichungen = paareAus(o.value(QStringLiteral("overrides")));
    return kurve;
}

int chorAn(const HammerSaite& saite, int taste)
{
    const double wert = wertAn(saite.chor, taste, KurvenSkala::Linear);
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
    richteKurve(saite.inharmonizitaet, kGrenzeB.von, kGrenzeB.bis, v.inharmonizitaet);
    richteKurve(saite.abklingS, kGrenzeAbkling.von, kGrenzeAbkling.bis, v.abklingS);
    richteKurve(saite.abklingSpaetS, kGrenzeAbkling.von, kGrenzeAbkling.bis, v.abklingSpaetS);
    richteKurve(saite.abklingHoch, kGrenzeHoch.von, kGrenzeHoch.bis, v.abklingHoch);
    richteKurve(saite.chor, kGrenzeChor.von, kGrenzeChor.bis, v.chor);
    richteKurve(saite.verstimmungCent, kGrenzeCent.von, kGrenzeCent.bis, v.verstimmungCent);
    richteKurve(saite.hammerMasseG, kGrenzeMasse.von, kGrenzeMasse.bis, v.hammerMasseG);
    richteKurve(saite.hammerSteife, kGrenzeSteife.von, kGrenzeSteife.bis, v.hammerSteife);
    richteKurve(saite.hammerExponent, kGrenzeExponent.von, kGrenzeExponent.bis, v.hammerExponent);
    richteKurve(saite.anschlagStelle, kGrenzeStelle.von, kGrenzeStelle.bis, v.anschlagStelle);
    richteKurve(saite.daempferS, kGrenzeDaempfer.von, kGrenzeDaempfer.bis, v.daempferS);
    richteKurve(saite.saitenLaengeM, kGrenzeLaenge.von, kGrenzeLaenge.bis, v.saitenLaengeM);
    richteKurve(saite.saitenMasseGm, kGrenzeMasseJeM.von, kGrenzeMasseJeM.bis, v.saitenMasseGm);
    richteKurve(saite.laengsVerhaeltnis, kGrenzeLaengs.von, kGrenzeLaengs.bis, v.laengsVerhaeltnis);
    richteKurve(saite.stegWinkelGrad, kGrenzeWinkel.von, kGrenzeWinkel.bis, v.stegWinkelGrad);
    richteKurve(saite.polarisationWinkelGrad, kGrenzePolarisationWinkel.von, kGrenzePolarisationWinkel.bis,
                v.polarisationWinkelGrad);
    richteKurve(saite.polarisationCent, kGrenzePolarisationCent.von, kGrenzePolarisationCent.bis, v.polarisationCent);
    richteKurve(saite.waagrechtVerhaeltnis, kGrenzeVerhaeltnis.von, kGrenzeVerhaeltnis.bis, v.waagrechtVerhaeltnis);
    saite.resonanzboden.koerper = saite.resonanzboden.koerper.trimmed();
}

QJsonObject hammerSaiteAlsJson(const HammerSaite& s)
{
    return QJsonObject{{QStringLiteral("inharmonicity"), kurveAlsJson(s.inharmonizitaet)},
                       {QStringLiteral("decay"), kurveAlsJson(s.abklingS)},
                       {QStringLiteral("lateDecay"), kurveAlsJson(s.abklingSpaetS)},
                       {QStringLiteral("highDecay"), kurveAlsJson(s.abklingHoch)},
                       {QStringLiteral("strings"), kurveAlsJson(s.chor)},
                       {QStringLiteral("detune"), kurveAlsJson(s.verstimmungCent)},
                       {QStringLiteral("hammerMass"), kurveAlsJson(s.hammerMasseG)},
                       {QStringLiteral("hammerStiffness"), kurveAlsJson(s.hammerSteife)},
                       {QStringLiteral("hammerExponent"), kurveAlsJson(s.hammerExponent)},
                       {QStringLiteral("strikePosition"), kurveAlsJson(s.anschlagStelle)},
                       {QStringLiteral("damper"), kurveAlsJson(s.daempferS)},
                       {QStringLiteral("stringLength"), kurveAlsJson(s.saitenLaengeM)},
                       {QStringLiteral("stringMass"), kurveAlsJson(s.saitenMasseGm)},
                       {QStringLiteral("longitudinalRatio"), kurveAlsJson(s.laengsVerhaeltnis)},
                       {QStringLiteral("bridgeAngle"), kurveAlsJson(s.stegWinkelGrad)},
                       // seit Fassung 18 (P7 a): die zweite Schwingungsebene, immer geschrieben
                       {QStringLiteral("polarizationAngle"), kurveAlsJson(s.polarisationWinkelGrad)},
                       {QStringLiteral("polarizationDetune"), kurveAlsJson(s.polarisationCent)},
                       {QStringLiteral("horizontalRatio"), kurveAlsJson(s.waagrechtVerhaeltnis)},
                       // seit Fassung 17 (SB1 a): nur der Verweis auf den Koerper, immer geschrieben, leer = ohne Boden
                       {QStringLiteral("soundboard"),
                        QJsonObject{{QStringLiteral("body"), s.resonanzboden.koerper}}}};
}

HammerSaite hammerSaiteAusJson(const QJsonObject& o)
{
    HammerSaite s;
    s.inharmonizitaet = kurveAusJson(o.value(QStringLiteral("inharmonicity")));
    s.abklingS = kurveAusJson(o.value(QStringLiteral("decay")));
    s.abklingSpaetS = kurveAusJson(o.value(QStringLiteral("lateDecay")));
    s.abklingHoch = kurveAusJson(o.value(QStringLiteral("highDecay")));
    s.chor = kurveAusJson(o.value(QStringLiteral("strings")));
    s.verstimmungCent = kurveAusJson(o.value(QStringLiteral("detune")));
    s.hammerMasseG = kurveAusJson(o.value(QStringLiteral("hammerMass")));
    s.hammerSteife = kurveAusJson(o.value(QStringLiteral("hammerStiffness")));
    s.hammerExponent = kurveAusJson(o.value(QStringLiteral("hammerExponent")));
    s.anschlagStelle = kurveAusJson(o.value(QStringLiteral("strikePosition")));
    s.daempferS = kurveAusJson(o.value(QStringLiteral("damper")));
    s.saitenLaengeM = kurveAusJson(o.value(QStringLiteral("stringLength")));
    s.saitenMasseGm = kurveAusJson(o.value(QStringLiteral("stringMass")));
    s.laengsVerhaeltnis = kurveAusJson(o.value(QStringLiteral("longitudinalRatio")));
    s.stegWinkelGrad = kurveAusJson(o.value(QStringLiteral("bridgeAngle")));
    s.polarisationWinkelGrad = kurveAusJson(o.value(QStringLiteral("polarizationAngle")));
    s.polarisationCent = kurveAusJson(o.value(QStringLiteral("polarizationDetune")));
    s.waagrechtVerhaeltnis = kurveAusJson(o.value(QStringLiteral("horizontalRatio")));
    // Fassung 16 trug `amount` (wirkte nie) — es wird ueberlesen (SB1 a)
    s.resonanzboden.koerper = o.value(QStringLiteral("soundboard")).toObject().value(QStringLiteral("body")).toString();
    return s;
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

double rigaudB(int m, double sB, double yB)
{
    constexpr double kSt = 9.26e-2;
    constexpr double kYt = -13.64;
    return std::exp(sB * m + yB) + std::exp(kSt * m + kYt);
}

bool pedalErlaubt(PedalPlatz platz, PedalArt art)
{
    if (art == PedalArt::Keins) return true;
    switch (platz)
    {
        case PedalPlatz::Links: return art == PedalArt::UnaCorda || art == PedalArt::Leise;
        case PedalPlatz::Mitte: return art == PedalArt::Sostenuto || art == PedalArt::Moderator;
        case PedalPlatz::Rechts: return art == PedalArt::Sustain;
    }
    return false;
}

QLatin1String pedalName(PedalArt art)
{
    switch (art)
    {
        case PedalArt::UnaCorda: return QLatin1String("unaCorda");
        case PedalArt::Leise: return QLatin1String("soft");
        case PedalArt::Sostenuto: return QLatin1String("sostenuto");
        case PedalArt::Moderator: return QLatin1String("practice");
        case PedalArt::Sustain: return QLatin1String("sustain");
        case PedalArt::Keins: break;
    }
    return QLatin1String("none");
}

PedalArt pedalAusName(const QString& name)
{
    for (const PedalArt art : {PedalArt::UnaCorda, PedalArt::Leise, PedalArt::Sostenuto, PedalArt::Moderator, PedalArt::Sustain})
        if (name == pedalName(art)) return art;
    return PedalArt::Keins;
}

void richteTastatur(Tastatur& t)
{
    t.tiefste = std::clamp(t.tiefste, kTasteMin, kTasteMax);
    t.hoechste = std::clamp(t.hoechste, t.tiefste, kTasteMax);
    t.ungedaempftAb = std::clamp(t.ungedaempftAb, t.tiefste, t.hoechste + 1);
    if (!pedalErlaubt(PedalPlatz::Links, t.links)) t.links = PedalArt::Keins;
    if (!pedalErlaubt(PedalPlatz::Mitte, t.mitte)) t.mitte = PedalArt::Keins;
    if (!pedalErlaubt(PedalPlatz::Rechts, t.rechts)) t.rechts = PedalArt::Keins;
}

QJsonObject tastaturAlsJson(const Tastatur& t)
{
    return QJsonObject{{QStringLiteral("range"), QJsonArray{t.tiefste, t.hoechste}},
                       {QStringLiteral("undampedFrom"), t.ungedaempftAb},
                       {QStringLiteral("pedals"), QJsonObject{{QStringLiteral("left"), pedalName(t.links)},
                                                              {QStringLiteral("middle"), pedalName(t.mitte)},
                                                              {QStringLiteral("right"), pedalName(t.rechts)}}}};
}

Tastatur tastaturAusJson(const QJsonObject& o)
{
    Tastatur t;
    const QJsonArray umfang = o.value(QStringLiteral("range")).toArray();
    if (umfang.size() == 2)
    {
        t.tiefste = umfang[0].toInt(t.tiefste);
        t.hoechste = umfang[1].toInt(t.hoechste);
    }
    t.ungedaempftAb = o.value(QStringLiteral("undampedFrom")).toInt(t.ungedaempftAb);
    const QJsonObject pedale = o.value(QStringLiteral("pedals")).toObject();
    // fehlt der Eintrag eines Platzes, gilt die Vorgabe; ein unbekannter heisst keins
    const auto pedal = [&pedale](const char* platz, PedalArt vorgabe) {
        const QJsonValue wert = pedale.value(QLatin1String(platz));
        return wert.isString() ? pedalAusName(wert.toString()) : vorgabe;
    };
    t.links = pedal("left", t.links);
    t.mitte = pedal("middle", t.mitte);
    t.rechts = pedal("right", t.rechts);
    richteTastatur(t);
    return t;
}

int anschlagStufe(double laut)
{
    if (!(laut > 0.0)) return kAnschlagStufeMin;
    const double db = 20.0 * std::log10(laut);
    const auto stufe = static_cast<int>(std::lround(db / kAnschlagSchrittDb));
    return std::clamp(stufe, kAnschlagStufeMin, 0);
}

double hammerTempo(int stufe)
{
    return kHammerTempoMax * std::pow(10.0, std::clamp(stufe, kAnschlagStufeMin, 0) * kAnschlagSchrittDb / 20.0);
}

double anschlagRest(double laut, int stufe)
{
    return laut / std::pow(10.0, std::clamp(stufe, kAnschlagStufeMin, 0) * kAnschlagSchrittDb / 20.0);
}

}  // namespace lumi::loop
