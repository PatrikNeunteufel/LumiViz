/**
 * @file Vorgaben.cpp
 * @brief Klasse 8: Vorgaben — Klarinette (Kern zuerst, F-1 a), Sopran- und Tenorsaxophon; Altsaxophon, Oboe, Fagott
 *        als Geruest ohne Bohrung (T8 Kipppunkt b, Frage F-B2)
 *
 * Belegkraft: *nur Bericht* (Wortlaut im Bericht `recherche/2026-10-26_physik_spielweisen.md`: Chatziioannou et al.
 * 2019 Tab. I, Debut et al. 2005 (Bohrung — umformatiert, „vor Uebernahme am Original pruefen“), Chen, Smith & Wolfe
 * 2009 S. 18/20), *Rechnung*, *offen*. Tabelle: `klassen/Klasse 8 - Rohrblatt und Rohr/modell.md`.
 */
#include "klassen/08_rohrblatt_rohr/Rohrblatt.hpp"

#include "gemeinsam/Werkzeug.hpp"

#include <cmath>
#include <numbers>

namespace ic::rohrblatt
{

#if __has_include("klassen/08_rohrblatt_rohr/Stimmung.inc")
#include "klassen/08_rohrblatt_rohr/Stimmung.inc"
#define IC_ROHRBLATT_STIMMUNG 1
#endif

namespace
{

void normiere(RohrblattSpiel& s)
{
    s.ausgangBezug = 1.0;
    if (!s.spielbar || s.laengeJeNote.punkte.empty()) return;
    RohrblattAnschlag a;
    const int n = (s.noteMin + s.noteMax) / 2;
    a.kette = {{0.0, n, 440.0 * std::exp2((n - 69) / 12.0)}};
    a.expression = 1.0;
    a.haltenS = 0.5;
    std::vector<float> ton;
    rechneRohrblattTon(s, a, ton, a.rate / 2);
    const double sp = spitze(ton);
    s.ausgangBezug = sp > 0.0 ? sp / 0.5 : 1.0;
}

RohrblattSpiel geruest(const std::string& name, int von, int bis, const std::string& grund)
{
    RohrblattSpiel s;
    s.name = name;
    s.noteMin = von;
    s.noteMax = bis;
    s.spielbar = false;
    s.grund = grund;
    s.kennzeichen = "nicht spielbar (Bohrung offen)";
    return s;
}

/// Ein Kegel mit Halbwinkel `winkelGrad` ab dem Radius `r0` ueber `laenge` (Saxophon nach Chen et al. 2009).
Profil kegel(double r0, double winkelGrad, double laenge, double mundstueck)
{
    Profil p;
    const double t = std::tan(winkelGrad * std::numbers::pi / 180.0);
    for (int i = 0; i <= 20; ++i)
    {
        const double x = laenge * i / 20.0;
        p.punkte.push_back({x, r0 + t * x});
    }
    p.mundstueckM3 = mundstueck;
    return p;
}

}  // namespace

RohrblattSpiel baueVorgabeKlarinette(bool gestimmt)
{
    RohrblattSpiel s;
    s.name = "Klarinette";
    s.noteMin = 50;  // B♭-Klarinette klingend D3 … B♭5 (Chalumeau und Clarion; Altissimo offen)
    s.noteMax = 82;
    s.registerAb = 69;  // klingend A4 = gegriffen H4: das Clarion mit dem Registerloch (die Duodezime)
    s.registerMode = 2;
    // Debut, Kergomard, Laloë 2005 (arXiv physics/0309051), Bohrung Position/Durchmesser in mm (nur Bericht; „vor
    // Uebernahme am Original pruefen“): 0/0 · 89/14,8 · 117,5/14,6 · 117,5/15,6 · 152,5/14,5 · 492,5/14,5 · 526/15,1 ·
    // 565/16,9 · 578,2/18,4 · 598/22,2 · 664/60. Der Punkt 0/0 (die Spitze des Blatts) als Zylinder 14,8 mm bis 89 mm
    // (offen: das Innere des Mundstuecks); der Sprung bei 117,5 mm als Stufe
    s.profil.punkte = {{0.0, 0.0074}, {0.089, 0.0074}, {0.1175, 0.0073}, {0.11751, 0.0078}, {0.1525, 0.00725}, {0.4925, 0.00725},
                       {0.526, 0.00755}, {0.565, 0.00845}, {0.5782, 0.0092}, {0.598, 0.0111}, {0.664, 0.030}};
    s.kennzeichen = "ungemessen";
    if (!gestimmt) return s;
#ifdef IC_ROHRBLATT_STIMMUNG
    stimmungKlarinette(s);
#else
    stimme(s);
#endif
    normiere(s);
    return s;
}

RohrblattSpiel baueVorgabeSopransaxophon(bool gestimmt)
{
    RohrblattSpiel s;
    s.name = "Sopransaxophon";
    s.noteMin = 56;  // A♭3 … E♭6 klingend (offen)
    s.noteMax = 87;
    s.registerAb = 74;
    s.registerMode = 2;  // die Oktave (Kegel: alle Moden)
    // Chen, Smith, Wolfe 2009 (nur Bericht): Halbwinkel „1.74°“, Laenge 710 mm, „truncated at a diameter of 9.2 mm, and
    // the missing cone of length ~150 mm is replaced by a mouthpiece with a volume of 2.25 cm3“ (S. 18, 20)
    s.profil = kegel(0.0046, 1.74, 0.710, 2.25e-6);
    s.druckMaxPa = 7800.0;  // Altsax 1,5–7,8 kPa (Fuks & Sundberg, nur Bericht) — uebertragen
    s.kennzeichen = "ungemessen (Blatt der Klarinette uebertragen)";
    if (!gestimmt) return s;
#ifdef IC_ROHRBLATT_STIMMUNG
    stimmungSopransaxophon(s);
#else
    stimme(s);
#endif
    normiere(s);
    return s;
}

RohrblattSpiel baueVorgabeTenorsaxophon(bool gestimmt)
{
    RohrblattSpiel s;
    s.name = "Tenorsaxophon";
    s.noteMin = 44;  // A♭2 … E♭5 klingend (offen)
    s.noteMax = 75;
    s.registerAb = 62;
    s.registerMode = 2;
    // Chen et al. 2009 (nur Bericht): Halbwinkel „1.52°“, Laenge 1490 mm; Abschnitt und Mundstueck nicht genannt —
    // wie beim Sopran ein fehlender Kegel im Verhaeltnis der Laengen (150·1490/710 = 315 mm): r0 = tan(1,52°)·315 mm =
    // 8,4 mm, Volumen πr0²·L/3 = 23 cm³ (Rechnung, offen)
    const double fehlend = 0.150 * 1490.0 / 710.0;
    const double r0 = std::tan(1.52 * std::numbers::pi / 180.0) * fehlend;
    s.profil = kegel(r0, 1.52, 1.490, std::numbers::pi * r0 * r0 * fehlend / 3.0);
    s.druckMaxPa = 7800.0;
    s.kennzeichen = "ungemessen (Blatt der Klarinette uebertragen, Mundstueck abgeleitet)";
    if (!gestimmt) return s;
#ifdef IC_ROHRBLATT_STIMMUNG
    stimmungTenorsaxophon(s);
#else
    stimme(s);
#endif
    normiere(s);
    return s;
}

RohrblattSpiel baueVorgabeAltsaxophon(bool gestimmt)
{
    RohrblattSpiel s;
    s.name = "Altsaxophon";
    s.noteMin = 49;  // D♭3 … A♭5 klingend (offen)
    s.noteMax = 80;
    s.registerAb = 67;  // wie Sopran und Tenor: 18 Halbtoene ueber dem tiefsten Ton (offen)
    s.registerMode = 2;
    // Petersen, Colinot, Kergomard, Guillemain, Acta Acustica 2020, 4, 13, Tab. A1 S. 11 (nachgelesen, CC BY 4.0): „apex
    // angle θ = 0.028 rad and input radius a_in = 6.25 mm, corresponding to the geometry of the alto saxophone analyzed in
    // Section 3“ (Buffet Crampon) — in Tab. A1 waechst der Radius mit 0,028 je Laenge (Loch 10: 38,5 mm bei 1151,2 mm),
    // also r = 6,25 mm + 0,028·x (Halbwinkel atan 0,028 = 1,60°); die Laenge „about 105 cm“ (Ukshini & Dirckx, Acoustics
    // 2025, 7, 15, S. 3, nachgelesen, CC BY 4.0: Selmer Series II mit Mundstueck) als Laenge des Kegels (offen: mit oder
    // ohne Mundstueck); der fehlende Kegel 6,25/0,028 = 223 mm als Mundstueck mit seinem Volumen πr²·L/3 = 9,1 cm³
    // (Rechnung wie beim Tenor). Probe: der volle Kegel 1,273 m hat c/2L = 135 Hz, der tiefste Ton D♭3 138,6 Hz
    const double r0 = 0.00625, steigung = 0.028;
    const double fehlend = r0 / steigung;
    s.profil = kegel(r0, std::atan(steigung) * 180.0 / std::numbers::pi, 1.05, std::numbers::pi * r0 * r0 * fehlend / 3.0);
    s.druckMaxPa = 7800.0;  // Altsax 1,5–7,8 kPa (Fuks & Sundberg, nur Bericht)
    s.kennzeichen = "ungemessen (Kegel nach Petersen 2020 und Ukshini 2025; Blatt der Klarinette uebertragen, Mundstueck abgeleitet)";
    if (!gestimmt) return s;
#ifdef IC_ROHRBLATT_STIMMUNG
    stimmungAltsaxophon(s);
#else
    stimme(s);
#endif
    normiere(s);
    return s;
}

RohrblattSpiel baueVorgabeOboe(bool gestimmt)
{
    RohrblattSpiel s;
    s.name = "Oboe";
    s.noteMin = 58;  // B♭3 … G6 (offen)
    s.noteMax = 91;
    s.registerAb = 76;  // E5: ab hier mit der Oktavklappe (offen)
    s.registerMode = 2;
    // Mamou-Mani, Sharp, Meurisse, Ring: „Investigating the consistency of woodwind instrument manufacturing by comparing
    // five nominally identical oboes“ (Open University, eingereichte Fassung), Abb. 11 S. 24 (Messscheiben 2,0 … 7,5 mm,
    // Einstecktiefe — Punkte abgelesen; K853/K855/K856/K862 liegen aufeinander): Ober- und Unterstueck vom Rohr-Ende,
    // Abstand/Radius in mm 0/2,0 · 35/2,5 · 73/3,0 · 114/3,5 · 155/4,0 · 196/4,5 · 241/5,0 · 279/5,5 · 316/6,0 · 354/6,5 ·
    // 395/7,0 · 432/7,5 — ein Kegel mit 0,0127 je Laenge (Halbwinkel 0,73°; Chen 2009: 0,71°, nur Bericht). Der Becher
    // fehlt (die groesste Scheibe passte nicht): derselbe Kegel weiter bis zur Laenge des tiefsten Tons (Rechnung, offen);
    // Rohr und Huelse als der fehlende Kegel 2,0/0,0127 = 157 mm mit seinem Volumen (Rechnung wie beim Saxophon)
    s.profil.punkte = {{0.000, 0.0020}, {0.035, 0.0025}, {0.073, 0.0030}, {0.114, 0.0035}, {0.155, 0.0040}, {0.196, 0.0045},
                       {0.241, 0.0050}, {0.279, 0.0055}, {0.316, 0.0060}, {0.354, 0.0065}, {0.395, 0.0070}, {0.432, 0.0075}};
    const double steigung = (0.0075 - 0.0020) / 0.432;
    const double fehlend = 0.0020 / steigung;
    const double voll = 343.0 / (2.0 * 440.0 * std::exp2((58 - 69) / 12.0));  // c/2f des tiefsten Tons: der volle Kegel
    s.profil.punkte.push_back({voll - fehlend, 0.0075 + steigung * (voll - fehlend - 0.432)});
    s.profil.mundstueckM3 = std::numbers::pi * 0.0020 * 0.0020 * fehlend / 3.0;
    // Befund (IC Session 4): mit dem Blatt der Klarinette 7 von 36 Pruefpunkten auf ±10 Cent — das untere Register
    // −11 … −65 Cent, ab der Oktavklappe ueber eine Oktave zu tief; das Doppelrohrblatt (Oeffnung, Steife, Kennlinie) ist
    // ohne Quelle — darum nicht spielbar (T8 a), die Bohrung bleibt
    s.spielbar = false;
    s.grund = "Bohrung da (Mamou-Mani et al. Abb. 11); das Blatt der Klarinette treibt die Oboe nicht (7/36) — Doppelrohrblatt offen";
    s.kennzeichen = "nicht spielbar (Doppelrohrblatt offen; Kegel abgelesen)";
    (void)gestimmt;
    return s;
}

RohrblattSpiel baueVorgabeFagott()
{
    return geruest("Fagott", 34, 75, "nur der Halbwinkel 0,41° und Barockfagott (Dart 2011, nicht abgerufen) — F-B2");
}

}  // namespace ic::rohrblatt
