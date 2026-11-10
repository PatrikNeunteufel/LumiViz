/**
 ****************************************************************************************
 * @file   test_LoopTrommel.cpp
 * @brief  Loop, Stufe 34h: die mitgelieferten Instrumente des Kerns `membrane` (Klasse 5, Membran)
 *         (docs/audio/Instrumente_IC_Konzept.md 34h-4, TM1 a, TM4 a)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Liest vom Arbeitsordner aus — die Test-Exe aus dem Projekt-Root starten.
 *
 * - **Schreiben** (Muster `test_LoopStab.cpp`): `LUMIVIZ_TROMMEL_INSTRUMENTE_SCHREIBEN=1` schreibt die sieben Vorlagen
 *   (TM4 a) als `asset/loop/instruments/<name>.lvinst` mit dem Schreiber des Programms — ohne Bezug. Danach je Datei
 *   `InstrumentTuneTool <datei> --reference` (setzt den Bezug, zwei Runden) und `python asset/loop/instrument_katalog.py`.
 * - **Waechter**: jede Datei lesbar, Kern `membrane`, ihre Werte die der Vorlage (bis auf den Bezug), die Tastatur nach dem
 *   Umfang, der Bezug Bit fuer Bit wie IC `normiere` und der Ton aus den Werten der Datei Bit fuer Bit wie der IC-Abdruck
 *   (`werte/trommel_ic.txt`: ff an der Mitte des Umfangs, ueber den Kern bei Stufe 0).
 * - **Spielweise** (34h-5, TM2 a): feste Werte der Spuren des Kerns an Drum-Spur und Kit-Rolle — im Namen des Tons
 *   (`:k…`), in `.lvloop` Fassung 18 und `.lvkit` Fassung 2 (`coreTracks`), im Kitwechsel und im Klang.
 ****************************************************************************************
 */

#include <doctest.h>

#include "AudioCore/Lautheit.hpp"
#include "InstrumentCore/Pruefsumme.hpp"
#include "InstrumentModels/Kerne.hpp"
#include "InstrumentModels/trommel/TrommelKern.hpp"
#include "loop/Aufnahme.hpp"
#include "loop/DrumKit.hpp"
#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"
#include "HoerlisteAblage.hpp"
#include "HoerlistePegel.hpp"
#include "loop/Tastatur.hpp"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace lumi::loop;
namespace kern = lumi::kern;
namespace trommel = lumi::kern::trommel;

namespace
{

QString dateiVon(const trommel::Vorlage& v)
{
    return QStringLiteral("asset/loop/instruments/") + QString::fromStdString(v.name) + QStringLiteral(".lvinst");
}

/// Ohne Pedale (eine Trommel hat keinen Daempfer; den Umfang traegt seit KL4 a die Tafel).
Tastatur tastaturVon(const trommel::TrommelSpiel& s)
{
    Tastatur t;
    t.links = PedalArt::Keins;
    t.mitte = PedalArt::Keins;
    t.rechts = PedalArt::Keins;
    t.bassSustainBis = s.note;
    richteTastatur(t);
    return t;
}

/// Das Instrument einer Vorlage, wie es das Programm schreibt.
Instrument instrumentVon(const trommel::Vorlage& v)
{
    Instrument i;
    i.titel = QString::fromStdString(v.titel);
    Schicht s;
    s.name = QString::fromStdString(v.titel);
    s.kernName = QString::fromUtf8(trommel::kKernName.data(), static_cast<qsizetype>(trommel::kKernName.size()));
    s.kernWerte = trommel::werteVon(v);  // mit den Quellen der Vorlage (QM1 a)
    std::vector<std::string> fehler;
    richteSchicht(s, &fehler);
    i.schichten[0] = s;
    i.tastatur = tastaturVon(v.spiel);
    return i;
}

std::string hexVon(double x)
{
    std::uint64_t b = 0;
    std::memcpy(&b, &x, sizeof b);
    char t[32];
    std::snprintf(t, sizeof t, "%016llx", static_cast<unsigned long long>(b));
    return t;
}

struct IcBezug
{
    std::map<std::string, std::string> abdruck;
    std::map<std::string, std::string> bezug;
};

IcBezug icBezug()
{
    IcBezug r;
    std::ifstream f(std::filesystem::path(__FILE__).parent_path() / "werte" / "trommel_ic.txt");
    std::string zeile;
    while (std::getline(f, zeile))
    {
        std::istringstream z(zeile);
        std::string art, name, wert;
        z >> art >> name >> wert;
        if (art == "abdruck") r.abdruck[name] = wert;
        if (art == "bezug") r.bezug[name] = wert;
    }
    return r;
}

}  // namespace

TEST_CASE("Loop Trommel 34h die mitgelieferten Trommeln tragen ihre Vorlage")
{
    if (qEnvironmentVariableIntValue("LUMIVIZ_TROMMEL_INSTRUMENTE_SCHREIBEN") == 1)
        for (const trommel::Vorlage& v : trommel::vorlagen())
        {
            QFile aus(dateiVon(v));
            REQUIRE_MESSAGE(aus.open(QIODevice::WriteOnly | QIODevice::Truncate), "asset/loop/instruments nicht gefunden — aus dem Projekt-Root starten");
            REQUIRE(aus.write(QJsonDocument(alsJson(instrumentVon(v))).toJson(QJsonDocument::Indented)) > 0);
            MESSAGE(v.name << ".lvinst geschrieben — danach InstrumentTuneTool <datei> --reference, dann instrument_katalog.py");
        }

    const IcBezug ic = icBezug();
    REQUIRE_MESSAGE(ic.bezug.size() == 7, "werte/trommel_ic.txt fehlt");
    const kern::Parametertafel& tafel = trommel::kernTyp().tafel;
    int geprueft = 0;
    for (const trommel::Vorlage& v : trommel::vorlagen())
    {
        CAPTURE(v.name);
        QFile datei(dateiVon(v));
        REQUIRE_MESSAGE(datei.open(QIODevice::ReadOnly), "nicht gefunden — LUMIVIZ_TROMMEL_INSTRUMENTE_SCHREIBEN=1, dann InstrumentTuneTool");
        Instrument ist;
        REQUIRE_MESSAGE(ausJson(QJsonDocument::fromJson(datei.readAll()).object(), ist), "nicht lesbar — InstrumentTuneTool <datei> --reference");
        REQUIRE(ist.schichten.size() == 1);
        const Schicht& s = ist.schichten[0];
        REQUIRE(istKernSchicht(s));
        CHECK(s.kernName == QStringLiteral("membrane"));
        CHECK(s.koerper.isEmpty());
        REQUIRE(ist.tastatur.has_value());
        CHECK(*ist.tastatur == tastaturVon(v.spiel));
        CHECK(kern::ungemessen(tafel, s.kernWerte));
        ++geprueft;

        // die Werte der Vorlage (gerichtet), ohne den Bezug
        kern::Werte soll = trommel::werteAus(v.spiel);
        CHECK(kern::richteWerte(tafel, soll).empty());
        for (const kern::ParameterAngabe& a : tafel.werte)
        {
            if (a.name == tafel.bezugWert) continue;
            CAPTURE(a.name);
            const kern::Wert* w = s.kernWerte.finde(a.name);
            REQUIRE(w != nullptr);
            CHECK_MESSAGE(*w == *soll.finde(a.name), "veraltet — LUMIVIZ_TROMMEL_INSTRUMENTE_SCHREIBEN=1, InstrumentTuneTool, instrument_katalog.py");
        }

        // der Bezug Bit fuer Bit wie IC `normiere`, der Ton ff an der Mitte Bit fuer Bit wie der IC-Abdruck
        CHECK(hexVon(s.kernWerte.zahl(tafel.bezugWert)) == ic.bezug.at(v.spiel.name));
        const std::shared_ptr<const kern::Tonkern> k = kern::kerne().erzeuge(trommel::kKernName, s.kernWerte);
        REQUIRE(k != nullptr);
        const int note = (v.spiel.note + v.spiel.noteMax) / 2;
        std::vector<float> ton;
        k->rechne(kern::einzelnote(note, 440.0 * std::exp2((note - 69) / 12.0), 0, 12000, 24000, 48000.0), ton);
        CHECK(kern::abdruck(ton) == ic.abdruck.at("trommel_" + v.spiel.name + "_ff"));
    }
    CHECK(geprueft == 7);
}

namespace
{

Instrument liesInstrument(const QString& name)
{
    QFile f(QStringLiteral("asset/loop/instruments/") + name + QStringLiteral(".lvinst"));
    Instrument i;
    REQUIRE_MESSAGE(f.open(QIODevice::ReadOnly), name.toStdString());
    REQUIRE(ausJson(QJsonDocument::fromJson(f.readAll()).object(), i));
    i.name = name;
    return i;
}

std::vector<KernSpurWert> randschlag() { return {KernSpurWert{QStringLiteral("stroke"), 4.0}}; }

}  // namespace

TEST_CASE("Loop Trommel 34h-5 Spielweise: der Name des Tons traegt die festen Werte der Spuren des Kerns (TM2 a)")
{
    TonAngabe t;
    t.instrument = QStringLiteral("snare_drum");
    t.hoehe = 38;
    t.dauerMs = 250;
    const QString ohne = tonKlang(t);
    CHECK(ohne == QStringLiteral("inst:snare_drum:38:250"));  // ohne Werte derselbe Name wie bisher
    t.kernSpuren = {KernSpurWert{QStringLiteral("stroke"), 4.0}, KernSpurWert{QStringLiteral("roll"), 1.0}};
    t.anschlagStufe = -6;
    t.endeMs = 900;
    const QString name = tonKlang(t);
    CHECK(name == QStringLiteral("inst:snare_drum:38:250:kroll=1,stroke=4:a-6:e900"));  // geordnet, vor Stufe und Ende
    TonAngabe zurueck;
    REQUIRE(zerlegeTon(name, zurueck));
    CHECK(zurueck.kernSpuren == richteKernSpuren(t.kernSpuren));
    CHECK(zurueck.anschlagStufe == -6);
    CHECK(zurueck.endeMs == 900);
    CHECK(tonKlang(zurueck) == name);
    // richten: geordnet, der letzte gilt, nur Buchstaben, nur endliche Zahlen
    const std::vector<KernSpurWert> gerichtet = richteKernSpuren({{QStringLiteral("stroke"), 1.0},
                                                                  {QStringLiteral("roll"), 1.0},
                                                                  {QStringLiteral("stroke"), 3.0},
                                                                  {QStringLiteral("bad name"), 1.0},
                                                                  {QStringLiteral("x1"), 1.0},
                                                                  {QStringLiteral("nan"), std::nan("")}});
    CHECK(gerichtet == std::vector<KernSpurWert>{{QStringLiteral("roll"), 1.0}, {QStringLiteral("stroke"), 3.0}});
    // eine Drum-Spur spielt mit ihnen
    Spur s{QStringLiteral("Rim"), QString(), 0.8, 0.0, false};
    s.instrument = QStringLiteral("snare_drum");
    s.hoehe = 38;
    CHECK(s.spielt() == QStringLiteral("inst:snare_drum:38:250"));
    s.kernSpuren = randschlag();
    CHECK(s.spielt() == QStringLiteral("inst:snare_drum:38:250:kstroke=4"));
}

TEST_CASE("Loop Trommel 34h-5 Spielweise in der Datei: .lvloop Fassung 18 und .lvkit Fassung 2, immer geschrieben")
{
    Loop loop;
    loop.laut = 1.0;
    loop.schlaegeJeTakt = 1;
    Muster m;
    m.name = QStringLiteral("D");
    Spur snare{QStringLiteral("Snare"), QStringLiteral("kit:snare"), 1.0, 0.0, false};
    snare.instrument = QStringLiteral("snare_drum");
    snare.hoehe = 38;
    Spur rim = snare;
    rim.name = QStringLiteral("Rim");
    rim.kernSpuren = randschlag();
    m.spuren = {{QStringLiteral("Kick"), QStringLiteral("kit:kick"), 1.0, 0.0, false}, snare, rim};
    m.zeilen = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}};
    loop.muster = {m};
    loop.kanaele = {{QStringLiteral("K"), true, {{QStringLiteral("D"), 0, 1}}}};
    loop.richte();
    const QJsonObject json = alsJson(loop, QString());
    CHECK(json.value(QStringLiteral("version")).toInt() == 18);
    const QJsonArray spuren = json.value(QStringLiteral("patterns")).toArray().at(0).toObject().value(QStringLiteral("tracks")).toArray();
    REQUIRE(spuren.size() == 3);
    CHECK_FALSE(spuren.at(0).toObject().contains(QStringLiteral("coreTracks")));  // ohne Instrument nicht
    REQUIRE(spuren.at(1).toObject().value(QStringLiteral("coreTracks")).isObject());  // mit Instrument immer, auch leer
    CHECK(spuren.at(1).toObject().value(QStringLiteral("coreTracks")).toObject().isEmpty());
    CHECK(spuren.at(2).toObject().value(QStringLiteral("coreTracks")).toObject() == QJsonObject{{QStringLiteral("stroke"), 4.0}});
    Loop gelesen;
    REQUIRE(ausJson(json, gelesen, QString()));
    CHECK(gelesen == loop);
    CHECK(gelesen.klangNamen().contains(QStringLiteral("inst:snare_drum:38:250:kstroke=4")));

    // das Kit
    DrumKit kit;
    kit.titel = QStringLiteral("Probe");
    KitKlang k;
    k.rolle = QStringLiteral("rim");
    k.instrument = QStringLiteral("snare_drum");
    k.hoehe = 38;
    k.kernSpuren = randschlag();
    KitKlang kick;
    kick.rolle = QStringLiteral("kick");
    kick.klang = QStringLiteral("kit:kick");
    kit.klaenge = {kick, k};
    const QJsonObject kj = alsJson(kit, QString());
    CHECK(kj.value(QStringLiteral("version")).toInt() == 2);
    const QJsonArray rollen = kj.value(QStringLiteral("roles")).toArray();
    CHECK_FALSE(rollen.at(0).toObject().contains(QStringLiteral("coreTracks")));
    CHECK(rollen.at(1).toObject().value(QStringLiteral("coreTracks")).toObject() == QJsonObject{{QStringLiteral("stroke"), 4.0}});
    DrumKit zurueck;
    REQUIRE(ausJson(kj, zurueck, QString()));
    CHECK(zurueck.klaenge == kit.klaenge);
    // eine Datei der Fassung 1 (ohne coreTracks) bleibt lesbar: ohne Spielweise
    QJsonObject alt = kj;
    alt[QStringLiteral("version")] = 1;
    QJsonArray ohne = alt.value(QStringLiteral("roles")).toArray();
    QJsonObject r = ohne.at(1).toObject();
    r.remove(QStringLiteral("coreTracks"));
    ohne[1] = r;
    alt[QStringLiteral("roles")] = ohne;
    REQUIRE(ausJson(alt, zurueck, QString()));
    CHECK(zurueck.klaenge[1].kernSpuren.empty());
}

TEST_CASE("Loop Trommel 34h-5 Spielweise im Kitwechsel: sie wandert mit, weicht ab und geht zurueck ins Kit")
{
    DrumKit kit;
    kit.name = QStringLiteral("probe");
    KitKlang k;
    k.rolle = QStringLiteral("rim");
    k.instrument = QStringLiteral("snare_drum");
    k.hoehe = 38;
    k.kernSpuren = randschlag();
    kit.klaenge = {k};
    Muster m;
    m.name = QStringLiteral("D");
    Spur rim{QStringLiteral("Rim"), QStringLiteral("kit:snare"), 1.0, 0.0, false};
    rim.rolle = QStringLiteral("rim");
    m.spuren = {rim};
    m.zeilen = {{1, 0, 0, 0}};
    (void)wendeKitAn(m, kit);
    REQUIRE(m.spuren.size() == 1);
    CHECK(m.spuren[0].instrument == QStringLiteral("snare_drum"));
    CHECK(m.spuren[0].kernSpuren == randschlag());
    CHECK_FALSE(weichtAb(m, kit));
    m.spuren[0].kernSpuren.clear();
    CHECK(weichtAb(m, kit));
    m.spuren[0].kernSpuren = randschlag();
    const DrumKit zurueck = kitAusMuster(m, QStringLiteral("Zurueck"));
    REQUIRE(zurueck.klaenge.size() == 1);
    CHECK(zurueck.klaenge[0].kernSpuren == randschlag());
    CHECK(spurAusKitKlang(k).kernSpuren == randschlag());
}

TEST_CASE("Loop Trommel 34h-5 Spielweise im Klang: der Kern spielt den Randschlag, der Wirbel klingt laenger")
{
    const Instrument snare = liesInstrument(QStringLiteral("snare_drum"));
    TonAngabe t;
    t.instrument = snare.name;
    t.hoehe = 38;
    t.dauerMs = 800;
    const Klang normal = rendereTon(snare, t, 48000);
    TonAngabe rim = t;
    rim.kernSpuren = {KernSpurWert{QStringLiteral("stroke"), 4.0}};
    const Klang reifen = rendereTon(snare, rim, 48000);
    CHECK(reifen.daten != normal.daten);
    CHECK(rendereTon(snare, rim, 48000).daten == reifen.daten);  // fest
    // ein unbekannter Name aendert am Ton nichts (der Kern liest ihn nicht)
    TonAngabe fremd = t;
    fremd.kernSpuren = {KernSpurWert{QStringLiteral("motor"), 3.0}};
    CHECK(rendereTon(snare, fremd, 48000).daten == normal.daten);
    // der Wirbel ueber die Laenge der Note (0,8 s): laenger als der eine Schlag und spaet noch laut
    TonAngabe wirbel = t;
    wirbel.kernSpuren = {KernSpurWert{QStringLiteral("roll"), 1.0}};
    const Klang w = rendereTon(snare, wirbel, 48000);
    CHECK(w.daten.size() > normal.daten.size());
    double spaet = 0.0, spaetNormal = 0.0;
    for (std::size_t i = 2 * 28800; i < 2 * 33600 && i < w.daten.size(); ++i) spaet += static_cast<double>(w.daten[i]) * w.daten[i];
    for (std::size_t i = 2 * 28800; i < 2 * 33600 && i < normal.daten.size(); ++i)
        spaetNormal += static_cast<double>(normal.daten[i]) * normal.daten[i];
    CHECK(spaet > 100.0 * spaetNormal);
}

TEST_CASE("Loop Trommel 34h-6 Hoerproben (LUMIVIZ_34H_WAV) — Liste fuer HoerTest_Helper: je Trommel drei Staerken, Spielweisen, IC gegen Synth")
{
    // Werkzeug (Teilplan 34h-6): schreibt die Liste der Trommeln im Format von HoerTest_Helper (dessen Konzept §11, §12,
    // Schema 1; Muster `hoertest/listen/stab_34g`) in den Ordner der Variable — der Hoerort `…\cmake\samples\Hoerproben\LumiViz\membran_34h` (AU3, S182; Liste
    // und Abdruecke danach auch versioniert unter `hoertest/listen/membran_34h`, AU3-1 a):
    // `liste.json`, `klaenge/` (gitignored) und `klaenge.sha256`. Gerechnet wie im Programm (rendereTon aus der Datei),
    // 1,5 s gehalten; je Klang auf einen gemeinsamen Pegel (hoechstens −23 LUFS, ohne Spitze ueber 0,89). Die Staerken
    // a-18, a-9, a0 sind die Geschwindigkeiten pp, mf, ff von IC (TM1 a: tempoFf · 10^(Stufe/20); tempoPp = tempoFf/8).
    const QString ordner = qEnvironmentVariable("LUMIVIZ_34H_WAV");
    if (ordner.isEmpty())
    {
        MESSAGE("LUMIVIZ_34H_WAV nicht gesetzt — keine Hoerproben");
        return;
    }
    const QDir dir(ordner);
    REQUIRE(QDir().mkpath(dir.filePath(QStringLiteral("klaenge"))));
    constexpr int kRate = 48000;
    constexpr int kHaltenMs = 1500;
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
    const auto ablegen = [&](const Instrument& inst, const QString& instrument, int hoehe, const Staerke& st, const QString& spielweise,
                             const std::vector<KernSpurWert>& werte, const QString& fassung) {
        const QString id = inst.name + QStringLiteral("_") + QString::number(hoehe) + QStringLiteral("_") + QString::fromLatin1(st.name) +
                           (spielweise.isEmpty() ? QString() : QStringLiteral("_") + spielweise);
        if (ids.contains(id)) return id;
        ids << id;
        TonAngabe t;
        t.instrument = inst.name;
        t.hoehe = hoehe;
        t.dauerMs = kHaltenMs;
        t.anschlagStufe = st.stufe;
        t.kernSpuren = werte;
        Klang k = rendereTon(inst, t, kRate);
        const lumi::audio::Lautheit vorher = lumi::audio::messeLautheit(k.daten, kRate);
        const QString datei = QStringLiteral("klaenge/") + id + QStringLiteral(".wav");
        gerechnet.push_back(hoerliste::Klang{id, datei, std::move(k.daten)});
        QJsonObject achsen{{QStringLiteral("instrument"), instrument},
                           {QStringLiteral("taste"), hoehe},
                           {QStringLiteral("ton"), tonName(hoehe)},
                           {QStringLiteral("staerke"), QString::fromLatin1(st.name)}};
        if (!spielweise.isEmpty()) achsen[QStringLiteral("spielweise")] = spielweise;
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
    const auto einzeln = [&](const QString& id, const QString& frage) {
        punkte.append(QJsonObject{{QStringLiteral("id"), punktId()},
                                  {QStringLiteral("art"), QStringLiteral("einzel")},
                                  {QStringLiteral("klang"), id},
                                  {QStringLiteral("frage"), frage},
                                  {QStringLiteral("pole"), QJsonArray{QStringLiteral("nein"), QStringLiteral("ja")}}});
    };
    for (const trommel::Vorlage& v : trommel::vorlagen())
    {
        const Instrument inst = liesInstrument(QString::fromStdString(v.name));
        std::vector<int> lagen{(v.spiel.note + v.spiel.noteMax) / 2};
        if (v.spiel.stimmbar) lagen = {v.spiel.note, (v.spiel.note + v.spiel.noteMax) / 2, v.spiel.noteMax};
        for (const int hoehe : lagen)
            for (const Staerke& st : staerken)
                einzeln(ablegen(inst, inst.titel, hoehe, st, QString(), {}, QStringLiteral("ic")),
                        QStringLiteral("Klingt es nach %1 (ungemessen)? Farbe ueber die Staerke glaubhaft?").arg(inst.titel));
    }
    // die Spielweisen der Snare und des Toms (TM2 a) bei mf
    const Instrument snare = liesInstrument(QStringLiteral("snare_drum"));
    const struct
    {
        const char* name;
        const char* spur;
        double wert;
        const char* frage;
    } weisen[] = {{"center", "stroke", 1, "in der Mitte geschlagen"},  {"edge", "stroke", 2, "nahe am Rand"},
                  {"rimshot", "stroke", 3, "als Randschlag (Rimshot)"}, {"crossstick", "stroke", 4, "als Cross-Stick"},
                  {"muted", "stroke", 5, "gedaempft (Hand auf dem Fell)"}, {"roll", "roll", 1, "als Wirbel"},
                  {"snaresoff", "snaresOff", 1, "ohne Teppich"}};
    for (const auto& w : weisen)
        einzeln(ablegen(snare, snare.titel, 38, staerken[1], QString::fromLatin1(w.name), {KernSpurWert{QString::fromLatin1(w.spur), w.wert}},
                        QStringLiteral("ic")),
                QStringLiteral("Klingt die Snare %1 glaubhaft?").arg(QString::fromLatin1(w.frage)));
    const Instrument tom = liesInstrument(QStringLiteral("low_tom"));
    einzeln(ablegen(tom, tom.titel, 45, staerken[1], QStringLiteral("muted"), {KernSpurWert{QStringLiteral("stroke"), 5}}, QStringLiteral("ic")),
            QStringLiteral("Klingt der Tom gedaempft glaubhaft?"));
    // blind: die Conga aus IC gegen die Nachbildung (IK2-8 a: nach dem Hoeren entscheidet Patrik)
    const Instrument ic = liesInstrument(QStringLiteral("conga"));
    const Instrument synth = liesInstrument(QStringLiteral("synth_conga"));
    int paare = 0;
    for (const int hoehe : {60, 63, 67})
    {
        const QString a = ablegen(ic, ic.titel, 63, staerken[1], QString(), {}, QStringLiteral("ic"));
        const QString b = ablegen(synth, ic.titel, hoehe, staerken[1], QString(), {}, QStringLiteral("synth"));
        punkte.append(QJsonObject{{QStringLiteral("id"), punktId()},
                                  {QStringLiteral("art"), QStringLiteral("paar")},
                                  {QStringLiteral("klaenge"), QJsonArray{a, b}},
                                  {QStringLiteral("vorzug"), QStringLiteral("Welcher klingt mehr nach einer Conga?")}});
        ++paare;
    }
    // der gemeinsame Pegel, jeder Klang nachgeregelt (in Runden, S179: die Bassdrum pp), stille nicht in der Liste
    // (PN1 a, PN2 a, KZ1 a)
    const hoerliste::Pegel pegel = hoerliste::gleicheAn(gerechnet, kRate, kZielHoechstens, kSpitzeHoechstens);
    const double ziel = pegel.ziel;
    const QStringList& stille = pegel.stille;
    hoerliste::ohneStille(klaenge, messwerte, punkte, stille);
    const QStringList summen = hoerliste::schreibeKlaenge(dir, gerechnet, kRate);
    const QJsonObject liste{
        {QStringLiteral("schema"), 1},
        {QStringLiteral("titel"), QStringLiteral("LumiViz 34h: die Trommeln des Kerns membrane, Spielweisen, gegen die Nachbildung der Conga")},
        {QStringLiteral("ziel"), QStringLiteral("Erstes Hoeren der sieben Trommeln aus IC (ungemessen, Z2 a), ihrer Spielweisen (TM2 a) und welche Conga "
                                                "den Namen behaelt, IC oder Synth (IK2-8 a)")},
        {QStringLiteral("katalog"), QJsonValue::Null},
        {QStringLiteral("kontrolle"), QStringLiteral("moderat")},
        {QStringLiteral("pegel"), QJsonObject{{QStringLiteral("lufs"), ziel}, {QStringLiteral("toleranz_lu"), 1}}},
        {QStringLiteral("dauer_s"), QJsonValue::Null},
        {QStringLiteral("gruppe_nach"), QJsonArray{QStringLiteral("instrument")}},
        {QStringLiteral("fassungen"),
         QJsonObject{{QStringLiteral("ic"), QStringLiteral("die Vorgabe des Kerns membrane aus InstrumentsCalculation (LumiViz Stufe 34h), ungemessen")},
                     {QStringLiteral("synth"), QStringLiteral("die Nachbildung aus Schichten (synth_conga)")}}},
        {QStringLiteral("klaenge"), klaenge},
        {QStringLiteral("messwerte"), messwerte},
        {QStringLiteral("punkte"), punkte}};
    QFile aus(dir.filePath(QStringLiteral("liste.json")));
    REQUIRE(aus.open(QIODevice::WriteOnly | QIODevice::Truncate));
    REQUIRE(aus.write(QJsonDocument(liste).toJson(QJsonDocument::Indented)) > 0);
    aus.close();
    QFile summenDatei(dir.filePath(QStringLiteral("klaenge.sha256")));
    REQUIRE(summenDatei.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QByteArray text = "# Abdruck jedes Klangs (SHA-256 der WAV-Datei). Geschrieben mit: LUMIVIZ_34H_WAV=<dieser Ordner> LumiViz.UnitTests "
                      "--test-case=\"Loop Trommel 34h-6 Hoerproben*\"\n";
    for (const QString& z : summen) text += z.toUtf8() + '\n';
    REQUIRE(summenDatei.write(text) > 0);
    summenDatei.close();
    hoerliste::legeInsRepo(dir);  // AU3-1 a: Liste und Abdruecke auch versioniert unter hoertest/listen/<liste>
    MESSAGE(klaenge.size() << " Klaenge, " << punkte.size() << " Punkte (davon " << paare << " Paare IC gegen Synth) unter " << ordner.toStdString());
    MESSAGE("Pegel " << ziel << " LUFS; " << stille.size() << " still (unter −70 LUFS, nicht in der Liste, PN2 a): "
                     << stille.join(QStringLiteral(", ")).toStdString());
    CHECK(paare == 3);
    CHECK(klaenge.size() + stille.size() == 6 * 3 + 3 * 3 + 7 + 1 + 3);  // sechs feste Trommeln, die Pauke in drei Lagen, Spielweisen, Synth
}

TEST_CASE("Loop Trommel 34h-6 Rechenzeit: je Trommel der Anteil an der Echtzeit und das Verhaeltnis zum Fluegel")
{
    // Meldet nur Zahlen (wie `*34g*` in test_InstrumentModelsStab): je Trommel ff an der Mitte, 2 s, das Beste aus drei
    // Laeufen; die Kennzahl ist das Verhaeltnis zum Fluegel-Kern (A0, Stufe 0, 4 s gehalten, 4,8 s Ton, auf 2 s
    // gerechnet). Die Grenze von 4,6 % prueft Claude von Hand (RZ1-1 a: dreimal allein nach der Taktprobe, Release-clang).
    const auto anteil = [](const kern::Tonkern& k, const kern::Tonauftrag& a) {
        std::vector<float> ton;
        double best = 1e9;
        for (int i = 0; i < 3; ++i)
        {
            const auto t0 = std::chrono::steady_clock::now();
            k.rechne(a, ton);
            best = std::min(best, std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count());
        }
        return best / (static_cast<double>(a.frames) / a.rate);
    };
    double schwerst = 0.0;
    std::string welcher;
    for (const trommel::Vorlage& v : trommel::vorlagen())
    {
        const Instrument inst = liesInstrument(QString::fromStdString(v.name));
        const std::shared_ptr<const kern::Tonkern> k = kern::kerne().erzeuge(trommel::kKernName, inst.schichten[0].kernWerte);
        REQUIRE(k != nullptr);
        const int note = (v.spiel.note + v.spiel.noteMax) / 2;
        const double a = anteil(*k, kern::einzelnote(note, 440.0 * std::exp2((note - 69) / 12.0), 0, 24000, 96000, 48000.0));
        MESSAGE(v.name << " ff an " << note << ": " << 100.0 * a << " % der Echtzeit");
        if (a > schwerst)
        {
            schwerst = a;
            welcher = v.name;
        }
    }
    const std::shared_ptr<const kern::Tonkern> f = kern::kerne().erzeuge("hammerString", kern::vorgaben(kern::kerne().typ("hammerString")->tafel));
    REQUIRE(f != nullptr);
    const double fl = anteil(*f, kern::einzelnote(21, 27.5, 0, 4 * 48000, static_cast<kern::Frames>(4.8 * 48000), 48000.0));
    MESSAGE("schwerster Ton: " << welcher << " " << 100.0 * schwerst << " % der Echtzeit, " << schwerst / fl << " x Fluegel (A0 " << 100.0 * fl << " %)");
    CHECK(schwerst > 0.0);
    CHECK(fl > 0.0);
}
