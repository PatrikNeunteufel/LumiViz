/**
 * @file ZungeStimmen.cpp
 * @brief Messen am Kern `freeReed` (Herleitung im Kopf von `ZungeStimmen.hpp`)
 */
#include "InstrumentMeasure/ZungeStimmen.hpp"

#include "AudioCore/Lautheit.hpp"
#include "InstrumentCore/Pool.hpp"
#include "InstrumentMeasure/Bausteine.hpp"
#include "InstrumentModels/zunge/ZungeKern.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <string>
#include <utility>

namespace lumi::messung
{

using kern::zunge::Chor;
using kern::zunge::ZungenAnschlag;
using kern::zunge::ZungenMessung;
using kern::zunge::ZungenSpiel;

namespace
{

constexpr int kRate = 48000;
constexpr double kHaltenS = 1.0;
constexpr std::int64_t kFrames = 57600;  // 1,2 s: Halten und Absetzen

double hzVon(double note)
{
    return 440.0 * std::exp2((note - 69.0) / 12.0);
}

bool enthaelt(const std::vector<std::string>& namen, std::string_view name)
{
    return std::find(namen.begin(), namen.end(), name) != namen.end();
}

}  // namespace

ZungenTon messeZungenTon(std::span<const double> ton, double haltenS, int rate, double amplitudeRel)
{
    // IC Zunge.cpp:101–122 (die Zeitpunkte wie dort: `halten` in Frames, gerundet)
    ZungenTon m;
    m.amplitudeRel = amplitudeRel;
    const auto frames = static_cast<std::int64_t>(ton.size());
    const auto haltenF = static_cast<std::size_t>(std::min<std::int64_t>(frames, std::llround(haltenS * rate)));
    const auto ab = static_cast<std::size_t>(0.5 * haltenS * rate);
    if (haltenF > ab + 4800)
    {
        const std::vector<double> stueck(ton.begin() + static_cast<std::ptrdiff_t>(ab), ton.begin() + static_cast<std::ptrdiff_t>(haltenF));
        const Tonhoehe th = tonhoehe(stueck, rate, 30.0, 3000.0);
        std::vector<double> hz;
        for (const double x : th.hz)
            if (x > 0.0) hz.push_back(x);
        if (hz.size() > th.hz.size() / 2)
        {
            std::nth_element(hz.begin(), hz.begin() + static_cast<std::ptrdiff_t>(hz.size() / 2), hz.end());
            m.spielHz = hz[hz.size() / 2];
            m.schwingt = true;
        }
    }
    const std::vector<double> alles(ton.begin(), ton.begin() + static_cast<std::ptrdiff_t>(haltenF));
    m.einschwingS = einschwingen(alles, rate);
    return m;
}

ZungenTon messeEineZunge(const ZungenSpiel& spiel, double f0, int zungenNote, const ZungenAnschlag& a, std::int64_t frames, std::vector<double>* ton)
{
    std::vector<double> t;
    ZungenMessung k;
    kern::zunge::rechneEineZunge(spiel, f0, zungenNote, a, t, frames, &k);
    const ZungenTon m = messeZungenTon(t, a.haltenS, a.rate, k.amplitudeRel);
    if (ton != nullptr) *ton = std::move(t);
    return m;
}

ZungenTon messeErsteZunge(const ZungenSpiel& spiel, const ZungenAnschlag& a, std::int64_t frames)
{
    const bool bass = spiel.bassAb > 0 && a.note < spiel.bassAb;
    const std::vector<Chor>& choere = bass ? spiel.bassChoere : spiel.choere;
    if (choere.empty() || frames <= 0 || a.rate <= 0) return ZungenTon{};
    const kern::zunge::ZungeDesChors z = kern::zunge::zungeDesChors(spiel, choere.front(), a.note);
    return messeEineZunge(spiel, z.f0, z.note, a, frames);
}

ZungenTaste messeZungenTaste(const ZungenSpiel& spiel, double f0, int zungenNote, double druckPa, double ventilFaktor)
{
    // die Messkette des Pruefstands S190 (`messe`), die Lautheit mit AudioCore
    ZungenAnschlag a;
    a.note = zungenNote;
    a.druckPa = druckPa;
    a.haltenS = kHaltenS;
    a.ventilFaktor = ventilFaktor;
    a.rate = kRate;
    std::vector<double> aus, y;
    ZungenMessung k;
    k.auslenkung = &y;
    kern::zunge::rechneEineZunge(spiel, f0, zungenNote, a, aus, kFrames, &k);
    const kern::zunge::ZungeAnDerTaste z = kern::zunge::zungeAn(spiel, zungenNote);
    ZungenTaste m;
    const auto ende = static_cast<std::size_t>(kHaltenS * kRate);
    const std::size_t viertel = kRate / 4;
    const auto spanne = [&](std::size_t von, std::size_t bis, double& mitte) {
        double lo = 1e30, hi = -1e30;
        for (std::size_t i = von; i < bis; ++i)
        {
            lo = std::min(lo, y[i]);
            hi = std::max(hi, y[i]);
        }
        mitte = 0.5 * (lo + hi);
        return 0.5 * (hi - lo);
    };
    double mitte = 0.0, mitteVor = 0.0;
    const double amp = spanne(ende - viertel, ende, mitte);
    const double ampVor = spanne(ende - 2 * viertel, ende - viertel, mitteVor);
    m.ampLaenge = amp / z.laenge;
    m.ampRuhe = amp / z.ruhe;
    m.waechst = amp > 1.1 * ampVor;
    m.spricht = amp >= 0.25 * z.ruhe && !m.waechst;
    // Tonhoehe: aufsteigende Nulldurchgaenge um die Mitte
    double erster = -1.0, letzter = -1.0;
    int zahl = 0;
    for (std::size_t i = ende - viertel + 1; i < ende; ++i)
    {
        const double u = y[i - 1] - mitte, v = y[i] - mitte;
        if (u < 0.0 && v >= 0.0)
        {
            const double t = (static_cast<double>(i - 1) + u / (u - v)) / kRate;
            if (zahl == 0) erster = t;
            letzter = t;
            ++zahl;
        }
    }
    if (zahl >= 3) m.hz = (zahl - 1) / (letzter - erster);
    // Lautheit: Momentary (0,4 s) ueber das Ende des Haltens, mono als linker Kanal
    {
        const std::size_t fenster = kRate * 2 / 5;
        std::vector<float> stereo(2 * fenster, 0.0f);
        for (std::size_t i = 0; i < fenster; ++i) stereo[2 * i] = static_cast<float>(aus[ende - fenster + i]);
        audio::LautheitsMessung l(kRate);
        l.fuettere(stereo.data(), fenster);
        m.lufs = l.ergebnis().momentanMaxLufs;
    }
    // Einschwingen am Schall: Spitze je 5 ms
    {
        const std::size_t block = kRate / 200;
        std::vector<double> h;
        for (std::size_t i = 0; i + block <= ende; i += block)
        {
            double s = 0.0;
            for (std::size_t j = 0; j < block; ++j) s = std::max(s, std::abs(aus[i + j]));
            h.push_back(s);
        }
        double end = 0.0;
        for (std::size_t i = h.size() - 50; i < h.size(); ++i) end = std::max(end, h[i]);
        double t5 = -1.0, t50 = -1.0;
        for (std::size_t i = 0; i < h.size(); ++i)
        {
            if (t50 < 0.0 && h[i] >= end * std::pow(10.0, -50.0 / 20.0)) t50 = 0.005 * static_cast<double>(i);
            if (t5 < 0.0 && h[i] >= end * std::pow(10.0, -5.0 / 20.0)) t5 = 0.005 * static_cast<double>(i);
        }
        m.einS = t5;
        m.anstiegS = t5 - t50;
    }
    return m;
}

double stimmeZunge(const ZungenSpiel& spiel, int zungenNote, double druckPa, bool* spricht)
{
    const double f = hzVon(zungenNote);
    double f0 = 1.02 * f;
    bool ok = true;
    for (int runde = 0; runde < 6; ++runde)
    {
        const ZungenTaste m = messeZungenTaste(spiel, f0, zungenNote, druckPa);
        if (m.ampRuhe < 0.25 || m.hz <= 0.0)
        {
            ok = false;
            break;
        }
        if (std::abs(1200.0 * std::log2(m.hz / f)) < 0.05) break;
        f0 *= f / m.hz;
    }
    if (spricht != nullptr) *spricht = ok;
    return f0;
}

ZungenBend messeBend(const ZungenSpiel& spiel, double f0, int zungenNote, double druckPa)
{
    ZungenBend b;
    const auto miss = [&](double f) { return messeZungenTaste(spiel, f0, zungenNote, druckPa, f); };
    const ZungenTaste offen = miss(1.0);
    const auto cent = [&](const ZungenTaste& x) { return std::min(0.0, 1200.0 * std::log2(x.hz / offen.hz)); };
    if (!offen.spricht || offen.hz <= 0.0)
    {
        b.centJeStufe.assign(kern::zunge::kBendStufen, 0.0);
        return b;
    }
    // das Raster bis zur ersten Oeffnung, die nicht mehr anspricht (unter dem Raster weiter in Vierteln, hoechstens 2^(−4))
    double gutF = 1.0, gutC = 0.0, erste = 0.0;
    std::vector<double> raster;
    for (int k = 0; k < 16; ++k)
    {
        const double f = kern::zunge::bendStufe(k);
        const ZungenTaste x = miss(f);
        if (!x.spricht || x.hz <= 0.0)
        {
            erste = f;
            break;
        }
        gutF = f;
        gutC = cent(x);
        if (k < kern::zunge::kBendStufen) raster.push_back(gutC);
    }
    for (int r = 0; r < 12 && erste > 0.0 && gutF / erste > 1.01; ++r)
    {
        const double mitte = std::sqrt(gutF * erste);
        const ZungenTaste x = miss(mitte);
        if (x.spricht && x.hz > 0.0)
        {
            gutF = mitte;
            gutC = cent(x);
        }
        else
            erste = mitte;
    }
    b.grenzeOeffnung = gutF;
    b.grenzeCent = gutC;
    b.centJeStufe = raster;
    b.centJeStufe.resize(kern::zunge::kBendStufen, gutC);
    return b;
}

ZungenStimmung zungenStimmung(const ZungenSpiel& s)
{
    const std::size_t anzahl = static_cast<std::size_t>(std::max(0, s.noteMax - s.noteMin + 1));
    std::vector<double> f0(anzahl, 0.0);
    std::vector<char> ok(anzahl, 0);
    std::vector<ZungenBend> bend(anzahl);
    const double druck = kern::zunge::windMitte(s);
    kern::parallelJe(anzahl, [&](std::size_t i) {
        bool spricht = false;
        const int note = s.noteMin + static_cast<int>(i);
        f0[i] = stimmeZunge(s, note, druck, &spricht);
        ok[i] = spricht ? 1 : 0;
        if (s.bendUeberVentil && spricht)
            bend[i] = messeBend(s, f0[i], note, druck);
        else
            bend[i].centJeStufe.assign(kern::zunge::kBendStufen, 0.0);
    });
    const auto gestimmt = [](std::string herkunft) {
        kern::Kurve k;
        k.belegkraft = kern::Belegkraft::Rechnung;
        k.herkunft = std::move(herkunft);
        k.quellen = {"lumiviz"};
        return k;
    };
    ZungenStimmung st;
    st.eigenfrequenz = gestimmt("tuned through the model: f0 of the reed so that it plays the key at the middle wind, within 0.05 cent "
                                "(ZV5 a; zungenStimmung)");
    const std::string bendHerkunft = s.bendUeberVentil ? "tuned through the model (ZV8 a; messeBend at the middle wind)" : "no bend (ZV7 a)";
    for (int k = 0; k < kern::zunge::kBendStufen; ++k) st.bendCentJeStufe.push_back(gestimmt(bendHerkunft));
    st.bendGrenzeOeffnung = gestimmt(bendHerkunft);
    st.bendGrenzeCent = gestimmt(bendHerkunft);
    for (std::size_t i = 0; i < anzahl; ++i)
    {
        const int note = s.noteMin + static_cast<int>(i);
        st.eigenfrequenz.punkte.push_back({note, f0[i]});
        for (std::size_t k = 0; k < st.bendCentJeStufe.size(); ++k) st.bendCentJeStufe[k].punkte.push_back({note, bend[i].centJeStufe[k]});
        st.bendGrenzeOeffnung.punkte.push_back({note, bend[i].grenzeOeffnung});
        st.bendGrenzeCent.punkte.push_back({note, bend[i].grenzeCent});
        if (ok[i] == 0) st.stumm.push_back(note);
    }
    return st;
}

ZungenWindbereich messeWindbereich(const ZungenSpiel& s)
{
    ZungenWindbereich w;
    for (int n = s.noteMin; n <= s.noteMax; ++n) w.noten.push_back(n);
    const std::size_t anzahl = w.noten.size();
    w.min.resize(anzahl);
    w.mitte.resize(anzahl);
    w.max.resize(anzahl);
    const double druck[3] = {s.windMinPa, kern::zunge::windMitte(s), s.windMaxPa};
    kern::parallelJe(3 * anzahl, [&](std::size_t k) {
        const std::size_t i = k / 3;
        const int note = w.noten[i];
        const double f0 = kern::wertAn(s.eigenfrequenzJeNote, note, kern::Skala::Logarithmisch);
        const ZungenTaste m = messeZungenTaste(s, f0, note, druck[k % 3]);
        (k % 3 == 0 ? w.min : k % 3 == 1 ? w.mitte : w.max)[i] = m;
    });
    return w;
}

Stimmer zungeStimmer()
{
    using namespace kern::zunge;
    return Stimmer{std::string(kKernName), [](const kern::Fabrik&, const kern::Werte& werte, const std::vector<std::string>& namen) {
                       GestimmteWerte aus;
                       const std::vector<std::string> alle = gestimmteNamen();
                       if (std::none_of(alle.begin(), alle.end(), [&](const std::string& n) { return enthaelt(namen, n); })) return aus;
                       // alle entstehen zusammen (der Bend an der gestimmten f₀)
                       const ZungenStimmung st = zungenStimmung(spielAus(werte));
                       const auto gib = [&](const std::string& name, const kern::Kurve& k) {
                           if (enthaelt(namen, name)) aus.emplace_back(name, k);
                       };
                       gib(std::string(kEigenfrequenz), st.eigenfrequenz);
                       for (int k = 0; k < kBendStufen; ++k) gib(bendStufenName(k), st.bendCentJeStufe[static_cast<std::size_t>(k)]);
                       gib(std::string(kBendGrenzeOeffnung), st.bendGrenzeOeffnung);
                       gib(std::string(kBendGrenzeCent), st.bendGrenzeCent);
                       return aus;
                   }};
}

}  // namespace lumi::messung
