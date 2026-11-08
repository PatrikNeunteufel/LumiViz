/**
 ****************************************************************************************
 * @file   Strukturmessung.hpp
 * @brief  7.5.8 (SM4 b): einen Teilton an der Struktur des Ziels messen — zwei Geraden mit dem Knick von
 *         *Salamander*, die Schwebung als Anteil bei der Frequenz von *Salamander*, ohne eigene Erkennung
 *         (docs/audio/Spielweisen_Konzept.md §4.1; Entscheide Patrik S156/S161: ZF2 a, ZF3 a, ZF4 a, SG1 a)
 *
 * @details
 * Reines C++ ohne Qt. Die Struktur eines Teiltons (`TeiltonStruktur`: Bereich, Knick, Schwebungsfrequenz) kommt
 * einmal aus der Erkennung an *Salamander* (`strukturAus` je Ebene am Kanalmittel, `verdichteStruktur` ueber die
 * Ebenen v6–v16: Knick und Schwebung, wo mindestens die Haelfte der Ebenen sie erkennt). An dieser Struktur werden
 * dann *beide* Seiten gemessen (`misstAnStruktur`): die erste Stufe vorwaerts ab der eigenen Spitze (HU2 a), die
 * Tiefe des Knicks und die spaete Stufe (Gerade an der Kurve nach Schroeder ab dem Knick der Struktur) und die
 * Schwebung als Anteil des Rests bei der Frequenz der Struktur (Spitze zu Spitze) mit der Tiefe der Einbrueche je
 * Periode dieser Frequenz. Alle Groessen haengen glatt von der Huellkurve ab — keine Schwelle, keine Wahl des
 * besten Kandidaten (ZF3 a). `strukturGroessen` legt sie je Ebene fuer Ziel und Toleranz (`Fluegelmessung.hpp`:
 * `ziele`, `toleranzAus`) zurecht; `schreibeStruktur` / `liesStruktur` halten die Struktur in
 * `werte/salamander_struktur.txt`.
 ****************************************************************************************
 */

#pragma once

#include "InstrumentMeasure/Fluegelmessung.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <filesystem>
#include <fstream>
#include <map>
#include <numbers>
#include <optional>
#include <span>
#include <sstream>
#include <string>
#include <vector>

namespace lumi::messung
{

/// Die Struktur eines Teiltons des Ziels (ZF3 a): der gemessene Bereich in Rahmen ab dem Einsatz, der Knick (wo zwei
/// Stufen erkannt sind) und die Schwebungsfrequenz (wo eine erkannt ist). `ebenen`, `mitKnick`, `mitSchwebung`: so
/// viele Ebenen trugen zur Verdichtung bei.
struct TeiltonStruktur
{
    bool da = false;
    std::size_t von = 0;
    std::size_t bis = 0;
    bool knick = false;
    double knickS = 0.0;
    bool schwebt = false;
    double schwebungHz = 0.0;
    int ebenen = 0;
    int mitKnick = 0;
    int mitSchwebung = 0;
};

/// Die Struktur eines Tons, Teilton 1 bis `kKurvenTeiltoene` (Index k − 1).
using TonStruktur = std::array<TeiltonStruktur, kKurvenTeiltoene>;

/// Bis zu welcher Frequenz `miss` die Schwebung an einem Ton dieser Nennfrequenz sucht (die halbe Fensterrate).
double schwebungBisHz(double nennFrequenz, const Einstellungen& einst = {});

/// Die Struktur eines Teiltons an einer Ebene aus dem Kanalmittel, mit der Erkennung von `miss` (zwei Stufen nach
/// Schroeder, Schwebung im Rest um die Stufen) — nur fuer das Ziel (ZF3 a).
TeiltonStruktur strukturAus(const Teilton& m, double nennFrequenz, const Einstellungen& einst = {});

/// Die Struktur ueber die Ebenen: Bereich als Median; Knick und Schwebung, wo mindestens die Haelfte der Ebenen mit
/// Bereich sie erkennt (Median ueber diese); sonst eine Gerade bzw. keine Schwebung (ZF3 a, ZF7 a).
TeiltonStruktur verdichteStruktur(const std::vector<TeiltonStruktur>& ebenen);

/// Die Groessen eines Teiltons an fremder Struktur (ZF3 a).
struct StrukturWerte
{
    bool gemessen = false;
    /// Die erste Stufe ab der eigenen Spitze (HU2 a); `knickTiefeDb` am Knick der Struktur.
    Vorwaerts vorwaerts;
    /// Die spaete Stufe: Gerade an der Kurve nach Schroeder ab dem Knick der Struktur (ohne Knick: ueber den Bereich).
    bool spaet = false;
    double t60Spaet = 0.0;
    /// Die Schwebung bei der Frequenz der Struktur: Anteil des Rests um die Geraden, Spitze zu Spitze in dB, und die
    /// Tiefe der Einbrueche je Periode dieser Frequenz (Median), wie `findeSchwebung` — ohne Suche der Frequenz.
    bool schwebung = false;
    double schwebungDb = 0.0;
    double einbruchDb = 0.0;
};

/// Misst den Teilton `t` an der Struktur `s`: im Bereich der Struktur (gekuerzt auf die Huellkurve von `t`), zwei
/// Geraden mit dem Knick der Struktur (wo sie einen hat), die Schwebung bei ihrer Frequenz. `randS` wie K6-1 a: die
/// Gerade der spaeten Stufe endet so weit vor dem Ende, wo genug Rahmen bleiben.
StrukturWerte misstAnStruktur(const Teilton& t, const TeiltonStruktur& s, const Einstellungen& einst = {}, double randS = 1.0);

/// Die Groessen an der Struktur je Ebene (ZF2 a mit HU2 a): je Teilton 1–3 `vorwaerts_t60[_k]` (Faktor),
/// `knick_tiefe_db[_k]` (dB), `t60_spaet[_k]` (Faktor), `schwebung_db_k` (dB, Spitze zu Spitze bei der Frequenz der
/// Struktur), `tiefe_db_k` (dB). `huellen[e]` sind die Huellen der Ebene e (ab `kKurvenEbeneAb`, aus `messeTon` mit
/// `huellen`), `struktur` die des Ziels. Der ganze Ton (`huelle_*_s`) kommt nicht von hier (`erkennungsGroessen`).
std::vector<GroesseJeEbene> strukturGroessen(const std::vector<TonHuellen>& huellen, const TonStruktur& struktur,
                                                    const Einstellungen& einst = {});

/// Der ganze Ton bis −20/−30/−40 dB je Ebene (`huelle_20/30/40_s`, Faktor) aus den Kanalmessungen wie `erkennungsGroessen`.
std::vector<GroesseJeEbene> huellenGroessen(const std::vector<std::array<KanalMessung, 2>>& messungen,
                                                   const std::vector<std::array<KanalMessung, 2>>& einzeln = {});

// ---------------------------------------------------------------------------------------
// Die Datei werte/salamander_struktur.txt
// ---------------------------------------------------------------------------------------

inline const char* const kStrukturKopf =
    "# Struktur des Ziels von Salamander Grand Piano V3 (Yamaha C5), Alexander Holm 2011, CC BY 3.0\n"
    "# (http://creativecommons.org/licenses/by/3.0/), Fassung 48 kHz/24 bit von archive.org, SHA-256 des Archivs\n"
    "# 04a4442f2432d846f7a6aaf7156e83e06a84342f4d7475e6e070587b3b28ef21. Nur Zahlen; geschrieben von\n"
    "# test_LoopSalamander.cpp (LUMIVIZ_SALAMANDER=<Ordner>), Messung Teiltonmessung.hpp / Strukturmessung.hpp.\n"
    "# 7.5.8 (Spielweisen_Konzept ZF3 a): je Taste und Teilton 1-3 der Bereich in Rahmen zu 10 ms ab dem Einsatz\n"
    "# (Median ueber die Ebenen v6-v16 am Kanalmittel), der Knick in s (0 = eine Gerade) und die Schwebung in Hz\n"
    "# (0 = keine) - Knick und Schwebung nur, wo mindestens die Haelfte der Ebenen sie erkennt.\n"
    "# Zeile: Taste <Tab> Teilton <Tab> von <Tab> bis <Tab> Knick s <Tab> Schwebung Hz <Tab> Ebenen <Tab> mit Knick\n"
    "#   <Tab> mit Schwebung\n";

using Strukturen = std::map<int, TonStruktur>;

bool schreibeStruktur(const std::filesystem::path& pfad, const Strukturen& strukturen);

std::optional<Strukturen> liesStruktur(const std::filesystem::path& pfad);

}  // namespace lumi::messung
