/**
 ****************************************************************************************
 * @file   ImportBrowserPanel.cpp
 * @brief  ImportBrowserPanel implementation
 *
 * @author Patrik Neunteufel
 * @date   July 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#include "UI/panels/ImportBrowserPanel.hpp"
#include "services/ServiceContainer.hpp"
#include "services/IEventBus.hpp"
#include "services/events/UIEvents.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QStyle>
#include <QFileDialog>
#include <QFileInfo>
#include <QSettings>

#include <BasicLogger.h>

// =============================================================================
// Construction
// =============================================================================

ImportBrowserPanel::ImportBrowserPanel(ServiceContainer& services, QWidget* parent)
    : PanelBase(services, QStringLiteral("import_browser"), tr("Import Browser"), parent)
{
    // Restore last folder + filter before the UI reads them (settingsPrefix() is
    // valid once the base constructor has run).
    QSettings settings;
    settings.beginGroup(settingsPrefix());
    const QString lastDir = settings.value(QStringLiteral("lastDir")).toString();
    m_filter = static_cast<Filter>(
        settings.value(QStringLiteral("filter"), static_cast<int>(Filter::Both)).toInt());
    settings.endGroup();

    QString start = lastDir;
    if (start.isEmpty() || !QDir(start).exists())
    {
        start = QDir::homePath();
    }
    m_dir = QDir(start);

    setupUI();
    setupConnections();
}

// =============================================================================
// IPanel Implementation
// =============================================================================

int ImportBrowserPanel::preferredArea() const
{
    return Qt::RightDockWidgetArea;
}

void ImportBrowserPanel::onActivate()
{
    subscribeToEvents();
    refresh();
}

void ImportBrowserPanel::onDeactivate()
{
    // RAII handles unsubscribe on destruction; clearing releases them now.
    m_eventSubscriptions.clear();
}

void ImportBrowserPanel::saveState()
{
    PanelBase::saveState();

    QSettings settings;
    settings.beginGroup(settingsPrefix());
    settings.setValue(QStringLiteral("lastDir"), m_dir.absolutePath());
    settings.setValue(QStringLiteral("filter"), static_cast<int>(m_filter));
    settings.endGroup();
}

void ImportBrowserPanel::restoreState()
{
    PanelBase::restoreState();
    refresh();
}

// =============================================================================
// Event Subscription
// =============================================================================

void ImportBrowserPanel::subscribeToEvents()
{
    auto* bus = eventBus();
    if (bus == nullptr)
    {
        BasicLogger::logWarning("ImportBrowserPanel: EventBus not available");
        return;
    }

    m_eventSubscriptions.push_back(bus->subscribeScoped<AvsImportResultEvent>(
        [this](const AvsImportResultEvent& e) {
            onImportResult(e.path, e.ok, e.noteCount);
        }));
}

// =============================================================================
// Navigation
// =============================================================================

void ImportBrowserPanel::navigateTo(const QString& dirPath)
{
    QDir target(dirPath);
    if (!target.exists())
    {
        setStatus(tr("Folder not found: %1").arg(dirPath));
        return;
    }
    target.makeAbsolute();
    m_dir = target;
    refresh();
}

QStringList ImportBrowserPanel::currentNameFilters() const
{
    switch (m_filter)
    {
        case Filter::Avs:  return {QStringLiteral("*.avs")};
        case Filter::Milk: return {QStringLiteral("*.milk")};
        case Filter::Both: break;
    }
    return {QStringLiteral("*.avs"), QStringLiteral("*.milk")};
}

void ImportBrowserPanel::refresh()
{
    if (m_pListWidget == nullptr) return;

    m_pListWidget->clear();
    m_pPathEdit->setText(m_dir.absolutePath());

    const QIcon dirIcon = style()->standardIcon(QStyle::SP_DirIcon);
    const QIcon fileIcon = style()->standardIcon(QStyle::SP_FileIcon);
    const QIcon upIcon = style()->standardIcon(QStyle::SP_FileDialogToParent);

    // Parent entry (unless at a filesystem root).
    if (!m_dir.isRoot())
    {
        auto* up = new QListWidgetItem(upIcon, QStringLiteral(".."), m_pListWidget);
        up->setData(Qt::UserRole, m_dir.absolutePath());
        up->setData(Qt::UserRole + 1, Type_Up);
    }

    // Sub-folders first (always shown, regardless of the preset filter).
    const QFileInfoList dirs = m_dir.entryInfoList(
        QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase);
    for (const QFileInfo& fi : dirs)
    {
        auto* item = new QListWidgetItem(dirIcon, fi.fileName(), m_pListWidget);
        item->setData(Qt::UserRole, fi.absoluteFilePath());
        item->setData(Qt::UserRole + 1, Type_Dir);
        item->setToolTip(fi.absoluteFilePath());
    }

    // Matching preset files.
    const QFileInfoList files = m_dir.entryInfoList(
        currentNameFilters(), QDir::Files, QDir::Name | QDir::IgnoreCase);
    int presetCount = 0;
    for (const QFileInfo& fi : files)
    {
        const bool isMilk = fi.suffix().compare(QStringLiteral("milk"),
                                                Qt::CaseInsensitive) == 0;
        auto* item = new QListWidgetItem(fileIcon, fi.fileName(), m_pListWidget);
        item->setData(Qt::UserRole, fi.absoluteFilePath());
        item->setData(Qt::UserRole + 1, isMilk ? Type_Milk : Type_Avs);
        item->setToolTip(fi.absoluteFilePath());
        ++presetCount;
    }

    // Re-apply the current search text to the freshly built list.
    onSearchChanged(m_pSearchEdit->text());

    setStatus(tr("%1 preset(s) · %2 folder(s)").arg(presetCount).arg(dirs.size()));
}

// =============================================================================
// Slots
// =============================================================================

void ImportBrowserPanel::onItemDoubleClicked(QListWidgetItem* item)
{
    if (item == nullptr) return;

    const int type = item->data(Qt::UserRole + 1).toInt();
    const QString path = item->data(Qt::UserRole).toString();

    switch (type)
    {
        case Type_Up:
            onUpClicked();
            return;

        case Type_Dir:
            navigateTo(path);
            return;

        case Type_Avs:
            if (auto* bus = eventBus())
            {
                bus->publish(ImportAvsPresetEvent{path.toStdString()});
            }
            return;

        case Type_Milk:
            if (auto* bus = eventBus())
            {
                bus->publish(ImportMilkPresetEvent{path.toStdString()});
            }
            return;

        default:
            return;
    }
}

void ImportBrowserPanel::onUpClicked()
{
    QDir up = m_dir;
    if (up.cdUp())
    {
        navigateTo(up.absolutePath());
    }
}

void ImportBrowserPanel::onBrowseClicked()
{
    const QString dir = QFileDialog::getExistingDirectory(
        this, tr("Choose preset folder"), m_dir.absolutePath());
    if (!dir.isEmpty())
    {
        navigateTo(dir);
    }
}

void ImportBrowserPanel::onPathEntered()
{
    navigateTo(m_pPathEdit->text().trimmed());
}

void ImportBrowserPanel::onFilterChanged(int index)
{
    m_filter = static_cast<Filter>(index);
    refresh();
}

void ImportBrowserPanel::onSearchChanged(const QString& text)
{
    for (int i = 0; i < m_pListWidget->count(); ++i)
    {
        auto* item = m_pListWidget->item(i);
        // The ".." entry stays visible so navigation is never filtered away.
        const bool isUp = item->data(Qt::UserRole + 1).toInt() == Type_Up;
        const bool matches = isUp || text.isEmpty() ||
                             item->text().contains(text, Qt::CaseInsensitive);
        item->setHidden(!matches);
    }
}

// =============================================================================
// Import feedback
// =============================================================================

void ImportBrowserPanel::onImportResult(const std::string& path, bool ok, int noteCount)
{
    const QFileInfo fi(QString::fromStdString(path));
    if (!ok)
    {
        setStatus(tr("✗ %1 — not a valid AVS preset").arg(fi.fileName()));
    }
    else if (noteCount > 0)
    {
        setStatus(tr("⚠ %1 — imported with %2 note(s)").arg(fi.fileName()).arg(noteCount));
    }
    else
    {
        setStatus(tr("✓ %1 — imported").arg(fi.fileName()));
    }
}

void ImportBrowserPanel::setStatus(const QString& text)
{
    if (m_pStatusLabel != nullptr)
    {
        m_pStatusLabel->setText(text);
    }
}

// =============================================================================
// UI Setup
// =============================================================================

void ImportBrowserPanel::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    // Navigation row: Up · path · Browse
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

    mainLayout->addLayout(navLayout);

    // Filter row: kind combo · search
    auto* filterLayout = new QHBoxLayout();

    m_pFilterCombo = new QComboBox(this);
    m_pFilterCombo->addItem(tr("AVS (*.avs)"));       // Filter::Avs
    m_pFilterCombo->addItem(tr("MilkDrop (*.milk)")); // Filter::Milk
    m_pFilterCombo->addItem(tr("Both"));              // Filter::Both
    m_pFilterCombo->setCurrentIndex(static_cast<int>(m_filter));
    m_pFilterCombo->setToolTip(tr("Which preset types to list"));
    filterLayout->addWidget(m_pFilterCombo);

    m_pSearchEdit = new QLineEdit(this);
    m_pSearchEdit->setPlaceholderText(tr("Filter…"));
    m_pSearchEdit->setClearButtonEnabled(true);
    filterLayout->addWidget(m_pSearchEdit, 1);

    mainLayout->addLayout(filterLayout);

    // Entry list
    m_pListWidget = new QListWidget(this);
    m_pListWidget->setAlternatingRowColors(true);
    m_pListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    mainLayout->addWidget(m_pListWidget, 1);

    // Status line
    m_pStatusLabel = new QLabel(this);
    m_pStatusLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_pStatusLabel->setWordWrap(true);
    mainLayout->addWidget(m_pStatusLabel);
}

void ImportBrowserPanel::setupConnections()
{
    connect(m_pListWidget, &QListWidget::itemDoubleClicked,
            this, &ImportBrowserPanel::onItemDoubleClicked);
    connect(m_pUpButton, &QPushButton::clicked,
            this, &ImportBrowserPanel::onUpClicked);
    connect(m_pBrowseButton, &QPushButton::clicked,
            this, &ImportBrowserPanel::onBrowseClicked);
    connect(m_pPathEdit, &QLineEdit::returnPressed,
            this, &ImportBrowserPanel::onPathEntered);
    connect(m_pFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ImportBrowserPanel::onFilterChanged);
    connect(m_pSearchEdit, &QLineEdit::textChanged,
            this, &ImportBrowserPanel::onSearchChanged);
}
