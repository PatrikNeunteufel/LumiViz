/**
 ****************************************************************************************
 * @file   PlaylistPanel.cpp
 * @brief  PlaylistPanel implementation with full playlist integration
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.0.0
 ****************************************************************************************
 */

#include "UI/panels/PlaylistPanel.hpp"
#include "services/ServiceContainer.hpp"
#include "services/IEventBus.hpp"
#include "audio/IPlaylist.hpp"
#include "audio/IAudioPlayer.hpp"
#include "audio/AudioEvents.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QStyle>
#include <QFileDialog>
#include <QFileInfo>

#include <BasicLogger.h>

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
    subscribeToEvents();
    refreshPlaylist();
    
    // Sync current playing index
    if (auto* player = services().tryResolve<IAudioPlayer>())
    {
        highlightCurrentTrack(player->playlistIndex());
    }
}

void PlaylistPanel::onDeactivate()
{
    unsubscribeFromEvents();
}

void PlaylistPanel::saveState()
{
    PanelBase::saveState();
    // Playlist is saved by the Playlist service itself
}

void PlaylistPanel::restoreState()
{
    PanelBase::restoreState();
    refreshPlaylist();
}

// =============================================================================
// Event Subscription
// =============================================================================

void PlaylistPanel::subscribeToEvents()
{
    auto* eventBus = services().tryResolve<IEventBus>();
    if (eventBus == nullptr)
    {
        BasicLogger::logWarning("PlaylistPanel: EventBus not available");
        return;
    }
    
    // Playlist content changed
    int id1 = eventBus->subscribe<PlaylistChangedEvent>(
        [this](const PlaylistChangedEvent& /*e*/) {
            onPlaylistChanged();
        });
    m_subscriptionIds.push_back(id1);
    
    // Current track index changed
    int id2 = eventBus->subscribe<PlaylistIndexChangedEvent>(
        [this](const PlaylistIndexChangedEvent& e) {
            onPlaylistIndexChanged(e.currentIndex, e.previousIndex);
        });
    m_subscriptionIds.push_back(id2);
    
    BasicLogger::logDebug("PlaylistPanel: Subscribed to playlist events");
}

void PlaylistPanel::unsubscribeFromEvents()
{
    auto* eventBus = services().tryResolve<IEventBus>();
    if (eventBus == nullptr)
    {
        return;
    }
    
    for (int id : m_subscriptionIds)
    {
        eventBus->unsubscribe(id);
    }
    m_subscriptionIds.clear();
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

    if (files.isEmpty())
    {
        return;
    }
    
    auto* playlist = services().tryResolve<IPlaylist>();
    if (playlist == nullptr)
    {
        // Fallback: Just add to UI (will be lost)
        for (const QString& file : files)
        {
            QFileInfo fi(file);
            m_pListWidget->addItem(fi.fileName());
        }
        return;
    }
    
    // Add to playlist service
    for (const QString& file : files)
    {
        playlist->addTrack(file);
    }
    
    BasicLogger::logInfo("PlaylistPanel: Added " + std::to_string(files.size()) + " files");
}

void PlaylistPanel::onRemoveClicked()
{
    int currentRow = m_pListWidget->currentRow();
    if (currentRow < 0)
    {
        return;
    }
    
    auto* playlist = services().tryResolve<IPlaylist>();
    if (playlist != nullptr)
    {
        playlist->removeTrack(currentRow);
    }
    else
    {
        // Fallback: Just remove from UI
        delete m_pListWidget->takeItem(currentRow);
    }
}

void PlaylistPanel::onClearClicked()
{
    auto* playlist = services().tryResolve<IPlaylist>();
    if (playlist != nullptr)
    {
        playlist->clear();
    }
    else
    {
        m_pListWidget->clear();
    }
}

void PlaylistPanel::onItemDoubleClicked(QListWidgetItem* item)
{
    if (item == nullptr)
    {
        return;
    }
    
    int index = m_pListWidget->row(item);
    
    // Play this track
    auto* player = services().tryResolve<IAudioPlayer>();
    if (player != nullptr)
    {
        player->playIndex(index);
    }
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

void PlaylistPanel::onSelectionChanged()
{
    // Enable/disable remove button based on selection
    bool hasSelection = m_pListWidget->currentRow() >= 0;
    m_pRemoveButton->setEnabled(hasSelection);
}

// =============================================================================
// Event Handlers
// =============================================================================

void PlaylistPanel::onPlaylistChanged()
{
    refreshPlaylist();
}

void PlaylistPanel::onPlaylistIndexChanged(int newIndex, int /*oldIndex*/)
{
    highlightCurrentTrack(newIndex);
}

// =============================================================================
// Playlist Sync
// =============================================================================

void PlaylistPanel::refreshPlaylist()
{
    m_pListWidget->clear();
    
    auto* playlist = services().tryResolve<IPlaylist>();
    if (playlist == nullptr)
    {
        return;
    }
    
    int count = playlist->count();
    for (int i = 0; i < count; ++i)
    {
        QString filePath = playlist->filePathAt(i);
        QFileInfo fi(filePath);
        
        auto* item = new QListWidgetItem(fi.fileName(), m_pListWidget);
        item->setData(Qt::UserRole, filePath);
        item->setToolTip(filePath);
    }
    
    // Restore highlight
    highlightCurrentTrack(m_currentPlayingIndex);
    
    BasicLogger::logDebug("PlaylistPanel: Refreshed playlist with " + 
                          std::to_string(count) + " tracks");
}

void PlaylistPanel::highlightCurrentTrack(int index)
{
    m_currentPlayingIndex = index;
    
    // Reset all items
    for (int i = 0; i < m_pListWidget->count(); ++i)
    {
        auto* item = m_pListWidget->item(i);
        QFont font = item->font();
        font.setBold(false);
        item->setFont(font);
        item->setBackground(Qt::transparent);
    }
    
    // Highlight current
    if (index >= 0 && index < m_pListWidget->count())
    {
        auto* item = m_pListWidget->item(index);
        QFont font = item->font();
        font.setBold(true);
        item->setFont(font);
        item->setBackground(QColor(60, 60, 80));
        
        // Scroll to make visible
        m_pListWidget->scrollToItem(item);
    }
}

// =============================================================================
// UI Setup
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
    m_pListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
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
    m_pRemoveButton->setEnabled(false);
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
    connect(m_pListWidget, &QListWidget::itemSelectionChanged,
            this, &PlaylistPanel::onSelectionChanged);
    connect(m_pSearchEdit, &QLineEdit::textChanged,
            this, &PlaylistPanel::onSearchChanged);
}

// =============================================================================
// SELF-REGISTRATION
// =============================================================================
// NOTE: Registration is now handled centrally in PanelAutoReg.cpp
// to avoid linker issues with static libraries (dead code elimination).
// The REGISTER_PANEL macro is no longer used here.
