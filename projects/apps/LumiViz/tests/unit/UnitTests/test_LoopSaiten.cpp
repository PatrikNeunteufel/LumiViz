/**
 ****************************************************************************************
 * @file   test_LoopSaiten.cpp
 * @brief  Saiteninstrumente, Stufe 23a: das Modell — die Saiten am Instrument, die
 *         Saite einer Note, String choice am Muster; `.lvinst` Fassung 10, `.lvloop`
 *         Fassung 8 (docs/audio/Saiten_Konzept.md; Entscheide Patrik S132: S-M1 a,
 *         S-M2 a, S-M2b, S-M2c a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Nur Modell und Dateien: ohne Ton, ohne Fenster, ohne Qt-Anwendung. Welche Saite eine
 * Note ohne eigene bekommt, rechnet erst Stufe 23b. Der Beleg der Stufe: jede
 * mitgelieferte `.lvinst` und jeder mitgelieferte Loop liest sich wie vorher — ohne
 * Saiten — und schreibt sich zu demselben Modell zurueck.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"
#include "loop/Saiten.hpp"

#include "LoopOhneEngine.hpp"  // repoWurzel, mitgelieferteLoops

#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QTemporaryDir>

#include <vector>

using namespace lumi::loop;

namespace
{

using looptest::mitgelieferteLoops;
using looptest::repoWurzel;

/// Ein Loop mit einem Instrument-Muster und einem Clip davon.
Loop gitarrenLoop()
{
    Loop loop;
    Muster m;
    m.name = QStringLiteral("Riff");
    m.art = MusterArt::Instrument;
    m.instrument = QStringLiteral("guitar");
    m.takte = 1;
    m.noten = {Note{0, 4, 40, 1.0, 0}, Note{0, 4, 47, 0.8, 1}, Note{4, 2, 52, 0.9}, Note{8, 2, 64, 1.0, 5}};
    m.saitenwahl = Saitenwahl::Hand;
    loop.muster.push_back(m);
    Kanal kanal;
    kanal.name = QStringLiteral("Guitar");
    kanal.clips.push_back(LoopClip{});
    kanal.clips[0].muster = m.name;
    loop.kanaele.push_back(kanal);
    loop.richte();
    return loop;
}

QJsonArray notenIn(const QJsonObject& wurzel)
{
    return wurzel.value(QStringLiteral("patterns")).toArray().at(0).toObject().value(QStringLiteral("notes")).toArray();
}

}  // namespace

TEST_CASE("Loop Saiten: der Saitensatz — Grenzen, Vorlagen, die Regel als Name")
{
    Saitensatz keiner;
    CHECK_FALSE(keiner.istSaiteninstrument());
    CHECK(keiner.anzahl() == 0);
    CHECK(vorlageVon(keiner) == nullptr);
    CHECK(keiner.wahl == Saitenwahl::JeKlang);  // Standard Per chord (S-M2c a)

    // --- die Vorlagen der Auswahl Tuning (S-U1 a), in dieser Reihenfolge
    const std::vector<Stimmungsvorlage>& vorlagen = stimmungsvorlagen();
    REQUIRE(vorlagen.size() == 6);
    QStringList titel;
    for (const Stimmungsvorlage& v : vorlagen) titel << QString::fromLatin1(v.titel);
    CHECK(titel == QStringList{QStringLiteral("Guitar standard"), QStringLiteral("Drop D"), QStringLiteral("Bass 4"),
                               QStringLiteral("Bass 5"), QStringLiteral("Ukulele"), QStringLiteral("Violin")});
    for (const Stimmungsvorlage& v : vorlagen)
    {
        CAPTURE(v.titel);
        CHECK(!v.leer.empty());
        CHECK(static_cast<int>(v.leer.size()) <= kSaitenMax);
    }
    // die Toene als Namen, damit ein Tippfehler in den Nummern auffaellt
    const auto namen = [](const std::vector<int>& toene) {
        QStringList n;
        for (const int ton : toene) n << tonName(ton);
        return n.join(QLatin1Char(' '));
    };
    CHECK(namen(vorlagen[0].leer) == QStringLiteral("E2 A2 D3 G3 B3 E4"));
    CHECK(namen(vorlagen[1].leer) == QStringLiteral("D2 A2 D3 G3 B3 E4"));
    CHECK(namen(vorlagen[2].leer) == QStringLiteral("E1 A1 D2 G2"));
    CHECK(namen(vorlagen[3].leer) == QStringLiteral("B0 E1 A1 D2 G2"));
    CHECK(namen(vorlagen[4].leer) == QStringLiteral("G4 C4 E4 A4"));  // die G-Saite ist hoch
    CHECK(namen(vorlagen[5].leer) == QStringLiteral("G3 D4 A4 E5"));

    // --- die Vorlage folgt aus den Toenen, das Capo zaehlt nicht
    Saitensatz gitarre{vorlagen[0].leer, 3, Saitenwahl::JeKlang};
    REQUIRE(vorlageVon(gitarre) != nullptr);
    CHECK(QString::fromLatin1(vorlageVon(gitarre)->schluessel) == QStringLiteral("guitar"));
    gitarre.leer[0] = 38;
    REQUIRE(vorlageVon(gitarre) != nullptr);
    CHECK(QString::fromLatin1(vorlageVon(gitarre)->schluessel) == QStringLiteral("dropD"));
    gitarre.leer[1] = 44;  // eine eigene Stimmung: Custom
    CHECK(vorlageVon(gitarre) == nullptr);

    // --- Grenzen
    Saitensatz wild;
    for (int i = 0; i < 15; ++i) wild.leer.push_back(i == 0 ? -5 : 200);
    wild.capo = 40;
    wild.wahl = static_cast<Saitenwahl>(7);
    richteSaiten(wild);
    CHECK(wild.anzahl() == kSaitenMax);
    CHECK(wild.leer.front() == 0);
    CHECK(wild.leer.back() == 127);
    CHECK(wild.capo == kCapoMax);
    CHECK(wild.wahl == Saitenwahl::JeKlang);
    wild.capo = -2;
    richteSaiten(wild);
    CHECK(wild.capo == 0);

    // --- die Regel als Name in der Datei
    CHECK(saitenwahlName(Saitenwahl::JeKlang) == QLatin1String("perChord"));
    CHECK(saitenwahlName(Saitenwahl::Hand) == QLatin1String("followHand"));
    CHECK(saitenwahlAusName(QStringLiteral("followHand")) == Saitenwahl::Hand);
    CHECK(saitenwahlAusName(QStringLiteral("perChord")) == Saitenwahl::JeKlang);
    CHECK(saitenwahlAusName(QString()) == Saitenwahl::JeKlang);
    CHECK(saitenwahlAusName(QStringLiteral("strum")) == Saitenwahl::JeKlang);
}

TEST_CASE("Loop Saiten: .lvinst Fassung 10 — der Block strings, hin und zurueck, eine aeltere Datei")
{
    QTemporaryDir ordner;
    REQUIRE(ordner.isValid());
    Instrument instrument;
    instrument.titel = QStringLiteral("Twelve");
    instrument.schichten[0].schwingung = Schwingung::Saite;
    instrument.saiten = Saitensatz{stimmungsvorlagen()[0].leer, 2, Saitenwahl::Hand};

    const QJsonObject json = alsJson(instrument);
    CHECK(kInstrumentFassung == 10);
    CHECK(json.value(QStringLiteral("version")).toInt() == 10);
    const QJsonObject saiten = json.value(QStringLiteral("strings")).toObject();
    CHECK(saiten.value(QStringLiteral("open")).toArray() == QJsonArray{40, 45, 50, 55, 59, 64});
    CHECK(saiten.value(QStringLiteral("capo")).toInt() == 2);
    CHECK(saiten.value(QStringLiteral("stringChoice")).toString() == QStringLiteral("followHand"));

    Instrument gelesen;
    REQUIRE(ausJson(json, gelesen));
    CHECK(gelesen == instrument);
    const QString pfad = ordner.filePath(QStringLiteral("twelve.lvinst"));
    REQUIRE(speichereInstrument(instrument, pfad));
    Instrument ausDatei;
    REQUIRE(ladeInstrument(pfad, ausDatei));
    ausDatei.name = instrument.name;
    CHECK(ausDatei == instrument);

    // --- ohne Saiten traegt die Datei keinen Block
    Instrument piano;
    const QJsonObject ohne = alsJson(piano);
    CHECK_FALSE(ohne.contains(QStringLiteral("strings")));
    REQUIRE(ausJson(ohne, gelesen));
    CHECK_FALSE(gelesen.saiten.istSaiteninstrument());

    // --- eine Datei der Fassung 9 hat keine Saiten; ein Block darin zaehlt trotzdem (gerichtet)
    QJsonObject alt = json;
    alt[QStringLiteral("version")] = 9;
    alt.remove(QStringLiteral("strings"));
    REQUIRE(ausJson(alt, gelesen));
    CHECK(gelesen.saiten == Saitensatz{});
    QJsonObject wild = json;
    wild[QStringLiteral("strings")] =
        QJsonObject{{QStringLiteral("open"), QJsonArray{40, QStringLiteral("x"), 300}}, {QStringLiteral("capo"), 99}};
    REQUIRE(ausJson(wild, gelesen));
    CHECK(gelesen.saiten.leer == std::vector<int>{40, 127});  // was keine Zahl ist, faellt weg
    CHECK(gelesen.saiten.capo == kCapoMax);
    CHECK(gelesen.saiten.wahl == Saitenwahl::JeKlang);  // fehlt: Per chord

    // --- eine Fassung von morgen liest das Programm nicht
    QJsonObject morgen = json;
    morgen[QStringLiteral("version")] = kInstrumentFassung + 1;
    bool zuNeu = false;
    CHECK_FALSE(ausJson(morgen, gelesen, &zuNeu));
    CHECK(zuNeu);
}

TEST_CASE("Loop Saiten: .lvloop Fassung 8 — die Saite einer Note und String choice am Muster")
{
    const Loop loop = gitarrenLoop();
    const QJsonObject json = alsJson(loop);
    CHECK(json.value(QStringLiteral("version")).toInt() == 8);

    // --- die Saite steht als fuenfter Wert, nur wenn die Note eine traegt
    const QJsonArray noten = notenIn(json);
    REQUIRE(noten.size() == 4);
    CHECK(noten.at(0).toArray() == QJsonArray{0, 4, 40, 1.0, 0});
    CHECK(noten.at(1).toArray() == QJsonArray{0, 4, 47, 0.8, 1});
    CHECK(noten.at(2).toArray().size() == 4);  // automatisch: keine Saite
    CHECK(noten.at(3).toArray().at(4).toInt() == 5);
    const QJsonObject muster = json.value(QStringLiteral("patterns")).toArray().at(0).toObject();
    CHECK(muster.value(QStringLiteral("stringChoice")).toString() == QStringLiteral("followHand"));

    Loop gelesen;
    QStringList bericht;
    REQUIRE(ausJson(json, gelesen, {}, &bericht));
    CHECK(bericht.isEmpty());
    CHECK(gelesen == loop);
    CHECK(gelesen.muster[0].noten[2].saite == -1);
    CHECK(gelesen.muster[0].saitenwahl == Saitenwahl::Hand);

    // --- Per chord ist die Vorgabe und steht nicht in der Datei
    Loop proAkkord = loop;
    proAkkord.muster[0].saitenwahl = Saitenwahl::JeKlang;
    const QJsonObject ohne = alsJson(proAkkord);
    CHECK_FALSE(ohne.value(QStringLiteral("patterns")).toArray().at(0).toObject().contains(QStringLiteral("stringChoice")));
    REQUIRE(ausJson(ohne, gelesen));
    CHECK(gelesen.muster[0].saitenwahl == Saitenwahl::JeKlang);

    // --- eine Datei der Fassung 7: vier Werte je Note, keine Regel — alles automatisch, Per chord
    QJsonObject alt = ohne;
    alt[QStringLiteral("version")] = 7;
    QJsonObject altMuster = alt.value(QStringLiteral("patterns")).toArray().at(0).toObject();
    QJsonArray vier;
    for (const QJsonValue& n : notenIn(ohne))
    {
        QJsonArray zeile = n.toArray();
        while (zeile.size() > 4) zeile.removeLast();
        vier.append(zeile);
    }
    altMuster[QStringLiteral("notes")] = vier;
    alt[QStringLiteral("patterns")] = QJsonArray{altMuster};
    REQUIRE(ausJson(alt, gelesen, {}, &bericht));
    CHECK(bericht.isEmpty());
    for (const Note& note : gelesen.muster[0].noten) CHECK(note.saite == -1);

    // --- Grenzen: eine Saite ausserhalb gilt als keine; ein Drumkit hat keine Regel
    Loop wild = loop;
    wild.muster[0].noten[0].saite = kSaitenMax;
    wild.muster[0].noten[1].saite = -7;
    wild.muster[0].saitenwahl = static_cast<Saitenwahl>(5);
    wild.richte();
    CHECK(wild.muster[0].noten[0].saite == -1);
    CHECK(wild.muster[0].noten[1].saite == -1);
    CHECK(wild.muster[0].saitenwahl == Saitenwahl::JeKlang);
    Loop trommel = loop;
    trommel.muster[0].art = MusterArt::Drumkit;
    trommel.richte();
    CHECK(trommel.muster[0].saitenwahl == Saitenwahl::JeKlang);
    CHECK(trommel.muster[0].noten.empty());

    // --- als Datei
    QTemporaryDir ordner;
    REQUIRE(ordner.isValid());
    const QString pfad = ordner.filePath(QStringLiteral("riff.lvloop"));
    REQUIRE(speichereLoop(loop, pfad));
    Loop ausDatei;
    REQUIRE(ladeLoop(pfad, ausDatei));
    CHECK(ausDatei == loop);
}

TEST_CASE("Loop Saiten: jede mitgelieferte Datei liest sich ohne Saiten und schreibt sich gleich")
{
    // --- Instrumente: keines traegt bis Stufe 23g Saiten
    const QDir instrumente(repoWurzel().filePath(QStringLiteral("asset/loop/instruments")));
    const QStringList dateien = instrumente.entryList({QStringLiteral("*.lvinst")}, QDir::Files, QDir::Name);
    REQUIRE(dateien.size() >= 93);
    for (const QString& datei : dateien)
    {
        CAPTURE(datei.toStdString());
        Instrument instrument;
        REQUIRE(ladeInstrument(instrumente.filePath(datei), instrument));
        CHECK_FALSE(instrument.saiten.istSaiteninstrument());
        CHECK_FALSE(alsJson(instrument).contains(QStringLiteral("strings")));
        Instrument nochmal;
        nochmal.name = instrument.name;
        REQUIRE(ausJson(alsJson(instrument), nochmal));
        CHECK(nochmal == instrument);
    }

    // --- Loops: keine Note traegt eine Saite, kein Muster eine Regel
    const QStringList loops = mitgelieferteLoops();
    REQUIRE(loops.size() >= 38);
    int noten = 0;
    for (const QString& pfad : loops)
    {
        CAPTURE(pfad.toStdString());
        Loop loop;
        REQUIRE(ladeLoop(pfad, loop));
        for (const Muster& m : loop.muster)
        {
            CHECK(m.saitenwahl == Saitenwahl::JeKlang);
            for (const Note& note : m.noten)
            {
                CHECK(note.saite == -1);
                ++noten;
            }
        }
        const QString basis = QFileInfo(pfad).absolutePath();
        Loop nochmal;
        REQUIRE(ausJson(alsJson(loop, basis), nochmal, basis));
        CHECK(nochmal == loop);
    }
    MESSAGE(dateien.size() << " Instrumente, " << loops.size() << " Loops mit " << noten << " Noten geprueft");
    CHECK(noten > 0);
}
