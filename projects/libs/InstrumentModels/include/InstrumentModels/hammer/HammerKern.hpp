/**
 * @file HammerKern.hpp
 * @brief InstrumentModels, Kern `hammerString`: der Fluegel als erster Kern hinter `Tonkern` — Tafel, Werte ↔ Spiel,
 *        Adapter (Instrumente_IC_Konzept.md 34d, IK2-3 a, IK2-4 a, FU1 a, FU2 a)
 *
 * Die Tafel traegt den Umfang je Instrument (Gruppe *Keys*, `noteMin`/`noteMax`, KL4 a — die Kurven reichen ueber die
 * ganze Tafel A0–C8, KL4-1 a), die 22 Kurven aus `kurvenAngaben()` (Gruppen *String*, *Choir*, *Decay*, *Hammer*, *Damper*) und
 * die Gruppe *Pedals* (FU2 a): `undampedFrom`, `softFactor`, `moderatorStiffness`, `moderatorExponent`. Der
 * Resonanzboden steht nicht in der Tafel, sondern an der Schicht (FU1 a) — die App faltet ihn.
 *
 * Der Adapter haelt das Spiel (`HammerSaite`, `Pedalwerte`) und rechnet je Auftrag eine Note mit `rechneHammerTon`.
 * Getreten werden *Soft* und *Practice* ueber die Spuren `soft` und `practice` (Wert ≥ 0,5 am Beginn der Note);
 * Sustain und Sostenuto stehen in der Planung (`halten`).
 *
 * Reines C++ ohne Qt.
 */
#pragma once

#include "InstrumentModels/hammer/HammerSaite.hpp"
#include "InstrumentModels/hammer/HammerTon.hpp"
#include "InstrumentCore/Tonkern.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace lumi::kern::hammer
{

/// Der Name des Kerns in der Fabrik und in der `.lvinst` (`"core"`).
inline constexpr std::string_view kKernName = "hammerString";

/// Die Spuren, die der Kern liest (FU2 a): Pedal *Soft* und *Practice* getreten (0/1).
inline constexpr std::string_view kSpurSoft = "soft";
inline constexpr std::string_view kSpurPractice = "practice";

/// Die erste Taste ohne Daempfer in der Vorgabe: F6 — die obersten 20 Tasten (SFZ von *Salamander*: *Notes without
/// dampers* ab `lokey=89`; D1 a, S152).
inline constexpr int kUngedaempftAbVorgabe = 89;

/// Die Werte der Gruppe *Pedals* (FU2 a; aus IC `Tastatur`, Klasse 1 Rest).
struct Pedalwerte
{
    /// die erste Taste ohne Daempfer; `kTasteMax` + 1 = jede hat einen
    int ungedaempftAb = kUngedaempftAbVorgabe;
    /// *Soft*: so viel langsamer trifft der Hammer (1 = wie ohne; offen, IC K1 F-4-1 a)
    double softFaktor = 1.0;
    /// *Practice*: der Filz des Moderators in Reihe, F = Q_m·δ_m^p_m (offen, IC Entwurf §2.1)
    double moderatorSteife = 2.0;
    double moderatorExponent = 2.0;

    [[nodiscard]] bool operator==(const Pedalwerte&) const = default;
};

/// Die Art des Kerns: Tafel, Auskunft und `erzeuge`.
[[nodiscard]] const KernTyp& kernTyp();

/// Die Werte einer Hammer-Saite (der Umfang `noteMin`/`noteMax` nach KL4 a, alle Kurven, dazu die Pedalwerte) in der
/// Reihenfolge der Tafel.
[[nodiscard]] Werte werteAus(const HammerSaite& saite, const Pedalwerte& pedale = {},
                             std::pair<int, int> umfang = {kTasteMin, kTasteMax});
/// Die Hammer-Saite aus gerichteten Werten (`richteWerte` ohne Fehler); fehlt eine Kurve, bleibt sie leer.
[[nodiscard]] HammerSaite saiteAus(const Werte& werte);
/// Die Pedalwerte aus gerichteten Werten; fehlt einer, gilt die Vorgabe.
[[nodiscard]] Pedalwerte pedalwerteAus(const Werte& werte);

/// Eine Vorlage: ein mitgeliefertes Instrument des Kerns neben dem Fluegel (34i, KL2 a) — Name der Datei, Titel, Saite
/// und Pedalwerte, ob es ausgeliefert wird. Der Fluegel selbst ist die Tafel (`hammerSaiteVorgabe`), keine Vorlage.
struct Vorlage
{
    std::string name;
    std::string titel;
    HammerSaite saite;
    Pedalwerte pedale;
    /// der Tastenumfang des Instruments (KL4 a), tiefste und hoechste Taste
    std::pair<int, int> umfang{kTasteMin, kTasteMax};
    bool ausgeliefert = true;
    /// QM1 a: die Quellen der Werte ohne Kurve, die nicht die eigene Vorgabe (`lumiviz`) sind (die Kurven tragen ihre
    /// in `saite`)
    QuellenJeWert quellen;
};
/// Die Vorlagen aus IC `VorgabenRest.cpp` in dessen Reihenfolge: Upright, Honky-tonk, CP-70 (34i-1).
[[nodiscard]] const std::vector<Vorlage>& vorlagen();
/// Die Werte einer Vorlage mit ihren Quellen (QM1 a): `werteAus(saite, pedale)`, dazu je Wert ohne Kurve die Quellen
/// der Vorlage, sonst `lumiviz`.
[[nodiscard]] Werte werteVon(const Vorlage& vorlage);

/// Der Anschlag fuer die erste Note eines Auftrags: Taste, Tonhoehe, Stufe, Haltezeit (Ende − Beginn), Daempfer nach
/// `ungedaempftAb`, Rate; Soft und Practice aus den Spuren am Beginn der Note.
[[nodiscard]] HammerAnschlag anschlagAus(const Tonauftrag& auftrag, const Pedalwerte& pedale);

/// Der Fluegel hinter `Tonkern`.
class HammerKern final : public Tonkern
{
public:
    HammerKern(HammerSaite saite, Pedalwerte pedale);

    [[nodiscard]] Frames nachklang(const Tonauftrag& auftrag, Frames hoechstens) const override;
    /// Eine Note (Rechenweg Note): der Ton von `rechneHammerTon` mit `auftrag.frames` Werten, in Einheiten von
    /// `kHammerKraftBezug` Newton am Steg. Ohne Note bleibt er still. Innere Groesse `contact`: Kontaktzeit in s und
    /// Kraftspitze in N.
    void rechne(const Tonauftrag& auftrag, std::vector<float>& ton, Innere* innere = nullptr) const override;

    [[nodiscard]] const HammerSaite& saite() const { return m_saite; }
    [[nodiscard]] const Pedalwerte& pedale() const { return m_pedale; }

private:
    HammerSaite m_saite;
    Pedalwerte m_pedale;
};

}  // namespace lumi::kern::hammer
