/**
 * @file Vorgaben.cpp
 * @brief Klasse 3: Vorgaben — Cello (Kern zuerst, F-1 a), Violine; Viola und Kontrabass aus Haendlerdaten, ungemessen
 *        (F-B3 a)
 *
 * Belegkraft: *Quelle* (Galluzzo et al. 2017 Gl. 1, Zhang & Woodhouse 2014 Tab. II, Schoonderwaldt et al. 2008 S. 604,
 * nachgelesen IC Session 1), *nur Bericht* (`klassen/Klasse 3 - Bogen-Saite/recherche/2026-10-26_physik_rechenverfahren.md`
 * §4), *Haendler* (Saitenspannungen, nach Saiten_Konzept §6b als Herstellerdaten), *Rechnung*, *offen*. Tabelle:
 * `klassen/Klasse 3 - Bogen-Saite/modell.md`.
 */
#include "klassen/03_bogen_saite/Bogen.hpp"

#include "gemeinsam/Werkzeug.hpp"

#include <cmath>

namespace ic::bogen
{

#if __has_include("klassen/03_bogen_saite/Stimmung.inc")
#include "klassen/03_bogen_saite/Stimmung.inc"
#define IC_BOGEN_STIMMUNG 1
#endif

namespace
{

double hzVon(int note)
{
    return 440.0 * std::exp2((note - 69) / 12.0);
}

/// Z0 = T/c = T/(2·L·f) aus Spannung, Mensur und Stimmung (Rechnung).
double z0Aus(double spannungN, double mensurM, int note)
{
    return spannungN / (2.0 * mensurM * hzVon(note));
}

void normiere(StreichSpiel& s)
{
    s.ausgangBezug = 1.0;
    if (!s.spielbar) return;
    StreichAnschlag a;
    const int n = (s.noteMin + s.noteMax) / 2;
    a.kette = {{0.0, n, hzVon(n)}};
    a.expression = 1.0;
    a.haltenS = 0.6;
    std::vector<float> ton;
    rechneStreichTon(s, a, ton, static_cast<std::int64_t>(0.6 * a.rate));
    const double sp = spitze(ton);
    s.ausgangBezug = sp > 0.0 ? sp / 0.5 : 1.0;
}

}  // namespace

StreichSpiel baueVorgabeCello(bool gestimmt)
{
    StreichSpiel s;
    s.name = "Cello";
    s.noteMin = 36;  // C2 … A5 (offen: die Hoehe der Daumenlage)
    s.noteMax = 81;
    s.mensurM = 0.69;  // „auf 690 mm bezogen“ (aitchisoncellos.com, Haendler — nur Bericht); passt zu Galluzzo: Z0 = T/(2Lf)
    // D3 nach Galluzzo et al. 2017 Tab. 1 (nur Bericht): 146,8 Hz, 111 N, 2,7 g/m, Z0 0,55 kg/s; Q 500 (Serafin et al.
    // 1999, nur Bericht). C, G, A: Z0 aus den Spannungen Larsen (Haendler, nur Bericht) 30,1 / 27,6 / 39,3 lb; Q wie D (offen)
    const double lb = 4.448;
    s.saiten = {{36, z0Aus(30.1 * lb, s.mensurM, 36), 500.0, "Larsen C 30,1 lb (Haendler), Z0 = T/(2Lf)"},
                {43, z0Aus(27.6 * lb, s.mensurM, 43), 500.0, "Larsen G 27,6 lb (Haendler), Z0 = T/(2Lf)"},
                {50, 0.55, 500.0, "Galluzzo et al. 2017 Tab. 1 (nur Bericht)"},
                {57, z0Aus(39.3 * lb, s.mensurM, 57), 500.0, "Larsen A 39,3 lb (Haendler), Z0 = T/(2Lf)"}};
    s.bogenAbstandM = 0.055;  // β = 0,08 an D3 (Galluzzo, nur Bericht) × 0,69 m; Spieler 4,5–9,2 cm (Chudy Tab. 1, nachgelesen)
    s.vbPp = 0.08;            // um 0,166 m/s im Legato (Chudy, nachgelesen) — der Weg 0,08 … 0,4 offen
    s.vbFf = 0.4;
    // Korpus am Steg: die Wolfton-Mode 171,7–176,5 Hz, Q 13,4 (vom Spieler gehalten), −25,6 dB re 1 m/(N·s) = 0,0525 s/kg
    // (Zhang & Woodhouse 2014 Tab. II, Quelle); A0 „near 104 Hz“ und der Steghuegel um 1 kHz (nur Bericht) — Guete und
    // Staerke offen (am Modalfit B6 zu setzen, lokal)
    s.korpus = {{104.0, 20.0, 0.02}, {174.1, 13.4, 0.0525}, {1000.0, 2.0, 0.03}};
    s.kennzeichen = "ungemessen (Korpus ausser der Wolfton-Mode offen)";
    if (!gestimmt) return s;
#ifdef IC_BOGEN_STIMMUNG
    stimmungCello(s);
#else
    stimme(s);
#endif
    normiere(s);
    return s;
}

StreichSpiel baueVorgabeVioline(bool gestimmt)
{
    StreichSpiel s;
    s.name = "Violine";
    s.noteMin = 55;  // G3 … C7 (offen)
    s.noteMax = 96;
    s.mensurM = 0.327;  // Bestand LumiViz (Saiten_Konzept, Vorlage Violine)
    // D und E nach Schoonderwaldt et al. 2008 S. 607 (nur Bericht): Z0 0,25 / 0,18 kg/s; Q 410 an der Violine (ebd.
    // S. 618, nur Bericht); G und A aus den Spannungen Dominant (Haendler, nur Bericht) 4,4 / 5,5 kg
    s.saiten = {{55, z0Aus(4.4 * 9.81, s.mensurM, 55), 410.0, "Dominant G 4,4 kg (Haendler), Z0 = T/(2Lf)"},
                {62, 0.25, 410.0, "Schoonderwaldt et al. 2008 S. 607 (nur Bericht)"},
                {69, z0Aus(5.5 * 9.81, s.mensurM, 69), 410.0, "Dominant A 5,5 kg (Haendler), Z0 = T/(2Lf)"},
                {76, 0.18, 410.0, "Schoonderwaldt et al. 2008 S. 607 (nur Bericht)"}};
    s.bogenAbstandM = 0.025;  // offen (β ≈ 1/30 … 1/6, nur Bericht)
    s.vbPp = 0.08;
    s.vbFf = 0.4;
    // Korpus: A0 270–275, CBR ~407, B1− ~475, B1+ ~541 Hz, ζ ≈ 1 % (Bestand LumiViz S132); Steghuegel um 2,3 kHz (nur
    // Bericht). **Staerken offen** (F-7 a: aus CNSM mit Maestre 2013 zu bestimmen — lokal, Setz-Werkzeug)
    s.korpus = {{272.5, 50.0, 0.05}, {407.0, 50.0, 0.05}, {475.0, 50.0, 0.05}, {541.0, 50.0, 0.05}, {2300.0, 2.0, 0.05}};
    s.kennzeichen = "ungemessen (Staerken des Korpus offen)";
    if (!gestimmt) return s;
#ifdef IC_BOGEN_STIMMUNG
    stimmungVioline(s);
#else
    stimme(s);
#endif
    normiere(s);
    return s;
}

// Viola und Kontrabass nach F-B3 a (Patrik 2026-10-30): Mensur und Spannungen aus Haendlerangaben (Saiten_Konzept §6b als
// Herstellerdaten), Z0 = T/(2Lf), Guete von Violine bzw. Cello uebertragen, Korpus aus den Bestandsfrequenzen mit offenen
// Staerken — gebaut und „ungemessen“; die Recherche nach Askenfelt 1982 und einer Viola-Admittanz laeuft als Auftrag weiter.

StreichSpiel baueVorgabeViola(bool gestimmt)
{
    StreichSpiel s;
    s.name = "Viola";
    s.noteMin = 48;  // C3 … E6 (offen)
    s.noteMax = 88;
    // 375 mm fuer 16–16,5 Zoll (For-Tune Charm, Thomann; D'Addario Kaplan Vivo LM 380 mm — Haendler, nur Bericht)
    s.mensurM = 0.375;
    // Dominant 141 medium (electricviolinshop.com, Haendler, nur Bericht): C 4,9 / G 4,8 / D 5,0 / A 6,0 kg; Q 410 von
    // der Violine uebertragen (F-B3 a)
    s.saiten = {{48, z0Aus(4.9 * 9.81, s.mensurM, 48), 410.0, "Dominant C 4,9 kg (Haendler), Z0 = T/(2Lf)"},
                {55, z0Aus(4.8 * 9.81, s.mensurM, 55), 410.0, "Dominant G 4,8 kg (Haendler), Z0 = T/(2Lf)"},
                {62, z0Aus(5.0 * 9.81, s.mensurM, 62), 410.0, "Dominant D 5,0 kg (Haendler), Z0 = T/(2Lf)"},
                {69, z0Aus(6.0 * 9.81, s.mensurM, 69), 410.0, "Dominant A 6,0 kg (Haendler), Z0 = T/(2Lf)"}};
    s.bogenAbstandM = 0.029;  // offen: der Abstand der Violine im Verhaeltnis der Mensuren (Rechnung)
    s.vbPp = 0.08;
    s.vbFf = 0.4;
    // Korpus: die Moden der Violine, verschoben um die Faktoren der Leistungsantwort „23, 31, 35, and 44 percent lower“ an
    // 290 / 530 / 1500 / 3350 Hz (Paetynen & Lokki S. 24, wortlich): A0 × 0,77, CBR/B1−/B1+ × 0,69, Steghuegel × 0,65
    // (Rechnung); Guete und Staerke wie Violine — **offen**
    s.korpus = {{272.5 * 0.77, 50.0, 0.05}, {407.0 * 0.69, 50.0, 0.05}, {475.0 * 0.69, 50.0, 0.05},
                {541.0 * 0.69, 50.0, 0.05}, {2300.0 * 0.65, 2.0, 0.05}};
    s.kennzeichen = "ungemessen (Saiten Haendler, Korpus verschoben, Staerken offen)";
    if (!gestimmt) return s;
#ifdef IC_BOGEN_STIMMUNG
    stimmungViola(s);
#else
    stimme(s);
#endif
    normiere(s);
    return s;
}

StreichSpiel baueVorgabeKontrabass(bool gestimmt)
{
    StreichSpiel s;
    s.name = "Kontrabass";
    s.noteMin = 28;  // E1 … G4 (offen)
    s.noteMax = 67;
    s.mensurM = 1.06;  // 3/4-Mensur 106 cm (Thomann zu Spirocore 3/4, Haendler — nur Bericht)
    // Spirocore 3885.0 medium, Orchesterstimmung (Long & McQuade, Haendler, nur Bericht): E 33 / A 32 / D 31 / G 30,5 kg;
    // Q 500 vom Cello uebertragen (F-B3 a)
    s.saiten = {{28, z0Aus(33.0 * 9.81, s.mensurM, 28), 500.0, "Spirocore E 33 kg (Haendler), Z0 = T/(2Lf)"},
                {33, z0Aus(32.0 * 9.81, s.mensurM, 33), 500.0, "Spirocore A 32 kg (Haendler), Z0 = T/(2Lf)"},
                {38, z0Aus(31.0 * 9.81, s.mensurM, 38), 500.0, "Spirocore D 31 kg (Haendler), Z0 = T/(2Lf)"},
                {43, z0Aus(30.5 * 9.81, s.mensurM, 43), 500.0, "Spirocore G 30,5 kg (Haendler), Z0 = T/(2Lf)"}};
    s.bogenAbstandM = 0.085;  // offen: β = 0,08 wie am Cello × 1,06 m (Rechnung)
    s.vbPp = 0.08;
    s.vbFf = 0.4;
    // Korpus: A0 66 Hz (65–67) und T1 115 Hz (Bestand LumiViz S132); Guete und Staerke wie die Cello-Moden — **offen**
    // (Askenfelt 1982 nicht erreicht)
    s.korpus = {{66.0, 20.0, 0.02}, {115.0, 13.4, 0.0525}};
    s.kennzeichen = "ungemessen (Saiten Haendler, Guete und Staerke des Korpus offen)";
    if (!gestimmt) return s;
#ifdef IC_BOGEN_STIMMUNG
    stimmungKontrabass(s);
#else
    stimme(s);
#endif
    normiere(s);
    return s;
}

}  // namespace ic::bogen
