/**
 * @file Pegel.cpp
 * @brief Loop: die Standardprobe und ihre Messung (Loop_Konzept §4n, Stufe 27b)
 */

#include "loop/Pegel.hpp"

#include "loop/Sample.hpp"

#include <algorithm>
#include <cmath>

namespace lumi::loop
{

namespace
{

constexpr int kStufen[] = {0, 2, 4, 5, 7, 9, 11, 12, 14, 16, 17, 19, 21, 23, 24};
constexpr int kAkkord[] = {0, 4, 7, 12};
constexpr int kTakte = 4;  // drei Takte Probe, einer Ausklang

}  // namespace

Loop standardProbe(const QString& instrument, const QString& art)
{
    const int grund = art == QLatin1String("bass") ? 36 : 60;
    Muster m;
    m.name = QStringLiteral("Probe");
    m.art = MusterArt::Instrument;
    m.instrument = instrument;
    m.takte = kTakte;
    int schritt = 0;
    for (const int stufe : kStufen)
    {
        m.noten.push_back({schritt, 2, grund + stufe, 0.9});
        schritt += 2;
    }
    schritt += 2;
    for (const int stufe : kAkkord) m.noten.push_back({schritt, 16, grund + stufe, 0.7});

    Loop loop;
    loop.laut = 1.0;
    loop.muster = {m};
    loop.kanaele = {{QStringLiteral("Probe"), true, {{QStringLiteral("Probe"), 0, 1}}}};
    loop.richte();
    return loop;
}

std::vector<float> rechneStandardprobe(const Instrument& instrument, int rate)
{
    const QString name = instrument.name.isEmpty() ? QStringLiteral("probe") : instrument.name;
    const Loop probe = standardProbe(name, instrument.art);
    const Saitensatz saiten = instrument.saiten;
    const SaitenVon saitenVon = [saiten](const QString&) { return saiten; };
    const Ablauf ablauf = plane(probe, rate, saitenVon);
    // das Instrument, wie es gerade ist (auch ungespeichert): ohne den Tonspeicher gerechnet
    Klaenge klaenge;
    klaenge.reserve(static_cast<std::size_t>(ablauf.klangNamen.size()));
    for (const QString& klangName : ablauf.klangNamen)
    {
        TonAngabe ton;
        klaenge.push_back(zerlegeTon(klangName, ton) ? std::make_shared<const Klang>(rendereTon(instrument, ton, rate)) : nullptr);
    }
    const auto frames = static_cast<int>(ablauf.durchlaufFrames);
    std::vector<float> aus(static_cast<std::size_t>(std::max(0, frames)) * 2);
    if (frames > 0) rechne(ablauf, klaenge, 0, frames, aus.data());
    return aus;
}

audio::Lautheit messeInstrument(const Instrument& instrument, int rate)
{
    return audio::messeLautheit(rechneStandardprobe(instrument, rate), rate);
}

std::optional<double> abstandZumBezug(const Instrument& instrument, int rate)
{
    if (instrument.art == QLatin1String("drums")) return std::nullopt;
    for (const Schicht& schicht : instrument.schichten)
        if (schicht.an && schicht.schwingung == Schwingung::Sample && sampleDaten(schicht.sample) == nullptr)
            return std::nullopt;
    const double wert = messeInstrument(instrument, rate).kurzzeitMaxLufs;
    if (wert <= audio::kLufsStille) return std::nullopt;
    return wert - kInstrumentBezugLufs;
}

std::optional<double> lautAufBezug(const Instrument& instrument, int rate)
{
    const std::optional<double> abstand = abstandZumBezug(instrument, rate);
    if (!abstand) return std::nullopt;
    return std::clamp(instrument.laut * std::pow(10.0, -*abstand / 20.0), 0.0, kInstrumentLautMax);
}

audio::Lautheit messeSchlag(const Klang& klang, int rate)
{
    std::vector<float> ton = klang.daten;
    ton.resize(ton.size() + 2 * static_cast<std::size_t>(std::max(1, rate)), 0.0f);
    return audio::messeLautheit(ton, rate);
}

audio::Lautheit messeDrumInstrument(const Instrument& instrument, int rate)
{
    return messeSchlag(rendereTon(instrument, 36, kSpurDauerMs, rate), rate);
}

std::vector<float> rechneDrumProbe(const Klang& schlag, int rate)
{
    Muster m;
    m.name = QStringLiteral("Probe");
    m.art = MusterArt::Drumkit;
    m.takte = 3;  // zwei Takte Probe, einer Ausklang
    m.spuren = {Spur(QStringLiteral("Probe"), QStringLiteral("kit:probe"), 1.0, 0.0)};
    m.zeilen = {std::vector<double>(static_cast<std::size_t>(3 * 16), 0.0)};
    for (int schritt = 0; schritt < 2 * 16; schritt += 4) m.zeilen[0][static_cast<std::size_t>(schritt)] = 1.0;
    Loop probe;
    probe.laut = 1.0;
    probe.muster = {m};
    probe.kanaele = {{QStringLiteral("Probe"), true, {{QStringLiteral("Probe"), 0, 1}}}};
    probe.richte();
    const Ablauf ablauf = plane(probe, rate);
    const Klaenge klaenge(static_cast<std::size_t>(ablauf.klangNamen.size()), std::make_shared<const Klang>(schlag));
    const auto frames = static_cast<int>(ablauf.durchlaufFrames);
    std::vector<float> aus(static_cast<std::size_t>(std::max(0, frames)) * 2);
    if (frames > 0) rechne(ablauf, klaenge, 0, frames, aus.data());
    return aus;
}

audio::Lautheit messeDrumProbe(const Klang& schlag, int rate)
{
    return audio::messeLautheit(rechneDrumProbe(schlag, rate), rate);
}

}  // namespace lumi::loop
