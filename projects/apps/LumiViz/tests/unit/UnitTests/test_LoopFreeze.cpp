/**
 ****************************************************************************************
 * @file   test_LoopFreeze.cpp
 * @brief  Loop: Freeze — der Fingerabdruck eines Clips und die Klangfassung
 *         (docs/audio/Spielweisen_Konzept.md §4.4, Stufe 30d; Entscheide F1 a, F3 a,
 *         WX5 a, FZ1 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton und ohne Fenster; die mitgelieferten Instrumente brauchen eine
 * `QCoreApplication`, eigene Dateien liegen in einem Wegwerf-Ordner.
 *
 * Der erste Fall haelt die Klangfassung an `werte/loop_werte.txt` fest: in
 * `werte/klangfassung.txt` steht die Fassung und je Eintrag von `loop_werte.txt`
 * ein Fingerabdruck seiner **Eingaben** (Instrument-Datei mit Samples und
 * Zeichnungen, Loop-Datei mit ihren Instrumenten und Klangdateien — aus dem
 * Inhalt, nicht aus Pfad oder Aenderungszeit) mit den Werten. Haben Eingaben
 * denselben Abdruck, aber andere Werte, hat sich der Rechenkern geaendert: dann
 * muss `kKlangfassung` wachsen. Geaenderte oder neue mitgelieferte Dateien
 * verlangen das nicht (das sieht der Abdruck eines Clips selbst).
 * `LUMIVIZ_KLANGFASSUNG_SCHREIBEN=1` schreibt die Datei neu.
 ****************************************************************************************
 */

#include <doctest.h>

#include "LoopOhneEngine.hpp"
#include "loop/Aufnahme.hpp"  // schreibeWav
#include "loop/Freeze.hpp"
#include "loop/Instrument.hpp"
#include "loop/Kit.hpp"
#include "loop/Loop.hpp"
#include "loop/Sample.hpp"
#include "shape/Kontur.hpp"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QTemporaryDir>
#include <QTextStream>

#include <map>

using namespace lumi::loop;

namespace
{

constexpr int kRate = 44100;

struct FreezeApp
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QCoreApplication app{argc, argv};
};

QString werteOrdner()
{
    return QFileInfo(QString::fromLocal8Bit(__FILE__)).absoluteDir().filePath(QStringLiteral("werte"));
}

/// Der Inhalt einer Datei, ohne Wagenruecklauf (Git kann die Zeilenenden tauschen).
QByteArray inhalt(const QString& pfad)
{
    QFile datei(pfad);
    if (pfad.isEmpty() || !datei.open(QIODevice::ReadOnly)) return QByteArrayLiteral("-");
    QByteArray roh = datei.readAll();
    roh.replace('\r', QByteArray());
    return roh;
}

/// Die Eingaben eines Instruments: es selbst, seine Aufnahmen und Zeichnungen — aus dem Inhalt.
QByteArray instrumentEingaben(const Instrument& instrument)
{
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(QJsonDocument(alsJson(instrument)).toJson(QJsonDocument::Compact));
    for (const Schicht& schicht : instrument.schichten)
    {
        if (schicht.schwingung == Schwingung::Sample) hash.addData(inhalt(sampleDatei(schicht.sample)));
        if (schicht.schwingung == Schwingung::Form)
        {
            lumi::shape::Zeichnung zeichnung;
            if (lumi::shape::holeZeichnung(schicht.form, zeichnung))
                hash.addData(QJsonDocument(lumi::shape::alsJson(zeichnung)).toJson(QJsonDocument::Compact));
        }
    }
    return hash.result().toHex();
}

/// Die Eingaben eines Loops: die Datei, ihre Instrumente und Klangdateien — aus dem Inhalt.
QByteArray loopEingaben(const QString& pfad)
{
    Loop loop;
    if (!ladeLoop(pfad, loop)) return QByteArrayLiteral("-");
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(inhalt(pfad));
    for (const Muster& muster : loop.muster)
    {
        if (muster.art == MusterArt::Instrument)
        {
            Instrument instrument;
            hash.addData(holeInstrument(muster.instrument, instrument) ? instrumentEingaben(instrument) : QByteArrayLiteral("-"));
        }
        for (const Spur& spur : muster.spuren) hash.addData(inhalt(klangDatei(spur.spielt())));
    }
    return hash.result().toHex();
}

struct Eintrag
{
    QByteArray eingaben;
    QString werte;  ///< die Spalten aus loop_werte.txt, wie sie dort stehen
};

/// `loop_werte.txt`: Eintrag -> Werte (als Text).
std::map<QString, QString> liesWerte()
{
    std::map<QString, QString> werte;
    QFile datei(QDir(werteOrdner()).filePath(QStringLiteral("loop_werte.txt")));
    REQUIRE(datei.open(QIODevice::ReadOnly | QIODevice::Text));
    QTextStream ein(&datei);
    while (!ein.atEnd())
    {
        const QString zeile = ein.readLine();
        if (zeile.startsWith(QLatin1Char('#')) || zeile.trimmed().isEmpty()) continue;
        const int tab = zeile.indexOf(QLatin1Char('\t'));
        if (tab > 0) werte[zeile.left(tab)] = zeile.mid(tab + 1);
    }
    return werte;
}

/// Je Eintrag die Eingaben, so wie sie jetzt sind.
std::map<QString, Eintrag> eintraegeJetzt()
{
    std::map<QString, QByteArray> instrumentVon;
    for (const Instrument& instrument : instrumente()) instrumentVon[instrument.name] = instrumentEingaben(instrument);
    const QDir loops(looptest::repoWurzel().filePath(QStringLiteral("asset/loop")));
    std::map<QString, Eintrag> alle;
    for (const auto& [name, werte] : liesWerte())
    {
        Eintrag e;
        e.werte = werte;
        if (name.startsWith(QLatin1String("inst:")))
        {
            const QString instrument = name.section(QLatin1Char(':'), 1, 1);
            const auto ort = instrumentVon.find(instrument);
            e.eingaben = ort != instrumentVon.end() ? ort->second : QByteArrayLiteral("-");
        }
        else if (name.startsWith(QLatin1String("loop:")))
            e.eingaben = loopEingaben(loops.filePath(name.mid(5)));
        alle[name] = e;
    }
    return alle;
}

QString klangfassungDatei()
{
    return QDir(werteOrdner()).filePath(QStringLiteral("klangfassung.txt"));
}

Loop probeLoop()
{
    Muster grund;
    grund.name = QStringLiteral("Grund");
    grund.takte = 1;
    grund.spuren = {{QStringLiteral("Kick"), QStringLiteral("kit:kick"), 1.0, 0.0, false}};
    grund.zeilen = {std::vector<double>(16, 0.0)};
    grund.zeilen[0][0] = 1.0;
    grund.zeilen[0][8] = 0.7;
    Muster keys;
    keys.name = QStringLiteral("Keys");
    keys.art = MusterArt::Instrument;
    keys.instrument = QStringLiteral("piano");
    keys.takte = 1;
    keys.noten = {Note{0, 4, 60}, Note{8, 4, 64}};
    Loop loop;
    loop.muster = {grund, keys};
    loop.kanaele = {{QStringLiteral("Drums"), true, {{QStringLiteral("Grund"), 0, 2}}},
                    {QStringLiteral("Keys"), true, {{QStringLiteral("Keys"), 1, 1}, {QStringLiteral("Keys"), 4, 2}}}};
    loop.richte();
    return loop;
}

}  // namespace

TEST_SUITE("Loop Freeze")
{
    TEST_CASE("Loop Freeze 30d: die Klangfassung haelt mit loop_werte.txt Schritt")
    {
        FreezeApp app;
        const std::map<QString, Eintrag> jetzt = eintraegeJetzt();
        REQUIRE(jetzt.size() > 300);
        for (const auto& [name, e] : jetzt)
        {
            CAPTURE(name.toStdString());
            CHECK(e.eingaben != QByteArrayLiteral("-"));
        }

        if (qEnvironmentVariableIntValue("LUMIVIZ_KLANGFASSUNG_SCHREIBEN") == 1)
        {
            QFile datei(klangfassungDatei());
            REQUIRE(datei.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate));
            QTextStream aus(&datei);
            aus << "# Loop: die Klangfassung (Freeze, Stufe 30d) und je Eintrag aus loop_werte.txt der Abdruck\n";
            aus << "# seiner Eingaben mit seinen Werten — geschrieben von test_LoopFreeze.cpp (LUMIVIZ_KLANGFASSUNG_SCHREIBEN=1)\n";
            aus << "klangfassung\t" << kKlangfassung << "\n";
            for (const auto& [name, e] : jetzt) aus << name << '\t' << QString::fromLatin1(e.eingaben) << '\t' << e.werte << '\n';
            MESSAGE("klangfassung.txt neu geschrieben: Fassung " << kKlangfassung << ", " << jetzt.size() << " Eintraege");
            return;
        }

        QFile datei(klangfassungDatei());
        REQUIRE_MESSAGE(datei.open(QIODevice::ReadOnly | QIODevice::Text),
                        "werte/klangfassung.txt fehlt — mit LUMIVIZ_KLANGFASSUNG_SCHREIBEN=1 schreiben");
        QTextStream ein(&datei);
        int fassung = -1;
        std::map<QString, Eintrag> damals;
        while (!ein.atEnd())
        {
            const QStringList teile = ein.readLine().split(QLatin1Char('\t'));
            if (teile.size() == 2 && teile[0] == QLatin1String("klangfassung")) fassung = teile[1].toInt();
            else if (teile.size() >= 3 && !teile[0].startsWith(QLatin1Char('#')))
                damals[teile[0]] = Eintrag{teile[1].toLatin1(), teile.mid(2).join(QLatin1Char('\t'))};
        }
        REQUIRE(fassung > 0);
        REQUIRE_MESSAGE(fassung <= kKlangfassung, "klangfassung.txt nennt eine hoehere Fassung als das Programm");
        REQUIRE_MESSAGE(fassung == kKlangfassung, "kKlangfassung ist gewachsen — klangfassung.txt mit "
                                                  "LUMIVIZ_KLANGFASSUNG_SCHREIBEN=1 neu schreiben");

        // gleiche Eingaben, andere Werte: der Rechenkern hat sich geaendert
        QStringList geaendert;
        int verglichen = 0;
        for (const auto& [name, e] : jetzt)
        {
            const auto ort = damals.find(name);
            if (ort == damals.end() || ort->second.eingaben != e.eingaben) continue;
            ++verglichen;
            if (ort->second.werte != e.werte) geaendert << name;
        }
        MESSAGE(verglichen << " Eintraege mit gleichen Eingaben verglichen, " << geaendert.size() << " anders");
        CHECK(verglichen > 300);  // sonst prueft der Fall nichts
        CHECK_MESSAGE(geaendert.isEmpty(), "loop_werte.txt hat sich bei gleichen Eingaben geaendert ("
                                               << geaendert.mid(0, 5).join(QStringLiteral(", ")).toStdString()
                                               << " …) — kKlangfassung in loop/Freeze.hpp hochzaehlen, dann "
                                                  "LUMIVIZ_KLANGFASSUNG_SCHREIBEN=1");
    }

    TEST_CASE("Loop Freeze 30d: der Abdruck eines Clips — was klingt, aendert ihn, was live bleibt, nicht")
    {
        FreezeApp app;
        const SaitenVon saiten = saitenAusInstrumenten();
        const Loop loop = probeLoop();
        const ClipOrt keys{1, 1};
        const QByteArray grund = clipAbdruck(loop, keys, kRate, saiten);
        REQUIRE(grund.size() == 40);
        CHECK(clipAbdruck(loop, keys, kRate, saiten) == grund);  // fest
        CHECK(clipAbdruck(loop, ClipOrt{1, 2}, kRate, saiten).isEmpty());  // dort liegt keiner
        CHECK(clipAbdruck(loop, ClipOrt{0, 0}, kRate, saiten) != grund);

        const auto mit = [&](auto aendere, ClipOrt ort = ClipOrt{1, 1}) {
            Loop neu = loop;
            aendere(neu);
            neu.richte();
            return clipAbdruck(neu, ort, kRate, saiten);
        };
        SUBCASE("live und ohne Klang: gleicher Abdruck")
        {
            CHECK(mit([](Loop& l) { l.kanaele[1].gainDb = 6.0; }) == grund);         // Fader
            CHECK(mit([](Loop& l) { l.kanaele[1].an = false; }) == grund);           // Schalter des Kanals
            CHECK(mit([](Loop& l) { l.laut = 0.9; }) == grund);                      // Lautstaerke des Loops
            CHECK(mit([](Loop& l) { l.limiter = false; }) == grund);                 // Limiter
            CHECK(mit([](Loop& l) { l.tags.titel = QStringLiteral("Neu"); }) == grund);
            CHECK(mit([](Loop& l) { l.kanaele[1].name = QStringLiteral("Piano"); }) == grund);
            CHECK(mit([](Loop& l) { l.kanaele[0].clips[0].wiederholungen = 3; }) == grund);  // ein anderer Kanal
            // derselbe Clip an anderer Stelle (der zweite liegt ab Takt 4) und unter anderem Namen
            Loop zwei = loop;
            zwei.kanaele[1].clips[1].wiederholungen = 1;
            zwei.richte();
            CHECK(clipAbdruck(zwei, ClipOrt{1, 4}, kRate, saiten) == grund);
            Loop umbenannt = loop;
            umbenannt.muster[1].name = QStringLiteral("Akkorde");
            for (LoopClip& c : umbenannt.kanaele[1].clips) c.muster = QStringLiteral("Akkorde");
            umbenannt.richte();
            CHECK(clipAbdruck(umbenannt, keys, kRate, saiten) == grund);
        }
        SUBCASE("was klingt: anderer Abdruck")
        {
            CHECK(mit([](Loop& l) { l.muster[1].noten[0].hoehe = 62; }) != grund);
            CHECK(mit([](Loop& l) { l.muster[1].laut = 0.5; }) != grund);
            CHECK(mit([](Loop& l) { l.bpm = 100.0; }) != grund);
            CHECK(mit([](Loop& l) { l.swing = 0.4; }) != grund);
            CHECK(mit([](Loop& l) { l.schritteJeSchlag = 3; }) != grund);
            CHECK(mit([](Loop& l) { l.muster[1].instrument = QStringLiteral("organ"); }) != grund);
            CHECK(mit([](Loop& l) { l.kanaele[1].clips[0].wiederholungen = 2; }) != grund);
            CHECK(mit([](Loop& l) { l.kanaele[1].clips[0].expression = {SpurPunkt{0.0, 0.5}}; }) != grund);
            CHECK(mit([](Loop& l) {
                      ClipHall hall;
                      hall.an = true;
                      l.kanaele[1].clips[0].effekte.reihe = {hall};
                  }) != grund);
            CHECK(clipAbdruck(loop, keys, 48000, saiten) != grund);  // eine andere Rate
        }
    }

    TEST_CASE("Loop Freeze 30d, FZ2 a: eine geaenderte Datei aendert den Abdruck, eine verschobene nicht")
    {
        FreezeApp app;
        QTemporaryDir tmp;
        REQUIRE(tmp.isValid());
        const SaitenVon saiten = saitenAusInstrumenten();
        Loop loop = probeLoop();

        // ein eigenes Instrument als Datei
        const QString inst = QDir(tmp.path()).filePath(QStringLiteral("eigenes.lvinst"));
        Instrument instrument;
        instrument.titel = QStringLiteral("Eigenes");
        REQUIRE(speichereInstrument(instrument, inst));
        loop.muster[1].instrument = inst;
        const QByteArray vorher = clipAbdruck(loop, ClipOrt{1, 1}, kRate, saiten);
        REQUIRE_FALSE(vorher.isEmpty());
        instrument.laut = 0.5;
        REQUIRE(speichereInstrument(instrument, inst));
        CHECK(clipAbdruck(loop, ClipOrt{1, 1}, kRate, saiten) != vorher);

        // eine Klangdatei fuer die Drum-Spur
        const QString wav = QDir(tmp.path()).filePath(QStringLiteral("klopf.wav"));
        REQUIRE(schreibeWav(wav, std::vector<float>(200, 0.25f), kRate));
        loop.muster[0].spuren[0].klang = wav;
        const QByteArray drums = clipAbdruck(loop, ClipOrt{0, 0}, kRate, saiten);
        REQUIRE_FALSE(drums.isEmpty());
        REQUIRE(schreibeWav(wav, std::vector<float>(400, 0.25f), kRate));
        const QByteArray drumsNeu = clipAbdruck(loop, ClipOrt{0, 0}, kRate, saiten);
        CHECK(drumsNeu != drums);

        // --- FZ2 a: derselbe Inhalt an anderem Ort oder neu geschrieben: derselbe Abdruck
        const QByteArray instNeu = clipAbdruck(loop, ClipOrt{1, 1}, kRate, saiten);
        const QString anderswo = QDir(tmp.path()).filePath(QStringLiteral("anderswo"));
        REQUIRE(QDir().mkpath(anderswo));
        const QString instKopie = QDir(anderswo).filePath(QStringLiteral("kopie.lvinst"));
        const QString wavKopie = QDir(anderswo).filePath(QStringLiteral("kopie.wav"));
        REQUIRE(QFile::copy(inst, instKopie));
        REQUIRE(QFile::copy(wav, wavKopie));
        Loop verschoben = loop;
        verschoben.muster[1].instrument = instKopie;
        verschoben.muster[0].spuren[0].klang = wavKopie;
        CHECK(clipAbdruck(verschoben, ClipOrt{1, 1}, kRate, saiten) == instNeu);
        CHECK(clipAbdruck(verschoben, ClipOrt{0, 0}, kRate, saiten) == drumsNeu);
        // dieselben Bytes noch einmal geschrieben (neue Aenderungszeit)
        REQUIRE(schreibeWav(wav, std::vector<float>(400, 0.25f), kRate));
        CHECK(clipAbdruck(loop, ClipOrt{0, 0}, kRate, saiten) == drumsNeu);
        // eine Datei, die fehlt, gibt einen eigenen Abdruck — und keinen leeren
        Loop fehlt = loop;
        fehlt.muster[0].spuren[0].klang = QDir(tmp.path()).filePath(QStringLiteral("fehlt.wav"));
        const QByteArray ohne = clipAbdruck(fehlt, ClipOrt{0, 0}, kRate, saiten);
        CHECK(ohne.size() == 40);
        CHECK(ohne != drumsNeu);
    }
}
