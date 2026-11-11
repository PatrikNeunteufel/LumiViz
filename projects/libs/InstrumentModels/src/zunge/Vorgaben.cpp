/**
 * @file Vorgaben.cpp
 * @brief InstrumentModels, Klasse 10: Vorgaben — Akkordeon (Kern zuerst, F-8 a), Harmonium, Melodica, Mundharmonika (gerade Toene;
 *        Bending ueber den Vokaltrakt nicht gebaut — F-6 a, ungemessen)
 *
 * **Rechenfassung 2** (34j-6, S191): die Groessen je Taste als Gesetze ueber den Umfang — Wert an C4 mal
 * (f / f_C4)^Exponent, je Abschnitt; hergeleitet und am Pruefstand gegen alle Zielgroessen gerechnet in
 * `docs/modellierung/zunge/2026-11-11_S190_vorgaben_je_taste.md` §3–§6 (Entscheide ZV4 a–ZV7 a, ZH1 a–ZH3 a), die Saetze
 * dort: Akkordeon D3 mit beschwerten Basszungen, Harmonium mit ζ ∝ 1/f, Mundharmonika nach Millot & Baumann, Melodica
 * wie D3. Belegkraft je Kurve nach ZH1 a: *gemessen* in einer Quelle → `Quelle`, *abgeleitet* → `Rechnung`, *Annahme*
 * → `Offen`; eine Kurve mit gemessenem Wert an C4 und abgeleitetem Exponenten ist `Rechnung`.
 *
 * Die Stuetzpunkte: je Abschnitt seine Enden und deren Nachbarn — dann liest die monotone kubische Kurve
 * (logarithmisch) an jeder Taste das Gesetz, auch an einem Knick zwischen zwei Abschnitten (die Steigung an einem
 * inneren Punkt ist das harmonische Mittel der Sekanten links und rechts, beide aus demselben Gesetz).
 *
 * Bis Rechenfassung 1 (IC `rechnung/klassen/10_zunge/Vorgaben.cpp`, `4409088`, J67): nur die Laenge gestuft (∝ 1/√f),
 * alles andere fest an der C4-Zunge nach Adachi et al. 2026 Tab. 1; das Fenster des Drucks je Note gestimmt (Befund
 * S188).
 */
#include "InstrumentModels/zunge/Zunge.hpp"

#include <algorithm>
#include <cmath>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

namespace lumi::kern::zunge
{

namespace
{

double hzVon(int note)
{
    return 440.0 * std::exp2((static_cast<double>(note) - 69.0) / 12.0);
}

/// Ein Abschnitt eines Gesetzes: von … bis (MIDI, die Tonhoehe der Zunge), Wert an C4, Exponent.
struct Abschnitt
{
    int von = 0, bis = 0;
    double c4 = 0.0, hoch = 0.0;
};

Kurve gesetz(std::initializer_list<Abschnitt> abschnitte, Belegkraft b, std::string herkunft, std::vector<std::string> quellen)
{
    Kurve k;
    for (const Abschnitt& a : abschnitte)
        for (const int n : {a.von, a.von + 1, a.bis - 1, a.bis})
            if (n >= a.von && n <= a.bis && std::none_of(k.punkte.begin(), k.punkte.end(), [&](const NotenWert& p) { return p.note == n; }))
                k.punkte.push_back({n, a.c4 * std::pow(hzVon(n) / hzVon(60), a.hoch)});
    std::sort(k.punkte.begin(), k.punkte.end(), [](const NotenWert& x, const NotenWert& y) { return x.note < y.note; });
    k.belegkraft = b;
    k.herkunft = std::move(herkunft);
    k.quellen = std::move(quellen);
    return k;
}

/// Ein Gesetz ueber den ganzen Umfang eines Spiels.
Kurve gesetz(const ZungenSpiel& s, double c4, double hoch, Belegkraft b, std::string herkunft, std::vector<std::string> quellen)
{
    return gesetz({{s.noteMin, s.noteMax, c4, hoch}}, b, std::move(herkunft), std::move(quellen));
}

const std::string kHerleitung = " (S190 34j-5, docs/modellierung/zunge/2026-11-11_S190_vorgaben_je_taste.md)";

/// Die Gesetze des Akkordeon-Diskants D3 (§3) auf dem Umfang von `s`; die Melodica traegt sie als Annahme (ZH2 a).
void diskantD3(ZungenSpiel& s, const std::string& wer)
{
    s.laengeM = gesetz(s, 32.8e-3, -0.42, Belegkraft::Rechnung,
                       wer + "length 32.8 mm at C4 (Adachi et al. 2026 Tab. 1), exponent -0.42 through the reeds 42 mm / 142.7 Hz and "
                             "37 mm / 226.4 Hz (Llanos-Vazquez et al. 2014 Tabla 3)" + kHerleitung,
                       {"adachi2026", "llanosvazquez2014"});
    s.breiteM = gesetz(s, 3.45e-3, -0.2, Belegkraft::Rechnung,
                       wer + "width 3.45 mm at C4 (Adachi et al. 2026 Tab. 1), exponent -0.2 through a bass reed 60 x 6 mm (forum "
                             "accordionists.info, low weight)" + kHerleitung,
                       {"adachi2026", "accordionists-info"});
    s.flaechenmasse = gesetz(s, 2.92, -0.7, Belegkraft::Rechnung,
                             wer + "mass per area 2.92 kg/m2 at C4 ((2/3) x 0.248 g over half of 32.8 x 3.45 mm, Adachi et al. 2026 Tab. 1), "
                                   "exponent -0.7 derived from the threshold: closing pressure = mass per area x w0^2 x rest opening" +
                                 kHerleitung,
                             {"adachi2026", "fletcher1993"});
    s.ruheM = gesetz(s, 0.3e-3, -0.7, Belegkraft::Rechnung,
                     wer + "rest opening 0.30 mm at C4 (Adachi et al. 2026 Tab. 1), exponent -0.7 derived — keeps the opening about the "
                           "reed thickness (workshop rule, report only)" + kHerleitung,
                     {"adachi2026"});
    s.seitenspaltM = gesetz(s, 0.14e-3, -0.7, Belegkraft::Rechnung,
                            wer + "side gap 0.14 mm at C4 (Adachi et al. 2026 Tab. 1), exponent -0.7 derived" + kHerleitung, {"adachi2026"});
    s.zeta = gesetz(s, 0.003, 0.0, Belegkraft::Quelle, wer + "damping ratio 0.003 (Adachi et al. 2026 Tab. 1)", {"adachi2026"});
    s.kanalLaengeM = gesetz(s, 60e-3, -0.42, Belegkraft::Rechnung,
                            wer + "channel length 60 mm at C4, with the reed length; derived from the build-up time at A4, 60-90 ms "
                                  "(Llanos-Vazquez et al. 2014)" + kHerleitung,
                            {"llanosvazquez2014"});
    s.kanalFlaecheM2 = gesetz(s, 3.45e-3 * 2e-3, 0.0, Belegkraft::Offen, wer + "channel area 3.45 x 2 mm2 (open, IC)", {"lumiviz"});
    s.ventilFlaecheM2 = gesetz(s, 6e-6, -0.9, Belegkraft::Rechnung,
                               wer + "pallet 6 mm2 at C4, exponent -0.9; derived from the pitch over the pressure at C4, -16 cent/kPa "
                                     "measured (Adachi et al. 2026 Fig. 5) — gives -11 (ZV5 a)" + kHerleitung,
                               {"adachi2026"});
    s.kanzelleM3 = gesetz(s, 1e-6, -0.9, Belegkraft::Rechnung, wer + "chamber 1 cm3 at C4, exponent -0.9, derived with the pallet" + kHerleitung,
                          {"adachi2026"});
}

}  // namespace

ZungenSpiel baueVorgabeAkkordeon()
{
    ZungenSpiel s;
    s.name = "Akkordeon";
    s.noteMin = 29;  // Bass-Seite F1 … E3, Diskant F3 … A6 (41 Tasten; offen)
    s.noteMax = 93;
    s.bassAb = 53;
    diskantD3(s, "accordion F1-A6 (treble D3; the bass side below F3 with the same law): ");
    // die Bass-Seite (§4): beschwerte Zungen unter F3 (Llanos-Vazquez et al. 2014: (f/f')² = 0,98 + 3,7·M/m, gemessen) —
    // Masse je Flaeche ∝ f^−1,2, Aufbiegung ∝ f^−0,8, Seitenspalt ∝ f^−0,4; Laenge, Breite, Kanal, Ventil wie der Diskant
    constexpr int kF3 = 53;
    s.flaechenmasse = gesetz({{s.noteMin, kF3 - 1, 2.388, -1.2}, {kF3, s.noteMax, 2.92, -0.7}}, Belegkraft::Rechnung,
                             "accordion: treble D3 from F3, 2.92 kg/m2 at C4 ((2/3) x 0.248 g, Adachi et al. 2026 Tab. 1), exponent -0.7 "
                             "derived from the threshold; bass reeds below F3 loaded (Llanos-Vazquez et al. 2014: (f/f')^2 = 0.98 + "
                             "3.7 M/m, measured): exponent -1.2, continuous at F3" + kHerleitung,
                             {"adachi2026", "llanosvazquez2014", "fletcher1993"});
    s.ruheM = gesetz({{s.noteMin, kF3 - 1, 0.288e-3, -0.8}, {kF3, s.noteMax, 0.3e-3, -0.7}}, Belegkraft::Rechnung,
                     "accordion: treble 0.30 mm at C4 (Adachi et al. 2026 Tab. 1), exponent -0.7; below F3 exponent -0.8, continuous "
                     "at F3 (derived)" + kHerleitung,
                     {"adachi2026"});
    s.seitenspaltM = gesetz({{s.noteMin, kF3 - 1, 0.14e-3, -0.4}, {kF3, s.noteMax, 0.14e-3, -0.7}}, Belegkraft::Rechnung,
                            "accordion: treble 0.14 mm at C4 (Adachi et al. 2026 Tab. 1), exponent -0.7; below F3 exponent -0.4 "
                            "(derived; 0.165 mm against 0.186 mm at F3)" + kHerleitung,
                            {"adachi2026"});
    s.windMinPa = 200.0;  // Annahme (Balgdruck nirgends gemessen; Laborachse bei Adachi 0–2,5 kPa)
    s.windMaxPa = 1600.0;
    // Musette (F-4 a, Hergert, nachgelesen S. 4460): A4 Schwebung 2,00 Hz = 7,9 ct, steigend mit der Hoehe
    s.choere = {{0, 0.0, 1.0}, {0, 2.0, 0.9}};  // M, M+
    // Bass-Seite 8′ und 4′ (ZV6 a): der 16′ von F1 laege bei 21,8 Hz, die tiefste echte Zunge um E1
    s.bassChoere = {{0, 0.0, 1.0}, {12, 0.0, 0.7}};
    s.bendUeberVentil = true;  // ZV7 a: die halb gedrueckte Taste
    // f₀ und Bezug: InstrumentTuneTool (IK2-10 a, IK2-11 a)
    return s;
}

ZungenSpiel baueVorgabeHarmonium()
{
    ZungenSpiel s;
    s.name = "Harmonium";
    s.noteMin = 48;  // C3 … F6 (indisches Handharmonium, offen)
    s.noteMax = 89;
    const std::string wer = "harmonium (C3-F6), laws through the measured reeds C3 (Cottingham 1998: 150 N/m, 0.21 g, 4 mm wide, about "
                            "4 cm, zeta 0.006) and B5 (Puranik & Scavone 2023: 16 x 2 x 0.4 mm brass, rest 0.1 mm): ";
    s.laengeM = gesetz(s, 29.6e-3, -0.463, Belegkraft::Rechnung, wer + "length 29.6 mm at C4, exponent -0.463" + kHerleitung,
                       {"cottingham1998", "puranik2023"});
    s.breiteM = gesetz(s, 3.19e-3, -0.35, Belegkraft::Rechnung, wer + "width 3.19 mm at C4, exponent -0.35 (4 mm, 2 mm)" + kHerleitung,
                       {"cottingham1998", "puranik2023"});
    s.flaechenmasse = gesetz(s, 5.81, -0.187, Belegkraft::Rechnung,
                             wer + "mass per area 5.81 kg/m2 at C4, exponent -0.187 (C3 M/S_r 6.56, B5 (4/3) rho h 4.53)" + kHerleitung,
                             {"cottingham1998", "puranik2023"});
    s.ruheM = gesetz(s, 0.296e-3, -0.81, Belegkraft::Rechnung,
                     wer + "rest opening 0.296 mm at C4, exponent -0.81: B5 0.1 mm measured, C3 0.5 mm derived from threshold and pitch "
                           "minimum (Cottingham, Reed, Busha 1999 Fig. 3)" + kHerleitung,
                     {"puranik2023", "cottingham-reed-busha1999"});
    s.seitenspaltM = gesetz(s, 0.1e-3, -0.5, Belegkraft::Offen, wer + "side gap 0.10 mm at C4, exponent -0.5 (assumption)" + kHerleitung,
                            {"lumiviz"});
    s.zeta = gesetz(s, 0.00314, -1.0, Belegkraft::Offen,
                    wer + "damping 0.006 at C3 measured (Cottingham 1998); falling as 1/f is an assumption — with 0.006 for all keys the "
                          "thick brass reed B5 cannot speak below 2 zeta x closing pressure, about 210 Pa, but sounds at 0.2 kPa (Puranik & "
                          "Scavone 2023)" + kHerleitung,
                    {"cottingham1998", "puranik2023"});
    s.kanalLaengeM = gesetz(s, 120e-3, -0.42, Belegkraft::Rechnung, wer + "channel length 120 mm at C4, derived from the build-up" + kHerleitung,
                            {"lumiviz"});
    s.kanalFlaecheM2 = gesetz(s, 3.45e-3 * 2e-3, 0.0, Belegkraft::Offen, wer + "channel area 3.45 x 2 mm2 (open)", {"lumiviz"});
    s.ventilFlaecheM2 = gesetz(s, 4e-6, -0.6, Belegkraft::Rechnung,
                               wer + "pallet 4 mm2 at C4, exponent -0.6; derived from the pitch over the pressure at C3, about -24 cent/kPa "
                                     "measured (Cottingham, Reed, Busha 1999)" + kHerleitung,
                               {"cottingham-reed-busha1999"});
    s.kanzelleM3 = gesetz(s, 1e-6, -0.9, Belegkraft::Rechnung, wer + "chamber 1 cm3 at C4, exponent -0.9, derived with the pallet" + kHerleitung,
                          {"lumiviz"});
    // Kanzellendruck 0,2–1,5 kPa „in a real harmonium“ (Puranik & Scavone 2023, nachgelesen S. 1810)
    s.windMinPa = 200.0;
    s.windMaxPa = 1500.0;
    s.gehaeuseTiefpassHz = 3000.0;  // DAFx23: 10 Biquads (Werte nicht da) — offen
    // f₀ und Bezug: InstrumentTuneTool (IK2-10 a, IK2-11 a)
    return s;
}

ZungenSpiel baueVorgabeMelodica()
{
    ZungenSpiel s;
    s.name = "Melodica";
    s.noteMin = 53;  // F3 … C6 (32 Tasten, offen)
    s.noteMax = 84;
    // ZH2 a: die Gesetze des Akkordeons D3, ein Chor, Atemdruck wie der Balg — alles Annahme, „ungemessen“
    diskantD3(s, "melodica (F3-C6) as the accordion treble D3 (ZH2 a, assumption, unmeasured): ");
    s.windMinPa = 200.0;
    s.windMaxPa = 1600.0;
    s.bendUeberVentil = true;  // ZV7 a
    // f₀ und Bezug: InstrumentTuneTool (IK2-10 a, IK2-11 a)
    return s;
}

ZungenSpiel baueVorgabeMundharmonika()
{
    ZungenSpiel s;
    s.name = "Mundharmonika";
    s.noteMin = 60;  // diatonisch in C (Richter): Blasen C4 … C7, Ziehen D4 … A6
    s.noteMax = 96;
    const std::string wer = "harmonica (C4-C7), anchor the reed of Millot & Baumann 2007 (Lee Oskar, 444 Hz: 12.95 x 2.1 mm, 47.9 N/m, "
                            "rest 0.528 mm, gap 50 um, Q = 95, reed 110 um, plate 900 um — Tab. 2 p. 14): ";
    s.laengeM = gesetz(s, 16.2e-3, -0.427, Belegkraft::Rechnung,
                       wer + "length 16.2 mm at C4, exponent -0.427 through the slot lengths 17 ... 7 mm (Harp-L, D. Payne 2009, low "
                             "weight); gives 13.6 mm at 444 Hz against 12.95 measured" + kHerleitung,
                       {"millot2007", "harp-l-seydel"});
    s.breiteM = gesetz(s, 2.1e-3, 0.0, Belegkraft::Quelle, wer + "width 2.1 mm over the range", {"millot2007"});
    s.flaechenmasse = gesetz(s, 0.656, -0.7, Belegkraft::Rechnung, wer + "mass per area 0.656 kg/m2 at C4, exponent -0.7" + kHerleitung,
                             {"millot2007"});
    s.ruheM = gesetz(s, 0.765e-3, -0.7, Belegkraft::Rechnung,
                     wer + "rest opening 0.765 mm at C4, exponent -0.7 (rest plus half the thickness, 0.583 mm, at 444 Hz); the "
                           "amplitude lies at 25-35 % of the length at C4 — workshops gap 0.15-0.2 mm (forums), not taken" + kHerleitung,
                     {"millot2007"});
    s.seitenspaltM = gesetz(s, 0.05e-3, 0.0, Belegkraft::Quelle, wer + "side gap 50 um", {"millot2007"});
    s.zeta = gesetz(s, 1.0 / (2.0 * 95.0), 0.0, Belegkraft::Quelle, wer + "Q = 95", {"millot2007"});
    s.kanalLaengeM = gesetz(s, 60e-3, -0.42, Belegkraft::Rechnung, wer + "channel 60 mm at C4 with the length, derived" + kHerleitung,
                            {"lumiviz"});
    s.kanalFlaecheM2 = gesetz(s, 3.45e-3 * 2e-3, 0.0, Belegkraft::Offen, wer + "channel area 3.45 x 2 mm2 (open)", {"lumiviz"});
    s.ventilFlaecheM2 = gesetz(s, 3e-4, 0.0, Belegkraft::Offen, wer + "the mouth channel open, no pallet (3 cm2)", {"lumiviz"});
    s.kanzelleM3 = gesetz(s, 14.8e-3 * 14.6e-3 * 20e-3, 0.0, Belegkraft::Offen, wer + "chamber 4.3 cm3 (open, IC)", {"lumiviz"});
    // Atemdruck 150–1 000 Pa — Annahme (Millot rechnet mit 570 und 783 Pa; die gemessene Zunge erstickt ueber 1,9 kPa)
    s.windMinPa = 150.0;
    s.windMaxPa = 1000.0;
    // f₀ und Bezug: InstrumentTuneTool (IK2-10 a, IK2-11 a)
    return s;
}

}  // namespace lumi::kern::zunge
