/**
 * @file HammerSaite.hpp
 * @brief Loop: die Hammer-Saite — Kurven ueber die Tastatur, die Groessen einer
 *        Schicht *Hammer* + *Modal*, der Block *Keyboard* und die Stufen der
 *        Anschlagstaerke (docs/audio/Spielweisen_Konzept.md §4.1, Stufe 31a;
 *        Entscheide Patrik S145: HS1 a, HA1 a bis HA4 a; S146: HK1 a, HK2 a)
 *
 * Eine Groesse des Fluegels — Inharmonizitaet, Abklingzeit, Hammer, Daempfer —
 * ist eine Kurve aus Stuetzpunkten an Tasten (HA1 a). Dazwischen wird in der
 * Skala der Groesse monoton kubisch interpoliert (Fritsch–Carlson), vor dem
 * ersten und nach dem letzten Punkt gilt der Randwert; eine Taste kann eine
 * Abweichung tragen, die den Wert der Kurve dort ersetzt (HK1 a).
 *
 * Die Vorgaben stammen, wo belegt, aus der Literatur (Befunde S146 im Konzept):
 * Saiten und Hammer aus Chabassier & Durufle, INRIA RT-0425 (2012), Anhang A
 * und Gl. 2–4 (HT1 a, HT2 a); B aus Rigaud, David & Daudet 2011, Gl. 4/5. Was
 * nicht belegt ist, steht hier als **vorlaeufig** — die Messung an *Salamander*
 * (Stufe 31d) setzt es.
 *
 * Hier steht nur das Modell; gerechnet wird ein Ton erst mit Stufe 31b.
 * Kein Qt-Widget, keine Audio-Engine.
 */
#pragma once

#include <QJsonObject>
#include <QJsonValue>
#include <QLatin1String>
#include <QString>

#include <optional>
#include <vector>

namespace lumi::loop
{

// --- Kurven ueber die Tastatur (HA1 a, HK1 a) --------------------------------

/// Die Tasten eines Fluegels als MIDI-Nummern: A0 bis C8 (wie `kHoeheMin`,
/// `kHoeheMax` in Loop.hpp). Taste n der Literatur (1 … 88) ist MIDI n + 20.
inline constexpr int kTasteMin = 21;
inline constexpr int kTasteMax = 108;

/// In welcher Skala eine Groesse zwischen den Stuetzpunkten verlaeuft (HK1 a).
enum class KurvenSkala : int
{
    Linear = 0,
    /// Logarithmisch: in ln(Wert) interpoliert — nur fuer Groessen ueber null.
    Logarithmisch = 1,
};

/// Ein Wert an einer Taste.
struct TastenWert
{
    int taste = kTasteMin;  ///< MIDI-Nummer, `kTasteMin` … `kTasteMax`
    double wert = 0.0;

    [[nodiscard]] bool operator==(const TastenWert&) const = default;
};

/// Eine Groesse ueber die Tastatur: Stuetzpunkte, dazu Abweichungen je Taste.
/// Beide nach Taste sortiert, je Taste hoechstens ein Eintrag (`richteKurve`).
struct Tastenkurve
{
    std::vector<TastenWert> punkte;
    /// Eine Taste mit eigenem Wert; leer = ueberall die Kurve.
    std::vector<TastenWert> abweichungen;

    [[nodiscard]] bool operator==(const Tastenkurve&) const = default;
};

/// Der Wert an einer Taste: die Abweichung, wenn die Taste eine traegt, sonst
/// die Kurve (HK1 a). Ohne Stuetzpunkte 0. Eine Taste ausserhalb gilt als die
/// naechste am Rand.
[[nodiscard]] double wertAn(const Tastenkurve& kurve, int taste, KurvenSkala skala);

/// Eine Kurve in ihre Grenzen: Tasten auf `kTasteMin` … `kTasteMax`, nach Taste
/// sortiert, je Taste der erste Eintrag, Werte auf `von` … `bis` (nicht endliche
/// fallen weg). Fehlen alle Stuetzpunkte, gilt `ersatz`.
void richteKurve(Tastenkurve& kurve, double von, double bis, const Tastenkurve& ersatz);

/// Wie eine Kurve in einer Datei steht: `{"points": [[taste, wert], …],
/// "overrides": [[taste, wert], …]}` — beide immer.
[[nodiscard]] QJsonObject kurveAlsJson(const Tastenkurve& kurve);
/// Liest eine Kurve (ungerichtet — danach `richteKurve`). Was keine Zahlenpaare
/// sind, faellt weg.
[[nodiscard]] Tastenkurve kurveAusJson(const QJsonValue& wert);

// --- die Groessen einer Schicht *Hammer* + *Modal* (HS1 a, HA1 a, HA2 a) -----

/// Der Resonanzboden. Woher seine Antwort kommt, entscheidet Stufe 31c; bis
/// dahin traegt er nur den Anteil (0 = keiner).
struct Resonanzboden
{
    double anteil = 0.0;  ///< 0 … 1: so viel des Tons kommt durch den Boden (vorlaeufig)

    [[nodiscard]] bool operator==(const Resonanzboden&) const = default;
};

/// Grenzen der Groessen (fuer `richteHammerSaite` und spaeter den Creator).
inline constexpr int kChorMin = 1;
inline constexpr int kChorMax = 3;

/// Die Hammer-Saite einer Schicht: je Groesse eine Kurve ueber die Tastatur.
/// Die Skala jeder Groesse steht bei ihr (HK1 a).
struct HammerSaite
{
    /// Inharmonicity B (log): f_k = k·f0·√(1 + B·k²). 1e-6 … 0,1.
    Tastenkurve inharmonizitaet;
    /// Decay (log): so lange, bis der Grundton in der ersten Stufe um 60 dB fiele, in s.
    /// 0,05 … 120. Bei einer Saite im Chor die ganze Abklingzeit.
    Tastenkurve abklingS;
    /// Late decay (log, HT3 a): dasselbe fuer die zweite Stufe (die Gegenlagen des
    /// Chors), in s, 0,05 … 120; kuerzer als *Decay* gilt als *Decay*.
    Tastenkurve abklingSpaetS;
    /// High decay (log): die Abklingzeit eines Teiltons bei der zehnfachen
    /// Frequenz des Grundtons im Verhaeltnis zu der des Grundtons, 0,001 … 1.
    /// Die Verluste wachsen dazwischen mit dem Quadrat der Frequenz (31b).
    Tastenkurve abklingHoch;
    /// Strings (linear, gerundet): Saiten im Chor, `kChorMin` … `kChorMax`.
    Tastenkurve chor;
    /// Detune (linear): so weit liegen die aeusseren Saiten des Chors
    /// auseinander, in Cent, 0 … 20.
    Tastenkurve verstimmungCent;
    /// Hammer mass (linear), in g, 1 … 30.
    Tastenkurve hammerMasseG;
    /// Hammer stiffness Q0 (log), in N/mm^p (F = Q0·u^p, Stulov Gl. 4), 1 … 1e6.
    Tastenkurve hammerSteife;
    /// Hammer exponent p (linear), 1 … 6.
    Tastenkurve hammerExponent;
    /// Strike position (linear): wo der Hammer trifft, als Anteil der Saite
    /// von der Agraffe aus, 0,02 … 0,5.
    Tastenkurve anschlagStelle;
    /// Damper (log): so lange, bis ein Ton unter dem Daempfer um 60 dB gefallen
    /// ist, in s, 0,01 … 10 (Bank 2010: der Daempfer verkuerzt die Abklingzeiten).
    Tastenkurve daempferS;
    // --- die Saite selbst (HT1 a): der Hammer treibt die Moden mit 2/(Lµ), die
    // Laengsschwingung braucht dazu ES (Bank 2010, Gl. 4c, 26); T0 und ES folgen
    // aus f0 = (1/2L)√(T0/µ) und f_l0 = (1/2L)√(ES/µ) ---
    /// String length (log): die schwingende Laenge, in m, 0,02 … 8.
    Tastenkurve saitenLaengeM;
    /// String mass (log): die Masse je Laenge µ, in g/m, 0,5 … 500.
    Tastenkurve saitenMasseGm;
    /// Longitudinal ratio (linear): die Grundfrequenz der Laengsschwingung im
    /// Verhaeltnis zum Grundton, 5 … 40.
    Tastenkurve laengsVerhaeltnis;
    Resonanzboden resonanzboden;

    [[nodiscard]] bool operator==(const HammerSaite&) const = default;
};

/// Die Saiten im Chor an einer Taste: die Kurve, auf eine ganze Zahl gerundet (HK1 a).
[[nodiscard]] int chorAn(const HammerSaite& saite, int taste);

/// Die Vorgaben einer neuen Hammer-Saite (Befunde S146): Hammer nach RT-0425
/// Gl. 2–4 (Stuetzpunkte alle sechs Tasten) und B nach Rigaud Gl. 5 (Diskant
/// belegt, Bass abgelesen; Stuetzpunkte an A0, A1, … A7 und C8); Laenge, Masse, Laengsverhaeltnis und
/// Anschlagstelle aus RT-0425 Anhang A (C1 … B7, ausgewaehlte Tasten, auf 2 %
/// genau); der Rest vorlaeufig, mit wenigen Punkten.
[[nodiscard]] const HammerSaite& hammerSaiteVorgabe();

/// Jede Kurve in ihre Grenzen; eine leere bekommt die Vorgabe.
void richteHammerSaite(HammerSaite& saite);

/// Die Hammer-Saite als JSON (Block `hammerString` einer Schicht) und zurueck
/// (ungerichtet — danach `richteHammerSaite`). Fehlt eine Kurve, bleibt sie leer.
[[nodiscard]] QJsonObject hammerSaiteAlsJson(const HammerSaite& saite);
[[nodiscard]] HammerSaite hammerSaiteAusJson(const QJsonObject& o);

/// Chabassier & Durufle, RT-0425, Gl. 2–4 — die Groessen des Hammers an Taste i
/// (1 … 88, i = MIDI − 20): Masse in g, Exponent p, Steife K_H in N·m⁻ᵖ und
/// umgerechnet Q0 = K_H·10⁻³ᵖ in N/mm^p (HT2 a).
[[nodiscard]] double rtHammerMasseG(int i);
[[nodiscard]] double rtHammerExponent(int i);
[[nodiscard]] double rtHammerSteifeNm(int i);
[[nodiscard]] double rtHammerSteife(int i);
/// Rigaud, David & Daudet 2011, Gl. 4/5: B an MIDI-Nummer m, mit dem
/// Diskantsteg der Arbeit (s_T = 9,26·10⁻², y_T = −13,64) und dem Bassteg
/// `sB`, `yB`.
[[nodiscard]] double rigaudB(int m, double sB, double yB);
/// Der Bassteg der Vorgabe: abgelesen aus Abb. 1 der Arbeit (ein Beispiel ohne
/// genannten Fluegel; S146, vorlaeufig bis 31d).
inline constexpr double kRigaudBassSteigung = -0.0618;
inline constexpr double kRigaudBassAchse = -6.89;

// --- der Block *Keyboard* am Instrument (HA2 a, HA4 a) -----------------------

/// Die Art eines Pedals; je Platz sind nur bestimmte erlaubt (`pedalErlaubt`).
enum class PedalArt : int
{
    Keins = 0,
    UnaCorda = 1,   ///< links: die Mechanik verschiebt sich, der Hammer trifft weniger Saiten
    Leise = 2,      ///< links, *Soft*: die Haemmer ruecken naeher an die Saiten
    Sostenuto = 3,  ///< Mitte: haelt die Daempfer, die beim Treten oben sind
    Moderator = 4,  ///< Mitte, *Practice*: ein Filz zwischen Hammer und Saite
    Sustain = 5,    ///< rechts: hebt alle Daempfer
};

/// Der Platz eines Pedals.
enum class PedalPlatz : int
{
    Links = 0,
    Mitte = 1,
    Rechts = 2,
};

/// Darf diese Art auf diesem Platz stehen (HA4 a)? Keins ueberall.
[[nodiscard]] bool pedalErlaubt(PedalPlatz platz, PedalArt art);
/// Wie die Art in einer Datei steht: `none`, `unaCorda`, `soft`, `sostenuto`,
/// `practice`, `sustain`.
[[nodiscard]] QLatin1String pedalName(PedalArt art);
/// Fehlt die Art oder ist sie unbekannt: keins.
[[nodiscard]] PedalArt pedalAusName(const QString& name);

/// Die erste Taste ohne Daempfer in der Vorgabe: G6 — die obersten 18 Tasten.
/// **Vorlaeufig, ohne Beleg an der Quelle** (Befund S146: Hersteller nennen keine
/// Zahl, Foren E6 bis G6); in 31d an *Salamander* zu pruefen.
inline constexpr int kUngedaempftAbVorgabe = 91;

/// Die Ausstattung einer Tastatur (HA4 a). Die Vorgaben sind die des Fluegels.
struct Tastatur
{
    int tiefste = kTasteMin;   ///< `range`: die tiefste Taste
    int hoechste = kTasteMax;  ///< die hoechste
    /// `undampedFrom`: die erste Taste ohne Daempfer; `hoechste` + 1 = jede hat einen.
    int ungedaempftAb = kUngedaempftAbVorgabe;
    PedalArt links = PedalArt::UnaCorda;
    PedalArt mitte = PedalArt::Sostenuto;
    PedalArt rechts = PedalArt::Sustain;

    [[nodiscard]] bool operator==(const Tastatur&) const = default;
};

/// In die Grenzen: der Umfang innerhalb `kTasteMin` … `kTasteMax` (tiefste nicht
/// ueber hoechste), `ungedaempftAb` in `tiefste` … `hoechste` + 1, ein Pedal, das
/// auf seinem Platz nicht erlaubt ist, wird keins.
void richteTastatur(Tastatur& tastatur);
/// Der Block `keyboard`: `{"range": [tiefste, hoechste], "undampedFrom": …,
/// "pedals": {"left": …, "middle": …, "right": …}}` — und zurueck (gerichtet).
[[nodiscard]] QJsonObject tastaturAlsJson(const Tastatur& tastatur);
[[nodiscard]] Tastatur tastaturAusJson(const QJsonObject& o);

// --- die Anschlagstaerke (HA3 a, HK2 a) --------------------------------------

/// So schnell trifft der Hammer bei voller Lautstaerke der Note, in m/s (KTH,
/// forte, „about 5 m/s“; Stulov: „not greater than 5 m/s“).
inline constexpr double kHammerTempoMax = 5.0;
/// Der Abstand der Stufen in dB der Hammergeschwindigkeit — bis 31d ihn festlegt.
inline constexpr double kAnschlagSchrittDb = 1.0;
/// Die leiseste Stufe (−40 dB, 0,05 m/s); darunter wird nur der Pegel leiser.
inline constexpr int kAnschlagStufeMin = -40;

/// Die Stufe einer Note mit dieser Lautstaerke: round(20·lg laut / Schritt), auf
/// `kAnschlagStufeMin` … 0. Lautstaerke 0 oder weniger: die leiseste.
[[nodiscard]] int anschlagStufe(double laut);
/// Die Hammergeschwindigkeit einer Stufe in m/s: vMax · 10^(stufe·Schritt/20).
[[nodiscard]] double hammerTempo(int stufe);
/// Der Pegel, der den Unterschied zwischen Note und Stufe genau ausgleicht:
/// laut / 10^(stufe·Schritt/20).
[[nodiscard]] double anschlagRest(double laut, int stufe);

}  // namespace lumi::loop
