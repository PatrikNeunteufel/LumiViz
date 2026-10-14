/**
 * @file test_ShapeKontur.cpp
 * @brief Klang aus einer Form, Stufe 19a: die Zeichnung aus Konturen, die Datei
 *        `.lvcontour`, wo sie liegt, und die Reihe (shape/Kontur.hpp;
 *        docs/audio/Shape_Konzept.md §5 und §7)
 *
 * Ohne Fenster und ohne Ton. Die eigenen Zeichnungen liegen in einem
 * Wegwerf-Ordner (`setzeEigenenKonturOrdnerFuerTests`). Nur der Fall zu den
 * mitgelieferten Formen braucht eine `QCoreApplication` — er liest
 * `asset/contours` aus dem Repo.
 */

#include <doctest.h>

#include "core/Datensicherung.hpp"
#include "shape/Kontur.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QColor>
#include <QImage>
#include <QJsonObject>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPolygonF>
#include <QStringList>
#include <QTemporaryDir>

#include <algorithm>
#include <cmath>
#include <complex>
#include <vector>

using namespace lumi::shape;

namespace
{

constexpr double kPi = 3.14159265358979323846;

/// Der Ordner der eigenen Zeichnungen fuer einen Fall; danach wieder der echte.
struct EigenerOrdner
{
    QTemporaryDir tmp;
    QString ordner;
    EigenerOrdner()
    {
        REQUIRE(tmp.isValid());
        ordner = QDir(tmp.path()).filePath(QStringLiteral("contours"));
        setzeEigenenKonturOrdnerFuerTests(ordner);
    }
    ~EigenerOrdner() { setzeEigenenKonturOrdnerFuerTests(QString()); }
};

QPainterPath quadrat(double links, double oben, double kante)
{
    QPainterPath pfad;
    pfad.moveTo(links, oben);
    pfad.lineTo(links + kante, oben);
    pfad.lineTo(links + kante, oben + kante);
    pfad.lineTo(links, oben + kante);
    pfad.closeSubpath();
    return pfad;
}

/// Ein Tropfen: eine Strecke, zwei Kurven.
QPainterPath tropfen()
{
    QPainterPath pfad;
    pfad.moveTo(10.5, 20.25);
    pfad.lineTo(60.0, 20.25);
    pfad.cubicTo(90.0, 20.25, 90.0, 80.125, 60.0, 80.125);
    pfad.cubicTo(30.0, 80.125, 0.1, 50.0, 10.5, 20.25);
    pfad.closeSubpath();
    return pfad;
}

/// Ein Kreis aus vielen Strecken, gegen den Uhrzeigersinn gesehen mit y nach oben.
QPainterPath kreisGegenUhr(double cx, double cy, double radius, int ecken)
{
    QPainterPath pfad;
    for (int i = 0; i < ecken; ++i)
    {
        const double winkel = 2.0 * kPi * i / ecken;
        const QPointF punkt(cx + radius * std::cos(winkel), cy - radius * std::sin(winkel));
        if (i == 0)
            pfad.moveTo(punkt);
        else
            pfad.lineTo(punkt);
    }
    pfad.closeSubpath();
    return pfad;
}

/// Die groesste Abweichung der Form mit `harmonische` Teiltoenen von ihren Punkten.
double abweichung(const Reihe& r, const std::vector<Punkt>& punkte, double harmonische)
{
    double groesste = 0.0;
    for (std::size_t n = 0; n < punkte.size(); ++n)
        groesste = std::max(groesste, std::abs(wert(r, static_cast<double>(n) / static_cast<double>(punkte.size()), harmonische) - punkte[n]));
    return groesste;
}

}  // namespace

TEST_CASE("Shape Kontur: der Pfad als Text — Strecken und Kurven kommen genau zurueck")
{
    const QPainterPath pfad = tropfen();
    const QString text = pfadAlsText(pfad);
    CHECK(text == QStringLiteral("M 10.5 20.25 L 60 20.25 C 90 20.25 90 80.125 60 80.125 C 30 80.125 0.1 50 10.5 20.25 Z"));
    QPainterPath gelesen;
    REQUIRE(pfadAusText(text, gelesen));
    CHECK(gelesen == pfad);
    CHECK(pfadAlsText(gelesen) == text);

    // ein Vieleck: die schliessende Strecke steht da, dann Z
    const QString eckig = pfadAlsText(quadrat(0.0, 0.0, 10.0));
    CHECK(eckig == QStringLiteral("M 0 0 L 10 0 L 10 10 L 0 10 L 0 0 Z"));
    REQUIRE(pfadAusText(eckig, gelesen));
    CHECK(gelesen == quadrat(0.0, 0.0, 10.0));

    // ein offener Zug bleibt beim Lesen des Texts offen (geschlossen wird er erst als Kontur)
    REQUIRE(pfadAusText(QStringLiteral("M 0 0 L 10 0 L 10 10"), gelesen));
    CHECK(pfadAlsText(gelesen) == QStringLiteral("M 0 0 L 10 0 L 10 10"));
    // Zeilenumbrueche und mehrere Luecken stoeren nicht
    REQUIRE(pfadAusText(QStringLiteral("  M 0 0\n L 10   0 \t L 10 10 Z "), gelesen));
    CHECK(gelesen.elementCount() == 4);

    // kein Pfad: `gelesen` bleibt, wie es war
    const QPainterPath vorher = gelesen;
    for (const char* kaputt : {"", "M 0", "L 1 1", "M 0 0 L 1", "M 0 0 X 1 1", "M 0 0 C 1 1 2 2 3", "M a b L 1 1", "M 0 0", "Z",
                               "M 0 0 L nan 1"})
    {
        CAPTURE(kaputt);
        CHECK_FALSE(pfadAusText(QString::fromLatin1(kaputt), gelesen));
        CHECK(gelesen == vorher);
    }
}

TEST_CASE("Shape Kontur: die Datei .lvcontour — hin und zurueck, feste Kennungen, Grenzen, eine Fassung von morgen")
{
    QTemporaryDir ordner;
    REQUIRE(ordner.isValid());

    // --- eine Zeichnung bauen: drei Konturen, eine davon nicht gewaehlt
    Zeichnung zeichnung;
    zeichnung.titel = QStringLiteral("Probe");
    CHECK(zeichnung.neueKontur(QPainterPath()) == 0);  // ohne Pfad keine Kontur
    CHECK(zeichnung.neueKontur(tropfen(), QStringLiteral("Drop")) == 1);
    CHECK(zeichnung.neueKontur(quadrat(100.0, 10.0, 40.0), QStringLiteral("Box"), QStringLiteral("#123456")) == 2);
    QPainterPath offen;
    offen.moveTo(200.0, 0.0);
    offen.lineTo(240.0, 0.0);
    offen.lineTo(220.0, 30.0);
    CHECK(zeichnung.neueKontur(offen) == 3);
    REQUIRE(zeichnung.konturen.size() == 3);
    CHECK(zeichnung.konturen[0].farbe == konturFarbe(1));
    CHECK(zeichnung.konturen[1].farbe == QStringLiteral("#123456"));
    CHECK(pfadAlsText(zeichnung.konturen[2].pfad).endsWith(QStringLiteral(" Z")));  // eine Kontur ist geschlossen
    zeichnung.konturen[2].gewaehlt = false;
    zeichnung.nutzung = Nutzung::EinPfad;
    zeichnung.spruenge = Spruenge::VonHand;
    zeichnung.bruecken.push_back(Bruecke{1, QPointF(60.0, 20.25), 2, QPointF(100.0, 10.0)});
    zeichnung.herkunft.art = HerkunftArt::Bild;
    zeichnung.herkunft.quelle = QStringLiteral("foto.png");
    zeichnung.herkunft.einstellungen = QJsonObject{{QStringLiteral("splitBy"), QStringLiteral("colour")}, {QStringLiteral("levels"), 6}};
    CHECK(zeichnung.naechsteKennung == 4);

    // --- als JSON und zurueck
    const QJsonObject json = alsJson(zeichnung);
    CHECK(json.value(QStringLiteral("format")).toString() == QStringLiteral("lumiviz-contour"));
    CHECK(json.value(QStringLiteral("version")).toInt() == kKonturFassung);
    CHECK(kKonturFassung == 1);
    CHECK(json.value(QStringLiteral("useAs")).toString() == QStringLiteral("onePath"));
    CHECK(json.value(QStringLiteral("jumps")).toString() == QStringLiteral("byHand"));
    CHECK(json.value(QStringLiteral("nextId")).toInt() == 4);
    REQUIRE(json.value(QStringLiteral("contours")).toArray().size() == 3);
    const QJsonObject erste = json.value(QStringLiteral("contours")).toArray().at(0).toObject();
    CHECK(erste.value(QStringLiteral("id")).toInt() == 1);
    CHECK(erste.value(QStringLiteral("name")).toString() == QStringLiteral("Drop"));
    CHECK(erste.value(QStringLiteral("path")).toString().startsWith(QStringLiteral("M 10.5 20.25 L 60 20.25 C ")));
    CHECK_FALSE(json.contains(QStringLiteral("coefficients")));  // die Reihe steht nicht in der Datei (D4 b)
    CHECK(json.value(QStringLiteral("source")).toObject().value(QStringLiteral("kind")).toString() == QStringLiteral("image"));
    Zeichnung gelesen;
    gelesen.name = QStringLiteral("bleibt");
    REQUIRE(ausJson(json, gelesen));
    CHECK(gelesen.name == QStringLiteral("bleibt"));
    gelesen.name = zeichnung.name;
    CHECK(gelesen == zeichnung);

    // --- als Datei und zurueck; ohne Titel gilt der Name der Datei
    const QString pfad = QDir(ordner.path()).filePath(QStringLiteral("probe.lvcontour"));
    REQUIRE(speichereZeichnung(zeichnung, pfad));
    Zeichnung ausDatei;
    REQUIRE(ladeZeichnung(pfad, ausDatei));
    CHECK(ausDatei.name == pfad);
    ausDatei.name = zeichnung.name;
    CHECK(ausDatei == zeichnung);
    Zeichnung ohneTitel = zeichnung;
    ohneTitel.titel.clear();
    const QString zweite = QDir(ordner.path()).filePath(QStringLiteral("ohne_titel.lvcontour"));
    REQUIRE(speichereZeichnung(ohneTitel, zweite));
    REQUIRE(ladeZeichnung(zweite, ausDatei));
    CHECK(ausDatei.titel == QStringLiteral("ohne_titel"));
    CHECK_FALSE(ladeZeichnung(QDir(ordner.path()).filePath(QStringLiteral("fehlt.lvcontour")), ausDatei));

    // --- die Kennung ist fest: eine Kontur geht, die anderen behalten ihre, und keine wird neu vergeben
    Zeichnung weniger = zeichnung;
    REQUIRE(weniger.entferne(2));
    CHECK_FALSE(weniger.entferne(2));
    CHECK(weniger.finde(2) == nullptr);
    REQUIRE(weniger.finde(3) != nullptr);
    CHECK(weniger.finde(3)->pfad == zeichnung.konturen[2].pfad);
    CHECK(weniger.bruecken.empty());  // die Bruecke hing an ihr
    CHECK(weniger.neueKontur(quadrat(0.0, 0.0, 5.0)) == 4);
    Zeichnung nachDatei;
    REQUIRE(ausJson(alsJson(weniger), nachDatei));
    CHECK(nachDatei.naechsteKennung == 5);
    CHECK(nachDatei.neueKontur(quadrat(0.0, 0.0, 5.0)) == 5);

    // --- Grenzen beim Lesen
    QJsonObject wild = json;
    QJsonArray konturen = json.value(QStringLiteral("contours")).toArray();
    QJsonObject doppelt = konturen.at(1).toObject();
    doppelt[QStringLiteral("id")] = 1;  // die Kennung gibt es schon
    konturen.append(doppelt);
    QJsonObject ohnePfad = konturen.at(1).toObject();
    ohnePfad[QStringLiteral("id")] = 9;
    ohnePfad[QStringLiteral("path")] = QStringLiteral("M 0 0");
    konturen.append(ohnePfad);
    QJsonObject ohneKennung = konturen.at(1).toObject();
    ohneKennung.remove(QStringLiteral("id"));
    konturen.append(ohneKennung);
    QJsonObject bunt = konturen.at(1).toObject();
    bunt[QStringLiteral("id")] = 12;
    bunt[QStringLiteral("color")] = QStringLiteral("rot");
    bunt[QStringLiteral("path")] = QStringLiteral("M 0 0 L 10 0 L 10 10");  // offen: wird geschlossen
    konturen.append(bunt);
    wild[QStringLiteral("contours")] = konturen;
    wild[QStringLiteral("nextId")] = 2;  // zu klein
    QJsonArray bruecken = json.value(QStringLiteral("bridges")).toArray();
    bruecken.append(QJsonObject{{QStringLiteral("from"), 1}, {QStringLiteral("fromPoint"), QJsonArray{0, 0}},
                                {QStringLiteral("to"), 77}, {QStringLiteral("toPoint"), QJsonArray{1, 1}}});  // 77 gibt es nicht
    bruecken.append(QJsonObject{{QStringLiteral("from"), 1}, {QStringLiteral("fromPoint"), QJsonArray{0, 0}},
                                {QStringLiteral("to"), 1}, {QStringLiteral("toPoint"), QJsonArray{1, 1}}});  // zu sich selbst
    bruecken.append(QJsonObject{{QStringLiteral("from"), 1}, {QStringLiteral("to"), 2}});  // ohne Punkte
    wild[QStringLiteral("bridges")] = bruecken;
    wild[QStringLiteral("useAs")] = QStringLiteral("etwas");
    Zeichnung bereinigt;
    REQUIRE(ausJson(wild, bereinigt));
    REQUIRE(bereinigt.konturen.size() == 4);  // die drei und die bunte
    CHECK(bereinigt.konturen[3].kennung == 12);
    CHECK(bereinigt.konturen[3].farbe == konturFarbe(12));
    CHECK(pfadAlsText(bereinigt.konturen[3].pfad) == QStringLiteral("M 0 0 L 10 0 L 10 10 L 0 0 Z"));
    CHECK(bereinigt.naechsteKennung == 13);
    CHECK(bereinigt.bruecken.size() == 1);
    CHECK(bereinigt.nutzung == Nutzung::Schichten);  // unbekannt: die Vorgabe

    // --- hoechstens 64 Konturen
    QJsonArray viele;
    for (int i = 1; i <= 80; ++i)
    {
        QJsonObject k = konturen.at(1).toObject();
        k[QStringLiteral("id")] = i;
        viele.append(k);
    }
    wild[QStringLiteral("contours")] = viele;
    REQUIRE(ausJson(wild, bereinigt));
    CHECK(bereinigt.konturen.size() == static_cast<std::size_t>(kKonturenMax));
    CHECK(kKonturenMax == 64);
    CHECK(bereinigt.neueKontur(quadrat(0.0, 0.0, 5.0)) == 0);  // voll

    // --- eine Fassung von morgen liest das Programm nicht — und sagt es; ein anderes Format auch nicht
    QJsonObject morgen = json;
    morgen[QStringLiteral("version")] = kKonturFassung + 1;
    bool zuNeu = false;
    Zeichnung bleibt = zeichnung;
    CHECK_FALSE(ausJson(morgen, bleibt, &zuNeu));
    CHECK(zuNeu);
    CHECK(bleibt == zeichnung);
    QJsonObject fremd = json;
    fremd[QStringLiteral("format")] = QStringLiteral("lumiviz-instrument");
    CHECK_FALSE(ausJson(fremd, bleibt, &zuNeu));
    CHECK_FALSE(zuNeu);
}

TEST_CASE("Shape Kontur: eigene Zeichnungen im Benutzerordner, eine Datei beim Pfad, der Bereich im Export")
{
    const EigenerOrdner eigen;
    CHECK(eigenerKonturOrdner() == eigen.ordner);
    CHECK(eigeneZeichnungen().empty());  // den Ordner gibt es noch nicht

    // --- der Name: klein, a bis z, Ziffern und _
    CHECK(eigenerKonturName(QStringLiteral(" Mein Herz 2! ")) == QStringLiteral("mein_herz_2"));
    CHECK(eigenerKonturName(QStringLiteral("***")).isEmpty());
    CHECK(eigeneKonturDatei(QStringLiteral("mein_herz")) == QDir(eigen.ordner).filePath(QStringLiteral("mein_herz.lvcontour")));
    CHECK(eigeneKonturDatei(QStringLiteral("Mein Herz")).isEmpty());
    CHECK(eigeneKonturDatei(QStringLiteral("../raus")).isEmpty());  // nichts ausserhalb des Ordners

    // --- speichern legt den Ordner an; die Nennung ist user:<name>
    Zeichnung herz;
    herz.titel = QStringLiteral("Mein Herz");
    REQUIRE(herz.neueKontur(tropfen(), QStringLiteral("Drop")) == 1);
    CHECK_FALSE(speichereEigeneZeichnung(herz, QStringLiteral("Mein Herz")));
    REQUIRE(speichereEigeneZeichnung(herz, QStringLiteral("mein_herz")));
    CHECK(herz.name == QStringLiteral("user:mein_herz"));
    CHECK(QFileInfo::exists(QDir(eigen.ordner).filePath(QStringLiteral("mein_herz.lvcontour"))));
    CHECK(istEigeneZeichnung(herz.name));
    CHECK_FALSE(istKonturDatei(herz.name));

    Zeichnung geholt;
    REQUIRE(holeZeichnung(QStringLiteral("user:mein_herz"), geholt));
    CHECK(geholt == herz);
    CHECK_FALSE(holeZeichnung(QStringLiteral("user:gibt_es_nicht"), geholt));
    CHECK_FALSE(holeZeichnung(QStringLiteral("user:../raus"), geholt));

    // --- die Liste: nach Titel; was keine Schicht nennen kann, fehlt
    Zeichnung apfel = herz;
    apfel.titel = QStringLiteral("Apfel");
    REQUIRE(speichereEigeneZeichnung(apfel, QStringLiteral("zz_apfel")));
    REQUIRE(speichereZeichnung(herz, QDir(eigen.ordner).filePath(QStringLiteral("Von Hand.lvcontour"))));
    const std::vector<Zeichnung> eigene = eigeneZeichnungen();
    REQUIRE(eigene.size() == 2);
    CHECK(eigene[0].titel == QStringLiteral("Apfel"));
    CHECK(eigene[0].name == QStringLiteral("user:zz_apfel"));
    CHECK(eigene[1].name == QStringLiteral("user:mein_herz"));

    // --- eine Datei an freiem Ort steht beim Pfad
    const QString frei = QDir(eigen.tmp.path()).filePath(QStringLiteral("woanders.lvcontour"));
    REQUIRE(speichereZeichnung(herz, frei));
    CHECK(istKonturDatei(frei));
    REQUIRE(holeZeichnung(frei, geholt));
    CHECK(geholt.name == frei);
    CHECK(geholt.konturen == herz.konturen);

    // --- loeschen
    REQUIRE(loescheEigeneZeichnung(QStringLiteral("mein_herz")));
    CHECK_FALSE(loescheEigeneZeichnung(QStringLiteral("mein_herz")));
    CHECK_FALSE(holeZeichnung(QStringLiteral("user:mein_herz"), geholt));
    CHECK(eigeneZeichnungen().size() == 1);

    // --- eigene Formen gehen in Export und Import der eigenen Daten (D2 a)
    const lumi::core::datensicherung::Bereich* bereich = lumi::core::datensicherung::findeBereich(QStringLiteral("shapes"));
    REQUIRE(bereich != nullptr);
    CHECK(bereich->titel == QStringLiteral("Shapes"));
    CHECK(bereich->ordner == QStringList{QStringLiteral("contours")});
    CHECK(QDir(eigenerKonturOrdner()).dirName() == QStringLiteral("contours"));
    const auto dateien = lumi::core::datensicherung::sammle(eigen.tmp.path(), *bereich);
    CHECK(dateien.size() == 2);  // zz_apfel und die von Hand hineingelegte
}

TEST_CASE("Shape Kontur: jede mitgelieferte Form laedt, ist geschlossen und ergibt eine Reihe")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    const QCoreApplication app{argc, argv};

    REQUIRE_FALSE(konturOrdner().isEmpty());
    CHECK(QDir(konturOrdner()).dirName() == QStringLiteral("contours"));
    const std::vector<Zeichnung>& alle = zeichnungen();
    // wer eine Form dazulegt, traegt sie in asset/contours/make_contours.py ein und stellt die Zahl nach
    REQUIRE(alle.size() == 20);
    QStringList namen;
    for (const Zeichnung& zeichnung : alle) namen << zeichnung.name;
    namen.sort();
    CHECK(namen == QStringList{QStringLiteral("atom"),     QStringLiteral("bolt"),           QStringLiteral("cat"),
                               QStringLiteral("circle"),   QStringLiteral("cross"),          QStringLiteral("drop"),
                               QStringLiteral("flower"),   QStringLiteral("gear"),           QStringLiteral("heart"),
                               QStringLiteral("heart_and_star"), QStringLiteral("house"),    QStringLiteral("infinity"),
                               QStringLiteral("moon"),     QStringLiteral("note"),           QStringLiteral("rings"),
                               QStringLiteral("smiley"),   QStringLiteral("square"),         QStringLiteral("star"),
                               QStringLiteral("triangle"), QStringLiteral("yin_yang")});
    for (const Zeichnung& zeichnung : alle)
    {
        CAPTURE(zeichnung.name.toStdString());
        CHECK_FALSE(zeichnung.titel.isEmpty());
        REQUIRE_FALSE(zeichnung.konturen.empty());
        CHECK(zeichnung.naechsteKennung == static_cast<int>(zeichnung.konturen.size()) + 1);
        for (const Kontur& kontur : zeichnung.konturen)
        {
            CHECK(kontur.kennung > 0);
            CHECK(kontur.gewaehlt);
            CHECK(pfadAlsText(kontur.pfad).endsWith(QStringLiteral(" Z")));
            const std::vector<Punkt> punkte = abtasten(kontur.pfad);
            REQUIRE(punkte.size() == static_cast<std::size_t>(kAbtastPunkte));
            const std::vector<Punkt> mittig = normiert(punkte);
            const Reihe r = reihe(mittig, 64);
            REQUIRE(r.harmonische() == 64);
            CHECK(std::abs(r.mitte) < 1.0e-9);  // kein Gleichanteil
            CHECK(std::max(std::abs(r.plus[0]), std::abs(r.minus[0])) > 0.3);  // der Grundton traegt die Form
            CHECK(abweichung(r, mittig, 64.0) < 0.03);  // mit 64 Teiltoenen ist die Form da
        }
        // schreiben und lesen aendert nichts
        Zeichnung nochmal;
        nochmal.name = zeichnung.name;
        REQUIRE(ausJson(alsJson(zeichnung), nochmal));
        CHECK(nochmal == zeichnung);
    }

    // `LUMIVIZ_SHAPE_BILD=<datei.png>` legt ein Bild ab: je Kontur eine Zeile, die Form mit
    // 1, 4, 16 und 64 Teiltoenen in ihrer Farbe ueber der Zeichnung in Grau
    if (const QString bild = qEnvironmentVariable("LUMIVIZ_SHAPE_BILD"); !bild.isEmpty())
    {
        const int feld = 180;
        const double stufen[4] = {1.0, 4.0, 16.0, 64.0};
        int zeilen = 0;
        for (const Zeichnung& zeichnung : alle) zeilen += static_cast<int>(zeichnung.konturen.size());
        QImage blatt(4 * feld, zeilen * feld, QImage::Format_ARGB32);
        blatt.fill(QColor(24, 27, 33));
        QPainter maler(&blatt);
        maler.setRenderHint(QPainter::Antialiasing);
        int zeile = 0;
        for (const Zeichnung& zeichnung : alle)
        {
            for (const Kontur& kontur : zeichnung.konturen)
            {
                const std::vector<Punkt> mittig = normiert(abtasten(kontur.pfad, 1024));
                const Reihe r = reihe(mittig, 64);
                // y zeigt in der Rechnung nach oben, im Bild nach unten
                const auto zug = [&](const std::vector<Punkt>& punkte, int spalte) {
                    QPolygonF vieleck;
                    for (const Punkt& p : punkte)
                        vieleck << QPointF(spalte * feld + feld / 2.0 + p.real() * feld * 0.42,
                                           zeile * feld + feld / 2.0 - p.imag() * feld * 0.42);
                    return vieleck;
                };
                for (int spalte = 0; spalte < 4; ++spalte)
                {
                    maler.setPen(QPen(QColor(110, 114, 122), 1.0, Qt::DashLine));
                    maler.drawPolygon(zug(mittig, spalte));
                    maler.setPen(QPen(QColor(kontur.farbe), 2.0));
                    maler.drawPolygon(zug(umriss(r, stufen[spalte], 720), spalte));
                }
                ++zeile;
            }
        }
        maler.end();
        CHECK(blatt.save(bild));
    }

    // eine Schicht nennt sie beim Namen
    Zeichnung geholt;
    REQUIRE(holeZeichnung(QStringLiteral("heart_and_star"), geholt));
    CHECK(geholt.titel == QStringLiteral("Heart and star"));
    REQUIRE(geholt.konturen.size() == 2);
    CHECK(geholt.nutzung == Nutzung::EinPfad);
    CHECK(geholt.finde(2)->name == QStringLiteral("Star"));
    CHECK_FALSE(holeZeichnung(QStringLiteral("gibt_es_nicht"), geholt));

    // der mitgelieferte Kreis (vier Kurven) ist ein reiner Sinus: ein Koeffizient traegt alles
    REQUIRE(holeZeichnung(QStringLiteral("circle"), geholt));
    const Reihe kreis = reihe(normiert(abtasten(geholt.konturen[0].pfad)), 32);
    CHECK(std::max(std::abs(kreis.plus[0]), std::abs(kreis.minus[0])) == doctest::Approx(1.0).epsilon(0.002));
    CHECK(std::min(std::abs(kreis.plus[0]), std::abs(kreis.minus[0])) < 0.002);
    for (int k = 2; k <= 32; ++k)
    {
        CAPTURE(k);
        CHECK(std::abs(kreis.plus[static_cast<std::size_t>(k - 1)]) < 0.002);
        CHECK(std::abs(kreis.minus[static_cast<std::size_t>(k - 1)]) < 0.002);
    }
}

TEST_CASE("Shape Reihe: abtasten in gleichen Abstaenden, y nach oben — und ein Kreis ist ein reiner Sinus")
{
    // --- ein Kreis um (100, 100) mit Radius 50, gegen den Uhrzeigersinn
    const std::vector<Punkt> kreis = abtasten(kreisGegenUhr(100.0, 100.0, 50.0, 720), 1024);
    REQUIRE(kreis.size() == 1024);
    // der Stift faehrt gleich schnell: jeder Schritt ist gleich lang
    double kleinster = 1.0e30;
    double groesster = 0.0;
    for (std::size_t n = 0; n < kreis.size(); ++n)
    {
        const double schritt = std::abs(kreis[(n + 1) % kreis.size()] - kreis[n]);
        kleinster = std::min(kleinster, schritt);
        groesster = std::max(groesster, schritt);
    }
    CHECK(groesster / kleinster < 1.001);
    CHECK(groesster == doctest::Approx(2.0 * kPi * 50.0 / 1024.0).epsilon(0.001));
    // der erste Punkt ist der Anfang des Pfads; y ist umgedreht
    CHECK(kreis[0].real() == doctest::Approx(150.0));
    CHECK(kreis[0].imag() == doctest::Approx(-100.0));
    CHECK(kreis[256].real() == doctest::Approx(100.0).epsilon(0.001));
    CHECK(kreis[256].imag() == doctest::Approx(-50.0).epsilon(0.001));  // ein Viertel weiter: oben

    // --- die Reihe: die Mitte ist der Schwerpunkt, der erste Koeffizient der Radius, sonst nichts
    const Reihe roh = reihe(kreis, 16);
    REQUIRE(roh.harmonische() == 16);
    CHECK(roh.mitte.real() == doctest::Approx(100.0).epsilon(0.0001));
    CHECK(roh.mitte.imag() == doctest::Approx(-100.0).epsilon(0.0001));
    CHECK(std::abs(roh.plus[0]) == doctest::Approx(50.0).epsilon(0.0001));  // gegen den Uhrzeigersinn: +1
    CHECK(std::abs(roh.minus[0]) < 0.01);
    for (int k = 2; k <= 16; ++k)
    {
        CAPTURE(k);
        CHECK(std::abs(roh.plus[static_cast<std::size_t>(k - 1)]) < 0.01);
        CHECK(std::abs(roh.minus[static_cast<std::size_t>(k - 1)]) < 0.01);
    }
    // an ihrer Stelle in der Zeichnung — oder um den Nullpunkt
    CHECK(std::abs(wert(roh, 0.0, 16.0, true) - Punkt(150.0, -100.0)) < 0.02);
    CHECK(std::abs(wert(roh, 0.0, 16.0) - Punkt(50.0, 0.0)) < 0.02);
    CHECK(std::abs(wert(roh, 0.25, 16.0) - Punkt(0.0, 50.0)) < 0.02);

    // --- normiert: um den Nullpunkt, der groesste Ausschlag ist 1
    const std::vector<Punkt> mittig = normiert(kreis);
    double ausschlag = 0.0;
    Punkt summe{0.0, 0.0};
    for (const Punkt& p : mittig)
    {
        ausschlag = std::max({ausschlag, std::abs(p.real()), std::abs(p.imag())});
        summe += p;
    }
    CHECK(ausschlag == doctest::Approx(1.0));
    CHECK(std::abs(summe) < 1.0e-9);

    // --- nicht mehr Harmonische, als die Punkte hergeben; zu wenig Punkte: nichts
    CHECK(reihe(abtasten(quadrat(0.0, 0.0, 10.0), 9), 512).harmonische() == 4);
    CHECK(reihe(kreis, 100000).harmonische() == 511);
    CHECK(kHarmonischeMax == 512);
    CHECK(abtasten(quadrat(0.0, 0.0, 10.0), 2).empty());
    CHECK(abtasten(QPainterPath(), 64).empty());
    CHECK(reihe({}, 8).harmonische() == 0);

    // --- eine Kurve wird fein zerlegt, auch wenn die Zeichnung klein ist: ein Kreis mit Radius 0,5
    QPainterPath klein;
    klein.addEllipse(QPointF(0.0, 0.0), 0.5, 0.5);
    const std::vector<Punkt> winzig = abtasten(klein, 512);
    REQUIRE(winzig.size() == 512);
    for (const Punkt& p : winzig) CHECK(std::abs(p) == doctest::Approx(0.5).epsilon(0.001));
}

TEST_CASE("Shape Reihe: Harmonics — mehr Teiltoene, genauere Form; dazwischen weich")
{
    // ein Quadrat: Ecken brauchen viele Teiltoene
    const std::vector<Punkt> punkte = normiert(abtasten(quadrat(30.0, 30.0, 140.0), 2048));
    REQUIRE(punkte.size() == 2048);
    const Reihe r = reihe(punkte, 256);
    REQUIRE(r.harmonische() == 256);
    CHECK(std::abs(r.mitte) < 1.0e-9);

    const double mitEinem = abweichung(r, punkte, 1.0);
    const double mitAcht = abweichung(r, punkte, 8.0);
    const double mitVielen = abweichung(r, punkte, 64.0);
    const double mitAllen = abweichung(r, punkte, 256.0);
    MESSAGE("Quadrat, groesste Abweichung: 1 Teilton " << mitEinem << ", 8 " << mitAcht << ", 64 " << mitVielen << ", 256 " << mitAllen);
    CHECK(mitEinem > mitAcht);
    CHECK(mitAcht > mitVielen);
    CHECK(mitVielen > mitAllen);
    CHECK(mitEinem > 0.2);    // ein Teilton: ein Kreis, weit weg von den Ecken
    CHECK(mitVielen < 0.03);
    CHECK(mitAllen < 0.01);

    // das Gewicht: bis zur Stellung voll, die naechste anteilig, darueber nicht
    CHECK(gewicht(3.4, 1) == 1.0);
    CHECK(gewicht(3.4, 3) == 1.0);
    CHECK(gewicht(3.4, 4) == doctest::Approx(0.4));
    CHECK(gewicht(3.4, 5) == 0.0);
    CHECK(gewicht(0.0, 1) == 0.0);
    CHECK(gewicht(1.0, 1) == 1.0);

    // zwischen zwei ganzen Stellungen liegt der Punkt genau dazwischen: keine Stufe
    for (const double umlauf : {0.0, 0.13, 0.5, 0.77})
    {
        CAPTURE(umlauf);
        const Punkt drei = wert(r, umlauf, 3.0);
        const Punkt vier = wert(r, umlauf, 4.0);
        CHECK(std::abs(wert(r, umlauf, 3.4) - (0.6 * drei + 0.4 * vier)) < 1.0e-12);
        CHECK(std::abs(wert(r, umlauf, 3.0001) - drei) < 1.0e-3);
    }
    CHECK(wert(r, 0.3, 0.0) == Punkt(0.0, 0.0));  // ohne Teilton: Stille
    // mehr verlangt, als die Reihe traegt: sie gibt, was sie hat
    CHECK(std::abs(wert(r, 0.3, 5000.0) - wert(r, 0.3, 256.0)) < 1.0e-12);

    // der Umriss ist ein geschlossener Zug: nach einem Umlauf wieder am Anfang
    const std::vector<Punkt> zug = umriss(r, 32.0, 200);
    REQUIRE(zug.size() == 200);
    CHECK(std::abs(wert(r, 1.0, 32.0) - zug[0]) < 1.0e-9);
    CHECK(umriss(r, 32.0, 0).empty());
}
