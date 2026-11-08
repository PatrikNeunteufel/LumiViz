/**
 * @file   LevenbergMarquardt.cpp
 * @brief  Die Rechnung zu `InstrumentMeasure/LevenbergMarquardt.hpp` (Herleitung dort). Wortgleich aus dem Kopf der
 *         Unit-Tests gezogen (Instrumente_IC_Konzept.md, Teilplan 34e-1): in der .cpp, damit die Schalter nach
 *         E1 a (PRIVATE) gelten.
 */
#include "InstrumentMeasure/LevenbergMarquardt.hpp"

namespace lumi::messung
{

double quadratsumme(const std::vector<double>& r)
{
    double s = 0.0;
    for (const double v : r) s += v * v;
    return s;
}

std::vector<double> loese(Matrix a, std::vector<double> b)
{
    const std::size_t n = a.zeilen;
    std::vector<std::size_t> reihe(n);
    for (std::size_t i = 0; i < n; ++i) reihe[i] = i;
    std::vector<bool> frei(n, false);
    double groesst = 0.0;
    for (const double v : a.werte) groesst = std::max(groesst, std::abs(v));
    const double klein = groesst * 1e-14;
    for (std::size_t k = 0; k < n; ++k)
    {
        std::size_t p = k;
        for (std::size_t i = k + 1; i < n; ++i)
            if (std::abs(a(i, k)) > std::abs(a(p, k))) p = i;
        if (std::abs(a(p, k)) <= klein)
        {
            frei[k] = true;
            continue;
        }
        if (p != k)
        {
            for (std::size_t j = 0; j < n; ++j) std::swap(a(k, j), a(p, j));
            std::swap(b[k], b[p]);
        }
        for (std::size_t i = k + 1; i < n; ++i)
        {
            const double f = a(i, k) / a(k, k);
            if (f == 0.0) continue;
            for (std::size_t j = k; j < n; ++j) a(i, j) -= f * a(k, j);
            b[i] -= f * b[k];
        }
    }
    std::vector<double> x(n, 0.0);
    for (std::size_t k = n; k-- > 0;)
    {
        if (frei[k]) continue;
        double s = b[k];
        for (std::size_t j = k + 1; j < n; ++j) s -= a(k, j) * x[j];
        x[k] = s / a(k, k);
    }
    return x;
}

std::vector<double> diagonaleJtJ(const Matrix& j)
{
    std::vector<double> d(j.spalten, 0.0);
    for (std::size_t i = 0; i < j.zeilen; ++i)
        for (std::size_t k = 0; k < j.spalten; ++k) d[k] += j(i, k) * j(i, k);
    return d;
}

std::vector<double> lmSchritt(const Matrix& j, const std::vector<double>& r, double lambda)
{
    const std::size_t n = j.spalten;
    Matrix a(n, n);
    std::vector<double> g(n, 0.0);
    for (std::size_t i = 0; i < j.zeilen; ++i)
        for (std::size_t p = 0; p < n; ++p)
        {
            const double jp = j(i, p);
            if (jp == 0.0) continue;
            g[p] -= jp * r[i];
            for (std::size_t q = p; q < n; ++q) a(p, q) += jp * j(i, q);
        }
    double groesst = 0.0;
    for (std::size_t p = 0; p < n; ++p)
    {
        for (std::size_t q = 0; q < p; ++q) a(p, q) = a(q, p);
        groesst = std::max(groesst, a(p, p));
    }
    const double boden = groesst > 0.0 ? groesst * 1e-12 : 1e-12;
    for (std::size_t p = 0; p < n; ++p) a(p, p) += lambda * std::max(a(p, p), boden);
    return loese(std::move(a), std::move(g));
}

std::vector<double> lmSchritt(const Matrix& j, const std::vector<double>& r, double lambda, const std::vector<bool>& frei)
{
    std::vector<std::size_t> spalten;
    for (std::size_t p = 0; p < j.spalten; ++p)
        if (frei[p]) spalten.push_back(p);
    if (spalten.size() == j.spalten) return lmSchritt(j, r, lambda);
    std::vector<double> delta(j.spalten, 0.0);
    if (spalten.empty()) return delta;
    Matrix jf(j.zeilen, spalten.size());
    for (std::size_t i = 0; i < j.zeilen; ++i)
        for (std::size_t q = 0; q < spalten.size(); ++q) jf(i, q) = j(i, spalten[q]);
    const std::vector<double> d = lmSchritt(jf, r, lambda);
    for (std::size_t q = 0; q < spalten.size(); ++q) delta[spalten[q]] = d[q];
    return delta;
}

std::vector<double> klemmeIn(const LmRaum& raum, std::vector<double> x)
{
    for (std::size_t j = 0; j < x.size(); ++j) x[j] = std::clamp(x[j], raum.unten[j], raum.oben[j]);
    return x;
}

std::vector<std::size_t> anGrenzeIn(const LmRaum& raum, const std::vector<double>& x)
{
    std::vector<std::size_t> g;
    for (std::size_t j = 0; j < x.size(); ++j)
        if (x[j] <= raum.unten[j] || x[j] >= raum.oben[j]) g.push_back(j);
    return g;
}

LmErgebnis levenbergMarquardt(const LmResiduen& residuen, const LmJacobi& jacobi, const LmStart& start, const LmRaum& raum,
                                     const LmEinstellungen& einst, const LmBeobachter& beobachter)
{
    LmErgebnis e;
    e.x = klemmeIn(raum, start.x);
    if (start.r.empty())
    {
        e.r = residuen(e.x);
        ++e.auswertungen;
    }
    else
        e.r = start.r;
    e.rest = quadratsumme(e.r);
    e.lambda = start.lambda;
    e.iterationen = start.iteration;
    while (e.ende.empty())
    {
        if (e.iterationen >= einst.iterationenHoechstens)
        {
            e.ende = "Iterationen";
            break;
        }
        LmIteration it;
        it.nummer = ++e.iterationen;
        it.restVorher = e.rest;
        it.lambdaVorher = e.lambda;
        const Matrix j = jacobi(e.x, e.r);
        it.diagonale = diagonaleJtJ(j);
        bool klein = false;
        while (it.versuche < einst.versucheHoechstens)
        {
            const std::vector<double> delta = lmSchritt(j, e.r, e.lambda);
            std::vector<double> xn(e.x.size());
            for (std::size_t k = 0; k < xn.size(); ++k) xn[k] = e.x[k] + delta[k];
            xn = klemmeIn(raum, std::move(xn));
            double anteil = 0.0;
            for (std::size_t k = 0; k < xn.size(); ++k) anteil = std::max(anteil, std::abs(xn[k] - e.x[k]) / raum.ende[k]);
            it.schrittAnteil = anteil;
            // ME4 a (S177, wie IC setzen): auch der kleine Schritt wird ausgewertet — senkt er, wird er genommen; danach Ende
            if (anteil < 1.0) klein = true;
            std::vector<double> rn = residuen(xn);
            ++e.auswertungen;
            ++it.versuche;
            ++it.auswertungenResiduen;
            const double restNeu = quadratsumme(rn);
            if (restNeu < e.rest)
            {
                e.x = std::move(xn);
                e.r = std::move(rn);
                e.rest = restNeu;
                e.lambda /= einst.lambdaFaktor;
                it.angenommen = true;
                break;
            }
            ++it.verworfen;
            if (klein) break;
            e.lambda *= einst.lambdaFaktor;
        }
        if (klein) e.ende = "Schritte klein";
        else if (!it.angenommen) e.ende = "kein besserer Schritt";
        it.x = e.x;
        it.r = e.r;
        it.rest = e.rest;
        it.lambda = e.lambda;
        it.anGrenze = anGrenzeIn(raum, e.x);
        if (e.ende.empty() && e.iterationen >= einst.iterationenHoechstens) e.ende = "Iterationen";
        it.ende = e.ende;
        if (beobachter) beobachter(it);
    }
    e.anGrenze = anGrenzeIn(raum, e.x);
    return e;
}

std::vector<double> differenzSchritte(const LmRaum& raum, const std::vector<double>& x, const std::vector<double>& mass)
{
    std::vector<double> h(x.size());
    for (std::size_t j = 0; j < x.size(); ++j) h[j] = x[j] + mass[j] <= raum.oben[j] ? mass[j] : -mass[j];
    return h;
}

Matrix jacobiJeGruppe(const LmResiduenViele& viele, const std::vector<double>& x, const std::vector<double>& r,
                             const std::vector<std::vector<std::size_t>>& gruppen, const std::vector<double>& h,
                             const std::vector<std::vector<std::size_t>>& zeilen)
{
    std::vector<std::vector<double>> punkte;
    for (const std::vector<std::size_t>& g : gruppen)
    {
        std::vector<double> p = x;
        for (const std::size_t j : g) p[j] += h[j];
        punkte.push_back(std::move(p));
    }
    const std::vector<std::vector<double>> rs = viele(punkte);
    Matrix jm(r.size(), x.size());
    for (std::size_t gi = 0; gi < gruppen.size(); ++gi)
        for (const std::size_t j : gruppen[gi])
            for (const std::size_t i : zeilen[j]) jm(i, j) = (rs[gi][i] - r[i]) / h[j];
    return jm;
}

Matrix jacobiEinzeln(const LmResiduenViele& viele, const std::vector<double>& x, const std::vector<double>& r, const std::vector<double>& h)
{
    std::vector<std::vector<double>> punkte;
    for (std::size_t j = 0; j < x.size(); ++j)
    {
        std::vector<double> p = x;
        p[j] += h[j];
        punkte.push_back(std::move(p));
    }
    const std::vector<std::vector<double>> rs = viele(punkte);
    Matrix jm(r.size(), x.size());
    for (std::size_t j = 0; j < x.size(); ++j)
        for (std::size_t i = 0; i < r.size(); ++i) jm(i, j) = (rs[j][i] - r[i]) / h[j];
    return jm;
}

LmErgebnisBloecke levenbergMarquardtJeBlock(const LmResiduenBloecke& residuen, const std::vector<LmBlock>& bloecke,
                                                   const std::vector<std::vector<std::size_t>>& gruppen, const std::vector<double>& differenz,
                                                   const LmStartBloecke& start, const LmRaum& raum, const LmEinstellungen& einst,
                                                   const LmBeobachterBloecke& beobachter)
{
    const std::size_t nb = bloecke.size();
    LmErgebnisBloecke e;
    e.x = klemmeIn(raum, start.x);
    e.iterationen = start.nummer;
    e.bloecke.resize(nb);
    std::vector<std::size_t> blockVon(e.x.size(), nb);
    std::vector<std::size_t> spalteVon(e.x.size(), 0);
    for (std::size_t b = 0; b < nb; ++b)
    {
        for (std::size_t q = 0; q < bloecke[b].parameter.size(); ++q)
        {
            blockVon[bloecke[b].parameter[q]] = b;
            spalteVon[bloecke[b].parameter[q]] = q;
        }
        LmBlockStand& st = e.bloecke[b];
        st.lambda = start.lambda.empty() ? einst.lambdaStart : start.lambda[b];
        st.iterationen = start.iteration.empty() ? 0 : start.iteration[b];
        st.ende = start.ende.empty() ? std::string() : start.ende[b];
    }
    {
        std::vector<std::size_t> alle(nb);
        for (std::size_t b = 0; b < nb; ++b) alle[b] = b;
        const std::vector<std::vector<double>> rs = residuen({e.x}, alle);
        e.auswertungen += static_cast<int>(nb);
        for (std::size_t b = 0; b < nb; ++b)
        {
            e.bloecke[b].r = rs[b];
            e.bloecke[b].rest = quadratsumme(rs[b]);
            ++e.bloecke[b].auswertungen;
        }
    }
    while (true)
    {
        std::vector<std::size_t> aktiv;
        std::vector<std::size_t> aktivIndex(nb, nb);
        for (std::size_t b = 0; b < nb; ++b)
        {
            LmBlockStand& st = e.bloecke[b];
            if (st.ende.empty() && st.iterationen >= einst.iterationenHoechstens) st.ende = "Iterationen";
            if (st.ende.empty())
            {
                aktivIndex[b] = aktiv.size();
                aktiv.push_back(b);
            }
        }
        if (aktiv.empty()) break;
        LmIterationBloecke it;
        it.nummer = ++e.iterationen;
        it.bloecke.resize(nb);
        std::vector<bool> parameterAktiv(e.x.size(), false);
        for (const std::size_t b : aktiv)
        {
            for (const std::size_t j : bloecke[b].parameter) parameterAktiv[j] = true;
            LmBlockStand& st = e.bloecke[b];
            LmBlockSchritt& s = it.bloecke[b];
            s.aktiv = true;
            s.restVorher = st.rest;
            s.lambdaVorher = st.lambda;
            ++st.iterationen;
        }

        // (1) die Jacobi-Matrix je aktivem Block: eine Auswertung je Gruppe, alle Gruppen in einem Aufruf
        const std::vector<double> h = differenzSchritte(raum, e.x, differenz);
        std::vector<std::vector<double>> punkte;
        std::vector<std::size_t> gruppeVon;
        for (std::size_t g = 0; g < gruppen.size(); ++g)
        {
            std::vector<double> p = e.x;
            bool ausgelenkt = false;
            for (const std::size_t j : gruppen[g])
                if (parameterAktiv[j])
                {
                    p[j] += h[j];
                    ausgelenkt = true;
                }
            if (!ausgelenkt) continue;
            punkte.push_back(std::move(p));
            gruppeVon.push_back(g);
        }
        std::vector<Matrix> jb(nb);
        for (const std::size_t b : aktiv) jb[b] = Matrix(e.bloecke[b].r.size(), bloecke[b].parameter.size());
        if (!punkte.empty())
        {
            const std::vector<std::vector<double>> rs = residuen(punkte, aktiv);
            it.auswertungen += static_cast<int>(punkte.size() * aktiv.size());
            for (std::size_t pi = 0; pi < punkte.size(); ++pi)
                for (const std::size_t j : gruppen[gruppeVon[pi]])
                {
                    if (!parameterAktiv[j]) continue;
                    const std::size_t b = blockVon[j];
                    const std::vector<double>& rr = rs[pi * aktiv.size() + aktivIndex[b]];
                    const std::vector<double>& r0 = e.bloecke[b].r;
                    for (std::size_t i = 0; i < r0.size(); ++i) jb[b](i, spalteVon[j]) = (rr[i] - r0[i]) / h[j];
                }
        }

        // A3: Parameter ohne Wirkung bekommen keinen Schritt
        std::vector<std::vector<bool>> frei(nb);
        for (const std::size_t b : aktiv)
        {
            LmBlockSchritt& s = it.bloecke[b];
            s.diagonale = diagonaleJtJ(jb[b]);
            frei[b].assign(bloecke[b].parameter.size(), true);
            for (std::size_t q = 0; q < bloecke[b].parameter.size(); ++q)
            {
                const std::size_t j = bloecke[b].parameter[q];
                if (std::sqrt(s.diagonale[q]) * (raum.oben[j] - raum.unten[j]) < einst.wirkungMindestens)
                {
                    frei[b][q] = false;
                    s.ohneWirkung.push_back(j);
                }
            }
        }

        // (2) die Versuche: jeder noch nicht entschiedene Block mit seinem λ, alle in einem gemeinsamen Punkt
        std::vector<bool> entschieden(nb, false);
        std::vector<bool> klein(nb, false);
        while (true)
        {
            std::vector<double> xv = e.x;
            std::vector<std::size_t> versuch;
            for (const std::size_t b : aktiv)
            {
                if (entschieden[b]) continue;
                LmBlockSchritt& s = it.bloecke[b];
                if (s.versuche >= einst.versucheHoechstens)
                {
                    entschieden[b] = true;
                    continue;
                }
                const LmBlockStand& st = e.bloecke[b];
                const std::vector<double> delta = lmSchritt(jb[b], st.r, st.lambda, frei[b]);
                double anteil = 0.0;
                for (std::size_t q = 0; q < delta.size(); ++q)
                {
                    const std::size_t j = bloecke[b].parameter[q];
                    xv[j] = std::clamp(e.x[j] + delta[q], raum.unten[j], raum.oben[j]);
                    anteil = std::max(anteil, std::abs(xv[j] - e.x[j]) / raum.ende[j]);
                }
                s.schrittAnteil = anteil;
                // ME4 a (S177, wie IC setzen): auch der kleine Schritt wird ausgewertet — senkt er, wird er genommen; danach Ende
                if (anteil < 1.0) klein[b] = true;
                versuch.push_back(b);
            }
            if (versuch.empty()) break;
            const std::vector<std::vector<double>> rv = residuen({xv}, versuch);
            it.auswertungen += static_cast<int>(versuch.size());
            for (std::size_t k = 0; k < versuch.size(); ++k)
            {
                const std::size_t b = versuch[k];
                LmBlockStand& st = e.bloecke[b];
                LmBlockSchritt& s = it.bloecke[b];
                ++st.auswertungen;
                ++s.versuche;
                const double restNeu = quadratsumme(rv[k]);
                if (restNeu < st.rest)
                {
                    for (const std::size_t j : bloecke[b].parameter) e.x[j] = xv[j];
                    st.r = rv[k];
                    st.rest = restNeu;
                    st.lambda /= einst.lambdaFaktor;
                    s.angenommen = true;
                    entschieden[b] = true;
                    continue;
                }
                ++s.verworfen;
                if (klein[b])
                {
                    entschieden[b] = true;
                    continue;
                }
                st.lambda *= einst.lambdaFaktor;
            }
        }

        // (3) das Ende je Block
        for (const std::size_t b : aktiv)
        {
            LmBlockStand& st = e.bloecke[b];
            LmBlockSchritt& s = it.bloecke[b];
            if (klein[b]) st.ende = "Schritte klein";
            else if (!s.angenommen) st.ende = "kein besserer Schritt";
            if (st.ende.empty() && st.iterationen >= einst.iterationenHoechstens) st.ende = "Iterationen";
            s.rest = st.rest;
            s.lambda = st.lambda;
            s.ende = st.ende;
            st.diagonale = s.diagonale;
            st.ohneWirkung = s.ohneWirkung;
        }
        e.auswertungen += it.auswertungen;
        it.x = e.x;
        if (beobachter) beobachter(it);
    }
    e.anGrenze = anGrenzeIn(raum, e.x);
    return e;
}

}  // namespace lumi::messung
