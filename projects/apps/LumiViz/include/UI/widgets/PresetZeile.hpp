/**
 * @file PresetZeile.hpp
 * @brief Preset-Zeile der Panels „Effect Chain" und „Karaoke" (S89)
 *
 * Auftrag Patrik S89: Presets sind nicht nur ueber das Menue File und den
 * Effect Browser lad- und speicherbar, sondern auch im Panel selbst — in
 * beiden Panels gleich:
 *
 *   Preset: [ Name * ▾ ]  ★  Load…  Save  Save As…
 *
 * Das Dropdown zeigt oben das laufende Preset (mit Stern, solange es
 * ungespeicherte Aenderungen traegt), darunter die **Favoriten** und die
 * **zehn zuletzt benutzten** (UI/PresetListen.hpp) — nicht alle Dateien eines
 * Ordners. Eine Auswahl **laedt sofort** (Entscheid 1a); das Mausrad wechselt
 * nichts. ★ macht das laufende Preset zum Favoriten. Einen Loeschknopf gibt
 * es nicht (Entscheid 2a): was fehlt, faellt beim Anklicken aus der Liste.
 *
 * Die Zeile selbst laedt und speichert nichts — sie meldet nur; die Panels
 * haengen die Wege daran.
 */

#pragma once

#include "UI/PresetListen.hpp"

#include <QString>
#include <QWidget>

class QComboBox;
class QHBoxLayout;
class QToolButton;

namespace lumi::ui
{

class PresetZeile : public QWidget
{
    Q_OBJECT

public:
    explicit PresetZeile(PresetArt art, QWidget* parent = nullptr);

    /// Das laufende Preset (leer = keins / neu). Baut das Dropdown neu.
    void setLaufend(const QString& pfad);
    [[nodiscard]] QString laufend() const { return m_laufend; }
    /// Ungespeicherte Aenderungen? Zeigt den Stern hinter dem Namen.
    void setGeaendert(bool geaendert);
    /// Listen neu lesen (nach PresetListsChangedEvent).
    void aktualisiere();
    /// Bedienbar? (ohne Multi-Effect-Visualizer nicht)
    void setBedienbar(bool an);
    /// Weitere Knoepfe hinter „Save As…" (z. B. „Default" im Karaoke-Panel).
    void haengeAn(QWidget* knopf);

    /// Text der ersten Zeile, wenn kein Preset laeuft.
    void setOhneNameText(const QString& text);

    /// Fester Eintrag „neu anfangen" direkt unter dem laufenden Preset (S93,
    /// Wunsch Patrik) — immer da, auch wenn schon eine neue Kette laeuft.
    /// Ihn zu waehlen meldet `neu()`; leer = kein solcher Eintrag.
    void setNeuEintrag(const QString& text);

signals:
    void neu();                           ///< der Eintrag „neu anfangen"
    void ladeDatei(const QString& pfad);  ///< Auswahl im Dropdown
    void ladenDialog();                   ///< „Load…"
    void speichern(bool saveAs);          ///< „Save" / „Save As…"
    void listenGeaendert();               ///< ★ geschaltet oder ein fehlender Eintrag entfernt

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void baueListe();

    PresetArt m_art;
    QString m_laufend;
    QString m_ohneName;
    QString m_neuEintrag;  ///< leer = kein Eintrag „neu anfangen"
    bool m_geaendert = false;
    QComboBox* m_combo = nullptr;
    QToolButton* m_stern = nullptr;
    QHBoxLayout* m_layout = nullptr;
};

}  // namespace lumi::ui
