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
#include <chrono>
#include <cmath>
#include <fstream>
#include <functional>
#include <map>
#include <iomanip>
#include <numbers>
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


/// HS-2 a: der Ton gemessen wie *Salamander* (`messeKanal`): Pegel des ganzen Signals der ersten 0,3 s und Summe der
/// Teiltoene 1 … 8 zu Beginn, dB; −300, wo nichts erkannt ist.
std::pair<double, double> ganzUndTeiltoene(const hammer::HammerSaite& s, int taste, double tempo, int ueber)
{
    hammer::HammerAnschlag a;
    a.ueberabtastung = ueber;
    a.taste = taste;
    a.hz = hz(taste);
    a.stufe = static_cast<int>(std::lround(20.0 * std::log10(tempo / hammer::kHammerTempoMax)));
    a.tempoFaktor = tempo / hammer::hammerTempo(a.stufe);
    a.halten = kRate;
    a.gedaempft = false;
    a.rate = kRate;
    std::vector<float> mono;
    hammer::rechneHammerTon(s, a, mono, kRate);
    const std::vector<double> d(mono.begin(), mono.end());
    const lumi::messung::KanalMessung m = lumi::messung::messeKanal(d, kRate, hz(taste), lumi::kern::fft(), false);
    if (!m.erkannt) return {-300.0, -300.0};
    double summe = 0.0;
    for (const lumi::messung::TeiltonKurz& t : m.teiltoene)
        if (t.k <= 8) summe += std::pow(10.0, t.anfangDb / 10.0);
    return {m.pegelDb, summe > 0.0 ? 10.0 * std::log10(summe) : -300.0};
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
    std::map<int, std::array<double, 7>> hellSal;  // HS-2: Helligkeit je Staerke an jeder gemessenen Taste (0 = nicht gemessen)
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
            if (groesse.rfind("helligkeit_", 0) == 0)
            {
                const int n = std::stoi(groesse.substr(11));
                if (n >= 2 && n <= 8) hellSal[taste][static_cast<std::size_t>(n - 2)] = wert;
            }
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
        // seit HS-3 a (S187) ist die Vorgabe S2; der Satz von vorher (HU3-1 c) hier von Hand
        {"vor HS-3 (Stulov Gl. 9, eps mit Minus)", [](hammer::HammerSaite& s) {
             for (auto& p : s.hammerHysterese.punkte) p.wert = 0.9894 - 8.8e-5 * (p.note - 20) * (p.note - 20);
             for (auto& p : s.hammerSteife.punkte) p.wert = 183.0 * std::exp(0.045 * (p.note - 20));
         }},
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
    // HS-2 (S187): fuer t >> τ0 rechnet der Filz F ≈ Q0·(φ + α·dφ/dt), α = ε·τ0/(1 − ε) — S2 mit kleinerem α zeigt,
    // was die Zaehigkeit an jeder Zielgroesse ausmacht. α ueber ε: ε' = kα/(τ0 + kα), Q0 und τ0 wie S2 (τ0 kleiner
    // ginge nicht: der feine Schritt muss unter τ0 bleiben, sonst verschwindet die Zaehigkeit in der Rekursion)
    for (const double faktor : {0.3, 0.1, 0.03})
    {
        std::ostringstream name;
        name << "S2, alpha x " << faktor << " (eps kleiner, Q0 und tau0 wie S2)";
        const auto s2 = saetze[3].setze;
        saetze.push_back({name.str(), [s2, faktor](hammer::HammerSaite& s) {
                              s2(s);
                              for (std::size_t i = 0; i < s.hammerHysterese.punkte.size(); ++i)
                              {
                                  auto& p = s.hammerHysterese.punkte[i];
                                  const double tau = hammer::stulovHammerRelaxationUs(p.note - 20);
                                  const double alpha = p.wert * tau / (1.0 - p.wert);
                                  p.wert = faktor * alpha / (tau + faktor * alpha);
                              }
                          }});
    }
    // HS-4 (S187): steilerer Filz bei gleicher Zaehigkeit — p um dp hoeher, Q0 so, dass die Kraft bei 0,5 mm Eindrueckung
    // gleich bleibt (Q0·0,5^p fest)
    for (const double dp : {0.5, 1.0, -0.5})
    {
        std::ostringstream name;
        name << "S2, p " << (dp > 0 ? "+" : "") << dp << " (Q0 bei 0,5 mm gleich)";
        const auto s2 = saetze[3].setze;
        saetze.push_back({name.str(), [s2, dp](hammer::HammerSaite& s) {
                              s2(s);
                              for (std::size_t i = 0; i < s.hammerExponent.punkte.size(); ++i)
                                  s.hammerExponent.punkte[i].wert += dp;
                              for (auto& p : s.hammerSteife.punkte) p.wert *= std::pow(0.5, -dp);
                          }});
    }
    if (const QString nur = qEnvironmentVariable("LUMIVIZ_HS1_NUR"); !nur.isEmpty())
        std::erase_if(saetze, [&](const Satz& s) { return s.name.find(nur.toStdString()) == std::string::npos; });
    if (qEnvironmentVariableIsSet("LUMIVIZ_HS1_NUR_S2"))
        saetze.erase(saetze.begin(), saetze.begin() + 3);

    std::ostringstream o;
    o << std::fixed;
    // die Konvergenz: der Kontakt 8- (wie im Programm), 32- und 128-fach feiner (τ0 ≈ 1,7 µs gegen 2,6 µs bei 8);
    // `LUMIVIZ_HS1_FEIN=<n>` rechnet nur eine Stufe (HS-2: 32-fach ist fuer alle Groessen konvergiert)
    std::vector<int> stufenFein{8, 32, 128};
    if (qEnvironmentVariableIsSet("LUMIVIZ_HS1_FEIN")) stufenFein = {qEnvironmentVariableIntValue("LUMIVIZ_HS1_FEIN")};
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
        // (2) HS-2 a: an beiden Seiten gleich gemessen (`messeKanal`) — ganzes Signal 0,3 s und Summe der Teiltoene, gegen
        // die Messung `LUMIVIZ_HS2_SAL_UMFANG` an *Salamander* (Datei `<…>.md.txt`: Taste, ganz, Teiltoene, Teilton 1)
        if (const QString salDatei = qEnvironmentVariable("LUMIVIZ_HS1_SAL_TEILTOENE"); !salDatei.isEmpty())
        {
            std::map<int, std::pair<double, double>> sal;
            std::ifstream f(salDatei.toStdString());
            REQUIRE(f.good());
            std::string zeile;
            while (std::getline(f, zeile))
            {
                if (zeile.empty() || zeile[0] == '#') continue;
                std::istringstream z(zeile);
                int taste = 0;
                double ganz = 0.0, teile = 0.0;
                z >> taste >> ganz >> teile;
                sal[taste] = {ganz, teile};
            }
            REQUIRE(sal.size() >= 20);
            double fGanz = 0.0, fTeile = 0.0, fTeileHoch = 0.0;
            int nGanz = 0, nTeile = 0, nTeileHoch = 0;
            o << "Umfang gleich gemessen, ganz 0,3 s / Teiltoene, dB (Salamander):";
            for (const auto& [taste, ziel] : sal)
            {
                const auto leise = ganzUndTeiltoene(s, taste, 0.45, ueber);
                const auto laut = ganzUndTeiltoene(s, taste, 5.0, ueber);
                const double uGanz = laut.first - leise.first, uTeile = laut.second - leise.second;
                o << std::setprecision(1) << ' ' << taste << ": " << uGanz << " (" << ziel.first << ") / " << uTeile << " ("
                  << ziel.second << ");";
                if (leise.first > -299.0 && laut.first > -299.0)
                {
                    fGanz += (uGanz - ziel.first) * (uGanz - ziel.first);
                    ++nGanz;
                }
                if (leise.second > -299.0 && laut.second > -299.0 && ziel.second > -100.0 && ziel.second < 200.0)
                {
                    fTeile += (uTeile - ziel.second) * (uTeile - ziel.second);
                    ++nTeile;
                    if (taste >= 87)
                    {
                        fTeileHoch += (uTeile - ziel.second) * (uTeile - ziel.second);
                        ++nTeileHoch;
                    }
                }
            }
            o << " — RMS ganz " << std::sqrt(fGanz / std::max(1, nGanz)) << " dB (" << nGanz << "), Teiltoene "
              << std::sqrt(fTeile / std::max(1, nTeile)) << " dB (" << nTeile << "), Teiltoene ab 87 "
              << std::sqrt(fTeileHoch / std::max(1, nTeileHoch)) << " dB (" << nTeileHoch << ")\n\n";
        }
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
        // (3) ueber den ganzen Umfang (HS-2): RMS je Bereich ueber alle gemessenen Teiltoene
        if (!qEnvironmentVariableIsSet("LUMIVIZ_HS1_OHNE_UMFANG_HELL"))
        {
            struct Bereich
            {
                const char* name;
                int von, bis;
                double f = 0.0;
                int n = 0;
            };
            std::array<Bereich, 3> bereiche{{{"Bass 21–45", 21, 45}, {"Mitte 48–69", 48, 69}, {"Diskant 72–105", 72, 105}}};
            o << "Helligkeit ueber den Umfang, Modell − Salamander je Taste (Mittel n = 2 … 8):" << std::setprecision(2);
            for (const auto& [taste, sal] : hellSal)
            {
                const std::array<double, 7> w = helligkeit(taste);
                double summe = 0.0;
                int zahl = 0;
                for (std::size_t j = 0; j < 7; ++j)
                    if (sal[j] != 0.0 && std::isfinite(w[j]))
                    {
                        const double d = w[j] - sal[j];
                        summe += d;
                        ++zahl;
                        for (Bereich& b : bereiche)
                            if (taste >= b.von && taste <= b.bis)
                            {
                                b.f += d * d;
                                ++b.n;
                            }
                    }
                if (zahl > 0) o << ' ' << taste << ": " << summe / zahl << ';';
            }
            o << " — RMS";
            for (const Bereich& b : bereiche) o << ' ' << b.name << ' ' << (b.n > 0 ? std::sqrt(b.f / b.n) : std::nan("")) << " (" << b.n << ')';
            o << "\n\n";
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

namespace
{

/// Steigung der Ausgleichsgeraden y(x).
double steigung(const std::vector<double>& x, const std::vector<double>& y)
{
    const auto n = static_cast<double>(x.size());
    if (x.size() < 3) return std::nan("");
    double sx = 0, sy = 0, sxx = 0, sxy = 0;
    for (std::size_t i = 0; i < x.size(); ++i)
    {
        sx += x[i];
        sy += y[i];
        sxx += x[i] * x[i];
        sxy += x[i] * y[i];
    }
    return (n * sxy - sx * sy) / (n * sxx - sx * sx);
}

/// Betrag des Kraftpulses bei `f` Hz (Fourier-Summe ueber den feinen Schritt), dB.
double pulsDb(const std::vector<double>& kraft, double dtFein, double f)
{
    double re = 0.0, im = 0.0;
    for (std::size_t i = 0; i < kraft.size(); ++i)
    {
        const double phase = 2.0 * std::numbers::pi * f * static_cast<double>(i) * dtFein;
        re += kraft[i] * std::cos(phase);
        im -= kraft[i] * std::sin(phase);
    }
    return 20.0 * std::log10(std::hypot(re, im) * dtFein + 1e-300);
}

}  // namespace

TEST_CASE("Loop Leiser Anschlag HS-2 Diagnose (LUMIVIZ_HS2_MESSUNG) — Helligkeit je Staerke am Ton und am Kraftpuls")
{
    const QString datei = qEnvironmentVariable("LUMIVIZ_HS2_MESSUNG");
    if (datei.isEmpty())
    {
        MESSAGE("LUMIVIZ_HS2_MESSUNG=<datei.md> zerlegt die Helligkeit je Staerke an C4/C6 (Ton und Kraftpuls je Stufe)");
        return;
    }
    // Befund S186 §5: mit dem belegten eps wird der Hammer an C6 mit der Staerke nicht heller. Hier je Stufe: Tempo,
    // Kontaktzeit, groesste Kraft, Grundton am Ton, Teiltoene 2 … 8 gegen den Grundton am Ton und am Kraftpuls allein
    // (ohne Saite: zeigt, ob der Hammer selbst nicht heller wird). Steigungen ueber alle Stufen (-30 … 0, wie HS-1) und
    // ueber den spielbaren Bereich -21 … 0 (0,45 … 5 m/s, DY2 a).
    const double epsN = 0.000088;
    struct Fall
    {
        std::string name;
        std::function<void(hammer::HammerSaite&)> setze;
    };
    const std::vector<Fall> faelle{
        {"vor HS-3 (eps mit Minus)", [](hammer::HammerSaite& s) {
             for (auto& p : s.hammerHysterese.punkte) p.wert = 0.9894 - 8.8e-5 * (p.note - 20) * (p.note - 20);
             for (auto& p : s.hammerSteife.punkte) p.wert = 183.0 * std::exp(0.045 * (p.note - 20));
         }},
        {"S2 (Kartofelev & Stulov 2014)", [epsN](hammer::HammerSaite& s) {
             for (auto& p : s.hammerHysterese.punkte) p.wert = 0.9894 + epsN * (p.note - 20);
             for (auto& p : s.hammerSteife.punkte)
             {
                 const int n = p.note - 20;
                 p.wert = 15500.0 * std::exp(0.059 * n) * (1.0 - (0.9894 + epsN * n));
             }
         }},
    };
    std::vector<int> fein{32};
    if (qEnvironmentVariableIsSet("LUMIVIZ_HS2_FEIN")) fein = {qEnvironmentVariableIntValue("LUMIVIZ_HS2_FEIN")};
    std::ostringstream o;
    o << std::fixed;
    for (const Fall& fall : faelle)
        for (const int ueber : fein)
            for (const int taste : {60, 84})
            {
                hammer::HammerSaite s = hammer::hammerSaiteVorgabe();
                fall.setze(s);
                o << "\n## " << fall.name << " — Taste " << taste << ", Kontakt " << ueber << "-fach\n\n";
                o << "| Stufe | m/s | Kontakt ms | Kraft N | Grund dB | Ton n=2 … 8 gegen Grund, dB | Puls Grund dB | Puls n=2 … 8 gegen Grund, dB |\n";
                o << "|---|---|---|---|---|---|---|---|\n";
                std::array<std::vector<double>, 7> xT, yT, xTs, yTs, xP, yP, xPs, yPs;
                std::vector<double> stufen, grundT;
                for (int stufe = -30; stufe <= 0; stufe += 2)
                {
                    hammer::HammerAnschlag a;
                    a.taste = taste;
                    a.hz = hz(taste);
                    a.stufe = stufe;
                    a.halten = kRate;
                    a.rate = kRate;
                    a.ueberabtastung = ueber;
                    hammer::HammerMessung m;
                    m.mitKraftVerlauf = true;
                    std::vector<float> mono;
                    hammer::rechneHammerTon(s, a, mono, kRate, &m);
                    const std::vector<double> d(mono.begin(), mono.end());
                    const lumi::messung::KanalMessung km = lumi::messung::messeKanal(d, kRate, hz(taste), lumi::kern::fft(), false);
                    const lumi::messung::TeiltonKurz* t1 = km.erkannt ? km.teilton(1) : nullptr;
                    const double dtFein = 1.0 / (static_cast<double>(kRate) * ueber);
                    const double p1 = pulsDb(m.kraftN, dtFein, hz(taste));
                    o << "| " << stufe << " | " << std::setprecision(2) << hammer::hammerTempo(stufe) << " | " << m.kontaktS * 1e3
                      << " | " << std::setprecision(1) << m.kraftSpitzeN << " | " << (t1 ? t1->anfangDb : std::nan("")) << " |";
                    for (int n = 2; n <= 8; ++n)
                    {
                        const lumi::messung::TeiltonKurz* tn = t1 ? km.teilton(n) : nullptr;
                        if (tn != nullptr)
                        {
                            const double r = tn->anfangDb - t1->anfangDb;
                            o << ' ' << r;
                            xT[n - 2].push_back(t1->anfangDb);
                            yT[n - 2].push_back(r);
                            if (stufe >= -21)
                            {
                                xTs[n - 2].push_back(t1->anfangDb);
                                yTs[n - 2].push_back(r);
                            }
                        }
                        else
                            o << " –";
                    }
                    o << " | " << p1 << " |";
                    for (int n = 2; n <= 8; ++n)
                    {
                        const double r = pulsDb(m.kraftN, dtFein, n * hz(taste)) - p1;
                        o << ' ' << r;
                        xP[n - 2].push_back(p1);
                        yP[n - 2].push_back(r);
                        if (stufe >= -21)
                        {
                            xPs[n - 2].push_back(p1);
                            yPs[n - 2].push_back(r);
                        }
                    }
                    o << " |\n";
                    if (t1 != nullptr)
                    {
                        stufen.push_back(stufe);
                        grundT.push_back(t1->anfangDb);
                    }
                }
                const auto reihe = [&](const char* name, const std::array<std::vector<double>, 7>& x,
                                       const std::array<std::vector<double>, 7>& y) {
                    o << name << ":" << std::setprecision(2);
                    for (std::size_t j = 0; j < 7; ++j) o << ' ' << steigung(x[j], y[j]);
                    o << "\n\n";
                };
                o << "\nGrund am Ton je dB Stufe: " << std::setprecision(2) << steigung(stufen, grundT) << "\n\n";
                reihe("Steigung Ton, alle Stufen (wie HS-1)", xT, yT);
                reihe("Steigung Ton, -21 … 0", xTs, yTs);
                reihe("Steigung Puls, alle Stufen", xP, yP);
                reihe("Steigung Puls, -21 … 0", xPs, yPs);
            }
    // Rechenzeit des Kontakts: ein Ton von 1 s mit S2, 8- gegen 32-fach, je fuenfmal, das kleinste (nur das Verhaeltnis
    // zaehlt — die Rechenzeit gegen die Echtzeit ist ausgesetzt, bis die Kuehlung getauscht ist)
    {
        hammer::HammerSaite s = hammer::hammerSaiteVorgabe();
        faelle[1].setze(s);
        o << "\n## Rechenzeit 1 s Ton, S2, kleinstes von fuenf\n\n| Taste | Stufe | 8-fach ms | 32-fach ms | Verhaeltnis |\n|---|---|---|---|---|\n";
        for (const int taste : {21, 48, 84, 108})
            for (const int stufe : {-21, 0})
            {
                double ms[2] = {1e300, 1e300};
                for (int i = 0; i < 2; ++i)
                    for (int wdh = 0; wdh < 5; ++wdh)
                    {
                        hammer::HammerAnschlag a;
                        a.taste = taste;
                        a.hz = hz(taste);
                        a.stufe = stufe;
                        a.halten = kRate;
                        a.rate = kRate;
                        a.ueberabtastung = i == 0 ? 8 : 32;
                        std::vector<float> mono;
                        const auto t0 = std::chrono::steady_clock::now();
                        hammer::rechneHammerTon(s, a, mono, kRate);
                        ms[i] = std::min(ms[i], std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count());
                    }
                o << "| " << taste << " | " << stufe << " | " << std::setprecision(1) << ms[0] << " | " << ms[1] << " | "
                  << std::setprecision(2) << ms[1] / ms[0] << " |\n";
            }
    }
    std::ofstream aus(datei.toStdString(), std::ios::binary);
    REQUIRE(aus.good());
    aus << o.str();
    MESSAGE("Diagnose nach " << datei.toStdString());
}
