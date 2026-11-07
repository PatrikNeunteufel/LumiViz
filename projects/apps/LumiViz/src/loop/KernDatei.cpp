/**
 * @file KernDatei.cpp
 * @brief Loop: `coreParams` nach der Tafel (34d, IK2-4 a)
 */
#include "loop/KernDatei.hpp"

#include <QJsonArray>
#include <QString>

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace lumi::loop
{

namespace
{

QString text(std::string_view s)
{
    return QString::fromUtf8(s.data(), static_cast<qsizetype>(s.size()));
}

QJsonArray paareAlsJson(const std::vector<kern::NotenWert>& paare)
{
    QJsonArray liste;
    for (const kern::NotenWert& p : paare) liste.append(QJsonArray{p.note, p.wert});
    return liste;
}

std::vector<kern::NotenWert> paareAus(const QJsonValue& wert)
{
    std::vector<kern::NotenWert> paare;
    for (const QJsonValue& eintrag : wert.toArray())
    {
        const QJsonArray paar = eintrag.toArray();
        if (paar.size() != 2 || !paar[0].isDouble() || !paar[1].isDouble()) continue;
        paare.push_back(kern::NotenWert{paar[0].toInt(), paar[1].toDouble()});
    }
    return paare;
}

QJsonObject angabenAlsJson(const std::vector<kern::ParameterAngabe>& angaben, const kern::Werte& werte);
kern::Werte angabenAusJson(const std::vector<kern::ParameterAngabe>& angaben, const QJsonObject& o);

QJsonValue wertAlsJson(const kern::ParameterAngabe& a, const kern::Wert& w)
{
    switch (a.art)
    {
    case kern::WertArt::Zahl:
        if (const auto* z = std::get_if<double>(&w)) return *z;
        break;
    case kern::WertArt::Ganzzahl:
        if (const auto* g = std::get_if<std::int64_t>(&w)) return static_cast<qint64>(*g);
        break;
    case kern::WertArt::Schalter:
        if (const auto* b = std::get_if<bool>(&w)) return *b;
        break;
    case kern::WertArt::Wahl:
        if (const auto* s = std::get_if<std::string>(&w)) return QString::fromStdString(*s);
        break;
    case kern::WertArt::Kurve:
        if (const auto* k = std::get_if<kern::Kurve>(&w)) return kurveAlsJson(*k);
        break;
    case kern::WertArt::Liste:
        if (const auto* l = std::get_if<kern::Liste>(&w))
        {
            QJsonArray eintraege;
            for (const kern::Werte& e : l->eintraege) eintraege.append(angabenAlsJson(a.felder, e));
            return eintraege;
        }
        break;
    }
    return QJsonValue();  // eine Art, die nicht passt: null — beim Lesen fehlt der Wert dann
}

bool wertAusJson(const kern::ParameterAngabe& a, const QJsonValue& v, kern::Wert& w)
{
    switch (a.art)
    {
    case kern::WertArt::Zahl:
        if (!v.isDouble()) return false;
        w = v.toDouble();
        return true;
    case kern::WertArt::Ganzzahl:
    {
        if (!v.isDouble()) return false;
        const double d = v.toDouble();
        if (d != std::floor(d)) return false;
        w = static_cast<std::int64_t>(v.toInteger());
        return true;
    }
    case kern::WertArt::Schalter:
        if (!v.isBool()) return false;
        w = v.toBool();
        return true;
    case kern::WertArt::Wahl:
        if (!v.isString()) return false;
        w = v.toString().toStdString();
        return true;
    case kern::WertArt::Kurve:
        if (!v.isObject()) return false;
        w = kurveAusJson(v);
        return true;
    case kern::WertArt::Liste:
    {
        if (!v.isArray()) return false;
        kern::Liste l;
        for (const QJsonValue& e : v.toArray()) l.eintraege.push_back(angabenAusJson(a.felder, e.toObject()));
        w = std::move(l);
        return true;
    }
    }
    return false;
}

QJsonObject angabenAlsJson(const std::vector<kern::ParameterAngabe>& angaben, const kern::Werte& werte)
{
    QJsonObject o;
    for (const kern::ParameterAngabe& a : angaben)
        if (const kern::Wert* w = werte.finde(a.name)) o[QString::fromStdString(a.name)] = wertAlsJson(a, *w);
    return o;
}

kern::Werte angabenAusJson(const std::vector<kern::ParameterAngabe>& angaben, const QJsonObject& o)
{
    kern::Werte werte;
    for (const kern::ParameterAngabe& a : angaben)
    {
        const QJsonValue v = o.value(QString::fromStdString(a.name));
        kern::Wert w;
        if (wertAusJson(a, v, w)) werte.felder.emplace_back(a.name, std::move(w));
    }
    // was die Tafel nicht kennt, wird mitgelesen — `richteWerte` meldet es (als Zahl, damit der Name erhalten bleibt)
    for (auto it = o.begin(); it != o.end(); ++it)
    {
        const std::string name = it.key().toStdString();
        const bool bekannt = std::any_of(angaben.begin(), angaben.end(), [&](const kern::ParameterAngabe& a) { return a.name == name; });
        if (!bekannt) werte.felder.emplace_back(name, it.value().toDouble(0.0));
    }
    return werte;
}

}  // namespace

QJsonObject kurveAlsJson(const kern::Kurve& kurve)
{
    return QJsonObject{{QStringLiteral("points"), paareAlsJson(kurve.punkte)},
                       {QStringLiteral("overrides"), paareAlsJson(kurve.abweichungen)},
                       {QStringLiteral("evidence"), text(kern::belegkraftName(kurve.belegkraft))},
                       {QStringLiteral("source"), QString::fromStdString(kurve.herkunft)}};
}

kern::Kurve kurveAusJson(const QJsonValue& wert)
{
    const QJsonObject o = wert.toObject();
    kern::Kurve kurve;
    kurve.punkte = paareAus(o.value(QStringLiteral("points")));
    kurve.abweichungen = paareAus(o.value(QStringLiteral("overrides")));
    kurve.belegkraft = kern::Belegkraft::Offen;
    (void)kern::belegkraftAusName(o.value(QStringLiteral("evidence")).toString().toStdString(), kurve.belegkraft);
    kurve.herkunft = o.value(QStringLiteral("source")).toString().toStdString();
    return kurve;
}

QJsonObject werteAlsJson(const kern::Parametertafel& tafel, const kern::Werte& werte)
{
    QJsonObject o = angabenAlsJson(tafel.werte, werte);
    if (!werte.gestimmtAus.empty())
    {
        QJsonObject gestimmt;
        for (const auto& [name, abdruck] : werte.gestimmtAus) gestimmt[QString::fromStdString(name)] = QString::fromStdString(abdruck);
        o[QStringLiteral("tuned")] = gestimmt;
    }
    return o;
}

kern::Werte werteAusJson(const kern::Parametertafel& tafel, const QJsonObject& o)
{
    QJsonObject ohne = o;
    ohne.remove(QStringLiteral("tuned"));
    kern::Werte werte = angabenAusJson(tafel.werte, ohne);
    const QJsonObject gestimmt = o.value(QStringLiteral("tuned")).toObject();
    for (auto it = gestimmt.begin(); it != gestimmt.end(); ++it)
        werte.merkeAbdruck(it.key().toStdString(), it.value().toString().toStdString());
    return werte;
}

}  // namespace lumi::loop
