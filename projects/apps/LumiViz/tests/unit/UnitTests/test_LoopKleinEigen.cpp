/**
 ****************************************************************************************
 * @file   test_LoopKleinEigen.cpp
 * @brief  Loop, Stufe 31d-7: der kleine komplexe Eigenloeser (`loop/Eigenzerlegung`) gegen
 *         Eigen und gegen die Moden des Chors (docs/audio/Spielweisen_Konzept.md §4.1,
 *         Entscheide Patrik S153: P6 a, P8 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Qt-Anwendung, ohne Dateien. Eigen ist nur hier gebunden, nicht an die App.
 *
 * - Eine diagonale Matrix bleibt Bit fuer Bit, wie sie ist.
 * - Zufaellige Matrizen 1 … 8: Eigenwerte wie Eigen, Residuum, links · rechts = Einheit.
 * - Der Chor in einer Ebene: dieselben Eigenwerte und Anteile wie `chorModen`.
 * - Zwei Ebenen ohne Verstimmung (zusammenfallende Eigenwerte) und eng verstimmt: die
 *   Uebertragung c·(z − A)⁻¹·b aus den Moden, die Projektoren der isolierten Eigenwerte wie Eigen.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Eigenzerlegung.hpp"
#include "loop/HammerTon.hpp"

#include <Eigen/Dense>

#include <algorithm>
#include <cmath>
#include <complex>
#include <numbers>
#include <random>
#include <vector>

using namespace lumi::loop;

namespace
{

using Komplex = std::complex<double>;

double normF(const KleineMatrix& a)
{
    double summe = 0.0;
    for (const Komplex& w : a.werte) summe += std::norm(w);
    return std::sqrt(summe);
}

Eigen::MatrixXcd alsEigen(const KleineMatrix& a)
{
    Eigen::MatrixXcd m(a.n, a.n);
    for (int z = 0; z < a.n; ++z)
        for (int s = 0; s < a.n; ++s) m(z, s) = a(z, s);
    return m;
}

/// Groesster Betrag von A·v − λ·v ueber alle Eigenpaare.
double residuum(const KleineMatrix& a, const Eigenzerlegung& z)
{
    double groesstes = 0.0;
    for (int m = 0; m < a.n; ++m)
        for (int r = 0; r < a.n; ++r)
        {
            Komplex summe(0.0, 0.0);
            for (int s = 0; s < a.n; ++s) summe += a(r, s) * z.rechts(s, m);
            groesstes = std::max(groesstes, std::abs(summe - z.werte[static_cast<std::size_t>(m)] * z.rechts(r, m)));
        }
    return groesstes;
}

/// Groesste Abweichung von links · rechts gegen die Einheit.
double gegenEinheit(const Eigenzerlegung& z)
{
    const int n = z.rechts.n;
    double groesste = 0.0;
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
        {
            Komplex summe(0.0, 0.0);
            for (int k = 0; k < n; ++k) summe += z.links(i, k) * z.rechts(k, j);
            groesste = std::max(groesste, std::abs(summe - (i == j ? Komplex(1.0, 0.0) : Komplex(0.0, 0.0))));
        }
    return groesste;
}

/// Die Eigenwerte wie Eigen: jeder eigene dem naechsten noch freien von Eigen zugeordnet (Sortieren
/// taeugt nicht — bei gleichem Imaginaerteil vertauscht das Rauschen die Reihenfolge).
double gegenEigenWerte(const KleineMatrix& a, const Eigenzerlegung& z)
{
    Eigen::ComplexEigenSolver<Eigen::MatrixXcd> loeser(alsEigen(a));
    REQUIRE(loeser.info() == Eigen::Success);
    std::vector<Komplex> bezug(loeser.eigenvalues().data(), loeser.eigenvalues().data() + a.n);
    std::vector<bool> frei(bezug.size(), true);
    double groesste = 0.0;
    for (const Komplex& eigener : z.werte)
    {
        std::size_t beste = bezug.size();
        for (std::size_t i = 0; i < bezug.size(); ++i)
            if (frei[i] && (beste == bezug.size() || std::abs(bezug[i] - eigener) < std::abs(bezug[beste] - eigener))) beste = i;
        REQUIRE(beste < bezug.size());
        frei[beste] = false;
        groesste = std::max(groesste, std::abs(bezug[beste] - eigener));
    }
    return groesste;
}

/// c·(z − A)⁻¹·b aus den Moden: Σ (c·v_m)(w_m·b) / (z − λ_m).
Komplex uebertragungModal(const Eigenzerlegung& zer, const std::vector<Komplex>& c, const std::vector<Komplex>& b, Komplex z)
{
    const int n = zer.rechts.n;
    Komplex summe(0.0, 0.0);
    for (int m = 0; m < n; ++m)
    {
        Komplex aus(0.0, 0.0);
        Komplex ein(0.0, 0.0);
        for (int i = 0; i < n; ++i)
        {
            aus += c[static_cast<std::size_t>(i)] * zer.rechts(i, m);
            ein += zer.links(m, i) * b[static_cast<std::size_t>(i)];
        }
        summe += aus * ein / (z - zer.werte[static_cast<std::size_t>(m)]);
    }
    return summe;
}

/// Dasselbe direkt geloest (Eigen).
Komplex uebertragungDirekt(const KleineMatrix& a, const std::vector<Komplex>& c, const std::vector<Komplex>& b, Komplex z)
{
    const Eigen::MatrixXcd m = Eigen::MatrixXcd::Identity(a.n, a.n) * z - alsEigen(a);
    Eigen::VectorXcd bv(a.n);
    Eigen::VectorXcd cv(a.n);
    for (int i = 0; i < a.n; ++i)
    {
        bv(i) = b[static_cast<std::size_t>(i)];
        cv(i) = c[static_cast<std::size_t>(i)];
    }
    const Eigen::VectorXcd x = m.fullPivLu().solve(bv);
    return cv.transpose() * x;
}

/// Die Matrix eines Teiltons mit N Saiten in zwei Ebenen (P2 a): Saite j, Ebene a = 0, 1 an
/// Stelle 2j + a; die erste Eigenrichtung um γ gegen die Senkrechte gedreht; der Steg nimmt
/// senkrecht σ_V und waagrecht σ_H auf die Summe.
KleineMatrix zweiEbenen(const std::vector<double>& omega, double delta, double gamma, double sigmaSaite, double sigmaV,
                        double sigmaH)
{
    const int n = static_cast<int>(omega.size());
    KleineMatrix a(2 * n);
    std::vector<double> senk(static_cast<std::size_t>(2 * n));
    std::vector<double> waag(static_cast<std::size_t>(2 * n));
    for (int j = 0; j < n; ++j)
    {
        a(2 * j, 2 * j) = Komplex(-sigmaSaite, omega[static_cast<std::size_t>(j)]);
        a(2 * j + 1, 2 * j + 1) = Komplex(-sigmaSaite, omega[static_cast<std::size_t>(j)] * (1.0 + delta));
        senk[static_cast<std::size_t>(2 * j)] = std::cos(gamma);
        senk[static_cast<std::size_t>(2 * j + 1)] = -std::sin(gamma);
        waag[static_cast<std::size_t>(2 * j)] = std::sin(gamma);
        waag[static_cast<std::size_t>(2 * j + 1)] = std::cos(gamma);
    }
    for (int z = 0; z < 2 * n; ++z)
        for (int s = 0; s < 2 * n; ++s)
            a(z, s) -= sigmaV * senk[static_cast<std::size_t>(z)] * senk[static_cast<std::size_t>(s)] +
                       sigmaH * waag[static_cast<std::size_t>(z)] * waag[static_cast<std::size_t>(s)];
    return a;
}

/// Der senkrechte Anteil je Stelle (Anregung und Abnahme am Steg).
std::vector<Komplex> senkrecht(int saiten, double gamma)
{
    std::vector<Komplex> v;
    for (int j = 0; j < saiten; ++j)
    {
        v.emplace_back(std::cos(gamma), 0.0);
        v.emplace_back(-std::sin(gamma), 0.0);
    }
    return v;
}

}  // namespace

TEST_CASE("Loop KleinEigen 31d-7 eine diagonale Matrix bleibt, wie sie ist")
{
    KleineMatrix a(3);
    a(0, 0) = Komplex(-0.25, 1643.7);
    a(1, 1) = Komplex(-0.5, 1644.1);
    a(2, 2) = Komplex(-1.0 / 3.0, 1642.9);
    const Eigenzerlegung z = zerlegeKlein(a);
    REQUIRE(z.werte.size() == 3);
    CHECK(z.vollstaendig);
    for (int i = 0; i < 3; ++i)
    {
        CHECK(z.werte[static_cast<std::size_t>(i)] == a(i, i));  // Bit fuer Bit
        for (int j = 0; j < 3; ++j)
        {
            CHECK(z.rechts(i, j) == Komplex(i == j ? 1.0 : 0.0, 0.0));
            CHECK(z.links(i, j) == Komplex(i == j ? 1.0 : 0.0, 0.0));
        }
    }
    CHECK(zerlegeKlein(KleineMatrix{}).werte.empty());
}

TEST_CASE("Loop KleinEigen 31d-7 zufaellige Matrizen wie Eigen")
{
    std::mt19937_64 zufall(20261025);
    std::uniform_real_distribution<double> gleich(-1.0, 1.0);
    for (int n = 1; n <= 8; ++n)
        for (int runde = 0; runde < 20; ++runde)
        {
            CAPTURE(n);
            CAPTURE(runde);
            KleineMatrix a(n);
            for (Komplex& w : a.werte) w = Komplex(gleich(zufall), gleich(zufall));
            const Eigenzerlegung z = zerlegeKlein(a);
            REQUIRE(z.werte.size() == static_cast<std::size_t>(n));
            CHECK(z.vollstaendig);
            const double groesse = normF(a);
            CHECK(gegenEigenWerte(a, z) < 1e-12 * groesse);
            CHECK(residuum(a, z) < 1e-12 * groesse);
            CHECK(gegenEinheit(z) < 1e-10);
        }
}

TEST_CASE("Loop KleinEigen 31d-7 der Chor in einer Ebene wie chorModen")
{
    const double omega = 2.0 * std::numbers::pi * 261.6;
    const double sigmaSaite = 0.25;
    const double sigmaSteg = 0.17;
    for (const int n : {2, 3})
        for (const double cent : {0.5, 2.0, 6.0})
        {
            CAPTURE(n);
            CAPTURE(cent);
            std::vector<double> omegas;
            for (int j = 0; j < n; ++j) omegas.push_back(omega * std::exp2((j - (n - 1) / 2.0) * cent / ((n - 1) * 1200.0)));
            KleineMatrix a(n);
            for (int z = 0; z < n; ++z)
                for (int s = 0; s < n; ++s)
                    a(z, s) = Komplex(z == s ? -sigmaSaite : 0.0, z == s ? omegas[static_cast<std::size_t>(z)] : 0.0) - sigmaSteg;
            const Eigenzerlegung z = zerlegeKlein(a);
            REQUIRE(z.vollstaendig);
            const auto moden = chorModen(omegas, sigmaSaite, sigmaSteg);
            REQUIRE(moden.size() == static_cast<std::size_t>(n));
            const double groesse = normF(a);
            // je Mode des Chors die naechste der Zerlegung: derselbe Eigenwert und Anteil (1·v)(w·1)/N
            for (const ChorMode& m : moden)
            {
                int beste = 0;
                for (int k = 1; k < n; ++k)
                    if (std::abs(z.werte[static_cast<std::size_t>(k)] - m.lambda) <
                        std::abs(z.werte[static_cast<std::size_t>(beste)] - m.lambda))
                        beste = k;
                CHECK(std::abs(z.werte[static_cast<std::size_t>(beste)] - m.lambda) < 1e-12 * groesse);
                Komplex aus(0.0, 0.0);
                Komplex ein(0.0, 0.0);
                for (int i = 0; i < n; ++i)
                {
                    aus += z.rechts(i, beste);
                    ein += z.links(beste, i);
                }
                CHECK(std::abs(aus * ein / static_cast<double>(n) - m.anteil) < 1e-9);
            }
        }
}

TEST_CASE("Loop KleinEigen 31d-7 zwei Ebenen: zusammenfallend und eng verstimmt")
{
    const double omega = 2.0 * std::numbers::pi * 261.6;
    const double gamma = 0.3;
    const auto pruefe = [&](const KleineMatrix& a, int saiten) {
        const Eigenzerlegung z = zerlegeKlein(a);
        REQUIRE(z.werte.size() == static_cast<std::size_t>(a.n));
        CHECK(z.vollstaendig);
        const double groesse = normF(a);
        CHECK(gegenEigenWerte(a, z) < 1e-12 * groesse);
        CHECK(residuum(a, z) < 1e-12 * groesse);
        CHECK(gegenEinheit(z) < 1e-9);
        // die Uebertragung vom Hammer (senkrecht) zum Steg (senkrecht) an einigen Stellen nahe den Polen
        const std::vector<Komplex> s = senkrecht(saiten, gamma);
        for (const Komplex stelle : {Komplex(0.1, omega), Komplex(-0.2, omega * 1.0001), Komplex(1.0, omega * 0.9999)})
        {
            const Komplex direkt = uebertragungDirekt(a, s, s, stelle);
            CHECK(std::abs(uebertragungModal(z, s, s, stelle) - direkt) < 1e-9 * std::abs(direkt));
        }
        // die Projektoren v_m·w_m der isolierten Eigenwerte wie Eigen
        Eigen::ComplexEigenSolver<Eigen::MatrixXcd> loeser(alsEigen(a));
        const Eigen::MatrixXcd vEigen = loeser.eigenvectors();
        const Eigen::MatrixXcd wEigen = vEigen.inverse();
        for (int m = 0; m < a.n; ++m)
        {
            const Komplex lambda = z.werte[static_cast<std::size_t>(m)];
            double abstand = 1e300;
            for (int k = 0; k < a.n; ++k)
                if (k != m) abstand = std::min(abstand, std::abs(z.werte[static_cast<std::size_t>(k)] - lambda));
            if (abstand < 1e-6 * groesse) continue;
            int e = 0;
            for (int k = 1; k < a.n; ++k)
                if (std::abs(loeser.eigenvalues()(k) - lambda) < std::abs(loeser.eigenvalues()(e) - lambda)) e = k;
            double groesste = 0.0;
            for (int i = 0; i < a.n; ++i)
                for (int j = 0; j < a.n; ++j)
                    groesste = std::max(groesste, std::abs(z.rechts(i, m) * z.links(m, j) - vEigen(i, e) * wEigen(e, j)));
            CHECK(groesste < 1e-9);
        }
    };
    SUBCASE("drei Saiten, ohne Verstimmung und ohne δ: vier Eigenwerte fallen zusammen")
    {
        pruefe(zweiEbenen({omega, omega, omega}, 0.0, gamma, 0.25, 0.4, 0.02), 3);
    }
    SUBCASE("eine Saite, ohne δ")
    {
        pruefe(zweiEbenen({omega}, 0.0, gamma, 0.25, 0.4, 0.02), 1);
    }
    SUBCASE("drei Saiten eng verstimmt (0,002 Cent), δ 0,001 Cent")
    {
        const double eng = std::exp2(0.001 / 1200.0);
        pruefe(zweiEbenen({omega / eng, omega, omega * eng}, std::exp2(0.001 / 1200.0) - 1.0, gamma, 0.25, 0.4, 0.02), 3);
    }
    SUBCASE("drei Saiten, 2 Cent, δ 1,5 Cent")
    {
        const double cent = std::exp2(1.0 / 1200.0);
        pruefe(zweiEbenen({omega / cent, omega, omega * cent}, std::exp2(1.5 / 1200.0) - 1.0, gamma, 0.25, 0.4, 0.02), 3);
    }
}
