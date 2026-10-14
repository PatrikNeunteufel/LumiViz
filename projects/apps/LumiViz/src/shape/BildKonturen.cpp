/**
 * @file BildKonturen.cpp
 * @brief Klang aus einer Form: Konturen aus einem Bild (Stufe 20c)
 */

#include "shape/BildKonturen.hpp"

#include "shape/Kontur.hpp"  // kKonturenMax

#include <QJsonValue>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <string>
#include <unordered_set>
#include <utility>

namespace lumi::shape
{

namespace
{

/// So viele Durchgaenge, in denen zu kleine Flaechen in ihren Nachbarn aufgehen.
constexpr int kVerschmelzRunden = 4;
/// So viele Punkte nimmt die Farbteilung hoechstens als Stichprobe.
constexpr std::size_t kFarbStichprobe = 40000;
constexpr int kFarbRunden = 15;

using Farbe = std::array<double, 3>;

/// Ein Punkt des Bilds, auf Weiss gelegt (durchsichtig = weiss) — fuer Helligkeit und Farbe.
Farbe aufWeiss(QRgb p)
{
    const double a = qAlpha(p) / 255.0;
    return {qRed(p) * a + 255.0 * (1.0 - a), qGreen(p) * a + 255.0 * (1.0 - a), qBlue(p) * a + 255.0 * (1.0 - a)};
}

double helligkeit(const Farbe& f)
{
    return (0.299 * f[0] + 0.587 * f[1] + 0.114 * f[2]) / 255.0;
}

double abstand2(const Farbe& a, const Farbe& b)
{
    const double r = a[0] - b[0];
    const double g = a[1] - b[1];
    const double bl = a[2] - b[2];
    return r * r + g * g + bl * bl;
}

/// Weichzeichnen: zweimal ein gleitendes Mittel ueber 2r+1 Punkte, waagrecht und senkrecht.
QImage weich(const QImage& bild, int r)
{
    if (r <= 0) return bild;
    QImage aus = bild.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    const int w = aus.width();
    const int h = aus.height();
    std::vector<std::array<int, 4>> zeile(static_cast<std::size_t>(std::max(w, h)));
    const auto kanal = [](QRgb p, int k) { return k == 0 ? qAlpha(p) : k == 1 ? qRed(p) : k == 2 ? qGreen(p) : qBlue(p); };
    const auto lauf = [&](auto lies, auto schreibe, int laenge) {
        for (int i = 0; i < laenge; ++i)
        {
            const QRgb p = lies(i);
            for (int k = 0; k < 4; ++k) zeile[static_cast<std::size_t>(i)][static_cast<std::size_t>(k)] = kanal(p, k);
        }
        std::array<int, 4> summe{};
        const auto wert = [&](int i, int k) { return zeile[static_cast<std::size_t>(std::clamp(i, 0, laenge - 1))][static_cast<std::size_t>(k)]; };
        for (int k = 0; k < 4; ++k)
            for (int i = -r; i <= r; ++i) summe[static_cast<std::size_t>(k)] += wert(i, k);
        const int breite = 2 * r + 1;
        for (int i = 0; i < laenge; ++i)
        {
            schreibe(i, qRgba(summe[1] / breite, summe[2] / breite, summe[3] / breite, summe[0] / breite));
            for (int k = 0; k < 4; ++k) summe[static_cast<std::size_t>(k)] += wert(i + r + 1, k) - wert(i - r, k);
        }
    };
    for (int durchgang = 0; durchgang < 2; ++durchgang)
    {
        for (int y = 0; y < h; ++y)
        {
            auto* z = reinterpret_cast<QRgb*>(aus.scanLine(y));
            lauf([z](int i) { return z[i]; }, [z](int i, QRgb p) { z[i] = p; }, w);
        }
        for (int x = 0; x < w; ++x)
            lauf([&aus, x](int i) { return reinterpret_cast<const QRgb*>(aus.constScanLine(i))[x]; },
                 [&aus, x](int i, QRgb p) { reinterpret_cast<QRgb*>(aus.scanLine(i))[x] = p; }, h);
    }
    return aus.convertToFormat(QImage::Format_ARGB32);
}

/// Jeder Punkt eine Klasse 0 … stufen-1.
std::vector<int> klassen(const QImage& bild, const BildEinstellung& e)
{
    const int w = bild.width();
    const int h = bild.height();
    std::vector<int> k(static_cast<std::size_t>(w) * static_cast<std::size_t>(h), 0);
    const int stufen = std::clamp(e.stufen, 2, 8);
    const auto punkt = [&bild](int x, int y) { return reinterpret_cast<const QRgb*>(bild.constScanLine(y))[x]; };
    if (e.teilung != Teilung::Farbe)
    {
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
            {
                const QRgb p = punkt(x, y);
                const double wert = e.teilung == Teilung::Deckkraft ? qAlpha(p) / 255.0 : helligkeit(aufWeiss(p));
                k[static_cast<std::size_t>(y) * static_cast<std::size_t>(w) + static_cast<std::size_t>(x)] =
                    std::min(stufen - 1, static_cast<int>(wert * stufen));
            }
        return k;
    }

    // Farbe: k-means auf einer Stichprobe, ohne Zufall. Die Startmitten: die dunkelste Farbe, dann jeweils die,
    // die von allen bisherigen am weitesten weg liegt — so bekommt auch eine kleine, abweichende Flaeche eine
    // eigene Mitte (nach Quantilen der Helligkeit fielen zwei Mitten in eine grosse Flaeche)
    const std::size_t alle = k.size();
    const std::size_t schritt = std::max<std::size_t>(1, alle / kFarbStichprobe);
    std::vector<Farbe> probe;
    for (std::size_t i = 0; i < alle; i += schritt)
        probe.push_back(aufWeiss(punkt(static_cast<int>(i % static_cast<std::size_t>(w)), static_cast<int>(i / static_cast<std::size_t>(w)))));
    std::vector<Farbe> mitte;
    mitte.push_back(*std::min_element(probe.begin(), probe.end(), [](const Farbe& a, const Farbe& b) { return helligkeit(a) < helligkeit(b); }));
    std::vector<double> naechsterAbstand(probe.size());
    for (std::size_t i = 0; i < probe.size(); ++i) naechsterAbstand[i] = abstand2(probe[i], mitte[0]);
    while (mitte.size() < static_cast<std::size_t>(stufen))
    {
        const auto fern = std::max_element(naechsterAbstand.begin(), naechsterAbstand.end());
        if (*fern <= 0.0) break;  // weniger verschiedene Farben als Stufen
        mitte.push_back(probe[static_cast<std::size_t>(fern - naechsterAbstand.begin())]);
        for (std::size_t i = 0; i < probe.size(); ++i) naechsterAbstand[i] = std::min(naechsterAbstand[i], abstand2(probe[i], mitte.back()));
    }
    const auto naechste = [&mitte](const Farbe& f) {
        std::size_t beste = 0;
        double kleinste = abstand2(f, mitte[0]);
        for (std::size_t s = 1; s < mitte.size(); ++s)
            if (const double d = abstand2(f, mitte[s]); d < kleinste)
            {
                kleinste = d;
                beste = s;
            }
        return static_cast<int>(beste);
    };
    for (int runde = 0; runde < kFarbRunden; ++runde)
    {
        std::vector<Farbe> summe(mitte.size(), Farbe{0.0, 0.0, 0.0});
        std::vector<std::size_t> zahl(mitte.size(), 0);
        for (const Farbe& f : probe)
        {
            const auto s = static_cast<std::size_t>(naechste(f));
            for (int c = 0; c < 3; ++c) summe[s][static_cast<std::size_t>(c)] += f[static_cast<std::size_t>(c)];
            ++zahl[s];
        }
        for (std::size_t s = 0; s < mitte.size(); ++s)
            if (zahl[s] > 0)
                for (int c = 0; c < 3; ++c) mitte[s][static_cast<std::size_t>(c)] = summe[s][static_cast<std::size_t>(c)] / static_cast<double>(zahl[s]);
    }
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            k[static_cast<std::size_t>(y) * static_cast<std::size_t>(w) + static_cast<std::size_t>(x)] = naechste(aufWeiss(punkt(x, y)));
    return k;
}

/// Zusammenhaengende Punkte einer Klasse (je vier Nachbarn): die Flaeche jedes Punkts.
/// Zurueck: die Zahl der Flaechen; `flaechen` die Punkte je Flaeche.
int flaechenVon(const std::vector<int>& klasse, int w, int h, std::vector<int>& flaeche, std::vector<std::size_t>& groesse)
{
    flaeche.assign(klasse.size(), -1);
    groesse.clear();
    std::vector<std::size_t> stapel;
    int zahl = 0;
    for (std::size_t start = 0; start < klasse.size(); ++start)
    {
        if (flaeche[start] >= 0) continue;
        const int k = klasse[start];
        std::size_t punkte = 0;
        stapel.push_back(start);
        flaeche[start] = zahl;
        while (!stapel.empty())
        {
            const std::size_t i = stapel.back();
            stapel.pop_back();
            ++punkte;
            const int x = static_cast<int>(i % static_cast<std::size_t>(w));
            const int y = static_cast<int>(i / static_cast<std::size_t>(w));
            const auto nimm = [&](int nx, int ny) {
                if (nx < 0 || ny < 0 || nx >= w || ny >= h) return;
                const std::size_t j = static_cast<std::size_t>(ny) * static_cast<std::size_t>(w) + static_cast<std::size_t>(nx);
                if (flaeche[j] >= 0 || klasse[j] != k) return;
                flaeche[j] = zahl;
                stapel.push_back(j);
            };
            nimm(x - 1, y);
            nimm(x + 1, y);
            nimm(x, y - 1);
            nimm(x, y + 1);
        }
        groesse.push_back(punkte);
        ++zahl;
    }
    return zahl;
}

/// Ein Rand als Folge von Ecken (Punkte des Gitters der Punktkanten).
using Zug = std::vector<QPointF>;

double flaecheVon(const Zug& zug)
{
    double summe = 0.0;
    for (std::size_t i = 0; i < zug.size(); ++i)
    {
        const QPointF& a = zug[i];
        const QPointF& b = zug[(i + 1) % zug.size()];
        summe += a.x() * b.y() - b.x() * a.y();
    }
    return std::abs(summe) / 2.0;
}

void vereinfacheOffen(const Zug& zug, std::size_t von, std::size_t bis, double toleranz, std::vector<bool>& bleibt)
{
    if (bis <= von + 1) return;
    const QPointF a = zug[von];
    const QPointF b = zug[bis % zug.size()];
    const double dx = b.x() - a.x();
    const double dy = b.y() - a.y();
    const double laenge = std::hypot(dx, dy);
    double weitest = -1.0;
    std::size_t wo = von;
    for (std::size_t i = von + 1; i < bis; ++i)
    {
        const QPointF p = zug[i];
        const double d = laenge > 1e-12 ? std::abs(dy * (p.x() - a.x()) - dx * (p.y() - a.y())) / laenge : std::hypot(p.x() - a.x(), p.y() - a.y());
        if (d > weitest)
        {
            weitest = d;
            wo = i;
        }
    }
    if (weitest <= toleranz) return;
    bleibt[wo] = true;
    vereinfacheOffen(zug, von, wo, toleranz, bleibt);
    vereinfacheOffen(zug, wo, bis, toleranz, bleibt);
}

/// Douglas-Peucker fuer einen geschlossenen Zug: vom ersten Punkt und dem fernsten aus.
Zug vereinfache(const Zug& zug, double toleranz)
{
    if (zug.size() <= 4) return zug;
    std::size_t fern = 0;
    double weitest = -1.0;
    for (std::size_t i = 1; i < zug.size(); ++i)
        if (const double d = std::hypot(zug[i].x() - zug[0].x(), zug[i].y() - zug[0].y()); d > weitest)
        {
            weitest = d;
            fern = i;
        }
    std::vector<bool> bleibt(zug.size(), false);
    bleibt[0] = true;
    bleibt[fern] = true;
    vereinfacheOffen(zug, 0, fern, toleranz, bleibt);
    vereinfacheOffen(zug, fern, zug.size(), toleranz, bleibt);  // `bis` = zug.size() meint wieder den ersten
    Zug aus;
    for (std::size_t i = 0; i < zug.size(); ++i)
        if (bleibt[i]) aus.push_back(zug[i]);
    return aus.size() >= 3 ? aus : zug;
}

/// Derselbe Rand von beiden Seiten abgelaufen (Loch der einen, Umriss der anderen Flaeche) gibt denselben Schluessel.
std::string schluessel(const std::vector<std::uint32_t>& ecken)
{
    const std::size_t n = ecken.size();
    const std::size_t start = static_cast<std::size_t>(std::min_element(ecken.begin(), ecken.end()) - ecken.begin());
    std::vector<std::uint32_t> vor(n);
    std::vector<std::uint32_t> zurueck(n);
    for (std::size_t i = 0; i < n; ++i)
    {
        vor[i] = ecken[(start + i) % n];
        zurueck[i] = ecken[(start + n - i) % n];
    }
    const std::vector<std::uint32_t>& klein = std::lexicographical_compare(zurueck.begin(), zurueck.end(), vor.begin(), vor.end()) ? zurueck : vor;
    return std::string(reinterpret_cast<const char*>(klein.data()), klein.size() * sizeof(std::uint32_t));
}

}  // namespace

double BildEinstellung::toleranz() const
{
    // Detail 100: 0,75 Punkte (die Treppe der Punkte faellt gerade noch weg); Detail 0: 5 Punkte
    return 0.75 + (100.0 - std::clamp(detail, 0, 100)) / 100.0 * 4.25;
}

QJsonObject BildEinstellung::alsJson() const
{
    static const char* const kTeilungen[] = {"colour", "brightness", "transparency"};
    return QJsonObject{{QStringLiteral("splitBy"), QLatin1String(kTeilungen[static_cast<int>(teilung)])},
                       {QStringLiteral("levels"), stufen},
                       {QStringLiteral("smooth"), glaetten},
                       {QStringLiteral("minSize"), mindestProzent},
                       {QStringLiteral("detail"), detail}};
}

void BildEinstellung::ausJson(const QJsonObject& wurzel)
{
    const QString art = wurzel.value(QStringLiteral("splitBy")).toString();
    if (art == QLatin1String("colour")) teilung = Teilung::Farbe;
    if (art == QLatin1String("brightness")) teilung = Teilung::Helligkeit;
    if (art == QLatin1String("transparency")) teilung = Teilung::Deckkraft;
    stufen = std::clamp(wurzel.value(QStringLiteral("levels")).toInt(stufen), 2, 8);
    glaetten = std::clamp(wurzel.value(QStringLiteral("smooth")).toInt(glaetten), 0, 10);
    mindestProzent = std::clamp(wurzel.value(QStringLiteral("minSize")).toDouble(mindestProzent), 0.0, 5.0);
    detail = std::clamp(wurzel.value(QStringLiteral("detail")).toInt(detail), 0, 100);
}

QImage bildZumErkennen(const QImage& bild)
{
    if (bild.isNull()) return {};
    QImage aus = bild;
    if (aus.width() > kBildKanteMax || aus.height() > kBildKanteMax)
        aus = aus.scaled(kBildKanteMax, kBildKanteMax, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    return aus.convertToFormat(QImage::Format_ARGB32);
}

BildKonturen bildKonturen(const QImage& vorbereitet, const BildEinstellung& e)
{
    BildKonturen ergebnis;
    if (vorbereitet.isNull() || vorbereitet.width() < 1 || vorbereitet.height() < 1) return ergebnis;
    const QImage bild = weich(vorbereitet.convertToFormat(QImage::Format_ARGB32), std::clamp(e.glaetten, 0, 10));
    const int w = bild.width();
    const int h = bild.height();
    ergebnis.groesse = QSize(w, h);
    const std::size_t punkte = static_cast<std::size_t>(w) * static_cast<std::size_t>(h);

    // --- Klassen, Flaechen, zu kleine in ihrem groessten Nachbarn
    std::vector<int> klasse = klassen(bild, e);
    std::vector<int> flaeche;
    std::vector<std::size_t> groesse;
    const double mindest = std::clamp(e.mindestProzent, 0.0, 5.0) / 100.0 * static_cast<double>(punkte);
    int zahl = flaechenVon(klasse, w, h, flaeche, groesse);
    for (int runde = 0; runde < kVerschmelzRunden && zahl > 1; ++runde)
    {
        // je Flaeche ihr groesster Nachbar
        std::vector<int> nachbar(static_cast<std::size_t>(zahl), -1);
        const auto pruefe = [&](std::size_t i, std::size_t j) {
            const int a = flaeche[i];
            const int b = flaeche[j];
            if (a == b) return;
            if (nachbar[static_cast<std::size_t>(a)] < 0 || groesse[static_cast<std::size_t>(b)] > groesse[static_cast<std::size_t>(nachbar[static_cast<std::size_t>(a)])])
                nachbar[static_cast<std::size_t>(a)] = b;
            if (nachbar[static_cast<std::size_t>(b)] < 0 || groesse[static_cast<std::size_t>(a)] > groesse[static_cast<std::size_t>(nachbar[static_cast<std::size_t>(b)])])
                nachbar[static_cast<std::size_t>(b)] = a;
        };
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
            {
                const std::size_t i = static_cast<std::size_t>(y) * static_cast<std::size_t>(w) + static_cast<std::size_t>(x);
                if (x + 1 < w) pruefe(i, i + 1);
                if (y + 1 < h) pruefe(i, i + static_cast<std::size_t>(w));
            }
        bool geaendert = false;
        std::vector<int> neueKlasse(static_cast<std::size_t>(zahl), -1);
        for (int f = 0; f < zahl; ++f)
            if (static_cast<double>(groesse[static_cast<std::size_t>(f)]) < mindest && nachbar[static_cast<std::size_t>(f)] >= 0)
            {
                // die Klasse des Nachbarn, wie er vor dieser Runde war
                neueKlasse[static_cast<std::size_t>(f)] = -2 - nachbar[static_cast<std::size_t>(f)];
                geaendert = true;
            }
        if (!geaendert) break;
        std::vector<int> klasseVon(static_cast<std::size_t>(zahl), 0);
        for (std::size_t i = 0; i < punkte; ++i) klasseVon[static_cast<std::size_t>(flaeche[i])] = klasse[i];
        for (std::size_t i = 0; i < punkte; ++i)
            if (const int n = neueKlasse[static_cast<std::size_t>(flaeche[i])]; n <= -2) klasse[i] = klasseVon[static_cast<std::size_t>(-2 - n)];
        zahl = flaechenVon(klasse, w, h, flaeche, groesse);
    }

    // --- die Kanten der Punkte, an denen eine Flaeche endet; jede so gerichtet, dass die Flaeche links liegt
    struct Kante
    {
        std::uint32_t von = 0;
        std::uint32_t nach = 0;
        int flaeche = 0;
    };
    const auto ecke = [w](int x, int y) { return static_cast<std::uint32_t>(y) * static_cast<std::uint32_t>(w + 1) + static_cast<std::uint32_t>(x); };
    std::vector<Kante> kanten;
    const auto anders = [&](int x, int y, int f) {
        return x < 0 || y < 0 || x >= w || y >= h ||
               flaeche[static_cast<std::size_t>(y) * static_cast<std::size_t>(w) + static_cast<std::size_t>(x)] != f;
    };
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
        {
            const int f = flaeche[static_cast<std::size_t>(y) * static_cast<std::size_t>(w) + static_cast<std::size_t>(x)];
            if (anders(x, y - 1, f)) kanten.push_back({ecke(x + 1, y), ecke(x, y), f});
            if (anders(x, y + 1, f)) kanten.push_back({ecke(x, y + 1), ecke(x + 1, y + 1), f});
            if (anders(x - 1, y, f)) kanten.push_back({ecke(x, y), ecke(x, y + 1), f});
            if (anders(x + 1, y, f)) kanten.push_back({ecke(x + 1, y + 1), ecke(x + 1, y), f});
        }
    // nach (Flaeche, Anfang) sortiert: die Kanten, die an einer Ecke weitergehen, liegen beieinander
    const auto schluesselVon = [](const Kante& k) { return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(k.flaeche)) << 32) | k.von; };
    std::vector<std::size_t> ordnung(kanten.size());
    std::iota(ordnung.begin(), ordnung.end(), std::size_t{0});
    std::sort(ordnung.begin(), ordnung.end(), [&](std::size_t a, std::size_t b) { return schluesselVon(kanten[a]) < schluesselVon(kanten[b]); });
    std::vector<std::uint64_t> sortiert(ordnung.size());
    for (std::size_t i = 0; i < ordnung.size(); ++i) sortiert[i] = schluesselVon(kanten[ordnung[i]]);
    std::vector<bool> benutzt(kanten.size(), false);
    const auto ort = [w](std::uint32_t e) { return QPointF(e % static_cast<std::uint32_t>(w + 1), e / static_cast<std::uint32_t>(w + 1)); };

    std::vector<BildKontur> alle;
    std::unordered_set<std::string> gesehen;
    for (std::size_t erste = 0; erste < kanten.size(); ++erste)
    {
        if (benutzt[erste]) continue;
        const int f = kanten[erste].flaeche;
        std::vector<std::uint32_t> ecken;
        std::size_t jetzt = erste;
        benutzt[jetzt] = true;
        const std::uint32_t anfang = kanten[jetzt].von;
        ecken.push_back(anfang);
        // ablaufen; an einer Ecke mit zwei Wegen (zwei Punkte, die sich nur schraeg beruehren) erst links, dann
        // geradeaus, dann rechts — so bleiben Punkte, die sich nur schraeg beruehren, getrennt (vier Nachbarn)
        for (std::size_t schritte = 0; schritte < kanten.size() && kanten[jetzt].nach != anfang; ++schritte)
        {
            const QPointF a = ort(kanten[jetzt].von);
            const QPointF b = ort(kanten[jetzt].nach);
            const QPointF richtung = b - a;
            const QPointF links(richtung.y(), -richtung.x());
            const std::uint64_t such = (static_cast<std::uint64_t>(static_cast<std::uint32_t>(f)) << 32) | kanten[jetzt].nach;
            auto it = std::lower_bound(sortiert.begin(), sortiert.end(), such);
            std::size_t beste = kanten.size();
            int besterRang = 4;
            for (; it != sortiert.end() && *it == such; ++it)
            {
                const std::size_t k = ordnung[static_cast<std::size_t>(it - sortiert.begin())];
                if (benutzt[k]) continue;
                const QPointF weiter = ort(kanten[k].nach) - ort(kanten[k].von);
                const int rang = weiter == links ? 0 : weiter == richtung ? 1 : 2;
                if (rang < besterRang)
                {
                    besterRang = rang;
                    beste = k;
                }
            }
            if (beste == kanten.size()) break;  // sollte nicht vorkommen: jeder Rand ist geschlossen
            jetzt = beste;
            benutzt[jetzt] = true;
            ecken.push_back(kanten[jetzt].von);
        }
        // nur die Ecken, an denen die Richtung wechselt
        std::vector<std::uint32_t> knicke;
        const std::size_t n = ecken.size();
        for (std::size_t i = 0; i < n; ++i)
        {
            const QPointF vorher = ort(ecken[(i + n - 1) % n]);
            const QPointF hier = ort(ecken[i]);
            const QPointF danach = ort(ecken[(i + 1) % n]);
            const QPointF r1 = hier - vorher;
            const QPointF r2 = danach - hier;
            if (r1.x() * r2.y() - r1.y() * r2.x() != 0.0) knicke.push_back(ecken[i]);
        }
        if (knicke.size() < 3) continue;
        if (!gesehen.insert(schluessel(knicke)).second) continue;  // denselben Rand gibt es schon (von der anderen Seite)

        Zug zug;
        bool amRand = false;
        for (const std::uint32_t k : knicke)
        {
            const QPointF p = ort(k);
            zug.push_back(p);
            amRand = amRand || p.x() <= 0.0 || p.y() <= 0.0 || p.x() >= w || p.y() >= h;
        }
        BildKontur kontur;
        kontur.flaeche = flaecheVon(zug);
        if (kontur.flaeche < 1.0) continue;
        kontur.rand = amRand;
        const Zug glatt = vereinfache(zug, e.toleranz());
        kontur.pfad.moveTo(glatt.front());
        for (std::size_t i = 1; i < glatt.size(); ++i) kontur.pfad.lineTo(glatt[i]);
        kontur.pfad.closeSubpath();
        alle.push_back(std::move(kontur));
    }

    // die groessten zuerst; ueber kKonturenMax hinaus fallen die kleinsten weg (K2 a)
    std::stable_sort(alle.begin(), alle.end(), [](const BildKontur& a, const BildKontur& b) { return a.flaeche > b.flaeche; });
    if (alle.size() > static_cast<std::size_t>(kKonturenMax))
    {
        ergebnis.weggefallen = static_cast<int>(alle.size()) - kKonturenMax;
        alle.resize(static_cast<std::size_t>(kKonturenMax));
    }
    // anfangs gewaehlt: die groesste, die nicht an den Rand stoesst (K2 a)
    for (BildKontur& k : alle)
        if (!k.rand)
        {
            k.gewaehlt = true;
            break;
        }
    ergebnis.konturen = std::move(alle);
    return ergebnis;
}

}  // namespace lumi::shape
