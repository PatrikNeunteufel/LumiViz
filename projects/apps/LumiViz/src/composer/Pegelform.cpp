/**
 * @file Pegelform.cpp
 * @brief Composer: Form einer Blende oder eines Mittelstuecks, Platz der Teile
 *        im Clip (Stufe C4 Teil 2)
 */

#include "composer/Pegelform.hpp"

#include <QJsonArray>

#include <algorithm>
#include <cmath>
#include <numeric>

namespace lumi::composer
{

namespace
{

/// Ein Teil beim Verteilen: sein Wunsch und, nach dem Rechnen, sein Platz.
struct Teil
{
    std::int64_t von = 0;
    std::int64_t bis = 0;
    std::int64_t wunsch = 0;
    double ordnung = 0.0;  ///< Reihenfolge in einer Gruppe
    int gruppe = -1;
    Bereich* ziel = nullptr;
};

}  // namespace

double kurvenWert(Kurvenform form, double t)
{
    t = std::clamp(t, 0.0, 1.0);
    switch (form)
    {
        case Kurvenform::SKurve: return t * t * (3.0 - 2.0 * t);
        case Kurvenform::EaseIn: return t * t;
        case Kurvenform::EaseOut: return 1.0 - (1.0 - t) * (1.0 - t);
        case Kurvenform::Exponentiell: return t * t * t;
        case Kurvenform::Halten: return t >= 1.0 ? 1.0 : 0.0;
        case Kurvenform::Bezier: break;  // ohne Anfasser: linear
        case Kurvenform::Linear: break;
    }
    return t;
}

double bezierWert(const BezierGriffe& griffe, double t)
{
    t = std::clamp(t, 0.0, 1.0);
    const double x1 = std::clamp(griffe[0], 0.0, 1.0);
    const double y1 = std::clamp(griffe[1], 0.0, 1.0);
    const double x2 = std::clamp(griffe[2], 0.0, 1.0);
    const double y2 = std::clamp(griffe[3], 0.0, 1.0);
    // Die Kurve ist (x(s), y(s)); gesucht ist y an der Stelle x = t. Mit
    // Anfassern in 0…1 steigt x(s) stetig — Halbieren genuegt.
    const auto achse = [](double a, double b, double s) {
        const double u = 1.0 - s;
        return 3.0 * u * u * s * a + 3.0 * u * s * s * b + s * s * s;
    };
    double von = 0.0;
    double bis = 1.0;
    for (int i = 0; i < 40; ++i)
    {
        const double mitte = 0.5 * (von + bis);
        if (achse(x1, x2, mitte) < t)
            von = mitte;
        else
            bis = mitte;
    }
    return std::clamp(achse(y1, y2, 0.5 * (von + bis)), 0.0, 1.0);
}

double kurvenWert(const FormPunkt& punkt, double t)
{
    return punkt.form == Kurvenform::Bezier ? bezierWert(punkt.bezier, t) : kurvenWert(punkt.form, t);
}

bool einfach(const Pegelform& form)
{
    return form.punkte.size() == 2;
}

double Pegelform::wert(double x) const
{
    if (punkte.empty()) return art == FormArt::Blende ? std::clamp(x, 0.0, 1.0) : 1.0;
    if (x <= punkte.front().x) return std::clamp(punkte.front().y, 0.0, 1.0);
    if (x >= punkte.back().x) return std::clamp(punkte.back().y, 0.0, 1.0);
    for (std::size_t i = 0; i + 1 < punkte.size(); ++i)
    {
        const FormPunkt& a = punkte[i];
        const FormPunkt& b = punkte[i + 1];
        if (x >= b.x) continue;
        const double spanne = b.x - a.x;
        const double t = spanne > 0.0 ? (x - a.x) / spanne : 1.0;
        return std::clamp(a.y + (b.y - a.y) * kurvenWert(a, t), 0.0, 1.0);
    }
    return std::clamp(punkte.back().y, 0.0, 1.0);
}

void Pegelform::richte()
{
    for (FormPunkt& p : punkte)
    {
        p.x = std::clamp(p.x, 0.0, 1.0);
        p.y = std::clamp(p.y, 0.0, 1.0);
        for (double& g : p.bezier) g = std::clamp(g, 0.0, 1.0);
    }
    std::stable_sort(punkte.begin(), punkte.end(),
                     [](const FormPunkt& a, const FormPunkt& b) { return a.x < b.x; });

    const bool blende = art == FormArt::Blende;
    if (punkte.empty() || punkte.front().x > 0.0)
        punkte.insert(punkte.begin(), FormPunkt{0.0, blende ? 0.0 : 1.0});
    // der letzte Punkt bei x = 1 — ein einzelner Punkt bei 0 zaehlt nicht als Ende
    if (punkte.size() < 2 || punkte.back().x < 1.0) punkte.push_back(FormPunkt{1.0, 1.0});
    if (blende)
    {
        punkte.front().y = 0.0;
        punkte.back().y = 1.0;
    }
}

Pegelform einfacheBlende(Kurvenform form, const BezierGriffe& bezier)
{
    Pegelform f;
    f.art = FormArt::Blende;
    f.punkte = {FormPunkt{0.0, 0.0, form, bezier}, FormPunkt{1.0, 1.0}};
    return f;
}

Pegelform delle()
{
    Pegelform f;
    f.art = FormArt::Stueck;
    f.punkte = {FormPunkt{0.0, 1.0, Kurvenform::SKurve}, FormPunkt{0.5, 0.0, Kurvenform::SKurve},
                FormPunkt{1.0, 1.0}};
    return f;
}

double blendenPegel(const Pegelform& form, double x, bool amEnde)
{
    x = std::clamp(x, 0.0, 1.0);
    return form.wert(amEnde ? 1.0 - x : x);
}

double stueckPegel(const Pegelform& form, double x, bool gekippt)
{
    const double pegel = form.wert(std::clamp(x, 0.0, 1.0));
    return gekippt ? 1.0 - pegel : pegel;
}

bool gleich(const Pegelform& a, const Pegelform& b)
{
    return formAlsJson(a) == formAlsJson(b);
}

QJsonObject formAlsJson(const Pegelform& form)
{
    QJsonArray punkte;
    for (const FormPunkt& p : form.punkte)
    {
        QJsonObject po;
        po.insert(QStringLiteral("x"), p.x);
        po.insert(QStringLiteral("y"), p.y);
        po.insert(QStringLiteral("curve"), static_cast<int>(p.form));
        if (p.form == Kurvenform::Bezier)
            po.insert(QStringLiteral("bezier"),
                      QJsonArray{p.bezier[0], p.bezier[1], p.bezier[2], p.bezier[3]});
        punkte.append(po);
    }
    QJsonObject o;
    o.insert(QStringLiteral("kind"), form.art == FormArt::Stueck ? QStringLiteral("shape")
                                                                 : QStringLiteral("fade"));
    o.insert(QStringLiteral("points"), punkte);
    return o;
}

bool formAusJson(const QJsonObject& objekt, Pegelform& form)
{
    if (!objekt.value(QStringLiteral("points")).isArray()) return false;
    Pegelform neu;
    neu.art = objekt.value(QStringLiteral("kind")).toString() == QLatin1String("shape")
                  ? FormArt::Stueck
                  : FormArt::Blende;
    const QJsonArray punkte = objekt.value(QStringLiteral("points")).toArray();
    for (const QJsonValue& pv : punkte)
    {
        const QJsonObject po = pv.toObject();
        FormPunkt p;
        p.x = po.value(QStringLiteral("x")).toDouble();
        p.y = po.value(QStringLiteral("y")).toDouble();
        p.form = static_cast<Kurvenform>(
            std::clamp(po.value(QStringLiteral("curve")).toInt(), 0, kKurvenformen - 1));
        const QJsonArray griffe = po.value(QStringLiteral("bezier")).toArray();
        if (griffe.size() == 4)
            for (int i = 0; i < 4; ++i) p.bezier[static_cast<std::size_t>(i)] = griffe.at(i).toDouble();
        neu.punkte.push_back(p);
    }
    neu.richte();
    form = std::move(neu);
    return true;
}

Verteilung verteile(std::int64_t dauerMs, std::int64_t einMs, std::int64_t ausMs,
                    const std::vector<StueckLage>& stuecke)
{
    const std::int64_t dauer = std::max<std::int64_t>(0, dauerMs);
    Verteilung v;
    v.stuecke.resize(stuecke.size());
    v.aus.vonMs = v.aus.bisMs = dauer;

    // Die Wuensche: jedes Teil dort, wo es ohne die anderen laege.
    std::vector<Teil> teile;
    teile.reserve(stuecke.size() + 2);
    if (einMs > 0) teile.push_back({0, einMs, einMs, -1.0, -1, &v.ein});
    for (std::size_t i = 0; i < stuecke.size(); ++i)
    {
        const double lage = std::clamp(stuecke[i].lage, 0.0, 1.0);
        const auto mitte = static_cast<std::int64_t>(std::llround(lage * static_cast<double>(dauer)));
        const std::int64_t weite = std::max<std::int64_t>(0, stuecke[i].weiteMs);
        // ragt es ueber eine Kante, rueckt es hinein
        const std::int64_t von =
            weite >= dauer ? 0 : std::clamp<std::int64_t>(mitte - weite / 2, 0, dauer - weite);
        v.stuecke[i].vonMs = v.stuecke[i].bisMs = von;
        if (weite > 0) teile.push_back({von, von + weite, weite, lage, -1, &v.stuecke[i]});
    }
    if (ausMs > 0) teile.push_back({dauer - ausMs, dauer, ausMs, 2.0, -1, &v.aus});

    // Gruppen: was sich ueberdeckt, gehoert zusammen (blosses Beruehren nicht).
    const int n = static_cast<int>(teile.size());
    std::vector<int> gruppe(static_cast<std::size_t>(n));
    std::iota(gruppe.begin(), gruppe.end(), 0);
    const auto wurzel = [&gruppe](int i) {
        while (gruppe[static_cast<std::size_t>(i)] != i) i = gruppe[static_cast<std::size_t>(i)];
        return i;
    };
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
        {
            const Teil& a = teile[static_cast<std::size_t>(i)];
            const Teil& b = teile[static_cast<std::size_t>(j)];
            if (a.von < b.bis && b.von < a.bis) gruppe[static_cast<std::size_t>(wurzel(j))] = wurzel(i);
        }
    for (int i = 0; i < n; ++i) teile[static_cast<std::size_t>(i)].gruppe = wurzel(i);

    // Je Gruppe: der Platz, den sie zusammen belegt, im Verhaeltnis der Wuensche.
    std::stable_sort(teile.begin(), teile.end(), [](const Teil& a, const Teil& b) {
        return a.gruppe != b.gruppe ? a.gruppe < b.gruppe : a.ordnung < b.ordnung;
    });
    for (std::size_t anfang = 0; anfang < teile.size();)
    {
        std::size_t ende = anfang;
        std::int64_t von = teile[anfang].von;
        std::int64_t bis = teile[anfang].bis;
        std::int64_t summe = 0;
        for (; ende < teile.size() && teile[ende].gruppe == teile[anfang].gruppe; ++ende)
        {
            von = std::min(von, teile[ende].von);
            bis = std::max(bis, teile[ende].bis);
            summe += teile[ende].wunsch;
        }
        von = std::clamp<std::int64_t>(von, 0, dauer);
        bis = std::clamp<std::int64_t>(bis, von, dauer);
        const std::int64_t platz = bis - von;
        std::int64_t bisher = 0;
        std::int64_t ort = von;
        for (std::size_t i = anfang; i < ende; ++i)
        {
            bisher += teile[i].wunsch;
            // das letzte Teil der Gruppe schliesst buendig ab
            const std::int64_t naechster =
                i + 1 == ende ? bis
                              : von + static_cast<std::int64_t>(std::llround(
                                          static_cast<double>(platz) * static_cast<double>(bisher) /
                                          static_cast<double>(summe)));
            Bereich& b = *teile[i].ziel;
            b.vonMs = ort;
            b.bisMs = std::max(ort, naechster);
            b.gestaucht = b.weiteMs() < teile[i].wunsch;
            ort = b.bisMs;
        }
        anfang = ende;
    }
    return v;
}

}  // namespace lumi::composer
