/**
 ****************************************************************************************
 * @file   WidgetBase.cpp
 * @brief  WidgetBase implementation
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 ****************************************************************************************
 */

#include "UI/widgets/WidgetBase.hpp"
#include "services/ServiceContainer.hpp"
#include "services/IEventBus.hpp"

#include <QShowEvent>
#include <QHideEvent>

// =============================================================================
// Construction
// =============================================================================

WidgetBase::WidgetBase(ServiceContainer& services,
                       const QString& id,
                       const QString& name,
                       QWidget* parent)
    : QWidget(parent)
    , m_services(services)
    , m_widgetId(id)
    , m_name(name)
{
    setObjectName(id);
}

// =============================================================================
// Public Slots
// =============================================================================

void WidgetBase::start()
{
    if (m_isUpdating)
    {
        return;
    }

    m_isUpdating = true;
    startUpdates();
    Q_EMIT updatesStarted();
}

void WidgetBase::stop()
{
    if (!m_isUpdating)
    {
        return;
    }

    m_isUpdating = false;
    stopUpdates();
    Q_EMIT updatesStopped();
}

// =============================================================================
// Protected Methods
// =============================================================================

IEventBus* WidgetBase::eventBus() const
{
    return m_services.tryResolve<IEventBus>();
}

// =============================================================================
// Event Handlers
// =============================================================================

void WidgetBase::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    start();
}

void WidgetBase::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    stop();
}
