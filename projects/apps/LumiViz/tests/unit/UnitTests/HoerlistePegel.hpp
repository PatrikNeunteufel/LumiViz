/**
 ****************************************************************************************
 * @file   HoerlistePegel.hpp
 * @brief  Tests: die Klaenge einer Hoerliste auf einen gemeinsamen Pegel (S186, PN1 a, PN2 a, KZ1 a)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Gemeinsam fuer die Schreiber der Hoerlisten (`test_LoopStab`, `test_LoopKlasse1`, `test_LoopTrommel`):
 * - **KZ1 a:** ein Klang unter 0,4 s wird hinten mit Stille auf 0,4 s aufgefuellt — BS.1770-4 misst ihn dann wie jeden.
 * - **PN2 a:** ein stiller Klang (alle Bloecke unter dem absoluten Gate, −70 LUFS) kommt nicht in die Liste; er wird
 *   gemeldet (Befund am Modell). `ohneStille` nimmt ihn mit seinem Messwert und den Punkten, die ihn nennen, heraus.
 * - **PN1 a:** jeder Klang wird **nachgeregelt** (verstaerken, neu messen, bis 0,05 LU) — die integrierte Lautheit leiser
 *   Klaenge haengt am absoluten Gate und damit am Massstab, ein Faktor in einem Schritt trifft nicht (IC lag so bis 8,8 LU
 *   daneben, `docs/modellierung/anbindung-ic/2026-11-10_S186_lv5_pegel_der_hoerlisten.md`). Kommt danach eine Spitze ueber
 *   die Grenze, sinkt der gemeinsame Pegel um den Ueberschuss (S179: die Bassdrum pp).
 * - **LV6-3 a:** ein Klang ohne Ton (nach den ersten 20 ms weniger als −80 dB seiner Energie — nur ein Puls) kommt nicht
 *   in die Liste; gemeldet als Befund am Modell (`bereinige`).
 * - **LV6-1 a:** Klaenge, die nach dem Angleichen gleich klingen (Rest der Differenz unter −60 dB), kommen nur einmal in
 *   die Liste: je Gruppe gleicher Staerken die mittlere, deren Frage die Staerke nicht mehr nennt; je Gruppe gleicher
 *   Instrumente der erste (`bereinige`; Befund `docs/modellierung/anbindung-ic/2026-11-10_S186_lv6_gleiche_klaenge.md`).
 *   Die Kennungen der uebrigen Punkte bleiben (Luecken), ein Paar, das einen herausgenommenen Klang nennt, faellt mit.
 *
 * Messung: `lumi::audio::messeLautheit` (Stereo, verschraenkt).
 ****************************************************************************************
 */
#pragma once

#include <doctest.h>

#include "AudioCore/Lautheit.hpp"
#include "InstrumentCore/Pruefsumme.hpp"
#include "loop/Aufnahme.hpp"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QStringList>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <map>
#include <span>
#include <vector>

namespace hoerliste
{

/// Ein gerechneter Klang der Liste: Stereo, verschraenkt.
struct Klang
{
    QString id;
    QString datei;  ///< relativ zur Liste, `klaenge/<id>.wav`
    std::vector<float> daten;
};

/// Was `gleicheAn` getan hat.
struct Pegel
{
    double ziel = 0.0;       ///< der gemeinsame Pegel in LUFS
    QStringList stille;      ///< herausgenommen (PN2 a)
    QStringList aufgefuellt; ///< auf 0,4 s aufgefuellt (KZ1 a)
};

inline float spitzeVon(const std::vector<float>& daten)
{
    float s = 0.0f;
    for (const float x : daten) s = std::max(s, std::abs(x));
    return s;
}

/// KZ1 a, PN2 a, PN1 a — nimmt die stillen Klaenge aus `klaenge` heraus und regelt die uebrigen auf einen gemeinsamen
/// Pegel: hoechstens `zielHoechstens`, so tief, dass keine Spitze ueber `spitzeHoechstens` kommt.
inline Pegel gleicheAn(std::vector<Klang>& klaenge, int rate, double zielHoechstens, double spitzeHoechstens)
{
    Pegel p;
    const std::size_t kurz = static_cast<std::size_t>(std::lround(0.4 * rate)) * 2;  // Stereo
    std::vector<Klang> laut;
    std::vector<double> vorher;
    for (Klang& k : klaenge)
    {
        if (k.daten.size() < kurz)
        {
            k.daten.resize(kurz, 0.0f);
            p.aufgefuellt << k.id;
        }
        const lumi::audio::Lautheit l = lumi::audio::messeLautheit(k.daten, rate);
        if (l.stille)
        {
            p.stille << k.id;
            continue;
        }
        vorher.push_back(l.lufs);
        laut.push_back(std::move(k));
    }
    double ziel = zielHoechstens;
    for (std::size_t i = 0; i < laut.size(); ++i)
    {
        const float s = spitzeVon(laut[i].daten);
        if (s > 0.0f) ziel = std::min(ziel, vorher[i] + 20.0 * std::log10(spitzeHoechstens / s));
    }
    ziel = std::floor(ziel * 10.0) / 10.0;
    const std::vector<Klang> roh = laut;
    for (int versuch = 0; versuch < 6; ++versuch)
    {
        laut = roh;
        float hoechste = 0.0f;
        for (std::size_t i = 0; i < laut.size(); ++i)
        {
            double ist = vorher[i];
            for (int runde = 0; runde < 8 && std::abs(ist - ziel) > 0.05; ++runde)
            {
                const double faktor = std::pow(10.0, (ziel - ist) / 20.0);
                for (float& x : laut[i].daten) x = static_cast<float>(x * faktor);
                ist = lumi::audio::messeLautheit(laut[i].daten, rate).lufs;
            }
            CHECK_MESSAGE(std::abs(ist - ziel) <= 0.05, laut[i].datei.toStdString() << ": " << ist << " statt " << ziel << " LUFS");
            hoechste = std::max(hoechste, spitzeVon(laut[i].daten));
        }
        if (hoechste <= spitzeHoechstens) break;
        ziel = std::floor((ziel - 20.0 * std::log10(hoechste / spitzeHoechstens)) * 10.0) / 10.0;
    }
    for (const Klang& k : laut)
        CHECK_MESSAGE(spitzeVon(k.daten) <= spitzeHoechstens, k.datei.toStdString() << ": Spitze " << spitzeVon(k.daten));
    klaenge = std::move(laut);
    p.ziel = ziel;
    return p;
}

/// PN2 a, LV6-1 a, LV6-3 a: Klaenge aus der Liste — ihr Eintrag, ihr Messwert (gleiche Achsen und Fassung) und jeder
/// Punkt, der sie nennt (einzel ueber `klang`, paar ueber `klaenge`).
inline void ohneKlaenge(QJsonArray& klaenge, QJsonArray& messwerte, QJsonArray& punkte, const QStringList& stille)
{
    if (stille.isEmpty()) return;
    QJsonArray k2, m2, p2;
    std::vector<std::pair<QJsonValue, QJsonValue>> weg;
    for (const QJsonValue& k : klaenge)
    {
        const QJsonObject o = k.toObject();
        if (stille.contains(o.value(QStringLiteral("id")).toString()))
            weg.emplace_back(o.value(QStringLiteral("achsen")), o.value(QStringLiteral("fassung")));
        else
            k2.append(k);
    }
    for (const QJsonValue& m : messwerte)
    {
        const QJsonObject o = m.toObject();
        const bool still = std::any_of(weg.begin(), weg.end(), [&](const auto& w) {
            return o.value(QStringLiteral("achsen")) == w.first && o.value(QStringLiteral("fassung")) == w.second;
        });
        if (!still) m2.append(m);
    }
    for (const QJsonValue& pt : punkte)
    {
        const QJsonObject o = pt.toObject();
        bool nennt = stille.contains(o.value(QStringLiteral("klang")).toString());
        for (const QJsonValue& id : o.value(QStringLiteral("klaenge")).toArray()) nennt = nennt || stille.contains(id.toString());
        if (!nennt) p2.append(pt);
    }
    klaenge = k2;
    messwerte = m2;
    punkte = p2;
}

/// Was `bereinige` herausgenommen hat (LV6-1 a, LV6-3 a).
struct Bereinigt
{
    QStringList ohneTon;    ///< nur ein Puls, danach still (LV6-3 a)
    QStringList gleich;     ///< klingt nach dem Angleichen wie ein anderer Klang der Liste (LV6-1 a)
    QStringList vertreter;  ///< bleibt fuer eine Gruppe gleicher Staerken; seine Frage nennt die Staerke nicht mehr
};

inline constexpr double kGleichDb = -60.0;    ///< LV6-1 a: Rest der Differenz nach dem Angleichen
inline constexpr double kOhneTonDb = -80.0;   ///< LV6-3 a: Energie nach `kOhneTonNachS` gegen die ganze
inline constexpr double kOhneTonNachS = 0.02;

/// LV6-3 a: nach den ersten 20 ms weniger als −80 dB der Energie des Klangs (Stereo, verschraenkt).
inline bool ohneTon(const std::vector<float>& daten, int rate)
{
    const std::size_t ab = static_cast<std::size_t>(std::lround(kOhneTonNachS * rate)) * 2;
    double gesamt = 0.0, danach = 0.0;
    for (std::size_t i = 0; i < daten.size(); ++i)
    {
        const double e = static_cast<double>(daten[i]) * daten[i];
        gesamt += e;
        if (i >= ab) danach += e;
    }
    return gesamt > 0.0 && danach < gesamt * std::pow(10.0, kOhneTonDb / 10.0);
}

/// Der Rest der Differenz zweier Klaenge, beide auf gleiche Energie gebracht, in dB (0 dB, wenn einer still ist).
inline double restDb(const std::vector<float>& a, const std::vector<float>& b)
{
    const std::size_t n = std::min(a.size(), b.size());
    double ea = 0.0, eb = 0.0;
    for (std::size_t i = 0; i < n; ++i)
    {
        ea += static_cast<double>(a[i]) * a[i];
        eb += static_cast<double>(b[i]) * b[i];
    }
    if (n == 0 || ea <= 0.0 || eb <= 0.0) return 0.0;
    const double fa = 1.0 / std::sqrt(ea / static_cast<double>(n)), fb = 1.0 / std::sqrt(eb / static_cast<double>(n));
    double d = 0.0;
    for (std::size_t i = 0; i < n; ++i)
    {
        const double x = a[i] * fa - b[i] * fb;
        d += x * x;
    }
    return 10.0 * std::log10(d / static_cast<double>(n) + 1e-300);
}

/// Die Zahl einer Staerke: `a-30` → −30, `e50` → 50.
inline int staerkeWert(const QString& s)
{
    qsizetype i = 0;
    while (i < s.size() && !s[i].isDigit() && s[i] != QLatin1Char('-')) ++i;
    return s.mid(i).toInt();
}

/// LV6-3 a, LV6-1 a — vor `gleicheAn`: nimmt die Klaenge ohne Ton und die gleich klingenden aus `gerechnet` und aus der
/// Liste (Eintrag, Messwert, Punkte); verglichen werden zwei Klaenge gleicher Fassung, deren Achsen sich nur in der
/// Staerke oder nur im Instrument unterscheiden. Die Frage eines Vertreters gleicher Staerken verliert den Satz zur Staerke.
inline Bereinigt bereinige(QJsonArray& klaenge, QJsonArray& messwerte, QJsonArray& punkte, std::vector<Klang>& gerechnet, int rate)
{
    Bereinigt b;
    std::map<QString, const Klang*> daten;
    for (const Klang& k : gerechnet) daten[k.id] = &k;
    for (const Klang& k : gerechnet)
        if (ohneTon(k.daten, rate)) b.ohneTon << k.id;

    struct Eintrag
    {
        QString id;
        QJsonObject achsen;
        QJsonValue fassung;
    };
    std::vector<Eintrag> e;
    for (const QJsonValue& v : klaenge)
    {
        const QJsonObject o = v.toObject();
        const QString id = o.value(QStringLiteral("id")).toString();
        if (!b.ohneTon.contains(id) && daten.count(id) != 0)
            e.push_back({id, o.value(QStringLiteral("achsen")).toObject(), o.value(QStringLiteral("fassung"))});
    }
    std::vector<std::size_t> wurzel(e.size());
    for (std::size_t i = 0; i < e.size(); ++i) wurzel[i] = i;
    const auto finde = [&](std::size_t i) {
        while (wurzel[i] != i) i = wurzel[i] = wurzel[wurzel[i]];
        return i;
    };
    for (std::size_t i = 0; i < e.size(); ++i)
        for (std::size_t j = i + 1; j < e.size(); ++j)
        {
            if (e[i].fassung != e[j].fassung) continue;
            QStringList anders;
            QStringList schluessel = e[i].achsen.keys() + e[j].achsen.keys();
            schluessel.removeDuplicates();
            for (const QString& s : schluessel)
                if (e[i].achsen.value(s) != e[j].achsen.value(s)) anders << s;
            if (anders != QStringList{QStringLiteral("staerke")} && anders != QStringList{QStringLiteral("instrument")}) continue;
            if (restDb(daten.at(e[i].id)->daten, daten.at(e[j].id)->daten) < kGleichDb) wurzel[finde(j)] = finde(i);
        }
    std::map<std::size_t, std::vector<std::size_t>> gruppen;
    for (std::size_t i = 0; i < e.size(); ++i) gruppen[finde(i)].push_back(i);
    for (const auto& [_, g] : gruppen)
    {
        if (g.size() < 2) continue;
        const QJsonValue instrument = e[g.front()].achsen.value(QStringLiteral("instrument"));
        const bool staerken = std::all_of(g.begin(), g.end(), [&](std::size_t i) { return e[i].achsen.value(QStringLiteral("instrument")) == instrument; });
        std::vector<std::size_t> reihe = g;  // in der Reihenfolge der Liste
        if (staerken)
            std::stable_sort(reihe.begin(), reihe.end(), [&](std::size_t x, std::size_t y) {
                return staerkeWert(e[x].achsen.value(QStringLiteral("staerke")).toString()) <
                       staerkeWert(e[y].achsen.value(QStringLiteral("staerke")).toString());
            });
        const std::size_t bleibt = staerken ? reihe[(reihe.size() - 1) / 2] : reihe.front();
        if (staerken) b.vertreter << e[bleibt].id;
        for (const std::size_t i : reihe)
            if (i != bleibt) b.gleich << e[i].id;
    }

    const QStringList weg = b.ohneTon + b.gleich;
    std::erase_if(gerechnet, [&](const Klang& k) { return weg.contains(k.id); });
    ohneKlaenge(klaenge, messwerte, punkte, weg);
    for (QJsonValueRef pt : punkte)
    {
        QJsonObject o = pt.toObject();
        if (!b.vertreter.contains(o.value(QStringLiteral("klang")).toString())) continue;
        QString frage = o.value(QStringLiteral("frage")).toString();
        frage.remove(QStringLiteral(" Farbe ueber die Staerke glaubhaft?"));
        o[QStringLiteral("frage")] = frage;
        pt = o;
    }
    return b;
}

/// Schreibt die Klaenge als WAV unter `ort` und gibt je Klang die Zeile fuer `klaenge.sha256`. Eine WAV unter
/// `ort/klaenge`, die nicht mehr in der Liste steht (etwa ein still gewordener Klang), wird entfernt.
inline QStringList schreibeKlaenge(const QDir& ort, const std::vector<Klang>& klaenge, int rate)
{
    QStringList dateien;
    for (const Klang& k : klaenge) dateien << QDir::cleanPath(k.datei);
    const QDir unter(ort.filePath(QStringLiteral("klaenge")));
    for (const QString& alt : unter.entryList({QStringLiteral("*.wav")}, QDir::Files))
        if (!dateien.contains(QStringLiteral("klaenge/") + alt)) REQUIRE(QFile::remove(unter.filePath(alt)));
    QStringList summen;
    for (const Klang& k : klaenge)
    {
        REQUIRE(lumi::loop::schreibeWav(ort.filePath(k.datei), k.daten, rate));
        QFile geschrieben(ort.filePath(k.datei));
        REQUIRE(geschrieben.open(QIODevice::ReadOnly));
        const QByteArray bytes = geschrieben.readAll();
        summen << QString::fromStdString(lumi::kern::sha256(std::span<const std::uint8_t>(
                      reinterpret_cast<const std::uint8_t*>(bytes.constData()), static_cast<std::size_t>(bytes.size())))) +
                      QStringLiteral("  ") + k.datei;
    }
    return summen;
}

}  // namespace hoerliste
