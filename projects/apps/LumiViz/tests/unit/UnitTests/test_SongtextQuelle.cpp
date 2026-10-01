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
    // 1: eingebettet synchron schlaegt 2
    t.lyricsSynced = "[00:02.00]sylt\n";
    CHECK(findeSongtext(t, nutzer).quelle == "embedded (synced)");
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
