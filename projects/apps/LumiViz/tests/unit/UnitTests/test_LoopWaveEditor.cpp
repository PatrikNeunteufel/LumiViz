/**
 * @file test_LoopWaveEditor.cpp
 * @brief Loop, Stufe 15d: der Wave editor (docs/audio/Loop_Konzept.md §4p; Skizze SK15-1 a,
 *        Patrik S168)
 *
 * Ueber das Fenster, wie im Programm: die Zeilen (Datei mit Laenge, Rate, Kanaelen; Tempo mit
 * dem Hinweis zu Follow tempo; das Raster ohne Swing offset), die Felder und Knoepfe (je ein
 * Schritt fuer Undo), die Gesten in der Flaeche (Ausschnitt an den Kanten, Lage in Schritten
 * und mit Shift in Bruchteilen, Punkte der Spur Expression, *Clear lane*, das Lineal), FX und
 * was in 15d noch grau ist (Follow tempo, Record..., Autotune).
 *
 * Ohne Ton und ohne gezeigtes Fenster (`WA_DontShowOnScreen`). Der Dekodierer liefert zwei
 * Sekunden gleichmaessigen Ton; die Datei ist eine kleine echte WAV (fuer Rate und Kanaele).
 * `LUMIVIZ_WAVE_EDITOR_BILD=<datei.png>` legt ein Bild des Editors ab.
 */

#include <doctest.h>

#include "loop/Loop.hpp"
#include "loop/RasterZeile.hpp"
#include "loop/Sample.hpp"
#include "loop/WaveEditor.hpp"
#include "UI/widgets/Bedienhilfe.hpp"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFile>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QTemporaryDir>

#include <cstdint>

using namespace lumi::loop;

namespace
{

// in der Test-Exe merkt der Editor seine Lage nicht (wie die anderen, test_LoopEditorLage.cpp)
const bool kLageNichtMerken = (WaveEditor::setzeLageMerken(false), true);

struct Anwendung
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
};

struct EigeneEinstellungen
{
    QString org = QCoreApplication::organizationName();
    QString name = QCoreApplication::applicationName();
    EigeneEinstellungen()
    {
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("LoopWaveEditor"));
        QSettings().clear();
    }
    ~EigeneEinstellungen()
    {
        QSettings().clear();
        QCoreApplication::setOrganizationName(org);
        QCoreApplication::setApplicationName(name);
    }
};

/// Eine kleine WAV: Mono, 16 Bit, 44100 Hz, hundert stille Abtastwerte.
void schreibeWav(const QString& pfad)
{
    QFile f(pfad);
    REQUIRE(f.open(QIODevice::WriteOnly));
    const auto u32 = [&f](std::uint32_t w) {
        for (int i = 0; i < 4; ++i) f.putChar(static_cast<char>((w >> (8 * i)) & 0xff));
    };
    const auto u16 = [&f](std::uint16_t w) {
        f.putChar(static_cast<char>(w & 0xff));
        f.putChar(static_cast<char>(w >> 8));
    };
    const std::uint32_t daten = 200;
    f.write("RIFF");
    u32(36 + daten);
    f.write("WAVEfmt ");
    u32(16);
    u16(1);      // PCM
    u16(1);      // mono
    u32(44100);  // Rate
    u32(44100 * 2);
    u16(2);
    u16(16);
    f.write("data");
    u32(daten);
    f.write(QByteArray(static_cast<int>(daten), '\0'));
}

/// Zwei Sekunden gleichmaessiger Ton, gleich in welcher Rate.
std::vector<float> zweiSekunden(const QString&, int rate, double)
{
    return std::vector<float>(static_cast<std::size_t>(2 * 2 * rate), 0.5f);
}

/// 120 BPM, 4/4, vier Schritte je Schlag: ein Schritt 125 ms. Ein Wave-Muster von einem Takt,
/// Ausschnitt 0,5 … 1,0 s, Lage 2 Schritte; ein Clip ab Takt 0, dazu ein Drum-Muster.
Loop probe(const QString& datei)
{
    Loop loop;
    Muster w;
    w.name = QStringLiteral("Voc");
    w.art = MusterArt::Wave;
    w.takte = 1;
    w.wave.datei = datei;
    w.wave.start = 0.5;
    w.wave.ende = 1.0;
    w.wave.versatz = 2.0;
    w.wave.bezugBpm = 120.0;
    Muster d;
    d.name = QStringLiteral("Beat");
    d.spuren = {{QStringLiteral("Kick"), QStringLiteral("kit:kick"), 1.0, 0.0, false}};
    loop.muster = {w, d};
    loop.kanaele = {{QStringLiteral("Vocals"), true, {{QStringLiteral("Voc"), 0, 2}}},
                    {QStringLiteral("Drums"), true, {{QStringLiteral("Beat"), 0, 1}}}};
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

/// Druecken, ziehen, loslassen.
void ziehe(QWidget* ziel, QPoint von, QPoint nach, Qt::KeyboardModifiers tasten = Qt::NoModifier)
{
    maus(ziel, QEvent::MouseButtonPress, von, Qt::LeftButton, tasten);
    maus(ziel, QEvent::MouseMove, nach, Qt::LeftButton, tasten);
    maus(ziel, QEvent::MouseButtonRelease, nach, Qt::NoButton, tasten);
}

/// Der Editor mit dem Loop, gelegt (nicht gezeigt), die Flaeche mit 10 Pixeln je Schritt ab 0.
struct Bestand
{
    QTemporaryDir ordner;
    QString datei;
    WaveEditor editor;
    Loop zuletzt;
    QStringList gemeldet;

    Bestand()
    {
        REQUIRE(ordner.isValid());
        datei = QDir(ordner.path()).filePath(QStringLiteral("voc.wav"));
        schreibeWav(datei);
        editor.setDekodierer(&zweiSekunden);
        QObject::connect(&editor, &WaveEditor::bearbeitet, [this](const Loop& neu, const QString& was) {
            zuletzt = neu;
            gemeldet << was;
            editor.setLoop(neu);  // wie das Loop-Panel nach Undo
        });
        REQUIRE(editor.zeige(probe(datei), QStringLiteral("Voc")));
        editor.setAttribute(Qt::WA_DontShowOnScreen);
        editor.resize(1000, 520);
        editor.show();
        editor.flaeche()->setSchrittBreite(10.0);
        editor.flaeche()->setRoll(0.0);
    }

    [[nodiscard]] const WaveAngaben& wave() const { return zuletzt.findeMuster(QStringLiteral("Voc"))->wave; }
    [[nodiscard]] int welleY() const { return editor.flaeche()->welleOben() + 20; }
};

}  // namespace

TEST_SUITE("LoopWaveEditor")
{
    TEST_CASE("Loop Wave editor 15d: die Zeilen nach der Skizze")
    {
        Anwendung anwendung;
        EigeneEinstellungen einstellungen;
        Bestand b;
        WaveEditor& e = b.editor;
        CHECK(e.windowTitle() == QStringLiteral("Wave pattern: Voc"));
        CHECK(e.findChild<QLabel*>(QStringLiteral("waveUsers"))->text() == QStringLiteral("· used by 1 clips"));
        CHECK(e.dateiText() == QStringLiteral("voc.wav"));
        CHECK(e.dateiInfoText() == QStringLiteral("2.00 s · 44100 Hz · mono"));
        CHECK(e.findChild<QSpinBox*>(QStringLiteral("waveBars"))->maximum() == kWaveTakteMax);
        CHECK(e.tempoHinweis() == QStringLiteral("plays as recorded when on — the tempos are the same"));
        // in 15d grau: Follow tempo (15j), Record... (15k); Edit sample... nur fuer ein eigenes
        CHECK_FALSE(e.findChild<QCheckBox*>(QStringLiteral("waveFollowTempo"))->isEnabled());
        CHECK_FALSE(e.findChild<QPushButton*>(QStringLiteral("waveRecord"))->isEnabled());
        CHECK_FALSE(e.findChild<QPushButton*>(QStringLiteral("waveEditSample"))->isEnabled());
        // das Raster des Loops, ohne Swing offset (RS1-1 a)
        CHECK(e.rasterZeile()->rasterText().startsWith(QStringLiteral("Grid of the loop: 4 beats per bar")));
        CHECK(e.findChild<QDoubleSpinBox*>(QStringLiteral("waveSwingOffset")) == nullptr);
        CHECK(e.flaeche()->bild().spitzen.size() > 0);
        // keine andere Art
        CHECK_FALSE(e.zeige(probe(b.datei), QStringLiteral("Beat")));
        CHECK(e.muster() == QStringLiteral("Voc"));

        const QString ziel = qEnvironmentVariable("LUMIVIZ_WAVE_EDITOR_BILD");
        if (!ziel.isEmpty())
        {
            Loop bild = probe(b.datei);
            Muster* w = musterIn(bild, QStringLiteral("Voc"));
            w->takte = 2;
            w->zeitspuren.von(Steuerspur::Expression) = {{0.0, 1.0}, {12.0, 0.6}, {28.0, 0.9}};
            bild.richte();
            e.setLoop(bild);
            e.flaeche()->passeEin();
            e.setSpielt(true, 9.5);
            CHECK(e.grab().save(ziel));
        }
    }

    TEST_CASE("Loop Wave editor 15d: Felder und Knoepfe — je ein Schritt fuer Undo")
    {
        Anwendung anwendung;
        EigeneEinstellungen einstellungen;
        Bestand b;
        WaveEditor& e = b.editor;

        e.findChild<QDoubleSpinBox*>(QStringLiteral("waveGain"))->setValue(-6.0);
        CHECK(b.gemeldet.back() == QStringLiteral("Change gain"));
        CHECK(b.wave().gainDb == -6.0);
        e.findChild<QSpinBox*>(QStringLiteral("waveFadeIn"))->setValue(15);
        e.findChild<QSpinBox*>(QStringLiteral("waveFadeOut"))->setValue(120);
        CHECK(b.wave().fadeInMs == 15.0);
        CHECK(b.wave().fadeOutMs == 120.0);
        e.findChild<QDoubleSpinBox*>(QStringLiteral("waveReferenceTempo"))->setValue(96.0);
        CHECK(b.wave().bezugBpm == 96.0);
        CHECK(e.tempoHinweis() == QStringLiteral("plays 1.25× as fast when on"));
        e.findChild<QPushButton*>(QStringLiteral("waveSetToLoopTempo"))->click();
        CHECK(b.gemeldet.back() == QStringLiteral("Set reference to loop tempo"));
        CHECK(b.wave().bezugBpm == 120.0);
        e.setzeTonhoehenArt(TonhoehenArt::Stimme);
        CHECK(b.wave().tonhoehenArt == TonhoehenArt::Stimme);
        CHECK(e.findChild<QComboBox*>(QStringLiteral("wavePitchMethod"))->currentText() == QStringLiteral("Voice"));
        e.findChild<QSpinBox*>(QStringLiteral("waveBars"))->setValue(100);  // ueber 64: ein Wave-Muster darf
        CHECK(b.zuletzt.findeMuster(QStringLiteral("Voc"))->takte == 100);
        CHECK(b.gemeldet == QStringList{QStringLiteral("Change gain"), QStringLiteral("Change fade in"),
                                        QStringLiteral("Change fade out"), QStringLiteral("Change reference tempo"),
                                        QStringLiteral("Set reference to loop tempo"), QStringLiteral("Change pitch method"),
                                        QStringLiteral("Change pattern length")});

        SUBCASE("eine Datei zu den eigenen Samples: user:<name>, Edit sample... wird hell")
        {
            QTemporaryDir samples;
            REQUIRE(samples.isValid());
            setzeEigenenSampleOrdnerFuerTests(samples.path());
            REQUIRE(e.nimmDatei(b.datei, true));
            CHECK(b.wave().datei == QStringLiteral("user:voc"));
            CHECK(e.findChild<QPushButton*>(QStringLiteral("waveEditSample"))->isEnabled());
            QStringList verlangt;
            QObject::connect(&e, &WaveEditor::sampleBearbeitenVerlangt, [&verlangt](const QString& n) { verlangt << n; });
            e.findChild<QPushButton*>(QStringLiteral("waveEditSample"))->click();
            setzeEigenenSampleOrdnerFuerTests(QString());
            CHECK(verlangt == QStringList{QStringLiteral("user:voc")});
        }
        SUBCASE("FX: der Schalter des Bezugsclips")
        {
            Loop mit = b.editor.loop();
            mit.kanaele[0].clips[0].effekte.reihe = {ClipHall{true}};
            e.setLoop(mit);
            QPushButton* fx = e.findChild<QPushButton*>(QStringLiteral("waveFx"));
            CHECK(fx->isEnabled());
            CHECK(fx->isChecked());
            fx->click();
            CHECK(b.gemeldet.back() == QStringLiteral("Effects off"));
            CHECK_FALSE(b.zuletzt.kanaele[0].clips[0].effekteAn);
        }
    }

    TEST_CASE("Loop Wave editor 15d: die Gesten in der Flaeche")
    {
        Anwendung anwendung;
        EigeneEinstellungen einstellungen;
        Bestand b;
        WaveFlaeche* f = b.editor.flaeche();
        const int y = b.welleY();
        // der Ausschnitt 0,5 … 1,0 s liegt bei Lage 2 von Schritt 2 bis 6: x 20 bis 60
        REQUIRE(f->xVonSchritt(f->schrittVonDatei(0.5)) == doctest::Approx(20.0));
        REQUIRE(f->xVonSchritt(f->schrittVonDatei(1.0)) == doctest::Approx(60.0));

        SUBCASE("in der Welle ziehen: die Lage rastet in Schritten, mit Shift Bruchteile")
        {
            ziehe(f, QPoint(40, y), QPoint(72, y));
            CHECK(b.gemeldet.back() == QStringLiteral("Move wave"));
            CHECK(b.wave().versatz == 5.0);
            ziehe(f, QPoint(70, y), QPoint(72, y), Qt::ShiftModifier);
            CHECK(b.wave().versatz == doctest::Approx(5.2));
            CHECK(b.wave().start == 0.5);  // der Ausschnitt bleibt
        }
        SUBCASE("die linke Kante: der Anfang rueckt, der Ton bleibt, wo er ist")
        {
            ziehe(f, QPoint(20, y), QPoint(30, y));
            CHECK(b.gemeldet.back() == QStringLiteral("Trim wave start"));
            CHECK(b.wave().start == doctest::Approx(0.625));
            CHECK(b.wave().versatz == doctest::Approx(3.0));
        }
        SUBCASE("die rechte Kante: das Ende; hinter der Datei bis zu ihrem Ende")
        {
            ziehe(f, QPoint(60, y), QPoint(80, y));
            CHECK(b.gemeldet.back() == QStringLiteral("Trim wave end"));
            CHECK(b.wave().ende == doctest::Approx(1.25));
            ziehe(f, QPoint(80, y), QPoint(400, y));
            CHECK(b.wave().ende == 0.0);
        }
        SUBCASE("die Spur Expression: setzen, Doppelklick nimmt weg, Clear lane")
        {
            const QPoint punkt(50, f->spurOben() + 20);  // Schritt 5, Wert 0,5
            maus(f, QEvent::MouseButtonPress, punkt, Qt::LeftButton);
            maus(f, QEvent::MouseButtonRelease, punkt, Qt::NoButton);
            CHECK(b.gemeldet.back() == QStringLiteral("Change expression"));
            const auto& spur = b.zuletzt.findeMuster(QStringLiteral("Voc"))->zeitspuren.von(Steuerspur::Expression);
            REQUIRE(spur.size() == 1);
            CHECK(spur[0].stelle == 5.0);
            CHECK(spur[0].wert == doctest::Approx(0.5));
            // ein zweiter Punkt, dann der erste weg mit Doppelklick
            const QPoint zweiter(100, f->spurOben() + 10);
            maus(f, QEvent::MouseButtonPress, zweiter, Qt::LeftButton);
            maus(f, QEvent::MouseButtonRelease, zweiter, Qt::NoButton);
            maus(f, QEvent::MouseButtonPress, punkt, Qt::LeftButton);
            maus(f, QEvent::MouseButtonRelease, punkt, Qt::NoButton);
            maus(f, QEvent::MouseButtonDblClick, punkt, Qt::LeftButton);
            maus(f, QEvent::MouseButtonRelease, punkt, Qt::NoButton);
            CHECK(b.gemeldet.back() == QStringLiteral("Delete point"));
            CHECK(b.zuletzt.findeMuster(QStringLiteral("Voc"))->zeitspuren.von(Steuerspur::Expression).size() == 1);
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
            const QPoint frei(200, f->spurOben() + 20);
            QContextMenuEvent rechts(QContextMenuEvent::Mouse, frei, f->mapToGlobal(frei));
            QCoreApplication::sendEvent(f, &rechts);
            lumi::ui::setzeMenueBeobachterFuerTests({});
            CHECK(eintraege == QStringList{QStringLiteral("Delete point (off)"), QStringLiteral("Clear lane")});
            CHECK(b.gemeldet.back() == QStringLiteral("Clear lane"));
            CHECK(b.zuletzt.findeMuster(QStringLiteral("Voc"))->zeitspuren.von(Steuerspur::Expression).empty());
        }
        SUBCASE("das Lineal waehlt die Abspielstelle; ein Klick ohne Zug meldet nichts")
        {
            std::vector<int> stellen;
            QObject::connect(&b.editor, &WaveEditor::abspielstelleGewaehlt, [&stellen](int s) { stellen.push_back(s); });
            maus(f, QEvent::MouseButtonPress, QPoint(35, 5), Qt::LeftButton);
            maus(f, QEvent::MouseButtonRelease, QPoint(35, 5), Qt::NoButton);
            CHECK(stellen == std::vector<int>{3});
            maus(f, QEvent::MouseButtonPress, QPoint(40, y), Qt::LeftButton);
            maus(f, QEvent::MouseButtonRelease, QPoint(40, y), Qt::NoButton);
            CHECK(b.gemeldet.isEmpty());
        }
        SUBCASE("Fit: das ganze Muster in der Breite")
        {
            b.editor.findChild<QPushButton*>(QStringLiteral("waveFit"))->click();
            CHECK(f->schrittBreite() == doctest::Approx((f->width() - 4.0) / 16.0));
            CHECK(f->roll() == 0.0);
        }
    }
}
