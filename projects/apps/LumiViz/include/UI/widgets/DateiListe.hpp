/**
 ****************************************************************************************
 * @file   DateiListe.hpp
 * @brief  Liste eines Datei-Browsers als Drag-Quelle (S84)
 *
 * @details
 * Gemeinsame Liste von Effect Browser und Audio Browser: gezogene Eintraege
 * reisen als Datei-URLs (`DateiDrag.hpp`). Welcher Eintrag welche Datei ist,
 * sagt `pfadZumZiehen` (leer = nicht ziehbar, etwa Ordner und ".."; solche
 * Eintraege sollten zusaetzlich ohne `Qt::ItemIsDragEnabled` angelegt werden).
 *
 * Nur KOPIEREN: ein Zug in den Explorer darf die Datei nie verschieben. Die
 * Liste selbst nimmt nichts an.
 ****************************************************************************************
 */

#pragma once

#include "UI/widgets/DateiDrag.hpp"

#include <QListWidget>
#include <QMimeData>
#include <QString>
#include <QStringList>

#include <functional>

namespace lumi::ui
{

class DateiListe : public QListWidget
{
public:
    explicit DateiListe(QWidget* parent = nullptr) : QListWidget(parent)
    {
        setDragEnabled(true);
        setDragDropMode(QAbstractItemView::DragOnly);
        setDefaultDropAction(Qt::CopyAction);
    }

    /// Datei eines Eintrags, leer = nicht ziehbar.
    std::function<QString(const QListWidgetItem*)> pfadZumZiehen;

protected:
    [[nodiscard]] QStringList mimeTypes() const override
    {
        return {QStringLiteral("text/uri-list")};
    }

    [[nodiscard]] QMimeData* mimeData(const QList<QListWidgetItem*>& items) const override
    {
        QStringList pfade;
        for (const QListWidgetItem* item : items)
        {
            const QString pfad = pfadZumZiehen ? pfadZumZiehen(item) : QString();
            if (!pfad.isEmpty()) pfade.append(pfad);
        }
        return dateiMime(pfade);
    }

    [[nodiscard]] Qt::DropActions supportedDropActions() const override
    {
        return Qt::CopyAction;
    }
};

}  // namespace lumi::ui
