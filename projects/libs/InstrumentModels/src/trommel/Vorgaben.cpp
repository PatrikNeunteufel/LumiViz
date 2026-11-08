/**
 * @file Vorgaben.cpp
 * @brief InstrumentModels, Klasse 5: Vorgaben — Tom (Kern zuerst, F-1 a), Pauke, Bassdrum, Snare, Tabla; Conga und Bongo gebaut und
 *        ungemessen (F-7 a abgewandelt: nichts zurueckstellen)
 *
 * Uebernommen aus IC `rechnung/klassen/05_membran/Vorgaben.cpp` (`bb7e6d6`), die Werte wortgleich; geaendert
 * (Instrumente_IC_Konzept.md 34h-3, Uebernahme-Journal J45): `normiere` ist `kern::normiereNachTafel` (zwei Runden,
 * IK2-10 a), die Moden rechnet `bereite` erst der Kern (die Vorgaben bleiben Daten); das Kennzeichen „ungemessen“
 * folgt aus der Herkunft (IK2-9); der Belag der Tabla steht als Werte (`Belag`).
 *
 * Belegkraft: *Quelle* (Dahl 2011 PDF-S. 2, Marogna et al. 2010, nachgelesen IC Session 1), *nur Bericht* (Bericht
 * `klassen/Klasse 5 - Membran/recherche/2026-10-26_physik_rechenverfahren.md`: Packer 1993, Avanzini & Marogna 2010,
 * Bilbao et al., Sathej & Adhikari 2008), *Haendler*, *Rechnung*, *offen*. Tabelle: `klassen/Klasse 5 - Membran/modell.md`.
 */
#include "InstrumentModels/trommel/Trommel.hpp"

#include <cmath>
#include <numbers>

namespace lumi::kern::trommel
{

namespace
{

constexpr double kPi = std::numbers::pi;

/// Der Stock (Snare, Tom): Masse und Kontakt offen — gesetzt an die Kontaktzeit 5–8 ms (Dahl) bzw. 3–6 ms (Wagner).
Stossanreger stock()
{
    Stossanreger s;
    s.masseKg = 0.05;  // offen
    s.q0 = 60.0;       // N/mm^1,5 (offen, an die Kontaktzeit gesetzt)
    s.p = 1.5;
    return s;
}

/// Der Paukenschlaegel nach Bilbao et al. (nur Bericht): 0,028 kg, K = 1,6·10⁸ N/m^2,54 = 4,0 N/mm^2,54.
Stossanreger paukenschlaegel()
{
    Stossanreger s;
    s.masseKg = 0.028;
    s.q0 = 1.6e8 * std::pow(1e-3, 2.54);
    s.p = 2.54;
    return s;
}

Fell fell(double radius, double spannung, double dichte = 0.27)
{
    Fell f;
    f.radiusM = radius;
    f.spannungNm = spannung;
    f.dichteKgM2 = dichte;  // Avanzini & Marogna S. 898: 0,27 kg/m² (nur Bericht)
    f.innenLuft = 1.0;      // die Luft im Kessel als Masse wie ein Halbraum (Naeherung, offen)
    return f;
}

}  // namespace

TrommelSpiel baueVorgabeTom()
{
    TrommelSpiel s;
    s.name = "Tom";
    s.note = 45;  // Rolle tom (A2 im GM-Kit: Low Tom)
    s.noteMax = 45;
    // Avanzini & Marogna S. 896–899 (nur Bericht): R 16–20 cm, T₁ = 2000 / T₂ = 1800 N/m (Schwebungen) — R 0,165 m
    // (13″, Haendler); Kesseltiefe 0,28 m (offen)
    s.felle = {fell(0.165, 2000.0), fell(0.165, 1800.0)};
    s.kessel.volumenM3 = kPi * 0.165 * 0.165 * 0.28;
    s.schlagstelleRel = 0.4;  // offen
    s.schlaegel = stock();
    s.bisHz = 2500.0;
    // der Bezug: kern::normiereNachTafel (IK2-10 a); die Moden rechnet `bereite` (der Kern beim Bau)
    return s;
}

TrommelSpiel baueVorgabePauke()
{
    TrommelSpiel s;
    s.name = "Pauke";
    s.note = 48;  // 64 cm (25″): C3 … G3 (offen)
    s.noteMax = 55;
    s.stimmbar = true;
    // Packer 1993 (nur Bericht, nur Zahlen): a = 0,32 m, T = 5360 N/m, σ = 0,262 kg/m² (in sich widerspruechlich: 1650 ×
    // 1,9·10⁻⁴ = 0,31); Kessel L/a = 1,2 (S. 48) — als halbes Ellipsoid V = ⅔·π·a²·L (Rechnung, offen)
    Fell f = fell(0.32, 5360.0, 0.262);
    s.felle = {f};
    s.kessel.volumenM3 = 2.0 / 3.0 * kPi * 0.32 * 0.32 * 1.2 * 0.32;
    s.schlagstelleRel = 0.75;  // ¼ Radius vom Rand (nur Bericht)
    s.schlaegel = paukenschlaegel();
    s.tempoPp = 0.5;
    s.tempoFf = 5.0;  // Torin et al.: −5 m/s (nur Bericht)
    s.bisHz = 3000.0;
    // der Bezug: kern::normiereNachTafel (IK2-10 a); die Moden rechnet `bereite` (der Kern beim Bau)
    return s;
}

TrommelSpiel baueVorgabeBassdrum()
{
    TrommelSpiel s;
    s.name = "Bassdrum";
    s.note = 36;
    s.noteMax = 36;
    // 22″ (Haendler); Spannungen, Tiefe, Schlaegel **offen** (keine Zahlen gefunden — Bericht §2)
    s.felle = {fell(0.279, 1200.0), fell(0.279, 1000.0)};
    s.kessel.volumenM3 = kPi * 0.279 * 0.279 * 0.4;
    s.schlagstelleRel = 0.15;
    Stossanreger filz;
    filz.masseKg = 0.12;  // offen
    filz.q0 = 2.0;
    filz.p = 2.0;
    s.schlaegel = filz;
    s.tempoPp = 0.5;
    s.tempoFf = 4.0;
    s.bisHz = 1200.0;
    // der Bezug: kern::normiereNachTafel (IK2-10 a); die Moden rechnet `bereite` (der Kern beim Bau)
    return s;
}

TrommelSpiel baueVorgabeSnare()
{
    TrommelSpiel s;
    s.name = "Snare";
    s.note = 38;
    s.noteMax = 38;
    // 14″ (Haendler); Schlagfell T 3325 N/m, 0,26 kg/m² (Bilbao et al., Beispiel — nur Bericht); Resonanzfell duenner
    // (offen); Tiefe 0,14 m (offen)
    s.felle = {fell(0.178, 3325.0, 0.26), fell(0.178, 2500.0, 0.12)};
    s.kessel.volumenM3 = kPi * 0.178 * 0.178 * 0.14;
    s.schlagstelleRel = 0.3;
    s.schlaegel = stock();
    s.teppich = true;
    s.teppichPegel = 0.3;  // offen
    s.prellen = 1;
    // der Bezug: kern::normiereNachTafel (IK2-10 a); die Moden rechnet `bereite` (der Kern beim Bau)
    return s;
}

TrommelSpiel baueVorgabeTabla()
{
    TrommelSpiel s;
    s.name = "Tabla";
    s.note = 62;  // dayan (offen)
    s.noteMax = 62;
    // Sathej & Adhikari 2008 (nur Bericht): das syahi „approximately eight times as dense“, ein Viertel der Flaeche;
    // Profil tanh mit σ = 2,57, k = 0,492, ξ = 0,091 — hier als Dichte 1 + (σ² − 1)·½(1 − tanh((x − k)/ξ)) gedeutet
    // (σ² = 6,6 ≈ „eight“; offen); Radius 0,07 m, Spannung, Kessel offen
    Fell f = fell(0.07, 3000.0, 0.3);
    const Belag syahi{true, 2.57, 0.492, 0.091};
    f.belag = belagFunktion(syahi);
    s.felle = {f};
    s.belaege = {syahi};
    s.kessel.volumenM3 = kPi * 0.07 * 0.07 * 0.25;
    s.schlagstelleRel = 0.7;
    Stossanreger finger;
    finger.masseKg = 0.01;  // offen
    finger.q0 = 3.0;
    finger.p = 1.5;
    s.schlaegel = finger;
    s.tempoPp = 0.5;
    s.tempoFf = 4.0;
    s.bisHz = 4000.0;
    // der Bezug: kern::normiereNachTafel (IK2-10 a); die Moden rechnet `bereite` (der Kern beim Bau)
    return s;
}

TrommelSpiel baueVorgabeConga()
{
    TrommelSpiel s;
    s.name = "Conga";
    s.note = 63;
    s.noteMax = 63;
    // 11¾″ (Haendler); offener Kessel: ohne Luftfeder, innere Luftmasse halb (offen); Spannung offen — keine Messung
    // der Moden mit offenem Kessel gefunden (F-7 a: gebaut, ungemessen)
    Fell f = fell(0.149, 3000.0, 0.4);
    f.innenLuft = 0.5;
    s.felle = {f};
    s.schlagstelleRel = 0.8;
    Stossanreger hand;
    hand.masseKg = 0.3;  // offen
    hand.q0 = 6.0;
    hand.p = 1.5;
    s.schlaegel = hand;
    s.tempoPp = 0.5;
    s.tempoFf = 4.0;
    // der Bezug: kern::normiereNachTafel (IK2-10 a); die Moden rechnet `bereite` (der Kern beim Bau)
    return s;
}

TrommelSpiel baueVorgabeBongo()
{
    TrommelSpiel s = baueVorgabeConga();
    s.name = "Bongo";
    s.note = 60;
    s.noteMax = 60;
    s.felle.front().radiusM = 0.089;  // 7″ (Haendler)
    s.schlaegel.masseKg = 0.15;       // offen
    // der Bezug: kern::normiereNachTafel (IK2-10 a); die Moden rechnet `bereite` (der Kern beim Bau)
    return s;
}

}  // namespace lumi::kern::trommel
