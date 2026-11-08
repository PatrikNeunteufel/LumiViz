/**
 * @file Parametertafel.cpp
 * @brief InstrumentCore: Tafel und Werte eines Kerns (IK2-4 a, IK2-9, IK2-11 a)
 */
#include "InstrumentCore/Parametertafel.hpp"

#include "InstrumentCore/Pruefsumme.hpp"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdio>
#include <stdexcept>

namespace lumi::kern
{

namespace
{

template <typename T>
const T& lies(const Werte& werte, std::string_view name, const char* art)
{
    const Wert* w = werte.finde(name);
    if (w == nullptr) throw std::out_of_range("value missing: " + std::string(name));
    const T* t = std::get_if<T>(w);
    if (t == nullptr) throw std::out_of_range("value " + std::string(name) + " is not a " + art);
    return *t;
}

WertArt artVon(const Wert& w)
{
    switch (w.index())
    {
    case 0: return WertArt::Zahl;
    case 1: return WertArt::Ganzzahl;
    case 2: return WertArt::Schalter;
    case 3: return WertArt::Wahl;
    case 4: return WertArt::Kurve;
    default: return WertArt::Liste;
    }
}

const char* artName(WertArt art)
{
    switch (art)
    {
    case WertArt::Zahl: return "number";
    case WertArt::Ganzzahl: return "integer";
    case WertArt::Schalter: return "switch";
    case WertArt::Wahl: return "choice";
    case WertArt::Kurve: return "curve";
    case WertArt::Liste: return "list";
    }
    return "?";
}

bool darstellungPasst(WertArt art, Darstellung d)
{
    switch (art)
    {
    case WertArt::Zahl: return d == Darstellung::Regler;
    case WertArt::Ganzzahl: return d == Darstellung::Regler || d == Darstellung::Auswahl;
    case WertArt::Schalter: return d == Darstellung::Haken;
    case WertArt::Wahl: return d == Darstellung::Auswahl;
    case WertArt::Kurve: return d == Darstellung::Kurve;
    case WertArt::Liste: return d == Darstellung::Dialog;
    }
    return false;
}

std::string hexVon(double x)
{
    char text[24];
    std::snprintf(text, sizeof text, "%016llx", static_cast<unsigned long long>(std::bit_cast<std::uint64_t>(x)));
    return text;
}

void pruefeAngaben(const std::vector<ParameterAngabe>& angaben, const std::vector<std::string>* gruppen,
                   const std::string& wo, int noteMin, int noteMax, std::vector<std::string>& fehler)
{
    for (std::size_t i = 0; i < angaben.size(); ++i)
    {
        const ParameterAngabe& a = angaben[i];
        const std::string ort = wo + (a.name.empty() ? "#" + std::to_string(i) : a.name);
        if (a.name.empty()) fehler.push_back(ort + ": name empty");
        for (std::size_t j = 0; j < i; ++j)
            if (angaben[j].name == a.name)
            {
                fehler.push_back(ort + ": name twice");
                break;
            }
        if (gruppen != nullptr && std::find(gruppen->begin(), gruppen->end(), a.gruppe) == gruppen->end())
            fehler.push_back(ort + ": group not listed: " + a.gruppe);
        if (!darstellungPasst(a.art, a.darstellung)) fehler.push_back(ort + ": display does not fit the kind");
        if (artVon(a.vorgabe) != a.art)
        {
            fehler.push_back(ort + std::string(": default is not a ") + artName(a.art));
            continue;
        }
        const bool mitGrenzen = a.art == WertArt::Zahl || a.art == WertArt::Ganzzahl || a.art == WertArt::Kurve;
        if (mitGrenzen && !(a.von <= a.bis)) fehler.push_back(ort + ": bounds reversed");
        if (a.art == WertArt::Zahl)
        {
            const double v = std::get<double>(a.vorgabe);
            if (!(v >= a.von && v <= a.bis)) fehler.push_back(ort + ": default outside bounds");
        }
        if (a.art == WertArt::Ganzzahl)
        {
            const auto v = static_cast<double>(std::get<std::int64_t>(a.vorgabe));
            if (!(v >= a.von && v <= a.bis)) fehler.push_back(ort + ": default outside bounds");
        }
        if (a.art == WertArt::Wahl)
        {
            if (a.wahlen.empty()) fehler.push_back(ort + ": no choices");
            const std::string& v = std::get<std::string>(a.vorgabe);
            if (std::find(a.wahlen.begin(), a.wahlen.end(), v) == a.wahlen.end())
                fehler.push_back(ort + ": default not among the choices");
        }
        if (a.art == WertArt::Kurve)
        {
            const Kurve& k = std::get<Kurve>(a.vorgabe);
            if (k.punkte.empty() && !a.gestimmt) fehler.push_back(ort + ": default curve empty");
            for (const NotenWert& p : k.punkte)
            {
                if (p.note < noteMin || p.note > noteMax) fehler.push_back(ort + ": default point outside the keys");
                if (!(p.wert >= a.von && p.wert <= a.bis)) fehler.push_back(ort + ": default point outside bounds");
            }
            if (a.skala == Skala::Logarithmisch && !(a.von > 0.0)) fehler.push_back(ort + ": logarithmic needs bounds above 0");
        }
        if (a.art == WertArt::Liste)
        {
            if (a.felder.empty()) fehler.push_back(ort + ": list without fields");
            if (a.eintraegeMin > a.eintraegeMax) fehler.push_back(ort + ": list bounds reversed");
            const Liste& l = std::get<Liste>(a.vorgabe);
            if (l.eintraege.size() < a.eintraegeMin || l.eintraege.size() > a.eintraegeMax)
                fehler.push_back(ort + ": default list size outside bounds");
            pruefeAngaben(a.felder, nullptr, ort + ".", noteMin, noteMax, fehler);
        }
        if (a.gestimmt && a.art != WertArt::Kurve) fehler.push_back(ort + ": a tuned value must be a curve");
        if (!a.gestimmt && !a.abhaengigVon.empty()) fehler.push_back(ort + ": dependencies without tuning");
    }
}

/// `mitQuellen`: die Werte eines Instruments (QM1 a); an den Eintraegen einer Liste fallen Quellen weg.
std::vector<std::string> richteAngaben(const std::vector<ParameterAngabe>& angaben, int noteMin, int noteMax, const std::string& wo,
                                       Werte& werte, bool mitQuellen)
{
    std::vector<std::string> fehler;
    for (const auto& [name, wert] : werte.felder)
    {
        const bool bekannt = std::any_of(angaben.begin(), angaben.end(), [&](const ParameterAngabe& a) { return a.name == name; });
        if (!bekannt) fehler.push_back(wo + name + ": unknown");
    }
    Werte gerichtet;
    gerichtet.gestimmtAus = werte.gestimmtAus;
    for (const ParameterAngabe& a : angaben)
    {
        if (mitQuellen && a.art != WertArt::Kurve)
        {
            // QM1 a: die Quellen des Werts; fehlen sie, die der Angabe
            const std::vector<std::string>* q = werte.quellenVon(a.name);
            gerichtet.quellen.emplace_back(a.name, q != nullptr ? *q : a.quellen);
        }
        const std::string ort = wo + a.name;
        Wert* w = werte.finde(a.name);
        if (w == nullptr)
        {
            fehler.push_back(ort + ": missing");
            continue;
        }
        // ein Wert mit Fehler bleibt, wie er ist (gemeldet, nicht verworfen)
        if (artVon(*w) != a.art)
        {
            fehler.push_back(ort + std::string(": is not a ") + artName(a.art));
            gerichtet.felder.emplace_back(a.name, std::move(*w));
            continue;
        }
        switch (a.art)
        {
        case WertArt::Zahl:
        {
            double& v = std::get<double>(*w);
            if (!std::isfinite(v))
                fehler.push_back(ort + ": not finite");
            else
                v = std::clamp(v, a.von, a.bis);
            break;
        }
        case WertArt::Ganzzahl:
        {
            auto& v = std::get<std::int64_t>(*w);
            v = std::clamp(v, static_cast<std::int64_t>(std::ceil(a.von)), static_cast<std::int64_t>(std::floor(a.bis)));
            break;
        }
        case WertArt::Schalter: break;
        case WertArt::Wahl:
            if (std::find(a.wahlen.begin(), a.wahlen.end(), std::get<std::string>(*w)) == a.wahlen.end())
                fehler.push_back(ort + ": unknown choice " + std::get<std::string>(*w));
            break;
        case WertArt::Kurve:
        {
            Kurve& k = std::get<Kurve>(*w);
            if (a.gestimmt)
            {
                // eine gestimmte Kurve bekommt keinen Ersatz (IK2-11 a)
                richteKurve(k, a.von, a.bis, Kurve{}, noteMin, noteMax);
                if (k.punkte.empty()) fehler.push_back(ort + ": tuned value empty");
            }
            else
                richteKurve(k, a.von, a.bis, std::get<Kurve>(a.vorgabe), noteMin, noteMax);
            break;
        }
        case WertArt::Liste:
        {
            Liste& l = std::get<Liste>(*w);
            if (l.eintraege.size() < a.eintraegeMin || l.eintraege.size() > a.eintraegeMax)
                fehler.push_back(ort + ": " + std::to_string(l.eintraege.size()) + " entries, allowed " + std::to_string(a.eintraegeMin) +
                                 " ... " + std::to_string(a.eintraegeMax));
            for (std::size_t i = 0; i < l.eintraege.size(); ++i)
            {
                const std::vector<std::string> f =
                    richteAngaben(a.felder, noteMin, noteMax, ort + "[" + std::to_string(i) + "].", l.eintraege[i], false);
                fehler.insert(fehler.end(), f.begin(), f.end());
            }
            break;
        }
        }
        gerichtet.felder.emplace_back(a.name, std::move(*w));
    }
    werte = std::move(gerichtet);
    return fehler;
}

}  // namespace

bool Liste::operator==(const Liste& o) const
{
    return eintraege == o.eintraege;
}

const Wert* Werte::finde(std::string_view name) const
{
    for (const auto& [n, w] : felder)
        if (n == name) return &w;
    return nullptr;
}

Wert* Werte::finde(std::string_view name)
{
    for (auto& [n, w] : felder)
        if (n == name) return &w;
    return nullptr;
}

void Werte::setze(std::string name, Wert wert)
{
    if (Wert* w = finde(name))
        *w = std::move(wert);
    else
        felder.emplace_back(std::move(name), std::move(wert));
}

double Werte::zahl(std::string_view name) const { return lies<double>(*this, name, "number"); }
std::int64_t Werte::ganzzahl(std::string_view name) const { return lies<std::int64_t>(*this, name, "integer"); }
bool Werte::schalter(std::string_view name) const { return lies<bool>(*this, name, "switch"); }
const std::string& Werte::wahl(std::string_view name) const { return lies<std::string>(*this, name, "choice"); }
const Kurve& Werte::kurve(std::string_view name) const { return lies<Kurve>(*this, name, "curve"); }
const Liste& Werte::liste(std::string_view name) const { return lies<Liste>(*this, name, "list"); }

std::string Werte::abdruckVon(std::string_view gestimmt) const
{
    for (const auto& [n, a] : gestimmtAus)
        if (n == gestimmt) return a;
    return {};
}

void Werte::merkeAbdruck(std::string gestimmt, std::string abdruck)
{
    for (auto& [n, a] : gestimmtAus)
        if (n == gestimmt)
        {
            a = std::move(abdruck);
            return;
        }
    gestimmtAus.emplace_back(std::move(gestimmt), std::move(abdruck));
}

const std::vector<std::string>* Werte::quellenVon(std::string_view name) const
{
    for (const auto& [n, q] : quellen)
        if (n == name) return &q;
    return nullptr;
}

void Werte::setzeQuellen(std::string name, std::vector<std::string> schluessel)
{
    for (auto& [n, q] : quellen)
        if (n == name)
        {
            q = std::move(schluessel);
            return;
        }
    quellen.emplace_back(std::move(name), std::move(schluessel));
}

bool Werte::operator==(const Werte& o) const
{
    return felder == o.felder && gestimmtAus == o.gestimmtAus;  // QM1 a: die Quellen zaehlen nicht
}

Darstellung darstellungZu(WertArt art)
{
    switch (art)
    {
    case WertArt::Zahl:
    case WertArt::Ganzzahl: return Darstellung::Regler;
    case WertArt::Schalter: return Darstellung::Haken;
    case WertArt::Wahl: return Darstellung::Auswahl;
    case WertArt::Kurve: return Darstellung::Kurve;
    case WertArt::Liste: return Darstellung::Dialog;
    }
    return Darstellung::Regler;
}

const ParameterAngabe* findeAngabe(const Parametertafel& tafel, std::string_view name)
{
    for (const ParameterAngabe& a : tafel.werte)
        if (a.name == name) return &a;
    return nullptr;
}

std::vector<std::string> pruefeTafel(const Parametertafel& tafel)
{
    std::vector<std::string> fehler;
    if (tafel.kern.empty()) fehler.push_back("core name empty");
    if (tafel.noteMin < kNoteMin || tafel.noteMax > kNoteMax || tafel.noteMin > tafel.noteMax) fehler.push_back("key range invalid");
    for (std::size_t i = 0; i < tafel.gruppen.size(); ++i)
        for (std::size_t j = 0; j < i; ++j)
            if (tafel.gruppen[i] == tafel.gruppen[j]) fehler.push_back("group twice: " + tafel.gruppen[i]);
    pruefeAngaben(tafel.werte, &tafel.gruppen, "", tafel.noteMin, tafel.noteMax, fehler);
    if (!tafel.bezugWert.empty())
    {
        const ParameterAngabe* b = findeAngabe(tafel, tafel.bezugWert);
        if (b == nullptr || b->art != WertArt::Zahl || b->gestimmt) fehler.push_back("reference is not a number of the table: " + tafel.bezugWert);
        if (tafel.bezugRunden < 1) fehler.push_back("reference rounds below 1");
    }
    // KS1 a: der Umfang je Instrument — beide Namen oder keiner, je eine Ganzzahl, Vorgaben im Umfang der Tafel
    if (tafel.umfangVon.empty() != tafel.umfangBis.empty())
        fehler.push_back("key range of the instrument needs both names");
    else if (!tafel.umfangVon.empty())
    {
        std::int64_t umfang[2] = {0, 0};
        bool gut = true;
        for (int i = 0; i < 2; ++i)
        {
            const std::string& name = i == 0 ? tafel.umfangVon : tafel.umfangBis;
            const ParameterAngabe* b = findeAngabe(tafel, name);
            if (b == nullptr || b->art != WertArt::Ganzzahl || b->gestimmt || !std::holds_alternative<std::int64_t>(b->vorgabe))
            {
                fehler.push_back("key range of the instrument is not an integer of the table: " + name);
                gut = false;
                continue;
            }
            umfang[i] = std::get<std::int64_t>(b->vorgabe);
            if (umfang[i] < tafel.noteMin || umfang[i] > tafel.noteMax)
            {
                fehler.push_back("key range of the instrument outside the keys: " + name);
                gut = false;
            }
        }
        if (gut && umfang[0] > umfang[1]) fehler.push_back("key range of the instrument reversed");
    }
    // KS2 a: eine Abhaengigkeit ist ein nicht gestimmter Wert oder ein gestimmter, der vor dem Wert steht
    for (std::size_t i = 0; i < tafel.werte.size(); ++i)
    {
        const ParameterAngabe& a = tafel.werte[i];
        for (const std::string& von : a.abhaengigVon)
        {
            const auto it = std::find_if(tafel.werte.begin(), tafel.werte.end(), [&](const ParameterAngabe& b) { return b.name == von; });
            if (it == tafel.werte.end())
                fehler.push_back(a.name + ": dependency unknown: " + von);
            else if (it->gestimmt && static_cast<std::size_t>(it - tafel.werte.begin()) >= i)
                fehler.push_back(a.name + ": dependency is tuned and not before it: " + von);
        }
    }
    return fehler;
}

std::pair<int, int> tastenumfang(const Parametertafel& tafel, const Werte& werte)
{
    const std::pair<int, int> ganz{tafel.noteMin, tafel.noteMax};
    if (tafel.umfangVon.empty() || tafel.umfangBis.empty()) return ganz;
    const Wert* von = werte.finde(tafel.umfangVon);
    const Wert* bis = werte.finde(tafel.umfangBis);
    if (von == nullptr || bis == nullptr || !std::holds_alternative<std::int64_t>(*von) || !std::holds_alternative<std::int64_t>(*bis)) return ganz;
    const auto klemme = [&](std::int64_t n) { return static_cast<int>(std::clamp<std::int64_t>(n, tafel.noteMin, tafel.noteMax)); };
    const int a = klemme(std::get<std::int64_t>(*von));
    const int b = klemme(std::get<std::int64_t>(*bis));
    if (a > b) return ganz;
    return {a, b};
}

Werte vorgaben(const Parametertafel& tafel)
{
    Werte werte;
    for (const ParameterAngabe& a : tafel.werte)
    {
        werte.felder.emplace_back(a.name, a.vorgabe);
        if (a.art != WertArt::Kurve) werte.quellen.emplace_back(a.name, a.quellen);  // QM1 a
    }
    return werte;
}

std::vector<std::string> richteWerte(const Parametertafel& tafel, Werte& werte)
{
    // KS1 a: die Kurven richten sich nach dem Umfang des Instruments; ist er verkehrt, ein Fehler und der der Tafel
    std::vector<std::string> fehler;
    if (!tafel.umfangVon.empty() && !tafel.umfangBis.empty())
    {
        const Wert* von = werte.finde(tafel.umfangVon);
        const Wert* bis = werte.finde(tafel.umfangBis);
        if (von != nullptr && bis != nullptr && std::holds_alternative<std::int64_t>(*von) && std::holds_alternative<std::int64_t>(*bis) &&
            std::clamp<std::int64_t>(std::get<std::int64_t>(*von), tafel.noteMin, tafel.noteMax) >
                std::clamp<std::int64_t>(std::get<std::int64_t>(*bis), tafel.noteMin, tafel.noteMax))
            fehler.push_back(tafel.umfangVon + ", " + tafel.umfangBis + ": key range reversed");
    }
    const auto [noteMin, noteMax] = tastenumfang(tafel, werte);
    std::vector<std::string> f = richteAngaben(tafel.werte, noteMin, noteMax, "", werte, true);
    fehler.insert(fehler.end(), f.begin(), f.end());
    return fehler;
}

std::string wertAlsText(const Wert& wert)
{
    struct Schreiber
    {
        std::string operator()(double x) const { return "d" + hexVon(x); }
        std::string operator()(std::int64_t x) const { return "i" + std::to_string(x); }
        std::string operator()(bool x) const { return x ? "b1" : "b0"; }
        std::string operator()(const std::string& x) const { return "s" + std::to_string(x.size()) + ":" + x; }
        std::string operator()(const Kurve& k) const
        {
            std::string t = "k" + std::to_string(k.punkte.size());
            for (const NotenWert& p : k.punkte) t += " " + std::to_string(p.note) + "=" + hexVon(p.wert);
            t += " a" + std::to_string(k.abweichungen.size());
            for (const NotenWert& p : k.abweichungen) t += " " + std::to_string(p.note) + "=" + hexVon(p.wert);
            return t;
        }
        std::string operator()(const Liste& l) const
        {
            std::string t = "l" + std::to_string(l.eintraege.size()) + "[";
            for (const Werte& e : l.eintraege)
            {
                t += "{";
                for (const auto& [n, w] : e.felder) t += n + ":" + std::visit(*this, w) + ";";
                t += "}";
            }
            return t + "]";
        }
    };
    return std::visit(Schreiber{}, wert);
}

std::string eingabenAbdruck(const Parametertafel& tafel, const Werte& werte, std::string_view gestimmt)
{
    const ParameterAngabe* ziel = findeAngabe(tafel, gestimmt);
    if (ziel == nullptr || !ziel->gestimmt) return {};
    Sha256 sha;
    sha.dazu("core " + tafel.kern + "\nfassung " + std::to_string(tafel.rechenFassung) + "\n");
    bool vorDemZiel = true;  // KS2 a: gestimmte Werte zaehlen, wenn sie vor dem Ziel stehen
    for (const ParameterAngabe& a : tafel.werte)
    {
        if (&a == ziel) vorDemZiel = false;
        if (a.gestimmt && !vorDemZiel) continue;
        if (ziel->abhaengigVon.empty() && a.name == tafel.bezugWert) continue;  // 34f-3: der Bezug haengt am Gestimmten
        if (!ziel->abhaengigVon.empty() &&
            std::find(ziel->abhaengigVon.begin(), ziel->abhaengigVon.end(), a.name) == ziel->abhaengigVon.end())
            continue;
        const Wert* w = werte.finde(a.name);
        sha.dazu(a.name + " " + (w != nullptr ? wertAlsText(*w) : std::string("-")) + "\n");
    }
    return sha.fertig();
}

std::vector<std::string> veralteteGestimmte(const Parametertafel& tafel, const Werte& werte)
{
    std::vector<std::string> alt;
    for (const ParameterAngabe& a : tafel.werte)
        if (a.gestimmt && werte.abdruckVon(a.name) != eingabenAbdruck(tafel, werte, a.name)) alt.push_back(a.name);
    return alt;
}

bool ungemessen(const Parametertafel& tafel, const Werte& werte)
{
    for (const ParameterAngabe& a : tafel.werte)
    {
        if (!a.klangbestimmend) continue;
        if (a.art == WertArt::Kurve)
        {
            const Wert* w = werte.finde(a.name);
            const Kurve* k = w != nullptr ? std::get_if<Kurve>(w) : nullptr;
            if (k == nullptr || k->belegkraft != Belegkraft::Gemessen) return true;
        }
        else if (a.belegkraft != Belegkraft::Gemessen)
            return true;
    }
    return false;
}

std::string_view belegkraftName(Belegkraft belegkraft)
{
    switch (belegkraft)
    {
    case Belegkraft::Quelle: return "source";
    case Belegkraft::Rechnung: return "computed";
    case Belegkraft::Gemessen: return "measured";
    case Belegkraft::Offen: return "open";
    }
    return "open";
}

bool belegkraftAusName(std::string_view name, Belegkraft& belegkraft)
{
    for (const Belegkraft b : {Belegkraft::Quelle, Belegkraft::Rechnung, Belegkraft::Gemessen, Belegkraft::Offen})
        if (belegkraftName(b) == name)
        {
            belegkraft = b;
            return true;
        }
    return false;
}

}  // namespace lumi::kern
