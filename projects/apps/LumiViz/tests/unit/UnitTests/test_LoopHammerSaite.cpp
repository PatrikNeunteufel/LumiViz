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
 * Ohne Ton, ohne Engine, ohne Dateien aus dem Repo, ohne Qt-Anwendung.
 *
 * - Kurven ueber die Tastatur (HK1 a): Randwert, Abweichung je Taste, monoton kubisch
 *   ohne Ueberschwingen, die logarithmische Skala, Richten.
 * - Die Vorgaben: Hammer und Saiten nach Chabassier & Durufle RT-0425 (HT1 a, HT2 a), B nach
 *   Rigaud Gl. 5, der Chor.
 * - Die Datei: Fassung 16, `excitation`, `stringModel` `modal`, `hammerString`,
 *   `keyboard`; nur die Paare aus HA2 a; die Tastatur in ihren Grenzen.
 * - Die Anschlagstaerke (HK2 a): Stufen, Hammergeschwindigkeit, Rest, `:a<stufe>`.
 * - Bis 31b schweigt die Hammer-Saite; sie ist keine gegriffene Saite.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/HammerSaite.hpp"
#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"

#include <QJsonArray>
#include <QJsonObject>

#include <algorithm>
#include <cmath>

using namespace lumi::loop;

namespace
{

Tastenkurve kurve(std::initializer_list<TastenWert> punkte)
{
    Tastenkurve k;
    k.punkte.assign(punkte.begin(), punkte.end());
    return k;
}

/// Ein Instrument mit einer Hammer-Saite und einer Tastatur.
Instrument fluegel()
{
    Instrument instrument;
    instrument.titel = QStringLiteral("Test grand");
    Schicht& s = instrument.schichten[0];
    s.schwingung = Schwingung::Saite;
    s.saitenModell = SaitenModell::Modal;
    s.anregung = Anregung::Hammer;
    instrument.tastatur = Tastatur{};
    return instrument;
}

Instrument zurueck(const Instrument& instrument)
{
    Instrument gelesen;
    REQUIRE(ausJson(alsJson(instrument), gelesen));
    return gelesen;
}

}  // namespace

TEST_CASE("Loop Hammer-Saite 31a Kurve")
{
    SUBCASE("Randwert, Abweichung, ohne Punkte")
    {
        const Tastenkurve k = kurve({{40, 2.0}, {80, 4.0}});
        CHECK(wertAn(k, 21, KurvenSkala::Linear) == doctest::Approx(2.0));
        CHECK(wertAn(k, 108, KurvenSkala::Linear) == doctest::Approx(4.0));
        CHECK(wertAn(k, 60, KurvenSkala::Linear) == doctest::Approx(3.0));  // zwei Punkte: die Gerade
        Tastenkurve mit = k;
        mit.abweichungen = {{60, 9.0}};
        CHECK(wertAn(mit, 60, KurvenSkala::Linear) == doctest::Approx(9.0));
        CHECK(wertAn(mit, 61, KurvenSkala::Linear) == doctest::Approx(3.05));
        CHECK(wertAn(Tastenkurve{}, 60, KurvenSkala::Linear) == 0.0);
    }
    SUBCASE("monoton kubisch: trifft die Punkte, schwingt nicht ueber")
    {
        // ein Sprung mit flachen Stuecken davor und danach — ein gewoehnlicher Spline
        // schwaenge hier ueber 1 und unter 0
        const Tastenkurve k = kurve({{21, 0.0}, {40, 0.0}, {45, 1.0}, {108, 1.0}});
        for (const TastenWert& p : k.punkte) CHECK(wertAn(k, p.taste, KurvenSkala::Linear) == doctest::Approx(p.wert));
        double vorher = -1.0;
        for (int t = 21; t <= 108; ++t)
        {
            const double w = wertAn(k, t, KurvenSkala::Linear);
            CHECK(w >= 0.0);
            CHECK(w <= 1.0);
            CHECK(w >= vorher - 1e-12);  // steigt nur
            vorher = w;
        }
        // glatt: zwischen 40 und 45 kein gerader Weg
        CHECK(wertAn(k, 41, KurvenSkala::Linear) < 0.2);
        CHECK(wertAn(k, 44, KurvenSkala::Linear) > 0.8);
    }
    SUBCASE("logarithmisch: eine Exponentialfunktion aus wenigen Punkten genau")
    {
        // eine Exponentialfunktion (wie Q0 bei Stulov) ist in ln eine Gerade — monoton kubisch gibt
        // sie genau wieder
        const auto exponentiell = [](int n) { return 183.0 * std::exp(0.045 * n); };
        const Tastenkurve k = kurve({{21, exponentiell(1)}, {69, exponentiell(49)}, {108, exponentiell(88)}});
        for (int t = 21; t <= 108; t += 7)
            CHECK(wertAn(k, t, KurvenSkala::Logarithmisch) == doctest::Approx(exponentiell(t - 20)).epsilon(1e-9));
        // in den Werten selbst dagegen laege sie dazwischen zu hoch (gemessen: gut 6 %)
        CHECK(wertAn(k, 45, KurvenSkala::Linear) > exponentiell(25) * 1.05);
    }
    SUBCASE("richten: sortiert, je Taste einmal, Grenzen, Ersatz")
    {
        Tastenkurve k = kurve({{80, 5.0}, {10, 1.0}, {80, 7.0}, {60, std::nan("")}, {200, 50.0}});
        richteKurve(k, 0.0, 10.0, kurve({{21, 3.0}}));
        REQUIRE(k.punkte.size() == 3);
        CHECK(k.punkte[0] == TastenWert{21, 1.0});
        CHECK(k.punkte[1] == TastenWert{80, 5.0});  // der erste an 80 bleibt
        CHECK(k.punkte[2] == TastenWert{108, 10.0});
        Tastenkurve leer;
        richteKurve(leer, 0.0, 10.0, kurve({{21, 3.0}}));
        CHECK(leer.punkte == std::vector<TastenWert>{{21, 3.0}});
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

    const HammerSaite& v = hammerSaiteVorgabe();
    for (const int taste : {21, 27, 33, 39, 45, 51, 57, 63, 69, 75, 81, 87, 93, 99, 105, 108})
    {
        CHECK(wertAn(v.hammerMasseG, taste, KurvenSkala::Linear) == doctest::Approx(rtHammerMasseG(taste - 20)));
        CHECK(wertAn(v.hammerExponent, taste, KurvenSkala::Linear) == doctest::Approx(rtHammerExponent(taste - 20)));
        CHECK(wertAn(v.hammerSteife, taste, KurvenSkala::Logarithmisch) == doctest::Approx(rtHammerSteife(taste - 20)));
    }
    // zwischen den Stuetzpunkten: die Masse genau (eine Gerade), p und Q0 nah, B nah
    for (int taste = 21; taste <= 108; ++taste)
    {
        CHECK(wertAn(v.hammerMasseG, taste, KurvenSkala::Linear) == doctest::Approx(rtHammerMasseG(taste - 20)));
        CHECK(wertAn(v.hammerExponent, taste, KurvenSkala::Linear) == doctest::Approx(rtHammerExponent(taste - 20)).epsilon(0.01));
        CHECK(wertAn(v.hammerSteife, taste, KurvenSkala::Logarithmisch) == doctest::Approx(rtHammerSteife(taste - 20)).epsilon(0.02));
        CHECK(wertAn(v.inharmonizitaet, taste, KurvenSkala::Logarithmisch) ==
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
        CHECK(wertAn(v.saitenLaengeM, z.taste, KurvenSkala::Logarithmisch) == doctest::Approx(z.laenge).epsilon(0.02));
        CHECK(wertAn(v.saitenMasseGm, z.taste, KurvenSkala::Logarithmisch) ==
              doctest::Approx(z.rho * flaeche * 1e3).epsilon(0.02));
        CHECK(wertAn(v.laengsVerhaeltnis, z.taste, KurvenSkala::Linear) ==
              doctest::Approx(std::sqrt(2.02e11 * flaeche / z.t0)).epsilon(0.02));
    }
    CHECK(wertAn(v.anschlagStelle, 60, KurvenSkala::Linear) == doctest::Approx(0.12));
    // der Chor (vorlaeufig): eine, zwei, drei Saiten
    CHECK(chorAn(v, 21) == 1);
    CHECK(chorAn(v, 30) == 1);
    CHECK(chorAn(v, 31) == 2);
    CHECK(chorAn(v, 48) == 2);
    CHECK(chorAn(v, 49) == 3);
    CHECK(chorAn(v, 108) == 3);
    // eine neue Schicht traegt die Vorgaben schon
    CHECK(Schicht{}.hammerSaite == v);
}

TEST_CASE("Loop Hammer-Saite 31a Datei")
{
    SUBCASE("die Hammer-Saite und die Tastatur hin und zurueck, immer geschrieben")
    {
        Instrument instrument = fluegel();
        HammerSaite& h = instrument.schichten[0].hammerSaite;
        h.verstimmungCent.abweichungen = {{60, 3.5}};
        h.chor = kurve({{21, 2.0}, {108, 3.0}});
        h.resonanzboden.anteil = 0.4;
        instrument.tastatur->ungedaempftAb = 89;
        instrument.tastatur->links = PedalArt::Leise;
        instrument.tastatur->mitte = PedalArt::Moderator;

        const QJsonObject json = alsJson(instrument);
        CHECK(json.value(QStringLiteral("version")).toInt() == 16);
        const QJsonObject schicht = json.value(QStringLiteral("layers")).toArray().first().toObject();
        CHECK(schicht.value(QStringLiteral("stringModel")).toString() == QStringLiteral("modal"));
        CHECK(schicht.value(QStringLiteral("excitation")).toString() == QStringLiteral("hammer"));
        const QJsonObject hammer = schicht.value(QStringLiteral("hammerString")).toObject();
        for (const char* feld : {"inharmonicity", "decay", "highDecay", "strings", "detune", "hammerMass", "hammerStiffness",
                                 "hammerExponent", "strikePosition", "damper", "stringLength", "stringMass",
                                 "longitudinalRatio"})
        {
            CAPTURE(feld);
            const QJsonObject k = hammer.value(QLatin1String(feld)).toObject();
            CHECK(k.value(QStringLiteral("points")).isArray());
            CHECK(k.value(QStringLiteral("overrides")).isArray());  // auch leer
        }
        CHECK(hammer.value(QStringLiteral("soundboard")).toObject().value(QStringLiteral("amount")).toDouble() == 0.4);
        CHECK_FALSE(schicht.contains(QStringLiteral("pluckPosition")));  // nur was die Hammer-Saite traegt
        const QJsonObject tastatur = json.value(QStringLiteral("keyboard")).toObject();
        CHECK(tastatur.value(QStringLiteral("range")).toArray() == QJsonArray{21, 108});
        CHECK(tastatur.value(QStringLiteral("undampedFrom")).toInt() == 89);
        const QJsonObject pedale = tastatur.value(QStringLiteral("pedals")).toObject();
        CHECK(pedale.value(QStringLiteral("left")).toString() == QStringLiteral("soft"));
        CHECK(pedale.value(QStringLiteral("middle")).toString() == QStringLiteral("practice"));
        CHECK(pedale.value(QStringLiteral("right")).toString() == QStringLiteral("sustain"));

        const Instrument gelesen = zurueck(instrument);
        CHECK(gelesen.schichten[0].hammerSaite == instrument.schichten[0].hammerSaite);
        CHECK(gelesen.tastatur == instrument.tastatur);
        CHECK(gelesen == instrument);
    }
    SUBCASE("eine gezupfte Saite schreibt Pluck, ein Instrument ohne Tastatur keinen Block")
    {
        Instrument instrument;
        instrument.schichten[0].schwingung = Schwingung::Saite;
        instrument.schichten[0].saitenModell = SaitenModell::Wellenleiter;
        const QJsonObject json = alsJson(instrument);
        const QJsonObject schicht = json.value(QStringLiteral("layers")).toArray().first().toObject();
        CHECK(schicht.value(QStringLiteral("excitation")).toString() == QStringLiteral("pluck"));
        CHECK_FALSE(schicht.contains(QStringLiteral("hammerString")));
        CHECK_FALSE(json.contains(QStringLiteral("keyboard")));
        const Instrument gelesen = zurueck(instrument);
        CHECK(gelesen.schichten[0].anregung == Anregung::Zupfen);
        CHECK(gelesen.schichten[0].saitenModell == SaitenModell::Wellenleiter);
        CHECK_FALSE(gelesen.tastatur.has_value());
        // Classic schreibt weiter nur `damping`, wie bis Fassung 15
        Instrument klassisch;
        klassisch.schichten[0].schwingung = Schwingung::Saite;
        const QJsonObject k = alsJson(klassisch).value(QStringLiteral("layers")).toArray().first().toObject();
        CHECK_FALSE(k.contains(QStringLiteral("excitation")));
    }
    SUBCASE("nur die Paare aus HA2 a")
    {
        Schicht s;
        s.schwingung = Schwingung::Saite;
        s.saitenModell = SaitenModell::Wellenleiter;
        s.anregung = Anregung::Hammer;  // Hammer mit Waveguide gibt es nicht
        richteSchicht(s);
        CHECK(s.saitenModell == SaitenModell::Modal);
        Schicht t;
        t.schwingung = Schwingung::Saite;
        t.saitenModell = SaitenModell::Modal;  // Modal mit Pluck gibt es nicht
        richteSchicht(t);
        CHECK(t.anregung == Anregung::Hammer);
        // aus der Datei ebenso
        QJsonObject json = alsJson(fluegel());
        QJsonArray schichten = json.value(QStringLiteral("layers")).toArray();
        QJsonObject schicht = schichten.first().toObject();
        schicht.remove(QStringLiteral("excitation"));
        schicht.remove(QStringLiteral("hammerString"));  // fehlt: die Vorgaben
        schichten[0] = schicht;
        json[QStringLiteral("layers")] = schichten;
        Instrument gelesen;
        REQUIRE(ausJson(json, gelesen));
        CHECK(istHammerSchicht(gelesen.schichten[0]));
        CHECK(gelesen.schichten[0].anregung == Anregung::Hammer);
        CHECK(gelesen.schichten[0].hammerSaite == hammerSaiteVorgabe());
    }
    SUBCASE("die Tastatur in ihren Grenzen")
    {
        QJsonObject o = tastaturAlsJson(Tastatur{});
        o[QStringLiteral("range")] = QJsonArray{40, 30};  // hoechste unter tiefster
        o[QStringLiteral("undampedFrom")] = 200;
        o[QStringLiteral("pedals")] = QJsonObject{{QStringLiteral("left"), QStringLiteral("sustain")},
                                                  {QStringLiteral("middle"), QStringLiteral("wurst")}};
        const Tastatur t = tastaturAusJson(o);
        CHECK(t.tiefste == 40);
        CHECK(t.hoechste == 40);
        CHECK(t.ungedaempftAb == 41);                  // jede Taste hat einen Daempfer
        CHECK(t.links == PedalArt::Keins);             // Sustain gehoert nach rechts
        CHECK(t.mitte == PedalArt::Keins);             // unbekannt
        CHECK(t.rechts == PedalArt::Sustain);          // fehlt: die Vorgabe
        for (const PedalArt art : {PedalArt::UnaCorda, PedalArt::Leise})
            CHECK(pedalErlaubt(PedalPlatz::Links, art));
        CHECK_FALSE(pedalErlaubt(PedalPlatz::Mitte, PedalArt::UnaCorda));
        CHECK(pedalErlaubt(PedalPlatz::Rechts, PedalArt::Keins));
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

TEST_CASE("Loop Hammer-Saite 31a schweigt bis 31b")
{
    const Instrument instrument = fluegel();
    CHECK(hatHammer(instrument));
    CHECK(istHammerSchicht(instrument.schichten[0]));
    CHECK_FALSE(istSaitenSchicht(instrument.schichten[0]));  // keine gegriffene Saite
    CHECK_FALSE(folgtDerSaite(instrument.schichten[0]));
    const Klang klang = rendereTon(instrument, 60, 300.0, 48000);
    REQUIRE_FALSE(klang.daten.empty());
    CHECK(std::all_of(klang.daten.begin(), klang.daten.end(), [](float w) { return w == 0.0f; }));
    Instrument aus = instrument;
    aus.schichten[0].an = false;
    CHECK_FALSE(hatHammer(aus));
}
