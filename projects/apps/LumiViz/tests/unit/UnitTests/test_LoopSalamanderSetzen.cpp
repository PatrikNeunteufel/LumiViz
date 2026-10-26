/**
 ****************************************************************************************
 * @file   test_LoopSalamanderSetzen.cpp
 * @brief  Loop, Stufe 31d-4: die Groessen ohne Quelle durch das Modell hindurch setzen
 *         (docs/audio/Spielweisen_Konzept.md §4.1; Entscheide Patrik S151/S152: UF1 a, UF5 a,
 *         D1 a, S1 a, S2 a, S3 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * `LUMIVIZ_31D4_SETZEN=<Ordner>`: Startwert je Taste ist die Messung an Salamander
 * (`werte/salamander_c5.txt`); das Modell wird an den 30 Tasten gerechnet und gemessen
 * (`ModellMessung.hpp`), jeder Parameter je Taste berichtigt (S1 a, mit dem halben Schritt nach
 * S3 a: logarithmische mit √(Ziel / Modell), lineare mit der halben Differenz), hoechstens acht
 * Runden. Die Ziele nach S2 a; nach S3 a die Verstimmung nur bei mindestens zwei Saiten und messbarer
 * Schwebung, die zweite Stufe nicht gesetzt (dreimal Decay, Kipppunkt der Nacharbeit).
 * 31d-7 B (SZ3 a, SM1 a, SM2 a — ersetzt das Vorgehen oben): Stufe 1 setzt je Taste decay, highDecay, lateDecay
 * und Verstimmung (an Tasten mit einer Saite δ) gemeinsam am Modell der Moden (`modelltest::modenWerte`, Nelder–Mead,
 * Faktor 3 um die Messwerte, an der Grenze gemeldet; `stufe1_<n>.txt`); Stufe 2 setzt am gerechneten Ton nur u leise
 * und laut (Tiefe v6–v10, v12–v16, US1 a), γ (Tiefe der Tasten mit einer Saite, US3 a), B und Winkel; beide wechseln
 * hoechstens dreimal. Knick und Tiefe ueber v6–v16 im Bericht.
 * Legt je Runde `runde_<n>.txt` und am Ende `stuetzpunkte.txt` (zum Eintragen in
 * `baueVorgabe()`) in den Ordner. Ohne die Variable meldet der Fall das und prueft nichts.
 *
 * Dazu der dauerhafte Test (UF5 a): die Vorgabe trifft an jeder dritten Taste die Messung — er
 * rechnet rund 50 s und liest vom Arbeitsordner aus (Test-Exe aus dem Projekt-Root starten).
 ****************************************************************************************
 */

#include <doctest.h>

#include "Fluegelmessung.hpp"
#include "ModellMessung.hpp"
#include "NelderMead.hpp"

#include "loop/HammerSaite.hpp"
#include "loop/Tonspeicher.hpp"

#include <QString>
#include <QtGlobal>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <filesystem>
#include <functional>
#include <thread>
#include <fstream>
#include <iomanip>
#include <numbers>
#include <optional>
#include <sstream>
#include <vector>

using namespace lumi::messung;

namespace
{

const std::filesystem::path kWerteDatei = "projects/apps/LumiViz/tests/unit/UnitTests/werte/salamander_c5.txt";

/// B im Bass bis hier (S2 a: Uebergang zum Diskantsteg nach RT-0425), der Winkel bis hier.
constexpr int kBBis = 40;
constexpr int kWinkelBis = 45;
constexpr int kRunden = 8;  // S3 a
/// γ beim Setzen zwischen diesen Grenzen (P7 a erlaubt 0–90°; ab 45° koppeln beide Richtungen gleich).
constexpr double kGammaMin = 0.5;
constexpr double kGammaMax = 40.0;

/// Die hoechste Verstimmung, deren Schwebung am Grundton der Taste noch messbar ist (S3 a): die
/// Schwebung unter 8 Hz und unter der halben Fensterrate der Messung (`Teiltonmessung.hpp`).
double centGrenze(int midi)
{
    const double f = tastenFrequenz(midi);
    const double fensterS = std::clamp(Einstellungen{}.fensterPerioden / f, 0.02, 0.5);
    const double grenzeHz = std::min(Einstellungen{}.schwebungBisHz, 0.5 / fensterS);
    return 1200.0 * std::log2(1.0 + grenzeHz / f);
}

/// Ein Parameter beim Setzen: je Taste der Wert, ob es ein Ziel gibt, ob er trifft.
struct Parameter
{
    std::string name;   ///< wie in `baueVorgabe()`
    bool logarithmisch = true;
    std::vector<double> wert;
    std::vector<bool> ziel;
    std::vector<bool> trifft;
};

/// Der mittlere Abstand der Phantome (Salamander − Modell) ueber die gemeinsamen n ≥ 10, in dB.
std::optional<double> phantomAbstand(const TastenWerte& s, const TastenWerte& m)
{
    double summe = 0.0;
    int zahl = 0;
    for (const auto& [n, db] : s.phantome)
    {
        if (n < 10) continue;
        for (const auto& [nm, dbm] : m.phantome)
            if (nm == n)
            {
                summe += db - dbm;
                ++zahl;
            }
    }
    if (zahl == 0) return std::nullopt;
    return summe / zahl;
}

/// Stuetzpunkte aus den Tasten mit Ziel; `rest` haengt Punkte ausserhalb an (nach Taste sortiert).
lumi::loop::Tastenkurve kurveAus(const std::vector<int>& tasten, const Parameter& p, const std::vector<lumi::loop::TastenWert>& rest = {})
{
    lumi::loop::Tastenkurve k;
    for (std::size_t i = 0; i < tasten.size(); ++i)
        if (p.ziel[i]) k.punkte.push_back({tasten[i], p.wert[i]});
    for (const lumi::loop::TastenWert& w : rest) k.punkte.push_back(w);
    std::sort(k.punkte.begin(), k.punkte.end(), [](const auto& a, const auto& b) { return a.taste < b.taste; });
    return k;
}

/// Trifft die Tiefe der Einbrueche? Vorlaeufig ±20 % oder ±1,5 dB (US5 a: die Toleranz des dauerhaften Tests kommt
/// erst nach den Werten aus der Streuung ueber die Ebenen).
bool tiefeTrifft(const Wert& modell, const Wert& ziel)
{
    return modell.anzahl > 0 && std::abs(modell.wert - ziel.wert) <= std::max(1.5, 0.2 * ziel.wert);
}

/// γ aus der Tiefe an einer Saite (US3 a): zwei Eigenrichtungen, der Hammer stoesst senkrecht — am Steg kommen sie
/// mit cos²γ und sin²γ an; die Einbrueche zweier Moden mit Verhaeltnis r = tan²γ sind 20·log10((1 + r)/(1 − r)) tief.
double gammaAusTiefe(double tiefeDb)
{
    const double q = std::pow(10.0, tiefeDb / 20.0);
    const double r = (q - 1.0) / (q + 1.0);
    return std::clamp(std::atan(std::sqrt(r)) * 180.0 / std::numbers::pi, kGammaMin, kGammaMax);
}

/// γ beim Setzen: zu flach heisst γ zu klein; mit dem halben Schritt die Wurzel des Verhaeltnisses. Ohne erkannte
/// Schwebung im Modell (keine Tiefe) γ um die Haelfte groesser.
double gammaBerichtigt(double gammaGrad, const Wert& modell, double zielDb)
{
    const double neu = modell.anzahl > 0 && modell.wert > 0.0 ? gammaGrad * std::sqrt(zielDb / modell.wert) : gammaGrad * 1.5;
    return std::clamp(neu, kGammaMin, kGammaMax);
}

/// Der Stoss beim Setzen (US1 a): die Tiefe faellt mit u (Raster S154: an 60 49,8 → 21,5 → 15,5 → 13,2 dB fuer
/// u = 0 … 0,6, etwa wie u^−0,44) — zu tief heisst u zu klein; der Schritt u·(Modell/Ziel). Ohne erkannte Schwebung im
/// Modell (flacher als messbar) u halbiert.
double stossBerichtigt(double u, const Wert& modell, double zielDb)
{
    const double neu = modell.anzahl > 0 && modell.wert > 0.0 ? u * (modell.wert / zielDb) : 0.5 * u;
    return std::clamp(neu, 0.01, 1.0);
}

std::string punkteText(const lumi::loop::Tastenkurve& k)
{
    std::ostringstream s;
    s << std::setprecision(4) << '{';
    for (std::size_t i = 0; i < k.punkte.size(); ++i) s << (i ? ", " : "") << '{' << k.punkte[i].taste << ", " << k.punkte[i].wert << '}';
    s << '}';
    return s.str();
}

}  // namespace

TEST_CASE("Loop Salamander 31d-4 das Modell trifft die gesetzten Kurven (UF5 a)")
{
    // Der dauerhafte Test: die Vorgabe der Hammer-Saite, gerechnet wie im Programm mit dem Steinway-Boden, an
    // jeder dritten Taste (den 30 von Salamander) — wo eine Groesse einen Stuetzpunkt traegt, trifft das Modell
    // die Messung: B ±5 %, Phantome ±3 dB. Rund 50 s.
    // AUSGESETZT bis 31d-7 (N2 a, S152): Abklingzeiten (`decay`, `highDecay`, ±10 %) und Verstimmung (±0,5 Cent) —
    // mit der Messung nach Schroeder (N1 a) trifft die Vorgabe aus 31d-4 sie nicht mehr, und das Modell trennt die
    // zwei Stufen erst mit der zweiten Polarisation; danach wird neu gesetzt und hier wieder geprueft.
    // AUSGESETZT bis 31d-7 B Punkt 7 (WK1 a, S153): der Winkel an den Tasten mit Chor (MIDI 33–45) — seit der
    // Laengskraft je Saite (P5 a) liegen die Phantome dort 5–8 dB ueber dem in 31d-4 mit Y/N gesetzten Wert; B
    // setzt den Winkel mit γ und δ neu. An den Tasten mit einer Saite (21–30) rechnet der Ton wie vorher.
    const std::optional<SalamanderWerte> gelesen = liesWerte(kWerteDatei);
    REQUIRE_MESSAGE(gelesen.has_value(), "werte/salamander_c5.txt nicht gefunden — die Test-Exe aus dem Projekt-Root starten");
    const std::vector<TastenWerte> s = tastenWerteAus(*gelesen);
    REQUIRE(s.size() == 30);
    REQUIRE_MESSAGE(lumi::loop::bodenVon(QStringLiteral("lib:steinway_d")), "steinway_d nicht gefunden — aus dem Projekt-Root starten");
    lumi::loop::tonspeicher().leere();
    std::vector<int> tasten;
    for (const TastenWerte& w : s) tasten.push_back(w.midi);
    const lumi::loop::HammerSaite& v = lumi::loop::hammerSaiteVorgabe();
    const auto hatPunkt = [](const lumi::loop::Tastenkurve& k, int taste) {
        return std::any_of(k.punkte.begin(), k.punkte.end(), [taste](const lumi::loop::TastenWert& p) { return p.taste == taste; });
    };
    const std::vector<TastenWerte> m =
        modelltest::verdichteAlle(tasten, modelltest::messeTasten(modelltest::fluegel(QStringLiteral("lib:steinway_d")), tasten));
    int geprueft = 0;
    for (std::size_t i = 0; i < s.size(); ++i)
    {
        const TastenWerte& z = s[i];
        const TastenWerte& mo = m[i];
        CAPTURE(z.midi);
        if (z.midi <= kBBis && hatPunkt(v.inharmonizitaet, z.midi))
        {
            CHECK(mo.b.wert == doctest::Approx(z.b.wert).epsilon(0.05).scale(0.0));
            ++geprueft;
        }
        if (z.midi <= kWinkelBis && hatPunkt(v.stegWinkelGrad, z.midi) && lumi::loop::chorAn(v, z.midi) == 1)
        {
            const std::optional<double> abstand = phantomAbstand(z, mo);
            REQUIRE(abstand.has_value());
            CHECK(std::abs(*abstand) <= 3.0);
            ++geprueft;
        }
    }
    MESSAGE("31d-4: " << geprueft << " gesetzte Werte am Modell gegen Salamander geprueft");
    // B, Winkel an den Tasten mit einer Saite (31d-4); decay, highDecay, detune ausgesetzt bis 31d-7 (N2 a), der
    // Winkel im Chor bis 31d-7 B (WK1 a)
    CHECK(geprueft == 7 + 4);
}

TEST_CASE("Loop Salamander 31d-4 Setzen (LUMIVIZ_31D4_SETZEN)")
{
    // SZ3 a, SM1 a, SM2 a: Stufe 1 am Modell der Moden (je Taste Nelder–Mead ueber decay, highDecay, lateDecay und
    // Verstimmung bzw. δ), Stufe 2 am gerechneten Ton (u leise, u laut, γ an der Tiefe; B und Winkel wie bisher);
    // beide wechseln hoechstens dreimal
    const QString variable = qEnvironmentVariable("LUMIVIZ_31D4_SETZEN");
    if (variable.isEmpty())
    {
        MESSAGE("LUMIVIZ_31D4_SETZEN=<Ordner> setzt die Groessen ohne Quelle durch das Modell hindurch (SZ3 a, SM1 a, SM2 a)");
        return;
    }
    const std::filesystem::path ziel = variable.toStdU16String();
    std::filesystem::create_directories(ziel);
    const std::optional<SalamanderWerte> gelesen = liesWerte(kWerteDatei);
    REQUIRE_MESSAGE(gelesen.has_value(), "werte/salamander_c5.txt nicht gefunden — die Test-Exe aus dem Projekt-Root starten");
    const std::vector<TastenWerte> s = tastenWerteAus(*gelesen);
    REQUIRE(s.size() == 30);
    std::vector<int> tasten;
    for (const TastenWerte& w : s) tasten.push_back(w.midi);
    REQUIRE_MESSAGE(lumi::loop::bodenVon(QStringLiteral("lib:steinway_d")), "steinway_d nicht gefunden — aus dem Projekt-Root starten");
    lumi::loop::tonspeicher().leere();

    const std::size_t n = tasten.size();
    const auto neu = [n](std::string name, bool log) { return Parameter{std::move(name), log, std::vector<double>(n, 0.0), std::vector<bool>(n, false), std::vector<bool>(n, false)}; };
    Parameter decay = neu("abklingS", true);
    Parameter hoch = neu("abklingHoch", true);
    Parameter spaet = neu("abklingSpaetS", true);
    Parameter cent = neu("verstimmungCent", false);
    Parameter b = neu("inharmonizitaet", true);
    Parameter winkel = neu("stegWinkelGrad", false);
    Parameter gamma = neu("polarisationWinkelGrad", false);  // US3 a
    Parameter delta = neu("polarisationCent", false);         // P4 a
    Parameter stossLeise = neu("stossLeise", false);          // US1 a, US2 a
    Parameter stossLaut = neu("stossLaut", false);
    const lumi::loop::HammerSaite vorgabe = lumi::loop::hammerSaiteVorgabe();
    // Ziele und Startwerte (S2 a, US1 a, US3 a, RB1-1 a)
    for (std::size_t i = 0; i < n; ++i)
    {
        const TastenWerte& w = s[i];
        const int chor = lumi::loop::chorAn(vorgabe, w.midi);
        // SM3 a: Start an der fruehen Stufe von Teilton 1, sonst am Ausgleich (der Ausgleich bleibt Pruefgroesse am Ton)
        if (w.t60Frueh.anzahl > 0 && w.t60Frueh.wert > 0.0)
        {
            decay.ziel[i] = true;
            decay.wert[i] = w.t60Frueh.wert;
        }
        else if (w.t60Ausgleich.anzahl > 0 && w.t60Ausgleich.wert > 0.0)
        {
            decay.ziel[i] = true;
            decay.wert[i] = w.t60Ausgleich.wert;
        }
        if (w.highDecay.anzahl > 0 && w.highDecay.wert > 0.0)
        {
            hoch.ziel[i] = true;
            hoch.wert[i] = std::clamp(w.highDecay.wert, 0.001, 1.0);
        }
        if (w.zweiStufenAnteil.wert >= 0.5 && w.t60Spaet.anzahl > 0 && w.t60Spaet.wert > 0.0)
        {
            spaet.ziel[i] = true;
            spaet.wert[i] = std::max(w.t60Spaet.wert, decay.wert[i]);
        }
        if (w.schwebungCent.anzahl > 0 && w.schwebungCent.wert <= centGrenze(w.midi))
        {
            Parameter& p = chor >= 2 ? cent : delta;
            p.ziel[i] = true;
            p.wert[i] = std::clamp(w.schwebungCent.wert, 0.0, 20.0);
        }
        if (w.midi <= kBBis && w.b.anzahl > 0 && w.b.wert > 0.0)
        {
            b.ziel[i] = true;
            b.wert[i] = w.b.wert;
        }
        if (w.midi <= kWinkelBis && !w.phantome.empty())
        {
            winkel.ziel[i] = true;
            winkel.wert[i] = 2.0;
        }
        if (chor == 1 && w.tiefeDb.anzahl > 0 && w.tiefeDb.wert > 0.0)
        {
            gamma.ziel[i] = true;
            gamma.wert[i] = gammaAusTiefe(w.tiefeDb.wert);
        }
        if (chor >= 2 && w.tiefeLeise.anzahl > 0 && w.tiefeLeise.wert > 0.0)
        {
            stossLeise.ziel[i] = true;
            stossLeise.wert[i] = 0.25;
        }
        if (chor >= 2 && w.tiefeLaut.anzahl > 0 && w.tiefeLaut.wert > 0.0)
        {
            stossLaut.ziel[i] = true;
            stossLaut.wert[i] = 0.25;
        }
    }
    const auto saiteAus = [&]() {
        lumi::loop::HammerSaite saite = vorgabe;
        saite.abklingS = kurveAus(tasten, decay);
        saite.abklingHoch = kurveAus(tasten, hoch);
        saite.abklingSpaetS = kurveAus(tasten, spaet);
        saite.verstimmungCent = kurveAus(tasten, cent);
        std::vector<lumi::loop::TastenWert> rigaud;
        for (const lumi::loop::TastenWert& p : vorgabe.inharmonizitaet.punkte)
            if (p.taste > kBBis) rigaud.push_back(p);
        saite.inharmonizitaet = kurveAus(tasten, b, rigaud);
        const auto letzterWert = [&](const Parameter& p, double sonst) {
            double l = sonst;
            for (std::size_t i = 0; i < n; ++i)
                if (p.ziel[i]) l = p.wert[i];
            return l;
        };
        saite.stegWinkelGrad = kurveAus(tasten, winkel, {{lumi::loop::kTasteMax, letzterWert(winkel, 2.0)}});
        saite.polarisationWinkelGrad = kurveAus(tasten, gamma, {{lumi::loop::kTasteMax, letzterWert(gamma, 0.0)}});
        saite.polarisationCent = kurveAus(tasten, delta, {{lumi::loop::kTasteMax, letzterWert(delta, 0.0)}});
        saite.stossLeise = kurveAus(tasten, stossLeise);
        saite.stossLaut = kurveAus(tasten, stossLaut);
        lumi::loop::richteHammerSaite(saite);
        return saite;
    };

    // --- Stufe 1 (SM2 a): je Taste gemeinsam am Modell der Moden ---
    struct Ergebnis1
    {
        std::vector<double> x;
        double rest = 0.0;
        int auswertungen = 0;
        std::string grenzen;  ///< Parameter, die an ihrer Grenze liegen
    };
    // die freien Groessen einer Taste: decay, highDecay, lateDecay (log), Verstimmung oder δ (linear)
    const auto stufe1 = [&](std::size_t i, const lumi::loop::HammerSaite& basis, double startFaktor) {
        const TastenWerte& z = s[i];
        const int midi = tasten[i];
        Parameter& verstimmung = cent.ziel[i] ? cent : delta;
        std::vector<Parameter*> frei;
        for (Parameter* p : {&decay, &hoch, &spaet, &verstimmung})
            if (p->ziel[i]) frei.push_back(p);
        std::vector<double> unten, oben, x0;
        for (Parameter* p : frei)
        {
            const double v = p->wert[i];
            if (p->logarithmisch)
            {
                const double grenzeOben = p == &hoch ? std::min(1.0, 3.0 * v) : 3.0 * v;
                unten.push_back(std::log(v / 3.0));
                oben.push_back(std::log(grenzeOben));
                x0.push_back(std::log(v) + std::log(startFaktor));
            }
            else
            {
                unten.push_back(0.0);
                oben.push_back(std::min(20.0, std::max(1.0, 3.0 * v)));
                x0.push_back(v * startFaktor);
            }
        }
        const auto saiteMit = [&](const std::vector<double>& x) {
            lumi::loop::HammerSaite saite = basis;
            for (std::size_t j = 0; j < frei.size(); ++j)
            {
                const double v = frei[j]->logarithmisch ? std::exp(x[j]) : x[j];
                lumi::loop::Tastenkurve* k = frei[j] == &decay ? &saite.abklingS
                                             : frei[j] == &hoch ? &saite.abklingHoch
                                             : frei[j] == &spaet ? &saite.abklingSpaetS
                                             : frei[j] == &cent ? &saite.verstimmungCent
                                                                : &saite.polarisationCent;
                k->abweichungen = {{midi, v}};
            }
            return saite;
        };
        const auto rest = [&](const std::vector<double>& xRoh) {
            std::vector<double> x = xRoh;
            for (std::size_t j = 0; j < x.size(); ++j) x[j] = std::clamp(x[j], unten[j], oben[j]);
            const TastenWerte mo = modelltest::modenWerte(saiteMit(x), midi, -10);
            double r = 0.0;
            const auto logAbstand = [&r](const Wert& m, const Wert& zw) {
                if (zw.anzahl == 0 || zw.wert <= 0.0) return;  // kein Ziel
                if (m.anzahl == 0 || m.wert <= 0.0)
                    r += 4.0;  // nicht messbar am Modell: wie ein Faktor e²
                else
                    r += std::pow(std::log(m.wert / zw.wert), 2.0);
            };
            // SM3 a: je Teilton 1–3 die fruehe Stufe, die spaete (wo Salamander meist zwei zeigt) und die Schwebung
            logAbstand(mo.t60Frueh, z.t60Frueh);
            if (z.zweiStufenAnteil.wert >= 0.5) logAbstand(mo.zweiStufenAnteil.wert >= 0.5 ? mo.t60Spaet : Wert{}, z.t60Spaet);
            for (std::size_t j = 0; j < 2; ++j)
            {
                logAbstand(mo.t60FruehK[j], z.t60FruehK[j]);
                if (z.zweiStufenAnteilK[j].wert >= 0.5)
                    logAbstand(mo.zweiStufenAnteilK[j].wert >= 0.5 ? mo.t60SpaetK[j] : Wert{}, z.t60SpaetK[j]);
            }
            if (verstimmung.ziel[i])
                r += std::pow(((mo.schwebungCent.anzahl > 0 ? mo.schwebungCent.wert : 0.0) - z.schwebungCent.wert) / 5.0, 2.0);  // 0,5 Cent wie 10 %
            return r;
        };
        Ergebnis1 e;
        if (frei.empty()) return e;
        std::vector<double> schritte;
        for (std::size_t j = 0; j < frei.size(); ++j) schritte.push_back(frei[j]->logarithmisch ? 0.3 : 0.5);
        const NelderMead nm = nelderMead(rest, x0, schritte, 150);
        e.x = nm.x;
        for (std::size_t j = 0; j < e.x.size(); ++j)
        {
            e.x[j] = std::clamp(e.x[j], unten[j], oben[j]);
            if (e.x[j] <= unten[j] + 1e-6 || e.x[j] >= oben[j] - 1e-6) e.grenzen += ' ' + frei[j]->name;
        }
        e.rest = nm.rest;
        e.auswertungen = nm.auswertungen;
        return e;
    };
    const auto uebernehme = [&](std::size_t i, const Ergebnis1& e) {
        Parameter& verstimmung = cent.ziel[i] ? cent : delta;
        std::size_t j = 0;
        for (Parameter* p : {&decay, &hoch, &spaet, &verstimmung})
            if (p->ziel[i]) p->wert[i] = p->logarithmisch ? std::exp(e.x[j++]) : e.x[j++];
        if (spaet.ziel[i] && decay.ziel[i]) spaet.wert[i] = std::max(spaet.wert[i], decay.wert[i]);
    };

    // SM3 a, die zweite Probe: Stufe 1 an sechs Tasten mit zwei Startpunkten, dann der Ton mit den Werten des ersten —
    // je Teilton 1–3 Salamander | Summe der Moden | Ton
    if (qEnvironmentVariableIsSet("LUMIVIZ_31D7_PROBE2"))
    {
        const lumi::loop::HammerSaite basis = saiteAus();
        std::ostringstream probe;
        probe << std::fixed << std::setprecision(2)
              << "Zweite Probe (SM3 a): je Taste x (ln decay, ln highDecay, ln lateDecay, Cent) und Rest fuer zwei Startpunkte; "
                 "je Teilton T60 frueh / spaet (Stufen) und Schwebung in Cent: Salamander | Moden | Ton\n";
        for (const int midi : {33, 48, 60, 66, 81, 84})
        {
            const auto i = static_cast<std::size_t>(std::find(tasten.begin(), tasten.end(), midi) - tasten.begin());
            REQUIRE(i < n);
            const Ergebnis1 erster = stufe1(i, basis, 1.0);
            const Ergebnis1 zweiter = stufe1(i, basis, 1.6);
            probe << "MIDI " << midi << ": Rest " << erster.rest << " / " << zweiter.rest << " — x";
            for (const double v : erster.x) probe << ' ' << v;
            probe << " /";
            for (const double v : zweiter.x) probe << ' ' << v;
            probe << " — an der Grenze:" << erster.grenzen << " /" << zweiter.grenzen << '\n';
            uebernehme(i, erster);
            lumi::loop::HammerSaite saite = basis;
            const Parameter& verstimmung = cent.ziel[i] ? cent : delta;
            if (decay.ziel[i]) saite.abklingS.abweichungen = {{midi, decay.wert[i]}};
            if (hoch.ziel[i]) saite.abklingHoch.abweichungen = {{midi, hoch.wert[i]}};
            if (spaet.ziel[i]) saite.abklingSpaetS.abweichungen = {{midi, spaet.wert[i]}};
            if (verstimmung.ziel[i])
                (cent.ziel[i] ? saite.verstimmungCent : saite.polarisationCent).abweichungen = {{midi, verstimmung.wert[i]}};
            probe << "  Werte: decay " << decay.wert[i] << " s, highDecay " << hoch.wert[i] << ", lateDecay " << spaet.wert[i]
                  << " s, Verstimmung " << verstimmung.wert[i] << " Cent\n";
            const TastenWerte ausModen = modelltest::modenWerte(saite, midi, -10);
            lumi::loop::Instrument instrument = modelltest::fluegel(QStringLiteral("lib:steinway_d"), &saite);
            instrument.tastatur->ungedaempftAb = 89;
            const TastenWerte ausTon = modelltest::verdichteAlle({midi}, modelltest::messeTasten(instrument, {midi})).front();
            const TastenWerte& z = s[i];
            const auto stufe = [](const Wert& frueh, const Wert& spaet, const Wert& anteil) {
                std::ostringstream t;
                t << std::fixed << std::setprecision(2) << frueh.wert << " / " << (anteil.wert >= 0.5 ? spaet.wert : 0.0) << " ("
                  << anteil.wert << ')';
                return t.str();
            };
            for (const auto& [name, w] : {std::pair{"Salamander", &z}, std::pair{"Moden     ", &ausModen}, std::pair{"Ton       ", &ausTon}})
                probe << "  " << name << ": T1 " << stufe(w->t60Frueh, w->t60Spaet, w->zweiStufenAnteil) << " | T2 "
                      << stufe(w->t60FruehK[0], w->t60SpaetK[0], w->zweiStufenAnteilK[0]) << " | T3 "
                      << stufe(w->t60FruehK[1], w->t60SpaetK[1], w->zweiStufenAnteilK[1]) << " | Cent " << w->schwebungCent.wert
                      << " | Ausgleich " << w->t60Ausgleich.wert << " | highDecay " << w->highDecay.wert << " | Tiefe " << w->tiefeDb.wert
                      << '\n';
        }
        std::ofstream(ziel / "probe2.txt", std::ios::binary) << probe.str();
        MESSAGE(probe.str());
        return;
    }

    std::ostringstream verlauf;
    std::ostringstream vermerkt;
    std::vector<TastenWerte> m;
    int runde = 0;
    for (int wechsel = 1; wechsel <= 3; ++wechsel)
    {
        // Stufe 1 auf acht Threads ueber die Tasten
        const lumi::loop::HammerSaite basis = saiteAus();
        std::vector<Ergebnis1> ergebnisse(n);
        std::atomic<std::size_t> naechste{0};
        std::vector<std::thread> arbeiter;
        const unsigned zahl = std::max(1u, std::min(8u, std::thread::hardware_concurrency()));
        for (unsigned a = 0; a < zahl; ++a)
            arbeiter.emplace_back([&] {
                for (std::size_t i = naechste++; i < n; i = naechste++) ergebnisse[i] = stufe1(i, basis, 1.0);
            });
        for (std::thread& t : arbeiter) t.join();
        std::ostringstream bericht1;
        bericht1 << std::fixed << std::setprecision(3) << "Wechsel " << wechsel
                 << ", Stufe 1 (Modell der Moden): je Taste decay | highDecay | lateDecay | Verstimmung bzw. delta | Rest | Auswertungen | an der Grenze\n";
        for (std::size_t i = 0; i < n; ++i)
        {
            uebernehme(i, ergebnisse[i]);
            const Parameter& verstimmung = cent.ziel[i] ? cent : delta;
            bericht1 << tasten[i] << " | " << (decay.ziel[i] ? decay.wert[i] : 0.0) << " | " << (hoch.ziel[i] ? hoch.wert[i] : 0.0) << " | "
                     << (spaet.ziel[i] ? spaet.wert[i] : 0.0) << " | " << (verstimmung.ziel[i] ? verstimmung.wert[i] : 0.0) << " | "
                     << ergebnisse[i].rest << " | " << ergebnisse[i].auswertungen << " |" << ergebnisse[i].grenzen << '\n';
        }
        // SM2 a, Kipppunkt: im ersten Wechsel an drei Tasten ein zweiter Startpunkt — eindeutig?
        if (wechsel == 1)
        {
            bericht1 << "Zweiter Startpunkt (Faktor 1,6 auf alle Startwerte), dieselbe Suche:\n";
            for (const int midi : {33, 60, 81})
            {
                const auto it = std::find(tasten.begin(), tasten.end(), midi);
                if (it == tasten.end()) continue;
                const auto i = static_cast<std::size_t>(it - tasten.begin());
                const Ergebnis1 zweiter = stufe1(i, basis, 1.6);
                bericht1 << midi << ": Rest " << zweiter.rest << " (erster " << ergebnisse[i].rest << ") — x";
                for (const double v : zweiter.x) bericht1 << ' ' << v;
                bericht1 << " gegen";
                for (const double v : ergebnisse[i].x) bericht1 << ' ' << v;
                bericht1 << '\n';
            }
        }
        std::ofstream(ziel / ("stufe1_" + std::to_string(wechsel) + ".txt"), std::ios::binary) << bericht1.str();
        verlauf << bericht1.str() << '\n';

        // --- Stufe 2 (SM1 a): am gerechneten Ton nur u leise, u laut, γ — dazu B und Winkel ---
        const std::vector<double> uVorher = stossLeise.wert;
        const std::vector<double> lautVorher = stossLaut.wert;
        const std::vector<double> gammaVorher = gamma.wert;
        for (int r = 1; r <= kRunden; ++r)
        {
            ++runde;
            const lumi::loop::HammerSaite saite = saiteAus();
            lumi::loop::Instrument instrument = modelltest::fluegel(QStringLiteral("lib:steinway_d"), &saite);
            instrument.tastatur->ungedaempftAb = 89;  // D1 a
            m = modelltest::verdichteAlle(tasten, modelltest::messeTasten(instrument, tasten));

            std::ostringstream tabelle;
            tabelle << std::fixed << std::setprecision(3) << "Wechsel " << wechsel << ", Stufe 2, Runde " << r << " (" << runde << ")"
                    << " — je Taste Parameter | Salamander | Modell (* = trifft: T60 und highDecay ±10 %, B ±5 %, Cent ±0,5, "
                       "Phantome ±3 dB, Tiefe vorlaeufig ±20 % oder ±1,5 dB)\n";
            bool alle = true;
            for (std::size_t i = 0; i < n; ++i)
            {
                const TastenWerte& z = s[i];
                const TastenWerte& mo = m[i];
                const auto relativ = [](double ist, double soll, double grenze) { return soll > 0.0 && std::abs(ist / soll - 1.0) <= grenze; };
                // die Groessen aus Stufe 1 nur geprueft (am Ton)
                decay.trifft[i] = decay.ziel[i] && mo.t60Ausgleich.anzahl > 0 && relativ(mo.t60Ausgleich.wert, z.t60Ausgleich.wert, 0.10);
                hoch.trifft[i] = hoch.ziel[i] && mo.highDecay.anzahl > 0 && relativ(mo.highDecay.wert, z.highDecay.wert, 0.10);
                spaet.trifft[i] = spaet.ziel[i] && mo.t60Spaet.anzahl > 0 && relativ(mo.t60Spaet.wert, z.t60Spaet.wert, 0.10);
                const double centModell = mo.schwebungCent.anzahl > 0 ? mo.schwebungCent.wert : 0.0;
                cent.trifft[i] = cent.ziel[i] && std::abs(centModell - z.schwebungCent.wert) <= 0.5;
                delta.trifft[i] = delta.ziel[i] && std::abs(centModell - z.schwebungCent.wert) <= 0.5;
                // die Groessen der Stufe 2
                b.trifft[i] = b.ziel[i] && mo.b.anzahl > 0 && relativ(mo.b.wert, z.b.wert, 0.05);
                const std::optional<double> abstand = phantomAbstand(z, mo);
                winkel.trifft[i] = winkel.ziel[i] && abstand && std::abs(*abstand) <= 3.0;
                gamma.trifft[i] = gamma.ziel[i] && tiefeTrifft(mo.tiefeDb, z.tiefeDb);
                stossLeise.trifft[i] = stossLeise.ziel[i] && tiefeTrifft(mo.tiefeLeise, z.tiefeLeise);
                stossLaut.trifft[i] = stossLaut.ziel[i] && tiefeTrifft(mo.tiefeLaut, z.tiefeLaut);
                for (const Parameter* p : {&b, &winkel, &gamma, &stossLeise, &stossLaut})
                    if (p->ziel[i] && !p->trifft[i]) alle = false;

                tabelle << z.midi << " | decay " << decay.wert[i] << ' ' << z.t60Ausgleich.wert << ' ' << mo.t60Ausgleich.wert
                        << (decay.trifft[i] ? '*' : ' ') << " | high " << hoch.wert[i] << ' ' << z.highDecay.wert << ' ' << mo.highDecay.wert
                        << (hoch.trifft[i] ? '*' : ' ') << " | late " << (spaet.ziel[i] ? spaet.wert[i] : 0.0) << ' ' << z.t60Spaet.wert << " ("
                        << z.zweiStufenAnteil.wert << ") " << mo.t60Spaet.wert << " (" << mo.zweiStufenAnteil.wert << ")"
                        << (spaet.trifft[i] ? '*' : ' ') << " | cent " << (cent.ziel[i] ? cent.wert[i] : delta.wert[i]) << ' '
                        << z.schwebungCent.wert << ' ' << centModell << (cent.trifft[i] || delta.trifft[i] ? '*' : ' ');
                if (b.ziel[i]) tabelle << " | B " << 1e4 * b.wert[i] << ' ' << 1e4 * z.b.wert << ' ' << 1e4 * mo.b.wert << (b.trifft[i] ? '*' : ' ');
                if (winkel.ziel[i]) tabelle << " | Winkel " << winkel.wert[i] << " Abstand " << (abstand ? *abstand : 0.0) << (winkel.trifft[i] ? '*' : ' ');
                if (gamma.ziel[i]) tabelle << " | gamma " << gamma.wert[i] << (gamma.trifft[i] ? '*' : ' ');
                if (stossLeise.ziel[i])
                    tabelle << " | u leise " << stossLeise.wert[i] << ' ' << z.tiefeLeise.wert << ' ' << mo.tiefeLeise.wert << (stossLeise.trifft[i] ? '*' : ' ');
                if (stossLaut.ziel[i])
                    tabelle << " | u laut " << stossLaut.wert[i] << ' ' << z.tiefeLaut.wert << ' ' << mo.tiefeLaut.wert << (stossLaut.trifft[i] ? '*' : ' ');
                tabelle << " | Tiefe " << z.tiefeDb.wert << ' ' << mo.tiefeDb.wert << " | Knick " << z.knickS.wert << ' ' << mo.knickS.wert << '\n';
            }
            for (const Parameter* p : {&decay, &hoch, &spaet, &cent, &delta, &b, &winkel, &gamma, &stossLeise, &stossLaut})
            {
                int ziele = 0;
                int treffer = 0;
                for (std::size_t i = 0; i < n; ++i)
                {
                    ziele += p->ziel[i] ? 1 : 0;
                    treffer += p->trifft[i] ? 1 : 0;
                }
                tabelle << p->name << ": " << treffer << " von " << ziele << " Tasten treffen\n";
            }
            std::ofstream(ziel / ("runde_" + std::to_string(runde) + ".txt"), std::ios::binary) << tabelle.str();
            verlauf << tabelle.str() << '\n';
            if (alle || r == kRunden) break;
            for (std::size_t i = 0; i < n; ++i)
            {
                const TastenWerte& z = s[i];
                const TastenWerte& mo = m[i];
                if (b.ziel[i] && !b.trifft[i] && mo.b.anzahl > 0 && mo.b.wert > 0.0)
                    b.wert[i] = std::clamp(b.wert[i] * std::sqrt(z.b.wert / mo.b.wert), 1e-6, 0.1);
                if (const std::optional<double> abstand = phantomAbstand(z, mo); winkel.ziel[i] && !winkel.trifft[i] && abstand)
                {
                    const double rad = winkel.wert[i] * std::numbers::pi / 180.0;
                    winkel.wert[i] = std::clamp(std::atan(std::tan(rad) * std::pow(10.0, *abstand / 40.0)) * 180.0 / std::numbers::pi, 0.0, 10.0);
                }
                if (gamma.ziel[i] && !gamma.trifft[i]) gamma.wert[i] = gammaBerichtigt(gamma.wert[i], mo.tiefeDb, z.tiefeDb.wert);
                if (stossLeise.ziel[i] && !stossLeise.trifft[i])
                    stossLeise.wert[i] = stossBerichtigt(stossLeise.wert[i], mo.tiefeLeise, z.tiefeLeise.wert);
                if (stossLaut.ziel[i] && !stossLaut.trifft[i])
                    stossLaut.wert[i] = stossBerichtigt(stossLaut.wert[i], mo.tiefeLaut, z.tiefeLaut.wert);
            }
        }
        // ruhig? — u und γ aenderten sich um hoechstens 10 %
        bool ruhig = true;
        for (std::size_t i = 0; i < n; ++i)
            for (const auto& [neuWert, alt] : {std::pair{stossLeise.wert[i], uVorher[i]}, std::pair{stossLaut.wert[i], lautVorher[i]},
                                                std::pair{gamma.wert[i], gammaVorher[i]}})
                if (alt > 0.0 && std::abs(neuWert / alt - 1.0) > 0.10) ruhig = false;
        verlauf << "Wechsel " << wechsel << (ruhig ? ": u und gamma ruhig — Ende\n" : ": u oder gamma noch bewegt\n");
        if (ruhig) break;
    }

    // vermerkt, was am Ende am Ton nicht trifft (UF5 a); eingetragen wird alles, was gesetzt ist — die Pruefung am Ton
    // steht in der letzten Runde
    for (const Parameter* p : {&decay, &hoch, &spaet, &cent, &delta, &b, &winkel, &gamma, &stossLeise, &stossLaut})
        for (std::size_t i = 0; i < n; ++i)
            if (p->ziel[i] && !p->trifft[i]) vermerkt << ' ' << p->name << '@' << tasten[i];

    const lumi::loop::HammerSaite saite = saiteAus();
    std::ostringstream punkte;
    punkte << "// gemessen an Salamander (Yamaha C5), gesetzt durch das Modell hindurch (31d-7 B: SZ3 a, SM1 a, SM2 a — Stufe 1 am "
              "Modell der Moden, Stufe 2 am Ton), "
           << runde << " Runden am Ton; die letzte Runde ist mit diesen Werten gemessen\n"
           << "s.abklingS = kurveAusPunkten(" << punkteText(saite.abklingS) << ");\n"
           << "s.abklingSpaetS = kurveAusPunkten(" << punkteText(saite.abklingSpaetS) << ");\n"
           << "s.abklingHoch = kurveAusPunkten(" << punkteText(saite.abklingHoch) << ");\n"
           << "s.verstimmungCent = kurveAusPunkten(" << punkteText(saite.verstimmungCent) << ");\n"
           << "s.inharmonizitaet = kurveAusPunkten(" << punkteText(saite.inharmonizitaet) << ");\n"
           << "s.stegWinkelGrad = kurveAusPunkten(" << punkteText(saite.stegWinkelGrad) << ");\n"
           << "s.polarisationWinkelGrad = kurveAusPunkten(" << punkteText(saite.polarisationWinkelGrad) << ");\n"
           << "s.polarisationCent = kurveAusPunkten(" << punkteText(saite.polarisationCent) << ");\n"
           << "s.stossLeise = kurveAusPunkten(" << punkteText(saite.stossLeise) << ");\n"
           << "s.stossLaut = kurveAusPunkten(" << punkteText(saite.stossLaut) << ");\n"
           << "// am Ton ohne Treffer (UF5 a: vermerkt):" << vermerkt.str() << '\n';
    std::ofstream(ziel / "stuetzpunkte.txt", std::ios::binary) << punkte.str();
    MESSAGE(verlauf.str() << punkte.str());
}
