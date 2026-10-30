/**
 ****************************************************************************************
 * @file   Suchkurven.hpp
 * @brief  7.5.9 (SM4 b): die Kurven der Suche — neun Groessen der Hammer-Saite mit je einem Stuetzpunkt an den
 *         gemessenen Tasten, ihre Grenzen und Startpunkte (docs/audio/Spielweisen_Konzept.md §4.1; Entscheide Patrik
 *         S155/S157/S161: SQ2 a, ZF6 a, SP2 a, HU2 a (3), SG2 a, SG3 a)
 *
 * @details
 * Gesucht wird an den Stuetzpunkten (`Tastenkurve::punkte`), nicht an Abweichungen je Taste (SP2 a). Jede Kurve traegt
 * an jeder der 30 Tasten von *Salamander* einen Stuetzpunkt: frei an den Tasten, an denen sie wirkt (`detune`, `u leise`,
 * `u laut` nur im Chor ab zwei Saiten, `delta` nur bei einer Saite), fest mit dem Wert der Vorgabe an den uebrigen —
 * so bleibt die Kurve dort, wie die Vorgabe sie interpoliert (Lesart Claude S162: die Vorgabe an den gemessenen Tasten
 * als feste Stuetzpunkte; zwischen den gemessenen Tasten interpoliert die Kurve wie jede andere). Die Koordinate der
 * Suche ist ln(Wert) bei den Zeiten und `highDecay`, sonst der Wert; die Grenzen wirken durch Abschneiden (`klemme`).
 * Weil die Interpolation am Stuetzpunkt den Stuetzpunkt selbst liefert, wirkt der Stuetzpunkt einer Taste nur auf den
 * Ton dieser Taste (die Grundlage der Jacobi-Matrix je Kurve in 7.5.10). Mit `deltaImChor` ist δ an allen Tasten frei
 * (Probe 7.5.11, S163: im Chor steht δ sonst fest auf 0, und ohne δ wirken γ und h dort nicht — nur fuer die Werkzeuge).
 *
 * 7.5.12 (V2 a): `SuchOptionen` statt des Schalters. A1: die Suche 7.5.12 gibt δ an allen Tasten frei, mit der
 * Untergrenze 0,01 Cent (Lesart S163: numerischer Abstand von der Entartung beider Ebenen, nicht Physik). A4: `detune`
 * steht fest (`detuneFest`) — an den Tasten mit Chor ab zwei Saiten mit dem Start nach SG3 a, den die Stufe vor der
 * Suche je Taste ersetzt (`setzeFest`). A5: Startpunkt 3 ist das Spiegelbild von 2; `besterJeTaste` setzt aus mehreren
 * Laeufen je Taste den besten zusammen. Die Vorgabe `SuchOptionen{}` gibt den Suchraum von 7.5.9 bitgleich.
 ****************************************************************************************
 */

#pragma once

#include "Fluegelmessung.hpp"
#include "Strukturmessung.hpp"

#include "loop/HammerSaite.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace modelltest
{

using KurvenFeld = lumi::loop::Tastenkurve lumi::loop::HammerSaite::*;

/// `highDecay` nach Bensa et al. 2003, Gl. 37a/b, mit dem Streuband b1 × 0,5…3, b2 × 0,5…1,5 (SQ2 a); die
/// Wellenzahl π/L mit der Laenge aus der Vorgabe (Annahme: die Laengen des C6 fehlen).
inline std::pair<double, double> bensaBand(int midi, double laengeM)
{
    const double f0 = lumi::messung::tastenFrequenz(midi);
    const double b1 = std::max(1e-3, 4.4e-3 * f0 - 0.04);
    const double b2 = 1.0e-6 * f0 + 1e-5;
    const double xi2 = std::pow(std::numbers::pi / laengeM, 2.0);
    const auto hd = [&](double c1, double c2) { return (c1 * b1 + c2 * b2 * xi2) / (c1 * b1 + 100.0 * c2 * b2 * xi2); };
    return {hd(0.5, 1.5), hd(3.0, 0.5)};
}

/// Eine Kurve der Suche: je freie Taste Grenzen und Start (als Wert), an den uebrigen gemessenen Tasten fest.
struct SuchKurve
{
    std::string name;
    KurvenFeld feld = nullptr;
    bool log = true;
    std::vector<int> tasten;
    std::vector<double> unten;
    std::vector<double> oben;
    std::vector<double> start;
    std::vector<int> festTasten;
    std::vector<double> festWerte;
    std::string herkunft;

    [[nodiscard]] lumi::loop::KurvenSkala skala() const
    {
        return log ? lumi::loop::KurvenSkala::Logarithmisch : lumi::loop::KurvenSkala::Linear;
    }
    [[nodiscard]] double koordinate(double wert) const { return log ? std::log(wert) : wert; }
    [[nodiscard]] double wert(double koordinate) const { return log ? std::exp(koordinate) : koordinate; }
    /// Die Breite des Bereichs einer freien Taste in der Koordinate (log: ln(oben/unten)).
    [[nodiscard]] double bereich(std::size_t i) const { return koordinate(oben[i]) - koordinate(unten[i]); }
};

/// Alle Kurven der Suche; die Parameter liegen Kurve fuer Kurve hintereinander, je Kurve in der Reihenfolge ihrer Tasten.
struct Suchraum
{
    lumi::loop::HammerSaite basis;
    std::vector<SuchKurve> kurven;

    [[nodiscard]] std::size_t zahl() const
    {
        std::size_t n = 0;
        for (const SuchKurve& k : kurven) n += k.tasten.size();
        return n;
    }
    /// Der erste Parameter der Kurve `k`.
    [[nodiscard]] std::size_t ab(std::size_t k) const
    {
        std::size_t n = 0;
        for (std::size_t i = 0; i < k; ++i) n += kurven[i].tasten.size();
        return n;
    }
};

/// Die Schwebung von *Salamander* an einer Taste in Cent (SG3 a): der Median ueber die Teiltoene k mit Schwebung von
/// 1200·log₂(1 + f_Hz / (k·f_Taste)); ohne Schwebung `std::nullopt`. Median wie in der ganzen Messung
/// (`detail::median`: bei gerader Zahl der obere der beiden mittleren Werte).
inline std::optional<double> schwebungCentAn(const lumi::messung::TonStruktur& struktur, int midi)
{
    std::vector<double> cent;
    for (std::size_t i = 0; i < struktur.size(); ++i)
        if (struktur[i].da && struktur[i].schwebt && struktur[i].schwebungHz > 0.0)
        {
            const double k = static_cast<double>(i + 1);
            cent.push_back(1200.0 * std::log2(1.0 + struktur[i].schwebungHz / (k * lumi::messung::tastenFrequenz(midi))));
        }
    if (cent.empty()) return std::nullopt;
    return lumi::messung::detail::median(std::move(cent));
}

/// Wie der Suchraum gebaut wird; die Vorgabe ist der von 7.5.9 (die Suche 7.5.12 nimmt `suchOptionen7512()`).
struct SuchOptionen
{
    bool deltaImChor = false;     ///< δ auch an Tasten mit zwei und drei Saiten frei (Probe 7.5.11, A1)
    double deltaUntenCent = 0.0;  ///< die Untergrenze von δ an jeder freien Taste (A1: 0,01)
    bool detuneFest = false;      ///< `detune` nicht frei, sondern fest (A4)
    bool deltaFest = false;       ///< δ fest wie `detune` (A4-Weg 2, S165: beide gemeinsam gesetzt)
    bool deltaStartUnten = false; ///< δ startet an der Untergrenze statt an der Schwebung (A4-Weg 3, S165)
};

inline SuchOptionen suchOptionen7512()
{
    return {true, 0.01, true};
}

/// Der Bereich von `detune` an einer Taste (Tabelle 7.5.9: bis 6 Cent, ab 90 bis 7,5).
inline double detuneObenCent(int taste)
{
    return taste < 90 ? 6.0 : 7.5;
}

/// Die neun Kurven der Suche an den `tasten` (die gemessenen von *Salamander*) um die Saite `basis` (die Vorgabe),
/// Grenzen und Start nach der Tabelle des Bauauftrags 7.5.9 §3.3 (SQ2 a, HU2 a (3), SG2 a, SG3 a), mit den Optionen
/// `optionen` (s. Dateikopf). Mit `detuneFest` steht `detune` an allen Tasten fest: im Chor ab zwei Saiten mit dem Start
/// nach SG3 a (in die Grenzen), sonst mit der Vorgabe.
inline Suchraum suchraum(const lumi::loop::HammerSaite& basis, const std::vector<int>& tasten, const lumi::messung::Strukturen& strukturen,
                         const SuchOptionen& optionen = {})
{
    using lumi::loop::HammerSaite;
    using lumi::loop::KurvenSkala;
    Suchraum raum;
    raum.basis = basis;
    const auto log = [&basis](KurvenFeld f, int t) { return lumi::loop::wertAn(basis.*f, t, KurvenSkala::Logarithmisch); };
    const auto lin = [&basis](KurvenFeld f, int t) { return lumi::loop::wertAn(basis.*f, t, KurvenSkala::Linear); };
    const auto schwebung = [&strukturen](int t) -> std::optional<double> {
        const auto it = strukturen.find(t);
        return it != strukturen.end() ? schwebungCentAn(it->second, t) : std::nullopt;
    };
    // eine Kurve: `frei(t)` sagt, ob die Taste frei ist; `grenzen(t)` gibt unten, oben, Start
    const auto kurve = [&](std::string name, KurvenFeld feld, bool istLog, std::string herkunft, auto frei, auto grenzen) {
        SuchKurve k;
        k.name = std::move(name);
        k.feld = feld;
        k.log = istLog;
        k.herkunft = std::move(herkunft);
        for (const int t : tasten)
        {
            if (frei(t))
            {
                const auto [u, o, s] = grenzen(t);
                k.tasten.push_back(t);
                k.unten.push_back(u);
                k.oben.push_back(o);
                k.start.push_back(std::clamp(s, u, o));
            }
            else
            {
                k.festTasten.push_back(t);
                k.festWerte.push_back(lumi::loop::wertAn(basis.*feld, t, k.skala()));
            }
        }
        raum.kurven.push_back(std::move(k));
    };
    using Grenzen = std::tuple<double, double, double>;
    const auto alle = [](int) { return true; };
    const auto chor2 = [&basis](int t) { return lumi::loop::chorAn(basis, t) >= 2; };
    const auto deltaOffen = [&basis, &optionen](int t) { return optionen.deltaImChor || lumi::loop::chorAn(basis, t) == 1; };
    const auto deltaFrei = [&](int t) { return !optionen.deltaFest && deltaOffen(t); };
    const auto detuneFrei = [&](int t) { return !optionen.detuneFest && chor2(t); };

    kurve("decay", &HammerSaite::abklingS, true, "ohne Quelle, Faktor 3", alle, [&](int t) {
        const double s = log(&HammerSaite::abklingS, t);
        return Grenzen{s / 3.0, 3.0 * s, s};
    });
    kurve("lateDecay", &HammerSaite::abklingSpaetS, true, "ohne Quelle, Faktor 3; nie kuerzer als decay", alle, [&](int t) {
        const double s = log(&HammerSaite::abklingSpaetS, t);
        return Grenzen{s / 3.0, 3.0 * s, s};
    });
    kurve("highDecay", &HammerSaite::abklingHoch, true, "Bensa 2003", alle, [&](int t) {
        const auto [u, o] = bensaBand(t, log(&HammerSaite::saitenLaengeM, t));
        return Grenzen{u, o, log(&HammerSaite::abklingHoch, t)};
    });
    const auto detuneGrenzen = [&](int t) {
        const std::optional<double> c = schwebung(t);
        return Grenzen{0.0, detuneObenCent(t), c ? *c : lin(&HammerSaite::verstimmungCent, t)};
    };
    kurve("detune", &HammerSaite::verstimmungCent, false, "Kirk, Cuenca, Plath; Start an der Schwebung (SG3 a)", detuneFrei, detuneGrenzen);
    if (optionen.detuneFest)
    {
        // A4: fest am Start nach SG3 a, wo der Chor zwei oder drei Saiten hat
        SuchKurve& d = raum.kurven.back();
        d.herkunft = "fest (A4): Start nach SG3 a, je Taste an die Schwebung gesetzt";
        for (std::size_t i = 0; i < d.festTasten.size(); ++i)
            if (chor2(d.festTasten[i]))
            {
                const auto [u, o, s] = detuneGrenzen(d.festTasten[i]);
                d.festWerte[i] = std::clamp(s, u, o);
            }
    }
    for (const auto& [name, feld] : {std::pair{"u leise", &HammerSaite::stossLeise}, std::pair{"u laut", &HammerSaite::stossLaut}})
        kurve(name, feld, false, "ohne Quelle, 0-1", chor2, [&, feld = feld](int t) {
            const double v = lin(feld, t);
            return Grenzen{0.0, 1.0, v > 0.0 ? v : 0.25};
        });
    kurve("gamma", &HammerSaite::polarisationWinkelGrad, false, "ohne Quelle, 0-45 Grad (SG2 a), Start 30 (HU2 a (3))", alle,
          [](int) { return Grenzen{0.0, 45.0, 30.0}; });
    const auto deltaGrenzen = [&](int t) {
        const double v = lin(&HammerSaite::polarisationCent, t);
        const std::optional<double> c = schwebung(t);
        return Grenzen{optionen.deltaUntenCent, 8.0, optionen.deltaStartUnten ? optionen.deltaUntenCent : (v > 0.0 ? v : (c ? *c : 0.0))};
    };
    kurve("delta", &HammerSaite::polarisationCent, false, "Tan 2017, Mori 2004; 0-8 Cent", deltaFrei, deltaGrenzen);
    if (optionen.deltaFest)
    {
        // A4-Weg 2: fest am Start, wo δ sonst frei waere; die Stufe setzt es je Taste mit detune
        SuchKurve& d = raum.kurven.back();
        d.herkunft = "fest (A4 gemeinsam mit detune)";
        for (std::size_t i = 0; i < d.festTasten.size(); ++i)
            if (deltaOffen(d.festTasten[i]))
            {
                const auto [u, o, s] = deltaGrenzen(d.festTasten[i]);
                d.festWerte[i] = std::clamp(s, u, o);
            }
    }
    kurve("h", &HammerSaite::waagrechtVerhaeltnis, false, "0-1 (P7 a)", alle,
          [&](int t) { return Grenzen{0.0, 1.0, lin(&HammerSaite::waagrechtVerhaeltnis, t)}; });
    return raum;
}

/// Die Parameter in ihre Grenzen.
inline std::vector<double> klemme(const Suchraum& raum, std::vector<double> x)
{
    std::size_t j = 0;
    for (const SuchKurve& k : raum.kurven)
        for (std::size_t i = 0; i < k.tasten.size(); ++i, ++j)
            x[j] = std::clamp(x[j], k.koordinate(k.unten[i]), k.koordinate(k.oben[i]));
    return x;
}

/// Startpunkt 1 (die Starts der Tabelle), 2 (ZF6 a, SP2 a: log-Groessen × 1,6, lineare + 0,25 des Bereichs) oder 3
/// (A5, Lesart S164: das Spiegelbild von 2 — log ÷ 1,6, lineare − 0,25 des Bereichs), dann in die Grenzen; als
/// Koordinaten.
inline std::vector<double> startpunkt(const Suchraum& raum, int nummer)
{
    std::vector<double> x;
    for (const SuchKurve& k : raum.kurven)
        for (std::size_t i = 0; i < k.tasten.size(); ++i)
        {
            double c = k.koordinate(k.start[i]);
            if (nummer == 2) c += k.log ? std::log(1.6) : 0.25 * (k.oben[i] - k.unten[i]);
            else if (nummer == 3) c -= k.log ? std::log(1.6) : 0.25 * (k.oben[i] - k.unten[i]);
            x.push_back(c);
        }
    return klemme(raum, std::move(x));
}

/// Der feste Wert der Kurve `name` an `taste` (A4: `detune` je Taste nach der Stufe); falsch, wo die Taste dort nicht
/// fest ist.
inline bool setzeFest(Suchraum& raum, const std::string& name, int taste, double wert)
{
    for (SuchKurve& k : raum.kurven)
        if (k.name == name)
            for (std::size_t i = 0; i < k.festTasten.size(); ++i)
                if (k.festTasten[i] == taste)
                {
                    k.festWerte[i] = wert;
                    return true;
                }
    return false;
}

/// Der Start der Kurve `name` an einer freien `taste` (A4-Weg 1: detune startet am Ergebnis von A4), in die Grenzen;
/// falsch, wo die Taste dort nicht frei ist.
inline bool setzeStart(Suchraum& raum, const std::string& name, int taste, double wert)
{
    for (SuchKurve& k : raum.kurven)
        if (k.name == name)
            for (std::size_t i = 0; i < k.tasten.size(); ++i)
                if (k.tasten[i] == taste)
                {
                    k.start[i] = std::clamp(wert, k.unten[i], k.oben[i]);
                    return true;
                }
    return false;
}

/// Der feste Wert der Kurve `name` an `taste`, wenn sie dort fest ist.
inline std::optional<double> festWert(const Suchraum& raum, const std::string& name, int taste)
{
    for (const SuchKurve& k : raum.kurven)
        if (k.name == name)
            for (std::size_t i = 0; i < k.festTasten.size(); ++i)
                if (k.festTasten[i] == taste) return k.festWerte[i];
    return std::nullopt;
}

/// Die Parameter einer Taste (Indizes in x) — ein Block der Suche 7.5.12.
inline std::vector<std::size_t> parameterDerTaste(const Suchraum& raum, int taste)
{
    std::vector<std::size_t> p;
    std::size_t j = 0;
    for (const SuchKurve& k : raum.kurven)
        for (std::size_t i = 0; i < k.tasten.size(); ++i, ++j)
            if (k.tasten[i] == taste) p.push_back(j);
    return p;
}

/// Ein Lauf der Suche (A5): sein Raum (mit den festen Werten nach A4), sein Punkt und je Taste Σ r².
struct SuchLauf
{
    Suchraum raum;
    std::vector<double> x;
    std::vector<double> summeJeTaste;  ///< in der Reihenfolge von `tasten`
};

/// Der beste Lauf je Taste zusammengesetzt (A5): je Taste der mit der kleinsten Summe (bei Gleichstand der fruehere),
/// seine Stuetzpunkte und seine festen Werte an dieser Taste. Die Laeufe teilen dieselben Kurven und Tasten.
struct BesterJeTaste
{
    Suchraum raum;
    std::vector<double> x;
    std::vector<std::size_t> herkunft;  ///< je Taste der Index des Laufs
};

inline BesterJeTaste besterJeTaste(const std::vector<SuchLauf>& laeufe, const std::vector<int>& tasten)
{
    BesterJeTaste b;
    b.raum = laeufe.front().raum;
    b.x = laeufe.front().x;
    for (std::size_t t = 0; t < tasten.size(); ++t)
    {
        std::size_t best = 0;
        for (std::size_t l = 1; l < laeufe.size(); ++l)
            if (laeufe[l].summeJeTaste[t] < laeufe[best].summeJeTaste[t]) best = l;
        b.herkunft.push_back(best);
        for (const std::size_t j : parameterDerTaste(b.raum, tasten[t])) b.x[j] = laeufe[best].x[j];
        for (std::size_t k = 0; k < b.raum.kurven.size(); ++k)
        {
            SuchKurve& ziel = b.raum.kurven[k];
            const SuchKurve& von = laeufe[best].raum.kurven[k];
            for (std::size_t i = 0; i < ziel.festTasten.size(); ++i)
                if (ziel.festTasten[i] == tasten[t]) ziel.festWerte[i] = von.festWerte[i];
        }
    }
    return b;
}

/// Die Saite mit den Parametern `x` (Koordinaten; in die Grenzen geklemmt): je Kurve Stuetzpunkte an allen gemessenen
/// Tasten, ohne Abweichungen; die spaete Stufe nie kuerzer als die fruehe (die Saite liest kuerzer als *Decay*).
inline lumi::loop::HammerSaite saiteAus(const Suchraum& raum, const std::vector<double>& x)
{
    lumi::loop::HammerSaite saite = raum.basis;
    const std::vector<double> xs = klemme(raum, x);
    std::size_t j = 0;
    for (const SuchKurve& k : raum.kurven)
    {
        std::vector<lumi::loop::TastenWert> punkte;
        for (std::size_t i = 0; i < k.tasten.size(); ++i, ++j) punkte.push_back({k.tasten[i], k.wert(xs[j])});
        for (std::size_t i = 0; i < k.festTasten.size(); ++i) punkte.push_back({k.festTasten[i], k.festWerte[i]});
        std::sort(punkte.begin(), punkte.end(), [](const lumi::loop::TastenWert& a, const lumi::loop::TastenWert& b) { return a.taste < b.taste; });
        lumi::loop::Tastenkurve& ziel = saite.*k.feld;
        ziel.punkte = std::move(punkte);
        ziel.abweichungen.clear();
    }
    for (lumi::loop::TastenWert& p : saite.abklingSpaetS.punkte)
        p.wert = std::max(p.wert, lumi::loop::wertAn(saite.abklingS, p.taste, lumi::loop::KurvenSkala::Logarithmisch));
    return saite;
}

}  // namespace modelltest
