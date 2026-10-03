/**
 ****************************************************************************************
 * @file   AudioBrowserPanel.cpp
 * @brief  AudioBrowserPanel implementation (S84)
 ****************************************************************************************
 */

#include "UI/panels/AudioBrowserPanel.hpp"
#include "UI/widgets/AudioDateien.hpp"
#include "UI/widgets/DateiListe.hpp"
#include "audio/AlbumCover.hpp"
#include "audio/AudioEvents.hpp"
#include "audio/IAudioPlayer.hpp"
#include "services/IEventBus.hpp"
#include "services/ServiceContainer.hpp"

#include <QIcon>
#include <QImage>
#include <QPixmap>

#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSettings>
#include <QStandardPaths>
#include <QStyle>
#include <QVBoxLayout>

#include <BasicLogger.h>

AudioBrowserPanel::AudioBrowserPanel(ServiceContainer& services, QWidget* parent)
    : PanelBase(services, QStringLiteral("audio_browser"), tr("Audio Browser"), parent)
{
    QSettings settings;
    settings.beginGroup(settingsPrefix());
    QString start = settings.value(QStringLiteral("lastDir")).toString();
    settings.endGroup();
    if (start.isEmpty() || !QDir(start).exists())
        start = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    if (start.isEmpty() || !QDir(start).exists()) start = QDir::homePath();
    m_dir = QDir(start);

    m_coverLader = std::make_unique<lumi::audio::CoverLader>();
    connect(m_coverLader.get(), &lumi::audio::CoverLader::geladen, this,
            [this](int auftrag, const QString& pfad, const QImage& bild) {
                if (auftrag != m_coverAuftrag) return;  // alter Ordner
                if (QListWidgetItem* item = m_eintragNachPfad.value(pfad))
                    item->setIcon(QIcon(QPixmap::fromImage(bild)));
            });

    // S93 (Wunsch Patrik, wie im Effect Browser): der laufende Titel wird
    // gemerkt und markiert, der Browser wechselt aber nie von selbst den
    // Ordner — dafuer gibt es den Knopf ◎.
    if (auto* player = services.tryResolve<IAudioPlayer>())
        if (player->hasTrack()) m_aktuell = player->currentTrack().filePath;

    setupUI();
    refresh();

    if (auto* bus = eventBus())
        m_titelSubscription = bus->subscribeScoped<TrackChangedEvent>(
            [this](const TrackChangedEvent& e) { merkeAktuellenTitel(e.track.filePath); });
}

// Hier, nicht im Header: CoverLader ist dort nur vorwaerts deklariert.
AudioBrowserPanel::~AudioBrowserPanel() = default;

int AudioBrowserPanel::preferredArea() const
{
    return Qt::RightDockWidgetArea;
}

void AudioBrowserPanel::onActivate()
{
    // Neu listen: die Engine kennt ihre Plugin-Endungen erst nach dem Start.
    refresh();
}

// =============================================================================
// Navigation
// =============================================================================

void AudioBrowserPanel::navigateTo(const QString& dirPath)
{
    QDir target(dirPath);
    if (!target.exists())
    {
        setStatus(tr("Folder not found: %1").arg(dirPath));
        return;
    }
    target.makeAbsolute();
    m_dir = target;
    // Sofort merken — nicht erst in saveState(), das nicht jeder Weg erreicht.
    QSettings settings;
    settings.beginGroup(settingsPrefix());
    settings.setValue(QStringLiteral("lastDir"), m_dir.absolutePath());
    settings.endGroup();
    refresh();
}

void AudioBrowserPanel::refresh()
{
    if (m_pListWidget == nullptr) return;
    m_eintragNachPfad.clear();
    m_pListWidget->clear();
    m_pPathEdit->setText(m_dir.absolutePath());

    const QIcon dirIcon = style()->standardIcon(QStyle::SP_DirIcon);
    const QIcon fileIcon = style()->standardIcon(QStyle::SP_FileIcon);
    const QIcon upIcon = style()->standardIcon(QStyle::SP_FileDialogToParent);
    const auto nichtZiehbar = [](QListWidgetItem* item) {
        item->setFlags(item->flags() & ~Qt::ItemIsDragEnabled);
    };

    if (!m_dir.isRoot())
    {
        auto* up = new QListWidgetItem(upIcon, QStringLiteral(".."), m_pListWidget);
        up->setData(Qt::UserRole, m_dir.absolutePath());
        up->setData(Qt::UserRole + 1, Type_Up);
        nichtZiehbar(up);
    }

    const QFileInfoList dirs =
        m_dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase);
    for (const QFileInfo& fi : dirs)
    {
        auto* item = new QListWidgetItem(dirIcon, fi.fileName(), m_pListWidget);
        item->setData(Qt::UserRole, fi.absoluteFilePath());
        item->setData(Qt::UserRole + 1, Type_Dir);
        item->setToolTip(fi.absoluteFilePath());
        nichtZiehbar(item);
    }

    QStringList filter;
    for (const QString& e : lumi::ui::audioEndungen(services()))
        filter << QStringLiteral("*.") + e;
    const QFileInfoList files =
        m_dir.entryInfoList(filter, QDir::Files, QDir::Name | QDir::IgnoreCase);
    QStringList coverPfade;
    for (const QFileInfo& fi : files)
    {
        const QIcon typ = lumi::ui::audioTypIcon(fi.suffix());
        auto* item =
            new QListWidgetItem(typ.isNull() ? fileIcon : typ, fi.fileName(), m_pListWidget);
        item->setData(Qt::UserRole, fi.absoluteFilePath());
        item->setData(Qt::UserRole + 1, Type_Audio);
        item->setToolTip(fi.absoluteFilePath());
        m_eintragNachPfad.insert(fi.absoluteFilePath(), item);
        coverPfade.append(fi.absoluteFilePath());
    }

    // Cover im Hintergrund nachladen, von oben nach unten; ein neuer Auftrag
    // bricht den alten ab. Doppelte Kante: scharf auf HiDPI-Schirmen.
    m_coverAuftrag = m_coverLader->lade(coverPfade, kIconKante * 2);

    onSearchChanged(m_pSearchEdit->text());
    // Den laufenden Titel markieren, sooft die Liste neu entsteht (S93).
    markiereAktuellenTitel();
    setStatus(tr("%1 audio file(s) · %2 folder(s)").arg(files.size()).arg(dirs.size()));
}

void AudioBrowserPanel::merkeAktuellenTitel(const QString& pfad)
{
    m_aktuell = pfad;
    aktualisiereSprungKnopf();
    if (m_pListWidget == nullptr) return;
    if (m_aktuell.isEmpty())
        m_pListWidget->setCurrentRow(-1);
    else
        markiereAktuellenTitel();
}

void AudioBrowserPanel::markiereAktuellenTitel()
{
    if (m_aktuell.isEmpty() || m_pListWidget == nullptr) return;
    const QFileInfo gesucht(m_aktuell);
    for (int row = 0; row < m_pListWidget->count(); ++row)
    {
        QListWidgetItem* item = m_pListWidget->item(row);
        if (item->data(Qt::UserRole + 1).toInt() != Type_Audio) continue;
        if (QFileInfo(item->data(Qt::UserRole).toString()) != gesucht) continue;
        m_pListWidget->setCurrentRow(row);
        m_pListWidget->scrollToItem(item);
        return;
    }
}

bool AudioBrowserPanel::springeZumAktuellenTitel()
{
    if (m_aktuell.isEmpty() || !QFileInfo::exists(m_aktuell))
    {
        setStatus(tr("No current song to jump to"));
        return false;
    }
    const auto steht = [this] {
        const QListWidgetItem* item = m_pListWidget->currentItem();
        return item != nullptr && !item->isHidden() &&
               QFileInfo(item->data(Qt::UserRole).toString()) == QFileInfo(m_aktuell);
    };
    m_pListWidget->setCurrentRow(-1);
    navigateTo(QFileInfo(m_aktuell).absolutePath());
    if (!steht())
    {
        m_pSearchEdit->clear();  // ein Suchtext verdeckt ihn
        markiereAktuellenTitel();
    }
    return steht();
}

void AudioBrowserPanel::aktualisiereSprungKnopf()
{
    if (m_pJumpButton == nullptr) return;
    const bool da = !m_aktuell.isEmpty();
    m_pJumpButton->setEnabled(da);
    m_pJumpButton->setToolTip(
        da ? tr("Jump to the current song: show its folder and mark it\n%1")
                 .arg(QDir::toNativeSeparators(m_aktuell))
           : tr("Jump to the current song (none is loaded)"));
}

// =============================================================================
// Slots
// =============================================================================

void AudioBrowserPanel::onItemDoubleClicked(QListWidgetItem* item)
{
    if (item == nullptr) return;
    const QString path = item->data(Qt::UserRole).toString();
    switch (item->data(Qt::UserRole + 1).toInt())
    {
        case Type_Up: onUpClicked(); return;
        case Type_Dir: navigateTo(path); return;
        case Type_Audio:
            // Entscheid 2a: an die Playlist haengen und abspielen (steht sie
            // schon drin: dorthin springen).
            if (lumi::ui::spieleDateien(services(), {path}))
                setStatus(tr("▶ %1").arg(QFileInfo(path).fileName()));
            else
                setStatus(tr("✗ %1 — could not be played").arg(QFileInfo(path).fileName()));
            return;
        default: return;
    }
}

void AudioBrowserPanel::onUpClicked()
{
    QDir up = m_dir;
    if (up.cdUp()) navigateTo(up.absolutePath());
}

void AudioBrowserPanel::onBrowseClicked()
{
    const QString dir = QFileDialog::getExistingDirectory(this, tr("Choose music folder"),
                                                          m_dir.absolutePath());
    if (!dir.isEmpty()) navigateTo(dir);
}

void AudioBrowserPanel::onPathEntered()
{
    navigateTo(m_pPathEdit->text().trimmed());
}

void AudioBrowserPanel::onSearchChanged(const QString& text)
{
    for (int i = 0; i < m_pListWidget->count(); ++i)
    {
        auto* item = m_pListWidget->item(i);
        const bool isUp = item->data(Qt::UserRole + 1).toInt() == Type_Up;
        item->setHidden(!(isUp || text.isEmpty() ||
                          item->text().contains(text, Qt::CaseInsensitive)));
    }
}

void AudioBrowserPanel::setStatus(const QString& text)
{
    if (m_pStatusLabel != nullptr) m_pStatusLabel->setText(text);
}

// =============================================================================
// UI Setup
// =============================================================================

void AudioBrowserPanel::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    auto* navLayout = new QHBoxLayout();
    m_pUpButton = new QPushButton(this);
    m_pUpButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogToParent));
    m_pUpButton->setToolTip(tr("Up one folder"));
    m_pUpButton->setFixedWidth(32);
    navLayout->addWidget(m_pUpButton);

    m_pPathEdit = new QLineEdit(this);
    m_pPathEdit->setPlaceholderText(tr("Folder path…"));
    navLayout->addWidget(m_pPathEdit, 1);

    m_pBrowseButton = new QPushButton(this);
    m_pBrowseButton->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
    m_pBrowseButton->setToolTip(tr("Choose folder…"));
    m_pBrowseButton->setFixedWidth(32);
    navLayout->addWidget(m_pBrowseButton);

    // S93: wie im Effect Browser — in den Ordner des laufenden Titels
    m_pJumpButton = new QPushButton(QStringLiteral("◎"), this);
    m_pJumpButton->setObjectName(QStringLiteral("audioBrowserJump"));
    m_pJumpButton->setFixedWidth(32);
    navLayout->addWidget(m_pJumpButton);
    aktualisiereSprungKnopf();
    mainLayout->addLayout(navLayout);

    m_pSearchEdit = new QLineEdit(this);
    m_pSearchEdit->setPlaceholderText(tr("Filter…"));
    m_pSearchEdit->setClearButtonEnabled(true);
    mainLayout->addWidget(m_pSearchEdit);

    // Ziehen: in die Playlist, auf Player oder Visualisierung (S84).
    auto* liste = new lumi::ui::DateiListe(this);
    liste->pfadZumZiehen = [](const QListWidgetItem* item) {
        return item->data(Qt::UserRole + 1).toInt() == Type_Audio
                   ? item->data(Qt::UserRole).toString()
                   : QString();
    };
    m_pListWidget = liste;
    m_pListWidget->setObjectName(QStringLiteral("audioBrowserListe"));
    m_pListWidget->setAlternatingRowColors(true);
    m_pListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_pListWidget->setIconSize(QSize(kIconKante, kIconKante));
    m_pListWidget->setToolTip(
        tr("Double-click: add to the playlist and play. Drag: into the playlist, "
           "onto the player or the visualization."));
    mainLayout->addWidget(m_pListWidget, 1);

    m_pStatusLabel = new QLabel(this);
    m_pStatusLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_pStatusLabel->setWordWrap(true);
    mainLayout->addWidget(m_pStatusLabel);

    connect(m_pListWidget, &QListWidget::itemDoubleClicked, this,
            &AudioBrowserPanel::onItemDoubleClicked);
    connect(m_pUpButton, &QPushButton::clicked, this, &AudioBrowserPanel::onUpClicked);
    connect(m_pBrowseButton, &QPushButton::clicked, this, &AudioBrowserPanel::onBrowseClicked);
    connect(m_pJumpButton, &QPushButton::clicked, this,
            [this] { (void)springeZumAktuellenTitel(); });
    connect(m_pPathEdit, &QLineEdit::returnPressed, this, &AudioBrowserPanel::onPathEntered);
    connect(m_pSearchEdit, &QLineEdit::textChanged, this, &AudioBrowserPanel::onSearchChanged);
}
