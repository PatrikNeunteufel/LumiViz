/**
 * @file Grundton.hpp
 * @brief Der Grundton aus den Teiltoenen — eine zweite, unabhaengige Tonhoehenmessung (Instrumente_IC_Konzept.md 34k-3)
 *
 * **Wozu:** YIN (B2) springt bei schwachem Grundton auf Verhaeltnisse der Teiltoene (Befund S192 an der Vorlage der
 * Klasse 2), und die Messung des Stimmers (`ZupfStimmen`, IC) sucht nur die staerkste Spitze in ±60 Cent um die Note —
 * fehlt der Grundton, misst sie Rauschen. Diese Messung liest den Grundton aus den **Teiltoenen an der gemessenen
 * Tonhoehe** (ZV12 a / ZV14 a: nie an der Sollfrequenz): sie braucht ihn nicht im Signal.
 *
 * **Verfahren:** ein Ausschnitt mit Hann-Fenster; je Teilton k = 1 … `hoechstens` die Spitze von |X(f)| nahe der
 * Vorhersage (erst k·Schaetzung in ±50 Cent auf einem Raster von 1 Cent, dann Goldener Schnitt auf 10⁻⁵ Cent), genommen
 * nur, wenn sie innen liegt (nicht am Rand des Suchbereichs) und hoechstens `untergrenzeDb` unter dem staerksten Teilton.
 * Aus den Spitzen f_k das Gesetz der steifen Saite f_k = k·f₀·√(1 + B·k²) als gewichtete Gerade (f_k/k)² = f₀² + f₀²B·k²
 * (Gewicht Amplitude²; mit einem Teilton B = 0); ein zweiter Durchgang sucht an der Vorhersage aus dem ersten in ±20 Cent.
 * Die Amplitude einer Sinusschwingung unter einer reellen Huellkurve ist um ihre Frequenz symmetrisch — die Spitze sitzt
 * auch bei abklingenden Teiltoenen auf der Frequenz.
 *
 * Ohne Qt.
 */
#pragma once

#include <span>

namespace lumi::messung
{

/// Was `grundtonAusTeiltoenen` fand.
struct Grundton
{
    double hz = 0.0;     ///< f₀ der Geraden
    double steife = 0.0; ///< B (0 = harmonisch)
    int teiltoene = 0;   ///< so viele Teiltoene trugen die Gerade
    bool gefunden = false;
};

/// Der Grundton eines Signals aus seinen Teiltoenen (siehe Kopf): `schaetzHz` die Vorhersage (die Note oder eine erste
/// Messung, auf ±50 Cent genau), der Ausschnitt ab `abS` fuer `dauerS`.
[[nodiscard]] Grundton grundtonAusTeiltoenen(std::span<const double> signal, int rate, double schaetzHz, double abS = 0.1, double dauerS = 0.4,
                                             int hoechstens = 12, double untergrenzeDb = -60.0);

}  // namespace lumi::messung
