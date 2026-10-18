/**
 * @file test_ShapeTon.cpp
 * @brief Klang aus einer Form, Stufe 19c: die Form als Schwingung einer Schicht
 *        (shape/Tonform.hpp, loop/Instrument.hpp; docs/audio/Shape_Konzept.md
 *        §5, §9, §12; Entscheide Patrik S121: T1 a, T2 a)
 *
 * Ohne Ton und ohne Engine: Toene werden gerechnet und an ihren Teiltoenen
 * gemessen. Die mitgelieferten Formen liest der Test aus `asset/contours` (vom
 * Arbeitsordner aus — die Test-Exe aus dem Projekt-Root starten).
 * Der letzte Fall misst nur und meldet Zahlen: die Rechenzeit und die Kante des
 * Sprungs bei mehreren Konturen. `LUMIVIZ_19C_WAV=<Ordner>` legt dort Hoerproben
 * ab.
 */

#include <doctest.h>

#include "loop/Aufnahme.hpp"
#include "loop/Instrument.hpp"
#include "shape/Kontur.hpp"
#include "shape/Tonform.hpp"

#include <QDir>
#include <QJsonArray>
#include <QJsonObject>
#include <QPainterPath>
#include <QString>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <complex>
#include <functional>
#include <string>
#include <vector>

using namespace lumi::loop;
namespace shape = lumi::shape;

namespace
{

constexpr double kPi = 3.14159265358979323846;
constexpr int kRate = 44100;
constexpr double kPegel = 0.5;  ///< die Luft nach oben jeder gerechneten Schwingung (Instrument.cpp)
constexpr int kA2 = 45;         ///< 110 Hz

/// Ein Instrument mit einer Shape-Schicht, die sofort voll da ist und so bleibt.
Instrument formInstrument(const char* form, const std::function<void(Schicht&)>& aendere = {})
{
    Instrument instrument;
    Schicht& s = instrument.schichten[0];
    s.schwingung = Schwingung::Form;
    s.form = QString::fromLatin1(form);
    s.anstiegMs = 0.0;
    s.halten = 1.0;
    s.ausklangMs = 5.0;
    if (aendere) aendere(s);
    return instrument;
}

std::vector<float> ton(const Instrument& instrument, int hoehe = kA2, double dauerMs = 600.0)
{
    return rendereTon(instrument, hoehe, dauerMs, kRate).daten;
}

/// Ein Fenster aus ganzen Schwingungen, ab 100 ms: [von, bis) in Abtastwerten.
struct Fenster
{
    std::int64_t von = 0;
    std::int64_t bis = 0;
};
Fenster fenster(double hz, int schwingungen = 40, double abMs = 100.0)
{
    Fenster f;
    f.von = std::llround(abMs * kRate / 1000.0);
    f.bis = f.von + std::llround(schwingungen * kRate / hz);
    return f;
}

/// Die Staerke des k-ten Teiltons einer Seite im Fenster.
double staerke(const std::vector<float>& daten, int seite, double hz, int k, const Fenster& f)
{
    REQUIRE(static_cast<std::int64_t>(daten.size() / 2) >= f.bis);
    std::complex<double> summe{0.0, 0.0};
    for (std::int64_t t = f.von; t < f.bis; ++t)
        summe += static_cast<double>(daten[static_cast<std::size_t>(2 * t + seite)]) *
                 std::polar(1.0, -2.0 * kPi * k * hz * static_cast<double>(t) / kRate);
    return 2.0 * std::abs(summe) / static_cast<double>(f.bis - f.von);
}

double spitze(const std::vector<float>& daten, const Fenster& f, int seite = 0)
{
    double groesste = 0.0;
    for (std::int64_t t = f.von; t < f.bis; ++t)
        groesste = std::max(groesste, std::abs(static_cast<double>(daten[static_cast<std::size_t>(2 * t + seite)])));
    return groesste;
}

double groessterUnterschied(const std::vector<float>& a, const std::vector<float>& b)
{
    REQUIRE(a.size() == b.size());
    double groesster = 0.0;
    for (std::size_t i = 0; i < a.size(); ++i) groesster = std::max(groesster, std::abs(static_cast<double>(a[i]) - b[i]));
    return groesster;
}

QPainterPath quadrat(double links, double oben, double kante)
{
    QPainterPath pfad;
    pfad.moveTo(links, oben);
    pfad.lineTo(links + kante, oben);
    pfad.lineTo(links + kante, oben + kante);
    pfad.lineTo(links, oben + kante);
    pfad.closeSubpath();
    return pfad;
}

/// Eine feste Rechenarbeit, gegen die sich eine Messung vergleichen laesst (S106).
double taktprobeMs()
{
    const auto start = std::chrono::steady_clock::now();
    volatile double summe = 0.0;
    for (int i = 1; i <= 4'000'000; ++i) summe = summe + std::sqrt(static_cast<double>(i));
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
}

}  // namespace

TEST_CASE("Shape Ton: ein Kreis ist ein reiner Sinus — mit Stereo XY links und rechts um ein Viertel versetzt")
{
    const double hz = tonHz(kA2);
    const Fenster f = fenster(hz);
    const std::vector<float> mono = ton(formInstrument("circle"));
    REQUIRE(!mono.empty());
    CHECK(staerke(mono, 0, hz, 1, f) == doctest::Approx(kPegel).epsilon(0.01));
    for (const int k : {2, 3, 4, 5, 8, 16}) CHECK(staerke(mono, 0, hz, k, f) < 0.002);
    // ohne Stereo XY sind die Seiten gleich
    for (std::size_t i = 0; i + 1 < mono.size(); i += 2) REQUIRE(mono[i] == mono[i + 1]);

    const std::vector<float> xy = ton(formInstrument("circle", [](Schicht& s) { s.stereoXY = true; }));
    CHECK(staerke(xy, 0, hz, 1, f) == doctest::Approx(kPegel).epsilon(0.01));
    CHECK(staerke(xy, 1, hz, 1, f) == doctest::Approx(kPegel).epsilon(0.01));
    // links Cosinus, rechts Sinus: die Summe der Quadrate steht
    for (std::int64_t t = f.von; t < f.bis; t += 7)
    {
        const double l = xy[static_cast<std::size_t>(2 * t)];
        const double r = xy[static_cast<std::size_t>(2 * t + 1)];
        REQUIRE(l * l + r * r == doctest::Approx(kPegel * kPegel).epsilon(0.02));
    }
}

TEST_CASE("Shape Ton: ueber Harmonics und ueber der Bandgrenze liegt nichts, kein Gleichanteil")
{
    const double hz = tonHz(kA2);
    const Fenster f = fenster(hz);
    const std::vector<float> acht = ton(formInstrument("square", [](Schicht& s) { s.harmonische = 8.0; }));
    CHECK(staerke(acht, 0, hz, 1, f) > 0.2);
    CHECK(staerke(acht, 0, hz, 7, f) > 0.002);
    for (int k = 9; k <= 40; ++k)
    {
        CAPTURE(k);
        CHECK(staerke(acht, 0, hz, k, f) < 0.002);
    }
    double summe = 0.0;
    for (std::int64_t t = f.von; t < f.bis; ++t) summe += acht[static_cast<std::size_t>(2 * t)];
    CHECK(std::abs(summe / static_cast<double>(f.bis - f.von)) < 0.001);

    // eine hohe Note bekommt von selbst weniger: bei C7 (2093 Hz) traegt die Rate neun Teiltoene
    const int c7 = 96;
    const double hoch = tonHz(c7);
    const Fenster fHoch = fenster(hoch, 400);
    const std::vector<float> viele = ton(formInstrument("square", [](Schicht& s) { s.harmonische = 64.0; }), c7);
    CHECK(staerke(viele, 0, hoch, 1, fHoch) > 0.2);
    CHECK(staerke(viele, 0, hoch, 7, fHoch) > 0.002);
    CHECK(staerke(viele, 0, hoch, 10, fHoch) < 0.002);  // 20,9 kHz: waere noch darstellbar, bleibt aber weg
}

TEST_CASE("Shape Ton: Angle dreht die Form — Start und Reverse aendern die Staerke der Teiltoene nicht")
{
    const double hz = tonHz(kA2);
    const Fenster f = fenster(hz);
    const auto herz = [](const std::function<void(Schicht&)>& aendere) {
        return ton(formInstrument("heart", [&](Schicht& s) {
            s.harmonische = 16.0;
            aendere(s);
        }));
    };
    const std::vector<float> null = herz([](Schicht&) {});
    // 180 Grad: dasselbe mit umgekehrtem Vorzeichen
    const std::vector<float> halb = herz([](Schicht& s) { s.winkelGrad = 180.0; });
    REQUIRE(halb.size() == null.size());
    for (std::size_t i = 0; i < null.size(); ++i) REQUIRE(halb[i] == doctest::Approx(-null[i]).epsilon(1.0e-4).scale(1.0));
    // 90 Grad ohne Stereo XY ist die y-Bewegung — die rechte Seite von Stereo XY bei 0 Grad (T2 a)
    const std::vector<float> viertel = herz([](Schicht& s) { s.winkelGrad = 90.0; });
    const std::vector<float> xy = herz([](Schicht& s) { s.stereoXY = true; });
    for (std::size_t i = 0; i + 1 < xy.size(); i += 2)
    {
        REQUIRE(xy[i] == doctest::Approx(null[i]).epsilon(1.0e-4).scale(1.0));
        REQUIRE(viertel[i] == doctest::Approx(xy[i + 1]).epsilon(1.0e-4).scale(1.0));
    }
    // mit Stereo XY dreht Angle das Bild: bei 90 Grad spielt links, was rechts spielte
    const std::vector<float> xyViertel = herz([](Schicht& s) {
        s.stereoXY = true;
        s.winkelGrad = 90.0;
    });
    for (std::size_t i = 0; i + 1 < xy.size(); i += 2)
    {
        REQUIRE(xyViertel[i] == doctest::Approx(xy[i + 1]).epsilon(1.0e-4).scale(1.0));
        REQUIRE(xyViertel[i + 1] == doctest::Approx(-xy[i]).epsilon(1.0e-4).scale(1.0));
    }
    // Angle aendert die Klangfarbe: die Teiltoene sind anders stark
    CHECK(std::abs(staerke(viertel, 0, hz, 2, f) - staerke(null, 0, hz, 2, f)) > 0.005);

    // Start und Reverse: ein anderer Wellenzug, dieselben Staerken
    const std::vector<float> spaeter = herz([](Schicht& s) { s.formStartProzent = 37.0; });
    const std::vector<float> zurueck = herz([](Schicht& s) { s.rueckwaerts = true; });
    CHECK(groessterUnterschied(spaeter, null) > 0.05);
    CHECK(groessterUnterschied(zurueck, null) > 0.05);
    for (int k = 1; k <= 16; ++k)
    {
        CAPTURE(k);
        const double soll = staerke(null, 0, hz, k, f);
        CHECK(staerke(spaeter, 0, hz, k, f) == doctest::Approx(soll).epsilon(0.002).scale(1.0));
        CHECK(staerke(zurueck, 0, hz, k, f) == doctest::Approx(soll).epsilon(0.002).scale(1.0));
    }
}

TEST_CASE("Shape Ton: Harmonics laeuft weich — und unter der Harmonics envelope springt nichts")
{
    const double hz = tonHz(kA2);
    const Fenster f = fenster(hz);
    const auto fuenfter = [&](double harmonische) {
        return staerke(ton(formInstrument("square", [&](Schicht& s) { s.harmonische = harmonische; })), 0, hz, 5, f);
    };
    const double ganz = fuenfter(5.0);
    REQUIRE(ganz > 0.005);
    CHECK(fuenfter(4.0) < 0.001);
    CHECK(fuenfter(4.5) == doctest::Approx(ganz * 0.5).epsilon(0.02));
    CHECK(fuenfter(4.25) == doctest::Approx(ganz * 0.25).epsilon(0.04));
    CHECK(fuenfter(7.0) == doctest::Approx(ganz).epsilon(0.01));

    // Die Envelope hebt Harmonics: 2 + 40 mal Huellkurve. Gehalten wird bei 0,5 — dort sind es 22.
    const auto huelle = [](Schicht& s) {
        s.anstiegMs = 200.0;
        s.abfallMs = 100.0;
        s.halten = 0.5;
    };
    const std::vector<float> bewegt = ton(formInstrument("square", [&](Schicht& s) {
                                              huelle(s);
                                              s.harmonische = 2.0;
                                              s.harmonischeHuelle = 40.0;
                                          }),
                                          kA2, 1700.0);
    const std::vector<float> fest = ton(formInstrument("square", [&](Schicht& s) {
                                            huelle(s);
                                            s.harmonische = 22.0;
                                        }),
                                        kA2, 1700.0);
    const Fenster spaet = fenster(hz, 40, 1200.0);
    for (const int k : {1, 5, 13, 21})
    {
        CAPTURE(k);
        CHECK(staerke(bewegt, 0, hz, k, spaet) == doctest::Approx(staerke(fest, 0, hz, k, spaet)).epsilon(0.002).scale(1.0));
    }
    CHECK(staerke(bewegt, 0, hz, 23, spaet) < 0.001);
    // am Gipfel der Huellkurve ist mehr da als im Gehaltenen: 2 + 12 mal Huellkurve, langsam
    // fallend — kurz nach dem Gipfel (rund 14) spielt der 11. Teilton, spaet (unter 9) nicht mehr
    const std::vector<float> langsam = ton(formInstrument("square", [](Schicht& s) {
                                               s.anstiegMs = 200.0;
                                               s.abfallMs = 1500.0;
                                               s.halten = 0.5;
                                               s.harmonische = 2.0;
                                               s.harmonischeHuelle = 12.0;
                                           }),
                                           kA2, 1700.0);
    CHECK(staerke(langsam, 0, hz, 11, fenster(hz, 4, 205.0)) > 0.0008);
    CHECK(staerke(langsam, 0, hz, 11, fenster(hz, 40, 1250.0)) < 0.0003);
    // nichts springt: kein Schritt von Wert zu Wert ist groesser als bei einem festen Ton mit 42
    const std::vector<float> voll = ton(formInstrument("square", [](Schicht& s) { s.harmonische = 42.0; }), kA2, 1700.0);
    const auto groessterSchritt = [](const std::vector<float>& daten) {
        double groesster = 0.0;
        for (std::size_t i = 2; i < daten.size(); i += 2)
            groesster = std::max(groesster, std::abs(static_cast<double>(daten[i]) - daten[i - 2]));
        return groesster;
    };
    CHECK(groessterSchritt(bewegt) <= groessterSchritt(voll) * 1.2);

    // negativ: die Note beginnt runder und kommt bei der Form an
    const std::vector<float> runder = ton(formInstrument("square", [&](Schicht& s) {
                                              huelle(s);
                                              s.harmonische = 22.0;
                                              s.harmonischeHuelle = -40.0;
                                          }),
                                          kA2, 1700.0);
    CHECK(staerke(runder, 0, hz, 5, spaet) < 0.001);  // 22 − 40 · 0,5 = 2
}

TEST_CASE("Shape Ton: der Pegel — die Form am fernsten Punkt gemessen, die Spitze bleibt bei jedem Winkel im Rahmen")
{
    // T1 a (Patrik S121): kein Ausgleich je Einstellung; die Form ist so gross, dass ihr fernster
    // Punkt den Abstand 1 hat. Das Quadrat: 0,71 bei 0 Grad, 1 auf der Diagonale.
    const Fenster f = fenster(tonHz(kA2));
    double kleinste = 9.0;
    double groesste = 0.0;
    for (int grad = 0; grad < 360; grad += 15)
    {
        const double s = spitze(ton(formInstrument("square", [&](Schicht& schicht) {
                                    schicht.harmonische = 64.0;
                                    schicht.winkelGrad = grad;
                                })),
                                f);
        kleinste = std::min(kleinste, s);
        groesste = std::max(groesste, s);
        if (grad == 0) CHECK(s == doctest::Approx(kPegel * std::sqrt(0.5)).epsilon(0.03));
    }
    CHECK(groesste <= kPegel * 1.03);
    CHECK(groesste >= kPegel * 0.95);
    CHECK(kleinste >= kPegel * 0.68);
    // jede mitgelieferte Form, mit wenigen und mit vielen Teiltoenen
    for (const char* name : {"circle", "triangle", "star", "heart"})
        for (const double harmonische : {1.0, 4.0, 64.0})
        {
            CAPTURE(name);
            CAPTURE(harmonische);
            const double s = spitze(ton(formInstrument(name, [&](Schicht& schicht) { schicht.harmonische = harmonische; })), f);
            CHECK(s <= kPegel * 1.05);
            CHECK(s >= kPegel * 0.4);
        }
}

TEST_CASE("Shape Ton: Stimmen, Pitch sweep und Level gelten — ohne Form schweigt die Schicht")
{
    const std::vector<float> eine = ton(formInstrument("heart"));
    const std::vector<float> zwei = ton(formInstrument("heart", [](Schicht& s) {
        s.stimmen = 2;
        s.verstimmungCent = 20.0;
    }));
    CHECK(groessterUnterschied(eine, zwei) > 0.05);
    const std::vector<float> fallend = ton(formInstrument("heart", [](Schicht& s) { s.tonhoeheStartHt = 12.0; }));
    CHECK(groessterUnterschied(eine, fallend) > 0.05);
    const std::vector<float> leise = ton(formInstrument("heart", [](Schicht& s) { s.laut = 0.5; }));
    for (std::size_t i = 0; i < eine.size(); i += 97) REQUIRE(leise[i] == doctest::Approx(eine[i] * 0.5f).epsilon(1.0e-4).scale(1.0));
    // derselbe Aufruf ergibt dieselben Werte
    CHECK(ton(formInstrument("heart")) == eine);

    // eine Form, die es nicht gibt, keine genannt, eine Kontur, die es nicht gibt: Stille in voller Laenge
    for (const Instrument& stumm : {formInstrument("gibt_es_nicht"), formInstrument(""),
                                    formInstrument("heart", [](Schicht& s) { s.formKontur = 77; })})
    {
        const std::vector<float> still = ton(stumm);
        REQUIRE(still.size() == eine.size());
        CHECK(std::all_of(still.begin(), still.end(), [](float wert) { return wert == 0.0f; }));
    }
}

TEST_CASE("Shape Ton: die Datei — Fassung 9, hin und zurueck, eine aeltere, eine von morgen")
{
    CHECK(kInstrumentFassung >= 9);  // die Felder gibt es seit Fassung 9; geschrieben wird die juengste (S132: 10)
    Instrument instrument = formInstrument("user:mein_herz", [](Schicht& s) {
        s.formKontur = 3;
        s.harmonische = 37.5;
        s.harmonischeHuelle = -12.0;
        s.winkelGrad = 135.0;
        s.formStartProzent = 25.0;
        s.stereoXY = true;
        s.rueckwaerts = true;
    });
    instrument.schichten.push_back(Schicht{});  // ein Sinus daneben
    const QJsonObject json = alsJson(instrument);
    CHECK(json.value(QStringLiteral("version")).toInt() == kInstrumentFassung);
    const QJsonArray schichten = json.value(QStringLiteral("layers")).toArray();
    REQUIRE(schichten.size() == 2);
    const QJsonObject form = schichten[0].toObject();
    CHECK(form.value(QStringLiteral("waveform")).toString() == QStringLiteral("shape"));
    CHECK(form.value(QStringLiteral("shape")).toString() == QStringLiteral("user:mein_herz"));
    CHECK(form.value(QStringLiteral("shapeHarmonics")).toDouble() == doctest::Approx(37.5));
    // `harmonics` allein bleibt die Liste der Obertoene des Sinus
    CHECK(form.value(QStringLiteral("harmonics")).isArray());
    // eine Schicht, die keine Form spielt, traegt die Felder nicht
    CHECK_FALSE(schichten[1].toObject().contains(QStringLiteral("shape")));
    CHECK_FALSE(schichten[1].toObject().contains(QStringLiteral("shapeHarmonics")));

    Instrument gelesen;
    REQUIRE(ausJson(json, gelesen));
    const Schicht& s = gelesen.schichten[0];
    CHECK(s.schwingung == Schwingung::Form);
    CHECK(s.form == QStringLiteral("user:mein_herz"));
    CHECK(s.formKontur == 3);
    CHECK(s.harmonische == doctest::Approx(37.5));
    CHECK(s.harmonischeHuelle == doctest::Approx(-12.0));
    CHECK(s.winkelGrad == doctest::Approx(135.0));
    CHECK(s.formStartProzent == doctest::Approx(25.0));
    CHECK(s.stereoXY);
    CHECK(s.rueckwaerts);
    CHECK(gelesen.schichten[1] == Schicht{});

    // was ausserhalb liegt, kommt in seine Grenzen
    QJsonObject wild = json;
    QJsonArray wildeSchichten = schichten;
    QJsonObject wildeForm = form;
    wildeForm[QStringLiteral("shapeHarmonics")] = 9999;
    wildeForm[QStringLiteral("shapeHarmonicsEnvelope")] = -9999;
    wildeForm[QStringLiteral("shapeAngle")] = 720;
    wildeForm[QStringLiteral("shapeStart")] = -5;
    wildeForm[QStringLiteral("shapeContour")] = -2;
    wildeSchichten[0] = wildeForm;
    wild[QStringLiteral("layers")] = wildeSchichten;
    REQUIRE(ausJson(wild, gelesen));
    CHECK(gelesen.schichten[0].harmonische == doctest::Approx(kFormHarmonischeMax));
    CHECK(gelesen.schichten[0].harmonischeHuelle == doctest::Approx(-kFormHuelleMax));
    CHECK(gelesen.schichten[0].winkelGrad == doctest::Approx(360.0));
    CHECK(gelesen.schichten[0].formStartProzent == doctest::Approx(0.0));
    CHECK(gelesen.schichten[0].formKontur == 0);

    // eine Datei der Fassung 8 kennt die Felder nicht: die Vorgaben
    QJsonObject alt = alsJson(Instrument{});
    alt[QStringLiteral("version")] = 8;
    REQUIRE(ausJson(alt, gelesen));
    CHECK(gelesen.schichten[0].form.isEmpty());
    CHECK(gelesen.schichten[0].harmonische == doctest::Approx(Schicht{}.harmonische));
    CHECK_FALSE(gelesen.schichten[0].stereoXY);
    // eine Fassung von morgen wird abgelehnt und gemeldet
    QJsonObject morgen = json;
    morgen[QStringLiteral("version")] = kInstrumentFassung + 1;
    bool zuNeu = false;
    CHECK_FALSE(ausJson(morgen, gelesen, &zuNeu));
    CHECK(zuNeu);
}

TEST_CASE("Shape Ton: mehrere Konturen als ein Pfad — jede fuer sich, dazwischen ein kurzer Sprung")
{
    shape::Zeichnung zeichnung;
    REQUIRE(shape::holeZeichnung(QStringLiteral("heart_and_star"), zeichnung));
    REQUIRE(zeichnung.konturen.size() == 2);

    // --- eine Kontur beim Namen, alle als ein Pfad, eine fremde Kennung
    const shape::Tonform herz = shape::tonform(zeichnung, 1);
    REQUIRE(herz.konturen.size() == 1);
    CHECK(herz.sprung == 0.0);
    CHECK(shape::tonform(zeichnung, 7).leer());
    const shape::Tonform beide = shape::tonform(zeichnung, 0);
    REQUIRE(beide.konturen.size() == 2);
    CHECK(beide.sprung == doctest::Approx(shape::kSprungAnteil));
    CHECK(beide.konturen[0].anteil + beide.konturen[1].anteil + 2.0 * beide.sprung == doctest::Approx(1.0));
    // ohne Haken spielt eine Kontur im Pfad nicht mit
    shape::Zeichnung ohne = zeichnung;
    ohne.konturen[1].gewaehlt = false;
    CHECK(shape::tonform(ohne, 0).konturen.size() == 1);

    // --- der Weg des Stifts: zusammenhaengend, im Rahmen, ohne Gleichanteil
    const auto pruefeWeg = [](const shape::Tonform& form, double harmonische) {
        constexpr int kSchritte = 200'000;
        shape::Punkt vorher = shape::tonwert(form, 0.0, harmonische);
        shape::Punkt summe{0.0, 0.0};
        double fern = 0.0;
        double schritt = 0.0;
        for (int n = 1; n <= kSchritte; ++n)
        {
            const shape::Punkt p = shape::tonwert(form, static_cast<double>(n) / kSchritte, harmonische);
            schritt = std::max(schritt, std::abs(p - vorher));
            fern = std::max(fern, std::abs(p));
            summe += p;
            vorher = p;
        }
        CHECK(schritt < 0.01);  // auch ueber den Sprung und ueber das Ende des Umlaufs
        CHECK(fern <= 1.03);
        CHECK(fern >= 0.9);
        CHECK(std::abs(summe / static_cast<double>(kSchritte)) < 0.01);
    };
    pruefeWeg(herz, 512.0);
    pruefeWeg(beide, 512.0);
    pruefeWeg(beide, 16.0);
    // die Sprunge beginnen und enden auf den Konturen
    {
        const shape::TonKontur& erste = beide.konturen[0];
        const shape::TonKontur& zweite = beide.konturen[1];
        const shape::Punkt absprung = shape::wert(erste.reihe, erste.start, 64.0, true);
        const shape::Punkt ankunft = shape::wert(zweite.reihe, zweite.start, 64.0, true);
        CHECK(std::abs(shape::tonwert(beide, erste.anteil, 64.0) - absprung) < 1.0e-6);
        CHECK(std::abs(shape::tonwert(beide, erste.anteil + beide.sprung * 0.5, 64.0) - (absprung + ankunft) * 0.5) < 1.0e-6);
        CHECK(std::abs(shape::tonwert(beide, erste.anteil + beide.sprung, 64.0) - ankunft) < 1.0e-6);
        // gesprungen wird, wo sich die beiden am naechsten kommen: kuerzer als ein Viertel des Rahmens
        CHECK(std::abs(ankunft - absprung) < 0.5);
    }

    // --- drei Konturen (bisher nicht geprobt): jede wird betreten und dort wieder verlassen
    shape::Zeichnung drei;
    REQUIRE(drei.neueKontur(quadrat(0.0, 0.0, 100.0)) == 1);
    REQUIRE(drei.neueKontur(quadrat(150.0, 0.0, 100.0)) == 2);
    REQUIRE(drei.neueKontur(quadrat(150.0, 150.0, 60.0)) == 3);
    const shape::Tonform dreiForm = shape::tonform(drei, 0);
    REQUIRE(dreiForm.konturen.size() == 3);
    CHECK(dreiForm.konturen[2].anteil < dreiForm.konturen[0].anteil);  // die kleinere bekommt weniger vom Umlauf
    pruefeWeg(dreiForm, 512.0);

    // --- die Schicht: Contour 0 spielt beide, eine Kennung nur die eine
    const std::vector<float> pfad = ton(formInstrument("heart_and_star"));
    const std::vector<float> nurHerz = ton(formInstrument("heart_and_star", [](Schicht& s) { s.formKontur = 1; }));
    const std::vector<float> nurStern = ton(formInstrument("heart_and_star", [](Schicht& s) { s.formKontur = 2; }));
    const Fenster f = fenster(tonHz(kA2));
    CHECK(spitze(pfad, f) > 0.2);
    CHECK(groessterUnterschied(pfad, nurHerz) > 0.05);
    CHECK(groessterUnterschied(nurHerz, nurStern) > 0.05);
    CHECK(spitze(pfad, f) <= kPegel * 1.05);

    // --- gemerkt, bis eine Zeichnung geschrieben wird
    const auto a = shape::tonformFuer(QStringLiteral("heart"), 0);
    REQUIRE(a != nullptr);
    CHECK(shape::tonformFuer(QStringLiteral("heart"), 0) == a);
    CHECK(shape::tonformFuer(QStringLiteral("gibt_es_nicht"), 0) == nullptr);
    shape::vergissTonformen();
    CHECK(shape::tonformFuer(QStringLiteral("heart"), 0) != a);
}

TEST_CASE("Shape Ton Messung: die Rechenzeit — und die Kante des Sprungs")
{
    // --- Rechenzeit je Sekunde Ton: die kuerzeste von drei Messungen, mit der Taktprobe daneben
    const auto messe = [](const std::string& name, const Instrument& instrument, int hoehe) {
        double bester = 1.0e30;
        double takt = 0.0;
        std::int64_t frames = 0;
        for (int lauf = 0; lauf < 3; ++lauf)
        {
            takt = std::max(takt, taktprobeMs());
            const auto start = std::chrono::steady_clock::now();
            frames = rendereTon(instrument, hoehe, 1000.0, kRate).frames();
            bester = std::min(bester, std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count());
        }
        REQUIRE(frames > 0);
        MESSAGE(name << ": " << bester * kRate / static_cast<double>(frames) << " ms je Sekunde Ton (Taktprobe " << takt << " ms)");
    };
    (void)shape::tonformFuer(QStringLiteral("heart"), 0);           // das Aufbereiten zaehlt nicht mit
    (void)shape::tonformFuer(QStringLiteral("heart_and_star"), 0);
    messe("heart, A2, 24 Harmonics", formInstrument("heart"), kA2);
    messe("heart, A2, 64 Harmonics", formInstrument("heart", [](Schicht& s) { s.harmonische = 64.0; }), kA2);
    messe("heart, A1, 512 Harmonics (die Bandgrenze laesst 340)", formInstrument("heart", [](Schicht& s) { s.harmonische = 512.0; }), 33);
    messe("heart, A2, 64 Harmonics, drei Stimmen, Stereo XY", formInstrument("heart", [](Schicht& s) {
              s.harmonische = 64.0;
              s.stimmen = 3;
              s.verstimmungCent = 12.0;
              s.stereoXY = true;
          }),
          kA2);
    messe("heart_and_star als ein Pfad, A2, 64 Harmonics", formInstrument("heart_and_star", [](Schicht& s) { s.harmonische = 64.0; }), kA2);
    {
        // das Aufbereiten einer Form (einmal, dann gemerkt)
        shape::Zeichnung zeichnung;
        REQUIRE(shape::holeZeichnung(QStringLiteral("heart_and_star"), zeichnung));
        const auto start = std::chrono::steady_clock::now();
        const shape::Tonform form = shape::tonform(zeichnung, 0);
        MESSAGE("heart_and_star aufbereiten (zwei Konturen): "
                << std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count() << " ms");
        CHECK(form.konturen.size() == 2);
    }

    // --- die Kante des Sprungs (B2 d): wie viel vom Ton liegt ueber dem, was Harmonics zulaesst?
    // Eine Kontur, der ein Anteil a des Umlaufs gehoert, reicht mit H Teiltoenen bis zum Teilton
    // H / a des ganzen Umlaufs. Was darueber liegt, kommt vom Sprung.
    shape::Zeichnung zeichnung;
    REQUIRE(shape::holeZeichnung(QStringLiteral("heart_and_star"), zeichnung));
    const shape::Tonform beide = shape::tonform(zeichnung, 0);
    const double kleinsterAnteil = std::min(beide.konturen[0].anteil, beide.konturen[1].anteil);
    for (const int hoehe : {kA2, 69})
    {
        const double hz = tonHz(hoehe);
        const Fenster f = fenster(hz, 40);
        for (const double harmonische : {16.0, 48.0, 160.0})
        {
            const std::vector<float> daten = ton(formInstrument("heart_and_star", [&](Schicht& s) { s.harmonische = harmonische; }), hoehe);
            double gesamt = 0.0;
            for (std::int64_t t = f.von; t < f.bis; ++t)
            {
                const double w = daten[static_cast<std::size_t>(2 * t)];
                gesamt += w * w;
            }
            gesamt /= static_cast<double>(f.bis - f.von);
            const int grenze = std::min(static_cast<int>(std::ceil(harmonische / kleinsterAnteil)) + 1,
                                        static_cast<int>(kRate / 2.0 / hz) - 1);
            double darunter = 0.0;
            for (int k = 1; k <= grenze; ++k)
            {
                const double a = staerke(daten, 0, hz, k, f);
                darunter += a * a / 2.0;
            }
            const double rest = std::max(gesamt - darunter, 1.0e-12);
            MESSAGE("ein Pfad, " << tonName(hoehe).toStdString() << ", " << harmonische << " Harmonics: ueber dem Teilton "
                                 << grenze << " liegen " << 10.0 * std::log10(rest / gesamt) << " dB des Tons");
            CHECK(gesamt > 0.0);
        }
    }

    // --- Hoerproben
    const QString ordner = qEnvironmentVariable("LUMIVIZ_19C_WAV");
    if (ordner.isEmpty()) return;
    REQUIRE(QDir().mkpath(ordner));
    const auto lege = [&](const QString& name, const std::vector<float>& daten) {
        CHECK(schreibeWav(QDir(ordner).filePath(name + QStringLiteral(".wav")), daten, kRate));
    };
    const auto weich = [](Schicht& s) {
        s.anstiegMs = 10.0;
        s.ausklangMs = 200.0;
    };
    // jede Form mit 4, 16 und 64 Teiltoenen
    for (const char* name : {"circle", "square", "triangle", "star", "heart"})
        for (const int harmonische : {4, 16, 64})
            lege(QStringLiteral("form_%1_h%2").arg(QLatin1String(name)).arg(harmonische, 2, 10, QLatin1Char('0')),
                 ton(formInstrument(name, [&](Schicht& s) {
                         weich(s);
                         s.harmonische = harmonische;
                     }),
                     kA2, 1200.0));
    // Angle, Stereo XY
    for (const int grad : {0, 45, 90})
        lege(QStringLiteral("heart_angle_%1").arg(grad, 2, 10, QLatin1Char('0')), ton(formInstrument("heart", [&](Schicht& s) {
                                                                                          weich(s);
                                                                                          s.winkelGrad = grad;
                                                                                      }),
                                                                                      kA2, 1200.0));
    lege(QStringLiteral("heart_stereo_xy"), ton(formInstrument("heart", [&](Schicht& s) {
                                                    weich(s);
                                                    s.stereoXY = true;
                                                }),
                                                kA2, 1200.0));
    // Harmonics envelope: auf und zu
    const auto langeHuelle = [](Schicht& s) {
        s.anstiegMs = 600.0;
        s.abfallMs = 1500.0;
        s.halten = 0.3;
        s.ausklangMs = 300.0;
    };
    lege(QStringLiteral("star_envelope_opens"), ton(formInstrument("star", [&](Schicht& s) {
                                                        langeHuelle(s);
                                                        s.harmonische = 2.0;
                                                        s.harmonischeHuelle = 60.0;
                                                    }),
                                                    kA2, 2500.0));
    lege(QStringLiteral("star_envelope_closes"), ton(formInstrument("star", [&](Schicht& s) {
                                                         langeHuelle(s);
                                                         s.harmonische = 62.0;
                                                         s.harmonischeHuelle = -60.0;
                                                     }),
                                                     kA2, 2500.0));
    // die Kante des Sprungs: ein Pfad gegen die Konturen einzeln
    for (const int harmonische : {16, 48, 160})
    {
        lege(QStringLiteral("onepath_h%1").arg(harmonische, 3, 10, QLatin1Char('0')),
             ton(formInstrument("heart_and_star", [&](Schicht& s) {
                     weich(s);
                     s.harmonische = harmonische;
                 }),
                 kA2, 1500.0));
        lege(QStringLiteral("onepath_h%1_a4").arg(harmonische, 3, 10, QLatin1Char('0')),
             ton(formInstrument("heart_and_star", [&](Schicht& s) {
                     weich(s);
                     s.harmonische = harmonische;
                 }),
                 69, 1500.0));
    }
    lege(QStringLiteral("onepath_only_heart_h48"), ton(formInstrument("heart_and_star", [&](Schicht& s) {
                                                           weich(s);
                                                           s.harmonische = 48.0;
                                                           s.formKontur = 1;
                                                       }),
                                                       kA2, 1500.0));
    // Ecken daempfen (offen, §5): dieselbe Form mit und ohne Lanczos-Faktoren — hier von Hand
    // gerechnet, der Klangerzeuger daempft nicht
    for (const char* name : {"square", "star"})
    {
        shape::Zeichnung z;
        REQUIRE(shape::holeZeichnung(QString::fromLatin1(name), z));
        const shape::Tonform form = shape::tonform(z, 0);
        REQUIRE(form.konturen.size() == 1);
        for (const int harmonische : {8, 24})
        {
            shape::Reihe gedaempft = form.konturen[0].reihe;
            for (int k = 1; k <= harmonische; ++k)
            {
                const double x = kPi * k / (harmonische + 1.0);
                const double faktor = std::sin(x) / x;
                gedaempft.plus[static_cast<std::size_t>(k - 1)] *= faktor;
                gedaempft.minus[static_cast<std::size_t>(k - 1)] *= faktor;
            }
            const double hz = tonHz(kA2);
            const std::int64_t frames = std::llround(1.2 * kRate);
            for (const bool daempfen : {false, true})
            {
                std::vector<float> daten(static_cast<std::size_t>(2 * frames), 0.0f);
                for (std::int64_t t = 0; t < frames; ++t)
                {
                    const double blende = std::min({1.0, static_cast<double>(t) / 441.0, static_cast<double>(frames - 1 - t) / 4410.0});
                    const shape::Punkt p = shape::wert(daempfen ? gedaempft : form.konturen[0].reihe,
                                                       hz * static_cast<double>(t) / kRate, harmonische);
                    const auto w = static_cast<float>(p.real() * kPegel * blende);
                    daten[static_cast<std::size_t>(2 * t)] = w;
                    daten[static_cast<std::size_t>(2 * t + 1)] = w;
                }
                lege(QStringLiteral("corners_%1_h%2_%3")
                         .arg(QLatin1String(name))
                         .arg(harmonische, 2, 10, QLatin1Char('0'))
                         .arg(daempfen ? QStringLiteral("damped") : QStringLiteral("plain")),
                     daten);
            }
        }
    }
    MESSAGE("Hoerproben: " << ordner.toStdString());
}
