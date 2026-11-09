/**
 * @file Orgel.hpp
 * @brief Klasse 11 Tonrad: die Tonradorgel als Ganzes ueber einen Clip (F-1 a) — 91 Raeder mit gemeinsamer Phase aus der
 *        Zeit, Tasten × Zugriegel × Raeder mit Rueckfaltung, Uebersprechen, Tastenklick aus neun Kontakten, Perkussion
 *        mit Einzeltrigger, Scanner-Vibrato/Chorus, Schweller, Rotary speaker mit Hoch- und Auslauf
 *        (`klassen/Klasse 11 - Tonrad/modell-entwurf.md`; Entscheide Patrik 2026-10-29: F-1 … F-7 a ueber FRAGEN_KONSOLIDIERT
 *        T2, T3a, T3b, T7, T8). In LumiViz „Tonewheel Organ“ und „Rotary speaker“ (F-7 a).
 *
 * **Raeder** (Bericht `recherche/2026-10-26_tonradgenerator.md` §1, nur Bericht): F = 20 U/s × Zaehne × Uebersetzung,
 * Uebersetzungen C 85/104 … B 108/70, Zaehne 2 · 4 · 8 · 16 · 32 · 64 · 128 je zwoelf Raeder, die obersten sieben mit
 * 192 Zaehnen auf den Getrieben F … B; die tiefsten zwoelf „roughly square-wave“ (Grundton, 3., 5. Harmonische mit 1, ⅓,
 * ⅕), die uebrigen sinusfoermig (Filter ab Rad 44 bzw. 49); Pegel je Rad nach Tabelle V (Spannungen, nur Bericht).
 * **Zugriegel** 16′ … 1′ = −12, +7, 0, +12, +19, +24, +28, +31, +36 Halbtoene, Stufe 8 = 0 dB, −3 dB je Stufe; Rueckfaltung:
 * unter Rad 13 eine Oktave hoch, ueber Rad 91 eine Oktave tiefer (so oft noetig). **Uebersprechen** im Radfach (vier
 * Oktaven, −40 dB wie setBfree — offen). **Tastenklick:** neun Kontakte schliessen versetzt (bis 1,5 mm von 11 mm Weg,
 * Moro et al. 2017 — nur Bericht) und prellen (< 5 ms); der Kontakt schaltet die Welle des Busbars hart zu.
 */
#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace ic::tonrad
{

/// Die Frequenz von Rad 1 … 91 (Netz 60 Hz).
[[nodiscard]] double radHz(int rad);
/// Das Rad, das der Zugriegel `riegel` (0 = 16′ … 8 = 1′) fuer die Taste `taste` (1 = C2 … 61 = C7) nimmt (mit Rueckfaltung).
[[nodiscard]] int radFuer(int taste, int riegel);

using Registrierung = std::array<int, 9>;  ///< 16′ 5⅓′ 8′ 4′ 2⅔′ 2′ 1⅗′ 1⅓′ 1′, je 0 … 8

enum class Scanner
{
    Aus,
    V1, V2, V3,
    C1, C2, C3,
};

struct OrgelSpiel
{
    std::string name;
    Registrierung register_{8, 8, 8, 0, 0, 0, 0, 0, 0};
    bool perkussion = false;
    bool perkussionDritte = true;   ///< 2⅔′ (sonst 4′)
    bool perkussionSchnell = true;
    bool perkussionWeich = false;
    Scanner scanner = Scanner::Aus;
    bool rotary = true;
    double uebersprechen = 0.01;    ///< −40 dB (setBfree — offen)
    double klickMs = 4.0;           ///< Prellen der Kontakte (Moro: frueh < 5 ms — nur Bericht)
    double saettigung = 1.2;        ///< Vorverstaerker y = tanh(k·x)/k (offen; 0 = linear)
    // Rotary speaker (Bericht `recherche/2026-10-26_leslie.md`): Drehzahlen Nutzermessung Fisher (nur Bericht)
    double hornLangsamUpm = 48.0, hornSchnellUpm = 400.0;
    double trommelLangsamUpm = 40.0, trommelSchnellUpm = 342.0;
    double hornHochS = 0.8, hornAusS = 1.0;        ///< Zeitkonstanten (offen; Herrera: getrennt — Werte nicht gefunden)
    double trommelHochS = 4.0, trommelAusS = 5.0;  ///< (offen; Forum: Trommel etwa 5 s — nur Bericht)
    double hornRadiusM = 0.18;      ///< (offen; CLM-Code 0,18 m, Herkunft unbekannt)
    double ausgangBezug = 1.0;
    std::string kennzeichen = "ungemessen";
};

/// Eine gespielte Note (Obermanual); Taste 1 = C2 … 61 = C7 (MIDI 36 … 96).
struct OrgelNote
{
    double abS = 0.0;
    double bisS = 1.0;
    int midi = 60;
    double tempo = 0.7;  ///< m/s der Taste (Moro: angeschlagen 0,5–0,9) — der Versatz der Kontakte ∝ 1/tempo
};

/// Die Spur ueber die Passage (F-2, F-4 a): Rotary schnell/langsam, Registrierung, Schweller.
struct OrgelSpur
{
    std::vector<std::pair<double, bool>> rotarySchnell;  ///< (ab s, schnell)
    std::vector<std::pair<double, Registrierung>> register_;  ///< Wechsel, Uebergang in 50 ms
    double schweller = 0.7;  ///< Expression (F-3 a): 0 … 1
    bool rotaryStartSchnell = false;
};

struct OrgelMessung
{
    std::vector<double> hornUpm;   ///< je 10 ms
    double perkussionStarts = 0;   ///< so oft startete die Perkussion
};

void rechneOrgel(const OrgelSpiel& s, const std::vector<OrgelNote>& noten, const OrgelSpur& spur, std::vector<float>& mono,
                 std::int64_t frames, int rate = 48000, OrgelMessung* messung = nullptr);

/// `organ` (F-5 a): Standard-Registrierung, Scanner aus, Rotary langsam.
[[nodiscard]] OrgelSpiel baueVorgabeOrgel();
/// `house_organ` (F-5 a): kurze Registrierung mit Perkussion.
[[nodiscard]] OrgelSpiel baueVorgabeHausorgel();

}  // namespace ic::tonrad
