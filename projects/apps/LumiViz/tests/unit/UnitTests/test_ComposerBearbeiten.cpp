/**
 ****************************************************************************************
 * @file   test_ComposerBearbeiten.cpp
 * @brief  Composer, Stufe C3a: Clips verschieben, an den Kanten ziehen, loeschen
 *         (docs/ui/Composer_Konzept.md §6)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Reines Modell, kein Fenster. Je Art des Zusammenstosses (anhalten, kuerzen,
 * schieben) das Verschieben und beide Kanten; nach jedem Schritt gilt: die
 * Clips eines Kanals sind sortiert und ueberlappen nicht.
 *
 * Rueckbau (Konzept §12): diese Datei loeschen, Zeile in Source.cmake nehmen.
 ****************************************************************************************
 */

#include <doctest.h>

#include "composer/Bearbeiten.hpp"
#include "composer/Komposition.hpp"

#include <utility>
#include <vector>

using namespace lumi::composer;

namespace
{

VisualClip clip(const char* name, std::int64_t start, std::int64_t ende)
{
    VisualClip c;
    c.datei = QString::fromLatin1(name);
    c.startMs = start;
    c.endeMs = ende;
    return c;
}

/// Ein Kanal: A 0–10 s, B 20–30 s, C 30–45 s (B und C stossen aneinander).
Komposition probe()
{
    Kanal k;
    k.name = QStringLiteral("CH1");
    k.visual = {clip("A", 0, 10000), clip("B", 20000, 30000), clip("C", 30000, 45000)};
    Komposition komposition;
    komposition.kanaele = {k};
    return komposition;
}

const std::vector<VisualClip>& clips(const Komposition& k)
{
    return k.kanaele[0].visual;
}

/// Name, Start, Ende je Clip — als Text, damit ein Fehlschlag lesbar ist.
QString bild(const Komposition& k)
{
    QStringList teile;
    for (const VisualClip& c : clips(k))
        teile << QStringLiteral("%1[%2,%3)").arg(c.datei).arg(c.startMs).arg(c.endeMs);
    return teile.join(QLatin1Char(' '));
}

bool geordnet(const Komposition& k)
{
    std::int64_t frei = 0;
    for (const VisualClip& c : clips(k))
    {
        if (c.startMs < frei || c.endeMs <= c.startMs) return false;
        frei = c.endeMs;
    }
    return true;
}

}  // namespace

TEST_SUITE("ComposerBearbeiten")
{
    TEST_CASE("C3a anhalten: der Clip landet in der naechsten Luecke, Kanten gehen nur bis an den Nachbarn")
    {
        Komposition k = probe();
        CHECK(verschiebeClip(k, 0, 0, 5000) == 0);  // freie Stelle
        CHECK(bild(k) == QStringLiteral("A[5000,15000) B[20000,30000) C[30000,45000)"));

        CHECK(verschiebeClip(k, 0, 0, 15000) == 0);  // wuerde B treffen: buendig davor
        CHECK(bild(k) == QStringLiteral("A[10000,20000) B[20000,30000) C[30000,45000)"));

        CHECK(verschiebeClip(k, 0, 0, 40000) == 2);  // ueber die Nachbarn hinweg: hinter C
        CHECK(bild(k) == QStringLiteral("B[20000,30000) C[30000,45000) A[45000,55000)"));

        CHECK(verschiebeClip(k, 0, 2, -700) == 0);  // nicht vor den Anfang
        CHECK(bild(k) == QStringLiteral("A[0,10000) B[20000,30000) C[30000,45000)"));

        // zwischen B und C ist keine Luecke: C nach 22 s geht in die Luecke davor
        CHECK(verschiebeClip(k, 0, 2, 22000) == 2);
        CHECK(bild(k) == QStringLiteral("A[0,10000) B[20000,30000) C[30000,45000)"));
        k = probe();
        k.kanaele[0].visual[1] = clip("B", 26000, 30000);  // Luecke 10–26 s, C ist 15 s lang
        CHECK(verschiebeClip(k, 0, 2, 12000) == 1);
        CHECK(bild(k) == QStringLiteral("A[0,10000) C[11000,26000) B[26000,30000)"));
        CHECK(geordnet(k));

        // Kanten
        k = probe();
        CHECK(zieheKante(k, 0, 0, false, 25000) == 0);  // rechte Kante von A: bis an B
        CHECK(bild(k) == QStringLiteral("A[0,20000) B[20000,30000) C[30000,45000)"));
        CHECK(zieheKante(k, 0, 0, false, 50) == 0);  // nicht kuerzer als das Mindestmass
        CHECK(clips(k)[0].endeMs == kMinClipMs);
        k = probe();
        CHECK(zieheKante(k, 0, 1, true, 5000) == 1);  // linke Kante von B: bis an A
        CHECK(bild(k) == QStringLiteral("A[0,10000) B[10000,30000) C[30000,45000)"));
        CHECK(zieheKante(k, 0, 1, true, 99999) == 1);
        CHECK(clips(k)[1].startMs == 30000 - kMinClipMs);
        CHECK(zieheKante(k, 0, 2, false, 90000) == 2);  // der letzte Clip waechst frei
        CHECK(clips(k)[2].endeMs == 90000);
        CHECK(zieheKante(k, 0, 0, true, -5) == 0);
        CHECK(clips(k)[0].startMs == 0);
        CHECK(geordnet(k));
    }

    TEST_CASE("C3a kuerzen: der gezogene Clip gewinnt, der Nachbar weicht")
    {
        Komposition k = probe();
        CHECK(verschiebeClip(k, 0, 0, 15000, Stoss::Kuerzen) == 0);  // B vorn gekuerzt
        CHECK(bild(k) == QStringLiteral("A[15000,25000) B[25000,30000) C[30000,45000)"));

        k = probe();
        CHECK(verschiebeClip(k, 0, 0, 20000, Stoss::Kuerzen) == 0);  // B ganz ueberdeckt
        CHECK(bild(k) == QStringLiteral("A[20000,30000) C[30000,45000)"));

        k = probe();
        CHECK(verschiebeClip(k, 0, 0, 32000, Stoss::Kuerzen) == 2);  // mitten in C: der vordere Teil bleibt
        CHECK(bild(k) == QStringLiteral("B[20000,30000) C[30000,32000) A[32000,42000)"));

        k = probe();
        CHECK(verschiebeClip(k, 0, 1, 5000, Stoss::Kuerzen) == 1);  // A hinten gekuerzt
        CHECK(bild(k) == QStringLiteral("A[0,5000) B[5000,15000) C[30000,45000)"));

        // ein Rest unter dem Mindestmass verschwindet
        k = probe();
        CHECK(verschiebeClip(k, 0, 1, 50, Stoss::Kuerzen) == 0);
        CHECK(bild(k) == QStringLiteral("B[50,10050) C[30000,45000)"));

        // Kanten
        k = probe();
        CHECK(zieheKante(k, 0, 0, false, 25000, Stoss::Kuerzen) == 0);
        CHECK(bild(k) == QStringLiteral("A[0,25000) B[25000,30000) C[30000,45000)"));
        k = probe();
        CHECK(zieheKante(k, 0, 2, true, 4000, Stoss::Kuerzen) == 1);  // C schluckt B, kuerzt A
        CHECK(bild(k) == QStringLiteral("A[0,4000) C[4000,45000)"));
        CHECK(geordnet(k));
    }

    TEST_CASE("C3a schieben: die Nachbarn ruecken mit, ihre Laenge bleibt")
    {
        Komposition k = probe();
        CHECK(verschiebeClip(k, 0, 0, 15000, Stoss::Schieben) == 0);  // B und C ruecken nach
        CHECK(bild(k) == QStringLiteral("A[15000,25000) B[25000,35000) C[35000,50000)"));

        k = probe();
        CHECK(verschiebeClip(k, 0, 0, 5000, Stoss::Schieben) == 0);  // niemand im Weg
        CHECK(bild(k) == QStringLiteral("A[5000,15000) B[20000,30000) C[30000,45000)"));

        // nach links: B und A weichen, am Anfang ist Schluss — C kommt nur bis 20 s
        k = probe();
        CHECK(verschiebeClip(k, 0, 2, 5000, Stoss::Schieben) == 2);
        CHECK(bild(k) == QStringLiteral("A[0,10000) B[10000,20000) C[20000,35000)"));

        k = probe();
        CHECK(verschiebeClip(k, 0, 2, 25000, Stoss::Schieben) == 2);  // nur B weicht
        CHECK(bild(k) == QStringLiteral("A[0,10000) B[15000,25000) C[25000,40000)"));

        // Kanten
        k = probe();
        CHECK(zieheKante(k, 0, 0, false, 25000, Stoss::Schieben) == 0);
        CHECK(bild(k) == QStringLiteral("A[0,25000) B[25000,35000) C[35000,50000)"));
        k = probe();
        CHECK(zieheKante(k, 0, 2, true, 25000, Stoss::Schieben) == 2);
        CHECK(bild(k) == QStringLiteral("A[0,10000) B[15000,25000) C[25000,45000)"));
        k = probe();
        CHECK(zieheKante(k, 0, 2, true, 5000, Stoss::Schieben) == 2);  // am Anfang ist Schluss
        CHECK(bild(k) == QStringLiteral("A[0,10000) B[10000,20000) C[20000,45000)"));
        CHECK(geordnet(k));
    }

    TEST_CASE("C3a Pegelkurve: beim Ziehen einer Kante bleiben Ein- und Ausblendung an ihrer Kante")
    {
        Komposition k;
        Kanal kanal;
        kanal.visual = {clip("A", 10000, 30000)};
        kanal.visual[0].setzeBlenden(3000, 5000);
        k.kanaele = {kanal};
        const auto pegel = [&k](std::int64_t imClip) { return clips(k)[0].pegelBei(imClip); };
        REQUIRE(pegel(0) == doctest::Approx(0.0));
        REQUIRE(pegel(3000) == doctest::Approx(1.0));
        REQUIRE(pegel(15000) == doctest::Approx(1.0));
        REQUIRE(pegel(20000) == doctest::Approx(0.0));

        // rechte Kante hinaus: 30 s lang — die Ausblendung sitzt weiter am Ende
        REQUIRE(zieheKante(k, 0, 0, false, 40000) == 0);
        CHECK(pegel(0) == doctest::Approx(0.0));
        CHECK(pegel(3000) == doctest::Approx(1.0));
        CHECK(pegel(20000) == doctest::Approx(1.0));  // wo vorher das Ende war
        CHECK(pegel(25000) == doctest::Approx(1.0));
        CHECK(pegel(27500) == doctest::Approx(0.5));
        CHECK(pegel(30000) == doctest::Approx(0.0));

        // linke Kante hinein: 26 s lang — die Einblendung beginnt am neuen Anfang
        REQUIRE(zieheKante(k, 0, 0, true, 14000) == 0);
        CHECK(clips(k)[0].startMs == 14000);
        CHECK(pegel(0) == doctest::Approx(0.0));
        CHECK(pegel(1500) == doctest::Approx(0.5));
        CHECK(pegel(3000) == doctest::Approx(1.0));
        CHECK(pegel(21000) == doctest::Approx(1.0));
        CHECK(pegel(26000) == doctest::Approx(0.0));

        // verschieben: die Kurve wandert unveraendert mit
        const auto vorher = clips(k)[0].kurve.size();
        REQUIRE(verschiebeClip(k, 0, 0, 2000) == 0);
        CHECK(clips(k)[0].kurve.size() == vorher);
        CHECK(pegel(1500) == doctest::Approx(0.5));
        CHECK(pegel(26000) == doctest::Approx(0.0));

        // kuerzer als beide Blenden zusammen: die Punkte bleiben im Clip und sortiert
        REQUIRE(zieheKante(k, 0, 0, false, 6000) == 0);
        std::int64_t zuletzt = 0;
        for (const PegelPunkt& p : clips(k)[0].kurve)
        {
            CHECK(p.zeitMs >= zuletzt);
            CHECK(p.zeitMs <= 4000);
            zuletzt = p.zeitMs;
        }
    }

    TEST_CASE("C3a Kanal wechseln: der Clip zieht in einen anderen Kanal um, mit derselben Regel am Nachbarn")
    {
        const auto mitZweitem = [] {
            Komposition k = probe();
            Kanal zweiter;
            zweiter.name = QStringLiteral("CH2");
            zweiter.visual = {clip("X", 5000, 25000)};
            k.kanaele.push_back(zweiter);
            return k;
        };
        const auto ch2 = [](const Komposition& k) {
            QStringList teile;
            for (const VisualClip& c : k.kanaele[1].visual)
                teile << QStringLiteral("%1[%2,%3)").arg(c.datei).arg(c.startMs).arg(c.endeMs);
            return teile.join(QLatin1Char(' '));
        };

        // anhalten: vor X ist kein Platz fuer 10 s — also dahinter
        Komposition k = mitZweitem();
        CHECK(versetzeClip(k, 0, 0, 1, 0) == 1);
        CHECK(bild(k) == QStringLiteral("B[20000,30000) C[30000,45000)"));
        CHECK(ch2(k) == QStringLiteral("X[5000,25000) A[25000,35000)"));

        // freie Stelle im Zielkanal
        k = mitZweitem();
        CHECK(versetzeClip(k, 0, 2, 1, 30000) == 1);  // C 30–45 s
        CHECK(bild(k) == QStringLiteral("A[0,10000) B[20000,30000)"));
        CHECK(ch2(k) == QStringLiteral("X[5000,25000) C[30000,45000)"));

        // kuerzen: X weicht
        k = mitZweitem();
        CHECK(versetzeClip(k, 0, 0, 1, 0, Stoss::Kuerzen) == 0);
        CHECK(ch2(k) == QStringLiteral("A[0,10000) X[10000,25000)"));

        // schieben: X rueckt nach rechts bzw. nach links
        k = mitZweitem();
        CHECK(versetzeClip(k, 0, 0, 1, 0, Stoss::Schieben) == 0);
        CHECK(ch2(k) == QStringLiteral("A[0,10000) X[10000,30000)"));
        k = mitZweitem();
        CHECK(versetzeClip(k, 0, 1, 1, 20000, Stoss::Schieben) == 1);  // B 20–30 s
        CHECK(ch2(k) == QStringLiteral("X[0,20000) B[20000,30000)"));
        CHECK(bild(k) == QStringLiteral("A[0,10000) C[30000,45000)"));

        // die Kurve wandert mit
        k = mitZweitem();
        k.kanaele[0].visual[0].setzeBlenden(2000, 2000);
        const auto punkte = k.kanaele[0].visual[0].kurve.size();
        REQUIRE(versetzeClip(k, 0, 0, 1, 40000) == 1);
        CHECK(k.kanaele[1].visual[1].kurve.size() == punkte);
        CHECK(k.kanaele[1].visual[1].pegelBei(1000) == doctest::Approx(0.5));

        // Zielkanal, den es nicht gibt, oder ein Audio-Kanal: nichts aendert sich
        k = mitZweitem();
        const Komposition vorher = k;
        CHECK(versetzeClip(k, 0, 0, 2, 0) == -1);
        CHECK(versetzeClip(k, 0, 0, -1, 0) == -1);
        CHECK(gleich(k, vorher));
        Kanal ton;
        ton.art = KanalArt::Audio;
        k.kanaele.push_back(ton);
        CHECK(versetzeClip(k, 0, 0, 2, 0) == -1);
        CHECK(k.kanaele[0].visual.size() == 3);
    }

    TEST_CASE("C3a Ende (Befund Patrik S95): das Ende folgt dem letzten Clip — ausser es ist fest gesetzt")
    {
        // Eine Datei, deren Laenge genau das Ende des letzten Clips ist (wie
        // composer_demo.lvcomp), sagt damit nichts Eigenes: das Ende folgt.
        Komposition k = probe();
        k.laengeMs = 45000;
        Komposition gelesen;
        REQUIRE(ausJson(alsJson(k), gelesen));
        CHECK(gelesen.laengeMs == 0);
        CHECK(gelesen.laenge() == 45000);
        REQUIRE(zieheKante(gelesen, 0, 2, false, 38000) == 2);  // der letzte Clip wird kuerzer
        CHECK(gelesen.laenge() == 38000);                        // — das Ende kommt mit
        REQUIRE(loescheClip(gelesen, 0, 2));
        CHECK(gelesen.laenge() == 30000);

        // Eine Laenge UEBER das Ende des letzten Clips hinaus ist gewollt: sie bleibt fest.
        k.laengeMs = 60000;
        REQUIRE(ausJson(alsJson(k), gelesen));
        CHECK(gelesen.laengeMs == 60000);
        REQUIRE(zieheKante(gelesen, 0, 2, false, 38000) == 2);
        CHECK(gelesen.laenge() == 60000);
        // ... und waechst nur, wenn ein Clip darueber hinausragt
        REQUIRE(zieheKante(gelesen, 0, 2, false, 70000) == 2);
        CHECK(gelesen.laenge() == 70000);

        // Eine Laenge VOR dem Ende des letzten Clips schneidet ab — auch gewollt.
        k.laengeMs = 40000;
        REQUIRE(ausJson(alsJson(k), gelesen));
        CHECK(gelesen.laengeMs == 40000);
    }

    TEST_CASE("C3b Neues hinein: ein Clip faellt in den Kanal, Kanaele entstehen und gehen")
    {
        // anhalten: in freier Flaeche reicht der neue Clip hoechstens bis zum naechsten
        Komposition k = probe();  // A 0–10, B 20–30, C 30–45
        CHECK(fuegeClipEin(k, 0, QStringLiteral("N"), 12000) == 1);
        CHECK(bild(k) == QStringLiteral("A[0,10000) N[12000,20000) B[20000,30000) C[30000,45000)"));
        k = probe();
        CHECK(fuegeClipEin(k, 0, QStringLiteral("N"), 50000) == 3);  // hinter allem: volle 20 s
        CHECK(clips(k)[3].startMs == 50000);
        CHECK(clips(k)[3].endeMs == 50000 + kNeuerClipMs);
        CHECK(clips(k)[3].kurve.empty());  // voller Pegel
        k = probe();
        CHECK(fuegeClipEin(k, 0, QStringLiteral("N"), 25000) == 3);  // auf B: die naechste Luecke, die 20 s fasst
        CHECK(clips(k)[3].startMs == 45000);
        k = probe();
        CHECK(fuegeClipEin(k, 0, QStringLiteral("N"), 12000, 5000) == 1);  // eigene Laenge
        CHECK(clips(k)[1].endeMs == 17000);

        // kuerzen und schieben wie beim Verschieben
        k = probe();
        CHECK(fuegeClipEin(k, 0, QStringLiteral("N"), 5000, 20000, Stoss::Kuerzen) == 1);
        CHECK(bild(k) == QStringLiteral("A[0,5000) N[5000,25000) B[25000,30000) C[30000,45000)"));
        k = probe();
        CHECK(fuegeClipEin(k, 0, QStringLiteral("N"), 12000, 20000, Stoss::Schieben) == 1);
        CHECK(bild(k) == QStringLiteral("A[0,10000) N[12000,32000) B[32000,42000) C[42000,57000)"));
        CHECK(geordnet(k));

        // den Kanal gibt es nicht, oder er ist ein Audio-Kanal
        const Komposition vorher = k;
        CHECK(fuegeClipEin(k, 4, QStringLiteral("N"), 0) == -1);
        CHECK(fuegeClipEin(k, -1, QStringLiteral("N"), 0) == -1);
        CHECK(gleich(k, vorher));

        // eine neue Komposition: ein leerer Kanal, das Ende folgt den Clips
        Komposition neu = neueKomposition();
        REQUIRE(neu.kanaele.size() == 1);
        CHECK(neu.kanaele[0].name == QStringLiteral("CH1"));
        CHECK(neu.kanaele[0].art == KanalArt::Visual);
        CHECK(neu.kanaele[0].an);
        CHECK(neu.laengeMs == 0);
        CHECK(neu.laenge() == 0);
        CHECK(fuegeClipEin(neu, 0, QStringLiteral("N"), 3000) == 0);
        CHECK(neu.laenge() == 3000 + kNeuerClipMs);

        // Kanaele: der Name ist die kleinste freie Nummer — oder der gewuenschte
        CHECK(fuegeKanalEin(neu) == 1);
        CHECK(neu.kanaele[1].name == QStringLiteral("CH2"));
        CHECK(fuegeKanalEin(neu, QStringLiteral("Sterne")) == 2);
        CHECK(loescheKanal(neu, 0));  // samt seinem Clip
        CHECK(neu.laenge() == 0);
        CHECK(fuegeKanalEin(neu) == 2);
        CHECK(neu.kanaele[2].name == QStringLiteral("CH1"));
        CHECK_FALSE(loescheKanal(neu, 3));
        CHECK_FALSE(loescheKanal(neu, -1));
        Kanal ton;
        ton.art = KanalArt::Audio;
        neu.kanaele.push_back(ton);
        CHECK(fuegeClipEin(neu, 3, QStringLiteral("N"), 0) == -1);
    }

    TEST_CASE("C3a Laenge, Loeschen, Unsinn")
    {
        // eine gesetzte Laenge waechst mit; ohne gesetzte Laenge bleibt sie offen
        Komposition k = probe();
        k.laengeMs = 45000;
        REQUIRE(verschiebeClip(k, 0, 0, 15000, Stoss::Schieben) == 0);
        CHECK(k.laengeMs == 50000);
        CHECK(k.laenge() == 50000);
        k = probe();
        REQUIRE(zieheKante(k, 0, 2, false, 60000) == 2);
        CHECK(k.laengeMs == 0);
        CHECK(k.laenge() == 60000);

        // loeschen
        k = probe();
        CHECK(loescheClip(k, 0, 1));
        CHECK(bild(k) == QStringLiteral("A[0,10000) C[30000,45000)"));
        CHECK_FALSE(loescheClip(k, 0, 2));
        CHECK_FALSE(loescheClip(k, 3, 0));
        CHECK_FALSE(loescheClip(k, -1, 0));

        // was es nicht gibt, aendert nichts
        const Komposition vorher = k;
        CHECK(verschiebeClip(k, 0, 9, 1000) == -1);
        CHECK(verschiebeClip(k, 5, 0, 1000) == -1);
        CHECK(zieheKante(k, 0, -1, true, 1000) == -1);
        CHECK(gleich(k, vorher));
        CHECK_FALSE(gleich(k, probe()));

        // ein zweiter Kanal bleibt bei allem unberuehrt
        k = probe();
        Kanal zweiter;
        zweiter.name = QStringLiteral("CH2");
        zweiter.visual = {clip("X", 5000, 25000)};
        k.kanaele.push_back(zweiter);
        REQUIRE(verschiebeClip(k, 0, 0, 15000, Stoss::Kuerzen) == 0);
        CHECK(k.kanaele[1].visual[0].startMs == 5000);
        CHECK(k.kanaele[1].visual[0].endeMs == 25000);
        REQUIRE(verschiebeClip(k, 1, 0, 0) == 0);
        CHECK(bild(k) == QStringLiteral("A[15000,25000) B[25000,30000) C[30000,45000)"));
    }
}
