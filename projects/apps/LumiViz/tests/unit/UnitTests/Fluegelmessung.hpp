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
#include <fstream>
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

    [[nodiscard]] const TeiltonKurz* teilton(int k) const
    {
        for (const TeiltonKurz& t : teiltoene)
            if (t.k == k) return &t;
        return nullptr;
    }
};

/// Einen Kanal messen. `mitPhantomen` nur, wo sie gebraucht werden (Bass, laut — es kostet ein
/// langes Spektrum).
inline KanalMessung messeKanal(std::span<const double> ton, int rate, double nennFrequenz, const lumi::loop::Fft& fft,
                               bool mitPhantomen)
{
    KanalMessung m;
    const Ergebnis e = miss(ton, rate, nennFrequenz, fft);
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
    for (const Teilton& t : e.teiltoene) m.teiltoene.push_back({t.k, t.frequenz, t.anfangDb, t.abklingen, t.schwebung});
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

/// Ein Ton in Stereo: die Blende am Ende einmal an der Summe beider Kanaele bestimmt und an beiden
/// gleich abgeschnitten (`endeVorBlende`), dann jeder Kanal gemessen.
inline std::array<KanalMessung, 2> messeTon(std::span<const float> stereo, int rate, double nennFrequenz,
                                            const lumi::loop::Fft& fft, bool mitPhantomen)
{
    auto [links, rechts] = trenne(stereo);
    const double laengeS = static_cast<double>(links.size()) / rate;
    std::vector<double> summe(links.size());
    for (std::size_t i = 0; i < summe.size(); ++i) summe[i] = links[i] + rechts[i];
    const std::size_t ende = endeVorBlende(summe, rate);
    links.resize(ende);
    rechts.resize(ende);
    std::array<KanalMessung, 2> ton{messeKanal(links, rate, nennFrequenz, fft, mitPhantomen),
                                    messeKanal(rechts, rate, nennFrequenz, fft, mitPhantomen)};
    for (KanalMessung& k : ton)
    {
        k.endeS = static_cast<double>(ende) / rate;
        k.laengeS = laengeS;
    }
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
    for (std::size_t e = 0; e < messungen.size(); ++e)
        for (const KanalMessung& m : messungen[e])
        {
            if (!m.erkannt) continue;
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
    "# Zeile: Taste <Tab> Groesse <Tab> Wert <Tab> Zahl der Messungen (bei Listen: die Werte)\n";

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
