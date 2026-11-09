/**
 * @file Vorgaben.cpp
 * @brief Klasse 10: Vorgaben — Akkordeon (Kern zuerst, F-8 a), Harmonium, Melodica, Mundharmonika (gerade Toene;
 *        Bending ueber den Vokaltrakt nicht gebaut — F-6 a, ungemessen)
 *
 * Belegkraft: *Quelle* (Adachi et al. 2026 Tab. 1, Puranik & Scavone 2023, Hergert, Cottingham — nachgelesen IC
 * Session 2), *nur Bericht* (Bericht `klassen/Klasse 10 - Durchschlagende Zunge/recherche/2026-10-26_physik_rechenverfahren.md`),
 * *Rechnung*, *offen*. Tabelle: `klassen/Klasse 10 - Durchschlagende Zunge/modell.md`.
 */
#include "klassen/10_zunge/Zunge.hpp"

#include "gemeinsam/Werkzeug.hpp"

#include <cmath>

namespace ic::zunge
{

#if __has_include("klassen/10_zunge/Stimmung.inc")
#include "klassen/10_zunge/Stimmung.inc"
#define IC_ZUNGE_STIMMUNG 1
#endif

namespace
{

void normiere(ZungenSpiel& s)
{
    s.ausgangBezug = 1.0;
    ZungenAnschlag a;
    a.note = (std::max(s.noteMin, s.bassAb) + s.noteMax) / 2;
    a.expression = 1.0;
    a.haltenS = 0.6;
    std::vector<float> ton;
    rechneZungenTon(s, a, ton, static_cast<std::int64_t>(0.6 * a.rate));
    const double sp = spitze(ton);
    s.ausgangBezug = sp > 0.0 ? sp / 0.5 : 1.0;
}

}  // namespace

ZungenSpiel baueVorgabeAkkordeon(bool gestimmt)
{
    ZungenSpiel s;
    s.name = "Akkordeon";
    s.noteMin = 29;  // Bass-Seite F1 … E3, Diskant F3 … A6 (41 Tasten; offen)
    s.noteMax = 93;
    s.bassAb = 53;
    // Musette (F-4 a, Hergert, nachgelesen S. 4460): A4 Schwebung 2,00 Hz = 7,9 ct, steigend mit der Hoehe
    s.choere = {{"M", 0, 0.0, 1.0}, {"M+", 0, 2.0, 0.9}};
    s.bassChoere = {{"16′", -12, 0.0, 1.0}, {"8′", 0, 0.0, 0.7}};  // Bass-Register (offen)
    s.kennzeichen = "ungemessen (TinySOL lokal)";
    if (!gestimmt) return s;
#ifdef IC_ZUNGE_STIMMUNG
    stimmungAkkordeon(s);
#else
    stimme(s);
#endif
    normiere(s);
    return s;
}

ZungenSpiel baueVorgabeHarmonium(bool gestimmt)
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
    s.kennzeichen = "ungemessen (Euterpea CC0 lokal; Gehaeuse offen)";
    if (!gestimmt) return s;
#ifdef IC_ZUNGE_STIMMUNG
    stimmungHarmonium(s);
#else
    stimme(s);
#endif
    normiere(s);
    return s;
}

ZungenSpiel baueVorgabeMelodica(bool gestimmt)
{
    ZungenSpiel s;
    s.name = "Melodica";
    s.noteMin = 53;  // F3 … C6 (32 Tasten, offen)
    s.noteMax = 84;
    s.druckPpPa = 300.0;
    s.druckFfPa = 3000.0;   // Atem (offen)
    s.bendGrenzeCent = 100.0;  // ein Halbton (F-2 a: Ausstattung)
    s.kennzeichen = "ungemessen (fese CC BY 4.0 lokal)";
    if (!gestimmt) return s;
#ifdef IC_ZUNGE_STIMMUNG
    stimmungMelodica(s);
#else
    stimme(s);
#endif
    normiere(s);
    return s;
}

ZungenSpiel baueVorgabeMundharmonika(bool gestimmt)
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
    s.kennzeichen = "ungemessen (gerade Toene; Bending ueber den Vokaltrakt nicht gebaut — F-6 a)";
    if (!gestimmt) return s;
#ifdef IC_ZUNGE_STIMMUNG
    stimmungMundharmonika(s);
#else
    stimme(s);
#endif
    normiere(s);
    return s;
}

}  // namespace ic::zunge
