/**
 * @file Saiten.cpp
 * @brief InstrumentModels, Kern `pluckedString` Rechenfassung 2: die Saiten der fuenf Instrumente (siehe Kopf von
 *        `Saiten.hpp`; die Zahlen wie `proben/2026-11-15_S199_pruefstand_zupf.py`, bis 34k-7d im Test von 34k-7c)
 */
#include "InstrumentModels/zupf/Saiten.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace lumi::kern::zupf
{

namespace
{
constexpr double kPi = std::numbers::pi;

double hzVon(int taste) { return 440.0 * std::exp2((taste - 69) / 12.0); }

// Bell 1988 Tab. 2.1 (Nylon 00–8, Darm 9–33): Hz, Durchmesser mm, Laenge m
const double kHarfeMono[35][3] = {
    {2960, 0.52, 0.071}, {2637, 0.52, 0.082}, {2489, 0.56, 0.088}, {2218, 0.56, 0.095}, {1976, 0.61, 0.105}, {1864, 0.62, 0.113},
    {1661, 0.65, 0.123}, {1480, 0.69, 0.132}, {1318, 0.70, 0.143}, {1244, 0.69, 0.154}, {1109, 0.70, 0.164}, {988, 0.73, 0.177},
    {932, 0.76, 0.192},  {830, 0.80, 0.208},  {740, 0.85, 0.225},  {659, 0.90, 0.242},  {622, 0.95, 0.265},  {554, 1.00, 0.285},
    {494, 1.06, 0.327},  {466, 1.12, 0.354},  {415, 1.18, 0.385},  {370, 1.24, 0.421},  {330, 1.30, 0.455},  {311, 1.36, 0.498},
    {277, 1.44, 0.542},  {247, 1.52, 0.591},  {233, 1.60, 0.644},  {207, 1.68, 0.699},  {185, 1.76, 0.753},  {165, 1.84, 0.813},
    {155, 1.92, 0.880},  {139, 2.00, 0.945},  {123, 2.10, 0.999},  {116, 2.20, 1.053},  {104, 2.35, 1.103}};
// Bell 1988 Tab. 2.2 (Metall 34–44): Hz, Kern mm, gesamt mm, Laenge m (am Bild gelesen, S198)
const double kHarfeMetall[11][4] = {{92, 0.80, 1.45, 1.150}, {82, 0.80, 1.55, 1.180}, {77, 0.80, 1.65, 1.222}, {69, 0.85, 1.75, 1.257},
                                    {61, 0.90, 1.92, 1.299}, {58, 0.95, 2.05, 1.330}, {52, 1.00, 2.15, 1.359}, {46, 1.10, 2.30, 1.397},
                                    {41, 1.10, 2.45, 1.421}, {39, 1.25, 2.60, 1.445}, {35, 1.30, 2.75, 1.475}};

// Taskin 1769 (Edinburgh 96075): Taste, Laenge mm
const double kTaskin[11][2] = {{29, 1765}, {36, 1681}, {41, 1443}, {48, 1178}, {53, 982}, {60, 711},
                               {65, 535},  {72, 357},  {77, 268},  {84, 186},  {89, 148}};
// Di Veroli Tab. 7, 8: bis Taste, Durchmesser mm, Dichte kg/m³, E GPa (Dichte und E Annahme)
const double kTaskinDraht[13][4] = {{31, 0.620, 8700, 115}, {33, 0.572, 8700, 115}, {36, 0.518, 8700, 115}, {38, 0.463, 8700, 115},
                                    {40, 0.463, 8500, 110}, {43, 0.410, 8500, 110}, {45, 0.377, 8500, 110}, {47, 0.339, 8500, 110},
                                    {51, 0.339, 7800, 170}, {56, 0.300, 7800, 170}, {64, 0.269, 7800, 170}, {75, 0.244, 7800, 170},
                                    {89, 0.213, 7800, 170}};

// Woodhouse u. a. 2012 Tab. I, Satz (a) Nylon; Saite 6 … 1: Taste, T, m, EI, η_F, η_B, η_A
const double kGitarre[6][7] = {{40, 71.6, 6.24e-3, 57e-6, 2e-5, 2.0e-2, 1.2},  {45, 73.9, 3.61e-3, 40e-6, 7e-5, 2.5e-2, 0.9},
                               {50, 71.2, 1.95e-3, 51e-6, 5e-5, 2.0e-2, 1.2},  {55, 58.3, 0.90e-3, 310e-6, 14e-5, 2.0e-2, 1.7},
                               {59, 53.4, 0.52e-3, 160e-6, 40e-5, 2.0e-2, 1.2}, {64, 70.3, 0.38e-3, 130e-6, 40e-5, 2.4e-2, 1.5}};

struct Lautenzug
{
    double dmm;
    char werkstoff;
    int oktave;
};
// Woodhouse & Lynch-Aird 2019 Tab. II: je Chor die leere Taste und die Saiten
const std::vector<std::pair<int, std::vector<Lautenzug>>>& laute()
{
    static const std::vector<std::pair<int, std::vector<Lautenzug>>> l = {
        {38, {{1.60, 'F', 0}, {0.925, 'N', 12}}}, {41, {{1.40, 'F', 0}, {0.875, 'N', 12}}}, {43, {{1.25, 'F', 0}, {0.775, 'N', 12}}},
        {48, {{0.91, 'F', 0}, {0.91, 'F', 0}}},   {53, {{0.900, 'N', 0}, {0.900, 'N', 0}}}, {57, {{0.725, 'N', 0}, {0.725, 'N', 0}}},
        {62, {{0.52, 'N', 0}, {0.52, 'N', 0}}},   {67, {{0.475, 'N', 0}}}};
    return l;
}
// D'Addario EJ95A: 'u' umsponnen (Zug kg), 'n' Nylon (Durchmesser mm)
const std::vector<std::pair<int, std::vector<std::pair<char, double>>>>& oud()
{
    static const std::vector<std::pair<int, std::vector<std::pair<char, double>>>> o = {
        {36, {{'u', 3.659}}}, {41, {{'u', 2.86}, {'u', 2.86}}}, {45, {{'u', 3.24}, {'u', 3.24}}}, {50, {{'u', 4.034}, {'u', 4.034}}},
        {55, {{'n', 0.031 * 25.4}, {'n', 0.031 * 25.4}}}, {60, {{'n', 0.026 * 25.4}, {'n', 0.026 * 25.4}}}};
    return o;
}

/// Im Einklang zwei Saiten: die erste um die halbe Verstimmung kuerzer, die zweite laenger (Pruefstand `chor_instrument`).
void verstimme(Saitenchor& c, double cent)
{
    if (c.saiten.size() != 2 || c.saiten[0].oktave != c.saiten[1].oktave || !(c.saiten[0].werte == c.saiten[1].werte)) return;
    c.saiten[0].laengenFaktor = std::exp2(-0.5 * cent / 1200.0);
    c.saiten[1].laengenFaktor = std::exp2(0.5 * cent / 1200.0);
}
}  // namespace

Saitenwerte monofil(double durchmesserMm, double hz, double laenge, char werkstoff)
{
    double e0 = 4.5, anstieg = 39.0, e2 = 0.25, rho = 1070.0;  // N
    if (werkstoff == 'F') e0 = 3.2, anstieg = 41.0, e2 = 0.18, rho = 1800.0;
    if (werkstoff == 'D') e0 = 6.0, anstieg = 0.0, e2 = 0.24, rho = 1320.0;
    const double d = durchmesserMm / 1000.0, flaeche = kPi * d * d / 4.0, m = rho * flaeche;
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

std::optional<Tastensaite> harfenSaite(int taste)
{
    std::vector<int> offen;
    for (const auto& z : kHarfeMono) offen.push_back(static_cast<int>(std::lround(69 + 12 * std::log2(z[0] / 440.0))));
    for (const auto& z : kHarfeMetall) offen.push_back(static_cast<int>(std::lround(69 + 12 * std::log2(z[0] / 440.0))));
    const int tiefste = *std::min_element(offen.begin(), offen.end());
    for (const int versatz : {1, 2, 0})
    {
        const auto it = std::find(offen.begin(), offen.end(), taste - versatz);
        if (it == offen.end() || !(versatz != 0 || taste == tiefste)) continue;
        const auto i = static_cast<std::size_t>(it - offen.begin());
        Tastensaite t;
        double L = 0.0;
        if (i < 35)
        {
            L = kHarfeMono[i][2];
            t.werte = monofil(kHarfeMono[i][1], kHarfeMono[i][0], L, i <= 9 ? 'N' : 'D');
        }
        else
        {
            const auto& z = kHarfeMetall[i - 35];
            const double hz = z[0], kern = z[1] / 1000.0, gesamt = z[2] / 1000.0;
            L = z[3];
            const double m = 7850 * kPi * kern * kern / 4 + 8900 * kPi * (gesamt * gesamt - kern * kern) / 4;
            t.werte.zug = 4 * L * L * hz * hz * m;
            t.werte.masse = m;
            t.werte.biegesteife = 200e9 * kPi * std::pow(kern, 4) / 64;
            t.werte.verlustInnen = 5e-5;
            t.werte.verlustBiegung = 2.0e-2;
            t.werte.verlustLuft = 1.1;
        }
        t.laenge = L * std::exp2(-versatz / 12.0);
        t.stegTaste = taste;
        return t;
    }
    return std::nullopt;
}

Tastensaite cembaloSaite(int taste)
{
    std::size_t j = 0;
    while (j + 2 < 11 && kTaskin[j + 1][0] <= taste) ++j;
    const double t = (taste - kTaskin[j][0]) / (kTaskin[j + 1][0] - kTaskin[j][0]);
    const double L = std::exp((1 - t) * std::log(kTaskin[j][1]) + t * std::log(kTaskin[j + 1][1])) / 1000.0;
    std::size_t k = 0;
    while (k + 1 < 13 && taste > kTaskinDraht[k][0]) ++k;
    const double d = kTaskinDraht[k][1] / 1000.0, rho = kTaskinDraht[k][2], e = kTaskinDraht[k][3];
    const double m = rho * kPi * d * d / 4;
    Tastensaite s;
    s.werte.masse = m;
    s.werte.zug = 4 * L * L * hzVon(taste) * hzVon(taste) * m;
    s.werte.biegesteife = e * 1e9 * kPi * std::pow(d, 4) / 64;
    s.werte.verlustBiegung = 1e-3;
    s.werte.durchmesser = d;
    s.werte.dichte = rho;
    s.laenge = L;
    s.stegTaste = taste;
    return s;
}

Saitensatz saitensatzGitarre()
{
    Saitensatz s;
    s.mensur = 0.650;
    s.buende = 19;
    for (const auto& g : kGitarre)
    {
        Saitenchor c;
        c.leer = static_cast<int>(g[0]);
        Chorsaite cs;
        cs.werte.zug = g[1];
        cs.werte.masse = g[2];
        cs.werte.biegesteife = g[3];
        cs.werte.verlustInnen = g[4];
        cs.werte.verlustBiegung = g[5];
        cs.werte.verlustLuft = g[6];
        c.saiten.push_back(cs);
        s.choere.push_back(c);
    }
    return s;
}

Saitensatz saitensatzLaute()
{
    Saitensatz s;
    s.mensur = 0.625;
    s.buende = 12;
    for (const auto& [leer, zuege] : laute())
    {
        Saitenchor c;
        c.leer = leer;
        for (const Lautenzug& z : zuege) c.saiten.push_back({monofil(z.dmm, hzVon(leer + z.oktave), s.mensur, z.werkstoff), z.oktave, 1.0});
        verstimme(c, 4.0);
        s.choere.push_back(c);
    }
    return s;
}

Saitensatz saitensatzOud()
{
    Saitensatz s;
    s.mensur = 0.60;
    s.buende = 12;
    for (const auto& [leer, zuege] : oud())
    {
        Saitenchor c;
        c.leer = leer;
        for (const auto& [art, wert] : zuege)
            c.saiten.push_back({art == 'u' ? umsponnen(wert * 9.80665, hzVon(leer), s.mensur) : monofil(wert, hzVon(leer), s.mensur, 'N'), 0, 1.0});
        verstimme(c, 3.0);
        s.choere.push_back(c);
    }
    return s;
}

}  // namespace lumi::kern::zupf
