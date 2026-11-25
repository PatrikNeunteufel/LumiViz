/**
 ****************************************************************************************
 * @file   test_InstrumentMeasureUmlauf.cpp
 * @brief  Stufe 34k-7c: der Umlauf-Filter der gezupften Saite — die Glieder (`InstrumentModels/gemeinsam/Umlauf`) und der
 *         Entwurf (`InstrumentMeasure/UmlaufEntwurf`); docs/audio/Instrumente_IC_Konzept.md UL1 a–UL3-1 a, ZK1 a
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Ohne Qt-Anwendung, ohne Ton.
 *
 * - Die Glieder im Takt (Verlust, Kette, Thiran) gegen ihre Antwort; der Verzug der Kette und des Thiran-Glieds; das
 *   Stimmen des Glieds.
 * - Das Ziel: der Grundton der Saite aus dem Umlauf gegen die Proben.
 * - Die Tafel je Taste an Stichtasten von Harfe und Cembalo (Proben `2026-11-18_S205_tafel_je_taste_*.txt`): Grundton,
 *   Zahl der Teiltoene, Abel-Smith-Zahl wie die Probe, UL2 a haelt (3 Cent, Abklingen in der Schwelle).
 * - Die gebundene Tafel je Saite an Laute Chor 1.1 und Oud Chor 5.1 (`…_gebunden_stuetzstellen_*_rand_verlust_halb.txt`):
 *   UL2 a an Buenden, halben, Achteln und an den fremden Stellen.
 * - Messfall `LUMIVIZ_34K7C_MESSUNG=<datei.md>` (`LUMIVIZ_34K7C_NUR=Harfe,Cembalo` waehlt Instrumente): jede Taste von Harfe und Cembalo, jede Saite von Gitarre, Laute und Oud
 *   mit Ergebnis und Rechenzeit (Kipppunkt ZK1 a). RZ2 a (S221): `LUMIVIZ_34K7C_TASTEN=<T32,Saite 5,…>` waehlt Tasten bzw.
 *   Saiten, `LUMIVIZ_34K7C_RANGVERLUST=norm|scipy|beide` die Fassung (mit TK2 a, wie scipy, beide); die Datei traegt dazu
 *   je Entwurf den Rechenaufwand.
 * - Rechenaufwand und Schalter Rangverlust (RZ2 a): die Zaehler gehen auf; ohne Rangverlust rechnen beide Fassungen gleich.
 *
 * Die Saiten sind die des Pruefstands S199 (`proben/2026-11-15_S199_pruefstand_zupf.py`: `GITARRE`, `harfe_taste`,
 * `cembalo_taste`, `choere`); sie ziehen mit 34k-7d in die Vorgaben des Kerns.
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentMeasure/UmlaufEntwurf.hpp"
#include "InstrumentModels/gemeinsam/Umlauf.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <complex>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <numbers>
#include <optional>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace
{
namespace ku = lumi::kern::umlauf;
namespace mu = lumi::messung::umlauf;
using ku::Saitenwerte;
constexpr double kPi = std::numbers::pi;

// ---------------------------------------------------------------- die Saiten des Pruefstands S199
double hzVon(int taste) { return 440.0 * std::exp2((taste - 69) / 12.0); }

// Woodhouse & Lynch-Aird 2019 Tab. I: E0 GPa, Anstieg je GPa Spannung, E'' GPa, rho
Saitenwerte monofil(double dmm, double hz, double laenge, char werkstoff)
{
    double e0 = 4.5, anstieg = 39.0, e2 = 0.25, rho = 1070.0;  // N
    if (werkstoff == 'F') e0 = 3.2, anstieg = 41.0, e2 = 0.18, rho = 1800.0;
    if (werkstoff == 'D') e0 = 6.0, anstieg = 0.0, e2 = 0.24, rho = 1320.0;
    const double d = dmm / 1000.0, flaeche = kPi * d * d / 4.0, m = rho * flaeche;
    const double T = 4.0 * laenge * laenge * hz * hz * m;
    const double E = (e0 + anstieg * T / flaeche / 1e9) * 1e9;
    Saitenwerte s;
    s.zug = T;
    s.masse = m;
    s.biegesteife = E * kPi * std::pow(d, 4) / 64.0;
    s.verlustBiegung = e2 * 1e9 / E;
    s.durchmesser = d;
    s.dichte = rho;
    return s;
}

Saitenwerte umsponnen(double zug, double hz, double laenge)
{
    Saitenwerte s;
    s.zug = zug;
    s.masse = zug / (4.0 * laenge * laenge * hz * hz);
    s.biegesteife = 50e-6;
    s.verlustInnen = 5e-5;
    s.verlustBiegung = 2.0e-2;
    s.verlustLuft = 1.1;
    return s;
}

struct Gitarrensaite
{
    int taste;
    double T, m, EI, etaF, etaB, etaA;
};
// Woodhouse u. a. 2012 Tab. I, Satz (a) Nylon; Saite 6 … 1
const Gitarrensaite kGitarre[6] = {{40, 71.6, 6.24e-3, 57e-6, 2e-5, 2.0e-2, 1.2}, {45, 73.9, 3.61e-3, 40e-6, 7e-5, 2.5e-2, 0.9},
                                   {50, 71.2, 1.95e-3, 51e-6, 5e-5, 2.0e-2, 1.2},  {55, 58.3, 0.90e-3, 310e-6, 14e-5, 2.0e-2, 1.7},
                                   {59, 53.4, 0.52e-3, 160e-6, 40e-5, 2.0e-2, 1.2}, {64, 70.3, 0.38e-3, 130e-6, 40e-5, 2.4e-2, 1.5}};
constexpr double kGitarreMensur = 0.650;

Saitenwerte gitarre(int saite)  // 6 … 1
{
    const Gitarrensaite& g = kGitarre[6 - saite];
    Saitenwerte s;
    s.zug = g.T;
    s.masse = g.m;
    s.biegesteife = g.EI;
    s.verlustInnen = g.etaF;
    s.verlustBiegung = g.etaB;
    s.verlustLuft = g.etaA;
    return s;
}

const double kHarfeMono[35][3] = {
    {2960, 0.52, 0.071}, {2637, 0.52, 0.082}, {2489, 0.56, 0.088}, {2218, 0.56, 0.095}, {1976, 0.61, 0.105}, {1864, 0.62, 0.113},
    {1661, 0.65, 0.123}, {1480, 0.69, 0.132}, {1318, 0.70, 0.143}, {1244, 0.69, 0.154}, {1109, 0.70, 0.164}, {988, 0.73, 0.177},
    {932, 0.76, 0.192},  {830, 0.80, 0.208},  {740, 0.85, 0.225},  {659, 0.90, 0.242},  {622, 0.95, 0.265},  {554, 1.00, 0.285},
    {494, 1.06, 0.327},  {466, 1.12, 0.354},  {415, 1.18, 0.385},  {370, 1.24, 0.421},  {330, 1.30, 0.455},  {311, 1.36, 0.498},
    {277, 1.44, 0.542},  {247, 1.52, 0.591},  {233, 1.60, 0.644},  {207, 1.68, 0.699},  {185, 1.76, 0.753},  {165, 1.84, 0.813},
    {155, 1.92, 0.880},  {139, 2.00, 0.945},  {123, 2.10, 0.999},  {116, 2.20, 1.053},  {104, 2.35, 1.103}};
const double kHarfeMetall[11][4] = {{92, 0.80, 1.45, 1.150}, {82, 0.80, 1.55, 1.180}, {77, 0.80, 1.65, 1.222}, {69, 0.85, 1.75, 1.257},
                                    {61, 0.90, 1.92, 1.299}, {58, 0.95, 2.05, 1.330}, {52, 1.00, 2.15, 1.359}, {46, 1.10, 2.30, 1.397},
                                    {41, 1.10, 2.45, 1.421}, {39, 1.25, 2.60, 1.445}, {35, 1.30, 2.75, 1.475}};

std::optional<std::pair<Saitenwerte, double>> harfe(int taste)
{
    std::vector<int> offen;
    for (const auto& z : kHarfeMono) offen.push_back(static_cast<int>(std::lround(69 + 12 * std::log2(z[0] / 440.0))));
    for (const auto& z : kHarfeMetall) offen.push_back(static_cast<int>(std::lround(69 + 12 * std::log2(z[0] / 440.0))));
    const int tiefste = *std::min_element(offen.begin(), offen.end());
    for (int versatz : {1, 2, 0})
    {
        const auto it = std::find(offen.begin(), offen.end(), taste - versatz);
        if (it == offen.end() || !(versatz != 0 || taste == tiefste)) continue;
        const auto i = static_cast<std::size_t>(it - offen.begin());
        Saitenwerte sa;
        double L = 0.0;
        if (i < 35)
        {
            L = kHarfeMono[i][2];
            sa = monofil(kHarfeMono[i][1], kHarfeMono[i][0], L, i <= 9 ? 'N' : 'D');
        }
        else
        {
            const auto& z = kHarfeMetall[i - 35];
            const double hz = z[0], kern = z[1] / 1000.0, gesamt = z[2] / 1000.0;
            L = z[3];
            const double m = 7850 * kPi * kern * kern / 4 + 8900 * kPi * (gesamt * gesamt - kern * kern) / 4;
            sa.zug = 4 * L * L * hz * hz * m;
            sa.masse = m;
            sa.biegesteife = 200e9 * kPi * std::pow(kern, 4) / 64;
            sa.verlustInnen = 5e-5;
            sa.verlustBiegung = 2.0e-2;
            sa.verlustLuft = 1.1;
        }
        return std::make_pair(sa, L * std::exp2(-versatz / 12.0));
    }
    return std::nullopt;
}

// Taskin 1769: Taste, Laenge mm; Besaitung nach Di Veroli: bis Taste, Durchmesser mm, Dichte, E GPa
const double kTaskin[11][2] = {{29, 1765}, {36, 1681}, {41, 1443}, {48, 1178}, {53, 982}, {60, 711},
                               {65, 535},  {72, 357},  {77, 268},  {84, 186},  {89, 148}};
const double kTaskinDraht[13][4] = {{31, 0.620, 8700, 115}, {33, 0.572, 8700, 115}, {36, 0.518, 8700, 115}, {38, 0.463, 8700, 115},
                                    {40, 0.463, 8500, 110}, {43, 0.410, 8500, 110}, {45, 0.377, 8500, 110}, {47, 0.339, 8500, 110},
                                    {51, 0.339, 7800, 170}, {56, 0.300, 7800, 170}, {64, 0.269, 7800, 170}, {75, 0.244, 7800, 170},
                                    {89, 0.213, 7800, 170}};

std::pair<Saitenwerte, double> cembalo(int taste)
{
    std::size_t j = 0;
    while (j + 2 < 11 && kTaskin[j + 1][0] <= taste) ++j;
    const double t = (taste - kTaskin[j][0]) / (kTaskin[j + 1][0] - kTaskin[j][0]);
    const double L = std::exp((1 - t) * std::log(kTaskin[j][1]) + t * std::log(kTaskin[j + 1][1])) / 1000.0;
    std::size_t k = 0;
    while (taste > kTaskinDraht[k][0]) ++k;
    const double d = kTaskinDraht[k][1] / 1000.0, rho = kTaskinDraht[k][2], e = kTaskinDraht[k][3];
    const double m = rho * kPi * d * d / 4;
    Saitenwerte s;
    s.masse = m;
    s.zug = 4 * L * L * hzVon(taste) * hzVon(taste) * m;
    s.biegesteife = e * 1e9 * kPi * std::pow(d, 4) / 64;
    s.verlustBiegung = 1e-3;
    s.durchmesser = d;
    s.dichte = rho;
    return {s, L};
}

constexpr double kLauteMensur = 0.625;
struct LautenSaite
{
    double dmm;
    char werkstoff;
    int oktave;
};
const std::vector<std::pair<int, std::vector<LautenSaite>>> kLaute = {
    {38, {{1.60, 'F', 0}, {0.925, 'N', 12}}}, {41, {{1.40, 'F', 0}, {0.875, 'N', 12}}}, {43, {{1.25, 'F', 0}, {0.775, 'N', 12}}},
    {48, {{0.91, 'F', 0}, {0.91, 'F', 0}}},   {53, {{0.900, 'N', 0}, {0.900, 'N', 0}}}, {57, {{0.725, 'N', 0}, {0.725, 'N', 0}}},
    {62, {{0.52, 'N', 0}, {0.52, 'N', 0}}},   {67, {{0.475, 'N', 0}}}};
constexpr double kOudMensur = 0.60;
// D'Addario EJ95A: 'u' umsponnen (Zug kg), 'n' Nylon (Durchmesser mm)
const std::vector<std::pair<int, std::vector<std::pair<char, double>>>> kOud = {
    {36, {{'u', 3.659}}}, {41, {{'u', 2.86}, {'u', 2.86}}}, {45, {{'u', 3.24}, {'u', 3.24}}}, {50, {{'u', 4.034}, {'u', 4.034}}},
    {55, {{'n', 0.031 * 25.4}, {'n', 0.031 * 25.4}}}, {60, {{'n', 0.026 * 25.4}, {'n', 0.026 * 25.4}}}};

struct BenannteSaite
{
    std::string name;
    Saitenwerte saite;
    double mensur;
    int buende;
};

std::vector<BenannteSaite> saitenVon(const std::string& inst)
{
    std::vector<BenannteSaite> out;
    if (inst == "Gitarre")
        for (int s = 6; s >= 1; --s) out.push_back({"Saite " + std::to_string(s), gitarre(s), kGitarreMensur, 19});
    if (inst == "Laute")
        for (std::size_t c = 0; c < kLaute.size(); ++c)
            for (std::size_t j = 0; j < kLaute[c].second.size(); ++j)
            {
                const LautenSaite& z = kLaute[c].second[j];
                if (j > 0 && z.oktave == kLaute[c].second[0].oktave && z.dmm == kLaute[c].second[0].dmm) continue;  // Einklang: einmal
                out.push_back({"Chor " + std::to_string(c + 1) + "." + std::to_string(j + 1),
                               monofil(z.dmm, hzVon(kLaute[c].first + z.oktave), kLauteMensur, z.werkstoff), kLauteMensur, 12});
            }
    if (inst == "Oud")
        for (std::size_t c = 0; c < kOud.size(); ++c)
        {
            const auto& [art, wert] = kOud[c].second[0];
            const double hz = hzVon(kOud[c].first);
            out.push_back({"Chor " + std::to_string(c + 1) + ".1",
                           art == 'u' ? umsponnen(wert * 9.80665, hz, kOudMensur) : monofil(wert, hz, kOudMensur, 'N'), kOudMensur, 12});
        }
    return out;
}

// die DFT der Impulsantwort bei ω
std::complex<double> dft(const std::vector<double>& h, double w)
{
    std::complex<double> s = 0.0;
    for (std::size_t n = 0; n < h.size(); ++n) s += h[n] * std::polar(1.0, -w * static_cast<double>(n));
    return s;
}

}  // namespace

TEST_CASE("InstrumentModels 34k-7c Glieder des Umlaufs im Takt gegen ihre Antwort")
{
    ku::Umlauffilter f;
    f.gliedOrdnung = 5;
    f.gliedVerzug = 5.3;
    f.kette = {{0.8, 0.4}, {0.5, 1.9}, {0.95, 0.05}};
    f.b = {0.6, -0.2, 0.05};  // ein Verlust mit |H| < 1
    f.a = {1.0, -0.5, 0.1};
    ku::Umlauflauf lauf(f);
    std::vector<double> h(6000);
    for (std::size_t n = 0; n < h.size(); ++n) h[n] = lauf.schritt(n == 0 ? 1.0 : 0.0);
    for (double w : {0.01, 0.3, 1.2, 2.5, 3.1})
    {
        const std::complex<double> a = f.antwort(w), b = dft(h, w);
        CHECK(std::abs(a - b) < 1e-9);
    }
    // Allpaesse: Betrag 1; der Verzug der Kette ist −arg der Antwort (abgewickelt)
    ku::Umlauffilter nurKette;
    nurKette.kette = f.kette;
    nurKette.gliedOrdnung = 1;
    nurKette.gliedVerzug = 1.3;  // (D = 1 gaebe 0/0 im Beiwert)
    double vor = 0.0, off = 0.0;
    for (int i = 1; i <= 3000; ++i)
    {
        const double w = kPi * i / 3001.0;
        const std::complex<double> hk = nurKette.antwort(w);
        CHECK(std::abs(std::abs(hk) - 1.0) < 1e-12);
        const double a = std::arg(hk);
        if (i > 1 && a - vor > kPi) off -= 2 * kPi;
        if (i > 1 && a - vor < -kPi) off += 2 * kPi;
        vor = a;
        CHECK(ku::verzugKette(f.kette, w) + ku::verzugThiran(1.3, 1, w) == doctest::Approx(-(a + off)).epsilon(1e-9));
    }
    // Thiran: bei kleinem ω der Verzug D; maximal flach
    for (double D : {4.5, 5.0, 5.49})
    {
        CHECK(ku::verzugThiran(D, 5, 1e-4) / 1e-4 == doctest::Approx(D).epsilon(1e-6));
        // stetig bis π: gegen die fein abgewickelte Phase der Antwort (S220: einmal abgewickelt lag sie ueber 4,8 kHz um 2π daneben)
        const std::vector<double> a = ku::thiranNenner(D, 5);
        const std::vector<double> b(a.rbegin(), a.rend());
        double v0 = 0.0, o = 0.0;
        for (int i = 1; i <= 2000; ++i)
        {
            const double w = 3.1 * i / 2000.0;
            const double p = std::arg(ku::antwort(b, a, w));
            if (i > 1 && p - v0 > kPi) o -= 2 * kPi;
            if (i > 1 && p - v0 < -kPi) o += 2 * kPi;
            v0 = p;
            CHECK(ku::verzugThiran(D, 5, w) == doctest::Approx(-(p + o)).epsilon(1e-9));
        }
    }
    // Stimmen: N und D treffen den Sollverzug am Grundton
    ku::Umlauffilter g = f;
    const double w1 = 2 * kPi * 220.0 / 48000.0;
    REQUIRE(ku::stimmeGlied(g, w1, 77.3 * w1));
    CHECK(g.gliedVerzug >= 4.5 - 1e-9);
    CHECK(g.gliedVerzug < 5.5 + 1e-9);
    CHECK(g.verzugGanz * w1 + ku::verzugThiran(g.gliedVerzug, 5, w1) == doctest::Approx(77.3 * w1).epsilon(1e-10));
    CHECK_FALSE(ku::stimmeGlied(g, w1, 5.0 * w1));  // kein Platz
}

TEST_CASE("InstrumentModels 34k-7c Ziel: der Grundton der Saite aus dem Umlauf")
{
    // der erste Teilton liegt, wo der Verzug 2π ist; die Proben S205 nennen ihn
    auto grund = [](const Saitenwerte& s, double L) {
        double lo = 1.0, hi = 20000.0;
        for (int i = 0; i < 200; ++i)
        {
            const double m = 0.5 * (lo + hi);
            (ku::umlaufDerSaite(s, L, m).verzug < 2 * kPi ? lo : hi) = m;
        }
        return lo;
    };
    const auto h60 = harfe(60);
    REQUIRE(h60);
    CHECK(grund(h60->first, h60->second) == doctest::Approx(261.72).epsilon(5e-5));
    const auto c29 = cembalo(29);
    CHECK(grund(c29.first, c29.second) == doctest::Approx(43.65).epsilon(5e-4));
    CHECK(grund(gitarre(6), kGitarreMensur) == doctest::Approx(82.4).epsilon(1e-3));
    CHECK(grund(gitarre(1), kGitarreMensur) == doctest::Approx(330.9).epsilon(1e-3));
    // der Betrag des Umlaufs liegt unter 1 und faellt
    const ku::Umlaufwert a = ku::umlaufDerSaite(gitarre(6), kGitarreMensur, 100.0), b = ku::umlaufDerSaite(gitarre(6), kGitarreMensur, 5000.0);
    CHECK(a.betrag < 1.0);
    CHECK(b.betrag < a.betrag);
}

TEST_CASE("InstrumentMeasure 34k-7c Tafel je Taste an Stichtasten von Harfe und Cembalo (UL2 a)")
{
    struct Stich
    {
        const char* inst;
        int taste;
        double f1;
        int zaehlend, abelSmith;
    };
    // tafel_je_taste_harfe.txt / _cembalo.txt: f1, zaehlend (mit Zugabe), Abel-Smith
    const Stich stiche[] = {{"Harfe", 27, 39.29, 57, 18},   {"Harfe", 46, 116.75, 31, 5}, {"Harfe", 60, 261.72, 25, 3},
                            {"Harfe", 96, 2095.05, 8, 0},   {"Harfe", 103, 3139.67, 5, 0}, {"Cembalo", 60, 261.63, 73, 4},
                            {"Cembalo", 84, 1046.56, 18, 0}};
    for (const Stich& st : stiche)
    {
        const std::string instrument = st.inst;  // CAPTURE haelt eine Referenz
        CAPTURE(instrument);
        CAPTURE(st.taste);
        Saitenwerte s;
        double L = 0.0;
        if (std::string(st.inst) == "Harfe")
        {
            const auto h = harfe(st.taste);
            REQUIRE(h);
            s = h->first;
            L = h->second;
        }
        else
            std::tie(s, L) = cembalo(st.taste);
        const auto t0 = std::chrono::steady_clock::now();
        const std::optional<mu::TafelTaste> t = mu::entwirfJeTaste(s, L);
        const double sek = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
        REQUIRE(t);
        MESSAGE(std::string(st.inst) << " T" << st.taste << ": Thiran " << t->filter.gliedOrdnung << ", " << t->filter.kette.size() << " Biquads, "
                        << t->bewertung.centGroesster << " Cent, Abklingen " << t->bewertung.abklingenGroesster << " %, " << sek << " s");
        CHECK(t->grundHz == doctest::Approx(st.f1).epsilon(1e-4));
        CHECK(t->zaehlendMitZugabe == st.zaehlend);
        CHECK(t->abelSmith == st.abelSmith);
        CHECK(t->bewertung.haelt());
        CHECK(std::abs(t->bewertung.grundtonCent) <= 0.1);  // UL2 a: Grundton ±0,1 Cent
        // das Filter traegt, was die Bewertung sagt: noch einmal gemessen
        mu::Umlauffilter f = t->filter;
        const mu::Bewertung b = mu::bewerte(s, L, f);
        CHECK(b.centGroesster == doctest::Approx(t->bewertung.centGroesster).epsilon(1e-9));
        CHECK(f.verzugGanz == t->filter.verzugGanz);
    }
}

namespace
{
// die Zaehler gehen auf: jede Auswertung ist ein Start, ein angenommener oder ein verworfener Schritt; jede Jacobi-Matrix ein
// Start oder ein angenommener Schritt
void pruefeAufwand(const mu::Rechenaufwand& a)
{
    CHECK(a.anpassungen > 0);
    CHECK(a.schritte > 0);
    CHECK(a.auswertungen == a.anpassungen + a.schritte + a.verworfen);
    CHECK(a.ableitungen == a.anpassungen + a.schritte);
    CHECK(a.zerlegungen >= a.schritte);
    CHECK(a.rangverluste <= a.zerlegungen);
    CHECK(a.sekundenResiduen + a.sekundenAbleitungen + a.sekundenZerlegung <= a.sekundenAnpassung);
    CHECK(a.sekundenAnpassung <= a.sekundenGesamt);
}
}  // namespace

TEST_CASE("InstrumentMeasure 34k-7c Rechenaufwand und Schalter Rangverlust (RZ2 a)")
{
    // Harfe T46: kurz, ohne Rangverlust — dort rechnen beide Fassungen denselben Weg
    const auto h = harfe(46);
    REQUIRE(h);
    const auto vorgabe = mu::entwirfJeTaste(h->first, h->second);
    mu::EntwurfOptionen scipy;
    scipy.rangverlust = mu::Rangverlust::WieScipy;
    const auto ohneTk2 = mu::entwirfJeTaste(h->first, h->second, scipy);
    REQUIRE(vorgabe);
    REQUIRE(ohneTk2);
    pruefeAufwand(vorgabe->aufwand);
    pruefeAufwand(ohneTk2->aufwand);
    MESSAGE("Harfe T46: " << vorgabe->aufwand.anpassungen << " Laeufe, " << vorgabe->aufwand.schritte << " Schritte, "
                          << vorgabe->aufwand.verworfen << " verworfen, " << vorgabe->aufwand.rangverluste << " Rangverluste");
    CHECK(vorgabe->aufwand.rangverluste == 0);
    CHECK(ohneTk2->aufwand.schritte == vorgabe->aufwand.schritte);
    CHECK(ohneTk2->aufwand.auswertungen == vorgabe->aufwand.auswertungen);
    CHECK(ohneTk2->bewertung.centGroesster == vorgabe->bewertung.centGroesster);
}

TEST_CASE("InstrumentMeasure 34k-7c Tafel je Saite gebunden: Laute Chor 1.1, Oud Chor 5.1 (UL3-1 a)")
{
    // die Gitarre (Saite 6: rund 5 min, Saite 1: rund 2 min) rechnet der Messfall; dort trifft sie die Probe Zahl fuer Zahl
    struct Stich
    {
        BenannteSaite saite;
        int M;
        double fremdCent, fremdAbklingen;  // gebunden_stuetzstellen_*_rand_verlust_halb.txt
    };
    const std::vector<BenannteSaite> laute = saitenVon("Laute"), oud = saitenVon("Oud");
    const Stich stiche[] = {{{"Laute " + laute[0].name, laute[0].saite, kLauteMensur, 12}, 8, 0.37, 12.09},
                            {{"Oud " + oud[4].name, oud[4].saite, kOudMensur, 12}, 8, 0.09, 3.99}};
    for (const Stich& st : stiche)
    {
        CAPTURE(st.saite.name);
        const auto t0 = std::chrono::steady_clock::now();
        const std::optional<mu::TafelSaite> t = mu::entwirfJeSaite(st.saite.saite, st.saite.mensur, st.saite.buende);
        const double sek = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
        REQUIRE(t);
        MESSAGE(st.saite.name << ": " << t->radiusReihe.size() << " Biquads, Start " << t->startFaktor << ", Daempfung " << t->daempfung
                              << " | Buende " << t->buendeBewertung.centGroesster << ", halb " << t->halbBewertung.centGroesster << ", Achtel "
                              << t->achtelBewertung.centGroesster << ", fremd " << t->fremdBewertung.centGroesster << " Cent, Abklingen fremd "
                              << t->fremdBewertung.abklingenGroesster << " % | " << sek << " s");
        pruefeAufwand(t->aufwand);
        CHECK(static_cast<int>(t->radiusReihe.size()) == st.M);
        CHECK(t->verlust.size() == static_cast<std::size_t>(2 * st.saite.buende + 1));
        for (const mu::Bewertung* b : {&t->buendeBewertung, &t->halbBewertung, &t->achtelBewertung, &t->fremdBewertung})
            CHECK(b->haelt());
        // wie die Probe (trf wie scipy, aber eigene SVD und der Verlust mit Austausch: daher eine Zugabe)
        CHECK(t->fremdBewertung.centGroesster <= st.fremdCent + 0.2);
        CHECK(t->fremdBewertung.abklingenGroesster <= st.fremdAbklingen + 2.0);
        // das Filter an einer Lage, die die Anpassung nicht kennt
        mu::Bewertung b;
        const std::optional<mu::Umlauffilter> f = mu::filterAn(*t, st.saite.saite, 4.3, &b);
        REQUIRE(f);
        CHECK(b.haelt());
        CHECK(f->gliedOrdnung == 5);
    }
}

TEST_CASE("InstrumentMeasure 34k-7c Messfall Umlauf-Filter (LUMIVIZ_34K7C_MESSUNG=<datei.md>)")
{
    const char* ziel = std::getenv("LUMIVIZ_34K7C_MESSUNG");
    if (ziel == nullptr || std::string(ziel).empty())
    {
        MESSAGE("LUMIVIZ_34K7C_MESSUNG=<datei.md> entwirft jede Taste von Harfe und Cembalo und jede Saite von Gitarre, Laute, Oud "
                "und misst die Rechenzeit (ZK1 a)");
        return;
    }
    // LUMIVIZ_34K7C_NUR=<Instrument,…> misst nur diese (etwa "Harfe,Cembalo"); ohne die Variable alle fuenf.
    const char* nurText = std::getenv("LUMIVIZ_34K7C_NUR");
    const std::string nur = nurText == nullptr ? std::string() : "," + std::string(nurText) + ",";
    const auto gewaehlt = [&nur](const char* inst) { return nur.empty() || nur.find("," + std::string(inst) + ",") != std::string::npos; };
    // LUMIVIZ_34K7C_TASTEN=<T32,Saite 5,…> nur diese Tasten bzw. Saiten (Namen wie in den Tabellen, Tasten mit "T").
    const char* tastenText = std::getenv("LUMIVIZ_34K7C_TASTEN");
    const std::string tasten = tastenText == nullptr ? std::string() : "," + std::string(tastenText) + ",";
    const auto tasteGewaehlt = [&tasten](const std::string& name) { return tasten.empty() || tasten.find("," + name + ",") != std::string::npos; };
    // LUMIVIZ_34K7C_RANGVERLUST=norm|scipy|beide (RZ2 a): mit TK2 a (Vorgabe), wie scipy, oder beide je Taste nacheinander.
    const char* rvText = std::getenv("LUMIVIZ_34K7C_RANGVERLUST");
    const std::string rv = rvText == nullptr || std::string(rvText).empty() ? "norm" : rvText;
    REQUIRE((rv == "norm" || rv == "scipy" || rv == "beide"));
    std::vector<std::pair<std::string, mu::EntwurfOptionen>> fassungen;
    if (rv != "scipy") fassungen.emplace_back("TK2 a", mu::EntwurfOptionen{});
    if (rv != "norm")
    {
        mu::EntwurfOptionen e;
        e.rangverlust = mu::Rangverlust::WieScipy;
        fassungen.emplace_back("scipy", e);
    }
    const auto zeilenName = [&fassungen](const std::string& name, const std::string& fassung) {
        return fassungen.size() > 1 || fassung != "TK2 a" ? name + " (" + fassung + ")" : name;
    };
    std::ostringstream aufwand;
    aufwand << "\n## Rechenaufwand (RZ2 a)\n\n| Instrument | Taste / Saite | Fassung | Laeufe | Schritte | verworfen | Auswertungen | "
               "Ableitungen | SVD | Rangverluste | s Residuen | s Ableitungen | s SVD | s Anpassung | s Rest | s gesamt |\n"
               "|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|\n";
    const auto aufwandZeile = [&aufwand](const char* inst, const std::string& name, const std::string& fassung, const mu::Rechenaufwand& a) {
        char zz[512];
        std::snprintf(zz, sizeof zz, "| %s | %s | %s | %d | %d | %d | %d | %d | %d | %d | %.1f | %.1f | %.1f | %.1f | %.1f | %.1f |\n", inst,
                      name.c_str(), fassung.c_str(), a.anpassungen, a.schritte, a.verworfen, a.auswertungen, a.ableitungen, a.zerlegungen,
                      a.rangverluste, a.sekundenResiduen, a.sekundenAbleitungen, a.sekundenZerlegung, a.sekundenAnpassung,
                      a.sekundenGesamt - a.sekundenAnpassung, a.sekundenGesamt);
        aufwand << zz;
    };
    std::ostringstream o;
    o << "# 34k-7c Entwurf des Umlauf-Filters in C++\n\n"
         "> test_InstrumentMeasureUmlauf, `LUMIVIZ_34K7C_MESSUNG`. Saiten des Pruefstands S199; UL2 a: 3 Cent, Abklingen −30,1 … "
         "+26,2 %. Zeit je Taste bzw. Saite auf einem Thread (Taste) bzw. mit dem Pool (Saite). Rangverlust: "
      << rv << (tasten.empty() ? std::string() : "; Tasten / Saiten: " + std::string(tastenText)) << ".\n";
    char z[512];
    for (const char* inst : {"Harfe", "Cembalo"})
    {
        if (!gewaehlt(inst)) continue;
        o << "\n## " << inst << " (Tafel je Taste)\n\n| Taste | f1 Hz | zaehlend | Abel-Smith | Glied | Biquads | Cent | Abklingen % | haelt | s |\n"
                                "|---|---|---|---|---|---|---|---|---|---|\n";
        double summe = 0.0;
        int gut = 0, alle = 0;
        const int von = std::string(inst) == "Harfe" ? 25 : 29, bis = std::string(inst) == "Harfe" ? 104 : 89;
        for (int taste = von; taste <= bis; ++taste)
        {
            const std::string name = "T" + std::to_string(taste);
            if (!tasteGewaehlt(name)) continue;
            Saitenwerte s;
            double L = 0.0;
            if (std::string(inst) == "Harfe")
            {
                const auto h = harfe(taste);
                if (!h) continue;
                std::tie(s, L) = *h;
            }
            else
                std::tie(s, L) = cembalo(taste);
            for (const auto& [fassung, optionen] : fassungen)
            {
                const std::string zeile = zeilenName(std::to_string(taste), fassung);
                const auto t0 = std::chrono::steady_clock::now();
                const auto t = mu::entwirfJeTaste(s, L, optionen);
                const double sek = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
                summe += sek;
                ++alle;
                if (!t)
                {
                    std::snprintf(z, sizeof z, "| %s | — | — | — | — | — | — | — | nicht rechenbar | %.1f |\n", zeile.c_str(), sek);
                    o << z;
                    continue;
                }
                gut += t->bewertung.haelt();
                std::snprintf(z, sizeof z, "| %s | %.2f | %d | %d | T%d | %zu | %.2f | %.2f | %s | %.1f |\n", zeile.c_str(), t->grundHz,
                              t->zaehlendMitZugabe, t->abelSmith, t->filter.gliedOrdnung, t->filter.kette.size(), t->bewertung.centGroesster,
                              t->bewertung.abklingenGroesster, t->bewertung.haelt() ? "ja" : "**nein**", sek);
                o << z;
                aufwandZeile(inst, name, fassung, t->aufwand);
                std::printf("%s %s %s %.1f s\n", inst, name.c_str(), fassung.c_str(), sek);
            }
        }
        std::snprintf(z, sizeof z, "\n%s: %d von %d Tasten halten UL2 a; Entwurf zusammen %.1f s.\n", inst, gut, alle, summe);
        o << z;
    }
    for (const char* inst : {"Gitarre", "Laute", "Oud"})
    {
        if (!gewaehlt(inst)) continue;
        o << "\n## " << inst << " (je Saite gebunden)\n\n| Saite | Biquads | Start | Daempfung | Buende | halb | Achtel | fremd Cent | "
                                "fremd Abklingen % | haelt | s |\n|---|---|---|---|---|---|---|---|---|---|---|\n";
        double summe = 0.0;
        for (const BenannteSaite& b : saitenVon(inst))
        {
            if (!tasteGewaehlt(b.name)) continue;
            for (const auto& [fassung, optionen] : fassungen)
            {
                const std::string zeile = zeilenName(b.name, fassung);
                const auto t0 = std::chrono::steady_clock::now();
                const auto t = mu::entwirfJeSaite(b.saite, b.mensur, b.buende, optionen);
                const double sek = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
                summe += sek;
                if (!t)
                {
                    o << "| " << zeile << " | — | — | — | — | — | — | — | — | nicht rechenbar | " << sek << " |\n";
                    continue;
                }
                const bool h =
                    t->buendeBewertung.haelt() && t->halbBewertung.haelt() && t->achtelBewertung.haelt() && t->fremdBewertung.haelt();
                std::snprintf(z, sizeof z, "| %s | %zu | %.2f | %.2f | %.2f | %.2f | %.2f | %.2f | %.2f | %s | %.1f |\n", zeile.c_str(),
                              t->radiusReihe.size(), t->startFaktor, t->daempfung, t->buendeBewertung.centGroesster,
                              t->halbBewertung.centGroesster, t->achtelBewertung.centGroesster, t->fremdBewertung.centGroesster,
                              t->fremdBewertung.abklingenGroesster, h ? "ja" : "**nein**", sek);
                o << z;
                aufwandZeile(inst, b.name, fassung, t->aufwand);
                std::printf("%s %s %s %.1f s\n", inst, b.name.c_str(), fassung.c_str(), sek);
            }
        }
        std::snprintf(z, sizeof z, "\n%s: Entwurf zusammen %.1f s.\n", inst, summe);
        o << z;
    }
    o << aufwand.str();
    std::ofstream(ziel) << o.str();
    MESSAGE("geschrieben: " << std::string(ziel));
}
