/**
 * @file   Hoerschwellen.cpp
 * @brief  Die Rechnung zu `InstrumentMeasure/Hoerschwellen.hpp` (Herleitung dort). Wortgleich aus dem Kopf der
 *         Unit-Tests gezogen (Instrumente_IC_Konzept.md, Teilplan 34e-1): in der .cpp, damit die Schalter nach
 *         E1 a (PRIVATE) gelten.
 */
#include "InstrumentMeasure/Hoerschwellen.hpp"

namespace lumi::messung
{

HoerschwellenGelesen liesHoerschwellen(const std::filesystem::path& pfad)
{
    HoerschwellenGelesen g;
    std::ifstream ein(pfad, std::ios::binary);
    if (!ein)
    {
        g.fehler = "Datei fehlt: " + pfad.string();
        return g;
    }
    Hoerschwellen h;
    std::string zeile;
    int nummer = 0;
    while (std::getline(ein, zeile))
    {
        ++nummer;
        if (!zeile.empty() && zeile.back() == '\r') zeile.pop_back();
        if (zeile.empty() || zeile.front() == '#') continue;
        std::istringstream s(zeile);
        std::vector<std::string> f;
        std::string teil;
        while (std::getline(s, teil, '\t')) f.push_back(teil);
        const auto kaputt = [&](const std::string& was) {
            g.fehler = pfad.string() + " Zeile " + std::to_string(nummer) + ": " + was;
            return g;
        };
        if (f.size() < 4) return kaputt("weniger als vier Spalten");
        Hoerschwelle w;
        w.muster = f[0];
        if (f[1] == "faktor") w.skala = HoerSkala::Faktor;
        else if (f[1] == "db") w.skala = HoerSkala::Dezibel;
        else if (f[1] == "ewert") w.skala = HoerSkala::Ewert;
        else return kaputt("Skala " + f[1] + " unbekannt");
        if (w.skala != HoerSkala::Ewert)
        {
            try
            {
                w.unten = std::stod(f[2]);
                w.oben = std::stod(f[3]);
            }
            catch (const std::exception&)
            {
                return kaputt("Grenzen keine Zahlen");
            }
        }
        if (f.size() > 4) w.quelle = f[4];
        h.zeilen.push_back(std::move(w));
    }
    if (h.zeilen.empty())
    {
        g.fehler = "keine Zeilen in " + pfad.string();
        return g;
    }
    g.werte = std::move(h);
    return g;
}

double ewertM(double ppDb)
{
    return std::tanh(std::max(ppDb, 0.0) * std::numbers::ln10 / 40.0);
}

double ewertPp(double m)
{
    return m >= 1.0 ? std::numeric_limits<double>::infinity() : 40.0 / std::numbers::ln10 * std::atanh(std::max(m, 0.0));
}

double ewertD(double m)
{
    const double lg = m > 0.0 ? 20.0 * std::log10(m) : -std::numeric_limits<double>::infinity();
    if (lg >= -18.0) return 1.2;
    if (lg >= -28.0) return 1.2 + (-18.0 - lg) / 10.0 * (4.1 - 1.2);
    return 4.1;
}

bool ewertGleich(double a, double b)
{
    const double klein = std::min(a, b);
    const double gross = std::max(a, b);
    if (gross < 0.87) return true;
    const double mk = ewertM(klein);
    return gross <= ewertPp(mk * std::pow(10.0, ewertD(mk) / 20.0));
}

bool hoerbarGleich(const Hoerschwellen& schwellen, const std::string& groesse, double modell, double ziel)
{
    const Hoerschwelle* h = schwellen.fuer(groesse);
    if (h == nullptr) return false;
    switch (h->skala)
    {
    case HoerSkala::Faktor:
    {
        if (modell <= 0.0 || ziel <= 0.0) return false;
        const double d = std::log(modell / ziel);
        return -h->unten <= d && d <= h->oben;
    }
    case HoerSkala::Dezibel:
    {
        const double d = modell - ziel;
        return -h->unten <= d && d <= h->oben;
    }
    case HoerSkala::Ewert: return ewertGleich(modell, ziel);
    }
    return false;
}

bool hoerbarGetroffen(const Hoerschwellen& schwellen, const Abweichung& a)
{
    return !a.fehlt && (a.getroffen || hoerbarGleich(schwellen, a.groesse, a.modell, a.ziel));
}

}  // namespace lumi::messung
