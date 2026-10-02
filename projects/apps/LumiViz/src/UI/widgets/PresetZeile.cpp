/**
 * @file PresetZeile.cpp
 * @brief Preset-Zeile der Panels „Effect Chain" und „Karaoke" (S89)
 */

#include "UI/widgets/PresetZeile.hpp"

#include "UI/widgets/PresetTypeIcons.hpp"

#include <QComboBox>
#include <QEvent>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QStandardItemModel>
#include <QToolButton>
#include <QVBoxLayout>

namespace lumi::ui
{

namespace
{
/// Ueberschrift im Dropdown: nicht waehlbar.
void kopfzeile(QComboBox* combo, const QString& text)
{
    combo->addItem(text);
    if (auto* modell = qobject_cast<QStandardItemModel*>(combo->model()))
        if (QStandardItem* item = modell->item(combo->count() - 1))
        {
            item->setFlags(Qt::NoItemFlags);
            QFont f = item->font();
            f.setItalic(true);
            item->setFont(f);
        }
}
}  // namespace

PresetZeile::PresetZeile(PresetArt art, QWidget* parent)
    : QWidget(parent)
    , m_art(art)
    , m_ohneName(art == PresetArt::Kette ? tr("(new effect chain)") : tr("(karaoke layer)"))
{
    // Zwei Reihen: oben Dropdown und ★, darunter die Knoepfe — in einer Reihe
    // waere die Zeile breiter als ein schmal gestelltes Panel (S78-Vertrag:
    // das Panel scrollt dann seitlich, „Save As…" laege ausserhalb).
    auto* aussen = new QVBoxLayout(this);
    aussen->setContentsMargins(0, 0, 0, 0);
    aussen->setSpacing(4);
    auto* oben = new QHBoxLayout();
    m_layout = new QHBoxLayout();
    aussen->addLayout(oben);
    aussen->addLayout(m_layout);
    oben->addWidget(new QLabel(tr("Preset:"), this));

    m_combo = new QComboBox(this);
    m_combo->setObjectName(QStringLiteral("presetZeileCombo"));
    m_combo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_combo->setMinimumContentsLength(14);
    m_combo->setToolTip(tr("The running preset, your favourites and the ten presets used last. "
                           "Choosing one loads it at once."));
    // das Mausrad darf nichts laden (Entscheid 1a)
    m_combo->setFocusPolicy(Qt::StrongFocus);
    m_combo->installEventFilter(this);
    oben->addWidget(m_combo, 1);

    m_stern = new QToolButton(this);
    m_stern->setObjectName(QStringLiteral("presetZeileStern"));
    m_stern->setText(QString::fromUtf8("★"));
    m_stern->setCheckable(true);
    m_stern->setToolTip(tr("Favourite: keep the running preset in this list"));
    oben->addWidget(m_stern);

    auto* laden = new QPushButton(tr("Load…"), this);
    laden->setObjectName(QStringLiteral("presetZeileLaden"));
    auto* sichern = new QPushButton(tr("Save"), this);
    sichern->setObjectName(QStringLiteral("presetZeileSpeichern"));
    sichern->setToolTip(tr("Save into the running preset file. Asks for a place if there is "
                           "none yet, or if the file is not yours to overwrite."));
    auto* sichernAls = new QPushButton(tr("Save As…"), this);
    sichernAls->setObjectName(QStringLiteral("presetZeileSpeichernAls"));
    for (QPushButton* b : {laden, sichern, sichernAls}) m_layout->addWidget(b);
    m_layout->addStretch(1);

    connect(m_combo, &QComboBox::activated, this, [this](int index) {
        const QString pfad = m_combo->itemData(index).toString();
        if (index == 0 || pfad.isEmpty())
        {
            baueListe();  // Kopfzeile oder das laufende Preset: nichts zu tun
            return;
        }
        if (!QFileInfo::exists(pfad))
        {
            // die Datei gibt es nicht mehr: aus beiden Listen
            PresetListen::vergiss(m_art, pfad);
            baueListe();
            emit listenGeaendert();
            return;
        }
        emit ladeDatei(pfad);
        // wurde das Laden abgebrochen, steht hier wieder das laufende Preset
        baueListe();
    });
    connect(m_stern, &QToolButton::toggled, this, [this](bool an) {
        if (m_laufend.isEmpty()) return;
        PresetListen::setzeFavorit(m_art, m_laufend, an);
        baueListe();
        emit listenGeaendert();
    });
    connect(laden, &QPushButton::clicked, this, [this] { emit ladenDialog(); });
    connect(sichern, &QPushButton::clicked, this, [this] { emit speichern(false); });
    connect(sichernAls, &QPushButton::clicked, this, [this] { emit speichern(true); });

    baueListe();
}

bool PresetZeile::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_combo && event->type() == QEvent::Wheel)
    {
        event->ignore();
        return true;  // geschluckt: kein Laden durch Blaettern
    }
    return QWidget::eventFilter(watched, event);
}

void PresetZeile::setLaufend(const QString& pfad)
{
    m_laufend = pfad.isEmpty() ? QString() : PresetListen::normal(pfad);
    baueListe();
}

void PresetZeile::setGeaendert(bool geaendert)
{
    if (m_geaendert == geaendert) return;
    m_geaendert = geaendert;
    // nur die erste Zeile aendert sich — nicht die offene Liste neu bauen
    m_combo->setItemText(0, (m_laufend.isEmpty() ? m_ohneName
                                                 : QFileInfo(m_laufend).completeBaseName()) +
                                (m_geaendert ? QStringLiteral(" *") : QString()));
}

void PresetZeile::aktualisiere()
{
    baueListe();
}

void PresetZeile::setBedienbar(bool an)
{
    for (QWidget* w : findChildren<QWidget*>()) w->setEnabled(an);
    if (an) m_stern->setEnabled(!m_laufend.isEmpty());  // ohne laufendes Preset kein Favorit
}

void PresetZeile::haengeAn(QWidget* knopf)
{
    knopf->setParent(this);
    m_layout->insertWidget(m_layout->count() - 1, knopf);  // vor dem Abstandhalter
}

void PresetZeile::setOhneNameText(const QString& text)
{
    m_ohneName = text;
    baueListe();
}

void PresetZeile::baueListe()
{
    const QSignalBlocker ruhe(m_combo);
    const QSignalBlocker ruheStern(m_stern);
    m_combo->clear();

    // Zeile 0: das laufende Preset
    const QString name =
        m_laufend.isEmpty() ? m_ohneName : QFileInfo(m_laufend).completeBaseName();
    m_combo->addItem(name + (m_geaendert ? QStringLiteral(" *") : QString()), m_laufend);
    m_combo->setItemData(0, m_laufend.isEmpty() ? QString() : m_laufend, Qt::ToolTipRole);

    const bool icons = !presetIconDir().isEmpty();
    const auto eintraege = [&](const QStringList& pfade) {
        for (const QString& pfad : pfade)
        {
            const QFileInfo fi(pfad);
            if (icons)
                m_combo->addItem(presetTypeIconForSuffix(fi.suffix()), fi.completeBaseName(), pfad);
            else
                m_combo->addItem(fi.completeBaseName(), pfad);
            const int i = m_combo->count() - 1;
            m_combo->setItemData(i, pfad, Qt::ToolTipRole);
            if (!fi.exists())  // fehlt: grau, faellt beim Anklicken aus der Liste
                m_combo->setItemData(i, QColor(Qt::gray), Qt::ForegroundRole);
        }
    };
    const QStringList favoriten = PresetListen::favoriten(m_art);
    if (!favoriten.isEmpty())
    {
        kopfzeile(m_combo, tr("— Favourites —"));
        eintraege(favoriten);
    }
    const QStringList zuletzt = PresetListen::zuletzt(m_art);
    if (!zuletzt.isEmpty())
    {
        kopfzeile(m_combo, tr("— Recent —"));
        eintraege(zuletzt);
    }
    m_combo->setCurrentIndex(0);

    m_stern->setEnabled(!m_laufend.isEmpty());
    m_stern->setChecked(!m_laufend.isEmpty() && PresetListen::istFavorit(m_art, m_laufend));
}

}  // namespace lumi::ui
