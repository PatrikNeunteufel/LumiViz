/**
 ****************************************************************************************
 * @file   test_LoopKoerperPruefstein.cpp
 * @brief  Loop, Stufe 31c-6 Teil 5: der Pruefstein des gerechneten Resonanzbodens —
 *         gegen MAESSTRO, gegen Messungen, gegen sich selbst mit hoeherer Ordnung
 *         (docs/audio/Spielweisen_Konzept.md §4.1, Entscheide Patrik S149/S150: PS1 a,
 *         KV1 a, EM1 a, PF1 a, PF2 a, BM7 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Qt-Anwendung; liest Koerper vom Arbeitsordner aus (die Test-Exe aus
 * dem Projekt-Root starten). Die Mobilitaet nach MAESSTRO Gl. 10 aus den **gerechneten** Moden
 * (PS1 a; ohne den statistischen Teil — MAESSTRO rechnet seine Basis bis 5 kHz).
 *
 * - **PS1 a (1)**, der MAESSTRO-Prueffall (`koerper/maesstro_rp.lvinstbody`): je Stegpunkt C1–B7
 *   das Mittel nach Gl. 12 gegen die Kurve „Reference piano" aus Abb. 6 (`werte/maesstro_abb6_rp.txt`,
 *   abgelesen mit `asset/loop/instrument_bodies/maesstro_ablesen.py`) — **nur berichtet** (PS3 a, S150),
 *   je Taste, je Oktave, Versatz und Abstand Bass/Diskant, bis die Daten des Referenzfluegels vorliegen.
 *   Dazu die Modendichte nach Gl. 6 (gleitend ueber sechs Moden) gegen Abb. 5 — ebenfalls nur berichtet.
 * - **PS1 a (2)**, der Steinway D: Gl. 12 je Stegpunkt, Spanne und Abstand Bass/Diskant — **nur
 *   berichtet** (PS4 a, S150; die Grenzen stammen aus dem vereinfachten Fluegel von MAESSTRO).
 * - **PS1 a (3)**, der Steinway D gegen Messungen: je Terzband ab 500 Hz das Mittel ueber den Diskant
 *   in −72 … −58 dB re 1 s/kg und kein Anstieg oder Abfall ueber 6 dB; unter 500 Hz das Mittel von |Y|
 *   ueber 50–1000 Hz gegen Plath 2019 Fig. 6.9 (0,0007 s/kg, bis Faktor 2).
 * - **EM1 a**: die erste Mode in 35–65 Hz, die Dichte unter 500 Hz in 0,04–0,14 /Hz.
 * - **KV1 a** nur mit `LUMIVIZ_KOERPER_K4=<datei.lvinstbody>` (der Steinway mit k = 4, nicht im
 *   Repo, 44 min zu rechnen): je Terzband 100 Hz – 5 kHz die Zahl der Moden bis zur oberen Grenze auf
 *   ±3 % und die Mobilitaet auf ±0,5 dB, die ersten 20 Moden je auf η/4 (KV2 a), Gl. 2 an den fuenf
 *   hoechsten Moden berichtet.
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentBody/Koerper.hpp"
#include "InstrumentBody/Mobilitaet.hpp"
#include "loop/Koerper.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include <algorithm>
#include <cmath>
#include <map>
#include <vector>

namespace kp = lumi::koerper;

namespace
{

QString werteDatei(const char* name)
{
    return QFileInfo(QString::fromLocal8Bit(__FILE__)).absoluteDir().filePath(QStringLiteral("werte/") + QLatin1String(name));
}

/// Zahlenpaare aus einer Bezugsdatei (Zeilen mit # ueberlesen).
std::map<double, double> liesPaare(const QString& pfad)
{
    std::map<double, double> paare;
    QFile datei(pfad);
    if (!datei.open(QIODevice::ReadOnly | QIODevice::Text)) return paare;
    QTextStream ein(&datei);
    while (!ein.atEnd())
    {
        const QString zeile = ein.readLine().trimmed();
        if (zeile.isEmpty() || zeile.startsWith(QLatin1Char('#'))) continue;
        const QStringList teile = zeile.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        if (teile.size() >= 2) paare[teile[0].toDouble()] = teile[1].toDouble();
    }
    return paare;
}

kp::Koerper lade(const QString& pfad)
{
    kp::Koerper koerper;
    REQUIRE_MESSAGE(lumi::loop::ladeKoerper(pfad, koerper), "nicht gelesen: ", pfad.toStdString(),
                    " (die Test-Exe aus dem Projekt-Root starten)");
    return koerper;
}

/// MAESSTRO Gl. 6, gleitend ueber sechs Moden: (Frequenz, Dichte) je Mittel.
std::vector<std::pair<double, double>> dichteGl6(const std::vector<kp::ModeImKoerper>& moden)
{
    std::vector<std::pair<double, double>> roh;
    for (std::size_t k = 0; k + 1 < moden.size(); ++k)
    {
        const double abstand = moden[k + 1].frequenzHz - moden[k].frequenzHz;
        if (abstand > 0.0) roh.push_back({0.5 * (moden[k].frequenzHz + moden[k + 1].frequenzHz), 1.0 / abstand});
    }
    std::vector<std::pair<double, double>> glatt;
    for (std::size_t k = 0; k + 6 <= roh.size(); ++k)
    {
        double f = 0.0, n = 0.0;
        for (std::size_t j = k; j < k + 6; ++j)
        {
            f += roh[j].first;
            n += roh[j].second;
        }
        glatt.push_back({f / 6.0, n / 6.0});
    }
    return glatt;
}

double interpoliert(const std::vector<std::pair<double, double>>& kurve, double f)
{
    for (std::size_t k = 0; k + 1 < kurve.size(); ++k)
        if (kurve[k].first <= f && f <= kurve[k + 1].first)
            return kurve[k].second + (f - kurve[k].first) / (kurve[k + 1].first - kurve[k].first) * (kurve[k + 1].second - kurve[k].second);
    return std::nan("");
}

std::vector<std::size_t> punkte(const kp::Koerper& koerper, bool nurDiskant)
{
    std::vector<std::size_t> p;
    for (std::size_t j = 0; j < koerper.stegTasten.size(); ++j)
        if (!nurDiskant || koerper.stegTasten[j] >= kp::kDiskantAb) p.push_back(j);
    return p;
}

}  // namespace

TEST_CASE("Loop Koerper 31c-6 Pruefstein MAESSTRO (PS1 a 1)")
{
    const kp::Koerper koerper = lade(QStringLiteral("projects/apps/LumiViz/tests/unit/UnitTests/koerper/maesstro_rp.lvinstbody"));
    const std::map<double, double> abb6 = liesPaare(werteDatei("maesstro_abb6_rp.txt"));
    REQUIRE(abb6.size() == 84);

    // je Stegpunkt C1–B7 (Index 1–84 = MIDI 24–107) Gl. 12 gegen Abb. 6 — nur berichtet (PS3 a, S150):
    // MAESSTRO legt die Formen auf eine Sinus-Basis unbekannter Groesse und verschiebt die Stegpunkte
    // auf `ref_point` (Recherche docs/modellierung/hammer-saite/recherche/2026-10-24_S150_maesstro_quellcode.md);
    // geprueft wird erst mit den Daten des Referenzfluegels, gleich gegen gleich
    std::map<int, double> uns;
    for (std::size_t j = 0; j < koerper.stegTasten.size(); ++j)
    {
        const int taste = koerper.stegTasten[j];
        if (abb6.count(taste - 23)) uns[taste] = kp::mittlereMobilitaetDb(koerper.moden, j);
    }
    REQUIRE(uns.size() == 84);
    double summe = 0.0;
    for (int start = 24; start < 108; start += 12)
    {
        double u = 0.0, a = 0.0;
        for (int taste = start; taste < start + 12; ++taste)
        {
            u += uns.at(taste);
            a += abb6.at(taste - 23);
        }
        summe += u - a;
        MESSAGE("Oktave ", start, "–", start + 11, ": ", u / 12.0, " dB, Abb. 6 ", a / 12.0, " dB (", (u - a) / 12.0, ")");
    }
    const auto mittel = [&](int von, int bis, bool abb) {
        double s = 0.0;
        for (int taste = von; taste <= bis; ++taste) s += abb ? abb6.at(taste - 23) : uns.at(taste);
        return s / (bis - von + 1);
    };
    MESSAGE("Versatz gegen Abb. 6 im Mittel ", summe / 84.0, " dB; Abstand Diskant (42–101) zu Bass (24–38) ",
            mittel(42, 101, false) - mittel(24, 38, false), " dB, Abb. 6 ", mittel(42, 101, true) - mittel(24, 38, true), " dB");

    // die Modendichte gegen Abb. 5: berichtet (PF1 a, keine Toleranz)
    const std::map<double, double> abb5 = liesPaare(werteDatei("maesstro_abb5_rp.txt"));
    REQUIRE(abb5.size() > 90);
    const auto gl6 = dichteGl6(koerper.moden);
    for (const double f : {200.0, 500.0, 1000.0, 1500.0, 2000.0, 2500.0, 3000.0, 3500.0, 4000.0, 4500.0})
        MESSAGE("Dichte bei ", f, " Hz: ", interpoliert(gl6, f), " /Hz, Abb. 5 ", abb5.at(f), " /Hz (Verhaeltnis ",
                interpoliert(gl6, f) / abb5.at(f), ")");
    MESSAGE(koerper.moden.size(), " Moden bis 5 kHz; erste ", koerper.moden.front().frequenzHz, " Hz");
}

TEST_CASE("Loop Koerper 31c-6 Pruefstein Steinway (PS1 a 2 und 3, EM1 a)")
{
    const kp::Koerper koerper = lade(QStringLiteral("asset/loop/instrument_bodies/steinway_d.lvinstbody"));
    const std::vector<kp::ModeImKoerper>& moden = koerper.moden;

    // (2) Gl. 12 je Stegpunkt, Spanne und Abstand Bass/Diskant — nur berichtet (PS4 a, S150): die Grenzen von PS1 a
    // (−75 … −57 dB, Bass 5–15 dB unter dem Diskant) stammen aus dem vereinfachten Fluegel von MAESSTRO (Stege 60
    // und 20 mm, getrennt, eine Rippe mehr); der Steinway folgt RT-0425 (Steg 29–69 mm, verbunden wie bei Plath)
    double bass = 0.0, diskant = 0.0, lo = 0.0, hi = -200.0;
    int nBass = 0, nDiskant = 0, hoch = 0;
    for (std::size_t j = 0; j < koerper.stegTasten.size(); ++j)
    {
        const double db = kp::mittlereMobilitaetDb(moden, j);
        lo = std::min(lo, db);
        hi = std::max(hi, db);
        hoch += db > -57.0 ? 1 : 0;
        (koerper.stegTasten[j] < kp::kDiskantAb ? bass : diskant) += db;
        ++(koerper.stegTasten[j] < kp::kDiskantAb ? nBass : nDiskant);
    }
    MESSAGE("Gl. 12 am Steinway: ", lo, " … ", hi, " dB (", hoch, " ueber −57 dB); Bass ", bass / nBass, ", Diskant ",
            diskant / nDiskant, " dB, Abstand ", diskant / nDiskant - bass / nBass, " dB");

    // (3) je Terzband ab 500 Hz das Mittel ueber den Diskant in −72 … −58 dB, kein Anstieg oder Abfall
    // ueber 6 dB; die Baender ganz unter 5 kHz (das Band „5000 Hz" reicht bis 5623 Hz, nur halb gerechnet)
    const std::vector<std::size_t> imDiskant = punkte(koerper, true);
    double bandLo = 0.0, bandHi = -200.0;
    for (const kp::Terzband& band : kp::terzbaender(500.0, 5000.0))
    {
        if (band.bisHz > 5000.0) continue;
        const double db = kp::bandMobilitaetDb(moden, imDiskant, band);
        MESSAGE("Terzband ", band.mitteHz, " Hz: Diskant ", db, " dB, Dichte ", kp::bandDichte(moden, band), " /Hz");
        CHECK_MESSAGE(db >= -72.0, band.mitteHz, " Hz");
        CHECK_MESSAGE(db <= -58.0, band.mitteHz, " Hz");
        bandLo = std::min(bandLo, db);
        bandHi = std::max(bandHi, db);
    }
    MESSAGE("Spanne der Terzbaender 500 Hz – 4 kHz: ", bandHi - bandLo, " dB");
    CHECK(bandHi - bandLo <= 6.0);
    // unter 500 Hz: Plath Fig. 6.9, das Mittel von |Y| ueber 50–1000 Hz, 0,0007 s/kg bis Faktor 2
    double y = 0.0;
    int n = 0;
    for (std::size_t j = 0; j < koerper.stegTasten.size(); ++j)
        for (double f = 50.0; f <= 1000.0; f += 1.0, ++n) y += std::abs(kp::mobilitaet(moden, j, f));
    MESSAGE("Mittel |Y| 50–1000 Hz ueber alle Stegpunkte: ", y / n, " s/kg (Plath 0,0007)");
    CHECK(y / n >= 0.00035);
    CHECK(y / n <= 0.0014);

    // EM1 a: die erste Mode in 35–65 Hz, die Dichte unter 500 Hz in 0,04–0,14 /Hz
    const double erste = moden.front().frequenzHz;
    const auto unter500 = std::count_if(moden.begin(), moden.end(), [](const kp::ModeImKoerper& m) { return m.frequenzHz < 500.0; });
    MESSAGE("erste Mode ", erste, " Hz (Corradi Stufe 3: 43,1 Hz); ", unter500, " Moden unter 500 Hz, ", unter500 / 500.0, " /Hz");
    CHECK(erste >= 35.0);
    CHECK(erste <= 65.0);
    CHECK(unter500 / 500.0 >= 0.04);
    CHECK(unter500 / 500.0 <= 0.14);
}

TEST_CASE("Loop Koerper 31c-6 Konvergenz k = 4 (KV1 a)")
{
    const QString pfadK4 = qEnvironmentVariable("LUMIVIZ_KOERPER_K4");
    if (pfadK4.isEmpty())
    {
        MESSAGE("LUMIVIZ_KOERPER_K4 nicht gesetzt — die Konvergenz wird nicht geprueft");
        return;
    }
    const kp::Koerper k3 = lade(QStringLiteral("asset/loop/instrument_bodies/steinway_d.lvinstbody"));
    const kp::Koerper k4 = lade(pfadK4);
    REQUIRE(k3.rechnung.ordnung == 3);
    REQUIRE(k4.rechnung.ordnung == 4);
    REQUIRE(k3.stegTasten == k4.stegTasten);
    REQUIRE(k3.bauplanAbdruck == k4.bauplanAbdruck);
    MESSAGE("k = 3: ", k3.moden.size(), " Moden, ", k3.rechnung.freieGroessen, " Groessen; k = 4: ", k4.moden.size(), " Moden, ",
            k4.rechnung.freieGroessen, " Groessen");

    // (1) je Terzband 100 Hz – 5 kHz: die Zahl der Moden bis zur oberen Grenze ±3 % (KV2 a, S150: gezaehlt bis zur
    // Grenze statt je Band — bei rund 50 Moden je Band laegen 3 % unter der Zaehlaufloesung), die Mobilitaet ueber
    // alle Stegpunkte ±0,5 dB
    const std::vector<std::size_t> alle = punkte(k3, false);
    const auto bis = [](const std::vector<kp::ModeImKoerper>& m, double f) {
        return std::count_if(m.begin(), m.end(), [f](const kp::ModeImKoerper& x) { return x.frequenzHz < f; });
    };
    for (const kp::Terzband& band : kp::terzbaender(100.0, 5000.0))
    {
        if (band.bisHz > 5000.0) continue;  // das Band „5000 Hz" ist nur halb gerechnet
        const auto n3 = bis(k3.moden, band.bisHz), n4 = bis(k4.moden, band.bisHz);
        const double y3 = kp::bandMobilitaetDb(k3.moden, alle, band), y4 = kp::bandMobilitaetDb(k4.moden, alle, band);
        MESSAGE("Terzband ", band.mitteHz, " Hz: Moden bis ", band.bisHz, " Hz ", n3, " / ", n4, ", Mobilitaet ", y3, " / ", y4,
                " dB (", y3 - y4, ")");
        CHECK_MESSAGE(std::abs(static_cast<double>(n3 - n4)) <= 0.03 * static_cast<double>(n4), band.mitteHz, " Hz: Zahl der Moden");
        CHECK_MESSAGE(std::abs(y3 - y4) <= 0.5, band.mitteHz, " Hz: Mobilitaet");
    }
    // (2) die ersten 20 Moden einzeln gegen ein Viertel ihrer Halbwertsbreite: |Δf| / f <= η / 4 (KV2 a)
    REQUIRE(k3.moden.size() >= 20);
    REQUIRE(k4.moden.size() >= 20);
    double groesste = 0.0;
    for (std::size_t i = 0; i < 20; ++i)
    {
        const double r = std::abs(k3.moden[i].frequenzHz - k4.moden[i].frequenzHz) / k4.moden[i].frequenzHz;
        const double grenze = k4.moden[i].verlustfaktor / 4.0;
        groesste = std::max(groesste, r / grenze);
        CHECK_MESSAGE(r <= grenze, "Mode ", i + 1, ": ", k3.moden[i].frequenzHz, " / ", k4.moden[i].frequenzHz, " Hz, eta ",
                      k4.moden[i].verlustfaktor);
    }
    MESSAGE("erste 20 Moden: groesste Abweichung das ", groesste, "-fache von eta/4");
    // (3) MAESSTRO Gl. 2 an den fuenf hoechsten Moden, nach Rang zugeordnet — berichtet
    const std::size_t n = std::min(k3.moden.size(), k4.moden.size());
    double gl2 = 0.0;
    for (std::size_t i = n - 5; i < n; ++i) gl2 += std::abs(k4.moden[i].frequenzHz - k3.moden[i].frequenzHz) / k3.moden[i].frequenzHz;
    MESSAGE("MAESSTRO Gl. 2 (fuenf hoechste nach Rang): ", gl2 / 5.0);
}
