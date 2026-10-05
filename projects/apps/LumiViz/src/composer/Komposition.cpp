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
constexpr int kVersion = 2;  // 2 (S96): Blenden und Stuecke statt der Punktliste `levels`

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

/// Die Form einer Ausblendung aus einer alten Datei. Dort fiel der Pegel mit
/// der Kurvenform von 1 auf 0 (1 − f(t)); heute laeuft eine Blende als
/// Ausblendung rueckwaerts (g(1 − t)). Derselbe Verlauf braucht also
/// g(u) = 1 − f(1 − u): die Form am Mittelpunkt gespiegelt.
Pegelform alteAusblende(Kurvenform form, const BezierGriffe& griffe)
{
    switch (form)
    {
        case Kurvenform::EaseIn: return einfacheBlende(Kurvenform::EaseOut);
        case Kurvenform::EaseOut: return einfacheBlende(Kurvenform::EaseIn);
        case Kurvenform::Exponentiell:  // 1 − (1 − u)^3, als Bézier genau
            return einfacheBlende(Kurvenform::Bezier, {1.0 / 3.0, 1.0, 2.0 / 3.0, 1.0});
        case Kurvenform::Bezier:
            return einfacheBlende(Kurvenform::Bezier,
                                  {1.0 - griffe[2], 1.0 - griffe[3], 1.0 - griffe[0], 1.0 - griffe[1]});
        case Kurvenform::Halten:
        {
            // voll bis zum Schluss: rueckwaerts gelesen ein Sprung am Anfang
            Pegelform sprung;
            sprung.punkte = {FormPunkt{0.0, 0.0}, FormPunkt{0.0, 1.0}, FormPunkt{1.0, 1.0}};
            return sprung;
        }
        case Kurvenform::Linear:
        case Kurvenform::SKurve: break;  // in sich symmetrisch
    }
    return einfacheBlende(form);
}

}  // namespace

Verteilung VisualClip::verteilung() const
{
    std::vector<StueckLage> lagen;
    lagen.reserve(stuecke.size());
    for (const ClipStueck& s : stuecke) lagen.push_back({s.lage, s.weiteMs});
    return verteile(dauerMs(), ein.dauerMs, aus.dauerMs, lagen);
}

double VisualClip::pegelBei(std::int64_t zeitImClipMs) const
{
    const double grund = std::clamp(grundpegel, 0.0, 1.0);
    if (ein.dauerMs <= 0 && aus.dauerMs <= 0 && stuecke.empty()) return grund;

    const Verteilung v = verteilung();
    // Anteil 0…1 im Bereich
    const auto anteil = [zeitImClipMs](const Bereich& b) {
        return std::clamp(static_cast<double>(zeitImClipMs - b.vonMs) / static_cast<double>(b.weiteMs()),
                          0.0, 1.0);
    };
    if (v.ein.weiteMs() > 0 && zeitImClipMs < v.ein.bisMs)
        return grund * blendenPegel(ein.form, anteil(v.ein), false);
    if (v.aus.weiteMs() > 0 && zeitImClipMs >= v.aus.vonMs)
        return grund * blendenPegel(aus.form, anteil(v.aus), true);
    for (std::size_t i = 0; i < stuecke.size(); ++i)
    {
        const Bereich& b = v.stuecke[i];
        // ein Stueck, das bis ans Clip-Ende reicht, gilt auch genau dort
        const bool hinter = zeitImClipMs >= b.bisMs && b.bisMs < dauerMs();
        if (b.weiteMs() <= 0 || zeitImClipMs < b.vonMs || hinter) continue;
        // die Form: 1 = der Grundpegel bleibt, 0 = ganz ausgelenkt
        const double w = stuecke[i].form.wert(anteil(b));
        return stuecke[i].gekippt ? grund + (1.0 - grund) * (1.0 - w) : grund * w;
    }
    return grund;
}

void VisualClip::setzeBlenden(std::int64_t einMs, std::int64_t ausMs, Kurvenform form)
{
    ein = {std::max<std::int64_t>(0, einMs), einfacheBlende(form)};
    aus = {std::max<std::int64_t>(0, ausMs), einfacheBlende(form)};
}

bool VisualClip::vollerPegel() const
{
    return grundpegel >= 1.0 && ein.dauerMs <= 0 && aus.dauerMs <= 0 && stuecke.empty();
}

void ausAltenPunkten(VisualClip& clip, std::vector<AltPunkt> punkte)
{
    clip.grundpegel = 1.0;
    clip.ein = {};
    clip.aus = {};
    clip.stuecke.clear();
    if (punkte.empty()) return;

    const std::int64_t dauer = std::max<std::int64_t>(1, clip.dauerMs());
    std::stable_sort(punkte.begin(), punkte.end(),
                     [](const AltPunkt& a, const AltPunkt& b) { return a.zeitMs < b.zeitMs; });
    for (AltPunkt& p : punkte)
    {
        p.zeitMs = std::clamp<std::int64_t>(p.zeitMs, 0, dauer);
        p.pegel = std::clamp(p.pegel, 0.0, 1.0);
    }

    // Das Muster, das die Blenden-Griffe bis S95 schrieben: zwei Punkte = eine
    // Seite; drei = beide, in der Mitte vereint; vier = beide.
    const auto null = [](double pegel) { return pegel < 0.001; };
    const auto voll = [](double pegel) { return pegel > 0.999; };
    const std::size_t n = punkte.size();
    if (n >= 2 && n <= 4)
    {
        const bool istEin = punkte[0].zeitMs == 0 && null(punkte[0].pegel) && voll(punkte[1].pegel);
        const bool istAus = punkte[n - 1].zeitMs == dauer && null(punkte[n - 1].pegel) &&
                            voll(punkte[n - 2].pegel);
        if ((n == 2 && istEin != istAus) || (n >= 3 && istEin && istAus))
        {
            if (istEin)
                clip.ein = {punkte[1].zeitMs, einfacheBlende(punkte[0].form, punkte[0].bezier)};
            if (istAus)
                clip.aus = {dauer - punkte[n - 2].zeitMs,
                            alteAusblende(punkte[n - 2].form, punkte[n - 2].bezier)};
            return;
        }
    }

    // Jede andere Kurve: ein Stueck ueber den ganzen Clip, Punkt fuer Punkt.
    ClipStueck stueck;
    stueck.lage = 0.5;
    stueck.weiteMs = dauer;
    stueck.form.art = FormArt::Stueck;
    stueck.form.punkte.clear();
    if (punkte.front().zeitMs > 0)  // vor dem ersten Punkt galt dessen Pegel
        stueck.form.punkte.push_back(FormPunkt{0.0, punkte.front().pegel});
    for (const AltPunkt& p : punkte)
        stueck.form.punkte.push_back(FormPunkt{static_cast<double>(p.zeitMs) / static_cast<double>(dauer),
                                               p.pegel, p.form, p.bezier});
    if (punkte.back().zeitMs < dauer)  // nach dem letzten der letzte
    {
        stueck.form.punkte.back().form = Kurvenform::Linear;
        stueck.form.punkte.push_back(FormPunkt{1.0, punkte.back().pegel});
    }
    clip.stuecke.push_back(std::move(stueck));
}

std::int64_t Komposition::laenge() const
{
    return laengeMs > 0 ? laengeMs : clipEnde();
}

std::int64_t Komposition::clipEnde() const
{
    std::int64_t ende = 0;
    for (const Kanal& k : kanaele)
        for (const VisualClip& c : k.clips) ende = std::max(ende, c.endeMs);
    return ende;
}

void Komposition::richte(QStringList* bericht)
{
    const auto melde = [bericht](const QString& text) {
        if (bericht != nullptr) bericht->append(text);
    };
    for (Kanal& kanal : kanaele)
    {
        const bool ton = kanal.art == KanalArt::Audio;
        for (Clip& c : kanal.clips)
        {
            c.startMs = std::max<std::int64_t>(0, c.startMs);
            // Einsatz und Song-Laenge gibt es nur am Ton
            c.einsatzMs = ton ? std::max<std::int64_t>(0, c.einsatzMs) : 0;
            c.quellMs = ton ? std::max<std::int64_t>(0, c.quellMs) : 0;
            if (c.endeMs <= c.startMs)
            {
                melde(QStringLiteral("%1: clip \"%2\" had no length — set to one second")
                          .arg(kanal.name, QFileInfo(c.datei).fileName()));
                c.endeMs = c.startMs + 1000;
            }
            // Blenden und Stuecke: Unsinn klemmen. Weiten ueber die Clip-Laenge
            // hinaus bleiben — sie sind eingestellt, gestaucht wird beim Rechnen.
            c.grundpegel = std::clamp(c.grundpegel, 0.0, 1.0);
            for (ClipBlende* b : {&c.ein, &c.aus})
            {
                b->dauerMs = std::max<std::int64_t>(0, b->dauerMs);
                b->form.art = FormArt::Blende;
                b->form.richte();
            }
            for (ClipStueck& s : c.stuecke)
            {
                s.lage = std::clamp(s.lage, 0.0, 1.0);
                s.weiteMs = std::max<std::int64_t>(0, s.weiteMs);
                s.form.art = FormArt::Stueck;
                s.form.richte();
            }
        }
        std::stable_sort(kanal.clips.begin(), kanal.clips.end(),
                         [](const VisualClip& a, const VisualClip& b) { return a.startMs < b.startMs; });
        for (std::size_t i = 1; i < kanal.clips.size(); ++i)
        {
            VisualClip& vor = kanal.clips[i - 1];
            VisualClip& c = kanal.clips[i];
            if (c.startMs >= vor.endeMs) continue;
            melde(QStringLiteral("%1: clip \"%2\" overlapped the one before — moved to its end")
                      .arg(kanal.name, QFileInfo(c.datei).fileName()));
            const std::int64_t dauer = c.dauerMs();
            c.startMs = vor.endeMs;
            c.endeMs = std::max(c.endeMs, c.startMs + std::min<std::int64_t>(dauer, 1000));
        }
    }
    laengeMs = std::max<std::int64_t>(0, laengeMs);
    if (rasterSekunden <= 0.0) rasterSekunden = 1.0;
}

int Komposition::tonKanal() const
{
    for (int k = 0; k < static_cast<int>(kanaele.size()); ++k)
        if (kanaele[static_cast<std::size_t>(k)].art == KanalArt::Audio && kanaele[static_cast<std::size_t>(k)].an)
            return k;
    return -1;
}

const Clip* Komposition::tonBei(std::int64_t stelleMs) const
{
    const int k = tonKanal();
    if (k < 0) return nullptr;
    for (const Clip& c : kanaele[static_cast<std::size_t>(k)].clips)
        if (stelleMs >= c.startMs && stelleMs < c.endeMs) return &c;
    return nullptr;
}

void Komposition::ergaenzeTon(const SongLaenge& laengeVon)
{
    for (Kanal& kanal : kanaele)
    {
        if (kanal.art != KanalArt::Audio) continue;
        for (Clip& c : kanal.clips)
        {
            if (c.quellMs <= 0 && laengeVon) c.quellMs = std::max<std::int64_t>(0, laengeVon(c.datei));
            if (c.quellMs <= 0) continue;  // unbekannt: der Clip bleibt, wie er ist
            // der Einsatz liegt im Song, und der Clip hoert spaetestens mit ihm auf
            c.einsatzMs = std::clamp<std::int64_t>(c.einsatzMs, 0, c.quellMs);
            const std::int64_t rest = c.quellMs - c.einsatzMs;
            if (c.dauerMs() > rest) c.endeMs = c.startMs + std::max<std::int64_t>(1, rest);
        }
    }
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
        const bool ton = kanal.art == KanalArt::Audio;
        {
            for (const Clip& c : kanal.clips)
            {
                QJsonObject o;
                o.insert(QStringLiteral("file"), fuerDatei(c.datei, basisOrdner));
                o.insert(QStringLiteral("start"), static_cast<double>(c.startMs));
                o.insert(QStringLiteral("end"), static_cast<double>(c.endeMs));
                if (ton)
                {
                    o.insert(QStringLiteral("offset"), static_cast<double>(c.einsatzMs));
                    if (c.quellMs > 0) o.insert(QStringLiteral("sourceLength"), static_cast<double>(c.quellMs));
                }
                else
                    o.insert(QStringLiteral("blend"), static_cast<int>(c.mischart));
                // Pegelkurve: nur, was der Clip hat — alle Teile sind freiwillig
                if (c.grundpegel < 1.0) o.insert(QStringLiteral("base"), c.grundpegel);
                const auto blende = [](const ClipBlende& b) {
                    QJsonObject bo;
                    bo.insert(QStringLiteral("ms"), static_cast<double>(b.dauerMs));
                    bo.insert(QStringLiteral("shape"), formAlsJson(b.form));
                    return bo;
                };
                if (c.ein.dauerMs > 0) o.insert(QStringLiteral("fadeIn"), blende(c.ein));
                if (c.aus.dauerMs > 0) o.insert(QStringLiteral("fadeOut"), blende(c.aus));
                if (!c.stuecke.empty())
                {
                    QJsonArray stuecke;
                    for (const ClipStueck& s : c.stuecke)
                    {
                        QJsonObject so;
                        so.insert(QStringLiteral("pos"), s.lage);
                        so.insert(QStringLiteral("width"), static_cast<double>(s.weiteMs));
                        so.insert(QStringLiteral("flipped"), s.gekippt);
                        so.insert(QStringLiteral("shape"), formAlsJson(s.form));
                        stuecke.append(so);
                    }
                    o.insert(QStringLiteral("shapes"), stuecke);
                }
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
             QStringList* bericht, const SongLaenge& laengeVon)
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
            {
                Clip c;
                c.datei = datei;
                c.startMs = static_cast<std::int64_t>(o.value(QStringLiteral("start")).toDouble());
                c.endeMs = static_cast<std::int64_t>(o.value(QStringLiteral("end")).toDouble());
                c.mischart = static_cast<Mischart>(
                    std::clamp(o.value(QStringLiteral("blend")).toInt(), 0, kMischarten - 1));
                if (kanal.art == KanalArt::Audio)
                {
                    c.einsatzMs = static_cast<std::int64_t>(o.value(QStringLiteral("offset")).toDouble());
                    c.quellMs = static_cast<std::int64_t>(o.value(QStringLiteral("sourceLength")).toDouble());
                    if (c.quellMs <= 0 && laengeVon) c.quellMs = laengeVon(datei);
                    // Bis S95 hatte ein Ton-Clip kein Ende: er reicht bis zum
                    // Schluss seines Songs (ohne bekannte Laenge gibt `richte`
                    // ihm eine Sekunde und meldet es).
                    if (!o.contains(QStringLiteral("end")))
                        c.endeMs = std::max<std::int64_t>(0, c.startMs) +
                                   std::max<std::int64_t>(0, c.quellMs - std::max<std::int64_t>(0, c.einsatzMs));
                }
                // Bis S95: die Pegelkurve als Punkte in Millisekunden (`levels`).
                // Sie wird in Blenden oder ein Stueck uebersetzt; geschrieben
                // wird nur noch die neue Fassung.
                const QJsonArray punkte = o.value(QStringLiteral("levels")).toArray();
                if (!punkte.isEmpty())
                {
                    std::vector<AltPunkt> alt;
                    for (const QJsonValue& pv : punkte)
                    {
                        const QJsonObject po = pv.toObject();
                        AltPunkt p;
                        p.zeitMs = static_cast<std::int64_t>(po.value(QStringLiteral("t")).toDouble());
                        p.pegel = po.value(QStringLiteral("level")).toDouble(1.0);
                        p.form = static_cast<Kurvenform>(
                            std::clamp(po.value(QStringLiteral("curve")).toInt(), 0, kKurvenformen - 1));
                        const QJsonArray griffe = po.value(QStringLiteral("bezier")).toArray();
                        if (griffe.size() == 4)
                            for (int i = 0; i < 4; ++i)
                                p.bezier[static_cast<std::size_t>(i)] =
                                    std::clamp(griffe.at(i).toDouble(), 0.0, 1.0);
                        alt.push_back(p);
                    }
                    // die Laenge, die `richte` dem Clip gleich gibt
                    VisualClip mass = c;
                    mass.startMs = std::max<std::int64_t>(0, mass.startMs);
                    if (mass.endeMs <= mass.startMs) mass.endeMs = mass.startMs + 1000;
                    ausAltenPunkten(mass, std::move(alt));
                    c.ein = mass.ein;
                    c.aus = mass.aus;
                    c.stuecke = mass.stuecke;
                }
                c.grundpegel = o.value(QStringLiteral("base")).toDouble(1.0);
                const auto blende = [&o](const QString& feld, ClipBlende& b) {
                    if (!o.value(feld).isObject()) return;
                    const QJsonObject bo = o.value(feld).toObject();
                    b.dauerMs = static_cast<std::int64_t>(bo.value(QStringLiteral("ms")).toDouble());
                    if (!formAusJson(bo.value(QStringLiteral("shape")).toObject(), b.form))
                        b.form = einfacheBlende();
                };
                blende(QStringLiteral("fadeIn"), c.ein);
                blende(QStringLiteral("fadeOut"), c.aus);
                const QJsonArray stuecke = o.value(QStringLiteral("shapes")).toArray();
                for (const QJsonValue& sv : stuecke)
                {
                    const QJsonObject so = sv.toObject();
                    ClipStueck s;
                    s.lage = so.value(QStringLiteral("pos")).toDouble(0.5);
                    s.weiteMs = static_cast<std::int64_t>(so.value(QStringLiteral("width")).toDouble());
                    s.gekippt = so.value(QStringLiteral("flipped")).toBool(false);
                    if (!formAusJson(so.value(QStringLiteral("shape")).toObject(), s.form))
                        s.form = delle();
                    c.stuecke.push_back(std::move(s));
                }
                kanal.clips.push_back(std::move(c));
            }
        }
        neu.kanaele.push_back(std::move(kanal));
    }
    neu.richte(bericht);
    neu.ergaenzeTon(laengeVon);
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

bool ladeKomposition(const QString& pfad, Komposition& komposition, QStringList* bericht,
                     const SongLaenge& laengeVon)
{
    QFile datei(pfad);
    if (!datei.open(QIODevice::ReadOnly)) return false;
    QJsonParseError fehler;
    const QJsonDocument doc = QJsonDocument::fromJson(datei.readAll(), &fehler);
    if (fehler.error != QJsonParseError::NoError || !doc.isObject()) return false;
    return ausJson(doc.object(), komposition, QFileInfo(pfad).absolutePath(), bericht, laengeVon);
}

}  // namespace lumi::composer
