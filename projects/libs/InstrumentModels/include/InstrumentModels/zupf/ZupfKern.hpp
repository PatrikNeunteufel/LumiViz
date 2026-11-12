/**
 * @file ZupfKern.hpp
 * @brief InstrumentModels, Kern `pluckedString` (Klasse 2, gezupfte Saite): Tafel, Werte ↔ Spiel, Adapter, die Vorlagen
 *        der Instrumente (Instrumente_IC_Konzept.md 34k-2, IK2-3 a, IK2-4 a, IK2-8 a, IK2-10 a, IK2-11 a, GZ1 a, GZ2 a)
 *
 * **Ein Kern, fuenf Instrumente** (KS1 a): die Tafel traegt den moeglichen Umfang MIDI 21–108, jedes Instrument seinen
 * eigenen in `noteMin`/`noteMax` (Gruppe *Keys*). Die Gruppen folgen den Abschnitten von `ZupfSpiel`: *String* (Laenge
 * und Zug je Taste als Kurven, Guete), *Pluck* (Zupfpunkt je Taste, Winkel, Dauer und Kraft des Stosses, Tastenstoss,
 * Daempfen), *Bridge* (die Stegmoden als Liste), *Courses* (Saiten je Chor, Verstimmung), *Sympathetic strings* (die
 * Saiten, die ungespielt am Steg haengen — nur im Clip, `rechneZupfClip`; der Rechenweg Note rechnet ohne Mitschwingen
 * bis 34k-7, GZ3 a), *Tunable*, *Output*. **Gestimmt** (IK2-11 a) ist der Faktor auf die Laufzeiten je Note
 * (`delayFactor`, IC `stimme`) — der Stimmer `InstrumentMeasure/ZupfStimmen` rechnet ihn (34k-3; bis dahin aus
 * `werte/zupf_ic.txt`). Den Bezug `outputReference` setzt `kern::normiereNachTafel` mit der Probe von LumiViz (Mitte des
 * Umfangs, 1 s); bei allen fuenf gleich IC `normiere` (0,5 s), die Spitze liegt in der ersten halben Sekunde (J76).
 *
 * **Adapter** (Rechenweg Note, in 34k-2 wie IC): die erste Note des Auftrags, gehalten bis zu ihrem Ende, danach daempft
 * Daempfer oder Hand in `dampingTime`; die Tonhoehe aus der Note (IC), nicht aus `hz`. **Staerke aus der Stufe** (wie TM1 a
 * an Hammer, Stab und Trommel): die Kraft des Stosses folgt der Stufe in dB, `kraft = forceFf · anschlagFaktor(stufe)`,
 * nach IC ausgedrueckt die Staerke e = 1 + ln(anschlagFaktor(stufe)) / ln(forceFf/forcePp) auf 0 … 1 (Stufe 0 genau
 * e = 1; unter `forcePp` bleibt die Staerke 0, der Rest ist Pegel). Der Tastenstoss folgt derselben Staerke (IC). Ohne
 * Spuren: *laissez vibrer*, Lautenzug, Polarisation, Saite, Bund und die Verlaeufe kommen mit 34k-8 (GZ1-1 a).
 *
 * Reines C++ ohne Qt.
 */
#pragma once

#include "InstrumentCore/Tonkern.hpp"
#include "InstrumentModels/zupf/Zupf.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace lumi::kern::zupf
{

/// Der Name des Kerns in der Fabrik und in der `.lvinst` (`"core"`, IK2-8 a: nach dem Verfahren).
inline constexpr std::string_view kKernName = "pluckedString";

/// Der gestimmte Wert: der Faktor auf die Laufzeiten je Note.
inline constexpr std::string_view kLaufzeit = "delayFactor";
/// Der Bezug des Ausgangs.
inline constexpr std::string_view kBezug = "outputReference";

/// Die Art des Kerns: Tafel, Auskunft und `erzeuge`.
[[nodiscard]] const KernTyp& kernTyp();

/// Die Werte eines Spiels in der Reihenfolge der Tafel (ohne Abdruecke).
[[nodiscard]] Werte werteAus(const ZupfSpiel& spiel);
/// Das Spiel aus gerichteten Werten; fehlt ein Wert, bleibt die Vorgabe von `ZupfSpiel`.
[[nodiscard]] ZupfSpiel spielAus(const Werte& werte);

/// Die Staerke 0 … 1 (IC) zu einer Stufe: die Kraft folgt der Stufe in dB (siehe Kopf); Stufe ≥ 0 genau 1.
[[nodiscard]] double staerkeBei(const ZupfSpiel& spiel, int stufe);
/// Der Anschlag fuer die erste Note eines Auftrags (siehe Kopf).
[[nodiscard]] ZupfAnschlag anschlagAus(const ZupfSpiel& spiel, const Tonauftrag& auftrag);

/// Eine Vorlage: ein Instrument des Kerns — Name der Datei, Titel, die Werte ohne gestimmte Werte und Bezug (die setzt
/// `InstrumentTuneTool`). Die Dateien kommen mit 34k-9, soweit 34k-6 sie traegt.
struct Vorlage
{
    std::string name;
    std::string titel;
    ZupfSpiel spiel;
    /// QM1 a: die Quellen der Werte ohne Kurve, die nicht die eigene Vorgabe (`lumiviz`) sind; die Tafel traegt die der
    /// ersten Vorlage. In 34k-2 leer: die Quellen der Klasse kommen mit der Recherche 34k-4 ins Register
    QuellenJeWert quellen;
    /// mit Datei unter `asset/loop/instruments/` (wie KL1 a): in 34k-2 keine — die Dateien kommen mit 34k-9
    bool ausgeliefert = false;
};
/// Die fuenf Vorlagen in der Reihenfolge der IC-Vorgaben (Cembalo, Harfe, Gitarre, Laute, Oud; GZ1 a, GZ1-1 a).
[[nodiscard]] const std::vector<Vorlage>& vorlagen();
/// Die Werte einer Vorlage mit ihren Quellen (QM1 a): `werteAus(spiel)`, dazu je Wert ohne Kurve die Quellen der
/// Vorlage, sonst `lumiviz`.
[[nodiscard]] Werte werteVon(const Vorlage& vorlage);

/// Eine gezupfte Saite hinter `Tonkern`.
class ZupfKern final : public Tonkern
{
public:
    explicit ZupfKern(ZupfSpiel spiel);

    /// Nach dem Ende der Note: das Daempfen und die langsamste Stegmode bis −60 dB (ln 1000 · Q / (π f)), dazu 20 ms.
    [[nodiscard]] Frames nachklang(const Tonauftrag& auftrag, Frames hoechstens) const override;
    /// Eine Note (Rechenweg Note) mit `auftrag.frames` Werten; ohne Note still.
    void rechne(const Tonauftrag& auftrag, std::vector<float>& ton, Innere* innere = nullptr) const override;

    [[nodiscard]] const ZupfSpiel& spiel() const { return m_spiel; }

private:
    ZupfSpiel m_spiel;
};

}  // namespace lumi::kern::zupf
