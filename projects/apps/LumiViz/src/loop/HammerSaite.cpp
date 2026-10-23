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

/// Die Tasten der Vorgabe: A0, A1, … A7 und C8.
constexpr int kVorgabeTasten[] = {21, 33, 45, 57, 69, 81, 93, 105, 108};

template <class Formel>
Tastenkurve kurveAus(Formel formel)
{
    Tastenkurve kurve;
    for (const int taste : kVorgabeTasten) kurve.punkte.push_back(TastenWert{taste, formel(taste)});
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
    // belegt (Befund S146): B nach Rigaud Gl. 5 — der Bassteg abgelesen, vorlaeufig
    s.inharmonizitaet = kurveAus([](int m) { return rigaudB(m, kRigaudBassSteigung, kRigaudBassAchse); });
    // belegt: der Hammer nach Stulov Gl. 9 (Taste n = MIDI − 20)
    s.hammerMasseG = kurveAus([](int m) { return stulovMasseG(m - 20); });
    s.hammerExponent = kurveAus([](int m) { return stulovExponent(m - 20); });
    s.hammerSteife = kurveAus([](int m) { return stulovSteife(m - 20); });
    // vorlaeufig, ohne Beleg — setzt die Messung an Salamander (31d)
    s.abklingS = kurveAusPunkten({{21, 25.0}, {33, 20.0}, {45, 15.0}, {57, 10.0}, {69, 6.0}, {81, 3.0}, {93, 1.5},
                                  {105, 0.7}, {108, 0.5}});
    s.abklingHoch = kurveAusPunkten({{21, 0.2}, {108, 0.2}});
    s.chor = kurveAusPunkten({{21, 1.0}, {30, 1.0}, {31, 2.0}, {48, 2.0}, {49, 3.0}, {108, 3.0}});
    s.verstimmungCent = kurveAusPunkten({{21, 1.0}, {108, 1.0}});
    s.anschlagStelle = kurveAusPunkten({{21, 0.125}, {108, 0.125}});
    s.daempferS = kurveAusPunkten({{21, 0.8}, {90, 0.2}});
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
    richteKurve(saite.abklingHoch, kGrenzeHoch.von, kGrenzeHoch.bis, v.abklingHoch);
    richteKurve(saite.chor, kGrenzeChor.von, kGrenzeChor.bis, v.chor);
    richteKurve(saite.verstimmungCent, kGrenzeCent.von, kGrenzeCent.bis, v.verstimmungCent);
    richteKurve(saite.hammerMasseG, kGrenzeMasse.von, kGrenzeMasse.bis, v.hammerMasseG);
    richteKurve(saite.hammerSteife, kGrenzeSteife.von, kGrenzeSteife.bis, v.hammerSteife);
    richteKurve(saite.hammerExponent, kGrenzeExponent.von, kGrenzeExponent.bis, v.hammerExponent);
    richteKurve(saite.anschlagStelle, kGrenzeStelle.von, kGrenzeStelle.bis, v.anschlagStelle);
    richteKurve(saite.daempferS, kGrenzeDaempfer.von, kGrenzeDaempfer.bis, v.daempferS);
    const double anteil = saite.resonanzboden.anteil;
    saite.resonanzboden.anteil = std::isfinite(anteil) ? std::clamp(anteil, 0.0, 1.0) : 0.0;
}

QJsonObject hammerSaiteAlsJson(const HammerSaite& s)
{
    return QJsonObject{{QStringLiteral("inharmonicity"), kurveAlsJson(s.inharmonizitaet)},
                       {QStringLiteral("decay"), kurveAlsJson(s.abklingS)},
                       {QStringLiteral("highDecay"), kurveAlsJson(s.abklingHoch)},
                       {QStringLiteral("strings"), kurveAlsJson(s.chor)},
                       {QStringLiteral("detune"), kurveAlsJson(s.verstimmungCent)},
                       {QStringLiteral("hammerMass"), kurveAlsJson(s.hammerMasseG)},
                       {QStringLiteral("hammerStiffness"), kurveAlsJson(s.hammerSteife)},
                       {QStringLiteral("hammerExponent"), kurveAlsJson(s.hammerExponent)},
                       {QStringLiteral("strikePosition"), kurveAlsJson(s.anschlagStelle)},
                       {QStringLiteral("damper"), kurveAlsJson(s.daempferS)},
                       {QStringLiteral("soundboard"),
                        QJsonObject{{QStringLiteral("amount"), s.resonanzboden.anteil}}}};
}

HammerSaite hammerSaiteAusJson(const QJsonObject& o)
{
    HammerSaite s;
    s.inharmonizitaet = kurveAusJson(o.value(QStringLiteral("inharmonicity")));
    s.abklingS = kurveAusJson(o.value(QStringLiteral("decay")));
    s.abklingHoch = kurveAusJson(o.value(QStringLiteral("highDecay")));
    s.chor = kurveAusJson(o.value(QStringLiteral("strings")));
    s.verstimmungCent = kurveAusJson(o.value(QStringLiteral("detune")));
    s.hammerMasseG = kurveAusJson(o.value(QStringLiteral("hammerMass")));
    s.hammerSteife = kurveAusJson(o.value(QStringLiteral("hammerStiffness")));
    s.hammerExponent = kurveAusJson(o.value(QStringLiteral("hammerExponent")));
    s.anschlagStelle = kurveAusJson(o.value(QStringLiteral("strikePosition")));
    s.daempferS = kurveAusJson(o.value(QStringLiteral("damper")));
    s.resonanzboden.anteil = o.value(QStringLiteral("soundboard")).toObject().value(QStringLiteral("amount")).toDouble(0.0);
    return s;
}

double stulovMasseG(int n)
{
    return 11.074 - 0.074 * n + 1e-4 * n * n;
}

double stulovExponent(int n)
{
    return 3.7 + 0.015 * n;
}

double stulovSteife(int n)
{
    return 183.0 * std::exp(0.045 * n);
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
