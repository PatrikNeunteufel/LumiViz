/**
 * @file Vorgaben.cpp
 * @brief Klasse 7: Vorgaben — Posaune (Kern zuerst, F-1 a), Alphorn, und Trompete, Tuba, Horn, Didgeridoo als
 *        Geruest ohne Bohrung (T8 a, Kipppunkt b: ohne veroeffentlichte Geometrie klingt das Instrument nur mit einer
 *        erfundenen Zahl — vorgelegt, nicht gebaut)
 *
 * Belegkraft: *Quelle* (nachgelesen IC Session 1: Willemsen et al. 2021 Tab. 1 S. 157, Marcinnò et al. 2026 Tab. 3
 * S. 18, Petiot et al. 2023 Tab. 1 S. 376 Mundruck), *nur Bericht* (`recherche/2026-10-26_physik_spielweisen.md`,
 * `…_alphorn_didgeridoo.md`), *Rechnung*, *offen*. Tabelle: `klassen/Klasse 7 - Lippe und Rohr/modell.md`.
 */
#include "klassen/07_lippe_rohr/Blech.hpp"

#include "gemeinsam/Werkzeug.hpp"

#include <cmath>
#include <numbers>

namespace ic::blech
{

#if __has_include("klassen/07_lippe_rohr/Stimmung.inc")
#include "klassen/07_lippe_rohr/Stimmung.inc"
#define IC_BLECH_STIMMUNG 1
#endif

namespace
{

/// Ein Schallstueck als Bessel-Horn r(x) ∝ (d1 + L − x)^(−ε) von r0 auf r1 (Benade, Lehrbuch — offen: das Profil der
/// Quelle fehlt, nur Anfangs- und Endradius), in `stuecke` Kegelstuempfe.
std::vector<Abschnitt> besselHorn(double laengeM, double r0, double r1, double eps, int stuecke = 60)
{
    const double d1 = laengeM / (std::pow(r1 / r0, 1.0 / eps) - 1.0);
    const auto r = [&](double x) { return r1 * std::pow((d1 + laengeM - x) / d1, -eps); };
    std::vector<Abschnitt> a;
    for (int i = 0; i < stuecke; ++i)
    {
        const double x0 = laengeM * i / stuecke, x1 = laengeM * (i + 1) / stuecke;
        a.push_back({x1 - x0, r(x0), r(x1)});
    }
    return a;
}

void normiere(BlechSpiel& s)
{
    s.ausgangBezug = 1.0;
    if (!s.spielbar) return;
    BlechAnschlag a;
    const int n = (s.noteMin + s.noteMax) / 2;
    a.kette = {{0.0, n, 440.0 * std::exp2((n - 69) / 12.0)}};
    a.expression = 1.0;
    a.haltenS = 0.5;
    std::vector<float> ton;
    rechneBlechTon(s, a, ton, a.rate / 2);
    const double sp = spitze(ton);
    s.ausgangBezug = sp > 0.0 ? sp / 0.5 : 1.0;
}

BlechSpiel geruest(const std::string& name, int von, int bis, const std::string& grund)
{
    BlechSpiel s;
    s.name = name;
    s.noteMin = von;
    s.noteMax = bis;
    s.spielbar = false;
    s.grund = grund;
    s.kennzeichen = "nicht spielbar (Bohrung offen)";
    return s;
}

}  // namespace

BlechSpiel baueVorgabePosaune(bool gestimmt)
{
    BlechSpiel s;
    s.name = "Posaune";
    s.noteMin = 40;  // E2 … F5 (Tenorposaune, offen)
    s.noteMax = 77;
    // Willemsen, Bilbao, Ducceschi, Serafin 2021 (DAFx20in21), Tab. 1 S. 157 (nachgelesen; „Geometry of a measured
    // trombone taken from [16]“ = Smyth & Scott 2011): Innenzug 70,8 cm r 0,69; Aussenzug ausgezogen 53 r 0,72 (zweimal);
    // Zugbogen 17,7 r 0,74; Innenzug 71,1 r 0,69; Schwanenhals 24,1 r 0,71; Stimmzug 25,4 r 0,75 → 1,07; Schallstueck
    // 50,2 r 1 → 10,8. Der Zylinder: die Zugrohre mit dem mittleren Radius 0,7 cm (Rechnung, offen: die Spruenge
    // 0,69/0,72/0,74 fehlen); der Zug fuegt zweimal den Aussenzug hinzu (0 … 1,06 m; Gesamtlaenge 2,593–3,653 m, Tab. 2)
    s.zylinderRadiusM = 0.007;
    s.zylinderGrundM = 0.708 + 0.177 + 0.711;
    s.zugMaxM = 2.0 * 0.53;
    s.rest.abschnitte = {{0.241, 0.0071, 0.0071}, {0.254, 0.0075, 0.0107}};
    for (const Abschnitt& a : besselHorn(0.502, 0.010, 0.108, 0.7)) s.rest.abschnitte.push_back(a);
    s.rest.herkunft = "Willemsen et al. 2021 Tab. 1 (nachgelesen); das Profil des Schallstuecks als Bessel-Horn ε = 0,7 (offen)";
    s.kesselM3 = 11.6e-6;  // Boutin 2013: Kesselvolumen „11.6 cm³“ (nur Bericht, Wortlaut im Bericht)
    // die Lippe: Breite 0,01 m, Ruhespalt H0 = 2,9·10⁻⁴ m (Willemsen Tab. 2, nur Bericht); µ = 1,3 kg/m², Q = 5
    // (Petiot et al. 2023 Tab. 2 an der Trompete, nur Bericht — uebertragen, offen)
    s.lippeBreiteM = 0.01;
    s.lippeSpaltM = 2.9e-4;
    s.lippeMasseJeFlaeche = 1.3;
    s.lippeGuete = 5.0;
    // der Druck: 500 bis 14 000 Pa (Petiot Tab. 1: „500 to 14000“, nachgelesen; Willemsen Tab. 2 nennt bis 6000 Pa —
    // nur Bericht — die hohe Lage spricht darunter im Modell nicht an); je Note der Weg ueber die Schwelle (stimme)
    s.druckMinPa = 500.0;
    s.druckMaxPa = 14000.0;
    s.beta = 1.2;
    // Daempfer (T3b a): Straight mit der Stoerresonanz um 77 Hz (nur Bericht), Cup −20 dB um 1 kHz (nur Bericht) —
    // als Helmholtz-Zweig mit Restflaeche; Werte offen. Wah-Wah am Harmon: die Hand schliesst den Stiel bis auf ein
    // Fuenftel der Restflaeche (0,01 — offen)
    s.daempfer = {{"Straight", 77.0, 5.0, 0.1}, {"Cup", 1000.0, 2.0, 0.1}, {"Harmon", 1500.0, 3.0, 0.05, 0.01}};
    s.kennzeichen = "ungemessen";
    bereite(s);
    if (!gestimmt) return s;
#ifdef IC_BLECH_STIMMUNG
    stimmungPosaune(s);
#else
    stimme(s, 1);
#endif
    normiere(s);
    return s;
}

BlechSpiel baueVorgabeAlphorn(bool gestimmt)
{
    BlechSpiel s;
    s.name = "Alphorn";
    s.noteMin = 42;  // Fis-Horn: Naturtoene ab 92 Hz bis 556 Hz (Marcinnò Tab. 1 S. 10, nur Bericht) — 42 … 73
    s.noteMax = 73;
    // Marcinnò et al. 2026 (arXiv 2607.22823), Tab. 3 S. 18 (nachgelesen), Horn 3: L1 … L4 = 539,92 / 844,91 / 754,84 /
    // 444,94 mm, R2 … R4 = 16,00 / 28,50 / 44,00 mm; R1 und das Ende des Bechers stehen nicht in der Tabelle — R1 aus
    // der Bohrung des Mundstuecks 5,5 mm (Lechgold, nur Bericht), der Becher 0,5 m von 44 auf 90 mm (Abb. 3b,
    // abgelesen, ungefaehr) als Bessel-Horn (offen)
    s.zylinderRadiusM = 0.00275;
    s.zylinderGrundM = 0.01;  // kein Zylinder: ein Zentimeter am Mundstueck traegt die Schleife
    s.zugMaxM = 0.0;
    s.rest.abschnitte = {{0.53992, 0.00275, 0.016}, {0.84491, 0.016, 0.0285}, {0.75484, 0.0285, 0.044}, {0.44494, 0.044, 0.044}};
    // die vierte Laenge endet bei R4 (Tab. 3 hat R2–R4 an den Enden von Kegel 1–3); Kegel 4 als Zylinder bis zum Becher (offen)
    for (const Abschnitt& a : besselHorn(0.5, 0.044, 0.090, 0.7)) s.rest.abschnitte.push_back(a);
    s.rest.verluste = true;
    s.rest.herkunft = "Marcinnò et al. 2026 Tab. 3 (nachgelesen); R1, Becher offen";
    // Kessel: Halbkugel mit 19 mm (Lechgold „19 mm Kesselinnendurchmesser“, nur Bericht): (π/12)·d³ (Rechnung)
    s.kesselM3 = std::numbers::pi / 12.0 * std::pow(0.019, 3);
    s.lippeBreiteM = 0.01;
    s.lippeSpaltM = 2.9e-4;
    s.lippeMasseJeFlaeche = 1.3;
    s.lippeGuete = 5.0;
    s.druckMinPa = 500.0;
    s.druckMaxPa = 14000.0;
    s.beta = 1.2;  // fast ohne Zylinder: das Schmettern bleibt schwach (Myers: B klein bei konischen Bohrungen, nur Bericht)
    s.kennzeichen = "ungemessen";
    bereite(s);
    if (!gestimmt) return s;
#ifdef IC_BLECH_STIMMUNG
    stimmungAlphorn(s);
#else
    stimme(s, 1);
#endif
    normiere(s);
    return s;
}

BlechSpiel baueVorgabeTrompete(bool gestimmt)
{
    BlechSpiel s;
    s.name = "Trompete";
    s.noteMin = 54;  // klingend F♯3 … A♯5 (offen)
    s.noteMax = 82;
    // Soares, Cochelin, Fréour, Vergez, Arimoto, Acta Acustica 2025, 9, 68, Abb. 4 S. 9 (nachgelesen, CC BY 4.0): die
    // vereinfachte B♭-Trompete in sieben Abschnitten („approximates the bore geometry of a typical Bb trumpet“, als
    // Prototyp gebaut und gemessen), Punkte 1 … 8: r = 8 / 8 / 2 / 5 / 5 / 15 / 20 / 60 mm bei z = −1355 / −1345 / −1340 /
    // −1270 / −430 / −80 / −40 / 0 mm. Kessel (1–2), Kehle (2–3) und Seele (3–4) als ein Volumen am Eingang (Rechnung:
    // 2,01 + 0,44 + 2,86 = 5,31 cm³; als Zylinder mit r = 5 mm waeren das 68 mm statt 85 mm — offen), der Zylinder 4–5
    // traegt die Ventile, 5–8 der Rest bis zum Becher
    const auto kegelVolumen = [](double h, double r0, double r1) { return std::numbers::pi * h / 3.0 * (r0 * r0 + r0 * r1 + r1 * r1); };
    s.kesselM3 = kegelVolumen(0.010, 0.008, 0.008) + kegelVolumen(0.005, 0.008, 0.002) + kegelVolumen(0.070, 0.002, 0.005);
    s.zylinderRadiusM = 0.005;
    s.zylinderGrundM = 1.270 - 0.430;
    s.zugMaxM = 0.0;
    s.rest.abschnitte = {{0.350, 0.005, 0.015}, {0.040, 0.015, 0.020}, {0.040, 0.020, 0.060}};
    s.rest.verluste = true;
    s.rest.herkunft = "Soares et al. 2025 Abb. 4 (nachgelesen)";
    // die Ventile: 2 / 1 / 3 Halbtoene, je L·(2^(k/12) − 1) mit der Laenge der Bohrung L = 1,355 m (Rechnung — die
    // Verlaengerung echter Ventilzuege und der Fehler der Kombinationen offen)
    const double l = 1.355;
    s.ventilM = {l * (std::exp2(2.0 / 12.0) - 1.0), l * (std::exp2(1.0 / 12.0) - 1.0), l * (std::exp2(3.0 / 12.0) - 1.0)};
    // die Lippe wie bei der Posaune (µ, Q: Petiot et al. 2023 Tab. 2 an der Trompete, nur Bericht; Breite und Ruhespalt
    // von der Posaune uebertragen — offen); der Druck 500 … 14 000 Pa (Petiot et al. 2023 Tab. 1, nachgelesen: Trompete)
    s.lippeBreiteM = 0.01;
    s.lippeSpaltM = 2.9e-4;
    s.lippeMasseJeFlaeche = 1.3;
    s.lippeGuete = 5.0;
    s.druckMinPa = 500.0;
    s.druckMaxPa = 14000.0;
    s.beta = 1.2;
    s.kennzeichen = "ungemessen (Bohrung vereinfacht nach Soares 2025; Ventile gerechnet; Lippe uebertragen)";
    bereite(s);
    if (!gestimmt) return s;
#ifdef IC_BLECH_STIMMUNG
    stimmungTrompete(s);
#else
    stimme(s, 1);
#endif
    normiere(s);
    return s;
}

BlechSpiel baueVorgabeTuba()
{
    return geruest("Tuba", 28, 65,
                   "keine freie Bohrung: Acta 2022 (arXiv 2207.05395) gibt keine Tuba-Bohrung (Profile nach Campbell et al. "
                   "2021, Buch), Myers 2019 nicht mehr abrufbar — F-B4");
}

BlechSpiel baueVorgabeHorn()
{
    return geruest("Horn", 34, 77, "keine Bohrung gefunden (Edinburgh-Dissertation nicht geoeffnet) — vorgelegt");
}

BlechSpiel baueVorgabeDidgeridoo(bool gestimmt)
{
    BlechSpiel s;
    s.name = "Didgeridoo";
    // der Grundton (Bordun) an der ersten Spitze der Impedanz 80,6 Hz (Rechnung; Causse S. 2: die acht Instrumente von
    // „60 Hz“ bis „88 Hz“), ±1 Halbton mit der Lippe (offen)
    s.noteMin = 39;
    s.noteMax = 41;
    // Causse, Goepp, Sluchin: „An investigation on ‚tonal‘ and ‚playability‘ qualities of eight didgeridoos“ (HAL-01161327),
    // Abb. 2 S. 3 (gemessener mittlerer Radius dreier Instrumente, Punkte abgelesen — Belegkraft: Abbildung): Didgeridoo 5
    // („in a middle distance“), vom Mund (Radius 13,4 mm; Text S. 2: Eingang 28–30 mm) bis zum Becher 1,465 m:
    // Abstand/Radius in mm 0/13,4 · 20/17,3 · 50/17,5 · 100/19,2 · 200/13,7 · 300/17,0 · 640/24,7 · 740/24,0 · 840/23,8 ·
    // 940/28,6 · 1040/26,2 · 1140/26,5 · 1240/31,2 · 1340/42,5 · 1440/50,5 · 1465/51,2
    const std::vector<std::pair<double, double>> p = {{0, 13.4},    {20, 17.3},   {50, 17.5},   {100, 19.2},  {200, 13.7},  {300, 17.0},
                                                      {640, 24.7},  {740, 24.0},  {840, 23.8},  {940, 28.6},  {1040, 26.2}, {1140, 26.5},
                                                      {1240, 31.2}, {1340, 42.5}, {1440, 50.5}, {1465, 51.2}};
    // kein Mundstueck: die Lippen am Wachsrand — ein Zentimeter Zylinder traegt die Schleife (wie beim Alphorn), der Kessel
    // klein (0,5 cm³, offen)
    s.zylinderRadiusM = 0.0134;
    s.zylinderGrundM = 0.01;
    s.zugMaxM = 0.0;
    for (std::size_t i = 1; i < p.size(); ++i)
    {
        const double von = std::max(p[i - 1].first, 10.0), bis = p[i].first;
        if (bis <= von) continue;
        const double r0 = p[i - 1].second + (p[i].second - p[i - 1].second) * (von - p[i - 1].first) / (p[i].first - p[i - 1].first);
        s.rest.abschnitte.push_back({(bis - von) * 1e-3, r0 * 1e-3, p[i].second * 1e-3});
    }
    s.rest.verluste = true;
    s.rest.herkunft = "Causse, Goepp, Sluchin (HAL-01161327) Abb. 2, Didgeridoo 5 (abgelesen)";
    s.kesselM3 = 0.5e-6;
    // die Lippe wie bei der Posaune (offen); der Vokaltrakt gebaut (T5 a, T3c a): Fletcher et al. 2006 II Tab. I (nur Bericht)
    s.lippeBreiteM = 0.01;
    s.lippeSpaltM = 2.9e-4;
    s.lippeMasseJeFlaeche = 1.3;
    s.lippeGuete = 5.0;
    s.druckMinPa = 500.0;
    s.druckMaxPa = 14000.0;
    s.beta = 1.2;
    s.vokalGewicht = 1.0;
    // Befund (IC Session 4): mit der Lippe der Posaune entsteht kein Bordun — bei jedem f_L/f nur ein Quietschen um 2 kHz
    // oder nichts, auch ohne Vokaltrakt und mit groesserem Kessel; der weite Eingang (13,4 mm gegen 7 mm) gibt ein Viertel
    // der Impedanz. Lippenwerte fuer das Didgeridoo ohne Quelle — darum nicht spielbar (T8 a), die Bohrung bleibt
    s.spielbar = false;
    s.grund = "Bohrung da (Causse et al. Abb. 2); die Lippe der Posaune treibt den Bordun nicht an — Lippenwerte fuer das Didgeridoo offen";
    s.kennzeichen = "nicht spielbar (Lippe offen; Bohrung aus einer Abbildung abgelesen)";
    (void)gestimmt;
    return s;
}

}  // namespace ic::blech
