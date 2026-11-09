/**
 * @file Vorgaben.cpp
 * @brief InstrumentModels, Kern `hammerString`: Upright, Honky-tonk und CP-70 als Vorgaben auf der Hammer-Saite
 *        (Instrumente_IC_Konzept.md 34i-1, KL2 a)
 *
 * Uebernommen aus IC `rechnung/klassen/01_hammer_saite/VorgabenRest.cpp` (`bb7e6d6`), die Werte und Rechnungen
 * wortgleich (`Tastenkurve` → `Kurve`, `KurvenSkala` → `Skala`, `ic::` → `lumi::kern`). Geaendert: die Tastatur
 * (`tastaturUpright`, `tastaturCp70`) steht nicht hier — die Plaetze der Pedale und `bassSustainBelow` traegt der Block
 * `keyboard` der Datei, die Pedalwerte die Tafel (FU2 a), den Umfang die Tafel (KL4 a); neu die Quellen je Kurve
 * (WZ1 a): eine vom Fluegel uebernommene Kurve behaelt seine Quellen (ihre Zahlen stammen daher), eine neu gerechnete
 * nennt, woraus sie gerechnet ist.
 *
 * Alle drei sind **ungemessen** (K18 und VCSL liegen lokal; CP-70 ohne freie Aufnahme, IC F-6 b: aus Literatur und
 * Modell bauen, nachsetzen, wenn Daten kommen). Was der Fluegel aus *Salamander* hat (Abklingen, Verstimmung, Winkel),
 * bleibt als Vorgabe stehen und traegt in `herkunft` „vom Fluegel uebernommen“ — gesetzt wird an K18 (IC M1–M6).
 */
#include "InstrumentModels/hammer/HammerSaite.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace lumi::kern::hammer
{

namespace
{

/// Yamaha U1: die laengste Basssaite 46,8″ = 1,189 m (IC-Bericht `recherche/…_upright_honkytonk_cimbalom.md`, nur
/// Bericht).
constexpr double kUprightBassM = 46.8 * 0.0254;

void markiere(Kurve& k, Belegkraft b, const std::string& h)
{
    k.belegkraft = b;
    k.herkunft = h;
}

void vomFluegel(HammerSaite& s)
{
    const std::string h = "vom Fluegel uebernommen (Vorbild: an Salamander gemessen bzw. RT-0425/Stulov) — offen fuer dieses "
                          "Instrument, zu setzen an seiner Aufnahme";
    for (Kurve* k : {&s.inharmonizitaet, &s.abklingS, &s.abklingSpaetS, &s.abklingHoch, &s.chor, &s.verstimmungCent,
                     &s.hammerMasseG, &s.hammerSteife, &s.hammerExponent, &s.hammerHysterese, &s.hammerRelaxationUs,
                     &s.anschlagStelle, &s.daempferS, &s.saitenLaengeM, &s.saitenMasseGm, &s.laengsVerhaeltnis,
                     &s.stegWinkelGrad, &s.polarisationWinkelGrad, &s.polarisationCent, &s.waagrechtVerhaeltnis,
                     &s.stossLeise, &s.stossLaut})
        markiere(*k, Belegkraft::Offen, h);
}

/// Eine Kurve neu an jeder Taste aus einer Formel (alle Tasten als Stuetzpunkte, damit die Interpolation exakt ist).
template <class F>
Kurve anJederTaste(F formel)
{
    Kurve k;
    for (int t = kTasteMin; t <= kTasteMax; ++t) k.punkte.push_back({t, formel(t)});
    return k;
}

/// Die Quellen einer Kurve, die aus anderen gerechnet ist: deren Quellen und die neuen, ohne doppelte.
std::vector<std::string> zusammen(std::vector<std::string> a, std::initializer_list<const char*> dazu)
{
    for (const char* q : dazu)
        if (std::find(a.begin(), a.end(), q) == a.end()) a.emplace_back(q);
    return a;
}

}  // namespace

HammerSaite baueVorgabeUpright()
{
    HammerSaite s = hammerSaiteVorgabe();
    vomFluegel(s);
    const HammerSaite& f = hammerSaiteVorgabe();
    // die Laenge: gekappt auf die Basssaite des U1; bei gleicher Spannung T = 4L²f0²µ wird µ ∝ 1/L², B ∝ 1/L²
    const auto laenge = [&](int t) { return std::min(wertAn(f.saitenLaengeM, t, Skala::Logarithmisch), kUprightBassM); };
    const auto faktor = [&](int t) {
        const double lf = wertAn(f.saitenLaengeM, t, Skala::Logarithmisch);
        return std::pow(lf / laenge(t), 2.0);
    };
    s.saitenLaengeM = anJederTaste(laenge);
    markiere(s.saitenLaengeM, Belegkraft::Offen,
             "Fluegel (RT-0425) gekappt auf 1,189 m (Yamaha U1 46,8″, nur Bericht) — keine Mensur eines Upright frei (F-7 a: "
             "aus Basslaenge und gemessenem B an K18 zurueckrechnen, lokal)");
    s.saitenLaengeM.quellen = zusammen(f.saitenLaengeM.quellen, {"yamaha-u1"});
    s.saitenMasseGm = anJederTaste([&](int t) { return std::clamp(wertAn(f.saitenMasseGm, t, Skala::Logarithmisch) * faktor(t), 0.5, 500.0); });
    markiere(s.saitenMasseGm, Belegkraft::Rechnung, "gleiche Spannung wie der Fluegel bei kuerzerer Saite: µ·(L_F/L_U)² (Rechnung, offen bis K18)");
    s.saitenMasseGm.quellen = zusammen(f.saitenMasseGm.quellen, {"yamaha-u1"});
    s.inharmonizitaet = anJederTaste([&](int t) { return std::clamp(wertAn(f.inharmonizitaet, t, Skala::Logarithmisch) * faktor(t), 1e-6, 0.1); });
    markiere(s.inharmonizitaet, Belegkraft::Offen,
             "B des Fluegels mal (L_F/L_U)² (Bericht: „bei 0,6-facher Laenge … etwa das 2,8-fache“, Einschaetzung); M1 an K18");
    s.inharmonizitaet.quellen = zusammen(f.inharmonizitaet.quellen, {"yamaha-u1"});
    // die Anschlagstelle 1/7 … 1/9 der Laenge (Patent, nur Bericht) — die Mitte 1/8
    s.anschlagStelle = kurveKonstant(0.125, Belegkraft::Offen, "1/8 (Patent: 1/7 … 1/9, nur Bericht); M4", kTasteMin, kTasteMax);
    s.anschlagStelle.quellen = {"lumiviz"};  // das Patent ist im Bericht nicht genannt — eigene Vorgabe in der Spanne
    return s;
}

HammerSaite baueVorgabeHonkytonk()
{
    HammerSaite s = baueVorgabeUpright();
    const HammerSaite& f = hammerSaiteVorgabe();
    // ±1 Hz bei A4 zwischen den aeusseren Saiten 2 Hz: 1200·log2(441/439) = 7,87 Cent; nur die blanken Saiten (ab dem
    // Bassteg MIDI 41, wo die Laenge des Fluegels springt — RT-0425); fest in Cent ueber den Umfang (offen)
    const double cent = 1200.0 * std::log2(441.0 / 439.0);
    s.verstimmungCent = anJederTaste([&](int t) { return t >= 41 ? cent : wertAn(f.verstimmungCent, t, Skala::Linear); });
    markiere(s.verstimmungCent, Belegkraft::Offen,
             "±1 Hz bei A4 (Stimmerpraxis, Forum, nur Bericht) = 7,87 Cent ab MIDI 41 (blanke Saiten), darunter wie das Upright; "
             "M3 an FreePats „Old Piano FB“ (lokal)");
    s.verstimmungCent.quellen = zusammen(f.verstimmungCent.quellen, {"ptg-honkytonk"});
    return s;
}

HammerSaite baueVorgabeCp70()
{
    HammerSaite s = hammerSaiteVorgabe();
    vomFluegel(s);
    s.chor = kurveAusPunkten({{21, 1.0}, {42, 1.0}, {43, 2.0}, {108, 2.0}}, Belegkraft::Offen,
                             "eine Saite bis MIDI 42, darueber zwei (smem, Modartt — nur Bericht)");
    s.chor.quellen = {"smem-cp70", "modartt-cp80"};
    return s;
}

}  // namespace lumi::kern::hammer
