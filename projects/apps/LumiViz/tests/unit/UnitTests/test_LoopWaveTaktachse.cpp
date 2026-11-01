/**
 * @file test_LoopWaveTaktachse.cpp
 * @brief Loop, Stufe 15e: Wave-Clips in der Taktachse, im Loop-Panel und in Publish
 *        (docs/audio/Loop_Konzept.md §4p; Skizze SK15-1 a, Entscheide WV2 a, WV3 a)
 *
 * - Das Modell: `neuerWaveClip` — Laenge aus der Datei bei Bezugstempo, aufgerundet; der Name
 *   nach der Datei; ohne Platz ein neuer Kanal.
 * - Die Taktachse wie im Programm: *New wave clip...* im Menue eines freien Takts, mit
 *   Dateiwahl und der Frage S2 a (gemerkt); die Welle am Clip, die Marken ~ und ♪.
 * - Das Loop-Panel oeffnet fuer ein Wave-Muster den Wave editor; eine Aenderung dort kommt im
 *   Loop an.
 * - Publish packt die Datei als Sample mit, dazu, was die Herkunft nennt (WV2 a, WV7 a).
 *
 * Ohne Ton und ohne gezeigtes Fenster; der Dekodierer liefert drei Sekunden gleichmaessigen Ton.
 * `LUMIVIZ_WAVE_ACHSE_BILD=<datei.png>` legt ein Bild der Taktachse ab.
 */

#include <doctest.h>

#include "PappTon.hpp"
#include "loop/Bearbeiten.hpp"
#include "loop/Loop.hpp"
#include "loop/LoopPanel.hpp"
#include "loop/Sample.hpp"
#include "loop/Taktachse.hpp"
#include "loop/WaveEditor.hpp"
#include "TonhoehenOrdner.hpp"
#include "paket/Import.hpp"
#include "paket/Packen.hpp"
#include "paket/Sammler.hpp"
#include "services/ServiceContainer.hpp"
#include "UI/widgets/Bedienhilfe.hpp"

#include <QAction>
#include <QApplication>
#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QSettings>
#include <QTemporaryDir>

#include <memory>
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
};

struct EigeneEinstellungen
{
    QString org = QCoreApplication::organizationName();
    QString name = QCoreApplication::applicationName();
    EigeneEinstellungen()
    {
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("LoopWaveTaktachse"));
        QSettings().clear();
    }
    ~EigeneEinstellungen()
    {
        QSettings().clear();
        QCoreApplication::setOrganizationName(org);
        QCoreApplication::setApplicationName(name);
    }
};

/// Drei Sekunden gleichmaessiger Ton, gleich in welcher Rate.
std::vector<float> dreiSekunden(const QString&, int rate, double)
{
    return std::vector<float>(static_cast<std::size_t>(2 * 3 * rate), 0.5f);
}

/// Eine Datei mit Inhalt (gelesen wird ueber den Dekodierer).
QString schreibe(const QString& ordner, const QString& name)
{
    REQUIRE(QDir().mkpath(ordner));
    const QString pfad = QDir(ordner).filePath(name);
    QFile f(pfad);
    REQUIRE(f.open(QIODevice::WriteOnly));
    f.write("RIFF");
    return pfad;
}

/// 120 BPM, 4/4 (ein Takt = 2 s); im Kanal K ein Drum-Clip D (4 Takte) ab Takt 8.
Loop mitDrums()
{
    Loop loop;
    Muster d;
    d.name = QStringLiteral("D");
    d.spuren = {{QStringLiteral("Kick"), QStringLiteral("kit:kick"), 1.0, 0.0, false}};
    loop.muster = {d};
    loop.kanaele = {{QStringLiteral("K"), true, {{QStringLiteral("D"), 8, 1}}}};
    loop.richte();
    return loop;
}

/// Die Engine aus Pappe, die dekodiert.
class WavePappEngine : public papp::PappMischEngine
{
public:
    [[nodiscard]] std::vector<float> decodeStereo(const QString& datei, int rate, double sekunden) override
    {
        return dreiSekunden(datei, rate, sekunden);
    }
};

}  // namespace

TEST_SUITE("LoopWaveTaktachse")
{
    TEST_CASE("Loop Wave 15e: neuerWaveClip im Modell")
    {
        Loop loop = mitDrums();
        // 5 s bei 2 s je Takt: 3 Takte, vor dem Drum-Clip ist Platz
        const std::optional<ClipOrt> eins = neuerWaveClip(loop, 0, 0, QStringLiteral("/a/voc.wav"), QStringLiteral("voc"), 5.0);
        REQUIRE(eins.has_value());
        CHECK(*eins == ClipOrt{0, 0});
        const Muster* voc = loop.findeMuster(QStringLiteral("voc"));
        REQUIRE(voc != nullptr);
        CHECK(voc->art == MusterArt::Wave);
        CHECK(voc->takte == 3);
        CHECK(voc->wave.datei == QStringLiteral("/a/voc.wav"));
        CHECK(voc->wave.bezugBpm == 120.0);
        // derselbe Titel noch einmal: ein neuer Name; ohne Laenge vier Takte
        const std::optional<ClipOrt> zwei = neuerWaveClip(loop, 0, 12, QStringLiteral("/a/voc.wav"), QStringLiteral("voc"), 0.0);
        REQUIRE(zwei.has_value());
        CHECK(loop.findeMuster(QStringLiteral("voc 1"))->takte == kTakteNeu);
        // kein Platz (3 Takte ab Takt 6 reichen in den Drum-Clip): ein neuer Kanal
        const std::optional<ClipOrt> drei = neuerWaveClip(loop, 0, 6, QStringLiteral("/a/voc.wav"), QStringLiteral("voc"), 5.0);
        REQUIRE(drei.has_value());
        CHECK(drei->kanal == 1);
        CHECK(loop.kanaele[1].name == QStringLiteral("voc 2"));
        CHECK(loop.kanaele[1].clips[0].startTakt == 6);
        // nichts zu tun
        CHECK_FALSE(neuerWaveClip(loop, 0, 0, QString(), QStringLiteral("x"), 1.0).has_value());
        CHECK_FALSE(neuerWaveClip(loop, 9, 0, QStringLiteral("/a/voc.wav"), QStringLiteral("x"), 1.0).has_value());
    }

    TEST_CASE("Loop Wave 15e: Taktachse — New wave clip..., Welle und Marken")
    {
        Anwendung anwendung;
        EigeneEinstellungen einstellungen;
        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());
        const QString datei = schreibe(QDir(ordner.path()).filePath(QStringLiteral("aufnahmen")), QStringLiteral("voc.wav"));
        setzeSampleKopieWahl(SampleKopie::AmOrt);  // S2 a gemerkt: keine Frage

        Taktachse achse;  // nicht gezeigt
        achse.resize(900, 260);
        achse.setLoop(mitDrums(), true);
        achse.setzeDekodierer(&dreiSekunden);
        Loop letzter;
        QStringList gemeldet;
        QObject::connect(&achse, &Taktachse::bearbeitet, [&](const Loop& neu, const QString& was) {
            letzter = neu;
            gemeldet << was;
        });
        QString titel;
        Taktachse::setzeDateiwahlFuerTests([&](bool, const QString& t, const QString&, const QString&) {
            titel = t;
            return datei;
        });

        // das Menue eines freien Takts: New wave clip... hinter New instrument clip
        QStringList texte;
        lumi::ui::setzeMenueBeobachterFuerTests([&](QMenu& menue) -> QAction* {
            QAction* wave = nullptr;
            for (QAction* a : menue.actions())
            {
                if (a->isSeparator()) continue;
                texte << a->text();
                if (a->text() == QStringLiteral("New wave clip...")) wave = a;
            }
            return wave;
        });
        const QPoint frei(achse.xVonTakt(0) + 4, achse.clipRechteck(0, 0).center().y());
        QContextMenuEvent rechts(QContextMenuEvent::Mouse, frei, achse.mapToGlobal(frei));
        QCoreApplication::sendEvent(&achse, &rechts);
        lumi::ui::setzeMenueBeobachterFuerTests({});
        const qsizetype instrument = texte.indexOf(QStringLiteral("New instrument clip"));
        REQUIRE(instrument >= 0);
        CHECK(texte.value(instrument + 1) == QStringLiteral("New wave clip..."));
        CHECK(titel == QStringLiteral("New wave clip"));
        CHECK(gemeldet == QStringList{QStringLiteral("New wave clip")});
        const Muster* voc = letzter.findeMuster(QStringLiteral("voc"));
        REQUIRE(voc != nullptr);
        CHECK(voc->art == MusterArt::Wave);
        CHECK(voc->takte == 2);  // 3 s bei 2 s je Takt
        CHECK(voc->wave.datei == datei);
        REQUIRE(achse.auswahl() != nullptr);
        CHECK(achse.auswahl()->muster == QStringLiteral("voc"));
        CHECK(achse.waveBild(datei).sekunden == doctest::Approx(3.0));

        // die Marken
        Muster m = *voc;
        CHECK(Taktachse::waveMarke(m).isEmpty());
        m.wave.folgtTempo = true;
        CHECK(Taktachse::waveMarke(m) == QStringLiteral("~"));
        m.wave.autotune.abschnitte = {{0.0, 1.0, 60, 0, 1.0, 0.0}};
        CHECK(Taktachse::waveMarke(m) == QStringLiteral("~♪"));

        SUBCASE("zu den eigenen Samples kopiert: user:<name>")
        {
            QTemporaryDir samples;
            REQUIRE(samples.isValid());
            setzeEigenenSampleOrdnerFuerTests(samples.path());
            setzeSampleKopieWahl(SampleKopie::Kopieren);
            achse.setLoop(letzter, true);
            REQUIRE(achse.neuerWaveClip(0, 12));
            setzeEigenenSampleOrdnerFuerTests(QString());
            CHECK(letzter.findeMuster(QStringLiteral("voc 1"))->wave.datei == QStringLiteral("user:voc"));
        }
        SUBCASE("abgebrochen: nichts")
        {
            Taktachse::setzeDateiwahlFuerTests([](bool, const QString&, const QString&, const QString&) { return QString(); });
            CHECK_FALSE(achse.neuerWaveClip(0, 12));
            CHECK(gemeldet.size() == 1);
        }
        Taktachse::setzeDateiwahlFuerTests({});

        const QString bild = qEnvironmentVariable("LUMIVIZ_WAVE_ACHSE_BILD");
        if (!bild.isEmpty())
        {
            Loop zeigen = letzter;
            Muster* w = musterIn(zeigen, QStringLiteral("voc"));
            w->wave.folgtTempo = true;
            w->wave.autotune.abschnitte = {{0.5, 1.0, 62, 0, 1.0, 20.0}};
            zeigen.kanaele[0].clips[0].wiederholungen = 3;
            zeigen.richte();
            achse.setLoop(zeigen, true);
            CHECK(achse.grab().save(bild));
        }
    }

    TEST_CASE("Loop Wave 15e: das Loop-Panel oeffnet den Wave editor")
    {
        TonhoehenOrdner tonhoehe;  // Stufe 15i: der Editor misst im Hintergrund
        Anwendung anwendung;
        EigeneEinstellungen einstellungen;
        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());
        const QString datei = schreibe(ordner.path(), QStringLiteral("voc.wav"));
        ServiceContainer dienste;
        dienste.registerSingleton<IAudioEngine>(
            [](ServiceContainer&) { return std::unique_ptr<IAudioEngine>(new WavePappEngine); });
        LoopPanel panel(dienste);  // nicht gezeigt
        panel.neu();
        REQUIRE(panel.taktachse()->setzeWaveClipEin(0, 0, datei));
        const Muster* voc = panel.loop().findeMuster(QStringLiteral("voc"));
        REQUIRE(voc != nullptr);
        CHECK(voc->takte == 2);  // die Taktachse liest die Laenge ueber die Engine

        CHECK(panel.waveEditor() == nullptr);
        REQUIRE(panel.oeffneEditor(QStringLiteral("voc"), false));
        WaveEditor* editor = panel.waveEditor();
        REQUIRE(editor != nullptr);
        CHECK(panel.editor() == nullptr);
        CHECK(panel.instrumentEditor() == nullptr);
        CHECK(editor->muster() == QStringLiteral("voc"));
        CHECK(editor->dateiInfoText().startsWith(QStringLiteral("3.00 s")));
        // eine Aenderung im Editor kommt im Loop an
        editor->findChild<QDoubleSpinBox*>(QStringLiteral("waveGain"))->setValue(-3.0);
        CHECK(panel.loop().findeMuster(QStringLiteral("voc"))->wave.gainDb == -3.0);
        // die Basis heisst jetzt anders: der Editor folgt ihr
        Loop umbenannt = panel.loop();
        REQUIRE(benenneBasisUm(umbenannt, QStringLiteral("voc"), QStringLiteral("Lead")));
        panel.setzeLoop(umbenannt);
        CHECK(editor->muster() == QStringLiteral("Lead"));
    }

    TEST_CASE("Loop Wave 15e: Publish packt die Datei als Sample mit, dazu die Herkunft")
    {
        Anwendung anwendung;
        EigeneEinstellungen einstellungen;
        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());
        const QString aufnahmen = QDir(ordner.path()).filePath(QStringLiteral("aufnahmen"));
        const QString voc = schreibe(aufnahmen, QStringLiteral("voc.wav"));
        const QString kick = schreibe(aufnahmen, QStringLiteral("kick_rec.wav"));

        // die Herkunft: ein Drum-Clip, der ein Sample auf der Platte nennt
        Loop clip;
        Muster d;
        d.name = QStringLiteral("D");
        d.spuren = {{QStringLiteral("Kick"), kick, 1.0, 0.0, false}};
        clip.muster = {d};
        clip.kanaele = {{QStringLiteral("K"), true, {{QStringLiteral("D"), 0, 1}}}};
        clip.richte();
        Loop loop;
        Muster w;
        w.name = QStringLiteral("Voc");
        w.art = MusterArt::Wave;
        w.wave.datei = voc;
        w.wave.herkunft = Herkunft(clip);
        loop.muster = {w};
        loop.kanaele = {{QStringLiteral("V"), true, {{QStringLiteral("Voc"), 0, 1}}}};
        loop.richte();
        const QString loopDatei = QDir(ordner.path()).filePath(QStringLiteral("loops/w.lvloop"));
        REQUIRE(QDir().mkpath(QFileInfo(loopDatei).absolutePath()));
        REQUIRE(speichereLoop(loop, loopDatei));

        const lumi::paket::Fund fund = lumi::paket::sammle(loopDatei);
        REQUIRE_MESSAGE(fund.ok, fund.fehler.toStdString());
        CHECK(fund.finde(lumi::paket::Art::Sample, QStringLiteral("voc")) != nullptr);
        CHECK(fund.finde(lumi::paket::Art::Sample, QStringLiteral("kick_rec")) != nullptr);

        const QString paket = QDir(ordner.path()).filePath(lumi::paket::paketName(fund));
        REQUIRE(lumi::paket::packe(fund, paket, QStringLiteral("1.2.3")).ok);
        const lumi::paket::PaketInhalt inhalt = lumi::paket::oeffnePaket(paket);
        REQUIRE_MESSAGE(inhalt.ok, inhalt.fehler.toStdString());
        const QJsonObject gepackt = QJsonDocument::fromJson(lumi::paket::paketDaten(paket, inhalt.hauptPfad)).object();
        const QJsonObject muster = gepackt.value(QStringLiteral("patterns")).toArray()[0].toObject();
        CHECK(muster.value(QStringLiteral("file")).toString() == QStringLiteral("user:voc"));
        const QJsonObject herkunft = muster.value(QStringLiteral("origin")).toObject().value(QStringLiteral("loop")).toObject();
        const QJsonObject spur =
            herkunft.value(QStringLiteral("patterns")).toArray()[0].toObject().value(QStringLiteral("tracks")).toArray()[0].toObject();
        CHECK(spur.value(QStringLiteral("sound")).toString() == QStringLiteral("user:kick_rec"));
    }
}
