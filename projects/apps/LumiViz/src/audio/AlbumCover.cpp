/**
 ****************************************************************************************
 * @file   AlbumCover.cpp
 * @brief  Albumcover lesen: ID3v2, FLAC, MP4 und Ordner-Dateien (S84)
 ****************************************************************************************
 */

#include "audio/AlbumCover.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>

#include <cstdint>
#include <utility>

namespace lumi::audio
{
namespace
{

/// Obergrenze fuer einen Tag/Block: schuetzt vor kaputten Groessenangaben.
constexpr qint64 kMaxBytes = 32LL * 1024 * 1024;

/// Frontcover laut ID3/FLAC-Bildtyp.
constexpr int kFront = 3;

std::uint32_t be32(const char* p)
{
    const auto* u = reinterpret_cast<const unsigned char*>(p);
    return (std::uint32_t{u[0]} << 24) | (std::uint32_t{u[1]} << 16) |
           (std::uint32_t{u[2]} << 8) | std::uint32_t{u[3]};
}

std::uint32_t be24(const char* p)
{
    const auto* u = reinterpret_cast<const unsigned char*>(p);
    return (std::uint32_t{u[0]} << 16) | (std::uint32_t{u[1]} << 8) | std::uint32_t{u[2]};
}

/// ID3v2.4 und Kopfgroesse: 4 × 7 Bit („syncsafe").
std::uint32_t syncsafe(const char* p)
{
    const auto* u = reinterpret_cast<const unsigned char*>(p);
    return (std::uint32_t{u[0] & 0x7Fu} << 21) | (std::uint32_t{u[1] & 0x7Fu} << 14) |
           (std::uint32_t{u[2] & 0x7Fu} << 7) | std::uint32_t{u[3] & 0x7Fu};
}

/// Unsynchronisation rueckgaengig: jedes 0xFF 0x00 wird 0xFF.
QByteArray entsynchronisiere(const QByteArray& in)
{
    QByteArray out;
    out.reserve(in.size());
    for (qsizetype i = 0; i < in.size(); ++i)
    {
        out.append(in[i]);
        if (static_cast<unsigned char>(in[i]) == 0xFF && i + 1 < in.size() && in[i + 1] == 0)
            ++i;
    }
    return out;
}

/// Bester Kandidat: Frontcover vor allem anderen, sonst das erste Bild.
struct Auswahl
{
    QByteArray bild;
    bool front = false;
    void nimm(QByteArray daten, int typ)
    {
        if (daten.isEmpty() || front) return;
        if (typ == kFront || bild.isEmpty())
        {
            bild = std::move(daten);
            front = typ == kFront;
        }
    }
};

/// Ende eines Textes in `rahmen` ab `pos`: bei UTF-16 (Kodierung 1/2) zwei
/// Nullbytes auf gerader Grenze, sonst eines. Gibt die Position DANACH.
qsizetype hinterText(const QByteArray& rahmen, qsizetype pos, int kodierung)
{
    const bool breit = kodierung == 1 || kodierung == 2;
    if (!breit)
    {
        const qsizetype null = rahmen.indexOf('\0', pos);
        return null < 0 ? -1 : null + 1;
    }
    for (qsizetype i = pos; i + 1 < rahmen.size(); i += 2)
        if (rahmen[i] == 0 && rahmen[i + 1] == 0) return i + 2;
    return -1;
}

/// Inhalt eines APIC-/PIC-Rahmens → Bild (Typ in `typ`).
QByteArray apicBild(const QByteArray& rahmen, bool v22, int& typ)
{
    if (rahmen.size() < 4) return {};
    const int kodierung = static_cast<unsigned char>(rahmen[0]);
    qsizetype pos = 1;
    if (v22)
    {
        pos += 3;  // Bildformat, 3 Zeichen („JPG"/„PNG")
    }
    else
    {
        const qsizetype null = rahmen.indexOf('\0', pos);  // MIME ist immer Latin-1
        if (null < 0) return {};
        const QByteArray mime = rahmen.mid(pos, null - pos);
        if (mime == "-->") return {};  // nur ein Verweis (URL), kein Bild
        pos = null + 1;
    }
    if (pos >= rahmen.size()) return {};
    typ = static_cast<unsigned char>(rahmen[pos]);
    pos = hinterText(rahmen, pos + 1, kodierung);
    if (pos < 0 || pos >= rahmen.size()) return {};
    return rahmen.mid(pos);
}

/// ID3v2-Tag am Dateianfang lesen. `tagEnde` = Bytes bis hinter den Tag
/// (0 = kein ID3), damit FLAC dahinter weitersuchen kann.
QByteArray id3Cover(QFile& f, qint64& tagEnde)
{
    tagEnde = 0;
    f.seek(0);
    const QByteArray kopf = f.read(10);
    if (kopf.size() < 10 || !kopf.startsWith("ID3")) return {};
    const int version = static_cast<unsigned char>(kopf[3]);
    const int flags = static_cast<unsigned char>(kopf[5]);
    const qint64 groesse = syncsafe(kopf.constData() + 6);
    tagEnde = 10 + groesse + ((flags & 0x10) != 0 ? 10 : 0);  // Fusszeile (v2.4)
    if (version < 2 || version > 4 || groesse > kMaxBytes) return {};

    QByteArray tag = f.read(groesse);
    // v2.2/v2.3: Unsynchronisation gilt fuer den ganzen Tag; v2.4: je Rahmen.
    if ((flags & 0x80) != 0 && version < 4) tag = entsynchronisiere(tag);

    qsizetype pos = 0;
    if ((flags & 0x40) != 0 && version >= 3 && tag.size() >= 4)  // erweiterter Kopf
        pos = version == 3 ? 4 + be32(tag.constData()) : syncsafe(tag.constData());

    const bool v22 = version == 2;
    const int idLaenge = v22 ? 3 : 4;
    const int kopfLaenge = v22 ? 6 : 10;
    Auswahl auswahl;
    while (pos + kopfLaenge <= tag.size())
    {
        const char* r = tag.constData() + pos;
        if (r[0] == 0) break;  // Auffuellung
        const QByteArray id(r, idLaenge);
        const qint64 laenge = v22 ? be24(r + 3) : version == 4 ? syncsafe(r + 4) : be32(r + 4);
        const int rflags = v22 ? 0 : static_cast<unsigned char>(r[9]);
        pos += kopfLaenge;
        if (laenge <= 0 || pos + laenge > tag.size()) break;
        if (id == (v22 ? QByteArray("PIC") : QByteArray("APIC")))
        {
            QByteArray inhalt = tag.mid(pos, laenge);
            if (version == 4)
            {
                if ((rflags & 0x01) != 0) inhalt.remove(0, 4);  // Datenlaengen-Angabe
                if ((rflags & 0x02) != 0) inhalt = entsynchronisiere(inhalt);
            }
            const bool gepackt = !v22 && (version == 4 ? (rflags & 0x08) : (rflags & 0x80)) != 0;
            const bool verschluesselt =
                !v22 && (version == 4 ? (rflags & 0x04) : (rflags & 0x40)) != 0;
            if (!gepackt && !verschluesselt)
            {
                // Zwei Schritte: `typ` setzt apicBild — in EINEM Aufruf waere
                // die Auswertungsreihenfolge der Argumente offen (MSVC las 0).
                int typ = 0;
                QByteArray bild = apicBild(inhalt, v22, typ);
                auswahl.nimm(std::move(bild), typ);
            }
        }
        pos += laenge;
    }
    return auswahl.bild;
}

/// FLAC-Metadaten ab `start` (hinter einem etwaigen ID3-Kopf): Block 6.
QByteArray flacCover(QFile& f, qint64 start)
{
    f.seek(start);
    if (f.read(4) != "fLaC") return {};
    Auswahl auswahl;
    for (int block = 0; block < 128; ++block)
    {
        const QByteArray kopf = f.read(4);
        if (kopf.size() < 4) break;
        const bool letzter = (static_cast<unsigned char>(kopf[0]) & 0x80) != 0;
        const int art = static_cast<unsigned char>(kopf[0]) & 0x7F;
        const qint64 laenge = be24(kopf.constData() + 1);
        if (art == 6 && laenge <= kMaxBytes)
        {
            const QByteArray b = f.read(laenge);
            if (b.size() < 32) break;
            const int typ = static_cast<int>(be32(b.constData()));
            qint64 pos = 4;
            const qint64 mimeLaenge = be32(b.constData() + pos);
            pos += 4 + mimeLaenge;
            if (pos + 4 > b.size()) break;
            const qint64 beschrLaenge = be32(b.constData() + pos);
            pos += 4 + beschrLaenge + 16;  // Beschreibung, Breite/Hoehe/Tiefe/Farben
            if (pos + 4 > b.size()) break;
            const qint64 datenLaenge = be32(b.constData() + pos);
            pos += 4;
            if (pos + datenLaenge > b.size()) break;
            auswahl.nimm(b.mid(pos, datenLaenge), typ);
        }
        else if (!f.seek(f.pos() + laenge))
        {
            break;
        }
        if (letzter || auswahl.front) break;
    }
    return auswahl.bild;
}

/// MP4: Box-Baum `moov/udta/meta/ilst/covr/data` ablaufen.
QByteArray mp4Box(QFile& f, qint64 anfang, qint64 ende, const QList<QByteArray>& pfad, int tiefe)
{
    qint64 pos = anfang;
    while (pos + 8 <= ende)
    {
        f.seek(pos);
        const QByteArray kopf = f.read(8);
        if (kopf.size() < 8) return {};
        qint64 groesse = be32(kopf.constData());
        const QByteArray typ = kopf.mid(4, 4);
        qint64 inhalt = pos + 8;
        if (groesse == 1)  // 64-Bit-Groesse
        {
            const QByteArray gross = f.read(8);
            if (gross.size() < 8) return {};
            groesse = (static_cast<qint64>(be32(gross.constData())) << 32) |
                      be32(gross.constData() + 4);
            inhalt += 8;
        }
        else if (groesse == 0)
        {
            groesse = ende - pos;  // bis zum Ende
        }
        if (groesse < 8 || pos + groesse > ende) return {};
        if (typ == pfad[tiefe])
        {
            if (tiefe + 1 == pfad.size())  // `data`: 4 Typ + 4 Locale, dann Bild
            {
                const qint64 laenge = pos + groesse - inhalt - 8;
                if (laenge <= 0 || laenge > kMaxBytes) return {};
                f.seek(inhalt + 8);
                return f.read(laenge);
            }
            // `meta` ist eine „full box": 4 Bytes Version/Flags vor den Kindern
            const qint64 kinder = inhalt + (typ == "meta" ? 4 : 0);
            return mp4Box(f, kinder, pos + groesse, pfad, tiefe + 1);
        }
        pos += groesse;
    }
    return {};
}

QByteArray mp4Cover(QFile& f)
{
    f.seek(4);
    if (f.read(4) != "ftyp") return {};
    static const QList<QByteArray> kPfad = {"moov", "udta", "meta", "ilst", "covr", "data"};
    return mp4Box(f, 0, f.size(), kPfad, 0);
}

QImage verkleinert(const QImage& bild, int kante)
{
    if (bild.isNull() || kante <= 0) return bild;
    return bild.scaled(kante, kante, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

}  // namespace

QByteArray eingebettetesCover(const QString& pfad)
{
    QFile f(pfad);
    if (!f.open(QIODevice::ReadOnly)) return {};
    qint64 id3Ende = 0;
    QByteArray bild = id3Cover(f, id3Ende);
    if (bild.isEmpty()) bild = flacCover(f, id3Ende);
    if (bild.isEmpty()) bild = mp4Cover(f);
    return bild;
}

QString ordnerCover(const QString& ordner)
{
    const QDir dir(ordner);
    // Reihenfolge = Vorrang: die ueblichen Namen der Player/Ripper.
    for (const char* stamm : {"cover", "folder", "front", "albumart*"})
    {
        QStringList muster;
        for (const char* endung : {"jpg", "jpeg", "png"})
            muster << QStringLiteral("%1.%2").arg(QLatin1String(stamm), QLatin1String(endung));
        const QFileInfoList treffer =
            dir.entryInfoList(muster, QDir::Files, QDir::Name | QDir::IgnoreCase);
        if (!treffer.isEmpty()) return treffer.first().absoluteFilePath();
    }
    return {};
}

QImage coverVorschau(const QString& pfad, int kante)
{
    QImage bild = QImage::fromData(eingebettetesCover(pfad));
    if (bild.isNull())
    {
        const QString datei = ordnerCover(QFileInfo(pfad).absolutePath());
        if (!datei.isEmpty()) bild.load(datei);
    }
    return verkleinert(bild, kante);
}

// =============================================================================
// CoverLader
// =============================================================================

CoverLader::CoverLader(QObject* parent) : QObject(parent)
{
    m_arbeiter = new QObject();
    m_arbeiter->moveToThread(&m_thread);
    m_thread.setObjectName(QStringLiteral("CoverLader"));
    m_thread.start(QThread::LowPriority);
}

CoverLader::~CoverLader()
{
    m_auftrag = -1;  // laufende Schleife bricht beim naechsten Titel ab
    m_thread.quit();
    m_thread.wait();
    delete m_arbeiter;
}

int CoverLader::lade(const QStringList& pfade, int kante)
{
    const int nummer = ++m_auftrag;
    QMetaObject::invokeMethod(
        m_arbeiter,
        [this, nummer, pfade, kante]() {
            // Ordner-Cover je Ordner nur einmal suchen und laden.
            QHash<QString, QImage> ordnerBild;
            for (const QString& pfad : pfade)
            {
                if (m_auftrag.load() != nummer) break;
                QImage bild = QImage::fromData(eingebettetesCover(pfad));
                if (bild.isNull())
                {
                    const QString ordner = QFileInfo(pfad).absolutePath();
                    auto it = ordnerBild.find(ordner);
                    if (it == ordnerBild.end())
                    {
                        QImage o;
                        const QString datei = ordnerCover(ordner);
                        if (!datei.isEmpty()) o.load(datei);
                        it = ordnerBild.insert(ordner, verkleinert(o, kante));
                    }
                    bild = *it;
                }
                else
                {
                    bild = verkleinert(bild, kante);
                }
                if (!bild.isNull()) Q_EMIT geladen(nummer, pfad, bild);
            }
            Q_EMIT fertig(nummer);
        },
        Qt::QueuedConnection);
    return nummer;
}

}  // namespace lumi::audio
