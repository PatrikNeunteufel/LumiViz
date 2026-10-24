#include "loop/Boden.hpp"

#include "InstrumentBody/Antwort.hpp"
#include "InstrumentBody/Statistik.hpp"
#include "loop/HammerTon.hpp"
#include "loop/Koerper.hpp"
#include "loop/KoerperBauplan.hpp"
#include "loop/Tonspeicher.hpp"

#include <QCryptographicHash>
#include <QFile>
#include <QJsonDocument>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <map>
#include <mutex>

namespace lumi::loop
{

std::vector<koerper::ModeImKoerper> alleModen(const koerper::Koerper& koerper)
{
    std::vector<koerper::ModeImKoerper> alle = koerper.moden;
    for (koerper::ModeImKoerper& m : koerper::ziehe(koerper.statistik)) alle.push_back(std::move(m));
    return alle;
}

int stegpunktVon(const koerper::Koerper& koerper, int taste)
{
    for (std::size_t j = 0; j < koerper.stegTasten.size(); ++j)
        if (koerper.stegTasten[j] == taste) return static_cast<int>(j);
    return -1;
}

std::vector<double> bodenAntwort(const std::vector<koerper::ModeImKoerper>& moden, std::size_t stegpunkt,
                                 std::size_t hoerort, int rate, double laengeS, const Fft& umsetzung)
{
    const auto frames = static_cast<std::size_t>(std::ceil(laengeS * rate));
    if (frames == 0) return {};
    const std::size_t n = zweierpotenzAb(frames);
    const std::vector<std::complex<double>> h = koerper::antwortSpektrum(moden, stegpunkt, hoerort, rate, n);
    std::vector<double> zeit(n);
    umsetzung.rueckwaerts(h, zeit);
    zeit.resize(frames);
    return zeit;
}

std::vector<double> schalldruck(std::span<const float> kraft, std::span<const double> antwort, const Fft& umsetzung)
{
    std::vector<double> f(kraft.size());
    for (std::size_t i = 0; i < kraft.size(); ++i) f[i] = static_cast<double>(kraft[i]) * kHammerKraftBezug;
    return falte(umsetzung, f, antwort);
}

namespace
{

/// Der Hoerort dieses Namens, sonst `ersatz` (in den Grenzen der vorhandenen).
int hoerortNamens(const koerper::Koerper& koerper, const char* name, int ersatz)
{
    for (std::size_t h = 0; h < koerper.hoerorte.size(); ++h)
        if (koerper.hoerorte[h] == name) return static_cast<int>(h);
    return std::min(ersatz, static_cast<int>(koerper.hoerorte.size()) - 1);
}

std::shared_ptr<const BodenImKlang> ladeBoden(const QString& pfad)
{
    QFile datei(pfad);
    if (!datei.open(QIODevice::ReadOnly)) return nullptr;
    const QByteArray inhalt = datei.readAll();
    QJsonParseError fehler;
    const QJsonDocument doc = QJsonDocument::fromJson(inhalt, &fehler);
    if (fehler.error != QJsonParseError::NoError || !doc.isObject()) return nullptr;
    auto boden = std::make_shared<BodenImKlang>();
    if (!ausJson(doc.object(), boden->koerper) || boden->koerper.hoerorte.empty()) return nullptr;
    boden->moden = alleModen(boden->koerper);
    boden->laengeS = koerper::antwortLaengeS(boden->moden);
    boden->abdruck = QString::fromLatin1(QCryptographicHash::hash(inhalt, QCryptographicHash::Sha256).toHex());
    boden->links = hoerortNamens(boden->koerper, "left", 0);
    boden->rechts = hoerortNamens(boden->koerper, "right", 1);
    return boden;
}

}  // namespace

std::shared_ptr<const BodenImKlang> bodenVon(const QString& verweis)
{
    static const QString kVorsatz = QStringLiteral("lib:");
    if (!verweis.startsWith(kVorsatz) || verweis.size() == kVorsatz.size()) return nullptr;
    const QString ordner = mitgelieferterKoerperOrdner();
    if (ordner.isEmpty()) return nullptr;
    const QString pfad = ordner + QLatin1Char('/') + verweis.mid(kVorsatz.size()) + QStringLiteral(".lvinstbody");

    static std::mutex sperre;
    static std::map<QString, std::shared_ptr<const BodenImKlang>> gemerkt;
    {
        const std::lock_guard<std::mutex> halter(sperre);
        const auto it = gemerkt.find(pfad);
        if (it != gemerkt.end()) return it->second;
    }
    std::shared_ptr<const BodenImKlang> boden = ladeBoden(pfad);  // ausserhalb der Sperre: rund 0,1 s
    if (!boden) return nullptr;
    const std::lock_guard<std::mutex> halter(sperre);
    return gemerkt.emplace(pfad, boden).first->second;  // kam ein anderer Thread zuvor, gilt seiner
}

std::shared_ptr<const Klang> bodenAntwortFuer(const BodenImKlang& boden, int taste, int rate)
{
    const int j = stegpunktVon(boden.koerper, taste);
    if (j < 0) return nullptr;
    const QString schluessel = QStringLiteral("body:%1:%2:%3").arg(boden.abdruck).arg(taste).arg(rate);
    if (std::shared_ptr<const Klang> da = tonspeicher().hole(schluessel)) return da;
    const auto start = std::chrono::steady_clock::now();
    const std::vector<double> links = bodenAntwort(boden.moden, static_cast<std::size_t>(j), static_cast<std::size_t>(boden.links), rate, boden.laengeS);
    const std::vector<double> rechts = bodenAntwort(boden.moden, static_cast<std::size_t>(j), static_cast<std::size_t>(boden.rechts), rate, boden.laengeS);
    auto klang = std::make_shared<Klang>();
    klang->daten.resize(2 * links.size());
    for (std::size_t i = 0; i < links.size(); ++i)
    {
        klang->daten[2 * i] = static_cast<float>(links[i]);
        klang->daten[2 * i + 1] = static_cast<float>(rechts[i]);
    }
    const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
    tonspeicher().lege(schluessel, klang, ms);
    return klang;
}

void falteMitBoden(std::span<const float> kraft, const Klang& antwort, std::vector<float>& stereo, const Fft& umsetzung)
{
    const auto lang = static_cast<std::size_t>(antwort.frames());
    if (kraft.empty() || lang == 0)
    {
        stereo.clear();
        return;
    }
    const std::size_t laenge = kraft.size() + lang - 1;
    const std::size_t n = zweierpotenzAb(laenge);
    // die Kraft einmal ins Spektrum, je Seite die Antwort (FT1 a)
    std::vector<double> zeit(n, 0.0);
    for (std::size_t i = 0; i < kraft.size(); ++i) zeit[i] = static_cast<double>(kraft[i]) * kHammerKraftBezug;
    std::vector<std::complex<double>> f(n / 2 + 1), h(n / 2 + 1);
    umsetzung.vorwaerts(zeit, f);
    stereo.assign(2 * laenge, 0.0f);
    for (std::size_t seite = 0; seite < 2; ++seite)
    {
        std::fill(zeit.begin(), zeit.end(), 0.0);
        for (std::size_t i = 0; i < lang; ++i) zeit[i] = antwort.daten[2 * i + seite];
        umsetzung.vorwaerts(zeit, h);
        for (std::size_t k = 0; k < h.size(); ++k) h[k] *= f[k];
        umsetzung.rueckwaerts(h, zeit);
        for (std::size_t i = 0; i < laenge; ++i) stereo[2 * i + seite] = static_cast<float>(zeit[i] / kSchalldruckBezug);
    }
}

}  // namespace lumi::loop
