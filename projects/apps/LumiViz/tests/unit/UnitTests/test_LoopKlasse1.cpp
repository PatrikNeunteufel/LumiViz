/**
 ****************************************************************************************
 * @file   test_LoopKlasse1.cpp
 * @brief  Loop, Stufe 34i: die mitgelieferten Instrumente der Klasse 1 ohne Fluegel — `upright_piano`,
 *         `honky_tonk_piano`, `electric_grand_piano` (Kern `hammerString`) und `clavinet_modal` (Kern `struckString`)
 *         (docs/audio/Instrumente_IC_Konzept.md 34i-4, KL1 a, KL2 a, KL4 a)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Liest vom Arbeitsordner aus — die Test-Exe aus dem Projekt-Root starten.
 *
 * - **Schreiben** (Muster `test_LoopStab.cpp`): `LUMIVIZ_KLASSE1_INSTRUMENTE_SCHREIBEN=1` schreibt die vier
 *   ausgelieferten Vorlagen als `asset/loop/instruments/<name>.lvinst` mit dem Schreiber des Programms. Danach
 *   `InstrumentTuneTool asset/loop/instruments/clavinet_modal.lvinst --reference` (der Bezug des Clavinets; der Kern
 *   des Fluegels hat keinen) und `python asset/loop/instrument_katalog.py`.
 * - **Waechter** (ohne zu stimmen): jede Datei lesbar, ihr Kern, ihre Werte die der Vorlage (bis auf den Bezug), die
 *   Tastatur nach IC `tastaturUpright`/`tastaturCp70`, ungemessen; der Ton aus den Werten der Datei Bit fuer Bit gleich
 *   dem IC-Abdruck (`werte/klasse1_ic.txt`), beim Clavinet mit dem Bezug der Datei gleich dem aus IC `normiere`.
 ****************************************************************************************
 */

#include <doctest.h>

#include "AudioCore/Lautheit.hpp"
#include "InstrumentCore/Pruefsumme.hpp"
#include "InstrumentModels/Kerne.hpp"
#include "InstrumentModels/hammer/HammerKern.hpp"
#include "InstrumentModels/saite/SaiteKern.hpp"
#include "loop/Aufnahme.hpp"
#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"
#include "HoerlisteAblage.hpp"
#include "loop/Tastatur.hpp"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <span>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace lumi::loop;
namespace kern = lumi::kern;
namespace hammer = lumi::kern::hammer;
namespace saite = lumi::kern::saite;

namespace
{

QString dateiVon(const std::string& name)
{
    return QStringLiteral("asset/loop/instruments/") + QString::fromStdString(name) + QStringLiteral(".lvinst");
}

/// Die Tastatur nach IC: das Upright (und Honky-tonk) links *Soft*, Mitte *Practice*, rechts *Sustain* (F-2 a); der
/// CP-70 nur *Sustain*; das Clavinet ohne Pedal (das Garn daempft, IC `daempfer = false`).
Tastatur tastaturVon(const std::string& name)
{
    Tastatur t;
    if (name == "upright_piano" || name == "honky_tonk_piano")
    {
        t.links = PedalArt::Leise;
        t.mitte = PedalArt::Moderator;
        t.rechts = PedalArt::Sustain;
    }
    else if (name == "electric_grand_piano")
    {
        t.links = PedalArt::Keins;
        t.mitte = PedalArt::Keins;
        t.rechts = PedalArt::Sustain;
    }
    else
    {
        t.links = PedalArt::Keins;
        t.mitte = PedalArt::Keins;
        t.rechts = PedalArt::Keins;
    }
    richteTastatur(t);
    return t;
}

/// Das Instrument einer Vorlage, wie es das Programm schreibt.
Instrument instrumentVon(const std::string& titel, std::string_view kernName, const kern::Werte& werte, const std::string& name)
{
    Instrument i;
    i.titel = QString::fromStdString(titel);
    Schicht s;
    s.name = QString::fromStdString(titel);
    s.kernName = QString::fromUtf8(kernName.data(), static_cast<qsizetype>(kernName.size()));
    s.kernWerte = werte;  // mit den Quellen der Vorlage (QM1 a)
    std::vector<std::string> fehler;
    richteSchicht(s, &fehler);  // der Bezug des Clavinets fehlt noch — den setzt das Werkzeug
    i.schichten[0] = s;
    i.tastatur = tastaturVon(name);
    return i;
}

std::map<std::string, std::string> icAbdruecke()
{
    std::map<std::string, std::string> r;
    std::ifstream f(std::filesystem::path(__FILE__).parent_path() / "werte" / "klasse1_ic.txt");
    std::string zeile;
    while (std::getline(f, zeile))
    {
        std::istringstream z(zeile);
        std::string art, name;
        z >> art >> name;
        if (art == "abdruck") z >> r[name];
    }
    return r;
}

/// Der Bezug wie IC `SaitenVorgaben.cpp` `normiere` (siehe test_InstrumentModelsKlasse1.cpp).
double icBezug(saite::SaitenSpiel s)
{
    s.ausgangBezug = 1.0;
    saite::SaitenAnschlag a;
    a.note = (s.noteMin + s.noteMax) / 2;
    a.hz = 440.0 * std::exp2((a.note - 69) / 12.0);
    a.stufe = 0;
    a.halten = a.rate / 2;
    std::vector<float> ton;
    saite::rechneSaitenTon(s, a, ton, a.rate / 2);
    double sp = 0.0;
    for (const float x : ton) sp = std::max(sp, static_cast<double>(std::abs(x)));
    return sp > 0.0 ? sp / 0.5 : 1.0;
}

Instrument liesDatei(const std::string& name)
{
    QFile datei(dateiVon(name));
    Instrument ist;
    REQUIRE_MESSAGE(datei.open(QIODevice::ReadOnly), name << " nicht gefunden — LUMIVIZ_KLASSE1_INSTRUMENTE_SCHREIBEN=1, dann InstrumentTuneTool");
    REQUIRE_MESSAGE(ausJson(QJsonDocument::fromJson(datei.readAll()).object(), ist),
                    name << " nicht lesbar (ohne Bezug?) — InstrumentTuneTool <datei> --reference");
    return ist;
}

}  // namespace

TEST_CASE("Loop Klasse1 34i die mitgelieferten Instrumente der Klasse 1 tragen ihre Vorlage, der Ton wie IC")
{
    if (qEnvironmentVariableIntValue("LUMIVIZ_KLASSE1_INSTRUMENTE_SCHREIBEN") == 1)
    {
        const auto schreib = [](const std::string& name, const Instrument& i) {
            QFile aus(dateiVon(name));
            REQUIRE_MESSAGE(aus.open(QIODevice::WriteOnly | QIODevice::Truncate), "asset/loop/instruments nicht gefunden — aus dem Projekt-Root starten");
            REQUIRE(aus.write(QJsonDocument(alsJson(i)).toJson(QJsonDocument::Indented)) > 0);
            MESSAGE(name << ".lvinst geschrieben — danach InstrumentTuneTool (Clavinet --reference), dann instrument_katalog.py");
        };
        for (const hammer::Vorlage& v : hammer::vorlagen())
            if (v.ausgeliefert) schreib(v.name, instrumentVon(v.titel, hammer::kKernName, hammer::werteVon(v), v.name));
        for (const saite::Vorlage& v : saite::vorlagen())
            if (v.ausgeliefert) schreib(v.name, instrumentVon(v.titel, saite::kKernName, saite::werteVon(v), v.name));
    }

    const std::map<std::string, std::string> ic = icAbdruecke();
    REQUIRE_MESSAGE(ic.size() == 12, "werte/klasse1_ic.txt fehlt");
    int geprueft = 0;

    // die drei auf dem Kern des Fluegels: Werte wie die Vorlage, der Ton an Taste 60 Bit fuer Bit wie IC
    const std::map<std::string, std::string> hammerNamen = {
        {"upright_piano", "klasse1_Upright_m60_a-12"}, {"honky_tonk_piano", "klasse1_Honkytonk_m60_a-12"}, {"electric_grand_piano", "klasse1_CP70_m60_a-12"}};
    const kern::Parametertafel& htafel = hammer::kernTyp().tafel;
    for (const hammer::Vorlage& v : hammer::vorlagen())
    {
        CAPTURE(v.name);
        const Instrument ist = liesDatei(v.name);
        REQUIRE(ist.schichten.size() == 1);
        const Schicht& s = ist.schichten[0];
        REQUIRE(istKernSchicht(s));
        CHECK(s.kernName == QStringLiteral("hammerString"));
        REQUIRE(ist.tastatur.has_value());
        CHECK(*ist.tastatur == tastaturVon(v.name));
        CHECK(kern::ungemessen(htafel, s.kernWerte));
        kern::Werte soll = hammer::werteVon(v);
        REQUIRE(kern::richteWerte(htafel, soll).empty());
        for (const kern::ParameterAngabe& a : htafel.werte)
        {
            CAPTURE(a.name);
            const kern::Wert* w = s.kernWerte.finde(a.name);
            REQUIRE(w != nullptr);
            CHECK_MESSAGE(*w == *soll.finde(a.name), "veraltet — LUMIVIZ_KLASSE1_INSTRUMENTE_SCHREIBEN=1, instrument_katalog.py");
        }
        CHECK(kern::tastenumfang(htafel, s.kernWerte) == v.umfang);
        hammer::HammerAnschlag a;
        a.taste = 60;
        a.hz = 440.0 * std::exp2((60 - 69) / 12.0);
        a.stufe = -12;
        a.halten = 24000;
        a.gedaempft = 60 < hammer::pedalwerteAus(s.kernWerte).ungedaempftAb;
        std::vector<float> ton;
        hammer::rechneHammerTon(hammer::saiteAus(s.kernWerte), a, ton, 48000);
        CHECK(kern::abdruck(ton) == ic.at(hammerNamen.at(v.name)));
        ++geprueft;
    }

    // das Clavinet auf `struckString`: der Bezug der Datei gleich IC `normiere`, der Ton wie IC
    const kern::Parametertafel& stafel = saite::kernTyp().tafel;
    for (const saite::Vorlage& v : saite::vorlagen())
    {
        CAPTURE(v.name);
        if (!v.ausgeliefert)
        {
            CHECK_FALSE(QFile::exists(dateiVon(v.name)));  // KL1 a: Hackbrett und Cimbalom gehen nicht mit
            continue;
        }
        const Instrument ist = liesDatei(v.name);
        REQUIRE(ist.schichten.size() == 1);
        const Schicht& s = ist.schichten[0];
        REQUIRE(istKernSchicht(s));
        CHECK(s.kernName == QStringLiteral("struckString"));
        REQUIRE(ist.tastatur.has_value());
        CHECK(*ist.tastatur == tastaturVon(v.name));
        CHECK(kern::ungemessen(stafel, s.kernWerte));
        kern::Werte soll = saite::werteVon(v);
        (void)kern::richteWerte(stafel, soll);
        for (const kern::ParameterAngabe& a : stafel.werte)
        {
            if (a.name == stafel.bezugWert) continue;
            CAPTURE(a.name);
            const kern::Wert* w = s.kernWerte.finde(a.name);
            REQUIRE(w != nullptr);
            CHECK_MESSAGE(*w == *soll.finde(a.name), "veraltet — LUMIVIZ_KLASSE1_INSTRUMENTE_SCHREIBEN=1, InstrumentTuneTool, instrument_katalog.py");
        }
        const saite::SaitenSpiel spiel = saite::spielAus(s.kernWerte);
        CHECK(spiel.ausgangBezug == icBezug(v.spiel));
        saite::SaitenAnschlag a;
        a.note = 60;
        a.hz = 440.0 * std::exp2((60 - 69) / 12.0);
        a.stufe = -12;
        a.halten = 24000;
        std::vector<float> ton;
        saite::rechneSaitenTon(spiel, a, ton, 48000);
        CHECK(kern::abdruck(ton) == ic.at("klasse1_Clavinet_m60_a-12"));
        ++geprueft;
    }
    CHECK(geprueft == 4);
}

TEST_CASE("Loop Klasse1 34i Hoerproben (LUMIVIZ_34I_WAV) — Liste fuer HoerTest_Helper: je Instrument drei Lagen und drei Staerken, Paare")
{
    // Werkzeug (Teilplan 34i-6, Muster `Loop Stab 34g Hoerproben`): schreibt die Liste im Format von HoerTest_Helper
    // (dessen Konzept §11, §12, Schema 1) in den Ordner der Variable — der Hoerort `…\cmake\samples\Hoerproben\LumiViz\klasse1_34i` (AU3, S182; Liste
    // und Abdruecke danach auch versioniert unter `hoertest/listen/klasse1_34i`, AU3-1 a):
    // `liste.json`, `klaenge/` (gitignored) und `klaenge.sha256`. Gerechnet wie im Programm (rendereTon aus der Datei),
    // 2 s gehalten, jeder Klang auf einen gemeinsamen Pegel (hoechstens −23 LUFS, ohne Spitze ueber 0,89). Paare:
    // `clavinet_modal` gegen `clavinet` (IK2-8 a: das bessere behaelt den Namen), das Upright gegen `grand_piano`,
    // Practice am Upright (Spur `practice`), die Tonabnehmer des Clavinets (Steg gegen Mitte). Ohne die Variable meldet er
    // das und prueft nichts. Pruefen: `Hoertest.exe --liste-pruefen <ordner>/liste.json`.
    const QString ordner = qEnvironmentVariable("LUMIVIZ_34I_WAV");
    if (ordner.isEmpty())
    {
        MESSAGE("LUMIVIZ_34I_WAV nicht gesetzt — keine Hoerproben");
        return;
    }
    const QDir dir(ordner);
    REQUIRE(QDir().mkpath(dir.filePath(QStringLiteral("klaenge"))));
    constexpr int kRate = 48000;
    constexpr int kHaltenMs = 2000;
    constexpr double kZielHoechstens = -23.0;
    constexpr double kSpitzeHoechstens = 0.89;
    const auto lies = [](const QString& name) {
        Instrument i = liesDatei(name.toStdString());
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
    QStringList summen, ids, stille;
    struct Gerechnet
    {
        QString datei;
        std::vector<float> daten;
        double lufs;  ///< vorher; −300 = still
        float spitze;
    };
    std::vector<Gerechnet> gerechnet;
    const auto ablegen = [&](const Instrument& inst, const QString& instrument, int hoehe, const Staerke& st, const QString& fassung,
                             const QString& zusatz = {}, const std::vector<KernSpurWert>& spuren = {}) {
        const QString id = inst.name + QStringLiteral("_") + QString::number(hoehe) + QStringLiteral("_") + QString::fromLatin1(st.name) + zusatz;
        if (ids.contains(id)) return id;
        ids << id;
        TonAngabe t;
        t.instrument = inst.name;
        t.hoehe = hoehe;
        t.dauerMs = kHaltenMs;
        t.anschlagStufe = st.stufe;
        t.kernSpuren = spuren;
        Klang k = rendereTon(inst, t, kRate);
        const lumi::audio::Lautheit vorher = lumi::audio::messeLautheit(k.daten, kRate);
        if (vorher.stille) stille << id;
        const QString datei = QStringLiteral("klaenge/") + id + QStringLiteral(".wav");
        float spitze = 0.0f;
        for (const float x : k.daten) spitze = std::max(spitze, std::abs(x));
        gerechnet.push_back(Gerechnet{datei, std::move(k.daten), vorher.stille ? -300.0 : vorher.lufs, spitze});
        const QJsonObject achsen{{QStringLiteral("instrument"), instrument},
                                 {QStringLiteral("taste"), hoehe},
                                 {QStringLiteral("ton"), tonName(hoehe)},
                                 {QStringLiteral("staerke"), QString::fromLatin1(st.name)}};
        klaenge.append(QJsonObject{{QStringLiteral("id"), id}, {QStringLiteral("datei"), datei}, {QStringLiteral("fassung"), fassung},
                                   {QStringLiteral("achsen"), achsen}});
        messwerte.append(QJsonObject{{QStringLiteral("achsen"), achsen},
                                     {QStringLiteral("fassung"), fassung},
                                     {QStringLiteral("werte"), QJsonArray{QJsonObject{{QStringLiteral("name"), QStringLiteral("lufs_vorher")},
                                                                                      {QStringLiteral("wert"), vorher.stille ? -300.0 : std::round(vorher.lufs * 100.0) / 100.0},
                                                                                      {QStringLiteral("einheit"), QStringLiteral("LUFS")}}}}});
        return id;
    };
    const auto punktId = [&] { return QStringLiteral("p%1").arg(punkte.size() + 1, 3, 10, QLatin1Char('0')); };
    const auto paar = [&](const QString& a, const QString& b, const QString& frage) {
        punkte.append(QJsonObject{{QStringLiteral("id"), punktId()},
                                  {QStringLiteral("art"), QStringLiteral("paar")},
                                  {QStringLiteral("klaenge"), QJsonArray{a, b}},
                                  {QStringLiteral("vorzug"), frage}});
    };
    // je Instrument drei Lagen und drei Staerken
    std::map<QString, std::pair<int, int>> umfang;
    for (const hammer::Vorlage& v : hammer::vorlagen()) umfang[QString::fromStdString(v.name)] = v.umfang;
    for (const saite::Vorlage& v : saite::vorlagen())
        if (v.ausgeliefert) umfang[QString::fromStdString(v.name)] = {v.spiel.noteMin, v.spiel.noteMax};
    for (const QString name : {QStringLiteral("upright_piano"), QStringLiteral("honky_tonk_piano"), QStringLiteral("electric_grand_piano"),
                               QStringLiteral("clavinet_modal")})
    {
        const Instrument inst = lies(name);
        const auto [von, bis] = umfang.at(name);
        for (const int hoehe : {von + 5, (von + bis) / 2, bis - 5})
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
    // blind: das Clavinet aus IC gegen das heutige (IK2-8 a) und das Upright gegen den Fluegel
    int paare = 0;
    const Instrument modal = lies(QStringLiteral("clavinet_modal"));
    const Instrument waveguide = lies(QStringLiteral("clavinet"));
    for (const int hoehe : {41, 58, 76})
    {
        paar(ablegen(modal, QStringLiteral("Clavinet"), hoehe, staerken[1], QStringLiteral("ic")),
             ablegen(waveguide, QStringLiteral("Clavinet"), hoehe, staerken[1], QStringLiteral("waveguide")),
             QStringLiteral("Welcher klingt mehr nach einem Clavinet?"));
        ++paare;
    }
    const Instrument upright = lies(QStringLiteral("upright_piano"));
    const Instrument fluegel = lies(QStringLiteral("grand_piano"));
    for (const int hoehe : {33, 60, 84})
    {
        paar(ablegen(upright, QStringLiteral("Piano"), hoehe, staerken[1], QStringLiteral("ic")),
             ablegen(fluegel, QStringLiteral("Piano"), hoehe, staerken[1], QStringLiteral("grand")),
             QStringLiteral("Welcher klingt mehr nach einem Upright (Klavier) als nach einem Fluegel?"));
        ++paare;
    }
    // Practice am Upright (der Filz des Moderators) und die Tonabnehmer des Clavinets
    paar(ablegen(upright, upright.titel, 60, staerken[1], QStringLiteral("ic")),
         ablegen(upright, upright.titel, 60, staerken[1], QStringLiteral("practice"), QStringLiteral("_practice"),
                 {KernSpurWert{QStringLiteral("practice"), 1.0}}),
         QStringLiteral("Welcher klingt nach dem Moderator (Practice: gedaempft, dunkler)?"));
    ++paare;
    paar(ablegen(modal, modal.titel, 41, staerken[1], QStringLiteral("steg"), QStringLiteral("_steg"),
                 {KernSpurWert{QStringLiteral("pickupSelection"), 1.0}}),
         ablegen(modal, modal.titel, 41, staerken[1], QStringLiteral("mitte"), QStringLiteral("_mitte"),
                 {KernSpurWert{QStringLiteral("pickupSelection"), 2.0}}),
         QStringLiteral("Welcher klingt heller (Tonabnehmer am Steg)?"));
    ++paare;

    // der gemeinsame Pegel, dann jeder Klang darauf (wie 34g)
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
        {QStringLiteral("titel"), QStringLiteral("LumiViz 34i: Klasse 1 ohne Fluegel — Upright, Honky-tonk, CP-70, Clavinet")},
        {QStringLiteral("ziel"), QStringLiteral("Erstes Hoeren der vier Instrumente aus IC (ungemessen, Z2 a); welches Clavinet den Namen behaelt "
                                                "(IK2-8 a); ob das Upright sich vom Fluegel abhebt")},
        {QStringLiteral("katalog"), QJsonValue::Null},
        {QStringLiteral("kontrolle"), QStringLiteral("moderat")},
        {QStringLiteral("pegel"), QJsonObject{{QStringLiteral("lufs"), ziel}, {QStringLiteral("toleranz_lu"), 1}}},
        {QStringLiteral("dauer_s"), QJsonValue::Null},
        {QStringLiteral("gruppe_nach"), QJsonArray{QStringLiteral("instrument")}},
        {QStringLiteral("fassungen"),
         QJsonObject{{QStringLiteral("ic"), QStringLiteral("die Vorgabe aus InstrumentsCalculation (LumiViz Stufe 34i), ungemessen")},
                     {QStringLiteral("waveguide"), QStringLiteral("das heutige Clavinet (Waveguide-Saite mit zwei Tonabnehmern)")},
                     {QStringLiteral("grand"), QStringLiteral("der Fluegel grand_piano (gemessen an Salamander, mit Boden)")},
                     {QStringLiteral("practice"), QStringLiteral("das Upright mit dem Moderator (Spur practice)")},
                     {QStringLiteral("steg"), QStringLiteral("Clavinet modal, nur der Tonabnehmer am Steg")},
                     {QStringLiteral("mitte"), QStringLiteral("Clavinet modal, nur der mittlere Tonabnehmer")}}},
        {QStringLiteral("klaenge"), klaenge},
        {QStringLiteral("messwerte"), messwerte},
        {QStringLiteral("punkte"), punkte}};
    QFile aus(dir.filePath(QStringLiteral("liste.json")));
    REQUIRE(aus.open(QIODevice::WriteOnly | QIODevice::Truncate));
    REQUIRE(aus.write(QJsonDocument(liste).toJson(QJsonDocument::Indented)) > 0);
    aus.close();
    QFile summenDatei(dir.filePath(QStringLiteral("klaenge.sha256")));
    REQUIRE(summenDatei.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QByteArray text = "# Abdruck jedes Klangs (SHA-256 der WAV-Datei). Geschrieben mit: LUMIVIZ_34I_WAV=<dieser Ordner> LumiViz.UnitTests "
                      "--test-case=\"Loop Klasse1 34i Hoerproben*\"\n";
    for (const QString& z : summen) text += z.toUtf8() + '\n';
    REQUIRE(summenDatei.write(text) > 0);
    summenDatei.close();
    hoerliste::legeInsRepo(dir);  // AU3-1 a: Liste und Abdruecke auch versioniert unter hoertest/listen/<liste>
    MESSAGE(klaenge.size() << " Klaenge, " << punkte.size() << " Punkte (davon " << paare << " Paare) unter " << ordner.toStdString());
    MESSAGE("Pegel " << ziel << " LUFS; " << stille.size() << " still (unter −70 LUFS, ungeaendert): " << stille.join(QStringLiteral(", ")).toStdString());
    CHECK(paare == 8);
    CHECK(punkte.size() == 36 + 8);
}
