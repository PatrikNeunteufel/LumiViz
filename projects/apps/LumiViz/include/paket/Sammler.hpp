/**
 * @file Sammler.hpp
 * @brief Publish, Stufe P-A: was ein Ding nennt — ueber alle Stufen gesammelt
 *        (docs/core-services/Publish_Konzept.md §2a, §3; Entscheide P1 a, P3 a,
 *        Patrik S125)
 *
 * Ein Loop nennt Instrumente, Klaenge und ein Kit; ein Kit Klaenge und
 * Instrumente; ein Instrument Samples und Formen. `sammle` geht von einer Datei
 * aus alle Nennungen ab und teilt sie in drei Haufen:
 *
 *  - **Zugehoerige** — eigene (`user:<name>`) und Dateien an freiem Ort; sie
 *    gehen ins Paket und heissen beim Empfaenger `user:<name>` (P3 a).
 *  - **Mitgeliefertes** — `piano`, `kit:kick`, `lib:clap_1`, `heart`, `808`;
 *    nur der Name geht mit.
 *  - **Fehlendes** — eine Nennung, deren Datei es nicht gibt.
 *
 * Kein Qt-Widget, schreibt nichts. Wer Mitgeliefertes pruefen will (Instrumente,
 * Formen, Kits), braucht eine `QCoreApplication` — wie ueberall im Loop.
 */
#pragma once

#include <QString>
#include <QStringList>

#include <optional>
#include <vector>

namespace lumi::paket
{

/// Die Arten der Loop-Familie (Stufe P-A); Komposition und Effect chain folgen in P-B.
enum class Art : int
{
    Instrument = 0,
    Kit,
    Loop,
    Kontur,
    Sample,
};

/// Wie die Art im Manifest und im Namen des Pakets steht: `lvinst`, `lvkit`,
/// `lvloop`, `lvcontour`, `sample`.
[[nodiscard]] QString artText(Art art);
/// Die Art einer Datei nach ihrer Endung; ein Sample an jeder Endung von
/// `loop::sampleEndungen`. Leer = keine Art, die sich veroeffentlichen laesst.
[[nodiscard]] std::optional<Art> artVonDatei(const QString& datei);

enum class Herkunft : int
{
    Eigen,      ///< im Benutzerordner der Art (`user:<name>`)
    FreierOrt,  ///< irgendwo sonst — neben dem Loop, auf der Platte
};

/// Eine Datei, die mit ins Paket geht.
struct Zugehoerige
{
    Art art = Art::Sample;
    QString datei;  ///< absolut, wie sie beim Absender liegt
    /// Der Name beim Empfaenger: `user:<name>`. Eigene behalten ihren Namen,
    /// eine Datei an freiem Ort bekommt ihn aus dem Dateinamen
    /// (`loop::eigenerName`); gleiche Namen einer Art werden `<name>_2`.
    QString name;
    Herkunft herkunft = Herkunft::Eigen;
    /// Die Nennungen, die auf sie zeigten, so wie das Modell sie traegt (ein
    /// Pfad absolut) — die schreibt das Packen um.
    QStringList nennungen;
    /// Wer sie nennt: die Titel, nach der Reihenfolge des Fundes, je einmal.
    QStringList genanntVon;
};

struct Fehlend
{
    Art art = Art::Sample;
    QString nennung;
    QString genanntVon;

    [[nodiscard]] bool operator==(const Fehlend&) const = default;
};

struct Fund
{
    bool ok = false;
    QString fehler;  ///< nur wenn `ok` false ist: die Hauptdatei fehlt oder ist nicht lesbar
    Art art = Art::Loop;
    QString hauptdatei;  ///< absolut
    QString titel;       ///< wie das Programm das Ding nennt
    /// Fuer Instrument, Kit, Kontur und Sample: der Name beim Empfaenger
    /// (`user:<name>`). Ein Loop hat keinen — er kommt dorthin, wo der
    /// Empfaenger ihn ablegt.
    QString hauptName;
    /// Nach Art (in der Reihenfolge von `Art`), darin nach Name.
    std::vector<Zugehoerige> zugehoerige;
    /// Die Nennungen des Mitgelieferten, sortiert, je einmal.
    QStringList mitgeliefert;
    std::vector<Fehlend> fehlt;

    /// Die Zugehoerige einer Art mit diesem Namen; nullptr = keine.
    [[nodiscard]] const Zugehoerige* finde(Art art, const QString& name) const;
};

/// Sammelt, was `datei` nennt, ueber alle Stufen. Jede Datei kommt einmal vor,
/// auch wenn sie mehrfach genannt wird.
[[nodiscard]] Fund sammle(const QString& datei);

/// Die Datei, auf die eine Nennung einer Art zeigt — aufgeloest wie im Sammler,
/// ein relativer Pfad ab `basis`. Leer = Mitgeliefertes, nichts oder fehlt.
[[nodiscard]] QString dateiDerNennung(Art art, const QString& nennung, const QString& basis);
/// Ein Schluessel, der eine Datei einmal zaehlt, auch unter zwei Schreibweisen.
[[nodiscard]] QString dateiSchluessel(const QString& datei);

}  // namespace lumi::paket
