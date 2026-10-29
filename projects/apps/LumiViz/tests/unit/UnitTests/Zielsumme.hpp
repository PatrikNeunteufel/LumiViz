/**
 ****************************************************************************************
 * @file   Zielsumme.hpp
 * @brief  7.5.9 (SM4 b): das Modell gegen Ziel und Toleranz von *Salamander* — je Taste und Groesse das Verhaeltnis
 *         der Abweichung zur Toleranz, die Summe der Quadrate als Ziel der Suche (docs/audio/Spielweisen_Konzept.md
 *         §4.1; Entscheide Patrik S156/S161: ZF1 a, ZF2 a, ZF4 a, ZF5 a, ZF7 a, SG1 a, UG1 a)
 *
 * @details
 * Reines C++ ohne Qt. Je Taste und Groesse aus `werte/salamander_ziel.txt` (`liesZiele`): Ziel z, Toleranz τ, Skala.
 * Der Modellwert m ist der Median ueber die Ebenen des Modells am Kanalmittel (`zielAus`, wie das Ziel selbst). Die
 * Abweichung d in der Skala der Groesse (`abstandIn`: Faktor |ln m/z|, dB und Cent |m − z|), geteilt durch die
 * Toleranz, nie kleiner als die Messgenauigkeit (UG1 a, `untergrenze`): r = d / max(τ, τ_min). Getroffen bei r ≤ 1
 * (ZF1 a); das Ziel der Suche ist Σ r² ueber alle Groessen aller Tasten (ZF5 a). Eine Groesse mit Ziel, die das
 * Modell nicht hergibt (etwa ein Teilton, der vor dem Bereich der Struktur verklingt), zaehlt mit r = 10
 * (`kFehltVerhaeltnis`, Lesart Claude S161 wie RV1 a); Groessen ohne Ziel zaehlen nicht (ZF7 a).
 ****************************************************************************************
 */

#pragma once

#include "Fluegelmessung.hpp"
#include "Strukturmessung.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace lumi::messung
{

/// Die Untergrenze der Toleranz (UG1 a): die Messgenauigkeit an synthetischen Signalen (Spielweisen_Konzept 31d-1:
/// Abklingzeiten auf 3 %, Pegel auf 0,5 dB, Schwebung auf 2 %).
inline const double kUntergrenzeFaktor = std::log(1.03);
inline constexpr double kUntergrenzeDb = 0.5;
inline constexpr double kUntergrenzeCentAnteil = 0.02;

/// Das Verhaeltnis einer Groesse mit Ziel, die das Modell nicht hergibt (Lesart Claude S161).
inline constexpr double kFehltVerhaeltnis = 10.0;

/// Die kleinste Toleranz in der Skala einer Groesse (UG1 a); bei Cent ein Anteil am Ziel.
inline double untergrenze(Skala skala, double ziel)
{
    switch (skala)
    {
    case Skala::Faktor: return kUntergrenzeFaktor;
    case Skala::Dezibel: return kUntergrenzeDb;
    case Skala::Cent: return kUntergrenzeCentAnteil * std::abs(ziel);
    }
    return 0.0;
}

/// Eine Groesse an einer Taste: Modellwert, Ziel, wirksame Toleranz (mit Untergrenze) und ihr Verhaeltnis.
struct Abweichung
{
    std::string groesse;
    Skala skala = Skala::Dezibel;
    double modell = 0.0;
    double ziel = 0.0;
    double toleranz = 0.0;
    double verhaeltnis = 0.0;
    /// Das Verhaeltnis mit Vorzeichen (7.5.10): (m − z) / τ, bei Faktor ln(m/z) / τ — das Residuum der Suche; |residuum|
    /// = verhaeltnis. Fehlt der Modellwert, `kFehltVerhaeltnis`.
    double residuum = 0.0;
    bool getroffen = false;
    bool fehlt = false;
};

/// Eine Taste: ihre Groessen (in der Reihenfolge des Ziels), Σ r², wie viele getroffen und wie viele fehlen.
struct TastenSumme
{
    int midi = 0;
    std::vector<Abweichung> groessen;
    double summe = 0.0;
    int getroffen = 0;
    int fehlend = 0;
    bool fertig = false;
};

/// Vergleicht die Groessen des Modells an einer Taste (`modell`: je Groesse die Werte je Ebene) mit ihrem Ziel.
inline TastenSumme vergleiche(int midi, const TastenZiele& ziel, const std::vector<GroesseJeEbene>& modell)
{
    TastenSumme t;
    t.midi = midi;
    for (const auto& [name, z] : ziel)
    {
        Abweichung a;
        a.groesse = name;
        a.skala = z.skala;
        a.ziel = z.ziel.wert;
        a.toleranz = std::max(z.toleranz.wert(), untergrenze(z.skala, z.ziel.wert));
        const auto it = std::find_if(modell.begin(), modell.end(), [&name](const GroesseJeEbene& g) { return g.name == name; });
        const Wert m = it != modell.end() ? zielAus(it->ebenen) : Wert{};
        const bool gueltig = m.anzahl > 0 && (z.skala != Skala::Faktor || (m.wert > 0.0 && z.ziel.wert > 0.0));
        if (gueltig && a.toleranz > 0.0)
        {
            a.modell = m.wert;
            a.residuum = (z.skala == Skala::Faktor ? std::log(m.wert / z.ziel.wert) : m.wert - z.ziel.wert) / a.toleranz;
            a.verhaeltnis = std::abs(a.residuum);
        }
        else
        {
            a.fehlt = true;
            a.verhaeltnis = kFehltVerhaeltnis;
            a.residuum = kFehltVerhaeltnis;
        }
        a.getroffen = !a.fehlt && a.verhaeltnis <= 1.0;
        t.summe += a.verhaeltnis * a.verhaeltnis;
        t.getroffen += a.getroffen ? 1 : 0;
        t.fehlend += a.fehlt ? 1 : 0;
        t.groessen.push_back(std::move(a));
    }
    t.fertig = !t.groessen.empty() && t.getroffen == static_cast<int>(t.groessen.size());
    return t;
}

/// Die Groessen des Modells an einer Taste (ZF2 a mit HU2 a): an der Struktur des Ziels je Teilton 1–3
/// (`strukturGroessen`) und der ganze Ton bis −20/−30/−40 dB. `ebenen[e]` und `messungen[e]` sind dieselbe Ebene
/// des Modells (aus `messeTon` mit `huellen`); der ganze Ton steht dort in beiden Kanaelen gleich (Kanalmittel).
/// Anders als `huellenGroessen` zaehlt jede uebergebene Ebene — das Modell rechnet nur die Ebenen der Suche (SG1 a).
inline std::vector<GroesseJeEbene> modellGroessen(const std::vector<TonHuellen>& ebenen,
                                                  const std::vector<std::array<KanalMessung, 2>>& messungen, const TonStruktur& struktur)
{
    std::vector<GroesseJeEbene> alle = strukturGroessen(ebenen, struktur);
    for (int g = 0; g < 3; ++g)
    {
        GroesseJeEbene je{"huelle_" + std::to_string(20 + 10 * g) + "_s", Skala::Faktor, {}};
        for (const std::array<KanalMessung, 2>& m : messungen)
        {
            EbenenWerte w;
            for (std::size_t k = 0; k < 2; ++k)
                w.kanaele[k] = m[k].erkannt ? detail::huelleVon(m[k], g) : std::nullopt;
            w.mittel = w.kanaele[0] ? w.kanaele[0] : w.kanaele[1];
            je.ebenen.push_back(w);
        }
        alle.push_back(std::move(je));
    }
    return alle;
}

/// Die Residuen aller Tasten hintereinander, je Taste in der Reihenfolge ihrer Groessen (die des Ziels) — der Vektor,
/// dessen Quadratsumme Levenberg–Marquardt senkt (7.5.10). Die Zeilen einer Taste liegen beisammen.
inline std::vector<double> residuen(const std::vector<TastenSumme>& tasten)
{
    std::vector<double> r;
    for (const TastenSumme& t : tasten)
        for (const Abweichung& a : t.groessen) r.push_back(a.residuum);
    return r;
}

/// Ueber alle Tasten: Σ r², getroffene und alle Groessen, fehlende, fertige Tasten und die Ausreisser (eine Taste ueber
/// dem Dreifachen des Mittels der Tastensummen, SP2 a).
struct Gesamt
{
    double summe = 0.0;
    int groessen = 0;
    int getroffen = 0;
    int fehlend = 0;
    int tasten = 0;
    int fertig = 0;
    std::vector<int> ausreisser;
};

inline Gesamt gesamt(const std::vector<TastenSumme>& tasten)
{
    Gesamt g;
    for (const TastenSumme& t : tasten)
    {
        g.summe += t.summe;
        g.groessen += static_cast<int>(t.groessen.size());
        g.getroffen += t.getroffen;
        g.fehlend += t.fehlend;
        g.fertig += t.fertig ? 1 : 0;
    }
    g.tasten = static_cast<int>(tasten.size());
    if (g.tasten > 0)
    {
        const double mittel = g.summe / g.tasten;
        for (const TastenSumme& t : tasten)
            if (t.summe > 3.0 * mittel) g.ausreisser.push_back(t.midi);
    }
    return g;
}

/// Der Bericht: je Taste eine Zeile (Σ r², getroffen / alle, fehlend, die drei groessten r mit Groesse), dann die
/// Gesamtzeile; mit `einzeln` dazu je Taste jede Groesse (Modell, Ziel, Toleranz, r).
inline std::string berichte(const std::vector<TastenSumme>& tasten, bool einzeln = false)
{
    std::ostringstream s;
    s << std::fixed << std::setprecision(3);
    s << "Taste | Summe r^2 | getroffen / alle | fehlend | groesste r\n";
    for (const TastenSumme& t : tasten)
    {
        std::vector<const Abweichung*> sortiert;
        for (const Abweichung& a : t.groessen) sortiert.push_back(&a);
        std::sort(sortiert.begin(), sortiert.end(), [](const Abweichung* a, const Abweichung* b) { return a->verhaeltnis > b->verhaeltnis; });
        s << t.midi << " | " << t.summe << " | " << t.getroffen << " / " << t.groessen.size() << " | " << t.fehlend << " |";
        for (std::size_t i = 0; i < std::min<std::size_t>(3, sortiert.size()); ++i)
            s << ' ' << sortiert[i]->groesse << ' ' << sortiert[i]->verhaeltnis << (sortiert[i]->fehlt ? " (fehlt)" : "") << (i + 1 < 3 ? ";" : "");
        s << (t.fertig ? " | fertig" : "") << '\n';
        if (einzeln)
            for (const Abweichung& a : t.groessen)
                s << "    " << a.groesse << " [" << skalaName(a.skala) << "] Modell " << (a.fehlt ? std::string("-") : std::to_string(a.modell))
                  << " Ziel " << a.ziel << " Toleranz " << a.toleranz << " r " << a.verhaeltnis << (a.getroffen ? " getroffen" : "") << '\n';
    }
    const Gesamt g = gesamt(tasten);
    s << "Gesamt: Summe r^2 " << g.summe << ", getroffen " << g.getroffen << " / " << g.groessen << ", fehlend " << g.fehlend << ", fertige Tasten "
      << g.fertig << " / " << g.tasten << ", Ausreisser";
    if (g.ausreisser.empty()) s << " keine";
    for (const int m : g.ausreisser) s << ' ' << m;
    s << '\n';
    return s.str();
}

}  // namespace lumi::messung
