/**
 * @file Bogen.hpp
 * @brief Klasse 3 Bogen-Saite: ein Ton eines Streichinstruments — Saite als Waveguide mit Reibstelle am Bogenpunkt,
 *        Korpus als Moden der Admittanz am Steg mit Rueckwirkung auf die Saite (Wolfton), Bogen
 *        geschwindigkeitsgetrieben (`klassen/Klasse 3 - Bogen-Saite/modell-entwurf.md`; Entscheide Patrik 2026-10-26:
 *        F-1 a, **F-2 b** (Waveguide mit Reibstelle, Korpus modal am Steg), F-3 … F-8 a)
 *
 * **Saite** (Woodhouse 2014 S. 7 Gl. 2, nur Bericht): am Bogenpunkt v = v_h + f/(2Z0), v_h die Summe der ankommenden
 * Schnellewellen von Sattel und Steg; zwei Strecken je Richtung (Bogen–Sattel, Bogen–Steg) als Verzoegerungen mit
 * Lagrange-Interpolation 3. Ordnung; am Sattel (bzw. Finger) fest (−1) mit einem Verlustfilter erster Ordnung je
 * Umlauf, gesetzt aus der Guete der Saite. Steifigkeit (Dispersion) nicht gebaut (Inharmonizitaet am Cello D3
 * ≈ 10⁻⁴ — offen.md).
 *
 * **Reibung** (Galluzzo, Woodhouse & Mansour 2017 Gl. 1, nachgelesen S. 5): μ(Δv) = 0,4·e^(−|Δv|/0,01) +
 * 0,45·e^(−|Δv|/0,1) + 0,35; Haften bis μ_s = 1,2. Der Schnitt der Geraden mit der Kurve nach Friedlander mit Hysterese:
 * es haftet, solange es haftete und die Haftkraft reicht; gleitet es, bleibt es auf dem aeussersten Schnittpunkt.
 *
 * **Steg:** die Moden der Admittanz Y des Korpus im `RohrKern` (Eingang Kraft, Ausgang Schnelle), geschlossen geloest:
 * u = (u_vorher + 2·D·Z0·v⁺)/(1 + D·Z0), F = Z0·(2v⁺ − u), zurueck v⁻ = u − v⁺. **Ausgang:** du/dt am Steg (Abstrahlung
 * des Korpus genaehert, offen).
 *
 * **Bogen:** Geschwindigkeit v_B, Kraft F_B, Abstand zum Steg (absolut, wie gespielt); Expression als Weg (F-3 a):
 * v_B geometrisch zwischen `vbPp` und `vbFf`, F_B in der Mitte des **Fensters** des Modells (Schelleng-Fenster je
 * Note bei v_B pp und ff am Modell ermittelt, dazwischen geometrisch — „The minimum bow force was found to be independent
 * of bow velocity“, Schoonderwaldt et al. 2008 S. 604, nachgelesen: darum nicht aus der Formel skaliert). Bogenwechsel nach der Haarlaenge (F-4 a). Gerechnet doppelt
 * abgetastet.
 *
 * Reines C++, deterministisch.
 */
#pragma once

#include "gemeinsam/Kurve.hpp"
#include "gemeinsam/Rohr.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace ic::bogen
{

/// Eine Saite: leer gestimmt auf `leerNote`, Wellenwiderstand Z0, Guete der Saite.
struct StreichSaite
{
    int leerNote = 50;
    double z0 = 0.55;     ///< kg/s
    double guete = 500.0; ///< Q der Transversalwelle (Verluste je Umlauf)
    std::string herkunft;
};

enum class Art
{
    Arco,
    Pizzicato,
    Spiccato,   ///< Springbogen: die Kraft in Spruengen (Rate `springHz`)
    Tremolo,    ///< schnelle Bogenwechsel (Rate `springHz`)
    ColLegno,   ///< mit dem Holz: ein kurzer Stoss am Bogenpunkt, keine Reibung
};

enum class Lage
{
    Normal,
    Ponticello,  ///< naeher am Steg (Abstand ×0,5)
    Tasto,       ///< naeher am Griffbrett (Abstand ×2)
};

struct StreichSpiel
{
    std::string name;
    int noteMin = 36;
    int noteMax = 76;
    bool spielbar = true;
    std::string grund;
    std::vector<StreichSaite> saiten;  ///< tief → hoch
    double mensurM = 0.69;
    double bogenAbstandM = 0.055;      ///< vom Steg
    double haarLaengeM = 0.65;         ///< Bogenwechsel (F-4 a)
    double wechselS = 0.04;            ///< so lange geht v_B durch null (offen)
    std::vector<ImpedanzMode> korpus;  ///< Admittanz am Steg: hz, q, spitze in s/kg
    double daempferFaktor = 0.75;      ///< con sordino: Moden ueber 600 Hz ×Faktor in der Frequenz (offen)
    double daempferStaerke = 0.5;      ///< und ×Faktor in der Staerke (offen; Elie: 1–2,5 kHz 5–13 dB weniger)
    double vbPp = 0.08, vbFf = 0.4;    ///< m/s (offen; Cello Legato 0,166 m/s im Mittel, Chudy — nachgelesen)
    double reibungMuS = 1.2;           ///< Haften (Galluzzo Gl. 1 bei Δv = 0)
    // --- je Note gestimmt (`stimme`, Rechnung) ---
    Kurve kraftUntenJeNote;            ///< N bei v_B pp (log)
    Kurve kraftObenJeNote;             ///< N bei v_B pp (log)
    Kurve kraftUntenFfJeNote;          ///< N bei v_B ff (log)
    Kurve kraftObenFfJeNote;           ///< N bei v_B ff (log)
    Kurve laengeJeNote;                ///< Faktor auf die gegriffene Laenge (log), Feinstimmung
    double ausgangBezug = 1.0;
    std::string kennzeichen = "ungemessen";
};

struct KettenNote
{
    double abS = 0.0;
    int note = 50;
    double hz = 146.83;
};

struct StreichAnschlag
{
    std::vector<KettenNote> kette;
    double expression = 0.5;
    double haltenS = 1.0;
    Art art = Art::Arco;
    Lage lage = Lage::Normal;
    bool daempfer = false;     ///< con sordino (Passage)
    double vibratoCent = 0.0;  ///< Tiefe ± (F-5 a)
    double vibratoHz = 5.5;    ///< Vorgabe nach SK10 a / Geringer & Allen 2004 (nachgelesen)
    double springHz = 12.0;    ///< Spiccato/Tremolo („between 6 and 35 Hz“, Guettler 2002 — nachgelesen)
    double pitchCent = 0.0;    ///< Glissando: Verschiebung der Tonhoehe ueber die Laenge
    double kraftN = 0.0;       ///< > 0: die Bogenkraft unmittelbar in N (v_B nach der Staerke, fuer `stimme`); 0 = der Weg
    int rate = 48000;
};

struct StreichMessung
{
    double spielHz = 0.0;
    bool schwingt = false;
    double einschwingS = 0.0;
    double haftAnteil = 0.0;   ///< Anteil der Takte mit Haften am Bogenpunkt (Helmholtz: 1 − β)
};

void rechneStreichTon(const StreichSpiel& s, const StreichAnschlag& a, std::vector<float>& mono, std::int64_t frames,
                      StreichMessung* messung = nullptr);
/// Das Fenster der Kraft und die Feinstimmung je Note durch das Modell hindurch.
void stimme(StreichSpiel& s);

[[nodiscard]] StreichSpiel baueVorgabeCello(bool gestimmt = true);
[[nodiscard]] StreichSpiel baueVorgabeVioline(bool gestimmt = true);
[[nodiscard]] StreichSpiel baueVorgabeViola(bool gestimmt = true);
[[nodiscard]] StreichSpiel baueVorgabeKontrabass(bool gestimmt = true);

/// Das Ensemble (F-8 a): `stimmen` gerechnete Stimmen je mit gestreuter Stimmung, Einsatz, Vibrato und Korpus.
void rechneEnsembleTon(const StreichSpiel& s, const StreichAnschlag& a, int stimmen, std::vector<float>& mono, std::int64_t frames);

}  // namespace ic::bogen
