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

#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QTemporaryDir>

#include <vector>

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

// --- S87: Alternativen, Uebernehmen, Sperrliste (Wunsch Patrik)

TEST_CASE("SongtextOnline: Suche — freie Begriffe, Kandidaten mit Dauer und Text")
{
    using lumi::audio::lrclibSuchUrl;
    CHECK(lrclibSuchUrl("", "Wer").isEmpty());  // ohne Titel keine Suche
    const QString url = lrclibSuchUrl("Gettin' Over You", "David Guetta")
                            .toString(QUrl::FullyEncoded);
    CHECK(url.startsWith("https://lrclib.net/api/search?"));
    CHECK(url.contains("track_name=Gettin'%20Over%20You"));
    CHECK(url.contains("artist_name=David%20Guetta"));
    CHECK_FALSE(lrclibSuchUrl("Nur Titel", "").toString().contains("artist_name"));

    const auto k = lumi::audio::kandidatenAusLrclib(
        R"([{"id":11,"trackName":"Lied","artistName":"Wer","albumName":"Eins","duration":181.4,)"
        R"("instrumental":false,"plainLyrics":"a\nb\n","syncedLyrics":"[00:01.00] a\n[00:02.00] b\n"},)"
        R"({"id":12,"trackName":"Lied - Remix","artistName":"Wer","albumName":"Zwei","duration":240,)"
        R"("instrumental":false,"plainLyrics":"a\n","syncedLyrics":null},)"
        R"({"id":13,"trackName":"Lied","artistName":"Wer","albumName":"Drei","duration":200,)"
        R"("instrumental":true,"plainLyrics":null,"syncedLyrics":null},)"
        R"({"trackName":"ohne Kennung"}])");
    REQUIRE(k.size() == 3);  // der Eintrag ohne Kennung faellt weg
    CHECK(k[0].id == 11);
    CHECK(k[0].album == "Eins");
    CHECK(k[0].dauerSek == 181);
    CHECK(k[0].text.synchron);
    CHECK(k[0].text.herkunft == "LRCLIB id 11");
    CHECK_FALSE(k[1].text.synchron);
    CHECK(k[1].text.zeilen.size() == 1);
    CHECK(k[2].instrumental);
    CHECK(k[2].text.leer());
    CHECK(lumi::audio::kandidatenAusLrclib("kein json").empty());

    CHECK(lumi::audio::lrclibKennung("LRCLIB id 3396226, fetched 2026-10-01") == 3396226);
    CHECK(lumi::audio::lrclibKennung("LRCLIB") == 0);
    CHECK(lumi::audio::lrclibKennung("") == 0);
}

TEST_CASE("SongtextOnline: Uebernehmen ersetzt die geholte Fassung, nie eine eigene Datei")
{
    namespace ly = lumi::multieffect::lyrics;
    QTemporaryDir tmp;
    REQUIRE(tmp.isValid());
    const QString nutzer = tmp.path() + "/nutzer";
    TrackInfo t;
    t.filePath = tmp.path() + "/musik/07 - Lied.mp3";
    t.title = "Lied";
    t.artist = "Wer";

    // zuerst eine synchrone Fassung, dann eine unsynchrone: die alte .lrc muss
    // weg, sonst gewoenne sie in der Suchreihenfolge gegen die neue .txt
    ly::Songtext sync = ly::leseLrc("[00:01.00]falsch\n");
    sync.herkunft = "LRCLIB id 1";
    const ly::Songtext erster = lumi::audio::uebernehmeSongtext(t, sync, nutzer);
    CHECK(QFile::exists(nutzer + "/Wer - Lied.lrc"));
    CHECK(erster.datei == (nutzer + "/Wer - Lied.lrc").toStdString());
    CHECK(erster.herkunft.starts_with("LRCLIB id 1, fetched "));

    ly::Songtext plain = ly::leseLrc("richtig\n");
    plain.herkunft = "LRCLIB id 2";
    const ly::Songtext zweiter = lumi::audio::uebernehmeSongtext(t, plain, nutzer);
    CHECK(QFile::exists(nutzer + "/Wer - Lied.txt"));
    CHECK_FALSE(QFile::exists(nutzer + "/Wer - Lied.lrc"));
    CHECK(findeSongtext(t, nutzer).zeilen.at(0).text == "richtig");
    CHECK(zweiter.quelle == "LRCLIB, saved: Wer - Lied.txt");

    // eine SELBST angelegte .lrc (ohne Vermerk) bleibt liegen
    QFile::remove(nutzer + "/Wer - Lied.txt");
    schreibe(nutzer + "/Wer - Lied.lrc", kSync);
    (void)lumi::audio::uebernehmeSongtext(t, plain, nutzer);
    CHECK(QFile::exists(nutzer + "/Wer - Lied.lrc"));
}

TEST_CASE("SongtextOnline: Wrong lyrics — Datei weg, Kennung gesperrt, Abruf nimmt sie nicht mehr")
{
    namespace ly = lumi::multieffect::lyrics;
    using lumi::audio::SongtextAbruf;
    QTemporaryDir tmp;
    REQUIRE(tmp.isValid());
    const QString nutzer = tmp.path() + "/nutzer";
    TrackInfo t;
    t.filePath = tmp.path() + "/x.mp3";
    t.title = "Lied";
    t.artist = "Wer";

    CHECK(lumi::audio::songtextSperrliste(nutzer).empty());
    ly::Songtext s = ly::leseLrc("[00:01.00]falsch\n");
    s.herkunft = "LRCLIB id 7";
    const ly::Songtext geholt = lumi::audio::uebernehmeSongtext(t, s, nutzer);
    REQUIRE(QFile::exists(QString::fromStdString(geholt.datei)));

    REQUIRE(lumi::audio::verwerfeSongtext(t, geholt.herkunft, geholt.datei, nutzer));
    CHECK_FALSE(QFile::exists(QString::fromStdString(geholt.datei)));
    CHECK(lumi::audio::songtextGesperrt(7, nutzer));
    const auto liste = lumi::audio::songtextSperrliste(nutzer);
    REQUIRE(liste.size() == 1);
    CHECK(liste[0].id == 7);
    CHECK(liste[0].titel == "Wer - Lied");
    lumi::audio::sperreSongtext(7, "doppelt", nutzer);  // kein zweiter Eintrag
    CHECK(lumi::audio::songtextSperrliste(nutzer).size() == 1);

    // der selbsttaetige Abruf liefert denselben Eintrag: nicht wieder nehmen
    lumi::audio::SongtextOnline online;
    online.setBenutzerOrdner(nutzer);
    SongtextAbruf stand = SongtextAbruf::Ruhe;
    int funde = 0;
    QObject::connect(&online, &lumi::audio::SongtextOnline::zustand, &online,
                     [&](const QString&, SongtextAbruf z) { stand = z; });
    QObject::connect(&online, &lumi::audio::SongtextOnline::gefunden, &online,
                     [&](const QString&, const ly::Songtext&, const QString&) { ++funde; });
    const QByteArray antwort =
        R"({"id":7,"instrumental":false,"plainLyrics":"falsch\n","syncedLyrics":null})";
    online.antwort(t, 200, -1, antwort);
    CHECK(funde == 0);
    // gesperrt: die Suche koennte eine andere Fassung kennen — zweite Stufe
    CHECK(stand == SongtextAbruf::Laedt);
    // … sie liefert nur denselben, gesperrten Eintrag: nichts
    online.antwort(t, 200, -1, "[" + antwort + "]");
    CHECK(funde == 0);
    CHECK(stand == SongtextAbruf::NichtGefunden);
    CHECK_FALSE(QFile::exists(nutzer + "/Wer - Lied.txt"));

    // freigegeben (Settings → Audio): er kommt wieder
    lumi::audio::entsperreSongtext(7, nutzer);
    CHECK_FALSE(lumi::audio::songtextGesperrt(7, nutzer));
    online.suche(TrackInfo{});  // neuer Anlauf: wieder erste Stufe
    online.antwort(t, 200, -1, antwort);
    CHECK(funde == 1);

    // eine eigene Datei (ohne Vermerk) wird nie verworfen
    schreibe(nutzer + "/eigen.lrc", kSync);
    CHECK_FALSE(lumi::audio::verwerfeSongtext(t, "", (nutzer + "/eigen.lrc").toStdString(),
                                              nutzer));
    CHECK(QFile::exists(nutzer + "/eigen.lrc"));
}

// S87, Befund Patrik: „Try (Video Version)" von „Schiller feat. Nadia Ali" fand
// der Dienst nie — ohne die Zusaetze schon. Zweite Stufe: Suche ohne sie,
// uebernommen wird nur ein Eintrag mit passender Dauer.
TEST_CASE("SongtextOnline: nicht gefunden -> Suche ohne Zusaetze, nur bei passender Dauer")
{
    using lumi::audio::SongtextAbruf;
    using lumi::audio::songtextInterpretOhneZusatz;
    using lumi::audio::songtextTitelOhneZusatz;
    CHECK(songtextTitelOhneZusatz("Try (Video Version)") == "Try");
    CHECK(songtextTitelOhneZusatz("Creep [live]") == "Creep");
    CHECK(songtextTitelOhneZusatz("Gettin' Over (Feat. Chris Willis) (Remix)") == "Gettin' Over");
    CHECK(songtextTitelOhneZusatz("Lied") == "Lied");
    CHECK(songtextTitelOhneZusatz("(Intro)") == "(Intro)");  // sonst bliebe nichts
    CHECK(songtextInterpretOhneZusatz("Schiller feat. Nadia Ali") == "Schiller");
    CHECK(songtextInterpretOhneZusatz("David Guetta ft Akon") == "David Guetta");
    CHECK(songtextInterpretOhneZusatz("Republica;Jonny Male;Dave Arch") == "Republica");
    CHECK(songtextInterpretOhneZusatz("Daft Punk") == "Daft Punk");
    CHECK(songtextInterpretOhneZusatz("Featherstone") == "Featherstone");  // kein „feat"
    CHECK(songtextInterpretOhneZusatz("Earth, Wind & Fire") == "Earth, Wind & Fire");

    QTemporaryDir tmp;
    REQUIRE(tmp.isValid());
    const QString nutzer = tmp.path() + "/nutzer";
    TrackInfo t;
    t.filePath = tmp.path() + "/x.mp3";
    t.title = "Try (Video Version)";
    t.artist = "Schiller feat. Nadia Ali";
    t.durationMs = 215000;

    lumi::audio::SongtextOnline online;
    online.setBenutzerOrdner(nutzer);
    SongtextAbruf stand = SongtextAbruf::Ruhe;
    lumi::multieffect::lyrics::Songtext fund;
    int funde = 0;
    QObject::connect(&online, &lumi::audio::SongtextOnline::zustand, &online,
                     [&](const QString&, SongtextAbruf z) { stand = z; });
    QObject::connect(&online, &lumi::audio::SongtextOnline::gefunden, &online,
                     [&](const QString&, const lumi::multieffect::lyrics::Songtext& s,
                         const QString&) {
                         ++funde;
                         fund = s;
                     });

    const QByteArray suche =
        R"([{"id":21,"trackName":"Try","artistName":"Schiller","albumName":"Remix","duration":420,)"
        R"("instrumental":false,"plainLyrics":"remix\n","syncedLyrics":null},)"
        R"({"id":22,"trackName":"Try","artistName":"Schiller","albumName":"Single","duration":214,)"
        R"("instrumental":false,"plainLyrics":"single\n","syncedLyrics":null},)"
        R"({"id":23,"trackName":"Try","artistName":"Schiller","albumName":"Album","duration":216,)"
        R"("instrumental":false,"plainLyrics":"album\n","syncedLyrics":"[00:01.00] album\n"}])";

    // 404 zum vollen Titel: nicht aufgeben, sondern ohne Zusaetze suchen
    online.antwort(t, 404, -1, QByteArray());
    CHECK(stand == SongtextAbruf::Laedt);
    CHECK(funde == 0);
    // die Suche: Remix (420 s) faellt durch, von den passenden gewinnt die synchrone
    online.antwort(t, 200, -1, suche);
    REQUIRE(funde == 1);
    CHECK(stand == SongtextAbruf::Gefunden);
    CHECK(lumi::audio::lrclibKennung(fund.herkunft) == 23);
    CHECK(fund.synchron);

    // keine Fassung mit passender Dauer: nichts uebernehmen — das entscheidet der Mensch
    TrackInfo lang = t;
    lang.filePath = tmp.path() + "/y.mp3";
    lang.durationMs = 300000;
    online.suche(TrackInfo{});
    online.antwort(lang, 404, -1, QByteArray());
    online.antwort(lang, 200, -1, suche);
    CHECK(funde == 1);
    CHECK(stand == SongtextAbruf::NichtGefunden);

    // ein Titel OHNE Zusaetze: 404 bleibt 404, keine zweite Anfrage
    TrackInfo schlicht = t;
    schlicht.title = "Try";
    schlicht.artist = "Schiller";
    online.suche(TrackInfo{});
    online.antwort(schlicht, 404, -1, QByteArray());
    CHECK(stand == SongtextAbruf::NichtGefunden);
}

// S87, Befund Patrik: derselbe Song auf zwei Alben („feat." gegen „Feat." —
// unter Windows derselbe Dateiname). Wer fuer die eine Fassung einen Text
// waehlte, ueberschrieb den der anderen; beim naechsten Laden stand wieder
// der alte da. Die Dauer in der Datei haelt die Fassungen auseinander.
TEST_CASE("SongtextQuelle: zwei Fassungen eines Songs teilen sich den Namen nicht mehr")
{
    namespace ly = lumi::multieffect::lyrics;
    QTemporaryDir tmp;
    REQUIRE(tmp.isValid());
    const QString nutzer = tmp.path() + "/nutzer";
    TrackInfo kurz;
    kurz.filePath = tmp.path() + "/a/lied.mp3";
    kurz.title = "Lied (feat. X)";
    kurz.artist = "Wer";
    kurz.durationMs = 181000;
    TrackInfo lang = kurz;
    lang.filePath = tmp.path() + "/b/lied.mp3";
    lang.title = "Lied (Feat. X)";
    lang.durationMs = 242000;

    ly::Songtext a = ly::leseLrc("[00:01.00]kurze Fassung\n");
    a.herkunft = "LRCLIB id 1";
    ly::Songtext b = ly::leseLrc("[00:01.00]lange Fassung\n");
    b.herkunft = "LRCLIB id 2";

    // die erste bekommt den gemeinsamen Namen, mit ihrer Dauer in der Datei
    const ly::Songtext ersteDatei = lumi::audio::uebernehmeSongtext(kurz, a, nutzer);
    CHECK(QFileInfo(QString::fromStdString(ersteDatei.datei)).fileName() ==
          "Wer - Lied (feat. X).lrc");
    CHECK(findeSongtext(kurz, nutzer).laengeMs == 181000);
    // fuer die zweite gilt diese Datei NICHT (61 s Unterschied) …
    CHECK(findeSongtext(lang, nutzer).leer());
    // … und ihr Text landet unter dem Namen der Fassung
    const ly::Songtext zweiteDatei = lumi::audio::uebernehmeSongtext(lang, b, nutzer);
    CHECK(QFileInfo(QString::fromStdString(zweiteDatei.datei)).fileName() ==
          "Wer - Lied (Feat. X) [4-02].lrc");
    CHECK(findeSongtext(kurz, nutzer).zeilen.at(0).text == "kurze Fassung");
    CHECK(findeSongtext(lang, nutzer).zeilen.at(0).text == "lange Fassung");

    // eine neue Wahl fuer die lange Fassung laesst die kurze unberuehrt
    ly::Songtext c = ly::leseLrc("[00:01.00]bessere lange Fassung\n");
    c.herkunft = "LRCLIB id 3";
    (void)lumi::audio::uebernehmeSongtext(lang, c, nutzer);
    CHECK(findeSongtext(kurz, nutzer).zeilen.at(0).text == "kurze Fassung");
    CHECK(findeSongtext(lang, nutzer).zeilen.at(0).text == "bessere lange Fassung");

    // gleiche Dauer (±2,5 s) = dieselbe Aufnahme: sie teilen sich den Text
    TrackInfo zwilling = kurz;
    zwilling.filePath = tmp.path() + "/c/lied.mp3";
    zwilling.durationMs = 182500;
    CHECK(findeSongtext(zwilling, nutzer).zeilen.at(0).text == "kurze Fassung");

    // eine Datei OHNE Dauerangabe (selbst angelegt, aelter) gilt fuer alle
    schreibe(nutzer + "/Wer - Anderes.lrc", kSync);
    TrackInfo x;
    x.filePath = tmp.path() + "/x.mp3";
    x.title = "Anderes";
    x.artist = "Wer";
    x.durationMs = 99000;
    CHECK(lumi::audio::songtextPasst(findeSongtext(x, nutzer), x));
    CHECK_FALSE(findeSongtext(x, nutzer).leer());
}

// --- S87: der Dienst antwortet oft mit 503 „gleich noch einmal" — ohne
// Wiederholung erschien der Text erst beim naechsten Abspielen

TEST_CASE("SongtextOnline: Wartezeit waechst, Retry-After geht vor, nach fuenf ist Schluss")
{
    using lumi::audio::songtextWartezeitMs;
    CHECK(songtextWartezeitMs(1, 1) == 2000);
    CHECK(songtextWartezeitMs(2, 1) == 5000);
    CHECK(songtextWartezeitMs(5, -1) == 40000);
    CHECK(songtextWartezeitMs(6, 1) == -1);   // aufgeben
    CHECK(songtextWartezeitMs(0, 1) == -1);
    CHECK(songtextWartezeitMs(1, 30) == 30000);  // der Dienst will laenger Ruhe
    CHECK(songtextWartezeitMs(1, 600) == -1);    // so lange wartet niemand
}

TEST_CASE("SongtextOnline: 503 setzt die Wiederholung an, die naechste Antwort liefert den Text")
{
    using lumi::audio::SongtextAbruf;
    QTemporaryDir tmp;
    REQUIRE(tmp.isValid());
    TrackInfo t;
    t.filePath = tmp.path() + "/musik/07 - Lied.mp3";
    t.title = "Lied";
    t.artist = "Wer";

    lumi::audio::SongtextOnline online;
    online.setBenutzerOrdner(tmp.path() + "/nutzer");
    std::vector<SongtextAbruf> staende;
    int funde = 0;
    QString fundPfad;
    lumi::multieffect::lyrics::Songtext fund;
    QObject::connect(&online, &lumi::audio::SongtextOnline::zustand, &online,
                     [&](const QString& pfad, SongtextAbruf s) {
                         CHECK(pfad == t.filePath);
                         staende.push_back(s);
                     });
    QObject::connect(&online, &lumi::audio::SongtextOnline::gefunden, &online,
                     [&](const QString& pfad, const lumi::multieffect::lyrics::Songtext& text,
                         const QString&) {
                         ++funde;
                         fundPfad = pfad;
                         fund = text;
                     });

    // beschaeftigt: kein Fund, aber auch kein Ende — es wird gewartet
    online.antwort(t, 503, 1, QByteArray());
    REQUIRE(staende.size() == 1);
    CHECK(staende.back() == SongtextAbruf::Wartet);
    CHECK(funde == 0);

    // die Wiederholung gelingt: Fund fuer DENSELBEN Titel, gespeichert, gemeldet
    online.antwort(t, 200, -1,
                   R"({"id":7,"instrumental":false,"plainLyrics":"eins\n",)"
                   R"("syncedLyrics":"[00:01.00] eins\n"})");
    CHECK(funde == 1);
    CHECK(fundPfad == t.filePath);
    CHECK(fund.synchron);
    CHECK(staende.back() == SongtextAbruf::Gefunden);
    CHECK(QFile::exists(tmp.path() + "/nutzer/Wer - Lied.lrc"));
}

TEST_CASE("SongtextOnline: 404 und instrumental = nicht gefunden; Dauer-503 gibt auf")
{
    using lumi::audio::SongtextAbruf;
    QTemporaryDir tmp;
    REQUIRE(tmp.isValid());
    TrackInfo t;
    t.filePath = tmp.path() + "/x.mp3";
    t.title = "Lied";
    t.artist = "Wer";

    lumi::audio::SongtextOnline online;
    online.setBenutzerOrdner(tmp.path());
    SongtextAbruf stand = SongtextAbruf::Ruhe;
    int funde = 0;
    QObject::connect(&online, &lumi::audio::SongtextOnline::zustand, &online,
                     [&](const QString&, SongtextAbruf s) { stand = s; });
    QObject::connect(&online, &lumi::audio::SongtextOnline::gefunden, &online,
                     [&](const QString&, const lumi::multieffect::lyrics::Songtext&,
                         const QString&) { ++funde; });

    online.antwort(t, 404, -1, R"({"message":"Failed to find specified track","statusCode":404})");
    CHECK(stand == SongtextAbruf::NichtGefunden);
    online.antwort(t, 200, -1, R"({"instrumental":true,"plainLyrics":null,"syncedLyrics":null})");
    CHECK(stand == SongtextAbruf::NichtGefunden);
    online.antwort(t, 0, -1, QByteArray());  // kein Netz
    CHECK(stand == SongtextAbruf::Fehlgeschlagen);

    // fuenf Wiederholungen werden angesetzt, die sechste Absage beendet die Suche
    for (int i = 0; i < 5; ++i)
    {
        online.antwort(t, 503, 1, QByteArray());
        CHECK(stand == SongtextAbruf::Wartet);
    }
    online.antwort(t, 503, 1, QByteArray());
    CHECK(stand == SongtextAbruf::Fehlgeschlagen);
    CHECK(funde == 0);
    // kein Songtext gespeichert — nur der Merker „online nichts gefunden" (S88)
    CHECK(QDir(tmp.path()).entryList(QDir::Files) == QStringList{"notfound.json"});
}

// S88: bis S87 loeste ein Titel ohne Text bei jedem Abspielen eine Anfrage aus.
TEST_CASE("SongtextOnline: Merker — nichts gefunden wird eine Weile nicht wieder gefragt")
{
    using lumi::audio::SongtextAbruf;
    using lumi::audio::songtextFehlanzeige;
    QTemporaryDir tmp;
    REQUIRE(tmp.isValid());
    const QString nutzer = tmp.path() + "/nutzer";
    const QDate tag(2026, 10, 2);
    TrackInfo t;
    t.filePath = tmp.path() + "/x.mp3";
    t.title = "Lied";
    t.artist = "Wer";
    t.album = "Platte";
    t.durationMs = 200000;

    // Gegenprobe: ohne Merker gilt nichts, und eine Datei gibt es nicht
    CHECK_FALSE(songtextFehlanzeige(t, nutzer, tag));
    CHECK_FALSE(QFile::exists(nutzer + "/notfound.json"));

    // nicht gefunden: 7 Tage; Schreibweise und Leerraum zaehlen nicht
    lumi::audio::merkeSongtextFehlanzeige(t, false, nutzer, tag);
    CHECK(songtextFehlanzeige(t, nutzer, tag));
    TrackInfo gross = t;
    gross.title = " LIED ";
    CHECK(songtextFehlanzeige(gross, nutzer, tag));
    CHECK(songtextFehlanzeige(t, nutzer, tag.addDays(6)));
    CHECK_FALSE(songtextFehlanzeige(t, nutzer, tag.addDays(7)));
    CHECK_FALSE(songtextFehlanzeige(t, nutzer, tag.addDays(-1)));  // Uhr zurueckgestellt
    // eine andere Fassung (Dauer, Album) ist ein anderer Titel
    TrackInfo andere = t;
    andere.durationMs = 260000;
    CHECK_FALSE(songtextFehlanzeige(andere, nutzer, tag));
    andere = t;
    andere.album = "Andere Platte";
    CHECK_FALSE(songtextFehlanzeige(andere, nutzer, tag));

    // instrumental: 30 Tage; der neue Merker ersetzt den alten
    lumi::audio::merkeSongtextFehlanzeige(t, true, nutzer, tag);
    CHECK(songtextFehlanzeige(t, nutzer, tag.addDays(29)));
    CHECK_FALSE(songtextFehlanzeige(t, nutzer, tag.addDays(30)));

    // Abgelaufenes faellt beim naechsten Schreiben aus der Datei
    TrackInfo spaeter = t;
    spaeter.title = "Spaeter";
    lumi::audio::merkeSongtextFehlanzeige(spaeter, false, nutzer, tag.addDays(40));
    CHECK(lumi::audio::vergissSongtextFehlanzeigen(nutzer, tag.addDays(40)) == 1);
    CHECK_FALSE(QFile::exists(nutzer + "/notfound.json"));
    CHECK_FALSE(songtextFehlanzeige(spaeter, nutzer, tag.addDays(40)));

    // der Abruf setzt den Merker selbst: 404, instrumental, keine passende Dauer
    lumi::audio::SongtextOnline online;
    online.setBenutzerOrdner(nutzer);
    SongtextAbruf stand = SongtextAbruf::Ruhe;
    QObject::connect(&online, &lumi::audio::SongtextOnline::zustand, &online,
                     [&](const QString&, SongtextAbruf s) { stand = s; });
    online.antwort(t, 404, -1, QByteArray());
    CHECK(songtextFehlanzeige(t, nutzer));
    CHECK_FALSE(songtextFehlanzeige(t, nutzer, QDate::currentDate().addDays(7)));
    (void)lumi::audio::vergissSongtextFehlanzeigen(nutzer);
    online.antwort(t, 200, -1, R"({"instrumental":true,"plainLyrics":null,"syncedLyrics":null})");
    CHECK(songtextFehlanzeige(t, nutzer, QDate::currentDate().addDays(29)));
    (void)lumi::audio::vergissSongtextFehlanzeigen(nutzer);
    TrackInfo zusatz = t;
    zusatz.title = "Lied (Video Version)";
    online.suche(TrackInfo{});
    online.antwort(zusatz, 404, -1, QByteArray());  // -> zweite Stufe
    CHECK_FALSE(songtextFehlanzeige(zusatz, nutzer));  // noch nicht entschieden
    online.antwort(zusatz, 200, -1, "[]");
    CHECK(stand == SongtextAbruf::NichtGefunden);
    CHECK(songtextFehlanzeige(zusatz, nutzer));

    // kein Merker bei „beschaeftigt" und bei Netzfehlern — das sagt nichts ueber den Titel
    (void)lumi::audio::vergissSongtextFehlanzeigen(nutzer);
    online.suche(TrackInfo{});
    online.antwort(t, 503, 1, QByteArray());
    online.antwort(t, 0, -1, QByteArray());
    CHECK(stand == SongtextAbruf::Fehlgeschlagen);
    CHECK_FALSE(songtextFehlanzeige(t, nutzer));

    // eine Freigabe in der Sperrliste vergisst die Merker (sie soll sofort wirken)
    lumi::audio::sperreSongtext(7, "Wer - Lied", nutzer);
    lumi::audio::merkeSongtextFehlanzeige(t, false, nutzer);
    lumi::audio::entsperreSongtext(7, nutzer);
    CHECK_FALSE(songtextFehlanzeige(t, nutzer));
}

// Der Weg ueber `suche` braucht die Einstellung „Search lyrics online" — in
// abgeschotteten QSettings, die echten bleiben unberuehrt.
TEST_CASE("SongtextOnline: Merker — suche() fragt den gemerkten Titel nicht an")
{
    using lumi::audio::SongtextAbruf;
    const QString org = QCoreApplication::organizationName();
    const QString app = QCoreApplication::applicationName();
    QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
    QCoreApplication::setApplicationName(QStringLiteral("SongtextMerker"));
    QSettings().clear();
    QSettings().setValue(QStringLiteral("lyrics/onlineSearch"), true);
    {
        QTemporaryDir tmp;
        REQUIRE(tmp.isValid());
        TrackInfo t;
        t.filePath = tmp.path() + "/x.mp3";
        t.title = "Lied";
        t.artist = "Wer";

        lumi::audio::SongtextOnline online;
        online.setBenutzerOrdner(tmp.path());
        SongtextAbruf stand = SongtextAbruf::Ruhe;
        QObject::connect(&online, &lumi::audio::SongtextOnline::zustand, &online,
                         [&](const QString&, SongtextAbruf s) { stand = s; });

        // Gegenprobe: ohne Merker setzt suche() die Anfrage an (Atempause laeuft)
        online.suche(t);
        CHECK(stand == SongtextAbruf::Laedt);
        online.suche(TrackInfo{});  // abbrechen, bevor etwas hinausgeht
        CHECK(stand == SongtextAbruf::Ruhe);

        lumi::audio::merkeSongtextFehlanzeige(t, false, tmp.path());
        online.suche(t);
        CHECK(stand == SongtextAbruf::NichtGefunden);
        online.suche(TrackInfo{});
    }
    QSettings().clear();
    QCoreApplication::setOrganizationName(org);
    QCoreApplication::setApplicationName(app);
}
