/**
 ****************************************************************************************
 * @file   test_LoopLeiserAnschlag.cpp
 * @brief  Messfall LV6 (a), S186: der leise Anschlag in der Hoehe — Kontakt, Kraft und Nachklang je Taste und Stufe
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Befund LV-6 (`docs/modellierung/anbindung-ic/2026-11-10_S186_lv6_gleiche_klaenge.md` §4): bei a-30 im Diskant
 * (Upright G7, CP-70 H6) kommt nur ein Puls, danach null; stille Stabspiele in der Hoehe (PN2 a). Hier je Kern
 * (Hammer-Saite: Fluegel, Upright, Honky-tonk, CP-70; Stab: jede Vorlage mit Schlaegel) und je Taste und Stufe:
 * Geschwindigkeit, Kontaktzeit, groesste Kraft, Periode, Pegel des Rohtons und **Nachklang** — die Energie ab dem Ende
 * des Kontakts plus 1 ms gegen die ganze, in dB. Nur mit `LUMIVIZ_LV6A_MESSUNG=<datei.md>`; schreibt die Tafel dorthin.
 ****************************************************************************************
 */
#include <doctest.h>

#include "InstrumentModels/hammer/HammerKern.hpp"
#include "InstrumentModels/hammer/HammerTon.hpp"
#include "InstrumentModels/stab/StabKern.hpp"
#include "loop/Instrument.hpp"
#include "FluegelSchicht.hpp"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QtGlobal>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace
{

constexpr int kRate = 48000;

double hz(int taste) { return 440.0 * std::exp2((taste - 69) / 12.0); }

struct Zeile
{
    double tempo = 0.0;   ///< m/s
    double kontaktS = 0.0;
    double kraftN = 0.0;
    double pegelDb = 0.0;      ///< Energie des Rohtons, dB (willkuerlicher Bezug, je Kern gleich)
    double nachklangDb = 0.0;  ///< Energie ab Kontaktende + 1 ms gegen die ganze
    int kontakte = 1;          ///< so oft beruehrt der Schlaegel (Stab; der Hammer zaehlt nicht)
};

Zeile auswerten(const std::vector<float>& mono, double kontaktS)
{
    Zeile z;
    const auto ab = static_cast<std::size_t>(std::llround((kontaktS + 1e-3) * kRate));
    double gesamt = 0.0, danach = 0.0;
    for (std::size_t i = 0; i < mono.size(); ++i)
    {
        const double e = static_cast<double>(mono[i]) * mono[i];
        gesamt += e;
        if (i >= ab) danach += e;
    }
    z.pegelDb = 10.0 * std::log10(gesamt + 1e-300);
    z.nachklangDb = 10.0 * std::log10(danach / std::max(gesamt, 1e-300) + 1e-300);
    return z;
}

std::string zeile(const std::string& name, int taste, int stufe, const Zeile& z)
{
    std::ostringstream o;
    o << std::fixed << "| " << name << " | " << taste << " | " << stufe << " | " << std::setprecision(3) << z.tempo << " | "
      << std::setprecision(2) << z.kontaktS * 1e3 << " | " << 1e3 / hz(taste) << " | " << std::setprecision(1)
      << z.kontaktS * hz(taste) << " | " << std::setprecision(3) << z.kraftN << " | " << std::setprecision(1) << z.pegelDb
      << " | " << z.nachklangDb << " | " << z.kontakte << " |\n";
    return o.str();
}

}  // namespace

TEST_CASE("Loop Leiser Anschlag LV6 a (LUMIVIZ_LV6A_MESSUNG) — Kontakt und Nachklang je Taste und Stufe")
{
    const QString datei = qEnvironmentVariable("LUMIVIZ_LV6A_MESSUNG");
    if (datei.isEmpty())
    {
        MESSAGE("LUMIVIZ_LV6A_MESSUNG=<datei.md> misst Kontakt und Nachklang je Taste und Stufe");
        return;
    }
    namespace hammer = lumi::kern::hammer;
    namespace stab = lumi::kern::stab;
    const int stufen[] = {-40, -36, -30, -24, -18, -12, -6, 0};
    std::ostringstream t;
    t << "| Instrument | Taste | Stufe | v m/s | Kontakt ms | Periode ms | Kontakt/Periode | Kraft N | Pegel dB | Nachklang dB | Kontakte |\n"
         "|---|---|---|---|---|---|---|---|---|---|---|\n";

    std::vector<std::pair<std::string, hammer::HammerSaite>> saiten{{"grand (Vorgabe)", hammer::hammerSaiteVorgabe()}};
    {
        // der ausgelieferte Fluegel (gesetzt an Salamander), vom Arbeitsordner aus gelesen
        QFile f(QStringLiteral("asset/loop/instruments/grand_piano.lvinst"));
        REQUIRE_MESSAGE(f.open(QIODevice::ReadOnly), "asset/loop/instruments/grand_piano.lvinst vom Projekt-Root aus");
        lumi::loop::Instrument i;
        REQUIRE(lumi::loop::ausJson(QJsonDocument::fromJson(f.readAll()).object(), i));
        saiten.emplace_back("grand_piano", fluegeltest::saiteVon(i.schichten[0]));
    }
    // Probe: der Vorgabe-Fluegel mit fester Hysterese ε (Stulov Gl. 9 zeigt im Bild „+ 8,8·10⁻⁵ n²“, der Kern rechnet
    // „−“ seit HU3-1 c) — wirkt die Hysterese auf den Ton im Diskant?
    for (const double eps : {0.9, 0.99, 0.997})
    {
        hammer::HammerSaite s = hammer::hammerSaiteVorgabe();
        for (auto& p : s.hammerHysterese.punkte) p.wert = eps;
        std::ostringstream n;
        n << "grand eps " << eps;
        saiten.emplace_back(n.str(), s);
    }
    for (const hammer::Vorlage& v : hammer::vorlagen()) saiten.emplace_back(v.name, v.saite);
    for (const auto& [name, saite] : saiten)
        for (const int taste : {60, 72, 84, 90, 96, 100, 103, 106, 108})
            for (const int stufe : stufen)
            {
                hammer::HammerAnschlag a;
                a.taste = taste;
                a.hz = hz(taste);
                a.stufe = stufe;
                a.halten = kRate;
                a.gedaempft = false;
                a.rate = kRate;
                hammer::HammerMessung m;
                std::vector<float> mono;
                hammer::rechneHammerTon(saite, a, mono, kRate, &m);
                Zeile z = auswerten(mono, m.kontaktS);
                z.tempo = hammer::hammerTempo(stufe);
                z.kontaktS = m.kontaktS;
                z.kraftN = m.kraftSpitzeN;
                t << zeile(name, taste, stufe, z);
            }

    for (const stab::Vorlage& v : stab::vorlagen())
    {
        if (v.spiel.anregung != stab::Anregung::Stoss || v.spiel.bogen) continue;
        for (const int taste : {60, 72, 84, 91, 96, 103})
            for (const int stufe : stufen)
            {
                stab::StabAnschlag a;
                a.note = taste;
                a.hz = hz(taste);
                a.stufe = stufe;
                stab::StabMessung m;
                std::vector<float> mono;
                stab::rechneStabTon(v.spiel, a, mono, kRate, &m);
                Zeile z = auswerten(mono, m.kontaktS);
                z.tempo = v.spiel.tempoMax * std::pow(10.0, std::clamp(stufe, -40, 0) / 20.0);
                z.kontaktS = m.kontaktS;
                z.kraftN = m.kraftSpitzeN;
                z.kontakte = m.kontakte;
                t << zeile(v.name, taste, stufe, z);
            }
    }
    std::ofstream aus(datei.toStdString(), std::ios::binary);
    REQUIRE(aus.good());
    aus << t.str();
    MESSAGE("Tafel nach " << datei.toStdString());
}
