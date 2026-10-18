/**
 * @file SaitenBild.hpp
 * @brief Loop: das Saitenbild der Gruppe String im Instrument-Creator
 *        (docs/audio/Saiten_Konzept.md §4, Stufe 23e; Skizze gebilligt,
 *        Patrik S132: S-U1 a, Nachtrag S-U3 a)
 *
 * Sattel links, Steg rechts, die Saite dazwischen — massstaeblich: die Breite
 * ist die Mensur. Darauf eine Marke **pluck** (die Anschlagstelle) und, bei
 * Waveguide, je Tonabnehmer eine Marke **neck** und **bridge** (ein Humbucker
 * als zwei Striche, `kHumbuckerAbstandMm` auseinander; ein Piezo steht fest
 * am Steg und laesst sich nicht ziehen). Die Marken lassen sich mit der Maus
 * ziehen und stellen dieselben Werte wie ihre Regler.
 *
 * Kein Modell: das Bild zeigt, was es gesagt bekommt, und meldet, wohin
 * gezogen wird.
 */
#pragma once

#include "loop/Instrument.hpp"

#include <QWidget>

#include <array>

namespace lumi::loop
{

class SaitenBild : public QWidget
{
    Q_OBJECT

public:
    explicit SaitenBild(QWidget* parent = nullptr);

    /// Was das Bild zeigt. `anschlagOrt`: Anteil der Saite vom Steg;
    /// `mensurMm`: die Laenge vom Sattel zum Steg; `abnehmer`: Neck und
    /// Bridge (eine Art *None* hat keine Marke); `mitAbnehmern`: false = keine
    /// Tonabnehmer-Marken (Extended KS).
    void setze(double anschlagOrt, double mensurMm, const std::array<Abnehmer, 2>& abnehmer, bool mitAbnehmern);

    /// Die Marken, die sich greifen lassen.
    enum Griff : int
    {
        KeinGriff = -1,
        GriffAnschlag = 0,
        GriffHals = 1,
        GriffSteg = 2,
    };
    /// Die Marke unter einem Punkt; `KeinGriff` = keine in Reichweite.
    [[nodiscard]] Griff griffBei(QPoint ort) const;
    /// Wo eine Marke steht (fuer Tests); eine, die es nicht gibt: (-1, -1).
    [[nodiscard]] QPoint ortVon(Griff griff) const;
    /// So weit neben einer Marke greift die Maus sie noch.
    static constexpr int kGriffWeite = 8;

    /// x eines Anteils der Saite vom Steg, und umgekehrt.
    [[nodiscard]] int xVon(double anteilVomSteg) const;
    [[nodiscard]] double anteilBei(int x) const;

    [[nodiscard]] QSize sizeHint() const override { return {260, 92}; }
    [[nodiscard]] QSize minimumSizeHint() const override { return {160, 92}; }

Q_SIGNALS:
    /// Die Marke pluck wird gezogen: der neue Anteil vom Steg (in den Grenzen der Schicht).
    void anschlagGezogen(double anteilVomSteg);
    /// Eine Tonabnehmer-Marke wird gezogen: `platz` 0 = Neck, 1 = Bridge; die Lage in mm vom Steg.
    void abnehmerGezogen(int platz, double mmVomSteg);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    /// Die Saite von links (Sattel) nach rechts (Steg), in Punkten.
    [[nodiscard]] int links() const;
    [[nodiscard]] int rechts() const;
    [[nodiscard]] int saiteY() const;
    [[nodiscard]] bool ziehbar(int platz) const;
    void zeigeZeiger(Griff griff);

    double m_anschlag = 0.13;
    double m_mensur = 648.0;
    std::array<Abnehmer, 2> m_abnehmer{};
    bool m_mitAbnehmern = false;
    Griff m_gezogen = KeinGriff;
};

}  // namespace lumi::loop
