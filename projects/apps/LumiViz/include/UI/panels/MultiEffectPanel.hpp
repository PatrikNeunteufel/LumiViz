/**
 ****************************************************************************************
 * @file   MultiEffectPanel.hpp
 * @brief  Tree editor for the Multi Effect host's effect chain (Import Roadmap 5.7b)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 0.1.0
 *
 * @details
 * A dockable panel that edits the `MultiEffectVisualizer`'s effect chain: a tree
 * view of the nested lists/effects (add / remove / reorder / enable) plus a
 * per-node parameter editor (spinners, colors, EEL script fields). All chain
 * mutations run under the widget's renderMutex() and re-run compileChain(), the
 * editable-chain contract from decision E5.
 *
 * The panel follows the active visualizer via VisualizerChangedEvent; it is only
 * active when the "Multi Effect" host is selected (otherwise it shows a hint).
 ****************************************************************************************
 */

#pragma once

#include "UI/panels/PanelBase.hpp"
#include "visualizers/multieffect/EffectChain.hpp"
#include "visualizers/modules/ColorGradientModule.hpp"  // gradient combo delegate backing

#include <QList>
#include <QTreeWidget>

class QFormLayout;

#include <array>
#include <functional>
#include <vector>

class QComboBox;
class QToolButton;
class QVBoxLayout;
class QScrollArea;
class QLabel;
class QMutex;
class QNetworkAccessManager;
class MultiEffectVisualizer;
namespace lumi::ui {
class EffectPaletteWindow;
}

/// Where a drop landed relative to the target item (public mirror of Qt's
/// protected QAbstractItemView::DropIndicatorPosition).
enum class ChainDrop
{
    OnItem,    ///< onto the item (into a list, else after a leaf)
    Above,     ///< as a sibling before the target
    Below,     ///< as a sibling after the target
    Viewport,  ///< onto empty space -> end of root
};

/**
 * @class ChainTreeWidget
 * @brief QTreeWidget that reports internal drops to a callback instead of
 *        moving items itself.
 *
 * The effect chain (not the view) owns node ordering, so the view must never
 * mutate on its own — that would desync the path-in-UserRole model. dropEvent
 * forwards (source, target, where) to @ref onDrop and swallows the event; the
 * panel mutates the chain and rebuilds the tree from it. No Q_OBJECT: only a
 * virtual override, no new signals -> no moc needed.
 */
class ChainTreeWidget : public QTreeWidget
{
public:
    using QTreeWidget::QTreeWidget;

    std::function<void(QTreeWidgetItem* src, QTreeWidgetItem* target, ChainDrop where)>
        onDrop;
    /// Ein Eintrag aus dem Palettenfenster wurde abgelegt (S79).
    std::function<void(int paletteIndex, QTreeWidgetItem* target, ChainDrop where)>
        onPaletteDrop;
    std::function<void()> onDeleteKey;  ///< Delete/Backspace pressed

    /// Spalten-Mindestbreiten (S78, Entscheid 2a): keine Spalte wird schmaler
    /// als ihr Eintrag; die letzte fuellt den Rest. Passt die Summe nicht mehr
    /// in den Viewport, scrollt NUR der Baum mit seinem eigenen Balken — das
    /// Panel wird dadurch nie breiter. Einmal nach dem Setzen der Spalten rufen.
    void setColumnMinimums(const QList<int>& minimums);

    /// Drag-&-Drop-Einstellungen des Kettenbaums an EINER Stelle (S80): das
    /// Panel ruft sie beim Aufbau, der UI-Test (`test_PaletteUi`) ebenso —
    /// sonst prueft der Test eine Konfiguration, die es im Panel nicht gibt.
    void configureDragDrop();

protected:
    /// Neben den eigenen Zeilen auch Paletteneintraege (`lumi::ui::kPaletteMime`)
    /// annehmen — erst dadurch zeigt Qt beim Ziehen den Einfuege-Indikator.
    [[nodiscard]] QStringList mimeTypes() const override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    /// Klemmt die verstellbaren Spalten auf ihr Minimum und dehnt die letzte
    /// auf den Rest des Viewports (nie unter ihr Minimum).
    void fitColumns();

    QList<int> m_columnMinimums;
    bool m_fitting = false;  ///< Rekursionssperre (resizeSection -> sectionResized)
};

/**
 * @class MultiEffectPanel
 * @brief Dockable tree editor for the multi-effect chain.
 */
class MultiEffectPanel : public PanelBase
{
    Q_OBJECT

public:
    explicit MultiEffectPanel(ServiceContainer& services, QWidget* parent = nullptr);
    ~MultiEffectPanel() override = default;

    /// Das Panel bringt seine Scrollareas selbst mit (aussen waagrecht, Baum
    /// und Editor senkrecht) — keine zweite von Qt-ADS darum (S78).
    [[nodiscard]] bool scrollsItself() const override { return true; }

private:
    void setupUI();
    void connectToActiveVisualizer();
    void setHost(MultiEffectVisualizer* host, QMutex* mutex);

    // Tree <-> chain
    void rebuildTree();
    /// Kamera-Freigabe-Dialog (S70): fragt EINMAL je geladener Kette nach,
    /// wenn sie eine Kamera-Quelle enthaelt und die Freigabe dieses App-Laufs
    /// noch fehlt — der Dialog selbst ist die ausdrueckliche Nutzeraktion
    /// des Kamera-Vertrags (Offene_Punkte §7). Laeuft queued nach rebuildTree.
    void pruefeKameraFreigabe();
    void addTreeItem(QTreeWidgetItem* parentItem,
                     const lumi::multieffect::ChainNode& node, QList<int> path);
    [[nodiscard]] lumi::multieffect::ChainNode* nodeAtPath(const QList<int>& path);
    /// Herkunfts-Kopf fuer den Shader-Export dieses Knotens (S72) — leer, wenn
    /// der Typ keine Herkunft traegt oder sie unbelegt ist. Nimmt den STAND
    /// BEIM AUFRUF, damit ein zwischenzeitlicher Import im Export landet.
    [[nodiscard]] QString herkunftKopfFuer(const QList<int>& path);
    /// Import-Hinweise des zuletzt geladenen ISF-Shaders je Knoten (S72).
    /// Sie stehen als Zeile IM PANEL, nicht in einem Dialog (Entscheid
    /// Patrik: "das sollte kein Dialogfeld zeigen sondern einfach im Panel
    /// stehn") — ein Dialog je Effekt beim Laden einer Kette waere eine
    /// Klickorgie. Bewusst nur zur Laufzeit: die Hinweise gelten dem eben
    /// geladenen Shader, nicht dem Preset.
    QHash<std::uint64_t, QString> m_isfHinweise;
    [[nodiscard]] QList<int> currentPath() const;
    /// All selected item paths (same parent, sorted by index). Falls back to the
    /// current item when the selection is empty.
    [[nodiscard]] QList<QList<int>> selectedPaths() const;
    [[nodiscard]] QTreeWidgetItem* itemAtPath(const QList<int>& path) const;
    void selectByPath(const QList<int>& path);
    /// SSOT-Nachzug (S76): stellt Augen, Element-Labels und Sektions-Zaehler
    /// eines Milkdrop-Nodes auf den aktuellen Preset-Zustand ein — Gegenrichtung
    /// zur Auge→Checkbox-Kopplung, damit die Panel-Checkbox „… aktiv" den Baum
    /// nicht stehen laesst.
    void syncMilkEyes(const QList<int>& nodePath);
    void selectPaths(const QList<QList<int>>& paths);
    /// Keep only selected items on the current item's level (same effect list).
    void enforceSameLevelSelection();

    // Mutations (all lock the render mutex + recompile)
    void mutate(const QList<int>& path,
                const std::function<void(lumi::multieffect::ChainNode&)>& fn);
    void mutateStructure(const std::function<void()>& fn);
    void setNodeEnabled(const QList<int>& path, bool enabled);
    /// „+": Palettenfenster oeffnen bzw. nach vorn holen (S79).
    void openPalette();
    /// Paletteneintrag an der Auswahl einfuegen (Button „Add" im Fenster).
    void onAddEffect(int paletteIndex);
    /// Paletteneintrag an der Drop-Stelle einfuegen (Drag aus dem Fenster).
    void onPaletteDrop(int paletteIndex, QTreeWidgetItem* target, ChainDrop where);
    /// Gemeinsamer Kern: neuen Knoten als `pos`-tes Kind von `parentPath`
    /// einfuegen (pos < 0 = anhaengen), markieren, Editor aufbauen.
    bool insertPaletteNode(int paletteIndex, const QList<int>& parentPath, int pos);
    /// Milkdrop-Element (Wave/Shape/Sprite) in den Milkdrop-Node an `anchorPath`.
    bool insertMilkElement(int paletteIndex, const QList<int>& anchorPath);
    /// „Preset as Group" (S82): Preset-Datei → Inhalt einer Host-Gruppe, mit
    /// Meldungen (Probleme, verschachtelte Gruppen). Leeres `datei` = Datei-
    /// dialog (setzt `datei`). false = abgebrochen oder unlesbar.
    bool presetGruppeLaden(QString& datei,
                           std::vector<lumi::multieffect::ChainNode>& outInhalt);
    void onRemove();
    void onClone();
    void onMove(int delta);
    /// Load a SuperScope-3D template's EEL quartet into the node at `path`.
    void applyScope3DPreset(const QList<int>& path, int presetIndex);
    /// Load a 3D-Camera template's EEL slots into the node at `path`.
    void applyCamera3DPreset(const QList<int>& path, int presetIndex);
    void onDropRequested(QTreeWidgetItem* src, QTreeWidgetItem* target,
                         ChainDrop where);
    /// Move src to the drop destination while the render mutex is held (called
    /// from within mutateStructure). Returns true + the new path on success.
    bool moveNodeLocked(const QList<int>& srcPath, const QList<int>& targetPath,
                        ChainDrop where, QList<int>& finalPath);
    /// Move several same-parent nodes as a block to the drop destination.
    bool moveNodesLocked(const QList<QList<int>>& srcPaths, const QList<int>& targetPath,
                         ChainDrop where, QList<QList<int>>& finalPaths);
    void onItemChanged(QTreeWidgetItem* item, int column);
    void onSelectionChanged();

    // Property editor
    void buildPropertyEditor(const QList<int>& path);
    void clearPropertyEditor();
    /// Breiten-Vertrag des Editors (S78), zentral fuer JEDEN Knotentyp — laeuft
    /// am Ende von buildPropertyEditor ueber die fertige Seite: Felder bekommen
    /// die Mindestbreite, Combos richten sich nicht mehr nach ihrem laengsten
    /// Eintrag, Beschreibungstexte brechen um und duerfen die Seite nicht
    /// verbreitern,
    /// Code-Felder scrollen in sich. Doku: MultiEffectPanel.md §Feldbreiten.
    void begrenzeFeldbreiten(QFormLayout* form);
    /// Mindestbreite des Editor-Bereichs = Mindestbreite der aktuellen Seite;
    /// ist das Dock schmaler, scrollt die aeussere Scrollarea seitlich (S78).
    /// Queued nach dem Aufbau: neue Kinder zaehlen erst, wenn sie gezeigt sind.
    void aktualisiereMindestbreite();
    /// Breite zwischen Feld und Aussenkante des Editors (Innenrand, Rahmen,
    /// senkrechter Balken) — Baustein beider Mindestbreiten.
    [[nodiscard]] int editorRahmen() const;
    /// Zeile „Voreinstellung" (Laden/Speichern/Loeschen) — gilt fuer jeden
    /// Knotentyp, s. `NodePresetStore` und Knoten-Parameter-Konzept §3.
    void addPresetRow(QFormLayout* form, const QList<int>& path,
                      const lumi::multieffect::EffectParams& params);
    /// Speichern-Dialog: Name + abwaehlbare Felder. false = abgebrochen.
    /// Abgewaehlte Felder fehlen in der Datei und bleiben beim Laden stehen.
    bool askPresetToSave(const lumi::multieffect::EffectParams& params,
                         QString& outName, QStringList& outFields);
    /// Bild-Zeile der Bild-Knoten: Status + „Choose…"/„Clear" (S50-Vorgabe).
    /// `set(node, filename, base64)` schreibt beide Felder in einem Zug.
    void addImageRow(
        QFormLayout* form, const QList<int>& path, const std::string& filename,
        const std::string& imageData,
        std::function<void(lumi::multieffect::ChainNode&, std::string, std::string)> set);
    /// 7x7-Gitter fuer den Convolution-Kernel (49 Werte, Mitte hervorgehoben).
    void addKernelGrid(QFormLayout* form, const QList<int>& path,
                       const std::array<int, 49>& kernel);
    /// Stuetzstellen-Liste der Color-Map (Position 0..255 + Farbe, +/−).
    void addGradientStops(QFormLayout* form, const QList<int>& path,
                          const std::vector<int>& stopPos,
                          const std::vector<uint32_t>& stopColor);

    MultiEffectVisualizer* m_host = nullptr;
    QMutex* m_mutex = nullptr;

    /// Shadertoy-Import (Strang S3): GET nur auf Knopfdruck, lazy angelegt
    QNetworkAccessManager* m_netManager = nullptr;

    ChainTreeWidget* m_tree = nullptr;
    lumi::ui::EffectPaletteWindow* m_palette = nullptr;  ///< lazy, S79
    bool m_active = false;  ///< Multi-Effect-Visualizer angebunden?
    QToolButton* m_addButton = nullptr;
    QToolButton* m_removeButton = nullptr;
    QToolButton* m_cloneButton = nullptr;
    QToolButton* m_upButton = nullptr;
    QToolButton* m_downButton = nullptr;
    QScrollArea* m_propScroll = nullptr;
    QWidget* m_propContainer = nullptr;
    QVBoxLayout* m_propLayout = nullptr;
    QWidget* m_propPage = nullptr;  ///< current editor page (deleted as a whole)
    QLabel* m_hint = nullptr;

    bool m_updating = false;   ///< guards item-changed while rebuilding
    bool m_selecting = false;  ///< guards recursive selection enforcement
    /// Kamera-Dialog schon gezeigt (je geladener Kette; Reset bei
    /// EffectChainChangedEvent/setHost — Struktur-Edits fragen nicht erneut).
    bool m_kameraDialogGezeigt = false;

    /// Non-null backing for the gradient combo's preview delegate (the delegate
    /// renders each preset into its own temp module; it only needs a live ptr).
    lumi::modules::ColorGradientModule m_gradientPreview;
};
