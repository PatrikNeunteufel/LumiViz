/**
 ****************************************************************************************
 * @file   PanelBase.cpp
 * @brief  PanelBase implementation
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 ****************************************************************************************
 */

#include "UI/panels/PanelBase.hpp"
#include "services/ServiceContainer.hpp"
#include "services/IEventBus.hpp"

#include <QShowEvent>
#include <QHideEvent>
#include <QSettings>

// =============================================================================
// Construction
// =============================================================================

PanelBase::PanelBase(ServiceContainer& services,
                     const QString& id,
                     const QString& title,
                     QWidget* parent)
    : QWidget(parent)
    , m_services(services)
    , m_panelId(id)
    , m_title(title)
{
    setObjectName(id);
}

// =============================================================================
// IPanel Implementation
// =============================================================================

void PanelBase::saveState()
{
    QSettings settings;
    settings.beginGroup(settingsPrefix());
    
    // Save geometry
    settings.setValue("geometry", saveGeometry());
    
    settings.endGroup();
}

void PanelBase::restoreState()
{
    QSettings settings;
    settings.beginGroup(settingsPrefix());
    
    // Restore geometry
    QByteArray geometry = settings.value("geometry").toByteArray();
    if (!geometry.isEmpty())
    {
        restoreGeometry(geometry);
    }
    
    settings.endGroup();
}

// =============================================================================
// Public Slots
// =============================================================================

void PanelBase::setActive(bool active)
{
    if (m_isActive == active)
    {
        return;
    }

    m_isActive = active;

    if (active)
    {
        onActivate();
        Q_EMIT activated();
    }
    else
    {
        onDeactivate();
        Q_EMIT deactivated();
    }
}

// =============================================================================
// Protected Methods
// =============================================================================

IEventBus* PanelBase::eventBus() const
{
    return m_services.tryResolve<IEventBus>();
}

QString PanelBase::settingsPrefix() const
{
    return QString("panels/%1/").arg(m_panelId);
}

// =============================================================================
// Event Handlers
// =============================================================================

void PanelBase::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    setActive(true);
}

void PanelBase::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    setActive(false);
}
