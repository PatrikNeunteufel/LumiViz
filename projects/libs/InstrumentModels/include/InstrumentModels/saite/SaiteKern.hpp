/**
 * @file SaiteKern.hpp
 * @brief InstrumentModels, Kern `struckString` (Klasse 1 Rest, angeschlagene Saite): Tafel, Werte ↔ Spiel, Adapter, die
 *        Vorlagen der Instrumente (Instrumente_IC_Konzept.md 34i-3, KL1 a, KL2 a, KL4 a, KL4-1 a)
 *
 * **Ein Kern, drei Instrumente** (KL1 a): das Clavinet (Tangente: Stoss und Klemme, zwei Tonabnehmer, Verstaerker) wird
 * ausgeliefert; Hackbrett und Cimbalom (Schlaegel, Mittelsteg, Mitschwingen je Clip) ziehen als Vorlagen mit, ohne
 * Datei (Deluxe, wie die Steeldrum in KS4 a). Die Tafel traegt den moeglichen Umfang A0–C8, jedes Instrument seinen in
 * `noteMin`/`noteMax` (Gruppe *Keys*, KS1 a); die Kurven reichen ueber die ganze Tafel (KL4-1 a). Die Gruppen folgen
 * `SaitenSpiel`: *Keys*, *String*, *Decay*, *Course*, *Excitation*, *Tangent*, *Damper*, *Pickups*, *Amp*, *Output*.
 * Den Bezug `outputReference` setzt `kern::normiereNachTafel` (IK2-10 a, eine Runde: der Bezug teilt nur den Ausgang).
 *
 * **Adapter** (Rechenweg Note): die erste Note des Auftrags; Stufe in dB und Tonhoehe von aussen; losgelassen am Ende der
 * Note (`ende − beginn`; Sustain steht in der Planung), der Daempfer wirkt, wo das Instrument einen hat. Spuren am Beginn
 * der Note (im Loop mit 31g): `damperSlider` (0 … 1, der Daempferschieber des Clavinets), `keyPressure` (Cent nach oben,
 * an der Grenze der Tafel gekappt — Grenze 0 bis zu einer Messung, IC F-4-1 a), `pickupSelection` (0 beide, 1 Steg,
 * 2 Mitte, 3 Gegenphase). Mitschwingen der anderen Choere gibt es nur je Clip (`rechneSaitenClip`; Rechenweg Clip mit
 * 31e/34k) — hier aus.
 *
 * Reines C++ ohne Qt.
 */
#pragma once

#include "InstrumentCore/Tonkern.hpp"
#include "InstrumentModels/saite/Saite.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace lumi::kern::saite
{

/// Der Name des Kerns in der Fabrik und in der `.lvinst` (`"core"`, KL1 a).
inline constexpr std::string_view kKernName = "struckString";

/// Die Spuren, die der Kern liest (§6a: der Editor zeigt genau diese; im Loop mit 31g).
inline constexpr std::string_view kSpurSchieber = "damperSlider";    ///< 0 … 1 am Beginn der Note
inline constexpr std::string_view kSpurTastendruck = "keyPressure";  ///< Cent nach oben am Beginn der Note
inline constexpr std::string_view kSpurAbnehmer = "pickupSelection"; ///< 0 … 3 (`AbnehmerWahl`) am Beginn der Note

/// Der Bezug des Ausgangs.
inline constexpr std::string_view kBezug = "outputReference";

/// Die Art des Kerns: Tafel, Auskunft und `erzeuge`.
[[nodiscard]] const KernTyp& kernTyp();

/// Die Werte eines Spiels in der Reihenfolge der Tafel.
[[nodiscard]] Werte werteAus(const SaitenSpiel& spiel);
/// Das Spiel aus gerichteten Werten; fehlt ein Wert, bleibt die Vorgabe von `SaitenSpiel`.
[[nodiscard]] SaitenSpiel spielAus(const Werte& werte);

/// Die Wahl der Tonabnehmer aus dem Wert einer Spur `pickupSelection` (gerundet, auf 0 … 3).
[[nodiscard]] AbnehmerWahl abnehmerWahlAus(double wert);
/// Der Anschlag fuer die erste Note eines Auftrags (siehe Kopf).
[[nodiscard]] SaitenAnschlag anschlagAus(const SaitenSpiel& spiel, const Tonauftrag& auftrag);

/// Eine Vorlage: ein mitgeliefertes Instrument des Kerns — Name der Datei, Titel, die Werte ohne Bezug (den setzt
/// `InstrumentTuneTool`), ob es ausgeliefert wird.
struct Vorlage
{
    std::string name;
    std::string titel;
    SaitenSpiel spiel;
    bool ausgeliefert = true;
    /// QM1 a: die Quellen der Werte ohne Kurve, die nicht die eigene Vorgabe (`lumiviz`) sind (die Kurven tragen ihre in
    /// `spiel`); die Tafel traegt die der ersten Vorlage
    QuellenJeWert quellen;
};
/// Die Vorlagen in der Reihenfolge von IC `test_hammer_rest.cpp`: Clavinet, Hackbrett, Cimbalom (nur das Clavinet
/// ausgeliefert, KL1 a).
[[nodiscard]] const std::vector<Vorlage>& vorlagen();
/// Die Werte einer Vorlage mit ihren Quellen (QM1 a).
[[nodiscard]] Werte werteVon(const Vorlage& vorlage);

/// Eine angeschlagene Saite hinter `Tonkern`.
class SaiteKern final : public Tonkern
{
public:
    explicit SaiteKern(SaitenSpiel spiel);

    [[nodiscard]] Frames nachklang(const Tonauftrag& auftrag, Frames hoechstens) const override;
    /// Eine Note (Rechenweg Note) mit `auftrag.frames` Werten; ohne Note still. Innere Groessen `contact` (Kontaktzeit
    /// in s, Kraftspitze in N) und `release` (Clavinet: der erste Teilton der ganzen Saite nach dem Loslassen, Hz).
    void rechne(const Tonauftrag& auftrag, std::vector<float>& ton, Innere* innere = nullptr) const override;

    [[nodiscard]] const SaitenSpiel& spiel() const { return m_spiel; }

private:
    SaitenSpiel m_spiel;
};

}  // namespace lumi::kern::saite
