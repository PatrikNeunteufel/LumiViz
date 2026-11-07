/**
 * @file Tonkern.cpp
 * @brief InstrumentCore: Fabrik der Kerne und `normiere` (IK2-3 a, IK2-10 a)
 */
#include "InstrumentCore/Tonkern.hpp"

#include <algorithm>
#include <cmath>

namespace lumi::kern
{

std::vector<std::string> Fabrik::registriere(KernTyp typ)
{
    std::vector<std::string> fehler = pruefeTafel(typ.tafel);
    if (!typ.erzeuge) fehler.push_back("no create function");
    if (!typ.tafel.kern.empty() && this->typ(typ.tafel.kern) != nullptr) fehler.push_back("core registered twice: " + typ.tafel.kern);
    if (fehler.empty()) m_typen.push_back(std::move(typ));
    return fehler;
}

const KernTyp* Fabrik::typ(std::string_view name) const
{
    for (const KernTyp& t : m_typen)
        if (t.tafel.kern == name) return &t;
    return nullptr;
}

std::vector<std::string> Fabrik::namen() const
{
    std::vector<std::string> n;
    n.reserve(m_typen.size());
    for (const KernTyp& t : m_typen) n.push_back(t.tafel.kern);
    return n;
}

std::shared_ptr<const Tonkern> Fabrik::erzeuge(std::string_view name, Werte werte, std::vector<std::string>* fehler) const
{
    const KernTyp* t = typ(name);
    if (t == nullptr)
    {
        if (fehler != nullptr) fehler->push_back("unknown core: " + std::string(name));
        return nullptr;
    }
    std::vector<std::string> f = richteWerte(t->tafel, werte);
    if (!f.empty())
    {
        if (fehler != nullptr) fehler->insert(fehler->end(), f.begin(), f.end());
        return nullptr;
    }
    return t->erzeuge(werte);
}

double normiere(const Fabrik& fabrik, std::string_view kern, Werte werte, std::string_view bezugName, const Tonauftrag& probe,
                int runden, std::vector<std::string>* fehler)
{
    const KernTyp* t = fabrik.typ(kern);
    const ParameterAngabe* angabe = t != nullptr ? findeAngabe(t->tafel, bezugName) : nullptr;
    if (t != nullptr && (angabe == nullptr || angabe->art != WertArt::Zahl))
    {
        if (fehler != nullptr) fehler->push_back("reference is not a number of the table: " + std::string(bezugName));
        return 0.0;
    }
    double bezug = 1.0;
    for (int runde = 0; runde < std::max(1, runden); ++runde)
    {
        werte.setze(std::string(bezugName), bezug);
        const std::shared_ptr<const Tonkern> k = fabrik.erzeuge(kern, werte, fehler);
        if (!k) return 0.0;
        std::vector<float> ton;
        k->rechne(probe, ton);
        double spitze = 0.0;
        for (const float x : ton) spitze = std::max(spitze, static_cast<double>(std::abs(x)));
        const double roh = spitze * bezug;
        bezug = roh > 0.0 ? roh / kNormSpitze : 1.0;
    }
    return bezug;
}

}  // namespace lumi::kern
