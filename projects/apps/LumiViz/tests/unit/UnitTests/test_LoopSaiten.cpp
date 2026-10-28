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
#include "loop/Rechenkern.hpp"
#include "loop/Saiten.hpp"
#include "loop/Saitenplan.hpp"

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
    // die Mensuren (S-U3 a, S-K7 a; Saiten_Konzept §6b)
    std::vector<double> mensuren;
    for (const Stimmungsvorlage& v : vorlagen) mensuren.push_back(v.mensurMm);
    CHECK(mensuren == std::vector<double>{648.0, 648.0, 864.0, 864.0, 381.0, 327.0});

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
    CHECK(kInstrumentFassung >= 10);  // die Saiten gibt es seit Fassung 10; geschrieben wird die juengste (S134: 11)
    CHECK(json.value(QStringLiteral("version")).toInt() == kInstrumentFassung);
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
    CHECK(json.value(QStringLiteral("version")).toInt() == 14);  // seit Fassung 8; geschrieben wird die juengste (S134: 9, S136: 10, S139: 11, S140: 12, S144: 13)

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

namespace
{
/// Die mitgelieferten Instrumente mit Tuning seit Stufe 23g (S133, S-G1 a; `asset/loop/make_saiten_instrumente.py` — das Clavinet hat keine: eine Saite je Taste).
const QStringList kMitSaiten = {QStringLiteral("guitar.lvinst"),       QStringLiteral("nylon_guitar.lvinst"),
                                QStringLiteral("clean_guitar.lvinst"), QStringLiteral("muted_guitar.lvinst"),
                                QStringLiteral("overdrive_guitar.lvinst"), QStringLiteral("upright_bass.lvinst"),
                                QStringLiteral("slap_bass.lvinst"),
                                // neu (S-G3 a)
                                QStringLiteral("jazz_guitar.lvinst"),  QStringLiteral("finger_bass.lvinst"),
                                QStringLiteral("ukulele.lvinst"),      QStringLiteral("drop_d_guitar.lvinst"),
                                QStringLiteral("pizzicato_violin.lvinst"), QStringLiteral("piezo_guitar.lvinst")};
}  // namespace

TEST_CASE("Loop Saiten: jede mitgelieferte Datei liest sich und schreibt sich gleich — Saiten nur die aus 23g")
{
    // --- Instrumente: bis Stufe 23g trug keines Saiten, seit 23g genau die sieben in kMitSaiten
    const QDir instrumente(repoWurzel().filePath(QStringLiteral("asset/loop/instruments")));
    const QStringList dateien = instrumente.entryList({QStringLiteral("*.lvinst")}, QDir::Files, QDir::Name);
    REQUIRE(dateien.size() >= 93);
    for (const QString& datei : dateien)
    {
        CAPTURE(datei.toStdString());
        Instrument instrument;
        REQUIRE(ladeInstrument(instrumente.filePath(datei), instrument));
        const bool soll = kMitSaiten.contains(datei);
        CHECK(instrument.saiten.istSaiteninstrument() == soll);
        CHECK(alsJson(instrument).contains(QStringLiteral("strings")) == soll);
        Instrument nochmal;
        nochmal.name = instrument.name;
        REQUIRE(ausJson(alsJson(instrument), nochmal));
        CHECK(nochmal == instrument);
    }

    // --- Loops: keine Note traegt eine Saite, kein Muster eine Regel — ausser tapping_demo
    // (S136, S-W22 a): der Lauf liegt mit Absicht ganz auf der hohen E-Saite; und tabs_demo
    // (S138, 26h): Griffbilder und die Lead-Linie setzen ihre Saiten (S-M2 a, S-T5 a)
    const QStringList loops = mitgelieferteLoops();
    REQUIRE(loops.size() >= 40);
    int noten = 0;
    for (const QString& pfad : loops)
    {
        CAPTURE(pfad.toStdString());
        Loop loop;
        REQUIRE(ladeLoop(pfad, loop));
        const QString name = QFileInfo(pfad).fileName();
        const bool tapping = name == QStringLiteral("tapping_demo.lvloop");
        const bool tabs = name == QStringLiteral("tabs_demo.lvloop");
        for (const Muster& m : loop.muster)
        {
            CHECK(m.saitenwahl == Saitenwahl::JeKlang);
            const bool hoheE = tapping && m.name == QStringLiteral("Tapping");
            const bool gesetzt = tabs && (m.name == QStringLiteral("Rhythm") || m.name == QStringLiteral("Lead"));
            for (const Note& note : m.noten)
            {
                if (gesetzt)
                    CHECK(note.saite >= 0);
                else
                    CHECK(note.saite == (hoheE ? 5 : -1));
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

// =============================================================================
// Stufe 23b — Saitenwahl und Abdaempfen beim Planen (S-M2b, S-M3 a, S-A1 a)
// =============================================================================

namespace
{

const Saitensatz& gitarre()
{
    static const Saitensatz satz{stimmungsvorlagen()[0].leer, 0, Saitenwahl::JeKlang};
    return satz;
}

/// Ein Muster aus Noten, gerichtet in einem Loop (120 BPM, vier Schritte je Schlag).
Muster musterAus(std::vector<Note> noten, Saitenwahl wahl = Saitenwahl::JeKlang)
{
    Loop loop;
    Muster m;
    m.name = QStringLiteral("M");
    m.art = MusterArt::Instrument;
    m.instrument = QStringLiteral("gitarre");
    m.takte = 1;
    m.noten = std::move(noten);
    m.saitenwahl = wahl;
    loop.muster.push_back(m);
    loop.richte();
    return loop.muster[0];
}

std::vector<int> saitenVon(const std::vector<Griff>& griffe)
{
    std::vector<int> s;
    for (const Griff& g : griffe) s.push_back(g.saite);
    return s;
}

/// Eine Auskunft, die nur `gitarre` mit Saiten kennt.
InstrumentVon nurGitarre()
{
    return [](const QString& name) { return name == QStringLiteral("gitarre") ? gitarre() : Saitensatz{}; };
}

}  // namespace

TEST_CASE("Loop Saiten 23b: der Name eines Tons traegt Saite und Ende — und zerlegt sich wieder")
{
    const TonAngabe ohne{QStringLiteral("piano"), 60, 500};
    CHECK(tonKlang(ohne) == QStringLiteral("inst:piano:60:500"));  // wie bis Stufe 22
    CHECK(tonKlang(ohne) == tonKlang(QStringLiteral("piano"), 60, 500));
    const TonAngabe mit{QStringLiteral("C:/eigene/gitarre.lvinst"), 64, 250, 5, 1750};
    CHECK(tonKlang(mit) == QStringLiteral("inst:C:/eigene/gitarre.lvinst:64:250:s5:e1750"));
    TonAngabe gelesen;
    REQUIRE(zerlegeTon(tonKlang(mit), gelesen));
    CHECK(gelesen == mit);
    REQUIRE(zerlegeTon(tonKlang(ohne), gelesen));
    CHECK(gelesen == ohne);
    // nur Saite, nur Ende
    REQUIRE(zerlegeTon(QStringLiteral("inst:user:s3:40:100:s0"), gelesen));
    CHECK(gelesen == TonAngabe{QStringLiteral("user:s3"), 40, 100, 0, 0});
    REQUIRE(zerlegeTon(QStringLiteral("inst:piano:60:500:e300"), gelesen));
    CHECK(gelesen == TonAngabe{QStringLiteral("piano"), 60, 500, -1, 300});
    // der alte Weg nimmt den neuen Namen auch — Saite und Ende fallen weg
    QString instrument;
    int hoehe = 0;
    int dauer = 0;
    REQUIRE(zerlegeTon(tonKlang(mit), instrument, hoehe, dauer));
    CHECK(instrument == mit.instrument);
    CHECK(hoehe == 64);
    CHECK(dauer == 250);
    // kaputt
    CHECK_FALSE(zerlegeTon(QStringLiteral("inst:piano:60:500:sx"), gelesen));
    CHECK_FALSE(zerlegeTon(QStringLiteral("kit:kick"), gelesen));
}

TEST_CASE("Loop Saiten 23b: Per chord — offene Akkorde, belegte Saiten, gesetzte Saiten, die Grenzen")
{
    CHECK(bundAuf(gitarre(), 1, 47) == 2);
    CHECK(greifbarAuf(gitarre(), 5, 64 + kBuendeMax));
    CHECK_FALSE(greifbarAuf(gitarre(), 5, 64 + kBuendeMax + 1));
    CHECK_FALSE(greifbarAuf(gitarre(), 0, 39));
    CHECK_FALSE(greifbarAuf(gitarre(), 6, 64));

    // --- E-Dur offen: E2 B2 E3 G#3 B3 E4 — jede Saite einmal, Buende 0 2 2 1 0 0
    std::vector<Griff> griffe = greifeNoten(musterAus({{0, 4, 40}, {0, 4, 47}, {0, 4, 52}, {0, 4, 56}, {0, 4, 59}, {0, 4, 64}}), gitarre());
    CHECK(saitenVon(griffe) == std::vector<int>{0, 1, 2, 3, 4, 5});
    std::vector<int> buende;
    for (const Griff& g : griffe)
    {
        CHECK(g.greifbar);
        buende.push_back(g.bund);
    }
    CHECK(buende == std::vector<int>{0, 2, 2, 1, 0, 0});
    // --- C-Dur offen: C3 E3 G3 C4 E4 — auf der A- bis zur hohen E-Saite, Buende 3 2 0 1 0
    griffe = greifeNoten(musterAus({{0, 4, 48}, {0, 4, 52}, {0, 4, 55}, {0, 4, 60}, {0, 4, 64}}), gitarre());
    CHECK(saitenVon(griffe) == std::vector<int>{1, 2, 3, 4, 5});

    // --- eine gehaltene Note belegt ihre Saite: E4 offen klingt, F4 muss auf die H-Saite
    griffe = greifeNoten(musterAus({{0, 4, 64}, {1, 1, 65}}), gitarre());
    CHECK(saitenVon(griffe) == std::vector<int>{5, 4});
    CHECK(griffe[1].bund == 6);
    // ist sie vorbei, ist die Saite wieder frei
    griffe = greifeNoten(musterAus({{0, 1, 64}, {1, 1, 65}}), gitarre());
    CHECK(saitenVon(griffe) == std::vector<int>{5, 5});

    // --- eine gesetzte Saite gilt, solange sie greifbar ist (S-M2 a)
    Note gesetzt{0, 1, 64, 1.0, 3};  // E4 auf der G-Saite, Bund 9
    griffe = greifeNoten(musterAus({gesetzt}), gitarre());
    CHECK(griffe[0] == Griff{3, 9, true});
    gesetzt.hoehe = 30;  // auf keiner Saite greifbar: automatisch, und auch dort nicht
    griffe = greifeNoten(musterAus({gesetzt}), gitarre());
    CHECK_FALSE(griffe[0].greifbar);
    CHECK(griffe[0].saite == 0);  // die naechstbeste: die tiefe E-Saite
    CHECK(griffe[0].bund == 30 - 40);

    // --- mehr Noten zugleich als Saiten: eine Ukulele mit fuenf Toenen — vier gegriffen, einer nicht
    const Saitensatz ukulele{stimmungsvorlagen()[4].leer, 0, Saitenwahl::JeKlang};
    griffe = greifeNoten(musterAus({{0, 2, 60}, {0, 2, 64}, {0, 2, 67}, {0, 2, 69}, {0, 2, 72}}), ukulele);
    int greifbar = 0;
    std::vector<bool> benutzt(4, false);
    for (const Griff& g : griffe)
        if (g.greifbar)
        {
            ++greifbar;
            CHECK_FALSE(benutzt[static_cast<std::size_t>(g.saite)]);  // verschiedene Saiten
            benutzt[static_cast<std::size_t>(g.saite)] = true;
        }
    CHECK(greifbar == 4);

    // --- das Capo hebt jede leere Saite
    Saitensatz mitCapo = gitarre();
    mitCapo.capo = 2;
    griffe = greifeNoten(musterAus({{0, 1, 42}}), mitCapo);  // F#2: die leere tiefe E-Saite mit Capo 2
    CHECK(griffe[0] == Griff{0, 0, true});
    griffe = greifeNoten(musterAus({{0, 1, 41}}), mitCapo);  // F2 liegt darunter
    CHECK_FALSE(griffe[0].greifbar);

    // --- ohne Saiten: lauter leere Griffe
    griffe = greifeNoten(musterAus({{0, 1, 60}}), Saitensatz{});
    CHECK(griffe == std::vector<Griff>{Griff{}});
}

TEST_CASE("Loop Saiten 23b: Follow hand bleibt in der Lage, Per chord nimmt die tiefste")
{
    // C#5 nur hoch greifbar (hohe E-Saite, Bund 9), dann F#4: Per chord nimmt die
    // tiefste Lage (hohe E-Saite, Bund 2), Follow hand die naechste zur Hand
    // (H-Saite, Bund 7 — G-Saite Bund 11 ist gleich nah, aber hoeher)
    const std::vector<Note> melodie = {{0, 1, 73}, {1, 1, 66}};
    std::vector<Griff> akkord = greifeNoten(musterAus(melodie, Saitenwahl::JeKlang), gitarre());
    CHECK(akkord[0] == Griff{5, 9, true});
    CHECK(akkord[1] == Griff{5, 2, true});
    std::vector<Griff> hand = greifeNoten(musterAus(melodie, Saitenwahl::Hand), gitarre());
    CHECK(hand[0] == Griff{5, 9, true});
    CHECK(hand[1] == Griff{4, 7, true});
    // eine leere Saite dazwischen bewegt die Hand nicht
    hand = greifeNoten(musterAus({{0, 1, 73}, {1, 1, 64}, {2, 1, 66}}, Saitenwahl::Hand), gitarre());
    CHECK(hand[1] == Griff{5, 0, true});
    CHECK(hand[2] == Griff{4, 7, true});
}

TEST_CASE("Loop Saiten 23b: die Toene je Vorkommen — das Ende an der naechsten Note derselben Saite")
{
    // zwei E4 auf der hohen E-Saite (gesetzt), 120 BPM, ein Schritt 125 ms, ein Takt 2000 ms
    Loop loop;
    Muster m;
    m.name = QStringLiteral("Riff");
    m.art = MusterArt::Instrument;
    m.instrument = QStringLiteral("gitarre");
    m.takte = 1;
    m.noten = {Note{0, 4, 64, 1.0, 5}, Note{2, 2, 64, 1.0, 5}, Note{8, 1, 40, 1.0}};
    loop.muster.push_back(m);
    Kanal kanal;
    kanal.clips.push_back(LoopClip{});
    kanal.clips[0].muster = m.name;
    kanal.clips[0].wiederholungen = 2;
    loop.kanaele.push_back(kanal);
    loop.richte();
    REQUIRE(loop.durchlaufMs() == 4000.0);

    const std::vector<ClipToene> toene = toeneImKanal(loop, 0, nurGitarre());
    REQUIRE(toene.size() == 1);
    REQUIRE(toene[0].size() == 2);  // zwei Wiederholungen
    // die erste E4 endet, wo die zweite anschlaegt (250 ms), die zweite an der ersten der
    // naechsten Wiederholung — nach der letzten an der ersten des naechsten Durchlaufs
    CHECK(toene[0][0][0] == TonAngabe{QStringLiteral("gitarre"), 64, 500, 5, 250});
    // seit Stufe 24b (S-W10 a, Wechselschlag): Schritt 2 ist „und" — ein Upstroke
    CHECK(toene[0][0][1] == TonAngabe{QStringLiteral("gitarre"), 64, 250, 5, 1750, Richtung::Auf});
    CHECK(toene[0][1][0] == toene[0][0][0]);
    CHECK(toene[0][1][1] == toene[0][0][1]);
    // die E2 allein auf ihrer Saite: sie endet am naechsten Anschlag, eine Wiederholung spaeter
    CHECK(toene[0][0][2] == TonAngabe{QStringLiteral("gitarre"), 40, 125, 0, 2000});

    // --- klangNamen und plane rechnen dasselbe
    const QStringList namen = loop.klangNamen(nurGitarre());
    CHECK(namen == QStringList{QStringLiteral("inst:gitarre:64:500:s5:e250"), QStringLiteral("inst:gitarre:64:250:s5:u:e1750"),
                               QStringLiteral("inst:gitarre:40:125:s0:e2000")});
    const Ablauf ablauf = plane(loop, 8000, nurGitarre());
    CHECK(ablauf.klangNamen == namen);
    REQUIRE(ablauf.stimmen.size() == 3);
    for (const Stimme& stimme : ablauf.stimmen)
    {
        CHECK(stimme.klang >= 0);
        CHECK(stimme.schlaege.size() == 2);  // je Wiederholung einer
    }

    // --- ohne Auskunft, oder ein Instrument ohne Saiten: wie bis Stufe 22
    const QStringList alt{QStringLiteral("inst:gitarre:64:500"), QStringLiteral("inst:gitarre:64:250"),
                          QStringLiteral("inst:gitarre:40:125")};
    CHECK(loop.klangNamen() == alt);
    CHECK(loop.klangNamen([](const QString&) { return Saitensatz{}; }) == alt);
    CHECK(plane(loop, 8000).klangNamen == alt);
    CHECK(toeneImKanal(loop, 0, {})[0].empty());
}

TEST_CASE("Loop Saiten 23b: ein Ton mit Ende ist dort still — vorher Bit fuer Bit derselbe")
{
    Instrument instrument;
    instrument.schichten[0].schwingung = Schwingung::Saite;
    instrument.schichten[0].ausklangMs = 400.0;
    instrument.saiten = gitarre();
    const int rate = 8000;
    const Klang frei = rendereTon(instrument, TonAngabe{QString(), 64, 500, 5, 0}, rate);
    CHECK(frei.daten == rendereTon(instrument, 64, 500, rate).daten);  // ohne Ende wie bisher
    REQUIRE(frei.frames() > rate * 800 / 1000);

    const Klang gedaempft = rendereTon(instrument, TonAngabe{QString(), 64, 500, 5, 300}, rate);
    CHECK(gedaempft.frames() == rate * 300 / 1000);
    // bis zur Blende dieselben Werte, in der Blende leiser, am Ende fast still
    const std::int64_t blende = rate * kAbdaempfenMs / 1000;
    const std::int64_t ab = gedaempft.frames() - blende;
    bool gleich = true;
    for (std::int64_t f = 0; f < 2 * ab; ++f)
        if (gedaempft.daten[static_cast<std::size_t>(f)] != frei.daten[static_cast<std::size_t>(f)]) gleich = false;
    CHECK(gleich);
    for (std::int64_t f = ab; f < gedaempft.frames(); ++f)
        CHECK(std::abs(gedaempft.daten[static_cast<std::size_t>(2 * f)]) <=
              std::abs(frei.daten[static_cast<std::size_t>(2 * f)]) + 1e-9f);
    CHECK(std::abs(gedaempft.daten[gedaempft.daten.size() - 2]) <= std::abs(frei.daten[gedaempft.daten.size() - 2]) / 4.0f + 1e-9f);
    // ein Ende hinter dem Ausklang aendert nichts
    CHECK(rendereTon(instrument, TonAngabe{QString(), 64, 500, 5, 60000}, rate).daten == frei.daten);
}

TEST_CASE("Loop Saiten 23b: ein mitgelieferter Loop ohne Saiteninstrument plant mit der Auskunft genau wie ohne")
{
    // ein Loop, dessen Instrumente keine Saiten tragen: die Auskunft aendert nichts — dieselben Namen,
    // dieselben Stimmen, dieselben Schlaege. Seit 23g spielen einige Gitarren und Baesse (kMitSaiten):
    // dort tragen die Namen der Toene ihre Saite. Seit S157 (31d-8) spielt grand_piano_demo den Fluegel: die
    // Planung erfaehrt den Hammer ueber die Auskunft (HT5 a), die Namen tragen die Anschlagstufe `:a<stufe>`
    const InstrumentVon saiten = instrumentAuskunft();
    const QStringList loops = mitgelieferteLoops();
    REQUIRE(loops.size() >= 38);
    int ohneSaiten = 0;
    int mitSaiten = 0;
    int mitHammer = 0;
    for (const QString& pfad : loops)
    {
        CAPTURE(pfad.toStdString());
        Loop loop;
        REQUIRE(ladeLoop(pfad, loop));
        bool spieltHammer = false;
        for (const Muster& m : loop.muster)
            if (m.art == MusterArt::Instrument && saiten(m.instrument).hammer) spieltHammer = true;
        if (spieltHammer)
        {
            ++mitHammer;
            bool traegtStufe = false;
            for (const QString& name : loop.klangNamen(saiten))
                if (name.contains(QStringLiteral(":a"))) traegtStufe = true;
            CHECK(traegtStufe);
            continue;
        }
        bool spieltSaiten = false;
        for (const Muster& m : loop.muster)
            if (m.art == MusterArt::Instrument && saiten(m.instrument).saiten.istSaiteninstrument()) spieltSaiten = true;
        if (spieltSaiten)
        {
            ++mitSaiten;
            bool traegtSaite = false;
            for (const QString& name : loop.klangNamen(saiten))
                if (name.contains(QStringLiteral(":s"))) traegtSaite = true;
            CHECK(traegtSaite);
            continue;
        }
        ++ohneSaiten;
        CHECK(loop.klangNamen(saiten) == loop.klangNamen());
        const Ablauf mit = plane(loop, 44100, saiten);
        const Ablauf ohne = plane(loop, 44100);
        CHECK(mit.klangNamen == ohne.klangNamen);
        REQUIRE(mit.stimmen.size() == ohne.stimmen.size());
        bool gleich = true;
        for (std::size_t i = 0; i < mit.stimmen.size(); ++i)
        {
            const Stimme& a = mit.stimmen[i];
            const Stimme& b = ohne.stimmen[i];
            if (a.klang != b.klang || a.schlaege.size() != b.schlaege.size()) gleich = false;
            for (std::size_t k = 0; gleich && k < a.schlaege.size(); ++k)
                if (a.schlaege[k].frame != b.schlaege[k].frame || a.schlaege[k].links != b.schlaege[k].links ||
                    a.schlaege[k].rechts != b.schlaege[k].rechts || a.schlaege[k].bus != b.schlaege[k].bus)
                    gleich = false;
        }
        CHECK(gleich);
    }
    MESSAGE(ohneSaiten << " Loops ohne Saiteninstrument wie ohne Auskunft, " << mitSaiten << " mit Saiten, " << mitHammer
                       << " mit Hammer");
    CHECK(ohneSaiten > 0);
    CHECK(mitSaiten > 0);
    CHECK(mitHammer > 0);
}
