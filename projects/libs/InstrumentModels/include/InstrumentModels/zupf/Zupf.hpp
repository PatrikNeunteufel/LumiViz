/**
 * @file Zupf.hpp
 * @brief InstrumentModels, Kern `pluckedString` (Klasse 2, gezupfte Saite) Rechenfassung 2: das Spiel eines Instruments
 *        und die Note in der Saitenschleife (Instrumente_IC_Konzept.md GZ1-1 a, GZ5 a, GZ6 a, GZ10 a, GZ13 a, KQ30 a, ZK1 a,
 *        ZK2 a, ZB1 a, ZU1 a, ZW1 a, ZD1 a, ZD1-1 a, ZD2 a, ZN1 a; Herleitung `docs/modellierung/gezupfte-saite/
 *        2026-11-18_S205_herleitung.md`; Schnittfolge 34k-7d-4 (c))
 *
 * Seit 34k-7d-4 ersetzt Rechenfassung 2 die Fassung aus IC (ZB1 a). Ein `ZupfSpiel` traegt, was die Tafel des Kerns traegt:
 *
 * - **Saiten** (GZ1-1 a): Harfe und Cembalo je Taste als Kurven (Laenge, Zug, Masse je Laenge, Biegesteife, Verluste;
 *   monofil mit Durchmesser und Dichte, sonst nach Valette) — an jeder Taste ein Stuetzpunkt, die Kurven treffen die
 *   Saiten der Vorgaben (`zupf/Saiten`) genau; Gitarre, Laute und Oud einen Saitensatz (`Saitensatz`: Mensur, Buende,
 *   Choere mit leerer Saite, Oktave und Laengenfaktor). Ein Satz mit Choeren geht vor den Kurven.
 * - **Zupf**: der Ort je Taste (m vom Steg; ZW1 a an Gitarre, Laute, Oud 16 cm fest), die Auslenkung (Gitarre 4 mm, Laute
 *   und Oud 2 mm, Harfe 4 mm) oder beim Cembalo die Kraft je Taste beim Abloesen (KQ30 a, GZ13 a), der Pegel je dB Stufe
 *   (Auslenkung bzw. Kraft · 10^(Pegel · Stufe / 20): Gitarre, Laute, Oud, Harfe 1 nach GZ6 a, ZD1 a, ZD2 a; Cembalo
 *   1/18 — pp 1 dB unter ff, GZ10 a), die Neigung je Stufe k (GZ6 a: Gitarre 0,16, Laute und Oud uebertragen, ZD2 a;
 *   Harfe 0, ZD1-1 a; Cembalo 0), Winkel, Fingerbreite und die Geschwindigkeit beim Abloesen (Harfe 2 m/s, mit der
 *   Auslenkung im selben Verhaeltnis, ZD1 a).
 * - **Notenende** (ZN1 a): ein Daempfer am Sattel, in `daempfenS` um 60 dB.
 * - **Umlauf-Filter** (ZK1 a): die Tafel `loopFilterTable`, gestimmt (`zupf/UmlaufTafel`).
 * - **Bezug** des Ausgangs.
 *
 * Die Note in der Schleife (`schleifeFuer`): die Saite(n) der Taste bzw. des Chors (Saite und Bund aus `KettenNote`, ZK2 a;
 * ohne Angabe die hoechste Saite, deren leere Taste nicht ueber der Note liegt — wie der Pruefstand), das Umlauf-Filter
 * aus der Tafel (je Taste das Glied am Grundton gestimmt; je Saite `filterAn` an der Lage, ein im Einklang verstimmter Chor
 * um −12 lb(Faktor) Buende verschoben), der Zupf nach der Stufe, der Daempfer ab dem Ende der Note. Mitschwingen,
 * Tastenstoss, Lautenzug und *Pedals* kommen mit 34k-7d-5 und -6.
 *
 * Reines C++ ohne Qt.
 */
#pragma once

#include "InstrumentCore/Kurve.hpp"
#include "InstrumentCore/Steg.hpp"
#include "InstrumentModels/zupf/Saiten.hpp"
#include "InstrumentModels/zupf/Schleife.hpp"
#include "InstrumentModels/zupf/UmlaufTafel.hpp"

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace lumi::kern::zupf
{

/// Der Vorlauf der Note in der Schleife: was vor dem Loslassen liegt (Vorschwinger des Tiefpasses der Fingerbreite und der
/// Bandgrenze), faellt hierhin; so viel spaeter setzt der Ton ein (2 ms).
inline constexpr double kVorlauf = 0.002;

/// Das Spiel eines Instruments (siehe Kopf).
struct ZupfSpiel
{
    std::string name;
    int noteMin = 40;
    int noteMax = 84;
    // --- Saiten je Taste (Harfe, Cembalo); an jeder Taste ein Stuetzpunkt ---
    Kurve laenge, zug, masse, biegesteife, verlustBiegung, verlustInnen, verlustLuft, durchmesser, dichte;
    // --- oder ein Saitensatz (Gitarre, Laute, Oud) ---
    Saitensatz satz;
    // --- Zupf ---
    Kurve zupfort;                 ///< m vom Steg, je Taste
    bool kraftGegeben = false;     ///< Cembalo: die Kraft beim Abloesen statt der Auslenkung
    double auslenkung = 0.004;     ///< m bei Stufe 0
    Kurve kraft;                   ///< N bei Stufe 0, je Taste (nur mit `kraftGegeben`)
    double pegelJeStufe = 1.0;     ///< dB der Auslenkung bzw. Kraft je dB Stufe
    double neigungJeStufe = 0.0;   ///< k, dB je Oktave um 1 kHz je dB Stufe
    double winkelGrad = 30.0;
    double breite = 0.0075;        ///< m
    double tempo = 0.0;            ///< m/s beim Abloesen bei Stufe 0
    double daempfenS = 0.08;       ///< Notenende: in so vielen s um 60 dB (ZN1 a)
    // --- Umlauf-Filter (gestimmt) ---
    std::vector<UmlaufEintrag> umlauf;
    double ausgangBezug = 1.0;

    /// je Saite des Satzes (Gitarre, Laute, Oud) statt je Taste
    [[nodiscard]] bool jeSaite() const { return !satz.choere.empty(); }
};

/// Die Saite einer Taste aus den Kurven (Harfe, Cembalo); `std::nullopt` ausserhalb des Umfangs oder ohne Laenge.
[[nodiscard]] std::optional<Tastensaite> tastensaiteAus(const ZupfSpiel& spiel, int taste);

/// Die Kurven der Saiten je Taste aus einer Funktion der Saite (die Vorgaben bauen so ihre Kurven, Belegkraft und
/// Herkunft wie angegeben); Tasten ohne Saite bekommen keinen Stuetzpunkt.
void setzeSaitenJeTaste(ZupfSpiel& spiel, const std::function<std::optional<Tastensaite>(int)>& saite, Belegkraft belegkraft,
                        const std::string& herkunft);

/// Was eine Note braucht, ausser dem Steg.
struct ZupfNote
{
    std::vector<SchleifenSaite> saiten;
    Zupfangabe zupf;
    int stegTaste = -1;   ///< der Stegpunkt der Saite(n) im Koerper
    std::string fehler;   ///< leer = gut; sonst warum die Note still bleibt
};

/// Die Note `note` bei Stufe `stufe` (dB): Saite und Bund aus der Note (−1 = ohne Angabe), gehalten `halten` Frames, dann
/// gedaempft (siehe Kopf).
[[nodiscard]] ZupfNote zupfNote(const ZupfSpiel& spiel, int note, int saite, int stufe, long long halten, double rate);

/// Die Vorgaben der fuenf Instrumente ohne gestimmte Tafel und Bezug (die setzt `InstrumentTuneTool`).
[[nodiscard]] ZupfSpiel baueVorgabeCembalo();
[[nodiscard]] ZupfSpiel baueVorgabeHarfe();
[[nodiscard]] ZupfSpiel baueVorgabeGitarre();
[[nodiscard]] ZupfSpiel baueVorgabeLaute();
[[nodiscard]] ZupfSpiel baueVorgabeOud();

}  // namespace lumi::kern::zupf
