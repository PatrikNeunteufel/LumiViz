/**
 * @file Koerper.cpp
 * @brief Die Datei `.lvinstbody` (Stufe 31c-5d, BM3 a)
 */

#include "loop/Koerper.hpp"

#include "loop/KoerperBauplan.hpp"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>

#include <cmath>
#include <complex>

namespace lumi::loop
{

namespace
{

using koerper::Koerper;
using koerper::ModeImKoerper;
using koerper::StatistischerTeil;

QString text(const std::string& s)
{
    return QString::fromStdString(s);
}

std::string text(const QJsonValue& v)
{
    return v.toString().toStdString();
}

QJsonArray zahlenAlsJson(const std::vector<double>& zahlen)
{
    QJsonArray a;
    for (const double z : zahlen) a.append(z);
    return a;
}

bool zahlenAusJson(const QJsonValue& v, std::vector<double>& zahlen)
{
    zahlen.clear();
    for (const QJsonValue& e : v.toArray())
    {
        if (!e.isDouble()) return false;
        zahlen.push_back(e.toDouble());
    }
    return true;
}

QJsonArray komplexAlsJson(const std::vector<std::complex<double>>& zahlen)
{
    QJsonArray a;
    for (const auto& z : zahlen) a.append(QJsonArray{z.real(), z.imag()});
    return a;
}

bool komplexAusJson(const QJsonValue& v, std::vector<std::complex<double>>& zahlen)
{
    zahlen.clear();
    for (const QJsonValue& e : v.toArray())
    {
        const QJsonArray p = e.toArray();
        if (p.size() != 2 || !p[0].isDouble() || !p[1].isDouble()) return false;
        zahlen.emplace_back(p[0].toDouble(), p[1].toDouble());
    }
    return true;
}

}  // namespace

QJsonObject alsJson(const Koerper& k)
{
    QJsonArray tasten;
    for (const int t : k.stegTasten) tasten.append(t);
    QJsonArray orte;
    for (const std::string& o : k.hoerorte) orte.append(text(o));
    QJsonArray moden;
    for (const ModeImKoerper& m : k.moden)
    {
        moden.append(QJsonObject{
            {QStringLiteral("f"), m.frequenzHz},
            {QStringLiteral("eta"), m.verlustfaktor},
            {QStringLiteral("airShare"), m.luftAnteil},
            {QStringLiteral("springShare"), m.federAnteil},  // KQ53 a (Fassung 6): immer geschrieben
            {QStringLiteral("materialShares"), zahlenAlsJson(m.werkstoffAnteil)},
            {QStringLiteral("bridge"), zahlenAlsJson(m.amStegpunkt)},
            {QStringLiteral("bridgeAngle"), m.stegWinkelGrad},  // KQ35-2 a (Fassung 5): immer geschrieben
            {QStringLiteral("radiation"), komplexAlsJson(m.abstrahlung)},
        });
    }
    QJsonArray werkstoffe;
    for (std::size_t w = 0; w < k.werkstoffe.size(); ++w)
        werkstoffe.append(QJsonObject{{QStringLiteral("name"), text(k.werkstoffe[w])},
                                      {QStringLiteral("lossFactor"), w < k.werkstoffVerlust.size() ? k.werkstoffVerlust[w] : 0.0}});
    const StatistischerTeil& s = k.statistik;
    const koerper::Rechnung& r = k.rechnung;
    return QJsonObject{
        {QStringLiteral("format"), QLatin1String(kKoerperFormat)},
        {QStringLiteral("version"), koerper::kKoerperFassung},
        {QStringLiteral("title"), text(k.titel)},
        {QStringLiteral("description"), text(k.beschreibung)},
        {QStringLiteral("units"),
         k.herkunft == koerper::Herkunft::Messung
             ? QStringLiteral("f in Hz; eta loss factor; bridge: mode shape at the bridge point along the direction of "
                              "bridgeAngle, 1/sqrt(kg) (modal mass 1/value^2); bridgeAngle: direction in which the mode "
                              "moves the bridge, degrees from the normal of the top plate, across the strings (normal "
                              "component bridge * cos, in-plane component bridge * sin; 90 = in-plane only); radiation: "
                              "complex pressure per modal acceleration at the listening point, sqrt(kg)/m^2 (fitted to "
                              "the measured pressure per force)")
             : QStringLiteral("f in Hz; eta loss factor; bridge: mode shape at the bridge point along the direction of "
                              "bridgeAngle, 1/sqrt(kg) (modal mass 1/value^2); bridgeAngle: direction in which the mode "
                              "moves the bridge, degrees from the normal of the plate (0 for a body from a plan); "
                              "radiation: complex pressure per modal acceleration at the listening point, sqrt(kg)/m^2 "
                              "(Rayleigh, baffled plate)")},
        // KQ25 a (Fassung 4): woraus der Koerper stammt — beides immer geschrieben
        {QStringLiteral("origin"), k.herkunft == koerper::Herkunft::Messung ? QStringLiteral("measurement") : QStringLiteral("plan")},
        {QStringLiteral("plan"),
         QJsonObject{{QStringLiteral("file"), text(k.bauplan)}, {QStringLiteral("fingerprint"), text(k.bauplanAbdruck)}}},
        {QStringLiteral("measurement"),
         QJsonObject{{QStringLiteral("file"), text(k.messung)},
                     {QStringLiteral("fingerprint"), text(k.messungAbdruck)},
                     {QStringLiteral("fitVersion"), k.fitFassung}}},
        {QStringLiteral("computation"),
         QJsonObject{
             {QStringLiteral("version"), r.fassung},
             {QStringLiteral("mesher"), text(r.netzgeber)},
             {QStringLiteral("solver"), text(r.loeser)},
             {QStringLiteral("order"), r.ordnung},
             {QStringLiteral("meshForHz"), r.netzFuerHz},
             {QStringLiteral("fromHz"), r.vonHz},
             {QStringLiteral("toHz"), r.bisHz},
             {QStringLiteral("unknowns"), r.freieGroessen},
             {QStringLiteral("seconds"), r.sekunden},
         }},
        {QStringLiteral("bridgePoints"), tasten},
        {QStringLiteral("listeningPoints"), orte},
        {QStringLiteral("air"),
         QJsonObject{{QStringLiteral("density"), k.luft.dichte}, {QStringLiteral("speedOfSound"), k.luft.schall}}},
        {QStringLiteral("damping"),
         QJsonObject{
             {QStringLiteral("law"), QStringLiteral("alpha(f) = a f^2 + b f + c in 1/s, f in Hz; eta = alpha / (2 pi f)")},
             {QStringLiteral("a"), k.daempfung.a},
             {QStringLiteral("b"), k.daempfung.b},
             {QStringLiteral("c"), k.daempfung.c},
             {QStringLiteral("source"), text(k.daempfung.quelle)},  // seit HU4-1 a aus dem Bauplan
             {QStringLiteral("perMaterial"),
              QStringLiteral("eta = law + sum over materials with lossFactor > 0 of share * (lossFactor - law); share = strain "
                             "energy of the mode in that material / total (DM1 a, S149)")},
         }},
        {QStringLiteral("materials"), werkstoffe},
        {QStringLiteral("modes"), moden},
        {QStringLiteral("statistical"),
         QJsonObject{
             {QStringLiteral("fromHz"), s.vonHz},
             {QStringLiteral("toHz"), s.bisHz},
             {QStringLiteral("density"), s.dichte},
             {QStringLiteral("rise"), s.anstieg},
             {QStringLiteral("bridgeSpread"), zahlenAlsJson(s.stegStreuung)},
             // KQ35-2 a, KQ35-1 a (Fassung 5): beides immer geschrieben
             {QStringLiteral("bridgeAngle"), s.stegWinkelGrad},
             {QStringLiteral("parallelFactor"), s.parallelAnteil},
             {QStringLiteral("radiation"), zahlenAlsJson(s.hoerBetrag)},
             {QStringLiteral("seed"), QString::number(s.samen)},
             {QStringLiteral("bandFromHz"), s.bandVonHz},
             {QStringLiteral("bandToHz"), s.bandBisHz},
             {QStringLiteral("bandModes"), s.bandModen},
             {QStringLiteral("materialShares"), zahlenAlsJson(s.werkstoffAnteil)},
             {QStringLiteral("damping"),
              QJsonObject{{QStringLiteral("a"), s.daempfung.a}, {QStringLiteral("b"), s.daempfung.b}, {QStringLiteral("c"), s.daempfung.c}}},
         }},
    };
}

bool ausJson(const QJsonObject& wurzel, Koerper& koerper, bool* zuNeu)
{
    if (zuNeu != nullptr) *zuNeu = false;
    if (wurzel.value(QStringLiteral("format")).toString() != QLatin1String(kKoerperFormat)) return false;
    if (wurzel.value(QStringLiteral("version")).toInt() > koerper::kKoerperFassung)
    {
        if (zuNeu != nullptr) *zuNeu = true;
        return false;
    }
    Koerper neu;
    neu.titel = text(wurzel.value(QStringLiteral("title")));
    neu.beschreibung = text(wurzel.value(QStringLiteral("description")));
    const QJsonObject plan = wurzel.value(QStringLiteral("plan")).toObject();
    neu.bauplan = text(plan.value(QStringLiteral("file")));
    neu.bauplanAbdruck = text(plan.value(QStringLiteral("fingerprint")));
    // seit Fassung 4 (KQ25 a); davor immer aus einem Bauplan. Eine unbekannte Herkunft macht die Datei unlesbar.
    const QString herkunft = wurzel.value(QStringLiteral("origin")).toString(QStringLiteral("plan"));
    if (herkunft == QLatin1String("measurement"))
        neu.herkunft = koerper::Herkunft::Messung;
    else if (herkunft != QLatin1String("plan"))
        return false;
    const QJsonObject messung = wurzel.value(QStringLiteral("measurement")).toObject();
    neu.messung = text(messung.value(QStringLiteral("file")));
    neu.messungAbdruck = text(messung.value(QStringLiteral("fingerprint")));
    neu.fitFassung = messung.value(QStringLiteral("fitVersion")).toInt();
    const QJsonObject r = wurzel.value(QStringLiteral("computation")).toObject();
    neu.rechnung.fassung = r.value(QStringLiteral("version")).toInt();
    neu.rechnung.netzgeber = text(r.value(QStringLiteral("mesher")));
    neu.rechnung.loeser = text(r.value(QStringLiteral("solver")));
    neu.rechnung.ordnung = r.value(QStringLiteral("order")).toInt();
    neu.rechnung.netzFuerHz = r.value(QStringLiteral("meshForHz")).toDouble();
    neu.rechnung.vonHz = r.value(QStringLiteral("fromHz")).toDouble();
    neu.rechnung.bisHz = r.value(QStringLiteral("toHz")).toDouble();
    neu.rechnung.freieGroessen = r.value(QStringLiteral("unknowns")).toInt();
    neu.rechnung.sekunden = r.value(QStringLiteral("seconds")).toDouble();
    for (const QJsonValue& e : wurzel.value(QStringLiteral("bridgePoints")).toArray()) neu.stegTasten.push_back(e.toInt());
    for (const QJsonValue& e : wurzel.value(QStringLiteral("listeningPoints")).toArray()) neu.hoerorte.push_back(text(e));
    const QJsonObject luft = wurzel.value(QStringLiteral("air")).toObject();
    neu.luft.dichte = luft.value(QStringLiteral("density")).toDouble();
    neu.luft.schall = luft.value(QStringLiteral("speedOfSound")).toDouble();
    const QJsonObject d = wurzel.value(QStringLiteral("damping")).toObject();
    neu.daempfung = {d.value(QStringLiteral("a")).toDouble(), d.value(QStringLiteral("b")).toDouble(),
                     d.value(QStringLiteral("c")).toDouble(), text(d.value(QStringLiteral("source")))};
    // seit Fassung 2 (DM1 a); Fassung 1 hat keine — dann leer
    for (const QJsonValue& e : wurzel.value(QStringLiteral("materials")).toArray())
    {
        const QJsonObject o = e.toObject();
        neu.werkstoffe.push_back(text(o.value(QStringLiteral("name"))));
        neu.werkstoffVerlust.push_back(o.value(QStringLiteral("lossFactor")).toDouble());
    }
    for (const QJsonValue& e : wurzel.value(QStringLiteral("modes")).toArray())
    {
        const QJsonObject o = e.toObject();
        ModeImKoerper m;
        m.frequenzHz = o.value(QStringLiteral("f")).toDouble();
        m.verlustfaktor = o.value(QStringLiteral("eta")).toDouble();
        m.luftAnteil = o.value(QStringLiteral("airShare")).toDouble();  // seit Fassung 3 (F-K1 a); davor 0
        m.federAnteil = o.value(QStringLiteral("springShare")).toDouble();  // seit Fassung 6 (KQ53 a); davor 0
        if (!(m.federAnteil >= 0.0 && m.federAnteil <= 1.0)) return false;
        if (!zahlenAusJson(o.value(QStringLiteral("materialShares")), m.werkstoffAnteil)) return false;
        if (!zahlenAusJson(o.value(QStringLiteral("bridge")), m.amStegpunkt)) return false;
        m.stegWinkelGrad = o.value(QStringLiteral("bridgeAngle")).toDouble();  // seit Fassung 5 (KQ35-2 a); davor 0
        if (!(std::abs(m.stegWinkelGrad) <= 90.0)) return false;
        if (!komplexAusJson(o.value(QStringLiteral("radiation")), m.abstrahlung)) return false;
        // je Mode so viele Werte, wie es Stegpunkte und Hoerorte gibt — und Werkstoffe, wenn sie Anteile traegt
        if (m.amStegpunkt.size() != neu.stegTasten.size() || m.abstrahlung.size() != neu.hoerorte.size()) return false;
        if (!m.werkstoffAnteil.empty() && m.werkstoffAnteil.size() != neu.werkstoffe.size()) return false;
        neu.moden.push_back(std::move(m));
    }
    const QJsonObject s = wurzel.value(QStringLiteral("statistical")).toObject();
    StatistischerTeil& st = neu.statistik;
    st.vonHz = s.value(QStringLiteral("fromHz")).toDouble();
    st.bisHz = s.value(QStringLiteral("toHz")).toDouble();
    st.dichte = s.value(QStringLiteral("density")).toDouble();
    st.anstieg = s.value(QStringLiteral("rise")).toDouble();
    if (!zahlenAusJson(s.value(QStringLiteral("bridgeSpread")), st.stegStreuung)) return false;
    if (!zahlenAusJson(s.value(QStringLiteral("radiation")), st.hoerBetrag)) return false;
    // seit Fassung 5 (KQ35-2 a, KQ35-1 a); davor 0
    st.stegWinkelGrad = s.value(QStringLiteral("bridgeAngle")).toDouble();
    st.parallelAnteil = s.value(QStringLiteral("parallelFactor")).toDouble();
    if (!(st.stegWinkelGrad >= 0.0 && st.stegWinkelGrad < 90.0) || !(st.parallelAnteil >= 0.0)) return false;
    bool samenGut = false;
    st.samen = s.value(QStringLiteral("seed")).toString().toULongLong(&samenGut);
    if (!samenGut && st.dichte > 0.0) return false;
    st.bandVonHz = s.value(QStringLiteral("bandFromHz")).toDouble();
    st.bandBisHz = s.value(QStringLiteral("bandToHz")).toDouble();
    st.bandModen = s.value(QStringLiteral("bandModes")).toInt();
    const QJsonObject sd = s.value(QStringLiteral("damping")).toObject();
    st.daempfung = {sd.value(QStringLiteral("a")).toDouble(), sd.value(QStringLiteral("b")).toDouble(),
                    sd.value(QStringLiteral("c")).toDouble(), {}};
    if (!zahlenAusJson(s.value(QStringLiteral("materialShares")), st.werkstoffAnteil)) return false;
    if (!st.werkstoffAnteil.empty() && st.werkstoffAnteil.size() != neu.werkstoffe.size()) return false;
    st.werkstoffVerlust = neu.werkstoffVerlust;
    koerper = std::move(neu);
    return true;
}

bool speichereKoerper(const Koerper& koerper, const QString& pfad)
{
    if (!QDir().mkpath(QFileInfo(pfad).absolutePath())) return false;
    QSaveFile datei(pfad);
    if (!datei.open(QIODevice::WriteOnly)) return false;
    const QJsonDocument doc(alsJson(koerper));
    return datei.write(doc.toJson(QJsonDocument::Indented)) >= 0 && datei.commit();
}

bool ladeKoerper(const QString& pfad, Koerper& koerper, bool* zuNeu)
{
    if (zuNeu != nullptr) *zuNeu = false;
    QFile datei(pfad);
    if (!datei.open(QIODevice::ReadOnly)) return false;
    QJsonParseError fehler;
    const QJsonDocument doc = QJsonDocument::fromJson(datei.readAll(), &fehler);
    if (fehler.error != QJsonParseError::NoError || !doc.isObject()) return false;
    return ausJson(doc.object(), koerper, zuNeu);
}

QString messungAbdruck(const QByteArray& inhalt)
{
    // ohne Wagenruecklauf: der Abdruck haengt nicht daran, mit welchen Zeilenenden Git die Datei auscheckt
    QByteArray ohne = inhalt;
    ohne.replace('\r', "");
    return QString::fromLatin1(QCryptographicHash::hash(ohne, QCryptographicHash::Sha256).toHex());
}

QString bauplanAbdruck(const koerper::Bauplan& plan)
{
    const QByteArray json = QJsonDocument(alsJson(plan)).toJson(QJsonDocument::Compact);
    return QString::fromLatin1(QCryptographicHash::hash(json, QCryptographicHash::Sha256).toHex());
}

}  // namespace lumi::loop
