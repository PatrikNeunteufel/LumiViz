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

#include "Fluegelmessung.hpp"

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
inline double schwebungBisHz(double nennFrequenz, const Einstellungen& einst = {})
{
    const double fensterS = std::clamp(einst.fensterPerioden / nennFrequenz, 0.02, 0.5);
    return std::min(einst.schwebungBisHz, 0.5 / fensterS);
}

/// Die Struktur eines Teiltons an einer Ebene aus dem Kanalmittel, mit der Erkennung von `miss` (zwei Stufen nach
/// Schroeder, Schwebung im Rest um die Stufen) — nur fuer das Ziel (ZF3 a).
inline TeiltonStruktur strukturAus(const Teilton& m, double nennFrequenz, const Einstellungen& einst = {})
{
    TeiltonStruktur s;
    if (m.bereichBis <= m.bereichVon || m.bereichBis > m.pegelDb.size()) return s;
    if (static_cast<double>(m.bereichBis - m.bereichVon) * einst.schrittS < 0.3) return s;
    s.da = true;
    s.von = m.bereichVon;
    s.bis = m.bereichBis;
    s.ebenen = 1;
    const Abklingen a = detail::zweiStufen(m.pegelDb, einst.schrittS, s.von, s.bis);
    if (!a.erkannt) return s;
    if (a.zweiStufen)
    {
        s.knick = true;
        s.knickS = a.knickS;
        s.mitKnick = 1;
    }
    const std::vector<double> rest = detail::restUmStufen(m.pegelDb, einst.schrittS, s.von, s.bis, a);
    const Schwebung sch = detail::findeSchwebung(rest, einst.schrittS, schwebungBisHz(nennFrequenz, einst));
    if (sch.erkannt)
    {
        s.schwebt = true;
        s.schwebungHz = sch.frequenz;
        s.mitSchwebung = 1;
    }
    return s;
}

/// Die Struktur ueber die Ebenen: Bereich als Median; Knick und Schwebung, wo mindestens die Haelfte der Ebenen mit
/// Bereich sie erkennt (Median ueber diese); sonst eine Gerade bzw. keine Schwebung (ZF3 a, ZF7 a).
inline TeiltonStruktur verdichteStruktur(const std::vector<TeiltonStruktur>& ebenen)
{
    std::vector<double> von, bis, knick, hz;
    int da = 0, mitKnick = 0, mitSchwebung = 0;
    for (const TeiltonStruktur& e : ebenen)
    {
        if (!e.da) continue;
        ++da;
        von.push_back(static_cast<double>(e.von));
        bis.push_back(static_cast<double>(e.bis));
        if (e.knick)
        {
            ++mitKnick;
            knick.push_back(e.knickS);
        }
        if (e.schwebt)
        {
            ++mitSchwebung;
            hz.push_back(e.schwebungHz);
        }
    }
    TeiltonStruktur s;
    if (da == 0) return s;
    s.da = true;
    s.ebenen = da;
    s.mitKnick = mitKnick;
    s.mitSchwebung = mitSchwebung;
    s.von = static_cast<std::size_t>(std::lround(detail::median(std::move(von))));
    s.bis = static_cast<std::size_t>(std::lround(detail::median(std::move(bis))));
    if (2 * mitKnick >= da)
    {
        s.knick = true;
        s.knickS = detail::median(std::move(knick));
    }
    if (2 * mitSchwebung >= da)
    {
        s.schwebt = true;
        s.schwebungHz = detail::median(std::move(hz));
    }
    return s;
}

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
inline StrukturWerte misstAnStruktur(const Teilton& t, const TeiltonStruktur& s, const Einstellungen& einst = {}, double randS = 1.0)
{
    StrukturWerte w;
    if (!s.da || t.pegelDb.empty()) return w;
    const double schritt = einst.schrittS;
    const std::size_t von = s.von;
    const std::size_t bis = std::min(s.bis, t.pegelDb.size());
    if (bis <= von + 10) return w;
    w.gemessen = true;
    // die Spitze wie `messeTon`: in den ersten 0,5 s
    const std::size_t suche = std::min(t.pegelDb.size(), static_cast<std::size_t>(0.5 / schritt) + 1);
    const auto hoechster =
        static_cast<std::size_t>(std::max_element(t.pegelDb.begin(), t.pegelDb.begin() + static_cast<std::ptrdiff_t>(suche)) - t.pegelDb.begin());
    const auto knickIdx = s.knick ? static_cast<std::size_t>(std::lround(s.knickS / schritt)) : von;
    const bool knickInnen = s.knick && knickIdx > von && knickIdx + 10 < bis;
    Abklingen a;
    a.erkannt = true;
    a.zweiStufen = knickInnen;
    a.knickS = s.knickS;
    a.vonS = static_cast<double>(von) * schritt;
    a.bisS = static_cast<double>(bis - 1) * schritt;
    w.vorwaerts = detail::huellkurveVorwaerts(t.pegelDb, schritt, hoechster, von, bis, einst.vorwaertsS, a);

    // die spaete Stufe: die Kurve nach Schroeder ab dem Knick (ohne Knick ab `von`), eine Gerade bis `randS` vor dem Ende
    const std::size_t start = knickInnen ? knickIdx : von;
    {
        const std::size_t n = bis - start;
        std::vector<double> E(n + 1, 0.0);
        for (std::size_t j = n; j-- > 0;) E[j] = E[j + 1] + std::pow(10.0, t.pegelDb[start + j] / 10.0);
        if (E[0] > 0.0)
        {
            const auto rand = static_cast<std::size_t>(std::lround(randS / schritt));
            const std::size_t ende = n > rand + 10 ? n - rand : n;
            std::vector<double> zt(ende), zl(ende);
            for (std::size_t j = 0; j < ende; ++j)
            {
                zt[j] = static_cast<double>(start + j) * schritt;
                zl[j] = 10.0 * std::log10(std::max(E[j], 1e-300) / E[0]);
            }
            const detail::Geraden::Gerade g = detail::Geraden(zt, zl).ueber(0, ende);
            if (g.steigung < 0.0)
            {
                w.spaet = true;
                w.t60Spaet = -60.0 / g.steigung;
            }
        }
    }

    // die Schwebung bei der Frequenz der Struktur
    if (s.schwebt && s.schwebungHz > 0.0)
    {
        const std::vector<double> rest = detail::restUmStufen(t.pegelDb, schritt, von, bis, a);
        if (rest.size() >= 20)
        {
            std::complex<double> summe{};
            const std::complex<double> dreh = std::polar(1.0, -2.0 * std::numbers::pi * s.schwebungHz * schritt);
            std::complex<double> zeiger{1.0, 0.0};
            for (const double r : rest)
            {
                summe += r * zeiger;
                zeiger *= dreh;
            }
            w.schwebung = true;
            w.schwebungDb = 2.0 * (2.0 * std::abs(summe) / static_cast<double>(rest.size()));
            const double periode = 1.0 / (s.schwebungHz * schritt);
            std::vector<double> tiefen;
            for (std::size_t i = 0;; ++i)
            {
                const auto anfang = static_cast<std::size_t>(std::lround(static_cast<double>(i) * periode));
                const auto ende = static_cast<std::size_t>(std::lround(static_cast<double>(i + 1) * periode));
                if (ende > rest.size() || ende <= anfang) break;
                const auto [unten, oben] =
                    std::minmax_element(rest.begin() + static_cast<std::ptrdiff_t>(anfang), rest.begin() + static_cast<std::ptrdiff_t>(ende));
                tiefen.push_back(*oben - *unten);
            }
            if (!tiefen.empty()) w.einbruchDb = detail::median(std::move(tiefen));
        }
    }
    return w;
}

/// Die Groessen an der Struktur je Ebene (ZF2 a mit HU2 a): je Teilton 1–3 `vorwaerts_t60[_k]` (Faktor),
/// `knick_tiefe_db[_k]` (dB), `t60_spaet[_k]` (Faktor), `schwebung_db_k` (dB, Spitze zu Spitze bei der Frequenz der
/// Struktur), `tiefe_db_k` (dB). `huellen[e]` sind die Huellen der Ebene e (ab `kKurvenEbeneAb`, aus `messeTon` mit
/// `huellen`), `struktur` die des Ziels. Der ganze Ton (`huelle_*_s`) kommt nicht von hier (`erkennungsGroessen`).
inline std::vector<GroesseJeEbene> strukturGroessen(const std::vector<TonHuellen>& huellen, const TonStruktur& struktur,
                                                    const Einstellungen& einst = {})
{
    std::vector<GroesseJeEbene> alle;
    for (int k = 1; k <= kKurvenTeiltoene; ++k)
    {
        const std::string n = k == 1 ? "" : "_" + std::to_string(k);
        const std::string kk = "_" + std::to_string(k);
        GroesseJeEbene vorw{"vorwaerts_t60" + n, Skala::Faktor, {}};
        GroesseJeEbene knick{"knick_tiefe_db" + n, Skala::Dezibel, {}};
        GroesseJeEbene spaet{"t60_spaet" + n, Skala::Faktor, {}};
        GroesseJeEbene schweb{"schwebung_db" + kk, Skala::Dezibel, {}};
        GroesseJeEbene tiefe{"tiefe_db" + kk, Skala::Dezibel, {}};
        const TeiltonStruktur& s = struktur[static_cast<std::size_t>(k - 1)];
        for (const TonHuellen& h : huellen)
        {
            const std::array<const std::vector<Teilton>*, 3> seiten{&h.mittel, &h.links, &h.rechts};
            std::array<StrukturWerte, 3> w;
            for (std::size_t i = 0; i < 3; ++i)
                if (const Teilton* t = teiltonK(*seiten[i], k); t != nullptr) w[i] = misstAnStruktur(*t, s, einst);
            const auto wert = [&w](auto&& aus) {
                EbenenWerte e;
                e.mittel = aus(w[0]);
                e.kanaele[0] = aus(w[1]);
                e.kanaele[1] = aus(w[2]);
                return e;
            };
            vorw.ebenen.push_back(wert([](const StrukturWerte& x) {
                return x.gemessen && x.vorwaerts.erkannt && x.vorwaerts.t60 > 0.0 ? std::optional<double>(x.vorwaerts.t60) : std::nullopt;
            }));
            knick.ebenen.push_back(wert([](const StrukturWerte& x) {
                return x.gemessen && x.vorwaerts.knickErkannt ? std::optional<double>(x.vorwaerts.knickTiefeDb) : std::nullopt;
            }));
            spaet.ebenen.push_back(
                wert([](const StrukturWerte& x) { return x.gemessen && x.spaet && x.t60Spaet > 0.0 ? std::optional<double>(x.t60Spaet) : std::nullopt; }));
            schweb.ebenen.push_back(
                wert([](const StrukturWerte& x) { return x.gemessen && x.schwebung ? std::optional<double>(x.schwebungDb) : std::nullopt; }));
            tiefe.ebenen.push_back(wert(
                [](const StrukturWerte& x) { return x.gemessen && x.schwebung && x.einbruchDb > 0.0 ? std::optional<double>(x.einbruchDb) : std::nullopt; }));
        }
        for (GroesseJeEbene* g : {&vorw, &knick, &spaet, &schweb, &tiefe}) alle.push_back(std::move(*g));
    }
    return alle;
}

/// Der ganze Ton bis −20/−30/−40 dB je Ebene (`huelle_20/30/40_s`, Faktor) aus den Kanalmessungen wie `erkennungsGroessen`.
inline std::vector<GroesseJeEbene> huellenGroessen(const std::vector<std::array<KanalMessung, 2>>& messungen,
                                                   const std::vector<std::array<KanalMessung, 2>>& einzeln = {})
{
    std::vector<GroesseJeEbene> alle;
    for (const GroesseJeEbene& g : erkennungsGroessen(messungen, einzeln))
        if (g.name.rfind("huelle_", 0) == 0) alle.push_back(g);
    return alle;
}

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

inline bool schreibeStruktur(const std::filesystem::path& pfad, const Strukturen& strukturen)
{
    std::ofstream aus(pfad, std::ios::binary);
    if (!aus) return false;
    aus.precision(6);
    aus << kStrukturKopf;
    for (const auto& [midi, ton] : strukturen)
        for (std::size_t i = 0; i < ton.size(); ++i)
        {
            const TeiltonStruktur& s = ton[i];
            if (!s.da) continue;
            aus << midi << '\t' << i + 1 << '\t' << s.von << '\t' << s.bis << '\t' << (s.knick ? s.knickS : 0.0) << '\t'
                << (s.schwebt ? s.schwebungHz : 0.0) << '\t' << s.ebenen << '\t' << s.mitKnick << '\t' << s.mitSchwebung << '\n';
        }
    return static_cast<bool>(aus);
}

inline std::optional<Strukturen> liesStruktur(const std::filesystem::path& pfad)
{
    std::ifstream ein(pfad, std::ios::binary);
    if (!ein) return std::nullopt;
    Strukturen alle;
    std::string zeile;
    while (std::getline(ein, zeile))
    {
        if (!zeile.empty() && zeile.back() == '\r') zeile.pop_back();
        if (zeile.empty() || zeile.front() == '#') continue;
        std::vector<std::string> teile;
        std::stringstream s(zeile);
        std::string teil;
        while (std::getline(s, teil, '\t')) teile.push_back(teil);
        if (teile.size() != 9) return std::nullopt;
        const int k = std::stoi(teile[1]);
        if (k < 1 || k > kKurvenTeiltoene) return std::nullopt;
        TeiltonStruktur& t = alle[std::stoi(teile[0])][static_cast<std::size_t>(k - 1)];
        t.da = true;
        t.von = std::stoul(teile[2]);
        t.bis = std::stoul(teile[3]);
        t.knickS = std::stod(teile[4]);
        t.knick = t.knickS > 0.0;
        t.schwebungHz = std::stod(teile[5]);
        t.schwebt = t.schwebungHz > 0.0;
        t.ebenen = std::stoi(teile[6]);
        t.mitKnick = std::stoi(teile[7]);
        t.mitSchwebung = std::stoi(teile[8]);
    }
    return alle;
}

}  // namespace lumi::messung
