/**
 * @file Mitgeliefert.hpp
 * @brief Wo ein mitgelieferter Bereich liegt: neben der Exe oder im
 *        Projektordner (docs/core-services/Datenablage_Konzept.md, Stufe A1;
 *        Entscheide Patrik S114: D9 a mit Schalter)
 *
 * Mitgeliefertes — Voreinstellungen, Vorlagen, das Kit, die Instrumente, die
 * Format-Icons — liegt im Repo unter `asset/<Bereich>` und nach dem Bau als
 * Kopie unter `<Exe>/asset/<Bereich>` (`asset/Asset.cmake`). Bis S114 suchte
 * jeder Bereich seinen Ordner selbst, nur aufwaerts ab dem Programmordner;
 * neben einer weitergegebenen Exe fand er nichts.
 *
 * Die Reihenfolge:
 *  - **neben der Exe zuerst** (Vorgabe): `<Exe>/asset/<Bereich>`, wenn der
 *    Ordner da ist — so liest ein Start aus dem Bau-Ordner, was auch eine
 *    weitergegebene Exe liest. Sonst aufwaerts.
 *  - **Projekt zuerst** (Schalter in den Settings, `kSchluesselProjektZuerst`):
 *    aufwaerts, sonst neben der Exe. Fuer die Arbeit an den Dateien unter
 *    `asset/`, ohne Bau dazwischen.
 *
 * Aufwaerts heisst wie bis S114: ab dem Programmordner, er und bis zu elf
 * Ordner darueber (`kEbenenAufwaerts`), ein Ordner `asset/<Bereich>`, in dem
 * die Ankerdatei liegt. Der Anker (meist die Pflegenotiz `README.md`) wird
 * nicht mitkopiert — er unterscheidet das Repo von einer Kopie; die Kopie
 * neben der Exe zaehlt aufwaerts nie, auch wenn ihr Anker mitkam (die Icons).
 * Ohne `QCoreApplication` (ein Test ohne App) beginnt die Suche im
 * Arbeitsordner.
 */
#pragma once

#include <QString>

namespace lumi::core::mitgeliefert
{

/// Einstellung (bool): true = Projekt zuerst. Gilt nach einem Neustart — die
/// Bereiche loesen ihren Ordner einmal auf und merken ihn.
inline constexpr const char* kSchluesselProjektZuerst = "data/bundledFromProjectFirst";

/// So viele Ordner sieht die Suche aufwaerts an, den Programmordner
/// mitgezaehlt (wie bis S114).
inline constexpr int kEbenenAufwaerts = 12;

enum class Herkunft
{
    Keine,     ///< nirgends gefunden
    NebenExe,  ///< `<Exe>/asset/<Bereich>`
    Projekt,   ///< aufwaerts gefunden (Entwicklungsbaum)
};

struct Fund
{
    QString ordner;  ///< leer = nicht gefunden
    Herkunft herkunft = Herkunft::Keine;
};

/// Die Suche selbst, ohne Einstellungen und ohne Log. `bereich` ist der Pfad
/// unter `asset` (`loop`, `karaoke/trails`), `anker` eine Datei, die im Repo
/// in diesem Ordner liegt.
[[nodiscard]] Fund suche(const QString& exeOrdner, const QString& bereich, const QString& anker, bool projektZuerst);

/// Der Ordner eines Bereichs fuer dieses Programm, nach der eingestellten
/// Reihenfolge. Leer = nicht gefunden. Schreibt je Bereich einmal ins Log,
/// woher gelesen wird.
[[nodiscard]] QString ordner(const QString& bereich, const QString& anker = QStringLiteral("README.md"));

/// Die eingestellte Reihenfolge.
[[nodiscard]] bool projektZuerst();
void setzeProjektZuerst(bool an);

/// Liegt ueber dem Programmordner ein Projekt (ein Ordner `asset` mit den
/// Format-Icons)? Nur dann hat der Schalter eine Wirkung.
[[nodiscard]] bool projektVorhanden();

}  // namespace lumi::core::mitgeliefert
