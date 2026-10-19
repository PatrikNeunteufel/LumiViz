/**
 * @file Akkord.hpp
 * @brief Loop: Akkorde als eigene Einheit — Griffbild oder Voicing, die Datei
 *        `.lvchord`, die Umrechnung in Noten (docs/audio/Saiten_Konzept.md §1,
 *        Entscheide Patrik S137: S-T1 a bis S-T6 a; Stufe 26a)
 *
 * Ein Akkord ist Grundton und Art (C-Dur). Gespielt wird er je nach Instrument
 * verschieden (S-T6 a):
 *
 * - **Griffbild** (S-T3 a): fuer eine Stimmung — je Saite ein Bund, leer oder
 *   nicht gespielt, dazu die Finger. Es setzt Noten mit ihren Saiten; auf dem
 *   Griffbrett verschoben (Lage) wandert jeder gespielte Bund mit, eine leere
 *   Saite wird gegriffen.
 * - **Voicing** (S-T6 a): die Toene als Abstaende ueber dem tiefsten, ohne
 *   Stimmung — auf jeden Ton zu setzen.
 *
 * Ein Akkord setzt nur Noten (S-T1 a): das Muster merkt sich nicht, aus welchem
 * Akkord sie kamen.
 *
 * - **Ablage** (S-T3 a): mitgeliefert unter `asset/loop/chords` (auch in
 *   Unterordnern, beim Pfad darunter ohne Endung genannt: `guitar/c_major_1`);
 *   eigene im Benutzerordner unter `chords`, als `user:<name>`; sonst der Pfad
 *   einer Datei.
 *
 * Kein Qt-Widget, keine Audio-Engine.
 */
#pragma once

#include "loop/Loop.hpp"
#include "loop/Saiten.hpp"

#include <QJsonObject>
#include <QString>
#include <QStringList>

#include <optional>
#include <vector>

namespace lumi::loop
{

/// Die Namen der zwoelf Grundtoene, wie sie in der Datei stehen (von Claude
/// gesetzt: mit # oder b, wie Spieler sie meist schreiben).
[[nodiscard]] const QStringList& grundtonNamen();
/// „C#", „Db", „c#" → 1; unbekannt = nichts.
[[nodiscard]] std::optional<int> grundtonAusName(const QString& name);

/// Herkunft eines Akkords (S-T4 a): aus einer Quelle oder nach einer Regel
/// gerechnet — das Fenster zeigt gerechnete mit Hinweis.
inline constexpr const char* kHerkunftGerechnet = "computed";

/// Ein Griffbild oder ein Voicing.
struct Akkord
{
    QString name;          ///< die Nennung: `guitar/c_major_1`, `user:mein_c` oder der Pfad; steht nicht in der Datei
    QString titel;         ///< wie es im Programm steht: „C major“
    int grundton = 0;      ///< 0 = C … 11 = B
    QString art;           ///< `major`, `minor`, `7`, … — wie die Quelle sie nennt
    QString herkunft;      ///< leer = aus einer Quelle oder selbst gezeichnet; `computed` (S-T4 a)
    QString beschreibung;  ///< ein Satz, darf fehlen

    // --- Griffbild (S-T3 a): `leer` nicht leer
    /// Die Stimmung, fuer die es gilt: die Toene der leeren Saiten wie
    /// `Saitensatz::leer` (bei der Gitarre von der tiefen E-Saite an).
    std::vector<int> leer;
    /// Je Saite (wie `leer`): −1 nicht gespielt, 0 leer, sonst der Bund (ab dem Capo).
    std::vector<int> buende;
    /// Je Saite der Finger: 0 keiner, 1 Zeige- … 4 kleiner Finger.
    std::vector<int> finger;
    /// Die Buende, ueber die ein Finger mehrere Saiten greift (Barre), aufsteigend.
    std::vector<int> barre;

    // --- Voicing (S-T6 a): `leer` leer
    /// Die Toene als Halbtoene ueber dem tiefsten, aufsteigend, der erste 0:
    /// `[0, 4, 7]` (Grundstellung), `[0, 3, 8]` (erste Umkehrung von C-Dur auf E).
    std::vector<int> abstaende;

    [[nodiscard]] bool istGriffbild() const { return !leer.empty(); }

    [[nodiscard]] bool operator==(const Akkord&) const = default;
};

inline constexpr int kAkkordFassung = 1;
inline constexpr const char* kAkkordDateiEndung = ".lvchord";
/// So viele Toene traegt ein Voicing hoechstens (von Claude gesetzt: zwei Haende).
inline constexpr int kVoicingToeneMax = 10;
/// So weit reicht ein Voicing ueber seinem tiefsten Ton (von Claude gesetzt: vier Oktaven).
inline constexpr int kVoicingWeiteMax = 48;

/// Werte in ihre Grenzen: Saiten hoechstens `kSaitenMax`, Buende −1 … `kBuendeMax`,
/// Finger 0 … 4, `finger` so lang wie `leer`; ein Voicing aufsteigend ohne doppelte
/// Toene, mit 0 vorn, in 0 … `kVoicingWeiteMax`, hoechstens `kVoicingToeneMax`.
void richteAkkord(Akkord& akkord);

/// Als JSON (Format `lumiviz-chord`, Fassung 1): `title`, `root`, `type`,
/// `origin` (nur wenn gesetzt), `description`; ein Griffbild unter `strings`
/// (`open`, `frets`, `fingers`, `barres`), ein Voicing als `voicing`.
[[nodiscard]] QJsonObject alsJson(const Akkord& akkord);
/// Aus JSON; `zuNeu` = eine Fassung von morgen (dann false). Ein Akkord ohne
/// gespielte Saite oder ohne Toene gilt nicht. `name` bleibt.
bool ausJson(const QJsonObject& wurzel, Akkord& akkord, bool* zuNeu = nullptr);
bool ladeAkkord(const QString& pfad, Akkord& akkord, bool* zuNeu = nullptr);
bool speichereAkkord(const QString& pfad, const Akkord& akkord);

// --- Ablage ------------------------------------------------------------------

/// `asset/loop/chords` — neben der Exe oder im Projektordner. Leer = nicht da.
[[nodiscard]] QString mitgelieferterAkkordOrdner();
/// Die mitgelieferten Akkorde, nach Titel; einmal gelesen.
[[nodiscard]] const std::vector<Akkord>& mitgelieferteAkkorde();
/// Der Ordner der eigenen Akkorde im Benutzerordner (`chords`).
[[nodiscard]] QString eigenerAkkordOrdner();
void setzeEigenenAkkordOrdnerFuerTests(const QString& ordner);
/// Die Datei eines eigenen Akkords beim Namen (`eigenerName`); leer = kein gueltiger Name.
[[nodiscard]] QString eigeneAkkordDatei(const QString& name);
/// Die eigenen Akkorde als `user:<name>`, nach Titel; jedes Mal neu gelesen.
[[nodiscard]] std::vector<Akkord> eigeneAkkorde();
/// Einen Akkord nach seiner Nennung holen: mitgeliefert, `user:<name>` oder Pfad.
bool holeAkkord(const QString& nennung, Akkord& akkord);

// --- Was zu einem Instrument passt, und die Noten -----------------------------

/// Passt das Griffbild zu den Saiten (dieselben Toene der leeren Saiten; das Capo
/// zaehlt nicht)? Ein Voicing passt zu jedem Instrument ohne Saiten (S-T6 a).
[[nodiscard]] bool passtZu(const Akkord& akkord, const Saitensatz& saiten);
/// Die Akkorde aus `alle`, die zu den Saiten passen, in ihrer Reihenfolge.
[[nodiscard]] std::vector<Akkord> passendeAkkorde(const std::vector<Akkord>& alle, const Saitensatz& saiten);

/// Ein Ton eines Akkords: die Hoehe und (beim Griffbild) die Saite.
struct AkkordTon
{
    int hoehe = 60;
    int saite = -1;

    [[nodiscard]] bool operator==(const AkkordTon&) const = default;
};

/// Die Toene eines Griffbilds auf diesen Saiten, um `verschiebung` Buende auf dem
/// Griffbrett verschoben: jeder gespielte Bund wandert mit, eine leere Saite wird
/// auf `verschiebung` gegriffen; mit dem Capo der Saiten. Tiefste Saite zuerst.
/// Nichts, wenn es nicht passt oder ein Bund ausserhalb 0 … `kBuendeMax` laege.
[[nodiscard]] std::optional<std::vector<AkkordTon>> toeneDesGriffbilds(const Akkord& akkord, const Saitensatz& saiten,
                                                                      int verschiebung = 0);
/// Die Toene eines Voicings mit dem tiefsten auf `tiefster`; nichts, wenn einer
/// ausserhalb `kHoeheMin` … `kHoeheMax` laege.
[[nodiscard]] std::optional<std::vector<AkkordTon>> toeneDesVoicings(const Akkord& akkord, int tiefster);

/// Die Noten fuer ein Muster: alle am Schritt `schritt`, `laenge` Schritte lang,
/// Lautstaerke 1; ein Griffbild setzt die Saite jeder Note (S-M2 a), ein Voicing
/// keine. `bezug` ist beim Voicing der tiefste Ton, beim Griffbild die
/// Verschiebung in Buenden. Leer, wenn der Akkord so nicht zu setzen ist.
[[nodiscard]] std::vector<Note> akkordNoten(const Akkord& akkord, const Saitensatz& saiten, int schritt, int laenge,
                                            int bezug);

/// Das Griffbild um `verschiebung` Buende auf dem Griffbrett verschoben (S-T8 a F):
/// jeder gespielte Bund wandert mit, eine leere Saite wird auf `verschiebung`
/// gegriffen, die Barres wandern mit; Finger bleiben. Nichts, wenn es kein
/// Griffbild ist oder ein Bund ausserhalb 0 … `kBuendeMax` laege.
[[nodiscard]] std::optional<Akkord> verschobenesGriffbild(const Akkord& akkord, int verschiebung);

// --- Arten und Namen (Stufe 26e, S-T11 a L) --------------------------------------

/// Eine Art: der Name wie in den Vorlagen (chords-db) und die Stufen in Halbtoenen
/// ueber dem Grundton (Lehrbuchwissen; die None als 14, die Undezime als 17, die
/// Tredezime als 21).
struct AkkordArt
{
    QString name;
    std::vector<int> stufen;
};
/// Die Datei der Arten unter `asset/loop/chords` (S-T13 a): das Programm und
/// `make_chords.py` lesen beide aus ihr.
inline constexpr const char* kAkkordArtenDatei = "types.json";
inline constexpr const char* kAkkordArtenFormat = "lumiviz-chord-types";
/// Die Arten aus einer Datei (Format `lumiviz-chord-types`, Fassung 1: `types` mit
/// `name` und `steps`). Nicht gueltig: ein fremdes Format, eine andere Fassung, ein
/// Name doppelt oder leer, Stufen nicht aufsteigend ab 0 oder weiter als
/// `kVoicingWeiteMax`, keine Art.
bool ladeAkkordArten(const QString& pfad, std::vector<AkkordArt>& arten);
/// Die Arten, die das Programm erkennt, aus `types.json` der mitgelieferten Akkorde —
/// alle Arten der Vorlagen bis auf `alt` und `7sg` (in chords-db ohne gaengige
/// Bedeutung). Die Reihenfolge ist der Vorrang beim Erkennen: die einfachen zuerst (von
/// Claude gesetzt). Einmal gelesen; leer, wenn die Datei fehlt — dann erkennt
/// `erkenneAkkord` nichts.
[[nodiscard]] const std::vector<AkkordArt>& akkordArten();

/// Ein erkannter Name: Grundton, Art und — bei einem Slash-Akkord — der Ton im Bass.
struct AkkordName
{
    int grundton = 0;
    QString art;    ///< `major`, `minor`, `7`, … (ohne den Bass)
    int bass = -1;  ///< −1 = der Grundton liegt unten; sonst 0 … 11 (Slash-Akkord)

    [[nodiscard]] bool operator==(const AkkordName&) const = default;
};
/// Wie klingen diese Toene? (S-T11 a L; Regel von Claude gesetzt, wie Spieler zaehlen):
/// die Tonklassen muessen genau eine Art auf einem ihrer Toene bilden — oder sie ohne
/// Quinte, wenn die Art vier Toene oder mehr hat. Liegt der Grundton nicht unten, gilt
/// nur Dur oder Moll mit dem Bass als Slash (wie in den Vorlagen: `C/E`, `Am/G`).
/// Vorrang: vollstaendig mit dem Grundton im Bass, dann der Slash (C-E-A ist Am/C,
/// nicht C6 ohne Quinte), dann ohne Quinte; je Stufe die Reihenfolge von
/// `akkordArten`. Nichts bei weniger als drei Tonklassen oder ohne Treffer.
[[nodiscard]] std::optional<AkkordName> erkenneAkkord(const std::vector<int>& hoehen);
/// Die Art, wie sie in der Datei steht: `major`, `7`, beim Slash `/E` oder `m/E`.
[[nodiscard]] QString akkordArtText(const AkkordName& name);
/// Das Akkordsymbol als Titel: `C`, `Cm`, `C7`, `C/E`, `Am/G`.
[[nodiscard]] QString akkordSymbol(const AkkordName& name);

}  // namespace lumi::loop
