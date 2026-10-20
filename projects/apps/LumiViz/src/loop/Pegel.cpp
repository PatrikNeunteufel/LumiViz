/**
 * @file Pegel.cpp
 * @brief Loop: die Standardprobe und ihre Messung (Loop_Konzept §4n, Stufe 27b)
 */

#include "loop/Pegel.hpp"

#include <algorithm>

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
    std::vector<Klang> klaenge;
    klaenge.reserve(static_cast<std::size_t>(ablauf.klangNamen.size()));
    for (const QString& klangName : ablauf.klangNamen)
    {
        TonAngabe ton;
        klaenge.push_back(zerlegeTon(klangName, ton) ? rendereTon(instrument, ton, rate) : Klang{});
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

}  // namespace lumi::loop
