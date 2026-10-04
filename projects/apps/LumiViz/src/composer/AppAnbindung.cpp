/**
 * @file AppAnbindung.cpp
 * @brief Composer: eine Komposition in der App ein- und ausschalten (Stufe C2a)
 */

#include "composer/AppAnbindung.hpp"

#include "UI/widgets/VisualizerWidget.hpp"
#include "composer/Bildquelle.hpp"
#include "composer/Komposition.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QMutexLocker>
#include <QObject>
#include <QSettings>

#include <BasicLogger.h>

#include <memory>

namespace lumi::composer
{

namespace
{
const char* const kLetzterOrdner = "composer/lastDir";
const char* const kBeendenMarke = "composerBeendenProtokoll";  ///< Eigenschaft am Fenster
}

QStringList messungDerKomposition(VisualizerWidget& widget)
{
    QMutexLocker halt(&widget.renderMutex());
    const auto* quelle = dynamic_cast<const Bildquelle*>(widget.bildquelle());
    return quelle != nullptr ? quelle->messung().alsText() : QStringList{};
}

namespace
{
/// Die Zusammenfassung der laufenden Komposition ins Log — bevor sie geht.
void protokolliereMessung(VisualizerWidget& widget)
{
    QStringList zeilen = messungDerKomposition(widget);
    {
        QMutexLocker halt(&widget.renderMutex());
        if (const auto* quelle = dynamic_cast<const Bildquelle*>(widget.bildquelle()))
            zeilen += quelle->anlaufText();
    }
    for (const QString& zeile : zeilen) BasicLogger::logInfo(zeile.toStdString());
}
}  // namespace

bool kompositionLaeuft(const VisualizerWidget& widget)
{
    return dynamic_cast<const Bildquelle*>(widget.bildquelle()) != nullptr;
}

bool starteKomposition(VisualizerWidget& widget, const QString& pfad, QStringList* bericht)
{
    Komposition komposition;
    if (!ladeKomposition(pfad, komposition, bericht)) return false;

    // Noch kein Panel, also niemand, der Start druecken koennte: die Uhr
    // laeuft sofort (C2a). Gesetzt wird vor der Uebergabe an den Render-Thread
    // — danach nur noch unter dem Render-Mutex.
    auto quelle = std::make_unique<Bildquelle>();
    quelle->setKomposition(std::move(komposition));
    quelle->uhr().starte();
    protokolliereMessung(widget);  // eine laufende wird ersetzt

    // Wird die App mit laufender Komposition beendet, ruft niemand
    // schliesseKomposition — die Zusammenfassung kaeme nie ins Log (Befund
    // S94). Einmal je Fenster: beim Beenden schreiben, falls noch eine laeuft.
    // Der Render-Thread ist dann schon angehalten, die Bildquelle lebt noch.
    if (!widget.property(kBeendenMarke).toBool())
    {
        widget.setProperty(kBeendenMarke, true);
        QObject::connect(qApp, &QCoreApplication::aboutToQuit, &widget,
                         [&widget]() { protokolliereMessung(widget); });
    }
    widget.setBildquelle(std::move(quelle));
    BasicLogger::logInfo("Composer: composition started: " +
                         QDir::toNativeSeparators(pfad).toStdString());
    return true;
}

void schliesseKomposition(VisualizerWidget& widget)
{
    if (widget.bildquelle() == nullptr) return;
    protokolliereMessung(widget);
    widget.setBildquelle(nullptr);
    BasicLogger::logInfo("Composer: composition closed");
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
