/**
 * @file StabStimmen.cpp
 * @brief Der Stimmer des Kerns `bar` (Herleitung und Herkunft im Kopf von `StabStimmen.hpp`)
 */
#include "InstrumentMeasure/StabStimmen.hpp"

#include "InstrumentCore/Pool.hpp"
#include "InstrumentCore/Tonkern.hpp"
#include "InstrumentMeasure/Bausteine.hpp"
#include "InstrumentModels/stab/StabKern.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <span>
#include <string>
#include <vector>

namespace lumi::messung
{

using namespace kern::stab;
using kern::Belegkraft;
using kern::Kurve;

namespace
{

double hzVon(int note)
{
    return 440.0 * std::exp2((note - 69) / 12.0);
}

double rms(std::span<const float> werte)
{
    if (werte.empty()) return 0.0;
    double s = 0.0;
    for (const float w : werte) s += static_cast<double>(w) * w;
    return std::sqrt(s / static_cast<double>(werte.size()));
}

double dB(double x)
{
    return x > 0.0 ? 20.0 * std::log10(x) : -300.0;
}

Kurve konstant(double wert, std::string herkunft, std::vector<std::string> quellen, const StabSpiel& s)
{
    Kurve k = kern::kurveKonstant(wert, Belegkraft::Rechnung, std::move(herkunft), s.noteMin, s.noteMax);
    k.quellen = std::move(quellen);
    return k;
}

bool enthaelt(const std::vector<std::string>& namen, std::string_view name)
{
    return std::find(namen.begin(), namen.end(), name) != namen.end();
}

}  // namespace

Kurve stabPartnerPegel(const StabSpiel& spiel)
{
    if (spiel.partner == PartnerArt::Keiner || (spiel.abnahme != Abnahme::Akustisch && spiel.abnahme != Abnahme::Steg))
        return konstant(1.0, "without effect: no acoustic partner", {"lumiviz"}, spiel);
    // IC setzePartnerAbstrahl (vor normiere: Bezug 1)
    StabSpiel s = spiel;
    s.ausgangBezug = 1.0;
    char ziel[96];
    std::snprintf(ziel, sizeof ziel, "+%g dB with the partner at key %d", s.partnerZielDb, s.partnerZielNote);
    const std::string herkunft = std::string("tuned through the model: ") + ziel + " (\"by as much as 15 dB\", Rucz & Ulveczki 2021 p. 7)";
    StabAnschlag a;
    a.note = s.partnerZielNote;
    a.hz = hzVon(s.partnerZielNote);
    a.stufe = -6;
    const auto frames = static_cast<std::int64_t>(0.5 * a.rate);
    StabSpiel ohne = s;
    ohne.partner = PartnerArt::Keiner;
    std::vector<float> ton;
    rechneStabTon(ohne, a, ton, frames);
    const double bezug = rms(ton);
    double unten = -6.0, oben = 6.0;  // log10 des Gewichts
    for (int i = 0; i < 40; ++i)
    {
        const double mitte = 0.5 * (unten + oben);
        s.partnerAbstrahl = konstant(std::pow(10.0, mitte), herkunft, {"rucz2021"}, s);
        rechneStabTon(s, a, ton, frames);
        if (dB(rms(ton) / bezug) < s.partnerZielDb)
            unten = mitte;
        else
            oben = mitte;
    }
    return konstant(std::pow(10.0, 0.5 * (unten + oben)), herkunft, {"rucz2021"}, s);
}

double stabSpielHz(const StabMessung& messung, int rate)
{
    if (messung.gehalten.empty()) return 0.0;
    const Tonhoehe th = tonhoehe(messung.gehalten, rate, 30.0, 4000.0);
    std::vector<double> hz;
    for (const double x : th.hz)
        if (x > 0.0) hz.push_back(x);
    if (hz.size() <= th.hz.size() / 2) return 0.0;
    std::nth_element(hz.begin(), hz.begin() + static_cast<std::ptrdiff_t>(hz.size() / 2), hz.end());
    return hz[hz.size() / 2];
}

BogenFenster stabBogenFenster(const StabSpiel& s)
{
    BogenFenster aus;
    if (!s.bogen)
    {
        for (int i = 0; i < 3; ++i)
        {
            aus.unten[static_cast<std::size_t>(i)] = konstant(1.0, "without effect: no bow", {"lumiviz"}, s);
            aus.oben[static_cast<std::size_t>(i)] = konstant(1.0, "without effect: no bow", {"lumiviz"}, s);
        }
        return aus;
    }
    // IC stimmeBogen (Herleitung im Kopf)
    std::vector<double> raster;
    for (double f = 1e-2; f <= 200.0; f *= std::pow(2.0, 0.125)) raster.push_back(f);
    std::vector<int> noten;
    for (int n = s.noteMin; n <= s.noteMax; n += 3) noten.push_back(n);
    if (noten.back() != s.noteMax) noten.push_back(s.noteMax);
    const std::array<double, 3> vbs = {s.bogenVbPp, std::sqrt(s.bogenVbPp * s.bogenVbFf), s.bogenVbFf};
    std::vector<std::array<double, 2>> fenster(3 * noten.size(), {0.0, 0.0});
    kern::parallelJe(3 * noten.size(), [&](std::size_t jj) {
        const std::size_t j = jj / 3, stufe = jj % 3;
        const int note = noten[j];
        const double vb = vbs[stufe];
        std::vector<double> pegel(raster.size(), 0.0), cent(raster.size(), 1e9), haft(raster.size(), 0.0);
        for (std::size_t i = 0; i < raster.size(); ++i)
        {
            StabAnschlag a;
            a.note = note;
            a.hz = 440.0 * std::exp2((note - 69) / 12.0);
            a.gestrichen = true;
            a.haltenS = 0.8;
            a.bogenKraftN = raster[i];
            a.bogenVbMS = vb;
            std::vector<float> ton;
            StabMessung m;
            const auto frames = static_cast<std::int64_t>(0.8 * a.rate);
            rechneStabTon(s, a, ton, frames, &m);
            double e = 0.0;
            for (std::int64_t k = frames / 2; k < frames; ++k) e += double(ton[static_cast<std::size_t>(k)]) * ton[static_cast<std::size_t>(k)];
            pegel[i] = e;
            const double spielHz = stabSpielHz(m, a.rate);
            if (spielHz > 0.0) cent[i] = 1200.0 * std::log2(spielHz / (a.hz * kern::wertAn(s.grundJeNote, note, kern::Skala::Linear)));
            haft[i] = m.haftAnteil;
        }
        std::vector<bool> spielt(raster.size(), false);
        for (std::size_t i = 0; i < raster.size(); ++i)
            spielt[i] = pegel[i] > 100.0 * pegel[0] && std::abs(cent[i]) < 15.0 && haft[i] > 0.05 && haft[i] < 0.98;
        std::size_t besterA = 0, besteL = 0;
        for (std::size_t i = 0; i < raster.size();)
        {
            if (!spielt[i])
            {
                ++i;
                continue;
            }
            std::size_t k = i;
            while (k < raster.size() && spielt[k]) ++k;
            if (k - i > besteL)
            {
                besteL = k - i;
                besterA = i;
            }
            i = k;
        }
        if (besteL == 0) return;
        std::size_t lo = besterA, hi = besterA + besteL - 1;
        if (besteL >= 4)
        {
            ++lo;
            --hi;
        }
        fenster[jj] = {raster[lo], raster[hi]};
    });
    for (std::size_t stufe = 0; stufe < 3; ++stufe)
        for (int q = 0; q < 2; ++q)
        {
            Kurve k;
            k.belegkraft = Belegkraft::Rechnung;
            k.herkunft = "window of the bow force at the model (tuner): starts (+20 dB), +-15 cent at f_1, sticking 5-98 %";
            k.quellen = {"lumiviz"};
            for (std::size_t j = 0; j < noten.size(); ++j)
            {
                const double w = fenster[3 * j + stufe][static_cast<std::size_t>(q)];
                if (w > 0.0) k.punkte.push_back({noten[j], w});
            }
            (q == 0 ? aus.unten : aus.oben)[stufe] = k;
        }
    return aus;
}

Stimmer stabStimmer()
{
    return Stimmer{std::string(kKernName), [](const kern::Fabrik& fabrik, const kern::Werte& werte, const std::vector<std::string>& namen) {
                       StabSpiel s = spielAus(werte);
                       GestimmteWerte aus;
                       // KS2 a: zuerst der Pegel des Partners, die Fenster nehmen ihn
                       if (enthaelt(namen, kPartnerPegel))
                       {
                           s.partnerAbstrahl = stabPartnerPegel(s);
                           aus.emplace_back(std::string(kPartnerPegel), s.partnerAbstrahl);
                       }
                       bool fensterNeu = false;
                       for (int i = 0; i < 3; ++i) fensterNeu = fensterNeu || enthaelt(namen, kBogenUnten[i]) || enthaelt(namen, kBogenOben[i]);
                       if (!fensterNeu) return aus;
                       // IC: die Fenster nach normiere — mit dem Bezug des Spiels, der zu diesem Pegel gehoert
                       if (s.bogen)
                       {
                           kern::Werte w = werte;
                           w.setze(std::string(kPartnerPegel), s.partnerAbstrahl);
                           // die Probe des Bezugs ist angeschlagen: die Fenster wirken dort nicht — fehlen sie (beim
                           // ersten Stimmen), steht ein Platzhalter, damit die Tafel die Werte annimmt
                           for (int i = 0; i < 3; ++i)
                               for (const std::string_view n : {kBogenUnten[i], kBogenOben[i]})
                                   if (const kern::Wert* alt = w.finde(n); alt == nullptr || !std::holds_alternative<Kurve>(*alt) ||
                                                                           std::get<Kurve>(*alt).punkte.empty())
                                       w.setze(std::string(n), konstant(1.0, "placeholder", {"lumiviz"}, s));
                           const double bezug = kern::normiereNachTafel(fabrik, kKernName, w);
                           s.ausgangBezug = bezug > 0.0 ? bezug : 1.0;
                       }
                       const BogenFenster f = stabBogenFenster(s);
                       for (int i = 0; i < 3; ++i)
                       {
                           if (enthaelt(namen, kBogenUnten[i])) aus.emplace_back(std::string(kBogenUnten[i]), f.unten[static_cast<std::size_t>(i)]);
                           if (enthaelt(namen, kBogenOben[i])) aus.emplace_back(std::string(kBogenOben[i]), f.oben[static_cast<std::size_t>(i)]);
                       }
                       return aus;
                   }};
}

}  // namespace lumi::messung
