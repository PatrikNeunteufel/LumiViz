/**
 * @file Tonform.cpp
 * @brief Klang aus einer Form: eine Zeichnung fuer den Ton aufbereitet (s. Header)
 */

#include "shape/Tonform.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <mutex>
#include <utility>

namespace lumi::shape
{

namespace
{

/// Die Laenge eines geschlossenen Zugs.
double laenge(const std::vector<Punkt>& punkte)
{
    double summe = 0.0;
    for (std::size_t i = 0; i < punkte.size(); ++i) summe += std::abs(punkte[(i + 1) % punkte.size()] - punkte[i]);
    return summe;
}

Punkt schwerpunkt(const std::vector<Punkt>& punkte)
{
    Punkt summe{0.0, 0.0};
    for (const Punkt& p : punkte) summe += p;
    return punkte.empty() ? summe : summe / static_cast<double>(punkte.size());
}

/// Der Punkt von `punkte`, der `ziel` am naechsten liegt.
std::size_t naechster(const std::vector<Punkt>& punkte, const Punkt& ziel)
{
    std::size_t bester = 0;
    double abstand = std::norm(punkte[0] - ziel);
    for (std::size_t i = 1; i < punkte.size(); ++i)
        if (const double d = std::norm(punkte[i] - ziel); d < abstand)
        {
            abstand = d;
            bester = i;
        }
    return bester;
}

/// Wo sich zwei Zuege am naechsten kommen: erst grob (jeder 16. Punkt gegen
/// jeden 16.), dann abwechselnd genau — jeder Punkt des einen gegen den
/// gefundenen des anderen, bis sich nichts mehr bewegt.
std::pair<std::size_t, std::size_t> naechstesPaar(const std::vector<Punkt>& a, const std::vector<Punkt>& b)
{
    constexpr std::size_t kGrob = 16;
    std::size_t ia = 0;
    std::size_t ib = 0;
    double abstand = std::norm(a[0] - b[0]);
    for (std::size_t i = 0; i < a.size(); i += kGrob)
        for (std::size_t j = 0; j < b.size(); j += kGrob)
            if (const double d = std::norm(a[i] - b[j]); d < abstand)
            {
                abstand = d;
                ia = i;
                ib = j;
            }
    for (int runde = 0; runde < 8; ++runde)
    {
        const std::size_t neuB = naechster(b, a[ia]);
        const std::size_t neuA = naechster(a, b[neuB]);
        if (neuA == ia && neuB == ib) break;
        ia = neuA;
        ib = neuB;
    }
    return {ia, ib};
}

struct Schluessel
{
    QString nennung;
    int kontur = 0;
    [[nodiscard]] bool operator<(const Schluessel& o) const
    {
        return kontur != o.kontur ? kontur < o.kontur : nennung < o.nennung;
    }
};

std::mutex& sperre()
{
    static std::mutex eine;
    return eine;
}

std::map<Schluessel, std::shared_ptr<const Tonform>>& gemerkt()
{
    static std::map<Schluessel, std::shared_ptr<const Tonform>> eine;
    return eine;
}

}  // namespace

Tonform tonform(const Zeichnung& zeichnung, int kontur)
{
    // Die Zuege der gemeinten Konturen, y nach oben
    std::vector<std::vector<Punkt>> zuege;
    for (const Kontur& k : zeichnung.konturen)
    {
        if (kontur == 0 ? !k.gewaehlt : k.kennung != kontur) continue;
        std::vector<Punkt> punkte = abtasten(k.pfad);
        if (!punkte.empty()) zuege.push_back(std::move(punkte));
    }
    Tonform form;
    if (zuege.empty()) return form;

    if (zuege.size() == 1)
    {
        // eine Kontur: um ihren Schwerpunkt, der fernste Punkt im Abstand 1 (T1 a)
        std::vector<Punkt>& punkte = zuege.front();
        const Punkt mitte = schwerpunkt(punkte);
        double fern = 0.0;
        for (const Punkt& p : punkte) fern = std::max(fern, std::abs(p - mitte));
        const double mass = fern > 0.0 ? 1.0 / fern : 1.0;
        for (Punkt& p : punkte) p = (p - mitte) * mass;
        TonKontur eine;
        eine.reihe = reihe(punkte);
        eine.reihe.mitte = Punkt{0.0, 0.0};  // bis auf Rundung ohnehin null
        form.konturen.push_back(std::move(eine));
        return form;
    }

    // --- mehrere: wo jede betreten und verlassen wird (B1, Reihenfolge der Liste)
    const std::size_t zahl = zuege.size();
    std::vector<std::size_t> tor(zahl, 0);
    {
        const auto [erste, zweite] = naechstesPaar(zuege[0], zuege[1]);
        tor[0] = erste;
        tor[1] = zweite;
        for (std::size_t i = 2; i < zahl; ++i) tor[i] = naechster(zuege[i], zuege[i - 1][tor[i - 1]]);
    }
    // --- der Umlauf: jede nach ihrer Laenge, dazwischen (und zurueck zur ersten) ein Sprung
    form.sprung = kSprungAnteil;
    std::vector<double> lang(zahl, 0.0);
    double gesamt = 0.0;
    for (std::size_t i = 0; i < zahl; ++i) gesamt += lang[i] = laenge(zuege[i]);
    const double fuerKonturen = 1.0 - form.sprung * static_cast<double>(zahl);
    // --- der Mittelwert des ganzen Wegs in den Nullpunkt, der fernste Punkt auf 1
    Punkt mittel{0.0, 0.0};
    std::vector<double> anteil(zahl, 0.0);
    for (std::size_t i = 0; i < zahl; ++i)
    {
        anteil[i] = gesamt > 0.0 ? fuerKonturen * lang[i] / gesamt : fuerKonturen / static_cast<double>(zahl);
        mittel += anteil[i] * schwerpunkt(zuege[i]);
        const Punkt& von = zuege[i][tor[i]];
        const Punkt& nach = zuege[(i + 1) % zahl][tor[(i + 1) % zahl]];
        mittel += form.sprung * 0.5 * (von + nach);
    }
    double fern = 0.0;
    for (const std::vector<Punkt>& zug : zuege)
        for (const Punkt& p : zug) fern = std::max(fern, std::abs(p - mittel));
    const double mass = fern > 0.0 ? 1.0 / fern : 1.0;
    for (std::size_t i = 0; i < zahl; ++i)
    {
        for (Punkt& p : zuege[i]) p = (p - mittel) * mass;
        TonKontur eine;
        eine.reihe = reihe(zuege[i]);
        eine.anteil = anteil[i];
        eine.start = static_cast<double>(tor[i]) / static_cast<double>(zuege[i].size());
        form.konturen.push_back(std::move(eine));
    }
    return form;
}

Punkt tonwert(const Tonform& form, double umlauf, double harmonische, double hoechste)
{
    if (form.konturen.empty()) return Punkt{0.0, 0.0};
    if (form.konturen.size() == 1) return wert(form.konturen.front().reihe, umlauf, std::min(harmonische, std::max(1.0, hoechste)));

    double u = umlauf - std::floor(umlauf);
    const std::size_t zahl = form.konturen.size();
    const auto teiltoene = [&](const TonKontur& k) { return std::min(harmonische, std::max(1.0, hoechste * k.anteil)); };
    for (std::size_t i = 0; i < zahl; ++i)
    {
        const TonKontur& k = form.konturen[i];
        if (u < k.anteil) return wert(k.reihe, k.start + u / k.anteil, teiltoene(k), /*mitMitte=*/true);
        u -= k.anteil;
        if (u < form.sprung)
        {
            // der Sprung: gerade von dort, wo diese Kontur verlassen wird, dorthin, wo die naechste beginnt
            const TonKontur& n = form.konturen[(i + 1) % zahl];
            const Punkt von = wert(k.reihe, k.start, teiltoene(k), true);
            const Punkt nach = wert(n.reihe, n.start, teiltoene(n), true);
            return von + (nach - von) * (u / form.sprung);
        }
        u -= form.sprung;
    }
    // Rundung am Ende des Umlaufs: der Anfang der ersten
    const TonKontur& erste = form.konturen.front();
    return wert(erste.reihe, erste.start, teiltoene(erste), true);
}

std::shared_ptr<const Tonform> tonformFuer(const QString& nennung, int kontur)
{
    const Schluessel schluessel{nennung, kontur};
    {
        const std::lock_guard<std::mutex> halt(sperre());
        if (const auto treffer = gemerkt().find(schluessel); treffer != gemerkt().end()) return treffer->second;
    }
    // ausserhalb der Sperre rechnen: es dauert, und zwei gleiche Ergebnisse schaden nicht
    std::shared_ptr<const Tonform> neu;
    Zeichnung zeichnung;
    if (!nennung.isEmpty() && holeZeichnung(nennung, zeichnung))
    {
        Tonform form = tonform(zeichnung, kontur);
        if (!form.leer()) neu = std::make_shared<const Tonform>(std::move(form));
    }
    const std::lock_guard<std::mutex> halt(sperre());
    return gemerkt().emplace(schluessel, std::move(neu)).first->second;
}

void vergissTonformen()
{
    const std::lock_guard<std::mutex> halt(sperre());
    gemerkt().clear();
}

}  // namespace lumi::shape
