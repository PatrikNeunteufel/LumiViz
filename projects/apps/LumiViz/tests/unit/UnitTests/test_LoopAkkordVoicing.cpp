/**
 ****************************************************************************************
 * @file   test_LoopAkkordVoicing.cpp
 * @brief  Tabs und Akkorde, Stufe 26f: Voicings im Fenster *Chords* fuer Instrumente ohne
 *         Saiten — Lage und Symbol, die Klaviatur, das Fenster, der Dialog mit Tasten, der
 *         Weg ueber den Editor (docs/audio/Saiten_Konzept.md S-T6 a, S-T14 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Die Regeln ohne Qt-Anwendung; das Uebrige ohne gezeigtes Fenster, eigene Einstellungen
 * (`LumiVizTest` / `LoopAkkordVoicing`), eigene Akkorde in einem Wegwerf-Ordner.
 * `LUMIVIZ_VOICING_BILD=<datei.png>` legt Bilder von Fenster und Dialog ab.
 ****************************************************************************************
 */

#include <doctest.h>

#include "UI/widgets/Bedienhilfe.hpp"
#include "loop/AkkordDialog.hpp"
#include "loop/AkkordFenster.hpp"
#include "loop/GriffbildBild.hpp"
#include "loop/Instrument.hpp"
#include "loop/InstrumentEditor.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QDropEvent>
#include <QImage>
#include <QLabel>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QSettings>
#include <QTemporaryDir>
#include <QToolButton>

#include <algorithm>
#include <vector>

using namespace lumi::loop;

namespace
{

struct Anwendung
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
    Anwendung()
    {
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("LoopAkkordVoicing"));
        QSettings().clear();
    }
    ~Anwendung() { QSettings().clear(); }
};

struct Wegwerf
{
    QTemporaryDir ordner;
    Wegwerf() { setzeEigenenAkkordOrdnerFuerTests(ordner.path()); }
    ~Wegwerf() { setzeEigenenAkkordOrdnerFuerTests(QString()); }
};

Akkord hole(const QString& nennung)
{
    Akkord a;
    REQUIRE(holeAkkord(nennung, a));
    return a;
}

std::vector<int> hoehen(const Muster& m)
{
    std::vector<int> h;
    for (const Note& n : m.noten) h.push_back(n.hoehe);
    return h;
}

}  // namespace

TEST_CASE("Loop Akkord Voicing 26f: Lage, Symbol, tiefster Ton, anhoeren, Toene setzen")
{
    // die Lage aus der Beschreibung der Vorlagen
    CHECK(lageText(hole(QStringLiteral("voicings/c_major_root"))) == QStringLiteral("root"));
    CHECK(lageText(hole(QStringLiteral("voicings/c_major_inv1"))) == QStringLiteral("1st inv."));
    CHECK(lageText(hole(QStringLiteral("voicings/c_7_inv3"))) == QStringLiteral("3rd inv."));
    CHECK(lageText(hole(QStringLiteral("voicings/c_major_open"))) == QStringLiteral("open"));
    CHECK(lageText(hole(QStringLiteral("voicings/c_7_drop2"))) == QStringLiteral("drop 2"));
    // ... und gerechnet, wenn die Beschreibung nichts sagt
    Akkord eigen;
    eigen.titel = QStringLiteral("C");
    eigen.art = QStringLiteral("major");
    eigen.abstaende = {0, 3, 8};  // E G C
    CHECK(lageText(eigen) == QStringLiteral("1st inv."));
    eigen.abstaende = {0, 4, 7};
    CHECK(lageText(eigen) == QStringLiteral("root"));
    eigen.abstaende = {0, 7, 16};
    CHECK(lageText(eigen) == QStringLiteral("open"));

    // welcher Ton unten liegt, und wo er zuerst liegt
    CHECK(tiefsteTonklasse(hole(QStringLiteral("voicings/c_major_inv1"))) == 4);
    CHECK(tiefsteTonklasse(hole(QStringLiteral("voicings/c_major_inv2"))) == 7);
    CHECK(tiefsteTonklasse(hole(QStringLiteral("voicings/c_major_root"))) == 0);
    CHECK(vorgabeTiefster(hole(QStringLiteral("voicings/c_major_inv1")), 48) == 52);  // E3
    CHECK(vorgabeTiefster(hole(QStringLiteral("voicings/c_major_inv1")), 36) == 40);  // ein Bass: E2
    CHECK(galerieSymbol(hole(QStringLiteral("voicings/c_major_inv1"))) == QStringLiteral("C/E"));
    CHECK(galerieSymbol(hole(QStringLiteral("voicings/c_major_root"))) == QStringLiteral("C"));
    CHECK_FALSE(tiefsteTonklasse(hole(QStringLiteral("guitar/c_major_1"))).has_value());

    // anhoeren: alle Toene zugleich, ohne Saiten und Anschlag
    const Muster m = akkordZumAnhoeren(hole(QStringLiteral("voicings/c_major_inv1")), Saitensatz{}, Strum{},
                                       AkkordSpielart::AbAuf, 4, 52);
    CHECK(hoehen(m) == std::vector<int>{52, 55, 60});
    CHECK(m.anschlaege.empty());
    CHECK(std::all_of(m.noten.begin(), m.noten.end(), [](const Note& n) { return n.schritt == 0 && n.saite == -1 && n.laenge == 8; }));

    // Toene setzen: der tiefste folgt, der letzte geht, die Weite haelt
    Akkord v;
    int tiefster = 48;
    schalteVoicingTon(v, tiefster, 55);
    CHECK(v.abstaende == std::vector<int>{0});
    CHECK(tiefster == 55);
    schalteVoicingTon(v, tiefster, 48);
    schalteVoicingTon(v, tiefster, 52);
    CHECK(v.abstaende == std::vector<int>{0, 4, 7});
    CHECK(tiefster == 48);
    schalteVoicingTon(v, tiefster, 48);  // der tiefste geht: E wird der tiefste
    CHECK(v.abstaende == std::vector<int>{0, 3});
    CHECK(tiefster == 52);
    schalteVoicingTon(v, tiefster, 52 + kVoicingWeiteMax + 1);  // zu weit: bleibt
    CHECK(v.abstaende == std::vector<int>{0, 3});
    schalteVoicingTon(v, tiefster, 52);
    schalteVoicingTon(v, tiefster, 55);
    CHECK(v.abstaende.empty());
}

TEST_CASE("Loop Akkord Voicing 26f: die Klaviatur — Tasten, Treffer, Punkte")
{
    Anwendung anwendung;
    const KlaviaturLage lage(QRectF(0, 0, 140, 40), 48, 71);  // C3 … B4: 14 weisse Tasten
    CHECK(lage.taste(48) == QRectF(0, 0, 10, 40));
    CHECK(lage.taste(50).left() == doctest::Approx(10));
    CHECK(KlaviaturLage::schwarz(49));
    CHECK(lage.taste(49).center().x() == doctest::Approx(10));  // zwischen C und D
    CHECK(lage.taste(49).height() < 40);
    for (int t = 48; t <= 71; ++t) CHECK(lage.tonBei(lage.punkt(t)) == t);
    CHECK_FALSE(lage.tonBei(QPointF(-1, 10)).has_value());
    CHECK(lage.taste(47).isEmpty());
    CHECK(klaviaturBereich({52, 55, 60}, 2) == std::pair<int, int>{48, 71});
    CHECK(klaviaturBereich({52, 55, 60, 88}, 2) == std::pair<int, int>{48, 95});
    CHECK(klaviaturBereich({}, 3, 36) == std::pair<int, int>{36, 71});

    // das Bild: ein Punkt auf E3, keiner auf F3
    QPalette palette;
    palette.setColor(QPalette::Base, Qt::white);
    palette.setColor(QPalette::Text, Qt::black);
    palette.setColor(QPalette::Highlight, QColor(0x37, 0x8A, 0xDD));
    QImage bild(140, 40, QImage::Format_ARGB32);
    bild.fill(Qt::white);
    QPainter p(&bild);
    zeichneKlaviatur(p, QRectF(0, 0, 140, 40), 48, 71, {52, 55, 60}, GriffbildGroesse::Klein, palette);
    p.end();
    const auto blau = [&](int ton) {
        const QRgb farbe = bild.pixel(lage.punkt(ton).toPoint());
        return qBlue(farbe) > 180 && qRed(farbe) < 120;
    };
    CHECK(blau(52));
    CHECK(blau(60));
    CHECK_FALSE(blau(53));
}

TEST_CASE("Loop Akkord Voicing 26f: das Fenster ohne Saiten — Galerie, Oktave, Play, eigene Voicings (LUMIVIZ_VOICING_BILD)")
{
    Anwendung anwendung;
    Wegwerf wegwerf;
    REQUIRE(wegwerf.ordner.isValid());
    AkkordFenster fenster;
    fenster.setzeZiel(QStringLiteral("Chords"), QStringLiteral("Warm pad"), Saitensatz{}, Strum{}, 4, 48);
    fenster.liesNeu();
    CHECK(fenster.zeigtVoicings());
    CHECK(fenster.findChild<QLabel*>(QStringLiteral("chordTarget"))->text() ==
          QStringLiteral("For: Warm pad · Chords — voicings (no strings)"));
    CHECK(fenster.baumZeilen()[2] == QStringLiteral("C (73)"));  // 876 Voicings, je Grundton 73; keine Griffbilder
    REQUIRE(fenster.waehleOrdner(QStringLiteral("root:0:major")));
    const QStringList galerie = fenster.galerieZeilen();
    CHECK(galerie.front() == QStringLiteral("[C major · 4]"));
    CHECK(galerie.contains(QStringLiteral("voicings/c_major_inv1")));
    CHECK_FALSE(std::any_of(galerie.begin(), galerie.end(), [](const QString& z) { return z.startsWith(QStringLiteral("guitar/")); }));

    std::vector<Muster> gehoert;
    QObject::connect(&fenster, &AkkordFenster::vorhoerenVerlangt, &fenster, [&](const Muster& m) { gehoert.push_back(m); });
    std::vector<std::pair<Akkord, int>> verwendet;
    QObject::connect(&fenster, &AkkordFenster::verwenden, &fenster,
                     [&](const Akkord& a, int bezug) { verwendet.emplace_back(a, bezug); });

    // E, F: der tiefste Ton beginnt in der Oktave 3; −8va / +8va
    REQUIRE(fenster.waehle(QStringLiteral("voicings/c_major_inv1")));
    CHECK(fenster.tiefster() == 52);
    CHECK(fenster.detailText() == QStringLiteral("Notes: E3 G3 C4"));
    CHECK(fenster.findChild<QLabel*>(QStringLiteral("chordMoveLabel"))->text() == QStringLiteral("Lowest note E3"));
    REQUIRE(fenster.verschiebeOktave(1));
    CHECK(fenster.detailText() == QStringLiteral("Notes: E4 G4 C5"));
    CHECK_FALSE(fenster.verschiebe(1));  // Buende gibt es hier nicht
    // G: ein Knopf Play, keine Spielarten
    auto* play = fenster.findChild<QPushButton*>(QStringLiteral("chordPlay"));
    REQUIRE(play != nullptr);
    CHECK_FALSE(play->isHidden());
    CHECK(fenster.findChild<QToolButton*>(QStringLiteral("chordPlayDownUp"))->isHidden());
    CHECK(fenster.findChild<QPushButton*>(QStringLiteral("chordMoveUp"))->isHidden());
    play->click();
    REQUIRE(gehoert.size() == 1);
    CHECK(hoehen(gehoert.back()) == std::vector<int>{64, 67, 72});
    CHECK(gehoert.back().anschlaege.empty());
    // Use: mit dem gezeigten tiefsten Ton
    REQUIRE(fenster.verwende());
    REQUIRE(verwendet.size() == 1);
    CHECK(verwendet.back().second == 64);
    CHECK(verwendet.back().first.abstaende == std::vector<int>{0, 3, 8});

    // B: die Suche findet die Lage
    fenster.setzeSuche(QStringLiteral("drop 2"));
    CHECK(fenster.galerieZeilen().contains(QStringLiteral("voicings/c_7_drop2")));
    CHECK_FALSE(fenster.galerieZeilen().contains(QStringLiteral("voicings/c_7_root")));
    fenster.setzeSuche(QString());

    if (const QString ziel = qEnvironmentVariable("LUMIVIZ_VOICING_BILD"); !ziel.isEmpty())
    {
        REQUIRE(fenster.waehleOrdner(QStringLiteral("root:0")));
        REQUIRE(fenster.waehle(QStringLiteral("voicings/c_major_inv1")));
        fenster.setAttribute(Qt::WA_DontShowOnScreen);
        fenster.show();
        QCoreApplication::processEvents();
        CHECK(fenster.grab().save(ziel));
        MESSAGE("Fenster mit Voicings: " << ziel.toStdString());
        fenster.hide();
    }

    // H: New — der Dialog mit der Klaviatur, Toene mit der Maus
    lumi::ui::setzeDialogBeobachterFuerTests([](QDialog& d) {
        auto& dialog = dynamic_cast<AkkordDialog&>(d);
        CHECK(dialog.istVoicing());
        CHECK(dialog.klaviaturAb() == 36);  // drei Oktaven ab dem C unter der Oktave 3
        GriffbildEingabe* tasten = dialog.eingabe();
        tasten->resize(300, 200);
        for (const int ton : {48, 55, 64})  // C3 G3 E4: eine offene Lage
        {
            const QPointF ort = tasten->klaviaturLage().punkt(ton);
            QMouseEvent druck(QEvent::MouseButtonPress, ort, tasten->mapToGlobal(ort), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
            QCoreApplication::sendEvent(tasten, &druck);
            QMouseEvent los(QEvent::MouseButtonRelease, ort, tasten->mapToGlobal(ort), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
            QCoreApplication::sendEvent(tasten, &los);
        }
        CHECK(dialog.akkord().abstaende == std::vector<int>{0, 7, 16});
        CHECK(dialog.tiefster() == 48);
        REQUIRE(dialog.vorschlag().has_value());
        CHECK(*dialog.vorschlag() == AkkordName{0, QStringLiteral("major"), -1});
        REQUIRE(dialog.uebernimmVorschlag());
        REQUIRE(dialog.nimmZurueck());  // der letzte Ton
        CHECK(dialog.akkord().abstaende == std::vector<int>{0, 7});
        REQUIRE(dialog.stelleWiederHer());
        if (const QString ziel = qEnvironmentVariable("LUMIVIZ_VOICING_BILD"); !ziel.isEmpty())
        {
            dialog.setAttribute(Qt::WA_DontShowOnScreen);
            dialog.show();
            QCoreApplication::processEvents();
            CHECK(dialog.grab().save(QString(ziel).replace(QStringLiteral(".png"), QStringLiteral("_dialog.png"))));
            dialog.hide();
        }
        REQUIRE(dialog.speichere());
        return dialog.result();
    });
    REQUIRE(fenster.neu());
    lumi::ui::setzeDialogBeobachterFuerTests({});
    CHECK(fenster.gewaehlt() == QStringLiteral("user:c"));
    const Akkord gespeichert = hole(QStringLiteral("user:c"));
    CHECK_FALSE(gespeichert.istGriffbild());
    CHECK(gespeichert.abstaende == std::vector<int>{0, 7, 16});
    CHECK(lageText(gespeichert) == QStringLiteral("open"));
    // ohne Ton wird nicht gespeichert
    AkkordDialog leer(Akkord{}, Saitensatz{}, Strum{}, 4, AkkordSpielart::Ab, nullptr, 48);
    CHECK_FALSE(leer.speichere());
    CHECK(leer.hinweis() == QStringLiteral("Set at least one note"));
}

TEST_CASE("Loop Akkord Voicing 26f: ueber den Editor — Chords... ohne Saiten, Use, Ziehen auf eine Zeile")
{
    Anwendung anwendung;
    QTemporaryDir ordner;
    REQUIRE(ordner.isValid());
    Instrument pad;
    pad.titel = QStringLiteral("Pad");
    const QString datei = QDir(ordner.path()).filePath(QStringLiteral("pad.lvinst"));
    REQUIRE(speichereInstrument(pad, datei));
    Loop loop;
    Muster mel;
    mel.name = QStringLiteral("Mel");
    mel.art = MusterArt::Instrument;
    mel.instrument = datei;
    mel.noten = {Note{4, 2, 52, 1.0, -1}, Note{4, 2, 72, 1.0, -1}};  // E3 und C5 an Schritt 4
    loop.muster = {mel};
    loop.kanaele = {{QStringLiteral("Lead"), true, {{QStringLiteral("Mel"), 0, 1}}}};
    loop.richte();

    InstrumentEditor editor;
    QObject::connect(&editor, &InstrumentEditor::bearbeitet, &editor, [&](const Loop& neu, const QString&) {
        loop = neu;
        editor.setLoop(loop);
    });
    REQUIRE(editor.zeige(loop, QStringLiteral("Mel")));
    CHECK_FALSE(editor.findChild<QPushButton*>(QStringLiteral("instrumentChords"))->isHidden());
    editor.oeffneAkkorde(false);
    AkkordFenster* fenster = editor.akkordFenster();
    REQUIRE(fenster != nullptr);
    CHECK(fenster->zeigtVoicings());
    CHECK(fenster->findChild<QLabel*>(QStringLiteral("chordTarget"))->text() ==
          QStringLiteral("For: Pad · Mel — voicings (no strings)"));

    // Use an die Einfuegemarke: C3 E3 G3 — E3 dort weicht (derselbe Ton), C5 bleibt
    NotenRaster* raster = editor.raster();
    raster->setzeEinfuegemarke(4);
    REQUIRE(fenster->waehle(QStringLiteral("voicings/c_major_root")));
    REQUIRE(fenster->verwende());
    std::vector<int> anVier;
    for (const Note& n : loop.findeMuster(QStringLiteral("Mel"))->noten)
        if (n.schritt == 4) anVier.push_back(n.hoehe);
    std::sort(anVier.begin(), anVier.end());
    CHECK(anVier == std::vector<int>{48, 52, 55, 72});

    // ziehen: der tiefste Ton landet auf der Zeile unter der Maus (S-T6 a)
    raster->resize(NotenRaster::kTastenBreite + 32 * NotenRaster::kSchrittBreite, 600);
    raster->zeigeHoehe(57);
    QMimeData daten;
    daten.setData(QLatin1String(AkkordFenster::kMimeTyp), QByteArrayLiteral("voicings/c_major_inv1\n52"));
    const QPointF ort(raster->xVonSchritt(10) + 4, raster->yVonHoehe(57) + raster->zeilenHoehe() / 2);
    QDragEnterEvent hinein(ort.toPoint(), Qt::CopyAction, &daten, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(raster, &hinein);
    CHECK(hinein.isAccepted());
    QDropEvent fallen(ort, Qt::CopyAction, &daten, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(raster, &fallen);
    CHECK(fallen.isAccepted());
    std::vector<int> anZehn;
    for (const Note& n : loop.findeMuster(QStringLiteral("Mel"))->noten)
        if (n.schritt == 10) anZehn.push_back(n.hoehe);
    std::sort(anZehn.begin(), anZehn.end());
    CHECK(anZehn == std::vector<int>{57, 60, 65});  // die Form der ersten Umkehrung ab A3
}
