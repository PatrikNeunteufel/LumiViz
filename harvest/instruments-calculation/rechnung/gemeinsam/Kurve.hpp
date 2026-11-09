/**
 * @file Kurve.hpp
 * @brief Gemeinsam: eine Groesse als Kurve ueber den Tonumfang — Stuetzpunkte (Note, Wert), dazwischen
 *        monoton kubisch interpoliert, Abweichungen je Note, Grenzen durch Klemmen, Herkunft als Text
 *        (AUFTRAG_BAU §3; Lehre L2: gesetzt werden Kurven, nicht Werte je Note)
 *
 * Nach `Tastenkurve` im Vorbild (gemeinsam/vorbild-lumiviz/code/loop/HammerSaite.hpp, HK1 a): zwischen den
 * Punkten wird in der Skala der Groesse monoton kubisch interpoliert (Fritsch–Butland wie PCHIP), vor dem ersten
 * und nach dem letzten Punkt gilt der Randwert; eine Note kann eine Abweichung tragen, die den Wert der Kurve dort
 * ersetzt. Die Rechnung ist Bit fuer Bit die des Vorbilds (geprueft im Test der Hammer-Saite).
 *
 * Abweichung vom Vorbild: der Tonumfang ist nicht fest A0 … C8, sondern wird beim Richten angegeben (jede Klasse
 * hat ihren Umfang, MIDI 0 … 127); jede Kurve traegt ihre Herkunft (L2, AUFTRAG_BAU §4 Punkt 2: „Quelle mit Seite,
 * Rechnung mit Skript, oder ‚offen‘ mit Vorgabe und Grund“). Reines C++, kein Qt.
 */
#pragma once

#include <string>
#include <vector>

namespace ic
{

/// Der groesste Tonumfang einer Kurve (MIDI).
inline constexpr int kNoteMin = 0;
inline constexpr int kNoteMax = 127;

/// In welcher Skala eine Groesse zwischen den Stuetzpunkten verlaeuft.
enum class Skala : int
{
    Linear = 0,
    /// in ln(Wert) interpoliert — nur fuer Groessen ueber null
    Logarithmisch = 1,
};

/// Wie belegt eine Kurve ist (AUFTRAG_BAU §7: keine Zahl ohne Herkunft).
enum class Belegkraft : int
{
    Quelle = 0,      ///< aus einer Quelle mit Seite (selbst nachgelesen)
    Rechnung = 1,    ///< gerechnet, Skript oder Formel im Repo
    Gemessen = 2,    ///< an einer Aufnahme gemessen und durch das Modell gesetzt
    Offen = 3,       ///< „offen“: Vorgabe mit Grund, keine Quelle
};

/// Ein Wert an einer Note.
struct NotenWert
{
    int note = 60;
    double wert = 0.0;

    [[nodiscard]] bool operator==(const NotenWert&) const = default;
};

/// Eine Groesse ueber den Tonumfang: Stuetzpunkte, Abweichungen je Note, Herkunft.
struct Kurve
{
    std::vector<NotenWert> punkte;
    /// eine Note mit eigenem Wert; leer = ueberall die Kurve
    std::vector<NotenWert> abweichungen;
    Belegkraft belegkraft = Belegkraft::Offen;
    /// woher die Werte kommen: Quelle mit Seite, Rechnung, oder Grund der Vorgabe
    std::string herkunft;

    /// gleich heisst: dieselben Werte (die Herkunft zaehlt nicht zum Klang)
    [[nodiscard]] bool operator==(const Kurve& o) const { return punkte == o.punkte && abweichungen == o.abweichungen; }
};

/// Der Wert an einer Note: die Abweichung, wenn die Note eine traegt, sonst die Kurve. Ohne Stuetzpunkte 0. Eine
/// Note ausserhalb gilt als die naechste am Rand.
[[nodiscard]] double wertAn(const Kurve& kurve, int note, Skala skala);

/// Eine Kurve in ihre Grenzen: Noten auf `noteMin` … `noteMax`, nach Note sortiert, je Note der erste Eintrag,
/// Werte auf `von` … `bis` (nicht endliche fallen weg). Fehlen alle Stuetzpunkte, gilt `ersatz`.
void richteKurve(Kurve& kurve, double von, double bis, const Kurve& ersatz, int noteMin = kNoteMin, int noteMax = kNoteMax);

/// Eine Kurve aus Punkten mit Herkunft.
[[nodiscard]] Kurve kurveAusPunkten(std::initializer_list<NotenWert> punkte, Belegkraft belegkraft = Belegkraft::Offen,
                                    std::string herkunft = {});

/// Eine flache Kurve (ein Wert ueber den ganzen Umfang).
[[nodiscard]] Kurve kurveKonstant(double wert, Belegkraft belegkraft, std::string herkunft, int noteMin = kNoteMin,
                                  int noteMax = kNoteMax);

/// Die Grenzen einer Groesse mit ihrem Beleg (wie SQ2 a beim Fluegel).
struct Grenzen
{
    double von = 0.0;
    double bis = 0.0;
    std::string beleg;
};

/// Wie eine Kurve als Text steht (fuer Berichte und `modell.md`): „n:w n:w …“.
[[nodiscard]] std::string kurveAlsText(const Kurve& kurve);

}  // namespace ic
