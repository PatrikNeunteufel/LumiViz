/**
 ****************************************************************************************
 * @file   GradientEditorDialog.cpp
 * @brief  Dialog for editing multi-stop color gradients
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#include "UI/dialogs/GradientEditorDialog.hpp"

#include <QColorDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QPainter>
#include <QMouseEvent>
#include <QLinearGradient>
#include <QGroupBox>
#include <QFormLayout>

#include <algorithm>
#include <cmath>

namespace lumi::ui {

// =============================================================================
// GradientBarWidget Implementation
// =============================================================================

GradientBarWidget::GradientBarWidget(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setMinimumHeight(60);
}

void GradientBarWidget::setGradient(modules::ColorGradientModule* gradient)
{
    m_gradient = gradient;
    m_selectedStop = -1;
    m_selectedMidpoint = -1;
    update();
}

void GradientBarWidget::updateDisplay()
{
    update();
}

float GradientBarWidget::xToPosition(int x) const
{
    int barWidth = width() - 2 * MARGIN;
    float pos = static_cast<float>(x - MARGIN) / static_cast<float>(barWidth);
    return std::clamp(pos, 0.0f, 1.0f);
}

int GradientBarWidget::positionToX(float pos) const
{
    int barWidth = width() - 2 * MARGIN;
    return MARGIN + static_cast<int>(pos * barWidth);
}

int GradientBarWidget::hitTestStop(const QPoint& pos) const
{
    if (!m_gradient) return -1;
    
    const auto& stops = m_gradient->stops();
    int stopY = BAR_HEIGHT + 5;
    
    for (size_t i = 0; i < stops.size(); ++i)
    {
        int x = positionToX(stops[i].position);
        QRect rect(x - STOP_SIZE/2, stopY, STOP_SIZE, STOP_SIZE);
        if (rect.contains(pos))
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int GradientBarWidget::hitTestMidpoint(const QPoint& pos) const
{
    if (!m_gradient) return -1;
    
    const auto& stops = m_gradient->stops();
    const auto& midpoints = m_gradient->midpoints();
    int midY = BAR_HEIGHT + 5 + STOP_SIZE + 5;
    
    for (size_t i = 0; i < midpoints.size() && i + 1 < stops.size(); ++i)
    {
        float startPos = stops[i].position;
        float endPos = stops[i + 1].position;
        float midPos = startPos + (endPos - startPos) * midpoints[i].position;
        
        int x = positionToX(midPos);
        QRect rect(x - MIDPOINT_SIZE/2, midY, MIDPOINT_SIZE, MIDPOINT_SIZE);
        if (rect.contains(pos))
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void GradientBarWidget::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    if (!m_gradient)
    {
        painter.fillRect(rect(), Qt::gray);
        return;
    }
    
    // Draw gradient bar - sample pixel by pixel to respect midpoints
    QRect barRect(MARGIN, 5, width() - 2 * MARGIN, BAR_HEIGHT);
    int barWidth = barRect.width();
    
    for (int x = 0; x < barWidth; ++x)
    {
        float t = static_cast<float>(x) / static_cast<float>(barWidth);
        auto c = m_gradient->sample(t);  // This respects midpoints!
        QColor color = QColor::fromRgbF(
            std::clamp(c[0], 0.0f, 1.0f),
            std::clamp(c[1], 0.0f, 1.0f),
            std::clamp(c[2], 0.0f, 1.0f),
            std::clamp(c[3], 0.0f, 1.0f));
        painter.setPen(color);
        painter.drawLine(barRect.x() + x, barRect.y(), 
                         barRect.x() + x, barRect.y() + barRect.height());
    }
    
    // Draw border
    painter.setPen(Qt::black);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(barRect);
    
    // Draw color stops (triangles pointing up)
    const auto& stops = m_gradient->stops();
    int stopY = BAR_HEIGHT + 5;
    for (size_t i = 0; i < stops.size(); ++i)
    {
        int x = positionToX(stops[i].position);
        const auto& c = stops[i].color;
        QColor color = QColor::fromRgbF(c[0], c[1], c[2], c[3]);
        
        // Triangle pointing up
        QPolygon triangle;
        triangle << QPoint(x, stopY)
                 << QPoint(x - STOP_SIZE/2, stopY + STOP_SIZE)
                 << QPoint(x + STOP_SIZE/2, stopY + STOP_SIZE);
        
        painter.setBrush(color);
        painter.setPen(static_cast<int>(i) == m_selectedStop ? Qt::red : Qt::black);
        painter.drawPolygon(triangle);
    }
    
    // Draw midpoints (diamonds)
    const auto& midpoints = m_gradient->midpoints();
    int midY = BAR_HEIGHT + 5 + STOP_SIZE + 8;
    
    for (size_t i = 0; i < midpoints.size() && i + 1 < stops.size(); ++i)
    {
        float startPos = stops[i].position;
        float endPos = stops[i + 1].position;
        float midPos = startPos + (endPos - startPos) * midpoints[i].position;
        
        int x = positionToX(midPos);
        
        // Diamond shape
        QPolygon diamond;
        diamond << QPoint(x, midY - MIDPOINT_SIZE/2)
                << QPoint(x + MIDPOINT_SIZE/2, midY)
                << QPoint(x, midY + MIDPOINT_SIZE/2)
                << QPoint(x - MIDPOINT_SIZE/2, midY);
        
        painter.setBrush(Qt::white);
        painter.setPen(static_cast<int>(i) == m_selectedMidpoint ? Qt::red : Qt::darkGray);
        painter.drawPolygon(diamond);
    }
}

void GradientBarWidget::mousePressEvent(QMouseEvent* event)
{
    if (!m_gradient) return;
    
    // Check for stop hit
    int stopIndex = hitTestStop(event->pos());
    if (stopIndex >= 0)
    {
        m_selectedStop = stopIndex;
        m_selectedMidpoint = -1;
        m_dragging = true;
        emit stopSelected(stopIndex);
        update();
        return;
    }
    
    // Check for midpoint hit
    int midIndex = hitTestMidpoint(event->pos());
    if (midIndex >= 0)
    {
        m_selectedMidpoint = midIndex;
        m_selectedStop = -1;
        m_dragging = true;
        update();
        return;
    }
    
    // Deselect
    m_selectedStop = -1;
    m_selectedMidpoint = -1;
    emit stopSelected(-1);
    update();
}

void GradientBarWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_gradient || !m_dragging) return;
    
    float pos = xToPosition(event->pos().x());
    
    if (m_selectedStop >= 0)
    {
        // Don't move first/last stops past each other
        const auto& stops = m_gradient->stops();
        if (m_selectedStop == 0)
        {
            pos = std::min(pos, stops.size() > 1 ? stops[1].position - 0.01f : 1.0f);
        }
        else if (m_selectedStop == static_cast<int>(stops.size()) - 1)
        {
            pos = std::max(pos, stops.size() > 1 ? stops[stops.size() - 2].position + 0.01f : 0.0f);
        }
        else
        {
            // Keep between neighbors
            pos = std::clamp(pos, 
                             stops[m_selectedStop - 1].position + 0.01f,
                             stops[m_selectedStop + 1].position - 0.01f);
        }
        
        m_gradient->updateStop(m_selectedStop, pos, stops[m_selectedStop].color);
        emit stopMoved(m_selectedStop, pos);
        emit gradientChanged();
        update();
    }
    else if (m_selectedMidpoint >= 0)
    {
        // Calculate relative position within segment
        const auto& stops = m_gradient->stops();
        if (m_selectedMidpoint + 1 < static_cast<int>(stops.size()))
        {
            float startPos = stops[m_selectedMidpoint].position;
            float endPos = stops[m_selectedMidpoint + 1].position;
            float segmentLength = endPos - startPos;
            
            if (segmentLength > 0.001f)
            {
                float relativePos = (pos - startPos) / segmentLength;
                relativePos = std::clamp(relativePos, 0.1f, 0.9f);
                m_gradient->setMidpoint(m_selectedMidpoint, relativePos);
                emit midpointMoved(m_selectedMidpoint, relativePos);
                emit gradientChanged();
                update();
            }
        }
    }
}

void GradientBarWidget::mouseReleaseEvent(QMouseEvent* /*event*/)
{
    m_dragging = false;
}

void GradientBarWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (!m_gradient) return;
    
    // Double-click on stop to edit color
    int stopIndex = hitTestStop(event->pos());
    if (stopIndex >= 0)
    {
        m_selectedStop = stopIndex;
        emit stopColorChanged(stopIndex);
        return;
    }
    
    // Double-click on bar to add new stop (max 4 stops for shader)
    if (m_gradient->stopCount() >= 8)
    {
        return;  // Silently ignore - limit reached
    }
    
    QRect barRect(MARGIN, 5, width() - 2 * MARGIN, BAR_HEIGHT);
    if (barRect.contains(event->pos()))
    {
        float pos = xToPosition(event->pos().x());
        
        // Sample color at this position
        auto color = m_gradient->sample(pos);
        m_gradient->addStop(pos, color);
        emit gradientChanged();
        update();
    }
}

// =============================================================================
// GradientEditorDialog Implementation
// =============================================================================

GradientEditorDialog::GradientEditorDialog(modules::ColorGradientModule* gradient,
                                           QWidget* parent)
    : QDialog(parent)
    , m_gradient(gradient)
{
    setWindowTitle(tr("Gradient Editor"));
    setMinimumSize(450, 300);
    setupUi();
    updatePresetCombo();
    updateStopControls();  // Initialize button states (add button limit)
}

void GradientEditorDialog::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    
    // Gradient bar
    m_gradientBar = new GradientBarWidget(this);
    m_gradientBar->setGradient(m_gradient);
    mainLayout->addWidget(m_gradientBar);
    
    // Instructions
    auto* instructionLabel = new QLabel(
        tr("Double-click on bar to add stop. Drag stops to move. "
           "Double-click stop to change color."), this);
    instructionLabel->setWordWrap(true);
    instructionLabel->setStyleSheet("color: gray; font-size: 10px;");
    mainLayout->addWidget(instructionLabel);
    
    // Stop controls group
    auto* stopGroup = new QGroupBox(tr("Selected Stop"), this);
    auto* stopLayout = new QHBoxLayout(stopGroup);
    
    m_colorButton = new QPushButton(tr("Change Color"), this);
    m_colorButton->setEnabled(false);
    stopLayout->addWidget(m_colorButton);
    
    m_positionLabel = new QLabel(tr("Position: --"), this);
    stopLayout->addWidget(m_positionLabel);
    
    m_addButton = new QPushButton(tr("+ Add (2/8)"), this);
    m_addButton->setToolTip(tr("Add a color stop (max 4 for GPU shader)"));
    m_addButton->setMinimumWidth(100);
    stopLayout->addWidget(m_addButton);
    
    m_removeButton = new QPushButton(tr("- Remove"), this);
    m_removeButton->setEnabled(false);
    stopLayout->addWidget(m_removeButton);
    
    stopLayout->addStretch();
    mainLayout->addWidget(stopGroup);
    
    // Midpoint control
    auto* midpointGroup = new QGroupBox(tr("Midpoint"), this);
    auto* midpointLayout = new QHBoxLayout(midpointGroup);
    
    m_midpointLabel = new QLabel(tr("Position: 50%"), this);
    midpointLayout->addWidget(m_midpointLabel);
    
    m_midpointSlider = new QSlider(Qt::Horizontal, this);
    m_midpointSlider->setRange(10, 90);
    m_midpointSlider->setValue(50);
    m_midpointSlider->setEnabled(false);
    midpointLayout->addWidget(m_midpointSlider, 1);
    
    mainLayout->addWidget(midpointGroup);
    
    // Preset controls
    auto* presetGroup = new QGroupBox(tr("Presets"), this);
    auto* presetLayout = new QHBoxLayout(presetGroup);
    
    m_presetCombo = new QComboBox(this);
    presetLayout->addWidget(m_presetCombo, 1);
    
    m_savePresetButton = new QPushButton(tr("Save..."), this);
    presetLayout->addWidget(m_savePresetButton);
    
    m_resetButton = new QPushButton(tr("Reset"), this);
    presetLayout->addWidget(m_resetButton);
    
    mainLayout->addWidget(presetGroup);
    
    // Dialog buttons
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    auto* okButton = new QPushButton(tr("OK"), this);
    okButton->setDefault(true);
    buttonLayout->addWidget(okButton);
    
    auto* cancelButton = new QPushButton(tr("Cancel"), this);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(m_gradientBar, &GradientBarWidget::stopSelected,
            this, &GradientEditorDialog::onStopSelected);
    connect(m_gradientBar, &GradientBarWidget::stopColorChanged,
            this, &GradientEditorDialog::onColorButtonClicked);
    connect(m_gradientBar, &GradientBarWidget::gradientChanged,
            this, [this]() {
                updateStopControls();  // Update add button state
                notifyChange();
            });
    
    connect(m_colorButton, &QPushButton::clicked,
            this, &GradientEditorDialog::onColorButtonClicked);
    connect(m_addButton, &QPushButton::clicked,
            this, &GradientEditorDialog::onAddStopClicked);
    connect(m_removeButton, &QPushButton::clicked,
            this, &GradientEditorDialog::onRemoveStopClicked);
    
    connect(m_midpointSlider, &QSlider::valueChanged,
            this, &GradientEditorDialog::onMidpointSliderChanged);
    
    connect(m_presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GradientEditorDialog::onPresetChanged);
    connect(m_savePresetButton, &QPushButton::clicked,
            this, &GradientEditorDialog::onSavePresetClicked);
    connect(m_resetButton, &QPushButton::clicked,
            this, &GradientEditorDialog::onResetClicked);
    
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void GradientEditorDialog::updateStopControls()
{
    bool hasStop = m_currentStop >= 0;
    m_colorButton->setEnabled(hasStop);
    m_removeButton->setEnabled(hasStop && m_gradient->stopCount() > 2);
    
    // Disable add button when max stops reached (shader limit = 4)
    bool atLimit = m_gradient->stopCount() >= 8;
    m_addButton->setEnabled(!atLimit);
    
    // Update button text to show limit status
    if (atLimit)
    {
        m_addButton->setText(tr("+ Add (Max 8)"));
        m_addButton->setStyleSheet("color: #888;");
    }
    else
    {
        m_addButton->setText(tr("+ Add (%1/8)").arg(m_gradient->stopCount()));
        m_addButton->setStyleSheet("");
    }
    
    if (hasStop && m_currentStop < static_cast<int>(m_gradient->stops().size()))
    {
        const auto& stop = m_gradient->stops()[m_currentStop];
        m_positionLabel->setText(tr("Position: %1%").arg(
            static_cast<int>(stop.position * 100)));
        
        // Update color button background
        const auto& c = stop.color;
        QString style = QString("background-color: rgb(%1, %2, %3);")
            .arg(static_cast<int>(c[0] * 255))
            .arg(static_cast<int>(c[1] * 255))
            .arg(static_cast<int>(c[2] * 255));
        m_colorButton->setStyleSheet(style);
        
        // Enable midpoint slider if there's a next stop
        if (m_currentStop < static_cast<int>(m_gradient->midpoints().size()))
        {
            m_midpointSlider->setEnabled(true);
            float mid = m_gradient->midpoint(m_currentStop);
            m_midpointSlider->blockSignals(true);
            m_midpointSlider->setValue(static_cast<int>(mid * 100));
            m_midpointSlider->blockSignals(false);
            m_midpointLabel->setText(tr("Position: %1%").arg(static_cast<int>(mid * 100)));
        }
        else
        {
            m_midpointSlider->setEnabled(false);
        }
    }
    else
    {
        m_positionLabel->setText(tr("Position: --"));
        m_colorButton->setStyleSheet("");
        m_midpointSlider->setEnabled(false);
    }
}

void GradientEditorDialog::updatePresetCombo()
{
    m_presetCombo->blockSignals(true);
    m_presetCombo->clear();
    m_presetCombo->addItem(tr("(Custom)"));
    
    for (const auto& name : m_gradient->presetNames())
    {
        m_presetCombo->addItem(QString::fromStdString(name));
    }
    m_presetCombo->blockSignals(false);
}

void GradientEditorDialog::notifyChange()
{
    if (m_changeCallback)
    {
        m_changeCallback();
    }
    emit gradientChanged();
}

void GradientEditorDialog::onStopSelected(int index)
{
    m_currentStop = index;
    updateStopControls();
}

void GradientEditorDialog::onColorButtonClicked()
{
    if (m_currentStop < 0 || m_currentStop >= static_cast<int>(m_gradient->stops().size()))
    {
        return;
    }
    
    const auto& stop = m_gradient->stops()[m_currentStop];
    QColor initial = QColor::fromRgbF(stop.color[0], stop.color[1], 
                                       stop.color[2], stop.color[3]);
    
    QColor color = QColorDialog::getColor(initial, this, tr("Select Color"),
                                          QColorDialog::ShowAlphaChannel);
    if (color.isValid())
    {
        modules::Color4f newColor = {
            static_cast<float>(color.redF()),
            static_cast<float>(color.greenF()),
            static_cast<float>(color.blueF()),
            static_cast<float>(color.alphaF())
        };
        m_gradient->updateStop(m_currentStop, stop.position, newColor);
        m_gradientBar->updateDisplay();
        updateStopControls();
        notifyChange();
    }
}

void GradientEditorDialog::onAddStopClicked()
{
    // Shader only supports 4 color stops - block adding more
    if (m_gradient->stopCount() >= 8)
    {
        QMessageBox::warning(this, tr("Stop Limit Reached"),
            tr("The GPU shader supports a maximum of 8 color stops.\n"
               "Remove an existing stop before adding a new one."));
        return;
    }
    
    // Add a new stop at 0.5 with interpolated color
    float pos = 0.5f;
    auto color = m_gradient->sample(pos);
    m_gradient->addStop(pos, color);
    m_gradientBar->updateDisplay();
    updateStopControls();  // Update add button state
    notifyChange();
}

void GradientEditorDialog::onRemoveStopClicked()
{
    if (m_currentStop >= 0 && m_gradient->stopCount() > 2)
    {
        m_gradient->removeStop(m_currentStop);
        m_currentStop = -1;
        m_gradientBar->updateDisplay();
        updateStopControls();
        notifyChange();
    }
}

void GradientEditorDialog::onPresetChanged(int index)
{
    if (index <= 0) return;  // Skip "(Custom)"
    
    auto names = m_gradient->presetNames();
    if (index - 1 < static_cast<int>(names.size()))
    {
        m_gradient->loadPreset(names[index - 1]);
        m_gradientBar->updateDisplay();
        m_currentStop = -1;
        updateStopControls();
        notifyChange();
    }
}

void GradientEditorDialog::onSavePresetClicked()
{
    bool ok;
    QString name = QInputDialog::getText(this, tr("Save Preset"),
                                         tr("Preset name:"), QLineEdit::Normal,
                                         "", &ok);
    if (ok && !name.isEmpty())
    {
        m_gradient->savePreset(name.toStdString());
        updatePresetCombo();
        
        // Select the new preset
        int index = m_presetCombo->findText(name);
        if (index >= 0)
        {
            m_presetCombo->setCurrentIndex(index);
        }
    }
}

void GradientEditorDialog::onResetClicked()
{
    m_gradient->reset();
    m_gradientBar->updateDisplay();
    m_currentStop = -1;
    m_presetCombo->setCurrentIndex(0);
    updateStopControls();
    notifyChange();
}

void GradientEditorDialog::onMidpointSliderChanged(int value)
{
    if (m_currentStop >= 0 && m_currentStop < static_cast<int>(m_gradient->midpoints().size()))
    {
        float pos = static_cast<float>(value) / 100.0f;
        m_gradient->setMidpoint(m_currentStop, pos);
        m_midpointLabel->setText(tr("Position: %1%").arg(value));
        m_gradientBar->updateDisplay();
        notifyChange();
    }
}

} // namespace lumi::ui
