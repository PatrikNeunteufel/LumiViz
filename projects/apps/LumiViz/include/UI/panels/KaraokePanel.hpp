/**
 * @file KaraokePanel.hpp
 * @brief Panel der Karaoke-Ebene (S89, KE2 — docs/visuals/Karaoke_Ebene_Konzept.md)
 *
 * Oben der Hauptschalter der Ebene und die Preset-Zeile (wie im Panel „Effect
 * Chain": laufendes Karaoke-Preset, Favoriten, zuletzt benutzt, Load…, Save,
 * Save As…, dazu Default), darunter Reiter. Reiter 1 „Lyrics" ist das
 * Ketten-Panel im eingeschraenkten Modus (Entscheid K2 a): derselbe Editor wie
 * am Lyrics-Knoten einer Kette, gebunden an den Lyrics-Knoten der Ebene.
 * Reiter 2 „Lyrics Trail" (KE3): Schalter „Trail on" und die Trail-Liste der
 * Ebene als kurze Kette — die Palette zeigt nur Module der Rollen Transform
 * und Color (Entscheid K1 a), der Lyrics Trail am Ende ist fest.
 *
 * Karaoke-Presets (`.lvkar`) liegen, wo der Benutzer sie hinlegt (Dateidialog,
 * Auftrag Patrik S89) — die laufende Ebene bleibt `current.lvkar` im
 * Benutzerordner und schreibt sich bei jedem Edit selbst.
 */

#pragma once

#include "UI/panels/PanelBase.hpp"

#include <QString>
#include <QStringList>

class MultiEffectPanel;
class MultiEffectVisualizer;
class QCheckBox;
class QLabel;
class QMutex;
class QTabWidget;
namespace lumi::ui
{
class PresetZeile;
}

class KaraokePanel : public PanelBase
{
    Q_OBJECT

public:
    explicit KaraokePanel(ServiceContainer& services, QWidget* parent = nullptr);
    ~KaraokePanel() override = default;

    /// Der eingebettete Editor scrollt selbst (wie das Ketten-Panel, S78).
    [[nodiscard]] bool scrollsItself() const override { return true; }

    /// Der EINE Weg, die Ebene zu schalten (Panel, Menue, Tastenkuerzel — KE4):
    /// merkt die Einstellung (QSettings `karaoke/enabled`), schaltet den
    /// Renderer und meldet `KaraokeEnabledChangedEvent`. `bus` darf null sein.
    static void setzeKaraokeAn(IEventBus* bus, bool an);

    // --- Karaoke-Presets: Stand der ganzen Ebene als Datei (Entscheid K3 b) ---
    //
    // Die Wege sind statisch, weil auch das Fenster sie geht (Doppelklick auf
    // ein .lvkar im Effect Browser) — mit dem Visualizer und dem Render-Mutex
    // des Aufrufers.

    /// Worauf geladen/gespeichert wird. `bus` und `parent` duerfen null sein.
    struct Ziel
    {
        MultiEffectVisualizer* host = nullptr;
        QMutex* mutex = nullptr;
        IEventBus* bus = nullptr;
        QWidget* parent = nullptr;
    };
    /// Karaoke-Preset in die Ebene laden — OHNE Rueckfrage und ohne Dialog.
    /// Schreibt die Ablage, merkt das Preset als laufendes und als „zuletzt",
    /// meldet `KaraokeLayerChangedEvent`. false = unlesbar / kein Karaoke-Preset.
    static bool ladeKaraokeDatei(const Ziel& ziel, const QString& pfad,
                                 QStringList* report = nullptr);
    /// Die Ebene nach `pfad` speichern — ohne Dialog. false = Schreibfehler
    /// oder `pfad` ist die Ablage der laufenden Ebene selbst.
    static bool speichereKaraokeNach(const Ziel& ziel, const QString& pfad);
    /// „Save" (saveAs = false: in das laufende Karaoke-Preset, wenn es eines
    /// gibt) bzw. „Save As…" (immer Dateidialog). false = abgebrochen/Fehler.
    static bool speichereKaraoke(const Ziel& ziel, bool saveAs);
    /// Vor dem Ersetzen der Ebene (Laden, Default): hat sie ungespeicherte
    /// Aenderungen, fragt ein Dialog Save / Discard / Cancel (Entscheid 5a).
    /// true = weiter.
    [[nodiscard]] static bool darfKaraokeErsetzen(const Ziel& ziel, const QString& titel);

    /// Im Panel: mit Rueckfrage laden / zurueck auf das mitgelieferte Aussehen.
    bool ladeDatei(const QString& pfad);
    void setzeVorgabe();

    /// Endung der Karaoke-Presets, mit Punkt.
    static constexpr const char* kEndung = ".lvkar";

protected:
    void showEvent(QShowEvent* event) override;

private:
    void setupUI();
    [[nodiscard]] Ziel ziel();
    /// „Trail on" auf den Stand der Ebene stellen (Auge der Trail-Liste).
    void gleicheTrailSchalterAb();
    /// Zustandszeile (KE4): „Preset shows its own lyrics", solange die Ebene
    /// an ist und die laufende Kette selbst Lyrics zeigt — und der Stern der
    /// Preset-Zeile. Laeuft zweimal je Sekunde, nur bei sichtbarem Panel.
    void aktualisiereZustand();

    QCheckBox* m_schalter = nullptr;
    QLabel* m_zustand = nullptr;
    lumi::ui::PresetZeile* m_presetZeile = nullptr;
    QTabWidget* m_reiter = nullptr;
    MultiEffectPanel* m_editor = nullptr;  ///< Reiter 1: der Lyrics-Knoten der Ebene
    QCheckBox* m_trailSchalter = nullptr;
    MultiEffectPanel* m_trail = nullptr;   ///< Reiter 2: die Trail-Liste der Ebene
};
