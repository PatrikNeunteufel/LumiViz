/**
 * @file TrommelKern.hpp
 * @brief InstrumentModels, Kern `membrane` (Klasse 5, Membran): Tafel, Werte ↔ Spiel, Adapter, die Vorlagen der
 *        Instrumente (Instrumente_IC_Konzept.md 34h-3, IK2-3 a, IK2-4 a, TM1 a, TM2 a, TM4 a)
 *
 * **Ein Kern, sieben Trommeln:** die Tafel traegt den moeglichen Umfang MIDI 21–108, jedes Instrument seinen eigenen in
 * `noteMin`/`noteMax` (Gruppe *Keys*, KS1 a); eine feste Trommel hat eine Note (die Rolle im Kit), die Pauke einen Umfang
 * und `tunable`. Die Gruppen folgen den Abschnitten von `TrommelSpiel` (Keys, Heads, Shell, Stroke, Glide, Snares, Noise,
 * Roll, Output); die Felle sind eine Liste (ein oder zwei, je mit Belag als Werte). Gestimmte Werte gibt es keine; den
 * Bezug `outputReference` setzt `kern::normiereNachTafel` in zwei Runden (die Rauschanteile haengen am Bezug, IK2-10 a).
 * Ein Ton fuer das ganze Instrument: keine Kurven.
 *
 * **Adapter** (Rechenweg Note): die erste Note des Auftrags. Die Geschwindigkeit des Kopfes folgt der Stufe in dB wie bei
 * Hammer und Stab (TM1 a): tempoFf · 10^(stufe/20) — `tempoPp` aus IC steht nicht in der Tafel. Die Spielweisen sind
 * Spuren am Beginn der Note (TM2 a): `stroke` (0 normal, 1 Mitte, 2 Rand, 3 Randschlag, 4 Reifen/Cross-Stick,
 * 5 gedaempft), `roll` (≥ 0,5: Wirbel ueber die Laenge der Note), `snaresOff` (≥ 0,5: der Teppich ab); `pitch` (Cent)
 * ist das Pedal der Pauke ueber den Ton. Der Ton klingt nach dem Ende der Note aus, kein Daempfer.
 *
 * Reines C++ ohne Qt.
 */
#pragma once

#include "InstrumentCore/Tonkern.hpp"
#include "InstrumentModels/trommel/Trommel.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace lumi::kern::trommel
{

/// Der Name des Kerns in der Fabrik und in der `.lvinst` (`"core"`, TM4 a).
inline constexpr std::string_view kKernName = "membrane";

/// Die Spuren, die der Kern liest (TM2 a; feste Werte je Kit-Rolle und Drum-Spur, je Schritt mit 31g).
inline constexpr std::string_view kSpurSchlag = "stroke";          ///< 0 … 5 (`Schlag`), am Beginn der Note
inline constexpr std::string_view kSpurWirbel = "roll";            ///< 0/1 am Beginn der Note
inline constexpr std::string_view kSpurOhneTeppich = "snaresOff";  ///< 0/1 am Beginn der Note

/// Der Bezug des Ausgangs.
inline constexpr std::string_view kBezug = "outputReference";

/// Die Art des Kerns: Tafel, Auskunft und `erzeuge`.
[[nodiscard]] const KernTyp& kernTyp();

/// Die Werte eines Spiels in der Reihenfolge der Tafel (ohne Abdruecke).
[[nodiscard]] Werte werteAus(const TrommelSpiel& spiel);
/// Das Spiel aus gerichteten Werten (ohne `bereite`); fehlt ein Wert, bleibt die Vorgabe von `TrommelSpiel`.
[[nodiscard]] TrommelSpiel spielAus(const Werte& werte);

/// Der Schlag einer Spur `stroke` (gerundet, auf 0 … 5).
[[nodiscard]] Schlag schlagAus(double wert);
/// Der Anschlag fuer die erste Note eines Auftrags (siehe Kopf).
[[nodiscard]] TrommelAnschlag anschlagAus(const TrommelSpiel& spiel, const Tonauftrag& auftrag);

/// Eine Vorlage: ein mitgeliefertes Instrument des Kerns (TM4 a) — Name der Datei, Titel, die Werte ohne Bezug (den
/// setzt `InstrumentTuneTool --reference`).
struct Vorlage
{
    std::string name;
    std::string titel;
    TrommelSpiel spiel;
};
/// Die sieben Vorlagen in der Reihenfolge der IC-Vorgaben (Tom, Pauke, Bassdrum, Snare, Tabla, Conga, Bongo).
[[nodiscard]] const std::vector<Vorlage>& vorlagen();

/// Eine Trommel hinter `Tonkern` (bereitet ihre Moden beim Bau, `bereite`).
class TrommelKern final : public Tonkern
{
public:
    explicit TrommelKern(TrommelSpiel spiel);

    [[nodiscard]] Frames nachklang(const Tonauftrag& auftrag, Frames hoechstens) const override;
    /// Eine Note (Rechenweg Note) mit `auftrag.frames` Werten; ohne Note still. Innere Groessen `contact` (Kontaktzeit in
    /// s, Kraftspitze in N) und `glide` (das Gleiten der tiefsten Mode in Cent).
    void rechne(const Tonauftrag& auftrag, std::vector<float>& ton, Innere* innere = nullptr) const override;

    [[nodiscard]] const TrommelSpiel& spiel() const { return m_spiel; }

private:
    TrommelSpiel m_spiel;
};

}  // namespace lumi::kern::trommel
