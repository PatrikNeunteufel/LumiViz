/**
 * @file WavDatei.cpp
 * @brief Eine WAV-Datei schreiben und lesen (Stufe 30b)
 */

#include "audio/WavDatei.hpp"

#include <QByteArray>
#include <QFile>
#include <QSaveFile>
#include <QtEndian>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

namespace lumi::audio
{

namespace
{

constexpr quint16 kKanaele = 2;
constexpr quint16 kFormatPcm = 1;
constexpr quint16 kFormatFloat = 3;
constexpr quint16 kFormatErweitert = 0xFFFE;
/// So viele Frames werden am Stueck umgewandelt und geschrieben.
constexpr std::size_t kBlockFrames = 65536;
/// Der Startwert des Dithers — fest, damit dieselben Werte dieselbe Datei ergeben.
constexpr std::uint32_t kDitherStart = 0x4C56u;

template <typename T>
void haenge(QByteArray& ziel, T wert)
{
    const T klein = qToLittleEndian(wert);
    ziel.append(reinterpret_cast<const char*>(&klein), sizeof(T));
}

int bitsVon(WavTiefe tiefe)
{
    switch (tiefe)
    {
    case WavTiefe::Pcm16: return 16;
    case WavTiefe::Pcm24: return 24;
    case WavTiefe::Gleitkomma32: break;
    }
    return 32;
}

/// Gleichverteilt in [-0,5, 0,5) aus einem kleinen Generator mit eigener Rechnung — eine
/// Verteilung der Standardbibliothek darf je Compiler anders rechnen.
class Wuerfel
{
public:
    double naechster()
    {
        m_zustand = m_zustand * 1664525u + 1013904223u;
        return static_cast<double>(m_zustand >> 8) / static_cast<double>(1u << 24) - 0.5;
    }

private:
    std::uint32_t m_zustand = kDitherStart;
};

template <typename T>
T lies(const char* stelle)
{
    T wert;
    std::memcpy(&wert, stelle, sizeof(T));
    return qFromLittleEndian(wert);
}

}  // namespace

WavErgebnis schreibeWav(const QString& pfad, std::span<const float> stereo, int rate, WavTiefe tiefe, bool dither)
{
    WavErgebnis ergebnis;
    const std::size_t frames = stereo.size() / 2;
    if (frames == 0 || rate <= 0) return ergebnis;
    const int bits = bitsVon(tiefe);
    const quint16 blockBytes = static_cast<quint16>(kKanaele * bits / 8);
    const bool alsFloat = tiefe == WavTiefe::Gleitkomma32;
    const quint32 kopf = alsFloat ? 4 + (8 + 18) + (8 + 4) + 8 : 4 + (8 + 16) + 8;
    const std::uint64_t datenBytes = static_cast<std::uint64_t>(frames) * blockBytes;
    if (datenBytes + kopf > std::numeric_limits<quint32>::max()) return ergebnis;

    QByteArray daten;
    daten.append("RIFF", 4);
    haenge<quint32>(daten, kopf + static_cast<quint32>(datenBytes));
    daten.append("WAVE", 4);
    daten.append("fmt ", 4);
    haenge<quint32>(daten, alsFloat ? 18 : 16);  // Laenge des Abschnitts
    haenge<quint16>(daten, alsFloat ? kFormatFloat : kFormatPcm);
    haenge<quint16>(daten, kKanaele);
    haenge<quint32>(daten, static_cast<quint32>(rate));
    haenge<quint32>(daten, static_cast<quint32>(rate) * blockBytes);  // Bytes je Sekunde
    haenge<quint16>(daten, blockBytes);                               // Bytes je Abtastwert beider Seiten
    haenge<quint16>(daten, static_cast<quint16>(bits));
    if (alsFloat)
    {
        haenge<quint16>(daten, 0);  // keine weiteren Angaben
        // ein Format ausser PCM traegt die Zahl der Frames
        daten.append("fact", 4);
        haenge<quint32>(daten, 4);
        haenge<quint32>(daten, static_cast<quint32>(frames));
    }
    daten.append("data", 4);
    haenge<quint32>(daten, static_cast<quint32>(datenBytes));

    QSaveFile datei(pfad);
    if (!datei.open(QIODevice::WriteOnly) || datei.write(daten) != daten.size()) return ergebnis;

    Wuerfel wuerfel;
    const bool mitDither = dither && tiefe == WavTiefe::Pcm16;
    const double voll = tiefe == WavTiefe::Pcm16 ? 32767.0 : 8388607.0;
    for (std::size_t ab = 0; ab < frames; ab += kBlockFrames)
    {
        const std::size_t n = std::min(kBlockFrames, frames - ab);
        daten.clear();
        daten.reserve(static_cast<qsizetype>(n * blockBytes));
        for (std::size_t i = 2 * ab; i < 2 * (ab + n); ++i)
        {
            const float roh = stereo[i];
            ergebnis.spitze = std::max(ergebnis.spitze, std::abs(roh));
            if (alsFloat)
            {
                haenge<float>(daten, roh);
                continue;
            }
            if (std::abs(roh) > 1.0f) ++ergebnis.gekappt;
            double wert = std::clamp(static_cast<double>(roh), -1.0, 1.0) * voll;
            // TPDF: die Summe zweier gleichverteilter Zahlen, zusammen ±1 LSB
            if (mitDither) wert += wuerfel.naechster() + wuerfel.naechster();
            const auto ganz = static_cast<qint32>(std::clamp<long>(std::lround(wert), static_cast<long>(-voll), static_cast<long>(voll)));
            if (tiefe == WavTiefe::Pcm16)
                haenge<qint16>(daten, static_cast<qint16>(ganz));
            else
            {
                const qint32 klein = qToLittleEndian(ganz);
                daten.append(reinterpret_cast<const char*>(&klein), 3);  // die unteren drei Bytes
            }
        }
        if (datei.write(daten) != daten.size()) return WavErgebnis{};
    }
    ergebnis.geschrieben = datei.commit();
    return ergebnis;
}

std::optional<WavInhalt> liesWav(const QString& pfad)
{
    QFile datei(pfad);
    if (!datei.open(QIODevice::ReadOnly)) return std::nullopt;
    const QByteArray roh = datei.readAll();
    if (roh.size() < 12 || !roh.startsWith("RIFF") || roh.mid(8, 4) != "WAVE") return std::nullopt;
    quint16 format = 0;
    quint16 kanaele = 0;
    quint16 bits = 0;
    WavInhalt inhalt;
    qsizetype stelle = 12;
    while (stelle + 8 <= roh.size())
    {
        const QByteArray kennung = roh.mid(stelle, 4);
        const quint32 groesse = lies<quint32>(roh.constData() + stelle + 4);
        const char* anfang = roh.constData() + stelle + 8;
        const qsizetype verfuegbar = std::min<qsizetype>(groesse, roh.size() - stelle - 8);
        if (kennung == "fmt " && verfuegbar >= 16)
        {
            format = lies<quint16>(anfang);
            kanaele = lies<quint16>(anfang + 2);
            inhalt.rate = static_cast<int>(lies<quint32>(anfang + 4));
            bits = lies<quint16>(anfang + 14);
            if (format == kFormatErweitert && verfuegbar >= 26) format = lies<quint16>(anfang + 24);  // die Unterart
        }
        else if (kennung == "data")
        {
            if (kanaele < 1 || kanaele > 2 || inhalt.rate <= 0) return std::nullopt;
            if (format == kFormatFloat && bits == 32)
                inhalt.tiefe = WavTiefe::Gleitkomma32;
            else if (format == kFormatPcm && bits == 24)
                inhalt.tiefe = WavTiefe::Pcm24;
            else if (format == kFormatPcm && bits == 16)
                inhalt.tiefe = WavTiefe::Pcm16;
            else
                return std::nullopt;
            const int bytes = bits / 8;
            const qsizetype frames = verfuegbar / (bytes * kanaele);
            inhalt.stereo.reserve(static_cast<std::size_t>(2 * frames));
            const auto wert = [&](const char* p) -> float {
                if (inhalt.tiefe == WavTiefe::Gleitkomma32) return lies<float>(p);
                if (inhalt.tiefe == WavTiefe::Pcm16) return static_cast<float>(lies<qint16>(p) / 32767.0);
                // 24 Bit: die drei Bytes nach oben schieben, das Vorzeichen kommt mit
                const qint32 ganz = static_cast<qint32>(static_cast<quint32>(static_cast<quint8>(p[0])) << 8 |
                                                        static_cast<quint32>(static_cast<quint8>(p[1])) << 16 |
                                                        static_cast<quint32>(static_cast<quint8>(p[2])) << 24) >>
                                    8;
                return static_cast<float>(ganz / 8388607.0);
            };
            for (qsizetype f = 0; f < frames; ++f)
            {
                const char* p = anfang + f * bytes * kanaele;
                inhalt.stereo.push_back(wert(p));
                inhalt.stereo.push_back(wert(p + (kanaele - 1) * bytes));
            }
            return inhalt;
        }
        stelle += 8 + static_cast<qsizetype>(groesse) + (groesse % 2);
    }
    return std::nullopt;
}

}  // namespace lumi::audio
