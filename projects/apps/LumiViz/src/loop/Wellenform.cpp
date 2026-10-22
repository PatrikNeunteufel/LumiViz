/**
 * @file Wellenform.cpp
 * @brief Loop: ein Clip als Wellenform (Stufe 30a)
 */

#include "loop/Wellenform.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

namespace lumi::loop
{

namespace
{

/// Der Name des leeren Musters, das den Durchlauf verlaengert — eines, das der Loop nicht hat.
QString freierName(const Loop& loop)
{
    QString name = QStringLiteral("~tail");
    while (loop.findeMuster(name) != nullptr) name += QLatin1Char('~');
    return name;
}

/// `frames` Abtastwerte ab der gesetzten Stelle rechnen und anhaengen, in Bloecken wie im
/// Mixer. `weiter` sieht jeden Block (Anfang in `daten`, Zahl der Frames) und sagt, ob es
/// weitergeht. false = abgebrochen.
template <typename Weiter>
bool rechneBloecke(Klangerzeuger& erzeuger, std::int64_t frames, std::vector<float>& daten, Weiter&& weiter)
{
    daten.reserve(daten.size() + 2 * static_cast<std::size_t>(frames));
    std::int64_t gerechnet = 0;
    while (gerechnet < frames)
    {
        const int n = static_cast<int>(std::min<std::int64_t>(kEffektBlock, frames - gerechnet));
        const std::size_t anfang = daten.size();
        daten.resize(anfang + 2 * static_cast<std::size_t>(n));
        erzeuger.render(daten.data() + anfang, n);
        gerechnet += n;
        if (!weiter(anfang / 2, n)) return gerechnet >= frames;
    }
    return true;
}

}  // namespace

std::optional<Loop> clipFassung(const Loop& loop, ClipOrt ort)
{
    const int nr = findeClip(loop, ort);
    if (nr < 0) return std::nullopt;
    const Kanal& kanal = loop.kanaele[static_cast<std::size_t>(ort.kanal)];
    const LoopClip& clip = kanal.clips[static_cast<std::size_t>(nr)];
    const Muster* muster = loop.findeMuster(clip.muster);
    if (muster == nullptr || loop.bpm <= 0.0) return std::nullopt;

    Loop fassung = loop;
    fassung.laut = 1.0;  // nach den Effekten, bleibt live (WX5 a)
    fassung.muster = {*muster};
    Kanal eigener = kanal;
    eigener.an = true;
    eigener.gainDb = 0.0;  // der Fader bleibt live (F3 a)
    eigener.clips = {clip};

    // dahinter Platz fuer den laengsten Nachklang: ein ausgeschalteter Kanal zaehlt beim
    // Durchlauf mit, ein Muster ohne Spuren klingt nicht
    const double msJeTakt = 60000.0 / loop.bpm * loop.schlaegeJeTakt;
    const int platz = static_cast<int>(std::ceil(kNachklangHoechstensMs / msJeTakt)) + 1;
    Muster leer;
    leer.name = freierName(loop);
    leer.takte = 1;
    fassung.muster.push_back(leer);
    Kanal ende;
    ende.name = leer.name;
    ende.an = false;
    ende.clips = {LoopClip{leer.name, clip.startTakt + loop.clipTakte(clip) + platz, 1}};
    fassung.kanaele = {std::move(eigener), std::move(ende)};
    return fassung;
}

std::optional<Wellenform> rechneClip(const Loop& loop, ClipOrt ort, int rate, std::shared_ptr<const Klaenge> klaenge,
                                     const SaitenVon& saiten, const Fortschritt& fortschritt)
{
    const std::optional<Loop> fassung = clipFassung(loop, ort);
    if (!fassung || rate <= 0) return std::nullopt;
    const LoopClip& clip = fassung->kanaele.front().clips.front();
    const std::shared_ptr<const Programm> programm = baueProgramm(*fassung, rate, std::move(klaenge), saiten);
    const double schritt = programm->ablauf.schrittFrames;
    const double jeTakt = fassung->schritteJeTakt();
    // dieselbe Stelle wie live: der Clip bleibt an seinem Takt
    const std::int64_t anfang = std::llround(clip.startTakt * jeTakt * schritt);
    const std::int64_t ende = std::llround((clip.startTakt + fassung->clipTakte(clip)) * jeTakt * schritt);

    Wellenform welle;
    welle.rate = rate;
    welle.clipFrames = ende - anfang;
    const std::int64_t hoechstens = welle.clipFrames + std::int64_t{kNachklangHoechstensMs} * rate / 1000;
    const std::int64_t stilleFrames = std::int64_t{kNachklangStilleMs} * rate / 1000;
    const float grenze = static_cast<float>(std::pow(10.0, kNachklangStilleDb / 20.0));

    Klangerzeuger erzeuger;
    erzeuger.setzeProgramm(programm);
    erzeuger.setzeStelle(anfang);
    std::int64_t stillSeit = -1;  // seit wann hinter dem Ende alles unter der Grenze liegt
    std::int64_t still = -1;      // wo der Nachklang endet
    bool abgebrochen = false;
    (void)rechneBloecke(erzeuger, hoechstens, welle.klang.daten, [&](std::size_t ab, int n) {
        const float* werte = welle.klang.daten.data() + 2 * ab;
        for (int i = 0; i < n; ++i)
        {
            const auto frame = static_cast<std::int64_t>(ab) + i;
            if (frame < welle.clipFrames) continue;
            if (std::abs(werte[2 * i]) < grenze && std::abs(werte[2 * i + 1]) < grenze)
            {
                if (stillSeit < 0) stillSeit = frame;
                if (frame + 1 - stillSeit >= stilleFrames)
                {
                    still = stillSeit;
                    return false;
                }
            }
            else
                stillSeit = -1;
        }
        if (fortschritt && !fortschritt(static_cast<double>(ab + n) / static_cast<double>(hoechstens)))
        {
            abgebrochen = true;
            return false;
        }
        return true;
    });
    if (abgebrochen) return std::nullopt;
    welle.nachklangGekappt = still < 0;
    const std::int64_t laenge = still < 0 ? hoechstens : still;
    welle.klang.daten.resize(2 * static_cast<std::size_t>(laenge));
    if (fortschritt) (void)fortschritt(1.0);
    return welle;
}

std::optional<Wellenform> rechneAusschnitt(const Loop& loop, double versatzMs, double laengeMs, int rate,
                                           std::shared_ptr<const Klaenge> klaenge, const SaitenVon& saiten,
                                           const Fortschritt& fortschritt)
{
    if (rate <= 0) return std::nullopt;
    const std::shared_ptr<const Programm> programm = baueProgramm(loop, rate, std::move(klaenge), saiten);
    const std::int64_t durchlauf = programm->ablauf.durchlaufFrames;
    const auto frames = static_cast<std::int64_t>(std::llround(laengeMs * rate / 1000.0));
    if (durchlauf <= 0 || frames <= 0) return std::nullopt;
    // wie die Tonfuehrung: die Stelle im Durchlauf, er wiederholt sich
    const double durchlaufMs = 1000.0 * static_cast<double>(durchlauf) / rate;
    double rest = std::fmod(versatzMs, durchlaufMs);
    if (rest < 0.0) rest += durchlaufMs;

    Wellenform welle;
    welle.rate = rate;
    welle.clipFrames = frames;
    Klangerzeuger erzeuger;
    erzeuger.setzeProgramm(programm);
    erzeuger.setzeStelle(std::llround(rest * rate / 1000.0));
    bool abgebrochen = false;
    (void)rechneBloecke(erzeuger, frames, welle.klang.daten, [&](std::size_t ab, int n) {
        if (fortschritt && !fortschritt(static_cast<double>(ab + n) / static_cast<double>(frames)))
        {
            abgebrochen = true;
            return false;
        }
        return true;
    });
    if (abgebrochen) return std::nullopt;
    if (fortschritt) (void)fortschritt(1.0);
    return welle;
}

}  // namespace lumi::loop
