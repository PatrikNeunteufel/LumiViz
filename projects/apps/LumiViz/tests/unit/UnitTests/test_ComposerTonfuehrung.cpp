/**
 ****************************************************************************************
 * @file   test_ComposerTonfuehrung.cpp
 * @brief  Composer, Stufe C5: der Audiokanal fuehrt den Player
 *         (docs/ui/Composer_Konzept.md §4 und §8)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Kein Fenster, kein Ton: die Fuehrung laeuft gegen einen Player aus Pappe, der
 * mitschreibt, was man ihm sagt, und dessen Zeit der Test von Hand weiterdreht.
 * Geprueft wird, wer fuehrt — der Ton, solange ein Song spielt; die Uhr, wenn
 * sie springt; die Wanduhr in Luecken und nach dem Ende eines Songs.
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

/// Uhr und Player laufen `ms` weiter — in Takten von 33 ms, wie die Sitzung taktet.
/// Die Uhr folgt dabei dem, was die Fuehrung verlangt.
void laufe(Tonfuehrung& f, PappPlayer& p, const Komposition& k, std::int64_t& stelle, int ms)
{
    for (int rest = ms; rest > 0; rest -= 33)
    {
        const int schritt = std::min(33, rest);
        stelle += schritt;
        p.vor(schritt);
        const Tonfuehrung::Ergebnis e = f.takt(k, stelle, true);
        if (e.uhrStellen) stelle = e.stelleMs;
    }
}

}  // namespace

TEST_SUITE("ComposerTonfuehrung")
{
    TEST_CASE("Uebernehmen laesst die Playlist ruhen, Freigeben haengt sie wieder ein")
    {
        PappPlayer p;
        // nie benutzt, nur weitergereicht: irgendeine Adresse genuegt als Playlist
        int platz = 0;
        auto* liste = reinterpret_cast<IPlaylist*>(&platz);
        p.setPlaylist(liste);
        float pegel = -1.0f;
        {
            Tonfuehrung f;
            f.setPegelSenke([&pegel](float v) { pegel = v; });
            CHECK_FALSE(f.fuehrt());
            CHECK_FALSE(f.takt(probe(), 0, true).uhrStellen);  // ohne Player tut ein Takt nichts
            f.uebernimm(p);
            CHECK(f.fuehrt());
            CHECK(p.playlist() == nullptr);
            f.uebernimm(p);  // noch einmal: die gemerkte Playlist bleibt die echte
            f.gibFrei();
            CHECK(p.playlist() == liste);
            CHECK(pegel == doctest::Approx(1.0f));  // der Pegel steht wieder auf voll
            f.uebernimm(p);
            CHECK(p.playlist() == nullptr);
        }
        CHECK(p.playlist() == liste);  // auch wenn die Fuehrung einfach verschwindet
    }

    TEST_CASE("Luecke, Einstieg in einen Song, der Ton fuehrt, Ende des Clips")
    {
        PappPlayer p;
        p.laengen = {{kEins, 300000}, {kZwei, 300000}};
        const Komposition k = probe();
        Tonfuehrung f;
        f.uebernimm(p);
        std::int64_t stelle = 0;

        // vor dem ersten Song: Stille, die Wanduhr fuehrt
        CHECK_FALSE(f.takt(k, stelle, true).uhrStellen);
        laufe(f, p, k, stelle, 5000);
        CHECK(p.protokoll.isEmpty());
        CHECK_FALSE(p.isPlaying());
        CHECK(stelle == 5000);

        // am Anfang des Clips: laden, an den Einsatz setzen, spielen
        stelle = 9990;
        (void)f.takt(k, stelle, true);
        laufe(f, p, k, stelle, 33);
        // (die Uhr steht 23 ms hinter dem Clip-Anfang, der Einsatz liegt bei 5 s)
        CHECK(p.protokoll == QStringList{QStringLiteral("load ") + kEins, QStringLiteral("seek 5023"),
                                         QStringLiteral("play")});
        CHECK(p.isPlaying());

        // beide laufen gleich schnell: nichts wird nachgestellt
        laufe(f, p, k, stelle, 10000);
        CHECK(p.zahl(QStringLiteral("seek")) == 1);
        CHECK(stelle == 10000 + (p.positionMs() - 5000));

        // die Uhr laeuft dem Ton davon (Wanduhr gegen Tonausgabe): die UHR wird nachgestellt
        stelle += 200;
        const Tonfuehrung::Ergebnis e = f.takt(k, stelle, true);
        REQUIRE(e.uhrStellen);
        CHECK(e.stelleMs == 10000 + (p.positionMs() - 5000));
        stelle = e.stelleMs;
        CHECK(p.zahl(QStringLiteral("seek")) == 1);  // der Player wurde nicht angefasst
        // eine kleine Abweichung bleibt
        stelle += 30;
        CHECK_FALSE(f.takt(k, stelle, true).uhrStellen);
        stelle -= 30;
        (void)f.takt(k, stelle, true);

        // am Ende des Clips: der Player schweigt, obwohl der Song weiterginge
        laufe(f, p, k, stelle, static_cast<int>(70000 - stelle) + 40);
        CHECK(stelle >= 70000);
        CHECK_FALSE(p.isPlaying());
        CHECK(p.protokoll.last() == QStringLiteral("pause"));
        // in der Luecke fuehrt die Wanduhr
        laufe(f, p, k, stelle, 5000);
        CHECK(p.zahl(QStringLiteral("load")) == 1);

        // der zweite Song: laden, von vorn
        laufe(f, p, k, stelle, static_cast<int>(80000 - stelle) + 40);
        CHECK(p.currentTrack().filePath == kZwei);
        CHECK(p.isPlaying());
        CHECK(p.positionMs() < 200);
    }

    TEST_CASE("Springt der Player, folgt die Uhr — springt die Uhr, folgt der Player")
    {
        PappPlayer p;
        p.laengen = {{kEins, 300000}, {kZwei, 300000}};
        const Komposition k = probe();
        Tonfuehrung f;
        f.uebernimm(p);
        std::int64_t stelle = 20000;
        (void)f.takt(k, stelle, true);  // mitten hinein: der Song steht bei 15 s
        CHECK(p.positionMs() == 15000);
        laufe(f, p, k, stelle, 1000);

        // der Fortschrittsbalken im Player-Panel: der Ton springt, die Uhr geht mit
        p.seek(40000);
        p.protokoll.clear();
        laufe(f, p, k, stelle, 33);
        CHECK(stelle == 10000 + (p.positionMs() - 5000));
        CHECK(stelle > 45000);
        CHECK(p.protokoll.isEmpty());  // der Player wurde nicht zurueckgeholt

        // ein Klick ins Lineal des Composers: die Uhr springt, der Player wird gesetzt
        stelle = 30000;
        const Tonfuehrung::Ergebnis e = f.takt(k, stelle, true);
        CHECK_FALSE(e.uhrStellen);
        CHECK(p.positionMs() == 25000);
        CHECK(p.isPlaying());
        // ... auch rueckwaerts um ein kleines Stueck (Schleife, Stop)
        laufe(f, p, k, stelle, 500);
        stelle -= 400;
        CHECK_FALSE(f.takt(k, stelle, true).uhrStellen);
        CHECK(p.positionMs() == stelle - 10000 + 5000);

        // die Uhr haelt: der Player haelt mit; steht sie, fuehrt jede Bewegung
        (void)f.takt(k, stelle, false);
        CHECK(p.isPaused());
        const std::int64_t gehalten = stelle;
        CHECK_FALSE(f.takt(k, stelle, false).uhrStellen);
        stelle = 50000;  // Klick ins Lineal bei stehender Uhr
        CHECK_FALSE(f.takt(k, stelle, false).uhrStellen);
        CHECK(p.positionMs() == 45000);
        CHECK(p.isPaused());
        p.seek(12000);  // der Balken im Player-Panel bei stehender Uhr
        const Tonfuehrung::Ergebnis g = f.takt(k, stelle, false);
        REQUIRE(g.uhrStellen);
        CHECK(g.stelleMs == 10000 + (12000 - 5000));
        stelle = g.stelleMs;
        CHECK(stelle != gehalten);

        // die Uhr laeuft wieder: der Player spielt weiter, von dort
        (void)f.takt(k, stelle, true);
        CHECK(p.isPlaying());
        CHECK(p.positionMs() == 12000);

        // vom Song in die Luecke gesprungen und zurueck in denselben Song: kein neues Laden
        const int geladen = p.zahl(QStringLiteral("load"));
        stelle = 75000;
        (void)f.takt(k, stelle, true);
        CHECK_FALSE(p.isPlaying());
        stelle = 15000;
        (void)f.takt(k, stelle, true);
        CHECK(p.isPlaying());
        CHECK(p.positionMs() == 10000);
        CHECK(p.zahl(QStringLiteral("load")) == geladen);
    }

    TEST_CASE("Ein Song, der vor seinem Clip endet, und einer, der sich nicht laden laesst")
    {
        PappPlayer p;
        p.laengen = {{kEins, 300000}, {kZwei, 8000}};  // zwei ist kuerzer als sein Clip von 20 s
        Komposition k = probe();
        REQUIRE(fuegeSongEin(k, 1, kFehlt, 110000, 10000) == 2);
        Tonfuehrung f;
        f.uebernimm(p);
        std::int64_t stelle = 80000;
        (void)f.takt(k, stelle, true);
        REQUIRE(p.isPlaying());

        // nach 8 s ist der Song aus: von da an fuehrt die Wanduhr, niemand startet ihn neu
        laufe(f, p, k, stelle, 9000);
        CHECK(f.songAus());
        CHECK_FALSE(p.isPlaying());
        const int gespielt = p.zahl(QStringLiteral("play"));
        laufe(f, p, k, stelle, 5000);
        CHECK(stelle == 94000);
        CHECK(p.zahl(QStringLiteral("play")) == gespielt);
        // auch Pause und Weiter starten ihn nicht neu
        (void)f.takt(k, stelle, false);
        (void)f.takt(k, stelle, true);
        CHECK(p.zahl(QStringLiteral("play")) == gespielt);
        // ein Sprung der Uhr zurueck in den Song dagegen schon
        stelle = 82000;
        (void)f.takt(k, stelle, true);
        CHECK_FALSE(f.songAus());
        CHECK(p.isPlaying());
        CHECK(p.positionMs() == 2000);

        // eine Datei, die fehlt: ein Versuch, dann Stille bis zum Ende des Clips
        stelle = 110000;
        (void)f.takt(k, stelle, true);
        CHECK(f.songAus());
        laufe(f, p, k, stelle, 3000);
        CHECK(p.zahl(QStringLiteral("load ") + kFehlt) == 1);
        CHECK(stelle == 113000);
    }

    TEST_CASE("Der Pegel des Clips geht je Takt an die Senke")
    {
        PappPlayer p;
        p.laengen = {{kEins, 300000}, {kZwei, 300000}};
        Komposition k = probe();
        k.kanaele[1].clips[0].setzeBlenden(2000, 4000);
        k.kanaele[1].clips[0].grundpegel = 0.8;
        float pegel = -1.0f;
        Tonfuehrung f;
        f.setPegelSenke([&pegel](float v) { pegel = v; });
        f.uebernimm(p);
        (void)f.takt(k, 11000, true);  // eine von zwei Sekunden Einblendung
        CHECK(pegel == doctest::Approx(0.4f));
        (void)f.takt(k, 30000, true);
        CHECK(pegel == doctest::Approx(0.8f));
        (void)f.takt(k, 68000, true);  // zwei Sekunden vor Schluss
        CHECK(pegel == doctest::Approx(0.4f));
        (void)f.takt(k, 85000, true);  // der zweite Song: voller Pegel
        CHECK(pegel == doctest::Approx(1.0f));
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
