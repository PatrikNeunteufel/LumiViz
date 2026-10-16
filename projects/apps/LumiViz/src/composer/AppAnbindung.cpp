/**
 * @file AppAnbindung.cpp
 * @brief Composer: eine Komposition in der App ein- und ausschalten (Stufe C2a)
 */

#include "composer/AppAnbindung.hpp"

#include "UI/widgets/VisualizerWidget.hpp"
#include "composer/Bildquelle.hpp"
#include "composer/Mute.hpp"
#include "composer/MuteKnopf.hpp"
#include "composer/Sitzung.hpp"
#include "loop/ComposerAbfragen.hpp"

#include "UI/PresetSpeichern.hpp"
#include "composer/ComposerPanel.hpp"

#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QMutexLocker>
#include <QObject>
#include <QSettings>

namespace lumi::composer
{

namespace
{
const char* const kLetzterOrdner = "composer/lastDir";
}

QStringList messungDerKomposition(VisualizerWidget& widget)
{
    QMutexLocker halt(&widget.renderMutex());
    const auto* quelle = dynamic_cast<const Bildquelle*>(widget.bildquelle());
    return quelle != nullptr ? quelle->messung().alsText() : QStringList{};
}

bool kompositionLaeuft(const VisualizerWidget& widget)
{
    return dynamic_cast<const Bildquelle*>(widget.bildquelle()) != nullptr;
}

// Seit C2b fuehrt die Sitzung des Fensters die Komposition (geladen ist nicht
// an); die Funktionen hier sind der Weg des Menues und des Starts dorthin.

bool starteKomposition(VisualizerWidget& widget, const QString& pfad, QStringList* bericht)
{
    return Sitzung::fuer(widget).oeffne(pfad, bericht);
}

void schliesseKomposition(VisualizerWidget& widget)
{
    Sitzung* sitzung = Sitzung::finde(widget);
    if (sitzung == nullptr) return;
    if (!darfKompositionErsetzen(widget, widget.window(), QObject::tr("Close Composition"))) return;
    sitzung->entlade();
}

bool darfKompositionErsetzen(VisualizerWidget& widget, QWidget* eltern, const QString& titel)
{
    Sitzung* sitzung = Sitzung::finde(widget);
    if (sitzung == nullptr || !sitzung->ungespeichert()) return true;
    const QMessageBox::StandardButton wahl = QMessageBox::question(
        eltern, titel,
        QObject::tr("The composition '%1' has unsaved changes.")
            .arg(sitzung->datei().isEmpty() ? QObject::tr("untitled")
                                            : QFileInfo(sitzung->datei()).fileName()),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
    if (wahl == QMessageBox::Cancel) return false;
    if (wahl == QMessageBox::Discard) return true;
    return speichereMitDialog(widget, eltern, false);
}

bool speichereMitDialog(VisualizerWidget& widget, QWidget* eltern, bool immerFragen)
{
    Sitzung* sitzung = Sitzung::finde(widget);
    if (sitzung == nullptr || !sitzung->geladen()) return false;
    QString pfad = sitzung->datei();
    const QString titel = immerFragen || pfad.isEmpty() ? QObject::tr("Save Composition As")
                                                        : QObject::tr("Save Composition");
    if (immerFragen || pfad.isEmpty())
    {
        QSettings einstellungen;
        const QString vorschlag =
            !pfad.isEmpty() ? pfad
                            : einstellungen.value(QLatin1String(kLetzterOrdner)).toString() +
                                  QStringLiteral("/untitled.lvcomp");
        pfad = QFileDialog::getSaveFileName(eltern, titel, vorschlag,
                                            QObject::tr("LumiViz Composition (*.lvcomp)"));
        if (pfad.isEmpty()) return false;
        if (!pfad.endsWith(QStringLiteral(".lvcomp"), Qt::CaseInsensitive))
            pfad += QStringLiteral(".lvcomp");
        einstellungen.setValue(QLatin1String(kLetzterOrdner), QFileInfo(pfad).absolutePath());
    }
    if (sitzung->speichereNach(pfad)) return true;
    QMessageBox::warning(eltern, titel,
                         QObject::tr("Could not save the composition:\n%1")
                             .arg(QDir::toNativeSeparators(pfad)));
    return false;
}

bool clipKetteGebunden(const VisualizerWidget& widget)
{
    const Sitzung* sitzung = Sitzung::finde(widget);
    return sitzung != nullptr && sitzung->bindung() == Sitzung::Bindung::Clip;
}

bool speichereClipKette(VisualizerWidget& widget, QWidget* eltern)
{
    Sitzung* sitzung = Sitzung::finde(widget);
    const QString titel = QObject::tr("Save Clip Preset As");
    if (sitzung == nullptr) return false;
    const QString alt = sitzung->gebundeneDatei();
    if (alt.isEmpty())
    {
        QMessageBox::information(eltern, titel,
                                 QObject::tr("The Effect Chain panel is just switching to another "
                                             "clip. Please try again."));
        return false;
    }

    // --- der neue Name: nie das Original (Entscheid Patrik S95)
    const QString endung = lumi::ui::kettenEndung(sitzung->gebundeneKetteHatHostGruppe());
    const QFileInfo original(alt);
    QString pfad = original.absolutePath() + QLatin1Char('/') + original.completeBaseName() +
                   QStringLiteral("_comp") + endung;
    for (;;)
    {
        pfad = QFileDialog::getSaveFileName(eltern, titel, pfad,
                                            QObject::tr("LumiViz Effect Chain (*.lvfx *.lvfx2)"));
        if (pfad.isEmpty()) return false;
        if (pfad.endsWith(QStringLiteral(".lvfx2"), Qt::CaseInsensitive))
            pfad.chop(6);
        else if (pfad.endsWith(QStringLiteral(".lvfx"), Qt::CaseInsensitive))
            pfad.chop(5);
        pfad += endung;
        if (QFileInfo(pfad) != original) break;
        QMessageBox::information(
            eltern, titel,
            QObject::tr("The preset of a clip is always saved under a new name, so the original "
                        "'%1' stays as it is. Please choose another name.")
                .arg(original.fileName()));
    }

    // --- benutzen weitere Clips das alte Preset? Welche wechseln mit?
    std::vector<std::pair<int, int>> auchDiese;
    const std::vector<std::pair<int, int>> andere = sitzung->andereClipsMitDerDatei();
    if (!andere.empty())
    {
        QDialog frage(eltern);
        frage.setWindowTitle(titel);
        auto* aufbau = new QVBoxLayout(&frage);
        aufbau->addWidget(new QLabel(
            QObject::tr("%n other clip(s) of this composition use '%1'.\n"
                        "Tick the ones that should switch to the new preset as well.",
                        nullptr, static_cast<int>(andere.size()))
                .arg(original.fileName()),
            &frage));
        auto* liste = new QListWidget(&frage);
        for (const auto& [k, c] : andere)
        {
            const Kanal& kanal = sitzung->komposition().kanaele[static_cast<std::size_t>(k)];
            const VisualClip& clip = kanal.clips[static_cast<std::size_t>(c)];
            auto* zeile = new QListWidgetItem(
                QStringLiteral("%1 · %2 – %3")
                    .arg(kanal.name, ComposerPanel::zeitText(clip.startMs),
                         ComposerPanel::zeitText(clip.endeMs)),
                liste);
            zeile->setFlags(zeile->flags() | Qt::ItemIsUserCheckable);
            zeile->setCheckState(Qt::Checked);
        }
        aufbau->addWidget(liste);
        auto* knoepfe = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &frage);
        QObject::connect(knoepfe, &QDialogButtonBox::accepted, &frage, &QDialog::accept);
        QObject::connect(knoepfe, &QDialogButtonBox::rejected, &frage, &QDialog::reject);
        aufbau->addWidget(knoepfe);
        if (frage.exec() != QDialog::Accepted) return false;  // nichts geschrieben
        for (int i = 0; i < liste->count(); ++i)
            if (liste->item(i)->checkState() == Qt::Checked)
                auchDiese.push_back(andere[static_cast<std::size_t>(i)]);
    }

    if (sitzung->speichereClipKette(pfad, auchDiese)) return true;
    QMessageBox::warning(eltern, titel,
                         QObject::tr("Could not save:\n%1").arg(QDir::toNativeSeparators(pfad)));
    return false;
}

bool beginneKomposition(VisualizerWidget& widget, QWidget* eltern)
{
    if (!darfKompositionErsetzen(widget, eltern, QObject::tr("New Composition"))) return false;
    Sitzung::fuer(widget).neu();
    return true;
}

bool stelleKompositionWiederHer(VisualizerWidget& widget)
{
    return Sitzung::fuer(widget).stelleWiederHer();
}

bool transport(TransportCommandEvent::Action aktion)
{
    Sitzung* sitzung = Sitzung::fuehrende();
    if (sitzung == nullptr) return false;  // kein Audiokanal fuehrt: der Player wie immer
    using Aktion = TransportCommandEvent::Action;
    switch (aktion)
    {
        case Aktion::PlayPause:
            if (sitzung->laeuft())
                sitzung->pausiere();
            else
                sitzung->spiele();
            return true;
        case Aktion::Stop:
            sitzung->stoppe();
            return true;
        case Aktion::Next:
            (void)sitzung->springeZumSong(true);  // nach dem letzten Song bleibt alles, wo es ist
            return true;
        case Aktion::Previous:
            (void)sitzung->springeZumSong(false);
            return true;
        case Aktion::VolumeUp:
        case Aktion::VolumeDown:
        case Aktion::ToggleMute:
            break;  // Lautstaerke und Mute bleiben beim Player
    }
    return false;
}

AudioStreamHandle tonMixer()
{
    const Sitzung* sitzung = Sitzung::fuehrende();
    return sitzung != nullptr ? static_cast<AudioStreamHandle>(sitzung->tonMixer()) : INVALID_STREAM;
}

bool fuehrtTon()
{
    return Sitzung::fuehrende() != nullptr;
}

std::vector<double> loopStellenMs(const QString& datei)
{
    const Sitzung* sitzung = Sitzung::fuehrende();
    return sitzung != nullptr ? sitzung->loopStellenMs(datei) : std::vector<double>{};
}

bool mitlaufenSeitenweise()
{
    return Sitzung::mitlaufenSeitenweise();
}

void meldeLoopAbfragenAn()
{
    loop::setzeComposerAbfragen({[] { return fuehrtTon(); },
                                 [](const QString& datei) { return loopStellenMs(datei); },
                                 [] { return mitlaufenSeitenweise(); }});
}

bool meldetBindung()
{
    return Sitzung::meldetBindung();
}

QWidget* erzeugeMuteKnopf(ServiceContainer& dienste, QWidget* eltern)
{
    return new MuteKnopf(dienste, eltern);
}

float muteFaktorFuerVisuals()
{
    const MuteSchalter* schalter = MuteSchalter::vorhanden();
    return schalter != nullptr ? static_cast<float>(schalter->faktorFuerVisuals()) : 1.0f;
}

TonStand tonStand()
{
    TonStand stand;
    const Sitzung* sitzung = Sitzung::fuehrende();
    if (sitzung == nullptr) return stand;
    const Sitzung::TonStand s = sitzung->tonStand();
    stand.fuehrt = true;
    stand.laeuft = sitzung->laeuft();
    stand.hatSong = s.hatSong;
    stand.song = s.song;
    stand.stelleMs = s.stelleMs;
    stand.hatAus = s.hatAus;
    stand.aus = s.aus;
    stand.ausStelleMs = s.ausStelleMs;
    return stand;
}

bool springeImSong(double anteil)
{
    Sitzung* sitzung = Sitzung::fuehrende();
    if (sitzung == nullptr) return false;
    (void)sitzung->springeImSong(anteil);  // ohne laufenden Song bleibt alles, wo es ist
    return true;
}

bool oeffneKomposition(VisualizerWidget& widget, QWidget* eltern, const QString& pfad)
{
    const QString titel = QObject::tr("Open Composition");
    if (!darfKompositionErsetzen(widget, eltern, titel)) return false;
    QString datei = pfad;
    if (datei.isEmpty())
    {
        QSettings einstellungen;
        datei = QFileDialog::getOpenFileName(
            eltern, titel, einstellungen.value(QLatin1String(kLetzterOrdner)).toString(),
            QObject::tr("LumiViz Composition (*.lvcomp);;All Files (*)"));
        if (datei.isEmpty()) return false;
        einstellungen.setValue(QLatin1String(kLetzterOrdner), QFileInfo(datei).absolutePath());
    }

    QStringList bericht;
    if (!starteKomposition(widget, datei, &bericht))
    {
        QMessageBox::warning(eltern, titel,
                             QObject::tr("Could not load the composition:\n%1")
                                 .arg(QDir::toNativeSeparators(datei)));
        return false;
    }
    if (!bericht.isEmpty())
    {
        QMessageBox::information(eltern, titel,
                                 QObject::tr("The composition was loaded with remarks:\n%1")
                                     .arg(bericht.join(QLatin1Char('\n'))));
    }
    return true;
}

}  // namespace lumi::composer
