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
 *
 * Die Tasten von *Salamander*: 30 in kleinen Terzen von A0 bis C8, je 16 Ebenen `v1`–`v16`
 * (Befund S152). `undampedFrom` steht nicht hier — es kommt aus der SFZ (D1 a).
 ****************************************************************************************
 */

#pragma once

#include "Teiltonmessung.hpp"

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
inline std::vector<SalamanderTaste> salamanderTasten()
{
    static const char* const namen[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    std::vector<SalamanderTaste> tasten;
    for (int midi = 21; midi <= 108; midi += 3)
        tasten.push_back({midi, std::string(namen[midi % 12]) + std::to_string(midi / 12 - 1)});
    return tasten;
}

/// Die WAV-Datei einer Taste und Ebene; `ordner` ist der Ordner mit den WAVs oder der darueber
/// (`…\SalamanderGrandPianoV3_48khz24bit`, dann `48khz24bit` darin).
inline std::filesystem::path salamanderDatei(const std::filesystem::path& ordner, const SalamanderTaste& taste, int ebene)
{
    const std::filesystem::path unter = ordner / "48khz24bit";
    const std::filesystem::path basis = std::filesystem::is_directory(unter) ? unter : ordner;
    return basis / (taste.name + "v" + std::to_string(ebene) + ".wav");
}

/// Die Tonhoehe einer Taste in gleichstufiger Stimmung, A4 = 440 Hz.
inline double tastenFrequenz(int midi)
{
    return 440.0 * std::exp2((midi - 69) / 12.0);
}

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
inline std::array<double, 3> huellkurveZeiten(std::span<const double> ton, int rate)
{
    std::array<double, 3> zeiten{};
    if (rate <= 0) return zeiten;
    const auto n = static_cast<std::size_t>(std::max(1, rate / 20));
    if (ton.size() < 2 * n) return zeiten;
    std::vector<double> summe(ton.size() + 1, 0.0);
    for (std::size_t i = 0; i < ton.size(); ++i) summe[i + 1] = summe[i] + ton[i] * ton[i];
    const std::size_t rahmen = ton.size() - n + 1;
    std::vector<double> db(rahmen);
    for (std::size_t i = 0; i < rahmen; ++i)
        db[i] = 10.0 * std::log10(std::max((summe[i + n] - summe[i]) / static_cast<double>(n), 1e-30));
    const auto spitze = static_cast<std::size_t>(std::max_element(db.begin(), db.end()) - db.begin());
    constexpr double kGrenzen[3] = {-20.0, -30.0, -40.0};
    for (std::size_t g = 0; g < 3; ++g)
        for (std::size_t i = spitze; i < rahmen; ++i)
            if (db[i] <= db[spitze] + kGrenzen[g])
            {
                zeiten[g] = static_cast<double>(i - spitze) / rate;
                break;
            }
    return zeiten;
}

/// Einen Kanal messen. `mitPhantomen` nur, wo sie gebraucht werden (Bass, laut — es kostet ein
/// langes Spektrum). `huellen` (wenn gegeben) bekommt Teilton 1 bis `kKurvenTeiltoene` mit Huellkurve und Boden.
inline KanalMessung messeKanal(std::span<const double> ton, int rate, double nennFrequenz, const lumi::loop::Fft& fft,
                               bool mitPhantomen, std::vector<Teilton>* huellen = nullptr)
{
    KanalMessung m;
    m.huelleS = huellkurveZeiten(ton, rate);
    const Ergebnis e = miss(ton, rate, nennFrequenz, fft);
    if (huellen != nullptr)
        for (const Teilton& t : e.teiltoene)
            if (t.k <= kKurvenTeiltoene) huellen->push_back(t);
    if (!e.erkannt) return m;
    m.erkannt = true;
    m.einsatzS = e.einsatzS;
    m.f0 = e.f0;
    m.b = e.b;
    m.bFehler = e.bFehler;
    m.hochErkannt = e.hochErkannt;
    m.sigmaB1 = e.sigmaB1;
    m.sigmaB3 = e.sigmaB3;
    m.highDecay = e.highDecay;
    for (const Teilton& t : e.teiltoene) m.teiltoene.push_back({t.k, t.frequenz, t.anfangDb, t.abklingen, t.schwebung, t.vorwaerts});
    m.pegelDb = pegelZuBeginn(ton, rate, e.einsatzS);
    m.direktDb = direktschallDb(ton, rate, e.einsatzS);
    m.terzDb = terzPegel(ton, rate, e.einsatzS, fft);
    if (mitPhantomen) m.phantome = phantome(ton, rate, e, fft);
    return m;
}

/// Ein Ton in Stereo (links und rechts im Wechsel) als zwei Kanaele.
inline std::pair<std::vector<double>, std::vector<double>> trenne(std::span<const float> stereo)
{
    std::pair<std::vector<double>, std::vector<double>> lr;
    lr.first.reserve(stereo.size() / 2);
    lr.second.reserve(stereo.size() / 2);
    for (std::size_t i = 0; i + 1 < stereo.size(); i += 2)
    {
        lr.first.push_back(stereo[i]);
        lr.second.push_back(stereo[i + 1]);
    }
    return lr;
}

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
inline Teilton kanalMittel(const Teilton& a, const Teilton& b)
{
    if (b.pegelDb.empty()) return a;
    if (a.pegelDb.empty()) return b;
    Teilton m;
    m.k = a.k;
    m.frequenz = 0.5 * (a.frequenz + b.frequenz);
    const auto mittel = [](const std::vector<double>& x, const std::vector<double>& y) {
        std::vector<double> z(std::min(x.size(), y.size()));
        for (std::size_t i = 0; i < z.size(); ++i)
            z[i] = 10.0 * std::log10(std::max(0.5 * (std::pow(10.0, x[i] / 10.0) + std::pow(10.0, y[i] / 10.0)), 1e-300));
        return z;
    };
    m.pegelDb = mittel(a.pegelDb, b.pegelDb);
    m.bodenDb = mittel(a.bodenDb, b.bodenDb);
    const bool hatA = a.bereichBis > a.bereichVon;
    const bool hatB = b.bereichBis > b.bereichVon;
    m.bereichVon = hatA && hatB ? std::max(a.bereichVon, b.bereichVon) : (hatA ? a.bereichVon : b.bereichVon);
    m.bereichBis = std::min(m.pegelDb.size(), hatA && hatB ? std::max(a.bereichBis, b.bereichBis) : (hatA ? a.bereichBis : b.bereichBis));
    if (m.bereichBis <= m.bereichVon) m.bereichVon = m.bereichBis = 0;
    return m;
}

/// Die Kurven aus den Huellen beider Kanaele (`messeKanal`): je Teilton gemittelt, die Kurve nach Schroeder bis zum
/// Ende des Bereichs und die Autokovarianz ueber ihn.
inline TonKurven kurvenAus(const std::vector<Teilton>& links, const std::vector<Teilton>& rechts, double schrittS)
{
    TonKurven kurven;
    for (int k = 1; k <= kKurvenTeiltoene; ++k)
    {
        const auto finde = [k](const std::vector<Teilton>& v) {
            for (const Teilton& t : v)
                if (t.k == k) return t;
            return Teilton{};
        };
        const Teilton t = kanalMittel(finde(links), finde(rechts));
        TeiltonKurven& z = kurven[static_cast<std::size_t>(k - 1)];
        if (t.bereichBis <= t.bereichVon) continue;
        z.bereichVon = t.bereichVon;
        z.bereichBis = t.bereichBis;
        z.kurve = schroederKurve(t, schrittS, t.bereichVon, t.bereichBis);
        z.kovarianz = autokovarianz(t, schrittS, t.bereichVon, t.bereichBis);
        z.erkannt = !z.kurve.db.empty();
    }
    return kurven;
}

/// Ein Ton in Stereo: die Blende am Ende einmal an der Summe beider Kanaele bestimmt und an beiden
/// gleich abgeschnitten (`endeVorBlende`), dann jeder Kanal gemessen. `kurven` (wenn gegeben) bekommt die
/// Kurven fuer das Setzen (7.5.2) — aus derselben Messung, die Werte aendern sich nicht.
inline std::array<KanalMessung, 2> messeTon(std::span<const float> stereo, int rate, double nennFrequenz,
                                            const lumi::loop::Fft& fft, bool mitPhantomen, TonKurven* kurven = nullptr)
{
    auto [links, rechts] = trenne(stereo);
    const double laengeS = static_cast<double>(links.size()) / rate;
    std::vector<double> summe(links.size());
    for (std::size_t i = 0; i < summe.size(); ++i) summe[i] = links[i] + rechts[i];
    const std::size_t ende = endeVorBlende(summe, rate);
    links.resize(ende);
    rechts.resize(ende);
    std::vector<Teilton> huellenL;
    std::vector<Teilton> huellenR;
    std::array<KanalMessung, 2> ton{messeKanal(links, rate, nennFrequenz, fft, mitPhantomen, &huellenL),
                                    messeKanal(rechts, rate, nennFrequenz, fft, mitPhantomen, &huellenR)};
    for (KanalMessung& k : ton)
    {
        k.endeS = static_cast<double>(ende) / rate;
        k.laengeS = laengeS;
    }
    if (kurven != nullptr) *kurven = kurvenAus(huellenL, huellenR, Einstellungen{}.schrittS);
    // HU2 a (S159): die Huellkurve vorwaerts und der ganze Ton aus dem Kanalmittel in der Energie (wie die Kurven, K1 a) —
    // in beiden Kanaelen derselbe Wert. Grund: an Salamander laufen die Kanaele auseinander (C4, Teilton 1: links faellt
    // der Grundton in den ersten 0,5 s mit 12 dB/s, rechts mit 23 dB/s — der linke Kanal ist dort 5 dB leiser und traegt
    // frueh mehr Nachklang); der Median ueber beide Kanaele laege dazwischen und beschriebe keinen von beiden.
    const Einstellungen einst;
    for (int k = 1; k <= kKurvenTeiltoene; ++k)
    {
        const auto finde = [k](const std::vector<Teilton>& h) -> const Teilton* {
            for (const Teilton& t : h)
                if (t.k == k) return &t;
            return nullptr;
        };
        const Teilton* l = finde(huellenL);
        const Teilton* r = finde(huellenR);
        if (l == nullptr || r == nullptr || l->pegelDb.empty() || r->pegelDb.empty()) continue;
        const std::size_t n = std::min(l->pegelDb.size(), r->pegelDb.size());
        std::vector<double> mitte(n);
        for (std::size_t i = 0; i < n; ++i)
            mitte[i] = 10.0 * std::log10(0.5 * (std::pow(10.0, l->pegelDb[i] / 10.0) + std::pow(10.0, r->pegelDb[i] / 10.0)));
        const std::size_t suche = std::min(n, static_cast<std::size_t>(0.5 / einst.schrittS) + 1);
        const auto hoechster = static_cast<std::size_t>(std::max_element(mitte.begin(), mitte.begin() + static_cast<std::ptrdiff_t>(suche)) -
                                                        mitte.begin());
        const std::size_t von = std::max(l->bereichVon, r->bereichVon);
        const std::size_t bis = std::min({l->bereichBis, r->bereichBis, n});
        const Abklingen a = bis > von && static_cast<double>(bis - von) * einst.schrittS >= 0.3 ? detail::zweiStufen(mitte, einst.schrittS, von, bis)
                                                                                                   : Abklingen{};
        const Vorwaerts v = detail::huellkurveVorwaerts(mitte, einst.schrittS, hoechster, von, bis, einst.vorwaertsS, a);
        for (KanalMessung& km : ton)
            for (TeiltonKurz& tk : km.teiltoene)
                if (tk.k == k) tk.vorwaerts = v;
    }
    std::vector<double> mono(ende);
    for (std::size_t i = 0; i < ende; ++i) mono[i] = std::sqrt(0.5 * (links[i] * links[i] + rechts[i] * rechts[i]));
    const std::array<double, 3> huelle = huellkurveZeiten(mono, rate);
    for (KanalMessung& km : ton) km.huelleS = huelle;
    return ton;
}

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

inline Wert medianWert(std::vector<double> werte)
{
    const auto anzahl = static_cast<int>(werte.size());
    if (anzahl == 0) return {};
    return {median(std::move(werte)), anzahl};
}

}  // namespace detail

/// Je Taste: `messungen[e][k]` ist Ebene e + 1 (v1 … v16), Kanal k (0 links, 1 rechts). Die Pegel
/// gegen das Mittel der Tastatur setzt `setzeRelativePegel` danach ueber alle Tasten.
inline TastenWerte verdichte(int midi, const std::vector<std::array<KanalMessung, 2>>& messungen)
{
    TastenWerte w;
    w.midi = midi;
    std::vector<double> b, frueh, spaet, knick, zwei, ausgleich, hoch, cent, tiefe, tiefeLeise, tiefeLaut, direkt;
    std::array<std::vector<double>, 2> fruehK, spaetK, zweiK;  // Teilton 2 und 3
    std::vector<double> vorw, knickT;                           // HU2 a: Teilton 1
    std::array<std::vector<double>, 2> vorwK, knickK;           // Teilton 2 und 3
    std::array<std::vector<double>, 3> huelle;                  // der ganze Ton
    for (std::size_t e = 0; e < messungen.size(); ++e)
        for (const KanalMessung& m : messungen[e])
        {
            if (!m.erkannt) continue;
            if (e + 1 >= 6)
            {
                for (int k = 1; k <= 3; ++k)
                    if (const TeiltonKurz* t = m.teilton(k); t != nullptr && t->vorwaerts.erkannt)
                    {
                        (k == 1 ? vorw : vorwK[static_cast<std::size_t>(k - 2)]).push_back(t->vorwaerts.t60);
                        if (t->vorwaerts.knickErkannt)
                            (k == 1 ? knickT : knickK[static_cast<std::size_t>(k - 2)]).push_back(t->vorwaerts.knickTiefeDb);
                    }
                for (std::size_t g = 0; g < 3; ++g)
                    if (m.huelleS[g] > 0.0) huelle[g].push_back(m.huelleS[g]);
            }
            if (m.teiltoene.size() >= 5) b.push_back(m.b);
            if (m.hochErkannt)
            {
                hoch.push_back(m.highDecay);
                const double w1 = 2.0 * std::numbers::pi * m.f0 * std::sqrt(1.0 + m.b);
                const double s1 = m.sigmaB1 + m.sigmaB3 * w1 * w1;
                if (s1 > 0.0) ausgleich.push_back(std::log(1000.0) / s1);
            }
            for (int k = 1; k <= 3; ++k)
                if (const TeiltonKurz* t = m.teilton(k); t != nullptr && t->schwebung.erkannt)
                    cent.push_back(1200.0 * std::log2(1.0 + t->schwebung.frequenz / t->frequenz));
            if (e + 1 >= 6)
                for (int k = 1; k <= 3; ++k)
                    if (const TeiltonKurz* t = m.teilton(k); t != nullptr && t->schwebung.erkannt)
                    {
                        tiefe.push_back(t->schwebung.einbruchDb);
                        if (e + 1 <= 10) tiefeLeise.push_back(t->schwebung.einbruchDb);
                        if (e + 1 >= 12) tiefeLaut.push_back(t->schwebung.einbruchDb);
                    }
            if (e + 1 >= 6)
                for (int k = 2; k <= 3; ++k)
                    if (const TeiltonKurz* t = m.teilton(k); t != nullptr && t->abklingen.erkannt)
                    {
                        const auto j = static_cast<std::size_t>(k - 2);
                        fruehK[j].push_back(t->abklingen.t60Frueh);
                        zweiK[j].push_back(t->abklingen.zweiStufen ? 1.0 : 0.0);
                        if (t->abklingen.zweiStufen) spaetK[j].push_back(t->abklingen.t60Spaet);
                    }
            if (e + 1 >= 6)
                if (const TeiltonKurz* t = m.teilton(1); t != nullptr && t->abklingen.erkannt)
                {
                    frueh.push_back(t->abklingen.t60Frueh);
                    zwei.push_back(t->abklingen.zweiStufen ? 1.0 : 0.0);
                    if (t->abklingen.zweiStufen)
                    {
                        spaet.push_back(t->abklingen.t60Spaet);
                        knick.push_back(t->abklingen.knickS);
                    }
                }
            if (e + 1 == kEbenen) direkt.push_back(m.direktDb);
        }
    w.b = detail::medianWert(b);
    w.t60Frueh = detail::medianWert(frueh);
    w.t60Spaet = detail::medianWert(spaet);
    w.knickS = detail::medianWert(knick);
    if (!zwei.empty())
    {
        double s = 0.0;
        for (const double z : zwei) s += z;
        w.zweiStufenAnteil = {s / static_cast<double>(zwei.size()), static_cast<int>(zwei.size())};
    }
    for (std::size_t j = 0; j < 2; ++j)
    {
        w.t60FruehK[j] = detail::medianWert(fruehK[j]);
        w.t60SpaetK[j] = detail::medianWert(spaetK[j]);
        if (!zweiK[j].empty())
        {
            double s = 0.0;
            for (const double z : zweiK[j]) s += z;
            w.zweiStufenAnteilK[j] = {s / static_cast<double>(zweiK[j].size()), static_cast<int>(zweiK[j].size())};
        }
    }
    w.t60Ausgleich = detail::medianWert(ausgleich);
    w.highDecay = detail::medianWert(hoch);
    w.vorwaertsT60 = detail::medianWert(vorw);
    w.knickTiefe = detail::medianWert(knickT);
    for (std::size_t j = 0; j < 2; ++j)
    {
        w.vorwaertsT60K[j] = detail::medianWert(vorwK[j]);
        w.knickTiefeK[j] = detail::medianWert(knickK[j]);
    }
    for (std::size_t g = 0; g < 3; ++g) w.huelle[g] = detail::medianWert(huelle[g]);
    w.schwebungCent = detail::medianWert(cent);
    w.tiefeDb = detail::medianWert(tiefe);
    w.tiefeLeise = detail::medianWert(tiefeLeise);
    w.tiefeLaut = detail::medianWert(tiefeLaut);
    w.direkt = detail::medianWert(direkt);

    // Helligkeit (V1 a): je Kanal die Steigung von (Teilton n − Teilton 1) ueber Teilton 1, dann Median
    for (int n = 2; n <= 8; ++n)
    {
        std::vector<double> steigungen;
        for (std::size_t kanal = 0; kanal < 2; ++kanal)
        {
            std::vector<double> x, y;
            for (const auto& ebene : messungen)
            {
                if (!ebene[kanal].erkannt) continue;
                const TeiltonKurz* t1 = ebene[kanal].teilton(1);
                const TeiltonKurz* tn = ebene[kanal].teilton(n);
                if (t1 == nullptr || tn == nullptr) continue;
                x.push_back(t1->anfangDb);
                y.push_back(tn->anfangDb - t1->anfangDb);
            }
            if (x.size() < 6) continue;
            const detail::Geraden geraden(x, y);
            steigungen.push_back(geraden.ueber(0, x.size()).steigung);
        }
        w.helligkeit.push_back(detail::medianWert(steigungen));
    }

    // Pegelumfang: der Pegel des ganzen Tons zu Beginn, v16 gegen v1, Median der Kanaele (Teilton 1 ist im
    // Diskant leise kaum messbar — Befund S152: am gerechneten C8 gab er 110 dB statt 51)
    if (messungen.size() == kEbenen)
    {
        std::vector<double> umfang;
        for (std::size_t kanal = 0; kanal < 2; ++kanal)
            if (messungen.front()[kanal].erkannt && messungen.back()[kanal].erkannt)
                umfang.push_back(messungen.back()[kanal].pegelDb - messungen.front()[kanal].pegelDb);
        w.pegelumfang = detail::medianWert(umfang);
        w.terzL = messungen[11][0].terzDb;
        w.terzR = messungen[11][1].terzDb;
        std::map<int, std::vector<double>> je;
        for (const KanalMessung& m : messungen.back())
            for (const Phantom& p : m.phantome) je[p.n].push_back(p.abstandDb);
        for (auto& [n, werte] : je)
            if (werte.size() == 2) w.phantome.emplace_back(n, 0.5 * (werte[0] + werte[1]));
    }
    return w;
}

/// Die Pegel zu Beginn je Taste und Kanal gegen das Mittel der Tastatur (UF6 a): je Ebene der
/// Abstand zum Mittel ueber die Tasten, dann der Median ueber die Ebenen.
inline void setzeRelativePegel(std::vector<TastenWerte>& werte,
                               const std::vector<std::vector<std::array<KanalMessung, 2>>>& messungen)
{
    for (std::size_t kanal = 0; kanal < 2; ++kanal)
    {
        std::vector<std::vector<double>> abstaende(werte.size());
        for (std::size_t e = 0; e < static_cast<std::size_t>(kEbenen); ++e)
        {
            double summe = 0.0;
            int zahl = 0;
            for (const auto& taste : messungen)
                if (e < taste.size() && taste[e][kanal].erkannt)
                {
                    summe += taste[e][kanal].pegelDb;
                    ++zahl;
                }
            if (zahl == 0) continue;
            const double mittel = summe / zahl;
            for (std::size_t t = 0; t < messungen.size(); ++t)
                if (e < messungen[t].size() && messungen[t][e][kanal].erkannt)
                    abstaende[t].push_back(messungen[t][e][kanal].pegelDb - mittel);
        }
        for (std::size_t t = 0; t < werte.size(); ++t)
            (kanal == 0 ? werte[t].pegelL : werte[t].pegelR) = detail::medianWert(abstaende[t]);
    }
}

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

inline void schreibeZeile(std::ostream& aus, int midi, const char* groesse, const Wert& w)
{
    aus << midi << '\t' << groesse << '\t' << w.wert << '\t' << w.anzahl << '\n';
}

/// Kopf fuer dieselbe Messung am Modell (31d-3 Teil 2): die Stufen −30 … 0 dB in Schritten von 2 stehen
/// an der Stelle der Ebenen v1 … v16 (V1 a).
inline const char* const kModellKopf =
    "# Gemessen am gerechneten Steinway D: Hammer-Saite mit Boden (lib:steinway_d, Hoerorte left/right), wie im\n"
    "# Programm (rendereTon, 48 kHz); Anschlagstufen -30 ... 0 dB in Schritten von 2 an der Stelle der Ebenen\n"
    "# v1 ... v16 (V1 a). Geschrieben von test_LoopSalamander.cpp (LUMIVIZ_31D3_MODELL=<Ordner>).\n"
    "# Zeile: Taste <Tab> Groesse <Tab> Wert <Tab> Zahl der Messungen (bei Listen: die Werte)\n";

inline bool schreibeWerte(const std::filesystem::path& pfad, const std::vector<TastenWerte>& werte, const char* kopf = kSalamanderKopf)
{
    std::ofstream aus(pfad, std::ios::binary);
    if (!aus) return false;
    aus.precision(6);
    aus << kopf;
    for (const TastenWerte& w : werte)
    {
        schreibeZeile(aus, w.midi, "B", w.b);
        schreibeZeile(aus, w.midi, "t60_frueh", w.t60Frueh);
        schreibeZeile(aus, w.midi, "t60_spaet", w.t60Spaet);
        schreibeZeile(aus, w.midi, "knick_s", w.knickS);
        schreibeZeile(aus, w.midi, "zwei_stufen", w.zweiStufenAnteil);
        for (std::size_t j = 0; j < 2; ++j)
        {
            const std::string k = std::to_string(j + 2);
            schreibeZeile(aus, w.midi, ("t60_frueh_" + k).c_str(), w.t60FruehK[j]);
            schreibeZeile(aus, w.midi, ("t60_spaet_" + k).c_str(), w.t60SpaetK[j]);
            schreibeZeile(aus, w.midi, ("zwei_stufen_" + k).c_str(), w.zweiStufenAnteilK[j]);
        }
        schreibeZeile(aus, w.midi, "t60_ausgleich", w.t60Ausgleich);
        schreibeZeile(aus, w.midi, "high_decay", w.highDecay);
        schreibeZeile(aus, w.midi, "schwebung_cent", w.schwebungCent);
        schreibeZeile(aus, w.midi, "tiefe_db", w.tiefeDb);
        schreibeZeile(aus, w.midi, "tiefe_leise_db", w.tiefeLeise);
        schreibeZeile(aus, w.midi, "tiefe_laut_db", w.tiefeLaut);
        schreibeZeile(aus, w.midi, "vorwaerts_t60", w.vorwaertsT60);
        schreibeZeile(aus, w.midi, "knick_tiefe_db", w.knickTiefe);
        for (std::size_t j = 0; j < 2; ++j)
        {
            const std::string k = std::to_string(j + 2);
            schreibeZeile(aus, w.midi, ("vorwaerts_t60_" + k).c_str(), w.vorwaertsT60K[j]);
            schreibeZeile(aus, w.midi, ("knick_tiefe_db_" + k).c_str(), w.knickTiefeK[j]);
        }
        schreibeZeile(aus, w.midi, "huelle_20_s", w.huelle[0]);
        schreibeZeile(aus, w.midi, "huelle_30_s", w.huelle[1]);
        schreibeZeile(aus, w.midi, "huelle_40_s", w.huelle[2]);
        for (std::size_t i = 0; i < w.helligkeit.size(); ++i)
            schreibeZeile(aus, w.midi, ("helligkeit_" + std::to_string(i + 2)).c_str(), w.helligkeit[i]);
        schreibeZeile(aus, w.midi, "pegelumfang", w.pegelumfang);
        schreibeZeile(aus, w.midi, "pegel_links", w.pegelL);
        schreibeZeile(aus, w.midi, "pegel_rechts", w.pegelR);
        schreibeZeile(aus, w.midi, "direktschall", w.direkt);
        for (const auto& [name, terz] : {std::pair{"terz_links", &w.terzL}, std::pair{"terz_rechts", &w.terzR}})
        {
            aus << w.midi << '\t' << name;
            for (const double v : *terz) aus << '\t' << v;
            aus << '\n';
        }
        aus << w.midi << "\tphantome";
        for (const auto& [n, db] : w.phantome) aus << '\t' << n << ':' << db;
        aus << '\n';
    }
    return static_cast<bool>(aus);
}

/// Gelesen: je Taste je Groesse die Zahlen der Zeile (bei Einzelwerten Wert und Anzahl).
using SalamanderWerte = std::map<int, std::map<std::string, std::vector<std::string>>>;

inline std::optional<SalamanderWerte> liesWerte(const std::filesystem::path& pfad)
{
    std::ifstream ein(pfad, std::ios::binary);
    if (!ein) return std::nullopt;
    SalamanderWerte werte;
    std::string zeile;
    while (std::getline(ein, zeile))
    {
        if (!zeile.empty() && zeile.back() == '\r') zeile.pop_back();
        if (zeile.empty() || zeile.front() == '#') continue;
        std::vector<std::string> teile;
        std::stringstream s(zeile);
        std::string teil;
        while (std::getline(s, teil, '\t')) teile.push_back(teil);
        if (teile.size() < 2) return std::nullopt;
        werte[std::stoi(teile[0])][teile[1]] = std::vector<std::string>(teile.begin() + 2, teile.end());
    }
    return werte;
}

/// Die gelesene Datei zurueck in `TastenWerte` (fuer den Vergleich mit dem Modell und fuer 31d-4).
inline std::vector<TastenWerte> tastenWerteAus(const SalamanderWerte& gelesen)
{
    std::vector<TastenWerte> liste;
    for (const auto& [midi, zeilen] : gelesen)
    {
        TastenWerte w;
        w.midi = midi;
        const auto wert = [&zeilen](const std::string& name) -> Wert {
            const auto it = zeilen.find(name);
            if (it == zeilen.end() || it->second.size() < 2) return {};
            return {std::stod(it->second[0]), std::stoi(it->second[1])};
        };
        const auto zahlen = [&zeilen](const std::string& name) {
            std::vector<double> z;
            if (const auto it = zeilen.find(name); it != zeilen.end())
                for (const std::string& s : it->second) z.push_back(std::stod(s));
            return z;
        };
        w.b = wert("B");
        w.t60Frueh = wert("t60_frueh");
        w.t60Spaet = wert("t60_spaet");
        w.knickS = wert("knick_s");
        w.zweiStufenAnteil = wert("zwei_stufen");
        for (std::size_t j = 0; j < 2; ++j)
        {
            const std::string k = std::to_string(j + 2);
            w.t60FruehK[j] = wert("t60_frueh_" + k);
            w.t60SpaetK[j] = wert("t60_spaet_" + k);
            w.zweiStufenAnteilK[j] = wert("zwei_stufen_" + k);
        }
        w.t60Ausgleich = wert("t60_ausgleich");
        w.highDecay = wert("high_decay");
        w.schwebungCent = wert("schwebung_cent");
        w.tiefeDb = wert("tiefe_db");
        w.tiefeLeise = wert("tiefe_leise_db");
        w.tiefeLaut = wert("tiefe_laut_db");
        w.vorwaertsT60 = wert("vorwaerts_t60");
        w.knickTiefe = wert("knick_tiefe_db");
        for (std::size_t j = 0; j < 2; ++j)
        {
            const std::string k = std::to_string(j + 2);
            w.vorwaertsT60K[j] = wert("vorwaerts_t60_" + k);
            w.knickTiefeK[j] = wert("knick_tiefe_db_" + k);
        }
        w.huelle[0] = wert("huelle_20_s");
        w.huelle[1] = wert("huelle_30_s");
        w.huelle[2] = wert("huelle_40_s");
        for (int n = 2; n <= 8; ++n) w.helligkeit.push_back(wert("helligkeit_" + std::to_string(n)));
        w.pegelumfang = wert("pegelumfang");
        w.pegelL = wert("pegel_links");
        w.pegelR = wert("pegel_rechts");
        w.direkt = wert("direktschall");
        w.terzL = zahlen("terz_links");
        w.terzR = zahlen("terz_rechts");
        if (const auto it = zeilen.find("phantome"); it != zeilen.end())
            for (const std::string& s : it->second)
                if (const auto doppelpunkt = s.find(':'); doppelpunkt != std::string::npos)
                    w.phantome.emplace_back(std::stoi(s.substr(0, doppelpunkt)), std::stod(s.substr(doppelpunkt + 1)));
        liste.push_back(std::move(w));
    }
    return liste;
}

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

inline bool schreibeKurven(const std::filesystem::path& pfad, const KurvenBezug& bezug)
{
    std::ofstream aus(pfad, std::ios::binary);
    if (!aus) return false;
    aus << kKurvenKopf;
    for (const auto& [s, z] : bezug)
    {
        if (!z.erkannt) continue;
        aus << s.midi << '\t' << s.ebene << '\t' << s.k << "\tschroeder\t" << z.bereichVon << '\t' << z.bereichBis << '\t'
            << z.kurve.ersterPunkt << '\t' << z.kurve.gueltig;
        aus << std::fixed << std::setprecision(2);
        for (const double v : z.kurve.db) aus << '\t' << v;
        aus << '\n' << std::defaultfloat << std::setprecision(4);
        aus << s.midi << '\t' << s.ebene << '\t' << s.k << "\tkovarianz";
        for (const double v : z.kovarianz.db2) aus << '\t' << v;
        aus << '\n' << std::defaultfloat;
    }
    return static_cast<bool>(aus);
}

/// Gelesen wie geschrieben; Raster nach K5 a (0,05 s und 0,02 s).
inline std::optional<KurvenBezug> liesKurven(const std::filesystem::path& pfad)
{
    std::ifstream ein(pfad, std::ios::binary);
    if (!ein) return std::nullopt;
    KurvenBezug bezug;
    std::string zeile;
    while (std::getline(ein, zeile))
    {
        if (!zeile.empty() && zeile.back() == '\r') zeile.pop_back();
        if (zeile.empty() || zeile.front() == '#') continue;
        std::vector<std::string> teile;
        std::stringstream s(zeile);
        std::string teil;
        while (std::getline(s, teil, '\t')) teile.push_back(teil);
        if (teile.size() < 4) return std::nullopt;
        TeiltonKurven& z = bezug[{std::stoi(teile[0]), std::stoi(teile[1]), std::stoi(teile[2])}];
        if (teile[3] == "schroeder")
        {
            if (teile.size() < 8) return std::nullopt;
            z.erkannt = true;
            z.bereichVon = std::stoul(teile[4]);
            z.bereichBis = std::stoul(teile[5]);
            z.kurve.rasterS = 0.05;
            z.kurve.ersterPunkt = std::stoul(teile[6]);
            z.kurve.gueltig = std::stoul(teile[7]);
            for (std::size_t i = 8; i < teile.size(); ++i) z.kurve.db.push_back(std::stod(teile[i]));
        }
        else if (teile[3] == "kovarianz")
        {
            z.kovarianz.rasterS = 0.02;
            for (std::size_t i = 4; i < teile.size(); ++i) z.kovarianz.db2.push_back(std::stod(teile[i]));
        }
        else
            return std::nullopt;
    }
    return bezug;
}

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

inline BodenVergleich vergleicheBoden(const std::vector<int>& tasten, const std::vector<std::vector<double>>& a,
                                      const std::vector<std::vector<double>>& b, double umfangDb = 60.0)
{
    const std::vector<double> mitten = terzMitten();
    const auto lautestes = [](const std::vector<std::vector<double>>& x, std::size_t t) {
        return t < x.size() && !x[t].empty() ? *std::max_element(x[t].begin(), x[t].end()) : 0.0;
    };
    const auto gilt = [&](std::size_t t, std::size_t band) {
        return t < a.size() && t < b.size() && band < a[t].size() && band < b[t].size() &&
               mitten[band] >= 0.8 * tastenFrequenz(tasten[t]) && a[t][band] >= lautestes(a, t) - umfangDb &&
               b[t][band] >= lautestes(b, t) - umfangDb;
    };
    BodenVergleich v;
    v.mittelDb.assign(mitten.size(), -300.0);
    for (std::size_t band = 0; band < mitten.size(); ++band)
    {
        double summe = 0.0;
        int zahl = 0;
        for (std::size_t t = 0; t < tasten.size(); ++t)
            if (gilt(t, band))
            {
                summe += a[t][band] - b[t][band];
                ++zahl;
            }
        if (zahl > 0) v.mittelDb[band] = summe / zahl;
    }
    for (std::size_t t = 0; t < tasten.size(); ++t)
    {
        double summe = 0.0;
        int zahl = 0;
        for (std::size_t band = 0; band < mitten.size(); ++band)
            if (gilt(t, band))
            {
                const double d = a[t][band] - b[t][band] - v.mittelDb[band];
                summe += d * d;
                ++zahl;
            }
        v.abweichungDb.push_back(zahl > 0 ? std::sqrt(summe / zahl) : 0.0);
    }
    return v;
}

}  // namespace lumi::messung
