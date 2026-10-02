/**
 * @file KaraokePanel.hpp
 * @brief Panel der Karaoke-Ebene (S89, KE2 — docs/visuals/Karaoke_Ebene_Konzept.md)
 *
 * Oben der Hauptschalter der Ebene und die Zeile der Karaoke-Presets
 * (`.lvkar` im Ordner der Ablage), darunter Reiter. Reiter 1 „Lyrics" ist das
 * Ketten-Panel im eingeschraenkten Modus (Entscheid K2 a): derselbe Editor wie
 * am Lyrics-Knoten einer Kette, gebunden an den Lyrics-Knoten der Ebene.
 * Reiter 2 „Lyrics Trail" (KE3): Schalter „Trail on" und die Trail-Liste der
 * Ebene als kurze Kette — die Palette zeigt nur Module der Rollen Transform
 * und Color (Entscheid K1 a), der Lyrics Trail am Ende ist fest.
 */

#pragma once

#include "UI/panels/PanelBase.hpp"

#include <QString>
#include <QStringList>

class MultiEffectPanel;
class QCheckBox;
class QComboBox;
class QTabWidget;

class KaraokePanel : public PanelBase
{
    Q_OBJECT

public:
    explicit KaraokePanel(ServiceContainer& services, QWidget* parent = nullptr);
    ~KaraokePanel() override = default;

    /// Der eingebettete Editor scrollt selbst (wie das Ketten-Panel, S78).
    [[nodiscard]] bool scrollsItself() const override { return true; }

    // --- Karaoke-Presets: benannte Staende der ganzen Ebene (Entscheid K3 b) ---
    /// Namen der Presets im Ordner der Ablage, sortiert, ohne Endung; die
    /// laufende Ebene (`current.lvkar`) zaehlt nicht dazu.
    [[nodiscard]] QStringList presetNamen() const;
    /// Stand der Ebene unter `name` speichern (Name wird bereinigt). false =
    /// leerer/reservierter Name, keine Ablage oder Schreibfehler.
    bool speicherePreset(const QString& name);
    bool ladePreset(const QString& name);
    bool loeschePreset(const QString& name);
    /// Zurueck auf das mitgelieferte Aussehen.
    void setzeVorgabe();

    /// Endung der Karaoke-Presets, mit Punkt.
    static constexpr const char* kEndung = ".lvkar";

protected:
    void showEvent(QShowEvent* event) override;

private:
    void setupUI();
    /// „Trail on" auf den Stand der Ebene stellen (Auge der Trail-Liste).
    void gleicheTrailSchalterAb();
    QCheckBox* m_trailSchalter = nullptr;
    MultiEffectPanel* m_trail = nullptr;  ///< Reiter 2: die Trail-Liste der Ebene
    void fuelleCombo(const QString& gewaehlt = {});
    [[nodiscard]] QString ordner() const;                      ///< leer = keine Ablage gesetzt
    [[nodiscard]] QString dateiVon(const QString& name) const; ///< leer = Name ungueltig

    QCheckBox* m_schalter = nullptr;
    QComboBox* m_presets = nullptr;
    QTabWidget* m_reiter = nullptr;
    MultiEffectPanel* m_editor = nullptr;
};
