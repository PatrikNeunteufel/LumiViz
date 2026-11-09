/**
 * @file Vorgaben.cpp
 * @brief Klasse 2: Vorgaben — Cembalo (Chadefaux et al. 2013, Dulcken 1755), Harfe (Chadefaux et al. 2012), Gitarre
 *        Deluxe (zwei Polarisationen; Korpus offen bis zum Fit an Mores, lokal), Laute und Oud (Doppelchoere, F-9 a);
 *        Koto zurueckgestellt (K2 F-1 a)
 *
 * Belegkraft: *Quelle* (Chadefaux et al. 2013 Tab. 1/2 S. 158, CC BY 3.0, im Repo; Bader et al. 2025 Dulcken, CC BY
 * 4.0, im Repo; Chadefaux et al. 2012 PDF-S. 3/9, Valimaki et al. 2004 S. 935–936 — nachgelesen IC Session 2), *nur
 * Bericht*, *Rechnung*, *offen*. Tabelle: `klassen/Klasse 2 - Gezupfte Saite/modell.md`.
 */
#include "klassen/02_zupf_saite/Zupf.hpp"

#include "gemeinsam/Werkzeug.hpp"

#include <cmath>

namespace ic::zupf
{

namespace
{

double hzVon(double note)
{
    return 440.0 * std::exp2((note - 69.0) / 12.0);
}

/// Laenge je Note: L₀ an der Bezugsnote, darueber ∝ 1/f, darunter verkuerzt ∝ f^−exponent (offen).
Kurve laengen(int von, int bis, int bezug, double l0, double exponentUnten)
{
    std::vector<NotenWert> p;
    for (int n = von; n <= bis; ++n)
    {
        const double r = hzVon(bezug) / hzVon(n);
        p.push_back({n, l0 * (n >= bezug ? r : std::pow(r, exponentUnten))});
    }
    Kurve k = kurveAusPunkten({{von, p.front().wert}, {bis, p.back().wert}});
    k.punkte = p;
    k.belegkraft = Belegkraft::Offen;
    k.herkunft = "an der Bezugsnote gemessen, sonst ∝ 1/f, im Bass verkuerzt (offen)";
    return k;
}

/// Die Stegmoden aus einer Liste (Frequenz, Pegel in dB gegen die staerkste), gleiche Guete, abwechselnde Richtung.
std::vector<StegMode> stegAus(const std::vector<std::pair<double, double>>& spitzen, double staerkste, double q, double winkel)
{
    std::vector<StegMode> aus;
    int i = 0;
    for (const auto& [hz, db] : spitzen)
        aus.push_back({hz, q, staerkste * std::pow(10.0, db / 20.0), (i++ % 2 == 0 ? 1.0 : -1.0) * winkel});
    return aus;
}

void normiere(ZupfSpiel& s)
{
    s.ausgangBezug = 1.0;
    if (!s.spielbar) return;
    stimme(s);
    ZupfAnschlag a;
    a.note = (s.noteMin + s.noteMax) / 2;
    a.staerke = 1.0;
    a.haltenS = 0.5;
    std::vector<float> ton;
    rechneZupfTon(s, a, ton, a.rate / 2);
    const double sp = spitze(ton);
    s.ausgangBezug = sp > 0.0 ? sp / 0.5 : 1.0;
}

}  // namespace

ZupfSpiel baueVorgabeCembalo()
{
    ZupfSpiel s;
    s.name = "Cembalo";
    s.noteMin = 29;  // FF … f³ (Taskin, nur Bericht)
    s.noteMax = 89;
    // Chadefaux et al. 2013 Tab. 2 (Quelle): G4, „String length Ls 47.5cm“, „String tension T 85.7N“; ueber den Umfang ∝ 1/f,
    // im Bass verkuerzt (offen); der Zug fest (offen)
    s.laengeJeNote = laengen(s.noteMin, s.noteMax, 67, 0.475, 0.85);
    s.zugJeNote = kurveKonstant(85.7, Belegkraft::Offen, "Chadefaux Tab. 2 an G4, ueber den Umfang fest (offen)", s.noteMin, s.noteMax);
    s.guete = 1500.0;  // offen
    // Valimaki et al. 2004 (nachgelesen S. 936): „from 12% to about 50% of the string length in the bass and in the treble range“
    s.zupfpunktJeNote = kurveAusPunkten({{s.noteMin, 0.12}, {s.noteMax, 0.5}}, Belegkraft::Quelle, "Valimaki et al. 2004 S. 936");
    // Chadefaux Tab. 1 (Quelle): Winkel beim Abloesen 17 ± 2° (langsam), 20 ± 1° (schnell); Auslenkung 0,71 mm → Kraft
    // T·δ·(1/x + 1/(L − x)) ≈ 0,8 N an G4 (Rechnung); pp … ff schwach (F-4 a: die Staerke wirkt vor allem im Tastenstoss)
    s.polarisationGrad = 18.0;
    s.kraftPpN = 0.6;
    s.kraftFfN = 1.0;
    s.stossS = 0.004;  // offen (Wechselwirkung 12 / 7,9 ms, Chadefaux — die Freigabe schneller)
    // Dulcken 1755 (Bader et al. 2025, Quelle): Helmholtz 37 Hz, „T60 = 235 ms“ am 8′-Steg → Q = π·f·T60/ln 1000;
    // die Frequenzen ueber 37 Hz und die Staerken offen
    for (const double hz : {37.0, 120.0, 180.0, 260.0, 350.0, 480.0, 650.0, 900.0, 1300.0, 1800.0})
        s.steg.push_back({hz, std::max(3.0, 3.141592653589793 * hz * 0.235 / 6.9078), 0.004, hz < 300.0 ? 0.0 : 25.0});
    s.tastenstoss = 0.3;  // offen (Penttinen: +10 dB in den Moden < 300 Hz je Stufe)
    s.daempfenS = 0.06;
    s.kennzeichen = "ungemessen (VCSL lokal)";
    normiere(s);
    return s;
}

ZupfSpiel baueVorgabeHarfe()
{
    ZupfSpiel s;
    s.name = "Harfe";
    s.noteMin = 24;  // C1 … G7 (47 Saiten, offen)
    s.noteMax = 103;
    // Chadefaux et al. 2012 (Quelle PDF-S. 3): D♭2 138,6 Hz, 260 N, 97,5 cm, Darm 1,96 mm; ueber den Umfang offen
    s.laengeJeNote = laengen(s.noteMin, s.noteMax, 49, 0.975, 0.6);
    s.zugJeNote = kurveAusPunkten({{24, 300.0}, {49, 260.0}, {103, 60.0}}, Belegkraft::Offen, "D♭2 260 N (Chadefaux), sonst offen");
    s.guete = 600.0;  // Darm (offen)
    s.zupfpunktRel = 0.5;  // die Mitte (offen)
    // „Dtr = 4 mm“, „Vtr = 2 m/s“ (Quelle PDF-S. 9) → T·δ·4/L ≈ 4 N an D♭2 (Rechnung); Winkel 7–43° (nur Bericht)
    s.polarisationGrad = 25.0;
    s.kraftPpN = 1.0;
    s.kraftFfN = 8.0;
    s.stossS = 0.003;
    s.steg = stegAus({{150.0, -6.0}, {290.0, 0.0}, {450.0, -4.0}, {800.0, -8.0}, {1400.0, -12.0}}, 0.02, 20.0, 20.0);  // A0/T1 offen
    s.daempfenS = 0.15;
    s.kennzeichen = "ungemessen (Koerper offen; VSCO lokal)";
    // Mitschwingen je Clip (K2 F-3 a): alle 47 Saiten, die Pedale in der Mitte (C-Dur — offen; die Stimmung der Pedale
    // waere eine Passage-Spur)
    for (int n = s.noteMin; n <= s.noteMax; ++n)
    {
        const int k = n % 12;
        if (k == 0 || k == 2 || k == 4 || k == 5 || k == 7 || k == 9 || k == 11) s.mitschwingNoten.push_back(n);
    }
    normiere(s);
    return s;
}

ZupfSpiel baueVorgabeGitarre()
{
    ZupfSpiel s;
    s.name = "Gitarre (Deluxe)";
    s.noteMin = 40;  // E2 … B5
    s.noteMax = 83;
    s.laengeJeNote = kurveKonstant(0.65, Belegkraft::Offen, "Mensur 650 mm (Bestand LumiViz)", s.noteMin, s.noteMax);
    s.zugJeNote = kurveKonstant(70.0, Belegkraft::Offen, "um 70 N je Saite (offen)", s.noteMin, s.noteMax);
    s.guete = 800.0;
    s.zupfpunktRel = 0.18;
    s.polarisationGrad = 30.0;  // Rest-/Free-Stroke (Scherrer & Depalle) — offen
    s.kraftPpN = 0.5;
    s.kraftFfN = 4.0;
    // Korpus *Classical* (Wright TOP26): die gekoppelten Spitzen seiner Abb. 7.1 (112 / 195 / 215 / 435 / 505 / 674 /
    // 880 Hz, Bestand LumiViz §5f) mit den dort gemessenen Pegeln; Guete, Staerke und Richtung **offen** — der Korpus
    // Deluxe kommt aus dem Modalfit an Mores (lokal, F-2 a)
    s.steg = stegAus({{112.0, -7.5}, {195.0, 0.0}, {215.0, -7.6}, {435.0, -14.4}, {505.0, -9.8}, {674.0, -11.2}, {880.0, -19.5}}, 0.05, 30.0, 25.0);
    s.kennzeichen = "ungemessen (Korpus offen bis zum Fit an Mores)";
    s.mitschwingNoten = {40, 45, 50, 55, 59, 64};  // die leeren Saiten E2 A2 D3 G3 H3 E4 (K2 F-3 a)
    normiere(s);
    return s;
}

ZupfSpiel baueVorgabeLaute()
{
    ZupfSpiel s = baueVorgabeGitarre();
    s.name = "Laute";
    s.mitschwingNoten.clear();  // die leeren Choere der Laute: Stimmung offen (Gamut nur Bericht)
    s.noteMin = 43;
    s.noteMax = 79;
    s.laengeJeNote = kurveKonstant(0.60, Belegkraft::Offen, "offen (Gamut-Tabelle nur Bericht)", s.noteMin, s.noteMax);
    s.zugJeNote = kurveKonstant(35.0, Belegkraft::Offen, "Darm, offen", s.noteMin, s.noteMax);
    s.guete = 500.0;
    s.chor = {2, 4.0};  // Doppelchor (F-9 a), Verstimmung offen
    s.steg = stegAus({{130.0, -6.0}, {240.0, 0.0}, {380.0, -8.0}, {600.0, -10.0}, {1000.0, -16.0}}, 0.06, 25.0, 25.0);  // offen
    s.kennzeichen = "ungemessen (Muschelboden und Saiten offen; ohne Aufnahme)";
    normiere(s);
    return s;
}

ZupfSpiel baueVorgabeOud()
{
    ZupfSpiel s = baueVorgabeLaute();
    s.name = "Oud";
    s.noteMin = 40;
    s.noteMax = 77;
    s.chor = {2, 3.0};
    s.steg = stegAus({{120.0, -5.0}, {230.0, 0.0}, {350.0, -7.0}, {560.0, -10.0}, {950.0, -15.0}}, 0.06, 25.0, 25.0);  // Abdallah 2026 nicht gelesen — offen
    s.kennzeichen = "ungemessen (Decke offen, Abdallah et al. 2026 lesen; ohne Aufnahme)";
    normiere(s);
    return s;
}

ZupfSpiel baueVorgabeKoto()
{
    ZupfSpiel s;
    s.name = "Koto";
    s.spielbar = false;
    s.grund = "zurueckgestellt bis zu einer freien Aufnahme (K2 F-1 a); Oshi-de als Bend (F-8 a) danach";
    s.kennzeichen = "nicht spielbar (zurueckgestellt, K2 F-1 a)";
    return s;
}

}  // namespace ic::zupf
