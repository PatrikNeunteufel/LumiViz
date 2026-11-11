/**
 ****************************************************************************************
 * @file   test_LoopModelled.cpp
 * @brief  Loop, S182: die Loops der Instrumente mit Kern unter `asset/loop/modelled_instruments/` (Entscheide Patrik
 *         IL1 a, IL2 a, IL3 a; Skript `asset/loop/make_modelled_loops.py`)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Liest vom Arbeitsordner aus — die Test-Exe aus dem Projekt-Root starten.
 *
 * - **Aufbau:** je Instrument mit Kern genau ein Loop der Nuancen `<instrument>.lvloop`, das Ensemble, die Beispiele je
 *   Klasse und `full/nocturne_demo` — alle mitgeliefert (`asset/Asset.cmake`), lesbar ohne Bericht; jede Note im Umfang
 *   aus der Tafel des Instruments.
 * - **Spielweisen (IL2 a):** die Spielweisen der Instrumente stehen als Drum-Spuren mit festem `coreTracks` (TM2 a).
 *   Fuer `bar`, `hammerString` und `struckString` ist dieser Weg sonst ungeprueft — ein Name, den der Kern nicht liest,
 *   aendert nichts (Fall „34h-5 Spielweise im Klang“). Deshalb hier je Spur der Ton mit und ohne ihre Werte, und der
 *   Unterschied muss hoerbar sein: die Energie der Differenz mindestens −40 dB unter der des Tons ohne. Die Zahlen je
 *   Spur stehen in der Ausgabe.
 ****************************************************************************************
 */

#include <doctest.h>

#include "LoopOhneEngine.hpp"
#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <vector>

using namespace lumi::loop;

namespace
{

const QString kOrdner = QStringLiteral("asset/loop/modelled_instruments/");

/// Die Instrumente, die einen Kern rechnen — wie `ALLE` in `make_modelled_loops.py`.
const QStringList kMitKern = {
    QStringLiteral("grand_piano"),         QStringLiteral("upright_piano"),      QStringLiteral("honky_tonk_piano"),
    QStringLiteral("electric_grand_piano"), QStringLiteral("marimba"),           QStringLiteral("vibraphone"),
    QStringLiteral("xylophone"),           QStringLiteral("glockenspiel"),       QStringLiteral("tubular_bells"),
    QStringLiteral("celesta"),             QStringLiteral("kalimba"),            QStringLiteral("tine_electric_piano"),
    QStringLiteral("reed_electric_piano"), QStringLiteral("bass_drum"),          QStringLiteral("snare_drum"),
    QStringLiteral("low_tom"),             QStringLiteral("conga"),              QStringLiteral("bongo"),
    QStringLiteral("tabla"),               QStringLiteral("timpani"),            QStringLiteral("clavinet_modal"),
};

Instrument liesInstrument(const QString& name)
{
    QFile f(QStringLiteral("asset/loop/instruments/") + name + QStringLiteral(".lvinst"));
    Instrument i;
    REQUIRE_MESSAGE(f.open(QIODevice::ReadOnly), name.toStdString());
    REQUIRE(ausJson(QJsonDocument::fromJson(f.readAll()).object(), i));
    i.name = name;
    return i;
}

/// Der Umfang aus der Tafel der Datei (`coreParams` `noteMin`/`noteMax`, KL4 a).
std::pair<int, int> umfangVon(const QString& name)
{
    QFile f(QStringLiteral("asset/loop/instruments/") + name + QStringLiteral(".lvinst"));
    REQUIRE(f.open(QIODevice::ReadOnly));
    const QJsonObject wurzel = QJsonDocument::fromJson(f.readAll()).object();
    for (const QJsonValue& s : wurzel.value(QStringLiteral("layers")).toArray())
    {
        const QJsonObject werte = s.toObject().value(QStringLiteral("coreParams")).toObject();
        if (werte.isEmpty()) continue;
        auto wert = [&](const char* k) {
            const QJsonValue v = werte.value(QLatin1String(k));
            return static_cast<int>(std::lround(v.isObject() ? v.toObject().value(QStringLiteral("value")).toDouble() : v.toDouble()));
        };
        return {wert("noteMin"), wert("noteMax")};
    }
    FAIL("keine Kern-Schicht: " << name.toStdString());
    return {0, 0};
}

/// Energie der Differenz zur Energie von `bezug`, in dB (beide Kanaele, der kuerzere mit Stille verlaengert).
double abstandDb(const Klang& bezug, const Klang& anders)
{
    const std::size_t n = std::max(bezug.daten.size(), anders.daten.size());
    double e = 0.0, d = 0.0;
    for (std::size_t i = 0; i < n; ++i)
    {
        const double a = i < bezug.daten.size() ? bezug.daten[i] : 0.0;
        const double b = i < anders.daten.size() ? anders.daten[i] : 0.0;
        e += a * a;
        d += (a - b) * (a - b);
    }
    if (d <= 0.0) return -1000.0;
    return 10.0 * std::log10(d / std::max(e, 1e-30));
}

}  // namespace

TEST_CASE("Modelled loops S182 Aufbau: je Instrument mit Kern ein Loop der Nuancen, alle mitgeliefert und lesbar")
{
    const QStringList geliefert = looptest::mitgelieferteLoops();
    QStringList hier;
    for (const QString& p : geliefert)
        if (p.contains(QStringLiteral("/modelled_instruments/"))) hier << QFileInfo(p).absoluteFilePath();

    QStringList erwartet;
    for (const QString& name : kMitKern) erwartet << name;
    erwartet << QStringLiteral("modelled_ensemble") << QStringLiteral("grand_piano_demo") << QStringLiteral("bars_demo")
             << QStringLiteral("membrane_demo") << QStringLiteral("keys_demo");
    for (const QString& name : erwartet)
        CHECK_MESSAGE(hier.contains(QFileInfo(kOrdner + name + QStringLiteral(".lvloop")).absoluteFilePath()), name.toStdString());
    CHECK_MESSAGE(hier.contains(QFileInfo(kOrdner + QStringLiteral("full/nocturne_demo.lvloop")).absoluteFilePath()),
                  "full/nocturne_demo");
    CHECK(hier.size() == erwartet.size() + 1);

    std::map<QString, std::pair<int, int>> umfang;
    for (const QString& name : kMitKern) umfang[name] = umfangVon(name);
    for (const QString& pfad : hier)
    {
        CAPTURE(pfad.toStdString());
        Loop loop;
        QStringList bericht;
        REQUIRE(ladeLoop(pfad, loop, &bericht));
        CHECK_MESSAGE(bericht.isEmpty(), bericht.join(QStringLiteral("; ")).toStdString());
        for (const Muster& m : loop.muster)
        {
            const auto u = umfang.find(m.instrument);
            if (m.art != MusterArt::Instrument || u == umfang.end()) continue;
            for (const Note& n : m.noten)
            {
                CHECK_MESSAGE(n.hoehe >= u->second.first, m.name.toStdString() << " " << n.hoehe);
                CHECK_MESSAGE(n.hoehe <= u->second.second, m.name.toStdString() << " " << n.hoehe);
            }
        }
    }
}

TEST_CASE("Modelled loops S182 Spielweisen (IL2 a): jede Drum-Spur mit coreTracks aendert den Ton hoerbar")
{
    std::map<QString, Instrument> instrumente;
    int geprueft = 0;
    std::set<QString> mitSpielweise;
    for (const QString& name : kMitKern)
    {
        const QString pfad = kOrdner + name + QStringLiteral(".lvloop");
        Loop loop;
        REQUIRE_MESSAGE(ladeLoop(pfad, loop), pfad.toStdString());
        for (const Muster& m : loop.muster)
        {
            if (m.art != MusterArt::Drumkit) continue;
            for (const Spur& s : m.spuren)
            {
                if (s.kernSpuren.empty()) continue;
                REQUIRE(s.instrument == name);
                if (!instrumente.count(name)) instrumente.emplace(name, liesInstrument(name));
                const Instrument& inst = instrumente.at(name);
                TonAngabe ohne;
                ohne.instrument = inst.name;
                ohne.hoehe = s.hoehe;
                ohne.dauerMs = s.dauerMs;
                TonAngabe mit = ohne;
                mit.kernSpuren = s.kernSpuren;
                const double db = abstandDb(rendereTon(inst, ohne, 48000), rendereTon(inst, mit, 48000));
                MESSAGE(name.toStdString() << " / " << s.name.toStdString() << ": Differenz " << db << " dB");
                CHECK_MESSAGE(db > -40.0, name.toStdString() << " / " << s.name.toStdString());
                ++geprueft;
                mitSpielweise.insert(name);
            }
        }
    }
    MESSAGE(geprueft << " Spielweisen an " << mitSpielweise.size() << " Instrumenten");
    // S187 (nach HS-3 a): `LUMIVIZ_IL2_PROBE=<instrument>` misst den Daempfer je Taste und Laenge (nur Zahlen)
    if (const QString probe = qEnvironmentVariable("LUMIVIZ_IL2_PROBE"); !probe.isEmpty())
    {
        const Instrument inst = liesInstrument(probe);
        for (const double druck : {0.5, 0.0})
            for (const int hoehe : {60, 72, 84, 96})
                for (const int dauer : {150, 300, 600})
                {
                    TonAngabe ohne;
                    ohne.instrument = inst.name;
                    ohne.hoehe = hoehe;
                    ohne.dauerMs = dauer;
                    TonAngabe mit = ohne;
                    mit.kernSpuren = kernSpurenAusJson(QJsonObject{{QStringLiteral("damperPressure"), druck}});
                    MESSAGE(probe.toStdString() << " damperPressure " << druck << " Taste " << hoehe << " " << dauer << " ms: "
                                                << abstandDb(rendereTon(inst, ohne, 48000), rendereTon(inst, mit, 48000)) << " dB");
                }
    }
    // alle ausser dem CP-70 (er hat nur Sustain), dem Fluegel (una corda mit `softFactor` 1 ohne Wirkung) und der
    // Kalimba (gezupft) — S182; was die Werte aus IC heute abschalten, steht in make_modelled_loops.py
    CHECK(mitSpielweise.size() == 18);
    for (const char* ohne : {"electric_grand_piano", "grand_piano", "kalimba"}) CHECK_MESSAGE(!mitSpielweise.count(QLatin1String(ohne)), ohne);
}
