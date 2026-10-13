/**
 * @file Bausteine.hpp
 * @brief Loop: was die Reihen von Bausteinen an Schicht und Clip gemeinsam
 *        haben (docs/audio/Loop_Konzept.md §4j, Stufe 16; Entscheide Patrik
 *        S117: K1 a eine Reihe, K4 a hoechstens acht)
 *
 * Der Weg des Tons ist an beiden Stellen eine **Reihe**: an der Schicht eines
 * Instruments (Filter, Drive, Envelope — `SchichtBaustein`, Instrument.hpp),
 * am Clip (Filter, Distortion, Echo, Reverb — `ClipBaustein`, Loop.hpp). Hier
 * steht nur, was beide teilen. Kein Qt-Widget, keine Audio-Engine.
 */
#pragma once

#include <QLatin1String>
#include <QString>

namespace lumi::loop
{

/// So viele Bausteine traegt eine Reihe hoechstens, an Schicht und Clip gleich
/// (K4 a); Envelope zaehlt mit, ausgeschaltete auch. Eine Datei mit mehr wird
/// nach dem achten abgeschnitten.
inline constexpr int kBausteineMax = 8;

/// Die Art eines Filters (Stufe 11e; Skizze gebilligt, Patrik S116). Der
/// Tiefpass laesst durch, was unter der Grenze liegt, der Hochpass, was
/// darueber liegt, der Bandpass, was um sie herum liegt.
enum class FilterArt : int
{
    Tiefpass = 0,
    Hochpass,
    Bandpass,
};

/// Wie die Art in einer Datei steht: `lowpass`, `highpass`, `bandpass`.
[[nodiscard]] inline QLatin1String filterArtName(FilterArt art)
{
    return QLatin1String(art == FilterArt::Hochpass ? "highpass" : art == FilterArt::Bandpass ? "bandpass" : "lowpass");
}
/// Fehlt die Art oder ist sie unbekannt, gilt der Tiefpass.
[[nodiscard]] inline FilterArt filterArtAus(const QString& name)
{
    return name == QLatin1String("highpass")   ? FilterArt::Hochpass
           : name == QLatin1String("bandpass") ? FilterArt::Bandpass
                                               : FilterArt::Tiefpass;
}

}  // namespace lumi::loop
