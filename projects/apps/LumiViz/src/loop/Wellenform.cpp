/**
 * @file Wellenform.cpp
 * @brief Loop: ein Clip als Wellenform (Stufe 30a; der ganze Loop seit 30c)
 */

#include "loop/Wellenform.hpp"

#include "audio/Begrenzer.hpp"

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

/// Hinter `abTakt` Platz fuer den laengsten Nachklang: ein ausgeschalteter Kanal zaehlt beim
/// Durchlauf mit, ein Muster ohne Spuren klingt nicht.
void platzFuerNachklang(Loop& fassung, const Loop& vorbild, int abTakt)
{
    const double msJeTakt = 60000.0 / vorbild.bpm * vorbild.schlaegeJeTakt;
    const int platz = static_cast<int>(std::ceil(kNachklangHoechstensMs / msJeTakt)) + 1;
    Muster leer;
    leer.name = freierName(fassung);
    leer.takte = 1;
    fassung.muster.push_back(leer);
    Kanal ende;
    ende.name = leer.name;
    ende.an = false;
    ende.clips = {LoopClip{leer.name, abTakt + platz, 1}};
    fassung.kanaele.push_back(std::move(ende));
}

/// `frames` Abtastwerte ab der gesetzten Stelle rechnen und anhaengen, in Bloecken wie im
/// Mixer. `weiter` sieht jeden Block (Anfang in Frames, Zahl der Frames) und sagt, ob es
/// weitergeht.
template <typename Weiter>
void rechneBloecke(Klangerzeuger& erzeuger, std::int64_t frames, std::vector<float>& daten, Weiter&& weiter)
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
        if (!weiter(anfang / 2, n)) return;
    }
}

/// Ab `anfang` den Clip (`clipFrames`) und seinen Nachklang rechnen: er endet, wenn die
/// Summe `kNachklangStilleMs` lang unter `kNachklangStilleDb` bleibt, sonst nach
/// `kNachklangHoechstensMs`. false = abgebrochen.
bool rechneMitNachklang(const std::shared_ptr<const Programm>& programm, std::int64_t anfang, Wellenform& welle,
                        const Fortschritt& fortschritt)
{
    const int rate = welle.rate;
    const std::int64_t hoechstens = welle.clipFrames + std::int64_t{kNachklangHoechstensMs} * rate / 1000;
    const std::int64_t stilleFrames = std::int64_t{kNachklangStilleMs} * rate / 1000;
    const float grenze = static_cast<float>(std::pow(10.0, kNachklangStilleDb / 20.0));

    Klangerzeuger erzeuger;
    erzeuger.setzeProgramm(programm);
    erzeuger.setzeStelle(anfang);
    std::int64_t stillSeit = -1;  // seit wann hinter dem Ende alles unter der Grenze liegt
    std::int64_t still = -1;      // wo der Nachklang endet
    bool abgebrochen = false;
    rechneBloecke(erzeuger, hoechstens, welle.klang.daten, [&](std::size_t ab, int n) {
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
    if (abgebrochen) return false;
    welle.nachklangGekappt = still < 0;
    welle.klang.daten.resize(2 * static_cast<std::size_t>(still < 0 ? hoechstens : still));
    return true;
}

/// Den Limiter des Loops ueber alles legen, wie im Panel (27e-3). Er verzoegert um
/// `verzoegerungFrames()`: so viele Frames Stille kommen hinten dazu, vorn fallen so viele weg.
void begrenze(std::vector<float>& daten, int rate, std::size_t nachlauf = 0)
{
    Begrenzer begrenzer(rate);
    const std::size_t verzug = begrenzer.verzoegerungFrames();
    const std::size_t frames = daten.size() / 2;
    if (nachlauf < verzug) daten.resize(daten.size() + 2 * (verzug - nachlauf), 0.0f);
    begrenzer.verarbeite(daten.data(), daten.size() / 2);
    daten.erase(daten.begin(), daten.begin() + static_cast<std::ptrdiff_t>(2 * verzug));
    daten.resize(2 * (frames - nachlauf));
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
    fassung.kanaele = {std::move(eigener)};
    platzFuerNachklang(fassung, loop, clip.startTakt + loop.clipTakte(clip));
    return fassung;
}

std::optional<Wellenform> rechneClip(const Loop& loop, ClipOrt ort, int rate, std::shared_ptr<const Klaenge> klaenge,
                                     const InstrumentVon& saiten, const Fortschritt& fortschritt)
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
    if (!rechneMitNachklang(programm, anfang, welle, fortschritt)) return std::nullopt;
    if (fortschritt) (void)fortschritt(1.0);
    return welle;
}

std::optional<Loop> loopFassung(const Loop& loop, int durchlaeufe)
{
    if (loop.bpm <= 0.0) return std::nullopt;
    const bool clips = std::any_of(loop.kanaele.begin(), loop.kanaele.end(), [](const Kanal& k) { return !k.clips.empty(); });
    if (!clips) return std::nullopt;
    const int mal = std::clamp(durchlaeufe, 1, kDurchlaeufeMax);
    const int takte = loop.durchlaufTakte();
    Loop fassung = loop;
    for (Kanal& kanal : fassung.kanaele)
    {
        const std::vector<LoopClip> einmal = kanal.clips;
        for (int runde = 1; runde < mal; ++runde)
            for (LoopClip clip : einmal)
            {
                clip.startTakt += runde * takte;
                kanal.clips.push_back(std::move(clip));
            }
    }
    platzFuerNachklang(fassung, loop, mal * takte);
    return fassung;
}

QStringList loopKlangNamen(const Loop& loop, LoopEnde ende, int durchlaeufe, const InstrumentVon& saiten)
{
    if (ende == LoopEnde::Nahtlos) return loop.klangNamen(saiten);
    const std::optional<Loop> fassung = loopFassung(loop, durchlaeufe);
    return fassung ? fassung->klangNamen(saiten) : QStringList{};
}

std::optional<Wellenform> rechneLoop(const Loop& loop, LoopEnde ende, int durchlaeufe, int rate,
                                     std::shared_ptr<const Klaenge> klaenge, const InstrumentVon& saiten,
                                     const Fortschritt& fortschritt)
{
    if (rate <= 0) return std::nullopt;
    const int mal = std::clamp(durchlaeufe, 1, kDurchlaeufeMax);
    Wellenform welle;
    welle.rate = rate;

    if (ende == LoopEnde::Nachklang)
    {
        const std::optional<Loop> fassung = loopFassung(loop, mal);
        if (!fassung) return std::nullopt;
        const std::shared_ptr<const Programm> programm = baueProgramm(*fassung, rate, std::move(klaenge), saiten);
        welle.clipFrames = std::llround(static_cast<double>(mal * loop.durchlaufSchritte()) * programm->ablauf.schrittFrames);
        if (!rechneMitNachklang(programm, 0, welle, fortschritt)) return std::nullopt;
        if (loop.limiter) begrenze(welle.klang.daten, rate);
        if (fortschritt) (void)fortschritt(1.0);
        return welle;
    }

    // nahtlos: der erste Durchlauf gibt Effekten und Limiter ihr Gedaechtnis und faellt weg
    const std::shared_ptr<const Programm> programm = baueProgramm(loop, rate, std::move(klaenge), saiten);
    const std::int64_t durchlauf = programm->ablauf.durchlaufFrames;
    if (durchlauf <= 0) return std::nullopt;
    const std::size_t verzug = loop.limiter ? Begrenzer(rate).verzoegerungFrames() : 0;
    const std::int64_t alle = durchlauf * (mal + 1) + static_cast<std::int64_t>(verzug);
    Klangerzeuger erzeuger;
    erzeuger.setzeProgramm(programm);
    erzeuger.setzeStelle(0);
    bool abgebrochen = false;
    rechneBloecke(erzeuger, alle, welle.klang.daten, [&](std::size_t ab, int n) {
        if (fortschritt && !fortschritt(static_cast<double>(ab + n) / static_cast<double>(alle)))
        {
            abgebrochen = true;
            return false;
        }
        return true;
    });
    if (abgebrochen) return std::nullopt;
    if (loop.limiter) begrenze(welle.klang.daten, rate, verzug);
    welle.klang.daten.erase(welle.klang.daten.begin(), welle.klang.daten.begin() + 2 * durchlauf);
    welle.clipFrames = durchlauf * mal;
    welle.klang.daten.resize(2 * static_cast<std::size_t>(welle.clipFrames));
    if (fortschritt) (void)fortschritt(1.0);
    return welle;
}

std::optional<Wellenform> rechneAusschnitt(const Loop& loop, double versatzMs, double laengeMs, int rate,
                                           std::shared_ptr<const Klaenge> klaenge, const InstrumentVon& saiten,
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
    rechneBloecke(erzeuger, frames, welle.klang.daten, [&](std::size_t ab, int n) {
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
