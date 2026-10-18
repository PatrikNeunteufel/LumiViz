/**
 * @file StrumZeile.cpp
 * @brief Die Zeile Strum (Stufe 24d, 24e)
 */
#include "loop/StrumZeile.hpp"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QToolButton>

#include <algorithm>

namespace lumi::loop
{

StrumZeile::StrumZeile(const QString& kennung, QWidget* parent) : QWidget(parent), m_kennung(kennung)
{
    setObjectName(kennung + QStringLiteral("Row"));
    auto* reihe = new QHBoxLayout(this);
    reihe->setContentsMargins(0, 0, 0, 0);

    reihe->addWidget(new QLabel(tr("Strum"), this));
    reihe->addWidget(new QLabel(tr("Spread"), this));
    m_spread = new QDoubleSpinBox(this);
    m_spread->setObjectName(kennung + QStringLiteral("Spread"));
    m_spread->setRange(0.0, kSpreadMaxMs);
    m_spread->setDecimals(1);
    m_spread->setSingleStep(1.0);
    m_spread->setSuffix(tr(" ms"));
    m_spread->setToolTip(tr("How long the hand takes from one string to the next. 0 = all strings at once.\n"
                            "A chord over six strings takes five times this long"));
    reihe->addWidget(m_spread);
    connect(m_spread, &QDoubleSpinBox::valueChanged, this, &StrumZeile::setzeSpread);

    reihe->addWidget(new QLabel(tr("Hand"), this));
    m_raster = new QComboBox(this);
    m_raster->setObjectName(kennung + QStringLiteral("Hand"));
    m_raster->addItem(tr("Beat"), static_cast<int>(HandRaster::Schlag));
    m_raster->addItem(tr("1/2 beat"), static_cast<int>(HandRaster::HalberSchlag));
    m_raster->addItem(tr("Step"), static_cast<int>(HandRaster::Schritt));
    m_raster->setToolTip(tr("How often the hand moves down and up within a bar. A chord takes the direction of the\n"
                            "place it starts on; between two places it takes the earlier one"));
    reihe->addWidget(m_raster);
    connect(m_raster, &QComboBox::activated, this,
            [this](int platz) { setzeHandRaster(static_cast<HandRaster>(m_raster->itemData(platz).toInt())); });

    m_feldReihe = new QHBoxLayout;
    m_feldReihe->setSpacing(2);
    reihe->addLayout(m_feldReihe);

    m_abwechselnd = new QPushButton(tr("Alternate"), this);
    m_abwechselnd->setObjectName(kennung + QStringLiteral("Alternate"));
    m_abwechselnd->setToolTip(tr("All places back to down, up, down, up"));
    reihe->addWidget(m_abwechselnd);
    connect(m_abwechselnd, &QPushButton::clicked, this, &StrumZeile::abwechselnd);

    m_hinweis = new QLabel(this);
    m_hinweis->setObjectName(kennung + QStringLiteral("Hint"));
    m_hinweis->setEnabled(false);
    m_hinweis->setVisible(false);
    reihe->addWidget(m_hinweis);
    reihe->addStretch(1);
    zeigeFelder();
}

void StrumZeile::zeige(const Strum& strum)
{
    m_strum = strum;
    richteStrum(m_strum, stellen());
    zeigeFelder();
}

void StrumZeile::setzeRaster(int schlaegeJeTakt, int schritteJeSchlag)
{
    m_schlaege = std::max(1, schlaegeJeTakt);
    m_schritte = std::max(1, schritteJeSchlag);
    zeigeFelder();
}

void StrumZeile::setzeHinweis(const QString& text)
{
    m_hinweis->setText(text);
    m_hinweis->setVisible(!text.isEmpty());
}

int StrumZeile::stellen() const
{
    return std::min(kHandStellenMax, handStellen(m_strum.raster, m_schlaege, m_schritte));
}

QToolButton* StrumZeile::feld(int stelle) const
{
    return stelle >= 0 && stelle < static_cast<int>(m_felder.size()) && stelle < stellen()
               ? m_felder[static_cast<std::size_t>(stelle)]
               : nullptr;
}

bool StrumZeile::weichtAb(int stelle) const
{
    const Richtung abwechselnd = stelle % 2 == 0 ? Richtung::Ab : Richtung::Auf;
    return richtungAnStelle(m_strum, stelle) != abwechselnd;
}

void StrumZeile::setzeSpread(double ms)
{
    if (ms == m_strum.spreadMs) return;
    m_strum.spreadMs = ms;
    melde();
}

void StrumZeile::setzeHandRaster(HandRaster raster)
{
    if (raster == m_strum.raster) return;
    m_strum.raster = raster;
    melde();
}

void StrumZeile::dreheStelle(int stelle)
{
    if (stelle < 0 || stelle >= stellen()) return;
    // bis zur Stelle aufschreiben, was ohnehin gilt, dann umdrehen
    while (static_cast<int>(m_strum.hand.size()) <= stelle)
    {
        const auto platz = static_cast<int>(m_strum.hand.size());
        m_strum.hand.push_back(platz % 2 == 0 ? Richtung::Ab : Richtung::Auf);
    }
    Richtung& richtung = m_strum.hand[static_cast<std::size_t>(stelle)];
    richtung = richtung == Richtung::Ab ? Richtung::Auf : Richtung::Ab;
    melde();
}

void StrumZeile::abwechselnd()
{
    if (m_strum.hand.empty()) return;
    m_strum.hand.clear();
    melde();
}

void StrumZeile::melde()
{
    richteStrum(m_strum, stellen());
    zeigeFelder();
    Q_EMIT geaendert(m_strum);
}

void StrumZeile::zeigeFelder()
{
    {
        const QSignalBlocker ruhe(m_spread);
        m_spread->setValue(m_strum.spreadMs);
    }
    m_raster->setCurrentIndex(m_raster->findData(static_cast<int>(m_strum.raster)));
    const int anzahl = stellen();
    while (static_cast<int>(m_felder.size()) < anzahl)
    {
        const int stelle = static_cast<int>(m_felder.size());
        auto* feld = new QToolButton(this);
        feld->setObjectName(m_kennung + QStringLiteral("Cell%1").arg(stelle + 1));
        feld->setCheckable(true);
        feld->setFixedWidth(24);
        feld->setCursor(Qt::PointingHandCursor);
        m_feldReihe->addWidget(feld);
        connect(feld, &QToolButton::clicked, this, [this, stelle] { dreheStelle(stelle); });
        m_felder.push_back(feld);
    }
    for (std::size_t i = 0; i < m_felder.size(); ++i)
    {
        QToolButton* feld = m_felder[i];
        const int stelle = static_cast<int>(i);
        feld->setVisible(stelle < anzahl);
        if (stelle >= anzahl) continue;
        const bool ab = richtungAnStelle(m_strum, stelle) == Richtung::Ab;
        feld->setText(ab ? QStringLiteral("↓") : QStringLiteral("↑"));
        const QSignalBlocker ruhe(feld);
        feld->setChecked(weichtAb(stelle));
        feld->setToolTip(tr("Place %1 of the bar: %2 — click to turn it%3")
                             .arg(stelle + 1)
                             .arg(ab ? tr("down") : tr("up"))
                             .arg(weichtAb(stelle) ? tr(" (set by you)") : QString()));
    }
    m_abwechselnd->setEnabled(!m_strum.hand.empty());
}

}  // namespace lumi::loop
