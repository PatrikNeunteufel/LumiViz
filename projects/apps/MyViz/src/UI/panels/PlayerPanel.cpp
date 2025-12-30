/**
 ****************************************************************************************
 * @file   PlayerPanel.cpp
 * @brief  PlayerPanel implementation (Stub)
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 ****************************************************************************************
 */

#include "UI/panels/PlayerPanel.hpp"
#include "services/PanelRegistry.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QStyle>

// =============================================================================
// Construction
// =============================================================================

PlayerPanel::PlayerPanel(ServiceContainer& services, QWidget* parent)
    : PanelBase(services, QStringLiteral("player"), tr("Player"), parent)
{
    setupUI();
    setupConnections();
}

// =============================================================================
// IPanel Implementation
// =============================================================================

int PlayerPanel::preferredArea() const
{
    return Qt::BottomDockWidgetArea;
}

// =============================================================================
// Lifecycle
// =============================================================================

void PlayerPanel::onActivate()
{
    // TODO: Subscribe to audio events
}

void PlayerPanel::onDeactivate()
{
    // TODO: Unsubscribe from audio events
}

// =============================================================================
// Private Methods
// =============================================================================

void PlayerPanel::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);

    // Track info
    m_pTrackLabel = new QLabel(tr("No track loaded"), this);
    m_pTrackLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_pTrackLabel);

    // Progress slider
    m_pProgressSlider = new QSlider(Qt::Horizontal, this);
    m_pProgressSlider->setRange(0, 100);
    m_pProgressSlider->setValue(0);
    mainLayout->addWidget(m_pProgressSlider);

    // Time label
    m_pTimeLabel = new QLabel(QStringLiteral("00:00 / 00:00"), this);
    m_pTimeLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_pTimeLabel);

    // Control buttons
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_pPrevButton = new QPushButton(this);
    m_pPrevButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
    m_pPrevButton->setToolTip(tr("Previous"));
    buttonLayout->addWidget(m_pPrevButton);

    m_pPlayButton = new QPushButton(this);
    m_pPlayButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    m_pPlayButton->setToolTip(tr("Play"));
    buttonLayout->addWidget(m_pPlayButton);

    m_pStopButton = new QPushButton(this);
    m_pStopButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    m_pStopButton->setToolTip(tr("Stop"));
    buttonLayout->addWidget(m_pStopButton);

    m_pNextButton = new QPushButton(this);
    m_pNextButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
    m_pNextButton->setToolTip(tr("Next"));
    buttonLayout->addWidget(m_pNextButton);

    buttonLayout->addSpacing(20);

    // Volume
    auto* volumeIcon = new QLabel(this);
    volumeIcon->setPixmap(style()->standardIcon(QStyle::SP_MediaVolume).pixmap(16, 16));
    buttonLayout->addWidget(volumeIcon);

    m_pVolumeSlider = new QSlider(Qt::Horizontal, this);
    m_pVolumeSlider->setRange(0, 100);
    m_pVolumeSlider->setValue(80);
    m_pVolumeSlider->setMaximumWidth(100);
    buttonLayout->addWidget(m_pVolumeSlider);

    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
}

void PlayerPanel::setupConnections()
{
    // TODO: Connect buttons to audio service
}

// =============================================================================
// SELF-REGISTRATION
// =============================================================================

REGISTER_PANEL("player", "Player", true, PlayerPanel)
