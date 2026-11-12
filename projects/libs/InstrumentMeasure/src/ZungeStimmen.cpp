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
#include <iterator>
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

double tonLautheit(const ZungenSpiel& s, int note, double staerke)
{
    ZungenAnschlag a;
    a.note = note;
    a.expression = staerke;
    a.haltenS = kHaltenS;
    a.rate = kRate;
    std::vector<float> t;
    kern::zunge::rechneZungenTon(s, a, t, kFrames);
    // Momentary (0,4 s) ueber das Ende des Haltens, auf beiden Kanaelen
    const std::size_t fenster = kRate * 2 / 5, ende = static_cast<std::size_t>(kHaltenS * kRate);
    std::vector<float> stereo(2 * fenster, 0.0f);
    for (std::size_t i = 0; i < fenster; ++i) stereo[2 * i] = stereo[2 * i + 1] = t[ende - fenster + i];
    audio::LautheitsMessung l(kRate);
    l.fuettere(stereo.data(), fenster);
    return l.ergebnis().momentanMaxLufs;
}

ZungenDynamik messeDynamik(const ZungenSpiel& s, int note)
{
    constexpr int kRaster = 21;  // Staerke 0, 0,05 … 1
    std::vector<double> rel(kRaster);
    for (int j = 0; j < kRaster; ++j) rel[static_cast<std::size_t>(j)] = tonLautheit(s, note, j / 20.0);
    const double voll = rel.back();
    for (double& x : rel) x -= voll;
    ZungenDynamik d;
    d.dynamikLu = -rel.front();
    for (int k = 0; k < kern::zunge::kStaerkeStufen; ++k)
    {
        const double ziel = kern::zunge::staerkeStufeDb(k);
        if (ziel <= rel.front())
        {
            d.staerkeJeStufe.push_back(0.0);  // unter der Dynamik der Taste
            continue;
        }
        // von oben die erste Stelle, an der die Kurve den Pegel erreicht: das groesste j mit rel[j] ≤ ziel < rel[j + 1]
        int j = kRaster - 2;
        while (j > 0 && rel[static_cast<std::size_t>(j)] > ziel) --j;
        double lo = j / 20.0, hi = (j + 1) / 20.0;
        double lLo = rel[static_cast<std::size_t>(j)], lHi = rel[static_cast<std::size_t>(j + 1)];
        double st = -1.0;
        for (int r = 0; r < 14 && st < 0.0; ++r)
        {
            const double mitte = 0.5 * (lo + hi);
            const double l = tonLautheit(s, note, mitte) - voll;
            if (std::abs(l - ziel) < 0.05)
                st = mitte;
            else if (l < ziel)
            {
                lo = mitte;
                lLo = l;
            }
            else
            {
                hi = mitte;
                lHi = l;
            }
        }
        if (st < 0.0) st = lHi > lLo ? lo + (ziel - lLo) / (lHi - lLo) * (hi - lo) : lo;
        d.staerkeJeStufe.push_back(std::clamp(st, 0.0, 1.0));
    }
    return d;
}

double messeNachklang(const ZungenSpiel& s, int note)
{
    double laengster = 0.0;
    for (const double h : kNachklangHalten)
        for (const double e : kNachklangStaerken) laengster = std::max(laengster, messeNachklang(s, note, h, e));
    return laengster;
}

double messeNachklang(const ZungenSpiel& s, int note, double haltenS, double staerke)
{
    ZungenAnschlag a;
    a.note = note;
    a.expression = staerke;
    a.haltenS = haltenS;
    a.rate = kRate;
    const auto ende = static_cast<std::size_t>(std::llround(haltenS * kRate));
    const std::size_t block = kRate / 200;
    std::vector<float> t;
    std::vector<double> bloecke;
    double nachS = 8.0;
    for (;;)
    {
        kern::zunge::rechneZungenTon(s, a, t, static_cast<std::int64_t>(ende) + static_cast<std::int64_t>(nachS * kRate));
        // die Bloecke ab dem Beginn des Tons; der lauteste ueber die ganze Note, auch nach dem Wind (ZV13 a)
        bloecke.clear();
        for (std::size_t k = 0; k + block <= t.size(); k += block)
        {
            double e = 0.0;
            for (std::size_t j = 0; j < block; ++j) e += double(t[k + j]) * double(t[k + j]);
            bloecke.push_back(e / static_cast<double>(block));
        }
        const double ref = bloecke.empty() ? 0.0 : *std::max_element(bloecke.begin(), bloecke.end());
        std::size_t letzter = ende;
        for (std::size_t b = ende / block; b < bloecke.size(); ++b)
            if (bloecke[b] > ref * 1e-6) letzter = (b + 1) * block;
        // ausgeklungen, wenn die letzte Viertelsekunde der Rechnung still ist (die Blende am Ende zaehlt nicht)
        if (letzter + kRate / 4 <= t.size() || nachS >= 32.0) return static_cast<double>(letzter - ende) / kRate;
        nachS *= 2.0;
    }
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
    // die zweite Runde am ganzen Ton (ZV10 a, ZV11 a): mit der gestimmten f₀ aller Noten — die Fusslagen einer Taste
    // spielen auf anderen Noten
    ZungenSpiel mitF0 = s;
    mitF0.eigenfrequenzJeNote = st.eigenfrequenz;
    mitF0.bendCentJeStufe = st.bendCentJeStufe;
    mitF0.bendGrenzeOeffnung = st.bendGrenzeOeffnung;
    mitF0.bendGrenzeCent = st.bendGrenzeCent;
    // je Taste die Dynamik und die 18 Ausklaenge (ZV13 a) als eigene Auftraege auf dem Pool
    constexpr std::size_t kH = std::size(kNachklangHalten), kS = std::size(kNachklangStaerken), kJe = 1 + kH * kS;
    std::vector<ZungenDynamik> dynamik(anzahl);
    std::vector<double> ausklang(anzahl * kH * kS, 0.0);
    kern::parallelJe(kJe * anzahl, [&](std::size_t k) {
        const std::size_t i = k / kJe, j = k % kJe;
        const int note = s.noteMin + static_cast<int>(i);
        if (j == 0)
            dynamik[i] = messeDynamik(mitF0, note);
        else
            ausklang[i * kH * kS + j - 1] = messeNachklang(mitF0, note, kNachklangHalten[(j - 1) / kS], kNachklangStaerken[(j - 1) % kS]);
    });
    std::vector<double> nachklang(anzahl, 0.0);
    for (std::size_t i = 0; i < anzahl; ++i)
        nachklang[i] = *std::max_element(ausklang.begin() + static_cast<std::ptrdiff_t>(i * kH * kS),
                                         ausklang.begin() + static_cast<std::ptrdiff_t>((i + 1) * kH * kS));
    for (int k = 0; k < kern::zunge::kStaerkeStufen; ++k)
        st.staerkeJeStufe.push_back(gestimmt("tuned through the model (ZV10 a; messeDynamik: the whole tone of the key, within 0.05 LU)"));
    st.nachklang = gestimmt("tuned through the model (ZV11 a, ZV13 a; messeNachklang: the longest release over six hold times and three "
                            "strengths, down to -60 dB under the loudest 5 ms block of the note)");
    for (std::size_t i = 0; i < anzahl; ++i)
    {
        const int note = s.noteMin + static_cast<int>(i);
        for (std::size_t k = 0; k < st.staerkeJeStufe.size(); ++k) st.staerkeJeStufe[k].punkte.push_back({note, dynamik[i].staerkeJeStufe[k]});
        st.nachklang.punkte.push_back({note, nachklang[i]});
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
                       for (int k = 0; k < kStaerkeStufen; ++k) gib(staerkeStufenName(k), st.staerkeJeStufe[static_cast<std::size_t>(k)]);
                       gib(std::string(kNachklang), st.nachklang);
                       return aus;
                   }};
}

}  // namespace lumi::messung
