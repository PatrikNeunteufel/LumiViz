/**
 ****************************************************************************************
 * @file   Fluegelmessung.hpp
 * @brief  Stufe 31d-3: einen Fluegelton messen und je Taste verdichten — fuer *Salamander*
 *         und fuer die gerechneten Toene gleich (docs/audio/Spielweisen_Konzept.md §4.1;
 *         Entscheide Patrik S151/S152: UF1 a, UF3 a, UF4 a, UF6 a, D1 a, V1 a, IK1)
 *
 * @details
 * Reines C++ ohne Qt (IK1). Ein Ton wird je Kanal mit `Teiltonmessung.hpp` gemessen und auf
 * das verdichtet, was 31d braucht (`KanalMessung`, ohne die Huellkurven); `verdichte` macht
 * daraus je Taste die Zeilen der Tabelle A (`TastenWerte`). `schreibeWerte` / `liesWerte`
 * halten sie in `werte/salamander_c5.txt` (UF3 a: nur Zahlen, mit Quelle und Lizenz).
 * Dazu fuer das Setzen an den Kurven (31d-7 B, 7.5.2): `messeTon` mit `TonKurven` legt je Teilton 1–3 die
 * Kurve nach Schroeder und die Autokovarianz ab (Kanaele in der Energie gemittelt, `kanalMittel`);
 * `schreibeKurven` / `liesKurven` halten sie je Taste und Ebene v6–v16 in `werte/salamander_kurven.txt`.
 * 7.5.7 (ZF4 a): `toleranzAus` misst je Groesse die Streuung der Aufnahme selbst — benachbarte Ebenen am Kanalmittel
 * und die beiden Kanaele derselben Ebene (`messeTon` mit `einzeln`), die groessere ist die Toleranz; `ziele` legt
 * Ziel (Median ueber die Ebenen) und Toleranz je Groesse zusammen, `schreibeZiele` / `liesZiele` halten sie in
 * `werte/salamander_ziel.txt`. Die Groessen an der Struktur des Ziels (7.5.8) liefert `Strukturmessung.hpp`
 * (`messeTon` mit `huellen`).
 *
 * Die Tasten von *Salamander*: 30 in kleinen Terzen von A0 bis C8, je 16 Ebenen `v1`–`v16`
 * (Befund S152). `undampedFrom` steht nicht hier — es kommt aus der SFZ (D1 a).
 ****************************************************************************************
 */

#pragma once

#include "InstrumentMeasure/Teiltonmessung.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <compare>
#include <fstream>
#include <iomanip>
#include <map>
#include <numbers>
#include <optional>
#include <span>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace lumi::messung
{

inline constexpr int kEbenen = 16;

struct SalamanderTaste
{
    int midi = 0;
    std::string name;  ///< wie im Dateinamen, etwa `D#1`
};

/// Die 30 Tasten von *Salamander*: A0, C1, D#1, F#1, A1, … C8.
std::vector<SalamanderTaste> salamanderTasten();

/// Die WAV-Datei einer Taste und Ebene; `ordner` ist der Ordner mit den WAVs oder der darueber
/// (`…\SalamanderGrandPianoV3_48khz24bit`, dann `48khz24bit` darin).
std::filesystem::path salamanderDatei(const std::filesystem::path& ordner, const SalamanderTaste& taste, int ebene);

/// Die Tonhoehe einer Taste in gleichstufiger Stimmung, A4 = 440 Hz.
double tastenFrequenz(int midi);

/// Ein Teilton, verdichtet.
struct TeiltonKurz
{
    int k = 0;
    double frequenz = 0.0;
    double anfangDb = 0.0;
    Abklingen abklingen;
    Schwebung schwebung;
    /// HU2 a: erste Stufe aus der Huellkurve vorwaerts, Tiefe des Knicks — aus `messeTon` das Kanalmittel in der
    /// Energie (in beiden Kanaelen gleich), aus `messeKanal` allein der eine Kanal.
    Vorwaerts vorwaerts;
};

/// Ein Kanal eines Tons, verdichtet: was 31d je Ton braucht.
struct KanalMessung
{
    bool erkannt = false;
    double einsatzS = 0.0;
    double endeS = 0.0;    ///< gemessen bis hier (vor einer Blende am Ende, `messeTon`)
    double laengeS = 0.0;  ///< die Laenge des ganzen Tons
    double f0 = 0.0;
    double b = 0.0;
    double bFehler = 0.0;
    bool hochErkannt = false;
    double sigmaB1 = 0.0;
    double sigmaB3 = 0.0;
    double highDecay = 0.0;
    std::vector<TeiltonKurz> teiltoene;
    double pegelDb = -300.0;
    double direktDb = 0.0;
    std::vector<double> terzDb;
    std::vector<Phantom> phantome;
    /// Der ganze Ton ab seiner Spitze bis −20, −30, −40 dB, in s (HU2 a, `huellkurveZeiten`); 0 = so weit faellt er nicht.
    /// Aus `messeTon` das Kanalmittel in der Energie (in beiden Kanaelen gleich), aus `messeKanal` allein der eine Kanal.
    std::array<double, 3> huelleS{};

    [[nodiscard]] const TeiltonKurz* teilton(int k) const
    {
        for (const TeiltonKurz& t : teiltoene)
            if (t.k == k) return &t;
        return nullptr;
    }
};

/// Die Teiltoene, deren Kurven gesetzt werden (SM4 a: Teilton 1–3).
inline constexpr int kKurvenTeiltoene = 3;

/// Der ganze Ton ab seiner Spitze (HU2 a, S159): die Huellkurve als Energie ueber 50 ms gemittelt, in dB; zurueck die
/// Zeit von der Spitze bis −20, −30 und −40 dB darunter, in s — 0, wo der Ton so weit nicht faellt. Die Pruefgroesse
/// aus HU1 a: sie zeigt, was man als Laenge hoert, gleich wie sich die Stufen auf die Teiltoene verteilen.
std::array<double, 3> huellkurveZeiten(std::span<const double> ton, int rate);

/// Einen Kanal messen. `mitPhantomen` nur, wo sie gebraucht werden (Bass, laut — es kostet ein
/// langes Spektrum). `huellen` (wenn gegeben) bekommt Teilton 1 bis `kKurvenTeiltoene` mit Huellkurve und Boden.
KanalMessung messeKanal(std::span<const double> ton, int rate, double nennFrequenz, const lumi::kern::Fft& fft,
                               bool mitPhantomen, std::vector<Teilton>* huellen = nullptr);

/// Ein Ton in Stereo (links und rechts im Wechsel) als zwei Kanaele.
std::pair<std::vector<double>, std::vector<double>> trenne(std::span<const float> stereo);

/// Phantomteiltoene werden bis zu dieser Taste gemessen (Messplan Tabelle A: im Bass).
inline constexpr int kPhantomBis = 45;

/// Die Kurven eines Teiltons fuer das Setzen (SM4 a, 7.5.2): beide Kanaele in der Energie gemittelt (K1 a).
struct TeiltonKurven
{
    bool erkannt = false;
    std::size_t bereichVon = 0;  ///< in Rahmen ab dem Einsatz (`Einstellungen::schrittS`)
    std::size_t bereichBis = 0;
    Abklingkurve kurve;          ///< nach Schroeder bis `bereichBis` (K5 a, K6 a)
    Autokovarianz kovarianz;     ///< des Rests (K3 a)
};

/// Die Kurven eines Tons, Teilton 1 bis `kKurvenTeiltoene` (Index k − 1).
using TonKurven = std::array<TeiltonKurven, kKurvenTeiltoene>;

/// Zwei Kanaele eines Teiltons in einen: Huellkurve und Boden je Rahmen in der Energie gemittelt (K1 a), der Bereich
/// vom spaeteren Anfang bis zum spaeteren Ende. Die Rahmen beider Kanaele zaehlen je ab ihrem Einsatz — bei
/// *Salamander* liegen die Einsaetze der Kanaele innerhalb weniger Millisekunden.
Teilton kanalMittel(const Teilton& a, const Teilton& b);

/// Die Kurven aus den Huellen beider Kanaele (`messeKanal`): je Teilton gemittelt, die Kurve nach Schroeder bis zum
/// Ende des Bereichs und die Autokovarianz ueber ihn.
TonKurven kurvenAus(const std::vector<Teilton>& links, const std::vector<Teilton>& rechts, double schrittS);

/// Ein Ton in Stereo: die Blende am Ende einmal an der Summe beider Kanaele bestimmt und an beiden
/// gleich abgeschnitten (`endeVorBlende`), dann jeder Kanal gemessen. `kurven` (wenn gegeben) bekommt die
/// Kurven fuer das Setzen (7.5.2) — aus derselben Messung, die Werte aendern sich nicht.
/// Die Huellen eines Tons fuer die Messung an der Struktur (7.5.8, ZF3 a): je Teilton 1–3 das Kanalmittel (K1 a) und
/// jeder Kanal fuer sich, mit `pegelDb`, `bodenDb` und Bereich (`Teilton`).
struct TonHuellen
{
    std::vector<Teilton> mittel;
    std::vector<Teilton> links;
    std::vector<Teilton> rechts;
};

/// Der Teilton k einer Liste, `nullptr` ohne ihn.
const Teilton* teiltonK(const std::vector<Teilton>& v, int k);

std::array<KanalMessung, 2> messeTon(std::span<const float> stereo, int rate, double nennFrequenz,
                                            const lumi::kern::Fft& fft, bool mitPhantomen, TonKurven* kurven = nullptr,
                                            std::array<KanalMessung, 2>* einzeln = nullptr, TonHuellen* huellen = nullptr);

// ---------------------------------------------------------------------------------------
// Verdichten je Taste
// ---------------------------------------------------------------------------------------

/// Die Zeilen der Tabelle A fuer eine Taste (beide Kanaele zusammen, wo nicht anders gesagt).
/// Jeder Wert traegt die Zahl der Messungen, aus denen er stammt (0 = nicht gemessen).
struct Wert
{
    double wert = 0.0;
    int anzahl = 0;
};

struct TastenWerte
{
    int midi = 0;
    Wert b;                ///< Median ueber Ebenen und Kanaele mit mindestens fuenf Teiltoenen
    Wert t60Frueh;         ///< Teilton 1, erste Stufe; Median ueber v6–v16
    Wert t60Spaet;         ///< Teilton 1, zweite Stufe, nur wo zwei Stufen erkannt
    Wert knickS;
    Wert zweiStufenAnteil; ///< Anteil der Messungen von Teilton 1 mit zwei Stufen
    /// Teilton 2 und 3 wie Teilton 1 (SM3 a): erste Stufe, zweite Stufe (nur wo zwei erkannt), Anteil mit zwei Stufen
    std::array<Wert, 2> t60FruehK;
    std::array<Wert, 2> t60SpaetK;
    std::array<Wert, 2> zweiStufenAnteilK;
    Wert t60Ausgleich;     ///< T60 bei f1 aus σ = b1 + b3·ω² (auch wo Teilton 1 schwach ist)
    Wert highDecay;
    Wert schwebungCent;    ///< Teiltoene 1–3, 1200·log2(1 + f_Schwebung / f_k)
    Wert tiefeDb;          ///< Tiefe der Einbrueche, Teiltoene 1–3 mit Schwebung, Median ueber v6–v16 (P4 a, TB1 a)
    Wert tiefeLeise;       ///< dieselbe ueber v6–v10 (US2 a: das Ziel von `stossLeise`)
    Wert tiefeLaut;        ///< dieselbe ueber v12–v16 (US2 a: das Ziel von `stossLaut`)
    // HU2 a (S159): die Huellkurve vorwaerts als Mess- und Setzgroesse, Median ueber v6–v16 und beide Kanaele
    Wert vorwaertsT60;                   ///< Teilton 1: T60 aus der Geraden der ersten 0,5 s nach der Spitze
    std::array<Wert, 2> vorwaertsT60K;   ///< Teilton 2 und 3
    Wert knickTiefe;                     ///< Teilton 1: Pegel am Knick unter der Spitze, dB (nur wo zwei Stufen)
    std::array<Wert, 2> knickTiefeK;     ///< Teilton 2 und 3
    std::array<Wert, 3> huelle;          ///< der ganze Ton ab seiner Spitze bis −20 / −30 / −40 dB, s
    std::vector<Wert> helligkeit;  ///< Teilton n = 2 … 8: Steigung dB Helligkeit je dB Pegel (V1 a)
    Wert pegelumfang;      ///< Pegel des Tons zu Beginn (0,3 s), v16 gegen v1
    Wert pegelL;           ///< Pegel zu Beginn gegen das Mittel der Tastatur, je Kanal (UF6 a)
    Wert pegelR;
    Wert direkt;           ///< Direktschall, v16
    std::vector<double> terzL;  ///< Terzpegel v12, je Kanal
    std::vector<double> terzR;
    std::vector<std::pair<int, double>> phantome;  ///< n, Abstand in dB (v16, Mittel der Kanaele)
};

namespace detail
{

Wert medianWert(std::vector<double> werte);

}  // namespace detail

/// Je Taste: `messungen[e][k]` ist Ebene e + 1 (v1 … v16), Kanal k (0 links, 1 rechts). Die Pegel
/// gegen das Mittel der Tastatur setzt `setzeRelativePegel` danach ueber alle Tasten.
TastenWerte verdichte(int midi, const std::vector<std::array<KanalMessung, 2>>& messungen);

/// Die Pegel zu Beginn je Taste und Kanal gegen das Mittel der Tastatur (UF6 a): je Ebene der
/// Abstand zum Mittel ueber die Tasten, dann der Median ueber die Ebenen.
void setzeRelativePegel(std::vector<TastenWerte>& werte,
                               const std::vector<std::vector<std::array<KanalMessung, 2>>>& messungen);

// ---------------------------------------------------------------------------------------
// Die Datei werte/salamander_c5.txt
// ---------------------------------------------------------------------------------------

/// Kopf der Datei: Herkunft und Lizenz (UF3 a, CC BY 3.0 verlangt die Nennung).
inline const char* const kSalamanderKopf =
    "# Gemessen an Salamander Grand Piano V3 (Yamaha C5), Alexander Holm 2011, CC BY 3.0\n"
    "# (http://creativecommons.org/licenses/by/3.0/), Fassung 48 kHz/24 bit von archive.org, SHA-256 des Archivs\n"
    "# 04a4442f2432d846f7a6aaf7156e83e06a84342f4d7475e6e070587b3b28ef21. Nur Zahlen; geschrieben von\n"
    "# test_LoopSalamander.cpp (LUMIVIZ_SALAMANDER=<Ordner>), Messung Teiltonmessung.hpp / Fluegelmessung.hpp.\n"
    "# Zeile: Taste <Tab> Groesse <Tab> Wert <Tab> Zahl der Messungen (bei Listen: die Werte)\n"
    "# Seit S159 (HU2 a): vorwaerts_t60[_k] = T60 aus der Geraden der ersten 0,5 s nach der Spitze des Teiltons,\n"
    "# knick_tiefe_db[_k] = Pegel am Knick unter der Spitze, huelle_20/30/40_s = der ganze Ton ab seiner Spitze.\n";

void schreibeZeile(std::ostream& aus, int midi, const char* groesse, const Wert& w);

/// Kopf fuer dieselbe Messung am Modell (31d-3 Teil 2): die Stufen −30 … 0 dB in Schritten von 2 stehen
/// an der Stelle der Ebenen v1 … v16 (V1 a).
inline const char* const kModellKopf =
    "# Gemessen am gerechneten Steinway D: Hammer-Saite mit Boden (lib:steinway_d, Hoerorte left/right), wie im\n"
    "# Programm (rendereTon, 48 kHz); Anschlagstufen -30 ... 0 dB in Schritten von 2 an der Stelle der Ebenen\n"
    "# v1 ... v16 (V1 a). Geschrieben von test_LoopSalamander.cpp (LUMIVIZ_31D3_MODELL=<Ordner>).\n"
    "# Zeile: Taste <Tab> Groesse <Tab> Wert <Tab> Zahl der Messungen (bei Listen: die Werte)\n";

bool schreibeWerte(const std::filesystem::path& pfad, const std::vector<TastenWerte>& werte, const char* kopf = kSalamanderKopf);

/// Gelesen: je Taste je Groesse die Zahlen der Zeile (bei Einzelwerten Wert und Anzahl).
using SalamanderWerte = std::map<int, std::map<std::string, std::vector<std::string>>>;

std::optional<SalamanderWerte> liesWerte(const std::filesystem::path& pfad);

/// Die gelesene Datei zurueck in `TastenWerte` (fuer den Vergleich mit dem Modell und fuer 31d-4).
std::vector<TastenWerte> tastenWerteAus(const SalamanderWerte& gelesen);

// ---------------------------------------------------------------------------------------
// Die Kurven je Taste und Ebene (7.5.2; K1 a, K2 a, K5 a)
// ---------------------------------------------------------------------------------------

/// Die Ebenen, deren Kurven eingehen (K2 a): v6–v16.
inline constexpr int kKurvenEbeneAb = 6;

/// Der Schluessel einer Kurve: Taste, Ebene (1 … 16), Teilton (1 … `kKurvenTeiltoene`).
struct KurvenSchluessel
{
    int midi = 0;
    int ebene = 0;
    int k = 0;

    [[nodiscard]] auto operator<=>(const KurvenSchluessel&) const = default;
};

using KurvenBezug = std::map<KurvenSchluessel, TeiltonKurven>;

inline const char* const kKurvenKopf =
    "# Kurven von Salamander Grand Piano V3 (Yamaha C5), Alexander Holm 2011, CC BY 3.0\n"
    "# (http://creativecommons.org/licenses/by/3.0/), Fassung 48 kHz/24 bit von archive.org, SHA-256 des Archivs\n"
    "# 04a4442f2432d846f7a6aaf7156e83e06a84342f4d7475e6e070587b3b28ef21. Nur Zahlen; geschrieben von\n"
    "# test_LoopSalamander.cpp (LUMIVIZ_SALAMANDER=<Ordner>), Messung Teiltonmessung.hpp / Fluegelmessung.hpp.\n"
    "# Je Taste, Ebene v6-v16, Teilton 1-3 (beide Kanaele in der Energie gemittelt; Spielweisen_Konzept 7.5, K1 a-K6 a):\n"
    "# Taste <Tab> Ebene <Tab> Teilton <Tab> schroeder <Tab> Bereich von <Tab> bis (Rahmen zu 10 ms ab dem Einsatz)\n"
    "#   <Tab> erster Punkt <Tab> gueltig <Tab> L in dB je 0,05 s ab erster Punkt * 0,05 s\n"
    "# Taste <Tab> Ebene <Tab> Teilton <Tab> kovarianz <Tab> C in dB^2 je 0,02 s ab 0\n";

bool schreibeKurven(const std::filesystem::path& pfad, const KurvenBezug& bezug);

/// Gelesen wie geschrieben; Raster nach K5 a (0,05 s und 0,02 s).
std::optional<KurvenBezug> liesKurven(const std::filesystem::path& pfad);

// ---------------------------------------------------------------------------------------
// Die Toleranzen je Taste und Groesse (7.5.7; ZF1 a, ZF2 a, ZF4 a, ZF5 a)
// ---------------------------------------------------------------------------------------

/// Die Skala, in der eine Groesse verglichen wird — Abweichung und Toleranz in derselben (ZF5 a): Zeiten als Faktor
/// (Betrag des natuerlichen Logarithmus des Verhaeltnisses, wie die Zeiten unter den Parametern in SP2 a), Pegel und
/// Tiefen in dB, die Schwebung in Cent.
enum class Skala
{
    Faktor,
    Dezibel,
    Cent
};

const char* skalaName(Skala s);

std::optional<Skala> skalaAus(const std::string& name);

/// Der Abstand zweier Werte in der Skala der Groesse (Faktor: beide Werte muessen positiv sein).
double abstandIn(Skala s, double a, double b);

/// Das Mittel zweier Werte in der Skala der Groesse (Faktor: geometrisch — das Mittel der Logarithmen).
double mittelIn(Skala s, double a, double b);

/// Die Toleranz einer Groesse an einer Taste (ZF4 a): die groessere von zwei gemessenen Streuungen bei *Salamander* —
/// der Median des Abstands benachbarter Ebenen (v6–v16, am Kanalmittel) und der Median des Abstands der beiden Kanaele
/// derselben Ebene. Beide in der Skala der Groesse.
struct Toleranz
{
    Skala skala = Skala::Dezibel;
    double ebenen = 0.0;
    double kanaele = 0.0;
    int paareEbenen = 0;
    int paareKanaele = 0;

    [[nodiscard]] double wert() const { return std::max(ebenen, kanaele); }
    [[nodiscard]] bool gemessen() const { return paareEbenen + paareKanaele > 0; }
};

/// Je Taste die Toleranz je Groesse. Die Namen wie in `salamander_c5.txt` (`vorwaerts_t60[_k]`, `knick_tiefe_db[_k]`,
/// `t60_spaet[_k]`, `huelle_20/30/40_s`); Schwebung und Tiefe je Teilton (ZF2 a) heissen `schwebung_cent_k` und
/// `tiefe_db_k` mit k = 1 … 3 — die Zeilen `schwebung_cent` und `tiefe_db` der Wertedatei fassen die Teiltoene zusammen.
using TastenToleranzen = std::map<std::string, Toleranz>;
using Toleranzen = std::map<int, TastenToleranzen>;

/// Eine Groesse, die gesetzt wird (ZF2 a mit HU2 a): Name, Skala, und wie sie aus einer Kanalmessung kommt
/// (`std::nullopt`, wo sie dort nicht erkannt ist).
struct SetzGroesse
{
    std::string name;
    Skala skala = Skala::Dezibel;
    std::optional<double> (*wert)(const KanalMessung&, int k) = nullptr;
    int k = 0;  ///< Teilton (0: der ganze Ton — `k` zaehlt dann die Grenze −20/−30/−40 dB als 0 … 2)
};

namespace detail
{

std::optional<double> vorwaertsT60Von(const KanalMessung& m, int k);
std::optional<double> knickTiefeVon(const KanalMessung& m, int k);
std::optional<double> t60SpaetVon(const KanalMessung& m, int k);
std::optional<double> schwebungCentVon(const KanalMessung& m, int k);
std::optional<double> tiefeVon(const KanalMessung& m, int k);
std::optional<double> huelleVon(const KanalMessung& m, int g);

}  // namespace detail

/// Die Groessen, die gesetzt werden (ZF2 a mit HU2 a): je Teilton 1–3 die erste Stufe vorwaerts, die Tiefe des Knicks,
/// die spaete Stufe, die Schwebung und die Tiefe der Einbrueche; dazu der ganze Ton bis −20/−30/−40 dB.
std::vector<SetzGroesse> setzGroessen();

/// Die Werte einer Groesse an einer Ebene: am Kanalmittel und je Kanal (`std::nullopt`, wo sie nicht gemessen ist).
struct EbenenWerte
{
    std::optional<double> mittel;
    std::array<std::optional<double>, 2> kanaele;
};

/// Eine Groesse mit ihren Werten je Ebene (ab `kKurvenEbeneAb`, in der Reihenfolge der Ebenen).
struct GroesseJeEbene
{
    std::string name;
    Skala skala = Skala::Dezibel;
    std::vector<EbenenWerte> ebenen;
};

/// Die Toleranz aus den Werten je Ebene (ZF4 a): der Median des Abstands benachbarter Ebenen am Kanalmittel (wo beide
/// gemessen sind) und der Median des Abstands der beiden Kanaele derselben Ebene (wo beide gemessen sind).
Toleranz toleranzAus(Skala skala, const std::vector<EbenenWerte>& ebenen);

/// Das Ziel einer Groesse: der Median der Werte am Kanalmittel ueber die Ebenen (ZF3 a: einmal gemessen, als Ziel).
Wert zielAus(const std::vector<EbenenWerte>& ebenen);

/// Die Groessen aus der Erkennung je Ebene (ZF2 a mit HU2 a, `setzGroessen`): `messungen[e]` ist Ebene e + 1 wie in
/// `verdichte` (aus `messeTon`: Huellkurve vorwaerts und ganzer Ton als Kanalmittel), `einzeln[e]` dieselbe Ebene mit
/// jedem Kanal fuer sich (`messeTon` mit `einzeln`; leer erlaubt — dann ohne Kanalabstand fuer die Groessen des
/// Kanalmittels). Das Kanalmittel einer Groesse je Kanal: beide erkannt, ihr Mittel in der Skala; einer, der eine.
std::vector<GroesseJeEbene> erkennungsGroessen(const std::vector<std::array<KanalMessung, 2>>& messungen,
                                                      const std::vector<std::array<KanalMessung, 2>>& einzeln = {});

/// Die Toleranzen einer Taste aus den Groessen je Ebene (ZF4 a); nur Groessen mit wenigstens einem Paar.
TastenToleranzen toleranzen(const std::vector<GroesseJeEbene>& groessen);

/// Wie `toleranzen(erkennungsGroessen(messungen, einzeln))`.
TastenToleranzen toleranzen(const std::vector<std::array<KanalMessung, 2>>& messungen,
                                   const std::vector<std::array<KanalMessung, 2>>& einzeln = {});

/// Ziel und Toleranz einer Groesse an einer Taste (7.5.9 liest sie; UG1 a: die Untergrenze kommt beim Teilen, nicht hier).
struct ZielGroesse
{
    Skala skala = Skala::Dezibel;
    Wert ziel;
    Toleranz toleranz;
};
using TastenZiele = std::map<std::string, ZielGroesse>;
using Ziele = std::map<int, TastenZiele>;

/// Ziel und Toleranz je Groesse aus den Groessen je Ebene; nur Groessen mit einem Ziel und wenigstens einem Paar.
TastenZiele ziele(const std::vector<GroesseJeEbene>& groessen);

inline const char* const kZielKopf =
    "# Ziel und Toleranz von Salamander Grand Piano V3 (Yamaha C5), Alexander Holm 2011, CC BY 3.0\n"
    "# (http://creativecommons.org/licenses/by/3.0/), Fassung 48 kHz/24 bit von archive.org, SHA-256 des Archivs\n"
    "# 04a4442f2432d846f7a6aaf7156e83e06a84342f4d7475e6e070587b3b28ef21. Nur Zahlen; geschrieben von\n"
    "# test_LoopSalamander.cpp (LUMIVIZ_SALAMANDER=<Ordner>), Messung Teiltonmessung.hpp / Fluegelmessung.hpp /\n"
    "# Strukturmessung.hpp. 7.5.7 und 7.5.8 (Spielweisen_Konzept ZF1 a-ZF4 a): je Taste und Groesse, gemessen an der\n"
    "# Struktur des Ziels (salamander_struktur.txt), in der Skala der Groesse — faktor = |ln(a/b)| (Zeiten), db, cent.\n"
    "# ziel = Median ueber die Ebenen v6-v16 am Kanalmittel (anzahl Ebenen); ebenen = Median des Abstands benachbarter\n"
    "# Ebenen am Kanalmittel; kanaele = Median des Abstands links/rechts derselben Ebene; toleranz = die groessere (ZF4 a).\n"
    "# Zeile: Taste <Tab> Groesse <Tab> Skala <Tab> Ziel <Tab> Anzahl <Tab> Toleranz <Tab> Ebenen <Tab> Kanaele <Tab> Paare Ebenen\n"
    "#   <Tab> Paare Kanaele\n";

bool schreibeZiele(const std::filesystem::path& pfad, const Ziele& ziele);

std::optional<Ziele> liesZiele(const std::filesystem::path& pfad);

// ---------------------------------------------------------------------------------------
// Der Boden im Vergleich (UF6 a)
// ---------------------------------------------------------------------------------------

/// Das Verhaeltnis zweier Toene je Taste in Terzbaendern (a gegen b, in dB), das Mittel ueber die
/// Tastatur je Band (Raum, Mikrofon, Unterschied der Fluegel — nur berichtet) und je Taste die
/// Abweichung von diesem Mittel als Effektivwert ueber die Baender. Gezaehlt werden Baender ab 0,8·f
/// der Taste, die in beiden Toenen hoechstens `umfangDb` unter deren lautestem Band liegen — ein Band
/// fast ohne Schall gibt ein zufaelliges Verhaeltnis (Befund S152: die hohen Baender des Modells).
struct BodenVergleich
{
    std::vector<double> mittelDb;      ///< je Band; −300 = kein Beitrag
    std::vector<double> abweichungDb;  ///< je Taste, Effektivwert ueber die Baender
};

BodenVergleich vergleicheBoden(const std::vector<int>& tasten, const std::vector<std::vector<double>>& a,
                                      const std::vector<std::vector<double>>& b, double umfangDb = 60.0);

}  // namespace lumi::messung
