/**
 * @file EigenloeserSpectra.hpp
 * @brief Die erste Umsetzung des Eigenloesers: Spectra 1.2.0 (Lanczos mit implizitem
 *        Neustart, generalisiert, Shift-Invert) und Eigen 3.4.1 (`SimplicialLDLT`)
 *        (docs/audio/Spielweisen_Konzept.md §4.1; Entscheide Patrik S147/S148: BM5-1 a,
 *        EG1 a, EG2 a; Stufe 31c-4)
 *
 * **Zerlegung (EG2 a):** ein eigener Operator fuer `SymGEigsShiftSolver` zerlegt K − σM
 * mit `SimplicialLDLT` (AMD-Ordnung, das Muster einmal analysiert — K und M haben dasselbe).
 * Aus D zaehlt er die negativen Eintraege: so viele Eigenwerte liegen unter σ (Sylvester).
 *
 * **Fenster in Teilfenstern (EG1 a):** von unten nach oben, je Schritt das Stueck [P, σ)
 * zwischen dem letzten Shift P und dem neuen σ. Die Zaehlungen an P und σ sagen genau, wie
 * viele Eigenwerte darin liegen. Spectra holt die naechsten um σ; fehlen noch welche, laeuft
 * es noch einmal, die schon gefundenen Formen gesperrt (Deflation: der Operator projiziert
 * sie bezueglich M heraus). So kommen auch beide Formen eines doppelten Eigenwerts — ein
 * Lanczos mit einem Startvektor saehe nur eine. Geliefert wird ein Stueck erst, wenn die
 * Zahl stimmt; was ueber σ schon gefunden ist, wartet auf das naechste Stueck.
 *
 * **Rest je Mode (EG2 a):** ‖Kφ − λMφ‖ / ‖λMφ‖ ueber `restGrenze` ist ein Fehler, nicht
 * eine stille Mode (`SimplicialLDLT` sucht keinen Pivot).
 *
 * **Nachschaerfen (RS1 a, S151):** Die Toleranz von Spectra gilt fuer ν = 1/(λ − σ); eine Mode
 * weit unter σ traegt im eigentlichen Problem einen um rund σ/λ groesseren Rest (beim steifen
 * MAESSTRO-Fluegel MP1 liegen die Shifts weit auseinander). Liegt der Rest ueber der Grenze, wird
 * die Mode mit einem Shift knapp unter ihrem Eigenwert inverse iteriert, die uebrigen Formen des
 * Stuecks gesperrt; erst wenn auch das nicht reicht, ein Fehler. Eine Mode unter der Grenze bleibt,
 * wie sie ist — ein Koerper ohne solche Moden rechnet Bit fuer Bit wie vorher.
 *
 * Kein Typ von Eigen oder Spectra geht ueber die Schnittstelle (BM5-1 a).
 */
#pragma once

#include "InstrumentBody/Eigenloeser.hpp"

#include <Eigen/SparseCholesky>
#include <Eigen/SparseCore>
#include <Spectra/MatOp/SparseSymMatProd.h>
#include <Spectra/SymGEigsShiftSolver.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace lumi::koerper
{

namespace spectra_detail
{

using Duenn = Eigen::SparseMatrix<double, Eigen::ColMajor, int>;

/// Das obere Dreieck zeilenweise ist das untere spaltenweise.
inline Duenn alsUnteresDreieck(const DuenneMatrix& a)
{
    std::vector<int> start(a.start.size());  // passt: `loese` prueft vorher, dass die Eintraege in int passen
    for (std::size_t i = 0; i < start.size(); ++i) start[i] = static_cast<int>(a.start[i]);
    const Eigen::Map<const Duenn> karte(a.n, a.n, static_cast<Eigen::Index>(a.werte.size()), start.data(), a.spalten.data(),
                                        a.werte.data());
    return Duenn(karte);
}

/// (K − σM)⁻¹ x fuer Spectra, mit der Zaehlung der Eigenwerte unter σ und gesperrten Formen.
class LdltShiftInvert
{
public:
    using Scalar = double;

    LdltShiftInvert(const Duenn& k, const Duenn& m) : m_k(k), m_m(m), m_a(k)
    {
        const auto beginn = std::chrono::steady_clock::now();
        m_ldlt.analyzePattern(m_a);
        m_sekunden = std::chrono::duration<double>(std::chrono::steady_clock::now() - beginn).count();
    }

    [[nodiscard]] Eigen::Index rows() const { return m_a.rows(); }
    [[nodiscard]] Eigen::Index cols() const { return m_a.cols(); }

    /// Zerlegt K − σM (derselbe Shift ein zweites Mal: nichts zu tun). Wirft, wenn ein Pivot
    /// genau 0 ist — σ liegt dann auf einem Eigenwert.
    void set_shift(double sigma)
    {
        if (m_gueltig && sigma == m_sigma) return;
        m_gueltig = false;
        const double* k = m_k.valuePtr();
        const double* m = m_m.valuePtr();
        double* a = m_a.valuePtr();
        for (Eigen::Index i = 0; i < m_a.nonZeros(); ++i) a[i] = k[i] - sigma * m[i];
        const auto beginn = std::chrono::steady_clock::now();
        m_ldlt.factorize(m_a);
        m_sekunden += std::chrono::duration<double>(std::chrono::steady_clock::now() - beginn).count();
        ++m_faktorisierungen;
        if (m_ldlt.info() != Eigen::Success) throw std::runtime_error("factorization failed at a shift");
        m_unter = static_cast<int>((m_ldlt.vectorD().array() < 0.0).count());
        m_sigma = sigma;
        m_gueltig = true;
    }

    /// y = P (K − σM)⁻¹ x mit P = I − Φ (MΦ)ᵀ: die gesperrten Formen (M-orthonormal) fallen heraus.
    void perform_op(const double* x, double* y) const
    {
        const Eigen::Map<const Eigen::VectorXd> ein(x, rows());
        Eigen::Map<Eigen::VectorXd> aus(y, rows());
        aus = m_ldlt.solve(ein);
        if (m_phi.cols() > 0) aus -= m_phi * (m_mphi.transpose() * aus);
    }

    /// Sperrt diese Formen (Spalten, M-orthonormal; `mphi` = M · `phi`); leer = keine.
    void sperre(Eigen::MatrixXd phi, Eigen::MatrixXd mphi)
    {
        m_phi = std::move(phi);
        m_mphi = std::move(mphi);
    }

    /// Eigenwerte unter dem letzten Shift.
    [[nodiscard]] int unter() const { return m_unter; }
    [[nodiscard]] int faktorisierungen() const { return m_faktorisierungen; }
    /// Sekunden in Zerlegungen, die Analyse des Musters eingeschlossen.
    [[nodiscard]] double sekunden() const { return m_sekunden; }

private:
    const Duenn& m_k;
    const Duenn& m_m;
    Duenn m_a;
    Eigen::SimplicialLDLT<Duenn, Eigen::Lower> m_ldlt;
    Eigen::MatrixXd m_phi, m_mphi;
    double m_sigma = 0.0;
    double m_sekunden = 0.0;
    bool m_gueltig = false;
    int m_unter = 0;
    int m_faktorisierungen = 0;
};

/// Eine gefundene, noch nicht gelieferte Mode (Form M-normiert).
struct Gefunden
{
    double lambda = 0.0;
    Eigen::VectorXd phi;
};

}  // namespace spectra_detail

class EigenloeserSpectra final : public Eigenloeser
{
public:
    int modenJeSchritt = 40;    ///< so viele neue Eigenwerte soll ein Shift unter sich haben (Ziel)
    double restGrenze = 1e-6;   ///< groesster erlaubter Rest je Mode
    double nachschaerfAbstand = 1e-6;  ///< Nachschaerfen (RS1 a): der Shift bei λ · (1 − dies)
    int nachschaerfSchritte = 10;      ///< hoechstens so viele Schritte der inversen Iteration je Mode

    [[nodiscard]] std::string name() const override { return "Spectra 1.2.0 + Eigen SimplicialLDLT"; }

    [[nodiscard]] EigenErgebnis loese(const EigenAuftrag& a) const override
    {
        const auto beginn = std::chrono::steady_clock::now();
        EigenErgebnis erg;
        erg.fehler = pruefeAuftrag(a);
        if (erg.fehler.empty() && (a.k->start != a.m->start || a.k->spalten != a.m->spalten))
            erg.fehler = "stiffness and mass matrix need the same pattern";
        if (erg.fehler.empty() && a.k->eintraege() > std::numeric_limits<int>::max())
            erg.fehler = "too many entries for this solver";
        if (erg.fehler.empty())
        {
            try
            {
                rechne(a, erg);
            }
            catch (const std::exception& e)
            {
                erg.fehler = e.what();
            }
        }
        erg.sekunden = std::chrono::duration<double>(std::chrono::steady_clock::now() - beginn).count();
        return erg;
    }

private:
    void rechne(const EigenAuftrag& a, EigenErgebnis& erg) const
    {
        using namespace spectra_detail;
        using BOp = Spectra::SparseSymMatProd<double, Eigen::Lower, Eigen::ColMajor, int>;
        using Loeser = Spectra::SymGEigsShiftSolver<LdltShiftInvert, BOp, Spectra::GEigsMode::ShiftInvert>;

        const Duenn k = alsUnteresDreieck(*a.k);
        const Duenn m = alsUnteresDreieck(*a.m);
        const int n = a.k->n;
        LdltShiftInvert op(k, m);
        BOp bop(m);

        // zaehlt die Eigenwerte unter σ; liegt σ genau auf einem, wird es knapp verschoben
        const auto zaehle = [&](double& sigma) {
            for (int versuch = 0;; ++versuch)
            {
                try
                {
                    op.set_shift(sigma);
                    erg.faktorisierungen = op.faktorisierungen(), erg.sekundenZerlegung = op.sekunden();
                    return op.unter();
                }
                catch (const std::runtime_error&)
                {
                    erg.faktorisierungen = op.faktorisierungen(), erg.sekundenZerlegung = op.sekunden();
                    if (versuch >= 3) throw;
                    sigma *= 1.0 + 1e-9;
                }
            }
        };

        double la = eigenwertVon(a.vonHz), lb = eigenwertVon(a.bisHz);
        const int nuA = la > 0.0 ? zaehle(la) : 0;  // K positiv definit: unter 0 liegt nichts
        const int nuB = zaehle(lb);
        erg.imFenster = nuB - nuA;
        if (erg.imFenster < 0) throw std::runtime_error("the count below the window exceeds the count above it");
        if (erg.imFenster == 0) return;
        if (erg.imFenster >= n - 1) throw std::runtime_error("the window holds (nearly) every mode — use a direct solver");

        const int ziel = std::max(4, modenJeSchritt);
        double lP = la;   // der letzte Shift: alles unter ihm ist geliefert
        int nuP = nuA;
        std::vector<Gefunden> offen;  // ueber P gefunden, noch nicht geliefert, aufsteigend
        double dichte = erg.imFenster / (a.bisHz - a.vonHz);  // Moden je Hz, bei der Platte etwa gleichmaessig
        int geliefert = 0;
        Eigen::VectorXd mphi(n), kphi(n);

        for (int schritt = 0; geliefert < erg.imFenster; ++schritt)
        {
            if (schritt > 100000) throw std::runtime_error("too many steps");
            const double fP = frequenzVon(lP);
            const int p = static_cast<int>(offen.size());

            // der Shift: rund `ziel` neue Eigenwerte zwischen den offenen und σ
            double fS = std::min(a.bisHz, fP + (p + ziel) / std::max(dichte, 1e-12));
            double sigma = fS >= a.bisHz ? lb : eigenwertVon(fS);
            int nu = 0, c = 0;
            for (int anpassen = 0;; ++anpassen)
            {
                nu = zaehle(sigma);
                c = nu - nuP;
                if (c < 0) throw std::runtime_error("the count went down between two shifts");
                if (c <= p + 2 * ziel || anpassen >= 20) break;
                fS = fP + (fS - fP) * (p + ziel) / c;  // zu viele: naeher heran
                sigma = eigenwertVon(fS);
            }
            if (c > p + 2 * ziel) throw std::runtime_error("could not place a shift");
            const bool letzter = sigma >= lb;

            // die Moden in [P, σ): die offenen, dazu so lange Laeufe um σ mit gesperrten Formen, bis die Zahl stimmt
            std::vector<Gefunden> gefunden = std::move(offen);
            offen.clear();
            const auto imStueck = [&] {
                return static_cast<int>(std::count_if(gefunden.begin(), gefunden.end(),
                                                      [&](const Gefunden& g) { return g.lambda >= lP && g.lambda < sigma; }));
            };
            int extra = std::max(4, (c + 1) / 2);
            for (int versuch = 0;; ++versuch)
            {
                const int da = imStueck();
                if (da == c) break;
                if (da > c) throw std::runtime_error("more modes between two shifts than counted");
                if (versuch >= 8) throw std::runtime_error("could not find every mode between two shifts");

                const int gesperrt = static_cast<int>(gefunden.size());
                Eigen::MatrixXd phi(n, gesperrt), mphiG(n, gesperrt);
                for (int j = 0; j < gesperrt; ++j)
                {
                    phi.col(j) = gefunden[static_cast<std::size_t>(j)].phi;
                    mphiG.col(j) = m.selfadjointView<Eigen::Lower>() * phi.col(j);
                }
                // der Startvektor fest (gleich auf jedem Compiler), ohne die gesperrten Formen
                Eigen::VectorXd startvektor(n);
                for (int i = 0; i < n; ++i)
                    startvektor(i) = static_cast<double>((static_cast<std::uint64_t>(i) * 2654435761u + 12345u) % 1000003u) / 1000003.0 - 0.5;
                if (gesperrt > 0) startvektor -= phi * (mphiG.transpose() * startvektor);
                op.sperre(std::move(phi), std::move(mphiG));
                const int frei = n - gesperrt;
                const int nev = std::min(frei - 1, c - da + extra);
                if (nev < 1) throw std::runtime_error("no room left for the missing modes");
                const int ncv = std::min(frei, std::max(2 * nev + 1, nev + 20));
                Loeser loeser(op, bop, nev, ncv, sigma);
                loeser.init(startvektor.data());
                loeser.compute(Spectra::SortRule::LargestMagn, 1000, a.toleranz, Spectra::SortRule::SmallestAlge);
                erg.faktorisierungen = op.faktorisierungen(), erg.sekundenZerlegung = op.sekunden();
                const Eigen::VectorXd werte = loeser.eigenvalues();  // nur die konvergierten
                const Eigen::MatrixXd formen = loeser.eigenvectors();
                int neuImStueck = 0;
                for (Eigen::Index i = 0; i < werte.size(); ++i)
                {
                    // gesperrte Richtungen kaemen mit ν ≈ 0, also λ weit weg — nur [P, b) zaehlt
                    if (!(werte(i) >= lP && werte(i) < lb)) continue;
                    Gefunden g;
                    g.lambda = werte(i);
                    g.phi = formen.col(i);
                    mphi = m.selfadjointView<Eigen::Lower>() * g.phi;
                    g.phi /= std::sqrt(g.phi.dot(mphi));
                    if (g.lambda < sigma) ++neuImStueck;
                    gefunden.push_back(std::move(g));
                }
                if (neuImStueck == 0) extra *= 2;
            }
            op.sperre({}, {});
            std::sort(gefunden.begin(), gefunden.end(), [](const Gefunden& x, const Gefunden& y) { return x.lambda < y.lambda; });

            const auto restVon = [&](const Gefunden& g) {
                mphi = m.selfadjointView<Eigen::Lower>() * g.phi;
                kphi = k.selfadjointView<Eigen::Lower>() * g.phi;
                return (kphi - g.lambda * mphi).norm() / (std::abs(g.lambda) * mphi.norm());
            };
            // RS1 a: inverse Iteration mit dem Shift knapp unter λ, die uebrigen Formen des Stuecks gesperrt
            const auto nachschaerfen = [&](std::size_t i) {
                const int andere = static_cast<int>(gefunden.size()) - 1;
                Eigen::MatrixXd phi(n, andere), mphiG(n, andere);
                for (int j = 0, s = 0; j <= andere; ++j)
                {
                    if (static_cast<std::size_t>(j) == i) continue;
                    phi.col(s) = gefunden[static_cast<std::size_t>(j)].phi;
                    mphiG.col(s) = m.selfadjointView<Eigen::Lower>() * phi.col(s);
                    ++s;
                }
                op.sperre(std::move(phi), std::move(mphiG));
                Gefunden& g = gefunden[i];
                double shift = g.lambda * (1.0 - nachschaerfAbstand);
                zaehle(shift);
                double rest = restVon(g);
                Eigen::VectorXd y(n);
                for (int schritt = 0; schritt < nachschaerfSchritte && !(rest <= restGrenze); ++schritt)
                {
                    const Eigen::VectorXd ein = m.selfadjointView<Eigen::Lower>() * g.phi;
                    op.perform_op(ein.data(), y.data());
                    mphi = m.selfadjointView<Eigen::Lower>() * y;
                    g.phi = y / std::sqrt(y.dot(mphi));
                    g.lambda = g.phi.dot(k.selfadjointView<Eigen::Lower>() * g.phi);
                    rest = restVon(g);
                }
                op.sperre({}, {});
                erg.faktorisierungen = op.faktorisierungen(), erg.sekundenZerlegung = op.sekunden();
                if (!(g.lambda >= lP && g.lambda < sigma))
                    throw std::runtime_error("a sharpened mode at " + std::to_string(frequenzVon(g.lambda)) + " Hz left its piece");
                ++erg.nachgeschaerft;  // haelt sie die Grenze noch immer nicht, meldet das die Lieferung
            };

            // erst nachschaerfen, was im Stueck ueber der Grenze liegt (die Formen aller noch gesperrt), dann neu ordnen
            bool geschaerft = false;
            for (std::size_t i = 0; i < gefunden.size(); ++i)
            {
                if (gefunden[i].lambda >= sigma || restVon(gefunden[i]) <= restGrenze) continue;
                nachschaerfen(i);
                geschaerft = true;
            }
            if (geschaerft)
                std::sort(gefunden.begin(), gefunden.end(), [](const Gefunden& x, const Gefunden& y) { return x.lambda < y.lambda; });

            // liefern: [P, σ), aufsteigend; der Rest wartet
            for (auto& g : gefunden)
            {
                if (g.lambda >= sigma)
                {
                    if (!letzter) offen.push_back(std::move(g));
                    continue;
                }
                const double rest = restVon(g);
                erg.groessterRest = std::max(erg.groessterRest, rest);
                if (!(rest <= restGrenze))
                {
                    char text[160];
                    std::snprintf(text, sizeof text, "the residual of a mode at %.6f Hz is %.3e, above the limit %.1e", frequenzVon(g.lambda),
                                  rest, restGrenze);
                    throw std::runtime_error(text);
                }
                erg.frequenzenHz.push_back(frequenzVon(g.lambda));
                if (a.jeMode)
                {
                    Mode mode;
                    mode.frequenzHz = erg.frequenzenHz.back();
                    mode.form.assign(g.phi.data(), g.phi.data() + n);
                    a.jeMode(mode);
                }
            }
            geliefert += c;

            const double fSigma = frequenzVon(sigma);
            if (fSigma > fP) dichte = std::max(1e-12, c / (fSigma - fP));
            lP = sigma;
            nuP = nu;
            if (a.weiter && !a.weiter(static_cast<double>(geliefert) / erg.imFenster)) throw std::runtime_error("cancelled");
            if (letzter && geliefert != erg.imFenster) throw std::runtime_error("the modes do not add up to the count of the window");
        }
    }
};

}  // namespace lumi::koerper
