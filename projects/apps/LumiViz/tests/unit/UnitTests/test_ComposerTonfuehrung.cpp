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

#include <QMap>
#include <QStringList>

#include <algorithm>

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
            CHECK(p.isPaused());  // er bleibt angehalten
            REQUIRE(f.uebernimm(e, p));
            CHECK(p.playlist() == nullptr);
        }
        CHECK(p.playlist() == liste);  // auch wenn die Fuehrung einfach verschwindet
        CHECK(e.zahl(QStringLiteral("free mixer")) == 2);

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
        // gleich danach noch einmal daneben: in der Schonfrist bleibt sie
        e.verschiebe(a, 200);
        (void)laufe(f, e, k, stelle, 66);
        CHECK(f.nachgesetzt() == 1);
        (void)laufe(f, e, k, stelle, 600);
        CHECK(f.nachgesetzt() == 2);
        // eine kleine Abweichung bleibt
        (void)laufe(f, e, k, stelle, 600);
        e.verschiebe(a, 50);
        (void)laufe(f, e, k, stelle, 1000);
        CHECK(f.nachgesetzt() == 2);

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
