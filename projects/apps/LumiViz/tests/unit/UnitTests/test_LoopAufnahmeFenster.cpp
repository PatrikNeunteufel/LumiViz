/**
 * @file test_LoopAufnahmeFenster.cpp
 * @brief Loop, Stufe 11b, Schritt 2: das Fenster „Record sample" — aufnehmen,
 *        zuschneiden, in Dauerschleife anhoeren, speichern
 *        (docs/audio/Loop_Konzept.md §4h; Skizze gebilligt, Patrik S115);
 *        seit Stufe 17a auch „Edit sample": ein bestehendes Sample oeffnen,
 *        Save und Save as... (§4k; Entscheide M1 a bis M4 a, Patrik S119)
 *
 * Ohne Ton, ohne Eingang, ohne gezeigtes Fenster: die Engine ist aus Pappe —
 * sie „nimmt auf", was der Test ihr gibt, und reicht die Quelle heraus, die
 * den Ausschnitt spielt. Eigene Samples liegen in einem Wegwerf-Ordner.
 * `LUMIVIZ_RECORD_SAMPLE_BILD=<datei.png>` legt ein Bild des Fensters ab,
 * `LUMIVIZ_EDIT_SAMPLE_BILD=<datei.png>` zwei vom Bearbeiten (das zweite mit
 * `_kit`: ein Klang des Kits). Die Dialoge selbst werden nicht gezeigt.
 */

#include <doctest.h>

#include "PappAufnahme.hpp"
#include "loop/Aufnahme.hpp"
#include "loop/AufnahmeFenster.hpp"
#include "loop/Instrument.hpp"
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
        auto* sichernAls = fenster.findChild<QPushButton*>(QStringLiteral("recordSaveAs"));
        auto* sichern = fenster.findChild<QPushButton*>(QStringLiteral("recordSave"));
        auto* hinweis = fenster.findChild<QLabel*>(QStringLiteral("recordHint"));
        for (QObject* teil : std::initializer_list<QObject*>{wahl, record, stop, stand, pegel, bild, play, wiederholen, schnitt,
                                                              ohneStilleHaken, normal, sichernAls, sichern, hinweis})
            REQUIRE(teil != nullptr);
        CHECK(fenster.windowTitle() == QStringLiteral("Record sample"));
        // seit Stufe 17 (M2b a): kein Namensfeld mehr, unten Cancel, Save as..., Save
        CHECK(fenster.findChild<QLineEdit*>(QStringLiteral("recordName")) == nullptr);
        CHECK(sichernAls->text() == QStringLiteral("Save as..."));
        CHECK(sichern->text() == QStringLiteral("Save"));

        // --- ohne Engine: nichts geht, und die Zeile sagt es
        CHECK_FALSE(record->isEnabled());
        CHECK_FALSE(play->isEnabled());
        CHECK_FALSE(sichern->isEnabled());
        CHECK_FALSE(sichernAls->isEnabled());
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
        CHECK(fenster.vorschlagTitel() == QStringLiteral("Recording"));
        CHECK(fenster.herkunft() == AufnahmeFenster::Herkunft::Aufnahme);
        CHECK_FALSE(sichernAls->isEnabled());  // noch nichts aufgenommen
        CHECK_FALSE(sichern->isEnabled());
        CHECK_FALSE(fenster.geaendert());
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
        // eine frische Aufnahme hat noch keinen Namen: nur Save as...
        CHECK(sichernAls->isEnabled());
        CHECK_FALSE(sichern->isEnabled());
        CHECK(sichern->toolTip() == QStringLiteral("A new recording has no name yet — use Save as..."));
        CHECK(fenster.frisch());
        CHECK(fenster.geaendert());
        CHECK(fenster.speichere().isEmpty());
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

    TEST_CASE("Loop Record sample: Play und Repeat — der Ausschnitt folgt den Griffen; Save as legt das Sample ab")
    {
        Umgebung u;
        AufnahmeEngine engine;
        engine.eingaenge = {eingang(0, QStringLiteral("Default"), true)};
        const std::vector<float> aufnahme = probe();

        AufnahmeFenster fenster;
        fenster.setEngine(&engine);
        fenster.beginne();
        QStringList gespeichert;
        std::vector<double> vornWeg;
        QObject::connect(&fenster, &AufnahmeFenster::gespeichert, &fenster, [&](const QString& nennung, double ms) {
            gespeichert << nennung;
            vornWeg.push_back(ms);
        });
        auto* play = fenster.findChild<QPushButton*>(QStringLiteral("recordPlay"));
        auto* wiederholen = fenster.findChild<QCheckBox*>(QStringLiteral("recordRepeat"));
        auto* bild = fenster.findChild<KurvenBild*>(QStringLiteral("recordPicture"));

        CHECK_FALSE(fenster.spiele());  // nichts aufgenommen
        CHECK(fenster.speichere().isEmpty());
        CHECK(fenster.speichereAls(QStringLiteral("Leer")).isEmpty());
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

        // --- der Dialog von Save as...: der Name und die Datei, die daraus wird
        bool geht = false;
        CHECK(AufnahmeFenster::dateiZeile(QStringLiteral("Table Knock"), &geht) == QStringLiteral("File: My files · samples · table_knock.wav"));
        CHECK(geht);
        CHECK(AufnahmeFenster::dateiZeile(QStringLiteral("???"), &geht) == QStringLiteral("Type a name with letters or digits"));
        CHECK_FALSE(geht);
        CHECK(fenster.speichereAls(QStringLiteral("???")).isEmpty());

        // --- Save as...: der Ausschnitt liegt als WAV bei den eigenen Samples, das Fenster meldet es
        fenster.setzeAusschnitt({24000, 28800});
        REQUIRE(fenster.spiele());
        CHECK(fenster.speichereAls(QStringLiteral("Table Knock")) == QStringLiteral("user:table_knock"));
        CHECK_FALSE(fenster.spielt());  // das Anhoeren haelt
        CHECK(gespeichert == QStringList{QStringLiteral("user:table_knock")});
        CHECK(vornWeg == std::vector<double>{-1.0});  // eine Aufnahme: kein Einsatz mitzuziehen
        const QString pfad = QDir(u.ordner).filePath(QStringLiteral("table_knock.wav"));
        REQUIRE(QFileInfo::exists(pfad));
        CHECK(QFileInfo(pfad).size() == 44 + 4 * 4800);  // 0,1 s Stereo in 16 Bit
        CHECK(eigeneSamples() == QStringList{QStringLiteral("user:table_knock")});

        // --- beim naechsten Mal: ein neuer Vorschlag; den vergebenen Namen nimmt Save as... nicht (M2 b)
        fenster.beginne();
        CHECK(fenster.frames() == 0);
        CHECK(fenster.vorschlagTitel() == QStringLiteral("Recording"));
        CHECK(AufnahmeFenster::dateiZeile(QStringLiteral("table knock"), &geht) ==
              QStringLiteral("My samples already holds table_knock.wav — choose another name"));
        CHECK_FALSE(geht);
        fenster.setzeAufnahme(aufnahme, 48000);
        CHECK(fenster.speichereAls(QStringLiteral("table knock")).isEmpty());
        CHECK(QFileInfo(pfad).size() == 44 + 4 * 4800);  // unberuehrt
        CHECK(gespeichert.size() == 1);
        fenster.beginne();

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
            (void)gezeigt.speichereAls(QStringLiteral("Door slam"));  // sonst fragt das Schliessen nach
        }
    }

    TEST_CASE("Loop Edit sample: ein bestehendes Sample oeffnen — Save ersetzt das eigene, Kit und Datei sind schreibgeschuetzt")
    {
        Umgebung u;
        QTemporaryDir instrumente;
        REQUIRE(instrumente.isValid());
        setzeEigenenInstrumentOrdnerFuerTests(instrumente.path());
        AufnahmeEngine engine;
        engine.eingaenge = {eingang(0, QStringLiteral("Default"), true)};
        const std::vector<float> klang = probe();

        AufnahmeFenster fenster;
        fenster.setEngine(&engine);
        QStringList gespeichert;
        std::vector<double> vornWeg;
        QObject::connect(&fenster, &AufnahmeFenster::gespeichert, &fenster, [&](const QString& nennung, double ms) {
            gespeichert << nennung;
            vornWeg.push_back(ms);
        });
        auto* stand = fenster.findChild<QLabel*>(QStringLiteral("recordState"));
        auto* bild = fenster.findChild<KurvenBild*>(QStringLiteral("recordPicture"));
        auto* ohneStilleHaken = fenster.findChild<QCheckBox*>(QStringLiteral("recordTrimSilence"));
        auto* normal = fenster.findChild<QCheckBox*>(QStringLiteral("recordNormalize"));
        auto* record = fenster.findChild<QPushButton*>(QStringLiteral("recordStart"));
        auto* sichernAls = fenster.findChild<QPushButton*>(QStringLiteral("recordSaveAs"));
        auto* sichern = fenster.findChild<QPushButton*>(QStringLiteral("recordSave"));

        // ein eigenes Sample, wie Save as... es ablegt
        REQUIRE(speichereAufnahme(klang, 48000, QStringLiteral("Table Knock")) == QStringLiteral("user:table_knock"));
        const QString pfad = QDir(u.ordner).filePath(QStringLiteral("table_knock.wav"));
        REQUIRE(QFileInfo(pfad).size() == 44 + 4 * 96000);

        SUBCASE("ein eigenes Sample: das Ganze im Bild, Save ersetzt es, der Schnitt vorn geht mit hinaus")
        {
            normal->setChecked(true);  // von einer Aufnahme davor stehen geblieben
            fenster.bearbeite(QStringLiteral("user:table_knock"), klang, 48000);
            CHECK(fenster.windowTitle() == QStringLiteral("Edit sample – table_knock"));
            CHECK(fenster.nennung() == QStringLiteral("user:table_knock"));
            CHECK(fenster.herkunft() == AufnahmeFenster::Herkunft::Eigenes);
            CHECK_FALSE(fenster.frisch());
            CHECK(stand->text() == QStringLiteral("2.00 s · My samples"));
            // oeffnen aendert nichts: kein Schnitt, kein Anheben, nichts zu verlieren
            CHECK_FALSE(ohneStilleHaken->isChecked());
            CHECK_FALSE(normal->isChecked());
            CHECK(fenster.ausschnitt() == Ausschnitt{0, 96000});
            CHECK(bild->griff(KurvenBild::GriffVon) == doctest::Approx(0.0));
            CHECK(bild->griff(KurvenBild::GriffBis) == doctest::Approx(1.0));
            CHECK(bild->werte().size() == 600);
            CHECK_FALSE(fenster.geaendert());
            CHECK(fenster.ergebnis() == klang);  // an keinem Ende geblendet
            CHECK(record->isEnabled());           // Record bleibt (M4 a)
            CHECK(sichernAls->isEnabled());
            CHECK(sichern->isEnabled());
            CHECK(sichern->toolTip() == QStringLiteral("Replace table_knock in My samples"));
            CHECK(fenster.vorschlagTitel() == QStringLiteral("table_knock 2"));
            CHECK(fenster.ersetzenFrage() == QStringLiteral("Replace table_knock.wav?\nWhat you cut away is gone."));

            // vorn und hinten schneiden: geblendet wird nur dort, wo geschnitten ist
            fenster.setzeAusschnitt({24000, 96000});
            CHECK(fenster.geaendert());
            CHECK(fenster.blenden().vorn);
            CHECK_FALSE(fenster.blenden().hinten);
            CHECK(fenster.ergebnis() == schneideAus(klang, 48000, {24000, 96000}, false, {true, false}));
            fenster.setzeAusschnitt({24000, 72000});
            CHECK(fenster.ergebnis() == schneideAus(klang, 48000, {24000, 72000}, false));

            // Save: dieselbe Nennung, die Datei ist kuerzer, 500 ms sind vorn weg
            CHECK(fenster.speichere() == QStringLiteral("user:table_knock"));
            CHECK(gespeichert == QStringList{QStringLiteral("user:table_knock")});
            REQUIRE(vornWeg.size() == 1);
            CHECK(vornWeg[0] == doctest::Approx(500.0));
            CHECK(QFileInfo(pfad).size() == 44 + 4 * 48000);
            CHECK(eigeneSamples() == QStringList{QStringLiteral("user:table_knock")});
        }

        SUBCASE("Save as... legt eine Kopie daneben, das Original bleibt")
        {
            fenster.bearbeite(QStringLiteral("user:table_knock"), klang, 48000);
            fenster.setzeAusschnitt({12000, 60000});
            CHECK(fenster.speichereAls(QStringLiteral("table knock")).isEmpty());  // vergeben
            CHECK(fenster.speichereAls(fenster.vorschlagTitel()) == QStringLiteral("user:table_knock_2"));
            CHECK(gespeichert == QStringList{QStringLiteral("user:table_knock_2")});
            CHECK(vornWeg[0] == doctest::Approx(250.0));
            CHECK(QFileInfo(pfad).size() == 44 + 4 * 96000);
            CHECK(QFileInfo(QDir(u.ordner).filePath(QStringLiteral("table_knock_2.wav"))).size() == 44 + 4 * 48000);
            // der naechste Vorschlag zaehlt weiter
            fenster.bearbeite(QStringLiteral("user:table_knock"), klang, 48000);
            CHECK(fenster.vorschlagTitel() == QStringLiteral("table_knock 3"));
        }

        SUBCASE("ein Klang des Kits und eine Datei an freiem Ort: Save ist grau")
        {
            fenster.bearbeite(QStringLiteral("kit:kick"), klang, 48000);
            CHECK(fenster.windowTitle() == QStringLiteral("Edit sample – kick"));
            CHECK(fenster.herkunft() == AufnahmeFenster::Herkunft::Mitgeliefert);
            CHECK(stand->text() == QStringLiteral("2.00 s · Built-in, read-only"));
            CHECK_FALSE(sichern->isEnabled());
            CHECK(sichern->toolTip() == QStringLiteral("Built-in sounds and files stay as they are — use Save as..."));
            CHECK(sichernAls->isEnabled());
            CHECK(fenster.speichere().isEmpty());
            CHECK(fenster.vorschlagTitel() == QStringLiteral("kick 2"));
            fenster.setzeAusschnitt({0, 48000});
            CHECK(fenster.speichereAls(QStringLiteral("kick 2")) == QStringLiteral("user:kick_2"));
            CHECK(vornWeg == std::vector<double>{0.0});

            const QString frei = QDir(u.tmp.path()).filePath(QStringLiteral("cello.mp3"));
            fenster.bearbeite(frei, klang, 48000);
            CHECK(fenster.windowTitle() == QStringLiteral("Edit sample – cello.mp3"));
            CHECK(fenster.herkunft() == AufnahmeFenster::Herkunft::Datei);
            CHECK(stand->text() == QStringLiteral("2.00 s · File, read-only"));
            CHECK_FALSE(sichern->isEnabled());
            CHECK(fenster.vorschlagTitel() == QStringLiteral("cello 2"));

            // ueber einer Datei aufgenommen: mit ihr hat das Bild nichts mehr zu tun
            fenster.setzeAufnahme(probe(0.2f), 48000);
            CHECK(fenster.nennung().isEmpty());
            CHECK(fenster.windowTitle() == QStringLiteral("Record sample"));
            CHECK(ohneStilleHaken->isChecked());
            CHECK(fenster.ausschnitt() == ohneStille(probe(0.2f), 48000));
            CHECK(fenster.vorschlagTitel() == QStringLiteral("Recording"));
            CHECK_FALSE(sichern->isEnabled());
        }

        SUBCASE("ueber einem eigenen Sample neu aufgenommen: Save schreibt unter demselben Namen")
        {
            fenster.bearbeite(QStringLiteral("user:table_knock"), klang, 48000);
            engine.aufnahme = probe(0.2f);
            REQUIRE(fenster.starteAufnahme());
            engine.angekommen = 96000;
            REQUIRE(fenster.stoppeAufnahme());
            CHECK(fenster.nennung() == QStringLiteral("user:table_knock"));
            CHECK(fenster.windowTitle() == QStringLiteral("Edit sample – table_knock"));
            CHECK(fenster.frisch());
            CHECK(stand->text() == QStringLiteral("2.0 s recorded"));
            CHECK(ohneStilleHaken->isChecked());
            CHECK(sichern->isEnabled());
            CHECK(fenster.ersetzenFrage() == QStringLiteral("Replace table_knock.wav with the new recording?"));
            CHECK(fenster.speichere() == QStringLiteral("user:table_knock"));
            CHECK(vornWeg == std::vector<double>{-1.0});  // ein anderer Klang
            // danach von vorn: Record sample, ohne Sample
            fenster.beginne();
            CHECK(fenster.nennung().isEmpty());
            CHECK(fenster.windowTitle() == QStringLiteral("Record sample"));
            CHECK(bild->werte().empty());
        }

        SUBCASE("die Rueckfrage nennt die eigenen Instrumente, die das Sample spielen")
        {
            const auto lege = [](const QString& titel, const QString& sample) {
                Instrument instrument;
                instrument.titel = titel;
                instrument.schichten[0].schwingung = Schwingung::Sample;
                instrument.schichten[0].sample = sample;
                REQUIRE(speichereEigenes(instrument, eigenerName(titel)));
            };
            lege(QStringLiteral("Table Kit"), QStringLiteral("user:table_knock"));
            lege(QStringLiteral("Other"), QStringLiteral("user:something_else"));
            CHECK(eigeneInstrumenteMitSample(QStringLiteral("user:table_knock")) == QStringList{QStringLiteral("Table Kit")});
            CHECK(eigeneInstrumenteMitSample(QString()).isEmpty());
            fenster.bearbeite(QStringLiteral("user:table_knock"), klang, 48000);
            CHECK(fenster.ersetzenFrage() ==
                  QStringLiteral("Replace table_knock.wav?\nWhat you cut away is gone.\nUsed by: Table Kit — they will play the new version."));
            lege(QStringLiteral("Knock Bass"), QStringLiteral("user:table_knock"));
            lege(QStringLiteral("Alpha"), QStringLiteral("user:table_knock"));
            lege(QStringLiteral("Zed"), QStringLiteral("user:table_knock"));
            lege(QStringLiteral("Yak"), QStringLiteral("user:table_knock"));
            CHECK(fenster.ersetzenFrage().endsWith(
                QStringLiteral("Used by: Alpha, Knock Bass, Table Kit and 2 more — they will play the new version.")));
        }

        if (const QString ziel = qEnvironmentVariable("LUMIVIZ_EDIT_SAMPLE_BILD"); !ziel.isEmpty())
        {
            // zwei Bilder: ein eigenes Sample, zugeschnitten — und (mit _kit) ein Klang des Kits
            for (const bool kit : {false, true})
            {
                AufnahmeFenster gezeigt;
                gezeigt.setEngine(&engine);
                std::vector<float> schlag(2 * 40320, 0.0f);  // 0,84 Sekunden
                for (std::size_t t = 3800; t < 40320; ++t)
                {
                    const double seit = static_cast<double>(t - 3800);
                    schlag[2 * t] = schlag[2 * t + 1] =
                        static_cast<float>(0.7 * std::sin(2.0 * kPi * 140.0 * seit / 48000.0) * std::exp(-seit / 6000.0));
                }
                gezeigt.bearbeite(kit ? QStringLiteral("kit:kick") : QStringLiteral("user:table_knock"), std::move(schlag), 48000);
                gezeigt.setzeAusschnitt({kit ? 0 : 3800, 31700});
                gezeigt.setAttribute(Qt::WA_DontShowOnScreen);
                gezeigt.show();
                QCoreApplication::processEvents();
                QString datei = ziel;
                if (kit) datei.replace(QStringLiteral(".png"), QStringLiteral("_kit.png"));
                CHECK(gezeigt.grab().save(datei));
                gezeigt.setzeAusschnitt({0, 40320});  // sonst fragt das Schliessen nach
            }
        }
        fenster.setEngine(nullptr);
        setzeEigenenInstrumentOrdnerFuerTests(QString());
    }
}
