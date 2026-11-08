/**
 * @file   Zielsumme.cpp
 * @brief  Die Rechnung zu `InstrumentMeasure/Zielsumme.hpp` (Herleitung dort). Wortgleich aus dem Kopf der
 *         Unit-Tests gezogen (Instrumente_IC_Konzept.md, Teilplan 34e-1): in der .cpp, damit die Schalter nach
 *         E1 a (PRIVATE) gelten.
 */
#include "InstrumentMeasure/Zielsumme.hpp"

namespace lumi::messung
{

double untergrenze(Skala skala, double ziel)
{
    switch (skala)
    {
    case Skala::Faktor: return kUntergrenzeFaktor;
    case Skala::Dezibel: return kUntergrenzeDb;
    case Skala::Cent: return kUntergrenzeCentAnteil * std::abs(ziel);
    }
    return 0.0;
}

TastenSumme vergleiche(int midi, const TastenZiele& ziel, const std::vector<GroesseJeEbene>& modell)
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

std::vector<GroesseJeEbene> modellGroessen(const std::vector<TonHuellen>& ebenen,
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

std::vector<double> residuen(const std::vector<TastenSumme>& tasten)
{
    std::vector<double> r;
    for (const TastenSumme& t : tasten)
        for (const Abweichung& a : t.groessen) r.push_back(a.residuum);
    return r;
}

Gesamt gesamt(const std::vector<TastenSumme>& tasten)
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

std::string berichte(const std::vector<TastenSumme>& tasten, bool einzeln)
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
