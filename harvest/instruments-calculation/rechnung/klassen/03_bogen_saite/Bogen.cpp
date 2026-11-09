/**
 * @file Bogen.cpp
 * @brief Klasse 3: der Ton eines Streichinstruments (Herleitung im Kopf von `Bogen.hpp`)
 */
#include "klassen/03_bogen_saite/Bogen.hpp"

#include "gemeinsam/Reibung.hpp"
#include "gemeinsam/Werkzeug.hpp"
#include "messung/Bausteine.hpp"

#include <algorithm>
#include <cmath>
#include <complex>
#include <numbers>

namespace ic::bogen
{

namespace
{

constexpr double kPi = std::numbers::pi;
constexpr int kUeber = 2;            // doppelt abgetastet (der Steg-Abschnitt der hohen Noten ist kurz)
constexpr std::size_t kRing = 1u << 14;
constexpr double kAusblendenS = 0.005;

double hzVon(int note)
{
    return 440.0 * std::exp2((note - 69) / 12.0);
}

/// Eine Verzoegerung mit Lagrange-Interpolation 3. Ordnung (Verzoegerung ≥ 2 Takte).
struct Ring
{
    std::vector<double> x = std::vector<double>(kRing, 0.0);
    std::size_t pos = 0;
    [[nodiscard]] double lies(double d) const
    {
        d = std::clamp(d, 2.0, static_cast<double>(kRing - 4));
        const auto i = static_cast<std::size_t>(std::floor(d));
        const double f = d - static_cast<double>(i);
        const auto at = [&](std::size_t k) { return x[(pos + kRing - k) % kRing]; };
        // Punkte bei i−1, i, i+1, i+2 Takten zurueck; f in [0, 1) zwischen i und i+1
        const double t = f + 1.0;  // Lage relativ zum Punkt i−1
        const double l0 = -(t - 1.0) * (t - 2.0) * (t - 3.0) / 6.0;
        const double l1 = t * (t - 2.0) * (t - 3.0) / 2.0;
        const double l2 = -t * (t - 1.0) * (t - 3.0) / 2.0;
        const double l3 = t * (t - 1.0) * (t - 2.0) / 6.0;
        return l0 * at(i - 1) + l1 * at(i) + l2 * at(i + 1) + l3 * at(i + 2);
    }
    void schreibe(double v)
    {
        x[pos] = v;
        pos = (pos + 1) % kRing;
    }
};

/// Der Verlust je Umlauf erster Ordnung y = b·x + a·y₁: Teilton k verliert e^(−πk/Q) je Periode (konstante Guete).
struct Verlust
{
    double a = 0.0, b = 1.0, y1 = 0.0;
    double verzoegerung = 0.0;  ///< Phasenlaufzeit bei f in Takten
    void setze(double q, double f, double fs)
    {
        // H = g·(1 − a)/(1 − a·z⁻¹): das Verhaeltnis |H(8f)|/|H(f)| faellt mit a von 1 ab — a aus ihm, dann g aus
        // |H(f)|; der Gleichanteil g hoechstens 1 (sonst waechst der Ton)
        const double g1 = std::exp(-kPi / q), g8 = std::exp(-8.0 * kPi / q);
        const double w1 = 2.0 * kPi * f / fs, w8 = std::min(kPi, 8.0 * w1);
        const auto betrag = [](double aa, double w) { return std::abs(1.0 - aa * std::polar(1.0, -w)); };
        double lo = 0.0, hi = 0.999;
        for (int i = 0; i < 50; ++i)
        {
            const double m = 0.5 * (lo + hi);
            if (betrag(m, w1) / betrag(m, w8) > g8 / g1)
                lo = m;
            else
                hi = m;
        }
        a = 0.5 * (lo + hi);
        const double g = std::min(1.0, g1 * betrag(a, w1) / (1.0 - a));
        b = g * (1.0 - a);
        const std::complex<double> h = b / (1.0 - a * std::polar(1.0, -w1));
        verzoegerung = -std::arg(h) / w1;
    }
    double schritt(double x)
    {
        y1 = b * x + a * y1;
        return y1;
    }
};

struct NotenStand
{
    double c = 200.0;       ///< Wellengeschwindigkeit der Saite
    double z0 = 0.55;
    double laenge = 0.69;   ///< gegriffene Laenge
    double hz = 146.8;
};

NotenStand standFuer(const StreichSpiel& s, int note, double hz)
{
    NotenStand st;
    const StreichSaite* saite = &s.saiten.front();
    for (const StreichSaite& x : s.saiten)
        if (x.leerNote <= note) saite = &x;
    const double fLeer = hzVon(saite->leerNote);
    st.c = 2.0 * s.mensurM * fLeer;
    st.z0 = saite->z0;
    const double korrektur = s.laengeJeNote.punkte.empty() ? 1.0 : wertAn(s.laengeJeNote, std::clamp(note, s.noteMin, s.noteMax), Skala::Logarithmisch);
    st.laenge = s.mensurM * fLeer / hz * korrektur;
    st.hz = hz;
    return st;
}

const StreichSaite& saiteFuer(const StreichSpiel& s, int note)
{
    const StreichSaite* saite = &s.saiten.front();
    for (const StreichSaite& x : s.saiten)
        if (x.leerNote <= note) saite = &x;
    return *saite;
}

}  // namespace

void rechneStreichTon(const StreichSpiel& s, const StreichAnschlag& a, std::vector<float>& mono, std::int64_t frames, StreichMessung* messung)
{
    mono.assign(static_cast<std::size_t>(std::max<std::int64_t>(0, frames)), 0.0f);
    if (frames <= 0 || a.rate <= 0 || a.kette.empty() || !s.spielbar || s.saiten.empty()) return;
    const double fs = static_cast<double>(a.rate) * kUeber;
    const double dt = 1.0 / fs;
    const std::int64_t n2 = frames * kUeber;
    // der Korpus (con sordino: die Moden ueber 600 Hz tiefer — „shift the bridge hill downwards“, Woodhouse 2014)
    std::vector<ImpedanzMode> korpus = s.korpus;
    if (a.daempfer)
        for (ImpedanzMode& m : korpus)
            if (m.hz > 600.0)
            {
                m.hz *= s.daempferFaktor;
                m.spitze *= s.daempferStaerke;
            }
    RohrKern steg(korpus, fs);
    const double dSteg = steg.durchgriff();
    std::vector<NotenStand> staende;
    for (const KettenNote& k : a.kette) staende.push_back(standFuer(s, k.note, k.hz));
    Verlust verlust;
    verlust.setze(saiteFuer(s, a.kette.front().note).guete, staende.front().hz, fs);
    Ring zumSattel, vomSattel, zumSteg, vomSteg;
    const double e = std::clamp(a.expression, 0.0, 1.0);
    const double vbZiel = s.vbPp * std::pow(s.vbFf / s.vbPp, e);
    const auto kraftZiel = [&](int note) {
        if (a.kraftN > 0.0) return a.kraftN;
        const int n = std::clamp(note, s.noteMin, s.noteMax);
        const auto an = [&](const Kurve& k) { return k.punkte.empty() ? 0.5 : wertAn(k, n, Skala::Logarithmisch); };
        const double u = std::pow(an(s.kraftUntenJeNote), 1.0 - e) * std::pow(an(s.kraftUntenFfJeNote), e);
        const double o = std::max(u, std::pow(an(s.kraftObenJeNote), 1.0 - e) * std::pow(an(s.kraftObenFfJeNote), e));
        // in der geometrischen Mitte des Fensters bei dieser Staerke (offen: die Kraft traegt die Obertoene, Askenfelt —
        // nur Bericht)
        return std::sqrt(u * o);
    };
    double abstand = s.bogenAbstandM * (a.lage == Lage::Ponticello ? 0.5 : a.lage == Lage::Tasto ? 2.0 : 1.0);
    const auto halten = static_cast<std::int64_t>(std::llround(a.haltenS * fs));
    std::vector<double> aus(static_cast<std::size_t>(n2), 0.0);
    std::size_t aktiv = 0;
    double laengeJetzt = staende.front().laenge;
    double c = staende.front().c, z0 = staende.front().z0;
    bool haftet = false;
    double letzteWurzel = -1.0;
    double uVor = 0.0, weg = 0.0, richtung = 1.0, richtungZiel = 1.0;
    double phaseV = 0.0;
    std::int64_t haftZahl = 0, messZahl = 0;
    const std::int64_t messAb = std::min(halten, n2) / 2;
    for (std::int64_t n = 0; n < n2; ++n)
    {
        const double t = static_cast<double>(n) * dt;
        while (aktiv + 1 < a.kette.size() && t >= a.kette[aktiv + 1].abS)
        {
            ++aktiv;
            c = staende[aktiv].c;
            z0 = staende[aktiv].z0;
            verlust.setze(saiteFuer(s, a.kette[aktiv].note).guete, staende[aktiv].hz, fs);
        }
        // die gegriffene Laenge: Legato als Gleiten in 15 ms (der Finger), Vibrato als Laengenaenderung (F-5 a)
        const double ziel = staende[aktiv].laenge;
        laengeJetzt = std::exp(std::log(laengeJetzt) + std::clamp(std::log(ziel / laengeJetzt), -dt / 0.015, dt / 0.015));
        phaseV += 2.0 * kPi * a.vibratoHz * dt;
        const double vib = std::exp2(-(a.pitchCent + a.vibratoCent * std::sin(phaseV)) / 1200.0);
        const double laenge = laengeJetzt * vib;
        const double xb = std::min(abstand, 0.4 * laenge);
        const double dB = xb / c * fs;
        const double dN = std::max(2.0, (laenge - xb) / c * fs - verlust.verzoegerung);
        // der Bogen: Ansatz und Absetzen in 50 ms (offen), Bogenwechsel nach der Haarlaenge, Tremolo, Spiccato
        const double hoch = std::min(1.0, t / 0.05) * (n < halten ? 1.0 : std::max(0.0, 1.0 - (t - a.haltenS) / 0.05));
        double vb = vbZiel * hoch;
        double fb = n < halten + static_cast<std::int64_t>(0.05 * fs) ? kraftZiel(a.kette[aktiv].note) : 0.0;
        if (a.art == Art::Tremolo)
            richtung = std::tanh(4.0 * std::sin(kPi * a.springHz * t));
        else
        {
            weg += std::abs(vb) * dt;
            if (weg > s.haarLaengeM)
            {
                weg = 0.0;
                richtungZiel = -richtungZiel;
            }
            richtung += std::clamp(richtungZiel - richtung, -2.0 * dt / s.wechselS, 2.0 * dt / s.wechselS);
        }
        vb *= richtung;
        if (a.art == Art::Spiccato) fb *= std::max(0.0, std::sin(2.0 * kPi * a.springHz * t)) * 1.5;
        // die ankommenden Wellen am Bogenpunkt
        const double vonSattel = vomSattel.lies(dN);
        const double vonSteg = vomSteg.lies(dB);
        const double vh = vonSattel + vonSteg;
        double f = 0.0;
        if (a.art == Art::Arco || a.art == Art::Spiccato || a.art == Art::Tremolo)
            f = reibung(vh, vb, fb, 2.0 * z0, s.reibungMuS, haftet, letzteWurzel);
        else
        {
            // Pizzicato (doppelter Abstand) und Col legno: ein Kraftstoss — Pizzicato 3 ms („no tone build-up longer
            // than 4 ms“, Melka nach Guettler — nur Bericht), das Holz 1 ms; Hoehe mit der Staerke (offen)
            const double dauer = a.art == Art::Pizzicato ? 0.003 : 0.001;
            if (t < dauer) f = (0.5 + 1.5 * e) * (a.art == Art::Pizzicato ? 8.0 : 4.0) * 0.5 * (1.0 - std::cos(2.0 * kPi * t / dauer));
            haftet = false;
        }
        if (n >= messAb && n < halten)
        {
            ++messZahl;
            if (haftet) ++haftZahl;
        }
        const double einspeisen = f / (2.0 * z0);
        zumSattel.schreibe(vonSteg + einspeisen);
        zumSteg.schreibe(vonSattel + einspeisen);
        // Sattel bzw. Finger: fest, mit dem Verlust je Umlauf
        vomSattel.schreibe(-verlust.schritt(zumSattel.lies(dN)));
        // Steg: die Admittanz des Korpus, geschlossen geloest
        const double vPlus = zumSteg.lies(dB);
        const double u = (steg.vorher() + 2.0 * dSteg * z0 * vPlus) / (1.0 + dSteg * z0);
        const double fSteg = z0 * (2.0 * vPlus - u);
        steg.nachher(fSteg);
        vomSteg.schreibe(u - vPlus);
        aus[static_cast<std::size_t>(n)] = (u - uVor) * fs * 1e-3;
        uVor = u;
    }
    // zurueck auf die Rate: Mittel je zwei Takte
    std::vector<double> summe(static_cast<std::size_t>(frames), 0.0);
    for (std::int64_t i = 0; i < frames; ++i)
    {
        double w = 0.0;
        for (int k = 0; k < kUeber; ++k) w += aus[static_cast<std::size_t>(i * kUeber + k)];
        summe[static_cast<std::size_t>(i)] = w / kUeber;
    }
    if (messung != nullptr)
    {
        messung->haftAnteil = messZahl > 0 ? static_cast<double>(haftZahl) / static_cast<double>(messZahl) : 0.0;
        const auto haltenF = static_cast<std::size_t>(std::min<std::int64_t>(frames, std::llround(a.haltenS * a.rate)));
        const auto ab = static_cast<std::size_t>(std::max(a.kette.back().abS + 0.1, 0.5 * a.haltenS) * a.rate);
        if (haltenF > ab + 4800)
        {
            std::vector<double> stueck(summe.begin() + static_cast<std::ptrdiff_t>(ab), summe.begin() + static_cast<std::ptrdiff_t>(haltenF));
            const messung::Tonhoehe th = messung::tonhoehe(stueck, a.rate, 30.0, 4000.0);
            std::vector<double> hz;
            for (const double x : th.hz)
                if (x > 0.0) hz.push_back(x);
            if (hz.size() > th.hz.size() / 2)
            {
                std::nth_element(hz.begin(), hz.begin() + static_cast<std::ptrdiff_t>(hz.size() / 2), hz.end());
                messung->spielHz = hz[hz.size() / 2];
                messung->schwingt = true;
            }
        }
        std::vector<double> alles(summe.begin(), summe.begin() + static_cast<std::ptrdiff_t>(haltenF));
        messung->einschwingS = messung::einschwingen(alles, a.rate);
    }
    const auto blende = std::min<std::int64_t>(frames, std::max<std::int64_t>(1, std::llround(kAusblendenS * a.rate)));
    for (std::int64_t i = 0; i < frames; ++i)
    {
        double w = summe[static_cast<std::size_t>(i)] / s.ausgangBezug;
        const std::int64_t rest = frames - 1 - i;
        if (rest < blende) w *= static_cast<double>(rest) / static_cast<double>(blende);
        mono[static_cast<std::size_t>(i)] = static_cast<float>(w);
    }
}

void stimme(StreichSpiel& s)
{
    if (!s.spielbar) return;
    const std::size_t anzahl = static_cast<std::size_t>(s.noteMax - s.noteMin + 1);
    std::vector<double> korrektur(anzahl, 1.0), uPp(anzahl, 0.0), oPp(anzahl, 0.0), uFf(anzahl, 0.0), oFf(anzahl, 0.0);
    parallelJe(anzahl, [&](std::size_t i) {
        const int note = s.noteMin + static_cast<int>(i);
        const double f = hzVon(note);
        StreichSpiel t = s;
        const auto spielt = [&](double kraft, double k, double e) {
            t.laengeJeNote = kurveAusPunkten({{note, k}});
            StreichAnschlag a;
            a.kette = {{0.0, note, f}};
            a.kraftN = kraft;
            a.expression = e;
            a.haltenS = 0.6;
            StreichMessung m;
            std::vector<float> ton;
            rechneStreichTon(t, a, ton, static_cast<std::int64_t>(0.6 * a.rate), &m);
            return m;
        };
        // das Fenster der Kraft (Schelleng) am Modell bei v_B der Staerke e: ±50 Cent und Haften ueber die Haelfte der
        // Takte (Helmholtz: Haften 1 − β je Periode); Raster 2^(1/4) von 5 mN bis 8 N, der laengste Lauf ohne die Raender
        const double schritt = std::exp2(0.25);
        const auto fenster = [&](double k, double e) {
            std::pair<double, double> best{0.0, 0.0};
            int bestLauf = 0, lauf = 0;
            double anfang = 0.0;
            for (double x = 0.005; x <= 8.0; x *= schritt)
            {
                const StreichMessung m = spielt(x, k, e);
                if (m.schwingt && std::abs(1200.0 * std::log2(m.spielHz / f)) < 50.0 && m.haftAnteil > 0.5)
                {
                    if (lauf++ == 0) anfang = x;
                    if (lauf > bestLauf)
                    {
                        bestLauf = lauf;
                        best = {anfang, x};
                    }
                }
                else
                    lauf = 0;
            }
            if (best.second >= best.first * schritt * schritt * schritt * 0.999)
            {
                best.first *= schritt;
                best.second /= schritt;
            }
            return best;
        };
        // grob vorstimmen bei 0,1 N (sonst faellt eine verstimmte Note aus dem ±50-Cent-Kriterium und findet kein
        // Fenster — die hohen Noten der Violine lagen ungestimmt +63 Cent)
        double k = 1.0;
        for (int runde = 0; runde < 3; ++runde)
        {
            const StreichMessung m = spielt(0.1, k, 0.0);
            if (!m.schwingt) break;
            const double fehler = 1200.0 * std::log2(m.spielHz / f);
            if (std::abs(fehler) < 5.0 || std::abs(fehler) > 300.0) break;
            k *= m.spielHz / f;
        }
        std::pair<double, double> wp = fenster(k, 0.0);
        if (wp.first > 0.0)
        {
            const double mitte = std::sqrt(wp.first * wp.second);
            for (int runde = 0; runde < 4; ++runde)
            {
                const StreichMessung m = spielt(mitte, k, 0.0);
                if (!m.schwingt) break;
                const double fehler = 1200.0 * std::log2(m.spielHz / f);
                if (std::abs(fehler) < 2.0 || std::abs(fehler) > 300.0) break;
                k *= m.spielHz / f;
            }
            const std::pair<double, double> w2 = fenster(k, 0.0);
            if (w2.first > 0.0) wp = w2;
        }
        const std::pair<double, double> wf = fenster(k, 1.0);
        korrektur[i] = k;
        uPp[i] = wp.first;
        oPp[i] = wp.second;
        uFf[i] = wf.first;
        oFf[i] = wf.second;
    });
    Kurve kk, ku, ko, kuf, kof;
    kk.belegkraft = ku.belegkraft = ko.belegkraft = kuf.belegkraft = kof.belegkraft = Belegkraft::Rechnung;
    kk.herkunft = "Feinstimmung der gegriffenen Laenge in der Mitte des Fensters bei pp (stimme)";
    ku.herkunft = "unterer Rand des Fensters der Bogenkraft bei v_B pp (±50 Cent, Haften > 50 %, ohne Rand; stimme)";
    ko.herkunft = "oberer Rand dieses Fensters (stimme)";
    kuf.herkunft = "unterer Rand des Fensters der Bogenkraft bei v_B ff (stimme)";
    kof.herkunft = "oberer Rand dieses Fensters (stimme)";
    for (std::size_t i = 0; i < anzahl; ++i)
    {
        const int note = s.noteMin + static_cast<int>(i);
        kk.punkte.push_back({note, korrektur[i]});
        // ohne Fenster: kein Punkt (die Nachbarn tragen; offen.md)
        if (uPp[i] > 0.0)
        {
            ku.punkte.push_back({note, uPp[i]});
            ko.punkte.push_back({note, oPp[i]});
        }
        if (uFf[i] > 0.0)
        {
            kuf.punkte.push_back({note, uFf[i]});
            kof.punkte.push_back({note, oFf[i]});
        }
    }
    s.laengeJeNote = kk;
    s.kraftUntenJeNote = ku;
    s.kraftObenJeNote = ko;
    s.kraftUntenFfJeNote = kuf;
    s.kraftObenFfJeNote = kof;
}

void rechneEnsembleTon(const StreichSpiel& s, const StreichAnschlag& a, int stimmen, std::vector<float>& mono, std::int64_t frames)
{
    mono.assign(static_cast<std::size_t>(std::max<std::int64_t>(0, frames)), 0.0f);
    if (stimmen <= 0 || frames <= 0) return;
    std::vector<std::vector<float>> teile(static_cast<std::size_t>(stimmen));
    parallelJe(teile.size(), [&](std::size_t i) {
        // Streuung je Stimme (F-8 a, als gesetzt gekennzeichnet): Stimmung ±6 Cent, Einsatz 0 … 40 ms (Rasch 1979:
        // 30–50 ms im Trio — nur Bericht), Vibrato-Rate ±0,4 Hz, Korpus ±3 % in der Frequenz ueber 1 kHz (offen);
        // fester Startwert je Stimme
        std::uint64_t z = 0x9E3779B97F4A7C15ull * (i + 1);
        const auto zufall = [&] {
            z ^= z >> 12;
            z ^= z << 25;
            z ^= z >> 27;
            return static_cast<double>((z * 0x2545F4914F6CDD1Dull) >> 11) / 9007199254740992.0;
        };
        StreichSpiel t = s;
        for (ImpedanzMode& m : t.korpus)
            if (m.hz > 1000.0) m.hz *= 1.0 + 0.06 * (zufall() - 0.5);
        StreichAnschlag b = a;
        b.pitchCent += 12.0 * (zufall() - 0.5);
        b.vibratoHz += 0.8 * (zufall() - 0.5);
        const double einsatz = 0.04 * zufall();
        b.haltenS = std::max(0.0, a.haltenS - einsatz);
        std::vector<float> ton;
        const auto versatz = static_cast<std::int64_t>(std::llround(einsatz * a.rate));
        rechneStreichTon(t, b, ton, std::max<std::int64_t>(0, frames - versatz));
        teile[i].assign(static_cast<std::size_t>(frames), 0.0f);
        for (std::size_t j = 0; j < ton.size(); ++j) teile[i][j + static_cast<std::size_t>(versatz)] = ton[j];
    });
    const double g = 1.0 / std::sqrt(static_cast<double>(stimmen));
    for (std::size_t j = 0; j < mono.size(); ++j)
    {
        double w = 0.0;
        for (const auto& tt : teile) w += tt[j];
        mono[j] = static_cast<float>(w * g);
    }
}

}  // namespace ic::bogen
