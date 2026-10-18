/**
 ****************************************************************************************
 * @file   test_LoopSaiteKs.cpp
 * @brief  Saiteninstrumente, Stufe 23c: die Saite *Extended KS* — gemessen am
 *         gerechneten Ton (docs/audio/Saiten_Konzept.md; Entscheide Patrik S132:
 *         S-K1 a + b, S-K2 b, S-K3 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Qt-Anwendung. Jeder Regler hat eine bekannte Wirkung,
 * und die wird gemessen: die Tonhoehe, Ring time (der Grundton faellt in dieser Zeit
 * um 60 dB, gleich bei jeder Tonhoehe), Pluck position (bei einem Viertel fehlt der
 * vierte Teilton), High damping und Pluck (die Hoehen), Stiffness (die Obertoene
 * liegen hoeher), String variation (eine tiefe Saite ist dunkler). *Classic* bleibt,
 * wie sie war. `LUMIVIZ_23C_WAV=<Ordner>` legt Hoerproben ab.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Aufnahme.hpp"  // schreibeWav
#include "loop/Instrument.hpp"
#include "loop/Saiten.hpp"

#include "SaitenMessung.hpp"  // betrag, spitze, cent, db, teilton

#include <QDir>
#include <QJsonArray>
#include <QJsonObject>

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

using namespace lumi::loop;

namespace
{

using namespace saitentest;
constexpr int kRate = kSaitenRate;

/// Ein Instrument mit einer Saite *Extended KS*; `stelle` aendert die Schicht.
template <class F>
Instrument ksSaite(F stelle)
{
    Instrument instrument;
    Schicht& s = instrument.schichten[0];
    s.schwingung = Schwingung::Saite;
    s.saitenModell = SaitenModell::ErweiterterKs;
    s.anstiegMs = 1.0;
    s.ausklangMs = 200.0;
    s.hoehenDaempfung = 0.0;
    s.steifheit = 0.0;
    s.anschlagHaerte = 1.0;
    s.klingSekunden = 6.0;
    stelle(s);
    return instrument;
}
Instrument ksSaite()
{
    return ksSaite([](Schicht&) {});
}

}  // namespace

TEST_CASE("Loop Saite KS: die Datei — die Regler nur an einer Saite Extended KS, Classic wie bisher")
{
    Instrument ks = ksSaite([](Schicht& s) {
        s.anschlagOrt = 0.2;
        s.anschlagHaerte = 0.3;
        s.klingSekunden = 2.5;
        s.hoehenDaempfung = 0.6;
        s.steifheit = 0.4;
        s.saitenUnterschied = 0.7;
    });
    const QJsonObject json = alsJson(ks);
    const QJsonObject schicht = json.value(QStringLiteral("layers")).toArray().at(0).toObject();
    CHECK(schicht.value(QStringLiteral("stringModel")).toString() == QStringLiteral("extendedKS"));
    CHECK(schicht.value(QStringLiteral("pluckPosition")).toDouble() == 0.2);
    CHECK(schicht.value(QStringLiteral("ringTime")).toDouble() == 2.5);
    Instrument gelesen;
    REQUIRE(ausJson(json, gelesen));
    CHECK(gelesen == ks);

    // Classic: kein Feld der neuen Saite, die alten Werte bleiben
    Instrument classic;
    classic.schichten[0].schwingung = Schwingung::Saite;
    const QJsonObject alt = alsJson(classic).value(QStringLiteral("layers")).toArray().at(0).toObject();
    for (const char* feld : {"stringModel", "pluckPosition", "pluck", "ringTime", "highDamping", "stiffness", "stringVariation"})
        CHECK_FALSE(alt.contains(QLatin1String(feld)));
    CHECK(alt.contains(QStringLiteral("damping")));
    // ein unbekanntes Verfahren liest sich als Classic; Werte ausserhalb in ihre Grenzen
    QJsonObject wild = schicht;
    wild[QStringLiteral("stringModel")] = QStringLiteral("quantum");
    wild[QStringLiteral("pluckPosition")] = 0.9;
    wild[QStringLiteral("ringTime")] = 999.0;
    QJsonObject wurzel = json;
    wurzel[QStringLiteral("layers")] = QJsonArray{wild};
    REQUIRE(ausJson(wurzel, gelesen));
    CHECK(gelesen.schichten[0].saitenModell == SaitenModell::Klassisch);
    CHECK(gelesen.schichten[0].anschlagOrt == kAnschlagOrtMax);
    CHECK(gelesen.schichten[0].klingSekunden == kKlingSekundenMax);
}

TEST_CASE("Loop Saite KS: Classic rechnet wie bisher — die neuen Regler wirken dort nicht")
{
    Instrument classic;
    classic.schichten[0].schwingung = Schwingung::Saite;
    const Klang vorher = rendereTon(classic, 57, 600.0, kRate);
    Instrument mitReglern = classic;
    Schicht& s = mitReglern.schichten[0];
    s.anschlagOrt = 0.4;
    s.anschlagHaerte = 0.1;
    s.klingSekunden = 0.5;
    s.hoehenDaempfung = 1.0;
    s.steifheit = 1.0;
    s.saitenUnterschied = 1.0;
    mitReglern.saiten = Saitensatz{stimmungsvorlagen()[0].leer, 0, Saitenwahl::JeKlang};
    CHECK(rendereTon(mitReglern, 57, 600.0, kRate).daten == vorher.daten);
    CHECK(rendereTon(mitReglern, TonAngabe{QString(), 57, 600, 0, 0}, kRate).daten == vorher.daten);
}

TEST_CASE("Loop Saite KS: die Tonhoehe stimmt — mit jedem High damping, ohne Stiffness")
{
    for (const int hoehe : {33, 45, 57, 69, 81})
        for (const double hoehen : {0.0, 0.5, 1.0})
        {
            CAPTURE(hoehe);
            CAPTURE(hoehen);
            const Klang ton = rendereTon(ksSaite([&](Schicht& s) { s.hoehenDaempfung = hoehen; }), hoehe, 2000.0, kRate);
            const double soll = tonHz(hoehe);
            CHECK(std::abs(cent(spitze(ton, soll, 60.0), soll)) < 3.0);
        }
}

TEST_CASE("Loop Saite KS: Ring time — der Grundton faellt in dieser Zeit um 60 dB, bei jeder Tonhoehe")
{
    for (const int hoehe : {40, 57, 76})
        for (const double sekunden : {1.0, 3.0})
        {
            CAPTURE(hoehe);
            CAPTURE(sekunden);
            const Klang ton = rendereTon(ksSaite([&](Schicht& s) { s.klingSekunden = sekunden; }), hoehe, 4000.0, kRate);
            const double f0 = spitze(ton, tonHz(hoehe), 30.0);
            const double a = betrag(ton, f0, 0.2, 0.2);
            const double b = betrag(ton, f0, 0.8, 0.2);
            const double dbJeSekunde = (db(a) - db(b)) / 0.6;
            const double t60 = 60.0 / dbJeSekunde;
            MESSAGE("Ring time " << sekunden << " s bei " << tonName(hoehe).toStdString() << ": gemessen " << t60 << " s");
            CHECK(t60 == doctest::Approx(sekunden).epsilon(0.15));
        }
}

TEST_CASE("Loop Saite KS: Pluck position — bei einem Viertel fehlt der vierte Teilton")
{
    const double f0 = tonHz(45);
    const Klang viertel = rendereTon(ksSaite([](Schicht& s) { s.anschlagOrt = 0.25; }), 45, 1000.0, kRate);
    const double t3 = teilton(viertel, f0, 3);
    const double t4 = teilton(viertel, f0, 4);
    const double t5 = teilton(viertel, f0, 5);
    MESSAGE("Pluck position 1/4: Teilton 4 gegen 3 " << db(t4 / t3) << " dB, gegen 5 " << db(t4 / t5) << " dB");
    CHECK(db(t4 / t3) < -15.0);
    CHECK(db(t4 / t5) < -15.0);
    // bei einem Fuenftel fehlt der fuenfte, der vierte ist da
    const Klang fuenftel = rendereTon(ksSaite([](Schicht& s) { s.anschlagOrt = 0.2; }), 45, 1000.0, kRate);
    CHECK(db(teilton(fuenftel, f0, 5) / teilton(fuenftel, f0, 4)) < -15.0);
    CHECK(db(teilton(fuenftel, f0, 4) / teilton(fuenftel, f0, 3)) > -10.0);
}

TEST_CASE("Loop Saite KS: High damping und Pluck machen die Hoehen leiser")
{
    const double f0 = tonHz(45);
    const auto hell = [&](const Instrument& instrument) {
        const Klang ton = rendereTon(instrument, 45, 1500.0, kRate);
        const double h = spitze(ton, 7.0 * f0, 40.0, 0.4, 0.3);
        return db(betrag(ton, h, 0.4, 0.3) / betrag(ton, spitze(ton, f0, 30.0, 0.4, 0.3), 0.4, 0.3));
    };
    const double wenig = hell(ksSaite([](Schicht& s) { s.hoehenDaempfung = 0.1; }));
    const double viel = hell(ksSaite([](Schicht& s) { s.hoehenDaempfung = 0.9; }));
    MESSAGE("Teilton 7 gegen den Grundton nach 0,4 s: High damping 0,1 " << wenig << " dB, 0,9 " << viel << " dB");
    CHECK(viel < wenig - 6.0);
    const double hart = hell(ksSaite([](Schicht& s) { s.anschlagHaerte = 1.0; }));
    const double weich = hell(ksSaite([](Schicht& s) { s.anschlagHaerte = 0.0; }));
    MESSAGE("Pluck hart " << hart << " dB, weich " << weich << " dB");
    CHECK(weich < hart - 6.0);
}

TEST_CASE("Loop Saite KS: Stiffness hebt die Obertoene — der Grundton bleibt")
{
    const double f0 = tonHz(40);
    const auto teiltonLage = [&](double steif) {
        const Klang ton = rendereTon(ksSaite([&](Schicht& s) { s.steifheit = steif; }), 40, 2000.0, kRate);
        const double grund = spitze(ton, f0, 40.0);
        const double acht = spitze(ton, 8.0 * grund, 150.0, 0.1, 0.5);
        return std::pair{grund, cent(acht, 8.0 * grund)};
    };
    const auto [grundOhne, ohne] = teiltonLage(0.0);
    const auto [grundMit, mit] = teiltonLage(1.0);
    MESSAGE("Teilton 8 gegen 8 x Grundton: Stiffness 0 " << ohne << " Cent, Stiffness 1 " << mit << " Cent");
    CHECK(std::abs(ohne) < 5.0);
    CHECK(mit > ohne + 10.0);
    CHECK(std::abs(cent(grundOhne, f0)) < 3.0);
    CHECK(std::abs(cent(grundMit, f0)) < 3.0);
}

TEST_CASE("Loop Saite KS: String variation — dieselbe Note auf einer tieferen Saite klingt dunkler")
{
    // E3 auf der tiefen E-Saite (dick) und auf der D-Saite
    const auto ton = [](double unterschied, int saite) {
        Instrument instrument = ksSaite([&](Schicht& s) {
            s.saitenUnterschied = unterschied;
            s.hoehenDaempfung = 0.2;
        });
        instrument.saiten = Saitensatz{stimmungsvorlagen()[0].leer, 0, Saitenwahl::JeKlang};
        return rendereTon(instrument, TonAngabe{QString(), 52, 1500, saite, 0}, kRate);
    };
    const double f0 = tonHz(52);
    const auto hell = [&](const Klang& k) {
        return db(betrag(k, spitze(k, 6.0 * f0, 60.0, 0.3, 0.3), 0.3, 0.3) / betrag(k, spitze(k, f0, 30.0, 0.3, 0.3), 0.3, 0.3));
    };
    CHECK(ton(0.0, 0).daten == ton(0.0, 2).daten);  // ohne Unterschied gleich
    const double dick = hell(ton(1.0, 0));
    const double duenner = hell(ton(1.0, 2));
    MESSAGE("Teilton 6 gegen den Grundton: tiefe E-Saite " << dick << " dB, D-Saite " << duenner << " dB");
    CHECK(dick < duenner - 3.0);
    // ohne Saite keine Dicke: wie die duennste
    CHECK(ton(1.0, -1).daten == ton(1.0, 5).daten);
}

TEST_CASE("Loop Saite KS: jeder Ton im Umfang bleibt endlich und im Rahmen, mit jeder Stellung")
{
    for (const int hoehe : {kHoeheMin, 40, 64, 88, kHoeheMax})
        for (const double wert : {0.0, 1.0})
        {
            CAPTURE(hoehe);
            CAPTURE(wert);
            const Instrument instrument = ksSaite([&](Schicht& s) {
                s.hoehenDaempfung = wert;
                s.steifheit = wert;
                s.anschlagHaerte = wert;
                s.anschlagOrt = wert > 0.0 ? kAnschlagOrtMax : kAnschlagOrtMin;
                s.klingSekunden = wert > 0.0 ? kKlingSekundenMax : kKlingSekundenMin;
            });
            const Klang ton = rendereTon(instrument, hoehe, 500.0, 48000);
            bool endlich = true;
            float groesster = 0.0f;
            for (const float x : ton.daten)
            {
                if (!std::isfinite(x)) endlich = false;
                groesster = std::max(groesster, std::abs(x));
            }
            CHECK(endlich);
            CHECK(groesster <= 0.9f);
            CHECK(groesster > 0.001f);  // er klingt
            CHECK(std::abs(ton.daten[ton.daten.size() - 2]) < 0.002f);  // und endet bei null
            CHECK(rendereTon(instrument, hoehe, 500.0, 48000).daten == ton.daten);  // derselbe Aufruf, dieselben Werte
        }
}

TEST_CASE("Loop Saite KS: Hoerproben (LUMIVIZ_23C_WAV)")
{
    const QString ordner = qEnvironmentVariable("LUMIVIZ_23C_WAV");
    if (ordner.isEmpty())
    {
        MESSAGE("LUMIVIZ_23C_WAV=<Ordner> legt Hoerproben ab");
        return;
    }
    REQUIRE(QDir().mkpath(ordner));
    struct Probe
    {
        const char* name;
        Instrument instrument;
    };
    Instrument classic;
    classic.schichten[0].schwingung = Schwingung::Saite;
    const std::vector<Probe> proben = {
        {"00_classic", classic},
        {"01_ks_vorgabe", ksSaite([](Schicht& s) { s = Schicht{}; s.schwingung = Schwingung::Saite; s.saitenModell = SaitenModell::ErweiterterKs; })},
        {"02_ks_pluck_mitte", ksSaite([](Schicht& s) { s.anschlagOrt = 0.5; })},
        {"03_ks_pluck_steg", ksSaite([](Schicht& s) { s.anschlagOrt = 0.04; })},
        {"04_ks_finger", ksSaite([](Schicht& s) { s.anschlagHaerte = 0.0; s.hoehenDaempfung = 0.4; })},
        {"05_ks_kurz", ksSaite([](Schicht& s) { s.klingSekunden = 0.4; })},
        {"06_ks_dunkel", ksSaite([](Schicht& s) { s.hoehenDaempfung = 0.9; })},
        {"07_ks_steif", ksSaite([](Schicht& s) { s.steifheit = 1.0; })},
    };
    for (const Probe& probe : proben)
    {
        // eine kleine Folge: E2, A2, E3, A3, E4, je eine halbe Sekunde Abstand, jeder Ton 2 s
        std::vector<float> folge(static_cast<std::size_t>(2 * kRate * 5), 0.0f);
        int nr = 0;
        for (const int hoehe : {40, 45, 52, 57, 64})
        {
            const Klang ton = rendereTon(probe.instrument, hoehe, 2000.0, kRate);
            const auto ab = static_cast<std::size_t>(2 * nr * kRate / 2);
            for (std::size_t i = 0; i < ton.daten.size() && ab + i < folge.size(); ++i) folge[ab + i] += ton.daten[i] * 0.5f;
            ++nr;
        }
        REQUIRE(schreibeWav(QDir(ordner).filePath(QString::fromLatin1(probe.name) + QStringLiteral(".wav")), folge, kRate));
    }
    MESSAGE(proben.size() << " Hoerproben in " << ordner.toStdString());
}
