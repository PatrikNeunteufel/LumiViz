/**
 * @file Loop.cpp
 * @brief Loop: Datenmodell und Datei `.lvloop` (Stufe 3)
 */

#include "loop/Loop.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>

#include <algorithm>
#include <cmath>

namespace lumi::loop
{

namespace
{

const QString kFormat = QStringLiteral("lumiviz-loop");
constexpr int kVersion = 1;
const QString kKitVorsatz = QStringLiteral("kit:");

// Pfade wie in der `.lvcomp` (composer/Komposition.cpp): relativ zum Ordner
// der Datei, wenn das Sample darunter liegt, sonst absolut.
QString fuerDatei(const QString& datei, const QString& basisOrdner)
{
    if (datei.isEmpty() || basisOrdner.isEmpty()) return QDir::fromNativeSeparators(datei);
    const QString rel = QDir(basisOrdner).relativeFilePath(datei);
    if (rel.startsWith(QStringLiteral("..")) || QFileInfo(rel).isAbsolute())
        return QDir::fromNativeSeparators(datei);
    return rel;
}

QString ausDatei(const QString& eintrag, const QString& basisOrdner)
{
    if (eintrag.isEmpty()) return {};
    if (QFileInfo(eintrag).isAbsolute() || basisOrdner.isEmpty())
        return QDir::cleanPath(QDir::fromNativeSeparators(eintrag));
    return QDir::cleanPath(QDir(basisOrdner).absoluteFilePath(eintrag));
}

double inGrenzen(double wert, double von, double bis, double ersatz)
{
    return std::isfinite(wert) ? std::clamp(wert, von, bis) : ersatz;
}

}  // namespace

bool istKitKlang(const QString& klang)
{
    return klang.startsWith(kKitVorsatz);
}

const Muster* Loop::findeMuster(const QString& name) const
{
    const auto it = std::find_if(muster.begin(), muster.end(), [&name](const Muster& m) { return m.name == name; });
    return it == muster.end() ? nullptr : &*it;
}

std::vector<const Muster*> Loop::ablaufMuster() const
{
    std::vector<const Muster*> folge;
    for (const QString& name : abfolge) folge.push_back(findeMuster(name));
    if (folge.empty()) folge.push_back(muster.empty() ? nullptr : &muster.front());
    return folge;
}

int Loop::durchlaufSchritte() const
{
    int schritte = 0;
    for (const Muster* m : ablaufMuster()) schritte += (m != nullptr ? m->takte : 1) * schritteJeTakt();
    return schritte;
}

double Loop::durchlaufMs() const
{
    if (bpm <= 0.0 || schritteJeSchlag <= 0) return 0.0;
    return durchlaufSchritte() * 60000.0 / bpm / schritteJeSchlag;
}

void Loop::richte(QStringList* bericht)
{
    bpm = inGrenzen(bpm, kBpmMin, kBpmMax, 120.0);
    schlaegeJeTakt = std::clamp(schlaegeJeTakt, 1, kRasterMax);
    schritteJeSchlag = std::clamp(schritteJeSchlag, 1, kRasterMax);
    swing = inGrenzen(swing, 0.0, 1.0, 0.0);
    laut = inGrenzen(laut, 0.0, 1.0, 0.5);
    for (Spur& spur : spuren)
    {
        spur.laut = inGrenzen(spur.laut, 0.0, 1.0, 1.0);
        spur.pan = inGrenzen(spur.pan, -1.0, 1.0, 0.0);
    }

    QStringList namen;
    for (std::size_t i = 0; i < muster.size(); ++i)
    {
        Muster& m = muster[i];
        if (m.name.isEmpty()) m.name = QStringLiteral("Pattern %1").arg(i + 1);
        if (namen.contains(m.name))
        {
            if (bericht != nullptr) bericht->append(QStringLiteral("Pattern name used twice: %1").arg(m.name));
        }
        else
            namen.append(m.name);
        m.takte = std::clamp(m.takte, 1, kTakteMax);
        m.zeilen.resize(spuren.size());
        const auto schritte = static_cast<std::size_t>(m.takte * schritteJeTakt());
        for (std::vector<double>& zeile : m.zeilen)
        {
            zeile.resize(schritte, 0.0);
            for (double& wert : zeile) wert = inGrenzen(wert, 0.0, 1.0, 0.0);
        }
    }
    if (bericht != nullptr)
        for (const QString& name : abfolge)
            if (!namen.contains(name)) bericht->append(QStringLiteral("Pattern not found: %1").arg(name));
}

QJsonObject alsJson(const Loop& loop, const QString& basisOrdner)
{
    QJsonObject wurzel;
    wurzel[QStringLiteral("format")] = kFormat;
    wurzel[QStringLiteral("version")] = kVersion;
    wurzel[QStringLiteral("bpm")] = loop.bpm;
    wurzel[QStringLiteral("beatsPerBar")] = loop.schlaegeJeTakt;
    wurzel[QStringLiteral("stepsPerBeat")] = loop.schritteJeSchlag;
    wurzel[QStringLiteral("swing")] = loop.swing;
    wurzel[QStringLiteral("volume")] = loop.laut;

    QJsonArray spuren;
    for (const Spur& spur : loop.spuren)
    {
        QJsonObject o;
        o[QStringLiteral("name")] = spur.name;
        o[QStringLiteral("sound")] = istKitKlang(spur.klang) ? spur.klang : fuerDatei(spur.klang, basisOrdner);
        o[QStringLiteral("volume")] = spur.laut;
        o[QStringLiteral("pan")] = spur.pan;
        o[QStringLiteral("mute")] = spur.stumm;
        spuren.append(o);
    }
    wurzel[QStringLiteral("tracks")] = spuren;

    QJsonArray muster;
    for (const Muster& m : loop.muster)
    {
        QJsonObject o;
        o[QStringLiteral("name")] = m.name;
        o[QStringLiteral("bars")] = m.takte;
        QJsonArray zeilen;
        for (const std::vector<double>& zeile : m.zeilen)
        {
            QJsonArray werte;
            for (const double wert : zeile) werte.append(wert);
            zeilen.append(werte);
        }
        o[QStringLiteral("rows")] = zeilen;
        muster.append(o);
    }
    wurzel[QStringLiteral("patterns")] = muster;
    wurzel[QStringLiteral("sequence")] = QJsonArray::fromStringList(loop.abfolge);
    return wurzel;
}

bool ausJson(const QJsonObject& wurzel, Loop& loop, const QString& basisOrdner, QStringList* bericht)
{
    if (wurzel.value(QStringLiteral("format")).toString() != kFormat) return false;
    if (bericht != nullptr && wurzel.value(QStringLiteral("version")).toInt(kVersion) > kVersion)
        bericht->append(QStringLiteral("File was written by a newer version"));

    Loop neu;
    neu.bpm = wurzel.value(QStringLiteral("bpm")).toDouble(neu.bpm);
    neu.schlaegeJeTakt = wurzel.value(QStringLiteral("beatsPerBar")).toInt(neu.schlaegeJeTakt);
    neu.schritteJeSchlag = wurzel.value(QStringLiteral("stepsPerBeat")).toInt(neu.schritteJeSchlag);
    neu.swing = wurzel.value(QStringLiteral("swing")).toDouble(neu.swing);
    neu.laut = wurzel.value(QStringLiteral("volume")).toDouble(neu.laut);

    for (const QJsonValue& sv : wurzel.value(QStringLiteral("tracks")).toArray())
    {
        const QJsonObject o = sv.toObject();
        Spur spur;
        spur.name = o.value(QStringLiteral("name")).toString();
        const QString klang = o.value(QStringLiteral("sound")).toString();
        spur.klang = istKitKlang(klang) ? klang : ausDatei(klang, basisOrdner);
        spur.laut = o.value(QStringLiteral("volume")).toDouble(1.0);
        spur.pan = o.value(QStringLiteral("pan")).toDouble(0.0);
        spur.stumm = o.value(QStringLiteral("mute")).toBool(false);
        if (bericht != nullptr && !spur.klang.isEmpty() && !istKitKlang(spur.klang) &&
            !QFileInfo::exists(spur.klang))
            bericht->append(QStringLiteral("File not found: %1").arg(QDir::toNativeSeparators(spur.klang)));
        neu.spuren.push_back(std::move(spur));
    }

    for (const QJsonValue& mv : wurzel.value(QStringLiteral("patterns")).toArray())
    {
        const QJsonObject o = mv.toObject();
        Muster m;
        m.name = o.value(QStringLiteral("name")).toString();
        m.takte = o.value(QStringLiteral("bars")).toInt(1);
        for (const QJsonValue& zv : o.value(QStringLiteral("rows")).toArray())
        {
            std::vector<double> zeile;
            for (const QJsonValue& wert : zv.toArray()) zeile.push_back(wert.toDouble());
            m.zeilen.push_back(std::move(zeile));
        }
        neu.muster.push_back(std::move(m));
    }

    for (const QJsonValue& name : wurzel.value(QStringLiteral("sequence")).toArray())
        neu.abfolge.append(name.toString());

    neu.richte(bericht);
    loop = std::move(neu);
    return true;
}

bool speichereLoop(const Loop& loop, const QString& pfad)
{
    QSaveFile datei(pfad);
    if (!datei.open(QIODevice::WriteOnly)) return false;
    const QJsonDocument doc(alsJson(loop, QFileInfo(pfad).absolutePath()));
    return datei.write(doc.toJson(QJsonDocument::Indented)) >= 0 && datei.commit();
}

bool ladeLoop(const QString& pfad, Loop& loop, QStringList* bericht)
{
    QFile datei(pfad);
    if (!datei.open(QIODevice::ReadOnly)) return false;
    QJsonParseError fehler;
    const QJsonDocument doc = QJsonDocument::fromJson(datei.readAll(), &fehler);
    if (fehler.error != QJsonParseError::NoError || !doc.isObject()) return false;
    return ausJson(doc.object(), loop, QFileInfo(pfad).absolutePath(), bericht);
}

}  // namespace lumi::loop
