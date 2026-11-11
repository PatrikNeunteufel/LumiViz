/**
 * @file Vorgaben.cpp
 * @brief InstrumentModels, Klasse 10: Vorgaben — Akkordeon (Kern zuerst, F-8 a), Harmonium, Melodica, Mundharmonika (gerade Toene;
 *        Bending ueber den Vokaltrakt nicht gebaut — F-6 a, ungemessen)
 *
 * Belegkraft: *Quelle* (Adachi et al. 2026 Tab. 1, Puranik & Scavone 2023, Hergert, Cottingham — nachgelesen IC
 * Session 2), *nur Bericht* (Bericht `klassen/Klasse 10 - Durchschlagende Zunge/recherche/2026-10-26_physik_rechenverfahren.md`),
 * *Rechnung*, *offen*. Tabelle: `klassen/Klasse 10 - Durchschlagende Zunge/modell.md` (heute
 * `docs/modellierung/zunge/ic/modell.md`).
 *
 * Uebernommen aus IC `rechnung/klassen/10_zunge/Vorgaben.cpp` (`4409088`), die Werte wortgleich; geaendert
 * (Instrumente_IC_Konzept.md 34j-2, Uebernahme-Journal J67): die Vorgaben sind ungestimmt — f₀ und das Fenster des
 * Drucks je Note (IC `Stimmung.inc`) und der Bezug (IC `normiere`) kommen aus `InstrumentTuneTool` (IK2-10 a,
 * IK2-11 a); die Choere ohne Namen (die Namen stehen im Kommentar), ohne `kennzeichen` (IK2-9).
 */
#include "InstrumentModels/zunge/Zunge.hpp"

namespace lumi::kern::zunge
{

ZungenSpiel baueVorgabeAkkordeon()
{
    ZungenSpiel s;
    s.name = "Akkordeon";
    s.noteMin = 29;  // Bass-Seite F1 … E3, Diskant F3 … A6 (41 Tasten; offen)
    s.noteMax = 93;
    s.bassAb = 53;
    // Musette (F-4 a, Hergert, nachgelesen S. 4460): A4 Schwebung 2,00 Hz = 7,9 ct, steigend mit der Hoehe
    s.choere = {{0, 0.0, 1.0}, {0, 2.0, 0.9}};  // M, M+
    s.bassChoere = {{-12, 0.0, 1.0}, {0, 0.0, 0.7}};  // Bass-Register 16′, 8′ (offen)
    // f₀, Fenster und Bezug: InstrumentTuneTool (IK2-10 a, IK2-11 a)
    return s;
}

ZungenSpiel baueVorgabeHarmonium()
{
    ZungenSpiel s;
    s.name = "Harmonium";
    s.noteMin = 48;  // C3 … F6 (indisches Handharmonium, offen)
    s.noteMax = 89;
    // Puranik & Scavone 2023 Tab. 1 (nur Bericht): V₁ = 0,0013 m³, L₂ = 50 mm, S₂ = 6,4·10⁻⁵ m²; Messing (die Masse
    // der C4-Zunge ×8490/7850 — offen); Kanzellendruck 0,2–1,5 kPa „in a real harmonium“ (nachgelesen S. 1810)
    s.kanzelleM3 = 0.0013;
    s.kanalLaengeM = 50e-3;
    s.kanalFlaecheM2 = 6.4e-5;
    s.masseC4Kg *= 8490.0 / 7850.0;
    s.druckPpPa = 200.0;
    s.druckFfPa = 1500.0;
    s.gehaeuseTiefpassHz = 3000.0;  // DAFx23: 10 Biquads (Werte nicht da) — offen
    // f₀, Fenster und Bezug: InstrumentTuneTool (IK2-10 a, IK2-11 a)
    return s;
}

ZungenSpiel baueVorgabeMelodica()
{
    ZungenSpiel s;
    s.name = "Melodica";
    s.noteMin = 53;  // F3 … C6 (32 Tasten, offen)
    s.noteMax = 84;
    s.druckPpPa = 300.0;
    s.druckFfPa = 3000.0;   // Atem (offen)
    s.bendGrenzeCent = 100.0;  // ein Halbton (F-2 a: Ausstattung)
    // f₀, Fenster und Bezug: InstrumentTuneTool (IK2-10 a, IK2-11 a)
    return s;
}

ZungenSpiel baueVorgabeMundharmonika()
{
    ZungenSpiel s;
    s.name = "Mundharmonika";
    s.noteMin = 60;  // diatonisch in C (Richter): Blasen C4 … C7, Ziehen D4 … A6
    s.noteMax = 96;
    s.zeta = 1.0 / (2.0 * 95.0);  // Q = 95 (Millot & Baumann, nur Bericht)
    s.laengeC4M = 20e-3;          // kleiner als am Akkordeon (offen)
    s.masseC4Kg = 2.0 / 3.0 * 0.248e-3 * 20.0 / 32.8;
    s.druckPpPa = 300.0;
    s.druckFfPa = 3000.0;
    s.bendGrenzeCent = 100.0;     // je Kanzelle (F-6 a) — hier fest (offen)
    // f₀, Fenster und Bezug: InstrumentTuneTool (IK2-10 a, IK2-11 a)
    return s;
}

}  // namespace lumi::kern::zunge
