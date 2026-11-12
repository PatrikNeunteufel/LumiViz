/**
 ****************************************************************************************
 * @file   test_LoopZunge.cpp
 * @brief  Stufe 34j-7, 34j-8: der Kern `freeReed` im Ton der App — laufender Wind und die mitgelieferten Instrumente
 *         `accordion`, `harmonium`, `melodica`, `harmonica` (docs/audio/Instrumente_IC_Konzept.md 34j-7, 34j-8, ZV3 a,
 *         SK4 a, IK2-8 a)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Ohne Qt-Anwendung, ohne Ton, ohne Engine. Liest vom Arbeitsordner aus — die Test-Exe aus dem Projekt-Root starten.
 *
 * - **Schreiben** (Muster `test_LoopStab.cpp`): `LUMIVIZ_ZUNGE_INSTRUMENTE_SCHREIBEN=1` schreibt die vier Vorlagen als
 *   `asset/loop/instruments/<name>.lvinst` mit dem Schreiber des Programms — ohne gestimmte Werte und Bezug. Danach je
 *   Datei `InstrumentTuneTool <datei> --all --reference` und `python asset/loop/instrument_katalog.py`.
 * - **Waechter** (ohne zu stimmen): jede Datei lesbar, Kern `freeReed`, ohne Pedale, ihre Werte die der Vorlage (bis auf
 *   die gestimmten und den Bezug), die gestimmten Kurven und der Bezug Bit fuer Bit wie `werte/zunge_ic.txt` (die
 *   schreibt der Stimmer, `test_InstrumentModelsZunge`), der Ton ueber den Kern Bit fuer Bit wie der Abdruck dort.
 * - Die Faelle der App spielen die mitgelieferten Dateien, wie das Programm.
 *
 * - Ein Kern mit dauernder Anregung bekommt Expression, Pitch und Vibrato der Note als Spuren in den Auftrag; die
 *   Expression wirkt nicht als Pegel hinter dem Ton: eine Expression des Musters gleich einem festen Wert der Spur des
 *   Kerns, Bit fuer Bit.
 * - Ein Crescendo am gehaltenen Ton macht ihn lauter und heller; die Spur Pitch senkt die Tonhoehe ueber das Ventil.
 * - 34j-9: die Hoerliste `zunge_34j` (`LUMIVIZ_34J_WAV=<Hoerort>`) und die Rechenzeit (nur Zahlen).
 ****************************************************************************************
 */

#include <doctest.h>

#include "AudioCore/Lautheit.hpp"
#include "HoerlisteAblage.hpp"
#include "HoerlistePegel.hpp"
#include "InstrumentCore/Pruefsumme.hpp"
#include "InstrumentMeasure/Bausteine.hpp"
#include "InstrumentMeasure/ZungeStimmen.hpp"
#include "InstrumentModels/Kerne.hpp"
#include "InstrumentModels/zunge/ZungeKern.hpp"
#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"
#include "loop/Tastatur.hpp"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <complex>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>
#include <numbers>
#include <sstream>
#include <string>
#include <vector>

using namespace lumi::loop;
namespace kern = lumi::kern;
namespace zunge = lumi::kern::zunge;

namespace
{

constexpr int kRate = 48000;

const zunge::Vorlage& vorlage(std::string_view name)
{
    for (const zunge::Vorlage& v : zunge::vorlagen())
        if (v.name == name) return v;
    FAIL("keine Vorlage " << name);
    return zunge::vorlagen().front();
}

QString dateiVon(const zunge::Vorlage& v)
{
    return QStringLiteral("asset/loop/instruments/") + QString::fromStdString(v.name) + QStringLiteral(".lvinst");
}

/// Ohne Pedale (eine Zunge hat keinen Daempfer; den Umfang traegt die Tafel, KL4 a).
Tastatur tastaturVon(const zunge::ZungenSpiel& s)
{
    Tastatur t;
    t.links = PedalArt::Keins;
    t.mitte = PedalArt::Keins;
    t.rechts = PedalArt::Keins;
    t.bassSustainBis = s.noteMin;
    richteTastatur(t);
    return t;
}

/// Das Instrument einer Vorlage, wie es das Programm schreibt — ohne gestimmte Werte und Bezug (die setzt das Werkzeug).
Instrument zumSchreiben(const zunge::Vorlage& v)
{
    Instrument i;
    i.titel = QString::fromStdString(v.titel);
    Schicht s;
    s.name = i.titel;
    s.kernName = QString::fromUtf8(zunge::kKernName.data(), static_cast<qsizetype>(zunge::kKernName.size()));
    s.kernWerte = zunge::werteVon(v);  // mit den Quellen der Vorlage (QM1 a)
    std::vector<std::string> fehler;
    richteSchicht(s, &fehler);  // meldet die leeren gestimmten — die stimmt das Werkzeug
    i.schichten[0] = s;
    i.tastatur = tastaturVon(v.spiel);
    return i;
}

/// Die mitgelieferte Datei einer Vorlage, wie das Programm sie liest.
Instrument liesDatei(const zunge::Vorlage& v)
{
    QFile datei(dateiVon(v));
    Instrument i;
    REQUIRE_MESSAGE(datei.open(QIODevice::ReadOnly), v.name << " nicht gefunden — LUMIVIZ_ZUNGE_INSTRUMENTE_SCHREIBEN=1, dann InstrumentTuneTool");
    REQUIRE_MESSAGE(ausJson(QJsonDocument::fromJson(datei.readAll()).object(), i),
                    v.name << " nicht lesbar (nicht gestimmt?) — InstrumentTuneTool <datei> --all --reference");
    i.name = QString::fromStdString(v.name);
    return i;
}

/// Das Instrument einer Vorlage aus der mitgelieferten Datei (34j-8; bis dahin am Modell gestimmt).
Instrument instrumentVon(std::string_view name)
{
    return liesDatei(vorlage(name));
}

double ausHex(const std::string& h)
{
    const std::uint64_t b = std::stoull(h, nullptr, 16);
    double x = 0.0;
    std::memcpy(&x, &b, sizeof x);
    return x;
}

std::string hexVon(double x)
{
    std::uint64_t b = 0;
    std::memcpy(&b, &x, sizeof b);
    char t[32];
    std::snprintf(t, sizeof t, "%016llx", static_cast<unsigned long long>(b));
    return t;
}

/// Die Bezugsdatei des Kerns (`werte/zunge_ic.txt`): Abdruecke, Bezuege und die gestimmten Kurven je Instrument.
struct Beleg
{
    std::map<std::string, std::string> abdruck, bezugLv;
    std::map<std::string, std::map<std::string, std::vector<kern::NotenWert>>> kurven;
};

Beleg liesBeleg()
{
    Beleg r;
    std::ifstream f(std::filesystem::path(__FILE__).parent_path() / "werte" / "zunge_ic.txt");
    std::string zeile;
    while (std::getline(f, zeile))
    {
        std::istringstream z(zeile);
        std::string art, name;
        z >> art >> name;
        if (art == "abdruck" || art == "bezugLv")
            z >> (art == "abdruck" ? r.abdruck : r.bezugLv)[name];
        else if (art == "kurve")
        {
            std::string welche, hex;
            int note = 0;
            z >> welche >> note >> hex;
            r.kurven[name][welche].push_back(kern::NotenWert{note, ausHex(hex)});
        }
    }
    return r;
}

const Instrument& akkordeon()
{
    static const Instrument i = instrumentVon("accordion");
    return i;
}

const Instrument& melodica()
{
    static const Instrument i = instrumentVon("melodica");
    return i;
}

/// Der linke Kanal eines Tons.
std::vector<double> links(const Klang& k)
{
    std::vector<double> m;
    for (std::size_t i = 0; i + 1 < k.daten.size(); i += 2) m.push_back(k.daten[i]);
    return m;
}

double rmsDb(const std::vector<double>& x, double vonS, double bisS)
{
    const auto a = static_cast<std::size_t>(vonS * kRate), b = std::min(x.size(), static_cast<std::size_t>(bisS * kRate));
    double s = 0.0;
    for (std::size_t i = a; i < b; ++i) s += x[i] * x[i];
    return 10.0 * std::log10(s / static_cast<double>(std::max<std::size_t>(1, b - a)) + 1e-30);
}

/// Der Schwerpunkt des Spektrums ueber die ersten zwanzig Teiltoene (Fenster nach Hann).
double schwerpunktHz(const std::vector<double>& x, double vonS, double f0)
{
    const auto a = static_cast<std::size_t>(vonS * kRate);
    const std::size_t n = 9600;
    double summe = 0.0, gewicht = 0.0;
    for (int k = 1; k <= 20; ++k)
    {
        std::complex<double> s(0.0, 0.0);
        for (std::size_t i = 0; i < n && a + i < x.size(); ++i)
            s += (0.5 - 0.5 * std::cos(2.0 * std::numbers::pi * i / n)) * x[a + i] *
                 std::exp(std::complex<double>(0.0, -2.0 * std::numbers::pi * k * f0 * static_cast<double>(i) / kRate));
        summe += k * f0 * std::abs(s);
        gewicht += std::abs(s);
    }
    return summe / gewicht;
}

double tonhoeheHz(const std::vector<double>& x, double vonS, double bisS)
{
    const std::vector<double> stueck(x.begin() + static_cast<std::ptrdiff_t>(vonS * kRate), x.begin() + static_cast<std::ptrdiff_t>(bisS * kRate));
    const lumi::messung::Tonhoehe th = lumi::messung::tonhoehe(stueck, kRate, 30.0, 3000.0);
    std::vector<double> hz;
    for (const double f : th.hz)
        if (f > 0.0) hz.push_back(f);
    REQUIRE_FALSE(hz.empty());
    std::nth_element(hz.begin(), hz.begin() + static_cast<std::ptrdiff_t>(hz.size() / 2), hz.end());
    return hz[hz.size() / 2];
}

TonAngabe ton(int hoehe, int dauerMs)
{
    TonAngabe t;
    t.hoehe = hoehe;
    t.dauerMs = dauerMs;
    return t;
}

}  // namespace

TEST_CASE("Loop Zunge 34j-8 die mitgelieferten Zungeninstrumente tragen ihre Vorlage und ihre Stimmung")
{
    if (qEnvironmentVariableIntValue("LUMIVIZ_ZUNGE_INSTRUMENTE_SCHREIBEN") == 1)
        for (const zunge::Vorlage& v : zunge::vorlagen())
        {
            QFile aus(dateiVon(v));
            REQUIRE_MESSAGE(aus.open(QIODevice::WriteOnly | QIODevice::Truncate), "asset/loop/instruments nicht gefunden — aus dem Projekt-Root starten");
            REQUIRE(aus.write(QJsonDocument(alsJson(zumSchreiben(v))).toJson(QJsonDocument::Indented)) > 0);
            MESSAGE(v.name << ".lvinst geschrieben — danach InstrumentTuneTool <datei> --all --reference, dann instrument_katalog.py");
        }

    const Beleg b = liesBeleg();
    REQUIRE_MESSAGE(b.bezugLv.size() == 4, "werte/zunge_ic.txt fehlt");
    const kern::Parametertafel& tafel = zunge::kernTyp().tafel;
    int geprueft = 0;
    for (const zunge::Vorlage& v : zunge::vorlagen())
    {
        CAPTURE(v.name);
        const Instrument ist = liesDatei(v);
        REQUIRE(ist.schichten.size() == 1);
        const Schicht& s = ist.schichten[0];
        REQUIRE(istKernSchicht(s));
        CHECK(s.kernName == QStringLiteral("freeReed"));
        CHECK(s.koerper.isEmpty());
        REQUIRE(ist.tastatur.has_value());
        CHECK(*ist.tastatur == tastaturVon(v.spiel));
        CHECK(kern::ungemessen(tafel, s.kernWerte));
        kern::Werte gerichtet = s.kernWerte;
        CHECK(kern::richteWerte(tafel, gerichtet).empty());  // vollstaendig gestimmt, mit Bezug
        ++geprueft;

        // die Werte der Vorlage (gerichtet), ohne die gestimmten und den Bezug
        kern::Werte soll = zunge::werteVon(v);
        (void)kern::richteWerte(tafel, soll);  // meldet die leeren gestimmten — die stimmt das Werkzeug
        for (const kern::ParameterAngabe& a : tafel.werte)
        {
            if (a.gestimmt || a.name == tafel.bezugWert) continue;
            CAPTURE(a.name);
            const kern::Wert* w = s.kernWerte.finde(a.name);
            REQUIRE(w != nullptr);
            CHECK_MESSAGE(*w == *soll.finde(a.name), "veraltet — LUMIVIZ_ZUNGE_INSTRUMENTE_SCHREIBEN=1, InstrumentTuneTool, instrument_katalog.py");
        }
        // die Stimmung des Werkzeugs gleich der des Stimmers in der Bezugsdatei, Punkt fuer Punkt; der Bezug ebenso
        for (const std::string& n : zunge::gestimmteNamen())
        {
            CAPTURE(n);
            CHECK_MESSAGE(s.kernWerte.kurve(n).punkte == b.kurven.at(v.spiel.name).at(n), "veraltet — InstrumentTuneTool <datei> --all --reference");
        }
        CHECK(hexVon(s.kernWerte.zahl(tafel.bezugWert)) == b.bezugLv.at(v.spiel.name));
        // der Ton ueber den Kern aus den Werten der Datei: ff an der Mitte, 0,4 s gehalten, wie der Abdruck `_e1`
        const std::shared_ptr<const kern::Tonkern> k = kern::kerne().erzeuge(zunge::kKernName, s.kernWerte);
        REQUIRE(k != nullptr);
        const int tief = std::max(v.spiel.noteMin, v.spiel.bassAb), mitte = (tief + v.spiel.noteMax) / 2;
        std::vector<float> ton;
        k->rechne(kern::einzelnote(mitte, 440.0 * std::exp2((mitte - 69) / 12.0), 0, 19200, 28800, 48000.0), ton);
        CHECK(kern::abdruck(ton) == b.abdruck.at("zunge_" + v.spiel.name + "_m" + std::to_string(mitte) + "_e1"));
    }
    CHECK(geprueft == 4);
}

TEST_CASE("Loop Zunge 34j-7: die Expression ist der Wind, kein Pegel (ZV3 a, SK4 a)")
{
    const Instrument& a = akkordeon();
    REQUIRE(anregungDauernd(a.schichten[0]));
    // die Expression des Musters, fest 0,5, gleich dem festen Wert der Spur des Kerns — beide gehen als Spur in den
    // Auftrag, keine wird als Pegel hinter den Ton gelegt
    TonAngabe muster = ton(69, 800);
    muster.ausdruckMuster = {{0, 500}};
    TonAngabe fest = ton(69, 800);
    fest.kernSpuren = {{QStringLiteral("expression"), 0.5}};
    const Klang km = rendereTon(a, muster, kRate), kf = rendereTon(a, fest, kRate);
    REQUIRE_FALSE(km.daten.empty());
    CHECK(km.daten == kf.daten);
    // Muster mal Clip: 0,5 mal 1 ist dasselbe; 0,5 mal 0,5 ist leiser, aber nicht um e² = −12 dB des Pegels, sondern wie der
    // Balg bei der Staerke 0,25 (S191: rund −10 LU gegen 1, −4 gegen 0,5)
    TonAngabe beide = muster;
    beide.ausdruckClip = {{0, 1000}};
    CHECK(rendereTon(a, beide, kRate).daten == km.daten);
    beide.ausdruckClip = {{0, 500}};
    const std::vector<double> l05 = links(km), l025 = links(rendereTon(a, beide, kRate));
    const double unterschied = rmsDb(l025, 0.4, 0.8) - rmsDb(l05, 0.4, 0.8);
    MESSAGE("Akkordeon A4: Expression 0,25 gegen 0,5 " << unterschied << " dB");
    CHECK(unterschied < -1.0);
    CHECK(unterschied > -10.0);
    // eine Schicht ohne dauernde Anregung behaelt den Pegel (SK12 a) — der Kern `bar` mit Bogen ist nicht betroffen
    CHECK_FALSE(kern::kerne().typ("bar")->auskunft.anregungDauernd);
}

TEST_CASE("Loop Zunge 34j-7: ein Crescendo am gehaltenen Ton wird lauter und heller")
{
    const Instrument& m = melodica();
    TonAngabe t = ton(69, 2000);
    t.ausdruckMuster = {{0, 0}, {1500, 1000}};
    const std::vector<double> x = links(rendereTon(m, t, kRate));
    const double leise = rmsDb(x, 0.25, 0.45), laut = rmsDb(x, 1.55, 1.75);
    // ZV14 a (S192): die Teiltoene an der gespielten Tonhoehe gelesen — sie folgt dem Wind (ZV5 a); an der Sollfrequenz
    // fielen bei ff die hohen Teiltoene aus dem Fenster (Befund S191 „lauter wird dunkler“, ZV12 a)
    const double fLeise = tonhoeheHz(x, 0.25, 0.45), fLaut = tonhoeheHz(x, 1.55, 1.75);
    const double hellLeise = schwerpunktHz(x, 0.25, fLeise), hellLaut = schwerpunktHz(x, 1.55, fLaut);
    MESSAGE("Melodica A4 Crescendo: " << laut - leise << " dB lauter, " << fLeise << " → " << fLaut << " Hz, Schwerpunkt " << hellLeise << " → "
                                      << hellLaut << " Hz");
    CHECK(laut - leise > 8.0);
    CHECK(fLaut < fLeise);  // die Tonhoehe faellt mit dem Druck (ZV5 a)
    // belegt ist die Richtung (Puranik & Scavone 2023: die Zungenbewegung wird mit dem Druck obertonreicher), nicht das
    // Mass; gemessen S192 1,08-fach (das Fenster bei 0,25 s liegt bei Expression rund 0,17; Staerke 0 → 1 am Kern 1,22)
    CHECK(hellLaut > 1.03 * hellLeise);
    // ohne Verlauf, fest bei 0 und bei 1: das Crescendo liegt zwischen beiden
    TonAngabe pp = ton(69, 2000), ff = ton(69, 2000);
    pp.ausdruckMuster = {{0, 0}};
    ff.ausdruckMuster = {{0, 1000}};
    CHECK(rmsDb(links(rendereTon(m, pp, kRate)), 1.55, 1.75) < laut);
    CHECK(rmsDb(links(rendereTon(m, ff, kRate)), 1.55, 1.75) > leise);
}

TEST_CASE("Loop Zunge 34j-7: die Spur Pitch senkt die Tonhoehe ueber das Ventil, das Vibrato laeuft mit")
{
    const Instrument& m = melodica();
    // am mittleren Druck (Expression 0,5), wo der Stimmer die Kurve misst (ZV8 a)
    TonAngabe t = ton(60, 2000);
    t.ausdruckMuster = {{0, 500}};
    t.tonhoehe = {{0, 0}, {600, 0}, {1200, -20}};
    const std::vector<double> x = links(rendereTon(m, t, kRate));
    const double vorher = tonhoeheHz(x, 0.3, 0.6), nachher = tonhoeheHz(x, 1.4, 1.9);
    const double cent = 1200.0 * std::log2(nachher / vorher);
    // mit vollem Wind (ohne Expression) wird derselbe Bend tiefer (ZV7 a)
    TonAngabe ff = t;
    ff.ausdruckMuster.clear();
    const std::vector<double> xf = links(rendereTon(m, ff, kRate));
    const double centFf = 1200.0 * std::log2(tonhoeheHz(xf, 1.4, 1.9) / tonhoeheHz(xf, 0.3, 0.6));
    MESSAGE("Melodica C4: Pitch −20 Cent ueber die Spur ergibt " << cent << " Cent in der Mitte, " << centFf << " Cent bei vollem Wind");
    CHECK(cent < -15.0);
    CHECK(cent > -25.0);
    CHECK(centFf < cent);
    // ohne Spur Pitch derselbe Ton bis 600 ms (die Spur steht dort bei 0)
    TonAngabe ohneSpur = ton(60, 2000);
    ohneSpur.ausdruckMuster = {{0, 500}};
    const std::vector<double> ohne = links(rendereTon(m, ohneSpur, kRate));
    CHECK(std::equal(x.begin(), x.begin() + 600 * kRate / 1000, ohne.begin()));
    // das Vibrato als Spur: aendert den Ton erst, wo es einsetzt
    TonAngabe v = ohneSpur;
    v.vibrato = {{0, 0}, {1000, 0}, {1001, 30}};
    const std::vector<double> xv = links(rendereTon(m, v, kRate));
    CHECK(std::equal(xv.begin(), xv.begin() + kRate, ohne.begin()));
    CHECK_FALSE(std::equal(xv.begin(), xv.end(), ohne.begin()));
}

TEST_CASE("Loop Zunge 34j-9 Hoerproben (LUMIVIZ_34J_WAV) — Liste fuer HoerTest_Helper: je Instrument drei Lagen und drei Staerken, Wind, Musette")
{
    // Werkzeug (Teilplan 34j-9, Muster `Loop Klasse1 34i Hoerproben`): schreibt die Liste im Format von HoerTest_Helper
    // (dessen Konzept §11, §12, Schema 1) in den Ordner der Variable — der Hoerort
    // `…\cmake\samples\Hoerproben\LumiViz\zunge_34j` (AU3), Liste und Abdruecke danach auch unter `hoertest/listen/zunge_34j`
    // (AU3-1 a). Sie loest IC `10_zunge` ab (LG1 a). Gerechnet wie im Programm (rendereTon aus der Datei), jeder Klang auf
    // einen gemeinsamen Pegel (hoechstens −23 LUFS, ohne Spitze ueber 0,89). Je Instrument tief / mitte / hoch bei den
    // Stufen −18 / −9 / 0 (2 s), ein Crescendo ueber die Expression (0 → 1 in 2,5 s), Vibrato (20 Cent, 5 Hz); dazu die
    // zwei offenen Befunde aus ZV14 a (Crescendo an G3 bei Akkordeon und Melodica), der Balgwechsel am Akkordeon (ueber den
    // Kern, die Spur `bellows` kommt mit 31g in den Loop), die Bass-Seite und Musette gegen einen Chor. Ohne die Variable
    // meldet er das und prueft nichts. Pruefen: `Hoertest.exe --liste-pruefen <ordner>/liste.json`.
    const QString ordner = qEnvironmentVariable("LUMIVIZ_34J_WAV");
    if (ordner.isEmpty())
    {
        MESSAGE("LUMIVIZ_34J_WAV nicht gesetzt — keine Hoerproben");
        return;
    }
    const QDir dir(ordner);
    REQUIRE(QDir().mkpath(dir.filePath(QStringLiteral("klaenge"))));
    constexpr int kHaltenMs = 2000;
    constexpr double kZielHoechstens = -23.0;
    constexpr double kSpitzeHoechstens = 0.89;
    struct Staerke
    {
        const char* name;
        int stufe;
    };
    const Staerke staerken[] = {{"a-18", -18}, {"a-9", -9}, {"a0", 0}};
    QJsonArray klaenge, messwerte, punkte;
    QStringList ids;
    std::vector<hoerliste::Klang> gerechnet;
    const auto eintragen = [&](const QString& id, std::vector<float> daten, const QJsonObject& achsen, const QString& fassung) {
        REQUIRE_FALSE(ids.contains(id));
        ids << id;
        const lumi::audio::Lautheit vorher = lumi::audio::messeLautheit(daten, kRate);
        const QString datei = QStringLiteral("klaenge/") + id + QStringLiteral(".wav");
        gerechnet.push_back(hoerliste::Klang{id, datei, std::move(daten)});
        klaenge.append(QJsonObject{{QStringLiteral("id"), id}, {QStringLiteral("datei"), datei}, {QStringLiteral("fassung"), fassung},
                                   {QStringLiteral("achsen"), achsen}});
        messwerte.append(QJsonObject{{QStringLiteral("achsen"), achsen},
                                     {QStringLiteral("fassung"), fassung},
                                     {QStringLiteral("werte"), QJsonArray{QJsonObject{{QStringLiteral("name"), QStringLiteral("lufs_vorher")},
                                                                                      {QStringLiteral("wert"), vorher.stille ? -300.0 : std::round(vorher.lufs * 100.0) / 100.0},
                                                                                      {QStringLiteral("einheit"), QStringLiteral("LUFS")}}}}});
    };
    const auto achsenVon = [](const Instrument& inst, int hoehe, const QString& staerke, const QString& spielweise) {
        QJsonObject a{{QStringLiteral("instrument"), inst.titel}, {QStringLiteral("taste"), hoehe}, {QStringLiteral("ton"), tonName(hoehe)},
                      {QStringLiteral("staerke"), staerke}};
        if (!spielweise.isEmpty()) a[QStringLiteral("spielweise")] = spielweise;
        return a;
    };
    // ein Ton wie im Programm; `spielweise` leer: nur Lage und Staerke
    const auto ablegen = [&](const Instrument& inst, const TonAngabe& t, const Staerke& st, const QString& fassung, const QString& spielweise = {}) {
        const QString id = inst.name + QStringLiteral("_") + QString::number(t.hoehe) + QStringLiteral("_") + QString::fromLatin1(st.name) +
                           (spielweise.isEmpty() ? QString() : QStringLiteral("_") + spielweise);
        TonAngabe a = t;
        a.instrument = inst.name;
        a.anschlagStufe = st.stufe;
        eintragen(id, rendereTon(inst, a, kRate).daten, achsenVon(inst, t.hoehe, QString::fromLatin1(st.name), spielweise), fassung);
        return id;
    };
    const auto punktId = [&] { return QStringLiteral("p%1").arg(punkte.size() + 1, 3, 10, QLatin1Char('0')); };
    const auto einzel = [&](const QString& id, const QString& frage) {
        punkte.append(QJsonObject{{QStringLiteral("id"), punktId()},
                                  {QStringLiteral("art"), QStringLiteral("einzel")},
                                  {QStringLiteral("klang"), id},
                                  {QStringLiteral("frage"), frage},
                                  {QStringLiteral("pole"), QJsonArray{QStringLiteral("nein"), QStringLiteral("ja")}}});
    };
    const auto paar = [&](const QString& a, const QString& b, const QString& frage) {
        punkte.append(QJsonObject{{QStringLiteral("id"), punktId()},
                                  {QStringLiteral("art"), QStringLiteral("paar")},
                                  {QStringLiteral("klaenge"), QJsonArray{a, b}},
                                  {QStringLiteral("vorzug"), frage}});
    };
    const std::string namen[] = {"accordion", "harmonium", "melodica", "harmonica"};
    // je Instrument drei Lagen und drei Staerken
    for (const std::string& name : namen)
    {
        const Instrument inst = instrumentVon(name);
        const zunge::ZungenSpiel& s = vorlage(name).spiel;
        for (const int hoehe : {s.noteMin + 5, (s.noteMin + s.noteMax) / 2, s.noteMax - 5})
            for (const Staerke& st : staerken)
                einzel(ablegen(inst, ton(hoehe, kHaltenMs), st, QStringLiteral("vorgabe")),
                       QStringLiteral("Klingt es nach %1 (ungemessen)? Farbe ueber die Staerke glaubhaft?").arg(inst.titel));
    }
    // der Wind: Crescendo und Vibrato an der Mitte; die Befunde aus ZV14 a an G3 (Akkordeon E3–A3, Melodica F3–A3 werden
    // mit dem Wind leicht dunkler)
    for (const std::string& name : namen)
    {
        const Instrument inst = instrumentVon(name);
        const zunge::ZungenSpiel& s = vorlage(name).spiel;
        const int mitte = (s.noteMin + s.noteMax) / 2;
        std::vector<int> tasten{mitte};
        if (name == "accordion" || name == "melodica") tasten.push_back(55);
        for (const int hoehe : tasten)
        {
            TonAngabe t = ton(hoehe, 3000);
            t.ausdruckMuster = {{0, 0}, {2500, 1000}};
            einzel(ablegen(inst, t, staerken[2], QStringLiteral("wind"), QStringLiteral("crescendo")),
                   hoehe == mitte ? QStringLiteral("Waechst der Ton glaubhaft mit dem Wind (lauter, heller)?")
                                  : QStringLiteral("Waechst der Ton glaubhaft mit dem Wind? (Befund ZV14 a: hier wird er leicht dunkler)"));
        }
        TonAngabe v = ton(mitte, kHaltenMs);
        v.vibrato = {{0, 20}};
        einzel(ablegen(inst, v, staerken[1], QStringLiteral("wind"), QStringLiteral("vibrato")),
               QStringLiteral("Klingt das Vibrato (Wind, 5 Hz) glaubhaft?"));
    }
    const Instrument& akk = akkordeon();
    // der Balgwechsel ueber den Kern (der Loop kennt die Spur erst mit 31g): 1 s ziehen, dann schieben
    {
        const int hoehe = 69;
        const std::shared_ptr<const kern::Tonkern> k = kern::kerne().erzeuge(zunge::kKernName, akk.schichten[0].kernWerte);
        REQUIRE(k != nullptr);
        kern::Tonauftrag a = kern::einzelnote(hoehe, 440.0, -9, 2 * kRate, 0, kRate);
        a.frames = 2 * kRate + k->nachklang(a, 4 * kRate);
        a.spuren.push_back(kern::Spur{std::string(zunge::kSpurBalg), {{0, 0.0}, {kRate - 1, 0.0}, {kRate, 1.0}}});
        std::vector<float> mono;
        k->rechne(a, mono);
        std::vector<float> stereo(2 * mono.size());
        for (std::size_t i = 0; i < mono.size(); ++i) stereo[2 * i] = stereo[2 * i + 1] = mono[i];
        const QString id = akk.name + QStringLiteral("_69_a-9_balgwechsel");
        eintragen(id, std::move(stereo), achsenVon(akk, hoehe, QStringLiteral("a-9"), QStringLiteral("balgwechsel")), QStringLiteral("wind"));
        einzel(id, QStringLiteral("Klingt der Balgwechsel nach 1 s glaubhaft (kurzer Einbruch des Drucks)?"));
    }
    // die Bass-Seite (unter F3, 8′ + 4′, ZV6 a)
    for (const int hoehe : {41, 48})
        einzel(ablegen(akk, ton(hoehe, kHaltenMs), staerken[1], QStringLiteral("vorgabe")),
               QStringLiteral("Klingt es nach der Bass-Seite eines Akkordeons (Stradella, 8′ + 4′)?"));
    // Musette gegen einen Chor: dieselbe Datei, nur der erste Chor
    Instrument einChor = akk;
    einChor.name = QStringLiteral("accordion_ein_chor");
    {
        kern::Wert* w = einChor.schichten[0].kernWerte.finde("ranks");
        REQUIRE(w != nullptr);
        auto* liste = std::get_if<kern::Liste>(w);
        REQUIRE(liste != nullptr);
        REQUIRE(liste->eintraege.size() == 2);
        liste->eintraege.resize(1);
    }
    int paare = 0;
    for (const int hoehe : {57, 69})
    {
        const QString musette = ablegen(akk, ton(hoehe, kHaltenMs), staerken[1], QStringLiteral("musette"), QStringLiteral("musette"));
        const QString id = einChor.name + QStringLiteral("_") + QString::number(hoehe) + QStringLiteral("_a-9");
        TonAngabe t = ton(hoehe, kHaltenMs);
        t.instrument = einChor.name;
        t.anschlagStufe = -9;
        eintragen(id, rendereTon(einChor, t, kRate).daten, achsenVon(akk, hoehe, QStringLiteral("a-9"), QStringLiteral("ein_chor")),
                  QStringLiteral("ein_chor"));
        paar(musette, id, QStringLiteral("Welcher klingt mehr nach einem Akkordeon (Musette: zwei Choere, schwebend)?"));
        ++paare;
    }

    const qsizetype punkteGeplant = punkte.size();
    const hoerliste::Bereinigt bereinigt = hoerliste::bereinige(klaenge, messwerte, punkte, gerechnet, kRate);
    const hoerliste::Pegel pegel = hoerliste::gleicheAn(gerechnet, kRate, kZielHoechstens, kSpitzeHoechstens);
    hoerliste::ohneKlaenge(klaenge, messwerte, punkte, pegel.stille);
    const QStringList summen = hoerliste::schreibeKlaenge(dir, gerechnet, kRate);
    const QJsonObject liste{
        {QStringLiteral("schema"), 1},
        {QStringLiteral("titel"), QStringLiteral("LumiViz 34j: Klasse 10 durchschlagende Zunge — Akkordeon, Harmonium, Melodica, Mundharmonika")},
        {QStringLiteral("ziel"), QStringLiteral("Erstes Hoeren der vier Instrumente des Kerns freeReed (ungemessen, Z2 a): Lage und Staerke, der "
                                                "Wind (Crescendo, Vibrato, Balgwechsel), die Bass-Seite und Musette; die offenen Befunde aus "
                                                "ZV14 a (unterer Diskant mit dem Wind dunkler). Loest IC 10_zunge ab (LG1 a)")},
        {QStringLiteral("katalog"), QJsonValue::Null},
        {QStringLiteral("kontrolle"), QStringLiteral("moderat")},
        {QStringLiteral("pegel"), QJsonObject{{QStringLiteral("lufs"), pegel.ziel}, {QStringLiteral("toleranz_lu"), 1}}},
        {QStringLiteral("dauer_s"), QJsonValue::Null},
        {QStringLiteral("gruppe_nach"), QJsonArray{QStringLiteral("instrument")}},
        {QStringLiteral("fassungen"),
         QJsonObject{{QStringLiteral("vorgabe"), QStringLiteral("die mitgelieferte Datei (Kern freeReed, Vorgaben je Taste S190, gestimmt), ungemessen")},
                     {QStringLiteral("wind"), QStringLiteral("dieselbe Datei mit Wind ueber der Zeit (Expression, Vibrato, Balg; ZV3 a)")},
                     {QStringLiteral("musette"), QStringLiteral("das Akkordeon, wie geliefert: zwei Choere 8′ gegeneinander verstimmt (Musette)")},
                     {QStringLiteral("ein_chor"), QStringLiteral("das Akkordeon mit nur dem ersten Chor")}}},
        {QStringLiteral("klaenge"), klaenge},
        {QStringLiteral("messwerte"), messwerte},
        {QStringLiteral("punkte"), punkte}};
    QFile aus(dir.filePath(QStringLiteral("liste.json")));
    REQUIRE(aus.open(QIODevice::WriteOnly | QIODevice::Truncate));
    REQUIRE(aus.write(QJsonDocument(liste).toJson(QJsonDocument::Indented)) > 0);
    aus.close();
    QFile summenDatei(dir.filePath(QStringLiteral("klaenge.sha256")));
    REQUIRE(summenDatei.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QByteArray text = "# Abdruck jedes Klangs (SHA-256 der WAV-Datei). Geschrieben mit: LUMIVIZ_34J_WAV=<dieser Ordner> LumiViz.UnitTests "
                      "--test-case=\"Loop Zunge 34j-9 Hoerproben*\"\n";
    for (const QString& z : summen) text += z.toUtf8() + '\n';
    REQUIRE(summenDatei.write(text) > 0);
    summenDatei.close();
    hoerliste::legeInsRepo(dir);  // AU3-1 a
    MESSAGE(klaenge.size() << " Klaenge, " << punkte.size() << " Punkte (davon " << paare << " Paare) unter " << ordner.toStdString());
    MESSAGE("Pegel " << pegel.ziel << " LUFS; " << pegel.stille.size() << " still: " << pegel.stille.join(QStringLiteral(", ")).toStdString());
    MESSAGE("ohne Ton (LV6-3 a): " << bereinigt.ohneTon.join(QStringLiteral(", ")).toStdString()
                                   << "; gleich klingend (LV6-1 a): " << bereinigt.gleich.join(QStringLiteral(", ")).toStdString());
    CHECK(paare == 2);
    CHECK(punkteGeplant == 36 + (4 + 2) + 4 + 1 + 2 + 2);
    CHECK(bereinigt.ohneTon.isEmpty());
    CHECK(pegel.stille.isEmpty());
}

TEST_CASE("Loop Zunge 34j-9 Rechenzeit: Akkordeon (Musette und Bass-Seite) und die drei anderen, Anteil an der Echtzeit")
{
    // Meldet nur Zahlen (wie `*34h-6 Rechenzeit*`): je Ton ff, 2 s gehalten, das Beste aus drei Laeufen; dazu das Verhaeltnis
    // zum Fluegel-Kern (A0, Stufe 0, 4 s gehalten, 4,8 s Ton). Die Grenze von 4,6 % prueft Claude von Hand (RZ1-1 a: dreimal
    // allein nach der Taktprobe, Release-clang) — ausgesetzt, bis die Kuehlung getauscht ist (S181).
    const auto anteil = [](const kern::Tonkern& k, const kern::Tonauftrag& a) {
        std::vector<float> t;
        double best = 1e9;
        for (int i = 0; i < 3; ++i)
        {
            const auto t0 = std::chrono::steady_clock::now();
            k.rechne(a, t);
            best = std::min(best, std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count());
        }
        return best / (static_cast<double>(a.frames) / a.rate);
    };
    double schwerst = 0.0;
    std::string welcher;
    const auto miss = [&](const std::string& name, int note, const std::string& was) {
        const Instrument inst = instrumentVon(name);
        const std::shared_ptr<const kern::Tonkern> k = kern::kerne().erzeuge(zunge::kKernName, inst.schichten[0].kernWerte);
        REQUIRE(k != nullptr);
        kern::Tonauftrag a = kern::einzelnote(note, 440.0 * std::exp2((note - 69) / 12.0), 0, 2 * kRate, 0, kRate);
        a.frames = 2 * kRate + k->nachklang(a, 4 * kRate);
        const double x = anteil(*k, a);
        MESSAGE(name << " " << was << " an " << note << ": " << 100.0 * x << " % der Echtzeit");
        if (x > schwerst)
        {
            schwerst = x;
            welcher = name + " " + std::to_string(note);
        }
    };
    miss("accordion", 69, "Musette (zwei Choere)");
    miss("accordion", 41, "Bass-Seite (8′ + 4′)");
    miss("accordion", 29, "tiefster Bass");
    for (const std::string name : {"harmonium", "melodica", "harmonica"})
    {
        const zunge::ZungenSpiel& s = vorlage(name).spiel;
        miss(name, (s.noteMin + s.noteMax) / 2, "Mitte");
    }
    const std::shared_ptr<const kern::Tonkern> f = kern::kerne().erzeuge("hammerString", kern::vorgaben(kern::kerne().typ("hammerString")->tafel));
    REQUIRE(f != nullptr);
    const double fl = anteil(*f, kern::einzelnote(21, 27.5, 0, 4 * kRate, static_cast<kern::Frames>(4.8 * kRate), kRate));
    MESSAGE("schwerster Ton: " << welcher << " " << 100.0 * schwerst << " % der Echtzeit, " << schwerst / fl << " x Fluegel (A0 " << 100.0 * fl << " %)");
    CHECK(schwerst > 0.0);
    CHECK(fl > 0.0);
}
