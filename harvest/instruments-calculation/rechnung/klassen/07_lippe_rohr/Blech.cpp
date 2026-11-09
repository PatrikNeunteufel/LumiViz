/**
 * @file Blech.cpp
 * @brief Klasse 7: der Ton eines Blechblasinstruments (Herleitung im Kopf von `Blech.hpp`)
 */
#include "klassen/07_lippe_rohr/Blech.hpp"

#include "gemeinsam/Rohr.hpp"
#include "gemeinsam/Rohrschleife.hpp"
#include "gemeinsam/Werkzeug.hpp"
#include "messung/Bausteine.hpp"

#include <algorithm>
#include <bit>
#include <cmath>
#include <numbers>

namespace ic::blech
{

namespace
{

constexpr double kPi = std::numbers::pi;
constexpr double kAusblendenS = 0.005;

double lin(const Kurve& k, int n)
{
    return wertAn(k, n, Skala::Linear);
}

/// Was der Ton je Note der Kette braucht (aus den Kurven oder beim Stimmen vorgegeben).
struct Steuerung
{
    std::vector<double> lippeJeNote;  // f_L/f
    std::vector<double> zusatz;       // m
};

/// Die Filter fuer Daempfer `index` (0 = offen).
SchleifenFilter filterFuer(const BlechSpiel& s, int index, int rate, bool zu = false)
{
    Bohrung rest = s.rest;
    if (index > 0)
    {
        const Daempfer& d = s.daempfer[static_cast<std::size_t>(index - 1)];
        rest.daempferHz = d.hz;
        rest.daempferQ = d.q;
        rest.daempferRest = zu ? d.restZu : d.restFlaeche;
    }
    return schleifenFilter(rest, s.zylinderRadiusM, s.zylinderGrundM + 0.5 * s.zugMaxM, rate, Luft{}, 2048);
}

/// Der Vokaltrakt in Reihe (zwei Resonanzen, Fletcher et al. 2006 II) als Moden der Impedanz.
std::vector<ImpedanzMode> vokalModen(const BlechSpiel& s, double vokal)
{
    // die Spur „Vokal“ (T3c a) verschiebt beide Resonanzen; die Abbildung u → i ist offen (Vorgabe: unveraendert)
    (void)vokal;
    return {{s.vokalHz1, s.vokalQ, s.vokalSpitzePa * s.vokalGewicht}, {s.vokalHz2, s.vokalQ, s.vokalSpitzePa * s.vokalGewicht}};
}

/// Der Ton mit vorgegebener Steuerung — die gemeinsame Rechnung von `rechneBlechTon` und `stimme`.
void simuliere(const BlechSpiel& s, const BlechAnschlag& a, const Steuerung& st, std::vector<float>& mono, std::int64_t frames,
               BlechMessung* messung)
{
    mono.assign(static_cast<std::size_t>(std::max<std::int64_t>(0, frames)), 0.0f);
    if (frames <= 0 || a.rate <= 0 || a.kette.empty() || !s.spielbar) return;
    const double fs = a.rate;
    const double dt = 1.0 / fs;
    const Luft luft;
    const double zylinder0 = s.zylinderGrundM + st.zusatz.front();
    const int index = a.daempfer >= 0 && a.daempfer < static_cast<int>(s.daempfer.size()) ? a.daempfer + 1 : 0;
    SchleifenFilter f;
    if (s.filter && static_cast<std::size_t>(index) < s.filter->size())
        f = (*s.filter)[static_cast<std::size_t>(index)];
    else
        f = filterFuer(s, index, a.rate);
    Rohrschleife schleife(std::move(f), s.kesselM3, zylinder0, a.rate, luft);
    schleife.setzeNichtlinear(s.beta);
    // Wah-Wah: der Satz bei geschlossener Hand, gemischt nach der Oeffnung
    const bool mitWah = index > 0 && !a.wah.empty() && s.daempfer[static_cast<std::size_t>(index - 1)].restZu > 0.0;
    if (mitWah)
    {
        if (s.filterZu && static_cast<std::size_t>(index) < s.filterZu->size() && !(*s.filterZu)[static_cast<std::size_t>(index)].reflexion.empty())
            schleife.setzeZweitFilter((*s.filterZu)[static_cast<std::size_t>(index)]);
        else
            schleife.setzeZweitFilter(filterFuer(s, index, a.rate, true));
    }
    const auto oeffnung = [&](double t) {
        if (t <= a.wah.front().first) return a.wah.front().second;
        for (std::size_t k = 1; k < a.wah.size(); ++k)
            if (t <= a.wah[k].first)
            {
                const auto& [t0, o0] = a.wah[k - 1];
                const auto& [t1, o1] = a.wah[k];
                return t1 > t0 ? o0 + (o1 - o0) * (t - t0) / (t1 - t0) : o1;
            }
        return a.wah.back().second;
    };
    // Vokaltrakt (T5 a): p_Mund = P − Z_V·U, normal null
    const bool mitVokal = s.vokalGewicht > 0.0;
    RohrKern vokal(vokalModen(s, a.vokal), fs);
    // Growl: Mundraum zwischen Stimmlippen und Lippe
    const double mundC = s.mundM3 / (luft.dichte * luft.schall * luft.schall);
    double pMund = 0.0;
    // die Lippe
    double y = 0.0, v = 0.0;
    double zusatzJetzt = st.zusatz.front();
    double phaseVib = 0.0, phaseGrowl = 0.0;
    const auto halten = static_cast<std::int64_t>(std::llround(a.haltenS * fs));
    const double e = std::clamp(a.expression, 0.0, 1.0);
    double druckMax = s.druckMinPa * std::pow(s.druckMaxPa / s.druckMinPa, e);
    if (!s.schwelleJeNote.punkte.empty())
    {
        const double ps = wertAn(s.schwelleJeNote, std::clamp(a.kette.front().note, s.noteMin, s.noteMax), Skala::Logarithmisch);
        // pp knapp ueber der Schwelle, ff hoechstens das Sechsfache (offen) und nie ueber P_max
        const double von = 1.3 * ps, bis = std::max(von, std::min(s.druckMaxPa, 6.0 * ps));
        druckMax = von * std::pow(bis / von, e);
    }
    std::vector<double> summe(static_cast<std::size_t>(frames), 0.0);
    std::size_t aktiv = 0;
    const std::int64_t schwankungAb = std::min(frames, halten) * 4 / 5;
    double pkMin = 1e300, pkMax = -1e300;
    for (std::int64_t n = 0; n < frames; ++n)
    {
        const double t = static_cast<double>(n) * dt;
        while (aktiv + 1 < a.kette.size() && t >= a.kette[aktiv + 1].abS) ++aktiv;
        const KettenNote& kn = a.kette[aktiv];
        // der Druck: Anstieg, Halten, Absetzen
        double env = std::min(1.0, t / std::max(1e-4, s.anblasS));
        if (n >= halten) env *= std::max(0.0, 1.0 - (t - a.haltenS) / std::max(1e-4, s.absetzenS));
        const double druck = druckMax * env;
        // die Lippe: Note, Bend, Vibrato, Fall/Doit
        double cent = a.pitchCent;
        phaseVib += 2.0 * kPi * a.vibratoHz * dt;
        cent += a.vibratoCent * std::sin(phaseVib);
        const double rest0 = a.haltenS - a.endeLaengeS;
        if (a.ende != Endart::Normal && t > rest0)
        {
            const double anteil = std::min(1.0, (t - rest0) / std::max(1e-3, a.endeLaengeS));
            cent += (a.ende == Endart::Fall ? -1.0 : 1.0) * 100.0 * a.endeTiefe * anteil * anteil;
        }
        const double fL = kn.hz * st.lippeJeNote[aktiv] * std::exp2(cent / 1200.0);
        const double wL = 2.0 * kPi * fL;
        // der Zug: auf die Laenge der Note in `zugBewegungS`, mit dem Bend (eine Zugbewegung)
        if (s.zugMaxM > 0.0)
        {
            // der Bend (Pitch) als Zugbewegung: L·(2^(−c/1200) − 1); das Vibrato bleibt an der Lippe (T3a a)
            const double l = s.zylinderGrundM + st.zusatz[aktiv] + laenge(s.rest);
            const double ziel = st.zusatz[aktiv] + l * (std::exp2(-a.pitchCent / 1200.0) - 1.0);
            const double schritt = s.zugMaxM / std::max(1e-3, s.zugBewegungS) * dt;
            zusatzJetzt += std::clamp(ziel - zusatzJetzt, -schritt, schritt);
            schleife.setzeZylinderLaenge(s.zylinderGrundM + std::clamp(zusatzJetzt, 0.0, s.zugMaxM));
        }
        else if (aktiv > 0 || st.zusatz[aktiv] != zusatzJetzt)
        {
            zusatzJetzt = st.zusatz[aktiv];  // Ventile: der Weg wechselt sofort
            schleife.setzeZylinderLaenge(s.zylinderGrundM + zusatzJetzt);
        }
        if (mitWah) schleife.setzeMischung(1.0 - std::clamp(oeffnung(t), 0.0, 1.0));
        const double pk = schleife.kesselDruck();
        if (n >= schwankungAb && n < halten)
        {
            pkMin = std::min(pkMin, pk);
            pkMax = std::max(pkMax, pk);
        }
        // der Mund: ohne Growl der Druck der Lunge, mit Growl der Raum hinter dem zweiten Ventil
        double quelle = druck;
        if (a.growl)
        {
            phaseGrowl += 2.0 * kPi * kn.hz * std::exp2(a.growlIntervall / 12.0) * dt;
            const double offen = s.growlFlaecheM2 * 0.5 * (1.0 + std::sin(phaseGrowl));
            const double dpg = druck - pMund;
            const double ug = offen * std::sqrt(2.0 * std::abs(dpg) / luft.dichte) * (dpg >= 0.0 ? 1.0 : -1.0);
            quelle = pMund;
            pMund += dt * ug / mundC;  // der Fluss durch die Lippe geht unten ab
        }
        // Lippe offen, Fluss nach Bernoulli; mit Vokaltrakt geschlossen geloest: U = a·sgn(X − D·U)·√|X − D·U|
        const double h = std::max(0.0, s.lippeSpaltM + y);
        const double kA = s.lippeBreiteM * h * std::sqrt(2.0 / luft.dichte);
        double x = quelle - pk;
        double d = 0.0;
        if (mitVokal)
        {
            x -= vokal.vorher();
            d = vokal.durchgriff();
        }
        double u = 0.0;
        if (kA > 0.0)
        {
            const double a2 = kA * kA;
            if (x >= 0.0)
                u = 0.5 * (-a2 * d + std::sqrt(a2 * a2 * d * d + 4.0 * a2 * x));
            else
                u = -0.5 * (-a2 * d + std::sqrt(a2 * a2 * d * d - 4.0 * a2 * x));
        }
        const double dp = x - d * u;  // Druck ueber der Lippe
        if (mitVokal) vokal.nachher(u);
        if (a.growl) pMund -= dt * u / mundC;
        // die Lippe nach aussen schlagend
        v += dt * (dp / s.lippeMasseJeFlaeche - wL / s.lippeGuete * v - wL * wL * y);
        y += dt * v;
        const double abgestrahlt = schleife.schritt(u);
        summe[static_cast<std::size_t>(n)] = a.kesselAusgeben ? pk : abgestrahlt;
    }
    if (messung != nullptr)
    {
        messung->kesselSchwankung = pkMax > pkMin ? (pkMax - pkMin) / std::max(1.0, druckMax) : 0.0;
        // die Tonhoehe im gehaltenen Teil der letzten Note
        const double ab = std::max(a.kette.back().abS + 0.1, 0.5 * a.haltenS);
        const auto von = static_cast<std::size_t>(std::clamp(ab, 0.0, a.haltenS) * fs);
        const auto bis = static_cast<std::size_t>(std::min<std::int64_t>(frames, halten));
        if (bis > von + 4800)
        {
            std::vector<double> stueck(summe.begin() + static_cast<std::ptrdiff_t>(von), summe.begin() + static_cast<std::ptrdiff_t>(bis));
            const messung::Tonhoehe th = messung::tonhoehe(stueck, a.rate, 25.0, 2000.0);
            std::vector<double> hz;
            for (const double f : th.hz)
                if (f > 0.0) hz.push_back(f);
            if (hz.size() > th.hz.size() / 2)
            {
                std::nth_element(hz.begin(), hz.begin() + static_cast<std::ptrdiff_t>(hz.size() / 2), hz.end());
                messung->spielHz = hz[hz.size() / 2];
                messung->schwingt = true;
            }
        }
    }
    const auto blende = std::min<std::int64_t>(frames, std::max<std::int64_t>(1, std::llround(kAusblendenS * fs)));
    for (std::int64_t i = 0; i < frames; ++i)
    {
        double wert = summe[static_cast<std::size_t>(i)] / s.ausgangBezug;
        const std::int64_t rest1 = frames - 1 - i;
        if (rest1 < blende) wert *= static_cast<double>(rest1) / static_cast<double>(blende);
        mono[static_cast<std::size_t>(i)] = static_cast<float>(wert);
    }
}

/// Die Lippe je Note bei der Staerke `expression` aus den drei gestimmten Kurven (in e linear zwischen pp, mf, ff).
double lippeAn(const BlechSpiel& s, int note, double expression)
{
    const double leise = lin(s.lippeJeNoteLeise, note), mitte = lin(s.lippeJeNote, note), laut = lin(s.lippeJeNoteLaut, note);
    const double e = std::clamp(expression, 0.0, 1.0);
    return e < 0.5 ? leise + (mitte - leise) * e / 0.5 : mitte + (laut - mitte) * (e - 0.5) / 0.5;
}

}  // namespace

Bohrung ganzeBohrung(const BlechSpiel& s, double zusatzM)
{
    Bohrung b = s.rest;
    b.abschnitte.insert(b.abschnitte.begin(), Abschnitt{s.zylinderGrundM + zusatzM, s.zylinderRadiusM, s.zylinderRadiusM});
    return b;
}

void rechneBlechTon(const BlechSpiel& s, const BlechAnschlag& a, std::vector<float>& mono, std::int64_t frames, BlechMessung* messung)
{
    Steuerung st;
    for (const KettenNote& k : a.kette)
    {
        const int n = std::clamp(k.note, s.noteMin, s.noteMax);
        st.lippeJeNote.push_back(lippeAn(s, n, a.expression));
        st.zusatz.push_back(std::clamp(lin(s.zugJeNote, n), 0.0, std::max(s.zugMaxM, 10.0)));
    }
    simuliere(s, a, st, mono, frames, messung);
}

std::vector<double> registerSpitzen(const BlechSpiel& s, double zusatzM, double bisHz)
{
    // Z = 1/(iωC + 1/Z_Bohrung): der Kessel parallel zum Eingang der Bohrung
    const Luft luft;
    const Bohrung b = ganzeBohrung(s, zusatzM);
    const double c = s.kesselM3 / (luft.dichte * luft.schall * luft.schall);
    const auto betrag = [&](double hz) {
        const Komplex zb = eingangsImpedanz(b, hz, luft);
        return std::abs(1.0 / (Komplex(0.0, 2.0 * kPi * hz * c) + 1.0 / zb));
    };
    std::vector<double> spitzen;
    const double schritt = 0.5;
    double y0 = betrag(schritt), y1 = betrag(2 * schritt);
    for (double f = 3 * schritt; f <= bisHz; f += schritt)
    {
        const double y2 = betrag(f);
        if (y1 > y0 && y1 >= y2)
        {
            const double d = 0.5 * (y0 - y2) / (y0 - 2.0 * y1 + y2);
            spitzen.push_back(f - schritt + d * schritt);
        }
        y0 = y1;
        y1 = y2;
    }
    return spitzen;
}

void stimme(BlechSpiel& s, int schritt)
{
    if (!s.spielbar) return;
    Kurve leise, mitte, laut, zug, schwellen;
    for (Kurve* k : {&leise, &mitte, &laut, &zug, &schwellen})
    {
        k->belegkraft = Belegkraft::Rechnung;
        k->herkunft = "gestimmt durch das Modell hindurch: Register und Zuglage aus den Spitzen der Eingangsimpedanz mit "
                      "Kessel, dann die Spielfrequenz auf die Note (stimme); je Note ein Stuetzpunkt";
    }
    // die Spitzen auf einem Raster der Zuglage (ohne Zug nur eine)
    const int raster = s.zugMaxM > 0.0 ? 22 : 1;
    const double bis = 440.0 * std::exp2((s.noteMax + 3 - 69) / 12.0);
    std::vector<std::vector<double>> spitzen;
    for (int i = 0; i < raster; ++i) spitzen.push_back(registerSpitzen(s, raster > 1 ? s.zugMaxM * i / (raster - 1) : 0.0, bis));
    // die Ventile: je Teilmenge (Bit k = Ventil k gedrueckt) die Summe der Zusatzlaengen
    std::vector<double> griffe;
    if (!s.ventilM.empty() && s.zugMaxM <= 0.0)
        for (std::size_t g = 0; g < (std::size_t{1} << s.ventilM.size()); ++g)
        {
            double summe = 0.0;
            for (std::size_t k = 0; k < s.ventilM.size(); ++k)
                if (g & (std::size_t{1} << k)) summe += s.ventilM[k];
            griffe.push_back(summe);
        }
    // der Griff je Note am Modell (das Modell spielt ueber den Spitzen der Impedanz — die Spitzen allein waehlen falsch):
    // jeder Griff bei mf mit f_L/f = 0,82, der naechste gewinnt (je gedruecktes Ventil 3 Cent Aufschlag)
    const auto griffWahl = [&](int note, double stimmzug, double* cent) {
        const double f = 440.0 * std::exp2((note - 69) / 12.0);
        double beste = 1e300, wahl = stimmzug;
        for (std::size_t g = 0; g < griffe.size(); ++g)
        {
            BlechAnschlag a;
            a.kette = {{0.0, note, f}};
            a.expression = 0.5;
            a.haltenS = 0.5;
            Steuerung st{{0.82}, {griffe[g] + stimmzug}};
            std::vector<float> ton;
            BlechMessung m;
            simuliere(s, a, st, ton, static_cast<std::int64_t>(0.5 * a.rate), &m);
            if (!m.schwingt) continue;
            const double c = 1200.0 * std::log2(m.spielHz / f);
            if (std::abs(c) + 3.0 * std::popcount(g) < beste)
            {
                beste = std::abs(c) + 3.0 * std::popcount(g);
                wahl = griffe[g] + stimmzug;
                if (cent) *cent = c;
            }
        }
        return wahl;
    };
    // der Stimmzug (fuer das ganze Instrument): das obere Quartil der Abweichung beim besten Griff, als Laenge (f ∝ 1/L) —
    // die Lippe hebt den Ton leicht, senkt ihn aber kaum (unter f_L/f ≈ 0,75 springt das Register), also eher tief stimmen
    double stimmzug = 0.0;
    if (!griffe.empty())
    {
        std::vector<double> abw;
        for (int note = s.noteMin; note <= s.noteMax; note += 2)
        {
            double c = 0.0;
            griffWahl(note, 0.0, &c);
            abw.push_back(c);
        }
        std::sort(abw.begin(), abw.end());
        const double quartil = abw[(3 * abw.size()) / 4];
        stimmzug = std::clamp((s.zylinderGrundM + laenge(s.rest)) * (std::exp2(quartil / 1200.0) - 1.0), 0.0, 0.2);
    }
    const double lRest = laenge(s.rest);
    for (int note = s.noteMin; note <= s.noteMax; note += std::max(1, schritt))
    {
        const double f = 440.0 * std::exp2((note - 69) / 12.0);
        // 1. Register (Spitze n ≥ 2) und Zuglage: die kleinste Zuglage, bei der Spitze n die Note erreicht; mit Ventilen
        // der Griff am Modell (griffWahl) plus Stimmzug
        double zusatz = 0.0;
        if (!griffe.empty())
            zusatz = griffWahl(note, stimmzug, nullptr);
        else if (raster > 1)
        {
            bool gefunden = false;
            for (std::size_t n = 1; n < spitzen[0].size() && !gefunden; ++n)
                for (int i = 0; i + 1 < raster; ++i)
                {
                    if (n >= spitzen[static_cast<std::size_t>(i)].size() || n >= spitzen[static_cast<std::size_t>(i + 1)].size()) break;
                    const double fa = spitzen[static_cast<std::size_t>(i)][n], fb = spitzen[static_cast<std::size_t>(i + 1)][n];
                    if (f <= fa && f >= fb)
                    {
                        const double t = (fa - f) / (fa - fb);
                        zusatz = s.zugMaxM * (i + t) / (raster - 1);
                        gefunden = true;
                        break;
                    }
                }
        }
        // 2. die Anblasschwelle bei mf-Lippe (Halbierung in ln P zwischen 50 Pa und P_max)
        double schwelle = s.druckMaxPa;
        {
            double lo = std::log(50.0), hi = std::log(s.druckMaxPa);
            for (int i = 0; i < 9; ++i)
            {
                const double mitteP = 0.5 * (lo + hi);
                BlechSpiel t = s;
                t.schwelleJeNote = Kurve{};
                t.druckMinPa = t.druckMaxPa = std::exp(mitteP);
                BlechAnschlag a;
                a.kette = {{0.0, note, f}};
                a.expression = 0.0;
                a.haltenS = 0.4;
                Steuerung st{{0.9}, {zusatz}};  // die Lippe der Vorgabe (vor der Feinstimmung)
                std::vector<float> ton;
                BlechMessung m;
                simuliere(t, a, st, ton, static_cast<std::int64_t>(0.4 * a.rate), &m);
                if (m.kesselSchwankung >= 0.3)
                    hi = mitteP;
                else
                    lo = mitteP;
            }
            schwelle = std::exp(hi);
        }
        schwellen.punkte.push_back({note, schwelle});
        BlechSpiel mitSchwelle = s;
        mitSchwelle.schwelleJeNote = kurveAusPunkten({{note, schwelle}});
        // 3. Feinstimmung durch das Modell: mit Zug an der Zuglage (f ∝ 1/L), sonst an der Lippe
        std::array<double, 3> lippe = {0.9, 0.9, 0.9};
        const std::array<double, 3> staerke = {0.5, 0.0, 1.0};  // mf zuerst: sie legt den Zug fest
        if (!griffe.empty())
        {
            // Ventile: die Laenge steht mit dem Griff fest — die Lippe allein stimmt, im Sekantenverfahren (die
            // Spielfrequenz haengt steil an f_L/f: am offenen B♭4 0,8 → +4 Cent, 0,9 → +43 Cent); bei mf je Griff, der
            // kleinste Rest (plus 3 Cent je Ventil) legt den Griff fest
            const auto lippeStimmen = [&](double z, double e, double rStart, double& bestF) {
                double r = rStart;
                double rAlt = 0.0, fAlt = 0.0, bestR = r;
                bestF = 1e300;
                for (int runde = 0; runde < 10; ++runde)
                {
                    BlechAnschlag a;
                    a.kette = {{0.0, note, f}};
                    a.expression = e;
                    a.haltenS = 0.6;
                    Steuerung st{{r}, {z}};
                    std::vector<float> ton;
                    BlechMessung m;
                    simuliere(mitSchwelle, a, st, ton, static_cast<std::int64_t>(0.6 * a.rate), &m);
                    if (!m.schwingt)
                    {
                        r *= 1.03;
                        rAlt = 0.0;
                        continue;
                    }
                    const double fehler = 1200.0 * std::log2(m.spielHz / f);
                    if (std::abs(fehler) < std::abs(bestF))
                    {
                        bestF = fehler;
                        bestR = r;
                    }
                    if (std::abs(fehler) < 3.0) break;
                    double neu = r * std::exp2(-std::clamp(fehler, -300.0, 300.0) / 300.0);
                    if (rAlt > 0.0 && fehler != fAlt) neu = r - fehler * (r - rAlt) / (fehler - fAlt);
                    rAlt = r;
                    fAlt = fehler;
                    r = std::clamp(neu, 0.85 * r, 1.15 * r);
                }
                return bestR;
            };
            double besteKosten = 1e300;
            for (std::size_t g = 0; g < griffe.size(); ++g)
            {
                double rest = 0.0;
                const double r = lippeStimmen(griffe[g] + stimmzug, staerke[0], 0.85, rest);
                if (std::abs(rest) + 3.0 * std::popcount(g) < besteKosten)
                {
                    besteKosten = std::abs(rest) + 3.0 * std::popcount(g);
                    zusatz = griffe[g] + stimmzug;
                    lippe[0] = r;
                }
            }
            for (int k = 1; k < 3; ++k)
            {
                double rest = 0.0;
                lippe[static_cast<std::size_t>(k)] = lippeStimmen(zusatz, staerke[static_cast<std::size_t>(k)], lippe[0], rest);
            }
        }
        for (int k = 0; k < 3 && griffe.empty(); ++k)
        {
            double r = k == 0 ? 0.9 : lippe[0];
            for (int runde = 0; runde < 5; ++runde)
            {
                BlechAnschlag a;
                a.kette = {{0.0, note, f}};
                a.expression = staerke[static_cast<std::size_t>(k)];
                a.haltenS = 0.6;
                Steuerung st{{r}, {zusatz}};
                std::vector<float> ton;
                BlechMessung m;
                simuliere(mitSchwelle, a, st, ton, static_cast<std::int64_t>(0.6 * a.rate), &m);
                if (!m.schwingt)
                {
                    r *= 1.03;  // spricht nicht an: die Lippe naeher an die Note
                    continue;
                }
                const double fehler = 1200.0 * std::log2(m.spielHz / f);
                if (std::abs(fehler) < 3.0) break;
                if (raster > 1 && k == 0 && std::abs(fehler) < 300.0)
                    zusatz = std::clamp(zusatz + (s.zylinderGrundM + zusatz + lRest) * (m.spielHz / f - 1.0), 0.0, s.zugMaxM);
                else
                    r *= std::exp2(-std::clamp(fehler, -300.0, 300.0) / 1200.0);
            }
            lippe[static_cast<std::size_t>(k)] = r;
        }
        mitte.punkte.push_back({note, lippe[0]});
        leise.punkte.push_back({note, lippe[1]});
        laut.punkte.push_back({note, lippe[2]});
        zug.punkte.push_back({note, zusatz});
    }
    s.lippeJeNoteLeise = leise;
    s.lippeJeNote = mitte;
    s.lippeJeNoteLaut = laut;
    s.zugJeNote = zug;
    s.schwelleJeNote = schwellen;
}

void bereite(BlechSpiel& s)
{
    if (!s.spielbar)
    {
        s.filter.reset();
        s.filterZu.reset();
        return;
    }
    auto f = std::make_shared<std::vector<SchleifenFilter>>();
    auto zu = std::make_shared<std::vector<SchleifenFilter>>();
    for (int i = 0; i <= static_cast<int>(s.daempfer.size()); ++i)
    {
        f->push_back(filterFuer(s, i, 48000));
        zu->push_back(i > 0 && s.daempfer[static_cast<std::size_t>(i - 1)].restZu > 0.0 ? filterFuer(s, i, 48000, true) : SchleifenFilter{});
    }
    s.filter = f;
    s.filterZu = zu;
}

void richteBlechSpiel(BlechSpiel& s)
{
    s.lippeMasseJeFlaeche = std::clamp(s.lippeMasseJeFlaeche, 0.05, 20.0);
    s.lippeGuete = std::clamp(s.lippeGuete, 0.3, 50.0);
    s.lippeBreiteM = std::clamp(s.lippeBreiteM, 1e-3, 0.05);
    s.lippeSpaltM = std::clamp(s.lippeSpaltM, 0.0, 3e-3);
    s.druckMinPa = std::clamp(s.druckMinPa, 10.0, 2e4);
    s.druckMaxPa = std::clamp(s.druckMaxPa, s.druckMinPa, 3e4);
    s.beta = std::clamp(s.beta, 0.0, 2.0);
    s.vokalGewicht = std::clamp(s.vokalGewicht, 0.0, 1.0);
    s.zugMaxM = std::clamp(s.zugMaxM, 0.0, 2.0);
}

}  // namespace ic::blech
