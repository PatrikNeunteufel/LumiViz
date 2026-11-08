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
 ****************************************************************************************
 */

#include <doctest.h>

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

#include <cmath>
#include <filesystem>
#include <fstream>
#include <map>
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
    s.kernWerte = stab::werteAus(v.spiel);
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

TEST_CASE("Loop Stab 34g Hoerproben (LUMIVIZ_34G_WAV) — je Instrument drei Lagen und drei Staerken, IC gegen Synth")
{
    // Werkzeug (Teilplan 34g-6): legt die Hoerproben der neun Stabspiele und die blinden Vergleiche mit den
    // Nachbildungen (synth_marimba, synth_vibraphone, synth_kalimba) ab; ohne die Variable meldet er das und prueft
    // nichts. Gerechnet wie im Programm (rendereTon aus der Datei, mit Pegel), 2 s gehalten.
    const QString ordner = qEnvironmentVariable("LUMIVIZ_34G_WAV");
    if (ordner.isEmpty())
    {
        MESSAGE("LUMIVIZ_34G_WAV nicht gesetzt — keine Hoerproben");
        return;
    }
    REQUIRE(QDir().mkpath(ordner));
    constexpr int kRate = 48000;
    constexpr int kHaltenMs = 2000;
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
    const Staerke staerken[] = {{"pp", -30}, {"mf", -12}, {"ff", 0}};
    QJsonArray klaenge, paare;
    const auto ablegen = [&](const Instrument& inst, int hoehe, const Staerke& st, const QString& fassung) {
        TonAngabe t;
        t.instrument = inst.name;
        t.hoehe = hoehe;
        t.dauerMs = kHaltenMs;
        t.anschlagStufe = st.stufe;
        const Klang k = rendereTon(inst, t, kRate);
        const QString id = inst.name + QStringLiteral("_") + QString::number(hoehe) + QStringLiteral("_") + QString::fromLatin1(st.name);
        REQUIRE(schreibeWav(QDir(ordner).filePath(id + QStringLiteral(".wav")), k.daten, kRate));
        klaenge.append(QJsonObject{{QStringLiteral("id"), id},
                                   {QStringLiteral("datei"), id + QStringLiteral(".wav")},
                                   {QStringLiteral("taste"), hoehe},
                                   {QStringLiteral("name"), inst.titel},
                                   {QStringLiteral("staerke"), QString::fromLatin1(st.name)},
                                   {QStringLiteral("fassung"), fassung},
                                   {QStringLiteral("sekunden"), kHaltenMs / 1000.0}});
        return id;
    };
    int zahl = 0;
    for (const stab::Vorlage& v : stab::vorlagen())
    {
        if (!v.ausgeliefert) continue;
        const Instrument inst = lies(QString::fromStdString(v.name));
        const int lagen[] = {v.spiel.noteMin + 5, (v.spiel.noteMin + v.spiel.noteMax) / 2, v.spiel.noteMax - 5};
        for (const int hoehe : lagen)
            for (const Staerke& st : staerken)
            {
                (void)ablegen(inst, hoehe, st, QStringLiteral("IC"));
                ++zahl;
            }
    }
    // blind: das Modell aus IC gegen die Nachbildung gleichen Namens (IK2-8 a: nach dem Hoeren entscheidet Patrik)
    for (const char* name : {"marimba", "vibraphone", "kalimba"})
    {
        const Instrument ic = lies(QString::fromLatin1(name));
        const Instrument synth = lies(QStringLiteral("synth_") + QString::fromLatin1(name));
        const stab::StabSpiel s = stab::spielAus(ic.schichten[0].kernWerte);
        for (const int hoehe : {s.noteMin + 12, (s.noteMin + s.noteMax) / 2, s.noteMax - 12})
        {
            const QString a = ablegen(ic, hoehe, staerken[1], QStringLiteral("IC"));
            const QString b = ablegen(synth, hoehe, staerken[1], QStringLiteral("Synth"));
            paare.append(QJsonObject{{QStringLiteral("id"), a + QStringLiteral("_gegen_synth")},
                                     {QStringLiteral("a"), a},
                                     {QStringLiteral("b"), b},
                                     {QStringLiteral("kontrolle"), false}});
        }
    }
    const auto frage = [](const char* id, const char* text, const char* links, const char* rechts) {
        return QJsonObject{{QStringLiteral("id"), QString::fromUtf8(id)},
                           {QStringLiteral("text"), QString::fromUtf8(text)},
                           {QStringLiteral("links"), QString::fromUtf8(links)},
                           {QStringLiteral("rechts"), QString::fromUtf8(rechts)}};
    };
    QFile test(QDir(ordner).filePath(QStringLiteral("hoertest.json")));
    REQUIRE(test.open(QIODevice::WriteOnly | QIODevice::Truncate));
    test.write(QJsonDocument(QJsonObject{{QStringLiteral("titel"), QStringLiteral("34g: Stabspiele aus IC, gegen die Nachbildungen")},
                                         {QStringLiteral("fragen"),
                                          QJsonArray{frage("gesamt", "Klingt wie das echte Instrument", "gar nicht", "ganz"),
                                                     frage("anschlag", "Anschlag: Schlägel, Hammer, Zupfen", "weich", "hart"),
                                                     frage("farbe", "Klangfarbe", "dumpf", "hell, farbig"),
                                                     frage("laenge", "Länge: wie lange man den Ton hört", "kurz", "lang")}},
                                         {QStringLiteral("klaenge"), klaenge},
                                         {QStringLiteral("vergleiche"), paare}})
                   .toJson());
    MESSAGE(zahl << " Hoerproben der Stabspiele und " << paare.size() << " Vergleiche gegen Synth unter " << ordner.toStdString());
    CHECK(paare.size() == 9);
}
