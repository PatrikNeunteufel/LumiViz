/**
 * @file Vorgaben.cpp
 * @brief Klasse 6: Vorgaben — Tamtam und Gong (Kern zuerst, F-1 a), Crash, China, Splash, Ride, Hi-Hat; alle als
 *        **flache** Platte (Kuppe, Woelbung, Verjuengung nicht gebaut — F-5 a: an Iowa zu setzen, lokal) und ungemessen
 *
 * Belegkraft: *nur Bericht* (B20 8860 kg/m³, 110 GPa ohne Herkunft — Sekiguchi & Samejima; Daempfung Humbert; Kontakt
 * DAFx-15 100–300 N, 0,8–1 ms), *Haendler* (Durchmesser), *offen* (Dicken, Schlaegel). Tabelle:
 * `klassen/Klasse 6 - Becken und Gong/modell.md`.
 */
#include "klassen/06_becken_gong/Becken.hpp"

#include "gemeinsam/Werkzeug.hpp"

namespace ic::becken
{

namespace
{

void normiere(BeckenSpiel& s)
{
    s.ausgangBezug = 1.0;
    bereite(s);
    BeckenAnschlag a;
    a.staerke = 1.0;
    std::vector<float> ton;
    rechneBeckenTon(s, a, ton, a.rate / 8);  // 125 ms: die Spitze liegt im Schlag
    const double sp = spitze(ton);
    s.ausgangBezug = sp > 0.0 ? sp / 0.5 : 1.0;
}

BeckenSpiel becken(const std::string& name, int note, double zoll, double dickeMm, double bisHz)
{
    BeckenSpiel s;
    s.name = name;
    s.note = note;
    s.radiusM = 0.5 * zoll * 0.0254;
    s.dickeM = dickeMm * 1e-3;
    s.bisHz = bisHz;
    s.spannungsModen = 50;
    // der Stock (offen): an DAFx-15 (Modellwerte 100–300 N, 0,8–1 ms — nur Bericht) gesetzt
    s.schlaegelMasseKg = 0.03;
    s.schlaegelQ0 = 2e7;
    s.schlaegelP = 1.5;
    s.tempoPp = 1.0;
    s.tempoFf = 8.0;
    s.kennzeichen = "ungemessen (flache Platte; Form und Material offen)";
    return s;
}

}  // namespace

BeckenSpiel baueVorgabeTamtam()
{
    BeckenSpiel s = becken("Tamtam", 52, 28.0, 1.6, 1200.0);  // Iowa 28″ (Haendler); Dicke offen
    s.schlaegelMasseKg = 0.25;  // Filzschlaegel (offen)
    s.schlaegelQ0 = 2e5;
    s.tempoPp = 0.5;
    s.tempoFf = 4.0;
    s.kennzeichen = "ungemessen (Iowa 22/28/40″ reflexionsarm lokal)";
    normiere(s);
    return s;
}

BeckenSpiel baueVorgabeGong()
{
    BeckenSpiel s = becken("Gong", 52, 17.0, 2.0, 2000.0);  // ohne Kuppe (offen)
    s.schlaegelMasseKg = 0.15;
    s.schlaegelQ0 = 5e5;
    s.tempoPp = 0.5;
    s.tempoFf = 4.0;
    s.kennzeichen = "ungemessen (ohne Kuppe; VSCO 17 Staerken, Beijing-Opera-Gongs lokal)";
    normiere(s);
    return s;
}

BeckenSpiel baueVorgabeCrash()
{
    BeckenSpiel s = becken("Crash", 49, 18.0, 0.9, 2500.0);
    normiere(s);
    return s;
}

BeckenSpiel baueVorgabeChina()
{
    BeckenSpiel s = becken("China", 52, 18.0, 0.8, 2500.0);  // die umgebogene Krempe fehlt (offen)
    normiere(s);
    return s;
}

BeckenSpiel baueVorgabeSplash()
{
    BeckenSpiel s = becken("Splash", 55, 10.0, 0.7, 3000.0);
    normiere(s);
    return s;
}

BeckenSpiel baueVorgabeRide()
{
    BeckenSpiel s = becken("Ride", 51, 20.0, 1.3, 2500.0);
    normiere(s);
    return s;
}

BeckenSpiel baueVorgabeHihat()
{
    BeckenSpiel s = becken("Hi-Hat", 42, 14.0, 1.0, 3000.0);
    s.hihat = true;
    s.kennzeichen = "ungemessen (Kontakt der Schalen nicht gebaut; Oeffnung als Daempfung)";
    normiere(s);
    return s;
}

}  // namespace ic::becken
