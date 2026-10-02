/**
 ****************************************************************************************
 * @file   EffectPaletteWindow.cpp
 * @brief  Nicht-modales Effekt-Palettenfenster (s. EffectPaletteWindow.hpp)
 *
 * @author Patrik Neunteufel
 * @date   September 2026
 ****************************************************************************************
 */

#include "UI/panels/EffectPaletteWindow.hpp"

#include "UI/panels/EffectPalette.hpp"
#include "visualizers/multieffect/ScriptVariables.hpp"

#include <QFont>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QHideEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMimeData>
#include <QPushButton>
#include <QSettings>
#include <QToolButton>
#include <QTreeWidgetItemIterator>
#include <QVBoxLayout>

#include <iterator>

namespace lumi::ui {

namespace {

constexpr int kRecentMax = 8;
constexpr int kIndexRole = Qt::UserRole;  ///< Paletten-Index; -1 = Kopf

const QString kKeyGeometry = QStringLiteral("effectPalette/geometry");
const QString kKeyFavorites = QStringLiteral("effectPalette/favorites");
const QString kKeyRecent = QStringLiteral("effectPalette/recent");

constexpr Origin kOrigins[] = {Origin::Avs, Origin::Ape, Origin::MilkDrop, Origin::Native};
constexpr Role kRoles[] = {Role::Render, Role::Transform, Role::Color, Role::Control};

/// Namen (stabil ueber Palettenumbauten) -> Indizes (nur, was es noch gibt).
QList<int> indicesForNames(const QStringList& names)
{
    const auto& palette = effectPalette();
    QList<int> out;
    for (const QString& n : names)
        for (size_t i = 0; i < palette.size(); ++i)
            if (!palette[i].isHeader() && n == QString::fromUtf8(palette[i].name))
            {
                out.append(static_cast<int>(i));
                break;
            }
    return out;
}

QStringList namesForIndices(const QList<int>& indices)
{
    QStringList out;
    for (int i : indices) out.append(QString::fromUtf8(effectPalette()[static_cast<size_t>(i)].name));
    return out;
}

/// Kategorie (Kopf-Titel) eines Eintrags.
QString categoryOf(int paletteIndex)
{
    const auto& palette = effectPalette();
    for (int i = paletteIndex; i >= 0; --i)
        if (palette[static_cast<size_t>(i)].isHeader())
            return paletteCategoryTitle(palette[static_cast<size_t>(i)]);
    return {};
}

QToolButton* makeChip(const QString& text, const QString& tip, QWidget* parent)
{
    auto* b = new QToolButton(parent);
    b->setText(text);
    b->setToolTip(tip);
    b->setCheckable(true);
    b->setAutoRaise(false);
    return b;
}

}  // namespace

// =============================================================================
// PaletteTree
// =============================================================================

QStringList PaletteTree::mimeTypes() const
{
    return {QString::fromUtf8(kPaletteMime)};
}

QMimeData* PaletteTree::mimeData(const QList<QTreeWidgetItem*>& items) const
{
    if (items.isEmpty()) return nullptr;
    const int index = items.first()->data(0, kIndexRole).toInt();
    if (index < 0) return nullptr;
    auto* mime = new QMimeData();
    mime->setData(QString::fromUtf8(kPaletteMime), QByteArray::number(index));
    return mime;
}

// =============================================================================
// EffectPaletteWindow
// =============================================================================

EffectPaletteWindow::EffectPaletteWindow(QWidget* parent)
    : QWidget(parent, Qt::Tool | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{
    setWindowTitle(tr("Effect Palette"));
    setAttribute(Qt::WA_DeleteOnClose, false);  // Schliessen = verstecken

    auto* root = new QVBoxLayout(this);

    m_search = new QLineEdit(this);
    m_search->setPlaceholderText(tr("Search name, original name, description…"));
    m_search->setClearButtonEnabled(true);
    m_search->installEventFilter(this);  // Pfeil runter -> Baum, Enter -> Add
    root->addWidget(m_search);

    // Filter-Chips: innerhalb einer Gruppe ODER, zwischen den Gruppen UND;
    // keine Chip einer Gruppe gedrueckt = diese Gruppe filtert nicht.
    auto* originRow = new QHBoxLayout();
    for (Origin o : kOrigins)
    {
        QToolButton* chip = makeChip(originText(o), originToolTip(o), this);
        chip->setIcon(originIcon(o));
        connect(chip, &QToolButton::toggled, this, [this] { applyFilter(); });
        originRow->addWidget(chip);
        m_originChips.push_back(chip);
    }
    originRow->addStretch();
    root->addLayout(originRow);

    auto* roleRow = new QHBoxLayout();
    const QString roleTips[] = {
        tr("Draws new content"),
        tr("Moves, warps or feeds back the existing image"),
        tr("Changes colours / pixels of the existing image"),
        tr("Draws nothing: structure, timing, buffers, state"),
    };
    for (size_t i = 0; i < std::size(kRoles); ++i)
    {
        QToolButton* chip = makeChip(roleText(kRoles[i]), roleTips[i], this);
        connect(chip, &QToolButton::toggled, this, [this] { applyFilter(); });
        roleRow->addWidget(chip);
        m_roleChips.push_back(chip);
    }
    roleRow->addStretch();
    root->addLayout(roleRow);

    m_tree = new PaletteTree(this);
    m_tree->setHeaderHidden(true);
    m_tree->setColumnCount(1);
    m_tree->setRootIsDecorated(true);
    m_tree->setDragEnabled(true);
    m_tree->setDragDropMode(QAbstractItemView::DragOnly);
    m_tree->setDefaultDropAction(Qt::CopyAction);
    m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(m_tree, &QTreeWidget::currentItemChanged, this, [this] { updateDetail(); });
    connect(m_tree, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem* item) {
        if (item != nullptr && item->data(0, kIndexRole).toInt() >= 0) requestAdd();
    });
    m_tree->installEventFilter(this);  // Enter -> Add
    root->addWidget(m_tree, 1);

    m_detail = new QLabel(this);
    m_detail->setWordWrap(true);
    m_detail->setTextFormat(Qt::RichText);
    m_detail->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_detail->setMinimumHeight(m_detail->fontMetrics().lineSpacing() * 5);
    root->addWidget(m_detail);

    auto* buttons = new QHBoxLayout();
    m_favButton = new QToolButton(this);
    m_favButton->setText(QString::fromUtf8("★"));
    m_favButton->setCheckable(true);
    m_favButton->setToolTip(tr("Favourite (shown at the top)"));
    connect(m_favButton, &QToolButton::clicked, this, [this] { toggleFavorite(); });
    buttons->addWidget(m_favButton);
    buttons->addStretch();
    m_addButton = new QPushButton(tr("Add"), this);
    m_addButton->setToolTip(
        tr("Insert into the selected list, else below the selected effect, else at the "
           "end — or drag the entry into the chain"));
    connect(m_addButton, &QPushButton::clicked, this, [this] { requestAdd(); });
    buttons->addWidget(m_addButton);
    auto* closeButton = new QPushButton(tr("Close"), this);
    connect(closeButton, &QPushButton::clicked, this, &QWidget::close);
    buttons->addWidget(closeButton);
    root->addLayout(buttons);

    connect(m_search, &QLineEdit::textChanged, this, [this] { applyFilter(); });

    loadSettings();
    buildTree();
    applyFilter();
    setInsertEnabled(false);
}

void EffectPaletteWindow::loadSettings()
{
    QSettings settings;
    m_favorites = indicesForNames(settings.value(kKeyFavorites).toStringList());
    m_recent = indicesForNames(settings.value(kKeyRecent).toStringList());
    const QByteArray geo = settings.value(kKeyGeometry).toByteArray();
    if (geo.isEmpty() || !restoreGeometry(geo)) resize(380, 560);
}

void EffectPaletteWindow::saveLists() const
{
    QSettings settings;
    settings.setValue(kKeyFavorites, namesForIndices(m_favorites));
    settings.setValue(kKeyRecent, namesForIndices(m_recent));
}

QTreeWidgetItem* EffectPaletteWindow::makeLeaf(QTreeWidgetItem* parent, int paletteIndex)
{
    const EffectType& t = effectPalette()[static_cast<size_t>(paletteIndex)];
    auto* item = new QTreeWidgetItem(parent);
    item->setText(0, QString::fromUtf8(t.name));
    item->setIcon(0, originIcon(t.origin));
    item->setToolTip(0, QString::fromUtf8(t.summary));
    item->setData(0, kIndexRole, paletteIndex);
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
    return item;
}

void EffectPaletteWindow::buildTree()
{
    const int keep = currentIndex();
    m_tree->clear();

    auto makeSection = [this](const QString& title) {
        auto* item = new QTreeWidgetItem(m_tree);
        item->setText(0, title);
        item->setData(0, kIndexRole, -1);
        item->setData(0, Qt::UserRole + 1, title);  // Titel ohne Zaehler
        item->setFlags(Qt::ItemIsEnabled);
        QFont f = item->font(0);
        f.setBold(true);
        item->setFont(0, f);
        return item;
    };

    m_favSection = makeSection(QString::fromUtf8("★ ") + tr("Favourites"));
    for (int i : m_favorites) makeLeaf(m_favSection, i);
    m_recentSection = makeSection(tr("Recently used"));
    for (int i : m_recent) makeLeaf(m_recentSection, i);

    const auto& palette = effectPalette();
    QTreeWidgetItem* section = nullptr;
    for (size_t i = 0; i < palette.size(); ++i)
    {
        if (palette[i].isHeader())
            section = makeSection(paletteCategoryTitle(palette[i]));
        else if (section != nullptr)
            makeLeaf(section, static_cast<int>(i));
    }

    m_tree->expandAll();
    // Auswahl wiederherstellen (erster Treffer ausserhalb der Kopf-Abschnitte
    // genuegt: derselbe Index steht hoechstens in Favoriten/Zuletzt doppelt).
    if (keep >= 0)
    {
        for (QTreeWidgetItemIterator it(m_tree); *it != nullptr; ++it)
            if ((*it)->data(0, kIndexRole).toInt() == keep)
            {
                m_tree->setCurrentItem(*it);
                break;
            }
    }
}

void EffectPaletteWindow::setUebergeordneterFilter(bool (*erlaubt)(const EffectType&),
                                                   const QString& titel)
{
    m_erlaubt = erlaubt;
    if (!titel.isEmpty()) setWindowTitle(titel);
    // Rollen-Chips ohne erlaubten Eintrag ausblenden
    const auto& palette = effectPalette();
    for (size_t r = 0; r < std::size(kRoles) && r < m_roleChips.size(); ++r)
    {
        bool hat = erlaubt == nullptr;
        for (size_t i = 0; i < palette.size() && !hat; ++i)
            hat = !palette[i].isHeader() && palette[i].role == kRoles[r] && erlaubt(palette[i]);
        m_roleChips[r]->setChecked(false);
        m_roleChips[r]->setVisible(hat);
    }
    applyFilter();
}

bool EffectPaletteWindow::matches(int paletteIndex) const
{
    const EffectType& t = effectPalette()[static_cast<size_t>(paletteIndex)];
    if (m_erlaubt != nullptr && !m_erlaubt(t)) return false;

    auto groupPasses =[](const std::vector<QToolButton*>& chips, size_t hit) {
        bool any = false;
        for (const QToolButton* c : chips) any = any || c->isChecked();
        return !any || chips[hit]->isChecked();
    };
    size_t originPos = 0;
    while (originPos < std::size(kOrigins) && kOrigins[originPos] != t.origin) ++originPos;
    size_t rolePos = 0;
    while (rolePos < std::size(kRoles) && kRoles[rolePos] != t.role) ++rolePos;
    if (!groupPasses(m_originChips, originPos)) return false;
    if (!groupPasses(m_roleChips, rolePos)) return false;

    const QString query = m_search->text().trimmed();
    if (query.isEmpty()) return true;
    const QString hay = QString::fromUtf8(t.name) + ' ' + QString::fromUtf8(t.originalName) +
                        ' ' + QString::fromUtf8(t.summary) + ' ' + categoryOf(paletteIndex) +
                        ' ' + originText(t.origin) + ' ' + roleText(t.role);
    // Jedes Wort muss vorkommen (in beliebiger Reihenfolge).
    for (const QString& word : query.split(' ', Qt::SkipEmptyParts))
        if (!hay.contains(word, Qt::CaseInsensitive)) return false;
    return true;
}

void EffectPaletteWindow::applyFilter()
{
    QTreeWidgetItem* firstHit = nullptr;
    for (int s = 0; s < m_tree->topLevelItemCount(); ++s)
    {
        QTreeWidgetItem* section = m_tree->topLevelItem(s);
        int visible = 0;
        for (int c = 0; c < section->childCount(); ++c)
        {
            QTreeWidgetItem* leaf = section->child(c);
            const bool show = matches(leaf->data(0, kIndexRole).toInt());
            leaf->setHidden(!show);
            if (show)
            {
                ++visible;
                if (firstHit == nullptr && section != m_favSection &&
                    section != m_recentSection)
                    firstHit = leaf;
            }
        }
        section->setHidden(visible == 0);
        section->setText(0, QStringLiteral("%1 (%2)")
                                .arg(section->data(0, Qt::UserRole + 1).toString())
                                .arg(visible));
    }
    // Beim Tippen den ersten Treffer markieren: Enter fuegt ihn ein.
    QTreeWidgetItem* cur = m_tree->currentItem();
    if (!m_search->text().trimmed().isEmpty() &&
        (cur == nullptr || cur->isHidden() || cur->data(0, kIndexRole).toInt() < 0))
        m_tree->setCurrentItem(firstHit);
    updateDetail();
}

int EffectPaletteWindow::currentIndex() const
{
    const QTreeWidgetItem* item = m_tree->currentItem();
    if (item == nullptr || item->isHidden()) return -1;
    return item->data(0, kIndexRole).toInt();
}

void EffectPaletteWindow::updateDetail()
{
    const int index = currentIndex();
    m_favButton->setEnabled(index >= 0);
    m_addButton->setEnabled(index >= 0 && m_insertEnabled);
    if (index < 0)
    {
        m_favButton->setChecked(false);
        m_detail->setText(tr("<i>Pick an effect: Add, double-click, Enter — or drag it "
                             "into the chain.</i>"));
        return;
    }
    m_favButton->setChecked(m_favorites.contains(index));

    const EffectType& t = effectPalette()[static_cast<size_t>(index)];
    QString formelText;  // nicht `slots` — das ist in Qt ein Makro
    if (t.milkElement != MilkElement::None)
        formelText = tr("MilkDrop element code");
    else if (const auto* table = lumi::multieffect::scriptVariables(t.make()))
    {
        int count = 0;
        for (const auto& v : table->vars)
            count += static_cast<int>(lumi::multieffect::splitScriptVarNames(v.names).size());
        formelText = tr("yes — %n module variable(s)", nullptr, count);
    }
    else
        formelText = tr("none");

    QString text = QStringLiteral("<b>%1</b><br><small>%2 · %3")
                       .arg(QString::fromUtf8(t.name).toHtmlEscaped(), originText(t.origin),
                            roleText(t.role));
    if (*t.originalName != '\0')
        text += QStringLiteral(" · „%1“").arg(QString::fromUtf8(t.originalName).toHtmlEscaped());
    text += QStringLiteral("</small><br>%1<br><small>%2 %3</small>")
                .arg(QString::fromUtf8(t.summary).toHtmlEscaped(), tr("Formula fields:"), formelText);
    if (!m_insertEnabled)
        text += QStringLiteral("<br><small><i>%1</i></small>")
                    .arg(tr("Select the \"Multi Effect\" visualizer to insert."));
    m_detail->setText(text);
}

void EffectPaletteWindow::setInsertEnabled(bool enabled)
{
    m_insertEnabled = enabled;
    m_tree->setDragEnabled(enabled);
    updateDetail();
}

void EffectPaletteWindow::requestAdd()
{
    const int index = currentIndex();
    if (index < 0 || !m_insertEnabled) return;
    emit addRequested(index);
}

void EffectPaletteWindow::noteUsed(int paletteIndex)
{
    m_recent.removeAll(paletteIndex);
    m_recent.prepend(paletteIndex);
    while (m_recent.size() > kRecentMax) m_recent.removeLast();
    saveLists();
    buildTree();
    applyFilter();
}

void EffectPaletteWindow::toggleFavorite()
{
    const int index = currentIndex();
    if (index < 0) return;
    if (m_favorites.contains(index))
        m_favorites.removeAll(index);
    else
        m_favorites.append(index);
    saveLists();
    buildTree();
    applyFilter();
}

void EffectPaletteWindow::present()
{
    show();
    raise();
    activateWindow();
    m_search->setFocus();
    m_search->selectAll();
}

void EffectPaletteWindow::hideEvent(QHideEvent* event)
{
    QSettings().setValue(kKeyGeometry, saveGeometry());
    QWidget::hideEvent(event);
}

bool EffectPaletteWindow::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
    {
        const auto* key = static_cast<QKeyEvent*>(event);
        const bool enter = key->key() == Qt::Key_Return || key->key() == Qt::Key_Enter;
        if (enter && (watched == m_search || watched == m_tree))
        {
            requestAdd();
            return true;
        }
        if (watched == m_search && key->key() == Qt::Key_Down)
        {
            m_tree->setFocus();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

}  // namespace lumi::ui
