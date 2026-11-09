/**
 * @file Vorgaben.cpp
 * @brief InstrumentModels, Kern `struckString`: Vorgaben fuer Clavinet, Hackbrett, Cimbalom (F-6 b: aus Literatur und
 *        Modell, ungemessen)
 *
 * Uebernommen aus IC `rechnung/klassen/01_hammer_saite/SaitenVorgaben.cpp` (`bb7e6d6`), die Werte wortgleich;
 * geaendert (Instrumente_IC_Konzept.md 34i-3): `normiere` ist `kern::normiereNachTafel` (IK2-10 a, der Bezug ist ein
 * Wert der Tafel — gesetzt von `InstrumentTuneTool --reference`); die Quellen je Kurve (WZ1 a) setzt `mitQuellen`.
 *
 * Belegkraft wie in `klassen/04_platte_stab/Vorgaben.cpp`: *Quelle* = nachgelesen (Gabrielli et al. 2013, IC Session 1),
 * *nur Bericht* = Wortlaut im Bericht `recherche/2026-10-26_upright_honkytonk_cimbalom.md` bzw. `…_cp70_clavinet.md`,
 * *Rechnung*, *offen* (Vorgabe mit Grund). Tabelle aller Groessen: `klassen/Klasse 1 - Hammer-Saite/modell.md`.
 */
#include "InstrumentModels/hammer/HammerSaite.hpp"
#include "InstrumentModels/saite/Saite.hpp"

#include <cmath>
#include <initializer_list>
#include <numbers>
#include <string>

namespace lumi::kern::saite
{

namespace
{

constexpr double kPi = std::numbers::pi;

Kurve konst(double w, Belegkraft b, const std::string& h, const SaitenSpiel& s)
{
    return kurveKonstant(w, b, h, s.noteMin, s.noteMax);
}

/// Eine Kurve des Fluegels auf diesen Umfang uebertragen (dieselben Stuetzpunkte).
Kurve vomFluegel(const Kurve& k, const std::string& was)
{
    Kurve c = k;
    c.belegkraft = Belegkraft::Offen;
    c.herkunft = was + " vom Fluegel uebertragen (Vorbild, an Salamander gemessen) — offen";
    return c;
}

/// Die Schluessel der Quellen je Kurve (WZ1 a), je Instrument aus der Herkunft oben — ausdruecklich, nicht nach Namen im
/// Text (das B des Clavinets nennt Gabrielli, der Wert 10⁻⁴ ist aber offen). Was ohne Quelle gesetzt ist, traegt
/// `lumiviz`; eine vom Fluegel uebertragene Kurve die Messung an *Salamander*.
void mitQuellen(SaitenSpiel& s)
{
    for (Kurve* k : {&s.laengeM, &s.durchmesserMm, &s.inharmonizitaet, &s.t60S, &s.t60SpaetS, &s.abklingHoch, &s.chor, &s.verstimmungCent,
                     &s.polarisationWinkelGrad, &s.polarisationCent, &s.waagrechtVerhaeltnis, &s.anschlagStelle, &s.daempferT60S,
                     &s.garnT60S, &s.schieberT60S})
        k->quellen = {"lumiviz"};
    if (s.name == "Clavinet")
    {
        // T60 „20 s or more“ (Gabrielli et al. 2013, nur Bericht): die Grenze aus der Quelle, gesetzt 10 s
        s.t60S.quellen = s.t60SpaetS.quellen = {"gabrielli2013"};
        // CL1 a (S183): die Staerken je Taste von zwei Haendlern (nur Bericht), die Zahl der umsponnenen Saiten aus Gabrielli
        s.durchmesserMm.quellen = {"vintagevibe-clavinet-strings", "electrickeys-clavinet-strings", "gabrielli2013"};
        return;
    }
    // Hackbrett, Cimbalom: Laenge, Masse, B aus der Spannungstabelle (Peterson 2005, nur Bericht); das Abklingen vom
    // Fluegel uebertragen (an Salamander gemessen), weil Peterson es „comparable to that of an undamped piano“ nennt
    s.laengeM.quellen = s.durchmesserMm.quellen = s.inharmonizitaet.quellen = {"peterson2005"};
    s.t60S.quellen = s.t60SpaetS.quellen = {"salamander", "peterson2005"};
    s.abklingHoch.quellen = {"salamander"};
}

/// Peterson 2005 (Acoustics Today, Spannungstabelle rechter Diskantsteg; nur Bericht, Wortlaut im Bericht): Ton, Radius
/// (Zoll), Laenge (Zoll), Spannung (lb) — G5 0,009/9/35, D5 0,010/12/43, G4 0,010/15/30, D4 0,011/18/29,
/// G♯3 0,012/21/21 — der Bericht nennt bei der letzten Zeile „G#3 196“; 196 Hz ist G3, genommen ist die Frequenz (MIDI 55). Stahl ρ = 7850 kg/m³, E = 2·10¹¹ Pa (Lehrbuch, offen).
struct PetersonZeile
{
    int note;
    double radiusZoll, laengeZoll, spannungLb;
};
constexpr PetersonZeile kPeterson[] = {{55, 0.012, 21, 21}, {62, 0.011, 18, 29}, {67, 0.010, 15, 30}, {74, 0.010, 12, 43}, {79, 0.009, 9, 35}};

SaitenSpiel hackbrettArt(const std::string& name, int von, int bis)
{
    SaitenSpiel s;
    s.name = name;
    s.noteMin = von;
    s.noteMax = bis;
    const std::string peterson = "Peterson 2005, Spannungstabelle (nur Bericht)";
    for (const PetersonZeile& z : kPeterson)
    {
        const double r = z.radiusZoll * 0.0254;
        const double l = z.laengeZoll * 0.0254;
        const double t = z.spannungLb * 4.44822;
        s.laengeM.punkte.push_back({z.note, l});
        s.durchmesserMm.punkte.push_back({z.note, 2.0 * r * 1e3});  // CL1 a (S183): der Durchmesser; μ = ρπd²/4 rechnet der Kern
        const double d = 2.0 * r;
        s.inharmonizitaet.punkte.push_back({z.note, kPi * kPi * kPi * 2e11 * d * d * d * d / (64.0 * t * l * l)});
    }
    s.laengeM.belegkraft = s.durchmesserMm.belegkraft = Belegkraft::Offen;
    s.laengeM.herkunft = peterson + "; ausserhalb G♯3–G5 der Randwert (offen)";
    s.durchmesserMm.herkunft = "2r mit r aus " + peterson + "; ausserhalb G♯3–G5 der Randwert (offen)";
    s.inharmonizitaet.belegkraft = Belegkraft::Rechnung;
    s.inharmonizitaet.herkunft = "B = π³Ed⁴/(64TL²) (Lehrbuch) mit d, T, L aus " + peterson + " (Rechnung)";
    const hammer::HammerSaite& f = hammer::hammerSaiteVorgabe();
    s.t60S = vomFluegel(f.abklingSpaetS, "„sustain … comparable to that of an undamped piano“ (Peterson 2005, nur Bericht): T60 der zweiten Stufe");
    s.t60SpaetS = s.t60S;
    s.abklingHoch = vomFluegel(f.abklingHoch, "hohe Teiltoene");
    s.chor = konst(2.0, Belegkraft::Offen, "offen: zwei Saiten je Chor (Bericht: 2–5)", s);
    s.verstimmungCent = konst(0.3, Belegkraft::Offen,
                              "offen: 0,3 Cent — der doppelte Ausklang „can not be found in hammered dulcimers“ (Mayer & Rust 2010 nach "
                              "Peterson, nur Bericht), also eng gestimmt", s);
    s.polarisationWinkelGrad = konst(0.0, Belegkraft::Offen, "offen: ohne zweite Ebene", s);
    s.polarisationCent = konst(0.01, Belegkraft::Offen, "Untergrenze (L11)", s);
    s.waagrechtVerhaeltnis = konst(0.0, Belegkraft::Offen, "offen", s);
    s.anschlagStelle = konst(0.15, Belegkraft::Offen, "offen: nahe dem Steg (Einschaetzung)", s);
    s.anregung = Anregung::Schlaegel;
    // Schlaegel 8–12 g (Peterson 2005: „Standard wooden dulcimer hammers are much lighter (8-12 grams)“, nur Bericht);
    // die Steife des Holzkopfs vom Orff-Schlaegel (Rucz & Ulveczki 2021 Tab. 1, uebertragen, offen)
    s.schlaegel.masseKg = 0.010;
    s.schlaegel.q0 = 5e7 * std::pow(1e-3, 1.5);
    s.schlaegel.p = 1.5;
    s.tempoMax = 4.0;
    s.ueberabtastung = 32;
    s.stegTeilung = 0.6;  // 3 : 2 (Peterson 1994, nur Bericht); angeschlagen der lange Teil
    s.mittelstegKopplung = 0.5;  // offen: die Haelfte der Verluste der ersten Stufe geht ueber den Steg (Einschaetzung)
    s.daempfer = false;   // „Hackbrett ohne Daempfer“ (Bericht)
    // Mitschwingen der anderen Choere (Deluxe, je Clip — T2 a, K2 F-3 a); Kopplung wie am Mittelsteg (offen)
    s.mitschwingen = true;
    s.mitschwingKopplung = 0.5;
    s.daempferT60S = konst(0.3, Belegkraft::Offen, "ohne Daempfer", s);
    s.garnT60S = konst(1.0, Belegkraft::Offen, "ohne Bedeutung", s);
    s.schieberT60S = konst(1.0, Belegkraft::Offen, "ohne Bedeutung", s);
    s.abnahme = Abnahme::Steg;
    s.kennzeichen = "Deluxe, ungemessen (keine freie Aufnahme; F-1 a: Hackbrett und Cimbalom tragen die Marke Deluxe)";
    return s;
}

}  // namespace

SaitenSpiel baueVorgabeClavinet()
{
    SaitenSpiel s;
    s.name = "Clavinet";
    s.noteMin = 29;  // F1–E6, 60 Tasten (offen)
    s.noteMax = 88;
    s.laengeM = konst(0.5, Belegkraft::Offen,
                      "offen: 0,5 m — bestimmt die Lage der Tonabnehmer relativ (Steg 4 cm fest); eine Laengentabelle je Taste fehlt (Remaggi 2012: "
                      "67,8 cm bei 161 Hz)", s);
    // CL1 a (S183): die Staerken je Taste nach Vintage Vibe (Zaehlung 10/5/9/3/33) und Electric Keys — .032″ F1–D2, .028″ D♯2–G2,
    // .022″ G♯2–E3, .011″ F3–G3, .009″ G♯3–E6 (nur Bericht; Gabrielli 2013: „the first 23 strings wound“, Original .008″ statt .009″);
    // die Umspinnung als Stahl gerechnet (der ganze Durchmesser zaehlt am Tonabnehmer — Material nirgends belegt, offen)
    s.durchmesserMm = kurveAusPunkten({{29, 0.8128}, {38, 0.8128}, {39, 0.7112}, {43, 0.7112}, {44, 0.5588}, {52, 0.5588},
                                       {53, 0.2794}, {55, 0.2794}, {56, 0.2286}, {88, 0.2286}},
                                      Belegkraft::Offen,
                                      "Staerken je Taste .032/.028/.022/.011/.009 Zoll (Vintage Vibe, Electric Keys: nur Bericht; Gabrielli 2013: 23 "
                                      "umsponnen); Umspinnung als Stahl gerechnet (offen) — S183 CL1 a, Herleitung "
                                      "2026-11-09_S183_vorgaben_je_taste.md §4");
    s.inharmonizitaet = konst(1e-4, Belegkraft::Offen, "offen: B je Taste bei Gabrielli et al. 2013 Tab. 1 (nur Bericht, Werte nicht im Repo)", s);
    s.t60S = konst(10.0, Belegkraft::Offen, "offen: T60 „20 s or more“ (Gabrielli, nur Bericht) — 10 s", s);
    s.t60SpaetS = s.t60S;
    s.abklingHoch = konst(0.3, Belegkraft::Offen, "offen", s);
    s.chor = konst(1.0, Belegkraft::Rechnung, "eine Saite je Taste", s);
    s.verstimmungCent = konst(0.0, Belegkraft::Rechnung, "eine Saite", s);
    s.polarisationWinkelGrad = konst(0.0, Belegkraft::Offen, "offen: ohne zweite Ebene", s);
    s.polarisationCent = konst(0.01, Belegkraft::Offen, "Untergrenze (L11)", s);
    s.waagrechtVerhaeltnis = konst(0.0, Belegkraft::Offen, "offen", s);
    s.anschlagStelle = konst(0.0, Belegkraft::Rechnung, "die Tangente trifft das Ende des klingenden Teils", s);
    s.anregung = Anregung::Tangente;
    s.tempoMax = 4.0;  // „1 to 4 m/s“ (Gabrielli et al. 2013 S. 11, nachgelesen)
    s.tangenteWegMm = 1.0;  // offen
    s.ueberabtastung = 32;
    s.daempfer = false;
    s.daempferT60S = konst(0.3, Belegkraft::Offen, "ohne Bedeutung (das Garn daempft)", s);
    s.ganzJeKlingend = std::exp2(3.0 / 12.0);  // −3 Halbtoene nach dem Loslassen (Gabrielli S. 6, nachgelesen)
    s.garnT60S = konst(0.15, Belegkraft::Offen, "offen: die Garnwicklung daempft die ganze Saite (Einschaetzung)", s);
    s.schieberT60S = konst(0.3, Belegkraft::Offen, "offen: Daempferschieber in Stellung 1 (Einschaetzung)", s);
    s.tastendruckGrenzeCent = 0.0;  // Entscheid nach FRAGEN_KONSOLIDIERT / K1 F-4-1 a: Grenze 0 bis zu einer Messung
    s.abnahme = Abnahme::Magnetisch;
    s.abnehmerStellen = {0.10, 0.30};  // offen: Lagen nach Remaggi 2012 (4 cm bzw. 6,5–18,5 cm vom Ende, nur Bericht) auf 0,5 m
    s.verstaerker = true;  // Shelf −3 dB/130 Hz, +3 dB/4 kHz (Gabrielli et al. 2013 S. 6, nachgelesen); Klangschalter offen
    s.kennzeichen = "ungemessen (keine freie Aufnahme, F-6 b)";
    mitQuellen(s);
    return s;
}

SaitenSpiel baueVorgabeHackbrett()
{
    SaitenSpiel s = hackbrettArt("Hackbrett", 43, 86);
    mitQuellen(s);
    return s;
}

SaitenSpiel baueVorgabeCimbalom()
{
    SaitenSpiel s = hackbrettArt("Cimbalom", 38, 88);
    s.chor = kurveAusPunkten({{38, 3.0}, {55, 3.0}, {56, 4.0}, {88, 4.0}}, Belegkraft::Offen,
                             "vier Saiten im Diskant, drei im Bass (Wikipedia „Cimbalom“, nur Bericht); Grenze offen");
    // Schlaegel mit Baumwolle („two sticks, often with cotton-wound tips“, Wikipedia, nur Bericht): ein weicher Filz —
    // Stulov wie der Fluegel an MIDI 40 (uebertragen, offen)
    s.schlaegel = Stossanreger{};
    s.schlaegel.masseKg = 0.010;
    s.schlaegel.q0 = hammer::stulovHammerSteife(20);
    s.schlaegel.p = hammer::stulovHammerExponent(20);
    s.schlaegel.eps = hammer::stulovHammerHysterese(20);
    s.schlaegel.tauS = hammer::stulovHammerRelaxationUs(20) * 1e-6;
    s.daempfer = true;  // Daempferleiste am Pedal; ohne Pedal gedaempft (Bericht)
    s.daempferT60S = konst(0.3, Belegkraft::Offen, "offen: Daempferleiste (Einschaetzung)", s);
    mitQuellen(s);
    return s;
}

}  // namespace lumi::kern::saite
