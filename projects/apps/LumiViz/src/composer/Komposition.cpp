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
#include <cmath>

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

std::int64_t Komposition::ueberlappung() const
{
    return std::clamp<std::int64_t>(ueberlappungMs, 0, laenge() / 2);
}

std::int64_t Komposition::umlaufMs() const
{
    return laenge() - ueberlappung();
}

std::int64_t Komposition::endStelleZu(std::int64_t stelleMs) const
{
    const std::int64_t fenster = ueberlappung();
    if (fenster <= 0 || stelleMs < 0 || stelleMs >= fenster) return -1;
    return stelleMs + umlaufMs();
}

double Komposition::blendeBei(std::int64_t stelleMs) const
{
    return masterBei(MasterArt::Gesamt, stelleMs);
}

double Komposition::masterBei(MasterArt art, std::int64_t stelleMs) const
{
    if (!hatKurve(art) || laenge() <= 0) return 1.0;
    return std::clamp(kurveAlsClip(nullptr, art).pegelBei(stelleMs), 0.0, 1.0);
}

double Komposition::bildPegelBei(std::int64_t stelleMs) const
{
    return masterBei(MasterArt::Gesamt, stelleMs) * masterBei(MasterArt::Bild, stelleMs);
}

double Komposition::tonPegelBei(std::int64_t stelleMs) const
{
    return masterBei(MasterArt::Gesamt, stelleMs) * masterBei(MasterArt::Ton, stelleMs);
}

MasterKurve Komposition::masterKurve(MasterArt art) const
{
    switch (art)
    {
        case MasterArt::Bild: return bildKurve;
        case MasterArt::Ton: return tonKurve;
        case MasterArt::Gesamt: break;
    }
    return {einblendeMs, ausblendeMs, einblendeForm, ausblendeForm, grundpegel, gesamtStuecke};
}

void Komposition::setzeMasterKurve(MasterArt art, MasterKurve kurve)
{
    switch (art)
    {
        case MasterArt::Bild: bildKurve = std::move(kurve); return;
        case MasterArt::Ton: tonKurve = std::move(kurve); return;
        case MasterArt::Gesamt: break;
    }
    einblendeMs = kurve.einblendeMs;
    ausblendeMs = kurve.ausblendeMs;
    einblendeForm = std::move(kurve.einblendeForm);
    ausblendeForm = std::move(kurve.ausblendeForm);
    grundpegel = kurve.grundpegel;
    gesamtStuecke = std::move(kurve.stuecke);
}

bool Komposition::hatKurve(MasterArt art) const
{
    switch (art)
    {
        case MasterArt::Bild: return bildKurve.wirkt();
        case MasterArt::Ton: return tonKurve.wirkt();
        case MasterArt::Gesamt: break;
    }
    return einblendeMs > 0 || ausblendeMs > 0 || grundpegel < 1.0 || !gesamtStuecke.empty();
}

Clip Komposition::kurveAlsClip(std::vector<int>* herkunft, MasterArt art) const
{
    const MasterKurve kurve = masterKurve(art);
    Clip c;
    c.startMs = 0;
    c.endeMs = laenge();
    c.grundpegel = kurve.grundpegel;
    c.ein = {kurve.einblendeMs, kurve.einblendeForm};
    c.aus = {kurve.ausblendeMs, kurve.ausblendeForm};
    if (herkunft != nullptr) herkunft->clear();
    for (std::size_t i = 0; i < kurve.stuecke.size(); ++i)
    {
        const GesamtStueck& s = kurve.stuecke[i];
        // hinter dem Ende wirkt ein Stueck nicht (Entscheid Patrik S100, F7-1 a)
        if (c.endeMs <= 0 || s.mitteMs >= c.endeMs) continue;
        ClipStueck t;
        t.lage = static_cast<double>(s.mitteMs) / static_cast<double>(c.endeMs);
        t.weiteMs = s.weiteMs;
        t.gekippt = s.gekippt;
        t.form = s.form;
        c.stuecke.push_back(std::move(t));
        if (herkunft != nullptr) herkunft->push_back(static_cast<int>(i));
    }
    return c;
}

void Komposition::setzeKurveAusClip(const Clip& clip, MasterArt art)
{
    const std::int64_t ende = laenge();
    MasterKurve kurve = masterKurve(art);
    kurve.grundpegel = std::clamp(clip.grundpegel, 0.0, 1.0);
    kurve.einblendeMs = std::max<std::int64_t>(0, clip.ein.dauerMs);
    kurve.einblendeForm = clip.ein.form;
    kurve.ausblendeMs = std::max<std::int64_t>(0, clip.aus.dauerMs);
    kurve.ausblendeForm = clip.aus.form;
    // was hinter dem Ende liegt, trug der Clip nicht — es bleibt
    std::vector<GesamtStueck> neu;
    for (const GesamtStueck& s : kurve.stuecke)
        if (ende <= 0 || s.mitteMs >= ende) neu.push_back(s);
    if (ende > 0)
    {
        for (const ClipStueck& t : clip.stuecke)
        {
            GesamtStueck s;
            // die Mitte bleibt vor dem Ende — sonst waere das Stueck gleich wieder weg
            s.mitteMs = std::clamp<std::int64_t>(
                std::llround(std::clamp(t.lage, 0.0, 1.0) * static_cast<double>(ende)), 0, ende - 1);
            s.weiteMs = std::max<std::int64_t>(0, t.weiteMs);
            s.gekippt = t.gekippt;
            s.form = t.form;
            neu.push_back(std::move(s));
        }
    }
    kurve.stuecke = std::move(neu);
    setzeMasterKurve(art, std::move(kurve));
}

std::int32_t reihenFarbe(KanalArt art, int nr)
{
    // Bild: kuehl — Blau (die Farbe, die CH1 schon immer hatte), Violett,
    // Gruen, Tuerkis, Magenta, Indigo. Ton: warm — Orange (der Ton aller
    // Audio-Clips bis S99), Rot, Gelb, Rosa, Rotorange, Ocker.
    static constexpr std::array<std::int32_t, kReihenFarben> bild = {0x5785A5, 0x8157A5, 0x57A57E,
                                                                    0x57A5A5, 0xA557A5, 0x5757A5};
    static constexpr std::array<std::int32_t, kReihenFarben> ton = {0x967855, 0x965555, 0x968D55,
                                                                   0x96556B, 0x966755, 0x968555};
    const auto platz = static_cast<std::size_t>(((nr % kReihenFarben) + kReihenFarben) % kReihenFarben);
    return art == KanalArt::Audio ? ton[platz] : bild[platz];
}

std::int32_t Komposition::freieFarbe(KanalArt art) const
{
    int gleicheArt = 0;
    for (const Kanal& k : kanaele)
        if (k.art == art) ++gleicheArt;
    for (int nr = 0; nr < kReihenFarben; ++nr)
    {
        const std::int32_t farbe = reihenFarbe(art, nr);
        const bool vergeben = std::any_of(kanaele.begin(), kanaele.end(), [&](const Kanal& k) {
            return k.art == art && k.farbe == farbe;
        });
        if (!vergeben) return farbe;
    }
    return reihenFarbe(art, gleicheArt);  // mehr Kanaele als Farben: die Reihe laeuft um
}

void Komposition::vergibFarben()
{
    for (Kanal& k : kanaele)
        if (k.farbe < 0) k.farbe = freieFarbe(k.art);
}

std::int32_t Komposition::farbeVon(int kanal) const
{
    if (kanal < 0 || kanal >= static_cast<int>(kanaele.size())) return reihenFarbe(KanalArt::Visual, 0);
    const Kanal& k = kanaele[static_cast<std::size_t>(kanal)];
    if (k.farbe >= 0) return k.farbe;
    int platz = 0;
    for (int i = 0; i < kanal; ++i)
        if (kanaele[static_cast<std::size_t>(i)].art == k.art) ++platz;
    return reihenFarbe(k.art, platz);
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
    // Eingestellte Werte bleiben, auch ueber die Laenge hinaus — geklemmt wird
    // beim Rechnen (`ueberlappung`, `blendeBei`); nur Unsinn geht weg.
    ueberlappungMs = std::max<std::int64_t>(0, ueberlappungMs);
    einblendeMs = std::max<std::int64_t>(0, einblendeMs);
    ausblendeMs = std::max<std::int64_t>(0, ausblendeMs);
    grundpegel = std::clamp(grundpegel, 0.0, 1.0);
    for (Pegelform* f : {&einblendeForm, &ausblendeForm})
    {
        f->art = FormArt::Blende;
        f->richte();
    }
    for (GesamtStueck& s : gesamtStuecke)
    {
        s.mitteMs = std::max<std::int64_t>(0, s.mitteMs);
        s.weiteMs = std::max<std::int64_t>(0, s.weiteMs);
        s.form.art = FormArt::Stueck;
        s.form.richte();
    }
    // Visual-Master und Audio-Master (S101): dieselben Regeln
    for (MasterKurve* kurve : {&bildKurve, &tonKurve})
    {
        kurve->einblendeMs = std::max<std::int64_t>(0, kurve->einblendeMs);
        kurve->ausblendeMs = std::max<std::int64_t>(0, kurve->ausblendeMs);
        kurve->grundpegel = std::clamp(kurve->grundpegel, 0.0, 1.0);
        for (Pegelform* f : {&kurve->einblendeForm, &kurve->ausblendeForm})
        {
            f->art = FormArt::Blende;
            f->richte();
        }
        for (GesamtStueck& s : kurve->stuecke)
        {
            s.mitteMs = std::max<std::int64_t>(0, s.mitteMs);
            s.weiteMs = std::max<std::int64_t>(0, s.weiteMs);
            s.form.art = FormArt::Stueck;
            s.form.richte();
        }
    }
}

int Komposition::tonKanal() const
{
    for (int k = 0; k < static_cast<int>(kanaele.size()); ++k)
        if (kanaele[static_cast<std::size_t>(k)].art == KanalArt::Audio && kanaele[static_cast<std::size_t>(k)].an)
            return k;
    return -1;
}

std::vector<const Clip*> Komposition::toeneBei(std::int64_t stelleMs, std::int64_t vorlaufMs) const
{
    // Alle eingeschalteten Audio-Kanaele zaehlen (Befund Patrik S96: ein leerer
    // Kanal oben machte die darunter stumm) — und alle Clips an der Stelle
    // spielen zugleich (C5b, Entscheid Patrik S97).
    std::vector<const Clip*> toene;
    for (const Kanal& kanal : kanaele)
    {
        if (kanal.art != KanalArt::Audio || !kanal.an) continue;
        for (const Clip& c : kanal.clips)
            if (stelleMs >= c.startMs - vorlaufMs && stelleMs < c.endeMs) toene.push_back(&c);
    }
    return toene;
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
        // die Farbe des Kanals (S100), wenn er eine hat: "#rrggbb"
        if (kanal.farbe >= 0)
            k.insert(QStringLiteral("color"),
                     QStringLiteral("#%1").arg(kanal.farbe & 0xFFFFFF, 6, 16, QLatin1Char('0')));
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
    // Schleife und Blende (S99): nur, was von der Vorgabe abweicht — eine
    // Komposition ohne sie sieht in der Datei aus wie bisher
    if (komposition.ueberlappungMs > 0 || !komposition.songsDoppelt || komposition.bilderDoppelt)
    {
        QJsonObject schleife;
        schleife.insert(QStringLiteral("overlap"), static_cast<double>(komposition.ueberlappungMs));
        schleife.insert(QStringLiteral("songsTwice"), komposition.songsDoppelt);
        schleife.insert(QStringLiteral("visualsTwice"), komposition.bilderDoppelt);
        wurzel.insert(QStringLiteral("loop"), schleife);
    }
    if (komposition.einblendeMs > 0 || komposition.ausblendeMs > 0)
    {
        QJsonObject blende;
        blende.insert(QStringLiteral("in"), static_cast<double>(komposition.einblendeMs));
        blende.insert(QStringLiteral("out"), static_cast<double>(komposition.ausblendeMs));
        // die Formen (S100) nur, wenn sie von der linearen Vorgabe abweichen
        if (komposition.einblendeMs > 0 && !gleich(komposition.einblendeForm, einfacheBlende()))
            blende.insert(QStringLiteral("inShape"), formAlsJson(komposition.einblendeForm));
        if (komposition.ausblendeMs > 0 && !gleich(komposition.ausblendeForm, einfacheBlende()))
            blende.insert(QStringLiteral("outShape"), formAlsJson(komposition.ausblendeForm));
        wurzel.insert(QStringLiteral("fade"), blende);
    }
    // Grundpegel und Stuecke der ganzen Komposition (S100): die Lage als feste
    // Zeit (`at`), nicht als Anteil wie am Clip (`pos`)
    if (komposition.grundpegel < 1.0 || !komposition.gesamtStuecke.empty())
    {
        QJsonObject pegel;
        if (komposition.grundpegel < 1.0) pegel.insert(QStringLiteral("base"), komposition.grundpegel);
        QJsonArray stuecke;
        for (const GesamtStueck& s : komposition.gesamtStuecke)
        {
            QJsonObject so;
            so.insert(QStringLiteral("at"), static_cast<double>(s.mitteMs));
            so.insert(QStringLiteral("width"), static_cast<double>(s.weiteMs));
            so.insert(QStringLiteral("flipped"), s.gekippt);
            so.insert(QStringLiteral("shape"), formAlsJson(s.form));
            stuecke.append(so);
        }
        if (!stuecke.isEmpty()) pegel.insert(QStringLiteral("shapes"), stuecke);
        wurzel.insert(QStringLiteral("level"), pegel);
    }
    // Visual-Master und Audio-Master (S101): je ein Objekt, nur wenn die Kurve
    // wirkt — eine Komposition ohne sie sieht in der Datei aus wie bisher
    const auto master = [&wurzel](const QString& schluessel, const MasterKurve& kurve) {
        if (!kurve.wirkt()) return;
        QJsonObject o;
        if (kurve.einblendeMs > 0)
        {
            o.insert(QStringLiteral("in"), static_cast<double>(kurve.einblendeMs));
            if (!gleich(kurve.einblendeForm, einfacheBlende()))
                o.insert(QStringLiteral("inShape"), formAlsJson(kurve.einblendeForm));
        }
        if (kurve.ausblendeMs > 0)
        {
            o.insert(QStringLiteral("out"), static_cast<double>(kurve.ausblendeMs));
            if (!gleich(kurve.ausblendeForm, einfacheBlende()))
                o.insert(QStringLiteral("outShape"), formAlsJson(kurve.ausblendeForm));
        }
        if (kurve.grundpegel < 1.0) o.insert(QStringLiteral("base"), kurve.grundpegel);
        QJsonArray stuecke;
        for (const GesamtStueck& s : kurve.stuecke)
        {
            QJsonObject so;
            so.insert(QStringLiteral("at"), static_cast<double>(s.mitteMs));
            so.insert(QStringLiteral("width"), static_cast<double>(s.weiteMs));
            so.insert(QStringLiteral("flipped"), s.gekippt);
            so.insert(QStringLiteral("shape"), formAlsJson(s.form));
            stuecke.append(so);
        }
        if (!stuecke.isEmpty()) o.insert(QStringLiteral("shapes"), stuecke);
        wurzel.insert(schluessel, o);
    };
    master(QStringLiteral("visualMaster"), komposition.bildKurve);
    master(QStringLiteral("audioMaster"), komposition.tonKurve);
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
    const QJsonObject schleife = wurzel.value(QStringLiteral("loop")).toObject();
    neu.ueberlappungMs = static_cast<std::int64_t>(schleife.value(QStringLiteral("overlap")).toDouble());
    neu.songsDoppelt = schleife.value(QStringLiteral("songsTwice")).toBool(true);
    neu.bilderDoppelt = schleife.value(QStringLiteral("visualsTwice")).toBool(false);
    const QJsonObject blende = wurzel.value(QStringLiteral("fade")).toObject();
    neu.einblendeMs = static_cast<std::int64_t>(blende.value(QStringLiteral("in")).toDouble());
    neu.ausblendeMs = static_cast<std::int64_t>(blende.value(QStringLiteral("out")).toDouble());
    // ohne Form (Dateien bis S99) bleibt die lineare Vorgabe
    if (!formAusJson(blende.value(QStringLiteral("inShape")).toObject(), neu.einblendeForm))
        neu.einblendeForm = einfacheBlende();
    if (!formAusJson(blende.value(QStringLiteral("outShape")).toObject(), neu.ausblendeForm))
        neu.ausblendeForm = einfacheBlende();
    const QJsonObject pegel = wurzel.value(QStringLiteral("level")).toObject();
    neu.grundpegel = pegel.value(QStringLiteral("base")).toDouble(1.0);
    for (const QJsonValue& sv : pegel.value(QStringLiteral("shapes")).toArray())
    {
        const QJsonObject so = sv.toObject();
        GesamtStueck s;
        s.mitteMs = static_cast<std::int64_t>(so.value(QStringLiteral("at")).toDouble());
        s.weiteMs = static_cast<std::int64_t>(so.value(QStringLiteral("width")).toDouble());
        s.gekippt = so.value(QStringLiteral("flipped")).toBool(false);
        if (!formAusJson(so.value(QStringLiteral("shape")).toObject(), s.form)) s.form = delle();
        neu.gesamtStuecke.push_back(std::move(s));
    }
    // Visual-Master und Audio-Master (S101); ohne Objekt (Dateien bis S100)
    // bleibt die Vorgabe: wirkt nicht
    const auto master = [&wurzel](const QString& schluessel, MasterKurve& kurve) {
        const QJsonObject o = wurzel.value(schluessel).toObject();
        kurve.einblendeMs = static_cast<std::int64_t>(o.value(QStringLiteral("in")).toDouble());
        kurve.ausblendeMs = static_cast<std::int64_t>(o.value(QStringLiteral("out")).toDouble());
        if (!formAusJson(o.value(QStringLiteral("inShape")).toObject(), kurve.einblendeForm))
            kurve.einblendeForm = einfacheBlende();
        if (!formAusJson(o.value(QStringLiteral("outShape")).toObject(), kurve.ausblendeForm))
            kurve.ausblendeForm = einfacheBlende();
        kurve.grundpegel = o.value(QStringLiteral("base")).toDouble(1.0);
        for (const QJsonValue& sv : o.value(QStringLiteral("shapes")).toArray())
        {
            const QJsonObject so = sv.toObject();
            GesamtStueck s;
            s.mitteMs = static_cast<std::int64_t>(so.value(QStringLiteral("at")).toDouble());
            s.weiteMs = static_cast<std::int64_t>(so.value(QStringLiteral("width")).toDouble());
            s.gekippt = so.value(QStringLiteral("flipped")).toBool(false);
            if (!formAusJson(so.value(QStringLiteral("shape")).toObject(), s.form)) s.form = delle();
            kurve.stuecke.push_back(std::move(s));
        }
    };
    master(QStringLiteral("visualMaster"), neu.bildKurve);
    master(QStringLiteral("audioMaster"), neu.tonKurve);
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
        // ohne lesbare Farbe (Dateien bis S99) bleibt -1: wer laedt, vergibt sie
        if (const QString farbe = k.value(QStringLiteral("color")).toString();
            farbe.size() == 7 && farbe.startsWith(QLatin1Char('#')))
        {
            bool ok = false;
            const int wert = farbe.mid(1).toInt(&ok, 16);
            if (ok) kanal.farbe = wert;
        }
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
