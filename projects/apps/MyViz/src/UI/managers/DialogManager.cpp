/**
 ****************************************************************************************
 * @file   DialogManager.cpp
 * @brief  DialogManager implementation
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 ****************************************************************************************
 */

#include "UI/managers/DialogManager.hpp"
#include "services/DialogRegistry.hpp"
#include "services/ServiceContainer.hpp"
#include "services/IEventBus.hpp"
#include "services/events/UIEvents.hpp"

#include <QDialog>

// =============================================================================
// Construction
// =============================================================================

DialogManager::DialogManager(ServiceContainer& services, QWidget* parent)
    : QObject(parent)
    , m_services(services)
    , m_parentWidget(parent)
{
}

DialogManager::~DialogManager()
{
    // Unsubscribe from EventBus
    if (m_eventSubscriptionId != 0)
    {
        if (auto* eventBus = m_services.tryResolve<IEventBus>())
        {
            eventBus->unsubscribe(m_eventSubscriptionId);
        }
    }

    closeAll();
}

// =============================================================================
// Dialog Display
// =============================================================================

int DialogManager::show(const QString& dialogId)
{
    auto& registry = DialogRegistry::instance();
    std::string idStd = dialogId.toStdString();

    // Check if registered
    if (!registry.has(idStd))
    {
        return QDialog::Rejected;
    }

    // Get descriptor
    const auto* desc = registry.descriptor(idStd);
    if (desc == nullptr)
    {
        return QDialog::Rejected;
    }

    // Create dialog
    auto dialog = registry.create(idStd, m_services, m_parentWidget);
    if (!dialog)
    {
        return QDialog::Rejected;
    }

    Q_EMIT dialogOpened(dialogId);

    int result = QDialog::Rejected;

    if (desc->modal)
    {
        // Modal: exec() blocks until closed
        result = dialog->exec();
    }
    else
    {
        // Store for modeless tracking
        m_openDialogs.insert(dialogId, dialog.get());

        // Connect to track when closed
        connect(dialog.get(), &QDialog::finished,
                this, &DialogManager::onDialogFinished);

        dialog->show();
        dialog.release();  // Dialog manages its own lifetime

        result = QDialog::Accepted;
    }

    Q_EMIT dialogClosed(dialogId, result);

    return result;
}

QDialog* DialogManager::showModeless(const QString& dialogId)
{
    // Check if already open
    if (isOpen(dialogId))
    {
        QPointer<QDialog> existing = m_openDialogs.value(dialogId);
        if (!existing.isNull())
        {
            existing->raise();
            existing->activateWindow();
            return existing.data();
        }
    }

    auto& registry = DialogRegistry::instance();
    std::string idStd = dialogId.toStdString();

    // Create dialog
    auto dialog = registry.create(idStd, m_services, m_parentWidget);
    if (!dialog)
    {
        return nullptr;
    }

    QDialog* dialogPtr = dialog.release();

    // Store reference
    m_openDialogs.insert(dialogId, dialogPtr);

    // Connect to track when closed
    connect(dialogPtr, &QDialog::finished,
            this, &DialogManager::onDialogFinished);

    // Set to delete on close
    dialogPtr->setAttribute(Qt::WA_DeleteOnClose);

    Q_EMIT dialogOpened(dialogId);

    dialogPtr->show();
    return dialogPtr;
}

bool DialogManager::isOpen(const QString& dialogId) const
{
    auto it = m_openDialogs.find(dialogId);
    if (it == m_openDialogs.end())
    {
        return false;
    }

    return !it.value().isNull();
}

void DialogManager::close(const QString& dialogId)
{
    auto it = m_openDialogs.find(dialogId);
    if (it != m_openDialogs.end() && !it.value().isNull())
    {
        it.value()->close();
    }
}

void DialogManager::closeAll()
{
    for (auto it = m_openDialogs.begin(); it != m_openDialogs.end(); ++it)
    {
        if (!it.value().isNull())
        {
            it.value()->close();
        }
    }
    m_openDialogs.clear();
}

// =============================================================================
// EventBus Integration
// =============================================================================

void DialogManager::subscribeToEvents()
{
    auto* eventBus = m_services.tryResolve<IEventBus>();
    if (eventBus == nullptr)
    {
        return;
    }

    m_eventSubscriptionId = eventBus->subscribe<OpenDialogEvent>(
        [this](const OpenDialogEvent& event) {
            show(QString::fromStdString(event.dialogId));
        });
}

// =============================================================================
// Private Slots
// =============================================================================

void DialogManager::onDialogFinished(int result)
{
    auto* dialog = qobject_cast<QDialog*>(sender());
    if (dialog == nullptr)
    {
        return;
    }

    // Find dialog ID
    QString dialogId = m_openDialogs.key(dialog);
    if (!dialogId.isEmpty())
    {
        m_openDialogs.remove(dialogId);
        Q_EMIT dialogClosed(dialogId, result);
    }
}
