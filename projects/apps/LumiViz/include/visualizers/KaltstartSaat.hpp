/**
 ****************************************************************************************
 * @file   KaltstartSaat.hpp
 * @brief  Die Kaltstart-Rauschsaat des MilkDrop-Feedbackpuffers — GEMEINSAME
 *         Quelle fuer LumiViz und MilkdropRef (Session 75).
 *
 * @author Patrik Neunteufel
 * @date   August 2026
 * @version 1.0.0
 *
 * @details
 * **Warum das geteilt gehoert** (Vorbild `SynthAudio.hpp`, S74): LumiViz streut
 * beim Kaltstart Rauschen in den Feedback-Puffer, damit Verstaerker-Presets
 * ueberhaupt zuenden (S63). Der Original-Kern hat kein Gegenstueck und startet
 * mit genulltem VRAM. Im Referenzvergleich zeichnen wir deshalb, waehrend die
 * Referenz schwarz bleibt — nachgewiesen S75 an `Starfield`, `Helix`,
 * `The Beauty and the Math`.
 *
 * Bis S74 fiel das nicht auf, weil der MilkDrop-Vergleich als STAPEL lief: dort
 * erbt jedes Preset das Bild seines Vorgaengers und zeigt darum immer etwas.
 * Belegt an `A_Blank.milk` + `B_Starfield.milk` — beide melden denselben
 * Mittelwert, das „Starfield-Bild" war das geerbte Blank-Bild.
 *
 * Damit BEIDE Seiten im selben Startzustand beginnen koennen, liegt die Formel
 * hier: Qt-frei und C++14, damit der 32-bit-D3D9-`MilkdropRef` sie ebenso
 * uebersetzt wie die App.
 *
 * **Vorbehalt zur Bit-Gleichheit:** identisch ist die Saat nur bei gleicher
 * Puffergroesse. Der Kern haelt seine Feedback-Textur (`m_lpVS`) nicht
 * zwingend in Fenstergroesse; dann liefert dieselbe Formel dieselbe
 * Rauschstatistik, aber nicht dasselbe Bild. Fuer „zuendet das Preset" reicht
 * das, fuer einen Pixelvergleich der ersten Frames nicht.
 ****************************************************************************************
 */

#ifndef LUMI_KALTSTART_SAAT_HPP
#define LUMI_KALTSTART_SAAT_HPP

#include <cstddef>
#include <vector>

namespace lumi
{
namespace saat
{

/// Startwert der App seit S63 — Vorgabe, damit sich das App-Bild nicht aendert.
const unsigned int kSeed = 0x5EED63u;

/// Kein Rauschen: schwarzer Puffer wie der genullte VRAM der Referenz.
const unsigned int kSeedAus = 0u;

/**
 * @brief Eine Auswahl von Startwerten fuer Messreihen (Vorgabe Patrik, S75).
 *
 * Ein EINZELNER Seed beantwortet nur „stimmen die Bilder bei diesem einen
 * Rauschmuster ueberein". Interessanter ist, **wie stark ein Preset ueberhaupt
 * am Startzustand haengt**: streut sein Bild ueber mehrere Seiten hinweg
 * staerker als der Unterschied zwischen den Renderern, ist es grundsaetzlich
 * nicht pixelvergleichbar — genau die Frage der S67-Dunkelklasse, wo der Look
 * an nicht-genulltem VRAM haengt. Wer ueber mehrere Seeds misst, trennt „wir
 * rechnen anders" von „das Preset ist startzustands-abhaengig".
 *
 * `kSeedAus` gehoert in jede Reihe: er ist der Zustand der Referenz.
 */
inline std::vector<unsigned int> reihe()
{
    return {kSeedAus, kSeed, 0x13579BDFu, 0xA5A5A5A5u};
}

/**
 * @brief Rausch-Basis fuer den Feedback-Puffer, 4 Byte je Pixel (RGBA).
 *
 * xorshift32, ein Zug je Pixel, RGB aus den unteren Bytes, Alpha deckend.
 * @p seed == `kSeedAus` liefert eine schwarze (genullte) Basis — der
 * Referenz-Startzustand und zugleich die definierte Semantik des
 * Puffer-Wechsels „Loeschen" im Pruefstand.
 *
 * xorshift32 darf nie mit 0 laufen (bliebe 0) — deshalb ist 0 als „aus"
 * belegt und nicht als Rausch-Startwert.
 */
inline std::vector<unsigned char> basis(int w, int h, unsigned int seed)
{
    std::vector<unsigned char> noise(static_cast<std::size_t>(w) * h * 4);
    if (seed == kSeedAus) return noise;
    unsigned int s = seed;
    for (std::size_t i = 0; i < noise.size(); i += 4)
    {
        s ^= s << 13;
        s ^= s >> 17;
        s ^= s << 5;
        noise[i + 0] = static_cast<unsigned char>(s);
        noise[i + 1] = static_cast<unsigned char>(s >> 8);
        noise[i + 2] = static_cast<unsigned char>(s >> 16);
        noise[i + 3] = 255;
    }
    return noise;
}

}  // namespace saat
}  // namespace lumi

#endif  // LUMI_KALTSTART_SAAT_HPP
