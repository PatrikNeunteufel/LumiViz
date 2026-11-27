/**
 * @file ZupfKern.hpp
 * @brief InstrumentModels, Kern `pluckedString` (Klasse 2, gezupfte Saite) Rechenfassung 2: Tafel, Werte ↔ Spiel, Adapter,
 *        die Vorlagen der Instrumente (Instrumente_IC_Konzept.md IK2-3 a, IK2-8 a, IK2-10 a, IK2-11 a, GZ1-1 a, GZ5 a, GZ7 a,
 *        ZK1 a, ZK2 a, ZB1 a; Schnittfolge 34k-7d-4 (c), (e))
 *
 * **Ein Kern, fuenf Instrumente** (KS1 a): die Tafel traegt den moeglichen Umfang MIDI 21–108, jedes Instrument seinen
 * eigenen in `noteMin`/`noteMax` (Gruppe *Keys*). Gruppen: *Strings* (Harfe, Cembalo: die Saiten je Taste als Kurven; Gitarre,
 * Laute, Oud: der Saitensatz `stringSet` als Liste, ein Eintrag je Saite mit ihrem Chor, dazu Mensur und Buende), *Pluck*
 * (Ort, Auslenkung oder Kraft, Gesetz je Stufe, Winkel, Breite, Abloesetempo, Notenende), *Tunable* (die Tafel des
 * Umlauf-Filters `loopFilterTable`, gestimmt aus den Saiten — ZK1 a), *Output* (Bezug). Wie jeder Wert tragen die Kurven
 * der Saiten die Vorgabe der ersten Vorlage (das Cembalo); ein Instrument mit Saitensatz traegt sie, liest sie aber nicht —
 * der Satz geht vor (`ZupfSpiel::jeSaite`).
 *
 * **Adapter** (Rechenweg Note): die erste Note des Auftrags, Saite und Bund aus ihr (ZK2 a), gehalten bis zu ihrem Ende,
 * danach daempft der Daempfer am Sattel in `dampingTime` um 60 dB (ZN1 a). Der Kern rechnet den Koerper am Steg in seiner
 * Saitenschleife (`stegImAuftrag`, GZ5 a, GZ7 a): `rechneAmSteg` gibt die Kraft senkrecht zur Decke an dem Stegpunkt der
 * Note aus, durch den Bezug geteilt; ohne `Steg` im Auftrag (oder ohne den Stegpunkt darin) rechnet er mit starrem Steg.
 * `rechne` gibt dieselbe Kraft als Ton (fuer Bezug und Proben). Der Ton setzt `kVorlauf` nach dem Beginn der Note ein.
 * Die Staerke kommt aus der Anschlagstufe (TM1 a). Mitschwingen, Tastenstoss, Lautenzug, *Pedals* folgen mit 34k-7d-5, -6.
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
/// Die Fassung der Rechnung (ZB1 a: 2 seit 34k-7d-4).
inline constexpr int kRechenFassung = 2;
/// Der Bezug des Ausgangs.
inline constexpr std::string_view kBezug = "outputReference";

/// Die Art des Kerns: Tafel, Auskunft und `erzeuge`.
[[nodiscard]] const KernTyp& kernTyp();

/// Die Werte eines Spiels in der Reihenfolge der Tafel (ohne Abdruecke).
[[nodiscard]] Werte werteAus(const ZupfSpiel& spiel);
/// Das Spiel aus gerichteten Werten; fehlt ein Wert, bleibt die Vorgabe von `ZupfSpiel`.
[[nodiscard]] ZupfSpiel spielAus(const Werte& werte);

/// Eine Vorlage: ein Instrument des Kerns — Name der Datei, Titel, die Werte ohne gestimmte Tafel und Bezug (die setzt
/// `InstrumentTuneTool`). Die Dateien kommen mit 34k-9.
struct Vorlage
{
    std::string name;
    std::string titel;
    ZupfSpiel spiel;
    /// QM1 a: die Quellen der Werte ohne Kurve, die nicht die eigene Vorgabe (`lumiviz`) sind
    QuellenJeWert quellen;
    /// mit Datei unter `asset/loop/instruments/` (wie KL1 a): noch keine — die Dateien kommen mit 34k-9
    bool ausgeliefert = false;
};
/// Die fuenf Vorlagen (Cembalo, Harfe, Gitarre, Laute, Oud; GZ1 a, GZ1-1 a).
[[nodiscard]] const std::vector<Vorlage>& vorlagen();
/// Die Werte einer Vorlage mit ihren Quellen (QM1 a).
[[nodiscard]] Werte werteVon(const Vorlage& vorlage);

/// Eine gezupfte Saite hinter `Tonkern`.
class ZupfKern final : public Tonkern
{
public:
    explicit ZupfKern(ZupfSpiel spiel);

    /// Nach dem Ende der Note: Vorlauf, `dampingTime`, die langsamste Mode des Stegs im Auftrag bis −60 dB
    /// (ln 1000 / (π f η)), dazu 20 ms.
    [[nodiscard]] Frames nachklang(const Tonauftrag& auftrag, Frames hoechstens) const override;
    /// Die Kraft am Stegpunkt der Note als Ton (`auftrag.frames` Werte); ohne Note still.
    void rechne(const Tonauftrag& auftrag, std::vector<float>& ton, Innere* innere = nullptr) const override;
    /// Die Kraft am Stegpunkt der Note (eine `Stegkraft`, Taste des Stegpunkts; ohne Note Taste −1, still).
    void rechneAmSteg(const Tonauftrag& auftrag, std::vector<Stegkraft>& kraefte, Innere* innere = nullptr) const override;

    [[nodiscard]] const ZupfSpiel& spiel() const { return m_spiel; }

private:
    ZupfSpiel m_spiel;
};

}  // namespace lumi::kern::zupf
