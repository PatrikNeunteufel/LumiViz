/**
 * @file   Probe.cpp
 * @brief  Die Rechnung zu `InstrumentMeasure/Probe.hpp` (Herleitung dort). Wortgleich aus dem Kopf der
 *         Unit-Tests gezogen (Instrumente_IC_Konzept.md, Teilplan 34e-1): in der .cpp, damit die Schalter nach
 *         E1 a (PRIVATE) gelten.
 */
#include "InstrumentMeasure/Probe.hpp"

namespace lumi::messung
{

std::vector<TastenSumme> liesBericht(const std::filesystem::path& pfad)
{
    std::ifstream ein(pfad, std::ios::binary);
    std::vector<std::string> zeilen;
    std::string zeile;
    bool mitAbschnitt = false;
    while (std::getline(ein, zeile))
    {
        if (!zeile.empty() && zeile.back() == '\r') zeile.pop_back();
        mitAbschnitt = mitAbschnitt || zeile == "## Je Taste und Groesse";
        zeilen.push_back(zeile);
    }
    std::vector<TastenSumme> tasten;
    bool drin = !mitAbschnitt;
    for (const std::string& z : zeilen)
    {
        if (z == "## Je Taste und Groesse")
        {
            drin = true;
            continue;
        }
        if (!drin) continue;
        if (z.rfind("## ", 0) == 0 || z.rfind("Gesamt:", 0) == 0)
        {
            if (mitAbschnitt) break;
            continue;
        }
        if (z.rfind("    ", 0) == 0 && !tasten.empty())
        {
            std::istringstream s(z);
            std::string skala, w;
            Abweichung a;
            std::string modell;
            s >> a.groesse >> skala >> w >> modell >> w >> a.ziel >> w >> a.toleranz >> w >> a.verhaeltnis;
            std::string rest;
            s >> rest;
            a.getroffen = rest == "getroffen";
            a.skala = skala == "[faktor]" ? Skala::Faktor : (skala == "[cent]" ? Skala::Cent : Skala::Dezibel);
            a.fehlt = modell == "-";
            if (!a.fehlt)
            {
                a.modell = std::stod(modell);
                const double d = a.skala == Skala::Faktor ? std::log(a.modell / a.ziel) : a.modell - a.ziel;
                a.residuum = d >= 0.0 ? a.verhaeltnis : -a.verhaeltnis;
            }
            else
                a.residuum = a.verhaeltnis;
            TastenSumme& t = tasten.back();
            t.getroffen += a.getroffen ? 1 : 0;
            t.fehlend += a.fehlt ? 1 : 0;
            t.groessen.push_back(std::move(a));
            continue;
        }
        if (const std::size_t strich = z.find(" | "); strich != std::string::npos && !z.empty() && std::isdigit(static_cast<unsigned char>(z.front())))
        {
            // die Summe aus der Zeile der Taste (aus den gerundeten r nachgerechnet wiche sie um Hundertstel ab)
            TastenSumme t;
            t.midi = std::stoi(z.substr(0, strich));
            t.summe = std::stod(z.substr(strich + 3));
            tasten.push_back(std::move(t));
        }
    }
    for (TastenSumme& t : tasten) t.fertig = !t.groessen.empty() && t.getroffen == static_cast<int>(t.groessen.size());
    return tasten;
}

bool eindeutigNachHw2(const Hoerschwellen& schwellen, const std::vector<const Abweichung*>& jeStart)
{
    if (jeStart.empty()) return false;
    double klein = 0.0, gross = 0.0;
    for (std::size_t i = 0; i < jeStart.size(); ++i)
    {
        if (jeStart[i]->fehlt) return false;
        klein = i == 0 ? jeStart[i]->modell : std::min(klein, jeStart[i]->modell);
        gross = i == 0 ? jeStart[i]->modell : std::max(gross, jeStart[i]->modell);
    }
    const Abweichung& a = *jeStart.front();
    const double d = a.skala == Skala::Faktor ? (klein > 0.0 ? std::log(gross / klein) : std::numeric_limits<double>::infinity()) : gross - klein;
    if (d <= a.toleranz) return true;
    return hoerbarGleich(schwellen, a.groesse, gross, klein) && hoerbarGleich(schwellen, a.groesse, klein, gross);
}

double vorzeichenTestP(int k, int n)
{
    if (n <= 0) return 1.0;
    const int m = std::min(k, n - k);
    double summe = 0.0;
    double binom = 1.0;  // C(n, i)
    for (int i = 0; i <= m; ++i)
    {
        summe += binom;
        binom = binom * (n - i) / (i + 1);
    }
    return std::min(1.0, 2.0 * summe * std::pow(0.5, n));
}

std::vector<bool> holm(const std::vector<double>& p, double alpha)
{
    std::vector<std::size_t> reihe(p.size());
    std::iota(reihe.begin(), reihe.end(), std::size_t{0});
    std::stable_sort(reihe.begin(), reihe.end(), [&p](std::size_t a, std::size_t b) { return p[a] < p[b]; });
    std::vector<bool> verworfen(p.size(), false);
    const std::size_t m = p.size();
    for (std::size_t i = 0; i < m; ++i)
    {
        if (p[reihe[i]] > alpha / static_cast<double>(m - i)) break;
        verworfen[reihe[i]] = true;
    }
    return verworfen;
}

std::vector<GroessenBefund> systematik(const Hoerschwellen& schwellen, const std::vector<TastenSumme>& tasten, double alpha)
{
    std::map<std::string, GroessenBefund> je;
    std::map<std::string, std::vector<double>> betraege;
    for (const TastenSumme& t : tasten)
        for (const Abweichung& a : t.groessen)
        {
            GroessenBefund& b = je[a.groesse];
            b.groesse = a.groesse;
            ++b.tasten;
            betraege[a.groesse].push_back(a.verhaeltnis);
            if (hoerbarGetroffen(schwellen, a)) continue;
            if (a.fehlt)
            {
                ++b.fehlend;
                continue;
            }
            ++b.nichtGetroffen;
            b.zuGross += a.residuum > 0.0 ? 1 : 0;
        }
    std::vector<GroessenBefund> alle;
    for (auto& [name, b] : je)
    {
        b.p = vorzeichenTestP(b.zuGross, b.nichtGetroffen);
        const double grenze = 3.0 * detail::median(betraege[name]);
        for (const TastenSumme& t : tasten)
            for (const Abweichung& a : t.groessen)
                if (a.groesse == name && !hoerbarGetroffen(schwellen, a) && a.verhaeltnis > grenze) b.ausreisser.push_back(t.midi);
        alle.push_back(b);
    }
    std::vector<double> p;
    std::vector<std::size_t> getestet;
    for (std::size_t i = 0; i < alle.size(); ++i)
        if (alle[i].nichtGetroffen > 0)
        {
            p.push_back(alle[i].p);
            getestet.push_back(i);
        }
    const std::vector<bool> v = holm(p, alpha);
    for (std::size_t i = 0; i < getestet.size(); ++i) alle[getestet[i]].systematisch = v[i];
    return alle;
}

HoerZahlen hoerZahlen(const Hoerschwellen& schwellen, const std::vector<TastenSumme>& tasten)
{
    HoerZahlen z;
    for (const TastenSumme& t : tasten)
        for (const Abweichung& a : t.groessen)
        {
            ++z.groessen;
            z.getroffen += a.getroffen ? 1 : 0;
            z.hoerbarGleich += !a.fehlt && hoerbarGleich(schwellen, a.groesse, a.modell, a.ziel) ? 1 : 0;
            z.hoerbarGetroffen += hoerbarGetroffen(schwellen, a) ? 1 : 0;
        }
    return z;
}

}  // namespace lumi::messung
