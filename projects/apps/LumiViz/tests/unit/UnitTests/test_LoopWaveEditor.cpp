/**
 * @file test_LoopWaveEditor.cpp
 * @brief Loop, Stufe 15d: der Wave editor (docs/audio/Loop_Konzept.md §4p; Skizze SK15-1 a,
 *        Patrik S168)
 *
 * Ueber das Fenster, wie im Programm: die Zeilen (Datei mit Laenge, Rate, Kanaelen; Tempo mit
 * dem Hinweis zu Follow tempo; das Raster ohne Swing offset), die Felder und Knoepfe (je ein
 * Schritt fuer Undo), die Gesten in der Flaeche (Ausschnitt an den Kanten, Lage in Schritten
 * und mit Shift in Bruchteilen, Punkte der Spur Expression, *Clear lane*, das Lineal), FX und
 * was in 15d noch grau ist (Follow tempo). Seit 15i die Spur Autotune: messen im Hintergrund,
 * *Detect notes* mit *Use Voice*, Key und Scale, Strength fuer Vorgabe und Note, Zielnoten ziehen,
 * teilen, verbinden, aufziehen, loeschen, *Selected section only*;
 * `LUMIVIZ_WAVE_AUTOTUNE_BILD=<datei.png>` legt ein Bild mit zwei Zielnoten ab.
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
#include "TonhoehenOrdner.hpp"
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

#include <QKeyEvent>

#include <cmath>
#include <cstdint>
#include <numbers>

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

/// Stufe 15i: zwei Sekunden Gesang — bis 1 s A3 30 Cent zu hoch, danach C4 20 Cent zu tief,
/// mit Obertoenen bis 5 kHz.
std::vector<float> gesungen(const QString&, int rate, double)
{
    const auto n = static_cast<std::int64_t>(2 * rate);
    std::vector<float> d(static_cast<std::size_t>(2 * n));
    double phase = 0.0;
    for (std::int64_t i = 0; i < n; ++i)
    {
        const double midi = i < rate ? 57.3 : 59.8;
        const double f = 440.0 * std::pow(2.0, (midi - 69.0) / 12.0);
        phase += 2.0 * std::numbers::pi * f / rate;
        double w = 0.0;
        for (int k = 1; k * f < 5000.0; ++k) w += std::sin(k * phase) / k;
        d[static_cast<std::size_t>(2 * i)] = d[static_cast<std::size_t>(2 * i + 1)] = static_cast<float>(0.2 * w);
    }
    return d;
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
    TonhoehenOrdner tonhoehe;  // Stufe 15i: der Editor misst im Hintergrund — als erstes, stirbt zuletzt
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
        editor.resize(1000, 680);  // seit 15i mit der Spur Autotune und ihrer Zeile
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
        // Follow tempo seit 15j und Record... seit 15k-1 hell; Edit sample... nur fuer ein eigenes
        CHECK(e.findChild<QCheckBox*>(QStringLiteral("waveFollowTempo"))->isEnabled());
        CHECK(e.findChild<QPushButton*>(QStringLiteral("waveRecord"))->isEnabled());
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
            // Stufe 15e: das Fenster Record sample mit diesem Sample, nicht gezeigt
            CHECK(e.oeffneSampleBearbeiten(false));
            CHECK(e.aufnahmeFenster() != nullptr);
            setzeEigenenSampleOrdnerFuerTests(QString());
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
        SUBCASE("15f: die Spur Pitch — ganze Halbtoene, mit Shift Hundertstel, eigenes Undo-Wort")
        {
            // die Spur Pitch liegt unter Expression: ±12 Halbtoene auf 32 Pixeln (4 Luft oben und unten)
            const int oben = f->spurOben(Steuerspur::Pitch) + 4;
            const int unten = f->spurOben(Steuerspur::Pitch) + WaveFlaeche::kSpurHoehe - 4;
            CHECK(f->spurOben(Steuerspur::Pitch) == f->spurOben() + WaveFlaeche::kSpurHoehe);
            CHECK(f->welleOben() == f->spurOben(Steuerspur::Pitch) + WaveFlaeche::kSpurHoehe);
            const int y6 = unten - static_cast<int>(0.75 * (unten - oben));  // +6 Halbtoene
            const QPoint punkt(50, y6);
            maus(f, QEvent::MouseButtonPress, punkt, Qt::LeftButton);
            maus(f, QEvent::MouseButtonRelease, punkt, Qt::NoButton);
            CHECK(b.gemeldet.back() == QStringLiteral("Change pitch"));
            const Muster* m = b.zuletzt.findeMuster(QStringLiteral("Voc"));
            REQUIRE(m != nullptr);
            REQUIRE(m->zeitspuren.von(Steuerspur::Pitch).size() == 1);
            CHECK(m->zeitspuren.von(Steuerspur::Pitch)[0].stelle == 5.0);
            CHECK(m->zeitspuren.von(Steuerspur::Pitch)[0].wert == 6.0);
            CHECK(m->zeitspuren.von(Steuerspur::Expression).empty());  // die andere Spur bleibt
            // mit Shift ein Pixel tiefer: Hundertstel statt ganzer Halbtoene
            const QPoint fein(100, y6 + 1);
            maus(f, QEvent::MouseButtonPress, fein, Qt::LeftButton, Qt::ShiftModifier);
            maus(f, QEvent::MouseButtonRelease, fein, Qt::NoButton, Qt::ShiftModifier);
            const double erwartet = std::round((-12.0 + 24.0 * (unten - (y6 + 1)) / static_cast<double>(unten - oben)) * 100.0) / 100.0;
            m = b.zuletzt.findeMuster(QStringLiteral("Voc"));
            REQUIRE(m->zeitspuren.von(Steuerspur::Pitch).size() == 2);
            CHECK(m->zeitspuren.von(Steuerspur::Pitch)[1].wert == doctest::Approx(erwartet));
            CHECK(m->zeitspuren.von(Steuerspur::Pitch)[1].wert != std::round(erwartet));
            // Doppelklick nimmt den ersten weg
            maus(f, QEvent::MouseButtonPress, punkt, Qt::LeftButton);
            maus(f, QEvent::MouseButtonRelease, punkt, Qt::NoButton);
            maus(f, QEvent::MouseButtonDblClick, punkt, Qt::LeftButton);
            maus(f, QEvent::MouseButtonRelease, punkt, Qt::NoButton);
            CHECK(b.gemeldet.back() == QStringLiteral("Delete point"));
            CHECK(b.zuletzt.findeMuster(QStringLiteral("Voc"))->zeitspuren.von(Steuerspur::Pitch).size() == 1);
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

    TEST_CASE("Loop Wave editor 15j: Follow tempo — an und aus, die Welle folgt, Undo-Worte")
    {
        Anwendung anwendung;
        EigeneEinstellungen einstellungen;
        Bestand b;
        WaveEditor& e = b.editor;
        WaveFlaeche* f = e.flaeche();
        auto* folgt = e.findChild<QCheckBox*>(QStringLiteral("waveFollowTempo"));
        // Bezugstempo 60 bei einem Loop mit 120: Faktor 2
        e.setzeBezugBpm(60.0);
        CHECK(e.tempoHinweis() == QStringLiteral("plays 2.00× as fast when on"));
        const double breiteAus = f->xVonDatei(1.0) - f->xVonDatei(0.5);
        folgt->click();
        CHECK(b.gemeldet.back() == QStringLiteral("Follow tempo"));
        CHECK(b.wave().folgtTempo);
        CHECK(folgt->isChecked());
        // eine halbe Sekunde der Datei ist im Muster halb so breit; der Anfang des Ausschnitts bleibt
        CHECK(f->xVonDatei(1.0) - f->xVonDatei(0.5) == doctest::Approx(breiteAus / 2.0));
        CHECK(f->schrittVonDatei(0.5) == doctest::Approx(2.0));
        CHECK(f->ausschnittEnde() == 1.0);
        // der Klang traegt das Tempo; die Welle am Ende des Ausschnitts zu ziehen rechnet gedehnt
        CHECK(b.zuletzt.klangNamen().join(QLatin1Char(' ')).contains(QStringLiteral(";tempo:2|")));
        ziehe(f, QPoint(static_cast<int>(f->xVonDatei(1.0)), b.welleY()), QPoint(static_cast<int>(f->xVonDatei(0.75)), b.welleY()));
        CHECK(b.gemeldet.back() == QStringLiteral("Trim wave end"));
        CHECK(b.wave().ende == doctest::Approx(0.75).epsilon(0.02));
        folgt->click();
        CHECK(b.gemeldet.back() == QStringLiteral("Play as recorded"));
        CHECK_FALSE(b.wave().folgtTempo);
        CHECK_FALSE(b.zuletzt.klangNamen().join(QLatin1Char(' ')).contains(QStringLiteral(";tempo:")));
    }

    TEST_CASE("Loop Wave editor 15i: nichts gesungen — der Editor sagt es, Detect notes ersetzt nichts")
    {
        // nach dem ersten Versuch im Programm (S170): eine Beatbox-Aufnahme hat keine Tonhoehe, Zielnoten taeten nichts
        Anwendung anwendung;
        EigeneEinstellungen einstellungen;
        Bestand b;  // der Dekodierer liefert gleichmaessigen Pegel: gemessen, aber nichts gesungen
        WaveEditor& e = b.editor;
        e.warteAufTonhoehe();
        REQUIRE(e.tonhoeheBereit());
        CHECK_FALSE(e.flaeche()->gesungenImAusschnitt());
        const QString keinGesang =
            QStringLiteral("No sung pitch in this section — Autotune needs a single sung voice (not beatbox or speech)");
        CHECK(e.autotuneHinweis() == keinGesang);
        // eine von Hand gezogene Note bleibt, Detect notes ersetzt sie nicht durch nichts
        Loop mitNote = probe(b.datei);
        musterIn(mitNote, QStringLiteral("Voc"))->wave.autotune.abschnitte = {{0.6, 0.9, 57, 0, 1.0, 20.0}};
        mitNote.richte();
        e.setLoop(mitNote);
        CHECK(e.autotuneHinweis() == keinGesang);
        CHECK_FALSE(e.erkenneNoten());
        CHECK(e.autotuneHinweis() == QStringLiteral("No notes found — nothing sung in this section"));
        CHECK(b.gemeldet.isEmpty());
        CHECK_FALSE(e.stimmeVorgeschlagen());
    }

    TEST_CASE("Loop Wave editor 15i: die Spur Autotune — messen, Detect notes, Zielnoten bearbeiten")
    {
        Anwendung anwendung;
        EigeneEinstellungen einstellungen;
        Bestand b;
        WaveEditor& e = b.editor;
        WaveFlaeche* f = e.flaeche();
        // der Verlauf gilt je Inhalt der Datei: fuer den Gesang ein anderer Inhalt als fuer den Ton davor
        e.warteAufTonhoehe();
        {
            QFile datei(b.datei);
            REQUIRE(datei.open(QIODevice::Append));
            datei.write("sung");
        }
        e.setDekodierer(&gesungen);
        e.warteAufTonhoehe();
        REQUIRE(e.tonhoeheBereit());
        // 40 Pixel je Schritt: der Ausschnitt 0,5 … 1,0 s liegt bei Schritt 2 … 6, also x 80 … 240
        f->setSchrittBreite(40.0);
        f->setRoll(0.0);
        CHECK(f->xVonDatei(0.5) == doctest::Approx(80.0));
        auto* erkennen = e.findChild<QPushButton*>(QStringLiteral("waveDetectNotes"));
        auto* staerke = e.findChild<QSpinBox*>(QStringLiteral("waveStrength"));
        auto* nehmen = e.findChild<QPushButton*>(QStringLiteral("waveUseVoice"));
        REQUIRE(erkennen != nullptr);
        CHECK(erkennen->isEnabled());
        // die Spur zeigt den gesungenen Ton
        CHECK(f->tonUnten() <= 57);
        CHECK(f->tonOben() >= 58);
        CHECK(f->tonOben() - f->tonUnten() >= WaveFlaeche::kAutotuneTonMin);
        const auto noten = [&b]() -> const std::vector<AutotuneAbschnitt>& { return b.wave().autotune.abschnitte; };

        // Detect notes im Ausschnitt: eine Note A3, die Vorgabe 100 % / 20 ms; eine Stimme — Use Voice
        erkennen->click();
        REQUIRE_FALSE(b.gemeldet.isEmpty());
        CHECK(b.gemeldet.back() == QStringLiteral("Detect notes"));
        REQUIRE(noten().size() == 1);
        CHECK(noten()[0].zielHoehe == 57);
        CHECK(noten()[0].anfang == doctest::Approx(0.5).epsilon(0.02));
        CHECK(noten()[0].ende == doctest::Approx(1.0).epsilon(0.02));
        CHECK(noten()[0].staerke == 1.0);
        CHECK(noten()[0].speedMs == 20.0);
        CHECK(e.stimmeVorgeschlagen());
        CHECK(nehmen->isVisibleTo(&e));
        nehmen->click();
        CHECK(b.wave().tonhoehenArt == TonhoehenArt::Stimme);
        CHECK_FALSE(e.stimmeVorgeschlagen());
        CHECK_FALSE(nehmen->isVisibleTo(&e));

        // Key und Scale
        e.setzeTonart(2, Skala::Dur);
        CHECK(b.gemeldet.back() == QStringLiteral("Change key"));
        CHECK(b.wave().autotune.grundton == 2);
        CHECK(b.wave().autotune.skala == Skala::Dur);

        // Strength ohne Wahl: die Vorgabe; mit Wahl: die Note
        staerke->setValue(50);
        CHECK(b.wave().autotune.staerke == 0.5);
        CHECK(noten()[0].staerke == 1.0);
        const int mitteX = static_cast<int>(f->xVonDatei(0.75));
        const int aY = static_cast<int>(std::lround(f->yVonMidi(57.0)));
        maus(f, QEvent::MouseButtonPress, QPoint(mitteX, aY), Qt::LeftButton);
        maus(f, QEvent::MouseButtonRelease, QPoint(mitteX, aY), Qt::NoButton);
        CHECK(f->gewaehlteNote() == 0);
        CHECK(staerke->value() == 100);
        staerke->setValue(70);
        CHECK(b.gemeldet.back() == QStringLiteral("Change strength"));
        CHECK(noten()[0].staerke == doctest::Approx(0.7));

        // ziehen: einen Halbton hoeher, die Zeit bleibt; mit Shift in Cent
        ziehe(f, QPoint(mitteX, aY), QPoint(mitteX, static_cast<int>(std::lround(f->yVonMidi(58.0)))));
        CHECK(b.gemeldet.back() == QStringLiteral("Move note"));
        CHECK(noten()[0].zielHoehe == 58);
        CHECK(noten()[0].anfang == doctest::Approx(0.5).epsilon(0.02));
        const int bY = static_cast<int>(std::lround(f->yVonMidi(58.0)));
        const int bY2 = static_cast<int>(std::lround(f->yVonMidi(58.3)));
        // erwartet: der Weg in Pixeln, in Cent umgerechnet (ein Pixel sind rund 10 Cent)
        const auto erwartet = static_cast<int>(std::lround((f->midiVonY(bY2) - f->midiVonY(bY)) * 100.0));
        ziehe(f, QPoint(mitteX, bY), QPoint(mitteX, bY2), Qt::ShiftModifier);
        CHECK(noten()[0].zielHoehe == 58);
        CHECK(noten()[0].zielCent == erwartet);
        CHECK(std::abs(erwartet - 30) <= 11);

        // Ctrl+Klick teilt, Join with next verbindet
        const int cY = static_cast<int>(std::lround(f->yVonMidi(58.0 + noten()[0].zielCent / 100.0)));
        maus(f, QEvent::MouseButtonPress, QPoint(mitteX, cY), Qt::LeftButton, Qt::ControlModifier);
        maus(f, QEvent::MouseButtonRelease, QPoint(mitteX, cY), Qt::NoButton, Qt::ControlModifier);
        CHECK(b.gemeldet.back() == QStringLiteral("Split note"));
        REQUIRE(noten().size() == 2);
        CHECK(noten()[0].ende == doctest::Approx(0.75).epsilon(0.02));
        QStringList eintraege;
        lumi::ui::setzeMenueBeobachterFuerTests([&](QMenu& menue) -> QAction* {
            QAction* verbinden = nullptr;
            for (QAction* a : menue.actions())
            {
                if (a->isSeparator()) continue;
                eintraege << a->text() + (a->isEnabled() ? QString() : QStringLiteral(" (off)"));
                if (a->text() == QStringLiteral("Join with next")) verbinden = a;
            }
            return verbinden;
        });
        const QPoint links(static_cast<int>(f->xVonDatei(0.6)), cY);
        QContextMenuEvent rechts(QContextMenuEvent::Mouse, links, f->mapToGlobal(links));
        QCoreApplication::sendEvent(f, &rechts);
        lumi::ui::setzeMenueBeobachterFuerTests({});
        CHECK(eintraege == QStringList{QStringLiteral("Split here"), QStringLiteral("Join with next"),
                                       QStringLiteral("Delete note"), QStringLiteral("Clear notes")});
        CHECK(b.gemeldet.back() == QStringLiteral("Join notes"));
        REQUIRE(noten().size() == 1);
        CHECK(noten()[0].ende == doctest::Approx(1.0).epsilon(0.02));

        // auf freier Flaeche aufziehen: eine neue Note mit Ton unter der Maus und der Vorgabe
        // der Ton, wie die Spur ihn vor dem Zug zeigt (danach zeigt sie auch die neue Note)
        const int neuTon = f->tonUnten() + 1;
        const int neuY = static_cast<int>(std::lround(f->yVonMidi(neuTon)));
        ziehe(f, QPoint(static_cast<int>(f->xVonDatei(1.2)), neuY), QPoint(static_cast<int>(f->xVonDatei(1.4)), neuY));
        CHECK(b.gemeldet.back() == QStringLiteral("Add note"));
        REQUIRE(noten().size() == 2);
        CHECK(noten()[1].zielHoehe == neuTon);
        CHECK(noten()[1].anfang == doctest::Approx(1.2).epsilon(0.02));
        CHECK(noten()[1].ende == doctest::Approx(1.4).epsilon(0.02));
        CHECK(noten()[1].staerke == 0.5);  // die Vorgabe von oben
        // ein Klick auf freie Flaeche legt keine an
        const int frei = static_cast<int>(f->xVonDatei(1.7));
        const int vorher = static_cast<int>(b.gemeldet.size());
        maus(f, QEvent::MouseButtonPress, QPoint(frei, neuY), Qt::LeftButton);
        maus(f, QEvent::MouseButtonRelease, QPoint(frei, neuY), Qt::NoButton);
        CHECK(static_cast<int>(b.gemeldet.size()) == vorher);

        // Entf nimmt die gewaehlte weg, ein Doppelklick die andere
        f->waehleNote(1);
        QKeyEvent entf(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
        QCoreApplication::sendEvent(f, &entf);
        CHECK(b.gemeldet.back() == QStringLiteral("Delete note"));
        REQUIRE(noten().size() == 1);
        const int dY = static_cast<int>(std::lround(f->yVonMidi(noten()[0].zielHoehe + noten()[0].zielCent / 100.0)));
        maus(f, QEvent::MouseButtonPress, QPoint(mitteX, dY), Qt::LeftButton);
        maus(f, QEvent::MouseButtonRelease, QPoint(mitteX, dY), Qt::NoButton);
        maus(f, QEvent::MouseButtonDblClick, QPoint(mitteX, dY), Qt::LeftButton);
        maus(f, QEvent::MouseButtonRelease, QPoint(mitteX, dY), Qt::NoButton);
        CHECK(noten().empty());

        // Selected section only: nur unter der gewaehlten Note neu
        e.setzeNurGewaehlte(false);
        CHECK(e.erkenneNoten());
        REQUIRE(noten().size() == 1);
        f->waehleNote(0);
        e.setzeNurGewaehlte(true);
        CHECK(e.erkenneNoten());
        CHECK(noten().size() == 1);

        const QString bild = qEnvironmentVariable("LUMIVIZ_WAVE_AUTOTUNE_BILD");
        if (!bild.isEmpty())
        {
            Loop zwei = b.zuletzt;
            Muster* w = musterIn(zwei, QStringLiteral("Voc"));
            w->wave.start = 0.0;
            w->wave.ende = 0.0;
            w->wave.versatz = 0.0;
            w->wave.autotune.abschnitte = {{0.1, 0.95, 57, 0, 1.0, 20.0}, {1.05, 1.9, 60, 0, 0.6, 80.0}};
            zwei.richte();
            e.setLoop(zwei);
            f->passeEin();
            f->waehleNote(1);
            CHECK(e.grab().save(bild));
        }
    }
}
