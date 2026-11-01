/**
 * @file RasterZeile.hpp
 * @brief Loop: die Zeile mit dem Raster des Loops und dem **Swing offset** eines
 *        Musters — in Drum- und Instrument-Editor gleich (docs/audio/Loop_Konzept.md
 *        §4p, Stufe 15a; Entscheide Patrik RS1 a, RS1-1 a, Skizze SK15a-1 a, S168)
 *
 * Links grau das Raster des ganzen Loops — *Beats per bar*, *Steps per beat* und
 * *Swing* stellt man im Kopf des Loop-Panels ein (RS1 a), hier stehen sie nur.
 * Rechts **Swing offset** in Prozentpunkten und dahinter, mit welchem Swing das
 * Muster spielt (`swingVon`).
 *
 * Die Zeile haelt nichts fest; jede Aenderung meldet sie mit `offsetGeaendert`.
 * Kein Ton, keine Datei.
 */
#pragma once

#include "loop/Loop.hpp"

#include <QWidget>

class QDoubleSpinBox;
class QLabel;

namespace lumi::loop
{

class RasterZeile : public QWidget
{
    Q_OBJECT

public:
    /// `kennung`: der Vorsatz der Objektnamen (`drum`, `instrument`) — die Teile
    /// heissen `<kennung>Grid`, `<kennung>SwingOffset`, `<kennung>SwingPlays`. Ohne
    /// `mitOffset` (Wave editor, Stufe 15d) nur das Raster.
    explicit RasterZeile(const QString& kennung, QWidget* parent = nullptr, bool mitOffset = true);

    /// Raster und Swing von `loop`, den Offset von `muster` zeigen — ohne `offsetGeaendert`.
    void zeige(const Loop& loop, const Muster& muster);

    /// Die Texte, wie sie dastehen (fuer Tests).
    [[nodiscard]] QString rasterText() const;
    [[nodiscard]] QString spieltText() const;

Q_SIGNALS:
    /// Der Offset im Feld, als Bruchteil wie `Muster::swingOffset` (−1 … 1).
    void offsetGeaendert(double offset);

private:
    QLabel* m_raster = nullptr;
    QDoubleSpinBox* m_offset = nullptr;
    QLabel* m_spielt = nullptr;
};

/// Wie ein Swing im Programm steht: „20 %“.
[[nodiscard]] QString swingText(double swing);

}  // namespace lumi::loop
