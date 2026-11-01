/**
 ****************************************************************************************
 * @file   test_LoopDehner.cpp
 * @brief  Loop, Stufe 15f: Dauer und Tonhoehe hinter `lumi::loop::Dehner`
 *         (docs/audio/Loop_Konzept.md §4p, Entscheid Patrik WV6 a)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Qt-Anwendung. Jeder Fall laeuft gegen jede Umsetzung aus
 * `dehnerUmsetzungen()` (Memory „Externals austauschbar einbinden“). Gemessen wird am
 * gerechneten Ton: die Frequenz ueber Nulldurchgaenge in der Mitte (die Raender tragen den
 * Ein- und Ausschwung des Verfahrens), der Pegel als Effektivwert.
 *
 * - Tonhoehe: +12, −12, +7 und 0 Halbtoene an einem Sinus, die Laenge bleibt — nur fuer
 *   Umsetzungen ohne Formanterhalt (mit ihm folgt der Pegel der Huelle, ein Sinus bleibt kein
 *   Sinus der neuen Hoehe); an einem Klang mit Obertoenen fuer jede (Stufe 15h), gemessen mit
 *   pYIN.
 * - Dauer: doppelt und halb so lang, die Tonhoehe bleibt.
 * - Eine Kurve je Block: erst 0, dann +7 Halbtoene, am Klang mit Obertoenen.
 * - Zweimal gerechnet ist Bit fuer Bit gleich; kurze und leere Klaenge.
 * - Bezugswerte `werte/dehner_bezug.txt` (die Vorgabe) und `werte/dehner_bezug_<name>.txt` (jede
 *   weitere Umsetzung, Stufe 15h) — beide Compiler pruefen gegen dieselbe Datei; neu mit
 *   `LUMIVIZ_DEHNER_BEZUG_SCHREIBEN=1` (schreibt alle).
 * - Rechenzeit (meldet nur Zahlen).
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Dehner.hpp"
#include "loop/PYin.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QTextStream>

#include <chrono>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <memory>
#include <numbers>
#include <vector>

using namespace lumi::loop;

namespace
{

constexpr int kRate = 48000;

/// Ein Sinus in beiden Kanaelen.
std::vector<float> sinus(double hz, std::int64_t frames, double pegel = 0.5)
{
    std::vector<float> d(static_cast<std::size_t>(2 * frames));
    for (std::int64_t i = 0; i < frames; ++i)
    {
        const auto w = static_cast<float>(pegel * std::sin(2.0 * std::numbers::pi * hz * static_cast<double>(i) / kRate));
        d[static_cast<std::size_t>(2 * i)] = w;
        d[static_cast<std::size_t>(2 * i + 1)] = w;
    }
    return d;
}

/// Die Frequenz links zwischen den Abtastwerten `von` und `bis`: aufsteigende Nulldurchgaenge,
/// linear zwischen den Abtastwerten verortet.
double frequenz(const std::vector<float>& d, std::int64_t von, std::int64_t bis)
{
    double erster = -1.0;
    double letzter = -1.0;
    int durchgaenge = 0;
    for (std::int64_t i = von + 1; i < bis; ++i)
    {
        const double a = d[static_cast<std::size_t>(2 * (i - 1))];
        const double b = d[static_cast<std::size_t>(2 * i)];
        if (a < 0.0 && b >= 0.0)
        {
            const double t = static_cast<double>(i - 1) + a / (a - b);
            if (erster < 0.0) erster = t;
            letzter = t;
            ++durchgaenge;
        }
    }
    if (durchgaenge < 2) return 0.0;
    return (durchgaenge - 1) * kRate / (letzter - erster);
}

/// Der Effektivwert links zwischen `von` und `bis`.
double effektiv(const std::vector<float>& d, std::int64_t von, std::int64_t bis)
{
    double summe = 0.0;
    for (std::int64_t i = von; i < bis; ++i) summe += static_cast<double>(d[static_cast<std::size_t>(2 * i)]) * d[static_cast<std::size_t>(2 * i)];
    return std::sqrt(summe / static_cast<double>(bis - von));
}

const auto konstant = [](double ht) { return [ht](std::int64_t) { return ht; }; };

/// Ein Klang mit Obertoenen (Stufe 15h): alle Teiltoene unter 6 kHz mit 1 / k, in beiden Kanaelen.
std::vector<float> obertoene(double hz, std::int64_t frames, double pegel = 0.2)
{
    std::vector<float> d(static_cast<std::size_t>(2 * frames));
    for (std::int64_t i = 0; i < frames; ++i)
    {
        double w = 0.0;
        for (int k = 1; k * hz < 6000.0; ++k)
            w += std::sin(2.0 * std::numbers::pi * k * hz * static_cast<double>(i) / kRate) / k;
        d[static_cast<std::size_t>(2 * i)] = static_cast<float>(pegel * w);
        d[static_cast<std::size_t>(2 * i + 1)] = static_cast<float>(pegel * w);
    }
    return d;
}

/// Der Grundton zwischen `von` und `bis` mit pYIN: der Median der gesungenen Rahmen.
double grundton(const std::vector<float>& d, std::int64_t von, std::int64_t bis)
{
    const std::vector<float> stueck(d.begin() + static_cast<std::ptrdiff_t>(2 * von), d.begin() + static_cast<std::ptrdiff_t>(2 * bis));
    std::vector<float> hz;
    for (const float f : messeTonhoehenverlauf(stueck, kRate).hz)
        if (f > 0.0f) hz.push_back(f);
    if (hz.empty()) return 0.0;
    std::nth_element(hz.begin(), hz.begin() + static_cast<std::ptrdiff_t>(hz.size() / 2), hz.end());
    return hz[hz.size() / 2];
}

/// Zahlen in [−1, 1), auf jedem Rechner dieselben (SplitMix64).
std::vector<float> rauschen(std::int64_t frames, std::uint64_t samen)
{
    std::vector<float> d(static_cast<std::size_t>(2 * frames));
    for (float& w : d)
    {
        samen += 0x9E3779B97F4A7C15ull;
        std::uint64_t z = samen;
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
        z ^= z >> 31;
        w = static_cast<float>(static_cast<double>(z >> 11) / 9007199254740992.0 * 2.0 - 1.0);
    }
    return d;
}

/// Die Bezugsdatei einer Umsetzung: die Vorgabe `dehner_bezug.txt`, jede weitere
/// `dehner_bezug_<name>.txt` (Kleinbuchstaben und Ziffern, sonst `-`).
QString bezugsDatei(const Dehner& u, bool vorgabe)
{
    QString datei = QStringLiteral("werte/dehner_bezug.txt");
    if (!vorgabe)
    {
        QString kurz;
        for (const QChar c : QString::fromStdString(u.name()).toLower())
            kurz += c.isLetterOrNumber() ? c : QLatin1Char('-');
        while (kurz.contains(QStringLiteral("--"))) kurz.replace(QStringLiteral("--"), QStringLiteral("-"));
        datei = QStringLiteral("werte/dehner_bezug_") + kurz + QStringLiteral(".txt");
    }
    return QFileInfo(QString::fromLocal8Bit(__FILE__)).absoluteDir().filePath(datei);
}

}  // namespace

TEST_SUITE("LoopDehner")
{
    TEST_CASE("Loop Dehner 15f Tonhoehe: der Sinus steigt und faellt, die Laenge bleibt")
    {
        const std::int64_t n = 2 * kRate;
        const std::vector<float> ein = sinus(440.0, n);
        const double pegelEin = effektiv(ein, kRate / 2, 3 * kRate / 2);
        for (const auto& u : dehnerUmsetzungen())
        {
            if (u->behaeltFormanten()) continue;  // dafuer der Fall mit Obertoenen
            CAPTURE(u->name());
            for (const double ht : {12.0, -12.0, 7.0, 0.0})
            {
                CAPTURE(ht);
                const std::vector<float> aus = u->rechne(ein, kRate, n, konstant(ht));
                REQUIRE(aus.size() == ein.size());
                CHECK(frequenz(aus, kRate / 2, 3 * kRate / 2) == doctest::Approx(440.0 * std::pow(2.0, ht / 12.0)).epsilon(0.005));
                // der Pegel bleibt in ±1,5 dB
                const double verhaeltnis = effektiv(aus, kRate / 2, 3 * kRate / 2) / pegelEin;
                CHECK(verhaeltnis > 0.84);
                CHECK(verhaeltnis < 1.19);
            }
        }
    }

    TEST_CASE("Loop Dehner 15h Tonhoehe: ein Klang mit Obertoenen steigt und faellt, jede Umsetzung")
    {
        const std::int64_t n = 2 * kRate;
        const std::vector<float> ein = obertoene(220.0, n);
        for (const auto& u : dehnerUmsetzungen())
        {
            CAPTURE(u->name());
            for (const double ht : {12.0, -12.0, 7.0, 0.0})
            {
                CAPTURE(ht);
                const std::vector<float> aus = u->rechne(ein, kRate, n, konstant(ht));
                REQUIRE(aus.size() == ein.size());
                CHECK(grundton(aus, kRate / 2, 3 * kRate / 2) == doctest::Approx(220.0 * std::pow(2.0, ht / 12.0)).epsilon(0.005));
            }
        }
    }

    TEST_CASE("Loop Dehner 15f Dauer: doppelt und halb so lang, die Tonhoehe bleibt")
    {
        const std::int64_t n = 2 * kRate;
        const std::vector<float> ein = sinus(440.0, n);
        for (const auto& u : dehnerUmsetzungen())
        {
            CAPTURE(u->name());
            const std::vector<float> lang = u->rechne(ein, kRate, 2 * n, {});
            REQUIRE(lang.size() == 2 * ein.size());
            CHECK(frequenz(lang, kRate, 3 * kRate) == doctest::Approx(440.0).epsilon(0.005));
            const std::vector<float> kurz = u->rechne(ein, kRate, n / 2, {});
            REQUIRE(kurz.size() == ein.size() / 2);
            CHECK(frequenz(kurz, kRate / 4, 3 * kRate / 4) == doctest::Approx(440.0).epsilon(0.005));
        }
    }

    TEST_CASE("Loop Dehner 15f Kurve: die Tonhoehe je Block, erst 0, dann +7 Halbtoene")
    {
        // seit 15h am Klang mit Obertoenen, gemessen mit pYIN — so gilt der Fall fuer jede Umsetzung
        const std::int64_t n = 2 * kRate;
        const std::vector<float> ein = obertoene(220.0, n);
        for (const auto& u : dehnerUmsetzungen())
        {
            CAPTURE(u->name());
            std::vector<std::int64_t> gefragt;
            const std::vector<float> aus = u->rechne(ein, kRate, n, [&gefragt](std::int64_t k) {
                gefragt.push_back(k);
                return k < kRate ? 0.0 : 7.0;
            });
            CHECK(grundton(aus, kRate / 5, 4 * kRate / 5) == doctest::Approx(220.0).epsilon(0.005));
            CHECK(grundton(aus, 6 * kRate / 5, 9 * kRate / 5) == doctest::Approx(220.0 * std::pow(2.0, 7.0 / 12.0)).epsilon(0.005));
            // je Block einmal gefragt, in der Mitte des Blocks
            REQUIRE(!gefragt.empty());
            CHECK(gefragt.front() == kDehnerBlock / 2);
            CHECK(gefragt.size() == static_cast<std::size_t>((n + kDehnerBlock - 1) / kDehnerBlock));
        }
    }

    TEST_CASE("Loop Dehner 15f gleich: zweimal Bit fuer Bit, kurze und leere Klaenge")
    {
        const std::vector<float> ein = rauschen(kRate / 2, 7);
        for (const auto& u : dehnerUmsetzungen())
        {
            CAPTURE(u->name());
            const auto kurve = [](std::int64_t k) { return 3.0 * std::sin(static_cast<double>(k) / 4000.0); };
            CHECK(u->rechne(ein, kRate, kRate / 2, kurve) == u->rechne(ein, kRate, kRate / 2, kurve));
            // kuerzer als die Latenz des Verfahrens, und leer
            const std::vector<float> wenig = sinus(1000.0, 300);
            CHECK(u->rechne(wenig, kRate, 300, konstant(5.0)).size() == 600);
            CHECK(u->rechne({}, kRate, 100, {}) == std::vector<float>(200, 0.0f));
            CHECK(u->rechne(wenig, kRate, 0, {}).empty());
        }
    }

    TEST_CASE("Loop Dehner 15f Bezugswerte: beide Compiler rechnen gleich")
    {
        // zwei Toene und ein wenig Rauschen, eine Sekunde, die Tonhoehe langsam bis +5
        std::vector<float> ein = sinus(220.0, kRate, 0.3);
        const std::vector<float> zweiter = sinus(330.0, kRate, 0.2);
        const std::vector<float> rest = rauschen(kRate, 11);
        for (std::size_t i = 0; i < ein.size(); ++i) ein[i] += zweiter[i] + 0.02f * rest[i];
        const auto kurve = [](std::int64_t k) { return 5.0 * static_cast<double>(k) / kRate; };
        const std::vector<std::unique_ptr<Dehner>> alle = dehnerUmsetzungen();
        for (std::size_t nr = 0; nr < alle.size(); ++nr)
        {
            const Dehner& u = *alle[nr];
            CAPTURE(u.name());
            const QString pfad = bezugsDatei(u, nr == 0);
            const std::vector<float> aus = u.rechne(ein, kRate, kRate * 5 / 4, kurve);
            REQUIRE(aus.size() == static_cast<std::size_t>(2 * kRate * 5 / 4));
            // der Effektivwert je Viertel und zehn einzelne Abtastwerte, links und rechts
            std::vector<double> werte;
            for (int v = 0; v < 5; ++v) werte.push_back(effektiv(aus, v * kRate / 4, (v + 1) * kRate / 4));
            for (int j = 1; j <= 10; ++j)
            {
                const auto i = static_cast<std::size_t>(2 * (j * 5800));
                werte.push_back(aus[i]);
                werte.push_back(aus[i + 1]);
            }
            if (qEnvironmentVariableIntValue("LUMIVIZ_DEHNER_BEZUG_SCHREIBEN") == 1)
            {
                QFile datei(pfad);
                REQUIRE(datei.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text));
                QTextStream schreib(&datei);
                schreib.setRealNumberPrecision(12);
                schreib << "# " << QString::fromStdString(u.name())
                        << ": 220 + 330 Hz und Rauschen, 1 s auf 1,25 s, Tonhoehe 0 bis +5 Halbtoene - geschrieben von "
                           "test_LoopDehner.cpp (LUMIVIZ_DEHNER_BEZUG_SCHREIBEN=1)\n";
                schreib << "# fuenf Effektivwerte je Viertelsekunde, dann zehnmal links und rechts an 5800 j\n";
                for (const double w : werte) schreib << w << '\n';
                MESSAGE(QFileInfo(pfad).fileName().toStdString() << " neu geschrieben");
                continue;
            }
            QFile datei(pfad);
            REQUIRE_MESSAGE(datei.open(QIODevice::ReadOnly | QIODevice::Text),
                            QFileInfo(pfad).fileName().toStdString() << " fehlt — mit LUMIVIZ_DEHNER_BEZUG_SCHREIBEN=1 schreiben");
            std::vector<double> bezug;
            while (!datei.atEnd())
            {
                const QString zeile = QString::fromUtf8(datei.readLine()).trimmed();
                if (zeile.isEmpty() || zeile.startsWith(QLatin1Char('#'))) continue;
                bezug.push_back(zeile.toDouble());
            }
            REQUIRE(bezug.size() == werte.size());
            for (std::size_t i = 0; i < werte.size(); ++i)
            {
                CAPTURE(i);
                CHECK(werte[i] == doctest::Approx(bezug[i]).epsilon(1e-5).scale(0.3));
            }
        }
    }

    TEST_CASE("Loop Dehner 15f Rechenzeit (meldet nur Zahlen)")
    {
        const std::vector<float> ein = sinus(440.0, 10 * kRate);
        for (const auto& u : dehnerUmsetzungen())
        {
            const auto anfang = std::chrono::steady_clock::now();
            const std::vector<float> aus = u->rechne(ein, kRate, 10 * kRate, konstant(3.0));
            const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - anfang).count();
            CHECK(aus.size() == ein.size());
            MESSAGE(u->name() << ": 10 s Stereo, +3 Halbtoene, in " << ms << " ms (" << ms / 100.0 << " % der Echtzeit)");
        }
    }
}
