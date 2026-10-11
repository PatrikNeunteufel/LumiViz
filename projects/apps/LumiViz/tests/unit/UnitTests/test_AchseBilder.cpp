/**
 * @file test_AchseBilder.cpp
 * @brief Die Zeitachse des Composers in vielen Zustaenden als Bild — der Beleg
 *        fuer einen Umbau ohne Verhaltensaenderung (S109: die gemeinsame Basis
 *        `Achse` wird aus der `Zeitachse` herausgezogen).
 *
 * Der Fall zeichnet die Achse ohne Fenster (`grab`) und prueft selbst nur, dass
 * jedes Bild entsteht. Mit `LUMIVIZ_ACHSE_BILDER=<Ordner>` legt er je Zustand
 * ein PNG und in `zustand.txt` die Zahlen der Geometrie ab. Verglichen wird von
 * aussen: ein Lauf vor dem Umbau, einer danach, die Dateien muessen bytegleich
 * sein. Feste Vergleichsbilder liegen nicht im Repo — sie haengen an Schrift
 * und Skalierung des Rechners.
 */

#include <doctest.h>

#include "composer/Bearbeiten.hpp"
#include "composer/ComposerPanel.hpp"
#include "composer/Komposition.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QMouseEvent>
#include <QPixmap>
#include <QScrollBar>
#include <QTextStream>
#include <QWheelEvent>

#include <cmath>
#include <functional>
#include <memory>
#include <vector>

using namespace lumi::composer;

namespace
{

Clip clipVon(const QString& name, std::int64_t start, std::int64_t ende)
{
    Clip c;
    c.datei = QStringLiteral("C:/lumiviz-test/") + name;
    c.startMs = start;
    c.endeMs = ende;
    return c;
}

/// Zehn Kanaele (Bild, Ton, einer aus), Blenden, Stuecke, alle vier
/// Master-Kurven, eine Ueberlappung fuer die Schleife.
Komposition probe()
{
    Komposition k;
    Kanal ch1;
    ch1.name = QStringLiteral("CH1");
    ch1.clips = {clipVon(QStringLiteral("Rot.lvfx"), 0, 20000), clipVon(QStringLiteral("Blau.lvfx"), 30000, 60000)};
    ch1.clips[0].setzeBlenden(3000, 5000);
    ClipStueck stueck;
    stueck.lage = 0.5;
    stueck.weiteMs = 6000;
    ch1.clips[1].stuecke.push_back(stueck);
    ch1.clips[1].grundpegel = 0.8;

    Kanal ch2;
    ch2.name = QStringLiteral("CH2");
    ch2.clips = {clipVon(QStringLiteral("Ein langer Name fuer ein Preset.milk"), 15000, 35000)};
    ch2.clips[0].mischart = Mischart::Additive;

    Kanal a1;
    a1.art = KanalArt::Audio;
    a1.name = QStringLiteral("A1");
    a1.clips = {clipVon(QStringLiteral("Song.mp3"), 5000, 50000)};
    a1.clips[0].einsatzMs = 2000;
    a1.clips[0].quellMs = 60000;
    a1.clips[0].setzeBlenden(2000, 8000);

    Kanal ch3;
    ch3.name = QStringLiteral("CH3");
    ch3.an = false;
    ch3.clips = {clipVon(QStringLiteral("Aus.avs"), 10000, 25000)};

    k.kanaele = {ch1, ch2, a1, ch3};
    for (int i = 4; i < 10; ++i)
    {
        Kanal weiter;
        weiter.name = QStringLiteral("CH%1").arg(i);
        weiter.clips = {clipVon(QStringLiteral("Nr%1.lvfx").arg(i), i * 3000, i * 3000 + 12000)};
        k.kanaele.push_back(weiter);
    }
    k.vergibFarben();

    k.einblendeMs = 4000;
    k.ausblendeMs = 6000;
    GesamtStueck gesamt;
    gesamt.mitteMs = 20000;
    gesamt.weiteMs = 5000;
    k.gesamtStuecke.push_back(gesamt);
    k.bildKurve.grundpegel = 0.7;
    k.tonKurve.einblendeMs = 2000;
    GesamtStueck karaoke;
    karaoke.mitteMs = 40000;
    karaoke.weiteMs = 8000;
    k.karaokeKurve.stuecke.push_back(karaoke);
    k.ueberlappungMs = 8000;
    return k;
}

/// Eine gerechnete Huellkurve fuer den Song (60 s, ein Wert je 50 ms).
std::vector<float> huelle()
{
    std::vector<float> werte(1200);
    for (std::size_t i = 0; i < werte.size(); ++i)
        werte[i] = 0.15f + 0.8f * static_cast<float>(std::abs(std::sin(static_cast<double>(i) * 0.037)));
    return werte;
}

void maus(QWidget& ziel, QEvent::Type art, QPoint ort, Qt::MouseButton taste, Qt::MouseButtons tasten)
{
    QMouseEvent ereignis(art, QPointF(ort), QPointF(ziel.mapToGlobal(ort)), taste, tasten, Qt::NoModifier);
    QCoreApplication::sendEvent(&ziel, &ereignis);
}

void klick(QWidget& ziel, QPoint ort)
{
    maus(ziel, QEvent::MouseButtonPress, ort, Qt::LeftButton, Qt::LeftButton);
    maus(ziel, QEvent::MouseButtonRelease, ort, Qt::LeftButton, Qt::NoButton);
}

void rad(QWidget& ziel, QPoint ort, int raster, Qt::KeyboardModifiers tasten, bool quer = false)
{
    QWheelEvent ereignis(QPointF(ort), QPointF(ziel.mapToGlobal(ort)), QPoint(),
                         quer ? QPoint(raster, 0) : QPoint(0, raster), Qt::NoButton, tasten, Qt::NoScrollPhase,
                         false);
    QCoreApplication::sendEvent(&ziel, &ereignis);
}

struct Zustand
{
    const char* name;
    std::function<void(Zeitachse&)> stelleEin;
};

}  // namespace

TEST_SUITE("Achse")
{
    TEST_CASE("Zeitachse des Composers: jeder Zustand ergibt ein Bild (Beleg fuer den Umbau auf die Basis)")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};

        const Komposition k = probe();
        const QString ordner = qEnvironmentVariable("LUMIVIZ_ACHSE_BILDER");
        if (!ordner.isEmpty()) REQUIRE(QDir().mkpath(ordner));
        QString zahlen;
        QTextStream text(&zahlen);

        const auto frisch = [&](int breit = 900, int hoch = 320) {
            auto achse = std::make_unique<Zeitachse>();
            achse->resize(breit, hoch);
            achse->setHuelle(k.kanaele[2].clips[0].datei, huelle(), 50);
            achse->setKomposition(k, true);
            achse->setAn(true);
            achse->setStelle(12000);
            (void)achse->grab();  // ohne Fenster kommt das erste resizeEvent erst hier
            return achse;
        };

        const auto halteFest = [&](const char* name, Zeitachse& achse, QWidget& bild) {
            const QImage gezeichnet = bild.grab().toImage();
            CHECK_FALSE(gezeichnet.isNull());
            text << name << ": rollX " << achse.rollX() << " / " << achse.rollXMax() << ", rollY " << achse.rollY()
                 << " / " << achse.rollYMax() << ", msJePixel " << QString::number(achse.msJePixel(), 'g', 12)
                 << ", ganz " << achse.zeitGanz() << ", prozent " << achse.zeilenProzent() << ", verstellt "
                 << achse.zoomVerstellt() << ", inhalt " << achse.inhaltHoehe() << ", kanalHoch "
                 << achse.kanalHoch() << ", masterHoch " << achse.masterHoch() << ", xVon";
            for (const std::int64_t ms : {std::int64_t{0}, std::int64_t{8000}, std::int64_t{30000}, std::int64_t{60000}})
                text << ' ' << achse.xVon(ms);
            text << ", stelleVon";
            for (const int x : {0, Zeitachse::kKopfBreite, 300, 600, 899}) text << ' ' << achse.stelleVon(x);
            text << ", zeileBei";
            for (const int y : {5, 30, 60, 100, 150, 250, 319}) text << ' ' << achse.zeileBei(y);
            text << ", zeileOben";
            for (const int zeile : {-4, -1, 0, 3, 9}) text << ' ' << achse.zeileOben(zeile);
            text << ", yVonZeile " << achse.yVonZeile(0) << ' ' << achse.yVonZeile(5) << ", kanalBei "
                 << achse.kanalBei(150) << ", auswahl " << achse.gewaehlterKanal() << ' ' << achse.gewaehlterClip()
                 << ", stelle " << achse.stelleMs() << '\n';
            if (!ordner.isEmpty())
                CHECK(gezeichnet.save(QDir(ordner).filePath(QString::fromLatin1(name) + QStringLiteral(".png"))));
        };

        const std::vector<Zustand> zustaende = {
            {"01_ganz", [](Zeitachse&) {}},
            {"02_composer_aus", [](Zeitachse& a) { a.setAn(false); }},
            {"03_zoom4", [](Zeitachse& a) { a.zoomeZeit(4.0, a.xVon(8000)); }},
            {"04_zoom_gerollt",
             [](Zeitachse& a) {
                 a.zoomeZeit(6.0, a.xVon(8000));
                 a.setRollX(400);
             }},
            {"05_feinster", [](Zeitachse& a) { a.zoomeZeit(1e9, a.xVon(8000)); }},
            {"06_rollY", [](Zeitachse& a) { a.setRollY(90); }},
            {"07_rollY_anschlag", [](Zeitachse& a) { a.setRollY(99999); }},
            {"08_hoehe50", [](Zeitachse& a) { a.setZeilenProzent(50); }},
            {"09_hoehe300_fest", [](Zeitachse& a) { a.setZeilenProzent(300, 200); }},
            {"10_hoehe60_zoom",
             [](Zeitachse& a) {
                 a.setZeilenProzent(60, 120);
                 a.zoomeZeit(3.0, 500);
             }},
            {"11_bearbeiten", [](Zeitachse& a) { a.setBearbeiten(true); }},
            {"12_bearbeiten_gewaehlt",
             [](Zeitachse& a) {
                 a.setBearbeiten(true);
                 klick(a, QPoint(a.xVon(45000), a.yVonZeile(0)));
                 a.setTeil(Teil{TeilArt::Stueck, 0});
             }},
            {"13_master_gewaehlt",
             [](Zeitachse& a) {
                 a.setBearbeiten(true);
                 a.waehleGesamt();
             }},
            {"14_marke",
             [](Zeitachse& a) {
                 a.setBearbeiten(true);
                 a.setMarke(1, 40000);
             }},
            {"15_schleife",
             [](Zeitachse& a) {
                 a.setStelle(3000);
                 a.setSchleife(true, 55000);
             }},
            {"16_mitlaufen_stetig",
             [](Zeitachse& a) {
                 a.zoomeZeit(8.0, a.xVon(0));
                 a.setMitlaufen(Zeitachse::Mitlaufen::Stetig);
                 a.setStelle(30000);
             }},
            {"17_mitlaufen_seitenweise",
             [](Zeitachse& a) {
                 a.zoomeZeit(8.0, a.xVon(0));
                 a.setMitlaufen(Zeitachse::Mitlaufen::Seitenweise);
                 a.setStelle(30000);
             }},
            {"18_mitlaufen_gehalten",
             [](Zeitachse& a) {
                 a.zoomeZeit(8.0, a.xVon(0));
                 a.setMitlaufen(Zeitachse::Mitlaufen::Stetig);
                 a.halteMitlaufen(true);
                 a.setStelle(30000);
             }},
            {"19_rad_ctrl_alt", [](Zeitachse& a) { rad(a, QPoint(400, 150), 240, Qt::ControlModifier | Qt::AltModifier); }},
            {"20_rad_ctrl", [](Zeitachse& a) { rad(a, QPoint(620, 150), 360, Qt::ControlModifier); }},
            {"21_rad_alt", [](Zeitachse& a) { rad(a, QPoint(400, 200), -240, Qt::AltModifier); }},
            {"22_rad_rollen",
             [](Zeitachse& a) {
                 a.zoomeZeit(4.0, a.xVon(0));
                 rad(a, QPoint(400, 150), -240, Qt::ShiftModifier);
                 rad(a, QPoint(400, 150), -120, Qt::NoModifier, true);
                 rad(a, QPoint(400, 150), -120, Qt::NoModifier);
             }},
            {"23_schwenken",
             [](Zeitachse& a) {
                 a.zoomeZeit(4.0, a.xVon(0));
                 a.setRollX(200);
                 maus(a, QEvent::MouseButtonPress, QPoint(500, 150), Qt::MiddleButton, Qt::MiddleButton);
                 maus(a, QEvent::MouseMove, QPoint(430, 120), Qt::NoButton, Qt::MiddleButton);
                 maus(a, QEvent::MouseButtonRelease, QPoint(430, 120), Qt::MiddleButton, Qt::NoButton);
             }},
            {"24_fit_nach_zoom",
             [](Zeitachse& a) {
                 a.zoomeZeit(5.0, 400);
                 a.setZeilenProzent(180);
                 a.setzeZoomZurueck();
             }},
            {"25_sprung_im_lineal",
             [](Zeitachse& a) {
                 QObject::connect(&a, &Zeitachse::sprung, &a, [&a](qint64 ms) { a.setStelle(ms); });
                 a.zoomeZeit(3.0, 500);
                 klick(a, QPoint(640, 10));
             }},
        };
        for (const Zustand& zustand : zustaende)
        {
            auto achse = frisch();
            zustand.stelleEin(*achse);
            halteFest(zustand.name, *achse, *achse);
        }

        // --- andere Kompositionen und Groessen
        {
            auto achse = frisch();
            Komposition fest = k;
            fest.laengeMs = 80000;
            achse->setKomposition(fest, true);
            achse->setAn(true);
            halteFest("30_festes_ende", *achse, *achse);
        }
        {
            auto achse = frisch();
            achse->setKomposition({}, false);
            halteFest("31_nichts_geladen", *achse, *achse);
        }
        {
            auto achse = frisch();
            achse->setKomposition({}, true);
            achse->setBearbeiten(true);
            halteFest("32_leer_bearbeiten", *achse, *achse);
        }
        {
            auto achse = frisch(420, 200);
            achse->zoomeZeit(2.0, 200);
            halteFest("33_schmal", *achse, *achse);
        }
        {
            // hineingezoomt, dann breiter: passt wieder alles, ist sie wieder ganz
            auto achse = frisch(600, 320);
            achse->zoomeZeit(1.2, achse->xVon(5000));
            achse->resize(1100, 320);
            halteFest("34_breiter", *achse, *achse);
        }
        {
            // am Anschlag gerollt, dann hoeher: das Rollen kommt in seine neuen Grenzen
            auto achse = frisch(900, 320);
            achse->setRollY(99999);
            achse->resize(900, 420);
            halteFest("35_hoeher", *achse, *achse);
        }

        // --- der Rahmen mit den Rollbalken
        {
            auto* achse = frisch().release();  // der Rahmen uebernimmt sie
            ZeitachsenRahmen rahmen(achse);
            rahmen.resize(900, 320);
            (void)rahmen.grab();
            achse->zoomeZeit(4.0, achse->xVon(8000));
            achse->setRollY(40);
            halteFest("40_rahmen", *achse, rahmen);
            text << "40_rahmen: senkrecht " << rahmen.balkenSenkrecht()->maximum() << ' '
                 << rahmen.balkenSenkrecht()->value() << ' ' << rahmen.balkenSenkrecht()->pageStep()
                 << ", waagrecht " << rahmen.balkenWaagrecht()->maximum() << ' ' << rahmen.balkenWaagrecht()->value()
                 << ' ' << rahmen.balkenWaagrecht()->pageStep() << '\n';
            rahmen.balkenWaagrecht()->setValue(120);
            rahmen.balkenSenkrecht()->setValue(10);
            halteFest("41_rahmen_balken", *achse, rahmen);
        }

        text.flush();
        if (!ordner.isEmpty())
        {
            QFile datei(QDir(ordner).filePath(QStringLiteral("zustand.txt")));
            REQUIRE(datei.open(QIODevice::WriteOnly | QIODevice::Truncate));
            datei.write(zahlen.toUtf8());
            MESSAGE("Bilder der Achse abgelegt in " << ordner.toStdString());
        }
    }
}
