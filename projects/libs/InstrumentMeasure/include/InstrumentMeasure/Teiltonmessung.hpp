/**
 ****************************************************************************************
 * @file   Teiltonmessung.hpp
 * @brief  Teiltoene eines Tons ueber der Zeit messen: Frequenzen und B, zwei Abklingstufen,
 *         Schwebung, Helligkeit zu Beginn, Abklingen ueber der Frequenz (Stufe 31d-2;
 *         docs/audio/Spielweisen_Konzept.md §4.1, Entscheide Patrik S151: UF3 a, IK1)
 *
 * @details
 * Ein Code fuer *Salamander* und fuer die gerechneten Toene (UF3 a). Reines C++ ohne Qt,
 * nur gegen die Schnittstelle `lumi::kern::Fft` (IK1). Jede Ausgleichsrechnung ist linear
 * (Kipppunkt von UF3 a: nichtlinear mit mehreren Parametern).
 *
 * Ablauf von `miss`:
 * 1. *Einsatz:* die erste Probe ueber 1 % der Spitze.
 * 2. *Frequenzen:* ein langes Spektrum ab 50 ms nach dem Einsatz (Blackman-Harris, viermal
 *    aufgefuellt, Spitze parabolisch auf dB); Teilton k wird um k·f0·√(1 + B·k²) gesucht, B
 *    und f0 nach jedem gefundenen Teilton neu ausgeglichen: (f_k / k)² = f0² + f0²·B·k²
 *    (Gerade in k², kleinste Quadrate) ohne Ausreisser (eine fremde Spitze im Suchfenster, mehr als
 *    max(5 Cent, 4·MAD) neben der Reihe, zaehlt nicht). Gesucht wird nur im Band, das die Tonhoehe
 *    der Taste fuer Teilton k zulaesst (`stimmungCent`, `bHoechstens`) — eine Mode des Bodens neben
 *    einem schwachen Teilton 1 setzte sonst die ganze Reihe falsch. Fuenf Teiltoene in Folge ohne
 *    Spitze beenden die Suche.
 * 3. *Huellkurven:* je Teilton der Pegel ueber der Zeit aus einer Kurzzeit-FFT (Fenster
 *    `fensterPerioden` Perioden von f0 — der Nachbar liegt am Rand der Hauptkeule; Schritt
 *    `schrittS`), dazu je Rahmen der Boden in der Mitte zu beiden Nachbarn (Rauschen und das
 *    Leck der anderen Teiltoene — langsame tiefe Teiltoene legen einen fallenden Boden unter
 *    schnelle hohe).
 * 4. *Zwei Stufen* (seit N1 a auf der rueckwaerts integrierten Energie nach Schroeder, `zweiStufen`): ab dem
 *    Ende des Anschlags bis zuletzt `grenzeDb`
 *    ueber dem Boden — dem aus 3. oder, wenn der Teilton vor dem Ende darin ankommt, dem eigenen
 *    (Median der letzten 10 %, Gedanke nach Lundeby fuer Nachhall im Rauschen) —, eine Gerade
 *    und zwei Geraden mit dem besten Knick (die fruehe mindestens ein Zwanzigstel des Bereichs,
 *    die spaete ein Fuenftel; mindestens 10 dB Abfall ueber den Bereich, sonst keine Abklingzeit); zwei
 *    Stufen, wenn sie
 *    den Rest mindestens halbieren und die fruehe um das 1,3-fache steiler faellt. T60 = 60 dB
 *    durch die Steigung (wie `decay` und `lateDecay`).
 * 5. *Schwebung:* die staerkste Periodizitaet im Rest um die Geraden, mindestens zwei Perioden
 *    im Bereich, hoechstens die halbe Fensterrate; erkannt ab 1 dB Tiefe (Spitze zu Spitze) und
 *    dem Zehnfachen des Medians im Periodogramm.
 * 6. *Abklingen ueber der Frequenz:* σ = b1 + b3·ω² aus den fruehen Stufen (Bank 2010, Gl. 7);
 *    `highDecay` = σ(f1) / σ(10·f1) wie im Modell (`modell.md` §3).
 * 7. *Huellkurve vorwaerts* (HU2 a, S159; `Vorwaerts`): je Teilton die Gerade ueber die ersten `vorwaertsS` nach der
 *    Spitze als T60 — die erste Stufe, so wie man sie hoert; die Zwei-Geraden-Suche aus 4. war an *Salamander* je
 *    Kanal unstet (Befund HU1 a: C4, Teilton 1 1,2–3,7 s statt 2,5 s) — und, wo 4. zwei Stufen fand, der Pegel am
 *    Knick unter der Spitze (die Tiefe des Knicks). Der ganze Ton ab seiner Spitze steht in `Fluegelmessung.hpp`
 *    (`huellkurveZeiten`).
 *
 * Fuer das Setzen an den Kurven (31d-7 B, SM4 a, Teilplan 7.5.1, S155): je Teilton der Boden je Rahmen
 * (`bodenDb`) und der gemessene Bereich (`bereichVon`, `bereichBis`); daraus `schroederKurve` (die Kurve
 * nach Schroeder auf festem Raster, bis zu einem waehlbaren Ende, ohne die Energie des eigenen Bodens —
 * K5 a, K6 a) und `autokovarianz` (der Rest der Huellkurve um ihr gleitendes Mittel, K3 a). Beide
 * aendern an den Werten von `miss` nichts.
 *
 * Dazu fuer den ganzen Ton (31d-3): `endeVorBlende` (eine Blende am Ende der Datei abschneiden),
 * `pegelZuBeginn`, `direktschallDb` (SB1 a), `terzPegel` (UF6 a) und `phantome` (`bridgeAngle`).
 ****************************************************************************************
 */

#pragma once

#include "InstrumentCore/Fft.hpp"

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstddef>
#include <numbers>
#include <span>
#include <utility>
#include <vector>

namespace lumi::messung
{

struct Einstellungen
{
    int hoechsterTeilton = 60;
    /// Schritt der Huellkurven.
    double schrittS = 0.01;
    /// Laenge des Fensters der Huellkurven in Perioden von f0 (zwischen 20 ms und 0,5 s).
    double fensterPerioden = 8.0;
    /// Die Huellkurve zaehlt bis so weit ueber dem Rauschen.
    double grenzeDb = 10.0;
    /// … und hoechstens bis so weit unter ihre Spitze (US6 a, US6-1 a, S154): ein Ton ohne Rauschen (das Modell) faellt
    /// sonst bis zu Resten der Rechnung, und dort zaehlt die Messung Einbrueche, die keine sind; 60 dB schnitten an
    /// Salamander spaete Stufen ab, 90 dB liegen unter dem Rauschen jeder Aufnahme dort.
    double untergrenzeDb = 90.0;
    /// Die Helligkeit: die Spitze jeder Huellkurve in so viel Zeit nach dem Einsatz.
    double anfangBisS = 0.1;
    /// Eine Spitze im langen Spektrum zaehlt so weit ueber dessen Median.
    double spitzeUeberMedianDb = 20.0;
    /// Die Schwebung wird bis hoechstens hier gesucht.
    double schwebungBisHz = 8.0;
    /// Teilton k liegt zwischen k·f_Taste·2^(−stimmungCent/1200) und
    /// k·f_Taste·√((1 + bHoechstens·k²)/(1 + bHoechstens))·2^(stimmungCent/1200) — eine staerkere fremde
    /// Spitze ausserhalb (eine Mode des Bodens) ist kein Teilton (Befund S152, gerechnetes C2).
    double stimmungCent = 60.0;
    double bHoechstens = 0.02;
    /// Die Huellkurve vorwaerts: so lange nach der Spitze liegt die Gerade der ersten Stufe (HU2 a).
    double vorwaertsS = 0.5;
};

/// Die Huellkurve vorwaerts eines Teiltons (HU2 a, S159): die Steigung der ersten `vorwaertsS` nach der Spitze als
/// T60 — die erste Stufe, robust gegen das, was die Zwei-Geraden-Suche nach Schroeder an *Salamander* je Kanal
/// verbog — und der Pegel am Knick unter der Spitze.
struct Vorwaerts
{
    bool erkannt = false;
    double t60 = 0.0;           ///< s, aus der Geraden ab der Spitze ueber `vorwaertsS` (oder bis zum Ende des Bereichs)
    double bisS = 0.0;          ///< so lang war die Gerade
    bool knickErkannt = false;  ///< nur mit zwei Stufen (`Abklingen::zweiStufen`)
    /// Die Tiefe des Knicks: die Energie ab dem Knick gegen die Energie ab dem Anfang des Bereichs, in dB (negativ) —
    /// die Kurve nach Schroeder am Knick; so viel vom Ton liegt in der zweiten Stufe.
    double knickTiefeDb = 0.0;
};

struct Abklingen
{
    bool erkannt = false;
    bool zweiStufen = false;
    double t60Frueh = 0.0;  ///< s; ohne zweite Stufe gleich `t60Spaet`
    double t60Spaet = 0.0;  ///< s
    double knickS = 0.0;    ///< Zeit des Knicks nach dem Einsatz
    double vonS = 0.0;      ///< gemessener Bereich nach dem Einsatz
    double bisS = 0.0;
};

struct Schwebung
{
    bool erkannt = false;
    double frequenz = 0.0;  ///< Hz
    double tiefeDb = 0.0;   ///< Spitze zu Spitze der gefundenen Periodizitaet
    /// Tiefe der Einbrueche (TB1 a, S154): je volle Periode Maximum minus Minimum des Rests, Median ueber die
    /// Perioden — `tiefeDb` saettigt als Grundschwingung bei rund 17 dB, diese Groesse nicht.
    double einbruchDb = 0.0;
};

struct Teilton
{
    int k = 0;
    double frequenz = 0.0;  ///< Hz, aus dem langen Spektrum
    std::vector<double> pegelDb;  ///< Huellkurve, Amplitude in dB, ab dem Einsatz im Schritt `schrittS`
    /// Der Boden je Rahmen wie `pegelDb`: die Mitte zu beiden Nachbarn (Schritt 3) — Rauschen und Leck der anderen
    /// Teiltoene; `schroederKurve` zieht seine Energie ab (K6 a).
    std::vector<double> bodenDb;
    /// Der gemessene Bereich in Rahmen [von, bis): vom Ende des Anschlags bis `grenzeDb` ueber dem Boden bzw.
    /// `untergrenzeDb` unter der Spitze — auch, wenn der Teilton keine Abklingzeit bekommt; leer (0, 0), wenn
    /// es keinen gibt.
    std::size_t bereichVon = 0;
    std::size_t bereichBis = 0;
    double rauschenDb = 0.0;
    double anfangDb = 0.0;  ///< Spitze in `anfangBisS` nach dem Einsatz
    Abklingen abklingen;
    Schwebung schwebung;
    Vorwaerts vorwaerts;
};

struct Ergebnis
{
    bool erkannt = false;
    int rate = 0;
    double einsatzS = 0.0;
    double f0 = 0.0;  ///< aus dem Ausgleich; der erste Teilton liegt bei f0·√(1 + B)
    double b = 0.0;
    double bFehler = 0.0;  ///< Standardfehler von B (genaehert: nur die Steigung)
    double schrittS = 0.0;
    std::vector<Teilton> teiltoene;
    /// σ = b1 + b3·ω² ueber die fruehen Stufen; `highDecay` wie im Modell.
    bool hochErkannt = false;
    double sigmaB1 = 0.0;
    double sigmaB3 = 0.0;
    double highDecay = 0.0;

    [[nodiscard]] const Teilton* teilton(int k) const
    {
        for (const Teilton& t : teiltoene)
            if (t.k == k) return &t;
        return nullptr;
    }
};

namespace detail
{

std::vector<double> blackmanHarris(std::size_t n);

double db(double amplitude);

/// Ein gefenstertes Stueck ab `von` (ausserhalb des Tons null), aufgefuellt auf `n`, als Pegel in dB
/// je Bin; 0 dB = ein Sinus der Amplitude 1.
class Spektrum
{
public:
    Spektrum(const lumi::kern::Fft& fft, std::size_t fensterLaenge, std::size_t n);

    void rechne(std::span<const double> ton, std::ptrdiff_t von);

    [[nodiscard]] std::size_t bins() const { return m_db.size(); }
    [[nodiscard]] double binDb(std::size_t i) const { return m_db[std::min(i, m_db.size() - 1)]; }

    /// Die groesste Spitze zwischen den Bins `von` und `bis` (beide eingeschlossen), parabolisch
    /// auf dB: Stelle in Bins und Pegel.
    [[nodiscard]] std::pair<double, double> spitze(std::size_t von, std::size_t bis) const;

private:
    const lumi::kern::Fft& m_fft;
    std::vector<double> m_fenster;
    std::vector<double> m_zeit;
    std::vector<std::complex<double>> m_frei;
    std::vector<double> m_db;
    double m_normierung = 1.0;
};

/// Kleinste Quadrate einer Geraden y = a + s·x ueber Praefixsummen: jeder Bereich in O(1).
class Geraden
{
public:
    Geraden(std::span<const double> x, std::span<const double> y);

    struct Gerade
    {
        double achse = 0.0;
        double steigung = 0.0;
        double rest = 0.0;  ///< Summe der Quadrate der Abweichungen
    };

    /// Die Gerade ueber [von, bis).
    [[nodiscard]] Gerade ueber(std::size_t von, std::size_t bis) const;

private:
    std::vector<double> m_sx, m_sy, m_sxx, m_sxy, m_syy;
};

/// f0 und B aus (f_k / k)² = f0² + f0²·B·k²; `bFehler` aus dem Standardfehler der Steigung.
struct Inharmonie
{
    bool erkannt = false;
    double f0 = 0.0;
    double b = 0.0;
    double bFehler = 0.0;
};

Inharmonie gleicheAus(const std::vector<int>& k, const std::vector<double>& f);

double median(std::vector<double> werte);

/// Wie `gleicheAus`, aber ohne Ausreisser (Befund S152: am gerechneten C2 liegt neben dem schwachen
/// Teilton 2 eine staerkere fremde Spitze, 166 Cent daneben, und kippte B um das Dreihundertfache):
/// ausgleichen, die Abweichung jedes Teiltons in Cent, wer mehr als max(5 Cent, 4·1,4826·MAD) daneben
/// liegt, faellt heraus, bis sich nichts mehr aendert. `gilt` sagt je Punkt, ob er zaehlt.
Inharmonie gleicheAusRobust(const std::vector<int>& k, const std::vector<double>& f, std::vector<bool>& gilt);

/// Die Abklingzeiten aus der rueckwaerts integrierten Energie nach Schroeder (N1 a, S152): Einbrueche einer
/// Schwebung — am Modell fast vollstaendige Ausloeschungen — verbiegen Geraden in dB, die integrierte Energie
/// glaettet sie. Die Kurve L(t) = 10·lg(∫_t^Ende e / ∫_von^Ende e) faellt wie der Pegel in dB.
/// - Ausgewertet bis dorthin, wo der Pegel zuletzt 10 dB ueber dem Ende des Bereichs lag: die Abschneidung biegt
///   die Kurve am Ende, 10 dB davor ist der Fehler unter 0,5 dB.
/// - Mindestens 10 dB Abfall des Teiltons (Energie der ersten gegen die letzten 5 %), sonst keine Abklingzeit
///   (ein Teilton im Rauschen).
/// - Zwei Stufen: dieselbe Geradensuche mit Knick auf L (die fruehe darf kurz sein, die spaete mindestens ein
///   Fuenftel); die spaete Steigung ist die der Kurve. Fuer die fruehe wird die Energie der spaeten Geraden
///   abgezogen und der Rest neu integriert — sonst druckt die Energie der langen spaeten Stufe die fruehe flach.
Abklingen zweiStufen(std::span<const double> pegelDb, double schrittS, std::size_t von, std::size_t bis);

/// Der Wert der gemessenen Stufen zur Zeit t (die Geraden aus `zweiStufen`, an `pegelDb` angelegt).
std::vector<double> restUmStufen(std::span<const double> pegelDb, double schrittS, std::size_t von, std::size_t bis,
                                        const Abklingen& a);

/// Die Huellkurve vorwaerts (HU2 a, S159): die Gerade ueber `pegelDb` ab der Spitze `hoechster` ueber `vorwaertsS`
/// (hoechstens bis `bis`, mindestens zehn Rahmen) als T60; mit zwei Stufen (`a`) die Tiefe des Knicks als die Energie
/// ab dem Knick gegen die Energie ab `von` (die Kurve nach Schroeder am Knick) — so bleibt sie auch am Modell mit
/// seinen vollstaendigen Ausloeschungen stetig, wo der Pegel am Knick selbst in einem Einbruch liegen kann.
Vorwaerts huellkurveVorwaerts(std::span<const double> pegelDb, double schrittS, std::size_t hoechster, std::size_t von,
                                     std::size_t bis, double vorwaertsS, const Abklingen& a);

Schwebung findeSchwebung(std::span<const double> rest, double schrittS, double bisHz);

double median(std::vector<double> werte);

}  // namespace detail

/// Einen Ton messen. `nennFrequenz` ist die Tonhoehe der Taste (der erste Teilton liegt nahe).
Ergebnis miss(std::span<const double> ton, int rate, double nennFrequenz, const lumi::kern::Fft& fft,
                     const Einstellungen& einst = {});

/// Die Abklingkurve eines Teiltons nach Schroeder auf festem Raster (SM4 a, K5 a, K6 a; S155).
struct Abklingkurve
{
    double rasterS = 0.0;
    /// Punkt j liegt bei (ersterPunkt + j)·rasterS nach dem Einsatz — zwei Kurven mit demselben Raster
    /// liegen so Punkt fuer Punkt uebereinander.
    std::size_t ersterPunkt = 0;
    /// L = 10·lg(∫_t^Ende e / ∫_t0^Ende e) in dB, am ersten Punkt 0.
    std::vector<double> db;
    /// So viele Punkte vom Anfang bis `randS` vor dem Ende (K6-1 a) — dahinter mittelt ein Punkt ueber zu wenige
    /// Rahmen.
    std::size_t gueltig = 0;
};

/// Die Kurve nach Schroeder von Teilton `t` ueber die Rahmen [von, ende), abgetastet auf dem Raster `rasterS`
/// (der erste Punkt ist der erste Rasterpunkt ab `von`). Mit `bodenAbziehen` wird je Rahmen die Energie von
/// `bodenDb` abgezogen, unter 0 gilt 0 (K6 a). `ende` darf hinter `bereichBis` liegen (das gemeinsame Ende zweier
/// Kurven, K6 a), nicht hinter `pegelDb`; die Kurve endet frueher, wo nach dem Abzug keine Energie mehr kommt.
/// Gueltig bis `randS` vor dem Ende (K6-1 a). Leer, wenn der Bereich keinen Rasterpunkt oder keine Energie hat.
Abklingkurve schroederKurve(const Teilton& t, double schrittS, std::size_t von, std::size_t ende, double rasterS = 0.05,
                                   bool bodenAbziehen = true, double randS = 1.0);

/// Die Autokovarianz des Rests der Huellkurve um ihren glatten Verlauf (K3 a, Anteil Schwebung; S155).
struct Autokovarianz
{
    double rasterS = 0.0;
    /// C(τ) in dB² fuer τ = 0, rasterS, 2·rasterS, …
    std::vector<double> db2;
};

/// Der Rest von `pegelDb` ueber [von, bis) um sein gleitendes Mittel in dB ueber `glattS` (mittig, am Rand
/// symmetrisch gekuerzt), um den eigenen Mittelwert, davon C(τ) = Mittel von rest(t)·rest(t + τ) fuer τ bis `bisS`,
/// hoechstens die Haelfte des Bereichs,
/// auf dem Raster `rasterS`. Eine Schwebung mit Tiefe D (Spitze zu Spitze, sinusfoermig in dB) und Frequenz f gibt
/// C(τ) ≈ (D²/8)·cos(2π·f·τ); eine Schwebung langsamer als 1/`glattS` folgt teils dem glatten Verlauf.
Autokovarianz autokovarianz(const Teilton& t, double schrittS, std::size_t von, std::size_t bis, double bisS = 2.0,
                                   double rasterS = 0.02, double glattS = 2.0);

/// Wie viele Abtastwerte vor einer Blende am Ende liegen (Befund S152: jede Datei von *Salamander*
/// ist am Ende ausgeblendet; alle Teiltoene fallen dort gemeinsam steil). Gemessen am breitbandigen
/// Pegel in Rahmen von 50 ms: eine Gerade durch 40–70 % der Datei; die Blende beginnt, wo der Pegel
/// danach bis zum Ende mehr als 3 dB unter ihr bleibt — zwei Stufen oder ein Boden liegen darueber.
/// Zurueck eine Sekunde vor diese Stelle, nicht vor 70 %; ohne Blende die ganze Laenge.
std::size_t endeVorBlende(std::span<const double> ton, int rate);

// ---------------------------------------------------------------------------------------
// Groessen des ganzen Tons (31d-3): Pegel, Direktschall, Terzbaender, Phantomteiltoene.
// Alle messen ab dem Einsatz aus `miss`; gleich fuer Aufnahme und Modell (UF3 a).
// ---------------------------------------------------------------------------------------

/// Der Pegel des Tons in den ersten `dauerS` nach dem Einsatz (Effektivwert in dB, 0 dB = ein
/// Sinus der Amplitude 1).
double pegelZuBeginn(std::span<const double> ton, int rate, double einsatzS, double dauerS = 0.3);

/// Der Direktschall als Befund (SB1 a): die Energie der ersten `kurzS` nach dem Einsatz gegen die
/// der folgenden bis `langS`, je Zeit, in dB — ein Schlag vor dem Einschwingen der Teiltoene hebt sie.
double direktschallDb(std::span<const double> ton, int rate, double einsatzS, double kurzS = 0.005, double langS = 0.05);

/// Die Mitten der Terzbaender von 31,5 Hz bis 10 kHz (1000·2^(i/3), i = −15 … 10).
std::vector<double> terzMitten();

/// Der Pegel je Terzband (`terzMitten`) ueber `dauerS` ab dem Einsatz: die Leistung des Spektrums
/// (Hann) im Band, in dB — fuer die geglaettete Klangfarbe des Bodens (UF6 a). Nur Verhaeltnisse
/// zaehlen; gleich gerechnet fuer Aufnahme und Modell.
std::vector<double> terzPegel(std::span<const double> ton, int rate, double einsatzS, const lumi::kern::Fft& fft,
                                     double dauerS = 1.0);

/// Ein Phantomteilton: die Summe f_(n/2) + f_(n/2) der Laengsschwingung neben dem Querteilton n.
struct Phantom
{
    int n = 0;
    double frequenz = 0.0;
    double abstandDb = 0.0;  ///< Pegel gegen den Querteilton n
};

/// Die Phantomteiltoene bei 2·f_(n/2) fuer gerade n bis `bisN`, je gegen den Querteilton n, aus einem
/// Spektrum ueber bis zu `dauerS` ab 50 ms nach dem Einsatz (Blackman-Harris, zweimal aufgefuellt).
/// Gezaehlt nur, wo beide Spitzen mindestens vier Bins des Fensters auseinander liegen und die
/// Phantomspitze 10 dB ueber dem Median des Spektrums steht (`bridgeAngle`, Messplan Tabelle A).
std::vector<Phantom> phantome(std::span<const double> ton, int rate, const Ergebnis& e, const lumi::kern::Fft& fft,
                                     int bisN = 20, double dauerS = 4.0);

}  // namespace lumi::messung
