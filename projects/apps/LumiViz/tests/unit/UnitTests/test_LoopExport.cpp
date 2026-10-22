/**
 ****************************************************************************************
 * @file   test_LoopExport.cpp
 * @brief  Loop: *Export audio…* — der Auftrag, das Fenster, der Weg ueber Loop-Panel
 *         und Composer (docs/audio/Spielweisen_Konzept.md §4.4, Stufe 30c; Entscheide
 *         WX2 a bis WX9 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton und ohne gezeigtes Fenster; die Klaenge der Dateien liest ein
 * Dekodierer ohne Engine (`looptest::liesWav`), die Dateien entstehen in
 * Wegwerf-Ordnern, die Einstellungen in `LumiVizTest` / `LoopExport`. Der
 * Tonspeicher wird zu Beginn und am Ende jedes Falls geleert (er gilt fuer das
 * ganze Programm). Gerechnet an `effects_demo` und `tabs_demo`.
 ****************************************************************************************
 */

#include <doctest.h>

#include "LoopOhneEngine.hpp"
#include "UI/widgets/Bedienhilfe.hpp"
#include "audio/Begrenzer.hpp"
#include "audio/WavDatei.hpp"
#include "composer/ComposerPanel.hpp"
#include "composer/Komposition.hpp"
#include "loop/ExportAuftrag.hpp"
#include "loop/ExportFenster.hpp"
#include "loop/Instrument.hpp"
#include "loop/LoopPanel.hpp"
#include "loop/Taktachse.hpp"
#include "loop/Tonspeicher.hpp"
#include "loop/Wellenform.hpp"
#include "services/ServiceContainer.hpp"

#include <QAction>
#include <QApplication>
#include <QContextMenuEvent>
#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QRadioButton>
#include <QSettings>
#include <QSpinBox>
#include <QTemporaryDir>

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

using namespace lumi::loop;

namespace
{

constexpr int kRate = 44100;

struct EigeneEinstellungen
{
    QString org = QCoreApplication::organizationName();
    QString name = QCoreApplication::applicationName();
    EigeneEinstellungen()
    {
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("LoopExport"));
        QSettings().clear();
    }
    ~EigeneEinstellungen()
    {
        QSettings().clear();
        QCoreApplication::setOrganizationName(org);
        QCoreApplication::setApplicationName(name);
    }
};

/// Leert den Tonspeicher zu Beginn und am Ende — Klaenge aus diesem Dekodierer sollen
/// keinem anderen Fall begegnen.
struct LeererSpeicher
{
    LeererSpeicher() { tonspeicher().leere(); }
    ~LeererSpeicher() { tonspeicher().leere(); }
};

/// Dateien ohne Engine lesen (die Kit-WAVs sind 44,1 kHz).
Dekodierer ohneEngine()
{
    return [](const QString& datei, int, double) { return looptest::liesWav(datei).daten; };
}

QString beispielPfad(const char* datei)
{
    return looptest::repoWurzel().filePath(QStringLiteral("asset/loop/") + QLatin1String(datei));
}

Loop beispiel(const char* datei)
{
    Loop loop;
    REQUIRE(ladeLoop(beispielPfad(datei), loop));
    loop.richte();
    return loop;
}

/// Die Klaenge zu Namen, ohne Engine — wie der Auftrag sie laedt.
std::shared_ptr<const Klaenge> klaengeZu(const QStringList& namen)
{
    Tonauftrag auftrag(namen, kRate, ohneEngine());
    auftrag.rechneHier();
    return std::make_shared<const Klaenge>(auftrag.klaenge());
}

ExportErgebnis fuehreAus(const ExportWunsch& wunsch)
{
    ExportAuftrag auftrag(wunsch, ohneEngine());
    auftrag.warte();
    REQUIRE(auftrag.beendet());
    REQUIRE(auftrag.ergebnis().has_value());
    return *auftrag.ergebnis();
}

int clipIndex(const Loop& loop, int kanal, const QString& muster)
{
    const auto& clips = loop.kanaele[static_cast<std::size_t>(kanal)].clips;
    for (std::size_t i = 0; i < clips.size(); ++i)
        if (clips[i].muster == muster) return static_cast<int>(i);
    return -1;
}

}  // namespace

TEST_SUITE("Loop Export")
{
    TEST_CASE("Loop Export 30c Auftrag: ein Clip — die Datei gleicht der Rechnung Bit fuer Bit")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QCoreApplication app{argc, argv};
        LeererSpeicher speicher;
        QTemporaryDir tmp;
        REQUIRE(tmp.isValid());
        const Loop loop = beispiel("effects_demo.lvloop");
        const ClipOrt echo{2, 4};  // Keys, Piano stabs mit Echo

        ExportWunsch wunsch;
        wunsch.art = ExportArt::Clip;
        wunsch.loop = loop;
        wunsch.clip = echo;
        wunsch.datei = QDir(tmp.path()).filePath(QStringLiteral("clip.wav"));
        wunsch.rate = kRate;
        const ExportErgebnis e = fuehreAus(wunsch);
        REQUIRE(e.fehler.isEmpty());
        CHECK(e.wav.geschrieben);
        CHECK(e.frames > e.clipFrames);  // mit Nachklang
        CHECK(e.bytes == 58 + e.frames * 8);
        CHECK(e.bericht.isEmpty());

        const SaitenVon saiten = saitenAusInstrumenten();
        const std::optional<Wellenform> welle =
            rechneClip(loop, echo, kRate, klaengeZu(clipFassung(loop, echo)->klangNamen(saiten)), saiten);
        REQUIRE(welle.has_value());
        const std::optional<lumi::audio::WavInhalt> gelesen = lumi::audio::liesWav(wunsch.datei);
        REQUIRE(gelesen.has_value());
        CHECK(gelesen->rate == kRate);
        CHECK(gelesen->stereo == welle->klang.daten);

        // ein zweiter Export laedt alles aus dem Tonspeicher
        wunsch.datei = QDir(tmp.path()).filePath(QStringLiteral("clip2.wav"));
        ExportAuftrag zweiter(wunsch, ohneEngine());
        zweiter.warte();
        CHECK(zweiter.stand().toeneErledigt == zweiter.stand().toeneGesamt);
        CHECK(lumi::audio::liesWav(wunsch.datei)->stereo == welle->klang.daten);
    }

    TEST_CASE("Loop Export 30c, WX9 a: der ganze Loop — nahtlos und mit Nachklang, Passes, Limiter")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QCoreApplication app{argc, argv};
        LeererSpeicher speicher;
        const Loop loop = beispiel("effects_demo.lvloop");
        REQUIRE(loop.limiter);
        const SaitenVon saiten = saitenAusInstrumenten();
        const std::int64_t durchlauf = plane(loop, kRate, saiten).durchlaufFrames;

        // --- nahtlos, zwei Durchlaeufe: genau so lang, unter der Decke, beide gleich
        const auto nahtlosKlaenge = klaengeZu(loopKlangNamen(loop, LoopEnde::Nahtlos, 2, saiten));
        const std::optional<Wellenform> nahtlos = rechneLoop(loop, LoopEnde::Nahtlos, 2, kRate, nahtlosKlaenge, saiten);
        REQUIRE(nahtlos.has_value());
        CHECK(nahtlos->klang.frames() == 2 * durchlauf);
        CHECK(nahtlos->clipFrames == 2 * durchlauf);
        CHECK_FALSE(nahtlos->nachklangGekappt);
        float spitze = 0.0f;
        for (const float w : nahtlos->klang.daten) spitze = std::max(spitze, std::abs(w));
        CHECK(spitze <= Begrenzer::kDecke + 1e-6f);
        double abstand = 0.0;
        for (std::int64_t i = 0; i < 2 * durchlauf; ++i)
            abstand = std::max(abstand, static_cast<double>(std::abs(nahtlos->klang.daten[static_cast<std::size_t>(i)] -
                                                                     nahtlos->klang.daten[static_cast<std::size_t>(i + 2 * durchlauf)])));
        MESSAGE("nahtlos: groesste Abweichung zwischen den beiden Durchlaeufen " << abstand << ", Spitze " << spitze);
        CHECK(abstand < 1e-3);
        // der erste Durchlauf eines Exports aus der Stille klingt am Anfang anders: ihm fehlt der Nachklang
        const std::optional<Wellenform> einmal = rechneLoop(loop, LoopEnde::Nahtlos, 1, kRate, nahtlosKlaenge, saiten);
        REQUIRE(einmal.has_value());
        CHECK(einmal->klang.frames() == durchlauf);

        // --- mit Nachklang: die Clips dreimal hintereinander, dahinter Platz
        const std::optional<Loop> fassung = loopFassung(loop, 3);
        REQUIRE(fassung.has_value());
        REQUIRE(fassung->kanaele.size() == loop.kanaele.size() + 1);
        for (std::size_t k = 0; k < loop.kanaele.size(); ++k)
            CHECK(fassung->kanaele[k].clips.size() == 3 * loop.kanaele[k].clips.size());
        CHECK_FALSE(fassung->kanaele.back().an);
        CHECK(fassung->durchlaufTakte() > 3 * loop.durchlaufTakte());
        const std::optional<Wellenform> mitNachklang =
            rechneLoop(loop, LoopEnde::Nachklang, 2, kRate, klaengeZu(loopKlangNamen(loop, LoopEnde::Nachklang, 2, saiten)), saiten);
        REQUIRE(mitNachklang.has_value());
        CHECK(mitNachklang->clipFrames == 2 * durchlauf);
        CHECK(mitNachklang->klang.frames() > mitNachklang->clipFrames);
        CHECK_FALSE(mitNachklang->nachklangGekappt);
        MESSAGE("mit Nachklang: " << (mitNachklang->klang.frames() - mitNachklang->clipFrames) * 1000 / kRate << " ms hinter zwei Durchlaeufen");

        // --- der Auftrag schreibt 16 Bit: genau die Durchlaeufe
        QTemporaryDir tmp;
        REQUIRE(tmp.isValid());
        ExportWunsch wunsch;
        wunsch.art = ExportArt::Loop;
        wunsch.loop = loop;
        wunsch.ende = LoopEnde::Nahtlos;
        wunsch.durchlaeufe = 2;
        wunsch.tiefe = lumi::audio::WavTiefe::Pcm16;
        wunsch.datei = QDir(tmp.path()).filePath(QStringLiteral("loop.wav"));
        wunsch.rate = kRate;
        const ExportErgebnis e = fuehreAus(wunsch);
        REQUIRE(e.fehler.isEmpty());
        CHECK(e.frames == 2 * durchlauf);
        CHECK(e.wav.gekappt == 0);  // der Limiter hielt alles unter der Decke
        CHECK(lumi::audio::liesWav(wunsch.datei)->tiefe == lumi::audio::WavTiefe::Pcm16);
    }

    TEST_CASE("Loop Export 30c, WX3 a: ein Loop-Clip des Composers — 24 Bit, Versatz und Laenge")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QCoreApplication app{argc, argv};
        LeererSpeicher speicher;
        QTemporaryDir tmp;
        REQUIRE(tmp.isValid());
        const Loop loop = beispiel("effects_demo.lvloop");
        ExportWunsch wunsch;
        wunsch.art = ExportArt::Ausschnitt;
        wunsch.loop = loop;
        wunsch.versatzMs = 3000.0;
        wunsch.laengeMs = 7500.0;
        wunsch.tiefe = lumi::audio::WavTiefe::Pcm24;
        wunsch.datei = QDir(tmp.path()).filePath(QStringLiteral("ausschnitt.wav"));
        wunsch.rate = kRate;
        const ExportErgebnis e = fuehreAus(wunsch);
        REQUIRE(e.fehler.isEmpty());
        CHECK(e.frames == std::llround(7500.0 * kRate / 1000.0));
        CHECK(e.frames == e.clipFrames);

        const SaitenVon saiten = saitenAusInstrumenten();
        const std::optional<Wellenform> welle =
            rechneAusschnitt(loop, 3000.0, 7500.0, kRate, klaengeZu(loop.klangNamen(saiten)), saiten);
        REQUIRE(welle.has_value());
        const std::vector<float> gelesen = lumi::audio::liesWav(wunsch.datei)->stereo;
        REQUIRE(gelesen.size() == welle->klang.daten.size());
        double abweichung = 0.0;
        for (std::size_t i = 0; i < gelesen.size(); ++i)
            abweichung = std::max(abweichung, static_cast<double>(std::abs(gelesen[i] - std::clamp(welle->klang.daten[i], -1.0f, 1.0f))));
        CHECK(abweichung <= 0.5 / 8388607.0 + 1e-7);
    }

    TEST_CASE("Loop Export 30c Auftrag: Abbruch schreibt nichts, ein falscher Ort meldet sich")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QCoreApplication app{argc, argv};
        LeererSpeicher speicher;
        QTemporaryDir tmp;
        REQUIRE(tmp.isValid());

        ExportWunsch wunsch;
        wunsch.art = ExportArt::Loop;
        wunsch.loop = beispiel("tabs_demo.lvloop");  // viele Saitentoene: das Laden dauert
        wunsch.ende = LoopEnde::Nachklang;
        wunsch.datei = QDir(tmp.path()).filePath(QStringLiteral("abgebrochen.wav"));
        wunsch.rate = kRate;
        {
            ExportAuftrag auftrag(wunsch, ohneEngine());
            auftrag.brichAb();
            auftrag.warte();
            CHECK(auftrag.stand().phase == ExportPhase::Abgebrochen);
            CHECK(auftrag.beendet());
        }
        CHECK_FALSE(QFileInfo::exists(wunsch.datei));

        wunsch.loop = beispiel("effects_demo.lvloop");
        wunsch.art = ExportArt::Clip;
        wunsch.clip = ClipOrt{2, 4};
        wunsch.datei = QDir(tmp.path()).filePath(QStringLiteral("gibt/es/nicht/x.wav"));
        ExportAuftrag falsch(wunsch, ohneEngine());
        falsch.warte();
        CHECK(falsch.stand().phase == ExportPhase::Fehler);
        CHECK(falsch.ergebnis()->fehler == QStringLiteral("The file could not be written."));

        wunsch.clip = ClipOrt{2, 5};  // dort liegt keiner
        ExportAuftrag ohneClip(wunsch, ohneEngine());
        ohneClip.warte();
        CHECK(ohneClip.stand().phase == ExportPhase::Fehler);
        CHECK(ohneClip.ergebnis()->fehler == QStringLiteral("The clip is not in the loop any more."));
    }

    TEST_CASE("Loop Export 30c: der Vorschlag fuer den Dateinamen")
    {
        const Loop loop = beispiel("effects_demo.lvloop");
        ExportWunsch wunsch;
        wunsch.loop = loop;
        wunsch.art = ExportArt::Clip;
        wunsch.clip = ClipOrt{2, 4};
        CHECK(exportName(QStringLiteral("effects_demo"), wunsch) == QStringLiteral("effects_demo_Piano stabs_bar5.wav"));
        wunsch.art = ExportArt::Loop;
        CHECK(exportName(QStringLiteral("effects_demo"), wunsch) == QStringLiteral("effects_demo.wav"));
        wunsch.art = ExportArt::Ausschnitt;
        wunsch.versatzMs = 72500.0;
        CHECK(exportName(QStringLiteral("effects_demo"), wunsch) == QStringLiteral("effects_demo_1-13.wav"));
        wunsch.art = ExportArt::Loop;
        CHECK(exportName(QStringLiteral("a/b:c?"), wunsch) == QStringLiteral("a_b_c_.wav"));
        CHECK(tiefeText(lumi::audio::WavTiefe::Pcm24) == QStringLiteral("24-bit"));
    }

    TEST_CASE("Loop Export 30c, WX6 a: das Fenster — Wahl, Export, Ergebnis, gemerkt, Abbruch")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;
        LeererSpeicher speicher;
        QTemporaryDir tmp;
        REQUIRE(tmp.isValid());
        QStringList gezeigt;
        ExportFenster::setzeOrdnerZeigerFuerTests([&gezeigt](const QString& datei) { gezeigt << datei; });

        ExportWunsch clip;
        clip.art = ExportArt::Clip;
        clip.loop = beispiel("effects_demo.lvloop");
        clip.clip = ClipOrt{2, 4};
        clip.rate = kRate;
        const QString ziel = QDir(tmp.path()).filePath(QStringLiteral("piano.wav"));
        {
            ExportFenster fenster(clip, QStringLiteral("effects_demo"), QStringLiteral("Keys · Piano stabs, bars 5–8"),
                                  ohneEngine());
            CHECK(fenster.windowTitle() == QStringLiteral("Export audio"));
            CHECK(fenster.findChild<QLabel*>(QStringLiteral("exportClip"))->text() == QStringLiteral("Keys · Piano stabs, bars 5–8"));
            CHECK(fenster.findChild<QLabel*>(QStringLiteral("exportContains"))->text() ==
                  QStringLiteral("the clip with its effects, expression and tail — without channel fader and loop volume"));
            CHECK(fenster.findChild<QSpinBox*>(QStringLiteral("exportPasses")) == nullptr);  // nur beim ganzen Loop
            auto* datei = fenster.findChild<QLineEdit*>(QStringLiteral("exportFile"));
            REQUIRE(datei != nullptr);
            CHECK(datei->text().endsWith(QStringLiteral("effects_demo_Piano stabs_bar5.wav")));
            CHECK(fenster.findChild<QRadioButton*>(QStringLiteral("exportFormatFloat"))->isChecked());

            datei->setText(QDir::toNativeSeparators(ziel));
            fenster.findChild<QRadioButton*>(QStringLiteral("exportFormat24"))->setChecked(true);
            REQUIRE(fenster.starte());
            CHECK_FALSE(fenster.starte());  // solange der Auftrag da ist, beginnt kein zweiter
            fenster.warteAufEnde();
            REQUIRE(fenster.ergebnis().has_value());
            CHECK(fenster.ergebnis()->fehler.isEmpty());
            const QString status = fenster.findChild<QLabel*>(QStringLiteral("exportStatus"))->text();
            MESSAGE("Ergebnis: " << status.toStdString());
            CHECK(status.startsWith(QStringLiteral("Exported ")));
            CHECK(status.contains(QStringLiteral("(tail ")));
            CHECK(status.contains(QStringLiteral("24-bit")));
            CHECK(QFileInfo::exists(ziel));
            auto* ordner = fenster.findChild<QPushButton*>(QStringLiteral("exportShowFolder"));
            REQUIRE(ordner != nullptr);
            CHECK_FALSE(ordner->isHidden());
            CHECK(fenster.findChild<QPushButton*>(QStringLiteral("exportStart"))->isHidden());
            ordner->click();
            CHECK(gezeigt == QStringList{QDir::toNativeSeparators(ziel)});
        }
        // --- gemerkt: Format und Ordner; die Datei gibt es schon
        CHECK(QSettings().value(QStringLiteral("loop/exportFormat")).toString() == QStringLiteral("pcm24"));
        {
            ExportFenster wieder(clip, QStringLiteral("effects_demo"), QString(), ohneEngine());
            CHECK(wieder.findChild<QRadioButton*>(QStringLiteral("exportFormat24"))->isChecked());
            auto* datei = wieder.findChild<QLineEdit*>(QStringLiteral("exportFile"));
            CHECK(QFileInfo(QDir::fromNativeSeparators(datei->text())).absolutePath() == QFileInfo(ziel).absolutePath());
            datei->setText(QDir::toNativeSeparators(ziel));
            CHECK(wieder.findChild<QLabel*>(QStringLiteral("exportStatus"))->text() ==
                  QStringLiteral("This file exists and will be replaced."));
        }

        // --- der ganze Loop: Ending und Passes, mit Nachklang dreimal
        ExportWunsch ganz = clip;
        ganz.art = ExportArt::Loop;
        {
            ExportFenster fenster(ganz, QStringLiteral("effects_demo"), QStringLiteral("effects_demo"), ohneEngine());
            CHECK(fenster.findChild<QLabel*>(QStringLiteral("exportContains"))->text() ==
                  QStringLiteral("the whole loop as you hear it here — with faders, loop volume and limiter"));
            auto* nahtlos = fenster.findChild<QRadioButton*>(QStringLiteral("exportSeamless"));
            auto* nachklang = fenster.findChild<QRadioButton*>(QStringLiteral("exportWithTail"));
            auto* passes = fenster.findChild<QSpinBox*>(QStringLiteral("exportPasses"));
            REQUIRE(nahtlos != nullptr);
            REQUIRE(nachklang != nullptr);
            REQUIRE(passes != nullptr);
            CHECK(nahtlos->isChecked());
            CHECK(passes->value() == 1);
            CHECK(passes->maximum() == kDurchlaeufeMax);
            nachklang->setChecked(true);
            passes->setValue(3);
            fenster.findChild<QLineEdit*>(QStringLiteral("exportFile"))
                ->setText(QDir::toNativeSeparators(QDir(tmp.path()).filePath(QStringLiteral("ganz"))));
            REQUIRE(fenster.starte());
            fenster.warteAufEnde();
            REQUIRE(fenster.ergebnis().has_value());
            CHECK(fenster.ergebnis()->fehler.isEmpty());
            CHECK(QFileInfo::exists(QDir(tmp.path()).filePath(QStringLiteral("ganz.wav"))));  // .wav kommt dazu
            CHECK(fenster.findChild<QLabel*>(QStringLiteral("exportStatus"))->text().contains(QStringLiteral("(tail ")));
        }
        CHECK(QSettings().value(QStringLiteral("loop/exportEnding")).toString() == QStringLiteral("tail"));
        CHECK(QSettings().value(QStringLiteral("loop/exportPasses")).toInt() == 3);

        // --- Cancel waehrend des Exports: das Fenster bleibt, geschrieben wird nichts
        tonspeicher().leere();
        ExportWunsch lang = ganz;
        lang.loop = beispiel("tabs_demo.lvloop");
        {
            ExportFenster fenster(lang, QStringLiteral("tabs_demo"), QStringLiteral("tabs_demo"), ohneEngine());
            const QString abgebrochen = QDir(tmp.path()).filePath(QStringLiteral("abgebrochen.wav"));
            fenster.findChild<QLineEdit*>(QStringLiteral("exportFile"))->setText(QDir::toNativeSeparators(abgebrochen));
            REQUIRE(fenster.starte());
            fenster.findChild<QPushButton*>(QStringLiteral("exportCancel"))->click();
            fenster.warteAufEnde();
            CHECK(fenster.findChild<QLabel*>(QStringLiteral("exportStatus"))->text() ==
                  QStringLiteral("Export cancelled — nothing was written."));
            CHECK_FALSE(QFileInfo::exists(abgebrochen));
            CHECK_FALSE(fenster.findChild<QPushButton*>(QStringLiteral("exportStart"))->isHidden());
        }
        ExportFenster::setzeOrdnerZeigerFuerTests({});
    }

    TEST_CASE("Loop Export 30c, WX6 a, WX8 a: ueber das Loop-Panel und den Composer")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;
        LeererSpeicher speicher;
        // jedes Fenster Export audio wird gefangen und nicht gezeigt
        QStringList clips;
        QStringList enthalten;
        lumi::ui::setzeDialogBeobachterFuerTests([&](QDialog& d) {
            if (auto* fenster = qobject_cast<ExportFenster*>(&d))
            {
                clips << fenster->findChild<QLabel*>(QStringLiteral("exportClip"))->text();
                enthalten << fenster->findChild<QLabel*>(QStringLiteral("exportContains"))->text();
            }
            return static_cast<int>(QDialog::Rejected);
        });

        // --- das Loop-Panel: der Knopf in der Datei-Zeile und der Eintrag am Clip
        ServiceContainer dienste;
        {
            LoopPanel panel(dienste);  // nicht gezeigt
            panel.resize(900, 300);
            auto* knopf = panel.findChild<QPushButton*>(QStringLiteral("loopExportAudio"));
            REQUIRE(knopf != nullptr);
            CHECK(knopf->text() == QStringLiteral("Export audio..."));
            CHECK_FALSE(panel.oeffneExport());  // noch kein Loop
            REQUIRE(panel.lade(beispielPfad("effects_demo.lvloop")));
            knopf->click();
            CHECK(clips == QStringList{QStringLiteral("effects_demo")});
            CHECK(enthalten.back().startsWith(QStringLiteral("the whole loop")));

            Taktachse& achse = *panel.taktachse();
            achse.resize(900, 220);
            const Loop& loop = achse.loop();
            const int piano = clipIndex(loop, 2, QStringLiteral("Piano stabs"));
            REQUIRE(piano >= 0);
            const LoopClip& c = loop.kanaele[2].clips[static_cast<std::size_t>(piano)];
            achse.waehle(2, piano);
            REQUIRE(achse.verlangeExport());
            CHECK(clips.back() == QStringLiteral("Keys · Piano stabs, bars %1–%2").arg(c.startTakt + 1).arg(c.startTakt + loop.clipTakte(c)));
            CHECK(enthalten.back().startsWith(QStringLiteral("the clip with its effects")));
        }

        // --- der Composer: der Eintrag am Loop-Clip, auch ohne Edit, und das Fenster dazu
        using namespace lumi::composer;
        Komposition k = neueKomposition();
        REQUIRE(fuegeKanalEin(k, {}, KanalArt::Audio) == 1);
        REQUIRE(fuegeSongEin(k, 1, beispielPfad("effects_demo.lvloop"), 10000, 20000) == 0);
        ComposerPanel panel(dienste);
        panel.zeitachse()->resize(900, 320);
        panel.zeitachse()->setKomposition(k, true);
        (void)panel.zeitachse()->grab();
        QStringList namen;
        lumi::ui::setzeMenueBeobachterFuerTests([&](QMenu& menue) -> QAction* {
            QAction* exportEintrag = nullptr;
            for (QAction* a : menue.actions())
            {
                namen << a->objectName();
                if (a->objectName() == QStringLiteral("composerExportLoopAudio")) exportEintrag = a;
            }
            return exportEintrag;
        });
        Zeitachse& achse = *panel.zeitachse();
        REQUIRE_FALSE(achse.bearbeiten());
        const QPoint ort(achse.xVon(20000), achse.yVonZeile(1) + 4);
        QContextMenuEvent ereignis(QContextMenuEvent::Mouse, ort, achse.mapToGlobal(ort));
        QCoreApplication::sendEvent(&achse, &ereignis);
        CHECK(namen.contains(QStringLiteral("composerExportLoopAudio")));
        REQUIRE(clips.size() == 3);
        CHECK(clips.back().startsWith(QStringLiteral("effects_demo in ")));
        CHECK(enthalten.back() ==
              QStringLiteral("the loop from 0:00 for 0:20, with its faders — without the composition's gain, fades and limiter"));
        CHECK_FALSE(panel.oeffneLoopExport(0, 0));  // kein Audio-Kanal
        lumi::ui::setzeMenueBeobachterFuerTests({});
        lumi::ui::setzeDialogBeobachterFuerTests({});
    }
}
