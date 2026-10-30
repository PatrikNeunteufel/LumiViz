/**
 ****************************************************************************************
 * @file   test_LoopAustauschMenue.cpp
 * @brief  Loop, Stufe 33d: Export und Import von Mustern, Clips und Spuren ueber die
 *         Menues der Taktachse (docs/audio/Loop_Konzept.md §4o; PE1, PE3 b, LC1 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Durch das Fenster wie im Programm: Rechtsklick, Menue (ueber den Menuebeobachter),
 * Dateiwahl (ersetzt, `Taktachse::setzeDateiwahlFuerTests`), Meldung (ueber den
 * Dialogbeobachter). Der Benutzerordner liegt in einem Wegwerf-Ordner. Die Taktachse wird
 * nicht gezeigt. `LUMIVIZ_AUSTAUSCH_BILD=<datei.png>` legt ein Bild nach den Importen ab.
 ****************************************************************************************
 */

#include <doctest.h>

#include "UI/widgets/Bedienhilfe.hpp"
#include "loop/Austausch.hpp"
#include "loop/Loop.hpp"
#include "loop/Taktachse.hpp"

#include <QAction>
#include <QApplication>
#include <QContextMenuEvent>
#include <QDialog>
#include <QDir>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QTemporaryDir>

using namespace lumi::loop;

namespace
{

/// Faengt Menues, Meldungen und die Dateiwahl, solange er lebt.
struct Fang
{
    QString waehle;          ///< diesen Eintrag waehlen; leer = keinen
    QStringList texte;       ///< die Eintraege des letzten Menues
    QString antwort;         ///< was die Dateiwahl zurueckgibt (leer = abgebrochen)
    QString vorschlag;       ///< was die Dateiwahl vorschlug
    bool speichern = false;
    QStringList meldungen;   ///< Text jeder Meldung

    Fang()
    {
        lumi::ui::setzeMenueBeobachterFuerTests([this](QMenu& menue) -> QAction* {
            texte.clear();
            QAction* gewaehlt = nullptr;
            for (QAction* a : menue.actions())
            {
                if (a->isSeparator()) continue;
                texte << a->text();
                if (!waehle.isEmpty() && a->text() == waehle) gewaehlt = a;
            }
            return gewaehlt;
        });
        lumi::ui::setzeDialogBeobachterFuerTests([this](QDialog& d) {
            if (auto* box = qobject_cast<QMessageBox*>(&d)) meldungen << box->text();
            return static_cast<int>(QDialog::Accepted);
        });
        Taktachse::setzeDateiwahlFuerTests([this](bool zumSpeichern, const QString&, const QString& v, const QString&) {
            speichern = zumSpeichern;
            vorschlag = v;
            return antwort;
        });
    }
    ~Fang()
    {
        lumi::ui::setzeMenueBeobachterFuerTests({});
        lumi::ui::setzeDialogBeobachterFuerTests({});
        Taktachse::setzeDateiwahlFuerTests({});
        setzeAustauschBasisFuerTests({});
    }
};

void rechts(QWidget& ziel, QPoint ort)
{
    QContextMenuEvent ereignis(QContextMenuEvent::Mouse, ort, ziel.mapToGlobal(ort));
    QCoreApplication::sendEvent(&ziel, &ereignis);
}

/// A (4 Takte) und B (8), Drums: A 0–8, B 8–16; Perc: A 4–8 — wie in test_LoopMenues.
Loop arrangement()
{
    Loop loop;
    Muster a;
    a.name = QStringLiteral("A");
    a.takte = 4;
    a.spuren = {{QStringLiteral("Kick"), QStringLiteral("kit:kick"), 1.0, 0.0, false}};
    Muster b = a;
    b.name = QStringLiteral("B");
    b.takte = 8;
    loop.muster = {a, b};
    LoopClip mitHall(QStringLiteral("A"), 0, 2);
    mitHall.effekte.reihe = {ClipHall{true}};
    loop.kanaele = {{QStringLiteral("Drums"), true, {mitHall, {QStringLiteral("B"), 8, 1}}},
                    {QStringLiteral("Perc"), true, {{QStringLiteral("A"), 4, 1}}}};
    loop.kanaele[1].gainDb = -6.0;
    loop.richte();
    return loop;
}

}  // namespace

TEST_CASE("Loop Austausch 33d: Export und Import ueber die Menues der Taktachse")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
    QTemporaryDir ordner;
    REQUIRE(ordner.isValid());
    Fang fang;
    setzeAustauschBasisFuerTests(ordner.path());
    const QDir patterns(ordner.filePath(QStringLiteral("patterns")));
    const QDir tracks(ordner.filePath(QStringLiteral("tracks")));

    Taktachse achse;  // nicht gezeigt
    achse.resize(1400, 300);
    achse.setLoop(arrangement(), true);
    Loop letzter = arrangement();
    QString was;
    QObject::connect(&achse, &Taktachse::bearbeitet, &achse, [&](const Loop& neu, const QString& w) {
        letzter = neu;
        was = w;
        achse.zeigeBearbeitet(neu);
    });
    // Orte: die Mitte eines Clips; ein freier Takt in einer Zeile
    const QRect perc = achse.clipRechteck(1, 0);  // A, Takte 4–8
    const auto beiTakt = [&](int kanal, int t) {
        return QPoint(achse.xVonTakt(t) + 4, achse.clipRechteck(kanal, 0).center().y());
    };

    // --- Export pattern... an B: vorgeschlagen im Benutzerordner als b.lvpattern
    fang.waehle = QStringLiteral("Export pattern...");
    fang.antwort = patterns.filePath(QStringLiteral("mein_b.lvpattern"));
    rechts(achse, achse.clipRechteck(0, 1).center());
    CHECK(fang.texte.contains(QStringLiteral("Export clip...")));
    CHECK(fang.speichern);
    CHECK(QFileInfo(fang.vorschlag) == QFileInfo(patterns.filePath(QStringLiteral("b.lvpattern"))));
    Loop gelesen;
    REQUIRE(ladeAustausch(fang.antwort, AustauschArt::Muster, gelesen));
    CHECK(gelesen.muster[0] == *arrangement().findeMuster(QStringLiteral("B")));

    // --- Export clip... am ersten Clip (A mit Hall, zwei Wiederholungen); ohne Endung angehaengt
    fang.waehle = QStringLiteral("Export clip...");
    fang.antwort = tracks.filePath(QStringLiteral("hall"));
    rechts(achse, achse.clipRechteck(0, 0).center());
    CHECK(QFileInfo(fang.vorschlag) == QFileInfo(tracks.filePath(QStringLiteral("a.lvclip"))));
    REQUIRE(ladeAustausch(tracks.filePath(QStringLiteral("hall.lvclip")), AustauschArt::Clip, gelesen));
    CHECK(gelesen.kanaele[0].clips[0].wiederholungen == 2);
    CHECK(gelesen.kanaele[0].clips[0].effekte.einerAn());

    // --- Export track... am Namen von Perc
    fang.waehle = QStringLiteral("Export track...");
    fang.antwort = tracks.filePath(QStringLiteral("perc.lvtrack"));
    rechts(achse, QPoint(10, perc.center().y()));
    CHECK(QFileInfo(fang.vorschlag) == QFileInfo(tracks.filePath(QStringLiteral("perc.lvtrack"))));
    REQUIRE(ladeAustausch(fang.antwort, AustauschArt::Spur, gelesen));
    CHECK(gelesen.kanaele[0].gainDb == -6.0);
    CHECK(was.isEmpty());  // Exportieren aendert den Loop nicht
    CHECK(fang.meldungen.isEmpty());

    // --- Import pattern... auf einen freien Takt in Perc: ein Clip B ab Takt 9, ohne Meldung
    fang.waehle = QStringLiteral("Import pattern...");
    fang.antwort = patterns.filePath(QStringLiteral("mein_b.lvpattern"));
    rechts(achse, beiTakt(1, 9));
    CHECK_FALSE(fang.speichern);
    CHECK(fang.texte.contains(QStringLiteral("Import clip...")));
    CHECK(fang.texte.contains(QStringLiteral("Import track...")));
    CHECK(was == QStringLiteral("Import pattern"));
    REQUIRE(letzter.kanaele[1].clips.size() == 2);
    CHECK(letzter.kanaele[1].clips[1] == LoopClip(QStringLiteral("B"), 9, 1));
    CHECK(letzter.muster.size() == 2);  // B mitbenutzt
    CHECK(fang.meldungen.isEmpty());
    CHECK(achse.gewaehlterKanal() == 1);
    REQUIRE(achse.auswahl() != nullptr);
    CHECK(achse.auswahl()->startTakt == 9);

    // --- Import clip... auf Takt 2 in Perc: A mit zwei Wiederholungen reicht in Takt 4 — ein neuer Kanal
    fang.waehle = QStringLiteral("Import clip...");
    fang.antwort = tracks.filePath(QStringLiteral("hall.lvclip"));
    rechts(achse, beiTakt(1, 2));
    CHECK(was == QStringLiteral("Import clip"));
    REQUIRE(letzter.kanaele.size() == 3);
    CHECK(letzter.kanaele[2].name == QStringLiteral("Drums"));
    REQUIRE(letzter.kanaele[2].clips.size() == 1);
    CHECK(letzter.kanaele[2].clips[0].startTakt == 2);
    CHECK(letzter.kanaele[2].clips[0].effekte.einerAn());
    REQUIRE(fang.meldungen.size() == 1);
    CHECK(fang.meldungen[0].contains(QStringLiteral("new channel")));

    // --- Import track...: Perc als vierter Kanal, die Clips an ihren Takten, Fader mit
    fang.waehle = QStringLiteral("Import track...");
    fang.antwort = tracks.filePath(QStringLiteral("perc.lvtrack"));
    rechts(achse, beiTakt(1, 20));
    CHECK(was == QStringLiteral("Import track"));
    REQUIRE(letzter.kanaele.size() == 4);
    CHECK(letzter.kanaele[3].name == QStringLiteral("Perc"));
    CHECK(letzter.kanaele[3].gainDb == -6.0);
    CHECK(letzter.kanaele[3].clips == std::vector<LoopClip>{LoopClip(QStringLiteral("A"), 4, 1)});
    CHECK(fang.meldungen.size() == 1);

    if (const QString bild = qEnvironmentVariable("LUMIVIZ_AUSTAUSCH_BILD"); !bild.isEmpty()) CHECK(achse.grab().save(bild));

    // --- abgebrochen: nichts; eine Datei der falschen Art: eine Meldung, nichts
    was.clear();
    fang.antwort.clear();
    rechts(achse, beiTakt(1, 20));
    CHECK(was.isEmpty());
    fang.antwort = patterns.filePath(QStringLiteral("mein_b.lvpattern"));
    rechts(achse, beiTakt(1, 20));
    CHECK(was.isEmpty());
    REQUIRE(fang.meldungen.size() == 2);
    CHECK(fang.meldungen[1] == QStringLiteral("mein_b.lvpattern is not a track file."));
}
