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
    const std::int64_t alt = clip.dauerMs();
    const std::int64_t neu = std::max<std::int64_t>(0, endeMs - startMs);
    for (PegelPunkt& p : clip.kurve)
    {
        // naeher am Ende: den Abstand zum Ende behalten
        if (p.zeitMs * 2 > alt) p.zeitMs = neu - (alt - p.zeitMs);
        p.zeitMs = std::clamp<std::int64_t>(p.zeitMs, 0, neu);
    }
    std::stable_sort(clip.kurve.begin(), clip.kurve.end(),
                     [](const PegelPunkt& a, const PegelPunkt& b) { return a.zeitMs < b.zeitMs; });
    clip.startMs = startMs;
    clip.endeMs = startMs + neu;
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

    const std::int64_t dauer = ich.dauerMs();
    std::int64_t start = std::max<std::int64_t>(0, neuerStartMs);
    // Fuer das Schieben: wie viele Clips des Zielkanals links liegen. Im
    // eigenen Kanal sind es die bisherigen Vorgaenger; in einem fremden die,
    // deren Mitte vor der Mitte des Clips liegt.
    int links = clip;
    if (nachKanal != vonKanal)
    {
        links = 0;
        for (const VisualClip& o : *clips)
            if (o.startMs + o.endeMs < 2 * start + dauer) ++links;
    }
    switch (stoss)
    {
        case Stoss::Halten:
            start = naechsterPlatz(*clips, start, dauer);
            break;
        case Stoss::Kuerzen:
            kuerze(*clips, start, start + dauer);
            break;
        case Stoss::Schieben:
            start += schiebeLinks(*clips, links, start);
            schiebeRechts(*clips, links, start + dauer);
            break;
    }
    ich.startMs = start;
    ich.endeMs = start + dauer;
    const int platz = setzeEin(*clips, std::move(ich));
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

bool gleich(const Komposition& a, const Komposition& b)
{
    return alsJson(a) == alsJson(b);
}

}  // namespace lumi::composer
