/**
 * @file ZupfStimmen.cpp
 * @brief Stimmen am Kern `pluckedString` (Herleitung im Kopf von `ZupfStimmen.hpp`)
 */
#include "InstrumentMeasure/ZupfStimmen.hpp"

#include "InstrumentCore/Pool.hpp"
#include "InstrumentModels/zupf/ZupfKern.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <vector>

namespace lumi::messung
{

Stimmer zupfStimmer()
{
    using namespace kern::zupf;
    return Stimmer{std::string(kKernName), [](const kern::Fabrik&, const kern::Werte& werte, const std::vector<std::string>& namen) {
                       GestimmteWerte aus;
                       if (std::find(namen.begin(), namen.end(), std::string(kUmlaufTafel)) == namen.end()) return aus;
                       const ZupfSpiel s = spielAus(werte);
                       const UmlaufTafelEntwurf e =
                           s.jeSaite() ? entwirfUmlaufJeSaite(s.satz)
                                       : entwirfUmlaufJeTaste(s.noteMin, s.noteMax, [&](int t) { return tastensaiteAus(s, t); });
                       aus.emplace_back(std::string(kUmlaufTafel), umlaufTafelListe(e.eintraege));
                       return aus;
                   }};
}

// ---------------------------------------------------------------- Rechenfassung 2 (34k-7d-3, ZK1 a)

namespace
{
double sekundenSeit(std::chrono::steady_clock::time_point t0)
{
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
}
}  // namespace

UmlaufTafelEntwurf entwirfUmlaufJeTaste(int von, int bis, const std::function<std::optional<kern::zupf::Tastensaite>(int)>& saite)
{
    const auto t0 = std::chrono::steady_clock::now();
    UmlaufTafelEntwurf e;
    for (int taste = von; taste <= bis; ++taste)
    {
        const std::optional<kern::zupf::Tastensaite> s = saite(taste);
        if (!s) continue;
        const std::string name = "T" + std::to_string(taste);
        const std::optional<umlauf::TafelTaste> t = umlauf::entwirfJeTaste(s->werte, s->laenge);
        if (!t)
        {
            e.fehler.push_back(name);
            continue;
        }
        kern::zupf::UmlaufEintrag u;
        u.taste = taste;
        u.filter.gliedOrdnung = t->filter.gliedOrdnung;
        u.filter.kette = t->filter.kette;
        u.filter.b = t->filter.b;
        u.filter.a = t->filter.a;
        e.eintraege.push_back(std::move(u));
        e.bewertungen.emplace_back(name, t->bewertung);
    }
    e.sekunden = sekundenSeit(t0);
    return e;
}

UmlaufTafelEntwurf entwirfUmlaufJeSaite(const kern::zupf::Saitensatz& satz, int chorVon, int chorBis)
{
    const auto t0 = std::chrono::steady_clock::now();
    UmlaufTafelEntwurf e;
    const std::size_t von = static_cast<std::size_t>(std::max(0, chorVon));
    const std::size_t bis = chorBis < 0 ? satz.choere.size() : std::min(satz.choere.size(), static_cast<std::size_t>(chorBis) + 1);
    for (std::size_t c = von; c < bis; ++c)
        for (std::size_t j = 0; j < satz.choere[c].saiten.size(); ++j)
        {
            const kern::zupf::Chorsaite& z = satz.choere[c].saiten[j];
            const kern::zupf::Chorsaite& erste = satz.choere[c].saiten.front();
            if (j > 0 && z.oktave == erste.oktave && z.werte == erste.werte) continue;  // im Einklang: die Tafel der ersten
            const std::string name = "Chor " + std::to_string(c + 1) + "." + std::to_string(j + 1);
            const std::optional<umlauf::TafelSaite> t = umlauf::entwirfJeSaite(z.werte, satz.mensur, satz.buende);
            if (!t)
            {
                e.fehler.push_back(name);
                continue;
            }
            kern::zupf::UmlaufEintrag u;
            u.chor = static_cast<int>(c);
            u.saite = static_cast<int>(j);
            u.tafel = static_cast<const kern::umlauf::Saitentafel&>(*t);
            u.tafel.buende = 0;  // kommen aus dem Saitensatz
            u.tafel.mensur = 0.0;
            e.eintraege.push_back(std::move(u));
            e.bewertungen.emplace_back(name + " Buende", t->buendeBewertung);
            e.bewertungen.emplace_back(name + " halb", t->halbBewertung);
            e.bewertungen.emplace_back(name + " Achtel", t->achtelBewertung);
            e.bewertungen.emplace_back(name + " fremd", t->fremdBewertung);
        }
    e.sekunden = sekundenSeit(t0);
    return e;
}

}  // namespace lumi::messung
