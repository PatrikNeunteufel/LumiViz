/**
 ****************************************************************************************
 * @file   EffectPaletteWindow.hpp
 * @brief  Nicht-modales Fenster zum Einfuegen von Effekten in die Kette
 *
 * @author Patrik Neunteufel
 * @date   September 2026
 *
 * @details
 * Ersetzt die Add-Combo der Effektkette (Entscheid Patrik S79): 84 Module in
 * einer flachen Liste liessen sich weder suchen noch filtern. Das Fenster
 * bleibt offen, bis man es schliesst, und fuegt auf zwei Wegen ein:
 *   - Button „Add" / Doppelklick / Enter: in die gewaehlte Liste der Kette,
 *     sonst unter den markierten Effekt, sonst ans Ende (wie bisher „+");
 *   - Drag & Drop: an die Stelle im Kettenbaum, auf die man zieht.
 * Das Einfuegen selbst macht das Panel — das Fenster meldet nur den
 * Paletten-Index (`addRequested`, bzw. MIME `kPaletteMime` beim Ziehen).
 ****************************************************************************************
 */
#pragma once

#include <QStringList>
#include <QTreeWidget>
#include <QWidget>

#include <vector>

class QLabel;
class QLineEdit;
class QPushButton;
class QToolButton;

namespace lumi::ui {

/// MIME-Typ eines gezogenen Paletteneintrags; Nutzlast = Index als Dezimaltext.
inline constexpr const char* kPaletteMime = "application/x-lumiviz-palette-index";

/// Baum der Palette: zieht Eintraege als `kPaletteMime`.
class PaletteTree : public QTreeWidget
{
public:
    using QTreeWidget::QTreeWidget;

protected:
    [[nodiscard]] QStringList mimeTypes() const override;
    /// Nur kopieren: bei „Move" entfernte Qt nach dem Drop die Quellzeile.
    [[nodiscard]] Qt::DropActions supportedDropActions() const override
    {
        return Qt::CopyAction;
    }
    [[nodiscard]] QMimeData* mimeData(const QList<QTreeWidgetItem*>& items) const override;
};

class EffectPaletteWindow : public QWidget
{
    Q_OBJECT

public:
    explicit EffectPaletteWindow(QWidget* parent = nullptr);

    /// Einfuegen erlaubt? (nur mit aktivem Multi-Effect-Visualizer)
    void setInsertEnabled(bool enabled);

    /// Nach einem erfolgreichen Einfuegen (Button ODER Drop): in „Recently used".
    void noteUsed(int paletteIndex);

    /// Oeffnen bzw. nach vorn holen, Fokus in die Suche.
    void present();

signals:
    void addRequested(int paletteIndex);

protected:
    void hideEvent(QHideEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void buildTree();
    void applyFilter();
    void updateDetail();
    void requestAdd();
    void toggleFavorite();
    void loadSettings();
    void saveLists() const;

    /// Paletten-Index des aktuellen Eintrags, -1 bei Kopf/Leer.
    [[nodiscard]] int currentIndex() const;
    [[nodiscard]] bool matches(int paletteIndex) const;
    QTreeWidgetItem* makeLeaf(QTreeWidgetItem* parent, int paletteIndex);

    QLineEdit* m_search = nullptr;
    std::vector<QToolButton*> m_originChips;  ///< je lumi::ui::Origin
    std::vector<QToolButton*> m_roleChips;    ///< je lumi::ui::Role
    PaletteTree* m_tree = nullptr;
    QTreeWidgetItem* m_favSection = nullptr;
    QTreeWidgetItem* m_recentSection = nullptr;
    QLabel* m_detail = nullptr;
    QToolButton* m_favButton = nullptr;
    QPushButton* m_addButton = nullptr;
    bool m_insertEnabled = false;

    QList<int> m_favorites;  ///< Paletten-Indizes
    QList<int> m_recent;     ///< juengster zuerst
};

}  // namespace lumi::ui
