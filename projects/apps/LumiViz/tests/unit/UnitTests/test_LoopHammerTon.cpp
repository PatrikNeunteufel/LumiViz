/**
 ****************************************************************************************
 * @file   test_LoopHammerTon.cpp
 * @brief  Loop, Stufe 31b: ein Ton der Hammer-Saite — gemessen am gerechneten Ton
 *         (docs/audio/Spielweisen_Konzept.md §4.1, Entscheide Patrik S145/S146: HS1 a,
 *         HT1 a bis HT3 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Dateien aus dem Repo; gemessen mit `SaitenMessung.hpp`.
 *
 * - Der Chor: Σ g = 1; ohne Verstimmung nur die Gleichlage; die Stufen liegen bei σ_s + Nσ_b
 *   und σ_s.
 * - Die Teiltoene liegen nach B gestreckt, der erste auf der Tonhoehe.
 * - Laut gespielt heller (der nichtlineare Hammer), nicht nur lauter.
 * - Eine Saite klingt mit *Decay* ab; drei verstimmte Saiten zweistufig.
 * - Der Daempfer beim Loslassen; ueber `undampedFrom` klingt der Ton weiter.
 * - Derselbe Aufruf gibt dieselben Werte; der Klang endet bei null, beide Seiten gleich.
 *
 * `LUMIVIZ_31B_WAV=<Ordner>` legt Hoerproben ab (Patrik hoert sie gesammelt spaeter).
 *
 * Der Bezug (31d-7, P8 a): einige Toene, gerechnet mit der Hammer-Saite aus
 * `werte/hammer_bezug_saite.json` (Stand vor 31d-7, unabhaengig von der Vorgabe), gegen ihre
 * Pruefsummen in `werte/hammer_bezug_<compiler>.txt` — neu mit `LUMIVIZ_HAMMER_BEZUG_SCHREIBEN=1`
 * (die Saite wird dabei nur geschrieben, wenn sie fehlt).
 *
 * 31d-7 B (US5 a): der Stoss je Saite — die Moden, im Chor flachere Einbrueche, bei einer Saite unveraendert,
 * leise und laut an ihren Stufen.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Aufnahme.hpp"  // schreibeWav
#include "loop/HammerTon.hpp"
#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"
#include "loop/Pedalplan.hpp"
#include "loop/Rechenkern.hpp"

#include "SaitenMessung.hpp"  // betrag, spitze, cent, db, teilton

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <string>

using namespace lumi::loop;
using namespace saitentest;

namespace
{

Instrument fluegel()
{
    Instrument instrument;
    Schicht& s = instrument.schichten[0];
    s.schwingung = Schwingung::Saite;
    s.saitenModell = SaitenModell::Modal;
    s.anregung = Anregung::Hammer;
    instrument.tastatur = Tastatur{};
    return instrument;
}

Klang ton(const Instrument& instrument, int hoehe, double dauerMs, int stufe = 0)
{
    TonAngabe angabe;
    angabe.hoehe = hoehe;
    angabe.dauerMs = static_cast<int>(dauerMs);
    angabe.anschlagStufe = stufe;
    return rendereTon(instrument, angabe, kSaitenRate);
}

/// Der Pegel des Grundtons im Fenster ab `vonS` (0,2 s lang), in dB.
double pegel(const Klang& klang, double hz, double vonS)
{
    return db(betrag(klang, hz, vonS, 0.2));
}

}  // namespace

TEST_CASE("Loop Hammer-Ton 31b Chor: gekoppelte Moden")
{
    const double omega = 2.0 * kPi * 261.6;
    const double sigmaSaite = 0.25;
    const double sigmaSteg = 0.17;
    SUBCASE("ohne Verstimmung nur die Gleichlage, mit der ersten Stufe")
    {
        const auto moden = chorModen({omega, omega, omega}, sigmaSaite, sigmaSteg);
        REQUIRE(moden.size() == 1);
        CHECK(moden[0].lambda.real() == doctest::Approx(-(sigmaSaite + 3 * sigmaSteg)));
        CHECK(moden[0].lambda.imag() == doctest::Approx(omega));
        CHECK(std::abs(moden[0].anteil - 1.0) < 1e-12);
    }
    SUBCASE("verstimmt: Σ g = 1, die Abklingraten zwischen den beiden Stufen")
    {
        for (const int n : {2, 3})
        {
            CAPTURE(n);
            std::vector<double> omegas;
            for (int j = 0; j < n; ++j) omegas.push_back(omega * std::exp2((j - (n - 1) / 2.0) * 1.0 / 1200.0));
            const auto moden = chorModen(omegas, sigmaSaite, sigmaSteg);
            REQUIRE(moden.size() == static_cast<std::size_t>(n));
            std::complex<double> summe(0.0, 0.0);
            double spur = 0.0;
            for (const ChorMode& m : moden)
            {
                summe += m.anteil;
                spur += m.lambda.real();
                CHECK(-m.lambda.real() >= sigmaSaite - 1e-9);
                CHECK(-m.lambda.real() <= sigmaSaite + n * sigmaSteg + 1e-9);
            }
            CHECK(std::abs(summe - 1.0) < 1e-9);
            // die Spur von A bleibt: Σ Re λ = −N·σ_s − N·σ_b
            CHECK(spur == doctest::Approx(-n * sigmaSaite - n * sigmaSteg));
        }
    }
    SUBCASE("eine Saite: ein Mode mit dem ganzen Verlust")
    {
        const auto moden = chorModen({omega}, sigmaSaite, sigmaSteg);
        REQUIRE(moden.size() == 1);
        CHECK(moden[0].lambda.real() == doctest::Approx(-(sigmaSaite + sigmaSteg)));
    }
}

TEST_CASE("Loop Hammer-Ton 31b Teiltoene nach B")
{
    // E1 hat eine Saite (Vorgabe Chor) — keine Schwebung im Weg; leise angeschlagen, damit die
    // Phantomteiltoene der Laengsschwingung (∝ v²) die hohen Querteiltoene nicht ueberdecken
    const Instrument instrument = fluegel();
    const int taste = 28;
    const double hz = tonHz(taste);
    const Klang klang = ton(instrument, taste, 3000.0, -30);
    REQUIRE(klang.frames() > kSaitenRate);
    const double b = wertAn(instrument.schichten[0].hammerSaite.inharmonizitaet, taste, KurvenSkala::Logarithmisch);
    const double f0 = hz / std::sqrt(1.0 + b);
    // der erste Teilton auf der Tonhoehe
    CHECK(std::abs(cent(spitze(klang, hz, 20.0, 0.2, 1.5), hz)) <= 0.75);
    for (const int k : {5, 10, 15})
    {
        CAPTURE(k);
        const double soll = k * f0 * std::sqrt(1.0 + b * k * k);
        const double gemessen = spitze(klang, k * hz, 60.0, 0.2, 1.5);
        CAPTURE(cent(soll, k * hz));
        CHECK(std::abs(cent(gemessen, soll)) <= 1.0);
    }
}

TEST_CASE("Loop Hammer-Ton 31b laut heller, nicht nur lauter")
{
    const Instrument instrument = fluegel();
    const int taste = 48;  // C3
    const double hz = tonHz(taste);
    const Klang laut = ton(instrument, taste, 1000.0, 0);
    const Klang leise = ton(instrument, taste, 1000.0, -20);
    const double grundLaut = teilton(laut, hz, 1);
    const double grundLeise = teilton(leise, hz, 1);
    CHECK(grundLaut > grundLeise);
    // der achte Teilton waechst schneller als der Grundton
    const double hellLaut = db(teilton(laut, hz, 8) / grundLaut);
    const double hellLeise = db(teilton(leise, hz, 8) / grundLeise);
    MESSAGE("Teilton 8 zum Grundton: ff " << hellLaut << " dB, pp " << hellLeise << " dB; Grundton ff/pp "
                                          << db(grundLaut / grundLeise) << " dB");
    CHECK(hellLaut > hellLeise + 3.0);
}

TEST_CASE("Loop Hammer-Ton 31b Abklingen: eine Saite mit Decay, drei zweistufig")
{
    Instrument instrument = fluegel();
    SUBCASE("eine Saite: der Grundton faellt mit 60 dB je Decay")
    {
        const int taste = 28;
        const double hz = tonHz(taste);
        const double t60 = wertAn(instrument.schichten[0].hammerSaite.abklingS, taste, KurvenSkala::Logarithmisch);
        const Klang klang = ton(instrument, taste, 4000.0);
        const double fall = pegel(klang, hz, 0.5) - pegel(klang, hz, 3.5);
        MESSAGE("E1: " << fall << " dB in 3 s, soll " << 60.0 * 3.0 / t60);
        CHECK(fall == doctest::Approx(60.0 * 3.0 / t60).epsilon(0.15));
    }
    SUBCASE("drei verstimmte Saiten: spaeter langsamer als frueh")
    {
        const int taste = 60;
        const double hz = tonHz(taste);
        HammerSaite& h = instrument.schichten[0].hammerSaite;
        h.verstimmungCent = {{{21, 2.0}, {108, 2.0}}, {}};
        const Klang klang = ton(instrument, taste, 8000.0);
        const double frueh = (pegel(klang, hz, 0.2) - pegel(klang, hz, 1.2)) / 1.0;
        const double spaet = (pegel(klang, hz, 5.0) - pegel(klang, hz, 7.0)) / 2.0;
        MESSAGE("C4: frueh " << frueh << " dB/s, spaet " << spaet << " dB/s");
        CHECK(spaet < 0.7 * frueh);
        // ohne Verstimmung bleibt es eine Stufe
        h.verstimmungCent = {{{21, 0.0}, {108, 0.0}}, {}};
        const Klang rein = ton(instrument, taste, 8000.0);
        const double reinFrueh = pegel(rein, hz, 0.2) - pegel(rein, hz, 1.2);
        const double reinSpaet = (pegel(rein, hz, 5.0) - pegel(rein, hz, 7.0)) / 2.0;
        CHECK(reinSpaet == doctest::Approx(reinFrueh).epsilon(0.1));
    }
}

TEST_CASE("Loop Hammer-Ton 31b Daempfer und undampedFrom")
{
    const Instrument instrument = fluegel();
    SUBCASE("mit Daempfer: nach dem Loslassen schnell still")
    {
        const int taste = 60;
        const Klang klang = ton(instrument, taste, 300.0);
        const double vorher = pegel(klang, tonHz(taste), 0.1);
        const double nachher = pegel(klang, tonHz(taste), 0.7);
        MESSAGE("C4 mit Daempfer: " << vorher - nachher << " dB");
        CHECK(vorher - nachher > 30.0);
    }
    SUBCASE("ohne Daempfer (ab G6): klingt weiter, wie die Saite es will")
    {
        const int taste = 96;
        REQUIRE(taste >= kUngedaempftAbVorgabe);
        const Klang klang = ton(instrument, taste, 300.0);
        const double frei = pegel(klang, tonHz(taste), 0.1) - pegel(klang, tonHz(taste), 0.7);
        // dieselbe Taste mit Daempfer
        Instrument alle = instrument;
        alle.tastatur->ungedaempftAb = kTasteMax + 1;
        const Klang gedaempft = ton(alle, taste, 300.0);
        const double mit = pegel(gedaempft, tonHz(taste), 0.1) - pegel(gedaempft, tonHz(taste), 0.7);
        MESSAGE("C7 ohne Daempfer: " << frei << " dB, mit: " << mit << " dB in 0,6 s");
        CHECK(frei < mit - 30.0);
        // ohne Daempfer faellt sie mit der ersten Stufe (Decay) — nicht schneller
        const double t60 = wertAn(instrument.schichten[0].hammerSaite.abklingS, taste, KurvenSkala::Logarithmisch);
        CHECK(frei < 60.0 * 0.6 / t60 * 1.15);
    }
}

TEST_CASE("Loop Hammer-Ton 31b derselbe Ton, endet bei null")
{
    const Instrument instrument = fluegel();
    for (const int taste : {21, 45, 60, 84, 108})
    {
        CAPTURE(taste);
        const Klang a = ton(instrument, taste, 500.0);
        const Klang b = ton(instrument, taste, 500.0);
        CHECK(a.daten == b.daten);
        REQUIRE(a.frames() > 0);
        float spitzenwert = 0.0f;
        bool gleich = true;
        bool endlich = true;
        for (std::int64_t i = 0; i < a.frames(); ++i)
        {
            const float l = a.daten[static_cast<std::size_t>(2 * i)];
            const float r = a.daten[static_cast<std::size_t>(2 * i + 1)];
            gleich = gleich && l == r;
            endlich = endlich && std::isfinite(l);
            spitzenwert = std::max(spitzenwert, std::abs(l));
        }
        CHECK(gleich);
        CHECK(endlich);
        CHECK(spitzenwert > 0.0f);
        CHECK(a.daten.back() == 0.0f);
        MESSAGE("Taste " << taste << ": Spitze " << spitzenwert << ", " << a.frames() / double(kSaitenRate) << " s");
    }
}

TEST_CASE("Loop Hammer-Ton 31b Anschlag gegen Chabassier, Joly & Chaigne 2013, Tab. IV")
{
    // die groesste Auslenkung je Durchmesser (ADR) fuer dieselben Saiten und Haemmer, gerechnet
    // mit einem anderen Verfahren (finite Differenzen); Durchmesser aus Tab. III
    struct Zeile
    {
        int taste;
        double dMm;
        double tempo;
        double adr;
    };
    const Instrument instrument = fluegel();
    const HammerSaite& h = instrument.schichten[0].hammerSaite;
    for (const Zeile& z : {Zeile{27, 1.48, 0.5, 0.568}, Zeile{27, 1.48, 3.0, 3.59}, Zeile{36, 0.9502, 1.5, 2.01},
                           Zeile{53, 1.0525, 0.5, 0.337}, Zeile{53, 1.0525, 3.0, 2.10}, Zeile{73, 0.921, 4.5, 2.64},
                           Zeile{91, 0.794, 1.5, 0.443}})
    {
        CAPTURE(z.taste);
        CAPTURE(z.tempo);
        HammerAnschlag a;
        a.taste = z.taste;
        a.hz = tonHz(z.taste);
        a.stufe = static_cast<int>(std::lround(20.0 * std::log10(z.tempo / kHammerTempoMax)));
        a.halten = kSaitenRate;
        a.rate = kSaitenRate;
        HammerMessung m;
        std::vector<float> mono;
        rechneHammerTon(h, a, mono, kSaitenRate / 10, &m);
        // die Stufe rundet auf 1 dB: auf das genaue Tempo umrechnen (die Auslenkung waechst etwa mit v)
        const double adr = m.auslenkungSpitzeM / (z.dMm * 1e-3) * z.tempo / hammerTempo(a.stufe);
        MESSAGE("Taste " << z.taste << ", " << z.tempo << " m/s: ADR " << adr << " (Tab. IV " << z.adr << "), Kontakt "
                         << m.kontaktS * 1e3 << " ms, Kraft " << m.kraftSpitzeN << " N");
        CHECK(adr == doctest::Approx(z.adr).epsilon(0.5));
        CHECK(m.kontaktS > 0.3e-3);
        CHECK(m.kontaktS < 6e-3);
    }
}

TEST_CASE("Loop Hammer-Ton 31b-2 Laengsschwingung: Phantomteiltoene wachsen mit ES")
{
    // E1, eine Saite: der Phantomteilton bei f10 + f11 liegt dort rund 26 Hz neben f21
    Instrument normal = fluegel();
    const int taste = 28;
    const double hz = tonHz(taste);
    const HammerSaite& h = normal.schichten[0].hammerSaite;
    const double b = wertAn(h.inharmonizitaet, taste, KurvenSkala::Logarithmisch);
    const double f0 = hz / std::sqrt(1.0 + b);
    const auto fk = [&](int k) { return k * f0 * std::sqrt(1.0 + b * k * k); };
    const double phantom = fk(10) + fk(11);
    REQUIRE(fk(21) - phantom > 15.0);
    Instrument steif = normal;
    const double r = wertAn(h.laengsVerhaeltnis, taste, KurvenSkala::Linear);
    steif.schichten[0].hammerSaite.laengsVerhaeltnis = {{{21, 3.0 * r}, {108, 3.0 * r}}, {}};  // ES neunmal
    const Klang a = ton(normal, taste, 2000.0);
    const Klang s = ton(steif, taste, 2000.0);
    const double pa = db(betrag(a, phantom, 0.05, 0.5));
    const double ps = db(betrag(s, phantom, 0.05, 0.5));
    const double ga = db(betrag(a, hz, 0.05, 0.5));
    const double gs = db(betrag(s, hz, 0.05, 0.5));
    MESSAGE("E1 Phantom f10+f11 (" << phantom << " Hz): " << pa << " dB, mit ES ×9: " << ps << " dB; Grundton "
                                   << ga << " / " << gs << " dB");
    CHECK(ps - pa > 12.0);               // neunmal ES: bis +19 dB, die Laengsmoden verschieben etwas
    CHECK(std::abs(gs - ga) < 0.5);      // der Grundton merkt nichts
    // und der Phantomteilton steht ueber dem Rauschboden zwischen den Teiltoenen
    const double zwischen = db(betrag(a, fk(21) + 0.5 * (fk(22) - fk(21)), 0.05, 0.5));
    CHECK(pa > zwischen + 10.0);
}

TEST_CASE("Loop Hammer-Ton 31b-2 ohne Gleichanteil")
{
    const Instrument instrument = fluegel();
    for (const int taste : {21, 40, 60})
    {
        CAPTURE(taste);
        const Klang k = ton(instrument, taste, 1500.0, 0);
        double summe = 0.0;
        double quadrate = 0.0;
        const std::int64_t frames = std::min<std::int64_t>(k.frames(), kSaitenRate);
        for (std::int64_t i = 0; i < frames; ++i)
        {
            const double x = k.daten[static_cast<std::size_t>(2 * i)];
            summe += x;
            quadrate += x * x;
        }
        const double mittel = summe / frames;
        const double effektiv = std::sqrt(quadrate / frames);
        CHECK(std::abs(mittel) < 0.02 * effektiv);
    }
}

TEST_CASE("Loop Hammer-Ton 31b-3 Planung: die Stufe im Namen, der Rest als Pegel")
{
    // ein Takt des Instruments „fluegel" mit drei Noten verschiedener Staerke
    Loop loop;
    loop.laut = 1.0;
    Muster m;
    m.name = QStringLiteral("P");
    m.art = MusterArt::Instrument;
    m.instrument = QStringLiteral("fluegel");
    m.noten = {Note{0, 4, 60, 1.0}, Note{4, 4, 60, 0.5}, Note{8, 4, 62, 0.3}};
    loop.muster = {m};
    loop.kanaele = {Kanal{QStringLiteral("K"), true, {LoopClip(QStringLiteral("P"), 0, 1)}}};
    loop.richte();
    const InstrumentVon mitHammer = [](const QString&) { return InstrumentAuskunft(Saitensatz{}, true); };
    const InstrumentVon ohneHammer = [](const QString&) { return InstrumentAuskunft{}; };

    SUBCASE("mit Hammer: plane und klangNamen nennen dieselben Toene mit Stufe")
    {
        const Ablauf ablauf = plane(loop, kSaitenRate, mitHammer);
        QStringList erwartet = loop.klangNamen(mitHammer);
        QStringList geplant = ablauf.klangNamen;
        erwartet.sort();
        geplant.sort();
        CHECK(geplant == erwartet);
        REQUIRE(geplant.size() == 3);
        // die Stufen 0, −6, −10 (20·lg 0,3 = −10,5)
        int mitZusatz = 0;
        for (const QString& name : geplant)
        {
            TonAngabe ton;
            REQUIRE(zerlegeTon(name, ton));
            CHECK((ton.anschlagStufe == 0 || ton.anschlagStufe == -6 || ton.anschlagStufe == -10));
            if (ton.anschlagStufe < 0) ++mitZusatz;
            CHECK(name.contains(QStringLiteral(":a")) == (ton.anschlagStufe < 0));
        }
        CHECK(mitZusatz == 2);
        // der Pegel jedes Schlags ist der Rest: Note / Stufe — beide Seiten, Muster und Loop auf 1
        for (std::size_t s = 0; s < ablauf.stimmen.size(); ++s)
        {
            const Stimme& stimme = ablauf.stimmen[s];
            REQUIRE(stimme.klang >= 0);
            TonAngabe ton;
            REQUIRE(zerlegeTon(ablauf.klangNamen[stimme.klang], ton));
            const double laut = ton.anschlagStufe == 0 ? 1.0 : ton.anschlagStufe == -6 ? 0.5 : 0.3;
            for (const Schlag& schlag : stimme.schlaege)
            {
                CHECK(schlag.links == doctest::Approx(anschlagRest(laut, ton.anschlagStufe)));
                CHECK(schlag.links * std::pow(10.0, ton.anschlagStufe / 20.0) == doctest::Approx(laut));
            }
        }
        // je Vorkommen (wie mit Pedal) dieselbe Stufe
        CHECK(tonImClip(loop, loop.muster.front(), 1, 0, 1, true).anschlagStufe == -6);
    }
    SUBCASE("ohne Hammer: Namen und Pegel wie bisher")
    {
        const Ablauf ablauf = plane(loop, kSaitenRate, ohneHammer);
        for (const QString& name : ablauf.klangNamen) CHECK_FALSE(name.contains(QStringLiteral(":a")));
        CHECK(ablauf.klangNamen == plane(loop, kSaitenRate).klangNamen);  // wie ohne Auskunft
        bool halb = false;
        for (const Stimme& stimme : ablauf.stimmen)
            for (const Schlag& schlag : stimme.schlaege) halb = halb || schlag.links == doctest::Approx(0.5);
        CHECK(halb);
        CHECK(tonImClip(loop, loop.muster.front(), 1, 0, 1).anschlagStufe == 0);
    }
    SUBCASE("die Auskunft eines echten Instruments")
    {
        Instrument instrument = fluegel();
        CHECK(auskunftVon(instrument).hammer);
        instrument.schichten[0].an = false;
        CHECK_FALSE(auskunftVon(instrument).hammer);
    }
}

TEST_CASE("Loop Hammer-Ton 31b-3 Rechenzeit (meldet nur Zahlen)")
{
    // entscheidet HA3 a (Stufen von 1 dB oder genau) und 31e (Mitschwingen im Ton oder auf der
    // Summe); nur unter Release-clang aussagekraeftig. Die Grenze von P6 a (im Bass hoechstens 4,6 % der
    // Echtzeit) prueft Claude am Ende jedes Teilschritts von Hand: drei Laeufe allein nach der Taktprobe
    // (RZ1-1 a, S153 — eine Eichung im selben Lauf bildet das Drosseln des Rechners nicht ab)
    const Instrument instrument = fluegel();
    struct Probe
    {
        int taste;
        double dauerMs;
        int stufe;
    };
    double summeMs = 0.0;
    double summeS = 0.0;
    for (const Probe& p : {Probe{21, 500.0, 0}, Probe{21, 4000.0, 0}, Probe{40, 500.0, 0}, Probe{40, 4000.0, -20},
                           Probe{60, 500.0, 0}, Probe{60, 4000.0, 0}, Probe{84, 500.0, 0}, Probe{96, 500.0, 0},
                           Probe{108, 500.0, 0}})
    {
        const auto anfang = std::chrono::steady_clock::now();
        const Klang k = ton(instrument, p.taste, p.dauerMs, p.stufe);
        const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - anfang).count();
        const double s = k.frames() / static_cast<double>(kSaitenRate);
        summeMs += ms;
        summeS += s;
        MESSAGE("Taste " << p.taste << ", gehalten " << p.dauerMs << " ms, Stufe " << p.stufe << ": " << ms << " ms fuer "
                         << s << " s Ton (" << ms / (1000.0 * s) * 100.0 << " % Echtzeit)");
    }
    MESSAGE("zusammen " << summeMs << " ms fuer " << summeS << " s Ton");
    CHECK(summeMs > 0.0);
}

TEST_CASE("Loop Hammer-Ton 31b Hoerproben (LUMIVIZ_31B_WAV)")
{
    const QString ordner = qEnvironmentVariable("LUMIVIZ_31B_WAV");
    if (ordner.isEmpty())
    {
        MESSAGE("LUMIVIZ_31B_WAV=<Ordner> legt Hoerproben ab");
        return;
    }
    REQUIRE(QDir().mkpath(ordner));
    const Instrument instrument = fluegel();
    const auto lege = [&](const QString& name, const std::vector<std::pair<int, int>>& toene, double dauerMs, double abstandS) {
        const auto abstand = static_cast<std::size_t>(abstandS * kSaitenRate);
        std::vector<float> folge;
        std::size_t nr = 0;
        for (const auto& [hoehe, stufe] : toene)
        {
            const Klang k = ton(instrument, hoehe, dauerMs, stufe);
            const std::size_t ab = 2 * nr * abstand;
            if (folge.size() < ab + k.daten.size()) folge.resize(ab + k.daten.size(), 0.0f);
            for (std::size_t i = 0; i < k.daten.size(); ++i) folge[ab + i] += k.daten[i] * 0.5f;
            ++nr;
        }
        REQUIRE(schreibeWav(QDir(ordner).filePath(name + QStringLiteral(".wav")), folge, kSaitenRate));
    };
    // die Tasten C1 … C8 je ff, mf, pp
    for (const auto& [name, stufe] : {std::pair{"01_c_ff", 0}, std::pair{"02_c_mf", -12}, std::pair{"03_c_pp", -26}})
    {
        std::vector<std::pair<int, int>> toene;
        for (int hoehe = 24; hoehe <= 108; hoehe += 12) toene.emplace_back(hoehe, stufe);
        lege(QString::fromLatin1(name), toene, 1200.0, 1.5);
    }
    // C4 lang: die zweite Stufe des Chors
    lege(QStringLiteral("04_c4_lang"), {{60, 0}}, 8000.0, 0.0);
    // A0, E1 (eine Saite), G6 (ohne Daempfer) kurz angeschlagen
    lege(QStringLiteral("05_kurz"), {{21, -6}, {28, -6}, {91, -6}, {96, -6}}, 250.0, 1.5);
    MESSAGE("Hoerproben in " << ordner.toStdString());
}

namespace
{

/// Die Toene des Bezugs (P8 a): eine Saite (A0, D#1), zwei (C2), drei (C4, C6, C7); drei
/// Stufen an C4; mit Daempfer (800 ms gehalten), lange gehalten (6 s) und ohne Daempfer (C7).
struct BezugTon
{
    int taste;
    int stufe;
    int dauerMs;
};

constexpr BezugTon kBezugToene[] = {{21, 0, 800},   {27, -12, 800}, {36, -12, 800},  {60, 0, 800}, {60, -12, 800},
                                    {60, -30, 800}, {60, -12, 6000}, {84, -12, 800}, {96, -12, 800}};

/// So breit ist ein Fenster des Pegelverlaufs, der mit der Pruefsumme abgelegt wird.
constexpr double kBezugFensterS = 0.5;
/// So weit darf der Pegelverlauf eines Tons im Chor neben dem Bezug liegen (P8 a). Nach dem Wechsel
/// auf `zerlegeKlein` (31d-7 A Punkt 3) lagen alle Fenster hoechstens 5·10⁻⁷ dB daneben (so genau
/// schreibt die Datei); mit der Laengskraft je Saite statt Y/N (Punkt 4, P5 a) bis 0,0037 dB.
constexpr double kBezugChorToleranzDb = 0.01;

struct BezugWert
{
    std::int64_t frames = 0;
    std::uint64_t pruefsumme = 0;  ///< FNV-1a ueber die Bytes aller Abtastwerte
    std::vector<double> fensterDb;  ///< Effektivwert je Fenster, beide Kanaele, in dB
};

std::string bezugName(const BezugTon& t)
{
    return "m" + std::to_string(t.taste) + ":a" + std::to_string(t.stufe) + ":d" + std::to_string(t.dauerMs);
}

const char* bezugCompiler()
{
#if defined(__clang__)
    return "clang";
#elif defined(_MSC_VER)
    return "msvc";
#else
    return "andere";
#endif
}

QString werteDatei(const QString& name)
{
    return QFileInfo(QString::fromLocal8Bit(__FILE__)).absoluteDir().filePath(QStringLiteral("werte/") + name);
}

BezugWert bezugWert(const Klang& klang)
{
    BezugWert wert;
    wert.frames = klang.frames();
    std::uint64_t summe = 1469598103934665603ull;
    for (const float abtastwert : klang.daten)
    {
        unsigned char bytes[sizeof(float)];
        std::memcpy(bytes, &abtastwert, sizeof(float));
        for (const unsigned char byte : bytes)
        {
            summe ^= byte;
            summe *= 1099511628211ull;
        }
    }
    wert.pruefsumme = summe;
    const auto breite = static_cast<std::int64_t>(kBezugFensterS * kSaitenRate);
    for (std::int64_t ab = 0; ab < wert.frames; ab += breite)
    {
        const std::int64_t bis = std::min(wert.frames, ab + breite);
        double quadrate = 0.0;
        for (std::int64_t i = 2 * ab; i < 2 * bis; ++i)
            quadrate += static_cast<double>(klang.daten[static_cast<std::size_t>(i)]) * klang.daten[static_cast<std::size_t>(i)];
        const double effektiv = std::sqrt(quadrate / static_cast<double>(2 * (bis - ab)));
        wert.fensterDb.push_back(effektiv > 0.0 ? 20.0 * std::log10(effektiv) : -300.0);
    }
    return wert;
}

/// Das Instrument des Bezugs: Hammer-Saite und Tastatur aus der Datei (gerichtet wie beim Laden).
Instrument bezugInstrument(const QJsonObject& o)
{
    Instrument instrument = fluegel();
    HammerSaite saite = hammerSaiteAusJson(o.value(QStringLiteral("hammerString")).toObject());
    richteHammerSaite(saite);
    // ohne zweite Ebene, gleich was die Vorgabe spaeter setzt (31d-7: die Datei kennt sie nicht)
    saite.polarisationWinkelGrad = {{{21, 0.0}, {108, 0.0}}, {}};
    saite.polarisationCent = {{{21, 0.0}, {108, 0.0}}, {}};
    saite.waagrechtVerhaeltnis = {{{21, 0.0}, {108, 0.0}}, {}};
    // ebenso ohne ungleichen Stoss (31d-7 B, US5 a 1)
    saite.stossLeise = {{{21, 0.0}, {108, 0.0}}, {}};
    saite.stossLaut = {{{21, 0.0}, {108, 0.0}}, {}};
    instrument.schichten[0].hammerSaite = saite;
    instrument.tastatur = tastaturAusJson(o.value(QStringLiteral("keyboard")).toObject());
    return instrument;
}

}  // namespace

TEST_CASE("Loop Hammer-Ton 31d-7 Bezug: dieselben Toene wie vor dem Umbau")
{
    const QString saitePfad = werteDatei(QStringLiteral("hammer_bezug_saite.json"));
    const QString wertePfad = werteDatei(QStringLiteral("hammer_bezug_%1.txt").arg(QLatin1String(bezugCompiler())));
    const bool schreiben = qEnvironmentVariableIntValue("LUMIVIZ_HAMMER_BEZUG_SCHREIBEN") == 1;

    if (schreiben && !QFileInfo::exists(saitePfad))
    {
        // die Saite nur einmal: sie haelt den Stand fest, an dem der Bezug haengt
        const QJsonObject o{{QStringLiteral("hammerString"), hammerSaiteAlsJson(hammerSaiteVorgabe())},
                            {QStringLiteral("keyboard"), tastaturAlsJson(Tastatur{})}};
        QFile datei(saitePfad);
        REQUIRE(datei.open(QIODevice::WriteOnly));
        datei.write(QJsonDocument(o).toJson(QJsonDocument::Indented));
        MESSAGE("Saite des Bezugs geschrieben: " << saitePfad.toStdString());
    }
    QFile saiteDatei(saitePfad);
    if (!saiteDatei.open(QIODevice::ReadOnly))
        FAIL("werte/hammer_bezug_saite.json fehlt - mit LUMIVIZ_HAMMER_BEZUG_SCHREIBEN=1 schreiben");
    const Instrument instrument = bezugInstrument(QJsonDocument::fromJson(saiteDatei.readAll()).object());

    std::map<std::string, BezugWert> werte;
    for (const BezugTon& t : kBezugToene) werte[bezugName(t)] = bezugWert(ton(instrument, t.taste, t.dauerMs, t.stufe));

    if (schreiben)
    {
        std::ofstream aus(wertePfad.toStdString(), std::ios::binary);
        REQUIRE(aus.good());
        aus << "# Bezug der Hammer-Toene (31d-7, P8 a) unter " << bezugCompiler()
            << " - geschrieben von test_LoopHammerTon.cpp (LUMIVIZ_HAMMER_BEZUG_SCHREIBEN=1)\n"
            << "# Ton\tFrames\tPruefsumme\tdB je " << kBezugFensterS << " s\n";
        aus.precision(6);
        aus << std::fixed;
        for (const auto& [name, wert] : werte)
        {
            aus << name << '\t' << wert.frames << '\t' << wert.pruefsumme << '\t';
            for (std::size_t i = 0; i < wert.fensterDb.size(); ++i) aus << (i > 0 ? "," : "") << wert.fensterDb[i];
            aus << '\n';
        }
        MESSAGE("Bezug geschrieben: " << werte.size() << " Toene in " << wertePfad.toStdString());
        return;
    }

    std::ifstream ein(wertePfad.toStdString(), std::ios::binary);
    if (!ein.good())
        FAIL("werte/hammer_bezug_" << bezugCompiler() << ".txt fehlt - mit LUMIVIZ_HAMMER_BEZUG_SCHREIBEN=1 schreiben");
    std::map<std::string, BezugWert> bezug;
    std::string zeile;
    while (std::getline(ein, zeile))
    {
        if (zeile.empty() || zeile[0] == '#') continue;
        std::istringstream teile(zeile);
        std::string name;
        std::string fenster;
        BezugWert wert;
        teile >> name >> wert.frames >> wert.pruefsumme >> fenster;
        std::istringstream liste(fenster);
        for (std::string zahl; std::getline(liste, zahl, ',');) wert.fensterDb.push_back(std::stod(zahl));
        bezug[name] = wert;
    }
    CHECK(bezug.size() == werte.size());
    for (const BezugTon& t : kBezugToene)
    {
        const std::string name = bezugName(t);
        CAPTURE(name);
        const BezugWert& wert = werte[name];
        const auto gefunden = bezug.find(name);
        REQUIRE(gefunden != bezug.end());
        const BezugWert& soll = gefunden->second;
        CHECK(wert.frames == soll.frames);
        if (chorAn(instrument.schichten[0].hammerSaite, t.taste) == 1)
        {
            // eine Saite ohne zweite Ebene: Bit fuer Bit (P5 a, P6 a)
            CHECK(wert.pruefsumme == soll.pruefsumme);
            continue;
        }
        // im Chor zerlegt der neue Loeser (P6 a): der Pegelverlauf mit Toleranz
        double groesste = 0.0;
        REQUIRE(wert.fensterDb.size() == soll.fensterDb.size());
        for (std::size_t i = 0; i < soll.fensterDb.size(); ++i)
            if (soll.fensterDb[i] > -120.0) groesste = std::max(groesste, std::abs(wert.fensterDb[i] - soll.fensterDb[i]));
        MESSAGE(name << ": hoechstens " << groesste << " dB neben dem Bezug (Fenster ueber −120 dB)");
        CHECK(groesste < kBezugChorToleranzDb);
    }
}

namespace
{

/// Eine Kurve, die ueberall denselben Wert hat.
Tastenkurve ueberall(double wert)
{
    return {{{kTasteMin, wert}, {kTasteMax, wert}}, {}};
}

/// Die Steigung des Grundtons in dB/s von `vonS` bis `bisS` (Ausgleichsgerade ueber Fenster von 0,2 s
/// alle 0,1 s) — positiv = er faellt.
double fallJeS(const Klang& klang, double hz, double vonS, double bisS)
{
    double sx = 0.0;
    double sy = 0.0;
    double sxx = 0.0;
    double sxy = 0.0;
    int anzahl = 0;
    for (double t = vonS; t <= bisS + 1e-9; t += 0.1)
    {
        const double y = pegel(klang, hz, t);
        sx += t;
        sy += y;
        sxx += t * t;
        sxy += t * y;
        ++anzahl;
    }
    return -(anzahl * sxy - sx * sy) / (anzahl * sxx - sx * sx);
}

/// Die tiefste Stelle der Huellkurve des Grundtons unter ihrer Umgebung (± 0,5 s), in Fenstern von
/// 0,05 s zwischen `vonS` und `bisS`, in dB.
double tiefsterEinbruch(const Klang& klang, double hz, double vonS, double bisS)
{
    std::vector<double> huelle;
    for (double t = vonS; t <= bisS; t += 0.05) huelle.push_back(db(betrag(klang, hz, t, 0.05)));
    double tiefste = 0.0;
    const int weit = 10;
    for (int i = 0; i < static_cast<int>(huelle.size()); ++i)
    {
        double oben = huelle[static_cast<std::size_t>(i)];
        for (int j = std::max(0, i - weit); j <= std::min(static_cast<int>(huelle.size()) - 1, i + weit); ++j)
            oben = std::max(oben, huelle[static_cast<std::size_t>(j)]);
        tiefste = std::max(tiefste, oben - huelle[static_cast<std::size_t>(i)]);
    }
    return tiefste;
}

}  // namespace

TEST_CASE("Loop Hammer-Ton 31d-7 Moden in zwei Ebenen")
{
    const double omega = 2.0 * kPi * 261.6;
    const std::vector<double> drei{omega * std::exp2(-1.0 / 1200.0), omega, omega * std::exp2(1.0 / 1200.0)};
    SUBCASE("mit γ = 0 bekommt die waagrechte Ebene nichts")
    {
        const auto moden = ebenenModen(drei, 1.5, 0.0, 0.2, 0.3, 0.05);
        REQUIRE(moden.size() == 3);  // die drei waagrechten fallen weg
        std::complex<double> summe(0.0, 0.0);
        for (const EbenenMode& m : moden)
        {
            summe += m.anteil;
            REQUIRE(m.form.size() == 6);
            for (int j = 0; j < 3; ++j) CHECK(std::abs(m.form[static_cast<std::size_t>(2 * j + 1)]) < 1e-12);
        }
        CHECK(std::abs(summe - 1.0) < 1e-9);
        // und dieselben Moden wie der Chor in einer Ebene
        const auto chor = chorModen(drei, 0.2, 0.3);
        for (const ChorMode& c : chor)
        {
            double naechste = 1e300;
            for (const EbenenMode& m : moden) naechste = std::min(naechste, std::abs(m.lambda - c.lambda));
            CHECK(naechste < 1e-9);
        }
    }
    SUBCASE("gedreht: Σ g = 1, beide Ebenen tragen")
    {
        const auto moden = ebenenModen(drei, 1.5, 30.0 * kPi / 180.0, 0.2, 0.3, 0.05);
        CHECK(moden.size() == 6);
        std::complex<double> summe(0.0, 0.0);
        for (const EbenenMode& m : moden) summe += m.anteil;
        CHECK(std::abs(summe - 1.0) < 1e-9);
    }
    SUBCASE("eine Saite gedreht, aber ohne δ: der Hammer trifft nur die gekoppelte Richtung")
    {
        const auto moden = ebenenModen({omega}, 0.0, 30.0 * kPi / 180.0, 0.2, 0.3, 0.0);
        REQUIRE(moden.size() == 1);
        CHECK(moden[0].lambda.real() == doctest::Approx(-0.5));
    }
}

TEST_CASE("Loop Hammer-Ton 31d-7 eine Saite: zwei Stufen nur mit γ und δ")
{
    // C4 mit einer Saite: keine Schwebung im Chor im Weg; δ = 3 Cent schwebt mit 0,45 Hz
    Instrument instrument = fluegel();
    HammerSaite& h = instrument.schichten[0].hammerSaite;
    h.chor = ueberall(1.0);
    const int taste = 60;
    const double hz = tonHz(taste);
    const auto stufen = [&](double gammaGrad, double deltaCent) {
        h.polarisationWinkelGrad = ueberall(gammaGrad);
        h.polarisationCent = ueberall(deltaCent);
        const Klang klang = ton(instrument, taste, 10000.0);
        return std::pair{fallJeS(klang, hz, 0.2, 1.5), fallJeS(klang, hz, 5.0, 9.0)};
    };
    const auto [frueh0, spaet0] = stufen(0.0, 0.0);
    const auto [frueh, spaet] = stufen(30.0, 3.0);
    const auto [fruehOhneDelta, spaetOhneDelta] = stufen(30.0, 0.0);
    MESSAGE("C4, eine Saite: γ = 0 " << frueh0 << " | " << spaet0 << " dB/s; γ = 30°, δ = 3 Cent " << frueh << " | " << spaet
                                     << " dB/s; γ = 30°, δ = 0 " << fruehOhneDelta << " | " << spaetOhneDelta << " dB/s");
    CHECK(spaet0 == doctest::Approx(frueh0).epsilon(0.1));  // eine Ebene: eine Stufe
    CHECK(spaet < 0.7 * frueh);                               // zwei Ebenen, verstimmt: zwei Stufen
    CHECK(spaetOhneDelta == doctest::Approx(fruehOhneDelta).epsilon(0.1));  // ohne δ keine zweite (Tan §3.5)
}

TEST_CASE("Loop Hammer-Ton 31d-7 die zweite Ebene fuellt die Einbrueche")
{
    // C4 mit drei Saiten: in einer Ebene loeschen sich die Moden an den Einbruechen fast aus (31d-6)
    Instrument instrument = fluegel();
    HammerSaite& h = instrument.schichten[0].hammerSaite;
    const int taste = 60;
    const double hz = tonHz(taste);
    // gemessen S153 (31d-7 A): in einer Ebene 30,7 dB; mit γ = 20°, δ = 2 Cent 9,6 dB (Salamander 10–20 dB);
    // δ = 0,5 Cent fuellt kaum (21,7 dB); γ = 45° vertieft sie (55,6 dB) — beide Richtungen koppeln dann
    // gleich und loeschen sich wie zwei gleiche Saiten aus
    const double ohne = tiefsterEinbruch(ton(instrument, taste, 8000.0), hz, 0.5, 7.5);
    h.polarisationWinkelGrad = ueberall(20.0);
    h.polarisationCent = ueberall(2.0);
    const double mit = tiefsterEinbruch(ton(instrument, taste, 8000.0), hz, 0.5, 7.5);
    MESSAGE("C4, drei Saiten: tiefster Einbruch in einer Ebene " << ohne << " dB, mit γ = 20°, δ = 2 Cent " << mit << " dB");
    CHECK(mit < ohne - 10.0);
}

TEST_CASE("Loop Hammer-Ton 31d-7 Phantome in der zweiten Stufe aus beiden Ebenen")
{
    // E1, eine Saite, der Phantomteilton bei f10 + f11 (wie 31b-2). In der zweiten Stufe traegt die
    // schwach gekoppelte Richtung: quer kommt am Steg nur ihr senkrechter Anteil c an, die Spannung kommt
    // aus beiden Ebenen (P5 a). Das Verhaeltnis Phantom / (Teilton 10 · Teilton 11) waechst von frueh
    // nach spaet um rund 1/c² — mit γ = 0 bleibt es.
    Instrument instrument = fluegel();
    HammerSaite& h = instrument.schichten[0].hammerSaite;
    const int taste = 28;
    const double hz = tonHz(taste);
    const double b = wertAn(h.inharmonizitaet, taste, KurvenSkala::Logarithmisch);
    const double f0 = hz / std::sqrt(1.0 + b);
    const auto fk = [&](int k) { return k * f0 * std::sqrt(1.0 + b * k * k); };
    h.stegWinkelGrad = ueberall(10.0);  // die Laengskraft deutlich ueber dem Leck der Fenster
    h.polarisationCent = ueberall(10.0);
    const auto wachsen = [&](double gammaGrad) {
        h.polarisationWinkelGrad = ueberall(gammaGrad);
        const Klang klang = ton(instrument, taste, 11000.0);
        // die Spitzen im Fenster gesucht: die zweite Richtung liegt um δ hoeher (bei f10 + f11 rund 5 Hz)
        const auto pegelBei = [&](double hzUngefaehr, double vonS) {
            return db(betrag(klang, spitze(klang, hzUngefaehr, 20.0, vonS, 1.0), vonS, 1.0));
        };
        const auto verhaeltnis = [&](double vonS) {
            return pegelBei(fk(10) + fk(11), vonS) - pegelBei(fk(10), vonS) - pegelBei(fk(11), vonS);
        };
        return verhaeltnis(9.0) - verhaeltnis(0.3);
    };
    const double ohne = wachsen(0.0);
    const double mit = wachsen(30.0);
    MESSAGE("E1, Phantom f10 + f11 gegen die Teiltoene, von 0,3 s nach 9 s: γ = 0 " << ohne << " dB, γ = 30° " << mit << " dB");
    CHECK(std::abs(ohne) < 3.0);
    CHECK(mit > ohne + 6.0);
}

TEST_CASE("Loop Hammer-Ton 31d-7 das waagrechte Verhaeltnis: wie gerechnet, nie langsamer als Late decay")
{
    // P3-1 a: σ_H = h·σ_V, der innere Verlust bleibt σ_s = σ_spät. (Befund S153 zu P3 a: mit
    // σ_s = (1 − h)·σ_spät fiel σ_s bei einer Saite ganz heraus, im Chor klangen die Gegenlagen langsamer
    // als *Late decay*.)
    Instrument instrument = fluegel();
    HammerSaite& h = instrument.schichten[0].hammerSaite;
    const int taste = 60;
    const double hz = tonHz(taste);
    SUBCASE("im Chor: mit und ohne h klingt nichts langsamer als σ_spät")
    {
        // an den Eigenwerten gemessen — am Ton schwebt die zweite Stufe des Chors langsamer als jedes
        // Fenster, eine Gerade dort misst Zufall. A = −σ_s·I − (positiv semidefinit): kein Re λ ueber −σ_s.
        const double omega = 2.0 * kPi * hz;
        const std::vector<double> drei{omega * std::exp2(-0.5 / 1200.0), omega, omega * std::exp2(0.5 / 1200.0)};
        const double sigmaFrueh = 1.3;
        const double sigmaSpaet = 0.43;
        const double sigmaV = (sigmaFrueh - sigmaSpaet) / 3.0;
        for (const double verhaeltnis : {0.0, 0.5})
        {
            CAPTURE(verhaeltnis);
            const auto moden = ebenenModen(drei, 1.5, 20.0 * kPi / 180.0, sigmaSpaet, sigmaV, verhaeltnis * sigmaV);
            double langsam = 1e300;
            for (const EbenenMode& m : moden) langsam = std::min(langsam, -m.lambda.real());
            CHECK(langsam >= sigmaSpaet - 1e-12);
        }
    }
    h.chor = ueberall(1.0);
    h.polarisationWinkelGrad = ueberall(30.0);
    h.polarisationCent = ueberall(3.0);
    // die langsamste Mode des Grundtons, wie der Ton sie rechnet (P3-1 a)
    const auto gerechnet = [&](double verhaeltnis) {
        const double frueh = wertAn(h.abklingS, taste, KurvenSkala::Logarithmisch);
        const double spaet = std::max(frueh, wertAn(h.abklingSpaetS, taste, KurvenSkala::Logarithmisch));
        const double sigmaFrueh = std::log(1000.0) / frueh;
        const double sigmaSpaet = std::log(1000.0) / spaet;
        const double sigmaV = sigmaFrueh - sigmaSpaet;
        const auto moden =
            ebenenModen({2.0 * kPi * hz}, 3.0, 30.0 * kPi / 180.0, sigmaSpaet, sigmaV, verhaeltnis * sigmaV);
        double langsam = 1e300;
        for (const EbenenMode& m : moden) langsam = std::min(langsam, -m.lambda.real());
        return 20.0 / std::log(10.0) * langsam;  // dB/s
    };
    const auto gemessen = [&](double verhaeltnis) {
        h.waagrechtVerhaeltnis = ueberall(verhaeltnis);
        return fallJeS(ton(instrument, taste, 14000.0), hz, 7.0, 13.0);
    };
    const double ohne = gemessen(0.0);
    const double mit = gemessen(0.5);
    MESSAGE("C4, eine Saite, γ = 30°: zweite Stufe h = 0 " << ohne << " dB/s (gerechnet " << gerechnet(0.0) << "), h = 0,5 "
                                                           << mit << " dB/s (gerechnet " << gerechnet(0.5) << ")");
    CHECK(ohne == doctest::Approx(gerechnet(0.0)).epsilon(0.05));
    CHECK(mit == doctest::Approx(gerechnet(0.5)).epsilon(0.05));
    CHECK(mit > ohne * 1.2);  // waagrechter Verlust am Steg verkuerzt die zweite Stufe
}

TEST_CASE("Loop Hammer-Ton 31d-7 B Stoss je Saite: die Moden")
{
    // US5 a 2: der Stoss je Saite aus u, und die Anteile vom Hammer zum Steg summieren sich bei γ = 0 fuer jedes u zu 1
    CHECK(stossJeSaite(1, 0.5).empty());
    CHECK(stossJeSaite(3, 0.0).empty());
    CHECK(stossJeSaite(2, 0.25) == std::vector<double>{1.25, 0.75});
    CHECK(stossJeSaite(3, 0.25) == std::vector<double>{1.25, 1.0, 0.75});
    const double omega = 2.0 * kPi * 261.6;
    const std::vector<double> drei{omega * std::exp2(-1.0 / 1200.0), omega, omega * std::exp2(1.0 / 1200.0)};
    for (const double u : {0.0, 0.3, 0.8})
    {
        CAPTURE(u);
        const auto moden = ebenenModen(drei, 1.5, 0.0, 0.2, 0.3, 0.05, stossJeSaite(3, u));
        std::complex<double> summe(0.0, 0.0);
        for (const EbenenMode& m : moden) summe += m.anteil;
        CHECK(std::abs(summe - 1.0) < 1e-9);
    }
}

TEST_CASE("Loop Hammer-Ton 31d-7 B Stoss je Saite: am Ton")
{
    // US5 a 3: im Chor flachere Einbrueche; bei einer Saite aendert u nichts; leise und laut an ihren Stufen
    Instrument instrument = fluegel();
    HammerSaite& h = instrument.schichten[0].hammerSaite;
    const int taste = 60;
    const double hz = tonHz(taste);
    SUBCASE("C4, drei Saiten: der ungleiche Stoss fuellt die Einbrueche")
    {
        const double gleich = tiefsterEinbruch(ton(instrument, taste, 8000.0), hz, 0.5, 7.5);
        h.stossLeise = ueberall(0.25);
        h.stossLaut = ueberall(0.25);
        const double ungleich = tiefsterEinbruch(ton(instrument, taste, 8000.0), hz, 0.5, 7.5);
        MESSAGE("C4, drei Saiten: tiefster Einbruch bei gleichem Stoss " << gleich << " dB, mit u = 0,25 " << ungleich << " dB");
        CHECK(ungleich < gleich - 10.0);
    }
    SUBCASE("eine Saite: u aendert nichts, Bit fuer Bit")
    {
        h.chor = ueberall(1.0);
        const Klang ohne = ton(instrument, taste, 3000.0);
        h.stossLeise = ueberall(0.5);
        h.stossLaut = ueberall(0.7);
        const Klang mit = ton(instrument, taste, 3000.0);
        REQUIRE(ohne.daten.size() == mit.daten.size());
        CHECK(std::memcmp(ohne.daten.data(), mit.daten.data(), ohne.daten.size() * sizeof(float)) == 0);
    }
    SUBCASE("leise an Stufe −16, laut an −4, dazwischen linear, an den Grenzen fest")
    {
        const auto mitU = [&](double leise, double laut, int stufe) {
            h.stossLeise = ueberall(leise);
            h.stossLaut = ueberall(laut);
            return ton(instrument, taste, 1500.0, stufe);
        };
        const auto gleich = [](const Klang& a, const Klang& b) {
            REQUIRE(a.daten.size() == b.daten.size());
            float groesste = 0.0f;
            float abstand = 0.0f;
            for (std::size_t i = 0; i < a.daten.size(); ++i)
            {
                groesste = std::max(groesste, std::abs(a.daten[i]));
                abstand = std::max(abstand, std::abs(a.daten[i] - b.daten[i]));
            }
            return abstand <= 1e-5f * groesste;
        };
        CHECK(gleich(mitU(0.5, 0.0, kStossStufeLeise), mitU(0.5, 0.5, kStossStufeLeise)));
        CHECK(gleich(mitU(0.5, 0.0, kStossStufeLaut), mitU(0.0, 0.0, kStossStufeLaut)));
        CHECK(gleich(mitU(0.5, 0.0, -10), mitU(0.25, 0.25, -10)));  // die Mitte
        CHECK(gleich(mitU(0.5, 0.0, -30), mitU(0.5, 0.5, -30)));    // unter leise: leise
        CHECK(gleich(mitU(0.5, 0.0, 0), mitU(0.0, 0.0, 0)));        // ueber laut: laut
        CHECK_FALSE(gleich(mitU(0.5, 0.0, -10), mitU(0.0, 0.0, -10)));  // der Stoss wirkt ueberhaupt
    }
}
