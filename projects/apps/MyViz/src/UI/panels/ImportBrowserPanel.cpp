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
#include "UI/widgets/PresetTypeIcons.hpp"
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
#include <QCoreApplication>
#include <QTimer>

#include <BasicLogger.h>

namespace
{
/// Vorgabe-Startordner: das Verzeichnis der Exe (S73).
///
/// Dort liegt seit S73 der mitgelieferte Ordner `presets/` (Root-CMakeLists,
/// POST_BUILD) — wer zuruecksetzt oder die App zum ersten Mal startet, sieht
/// also sofort etwas Ladbares. Vorher stand hier `QDir::homePath()`, und der
/// Browser oeffnete im Benutzerordner, wo garantiert kein Preset liegt.
QString defaultStartDir()
{
    return QCoreApplication::applicationDirPath();
}

/// Preset fuer den allerersten Start (S73) — eines der mitgelieferten.
///
/// Relativ zur Exe, NICHT absolut: der Ordner `presets/` wird bei jedem Build
/// neben die jeweilige Exe kopiert (Root-CMakeLists), es gibt also keinen
/// festen Pfad, der ueber alle Build-Konfigurationen und Rechner stimmt.
/// Fehlt die Datei (eigener Build ohne asset/presets), bleibt der Start leer —
/// das ist kein Fehlerfall.
QString defaultPresetPath()
{
    return QCoreApplication::applicationDirPath() +
           QStringLiteral("/presets/avs/EyeCandy2/02_flowers.avs");
}
}  // namespace

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
        settings.value(QStringLiteral("filter"), static_cast<int>(Filter::All)).toInt());
    // Zuletzt geladenes Preset (S73) — MUSS innerhalb der Gruppe gelesen
    // werden, sonst kommt es aus der Wurzel und ist immer leer.
    m_lastPreset = settings.value(QStringLiteral("lastPreset")).toString();
    settings.endGroup();

    // Beim allerersten Start tritt das mitgelieferte Preset an seine Stelle;
    // existiert weder das eine noch das andere, bleibt es leer und der Start
    // laeuft wie vorher.
    if (m_lastPreset.isEmpty()) m_lastPreset = defaultPresetPath();
    if (!QFileInfo::exists(m_lastPreset)) m_lastPreset.clear();

    QString start = lastDir;
    // Der Ordner des Presets gewinnt: sonst zeigte der Browser einen anderen
    // Ordner als das, was gerade laeuft.
    if (!m_lastPreset.isEmpty())
    {
        start = QFileInfo(m_lastPreset).absolutePath();
    }
    if (start.isEmpty() || !QDir(start).exists())
    {
        start = defaultStartDir();
    }
    m_dir = QDir(start);

    setupUI();
    setupConnections();

    // Liste SOFORT fuellen (S73), nicht erst in onActivate().
    //
    // Der Blaettern-Hotkey ist ausdruecklich dafuer da, ohne sichtbares Panel
    // zu wirken (docs/ui/Hotkey_Konzept.md §1) — er liest aber die Liste. War
    // das Panel seit dem Start nie offen, war sie leer, und `preset.next`/
    // `preset.previous` taten nach jedem Neustart nichts, bis man einmal im
    // Browser etwas geladen hatte. Ein Verzeichnis-Listing kostet nichts.
    refresh();

    // Permanent subscription (survives onDeactivate): the Settings panel can
    // reset the remembered folder while this panel is hidden.
    if (auto* bus = eventBus())
    {
        m_resetSubscription = bus->subscribeScoped<ResetImportBrowserDirEvent>(
            [this](const ResetImportBrowserDirEvent&) { resetStoredDir(); });
        // Ebenfalls PERMANENT: der Hotkey soll blaettern, ohne dass dieses Panel
        // den Fokus oder ueberhaupt die Sichtbarkeit haben muss — genau dafuer
        // ist er da (docs/ui/Hotkey_Konzept.md §1, Stufe 1).
        m_stepSubscription = bus->subscribeScoped<PresetStepEvent>(
            [this](const PresetStepEvent& e) { onPresetStep(e.delta); });
    }

    // Zuletzt geladenes Preset wieder anwerfen (S73) — VERZOEGERT ueber die
    // Ereignisschleife: hier im Konstruktor steht der Visualizer, der das
    // Import-Event verarbeiten muss, noch nicht. Der Aufhaenger ist bewusst der
    // Konstruktor und nicht restoreState(): PanelManager::restoreState() wird
    // im ganzen Programm nirgends aufgerufen, die Ueberschreibung liefe also
    // nie (Befund S73).
    QTimer::singleShot(0, this, [this] { restoreLastPreset(); });
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
    settings.setValue(QStringLiteral("lastPreset"), m_lastPreset);
    settings.endGroup();
}

void ImportBrowserPanel::restoreState()
{
    PanelBase::restoreState();
    refresh();
}

void ImportBrowserPanel::restoreLastPreset()
{
    if (m_lastPreset.isEmpty()) return;
    if (!QFileInfo::exists(m_lastPreset))
    {
        BasicLogger::logInfo("ImportBrowserPanel: last preset is gone: " +
                             m_lastPreset.toStdString());
        m_lastPreset.clear();
        return;
    }

    // BEWUSST OHNE die Liste (S73): die wird erst in onActivate() gefuellt, und
    // dieses Panel ist beim Start womoeglich gar nicht sichtbar. Der Ladeweg
    // haengt allein am Pfad; markiert wird spaeter, sobald die Liste entsteht
    // (selectLastPresetInList aus refresh()).
    const int type =
        entryTypeForSuffix(QFileInfo(m_lastPreset).suffix());
    if (type != Type_Avs && type != Type_Milk && type != Type_Lvfx)
    {
        BasicLogger::logWarning("ImportBrowserPanel: last preset has an "
                                "unknown suffix: " + m_lastPreset.toStdString());
        m_lastPreset.clear();
        return;
    }

    BasicLogger::logInfo("ImportBrowserPanel: restoring last preset: " +
                         m_lastPreset.toStdString());
    openEntry(type, m_lastPreset);
    selectLastPresetInList();
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
        case Filter::Lvfx:
            return {QStringLiteral("*.lvfx"), QStringLiteral("*.lvfx2")};
        case Filter::All:  break;
    }
    return {QStringLiteral("*.avs"), QStringLiteral("*.milk"),
            QStringLiteral("*.lvfx"), QStringLiteral("*.lvfx2")};
}

int ImportBrowserPanel::entryTypeForSuffix(const QString& suffix)
{
    if (suffix.compare(QStringLiteral("milk"), Qt::CaseInsensitive) == 0) return Type_Milk;
    if (suffix.compare(QStringLiteral("lvfx"), Qt::CaseInsensitive) == 0) return Type_Lvfx;
    if (suffix.compare(QStringLiteral("lvfx2"), Qt::CaseInsensitive) == 0) return Type_Lvfx;
    return Type_Avs;
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

    // Matching preset files — Format-Icon je Endung (avs/milkdrop/lumiviz,
    // geteilt mit dem Effect-Chain-Panel); Fallback = generisches Datei-Icon.
    const bool haveTypeIcons = !lumi::ui::presetIconDir().isEmpty();
    const QFileInfoList files = m_dir.entryInfoList(
        currentNameFilters(), QDir::Files, QDir::Name | QDir::IgnoreCase);
    int presetCount = 0;
    for (const QFileInfo& fi : files)
    {
        const QIcon& icon = haveTypeIcons
                                ? lumi::ui::presetTypeIconForSuffix(fi.suffix())
                                : fileIcon;
        auto* item = new QListWidgetItem(icon, fi.fileName(), m_pListWidget);
        item->setData(Qt::UserRole, fi.absoluteFilePath());
        item->setData(Qt::UserRole + 1, entryTypeForSuffix(fi.suffix()));
        item->setToolTip(fi.absoluteFilePath());
        ++presetCount;
    }

    // Re-apply the current search text to the freshly built list.
    onSearchChanged(m_pSearchEdit->text());

    // Das laufende Preset markieren, sooft die Liste neu entsteht (S73) —
    // damit sichtbar ist, was gerade zu sehen ist. Reines Markieren, es wird
    // NICHT geladen: das Laden haengt an restoreLastPreset() bzw. am Nutzer.
    selectLastPresetInList();

    setStatus(tr("%1 preset(s) · %2 folder(s)").arg(presetCount).arg(dirs.size()));
}

void ImportBrowserPanel::selectLastPresetInList()
{
    if (m_lastPreset.isEmpty() || m_pListWidget == nullptr) return;

    const QFileInfo wanted(m_lastPreset);
    for (int row = 0; row < m_pListWidget->count(); ++row)
    {
        QListWidgetItem* item = m_pListWidget->item(row);
        if (QFileInfo(item->data(Qt::UserRole).toString()) != wanted) continue;
        m_pListWidget->setCurrentRow(row);
        m_pListWidget->scrollToItem(item);
        return;
    }
}

// =============================================================================
// Slots
// =============================================================================

void ImportBrowserPanel::onItemDoubleClicked(QListWidgetItem* item)
{
    if (item == nullptr) return;
    openEntry(item->data(Qt::UserRole + 1).toInt(),
              item->data(Qt::UserRole).toString());
}

void ImportBrowserPanel::openEntry(int type, const QString& path)
{
    // Merken, was zuletzt WIRKLICH geladen wurde (S73) — Ordner und ".." nicht.
    // Beim naechsten Start wird genau das wieder markiert und geladen.
    if (type == Type_Avs || type == Type_Milk || type == Type_Lvfx)
    {
        m_lastPreset = path;
    }

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

        case Type_Lvfx:
            if (auto* bus = eventBus())
            {
                bus->publish(LoadEffectChainEvent{path.toStdString()});
                setStatus(tr("↺ loading chain %1").arg(QFileInfo(path).fileName()));
            }
            return;

        default:
            return;
    }
}

// =============================================================================
// Preset-Navigation (Hotkey `preset.next` / `preset.previous`)
// =============================================================================

int ImportBrowserPanel::nextPresetRow(const QList<int>& types, int from, int delta)
{
    if (types.isEmpty() || delta == 0) return -1;
    const int step = delta > 0 ? 1 : -1;
    // Von "keine Auswahl" aus am jeweiligen Ende beginnen, damit der erste
    // Tastendruck etwas laedt statt ins Leere zu laufen.
    int row = from;
    if (row < 0 || row >= types.size()) row = step > 0 ? -1 : types.size();
    for (row += step; row >= 0 && row < types.size(); row += step)
    {
        const int type = types.at(row);
        // Ordner und ".." werden uebersprungen — Stufe 1 blaettert nur ueber die
        // Presets des AKTIVEN Verzeichnisses, ohne Rekursion (Entscheid 2).
        if (type == Type_Avs || type == Type_Milk || type == Type_Lvfx) return row;
    }
    return -1;  // am Ende anhalten, nicht umlaufen (Entscheid 3)
}

void ImportBrowserPanel::onPresetStep(int delta)
{
    if (m_pListWidget == nullptr) return;

    QList<int> types;
    types.reserve(m_pListWidget->count());
    for (int i = 0; i < m_pListWidget->count(); ++i)
    {
        types.append(m_pListWidget->item(i)->data(Qt::UserRole + 1).toInt());
    }

    const int row = nextPresetRow(types, m_pListWidget->currentRow(), delta);
    BasicLogger::logDebug("ImportBrowserPanel: step " + std::to_string(delta) +
                          " from " + std::to_string(m_pListWidget->currentRow()) +
                          " -> " + std::to_string(row) + " (" +
                          std::to_string(types.size()) + " entries)");
    if (row < 0)
    {
        setStatus(delta > 0 ? tr("End of folder") : tr("Start of folder"));
        return;
    }
    m_pListWidget->setCurrentRow(row);
    m_pListWidget->scrollToItem(m_pListWidget->item(row));
    // Derselbe Weg wie ein Doppelklick — eine Wahrheit fuer "Eintrag laden".
    onItemDoubleClicked(m_pListWidget->item(row));
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

void ImportBrowserPanel::resetStoredDir()
{
    QSettings settings;
    settings.beginGroup(settingsPrefix());
    settings.remove(QStringLiteral("lastDir"));
    // Auch die Preset-Erinnerung faellt (S73) — sonst zeigte der Browser nach
    // dem Zuruecksetzen den Programmordner, beim naechsten Start aber wieder
    // den Ordner des gemerkten Presets.
    settings.remove(QStringLiteral("lastPreset"));
    settings.endGroup();
    m_lastPreset.clear();

    const QString target = defaultStartDir();
    navigateTo(target);
    setStatus(tr("Start folder reset to %1").arg(target));
    BasicLogger::logInfo("ImportBrowserPanel: stored start folder reset");
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
    m_pFilterCombo->addItem(tr("AVS (*.avs)"));         // Filter::Avs
    m_pFilterCombo->addItem(tr("MilkDrop (*.milk)"));   // Filter::Milk
    m_pFilterCombo->addItem(tr("LumiViz (*.lvfx)"));    // Filter::Lvfx
    m_pFilterCombo->addItem(tr("All"));                 // Filter::All
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
