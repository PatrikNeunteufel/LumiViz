/**
 * @file Bounce.cpp
 * @brief Loop: *Bounce to wave clip* — den Loop aendern und pruefen (Stufe 15k-2, S170)
 */

#include "loop/Bounce.hpp"

#include "loop/Austausch.hpp"  // clipDatei

#include <algorithm>
#include <cmath>

namespace lumi::loop
{

namespace
{

/// „<vorsatz>“, sonst „<vorsatz> 2“, „<vorsatz> 3“ … — der erste Name, den der Loop nicht hat.
QString freierMusterName(const Loop& loop, const QString& vorsatz)
{
    if (loop.findeMuster(vorsatz) == nullptr) return vorsatz;
    for (int n = 2;; ++n)
    {
        const QString name = vorsatz + QLatin1Char(' ') + QString::number(n);
        if (loop.findeMuster(name) == nullptr) return name;
    }
}

/// Der Ursprungsclip der Herkunft (ab Takt 0) und sein Muster; nullptr = keine Herkunft.
const LoopClip* herkunftClip(const Muster& wave)
{
    if (wave.wave.herkunft.leer()) return nullptr;
    const Loop& h = wave.wave.herkunft.clip();
    if (h.kanaele.empty() || h.kanaele.front().clips.empty() || h.muster.empty()) return nullptr;
    return &h.kanaele.front().clips.front();
}

}  // namespace

bool bounceMoeglich(const Loop& loop, ClipOrt ort)
{
    const int clip = findeClip(loop, ort);
    if (clip < 0) return false;
    const LoopClip& c = loop.kanaele[static_cast<std::size_t>(ort.kanal)].clips[static_cast<std::size_t>(clip)];
    const Muster* m = loop.findeMuster(c.muster);
    return m != nullptr && m->art != MusterArt::Wave;
}

int bounceTakte(const Loop& loop, ClipOrt ort, const Wellenform& welle, bool* abgeschnitten)
{
    if (abgeschnitten != nullptr) *abgeschnitten = false;
    const int clip = findeClip(loop, ort);
    if (clip < 0) return 0;
    const Kanal& k = loop.kanaele[static_cast<std::size_t>(ort.kanal)];
    const LoopClip& c = k.clips[static_cast<std::size_t>(clip)];
    const int clipTakte = loop.clipTakte(c);
    const double taktSekunden = loop.bpm > 0.0 ? loop.schlaegeJeTakt * 60.0 / loop.bpm : 0.0;
    int takte = clipTakte;
    if (taktSekunden > 0.0 && welle.rate > 0)
        takte = std::max(clipTakte, static_cast<int>(std::ceil(static_cast<double>(welle.klang.frames()) / welle.rate / taktSekunden - 1e-9)));
    // hoechstens bis zum Anfang des naechsten Clips im Kanal
    for (const LoopClip& anderer : k.clips)
        if (anderer.startTakt > c.startTakt && anderer.startTakt - c.startTakt < takte)
        {
            takte = anderer.startTakt - c.startTakt;
            if (abgeschnitten != nullptr) *abgeschnitten = true;
        }
    return std::clamp(takte, kTakteMin, kWaveTakteMax);
}

QString setzeBounceEin(Loop& loop, ClipOrt ort, const QString& nennung, const Wellenform& welle)
{
    if (!bounceMoeglich(loop, ort) || nennung.isEmpty()) return {};
    const int clip = findeClip(loop, ort);
    const std::optional<Loop> herkunft = clipDatei(loop, ort.kanal, clip);
    if (!herkunft) return {};
    bool abgeschnitten = false;
    const int takte = bounceTakte(loop, ort, welle, &abgeschnitten);
    LoopClip& c = loop.kanaele[static_cast<std::size_t>(ort.kanal)].clips[static_cast<std::size_t>(clip)];

    Muster wave;
    wave.name = freierMusterName(loop, c.muster + QStringLiteral(" bounce"));
    wave.art = MusterArt::Wave;
    wave.takte = takte;
    wave.wave.datei = nennung;
    wave.wave.bezugBpm = loop.bpm;  // WV7 a: das Tempo des Loops, Follow tempo aus
    wave.wave.folgtTempo = false;
    if (abgeschnitten) wave.wave.fadeOutMs = kBounceBlendeMs;
    wave.wave.herkunft = Herkunft(*herkunft);

    // der Clip an seinem Ort: eine Wiederholung, keine Effekte, keine Expression (eingerechnet)
    const QString name = wave.name;
    c = LoopClip(name, c.startTakt, 1);
    loop.muster.push_back(std::move(wave));
    loop.richte();
    return name;
}

bool erneuereBounce(Loop& loop, const QString& wave, const Loop& quelle, ClipOrt ort, const QString& nennung,
                    const Wellenform& welle)
{
    Muster* ziel = nullptr;
    for (Muster& m : loop.muster)
        if (m.name == wave && m.art == MusterArt::Wave) ziel = &m;
    const int clip = findeClip(quelle, ort);
    if (ziel == nullptr || clip < 0 || nennung.isEmpty()) return false;
    const std::optional<Loop> herkunft = clipDatei(quelle, ort.kanal, clip);
    if (!herkunft) return false;
    bool abgeschnitten = false;
    const int takte = bounceTakte(quelle, ort, welle, &abgeschnitten);
    ziel->wave.datei = nennung;
    ziel->wave.herkunft = Herkunft(*herkunft);
    ziel->wave.fadeOutMs = abgeschnitten ? kBounceBlendeMs : 0.0;
    const bool laenger = takte != ziel->takte;
    loop.richte();
    // die Laenge ueber den ueblichen Weg: Clips dahinter ruecken, wenn noetig
    return !laenger || setzeMusterTakte(loop, wave, takte);
}

bool hatQuelle(const Muster& wave)
{
    return wave.art == MusterArt::Wave && herkunftClip(wave) != nullptr;
}

bool quelleGeaendert(const Loop& loop, const Muster& wave)
{
    if (!hatQuelle(wave)) return false;
    const Muster& ursprung = wave.wave.herkunft.clip().muster.front();
    const Muster* jetzt = loop.findeMuster(ursprung.name);
    return jetzt == nullptr || !(*jetzt == ursprung);
}

bool setzeQuelleZurueck(Loop& loop, ClipOrt ort)
{
    const int clip = findeClip(loop, ort);
    if (clip < 0) return false;
    const LoopClip& jetzt = loop.kanaele[static_cast<std::size_t>(ort.kanal)].clips[static_cast<std::size_t>(clip)];
    const Muster* wave = loop.findeMuster(jetzt.muster);
    if (wave == nullptr || !hatQuelle(*wave)) return false;
    LoopClip ursprung = *herkunftClip(*wave);
    const Muster musterDerHerkunft = wave->wave.herkunft.clip().muster.front();
    ursprung.startTakt = ort.startTakt;
    // das jetzige gleichnamige Muster, sonst das der Herkunft — erst pruefen, ob der Platz reicht
    const Muster* vorhanden = loop.findeMuster(musterDerHerkunft.name);
    const int takte = (vorhanden != nullptr ? vorhanden->takte : musterDerHerkunft.takte) * std::max(1, ursprung.wiederholungen);
    if (!istFrei(loop, ort.kanal, ort.startTakt, takte, clip)) return false;
    if (vorhanden == nullptr) loop.muster.push_back(musterDerHerkunft);
    loop.kanaele[static_cast<std::size_t>(ort.kanal)].clips[static_cast<std::size_t>(clip)] = std::move(ursprung);
    loop.richte();
    return true;
}

}  // namespace lumi::loop
