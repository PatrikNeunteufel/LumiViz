/**
 * @file ZungeStimmen.hpp
 * @brief Messen und Stimmen am Kern `freeReed` (Klasse 10): Tonhoehe, Ansprechen, Lautheit und Einschwingen einer Zunge,
 *        f₀ je Note am mittleren Druck (Instrumente_IC_Konzept.md 34j-2, 34j-3, 34j-6, IK2-2 a, IK2-11 a, ZV5 a)
 *
 * **Messen an der Taste** (34j-6, die Messkette des Pruefstands S190 `docs/modellierung/zunge/proben/
 * 2026-11-11_S190_pruefstand.cpp`): eine Zunge allein, 1 s gehalten, 1,2 s gerechnet, 48 kHz, Wind in 20 ms auf den
 * Druck. Tonhoehe aus den aufsteigenden Nulldurchgaengen der Auslenkung um ihre Mitte im letzten Viertel des Haltens;
 * Amplitude die halbe Spanne dort; „spricht“ = Amplitude ≥ ¼ der Ruhelage und gegen das Viertel davor um nicht mehr
 * als 10 % gewachsen; Lautheit des Schalls (ρ/(4π)·dU/dt) ueber die letzten 0,4 s des Haltens mit `AudioCore`
 * (Momentary nach BS.1770, mono); Einschwingen = erster 5-ms-Block mit der Spitze ≥ −5 dB der Endhuelle (der Spitze der
 * letzten 50 Bloecke), Anstieg ab −50 dB.
 *
 * **Stimmer** (34j-6, ZV5 a): je Note f₀ in hoechstens sechs Runden so, dass die Zunge am mittleren Druck
 * `windMitte(spiel)` auf der Note spielt (auf 0,05 Cent; Start 1,02 · f, sie schwingt „somewhat below“ f₀) — wie der
 * Pruefstand. Spricht sie dort nicht, behaelt sie den letzten Wert, und die Note steht in `stumm`. Dann, wo das Spiel
 * ueber das Ventil bendet (ZV7 a, ZV8 a), je Note der Bend: die Tonhoehe gegen offen bei den Oeffnungen 2^(−k/4),
 * k = 1 … 7, und die Grenze (die kleinste Oeffnung, die noch anspricht, in hoechstens 12 Halbierungen auf 1 %); ohne Bend
 * flache Kurven (0 Cent, Grenze 1). Je Note auf dem Pool (`kern::parallelJe`). In einer zweiten Runde, mit der
 * gestimmten f₀ aller Noten (der Ton einer Taste rechnet die Zungen ihrer Fusslagen), je Taste am ganzen Ton Stufe →
 * Staerke (ZV10 a, `messeDynamik`) und der Ausklang (ZV11 a, ZV13 a: der laengste ueber sechs Haltedauern und drei
 * Staerken, `messeNachklang`). Bis Rechenfassung 1 (IC `stimme`, J69)
 * stimmte er ein Fenster des Drucks je Note.
 *
 * **Messen wie IC** (34j-2, J68): `messeZungenTon` — Tonhoehe mit YIN (B2, 30 … 3000 Hz) als Median der stimmhaften Rahmen
 * von der Haelfte des Haltens bis zu seinem Ende, Einschwingen (B5) ueber das Halten; fuer die Faelle aus IC `test_zunge`.
 *
 * Ohne Qt.
 */
#pragma once

#include "InstrumentMeasure/Stimmen.hpp"
#include "InstrumentModels/zunge/Zunge.hpp"

#include <cstdint>
#include <span>
#include <vector>

namespace lumi::messung
{

/// Was IC `ZungenMessung` am Ton einer Zunge mass.
struct ZungenTon
{
    double spielHz = 0.0;
    bool schwingt = false;      ///< mehr als die Haelfte der Rahmen stimmhaft — auch an stummen Toenen moeglich (S188)
    double amplitudeRel = 0.0;  ///< aus dem Kern (`reedTip`)
    double einschwingS = 0.0;
};

/// Misst den Ton einer Zunge (`rechneEineZunge`, ohne Bezug) wie IC: `haltenS` und `rate` aus dem Anschlag.
[[nodiscard]] ZungenTon messeZungenTon(std::span<const double> ton, double haltenS, int rate, double amplitudeRel);

/// Eine Zunge mit f₀ an ihrer Tonhoehe rechnen und messen wie IC; mit `ton` der Ton dazu.
[[nodiscard]] ZungenTon messeEineZunge(const kern::zunge::ZungenSpiel& spiel, double f0, int zungenNote, const kern::zunge::ZungenAnschlag& a,
                                       std::int64_t frames, std::vector<double>* ton = nullptr);

/// Die erste Zunge des Tons: der erste Chor der Seite (Bass unter `bassAb`), wie IC gemessen.
[[nodiscard]] ZungenTon messeErsteZunge(const kern::zunge::ZungenSpiel& spiel, const kern::zunge::ZungenAnschlag& a, std::int64_t frames);

/// Eine Zunge an der Taste mit der Messkette des Pruefstands (siehe Kopf).
struct ZungenTaste
{
    bool spricht = false;
    bool waechst = false;     ///< im letzten Viertel um mehr als 10 % gewachsen
    double hz = 0.0;          ///< 0 = keine drei Nulldurchgaenge
    double lufs = -200.0;     ///< Momentary ueber die letzten 0,4 s des Haltens
    double einS = -1.0;       ///< bis −5 dB der Endhuelle
    double anstiegS = -1.0;   ///< −50 → −5 dB
    double ampLaenge = 0.0;   ///< Amplitude / Laenge
    double ampRuhe = 0.0;     ///< Amplitude / Ruhelage
};
[[nodiscard]] ZungenTaste messeZungenTaste(const kern::zunge::ZungenSpiel& spiel, double f0, int zungenNote, double druckPa,
                                           double ventilFaktor = 1.0);

/// f₀ einer Zunge, so dass sie am Druck auf ihrer Note spielt (siehe Kopf); `spricht` falsch, wenn sie dort nicht anspricht.
[[nodiscard]] double stimmeZunge(const kern::zunge::ZungenSpiel& spiel, int zungenNote, double druckPa, bool* spricht = nullptr);

/// Die gestimmten Kurven eines Spiels, mit Belegkraft *Rechnung*, und die Noten, die am mittleren Druck nicht ansprechen.
struct ZungenStimmung
{
    kern::Kurve eigenfrequenz;                  ///< `reedFrequency`
    std::vector<kern::Kurve> bendCentJeStufe;   ///< `bendStep1` … `bendStep7`
    kern::Kurve bendGrenzeOeffnung;             ///< `bendLimitOpening`
    kern::Kurve bendGrenzeCent;                 ///< `bendLimitCent`
    std::vector<kern::Kurve> staerkeJeStufe;    ///< `strengthAtLevel2` … `strengthAtLevel20`
    kern::Kurve nachklang;                      ///< `releaseTime`
    std::vector<int> stumm;
};

/// Die Lautheit des ganzen Tons einer Taste (alle Choere, Gehaeuse, Bezug) bei einer Staerke: 1 s gehalten, Momentary
/// ueber die letzten 0,4 s des Haltens (ZV10 a; die Groesse des Befunds S188 und des Messfalls S191).
[[nodiscard]] double tonLautheit(const kern::zunge::ZungenSpiel& spiel, int note, double staerke);

/// Stufe → Staerke einer Taste (ZV10 a): je Stufe −2 … −20 dB die Staerke, bei der der ganze Ton so viel leiser klingt
/// als bei Staerke 1 — auf dem Raster 0, 0,05 … 1 von oben die erste Stelle, die den Pegel erreicht, dann eingegabelt
/// auf 0,05 LU (hoechstens 14 Halbierungen); leiser als bei Staerke 0 kann die Taste nicht, dort 0.
struct ZungenDynamik
{
    std::vector<double> staerkeJeStufe;
    double dynamikLu = 0.0;  ///< Staerke 1 gegen 0
};
[[nodiscard]] ZungenDynamik messeDynamik(const kern::zunge::ZungenSpiel& spiel, int note);

/// Der Ausklang einer Note nach dem Wind (ZV11 a, ZV13 a): der ganze Ton bei einer Haltedauer und Staerke; vom Ende des
/// Winds, bis jeder 5-ms-Block (Mittel der Energie) 60 dB unter dem lautesten Block derselben Note bleibt — auch wenn die
/// Zunge nach dem Wind noch anwaechst —, in Sekunden. Gerechnet wird 8 s nach dem Wind, reicht das nicht, doppelt so
/// lang, hoechstens 32 s.
[[nodiscard]] double messeNachklang(const kern::zunge::ZungenSpiel& spiel, int note, double haltenS, double staerke);
/// Die Haltedauern und Staerken, ueber die der Ausklang einer Taste gestimmt wird (ZV13 a).
inline constexpr double kNachklangHalten[] = {0.1, 0.25, 0.5, 1.0, 2.0, 4.0};
inline constexpr double kNachklangStaerken[] = {0.0, 0.5, 1.0};
/// Der Ausklang einer Taste (`releaseTime`, ZV13 a): der laengste ueber `kNachklangHalten` × `kNachklangStaerken`.
[[nodiscard]] double messeNachklang(const kern::zunge::ZungenSpiel& spiel, int note);

/// Der Bend einer Zunge am Druck (ZV8 a): gegen offen gemessen bei den Oeffnungen `bendStufe(k)`, die Grenze (letzte
/// Oeffnung, die anspricht) auf 1 % eingegabelt; Cent ≤ 0, unter der Grenze der Wert der Grenze.
struct ZungenBend
{
    std::vector<double> centJeStufe;
    double grenzeOeffnung = 1.0;
    double grenzeCent = 0.0;
};
[[nodiscard]] ZungenBend messeBend(const kern::zunge::ZungenSpiel& spiel, double f0, int zungenNote, double druckPa);
/// Stimmt ein Spiel ueber seinen Umfang `noteMin` … `noteMax`; die gestimmte Kurve in `spiel` wirkt dabei nicht.
[[nodiscard]] ZungenStimmung zungenStimmung(const kern::zunge::ZungenSpiel& spiel);

/// Die Pruefgroesse „jede Taste deckt den Windbereich“ (34j-6): je Note `noteMin` … `noteMax` die Zunge dieser
/// Tonhoehe mit der gestimmten f₀ bei `windMinPa`, am mittleren Druck und bei `windMaxPa` (die Zungen der Fusslagen
/// liegen im Umfang und sind damit mitgeprueft).
struct ZungenWindbereich
{
    std::vector<int> noten;
    std::vector<ZungenTaste> min, mitte, max;  ///< je Note
};
[[nodiscard]] ZungenWindbereich messeWindbereich(const kern::zunge::ZungenSpiel& spiel);

/// Der Stimmer des Kerns `freeReed`.
[[nodiscard]] Stimmer zungeStimmer();

}  // namespace lumi::messung
