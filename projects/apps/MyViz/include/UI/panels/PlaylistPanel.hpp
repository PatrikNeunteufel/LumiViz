/**
 ****************************************************************************************
 * @file   PlaylistPanel.hpp
 * @brief  Playlist management panel
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 ****************************************************************************************
 */

#pragma once

#include "PanelBase.hpp"

class QListWidget;
class QPushButton;
class QLineEdit;

/**
 * @class PlaylistPanel
 * @brief Panel for playlist management
 *
 * Provides:
 *   - Track list display
 *   - Add/Remove tracks
 *   - Search/Filter
 *   - Drag & Drop reordering
 */
class PlaylistPanel : public PanelBase
{
    Q_OBJECT

public:
    explicit PlaylistPanel(ServiceContainer& services, QWidget* parent = nullptr);
    ~PlaylistPanel() override = default;

    [[nodiscard]] int preferredArea() const override;

protected:
    void onActivate() override;
    void onDeactivate() override;
    void saveState() override;
    void restoreState() override;

private Q_SLOTS:
    void onAddClicked();
    void onRemoveClicked();
    void onClearClicked();
    void onItemDoubleClicked();
    void onSearchChanged(const QString& text);

private:
    void setupUI();
    void setupConnections();

    // UI Elements
    QLineEdit* m_pSearchEdit = nullptr;
    QListWidget* m_pListWidget = nullptr;
    QPushButton* m_pAddButton = nullptr;
    QPushButton* m_pRemoveButton = nullptr;
    QPushButton* m_pClearButton = nullptr;
};
