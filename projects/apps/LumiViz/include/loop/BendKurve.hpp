/**
 * @file BendKurve.hpp
 * @brief Loop: die Kurve eines Bends zum Bearbeiten — im Dialog *Edit bend...* des
 *        Instrument-Editors (docs/audio/Saiten_Konzept.md S-L3 a G; Stufe 26d, S137)
 *
 * Waagrecht die Zeit ueber die Laenge der Note (0 … 1), senkrecht die Biegung in
 * Vierteltoenen von 0 bis `kBendHalbtoeneMax` (beschriftet in Ganztoenen wie in Tabs:
 * ½, full, 1½, 2). Punkte ziehen; ein Doppelklick auf eine freie Stelle setzt einen, auf
 * einen Punkt nimmt ihn weg. Ein Punkt bleibt zwischen seinen Nachbarn.
 */
#pragma once

#include "loop/Spielweise.hpp"

#include <QWidget>

#include <vector>

namespace lumi::loop
{

class BendKurve : public QWidget
{
    Q_OBJECT

public:
    explicit BendKurve(QWidget* parent = nullptr);

    void setzePunkte(const std::vector<BendPunkt>& punkte);
    [[nodiscard]] const std::vector<BendPunkt>& punkte() const { return m_punkte; }

    // --- Geometrie (auch fuer Tests) ------------------------------------------
    /// Wo ein Punkt im Widget liegt.
    [[nodiscard]] QPointF ortVon(const BendPunkt& punkt) const;
    /// Der Punkt zu einer Stelle im Widget: Zeit 0 … 1, Biegung im Raster.
    [[nodiscard]] BendPunkt punktBei(QPointF ort) const;
    /// Der Punkt der Kurve unter der Stelle (bis 7 Pixel daneben); -1 = keiner.
    [[nodiscard]] int punktNahe(QPointF ort) const;
    static constexpr int kRand = 28;  ///< links fuer die Beschriftung, sonst rundum

    [[nodiscard]] QSize sizeHint() const override { return {420, 220}; }
    [[nodiscard]] QSize minimumSizeHint() const override { return {240, 140}; }

Q_SIGNALS:
    void geaendert();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    [[nodiscard]] QRectF feld() const;
    std::vector<BendPunkt> m_punkte;
    int m_gezogen = -1;  ///< der Punkt, der gerade gezogen wird
};

}  // namespace lumi::loop
