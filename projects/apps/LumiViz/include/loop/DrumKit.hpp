/**
 * @file DrumKit.hpp
 * @brief Loop: Kits als eigene Einheit — Rollen, die Datei `.lvkit`, der
 *        Kitwechsel (docs/audio/Loop_Konzept.md §4l, Entscheide K1 a bis K9 a,
 *        Patrik S123; Stufe 21a)
 *
 * Ein Kit nennt je **Rolle** einen Klang mit Volume und Pan (K3 a). Eine
 * Drum-Spur traegt ihre Rolle (`Spur::rolle`, K1 a); ein Kitwechsel gibt jeder
 * Spur den Klang ihrer Rolle aus dem neuen Kit und laesst die Schritte stehen.
 *
 * - **Rollen** (K5 a): kick, snare, clap, rim, hat, open hat, ride, crash, tom,
 *   perc, fx — jede nummeriert mehrfach. Als Text: tom und perc immer mit
 *   Nummer („tom 1"), die anderen erst ab der zweiten („crash 2").
 * - **Kitwechsel** (K2 a, K5 a): gleiche Rolle mit gleicher Nummer; fehlt die
 *   Nummer im Kit, die hoechste vorhandene derselben Art; fehlt die Art, behaelt
 *   die Spur ihren Klang. Rollen des Kits ohne Spur werden nicht angelegt.
 * - **Ablage** (K3 a): mitgeliefert unter `asset/loop/kits`, beim Dateinamen
 *   genannt (`808`); eigene im Benutzerordner unter `kits`, als `user:<name>`;
 *   sonst der Pfad einer Datei.
 */
#pragma once

#include "loop/Loop.hpp"

#include <QJsonObject>
#include <QString>
#include <QStringList>

#include <optional>
#include <vector>

namespace lumi::loop
{

// --- Rollen ------------------------------------------------------------------

/// Die Arten von Rollen, in der Reihenfolge, in der sie angeboten werden.
[[nodiscard]] const QStringList& rollenArten();

/// Eine Rolle: ihre Art und ihre Nummer (ab 1).
struct Rolle
{
    QString art;
    int nummer = 1;

    [[nodiscard]] bool operator==(const Rolle&) const = default;
};

/// „tom 2" → {tom, 2}; „Open Hat" → {open hat, 1}; „tom" → {tom, 1}. Leer, wenn
/// die Art keine bekannte ist oder die Nummer kein Zaehler ab 1.
[[nodiscard]] std::optional<Rolle> leseRolle(const QString& text);
/// Die Rolle als Text: tom und perc immer mit Nummer, die anderen ab der zweiten.
[[nodiscard]] QString rollenText(const Rolle& rolle);

/// Die Art der Rolle, die eine Spur wohl spielt — aus ihrem Instrument, ihrem
/// Klang, sonst ihrem Namen (K1 a). Leer = nicht zu erraten.
[[nodiscard]] QString rateRollenArt(const Spur& spur);
/// Gibt jeder Drum-Spur ohne Rolle eine geratene und nummeriert je Art weiter,
/// ohne vergebene Nummern doppelt zu nutzen; Toms von tief nach hoch, wenn der
/// Name es sagt (low, floor vor high). Spuren mit Rolle bleiben, wie sie sind.
void rateRollen(Muster& muster);

// --- Das Kit -----------------------------------------------------------------

/// Ein Klang des Kits fuer eine Rolle — die Felder wie an einer Drum-Spur.
struct KitKlang
{
    QString rolle;                ///< als Text (`rollenText`)
    QString klang;                ///< `kit:`, `lib:`, `user:` oder ein Pfad; bleibt neben einem Instrument stehen
    QString instrument;           ///< leer = es spielt `klang`
    int hoehe = 36;               ///< Note des Instruments (C2, wie die Drums gebaut sind)
    int dauerMs = kSpurDauerMs;   ///< Length des Instruments
    double laut = 1.0;            ///< 0…1
    double pan = 0.0;             ///< -1…1

    [[nodiscard]] bool operator==(const KitKlang&) const = default;
};

struct DrumKit
{
    QString name;         ///< die Nennung: `808`, `user:mein_trap` oder der Pfad; steht nicht in der Datei
    QString titel;
    QString beschreibung;
    std::vector<KitKlang> klaenge;

    /// Der Klang fuer eine Rolle nach den Regeln des Kitwechsels: gleiche Nummer,
    /// sonst die hoechste derselben Art; nullptr = die Art fehlt.
    [[nodiscard]] const KitKlang* fuer(const Rolle& rolle) const;

    [[nodiscard]] bool operator==(const DrumKit&) const = default;
};

inline constexpr int kKitFassung = 1;
inline constexpr const char* kKitDateiEndung = ".lvkit";

/// Als JSON; ein Pfad unter `basisOrdner` steht relativ (wie in der `.lvloop`).
[[nodiscard]] QJsonObject alsJson(const DrumKit& kit, const QString& basisOrdner = {});
/// Aus JSON; unbekannte Rollen und doppelte fallen weg. `zuNeu` = eine Fassung
/// von morgen (dann false).
bool ausJson(const QJsonObject& wurzel, DrumKit& kit, const QString& basisOrdner = {}, bool* zuNeu = nullptr);
bool ladeKit(const QString& pfad, DrumKit& kit, bool* zuNeu = nullptr);
bool speichereKit(const QString& pfad, const DrumKit& kit);

// --- Ablage ------------------------------------------------------------------

/// `asset/loop/kits` — neben der Exe oder im Projektordner. Leer = nicht da.
[[nodiscard]] QString mitgelieferterKitOrdner();
/// Die mitgelieferten Kits, nach Titel; einmal gelesen.
[[nodiscard]] const std::vector<DrumKit>& mitgelieferteKits();
/// Der Ordner der eigenen Kits im Benutzerordner (`kits`).
[[nodiscard]] QString eigenerKitOrdner();
void setzeEigenenKitOrdnerFuerTests(const QString& ordner);
/// Die Datei eines eigenen Kits beim Namen (`eigenerName`); leer = kein gueltiger Name.
[[nodiscard]] QString eigeneKitDatei(const QString& name);
/// Die eigenen Kits als `user:<name>`, nach Titel; jedes Mal neu gelesen.
[[nodiscard]] std::vector<DrumKit> eigeneKits();
/// Ein Kit nach seiner Nennung holen: mitgeliefert, `user:<name>` oder Pfad.
bool holeKit(const QString& nennung, DrumKit& kit);

// --- Kitwechsel --------------------------------------------------------------

struct KitWechsel
{
    int getroffen = 0;            ///< so viele Spuren haben einen Klang aus dem Kit bekommen
    QStringList ohneGegenstueck;  ///< Rollen der Spuren, die das Kit nicht hat (sie behalten ihren Klang)
    QStringList fehlend;          ///< Rollen des Kits ohne Spur (fuer Add track, K2 a)
};

/// Das Kit auf ein Drum-Muster anwenden: erst die fehlenden Rollen raten, dann
/// je Spur Klang, Instrument, Note, Length, Volume und Pan aus dem Kit; die
/// Schritte bleiben. Setzt `muster.kit`.
KitWechsel wendeKitAn(Muster& muster, const DrumKit& kit);
/// Weicht das Muster vom Kit ab, das es nennt — „(changed)" (K4 a)? Eine Spur
/// ohne Rolle oder mit einer Rolle, die das Kit nicht hat, zaehlt nicht.
[[nodiscard]] bool weichtAb(const Muster& muster, const DrumKit& kit);
/// Ein Kit aus den Spuren eines Musters (Save as kit...): je Spur mit Rolle ein
/// Klang; Spuren ohne Rolle bekommen vorher eine geratene.
[[nodiscard]] DrumKit kitAusMuster(const Muster& muster, const QString& titel);

// --- Wer ein Kit nennt (Stufe 21c, „Used by" im Fenster Kits) ----------------

/// So viele zuletzt geoeffnete Loops merkt sich das Programm.
inline constexpr int kZuletztLoopsMax = 10;
/// Ein Loop ist geladen oder gespeichert worden: vorn in die Liste.
void merkeGeoeffnetenLoop(const QString& pfad);
/// Die zuletzt geoeffneten Loops, juengster zuerst, nur die, die es noch gibt.
[[nodiscard]] QStringList zuletztGeoeffneteLoops();
/// Die mitgelieferten Loops: jede `.lvloop` unter `asset/loop`, auch in Unterordnern.
[[nodiscard]] QStringList mitgelieferteLoopDateien();
/// Nennt ein Muster des Loops dieses Kit (Feld `kit`, K4 a)?
[[nodiscard]] bool nenntKit(const Loop& loop, const QString& nennung);
/// Die Dateien unter `dateien`, deren Loop das Kit nennt — beim Dateinamen ohne
/// Endung, jede einmal, in der Reihenfolge von `dateien`.
[[nodiscard]] QStringList loopsMitKit(const QString& nennung, const QStringList& dateien);

}  // namespace lumi::loop
