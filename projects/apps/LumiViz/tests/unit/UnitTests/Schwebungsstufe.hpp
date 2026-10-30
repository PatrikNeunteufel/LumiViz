/**
 ****************************************************************************************
 * @file   Schwebungsstufe.hpp
 * @brief  7.5.12 (V2 a, A4): `detune` je Taste vor der Suche an die Schwebungsfrequenz von *Salamander* gesetzt, dann
 *         fest (docs/audio/Spielweisen_Konzept.md §4.1, „Teilplan nach V2 a“; Bauauftrag 7.5.12 §5)
 *
 * @details
 * Probe 7.5.11 (S163): das Ziel ueber `detune` ist wellig (2–4 Minima, Kanten) — Levenberg–Marquardt findet dort nicht
 * das globale Minimum. Deshalb wird `detune` vorher je Taste eindimensional gesetzt. Messgroesse ist die
 * Schwebungsfrequenz des Modells je Teilton k = 1…3, gemessen mit derselben Kette wie das Ziel (`strukturAus` je Ebene
 * der Suche am Kanalmittel, `verdichteStruktur` ueber die Ebenen). Ziel (Lesart S164): g = Σ_k (ln f_Modell,k /
 * f_Salamander,k)² ueber die Teiltoene mit Schwebung im Ziel; ein Teilton, der im Ziel schwebt, im Modell nicht, zaehlt
 * (ln 3)². Verfahren: 15 Rasterpunkte ueber den Bereich, dann Goldener Schnitt im Intervall der Nachbarn des besten
 * Rasterpunkts, bis die halbe Breite unter 0,01 Cent liegt oder nach hoechstens 8 Schritten; zum Schluss die Mitte des
 * letzten Intervalls — der beste ausgewertete Punkt gilt (`minimiere1D`, reines C++). Alle Tasten gehen je Schritt in
 * einen Aufruf: eine „Spalte“ ist eine Saite, die an jeder Taste ihren eigenen Wert traegt (eine Taste haengt nur an
 * ihrem Stuetzpunkt, Lesart S161). Gesetzt werden die Tasten mit Chor ab zwei Saiten und Schwebung im Ziel; die
 * uebrigen behalten den Start nach SG3 a.
 ****************************************************************************************
 */

#pragma once

#include "Modellton.hpp"
#include "Strukturmessung.hpp"
#include "Suche.hpp"
#include "Suchkurven.hpp"

#include "loop/HammerSaite.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <numeric>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace modelltest
{

// ---- der eindimensionale Loeser (ohne Ton) ------------------------------------------------------------------------

/// Viele Aufgaben zugleich: `spalten[s][i]` ist der Wert fuer Aufgabe i in der Spalte s; zurueck g in derselben Form.
using Ziel1DSpalten = std::function<std::vector<std::vector<double>>(const std::vector<std::vector<double>>& spalten)>;

struct Minimum1D
{
    double x = 0.0;
    double g = 0.0;
    int auswertungen = 0;
};

/// Je Aufgabe i das Minimum von g ueber [unten[i], oben[i]] (s. Dateikopf). `aufrufe` (wenn gegeben) zaehlt die
/// Aufrufe von `ziel`.
inline std::vector<Minimum1D> minimiere1D(const std::vector<double>& unten, const std::vector<double>& oben, const Ziel1DSpalten& ziel,
                                          int rasterPunkte = 15, double genau = 0.01, int schritteHoechstens = 8, int* aufrufe = nullptr)
{
    const std::size_t n = unten.size();
    std::vector<Minimum1D> m(n);
    if (n == 0) return m;
    const auto rufe = [&](const std::vector<std::vector<double>>& spalten) {
        if (aufrufe != nullptr) ++*aufrufe;
        for (Minimum1D& e : m) e.auswertungen += static_cast<int>(spalten.size());
        return ziel(spalten);
    };
    const auto merke = [&m](std::size_t i, double x, double g) {
        if (g < m[i].g)
        {
            m[i].x = x;
            m[i].g = g;
        }
    };

    // das Raster
    std::vector<std::vector<double>> raster(static_cast<std::size_t>(rasterPunkte), std::vector<double>(n));
    for (int s = 0; s < rasterPunkte; ++s)
        for (std::size_t i = 0; i < n; ++i)
            raster[static_cast<std::size_t>(s)][i] = unten[i] + (oben[i] - unten[i]) * static_cast<double>(s) / (rasterPunkte - 1);
    const std::vector<std::vector<double>> gr = rufe(raster);
    std::vector<double> a(n), b(n);
    for (std::size_t i = 0; i < n; ++i)
    {
        std::size_t best = 0;
        for (std::size_t s = 1; s < raster.size(); ++s)
            if (gr[s][i] < gr[best][i]) best = s;
        m[i].x = raster[best][i];
        m[i].g = gr[best][i];
        a[i] = raster[best > 0 ? best - 1 : 0][i];
        b[i] = raster[std::min(best + 1, raster.size() - 1)][i];
    }

    // der Goldene Schnitt in [a, b]
    const double phi = (std::sqrt(5.0) - 1.0) / 2.0;
    std::vector<double> c(n), d(n), gc(n), gd(n);
    for (std::size_t i = 0; i < n; ++i)
    {
        c[i] = b[i] - phi * (b[i] - a[i]);
        d[i] = a[i] + phi * (b[i] - a[i]);
    }
    {
        const std::vector<std::vector<double>> g = rufe({c, d});
        for (std::size_t i = 0; i < n; ++i)
        {
            gc[i] = g[0][i];
            gd[i] = g[1][i];
            merke(i, c[i], gc[i]);
            merke(i, d[i], gd[i]);
        }
    }
    for (int schritt = 0; schritt < schritteHoechstens; ++schritt)
    {
        bool weiter = false;
        for (std::size_t i = 0; i < n; ++i) weiter = weiter || (b[i] - a[i]) / 2.0 >= genau;
        if (!weiter) break;
        std::vector<double> neu(n);
        std::vector<bool> links(n);
        for (std::size_t i = 0; i < n; ++i)
        {
            links[i] = gc[i] < gd[i];
            if (links[i])
            {
                b[i] = d[i];
                d[i] = c[i];
                gd[i] = gc[i];
                c[i] = b[i] - phi * (b[i] - a[i]);
                neu[i] = c[i];
            }
            else
            {
                a[i] = c[i];
                c[i] = d[i];
                gc[i] = gd[i];
                d[i] = a[i] + phi * (b[i] - a[i]);
                neu[i] = d[i];
            }
        }
        const std::vector<double> g = rufe({neu}).front();
        for (std::size_t i = 0; i < n; ++i)
        {
            (links[i] ? gc[i] : gd[i]) = g[i];
            merke(i, neu[i], g[i]);
        }
    }

    // die Mitte des letzten Intervalls
    std::vector<double> mitte(n);
    for (std::size_t i = 0; i < n; ++i) mitte[i] = (a[i] + b[i]) / 2.0;
    const std::vector<double> g = rufe({mitte}).front();
    for (std::size_t i = 0; i < n; ++i) merke(i, mitte[i], g[i]);
    return m;
}

// ---- die Schwebung des Modells -------------------------------------------------------------------------------

/// Je Saite und Taste die Struktur des Modells (Teiltoene 1–3), gemessen wie das Ziel: `strukturAus` je Ebene der
/// Suche am Kanalmittel, `verdichteStruktur` ueber die Ebenen. Alle Toene auf einem Pool, die laengsten zuerst.
inline std::vector<std::vector<lumi::messung::TonStruktur>> strukturenDesModells(const std::vector<lumi::loop::HammerSaite>& saiten,
                                                                                 const std::vector<int>& tasten,
                                                                                 const lumi::messung::Strukturen& strukturen)
{
    std::vector<lumi::loop::Instrument> instrumente;
    for (const lumi::loop::HammerSaite& s : saiten) instrumente.push_back(fluegelMit(s));
    std::vector<int> dauer;
    for (const int m : tasten)
    {
        const auto it = strukturen.find(m);
        dauer.push_back(dauerMsAus(it != strukturen.end() ? it->second : lumi::messung::TonStruktur{}));
    }
    const std::size_t je = kSuchEbenen.size();
    const std::size_t zahl = saiten.size() * tasten.size() * je;
    std::vector<std::size_t> reihe(zahl);
    std::iota(reihe.begin(), reihe.end(), std::size_t{0});
    std::stable_sort(reihe.begin(), reihe.end(), [&](std::size_t a, std::size_t b) { return dauer[(a / je) % tasten.size()] > dauer[(b / je) % tasten.size()]; });
    std::vector<lumi::messung::TonHuellen> huellen(zahl);
    parallelJe(static_cast<int>(zahl), poolThreads(), [&](int i) {
        const std::size_t auftrag = reihe[static_cast<std::size_t>(i)];
        const std::size_t taste = (auftrag / je) % tasten.size();
        const std::size_t saite = auftrag / (je * tasten.size());
        huellen[auftrag] = messeEbene(instrumente[saite], tasten[taste], kSuchEbenen[auftrag % je], dauer[taste]).huellen;
    });
    std::vector<std::vector<lumi::messung::TonStruktur>> alle(saiten.size(), std::vector<lumi::messung::TonStruktur>(tasten.size()));
    for (std::size_t s = 0; s < saiten.size(); ++s)
        for (std::size_t t = 0; t < tasten.size(); ++t)
            for (int k = 1; k <= lumi::messung::kKurvenTeiltoene; ++k)
            {
                std::vector<lumi::messung::TeiltonStruktur> jeEbene;
                for (std::size_t e = 0; e < je; ++e)
                    if (const lumi::messung::Teilton* tt = lumi::messung::teiltonK(huellen[(s * tasten.size() + t) * je + e].mittel, k); tt != nullptr)
                        jeEbene.push_back(lumi::messung::strukturAus(*tt, lumi::messung::tastenFrequenz(tasten[t])));
                alle[s][t][static_cast<std::size_t>(k - 1)] = lumi::messung::verdichteStruktur(jeEbene);
            }
    return alle;
}

/// Hat das Ziel an der Taste Schwebung (mindestens ein Teilton 1–3)?
inline bool schwebtImZiel(const lumi::messung::TonStruktur& ziel)
{
    for (const lumi::messung::TeiltonStruktur& s : ziel)
        if (s.da && s.schwebt && s.schwebungHz > 0.0) return true;
    return false;
}

/// Der Fehlbetrag eines Teiltons, der im Ziel schwebt und im Modell nicht (Lesart S164, wie r = 10 in 7.5.9).
inline const double kSchwebungFehlt = std::log(3.0) * std::log(3.0);

/// g = Σ_k (ln f_Modell,k / f_Ziel,k)² ueber die Teiltoene mit Schwebung im Ziel (s. Dateikopf).
inline double schwebungsAbstand(const lumi::messung::TonStruktur& modell, const lumi::messung::TonStruktur& ziel)
{
    double g = 0.0;
    for (std::size_t k = 0; k < ziel.size(); ++k)
    {
        if (!(ziel[k].da && ziel[k].schwebt && ziel[k].schwebungHz > 0.0)) continue;
        if (modell[k].da && modell[k].schwebt && modell[k].schwebungHz > 0.0)
        {
            const double l = std::log(modell[k].schwebungHz / ziel[k].schwebungHz);
            g += l * l;
        }
        else
            g += kSchwebungFehlt;
    }
    return g;
}

// ---- die Stufe ------------------------------------------------------------------------------------------------

/// Je Taste, was die Stufe getan hat; die Frequenzen je Teilton 1–3 (0 = keine Schwebung).
struct DetuneTaste
{
    int taste = 0;
    bool gesetzt = false;   ///< Chor ab zwei Saiten und Schwebung im Ziel
    std::string grund;      ///< warum nicht gesetzt
    double vorher = 0.0;    ///< Cent, am Start nach SG3 a
    double nachher = 0.0;   ///< Cent
    double gVorher = 0.0;
    double gNachher = 0.0;
    std::array<double, lumi::messung::kKurvenTeiltoene> zielHz{};
    std::array<double, lumi::messung::kKurvenTeiltoene> modellVorherHz{};
    std::array<double, lumi::messung::kKurvenTeiltoene> modellNachherHz{};
    bool mitDelta = false;  ///< A4-Weg 2 (S165): δ gemeinsam mit detune gesetzt
    double deltaVorher = 0.0;
    double deltaNachher = 0.0;
};

struct DetuneStufe
{
    Suchraum raum;  ///< mit `detune` je Taste gesetzt
    std::vector<DetuneTaste> tasten;
    int aufrufe = 0;
    int auswertungen = 0;  ///< Saiten × gesetzte Tasten
    double sekunden = 0.0;
};

inline std::array<double, lumi::messung::kKurvenTeiltoene> schwebungsHz(const lumi::messung::TonStruktur& s)
{
    std::array<double, lumi::messung::kKurvenTeiltoene> hz{};
    for (std::size_t k = 0; k < s.size(); ++k) hz[k] = s[k].da && s[k].schwebt ? s[k].schwebungHz : 0.0;
    return hz;
}

/// Die Saite zu `x` im Raum `raum` mit `detune` an den Tasten `tasten` auf `werte` gesetzt.
inline lumi::loop::HammerSaite saiteMitDetune(Suchraum raum, const std::vector<double>& x, const std::vector<int>& tasten, const std::vector<double>& werte)
{
    for (std::size_t i = 0; i < tasten.size(); ++i) setzeFest(raum, "detune", tasten[i], werte[i]);
    return saiteAus(raum, x);
}

/// A4: `detune` je Taste der Aufgabe gesetzt, die uebrigen Parameter am Punkt `x` (Startpunkt der Suche). Der Raum der
/// Aufgabe muss `detune` fest tragen (`SuchOptionen::detuneFest`).
inline DetuneStufe stufeDetune(const SuchAufgabe& a, const std::vector<double>& x)
{
    const auto anfang = std::chrono::steady_clock::now();
    DetuneStufe st;
    st.raum = a.raum;
    std::vector<int> gesetzt;
    std::vector<double> unten, oben, start;
    std::vector<lumi::messung::TonStruktur> ziel;
    for (const int t : a.tasten)
    {
        DetuneTaste d;
        d.taste = t;
        d.vorher = festWert(a.raum, "detune", t).value_or(0.0);
        d.nachher = d.vorher;
        const auto z = a.strukturen.find(t);
        if (z != a.strukturen.end()) d.zielHz = schwebungsHz(z->second);
        if (lumi::loop::chorAn(a.raum.basis, t) < 2) d.grund = "eine Saite";
        else if (z == a.strukturen.end() || !schwebtImZiel(z->second)) d.grund = "keine Schwebung im Ziel";
        else if (!festWert(a.raum, "detune", t)) d.grund = "detune nicht fest";
        else
        {
            d.gesetzt = true;
            gesetzt.push_back(t);
            unten.push_back(0.0);
            oben.push_back(detuneObenCent(t));
            start.push_back(d.vorher);
            ziel.push_back(z->second);
        }
        st.tasten.push_back(d);
    }
    if (!gesetzt.empty())
    {
        // je Spalte eine Saite; die erste Spalte des Rasteraufrufs ist zugleich der Start (g vorher)
        std::vector<lumi::messung::TonStruktur> amStart;
        bool startOffen = true;
        const Ziel1DSpalten g = [&](const std::vector<std::vector<double>>& spalten) {
            std::vector<lumi::loop::HammerSaite> saiten;
            if (startOffen) saiten.push_back(saiteMitDetune(a.raum, x, gesetzt, start));
            for (const std::vector<double>& s : spalten) saiten.push_back(saiteMitDetune(a.raum, x, gesetzt, s));
            const auto m = strukturenDesModells(saiten, gesetzt, a.strukturen);
            st.auswertungen += static_cast<int>(saiten.size() * gesetzt.size());
            std::size_t ab = 0;
            if (startOffen)
            {
                amStart = m.front();
                startOffen = false;
                ab = 1;
            }
            std::vector<std::vector<double>> werte;
            for (std::size_t s = ab; s < m.size(); ++s)
            {
                werte.emplace_back();
                for (std::size_t i = 0; i < gesetzt.size(); ++i) werte.back().push_back(schwebungsAbstand(m[s][i], ziel[i]));
            }
            return werte;
        };
        const std::vector<Minimum1D> min = minimiere1D(unten, oben, g, 15, 0.01, 8, &st.aufrufe);
        std::size_t i = 0;
        for (DetuneTaste& d : st.tasten)
        {
            if (!d.gesetzt) continue;
            d.gVorher = schwebungsAbstand(amStart[i], ziel[i]);
            d.modellVorherHz = schwebungsHz(amStart[i]);
            // der Start bleibt, wenn er nicht schlechter ist als das Gefundene
            if (min[i].g < d.gVorher) d.nachher = min[i].x;
            d.gNachher = std::min(min[i].g, d.gVorher);
            setzeFest(st.raum, "detune", d.taste, d.nachher);
            ++i;
        }
        // die Frequenzen des Modells am Ergebnis (fuer den Bericht)
        const auto m = strukturenDesModells({saiteAus(st.raum, x)}, gesetzt, a.strukturen);
        st.auswertungen += static_cast<int>(gesetzt.size());
        ++st.aufrufe;
        i = 0;
        for (DetuneTaste& d : st.tasten)
            if (d.gesetzt) d.modellNachherHz = schwebungsHz(m.front()[i++]);
    }
    st.sekunden = std::chrono::duration<double>(std::chrono::steady_clock::now() - anfang).count();
    return st;
}

/// Die Schwebung am Punkt `x` je Taste der Aufgabe gemessen (nach der Suche, nur Bericht): g und die Frequenzen.
inline std::vector<DetuneTaste> schwebungAm(const SuchAufgabe& a, const std::vector<double>& x, const std::vector<DetuneTaste>& nachStufe)
{
    std::vector<int> gesetzt;
    for (const DetuneTaste& d : nachStufe)
        if (d.gesetzt) gesetzt.push_back(d.taste);
    std::vector<DetuneTaste> ende = nachStufe;
    if (gesetzt.empty()) return ende;
    const auto m = strukturenDesModells({saiteAus(a.raum, x)}, gesetzt, a.strukturen);
    std::size_t i = 0;
    for (DetuneTaste& d : ende)
    {
        if (!d.gesetzt) continue;
        d.gNachher = schwebungsAbstand(m.front()[i], a.strukturen.at(d.taste));
        d.modellNachherHz = schwebungsHz(m.front()[i]);
        ++i;
    }
    return ende;
}

// ---- die Datei a4_<start>.txt ---------------------------------------------------------------------------------

inline std::string stufeText(const std::vector<DetuneTaste>& tasten)
{
    std::ostringstream s;
    s << std::setprecision(17)
      << "# taste\tgesetzt\tdetune_vorher\tdetune_nachher\tg_vorher\tg_nachher\tziel_hz_1..3\tmodell_vorher_hz_1..3\tmodell_nachher_hz_1..3\tgrund"
         "\tmit_delta\tdelta_vorher\tdelta_nachher\n";
    for (const DetuneTaste& d : tasten)
    {
        s << d.taste << '\t' << (d.gesetzt ? 1 : 0) << '\t' << d.vorher << '\t' << d.nachher << '\t' << d.gVorher << '\t' << d.gNachher;
        for (const auto* hz : {&d.zielHz, &d.modellVorherHz, &d.modellNachherHz})
            for (const double v : *hz) s << '\t' << v;
        s << '\t' << (d.grund.empty() ? "-" : d.grund) << '\t' << (d.mitDelta ? 1 : 0) << '\t' << d.deltaVorher << '\t' << d.deltaNachher << '\n';
    }
    return s.str();
}

/// Liest, was `stufeText` geschrieben hat (Wiederaufnahme: A4 wird nicht neu gerechnet).
inline std::optional<std::vector<DetuneTaste>> liesStufe(const std::filesystem::path& pfad)
{
    std::ifstream ein(pfad, std::ios::binary);
    if (!ein) return std::nullopt;
    std::vector<DetuneTaste> tasten;
    std::string zeile;
    while (std::getline(ein, zeile))
    {
        if (!zeile.empty() && zeile.back() == '\r') zeile.pop_back();
        if (zeile.empty() || zeile.front() == '#') continue;
        std::istringstream s(zeile);
        std::vector<std::string> f;
        std::string teil;
        while (std::getline(s, teil, '\t')) f.push_back(teil);
        if (f.size() < 16) continue;
        DetuneTaste d;
        d.taste = std::stoi(f[0]);
        d.gesetzt = f[1] == "1";
        d.vorher = std::stod(f[2]);
        d.nachher = std::stod(f[3]);
        d.gVorher = std::stod(f[4]);
        d.gNachher = std::stod(f[5]);
        for (std::size_t k = 0; k < 3; ++k)
        {
            d.zielHz[k] = std::stod(f[6 + k]);
            d.modellVorherHz[k] = std::stod(f[9 + k]);
            d.modellNachherHz[k] = std::stod(f[12 + k]);
        }
        d.grund = f[15] == "-" ? std::string() : f[15];
        if (f.size() >= 19)
        {
            d.mitDelta = f[16] == "1";
            d.deltaVorher = std::stod(f[17]);
            d.deltaNachher = std::stod(f[18]);
        }
        tasten.push_back(d);
    }
    if (tasten.empty()) return std::nullopt;
    return tasten;
}

/// Der Raum mit `detune` (und δ, wo gemeinsam gesetzt) wie in der Stufe.
inline Suchraum raumNachStufe(Suchraum raum, const std::vector<DetuneTaste>& tasten)
{
    for (const DetuneTaste& d : tasten)
        if (d.gesetzt)
        {
            setzeFest(raum, "detune", d.taste, d.nachher);
            if (d.mitDelta) setzeFest(raum, "delta", d.taste, d.deltaNachher);
        }
    return raum;
}

/// A4-Weg 1 (S165): `detune` startet am Ergebnis der Stufe und bleibt frei — der Raum `frei` (detune frei) bekommt die
/// Starts.
inline Suchraum raumMitStartAusStufe(Suchraum frei, const std::vector<DetuneTaste>& tasten)
{
    for (const DetuneTaste& d : tasten)
        if (d.gesetzt) setzeStart(frei, "detune", d.taste, d.nachher);
    return frei;
}

/// A4-Weg 2 (S165): `detune` und δ je Taste gemeinsam an die Schwebung, dann beide fest. Lesart S165: ein Raster 7 × 7
/// ueber beide Bereiche (detune 0 bis 6 bzw. 7,5 Cent, δ von der Untergrenze bis 8 Cent), dann zwei Runden
/// abwechselnd je Koordinate `minimiere1D` (5 Rasterpunkte, Goldener Schnitt) im Intervall ± einem Rasterschritt um den
/// besten Punkt, in der zweiten Runde ± einem Viertel; der Start bleibt, wenn er nicht schlechter ist. Der Raum muss
/// `detune` und δ fest tragen (`SuchOptionen::detuneFest`, `deltaFest`).
inline DetuneStufe stufeDetuneDelta(const SuchAufgabe& a, const std::vector<double>& x)
{
    const auto anfang = std::chrono::steady_clock::now();
    DetuneStufe st;
    st.raum = a.raum;
    std::vector<int> gesetzt;
    std::vector<double> dUnten, dOben, pUnten, pOben;
    std::vector<lumi::messung::TonStruktur> ziel;
    for (const int t : a.tasten)
    {
        DetuneTaste d;
        d.taste = t;
        d.vorher = festWert(a.raum, "detune", t).value_or(0.0);
        d.nachher = d.vorher;
        d.deltaVorher = festWert(a.raum, "delta", t).value_or(0.0);
        d.deltaNachher = d.deltaVorher;
        const auto z = a.strukturen.find(t);
        if (z != a.strukturen.end()) d.zielHz = schwebungsHz(z->second);
        if (lumi::loop::chorAn(a.raum.basis, t) < 2) d.grund = "eine Saite";
        else if (z == a.strukturen.end() || !schwebtImZiel(z->second)) d.grund = "keine Schwebung im Ziel";
        else if (!festWert(a.raum, "detune", t) || !festWert(a.raum, "delta", t)) d.grund = "detune oder delta nicht fest";
        else
        {
            d.gesetzt = true;
            d.mitDelta = true;
            gesetzt.push_back(t);
            dUnten.push_back(0.0);
            dOben.push_back(detuneObenCent(t));
            pUnten.push_back(0.01);  // die Untergrenze von A1
            pOben.push_back(8.0);
            ziel.push_back(z->second);
        }
        st.tasten.push_back(d);
    }
    if (gesetzt.empty())
    {
        st.sekunden = std::chrono::duration<double>(std::chrono::steady_clock::now() - anfang).count();
        return st;
    }
    const std::size_t n = gesetzt.size();
    const auto saite = [&](const std::vector<double>& detune, const std::vector<double>& delta) {
        Suchraum r = a.raum;
        for (std::size_t i = 0; i < n; ++i)
        {
            setzeFest(r, "detune", gesetzt[i], detune[i]);
            setzeFest(r, "delta", gesetzt[i], delta[i]);
        }
        return saiteAus(r, x);
    };
    // je Spalte ein Paar (detune, δ) je Taste; zurueck g je Spalte und Taste
    const auto werte = [&](const std::vector<std::vector<double>>& detune, const std::vector<std::vector<double>>& delta) {
        std::vector<lumi::loop::HammerSaite> saiten;
        for (std::size_t s = 0; s < detune.size(); ++s) saiten.push_back(saite(detune[s], delta[s]));
        const auto m = strukturenDesModells(saiten, gesetzt, a.strukturen);
        ++st.aufrufe;
        st.auswertungen += static_cast<int>(saiten.size() * n);
        std::vector<std::vector<double>> g(m.size(), std::vector<double>(n));
        for (std::size_t s = 0; s < m.size(); ++s)
            for (std::size_t i = 0; i < n; ++i) g[s][i] = schwebungsAbstand(m[s][i], ziel[i]);
        return g;
    };
    std::vector<double> startD, startP;
    for (const DetuneTaste& d : st.tasten)
        if (d.gesetzt)
        {
            startD.push_back(d.vorher);
            startP.push_back(d.deltaVorher);
        }

    // das Raster 7 × 7 und der Start in einem Aufruf
    constexpr int kRaster = 7;
    std::vector<std::vector<double>> rd, rp;
    rd.push_back(startD);
    rp.push_back(startP);
    for (int u = 0; u < kRaster; ++u)
        for (int v = 0; v < kRaster; ++v)
        {
            rd.emplace_back(n);
            rp.emplace_back(n);
            for (std::size_t i = 0; i < n; ++i)
            {
                rd.back()[i] = dUnten[i] + (dOben[i] - dUnten[i]) * u / (kRaster - 1);
                rp.back()[i] = pUnten[i] + (pOben[i] - pUnten[i]) * v / (kRaster - 1);
            }
        }
    const std::vector<std::vector<double>> gr = werte(rd, rp);
    std::vector<double> gStart(n), bestD(n), bestP(n), bestG(n);
    for (std::size_t i = 0; i < n; ++i)
    {
        gStart[i] = gr[0][i];
        std::size_t b = 1;
        for (std::size_t s = 2; s < gr.size(); ++s)
            if (gr[s][i] < gr[b][i]) b = s;
        bestD[i] = rd[b][i];
        bestP[i] = rp[b][i];
        bestG[i] = gr[b][i];
    }
    // zwei Runden je Koordinate
    for (int runde = 0; runde < 2; ++runde)
    {
        const double anteil = runde == 0 ? 1.0 : 0.25;
        for (int koordinate = 0; koordinate < 2; ++koordinate)
        {
            std::vector<double> unten(n), oben(n);
            for (std::size_t i = 0; i < n; ++i)
            {
                const double schritt = anteil * (koordinate == 0 ? (dOben[i] - dUnten[i]) : (pOben[i] - pUnten[i])) / (kRaster - 1);
                const double mitte = koordinate == 0 ? bestD[i] : bestP[i];
                unten[i] = std::max(koordinate == 0 ? dUnten[i] : pUnten[i], mitte - schritt);
                oben[i] = std::min(koordinate == 0 ? dOben[i] : pOben[i], mitte + schritt);
            }
            const Ziel1DSpalten g = [&](const std::vector<std::vector<double>>& spalten) {
                std::vector<std::vector<double>> d, p;
                for (const std::vector<double>& s : spalten)
                {
                    d.push_back(koordinate == 0 ? s : bestD);
                    p.push_back(koordinate == 0 ? bestP : s);
                }
                return werte(d, p);
            };
            const std::vector<Minimum1D> m = minimiere1D(unten, oben, g, 5, 0.01, 8);
            for (std::size_t i = 0; i < n; ++i)
                if (m[i].g < bestG[i])
                {
                    (koordinate == 0 ? bestD[i] : bestP[i]) = m[i].x;
                    bestG[i] = m[i].g;
                }
        }
    }
    std::size_t i = 0;
    for (DetuneTaste& d : st.tasten)
    {
        if (!d.gesetzt) continue;
        d.gVorher = gStart[i];
        if (bestG[i] < gStart[i])
        {
            d.nachher = bestD[i];
            d.deltaNachher = bestP[i];
        }
        d.gNachher = std::min(bestG[i], gStart[i]);
        setzeFest(st.raum, "detune", d.taste, d.nachher);
        setzeFest(st.raum, "delta", d.taste, d.deltaNachher);
        ++i;
    }
    // die Frequenzen am Start und am Ergebnis (fuer den Bericht)
    {
        std::vector<double> nd, np;
        for (const DetuneTaste& d : st.tasten)
            if (d.gesetzt)
            {
                nd.push_back(d.nachher);
                np.push_back(d.deltaNachher);
            }
        const auto m = strukturenDesModells({saite(startD, startP), saite(nd, np)}, gesetzt, a.strukturen);
        ++st.aufrufe;
        st.auswertungen += static_cast<int>(2 * n);
        i = 0;
        for (DetuneTaste& d : st.tasten)
            if (d.gesetzt)
            {
                d.modellVorherHz = schwebungsHz(m[0][i]);
                d.modellNachherHz = schwebungsHz(m[1][i]);
                ++i;
            }
    }
    st.sekunden = std::chrono::duration<double>(std::chrono::steady_clock::now() - anfang).count();
    return st;
}

}  // namespace modelltest
