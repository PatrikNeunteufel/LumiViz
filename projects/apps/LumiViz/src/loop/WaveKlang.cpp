/**
 * @file WaveKlang.cpp
 * @brief Loop: der Klang eines Wave-Musters (Stufe 15c, S169)
 */

#include "loop/WaveKlang.hpp"

#include "loop/Autotune.hpp"
#include "loop/Dehner.hpp"

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
const QString kStimme = QStringLiteral("voice");

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
    QString name = kWaveVorsatz + werte.join(QLatin1Char(','));
    for (const SpurPunkt& p : k.tonhoehe) name += QLatin1Char(';') + zahl(p.stelle) + QLatin1Char(':') + zahl(p.wert);
    // Stufe 15i: je Abschnitt Autotune `;at:<anfang>:<ende>:<ton>:<cent>:<strength>:<speed>`
    for (const AutotuneAbschnitt& a : k.autotune)
        name += QStringLiteral(";at:") + QStringList{zahl(a.anfang), zahl(a.ende), QString::number(a.zielHoehe),
                                                     QString::number(a.zielCent), zahl(a.staerke), zahl(a.speedMs)}
                                             .join(QLatin1Char(':'));
    if (k.tempo != 1.0) name += QStringLiteral(";tempo:") + zahl(k.tempo);  // Stufe 15j
    if (k.stimme && (!k.tonhoehe.empty() || !k.autotune.empty() || k.tempo != 1.0)) name += QLatin1Char(';') + kStimme;
    return name + QLatin1Char('|') + k.datei;
}

bool zerlegeWaveKlang(const QString& name, WaveKlang& k)
{
    if (!istWaveKlang(name)) return false;
    const QString rest = name.mid(kWaveVorsatz.size());
    const qsizetype trenner = rest.indexOf(QLatin1Char('|'));  // ein Pfad traegt nie `|`
    if (trenner < 0) return false;
    // Stufe 15f: hinter den Werten je Punkt der Spur Pitch `;<ms>:<halbtoene>`
    const QStringList teile = rest.left(trenner).split(QLatin1Char(';'));
    std::vector<SpurPunkt> tonhoehe;
    std::vector<AutotuneAbschnitt> autotune;
    double tempo = 1.0;
    bool stimme = false;
    for (qsizetype i = 1; i < teile.size(); ++i)
    {
        // Stufe 15h: `;voice` als letztes, nur mit Spur oder Follow tempo
        if (teile[i] == kStimme && i + 1 == teile.size() && (!tonhoehe.empty() || !autotune.empty() || tempo != 1.0))
        {
            stimme = true;
            continue;
        }
        // Stufe 15j: `;tempo:<faktor>` nach den Abschnitten
        if (teile[i].startsWith(QLatin1String("tempo:")))
        {
            bool ok = false;
            tempo = teile[i].mid(6).toDouble(&ok);
            if (!ok || !std::isfinite(tempo) || tempo <= 0.0 || tempo == 1.0) return false;
            continue;
        }
        if (tempo != 1.0) return false;  // nach dem Tempo kommt nur noch `voice`
        // Stufe 15i: ein Abschnitt Autotune
        if (teile[i].startsWith(QLatin1String("at:")))
        {
            const QStringList w = teile[i].mid(3).split(QLatin1Char(':'));
            if (w.size() != 6) return false;
            bool ok[6] = {};
            const AutotuneAbschnitt a{w[0].toDouble(&ok[0]), w[1].toDouble(&ok[1]), w[2].toInt(&ok[2]),
                                      w[3].toInt(&ok[3]), w[4].toDouble(&ok[4]), w[5].toDouble(&ok[5])};
            if (!std::all_of(std::begin(ok), std::end(ok), [](bool b) { return b; })) return false;
            if (!std::isfinite(a.anfang) || !std::isfinite(a.ende) || !std::isfinite(a.staerke) || !std::isfinite(a.speedMs))
                return false;
            autotune.push_back(a);
            continue;
        }
        if (!autotune.empty()) return false;  // Punkte der Spur Pitch stehen vor den Abschnitten
        const QStringList paar = teile[i].split(QLatin1Char(':'));
        if (paar.size() != 2) return false;
        bool okStelle = false;
        bool okWert = false;
        const SpurPunkt p{paar[0].toDouble(&okStelle), paar[1].toDouble(&okWert)};
        if (!okStelle || !okWert || !std::isfinite(p.stelle) || !std::isfinite(p.wert)) return false;
        tonhoehe.push_back(p);
    }
    const QStringList werte = teile.front().split(QLatin1Char(','));
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
    neu.tonhoehe = std::move(tonhoehe);
    neu.autotune = std::move(autotune);
    neu.tempo = tempo;
    neu.stimme = stimme;
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
    // Stufe 15f: die Spur Pitch in Millisekunden — nur, wenn ein Punkt die Tonhoehe aendert
    const std::vector<SpurPunkt>& pitch = muster.zeitspuren.von(Steuerspur::Pitch);
    if (std::any_of(pitch.begin(), pitch.end(), [](const SpurPunkt& p) { return p.wert != 0.0; }))
        for (const SpurPunkt& p : pitch) k.tonhoehe.push_back({p.stelle * schrittMs, p.wert});
    // Stufe 15i: nur Abschnitte, die etwas tun (Strength ueber 0)
    for (const AutotuneAbschnitt& a : w.autotune.abschnitte)
        if (a.staerke > 0.0) k.autotune.push_back(a);
    k.tempo = waveTempoFaktor(w, loop.bpm);  // Stufe 15j
    k.stimme = (!k.tonhoehe.empty() || !k.autotune.empty() || k.tempo != 1.0) && w.tonhoehenArt == TonhoehenArt::Stimme;  // Stufe 15h
    return k;
}

double waveBedarfSekunden(const WaveKlang& k)
{
    // bis zum Ende der letzten hoerbaren Stelle: der Musteranfang liegt bei start − versatz in der Datei
    // mit Follow tempo (Stufe 15j) braucht eine Sekunde des Musters `tempo` Sekunden der Datei
    double bis = k.startS + (k.laengeMs - k.versatzMs) / 1000.0 * (k.tempo > 0.0 ? k.tempo : 1.0);
    if (k.endeS > 0.0) bis = std::min(bis, k.endeS);
    return std::max(0.0, bis) + 0.01;  // ein Rest fuer das Runden auf Abtastwerte
}

double waveGainFaktor(double gainDb)
{
    if (gainDb <= kWaveGainMinDb) return 0.0;  // an der unteren Grenze: −∞, wie der Fader
    return std::pow(10.0, std::min(gainDb, kWaveGainMaxDb) / 20.0);
}

Klang rechneWave(const WaveKlang& k, const std::vector<float>& datei, int rate, const Tonhoehenverlauf* verlauf)
{
    Klang klang;
    if (rate <= 0) return klang;
    const auto dateiFrames = static_cast<std::int64_t>(datei.size() / 2);
    const std::int64_t n = std::max<std::int64_t>(0, static_cast<std::int64_t>(std::ceil(k.laengeMs * rate / 1000.0 - 1e-9)));
    const std::int64_t ende = k.endeS > 0.0 ? std::min(dateiFrames, std::llround(k.endeS * rate)) : dateiFrames;
    // Stufe 15j: mit *Follow tempo* liegt Abtastwert k des Musters bei start + (k − v) · r in der
    // Datei (v = die Lage in Abtastwerten, r = `tempo`); ohne genau wie seit 15c
    const bool gedehnt = k.tempo != 1.0 && k.tempo > 0.0;
    const double v = k.versatzMs * rate / 1000.0;
    const std::int64_t d = std::llround((k.startS - k.versatzMs / 1000.0) * rate);
    std::int64_t a = 0;
    std::int64_t b = 0;
    std::int64_t von = 0;  // der erste Abtastwert der Datei im hoerbaren Stueck
    std::int64_t bis = 0;
    if (!gedehnt)
    {
        a = std::max<std::int64_t>(0, std::llround(k.startS * rate) - d);
        b = std::min(n, ende - d);
        von = a + d;
        bis = b + d;
    }
    else
    {
        const double anfang = k.startS * rate;
        a = std::max<std::int64_t>(0, std::llround(v));
        b = std::min(n, std::llround(v + (static_cast<double>(ende) - anfang) / k.tempo));
        von = std::clamp<std::int64_t>(std::llround(anfang + (static_cast<double>(a) - v) * k.tempo), 0, dateiFrames);
        bis = std::clamp<std::int64_t>(std::llround(anfang + (static_cast<double>(b) - v) * k.tempo), von, dateiFrames);
    }
    if (b <= a || bis <= von) return klang;
    // die Sekunde der Datei unter Abtastwert i des hoerbaren Stuecks
    const auto dateiSekunde = [&k, gedehnt, a, d, v, rate](std::int64_t i) {
        return gedehnt ? k.startS + (static_cast<double>(a + i) - v) * k.tempo / rate : static_cast<double>(a + i + d) / rate;
    };

    const double gain = waveGainFaktor(k.gainDb);
    const double fadeIn = k.fadeInMs * rate / 1000.0;
    const double fadeOut = k.fadeOutMs * rate / 1000.0;
    // das hoerbare Stueck aus der Datei; mit Spur Pitch, Autotune oder Follow tempo durch den Dehner
    std::vector<float> stueck(datei.begin() + static_cast<std::ptrdiff_t>(2 * von),
                              datei.begin() + static_cast<std::ptrdiff_t>(2 * bis));
    // Stufe 15i: die Korrektur aus Autotune je Rahmen der Datei
    const bool mitAutotune = !k.autotune.empty() && verlauf != nullptr && !verlauf->hz.empty();
    const std::vector<double> korrektur = mitAutotune ? autotuneKorrektur(k.autotune, *verlauf) : std::vector<double>{};
    if (!k.tonhoehe.empty() || mitAutotune || gedehnt)
        stueck = (k.stimme ? dehnerStimme() : dehner())
                     .rechne(stueck, rate, b - a, [&k, &korrektur, &dateiSekunde, verlauf, mitAutotune, a, rate](std::int64_t i) {
                         const double spur = k.tonhoehe.empty() ? 0.0 : kurveBei(k.tonhoehe, 1000.0 * static_cast<double>(a + i) / rate, 0.0);
                         const double gezogen = mitAutotune ? autotuneBei(korrektur, *verlauf, dateiSekunde(i)) : 0.0;
                         return spur + gezogen;
                     });
    klang.daten.assign(static_cast<std::size_t>(2 * b), 0.0f);
    for (std::int64_t i = a; i < b; ++i)
    {
        double w = gain;
        if (fadeIn > 0.0) w *= std::min(1.0, static_cast<double>(i - a) / fadeIn);
        if (fadeOut > 0.0) w *= std::min(1.0, static_cast<double>(b - i) / fadeOut);
        const auto quelle = static_cast<std::size_t>(2 * (i - a));
        klang.daten[static_cast<std::size_t>(2 * i)] = static_cast<float>(stueck[quelle] * w);
        klang.daten[static_cast<std::size_t>(2 * i + 1)] = static_cast<float>(stueck[quelle + 1] * w);
    }
    return klang;
}

}  // namespace lumi::loop
