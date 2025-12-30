/**
 ****************************************************************************************
 * @file   PlaylistPanel.cpp
 * @brief  PlaylistPanel implementation (Stub)
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 ****************************************************************************************
 */

#include "UI/panels/PlaylistPanel.hpp"
#include "services/PanelRegistry.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QStyle>
#include <QFileDialog>

// =============================================================================
// Construction
// =============================================================================

PlaylistPanel::PlaylistPanel(ServiceContainer& services, QWidget* parent)
    : PanelBase(services, QStringLiteral("playlist"), tr("Playlist"), parent)
{
    setupUI();
    setupConnections();
}

// =============================================================================
// IPanel Implementation
// =============================================================================

int PlaylistPanel::preferredArea() const
{
    return Qt::RightDockWidgetArea;
}

// =============================================================================
// Lifecycle
// =============================================================================

void PlaylistPanel::onActivate()
{
    // TODO: Refresh playlist from service
}

void PlaylistPanel::onDeactivate()
{
    // Nothing to do
}

void PlaylistPanel::saveState()
{
    PanelBase::saveState();
    // TODO: Save playlist to settings
}

void PlaylistPanel::restoreState()
{
    PanelBase::restoreState();
    // TODO: Restore playlist from settings
}

// =============================================================================
// Slots
// =============================================================================

void PlaylistPanel::onAddClicked()
{
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        tr("Add Audio Files"),
        QString(),
        tr("Audio Files (*.mp3 *.wav *.flac *.ogg *.m4a);;All Files (*)")
    );

    for (const QString& file : files)
    {
        m_pListWidget->addItem(file);
    }
}

void PlaylistPanel::onRemoveClicked()
{
    auto* item = m_pListWidget->currentItem();
    if (item != nullptr)
    {
        delete m_pListWidget->takeItem(m_pListWidget->row(item));
    }
}

void PlaylistPanel::onClearClicked()
{
    m_pListWidget->clear();
}

void PlaylistPanel::onItemDoubleClicked()
{
    // TODO: Play selected track via EventBus
}

void PlaylistPanel::onSearchChanged(const QString& text)
{
    for (int i = 0; i < m_pListWidget->count(); ++i)
    {
        auto* item = m_pListWidget->item(i);
        bool matches = text.isEmpty() || 
                       item->text().contains(text, Qt::CaseInsensitive);
        item->setHidden(!matches);
    }
}

// =============================================================================
// Private Methods
// =============================================================================

void PlaylistPanel::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    // Search
    m_pSearchEdit = new QLineEdit(this);
    m_pSearchEdit->setPlaceholderText(tr("Search..."));
    m_pSearchEdit->setClearButtonEnabled(true);
    mainLayout->addWidget(m_pSearchEdit);

    // List
    m_pListWidget = new QListWidget(this);
    m_pListWidget->setAlternatingRowColors(true);
    m_pListWidget->setDragDropMode(QAbstractItemView::InternalMove);
    mainLayout->addWidget(m_pListWidget, 1);

    // Buttons
    auto* buttonLayout = new QHBoxLayout();

    m_pAddButton = new QPushButton(this);
    m_pAddButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    m_pAddButton->setToolTip(tr("Add files"));
    buttonLayout->addWidget(m_pAddButton);

    m_pRemoveButton = new QPushButton(this);
    m_pRemoveButton->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    m_pRemoveButton->setToolTip(tr("Remove selected"));
    buttonLayout->addWidget(m_pRemoveButton);

    buttonLayout->addStretch();

    m_pClearButton = new QPushButton(this);
    m_pClearButton->setIcon(style()->standardIcon(QStyle::SP_DialogResetButton));
    m_pClearButton->setToolTip(tr("Clear playlist"));
    buttonLayout->addWidget(m_pClearButton);

    mainLayout->addLayout(buttonLayout);
}

void PlaylistPanel::setupConnections()
{
    connect(m_pAddButton, &QPushButton::clicked, 
            this, &PlaylistPanel::onAddClicked);
    connect(m_pRemoveButton, &QPushButton::clicked, 
            this, &PlaylistPanel::onRemoveClicked);
    connect(m_pClearButton, &QPushButton::clicked, 
            this, &PlaylistPanel::onClearClicked);
    connect(m_pListWidget, &QListWidget::itemDoubleClicked,
            this, &PlaylistPanel::onItemDoubleClicked);
    connect(m_pSearchEdit, &QLineEdit::textChanged,
            this, &PlaylistPanel::onSearchChanged);
}

// =============================================================================
// SELF-REGISTRATION
// =============================================================================

REGISTER_PANEL("playlist", "Playlist", true, PlaylistPanel)
