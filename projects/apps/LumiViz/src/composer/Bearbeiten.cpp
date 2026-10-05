/**
 * @file Bearbeiten.cpp
 * @brief Composer: Clips verschieben, an den Kanten ziehen, loeschen (Stufe C3a)
 */

#include "composer/Bearbeiten.hpp"

#include <algorithm>
#include <cstdlib>
#include <limits>
#include <vector>

namespace lumi::composer
{

namespace
{
using Clips = std::vector<VisualClip>;

/// Der Kanal und sein Clip — nullptr, wenn es einen davon nicht gibt.
Clips* clipsVon(Komposition& komposition, int kanal, int clip)
{
    if (kanal < 0 || kanal >= static_cast<int>(komposition.kanaele.size())) return nullptr;
    Kanal& k = komposition.kanaele[static_cast<std::size_t>(kanal)];
    if (k.art != KanalArt::Visual) return nullptr;
    if (clip < 0 || clip >= static_cast<int>(k.visual.size())) return nullptr;
    return &k.visual;
}

void verschiebeUm(VisualClip& clip, std::int64_t delta)
{
    clip.startMs += delta;
    clip.endeMs += delta;
}

/// Anhalten: der Start, der `wunsch` am naechsten liegt und an dem ein Clip
/// der Laenge `dauer` zwischen die anderen passt. Hinter dem letzten ist immer Platz.
std::int64_t naechsterPlatz(const Clips& andere, std::int64_t wunsch, std::int64_t dauer)
{
    std::int64_t bester = -1;
    std::int64_t abstand = std::numeric_limits<std::int64_t>::max();
    const auto pruefe = [&](std::int64_t von, std::int64_t bis) {
        if (bis - von < dauer) return;
        const std::int64_t platz = std::clamp(wunsch, von, bis - dauer);
        const std::int64_t a = std::llabs(platz - wunsch);
        if (a < abstand)
        {
            abstand = a;
            bester = platz;
        }
    };
    std::int64_t frei = 0;
    for (const VisualClip& o : andere)
    {
        pruefe(frei, o.startMs);
        frei = std::max(frei, o.endeMs);
    }
    const std::int64_t hinten = std::max(wunsch, frei);
    if (bester < 0 || std::llabs(hinten - wunsch) < abstand) bester = hinten;
    return bester;
}

/// Kuerzen: alles, was in [von, bis) liegt, weicht. Von einem Clip, der den
/// Bereich ganz umschliesst, bleibt der vordere Teil.
void kuerze(Clips& andere, std::int64_t von, std::int64_t bis)
{
    for (VisualClip& o : andere)
    {
        if (o.endeMs <= von || o.startMs >= bis) continue;
        if (o.startMs < von)
            setzeGrenzen(o, o.startMs, von);
        else if (o.endeMs > bis)
            setzeGrenzen(o, bis, o.endeMs);
        else
            o.endeMs = o.startMs;  // ganz ueberdeckt
    }
    andere.erase(std::remove_if(andere.begin(), andere.end(),
                                [](const VisualClip& o) { return o.dauerMs() < kMinClipMs; }),
                 andere.end());
}

/// Schieben nach links: die ersten `n` Clips liegen links der Kante `grenze`
/// und weichen ihr aus. Liefert, um wie viel die Kante nach rechts ruecken
/// muss, weil am Anfang der Zeitachse kein Platz mehr war.
std::int64_t schiebeLinks(Clips& andere, int n, std::int64_t grenze)
{
    int geschoben = 0;
    for (int i = n - 1; i >= 0; --i)
    {
        VisualClip& o = andere[static_cast<std::size_t>(i)];
        if (o.endeMs <= grenze) break;
        verschiebeUm(o, grenze - o.endeMs);
        grenze = o.startMs;
        ++geschoben;
    }
    if (geschoben == 0) return 0;
    const std::int64_t vorn = andere[static_cast<std::size_t>(n - geschoben)].startMs;
    if (vorn >= 0) return 0;
    for (int i = n - geschoben; i < n; ++i) verschiebeUm(andere[static_cast<std::size_t>(i)], -vorn);
    return -vorn;
}

/// Schieben nach rechts: die Clips ab Platz `n` weichen der Kante `grenze` aus.
void schiebeRechts(Clips& andere, int n, std::int64_t grenze)
{
    for (int i = n; i < static_cast<int>(andere.size()); ++i)
    {
        VisualClip& o = andere[static_cast<std::size_t>(i)];
        if (o.startMs >= grenze) break;
        verschiebeUm(o, grenze - o.startMs);
        grenze = o.endeMs;
    }
}

/// Den bearbeiteten Clip wieder einsortieren; liefert seinen Platz.
int setzeEin(Clips& andere, VisualClip ich)
{
    const auto ort = std::find_if(andere.begin(), andere.end(), [&ich](const VisualClip& o) {
        return o.startMs >= ich.startMs;
    });
    const int platz = static_cast<int>(ort - andere.begin());
    andere.insert(ort, std::move(ich));
    return platz;
}

/// Den Clip `ich` (nicht in `andere`) an `start` legen, nach der Regel
/// `stoss`. `links` = wie viele der anderen fuer das Schieben links von ihm
/// liegen; -1 = aus der Lage bestimmen (die, deren Mitte vor seiner liegt).
int platziere(Clips& andere, VisualClip ich, std::int64_t start, Stoss stoss, int links)
{
    const std::int64_t dauer = ich.dauerMs();
    if (links < 0)
    {
        links = 0;
        for (const VisualClip& o : andere)
            if (o.startMs + o.endeMs < 2 * start + dauer) ++links;
    }
    switch (stoss)
    {
        case Stoss::Halten:
            start = naechsterPlatz(andere, start, dauer);
            break;
        case Stoss::Kuerzen:
            kuerze(andere, start, start + dauer);
            break;
        case Stoss::Schieben:
            start += schiebeLinks(andere, links, start);
            schiebeRechts(andere, links, start + dauer);
            break;
    }
    ich.startMs = start;
    ich.endeMs = start + dauer;
    return setzeEin(andere, std::move(ich));
}

/// Eine gesetzte Laenge waechst mit, wenn ein Clip ueber sie hinausragt.
void passeLaengeAn(Komposition& komposition)
{
    if (komposition.laengeMs <= 0) return;
    for (const Kanal& k : komposition.kanaele)
        for (const VisualClip& c : k.visual)
            komposition.laengeMs = std::max(komposition.laengeMs, c.endeMs);
}
}  // namespace

void setzeGrenzen(VisualClip& clip, std::int64_t startMs, std::int64_t endeMs)
{
    // Die Blenden haengen an ihren Kanten, die Stuecke an ihrem Anteil — und
    // alle behalten ihre eingestellte Weite (Entscheid Patrik S96). Was nicht
    // mehr Platz hat, staucht `VisualClip::verteilung` beim Rechnen.
    clip.startMs = startMs;
    clip.endeMs = startMs + std::max<std::int64_t>(0, endeMs - startMs);
}

int verschiebeClip(Komposition& komposition, int kanal, int clip, std::int64_t neuerStartMs,
                   Stoss stoss)
{
    return versetzeClip(komposition, kanal, clip, kanal, neuerStartMs, stoss);
}

int versetzeClip(Komposition& komposition, int vonKanal, int clip, int nachKanal,
                 std::int64_t neuerStartMs, Stoss stoss)
{
    Clips* quelle = clipsVon(komposition, vonKanal, clip);
    if (quelle == nullptr) return -1;
    if (nachKanal < 0 || nachKanal >= static_cast<int>(komposition.kanaele.size()) ||
        komposition.kanaele[static_cast<std::size_t>(nachKanal)].art != KanalArt::Visual)
        return -1;
    VisualClip ich = (*quelle)[static_cast<std::size_t>(clip)];
    quelle->erase(quelle->begin() + clip);
    Clips* clips = &komposition.kanaele[static_cast<std::size_t>(nachKanal)].visual;

    // Im eigenen Kanal liegen links die bisherigen Vorgaenger.
    const int platz = platziere(*clips, std::move(ich), std::max<std::int64_t>(0, neuerStartMs),
                                stoss, nachKanal == vonKanal ? clip : -1);
    passeLaengeAn(komposition);
    return platz;
}

int zieheKante(Komposition& komposition, int kanal, int clip, bool links, std::int64_t zeitMs,
               Stoss stoss)
{
    Clips* clips = clipsVon(komposition, kanal, clip);
    if (clips == nullptr) return -1;
    VisualClip ich = (*clips)[static_cast<std::size_t>(clip)];
    clips->erase(clips->begin() + clip);

    if (links)
    {
        std::int64_t t = std::clamp<std::int64_t>(zeitMs, 0, ich.endeMs - kMinClipMs);
        switch (stoss)
        {
            case Stoss::Halten:
                if (clip > 0) t = std::max(t, (*clips)[static_cast<std::size_t>(clip - 1)].endeMs);
                break;
            case Stoss::Kuerzen:
                kuerze(*clips, t, ich.endeMs);
                break;
            case Stoss::Schieben:
                t += schiebeLinks(*clips, clip, t);
                t = std::min(t, ich.endeMs - kMinClipMs);
                break;
        }
        setzeGrenzen(ich, t, ich.endeMs);
    }
    else
    {
        std::int64_t t = std::max(zeitMs, ich.startMs + kMinClipMs);
        switch (stoss)
        {
            case Stoss::Halten:
                if (clip < static_cast<int>(clips->size()))
                    t = std::min(t, (*clips)[static_cast<std::size_t>(clip)].startMs);
                break;
            case Stoss::Kuerzen:
                kuerze(*clips, ich.startMs, t);
                break;
            case Stoss::Schieben:
                schiebeRechts(*clips, clip, t);
                break;
        }
        setzeGrenzen(ich, ich.startMs, t);
    }
    const int platz = setzeEin(*clips, std::move(ich));
    passeLaengeAn(komposition);
    return platz;
}

bool loescheClip(Komposition& komposition, int kanal, int clip)
{
    Clips* clips = clipsVon(komposition, kanal, clip);
    if (clips == nullptr) return false;
    clips->erase(clips->begin() + clip);
    return true;
}

int fuegeClipEin(Komposition& komposition, int kanal, const QString& datei, std::int64_t startMs,
                 std::int64_t dauerMs, Stoss stoss)
{
    if (kanal < 0 || kanal >= static_cast<int>(komposition.kanaele.size())) return -1;
    Kanal& k = komposition.kanaele[static_cast<std::size_t>(kanal)];
    if (k.art != KanalArt::Visual) return -1;

    std::int64_t start = std::max<std::int64_t>(0, startMs);
    std::int64_t dauer = std::max(dauerMs, kMinClipMs);
    if (stoss == Stoss::Halten)
    {
        // Faellt der Clip in freie Flaeche, reicht er hoechstens bis zum naechsten Clip.
        bool frei = true;
        std::int64_t naechster = std::numeric_limits<std::int64_t>::max();
        for (const VisualClip& o : k.visual)
        {
            if (o.startMs <= start && start < o.endeMs) frei = false;
            if (o.startMs > start) naechster = std::min(naechster, o.startMs);
        }
        if (frei && naechster - start >= kMinClipMs) dauer = std::min(dauer, naechster - start);
    }
    VisualClip neu;
    neu.datei = datei;
    neu.startMs = start;
    neu.endeMs = start + dauer;
    const int platz = platziere(k.visual, std::move(neu), start, stoss, -1);
    passeLaengeAn(komposition);
    return platz;
}

int fuegeKanalEin(Komposition& komposition, const QString& name)
{
    Kanal neu;
    neu.art = KanalArt::Visual;
    neu.name = name;
    if (neu.name.isEmpty())
    {
        for (int n = 1; neu.name.isEmpty(); ++n)
        {
            const QString kandidat = QStringLiteral("CH%1").arg(n);
            const bool vergeben =
                std::any_of(komposition.kanaele.begin(), komposition.kanaele.end(),
                            [&kandidat](const Kanal& k) { return k.name == kandidat; });
            if (!vergeben) neu.name = kandidat;
        }
    }
    komposition.kanaele.push_back(std::move(neu));
    return static_cast<int>(komposition.kanaele.size()) - 1;
}

bool loescheKanal(Komposition& komposition, int kanal)
{
    if (kanal < 0 || kanal >= static_cast<int>(komposition.kanaele.size())) return false;
    komposition.kanaele.erase(komposition.kanaele.begin() + kanal);
    return true;
}

Komposition neueKomposition()
{
    Komposition k;
    (void)fuegeKanalEin(k);
    return k;
}

bool liesBlenden(const VisualClip& clip, Blenden& blenden)
{
    blenden = Blenden{};
    // eine Form mit eigenen Punkten zeigt sich nicht als Kurvenform mit Anfassern
    if ((clip.ein.dauerMs > 0 && !einfach(clip.ein.form)) ||
        (clip.aus.dauerMs > 0 && !einfach(clip.aus.form)))
        return false;
    if (clip.ein.dauerMs > 0)
    {
        blenden.einMs = clip.ein.dauerMs;
        blenden.einForm = clip.ein.form.punkte.front().form;
        blenden.einBezier = clip.ein.form.punkte.front().bezier;
    }
    if (clip.aus.dauerMs > 0)
    {
        blenden.ausMs = clip.aus.dauerMs;
        blenden.ausForm = clip.aus.form.punkte.front().form;
        blenden.ausBezier = clip.aus.form.punkte.front().bezier;
    }
    return true;
}

void setzeBlenden(VisualClip& clip, const Blenden& blenden)
{
    clip.ein = {std::max<std::int64_t>(0, blenden.einMs),
                einfacheBlende(blenden.einForm, blenden.einBezier)};
    clip.aus = {std::max<std::int64_t>(0, blenden.ausMs),
                einfacheBlende(blenden.ausForm, blenden.ausBezier)};
}

void schreibeFest(VisualClip& clip)
{
    const Verteilung v = clip.verteilung();
    const std::int64_t dauer = clip.dauerMs();
    if (v.ein.gestaucht) clip.ein.dauerMs = v.ein.weiteMs();
    if (v.aus.gestaucht) clip.aus.dauerMs = v.aus.weiteMs();
    for (std::size_t i = 0; i < clip.stuecke.size(); ++i)
    {
        if (!v.stuecke[i].gestaucht) continue;
        clip.stuecke[i].weiteMs = v.stuecke[i].weiteMs();
        if (dauer > 0)
            clip.stuecke[i].lage = static_cast<double>(v.stuecke[i].vonMs + v.stuecke[i].bisMs) /
                                   (2.0 * static_cast<double>(dauer));
    }
}

bool hatTeil(const VisualClip& clip, Teil teil)
{
    switch (teil.art)
    {
        case TeilArt::Ein: return clip.ein.dauerMs > 0;
        case TeilArt::Aus: return clip.aus.dauerMs > 0;
        case TeilArt::Stueck:
            return teil.stueck >= 0 && teil.stueck < static_cast<int>(clip.stuecke.size());
        case TeilArt::Keins: break;
    }
    return false;
}

Bereich bereichVon(const VisualClip& clip, Teil teil)
{
    if (!hatTeil(clip, teil)) return {};
    const Verteilung v = clip.verteilung();
    if (teil.art == TeilArt::Ein) return v.ein;
    if (teil.art == TeilArt::Aus) return v.aus;
    return v.stuecke[static_cast<std::size_t>(teil.stueck)];
}

Teil teilBei(const VisualClip& clip, std::int64_t zeitImClipMs)
{
    // dieselbe Reihenfolge wie `VisualClip::pegelBei`
    const Verteilung v = clip.verteilung();
    if (v.ein.weiteMs() > 0 && zeitImClipMs < v.ein.bisMs) return {TeilArt::Ein, -1};
    if (v.aus.weiteMs() > 0 && zeitImClipMs >= v.aus.vonMs) return {TeilArt::Aus, -1};
    for (int i = 0; i < static_cast<int>(v.stuecke.size()); ++i)
    {
        const Bereich& b = v.stuecke[static_cast<std::size_t>(i)];
        if (b.weiteMs() > 0 && zeitImClipMs >= b.vonMs && zeitImClipMs < b.bisMs)
            return {TeilArt::Stueck, i};
    }
    return {};
}

int fuegeStueckEin(VisualClip& clip, std::int64_t mitteMs)
{
    const std::int64_t dauer = std::max<std::int64_t>(1, clip.dauerMs());
    ClipStueck neu;
    neu.lage = std::clamp(static_cast<double>(mitteMs) / static_cast<double>(dauer), 0.0, 1.0);
    neu.weiteMs = std::max<std::int64_t>(1, std::min(kNeuesStueckMs, dauer / 4));
    clip.stuecke.push_back(std::move(neu));
    return static_cast<int>(clip.stuecke.size()) - 1;
}

bool loescheStueck(VisualClip& clip, int stueck)
{
    if (stueck < 0 || stueck >= static_cast<int>(clip.stuecke.size())) return false;
    clip.stuecke.erase(clip.stuecke.begin() + stueck);
    return true;
}

const char* kurvenformName(Kurvenform form)
{
    switch (form)
    {
        case Kurvenform::Linear: return "Linear";
        case Kurvenform::SKurve: return "S-curve";
        case Kurvenform::EaseIn: return "Ease in";
        case Kurvenform::EaseOut: return "Ease out";
        case Kurvenform::Exponentiell: return "Exponential";
        case Kurvenform::Halten: return "Hold";
        case Kurvenform::Bezier: return "Bezier";
    }
    return "Linear";
}

bool gleich(const Komposition& a, const Komposition& b)
{
    return alsJson(a) == alsJson(b);
}

}  // namespace lumi::composer
