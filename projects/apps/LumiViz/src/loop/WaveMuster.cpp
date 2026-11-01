/**
 * @file WaveMuster.cpp
 * @brief Loop: die Angaben eines Wave-Musters (Stufe 15b, S169)
 */

#include "loop/WaveMuster.hpp"

#include "loop/Loop.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace lumi::loop
{

namespace
{

double inGrenzen(double wert, double von, double bis, double ersatz)
{
    return std::isfinite(wert) ? std::clamp(wert, von, bis) : ersatz;
}

double abNull(double wert)
{
    return std::isfinite(wert) ? std::max(0.0, wert) : 0.0;
}

}  // namespace

Herkunft::Herkunft(Loop clip)
    : m_clip(std::make_shared<const Loop>(std::move(clip)))
{
}

bool Herkunft::operator==(const Herkunft& andere) const
{
    if (m_clip == andere.m_clip) return true;  // dieselbe oder beide leer
    if (!m_clip || !andere.m_clip) return false;
    return *m_clip == *andere.m_clip;
}

void richteAutotune(Autotune& autotune)
{
    autotune.grundton = std::clamp(autotune.grundton, 0, 11);
    const int skala = static_cast<int>(autotune.skala);
    if (skala < static_cast<int>(Skala::Chromatisch) || skala > static_cast<int>(Skala::Moll))
        autotune.skala = Skala::Chromatisch;
    autotune.staerke = inGrenzen(autotune.staerke, 0.0, 1.0, 1.0);
    autotune.speedMs = inGrenzen(autotune.speedMs, 0.0, kAutotuneSpeedMaxMs, 20.0);

    std::vector<AutotuneAbschnitt>& liste = autotune.abschnitte;
    for (AutotuneAbschnitt& a : liste)
    {
        a.anfang = abNull(a.anfang);
        a.ende = abNull(a.ende);
        a.zielHoehe = std::clamp(a.zielHoehe, 0, 127);
        a.zielCent = std::clamp(a.zielCent, -kAutotuneCentMax, kAutotuneCentMax);
        a.staerke = inGrenzen(a.staerke, 0.0, 1.0, autotune.staerke);
        a.speedMs = inGrenzen(a.speedMs, 0.0, kAutotuneSpeedMaxMs, autotune.speedMs);
    }
    std::stable_sort(liste.begin(), liste.end(),
                     [](const AutotuneAbschnitt& a, const AutotuneAbschnitt& b) { return a.anfang < b.anfang; });
    // Ueberlappung: der spaetere beginnt, wo der fruehere endet; was dann keine Laenge hat, geht
    double frei = 0.0;
    for (AutotuneAbschnitt& a : liste)
    {
        a.anfang = std::max(a.anfang, frei);
        if (a.ende > a.anfang) frei = a.ende;
    }
    liste.erase(std::remove_if(liste.begin(), liste.end(),
                               [](const AutotuneAbschnitt& a) { return !(a.ende > a.anfang); }),
                liste.end());
    if (liste.size() > static_cast<std::size_t>(kAutotuneAbschnitteMax))
        liste.resize(static_cast<std::size_t>(kAutotuneAbschnitteMax));
}

double waveTempoFaktor(const WaveAngaben& wave, double loopBpm)
{
    if (!wave.folgtTempo || !(wave.bezugBpm > 0.0) || !(loopBpm > 0.0)) return 1.0;
    return loopBpm / wave.bezugBpm;
}

void richteWave(WaveAngaben& wave, double schritte)
{
    wave.datei = wave.datei.trimmed();
    wave.start = abNull(wave.start);
    wave.ende = abNull(wave.ende);
    if (wave.ende > 0.0 && wave.ende <= wave.start) wave.ende = 0.0;  // ein leerer Ausschnitt: bis zum Ende
    const double weit = std::max(0.0, schritte);
    wave.versatz = inGrenzen(wave.versatz, -weit, weit, 0.0);
    wave.gainDb = inGrenzen(wave.gainDb, kWaveGainMinDb, kWaveGainMaxDb, 0.0);
    wave.fadeInMs = inGrenzen(wave.fadeInMs, 0.0, kWaveBlendeMaxMs, 0.0);
    wave.fadeOutMs = inGrenzen(wave.fadeOutMs, 0.0, kWaveBlendeMaxMs, 0.0);
    wave.bezugBpm = inGrenzen(wave.bezugBpm, kBpmMin, kBpmMax, 120.0);
    const int art = static_cast<int>(wave.tonhoehenArt);
    if (art < static_cast<int>(TonhoehenArt::Allgemein) || art > static_cast<int>(TonhoehenArt::Stimme))
        wave.tonhoehenArt = TonhoehenArt::Allgemein;
    richteAutotune(wave.autotune);
}

QString tonhoehenArtName(TonhoehenArt art)
{
    return art == TonhoehenArt::Stimme ? QStringLiteral("voice") : QStringLiteral("general");
}

TonhoehenArt tonhoehenArtAusName(const QString& name)
{
    return name == QLatin1String("voice") ? TonhoehenArt::Stimme : TonhoehenArt::Allgemein;
}

QString skalaName(Skala skala)
{
    switch (skala)
    {
    case Skala::Dur: return QStringLiteral("major");
    case Skala::Moll: return QStringLiteral("minor");
    case Skala::Chromatisch: break;
    }
    return QStringLiteral("chromatic");
}

Skala skalaAusName(const QString& name)
{
    if (name == QLatin1String("major")) return Skala::Dur;
    if (name == QLatin1String("minor")) return Skala::Moll;
    return Skala::Chromatisch;
}

}  // namespace lumi::loop
