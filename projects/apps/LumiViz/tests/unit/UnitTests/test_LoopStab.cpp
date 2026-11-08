/**
 ****************************************************************************************
 * @file   test_LoopStab.cpp
 * @brief  Loop, Stufe 34g: die mitgelieferten Instrumente des Kerns `bar` (Klasse 4, Platte und Stab)
 *         (docs/audio/Instrumente_IC_Konzept.md 34g-5, KS1 a–KS4 a, IK2-11 a)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Liest vom Arbeitsordner aus — die Test-Exe aus dem Projekt-Root starten.
 *
 * - **Schreiben** (Muster `grand_piano`, `test_LoopHammerSaite.cpp`): `LUMIVIZ_STAB_INSTRUMENTE_SCHREIBEN=1` schreibt
 *   die neun ausgelieferten Vorlagen (KS4 a) als `asset/loop/instruments/<name>.lvinst` mit dem Schreiber des
 *   Programms — ohne gestimmte Werte und Bezug. Danach je Datei `InstrumentTuneTool <datei> --all --reference`
 *   (stimmt den Pegel des Partners und die Fenster des Bogens, setzt den Bezug) und `python asset/loop/instrument_katalog.py`.
 * - **Waechter** (ohne zu stimmen): jede Datei lesbar, Kern `bar`, ihre Werte die der Vorlage (bis auf die gestimmten und
 *   den Bezug), die Tastatur nach dem Umfang; die Fenster des Vibraphons gleich denen aus IC (`werte/stab_ic.txt`) und
 *   der Ton aus den Werten der Datei Bit fuer Bit gleich dem IC-Abdruck. Ob die gestimmten Werte und der Bezug zu den
 *   Werten passen, pruefen `*Instrument Stimmen*` (Abdruck) und der Bezug dort.
 * - **Hoerproben** fuer HoerTest_Helper: `LUMIVIZ_34G_WAV=hoertest/listen/stab_34g` schreibt dort `liste.json`,
 *   `klaenge/` und `klaenge.sha256` (Konzept des Helpers §11, §12).
 ****************************************************************************************
 */

#include <doctest.h>

#include "AudioCore/Lautheit.hpp"
#include "InstrumentCore/Pruefsumme.hpp"
#include "InstrumentModels/Kerne.hpp"
#include "InstrumentModels/stab/StabKern.hpp"
#include "loop/Aufnahme.hpp"
#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"
#include "loop/Tastatur.hpp"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <span>
#include <sstream>
#include <string>
#include <vector>

using namespace lumi::loop;
namespace kern = lumi::kern;
namespace stab = lumi::kern::stab;

namespace
{

QString dateiVon(const stab::Vorlage& v)
{
    return QStringLiteral("asset/loop/instruments/") + QString::fromStdString(v.name) + QStringLiteral(".lvinst");
}

/// Ein Pedal rechts (Sustain) nur fuer Instrumente mit Daempfer; der Umfang des Instruments.
Tastatur tastaturVon(const stab::StabSpiel& s)
{
    Tastatur t;
    t.tiefste = s.noteMin;
    t.hoechste = s.noteMax;
    t.links = PedalArt::Keins;
    t.mitte = PedalArt::Keins;
    t.rechts = s.daempfer != stab::DaempferArt::Keiner ? PedalArt::Sustain : PedalArt::Keins;
    t.bassSustainBis = s.noteMin;
    richteTastatur(t);
    return t;
}

/// Das Instrument einer Vorlage, wie es das Programm schreibt (die gestimmten Werte so, wie die Vorlage sie traegt).
Instrument instrumentVon(const stab::Vorlage& v)
{
    Instrument i;
    i.titel = QString::fromStdString(v.titel);
    Schicht s;
    s.name = QString::fromStdString(v.titel);
    s.kernName = QString::fromUtf8(stab::kKernName.data(), static_cast<qsizetype>(stab::kKernName.size()));
    s.kernWerte = stab::werteVon(v);  // mit den Quellen der Vorlage (QM1 a)
    // gerichtet, wie das Programm schreibt (eine leere Kurve bekommt die Vorgabe der Tafel im Umfang des Instruments);
    // die gestimmten sind noch leer — das meldet das Richten, sie stimmt danach das Werkzeug
    std::vector<std::string> fehler;
    richteSchicht(s, &fehler);
    i.schichten[0] = s;
    i.tastatur = tastaturVon(v.spiel);
    return i;
}

/// Die Bezuege aus IC (`werte/stab_ic.txt`): Abdruecke der Toene und die Fenster des Bogens.
struct IcBezug
{
    std::map<std::string, std::string> abdruck;
    std::map<std::string, std::vector<kern::NotenWert>> fenster;
};

IcBezug icBezug()
{
    IcBezug r;
    std::ifstream f(std::filesystem::path(__FILE__).parent_path() / "werte" / "stab_ic.txt");
    std::string zeile;
    while (std::getline(f, zeile))
    {
        std::istringstream z(zeile);
        std::string art, name;
        z >> art >> name;
        if (art == "abdruck")
            z >> r.abdruck[name];
        else if (art == "fenster")
        {
            std::string paar;
            while (z >> paar)
            {
                const auto gleich = paar.find('=');
                r.fenster[name].push_back({std::stoi(paar.substr(0, gleich)), std::strtod(paar.c_str() + gleich + 1, nullptr)});
            }
        }
    }
    return r;
}

}  // namespace

TEST_CASE("Loop Stab 34g die mitgelieferten Stabspiele tragen ihre Vorlage")
{
    if (qEnvironmentVariableIntValue("LUMIVIZ_STAB_INSTRUMENTE_SCHREIBEN") == 1)
        for (const stab::Vorlage& v : stab::vorlagen())
        {
            if (!v.ausgeliefert) continue;
            QFile aus(dateiVon(v));
            REQUIRE_MESSAGE(aus.open(QIODevice::WriteOnly | QIODevice::Truncate), "asset/loop/instruments nicht gefunden — aus dem Projekt-Root starten");
            REQUIRE(aus.write(QJsonDocument(alsJson(instrumentVon(v))).toJson(QJsonDocument::Indented)) > 0);
            MESSAGE(v.name << ".lvinst geschrieben — danach InstrumentTuneTool <datei> --all --reference, dann instrument_katalog.py");
        }

    const IcBezug icb = icBezug();
    const std::map<std::string, std::string>& ic = icb.abdruck;
    REQUIRE_MESSAGE(ic.size() == 11, "werte/stab_ic.txt fehlt");
    REQUIRE(icb.fenster.size() == 6);
    const kern::Parametertafel& tafel = stab::kernTyp().tafel;
    int geprueft = 0;
    for (const stab::Vorlage& v : stab::vorlagen())
    {
        CAPTURE(v.name);
        QFile datei(dateiVon(v));
        if (!v.ausgeliefert)
        {
            CHECK_FALSE(datei.exists());  // KS4 a: die Steeldrum geht nicht mit
            continue;
        }
        REQUIRE_MESSAGE(datei.open(QIODevice::ReadOnly), "nicht gefunden — LUMIVIZ_STAB_INSTRUMENTE_SCHREIBEN=1, dann InstrumentTuneTool");
        Instrument ist;
        REQUIRE_MESSAGE(ausJson(QJsonDocument::fromJson(datei.readAll()).object(), ist),
                        "nicht lesbar (nicht gestimmt?) — InstrumentTuneTool <datei> --all --reference");
        REQUIRE(ist.schichten.size() == 1);
        const Schicht& s = ist.schichten[0];
        REQUIRE(istKernSchicht(s));
        CHECK(s.kernName == QStringLiteral("bar"));
        CHECK(s.koerper.isEmpty());
        REQUIRE(ist.tastatur.has_value());
        CHECK(*ist.tastatur == tastaturVon(v.spiel));
        CHECK(kern::ungemessen(tafel, s.kernWerte));
        ++geprueft;

        // die Werte der Vorlage (gerichtet), ohne die gestimmten und den Bezug
        kern::Werte soll = stab::werteAus(v.spiel);
        (void)kern::richteWerte(tafel, soll);  // meldet die leeren gestimmten — die stimmt das Werkzeug
        for (const kern::ParameterAngabe& a : tafel.werte)
        {
            if (a.gestimmt || a.name == tafel.bezugWert) continue;
            CAPTURE(a.name);
            const kern::Wert* w = s.kernWerte.finde(a.name);
            REQUIRE(w != nullptr);
            CHECK_MESSAGE(*w == *soll.finde(a.name), "veraltet — LUMIVIZ_STAB_INSTRUMENTE_SCHREIBEN=1, InstrumentTuneTool, instrument_katalog.py");
        }

        // die Fenster des Bogens, vom Werkzeug am Modell gestimmt, Bit fuer Bit wie IC `Stimmung.inc` (KS3 a)
        const stab::StabSpiel spiel = stab::spielAus(s.kernWerte);
        if (spiel.bogen)
            for (const auto& [name, punkte] : icb.fenster)
            {
                CAPTURE(name);
                CHECK(s.kernWerte.kurve(name).punkte == punkte);
            }

        // der Ton aus den Werten der Datei Bit fuer Bit wie IC (wie test_InstrumentModelsStab: Mitte, Stufe −12)
        stab::StabAnschlag n;
        n.note = (spiel.noteMin + spiel.noteMax) / 2;
        n.hz = 440.0 * std::exp2((n.note - 69) / 12.0);
        n.stufe = -12;
        if (spiel.motor) n.motor = {{0.0, 3.0}};
        std::vector<float> ton;
        stab::rechneStabTon(spiel, n, ton, stab::stabNachklang(spiel, n, 48000 * 3));
        CHECK(kern::abdruck(ton) == ic.at("stab_" + v.spiel.name + "_mitte_a-12"));
    }
    CHECK(geprueft == 9);
}

TEST_CASE("Loop Stab 34g Hoerproben (LUMIVIZ_34G_WAV) — Liste fuer HoerTest_Helper: je Instrument drei Lagen und drei Staerken, IC gegen Synth")
{
    // Werkzeug (Teilplan 34g-6): schreibt die Liste der Stabspiele im Format von HoerTest_Helper (dessen Konzept §11,
    // §12, Schema 1; Muster der IC-Liste `hoertest/listen/04_platte_stab`) in den Ordner der Variable — gedacht
    // `hoertest/listen/stab_34g`: `liste.json`, `klaenge/` (gitignored, HT8 a) und `klaenge.sha256`. Gerechnet wie im
    // Programm (rendereTon aus der Datei), 2 s gehalten, jeder Klang auf einen gemeinsamen Pegel (hoechstens −23 LUFS, ohne Spitze ueber
    // 0,89; BS.1770-4, `AudioCore/Lautheit`;
    // die Lautheit vorher steht als Messwert `lufs_vorher`). Ohne die Variable meldet er das und prueft nichts. Pruefen:
    // `Hoertest.exe --liste-pruefen <ordner>/liste.json`.
    const QString ordner = qEnvironmentVariable("LUMIVIZ_34G_WAV");
    if (ordner.isEmpty())
    {
        MESSAGE("LUMIVIZ_34G_WAV nicht gesetzt — keine Hoerproben");
        return;
    }
    const QDir dir(ordner);
    REQUIRE(QDir().mkpath(dir.filePath(QStringLiteral("klaenge"))));
    constexpr int kRate = 48000;
    constexpr int kHaltenMs = 2000;
    // der gemeinsame Pegel: hoechstens −23 LUFS (wie pv1), so tief, dass keine Spitze ueber 0,89 kommt (−1 dBFS; kurze
    // Anschlaege haben eine grosse Spitze je Lautheit — IC waehlte deshalb −29,4)
    constexpr double kZielHoechstens = -23.0;
    constexpr double kSpitzeHoechstens = 0.89;
    const auto lies = [](const QString& name) {
        QFile f(QStringLiteral("asset/loop/instruments/") + name + QStringLiteral(".lvinst"));
        Instrument i;
        REQUIRE_MESSAGE(f.open(QIODevice::ReadOnly), name.toStdString());
        REQUIRE(ausJson(QJsonDocument::fromJson(f.readAll()).object(), i));
        i.name = name;
        return i;
    };
    struct Staerke
    {
        const char* name;
        int stufe;
    };
    const Staerke staerken[] = {{"a-30", -30}, {"a-12", -12}, {"a0", 0}};
    QJsonArray klaenge, messwerte, punkte;
    QStringList summen;
    QStringList ids;
    QStringList stille;
    struct Gerechnet
    {
        QString datei;
        std::vector<float> daten;
        double lufs;  ///< vorher; −300 = still
        float spitze;
    };
    std::vector<Gerechnet> gerechnet;
    const auto ablegen = [&](const Instrument& inst, const QString& instrument, int hoehe, const Staerke& st, const QString& fassung) {
        const QString id = inst.name + QStringLiteral("_") + QString::number(hoehe) + QStringLiteral("_") + QString::fromLatin1(st.name);
        if (ids.contains(id)) return id;  // derselbe Klang aus der Reihe der Lagen und aus einem Vergleich
        ids << id;
        TonAngabe t;
        t.instrument = inst.name;
        t.hoehe = hoehe;
        t.dauerMs = kHaltenMs;
        t.anschlagStufe = st.stufe;
        Klang k = rendereTon(inst, t, kRate);
        const lumi::audio::Lautheit vorher = lumi::audio::messeLautheit(k.daten, kRate);
        // still (unter dem absoluten Gate, −70 LUFS): bleibt, wie er ist, mit −300 wie in der IC-Liste — ein Befund der
        // ungemessenen Vorgaben (in der Hoehe bei kleiner Staerke zu schwacher Anschlag; IC hat 10 von 108 still)
        if (vorher.stille) stille << id;
        const QString datei = QStringLiteral("klaenge/") + id + QStringLiteral(".wav");
        float spitze = 0.0f;
        for (const float x : k.daten) spitze = std::max(spitze, std::abs(x));
        gerechnet.push_back(Gerechnet{datei, std::move(k.daten), vorher.stille ? -300.0 : vorher.lufs, spitze});
        const QJsonObject achsen{{QStringLiteral("instrument"), instrument},
                                 {QStringLiteral("taste"), hoehe},
                                 {QStringLiteral("ton"), tonName(hoehe)},
                                 {QStringLiteral("staerke"), QString::fromLatin1(st.name)}};
        klaenge.append(QJsonObject{{QStringLiteral("id"), id},
                                   {QStringLiteral("datei"), datei},
                                   {QStringLiteral("fassung"), fassung},
                                   {QStringLiteral("achsen"), achsen}});
        messwerte.append(QJsonObject{{QStringLiteral("achsen"), achsen},
                                     {QStringLiteral("fassung"), fassung},
                                     {QStringLiteral("werte"), QJsonArray{QJsonObject{{QStringLiteral("name"), QStringLiteral("lufs_vorher")},
                                                                                      {QStringLiteral("wert"), vorher.stille ? -300.0 : std::round(vorher.lufs * 100.0) / 100.0},
                                                                                      {QStringLiteral("einheit"), QStringLiteral("LUFS")}}}}});
        return id;
    };
    const auto punktId = [&] { return QStringLiteral("p%1").arg(punkte.size() + 1, 3, 10, QLatin1Char('0')); };
    for (const stab::Vorlage& v : stab::vorlagen())
    {
        if (!v.ausgeliefert) continue;
        const Instrument inst = lies(QString::fromStdString(v.name));
        const int lagen[] = {v.spiel.noteMin + 5, (v.spiel.noteMin + v.spiel.noteMax) / 2, v.spiel.noteMax - 5};
        for (const int hoehe : lagen)
            for (const Staerke& st : staerken)
            {
                const QString id = ablegen(inst, inst.titel, hoehe, st, QStringLiteral("ic"));
                punkte.append(QJsonObject{{QStringLiteral("id"), punktId()},
                                          {QStringLiteral("art"), QStringLiteral("einzel")},
                                          {QStringLiteral("klang"), id},
                                          {QStringLiteral("frage"), QStringLiteral("Klingt es nach %1 (ungemessen)? Farbe ueber die Staerke glaubhaft?").arg(inst.titel)},
                                          {QStringLiteral("pole"), QJsonArray{QStringLiteral("nein"), QStringLiteral("ja")}}});
            }
    }
    // blind: das Modell aus IC gegen die Nachbildung gleichen Namens (IK2-8 a: nach dem Hoeren entscheidet Patrik)
    int paare = 0;
    for (const char* name : {"marimba", "vibraphone", "kalimba"})
    {
        const Instrument ic = lies(QString::fromLatin1(name));
        const Instrument synth = lies(QStringLiteral("synth_") + QString::fromLatin1(name));
        const stab::StabSpiel s = stab::spielAus(ic.schichten[0].kernWerte);
        for (const int hoehe : {s.noteMin + 12, (s.noteMin + s.noteMax) / 2, s.noteMax - 12})
        {
            const QString a = ablegen(ic, ic.titel, hoehe, staerken[1], QStringLiteral("ic"));
            const QString b = ablegen(synth, ic.titel, hoehe, staerken[1], QStringLiteral("synth"));
            punkte.append(QJsonObject{{QStringLiteral("id"), punktId()},
                                      {QStringLiteral("art"), QStringLiteral("paar")},
                                      {QStringLiteral("klaenge"), QJsonArray{a, b}},
                                      {QStringLiteral("vorzug"), QStringLiteral("Welcher klingt mehr nach %1?").arg(ic.titel)}});
            ++paare;
        }
    }
    // der gemeinsame Pegel, dann jeder Klang darauf — nachgeregelt, weil das absolute Gate (−70 LUFS) die integrierte
    // Lautheit sehr leiser Klaenge vom Massstab abhaengen laesst; stille bleiben, wie sie sind
    double ziel = kZielHoechstens;
    for (const Gerechnet& g : gerechnet)
        if (g.lufs > -299.0 && g.spitze > 0.0f) ziel = std::min(ziel, g.lufs + 20.0 * std::log10(kSpitzeHoechstens / g.spitze));
    ziel = std::floor(ziel * 10.0) / 10.0;
    for (Gerechnet& g : gerechnet)
    {
        if (g.lufs > -299.0)
        {
            double ist = g.lufs;
            for (int runde = 0; runde < 8 && std::abs(ist - ziel) > 0.05; ++runde)
            {
                const double faktor = std::pow(10.0, (ziel - ist) / 20.0);
                for (float& x : g.daten) x = static_cast<float>(x * faktor);
                ist = lumi::audio::messeLautheit(g.daten, kRate).lufs;
            }
            CHECK_MESSAGE(std::abs(ist - ziel) <= 0.05, g.datei.toStdString() << ": " << ist << " statt " << ziel << " LUFS");
        }
        float spitze = 0.0f;
        for (const float x : g.daten) spitze = std::max(spitze, std::abs(x));
        CHECK_MESSAGE(spitze < 1.0f, g.datei.toStdString() << ": Spitze " << spitze);
        REQUIRE(schreibeWav(dir.filePath(g.datei), g.daten, kRate));
        QFile geschrieben(dir.filePath(g.datei));
        REQUIRE(geschrieben.open(QIODevice::ReadOnly));
        const QByteArray bytes = geschrieben.readAll();
        summen << QString::fromStdString(kern::sha256(std::span<const std::uint8_t>(reinterpret_cast<const std::uint8_t*>(bytes.constData()),
                                                                                     static_cast<std::size_t>(bytes.size())))) +
                      QStringLiteral("  ") + g.datei;
    }
    const QJsonObject liste{
        {QStringLiteral("schema"), 1},
        {QStringLiteral("titel"), QStringLiteral("LumiViz 34g: die Stabspiele des Kerns bar, gegen die Nachbildungen")},
        {QStringLiteral("ziel"), QStringLiteral("Erstes Hoeren der neun Stabspiele aus IC (ungemessen, Z2 a) und welches den Namen behaelt, IC oder "
                                                "Synth (IK2-8 a)")},
        {QStringLiteral("katalog"), QJsonValue::Null},
        {QStringLiteral("kontrolle"), QStringLiteral("moderat")},
        {QStringLiteral("pegel"), QJsonObject{{QStringLiteral("lufs"), ziel}, {QStringLiteral("toleranz_lu"), 1}}},
        {QStringLiteral("dauer_s"), QJsonValue::Null},
        {QStringLiteral("gruppe_nach"), QJsonArray{QStringLiteral("instrument")}},
        {QStringLiteral("fassungen"),
         QJsonObject{{QStringLiteral("ic"), QStringLiteral("die Vorgabe des Kerns bar aus InstrumentsCalculation (LumiViz Stufe 34g), ungemessen")},
                     {QStringLiteral("synth"), QStringLiteral("die Nachbildung aus Schichten (synth_<name>)")}}},
        {QStringLiteral("klaenge"), klaenge},
        {QStringLiteral("messwerte"), messwerte},
        {QStringLiteral("punkte"), punkte}};
    QFile aus(dir.filePath(QStringLiteral("liste.json")));
    REQUIRE(aus.open(QIODevice::WriteOnly | QIODevice::Truncate));
    REQUIRE(aus.write(QJsonDocument(liste).toJson(QJsonDocument::Indented)) > 0);
    aus.close();
    QFile summenDatei(dir.filePath(QStringLiteral("klaenge.sha256")));
    REQUIRE(summenDatei.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QByteArray text = "# Abdruck jedes Klangs (SHA-256 der WAV-Datei). Geschrieben mit: LUMIVIZ_34G_WAV=<dieser Ordner> LumiViz.UnitTests "
                      "--test-case=\"Loop Stab 34g Hoerproben*\"\n";
    for (const QString& z : summen) text += z.toUtf8() + '\n';
    REQUIRE(summenDatei.write(text) > 0);
    MESSAGE(klaenge.size() << " Klaenge, " << punkte.size() << " Punkte (davon " << paare << " Paare IC gegen Synth) unter " << ordner.toStdString());
    MESSAGE("Pegel " << ziel << " LUFS; " << stille.size() << " still (unter −70 LUFS, ungeaendert): " << stille.join(QStringLiteral(", ")).toStdString());
    CHECK(paare == 9);
    CHECK(klaenge.size() == 81 + 9 + 6);  // die mittleren Noten der Paare sind schon in der Reihe der Lagen
}
