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
#include "InstrumentMeasure/Fluegelmessung.hpp"  // messeKanal (Helligkeit je Staerke wie an Salamander)

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QtGlobal>

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <functional>
#include <map>
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

Zeile auswerten(const std::vector<float>& mono, double kontaktS, int rate = kRate)
{
    Zeile z;
    const auto ab = static_cast<std::size_t>(std::llround((kontaktS + 1e-3) * rate));
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

namespace
{

namespace hammer = lumi::kern::hammer;

/// Ein Satz von Hammerwerten fuer HS-1: ein Name und was er an der Vorgabe des Fluegels aendert.
struct Satz
{
    std::string name;
    std::function<void(hammer::HammerSaite&)> setze;
};

/// Der Ton nach dem Kontakt (Kontaktende + 1 ms), dB, und die Kontaktzeit — der Kontakt `ueber`-fach feiner.
std::pair<double, double> tonUndKontakt(const hammer::HammerSaite& s, int taste, double tempo, int ueber)
{
    const int rate = kRate;
    hammer::HammerAnschlag a;
    a.ueberabtastung = ueber;
    a.taste = taste;
    a.hz = hz(taste);
    a.stufe = static_cast<int>(std::lround(20.0 * std::log10(tempo / hammer::kHammerTempoMax)));  // Stoss je Saite der Stufe
    a.tempoFaktor = tempo / hammer::hammerTempo(a.stufe);                                          // genau dieses Tempo
    a.halten = rate;
    a.gedaempft = false;
    a.rate = rate;
    hammer::HammerMessung m;
    std::vector<float> mono;
    hammer::rechneHammerTon(s, a, mono, rate, &m);
    const Zeile z = auswerten(mono, m.kontaktS, rate);
    return {z.pegelDb + z.nachklangDb, m.kontaktS};
}


/// `werte/<name>` neben dieser Datei.
QString werteDatei(const char* name)
{
    return QFileInfo(QString::fromLocal8Bit(__FILE__)).absoluteDir().filePath(QStringLiteral("werte/") + QLatin1String(name));
}

}  // namespace

TEST_CASE("Loop Leiser Anschlag HS-1 Saetze (LUMIVIZ_HS1_MESSUNG) — Hammerwerte gegen alle Zielgroessen")
{
    const QString datei = qEnvironmentVariable("LUMIVIZ_HS1_MESSUNG");
    if (datei.isEmpty())
    {
        MESSAGE("LUMIVIZ_HS1_MESSUNG=<datei.md> rechnet die Saetze von Hammerwerten gegen alle Zielgroessen");
        return;
    }
    // Zielgroessen (Befund modellierung/hammer-saite/2026-11-10_S186_befund_leiser_anschlag_diskant.md §5):
    // (1) Kontaktzeit pp / mf / ff an C6, C7, C8 gegen Askenfelt & Jansson 1990 Abb. 7 (abgelesen ±0,1 ms; pp ≈ 0,63 m/s
    //     wie DY2 a, mf 1,26, ff 5 m/s — die Geschwindigkeiten der Abbildung sind nicht genannt)
    // (2) Umfang des Tons ppp (0,45 m/s, DY2 a) → ff (5 m/s) je Taste gegen *Salamander* `pegelumfang` (v1 → v16)
    // (3) Helligkeit je Staerke an C4 und C6 gegen *Salamander* (HU3-1, `test_LoopHammerTon`)
    // (4) Spreizung ueber den Umfang (A0 … C8) — in (2) enthalten
    // (5) Auslenkung je Durchmesser gegen Chabassier, Joly & Chaigne 2013 Tab. IV
    std::map<int, double> umfangSal;
    {
        std::ifstream f(werteDatei("salamander_c5.txt").toStdString());
        REQUIRE(f.good());
        std::string zeile;
        while (std::getline(f, zeile))
        {
            if (zeile.empty() || zeile[0] == '#') continue;
            std::istringstream z(zeile);
            int taste = 0;
            std::string groesse;
            double wert = 0.0;
            z >> taste >> groesse >> wert;
            if (groesse == "pegelumfang") umfangSal[taste] = wert;
        }
        REQUIRE(umfangSal.size() >= 20);
    }
    struct Kontakt
    {
        int taste;
        double pp, mf, ff;
    };
    const Kontakt askenfelt[] = {{84, 2.5, 1.1, 0.85}, {96, 0.9, 0.65, 0.55}, {108, 1.2, 0.55, 0.45}};
    const std::array<double, 7> salC4{0.20, 0.25, 0.69, 0.98, 0.97, 1.65, 1.92};
    const std::array<double, 7> salC6{1.17, 0.73, 2.18, 1.48, 1.53, 1.81, 2.55};
    struct Adr
    {
        int taste;
        double dMm, tempo, adr;
    };
    const Adr chabassier[] = {{27, 1.48, 0.5, 0.568}, {27, 1.48, 3.0, 3.59}, {36, 0.9502, 1.5, 2.01}, {53, 1.0525, 0.5, 0.337},
                              {53, 1.0525, 3.0, 2.10}, {73, 0.921, 4.5, 2.64},   {91, 0.794, 1.5, 0.443}};

    std::vector<Satz> saetze{
        {"heute (Stulov Gl. 9, eps mit Minus)", [](hammer::HammerSaite&) {}},
        {"Probe eps = 0,997 fest", [](hammer::HammerSaite& s) {
             for (auto& p : s.hammerHysterese.punkte) p.wert = 0.997;
         }},
        // Kartofelev & Stulov 2014 (Acta Mechanica; Dissertation Kartofelev 2014, S. 73, Gl. 4–7, am Bild gelesen S186):
        // γ = 0,9894 + 0,000088 N (0,9895 … 0,9972) — das n² im Druck von 2009 ist der Druckfehler
        {"S1: eps = 0,9894 + 0,000088 n, Q0 nach 2009", [](hammer::HammerSaite& s) {
             for (auto& p : s.hammerHysterese.punkte) p.wert = 0.9894 + 0.000088 * (p.note - 20);
         }},
        {"S2: eps linear, Q0 = F0·(1 − eps), F0 = 15500·e^(0,059 n)", [](hammer::HammerSaite& s) {
             for (auto& p : s.hammerHysterese.punkte) p.wert = 0.9894 + 0.000088 * (p.note - 20);
             for (auto& p : s.hammerSteife.punkte)
             {
                 const int n = p.note - 20;
                 p.wert = 15500.0 * std::exp(0.059 * n) * (1.0 - (0.9894 + 0.000088 * n));
             }
         }},
    };

    std::ostringstream o;
    o << std::fixed;
    // die Konvergenz: der Kontakt 8- (wie im Programm), 32- und 128-fach feiner (τ0 ≈ 1,7 µs gegen 2,6 µs bei 8)
    std::vector<int> stufenFein{8, 32, 128};
    for (const Satz& satz : saetze)
        for (const int ueber : stufenFein)
    {
        hammer::HammerSaite s = hammer::hammerSaiteVorgabe();
        satz.setze(s);
        o << "\n## " << satz.name << " — Kontakt " << ueber << "-fach\n\n";
        // (1)
        o << "Kontaktzeit ms pp / mf / ff (Askenfelt & Jansson):";
        double fehler1 = 0.0;
        for (const Kontakt& k : askenfelt)
        {
            const double pp = tonUndKontakt(s, k.taste, 0.63, ueber).second * 1e3, mf = tonUndKontakt(s, k.taste, 1.26, ueber).second * 1e3,
                         ff = tonUndKontakt(s, k.taste, 5.0, ueber).second * 1e3;
            o << std::setprecision(2) << ' ' << k.taste << ": " << pp << " / " << mf << " / " << ff << " (" << k.pp << " / " << k.mf
              << " / " << k.ff << ");";
            for (const auto& [m, g] : {std::pair{pp, k.pp}, std::pair{mf, k.mf}, std::pair{ff, k.ff}}) fehler1 += std::pow(std::log(m / g), 2);
        }
        o << std::setprecision(3) << " — RMS log-Fehler " << std::sqrt(fehler1 / 9.0) << "\n\n";
        // (2), (4)
        o << "Umfang des Tons ppp → ff, dB (Salamander):";
        double fehler2 = 0.0;
        int n2 = 0;
        for (const auto& [taste, sal] : umfangSal)
        {
            const double u = tonUndKontakt(s, taste, 5.0, ueber).first - tonUndKontakt(s, taste, 0.45, ueber).first;
            o << std::setprecision(1) << ' ' << taste << ": " << u << " (" << sal << ");";
            fehler2 += (u - sal) * (u - sal);
            ++n2;
        }
        o << " — RMS " << std::sqrt(fehler2 / n2) << " dB\n\n";
        // (3)
        const auto helligkeit = [&](int taste) {
            std::array<std::vector<double>, 7> grund, rel;
            for (int stufe = -30; stufe <= 0; stufe += 2)
            {
                hammer::HammerAnschlag a;
                a.taste = taste;
                a.hz = hz(taste);
                a.stufe = stufe;
                a.halten = kRate;
                a.rate = kRate;
                a.ueberabtastung = ueber;
                std::vector<float> mono;
                hammer::rechneHammerTon(s, a, mono, kRate);
                const std::vector<double> d(mono.begin(), mono.end());
                const lumi::messung::KanalMessung m = lumi::messung::messeKanal(d, kRate, hz(taste), lumi::kern::fft(), false);
                const lumi::messung::TeiltonKurz* t1 = m.erkannt ? m.teilton(1) : nullptr;
                if (t1 == nullptr) continue;
                for (int n = 2; n <= 8; ++n)
                    if (const lumi::messung::TeiltonKurz* tn = m.teilton(n); tn != nullptr)
                    {
                        grund[static_cast<std::size_t>(n - 2)].push_back(t1->anfangDb);
                        rel[static_cast<std::size_t>(n - 2)].push_back(tn->anfangDb - t1->anfangDb);
                    }
            }
            std::array<double, 7> w{};
            for (std::size_t j = 0; j < 7; ++j)
                w[j] = grund[j].size() >= 6 ? lumi::messung::detail::Geraden(grund[j], rel[j]).ueber(0, grund[j].size()).steigung
                                            : std::nan("");
            return w;
        };
        for (const auto& [taste, sal] : {std::pair{60, salC4}, std::pair{84, salC6}})
        {
            const std::array<double, 7> w = helligkeit(taste);
            o << "Helligkeit je Staerke n = 2 … 8 an " << taste << ":" << std::setprecision(2);
            double f = 0.0;
            for (std::size_t j = 0; j < 7; ++j)
            {
                o << ' ' << w[j] << " (" << sal[j] << ")";
                f += std::pow(w[j] - sal[j], 2);
            }
            o << " — RMS " << std::sqrt(f / 7.0) << "\n\n";
        }
        // (5)
        o << "Auslenkung je Durchmesser / Chabassier Tab. IV:";
        for (const Adr& z : chabassier)
        {
            hammer::HammerAnschlag a;
            a.taste = z.taste;
            a.hz = hz(z.taste);
            a.stufe = static_cast<int>(std::lround(20.0 * std::log10(z.tempo / hammer::kHammerTempoMax)));
            a.tempoFaktor = z.tempo / hammer::hammerTempo(a.stufe);
            a.halten = kRate;
            a.rate = kRate;
            a.ueberabtastung = ueber;
            hammer::HammerMessung m;
            std::vector<float> mono;
            hammer::rechneHammerTon(s, a, mono, kRate / 10, &m);
            o << std::setprecision(2) << ' ' << z.taste << '@' << z.tempo << ": " << m.auslenkungSpitzeM / (z.dMm * 1e-3) / z.adr << ';';
        }
        o << "\n";
    }
    std::ofstream aus(datei.toStdString(), std::ios::binary);
    REQUIRE(aus.good());
    aus << o.str();
    MESSAGE("Saetze nach " << datei.toStdString());
}
