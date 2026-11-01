/**
 * @file WaveKlang.cpp
 * @brief Loop: der Klang eines Wave-Musters (Stufe 15c, S169)
 */

#include "loop/WaveKlang.hpp"

#include <QStringList>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <utility>

namespace lumi::loop
{

namespace
{

const QString kWaveVorsatz = QStringLiteral("wave:");
constexpr int kWerte = 7;

QString zahl(double wert)
{
    return QString::number(wert, 'g', 17);  // genau zurueckzulesen
}

}  // namespace

bool istWaveKlang(const QString& name)
{
    return name.startsWith(kWaveVorsatz);
}

QString waveKlangName(const WaveKlang& k)
{
    const QStringList werte{zahl(k.startS),  zahl(k.endeS),  zahl(k.versatzMs), zahl(k.laengeMs),
                            zahl(k.gainDb),  zahl(k.fadeInMs), zahl(k.fadeOutMs)};
    return kWaveVorsatz + werte.join(QLatin1Char(',')) + QLatin1Char('|') + k.datei;
}

bool zerlegeWaveKlang(const QString& name, WaveKlang& k)
{
    if (!istWaveKlang(name)) return false;
    const QString rest = name.mid(kWaveVorsatz.size());
    const qsizetype trenner = rest.indexOf(QLatin1Char('|'));  // ein Pfad traegt nie `|`
    if (trenner < 0) return false;
    const QStringList werte = rest.left(trenner).split(QLatin1Char(','));
    if (werte.size() != kWerte) return false;
    double z[kWerte] = {};
    for (int i = 0; i < kWerte; ++i)
    {
        bool ok = false;
        z[i] = werte[i].toDouble(&ok);
        if (!ok || !std::isfinite(z[i])) return false;
    }
    WaveKlang neu;
    neu.datei = rest.mid(trenner + 1);
    neu.startS = z[0];
    neu.endeS = z[1];
    neu.versatzMs = z[2];
    neu.laengeMs = z[3];
    neu.gainDb = z[4];
    neu.fadeInMs = z[5];
    neu.fadeOutMs = z[6];
    k = std::move(neu);
    return true;
}

WaveKlang waveKlangVon(const Loop& loop, const Muster& muster)
{
    const double schrittMs =
        loop.bpm > 0.0 && loop.schritteJeSchlag > 0 ? 60000.0 / loop.bpm / loop.schritteJeSchlag : 0.0;
    const WaveAngaben& w = muster.wave;
    WaveKlang k;
    k.datei = w.datei;
    k.startS = w.start;
    k.endeS = w.ende;
    k.versatzMs = w.versatz * schrittMs;
    k.laengeMs = static_cast<double>(muster.takte) * loop.schritteJeTakt() * schrittMs;
    k.gainDb = w.gainDb;
    k.fadeInMs = w.fadeInMs;
    k.fadeOutMs = w.fadeOutMs;
    return k;
}

double waveBedarfSekunden(const WaveKlang& k)
{
    // bis zum Ende der letzten hoerbaren Stelle: der Musteranfang liegt bei start − versatz in der Datei
    double bis = k.startS + (k.laengeMs - k.versatzMs) / 1000.0;
    if (k.endeS > 0.0) bis = std::min(bis, k.endeS);
    return std::max(0.0, bis) + 0.01;  // ein Rest fuer das Runden auf Abtastwerte
}

double waveGainFaktor(double gainDb)
{
    if (gainDb <= kWaveGainMinDb) return 0.0;  // an der unteren Grenze: −∞, wie der Fader
    return std::pow(10.0, std::min(gainDb, kWaveGainMaxDb) / 20.0);
}

Klang rechneWave(const WaveKlang& k, const std::vector<float>& datei, int rate)
{
    Klang klang;
    if (rate <= 0) return klang;
    const auto dateiFrames = static_cast<std::int64_t>(datei.size() / 2);
    const std::int64_t n = std::max<std::int64_t>(0, static_cast<std::int64_t>(std::ceil(k.laengeMs * rate / 1000.0 - 1e-9)));
    const std::int64_t d = std::llround((k.startS - k.versatzMs / 1000.0) * rate);
    const std::int64_t anfang = std::llround(k.startS * rate);
    const std::int64_t ende = k.endeS > 0.0 ? std::min(dateiFrames, std::llround(k.endeS * rate)) : dateiFrames;
    const std::int64_t a = std::max<std::int64_t>(0, anfang - d);
    const std::int64_t b = std::min(n, ende - d);
    if (b <= a) return klang;

    const double gain = waveGainFaktor(k.gainDb);
    const double fadeIn = k.fadeInMs * rate / 1000.0;
    const double fadeOut = k.fadeOutMs * rate / 1000.0;
    klang.daten.assign(static_cast<std::size_t>(2 * b), 0.0f);
    for (std::int64_t i = a; i < b; ++i)
    {
        double w = gain;
        if (fadeIn > 0.0) w *= std::min(1.0, static_cast<double>(i - a) / fadeIn);
        if (fadeOut > 0.0) w *= std::min(1.0, static_cast<double>(b - i) / fadeOut);
        const auto quelle = static_cast<std::size_t>(2 * (i + d));
        klang.daten[static_cast<std::size_t>(2 * i)] = static_cast<float>(datei[quelle] * w);
        klang.daten[static_cast<std::size_t>(2 * i + 1)] = static_cast<float>(datei[quelle + 1] * w);
    }
    return klang;
}

}  // namespace lumi::loop
