/**
 * @file AppAnbindung.cpp
 * @brief Composer: eine Komposition in der App ein- und ausschalten (Stufe C2a)
 */

#include "composer/AppAnbindung.hpp"

#include "UI/widgets/VisualizerWidget.hpp"
#include "composer/Bildquelle.hpp"
#include "composer/Sitzung.hpp"

#include <QDir>
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
            .arg(QFileInfo(sitzung->datei()).fileName()),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
    if (wahl == QMessageBox::Cancel) return false;
    if (wahl == QMessageBox::Discard) return true;
    if (sitzung->speichere()) return true;
    QMessageBox::warning(eltern, titel,
                         QObject::tr("Could not save the composition:\n%1")
                             .arg(QDir::toNativeSeparators(sitzung->datei())));
    return false;
}

bool stelleKompositionWiederHer(VisualizerWidget& widget)
{
    return Sitzung::fuer(widget).stelleWiederHer();
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
