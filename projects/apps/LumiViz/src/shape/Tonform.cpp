/**
 * @file Tonform.cpp
 * @brief Klang aus einer Form: eine Zeichnung fuer den Ton aufbereitet (s. Header)
 */

#include "shape/Tonform.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
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

/// Ein Stueck des Wegs: so viel einer Kontur ab `von` (0 … 1 auf ihrem Zug).
struct Stueck
{
    std::size_t zug = 0;
    double von = 0.0;
    double umfang = 1.0;  ///< 1 = einmal ganz herum
};

/// Der Weg des Stifts: die gemeinten Konturen (y nach oben, in den Massen der
/// Zeichnung) und die Stuecke in der Reihenfolge, in der er sie faehrt;
/// zwischen zwei Stuecken (und vom letzten zum ersten) springt er.
struct Weg
{
    std::vector<std::vector<Punkt>> zuege;
    std::vector<int> kennungen;
    std::vector<Stueck> stuecke;
};

std::size_t platzBei(const std::vector<Punkt>& zug, double stelle)
{
    const double n = static_cast<double>(zug.size());
    return static_cast<std::size_t>(std::llround((stelle - std::floor(stelle)) * n)) % zug.size();
}

double stelleVon(const std::vector<Punkt>& zug, std::size_t platz)
{
    return static_cast<double>(platz) / static_cast<double>(zug.size());
}

/// So weit von `stelle` vorwaerts bis `ziel`, auf einem Umlauf (0 … 1).
double vorwaerts(double stelle, double ziel)
{
    const double d = ziel - stelle;
    return d - std::floor(d);
}

/// Shortest (B1 a): jede Kontur wird einmal ganz gefahren und dort verlassen,
/// wo sie betreten wurde; gesucht sind Reihenfolge und Tore, mit denen alle
/// Spruenge zusammen kurz sind. Eine Naeherung: abwechselnd die Tore zur
/// Reihenfolge (je Tor der Punkt, der seinen beiden Nachbarn zusammen am
/// naechsten liegt) und die Reihenfolge zu den Toren (2-opt; die erste Kontur
/// der Liste bleibt vorn).
void kuerzeste(const std::vector<std::vector<Punkt>>& zuege, std::vector<std::size_t>& folge, std::vector<std::size_t>& tor)
{
    const std::size_t n = zuege.size();
    const auto ort = [&](std::size_t platzInFolge) {
        const std::size_t z = folge[platzInFolge % n];
        return zuege[z][tor[z]];
    };
    const auto kosten = [&](std::size_t a, std::size_t b) { return std::abs(ort(a) - ort(b)); };
    for (int runde = 0; runde < 6; ++runde)
    {
        // die Tore zur Reihenfolge
        for (std::size_t k = 0; k < n; ++k)
        {
            const Punkt vorher = ort(k + n - 1);
            const Punkt danach = ort(k + 1);
            const std::vector<Punkt>& zug = zuege[folge[k]];
            std::size_t bester = tor[folge[k]];
            double kuerzest = std::abs(zug[bester] - vorher) + std::abs(zug[bester] - danach);
            for (std::size_t i = 0; i < zug.size(); ++i)
                if (const double d = std::abs(zug[i] - vorher) + std::abs(zug[i] - danach); d < kuerzest)
                {
                    kuerzest = d;
                    bester = i;
                }
            tor[folge[k]] = bester;
        }
        // die Reihenfolge zu den Toren (2-opt), die erste bleibt vorn
        bool besser = true;
        for (int schritte = 0; besser && schritte < 64; ++schritte)
        {
            besser = false;
            for (std::size_t a = 1; a + 1 < n; ++a)
                for (std::size_t b = a + 1; b < n; ++b)
                {
                    const double alt = kosten(a - 1, a) + kosten(b, b + 1);
                    const double neu = std::abs(ort(a - 1) - ort(b)) + std::abs(ort(a) - ort(b + 1));
                    if (neu + 1e-12 < alt)
                    {
                        std::reverse(folge.begin() + static_cast<std::ptrdiff_t>(a), folge.begin() + static_cast<std::ptrdiff_t>(b) + 1);
                        besser = true;
                    }
                }
        }
    }
}

/// Der Weg zu einer Zeichnung (s. `tonform`).
Weg plane(const Zeichnung& zeichnung, int kontur)
{
    Weg weg;
    for (const Kontur& k : zeichnung.konturen)
    {
        if (kontur == 0 ? !k.gewaehlt : k.kennung != kontur) continue;
        std::vector<Punkt> punkte = abtasten(k.pfad);
        if (punkte.empty()) continue;
        weg.zuege.push_back(std::move(punkte));
        weg.kennungen.push_back(k.kennung);
    }
    const std::size_t zahl = weg.zuege.size();
    if (zahl == 0) return weg;
    if (zahl == 1)
    {
        weg.stuecke.push_back({0, 0.0, 1.0});
        return weg;
    }

    // In Reihenfolge der Liste (B1 b, die Vorgabe): von der ersten zur zweiten dort, wo sie sich am naechsten
    // kommen; jede weitere am Punkt, der dem letzten Tor am naechsten liegt — auch der Anfang fuer Shortest
    std::vector<std::size_t> tor(zahl, 0);
    {
        const auto [erste, zweite] = naechstesPaar(weg.zuege[0], weg.zuege[1]);
        tor[0] = erste;
        tor[1] = zweite;
        for (std::size_t i = 2; i < zahl; ++i) tor[i] = naechster(weg.zuege[i], weg.zuege[i - 1][tor[i - 1]]);
    }
    std::vector<std::size_t> folge(zahl);
    for (std::size_t i = 0; i < zahl; ++i) folge[i] = i;

    if (zeichnung.spruenge == Spruenge::VonHand)
    {
        // Von Hand (B1 d, G2 a): die Bruecken zwischen gemeinten Konturen, ohne Ring (eine, die einen
        // schloesse, bleibt weg). Der Stift faehrt jeden Baum ab: an einer Bruecke verlaesst er die Kontur,
        // faehrt die andere (mit allem, was an ihr haengt) und kommt an derselben Stelle zurueck. Was keine
        // Bruecke erreicht, kommt in Reihenfolge der Liste dazu, betreten am Punkt, der dem letzten am naechsten liegt.
        struct Kante
        {
            std::size_t a = 0;
            std::size_t b = 0;
            double stelleA = 0.0;
            double stelleB = 0.0;
        };
        std::vector<Kante> kanten;
        std::vector<std::size_t> gruppe(zahl);
        for (std::size_t i = 0; i < zahl; ++i) gruppe[i] = i;
        const auto wurzel = [&gruppe](std::size_t i) {
            while (gruppe[i] != i) i = gruppe[i] = gruppe[gruppe[i]];
            return i;
        };
        const auto platzVon = [&weg](int kennung) {
            const auto it = std::find(weg.kennungen.begin(), weg.kennungen.end(), kennung);
            return it == weg.kennungen.end() ? weg.kennungen.size() : static_cast<std::size_t>(it - weg.kennungen.begin());
        };
        for (const Bruecke& br : zeichnung.bruecken)
        {
            const std::size_t a = platzVon(br.von);
            const std::size_t b = platzVon(br.nach);
            if (a >= zahl || b >= zahl || a == b || wurzel(a) == wurzel(b)) continue;
            gruppe[wurzel(a)] = wurzel(b);
            kanten.push_back({a, b, stelleVon(weg.zuege[a], naechster(weg.zuege[a], Punkt{br.vonPunkt.x(), -br.vonPunkt.y()})),
                              stelleVon(weg.zuege[b], naechster(weg.zuege[b], Punkt{br.nachPunkt.x(), -br.nachPunkt.y()}))});
        }
        std::vector<bool> besucht(zahl, false);
        constexpr double kNichts = 1e-9;
        const std::function<void(std::size_t, double, std::size_t)> fahre = [&](std::size_t z, double eintritt, std::size_t herKante) {
            besucht[z] = true;
            // die Bruecken an dieser Kontur, in Fahrtrichtung ab dem Eintritt
            std::vector<std::pair<double, std::size_t>> abzweige;
            for (std::size_t k = 0; k < kanten.size(); ++k)
            {
                if (k == herKante) continue;
                if (kanten[k].a == z && !besucht[kanten[k].b]) abzweige.emplace_back(kanten[k].stelleA, k);
                if (kanten[k].b == z && !besucht[kanten[k].a]) abzweige.emplace_back(kanten[k].stelleB, k);
            }
            std::stable_sort(abzweige.begin(), abzweige.end(), [eintritt](const auto& x, const auto& y) {
                return vorwaerts(eintritt, x.first) < vorwaerts(eintritt, y.first);
            });
            double jetzt = eintritt;
            double gefahren = 0.0;
            for (const auto& [stelle, k] : abzweige)
            {
                const std::size_t ander = kanten[k].a == z ? kanten[k].b : kanten[k].a;
                if (besucht[ander]) continue;
                const double stueck = vorwaerts(jetzt, stelle);
                if (stueck > kNichts) weg.stuecke.push_back({z, jetzt, stueck});
                gefahren += stueck;
                jetzt = stelle;
                fahre(ander, kanten[k].a == z ? kanten[k].stelleB : kanten[k].stelleA, k);
            }
            const double rest = abzweige.empty() ? 1.0 : 1.0 - gefahren;
            if (rest > kNichts) weg.stuecke.push_back({z, jetzt, rest});
        };
        std::size_t letzter = 0;
        double letzteStelle = 0.0;
        for (std::size_t z = 0; z < zahl; ++z)
        {
            if (besucht[z]) continue;
            double eintritt = 0.0;
            if (weg.stuecke.empty())
            {
                // der Anfang: an der ersten Bruecke dieser Kontur, sonst an ihrem Anfang
                for (const Kante& k : kanten)
                    if (k.a == z || k.b == z)
                    {
                        eintritt = k.a == z ? k.stelleA : k.stelleB;
                        break;
                    }
            }
            else
            {
                eintritt = stelleVon(weg.zuege[z], naechster(weg.zuege[z], weg.zuege[letzter][platzBei(weg.zuege[letzter], letzteStelle)]));
            }
            fahre(z, eintritt, kanten.size());
            letzter = z;
            letzteStelle = eintritt;
        }
        return weg;
    }

    if (zeichnung.spruenge == Spruenge::Kuerzeste && zahl > 2) kuerzeste(weg.zuege, folge, tor);
    for (const std::size_t z : folge) weg.stuecke.push_back({z, stelleVon(weg.zuege[z], tor[z]), 1.0});
    return weg;
}

/// Wo ein Stueck beginnt und endet, auf seinem Zug.
Punkt anfangVon(const Weg& weg, const Stueck& s)
{
    return weg.zuege[s.zug][platzBei(weg.zuege[s.zug], s.von)];
}

Punkt endeVon(const Weg& weg, const Stueck& s)
{
    return weg.zuege[s.zug][platzBei(weg.zuege[s.zug], s.umfang >= 1.0 ? s.von : s.von + s.umfang)];
}

/// Der Mittelwert der Punkte eines Stuecks.
Punkt mitteVon(const Weg& weg, const Stueck& s)
{
    const std::vector<Punkt>& zug = weg.zuege[s.zug];
    if (s.umfang >= 1.0) return schwerpunkt(zug);
    const std::size_t anfang = platzBei(zug, s.von);
    const std::size_t zahl = std::max<std::size_t>(1, static_cast<std::size_t>(std::llround(s.umfang * static_cast<double>(zug.size()))));
    Punkt summe{0.0, 0.0};
    for (std::size_t i = 0; i < zahl; ++i) summe += zug[(anfang + i) % zug.size()];
    return summe / static_cast<double>(zahl);
}

}  // namespace

Tonform tonform(const Zeichnung& zeichnung, int kontur)
{
    Weg weg = plane(zeichnung, kontur);
    Tonform form;
    if (weg.stuecke.empty()) return form;

    if (weg.zuege.size() == 1)
    {
        // eine Kontur: um ihren Schwerpunkt, der fernste Punkt im Abstand 1 (T1 a)
        std::vector<Punkt>& punkte = weg.zuege.front();
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

    // --- der Umlauf: jedes Stueck nach seiner Laenge, dazwischen (und zurueck zum ersten) ein Sprung
    const std::size_t zahl = weg.stuecke.size();
    form.sprung = kSprungAnteil;
    std::vector<double> lang(weg.zuege.size(), 0.0);
    for (std::size_t z = 0; z < weg.zuege.size(); ++z) lang[z] = laenge(weg.zuege[z]);
    double gesamt = 0.0;
    for (const Stueck& s : weg.stuecke) gesamt += lang[s.zug] * s.umfang;
    const double fuerKonturen = 1.0 - form.sprung * static_cast<double>(zahl);
    // --- der Mittelwert des ganzen Wegs in den Nullpunkt, der fernste Punkt auf 1
    Punkt mittel{0.0, 0.0};
    std::vector<double> anteil(zahl, 0.0);
    for (std::size_t i = 0; i < zahl; ++i)
    {
        const Stueck& s = weg.stuecke[i];
        anteil[i] = gesamt > 0.0 ? fuerKonturen * lang[s.zug] * s.umfang / gesamt : fuerKonturen / static_cast<double>(zahl);
        mittel += anteil[i] * mitteVon(weg, s);
        const Punkt von = endeVon(weg, s);
        const Punkt nach = anfangVon(weg, weg.stuecke[(i + 1) % zahl]);
        mittel += form.sprung * 0.5 * (von + nach);
    }
    double fern = 0.0;
    for (const std::vector<Punkt>& zug : weg.zuege)
        for (const Punkt& p : zug) fern = std::max(fern, std::abs(p - mittel));
    const double mass = fern > 0.0 ? 1.0 / fern : 1.0;
    std::vector<Reihe> reihen;
    reihen.reserve(weg.zuege.size());
    for (std::vector<Punkt>& zug : weg.zuege)
    {
        for (Punkt& p : zug) p = (p - mittel) * mass;
        reihen.push_back(reihe(zug));
    }
    for (std::size_t i = 0; i < zahl; ++i)
    {
        const Stueck& s = weg.stuecke[i];
        TonKontur eine;
        eine.reihe = reihen[s.zug];
        eine.anteil = anteil[i];
        eine.start = s.von;
        eine.umfang = s.umfang;
        form.konturen.push_back(std::move(eine));
    }
    return form;
}

std::vector<std::pair<QPointF, QPointF>> sprungLinien(const Zeichnung& zeichnung)
{
    std::vector<std::pair<QPointF, QPointF>> linien;
    const Weg weg = plane(zeichnung, 0);
    if (weg.stuecke.size() < 2) return linien;
    for (std::size_t i = 0; i < weg.stuecke.size(); ++i)
    {
        const Punkt von = endeVon(weg, weg.stuecke[i]);
        const Punkt nach = anfangVon(weg, weg.stuecke[(i + 1) % weg.stuecke.size()]);
        linien.emplace_back(QPointF(von.real(), -von.imag()), QPointF(nach.real(), -nach.imag()));  // y wieder nach unten
    }
    return linien;
}

bool brueckeErlaubt(const Zeichnung& zeichnung, int von, int nach)
{
    if (von == nach || zeichnung.finde(von) == nullptr || zeichnung.finde(nach) == nullptr) return false;
    // verbinden die vorhandenen Bruecken die beiden schon (auch ueber andere)? Dann schloesse diese einen Ring
    std::map<int, int> gruppe;
    const std::function<int(int)> wurzel = [&gruppe, &wurzel](int k) {
        const auto it = gruppe.find(k);
        if (it == gruppe.end() || it->second == k) return k;
        return it->second = wurzel(it->second);
    };
    for (const Bruecke& br : zeichnung.bruecken)
    {
        const int a = wurzel(br.von);
        const int b = wurzel(br.nach);
        if (a != b) gruppe[a] = b;
    }
    return wurzel(von) != wurzel(nach);
}

Punkt tonwert(const Tonform& form, double umlauf, double harmonische, double hoechste)
{
    if (form.konturen.empty()) return Punkt{0.0, 0.0};
    if (form.konturen.size() == 1) return wert(form.konturen.front().reihe, umlauf, std::min(harmonische, std::max(1.0, hoechste)));

    double u = umlauf - std::floor(umlauf);
    const std::size_t zahl = form.konturen.size();
    // ein Stueck, dem ein Teil des Umlaufs gehoert, wird entsprechend schneller durchlaufen und traegt anteilig
    // weniger; bei einem Teil der Kontur (umfang < 1) entsprechend mehr
    const auto teiltoene = [&](const TonKontur& k) { return std::min(harmonische, std::max(1.0, hoechste * k.anteil / k.umfang)); };
    const auto ende = [](const TonKontur& k) { return k.umfang >= 1.0 ? k.start : k.start + k.umfang; };
    for (std::size_t i = 0; i < zahl; ++i)
    {
        const TonKontur& k = form.konturen[i];
        if (u < k.anteil) return wert(k.reihe, k.start + u / k.anteil * k.umfang, teiltoene(k), /*mitMitte=*/true);
        u -= k.anteil;
        if (u < form.sprung)
        {
            // der Sprung: gerade von dort, wo dieses Stueck endet, dorthin, wo das naechste beginnt
            const TonKontur& n = form.konturen[(i + 1) % zahl];
            const Punkt von = wert(k.reihe, ende(k), teiltoene(k), true);
            const Punkt nach = wert(n.reihe, n.start, teiltoene(n), true);
            return von + (nach - von) * (u / form.sprung);
        }
        u -= form.sprung;
    }
    // Rundung am Ende des Umlaufs: der Anfang des ersten
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
