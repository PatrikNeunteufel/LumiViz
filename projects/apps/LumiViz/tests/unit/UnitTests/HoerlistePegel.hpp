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

/// PN2 a: die stillen Klaenge aus der Liste — ihr Eintrag, ihr Messwert (gleiche Achsen und Fassung) und jeder Punkt,
/// der sie nennt (einzel ueber `klang`, paar ueber `klaenge`).
inline void ohneStille(QJsonArray& klaenge, QJsonArray& messwerte, QJsonArray& punkte, const QStringList& stille)
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
