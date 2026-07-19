/**
 ****************************************************************************************
 * @file   PlaylistPanel.cpp
 * @brief  PlaylistPanel implementation with full playlist integration
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.1.0
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
#include <QFile>
#include <QTextStream>

#include <BasicLogger.h>

#include <algorithm>
#include <functional>

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
    
    // Sync with player state
    if (auto* player = services().tryResolve<IAudioPlayer>())
    {
        highlightCurrentTrack(player->playlistIndex());
        
        // Sync shuffle/loop state
        m_shuffleEnabled = player->shuffle();
        m_loopEnabled = (player->repeatMode() == IAudioPlayer::RepeatMode::All);
        updateShuffleButton(m_shuffleEnabled);
        updateLoopButton(m_loopEnabled);
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
    m_eventSubscriptions.push_back(eventBus->subscribeScoped<PlaylistChangedEvent>(
        [this](const PlaylistChangedEvent& /*e*/) {
            onPlaylistChanged();
        }));

    // Current track index changed
    m_eventSubscriptions.push_back(eventBus->subscribeScoped<PlaylistIndexChangedEvent>(
        [this](const PlaylistIndexChangedEvent& e) {
            onPlaylistIndexChanged(e.currentIndex, e.previousIndex);
        }));

    // Playback mode (shuffle/loop) changed
    m_eventSubscriptions.push_back(eventBus->subscribeScoped<PlaybackModeChangedEvent>(
        [this](const PlaybackModeChangedEvent& e) {
            onPlaybackModeChanged(e.shuffle, e.repeatMode);
        }));

    BasicLogger::logDebug("PlaylistPanel: Subscribed to playlist events");
}

void PlaylistPanel::unsubscribeFromEvents()
{
    // RAII handles unsubscribe on destruction; clearing releases them now.
    m_eventSubscriptions.clear();
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
    QList<QListWidgetItem*> selectedItems = m_pListWidget->selectedItems();
    if (selectedItems.isEmpty())
    {
        return;
    }
    
    // Get indices and sort descending (remove from back to front to keep indices valid)
    QList<int> indices;
    for (auto* item : selectedItems)
    {
        indices.append(m_pListWidget->row(item));
    }
    std::sort(indices.begin(), indices.end(), std::greater<int>());
    
    auto* playlist = services().tryResolve<IPlaylist>();
    if (playlist != nullptr)
    {
        for (int idx : indices)
        {
            playlist->removeTrack(idx);
        }
    }
    else
    {
        // Fallback: Just remove from UI
        for (int idx : indices)
        {
            delete m_pListWidget->takeItem(idx);
        }
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
    bool hasSelection = !m_pListWidget->selectedItems().isEmpty();
    m_pRemoveButton->setEnabled(hasSelection);
}

void PlaylistPanel::onShuffleClicked()
{
    auto* player = services().tryResolve<IAudioPlayer>();
    if (player == nullptr)
    {
        return;
    }
    
    m_shuffleEnabled = !m_shuffleEnabled;
    player->setShuffle(m_shuffleEnabled);
    updateShuffleButton(m_shuffleEnabled);
    
    BasicLogger::logDebug("PlaylistPanel: Shuffle " + 
                          std::string(m_shuffleEnabled ? "enabled" : "disabled"));
}

void PlaylistPanel::onLoopClicked()
{
    auto* player = services().tryResolve<IAudioPlayer>();
    if (player == nullptr)
    {
        return;
    }
    
    m_loopEnabled = !m_loopEnabled;
    player->setRepeatMode(m_loopEnabled ? IAudioPlayer::RepeatMode::All 
                                        : IAudioPlayer::RepeatMode::None);
    updateLoopButton(m_loopEnabled);
    
    BasicLogger::logDebug("PlaylistPanel: Loop " + 
                          std::string(m_loopEnabled ? "enabled" : "disabled"));
}

void PlaylistPanel::onSaveClicked()
{
    auto* playlist = services().tryResolve<IPlaylist>();
    if (playlist == nullptr || playlist->isEmpty())
    {
        return;
    }
    
    QString filePath = QFileDialog::getSaveFileName(
        this,
        tr("Save Playlist"),
        QString(),
        tr("M3U Playlist (*.m3u);;All Files (*)")
    );
    
    if (filePath.isEmpty())
    {
        return;
    }
    
    // Ensure .m3u extension
    if (!filePath.endsWith(".m3u", Qt::CaseInsensitive))
    {
        filePath += ".m3u";
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        BasicLogger::logError("Failed to save playlist: " + filePath.toStdString());
        return;
    }
    
    QTextStream out(&file);
    out << "#EXTM3U\n";
    
    for (int i = 0; i < playlist->count(); ++i)
    {
        QString trackPath = playlist->filePathAt(i);
        out << trackPath << "\n";
    }
    
    file.close();
    BasicLogger::logInfo("Playlist saved: " + filePath.toStdString() + 
                         " (" + std::to_string(playlist->count()) + " tracks)");
}

void PlaylistPanel::onLoadClicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Load Playlist"),
        QString(),
        tr("Playlist Files (*.m3u *.m3u8 *.pls);;All Files (*)")
    );
    
    if (filePath.isEmpty())
    {
        return;
    }
    
    auto* playlist = services().tryResolve<IPlaylist>();
    if (playlist == nullptr)
    {
        return;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        BasicLogger::logError("Failed to load playlist: " + filePath.toStdString());
        return;
    }
    
    // Clear current playlist
    playlist->clear();
    
    QTextStream in(&file);
    QFileInfo playlistFileInfo(filePath);
    QString playlistDir = playlistFileInfo.absolutePath();
    int addedCount = 0;
    
    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        
        // Skip empty lines and comments
        if (line.isEmpty() || line.startsWith('#'))
        {
            continue;
        }
        
        // Handle relative paths
        QString trackPath = line;
        if (!QFileInfo(trackPath).isAbsolute())
        {
            trackPath = playlistDir + "/" + line;
        }
        
        // Check if file exists
        if (QFile::exists(trackPath))
        {
            playlist->addTrack(trackPath);
            addedCount++;
        }
        else
        {
            BasicLogger::logWarning("Playlist track not found: " + trackPath.toStdString());
        }
    }
    
    file.close();
    BasicLogger::logInfo("Playlist loaded: " + filePath.toStdString() + 
                         " (" + std::to_string(addedCount) + " tracks)");
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

void PlaylistPanel::onPlaybackModeChanged(bool shuffle, int repeatMode)
{
    m_shuffleEnabled = shuffle;
    m_loopEnabled = (repeatMode == 2);  // 2 = RepeatMode::All (playlist loop)
    updateShuffleButton(shuffle);
    updateLoopButton(m_loopEnabled);
}

void PlaylistPanel::updateShuffleButton(bool enabled)
{
    if (m_pShuffleButton == nullptr)
    {
        return;
    }
    
    m_pShuffleButton->setChecked(enabled);
    
    // Visual feedback - change style when active
    if (enabled)
    {
        m_pShuffleButton->setStyleSheet(
            "QPushButton { background-color: #4a6fa5; border: 1px solid #6a8fc5; }");
    }
    else
    {
        m_pShuffleButton->setStyleSheet("");
    }
}

void PlaylistPanel::updateLoopButton(bool enabled)
{
    if (m_pLoopButton == nullptr)
    {
        return;
    }
    
    m_pLoopButton->setChecked(enabled);
    
    // Visual feedback - change style when active
    if (enabled)
    {
        m_pLoopButton->setStyleSheet(
            "QPushButton { background-color: #4a6fa5; border: 1px solid #6a8fc5; }");
    }
    else
    {
        m_pLoopButton->setStyleSheet("");
    }
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
    
    // Reset all items to normal font
    for (int i = 0; i < m_pListWidget->count(); ++i)
    {
        auto* item = m_pListWidget->item(i);
        QFont font = item->font();
        font.setBold(false);
        item->setFont(font);
    }
    
    // Highlight current with bold font only (no background)
    if (index >= 0 && index < m_pListWidget->count())
    {
        auto* item = m_pListWidget->item(index);
        QFont font = item->font();
        font.setBold(true);
        item->setFont(font);
        
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
    m_pListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mainLayout->addWidget(m_pListWidget, 1);

    // All buttons in one row
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

    m_pClearButton = new QPushButton(this);
    m_pClearButton->setIcon(style()->standardIcon(QStyle::SP_DialogResetButton));
    m_pClearButton->setToolTip(tr("Clear playlist"));
    buttonLayout->addWidget(m_pClearButton);

    buttonLayout->addSpacing(8);

    // Save/Load buttons
    m_pSaveButton = new QPushButton(this);
    m_pSaveButton->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    m_pSaveButton->setToolTip(tr("Save playlist"));
    buttonLayout->addWidget(m_pSaveButton);

    m_pLoadButton = new QPushButton(this);
    m_pLoadButton->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    m_pLoadButton->setToolTip(tr("Load playlist"));
    buttonLayout->addWidget(m_pLoadButton);

    buttonLayout->addStretch();

    // Shuffle button with icon
    m_pShuffleButton = new QPushButton(this);
    m_pShuffleButton->setText(QStringLiteral("🔀"));  // Unicode shuffle symbol
    m_pShuffleButton->setCheckable(true);
    m_pShuffleButton->setToolTip(tr("Shuffle mode - play random next track"));
    m_pShuffleButton->setFixedWidth(32);
    buttonLayout->addWidget(m_pShuffleButton);

    // Loop button with icon
    m_pLoopButton = new QPushButton(this);
    m_pLoopButton->setText(QStringLiteral("🔁"));  // Unicode repeat symbol
    m_pLoopButton->setCheckable(true);
    m_pLoopButton->setToolTip(tr("Loop playlist - restart from beginning after last track"));
    m_pLoopButton->setFixedWidth(32);
    buttonLayout->addWidget(m_pLoopButton);

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
    connect(m_pSaveButton, &QPushButton::clicked,
            this, &PlaylistPanel::onSaveClicked);
    connect(m_pLoadButton, &QPushButton::clicked,
            this, &PlaylistPanel::onLoadClicked);
    connect(m_pShuffleButton, &QPushButton::clicked,
            this, &PlaylistPanel::onShuffleClicked);
    connect(m_pLoopButton, &QPushButton::clicked,
            this, &PlaylistPanel::onLoopClicked);
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
