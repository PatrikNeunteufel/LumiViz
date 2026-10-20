/**
 * @file test_LoopKits.cpp
 * @brief Loop, Stufe 21a: Kits als eigene Einheit — Rollen, die Datei `.lvkit`,
 *        die Ablage, der Kitwechsel, die mitgelieferten Kits und ihr Pegel
 *        (docs/audio/Loop_Konzept.md §4l; Entscheide K1 a bis K9 a, Patrik S123)
 *
 * Ohne Ton, ohne Fenster. Eigene Kits liegen in einem Wegwerf-Ordner
 * (`setzeEigenenKitOrdnerFuerTests`); die mitgelieferten liest der Test vom
 * Arbeitsordner aus — die Test-Exe aus dem Projekt-Root starten.
 * `LUMIVIZ_KIT_PEGEL_DATEI=<datei.tsv>` legt den Pegel je Klang nach EBU R128 ab
 * (seit Stufe 27d) — fuer `make_kits.py --messung`.
 */

#include <doctest.h>

#include "LoopOhneEngine.hpp"
#include "loop/DrumKit.hpp"
#include "loop/Instrument.hpp"
#include "loop/Kit.hpp"
#include "loop/Loop.hpp"
#include "loop/Pegel.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTextStream>

#include <algorithm>
#include <cmath>
#include <map>

using namespace lumi::loop;

namespace
{

struct KitApp
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QCoreApplication app{argc, argv};
};

/// Eigene Kits in einem Wegwerf-Ordner.
struct KitOrdner
{
    QTemporaryDir tmp;
    QString ordner;
    KitOrdner()
    {
        REQUIRE(tmp.isValid());
        ordner = QDir(tmp.path()).filePath(QStringLiteral("kits"));
        setzeEigenenKitOrdnerFuerTests(ordner);
    }
    ~KitOrdner() { setzeEigenenKitOrdnerFuerTests(QString()); }
};

Spur spur(const QString& name, const QString& klang, const QString& instrument = {})
{
    Spur s(name, klang);
    s.instrument = instrument;
    return s;
}

KitKlang kk(const QString& rolle, const QString& klang, double laut = 1.0, double pan = 0.0)
{
    KitKlang k;
    k.rolle = rolle;
    k.klang = klang;
    k.laut = laut;
    k.pan = pan;
    return k;
}

}  // namespace

TEST_SUITE("Loop Kits")
{
    TEST_CASE("Loop Kits: Rollen — lesen, schreiben, nummeriert (K5 a)")
    {
        CHECK(rollenArten().size() == 11);
        CHECK(leseRolle(QStringLiteral("kick")) == Rolle{QStringLiteral("kick"), 1});
        CHECK(leseRolle(QStringLiteral("  Open Hat ")) == Rolle{QStringLiteral("open hat"), 1});
        CHECK(leseRolle(QStringLiteral("tom 3")) == Rolle{QStringLiteral("tom"), 3});
        CHECK(leseRolle(QStringLiteral("tom")) == Rolle{QStringLiteral("tom"), 1});
        CHECK(leseRolle(QStringLiteral("crash 2")) == Rolle{QStringLiteral("crash"), 2});
        CHECK_FALSE(leseRolle(QStringLiteral("cowbell")).has_value());
        CHECK_FALSE(leseRolle(QStringLiteral("tom 0")).has_value());
        CHECK_FALSE(leseRolle(QStringLiteral("2")).has_value());
        CHECK_FALSE(leseRolle(QString()).has_value());
        CHECK(rollenText({QStringLiteral("tom"), 1}) == QStringLiteral("tom 1"));
        CHECK(rollenText({QStringLiteral("perc"), 2}) == QStringLiteral("perc 2"));
        CHECK(rollenText({QStringLiteral("kick"), 1}) == QStringLiteral("kick"));
        CHECK(rollenText({QStringLiteral("kick"), 2}) == QStringLiteral("kick 2"));
    }

    TEST_CASE("Loop Kits: Rollen raten — aus Instrument, Klang, Name; Toms von tief nach hoch (K1 a)")
    {
        CHECK(rateRollenArt(spur(QStringLiteral("X"), QStringLiteral("kit:kick"))) == QStringLiteral("kick"));
        CHECK(rateRollenArt(spur(QStringLiteral("X"), QStringLiteral("lib:hat_707_open"))) == QStringLiteral("open hat"));
        CHECK(rateRollenArt(spur(QStringLiteral("X"), QStringLiteral("lib:hat_808_closed"))) == QStringLiteral("hat"));
        CHECK(rateRollenArt(spur(QStringLiteral("X"), QStringLiteral("kit:snare"), QStringLiteral("drum_ride"))) ==
              QStringLiteral("ride"));  // das Instrument zaehlt vor dem Klang
        CHECK(rateRollenArt(spur(QStringLiteral("Hi-Hat"), QString())) == QStringLiteral("hat"));
        CHECK(rateRollenArt(spur(QStringLiteral("Open Hat"), QString())) == QStringLiteral("open hat"));
        CHECK(rateRollenArt(spur(QStringLiteral("Shaker"), QString())) == QStringLiteral("perc"));
        CHECK(rateRollenArt(spur(QStringLiteral("X"), QStringLiteral("lib:snap"))) == QStringLiteral("clap"));
        CHECK(rateRollenArt(spur(QStringLiteral("X"), QStringLiteral("lib:voice_male_hey"))) == QStringLiteral("fx"));
        CHECK(rateRollenArt(spur(QStringLiteral("X"), QStringLiteral("C:/samples/My Kick 2.wav"))) == QStringLiteral("kick"));
        CHECK(rateRollenArt(spur(QStringLiteral("Bottom"), QStringLiteral("user:bloop"))).isEmpty());

        Muster m;
        m.spuren = {spur(QStringLiteral("Tom High"), QStringLiteral("kit:tom")), spur(QStringLiteral("Kick"), QStringLiteral("kit:kick")),
                    spur(QStringLiteral("Tom Low"), QStringLiteral("kit:tom")), spur(QStringLiteral("Perc"), QStringLiteral("lib:conga_high")),
                    spur(QStringLiteral("Perc 2"), QStringLiteral("lib:darbuka_tak")), spur(QStringLiteral("Bloop"), QStringLiteral("user:bloop"))};
        m.spuren[4].rolle = QStringLiteral("perc 1");  // schon gesetzt: bleibt
        rateRollen(m);
        CHECK(m.spuren[0].rolle == QStringLiteral("tom 2"));
        CHECK(m.spuren[1].rolle == QStringLiteral("kick"));
        CHECK(m.spuren[2].rolle == QStringLiteral("tom 1"));  // low vor high
        CHECK(m.spuren[3].rolle == QStringLiteral("perc 2"));  // perc 1 war vergeben
        CHECK(m.spuren[4].rolle == QStringLiteral("perc 1"));
        CHECK(m.spuren[5].rolle.isEmpty());  // nicht zu erraten
    }

    TEST_CASE("Loop Kits: die Datei .lvkit — hin und zurueck, Pfade relativ, Unbekanntes faellt weg, Fassung von morgen")
    {
        const KitOrdner u;
        DrumKit kit;
        kit.titel = QStringLiteral("Mein Kit");
        kit.beschreibung = QStringLiteral("Ein Test.");
        kit.klaenge = {kk(QStringLiteral("kick"), QStringLiteral("lib:kick_909"), 0.9),
                       kk(QStringLiteral("hat"), QDir(u.tmp.path()).filePath(QStringLiteral("kits/klang/hat.wav")), 0.5, 0.2)};
        KitKlang tom = kk(QStringLiteral("tom 2"), QStringLiteral("kit:tom"));
        tom.instrument = QStringLiteral("drum_tom_high");
        tom.hoehe = 38;
        tom.dauerMs = 300;
        kit.klaenge.push_back(tom);

        const QString datei = eigeneKitDatei(QStringLiteral("mein_kit"));
        REQUIRE_FALSE(datei.isEmpty());
        REQUIRE(speichereKit(datei, kit));
        QFile f(datei);
        REQUIRE(f.open(QIODevice::ReadOnly));
        const QJsonObject json = QJsonDocument::fromJson(f.readAll()).object();
        CHECK(json.value(QStringLiteral("format")).toString() == QStringLiteral("lumiviz-kit"));
        CHECK(json.value(QStringLiteral("version")).toInt() == kKitFassung);
        const QJsonArray rollen = json.value(QStringLiteral("roles")).toArray();
        REQUIRE(rollen.size() == 3);
        CHECK(rollen.at(1).toObject().value(QStringLiteral("sound")).toString() == QStringLiteral("klang/hat.wav"));  // relativ
        CHECK_FALSE(rollen.at(0).toObject().contains(QStringLiteral("instrument")));
        CHECK(rollen.at(2).toObject().value(QStringLiteral("note")).toInt() == 38);

        DrumKit gelesen;
        REQUIRE(holeKit(QStringLiteral("user:mein_kit"), gelesen));
        CHECK(gelesen.name == QStringLiteral("user:mein_kit"));
        gelesen.name = kit.name;
        CHECK(gelesen == kit);
        REQUIRE(eigeneKits().size() == 1);
        CHECK(eigeneKits()[0].name == QStringLiteral("user:mein_kit"));
        CHECK(holeKit(datei, gelesen));  // auch beim Pfad
        CHECK_FALSE(holeKit(QStringLiteral("user:gibt_es_nicht"), gelesen));
        CHECK(eigeneKitDatei(QStringLiteral("Mein Kit")).isEmpty());  // kein gueltiger Name

        // --- unbekannte und doppelte Rollen fallen weg, Werte in Grenzen
        QJsonObject wild = alsJson(kit);
        QJsonArray r = wild.value(QStringLiteral("roles")).toArray();
        r.append(QJsonObject{{QStringLiteral("role"), QStringLiteral("cowbell")}, {QStringLiteral("sound"), QStringLiteral("kit:clap")}});
        r.append(QJsonObject{{QStringLiteral("role"), QStringLiteral("KICK")}, {QStringLiteral("sound"), QStringLiteral("kit:kick")}});
        r.append(QJsonObject{{QStringLiteral("role"), QStringLiteral("perc")}, {QStringLiteral("sound"), QStringLiteral("lib:rim")},
                             {QStringLiteral("volume"), 7.0}, {QStringLiteral("pan"), -3.0}});
        wild[QStringLiteral("roles")] = r;
        DrumKit bereinigt;
        REQUIRE(ausJson(wild, bereinigt));
        REQUIRE(bereinigt.klaenge.size() == 4);
        CHECK(bereinigt.klaenge[0].klang == QStringLiteral("lib:kick_909"));  // die erste kick gilt
        CHECK(bereinigt.klaenge[3].rolle == QStringLiteral("perc 1"));
        CHECK(bereinigt.klaenge[3].laut == 1.0);
        CHECK(bereinigt.klaenge[3].pan == -1.0);

        // --- eine Fassung von morgen
        wild[QStringLiteral("version")] = kKitFassung + 1;
        bool zuNeu = false;
        CHECK_FALSE(ausJson(wild, bereinigt, {}, &zuNeu));
        CHECK(zuNeu);
        wild[QStringLiteral("format")] = QStringLiteral("lumiviz-loop");
        CHECK_FALSE(ausJson(wild, bereinigt));
    }

    TEST_CASE("Loop Kits: der Kitwechsel — Rolle trifft Rolle, die hoechste Nummer, ohne Gegenstueck, fehlend, changed (K2 a, K4 a, K5 a)")
    {
        DrumKit kit;
        kit.name = QStringLiteral("probe");
        kit.klaenge = {kk(QStringLiteral("kick"), QStringLiteral("lib:kick_909"), 0.9),
                       kk(QStringLiteral("hat"), QStringLiteral("lib:hat_808_closed"), 0.5, 0.2),
                       kk(QStringLiteral("tom 1"), QStringLiteral("kit:tom"), 0.7, -0.25),
                       kk(QStringLiteral("tom 2"), QStringLiteral("kit:tom"), 0.7, 0.25),
                       kk(QStringLiteral("crash"), QString(), 0.6)};
        kit.klaenge[4].instrument = QStringLiteral("drum_crash");

        Muster m;
        m.spuren = {spur(QStringLiteral("Kick"), QStringLiteral("kit:kick")), spur(QStringLiteral("Snare"), QStringLiteral("kit:snare")),
                    spur(QStringLiteral("Tom 1"), QStringLiteral("kit:tom")), spur(QStringLiteral("Tom 2"), QStringLiteral("kit:tom")),
                    spur(QStringLiteral("Tom 3"), QStringLiteral("kit:tom")), spur(QStringLiteral("Bloop"), QStringLiteral("user:bloop"))};
        m.zeilen = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}, {1, 1, 0, 0}, {0, 0, 1, 1}};
        const auto schritte = m.zeilen;

        const KitWechsel w = wendeKitAn(m, kit);
        CHECK(m.zeilen == schritte);  // die Schritte bleiben
        CHECK(m.kit == QStringLiteral("probe"));
        CHECK(w.getroffen == 4);
        CHECK(m.spuren[0].klang == QStringLiteral("lib:kick_909"));
        CHECK(m.spuren[0].laut == 0.9);
        CHECK(m.spuren[1].klang == QStringLiteral("kit:snare"));  // das Kit hat keine Snare: sie bleibt
        CHECK(w.ohneGegenstueck == QStringList{QStringLiteral("snare")});
        CHECK(m.spuren[4].rolle == QStringLiteral("tom 3"));
        CHECK(m.spuren[4].pan == 0.25);  // tom 3 nimmt die hoechste, tom 2
        CHECK(m.spuren[5].klang == QStringLiteral("user:bloop"));  // ohne Rolle: bleibt
        CHECK(w.fehlend == QStringList{QStringLiteral("hat"), QStringLiteral("crash")});
        CHECK_FALSE(weichtAb(m, kit));

        m.spuren[0].laut = 0.5;
        CHECK(weichtAb(m, kit));  // „(changed)"
        wendeKitAn(m, kit);
        CHECK_FALSE(weichtAb(m, kit));

        // --- ein Instrument aus dem Kit, und zurueck zu einem Sample
        Muster mitCrash;
        mitCrash.spuren = {spur(QStringLiteral("Crash"), QStringLiteral("kit:hat_open"))};
        CHECK(rateRollenArt(mitCrash.spuren[0]) == QStringLiteral("open hat"));  // der Klang zaehlt vor dem Namen
        mitCrash.spuren[0].rolle = QStringLiteral("crash");                     // von Hand gesetzt (Klick aufs Etikett)
        wendeKitAn(mitCrash, kit);
        CHECK(mitCrash.spuren[0].instrument == QStringLiteral("drum_crash"));
        CHECK(mitCrash.spuren[0].klang == QStringLiteral("kit:hat_open"));  // bleibt fuer ein aelteres Programm stehen
        CHECK(mitCrash.spuren[0].hoehe == 36);
        DrumKit anderes;
        anderes.name = QStringLiteral("anderes");
        anderes.klaenge = {kk(QStringLiteral("crash"), QStringLiteral("lib:hat_707_open"))};
        wendeKitAn(mitCrash, anderes);
        CHECK(mitCrash.spuren[0].instrument.isEmpty());
        CHECK(mitCrash.spuren[0].klang == QStringLiteral("lib:hat_707_open"));

        // --- Save as kit...: aus den Spuren, je Rolle der erste
        const DrumKit gespeichert = kitAusMuster(m, QStringLiteral("Aus dem Muster"));
        CHECK(gespeichert.titel == QStringLiteral("Aus dem Muster"));
        REQUIRE(gespeichert.klaenge.size() == 5);  // kick, snare, tom 1, tom 2, tom 3 — bloop hat keine Rolle
        CHECK(gespeichert.klaenge[0].rolle == QStringLiteral("kick"));
        CHECK(gespeichert.klaenge[0].laut == 0.9);
    }

    TEST_CASE("Loop Kits: role und kit in der .lvloop — freiwillig, ohne neue Fassung (K1 a, K4 a)")
    {
        Loop loop;
        Muster m;
        m.name = QStringLiteral("Beat");
        m.spuren = {spur(QStringLiteral("Kick"), QStringLiteral("kit:kick")), spur(QStringLiteral("Hat"), QStringLiteral("kit:hat_closed"))};
        m.spuren[0].rolle = QStringLiteral("kick");
        m.kit = QStringLiteral("808");
        loop.muster.push_back(m);
        loop.richte();
        const QJsonObject json = alsJson(loop);
        CHECK(json.value(QStringLiteral("version")).toInt() == 11);  // role und kit brauchen keine neue Fassung; 7 seit E1 a, 8 seit S132, 9 seit S134, 10 seit S136, 11 seit S139
        const QJsonObject muster = json.value(QStringLiteral("patterns")).toArray().at(0).toObject();
        CHECK(muster.value(QStringLiteral("kit")).toString() == QStringLiteral("808"));
        const QJsonArray spuren = muster.value(QStringLiteral("tracks")).toArray();
        CHECK(spuren.at(0).toObject().value(QStringLiteral("role")).toString() == QStringLiteral("kick"));
        CHECK_FALSE(spuren.at(1).toObject().contains(QStringLiteral("role")));  // ohne Rolle kein Feld
        Loop gelesen;
        REQUIRE(ausJson(json, gelesen));
        CHECK(gelesen == loop);

        // ein Kit als Datei steht relativ zum Loop
        const QTemporaryDir tmp;
        loop.muster[0].kit = QDir(tmp.path()).filePath(QStringLiteral("kits/mein.lvkit"));
        const QJsonObject relativ = alsJson(loop, tmp.path());
        CHECK(relativ.value(QStringLiteral("patterns")).toArray().at(0).toObject().value(QStringLiteral("kit")).toString() ==
              QStringLiteral("kits/mein.lvkit"));
        REQUIRE(ausJson(relativ, gelesen, tmp.path()));
        CHECK(gelesen.muster[0].kit == QDir::cleanPath(loop.muster[0].kit));
    }

    TEST_CASE("Loop Kits: die mitgelieferten — vierzehn, jeder Klang da, jede Rolle gueltig (K6 a, K6b a, K6c a)")
    {
        const KitApp app;
        REQUIRE_FALSE(mitgelieferterKitOrdner().isEmpty());
        const std::vector<DrumKit>& alle = mitgelieferteKits();
        // wer ein Kit dazulegt, traegt es in asset/loop/kits/make_kits.py ein und stellt die Zahl nach
        REQUIRE(alle.size() == 14);
        QStringList namen;
        for (const DrumKit& kit : alle) namen << kit.name;
        namen.sort();
        CHECK(namen == QStringList{QStringLiteral("808"), QStringLiteral("909_house"), QStringLiteral("beatbox"),
                                   QStringLiteral("classic"), QStringLiteral("electro"), QStringLiteral("hand_drums"),
                                   QStringLiteral("hardrock"), QStringLiteral("heavy_metal"), QStringLiteral("jazz_brushes"),
                                   QStringLiteral("lo_fi"), QStringLiteral("reggae"), QStringLiteral("reggaeton"),
                                   QStringLiteral("rock_n_roll"), QStringLiteral("strange")});
        for (std::size_t i = 1; i < alle.size(); ++i)
            CHECK(alle[i - 1].titel.compare(alle[i].titel, Qt::CaseInsensitive) <= 0);
        for (const DrumKit& kit : alle)
        {
            CAPTURE(kit.name.toStdString());
            CHECK_FALSE(kit.titel.isEmpty());
            CHECK(kit.beschreibung.endsWith(QLatin1Char('.')));
            REQUIRE(kit.fuer({QStringLiteral("kick"), 1}) != nullptr);  // jedes Kit hat eine Kick
            DrumKit geholt;
            REQUIRE(holeKit(kit.name, geholt));
            CHECK(geholt == kit);
            for (const KitKlang& klang : kit.klaenge)
            {
                CAPTURE(klang.rolle.toStdString());
                CHECK(leseRolle(klang.rolle).has_value());
                if (!klang.instrument.isEmpty())
                    CHECK(findeInstrument(klang.instrument) != nullptr);
                else
                    CHECK(QFileInfo::exists(klangDatei(klang.klang)));
                CHECK(klang.laut > 0.0);
                CHECK(klang.laut <= 1.0);
            }
        }
    }

    // Entscheid K10 a (S124): die beiden Haken — fehlende Rollen anlegen, leere fremde Spuren entfernen
    TEST_CASE("Loop Kits: Kitwechsel mit Haken — Rollen anlegen, leere fremde Spuren entfernen (K10 a)")
    {
        DrumKit kit;
        kit.name = QStringLiteral("probe");
        kit.titel = QStringLiteral("Probe");
        for (const char* rolle : {"kick", "snare", "hat", "tom 1"})
        {
            KitKlang klang;
            klang.rolle = QString::fromLatin1(rolle);
            klang.klang = QStringLiteral("kit:") + QString::fromLatin1(rolle).remove(QLatin1Char(' '));
            klang.laut = 0.7;
            klang.pan = rolle[0] == 'h' ? 0.2 : 0.0;
            kit.klaenge.push_back(klang);
        }
        kit.klaenge[3].instrument = QStringLiteral("drum_tom_low");  // eine Rolle mit Instrument
        kit.klaenge[3].hoehe = 38;
        kit.klaenge[3].dauerMs = 300;
        const KitOptionen beide{true, true};

        // --- ein leeres Muster: alle Rollen als Spuren, in der Reihenfolge des Kits, mit leeren Schritten
        Loop loop;
        Muster leer;
        leer.name = QStringLiteral("P");
        leer.takte = 2;
        loop.muster = {leer};
        KitWechsel wechsel = wendeKitAn(loop, QStringLiteral("P"), kit, beide);
        const Muster* p = loop.findeMuster(QStringLiteral("P"));
        REQUIRE(p != nullptr);
        CHECK(wechsel.angelegt == QStringList{QStringLiteral("kick"), QStringLiteral("snare"), QStringLiteral("hat"),
                                             QStringLiteral("tom 1")});
        CHECK(wechsel.fehlend.isEmpty());
        CHECK(wechsel.getroffen == 0);
        REQUIRE(p->spuren.size() == 4);
        REQUIRE(p->zeilen.size() == 4);
        CHECK(p->spuren[0].name == QStringLiteral("Kick"));
        CHECK(p->spuren[2].name == QStringLiteral("Hat"));
        CHECK(p->spuren[3].name == QStringLiteral("Tom 1"));
        CHECK(p->spuren[3].instrument == QStringLiteral("drum_tom_low"));
        CHECK(p->spuren[3].hoehe == 38);
        CHECK(p->spuren[3].dauerMs == 300);
        CHECK(p->spuren[2].pan == doctest::Approx(0.2));
        CHECK(p->spuren[1].rolle == QStringLiteral("snare"));
        for (const auto& zeile : p->zeilen) CHECK(zeile == std::vector<double>(static_cast<std::size_t>(2 * loop.schritteJeTakt()), 0.0));
        CHECK(p->kit == QStringLiteral("probe"));
        CHECK_FALSE(weichtAb(*p, kit));

        // --- ein Muster mit Spuren: Kick (Schritte), Rim (leer), Perc (Schritte), eine Spur ohne Rolle (leer)
        Muster voll;
        voll.name = QStringLiteral("Q");
        voll.takte = 1;
        voll.spuren = {{QStringLiteral("Kick"), QStringLiteral("kit:kick"), 1.0, 0.0, false},
                       {QStringLiteral("Rim"), QStringLiteral("lib:rim"), 1.0, 0.0, false},
                       {QStringLiteral("Perc"), QStringLiteral("lib:tambourine"), 1.0, 0.0, false},
                       {QStringLiteral("Thing"), QStringLiteral("C:/irgendwo/ding.wav"), 1.0, 0.0, false}};
        voll.zeilen = {std::vector<double>(16, 0.0), std::vector<double>(16, 0.0), std::vector<double>(16, 0.0),
                       std::vector<double>(16, 0.0)};
        voll.zeilen[0][0] = 1.0;
        voll.zeilen[2][4] = 0.5;
        loop.muster = {voll};
        Loop ohne = loop;  // dieselbe Probe ohne Haken: wie bisher (K2 a)
        wechsel = wendeKitAn(loop, QStringLiteral("Q"), kit, beide);
        const Muster* q = loop.findeMuster(QStringLiteral("Q"));
        REQUIRE(q != nullptr);
        CHECK(wechsel.entfernt == QStringList{QStringLiteral("Rim")});                // leer, nicht im Kit
        CHECK(wechsel.ohneGegenstueck == QStringList{QStringLiteral("perc 1")});      // traegt Schritte: bleibt
        CHECK(wechsel.angelegt == QStringList{QStringLiteral("snare"), QStringLiteral("hat"), QStringLiteral("tom 1")});
        QStringList namen;
        for (const Spur& s : q->spuren) namen << s.name;
        // die neuen an ihrem Platz im Kit: vor der ersten Spur, die das Kit nicht kennt
        CHECK(namen == QStringList{QStringLiteral("Kick"), QStringLiteral("Snare"), QStringLiteral("Hat"), QStringLiteral("Tom 1"),
                                   QStringLiteral("Perc"), QStringLiteral("Thing")});
        REQUIRE(q->zeilen.size() == 6);
        CHECK(q->zeilen[0][0] == 1.0);   // Kick behaelt seine Schritte
        CHECK(q->zeilen[4][4] == 0.5);   // Perc auch — die Zeilen sind mit den Spuren gewandert
        CHECK(q->zeilen[1] == std::vector<double>(16, 0.0));
        CHECK(q->spuren[5].rolle.isEmpty());  // ohne Rolle: bleibt, auch leer

        // --- ohne Haken: nichts angelegt, nichts entfernt
        wechsel = wendeKitAn(ohne, QStringLiteral("Q"), kit, KitOptionen{});
        CHECK(ohne.findeMuster(QStringLiteral("Q"))->spuren.size() == 4);
        CHECK(wechsel.angelegt.isEmpty());
        CHECK(wechsel.entfernt.isEmpty());
        CHECK(wechsel.fehlend == QStringList{QStringLiteral("snare"), QStringLiteral("hat"), QStringLiteral("tom 1")});
        // ein Name ist vergeben: die neue Spur heisst weiter
        Muster zwei;
        zwei.spuren = {{QStringLiteral("Snare"), QString(), 1.0, 0.0, false}};
        CHECK(freierSpurName(zwei, QStringLiteral("snare")) == QStringLiteral("Snare (2)"));
        CHECK(wendeKitAn(ohne, QStringLiteral("gibt es nicht"), kit, beide).angelegt.isEmpty());
    }

    // Stufe 27d (Entscheide Patrik S139, LL4 a, LL9 a, LL10 a, LL11 a): gemessen nach EBU R128 — die
    // Kick mit der Drum-Probe (Short-term-Maximum, wie die Instrumente), jede Rolle als Schlag
    // (Momentary-Maximum) gegen die Kick ihres Kits. `LUMIVIZ_KIT_PEGEL_DATEI=<datei.tsv>` legt die
    // Messung bei Volume 1 fuer `make_kits.py --messung` ab.
    TEST_CASE("Loop Kits: Pegel — die Kick jedes Kits auf dem Drum-Anker, jede Rolle auf ihrem Abstand (R128, Stufe 27d)")
    {
        const KitApp app;
        constexpr int kRate = 44100;
        struct Zeile
        {
            QString kit;
            QString rolle;
            QString was;
            double laut;
            double schlag;  // Momentary-Maximum eines Schlags bei Volume 1
            double probe;   // Short-term-Maximum der Drum-Probe bei Volume 1
        };
        std::vector<Zeile> zeilen;
        for (const DrumKit& kit : mitgelieferteKits())
            for (const KitKlang& klang : kit.klaenge)
            {
                const QString was = klang.instrument.isEmpty() ? klang.klang : klang.instrument;
                Klang ton;
                if (!klang.instrument.isEmpty())
                {
                    const Instrument* instrument = findeInstrument(klang.instrument);
                    REQUIRE(instrument != nullptr);
                    looptest::merkeSamplesOhneEngine(*instrument);
                    ton = rendereTon(*instrument, klang.hoehe, klang.dauerMs, kRate);
                }
                else
                    ton = looptest::liesWav(looptest::mitgelieferteWav(klang.klang));
                CAPTURE(was.toStdString());
                REQUIRE_FALSE(ton.daten.empty());
                zeilen.push_back({kit.name, klang.rolle, was, klang.laut, messeSchlag(ton, kRate).momentanMaxLufs,
                                  messeDrumProbe(ton, kRate).kurzzeitMaxLufs});
            }
        REQUIRE_FALSE(zeilen.empty());
        if (const QString datei = qEnvironmentVariable("LUMIVIZ_KIT_PEGEL_DATEI"); !datei.isEmpty())
        {
            QFile f(datei);
            REQUIRE(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
            QTextStream aus(&f);
            aus << "# Loop Kits: Pegel (test_LoopKits.cpp, Stufe 27d): Kit <Tab> Rolle <Tab> Klang <Tab> Volume <Tab> "
                   "Schlag M max <Tab> Probe S max (beide bei Volume 1, LUFS)\n";
            for (const Zeile& z : zeilen)
                aus << z.kit << '\t' << z.rolle << '\t' << z.was << '\t' << z.laut << '\t' << QString::number(z.schlag, 'f', 3)
                    << '\t' << QString::number(z.probe, 'f', 3) << '\n';
            MESSAGE("abgelegt: " << datei.toStdString());
        }

        const auto median = [](std::vector<double> w) {
            std::sort(w.begin(), w.end());
            return w.size() % 2 == 1 ? w[w.size() / 2] : (w[w.size() / 2 - 1] + w[w.size() / 2]) / 2.0;
        };
        const auto db = [](double laut) { return 20.0 * std::log10(laut); };
        // die Kicks: mit ihrem Volume auf der Drum-Probe beieinander (±1 LU um ihren Median, den Anker)
        std::map<QString, const Zeile*> kickVon;
        std::vector<double> kicks;
        for (const Zeile& z : zeilen)
            if (z.rolle == QLatin1String("kick"))
            {
                kickVon[z.kit] = &z;
                kicks.push_back(z.probe + db(z.laut));
            }
        REQUIRE(kickVon.size() == mitgelieferteKits().size());
        const double anker = median(kicks);
        MESSAGE("Drum-Anker (Kick auf der Drum-Probe, S max): " << anker << " LUFS; der Bezug der Instrumente liegt "
                                                                 << (kInstrumentBezugLufs - anker) << " LU darueber");
        for (const auto& [kit, kick] : kickVon)
        {
            CAPTURE(kit.toStdString());
            const bool gekappt = kick->laut >= 0.999 && kick->probe < anker - 1.0;
            if (!gekappt) CHECK(std::abs(kick->probe + db(kick->laut) - anker) <= 1.0);
        }
        // jede andere Rolle: als Schlag gegen die Kick ihres Kits, je Art ±1 LU um den Median — ausser
        // eine gekappte (Volume 1 und doch darunter, LL11 a: der Bericht nennt sie)
        std::map<QString, std::vector<std::pair<const Zeile*, double>>> jeArt;
        for (const Zeile& z : zeilen)
        {
            if (z.rolle == QLatin1String("kick")) continue;
            const Zeile* kick = kickVon[z.kit];
            jeArt[leseRolle(z.rolle)->art].push_back({&z, z.schlag + db(z.laut) - (kick->schlag + db(kick->laut))});
        }
        int gekappteRollen = 0;
        for (const auto& [art, werte] : jeArt)
        {
            std::vector<double> nur;
            for (const auto& w : werte) nur.push_back(w.second);
            const double m = median(nur);
            MESSAGE(art.toStdString() << ": " << werte.size() << " Rollen, Abstand zur Kick im Median " << m << " LU");
            for (const auto& [z, abstand] : werte)
            {
                CAPTURE(z->kit.toStdString());
                CAPTURE(z->rolle.toStdString());
                if (z->laut >= 0.999 && abstand < m - 1.0)
                {
                    ++gekappteRollen;
                    MESSAGE("gekappt: " << z->kit.toStdString() << ", " << z->rolle.toStdString() << " ("
                                        << z->was.toStdString() << ") fehlen " << (m - abstand) << " LU");
                    continue;
                }
                CHECK(std::abs(abstand - m) <= 1.0);
            }
        }
        MESSAGE(gekappteRollen << " Rollen gekappt");
    }

}
