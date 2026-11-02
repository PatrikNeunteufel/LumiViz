/**
 ****************************************************************************************
 * @file   test_LoopSteuersignal.cpp
 * @brief  Loop: das Steuersignal aus einem Kanal und der Baustein Sidechain
 *         (docs/audio/Loop_Konzept.md §4p, Stufe 15l; Entscheide Patrik WV5 a, ST1 a, ST2 a)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Ohne Ton und ohne Engine: der kleine Loop hat einen Kanal Pad (ein gehaltener Ton 0,5 ueber
 * das ganze Muster, mit einem Sidechain) und einen Kanal Kick (0,8 fuer 100 ms am Anfang).
 * Die Faelle pruefen die Kennung der Kanaele und die Datei (Fassung 17), den Kreis, die
 * Rechnung im Klangerzeuger gegen eine Huelle von Hand, Freeze und Abdruck, den Austausch
 * und das Fenster *Clip effects*. `LUMIVIZ_SIDECHAIN_BILD=<datei.png>` legt ein Bild des
 * Fensters mit dem Baustein Sidechain ab.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Austausch.hpp"
#include "loop/ClipEffektFenster.hpp"
#include "loop/Freeze.hpp"
#include "loop/Klangerzeuger.hpp"
#include "loop/Loop.hpp"
#include "loop/Rechenkern.hpp"
#include "loop/SignalWeg.hpp"
#include "loop/Wellenform.hpp"

#include <QApplication>
#include <QComboBox>
#include <QImage>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QSlider>
#include <QStackedWidget>

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

using namespace lumi::loop;

namespace
{

constexpr int kRate = 44100;
constexpr int kDurchlauf = 88200;  ///< vier Takte zu einem Schlag bei 120 BPM
constexpr int kKickFrames = 4410;  ///< 100 ms
constexpr float kKick = 0.8f;
constexpr float kPad = 0.5f;

Muster drumMuster(const QString& name, const QString& klang)
{
    Muster m;
    m.name = name;
    m.spuren = {{name.left(1), klang, 1.0, 0.0, false}};
    m.zeilen = {{1.0}};
    return m;
}

/// Pad (Kanal 0) mit einem Sidechain, der Kick (Kanal 1) nennt; Lautstaerke des Loops 1.
Loop steuerLoop(double tiefeDb = 12.0)
{
    Loop loop;
    loop.laut = 1.0;
    loop.schlaegeJeTakt = 1;
    loop.muster = {drumMuster(QStringLiteral("Pad"), QStringLiteral("pad.wav")),
                   drumMuster(QStringLiteral("Kick"), QStringLiteral("kick.wav")),
                   drumMuster(QStringLiteral("Hat"), QStringLiteral("hat.wav"))};
    loop.kanaele = {{QStringLiteral("Pad"), true, {LoopClip{QStringLiteral("Pad"), 0, 1}}},
                    {QStringLiteral("Kick"), true, {LoopClip{QStringLiteral("Kick"), 0, 1}}}};
    loop.richte();
    loop.kanaele[0].clips[0].effekte.reihe = {ClipSidechain{true, loop.kanaele[1].kennung, tiefeDb, 0.1, 10.0}};
    loop.richte();
    return loop;
}

ClipSidechain& sidechainIn(Loop& loop, int kanal)
{
    return std::get<ClipSidechain>(loop.kanaele[static_cast<std::size_t>(kanal)].clips[0].effekte.reihe[0]);
}

std::shared_ptr<const Klaenge> klaengeFuer(const QStringList& namen)
{
    Klaenge alle;
    for (const QString& name : namen)
    {
        Klang klang;
        if (name == QStringLiteral("kick.wav"))
            klang.daten.assign(2 * kKickFrames, kKick);
        else if (name == QStringLiteral("pad.wav"))
            klang.daten.assign(2 * kDurchlauf, kPad);
        else
            klang.daten.assign(2 * 1000, 0.3f);
        alle.push_back(std::make_shared<const Klang>(std::move(klang)));
    }
    return std::make_shared<const Klaenge>(std::move(alle));
}

/// Einen Durchlauf aus dem Klangerzeuger, in Bloecken wie der Mixer.
std::vector<float> spiele(const Loop& loop, int block = 441, int frames = kDurchlauf)
{
    Klangerzeuger erzeuger;
    erzeuger.setzeProgramm(baueProgramm(loop, kRate, klaengeFuer(loop.klangNamen())));
    std::vector<float> werte(2 * static_cast<std::size_t>(frames));
    for (int stelle = 0; stelle < frames; stelle += block)
        erzeuger.render(werte.data() + 2 * static_cast<std::ptrdiff_t>(stelle), std::min(block, frames - stelle));
    return werte;
}

/// Der Faktor des Sidechains je Abtastwert, von Hand: die Huelle folgt der Spitze des Kicks
/// mit Attack und Release (Tiefpaesse erster Ordnung), der Ton faellt um Tiefe mal Huelle dB.
std::vector<double> faktorVonHand(double tiefeDb, double attackMs, double releaseMs, int frames = kDurchlauf)
{
    const double a = 1.0 - std::exp(-1000.0 / (attackMs * kRate));
    const double r = 1.0 - std::exp(-1000.0 / (releaseMs * kRate));
    std::vector<double> faktor(static_cast<std::size_t>(frames));
    double huelle = 0.0;
    for (int f = 0; f < frames; ++f)
    {
        const double spitze = f < kKickFrames ? kKick : 0.0;
        huelle += (spitze > huelle ? a : r) * (spitze - huelle);
        faktor[static_cast<std::size_t>(f)] = std::pow(10.0, -tiefeDb * std::min(huelle, 1.0) / 20.0);
    }
    return faktor;
}

double groessterAbstand(const std::vector<float>& a, const std::vector<float>& b, std::size_t bis)
{
    double abstand = 0.0;
    for (std::size_t i = 0; i < std::min({bis, a.size(), b.size()}); ++i)
        abstand = std::max(abstand, std::abs(static_cast<double>(a[i]) - b[i]));
    return abstand;
}

}  // namespace

TEST_CASE("Loop Steuersignal 15l: Kennung je Kanal, Datei Fassung 17, der Baustein Sidechain")
{
    Loop loop = steuerLoop();
    CHECK(loop.kanaele[0].kennung == 1);
    CHECK(loop.kanaele[1].kennung == 2);
    CHECK(kanalMitKennung(loop, 2) == 1);
    CHECK(kanalMitKennung(loop, 0) == -1);
    CHECK(kanalMitKennung(loop, 9) == -1);
    CHECK(steuerKennung(loop.kanaele[0].clips[0].effekte.reihe[0]) == 2);
    CHECK(steuerKennung(ClipBaustein(ClipHall{})) == 0);

    SUBCASE("die Datei: id je Kanal, sidechain mit allen Feldern, wieder gelesen gleich")
    {
        const QJsonObject json = alsJson(loop);
        CHECK(json.value(QStringLiteral("version")).toInt() == 17);
        const QJsonArray kanaele = json.value(QStringLiteral("channels")).toArray();
        CHECK(kanaele[0].toObject().value(QStringLiteral("id")).toInt() == 1);
        CHECK(kanaele[1].toObject().value(QStringLiteral("id")).toInt() == 2);
        const QJsonObject baustein =
            kanaele[0].toObject().value(QStringLiteral("clips")).toArray()[0].toObject().value(QStringLiteral("path")).toArray()[0].toObject();
        CHECK(baustein.value(QStringLiteral("type")).toString() == QStringLiteral("sidechain"));
        CHECK(baustein.value(QStringLiteral("on")).toBool());
        CHECK(baustein.value(QStringLiteral("channel")).toInt() == 2);
        CHECK(baustein.value(QStringLiteral("depth")).toDouble() == 12.0);
        CHECK(baustein.value(QStringLiteral("attack")).toDouble() == 0.1);
        CHECK(baustein.value(QStringLiteral("release")).toDouble() == 10.0);
        Loop gelesen;
        REQUIRE(ausJson(json, gelesen));
        CHECK(gelesen == loop);
    }
    SUBCASE("Fassung 16 ohne id: richte vergibt die Kennungen in der Folge der Kanaele")
    {
        QJsonObject json = alsJson(loop);
        json[QStringLiteral("version")] = 16;
        QJsonArray kanaele = json.value(QStringLiteral("channels")).toArray();
        for (int i = 0; i < kanaele.size(); ++i)
        {
            QJsonObject k = kanaele[i].toObject();
            k.remove(QStringLiteral("id"));
            k[QStringLiteral("clips")] = QJsonArray{QJsonObject{{QStringLiteral("pattern"), k.value(QStringLiteral("name"))}}};
            kanaele[i] = k;
        }
        json[QStringLiteral("channels")] = kanaele;
        Loop gelesen;
        REQUIRE(ausJson(json, gelesen));
        CHECK(gelesen.kanaele[0].kennung == 1);
        CHECK(gelesen.kanaele[1].kennung == 2);
    }
    SUBCASE("die Grenzen: Tiefe, Attack, Release; eine Kennung unter 0 wird keine")
    {
        sidechainIn(loop, 0) = ClipSidechain{true, -3, 99.0, 0.0, 99999.0};
        loop.richte();
        const ClipSidechain& s = sidechainIn(loop, 0);
        CHECK(s.kanal == 0);
        CHECK(s.tiefeDb == kSidechainTiefeMaxDb);
        CHECK(s.attackMs == kSidechainAttackMinMs);
        CHECK(s.releaseMs == kSidechainReleaseMaxMs);
    }
    SUBCASE("Umbenennen und Verschieben aendern die Kennung nicht")
    {
        loop.kanaele[1].name = QStringLiteral("Bass drum");
        std::swap(loop.kanaele[0], loop.kanaele[1]);
        loop.richte();
        CHECK(loop.kanaele[0].kennung == 2);
        CHECK(loop.kanaele[1].kennung == 1);
        CHECK(steuerKanal(loop, 1, loop.kanaele[1].clips[0].effekte.reihe[0]) == 0);
    }
    SUBCASE("doppelte und fehlende Kennungen: die erste bleibt, die naechste liegt ueber jedem Verweis")
    {
        Kanal kopie = loop.kanaele[1];
        kopie.name = QStringLiteral("Kick 2");
        loop.kanaele.push_back(kopie);  // dieselbe Kennung 2
        Kanal ohne;
        ohne.name = QStringLiteral("Neu");
        loop.kanaele.push_back(ohne);
        sidechainIn(loop, 0).kanal = 7;  // ein Verweis auf einen geloeschten Kanal
        loop.richte();
        CHECK(loop.kanaele[1].kennung == 2);
        CHECK(loop.kanaele[2].kennung == 8);
        CHECK(loop.kanaele[3].kennung == 9);
        CHECK(freieKennung(loop) == 10);
    }
}

TEST_CASE("Loop Steuersignal 15l: ein Kreis gibt kein Signal und steht im Bericht")
{
    Loop loop = steuerLoop();
    const ClipBaustein padNenntKick = loop.kanaele[0].clips[0].effekte.reihe[0];
    CHECK(steuerKanal(loop, 0, padNenntKick) == 1);

    SUBCASE("der eigene Kanal ist ein Kreis")
    {
        sidechainIn(loop, 0).kanal = loop.kanaele[0].kennung;
        CHECK(steuerKanal(loop, 0, loop.kanaele[0].clips[0].effekte.reihe[0]) == -1);
        QStringList bericht;
        loop.richte(&bericht);
        CHECK(bericht.contains(QStringLiteral("Sidechain in a loop: Pad from Pad")));
    }
    SUBCASE("zwei Kanaele, die einander nennen — auch ueber einen ausgeschalteten Baustein")
    {
        loop.kanaele[1].clips[0].effekte.reihe = {ClipSidechain{false, loop.kanaele[0].kennung, 6.0, 5.0, 100.0}};
        CHECK(steuerKanal(loop, 0, padNenntKick) == -1);
        CHECK(steuerKanal(loop, 1, loop.kanaele[1].clips[0].effekte.reihe[0]) == -1);
        QStringList bericht;
        loop.richte(&bericht);
        CHECK(bericht.contains(QStringLiteral("Sidechain in a loop: Pad from Kick")));
        CHECK(bericht.contains(QStringLiteral("Sidechain in a loop: Kick from Pad")));
        // und es klingt wie ohne Sidechain
        CHECK(plane(loop, kRate).abgriffe.empty());
    }
    SUBCASE("ein Kanal, den es nicht gibt")
    {
        sidechainIn(loop, 0).kanal = 42;
        CHECK(steuerKanal(loop, 0, loop.kanaele[0].clips[0].effekte.reihe[0]) == -1);
        QStringList bericht;
        loop.richte(&bericht);
        CHECK(bericht.contains(QStringLiteral("Sidechain channel not found: 42 (channel Pad)")));
    }
}

TEST_CASE("Loop Steuersignal 15l: der Klangerzeuger senkt den Pad mit dem Kick — vor Fader und Schalter")
{
    const std::vector<double> faktor = faktorVonHand(12.0, 0.1, 10.0);

    SUBCASE("ohne wirksamen Sidechain Bit fuer Bit wie ohne Baustein")
    {
        Loop ohne = steuerLoop();
        ohne.kanaele[0].clips[0].effekte.reihe.clear();
        Loop aus = steuerLoop();
        sidechainIn(aus, 0).an = false;
        CHECK(plane(aus, kRate).abgriffe.empty());
        const std::vector<float> a = spiele(ohne);
        const std::vector<float> b = spiele(aus);
        // ohne eingeschalteten Baustein hat der Clip keinen Bus und der Kick keinen Abgriff
        CHECK(groessterAbstand(a, b, a.size()) == 0.0);
        CHECK(a[2 * 1000] == kKick + kPad);
        CHECK(a[2 * 10000] == kPad);
    }
    SUBCASE("der Kick ausgeschaltet: man hoert nur den Pad, gesenkt im Takt des Kicks")
    {
        Loop loop = steuerLoop();
        loop.kanaele[1].an = false;
        const Ablauf ablauf = plane(loop, kRate);
        REQUIRE(ablauf.abgriffe.size() == 1u);
        CHECK(ablauf.abgriffe[0].kanal == 1);
        CHECK(ablauf.abgriffe[0].faktor == 0.0f);  // er klingt nicht, er steuert
        REQUIRE(ablauf.busse.size() == 1u);
        CHECK(ablauf.busse[0].steuer == std::vector<int>{0});
        const std::vector<float> gespielt = spiele(loop);
        double abstand = 0.0;
        for (int f = 0; f < kDurchlauf; ++f)
            abstand = std::max(abstand, std::abs(gespielt[2 * static_cast<std::size_t>(f)] - kPad * faktor[static_cast<std::size_t>(f)]));
        MESSAGE("groesster Abstand zur Huelle von Hand: " << abstand);
        CHECK(abstand < 1.0e-5);
        // auf dem Plateau: Huelle 0,8, also 12 · 0,8 = 9,6 dB leiser
        CHECK(gespielt[2 * 3000] == doctest::Approx(kPad * std::pow(10.0, -9.6 / 20.0)).epsilon(1.0e-4));
        CHECK(gespielt[2 * 30000] == doctest::Approx(kPad).epsilon(1.0e-6));

        SUBCASE("der Fader des Kicks zaehlt nicht — abgegriffen vor dem Fader")
        {
            Loop leiser = loop;
            leiser.kanaele[1].gainDb = -20.0;
            CHECK(spiele(leiser) == gespielt);
        }
        SUBCASE("in anderen Bloecken gerechnet: dieselben Werte")
        {
            CHECK(spiele(loop, 4096) == gespielt);
            CHECK(spiele(loop, 37) == gespielt);
        }
        SUBCASE("der Kick angeschaltet, mit Fader: dazu der Kick, mal Fader — der Pad bleibt gleich gesenkt")
        {
            Loop an = loop;
            an.kanaele[1].an = true;
            an.kanaele[1].gainDb = -6.0;
            const std::vector<float> mitKick = spiele(an);
            const double fader = std::pow(10.0, -6.0 / 20.0);
            CHECK(mitKick[2 * 1000] - gespielt[2 * 1000] == doctest::Approx(kKick * fader).epsilon(1.0e-5));
            CHECK(mitKick[2 * 30000] == gespielt[2 * 30000]);
        }
    }
    SUBCASE("die Folge: der genannte Kanal rechnet zuerst, auch wenn er weiter unten steht und selbst gesteuert ist")
    {
        Loop loop = steuerLoop();
        // Hat (Kanal 2, aus) steuert Kick; Kick (Kanal 1) steuert Pad (Kanal 0)
        loop.kanaele.push_back({QStringLiteral("Hat"), false, {LoopClip{QStringLiteral("Hat"), 0, 1}}});
        loop.richte();
        loop.kanaele[1].clips[0].effekte.reihe = {ClipSidechain{true, loop.kanaele[2].kennung, 0.0, 5.0, 100.0}};
        loop.richte();
        loop.kanaele[1].an = false;
        const Ablauf ablauf = plane(loop, kRate);
        REQUIRE(ablauf.abgriffe.size() == 2u);
        CHECK(ablauf.abgriffe[0].kanal == 2);  // Hat zuerst — Kick braucht ihn
        CHECK(ablauf.abgriffe[1].kanal == 1);
        CHECK(ablauf.abgriffe[1].busse.size() == 1u);
        // Tiefe 0: der Kick bleibt, wie er ist — der Pad wie im Fall oben
        const std::vector<float> gespielt = spiele(loop);
        CHECK(gespielt[2 * 3000] == doctest::Approx(kPad * std::pow(10.0, -9.6 / 20.0)).epsilon(1.0e-4));
    }
}

TEST_CASE("Loop Steuersignal 15l: Freeze nimmt den genannten Kanal mit, der Abdruck folgt ihm")
{
    Loop loop = steuerLoop();
    loop.kanaele[1].an = false;  // live: nur der Pad klingt
    const ClipOrt pad{0, 0};

    SUBCASE("die Fassung: der genannte Kanal ausgeschaltet, ohne Fader; gerechnet wie live")
    {
        const std::optional<Loop> fassung = clipFassung(loop, pad);
        REQUIRE(fassung.has_value());
        REQUIRE(fassung->kanaele.size() == 3u);  // Pad, Kick, Platz fuer den Nachklang
        CHECK(fassung->kanaele[1].name == QStringLiteral("Kick"));
        CHECK_FALSE(fassung->kanaele[1].an);
        CHECK(fassung->findeMuster(QStringLiteral("Kick")) != nullptr);
        const auto klaenge = klaengeFuer(fassung->klangNamen());
        const std::optional<Wellenform> welle = rechneClip(loop, pad, kRate, klaenge, {});
        REQUIRE(welle.has_value());
        const std::vector<float> live = spiele(loop);
        CHECK(groessterAbstand(welle->klang.daten, live, 2 * static_cast<std::size_t>(kDurchlauf)) == 0.0);
    }
    SUBCASE("der Abdruck")
    {
        const QByteArray grund = clipAbdruck(loop, pad, kRate, {});
        REQUIRE_FALSE(grund.isEmpty());
        const auto abdruck = [&](const Loop& anders) { return clipAbdruck(anders, pad, kRate, {}); };

        Loop umbenannt = loop;
        umbenannt.kanaele[1].name = QStringLiteral("Bass drum");
        umbenannt.muster[1].name = QStringLiteral("Bum");
        umbenannt.kanaele[1].clips[0].muster = QStringLiteral("Bum");
        umbenannt.richte();
        CHECK(abdruck(umbenannt) == grund);  // Namen klingen nicht
        Loop schalter = loop;
        schalter.kanaele[1].an = true;
        schalter.kanaele[1].gainDb = -9.0;
        CHECK(abdruck(schalter) == grund);  // Schalter und Fader des genannten Kanals auch nicht
        Loop kennung = loop;
        kennung.kanaele[1].kennung = 40;
        sidechainIn(kennung, 0).kanal = 40;
        CHECK(abdruck(kennung) == grund);  // die Kennung auch nicht

        Loop andererKick = loop;
        andererKick.muster[1].zeilen[0][8] = 1.0;  // ein zweiter Schlag
        CHECK(abdruck(andererKick) != grund);
        Loop tiefer = loop;
        sidechainIn(tiefer, 0).tiefeDb = 6.0;
        CHECK(abdruck(tiefer) != grund);
        // mit Steuersignal klingt, wo der Clip zum Kick liegt
        Loop spaeter = loop;
        spaeter.kanaele[0].clips[0].startTakt = 4;
        spaeter.richte();
        CHECK(clipAbdruck(spaeter, ClipOrt{0, 4}, kRate, {}) != grund);
        // ohne wirksamen Sidechain nicht (der Clip allein auf Takt 0, wie bis 15k)
        Loop ohne = loop;
        sidechainIn(ohne, 0).an = false;
        Loop ohneSpaeter = spaeter;
        sidechainIn(ohneSpaeter, 0).an = false;
        CHECK(abdruck(ohne) == clipAbdruck(ohneSpaeter, ClipOrt{0, 4}, kRate, {}));
    }
}

TEST_CASE("Loop Steuersignal 15l: Austausch — ein Verweis aus der Datei hinaus wird keiner")
{
    const Loop loop = steuerLoop();

    SUBCASE(".lvclip in einen anderen Loop, der zufaellig dieselbe Kennung hat")
    {
        const std::optional<Loop> teil = clipDatei(loop, 0, 0);
        REQUIRE(teil.has_value());
        Loop ziel = steuerLoop();  // Kanal 1 hat Kennung 2 — wie Kick in der Quelle, aber ein anderer Loop
        const Einsetzung e = setzeEin(ziel, *teil, AustauschArt::Clip, -1, 8);
        CHECK(e.ohneSteuersignal == 1);
        REQUIRE(e.kanal >= 0);
        const LoopClip& clip = ziel.kanaele[static_cast<std::size_t>(e.kanal)].clips.back();
        CHECK(std::get<ClipSidechain>(clip.effekte.reihe[0]).kanal == 0);
    }
    SUBCASE(".lvtrack: der Kanal bekommt eine neue Kennung, ein Verweis auf sich selbst folgt ihm")
    {
        Loop quelle = loop;
        quelle.kanaele[1].clips[0].effekte.reihe = {ClipSidechain{true, quelle.kanaele[1].kennung, 6.0, 5.0, 100.0}};
        const std::optional<Loop> teil = spurDatei(quelle, 1);
        REQUIRE(teil.has_value());
        Loop ziel = loop;
        const Einsetzung e = setzeEin(ziel, *teil, AustauschArt::Spur);
        CHECK(e.ohneSteuersignal == 0);
        const Kanal& neu = ziel.kanaele[static_cast<std::size_t>(e.kanal)];
        CHECK(neu.kennung == 3);
        CHECK(std::get<ClipSidechain>(neu.clips[0].effekte.reihe[0]).kanal == 3);
        // der Pad nennt weiter den alten Kick
        CHECK(steuerKanal(ziel, 0, ziel.kanaele[0].clips[0].effekte.reihe[0]) == 1);
    }
}

TEST_CASE("Loop Steuersignal 15l: das Fenster Clip effects — Add Sidechain, Channel, Hinweis beim Kreis")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};

    Loop loop = steuerLoop();
    loop.kanaele[0].clips[0].effekte.reihe.clear();
    ClipEffektFenster fenster;  // nicht gezeigt
    QObject::connect(&fenster, &ClipEffektFenster::bearbeitet, &fenster, [&](const Loop& neu, const QString&) {
        loop = neu;
        fenster.zeige(loop, fenster.orte());
    });
    fenster.signalWeg()->resize(1200, 40);
    auto* kanal = fenster.findChild<QComboBox*>(QStringLiteral("fxSidechainChannel"));
    auto* hinweis = fenster.findChild<QLabel*>(QStringLiteral("fxSidechainHint"));
    auto* tiefe = fenster.findChild<QSlider*>(QStringLiteral("fxSidechainDepth"));
    auto* tiefeWert = fenster.findChild<QLabel*>(QStringLiteral("fxSidechainDepthValue"));
    auto* seiten = fenster.findChild<QStackedWidget*>(QStringLiteral("fxPages"));
    for (QObject* teil : std::initializer_list<QObject*>{kanal, hinweis, tiefe, tiefeWert, seiten}) REQUIRE(teil != nullptr);

    fenster.zeige(loop, {{0, 0}});
    REQUIRE(fenster.neuerBaustein(ClipSidechain{}));
    REQUIRE(loop.kanaele[0].clips[0].effekte.reihe.size() == 1u);
    CHECK(seiten->currentIndex() == 5);  // die Seite Sidechain
    CHECK(fenster.signalWeg()->kaesten()[1].titel == QStringLiteral("Sidechain"));
    CHECK(fenster.signalWeg()->kaesten()[1].wert == QString::fromUtf8("no channel · -12 dB"));
    // None und jeder Kanal ausser dem eigenen
    REQUIRE(kanal->count() == 2);
    CHECK(kanal->itemText(0) == QStringLiteral("None"));
    CHECK(kanal->itemText(1) == QStringLiteral("Kick"));
    CHECK(kanal->currentIndex() == 0);
    CHECK(tiefeWert->text() == QStringLiteral("-12 dB"));

    kanal->setCurrentIndex(1);
    Q_EMIT kanal->activated(1);
    CHECK(std::get<ClipSidechain>(loop.kanaele[0].clips[0].effekte.reihe[0]).kanal == loop.kanaele[1].kennung);
    CHECK(fenster.signalWeg()->kaesten()[1].wert == QString::fromUtf8("Kick · -12 dB"));
    CHECK(hinweis->isHidden());
    tiefe->setValue(20);
    CHECK(std::get<ClipSidechain>(loop.kanaele[0].clips[0].effekte.reihe[0]).tiefeDb == 20.0);

    SUBCASE("ein Kreis: der Hinweis sagt, warum kein Signal kommt")
    {
        loop.kanaele[1].clips[0].effekte.reihe = {ClipSidechain{true, loop.kanaele[0].kennung, 6.0, 5.0, 100.0}};
        fenster.zeige(loop, {{0, 0}});
        fenster.waehleStelle(0);
        CHECK_FALSE(hinweis->isHidden());
        CHECK(hinweis->text().startsWith(QStringLiteral("No signal")));
    }
    SUBCASE("das Bild des Fensters")
    {
        fenster.setAttribute(Qt::WA_DontShowOnScreen);
        fenster.show();
        QCoreApplication::processEvents();
        const QImage bild = fenster.grab().toImage();
        CHECK_FALSE(bild.isNull());
        const QString datei = qEnvironmentVariable("LUMIVIZ_SIDECHAIN_BILD");
        if (!datei.isEmpty()) CHECK(bild.save(datei));
    }
}
