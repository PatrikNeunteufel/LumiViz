/**
 ****************************************************************************************
 * @file   test_LoopEditorSpuren.cpp
 * @brief  Loop, Stufe 28d: die Spuren ueber der Zeit im Instrument-Editor
 *         (docs/audio/Spielweisen_Konzept.md §1a, §3; Entscheide Patrik S140: SF6 a,
 *         SF7 a, SK1 a, SK2 a, SK3 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton und ohne gezeigtes Fenster; die Instrumente liegen als Dateien in einem
 * Wegwerf-Ordner. `LUMIVIZ_EDITOR_SPUREN_BILD=<datei.png>` legt ein Bild des Editors ab
 * (eine Gitarre mit Tonabnehmer und einer Schicht mit Own note: alle fuenf Spuren).
 *
 * - Welche Spuren ein Instrument traegt (`traegtSpur`, `pitchBereichHt`).
 * - Die Leiste im Raster: Lage unter der Spur Strum, ueber den Tab-Linien; Werte und
 *   Stellen; Kuerzel und Namen; der Wert als Text.
 * - Ueber den Editor, wie im Programm: die Zeile *Lanes*, ein Punkt setzen, ziehen (mit
 *   Alt frei), Doppelklick, *Clear lane* — je ein Schritt fuer Undo, im Loop angekommen;
 *   ausblenden ist nur Ansicht (SK1 a).
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Instrument.hpp"
#include "loop/InstrumentEditor.hpp"
#include "loop/Loop.hpp"
#include "loop/Saiten.hpp"
#include "UI/widgets/Bedienhilfe.hpp"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QDir>
#include <QMenu>
#include <QMouseEvent>
#include <QTemporaryDir>

using namespace lumi::loop;
using Leiste = NotenRaster::LeistenSpur;

namespace
{

struct Anwendung
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
};

/// Eine Gitarre mit einer Saite: *Extended KS* ohne Tonabnehmer, oder *Waveguide* mit einem.
Instrument gitarre(bool abnehmer = false)
{
    Instrument i;
    i.titel = QStringLiteral("Steel");
    i.saiten.leer = {40, 45, 50, 55, 59, 64};
    i.saiten.mensurMm = 645.0;
    Schicht& s = i.schichten[0];
    s.schwingung = Schwingung::Saite;
    s.saitenModell = abnehmer ? SaitenModell::Wellenleiter : SaitenModell::ErweiterterKs;
    if (abnehmer) s.abnehmerSteg = Abnehmer{AbnehmerArt::Humbucker, 40.0};
    return i;
}

/// Eine Sinus-Schicht, die der Saite folgt oder ihre eigene Note spielt.
Schicht sinus(bool folgt)
{
    Schicht s;
    s.folgtSaite = folgt;
    s.pitchRangeHt = 5.0;
    return s;
}

/// Ein Loop in 4/4 mit einem Muster „Mel" fuer dieses Instrument: zwei Noten.
Loop probe(const QString& instrument)
{
    Loop loop;
    Muster mel;
    mel.name = QStringLiteral("Mel");
    mel.art = MusterArt::Instrument;
    mel.instrument = instrument;
    mel.noten = {Note{0, 4, 52, 1.0, -1}, Note{8, 4, 55, 1.0, -1}};
    loop.muster = {mel};
    loop.kanaele = {{QStringLiteral("Lead"), true, {{QStringLiteral("Mel"), 0, 1}}}};
    loop.richte();
    return loop;
}

void maus(QWidget* ziel, QEvent::Type art, QPoint ort, Qt::MouseButtons gedrueckt,
          Qt::KeyboardModifiers tasten = Qt::NoModifier)
{
    const Qt::MouseButton knopf = art == QEvent::MouseMove ? Qt::NoButton : Qt::LeftButton;
    QMouseEvent e(art, ort, ziel->mapToGlobal(ort), knopf, gedrueckt, tasten);
    QCoreApplication::sendEvent(ziel, &e);
}

void klick(QWidget* ziel, QPoint ort)
{
    maus(ziel, QEvent::MouseButtonPress, ort, Qt::LeftButton);
    maus(ziel, QEvent::MouseButtonRelease, ort, Qt::NoButton);
}

}  // namespace

TEST_SUITE("LoopEditorSpuren")
{
    TEST_CASE("Loop Editor Spuren 28d-1: welche Spuren ein Instrument traegt (SF6 a, SF7 a)")
    {
        const auto spuren = [](const Instrument& i) {
            std::vector<bool> traegt;
            for (const Steuerspur s : kAlleSteuerspuren) traegt.push_back(traegtSpur(i, s));
            return traegt;  // Sustain, Sostenuto, Expression, Vibrato, Pitch
        };
        // ein Synth: alle fuenf, Pitch so weit wie seine Pitch range
        Instrument synth;
        CHECK(spuren(synth) == std::vector<bool>{true, true, true, true, true});
        CHECK(pitchBereichHt(synth) == kPitchRangeVorgabe);
        // die Saite allein, ohne Tonabnehmer und Hebel: nur Vibrato (der Finger)
        Instrument ks = gitarre();
        CHECK(spuren(ks) == std::vector<bool>{false, false, false, true, false});
        CHECK(pitchBereichHt(ks) == 0.0);
        // mit Vibratohebel: Pitch, so weit wie seine groessere Grenze
        ks.saiten.hebelRunterHt = 3.0;
        ks.saiten.hebelHochHt = 1.0;
        CHECK(spuren(ks) == std::vector<bool>{false, false, false, true, true});
        CHECK(pitchBereichHt(ks) == 3.0);
        // mit Tonabnehmer: Expression
        CHECK(spuren(gitarre(true)) == std::vector<bool>{false, false, true, true, false});
        // eine Schicht, die der Saite folgt: Expression, Vibrato, Pitch — die Pedale nicht (28b)
        Instrument folgend = gitarre();
        folgend.schichten.push_back(sinus(true));
        CHECK(spuren(folgend) == std::vector<bool>{false, false, true, true, true});
        CHECK(pitchBereichHt(folgend) == 5.0);
        // eine mit Own note: alle fuenf
        Instrument eigen = gitarre();
        eigen.schichten.push_back(sinus(false));
        CHECK(spuren(eigen) == std::vector<bool>{true, true, true, true, true});
        // keine Schicht an: keine
        synth.schichten[0].an = false;
        CHECK(spuren(synth) == std::vector<bool>{false, false, false, false, false});
    }

    TEST_CASE("Loop Editor Spuren 28d-2: die Leiste im Raster — Lage, Werte, Stellen, Namen")
    {
        Anwendung anwendung;
        NotenRaster raster;
        Muster m = probe(QStringLiteral("probe")).muster.front();
        m.zeitspuren.von(Steuerspur::Expression) = {{4.0, 0.5}, {12.0, 1.0}};
        m.zeitspuren.von(Steuerspur::Pitch) = {{0.0, 0.0}, {8.0, 7.0}};  // jenseits des Bereichs: gekappt gezeichnet
        raster.setMuster(m, 4, 4);
        raster.setzeGetrageneSpuren({true, true, true, true, true}, 2.0);
        raster.resize(NotenRaster::kTastenBreite + 32 * NotenRaster::kSchrittBreite, 600);

        // SK1 a: beim Oeffnen die Spuren mit Werten
        CHECK(raster.sichtbareSpuren().empty());
        raster.zeigeSpurenMitWerten();
        const std::vector<Leiste> sichtbar = raster.sichtbareSpuren();
        REQUIRE(sichtbar.size() == 2);
        CHECK(sichtbar[0] == Leiste{Steuerspur::Expression, {}});
        CHECK(sichtbar[1] == Leiste{Steuerspur::Pitch, {}});
        CHECK(raster.angeboteneSpuren().size() == 5);
        // ohne Saiten: unter der Zeile der Takte, darueber die Noten
        CHECK(raster.spurenOben() == NotenRaster::kKopfHoehe);
        CHECK(raster.spurenHoehe() == 2 * NotenRaster::kSpurHoehe);
        CHECK(raster.rasterOben() == NotenRaster::kKopfHoehe + 2 * NotenRaster::kSpurHoehe);
        CHECK(raster.spurBei(raster.spurOben(1) + 3) == 1);
        CHECK(raster.spurBei(raster.rasterOben() + 1) == -1);
        CHECK(raster.hoeheBei(raster.spurOben(0) + 2) == -1);  // die Spuren sind keine Tonhoehen

        // Werte: Expression 0 … 1, Pitch im Bereich des Instruments, eine Spur hin und zurueck
        CHECK(raster.spurBereich(sichtbar[0]) == std::pair<double, double>{0.0, 1.0});
        CHECK(raster.spurBereich(sichtbar[1]) == std::pair<double, double>{-2.0, 2.0});
        CHECK(raster.wertBei(0, raster.yVonWert(0, 0.5)) == doctest::Approx(0.5));
        CHECK(raster.wertBei(1, raster.yVonWert(1, -1.0)) == doctest::Approx(-1.0));
        CHECK(raster.wertBei(1, raster.spurOben(1)) == 2.0);  // ganz oben: die Grenze
        // Stellen: auf den Schritt, mit Alt frei
        CHECK(raster.stelleBei(raster.xVonStelle(5.0) + 3, false) == 5.0);
        CHECK(raster.stelleBei(raster.xVonStelle(5.0) + 4, true) == 5.25);
        CHECK(raster.spurPunktBei(0, QPoint(raster.xVonStelle(4.0) + 2, raster.yVonWert(0, 0.5) - 1)) == 0);
        CHECK(raster.spurPunktBei(0, QPoint(raster.xVonStelle(8.0), raster.yVonWert(0, 0.5))) == -1);

        // SK2 a: Kuerzel und Namen; der Wert als Text
        CHECK(NotenRaster::spurKuerzel(sichtbar[0]) == QStringLiteral("Expr"));
        CHECK(NotenRaster::spurKuerzel(Leiste{Steuerspur::Sostenuto, {}}) == QStringLiteral("Sost"));
        CHECK(NotenRaster::spurKuerzel(Leiste{Steuerspur::Sustain, QStringLiteral("unaCorda")}) == QStringLiteral("unaCor"));
        CHECK(NotenRaster::spurName(sichtbar[1]) == QStringLiteral("Pitch"));
        CHECK(NotenRaster::wertText(sichtbar[0], 0.5) == QStringLiteral("0.50 (-12.0 dB)"));
        CHECK(NotenRaster::wertText(Leiste{Steuerspur::Sustain, {}}, 0.5) == QStringLiteral("50 %"));
        CHECK(NotenRaster::wertText(sichtbar[1], 1.5) == QStringLiteral("+1.50 semitones"));
        CHECK(NotenRaster::wertText(Leiste{Steuerspur::Vibrato, {}}, 30.0) == QStringLiteral("30 cents"));

        // mit Saiten: Strum, dann die Spuren, dann die Tab-Linien
        Saitensatz saiten = gitarre().saiten;
        raster.setSaiten(saiten);
        CHECK(raster.spurenOben() == raster.strumOben() + NotenRaster::kStrumHoehe);
        CHECK(raster.tabOben() == raster.spurenOben() + 2 * NotenRaster::kSpurHoehe);
        CHECK(raster.rasterOben() == raster.tabOben() + raster.tabHoehe());
        CHECK(raster.strumBei(QPoint(raster.xVonSchritt(0) + 4, raster.strumOben() + 8)) == 0);
        CHECK(raster.strumBei(QPoint(raster.xVonSchritt(0) + 4, raster.spurOben(0) + 8)) == -1);
        CHECK(raster.tabSaiteBei(raster.spurOben(1) + 8) == -1);

        // ausblenden ist nur Ansicht (SK1 a)
        raster.setzeSpurSichtbar(sichtbar[1], false);
        CHECK(raster.sichtbareSpuren().size() == 1);
        CHECK(raster.hatWerte(sichtbar[1]));
        CHECK(raster.muster().zeitspuren.von(Steuerspur::Pitch).size() == 2);
    }

    TEST_CASE("Loop Editor Spuren 28d-3: ueber den Editor — setzen, ziehen, loeschen, Clear lane")
    {
        // wie im Programm: das Raster meldet, der Editor baut die neue Fassung des Loops,
        // das Panel gibt sie mit setLoop zurueck
        Anwendung anwendung;
        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());
        Instrument keys;
        keys.titel = QStringLiteral("Keys");
        const QString datei = QDir(ordner.path()).filePath(QStringLiteral("keys.lvinst"));
        REQUIRE(speichereInstrument(keys, datei));

        InstrumentEditor editor;
        Loop loop = probe(datei);
        QStringList gemeldet;
        QObject::connect(&editor, &InstrumentEditor::bearbeitet, &editor, [&](const Loop& neu, const QString& was) {
            gemeldet << was;
            loop = neu;
            editor.setLoop(loop);
        });
        REQUIRE(editor.zeige(loop, QStringLiteral("Mel")));
        NotenRaster* raster = editor.raster();
        raster->resize(NotenRaster::kTastenBreite + 32 * NotenRaster::kSchrittBreite, 600);
        const auto mel = [&]() -> const Muster& { return *loop.findeMuster(QStringLiteral("Mel")); };
        const auto expression = [&]() { return mel().zeitspuren.von(Steuerspur::Expression); };

        // die Zeile Lanes: je Spur des Synths ein Haken, in der Reihenfolge der Skizze; ohne Werte aus
        const std::vector<QCheckBox*>& haken = editor.spurHaken();
        REQUIRE(haken.size() == 5);
        CHECK(haken[0]->text() == QStringLiteral("Sustain"));
        CHECK(haken[4]->text() == QStringLiteral("Pitch"));
        for (QCheckBox* h : haken) CHECK_FALSE(h->isChecked());
        CHECK(raster->spurenHoehe() == 0);
        const int notenVorher = raster->rasterOben();
        haken[2]->setChecked(true);  // Expression
        REQUIRE(raster->sichtbareSpuren().size() == 1);
        CHECK(raster->rasterOben() == notenVorher + NotenRaster::kSpurHoehe);
        CHECK(gemeldet.isEmpty());  // einblenden aendert den Loop nicht

        // ein Klick setzt einen Punkt auf den Schritt — ein Schritt fuer Undo, im Loop angekommen
        const int y = raster->yVonWert(0, 0.5);
        klick(raster, QPoint(raster->xVonStelle(4.0) + 3, y));
        REQUIRE(gemeldet.size() == 1);
        CHECK(gemeldet.back() == QStringLiteral("Add lane point"));
        CHECK(expression() == std::vector<SpurPunkt>{{4.0, 0.5}});
        CHECK(raster->muster().zeitspuren.von(Steuerspur::Expression).size() == 1);

        // ziehen: auf Schritt 6 und hoeher; mit Alt frei
        const QPoint punkt(raster->xVonStelle(4.0), y);
        maus(raster, QEvent::MouseButtonPress, punkt, Qt::LeftButton);
        maus(raster, QEvent::MouseMove, QPoint(raster->xVonStelle(6.0) + 2, raster->yVonWert(0, 0.75)), Qt::LeftButton);
        maus(raster, QEvent::MouseButtonRelease, QPoint(raster->xVonStelle(6.0) + 2, raster->yVonWert(0, 0.75)), Qt::NoButton);
        REQUIRE(gemeldet.size() == 2);
        CHECK(gemeldet.back() == QStringLiteral("Move lane point"));
        CHECK(expression() == std::vector<SpurPunkt>{{6.0, 0.75}});
        const QPoint neu(raster->xVonStelle(6.0), raster->yVonWert(0, 0.75));
        maus(raster, QEvent::MouseButtonPress, neu, Qt::LeftButton, Qt::AltModifier);
        maus(raster, QEvent::MouseMove, QPoint(raster->xVonStelle(6.0) + 4, neu.y()), Qt::LeftButton, Qt::AltModifier);
        maus(raster, QEvent::MouseButtonRelease, QPoint(raster->xVonStelle(6.0) + 4, neu.y()), Qt::NoButton, Qt::AltModifier);
        CHECK(expression() == std::vector<SpurPunkt>{{6.25, 0.75}});

        // ein Punkt bleibt zwischen seinen Nachbarn
        klick(raster, QPoint(raster->xVonStelle(10.0), raster->yVonWert(0, 0.25)));
        REQUIRE(expression().size() == 2);
        maus(raster, QEvent::MouseButtonPress, QPoint(raster->xVonStelle(10.0), raster->yVonWert(0, 0.25)), Qt::LeftButton);
        maus(raster, QEvent::MouseMove, QPoint(raster->xVonStelle(2.0), raster->yVonWert(0, 0.25)), Qt::LeftButton);
        maus(raster, QEvent::MouseButtonRelease, QPoint(raster->xVonStelle(2.0), raster->yVonWert(0, 0.25)), Qt::NoButton);
        CHECK(expression().back().stelle == 10.0);

        // ein Doppelklick nimmt einen Punkt weg
        const int vorDoppel = static_cast<int>(gemeldet.size());
        const QPoint zweiter(raster->xVonStelle(10.0), raster->yVonWert(0, 0.25));
        maus(raster, QEvent::MouseButtonPress, zweiter, Qt::LeftButton);
        maus(raster, QEvent::MouseButtonRelease, zweiter, Qt::NoButton);
        CHECK(static_cast<int>(gemeldet.size()) == vorDoppel);  // greifen ohne ziehen aendert nichts
        maus(raster, QEvent::MouseButtonDblClick, zweiter, Qt::LeftButton);
        CHECK(gemeldet.back() == QStringLiteral("Delete lane point"));
        CHECK(expression() == std::vector<SpurPunkt>{{6.25, 0.75}});

        // das Menue: Clear lane
        QStringList eintraege;
        lumi::ui::setzeMenueBeobachterFuerTests([&](QMenu& menue) -> QAction* {
            QAction* leeren = nullptr;
            for (QAction* a : menue.actions())
            {
                eintraege << a->text() + (a->isEnabled() ? QString() : QStringLiteral(" (off)"));
                if (a->text() == QStringLiteral("Clear lane")) leeren = a;
            }
            return leeren;
        });
        const QPoint frei(raster->xVonStelle(12.0), raster->spurOben(0) + 10);
        QContextMenuEvent rechts(QContextMenuEvent::Mouse, frei, raster->mapToGlobal(frei));
        QCoreApplication::sendEvent(raster, &rechts);
        lumi::ui::setzeMenueBeobachterFuerTests({});
        CHECK(eintraege == QStringList{QStringLiteral("Delete point (off)"), QStringLiteral("Clear lane")});
        CHECK(gemeldet.back() == QStringLiteral("Clear lane"));
        CHECK(expression().empty());
        CHECK(raster->sichtbareSpuren().size() == 1);  // die Spur bleibt eingeblendet, jetzt leer

        // SK1 a: eine ausgeblendete Spur mit Werten traegt einen Punkt hinter dem Namen und klingt weiter
        klick(raster, QPoint(raster->xVonStelle(2.0), raster->yVonWert(0, 0.5)));
        haken[2]->setChecked(false);
        CHECK(haken[2]->text() == QString::fromUtf8("Expression •"));
        CHECK(haken[2]->toolTip() == QStringLiteral("hidden, still playing"));
        CHECK(expression().size() == 1);
        CHECK(raster->spurenHoehe() == 0);

        // ein Instrument, das Expression nicht traegt: die Spur mit Werten bleibt angeboten
        Instrument steel = gitarre();
        const QString steelDatei = QDir(ordner.path()).filePath(QStringLiteral("steel.lvinst"));
        REQUIRE(speichereInstrument(steel, steelDatei));
        editor.setzeInstrument(steelDatei);
        CHECK(gemeldet.back() == QStringLiteral("Change instrument"));
        REQUIRE(editor.spurHaken().size() == 2);  // Expression (mit Werten) und Vibrato
        CHECK(editor.spurHaken()[0]->text() == QString::fromUtf8("Expression •"));
        CHECK(editor.spurHaken()[0]->toolTip().contains(QStringLiteral("does not use it")));
        CHECK(editor.spurHaken()[1]->text() == QStringLiteral("Vibrato"));
    }

    TEST_CASE("Loop Editor Spuren 28d: ein anderes Muster oeffnen zeigt seine Spuren mit Werten (SK1 a)")
    {
        Anwendung anwendung;
        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());
        Instrument keys;
        const QString datei = QDir(ordner.path()).filePath(QStringLiteral("keys.lvinst"));
        REQUIRE(speichereInstrument(keys, datei));
        Loop loop = probe(datei);
        Muster zweites = loop.muster.front();
        zweites.name = QStringLiteral("Bass");
        zweites.zeitspuren.von(Steuerspur::Sustain) = {{0.0, 1.0}, {8.0, 0.0}};
        loop.muster.push_back(zweites);
        loop.kanaele.front().clips.push_back(LoopClip(QStringLiteral("Bass"), 1, 1));
        loop.richte();

        InstrumentEditor editor;
        REQUIRE(editor.zeige(loop, QStringLiteral("Mel")));
        CHECK(editor.raster()->sichtbareSpuren().empty());
        REQUIRE(editor.zeige(loop, QStringLiteral("Bass")));
        REQUIRE(editor.raster()->sichtbareSpuren().size() == 1);
        CHECK(editor.raster()->sichtbareSpuren()[0] == Leiste{Steuerspur::Sustain, {}});
        CHECK(editor.spurHaken()[0]->isChecked());
    }

    TEST_CASE("Loop Editor Spuren 28d: Bild des Editors mit allen fuenf Spuren")
    {
        const QString ziel = qEnvironmentVariable("LUMIVIZ_EDITOR_SPUREN_BILD");
        if (ziel.isEmpty()) return;
        Anwendung anwendung;
        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());
        Instrument voll = gitarre(true);
        voll.saiten.hebelRunterHt = 2.0;
        voll.saiten.hebelHochHt = 1.0;
        voll.schichten.push_back(sinus(false));
        const QString datei = QDir(ordner.path()).filePath(QStringLiteral("voll.lvinst"));
        REQUIRE(speichereInstrument(voll, datei));
        Loop bild = probe(datei);
        Zeitspuren& z = bild.muster.front().zeitspuren;
        z.von(Steuerspur::Sustain) = {{0.0, 1.0}, {7.0, 0.0}, {8.0, 0.5}, {14.0, 0.0}};
        z.von(Steuerspur::Expression) = {{0.0, 0.3}, {8.0, 1.0}, {12.0, 0.6}};
        z.von(Steuerspur::Vibrato) = {{4.0, 0.0}, {10.0, 40.0}};
        z.von(Steuerspur::Pitch) = {{8.0, 0.0}, {10.0, -2.0}, {12.0, 7.0}};  // 7 jenseits von ±5: gekappt gezeichnet
        bild.richte();
        InstrumentEditor gezeigt;
        REQUIRE(gezeigt.zeige(bild, QStringLiteral("Mel")));
        gezeigt.spurHaken()[1]->setChecked(true);  // Sostenuto leer: „Instrument default"
        gezeigt.setAttribute(Qt::WA_DontShowOnScreen);
        gezeigt.resize(1280, 760);
        gezeigt.show();  // das Layout steht erst nach dem Zeigen; auf den Bildschirm kommt nichts
        QCoreApplication::processEvents();
        gezeigt.raster()->zeigeHoehe(52);
        QCoreApplication::processEvents();
        CHECK(gezeigt.grab().save(ziel));
    }
}
