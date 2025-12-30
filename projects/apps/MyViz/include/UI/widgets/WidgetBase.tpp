/**
 ****************************************************************************************
 * @file   WidgetBase.tpp
 * @brief  WidgetBase template implementation
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.0.0
 *
 * @note This file is included at the end of WidgetBase.hpp
 *       Do not include directly!
 ****************************************************************************************
 */

#pragma once

#include "services/ServiceContainer.hpp"
#include "services/IEventBus.hpp"

#include <QShowEvent>
#include <QHideEvent>

// =============================================================================
// Construction
// =============================================================================

template<typename BaseWidget>
WidgetBase<BaseWidget>::WidgetBase(ServiceContainer& services,
                                    const QString& id,
                                    const QString& name,
                                    QWidget* parent)
    : BaseWidget(parent)
    , m_services(services)
    , m_widgetId(id)
    , m_name(name)
{
    // Set object name for debugging/styling
    BaseWidget::setObjectName(id);
}

// =============================================================================
// Update Control
// =============================================================================

template<typename BaseWidget>
void WidgetBase<BaseWidget>::startUpdates()
{
    if (m_isUpdating)
    {
        return;
    }

    m_isUpdating = true;
    onStartUpdates();
}

template<typename BaseWidget>
void WidgetBase<BaseWidget>::stopUpdates()
{
    if (!m_isUpdating)
    {
        return;
    }

    m_isUpdating = false;
    onStopUpdates();
}

// =============================================================================
// Protected Accessors
// =============================================================================

template<typename BaseWidget>
IEventBus* WidgetBase<BaseWidget>::eventBus()
{
    return m_services.tryResolve<IEventBus>();
}

// =============================================================================
// Event Handlers
// =============================================================================

template<typename BaseWidget>
void WidgetBase<BaseWidget>::showEvent(QShowEvent* event)
{
    BaseWidget::showEvent(event);
    startUpdates();
}

template<typename BaseWidget>
void WidgetBase<BaseWidget>::hideEvent(QHideEvent* event)
{
    stopUpdates();
    BaseWidget::hideEvent(event);
}
