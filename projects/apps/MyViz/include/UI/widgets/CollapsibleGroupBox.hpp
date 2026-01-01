/**
 ****************************************************************************************
 * @file   CollapsibleGroupBox.hpp
 * @brief  A QGroupBox that can be collapsed/expanded
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 1.0.0
 *
 * @details
 * Provides a groupbox with a clickable header that toggles content visibility.
 * Content smoothly animates when expanding/collapsing.
 *
 * Usage:
 * @code
 * auto* group = new CollapsibleGroupBox("Shape Settings", this);
 * group->addWidget(shapeCombo);
 * group->addWidget(sidesSlider);
 * group->setCollapsed(false);  // Start expanded
 * @endcode
 ****************************************************************************************
 */

#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QToolButton>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QFrame>

/**
 * @class CollapsibleGroupBox
 * @brief A group box that can be collapsed/expanded with animation
 */
class CollapsibleGroupBox : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Construct a collapsible group box
     * @param title The title shown in the header
     * @param parent Parent widget
     */
    explicit CollapsibleGroupBox(const QString& title, QWidget* parent = nullptr);

    /**
     * @brief Set whether the group is collapsed
     * @param collapsed true to collapse, false to expand
     */
    void setCollapsed(bool collapsed);

    /**
     * @brief Check if the group is collapsed
     */
    [[nodiscard]] bool isCollapsed() const { return m_collapsed; }

    /**
     * @brief Toggle collapsed state
     */
    void toggle();

    /**
     * @brief Get the content layout to add widgets to
     */
    QVBoxLayout* contentLayout() { return m_contentLayout; }

    /**
     * @brief Add a widget to the content area
     */
    void addWidget(QWidget* widget);

    /**
     * @brief Add a layout to the content area
     */
    void addLayout(QLayout* layout);

    /**
     * @brief Set the title
     */
    void setTitle(const QString& title);

Q_SIGNALS:
    /**
     * @brief Emitted when collapsed state changes
     * @param collapsed New collapsed state
     */
    void collapsedChanged(bool collapsed);

private Q_SLOTS:
    void onToggleClicked();

private:
    void updateToggleButton();

    QToolButton* m_toggleButton = nullptr;
    QFrame* m_contentFrame = nullptr;
    QVBoxLayout* m_contentLayout = nullptr;
    QParallelAnimationGroup* m_animation = nullptr;

    bool m_collapsed = false;
    int m_contentHeight = 0;
};
