/**
 ****************************************************************************************
 * @file   AboutDialog.hpp
 * @brief  About dialog showing application information
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 *
 * @details
 * Example of a self-registering dialog using the DialogRegistry system.
 ****************************************************************************************
 */

#pragma once

#include "DialogBase.hpp"

class QLabel;
class QPushButton;

/**
 * @class AboutDialog
 * @brief About dialog for LumiViz
 *
 * Shows:
 *   - Application name and version
 *   - Copyright information
 *   - Qt and OpenGL versions
 */
class AboutDialog : public DialogBase
{
    Q_OBJECT

public:
    /**
     * @brief Construct AboutDialog
     * @param services ServiceContainer reference
     * @param parent Parent widget
     */
    explicit AboutDialog(ServiceContainer& services, QWidget* parent = nullptr);

    ~AboutDialog() override = default;

    /**
     * @brief Get dialog ID
     * @return "about"
     */
    [[nodiscard]] QString dialogId() const override { return QStringLiteral("about"); }

private:
    void setupUI();
    void setupConnections();

    // UI Elements
    QLabel* m_pLogoLabel = nullptr;
    QLabel* m_pTitleLabel = nullptr;
    QLabel* m_pVersionLabel = nullptr;
    QLabel* m_pDescriptionLabel = nullptr;
    QLabel* m_pCopyrightLabel = nullptr;
    QLabel* m_pSystemInfoLabel = nullptr;
    QPushButton* m_pCloseButton = nullptr;
};
