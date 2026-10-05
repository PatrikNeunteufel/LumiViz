/**
 * @file Komposition.cpp
 * @brief Composer: Datenmodell und Datei `.lvcomp` (Stufe C1b)
 */

#include "composer/Komposition.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>

#include <algorithm>

namespace lumi::composer
{

namespace
{

const QString kFormat = QStringLiteral("lumiviz-composition");
constexpr int kVersion = 1;

/// Datei fuer die Ablage: relativ zum Ordner der `.lvcomp`, wenn sie darunter liegt.
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

double kurvenWert(const PegelPunkt& punkt, double t)
{
    return punkt.form == Kurvenform::Bezier ? bezierWert(punkt.bezier, t) : kurvenWert(punkt.form, t);
}

double VisualClip::pegelBei(std::int64_t zeitImClipMs) const
{
    if (kurve.empty()) return 1.0;
    if (zeitImClipMs <= kurve.front().zeitMs) return std::clamp(kurve.front().pegel, 0.0, 1.0);
    if (zeitImClipMs >= kurve.back().zeitMs) return std::clamp(kurve.back().pegel, 0.0, 1.0);
    for (std::size_t i = 0; i + 1 < kurve.size(); ++i)
    {
        const PegelPunkt& a = kurve[i];
        const PegelPunkt& b = kurve[i + 1];
        if (zeitImClipMs >= b.zeitMs) continue;
        const std::int64_t spanne = b.zeitMs - a.zeitMs;
        const double t = spanne > 0 ? static_cast<double>(zeitImClipMs - a.zeitMs) /
                                          static_cast<double>(spanne)
                                    : 1.0;
        return std::clamp(a.pegel + (b.pegel - a.pegel) * kurvenWert(a, t), 0.0, 1.0);
    }
    return std::clamp(kurve.back().pegel, 0.0, 1.0);
}

void VisualClip::setzeBlenden(std::int64_t einMs, std::int64_t ausMs, Kurvenform form)
{
    kurve.clear();
    const std::int64_t dauer = std::max<std::int64_t>(0, dauerMs());
    einMs = std::clamp<std::int64_t>(einMs, 0, dauer);
    ausMs = std::clamp<std::int64_t>(ausMs, 0, dauer);
    if (einMs + ausMs > dauer)
    {
        // beide Blenden treffen sich in der Mitte der Ueberlappung
        const std::int64_t mitte = (einMs + (dauer - ausMs)) / 2;
        einMs = mitte;
        ausMs = dauer - mitte;
    }
    if (einMs == 0 && ausMs == 0) return;
    if (einMs > 0)
    {
        kurve.push_back({0, 0.0, form});
        kurve.push_back({einMs, 1.0, Kurvenform::Linear});
    }
    if (ausMs > 0)
    {
        if (kurve.empty() || kurve.back().zeitMs < dauer - ausMs)
            kurve.push_back({dauer - ausMs, 1.0, form});
        else
            kurve.back().form = form;
        kurve.push_back({dauer, 0.0, Kurvenform::Linear});
    }
}

std::int64_t Komposition::laenge() const
{
    return laengeMs > 0 ? laengeMs : clipEnde();
}

std::int64_t Komposition::clipEnde() const
{
    std::int64_t ende = 0;
    for (const Kanal& k : kanaele)
        for (const VisualClip& c : k.visual) ende = std::max(ende, c.endeMs);
    return ende;
}

void Komposition::richte(QStringList* bericht)
{
    const auto melde = [bericht](const QString& text) {
        if (bericht != nullptr) bericht->append(text);
    };
    for (Kanal& kanal : kanaele)
    {
        if (kanal.art != KanalArt::Visual) kanal.visual.clear();
        if (kanal.art != KanalArt::Audio) kanal.audio.clear();

        for (VisualClip& c : kanal.visual)
        {
            c.startMs = std::max<std::int64_t>(0, c.startMs);
            if (c.endeMs <= c.startMs)
            {
                melde(QStringLiteral("%1: clip \"%2\" had no length — set to one second")
                          .arg(kanal.name, QFileInfo(c.datei).fileName()));
                c.endeMs = c.startMs + 1000;
            }
            std::stable_sort(c.kurve.begin(), c.kurve.end(),
                             [](const PegelPunkt& a, const PegelPunkt& b) { return a.zeitMs < b.zeitMs; });
            for (PegelPunkt& p : c.kurve)
            {
                p.zeitMs = std::clamp<std::int64_t>(p.zeitMs, 0, c.dauerMs());
                p.pegel = std::clamp(p.pegel, 0.0, 1.0);
            }
        }
        std::stable_sort(kanal.visual.begin(), kanal.visual.end(),
                         [](const VisualClip& a, const VisualClip& b) { return a.startMs < b.startMs; });
        for (std::size_t i = 1; i < kanal.visual.size(); ++i)
        {
            VisualClip& vor = kanal.visual[i - 1];
            VisualClip& c = kanal.visual[i];
            if (c.startMs >= vor.endeMs) continue;
            melde(QStringLiteral("%1: clip \"%2\" overlapped the one before — moved to its end")
                      .arg(kanal.name, QFileInfo(c.datei).fileName()));
            const std::int64_t dauer = c.dauerMs();
            c.startMs = vor.endeMs;
            c.endeMs = std::max(c.endeMs, c.startMs + std::min<std::int64_t>(dauer, 1000));
        }
        for (AudioClip& a : kanal.audio)
        {
            a.startMs = std::max<std::int64_t>(0, a.startMs);
            a.einsatzMs = std::max<std::int64_t>(0, a.einsatzMs);
        }
        std::stable_sort(kanal.audio.begin(), kanal.audio.end(),
                         [](const AudioClip& a, const AudioClip& b) { return a.startMs < b.startMs; });
    }
    laengeMs = std::max<std::int64_t>(0, laengeMs);
    if (rasterSekunden <= 0.0) rasterSekunden = 1.0;
}

QJsonObject alsJson(const Komposition& komposition, const QString& basisOrdner)
{
    QJsonArray kanaele;
    for (const Kanal& kanal : komposition.kanaele)
    {
        QJsonObject k;
        k.insert(QStringLiteral("kind"),
                 kanal.art == KanalArt::Audio ? QStringLiteral("audio") : QStringLiteral("visual"));
        k.insert(QStringLiteral("name"), kanal.name);
        k.insert(QStringLiteral("enabled"), kanal.an);
        QJsonArray clips;
        if (kanal.art == KanalArt::Visual)
        {
            for (const VisualClip& c : kanal.visual)
            {
                QJsonObject o;
                o.insert(QStringLiteral("file"), fuerDatei(c.datei, basisOrdner));
                o.insert(QStringLiteral("start"), static_cast<double>(c.startMs));
                o.insert(QStringLiteral("end"), static_cast<double>(c.endeMs));
                o.insert(QStringLiteral("blend"), static_cast<int>(c.mischart));
                QJsonArray punkte;
                for (const PegelPunkt& p : c.kurve)
                {
                    QJsonObject po;
                    po.insert(QStringLiteral("t"), static_cast<double>(p.zeitMs));
                    po.insert(QStringLiteral("level"), p.pegel);
                    po.insert(QStringLiteral("curve"), static_cast<int>(p.form));
                    if (p.form == Kurvenform::Bezier)  // nur dann: alte Dateien bleiben, wie sie sind
                        po.insert(QStringLiteral("bezier"),
                                  QJsonArray{p.bezier[0], p.bezier[1], p.bezier[2], p.bezier[3]});
                    punkte.append(po);
                }
                o.insert(QStringLiteral("levels"), punkte);
                clips.append(o);
            }
        }
        else
        {
            for (const AudioClip& a : kanal.audio)
            {
                QJsonObject o;
                o.insert(QStringLiteral("file"), fuerDatei(a.datei, basisOrdner));
                o.insert(QStringLiteral("start"), static_cast<double>(a.startMs));
                o.insert(QStringLiteral("offset"), static_cast<double>(a.einsatzMs));
                clips.append(o);
            }
        }
        k.insert(QStringLiteral("clips"), clips);
        kanaele.append(k);
    }
    QJsonObject wurzel;
    wurzel.insert(QStringLiteral("format"), kFormat);
    wurzel.insert(QStringLiteral("version"), kVersion);
    wurzel.insert(QStringLiteral("length"), static_cast<double>(komposition.laengeMs));
    wurzel.insert(QStringLiteral("gridSeconds"), komposition.rasterSekunden);
    wurzel.insert(QStringLiteral("channels"), kanaele);
    return wurzel;
}

bool ausJson(const QJsonObject& wurzel, Komposition& komposition, const QString& basisOrdner,
             QStringList* bericht)
{
    if (wurzel.value(QStringLiteral("format")).toString() != kFormat) return false;
    Komposition neu;
    neu.laengeMs = static_cast<std::int64_t>(wurzel.value(QStringLiteral("length")).toDouble());
    neu.rasterSekunden = wurzel.value(QStringLiteral("gridSeconds")).toDouble(1.0);
    const QJsonArray kanaele = wurzel.value(QStringLiteral("channels")).toArray();
    for (const QJsonValue& kv : kanaele)
    {
        const QJsonObject k = kv.toObject();
        Kanal kanal;
        kanal.art = k.value(QStringLiteral("kind")).toString() == QLatin1String("audio")
                        ? KanalArt::Audio
                        : KanalArt::Visual;
        kanal.name = k.value(QStringLiteral("name")).toString();
        kanal.an = k.value(QStringLiteral("enabled")).toBool(true);
        const QJsonArray clips = k.value(QStringLiteral("clips")).toArray();
        for (const QJsonValue& cv : clips)
        {
            const QJsonObject o = cv.toObject();
            const QString datei = ausDatei(o.value(QStringLiteral("file")).toString(), basisOrdner);
            if (bericht != nullptr && !datei.isEmpty() && !QFileInfo::exists(datei))
                bericht->append(QStringLiteral("File not found: %1").arg(QDir::toNativeSeparators(datei)));
            if (kanal.art == KanalArt::Visual)
            {
                VisualClip c;
                c.datei = datei;
                c.startMs = static_cast<std::int64_t>(o.value(QStringLiteral("start")).toDouble());
                c.endeMs = static_cast<std::int64_t>(o.value(QStringLiteral("end")).toDouble());
                c.mischart = static_cast<Mischart>(
                    std::clamp(o.value(QStringLiteral("blend")).toInt(), 0, kMischarten - 1));
                const QJsonArray punkte = o.value(QStringLiteral("levels")).toArray();
                for (const QJsonValue& pv : punkte)
                {
                    const QJsonObject po = pv.toObject();
                    PegelPunkt p;
                    p.zeitMs = static_cast<std::int64_t>(po.value(QStringLiteral("t")).toDouble());
                    p.pegel = po.value(QStringLiteral("level")).toDouble(1.0);
                    p.form = static_cast<Kurvenform>(
                        std::clamp(po.value(QStringLiteral("curve")).toInt(), 0, kKurvenformen - 1));
                    const QJsonArray griffe = po.value(QStringLiteral("bezier")).toArray();
                    if (griffe.size() == 4)
                        for (int i = 0; i < 4; ++i)
                            p.bezier[static_cast<std::size_t>(i)] =
                                std::clamp(griffe.at(i).toDouble(), 0.0, 1.0);
                    c.kurve.push_back(p);
                }
                kanal.visual.push_back(std::move(c));
            }
            else
            {
                AudioClip a;
                a.datei = datei;
                a.startMs = static_cast<std::int64_t>(o.value(QStringLiteral("start")).toDouble());
                a.einsatzMs = static_cast<std::int64_t>(o.value(QStringLiteral("offset")).toDouble());
                kanal.audio.push_back(std::move(a));
            }
        }
        neu.kanaele.push_back(std::move(kanal));
    }
    neu.richte(bericht);
    // Eine Laenge, die genau das Ende des letzten Clips ist, sagt nichts
    // Eigenes: dann folgt das Ende dem letzten Clip (Befund Patrik S95 — wird
    // ein Clip kuerzer, soll die Komposition nicht leer weiterlaufen). Fest
    // bleibt nur, was davon abweicht.
    if (neu.laengeMs == neu.clipEnde()) neu.laengeMs = 0;
    komposition = std::move(neu);
    return true;
}

bool speichereKomposition(const Komposition& komposition, const QString& pfad)
{
    QSaveFile datei(pfad);
    if (!datei.open(QIODevice::WriteOnly)) return false;
    const QJsonDocument doc(alsJson(komposition, QFileInfo(pfad).absolutePath()));
    return datei.write(doc.toJson(QJsonDocument::Indented)) >= 0 && datei.commit();
}

bool ladeKomposition(const QString& pfad, Komposition& komposition, QStringList* bericht)
{
    QFile datei(pfad);
    if (!datei.open(QIODevice::ReadOnly)) return false;
    QJsonParseError fehler;
    const QJsonDocument doc = QJsonDocument::fromJson(datei.readAll(), &fehler);
    if (fehler.error != QJsonParseError::NoError || !doc.isObject()) return false;
    return ausJson(doc.object(), komposition, QFileInfo(pfad).absolutePath(), bericht);
}

}  // namespace lumi::composer
