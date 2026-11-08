/**
 ****************************************************************************************
 * @file   test_LoopHammerSaite.cpp
 * @brief  Loop, Stufe 31a: Modell und Datei der Hammer-Saite
 *         (docs/audio/Spielweisen_Konzept.md §4.1, Entscheide Patrik S145: HS1 a, HA1 a
 *         bis HA4 a; S146: HK1 a, HK2 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Qt-Anwendung; ohne Dateien aus dem Repo bis auf einen Fall:
 * der Waechter ueber `asset/loop/instruments/grand_piano.lvinst` (31d-8, SP1 a) liest vom
 * Arbeitsordner aus — die Test-Exe aus dem Projekt-Root starten.
 * `LUMIVIZ_GRAND_PIANO_SCHREIBEN=1` schreibt die Datei aus der Vorgabe neu (nach jeder neuen
 * Vorgabe in `baueVorgabe()`); danach `python asset/loop/instrument_katalog.py`.
 *
 * - Kurven ueber die Tastatur (HK1 a): Randwert, Abweichung je Taste, monoton kubisch
 *   ohne Ueberschwingen, die logarithmische Skala, Richten.
 * - Die Vorgaben: Hammer und Saiten nach Chabassier & Durufle RT-0425 (HT1 a, HT2 a), B nach
 *   Rigaud Gl. 5, der Chor.
 * - Die Datei: Fassung 21 (34d, IK2-4 a, FU1 a, FU2 a): `core` `hammerString`, `coreParams` nach der Tafel
 *   (jede Kurve mit `evidence` und `source`, `undampedFrom` in der Tafel), `body` an der Schicht, `keyboard`
 *   ohne `undampedFrom`; ein fehlender Wert oder ein unbekannter Kern macht die Datei unlesbar (IK2-11 a);
 *   Fassung 20 (`modal`) gilt nicht mehr als Fluegel; die Tastatur in ihren Grenzen.
 * - Die Anschlagstaerke (HK2 a): Stufen, Hammergeschwindigkeit, Rest, `:a<stufe>`.
 * - Die Hammer-Saite ist keine gegriffene Saite; seit 31b klingt sie (gemessen in
 *   test_LoopHammerTon.cpp).
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentCore/Anschlag.hpp"
#include "FluegelSchicht.hpp"
#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>
#include <cmath>

using namespace lumi::loop;
using namespace lumi::kern::hammer;
using lumi::kern::anschlagRest;
using lumi::kern::anschlagStufe;
using lumi::kern::kAnschlagStufeMin;

namespace
{

lumi::kern::Kurve kurve(std::initializer_list<lumi::kern::NotenWert> punkte)
{
    lumi::kern::Kurve k;
    k.punkte.assign(punkte.begin(), punkte.end());
    return k;
}

/// Ein Instrument mit einer Kern-Schicht des Fluegels und einer Tastatur.
Instrument fluegel()
{
    Instrument instrument;
    instrument.titel = QStringLiteral("Test grand");
    instrument.schichten[0] = fluegeltest::fluegelSchicht();
    instrument.tastatur = Tastatur{};
    return instrument;
}

Instrument zurueck(const Instrument& instrument)
{
    Instrument gelesen;
    REQUIRE(ausJson(alsJson(instrument), gelesen));
    return gelesen;
}

/// Die erste Schicht einer Datei und ihr `coreParams`.
QJsonObject ersteSchicht(const QJsonObject& json)
{
    return json.value(QStringLiteral("layers")).toArray().first().toObject();
}

/// Die erste Schicht in der Datei ersetzen.
void setzeErsteSchicht(QJsonObject& json, const QJsonObject& schicht)
{
    QJsonArray schichten = json.value(QStringLiteral("layers")).toArray();
    schichten[0] = schicht;
    json[QStringLiteral("layers")] = schichten;
}

/// Das mitgelieferte `grand_piano` (31d-8, SP1 a): die Vorgabe der Hammer-Saite mit dem Boden des
/// Steinway D und der Tastatur der Vorgabe — wie es das Programm schreibt.
Instrument grandPiano()
{
    Instrument instrument = fluegel();
    instrument.titel = QStringLiteral("Grand Piano");
    instrument.schichten[0] = fluegeltest::fluegelSchicht(hammerSaiteVorgabe(), QStringLiteral("lib:steinway_d"));
    instrument.schichten[0].name = QStringLiteral("Hammer string");
    // wie in der Datei bis Fassung 20 (`waveform` `string`); eine Kern-Schicht rechnet ueber `Tonkern`, die Schwingung
    // aendert daran nichts
    instrument.schichten[0].schwingung = Schwingung::Saite;
    return instrument;
}

const QString kGrandPianoDatei = QStringLiteral("asset/loop/instruments/grand_piano.lvinst");

}  // namespace

TEST_CASE("Loop Hammer-Saite 31d-8 grand_piano traegt die Vorgabe")
{
    // Werkzeug: die Datei aus der Vorgabe neu schreiben (Titel, Schicht, Kurven, Tastatur); Art, Genres,
    // Beschreibung und Pegel setzt danach `instrument_katalog.py` — sie aendern Kurven und Tastatur nicht
    const Instrument soll = grandPiano();
    if (qEnvironmentVariableIntValue("LUMIVIZ_GRAND_PIANO_SCHREIBEN") == 1)
    {
        QFile aus(kGrandPianoDatei);
        REQUIRE_MESSAGE(aus.open(QIODevice::WriteOnly | QIODevice::Truncate),
                        "asset/loop/instruments nicht gefunden — aus dem Projekt-Root starten");
        REQUIRE(aus.write(QJsonDocument(alsJson(soll)).toJson(QJsonDocument::Indented)) > 0);
        MESSAGE("grand_piano.lvinst geschrieben — danach python asset/loop/instrument_katalog.py");
    }

    QFile datei(kGrandPianoDatei);
    REQUIRE_MESSAGE(datei.open(QIODevice::ReadOnly), "grand_piano.lvinst nicht gefunden — aus dem Projekt-Root starten");
    Instrument ist;
    REQUIRE_MESSAGE(ausJson(QJsonDocument::fromJson(datei.readAll()).object(), ist),
                    "grand_piano.lvinst nicht lesbar (Fassung 21?) — LUMIVIZ_GRAND_PIANO_SCHREIBEN=1, dann instrument_katalog.py");
    CHECK(ist.titel == soll.titel);
    CHECK(ist.art == QStringLiteral("keys"));
    REQUIRE(ist.schichten.size() == 1);
    REQUIRE(istKernSchicht(ist.schichten[0]));
    CHECK(ist.schichten[0].kernName == QStringLiteral("hammerString"));
    CHECK(ist.schichten[0].koerper == QStringLiteral("lib:steinway_d"));
    CHECK_MESSAGE(fluegeltest::saiteVon(ist.schichten[0]) == fluegeltest::saiteVon(soll.schichten[0]),
                  "grand_piano.lvinst ist veraltet — LUMIVIZ_GRAND_PIANO_SCHREIBEN=1, dann instrument_katalog.py");
    CHECK(fluegeltest::pedaleVon(ist.schichten[0]) == fluegeltest::pedaleVon(soll.schichten[0]));
    CHECK_MESSAGE(ist.tastatur == soll.tastatur, "Tastatur fehlt oder weicht ab — instrument_katalog.ergaenze traegt `keyboard`?");
}

TEST_CASE("Loop Hammer-Saite 31a Kurve")
{
    SUBCASE("Randwert, Abweichung, ohne Punkte")
    {
        const lumi::kern::Kurve k = kurve({{40, 2.0}, {80, 4.0}});
        CHECK(wertAn(k, 21, lumi::kern::Skala::Linear) == doctest::Approx(2.0));
        CHECK(wertAn(k, 108, lumi::kern::Skala::Linear) == doctest::Approx(4.0));
        CHECK(wertAn(k, 60, lumi::kern::Skala::Linear) == doctest::Approx(3.0));  // zwei Punkte: die Gerade
        lumi::kern::Kurve mit = k;
        mit.abweichungen = {{60, 9.0}};
        CHECK(wertAn(mit, 60, lumi::kern::Skala::Linear) == doctest::Approx(9.0));
        CHECK(wertAn(mit, 61, lumi::kern::Skala::Linear) == doctest::Approx(3.05));
        CHECK(wertAn(lumi::kern::Kurve{}, 60, lumi::kern::Skala::Linear) == 0.0);
    }
    SUBCASE("monoton kubisch: trifft die Punkte, schwingt nicht ueber")
    {
        // ein Sprung mit flachen Stuecken davor und danach — ein gewoehnlicher Spline
        // schwaenge hier ueber 1 und unter 0
        const lumi::kern::Kurve k = kurve({{21, 0.0}, {40, 0.0}, {45, 1.0}, {108, 1.0}});
        for (const lumi::kern::NotenWert& p : k.punkte) CHECK(wertAn(k, p.note, lumi::kern::Skala::Linear) == doctest::Approx(p.wert));
        double vorher = -1.0;
        for (int t = 21; t <= 108; ++t)
        {
            const double w = wertAn(k, t, lumi::kern::Skala::Linear);
            CHECK(w >= 0.0);
            CHECK(w <= 1.0);
            CHECK(w >= vorher - 1e-12);  // steigt nur
            vorher = w;
        }
        // glatt: zwischen 40 und 45 kein gerader Weg
        CHECK(wertAn(k, 41, lumi::kern::Skala::Linear) < 0.2);
        CHECK(wertAn(k, 44, lumi::kern::Skala::Linear) > 0.8);
    }
    SUBCASE("logarithmisch: eine Exponentialfunktion aus wenigen Punkten genau")
    {
        // eine Exponentialfunktion (wie Q0 bei Stulov) ist in ln eine Gerade — monoton kubisch gibt
        // sie genau wieder
        const auto exponentiell = [](int n) { return 183.0 * std::exp(0.045 * n); };
        const lumi::kern::Kurve k = kurve({{21, exponentiell(1)}, {69, exponentiell(49)}, {108, exponentiell(88)}});
        for (int t = 21; t <= 108; t += 7)
            CHECK(wertAn(k, t, lumi::kern::Skala::Logarithmisch) == doctest::Approx(exponentiell(t - 20)).epsilon(1e-9));
        // in den Werten selbst dagegen laege sie dazwischen zu hoch (gemessen: gut 6 %)
        CHECK(wertAn(k, 45, lumi::kern::Skala::Linear) > exponentiell(25) * 1.05);
    }
    SUBCASE("richten: sortiert, je Taste einmal, Grenzen, Ersatz")
    {
        lumi::kern::Kurve k = kurve({{80, 5.0}, {10, 1.0}, {80, 7.0}, {60, std::nan("")}, {200, 50.0}});
        richteKurve(k, 0.0, 10.0, kurve({{21, 3.0}}), kTasteMin, kTasteMax);  // der Umfang des Fluegels, nicht MIDI
        REQUIRE(k.punkte.size() == 3);
        CHECK(k.punkte[0] == lumi::kern::NotenWert{21, 1.0});
        CHECK(k.punkte[1] == lumi::kern::NotenWert{80, 5.0});  // der erste an 80 bleibt
        CHECK(k.punkte[2] == lumi::kern::NotenWert{108, 10.0});
        lumi::kern::Kurve leer;
        richteKurve(leer, 0.0, 10.0, kurve({{21, 3.0}}), kTasteMin, kTasteMax);
        CHECK(leer.punkte == std::vector<lumi::kern::NotenWert>{{21, 3.0}});
    }
}

TEST_CASE("Loop Hammer-Saite 31a Vorgaben")
{
    // RT-0425 Gl. 2–4 gegen Anhang A (C1 = Taste 4, G6 = Taste 71, am Bild gelesen)
    CHECK(rtHammerMasseG(4) == doctest::Approx(10.95).epsilon(1e-3));
    CHECK(rtHammerExponent(4) == doctest::Approx(2.310).epsilon(1e-3));
    CHECK(rtHammerSteifeNm(4) == doctest::Approx(7.160e7).epsilon(2e-3));
    CHECK(rtHammerMasseG(71) == doctest::Approx(6.77).epsilon(2e-3));
    CHECK(rtHammerExponent(71) == doctest::Approx(3.015).epsilon(1e-3));
    CHECK(rtHammerSteifeNm(71) == doctest::Approx(2.585e11).epsilon(2e-3));
    // umgerechnet auf mm: K·u^p mit u = 1 mm
    CHECK(rtHammerSteife(4) == doctest::Approx(rtHammerSteifeNm(4) * std::pow(1e-3, rtHammerExponent(4))));
    // Rigaud Gl. 5: im Diskant traegt der Bassteg kaum noch etwas (bei C8 5·10⁻⁵ des Werts) —
    // e^(0,0926·108 − 13,64)
    CHECK(rigaudB(108, kRigaudBassSteigung, kRigaudBassAchse) == doctest::Approx(std::exp(0.0926 * 108 - 13.64)).epsilon(1e-4));
    // abgelesen aus Abb. 1: B(A0) ≈ 2,8·10⁻⁴, das Minimum um m = 40 bei ≈ 1,35·10⁻⁴
    CHECK(rigaudB(21, kRigaudBassSteigung, kRigaudBassAchse) == doctest::Approx(2.8e-4).epsilon(0.05));
    CHECK(rigaudB(41, kRigaudBassSteigung, kRigaudBassAchse) == doctest::Approx(1.35e-4).epsilon(0.05));

    // Stulov & Kartofelev 2009 Gl. 9 (HU3-1 c): an Taste 1 m 11,0 g, p 3,715, Q0 191 N/mm^p, ε 0,989, τ 2,70 µs;
    // an Taste 88 m 5,33 g, p 5,02, Q0 9600, ε 0,308, τ 1,66 µs
    CHECK(stulovHammerMasseG(1) == doctest::Approx(11.0001));
    CHECK(stulovHammerExponent(88) == doctest::Approx(5.02));
    CHECK(stulovHammerSteife(88) == doctest::Approx(183.0 * std::exp(0.045 * 88)));
    CHECK(stulovHammerHysterese(1) == doctest::Approx(0.9893));
    CHECK(stulovHammerHysterese(88) == doctest::Approx(0.308).epsilon(0.01));
    CHECK(stulovHammerRelaxationUs(88) == doctest::Approx(1.66).epsilon(0.01));
    const HammerSaite& v = hammerSaiteVorgabe();
    for (const int taste : {21, 27, 33, 39, 45, 51, 57, 63, 69, 75, 81, 87, 93, 99, 105, 108})
    {
        CHECK(wertAn(v.hammerMasseG, taste, lumi::kern::Skala::Linear) == doctest::Approx(stulovHammerMasseG(taste - 20)));
        CHECK(wertAn(v.hammerExponent, taste, lumi::kern::Skala::Linear) == doctest::Approx(stulovHammerExponent(taste - 20)));
        CHECK(wertAn(v.hammerSteife, taste, lumi::kern::Skala::Logarithmisch) == doctest::Approx(stulovHammerSteife(taste - 20)));
        CHECK(wertAn(v.hammerHysterese, taste, lumi::kern::Skala::Linear) == doctest::Approx(stulovHammerHysterese(taste - 20)));
        CHECK(wertAn(v.hammerRelaxationUs, taste, lumi::kern::Skala::Logarithmisch) == doctest::Approx(stulovHammerRelaxationUs(taste - 20)));
    }
    // zwischen den Stuetzpunkten: p genau (eine Gerade), Q0 genau (eine Gerade in ln), Masse, ε und τ nah (Parabeln), B nah
    for (int taste = 21; taste <= 108; ++taste)
    {
        CHECK(wertAn(v.hammerMasseG, taste, lumi::kern::Skala::Linear) == doctest::Approx(stulovHammerMasseG(taste - 20)).epsilon(0.01));
        CHECK(wertAn(v.hammerExponent, taste, lumi::kern::Skala::Linear) == doctest::Approx(stulovHammerExponent(taste - 20)).epsilon(0.001));
        CHECK(wertAn(v.hammerSteife, taste, lumi::kern::Skala::Logarithmisch) == doctest::Approx(stulovHammerSteife(taste - 20)).epsilon(0.005));
        CHECK(wertAn(v.hammerHysterese, taste, lumi::kern::Skala::Linear) == doctest::Approx(stulovHammerHysterese(taste - 20)).epsilon(0.01));
        CHECK(wertAn(v.hammerRelaxationUs, taste, lumi::kern::Skala::Logarithmisch) == doctest::Approx(stulovHammerRelaxationUs(taste - 20)).epsilon(0.02));
        CHECK(wertAn(v.inharmonizitaet, taste, lumi::kern::Skala::Logarithmisch) ==
              doctest::Approx(rigaudB(taste, kRigaudBassSteigung, kRigaudBassAchse)).epsilon(0.1));
    }
    // die Saiten an Tasten, die keine Stuetzpunkte sind, gegen RT-0425 Anhang A (L, d, ρ, T0):
    // µ = ρπd²/4, f_l0/f0 = √(EA/T0) mit E = 2,02·10¹¹ Pa — auf 2 %
    struct Zeile
    {
        int taste;
        double laenge, dMm, rho, t0;
    };
    for (const Zeile& z : {Zeile{30, 1.879, 1.370, 33851.0, 1515.0}, Zeile{46, 1.403, 1.067, 7850.0, 754.0},
                           Zeile{69, 0.404, 0.947, 7850.0, 701.0}, Zeile{72, 0.344, 0.932, 7850.0, 696.0},
                           Zeile{100, 0.076, 0.802, 7850.0, 646.0}})
    {
        CAPTURE(z.taste);
        const double flaeche = 3.141592653589793 * std::pow(z.dMm * 1e-3, 2) / 4.0;
        CHECK(wertAn(v.saitenLaengeM, z.taste, lumi::kern::Skala::Logarithmisch) == doctest::Approx(z.laenge).epsilon(0.02));
        CHECK(wertAn(v.saitenMasseGm, z.taste, lumi::kern::Skala::Logarithmisch) ==
              doctest::Approx(z.rho * flaeche * 1e3).epsilon(0.02));
        CHECK(wertAn(v.laengsVerhaeltnis, z.taste, lumi::kern::Skala::Linear) ==
              doctest::Approx(std::sqrt(2.02e11 * flaeche / z.t0)).epsilon(0.02));
    }
    CHECK(wertAn(v.anschlagStelle, 60, lumi::kern::Skala::Linear) == doctest::Approx(0.12));
    // der Chor (vorlaeufig): eine, zwei, drei Saiten
    CHECK(chorAn(v, 21) == 1);
    CHECK(chorAn(v, 30) == 1);
    CHECK(chorAn(v, 31) == 2);
    CHECK(chorAn(v, 48) == 2);
    CHECK(chorAn(v, 49) == 3);
    CHECK(chorAn(v, 108) == 3);
    // eine neue Kern-Schicht des Fluegels traegt die Vorgaben schon
    CHECK(fluegeltest::saiteVon(fluegeltest::fluegelSchicht()) == v);
}

TEST_CASE("Loop Hammer-Saite 34d Datei")
{
    SUBCASE("Kern, Werte nach der Tafel, Koerper und Tastatur hin und zurueck, immer geschrieben")
    {
        Instrument instrument = fluegel();
        HammerSaite h = fluegeltest::saiteVon(instrument.schichten[0]);
        h.verstimmungCent.abweichungen = {{60, 3.5}};
        h.chor = kurve({{21, 2.0}, {108, 3.0}});
        h.stossLeise = kurve({{21, 0.1}, {108, 0.4}});  // hin und zurueck nicht die Vorgabe
        h.stossLaut = kurve({{21, 0.3}, {108, 0.2}});
        fluegeltest::setzeSaite(instrument.schichten[0], h);
        fluegeltest::setzeUngedaempftAb(instrument.schichten[0], 80);
        instrument.schichten[0].koerper = QStringLiteral("lib:steinway_d");
        instrument.tastatur->links = PedalArt::Leise;
        instrument.tastatur->mitte = PedalArt::Moderator;

        const QJsonObject json = alsJson(instrument);
        CHECK(json.value(QStringLiteral("version")).toInt() == 23);  // seit QM1 a: Quellen je Wert `coreSources`; seit 34f-1: Quellen je Kurve (WZ1 a); seit 34d: der Kern (IK2-4 a, FU1 a, FU2 a)
        const QJsonObject schicht = ersteSchicht(json);
        CHECK(schicht.value(QStringLiteral("core")).toString() == QStringLiteral("hammerString"));
        CHECK(schicht.value(QStringLiteral("body")).toString() == QStringLiteral("lib:steinway_d"));
        for (const char* weg : {"hammerString", "stringModel", "excitation", "pluckPosition"})
        {
            CAPTURE(weg);
            CHECK_FALSE(schicht.contains(QLatin1String(weg)));  // bis Fassung 20, oder nur an der gezupften Saite
        }
        // jeder Wert der Tafel, in ihrer Reihenfolge; jede Kurve mit Belegkraft und Herkunft
        const QJsonObject werte = schicht.value(QStringLiteral("coreParams")).toObject();
        const lumi::kern::Parametertafel& tafel = kernTyp().tafel;
        CHECK(werte.size() == static_cast<qsizetype>(tafel.werte.size()));
        for (const lumi::kern::ParameterAngabe& a : tafel.werte)
        {
            CAPTURE(a.name);
            const QJsonValue w = werte.value(QString::fromStdString(a.name));
            REQUIRE_FALSE(w.isUndefined());
            if (a.art != lumi::kern::WertArt::Kurve) continue;
            const QJsonObject k = w.toObject();
            CHECK(k.value(QStringLiteral("points")).isArray());
            CHECK(k.value(QStringLiteral("overrides")).isArray());  // auch leer
            CHECK(k.value(QStringLiteral("evidence")).isString());
            CHECK(k.value(QStringLiteral("source")).isString());
        }
        CHECK(werte.value(QStringLiteral("undampedFrom")).toInt() == 80);  // seit FU2 a in der Tafel
        const QJsonObject tastatur = json.value(QStringLiteral("keyboard")).toObject();
        CHECK(tastatur.value(QStringLiteral("range")).toArray() == QJsonArray{21, 108});
        CHECK_FALSE(tastatur.contains(QStringLiteral("undampedFrom")));
        const QJsonObject pedale = tastatur.value(QStringLiteral("pedals")).toObject();
        CHECK(pedale.value(QStringLiteral("left")).toString() == QStringLiteral("soft"));
        CHECK(pedale.value(QStringLiteral("middle")).toString() == QStringLiteral("practice"));
        CHECK(pedale.value(QStringLiteral("right")).toString() == QStringLiteral("sustain"));

        const Instrument gelesen = zurueck(instrument);
        CHECK(fluegeltest::saiteVon(gelesen.schichten[0]) == h);
        CHECK(fluegeltest::pedaleVon(gelesen.schichten[0]).ungedaempftAb == 80);
        CHECK(gelesen.schichten[0].koerper == QStringLiteral("lib:steinway_d"));
        CHECK(gelesen.tastatur == instrument.tastatur);
        CHECK(gelesen == instrument);
    }
    SUBCASE("ein fehlender Wert macht die Datei unlesbar — kein stiller Ersatz (IK2-11 a)")
    {
        QJsonObject json = alsJson(fluegel());
        QJsonObject schicht = ersteSchicht(json);
        QJsonObject werte = schicht.value(QStringLiteral("coreParams")).toObject();
        werte.remove(QStringLiteral("strikeUnevennessLoud"));
        schicht[QStringLiteral("coreParams")] = werte;
        setzeErsteSchicht(json, schicht);
        Instrument gelesen;
        CHECK_FALSE(ausJson(json, gelesen));
    }
    SUBCASE("ein Kern, den dieses Programm nicht kennt, ebenso")
    {
        QJsonObject json = alsJson(fluegel());
        QJsonObject schicht = ersteSchicht(json);
        schicht[QStringLiteral("core")] = QStringLiteral("gibtEsNicht");
        setzeErsteSchicht(json, schicht);
        Instrument gelesen;
        CHECK_FALSE(ausJson(json, gelesen));
    }
    SUBCASE("Fassung 20 mit `modal` und `hammerString` liest dieses Programm nicht mehr als Fluegel")
    {
        // seit 34d ist der Fluegel der Kern `hammerString` (keine Rueckwaerts-Pflege, das Programm ist unveroeffentlicht);
        // eine alte Schicht wird eine Saite Classic, die mitgelieferten Dateien stehen in Fassung 21
        QJsonObject json = alsJson(fluegel());
        json[QStringLiteral("version")] = 20;
        QJsonObject schicht = ersteSchicht(json);
        schicht.remove(QStringLiteral("core"));
        schicht.remove(QStringLiteral("coreParams"));
        schicht[QStringLiteral("stringModel")] = QStringLiteral("modal");
        schicht[QStringLiteral("excitation")] = QStringLiteral("hammer");
        schicht[QStringLiteral("hammerString")] = QJsonObject{};
        setzeErsteSchicht(json, schicht);
        Instrument gelesen;
        REQUIRE(ausJson(json, gelesen));
        CHECK_FALSE(istKernSchicht(gelesen.schichten[0]));
        CHECK(gelesen.schichten[0].saitenModell == SaitenModell::Klassisch);
    }
    SUBCASE("eine gezupfte Saite schreibt Pluck, ein Instrument ohne Tastatur keinen Block")
    {
        Instrument instrument;
        instrument.schichten[0].schwingung = Schwingung::Saite;
        instrument.schichten[0].saitenModell = SaitenModell::Wellenleiter;
        const QJsonObject json = alsJson(instrument);
        const QJsonObject schicht = ersteSchicht(json);
        CHECK(schicht.value(QStringLiteral("excitation")).toString() == QStringLiteral("pluck"));
        CHECK_FALSE(schicht.contains(QStringLiteral("core")));
        CHECK_FALSE(schicht.contains(QStringLiteral("coreParams")));
        CHECK_FALSE(json.contains(QStringLiteral("keyboard")));
        const Instrument gelesen = zurueck(instrument);
        CHECK_FALSE(istKernSchicht(gelesen.schichten[0]));
        CHECK(gelesen.schichten[0].saitenModell == SaitenModell::Wellenleiter);
        CHECK_FALSE(gelesen.tastatur.has_value());
        // Classic schreibt weiter nur `damping`, wie bis Fassung 15
        Instrument klassisch;
        klassisch.schichten[0].schwingung = Schwingung::Saite;
        const QJsonObject k = ersteSchicht(alsJson(klassisch));
        CHECK_FALSE(k.contains(QStringLiteral("excitation")));
    }
    SUBCASE("die Tastatur in ihren Grenzen")
    {
        QJsonObject o = tastaturAlsJson(Tastatur{});
        o[QStringLiteral("range")] = QJsonArray{40, 30};  // hoechste unter tiefster
        o[QStringLiteral("undampedFrom")] = 70;           // aus Fassung 20: wird ueberlesen
        o[QStringLiteral("bassSustainBelow")] = 200;
        o[QStringLiteral("pedals")] = QJsonObject{{QStringLiteral("left"), QStringLiteral("sustain")},
                                                  {QStringLiteral("middle"), QStringLiteral("wurst")}};
        const Tastatur t = tastaturAusJson(o);
        CHECK(t.tiefste == 40);
        CHECK(t.hoechste == 40);
        CHECK(t.bassSustainBis == 41);                 // bis hoechste + 1
        CHECK(t.links == PedalArt::Keins);             // Sustain gehoert nach rechts
        CHECK(t.mitte == PedalArt::Keins);             // unbekannt
        CHECK(t.rechts == PedalArt::Sustain);          // fehlt: die Vorgabe
        CHECK_FALSE(tastaturAlsJson(t).contains(QStringLiteral("undampedFrom")));
        for (const PedalArt art : {PedalArt::UnaCorda, PedalArt::Leise})
            CHECK(pedalErlaubt(PedalPlatz::Links, art));
        CHECK_FALSE(pedalErlaubt(PedalPlatz::Mitte, PedalArt::UnaCorda));
        CHECK(pedalErlaubt(PedalPlatz::Rechts, PedalArt::Keins));
    }
    SUBCASE("die zweite Ebene und der Stoss je Saite in ihren Grenzen (P7 a, US4 a)")
    {
        // ausserhalb der Grenzen: γ 0 … 90°, δ 0 … 20 Cent, h 0 … 1, u 0 … 1
        HammerSaite s = hammerSaiteVorgabe();
        s.polarisationWinkelGrad = kurve({{21, -5.0}, {108, 120.0}});
        s.polarisationCent = kurve({{21, -1.0}, {108, 30.0}});
        s.waagrechtVerhaeltnis = kurve({{21, -0.5}, {108, 2.0}});
        s.stossLeise = kurve({{21, -0.2}, {108, 1.5}});
        s.stossLaut = kurve({{21, 2.0}, {108, -1.0}});
        richteHammerSaite(s);
        CHECK(s.polarisationWinkelGrad == kurve({{21, 0.0}, {108, 90.0}}));
        CHECK(s.polarisationCent == kurve({{21, 0.0}, {108, 20.0}}));
        CHECK(s.waagrechtVerhaeltnis == kurve({{21, 0.0}, {108, 1.0}}));
        CHECK(s.stossLeise == kurve({{21, 0.0}, {108, 1.0}}));
        CHECK(s.stossLaut == kurve({{21, 1.0}, {108, 0.0}}));
    }
    SUBCASE("eine Fassung von morgen liest dieses Programm nicht")
    {
        QJsonObject json = alsJson(fluegel());
        json[QStringLiteral("version")] = kInstrumentFassung + 1;
        Instrument gelesen;
        bool zuNeu = false;
        CHECK_FALSE(ausJson(json, gelesen, &zuNeu));
        CHECK(zuNeu);
    }
}

TEST_CASE("Loop Hammer-Saite 31a Anschlag")
{
    CHECK(anschlagStufe(1.0) == 0);
    CHECK(anschlagStufe(0.5) == -6);   // −6,02 dB
    CHECK(anschlagStufe(0.1) == -20);
    CHECK(anschlagStufe(0.001) == kAnschlagStufeMin);  // −60 dB: die leiseste
    CHECK(anschlagStufe(0.0) == kAnschlagStufeMin);
    CHECK(anschlagStufe(1.5) == 0);
    CHECK(hammerTempo(0) == doctest::Approx(5.0));
    CHECK(hammerTempo(-20) == doctest::Approx(0.5));
    CHECK(hammerTempo(kAnschlagStufeMin) == doctest::Approx(0.05));
    // der Rest gleicht genau aus: Stufe mal Rest ist die Lautstaerke der Note
    for (const double laut : {1.0, 0.8, 0.5, 0.3, 0.07, 0.001})
    {
        CAPTURE(laut);
        const int stufe = anschlagStufe(laut);
        CHECK(anschlagRest(laut, stufe) * std::pow(10.0, stufe / 20.0) == doctest::Approx(laut));
        if (laut >= 0.01) CHECK(std::abs(20.0 * std::log10(anschlagRest(laut, stufe))) <= 0.5 + 1e-9);
    }

    SUBCASE("im Namen des Tons")
    {
        TonAngabe ton;
        ton.instrument = QStringLiteral("grand_piano");
        ton.hoehe = 60;
        ton.dauerMs = 500;
        CHECK(tonKlang(ton) == QStringLiteral("inst:grand_piano:60:500"));  // Stufe 0: kein Zusatz
        ton.anschlagStufe = -6;
        ton.teil = TonTeil::Eigen;
        ton.endeMs = 800;
        const QString name = tonKlang(ton);
        CHECK(name == QStringLiteral("inst:grand_piano:60:500:p2:a-6:e800"));
        TonAngabe zerlegt;
        REQUIRE(zerlegeTon(name, zerlegt));
        CHECK(zerlegt == ton);
        // ein Pfad mit Doppelpunkt davor stoert nicht
        ton.instrument = QStringLiteral("C:/a-1/fluegel.lvinst");
        REQUIRE(zerlegeTon(tonKlang(ton), zerlegt));
        CHECK(zerlegt == ton);
        // `:v` bleibt der Verlauf einer Kette
        ton.verlauf = {TonWechsel{240, 62, false}};
        REQUIRE(zerlegeTon(tonKlang(ton), zerlegt));
        CHECK(zerlegt == ton);
        CHECK_FALSE(zerlegeTon(QStringLiteral("inst:grand_piano:60:500:a-x"), zerlegt));
    }
}

TEST_CASE("Loop Hammer-Saite 31a keine gegriffene Saite, klingt seit 31b")
{
    const Instrument instrument = fluegel();
    CHECK(stufeImNamen(instrument));
    CHECK(istKernSchicht(instrument.schichten[0]));
    CHECK_FALSE(istSaitenSchicht(instrument.schichten[0]));  // keine gegriffene Saite
    CHECK_FALSE(folgtDerSaite(instrument.schichten[0]));
    const Klang klang = rendereTon(instrument, 60, 300.0, 48000);
    REQUIRE_FALSE(klang.daten.empty());
    CHECK(std::any_of(klang.daten.begin(), klang.daten.end(), [](float w) { return w != 0.0f; }));
    Instrument aus = instrument;
    aus.schichten[0].an = false;
    CHECK_FALSE(stufeImNamen(aus));
}
