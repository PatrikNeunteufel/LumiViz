/**
 * @file RasterZeile.cpp
 * @brief Die Zeile mit dem Raster des Loops und dem Swing offset (Stufe 15a)
 */
#include "loop/RasterZeile.hpp"

#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSignalBlocker>

#include <cmath>

namespace lumi::loop
{

QString swingText(double swing)
{
    return QStringLiteral("%1 %").arg(std::lround(swing * 100.0));
}

RasterZeile::RasterZeile(const QString& kennung, QWidget* parent, bool mitOffset) : QWidget(parent)
{
    setObjectName(kennung + QStringLiteral("GridRow"));
    auto* reihe = new QHBoxLayout(this);
    reihe->setContentsMargins(0, 0, 0, 0);

    m_raster = new QLabel(this);
    m_raster->setObjectName(kennung + QStringLiteral("Grid"));
    m_raster->setEnabled(false);  // grau: eine Auskunft
    m_raster->setToolTip(tr("The grid of the whole loop. Set in the Loop panel, next to BPM"));
    reihe->addWidget(m_raster);
    if (!mitOffset)
    {
        // ein Wave-Muster swingt nicht (Stufe 15d, RS1-1 a): nur das Raster
        reihe->addStretch(1);
        return;
    }
    reihe->addSpacing(18);

    reihe->addWidget(new QLabel(tr("Swing offset"), this));
    m_offset = new QDoubleSpinBox(this);
    m_offset->setObjectName(kennung + QStringLiteral("SwingOffset"));
    m_offset->setRange(-kSwingOffsetMax * 100.0, kSwingOffsetMax * 100.0);
    m_offset->setDecimals(0);
    m_offset->setKeyboardTracking(false);  // erst die fertige Zahl gilt — nicht jede Ziffer ein Schritt fuer Undo
    m_offset->setToolTip(tr("How much more (+) or less (−) this pattern swings than the loop, in percentage points.\n"
                            "0 = the swing of the loop. A soloist often swings a little less than the drums"));
    reihe->addWidget(m_offset);
    connect(m_offset, &QDoubleSpinBox::valueChanged, this,
            [this](double punkte) { Q_EMIT offsetGeaendert(punkte / 100.0); });

    m_spielt = new QLabel(this);
    m_spielt->setObjectName(kennung + QStringLiteral("SwingPlays"));
    m_spielt->setEnabled(false);
    m_spielt->setToolTip(tr("The swing this pattern plays with: the swing of the loop and the offset, 0 to 100 %"));
    reihe->addWidget(m_spielt);
    reihe->addStretch(1);
}

void RasterZeile::zeige(const Loop& loop, const Muster& muster)
{
    m_raster->setText(tr("Grid of the loop: %1 beats per bar · %2 steps per beat · swing %3")
                          .arg(loop.schlaegeJeTakt)
                          .arg(loop.schritteJeSchlag)
                          .arg(swingText(loop.swing)));
    if (m_offset == nullptr) return;
    {
        const QSignalBlocker ruhe(m_offset);
        m_offset->setValue(muster.swingOffset * 100.0);
    }
    m_spielt->setText(tr("plays %1").arg(swingText(swingVon(loop, muster))));
}

QString RasterZeile::rasterText() const
{
    return m_raster->text();
}

QString RasterZeile::spieltText() const
{
    return m_spielt != nullptr ? m_spielt->text() : QString();
}

}  // namespace lumi::loop
