/**
 ****************************************************************************************
 * @file   MilkdropGestaltung.hpp
 * @brief  Gestaltungs-Regler des Milkdrop-Knotens: Linear-Drift, Radial-Drift und
 *         Weichzeichnen je Frame (S80) — KEIN Original-Verhalten
 *
 * @author Patrik Neunteufel
 * @date   September 2026
 *
 * @details
 * Hintergrund: bis S80 addierte der Warp-Mesh den D3D9-Halbtexel-Versatz auch
 * unter OpenGL. Das verschob jedes Bild je Frame um ein halbes Pixel nach oben
 * links und zeichnete es per GL_LINEAR weich — ein Portierungsfehler (Sonde gegen
 * MilkdropRef, Offene_Punkte §3), aber ein Look, den man behalten wollte. Hier
 * wird er zu drei unabhaengigen Werkzeugen:
 *
 * - **Linear-Drift** (Staerke px/Frame, Richtung): verschiebt das ganze Bild
 *   SCHARF. Der Bruchteil wird aufgespart und in ganzen Pixeln ausgezahlt
 *   (0,5 px/Frame = jeden zweiten Frame ein ganzes Pixel).
 * - **Radial-Drift** (Staerke, Winkel, Zentrum): jeder Punkt wandert gleich
 *   schnell, die Richtung haengt von seiner Lage zum Zentrum ab — 0 = nach
 *   aussen, π/2 = Wirbel gegen den Uhrzeigersinn, π = Sog nach innen, 3π/2 =
 *   Wirbel im Uhrzeigersinn. Anders als MilkDrops zoom/rot (proportional zum
 *   Abstand) ist die Geschwindigkeit ueberall gleich. Nie ganz scharf: jeder
 *   Punkt wandert um einen anderen Bruchteil, GL_LINEAR mischt dabei.
 * - **Weichzeichnen** (0–1): der Abgriff pendelt je Frame um ±0,5·w Pixel. Im
 *   Mittel verschiebt das nichts, jeder Frame mischt aber mit dem Nachbarpixel.
 *
 * Linear-Drift 0,5·√2 px bei 135° plus Weichzeichnen 1 ergibt in JEDEM Frame
 * exakt +0,5 px im Abgriff — bitgleich den Stand vor S80 („Weicher Sog ↖").
 *
 * Richtungen auf dem Schirm, y nach OBEN (wie MilkDrop-Shapes): 0 = rechts,
 * π/2 = oben. Intern arbeitet der Mesh im Abgriff (u, v): u waechst nach
 * rechts, v nach UNTEN; das Bild wandert um d, wenn der Abgriff um −d greift.
 ****************************************************************************************
 */

#pragma once

#include <cmath>

namespace lumi::milkdrop {

/// Zustand zwischen den Frames (Aufspar-Konto + Pendel-Phase). Beide gehoeren
/// zusammen — wer eins zuruecksetzt, setzt beide zurueck.
struct GestaltungsLauf
{
    double kontoU = 0.0;  ///< aufgesparte Linear-Drift im Abgriff, Pixel
    double kontoV = 0.0;
    long frame = 0;       ///< gerade = Pendel +, ungerade = Pendel −
};

/// Abgriff-Versatz in Pixeln (u nach rechts, v nach unten).
struct GestaltungsVersatz
{
    double u = 0.0;
    double v = 0.0;
};

/// Schirm-Verschiebung (x rechts, y oben, Pixel) -> Abgriff-Versatz.
[[nodiscard]] inline GestaltungsVersatz abgriffFuer(double dx, double dy)
{
    // Bild nach rechts = Abgriff greift links (u −); Bild nach oben = Abgriff
    // greift weiter unten im Bild (v +, weil v nach unten waechst).
    return {-dx, dy};
}

/// Polar -> kartesisch, auf 1e-4 px gerundet: der Panel-Regler speichert die
/// Staerke mit vier Nachkommastellen (0,7071 fuer 0,5·√2), und 0,7071·cos(135°)
/// muss exakt −0,5 ergeben — sonst kippt das Aufspar-Konto an ganzen Pixeln um
/// einen Frame und „Weicher Sog ↖" waere nicht mehr bitgleich.
[[nodiscard]] inline GestaltungsVersatz polar(double staerke, double richtungRad)
{
    const auto runde = [](double x) { return std::round(x * 1e4) / 1e4; };
    return {runde(staerke * std::cos(richtungRad)), runde(staerke * std::sin(richtungRad))};
}

/**
 * @brief Gleichmaessiger Versatz dieses Frames (Linear-Drift + Weichzeichnen)
 *        und den Lauf fortschreiben — genau einmal je Frame rufen
 * @param dx, dy Linear-Drift auf dem Schirm, Pixel je Frame (x rechts, y oben)
 * @param weich  0 = scharf, 1 = volles Halbpixel-Pendel
 */
[[nodiscard]] inline GestaltungsVersatz naechsterVersatz(GestaltungsLauf& lauf, double dx,
                                                         double dy, double weich)
{
    const GestaltungsVersatz d = abgriffFuer(dx, dy);
    const double altU = lauf.kontoU;
    const double altV = lauf.kontoV;
    lauf.kontoU += d.u;
    lauf.kontoV += d.v;
    GestaltungsVersatz out;
    out.u = std::floor(lauf.kontoU) - std::floor(altU);
    out.v = std::floor(lauf.kontoV) - std::floor(altV);
    const double pendel = ((lauf.frame % 2) == 0 ? 0.5 : -0.5) * weich;
    out.u += pendel;
    out.v += pendel;
    ++lauf.frame;
    // Konto klein halten (Genauigkeit ueber lange Laeufe) — ganze Pixel
    // abziehen aendert die floor-Differenzen nicht.
    lauf.kontoU -= std::floor(lauf.kontoU);
    lauf.kontoV -= std::floor(lauf.kontoV);
    return out;
}

/**
 * @brief Radial-Drift an einem Punkt: Abgriff-Versatz in Pixeln
 * @param px, py      Punkt auf dem Schirm, Pixel (x rechts, y OBEN)
 * @param zx, zy      Zentrum, Pixel (gleiche Achsen)
 * @param staerke     Pixel je Frame (negativ = Gegenrichtung)
 * @param winkelRad   0 = aussen, π/2 = Wirbel ↺, π = innen, 3π/2 = Wirbel ↻
 * Im Zentrum selbst (Abstand < 0,5 px) ist die Richtung unbestimmt: kein Versatz.
 */
[[nodiscard]] inline GestaltungsVersatz radialVersatz(double px, double py, double zx,
                                                      double zy, double staerke,
                                                      double winkelRad)
{
    const double rx = px - zx;
    const double ry = py - zy;
    const double len = std::sqrt(rx * rx + ry * ry);
    if (len < 0.5 || staerke == 0.0) return {};
    const double c = std::cos(winkelRad);
    const double s = std::sin(winkelRad);
    // radiale Einheitsrichtung um den Winkel gegen den Uhrzeigersinn drehen
    const double ux = (rx * c - ry * s) / len;
    const double uy = (rx * s + ry * c) / len;
    return abgriffFuer(staerke * ux, staerke * uy);
}

}  // namespace lumi::milkdrop
