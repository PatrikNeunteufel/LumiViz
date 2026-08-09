/**
 ****************************************************************************************
 * @file   ShortcutManager.hpp
 * @brief  Hotkey-Schicht: Taste -> Aktion -> Ereignis (Konzept:
 *         docs/ui/Hotkey_Konzept.md)
 *
 * @author Patrik Neunteufel
 * @date   July 2026
 * @version 1.0.0
 *
 * @details
 * Die Belegung kommt aus `lumi::services::shortcutActions()` (SSOT), Abweichungen
 * aus `QSettings` unter `shortcuts/<id>`. Nur Abweichungen werden gespeichert —
 * eine fehlende Einstellung heisst "Vorbelegung", damit eine geaenderte
 * Vorbelegung in einer neuen Version automatisch fuer jeden wirkt, der die Taste
 * nie angefasst hat.
 *
 * ## Ereignisfilter statt QShortcut (Konzept §5)
 *
 * `QShortcut` greift VOR dem Fokus-Widget und verschluckt die Taste, auch wenn
 * der Handler nichts tut — ein `PageDown` im EEL-Editor wuerde dann das Preset
 * wechseln statt zu blaettern. Dieser Manager haengt deshalb als Filter an
 * `qApp` und laesst das Ereignis unangetastet durch, sobald das Fokus-Widget
 * Texteingabe annimmt (und die Sequenz unmodifiziert ist).
 *
 * Threading: reine UI-Schicht, laeuft im GUI-Thread.
 ****************************************************************************************
 */

#pragma once

#include "services/ShortcutRegistry.hpp"

#include <QHash>
#include <QKeySequence>
#include <QObject>
#include <QString>

class ServiceContainer;
class QWidget;

/**
 * @class ShortcutManager
 * @brief Uebersetzt Tastendruecke in Aktions-Ereignisse
 */
class ShortcutManager : public QObject
{
    Q_OBJECT

public:
    ShortcutManager(ServiceContainer& services, QObject* parent = nullptr);
    ~ShortcutManager() override;

    /// @brief Aktuelle Sequenz einer Aktion (Override oder Vorbelegung).
    [[nodiscard]] QKeySequence sequenceFor(const QString& actionId) const;

    /// @brief Vorbelegung einer Aktion.
    [[nodiscard]] static QKeySequence defaultSequenceFor(const QString& actionId);

    /**
     * @brief Sequenz einer Aktion setzen (leer = keine Taste).
     * @return leerer String bei Erfolg, sonst der Grund der Ablehnung.
     *
     * Abgelehnt wird eine Sequenz, die eine ANDERE Aktion schon haelt
     * (Kollision) oder die nach der Reservierungs-Regel einer fremden
     * Kategorie gehoert (§2) — kein stilles Ueberschreiben.
     */
    QString setSequence(const QString& actionId, const QKeySequence& sequence);

    /// @brief Eine Aktion auf ihre Vorbelegung zuruecksetzen.
    void resetToDefault(const QString& actionId);

    /// @brief Alle Aktionen auf ihre Vorbelegung zuruecksetzen.
    void resetAllToDefaults();

    /// @brief Aktion, die diese Sequenz haelt (leer wenn keine).
    [[nodiscard]] QString actionForSequence(const QKeySequence& sequence) const;

    /**
     * @brief Nimmt dieses Widget Texteingabe an?
     *
     * Frei und statisch, damit die Regel testbar bleibt: Zeilen-/Mehrzeilen-
     * Editoren (nicht schreibgeschuetzt), Zahlenfelder, editierbare Combos und
     * das Tastenaufnahme-Feld selbst.
     */
    [[nodiscard]] static bool acceptsTextInput(const QWidget* widget);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void loadOverrides();
    [[nodiscard]] bool shouldIgnoreWhileTyping(const QKeySequence& sequence) const;
    void dispatch(const QString& actionId);

    ServiceContainer& m_services;
    /// Aktions-Bezeichner -> aktuelle Sequenz (leer = keine Taste)
    QHash<QString, QKeySequence> m_sequences;
};
