/**
 * @file SvgKonturen.hpp
 * @brief Klang aus einer Form: die Konturen eines SVG (docs/audio/
 *        Shape_Konzept.md §8; Stufe 20b; Entscheide Patrik S1 a, S120, und
 *        O2 a, S122)
 *
 * **Der Weg (S1 a):** Qt Svg gibt die Pfade eines SVG nicht heraus, zeichnet
 * sie aber. `QSvgRenderer` zeichnet deshalb auf ein eigenes Zeichengeraet, das
 * nichts malt und jeden Aufruf mitschreibt — Pfade, Rechtecke, Kreise,
 * Vielecke, mit der Lage (Verschiebung, Drehung, Groesse) schon angewandt, in
 * den Koordinaten der viewBox. Belegt in test_ShapeBelege.cpp.
 *
 * **Die Konturen (O2 a):** jeder Teilpfad wird eine Kontur, ein Loch ebenso;
 * ein offener wird mit einer geraden Strecke geschlossen. Die Farbe ist die
 * Fuellung, ohne Fuellung der Strich — teilen alle dieselbe, bleibt sie leer,
 * und die Konturen bekommen die Farben der Reihe (O3 a, Patrik S122); was unsichtbar ist, faellt weg —
 * QPainter reicht es gar nicht erst weiter. Farben ueber CSS-Variablen
 * (`var(--x)`) kennt Qt Svg nicht: solche Pfade sind unsichtbar und fehlen.
 * Sind es mehr als `kKonturenMax`,
 * bleiben die laengsten (nach Laenge des Umrisses — offene Linien haben keine
 * Flaeche); anfangs haben alle den Haken, wenn es hoechstens
 * `kSvgAlleGewaehlt` sind, sonst nur die laengste.
 *
 * Kein Widget. Qt Svg braucht keine laufende Anwendung zum Lesen; die Tests
 * laufen trotzdem mit einer `QApplication` (Schriften fuer `<text>`).
 */
#pragma once

#include <QByteArray>
#include <QPainterPath>
#include <QRectF>
#include <QString>

#include <vector>

namespace lumi::shape
{

/// So viele Konturen haben anfangs alle den Haken — so viele Schichten traegt
/// ein Instrument (`loop::kSchichtenMax`), mehr lassen sich mit As layers
/// nicht einzeln spielen.
inline constexpr int kSvgAlleGewaehlt = 8;

/// Eine Kontur aus dem SVG.
struct SvgTeil
{
    QPainterPath pfad;     ///< geschlossen, in den Koordinaten der viewBox (y nach unten)
    QString farbe;         ///< `#rrggbb`; leer = alle Konturen der Datei haben dieselbe Farbe (O3 a): die der Reihe nehmen
    bool offen = false;    ///< war offen und ist mit einer Strecke geschlossen
    double laenge = 0.0;   ///< Laenge des Umrisses
    bool gewaehlt = true;  ///< der Haken, wie O2 a ihn anfangs setzt
};

struct SvgKonturen
{
    bool gelesen = false;          ///< false = keine SVG-Datei oder nicht lesbar
    std::vector<SvgTeil> teile;    ///< in der Reihenfolge der Datei
    int weggefallen = 0;           ///< so viele kuerzere sind ueber `kKonturenMax` hinaus weggefallen
    QRectF rahmen;                 ///< die viewBox
};

/// Die Konturen eines SVG aus seinem Inhalt.
[[nodiscard]] SvgKonturen svgKonturen(const QByteArray& inhalt);
/// Dasselbe aus einer Datei.
[[nodiscard]] SvgKonturen liesSvgKonturen(const QString& datei);

}  // namespace lumi::shape
