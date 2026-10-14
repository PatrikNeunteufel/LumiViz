/**
 * @file test_ShapeBild.cpp
 * @brief Klang aus einer Form, Stufe 20c: Konturen aus einem Bild
 *        (shape/BildKonturen.hpp) und Open image... mit From image im Fenster
 *        Edit shape (docs/audio/Shape_Konzept.md §8, §8a; Entscheide Patrik
 *        K1 a, K2 a, S120, und R1 a, R2 a, S122)
 *
 * Die Bilder malt der Test selbst. Ohne Ton und ohne gezeigtes Fenster.
 * `LUMIVIZ_SHAPE_BILD_FENSTER=<datei.png>` legt ein Bild des Fensters mit
 * einem Bild ab; `LUMIVIZ_SHAPE_BILD_PROBE=<bilddatei>` erkennt zusaetzlich
 * ein eigenes Bild und legt es daneben als `_probe.png` ab (die Regler wie
 * vorgegeben).
 */

#include <doctest.h>

#include "loop/FormFenster.hpp"
#include "shape/BildKonturen.hpp"
#include "shape/Kontur.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QDir>
#include <QGroupBox>
#include <QImage>
#include <QPainter>
#include <QPushButton>
#include <QTemporaryDir>
#include <QTreeWidget>

#include <cmath>
#include <vector>

using namespace lumi::loop;
namespace shape = lumi::shape;

namespace
{

constexpr double kPi = 3.14159265358979323846;

/// Weiss mit einem schwarzen Kreis (Mitte 50,50, Radius 30) und einem schwarzen Quadrat
/// (110…170, 20…80) mit einem weissen Loch (130…150, 40…60).
QImage kreisUndQuadrat()
{
    QImage bild(200, 100, QImage::Format_ARGB32);
    bild.fill(Qt::white);
    QPainter p(&bild);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::black);
    p.drawEllipse(QPointF(50.0, 50.0), 30.0, 30.0);
    p.fillRect(110, 20, 60, 60, Qt::black);
    p.fillRect(130, 40, 20, 20, Qt::white);
    return bild;
}

shape::BildEinstellung genau()
{
    shape::BildEinstellung e;
    e.teilung = shape::Teilung::Helligkeit;
    e.stufen = 2;
    e.glaetten = 0;
    e.mindestProzent = 0.0;
    e.detail = 100;
    return e;
}

int ecken(const QPainterPath& pfad)
{
    return pfad.elementCount();
}

}  // namespace

TEST_CASE("Shape Bild: Helligkeit — Flaechen, ihre Raender, Loecher, der Bildrand, die groesste mit Haken")
{
    const shape::BildKonturen k = shape::bildKonturen(shape::bildZumErkennen(kreisUndQuadrat()), genau());
    CHECK(k.groesse == QSize(200, 100));
    CHECK(k.weggefallen == 0);
    // der Rand des Bilds (um den weissen Grund), das Quadrat, der Kreis, das Loch — ein geteilter Rand kommt einmal
    REQUIRE(k.konturen.size() == 4);
    CHECK(k.konturen[0].rand);
    CHECK(k.konturen[0].flaeche == doctest::Approx(20000.0));
    CHECK(k.konturen[0].pfad.boundingRect() == QRectF(0.0, 0.0, 200.0, 100.0));
    CHECK_FALSE(k.konturen[0].gewaehlt);  // am Rand: gezeigt, aber anfangs ohne Haken (K2 a)
    CHECK_FALSE(k.konturen[1].rand);
    CHECK(k.konturen[1].flaeche == doctest::Approx(3600.0));
    CHECK(k.konturen[1].pfad.boundingRect() == QRectF(110.0, 20.0, 60.0, 60.0));
    CHECK(k.konturen[1].gewaehlt);  // die groesste, die nicht an den Rand stoesst
    CHECK(k.konturen[2].flaeche == doctest::Approx(kPi * 900.0).epsilon(0.03));
    CHECK(k.konturen[2].pfad.boundingRect().center().x() == doctest::Approx(50.0).epsilon(0.02));
    CHECK_FALSE(k.konturen[2].gewaehlt);
    CHECK(k.konturen[3].flaeche == doctest::Approx(400.0));
    CHECK(k.konturen[3].pfad.boundingRect() == QRectF(130.0, 40.0, 20.0, 20.0));
    // nur Strecken (D4 b), geschlossen
    for (const shape::BildKontur& kontur : k.konturen)
        for (int i = 0; i < kontur.pfad.elementCount(); ++i) CHECK(kontur.pfad.elementAt(i).type != QPainterPath::CurveToElement);
    CHECK(shape::pfadAlsText(k.konturen[1].pfad).endsWith(QStringLiteral("Z")));
}

TEST_CASE("Shape Bild: Farbe, Deckkraft, Min size, Detail, Smooth, mehr als 64, die Einstellung als JSON")
{
    // --- Farbe: drei Streifen und ein weisses Feld im gruenen — jede Farbe eine Flaeche
    {
        QImage bild(150, 60, QImage::Format_ARGB32);
        QPainter p(&bild);
        p.fillRect(0, 0, 50, 60, QColor(220, 30, 30));
        p.fillRect(50, 0, 50, 60, QColor(30, 200, 30));
        p.fillRect(100, 0, 50, 60, QColor(30, 30, 220));
        p.fillRect(60, 20, 30, 20, Qt::white);
        p.end();
        shape::BildEinstellung e = genau();
        e.teilung = shape::Teilung::Farbe;
        e.stufen = 4;
        const shape::BildKonturen k = shape::bildKonturen(shape::bildZumErkennen(bild), e);
        REQUIRE(k.konturen.size() == 4);
        int amRand = 0;
        for (const shape::BildKontur& kontur : k.konturen) amRand += kontur.rand ? 1 : 0;
        CHECK(amRand == 3);
        CHECK(k.konturen[3].pfad.boundingRect() == QRectF(60.0, 20.0, 30.0, 20.0));
        CHECK(k.konturen[3].gewaehlt);
    }
    // --- Deckkraft: ein deckender Kreis auf durchsichtigem Grund
    {
        QImage bild(100, 100, QImage::Format_ARGB32);
        bild.fill(Qt::transparent);
        QPainter p(&bild);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 255, 255));  // weiss auf durchsichtig: nach Helligkeit nicht zu finden
        p.drawEllipse(QPointF(50.0, 50.0), 25.0, 25.0);
        p.end();
        shape::BildEinstellung e = genau();
        e.teilung = shape::Teilung::Deckkraft;
        const shape::BildKonturen k = shape::bildKonturen(shape::bildZumErkennen(bild), e);
        REQUIRE(k.konturen.size() == 2);
        CHECK(k.konturen[1].gewaehlt);
        CHECK(k.konturen[1].flaeche == doctest::Approx(kPi * 625.0).epsilon(0.04));
        e.teilung = shape::Teilung::Helligkeit;
        CHECK(shape::bildKonturen(shape::bildZumErkennen(bild), e).konturen.size() == 1);  // nur der Bildrand
    }
    // --- Min size: ein Punkt von 3 × 3 geht im weissen Grund auf
    {
        QImage bild = kreisUndQuadrat();
        QPainter p(&bild);
        p.fillRect(100, 90, 3, 3, Qt::black);
        p.end();
        shape::BildEinstellung e = genau();
        CHECK(shape::bildKonturen(shape::bildZumErkennen(bild), e).konturen.size() == 5);
        e.mindestProzent = 0.5;  // 100 Punkte
        CHECK(shape::bildKonturen(shape::bildZumErkennen(bild), e).konturen.size() == 4);
    }
    // --- Detail: weniger Ecken am Kreis
    {
        shape::BildEinstellung e = genau();
        const shape::BildKonturen fein = shape::bildKonturen(shape::bildZumErkennen(kreisUndQuadrat()), e);
        e.detail = 0;
        const shape::BildKonturen grob = shape::bildKonturen(shape::bildZumErkennen(kreisUndQuadrat()), e);
        INFO("Ecken des Kreises: " << ecken(fein.konturen[2].pfad) << " gegen " << ecken(grob.konturen[2].pfad));
        CHECK(ecken(grob.konturen[2].pfad) < ecken(fein.konturen[2].pfad));
        CHECK(ecken(grob.konturen[2].pfad) >= 4);
        CHECK(e.toleranz() == doctest::Approx(5.0));
        e.detail = 100;
        CHECK(e.toleranz() == doctest::Approx(0.75));
    }
    // --- Smooth: ein feines Schachbrett wird zu einer Flaeche
    {
        QImage bild(120, 120, QImage::Format_ARGB32);
        bild.fill(Qt::white);
        for (int y = 30; y < 90; ++y)
            for (int x = 30; x < 90; ++x)
                if (((x / 2) + (y / 2)) % 2 == 0) bild.setPixel(x, y, qRgb(0, 0, 0));
        shape::BildEinstellung e = genau();
        e.stufen = 3;  // weichgezeichnet liegt das Brett bei 0,5 — bei zwei Stufen genau auf der Schwelle
        const std::size_t ohne = shape::bildKonturen(shape::bildZumErkennen(bild), e).konturen.size();
        e.glaetten = 3;
        const std::size_t mit = shape::bildKonturen(shape::bildZumErkennen(bild), e).konturen.size();
        INFO("Konturen ohne Smooth " << ohne << ", mit " << mit);
        CHECK(ohne >= 64);
        CHECK(mit <= 4);
    }
    // --- mehr als 64: hundert Quadrate — die groessten bleiben, die anderen zaehlen
    {
        QImage bild(100, 100, QImage::Format_ARGB32);
        bild.fill(Qt::white);
        QPainter p(&bild);
        for (int i = 0; i < 100; ++i) p.fillRect((i % 10) * 10 + 2, (i / 10) * 10 + 2, 6, 6, Qt::black);
        p.end();
        const shape::BildKonturen k = shape::bildKonturen(shape::bildZumErkennen(bild), genau());
        CHECK(k.konturen.size() == static_cast<std::size_t>(shape::kKonturenMax));
        CHECK(k.weggefallen == 101 - shape::kKonturenMax);
        CHECK(k.konturen.front().rand);  // der Grund ist am groessten
    }
    // --- verkleinert auf hoechstens 1024 Kante
    CHECK(shape::bildZumErkennen(QImage(2000, 1000, QImage::Format_RGB32)).size() == QSize(1024, 512));
    CHECK(shape::bildZumErkennen(QImage(300, 200, QImage::Format_RGB32)).size() == QSize(300, 200));
    CHECK(shape::bildZumErkennen(QImage()).isNull());
    // --- die Einstellung als JSON, in ihren Grenzen
    shape::BildEinstellung e;
    e.teilung = shape::Teilung::Deckkraft;
    e.stufen = 5;
    e.glaetten = 4;
    e.mindestProzent = 1.5;
    e.detail = 20;
    shape::BildEinstellung zurueck;
    zurueck.ausJson(e.alsJson());
    CHECK(zurueck == e);
    zurueck.ausJson(QJsonObject{{QStringLiteral("levels"), 99}, {QStringLiteral("detail"), -5}, {QStringLiteral("splitBy"), QStringLiteral("colour")}});
    CHECK(zurueck.stufen == 8);
    CHECK(zurueck.detail == 0);
    CHECK(zurueck.teilung == shape::Teilung::Farbe);
}

TEST_CASE("Shape Bild im Fenster: Apply live — die Regler ersetzen nur die Konturen aus dem Bild, Undo, edge, Add")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};

    FormFenster fenster;
    fenster.flaeche()->resize(400, 400);
    fenster.setzeLive(true);
    auto* ausBild = fenster.findChild<QGroupBox*>(QStringLiteral("shapeFromImage"));
    auto* zeigeBild = fenster.findChild<QCheckBox*>(QStringLiteral("shapeShowImage"));
    REQUIRE(ausBild != nullptr);
    CHECK_FALSE(ausBild->isEnabled());  // ohne Bild
    CHECK_FALSE(fenster.hatBild());
    CHECK(fenster.findChild<QPushButton*>(QStringLiteral("shapeOpenImage"))->isEnabled());

    fenster.setzeBildEinstellung(genau());
    REQUIRE(fenster.nimmBild(kreisUndQuadrat(), QStringLiteral("probe.png"), FormFenster::Einfuegen::Hinzufuegen) == 4);
    const shape::Zeichnung& z = fenster.zeichnung();
    CHECK(fenster.hatBild());
    CHECK(ausBild->isEnabled());
    CHECK(zeigeBild->isEnabled());
    CHECK(z.herkunft.art == shape::HerkunftArt::Bild);
    CHECK(z.herkunft.quelle == QStringLiteral("probe.png"));
    CHECK(z.herkunft.einstellungen.value(QStringLiteral("levels")).toInt() == 2);
    REQUIRE(z.konturen.size() == 4);
    CHECK(z.konturen[1].pfad.boundingRect() == QRectF(110.0, 20.0, 60.0, 60.0));  // in den Punkten des Bilds
    CHECK(fenster.amRand(z.konturen[0].kennung));
    CHECK(fenster.liste()->topLevelItem(0)->text(1) == QStringLiteral("edge"));
    CHECK(fenster.liste()->topLevelItem(1)->checkState(0) == Qt::Checked);
    CHECK(fenster.flaeche()->ansicht().contains(QRectF(0.0, 0.0, 200.0, 100.0)));  // das Bild passt hinein
    for (const shape::Kontur& k : z.konturen) CHECK(fenster.ausBild(k.kennung));

    // --- eine gezeichnete Kontur dazu; ein Regler ersetzt nur die aus dem Bild
    QPainterPath dreieck;
    dreieck.moveTo(10.0, 10.0);
    dreieck.lineTo(30.0, 10.0);
    dreieck.lineTo(20.0, 30.0);
    dreieck.closeSubpath();
    const int gezeichnet = fenster.fuegeKonturHinzu(dreieck);
    REQUIRE(gezeichnet == 5);
    CHECK_FALSE(fenster.ausBild(gezeichnet));
    const int alteKreis = z.konturen[2].kennung;
    shape::BildEinstellung grob = genau();
    grob.mindestProzent = 2.5;  // 500 Punkte: das Loch (400) geht im Quadrat auf, der Kreis (rund 2830) bleibt
    fenster.setzeBildEinstellung(grob);
    REQUIRE(z.konturen.size() == 4);  // die gezeichnete und drei aus dem Bild
    CHECK(z.konturen[0].kennung == gezeichnet);
    CHECK(z.finde(alteKreis) == nullptr);  // neue Kennungen
    CHECK(fenster.ausBild(z.konturen[1].kennung));
    CHECK(z.herkunft.einstellungen.value(QStringLiteral("minSize")).toDouble() == doctest::Approx(2.5));
    // Undo bringt die alten zurueck — und sie gelten wieder als die aus dem Bild
    REQUIRE(fenster.rueckgaengig());
    CHECK(z.konturen.size() == 5);
    CHECK(z.finde(alteKreis) != nullptr);
    CHECK(fenster.ausBild(alteKreis));
    fenster.setzeBildEinstellung(genau());
    CHECK(z.konturen.size() == 5);
    CHECK(z.finde(gezeichnet) != nullptr);

    // --- Add in eine Zeichnung, die schon etwas traegt: eingepasst in den Ausschnitt
    FormFenster zweites;
    zweites.flaeche()->resize(400, 400);
    zweites.setzeLive(true);
    zweites.setzeBildEinstellung(genau());
    REQUIRE(zweites.fuegeKonturHinzu(dreieck) == 1);
    const QRectF ansicht = zweites.flaeche()->ansicht();
    REQUIRE(zweites.nimmBild(kreisUndQuadrat(), QStringLiteral("probe.png"), FormFenster::Einfuegen::Hinzufuegen) == 4);
    CHECK(zweites.zeichnung().herkunft.art == shape::HerkunftArt::Gezeichnet);  // Add: die Herkunft bleibt
    const QRectF grund = zweites.zeichnung().konturen[1].pfad.boundingRect();  // der Bildrand, eingepasst
    CHECK(grund.width() == doctest::Approx(ansicht.width() / 1.5));
    CHECK(grund.center().x() == doctest::Approx(ansicht.center().x()));

    // --- kein Bild
    CHECK(zweites.nimmBild(QImage(), QStringLiteral("leer"), FormFenster::Einfuegen::Ersetzen) == -1);
    CHECK(zweites.nimmBild(QStringLiteral("gibt/es/nicht.png"), FormFenster::Einfuegen::Ersetzen) == -1);

    // --- New shape: das Bild ist weg, die Regler grau
    fenster.beginneNeu();
    CHECK_FALSE(fenster.hatBild());
    CHECK_FALSE(ausBild->isEnabled());
    CHECK_FALSE(fenster.amRand(1));

    if (const QString ziel = qEnvironmentVariable("LUMIVIZ_SHAPE_BILD_FENSTER"); !ziel.isEmpty())
    {
        FormFenster gezeigt;
        gezeigt.setzeLive(true);
        gezeigt.setAttribute(Qt::WA_DontShowOnScreen);
        gezeigt.show();
        QCoreApplication::processEvents();
        gezeigt.setzeBildEinstellung(genau());
        REQUIRE(gezeigt.nimmBild(kreisUndQuadrat(), QStringLiteral("probe.png"), FormFenster::Einfuegen::Ersetzen) == 4);
        QCoreApplication::processEvents();
        CHECK(gezeigt.grab().save(ziel));
        if (const QString probe = qEnvironmentVariable("LUMIVIZ_SHAPE_BILD_PROBE"); !probe.isEmpty())
        {
            gezeigt.setzeBildEinstellung(shape::BildEinstellung{});
            CHECK(gezeigt.nimmBild(probe, FormFenster::Einfuegen::Ersetzen) > 0);
            QCoreApplication::processEvents();
            CHECK(gezeigt.grab().save(QString(ziel).replace(QStringLiteral(".png"), QStringLiteral("_probe.png"))));
        }
        gezeigt.hide();
    }
}

TEST_CASE("Shape Bild im Fenster: ohne Apply live — Vorschau in Grau, Take contours, danach aendern die Regler nur die Vorschau")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};

    FormFenster fenster;
    fenster.flaeche()->resize(400, 400);
    fenster.setzeLive(false);
    auto* nehmen = fenster.findChild<QPushButton*>(QStringLiteral("shapeTakeContours"));
    REQUIRE(nehmen != nullptr);
    CHECK_FALSE(fenster.findChild<QCheckBox*>(QStringLiteral("shapeApplyLive"))->isChecked());
    fenster.setzeBildEinstellung(genau());

    // --- nur vorgeschlagen: die Zeichnung bleibt leer
    REQUIRE(fenster.nimmBild(kreisUndQuadrat(), QStringLiteral("probe.png"), FormFenster::Einfuegen::Ersetzen) == 4);
    CHECK(fenster.zeichnung().konturen.empty());
    CHECK(fenster.vorschau().konturen.size() == 4);
    CHECK(nehmen->isEnabled());
    CHECK_FALSE(fenster.rueckgaengig());  // nichts geschehen, kein Schritt

    // --- ein Regler aendert nur die Vorschau
    shape::BildEinstellung grob = genau();
    grob.mindestProzent = 2.5;
    fenster.setzeBildEinstellung(grob);
    CHECK(fenster.vorschau().konturen.size() == 3);
    CHECK(fenster.zeichnung().konturen.empty());

    // --- Take contours: in die Zeichnung, ein Schritt fuer Undo
    REQUIRE(fenster.uebernimmVorschau(FormFenster::Einfuegen::Ersetzen) == 3);
    REQUIRE(fenster.zeichnung().konturen.size() == 3);
    CHECK(fenster.zeichnung().herkunft.art == shape::HerkunftArt::Bild);
    const int quadrat = fenster.zeichnung().konturen[1].kennung;
    // danach aendern die Regler nur die Vorschau
    fenster.setzeBildEinstellung(genau());
    CHECK(fenster.vorschau().konturen.size() == 4);
    CHECK(fenster.zeichnung().konturen.size() == 3);
    CHECK(fenster.zeichnung().finde(quadrat) != nullptr);

    // --- Apply live wieder an: die Vorschau verschwindet; der naechste Regler ersetzt die uebernommenen
    fenster.setzeLive(true);
    CHECK(fenster.vorschau().konturen.empty());
    fenster.setzeBildEinstellung(grob);
    CHECK(fenster.zeichnung().konturen.size() == 3);
    CHECK(fenster.zeichnung().finde(quadrat) == nullptr);
    CHECK(fenster.uebernimmVorschau(FormFenster::Einfuegen::Ersetzen) == 0);  // mit Apply live gibt es nichts zu uebernehmen
}
