/**
 ****************************************************************************************
 * @file   ImportBrowserPanel.hpp
 * @brief  Directory browser for AVS/MilkDrop/LumiViz presets („Effect Browser")
 *
 * @author Patrik Neunteufel
 * @date   July 2026
 * @version 1.0.0
 *
 * @details
 * ## ImportBrowserPanel — Titel „Effect Browser" (seit S82)
 *
 * Sichtbar heisst das Panel seit S82 „Effect Browser": es laedt nebst .avs und
 * .milk auch die eigenen .lvfx (geladene .avs/.milk speichern als .lvfx). Klasse
 * und Panel-id `import_browser` bleiben — an der id haengen Dock-Layout und
 * gespeicherter Ordner.
 *
 * A file-browser panel (styled like the Playlist) for quickly checking imports:
 * - Shows the contents of a folder, filtered to `*.avs` / `*.milk` / `*.lvfx` / all.
 * - Double-click a folder to descend, ".." or the Up button to ascend.
 * - Double-click a preset to import it into the Multi Effect host.
 * - Drag a preset into the Effect Chain (S84): it travels as a file URL
 *   (`UI/widgets/DateiListe.hpp`) and becomes a preset reference there. Copy
 *   only — dragging into Explorer never moves the file; folders don't drag.
 *
 * ## Integration
 *
 * The panel does not import itself — it publishes an ImportAvsPresetEvent (or
 * ImportMilkPresetEvent) carrying the file path. MainWindow owns the import
 * orchestration (host auto-activation, render mutex, report dialog) and answers
 * with an AvsImportResultEvent, which the panel shows as a non-modal status.
 *
 * ```
 * ┌────────────────────┐  ImportAvsPresetEvent{path}  ┌────────────┐
 * │ ImportBrowserPanel │ ────────────────────────────►│ MainWindow │
 * │                    │◄──── AvsImportResultEvent ────│            │
 * └────────────────────┘                              └────────────┘
 * ```
 ****************************************************************************************
 */

#pragma once

#include "PanelBase.hpp"

#include <QDir>
#include <QString>

#include <string>

class QListWidget;
class QListWidgetItem;
class QPushButton;
class QLineEdit;
class QComboBox;
class QLabel;

/**
 * @class ImportBrowserPanel
 * @brief Folder browser that imports AVS/MilkDrop presets on double-click
 */
class ImportBrowserPanel : public PanelBase
{
    Q_OBJECT

public:
    explicit ImportBrowserPanel(ServiceContainer& services, QWidget* parent = nullptr);
    ~ImportBrowserPanel() override = default;

    [[nodiscard]] int preferredArea() const override;

    /// Das laufende Preset, wie das Hauptfenster es gemeldet hat (leer = keins).
    [[nodiscard]] const QString& aktivesPreset() const { return m_lastPreset; }
    /// Ordner, den der Browser gerade zeigt.
    [[nodiscard]] QString ordner() const { return m_dir.absolutePath(); }
    /// In den Ordner des laufenden Presets wechseln und es markieren (S93,
    /// Knopf ◎). Loest dafuer die Ansicht „★ Favorites", einen Typfilter oder
    /// Suchtext, wenn sie es verdecken. false = nichts laeuft / Datei fehlt.
    bool springeZumAktivenPreset();

    /// Per-item role Qt::UserRole+1 — what a double-click should do. Oeffentlich,
    /// weil es der Eingabe-Vertrag von nextPresetRow() ist.
    enum EntryType
    {
        Type_Up = 0,
        Type_Dir = 1,
        Type_Avs = 2,
        Type_Milk = 3,
        Type_Lvfx = 4,
        /// S89: Karaoke-Preset (.lvkar) — laedt in die Karaoke-Ebene, nicht in
        /// die Kette. Kein „Preset" im Sinn von nextPresetRow()/Zuletzt-geladen
        /// und nicht in die Kette ziehbar.
        Type_Karaoke = 5,
        /// Composer-Naht N8 (docs/ui/Composer_Konzept.md §12.2; S100): eine
        /// Komposition (.lvcomp) — ein Doppelklick oeffnet sie im Composer.
        /// Kein Preset: nicht im Blaettern, nicht bei den Favoriten, nicht in
        /// die Kette ziehbar.
        Type_Komposition = 6
    };

    /**
     * @brief Nachbar-Zeile, die ein Preset traegt — der Kern der
     *        Hotkey-Navigation (docs/ui/Hotkey_Konzept.md, Stufe 1).
     *
     * @param types  EntryType je Zeile in Anzeigereihenfolge
     * @param from   aktuelle Zeile, oder -1 fuer "keine Auswahl"
     * @param delta  Vorzeichen entscheidet die Richtung
     * @return Zeilenindex, oder -1 wenn es kein anderes Preset gibt
     *
     * Ordner und ".." werden uebersprungen (keine Rekursion, §9.2). Am Ende
     * des Verzeichnisses laeuft es um — nach dem letzten Preset kommt das
     * erste, vor dem ersten das letzte (Entscheid Patrik S60, §9.3; gebaut
     * S129). Ist das aktuelle das einzige Preset, kommt -1 statt desselben.
     * Frei von Qt-Widgets, damit die Regel pruefbar ist.
     */
    [[nodiscard]] static int nextPresetRow(const QList<int>& types, int from,
                                          int delta);

    /**
     * @brief S91 (Entscheid F1 a): Favorit setzen oder nehmen, ohne das Preset
     *        zu laden — der Rechtsklick in der Liste ruft das.
     *
     * Dieselben Listen wie der Stern der Preset-Zeile (`PresetListen`): Ketten-
     * Presets und Karaoke-Presets (.lvkar) getrennt. Meldet die Aenderung als
     * PresetListsChangedEvent. Ordner und ".." tun nichts (false).
     */
    bool schalteFavorit(int reihe);

    /// Die Ansicht „★ Favorites" ein- oder ausschalten (aus = Filter „All").
    void setFavoritenAnsicht(bool an);
    [[nodiscard]] bool favoritenAnsicht() const { return m_filter == Filter::Favoriten; }

protected:
    void onActivate() override;
    void onDeactivate() override;
    void saveState() override;
    void restoreState() override;

private Q_SLOTS:
    void onItemDoubleClicked(QListWidgetItem* item);
    void onUpClicked();
    void onBrowseClicked();
    void onPathEntered();
    void onFilterChanged(int index);
    void onSearchChanged(const QString& text);

private:
    /// Which preset kinds to list (folders are always shown for navigation).
    enum class Filter
    {
        Avs = 0,
        Milk = 1,
        Lvfx = 2,   ///< native LumiViz effect chains
        All = 3,
        /// S91: keine Ordneransicht — alle Favoriten (Kette und Karaoke) aus
        /// allen Ordnern als eine Liste; Pfadzeile, Up und Browse ruhen.
        Favoriten = 4
    };
    void refreshFavoriten();  ///< die Liste der Ansicht „★ Favorites" aufbauen
    void zeigeKontextMenue(const QPoint& stelle);

    void setupUI();
    void setupConnections();
    void subscribeToEvents();

    void navigateTo(const QString& dirPath);
    void refresh();
    [[nodiscard]] QStringList currentNameFilters() const;
    [[nodiscard]] static int entryTypeForSuffix(const QString& suffix);
    void setStatus(const QString& text);
    void onImportResult(const std::string& path, bool ok, int noteCount);
    void onPresetStep(int delta);
    /// Forget the persisted start folder and return to the application
    /// directory, where the bundled presets/ folder lives (S73; was the home
    /// directory before). Triggered by ResetImportBrowserDirEvent from the
    /// Settings panel.
    void resetStoredDir();

    /// Das laufende Preset des Hauptfensters uebernehmen (S93) — merken und,
    /// wenn die Liste es zeigt, markieren. Leer = nichts laeuft unter einem
    /// Namen. Gesichert und beim Start geladen wird es NICHT hier
    /// (UI/StartPreset.hpp); der Ordner des Browsers bleibt, wo er ist.
    void merkeLaufendesPreset(const QString& pfad);
    void aktualisiereSprungKnopf();

    /// Gemeinsamer Weg "Eintrag oeffnen" fuer Doppelklick und
    /// Blaettern-Hotkey.
    void openEntry(int type, const QString& path);

    /// Das laufende Preset in der Liste markieren, ohne es zu laden.
    void selectLastPresetInList();

    // UI Elements
    QLineEdit* m_pPathEdit = nullptr;
    QPushButton* m_pUpButton = nullptr;
    QPushButton* m_pJumpButton = nullptr;  ///< ◎ zum aktiven Preset (S93)
    QPushButton* m_pBrowseButton = nullptr;
    QComboBox* m_pFilterCombo = nullptr;
    QLineEdit* m_pSearchEdit = nullptr;
    QListWidget* m_pListWidget = nullptr;
    QLabel* m_pStatusLabel = nullptr;

    // State
    QDir m_dir;
    Filter m_filter = Filter::All;

    /// Zuletzt GELADENES Preset (absoluter Pfad, leer = keins). Wird beim
    /// Doppelklick bzw. beim Blaettern gesetzt — also genau dann, wenn ein
    /// Preset wirklich zur Anzeige kam, nicht schon beim blossen Anklicken.
    QString m_lastPreset;

    /// Lives for the whole panel lifetime (NOT cleared in onDeactivate) — the
    /// Settings-panel reset must reach this panel even while it is hidden.
    IEventBus::SubscriberHandle m_resetSubscription;
    /// Permanent wie m_resetSubscription: der Hotkey wirkt auch, wenn dieses
    /// Panel nicht sichtbar ist.
    IEventBus::SubscriberHandle m_stepSubscription;
    /// Permanent: das laufende Preset wird auch gemerkt, wenn dieses Panel
    /// nie sichtbar war (S93).
    IEventBus::SubscriberHandle m_laufendSubscription;
};
