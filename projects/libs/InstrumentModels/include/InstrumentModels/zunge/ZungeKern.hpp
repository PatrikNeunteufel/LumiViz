/**
 * @file ZungeKern.hpp
 * @brief InstrumentModels, Kern `freeReed` (Klasse 10, durchschlagende Zunge): Tafel, Werte ↔ Spiel, Adapter, die
 *        Vorlagen der Instrumente (Instrumente_IC_Konzept.md 34j-2, IK2-3 a, IK2-4 a, IK2-8 a, IK2-10 a, IK2-11 a)
 *
 * **Ein Kern, vier Instrumente** (KS1 a): die Tafel traegt den moeglichen Umfang MIDI 21–108, jedes Instrument seinen
 * eigenen in `noteMin`/`noteMax` (Gruppe *Keys*, dazu `bassBelow`: darunter spielt die Bass-Seite des Akkordeons, 0 =
 * keine). Die Gruppen folgen den Abschnitten von `ZungenSpiel` (Keys, Reed, Chamber, Wind, Ranks, Bass ranks, Case,
 * Tunable, Output); die Choere sind Listen. **Gestimmt** (IK2-11 a) sind f₀ der Zunge je Note (`reedFrequency`) und das
 * Fenster des Drucks, in dem sie anspricht (`windLow`, `windHigh`) — der Stimmer `InstrumentMeasure/ZungeStimmen`
 * rechnet sie (34j-3; bis dahin aus `werte/zunge_ic.txt`). Den Bezug `outputReference` setzt `kern::normiereNachTafel`
 * mit der Probe von LumiViz (Mitte des Umfangs, 1 s) — IC normierte 0,6 s an der Mitte ab `bassBelow`; am Akkordeon gibt
 * das einen anderen Bezug (Uebernahme-Journal J67).
 *
 * **Adapter** (Rechenweg Note, in 34j-2 wie IC: feste Werte am Beginn der Note): die erste Note des Auftrags; die Expression
 * (Spur `expression` am Beginn, ohne Spur 1 — SK4 a) waehlt den Druck im Fenster der Note (IC); `pitch` (Cent, nur abwaerts,
 * gekappt) schliesst das Ventil, `vibrato` (Cent) moduliert den Druck mit 5 Hz; gehalten wird bis zum Ende der Note, danach
 * setzt der Wind in 30 ms ab. Die Stufe wirkt in 34j-2 nicht (die Abbildung Stufe → Staerke legt 34j-7 an der gemessenen
 * Kurve fest, SK4 a); die Tonhoehe kommt aus der Note (IC), nicht aus `hz`. Laufender Wind, `bellows` und der Ausklang der
 * Zunge kommen mit 34j-7 (ZV3 a).
 *
 * Reines C++ ohne Qt.
 */
#pragma once

#include "InstrumentCore/Tonkern.hpp"
#include "InstrumentModels/zunge/Zunge.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace lumi::kern::zunge
{

/// Der Name des Kerns in der Fabrik und in der `.lvinst` (`"core"`, IK2-8 a: nach dem Verfahren).
inline constexpr std::string_view kKernName = "freeReed";

/// Die Namen der gestimmten Werte, in der Reihenfolge der Tafel.
inline constexpr std::string_view kEigenfrequenz = "reedFrequency";
inline constexpr std::string_view kDruckUnten = "windLow";
inline constexpr std::string_view kDruckOben = "windHigh";
/// Der Bezug des Ausgangs.
inline constexpr std::string_view kBezug = "outputReference";
/// Die innere Groesse: die Spitze der Auslenkung der ersten Zunge / ihre Laenge.
inline constexpr std::string_view kInnereSpitze = "reedTip";

/// Die Art des Kerns: Tafel, Auskunft und `erzeuge`.
[[nodiscard]] const KernTyp& kernTyp();

/// Die Werte eines Spiels in der Reihenfolge der Tafel (ohne Abdruecke).
[[nodiscard]] Werte werteAus(const ZungenSpiel& spiel);
/// Das Spiel aus gerichteten Werten; fehlt ein Wert, bleibt die Vorgabe von `ZungenSpiel`.
[[nodiscard]] ZungenSpiel spielAus(const Werte& werte);

/// Der Anschlag fuer die erste Note eines Auftrags (siehe Kopf).
[[nodiscard]] ZungenAnschlag anschlagAus(const Tonauftrag& auftrag);

/// Eine Vorlage: ein Instrument des Kerns — Name der Datei, Titel, die Werte ohne gestimmte Werte und Bezug (die setzt
/// `InstrumentTuneTool`). Die Dateien kommen mit 34j-8 (IK2-8 a), soweit 34j-5 sie traegt.
struct Vorlage
{
    std::string name;
    std::string titel;
    ZungenSpiel spiel;
    /// QM1 a: die Quellen der Werte ohne Kurve, die nicht die eigene Vorgabe (`lumiviz`) sind; die Tafel traegt die der
    /// ersten Vorlage. In 34j-2 leer: die Quellen der Klasse kommen mit der Recherche 34j-4 ins Register
    QuellenJeWert quellen;
};
/// Die vier Vorlagen in der Reihenfolge der IC-Vorgaben (Akkordeon, Harmonium, Melodica, Mundharmonika).
[[nodiscard]] const std::vector<Vorlage>& vorlagen();
/// Die Werte einer Vorlage mit ihren Quellen (QM1 a): `werteAus(spiel)`, dazu je Wert ohne Kurve die Quellen der
/// Vorlage, sonst `lumiviz`.
[[nodiscard]] Werte werteVon(const Vorlage& vorlage);

/// Eine Zunge hinter `Tonkern`.
class ZungeKern final : public Tonkern
{
public:
    explicit ZungeKern(ZungenSpiel spiel);

    /// Bis 34j-7 hoechstens eine Sekunde (gemessen klingt die Zunge 31–777 ms nach dem Absetzen des Winds aus, S188).
    [[nodiscard]] Frames nachklang(const Tonauftrag& auftrag, Frames hoechstens) const override;
    /// Eine Note (Rechenweg Note) mit `auftrag.frames` Werten; ohne Note still. Innere Groesse `reedTip`.
    void rechne(const Tonauftrag& auftrag, std::vector<float>& ton, Innere* innere = nullptr) const override;

    [[nodiscard]] const ZungenSpiel& spiel() const { return m_spiel; }

private:
    ZungenSpiel m_spiel;
};

}  // namespace lumi::kern::zunge
