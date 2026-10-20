/**
 ****************************************************************************************
 * @file   test_LoopPegel.cpp
 * @brief  Loop, Stufe 27b: die Standardprobe und die Messung der Instrumente und
 *         Drum-Klaenge nach EBU R128 (docs/audio/Loop_Konzept.md §4n, LL2 a, LL4 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Fenster, ohne Audio-Engine; die Instrumente aus dem Repo (die
 * Test-Exe aus dem Projekt-Root starten).
 *
 * - Die Standardprobe: Aufbau, Grundton je Art, Laenge.
 * - Die Messung folgt der Lautstaerke des Instruments (halbe Lautstaerke = 6,02 dB
 *   leiser), auch an einem Saiteninstrument.
 * - Messung (meldet nur Zahlen, prueft nichts ausser, dass gemessen wurde): jedes
 *   mitgelieferte Instrument mit der Standardprobe (Short-term-Maximum), die Drums
 *   mit einem Schlag (Momentary-Maximum), dazu je mitgeliefertem Kit jede Rolle mit
 *   Volume und ihr Abstand zur Kick. `LUMIVIZ_PEGEL_DATEI=<datei.tsv>` legt die Werte
 *   fuer die Skripte von 27c und 27d ab.
 ****************************************************************************************
 */

#include <doctest.h>

#include "LoopOhneEngine.hpp"
#include "audio/Lautheit.hpp"
#include "loop/DrumKit.hpp"
#include "loop/Instrument.hpp"
#include "loop/InstrumentCreator.hpp"
#include "loop/InstrumentPalette.hpp"
#include "loop/Pegel.hpp"
#include "UI/widgets/Bedienhilfe.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QDialog>
#include <QDir>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTemporaryDir>
#include <QTreeWidget>
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

#include <algorithm>
#include <cmath>
#include <map>
#include <vector>

using namespace lumi::loop;

namespace
{

constexpr int kRate = 44100;

struct PegelApp
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QCoreApplication app{argc, argv};
};

double median(std::vector<double> werte)
{
    if (werte.empty()) return 0.0;
    std::sort(werte.begin(), werte.end());
    const std::size_t mitte = werte.size() / 2;
    return werte.size() % 2 == 1 ? werte[mitte] : (werte[mitte - 1] + werte[mitte]) / 2.0;
}

/// Ein Klang des Kits als Schlag: das Instrument auf seiner Note und Laenge,
/// sonst die WAV-Datei; mit dem Volume der Rolle. `kLufsStille`, wenn er fehlt.
double schlagDesKits(const KitKlang& k)
{
    Klang klang;
    if (!k.instrument.isEmpty())
    {
        Instrument instrument;
        if (!holeInstrument(k.instrument, instrument)) return lumi::audio::kLufsStille;
        looptest::merkeSamplesOhneEngine(instrument);
        klang = rendereTon(instrument, k.hoehe, k.dauerMs, kRate);
    }
    else
    {
        const QString wav = looptest::mitgelieferteWav(k.klang);
        if (wav.isEmpty()) return lumi::audio::kLufsStille;
        klang = looptest::liesWav(wav);
    }
    const lumi::audio::Lautheit l = messeSchlag(klang, kRate);
    if (l.momentanMaxLufs <= lumi::audio::kLufsStille || k.laut <= 0.0) return lumi::audio::kLufsStille;
    return l.momentanMaxLufs + 20.0 * std::log10(k.laut);
}

}  // namespace

TEST_SUITE("LoopPegel")
{
    TEST_CASE("Loop Pegel: die Standardprobe — Tonleiter und Akkord, Grundton je Art, vier Takte")
    {
        const Loop probe = standardProbe(QStringLiteral("piano"), QStringLiteral("keys"));
        REQUIRE(probe.muster.size() == 1);
        const Muster& m = probe.muster.front();
        CHECK(m.instrument == QStringLiteral("piano"));
        CHECK(m.takte == 4);
        CHECK(m.noten.size() == 19);
        CHECK(std::min_element(m.noten.begin(), m.noten.end(), [](const Note& a, const Note& b) {
                  return a.hoehe < b.hoehe;
              })->hoehe == 60);
        CHECK(probe.laut == 1.0);
        CHECK(probe.durchlaufMs() == doctest::Approx(8000.0));
        // der Akkord: vier Noten am selben Schritt, einen Takt lang
        int imAkkord = 0;
        for (const Note& n : m.noten)
            if (n.laenge == 16) ++imAkkord;
        CHECK(imAkkord == 4);

        const Loop bass = standardProbe(QStringLiteral("bass"), QStringLiteral("bass"));
        CHECK(std::min_element(bass.muster.front().noten.begin(), bass.muster.front().noten.end(),
                               [](const Note& a, const Note& b) { return a.hoehe < b.hoehe; })
                  ->hoehe == 36);
    }

    TEST_CASE("Loop Pegel: die Messung folgt der Lautstaerke des Instruments, auch mit Saiten")
    {
        const PegelApp app;
        for (const char* name : {"organ", "clean_guitar"})
        {
            CAPTURE(name);
            Instrument instrument;
            REQUIRE(holeInstrument(QString::fromLatin1(name), instrument));
            instrument.laut = 1.0;
            const lumi::audio::Lautheit voll = messeInstrument(instrument, kRate);
            instrument.laut = 0.5;
            const lumi::audio::Lautheit halb = messeInstrument(instrument, kRate);
            REQUIRE(voll.kurzzeitMaxLufs > -70.0);
            MESSAGE(name << ": " << voll.kurzzeitMaxLufs << " LUFS bei Volume 1, " << halb.kurzzeitMaxLufs << " bei 0,5");
            CHECK(std::abs(voll.kurzzeitMaxLufs - halb.kurzzeitMaxLufs - 20.0 * std::log10(2.0)) <= 0.05);
        }
        // eine Probe ist acht Sekunden, Stereo
        Instrument organ;
        REQUIRE(holeInstrument(QStringLiteral("organ"), organ));
        CHECK(rechneStandardprobe(organ, kRate).size() == static_cast<std::size_t>(8 * kRate * 2));
    }

    // Stufe 27c (LL2 a): die mitgelieferten Instrumente stehen auf dem Bezug. Wer eines im Klang
    // aendert, misst neu und laesst asset/loop/instrument_pegel.py --messung laufen.
    TEST_CASE("Loop Pegel 27c: jedes mitgelieferte Instrument ausser den Drums liegt auf dem Bezug (±1 LU)")
    {
        const PegelApp app;
        // derselbe Bezug im Programm und im Skript
        QFile skript(looptest::repoWurzel().filePath(QStringLiteral("asset/loop/instrument_pegel.py")));
        REQUIRE(skript.open(QIODevice::ReadOnly));
        const QRegularExpressionMatch treffer =
            QRegularExpression(QStringLiteral("\\nBEZUG_LUFS = (-?[0-9.]+)\\n")).match(QString::fromUtf8(skript.readAll()));
        REQUIRE(treffer.hasMatch());
        CHECK(treffer.captured(1).toDouble() == kInstrumentBezugLufs);

        int geprueft = 0;
        int ueberEins = 0;
        for (const Instrument& instrument : instrumente())
        {
            if (instrument.art == QLatin1String("drums")) continue;
            CAPTURE(instrument.name.toStdString());
            looptest::merkeSamplesOhneEngine(instrument);
            const double wert = messeInstrument(instrument, kRate).kurzzeitMaxLufs;
            CHECK(std::abs(wert - kInstrumentBezugLufs) <= 1.0);
            ++geprueft;
            if (instrument.laut > 1.0) ++ueberEins;
        }
        MESSAGE(geprueft << " Instrumente auf dem Bezug, davon " << ueberEins << " mit Volume ueber 1");
        CHECK(geprueft > 80);
    }

    TEST_CASE("Loop Pegel: Messung aller mitgelieferten Instrumente und Kits (meldet nur Zahlen)")
    {
        const PegelApp app;
        struct Zeile
        {
            QString name;
            QString art;
            double wert;  // Short-term-Maximum, bei Drums Momentary-Maximum
            double laut;
        };
        std::vector<Zeile> zeilen;
        for (const Instrument& instrument : instrumente())
        {
            looptest::merkeSamplesOhneEngine(instrument);
            const bool drum = instrument.art == QLatin1String("drums");
            const lumi::audio::Lautheit l = drum ? messeDrumInstrument(instrument, kRate) : messeInstrument(instrument, kRate);
            zeilen.push_back({instrument.name, instrument.art, drum ? l.momentanMaxLufs : l.kurzzeitMaxLufs, instrument.laut});
        }
        std::sort(zeilen.begin(), zeilen.end(), [](const Zeile& a, const Zeile& b) { return a.wert < b.wert; });
        std::vector<double> ohneDrums;
        for (const Zeile& z : zeilen)
        {
            MESSAGE(z.name.toStdString() << " (" << z.art.toStdString() << ", Volume " << z.laut << "): " << z.wert
                                         << (z.art == QLatin1String("drums") ? " LUFS M max" : " LUFS S max"));
            if (z.art != QLatin1String("drums")) ohneDrums.push_back(z.wert);
        }
        REQUIRE_FALSE(ohneDrums.empty());
        std::vector<double> sortiert = ohneDrums;
        std::sort(sortiert.begin(), sortiert.end());
        MESSAGE("ohne Drums: " << ohneDrums.size() << " Instrumente, " << sortiert.front() << " bis " << sortiert.back()
                               << " LUFS, Median " << median(ohneDrums));
        CHECK(zeilen.size() > 100);

        // die Kits: je Rolle der Schlag mit Volume und der Abstand zur Kick
        struct KitZeile
        {
            QString kit;
            QString rolle;
            QString klang;
            double wert;
        };
        std::vector<KitZeile> kitZeilen;
        std::map<QString, std::vector<double>> abstandJeArt;
        for (const DrumKit& kit : mitgelieferteKits())
        {
            double kick = lumi::audio::kLufsStille;
            for (const KitKlang& k : kit.klaenge)
                if (k.rolle.startsWith(QLatin1String("kick")))
                {
                    kick = schlagDesKits(k);
                    break;
                }
            for (const KitKlang& k : kit.klaenge)
            {
                const double wert = schlagDesKits(k);
                kitZeilen.push_back({kit.name, k.rolle, k.instrument.isEmpty() ? k.klang : k.instrument, wert});
                const QString art = QString(k.rolle).remove(QRegularExpression(QStringLiteral(" \\d+$")));
                MESSAGE("Kit " << kit.name.toStdString() << ", " << k.rolle.toStdString() << " ("
                               << (k.instrument.isEmpty() ? k.klang : k.instrument).toStdString() << ", Volume " << k.laut
                               << "): " << wert << " LUFS M max, Abstand zur Kick " << (wert - kick));
                if (wert > lumi::audio::kLufsStille && kick > lumi::audio::kLufsStille) abstandJeArt[art].push_back(wert - kick);
            }
        }
        for (const auto& [art, abstaende] : abstandJeArt)
            MESSAGE("Abstand zur Kick, " << art.toStdString() << ": Median " << median(abstaende) << " LU ueber "
                                         << abstaende.size() << " Kits");
        CHECK_FALSE(kitZeilen.empty());

        if (const QString datei = qEnvironmentVariable("LUMIVIZ_PEGEL_DATEI"); !datei.isEmpty())
        {
            QFile f(datei);
            REQUIRE(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
            QTextStream aus(&f);
            aus << "# Loop Pegel (test_LoopPegel.cpp, Stufe 27b): Art <Tab> Name <Tab> Wert LUFS <Tab> Volume bzw. Rolle\n";
            for (const Zeile& z : zeilen)
                aus << "instrument\t" << z.name << '\t' << QString::number(z.wert, 'f', 3) << '\t' << z.laut << '\t' << z.art << '\n';
            for (const KitZeile& k : kitZeilen)
                aus << "kit\t" << k.kit << '\t' << QString::number(k.wert, 'f', 3) << '\t' << k.rolle << '\t' << k.klang << '\n';
            MESSAGE("abgelegt: " << datei.toStdString());
        }
    }
    // Stufe 27f (S139, Entscheid Patrik LL14 a): Match level im Creator und im Fenster Instruments —
    // eigene Instrumente in einem Wegwerf-Ordner, die Rueckfrage ueber `setzeDialogBeobachterFuerTests`
    TEST_CASE("Loop Pegel 27f: Match im Creator, die Rueckfrage beim Speichern, die Marke im Fenster Instruments")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        QTemporaryDir tmp;
        REQUIRE(tmp.isValid());
        setzeEigenenInstrumentOrdnerFuerTests(QDir(tmp.path()).filePath(QStringLiteral("instruments")));
        struct Zurueck
        {
            ~Zurueck()
            {
                setzeEigenenInstrumentOrdnerFuerTests(QString());
                lumi::ui::setzeDialogBeobachterFuerTests({});
            }
        } zurueck;

        // --- die Abweichung: ein Instrument doppelt so laut liegt 6,02 dB ueber dem Bezug
        Instrument orgel = *findeInstrument(QStringLiteral("organ"));
        REQUIRE(abstandZumBezug(orgel, kRate).has_value());
        CHECK(std::abs(*abstandZumBezug(orgel, kRate)) <= 0.05);  // mitgeliefert: auf dem Bezug
        Instrument laut = orgel;
        laut.laut = orgel.laut * 2.0;
        CHECK(*abstandZumBezug(laut, kRate) == doctest::Approx(20.0 * std::log10(2.0)).epsilon(0.01));
        CHECK(*lautAufBezug(laut, kRate) == doctest::Approx(orgel.laut).epsilon(0.001));
        CHECK_FALSE(abstandZumBezug(*findeInstrument(QStringLiteral("drum_kick")), kRate).has_value());

        // --- im Creator: die Anzeige, Match
        InstrumentCreator creator;  // nicht gezeigt
        creator.oeffne(laut);
        auto* zeile = creator.findChild<QLabel*>(QStringLiteral("creatorLoudness"));
        auto* angleichen = creator.findChild<QPushButton*>(QStringLiteral("creatorMatch"));
        auto* save = creator.findChild<QPushButton*>(QStringLiteral("creatorSave"));
        auto* saveAs = creator.findChild<QPushButton*>(QStringLiteral("creatorSaveAs"));
        REQUIRE(zeile != nullptr);
        REQUIRE(angleichen != nullptr);
        REQUIRE(save != nullptr);
        REQUIRE(saveAs != nullptr);
        creator.messeAbstand();
        REQUIRE(creator.abstand().has_value());
        CHECK(*creator.abstand() == doctest::Approx(6.02).epsilon(0.01));
        CHECK(zeile->text() == QStringLiteral("6.0 dB louder than the other instruments"));
        CHECK(angleichen->isEnabled());
        angleichen->click();
        CHECK(std::abs(*creator.abstand()) <= 0.05);
        CHECK(creator.instrument().laut == doctest::Approx(orgel.laut).epsilon(0.001));
        CHECK(zeile->text() == QStringLiteral("as loud as the other instruments"));
        CHECK_FALSE(angleichen->isEnabled());
        // ueber 1 geht es auch (LL6 a): ein leises wird angehoben
        creator.aendere([](Instrument& i) { i.laut = 0.05; });
        creator.messeAbstand();
        CHECK(zeile->text().endsWith(QStringLiteral("dB quieter than the other instruments")));
        REQUIRE(creator.gleicheAn());
        CHECK(std::abs(*creator.abstand()) <= 0.05);

        // --- Drums: nicht zu messen, sie folgen den Kits
        creator.oeffne(*findeInstrument(QStringLiteral("drum_kick")));
        creator.messeAbstand();
        CHECK_FALSE(creator.abstand().has_value());
        CHECK(zeile->text() == QStringLiteral("Drums follow the kits"));
        CHECK_FALSE(angleichen->isEnabled());

        // --- beim Speichern: weicht es ab, fragt der Creator — Save as it is, Match and save, Cancel
        QString antwort;
        int fragen = 0;
        QString frageText;
        lumi::ui::setzeDialogBeobachterFuerTests([&](QDialog& d) {
            if (d.objectName() == QStringLiteral("creatorLevelQuestion"))
            {
                ++fragen;
                auto& box = dynamic_cast<QMessageBox&>(d);
                frageText = box.text();
                // leer = abbrechen (ein leerer Name faende mit findChild jeden Knopf)
                if (antwort.isEmpty())
                    box.button(QMessageBox::Cancel)->click();
                else if (auto* knopf = d.findChild<QPushButton*>(antwort))
                    knopf->click();
                return static_cast<int>(QDialog::Accepted);
            }
            // der Name der eigenen Kopie: der Vorschlag
            if (auto* sichern = d.findChildren<QPushButton*>().isEmpty() ? nullptr : d.findChildren<QPushButton*>().last())
                sichern->click();
            return static_cast<int>(QDialog::Accepted);
        });
        Instrument eigenesLaut = laut;
        eigenesLaut.titel = QStringLiteral("Loud Organ");
        REQUIRE(speichereEigenes(eigenesLaut, QStringLiteral("loud_organ")));
        Instrument eigenes;
        REQUIRE(holeInstrument(QStringLiteral("user:loud_organ"), eigenes));
        creator.oeffne(eigenes);
        creator.aendere([](Instrument& i) { i.beschreibung = QStringLiteral("loud"); });
        // abbrechen: nichts geschrieben
        antwort.clear();
        save->click();
        CHECK(fragen == 1);
        CHECK(frageText == QStringLiteral("Loud Organ is 6.0 dB louder than the other instruments. Match its level before saving?"));
        CHECK(creator.geaendert());
        // so lassen: geschrieben, wie es ist
        antwort = QStringLiteral("creatorLevelKeep");
        save->click();
        CHECK(fragen == 2);
        CHECK_FALSE(creator.geaendert());
        REQUIRE(holeInstrument(QStringLiteral("user:loud_organ"), eigenes));
        CHECK(eigenes.laut == doctest::Approx(laut.laut));
        // im Fenster Instruments: die Marke bei diesem eigenen
        {
            InstrumentPalette fenster;
            fenster.liesNeu();
            REQUIRE(fenster.abstandVon(QStringLiteral("user:loud_organ")).has_value());
            CHECK(*fenster.abstandVon(QStringLiteral("user:loud_organ")) == doctest::Approx(6.02).epsilon(0.01));
            CHECK_FALSE(fenster.abstandVon(QStringLiteral("organ")).has_value());  // mitgeliefert: keine
            const auto* baum = fenster.findChild<QTreeWidget*>(QStringLiteral("instrumentPaletteTree"));
            REQUIRE(baum != nullptr);
            const auto treffer = baum->findItems(QStringLiteral("Loud Organ"), Qt::MatchExactly | Qt::MatchRecursive, 0);
            REQUIRE_FALSE(treffer.isEmpty());
            CHECK(treffer.front()->text(1) == QStringLiteral("+6.0 dB"));
            CHECK(treffer.front()->text(0) == QStringLiteral("Loud Organ"));  // der Name bleibt, wie er ist
            if (const QString bild = qEnvironmentVariable("LUMIVIZ_MATCH_LEVEL_BILD"); !bild.isEmpty())
            {
                fenster.resize(330, 560);
                (void)fenster.waehle(QStringLiteral("user:loud_organ"));
                CHECK(fenster.grab().save(bild + QStringLiteral(".instruments.png")));
            }
        }
        // angleichen und speichern: auf dem Bezug; danach fragt Save nicht mehr
        creator.aendere([](Instrument& i) { i.beschreibung = QStringLiteral("matched"); });
        antwort = QStringLiteral("creatorLevelMatchSave");
        save->click();
        CHECK(fragen == 3);
        CHECK_FALSE(creator.geaendert());
        REQUIRE(holeInstrument(QStringLiteral("user:loud_organ"), eigenes));
        CHECK(eigenes.laut == doctest::Approx(orgel.laut).epsilon(0.001));
        creator.aendere([](Instrument& i) { i.beschreibung = QStringLiteral("again"); });
        save->click();
        CHECK(fragen == 3);
        CHECK_FALSE(creator.geaendert());
        {
            InstrumentPalette fenster;
            fenster.liesNeu();
            CHECK_FALSE(fenster.abstandVon(QStringLiteral("user:loud_organ")).has_value());
        }
        // Save as... fragt ebenso (der Knopf ruft frageNachPegel vor dem Namen; der Name-Dialog
        // laeuft ueber exec und wird hier nicht geoeffnet)
        creator.aendere([](Instrument& i) { i.laut *= 4.0; });
        antwort = QStringLiteral("creatorLevelKeep");
        CHECK(creator.frageNachPegel());
        CHECK(fragen == 4);
        antwort.clear();
        CHECK_FALSE(creator.frageNachPegel());  // abgebrochen
        CHECK(saveAs->isEnabled());

        if (const QString bild = qEnvironmentVariable("LUMIVIZ_MATCH_LEVEL_BILD"); !bild.isEmpty())
        {
            creator.resize(1500, 1020);
            creator.messeAbstand();
            CHECK(creator.grab().save(bild));
        }
    }
}
