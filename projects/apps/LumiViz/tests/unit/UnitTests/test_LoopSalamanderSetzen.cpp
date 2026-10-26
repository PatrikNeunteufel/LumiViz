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
 * 31d-7 B (P4 a): dazu γ aus dem Knick von Teilton 1 (wo Salamander in mindestens der Haelfte der Messungen
 * zwei Stufen zeigt) und δ aus der Schwebung an den Tasten mit einer Saite, darueber der letzte Wert; die
 * Tiefe der Einbrueche (TB1 a) steht nur im Bericht.
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

#include "loop/HammerSaite.hpp"
#include "loop/Tonspeicher.hpp"

#include <QString>
#include <QtGlobal>

#include <algorithm>
#include <cmath>
#include <filesystem>
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

/// Der Unterschied der Abklingraten beider Stufen von Teilton 1 in 1/s (Amplitude; T60 = 60 dB).
double stufenAbstand(const TastenWerte& w)
{
    return std::log(1000.0) * (1.0 / w.t60Frueh.wert - 1.0 / w.t60Spaet.wert);
}

/// γ aus dem Knick (P4 a): die schwach gekoppelte Richtung traegt sin²γ, die stark gekoppelte cos²γ der
/// Energie; der Knick liegt, wo beide gleich laut sind — tan γ = exp(−Δσ·t_Knick).
double gammaAusKnick(double knickS, double abstand)
{
    return std::atan(std::exp(-abstand * knickS)) * 180.0 / std::numbers::pi;
}

/// γ beim Setzen: der Knick des Modells um (t_Modell − t_Ziel) zu spaet heisst tan γ um exp(Δσ·(t_Modell − t_Ziel))
/// zu klein; mit dem halben Schritt (S3 a) die Wurzel davon. Grenzen: γ = 45° vertieft die Einbrueche (Befund S153).
double gammaBerichtigt(double gammaGrad, double knickModell, double knickZiel, double abstand)
{
    const double rad = gammaGrad * std::numbers::pi / 180.0;
    const double faktor = std::exp(0.5 * abstand * (knickModell - knickZiel));
    return std::clamp(std::atan(std::tan(rad) * faktor) * 180.0 / std::numbers::pi, kGammaMin, kGammaMax);
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
    const QString variable = qEnvironmentVariable("LUMIVIZ_31D4_SETZEN");
    if (variable.isEmpty())
    {
        MESSAGE("LUMIVIZ_31D4_SETZEN=<Ordner> setzt die Groessen ohne Quelle durch das Modell hindurch (S1 a, S2 a)");
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
    Parameter gamma = neu("polarisationWinkelGrad", false);  // P4 a
    Parameter delta = neu("polarisationCent", false);
    // S2 a: Ziele und Startwerte
    for (std::size_t i = 0; i < n; ++i)
    {
        const TastenWerte& w = s[i];
        if (w.t60Ausgleich.anzahl > 0 && w.t60Ausgleich.wert > 0.0)
        {
            decay.ziel[i] = true;
            decay.wert[i] = w.t60Ausgleich.wert;
        }
        if (w.highDecay.anzahl > 0 && w.highDecay.wert > 0.0)
        {
            hoch.ziel[i] = true;
            hoch.wert[i] = std::clamp(w.highDecay.wert, 0.001, 1.0);
        }
        // N1 a: die zweite Stufe wieder gesetzt (S2 a: wo Salamander in mindestens der Haelfte der Messungen zwei zeigt)
        if (w.zweiStufenAnteil.wert >= 0.5 && w.t60Spaet.anzahl > 0 && w.t60Spaet.wert > 0.0)
        {
            spaet.ziel[i] = true;
            spaet.wert[i] = std::max(w.t60Spaet.wert, decay.wert[i]);
        }
        // S3 a: Verstimmung nur, wo das Modell mindestens zwei Saiten hat und die Schwebung messbar bleibt
        if (w.schwebungCent.anzahl > 0 && lumi::loop::chorAn(lumi::loop::hammerSaiteVorgabe(), w.midi) >= 2 &&
            w.schwebungCent.wert <= centGrenze(w.midi))
        {
            cent.ziel[i] = true;
            cent.wert[i] = std::clamp(w.schwebungCent.wert, 0.0, 20.0);
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
        // P4 a: γ aus dem Knick von Teilton 1, wo Salamander meist zwei Stufen zeigt
        if (w.zweiStufenAnteil.wert >= 0.5 && w.knickS.anzahl > 0 && w.knickS.wert > 0.0 && w.t60Spaet.wert > w.t60Frueh.wert &&
            w.t60Frueh.wert > 0.0)
        {
            gamma.ziel[i] = true;
            gamma.wert[i] = std::clamp(gammaAusKnick(w.knickS.wert, stufenAbstand(w)), kGammaMin, kGammaMax);
        }
        // P4 a: δ aus der Schwebung an den Tasten mit einer Saite (darueber der letzte Wert)
        if (w.schwebungCent.anzahl > 0 && lumi::loop::chorAn(lumi::loop::hammerSaiteVorgabe(), w.midi) == 1 &&
            w.schwebungCent.wert <= centGrenze(w.midi))
        {
            delta.ziel[i] = true;
            delta.wert[i] = std::clamp(w.schwebungCent.wert, 0.0, 20.0);
        }
    }
    const lumi::loop::HammerSaite vorgabe = lumi::loop::hammerSaiteVorgabe();
    const auto saiteAus = [&]() {
        lumi::loop::HammerSaite saite = vorgabe;
        saite.abklingS = kurveAus(tasten, decay);
        saite.abklingHoch = kurveAus(tasten, hoch);
        saite.abklingSpaetS = kurveAus(tasten, spaet);  // N1 a; zwischen den Zielen interpoliert (S2 a)
        saite.verstimmungCent = kurveAus(tasten, cent);
        // B: im Bass gesetzt, darueber die Punkte der Vorgabe (Rigaud)
        std::vector<lumi::loop::TastenWert> rigaud;
        for (const lumi::loop::TastenWert& p : vorgabe.inharmonizitaet.punkte)
            if (p.taste > kBBis) rigaud.push_back(p);
        saite.inharmonizitaet = kurveAus(tasten, b, rigaud);
        // der Winkel: im Bass gesetzt, darueber der letzte Wert bis C8
        double letzter = 2.0;
        for (std::size_t i = 0; i < n; ++i)
            if (winkel.ziel[i]) letzter = winkel.wert[i];
        saite.stegWinkelGrad = kurveAus(tasten, winkel, {{lumi::loop::kTasteMax, letzter}});
        // P4 a: γ zwischen den Zielen interpoliert, an den Enden fortgefuehrt; δ ueber den Tasten mit einer Saite
        // der letzte Wert bis C8
        saite.polarisationWinkelGrad = kurveAus(tasten, gamma);
        double letztesDelta = 0.0;
        for (std::size_t i = 0; i < n; ++i)
            if (delta.ziel[i]) letztesDelta = delta.wert[i];
        saite.polarisationCent = kurveAus(tasten, delta, {{lumi::loop::kTasteMax, letztesDelta}});
        lumi::loop::richteHammerSaite(saite);
        return saite;
    };

    // Durchgaenge: nach den Runden fallen Tasten ohne Treffer heraus (UF5 a: sie tragen nicht bei), und mit genau
    // dieser Kurve wird weiter berichtigt — ueber die Kopplung im Chor aendert eine fehlende Verstimmung auch die
    // Abklingzeiten der Taste (Befund S152). Eingetragen wird die Kurve des letzten Durchgangs, gemessen.
    std::ostringstream verlauf;
    std::ostringstream vermerkt;
    std::vector<TastenWerte> m;
    int runde = 0;
    int durchgang = 1;
    bool gemessen = false;
    for (; durchgang <= 3; ++durchgang)
    {
        for (int r = 1; r <= kRunden; ++r)
        {
            ++runde;
            const lumi::loop::HammerSaite saite = saiteAus();
            lumi::loop::Instrument instrument = modelltest::fluegel(QStringLiteral("lib:steinway_d"), &saite);
            instrument.tastatur->ungedaempftAb = 89;  // D1 a
            m = modelltest::verdichteAlle(tasten, modelltest::messeTasten(instrument, tasten));

            std::ostringstream tabelle;
            tabelle << std::fixed << std::setprecision(3) << "Durchgang " << durchgang << ", Runde " << r << " (" << runde << ")"
                    << " — je Taste Parameter | Salamander | Modell (* = trifft: T60 und highDecay ±10 %, B ±5 %, Cent ±0,5, "
                       "Phantome ±3 dB)\n";
            bool alle = true;
            for (std::size_t i = 0; i < n; ++i)
            {
                const TastenWerte& z = s[i];
                const TastenWerte& mo = m[i];
                const auto relativ = [](double ist, double soll, double grenze) { return soll > 0.0 && std::abs(ist / soll - 1.0) <= grenze; };
                decay.trifft[i] = decay.ziel[i] && mo.t60Ausgleich.anzahl > 0 && relativ(mo.t60Ausgleich.wert, z.t60Ausgleich.wert, 0.10);
                hoch.trifft[i] = hoch.ziel[i] && mo.highDecay.anzahl > 0 && relativ(mo.highDecay.wert, z.highDecay.wert, 0.10);
                const double centModell = mo.schwebungCent.anzahl > 0 ? mo.schwebungCent.wert : 0.0;
                cent.trifft[i] = cent.ziel[i] && std::abs(centModell - z.schwebungCent.wert) <= 0.5;
                spaet.trifft[i] = spaet.ziel[i] && mo.t60Spaet.anzahl > 0 && relativ(mo.t60Spaet.wert, z.t60Spaet.wert, 0.10);
                b.trifft[i] = b.ziel[i] && mo.b.anzahl > 0 && relativ(mo.b.wert, z.b.wert, 0.05);
                const std::optional<double> abstand = phantomAbstand(z, mo);
                winkel.trifft[i] = winkel.ziel[i] && abstand && std::abs(*abstand) <= 3.0;
                // γ: der Knick des Modells auf ±20 % oder ±0,2 s (die Messung sucht ihn im Raster der Huellkurve)
                gamma.trifft[i] = gamma.ziel[i] && mo.knickS.anzahl > 0 && mo.zweiStufenAnteil.wert >= 0.5 &&
                                  std::abs(mo.knickS.wert - z.knickS.wert) <= std::max(0.2, 0.2 * z.knickS.wert);
                delta.trifft[i] = delta.ziel[i] && std::abs(centModell - z.schwebungCent.wert) <= 0.5;
                for (const Parameter* p : {&decay, &hoch, &spaet, &cent, &b, &winkel, &gamma, &delta})
                    if (p->ziel[i] && !p->trifft[i]) alle = false;

                // die zweite Stufe: Parameter | Salamander (Anteil zweistufig) | Modell (Anteil)
                tabelle << z.midi << " | decay " << decay.wert[i] << ' ' << z.t60Ausgleich.wert << ' ' << mo.t60Ausgleich.wert
                        << (decay.trifft[i] ? '*' : ' ') << " | high " << hoch.wert[i] << ' ' << z.highDecay.wert << ' ' << mo.highDecay.wert
                        << (hoch.trifft[i] ? '*' : ' ') << " | late " << (spaet.ziel[i] ? spaet.wert[i] : 0.0) << ' ' << z.t60Spaet.wert << " ("
                        << z.zweiStufenAnteil.wert << ") " << mo.t60Spaet.wert << " (" << mo.zweiStufenAnteil.wert << ")"
                        << (spaet.trifft[i] ? '*' : ' ') << " | cent "
                        << cent.wert[i] << ' ' << z.schwebungCent.wert << ' ' << centModell << (cent.trifft[i] ? '*' : ' ');
                if (b.ziel[i]) tabelle << " | B " << 1e4 * b.wert[i] << ' ' << 1e4 * z.b.wert << ' ' << 1e4 * mo.b.wert << (b.trifft[i] ? '*' : ' ');
                if (winkel.ziel[i]) tabelle << " | Winkel " << winkel.wert[i] << " Abstand " << (abstand ? *abstand : 0.0) << (winkel.trifft[i] ? '*' : ' ');
                if (gamma.ziel[i])
                    tabelle << " | gamma " << gamma.wert[i] << " Knick " << z.knickS.wert << ' ' << mo.knickS.wert << " (" << mo.zweiStufenAnteil.wert
                            << ')' << (gamma.trifft[i] ? '*' : ' ');
                if (delta.ziel[i]) tabelle << " | delta " << delta.wert[i] << ' ' << z.schwebungCent.wert << ' ' << centModell << (delta.trifft[i] ? '*' : ' ');
                // TB1 a: die Tiefe der Einbrueche nur im Bericht (Pruefgroesse, kein Ziel)
                tabelle << " | Tiefe " << z.tiefeDb.wert << ' ' << mo.tiefeDb.wert;
                tabelle << '\n';
            }
            for (const Parameter* p : {&decay, &hoch, &spaet, &cent, &b, &winkel, &gamma, &delta})
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
            gemessen = true;
            if (alle || r == kRunden) break;
            gemessen = false;

            // S1 a mit dem halben Schritt (S3 a): logarithmisch √(Ziel / Modell), linear die halbe Differenz
            for (std::size_t i = 0; i < n; ++i)
            {
                const TastenWerte& z = s[i];
                const TastenWerte& mo = m[i];
                if (decay.ziel[i] && !decay.trifft[i] && mo.t60Ausgleich.anzahl > 0 && mo.t60Ausgleich.wert > 0.0)
                    decay.wert[i] = std::clamp(decay.wert[i] * std::sqrt(z.t60Ausgleich.wert / mo.t60Ausgleich.wert), 0.05, 120.0);
                if (hoch.ziel[i] && !hoch.trifft[i] && mo.highDecay.anzahl > 0 && mo.highDecay.wert > 0.0)
                    hoch.wert[i] = std::clamp(hoch.wert[i] * std::sqrt(z.highDecay.wert / mo.highDecay.wert), 0.001, 1.0);
                if (spaet.ziel[i] && !spaet.trifft[i] && mo.t60Spaet.anzahl > 0 && mo.t60Spaet.wert > 0.0)
                    spaet.wert[i] = std::clamp(spaet.wert[i] * std::sqrt(z.t60Spaet.wert / mo.t60Spaet.wert), 0.05, 120.0);
                if (spaet.ziel[i] && decay.ziel[i]) spaet.wert[i] = std::max(spaet.wert[i], decay.wert[i]);
                if (cent.ziel[i] && !cent.trifft[i])
                    cent.wert[i] = std::clamp(cent.wert[i] + 0.5 * (z.schwebungCent.wert - (mo.schwebungCent.anzahl > 0 ? mo.schwebungCent.wert : 0.0)),
                                              0.0, centGrenze(z.midi));
                if (b.ziel[i] && !b.trifft[i] && mo.b.anzahl > 0 && mo.b.wert > 0.0)
                    b.wert[i] = std::clamp(b.wert[i] * std::sqrt(z.b.wert / mo.b.wert), 1e-6, 0.1);
                if (const std::optional<double> abstand = phantomAbstand(z, mo); winkel.ziel[i] && !winkel.trifft[i] && abstand)
                {
                    const double rad = winkel.wert[i] * std::numbers::pi / 180.0;
                    winkel.wert[i] = std::clamp(std::atan(std::tan(rad) * std::pow(10.0, *abstand / 40.0)) * 180.0 / std::numbers::pi, 0.0, 10.0);
                }
                if (gamma.ziel[i] && !gamma.trifft[i])
                {
                    // ohne zwei Stufen im Modell: der Knick liegt jenseits des Gemessenen — γ wie bei einem spaeten Knick
                    const double knickModell = mo.knickS.anzahl > 0 && mo.zweiStufenAnteil.wert >= 0.5 ? mo.knickS.wert : 2.0 * z.knickS.wert;
                    gamma.wert[i] = gammaBerichtigt(gamma.wert[i], knickModell, z.knickS.wert, stufenAbstand(z));
                }
                if (delta.ziel[i] && !delta.trifft[i])
                    delta.wert[i] = std::clamp(delta.wert[i] + 0.5 * (z.schwebungCent.wert - (mo.schwebungCent.anzahl > 0 ? mo.schwebungCent.wert : 0.0)),
                                               0.0, std::min(20.0, centGrenze(z.midi)));
            }
        }
        // Tasten ohne Treffer fallen heraus; ist keine gefallen, ist die Kurve die zuletzt gemessene
        bool entfernt = false;
        for (Parameter* p : {&decay, &hoch, &spaet, &cent, &b, &winkel, &gamma, &delta})
            for (std::size_t i = 0; i < n; ++i)
                if (p->ziel[i] && !p->trifft[i])
                {
                    vermerkt << ' ' << p->name << '@' << tasten[i];
                    p->ziel[i] = false;
                    entfernt = true;
                }
        if (!entfernt) break;
        gemessen = false;
    }

    // zum Eintragen in baueVorgabe() — nur aus Tasten, die treffen (UF5 a); die Kurve laeuft durch ihre Punkte
    const lumi::loop::HammerSaite saite = saiteAus();
    std::ostringstream punkte;
    punkte << "// gemessen an Salamander (Yamaha C5), gesetzt durch das Modell hindurch (31d-4, S1 a, S2 a, S3 a; 31d-6, N1 a: Messung nach Schroeder), "
           << std::min(durchgang, 3) << " Durchgaenge, " << runde << " Runden; "
           << (gemessen ? "die Kurve ist so gemessen" : "ACHTUNG: die Kurve nach dem letzten Herausnehmen ist nicht gemessen") << "\n"
           << "s.abklingS = kurveAusPunkten(" << punkteText(saite.abklingS) << ");\n"
           << "s.abklingSpaetS = kurveAusPunkten(" << punkteText(saite.abklingSpaetS) << ");\n"
           << "s.abklingHoch = kurveAusPunkten(" << punkteText(saite.abklingHoch) << ");\n"
           << "s.verstimmungCent = kurveAusPunkten(" << punkteText(saite.verstimmungCent) << ");\n"
           << "s.inharmonizitaet = kurveAusPunkten(" << punkteText(saite.inharmonizitaet) << ");\n"
           << "s.stegWinkelGrad = kurveAusPunkten(" << punkteText(saite.stegWinkelGrad) << ");\n"
           << "s.polarisationWinkelGrad = kurveAusPunkten(" << punkteText(saite.polarisationWinkelGrad) << ");\n"
           << "s.polarisationCent = kurveAusPunkten(" << punkteText(saite.polarisationCent) << ");\n"
           << "// ohne Treffer, nicht in der Kurve (UF5 a: vermerkt):" << vermerkt.str() << '\n';
    std::ofstream(ziel / "stuetzpunkte.txt", std::ios::binary) << punkte.str();
    MESSAGE(verlauf.str() << punkte.str());
}
