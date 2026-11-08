/**
 * @file StabKern.hpp
 * @brief InstrumentModels, Kern `bar` (Klasse 4, Platte und Stab): Tafel, Werte ↔ Spiel, Adapter, die Vorlagen der
 *        Instrumente (Instrumente_IC_Konzept.md 34g-3, IK2-3 a, IK2-4 a, KS1 a–KS4 a)
 *
 * **Ein Kern, viele Instrumente** (KS1 a): die Tafel traegt den moeglichen Umfang MIDI 21–108, jedes Instrument seinen
 * eigenen in `noteMin`/`noteMax` (Gruppe *Keys*). Die Gruppen folgen den Abschnitten von `StabSpiel` (Bar, Partner,
 * Mallet, Damper, Motor, Pickup, Bow, Output). **Gestimmt** sind `partnerGain` (der Pegel des Partners, mit Ziel
 * `partnerGainDb` an `partnerGainNote`, KS2 a) und danach die sechs Fenster des Bogens (`bowForceLowPp` …, KS3 a); den
 * Bezug `outputReference` setzt `kern::normiereNachTafel` (IK2-10 a). Instrumente ohne Partner oder ohne Bogen tragen dort
 * einen Wert ohne Wirkung (der Stimmer schreibt ihn mit Herkunft) — ein gestimmter Wert fehlt nie (IK2-11 a).
 *
 * **Adapter** (Rechenweg Note): die erste Note des Auftrags; Stufe, Tonhoehe von aussen; der Daempfer greift ab dem
 * Ende der Note (`ende − beginn`, wie beim Fluegel — Sustain und Sostenuto stehen in der Planung), mit dem Andruck der
 * Spur `damperPressure` (Vorgabe 1); die Spur `motor` (U/s) wird die Passage-Spur des Motors (ohne Spur kein Motor —
 * die Phase der Klappen beginnt bei 0, damit gleiche Noten gleiche Toene bleiben); `malletOffset` verschiebt die
 * Anschlagstelle; `bowed` (≥ 0,5 am Beginn) streicht die Note ab `bowedFrom`, mit der Expression als Weg des Bogens.
 *
 * Reines C++ ohne Qt.
 */
#pragma once

#include "InstrumentCore/Tonkern.hpp"
#include "InstrumentModels/stab/Stab.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace lumi::kern::stab
{

/// Der Name des Kerns in der Fabrik und in der `.lvinst` (`"core"`, KS4 a).
inline constexpr std::string_view kKernName = "bar";

/// Die Spuren, die der Kern liest (§6a: der Editor zeigt genau diese; im Loop mit 31g).
inline constexpr std::string_view kSpurMotor = "motor";                  ///< Drehzahl in U/s
inline constexpr std::string_view kSpurGestrichen = "bowed";             ///< 0/1 am Beginn der Note
inline constexpr std::string_view kSpurSchlaegelOrt = "malletOffset";    ///< Abweichung der Anschlagstelle, x/L
inline constexpr std::string_view kSpurDaempferAndruck = "damperPressure";  ///< 0 … 1 (halbes Pedal)

/// Die Namen der gestimmten Werte, in der Reihenfolge der Tafel.
inline constexpr std::string_view kPartnerPegel = "partnerGain";
inline constexpr std::string_view kBogenUnten[3] = {"bowForceLowPp", "bowForceLowMf", "bowForceLowFf"};
inline constexpr std::string_view kBogenOben[3] = {"bowForceHighPp", "bowForceHighMf", "bowForceHighFf"};
/// Der Bezug des Ausgangs.
inline constexpr std::string_view kBezug = "outputReference";

/// Die Art des Kerns: Tafel, Auskunft und `erzeuge`.
[[nodiscard]] const KernTyp& kernTyp();

/// Die Werte eines Spiels in der Reihenfolge der Tafel (ohne Abdruecke).
[[nodiscard]] Werte werteAus(const StabSpiel& spiel);
/// Das Spiel aus gerichteten Werten; fehlt ein Wert, bleibt die Vorgabe von `StabSpiel`.
[[nodiscard]] StabSpiel spielAus(const Werte& werte);

/// Der Anschlag fuer die erste Note eines Auftrags (siehe Kopf).
[[nodiscard]] StabAnschlag anschlagAus(const StabSpiel& spiel, const Tonauftrag& auftrag);

/// Eine Vorlage: ein mitgeliefertes Instrument des Kerns (KS4 a) — Name der Datei, Titel, die Werte ohne gestimmte Werte
/// und Bezug (die setzt `InstrumentTuneTool`), ob es ausgeliefert wird.
struct Vorlage
{
    std::string name;
    std::string titel;
    StabSpiel spiel;
    bool ausgeliefert = true;
    /// QM1 a: die Quellen der Werte ohne Kurve, die nicht die eigene Vorgabe (`lumiviz`) sind (die Kurven tragen ihre in
    /// `spiel`); die Tafel traegt die der ersten Vorlage
    QuellenJeWert quellen;
};
/// Die zehn Vorlagen in der Reihenfolge von IC `alleStabSpiele` (die Steeldrum nicht ausgeliefert, KS4 a).
[[nodiscard]] const std::vector<Vorlage>& vorlagen();
/// Die Werte einer Vorlage mit ihren Quellen (QM1 a): `werteAus(spiel)`, dazu je Wert ohne Kurve die Quellen der
/// Vorlage, sonst `lumiviz`.
[[nodiscard]] Werte werteVon(const Vorlage& vorlage);

/// Ein Stabspiel hinter `Tonkern`.
class StabKern final : public Tonkern
{
public:
    explicit StabKern(StabSpiel spiel);

    [[nodiscard]] Frames nachklang(const Tonauftrag& auftrag, Frames hoechstens) const override;
    /// Eine Note (Rechenweg Note) mit `auftrag.frames` Werten; ohne Note still. Innere Groesse `contact`: Kontaktzeit
    /// in s und Kraftspitze in N.
    void rechne(const Tonauftrag& auftrag, std::vector<float>& ton, Innere* innere = nullptr) const override;

    [[nodiscard]] const StabSpiel& spiel() const { return m_spiel; }

private:
    StabSpiel m_spiel;
};

}  // namespace lumi::kern::stab
