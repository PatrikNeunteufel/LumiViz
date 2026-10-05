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
    if (Sitzung* sitzung = Sitzung::finde(widget)) sitzung->entlade();
}

bool stelleKompositionWiederHer(VisualizerWidget& widget)
{
    return Sitzung::fuer(widget).stelleWiederHer();
}

bool oeffneKomposition(VisualizerWidget& widget, QWidget* eltern, const QString& pfad)
{
    const QString titel = QObject::tr("Open Composition");
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
