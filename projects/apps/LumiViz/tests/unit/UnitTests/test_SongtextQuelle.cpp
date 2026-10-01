/**
 ****************************************************************************************
 * @file   test_SongtextQuelle.cpp
 * @brief  Suchreihenfolge der Songtext-Quellen (Entscheid Patrik 2a, S86)
 *
 * @details
 * Synchron vor unsynchron, nah am Song vor fern — geprueft an echten Dateien in
 * einem Wegwerf-Ordner. Der Benutzerordner ist ein Parameter, der Test fasst
 * den echten nicht an.
 ****************************************************************************************
 */

#include <doctest.h>

#include "audio/SongtextOnline.hpp"
#include "audio/SongtextQuelle.hpp"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>

using lumi::audio::findeSongtext;

namespace
{
void schreibe(const QString& pfad, const char* inhalt)
{
    QDir().mkpath(QFileInfo(pfad).absolutePath());
    QFile f(pfad);
    REQUIRE(f.open(QIODevice::WriteOnly));
    f.write(inhalt);
}

const char* kSync = "[00:01.00]synchron\n";
const char* kPlain = "unsynchron\n";
}  // namespace

TEST_CASE("SongtextQuelle: Reihenfolge — synchron vor unsynchron, nah vor fern")
{
    QTemporaryDir tmp;
    REQUIRE(tmp.isValid());
    const QString musik = tmp.path() + "/musik";
    const QString nutzer = tmp.path() + "/nutzer";
    QDir().mkpath(musik);

    TrackInfo t;
    t.filePath = musik + "/07 - Blue Monday.mp3";
    t.title = "Blue Monday";
    t.artist = "New Order";
    const QString neben = musik + "/07 - Blue Monday";
    const QString fern = nutzer + "/New Order - Blue Monday";

    // nichts da
    CHECK(findeSongtext(t, nutzer).leer());

    // 6: .txt im Benutzerordner
    schreibe(fern + ".txt", kPlain);
    CHECK(findeSongtext(t, nutzer).quelle == "user folder: New Order - Blue Monday.txt");
    CHECK_FALSE(findeSongtext(t, nutzer).synchron);
    // 5: .txt neben dem Song schlaegt 6
    schreibe(neben + ".txt", kPlain);
    CHECK(findeSongtext(t, nutzer).quelle == "07 - Blue Monday.txt");
    // 4: eingebettet unsynchron schlaegt 5
    t.lyricsPlain = "eingebettet\n";
    CHECK(findeSongtext(t, nutzer).quelle == "embedded");
    // 3: .lrc im Benutzerordner (mit Zeit) schlaegt alles Unsynchrone
    schreibe(fern + ".lrc", kSync);
    CHECK(findeSongtext(t, nutzer).quelle == "user folder: New Order - Blue Monday.lrc");
    CHECK(findeSongtext(t, nutzer).synchron);
    // 2: .lrc neben dem Song schlaegt 3
    schreibe(neben + ".lrc", kSync);
    CHECK(findeSongtext(t, nutzer).quelle == "07 - Blue Monday.lrc");
    // der Fund aus einer Datei nennt ihren Pfad (Knopf „Open folder" im Editor) …
    CHECK(QString::fromStdString(findeSongtext(t, nutzer).datei) ==
          QFileInfo(neben + ".lrc").absoluteFilePath());
    // 1: eingebettet synchron schlaegt 2
    t.lyricsSynced = "[00:02.00]sylt\n";
    CHECK(findeSongtext(t, nutzer).quelle == "embedded (synced)");
    // … eingebetteter Text hat keine Datei
    CHECK(findeSongtext(t, nutzer).datei.empty());
}

TEST_CASE("SongtextQuelle: synchron ist, was Zeitmarken traegt — nicht die Endung")
{
    QTemporaryDir tmp;
    REQUIRE(tmp.isValid());
    TrackInfo t;
    t.filePath = tmp.path() + "/lied.flac";
    t.title = "Lied";

    // eine .lrc OHNE Zeitmarken ist unsynchron und verliert gegen eingebetteten Text
    schreibe(tmp.path() + "/lied.lrc", kPlain);
    t.lyricsPlain = "eingebettet\n";
    CHECK(findeSongtext(t, QString()).quelle == "embedded");
    // ohne eingebetteten Text wird sie als unsynchroner Text genommen
    t.lyricsPlain.clear();
    CHECK(findeSongtext(t, QString()).quelle == "lied.lrc");
    CHECK_FALSE(findeSongtext(t, QString()).synchron);
    // ein eingebetteter Fliesstext MIT Zeitmarken gilt als synchron
    t.lyricsPlain = "[00:03.00]mit Zeit\n";
    CHECK(findeSongtext(t, QString()).synchron);
    CHECK(findeSongtext(t, QString()).quelle == "embedded (synced)");
}

TEST_CASE("SongtextQuelle: Name im Benutzerordner und Speichern")
{
    TrackInfo t;
    t.title = "What/Ever: Now?";
    t.artist = "AC/DC";
    CHECK(lumi::audio::songtextName(t) == "AC_DC - What_Ever_ Now_");
    t.artist.clear();
    CHECK(lumi::audio::songtextName(t) == "What_Ever_ Now_");
    t.title.clear();
    CHECK(lumi::audio::songtextName(t).isEmpty());

    // Speichern und ueber die Suche wiederfinden (der Weg des Online-Funds
    // und eines spaeteren Nachsynchronisierens)
    QTemporaryDir tmp;
    REQUIRE(tmp.isValid());
    TrackInfo lied;
    lied.filePath = tmp.path() + "/x.mp3";
    lied.title = "Titel";
    lied.artist = "Wer";
    auto text = lumi::multieffect::lyrics::leseLrc("[00:05.00]eins\n[00:09.50]zwei\n");
    const QString ziel = tmp.path() + "/nutzer/" + lumi::audio::songtextName(lied) + ".lrc";
    REQUIRE(lumi::audio::speichereSongtext(text, ziel));
    const auto gefunden = findeSongtext(lied, tmp.path() + "/nutzer");
    REQUIRE(gefunden.zeilen.size() == 2);
    CHECK(gefunden.synchron);
    CHECK(gefunden.zeilen[1].zeitMs == 9500);
}

// --- Online-Abruf (Entscheide 3a/4a): Anfrage, Antwort, Speicherort — ohne Netz

TEST_CASE("SongtextOnline: Anfrage nur mit Titel UND Interpret, Dauer in Sekunden")
{
    TrackInfo t;
    t.title = "I Want to Live";
    CHECK(lumi::audio::lrclibUrl(t).isEmpty());  // Interpret fehlt: gar nicht fragen
    t.artist = "Borislav Slavov";
    t.album = "Baldur's Gate 3";
    t.durationMs = 233400;
    const QString url = lumi::audio::lrclibUrl(t).toString(QUrl::FullyEncoded);
    CHECK(url.startsWith("https://lrclib.net/api/get?"));
    CHECK(url.contains("track_name=I%20Want%20to%20Live"));
    CHECK(url.contains("artist_name=Borislav%20Slavov"));
    CHECK(url.contains("duration=233"));
    // Sonderzeichen: & und + duerfen die Anfrage nicht zerlegen
    t.title = "Rock & Roll + More";
    const QString u2 = lumi::audio::lrclibUrl(t).toString(QUrl::FullyEncoded);
    CHECK(u2.contains("track_name=Rock%20%26%20Roll%20%2B%20More"));
    // Dauer ausserhalb 1..3600 s wird weggelassen statt abgewiesen
    t.durationMs = 4000 * 1000;
    CHECK_FALSE(lumi::audio::lrclibUrl(t).toString().contains("duration="));
}

TEST_CASE("SongtextOnline: Antwort — synchron vor unsynchron, instrumental leer")
{
    using lumi::audio::songtextAusLrclib;
    const auto beide = songtextAusLrclib(
        R"({"id":3396226,"instrumental":false,"plainLyrics":"eins\nzwei\n",)"
        R"("syncedLyrics":"[00:17.12] eins\n[00:21.00] zwei\n"})");
    REQUIRE(beide.zeilen.size() == 2);
    CHECK(beide.synchron);
    CHECK(beide.zeilen[0].zeitMs == 17120);
    CHECK(beide.zeilen[0].text == "eins");
    CHECK(beide.quelle == "LRCLIB");
    // Herkunft fuer die Datei: Dienst und Kennung (das Datum haengt der Abruf an)
    CHECK(beide.herkunft == "LRCLIB id 3396226");
    // gespeichert und ueber die lokale Suche wiedergefunden, traegt der Text den
    // Vermerk weiter — auch nach einem Neustart ist die Herkunft bekannt
    {
        QTemporaryDir tmp;
        REQUIRE(tmp.isValid());
        TrackInfo lied;
        lied.filePath = tmp.path() + "/x.mp3";
        lied.title = "Titel";
        lied.artist = "Wer";
        REQUIRE(lumi::audio::speichereSongtext(beide, tmp.path() + "/Wer - Titel.lrc"));
        const auto gefunden = findeSongtext(lied, tmp.path());
        CHECK(gefunden.herkunft == "LRCLIB id 3396226");
        CHECK(gefunden.zeilen.size() == 2);
    }

    const auto nurText = songtextAusLrclib(
        R"({"instrumental":false,"plainLyrics":"eins\nzwei\n","syncedLyrics":null})");
    CHECK_FALSE(nurText.synchron);
    CHECK(nurText.zeilen.size() == 2);

    CHECK(songtextAusLrclib(R"({"instrumental":true,"plainLyrics":null,"syncedLyrics":null})")
              .leer());
    CHECK(songtextAusLrclib(R"({"message":"Failed to find specified track","statusCode":404})")
              .leer());
    CHECK(songtextAusLrclib("kein json").leer());
}

TEST_CASE("SongtextOnline: Speicherort nach Einstellung, Ausweichen bei gesperrtem Ordner")
{
    using lumi::audio::SongtextAblage;
    using lumi::audio::songtextZiel;
    QTemporaryDir tmp;
    REQUIRE(tmp.isValid());
    TrackInfo t;
    t.filePath = tmp.path() + "/musik/07 - Lied.mp3";
    t.title = "Lied";
    t.artist = "Wer";
    QDir().mkpath(tmp.path() + "/musik");
    const QString nutzer = tmp.path() + "/nutzer";

    // Vorgabe Benutzerordner: Name aus Interpret und Titel; synchron .lrc, sonst .txt
    CHECK(songtextZiel(t, true, SongtextAblage::Benutzerordner, nutzer) ==
          nutzer + "/Wer - Lied.lrc");
    CHECK(songtextZiel(t, false, SongtextAblage::Benutzerordner, nutzer) ==
          nutzer + "/Wer - Lied.txt");
    // neben dem Song: der Name des Songs
    bool ausgewichen = true;
    CHECK(songtextZiel(t, true, SongtextAblage::NebenDemSong, nutzer, &ausgewichen) ==
          tmp.path() + "/musik/07 - Lied.lrc");
    CHECK_FALSE(ausgewichen);
    // Ordner des Songs gibt es nicht (nicht beschreibbar): Benutzerordner, gemeldet
    t.filePath = tmp.path() + "/gibt-es-nicht/07 - Lied.mp3";
    CHECK(songtextZiel(t, true, SongtextAblage::NebenDemSong, nutzer, &ausgewichen) ==
          nutzer + "/Wer - Lied.lrc");
    CHECK(ausgewichen);
}
