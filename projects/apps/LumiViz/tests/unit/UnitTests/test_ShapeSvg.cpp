/**
 * @file test_ShapeSvg.cpp
 * @brief Klang aus einer Form, Stufe 20b: Konturen aus einem SVG
 *        (shape/SvgKonturen.hpp) und Open SVG... im Fenster Edit shape
 *        (docs/audio/Shape_Konzept.md §8, §8a; Entscheide Patrik S1 a, S120,
 *        O1 a und O2 a, S122)
 *
 * Ohne Ton und ohne gezeigtes Fenster. Die kleinen SVG stehen im Test; der
 * letzte Fall liest Patriks Proben unter `asset/shapes` (vom Arbeitsordner
 * aus — die Test-Exe aus dem Projekt-Root starten) und meldet, was ankommt.
 */

#include <doctest.h>

#include "loop/FormFenster.hpp"
#include "shape/Kontur.hpp"
#include "shape/SvgKonturen.hpp"

#include <QApplication>
#include <QDir>
#include <QDirIterator>
#include <QElapsedTimer>
#include <QFile>
#include <QLabel>
#include <QPushButton>
#include <QTemporaryDir>

#include <cmath>
#include <string>
#include <vector>

using namespace lumi::loop;
namespace shape = lumi::shape;

namespace
{

QByteArray svg(const char* rumpf, const char* viewBox = "0 0 100 100")
{
    return QByteArray("<svg xmlns='http://www.w3.org/2000/svg' viewBox='") + viewBox + "'>" + rumpf + "</svg>";
}

/// Wie potrace schreibt: eine Gruppe mit y nach oben, ein Pfad mit Umriss und Loch.
const char* const kPotrace =
    "<g transform='translate(0,100) scale(0.1,-0.1)' fill='#000000' stroke='none'>"
    "<path d='M100 100 L900 100 L900 900 L100 900 Z M300 300 L700 300 L700 700 L300 700 Z'/></g>";

/// `zahl` Quadrate, jedes groesser als das vorige.
QByteArray quadrate(int zahl)
{
    std::string rumpf;
    for (int i = 0; i < zahl; ++i)
        rumpf += "<rect x='" + std::to_string(i * 14) + "' y='0' width='" + std::to_string(i + 1) + "' height='" +
                 std::to_string(i + 1) + "' fill='#ff0000'/>";
    return svg(rumpf.c_str(), "0 0 1000 100");
}

QString schreibe(const QTemporaryDir& ordner, const char* name, const QByteArray& inhalt)
{
    const QString pfad = QDir(ordner.path()).filePath(QLatin1String(name));
    QFile datei(pfad);
    REQUIRE(datei.open(QIODevice::WriteOnly));
    datei.write(inhalt);
    return pfad;
}

}  // namespace

TEST_CASE("Shape SVG: Teilpfade werden Konturen — Lage, Loecher, offene Linien, Farbe, unsichtbares faellt weg")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};

    // --- ein Pfad mit Kurven (wie shape-heart.svg in asset/shapes): eine Kontur, Kurven bleiben Kurven
    const shape::SvgKonturen herz = shape::svgKonturen(svg(
        "<path d='M438.82 41.18c-54.9-54.9-143.92-54.9-198.82 0-54.9-54.9-143.92-54.9-198.82 0-54.9 54.9-54.9 143.92 0 198.82L240 "
        "438.82 438.82 240c54.9-54.9 54.9-143.92 0-198.82Z' fill='#808'></path>",
        "0 0 480 438.82"));
    REQUIRE(herz.gelesen);
    REQUIRE(herz.teile.size() == 1);
    CHECK(herz.rahmen == QRectF(0.0, 0.0, 480.0, 438.82));
    CHECK(herz.teile[0].farbe.isEmpty());  // eine Farbe fuer alle: die der Reihe (O3 a)
    CHECK_FALSE(herz.teile[0].offen);
    CHECK(herz.teile[0].gewaehlt);
    const QRectF lage = herz.teile[0].pfad.boundingRect();
    CHECK(lage.left() == doctest::Approx(0.0).epsilon(0.01));
    CHECK(lage.right() == doctest::Approx(480.0).epsilon(0.01));
    CHECK(lage.bottom() == doctest::Approx(438.82).epsilon(0.01));
    bool kurve = false;
    for (int i = 0; i < herz.teile[0].pfad.elementCount(); ++i)
        kurve = kurve || herz.teile[0].pfad.elementAt(i).type == QPainterPath::CurveToElement;
    CHECK(kurve);

    // --- wie potrace: y nach oben in einer Gruppe, Umriss und Loch in einem Pfad — zwei Konturen, an ihrer Stelle
    const shape::SvgKonturen potrace = shape::svgKonturen(svg(kPotrace));
    REQUIRE(potrace.teile.size() == 2);
    CHECK(potrace.teile[0].pfad.boundingRect() == QRectF(10.0, 10.0, 80.0, 80.0));
    CHECK(potrace.teile[1].pfad.boundingRect() == QRectF(30.0, 30.0, 40.0, 40.0));
    CHECK(potrace.teile[0].laenge == doctest::Approx(320.0));
    CHECK(potrace.teile[1].laenge == doctest::Approx(160.0));
    CHECK(potrace.teile[0].gewaehlt);
    CHECK(potrace.teile[1].gewaehlt);  // hoechstens acht: alle mit Haken (O2 a)
    CHECK(potrace.teile[0].farbe.isEmpty());  // beide schwarz: die Farben der Reihe (O3 a)
    CHECK(potrace.teile[1].farbe.isEmpty());

    // --- eine offene Linie (nur Strich) wird mit einer Strecke geschlossen; Rechteck und Kreis kommen an
    const shape::SvgKonturen gemischt = shape::svgKonturen(svg(
        "<polyline points='10,10 50,10 50,50' fill='none' stroke='#ffff00'/>"
        "<rect x='10' y='60' width='30' height='10' fill='#00ff00'/>"
        "<circle cx='70' cy='70' r='10' fill='#0000ff'/>"
        "<path d='M0 0 L10 0 L10 10 Z' fill='none'/>"));  // unsichtbar: faellt weg
    REQUIRE(gemischt.teile.size() == 3);
    CHECK(gemischt.teile[0].offen);
    CHECK(gemischt.teile[0].farbe == QStringLiteral("#ffff00"));
    CHECK(gemischt.teile[0].laenge == doctest::Approx(80.0 + std::sqrt(3200.0)));
    CHECK(shape::pfadAlsText(gemischt.teile[0].pfad).endsWith(QStringLiteral("Z")));
    CHECK(gemischt.teile[1].pfad.boundingRect() == QRectF(10.0, 60.0, 30.0, 10.0));
    CHECK(gemischt.teile[2].pfad.boundingRect().center().x() == doctest::Approx(70.0));
    CHECK(gemischt.teile[2].farbe == QStringLiteral("#0000ff"));

    // --- kein SVG, oder nichts zu sehen
    CHECK_FALSE(shape::svgKonturen(QByteArray("kein svg")).gelesen);
    CHECK_FALSE(shape::svgKonturen(QByteArray()).gelesen);
    const shape::SvgKonturen leer = shape::svgKonturen(svg("<g></g>"));
    CHECK(leer.gelesen);
    CHECK(leer.teile.empty());
    // eine Farbe ueber eine CSS-Variable (wie die Dateien `-raw` unter asset/shapes/2) kennt Qt Svg nicht:
    // der Pfad ist unsichtbar und kommt nicht an
    const shape::SvgKonturen variable = shape::svgKonturen(svg("<path d='M0 0 L10 0 L10 10' fill='none' stroke='var(--x, currentColor)'/>"));
    CHECK(variable.gelesen);
    CHECK(variable.teile.empty());
    CHECK_FALSE(shape::liesSvgKonturen(QStringLiteral("gibt/es/nicht.svg")).gelesen);
}

TEST_CASE("Shape SVG: mehr als 64 — die laengsten bleiben in ihrer Reihenfolge; der Haken bis acht fuer alle, sonst die laengste")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};

    const shape::SvgKonturen siebzig = shape::svgKonturen(quadrate(70));
    REQUIRE(siebzig.teile.size() == static_cast<std::size_t>(shape::kKonturenMax));
    CHECK(siebzig.weggefallen == 6);
    // die sechs kleinsten (die ersten sechs der Datei) sind weg, die Reihenfolge bleibt
    CHECK(siebzig.teile.front().laenge == doctest::Approx(4.0 * 7.0));
    CHECK(siebzig.teile.back().laenge == doctest::Approx(4.0 * 70.0));
    int mitHaken = 0;
    for (const shape::SvgTeil& t : siebzig.teile) mitHaken += t.gewaehlt ? 1 : 0;
    CHECK(mitHaken == 1);
    CHECK(siebzig.teile.back().gewaehlt);

    const shape::SvgKonturen acht = shape::svgKonturen(quadrate(8));
    REQUIRE(acht.teile.size() == 8);
    CHECK(acht.weggefallen == 0);
    for (const shape::SvgTeil& t : acht.teile) CHECK(t.gewaehlt);

    const shape::SvgKonturen neun = shape::svgKonturen(quadrate(9));
    REQUIRE(neun.teile.size() == 9);
    mitHaken = 0;
    for (const shape::SvgTeil& t : neun.teile) mitHaken += t.gewaehlt ? 1 : 0;
    CHECK(mitHaken == 1);
    CHECK(neun.teile.back().gewaehlt);
}

TEST_CASE("Shape SVG im Fenster: Open SVG... ersetzt oder fuegt hinzu — eingepasst, ein Schritt fuer Undo, die Zeile zu den weggefallenen")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
    QTemporaryDir ordner;
    REQUIRE(ordner.isValid());
    const QString potrace = schreibe(ordner, "potrace.svg", svg(kPotrace));
    const QString zwei = schreibe(ordner, "zwei.svg",
                                  svg("<rect x='0' y='0' width='1000' height='500' fill='#00ff00'/>"
                                      "<circle cx='500' cy='250' r='100' fill='#0000ff'/>",
                                      "0 0 1000 500"));
    const QString siebzig = schreibe(ordner, "siebzig.svg", quadrate(70));
    const QString leer = schreibe(ordner, "leer.svg", svg("<g></g>"));
    const QString kaputt = schreibe(ordner, "kaputt.svg", QByteArray("kein svg"));

    FormFenster fenster;
    fenster.flaeche()->resize(400, 400);
    CHECK(fenster.findChild<QPushButton*>(QStringLiteral("shapeOpenSvg"))->isEnabled());
    CHECK(fenster.findChild<QPushButton*>(QStringLiteral("shapeOpenImage"))->isEnabled() == false);

    // --- in eine leere Zeichnung: in den eigenen Massen, die Herkunft wird SVG — auch wenn Add verlangt ist
    REQUIRE(fenster.nimmSvg(potrace, FormFenster::Einfuegen::Hinzufuegen) == 2);
    const shape::Zeichnung& z = fenster.zeichnung();
    CHECK(z.herkunft.art == shape::HerkunftArt::Svg);
    CHECK(z.herkunft.quelle == QStringLiteral("potrace.svg"));
    CHECK(z.konturen[0].pfad.boundingRect() == QRectF(10.0, 10.0, 80.0, 80.0));
    CHECK(z.konturen[0].farbe == shape::konturFarbe(z.konturen[0].kennung));  // schwarz in der Datei: die Reihe (O3 a)
    CHECK(z.konturen[1].farbe == shape::konturFarbe(z.konturen[1].kennung));
    CHECK(z.konturen[0].farbe != z.konturen[1].farbe);
    CHECK(fenster.flaeche()->ansicht().contains(QRectF(10.0, 10.0, 80.0, 80.0)));
    CHECK(fenster.aktuelle() == 1);
    CHECK(fenster.form() != nullptr);

    // --- Add: ans Ende, eingepasst in den Ausschnitt ohne seinen Rand; die Herkunft bleibt
    const QRectF ansicht = fenster.flaeche()->ansicht();
    REQUIRE(fenster.nimmSvg(zwei, FormFenster::Einfuegen::Hinzufuegen) == 2);
    REQUIRE(z.konturen.size() == 4);
    CHECK(z.herkunft.quelle == QStringLiteral("potrace.svg"));
    const QRectF rechteck = z.konturen[2].pfad.boundingRect();
    CHECK(rechteck.width() == doctest::Approx(ansicht.width() / 1.5));
    CHECK(rechteck.height() == doctest::Approx(ansicht.width() / 1.5 / 2.0));
    CHECK(rechteck.center().x() == doctest::Approx(ansicht.center().x()));
    CHECK(rechteck.center().y() == doctest::Approx(ansicht.center().y()));
    CHECK(z.konturen[3].pfad.boundingRect().center().x() == doctest::Approx(ansicht.center().x()));
    CHECK(fenster.aktuelle() == z.konturen[2].kennung);

    // --- ein Schritt fuer Undo
    REQUIRE(fenster.rueckgaengig());
    CHECK(fenster.zeichnung().konturen.size() == 2);

    // --- Replace: die bisherigen fallen weg, die Kennungen laufen weiter
    REQUIRE(fenster.nimmSvg(zwei, FormFenster::Einfuegen::Ersetzen) == 2);
    REQUIRE(z.konturen.size() == 2);
    CHECK(z.konturen[0].kennung > 2);
    CHECK(z.herkunft.quelle == QStringLiteral("zwei.svg"));
    CHECK(z.konturen[0].pfad.boundingRect() == QRectF(0.0, 0.0, 1000.0, 500.0));  // in den eigenen Massen

    // --- mehr als 64: die Zeile unter der Liste
    auto* zeile = fenster.findChild<QLabel*>(QStringLiteral("shapeLeftOut"));
    REQUIRE(zeile != nullptr);
    CHECK(zeile->text().isEmpty());
    REQUIRE(fenster.nimmSvg(siebzig, FormFenster::Einfuegen::Ersetzen) == 64);
    CHECK(fenster.weggefallen() == 6);
    CHECK(zeile->text() == QStringLiteral("6 smaller ones left out"));
    int mitHaken = 0;
    for (const shape::Kontur& k : z.konturen) mitHaken += k.gewaehlt ? 1 : 0;
    CHECK(mitHaken == 1);
    // eine volle Zeichnung nimmt nichts mehr — die neuen zaehlen zu den weggefallenen
    CHECK(fenster.nimmSvg(zwei, FormFenster::Einfuegen::Hinzufuegen) == 0);
    CHECK(z.konturen.size() == 64);
    CHECK(fenster.weggefallen() == 8);

    // --- nichts zu sehen, oder kein SVG: nichts geschieht, auch kein Schritt fuer Undo
    REQUIRE(fenster.rueckgaengig());  // zurueck vor das volle Hinzufuegen
    const std::size_t vorher = fenster.zeichnung().konturen.size();
    CHECK(fenster.nimmSvg(leer, FormFenster::Einfuegen::Hinzufuegen) == 0);
    CHECK(fenster.nimmSvg(kaputt, FormFenster::Einfuegen::Hinzufuegen) == -1);
    CHECK(fenster.nimmSvg(QDir(ordner.path()).filePath(QStringLiteral("fehlt.svg")), FormFenster::Einfuegen::Ersetzen) == -1);
    CHECK(fenster.zeichnung().konturen.size() == vorher);
    REQUIRE(fenster.rueckgaengig());
    CHECK(fenster.zeichnung().herkunft.quelle == QStringLiteral("zwei.svg"));

    // --- eine neue Zeichnung beginnt ohne weggefallene
    fenster.beginneNeu();
    CHECK(fenster.weggefallen() == 0);
    CHECK(zeile->text().isEmpty());
}

TEST_CASE("Shape SVG: Patriks Proben unter asset/shapes — jede gelesen, hoechstens 64, die Messung je Datei")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
    const QString wurzel = QDir::current().filePath(QStringLiteral("asset/shapes"));
    if (!QFileInfo(wurzel).isDir())
    {
        MESSAGE("asset/shapes nicht gefunden (die Test-Exe aus dem Projekt-Root starten) — nichts geprueft");
        return;
    }
    int dateien = 0;
    int bisAcht = 0;
    int mitWeggefallenen = 0;
    int offene = 0;
    int ohne = 0;  ///< Dateien ohne Kontur (Farben ueber CSS-Variablen)
    qint64 langsamsteMs = 0;
    QString langsamste;
    QDirIterator suche(wurzel, {QStringLiteral("*.svg")}, QDir::Files, QDirIterator::Subdirectories);
    while (suche.hasNext())
    {
        const QString datei = suche.next();
        QElapsedTimer uhr;
        uhr.start();
        const shape::SvgKonturen svg = shape::liesSvgKonturen(datei);
        const qint64 ms = uhr.elapsed();
        if (ms > langsamsteMs)
        {
            langsamsteMs = ms;
            langsamste = QFileInfo(datei).fileName();
        }
        INFO(datei.toStdString());
        CHECK(svg.gelesen);
        CHECK(svg.teile.size() <= static_cast<std::size_t>(shape::kKonturenMax));
        ++dateien;
        // leer darf nur sein, was seine Farben ueber CSS-Variablen nennt (Qt Svg kennt sie nicht)
        QFile roh(datei);
        REQUIRE(roh.open(QIODevice::ReadOnly));
        const bool cssVariablen = roh.readAll().contains("var(--");
        CHECK((!svg.teile.empty() || cssVariablen));
        if (svg.teile.empty()) ++ohne;
        if (svg.teile.size() <= static_cast<std::size_t>(shape::kSvgAlleGewaehlt) && svg.weggefallen == 0) ++bisAcht;
        if (svg.weggefallen > 0) ++mitWeggefallenen;
        for (const shape::SvgTeil& t : svg.teile) offene += t.offen ? 1 : 0;
    }
    MESSAGE(dateien << " Dateien: " << bisAcht << " mit hoechstens acht Konturen, " << mitWeggefallenen
                    << " mit mehr als 64, " << ohne << " ohne Kontur (CSS-Variablen); " << offene << " offene Konturen geschlossen; am laengsten " << langsamsteMs
                    << " ms (" << langsamste.toStdString() << ")");
    CHECK(dateien > 0);

    // `LUMIVIZ_SHAPE_SVG_BILD=<datei.png>`: drei Proben im Fenster, je ein Bild (`_1` bis `_3`)
    if (const QString ziel = qEnvironmentVariable("LUMIVIZ_SHAPE_SVG_BILD"); !ziel.isEmpty())
    {
        const QStringList proben = {QStringLiteral("1/1299642.svg"), QStringLiteral("1/rocketship8.svg"),
                                    QStringLiteral("2/flow-lines-2026-10-13.svg")};
        for (int i = 0; i < proben.size(); ++i)
        {
            FormFenster fenster;
            fenster.setAttribute(Qt::WA_DontShowOnScreen);
            fenster.show();  // das Layout steht erst nach dem Zeigen; auf den Bildschirm kommt nichts
            QCoreApplication::processEvents();
            CHECK(fenster.nimmSvg(QDir(wurzel).filePath(proben[i]), FormFenster::Einfuegen::Ersetzen) > 0);
            QCoreApplication::processEvents();
            const QString datei = QString(ziel).replace(QStringLiteral(".png"), QStringLiteral("_%1.png").arg(i + 1));
            CHECK(fenster.grab().save(datei));
            fenster.hide();
        }
    }
}
