/**
 ****************************************************************************************
 * @file   test_LoopSaiteWg.cpp
 * @brief  Saiteninstrumente, Stufe 23d (erster Teil): die Saite *Waveguide*, die Mensur
 *         und die Tonabnehmer als Abtaststellen — gemessen am gerechneten Ton
 *         (docs/audio/Saiten_Konzept.md; Entscheide Patrik S132: S-K1 a, S-K5 a, S-U3 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Qt-Anwendung. Die Resonanz der Tonabnehmer mit dem Kabel
 * kommt im zweiten Teil von 23d. Gemessen: Tonhoehe und Ring time wie bei *Extended KS*;
 * die Anschlagstelle und die Lage eines Tonabnehmers loeschen je die Teiltoene mit
 * einem Knoten dort; ein Humbucker (zwei Spulen, 18 mm) nimmt die Teiltoene, deren halbe
 * Wellenlaenge 18 mm ist; *Pickup select*; der Piezo-Hochpass (106 Hz).
 * `LUMIVIZ_23D_WAV=<Ordner>` legt Hoerproben ab.
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

#include <cmath>
#include <vector>

using namespace lumi::loop;
using namespace saitentest;

namespace
{

constexpr int kRate = kSaitenRate;

template <class F>
Instrument wgSaite(F stelle)
{
    Instrument instrument;
    instrument.saiten = Saitensatz{stimmungsvorlagen()[0].leer, 0, Saitenwahl::JeKlang, 648.0};
    Schicht& s = instrument.schichten[0];
    s.schwingung = Schwingung::Saite;
    s.saitenModell = SaitenModell::Wellenleiter;
    s.anstiegMs = 1.0;
    s.ausklangMs = 200.0;
    s.hoehenDaempfung = 0.0;
    s.steifheit = 0.0;
    s.anschlagHaerte = 1.0;
    s.klingSekunden = 6.0;
    s.saitenUnterschied = 0.0;
    stelle(s);
    return instrument;
}

}  // namespace

TEST_CASE("Loop Saite WG: die Datei — Waveguide, Tonabnehmer, Mensur")
{
    Instrument wg = wgSaite([](Schicht& s) {
        s.abnehmerHals = Abnehmer{AbnehmerArt::SingleCoil, 152.0};
        s.abnehmerSteg = Abnehmer{AbnehmerArt::Humbucker, 41.0};
        s.abnehmerWahl = AbnehmerWahl::Steg;
    });
    const QJsonObject json = alsJson(wg);
    const QJsonObject schicht = json.value(QStringLiteral("layers")).toArray().at(0).toObject();
    CHECK(schicht.value(QStringLiteral("stringModel")).toString() == QStringLiteral("waveguide"));
    CHECK(schicht.value(QStringLiteral("pickupNeck")).toObject().value(QStringLiteral("type")).toString() == QStringLiteral("singleCoil"));
    CHECK(schicht.value(QStringLiteral("pickupBridge")).toObject().value(QStringLiteral("position")).toDouble() == 41.0);
    CHECK(schicht.value(QStringLiteral("pickupSelect")).toString() == QStringLiteral("bridge"));
    CHECK(json.value(QStringLiteral("strings")).toObject().value(QStringLiteral("scale")).toDouble() == 648.0);
    Instrument gelesen;
    REQUIRE(ausJson(json, gelesen));
    CHECK(gelesen == wg);

    // Extended KS traegt keine Tonabnehmer; eine Mensur von 0 steht nicht in der Datei
    Instrument ks = wg;
    ks.schichten[0].saitenModell = SaitenModell::ErweiterterKs;
    ks.saiten.mensurMm = 0.0;
    const QJsonObject ksJson = alsJson(ks);
    CHECK_FALSE(ksJson.value(QStringLiteral("layers")).toArray().at(0).toObject().contains(QStringLiteral("pickupNeck")));
    CHECK_FALSE(ksJson.value(QStringLiteral("strings")).toObject().contains(QStringLiteral("scale")));
    CHECK(mensurVon(ks.saiten) == kMensurVorgabeMm);
    // Grenzen
    Saitensatz wild = wg.saiten;
    wild.mensurMm = 5000.0;
    richteSaiten(wild);
    CHECK(wild.mensurMm == kMensurMaxMm);
    wild.mensurMm = -3.0;
    richteSaiten(wild);
    CHECK(wild.mensurMm == 0.0);
}

TEST_CASE("Loop Saite WG: Tonhoehe und Ring time wie Extended KS")
{
    for (const int hoehe : {40, 57, 76})
    {
        CAPTURE(hoehe);
        const Klang ton = rendereTon(wgSaite([](Schicht& s) { s.klingSekunden = 2.0; }), hoehe, 3000.0, kRate);
        const double soll = tonHz(hoehe);
        const double f0 = spitze(ton, soll, 60.0);
        CHECK(std::abs(cent(f0, soll)) < 3.0);
        const double t60 = 60.0 / ((db(betrag(ton, f0, 0.2, 0.2)) - db(betrag(ton, f0, 0.8, 0.2))) / 0.6);
        MESSAGE("Waveguide " << tonName(hoehe).toStdString() << ": Ring time 2 s, gemessen " << t60 << " s");
        CHECK(t60 == doctest::Approx(2.0).epsilon(0.15));
    }
}

TEST_CASE("Loop Saite WG: die Anschlagstelle und die Lage des Tonabnehmers loeschen ihre Teiltoene")
{
    const double f0 = tonHz(45);
    // angeschlagen bei einem Viertel, abgenommen am Steg: der vierte Teilton fehlt
    const Klang viertel = rendereTon(wgSaite([](Schicht& s) { s.anschlagOrt = 0.25; }), 45, 1000.0, kRate);
    const double a = db(teilton(viertel, f0, 4) / teilton(viertel, f0, 3));
    MESSAGE("Waveguide, Anschlag 1/4: Teilton 4 gegen 3 " << a << " dB");
    CHECK(a < -15.0);
    // ein Single coil bei einem Fuenftel der Mensur (vom Steg): der fuenfte fehlt, angeschlagen woanders
    const Klang abnehmer = rendereTon(wgSaite([](Schicht& s) {
                                          s.anschlagOrt = 0.13;
                                          s.abnehmerSteg = Abnehmer{AbnehmerArt::SingleCoil, 648.0 / 5.0};
                                      }),
                                      45, 1000.0, kRate);
    const double b = db(teilton(abnehmer, f0, 5) / teilton(abnehmer, f0, 4));
    MESSAGE("Single coil bei 1/5: Teilton 5 gegen 4 " << b << " dB");
    CHECK(b < -15.0);
    // dieselbe Saite ohne Tonabnehmer hat den fuenften
    const Klang akustisch = rendereTon(wgSaite([](Schicht& s) { s.anschlagOrt = 0.13; }), 45, 1000.0, kRate);
    CHECK(db(teilton(akustisch, f0, 5) / teilton(akustisch, f0, 4)) > -10.0);
}

TEST_CASE("Loop Saite WG: ein Humbucker nimmt die Teiltoene, deren halbe Wellenlaenge 18 mm ist")
{
    // E2 auf 648 mm: der Teilton n hat eine halbe Wellenlaenge von 648/n mm; bei n = 36 sind es 18 mm
    const double f0 = tonHz(40);
    const auto ton = [](AbnehmerArt art) {
        return rendereTon(wgSaite([&](Schicht& s) {
                              s.anschlagOrt = 0.11;
                              s.abnehmerSteg = Abnehmer{art, 40.0};
                          }),
                          40, 1000.0, kRate);
    };
    const Klang einfach = ton(AbnehmerArt::SingleCoil);
    const Klang doppelt = ton(AbnehmerArt::Humbucker);
    const double luecke = db(teilton(doppelt, f0, 36) / teilton(einfach, f0, 36));
    const double tief = db(teilton(doppelt, f0, 3) / teilton(einfach, f0, 3));
    MESSAGE("Humbucker gegen Single coil: Teilton 3 " << tief << " dB, Teilton 36 " << luecke << " dB");
    // beide sind auf denselben Pegel bemessen: die tiefen Teiltoene liegen nah beieinander
    CHECK(std::abs(tief) < 3.0);
    CHECK(luecke < -12.0);
}

TEST_CASE("Loop Saite WG: Pickup select — Neck, Bridge, Both; einer auf None")
{
    const auto ton = [](AbnehmerWahl wahl, AbnehmerArt hals = AbnehmerArt::SingleCoil) {
        return rendereTon(wgSaite([&](Schicht& s) {
                              s.abnehmerHals = Abnehmer{hals, 160.0};
                              s.abnehmerSteg = Abnehmer{AbnehmerArt::SingleCoil, 40.0};
                              s.abnehmerWahl = wahl;
                          }),
                          45, 600.0, kRate);
    };
    const Klang hals = ton(AbnehmerWahl::Hals);
    const Klang steg = ton(AbnehmerWahl::Steg);
    const Klang beide = ton(AbnehmerWahl::Beide);
    CHECK(hals.daten != steg.daten);
    REQUIRE(beide.daten.size() == hals.daten.size());
    double abweichung = 0.0;
    for (std::size_t i = 0; i < beide.daten.size(); ++i)
        abweichung = std::max(abweichung, std::abs(static_cast<double>(beide.daten[i]) -
                                                   0.5 * (static_cast<double>(hals.daten[i]) + steg.daten[i])));
    CHECK(abweichung < 1e-5);  // Both ist die Haelfte von jedem
    // steht der Hals auf None, gilt Neck fuer den Steg
    CHECK(ton(AbnehmerWahl::Hals, AbnehmerArt::Keiner).daten == steg.daten);
    // der Steg-Tonabnehmer klingt heller als der am Hals (mehr hohe Teiltoene)
    const double f0 = tonHz(45);
    const auto hell = [&](const Klang& k) { return db(teilton(k, f0, 9) / teilton(k, f0, 1)); };
    MESSAGE("Teilton 9 gegen 1: Neck " << hell(hals) << " dB, Bridge " << hell(steg) << " dB");
    CHECK(hell(steg) > hell(hals) + 3.0);
}

TEST_CASE("Loop Saite WG: der Piezo nimmt die Kraft am Steg, durch seinen Hochpass bei 106 Hz")
{
    // E1 (41,2 Hz): der Hochpass laesst |H| = f / sqrt(f^2 + 106^2) durch, rund −8,7 dB
    const double f0 = tonHz(28);
    const auto ton = [](AbnehmerArt art) {
        return rendereTon(wgSaite([&](Schicht& s) { s.abnehmerSteg = Abnehmer{art, 0.0}; }), 28, 2000.0, kRate);
    };
    const Klang akustisch = ton(AbnehmerArt::Keiner);
    const Klang piezo = ton(AbnehmerArt::Piezo);
    const double grund = db(betrag(piezo, f0, 0.3, 0.6) / betrag(akustisch, f0, 0.3, 0.6));
    const double soll = db(f0 / std::sqrt(f0 * f0 + 106.0 * 106.0));
    MESSAGE("Piezo gegen akustisch beim Grundton E1: " << grund << " dB (Hochpass erster Ordnung: " << soll << " dB)");
    CHECK(grund == doctest::Approx(soll).epsilon(0.15));
    // hoch oben laesst er fast alles
    CHECK(db(teilton(piezo, f0, 30) / teilton(akustisch, f0, 30)) > -1.0);
}

TEST_CASE("Loop Saite WG: Cable — die Resonanz eines magnetischen Tonabnehmers aus Messwerten")
{
    // die Bezugswerte (Saiten_Konzept §6b): Single coil 3,5 kHz bei 600 pF, +4 dB; Humbucker 3 kHz bei 450 pF, Q 1,83
    const Resonanz single = abnehmerResonanz(AbnehmerArt::SingleCoil, 600.0);
    CHECK(single.hz == doctest::Approx(3500.0));
    CHECK(db(single.guete) == doctest::Approx(4.0));
    const Resonanz humbucker = abnehmerResonanz(AbnehmerArt::Humbucker, 450.0);
    CHECK(humbucker.hz == doctest::Approx(3000.0));
    CHECK(humbucker.guete == doctest::Approx(1.83));
    // viermal die Kapazitaet: die halbe Frequenz (f ∝ 1/√C), die Guete bleibt
    CHECK(abnehmerResonanz(AbnehmerArt::Humbucker, 1800.0).hz == doctest::Approx(1500.0));
    CHECK(abnehmerResonanz(AbnehmerArt::Humbucker, 1800.0).guete == doctest::Approx(1.83));
    // Piezo und None haben keine
    CHECK(abnehmerResonanz(AbnehmerArt::Piezo, 500.0).hz == 0.0);
    CHECK(abnehmerResonanz(AbnehmerArt::Keiner, 500.0).hz == 0.0);
    // Grenzen: das Kabel wird auf kKabelMinPf … kKabelMaxPf begrenzt
    CHECK(abnehmerResonanz(AbnehmerArt::SingleCoil, 1.0).hz == doctest::Approx(3500.0 * std::sqrt(600.0 / kKabelMinPf)));
    CHECK(abnehmerResonanz(AbnehmerArt::SingleCoil, 1e6).hz == doctest::Approx(3500.0 * std::sqrt(600.0 / kKabelMaxPf)));

    // im Ton: ein kurzes Kabel laesst um 5 kHz deutlich mehr durch als ein langes
    const double f0 = tonHz(52);
    const auto hell = [&](double pf) {
        const Klang ton = rendereTon(wgSaite([&](Schicht& s) {
                                         s.abnehmerSteg = Abnehmer{AbnehmerArt::SingleCoil, 41.0};
                                         s.kabelPf = pf;
                                     }),
                                     52, 1000.0, kRate);
        const int k = static_cast<int>(std::lround(5000.0 / f0));
        return db(teilton(ton, f0, k) / teilton(ton, f0, 1));
    };
    const double kurz = hell(300.0);
    const double lang = hell(1500.0);
    MESSAGE("Single coil, Teilton um 5 kHz gegen den Grundton: 300 pF " << kurz << " dB, 1500 pF " << lang << " dB");
    CHECK(kurz > lang + 10.0);
    // die Datei traegt Cable
    Instrument mitKabel = wgSaite([](Schicht& s) { s.kabelPf = 820.0; });
    Instrument gelesen;
    REQUIRE(ausJson(alsJson(mitKabel), gelesen));
    CHECK(gelesen.schichten[0].kabelPf == 820.0);
}

TEST_CASE("Loop Saite WG: jeder Ton im Umfang bleibt endlich und im Rahmen")
{
    for (const int hoehe : {kHoeheMin, 40, 64, 88, kHoeheMax})
        for (const AbnehmerArt art : {AbnehmerArt::Keiner, AbnehmerArt::SingleCoil, AbnehmerArt::Humbucker, AbnehmerArt::Piezo})
        {
            CAPTURE(hoehe);
            CAPTURE(static_cast<int>(art));
            const Instrument instrument = wgSaite([&](Schicht& s) {
                s.hoehenDaempfung = 0.5;
                s.steifheit = 1.0;
                s.abnehmerHals = Abnehmer{art, 160.0};
                s.abnehmerSteg = Abnehmer{art, 5.0};
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
            CHECK(groesster > 0.0005f);
            CHECK(std::abs(ton.daten[ton.daten.size() - 2]) < 0.002f);
        }
}

TEST_CASE("Loop Saite WG: Hoerproben (LUMIVIZ_23D_WAV)")
{
    const QString ordner = qEnvironmentVariable("LUMIVIZ_23D_WAV");
    if (ordner.isEmpty())
    {
        MESSAGE("LUMIVIZ_23D_WAV=<Ordner> legt Hoerproben ab");
        return;
    }
    REQUIRE(QDir().mkpath(ordner));
    struct Probe
    {
        const char* name;
        Instrument instrument;
    };
    const auto vorgabe = [](Schicht& s) {
        const Schicht leer;
        s.anschlagOrt = leer.anschlagOrt;
        s.anschlagHaerte = leer.anschlagHaerte;
        s.klingSekunden = leer.klingSekunden;
        s.hoehenDaempfung = leer.hoehenDaempfung;
        s.steifheit = leer.steifheit;
    };
    const std::vector<Probe> proben = {
        {"01_wg_akustisch", wgSaite(vorgabe)},
        {"02_wg_single_neck", wgSaite([&](Schicht& s) { vorgabe(s); s.abnehmerHals = {AbnehmerArt::SingleCoil, 152.0}; s.abnehmerWahl = AbnehmerWahl::Hals; })},
        {"03_wg_single_bridge", wgSaite([&](Schicht& s) { vorgabe(s); s.abnehmerSteg = {AbnehmerArt::SingleCoil, 41.0}; s.abnehmerWahl = AbnehmerWahl::Steg; })},
        {"04_wg_humbucker_bridge", wgSaite([&](Schicht& s) { vorgabe(s); s.abnehmerSteg = {AbnehmerArt::Humbucker, 41.0}; s.abnehmerWahl = AbnehmerWahl::Steg; })},
        {"05_wg_both", wgSaite([&](Schicht& s) { vorgabe(s); s.abnehmerHals = {AbnehmerArt::SingleCoil, 152.0}; s.abnehmerSteg = {AbnehmerArt::SingleCoil, 41.0}; })},
        {"06_wg_piezo", wgSaite([&](Schicht& s) { vorgabe(s); s.abnehmerSteg = {AbnehmerArt::Piezo, 0.0}; s.abnehmerWahl = AbnehmerWahl::Steg; })},
    };
    for (const Probe& probe : proben)
    {
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
