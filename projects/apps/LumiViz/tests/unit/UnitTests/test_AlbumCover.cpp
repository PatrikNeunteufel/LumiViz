/**
 ****************************************************************************************
 * @file   test_AlbumCover.cpp
 * @brief  Albumcover lesen (S84): ID3v2.2/2.3/2.4, FLAC, MP4, Ordner-Datei
 *
 * @details
 * Die Dateien werden hier Byte fuer Byte nach Spezifikation gebaut — so ist
 * jeder Sonderfall (Unsynchronisation, UTF-16-Beschreibung, Front vor Rueck-
 * seite, FLAC hinter ID3) gezielt pruefbar, ohne echte Musik im Repo. Die
 * „Bilder" sind beliebige Bytes MIT 0xFF darin: verglichen wird byte-genau,
 * decodiert wird nicht.
 ****************************************************************************************
 */

#include <doctest.h>

#include "audio/AlbumCover.hpp"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>

using lumi::audio::eingebettetesCover;
using lumi::audio::ordnerCover;

namespace
{

QByteArray be32(quint32 v)
{
    QByteArray b(4, '\0');
    b[0] = static_cast<char>((v >> 24) & 0xFF);
    b[1] = static_cast<char>((v >> 16) & 0xFF);
    b[2] = static_cast<char>((v >> 8) & 0xFF);
    b[3] = static_cast<char>(v & 0xFF);
    return b;
}

QByteArray be24(quint32 v)
{
    return be32(v).mid(1);
}

QByteArray syncsafe(quint32 v)
{
    QByteArray b(4, '\0');
    b[0] = static_cast<char>((v >> 21) & 0x7F);
    b[1] = static_cast<char>((v >> 14) & 0x7F);
    b[2] = static_cast<char>((v >> 7) & 0x7F);
    b[3] = static_cast<char>(v & 0x7F);
    return b;
}

/// Unterscheidbare „Bilder" mit 0xFF-Bytes (Unsynchronisations-Fall).
const QByteArray kFront = QByteArray::fromHex("FFD8FFE000104A464946FF00FFAA01");
const QByteArray kRueck = QByteArray::fromHex("89504E470D0A1A0A0000FFEE");

QByteArray apic(int typ, const QByteArray& bild, int kodierung = 0,
                const QByteArray& beschreibung = QByteArray("\0", 1))
{
    QByteArray r;
    r.append(static_cast<char>(kodierung));
    r.append("image/jpeg");
    r.append('\0');
    r.append(static_cast<char>(typ));
    r.append(beschreibung);
    r.append(bild);
    return r;
}

QByteArray id3Tag(int version, const QList<QPair<QByteArray, QByteArray>>& rahmen,
                  bool unsync = false)
{
    QByteArray koerper;
    for (const auto& [id, inhalt] : rahmen)
    {
        koerper.append(id);
        koerper.append(version == 4 ? syncsafe(static_cast<quint32>(inhalt.size()))
                                    : be32(static_cast<quint32>(inhalt.size())));
        koerper.append(QByteArray(2, '\0'));
        koerper.append(inhalt);
    }
    if (unsync)
    {
        QByteArray u;
        for (char c : koerper)
        {
            u.append(c);
            if (static_cast<unsigned char>(c) == 0xFF) u.append('\0');
        }
        koerper = u;
    }
    koerper.append(QByteArray(16, '\0'));  // Auffuellung
    QByteArray tag("ID3");
    tag.append(static_cast<char>(version));
    tag.append('\0');
    tag.append(static_cast<char>(unsync ? 0x80 : 0x00));
    tag.append(syncsafe(static_cast<quint32>(koerper.size())));
    return tag + koerper;
}

QString schreibe(const QTemporaryDir& dir, const QString& name, const QByteArray& inhalt)
{
    const QString pfad = dir.filePath(name);
    QFile f(pfad);
    REQUIRE(f.open(QIODevice::WriteOnly));
    f.write(inhalt);
    return pfad;
}

QByteArray flacBild(int typ, const QByteArray& bild)
{
    QByteArray b = be32(static_cast<quint32>(typ));
    const QByteArray mime("image/jpeg");
    b += be32(static_cast<quint32>(mime.size())) + mime;
    b += be32(0);                    // Beschreibung leer
    b += QByteArray(16, '\0');       // Breite, Hoehe, Tiefe, Farben
    b += be32(static_cast<quint32>(bild.size())) + bild;
    return b;
}

QByteArray flacDatei(const QList<QPair<int, QByteArray>>& bilder)
{
    QByteArray f("fLaC");
    const bool nurInfo = bilder.isEmpty();
    f.append(static_cast<char>(nurInfo ? 0x80 : 0x00));  // STREAMINFO
    f += be24(34) + QByteArray(34, '\0');
    for (qsizetype i = 0; i < bilder.size(); ++i)
    {
        const QByteArray block = flacBild(bilder[i].first, bilder[i].second);
        const bool letzter = i + 1 == bilder.size();
        f.append(static_cast<char>((letzter ? 0x80 : 0x00) | 6));
        f += be24(static_cast<quint32>(block.size())) + block;
    }
    return f + QByteArray("audio-frames");
}

QByteArray box(const QByteArray& typ, const QByteArray& inhalt)
{
    return be32(static_cast<quint32>(8 + inhalt.size())) + typ + inhalt;
}

}  // namespace

TEST_SUITE("AlbumCover")
{
    TEST_CASE("ID3v2.3: Frontcover gewinnt vor der Rueckseite")
    {
        QTemporaryDir dir;
        const QString p = schreibe(
            dir, QStringLiteral("a.mp3"),
            id3Tag(3, {{"TIT2", QByteArray("\0Titel", 6)},
                       {"APIC", apic(4, kRueck)},
                       {"APIC", apic(3, kFront)}}) + QByteArray("MPEG"));
        CHECK(eingebettetesCover(p) == kFront);
    }

    TEST_CASE("ID3v2.3: ohne Frontcover das erste Bild")
    {
        QTemporaryDir dir;
        const QString p = schreibe(dir, QStringLiteral("a.mp3"),
                                   id3Tag(3, {{"APIC", apic(4, kRueck)},
                                              {"APIC", apic(0, kFront)}}));
        CHECK(eingebettetesCover(p) == kRueck);
    }

    TEST_CASE("ID3v2.3 mit Unsynchronisation: 0xFF 0x00 wird zurueckgenommen")
    {
        QTemporaryDir dir;
        const QString p = schreibe(dir, QStringLiteral("a.mp3"),
                                   id3Tag(3, {{"APIC", apic(3, kFront)}}, true));
        CHECK(eingebettetesCover(p) == kFront);
    }

    TEST_CASE("ID3v2.4: syncsafe Rahmengroesse, UTF-16-Beschreibung")
    {
        QTemporaryDir dir;
        // UTF-16 mit BOM: „ab" + zwei Nullbytes als Ende
        const QByteArray beschr = QByteArray::fromHex("FFFE610062000000");
        QByteArray gross = kFront;
        gross += QByteArray(200, 'x');  // > 127: syncsafe unterscheidet sich von be32
        const QString p = schreibe(dir, QStringLiteral("a.mp3"),
                                   id3Tag(4, {{"APIC", apic(3, gross, 1, beschr)}}));
        CHECK(eingebettetesCover(p) == gross);
    }

    TEST_CASE("ID3v2.2: Rahmen PIC mit 3-Zeichen-Format")
    {
        QTemporaryDir dir;
        QByteArray pic;
        pic.append('\0');
        pic.append("JPG");
        pic.append(static_cast<char>(3));
        pic.append('\0');
        pic.append(kFront);
        QByteArray koerper = QByteArray("PIC") + be24(static_cast<quint32>(pic.size())) + pic;
        QByteArray tag("ID3");
        tag.append(static_cast<char>(2));
        tag.append('\0');
        tag.append('\0');
        tag.append(syncsafe(static_cast<quint32>(koerper.size())));
        const QString p = schreibe(dir, QStringLiteral("a.mp3"), tag + koerper);
        CHECK(eingebettetesCover(p) == kFront);
    }

    TEST_CASE("ID3: ein Verweis (MIME '-->') ist kein Bild")
    {
        QTemporaryDir dir;
        QByteArray link;
        link.append('\0');
        link.append("-->");
        link.append('\0');
        link.append(static_cast<char>(3));
        link.append('\0');
        link.append("http://example.org/cover.jpg");
        const QString p =
            schreibe(dir, QStringLiteral("a.mp3"), id3Tag(3, {{"APIC", link}}));
        CHECK(eingebettetesCover(p).isEmpty());
    }

    TEST_CASE("FLAC: PICTURE-Block, Front bevorzugt — auch hinter einem ID3-Kopf")
    {
        QTemporaryDir dir;
        const QByteArray flac = flacDatei({{4, kRueck}, {3, kFront}});
        CHECK(eingebettetesCover(schreibe(dir, QStringLiteral("a.flac"), flac)) == kFront);
        const QByteArray mitId3 = id3Tag(3, {{"TIT2", QByteArray("\0x", 2)}}) + flac;
        CHECK(eingebettetesCover(schreibe(dir, QStringLiteral("b.flac"), mitId3)) == kFront);
        CHECK(eingebettetesCover(schreibe(dir, QStringLiteral("c.flac"), flacDatei({})))
                  .isEmpty());
    }

    TEST_CASE("MP4: moov/udta/meta/ilst/covr/data")
    {
        QTemporaryDir dir;
        const QByteArray data = box("data", be32(13) + be32(0) + kFront);
        const QByteArray ilst = box("ilst", box("covr", data));
        const QByteArray meta = box("meta", be32(0) + box("hdlr", QByteArray(25, '\0')) + ilst);
        const QByteArray moov = box("moov", box("mvhd", QByteArray(100, '\0')) +
                                                box("udta", meta));
        const QByteArray datei =
            box("ftyp", QByteArray("M4A ") + be32(0)) + box("mdat", QByteArray(500, 'm')) + moov;
        CHECK(eingebettetesCover(schreibe(dir, QStringLiteral("a.m4a"), datei)) == kFront);

        // Gegenprobe: ohne covr kein Bild
        const QByteArray ohne = box("ftyp", QByteArray("M4A ") + be32(0)) +
                                box("moov", box("udta", box("meta", be32(0))));
        CHECK(eingebettetesCover(schreibe(dir, QStringLiteral("b.m4a"), ohne)).isEmpty());
    }

    TEST_CASE("Kein Tag, kaputte Groessen: leer statt Absturz")
    {
        QTemporaryDir dir;
        CHECK(eingebettetesCover(schreibe(dir, QStringLiteral("a.wav"), "RIFF....WAVE"))
                  .isEmpty());
        // ID3 mit Rahmen, der laenger sein will als der Tag
        QByteArray kaputt("ID3");
        kaputt.append(static_cast<char>(3));
        kaputt.append(QByteArray(2, '\0'));
        kaputt.append(syncsafe(20));
        kaputt.append("APIC");
        kaputt.append(be32(100000));
        kaputt.append(QByteArray(12, '\0'));
        CHECK(eingebettetesCover(schreibe(dir, QStringLiteral("b.mp3"), kaputt)).isEmpty());
        CHECK(eingebettetesCover(dir.filePath(QStringLiteral("gibtsnicht.mp3"))).isEmpty());
    }

    TEST_CASE("Ordner-Cover: cover vor folder, Gross-/Kleinschreibung egal")
    {
        QTemporaryDir dir;
        CHECK(ordnerCover(dir.path()).isEmpty());
        schreibe(dir, QStringLiteral("Folder.JPG"), "x");
        CHECK(QFileInfo(ordnerCover(dir.path())).fileName() == QStringLiteral("Folder.JPG"));
        schreibe(dir, QStringLiteral("cover.png"), "x");
        CHECK(QFileInfo(ordnerCover(dir.path())).fileName() == QStringLiteral("cover.png"));
    }
}
