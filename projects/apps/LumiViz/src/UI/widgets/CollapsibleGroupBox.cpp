/**
 ****************************************************************************************
 * @file   CollapsibleGroupBox.cpp
 * @brief  CollapsibleGroupBox implementation
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#include "UI/widgets/CollapsibleGroupBox.hpp"

#include <QLabel>

// =============================================================================
// Construction
// =============================================================================

CollapsibleGroupBox::CollapsibleGroupBox(const QString& title, QWidget* parent)
    : QWidget(parent)
{
    // Main layout
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Toggle button (acts as header)
    m_toggleButton = new QToolButton(this);
    m_toggleButton->setStyleSheet(
        "QToolButton {"
        "  border: none;"
        "  background-color: palette(button);"
        "  padding: 6px 8px;"
        "  font-weight: bold;"
        "  text-align: left;"
        "}"
        "QToolButton:hover {"
        "  background-color: palette(midlight);"
        "}"
    );
    m_toggleButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_toggleButton->setArrowType(Qt::DownArrow);
    m_toggleButton->setText(title);
    m_toggleButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_toggleButton->setCheckable(true);
    m_toggleButton->setChecked(true);  // Start expanded

    connect(m_toggleButton, &QToolButton::clicked,
            this, &CollapsibleGroupBox::onToggleClicked);

    // Header row: toggle button plus optional extra widgets (addHeaderWidget)
    m_headerLayout = new QHBoxLayout();
    m_headerLayout->setContentsMargins(0, 0, 0, 0);
    m_headerLayout->setSpacing(0);
    m_headerLayout->addWidget(m_toggleButton, 1);
    mainLayout->addLayout(m_headerLayout);

    // Content frame
    m_contentFrame = new QFrame(this);
    m_contentFrame->setFrameShape(QFrame::StyledPanel);
    m_contentFrame->setFrameShadow(QFrame::Sunken);
    m_contentFrame->setStyleSheet(
        "QFrame {"
        "  border: 1px solid palette(mid);"
        "  border-top: none;"
        "  background-color: palette(base);"
        "}"
    );

    m_contentLayout = new QVBoxLayout(m_contentFrame);
    m_contentLayout->setContentsMargins(8, 8, 8, 8);
    m_contentLayout->setSpacing(6);

    mainLayout->addWidget(m_contentFrame);

    // Animation for smooth collapse/expand
    m_animation = new QParallelAnimationGroup(this);

    auto* contentAnimation = new QPropertyAnimation(m_contentFrame, "maximumHeight", this);
    contentAnimation->setDuration(150);
    contentAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    m_animation->addAnimation(contentAnimation);

    // After expanding, release the height cap again. The animation ends on the
    // height measured at animation start — leaving it as maximumHeight would
    // squeeze/clip anything that appears later (dependsOn visibility, stage
    // previews) inside the group.
    connect(m_animation, &QParallelAnimationGroup::finished, this, [this]() {
        if (!m_collapsed)
        {
            m_contentFrame->setMaximumHeight(QWIDGETSIZE_MAX);
        }
    });

    // Start expanded
    m_collapsed = false;
    updateToggleButton();
}

// =============================================================================
// Public Methods
// =============================================================================

void CollapsibleGroupBox::setCollapsed(bool collapsed)
{
    if (m_collapsed == collapsed)
    {
        return;
    }

    m_collapsed = collapsed;

    // Get the content height
    if (!m_collapsed)
    {
        m_contentFrame->setMaximumHeight(QWIDGETSIZE_MAX);
        m_contentHeight = m_contentFrame->sizeHint().height();
    }
    else
    {
        m_contentHeight = m_contentFrame->height();
    }

    // Animate
    auto* contentAnim = qobject_cast<QPropertyAnimation*>(m_animation->animationAt(0));
    if (contentAnim)
    {
        m_animation->stop();
        contentAnim->setStartValue(m_collapsed ? m_contentHeight : 0);
        contentAnim->setEndValue(m_collapsed ? 0 : m_contentHeight);
        m_animation->start();
    }

    updateToggleButton();
    Q_EMIT collapsedChanged(m_collapsed);
}

void CollapsibleGroupBox::toggle()
{
    setCollapsed(!m_collapsed);
}

void CollapsibleGroupBox::addWidget(QWidget* widget)
{
    m_contentLayout->addWidget(widget);
}

void CollapsibleGroupBox::addLayout(QLayout* layout)
{
    m_contentLayout->addLayout(layout);
}

void CollapsibleGroupBox::setTitle(const QString& title)
{
    m_toggleButton->setText(title);
}

void CollapsibleGroupBox::addHeaderWidget(QWidget* widget)
{
    m_headerLayout->addWidget(widget, 0);
}

// =============================================================================
// Private Slots
// =============================================================================

void CollapsibleGroupBox::onToggleClicked()
{
    toggle();
}

// =============================================================================
// Private Methods
// =============================================================================

void CollapsibleGroupBox::updateToggleButton()
{
    m_toggleButton->setArrowType(m_collapsed ? Qt::RightArrow : Qt::DownArrow);
    m_toggleButton->setChecked(!m_collapsed);
}
