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
        Both = 2
    };

    /// Per-item role Qt::UserRole+1 — what a double-click should do.
    enum EntryType
    {
        Type_Up = 0,
        Type_Dir = 1,
        Type_Avs = 2,
        Type_Milk = 3
    };

    void setupUI();
    void setupConnections();
    void subscribeToEvents();

    void navigateTo(const QString& dirPath);
    void refresh();
    [[nodiscard]] QStringList currentNameFilters() const;
    void setStatus(const QString& text);
    void onImportResult(const std::string& path, bool ok, int noteCount);

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
    Filter m_filter = Filter::Both;
};
