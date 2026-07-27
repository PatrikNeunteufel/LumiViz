/**
 ****************************************************************************************
 * @file   ShortcutManager.cpp
 * @brief  ShortcutManager implementation
 *
 * @author Patrik Neunteufel
 * @date   July 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#include "UI/managers/ShortcutManager.hpp"

#include "audio/AudioEvents.hpp"
#include "services/IEventBus.hpp"
#include "services/ServiceContainer.hpp"
#include "services/events/UIEvents.hpp"

#include <BasicLogger.h>

#include <QAbstractSpinBox>
#include <QApplication>
#include <QComboBox>
#include <QEvent>
#include <QKeyEvent>
#include <QKeySequenceEdit>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QSettings>
#include <QTextEdit>

namespace {

/// Kanonische Textform — dieselbe Schreibweise wie die Registry-Vorbelegungen.
QString canonical(const QKeySequence& seq)
{
    return seq.toString(QKeySequence::PortableText);
}

QString settingsKey(const QString& actionId)
{
    return QStringLiteral("shortcuts/") + actionId;
}

}  // namespace

// =============================================================================
// Construction
// =============================================================================

ShortcutManager::ShortcutManager(ServiceContainer& services, QObject* parent)
    : QObject(parent)
    , m_services(services)
{
    loadOverrides();
    if (qApp != nullptr) qApp->installEventFilter(this);

    // Aktive Belegung ins Log — ohne das ist ein "der Hotkey geht nicht" nicht
    // von "der Manager existiert nicht" zu unterscheiden.
    std::string bindings;
    for (auto it = m_sequences.constBegin(); it != m_sequences.constEnd(); ++it)
    {
        const auto* action = lumi::services::shortcutAction(it.key().toStdString());
        if (action == nullptr || !action->wired) continue;
        if (!bindings.empty()) bindings += ", ";
        bindings += it.key().toStdString() + "=" +
                    it.value().toString(QKeySequence::PortableText).toStdString();
    }
    BasicLogger::logInfo("ShortcutManager: filter installed, active: " + bindings);
}

ShortcutManager::~ShortcutManager()
{
    if (qApp != nullptr) qApp->removeEventFilter(this);
}

void ShortcutManager::loadOverrides()
{
    m_sequences.clear();
    const QSettings settings;
    for (const lumi::services::ShortcutAction& action : lumi::services::shortcutActions())
    {
        const QString id = QString::fromUtf8(action.id.data(),
                                             static_cast<int>(action.id.size()));
        // Fehlende Einstellung = Vorbelegung (bewusst NICHT beim ersten Start
        // ausgeschrieben, damit eine neue Vorbelegung noch greifen kann).
        const QVariant stored = settings.value(settingsKey(id));
        m_sequences.insert(id, stored.isValid()
                                   ? QKeySequence(stored.toString(),
                                                  QKeySequence::PortableText)
                                   : defaultSequenceFor(id));
    }
}

// =============================================================================
// Query / mutate
// =============================================================================

QKeySequence ShortcutManager::sequenceFor(const QString& actionId) const
{
    return m_sequences.value(actionId);
}

QKeySequence ShortcutManager::defaultSequenceFor(const QString& actionId)
{
    const auto* action = lumi::services::shortcutAction(actionId.toStdString());
    if (action == nullptr) return {};
    return QKeySequence(QString::fromUtf8(action->defaultSequence.data(),
                                          static_cast<int>(
                                              action->defaultSequence.size())),
                        QKeySequence::PortableText);
}

QString ShortcutManager::actionForSequence(const QKeySequence& sequence) const
{
    if (sequence.isEmpty()) return {};
    for (auto it = m_sequences.constBegin(); it != m_sequences.constEnd(); ++it)
    {
        if (it.value() == sequence) return it.key();
    }
    return {};
}

QString ShortcutManager::setSequence(const QString& actionId,
                                     const QKeySequence& sequence)
{
    if (!m_sequences.contains(actionId)) return QObject::tr("Unknown action");

    if (!sequence.isEmpty())
    {
        // Kollision: kein stilles Ueberschreiben — wer tauschen will, macht die
        // andere Zuweisung zuerst frei (Konzept §6).
        const QString holder = actionForSequence(sequence);
        if (!holder.isEmpty() && holder != actionId)
        {
            const auto* other = lumi::services::shortcutAction(holder.toStdString());
            return QObject::tr("Already assigned to \"%1\"")
                .arg(other != nullptr
                         ? QString::fromUtf8(other->label.data(),
                                             static_cast<int>(other->label.size()))
                         : holder);
        }
        // Reservierung: Transporttasten gehoeren dem Player (§2).
        if (lumi::services::shortcutSequenceReservedFor(
                canonical(sequence).toStdString(), actionId.toStdString()))
        {
            return QObject::tr("Reserved for playback control");
        }
    }

    m_sequences.insert(actionId, sequence);
    QSettings settings;
    if (sequence == defaultSequenceFor(actionId))
    {
        settings.remove(settingsKey(actionId));  // nur Abweichungen speichern
    }
    else
    {
        settings.setValue(settingsKey(actionId), canonical(sequence));
    }
    return {};
}

void ShortcutManager::resetToDefault(const QString& actionId)
{
    if (!m_sequences.contains(actionId)) return;
    m_sequences.insert(actionId, defaultSequenceFor(actionId));
    QSettings settings;
    settings.remove(settingsKey(actionId));
}

void ShortcutManager::resetAllToDefaults()
{
    QSettings settings;
    for (auto it = m_sequences.begin(); it != m_sequences.end(); ++it)
    {
        it.value() = defaultSequenceFor(it.key());
        settings.remove(settingsKey(it.key()));
    }
}

// =============================================================================
// Focus guard + filter
// =============================================================================

bool ShortcutManager::acceptsTextInput(const QWidget* widget)
{
    if (widget == nullptr) return false;
    if (const auto* line = qobject_cast<const QLineEdit*>(widget))
        return !line->isReadOnly();
    if (const auto* plain = qobject_cast<const QPlainTextEdit*>(widget))
        return !plain->isReadOnly();
    if (const auto* text = qobject_cast<const QTextEdit*>(widget))
        return !text->isReadOnly();
    if (qobject_cast<const QAbstractSpinBox*>(widget) != nullptr) return true;
    if (const auto* combo = qobject_cast<const QComboBox*>(widget))
        return combo->isEditable();
    // Das Aufnahmefeld des Editors muss JEDE Taste bekommen, auch Ctrl-Kombis.
    if (qobject_cast<const QKeySequenceEdit*>(widget) != nullptr) return true;
    return false;
}

bool ShortcutManager::shouldIgnoreWhileTyping(const QKeySequence& sequence) const
{
    if (qApp == nullptr) return false;
    const QWidget* focus = QApplication::focusWidget();
    if (!acceptsTextInput(focus)) return false;
    // Im Vollbild rendert ein eigenes QWindow, das KEIN Widget-Fokus setzt —
    // focusWidget() liefert dann noch das zuletzt fokussierte Feld des
    // Hauptfensters (z. B. das Suchfeld des Browsers). Waere das der Maßstab,
    // wuerden die Hotkeys genau dort versagen, wo sie am meisten gebraucht
    // werden. Der Schutz gilt deshalb nur, wenn das Feld wirklich sichtbar ist
    // UND in dem Fenster liegt, das die Tasten bekommt.
    if (!focus->isVisible()) return false;
    const QWidget* window = focus->window();
    if (window != nullptr && !window->isActiveWindow()) return false;
    // Im Textfeld bleibt nur Platz fuer modifizierte Kombinationen — eine
    // unmodifizierte Taste (oder Shift+Taste) gehoert dort dem Text.
    // Ausnahme: das Aufnahmefeld schluckt grundsaetzlich alles.
    if (qobject_cast<const QKeySequenceEdit*>(focus) != nullptr) return true;
    if (sequence.isEmpty()) return true;
    const int key = sequence[0].toCombined();
    const int mods = key & static_cast<int>(Qt::KeyboardModifierMask);
    const int relevant = mods & ~static_cast<int>(Qt::ShiftModifier);
    return relevant == 0;
}

bool ShortcutManager::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() != QEvent::KeyPress) return QObject::eventFilter(watched, event);

    auto* keyEvent = static_cast<QKeyEvent*>(event);
    // Reine Modifikator-Druecke ergeben keine Sequenz.
    switch (keyEvent->key())
    {
        case Qt::Key_Shift:
        case Qt::Key_Control:
        case Qt::Key_Alt:
        case Qt::Key_Meta:
            return QObject::eventFilter(watched, event);
        default:
            break;
    }

    const QKeySequence pressed(keyEvent->keyCombination());
    const QString actionId = actionForSequence(pressed);
    if (actionId.isEmpty()) return QObject::eventFilter(watched, event);

    const auto* action = lumi::services::shortcutAction(actionId.toStdString());
    if (action == nullptr || !action->wired)
    {
        // Registriert und reserviert, aber ohne Wirkung (Stufe 2): die Taste
        // wird NICHT verschluckt, damit sie sich normal verhaelt.
        return QObject::eventFilter(watched, event);
    }
    if (shouldIgnoreWhileTyping(pressed)) return QObject::eventFilter(watched, event);

    dispatch(actionId);
    event->accept();
    return true;  // verbraucht
}

void ShortcutManager::dispatch(const QString& actionId)
{
    auto* bus = m_services.tryResolve<IEventBus>();
    if (bus == nullptr) return;

    using Transport = TransportCommandEvent::Action;
    // Tabelle statt Kette: eine neue Transport-Aktion ist damit eine Zeile hier
    // und eine Zeile in der Registry.
    static const QHash<QString, Transport> kTransport = {
        {QStringLiteral("transport.playPause"), Transport::PlayPause},
        {QStringLiteral("transport.next"), Transport::Next},
        {QStringLiteral("transport.previous"), Transport::Previous},
        {QStringLiteral("transport.volumeUp"), Transport::VolumeUp},
        {QStringLiteral("transport.volumeDown"), Transport::VolumeDown},
    };

    if (actionId == QStringLiteral("preset.next"))
    {
        BasicLogger::logDebug("ShortcutManager: preset.next");
        bus->publish(PresetStepEvent{1});
    }
    else if (actionId == QStringLiteral("preset.previous"))
    {
        BasicLogger::logDebug("ShortcutManager: preset.previous");
        bus->publish(PresetStepEvent{-1});
    }
    else if (actionId == QStringLiteral("view.screenshot"))
    {
        BasicLogger::logDebug("ShortcutManager: view.screenshot");
        bus->publish(ScreenshotRequestEvent{});
    }
    else if (const auto it = kTransport.constFind(actionId); it != kTransport.constEnd())
    {
        BasicLogger::logDebug("ShortcutManager: " + actionId.toStdString());
        bus->publish(TransportCommandEvent{it.value()});
    }
    else
    {
        BasicLogger::logDebug("ShortcutManager: no handler for " +
                              actionId.toStdString());
    }
}
