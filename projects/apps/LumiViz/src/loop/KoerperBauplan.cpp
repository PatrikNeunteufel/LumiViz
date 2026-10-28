/**
 * @file KoerperBauplan.cpp
 * @brief Die Datei `.lvinstbodyplan` (Stufe 31c-1, BM3 a; Fassung 2 mit Hoerorten seit 31c-5a, BM6 a)
 */

#include "loop/KoerperBauplan.hpp"

#include "core/Mitgeliefert.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>

#include <limits>

namespace lumi::loop
{

namespace
{

using koerper::Bauplan;
using koerper::Leiste;
using koerper::Material;
using koerper::MaterialArt;
using koerper::Punkt2;
using koerper::Raster;
using koerper::Stegpunkt;

QString text(const std::string& s)
{
    return QString::fromStdString(s);
}

std::string text(const QJsonValue& v)
{
    return v.toString().toStdString();
}

QJsonArray punktAlsJson(Punkt2 p)
{
    return QJsonArray{p.x, p.y};
}

bool punktAusJson(const QJsonValue& v, Punkt2& p)
{
    const QJsonArray a = v.toArray();
    if (a.size() != 2 || !a[0].isDouble() || !a[1].isDouble()) return false;
    p = {a[0].toDouble(), a[1].toDouble()};
    return true;
}

QJsonArray linieAlsJson(const std::vector<Punkt2>& linie)
{
    QJsonArray a;
    for (const Punkt2& p : linie) a.append(punktAlsJson(p));
    return a;
}

bool linieAusJson(const QJsonValue& v, std::vector<Punkt2>& linie)
{
    linie.clear();
    for (const QJsonValue& e : v.toArray())
    {
        Punkt2 p;
        if (!punktAusJson(e, p)) return false;
        linie.push_back(p);
    }
    return true;
}

QJsonArray zahlenAlsJson(const std::vector<double>& zahlen)
{
    // NaN kennt JSON nicht: es steht als null in der Datei
    QJsonArray a;
    for (const double z : zahlen) a.append(std::isnan(z) ? QJsonValue() : QJsonValue(z));
    return a;
}

std::vector<double> zahlenAusJson(const QJsonValue& v)
{
    std::vector<double> zahlen;
    for (const QJsonValue& e : v.toArray())
        zahlen.push_back(e.isDouble() ? e.toDouble() : std::numeric_limits<double>::quiet_NaN());
    return zahlen;
}

QJsonObject materialAlsJson(const Material& m)
{
    return QJsonObject{
        {QStringLiteral("name"), text(m.name)},
        {QStringLiteral("kind"), m.art == MaterialArt::Isotrop ? QStringLiteral("isotropic") : QStringLiteral("orthotropic")},
        {QStringLiteral("density"), m.dichte},
        {QStringLiteral("ex"), m.ex},
        {QStringLiteral("ey"), m.ey},
        {QStringLiteral("gxy"), m.gxy},
        {QStringLiteral("gxz"), m.gxz},
        {QStringLiteral("gyz"), m.gyz},
        {QStringLiteral("nuxy"), m.nuxy},
        {QStringLiteral("lossFactor"), m.verlustfaktor},
        {QStringLiteral("source"), text(m.quelle)},
    };
}

Material materialAusJson(const QJsonObject& o)
{
    Material m;
    m.name = text(o.value(QStringLiteral("name")));
    m.art = o.value(QStringLiteral("kind")).toString() == QLatin1String("isotropic") ? MaterialArt::Isotrop : MaterialArt::Orthotrop;
    m.dichte = o.value(QStringLiteral("density")).toDouble();
    m.ex = o.value(QStringLiteral("ex")).toDouble();
    m.ey = o.value(QStringLiteral("ey")).toDouble();
    m.gxy = o.value(QStringLiteral("gxy")).toDouble();
    m.gxz = o.value(QStringLiteral("gxz")).toDouble();
    m.gyz = o.value(QStringLiteral("gyz")).toDouble();
    m.nuxy = o.value(QStringLiteral("nuxy")).toDouble();
    m.verlustfaktor = o.value(QStringLiteral("lossFactor")).toDouble();
    m.quelle = text(o.value(QStringLiteral("source")));
    return m;
}

QJsonObject rasterAlsJson(const Raster& r)
{
    return QJsonObject{
        {QStringLiteral("origin"), punktAlsJson(r.ursprung)},
        {QStringLiteral("step"), r.schritt},
        {QStringLiteral("nx"), r.nx},
        {QStringLiteral("ny"), r.ny},
        {QStringLiteral("values"), zahlenAlsJson(r.werte)},
    };
}

Raster rasterAusJson(const QJsonObject& o)
{
    Raster r;
    punktAusJson(o.value(QStringLiteral("origin")), r.ursprung);
    r.schritt = o.value(QStringLiteral("step")).toDouble();
    r.nx = o.value(QStringLiteral("nx")).toInt();
    r.ny = o.value(QStringLiteral("ny")).toInt();
    r.werte = zahlenAusJson(o.value(QStringLiteral("values")));
    return r;
}

QJsonObject leisteAlsJson(const Leiste& l)
{
    return QJsonObject{
        {QStringLiteral("name"), text(l.name)},
        {QStringLiteral("material"), text(l.material)},
        {QStringLiteral("line"), linieAlsJson(l.linie)},
        {QStringLiteral("height"), zahlenAlsJson(l.hoehe)},
        {QStringLiteral("width"), zahlenAlsJson(l.breite)},
    };
}

bool leisteAusJson(const QJsonValue& v, Leiste& l)
{
    const QJsonObject o = v.toObject();
    l.name = text(o.value(QStringLiteral("name")));
    l.material = text(o.value(QStringLiteral("material")));
    l.hoehe = zahlenAusJson(o.value(QStringLiteral("height")));
    l.breite = zahlenAusJson(o.value(QStringLiteral("width")));
    return linieAusJson(o.value(QStringLiteral("line")), l.linie);
}

QJsonArray leistenAlsJson(const std::vector<Leiste>& leisten)
{
    QJsonArray a;
    for (const Leiste& l : leisten) a.append(leisteAlsJson(l));
    return a;
}

bool leistenAusJson(const QJsonValue& v, std::vector<Leiste>& leisten)
{
    leisten.clear();
    for (const QJsonValue& e : v.toArray())
    {
        Leiste l;
        if (!leisteAusJson(e, l)) return false;
        leisten.push_back(std::move(l));
    }
    return true;
}

}  // namespace

QJsonObject alsJson(const Bauplan& plan)
{
    QJsonArray materialien;
    for (const Material& m : plan.materialien) materialien.append(materialAlsJson(m));
    QJsonArray punkte;
    for (const Stegpunkt& s : plan.stegpunkte)
    {
        punkte.append(QJsonObject{
            {QStringLiteral("key"), s.taste},
            {QStringLiteral("x"), s.ort.x},
            {QStringLiteral("y"), s.ort.y},
            {QStringLiteral("source"), text(s.quelle)},
        });
    }
    QJsonArray hoerorte;
    for (const koerper::Hoerort& h : plan.hoerorte)
    {
        hoerorte.append(QJsonObject{
            {QStringLiteral("name"), text(h.name)},
            {QStringLiteral("x"), h.ort.x},
            {QStringLiteral("y"), h.ort.y},
            {QStringLiteral("z"), h.hoehe},
            {QStringLiteral("source"), text(h.quelle)},
        });
    }
    const koerper::Platte& p = plan.platte;
    return QJsonObject{
        {QStringLiteral("format"), QLatin1String(kBauplanFormat)},
        {QStringLiteral("version"), koerper::kBauplanFassung},
        {QStringLiteral("title"), text(plan.titel)},
        {QStringLiteral("description"), text(plan.beschreibung)},
        {QStringLiteral("source"), text(plan.quelle)},
        {QStringLiteral("units"), QStringLiteral("SI: m, kg/m^3, Pa")},
        {QStringLiteral("materials"), materialien},
        {QStringLiteral("plate"),
         QJsonObject{
             {QStringLiteral("material"), text(p.material)},
             {QStringLiteral("grainAngle"), p.faserWinkelGrad},
             {QStringLiteral("outline"), linieAlsJson(p.umriss)},
             {QStringLiteral("thickness"), p.dicke},
             {QStringLiteral("thicknessGrid"), rasterAlsJson(p.dickeRaster)},
         }},
        {QStringLiteral("ribs"), leistenAlsJson(plan.rippen)},
        {QStringLiteral("bridges"), leistenAlsJson(plan.stege)},
        {QStringLiteral("bridgePoints"), punkte},
        {QStringLiteral("listeningPoints"), hoerorte},
        // seit Fassung 3 (HU4 a, HU4-1 a), immer geschrieben
        {QStringLiteral("backside"),
         QJsonObject{
             {QStringLiteral("rimDepth"), plan.rueckseite.randtiefe},
             {QStringLiteral("shadowHz"), plan.rueckseite.schattenHz},
             {QStringLiteral("source"), text(plan.rueckseite.quelle)},
         }},
        {QStringLiteral("damping"),
         QJsonObject{
             {QStringLiteral("a"), plan.daempfung.a},
             {QStringLiteral("b"), plan.daempfung.b},
             {QStringLiteral("c"), plan.daempfung.c},
             {QStringLiteral("law"), QStringLiteral("alpha(f) = a f^2 + b f + c in 1/s, f in Hz; eta = alpha / (2 pi f)")},
             {QStringLiteral("source"), text(plan.daempfung.quelle)},
         }},
    };
}

bool ausJson(const QJsonObject& wurzel, Bauplan& plan, bool* zuNeu)
{
    if (zuNeu != nullptr) *zuNeu = false;
    if (wurzel.value(QStringLiteral("format")).toString() != QLatin1String(kBauplanFormat)) return false;
    if (wurzel.value(QStringLiteral("version")).toInt() > koerper::kBauplanFassung)
    {
        if (zuNeu != nullptr) *zuNeu = true;
        return false;
    }
    Bauplan neu;
    neu.titel = text(wurzel.value(QStringLiteral("title")));
    neu.beschreibung = text(wurzel.value(QStringLiteral("description")));
    neu.quelle = text(wurzel.value(QStringLiteral("source")));
    for (const QJsonValue& e : wurzel.value(QStringLiteral("materials")).toArray()) neu.materialien.push_back(materialAusJson(e.toObject()));
    const QJsonObject platte = wurzel.value(QStringLiteral("plate")).toObject();
    neu.platte.material = text(platte.value(QStringLiteral("material")));
    neu.platte.faserWinkelGrad = platte.value(QStringLiteral("grainAngle")).toDouble();
    if (!linieAusJson(platte.value(QStringLiteral("outline")), neu.platte.umriss)) return false;
    neu.platte.dicke = platte.value(QStringLiteral("thickness")).toDouble();
    neu.platte.dickeRaster = rasterAusJson(platte.value(QStringLiteral("thicknessGrid")).toObject());
    if (!leistenAusJson(wurzel.value(QStringLiteral("ribs")), neu.rippen)) return false;
    if (!leistenAusJson(wurzel.value(QStringLiteral("bridges")), neu.stege)) return false;
    for (const QJsonValue& e : wurzel.value(QStringLiteral("bridgePoints")).toArray())
    {
        const QJsonObject o = e.toObject();
        neu.stegpunkte.push_back({o.value(QStringLiteral("key")).toInt(),
                                  {o.value(QStringLiteral("x")).toDouble(), o.value(QStringLiteral("y")).toDouble()},
                                  text(o.value(QStringLiteral("source")))});
    }
    // seit Fassung 2; in Fassung 1 fehlt die Liste — dann keine Hoerorte
    for (const QJsonValue& e : wurzel.value(QStringLiteral("listeningPoints")).toArray())
    {
        const QJsonObject o = e.toObject();
        neu.hoerorte.push_back({text(o.value(QStringLiteral("name"))),
                                {o.value(QStringLiteral("x")).toDouble(), o.value(QStringLiteral("y")).toDouble()},
                                o.value(QStringLiteral("z")).toDouble(),
                                text(o.value(QStringLiteral("source")))});
    }
    // seit Fassung 3; fehlt der Block, bleibt die Rueckseite aus (Schallwand wie bis Fassung 2)
    const QJsonObject rueck = wurzel.value(QStringLiteral("backside")).toObject();
    neu.rueckseite.randtiefe = rueck.value(QStringLiteral("rimDepth")).toDouble();
    neu.rueckseite.schattenHz = rueck.value(QStringLiteral("shadowHz")).toDouble();
    neu.rueckseite.quelle = text(rueck.value(QStringLiteral("source")));
    // seit Fassung 3 (HU4-1 a); fehlt der Block, gilt RT-0425 Gl. 1 mit c = 0 wie bis Fassung 2
    if (wurzel.contains(QStringLiteral("damping")))
    {
        const QJsonObject d = wurzel.value(QStringLiteral("damping")).toObject();
        const koerper::Daempfung vorgabe;
        neu.daempfung.a = d.value(QStringLiteral("a")).toDouble(vorgabe.a);
        neu.daempfung.b = d.value(QStringLiteral("b")).toDouble(vorgabe.b);
        neu.daempfung.c = d.value(QStringLiteral("c")).toDouble(vorgabe.c);
        neu.daempfung.quelle = text(d.value(QStringLiteral("source")));
    }
    plan = std::move(neu);
    return true;
}

bool speichereBauplan(const Bauplan& plan, const QString& pfad)
{
    if (!QDir().mkpath(QFileInfo(pfad).absolutePath())) return false;
    QSaveFile datei(pfad);
    if (!datei.open(QIODevice::WriteOnly)) return false;
    const QJsonDocument doc(alsJson(plan));
    return datei.write(doc.toJson(QJsonDocument::Indented)) >= 0 && datei.commit();
}

bool ladeBauplan(const QString& pfad, Bauplan& plan, bool* zuNeu)
{
    if (zuNeu != nullptr) *zuNeu = false;
    QFile datei(pfad);
    if (!datei.open(QIODevice::ReadOnly)) return false;
    QJsonParseError fehler;
    const QJsonDocument doc = QJsonDocument::fromJson(datei.readAll(), &fehler);
    if (fehler.error != QJsonParseError::NoError || !doc.isObject()) return false;
    return ausJson(doc.object(), plan, zuNeu);
}

QString mitgelieferterKoerperOrdner()
{
    const QString loop = lumi::core::mitgeliefert::ordner(QStringLiteral("loop"));
    if (loop.isEmpty()) return {};
    const QString ordner = loop + QStringLiteral("/instrument_bodies");
    return QFileInfo(ordner).isDir() ? ordner : QString();
}

}  // namespace lumi::loop
