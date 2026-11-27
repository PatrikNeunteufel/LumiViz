/**
 * @file Vorgaben.cpp
 * @brief InstrumentModels, Kern `pluckedString` Rechenfassung 2: die Vorgaben der fuenf Instrumente (34k-7d-4 (c); die Saiten
 *        aus `zupf/Saiten`, der Zupf nach GZ6 a, GZ10 a, GZ13 a, KQ30 a, ZW1 a, ZD1 a, ZD1-1 a, ZD2 a, das Notenende nach ZN1 a)
 *
 * Dieselben Zahlen wie der Bezug `werte/zupf_pruefstand.txt` (`proben/2026-11-26_S222_bezug_zupf.py`, `eingaben`). Die
 * Fassung aus IC (Rechenfassung 1, Uebernahme-Journal J77) stand hier bis S224 (ZB1 a). Ungestimmt: die Tafel des
 * Umlauf-Filters und der Bezug kommen aus `InstrumentTuneTool` (ZK1 a, IK2-10 a).
 */
#include "InstrumentModels/zupf/Zupf.hpp"

#include <cmath>

namespace lumi::kern::zupf
{

namespace
{

/// Gitarre, Laute, Oud: der Umfang aus dem Satz (die tiefste leere Saite bis zum hoechsten Bund der hoechsten).
void umfangAusSatz(ZupfSpiel& s)
{
    s.noteMin = s.satz.choere.front().leer;
    s.noteMax = s.satz.choere.back().leer + s.satz.buende;
}

/// ZW1 a: 16 cm vom Steg, fuer alle Buende gleich.
void zupfortFest(ZupfSpiel& s)
{
    s.zupfort = kurveKonstant(0.16, Belegkraft::Quelle,
                              "16 cm vom Steg fuer alle Buende (ZW1 a; Traube 2004 Abschn. 3.1, 3.3; Scherrer 2013 Abb. 3.7, Tab. 2.1)",
                              s.noteMin, s.noteMax);
}

}  // namespace

ZupfSpiel baueVorgabeCembalo()
{
    ZupfSpiel s;
    s.name = "harpsichord";
    s.noteMin = 29;  // FF … f³ (Taskin 1769)
    s.noteMax = 89;
    setzeSaitenJeTaste(s, [](int t) { return std::optional<Tastensaite>(cembaloSaite(t)); }, Belegkraft::Quelle,
                       "ein 8′ je Taste: Mensur und Zupfpunkt des Taskin 1769 an elf Tasten, dazwischen logarithmisch; Draht nach Di Veroli "
                       "(Tab. 7, 8; Dichte und E Annahme); Verlust: Luft nach Stokes, Biegung η = 0,001 (Annahme) — KQ2 a, GZ12 a");
    s.kraftGegeben = true;
    std::vector<NotenWert> kraft;
    for (int t = s.noteMin; t <= s.noteMax; ++t) kraft.push_back({t, 1.07 * std::pow(10.0, -2.8 / 20.0 * (t - 60) / 12.0)});
    s.kraft = Kurve{kraft, {}, Belegkraft::Quelle,
                    "1,07 N an C4 bei ff, −2,8 dB je Oktave (KQ30 a: Chadefaux u. a. 2013 Tab. 1/2 S. 158 an G4; Form GZ13 a angepasst an "
                    "fuenf VCSL-Cembali)",
                    {}};
    s.pegelJeStufe = 1.0 / 18.0;  // GZ10 a: bis pp (Stufe −18) 1 dB weniger
    s.neigungJeStufe = 0.0;
    s.winkelGrad = 20.0;
    s.breite = 0.003;
    s.tempo = 0.0;
    s.daempfenS = 0.06;
    return s;
}

ZupfSpiel baueVorgabeHarfe()
{
    ZupfSpiel s;
    s.name = "concert_harp";
    s.noteMin = 25;  // die Tasten mit Stegpunkt (KQ22 a)
    s.noteMax = 104;
    setzeSaitenJeTaste(s, [](int t) { return harfenSaite(t); }, Belegkraft::Quelle,
                       "46 Saiten nach Bell 1988 Tab. 2.1 / 2.2 (Nylon, Darm, Metall; Kupfer als voller Ring Annahme), je Taste ueber die "
                       "Pedale in der Mitte (KQ22 a); Zupfort 0,36 der Laenge (Chadefaux u. a. 2012)");
    s.auslenkung = 0.004;  // Chadefaux u. a. 2012
    s.pegelJeStufe = 1.0;  // ZD1 a, ZD1-1 a
    s.neigungJeStufe = 0.0;
    s.winkelGrad = 25.0;
    s.breite = 0.0075;
    s.tempo = 2.0;  // m/s beim Abloesen, als verteilter Kraftstoss (S222)
    s.daempfenS = 0.15;
    return s;
}

ZupfSpiel baueVorgabeGitarre()
{
    ZupfSpiel s;
    s.name = "classical_guitar";
    s.satz = saitensatzGitarre();
    umfangAusSatz(s);
    zupfortFest(s);
    s.auslenkung = 0.004;  // GZ6 a: Pavlidou 1997 nach Scherrer 2013 Tab. 2.1
    s.pegelJeStufe = 1.0;
    s.neigungJeStufe = 0.16;  // GZ6 a, angepasst an Iowa MIS
    s.winkelGrad = 30.0;
    s.breite = 0.0075;
    s.daempfenS = 0.08;
    return s;
}

ZupfSpiel baueVorgabeLaute()
{
    ZupfSpiel s;
    s.name = "lute";
    s.satz = saitensatzLaute();
    umfangAusSatz(s);
    zupfortFest(s);
    s.auslenkung = 0.002;  // uebertragen
    s.pegelJeStufe = 1.0;  // ZD2 a: wie die Gitarre, uebertragen
    s.neigungJeStufe = 0.16;
    s.winkelGrad = 30.0;
    s.breite = 0.0075;
    s.daempfenS = 0.08;
    return s;
}

ZupfSpiel baueVorgabeOud()
{
    ZupfSpiel s;
    s.name = "oud";
    s.satz = saitensatzOud();
    umfangAusSatz(s);
    zupfortFest(s);
    s.auslenkung = 0.002;  // uebertragen
    s.pegelJeStufe = 1.0;  // ZD2 a
    s.neigungJeStufe = 0.16;
    s.winkelGrad = 30.0;
    s.breite = 0.0075;
    s.daempfenS = 0.08;
    return s;
}

}  // namespace lumi::kern::zupf
