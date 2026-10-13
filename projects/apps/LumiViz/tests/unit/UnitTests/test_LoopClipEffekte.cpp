/**
 ****************************************************************************************
 * @file   test_LoopClipEffekte.cpp
 * @brief  Loop: das Fenster „Clip effects" und sein Weg ins Loop-Panel
 *         (docs/audio/Loop_Konzept.md §4g, Stufe 12d; §4j, Stufe 16d —
 *         die Zeile Signal path, Entscheide Patrik K3 a, K4 a, K5 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton und ohne gezeigtes Fenster. `LUMIVIZ_CLIP_EFFEKTE_BILD=<datei.png>`
 * legt zwei Bilder des Fensters ab: zwei Clips mit derselben Reihe, und (mit
 * `_differ` im Namen) drei Clips mit verschiedenen Reihen.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Bearbeiten.hpp"
#include "loop/ClipEffektFenster.hpp"
#include "loop/Loop.hpp"
#include "loop/LoopPanel.hpp"
#include "loop/SignalWeg.hpp"

#include <QPalette>

#include <algorithm>
#include <cmath>
#include "loop/Taktachse.hpp"
#include "services/CommandBus.hpp"
#include "services/ICommandBus.hpp"
#include "services/ServiceContainer.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QSettings>
#include <QSlider>
#include <QStackedWidget>

#include <memory>
#include <variant>
#include <vector>

using namespace lumi::loop;

namespace
{

/// Drums: A ab Takt 0 und noch einmal ab Takt 4; Lead: B ab Takt 0. 100 BPM.
Loop fensterLoop()
{
    Loop loop;
    loop.bpm = 100.0;
    Muster a;
    a.name = QStringLiteral("Beat");
    Muster b;
    b.name = QStringLiteral("Lead");
    loop.muster = {a, b};
    loop.kanaele = {{QStringLiteral("Drums"), true, {{QStringLiteral("Beat"), 0, 1}, {QStringLiteral("Beat"), 4, 1}}},
                    {QStringLiteral("Lead"), true, {{QStringLiteral("Lead"), 0, 1}}}};
    loop.richte();
    return loop;
}

const ClipEffekte& effekteIn(const Loop& loop, int kanal, int clip)
{
    return loop.kanaele[static_cast<std::size_t>(kanal)].clips[static_cast<std::size_t>(clip)].effekte;
}

ClipEffekte& effekteIn(Loop& loop, int kanal, int clip)
{
    return loop.kanaele[static_cast<std::size_t>(kanal)].clips[static_cast<std::size_t>(clip)].effekte;
}

void maus(QWidget& ziel, QEvent::Type art, QPoint ort, Qt::MouseButton taste, Qt::MouseButtons tasten)
{
    QMouseEvent ereignis(art, QPointF(ort), QPointF(ziel.mapToGlobal(ort)), taste, tasten, Qt::NoModifier);
    QCoreApplication::sendEvent(&ziel, &ereignis);
}

void klick(QWidget& ziel, QPoint ort)
{
    maus(ziel, QEvent::MouseButtonPress, ort, Qt::LeftButton, Qt::LeftButton);
    maus(ziel, QEvent::MouseButtonRelease, ort, Qt::LeftButton, Qt::NoButton);
}

void ziehe(QWidget& ziel, QPoint von, QPoint nach)
{
    maus(ziel, QEvent::MouseButtonPress, von, Qt::LeftButton, Qt::LeftButton);
    maus(ziel, QEvent::MouseMove, nach, Qt::NoButton, Qt::LeftButton);
    maus(ziel, QEvent::MouseButtonRelease, nach, Qt::LeftButton, Qt::NoButton);
}

template <class Art>
bool ist(const ClipBaustein& baustein)
{
    return std::holds_alternative<Art>(baustein);
}

}  // namespace

TEST_CASE("Loop Clip effects: das Fenster zeigt die Reihe der markierten Clips — Add, Haken, Move, Remove, Regler")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};

    ClipEffektFenster fenster;  // nicht gezeigt
    Loop loop = fensterLoop();
    int aenderungen = 0;
    QString zuletzt;
    QObject::connect(&fenster, &ClipEffektFenster::bearbeitet, &fenster, [&](const Loop& neu, const QString& was) {
        ++aenderungen;
        zuletzt = was;
        loop = neu;
        fenster.zeige(loop, fenster.orte());  // wie das Panel: die Fassung kommt zurueck
    });

    SignalWeg* weg = fenster.signalWeg();
    REQUIRE(weg != nullptr);
    weg->resize(1200, 40);  // breit genug fuer eine Zeile
    auto* kopf = fenster.findChild<QLabel*>(QStringLiteral("fxHead"));
    auto* clips = fenster.findChild<QLabel*>(QStringLiteral("fxClips"));
    auto* an = fenster.findChild<QCheckBox*>(QStringLiteral("fxBlockOn"));
    auto* seiten = fenster.findChild<QStackedWidget*>(QStringLiteral("fxPages"));
    auto* leerText = fenster.findChild<QLabel*>(QStringLiteral("fxEmpty"));
    auto* zeigtWahl = fenster.findChild<QComboBox*>(QStringLiteral("fxShowing"));
    auto* hinweisZeile = fenster.findChild<QWidget*>(QStringLiteral("fxDifferRow"));
    auto* hinweis = fenster.findChild<QLabel*>(QStringLiteral("fxDiffer"));
    auto* angleichen = fenster.findChild<QPushButton*>(QStringLiteral("fxUseForAll"));
    auto* zeit = fenster.findChild<QComboBox*>(QStringLiteral("fxEchoTime"));
    auto* zeitMs = fenster.findChild<QLabel*>(QStringLiteral("fxEchoTimeValue"));
    auto* feedback = fenster.findChild<QSlider*>(QStringLiteral("fxEchoFeedback"));
    auto* decay = fenster.findChild<QSlider*>(QStringLiteral("fxReverbDecay"));
    auto* decayWert = fenster.findChild<QLabel*>(QStringLiteral("fxReverbDecayValue"));
    auto* mix = fenster.findChild<QSlider*>(QStringLiteral("fxReverbMix"));
    auto* mixWert = fenster.findChild<QLabel*>(QStringLiteral("fxReverbMixValue"));
    auto* drive = fenster.findChild<QSlider*>(QStringLiteral("fxDistDrive"));
    auto* filterArt = fenster.findChild<QComboBox*>(QStringLiteral("fxFilterType"));
    auto* cutoff = fenster.findChild<QSlider*>(QStringLiteral("fxFilterCutoff"));
    auto* cutoffWert = fenster.findChild<QLabel*>(QStringLiteral("fxFilterCutoffValue"));
    auto* links = fenster.findChild<QPushButton*>(QStringLiteral("fxBlockLeft"));
    auto* rechts = fenster.findChild<QPushButton*>(QStringLiteral("fxBlockRight"));
    auto* weg2 = fenster.findChild<QPushButton*>(QStringLiteral("fxBlockRemove"));
    auto* kopieren = fenster.findChild<QPushButton*>(QStringLiteral("fxCopy"));
    auto* einfuegen = fenster.findChild<QPushButton*>(QStringLiteral("fxPaste"));
    auto* zurueck = fenster.findChild<QPushButton*>(QStringLiteral("fxReset"));
    for (QObject* teil : std::initializer_list<QObject*>{kopf, clips, an, seiten, leerText, zeigtWahl, hinweisZeile, hinweis, angleichen,
                                                         zeit, zeitMs, feedback, decay, decayWert, mix, mixWert, drive, filterArt,
                                                         cutoff, cutoffWert, links, rechts, weg2, kopieren, einfuegen, zurueck})
        REQUIRE(teil != nullptr);
    CHECK(fenster.findChild<QLabel*>(QStringLiteral("fxPathHint")) != nullptr);  // sagt, dass man klicken und ziehen kann
    const auto namen = [&] {
        QStringList alle;
        for (const WegKasten& einer : weg->kaesten()) alle << einer.titel;
        return alle;
    };

    SUBCASE("ohne Clip ist es gesperrt und sagt, was zu tun ist")
    {
        CHECK(kopf->text() == QStringLiteral("No clip selected"));
        CHECK(clips->text() == QStringLiteral("Select one or more clips in the Loop panel"));
        CHECK_FALSE(weg->isEnabled());
        CHECK(an->isHidden());
        CHECK_FALSE(zurueck->isEnabled());
        CHECK_FALSE(fenster.setzeZurueck());
        CHECK_FALSE(fenster.neuerBaustein(ClipEcho{}));
        fenster.zeige(loop, {{0, 9}});  // den Clip gibt es nicht
        CHECK(fenster.orte().empty());
        CHECK(aenderungen == 0);
    }
    SUBCASE("ein Clip: Add holt einen Baustein, seine Regler stehen darunter, jede Aenderung meldet eine neue Fassung")
    {
        fenster.zeige(loop, {{0, 4}});
        CHECK(kopf->text() == QString::fromUtf8("Signal path — 1 clip"));
        CHECK(clips->text() == QStringLiteral("Beat (bar 5)"));
        // ein Clip ohne Effekte: Clip, + Add, Out — und ein Satz, was zu tun ist
        CHECK(namen() == QStringList{QStringLiteral("Clip"), QStringLiteral("Out")});
        CHECK(weg->kaesten()[0].wert == QStringLiteral("dry"));
        CHECK(weg->isEnabled());
        CHECK(weg->addMoeglich());
        CHECK(fenster.stelle() == ClipEffektFenster::kStelleClip);
        CHECK(an->isHidden());
        CHECK(weg2->isHidden());
        CHECK(seiten->currentIndex() == 0);
        CHECK(leerText->text().startsWith(QStringLiteral("No effects on this clip.")));
        CHECK(hinweisZeile->isHidden());
        CHECK(zeigtWahl->isHidden());

        // --- Add: ein Echo, eingeschaltet und gewaehlt
        REQUIRE(fenster.neuerBaustein(ClipEcho{}));
        CHECK(aenderungen == 1);
        CHECK(zuletzt == QStringLiteral("Clip effects"));
        REQUIRE(effekteIn(loop, 0, 1).reihe.size() == 1);
        CHECK(effekteIn(loop, 0, 1).wert<ClipEcho>().an);
        CHECK(effekteIn(loop, 0, 0).reihe.empty());  // der andere Clip derselben Basis bleibt
        CHECK(namen() == QStringList{QStringLiteral("Clip"), QStringLiteral("Echo"), QStringLiteral("Out")});
        CHECK(fenster.stelle() == 0);
        CHECK(weg->gewaehlt() == 1);
        CHECK_FALSE(an->isHidden());
        CHECK(an->text() == QStringLiteral("Echo"));
        CHECK(an->checkState() == Qt::Checked);
        CHECK(seiten->currentIndex() == 3);  // die Seite Echo
        CHECK(weg->kaesten()[1].wert == QString::fromUtf8("1/8 dotted · 35 %"));
        CHECK(zeit->currentText() == QStringLiteral("1/8 dotted"));
        CHECK(zeitMs->text() == QStringLiteral("450 ms"));  // 0,75 Schlaege bei 100 BPM

        feedback->setValue(70);
        CHECK(effekteIn(loop, 0, 1).wert<ClipEcho>().feedback == doctest::Approx(0.7));
        CHECK(feedback->maximum() == 90);  // darueber klaenge ein Echo nie aus
        zeit->setCurrentIndex(zeit->findText(QStringLiteral("1/4")));
        Q_EMIT zeit->activated(zeit->currentIndex());
        CHECK(effekteIn(loop, 0, 1).wert<ClipEcho>().zeit == EchoZeit::Viertel);
        CHECK(zeitMs->text() == QStringLiteral("600 ms"));
        CHECK(weg->kaesten()[1].wert == QString::fromUtf8("1/4 · 35 %"));
        CHECK(aenderungen == 3);

        // --- ein Reverb kommt hinter den gewaehlten Baustein
        REQUIRE(fenster.neuerBaustein(ClipHall{}));
        CHECK(namen() == QStringList{QStringLiteral("Clip"), QStringLiteral("Echo"), QStringLiteral("Reverb"), QStringLiteral("Out")});
        CHECK(fenster.stelle() == 1);
        CHECK(an->text() == QStringLiteral("Reverb"));
        CHECK(seiten->currentIndex() == 4);
        CHECK(decay->value() == 18);  // 1,8 s, die Vorgabe
        CHECK(decayWert->text() == QStringLiteral("1.8 s"));
        decay->setValue(35);
        CHECK(effekteIn(loop, 0, 1).wert<ClipHall>().sekunden == doctest::Approx(3.5));
        CHECK(decayWert->text() == QStringLiteral("3.5 s"));
        CHECK(aenderungen == 5);
        // derselbe Wert noch einmal meldet nichts
        decay->setValue(35);
        CHECK(aenderungen == 5);
        // im Zug laeuft nur die Anzeige mit; gemeldet wird beim Loslassen
        mix->setSliderDown(true);
        mix->setValue(60);
        CHECK(mixWert->text() == QStringLiteral("60 %"));
        CHECK(aenderungen == 5);
        CHECK(effekteIn(loop, 0, 1).wert<ClipHall>().mix == doctest::Approx(0.25));
        mix->setSliderDown(false);  // loest sliderReleased aus
        CHECK(aenderungen == 6);
        CHECK(effekteIn(loop, 0, 1).wert<ClipHall>().mix == doctest::Approx(0.6));

        // --- der Haken: ausgeschaltet steht der Baustein matt in der Reihe und bleibt
        an->click();
        CHECK_FALSE(effekteIn(loop, 0, 1).wert<ClipHall>().an);
        CHECK(effekteIn(loop, 0, 1).reihe.size() == 2);
        CHECK(weg->kaesten()[2].matt);
        CHECK(effekteIn(loop, 0, 1).einerAn());  // das Echo ist noch an
        an->click();
        CHECK(effekteIn(loop, 0, 1).wert<ClipHall>().an);
        CHECK_FALSE(weg->kaesten()[2].matt);

        // --- ← Move, und ziehen zurueck
        CHECK(links->isEnabled());
        CHECK_FALSE(rechts->isEnabled());
        links->click();
        CHECK(namen() == QStringList{QStringLiteral("Clip"), QStringLiteral("Reverb"), QStringLiteral("Echo"), QStringLiteral("Out")});
        CHECK(fenster.stelle() == 0);
        CHECK(ist<ClipHall>(effekteIn(loop, 0, 1).reihe[0]));
        ziehe(*weg, weg->kastenRect(1).center(), weg->kastenRect(2).center());
        CHECK(namen() == QStringList{QStringLiteral("Clip"), QStringLiteral("Echo"), QStringLiteral("Reverb"), QStringLiteral("Out")});
        CHECK(fenster.stelle() == 1);
        // ein Klick auf einen Kasten waehlt ihn; die festen zeigen keine Regler
        klick(*weg, weg->kastenRect(1).center());
        CHECK(fenster.stelle() == 0);
        CHECK(an->text() == QStringLiteral("Echo"));
        klick(*weg, weg->kastenRect(3).center());
        CHECK(fenster.stelle() == ClipEffektFenster::kStelleOut);
        CHECK(an->isHidden());
        CHECK(seiten->currentIndex() == 0);
        CHECK(leerText->text() == QStringLiteral("Click a block in the path to see its controls."));

        // --- Remove
        klick(*weg, weg->kastenRect(2).center());
        weg2->click();
        CHECK(namen() == QStringList{QStringLiteral("Clip"), QStringLiteral("Echo"), QStringLiteral("Out")});
        CHECK(fenster.stelle() == 0);
        weg2->click();
        CHECK(effekteIn(loop, 0, 1) == ClipEffekte{});
        CHECK(fenster.stelle() == ClipEffektFenster::kStelleClip);
        CHECK_FALSE(fenster.entferneBaustein());

        // --- der Filter (K3 a): Type, die Grenze in Hertz, Resonance
        REQUIRE(fenster.neuerBaustein(ClipFilter{}));
        CHECK(effekteIn(loop, 0, 1).wert<ClipFilter>() == ClipFilter{true, FilterArt::Tiefpass, 1000.0, 0.0});
        CHECK(seiten->currentIndex() == 1);
        CHECK(an->text() == QStringLiteral("Filter"));
        CHECK(filterArt->currentText() == QStringLiteral("Low-pass"));
        CHECK(cutoffWert->text() == QStringLiteral("1.0 kHz"));
        CHECK(weg->kaesten()[1].wert == QStringLiteral("Low-pass 1.0 kHz"));
        cutoff->setValue(cutoff->minimum());
        CHECK(effekteIn(loop, 0, 1).wert<ClipFilter>().hz == doctest::Approx(kClipFilterHzMin));
        CHECK(cutoffWert->text() == QStringLiteral("40 Hz"));
        cutoff->setValue(cutoff->maximum());
        CHECK(effekteIn(loop, 0, 1).wert<ClipFilter>().hz == doctest::Approx(kClipFilterHzMax));
        CHECK(cutoffWert->text() == QStringLiteral("18.0 kHz"));
        filterArt->setCurrentIndex(filterArt->findText(QStringLiteral("High-pass")));
        Q_EMIT filterArt->activated(filterArt->currentIndex());
        CHECK(effekteIn(loop, 0, 1).wert<ClipFilter>().art == FilterArt::Hochpass);
        CHECK(weg->kaesten()[1].wert == QStringLiteral("High-pass 18.0 kHz"));

        // --- hoechstens acht (K4 a)
        while (effekteIn(loop, 0, 1).reihe.size() < static_cast<std::size_t>(kBausteineMax))
            REQUIRE(fenster.neuerBaustein(ClipVerzerrung{}));
        CHECK_FALSE(fenster.neuerBaustein(ClipHall{}));
        CHECK(effekteIn(loop, 0, 1).reihe.size() == 8);
        CHECK_FALSE(weg->addMoeglich());
        CHECK(weg->addText() == QStringLiteral("8 of 8"));
        CHECK(drive->value() == 30);  // die Vorgabe der Distortion
    }
    SUBCASE("mehrere Clips mit derselben Folge: verschiedene Werte stehen als mixed, ein Regler setzt alle")
    {
        effekteIn(loop, 0, 1).reihe = {ClipEcho{false, EchoZeit::Halbe, 0.4, 0.3, 0.35}, ClipHall{true, 0.5, 4.0, 0.4, 0.5}};
        effekteIn(loop, 1, 0).reihe = {ClipEcho{}, ClipHall{}};
        fenster.zeige(loop, {{0, 4}, {1, 0}});
        CHECK(kopf->text() == QString::fromUtf8("Signal path — 2 clips"));
        CHECK(clips->text() == QString::fromUtf8("Beat (bar 5) · Lead (bar 1)"));
        CHECK(fenster.gleiche() == std::vector<int>{0, 1});
        CHECK(hinweisZeile->isHidden());  // dieselbe Folge: kein Hinweis, keine Wahl
        CHECK(zeigtWahl->isHidden());
        CHECK(fenster.stelle() == 0);  // der erste Baustein ist gewaehlt
        CHECK(an->checkState() == Qt::Unchecked);
        CHECK(zeit->currentIndex() == -1);
        CHECK(zeitMs->text().isEmpty());
        // der Kurzwert sagt, welcher Teil abweicht: die Zeit, nicht der Mix
        // — und „≠“ davor, sobald irgendein Wert abweicht (Befund Patrik S119, X5 a)
        CHECK(weg->kaesten()[1].wert == QString::fromUtf8("≠ mixed · 35 %"));
        CHECK(weg->kaesten()[2].wert == QString::fromUtf8("≠ mixed"));  // Decay und Mix weichen beide ab
        CHECK(weg->kaesten()[1].matt);  // beim gezeigten Clip ausgeschaltet
        CHECK(weg->kaesten()[1].weichtAb);
        CHECK(weg->kaesten()[2].weichtAb);
        CHECK_FALSE(weg->kaesten()[0].weichtAb);  // Clip und Out nie
        CHECK_FALSE(weg->kaesten()[3].weichtAb);
        // der Tooltip nennt die Regler, wie sie im Fenster heissen
        CHECK(weg->kaesten()[1].hilfe ==
              QStringLiteral("Switched off: it stays in the path and is skipped\nDiffers between the clips: Time"));
        CHECK(weg->kaesten()[2].hilfe == QStringLiteral("Differs between the clips: On/off, Decay, Mix"));
        {
            // weicht nur ab, was der Kurzwert nicht zeigt (Damping, Feedback): der Kasten sagt es trotzdem
            Loop nur = loop;
            effekteIn(nur, 0, 1).reihe = {ClipEcho{true, EchoZeit::AchtelPunktiert, 0.16, 0.7, 0.3}};
            effekteIn(nur, 1, 0).reihe = {ClipEcho{true, EchoZeit::AchtelPunktiert, 0.45, 0.3, 0.3}};
            fenster.zeige(nur, {{0, 4}, {1, 0}});
            CHECK(weg->kaesten()[1].wert == QString::fromUtf8("≠ 1/8 dotted · 30 %"));
            CHECK(weg->kaesten()[1].weichtAb);
            CHECK(weg->kaesten()[1].hilfe == QStringLiteral("Differs between the clips: Feedback, Damping"));
            // gleichgestellt: das Zeichen ist weg
            effekteIn(nur, 0, 1).reihe = effekteIn(nur, 1, 0).reihe;
            fenster.zeige(nur, {{0, 4}, {1, 0}});
            CHECK(weg->kaesten()[1].wert == QString::fromUtf8("1/8 dotted · 30 %"));
            CHECK_FALSE(weg->kaesten()[1].weichtAb);
            CHECK(weg->kaesten()[1].hilfe.isEmpty());
            // ein Clip allein weicht von nichts ab
            fenster.zeige(nur, {{0, 4}});
            CHECK_FALSE(weg->kaesten()[1].weichtAb);
            fenster.zeige(loop, {{0, 4}, {1, 0}});
        }
        // die Flaeche ist getoent, und der Kurzwert bleibt darauf sauber zu lesen (Patrik S119):
        // der Abstand der Helligkeiten nach WCAG, im dunklen wie im hellen Design mindestens 7 zu 1
        const auto hell = [](const QColor& farbe) {
            const auto kanal = [](double wert) { return wert <= 0.03928 ? wert / 12.92 : std::pow((wert + 0.055) / 1.055, 2.4); };
            return 0.2126 * kanal(farbe.redF()) + 0.7152 * kanal(farbe.greenF()) + 0.0722 * kanal(farbe.blueF());
        };
        const auto abstand = [&hell](const QColor& a, const QColor& b) {
            const double x = hell(a);
            const double y = hell(b);
            return (std::max(x, y) + 0.05) / (std::min(x, y) + 0.05);
        };
        QPalette dunkel;
        dunkel.setColor(QPalette::Window, QColor(0x20, 0x20, 0x20));
        dunkel.setColor(QPalette::Base, QColor(0x2A, 0x2A, 0x2A));
        QPalette licht;
        licht.setColor(QPalette::Window, QColor(0xF0, 0xF0, 0xF0));
        licht.setColor(QPalette::Base, QColor(0xFF, 0xFF, 0xFF));
        for (const QPalette& farben : {dunkel, licht, weg->palette()})
        {
            CHECK(SignalWeg::abweichFlaeche(farben) != farben.color(QPalette::Base));
            const double lesbar = abstand(SignalWeg::abweichSchrift(farben), SignalWeg::abweichFlaeche(farben));
            MESSAGE("Kurzwert auf getoenter Flaeche: " << lesbar << " zu 1");
            CHECK(lesbar >= 7.0);
        }

        fenster.waehleStelle(1);
        CHECK(an->checkState() == Qt::PartiallyChecked);  // einer an, einer aus
        CHECK(decayWert->text() == QStringLiteral("mixed"));
        CHECK(mixWert->text() == QStringLiteral("mixed"));
        // ein Klick auf „teils an" schaltet alle an
        an->click();
        CHECK(effekteIn(loop, 0, 1).wert<ClipHall>().an);
        CHECK(effekteIn(loop, 1, 0).wert<ClipHall>().an);
        CHECK(an->checkState() == Qt::Checked);
        // der Regler setzt den Wert fuer beide; was er nicht beruehrt, bleibt verschieden
        decay->setValue(25);
        CHECK(effekteIn(loop, 0, 1).wert<ClipHall>().sekunden == doctest::Approx(2.5));
        CHECK(effekteIn(loop, 1, 0).wert<ClipHall>().sekunden == doctest::Approx(2.5));
        CHECK(decayWert->text() == QStringLiteral("2.5 s"));
        CHECK(mixWert->text() == QStringLiteral("mixed"));
        CHECK(effekteIn(loop, 0, 1).wert<ClipHall>().mix == doctest::Approx(0.5));
        CHECK(effekteIn(loop, 0, 0).reihe.empty());  // nicht markiert: unberuehrt
        // Add, Move und Remove gelten fuer beide
        REQUIRE(fenster.neuerBaustein(ClipFilter{}));
        CHECK(effekteIn(loop, 0, 1).reihe.size() == 3);
        CHECK(effekteIn(loop, 1, 0).reihe.size() == 3);
        REQUIRE(fenster.schiebeBaustein(-1));
        CHECK(ist<ClipFilter>(effekteIn(loop, 0, 1).reihe[1]));
        CHECK(ist<ClipFilter>(effekteIn(loop, 1, 0).reihe[1]));
        REQUIRE(fenster.entferneBaustein());
        CHECK(effekteIn(loop, 1, 0).reihe.size() == 2);
    }
    SUBCASE("mehrere Clips mit verschiedenen Reihen: die Zeile zeigt einen, gestellt werden die mit derselben Folge")
    {
        // Entscheid Patrik S118, K5 a
        effekteIn(loop, 0, 0).reihe = {ClipEcho{true, EchoZeit::Achtel, 0.4, 0.3, 0.35}};
        effekteIn(loop, 0, 1).reihe = {ClipEcho{true, EchoZeit::Viertel, 0.2, 0.3, 0.5}};
        effekteIn(loop, 1, 0).reihe = {ClipFilter{true, FilterArt::Tiefpass, 800.0, 0.0}, ClipHall{true, 0.5, 3.0, 0.4, 0.4}};
        const ClipEffekte anders = effekteIn(loop, 1, 0);
        fenster.zeige(loop, {{0, 0}, {0, 4}, {1, 0}});
        CHECK(kopf->text() == QString::fromUtf8("Signal path — 3 clips"));
        CHECK(fenster.zeigt() == 0);
        CHECK(fenster.gleiche() == std::vector<int>{0, 1});
        CHECK_FALSE(hinweisZeile->isHidden());
        CHECK(hinweis->text() == QStringLiteral("2 of 3 clips have this path.\nNot changed from here: Lead (bar 1)"));
        CHECK(angleichen->text() == QStringLiteral("Use this path for all 3"));
        CHECK_FALSE(zeigtWahl->isHidden());
        CHECK(zeigtWahl->count() == 3);
        CHECK(zeigtWahl->currentText() == QStringLiteral("Beat (bar 1)"));
        CHECK(namen() == QStringList{QStringLiteral("Clip"), QStringLiteral("Echo"), QStringLiteral("Out")});

        // ein Regler stellt die beiden mit dieser Folge; der dritte bleibt, wie er ist
        feedback->setValue(60);
        CHECK(effekteIn(loop, 0, 0).wert<ClipEcho>().feedback == doctest::Approx(0.6));
        CHECK(effekteIn(loop, 0, 1).wert<ClipEcho>().feedback == doctest::Approx(0.6));
        CHECK(effekteIn(loop, 1, 0) == anders);
        // Add auch
        REQUIRE(fenster.neuerBaustein(ClipHall{}));
        CHECK(effekteIn(loop, 0, 0).reihe.size() == 2);
        CHECK(effekteIn(loop, 0, 1).reihe.size() == 2);
        CHECK(effekteIn(loop, 1, 0) == anders);
        CHECK(fenster.gleiche() == std::vector<int>{0, 1});

        // --- Showing: der dritte Clip — seine Reihe, und nur er wird gestellt
        zeigtWahl->setCurrentIndex(2);
        Q_EMIT zeigtWahl->activated(2);
        CHECK(fenster.zeigt() == 2);
        CHECK(namen() == QStringList{QStringLiteral("Clip"), QStringLiteral("Filter"), QStringLiteral("Reverb"), QStringLiteral("Out")});
        CHECK(weg->kaesten()[1].wert == QStringLiteral("Low-pass 800 Hz"));
        CHECK(fenster.gleiche() == std::vector<int>{2});
        CHECK(hinweis->text() == QStringLiteral("1 of 3 clips has this path.\nNot changed from here: Beat (bar 1), Beat (bar 5)"));
        const ClipEffekte erster = effekteIn(loop, 0, 0);
        fenster.waehleStelle(1);
        decay->setValue(50);
        CHECK(effekteIn(loop, 1, 0).wert<ClipHall>().sekunden == doctest::Approx(5.0));
        CHECK(effekteIn(loop, 0, 0) == erster);

        // --- Copy, Paste und Reset gelten fuer alle; Copy merkt den gezeigten
        const int vorher = aenderungen;
        kopieren->click();
        CHECK(aenderungen == vorher);  // merken aendert nichts

        // --- Use this path for all: die gezeigte Reihe mit ihren Werten auf alle drei
        angleichen->click();
        CHECK(aenderungen == vorher + 1);
        CHECK(effekteIn(loop, 0, 0) == effekteIn(loop, 1, 0));
        CHECK(effekteIn(loop, 0, 1) == effekteIn(loop, 1, 0));
        CHECK(fenster.gleiche() == std::vector<int>{0, 1, 2});
        CHECK(hinweisZeile->isHidden());
        CHECK(zeigtWahl->isHidden());
        CHECK_FALSE(fenster.gleicheAn());  // sie haben sie schon

        // Paste nach einer Aenderung an einem: wieder alle gleich
        fenster.zeige(loop, {{0, 0}});
        REQUIRE(fenster.setzeZurueck());
        fenster.zeige(loop, {{0, 0}, {0, 4}, {1, 0}});
        CHECK_FALSE(hinweisZeile->isHidden());
        einfuegen->click();
        CHECK(effekteIn(loop, 0, 0) == effekteIn(loop, 1, 0));
        CHECK(hinweisZeile->isHidden());
        zurueck->click();
        CHECK(effekteIn(loop, 0, 0) == ClipEffekte{});
        CHECK(effekteIn(loop, 0, 1) == ClipEffekte{});
        CHECK(effekteIn(loop, 1, 0) == ClipEffekte{});
    }
    SUBCASE("Copy, Paste und Reset")
    {
        effekteIn(loop, 0, 0).reihe = {ClipVerzerrung{true, 0.8, 0.2, 0.6}};
        fenster.zeige(loop, {{0, 0}});
        CHECK_FALSE(einfuegen->isEnabled());  // noch nichts gemerkt
        CHECK(drive->value() == 80);
        kopieren->click();
        CHECK(einfuegen->isEnabled());
        CHECK(aenderungen == 0);  // merken aendert nichts

        fenster.zeige(loop, {{0, 4}, {1, 0}});
        einfuegen->click();
        CHECK(aenderungen == 1);
        CHECK(effekteIn(loop, 0, 1) == effekteIn(loop, 0, 0));
        CHECK(effekteIn(loop, 1, 0) == effekteIn(loop, 0, 0));
        fenster.waehleStelle(0);
        CHECK(drive->value() == 80);

        zurueck->click();
        CHECK(aenderungen == 2);
        CHECK(effekteIn(loop, 0, 1) == ClipEffekte{});
        CHECK(effekteIn(loop, 1, 0) == ClipEffekte{});
        CHECK(effekteIn(loop, 0, 0).wert<ClipVerzerrung>().an);  // nicht markiert
        zurueck->click();  // schon zurueckgesetzt: nichts zu melden
        CHECK(aenderungen == 2);
    }
    SUBCASE("die Spitze steht im Kopf und im Kasten Out")
    {
        effekteIn(loop, 0, 1).reihe = {ClipHall{true, 0.55, 1.8, 0.4, 0.25}};
        fenster.zeige(loop, {{0, 4}});
        CHECK(weg->kaesten().back().wert == QString::fromUtf8("–"));
        fenster.setSpitze(0.62);
        CHECK(weg->kaesten().back().wert == QStringLiteral("Peak 0.62"));
        CHECK(fenster.stelle() == 0);  // die Wahl bleibt
        fenster.setSpitze(-1.0);
        CHECK(weg->kaesten().back().wert == QString::fromUtf8("–"));
    }
    SUBCASE("das Bild des Fensters")
    {
        effekteIn(loop, 0, 1).reihe = {ClipEcho{true, EchoZeit::AchtelPunktiert, 0.45, 0.3, 0.35}, ClipHall{true, 0.55, 1.8, 0.4, 0.25}};
        effekteIn(loop, 1, 0).reihe = {ClipEcho{true, EchoZeit::AchtelPunktiert, 0.45, 0.3, 0.2}, ClipHall{true, 0.55, 3.0, 0.4, 0.25}};
        fenster.zeige(loop, {{0, 4}, {1, 0}});
        fenster.setSpitze(0.62);
        fenster.setAttribute(Qt::WA_DontShowOnScreen);
        fenster.show();  // das Layout steht erst nach dem Zeigen; auf den Bildschirm kommt nichts
        QCoreApplication::processEvents();
        const QImage bild = fenster.grab().toImage();
        CHECK_FALSE(bild.isNull());
        const QString datei = qEnvironmentVariable("LUMIVIZ_CLIP_EFFEKTE_BILD");
        if (!datei.isEmpty()) CHECK(bild.save(datei));
        // drei Clips, einer mit einer anderen Reihe
        effekteIn(loop, 0, 0).reihe = {ClipFilter{true, FilterArt::Tiefpass, 800.0, 0.0}, ClipHall{true, 0.5, 3.0, 0.4, 0.4},
                                       ClipEcho{false, EchoZeit::Viertel, 0.4, 0.3, 0.2}};
        fenster.zeige(loop, {{0, 4}, {1, 0}, {0, 0}});
        // der Hinweis ist dazugekommen: das Fenster waechst erst, wenn das Layout neu gerechnet hat
        for (int i = 0; i < 3; ++i) QCoreApplication::processEvents();
        if (!datei.isEmpty())
        {
            const QFileInfo info(datei);
            CHECK(fenster.grab().save(info.dir().filePath(info.completeBaseName() + QStringLiteral("_differ.") + info.suffix())));
        }
    }
}

TEST_CASE("Loop Clip effects im Panel: Effects oeffnet das Fenster, es folgt den Marken, Undo holt zurueck")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
    // eigene Einstellungen: das Panel merkt sich die zuletzt geladene Datei
    const QString org = QCoreApplication::organizationName();
    const QString name = QCoreApplication::applicationName();
    QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
    QCoreApplication::setApplicationName(QStringLiteral("LoopClipEffekte"));
    QSettings().clear();
    {
        ServiceContainer dienste;
        dienste.registerSingleton<ICommandBus>([](ServiceContainer&) { return std::make_unique<CommandBus>(); });
        ICommandBus* bus = dienste.tryResolve<ICommandBus>();
        REQUIRE(bus != nullptr);

        LoopPanel panel(dienste);  // nicht gezeigt: kein Fenster
        Taktachse* achse = panel.taktachse();
        REQUIRE(achse != nullptr);
        CHECK_FALSE(panel.oeffneEffekte(false));  // kein Loop geladen
        CHECK(panel.effektFenster() == nullptr);

        panel.neu();
        REQUIRE(achse->neuerClip(0, 0));
        REQUIRE(achse->dupliziereAuswahl());  // zwei Clips derselben Basis: Takt 0 und Takt 4
        CHECK_FALSE(achse->markierte().empty());

        // „Effects..." im Menue des Clips
        int verlangt = 0;
        QObject::connect(achse, &Taktachse::effekteVerlangt, achse, [&verlangt] { ++verlangt; });
        achse->markiere({});
        CHECK_FALSE(achse->verlangeEffekte());  // ohne markierten Clip
        achse->markiere({{0, 4}});
        // (das Signal oeffnete das Fenster sichtbar — hier ohne Fenster)
        REQUIRE(panel.oeffneEffekte(false));
        ClipEffektFenster* fenster = panel.effektFenster();
        REQUIRE(fenster != nullptr);
        CHECK(fenster->orte() == std::vector<ClipOrt>{{0, 4}});

        auto* an = fenster->findChild<QCheckBox*>(QStringLiteral("fxBlockOn"));
        auto* kopf = fenster->findChild<QLabel*>(QStringLiteral("fxHead"));
        auto* hinweisZeile = fenster->findChild<QWidget*>(QStringLiteral("fxDifferRow"));
        auto* clipZeile = panel.findChild<QLabel*>(QStringLiteral("loopClip"));
        REQUIRE(an != nullptr);
        REQUIRE(kopf != nullptr);
        REQUIRE(hinweisZeile != nullptr);
        REQUIRE(clipZeile != nullptr);

        // eine Aenderung im Fenster laeuft ueber den CommandBus des Panels
        REQUIRE(fenster->neuerBaustein(ClipHall{}));
        CHECK(panel.loop().kanaele[0].clips[1].effekte.wert<ClipHall>().an);
        CHECK(panel.loop().kanaele[0].clips[0].effekte.reihe.empty());
        CHECK(bus->undoDescription() == "Clip effects");
        CHECK(panel.spieler().loop() == panel.loop());  // was gespielt wuerde, hat den Effekt
        CHECK(panel.ungespeichert());
        CHECK(an->checkState() == Qt::Checked);
        CHECK(fenster->stelle() == 0);  // die Fassung kam zurueck: der neue Baustein ist noch gewaehlt

        // das Fenster folgt den Marken: zwei Clips, einer ohne Effekte — verschiedene Reihen (K5 a)
        achse->markiere({{0, 0}, {0, 4}});
        CHECK(kopf->text() == QString::fromUtf8("Signal path — 2 clips"));
        CHECK(fenster->gleiche().size() == 1);
        CHECK_FALSE(hinweisZeile->isHidden());
        CHECK(clipZeile->text().startsWith(QStringLiteral("2 clips selected · 1 with effects")));
        achse->markiere({});
        CHECK(kopf->text() == QStringLiteral("No clip selected"));
        CHECK(clipZeile->text().contains(QStringLiteral("Ctrl+click")));

        // Undo: der Effekt ist wieder weg, das Fenster zeigt es
        achse->markiere({{0, 4}});
        CHECK(fenster->gezeigteReihe().wert<ClipHall>().an);
        REQUIRE(bus->undo());
        CHECK(panel.loop().kanaele[0].clips[1].effekte.reihe.empty());
        CHECK(fenster->gezeigteReihe().reihe.empty());
        CHECK(an->isHidden());
        REQUIRE(bus->redo());
        CHECK(fenster->gezeigteReihe().wert<ClipHall>().an);

        // die Effekte reisen mit dem Clip: verschieben und kopieren im Panel
        std::vector<ClipOrt> orte = {{0, 4}};
        Loop verschoben = panel.loop();
        REQUIRE(versetzeClips(verschoben, orte, 0, 8));
        panel.bearbeite(verschoben, QStringLiteral("Move clip"));
        CHECK(panel.loop().kanaele[0].clips[1].startTakt == 12);
        CHECK(panel.loop().kanaele[0].clips[1].effekte.wert<ClipHall>().an);

        // New: das Fenster hat keinen Clip mehr
        panel.neu();
        CHECK(kopf->text() == QStringLiteral("No clip selected"));
        (void)verlangt;
    }
    QSettings().clear();
    QCoreApplication::setOrganizationName(org);
    QCoreApplication::setApplicationName(name);
}
