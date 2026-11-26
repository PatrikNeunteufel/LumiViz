/**
 * @file Saiten.hpp
 * @brief InstrumentModels, Kern `pluckedString` Rechenfassung 2: die Saiten der fuenf Instrumente (Instrumente_IC_Konzept.md
 *        GZ1-1 a, GZ2 a, KQ2 a, KQ4 a, KQ5 a, KQ22 a; Herleitung `docs/modellierung/gezupfte-saite/2026-11-18_S205_herleitung.md`
 *        §2; Teilplan 34k-7d-3)
 *
 * Je Saite die Werte der Wellenzahl (`umlauf::Saitenwerte`: Zug, Masse je Laenge, Biegesteife, Verluste; monofil mit
 * Durchmesser und Dichte, sonst nach Valette) — dieselben Zahlen wie der Pruefstand S199 und die Bezugsdatei
 * `werte/zupf_pruefstand.txt` (`proben/2026-11-26_S222_bezug_zupf.py`):
 *
 * - **Harfe** (je Taste, KQ22 a): 46 Saiten nach Bell 1988 Tab. 2.1 (Nylon 00–8, Darm 9–33) und Tab. 2.2 (Metall 34–44:
 *   Stahlkern, Kupfer als voller Ring — Annahme; Biegung vom Kern, Verluste von den umsponnenen Gitarrensaiten
 *   uebertragen); E aus der Spannung (Woodhouse & Lynch-Aird 2019 Tab. I, Gl. 16, 17). Die weisse Taste ist ihre Saite
 *   um einen Halbton verkuerzt (Pedal Mitte), die schwarze die Saite darunter um zwei, die tiefste Taste die tiefste
 *   Saite leer.
 * - **Cembalo** (je Taste, KQ2 a, GZ12 a): ein 8′, Mensur des Taskin 1769 an elf Tasten, dazwischen logarithmisch; Draht
 *   nach Di Veroli je Bereich (Dichte und E Annahme); Verlust: Luft nach Stokes, Biegung η_E = 0,001 (Annahme).
 * - **Gitarre** (je Saite, GZ1-1 a): Woodhouse u. a. 2012 Tab. I, Satz (a) Nylon, Mensur 650 mm, 19 Buende.
 * - **Laute** (je Chor, KQ4 a): 8 Choere in G nach Woodhouse & Lynch-Aird 2019 Tab. II, Mensur 625 mm, 12 Buende; Choere
 *   1–3 mit Oktavsaite, im Einklang um ±2 Cent gegeneinander verstimmt (die Laenge).
 * - **Oud** (je Chor, KQ5 a): D'Addario EJ95A (Hersteller), Mensur 600 mm (Annahme), 12 Buende; umsponnene Saiten mit
 *   Biegesteife und Verlusten der Gitarrenbaesse (uebertragen); im Einklang um ±1,5 Cent verstimmt.
 *
 * Die Tafel des Kerns (34k-7d-4) traegt sie als Kurven je Taste (Harfe, Cembalo) bzw. als Saitensatz (die anderen).
 *
 * Reines C++ ohne Qt.
 */
#pragma once

#include "InstrumentModels/gemeinsam/Umlauf.hpp"

#include <optional>
#include <vector>

namespace lumi::kern::zupf
{

using umlauf::Saitenwerte;

/// Eine Saite einer Taste (Harfe, Cembalo): ihre Werte, ihre schwingende Laenge, die Taste ihres Stegpunkts im Koerper.
struct Tastensaite
{
    Saitenwerte werte;
    double laenge = 0.0;  ///< m
    int stegTaste = -1;
};

/// Die Saite der Taste an der Harfe (KQ22 a), oder `std::nullopt` ausserhalb des Umfangs.
[[nodiscard]] std::optional<Tastensaite> harfenSaite(int taste);
/// Die Saite der Taste am Cembalo (Tasten 29–89).
[[nodiscard]] Tastensaite cembaloSaite(int taste);

/// Eine Saite eines Chors.
struct Chorsaite
{
    Saitenwerte werte;
    int oktave = 0;              ///< Halbtoene ueber dem Chor (Oktavsaite 12)
    double laengenFaktor = 1.0;  ///< im Einklang verstimmt: die leere Laenge ist mensur · laengenFaktor
};

/// Ein Chor (eine Saite des Satzes, wie die App sie zaehlt).
struct Saitenchor
{
    int leer = 0;  ///< MIDI der leeren Saite
    std::vector<Chorsaite> saiten;
};

/// Ein Saitensatz (Gitarre, Laute, Oud): Mensur, Zahl der Buende, die Choere von der tiefsten an.
struct Saitensatz
{
    double mensur = 0.0;
    int buende = 0;
    std::vector<Saitenchor> choere;
};

[[nodiscard]] Saitensatz saitensatzGitarre();
[[nodiscard]] Saitensatz saitensatzLaute();
[[nodiscard]] Saitensatz saitensatzOud();

/// Eine monofile Saite aus Durchmesser (mm), Ton (Hz), Laenge (m) und Werkstoff ('N' Nylon, 'F' Fluorkarbon, 'D' Darm;
/// Woodhouse & Lynch-Aird 2019 Tab. I, Gl. 16, 17: E aus der Spannung).
[[nodiscard]] Saitenwerte monofil(double durchmesserMm, double hz, double laenge, char werkstoff);
/// Eine umsponnene Saite aus Zug (N), Ton und Laenge; Biegesteife und Verluste uebertragen (Woodhouse u. a. 2012 Tab. I,
/// Saiten 4–6).
[[nodiscard]] Saitenwerte umsponnen(double zug, double hz, double laenge);

}  // namespace lumi::kern::zupf
