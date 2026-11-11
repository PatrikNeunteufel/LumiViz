/**
 * @file Stimmen.cpp
 * @brief Gestimmte Werte und der Bezug als Werkzeug (Herleitung im Kopf von `Stimmen.hpp`)
 */
#include "InstrumentMeasure/Stimmen.hpp"

#include "InstrumentMeasure/StabStimmen.hpp"
#include "InstrumentMeasure/ZungeStimmen.hpp"

#include <algorithm>
#include <cmath>

namespace lumi::messung
{

const std::vector<Stimmer>& stimmer()
{
    // die Klassen bringen ihre mit: Klasse 4 in 34g (KS2 a, KS3 a), Klasse 10 in 34j, Klasse 2 in 34k
    static const std::vector<Stimmer> liste{stabStimmer(), zungeStimmer()};  // 34j-3: Klasse 10
    return liste;
}

const Stimmer* stimmerFuer(const std::vector<Stimmer>& liste, std::string_view kern)
{
    const auto it = std::find_if(liste.begin(), liste.end(), [&](const Stimmer& s) { return s.kern == kern; });
    return it != liste.end() ? &*it : nullptr;
}

StimmBericht stimmeUndNormiere(const kern::Fabrik& fabrik, const std::vector<Stimmer>& liste, std::string_view kern, kern::Werte& werte,
                               bool alle, bool bezug)
{
    StimmBericht b;
    const kern::KernTyp* typ = fabrik.typ(kern);
    if (typ == nullptr)
    {
        b.fehler.push_back("unknown core: " + std::string(kern));
        return b;
    }
    const kern::Parametertafel& tafel = typ->tafel;
    std::vector<std::string> zuStimmen;
    for (const kern::ParameterAngabe& a : tafel.werte)
        if (a.gestimmt) zuStimmen.push_back(a.name);
    if (!alle)
    {
        // KS2 a: wird ein gestimmter Wert neu gestimmt, auch jeder spaetere, der von ihm abhaengt (in Reihenfolge)
        const std::vector<std::string> alt = kern::veralteteGestimmte(tafel, werte);
        zuStimmen.clear();
        for (const kern::ParameterAngabe& a : tafel.werte)
        {
            if (!a.gestimmt) continue;
            bool mit = std::find(alt.begin(), alt.end(), a.name) != alt.end();
            for (const std::string& vorher : zuStimmen)
                if (a.abhaengigVon.empty() || std::find(a.abhaengigVon.begin(), a.abhaengigVon.end(), vorher) != a.abhaengigVon.end()) mit = true;
            if (mit) zuStimmen.push_back(a.name);
        }
    }
    if (!zuStimmen.empty())
    {
        const Stimmer* s = stimmerFuer(liste, kern);
        if (s == nullptr || !s->stimme)
        {
            b.fehler.push_back("no tuner for core: " + std::string(kern));
            return b;
        }
        const GestimmteWerte neu = s->stimme(fabrik, werte, zuStimmen);
        for (const std::string& name : zuStimmen)
        {
            const auto it = std::find_if(neu.begin(), neu.end(), [&](const auto& p) { return p.first == name; });
            if (it == neu.end())
            {
                b.fehler.push_back("tuner gave no value: " + name);
                continue;
            }
            werte.setze(name, it->second);
            b.gestimmt.push_back(name);
        }
        // die Abdruecke erst, wenn alle gesetzt sind: ein spaeterer haengt an den neuen Kurven der frueheren (KS2 a; der
        // Stimmer rechnet die Kette selbst in der Reihenfolge der Tafel)
        for (const std::string& name : b.gestimmt) werte.merkeAbdruck(name, kern::eingabenAbdruck(tafel, werte, name));
    }
    if (!tafel.bezugWert.empty())
    {
        b.mitBezug = true;
        if (const kern::Wert* w = werte.finde(tafel.bezugWert); w != nullptr && std::holds_alternative<double>(*w)) b.bezugVorher = std::get<double>(*w);
        b.bezug = b.bezugVorher;
        if (bezug)
        {
            const double neu = kern::normiereNachTafel(fabrik, kern, werte, &b.fehler);
            if (neu > 0.0)
            {
                werte.setze(tafel.bezugWert, neu);
                b.bezug = neu;
            }
        }
    }
    return b;
}

std::vector<std::string> veralteteWerte(const kern::Fabrik& fabrik, std::string_view kern, const kern::Werte& werte)
{
    const kern::KernTyp* typ = fabrik.typ(kern);
    return typ != nullptr ? kern::veralteteGestimmte(typ->tafel, werte) : std::vector<std::string>{};
}

bool bezugVeraltet(const kern::Fabrik& fabrik, std::string_view kern, const kern::Werte& werte, double toleranz, double* neu)
{
    const kern::KernTyp* typ = fabrik.typ(kern);
    if (typ == nullptr || typ->tafel.bezugWert.empty()) return false;
    const double gerechnet = kern::normiereNachTafel(fabrik, kern, werte);
    if (neu != nullptr) *neu = gerechnet;
    const kern::Wert* w = werte.finde(typ->tafel.bezugWert);
    if (w == nullptr || !std::holds_alternative<double>(*w)) return true;
    const double ist = std::get<double>(*w);
    return std::abs(ist - gerechnet) > toleranz * std::max(std::abs(gerechnet), 1e-300);
}

}  // namespace lumi::messung
