/**
 * @file Tonauftrag.hpp
 * @brief InstrumentCore: was ein Kern rechnen soll — Notenfolge, Stufe, Spuren, Rate, Laenge (Instrumente_IC_Konzept.md
 *        IK2-3 a)
 *
 * Ein Auftrag traegt eine Note, eine Kette (ein Ton ueber mehrere Noten) oder die Noten eines Clips (IC T2 a). **Zeit
 * ueberall in Frames** (`Frames`, auch die Punkte der Spuren), gezaehlt ab dem Anfang des Tons. Die Tonhoehe kommt
 * von aussen (`hz`; die Stimmung bleibt in der App, `tonHz`). Die Staerke ist die Anschlagstufe in dB (≤ 0, wie
 * `:a<stufe>`, `Anschlag.hpp`); dazu die Expression als Spur 0 … 1. Wie ein Kern daraus seine `staerke` 0 … 1 macht,
 * steht in seinem Adapter und im Uebernahme-Journal.
 *
 * `KettenNote` gab es in IC viermal je Klasse (Bogen, Blech, Rohrblatt, Luftstrahl: `{abS, note, hz}` in Sekunden) —
 * hier einmal, in Frames (IK2-2 a).
 *
 * Reines C++ ohne Qt.
 */
#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace lumi::kern
{

/// Zeit in Abtastwerten (IK2-3 a).
using Frames = std::int64_t;

/// Eine Note einer Notenfolge.
struct KettenNote
{
    Frames beginn = 0;     ///< ab hier klingt die Note (Frames ab Anfang des Tons)
    Frames ende = 0;       ///< hier wird sie losgelassen (> beginn)
    int note = 60;         ///< MIDI 0 … 127
    double hz = 261.6255653005986;
    bool gleitet = false;  ///< aus der vorigen Note gebunden (Legato, Slide) statt neu angesetzt

    [[nodiscard]] bool operator==(const KettenNote&) const = default;
};

/// Ein Punkt einer Spur.
struct SpurPunkt
{
    Frames frame = 0;
    double wert = 0.0;

    [[nodiscard]] bool operator==(const SpurPunkt&) const = default;
};

/// Eine Spur ueber die Zeit: zwischen den Punkten linear, davor der erste, danach der letzte Wert.
struct Spur
{
    std::string name;
    std::vector<SpurPunkt> punkte;

    [[nodiscard]] bool operator==(const Spur&) const = default;
};

/// Die festen Namen der Spuren (IK2-3 a); weitere nennt ein Kern in seiner Auskunft (`motor`, `balg`, `register`,
/// `rotary` …).
namespace spur
{
inline constexpr std::string_view kExpression = "expression";  ///< 0 … 1
inline constexpr std::string_view kPitch = "pitch";            ///< Cent
inline constexpr std::string_view kVibrato = "vibrato";        ///< Cent
}  // namespace spur

/// Der Auftrag an einen Kern.
struct Tonauftrag
{
    double rate = 48000.0;
    /// die Notenfolge, nach `beginn` geordnet; eine Note bei einem Kern mit Rechenweg Note
    std::vector<KettenNote> noten;
    /// die Anschlagstufe in dB, `kAnschlagStufeMin` … 0
    int stufe = 0;
    /// so viele Werte rechnet der Kern (Ende der letzten Note plus Nachklang, von der App bestimmt)
    Frames frames = 0;
    std::vector<Spur> spuren;

    [[nodiscard]] bool operator==(const Tonauftrag&) const = default;
};

/// Die Spur mit diesem Namen, oder `nullptr`.
[[nodiscard]] const Spur* findeSpur(const Tonauftrag& auftrag, std::string_view name);

/// Der Wert einer Spur an einem Frame (linear zwischen den Punkten, an den Raendern der Randwert); ohne Punkte
/// `ohne`.
[[nodiscard]] double spurWert(const Spur& spur, Frames frame, double ohne);

/// Ein Auftrag fuer eine Note: `beginn` 0, `ende` = `halten`, `frames` Werte.
[[nodiscard]] Tonauftrag einzelnote(int note, double hz, int stufe, Frames halten, Frames frames, double rate);

/// Was an einem Auftrag nicht stimmt (leer = gut): Rate ≤ 0, Stufe ausserhalb, `frames` < 0, eine Note mit
/// `ende` ≤ `beginn` oder ausserhalb 0 … 127, Noten nicht nach `beginn` geordnet, Spurpunkte nicht nach Frame
/// geordnet, ein Spurname doppelt.
[[nodiscard]] std::vector<std::string> pruefeAuftrag(const Tonauftrag& auftrag);

}  // namespace lumi::kern
