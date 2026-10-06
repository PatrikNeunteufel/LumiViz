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

#include <QJsonArray>
#include <QJsonObject>

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
    k.clips = {clip("A", 0, 10000), clip("B", 20000, 30000), clip("C", 30000, 45000)};
    Komposition komposition;
    komposition.kanaele = {k};
    return komposition;
}

const std::vector<VisualClip>& clips(const Komposition& k)
{
    return k.kanaele[0].clips;
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
        k.kanaele[0].clips[1] = clip("B", 26000, 30000);  // Luecke 10–26 s, C ist 15 s lang
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
        kanal.clips = {clip("A", 10000, 30000)};
        kanal.clips[0].setzeBlenden(3000, 5000);
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
        REQUIRE(verschiebeClip(k, 0, 0, 2000) == 0);
        CHECK(clips(k)[0].ein.dauerMs == 3000);
        CHECK(clips(k)[0].aus.dauerMs == 5000);
        CHECK(pegel(1500) == doctest::Approx(0.5));
        CHECK(pegel(26000) == doctest::Approx(0.0));

        // kuerzer als beide Blenden zusammen (4 s fuer 3 s + 5 s): sie teilen den
        // Clip im Verhaeltnis 3 : 5 — die eingestellten Dauern bleiben (Entscheid S96)
        REQUIRE(zieheKante(k, 0, 0, false, 6000) == 0);
        CHECK(clips(k)[0].dauerMs() == 4000);
        CHECK(clips(k)[0].ein.dauerMs == 3000);
        CHECK(clips(k)[0].aus.dauerMs == 5000);
        const Verteilung v = clips(k)[0].verteilung();
        CHECK(v.ein.bisMs == 1500);
        CHECK(v.aus.vonMs == 1500);
        CHECK(v.ein.gestaucht);
        CHECK(v.aus.gestaucht);
        CHECK(pegel(750) == doctest::Approx(0.5));
        CHECK(pegel(1500) == doctest::Approx(1.0));
        CHECK(pegel(2750) == doctest::Approx(0.5));

        // wieder laenger: die Blenden kommen in ihrer eingestellten Groesse zurueck
        REQUIRE(zieheKante(k, 0, 0, false, 30000) == 0);
        CHECK(pegel(3000) == doctest::Approx(1.0));
        CHECK(pegel(1500) == doctest::Approx(0.5));
        CHECK(pegel(28000 - 2500) == doctest::Approx(0.5));

        // den sichtbaren Stand festschreiben: nur, was gestaucht ist
        REQUIRE(zieheKante(k, 0, 0, false, 6000) == 0);
        VisualClip fest = clips(k)[0];
        schreibeFest(fest);
        CHECK(fest.ein.dauerMs == 1500);
        CHECK(fest.aus.dauerMs == 2500);
        CHECK(fest.pegelBei(750) == doctest::Approx(0.5));
        VisualClip frei = clip("A", 0, 20000);
        frei.setzeBlenden(3000, 5000);
        schreibeFest(frei);
        CHECK(frei.ein.dauerMs == 3000);
        CHECK(frei.aus.dauerMs == 5000);
    }

    TEST_CASE("C3a Kanal wechseln: der Clip zieht in einen anderen Kanal um, mit derselben Regel am Nachbarn")
    {
        const auto mitZweitem = [] {
            Komposition k = probe();
            Kanal zweiter;
            zweiter.name = QStringLiteral("CH2");
            zweiter.clips = {clip("X", 5000, 25000)};
            k.kanaele.push_back(zweiter);
            return k;
        };
        const auto ch2 = [](const Komposition& k) {
            QStringList teile;
            for (const VisualClip& c : k.kanaele[1].clips)
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
        k.kanaele[0].clips[0].setzeBlenden(2000, 2000);
        REQUIRE(versetzeClip(k, 0, 0, 1, 40000) == 1);
        CHECK(k.kanaele[1].clips[1].ein.dauerMs == 2000);
        CHECK(k.kanaele[1].clips[1].pegelBei(1000) == doctest::Approx(0.5));

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
        CHECK(k.kanaele[0].clips.size() == 3);
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
        CHECK(clips(k)[3].vollerPegel());
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

    TEST_CASE("C4 Blenden: Laenge und Form je Seite, Bézier mit zwei Anfassern, lesen und schreiben")
    {
        // --- die Bézier-Kurve selbst
        CHECK(bezierWert(kBezierVorgabe, 0.0) == doctest::Approx(0.0).epsilon(0.001));
        CHECK(bezierWert(kBezierVorgabe, 1.0) == doctest::Approx(1.0).epsilon(0.001));
        CHECK(bezierWert(kBezierVorgabe, 0.5) == doctest::Approx(0.5).epsilon(0.001));  // S-Kurve: symmetrisch
        CHECK(bezierWert(kBezierVorgabe, 0.2) < 0.2);
        CHECK(bezierWert(kBezierVorgabe, 0.8) > 0.8);
        const BezierGriffe gerade = {1.0 / 3.0, 1.0 / 3.0, 2.0 / 3.0, 2.0 / 3.0};
        for (const double t : {0.1, 0.37, 0.5, 0.9})
            CHECK(bezierWert(gerade, t) == doctest::Approx(t).epsilon(0.001));  // Anfasser auf der Geraden: linear
        double zuletzt = 0.0;
        for (int i = 0; i <= 50; ++i)  // steigt stetig
        {
            const double w = bezierWert({0.9, 0.1, 0.2, 0.7}, i / 50.0);
            CHECK(w >= zuletzt - 1e-9);
            zuletzt = w;
        }
        CHECK(bezierWert({5.0, -3.0, -2.0, 9.0}, 0.5) >= 0.0);  // Unsinn wird geklemmt
        // ohne Anfasser laeuft die Form linear; mit dem Punkt gilt die Kurve
        CHECK(kurvenWert(Kurvenform::Bezier, 0.3) == doctest::Approx(0.3));
        FormPunkt punkt{0.0, 0.0, Kurvenform::Bezier, kBezierVorgabe};
        CHECK(kurvenWert(punkt, 0.2) == doctest::Approx(bezierWert(kBezierVorgabe, 0.2)));
        punkt.form = Kurvenform::SKurve;
        CHECK(kurvenWert(punkt, 0.2) == doctest::Approx(kurvenWert(Kurvenform::SKurve, 0.2)));

        // --- Blenden lesen und setzen
        VisualClip c = clip("A", 10000, 30000);
        Blenden b;
        REQUIRE(liesBlenden(c, b));  // keine Kurve: keine Blenden
        CHECK(b.einMs == 0);
        CHECK(b.ausMs == 0);

        Blenden soll;
        soll.einMs = 3000;
        soll.ausMs = 5000;
        soll.einForm = Kurvenform::Bezier;
        soll.einBezier = {0.1, 0.8, 0.5, 1.0};
        soll.ausForm = Kurvenform::EaseOut;
        setzeBlenden(c, soll);
        REQUIRE(liesBlenden(c, b));
        CHECK(b.einMs == 3000);
        CHECK(b.ausMs == 5000);
        CHECK(b.einForm == Kurvenform::Bezier);
        CHECK(b.einBezier == soll.einBezier);
        CHECK(b.ausForm == Kurvenform::EaseOut);
        CHECK(c.pegelBei(0) == doctest::Approx(0.0));
        CHECK(c.pegelBei(1500) == doctest::Approx(bezierWert(soll.einBezier, 0.5)));
        CHECK(c.pegelBei(1500) > 0.7);  // diese Kurve steigt frueh
        CHECK(c.pegelBei(10000) == doctest::Approx(1.0));
        // die Ausblendung ist dieselbe Form, rueckwaerts gelesen (Entscheid S96)
        CHECK(c.pegelBei(17500) == doctest::Approx(kurvenWert(Kurvenform::EaseOut, 0.5)));
        CHECK(c.pegelBei(20000) == doctest::Approx(0.0));

        // nur eine Seite; zusammen laenger als der Clip; keine
        soll = Blenden{};
        soll.ausMs = 4000;
        setzeBlenden(c, soll);
        REQUIRE(liesBlenden(c, b));
        CHECK(b.einMs == 0);
        CHECK(b.ausMs == 4000);
        soll.einMs = 15000;
        soll.ausMs = 15000;  // zusammen laenger als der Clip
        setzeBlenden(c, soll);
        REQUIRE(liesBlenden(c, b));
        CHECK(b.einMs == 15000);  // eingestellt bleibt eingestellt ...
        CHECK(b.ausMs == 15000);
        CHECK(c.verteilung().ein.bisMs == 10000);  // ... gestaucht wird beim Rechnen
        CHECK(c.pegelBei(5000) == doctest::Approx(0.5));
        setzeBlenden(c, Blenden{});
        CHECK(c.vollerPegel());

        // die Fassung am Clip schreibt dasselbe
        c.setzeBlenden(2000, 2000, Kurvenform::SKurve);
        REQUIRE(liesBlenden(c, b));
        CHECK(b.einMs == 2000);
        CHECK(b.einForm == Kurvenform::SKurve);
        CHECK(b.ausForm == Kurvenform::SKurve);
        // beim Ziehen einer Kante bleiben die Blenden: die Griffe gibt es weiter
        Komposition k;
        Kanal kanal;
        kanal.clips = {c};
        k.kanaele = {kanal};
        REQUIRE(zieheKante(k, 0, 0, false, 50000) == 0);
        REQUIRE(liesBlenden(clips(k)[0], b));
        CHECK(b.einMs == 2000);
        CHECK(b.ausMs == 2000);

        // eine Form mit eigenen Punkten zeigt sich nicht als einfache Blende
        c.ein.form.punkte.insert(c.ein.form.punkte.begin() + 1, FormPunkt{0.5, 0.8});
        CHECK_FALSE(liesBlenden(c, b));

        // --- Grundpegel und Stuecke (Entscheide S96): alle Teile sind freiwillig
        VisualClip d = clip("D", 0, 10000);
        d.grundpegel = 0.0;  // dunkel — und nur im Buckel zu sehen
        d.stuecke = {ClipStueck{0.5, 2000, true, delle()}};
        CHECK(d.pegelBei(0) == doctest::Approx(0.0));
        CHECK(d.pegelBei(4000) == doctest::Approx(0.0));
        CHECK(d.pegelBei(5000) == doctest::Approx(1.0));
        CHECK(d.pegelBei(4500) == doctest::Approx(0.5));  // S-Kurve: zur Haelfte halb
        CHECK(d.pegelBei(9999) == doctest::Approx(0.0));
        d.grundpegel = 1.0;
        d.stuecke[0].gekippt = false;  // die Delle im vollen Pegel
        CHECK(d.pegelBei(3999) == doctest::Approx(1.0));
        CHECK(d.pegelBei(5000) == doctest::Approx(0.0));
        d.grundpegel = 0.5;  // von der Mitte aus: Delle nach unten, Buckel nach oben
        CHECK(d.pegelBei(5000) == doctest::Approx(0.0));
        CHECK(d.pegelBei(1000) == doctest::Approx(0.5));
        d.stuecke[0].gekippt = true;
        CHECK(d.pegelBei(5000) == doctest::Approx(1.0));
        CHECK(d.pegelBei(4000) == doctest::Approx(0.5));  // am Rand ohne Sprung
        d.setzeBlenden(1000, 0);  // die Blende laeuft bis zum Grundpegel
        CHECK(d.pegelBei(500) == doctest::Approx(0.25));
        CHECK(d.pegelBei(1000) == doctest::Approx(0.5));

        // --- alte Dateien: die Punktliste wird uebersetzt, der Verlauf bleibt
        VisualClip alt = clip("A", 0, 20000);
        ausAltenPunkten(alt, {{0, 0.0, Kurvenform::EaseIn}, {4000, 1.0}, {15000, 1.0, Kurvenform::EaseIn},
                              {20000, 0.0}});
        CHECK(alt.ein.dauerMs == 4000);
        CHECK(alt.aus.dauerMs == 5000);
        CHECK(alt.stuecke.empty());
        CHECK(alt.pegelBei(2000) == doctest::Approx(0.25));
        CHECK(alt.pegelBei(17500) == doctest::Approx(0.75));  // fiel mit Ease in: 1 − 0.25
        const BezierGriffe griffe = {0.1, 0.8, 0.5, 1.0};
        ausAltenPunkten(alt, {{16000, 1.0, Kurvenform::Bezier, griffe}, {20000, 0.0}});
        CHECK(alt.pegelBei(17000) == doctest::Approx(1.0 - bezierWert(griffe, 0.25)).epsilon(0.001));
        ausAltenPunkten(alt, {{16000, 1.0, Kurvenform::Exponentiell}, {20000, 0.0}});
        CHECK(alt.pegelBei(18000) == doctest::Approx(1.0 - 0.125).epsilon(0.001));
        ausAltenPunkten(alt, {{16000, 1.0, Kurvenform::Halten}, {20000, 0.0}});
        CHECK(alt.pegelBei(19999) == doctest::Approx(1.0));
        // jede andere Kurve: ein Stueck ueber den ganzen Clip
        ausAltenPunkten(alt, {{0, 0.0}, {4000, 1.0}, {9000, 0.2}, {12000, 1.0}, {20000, 0.0}});
        CHECK(alt.ein.dauerMs == 0);
        REQUIRE(alt.stuecke.size() == 1);
        CHECK(alt.stuecke[0].weiteMs == 20000);
        CHECK(alt.pegelBei(2000) == doctest::Approx(0.5));
        CHECK(alt.pegelBei(9000) == doctest::Approx(0.2));
        CHECK(alt.pegelBei(16000) == doctest::Approx(0.5));
        ausAltenPunkten(alt, {{5000, 0.3}, {10000, 0.9}});
        CHECK(alt.pegelBei(0) == doctest::Approx(0.3));  // vor dem ersten Punkt: dessen Pegel
        CHECK(alt.pegelBei(7500) == doctest::Approx(0.6));
        CHECK(alt.pegelBei(19000) == doctest::Approx(0.9));
        ausAltenPunkten(alt, {});
        CHECK(alt.vollerPegel());

        // --- Datei: die Anfasser stehen nur bei Bézier-Punkten; ohne Feld gilt die Vorgabe
        soll = Blenden{};
        soll.einMs = 3000;
        soll.einForm = Kurvenform::Bezier;
        soll.einBezier = {0.25, 0.5, 0.75, 0.125};
        c = clip("A", 0, 20000);
        setzeBlenden(c, soll);
        c.grundpegel = 0.75;
        c.stuecke = {ClipStueck{0.6, 2500, true, delle()}};
        kanal.clips = {c};
        k.kanaele = {kanal};
        const QJsonObject json = alsJson(k);
        const QJsonObject clipJson = json.value(QStringLiteral("channels")).toArray().at(0).toObject()
                                         .value(QStringLiteral("clips")).toArray().at(0).toObject();
        CHECK_FALSE(clipJson.contains(QStringLiteral("levels")));   // die alte Punktliste gibt es nicht mehr
        CHECK_FALSE(clipJson.contains(QStringLiteral("fadeOut")));  // nur, was der Clip hat
        CHECK(clipJson.value(QStringLiteral("base")).toDouble() == doctest::Approx(0.75));
        CHECK(clipJson.value(QStringLiteral("fadeIn")).toObject().value(QStringLiteral("ms")).toDouble() ==
              doctest::Approx(3000.0));
        CHECK(clipJson.value(QStringLiteral("shapes")).toArray().size() == 1);
        const QJsonArray punkte = clipJson.value(QStringLiteral("fadeIn")).toObject()
                                      .value(QStringLiteral("shape")).toObject()
                                      .value(QStringLiteral("points")).toArray();
        REQUIRE(punkte.size() == 2);
        CHECK(punkte.at(0).toObject().value(QStringLiteral("curve")).toInt() == 6);
        CHECK(punkte.at(0).toObject().value(QStringLiteral("bezier")).toArray().size() == 4);
        CHECK_FALSE(punkte.at(1).toObject().contains(QStringLiteral("bezier")));
        Komposition gelesen;
        REQUIRE(ausJson(json, gelesen));
        REQUIRE(liesBlenden(gelesen.kanaele[0].clips[0], b));
        CHECK(b.einForm == Kurvenform::Bezier);
        CHECK(b.einBezier == soll.einBezier);
        CHECK(gleich(gelesen, k));
        CHECK(QString::fromLatin1(kurvenformName(Kurvenform::Bezier)) == QStringLiteral("Bezier"));
    }

    TEST_CASE("C4 Stuecke: anlegen, finden, schieben und in der Weite ziehen — sie halten an den Nachbarn an")
    {
        VisualClip c = clip("A", 10000, 30000);  // 20 s
        c.setzeBlenden(3000, 5000);
        CHECK(teilBei(c, 1000) == Teil{TeilArt::Ein, -1});
        CHECK(teilBei(c, 10000) == Teil{});
        CHECK(teilBei(c, 16000) == Teil{TeilArt::Aus, -1});
        CHECK_FALSE(hatTeil(c, {TeilArt::Stueck, 0}));

        // ein neues Stueck: eine Delle, 2 s breit, die Mitte an der Stelle
        REQUIRE(fuegeStueckEin(c, 10000) == 0);
        CHECK(c.stuecke[0].lage == doctest::Approx(0.5));
        CHECK(c.stuecke[0].weiteMs == kNeuesStueckMs);
        CHECK(hatTeil(c, {TeilArt::Stueck, 0}));
        CHECK(teilBei(c, 9500) == Teil{TeilArt::Stueck, 0});
        CHECK(bereichVon(c, {TeilArt::Stueck, 0}).vonMs == 9000);
        CHECK(bereichVon(c, {TeilArt::Stueck, 0}).bisMs == 11000);
        // in einem kurzen Clip hoechstens ein Viertel
        VisualClip kurz = clip("K", 0, 4000);
        REQUIRE(fuegeStueckEin(kurz, 2000) == 0);
        CHECK(kurz.stuecke[0].weiteMs == 1000);

        // der Spielraum: von der Einblendung bis zur Ausblendung
        CHECK(spielraumVon(c, 0).vonMs == 3000);
        CHECK(spielraumVon(c, 0).bisMs == 15000);

        // schieben: frei im Spielraum, an den Blenden ist Schluss
        REQUIRE(verschiebeStueck(c, 0, 6000));
        CHECK(bereichVon(c, {TeilArt::Stueck, 0}).vonMs == 5000);
        REQUIRE(verschiebeStueck(c, 0, 500));
        CHECK(bereichVon(c, {TeilArt::Stueck, 0}).vonMs == 3000);  // buendig an der Einblendung
        CHECK_FALSE(c.verteilung().ein.gestaucht);
        REQUIRE(verschiebeStueck(c, 0, 19000));
        CHECK(bereichVon(c, {TeilArt::Stueck, 0}).bisMs == 15000);  // buendig an der Ausblendung
        CHECK_FALSE(c.verteilung().aus.gestaucht);

        // die Weite: die Mitte bleibt, breiter als bis zum naeheren Nachbarn wird es nicht
        REQUIRE(verschiebeStueck(c, 0, 10000));
        REQUIRE(setzeStueckWeite(c, 0, 6000));
        CHECK(bereichVon(c, {TeilArt::Stueck, 0}).vonMs == 7000);
        CHECK(bereichVon(c, {TeilArt::Stueck, 0}).bisMs == 13000);
        REQUIRE(setzeStueckWeite(c, 0, 30000));
        CHECK(c.stuecke[0].weiteMs == 10000);  // rechts sind es 5 s bis zur Ausblendung
        CHECK(bereichVon(c, {TeilArt::Stueck, 0}).bisMs == 15000);
        REQUIRE(setzeStueckWeite(c, 0, 10));
        CHECK(c.stuecke[0].weiteMs == kMinStueckMs);

        // zwei Stuecke: keines schiebt sich durch das andere
        REQUIRE(setzeStueckWeite(c, 0, 2000));  // 9–11 s
        REQUIRE(fuegeStueckEin(c, 13000) == 1);  // 12–14 s
        CHECK(spielraumVon(c, 0).bisMs == 12000);
        CHECK(spielraumVon(c, 1).vonMs == 11000);
        REQUIRE(verschiebeStueck(c, 0, 14000));
        CHECK(bereichVon(c, {TeilArt::Stueck, 0}).bisMs == 12000);
        REQUIRE(verschiebeStueck(c, 1, 4000));
        CHECK(bereichVon(c, {TeilArt::Stueck, 1}).vonMs == 12000);
        CHECK_FALSE(c.verteilung().stuecke[0].gestaucht);

        // ein gestauchtes Stueck anfassen: erst gilt, was zu sehen ist
        VisualClip eng = clip("E", 0, 6000);
        eng.setzeBlenden(4000, 0);
        eng.stuecke = {ClipStueck{0.5, 4000, false, delle()}};  // 1–5 s gegen 0–4 s: beide gestaucht
        REQUIRE(eng.verteilung().ein.gestaucht);
        const std::int64_t einJetzt = eng.verteilung().ein.weiteMs();
        REQUIRE(verschiebeStueck(eng, 0, 5500));
        CHECK(eng.ein.dauerMs == einJetzt);  // die Einblendung behaelt, was sie zeigte
        CHECK_FALSE(eng.verteilung().ein.gestaucht);
        CHECK(bereichVon(eng, {TeilArt::Stueck, 0}).bisMs == 6000);

        // loeschen; was es nicht gibt
        CHECK(loescheStueck(c, 0));
        CHECK(c.stuecke.size() == 1);
        CHECK_FALSE(loescheStueck(c, 5));
        CHECK_FALSE(verschiebeStueck(c, 3, 0));
        CHECK_FALSE(setzeStueckWeite(c, -1, 1000));
    }

    TEST_CASE("C5 Ton-Clips: ein Clip-Typ fuer Bild und Ton — Einsatz, Song-Laenge, Kanten, alte Dateien")
    {
        // ein Audio-Kanal heisst A1; Songs gehen nur in Audio-Kanaele, Presets nur in Visual-Kanaele
        Komposition k = neueKomposition();  // CH1
        REQUIRE(fuegeKanalEin(k, {}, KanalArt::Audio) == 1);
        CHECK(k.kanaele[1].name == QStringLiteral("A1"));
        CHECK(k.kanaele[1].art == KanalArt::Audio);
        CHECK(fuegeKanalEin(k, {}, KanalArt::Audio) == 2);
        CHECK(k.kanaele[2].name == QStringLiteral("A2"));
        CHECK(fuegeSongEin(k, 0, QStringLiteral("s.mp3"), 0, 180000) == -1);
        CHECK(fuegeClipEin(k, 1, QStringLiteral("p.lvfx"), 0) == -1);
        const auto ton = [&k]() -> const std::vector<Clip>& { return k.kanaele[1].clips; };

        // ein Song: so lang wie er ist, Einsatz am Anfang, voller Pegel
        REQUIRE(fuegeSongEin(k, 1, QStringLiteral("eins.mp3"), 10000, 180000) == 0);
        CHECK(ton()[0].startMs == 10000);
        CHECK(ton()[0].endeMs == 190000);
        CHECK(ton()[0].einsatzMs == 0);
        CHECK(ton()[0].quellMs == 180000);
        CHECK(ton()[0].vollerPegel());
        CHECK(k.laenge() == 190000);  // das Ende der Komposition folgt auch dem Ton
        // ohne bekannte Laenge: 20 s
        Komposition ohne = k;
        REQUIRE(fuegeSongEin(ohne, 2, QStringLiteral("x.mp3"), 0, 0) == 0);
        CHECK(ohne.kanaele[2].clips[0].dauerMs() == kNeuerClipMs);
        CHECK(ohne.kanaele[2].clips[0].quellMs == 0);

        // welcher Kanal spielt, und was ist wo zu hoeren
        CHECK(k.tonKanal() == 1);
        CHECK(k.toeneBei(5000).empty());  // Stille vor dem Song
        REQUIRE(k.toeneBei(10000).size() == 1);
        CHECK(k.toeneBei(5000, 6000).size() == 1);  // mit Vorlauf: er beginnt gleich
        CHECK(k.toeneBei(70000)[0]->songStelle(70000 - 10000) == 60000);
        CHECK(k.toeneBei(190000).empty());  // das Ende gehoert nicht mehr dazu
        k.kanaele[1].an = false;
        CHECK(k.tonKanal() == 2);  // der naechste eingeschaltete
        CHECK(k.toeneBei(70000).empty());
        k.kanaele[2].an = false;
        CHECK(k.tonKanal() == -1);
        k.kanaele[1].an = true;

        // die rechte Kante: abschneiden geht, laenger als der Song nicht
        REQUIRE(zieheKante(k, 1, 0, false, 100000) == 0);
        CHECK(ton()[0].endeMs == 100000);
        CHECK(ton()[0].einsatzMs == 0);
        REQUIRE(zieheKante(k, 1, 0, false, 500000) == 0);
        CHECK(ton()[0].endeMs == 190000);

        // die linke Kante: der Song beginnt spaeter — was bleibt, ist an derselben Stelle zu hoeren
        REQUIRE(zieheKante(k, 1, 0, true, 40000) == 0);
        CHECK(ton()[0].startMs == 40000);
        CHECK(ton()[0].einsatzMs == 30000);
        CHECK(ton()[0].endeMs == 190000);
        REQUIRE(k.toeneBei(70000).size() == 1);
        CHECK(k.toeneBei(70000)[0]->songStelle(70000 - 40000) == 60000);  // wie zuvor
        // ... und wieder nach links, aber nicht vor den Anfang des Songs
        REQUIRE(zieheKante(k, 1, 0, true, 0) == 0);
        CHECK(ton()[0].startMs == 10000);
        CHECK(ton()[0].einsatzMs == 0);
        // mit Einsatz reicht die rechte Kante entsprechend weniger weit
        REQUIRE(zieheKante(k, 1, 0, true, 40000) == 0);
        REQUIRE(zieheKante(k, 1, 0, false, 100000) == 0);
        REQUIRE(zieheKante(k, 1, 0, false, 999000) == 0);
        CHECK(ton()[0].endeMs == 40000 + (180000 - 30000));

        // verschieben: der Einsatz bleibt; die Pegelkurve gilt wie am Bild
        REQUIRE(verschiebeClip(k, 1, 0, 0) == 0);
        CHECK(ton()[0].startMs == 0);
        CHECK(ton()[0].einsatzMs == 30000);
        k.kanaele[1].clips[0].setzeBlenden(2000, 4000);
        CHECK(ton()[0].pegelBei(1000) == doctest::Approx(0.5));
        CHECK(ton()[0].pegelBei(150000 - 2000) == doctest::Approx(0.5));

        // ein zweiter Song dahinter; mit Ctrl darauf gezogen wird der erste gekuerzt — und,
        // wenn es seinen Anfang trifft, sein Einsatz vorgerueckt
        REQUIRE(fuegeSongEin(k, 1, QStringLiteral("zwei.mp3"), 200000, 60000) == 1);
        REQUIRE(verschiebeClip(k, 1, 0, 180000, Stoss::Kuerzen) == 0);  // eins liegt jetzt 180–330 s
        REQUIRE(ton().size() == 1);                                     // zwei ist ganz ueberdeckt: weg
        REQUIRE(fuegeSongEin(k, 1, QStringLiteral("drei.mp3"), 100000, 100000, Stoss::Kuerzen) == 0);
        REQUIRE(ton().size() == 2);
        CHECK(ton()[1].startMs == 200000);       // eins beginnt, wo drei endet
        CHECK(ton()[1].einsatzMs == 30000 + 20000);
        CHECK(geordnet(k));

        // Bild und Ton tauschen nicht den Kanal
        REQUIRE(fuegeClipEin(k, 0, QStringLiteral("p.lvfx"), 0) == 0);
        const Komposition vorher = k;
        CHECK(versetzeClip(k, 0, 0, 1, 500000) == -1);
        CHECK(versetzeClip(k, 1, 0, 0, 500000) == -1);
        CHECK(gleich(k, vorher));
        CHECK(versetzeClip(k, 1, 0, 2, 0) == 0);  // Ton zu Ton geht
        CHECK(k.kanaele[2].clips.size() == 1);

        // Datei: gelesen wie geschrieben
        Komposition gelesen;
        REQUIRE(ausJson(alsJson(k), gelesen));
        CHECK(gleich(gelesen, k));

        // eine alte Datei (bis S95): ein Ton-Clip ohne Ende reicht bis zum Schluss seines Songs
        QJsonObject alt = alsJson(neueKomposition());
        QJsonObject kanal;
        kanal.insert(QStringLiteral("kind"), QStringLiteral("audio"));
        kanal.insert(QStringLiteral("name"), QStringLiteral("Audio"));
        QJsonObject clipAlt;
        clipAlt.insert(QStringLiteral("file"), QStringLiteral("C:/musik/alt.mp3"));
        clipAlt.insert(QStringLiteral("start"), 5000);
        clipAlt.insert(QStringLiteral("offset"), 20000);
        kanal.insert(QStringLiteral("clips"), QJsonArray{clipAlt});
        QJsonArray kanaele = alt.value(QStringLiteral("channels")).toArray();
        kanaele.append(kanal);
        alt.insert(QStringLiteral("channels"), kanaele);
        const SongLaenge laenge = [](const QString& datei) -> std::int64_t {
            return datei.endsWith(QStringLiteral("alt.mp3")) ? 120000 : 0;
        };
        REQUIRE(ausJson(alt, gelesen, {}, nullptr, laenge));
        REQUIRE(gelesen.kanaele[1].clips.size() == 1);
        CHECK(gelesen.kanaele[1].clips[0].startMs == 5000);
        CHECK(gelesen.kanaele[1].clips[0].quellMs == 120000);
        CHECK(gelesen.kanaele[1].clips[0].endeMs == 5000 + (120000 - 20000));
        // ohne Auskunft ueber die Laenge: eine Sekunde, gemeldet
        QStringList bericht;
        REQUIRE(ausJson(alt, gelesen, {}, &bericht));
        CHECK(gelesen.kanaele[1].clips[0].dauerMs() == 1000);
        CHECK(bericht.join(QLatin1Char('\n')).contains(QStringLiteral("had no length")));
        // ein Clip, der laenger ist als sein Song, wird beim Ergaenzen gekuerzt
        Komposition lang = neueKomposition();
        (void)fuegeKanalEin(lang, {}, KanalArt::Audio);
        REQUIRE(fuegeSongEin(lang, 1, QStringLiteral("C:/musik/alt.mp3"), 0, 0) == 0);  // Laenge unbekannt: 20 s
        lang.kanaele[1].clips[0].endeMs = 500000;
        lang.ergaenzeTon(laenge);
        CHECK(lang.kanaele[1].clips[0].quellMs == 120000);
        CHECK(lang.kanaele[1].clips[0].endeMs == 120000);
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
        zweiter.clips = {clip("X", 5000, 25000)};
        k.kanaele.push_back(zweiter);
        REQUIRE(verschiebeClip(k, 0, 0, 15000, Stoss::Kuerzen) == 0);
        CHECK(k.kanaele[1].clips[0].startMs == 5000);
        CHECK(k.kanaele[1].clips[0].endeMs == 25000);
        REQUIRE(verschiebeClip(k, 1, 0, 0) == 0);
        CHECK(bild(k) == QStringLiteral("A[15000,25000) B[25000,30000) C[30000,45000)"));
    }

    TEST_CASE("S98 Kopie einfuegen: der Clip kommt mit allem, sein Anfang an die Stelle — nur in einen Kanal seiner Art")
    {
        Komposition k = neueKomposition();  // CH1
        REQUIRE(fuegeKanalEin(k, {}, KanalArt::Audio) == 1);
        REQUIRE(fuegeClipEin(k, 0, QStringLiteral("a.lvfx"), 0, 10000) == 0);
        REQUIRE(fuegeClipEin(k, 0, QStringLiteral("b.lvfx"), 20000, 10000) == 1);
        Clip vorlage = k.kanaele[0].clips[0];
        vorlage.setzeBlenden(2000, 3000);
        vorlage.grundpegel = 0.7;
        vorlage.mischart = Mischart::Additive;

        // in freie Flaeche: der Anfang liegt, wo er hin soll; Laenge, Kurve und Mischart kommen mit
        REQUIRE(fuegeKopieEin(k, 0, KanalArt::Visual, vorlage, 40000) == 2);
        const Clip& kopie = k.kanaele[0].clips[2];
        CHECK(kopie.startMs == 40000);
        CHECK(kopie.endeMs == 50000);
        CHECK(kopie.datei == QStringLiteral("a.lvfx"));
        CHECK(kopie.grundpegel == doctest::Approx(0.7));
        CHECK(kopie.mischart == Mischart::Additive);
        CHECK(kopie.pegelBei(1000) == doctest::Approx(0.35));  // die Einblendung haengt an seiner Kante
        CHECK(k.kanaele[0].clips[0].vollerPegel());            // das Original bleibt, wie es war

        // die Luecke 10–20 s ist genau so lang wie er: dort passt er hin
        REQUIRE(fuegeKopieEin(k, 0, KanalArt::Visual, vorlage, 10000) == 1);
        CHECK(k.kanaele[0].clips[1].startMs == 10000);
        // auf einem Clip: er wird nicht gekuerzt, sondern landet in der naechsten Luecke, in die er passt
        const int platz = fuegeKopieEin(k, 0, KanalArt::Visual, vorlage, 5000);
        REQUIRE(platz >= 0);
        CHECK(k.kanaele[0].clips[static_cast<std::size_t>(platz)].dauerMs() == 10000);
        CHECK(k.kanaele[0].clips[static_cast<std::size_t>(platz)].startMs == 30000);

        // ein Bild-Clip geht nicht in einen Audio-Kanal, und umgekehrt; einen Kanal, den es nicht gibt, auch nicht
        CHECK(fuegeKopieEin(k, 1, KanalArt::Visual, vorlage, 0) == -1);
        CHECK(fuegeKopieEin(k, 0, KanalArt::Audio, vorlage, 0) == -1);
        CHECK(fuegeKopieEin(k, 7, KanalArt::Visual, vorlage, 0) == -1);
        CHECK(k.kanaele[1].clips.empty());

        // ein Song behaelt Einsatz und Songlaenge
        REQUIRE(fuegeSongEin(k, 1, QStringLiteral("s.mp3"), 0, 60000) == 0);
        Clip song = k.kanaele[1].clips[0];
        song.einsatzMs = 15000;
        song.endeMs = 30000;
        REQUIRE(fuegeKopieEin(k, 1, KanalArt::Audio, song, 100000) == 1);
        CHECK(k.kanaele[1].clips[1].einsatzMs == 15000);
        CHECK(k.kanaele[1].clips[1].quellMs == 60000);
        CHECK(k.kanaele[1].clips[1].songStelle(0) == 15000);
        CHECK(k.kanaele[1].clips[1].dauerMs() == 30000);
    }
}
