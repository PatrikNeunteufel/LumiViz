/**
 * @file Setzen.cpp
 * @brief Das Setz-Werkzeug (Herleitung im Kopf von `Setzen.hpp`; aus IC `rechnung/setzen/Setzen.cpp`, bb7e6d6, Journal J24)
 */
#include "InstrumentMeasure/Setzen.hpp"

#include "InstrumentCore/Pool.hpp"
#include "InstrumentMeasure/LevenbergMarquardt.hpp"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace lumi::messung::setzen
{

using kern::parallelJe;

namespace
{

double nachU(const Parameter& p, double x)
{
    if (p.skala == Skala::Log) return std::log(x / p.unten) / std::log(p.oben / p.unten);
    return (x - p.unten) / (p.oben - p.unten);
}

double ausU(const Parameter& p, double u)
{
    u = std::clamp(u, 0.0, 1.0);
    if (p.skala == Skala::Log) return p.unten * std::pow(p.oben / p.unten, u);
    return p.unten + u * (p.oben - p.unten);
}

/// Der Zustand je Gruppe (L8).
struct Zustand
{
    std::vector<double> u;
    std::vector<bool> frei;  ///< wird gesucht
    double lambda = 1e-2;
    double rest = 0.0;
    std::vector<double> r;
    bool fertig = false;
    int iterationen = 0;  ///< so oft war die Gruppe aktiv (wie `LmBlockStand::iterationen`)
    std::string ende;
    std::vector<std::size_t> ohneWirkung;  ///< A3, aus der letzten Iteration
};

/// ME3 a: Schritt und Ende je Parameter (0 = aus der Einstellung).
double schrittVon(const Parameter& p, const Einstellung& e)
{
    return p.schritt > 0.0 ? p.schritt : e.schrittH;
}

double endeVon(const Parameter& p, const Einstellung& e)
{
    return p.ende > 0.0 ? p.ende : e.ende;
}

/// Im Protokoll stehen die Enden ohne Leerzeichen (gelesen mit `>>`).
std::string ohneLeer(std::string s)
{
    std::replace(s.begin(), s.end(), ' ', '_');
    return s;
}

std::string mitLeer(std::string s)
{
    std::replace(s.begin(), s.end(), '_', ' ');
    return s;
}

std::vector<double> werteAus(const Gruppe& g, const std::vector<double>& u)
{
    std::vector<double> w(u.size());
    for (std::size_t j = 0; j < u.size(); ++j) w[j] = ausU(g.parameter[j], u[j]);
    return w;
}

/// Je Messgroesse die Abweichung wie `messung::vergleiche`: r in der Skala der Groesse; fehlt der Wert (oder ist er bei
/// Faktor nicht positiv), zaehlt sie mit `kFehltVerhaeltnis`.
std::vector<messung::Abweichung> abweichungAus(const Gruppe& g, const std::vector<double>& m)
{
    std::vector<messung::Abweichung> aus;
    for (std::size_t i = 0; i < g.ziele.size(); ++i)
    {
        const Messgroesse& z = g.ziele[i];
        messung::Abweichung a;
        a.groesse = z.name;
        a.skala = z.skala;
        a.ziel = z.ziel;
        a.toleranz = z.toleranz;
        const bool gueltig = i < m.size() && std::isfinite(m[i]) && (z.skala != messung::Skala::Faktor || (m[i] > 0.0 && z.ziel > 0.0));
        if (gueltig)
        {
            a.modell = m[i];
            a.residuum = (z.skala == messung::Skala::Faktor ? std::log(m[i] / z.ziel) : m[i] - z.ziel) / z.toleranz;
            a.verhaeltnis = std::abs(a.residuum);
        }
        else
        {
            a.fehlt = true;
            a.residuum = a.verhaeltnis = messung::kFehltVerhaeltnis;
        }
        a.getroffen = !a.fehlt && a.verhaeltnis <= 1.0;
        aus.push_back(std::move(a));
    }
    return aus;
}

std::vector<double> reste(const Gruppe& g, const std::vector<double>& m)
{
    std::vector<double> r;
    for (const messung::Abweichung& a : abweichungAus(g, m)) r.push_back(a.residuum);
    return r;
}

int zaehleHoerbar(const std::vector<messung::Abweichung>& groessen, const messung::Hoerschwellen* schwellen)
{
    int n = 0;
    for (const messung::Abweichung& a : groessen)
        n += (schwellen != nullptr ? messung::hoerbarGetroffen(*schwellen, a) : a.getroffen) ? 1 : 0;
    return n;
}

/// Viele Auswertungen auf dem Pool (L8): je Aufgabe (Gruppe, u) der Rest-Vektor.
std::vector<std::vector<double>> werteAlle(const std::vector<Gruppe>& gruppen, const Modell& modell,
                                           const std::vector<std::pair<std::size_t, std::vector<double>>>& aufgaben, unsigned threads)
{
    std::vector<std::vector<double>> aus(aufgaben.size());
    parallelJe(
        aufgaben.size(),
        [&](std::size_t k) {
            const Gruppe& g = gruppen[aufgaben[k].first];
            aus[k] = reste(g, modell(aufgaben[k].first, werteAus(g, aufgaben[k].second)));
        },
        threads);
    return aus;
}

std::string protokollDatei(const std::string& ordner, int iteration)
{
    char z[32];
    std::snprintf(z, sizeof z, "iteration_%03d.tsv", iteration);
    return (std::filesystem::path(ordner) / z).string();
}

void schreibeProtokoll(const std::string& ordner, int iteration, const std::vector<Gruppe>& gruppen, const std::vector<Zustand>& z)
{
    if (ordner.empty()) return;
    std::filesystem::create_directories(ordner);
    std::ofstream aus(protokollDatei(ordner, iteration));
    aus << "# gruppe\tlambda\trest\tfertig\tende\titerationen\tu …\n";
    for (std::size_t g = 0; g < gruppen.size(); ++g)
    {
        char z0[64];
        std::snprintf(z0, sizeof z0, "%.17g\t%.17g", z[g].lambda, z[g].rest);
        aus << gruppen[g].name << '\t' << z0 << '\t' << (z[g].fertig ? 1 : 0) << '\t' << (z[g].ende.empty() ? "-" : ohneLeer(z[g].ende)) << '\t'
            << z[g].iterationen;
        for (const double u : z[g].u)
        {
            char zz[32];
            std::snprintf(zz, sizeof zz, "\t%.17g", u);
            aus << zz;
        }
        aus << '\n';
    }
}

/// Die letzte Iteration lesen (L7: Wiederaufnahme); 0, wenn keine.
int liesProtokoll(const std::string& ordner, const std::vector<Gruppe>& gruppen, std::vector<Zustand>& z)
{
    if (ordner.empty() || !std::filesystem::exists(ordner)) return 0;
    int letzte = 0;
    for (int i = 1; i < 10000; ++i)
        if (std::filesystem::exists(protokollDatei(ordner, i)))
            letzte = i;
        else
            break;
    if (letzte == 0) return 0;
    std::ifstream ein(protokollDatei(ordner, letzte));
    std::string zeile;
    std::size_t g = 0;
    while (std::getline(ein, zeile) && g < gruppen.size())
    {
        if (zeile.empty() || zeile[0] == '#') continue;
        std::istringstream s(zeile);
        std::string name, ende;
        int fertig = 0;
        s >> name >> z[g].lambda >> z[g].rest >> fertig >> ende >> z[g].iterationen;
        // „Iterationen“ ist kein Ende: die Wiederaufnahme rechnet weiter, wenn mehr erlaubt sind
        ende = ende == "-" ? "" : mitLeer(ende);
        z[g].fertig = fertig != 0 && ende != "Iterationen";
        z[g].ende = z[g].fertig ? ende : "";
        for (double& u : z[g].u) s >> u;
        ++g;
    }
    return letzte;
}

}  // namespace

Bericht setze(std::vector<Gruppe>& gruppen, const Modell& modell, const Einstellung& e)
{
    Bericht bericht;
    const std::size_t ng = gruppen.size();
    std::vector<Zustand> z(ng);
    for (std::size_t g = 0; g < ng; ++g)
    {
        for (const Parameter& p : gruppen[g].parameter)
        {
            z[g].u.push_back(std::clamp(nachU(p, p.wert), 0.0, 1.0));
            z[g].frei.push_back(!p.fest && !p.wellig);
        }
        z[g].lambda = e.lambdaStart;
    }
    bericht.gruppen.resize(ng);
    for (std::size_t g = 0; g < ng; ++g) bericht.gruppen[g].name = gruppen[g].name;
    for (std::size_t g = 0; g < ng; ++g)
        bericht.gruppen[g].groessenAnfang = abweichungAus(gruppen[g], modell(g, werteAus(gruppen[g], z[g].u)));
    int start = 0;
    if (e.start)
    {
        start = e.start->nummer;
        for (std::size_t g = 0; g < ng; ++g)
        {
            if (g < e.start->lambda.size()) z[g].lambda = e.start->lambda[g];
            if (g < e.start->iteration.size()) z[g].iterationen = e.start->iteration[g];
            if (g < e.start->ende.size()) z[g].ende = e.start->ende[g];
            z[g].fertig = !z[g].ende.empty() && z[g].ende != "Iterationen";
            if (!z[g].fertig) z[g].ende.clear();
        }
    }
    else if (e.wiederaufnahme)
        start = liesProtokoll(e.protokoll, gruppen, z);
    bericht.ersteIteration = start;
    // der Anfang: Reste
    {
        std::vector<std::pair<std::size_t, std::vector<double>>> a;
        for (std::size_t g = 0; g < ng; ++g) a.push_back({g, z[g].u});
        const auto r = werteAlle(gruppen, modell, a, e.threads);
        for (std::size_t g = 0; g < ng; ++g)
        {
            z[g].r = r[g];
            z[g].rest = messung::quadratsumme(r[g]);
            if (start == 0) bericht.gruppen[g].restAnfang = z[g].rest;
        }
    }
    const auto jacobi = [&](double faktor, const std::vector<std::size_t>& welche) {
        // je Gruppe und freie Spalte eine Auswertung — alle in einem Pool-Aufruf (L8)
        std::vector<std::pair<std::size_t, std::vector<double>>> a;
        std::vector<std::pair<std::size_t, std::size_t>> wo;
        for (const std::size_t g : welche)
            for (std::size_t j = 0; j < z[g].u.size(); ++j)
            {
                if (gruppen[g].parameter[j].fest) continue;
                std::vector<double> u = z[g].u;
                const double h = faktor * schrittVon(gruppen[g].parameter[j], e);
                u[j] = u[j] + h <= 1.0 ? u[j] + h : u[j] - h;
                a.push_back({g, u});
                wo.push_back({g, j});
            }
        const auto r = werteAlle(gruppen, modell, a, e.threads);
        std::vector<messung::Matrix> jj(ng);
        for (const std::size_t g : welche) jj[g] = messung::Matrix(gruppen[g].ziele.size(), z[g].u.size());
        for (std::size_t k = 0; k < a.size(); ++k)
        {
            const auto [g, j] = wo[k];
            const double hh = a[k].second[j] - z[g].u[j];
            for (std::size_t i = 0; i < gruppen[g].ziele.size(); ++i) jj[g](i, j) = (r[k][i] - z[g].r[i]) / hh;
        }
        return jj;
    };
    std::vector<std::size_t> alle(ng);
    for (std::size_t g = 0; g < ng; ++g) alle[g] = g;
    // L9 und L11 vor dem Lauf (auch nach der Wiederaufnahme — billig): Wirkung je Parameter, verrauschte Spalten
    {
        const auto j1 = jacobi(1.0, alle);
        const auto j2 = jacobi(0.5, alle);
        for (std::size_t g = 0; g < ng; ++g)
        {
            std::vector<double> diag(z[g].u.size(), 0.0);
            double groesst = 0.0;
            for (std::size_t j = 0; j < z[g].u.size(); ++j)
            {
                for (std::size_t i = 0; i < gruppen[g].ziele.size(); ++i) diag[j] += j1[g](i, j) * j1[g](i, j);
                groesst = std::max(groesst, diag[j]);
            }
            for (std::size_t j = 0; j < z[g].u.size(); ++j)
            {
                Parameter& p = gruppen[g].parameter[j];
                if (p.fest) continue;
                if (diag[j] <= e.wirkungMin * groesst || diag[j] == 0.0)
                {
                    p.fest = true;
                    z[g].frei[j] = false;
                    bericht.gruppen[g].wirkungslos.push_back(p.name);
                    continue;
                }
                double d = 0.0, n = 0.0;
                for (std::size_t i = 0; i < gruppen[g].ziele.size(); ++i)
                {
                    d += std::pow(j1[g](i, j) - j2[g](i, j), 2);
                    n += std::pow(j1[g](i, j), 2);
                }
                if (std::sqrt(d / std::max(n, 1e-300)) > e.rauschGrenze) bericht.gruppen[g].verrauscht.push_back(p.name);
            }
        }
        // L12: wellige Groessen zuerst eindimensional (Raster, dann goldener Schnitt), je Gruppe (nicht nach der
        // Wiederaufnahme: dort stehen sie schon)
        for (std::size_t g = 0; g < ng && start == 0; ++g)
            for (std::size_t j = 0; j < z[g].u.size(); ++j)
            {
                const Parameter& p = gruppen[g].parameter[j];
                if (!p.wellig || p.fest) continue;
                // L16: teilt ein anderer freier Parameter die Wahrnehmung, wird nicht allein gestaffelt
                bool geteilt = false;
                for (std::size_t k = 0; k < z[g].u.size() && !p.traegt.empty(); ++k)
                    if (k != j && !gruppen[g].parameter[k].fest && gruppen[g].parameter[k].traegt == p.traegt) geteilt = true;
                if (geteilt)
                {
                    bericht.gruppen[g].nichtGestaffelt.push_back(p.name);
                    continue;
                }
                std::vector<std::pair<std::size_t, std::vector<double>>> a;
                for (int k = 0; k < e.wellenRaster; ++k)
                {
                    std::vector<double> u = z[g].u;
                    u[j] = static_cast<double>(k) / (e.wellenRaster - 1);
                    a.push_back({g, u});
                }
                const auto r = werteAlle(gruppen, modell, a, e.threads);
                int best = 0;
                for (int k = 1; k < e.wellenRaster; ++k)
                    if (messung::quadratsumme(r[static_cast<std::size_t>(k)]) < messung::quadratsumme(r[static_cast<std::size_t>(best)])) best = k;
                double lo = std::max(0.0, (best - 1.0) / (e.wellenRaster - 1)), hi = std::min(1.0, (best + 1.0) / (e.wellenRaster - 1));
                const double phi = 0.5 * (std::sqrt(5.0) - 1.0);
                const auto f = [&](double uu) {
                    std::vector<double> u = z[g].u;
                    u[j] = uu;
                    return messung::quadratsumme(reste(gruppen[g], modell(g, werteAus(gruppen[g], u))));
                };
                double x1 = hi - phi * (hi - lo), x2 = lo + phi * (hi - lo), f1 = f(x1), f2 = f(x2);
                for (int it = 0; it < 40; ++it)
                {
                    if (f1 < f2)
                    {
                        hi = x2;
                        x2 = x1;
                        f2 = f1;
                        x1 = hi - phi * (hi - lo);
                        f1 = f(x1);
                    }
                    else
                    {
                        lo = x1;
                        x1 = x2;
                        f1 = f2;
                        x2 = lo + phi * (hi - lo);
                        f2 = f(x2);
                    }
                }
                z[g].u[j] = 0.5 * (lo + hi);
                z[g].r = reste(gruppen[g], modell(g, werteAus(gruppen[g], z[g].u)));
                z[g].rest = messung::quadratsumme(z[g].r);
            }
    }
    // LM je Gruppe mit eigenem λ (L8) — in LumiViz das LM je Block (7.5.12 A2/A3; ME3 a): x sind die u aller Gruppen
    // hintereinander, ein Block die freien Parameter einer Gruppe, die Gruppe der finiten Differenz die k-te freie
    // Spalte jeder Gruppe (je Block hoechstens ein Parameter)
    {
        std::vector<std::size_t> ab(ng + 1, 0);
        for (std::size_t g = 0; g < ng; ++g) ab[g + 1] = ab[g] + z[g].u.size();
        messung::LmRaum raum;
        std::vector<double> differenz;
        std::vector<double> x0;
        std::vector<messung::LmBlock> bloecke(ng);
        std::vector<std::vector<std::size_t>> spalten;
        for (std::size_t g = 0; g < ng; ++g)
        {
            std::size_t k = 0;
            for (std::size_t j = 0; j < z[g].u.size(); ++j)
            {
                const Parameter& p = gruppen[g].parameter[j];
                raum.unten.push_back(0.0);
                raum.oben.push_back(1.0);
                raum.ende.push_back(endeVon(p, e));
                differenz.push_back(schrittVon(p, e));
                x0.push_back(z[g].u[j]);
                if (!z[g].frei[j]) continue;
                bloecke[g].parameter.push_back(ab[g] + j);
                if (spalten.size() <= k) spalten.resize(k + 1);
                spalten[k++].push_back(ab[g] + j);
            }
        }
        messung::LmStartBloecke lmStart(x0);
        lmStart.nummer = start;
        for (std::size_t g = 0; g < ng; ++g)
        {
            lmStart.lambda.push_back(z[g].lambda);
            lmStart.iteration.push_back(z[g].iterationen);
            // eine Gruppe ohne freien Parameter hat nichts zu suchen
            lmStart.ende.push_back(z[g].fertig ? z[g].ende : (bloecke[g].parameter.empty() ? std::string("keine freien Parameter") : std::string()));
        }
        const auto uDer = [&](const std::vector<double>& x, std::size_t g) {
            return std::vector<double>(x.begin() + static_cast<std::ptrdiff_t>(ab[g]), x.begin() + static_cast<std::ptrdiff_t>(ab[g + 1]));
        };
        const messung::LmResiduenBloecke residuen = [&](const std::vector<std::vector<double>>& punkte, const std::vector<std::size_t>& welche) {
            std::vector<std::pair<std::size_t, std::vector<double>>> aufgaben;
            for (const std::vector<double>& p : punkte)
                for (const std::size_t g : welche) aufgaben.push_back({g, uDer(p, g)});
            return werteAlle(gruppen, modell, aufgaben, e.threads);
        };
        messung::LmEinstellungen lm;
        lm.lambdaStart = e.lambdaStart;
        lm.lambdaFaktor = e.lambdaFaktor;
        lm.iterationenHoechstens = e.iterationenHoechstens;
        lm.versucheHoechstens = e.versucheHoechstens;
        lm.wirkungMindestens = e.wirkungMindestens;
        const messung::LmBeobachterBloecke jeIteration = [&](const messung::LmIterationBloecke& it) {
            for (std::size_t g = 0; g < ng; ++g)
            {
                const messung::LmBlockSchritt& s = it.bloecke[g];
                z[g].u = uDer(it.x, g);
                if (!s.aktiv) continue;
                z[g].lambda = s.lambda;
                z[g].rest = s.rest;
                z[g].ende = s.ende;
                z[g].fertig = !s.ende.empty();
                ++z[g].iterationen;
                z[g].ohneWirkung.clear();
                for (const std::size_t j : s.ohneWirkung) z[g].ohneWirkung.push_back(j - ab[g]);
            }
            schreibeProtokoll(e.protokoll, it.nummer, gruppen, z);
            if (!e.beobachter) return;
            Iteration aus;
            aus.nummer = it.nummer;
            aus.auswertungen = it.auswertungen;
            for (std::size_t g = 0; g < ng; ++g)
            {
                const messung::LmBlockSchritt& s = it.bloecke[g];
                GruppenSchritt gs;
                gs.aktiv = s.aktiv;
                gs.angenommen = s.angenommen;
                gs.restVorher = s.aktiv ? s.restVorher : z[g].rest;
                gs.rest = z[g].rest;
                gs.lambdaVorher = s.aktiv ? s.lambdaVorher : z[g].lambda;
                gs.lambda = z[g].lambda;
                gs.versuche = s.versuche;
                gs.verworfen = s.verworfen;
                gs.iterationen = z[g].iterationen;
                gs.werte = werteAus(gruppen[g], z[g].u);
                gs.diagonale.assign(z[g].u.size(), 0.0);
                for (std::size_t q = 0; q < s.diagonale.size() && q < bloecke[g].parameter.size(); ++q)
                    gs.diagonale[bloecke[g].parameter[q] - ab[g]] = s.diagonale[q];
                gs.ohneWirkung = z[g].ohneWirkung;
                gs.ende = z[g].ende;
                aus.gruppen.push_back(std::move(gs));
            }
            e.beobachter(aus);
        };
        const messung::LmErgebnisBloecke ergebnis =
            messung::levenbergMarquardtJeBlock(residuen, bloecke, spalten, differenz, lmStart, raum, lm, jeIteration);
        for (std::size_t g = 0; g < ng; ++g)
        {
            z[g].u = uDer(ergebnis.x, g);
            z[g].rest = ergebnis.bloecke[g].rest;
            z[g].r = ergebnis.bloecke[g].r;
            z[g].lambda = ergebnis.bloecke[g].lambda;
            z[g].ende = ergebnis.bloecke[g].ende;
            z[g].fertig = !z[g].ende.empty() && z[g].ende != "Iterationen";
        }
    }
    // der Bericht (L7, L13, L14): Treffer und Rest, Diagonale, Grenzen, hoerbar
    const auto jEnde = jacobi(1.0, alle);
    for (std::size_t g = 0; g < ng; ++g)
    {
        GruppenBericht& b = bericht.gruppen[g];
        b.werte = werteAus(gruppen[g], z[g].u);
        for (std::size_t j = 0; j < z[g].u.size(); ++j) gruppen[g].parameter[j].wert = b.werte[j];
        b.rest = z[g].rest;
        b.iterationen = z[g].iterationen;
        b.ende = z[g].fertig ? z[g].ende : "Iterationen";
        b.lambda = z[g].lambda;
        for (const std::size_t j : z[g].ohneWirkung) b.ohneWirkung.push_back(gruppen[g].parameter[j].name);
        b.groessen = abweichungAus(gruppen[g], modell(g, b.werte));
        for (const messung::Abweichung& a : b.groessen) b.treffer += a.getroffen ? 1 : 0;
        b.hoerbarGetroffen = zaehleHoerbar(b.groessen, e.hoerschwellen);
        b.diagonale.assign(z[g].u.size(), 0.0);
        for (std::size_t j = 0; j < z[g].u.size(); ++j)
        {
            for (std::size_t i = 0; i < gruppen[g].ziele.size(); ++i) b.diagonale[j] += jEnde[g](i, j) * jEnde[g](i, j);
            if (!gruppen[g].parameter[j].fest && (z[g].u[j] < 1e-3 || z[g].u[j] > 1.0 - 1e-3)) b.amRand.push_back(gruppen[g].parameter[j].name);
        }
    }
    return bericht;
}

std::string Bericht::text() const
{
    std::ostringstream s;
    s << "Vorschlag (L18): gesetzt erst nach bestandener Probe und Gehoer\n";
    for (const GruppenBericht& g : gruppen)
    {
        s << g.name << ": Rest " << g.restAnfang << " → " << g.rest << ", Treffer " << g.treffer << ", hoerbar getroffen " << g.hoerbarGetroffen << ", Iterationen "
          << g.iterationen << " (" << g.ende << ")";
        if (!g.wirkungslos.empty())
        {
            s << "; wirkungslos:";
            for (const auto& n : g.wirkungslos) s << ' ' << n;
        }
        if (!g.verrauscht.empty())
        {
            s << "; verrauscht:";
            for (const auto& n : g.verrauscht) s << ' ' << n;
        }
        if (!g.nichtGestaffelt.empty())
        {
            s << "; nicht gestaffelt (L16):";
            for (const auto& n : g.nichtGestaffelt) s << ' ' << n;
        }
        if (!g.amRand.empty())
        {
            s << "; am Rand:";
            for (const auto& n : g.amRand) s << ' ' << n;
        }
        s << '\n';
    }
    return s.str();
}

std::vector<std::pair<double, std::vector<double>>> schnitt(const Gruppe& g, std::size_t gruppe, std::size_t parameter, const Modell& modell, int punkte)
{
    std::vector<std::pair<double, std::vector<double>>> aus(static_cast<std::size_t>(std::max(2, punkte)));
    std::vector<double> werte;
    for (const Parameter& p : g.parameter) werte.push_back(p.wert);
    parallelJe(aus.size(), [&](std::size_t k) {
        std::vector<double> w = werte;
        w[parameter] = ausU(g.parameter[parameter], static_cast<double>(k) / (aus.size() - 1));
        aus[k] = {w[parameter], modell(gruppe, w)};
    });
    return aus;
}

std::vector<int> welligkeit(const Gruppe& g, std::size_t gruppe, std::size_t parameter, const Modell& modell, int punkte)
{
    const auto s = schnitt(g, gruppe, parameter, modell, punkte);
    std::vector<std::vector<double>> rs;
    for (const auto& [x, m] : s) rs.push_back(reste(g, m));
    std::vector<int> minima(g.ziele.size(), 0);
    for (std::size_t i = 0; i < g.ziele.size(); ++i)
    {
        const auto r = [&](std::size_t k) { return std::abs(rs[k][i]); };
        for (std::size_t k = 1; k + 1 < s.size(); ++k)
            if (r(k) < r(k - 1) && r(k) <= r(k + 1)) ++minima[i];
    }
    return minima;
}

std::vector<messung::Abweichung> abweichungen(const Gruppe& g, std::size_t gruppe, const Modell& modell, const std::vector<double>& werte)
{
    return abweichungAus(g, modell(gruppe, werte));
}

Glaette glaette(const Gruppe& g, std::size_t gruppe, std::size_t parameter, const Modell& modell, int punkte)
{
    const auto s = schnitt(g, gruppe, parameter, modell, punkte);
    std::vector<double> summe;
    for (const auto& [x, m] : s) summe.push_back(messung::quadratsumme(reste(g, m)));
    Glaette gl;
    const std::size_t n = summe.size();
    for (std::size_t k = 0; k < n; ++k)
    {
        const bool links = k == 0 || summe[k] < summe[k - 1];
        const bool rechts = k + 1 == n || summe[k] < summe[k + 1];
        if (links && rechts) ++gl.minima;
    }
    for (std::size_t k = 0; k + 1 < n; ++k)
    {
        const double d = std::abs(summe[k + 1] - summe[k]);
        gl.mittlererSchritt += d;
        gl.groessterSchritt = std::max(gl.groessterSchritt, d);
    }
    gl.mittlererSchritt /= static_cast<double>(n - 1);
    return gl;
}

Mehrfach setzeMehrfach(const std::vector<Gruppe>& gruppen, const std::vector<std::vector<std::vector<double>>>& starts, const Modell& modell,
                       const Einstellung& e)
{
    Mehrfach m;
    for (std::size_t s = 0; s < starts.size(); ++s)
    {
        std::vector<Gruppe> gs = gruppen;
        for (std::size_t g = 0; g < gs.size() && g < starts[s].size(); ++g)
            for (std::size_t j = 0; j < gs[g].parameter.size() && j < starts[s][g].size(); ++j) gs[g].parameter[j].wert = starts[s][g][j];
        Einstellung es = e;
        if (!e.protokoll.empty()) es.protokoll = (std::filesystem::path(e.protokoll) / ("start_" + std::to_string(s))).string();
        m.laeufe.push_back(setze(gs, modell, es));
        m.gruppenJeLauf.push_back(std::move(gs));
    }
    if (m.laeufe.empty()) return m;
    for (std::size_t g = 0; g < gruppen.size(); ++g)
    {
        std::size_t best = 0;
        for (std::size_t l = 1; l < m.laeufe.size(); ++l)
            if (m.laeufe[l].gruppen[g].rest < m.laeufe[best].gruppen[g].rest) best = l;
        m.herkunft.push_back(best);
        m.beste.push_back(m.gruppenJeLauf[best][g]);
        m.bester.gruppen.push_back(m.laeufe[best].gruppen[g]);
    }
    return m;
}

std::vector<bool> eindeutig(const Mehrfach& m, std::size_t gruppe, const messung::Hoerschwellen& schwellen)
{
    std::vector<bool> aus;
    if (m.laeufe.empty()) return aus;
    for (std::size_t i = 0; i < m.laeufe.front().gruppen[gruppe].groessen.size(); ++i)
    {
        std::vector<const messung::Abweichung*> jeStart;
        for (const Bericht& b : m.laeufe) jeStart.push_back(&b.gruppen[gruppe].groessen[i]);
        aus.push_back(messung::eindeutigNachHw2(schwellen, jeStart));
    }
    return aus;
}

std::vector<messung::TastenSumme> alsTasten(const Bericht& b, bool amAnfang)
{
    std::vector<messung::TastenSumme> tasten;
    for (std::size_t g = 0; g < b.gruppen.size(); ++g)
    {
        messung::TastenSumme t;
        t.midi = static_cast<int>(g);
        t.groessen = amAnfang ? b.gruppen[g].groessenAnfang : b.gruppen[g].groessen;
        for (const messung::Abweichung& a : t.groessen)
        {
            t.summe += a.verhaeltnis * a.verhaeltnis;
            t.getroffen += a.getroffen ? 1 : 0;
            t.fehlend += a.fehlt ? 1 : 0;
        }
        t.fertig = !t.groessen.empty() && t.getroffen == static_cast<int>(t.groessen.size());
        tasten.push_back(std::move(t));
    }
    return tasten;
}

Probe probe(const std::vector<Gruppe>& vorgabe, const Mehrfach& m, const Modell& modell, const messung::Hoerschwellen& schwellen, int glattPunkte)
{
    Probe p;
    for (std::size_t g = 0; g < m.beste.size(); ++g)
    {
        const Gruppe& best = m.beste[g];
        ++p.gruppen;
        // (2) Glaette am besten Punkt, alle nicht festen Parameter (wirkungslose stehen nach L9 als fest)
        bool glatt = true;
        for (std::size_t j = 0; j < best.parameter.size(); ++j)
            if (!best.parameter[j].fest && !glaette(best, g, j, modell, glattPunkte).glatt()) glatt = false;
        p.glatt += glatt ? 1 : 0;
        if (!glatt) p.nichtGlatt.push_back(best.name);
        // (3) eindeutig, ohne die nach ZF1 a nicht erreichbaren Gruppen
        bool erreichbar = true;
        for (const std::string& rand : m.bester.gruppen[g].amRand)
            for (const Parameter& par : best.parameter)
                if (par.name == rand && par.grenzeBelegt) erreichbar = false;
        if (!erreichbar)
            p.nichtErreichbar.push_back(best.name);
        else
            for (const bool e : eindeutig(m, g, schwellen))
            {
                ++p.messbar;
                p.eindeutig += e ? 1 : 0;
            }
        // besser als die Vorgabe
        std::vector<double> werte;
        for (const Parameter& par : vorgabe[g].parameter) werte.push_back(par.wert);
        p.hoerbarVorgabe += zaehleHoerbar(abweichungen(vorgabe[g], g, modell, werte), &schwellen);
        p.hoerbarBest += zaehleHoerbar(m.bester.gruppen[g].groessen, &schwellen);
    }
    return p;
}

std::string Probe::text() const
{
    std::ostringstream s;
    s << "Probe (HW3 a): " << (bestanden() ? "bestanden" : "verfehlt") << "\n  (2) glatt " << glatt << " / " << gruppen << (glaetteHaelt() ? " — haelt" : " — verfehlt");
    if (!nichtGlatt.empty())
    {
        s << " (nicht glatt:";
        for (const auto& n : nichtGlatt) s << ' ' << n;
        s << ')';
    }
    s << "\n  (3) eindeutig " << eindeutig << " / " << messbar << (eindeutigGenug() ? " — haelt" : " — verfehlt");
    if (!nichtErreichbar.empty())
    {
        s << " (nicht erreichbar, zaehlen nicht:";
        for (const auto& n : nichtErreichbar) s << ' ' << n;
        s << ')';
    }
    s << "\n  hoerbar getroffen am besten Punkt " << hoerbarBest << ", an der Vorgabe " << hoerbarVorgabe
      << (besserAlsVorgabe() ? " — besser" : " — nicht besser") << '\n';
    return s.str();
}

}  // namespace lumi::messung::setzen
