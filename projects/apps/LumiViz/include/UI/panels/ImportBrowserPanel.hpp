/**
 ****************************************************************************************
 * @file   ImportBrowserPanel.hpp
 * @brief  Directory browser for importing AVS/MilkDrop presets
 *
 * @author Patrik Neunteufel
 * @date   July 2026
 * @version 1.0.0
 *
 * @details
 * ## ImportBrowserPanel
 *
 * A file-browser panel (styled like the Playlist) for quickly checking imports:
 * - Shows the contents of a folder, filtered to `*.avs` / `*.milk` / both.
 * - Double-click a folder to descend, ".." or the Up button to ascend.
 * - Double-click a preset to import it into the Multi Effect host.
 *
 * ## Integration
 *
 * The panel does not import itself — it publishes an ImportAvsPresetEvent (or
 * ImportMilkPresetEvent) carrying the file path. MainWindow owns the import
 * orchestration (host auto-activation, render mutex, report dialog) and answers
 * with an AvsImportResultEvent, which the panel shows as a non-modal status.
 *
 * ```
 * ┌────────────────────┐  ImportAvsPresetEvent{path}  ┌────────────┐
 * │ ImportBrowserPanel │ ────────────────────────────►│ MainWindow │
 * │                    │◄──── AvsImportResultEvent ────│            │
 * └────────────────────┘                              └────────────┘
 * ```
 ****************************************************************************************
 */

#pragma once

#include "PanelBase.hpp"

#include <QDir>
#include <QString>

#include <string>

class QListWidget;
class QListWidgetItem;
class QPushButton;
class QLineEdit;
class QComboBox;
class QLabel;

/**
 * @class ImportBrowserPanel
 * @brief Folder browser that imports AVS/MilkDrop presets on double-click
 */
class ImportBrowserPanel : public PanelBase
{
    Q_OBJECT

public:
    explicit ImportBrowserPanel(ServiceContainer& services, QWidget* parent = nullptr);
    ~ImportBrowserPanel() override = default;

    [[nodiscard]] int preferredArea() const override;

    /// Per-item role Qt::UserRole+1 — what a double-click should do. Oeffentlich,
    /// weil es der Eingabe-Vertrag von nextPresetRow() ist.
    enum EntryType
    {
        Type_Up = 0,
        Type_Dir = 1,
        Type_Avs = 2,
        Type_Milk = 3,
        Type_Lvfx = 4
    };

    /**
     * @brief Nachbar-Zeile, die ein Preset traegt — der Kern der
     *        Hotkey-Navigation (docs/ui/Hotkey_Konzept.md, Stufe 1).
     *
     * @param types  EntryType je Zeile in Anzeigereihenfolge
     * @param from   aktuelle Zeile, oder -1 fuer "keine Auswahl"
     * @param delta  Vorzeichen entscheidet die Richtung
     * @return Zeilenindex, oder -1 wenn in dieser Richtung keines mehr kommt
     *
     * Ordner und ".." werden uebersprungen (keine Rekursion), am Ende wird
     * angehalten statt umzulaufen. Frei von Qt-Widgets, damit die Regel
     * pruefbar ist.
     */
    [[nodiscard]] static int nextPresetRow(const QList<int>& types, int from,
                                          int delta);

protected:
    void onActivate() override;
    void onDeactivate() override;
    void saveState() override;
    void restoreState() override;

private Q_SLOTS:
    void onItemDoubleClicked(QListWidgetItem* item);
    void onUpClicked();
    void onBrowseClicked();
    void onPathEntered();
    void onFilterChanged(int index);
    void onSearchChanged(const QString& text);

private:
    /// Which preset kinds to list (folders are always shown for navigation).
    enum class Filter
    {
        Avs = 0,
        Milk = 1,
        Lvfx = 2,   ///< native LumiViz effect chains
        All = 3
    };

    void setupUI();
    void setupConnections();
    void subscribeToEvents();

    void navigateTo(const QString& dirPath);
    void refresh();
    [[nodiscard]] QStringList currentNameFilters() const;
    [[nodiscard]] static int entryTypeForSuffix(const QString& suffix);
    void setStatus(const QString& text);
    void onImportResult(const std::string& path, bool ok, int noteCount);
    void onPresetStep(int delta);
    /// Forget the persisted start folder and return to the application
    /// directory, where the bundled presets/ folder lives (S73; was the home
    /// directory before). Triggered by ResetImportBrowserDirEvent from the
    /// Settings panel.
    void resetStoredDir();

    /// Das zuletzt geladene Preset wieder laden (S73). Laeuft verzoegert aus
    /// dem Konstruktor, damit der Visualizer schon steht, wenn das
    /// Import-Event kommt.
    void restoreLastPreset();

    /// Gemeinsamer Weg "Eintrag oeffnen" fuer Doppelklick, Blaettern-Hotkey
    /// und die Wiederherstellung beim Start — eine Wahrheit, ein Ort, an dem
    /// `m_lastPreset` fortgeschrieben wird.
    void openEntry(int type, const QString& path);

    /// Das laufende Preset in der Liste markieren, ohne es zu laden.
    void selectLastPresetInList();

    // UI Elements
    QLineEdit* m_pPathEdit = nullptr;
    QPushButton* m_pUpButton = nullptr;
    QPushButton* m_pBrowseButton = nullptr;
    QComboBox* m_pFilterCombo = nullptr;
    QLineEdit* m_pSearchEdit = nullptr;
    QListWidget* m_pListWidget = nullptr;
    QLabel* m_pStatusLabel = nullptr;

    // State
    QDir m_dir;
    Filter m_filter = Filter::All;

    /// Zuletzt GELADENES Preset (absoluter Pfad, leer = keins). Wird beim
    /// Doppelklick bzw. beim Blaettern gesetzt — also genau dann, wenn ein
    /// Preset wirklich zur Anzeige kam, nicht schon beim blossen Anklicken.
    QString m_lastPreset;

    /// Lives for the whole panel lifetime (NOT cleared in onDeactivate) — the
    /// Settings-panel reset must reach this panel even while it is hidden.
    IEventBus::SubscriberHandle m_resetSubscription;
    /// Permanent wie m_resetSubscription: der Hotkey wirkt auch, wenn dieses
    /// Panel nicht sichtbar ist.
    IEventBus::SubscriberHandle m_stepSubscription;
};
