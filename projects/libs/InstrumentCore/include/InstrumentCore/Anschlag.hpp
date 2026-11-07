/**
 * @file Anschlag.hpp
 * @brief InstrumentCore: die Anschlagstufe — die Lautstaerke einer Note als Stufe in dB (HA3 a, HK2 a;
 *        Instrumente_IC_Konzept.md IK2-3 a: die Staerke im Tonauftrag ist eine Stufe ≤ 0)
 *
 * Ein Kern rechnet je Stufe einen eigenen Ton (der Name traegt `:a<stufe>`); was zwischen Note und Stufe bleibt,
 * gleicht der Pegel aus (`anschlagRest`). Bis S174 stand das in `loop/HammerSaite.hpp` der App und wortgleich in IC
 * (`klassen/01_hammer_saite/HammerSaite.cpp` `anschlagStufe`, `klassen/04_platte_stab/Stab.cpp` `stufeAus`) — jetzt
 * einmal hier (IK2-2 a, KN2 a). Die Geschwindigkeit des Hammers je Stufe (`hammerTempo`) gehoert dem Hammer.
 *
 * Reines C++ ohne Qt.
 */
#pragma once

namespace lumi::kern
{

/// Der Abstand der Stufen in dB — bis 31d ihn festlegt.
inline constexpr double kAnschlagSchrittDb = 1.0;
/// Die leiseste Stufe (−40 dB); darunter wird nur der Pegel leiser.
inline constexpr int kAnschlagStufeMin = -40;

/// Die Stufe einer Note mit dieser Lautstaerke: round(20·lg laut / Schritt), auf `kAnschlagStufeMin` … 0.
/// Lautstaerke 0 oder weniger: die leiseste.
[[nodiscard]] int anschlagStufe(double laut);
/// Der Faktor einer Stufe: 10^(stufe·Schritt/20), die Stufe auf `kAnschlagStufeMin` … 0.
[[nodiscard]] double anschlagFaktor(int stufe);
/// Der Pegel, der den Unterschied zwischen Note und Stufe genau ausgleicht: laut / `anschlagFaktor(stufe)`.
[[nodiscard]] double anschlagRest(double laut, int stufe);

}  // namespace lumi::kern
