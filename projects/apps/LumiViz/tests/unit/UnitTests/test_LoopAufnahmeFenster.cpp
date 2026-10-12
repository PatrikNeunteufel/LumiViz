/**
 * @file test_LoopAufnahmeFenster.cpp
 * @brief Loop, Stufe 11b, Schritt 2: das Fenster „Record sample" — aufnehmen,
 *        zuschneiden, in Dauerschleife anhoeren, speichern
 *        (docs/audio/Loop_Konzept.md §4h; Skizze gebilligt, Patrik S115)
 *
 * Ohne Ton, ohne Eingang, ohne gezeigtes Fenster: die Engine ist aus Pappe —
 * sie „nimmt auf", was der Test ihr gibt, und reicht die Quelle heraus, die
 * den Ausschnitt spielt. Eigene Samples liegen in einem Wegwerf-Ordner.
 * `LUMIVIZ_RECORD_SAMPLE_BILD=<datei.png>` legt ein Bild des Fensters ab.
 */

#include <doctest.h>

#include "PappAufnahme.hpp"
#include "loop/Aufnahme.hpp"
#include "loop/AufnahmeFenster.hpp"
#include "loop/InstrumentCreator.hpp"  // KurvenBild
#include "loop/Sample.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QProgressBar>
#include <QPushButton>
#include <QSettings>
#include <QTemporaryDir>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

using namespace lumi::loop;

namespace
{

constexpr double kPi = 3.14159265358979323846;

using papp::AufnahmeEngine;
using papp::eingang;

/// 0,3 s Stille, 1,5 s Ton, 0,2 s Stille — zwei Sekunden in 48000; rechts halb so laut.
std::vector<float> probe(float pegel = 0.4f)
{
    std::vector<float> daten(2 * 96000, 0.0f);
    for (std::size_t t = 14400; t < 86400; ++t)
    {
        const auto wert = pegel * static_cast<float>(std::sin(2.0 * kPi * 330.0 * static_cast<double>(t) / 48000.0 + 1.0));
        daten[2 * t] = wert;
        daten[2 * t + 1] = 0.5f * wert;
    }
    return daten;
}

void maus(QWidget& ziel, QEvent::Type art, QPoint ort, Qt::MouseButton taste, Qt::MouseButtons tasten)
{
    QMouseEvent ereignis(art, QPointF(ort), QPointF(ziel.mapToGlobal(ort)), taste, tasten, Qt::NoModifier);
    QCoreApplication::sendEvent(&ziel, &ereignis);
}

void ziehe(QWidget& ziel, QPoint von, QPoint nach)
{
    maus(ziel, QEvent::MouseButtonPress, von, Qt::LeftButton, Qt::LeftButton);
    maus(ziel, QEvent::MouseMove, nach, Qt::NoButton, Qt::LeftButton);
    maus(ziel, QEvent::MouseButtonRelease, nach, Qt::LeftButton, Qt::NoButton);
}

/// Qt-Anwendung, eigene Einstellungen, eigene Samples in einem Wegwerf-Ordner.
struct Umgebung
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
    QString org = QCoreApplication::organizationName();
    QString name = QCoreApplication::applicationName();
    QTemporaryDir tmp;
    QString ordner;

    Umgebung()
    {
        REQUIRE(tmp.isValid());
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("LoopRecordSample"));
        QSettings().clear();
        ordner = QDir(tmp.path()).filePath(QStringLiteral("samples"));
        setzeEigenenSampleOrdnerFuerTests(ordner);
        vergissSampleDaten();
    }
    ~Umgebung()
    {
        QSettings().clear();
        setzeEigenenSampleOrdnerFuerTests(QString());
        vergissSampleDaten();
        QCoreApplication::setOrganizationName(org);
        QCoreApplication::setApplicationName(name);
    }
};

float spitzeVon(const std::vector<float>& daten)
{
    float groesster = 0.0f;
    for (const float wert : daten) groesster = std::max(groesster, std::abs(wert));
    return groesster;
}

}  // namespace

TEST_SUITE("Loop Aufnahme Fenster")
{
    TEST_CASE("Loop Record sample: die Quelle spielt den Ausschnitt — einmal oder in Dauerschleife, mit Blende an den Enden")
    {
        auto daten = std::make_shared<const std::vector<float>>(probe());
        AusschnittQuelle quelle;
        std::vector<float> aus(2 * 4800);

        // ohne Daten und vor dem Start: Stille
        std::fill(aus.begin(), aus.end(), 1.0f);
        quelle.render(aus.data(), 4800);
        CHECK(spitzeVon(aus) == 0.0f);
        CHECK(quelle.fertig());

        // 0,1 s mitten im Ton, in derselben Rate: einmal, dann still und fertig
        quelle.setzeDaten(daten, 48000, 48000);
        quelle.setzeAusschnitt({24000, 28800});
        quelle.starte();
        CHECK_FALSE(quelle.fertig());
        quelle.render(aus.data(), 4800);
        CHECK(aus[0] == 0.0f);                                      // die Blende beginnt bei null
        CHECK(aus[2 * 2400] == doctest::Approx((*daten)[2 * (24000 + 2400)]));
        CHECK(aus[2 * 2400 + 1] == doctest::Approx((*daten)[2 * (24000 + 2400) + 1]));  // rechts bleibt rechts
        CHECK(std::abs(aus[2 * 4798]) < 0.02f);                     // und endet bei null
        quelle.render(aus.data(), 4800);
        CHECK(spitzeVon(aus) == 0.0f);
        CHECK(quelle.fertig());

        // in Dauerschleife: nach dem Ende beginnt der Ausschnitt wieder
        quelle.setzeWiederholen(true);
        quelle.starte();
        quelle.render(aus.data(), 4800);
        quelle.render(aus.data(), 4800);
        CHECK_FALSE(quelle.fertig());
        CHECK(spitzeVon(aus) > 0.3f);
        CHECK(quelle.stelle() >= 24000);
        CHECK(quelle.stelle() < 28800);

        // der Ausschnitt aendert sich unter dem Spielen: die Quelle folgt im naechsten Block
        quelle.setzeAusschnitt({48000, 52800});
        quelle.render(aus.data(), 4800);
        CHECK(quelle.stelle() >= 48000);
        CHECK(quelle.stelle() <= 52800);

        // Normalize beim Anhoeren, und eine andere Ausgaberate: halb so viele Abtastwerte reichen
        quelle.setzeWiederholen(false);
        quelle.setzeVerstaerkung(2.0f);
        quelle.setzeDaten(daten, 48000, 24000);
        quelle.setzeAusschnitt({24000, 28800});
        quelle.starte();
        quelle.render(aus.data(), 2400);
        CHECK(spitzeVon(aus) == doctest::Approx(0.8f).epsilon(0.02));
        CHECK_FALSE(quelle.fertig());
        quelle.render(aus.data(), 2400);
        CHECK(quelle.fertig());

        // ein Ausschnitt ueber die Aufnahme hinaus, oder ohne Inhalt: kein Absturz, Stille
        quelle.setzeVerstaerkung(1.0f);
        quelle.setzeAusschnitt({95000, 99999999});
        quelle.starte();
        quelle.render(aus.data(), 4800);
        CHECK(spitzeVon(aus) == 0.0f);
        quelle.setzeAusschnitt({5000, 5000});
        quelle.starte();
        quelle.render(aus.data(), 4800);
        CHECK(spitzeVon(aus) == 0.0f);
    }

    TEST_CASE("Loop Record sample: aufnehmen — Eingaenge, Record und Stop, das Bild mit den beiden Griffen")
    {
        Umgebung u;
        AufnahmeEngine engine;
        engine.eingaenge = {eingang(0, QStringLiteral("Default"), true), eingang(1, QStringLiteral("Microphone Array"), true),
                            eingang(2, QStringLiteral("Lautsprecher"), false, true)};
        engine.aufnahme = probe();

        AufnahmeFenster fenster;  // nicht gezeigt: kein Fenster
        auto* wahl = fenster.findChild<QComboBox*>(QStringLiteral("recordInput"));
        auto* record = fenster.findChild<QPushButton*>(QStringLiteral("recordStart"));
        auto* stop = fenster.findChild<QPushButton*>(QStringLiteral("recordStop"));
        auto* stand = fenster.findChild<QLabel*>(QStringLiteral("recordState"));
        auto* pegel = fenster.findChild<QProgressBar*>(QStringLiteral("recordLevel"));
        auto* bild = fenster.findChild<KurvenBild*>(QStringLiteral("recordPicture"));
        auto* play = fenster.findChild<QPushButton*>(QStringLiteral("recordPlay"));
        auto* wiederholen = fenster.findChild<QCheckBox*>(QStringLiteral("recordRepeat"));
        auto* schnitt = fenster.findChild<QLabel*>(QStringLiteral("recordTrimmed"));
        auto* ohneStilleHaken = fenster.findChild<QCheckBox*>(QStringLiteral("recordTrimSilence"));
        auto* normal = fenster.findChild<QCheckBox*>(QStringLiteral("recordNormalize"));
        auto* name = fenster.findChild<QLineEdit*>(QStringLiteral("recordName"));
        auto* datei = fenster.findChild<QLabel*>(QStringLiteral("recordFile"));
        auto* sichern = fenster.findChild<QPushButton*>(QStringLiteral("recordSave"));
        auto* hinweis = fenster.findChild<QLabel*>(QStringLiteral("recordHint"));
        for (QObject* teil : std::initializer_list<QObject*>{wahl, record, stop, stand, pegel, bild, play, wiederholen, schnitt,
                                                              ohneStilleHaken, normal, name, datei, sichern, hinweis})
            REQUIRE(teil != nullptr);
        CHECK(fenster.windowTitle() == QStringLiteral("Record sample"));

        // --- ohne Engine: nichts geht, und die Zeile sagt es
        CHECK_FALSE(record->isEnabled());
        CHECK_FALSE(play->isEnabled());
        CHECK_FALSE(sichern->isEnabled());
        CHECK(stand->text() == QStringLiteral("No audio engine — recording is not available"));
        CHECK_FALSE(fenster.starteAufnahme());

        // --- mit Engine: die Eingaenge; gewaehlt ist der Standard, der Mitschnitt ist bezeichnet
        fenster.setEngine(&engine);
        fenster.beginne();
        REQUIRE(wahl->count() == 3);
        CHECK(wahl->currentText() == QStringLiteral("Default"));
        CHECK(wahl->itemText(2) == QStringLiteral("Lautsprecher — what the computer plays"));
        CHECK(record->isEnabled());
        CHECK_FALSE(stop->isEnabled());
        CHECK(stand->text() == QStringLiteral("Press Record — up to 30 s"));
        CHECK(name->text() == QStringLiteral("Recording"));
        CHECK(datei->text() == QStringLiteral("File: My files · samples · recording.wav"));
        CHECK_FALSE(sichern->isEnabled());  // noch nichts aufgenommen
        CHECK(hinweis->isHidden());
        CHECK(bild->aussenGrau());
        CHECK(bild->gespiegelt());

        // --- Record: vom gewaehlten Eingang, 48000 Hz, hoechstens 30 Sekunden
        wahl->setCurrentIndex(1);
        Q_EMIT wahl->activated(1);
        record->click();
        CHECK(fenster.nimmtAuf());
        CHECK(engine.rufe == QStringList{QStringLiteral("start 1 48000 30")});
        CHECK_FALSE(record->isEnabled());
        CHECK(stop->isEnabled());
        CHECK_FALSE(wahl->isEnabled());
        CHECK_FALSE(play->isEnabled());
        engine.angekommen = 57600;
        engine.pegel = 0.5f;
        // der Takt des Fensters liest Menge und Pegel
        fenster.takt();
        CHECK(stand->text() == QStringLiteral("1.2 s recorded · up to 30 s"));
        CHECK(pegel->value() > 80);  // -6 dB auf einer Leiste von -48 bis 0
        CHECK(pegel->value() < 95);

        // --- Stop: die Aufnahme steht im Bild, die Griffe an den Raendern des Tons
        engine.angekommen = 96000;
        stop->click();
        CHECK_FALSE(fenster.nimmtAuf());
        CHECK(engine.rufe.last() == QStringLiteral("stop"));
        CHECK(fenster.frames() == 96000);
        CHECK(fenster.rate() == 48000);
        CHECK(stand->text() == QStringLiteral("2.0 s recorded"));
        CHECK(pegel->value() == 0);
        CHECK(bild->werte().size() == 600);
        CHECK(ohneStilleHaken->isChecked());
        CHECK(fenster.ausschnitt() == ohneStille(engine.aufnahme, 48000));
        CHECK(fenster.ausschnitt().von == doctest::Approx(14400 - 240).epsilon(0.01));
        CHECK(fenster.ausschnitt().bis == doctest::Approx(86400 + 1440).epsilon(0.01));
        CHECK(bild->griff(KurvenBild::GriffVon) == doctest::Approx(0.1475).epsilon(0.02));
        CHECK(bild->griff(KurvenBild::GriffBis) == doctest::Approx(0.915).epsilon(0.01));
        CHECK(bild->griff(KurvenBild::GriffStart) < 0.0);
        CHECK(schnitt->text().startsWith(QStringLiteral("Trimmed: 0.")));
        CHECK(schnitt->text().contains(QStringLiteral(" s to 1.83 s · 1.5")));
        CHECK_FALSE(hinweis->isHidden());
        CHECK(play->isEnabled());
        CHECK(sichern->isEnabled());
        CHECK(record->isEnabled());

        // --- die Griffe ziehen: der Anfang nach rechts, das Ende nach links; sie kreuzen sich nicht
        bild->resize(500, 120);
        ziehe(*bild, QPoint(bild->xVon(bild->griff(KurvenBild::GriffVon)), 50), QPoint(bild->xVon(0.5), 50));
        CHECK(fenster.ausschnitt().von == doctest::Approx(48000).epsilon(0.01));
        ziehe(*bild, QPoint(bild->xVon(bild->griff(KurvenBild::GriffBis)), 50), QPoint(bild->xVon(0.75), 50));
        CHECK(fenster.ausschnitt().bis == doctest::Approx(72000).epsilon(0.01));
        ziehe(*bild, QPoint(bild->xVon(bild->griff(KurvenBild::GriffBis)), 50), QPoint(bild->xVon(0.1), 50));
        CHECK(fenster.ausschnitt().bis == fenster.ausschnitt().von + 480);  // nicht kuerzer als 10 ms
        fenster.setzeAusschnitt({48000, 72000});
        CHECK(schnitt->text() == QStringLiteral("Trimmed: 1.00 s to 1.50 s · 0.50 s"));

        // --- Trim silence aus: die ganze Aufnahme; wieder an: die Raender des Tons
        ohneStilleHaken->setChecked(false);
        CHECK(fenster.ausschnitt() == Ausschnitt{0, 96000});
        ohneStilleHaken->setChecked(true);
        CHECK(fenster.ausschnitt() == ohneStille(engine.aufnahme, 48000));
        fenster.setzeAusschnitt({-5, 99999999});
        CHECK(fenster.ausschnitt() == Ausschnitt{0, 96000});  // begrenzt

        // --- was gespeichert wuerde: der Ausschnitt; Normalize hebt ihn
        fenster.setzeAusschnitt({48000, 72000});
        CHECK(fenster.ergebnis() == schneideAus(engine.aufnahme, 48000, {48000, 72000}, false));
        normal->setChecked(true);
        CHECK(spitzeVon(fenster.ergebnis()) == doctest::Approx(0.9f).epsilon(0.001));
        normal->setChecked(false);

        // --- eine neue Aufnahme ersetzt die alte
        engine.aufnahme = probe(0.2f);
        REQUIRE(fenster.starteAufnahme());
        CHECK_FALSE(play->isEnabled());
        engine.angekommen = 96000;
        engine.laeuft = false;  // die hoechste Laenge ist erreicht: die Engine hoert von selbst auf
        fenster.takt();
        CHECK_FALSE(fenster.nimmtAuf());
        CHECK(spitzeVon(fenster.ergebnis()) == doctest::Approx(0.2f).epsilon(0.01));

        // --- der Eingang laesst sich nicht oeffnen, oder es kommt nichts: die Zeile sagt es
        engine.oeffnet = false;
        CHECK_FALSE(fenster.starteAufnahme());
        CHECK(stand->text().startsWith(QStringLiteral("Could not open this input.")));
        CHECK_FALSE(fenster.nimmtAuf());
        CHECK(fenster.frames() == 96000);  // die vorige Aufnahme bleibt
        engine.oeffnet = true;
        REQUIRE(fenster.starteAufnahme());
        engine.angekommen = 0;
        REQUIRE(fenster.stoppeAufnahme());
        CHECK(stand->text() == QStringLiteral("Nothing arrived from this input."));
        CHECK(fenster.frames() == 96000);
        CHECK_FALSE(fenster.stoppeAufnahme());  // es laeuft keine mehr

        // --- der gewaehlte Eingang ist gemerkt
        AufnahmeFenster zweites;
        zweites.setEngine(&engine);
        zweites.beginne();
        CHECK(zweites.findChild<QComboBox*>(QStringLiteral("recordInput"))->currentText() == QStringLiteral("Microphone Array"));

        // --- ohne Eingang
        AufnahmeEngine leer;
        AufnahmeFenster drittes;
        drittes.setEngine(&leer);
        drittes.beginne();
        CHECK(drittes.findChild<QLabel*>(QStringLiteral("recordState"))->text() == QStringLiteral("No input found"));
        CHECK_FALSE(drittes.findChild<QPushButton*>(QStringLiteral("recordStart"))->isEnabled());
        CHECK_FALSE(drittes.starteAufnahme());
    }

    TEST_CASE("Loop Record sample: Play und Repeat — der Ausschnitt folgt den Griffen; Save and use legt das Sample ab")
    {
        Umgebung u;
        AufnahmeEngine engine;
        engine.eingaenge = {eingang(0, QStringLiteral("Default"), true)};
        const std::vector<float> aufnahme = probe();

        AufnahmeFenster fenster;
        fenster.setEngine(&engine);
        fenster.beginne();
        QStringList gespeichert;
        QObject::connect(&fenster, &AufnahmeFenster::gespeichert, &fenster, [&](const QString& nennung) { gespeichert << nennung; });
        auto* play = fenster.findChild<QPushButton*>(QStringLiteral("recordPlay"));
        auto* wiederholen = fenster.findChild<QCheckBox*>(QStringLiteral("recordRepeat"));
        auto* bild = fenster.findChild<KurvenBild*>(QStringLiteral("recordPicture"));
        auto* name = fenster.findChild<QLineEdit*>(QStringLiteral("recordName"));
        auto* datei = fenster.findChild<QLabel*>(QStringLiteral("recordFile"));
        auto* sichern = fenster.findChild<QPushButton*>(QStringLiteral("recordSave"));

        CHECK_FALSE(fenster.spiele());  // nichts aufgenommen
        CHECK(fenster.speichere().isEmpty());
        fenster.setzeAufnahme(aufnahme, 48000);
        fenster.setzeAusschnitt({24000, 28800});

        // --- Play: ein Mixer mit der Quelle, sie laeuft; am Ende haelt das Fenster von selbst
        play->click();
        REQUIRE(fenster.spielt());
        CHECK(engine.protokoll == QStringList{QStringLiteral("mixer"), QStringLiteral("add generator"), QStringLiteral("run <generator>")});
        REQUIRE(engine.generator == &fenster.quelle());
        CHECK(play->isChecked());
        CHECK(play->text() == QStringLiteral("■ Stop playing"));
        std::vector<float> aus(2 * 2400);
        engine.generator->render(aus.data(), 2400);
        CHECK(spitzeVon(aus) > 0.3f);
        fenster.takt();
        CHECK(bild->zeiger() == doctest::Approx(26400.0 / 96000.0).epsilon(0.01));  // die blaue Linie
        engine.generator->render(aus.data(), 2400);
        engine.generator->render(aus.data(), 2400);
        CHECK(fenster.quelle().fertig());
        fenster.takt();
        CHECK_FALSE(fenster.spielt());
        CHECK(engine.protokoll.last() == QStringLiteral("free mixer"));
        CHECK_FALSE(play->isChecked());
        CHECK(play->text() == QStringLiteral("▶ Play"));
        CHECK(bild->zeiger() < 0.0);

        // --- Repeat: es hoert nicht von selbst auf, und der Ausschnitt folgt einem gezogenen Griff
        wiederholen->setChecked(true);
        REQUIRE(fenster.spiele());
        for (int i = 0; i < 6; ++i) engine.generator->render(aus.data(), 2400);
        fenster.takt();
        CHECK(fenster.spielt());
        bild->resize(500, 120);
        fenster.setzeAusschnitt({48000, 52800});
        engine.generator->render(aus.data(), 2400);
        CHECK(fenster.quelle().stelle() >= 48000);
        CHECK(fenster.quelle().stelle() <= 52800);
        // Play noch einmal gedrueckt: Stop
        play->click();
        CHECK_FALSE(fenster.spielt());
        // waehrend einer Aufnahme wird nicht gespielt
        REQUIRE(fenster.spiele());
        REQUIRE(fenster.starteAufnahme());
        CHECK_FALSE(fenster.spielt());
        CHECK_FALSE(fenster.spiele());
        engine.aufnahme = aufnahme;
        engine.angekommen = 96000;
        REQUIRE(fenster.stoppeAufnahme());

        // --- der Name und die Datei, die daraus wird
        name->setText(QStringLiteral("Table Knock"));
        CHECK(datei->text() == QStringLiteral("File: My files · samples · table_knock.wav"));
        name->setText(QStringLiteral("???"));
        CHECK(datei->text() == QStringLiteral("Type a name with letters or digits"));
        CHECK_FALSE(sichern->isEnabled());
        CHECK(fenster.speichere().isEmpty());
        name->setText(QStringLiteral("Table Knock"));
        CHECK(sichern->isEnabled());

        // --- Save and use: der Ausschnitt liegt als WAV bei den eigenen Samples, das Fenster meldet es
        fenster.setzeAusschnitt({24000, 28800});
        REQUIRE(fenster.spiele());
        CHECK(fenster.speichere() == QStringLiteral("user:table_knock"));
        CHECK_FALSE(fenster.spielt());  // das Anhoeren haelt
        CHECK(gespeichert == QStringList{QStringLiteral("user:table_knock")});
        const QString pfad = QDir(u.ordner).filePath(QStringLiteral("table_knock.wav"));
        REQUIRE(QFileInfo::exists(pfad));
        CHECK(QFileInfo(pfad).size() == 44 + 4 * 4800);  // 0,1 s Stereo in 16 Bit
        CHECK(eigeneSamples() == QStringList{QStringLiteral("user:table_knock")});

        // --- beim naechsten Mal: ein neuer Name; der alte waere ein Ersetzen, und die Zeile sagt es
        fenster.beginne();
        CHECK(fenster.frames() == 0);
        CHECK(name->text() == QStringLiteral("Recording"));
        name->setText(QStringLiteral("table knock"));
        CHECK(datei->text() == QStringLiteral("File: My files · samples · table_knock.wav — replaces the one there"));

        if (const QString ziel = qEnvironmentVariable("LUMIVIZ_RECORD_SAMPLE_BILD"); !ziel.isEmpty())
        {
            // wie in der Skizze: eine Aufnahme, die ausklingt, zugeschnitten, mit der blauen Linie
            AufnahmeEngine fuerBild;
            fuerBild.eingaenge = {eingang(0, QStringLiteral("Microphone (USB Audio)"), true)};
            AufnahmeFenster gezeigt;
            gezeigt.setEngine(&fuerBild);
            gezeigt.beginne();
            std::vector<float> klang(2 * 153600, 0.0f);  // 3,2 Sekunden
            for (std::size_t t = 22000; t < 153600; ++t)
            {
                const double seit = static_cast<double>(t - 22000);
                klang[2 * t] = klang[2 * t + 1] = static_cast<float>(0.7 * std::sin(2.0 * kPi * 180.0 * seit / 48000.0) *
                                                                     std::exp(-seit / 22000.0) * std::min(1.0, seit / 300.0));
            }
            gezeigt.setzeAufnahme(std::move(klang), 48000);
            gezeigt.findChild<QLineEdit*>(QStringLiteral("recordName"))->setText(QStringLiteral("Door slam"));
            gezeigt.setzeWiederholen(true);
            REQUIRE(gezeigt.spiele());
            std::vector<float> stumm(2 * 30000);
            fuerBild.generator->render(stumm.data(), 30000);
            gezeigt.takt();
            gezeigt.setAttribute(Qt::WA_DontShowOnScreen);
            gezeigt.show();  // das Layout steht erst nach dem Zeigen; auf den Bildschirm kommt nichts
            QCoreApplication::processEvents();
            CHECK(gezeigt.grab().save(ziel));
            gezeigt.halte();
            (void)gezeigt.speichere();  // sonst fragt das Schliessen nach
        }
    }
}
