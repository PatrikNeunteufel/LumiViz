/**
 ****************************************************************************************
 * @file   test_LoopSpielweiseKlang.cpp
 * @brief  Spielweisen, Stufe 24c: der Klang — Up stroke, Palm mute, Tapping, Harmonic an
 *         den Saiten *Extended KS* und *Waveguide*, gemessen am gerechneten Ton
 *         (docs/audio/Saiten_Konzept.md §5o; Entscheide Patrik S134: S-W6 a, S-W7 a, S-W8 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Qt-Anwendung. Gemessen: ohne Spielweise Bit fuer Bit derselbe
 * Ton; Up dreht das Vorzeichen um und nimmt *Up level*; Palm mute verkuerzt die Klingdauer
 * wie ein Punktdaempfer 40 mm vor dem Steg (die Formel aus dem Recherche-Bericht S134, §1.4)
 * und die Hoehen mehr als den Grundton; ein Flageolett laesst nur die Vielfachen von n
 * klingen, n = 5 rund 14 Cent unter der gleichstufigen Note; Tapping ist weicher und nimmt
 * *Tap level*. Andere Schichten bleiben, wie sie sind. `LUMIVIZ_24C_WAV=<Ordner>` legt
 * Hoerproben ab.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Aufnahme.hpp"  // schreibeWav
#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"
#include "loop/Saiten.hpp"
#include "loop/Sample.hpp"  // merkeSampleDaten

#include "LoopOhneEngine.hpp"  // repoWurzel
#include "SaitenMessung.hpp"   // betrag, spitze, cent, db, teilton

#include <QDir>

#include <algorithm>
#include <cmath>
#include <vector>

using namespace lumi::loop;
using namespace saitentest;

namespace
{

constexpr int kRate = kSaitenRate;

/// Eine Gitarre (Standardstimmung, 648 mm) mit einer Saiten-Schicht des Verfahrens.
Instrument saite(SaitenModell modell)
{
    Instrument instrument;
    instrument.saiten = Saitensatz{stimmungsvorlagen()[0].leer, 0, Saitenwahl::JeKlang, 648.0};
    Schicht& s = instrument.schichten[0];
    s.schwingung = Schwingung::Saite;
    s.saitenModell = modell;
    s.anstiegMs = 1.0;
    s.ausklangMs = 200.0;
    s.hoehenDaempfung = 0.0;
    s.steifheit = 0.0;
    s.anschlagHaerte = 1.0;
    s.klingSekunden = 3.2;
    s.saitenUnterschied = 0.0;
    return instrument;
}

TonAngabe ton(int hoehe, int saiteNr, int dauerMs = 2000)
{
    TonAngabe t;
    t.instrument = QStringLiteral("probe");
    t.hoehe = hoehe;
    t.dauerMs = dauerMs;
    t.saite = saiteNr;
    return t;
}

/// Die Abklingrate des Betrags bei `hz` zwischen zwei Fenstern, als T60 in Sekunden.
double t60(const Klang& klang, double hz, double vonS, double bisS, double langS = 0.1)
{
    const double a = betrag(klang, hz, vonS, langS);
    const double b = betrag(klang, hz, bisS, langS);
    const double dbJeS = db(b / a) / (bisS - vonS);
    return dbJeS < 0.0 ? -60.0 / dbJeS : 1e9;
}

double effektiv(const Klang& klang, double vonS, double langS)
{
    const auto von = static_cast<std::int64_t>(vonS * kRate);
    const auto lang = static_cast<std::int64_t>(langS * kRate);
    double summe = 0.0;
    std::int64_t n = 0;
    for (; n < lang && von + n < klang.frames(); ++n)
    {
        const double x = klang.daten[static_cast<std::size_t>(2 * (von + n))];
        summe += x * x;
    }
    return n > 0 ? std::sqrt(summe / static_cast<double>(n)) : 0.0;
}

}  // namespace

TEST_SUITE("LoopSpielweiseKlang")
{
    TEST_CASE("Loop Spielweise Klang: ohne Spielweise Bit fuer Bit derselbe Ton; Up dreht das Vorzeichen und nimmt Up level")
    {
        for (const SaitenModell modell : {SaitenModell::ErweiterterKs, SaitenModell::Wellenleiter})
        {
            CAPTURE(static_cast<int>(modell));
            Instrument instrument = saite(modell);
            instrument.schichten[0].aufschlagLaut = 0.5;
            const TonAngabe ab = ton(57, 2, 800);
            const Klang ohne = rendereTon(instrument, ab, kRate);
            // dieselbe Rechnung wie bis Stufe 24b: die Tonhoehe allein, mit derselben Saite
            TonAngabe ohneSaite = ab;
            ohneSaite.saite = -1;
            CHECK(rendereTon(instrument, ohneSaite, kRate).daten == rendereTon(instrument, 57, 800, kRate).daten);

            TonAngabe auf = ab;
            auf.richtung = Richtung::Auf;
            const Klang hoch = rendereTon(instrument, auf, kRate);
            REQUIRE(hoch.daten.size() == ohne.daten.size());
            double abweichung = 0.0;
            for (std::size_t i = 0; i < ohne.daten.size(); ++i)
                abweichung = std::max(abweichung, std::abs(hoch.daten[i] + 0.5 * ohne.daten[i]));
            CHECK(abweichung < 1e-6);
        }

        // --- eine Schicht, die keine Saite Extended KS / Waveguide ist, bleibt, wie sie ist (S-W6 a)
        Instrument sinus;
        sinus.schichten[0].schwingung = Schwingung::Sinus;
        TonAngabe alles = ton(57, -1, 500);
        alles.richtung = Richtung::Auf;
        alles.artikulationen = kPalmMute | kTapping;
        CHECK(rendereTon(sinus, alles, kRate).daten == rendereTon(sinus, 57, 500, kRate).daten);
        Instrument klassisch = saite(SaitenModell::Klassisch);
        CHECK(rendereTon(klassisch, alles, kRate).daten == rendereTon(klassisch, 57, 500, kRate).daten);
    }

    TEST_CASE("Loop Spielweise Klang: Palm mute — ein Punktdaempfer 40 mm vor dem Steg, die Hoehen zuerst")
    {
        for (const SaitenModell modell : {SaitenModell::ErweiterterKs, SaitenModell::Wellenleiter})
        {
            CAPTURE(static_cast<int>(modell));
            const Instrument instrument = saite(modell);
            // A3 auf der D-Saite (7. Bund): schwingend 648 · 2^(−7/12) = 432,5 mm
            const TonAngabe normal = ton(57, 2);
            TonAngabe gedaempft = normal;
            gedaempft.artikulationen = kPalmMute;
            const Klang offen = rendereTon(instrument, normal, kRate);
            const Klang mute = rendereTon(instrument, gedaempft, kRate);
            const double f1 = tonHz(57);

            // die Formel (Bericht §1.4): T60 = 6,91 / (6,91/Ring time + 2 f1 rho sin²(pi x/L)), rho = Mute²
            const double x = 40.0 / (648.0 * std::exp2(-7.0 / 12.0));
            const double rho = 0.6 * 0.6;
            const double erwartet = 6.91 / (6.91 / 3.2 + 2.0 * f1 * rho * std::pow(std::sin(kPi * x), 2));
            const double gemessen = t60(mute, f1, 0.05, 0.35);
            MESSAGE("T60 bei A3: offen " << t60(offen, f1, 0.1, 1.0) << " s, Palm mute " << gemessen << " s (Formel " << erwartet
                                          << " s)");
            CHECK(t60(offen, f1, 0.1, 1.0) == doctest::Approx(3.2).epsilon(0.1));
            CHECK(gemessen == doctest::Approx(erwartet).epsilon(0.2));
            // die Hoehen zuerst: der vierte Teilton verliert in derselben Zeit mehr als der Grundton
            const double teilVier = db(betrag(mute, 4.0 * f1, 0.25, 0.1) / betrag(mute, 4.0 * f1, 0.05, 0.1));
            const double grund = db(betrag(mute, f1, 0.25, 0.1) / betrag(mute, f1, 0.05, 0.1));
            CHECK(teilVier < grund - 6.0);
            // der Anschlag bleibt: die ersten 20 ms in der gemessenen Spanne gegen ohne (−4 … +6 dB,
            // IDMT-SMT-Guitar, Bericht S134 §1.2) — die Hand nimmt das Ausklingen, nicht den Anschlag
            const double anschlag = db(effektiv(mute, 0.0, 0.02) / effektiv(offen, 0.0, 0.02));
            MESSAGE("die ersten 20 ms mit Palm mute: " << anschlag << " dB");
            CHECK(anschlag > -4.0);
            CHECK(anschlag < 6.0);
        }

        // --- Mute 0: kein Daempfer, derselbe Ton
        Instrument ohneHand = saite(SaitenModell::Wellenleiter);
        ohneHand.schichten[0].handDaempfung = 0.0;
        TonAngabe gedaempft = ton(57, 2, 500);
        gedaempft.artikulationen = kPalmMute;
        CHECK(rendereTon(ohneHand, gedaempft, kRate).daten == rendereTon(ohneHand, ton(57, 2, 500), kRate).daten);

        // --- ein Bass klingt gedaempft laenger als eine Gitarre (S-W6 a): A1 auf 864 mm gegen A3 auf 648 mm
        Instrument bass = saite(SaitenModell::Wellenleiter);
        bass.saiten = Saitensatz{{28, 33, 38, 43}, 0, Saitenwahl::JeKlang, 864.0};
        TonAngabe tief = ton(33, 1);
        tief.artikulationen = kPalmMute;
        TonAngabe gitarreA = ton(57, 2);
        gitarreA.artikulationen = kPalmMute;
        const double bassT60 = t60(rendereTon(bass, tief, kRate), tonHz(33), 0.05, 0.6, 0.2);
        const double gitarreT60 = t60(rendereTon(saite(SaitenModell::Wellenleiter), gitarreA, kRate), tonHz(57), 0.05, 0.35);
        MESSAGE("Palm mute T60: Bass A1 " << bassT60 << " s, Gitarre A3 " << gitarreT60 << " s");
        CHECK(bassT60 > 2.0 * gitarreT60);
    }

    TEST_CASE("Loop Spielweise Klang: Harmonic — nur die Vielfachen von n klingen, n = 5 rund 14 Cent tief")
    {
        for (const SaitenModell modell : {SaitenModell::ErweiterterKs, SaitenModell::Wellenleiter})
        {
            CAPTURE(static_cast<int>(modell));
            const Instrument instrument = saite(modell);
            const double e2 = tonHz(40);

            // E3 als Flageolett n = 2 der tiefen E-Saite: der Grundton E2 ist nach 0,2 s fort, E3 klingt
            TonAngabe flageolett = ton(52, 0);
            flageolett.teilton = 2;
            const Klang h = rendereTon(instrument, flageolett, kRate);
            const Klang leer = rendereTon(instrument, ton(40, 0), kRate);
            const double grundRest = db(betrag(h, e2, 0.3, 0.2) / betrag(h, 2.0 * e2, 0.3, 0.2));
            const double grundLeer = db(betrag(leer, e2, 0.3, 0.2) / betrag(leer, 2.0 * e2, 0.3, 0.2));
            MESSAGE("E2 gegen E3 nach 0,3 s: Flageolett " << grundRest << " dB, leere Saite " << grundLeer << " dB");
            CHECK(grundRest < -40.0);
            CHECK(grundLeer > -20.0);
            CHECK(betrag(h, 2.0 * e2, 0.3, 0.2) > 0.1 * betrag(leer, 2.0 * e2, 0.3, 0.2));  // E3 bleibt

            // n = 5: G#4 klingt als fuenfter Teilton von E2 — 13,7 Cent unter der gleichstufigen Note
            TonAngabe fuenf = ton(68, 0);
            fuenf.teilton = 5;
            const Klang g = rendereTon(instrument, fuenf, kRate);
            const double gemessen = cent(spitze(g, 5.0 * e2, 30.0, 0.3, 0.5), tonHz(68));
            MESSAGE("n = 5: " << gemessen << " Cent gegen G#4");
            CHECK(gemessen == doctest::Approx(-13.7).epsilon(0.1));

            // Touch 0: kein Finger — der Ton der leeren Saite
            Instrument ohneFinger = instrument;
            ohneFinger.schichten[0].knotenDruck = 0.0;
            CHECK(rendereTon(ohneFinger, flageolett, kRate).daten == rendereTon(ohneFinger, ton(40, 0), kRate).daten);
        }
    }

    TEST_CASE("Loop Spielweise Klang: Tapping — am Bund, weich, Tap level")
    {
        for (const SaitenModell modell : {SaitenModell::ErweiterterKs, SaitenModell::Wellenleiter})
        {
            CAPTURE(static_cast<int>(modell));
            Instrument instrument = saite(modell);
            instrument.schichten[0].tappingLaut = 1.0;
            const TonAngabe gezupft = ton(57, 2, 800);
            TonAngabe getappt = gezupft;
            getappt.artikulationen = kTapping;
            const Klang t = rendereTon(instrument, getappt, kRate);
            const double f1 = tonHz(57);
            // am Bund und weich: gegen einen harten Zupfton an derselben Stelle (am Ende der Saite)
            // bleibt der Grundton, der achte Teilton (1,76 kHz) faellt um den Tiefpass bei 640 Hz
            Instrument amEnde = instrument;
            amEnde.schichten[0].anschlagOrt = kAnschlagOrtMin;
            const Klang p = rendereTon(amEnde, gezupft, kRate);
            const double hellGezupft = db(teilton(p, f1, 8) / teilton(p, f1, 1));
            const double hellGetappt = db(teilton(t, f1, 8) / teilton(t, f1, 1));
            MESSAGE("8. Teilton gegen Grundton: gezupft am Ende " << hellGezupft << " dB, getappt " << hellGetappt << " dB");
            CHECK(hellGetappt < hellGezupft - 10.0);
            // nicht an der Anschlagstelle der Schicht: der Ort von Pluck position wirkt beim Tapping nicht
            Instrument woanders = instrument;
            woanders.schichten[0].anschlagOrt = 0.25;
            CHECK(rendereTon(woanders, getappt, kRate).daten == t.daten);
            // Tap level: linear
            instrument.schichten[0].tappingLaut = 0.5;
            const Klang halb = rendereTon(instrument, getappt, kRate);
            REQUIRE(halb.daten.size() == t.daten.size());
            double abweichung = 0.0;
            for (std::size_t i = 0; i < t.daten.size(); ++i) abweichung = std::max(abweichung, std::abs(halb.daten[i] - 0.5 * t.daten[i]));
            CHECK(abweichung < 1e-6);
            // ein getippter Ton hat keine Richtung
            TonAngabe getapptAuf = getappt;
            getapptAuf.richtung = Richtung::Auf;
            CHECK(rendereTon(instrument, getapptAuf, kRate).daten == halb.daten);
        }
    }

    TEST_CASE("Loop Spielweise Klang: Tapping — Tap level gilt bei 220 Hz, tiefer lauter (S-W18 a)")
    {
        // der Faktor: Tap level · √(220 Hz / f1), hoechstens 1,5
        CHECK(tappingFaktor(0.75, 220.0) == doctest::Approx(0.75));
        CHECK(tappingFaktor(0.75, 110.0) == doctest::Approx(0.75 * std::sqrt(2.0)));
        CHECK(tappingFaktor(0.75, 440.0) == doctest::Approx(0.75 / std::sqrt(2.0)));
        CHECK(tappingFaktor(1.0, tonHz(28)) == doctest::Approx(kTappingFaktorMax));  // E1: 2,31 ohne Grenze
        CHECK(tappingFaktor(0.0, 55.0) == 0.0);
        CHECK(tappingFaktor(2.0, 220.0) == doctest::Approx(1.0));  // Tap level in 0 … 1

        // im gerechneten Ton: am Bass E1 liegen Tap level 0,9 und 1,0 beide an der Grenze — derselbe
        // Ton; an A3 (220 Hz) nicht. So wirkt die Tonhoehe der Saite, nicht nur Tap level.
        for (const SaitenModell modell : {SaitenModell::ErweiterterKs, SaitenModell::Wellenleiter})
        {
            CAPTURE(static_cast<int>(modell));
            Instrument bass = saite(modell);
            bass.saiten = Saitensatz{{28, 33, 38, 43}, 0, Saitenwahl::JeKlang, 864.0};
            TonAngabe tief = ton(28, 0, 500);
            tief.artikulationen = kTapping;
            bass.schichten[0].tappingLaut = 1.0;
            const Klang voll = rendereTon(bass, tief, kRate);
            bass.schichten[0].tappingLaut = 0.9;
            CHECK(rendereTon(bass, tief, kRate).daten == voll.daten);

            Instrument gitarre = saite(modell);
            TonAngabe a3 = ton(57, 2, 500);
            a3.artikulationen = kTapping;
            gitarre.schichten[0].tappingLaut = 1.0;
            const Klang a3Voll = rendereTon(gitarre, a3, kRate);
            gitarre.schichten[0].tappingLaut = 0.9;
            CHECK(rendereTon(gitarre, a3, kRate).daten != a3Voll.daten);

            // A2 (110 Hz) mit Tap level 0,5: der Faktor 0,707 — so laut wie mit 0,707 bei 1 mal 0,707
            Instrument mitte = saite(modell);
            TonAngabe a2 = ton(45, 1, 500);
            a2.artikulationen = kTapping;
            mitte.schichten[0].tappingLaut = 1.0 / std::sqrt(2.0);  // Faktor 1
            const Klang eins = rendereTon(mitte, a2, kRate);
            mitte.schichten[0].tappingLaut = 0.5;
            const Klang halb = rendereTon(mitte, a2, kRate);
            REQUIRE(halb.daten.size() == eins.daten.size());
            double abweichung = 0.0;
            for (std::size_t i = 0; i < eins.daten.size(); ++i)
                abweichung = std::max(abweichung, std::abs(halb.daten[i] - std::sqrt(0.5) * eins.daten[i]));
            CHECK(abweichung < 1e-6);
        }
    }

    TEST_CASE("Loop Spielweise Klang 25c: Legato — ein Ton, die Tonhoehe springt, die Saite wird neu angeregt")
    {
        // der Sprung: der lauteste Effektivwert (10 ms) in den 40 ms nach dem Wechsel gegen die 10 ms davor
        const auto sprung = [](const Klang& k, double wechselS) {
            double danach = 0.0;
            for (double s = wechselS; s < wechselS + 0.04; s += 0.002) danach = std::max(danach, effektiv(k, s, 0.01));
            return db(danach / effektiv(k, wechselS - 0.01, 0.01));
        };
        // Achtel bei 100 BPM: der Wechsel 300 ms nach dem Anschlag; A3 auf der D-Saite (7. Bund)
        const auto legato = [](int von, int nach) {
            TonAngabe t = ton(von, 2, 600);
            t.verlauf = {TonWechsel{300, nach, false}};
            return t;
        };
        for (const SaitenModell modell : {SaitenModell::ErweiterterKs, SaitenModell::Wellenleiter})
        {
            CAPTURE(static_cast<int>(modell));
            Instrument instrument = saite(modell);
            // --- die Tonhoehe: vorher A3, nachher B3 — ein Ton, kein zweiter Anschlag
            const Klang h = rendereTon(instrument, legato(57, 59), kRate);
            CHECK(std::abs(cent(spitze(h, tonHz(59), 60.0, 0.36, 0.2), tonHz(59))) < 3.0);
            CHECK(std::abs(cent(spitze(h, tonHz(57), 60.0, 0.1, 0.18), tonHz(57))) < 3.0);

            // --- ohne Anregung (Legato level 0) springt der Pegel nicht: nur die Laenge aendert sich
            instrument.schichten[0].legatoLaut = 0.0;
            const double ohne = sprung(rendereTon(instrument, legato(57, 59), kRate), 0.3);
            CHECK(std::abs(ohne) < 1.5);

            // --- die Bemessung (S-L11 a): Hammer-on 1, 2, 3 Halbtoene, Pull-off ebenso — der Median
            // des Sprungs bei der Vorgabe ist +3 dB (Untergrenze aus Mitsou 2023, Recherche 25 §1)
            const auto median = [&](double laut) {
                instrument.schichten[0].legatoLaut = laut;
                std::vector<double> spruenge;
                for (const int weg : {1, 2, 3})
                {
                    spruenge.push_back(sprung(rendereTon(instrument, legato(57, 57 + weg), kRate), 0.3));
                    spruenge.push_back(sprung(rendereTon(instrument, legato(57 + weg, 57), kRate), 0.3));
                }
                std::sort(spruenge.begin(), spruenge.end());
                MESSAGE("Legato level " << laut << ": Sprung Median " << 0.5 * (spruenge[2] + spruenge[3]) << " dB (" << spruenge.front()
                                        << " … " << spruenge.back() << ")");
                return 0.5 * (spruenge[2] + spruenge[3]);
            };
            CHECK(median(kLegatoLautVorgabe) == doctest::Approx(3.0).epsilon(0.1));
            CHECK(median(1.0) > median(0.2) + 4.0);  // der Regler wirkt
        }
    }

    TEST_CASE("Loop Spielweise Klang 25c: Slide und Bend — die Tonhoehe gleitet, der Pegel waechst nicht")
    {
        for (const SaitenModell modell : {SaitenModell::ErweiterterKs, SaitenModell::Wellenleiter})
        {
            CAPTURE(static_cast<int>(modell));
            Instrument instrument = saite(modell);
            // --- Slide A3 -> D4 (5 Halbtoene), Ankunft bei 500 ms: mit 20 ms je Halbton ab 400 ms
            TonAngabe gleiten = ton(57, 2, 900);
            gleiten.verlauf = {TonWechsel{500, 62, true}};
            const Klang s = rendereTon(instrument, gleiten, kRate);
            CHECK(std::abs(cent(spitze(s, tonHz(57), 60.0, 0.15, 0.2), tonHz(57))) < 3.0);
            CHECK(std::abs(cent(spitze(s, tonHz(62), 60.0, 0.55, 0.25), tonHz(62))) < 3.0);
            // bis zum Beginn des Gleitens derselbe Ton wie ohne Slide
            const Klang gerade = rendereTon(instrument, ton(57, 2, 900), kRate);
            double vorher = 0.0;
            for (std::size_t i = 0; i < static_cast<std::size_t>(2 * 0.39 * kRate); ++i)
                vorher = std::max(vorher, std::abs(static_cast<double>(s.daten[i] - gerade.daten[i])));
            CHECK(vorher < 1e-6);
            // kein Pegelgewinn ueber den Slide (S-L8 a: Auslenkungswellen g = 1, dazu der Mehrverlust)
            CHECK(effektiv(s, 0.52, 0.05) < effektiv(s, 0.33, 0.05));
            // ohne Buende glatt — im Slide ein anderer Verlauf, davor gleich
            Instrument bundlos = instrument;
            bundlos.saiten.bundlos = true;
            const Klang glatt = rendereTon(bundlos, gleiten, kRate);
            double imSlide = 0.0;
            for (std::size_t i = static_cast<std::size_t>(2 * 0.41 * kRate); i < static_cast<std::size_t>(2 * 0.49 * kRate); ++i)
                imSlide = std::max(imSlide, std::abs(static_cast<double>(s.daten[i] - glatt.daten[i])));
            CHECK(imSlide > 1e-4);

            // --- Bend um einen Ganzton: Anstieg 200 … 290 ms, dann gehalten — B3, und fast gleich laut
            TonAngabe bend = ton(57, 2, 900);
            bend.biegung = {TonBiegung{200, 0}, TonBiegung{290, 8}};
            const Klang b = rendereTon(instrument, bend, kRate);
            CHECK(std::abs(cent(spitze(b, tonHz(59), 60.0, 0.4, 0.3), tonHz(59))) < 5.0);
            const double lauter = db(effektiv(b, 0.4, 0.2) / effektiv(gerade, 0.4, 0.2));
            MESSAGE("Bend um einen Ganzton: " << lauter << " dB gegen den Ton ohne Bend (hergeleitet +0,5 dB, gemessen −0,3 dB)");
            // die Ring time im Halten (S-L14 a, S137): gebogen wie gegriffen auf derselben Hoehe
            const Klang gegriffen = rendereTon(instrument, ton(59, 2, 900), kRate);
            const double gebogenT60 = t60(b, tonHz(59), 0.4, 0.8);
            const double gegriffenT60 = t60(gegriffen, tonHz(59), 0.4, 0.8);
            MESSAGE("T60 im Halten: gebogen " << gebogenT60 << " s, B3 gegriffen " << gegriffenT60 << " s, A3 "
                                            << t60(gerade, tonHz(57), 0.4, 0.8) << " s");
            CHECK(gebogenT60 == doctest::Approx(gegriffenT60).epsilon(0.03));
            // ... und rund −0,5 dB: das Gesetz √(Verzoegerung) allein (S-L8 a)
            CHECK(lauter > -0.8);
            CHECK(lauter < -0.2);

            // --- S-L15 b: bis zwei Ganztoene — C#4 im Halten, die Ring time wie gegriffen
            TonAngabe weit = ton(57, 2, 900);
            weit.biegung = {TonBiegung{200, 0}, TonBiegung{290, 16}};
            const Klang w = rendereTon(instrument, weit, kRate);
            CHECK(std::abs(cent(spitze(w, tonHz(61), 60.0, 0.4, 0.3), tonHz(61))) < 5.0);
            const Klang cis = rendereTon(instrument, ton(61, 2, 900), kRate);
            const double weitT60 = t60(w, tonHz(61), 0.4, 0.8);
            const double cisT60 = t60(cis, tonHz(61), 0.4, 0.8);
            MESSAGE("Bend um zwei Ganztoene: " << db(effektiv(w, 0.4, 0.2) / effektiv(gerade, 0.4, 0.2)) << " dB, T60 gebogen "
                                               << weitT60 << " s, C#4 gegriffen " << cisT60 << " s");
            CHECK(weitT60 == doctest::Approx(cisT60).epsilon(0.03));
        }
    }

    TEST_CASE("Loop Spielweise Klang 25c-4: Follow string — jede Schwingungsart geht Legato und Bend mit, Own note bleibt")
    {
        // eine Aufnahme fuer die Sample-Schicht: ein Sinus bei A3, eine Sekunde
        const QString aufnahme = QStringLiteral("user:folgt_probe");
        {
            std::vector<float> sinus(static_cast<std::size_t>(2 * kRate));
            for (std::size_t i = 0; i < sinus.size() / 2; ++i)
                sinus[2 * i] = sinus[2 * i + 1] = static_cast<float>(0.5 * std::sin(2.0 * 3.14159265358979323846 * tonHz(57) * i / kRate));
            merkeSampleDaten(aufnahme, kRate, std::move(sinus));
        }
        // Achtel bei 100 BPM: der Wechsel 300 ms nach dem Anschlag
        TonAngabe legato = ton(57, 2, 600);
        legato.verlauf = {TonWechsel{300, 59, false}};
        TonAngabe bend = ton(57, 2, 900);
        bend.biegung = {TonBiegung{200, 0}, TonBiegung{290, 8}};
        TonAngabe gerade = ton(57, 2, 600);
        TonAngabe geradeLang = ton(57, 2, 900);

        for (const Schwingung art : {Schwingung::Sinus, Schwingung::Saege, Schwingung::Rechteck, Schwingung::Form, Schwingung::Sample,
                                     Schwingung::Saite})
        {
            CAPTURE(static_cast<int>(art));
            Instrument instrument = saite(SaitenModell::Klassisch);  // Classic: eine uebrige Schicht
            Schicht& s = instrument.schichten[0];
            s.schwingung = art;
            s.form = QStringLiteral("circle");
            s.sample = aufnahme;
            s.grundton = 57;
            s.halten = 1.0;
            s.saiteDaempfung = 0.9999;
            // --- Own note (Vorgabe): Legato und Bend aendern nichts, Bit fuer Bit
            REQUIRE_FALSE(s.folgtSaite);
            const Klang flach = rendereTon(instrument, gerade, kRate);
            CHECK(rendereTon(instrument, legato, kRate).daten == flach.daten);
            CHECK(rendereTon(instrument, bend, kRate).daten == rendereTon(instrument, geradeLang, kRate).daten);
            // --- Follow string ohne Verlauf: derselbe Ton wie bisher
            s.folgtSaite = true;
            CHECK(rendereTon(instrument, gerade, kRate).daten == flach.daten);
            // --- mit Legato: vorher A3, nachher B3, ein Ton (die Bandgrenze richtet sich nach dem
            // hoechsten Griff — davor deshalb nicht Bit fuer Bit)
            const Klang l = rendereTon(instrument, legato, kRate);
            REQUIRE(l.daten.size() == flach.daten.size());
            CHECK(std::abs(cent(spitze(l, tonHz(57), 60.0, 0.1, 0.18), tonHz(57))) < 5.0);
            CHECK(std::abs(cent(spitze(l, tonHz(59), 60.0, 0.32, 0.2), tonHz(59))) < 5.0);
            // --- mit Bend um einen Ganzton: B3 im Halten
            const Klang b = rendereTon(instrument, bend, kRate);
            CHECK(std::abs(cent(spitze(b, tonHz(59), 60.0, 0.4, 0.3), tonHz(59))) < 5.0);
        }

        // --- Rauschen hat keine Tonhoehe: ohne Filter bleibt es, mit Filter folgt der Filter
        Instrument rauschen = saite(SaitenModell::Klassisch);
        rauschen.schichten[0].schwingung = Schwingung::Rauschen;
        rauschen.schichten[0].folgtSaite = true;
        CHECK(rendereTon(rauschen, legato, kRate).daten == rendereTon(rauschen, gerade, kRate).daten);
        rauschen.schichten[0].reihe = {SchichtFilter{FilterArt::Tiefpass, 4.0, 0.0, 0.0}, SchichtHuelle{}};
        const Klang mitFilter = rendereTon(rauschen, legato, kRate);
        const Klang ohneVerlauf = rendereTon(rauschen, gerade, kRate);
        const auto wechsel = static_cast<std::size_t>(2 * 0.3 * kRate);
        CHECK(std::equal(ohneVerlauf.daten.begin(), ohneVerlauf.daten.begin() + static_cast<std::ptrdiff_t>(wechsel), mitFilter.daten.begin()));
        double danach = 0.0;
        for (std::size_t i = wechsel; i < mitFilter.daten.size(); ++i)
            danach = std::max(danach, std::abs(static_cast<double>(mitFilter.daten[i] - ohneVerlauf.daten[i])));
        CHECK(danach > 1e-4);
        vergissSampleDaten(aufnahme);
    }

    TEST_CASE("Loop Spielweise Klang 25c-4: in einer Kette rechnet der Ton der Saite, was ihr folgt; Own note spielt je Note")
    {
        Instrument drei = saite(SaitenModell::ErweiterterKs);
        Schicht folgt;
        folgt.schwingung = Schwingung::Sinus;
        folgt.folgtSaite = true;
        Schicht eigen;
        eigen.schwingung = Schwingung::Saege;
        drei.schichten = {drei.schichten[0], folgt, eigen};

        TonAngabe kette = ton(57, 2, 600);
        kette.verlauf = {TonWechsel{300, 59, false}};
        // --- der Ton der Saite (`:p1`): die Saite und der Sinus — wie ein Instrument aus beiden allein
        TonAngabe saitenTon = kette;
        saitenTon.teil = TonTeil::Folgend;
        Instrument beide = drei;
        beide.schichten.pop_back();
        CHECK(rendereTon(drei, saitenTon, kRate).daten == rendereTon(beide, kette, kRate).daten);
        // --- der Ton der Note (`:p2`): nur die Saege, und sie spielt ihre Note, ohne Verlauf
        TonAngabe notenTon = kette;
        notenTon.teil = TonTeil::Eigen;
        Instrument nurEigen = drei;
        nurEigen.schichten = {eigen};
        CHECK(rendereTon(drei, notenTon, kRate).daten == rendereTon(nurEigen, ton(57, 2, 600), kRate).daten);
        // --- folgt keine Schicht ausser der Saite, ist der Ton der Saite die Saite allein (wie 25c-3b)
        Instrument ohneFolgen = drei;
        ohneFolgen.schichten[1].folgtSaite = false;
        Instrument nurSaite = drei;
        nurSaite.schichten = {drei.schichten[0]};
        CHECK(rendereTon(ohneFolgen, saitenTon, kRate).daten == rendereTon(nurSaite, kette, kRate).daten);
    }

    TEST_CASE("Loop Spielweise Klang: Hoerproben (LUMIVIZ_24C_WAV)")
    {
        const QString ziel = qEnvironmentVariable("LUMIVIZ_24C_WAV");
        if (ziel.isEmpty())
        {
            MESSAGE("LUMIVIZ_24C_WAV nicht gesetzt — keine Hoerproben");
            return;
        }
        REQUIRE(QDir().mkpath(ziel));
        const QDir instrumente(looptest::repoWurzel().filePath(QStringLiteral("asset/loop/instruments")));
        struct Art
        {
            const char* name;
            Richtung richtung;
            int artikulationen;
            int teilton;
            int hoehe;
        };
        const Art arten[] = {{"normal", Richtung::Ab, 0, 1, 57},          {"up", Richtung::Auf, 0, 1, 57},
                             {"palm_mute", Richtung::Ab, kPalmMute, 1, 57}, {"tapping", Richtung::Ab, kTapping, 1, 57},
                             {"harmonic_n2", Richtung::Ab, 0, 2, 52},       {"harmonic_n3", Richtung::Ab, 0, 3, 59}};
        for (const char* name : {"guitar", "nylon_guitar", "jazz_guitar", "overdrive_guitar"})
        {
            Instrument instrument;
            CAPTURE(name);
            REQUIRE(ladeInstrument(instrumente.filePath(QLatin1String(name) + QStringLiteral(".lvinst")), instrument));
            for (const Art& art : arten)
            {
                // auf der tiefen E-Saite die Flageoletts, sonst die D-Saite
                TonAngabe t = ton(art.hoehe, art.teilton > 1 ? 0 : 2, 1500);
                t.richtung = art.richtung;
                t.artikulationen = art.artikulationen;
                t.teilton = art.teilton;
                const Klang klang = rendereTon(instrument, t, kRate);
                const QString datei = QDir(ziel).filePath(QStringLiteral("%1_%2.wav").arg(QLatin1String(name), QLatin1String(art.name)));
                CHECK(schreibeWav(datei, klang.daten, kRate));
            }
        }
    }
}
