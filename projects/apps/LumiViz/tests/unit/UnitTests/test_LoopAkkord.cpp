/**
 ****************************************************************************************
 * @file   test_LoopAkkord.cpp
 * @brief  Tabs und Akkorde, Stufe 26a: das Modell — Griffbild oder Voicing, die Datei
 *         `.lvchord`, die Ablage, die Umrechnung in Noten, Datensicherung und Publish
 *         (docs/audio/Saiten_Konzept.md §1; Entscheide Patrik S137: S-T1 a bis S-T7 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Fenster und ohne Qt-Anwendung; eigene Akkorde in einem Wegwerf-Ordner
 * (`setzeEigenenAkkordOrdnerFuerTests`). Seit 26b die mitgelieferten Vorlagen aus
 * `asset/loop/chords/make_chords.py` (Zahl je Ordner; vom Arbeitsordner aus gelesen — die
 * Test-Exe aus dem Projekt-Root starten).
 ****************************************************************************************
 */

#include <doctest.h>

#include "core/Datensicherung.hpp"
#include "loop/Akkord.hpp"
#include "loop/Instrument.hpp"  // istEigenerName
#include "paket/Sammler.hpp"

#include "LoopOhneEngine.hpp"  // repoWurzel

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>

#include <algorithm>
#include <map>
#include <vector>

using namespace lumi::loop;

namespace
{

/// Gitarre in Standardstimmung, von der tiefen E-Saite an (wie `Saitensatz::leer`).
Saitensatz gitarre()
{
    Saitensatz s;
    s.leer = {40, 45, 50, 55, 59, 64};
    return s;
}

/// C-Dur in der ersten Lage, wie chords-db ihn schreibt: `x32010`, Finger `032010`.
Akkord cDur()
{
    Akkord a;
    a.titel = QStringLiteral("C major");
    a.grundton = 0;
    a.art = QStringLiteral("major");
    a.leer = gitarre().leer;
    a.buende = {-1, 3, 2, 0, 1, 0};
    a.finger = {0, 3, 2, 0, 1, 0};
    return a;
}

Akkord voicing(std::vector<int> abstaende)
{
    Akkord a;
    a.titel = QStringLiteral("C major");
    a.grundton = 0;
    a.art = QStringLiteral("major");
    a.abstaende = std::move(abstaende);
    return a;
}

std::vector<int> hoehen(const std::vector<AkkordTon>& toene)
{
    std::vector<int> h;
    for (const AkkordTon& t : toene) h.push_back(t.hoehe);
    return h;
}

/// Ein Wegwerf-Ordner fuer eigene Akkorde, solange er lebt.
struct EigenerOrdner
{
    QTemporaryDir ordner;
    EigenerOrdner() { setzeEigenenAkkordOrdnerFuerTests(ordner.path() + QStringLiteral("/chords")); }
    ~EigenerOrdner() { setzeEigenenAkkordOrdnerFuerTests({}); }
};

}  // namespace

TEST_SUITE("LoopAkkord")
{
    TEST_CASE("Loop Akkord 26a: Grundtoene — Namen mit # und b, enharmonisch gelesen")
    {
        CHECK(grundtonNamen().size() == 12);
        CHECK(grundtonNamen().at(1) == QStringLiteral("C#"));
        CHECK(grundtonNamen().at(10) == QStringLiteral("Bb"));
        for (int ton = 0; ton < 12; ++ton) CHECK(grundtonAusName(grundtonNamen().at(ton)) == ton);
        CHECK(grundtonAusName(QStringLiteral("Db")) == 1);
        CHECK(grundtonAusName(QStringLiteral("a#")) == 10);
        CHECK(grundtonAusName(QStringLiteral("Cb")) == 11);
        CHECK(grundtonAusName(QStringLiteral(" B# ")) == 0);
        CHECK_FALSE(grundtonAusName(QStringLiteral("H")));
        CHECK_FALSE(grundtonAusName(QStringLiteral("C##")));
        CHECK_FALSE(grundtonAusName(QString()));
    }

    TEST_CASE("Loop Akkord 26a: Griffbild — .lvchord Fassung 1, Grenzen, was nicht gilt")
    {
        Akkord c = cDur();
        c.barre = {3};
        c.herkunft = QString::fromLatin1(kHerkunftGerechnet);
        c.beschreibung = QStringLiteral("Open position");
        const QJsonObject json = alsJson(c);
        CHECK(json.value(QStringLiteral("format")).toString() == QStringLiteral("lumiviz-chord"));
        CHECK(json.value(QStringLiteral("version")).toInt() == kAkkordFassung);
        CHECK(json.value(QStringLiteral("root")).toString() == QStringLiteral("C"));
        CHECK(json.value(QStringLiteral("type")).toString() == QStringLiteral("major"));
        CHECK(json.value(QStringLiteral("origin")).toString() == QStringLiteral("computed"));
        CHECK_FALSE(json.contains(QStringLiteral("voicing")));
        const QJsonObject saiten = json.value(QStringLiteral("strings")).toObject();
        CHECK(saiten.value(QStringLiteral("frets")).toArray() == QJsonArray{-1, 3, 2, 0, 1, 0});
        CHECK(saiten.value(QStringLiteral("open")).toArray() == QJsonArray{40, 45, 50, 55, 59, 64});
        Akkord zurueck;
        REQUIRE(ausJson(json, zurueck));
        CHECK(zurueck == c);
        CHECK(zurueck.istGriffbild());

        // ohne Herkunft steht kein `origin` in der Datei
        CHECK_FALSE(alsJson(cDur()).contains(QStringLiteral("origin")));

        // Grenzen: Buende −1 … 24, Finger 0 … 4, fehlende Saiten spielen nicht, Barre richtig
        QJsonObject wild = json;
        QJsonObject s = saiten;
        s[QStringLiteral("frets")] = QJsonArray{-5, 30, 2};
        s[QStringLiteral("fingers")] = QJsonArray{7, -1};
        s[QStringLiteral("barres")] = QJsonArray{5, 0, 3, 5, 40};
        wild[QStringLiteral("strings")] = s;
        Akkord gerichtet;
        REQUIRE(ausJson(wild, gerichtet));
        CHECK(gerichtet.buende == std::vector<int>{-1, kBuendeMax, 2, -1, -1, -1});
        CHECK(gerichtet.finger == std::vector<int>{4, 0, 0, 0, 0, 0});
        CHECK(gerichtet.barre == std::vector<int>{3, 5});

        // was nicht gilt: falsches Format, Fassung von morgen, ohne Grundton, keine gespielte Saite
        bool zuNeu = true;
        QJsonObject fremd = json;
        fremd[QStringLiteral("format")] = QStringLiteral("lumiviz-kit");
        CHECK_FALSE(ausJson(fremd, gerichtet, &zuNeu));
        CHECK_FALSE(zuNeu);
        QJsonObject morgen = json;
        morgen[QStringLiteral("version")] = kAkkordFassung + 1;
        CHECK_FALSE(ausJson(morgen, gerichtet, &zuNeu));
        CHECK(zuNeu);
        QJsonObject ohneGrund = json;
        ohneGrund[QStringLiteral("root")] = QStringLiteral("H");
        CHECK_FALSE(ausJson(ohneGrund, gerichtet));
        QJsonObject stumm = json;
        QJsonObject alleAus = saiten;
        alleAus[QStringLiteral("frets")] = QJsonArray{-1, -1, -1, -1, -1, -1};
        stumm[QStringLiteral("strings")] = alleAus;
        CHECK_FALSE(ausJson(stumm, gerichtet));
        CHECK(gerichtet.barre == std::vector<int>{3, 5});  // bleibt, wie es war
    }

    TEST_CASE("Loop Akkord 26a: Voicing — die Abstaende, aufsteigend ab 0, in der Datei")
    {
        const Akkord grund = voicing({0, 4, 7});
        const QJsonObject json = alsJson(grund);
        CHECK_FALSE(json.contains(QStringLiteral("strings")));
        CHECK(json.value(QStringLiteral("voicing")).toArray() == QJsonArray{0, 4, 7});
        Akkord zurueck;
        REQUIRE(ausJson(json, zurueck));
        CHECK(zurueck == grund);
        CHECK_FALSE(zurueck.istGriffbild());

        // ungeordnet, doppelt, nicht ab 0: geordnet und auf den tiefsten Ton gerueckt
        Akkord a = voicing({16, 7, 4, 7, 60, -2});
        richteAkkord(a);
        CHECK(a.abstaende == std::vector<int>{0, 3, 12});
        // hoechstens zehn Toene
        Akkord viele = voicing({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11});
        richteAkkord(viele);
        CHECK(viele.abstaende.size() == static_cast<std::size_t>(kVoicingToeneMax));
        // ohne Ton gilt es nicht
        QJsonObject leer = json;
        leer[QStringLiteral("voicing")] = QJsonArray{};
        CHECK_FALSE(ausJson(leer, zurueck));
    }

    TEST_CASE("Loop Akkord 26a: die Toene eines Griffbilds — Saiten, Capo, auf dem Griffbrett verschoben")
    {
        const Akkord c = cDur();
        Saitensatz s = gitarre();
        REQUIRE(passtZu(c, s));
        const auto toene = toeneDesGriffbilds(c, s);
        REQUIRE(toene);
        // A-Saite 3. Bund C3, D 2. E3, G leer G3, H 1. C4, e leer E4
        CHECK(*toene == std::vector<AkkordTon>{{48, 1}, {52, 2}, {55, 3}, {60, 4}, {64, 5}});

        // mit Capo im 2. Bund klingt alles zwei Halbtoene hoeher (der Griff zaehlt ab dem Capo)
        s.capo = 2;
        CHECK(hoehen(*toeneDesGriffbilds(c, s)) == std::vector<int>{50, 54, 57, 62, 66});
        s.capo = 0;

        // um zwei Buende verschoben: der Griff wandert, die leeren Saiten werden gegriffen — D-Dur
        const auto d = toeneDesGriffbilds(c, s, 2);
        REQUIRE(d);
        CHECK(hoehen(*d) == std::vector<int>{50, 54, 57, 62, 66});
        for (const AkkordTon& t : *d) CHECK((t.hoehe % 12 == 2 || t.hoehe % 12 == 6 || t.hoehe % 12 == 9));
        // ueber den letzten Bund hinaus oder unter den Sattel: nichts
        CHECK_FALSE(toeneDesGriffbilds(c, s, kBuendeMax - 1));
        CHECK_FALSE(toeneDesGriffbilds(c, s, -1));

        // eine andere Stimmung passt nicht (Drop D), ein Voicing passt zu keinem Saiteninstrument
        Saitensatz dropD = gitarre();
        dropD.leer[0] = 38;
        CHECK_FALSE(passtZu(c, dropD));
        CHECK_FALSE(toeneDesGriffbilds(c, dropD));
        CHECK_FALSE(passtZu(voicing({0, 4, 7}), s));
        CHECK(passtZu(voicing({0, 4, 7}), Saitensatz{}));
        CHECK_FALSE(passtZu(c, Saitensatz{}));
        const std::vector<Akkord> alle = {c, voicing({0, 4, 7})};
        CHECK(passendeAkkorde(alle, s).size() == 1);
        CHECK(passendeAkkorde(alle, Saitensatz{}).front().abstaende == std::vector<int>{0, 4, 7});
        CHECK(passendeAkkorde(alle, dropD).empty());
    }

    TEST_CASE("Loop Akkord 26a: die Toene eines Voicings und die Noten fuer ein Muster")
    {
        const Akkord erste = voicing({0, 3, 8});  // C-Dur, erste Umkehrung auf E
        CHECK(hoehen(*toeneDesVoicings(erste, 64)) == std::vector<int>{64, 67, 72});
        for (const AkkordTon& t : *toeneDesVoicings(erste, 64)) CHECK(t.saite == -1);
        CHECK_FALSE(toeneDesVoicings(erste, kHoeheMax - 2));
        CHECK_FALSE(toeneDesVoicings(erste, kHoeheMin - 1));
        CHECK_FALSE(toeneDesVoicings(cDur(), 60));      // ein Griffbild ist kein Voicing
        CHECK_FALSE(toeneDesGriffbilds(erste, gitarre()));

        // die Noten: alle am Schritt, so lang, Lautstaerke 1; das Griffbild setzt die Saiten (S-M2 a)
        const std::vector<Note> griff = akkordNoten(cDur(), gitarre(), 8, 4, 0);
        REQUIRE(griff.size() == 5);
        for (const Note& n : griff)
        {
            CHECK(n.schritt == 8);
            CHECK(n.laenge == 4);
            CHECK(n.laut == 1.0);
            CHECK(n.saite >= 1);
        }
        CHECK(griff.front().hoehe == 48);
        CHECK(griff.front().saite == 1);
        const std::vector<Note> tasten = akkordNoten(erste, Saitensatz{}, 0, 2, 52);
        REQUIRE(tasten.size() == 3);
        CHECK(tasten[0].hoehe == 52);
        CHECK(tasten[2].saite == -1);
        // nicht zu setzen: leer
        CHECK(akkordNoten(cDur(), Saitensatz{}, 0, 1, 0).empty());
        // Schritt und Laenge in ihren Grenzen
        const std::vector<Note> grenze = akkordNoten(erste, Saitensatz{}, -3, 0, 60);
        CHECK(grenze.front().schritt == 0);
        CHECK(grenze.front().laenge == 1);
    }

    TEST_CASE("Loop Akkord 26a: Ablage — eigene als user:<name>, beim Pfad, Titel aus dem Dateinamen")
    {
        EigenerOrdner wegwerf;
        REQUIRE(wegwerf.ordner.isValid());
        CHECK(eigenerAkkordOrdner().endsWith(QStringLiteral("/chords")));
        CHECK(eigeneAkkorde().empty());
        CHECK(eigeneAkkordDatei(QStringLiteral("Mein C")).isEmpty());  // kein gueltiger Name
        const QString datei = eigeneAkkordDatei(QStringLiteral("mein_c"));
        CHECK(datei.endsWith(QStringLiteral("/chords/mein_c.lvchord")));
        REQUIRE(speichereAkkord(datei, cDur()));
        Akkord jazz = voicing({0, 4, 10, 14});
        jazz.titel = QStringLiteral("C9 shell");
        jazz.art = QStringLiteral("9");
        REQUIRE(speichereAkkord(eigeneAkkordDatei(QStringLiteral("c9_shell")), jazz));

        const std::vector<Akkord> eigene = eigeneAkkorde();
        REQUIRE(eigene.size() == 2);
        CHECK(eigene[0].titel == QStringLiteral("C major"));  // nach Titel
        CHECK(eigene[0].name == QStringLiteral("user:mein_c"));
        CHECK(eigene[1].name == QStringLiteral("user:c9_shell"));

        Akkord geholt;
        REQUIRE(holeAkkord(QStringLiteral("user:c9_shell"), geholt));
        CHECK(geholt.abstaende == std::vector<int>{0, 4, 10, 14});
        CHECK(geholt.name == QStringLiteral("user:c9_shell"));
        REQUIRE(holeAkkord(datei, geholt));
        CHECK(geholt.name == datei);
        CHECK_FALSE(holeAkkord(QStringLiteral("user:gibt_es_nicht"), geholt));
        CHECK_FALSE(holeAkkord(QStringLiteral("guitar/gibt_es_nicht"), geholt));

        // ohne Titel in der Datei gilt der Dateiname
        Akkord ohneTitel = cDur();
        ohneTitel.titel.clear();
        const QString frei = wegwerf.ordner.filePath(QStringLiteral("frei/g_shape.lvchord"));
        REQUIRE(speichereAkkord(frei, ohneTitel));
        REQUIRE(ladeAkkord(frei, geholt));
        CHECK(geholt.titel == QStringLiteral("g_shape"));
    }

    TEST_CASE("Loop Akkord 26b: die mitgelieferten Vorlagen — Zahl je Ordner, jede liest sich und klingt ihre Art")
    {
        // die Zahl je Ordner aus `asset/loop/chords/make_chords.py` (wer neu erzeugt, stellt sie nach)
        struct Ordner
        {
            const char* name;
            int zahl;
            std::vector<int> leer;  // leer = Voicings
            bool gerechnet;
        };
        const std::vector<Ordner> ordner = {
            {"guitar", 2069, {40, 45, 50, 55, 59, 64}, false}, {"ukulele", 2114, {67, 60, 64, 69}, false},
            {"drop_d", 468, {38, 45, 50, 55, 59, 64}, true},   {"bass", 426, {28, 33, 38, 43}, true},
            {"violin", 420, {55, 62, 69, 76}, true},           {"voicings", 876, {}, false},
        };
        // die Arten der gerechneten Griffbilder (wie ABSTAENDE im Skript; Lehrbuchwissen)
        const std::map<QString, std::vector<int>> abstaende = {
            {QStringLiteral("major"), {0, 4, 7}},  {QStringLiteral("minor"), {0, 3, 7}},    {QStringLiteral("7"), {0, 4, 7, 10}},
            {QStringLiteral("maj7"), {0, 4, 7, 11}}, {QStringLiteral("m7"), {0, 3, 7, 10}}, {QStringLiteral("sus2"), {0, 2, 7}},
            {QStringLiteral("sus4"), {0, 5, 7}},   {QStringLiteral("dim"), {0, 3, 6}},      {QStringLiteral("aug"), {0, 4, 8}},
            {QStringLiteral("m7b5"), {0, 3, 6, 10}}, {QStringLiteral("6"), {0, 4, 7, 9}},   {QStringLiteral("m6"), {0, 3, 7, 9}},
            {QStringLiteral("add9"), {0, 4, 7, 14}},
        };
        const QDir basis(looptest::repoWurzel().filePath(QStringLiteral("asset/loop/chords")));
        int zusammen = 0;
        for (const Ordner& o : ordner)
        {
            CAPTURE(o.name);
            const QDir hier(basis.filePath(QLatin1String(o.name)));
            const QStringList dateien = hier.entryList({QStringLiteral("*.lvchord")}, QDir::Files);
            CHECK(dateien.size() == o.zahl);
            zusammen += static_cast<int>(dateien.size());
            Saitensatz saiten;
            saiten.leer = o.leer;
            for (const QString& datei : dateien)
            {
                CAPTURE(datei.toStdString());
                Akkord a;
                REQUIRE(ladeAkkord(hier.filePath(datei), a));
                CHECK(istEigenerName(QFileInfo(datei).completeBaseName()));
                CHECK(a.herkunft == (o.gerechnet ? QString::fromLatin1(kHerkunftGerechnet) : QString()));
                if (o.leer.empty())
                {
                    CHECK_FALSE(a.istGriffbild());
                    CHECK(toeneDesVoicings(a, 48).has_value());
                    continue;
                }
                REQUIRE(a.leer == o.leer);
                const auto toene = toeneDesGriffbilds(a, saiten);
                REQUIRE(toene.has_value());
                if (!o.gerechnet) continue;
                // gerechnet (S-T4 a): nur Toene der Art, der Grundton im Bass, hoechstens vier Buende
                REQUIRE(abstaende.count(a.art) == 1);
                std::vector<int> stufen;
                for (const int s : abstaende.at(a.art)) stufen.push_back((a.grundton + s) % 12);
                int tiefster = 1000;
                for (const AkkordTon& t : *toene)
                {
                    CHECK(std::find(stufen.begin(), stufen.end(), t.hoehe % 12) != stufen.end());
                    tiefster = std::min(tiefster, t.hoehe);
                }
                CHECK(tiefster % 12 == a.grundton);
                int tief = 99;
                int hoch = 0;
                for (const int b : a.buende)
                    if (b > 0)
                    {
                        tief = std::min(tief, b);
                        hoch = std::max(hoch, b);
                    }
                CHECK(hoch - std::min(tief, hoch) <= 3);
            }
        }
        CHECK(zusammen == 6373);

        // bekannte Griffe: C-Dur in der ersten Lage (chords-db x32010), ein Voicing
        Akkord c;
        REQUIRE(ladeAkkord(basis.filePath(QStringLiteral("guitar/c_major_1.lvchord")), c));
        CHECK(c.buende == std::vector<int>{-1, 3, 2, 0, 1, 0});
        CHECK(c.titel == QStringLiteral("C"));
        CHECK(c.beschreibung == QStringLiteral("Position 1, open"));
        Akkord g7;
        REQUIRE(ladeAkkord(basis.filePath(QStringLiteral("voicings/g_7_inv1.lvchord")), g7));
        CHECK(g7.titel == QStringLiteral("G7"));
        CHECK(g7.abstaende == std::vector<int>{0, 3, 6, 8});  // B D F G

        // das Programm findet sie beim Pfad unter `asset/loop/chords` (vom Arbeitsordner aus)
        CHECK(static_cast<int>(mitgelieferteAkkorde().size()) == zusammen);
        Akkord geholt;
        REQUIRE(holeAkkord(QStringLiteral("guitar/c_major_1"), geholt));
        CHECK(geholt.buende == c.buende);
        CHECK(passendeAkkorde(mitgelieferteAkkorde(), gitarre()).size() == 2069);
        CHECK(passendeAkkorde(mitgelieferteAkkorde(), Saitensatz{}).size() == 876);
    }

    TEST_CASE("Loop Akkord 26a: Datensicherung und Publish kennen die Akkorde")
    {
        namespace ds = lumi::core::datensicherung;
        namespace pk = lumi::paket;
        const ds::Bereich* bereich = ds::findeBereich(QStringLiteral("chords"));
        REQUIRE(bereich != nullptr);
        CHECK(bereich->titel == QStringLiteral("Chords"));
        CHECK(bereich->ordner == QStringList{QStringLiteral("chords")});
        CHECK(bereich->anfangsAn);

        CHECK(pk::artVonDatei(QStringLiteral("a/b.LVCHORD")) == pk::Art::Akkord);
        CHECK(pk::artText(pk::Art::Akkord) == QStringLiteral("lvchord"));

        // ein eigener Akkord als Hauptding: er nennt nichts, heisst beim Empfaenger wie hier
        EigenerOrdner wegwerf;
        const QString datei = eigeneAkkordDatei(QStringLiteral("mein_c"));
        REQUIRE(speichereAkkord(datei, cDur()));
        const pk::Fund fund = pk::sammle(datei);
        REQUIRE(fund.ok);
        CHECK(fund.art == pk::Art::Akkord);
        CHECK(fund.titel == QStringLiteral("C major"));
        CHECK(fund.hauptName == QStringLiteral("mein_c"));
        CHECK(fund.zugehoerige.empty());
        CHECK(fund.mitgeliefert.empty());
        CHECK(fund.fehlt.empty());
        // nicht lesbar: kein Fund
        QFile kaputt(wegwerf.ordner.filePath(QStringLiteral("kaputt.lvchord")));
        REQUIRE(kaputt.open(QIODevice::WriteOnly));
        kaputt.write("{");
        kaputt.close();
        CHECK_FALSE(pk::sammle(kaputt.fileName()).ok);
    }
}
