/**
 * @file Zeichenform.hpp
 * @brief Klang aus einer Form: eine Zeichnung so aufbereitet, dass ein Bild sie
 *        zeichnen kann — alle Konturen gemeinsam eingepasst, je Kontur die
 *        Reihe, der Umriss und die Kreise, die ihn zeichnen
 *        (docs/audio/Shape_Konzept.md §10; Stufe 19b, der Knoten Shape)
 *
 * Die Reihe einer Kontur fuer den Klang liegt um den Nullpunkt und ist auf
 * vollen Pegel gebracht (`normiert`). Ein Bild braucht etwas anderes: jede
 * Kontur **an ihrer Stelle in der Zeichnung**, alle im selben Massstab. Hier
 * wird die ganze Zeichnung — auch die Konturen ohne Haken — in das Quadrat
 * −1 … 1 gelegt; welche Kontur gezeichnet wird, aendert deshalb nicht, wo die
 * anderen stehen.
 *
 * Teuer ist zweierlei, und beides wird gemerkt: die Reihe einer Kontur (einmal,
 * beim ersten Zeichnen) und ihr Umriss (neu nur, wenn sich Harmonics oder die
 * Punktzahl aendern).
 *
 * Kein Widget, kein OpenGL, keine Qt-Anwendung.
 */
#pragma once

#include "shape/Kontur.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace lumi::shape
{

/// Eine Kontur der Zeichnung, zum Zeichnen aufbereitet.
struct ZeichenKontur
{
    int kennung = 0;
    bool gewaehlt = true;
    float rot = 1.0f;  ///< die Farbe der Kontur, 0 … 1
    float gruen = 1.0f;
    float blau = 1.0f;
    /// Abgetastet (`kAbtastPunkte`), y nach oben, mit allen Konturen der
    /// Zeichnung gemeinsam in −1 … 1 gelegt.
    std::vector<Punkt> punkte;

    // --- gemerkt, entsteht beim ersten Zeichnen -----------------------------
    bool hatReihe = false;
    Reihe reihe;
    double umrissHarmonische = -1.0;
    std::vector<Punkt> umriss;
};

/// Eine Zeichnung, zum Zeichnen aufbereitet.
struct Zeichenform
{
    std::vector<ZeichenKontur> konturen;  ///< in der Reihenfolge der Liste

    [[nodiscard]] bool leer() const { return konturen.empty(); }
};

/// Die Zeichnung aufbereiten: jede Kontur abtasten, alle gemeinsam einpassen.
/// Konturen ohne Pfad fallen weg.
[[nodiscard]] Zeichenform zeichenform(const Zeichnung& zeichnung);

/// Die Plaetze der Konturen, die `contour` meint: 0 = jede mit Haken, sonst die
/// eine mit dieser Kennung (auch ohne Haken). Leer = keine.
[[nodiscard]] std::vector<std::size_t> gemeinteKonturen(const Zeichenform& form, int contour);

/// Der Umriss der Kontur mit `harmonische` Teiltoenen als geschlossener Zug aus
/// `punkte` Punkten, an ihrer Stelle in der Zeichnung. Gemerkt, bis sich einer
/// der beiden Werte aendert.
const std::vector<Punkt>& umrissVon(ZeichenKontur& kontur, double harmonische, int punkte);

/// Ein Kreis der Epizykel-Zeichnung.
struct Kreis
{
    Punkt mitte;
    double radius = 0.0;
};

/// Die Kreise und Arme, die die Kontur zeichnen, mit dem Stift an der Stelle
/// `umlauf` (0 … 1). Der erste Kreis sitzt in der Mitte der Kontur, jeder
/// weitere auf dem Rand des vorigen; die Reihenfolge ist +1, −1, +2, −2, ….
/// `arme` ist der Zug von der Mitte ueber alle Kreise zur Spitze — sein letzter
/// Punkt ist der Punkt des Umrisses an dieser Stelle.
struct Zeigerbild
{
    std::vector<Kreis> kreise;
    std::vector<Punkt> arme;
};
[[nodiscard]] Zeigerbild zeigerbild(ZeichenKontur& kontur, double umlauf, double harmonische);

}  // namespace lumi::shape
