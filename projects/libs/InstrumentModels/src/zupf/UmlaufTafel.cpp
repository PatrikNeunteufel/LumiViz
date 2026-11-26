/**
 * @file UmlaufTafel.cpp
 * @brief InstrumentModels, Kern `pluckedString`: die Tafel des Umlauf-Filters als Liste (siehe Kopf von `UmlaufTafel.hpp`)
 */
#include "InstrumentModels/zupf/UmlaufTafel.hpp"

#include <cstdint>
#include <numbers>
#include <string>

namespace lumi::kern::zupf
{

namespace
{

ParameterAngabe angabe(std::string_view name, std::string_view titel, WertArt art, Wert vorgabe, double von = 0.0, double bis = 1.0)
{
    ParameterAngabe a;
    a.name = std::string(name);
    a.titel = std::string(titel);
    a.art = art;
    a.darstellung = darstellungZu(art);
    a.von = von;
    a.bis = bis;
    a.vorgabe = std::move(vorgabe);
    a.belegkraft = Belegkraft::Rechnung;
    a.quellen = {"lumiviz"};
    return a;
}

ParameterAngabe liste(std::string_view name, std::string_view titel, std::vector<ParameterAngabe> felder, std::size_t max)
{
    ParameterAngabe a = angabe(name, titel, WertArt::Liste, Liste{});
    a.felder = std::move(felder);
    a.eintraegeMax = max;
    return a;
}

/// Eine Zahlenfolge: eine Liste von Eintraegen mit dem Feld `value`.
ParameterAngabe zahlen(std::string_view name, std::string_view titel)
{
    return liste(name, titel, {angabe("value", "Value", WertArt::Zahl, 0.0, -1e12, 1e12)}, 256);
}

Liste listeVon(const std::vector<double>& v)
{
    Liste l;
    for (const double x : v)
    {
        Werte e;
        e.setze("value", x);
        l.eintraege.push_back(std::move(e));
    }
    return l;
}

std::vector<double> folgeVon(const Werte& w, std::string_view name)
{
    std::vector<double> v;
    if (const Wert* x = w.finde(name))
        if (const Liste* l = std::get_if<Liste>(x))
            for (const Werte& e : l->eintraege) v.push_back(e.zahl("value"));
    return v;
}

const Liste* listeIn(const Werte& w, std::string_view name)
{
    const Wert* x = w.finde(name);
    return x != nullptr ? std::get_if<Liste>(x) : nullptr;
}

int ganz(const Werte& w, std::string_view name, int sonst)
{
    if (const Wert* x = w.finde(name))
        if (const auto* g = std::get_if<std::int64_t>(x)) return static_cast<int>(*g);
    return sonst;
}

}  // namespace

ParameterAngabe umlaufTafelAngabe()
{
    constexpr double kPi = std::numbers::pi;
    std::vector<ParameterAngabe> f;
    f.push_back(angabe("key", "Key", WertArt::Ganzzahl, std::int64_t{-1}, -1, 127));
    f.push_back(angabe("course", "Course", WertArt::Ganzzahl, std::int64_t{-1}, -1, 63));
    f.push_back(angabe("string", "String in the course", WertArt::Ganzzahl, std::int64_t{0}, 0, 7));
    f.push_back(angabe("order", "Order of the fractional delay", WertArt::Ganzzahl, std::int64_t{5}, 1, 9));
    f.push_back(angabe("degree", "Degree of the series", WertArt::Ganzzahl, std::int64_t{3}, 0, 9));
    f.push_back(liste("biquads", "Biquads",
                      {angabe("radius", "Pole radius", WertArt::Zahl, 0.5, 0.0, 1.0), angabe("angle", "Pole angle", WertArt::Zahl, 1.0, 0.0, kPi)},
                      256));
    f.push_back(liste("series", "Series per biquad", {zahlen("radius", "logit(r / 0.99)"), zahlen("angle", "logit(θ / π)")}, 256));
    f.push_back(liste("loss", "Loss filter",
                      {angabe("position", "Position in frets", WertArt::Zahl, 0.0, 0.0, 64.0), zahlen("b", "Numerator"), zahlen("a", "Denominator")},
                      256));
    ParameterAngabe t = liste(kUmlaufTafel, "Loop filter table", std::move(f), 512);
    t.eintraegeMin = 1;
    t.herkunft = "tuned from the strings (ZK1 a): per key the chain, loss and order of the Thiran stage (harp, harpsichord); per "
                 "string the Legendre series of the chain and the loss at the frets and half-frets (guitar, lute, oud) — "
                 "Bank & Välimäki 2003, Abel & Smith; the delay N and D are set at the fundamental when the core plays";
    return t;
}

Liste umlaufTafelListe(const std::vector<UmlaufEintrag>& eintraege)
{
    Liste l;
    for (const UmlaufEintrag& u : eintraege)
    {
        Werte e;
        e.setze("key", std::int64_t{u.taste});
        e.setze("course", std::int64_t{u.chor});
        e.setze("string", std::int64_t{u.saite});
        e.setze("order", std::int64_t{u.taste >= 0 ? u.filter.gliedOrdnung : 5});
        e.setze("degree", std::int64_t{u.taste >= 0 ? 0 : u.tafel.grad});
        Liste biquads, reihen, verlust;
        if (u.taste >= 0)
        {
            for (const umlauf::Biquad& q : u.filter.kette)
            {
                Werte b;
                b.setze("radius", q.radius);
                b.setze("angle", q.winkel);
                biquads.eintraege.push_back(std::move(b));
            }
            Werte v;
            v.setze("position", 0.0);
            v.setze("b", listeVon(u.filter.b));
            v.setze("a", listeVon(u.filter.a));
            verlust.eintraege.push_back(std::move(v));
        }
        else
        {
            for (std::size_t m = 0; m < u.tafel.radiusReihe.size(); ++m)
            {
                Werte r;
                r.setze("radius", listeVon(u.tafel.radiusReihe[m]));
                r.setze("angle", listeVon(u.tafel.winkelReihe[m]));
                reihen.eintraege.push_back(std::move(r));
            }
            for (const umlauf::Saitentafel::Verluststelle& s : u.tafel.verlust)
            {
                Werte v;
                v.setze("position", s.lage);
                v.setze("b", listeVon(s.b));
                v.setze("a", listeVon(s.a));
                verlust.eintraege.push_back(std::move(v));
            }
        }
        e.setze("biquads", std::move(biquads));
        e.setze("series", std::move(reihen));
        e.setze("loss", std::move(verlust));
        l.eintraege.push_back(std::move(e));
    }
    return l;
}

std::vector<UmlaufEintrag> umlaufTafelEintraege(const Liste& liste)
{
    std::vector<UmlaufEintrag> out;
    for (const Werte& e : liste.eintraege)
    {
        UmlaufEintrag u;
        u.taste = ganz(e, "key", -1);
        u.chor = ganz(e, "course", -1);
        u.saite = ganz(e, "string", 0);
        const Liste* verlust = listeIn(e, "loss");
        if (u.taste >= 0)
        {
            u.filter.gliedOrdnung = ganz(e, "order", 5);
            if (const Liste* b = listeIn(e, "biquads"))
                for (const Werte& q : b->eintraege) u.filter.kette.push_back({q.zahl("radius"), q.zahl("angle")});
            if (verlust != nullptr && !verlust->eintraege.empty())
            {
                u.filter.b = folgeVon(verlust->eintraege.front(), "b");
                u.filter.a = folgeVon(verlust->eintraege.front(), "a");
            }
        }
        else
        {
            u.tafel.grad = ganz(e, "degree", 3);
            if (const Liste* r = listeIn(e, "series"))
                for (const Werte& m : r->eintraege)
                {
                    u.tafel.radiusReihe.push_back(folgeVon(m, "radius"));
                    u.tafel.winkelReihe.push_back(folgeVon(m, "angle"));
                }
            if (verlust != nullptr)
                for (const Werte& s : verlust->eintraege) u.tafel.verlust.push_back({s.zahl("position"), folgeVon(s, "b"), folgeVon(s, "a")});
        }
        out.push_back(std::move(u));
    }
    return out;
}

}  // namespace lumi::kern::zupf
