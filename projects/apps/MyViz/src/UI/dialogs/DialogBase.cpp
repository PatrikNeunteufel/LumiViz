/**
 ****************************************************************************************
 * @file   DialogBase.cpp
 * @brief  DialogBase implementation
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 ****************************************************************************************
 */

#include "UI/dialogs/DialogBase.hpp"
#include "services/ServiceContainer.hpp"
#include "services/IEventBus.hpp"

#include <QScreen>
#include <QGuiApplication>
#include <QShowEvent>

// =============================================================================
// Construction
// =============================================================================

DialogBase::DialogBase(ServiceContainer& services,
                       const QString& title,
                       QWidget* parent)
    : QDialog(parent)
    , m_services(services)
{
    setWindowTitle(title);
    
    // Default dialog settings
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setSizeGripEnabled(true);
}

// =============================================================================
// Public Methods
// =============================================================================

QString DialogBase::dialogId() const
{
    return QString::fromUtf8(metaObject()->className());
}

// =============================================================================
// Protected Methods
// =============================================================================

IEventBus* DialogBase::eventBus() const
{
    return m_services.tryResolve<IEventBus>();
}

void DialogBase::centerOnParent()
{
    if (parentWidget() != nullptr)
    {
        // Center on parent
        QPoint parentCenter = parentWidget()->geometry().center();
        QPoint dialogPos = parentCenter - QPoint(width() / 2, height() / 2);
        move(dialogPos);
    }
    else
    {
        // Center on screen
        QScreen* screen = QGuiApplication::primaryScreen();
        if (screen != nullptr)
        {
            QRect screenGeometry = screen->availableGeometry();
            QPoint screenCenter = screenGeometry.center();
            QPoint dialogPos = screenCenter - QPoint(width() / 2, height() / 2);
            move(dialogPos);
        }
    }
}

void DialogBase::setRelativeMinimumSize(double widthRatio, double heightRatio)
{
    QSize referenceSize;
    
    if (parentWidget() != nullptr)
    {
        referenceSize = parentWidget()->size();
    }
    else
    {
        QScreen* screen = QGuiApplication::primaryScreen();
        if (screen != nullptr)
        {
            referenceSize = screen->availableGeometry().size();
        }
        else
        {
            referenceSize = QSize(1920, 1080);  // Fallback
        }
    }
    
    int minWidth = static_cast<int>(referenceSize.width() * widthRatio);
    int minHeight = static_cast<int>(referenceSize.height() * heightRatio);
    
    setMinimumSize(minWidth, minHeight);
}

// =============================================================================
// Event Handlers
// =============================================================================

void DialogBase::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    centerOnParent();
}
