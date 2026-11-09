/**
 * @file Vorgaben.cpp
 * @brief Klasse 9: Vorgaben — Blockfloete (Kern zuerst, F-1 a), Labialpfeife, Shakuhachi; Querfloete und Altfloete als
 *        Geruest ohne Bohrung (T8 Kipppunkt b, F-B2)
 *
 * Belegkraft: *Quelle* (Fletcher & Douglas 1980 S. 767–768, nachgelesen IC Session 1), *nur Bericht* (Bericht
 * `recherche/2026-10-26_physik_spielweisen.md`: Auvray et al. 2015, Terrien et al. 2013/2014, Castellengo 1999/2004,
 * Wikipedia „Normmensur“, NMM), *Rechnung*, *offen*. Tabelle: `klassen/Klasse 9 - Luftstrahl und Rohr/modell.md`.
 */
#include "klassen/09_luftstrahl_rohr/Luftstrahl.hpp"

#include "gemeinsam/Werkzeug.hpp"

#include <cmath>
#include <numbers>

namespace ic::luftstrahl
{

#if __has_include("klassen/09_luftstrahl_rohr/Stimmung.inc")
#include "klassen/09_luftstrahl_rohr/Stimmung.inc"
#define IC_STRAHL_STIMMUNG 1
#endif

namespace
{

void normiere(StrahlSpiel& s)
{
    s.ausgangBezug = 1.0;
    if (!s.spielbar || s.laengeJeNote.punkte.empty()) return;
    StrahlAnschlag a;
    const int n = (s.noteMin + s.noteMax) / 2;
    a.kette = {{0.0, n, 440.0 * std::exp2((n - 69) / 12.0)}};
    a.expression = 1.0;
    a.haltenS = 0.5;
    std::vector<float> ton;
    rechneStrahlTon(s, a, ton, a.rate / 2);
    const double sp = spitze(ton);
    s.ausgangBezug = sp > 0.0 ? sp / 0.5 : 1.0;
}

StrahlSpiel geruest(const std::string& name, int von, int bis, const std::string& grund)
{
    StrahlSpiel s;
    s.name = name;
    s.noteMin = von;
    s.noteMax = bis;
    s.spielbar = false;
    s.grund = grund;
    s.kennzeichen = "nicht spielbar (Bohrung offen)";
    return s;
}

}  // namespace

StrahlSpiel baueVorgabeBlockfloete(bool gestimmt)
{
    StrahlSpiel s;
    s.name = "Blockfloete";
    s.noteMin = 72;  // Sopranblockfloete C5 … C7 (offen)
    s.noteMax = 96;
    // Auvray et al. 2015 (nur Bericht): Labor-Blockfloete W = 4 mm, h = 1 mm, Innendurchmesser 19 mm; Schwellen des
    // Registerwechsels 695 Pa steigend, 534 Pa fallend; der Druck bis rund 700 Pa vor dem Ueberblasen
    s.radiusM = 0.0095;
    s.kanalHoeheM = 1e-3;
    s.abstandM = 4e-3;
    s.mundBreiteM = 0.012;   // offen (Breite des Kanals)
    s.mundFlaecheM2 = s.mundBreiteM * s.abstandM;
    s.druckMinPa = 150.0;    // offen
    s.druckMaxPa = 600.0;    // unter der Schwelle des Registerwechsels 695 Pa (nur Bericht)
    s.kennzeichen = "ungemessen";
    if (!gestimmt) return s;
#ifdef IC_STRAHL_STIMMUNG
    stimmungBlockfloete(s);
#else
    stimme(s);
#endif
    normiere(s);
    return s;
}

StrahlSpiel baueVorgabeLabialpfeife(bool gestimmt)
{
    StrahlSpiel s;
    s.name = "Labialpfeife";
    s.noteMin = 36;  // Prinzipal 8′ C2 … C7 (offen)
    s.noteMax = 96;
    s.orgel = true;
    // die Mensur: Durchmesser 155,5 mm an 8′ C (Wikipedia „Normmensur“, nur Bericht, „pruefbeduerftig“), halbiert alle
    // 16 Halbtoene (Normmensur nach Toepfer — Lehrbuch, offen); Pruefstein Castellengo 2004 (nur Bericht): eine Pfeife
    // 312 mm × 27 mm, Mund 21 × 7 mm
    s.radiusJeNote = kurveAusPunkten({{36, 0.15550 / 2.0}, {100, 0.15550 / 2.0 * std::exp2(-(100 - 36) / 16.0)}}, Belegkraft::Offen,
                                     "Normmensur: 155,5 mm an 8′ C (Wikipedia, nur Bericht), halbiert je 16 Halbtoene (offen)");
    s.radiusM = 0.0135;
    // Mund: Breite ein Viertel des Umfangs (Wikipedia, nur Bericht), Aufschnitt ein Drittel der Breite (Castellengo 21 × 7,
    // nur Bericht), Kernspalt 1 mm (offen) — an der Pfeife der Mitte; die Groessen skalieren mit der Mensur (offen)
    const double d = 2.0 * 0.15550 / 2.0 * std::exp2(-(66 - 36) / 16.0);
    s.mundBreiteM = std::numbers::pi * d / 4.0;
    s.abstandM = s.mundBreiteM / 3.0;
    s.mundFlaecheM2 = s.mundBreiteM * s.abstandM;
    s.mundJeUmfang = 0.25;
    s.kanalHoeheM = 1e-3;
    s.druckMinPa = 785.0;  // Wind 80 mm WS (Lehrbuch, offen) — fest je Werk
    s.druckMaxPa = 785.0;
    s.kennzeichen = "ungemessen (VAO/Palo zum Setzen, BY-SA, lokal)";
    if (!gestimmt) return s;
#ifdef IC_STRAHL_STIMMUNG
    stimmungLabialpfeife(s);
#else
    stimme(s);
#endif
    normiere(s);
    return s;
}

StrahlSpiel baueVorgabeShakuhachi(bool gestimmt)
{
    StrahlSpiel s;
    s.name = "Shakuhachi";
    s.noteMin = 62;  // D4 … (1,8 Shaku) — offen
    s.noteMax = 86;
    // NMM (nur Bericht, Rechte ungeprueft): 603 mm, Bohrung 19,7–22,3 mm, utaguchi 11,4 × 2,8 mm
    s.radiusM = 0.0105;
    s.mundBreiteM = 0.0114;
    s.abstandM = 0.006;      // offen: die offene Kante ohne Kernspalt — der Abstand Lippe–Kante
    s.kanalHoeheM = 1.0e-3;  // offen: Lippenspalt
    s.mundFlaecheM2 = 0.0114 * 0.0028;
    s.druckMinPa = 200.0;
    s.druckMaxPa = 900.0;
    s.rauschPegel = 0.01;    // „well pronounced around 9 kHz“ (Castellengo 2004, nur Bericht) — Pegel offen
    s.kennzeichen = "ungemessen (ein freier Ton)";
    if (!gestimmt) return s;
#ifdef IC_STRAHL_STIMMUNG
    stimmungShakuhachi(s);
#else
    stimme(s);
#endif
    normiere(s);
    return s;
}

StrahlSpiel baueVorgabeQuerfloete()
{
    return geruest("Querfloete", 60, 96, "Bohrung nur bei UNSW ohne Lizenz; Mund nach Terrien (nur Bericht) — F-B2");
}

StrahlSpiel baueVorgabeAltfloete()
{
    return geruest("Altfloete", 55, 86, "keine Geometrie — F-B2");
}

}  // namespace ic::luftstrahl
