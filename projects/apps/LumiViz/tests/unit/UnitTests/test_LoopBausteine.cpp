/**
 ****************************************************************************************
 * @file   test_LoopBausteine.cpp
 * @brief  Loop, Stufe 16a: Signal path — die Reihe von Bausteinen an der Schicht
 *         eines Instruments und am Clip, `.lvinst` Fassung 7, `.lvloop` Fassung 5
 *         (docs/audio/Loop_Konzept.md §4j; Entscheide Patrik S117: K1 a bis K4 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Nur Modell und Dateien: ohne Ton, ohne Fenster, ohne Qt-Anwendung. Der Beleg
 * der Stufe: jede mitgelieferte `.lvinst` und `.lvloop` liest sich zu der
 * Reihe, die sie bis dahin fest hatte — Filter, Drive, Envelope an der
 * Schicht, Distortion, Echo, Reverb am Clip. Die Dateien liest der Test aus
 * dem Repo (`asset/loop`); wer eine dazulegt, braucht hier nichts zu aendern.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Effekte.hpp"
#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <string>
#include <variant>
#include <vector>

using namespace lumi::loop;

namespace
{

/// .../projects/apps/LumiViz/tests/unit/UnitTests -> Wurzel des Repos
QDir repoWurzel()
{
    QDir wurzel = QFileInfo(QString::fromLocal8Bit(__FILE__)).absoluteDir();
    for (int i = 0; i < 6; ++i) wurzel.cdUp();
    return wurzel;
}

QJsonObject liesJson(const QString& pfad)
{
    QFile datei(pfad);
    if (!datei.open(QIODevice::ReadOnly)) return {};
    return QJsonDocument::fromJson(datei.readAll()).object();
}

template <class Art>
bool ist(const SchichtBaustein& baustein)
{
    return std::holds_alternative<Art>(baustein);
}

/// Die Reihe, die eine Schicht bis Fassung 6 fest hatte: Filter (wenn Cutoff
/// ueber 0), Drive (wenn ueber 0), Envelope — aus den flachen Feldern.
std::vector<SchichtBaustein> festeReihe(const QJsonObject& schicht)
{
    std::vector<SchichtBaustein> reihe;
    const double cutoff = schicht.value(QStringLiteral("filterCutoff")).toDouble(0.0);
    if (cutoff > 0.0)
    {
        const QString art = schicht.value(QStringLiteral("filterType")).toString();
        reihe.emplace_back(SchichtFilter{art == QStringLiteral("highpass")   ? FilterArt::Hochpass
                                         : art == QStringLiteral("bandpass") ? FilterArt::Bandpass
                                                                             : FilterArt::Tiefpass,
                                         cutoff, schicht.value(QStringLiteral("filterEnvelope")).toDouble(0.0),
                                         schicht.value(QStringLiteral("filterResonance")).toDouble(0.0)});
    }
    const double drive = schicht.value(QStringLiteral("drive")).toDouble(0.0);
    if (drive > 0.0) reihe.emplace_back(SchichtDrive{drive});
    reihe.emplace_back(SchichtHuelle{});
    return reihe;
}

EchoZeit echoZeit(const QString& name)
{
    if (name == QStringLiteral("1/16")) return EchoZeit::Sechzehntel;
    if (name == QStringLiteral("1/8")) return EchoZeit::Achtel;
    if (name == QStringLiteral("1/4")) return EchoZeit::Viertel;
    if (name == QStringLiteral("1/2")) return EchoZeit::Halbe;
    return EchoZeit::AchtelPunktiert;
}

/// Die Reihe, die ein Clip bis Fassung 4 fest hatte: was in `effects` stand,
/// in der Folge Distortion, Echo, Reverb.
ClipEffekte festeEffekte(const QJsonObject& effekte)
{
    ClipEffekte e;
    if (const QJsonValue wert = effekte.value(QStringLiteral("distortion")); wert.isObject())
    {
        const QJsonObject v = wert.toObject();
        ClipVerzerrung w;
        w.an = v.value(QStringLiteral("on")).toBool(false);
        w.drive = v.value(QStringLiteral("drive")).toDouble(w.drive);
        w.ton = v.value(QStringLiteral("tone")).toDouble(w.ton);
        w.mix = v.value(QStringLiteral("mix")).toDouble(w.mix);
        e.reihe.emplace_back(w);
    }
    if (const QJsonValue wert = effekte.value(QStringLiteral("echo")); wert.isObject())
    {
        const QJsonObject v = wert.toObject();
        ClipEcho w;
        w.an = v.value(QStringLiteral("on")).toBool(false);
        if (v.contains(QStringLiteral("time"))) w.zeit = echoZeit(v.value(QStringLiteral("time")).toString());
        w.feedback = v.value(QStringLiteral("feedback")).toDouble(w.feedback);
        w.daempfung = v.value(QStringLiteral("damping")).toDouble(w.daempfung);
        w.mix = v.value(QStringLiteral("mix")).toDouble(w.mix);
        e.reihe.emplace_back(w);
    }
    if (const QJsonValue wert = effekte.value(QStringLiteral("reverb")); wert.isObject())
    {
        const QJsonObject v = wert.toObject();
        ClipHall w;
        w.an = v.value(QStringLiteral("on")).toBool(false);
        w.groesse = v.value(QStringLiteral("size")).toDouble(w.groesse);
        w.sekunden = v.value(QStringLiteral("decay")).toDouble(w.sekunden);
        w.daempfung = v.value(QStringLiteral("damping")).toDouble(w.daempfung);
        w.mix = v.value(QStringLiteral("mix")).toDouble(w.mix);
        e.reihe.emplace_back(w);
    }
    return e;
}

/// Ein Loop mit einem Muster und einem Clip darin.
Loop einClip()
{
    Loop loop;
    Muster m;
    m.name = QStringLiteral("M");
    loop.muster = {m};
    loop.kanaele = {{QStringLiteral("K"), true, {{QStringLiteral("M"), 0, 1}}}};
    loop.richte();
    return loop;
}

QStringList arten(const QJsonArray& reihe)
{
    QStringList namen;
    for (const QJsonValue& baustein : reihe) namen << baustein.toObject().value(QStringLiteral("type")).toString();
    return namen;
}

}  // namespace

TEST_CASE("Loop Signal path: eine Schicht traegt eine Reihe — neu nur Envelope, genau eine, hoechstens acht Bausteine")
{
    // Entscheide Patrik S117: K2 a (Filter und Drive beliebig oft, Envelope genau einmal), K4 a (acht)
    CHECK(kBausteineMax == 8);
    Schicht neu;
    REQUIRE(neu.reihe.size() == 1);
    CHECK(ist<SchichtHuelle>(neu.reihe[0]));
    CHECK(neu.filter() == SchichtFilter{});  // kein Filter: die Vorgabe, offen
    CHECK(neu.filter().teilton == 0.0);
    CHECK(neu.drive().wert == 0.0);
    CHECK(neu.reihe.size() == 1);  // lesen legt nichts an

    // --- der erste Filter und der erste Drive kommen an ihren Platz der alten Folge
    neu.holeDrive().wert = 0.4;
    neu.holeFilter().teilton = 3.0;
    REQUIRE(neu.reihe.size() == 3);
    CHECK(ist<SchichtFilter>(neu.reihe[0]));
    CHECK(ist<SchichtDrive>(neu.reihe[1]));
    CHECK(ist<SchichtHuelle>(neu.reihe[2]));
    neu.holeFilter().resonanz = 0.2;  // noch einmal holen ist derselbe
    neu.holeDrive().wert = 0.5;
    CHECK(neu.reihe.size() == 3);
    CHECK(neu.filter() == SchichtFilter{FilterArt::Tiefpass, 3.0, 0.0, 0.2});
    CHECK(neu.drive().wert == 0.5);

    // --- richten: mehr als acht fallen weg, die Envelope nie
    std::vector<SchichtBaustein> viele;
    for (int i = 0; i < 12; ++i) viele.emplace_back(SchichtDrive{0.05 * i});
    viele.emplace_back(SchichtHuelle{});
    richteReihe(viele);
    REQUIRE(viele.size() == 8);
    CHECK(ist<SchichtHuelle>(viele[7]));
    CHECK(std::count_if(viele.begin(), viele.end(), ist<SchichtDrive>) == 7);
    CHECK(std::get<SchichtDrive>(viele[6]).wert == doctest::Approx(0.3));  // die ersten sieben bleiben

    // --- von mehreren Envelopes bleibt die erste; fehlt sie, kommt sie ans Ende
    std::vector<SchichtBaustein> doppelt = {SchichtHuelle{}, SchichtDrive{0.1}, SchichtHuelle{}, SchichtFilter{}};
    richteReihe(doppelt);
    REQUIRE(doppelt.size() == 3);
    CHECK(ist<SchichtHuelle>(doppelt[0]));
    CHECK(ist<SchichtDrive>(doppelt[1]));
    CHECK(ist<SchichtFilter>(doppelt[2]));
    std::vector<SchichtBaustein> leer;
    richteReihe(leer);
    CHECK(leer == std::vector<SchichtBaustein>{SchichtHuelle{}});

    // --- Werte in ihren Grenzen
    std::vector<SchichtBaustein> wild = {SchichtFilter{static_cast<FilterArt>(9), 999.0, -1.0, 5.0}, SchichtDrive{7.0}};
    richteReihe(wild);
    REQUIRE(wild.size() == 3);
    CHECK(std::get<SchichtFilter>(wild[0]) == SchichtFilter{FilterArt::Tiefpass, 64.0, 0.0, 1.0});
    CHECK(std::get<SchichtDrive>(wild[1]).wert == 1.0);
}

TEST_CASE("Loop Signal path: .lvinst Fassung 7 — die Reihe steht in der Datei, in jeder Reihenfolge")
{
    QTemporaryDir ordner;
    REQUIRE(ordner.isValid());
    Instrument instrument;
    instrument.titel = QStringLiteral("Path");
    // zwei Filter, Drive hinter der Envelope: was die feste Folge nicht konnte
    instrument.schichten[0].reihe = {SchichtFilter{FilterArt::Hochpass, 2.5, 1.0, 0.3}, SchichtHuelle{}, SchichtDrive{0.4},
                                     SchichtFilter{FilterArt::Bandpass, 6.0, 0.0, 0.5}};
    instrument.schichten.push_back(Schicht{});  // eine neue Schicht: nur Envelope

    const QJsonObject json = alsJson(instrument);
    CHECK(kInstrumentFassung >= 7);  // die Reihe gibt es seit Fassung 7; geschrieben wird die juengste (seit Stufe 18: 8)
    CHECK(json.value(QStringLiteral("version")).toInt() == kInstrumentFassung);
    const QJsonArray schichten = json.value(QStringLiteral("layers")).toArray();
    REQUIRE(schichten.size() == 2);
    const QJsonObject erste = schichten.at(0).toObject();
    // die flachen Felder bis Fassung 6 gibt es nicht mehr; die Huellkurve bleibt an der Schicht
    for (const char* feld : {"drive", "filterCutoff", "filterEnvelope", "filterType", "filterResonance"})
        CHECK_FALSE(erste.contains(QLatin1String(feld)));
    for (const char* feld : {"attack", "decay", "sustain", "release"}) CHECK(erste.contains(QLatin1String(feld)));
    const QJsonArray weg = erste.value(QStringLiteral("path")).toArray();
    CHECK(arten(weg) == QStringList{QStringLiteral("filter"), QStringLiteral("envelope"), QStringLiteral("drive"),
                                    QStringLiteral("filter")});
    CHECK(weg.at(0).toObject().value(QStringLiteral("filterType")).toString() == QStringLiteral("highpass"));
    CHECK(weg.at(0).toObject().value(QStringLiteral("cutoff")).toDouble() == 2.5);
    CHECK(weg.at(0).toObject().value(QStringLiteral("envelope")).toDouble() == 1.0);
    CHECK(weg.at(0).toObject().value(QStringLiteral("resonance")).toDouble() == 0.3);
    CHECK(weg.at(2).toObject().value(QStringLiteral("amount")).toDouble() == 0.4);
    CHECK(arten(schichten.at(1).toObject().value(QStringLiteral("path")).toArray()) == QStringList{QStringLiteral("envelope")});

    // --- hin und zurueck, als JSON und als Datei
    Instrument gelesen;
    REQUIRE(ausJson(json, gelesen));
    CHECK(gelesen == instrument);
    const QString pfad = ordner.filePath(QStringLiteral("path.lvinst"));
    REQUIRE(speichereInstrument(instrument, pfad));
    Instrument ausDatei;
    REQUIRE(ladeInstrument(pfad, ausDatei));
    ausDatei.name = instrument.name;
    CHECK(ausDatei == instrument);

    // --- ein Baustein, den das Programm nicht kennt, faellt weg; der Rest bleibt
    const auto mitReihe = [&](const QJsonArray& reihe) {
        QJsonObject schicht = erste;
        schicht[QStringLiteral("path")] = reihe;
        QJsonObject wurzel = json;
        wurzel[QStringLiteral("layers")] = QJsonArray{schicht};
        Instrument neu;
        REQUIRE(ausJson(wurzel, neu));
        return neu.schichten[0].reihe;
    };
    QJsonArray fremd = weg;
    fremd.insert(1, QJsonObject{{QStringLiteral("type"), QStringLiteral("chorus")}});
    CHECK(mitReihe(fremd) == instrument.schichten[0].reihe);
    // --- mehr als acht: nach dem achten abgeschnitten — die Envelope bleibt
    QJsonArray lang;
    for (int i = 0; i < 10; ++i)
        lang.append(QJsonObject{{QStringLiteral("type"), QStringLiteral("drive")}, {QStringLiteral("amount"), 0.1}});
    lang.append(QJsonObject{{QStringLiteral("type"), QStringLiteral("envelope")}});
    const std::vector<SchichtBaustein> gekuerzt = mitReihe(lang);
    REQUIRE(gekuerzt.size() == 8);
    CHECK(ist<SchichtHuelle>(gekuerzt[7]));
    // --- eine leere Reihe oder eine ohne Envelope bekommt sie
    CHECK(mitReihe(QJsonArray{}) == std::vector<SchichtBaustein>{SchichtHuelle{}});
    // --- eine Fassung von morgen liest das Programm nicht
    QJsonObject morgen = json;
    morgen[QStringLiteral("version")] = kInstrumentFassung + 1;
    bool zuNeu = false;
    CHECK_FALSE(ausJson(morgen, gelesen, &zuNeu));
    CHECK(zuNeu);
}

TEST_CASE("Loop Signal path: eine .lvinst bis Fassung 6 liest sich als Filter, Drive, Envelope — jede mitgelieferte")
{
    // --- von Hand: die flachen Felder der Fassung 6
    QJsonObject schicht{{QStringLiteral("waveform"), QStringLiteral("saw")},
                        {QStringLiteral("filterCutoff"), 4.0},
                        {QStringLiteral("filterEnvelope"), 2.0},
                        {QStringLiteral("filterType"), QStringLiteral("highpass")},
                        {QStringLiteral("filterResonance"), 0.25},
                        {QStringLiteral("drive"), 0.3}};
    QJsonObject wurzel{{QStringLiteral("format"), QStringLiteral("lumiviz-instrument")},
                       {QStringLiteral("version"), 6},
                       {QStringLiteral("layers"), QJsonArray{schicht}}};
    Instrument alt;
    REQUIRE(ausJson(wurzel, alt));
    CHECK(alt.schichten[0].reihe == std::vector<SchichtBaustein>{SchichtFilter{FilterArt::Hochpass, 4.0, 2.0, 0.25},
                                                                 SchichtDrive{0.3}, SchichtHuelle{}});
    // ein Filter mit Cutoff 0 und ein Drive mit 0 taten nichts: sie werden kein Baustein (K2 a)
    schicht[QStringLiteral("filterCutoff")] = 0.0;
    schicht[QStringLiteral("drive")] = 0.0;
    wurzel[QStringLiteral("layers")] = QJsonArray{schicht};
    REQUIRE(ausJson(wurzel, alt));
    CHECK(alt.schichten[0].reihe == std::vector<SchichtBaustein>{SchichtHuelle{}});
    // nur Drive
    schicht[QStringLiteral("drive")] = 0.6;
    wurzel[QStringLiteral("layers")] = QJsonArray{schicht};
    REQUIRE(ausJson(wurzel, alt));
    CHECK(alt.schichten[0].reihe == std::vector<SchichtBaustein>{SchichtDrive{0.6}, SchichtHuelle{}});
    // Fassung 1 ist flach: dieselben Felder an der Wurzel
    QJsonObject flach{{QStringLiteral("format"), QStringLiteral("lumiviz-instrument")},
                      {QStringLiteral("version"), 1},
                      {QStringLiteral("filterCutoff"), 3.0},
                      {QStringLiteral("drive"), 0.2}};
    REQUIRE(ausJson(flach, alt));
    CHECK(alt.schichten[0].reihe == std::vector<SchichtBaustein>{SchichtFilter{FilterArt::Tiefpass, 3.0, 0.0, 0.0},
                                                                 SchichtDrive{0.2}, SchichtHuelle{}});

    // --- jede mitgelieferte Datei: dieselbe Reihe, die sie bis Fassung 6 fest hatte
    const QDir ordner(repoWurzel().filePath(QStringLiteral("asset/loop/instruments")));
    const QStringList dateien = ordner.entryList({QStringLiteral("*.lvinst")}, QDir::Files, QDir::Name);
    REQUIRE(dateien.size() >= 55);
    int schichtenGeprueft = 0;
    int mitFilter = 0;
    int mitDrive = 0;
    for (const QString& datei : dateien)
    {
        CAPTURE(datei.toStdString());
        const QJsonObject json = liesJson(ordner.filePath(datei));
        REQUIRE(!json.isEmpty());
        Instrument instrument;
        REQUIRE(ladeInstrument(ordner.filePath(datei), instrument));
        const int fassung = json.value(QStringLiteral("version")).toInt(1);
        QJsonArray schichten = json.value(QStringLiteral("layers")).toArray();
        if (fassung < 2) schichten = QJsonArray{json};
        REQUIRE(static_cast<int>(instrument.schichten.size()) == schichten.size());
        for (int i = 0; i < schichten.size(); ++i)
        {
            CAPTURE(i);
            const QJsonObject s = schichten.at(i).toObject();
            const std::vector<SchichtBaustein>& reihe = instrument.schichten[static_cast<std::size_t>(i)].reihe;
            CHECK(std::count_if(reihe.begin(), reihe.end(), ist<SchichtHuelle>) == 1);
            if (s.contains(QStringLiteral("path"))) continue;  // schon Fassung 7: sie traegt ihre Reihe selbst
            CHECK(reihe == festeReihe(s));
            ++schichtenGeprueft;
            if (ist<SchichtFilter>(reihe[0])) ++mitFilter;
            if (std::any_of(reihe.begin(), reihe.end(), ist<SchichtDrive>)) ++mitDrive;
        }
        // schreiben und lesen ergeben dasselbe
        Instrument nochmal;
        nochmal.name = instrument.name;
        REQUIRE(ausJson(alsJson(instrument), nochmal));
        CHECK(nochmal == instrument);
    }
    MESSAGE(dateien.size() << " Instrumente, " << schichtenGeprueft << " Schichten bis Fassung 6 geprueft, davon " << mitFilter
                           << " mit Filter, " << mitDrive << " mit Drive");
    CHECK(mitFilter > 0);
    CHECK(mitDrive > 0);
}

TEST_CASE("Loop Signal path: ein Clip traegt eine Reihe — .lvloop Fassung 5 mit dem Filter als viertem Baustein")
{
    // Entscheide Patrik S117: K3 a (Filter, Distortion, Echo, Reverb, beliebig oft, freie Folge), K4 a (acht)
    ClipEffekte leer;
    CHECK(leer.reihe.empty());
    CHECK_FALSE(leer.einerAn());
    CHECK(leer.erster<ClipHall>() == nullptr);
    CHECK(leer.wert<ClipHall>() == ClipHall{});  // die Vorgabe, ausgeschaltet
    CHECK(leer.reihe.empty());                   // lesen legt nichts an

    // --- der erste je Art kommt an seinen Platz der alten Folge, der Filter davor
    ClipEffekte geholt;
    geholt.hole<ClipHall>().an = true;
    geholt.hole<ClipVerzerrung>().drive = 0.8;
    geholt.hole<ClipFilter>().hz = 500.0;
    geholt.hole<ClipEcho>().mix = 0.1;
    geholt.hole<ClipHall>().mix = 0.5;  // noch einmal holen ist derselbe
    REQUIRE(geholt.reihe.size() == 4);
    CHECK(std::holds_alternative<ClipFilter>(geholt.reihe[0]));
    CHECK(std::holds_alternative<ClipVerzerrung>(geholt.reihe[1]));
    CHECK(std::holds_alternative<ClipEcho>(geholt.reihe[2]));
    CHECK(std::holds_alternative<ClipHall>(geholt.reihe[3]));
    CHECK(geholt.einerAn());  // nur der Hall ist an

    // --- die Datei: eine freie Folge, zwei Echos, ein ausgeschalteter Baustein
    Loop loop = einClip();
    const ClipEffekte reihe{{ClipEcho{true, EchoZeit::Viertel, 0.5, 0.2, 0.4}, ClipFilter{true, FilterArt::Hochpass, 300.0, 0.4},
                             ClipVerzerrung{false, 0.6, 0.5, 1.0}, ClipEcho{true, EchoZeit::Achtel, 0.3, 0.1, 0.2},
                             ClipHall{true, 0.5, 2.0, 0.4, 0.3}}};
    loop.kanaele[0].clips[0].effekte = reihe;
    const QJsonObject json = alsJson(loop);
    CHECK(json.value(QStringLiteral("version")).toInt() == 7);  // seit S124: Muster ab 1 Takt (6), fxOn am Clip (7)
    const auto clipIn = [](const QJsonObject& wurzel) {
        return wurzel.value(QStringLiteral("channels")).toArray().at(0).toObject().value(QStringLiteral("clips")).toArray().at(0).toObject();
    };
    CHECK_FALSE(clipIn(json).contains(QStringLiteral("effects")));
    const QJsonArray weg = clipIn(json).value(QStringLiteral("path")).toArray();
    CHECK(arten(weg) == QStringList{QStringLiteral("echo"), QStringLiteral("filter"), QStringLiteral("distortion"),
                                    QStringLiteral("echo"), QStringLiteral("reverb")});
    CHECK(weg.at(1).toObject().value(QStringLiteral("filterType")).toString() == QStringLiteral("highpass"));
    CHECK(weg.at(1).toObject().value(QStringLiteral("cutoff")).toDouble() == 300.0);  // in Hertz
    CHECK(weg.at(1).toObject().value(QStringLiteral("resonance")).toDouble() == 0.4);
    CHECK(weg.at(2).toObject().value(QStringLiteral("on")).toBool(true) == false);
    Loop gelesen;
    QStringList bericht;
    REQUIRE(ausJson(json, gelesen, {}, &bericht));
    CHECK(bericht.isEmpty());
    CHECK(gelesen == loop);
    CHECK(gelesen.kanaele[0].clips[0].effekte == reihe);
    // jeder Baustein, der nachklingt, zaehlt: zwei Echos und der Hall
    ClipEffekte einEcho = reihe;
    einEcho.reihe.erase(einEcho.reihe.begin() + 3);
    CHECK(nachklangSekunden(reihe, 120.0) > nachklangSekunden(einEcho, 120.0));

    // --- ein Baustein, den das Programm nicht kennt, faellt weg und steht im Bericht
    const auto mitReihe = [&](const QJsonArray& neueReihe, QStringList* meldung = nullptr) {
        QJsonObject clip = clipIn(json);
        clip[QStringLiteral("path")] = neueReihe;
        QJsonObject kanal = json.value(QStringLiteral("channels")).toArray().at(0).toObject();
        kanal[QStringLiteral("clips")] = QJsonArray{clip};
        QJsonObject wurzel = json;
        wurzel[QStringLiteral("channels")] = QJsonArray{kanal};
        Loop neu;
        REQUIRE(ausJson(wurzel, neu, {}, meldung));
        return neu.kanaele[0].clips[0].effekte;
    };
    QJsonArray fremd = weg;
    fremd.insert(2, QJsonObject{{QStringLiteral("type"), QStringLiteral("autotune")}});
    bericht.clear();
    CHECK(mitReihe(fremd, &bericht) == reihe);
    CHECK(bericht == QStringList{QStringLiteral("Unknown effect 'autotune'")});

    // --- mehr als acht: nach dem achten abgeschnitten; Werte in ihren Grenzen
    QJsonArray lang;
    for (int i = 0; i < 11; ++i) lang.append(weg.at(i % weg.size()));
    CHECK(mitReihe(lang).reihe.size() == 8);
    QJsonObject tief = weg.at(1).toObject();
    tief[QStringLiteral("cutoff")] = 5.0;
    tief[QStringLiteral("resonance")] = 3.0;
    QJsonObject hoch = tief;
    hoch[QStringLiteral("cutoff")] = 99999.0;
    hoch[QStringLiteral("filterType")] = QStringLiteral("notch");  // unbekannt: der Tiefpass
    const ClipEffekte grenzen = mitReihe(QJsonArray{tief, hoch});
    REQUIRE(grenzen.reihe.size() == 2);
    CHECK(std::get<ClipFilter>(grenzen.reihe[0]) == ClipFilter{true, FilterArt::Hochpass, kClipFilterHzMin, 1.0});
    CHECK(std::get<ClipFilter>(grenzen.reihe[1]) == ClipFilter{true, FilterArt::Tiefpass, kClipFilterHzMax, 1.0});
    CHECK(kClipFilterHzMin == 40.0);
    CHECK(kClipFilterHzMax == 18000.0);
}

TEST_CASE("Loop Signal path: eine .lvloop bis Fassung 4 liest ihre Effekte als Distortion, Echo, Reverb — jede mitgelieferte")
{
    // --- von Hand: das Feld `effects` der Fassungen 3 und 4 trug nur, was von der Vorgabe abwich
    Loop loop = einClip();
    QJsonObject json = alsJson(loop);
    json[QStringLiteral("version")] = 4;
    {
        QJsonObject kanal = json.value(QStringLiteral("channels")).toArray().at(0).toObject();
        QJsonObject clip = kanal.value(QStringLiteral("clips")).toArray().at(0).toObject();
        clip[QStringLiteral("effects")] =
            QJsonObject{{QStringLiteral("reverb"), QJsonObject{{QStringLiteral("on"), true}, {QStringLiteral("decay"), 3.0}}},
                        {QStringLiteral("distortion"), QJsonObject{{QStringLiteral("on"), false}, {QStringLiteral("drive"), 0.8}}}};
        kanal[QStringLiteral("clips")] = QJsonArray{clip};
        json[QStringLiteral("channels")] = QJsonArray{kanal};
    }
    Loop alt;
    QStringList bericht;
    REQUIRE(ausJson(json, alt, {}, &bericht));
    CHECK(bericht.isEmpty());
    ClipVerzerrung verzerrung;  // aus, aber eingestellt: er steht in der Reihe
    verzerrung.drive = 0.8;
    ClipHall hall;
    hall.an = true;
    hall.sekunden = 3.0;
    // kein Echo in der Datei: kein Baustein
    CHECK(alt.kanaele[0].clips[0].effekte == ClipEffekte{{verzerrung, hall}});

    // --- jede mitgelieferte Datei: dieselbe Reihe, die ihre Clips bis Fassung 4 fest hatten
    int dateien = 0;
    int clipsGeprueft = 0;
    int mitEffekten = 0;
    QDirIterator lauf(repoWurzel().filePath(QStringLiteral("asset/loop")), {QStringLiteral("*.lvloop")}, QDir::Files,
                      QDirIterator::Subdirectories);
    while (lauf.hasNext())
    {
        const QString pfad = lauf.next();
        CAPTURE(pfad.toStdString());
        const QJsonObject wurzel = liesJson(pfad);
        REQUIRE(!wurzel.isEmpty());
        Loop geladen;
        REQUIRE(ladeLoop(pfad, geladen));
        ++dateien;
        const QJsonArray kanaele = wurzel.value(QStringLiteral("channels")).toArray();
        REQUIRE(static_cast<int>(geladen.kanaele.size()) == kanaele.size());
        for (int k = 0; k < kanaele.size(); ++k)
        {
            const std::vector<LoopClip>& clips = geladen.kanaele[static_cast<std::size_t>(k)].clips;
            for (const QJsonValue& eintrag : kanaele.at(k).toObject().value(QStringLiteral("clips")).toArray())
            {
                const QJsonObject c = eintrag.toObject();
                if (c.contains(QStringLiteral("path"))) continue;  // schon Fassung 5: er traegt seine Reihe selbst
                const QString muster = c.value(QStringLiteral("pattern")).toString();
                const int start = c.value(QStringLiteral("start")).toInt(0);
                CAPTURE(k);
                CAPTURE(start);
                const auto clip = std::find_if(clips.begin(), clips.end(), [&](const LoopClip& einer) {
                    return einer.muster == muster && einer.startTakt == start;
                });
                REQUIRE(clip != clips.end());
                const ClipEffekte erwartet = festeEffekte(c.value(QStringLiteral("effects")).toObject());
                CHECK(clip->effekte == erwartet);
                ++clipsGeprueft;
                if (!erwartet.reihe.empty()) ++mitEffekten;
            }
        }
        // schreiben und lesen ergeben dasselbe
        const QString basis = QFileInfo(pfad).absolutePath();
        Loop nochmal;
        REQUIRE(ausJson(alsJson(geladen, basis), nochmal, basis));
        CHECK(nochmal == geladen);
    }
    MESSAGE(dateien << " Loops, " << clipsGeprueft << " Clips bis Fassung 4 geprueft, davon " << mitEffekten << " mit Effekten");
    CHECK(dateien >= 20);
    CHECK(mitEffekten >= 5);  // die fuenf aus effects_demo.lvloop
}

// =============================================================================
// Stufe 16b: der Klangerzeuger rechnet die Reihe
// =============================================================================
// Dass die Reihe Filter, Drive, Envelope (und Distortion, Echo, Reverb am
// Clip) Wert fuer Wert ergibt, was vorher fest gerechnet wurde, belegt
// test_LoopWerte.cpp an jeder mitgelieferten Datei. Hier steht das Neue.

namespace
{

constexpr int kRate = 44100;

/// Der groesste Betrag der linken Seite in einem Stueck des Tons.
double spitzeIn(const Klang& klang, double vonMs, double bisMs)
{
    const auto von = static_cast<std::size_t>(vonMs * kRate / 1000.0);
    const auto bis = std::min(static_cast<std::size_t>(bisMs * kRate / 1000.0), klang.daten.size() / 2);
    double groesster = 0.0;
    for (std::size_t f = von; f < bis; ++f) groesster = std::max(groesster, static_cast<double>(std::abs(klang.daten[2 * f])));
    return groesster;
}

double summeBetraege(const std::vector<float>& daten)
{
    double summe = 0.0;
    for (const float wert : daten) summe += std::abs(wert);
    return summe;
}

/// Ein Instrument mit einer Schicht und dieser Reihe.
Instrument mitReihe(Schwingung schwingung, std::vector<SchichtBaustein> reihe)
{
    Instrument instrument;
    Schicht& schicht = instrument.schichten[0];
    schicht.schwingung = schwingung;
    schicht.anstiegMs = 5.0;
    schicht.abfallMs = 60.0;
    schicht.halten = 0.2;
    schicht.ausklangMs = 100.0;
    schicht.reihe = std::move(reihe);
    return instrument;
}

/// Ein Sinus von `hz` auf beiden Seiten.
std::vector<float> sinusTon(double hz, int frames, float spitze = 0.5f)
{
    std::vector<float> werte(2 * static_cast<std::size_t>(frames));
    for (int f = 0; f < frames; ++f)
    {
        const auto wert = static_cast<float>(spitze * std::sin(2.0 * 3.14159265358979323846 * hz * f / kRate));
        werte[static_cast<std::size_t>(2 * f)] = wert;
        werte[static_cast<std::size_t>(2 * f + 1)] = wert;
    }
    return werte;
}

/// Eine Kette ueber einen Ton rechnen, in Bloecken von `block` Abtastwerten.
std::vector<float> durchKette(const ClipEffekte& effekte, std::vector<float> werte, int block = 441)
{
    EffektKette kette;
    kette.richteEin(effekte, kRate, 120.0);
    const int frames = static_cast<int>(werte.size() / 2);
    for (int stelle = 0; stelle < frames; stelle += block)
        kette.rechne(werte.data() + 2 * static_cast<std::ptrdiff_t>(stelle), std::min(block, frames - stelle));
    return werte;
}

/// Eine feste Rechenarbeit, gegen die sich eine Messung vergleichen laesst:
/// drosselt der Rechner, dauert auch sie laenger (S106).
double taktprobeMs()
{
    const auto start = std::chrono::steady_clock::now();
    volatile double summe = 0.0;
    for (int i = 1; i <= 4'000'000; ++i) summe = summe + std::sqrt(static_cast<double>(i));
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
}

}  // namespace

TEST_CASE("Loop Signal path: die Schicht rechnet ihre Reihe — die Reihenfolge ist zu hoeren, zwei Filter filtern zweimal")
{
    // --- Drive vor oder hinter der Envelope (K2 a): hinter ihr verzerrt er den schon leisen
    //     Ton weniger hart, hebt ihn aber an — im gehaltenen Teil ist der Ton viel lauter
    const Instrument davor = mitReihe(Schwingung::Sinus, {SchichtDrive{0.8}, SchichtHuelle{}});
    const Instrument dahinter = mitReihe(Schwingung::Sinus, {SchichtHuelle{}, SchichtDrive{0.8}});
    const Klang tonDavor = rendereTon(davor, 57, 600.0, kRate);
    const Klang tonDahinter = rendereTon(dahinter, 57, 600.0, kRate);
    REQUIRE(tonDavor.frames() == tonDahinter.frames());
    // gehalten wird bei 0,2: davor 0,5 * 0,2 = 0,1; dahinter 0,5 * tanh(6,4 * 0,2) / tanh(6,4) = 0,43
    CHECK(spitzeIn(tonDavor, 400.0, 600.0) == doctest::Approx(0.1).epsilon(0.02));
    CHECK(spitzeIn(tonDahinter, 400.0, 600.0) == doctest::Approx(0.428).epsilon(0.02));
    // der Ton endet, wo die Envelope auch steht: ihr letzter Wert ist winzig, Drive hebt ihn kaum
    CHECK(std::abs(tonDahinter.daten.back()) < 1.0e-5f);

    // --- ein offener Filter und ein Drive mit 0 tun nichts: dieselben Werte wie ohne sie
    const Instrument schlicht = mitReihe(Schwingung::Saege, {SchichtFilter{FilterArt::Tiefpass, 3.0, 0.0, 0.0}, SchichtHuelle{}});
    const Instrument mitNichts =
        mitReihe(Schwingung::Saege, {SchichtDrive{0.0}, SchichtFilter{FilterArt::Tiefpass, 3.0, 0.0, 0.0},
                                     SchichtFilter{FilterArt::Hochpass, 0.0, 5.0, 0.7}, SchichtHuelle{}, SchichtDrive{0.0}});
    CHECK(rendereTon(mitNichts, 60, 300.0, kRate).daten == rendereTon(schlicht, 60, 300.0, kRate).daten);

    // --- zwei Filter (der Clap aus §4h): Rauschen durch Hochpass und Tiefpass ist ein Band —
    //     weniger als durch einen der beiden allein, und zweimal derselbe filtert steiler
    const SchichtFilter hoch{FilterArt::Hochpass, 4.0, 0.0, 0.0};
    const SchichtFilter tief{FilterArt::Tiefpass, 8.0, 0.0, 0.0};
    const auto rausch = [&](std::vector<SchichtBaustein> reihe) {
        return summeBetraege(rendereTon(mitReihe(Schwingung::Rauschen, std::move(reihe)), 69, 300.0, kRate).daten);
    };
    const double nurHoch = rausch({hoch, SchichtHuelle{}});
    const double nurTief = rausch({tief, SchichtHuelle{}});
    const double band = rausch({hoch, tief, SchichtHuelle{}});
    const double zweimalTief = rausch({tief, tief, SchichtHuelle{}});
    CHECK(band < nurHoch * 0.9);
    CHECK(band < nurTief * 0.9);
    CHECK(zweimalTief < nurTief * 0.95);
    CHECK(band > 0.0);

    // --- ein Filter hinter der Envelope schwingt nach: der Ton endet trotzdem bei null
    const Instrument filterHinten =
        mitReihe(Schwingung::Saege, {SchichtHuelle{}, SchichtFilter{FilterArt::Bandpass, 2.0, 0.0, 0.9}});
    const Klang hinten = rendereTon(filterHinten, 45, 200.0, kRate);
    CHECK(hinten.daten.back() == 0.0f);
    CHECK(hinten.daten[hinten.daten.size() - 2] == 0.0f);
    CHECK(spitzeIn(hinten, 0.0, 200.0) > 0.01);
    const Instrument filterVorn =
        mitReihe(Schwingung::Saege, {SchichtFilter{FilterArt::Bandpass, 2.0, 0.0, 0.9}, SchichtHuelle{}});
    CHECK(rendereTon(filterVorn, 45, 200.0, kRate).daten != hinten.daten);

    // --- die Saite hatte nie einen Filter: sie ueberspringt die der Reihe weiter; Drive wirkt
    const Instrument saite = mitReihe(Schwingung::Saite, {SchichtHuelle{}});
    const Instrument saiteFilter = mitReihe(Schwingung::Saite, {SchichtFilter{FilterArt::Tiefpass, 1.0, 0.0, 0.5}, SchichtHuelle{}});
    const Instrument saiteDrive = mitReihe(Schwingung::Saite, {SchichtDrive{0.6}, SchichtHuelle{}});
    CHECK(rendereTon(saiteFilter, 52, 300.0, kRate).daten == rendereTon(saite, 52, 300.0, kRate).daten);
    CHECK(rendereTon(saiteDrive, 52, 300.0, kRate).daten != rendereTon(saite, 52, 300.0, kRate).daten);

    // --- eine Aufnahme hat zwei Seiten: jeder Filter der Reihe fuehrt je Seite sein Gedaechtnis —
    //     das belegt test_LoopSample.cpp am Filter; hier nur: derselbe Aufruf, dieselben Werte
    const Instrument voll = mitReihe(Schwingung::Rechteck, {hoch, SchichtDrive{0.5}, tief, SchichtHuelle{}, SchichtDrive{0.2},
                                                           SchichtFilter{FilterArt::Bandpass, 3.0, 2.0, 0.4}, tief, hoch});
    CHECK(voll.schichten[0].reihe.size() == static_cast<std::size_t>(kBausteineMax));
    CHECK(rendereTon(voll, 60, 250.0, kRate).daten == rendereTon(voll, 60, 250.0, kRate).daten);
    CHECK(rendereTon(voll, 60, 250.0, kRate).daten.back() == 0.0f);
}

TEST_CASE("Loop Signal path: die Kette am Clip rechnet in freier Reihenfolge — und den Filter in Hertz")
{
    const int frames = 2 * kRate;  // zwei Sekunden: bei 120 BPM vier Viertel
    std::vector<float> schlag(2 * static_cast<std::size_t>(frames), 0.0f);
    for (int f = 0; f < 2000; ++f)  // ein kurzer Ton am Anfang, danach Stille
    {
        const auto wert = static_cast<float>(0.4 * std::sin(2.0 * 3.14159265358979323846 * 330.0 * f / kRate));
        schlag[static_cast<std::size_t>(2 * f)] = wert;
        schlag[static_cast<std::size_t>(2 * f + 1)] = wert;
    }
    const ClipVerzerrung verzerrung{true, 0.8, 1.0, 1.0};
    const ClipEcho viertel{true, EchoZeit::Viertel, 0.5, 0.0, 0.5};
    const ClipEcho sechzehntel{true, EchoZeit::Sechzehntel, 0.4, 0.0, 0.5};

    // --- die Reihenfolge: Distortion vor dem Echo verzerrt nur den Schlag, dahinter auch jede
    //     Wiederholung — die leisen Wiederholungen werden dann angehoben
    const std::vector<float> erstVerzerrt = durchKette({{verzerrung, viertel}}, schlag);
    const std::vector<float> erstEcho = durchKette({{viertel, verzerrung}}, schlag);
    CHECK(erstVerzerrt != erstEcho);
    const auto spaet = [&](const std::vector<float>& werte) {
        // ab der zweiten Wiederholung (sie liegt bei 1 s, die dritte bei 1,5 s)
        return summeBetraege(std::vector<float>(werte.begin() + 2 * static_cast<std::ptrdiff_t>(0.9 * kRate), werte.end()));
    };
    CHECK(spaet(erstEcho) > spaet(erstVerzerrt) * 1.5);

    // --- jeder Baustein beliebig oft: zwei Echos rechnen beide
    const std::vector<float> einEcho = durchKette({{viertel}}, schlag);
    const std::vector<float> zweiEchos = durchKette({{viertel, sechzehntel}}, schlag);
    CHECK(zweiEchos != einEcho);
    // zwischen Schlag und erster Viertel-Wiederholung (0,5 s) ist mit einem Echo Stille, mit zweien nicht
    const auto luecke = [&](const std::vector<float>& werte) {
        return summeBetraege(std::vector<float>(werte.begin() + 2 * static_cast<std::ptrdiff_t>(0.1 * kRate),
                                                werte.begin() + 2 * static_cast<std::ptrdiff_t>(0.45 * kRate)));
    };
    CHECK(luecke(einEcho) == 0.0);
    CHECK(luecke(zweiEchos) > 1.0);

    // --- ein ausgeschalteter Baustein steht in der Reihe und wird uebersprungen
    ClipVerzerrung aus = verzerrung;
    aus.an = false;
    ClipFilter filterAus{false, FilterArt::Hochpass, 5000.0, 0.5};
    CHECK(durchKette({{aus, viertel, filterAus}}, schlag) == einEcho);

    // --- der Filter: die Grenze in Hertz, gleich welche Note gespielt wird
    const std::vector<float> tiefTon = sinusTon(200.0, frames);
    const auto pegel = [&](const ClipFilter& filter, const std::vector<float>& ton) {
        const std::vector<float> aus = durchKette({{filter}}, ton);
        // die zweite Haelfte: der Filter ist eingeschwungen
        return summeBetraege(std::vector<float>(aus.begin() + frames, aus.end())) /
               summeBetraege(std::vector<float>(ton.begin() + frames, ton.end()));
    };
    CHECK(pegel({true, FilterArt::Hochpass, 2000.0, 0.0}, tiefTon) < 0.02);  // 200 Hz unter 2 kHz: 40 dB weg
    CHECK(pegel({true, FilterArt::Tiefpass, 2000.0, 0.0}, tiefTon) == doctest::Approx(1.0).epsilon(0.02));
    CHECK(pegel({true, FilterArt::Bandpass, 200.0, 0.5}, tiefTon) == doctest::Approx(1.0).epsilon(0.02));
    CHECK(pegel({true, FilterArt::Bandpass, 4000.0, 0.5}, tiefTon) < 0.05);
    CHECK(pegel({true, FilterArt::Tiefpass, 100.0, 0.0}, sinusTon(1000.0, frames)) < 0.02);
    // Resonance betont die Grenze: ein Ton genau dort kommt mit ihr lauter heraus als ohne
    const std::vector<float> anDerGrenze = sinusTon(1000.0, frames, 0.2f);
    CHECK(pegel({true, FilterArt::Tiefpass, 1000.0, 0.8}, anDerGrenze) > pegel({true, FilterArt::Tiefpass, 1000.0, 0.0}, anDerGrenze) * 1.5);
    // gleich in welchen Bloecken gerechnet wird
    const ClipEffekte alle{{ClipFilter{true, FilterArt::Bandpass, 800.0, 0.6}, sechzehntel, verzerrung,
                            ClipFilter{true, FilterArt::Tiefpass, 3000.0, 0.2}, ClipHall{true, 0.5, 1.0, 0.4, 0.3}}};
    CHECK(durchKette(alle, schlag, 441) == durchKette(alle, schlag, 64));
    CHECK(durchKette(alle, schlag, 441) == durchKette(alle, schlag, frames));

    // --- der Nachklang ueberlebt, wenn ein Baustein anderer Art dazukommt: der erste Echo hier
    //     gehoert zum ersten Echo dort
    EffektKette alt;
    alt.richteEin({{viertel}}, kRate, 120.0);
    std::vector<float> anfang(schlag.begin(), schlag.begin() + 2 * 4410);
    alt.rechne(anfang.data(), 4410);
    EffektKette neu;
    neu.richteEin({{filterAus, viertel}}, kRate, 120.0);
    neu.uebernimm(alt);
    std::vector<float> stille(2 * static_cast<std::size_t>(frames), 0.0f);
    neu.rechne(stille.data(), frames);
    CHECK(summeBetraege(stille) > 1.0);  // das Echo des Schlags kommt noch
    EffektKette leer;
    leer.richteEin({{filterAus, viertel}}, kRate, 120.0);
    std::vector<float> nichts(2 * static_cast<std::size_t>(frames), 0.0f);
    leer.rechne(nichts.data(), frames);
    CHECK(summeBetraege(nichts) < 1.0e-6);  // ohne uebernommenes Gedaechtnis: Stille (bis auf den Schutz vor Denormalen)

    // --- der Clip klingt mit einem Filter mit Resonanz etwas laenger nach
    CHECK(nachklangSekunden({{ClipFilter{true, FilterArt::Tiefpass, 40.0, 1.0}}}, 120.0) > 0.3);
    CHECK(nachklangSekunden({{ClipFilter{true, FilterArt::Tiefpass, 5000.0, 0.0}}}, 120.0) < 0.01);
    CHECK(nachklangSekunden({{filterAus}}, 120.0) == 0.0);
}

TEST_CASE("Loop Signal path Messung: acht Bausteine an der Schicht und am Clip — die Rechenzeit")
{
    // Drei Laeufe je Stellung, davor und danach die Taktprobe: dauert sie laenger als sonst,
    // hat der Rechner gedrosselt und die Zahl gilt nicht. Geprueft wird nur, dass es traegt.
    const SchichtFilter filter{FilterArt::Bandpass, 3.0, 2.0, 0.4};
    Instrument eins = mitReihe(Schwingung::Saege, {filter, SchichtDrive{0.3}, SchichtHuelle{}});
    Instrument acht = mitReihe(Schwingung::Saege, {filter, SchichtDrive{0.3}, filter, SchichtDrive{0.3}, filter, SchichtHuelle{},
                                                   filter, SchichtDrive{0.3}});
    for (Instrument* instrument : {&eins, &acht})
    {
        instrument->schichten[0].stimmen = 3;
        instrument->schichten[0].ausklangMs = 1000.0;
    }
    const auto messeTon = [&](const std::string& name, const Instrument& instrument) {
        double bester = 1.0e30;
        for (int lauf = 1; lauf <= 3; ++lauf)
        {
            const double davor = taktprobeMs();
            const auto start = std::chrono::steady_clock::now();
            const Klang klang = rendereTon(instrument, 60, 1000.0, kRate);  // zwei Sekunden Ton
            const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
            const double danach = taktprobeMs();
            const double sekunden = static_cast<double>(klang.frames()) / kRate;
            bester = std::min(bester, ms / sekunden);
            MESSAGE(name << ", Lauf " << lauf << ": " << ms << " ms fuer " << sekunden << " s Ton = " << ms / sekunden
                         << " ms je Sekunde; Taktprobe davor " << davor << " ms, danach " << danach << " ms");
        }
        return bester;
    };
    const double schichtEins = messeTon("Schicht mit Filter, Drive, Envelope", eins);
    const double schichtAcht = messeTon("Schicht mit acht Bausteinen (vier Filter, drei Drive)", acht);
    // ein Ton wird vorab gerechnet, nicht im Ton-Thread: er darf dauern, aber nicht ewig
    CHECK(schichtAcht < 1000.0);
    MESSAGE("Schicht: acht Bausteine kosten das " << schichtAcht / schichtEins << "-fache der alten Reihe");

    const int sekunden = 5;
    const int frames = sekunden * kRate;
    std::vector<float> ton(2 * static_cast<std::size_t>(frames));
    std::uint32_t folge = 12345u;
    for (float& wert : ton)
    {
        folge = folge * 1664525u + 1013904223u;
        wert = (static_cast<float>(folge >> 8) / 8388608.0f - 1.0f) * 0.5f;
    }
    const ClipHall hall{true, 0.5, 1.8, 0.4, 0.25};
    const ClipEffekte alteReihe{{ClipVerzerrung{true, 0.3, 0.5, 1.0}, ClipEcho{true, EchoZeit::AchtelPunktiert, 0.4, 0.3, 0.35}, hall}};
    const ClipEffekte achtHall{{hall, hall, hall, hall, hall, hall, hall, hall}};
    const ClipFilter clipFilter{true, FilterArt::Tiefpass, 2000.0, 0.3};
    const ClipEffekte achtFilter{{clipFilter, clipFilter, clipFilter, clipFilter, clipFilter, clipFilter, clipFilter, clipFilter}};
    const auto messeKette = [&](const std::string& name, const ClipEffekte& effekte) {
        double bester = 1.0e30;
        for (int lauf = 1; lauf <= 3; ++lauf)
        {
            EffektKette kette;
            kette.richteEin(effekte, kRate, 120.0);
            std::vector<float> werte = ton;
            const double davor = taktprobeMs();
            const auto start = std::chrono::steady_clock::now();
            for (int stelle = 0; stelle < frames; stelle += 441)
                kette.rechne(werte.data() + 2 * static_cast<std::ptrdiff_t>(stelle), std::min(441, frames - stelle));
            const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
            const double danach = taktprobeMs();
            bester = std::min(bester, ms / sekunden);
            MESSAGE(name << ", Lauf " << lauf << ": " << ms / sekunden << " ms je Sekunde Ton (" << ms / sekunden / 10.0
                         << " % eines Kerns); Taktprobe davor " << davor << " ms, danach " << danach << " ms");
        }
        return bester;
    };
    const double alt = messeKette("Clip mit Distortion, Echo, Reverb", alteReihe);
    const double halle = messeKette("Clip mit acht Reverb", achtHall);
    const double filterKette = messeKette("Clip mit acht Filtern", achtFilter);
    // die Kette rechnet im Ton-Thread: sie muss schneller sein, als der Ton laeuft
    CHECK(alt < 1000.0);
    CHECK(halle < 1000.0);
    CHECK(filterKette < 1000.0);
}

TEST_CASE("Loop Instrument Messung: ein Ton mit einer, vier, sechs und acht Schichten — Rechenzeit und Spitze")
{
    // Frage Patrik (S114, S120): kann ein Instrument mehr als vier Schichten tragen? `rendereTon`
    // rechnet, was im Instrument steht — die Grenze `kSchichtenMax` setzen die Datei und der
    // Creator. Jede Schicht hier ist eine teure: Saege, drei Stimmen, Filter, Drive, ein langer
    // Ausklang. Drei Laeufe je Zahl mit der Taktprobe; geprueft wird nur, dass es traegt.
    const SchichtFilter filter{FilterArt::Tiefpass, 6.0, 2.0, 0.2};
    Instrument vorlage = mitReihe(Schwingung::Saege, {filter, SchichtDrive{0.3}, SchichtHuelle{}});
    vorlage.schichten[0].stimmen = 3;
    vorlage.schichten[0].verstimmungCent = 8.0;
    vorlage.schichten[0].ausklangMs = 1000.0;
    const int versatz[8] = {0, 12, 7, -12, 19, 4, 24, -5};
    double eine = 0.0;
    for (const int zahl : {1, 4, 6, 8})
    {
        Instrument instrument = vorlage;
        instrument.schichten.resize(static_cast<std::size_t>(zahl), vorlage.schichten[0]);
        for (int i = 0; i < zahl; ++i) instrument.schichten[static_cast<std::size_t>(i)].versatzHalbtoene = versatz[i];
        double bester = 1.0e30;
        float spitze = 0.0f;
        for (int lauf = 1; lauf <= 3; ++lauf)
        {
            const double davor = taktprobeMs();
            const auto start = std::chrono::steady_clock::now();
            const Klang klang = rendereTon(instrument, 60, 1000.0, kRate);  // zwei Sekunden Ton
            const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
            const double danach = taktprobeMs();
            const double sekunden = static_cast<double>(klang.frames()) / kRate;
            bester = std::min(bester, ms / sekunden);
            for (const float wert : klang.daten) spitze = std::max(spitze, std::abs(wert));
            MESSAGE(zahl << " Schichten, Lauf " << lauf << ": " << ms << " ms fuer " << sekunden << " s Ton = " << ms / sekunden
                         << " ms je Sekunde; Taktprobe davor " << davor << " ms, danach " << danach << " ms");
        }
        if (zahl == 1) eine = bester;
        // ein Ton wird vorab gerechnet, nicht im Ton-Thread: er darf dauern, aber nicht ewig
        CHECK(bester < 1000.0);
        // die Schichten werden addiert: mit jeder waechst die Spitze, wenn keiner die Level zuruecknimmt
        MESSAGE(zahl << " Schichten: " << bester << " ms je Sekunde Ton, das " << bester / eine
                     << "-fache einer Schicht; Spitze des Tons " << spitze << " (alle Level auf 1)");
    }
}
