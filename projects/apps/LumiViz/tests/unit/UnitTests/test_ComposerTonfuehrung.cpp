/**
 ****************************************************************************************
 * @file   test_ComposerTonfuehrung.cpp
 * @brief  Composer, Stufe C5b: die Audio-Kanaele im Mixer — die Uhr fuehrt
 *         (docs/ui/Composer_Konzept.md §4 und §8)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Kein Fenster, kein Ton: die Fuehrung laeuft gegen eine Audio-Engine mit Mixer
 * und einen Player, beide aus Pappe. Die Engine schreibt mit, was an ihren
 * Quellen geschieht; ihre Zeit dreht der Test von Hand weiter. Geprueft wird:
 * die Uhr fuehrt und die Quellen folgen ihr, die Songs ueberlagern sich mit
 * ihren Pegeln, und der laufende Song ist der mit dem hoechsten Wert der
 * Pegelkurve.
 *
 * Rueckbau (Konzept §12): diese Datei loeschen, Zeile in Source.cmake nehmen.
 ****************************************************************************************
 */

#include <doctest.h>

#include "PappTon.hpp"
#include "composer/Bearbeiten.hpp"
#include "composer/Komposition.hpp"
#include "composer/Tonfuehrung.hpp"

#include <QJsonArray>
#include <QMap>
#include <QStringList>

#include <algorithm>
#include <cmath>
#include <limits>

using namespace lumi::composer;
using papp::PappEngine;
using papp::PappMischEngine;
using papp::PappPlayer;

namespace
{

const QString kEins = QStringLiteral("C:/musik/eins.mp3");
const QString kZwei = QStringLiteral("C:/musik/zwei.mp3");
const QString kFehlt = QStringLiteral("C:/musik/fehlt.mp3");

/// CH1 leer · A1: eins 10–70 s (ab 5 s im Song), zwei 80–100 s.
Komposition probe()
{
    Komposition k = neueKomposition();
    (void)fuegeKanalEin(k, {}, KanalArt::Audio);
    REQUIRE(fuegeSongEin(k, 1, kEins, 10000, 65000) == 0);
    k.kanaele[1].clips[0].einsatzMs = 5000;
    k.kanaele[1].clips[0].endeMs = 70000;
    REQUIRE(fuegeSongEin(k, 1, kZwei, 80000, 20000) == 1);
    return k;
}

/// Uhr und Mixer laufen `ms` weiter — in Takten von 33 ms, wie die Sitzung taktet.
Tonfuehrung::Ergebnis laufe(Tonfuehrung& f, PappMischEngine& e, const Komposition& k, std::int64_t& stelle,
                            int ms)
{
    Tonfuehrung::Ergebnis letztes;
    for (int rest = ms; rest > 0; rest -= 33)
    {
        const int schritt = std::min(33, rest);
        stelle += schritt;
        e.vor(schritt);
        letztes = f.takt(k, stelle, true);
    }
    return letztes;
}

}  // namespace

TEST_SUITE("ComposerTonfuehrung")
{
    TEST_CASE("Uebernehmen: ein Mixer, der Player schweigt, die Playlist ruht — Freigeben haengt sie wieder ein")
    {
        PappPlayer p;
        p.laengen = {{kEins, 300000}};
        // nie benutzt, nur weitergereicht: irgendeine Adresse genuegt als Playlist
        int platz = 0;
        auto* liste = reinterpret_cast<IPlaylist*>(&platz);
        p.setPlaylist(liste);
        REQUIRE(p.load(kEins));
        p.play();
        PappMischEngine e;
        {
            Tonfuehrung f;
            CHECK_FALSE(f.fuehrt());
            CHECK(f.mixer() == INVALID_STREAM);
            CHECK_FALSE(f.takt(probe(), 0, true).hatSong);  // ohne Engine tut ein Takt nichts
            REQUIRE(f.uebernimm(e, p));
            CHECK(f.fuehrt());
            CHECK(f.mixer() == e.mixer);
            CHECK(f.mixer() != INVALID_STREAM);
            CHECK(p.playlist() == nullptr);
            CHECK(p.isPaused());  // der Player schweigt
            CHECK(f.uebernimm(e, p));  // noch einmal: kein zweiter Mixer, die gemerkte Playlist bleibt die echte
            CHECK(e.zahl(QStringLiteral("mixer")) == 1);
            const AudioStreamHandle mixer = f.mixer();
            f.gibFrei();
            CHECK_FALSE(f.fuehrt());
            CHECK(p.playlist() == liste);
            CHECK(e.stream(mixer).frei);
            CHECK(p.isPaused());  // ohne den Wunsch bleibt er angehalten
            // mit dem Wunsch spielt er weiter — wenn er beim Uebernehmen spielte
            p.play();
            REQUIRE(f.uebernimm(e, p));
            CHECK(p.isPaused());
            f.gibFrei(true);
            CHECK(p.isPlaying());
            p.pause();
            REQUIRE(f.uebernimm(e, p));  // er spielte nicht: er bleibt auch mit dem Wunsch angehalten
            f.gibFrei(true);
            CHECK(p.isPaused());
            REQUIRE(f.uebernimm(e, p));
            CHECK(p.playlist() == nullptr);
        }
        CHECK(p.playlist() == liste);  // auch wenn die Fuehrung einfach verschwindet
        CHECK(e.zahl(QStringLiteral("free mixer")) == 4);
        CHECK(p.isPaused());  // verschwindet die Fuehrung einfach, startet sie den Player nicht

        // eine Engine ohne Mixer: die Fuehrung uebernimmt nichts, alles bleibt
        PappEngine ohne;
        Tonfuehrung g;
        CHECK_FALSE(g.uebernimm(ohne, p));
        CHECK_FALSE(g.fuehrt());
        CHECK(p.playlist() == liste);
    }

    TEST_CASE("Luecke, Vorlauf, Einstieg in einen Song, die Uhr fuehrt, Ende des Clips")
    {
        PappPlayer p;
        PappMischEngine e;
        e.laengen = {{kEins, 300000}, {kZwei, 300000}};
        const Komposition k = probe();
        Tonfuehrung f;
        REQUIRE(f.uebernimm(e, p));
        std::int64_t stelle = 0;

        // vor dem ersten Song: der Mixer laeuft und spielt Stille
        CHECK_FALSE(f.takt(k, stelle, true).hatSong);
        CHECK(e.stream(e.mixer).spielt);
        CHECK_FALSE(laufe(f, e, k, stelle, 5000).hatSong);
        CHECK(f.quellen() == 0);
        CHECK(e.protokoll == QStringList{QStringLiteral("mixer")});

        // zwei Sekunden vor dem Clip: die Datei ist geoeffnet, steht am Einsatz und haelt
        CHECK_FALSE(laufe(f, e, k, stelle, 3100).hatSong);
        REQUIRE(f.quellen() == 1);
        const AudioStreamHandle a = e.quelle(kEins);
        REQUIRE(a != INVALID_STREAM);
        CHECK(e.stream(a).haelt);
        CHECK(e.stream(a).pos == 5000);

        // am Anfang des Clips: an die Stelle der Uhr gesetzt, und sie laeuft
        CHECK(laufe(f, e, k, stelle, 1933).hatSong);
        CHECK_FALSE(e.stream(a).haelt);
        CHECK(e.stream(a).pos == 5000 + (stelle - 10000));
        CHECK(e.zahl(QStringLiteral("add")) == 1);
        CHECK(e.zahl(QStringLiteral("run")) == 1);
        const int gesetzt = e.zahl(QStringLiteral("seek"));

        // beide laufen gleich schnell: nichts wird nachgesetzt
        (void)laufe(f, e, k, stelle, 10000);
        CHECK(e.zahl(QStringLiteral("seek")) == gesetzt);
        CHECK(f.nachgesetzt() == 0);

        // die Quelle laeuft der Uhr davon: die QUELLE wird nachgesetzt, die Uhr bleibt
        e.verschiebe(a, 200);
        const std::int64_t davor = stelle;
        (void)laufe(f, e, k, stelle, 33);
        CHECK(stelle == davor + 33);
        CHECK(f.nachgesetzt() == 1);
        CHECK(e.stream(a).pos == 5000 + (stelle - 10000));
        // Ein stetiger Abstand gleich nach dem Setzen ist die Latenz der Ausgabe (was die Engine
        // als „zu hoeren" meldet, liegt hinter der Uhr) — er gilt als Grundabstand, kein Nachsetzen
        e.verschiebe(a, -45);
        (void)laufe(f, e, k, stelle, 66);
        CHECK(f.nachgesetzt() == 1);  // in der Schonfrist wird nicht geprueft
        (void)laufe(f, e, k, stelle, 2000);
        CHECK(f.nachgesetzt() == 1);
        CHECK(e.stream(a).pos == 5000 + (stelle - 10000) - 45);
        // eine kleine Abweichung davon bleibt
        e.verschiebe(a, 50);
        (void)laufe(f, e, k, stelle, 1000);
        CHECK(f.nachgesetzt() == 1);
        // eine grosse wird nachgesetzt — gemessen am Grundabstand, nicht an null
        e.verschiebe(a, -140);  // jetzt 90 ms hinter dem Grundabstand von -45
        (void)laufe(f, e, k, stelle, 33);
        CHECK(f.nachgesetzt() == 2);
        CHECK(e.stream(a).pos == 5000 + (stelle - 10000));

        // am Ende des Clips geht die Quelle aus dem Mixer, obwohl der Song weiterginge
        CHECK_FALSE(laufe(f, e, k, stelle, static_cast<int>(70000 - stelle) + 40).hatSong);
        CHECK(e.quelle(kEins) == INVALID_STREAM);
        CHECK(e.zahl(QStringLiteral("remove ") + kEins) == 1);
        CHECK(e.stream(e.mixer).spielt);  // der Mixer laeuft in der Luecke weiter
        (void)laufe(f, e, k, stelle, 5000);
        CHECK(f.quellen() == 0);

        // der zweite Song: von vorn
        CHECK(laufe(f, e, k, stelle, static_cast<int>(80000 - stelle) + 40).hatSong);
        const AudioStreamHandle b = e.quelle(kZwei);
        REQUIRE(b != INVALID_STREAM);
        CHECK_FALSE(e.stream(b).haelt);
        CHECK(e.stream(b).pos == stelle - 80000);
        // der Player wurde bei alledem nicht angefasst
        CHECK(p.protokoll.isEmpty());
    }

    TEST_CASE("Springt die Uhr, folgen die Quellen — haelt sie, haelt der Mixer")
    {
        PappPlayer p;
        PappMischEngine e;
        e.laengen = {{kEins, 300000}, {kZwei, 300000}};
        const Komposition k = probe();
        Tonfuehrung f;
        REQUIRE(f.uebernimm(e, p));
        std::int64_t stelle = 20000;
        CHECK(f.stelleImLaufenden(k, 0.5) == -1);  // noch kein laufender Song
        Tonfuehrung::Ergebnis r = f.takt(k, stelle, true);  // mitten hinein: der Song steht bei 15 s
        const AudioStreamHandle a = e.quelle(kEins);
        REQUIRE(a != INVALID_STREAM);
        CHECK(e.stream(a).pos == 15000);
        CHECK_FALSE(e.stream(a).haelt);
        // der laufende Song: gemeldet mit seinen Tags (ohne Tag der Dateiname)
        CHECK(r.hatSong);
        CHECK(r.songGewechselt);
        CHECK(r.song.filePath == kEins);
        CHECK(r.song.title == QStringLiteral("eins"));
        CHECK(r.song.durationMs == 300000);
        CHECK(r.songStelleMs == 15000);
        CHECK(r.songDauerMs == 300000);
        CHECK(r.songLaeuft);
        // ein Klick in den Balken des Player-Panels: ein Anteil der Songlaenge wird zur Stelle
        // der Komposition (Clip 10–70 s, ab 5 s im Song) — und bleibt im Clip
        CHECK(f.stelleImLaufenden(k, 0.1) == 35000);   // 30 s im Song
        CHECK(f.stelleImLaufenden(k, 0.0) == 10000);   // vor dem Einsatz: der Clip-Anfang
        CHECK(f.stelleImLaufenden(k, 0.9) == 69999);   // hinter dem Clip: sein Ende
        {
            Komposition ohne = k;  // der Clip ist nicht mehr da
            ohne.kanaele[1].clips.clear();
            CHECK(f.stelleImLaufenden(ohne, 0.1) == -1);
        }
        r = laufe(f, e, k, stelle, 1000);
        CHECK_FALSE(r.songGewechselt);  // derselbe Song wird nicht noch einmal gemeldet
        CHECK(r.songStelleMs == 5000 + (stelle - 10000));

        // ein Klick ins Lineal des Composers: die Uhr springt, die Quelle wird gesetzt
        stelle = 30000;
        (void)f.takt(k, stelle, true);
        CHECK(e.stream(a).pos == 25000);
        CHECK(e.stream(e.mixer).spielt);
        // ... auch rueckwaerts um ein kleines Stueck (Schleife, Stop)
        (void)laufe(f, e, k, stelle, 500);
        stelle -= 400;
        (void)f.takt(k, stelle, true);
        CHECK(e.stream(a).pos == stelle - 10000 + 5000);
        CHECK(f.nachgesetzt() == 0);  // ein Sprung der Uhr ist kein Davonlaufen

        // die Uhr haelt: der Mixer haelt — und mit ihm alles, was in ihm laeuft
        r = f.takt(k, stelle, false);
        CHECK_FALSE(e.stream(e.mixer).spielt);
        CHECK(r.hatSong);
        CHECK_FALSE(r.songLaeuft);
        const int stand = e.stream(a).pos;
        e.vor(1000);
        CHECK(e.stream(a).pos == stand);
        // ein Klick ins Lineal bei stehender Uhr: gesetzt, weiter gehalten
        stelle = 50000;
        (void)f.takt(k, stelle, false);
        CHECK(e.stream(a).pos == 45000);
        CHECK_FALSE(e.stream(e.mixer).spielt);

        // die Uhr laeuft wieder: der Mixer spielt weiter, von dort
        r = f.takt(k, stelle, true);
        CHECK(e.stream(e.mixer).spielt);
        CHECK(e.stream(a).pos == 45000);
        CHECK(r.songLaeuft);

        // vom Song in die Luecke gesprungen und zurueck in denselben Song
        stelle = 75000;
        CHECK_FALSE(f.takt(k, stelle, true).hatSong);
        CHECK(f.quellen() == 0);
        stelle = 15000;
        r = f.takt(k, stelle, true);
        REQUIRE(e.quelle(kEins) != INVALID_STREAM);
        CHECK(e.stream(e.quelle(kEins)).pos == 10000);
        CHECK(r.hatSong);
        CHECK_FALSE(r.songGewechselt);  // es ist derselbe Song wie vor der Luecke
    }

    TEST_CASE("Schleife S99: im Fenster spielen die Songs beider Stellen — ein Song in beiden bekommt eine zweite Quelle")
    {
        // Die Uhr der Schleife, wie `Uhr` sie rechnet: Laenge 60 s, Ueberlappung 10 s, Umlauf 50 s.
        constexpr std::int64_t kUmlauf = 50000;
        constexpr std::int64_t kFenster = 10000;
        struct Lage
        {
            std::int64_t stelle;
            Schleifenlage schleife;
        };
        const auto lage = [&](std::int64_t roh) {
            const std::int64_t stelle = roh % kUmlauf;
            Lage l{stelle, {}};
            l.schleife.zweiteMs = roh >= kUmlauf && stelle < kFenster ? stelle + kUmlauf : -1;
            l.schleife.umschlagInMs = kUmlauf - stelle;
            return l;
        };
        const auto laufeRund = [&](Tonfuehrung& f, PappMischEngine& e, const Komposition& k, std::int64_t& roh,
                                   std::int64_t bis) {
            Tonfuehrung::Ergebnis letztes;
            while (roh < bis)
            {
                const int schritt = static_cast<int>(std::min<std::int64_t>(33, bis - roh));
                roh += schritt;
                e.vor(schritt);
                const Lage l = lage(roh);
                letztes = f.takt(k, l.stelle, true, l.schleife);
            }
            return letztes;
        };
        const auto nah = [](double ist, double soll) { return std::abs(ist - soll) < 0.02; };

        // --- ein Song ueber die ganze Laenge, 10 s ein- und ausgeblendet
        Komposition k = neueKomposition();
        (void)fuegeKanalEin(k, {}, KanalArt::Audio);
        REQUIRE(fuegeSongEin(k, 1, kEins, 0, 60000) == 0);
        k.kanaele[1].clips[0].setzeBlenden(10000, 10000);
        k.ueberlappungMs = kFenster;
        REQUIRE(k.umlaufMs() == kUmlauf);
        REQUIRE(k.songsDoppelt);

        PappPlayer p;
        PappMischEngine e;
        e.laengen = {{kEins, 300000}, {kZwei, 300000}};
        Tonfuehrung f;
        REQUIRE(f.uebernimm(e, p));
        std::int64_t roh = 40000;  // erste Runde, mitten im Song
        Lage l = lage(roh);
        Tonfuehrung::Ergebnis r = f.takt(k, l.stelle, true, l.schleife);
        CHECK(r.songGewechselt);
        REQUIRE(e.quellenVon(kEins).size() == 1);
        const AudioStreamHandle erste = e.quellenVon(kEins)[0];
        CHECK(e.stream(erste).pos == 40000);

        // weit vor dem Umschlag bleibt es bei einer Quelle
        (void)laufeRund(f, e, k, roh, 47000);
        CHECK(e.quellenVon(kEins).size() == 1);
        // im Vorlauf vor dem Umschlag: die zweite ist geoeffnet, steht am Anfang und haelt
        (void)laufeRund(f, e, k, roh, 49000);
        REQUIRE(e.quellenVon(kEins).size() == 2);
        const AudioStreamHandle zweite = e.quellenVon(kEins)[1];
        CHECK(zweite != erste);
        CHECK(e.stream(zweite).haelt);
        CHECK(e.stream(zweite).pos == 0);
        CHECK(e.stream(erste).pos == 49000);
        const int gesetztVorher = e.zahl(QStringLiteral("seek"));

        // der Umschlag: die erste spielt ihr Ende weiter — ohne gesetzt zu werden —, die zweite beginnt
        r = laufeRund(f, e, k, roh, 55000);
        REQUIRE(e.quellenVon(kEins).size() == 2);
        CHECK(e.stream(erste).pos == 55000);
        CHECK_FALSE(e.stream(erste).haelt);
        CHECK(e.stream(zweite).pos == 5000);
        CHECK_FALSE(e.stream(zweite).haelt);
        // gesetzt wurde nur die zweite, einmal, bei ihrem Einsatz
        CHECK(e.zahl(QStringLiteral("seek")) == gesetztVorher + 1);
        CHECK(f.nachgesetzt() == 0);
        // jede mit ihrem Pegel aus der Kurve: das Ende blendet aus, der Anfang ein
        CHECK(nah(e.stream(erste).lautstaerke, 0.5));
        CHECK(nah(e.stream(zweite).lautstaerke, 0.5));
        // derselbe Song laeuft weiter: kein Songwechsel am Umschlag
        CHECK(r.hatSong);
        CHECK_FALSE(r.songGewechselt);
        // der laufende ist die Stimme mit dem hoeheren Pegel: spaeter im Fenster der Anfang
        r = laufeRund(f, e, k, roh, 58000);
        CHECK(nah(e.stream(erste).lautstaerke, 0.2));
        CHECK(nah(e.stream(zweite).lautstaerke, 0.8));
        CHECK(r.songStelleMs == 8000);

        // hinter dem Fenster: das Ende ist aus dem Mixer, der Anfang laeuft ungestoert weiter
        const int gesetztImFenster = e.zahl(QStringLiteral("seek"));
        r = laufeRund(f, e, k, roh, 62000);
        REQUIRE(e.quellenVon(kEins).size() == 1);
        CHECK(e.quellenVon(kEins)[0] == zweite);
        CHECK(e.stream(zweite).pos == 12000);
        CHECK(e.zahl(QStringLiteral("seek")) == gesetztImFenster);
        CHECK(e.zahl(QStringLiteral("remove ") + kEins) == 1);
        CHECK_FALSE(r.songGewechselt);

        // --- derselbe Song OHNE zweite Quelle (Songs overlap themselves aus): er spielt sein
        //     Ende aus und springt hinter dem Fenster im Song zurueck
        Komposition einfach = k;
        einfach.songsDoppelt = false;
        PappMischEngine e2;
        e2.laengen = {{kEins, 300000}};
        Tonfuehrung g;
        REQUIRE(g.uebernimm(e2, p));
        roh = 47000;
        l = lage(roh);
        (void)g.takt(einfach, l.stelle, true, l.schleife);
        (void)laufeRund(g, e2, einfach, roh, 55000);
        REQUIRE(e2.quellenVon(kEins).size() == 1);  // auch im Vorlauf und im Fenster nur eine
        const AudioStreamHandle eine = e2.quellenVon(kEins)[0];
        CHECK(e2.stream(eine).pos == 55000);         // sie spielt das Ende
        CHECK(nah(e2.stream(eine).lautstaerke, 0.5));
        (void)laufeRund(g, e2, einfach, roh, 60033);
        CHECK(e2.stream(eine).pos == 10033);         // hinter dem Fenster: zurueck an die vordere Stelle
        CHECK(e2.quellenVon(kEins).size() == 1);

        // --- zwei verschiedene Songs: eins am Anfang (0–20 s), zwei am Ende (40–60 s)
        Komposition zwei = neueKomposition();
        (void)fuegeKanalEin(zwei, {}, KanalArt::Audio);
        REQUIRE(fuegeSongEin(zwei, 1, kEins, 0, 20000) == 0);
        REQUIRE(fuegeSongEin(zwei, 1, kZwei, 40000, 20000) == 1);
        zwei.ueberlappungMs = kFenster;
        PappMischEngine e3;
        e3.laengen = {{kEins, 300000}, {kZwei, 300000}};
        Tonfuehrung h;
        REQUIRE(h.uebernimm(e3, p));
        roh = 45000;
        l = lage(roh);
        r = h.takt(zwei, l.stelle, true, l.schleife);
        CHECK(r.song.filePath == kZwei);
        (void)laufeRund(h, e3, zwei, roh, 49000);
        REQUIRE(e3.quelle(kEins) != INVALID_STREAM);  // der erste Song der naechsten Runde wartet
        CHECK(e3.stream(e3.quelle(kEins)).haelt);
        const int gesetztZwei = e3.zahl(QStringLiteral("seek ") + kZwei);
        r = laufeRund(h, e3, zwei, roh, 55000);
        CHECK(e3.stream(e3.quelle(kZwei)).pos == 15000);  // zwei spielt im Fenster weiter ...
        CHECK(e3.zahl(QStringLiteral("seek ") + kZwei) == gesetztZwei);  // ... ohne neu gesetzt zu werden
        CHECK(e3.stream(e3.quelle(kEins)).pos == 5000);   // ... und eins spielt daneben
        CHECK_FALSE(e3.stream(e3.quelle(kEins)).haelt);
        CHECK(e3.quellenZahl() == 2);
        (void)laufeRund(h, e3, zwei, roh, 61000);
        CHECK(e3.quelle(kZwei) == INVALID_STREAM);  // hinter dem Fenster ist zwei aus dem Mixer
        CHECK(e3.quellenZahl() == 1);

        // --- die Blende der GANZEN Komposition gilt auch fuer den Ton: dieselben zwei Songs auf
        //     vollem Pegel, die Komposition blendet 10 s ein und 10 s aus
        Komposition blende = zwei;
        blende.einblendeMs = 10000;
        blende.ausblendeMs = 10000;
        PappMischEngine e4;
        e4.laengen = {{kEins, 300000}, {kZwei, 300000}};
        Tonfuehrung i;
        REQUIRE(i.uebernimm(e4, p));
        // ohne Schleife: am Anfang herauf, in der Mitte voll, am Ende herunter
        (void)i.takt(blende, 2500, true);
        CHECK(nah(e4.stream(e4.quelle(kEins)).lautstaerke, 0.25));
        (void)i.takt(blende, 15000, true);
        CHECK(nah(e4.stream(e4.quelle(kEins)).lautstaerke, 1.0));
        (void)i.takt(blende, 57500, true);
        CHECK(nah(e4.stream(e4.quelle(kZwei)).lautstaerke, 0.25));
        // im Fenster der Schleife ueberblenden sie: vorn 2,5 s (eins 0,25), hinten 52,5 s (zwei 0,75)
        r = i.takt(blende, 2500, true, Schleifenlage{52500, kUmlauf - 2500});
        CHECK(nah(e4.stream(e4.quelle(kEins)).lautstaerke, 0.25));
        CHECK(nah(e4.stream(e4.quelle(kZwei)).lautstaerke, 0.75));
        CHECK(r.songStelleMs == 12500);  // der laufende Song ist noch zwei (der lautere), 12,5 s im Song
        // nach der Mitte des Fensters ist es eins
        r = i.takt(blende, 7500, true, Schleifenlage{57500, kUmlauf - 7500});
        CHECK(r.songGewechselt);
        CHECK(r.song.filePath == kEins);
    }

    TEST_CASE("Ein Song, der vor seinem Clip endet, und einer, der sich nicht oeffnen laesst")
    {
        PappPlayer p;
        PappMischEngine e;
        e.laengen = {{kEins, 300000}, {kZwei, 8000}};  // zwei ist kuerzer als sein Clip von 20 s
        Komposition k = probe();
        REQUIRE(fuegeSongEin(k, 1, kFehlt, 110000, 10000) == 2);
        Tonfuehrung f;
        REQUIRE(f.uebernimm(e, p));
        std::int64_t stelle = 80000;
        CHECK(f.takt(k, stelle, true).songLaeuft);
        const AudioStreamHandle b = e.quelle(kZwei);
        REQUIRE(b != INVALID_STREAM);

        // nach 8 s ist der Song aus: die Uhr laeuft weiter, die Quelle haelt an ihrem Ende
        Tonfuehrung::Ergebnis r = laufe(f, e, k, stelle, 9000);
        CHECK(stelle == 89000);
        CHECK(r.hatSong);
        CHECK_FALSE(r.songLaeuft);
        CHECK(e.stream(b).haelt);
        CHECK(e.stream(b).pos == 8000);
        const int gesetzt = e.zahl(QStringLiteral("seek"));
        (void)laufe(f, e, k, stelle, 5000);
        CHECK(e.zahl(QStringLiteral("seek")) == gesetzt);  // niemand setzt sie hinter ihr Ende
        CHECK(f.nachgesetzt() == 0);
        // ein Sprung der Uhr zurueck in den Song: sie spielt wieder
        stelle = 82000;
        r = f.takt(k, stelle, true);
        CHECK(r.songLaeuft);
        CHECK(e.stream(b).pos == 2000);
        CHECK_FALSE(e.stream(b).haelt);

        // eine Datei, die fehlt: ein Versuch, dann Stille bis zum Ende des Clips
        stelle = 110000;
        CHECK_FALSE(f.takt(k, stelle, true).hatSong);
        CHECK_FALSE(laufe(f, e, k, stelle, 3000).hatSong);
        CHECK(e.zahl(QStringLiteral("add ") + kFehlt) == 1);
        CHECK(f.quellen() == 0);
    }

    TEST_CASE("Der Pegel des Clips ist die Lautstaerke seiner Quelle — die des Players gilt fuer den Mixer")
    {
        PappPlayer p;
        PappMischEngine e;
        e.laengen = {{kEins, 300000}, {kZwei, 300000}};
        Komposition k = probe();
        k.kanaele[1].clips[0].setzeBlenden(2000, 4000);
        k.kanaele[1].clips[0].grundpegel = 0.8;
        Tonfuehrung f;
        REQUIRE(f.uebernimm(e, p));
        (void)f.takt(k, 11000, true);  // eine von zwei Sekunden Einblendung
        const AudioStreamHandle a = e.quelle(kEins);
        REQUIRE(a != INVALID_STREAM);
        CHECK(e.stream(a).lautstaerke == doctest::Approx(0.4f));
        (void)f.takt(k, 30000, true);
        CHECK(e.stream(a).lautstaerke == doctest::Approx(0.8f));
        (void)f.takt(k, 68000, true);  // zwei Sekunden vor Schluss
        CHECK(e.stream(a).lautstaerke == doctest::Approx(0.4f));
        (void)f.takt(k, 85000, true);  // der zweite Song: voller Pegel
        REQUIRE(e.quelle(kZwei) != INVALID_STREAM);
        CHECK(e.stream(e.quelle(kZwei)).lautstaerke == doctest::Approx(1.0f));

        // Lautstaerke und Mute des Players wirken auf den ganzen Ton der Komposition
        CHECK(e.stream(e.mixer).lautstaerke == doctest::Approx(1.0f));
        p.laut = 0.5f;
        (void)f.takt(k, 85033, true);
        CHECK(e.stream(e.mixer).lautstaerke == doctest::Approx(0.5f));
        CHECK(e.stream(e.quelle(kZwei)).lautstaerke == doctest::Approx(1.0f));  // der Pegel der Quelle bleibt
        p.stumm = true;
        (void)f.takt(k, 85066, true);
        CHECK(e.stream(e.mixer).lautstaerke == doctest::Approx(0.0f));
        p.stumm = false;
        (void)f.takt(k, 85100, true);
        CHECK(e.stream(e.mixer).lautstaerke == doctest::Approx(0.5f));

        // spielt der Player doch (ein Doppelklick im Audio Browser), haelt ihn der naechste Takt an
        p.laengen = {{kEins, 300000}};
        REQUIRE(p.load(kEins));
        p.play();
        CHECK(f.takt(k, 85133, true).playerAngehalten);
        CHECK(p.isPaused());
        CHECK_FALSE(f.takt(k, 85166, true).playerAngehalten);

        // S101: der Audio-Master ist ein weiterer Faktor am Pegel der Quelle — der Visual-Master nicht
        k.bildKurve.grundpegel = 0.1;
        (void)f.takt(k, 85200, true);
        CHECK(e.stream(e.quelle(kZwei)).lautstaerke == doctest::Approx(1.0f));
        k.tonKurve.grundpegel = 0.5;
        (void)f.takt(k, 85233, true);
        CHECK(e.stream(e.quelle(kZwei)).lautstaerke == doctest::Approx(0.5f));
        k.grundpegel = 0.5;  // MASTER dazu: malgenommen
        (void)f.takt(k, 85266, true);
        CHECK(e.stream(e.quelle(kZwei)).lautstaerke == doctest::Approx(0.25f));
    }

    TEST_CASE("S101 Mute: der Pegel des Players (Naht N17) gilt am Mixer — die Quellen bleiben, wie sie sind")
    {
        // ein Player mit dem zweiten Faktor neben der Lautstaerke, wie der echte
        struct PegelPlayer : PappPlayer
        {
            float p = 1.0f;
            [[nodiscard]] float pegel() const override { return p; }
            void setPegel(float neu) override { p = neu; }
        } p;
        PappMischEngine e;
        e.laengen = {{kEins, 300000}, {kZwei, 300000}};
        const Komposition k = probe();
        Tonfuehrung f;
        REQUIRE(f.uebernimm(e, p));
        (void)f.takt(k, 30000, true);
        const AudioStreamHandle a = e.quelle(kEins);
        REQUIRE(a != INVALID_STREAM);
        const float quelle = e.stream(a).lautstaerke;
        CHECK(e.stream(e.mixer).lautstaerke == doctest::Approx(1.0f));

        // der Mute senkt ab: der Mixer folgt je Takt, die Quelle nicht — das
        // Spektrum des Mixers (was die Visuals sehen) bleibt damit voll
        p.setPegel(0.2f);
        (void)f.takt(k, 30033, true);
        CHECK(e.stream(e.mixer).lautstaerke == doctest::Approx(0.2f));
        CHECK(e.stream(a).lautstaerke == doctest::Approx(quelle));
        // mit der Lautstaerke malgenommen; das harte Mute des Players gewinnt
        p.laut = 0.5f;
        (void)f.takt(k, 30066, true);
        CHECK(e.stream(e.mixer).lautstaerke == doctest::Approx(0.1f));
        p.stumm = true;
        (void)f.takt(k, 30100, true);
        CHECK(e.stream(e.mixer).lautstaerke == doctest::Approx(0.0f));
        p.stumm = false;
        p.setPegel(1.0f);
        (void)f.takt(k, 30133, true);
        CHECK(e.stream(e.mixer).lautstaerke == doctest::Approx(0.5f));
    }

    TEST_CASE("Die Audio-Kanaele ueberlagern sich — der laufende Song ist der mit dem hoechsten Pegel")
    {
        // wie in composer_demo_audio.lvcomp: A1 leer, A2 ein Song 0–210 s, A3 einer 193–397 s;
        // dazu eine Ueberblendung ueber die 17 s, in denen beide liegen
        Komposition k = neueKomposition();
        REQUIRE(fuegeKanalEin(k, {}, KanalArt::Audio) == 1);
        REQUIRE(fuegeKanalEin(k, {}, KanalArt::Audio) == 2);
        REQUIRE(fuegeKanalEin(k, {}, KanalArt::Audio) == 3);
        REQUIRE(fuegeSongEin(k, 2, kEins, 0, 210000) == 0);
        REQUIRE(fuegeSongEin(k, 3, kZwei, 193000, 204000) == 0);
        k.kanaele[2].clips[0].setzeBlenden(0, 17000);
        k.kanaele[3].clips[0].setzeBlenden(17000, 0);

        // das Modell: an jeder Stelle alle Clips der eingeschalteten Audio-Kanaele, der obere zuerst
        // (Befund Patrik S96: ein leerer Kanal oben macht die anderen nicht stumm)
        REQUIRE(k.toeneBei(1000).size() == 1);
        CHECK(k.toeneBei(1000)[0]->datei == kEins);
        REQUIRE(k.toeneBei(200000).size() == 2);
        CHECK(k.toeneBei(200000)[0]->datei == kEins);
        CHECK(k.toeneBei(200000)[1]->datei == kZwei);
        REQUIRE(k.toeneBei(250000).size() == 1);
        CHECK(k.toeneBei(250000)[0]->songStelle(250000 - 193000) == 57000);
        CHECK(k.toeneBei(397000).empty());
        // ein ausgeschalteter Kanal zaehlt nicht
        k.kanaele[2].an = false;
        CHECK(k.toeneBei(1000).empty());
        REQUIRE(k.toeneBei(200000).size() == 1);
        CHECK(k.toeneBei(200000)[0]->datei == kZwei);
        k.kanaele[2].an = true;

        PappPlayer p;
        PappMischEngine e;
        e.laengen = {{kEins, 210000}, {kZwei, 204000}};
        Tonfuehrung f;
        REQUIRE(f.uebernimm(e, p));
        Tonfuehrung::Ergebnis r = f.takt(k, 0, true);
        CHECK(r.songGewechselt);
        CHECK(r.song.filePath == kEins);
        CHECK(f.quellen() == 1);

        // in der Ueberblendung spielen BEIDE, jeder an seiner Stelle und mit seinem Pegel
        r = f.takt(k, 200000, true);
        REQUIRE(f.quellen() == 2);
        const AudioStreamHandle a = e.quelle(kEins);
        const AudioStreamHandle b = e.quelle(kZwei);
        CHECK(e.stream(a).pos == 200000);
        CHECK(e.stream(b).pos == 7000);
        CHECK_FALSE(e.stream(a).haelt);
        CHECK_FALSE(e.stream(b).haelt);
        CHECK(e.stream(a).lautstaerke == doctest::Approx(10.0 / 17.0));
        CHECK(e.stream(b).lautstaerke == doctest::Approx(7.0 / 17.0));
        // der laufende Song: noch der erste, sein Pegel ist der hoehere
        CHECK(r.hatSong);
        CHECK_FALSE(r.songGewechselt);
        CHECK(r.songStelleMs == 200000);

        // am Kreuzungspunkt Gleichstand: der bisherige bleibt
        r = f.takt(k, 201500, true);
        CHECK(e.stream(a).lautstaerke == doctest::Approx(e.stream(b).lautstaerke));
        CHECK_FALSE(r.songGewechselt);
        // dahinter wechselt alles auf den zweiten
        r = f.takt(k, 203000, true);
        CHECK(r.songGewechselt);
        CHECK(r.song.filePath == kZwei);
        CHECK(r.songStelleMs == 10000);
        // zurueck auf den Kreuzungspunkt: wieder Gleichstand, und wieder bleibt der bisherige — jetzt der zweite
        r = f.takt(k, 201500, true);
        CHECK_FALSE(r.songGewechselt);
        CHECK(r.songStelleMs == 8500);
        // ohne bisherigen gilt bei Gleichstand der oberste Kanal
        {
            PappMischEngine e2;
            e2.laengen = e.laengen;
            Tonfuehrung g;
            REQUIRE(g.uebernimm(e2, p));
            r = g.takt(k, 201500, true);
            CHECK(r.songGewechselt);
            CHECK(r.song.filePath == kEins);
        }

        // nach dem Ende des ersten spielt der zweite allein weiter, ohne neu anzusetzen
        (void)f.takt(k, 209990, true);
        const int gesetzt = e.zahl(QStringLiteral("seek ") + kZwei);
        std::int64_t stelle = 209990;
        (void)laufe(f, e, k, stelle, 300);
        CHECK(f.quellen() == 1);
        CHECK(e.quelle(kEins) == INVALID_STREAM);
        CHECK(e.zahl(QStringLiteral("seek ") + kZwei) == gesetzt);
        CHECK(e.stream(b).pos == stelle - 193000);

        // Next und Previous gehen ueber alle Audio-Kanaele
        CHECK(Tonfuehrung::songAnfang(k, 1000, true) == 193000);
        CHECK(Tonfuehrung::songAnfang(k, 250000, false) == 193000);
        CHECK(Tonfuehrung::songAnfang(k, 194000, false) == 0);
    }

    TEST_CASE("S126 Gain je Ton-Clip: Datei, Grenzen, Lautstaerke der Quelle — der laufende Song bleibt Sache der Kurve")
    {
        // der Faktor: 20·log10 der Amplitude
        Clip c;
        CHECK(c.gainFaktor() == doctest::Approx(1.0));
        c.gainDb = 20.0 * std::log10(2.0);
        CHECK(c.gainFaktor() == doctest::Approx(2.0));
        c.gainDb = -20.0;
        CHECK(c.gainFaktor() == doctest::Approx(0.1));

        // Datei: nur geschrieben, wenn nicht 0; fehlt es (bis S126), gilt 0 dB
        Komposition k = probe();
        k.kanaele[1].clips[0].gainDb = 13.3;
        const QJsonObject json = alsJson(k);
        const QJsonObject clipEins =
            json.value(QStringLiteral("channels")).toArray().at(1).toObject().value(QStringLiteral("clips")).toArray().at(0).toObject();
        const QJsonObject clipZwei =
            json.value(QStringLiteral("channels")).toArray().at(1).toObject().value(QStringLiteral("clips")).toArray().at(1).toObject();
        CHECK(clipEins.value(QStringLiteral("gain")).toDouble() == doctest::Approx(13.3));
        CHECK_FALSE(clipZwei.contains(QStringLiteral("gain")));
        Komposition gelesen;
        REQUIRE(ausJson(json, gelesen));
        CHECK(gelesen.kanaele[1].clips[0].gainDb == doctest::Approx(13.3));
        CHECK(gelesen.kanaele[1].clips[1].gainDb == 0.0);
        CHECK(gleich(k, gelesen));

        // Grenzen: geklemmt auf ±24 dB, Unsinn wird 0; ein Bild-Clip hat kein Gain
        {
            Komposition r = probe();
            REQUIRE(fuegeClipEin(r, 0, QStringLiteral("C:/presets/eins.milk"), 0, 5000) == 0);
            r.kanaele[0].clips[0].gainDb = 6.0;
            r.kanaele[1].clips[0].gainDb = 40.0;
            r.kanaele[1].clips[1].gainDb = std::numeric_limits<double>::quiet_NaN();
            r.richte();
            CHECK(r.kanaele[0].clips[0].gainDb == 0.0);
            CHECK(r.kanaele[1].clips[0].gainDb == kGainMaxDb);
            CHECK(r.kanaele[1].clips[1].gainDb == 0.0);
            r.kanaele[1].clips[0].gainDb = -40.0;
            r.richte();
            CHECK(r.kanaele[1].clips[0].gainDb == kGainMinDb);
        }

        // die Quelle bekommt Pegelkurve × Gain, auch ueber 1 (BASS verstaerkt dann)
        PappPlayer p;
        PappMischEngine e;
        e.laengen = {{kEins, 300000}, {kZwei, 300000}};
        k.kanaele[1].clips[0].setzeBlenden(2000, 0);
        k.kanaele[1].clips[0].gainDb = 20.0 * std::log10(2.0);
        Tonfuehrung f;
        REQUIRE(f.uebernimm(e, p));
        (void)f.takt(k, 11000, true);  // halb eingeblendet
        const AudioStreamHandle a = e.quelle(kEins);
        REQUIRE(a != INVALID_STREAM);
        CHECK(e.stream(a).lautstaerke == doctest::Approx(1.0f));
        (void)f.takt(k, 30000, true);
        CHECK(e.stream(a).lautstaerke == doctest::Approx(2.0f));
        k.tonKurve.grundpegel = 0.5;  // der Audio-Master kommt dazu
        (void)f.takt(k, 30033, true);
        CHECK(e.stream(a).lautstaerke == doctest::Approx(1.0f));

        // zwei Clips uebereinander: der laufende Song ist der mit der hoeheren
        // Kurve, auch wenn der andere mit seinem Gain lauter klingt
        Komposition u = neueKomposition();
        REQUIRE(fuegeKanalEin(u, {}, KanalArt::Audio) == 1);
        REQUIRE(fuegeKanalEin(u, {}, KanalArt::Audio) == 2);
        REQUIRE(fuegeSongEin(u, 1, kEins, 0, 100000) == 0);
        REQUIRE(fuegeSongEin(u, 2, kZwei, 0, 100000) == 0);
        u.kanaele[1].clips[0].grundpegel = 0.9;
        u.kanaele[2].clips[0].grundpegel = 0.5;
        u.kanaele[2].clips[0].gainDb = 12.0;
        PappMischEngine e2;
        e2.laengen = e.laengen;
        Tonfuehrung g;
        REQUIRE(g.uebernimm(e2, p));
        const Tonfuehrung::Ergebnis r = g.takt(u, 5000, true);
        CHECK(e2.stream(e2.quelle(kZwei)).lautstaerke > e2.stream(e2.quelle(kEins)).lautstaerke);
        CHECK(r.song.filePath == kEins);
    }

    TEST_CASE("S126 D1 Clip aus: Datei, kein Ton, nicht laufender Song, nicht Next / Previous")
    {
        // Datei: nur geschrieben, wenn aus; fehlt es, ist der Clip an
        Komposition k = probe();  // eins 10–70 s, zwei 80–100 s
        k.kanaele[1].clips[0].an = false;
        const QJsonObject json = alsJson(k);
        const QJsonArray clips =
            json.value(QStringLiteral("channels")).toArray().at(1).toObject().value(QStringLiteral("clips")).toArray();
        CHECK(clips.at(0).toObject().value(QStringLiteral("enabled")).toBool(true) == false);
        CHECK_FALSE(clips.at(1).toObject().contains(QStringLiteral("enabled")));
        Komposition gelesen;
        REQUIRE(ausJson(json, gelesen));
        CHECK_FALSE(gelesen.kanaele[1].clips[0].an);
        CHECK(gelesen.kanaele[1].clips[1].an);
        CHECK(gleich(k, gelesen));

        // das Modell: ein ausgeschalteter Clip ist kein Ton
        CHECK(k.toeneBei(30000).empty());
        CHECK(k.toeneBei(90000).size() == 1);
        // Next / Previous gehen an ihm vorbei
        CHECK(Tonfuehrung::songAnfang(k, 0, true) == 80000);
        CHECK(Tonfuehrung::songAnfang(k, 95000, false) == 80000);

        // die Fuehrung: keine Quelle, kein laufender Song; wieder an, spielt er
        PappPlayer p;
        PappMischEngine e;
        e.laengen = {{kEins, 300000}, {kZwei, 300000}};
        Tonfuehrung f;
        REQUIRE(f.uebernimm(e, p));
        const Tonfuehrung::Ergebnis r = f.takt(k, 30000, true);
        CHECK(e.quelle(kEins) == INVALID_STREAM);
        CHECK(f.quellen() == 0);
        CHECK_FALSE(r.hatSong);
        k.kanaele[1].clips[0].an = true;
        const Tonfuehrung::Ergebnis wieder = f.takt(k, 30033, true);
        CHECK(e.quelle(kEins) != INVALID_STREAM);
        CHECK(wieder.hatSong);
        CHECK(wieder.song.filePath == kEins);
        // waehrend er spielt ausgeschaltet: seine Quelle geht
        k.kanaele[1].clips[0].an = false;
        (void)f.takt(k, 30066, true);
        CHECK(f.quellen() == 0);
    }

    TEST_CASE("Next und Previous: die Anfaenge der Songs")
    {
        const Komposition k = probe();  // eins 10–70 s, zwei 80–100 s
        CHECK(Tonfuehrung::songAnfang(k, 0, true) == 10000);
        CHECK(Tonfuehrung::songAnfang(k, 10000, true) == 80000);
        CHECK(Tonfuehrung::songAnfang(k, 75000, true) == 80000);
        CHECK(Tonfuehrung::songAnfang(k, 85000, true) == -1);
        // zurueck: mitten im Song an seinen Anfang, in den ersten Sekunden zum Song davor
        CHECK(Tonfuehrung::songAnfang(k, 90000, false) == 80000);
        CHECK(Tonfuehrung::songAnfang(k, 81000, false) == 10000);
        CHECK(Tonfuehrung::songAnfang(k, 75000, false) == 10000);  // aus der Luecke
        CHECK(Tonfuehrung::songAnfang(k, 11000, false) == 10000);  // davor gibt es keinen
        CHECK(Tonfuehrung::songAnfang(k, 5000, false) == -1);
        // ohne eingeschalteten Audio-Kanal gibt es keine Songs
        Komposition stumm = k;
        stumm.kanaele[1].an = false;
        CHECK(Tonfuehrung::songAnfang(stumm, 0, true) == -1);
    }
}
