/**
 * @file Werkzeug.hpp
 * @brief Gemeinsam: WAV-Schreiber (32-bit float), Pegel (Spitze, RMS, LUFS-Naeherung nach ITU-R BS.1770),
 *        Abdruck (SHA-256 eines Tons als Bitgleich-Waechter), Rechenzeit, Pool fuer unabhaengige Aufgaben
 *        (AUFTRAG_BAU §3 „Fft-Schnittstelle, WAV-Schreiber, Pegel, Rechenzeit-Messung, Abdruck“)
 *
 * - **Pegel:** die LUFS-Naeherung ist die K-Bewertung nach ITU-R BS.1770-4 (Vorfilter Kugel + RLB-Hochpass) mit dem
 *   Integral ueber den ganzen Ton ohne Gating — fuer das Hoerproben-Werkzeug (gleich laut normiert) reicht das; der
 *   Bezug −13,4 LUFS gilt erst in LumiViz. Die Filter sind fuer jede Rate aus den analogen Groessen gebildet
 *   (Kugel: f0 = 1681,974 Hz, G = 3,99984 dB, Q = 0,70718; RLB: f0 = 38,135 Hz, Q = 0,50033 — die Formeln wie in
 *   libebur128 (MIT), **aus dem Gedaechtnis, offen**: geprueft wird im Test gegen die Koeffizienten bei 48 kHz aus
 *   BS.1770-4 Tab. 1/2, die als Zahlen im Test stehen — ebenfalls nicht selbst nachgelesen. Vorgabe fuer das
 *   Werkzeug (gleich laut normierte Hoerproben), nicht fuer Messungen).
 * - **Abdruck:** SHA-256 ueber die Bytes der float-Werte in Little-Endian (FIPS 180-4), hex.
 * - **Pool:** `parallelJe` rechnet unabhaengige Aufgaben auf Threads; jede Aufgabe schreibt nur ihr eigenes Ergebnis,
 *   die Reihenfolge der Threads aendert nichts (AUFTRAG_BAU §8 Determinismus, Vorbild `parallelJe`).
 *
 * Reines C++, kein Qt.
 */
#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <span>
#include <string>
#include <vector>

namespace ic
{

// --- WAV ---------------------------------------------------------------------------------------------------

/// Schreibt eine WAV-Datei mit 32-bit float, `kanaele` verschraenkt. false, wenn die Datei nicht geschrieben wurde.
bool schreibeWav(const std::string& pfad, std::span<const float> werte, int rate, int kanaele = 1);

/// Liest eine WAV-Datei (16/24-bit PCM oder 32-bit float) als float, verschraenkt. false bei einem Fehler.
bool leseWav(const std::string& pfad, std::vector<float>& werte, int& rate, int& kanaele);

// --- Pegel -------------------------------------------------------------------------------------------------

[[nodiscard]] double spitze(std::span<const float> werte);
[[nodiscard]] double rms(std::span<const float> werte);
/// 20·lg(x), mit −300 fuer x ≤ 0.
[[nodiscard]] double dB(double x);
/// Die Lautheit eines Mono-Tons nach BS.1770 (K-Bewertung, ohne Gating) in LUFS — Naeherung fuer das Werkzeug.
[[nodiscard]] double lufsNaeherung(std::span<const float> mono, int rate);
/// Die integrierte Lautheit eines Mono-Tons nach ITU-R BS.1770-4 in LUFS (HT14 a, Sync Hoertest IC-2): K-Bewertung,
/// Bloecke von 400 ms mit 75 % Ueberlappung, absolutes Gate −70 LUFS, relatives Gate 10 LU unter dem Mittel der
/// Bloecke ueber dem absoluten. Kuerzer als ein Block: der ganze Ton ohne Gate. Nichts ueber den Gates: −300.
[[nodiscard]] double lautheit(std::span<const float> mono, int rate);
/// Die Koeffizienten der beiden K-Filter bei `rate`: {b0, b1, b2, a1, a2} je Stufe (fuer die Tests).
struct Biquad
{
    double b0 = 1.0, b1 = 0.0, b2 = 0.0, a1 = 0.0, a2 = 0.0;
};
[[nodiscard]] Biquad kFilterKugel(int rate);
[[nodiscard]] Biquad kFilterHochpass(int rate);
/// Der RMS je Block von `blockS` Sekunden in dB (der letzte Block kuerzer) — wie die Spalte „dB je 0.5 s“ des
/// Bezugs im Vorbild.
[[nodiscard]] std::vector<double> dbJeBlock(std::span<const float> mono, int rate, double blockS);

// --- Abdruck -----------------------------------------------------------------------------------------------

/// SHA-256 stueckweise (fuer grosse Dateien).
class Sha256
{
public:
    Sha256();
    void dazu(const std::uint8_t* daten, std::size_t n);
    /// Der Abdruck (hex); danach ist das Objekt verbraucht.
    [[nodiscard]] std::string fertig();

private:
    std::uint32_t m_h[8];
    std::uint8_t m_puffer[64];
    std::size_t m_imPuffer = 0;
    std::uint64_t m_laenge = 0;
};

/// SHA-256 (hex) ueber Bytes.
[[nodiscard]] std::string sha256(std::span<const std::uint8_t> bytes);
/// SHA-256 (hex) ueber die float-Werte eines Tons (Little-Endian).
[[nodiscard]] std::string abdruck(std::span<const float> werte);

// --- Rechenzeit --------------------------------------------------------------------------------------------

/// Eine Stoppuhr (steady_clock).
class Stoppuhr
{
public:
    Stoppuhr() : m_start(std::chrono::steady_clock::now()) {}
    [[nodiscard]] double sekunden() const
    {
        return std::chrono::duration<double>(std::chrono::steady_clock::now() - m_start).count();
    }

private:
    std::chrono::steady_clock::time_point m_start;
};

/// Rechenzeit eines Tons: Anteil der Echtzeit (Rechenzeit / Dauer des Tons), das Mittel ueber `wiederholungen`.
[[nodiscard]] double echtzeitAnteil(const std::function<void()>& rechne, double tonDauerS, int wiederholungen = 3);

/// Die Grenze der Rechenzeit je Ton im tiefsten Ton: rund 5 % der Echtzeit (Fluegel: 4,6 %, SCHNITTSTELLE.md);
/// darueber traegt ein Verfahren die Marke **Deluxe** (AUFTRAG_BAU §4 Punkt 3e).
inline constexpr double kEchtzeitGrenze = 0.05;

// --- Pool --------------------------------------------------------------------------------------------------

/// Rechnet `aufgabe(i)` fuer i = 0 … anzahl − 1 auf bis zu `threads` Threads (0 = Zahl der Kerne, hoechstens 11 —
/// L7: mehr als 11 Threads wurden beim Fluegel langsamer). Jede Aufgabe muss fuer sich rechnen.
void parallelJe(std::size_t anzahl, const std::function<void(std::size_t)>& aufgabe, unsigned threads = 0);

}  // namespace ic
