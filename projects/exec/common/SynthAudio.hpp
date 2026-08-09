/**
 ****************************************************************************************
 * @file   SynthAudio.hpp
 * @brief  Der synthetische Prüfstands-Klang — EINE Quelle für alle vier Renderer
 *         (AvsStandalone, MilkdropStandalone, tools/AvsRef, tools/MilkdropRef)
 *
 * @author Patrik Neunteufel
 * @date   August 2026 (Session 74)
 * @version 1.0.0
 *
 * @details
 * Referenzvergleiche beruhen darauf, dass **beide Seiten dasselbe hören**. Bis
 * S74 stand die Formel dafür VIERMAL im Baum, jedes Mal als Kopie mit dem
 * Kommentar „formelgleich zu …". Diese Datei ist die Zusammenlegung: alle vier
 * Werkzeuge binden sie ein, Abweichungen sind damit baulich ausgeschlossen.
 *
 * Bewusst **ohne Qt und ohne C++20** — `tools/AvsRef` ist ein 32-bit-MSVC-
 * Projekt um den originalen vis_avs-Kern, `tools/MilkdropRef` ein D3D9-Host.
 * Beide bauen außerhalb der CMakeCraft-Solution und dürfen nichts davon sehen.
 *
 * ## Zwei Muster
 *
 * - **Klassisch** (Vorgabe): 220-Hz-Sinus + 120-BPM-Beat-Puls, exakt die
 *   Formel seit S41/S43. **Bit-identisch zum bisherigen Verhalten** — an
 *   diesem Signal hängen die Modul-Matrix, die Modul-Sonden und alle
 *   Feld-Sonden; wer es ändert, muss alles neu einmessen.
 * - **Musik**: aus einer echten Aufnahme abgeleitete Hüllkurven (acht
 *   log-verteilte Bänder je Bild plus Beat-Spur, siehe MusikProfil.hpp).
 *   Der Klang wird daraus NEU SYNTHETISIERT — es wird keine Musik abgespielt,
 *   sondern acht Sinus mit musikalisch bewegten Amplituden. Damit bleibt das
 *   Signal eine reine Funktion des Bild-Index und ist auf beiden Seiten
 *   identisch erzeugbar, hat aber die Dynamik echter Musik: Bass-Einsätze,
 *   Aufbauten, Stille zwischen den Schlägen.
 *
 * Nebenbei ist das Musik-Muster **in sich stimmiger** als das klassische: dort
 * hat das Spektrum (1/f-Rampe) nichts mit der Wellenform (ein 220-Hz-Sinus) zu
 * tun. Hier stammen beide aus denselben Hüllkurven — wer wie MilkDrop seine
 * FFT selbst aus dem PCM rechnet, sieht dasselbe Bild wie der, dem wir das
 * Spektrum fertig hinlegen.
 *
 * ## Zwei Geschmacksrichtungen im klassischen Muster
 *
 * Das SPEKTRUM war in den beiden Standalones nie gleich (die Wellenform schon):
 * der AVS-Pfad legt über alle Bins denselben Beat-Faktor, der MilkDrop-Pfad
 * gibt Bass/Mitten/Höhen eigene Hüllkurven (S64: ein gemeinsamer Faktor machte
 * die Bänder nach der Loudness-Normalisierung identisch, und Presets, die durch
 * Banddifferenzen teilen, liefen in 0/0-NaN). Beide Fassungen bleiben hier
 * erhalten — `Geschmack` wählt aus. Zusammenlegen wäre ein Eingriff in
 * bestehende Messreihen und gehört eigens entschieden.
 ****************************************************************************************
 */

#ifndef LUMI_SYNTH_AUDIO_HPP
#define LUMI_SYNTH_AUDIO_HPP

#include <algorithm>
#include <cmath>
#include <cstring>

namespace lumi
{
namespace synth
{

constexpr int kWaveFrames = 576;  ///< Rahmen je Bild in der Wellenform
constexpr int kBins = 512;        ///< Bänder je Kanal im Spektrum
constexpr double kPi = 3.14159265358979323846;
constexpr double kBildrate = 60.0;   ///< Bild-Index → Sekunden
constexpr double kAbtastrate = 44100.0;

/// Ein Bild Klang: beides interleaved L/R, so wie updateAudioStereo es nimmt
struct Frame
{
    float wave[kWaveFrames * 2];
    float spec[kBins * 2];
};

/// Welches Signal
enum class Muster
{
    Klassisch,  ///< 220-Hz-Sinus + Beat-Puls (Vorgabe, bit-identisch seit S41)
    Musik       ///< aus einer Aufnahme abgeleitete Hüllkurven (S74)
};

/// Spektrum-Geschmack des klassischen Musters (s. Dateikopf)
enum class Geschmack
{
    Avs,      ///< ein Beat-Faktor über alle Bins
    Milkdrop  ///< eigene Hüllkurven für Bass/Mitten/Höhen (S64)
};

/// Schalter, die es an den Werkzeugen schon gibt. Die Vorgaben ergeben exakt
/// das Signal, das bis S73 erzeugt wurde — nichts hier ändert Bestandsmessungen.
struct Optionen
{
    Muster muster = Muster::Klassisch;
    Geschmack geschmack = Geschmack::Avs;
    double beatHz = 2.0;          ///< MilkdropStandalone --beat-hz
    bool klangfarbe = false;      ///< MilkdropStandalone --klangfarbe
    bool audioBeat = false;       ///< MilkdropStandalone --audio-beat
    bool stereoSpektrum = false;  ///< AvsStandalone --stereo-spektrum
    bool stille = false;          ///< --silence
};

// =============================================================================================
// Musik-Profil — Datenvertrag
// =============================================================================================

/// Aus einer Aufnahme gewonnene Hüllkurven. Erzeugt von
/// `AvsStandalone --audio-profil-schreiben`, abgelegt als MusikProfil.hpp.
struct MusikProfil
{
    int baender;              ///< Anzahl Bänder je Bild (Kopf und Daten müssen passen)
    int bilder;               ///< Länge der Schleife in Bildern (60/s)
    const float* mitten;      ///< Mittenfrequenzen der Bänder in Hz [baender]
    const unsigned char* huellen;  ///< [bilder * baender], 0..255
    const unsigned char* beats;    ///< [bilder], 0/1
    const char* herkunft;     ///< Klartext-Vermerk, wo es herkommt
};

/// Liefert das eingebaute Profil. Definition in MusikProfil.hpp (erzeugt).
const MusikProfil& eingebautesProfil();

// =============================================================================================
// Klassisches Muster — die Formel seit S41/S43, unverändert
// =============================================================================================

namespace detail
{

inline void klassisch(double zeit, const Optionen& opt, Frame& aus)
{
    // Beat-Puls. Vorgabe 2 Hz = 120 BPM; --beat-hz ändert die Schlagfolge (S73).
    const double beat =
        0.55 + 0.45 * (std::max)(0.0, std::sin(zeit * 2.0 * kPi * opt.beatHz));

    // --klangfarbe (S73): die Spektralbalance wandert langsam zwischen Bass und
    // Höhen. Aus = 0 ⇒ exakt das alte Verhalten.
    const double kipp = opt.klangfarbe ? std::sin(zeit * 0.37) : 0.0;

    // Band-eigene Hüllkurven (S64) bzw. korrelierte mit Jitter (--audio-beat, S67)
    const double beatMid =
        opt.audioBeat ? beat * (0.97 + 0.02 * std::sin(zeit * 1.1))
                      : 0.55 + 0.45 * (std::max)(
                                          0.0, std::sin(zeit * 2.0 * kPi * 1.5 + 1.3));
    const double beatTreb =
        opt.audioBeat ? beat * (0.94 + 0.03 * std::sin(zeit * 1.7 + 0.5))
                      : 0.55 + 0.45 * (std::max)(
                                          0.0, std::sin(zeit * 2.0 * kPi * 2.7 + 2.1));

    for (int i = 0; i < kWaveFrames; ++i)
    {
        const double ph = zeit * 220.0 * 2.0 * kPi + i * (2.0 * kPi / 64.0);
        // Mit --klangfarbe zwei Oberwellen mit eigenem, langsamem Gewicht;
        // normiert, damit die Amplitude gleich bleibt.
        const double g2 = 0.5 + 0.5 * std::sin(zeit * 0.23);
        const double g3 = 0.5 + 0.5 * std::sin(zeit * 0.31 + 1.7);
        const double l =
            opt.klangfarbe
                ? (std::sin(ph) + g2 * 0.6 * std::sin(2.0 * ph) +
                   g3 * 0.4 * std::sin(3.0 * ph)) /
                      (1.0 + g2 * 0.6 + g3 * 0.4)
                : std::sin(ph);
        const double r =
            opt.klangfarbe
                ? (std::sin(ph + 0.7) + g2 * 0.6 * std::sin(2.0 * (ph + 0.7)) +
                   g3 * 0.4 * std::sin(3.0 * (ph + 0.7))) /
                      (1.0 + g2 * 0.6 + g3 * 0.4)
                : std::sin(ph + 0.7);
        aus.wave[i * 2 + 0] = static_cast<float>(beat * 0.5 * l);
        aus.wave[i * 2 + 1] = static_cast<float>(beat * 0.5 * r);
    }

    for (int b = 0; b < kBins; ++b)
    {
        float links = 0.0f;
        float rechts = 0.0f;
        if (opt.geschmack == Geschmack::Avs)
        {
            links = static_cast<float>(beat * 0.8 / (1.0 + b * 0.03));
            // --stereo-spektrum (S55): rechts fällt steiler ab, sonst sind
            // links, rechts und Mitte zwangsläufig identisch und Kanalfelder
            // nicht prüfbar.
            rechts = opt.stereoSpektrum
                         ? static_cast<float>(beat * 0.8 / (1.0 + b * 0.12))
                         : links;
        }
        else
        {
            // Bandgrenzen der MilkLoudness-Terzen (761,2/2897,1 Hz auf 512 Bins
            // linear bis 22050 Hz): Bin ~17,7 bzw. ~67,3
            const double env = (b < 18) ? beat : (b < 68) ? beatMid : beatTreb;
            const double abfall = 0.03 * (1.0 - 0.85 * kipp);
            const double gewicht = (b < 18)   ? 1.0 - 0.60 * (std::max)(0.0, kipp)
                                   : (b < 68) ? 1.0 - 0.30 * std::abs(kipp)
                                              : 1.0 + 0.80 * (std::max)(0.0, kipp);
            links = static_cast<float>(env * 0.8 * gewicht / (1.0 + b * abfall));
            rechts = links;
        }
        aus.spec[b * 2 + 0] = links;
        aus.spec[b * 2 + 1] = rechts;
    }
}

// =============================================================================================
// Musik-Muster — aus den Hüllkurven neu synthetisiert
// =============================================================================================

/// Hüllkurve eines Bandes für ein Bild, linear zwischen den Stützstellen
/// interpoliert. Läuft die Schleife aus, beginnt sie von vorn.
inline double huelle(const MusikProfil& p, int bild, int band)
{
    if (p.bilder <= 0) return 0.0;
    int i = bild % p.bilder;
    if (i < 0) i += p.bilder;
    return p.huellen[static_cast<size_t>(i) * p.baender + band] / 255.0;
}

inline void musik(int bild, const Optionen& opt, Frame& aus)
{
    const MusikProfil& p = eingebautesProfil();
    if (p.bilder <= 0 || p.baender <= 0)
    {
        std::memset(&aus, 0, sizeof(aus));
        return;
    }

    // --- Wellenform: acht Sinus an den Bandmitten, Amplitude = Hüllkurve ---------------
    // Die 576 Rahmen sind ein Ausschnitt echten 44,1-kHz-PCMs an der Stelle
    // `bild/60` — genau wie ein Visualizer nur die jüngsten 576 Samples sieht.
    // Die Phase läuft über die absolute Zeit, damit sie über Bildgrenzen
    // stetig bleibt und nicht je Bild neu anspringt.
    const double t0 = bild / kBildrate;
    double summe = 0.0;
    for (int k = 0; k < p.baender; ++k) summe += huelle(p, bild, k);
    // Normierung auf denselben Spitzenwert wie das klassische Muster (0,5),
    // damit Loudness-Vergleiche zwischen den Mustern nicht verrutschen
    const double norm = (summe > 1e-6) ? 0.5 / summe : 0.0;

    for (int i = 0; i < kWaveFrames; ++i)
    {
        const double t = t0 + i / kAbtastrate;
        double l = 0.0;
        double r = 0.0;
        for (int k = 0; k < p.baender; ++k)
        {
            const double a = huelle(p, bild, k);
            if (a <= 0.0) continue;
            const double f = p.mitten[k];
            // Feste, band-eigene Phasenversätze: ohne die stünden alle acht
            // Sinus bei t=0 auf demselben Nulldurchgang und die Wellenform
            // bekäme einen unnatürlichen Impuls je Periode.
            const double phi = k * 0.7853981633974483;  // k * pi/4
            l += a * std::sin(2.0 * kPi * f * t + phi);
            // Rechts leicht verstimmt statt bloß phasenverschoben — das gibt
            // eine echte Stereobreite, die sich über die Zeit ändert.
            r += a * std::sin(2.0 * kPi * f * 1.002 * t + phi + 0.7);
        }
        aus.wave[i * 2 + 0] = static_cast<float>(norm * l);
        aus.wave[i * 2 + 1] = static_cast<float>(norm * r);
    }

    // --- Spektrum: dieselben Hüllkurven, auf 512 lineare Bins gelegt --------------------
    // Bin b deckt b * 22050/512 Hz ab. Zwischen den Bandmitten wird im
    // Log-Frequenzraum interpoliert, damit kein Treppenprofil entsteht.
    for (int b = 0; b < kBins; ++b)
    {
        const double f = (b + 0.5) * (kAbtastrate * 0.5) / kBins;
        double wert;
        if (f <= p.mitten[0])
        {
            wert = huelle(p, bild, 0);
        }
        else if (f >= p.mitten[p.baender - 1])
        {
            wert = huelle(p, bild, p.baender - 1);
        }
        else
        {
            int k = 0;
            while (k + 1 < p.baender && p.mitten[k + 1] < f) ++k;
            const double lf = std::log(f);
            const double l0 = std::log(static_cast<double>(p.mitten[k]));
            const double l1 = std::log(static_cast<double>(p.mitten[k + 1]));
            const double s = (l1 > l0) ? (lf - l0) / (l1 - l0) : 0.0;
            wert = huelle(p, bild, k) * (1.0 - s) + huelle(p, bild, k + 1) * s;
        }
        // 0,8 = Spitzenwert des klassischen Musters, damit Presets mit fester
        // Schwelle (`above(getspec(...), 0.5)`) in beiden Mustern zünden
        const float v = static_cast<float>(wert * 0.8);
        aus.spec[b * 2 + 0] = v;
        aus.spec[b * 2 + 1] = opt.stereoSpektrum ? v * 0.75f : v;
    }
}

}  // namespace detail

// =============================================================================================
// Öffentliche Schnittstelle
// =============================================================================================

/// Ein Bild Klang erzeugen. REINE FUNKTION des Bild-Index — zwei Läufe mit
/// derselben Bildzahl liefern bitgleiche Puffer, auf beiden Seiten des
/// Vergleichs.
inline void erzeuge(int bild, const Optionen& opt, Frame& aus)
{
    if (opt.stille)
    {
        std::memset(&aus, 0, sizeof(aus));
        return;
    }
    if (opt.muster == Muster::Musik)
    {
        detail::musik(bild, opt, aus);
    }
    else
    {
        detail::klassisch(bild / kBildrate, opt, aus);
    }
}

/// Beat-Spur des Musik-Musters. Im klassischen Muster gibt es keine — dort
/// bleibt es beim Detektor bzw. bei `--beat-period`.
inline bool istBeat(int bild, const Optionen& opt)
{
    if (opt.muster != Muster::Musik || opt.stille) return false;
    const MusikProfil& p = eingebautesProfil();
    if (p.bilder <= 0) return false;
    int i = bild % p.bilder;
    if (i < 0) i += p.bilder;
    return p.beats[i] != 0;
}

/// Mustername aus einem Schalterwert; liefert false bei Unfug.
inline bool musterAusText(const char* text, Muster& aus)
{
    if (text == nullptr) return false;
    if (std::strcmp(text, "klassisch") == 0) { aus = Muster::Klassisch; return true; }
    if (std::strcmp(text, "musik") == 0) { aus = Muster::Musik; return true; }
    return false;
}

}  // namespace synth
}  // namespace lumi

// Die Profil-Daten stehen in einer ERZEUGTEN Datei. Sie hier unten einzubinden
// haelt SynthAudio.hpp fuer alle vier Werkzeuge selbstgenuegsam — der Typ
// `MusikProfil` ist an dieser Stelle bereits bekannt, und der Include-Waechter
// der Profil-Datei bricht die Rueckbindung auf.
#include "MusikProfil.hpp"

#endif  // LUMI_SYNTH_AUDIO_HPP
