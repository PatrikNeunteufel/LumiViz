/**
 ****************************************************************************************
 * @file   test_LoopGriffbild.cpp
 * @brief  Tabs und Akkorde, Stufe 26e-1: der Name aus den Toenen (S-T11 a L), das
 *         Verschieben auf dem Griffbrett (S-T8 a F) und das Griffbild als Bild
 *         (S-T8 a D, E) — docs/audio/Saiten_Konzept.md §1
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Die Namen und die Lage ohne Qt-Anwendung; das Bild mit einer. Die mitgelieferten
 * Vorlagen werden vom Arbeitsordner aus gelesen — die Test-Exe aus dem Projekt-Root
 * starten. `LUMIVIZ_GRIFFBILD_BILD=<datei.png>` legt einen Bogen mit Griffbildern ab.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Akkord.hpp"
#include "loop/GriffbildBild.hpp"

#include "LoopOhneEngine.hpp"  // repoWurzel

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QTemporaryDir>
#include <QPainter>
#include <QPalette>

#include <algorithm>
#include <set>
#include <vector>

using namespace lumi::loop;

namespace
{

struct Anwendung
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
};

const std::vector<int> kGitarre = {40, 45, 50, 55, 59, 64};

Akkord griff(std::vector<int> buende, std::vector<int> finger = {}, std::vector<int> barre = {})
{
    Akkord a;
    a.leer = kGitarre;
    a.buende = std::move(buende);
    a.finger = std::move(finger);
    a.barre = std::move(barre);
    richteAkkord(a);
    return a;
}

std::vector<int> toeneVon(const Akkord& a)
{
    Saitensatz s;
    s.leer = a.leer;
    std::vector<int> h;
    if (const auto toene = toeneDesGriffbilds(a, s))
        for (const AkkordTon& t : *toene) h.push_back(t.hoehe);
    return h;
}

const AkkordArt* artNamens(const QString& name)
{
    for (const AkkordArt& art : akkordArten())
        if (art.name == name) return &art;
    return nullptr;
}

int klassen(const std::vector<int>& stufen, int grundton = 0)
{
    int k = 0;
    for (const int s : stufen) k |= 1 << ((((s - grundton) % 12) + 12) % 12);
    return k;
}

}  // namespace

TEST_CASE("Loop Griffbild 26e-1: der Name aus den Toenen")
{
    // C-Dur offen x32010: C3 E3 G3 C4 E4
    CHECK(erkenneAkkord(toeneVon(griff({-1, 3, 2, 0, 1, 0}))) == AkkordName{0, QStringLiteral("major"), -1});
    // E-Dur, Am, G7, Dm7
    CHECK(erkenneAkkord(toeneVon(griff({0, 2, 2, 1, 0, 0}))) == AkkordName{4, QStringLiteral("major"), -1});
    CHECK(erkenneAkkord(toeneVon(griff({-1, 0, 2, 2, 1, 0}))) == AkkordName{9, QStringLiteral("minor"), -1});
    CHECK(erkenneAkkord(toeneVon(griff({3, 2, 0, 0, 0, 1}))) == AkkordName{7, QStringLiteral("7"), -1});
    CHECK(erkenneAkkord(toeneVon(griff({-1, -1, 0, 2, 1, 1}))) == AkkordName{2, QStringLiteral("m7"), -1});
    // C/E: dieselben Toene, E im Bass — und Am/G
    CHECK(erkenneAkkord(toeneVon(griff({0, 3, 2, 0, 1, 0}))) == AkkordName{0, QStringLiteral("major"), 4});
    CHECK(erkenneAkkord(toeneVon(griff({3, 0, 2, 2, 1, 0}))) == AkkordName{9, QStringLiteral("minor"), 7});
    // A/G#: der Bass gehoert nicht zum Akkord (chords-db a_over_ab_1, 442255)
    CHECK(erkenneAkkord(toeneVon(griff({4, 4, 2, 2, 5, 5}))) == AkkordName{9, QStringLiteral("major"), 8});
    // C6 gegen Am7 — derselbe Klang, der Bass entscheidet
    CHECK(erkenneAkkord({48, 52, 55, 57}) == AkkordName{0, QStringLiteral("6"), -1});
    CHECK(erkenneAkkord({45, 48, 52, 55}) == AkkordName{9, QStringLiteral("m7"), -1});
    // C-E-A: Am/C, nicht C6 ohne Quinte
    CHECK(erkenneAkkord({48, 52, 57}) == AkkordName{9, QStringLiteral("minor"), 0});
    // C7 ohne Quinte, Cmaj9 ohne Quinte
    CHECK(erkenneAkkord({48, 52, 58}) == AkkordName{0, QStringLiteral("7"), -1});
    CHECK(erkenneAkkord({48, 52, 59, 62}) == AkkordName{0, QStringLiteral("maj9"), -1});
    // ein Dreiklang ohne Quinte ist kein Akkord; zwei Klassen auch nicht (Power chord)
    CHECK_FALSE(erkenneAkkord({48, 52, 60}).has_value());
    CHECK_FALSE(erkenneAkkord({40, 47, 52}).has_value());
    CHECK_FALSE(erkenneAkkord({}).has_value());
    // ein Cluster: nichts
    CHECK_FALSE(erkenneAkkord({48, 49, 50, 51}).has_value());

    CHECK(akkordSymbol({0, QStringLiteral("major"), -1}) == QStringLiteral("C"));
    CHECK(akkordSymbol({9, QStringLiteral("minor"), 7}) == QStringLiteral("Am/G"));
    CHECK(akkordSymbol({10, QStringLiteral("7b9"), -1}) == QStringLiteral("Bb7b9"));
    CHECK(akkordArtText({0, QStringLiteral("major"), 4}) == QStringLiteral("/E"));
    CHECK(akkordArtText({9, QStringLiteral("minor"), 7}) == QStringLiteral("m/G"));
    CHECK(akkordArtText({0, QStringLiteral("maj7"), -1}) == QStringLiteral("maj7"));
}

TEST_CASE("Loop Griffbild 26e-1: die Datei der Arten (S-T13 a)")
{
    QTemporaryDir ordner;
    REQUIRE(ordner.isValid());
    const auto schreibe = [&](const QByteArray& inhalt) {
        const QString pfad = QDir(ordner.path()).filePath(QStringLiteral("types.json"));
        QFile datei(pfad);
        REQUIRE(datei.open(QIODevice::WriteOnly));
        datei.write(inhalt);
        return pfad;
    };
    std::vector<AkkordArt> arten;
    REQUIRE(ladeAkkordArten(schreibe(R"({"format": "lumiviz-chord-types", "version": 1, "types": [
        {"name": "major", "steps": [0, 4, 7]}, {"name": "9", "steps": [0, 4, 7, 10, 14]}]})"),
                            arten));
    REQUIRE(arten.size() == 2);
    CHECK(arten[1].name == QStringLiteral("9"));
    CHECK(arten[1].stufen == std::vector<int>{0, 4, 7, 10, 14});
    // nicht gueltig — und dann bleibt, was war
    for (const char* falsch : {
             R"({"format": "lumiviz-chord", "version": 1, "types": [{"name": "major", "steps": [0, 4, 7]}]})",
             R"({"format": "lumiviz-chord-types", "version": 2, "types": [{"name": "major", "steps": [0, 4, 7]}]})",
             R"({"format": "lumiviz-chord-types", "version": 1, "types": []})",
             R"({"format": "lumiviz-chord-types", "version": 1, "types": [{"name": "", "steps": [0, 4, 7]}]})",
             R"({"format": "lumiviz-chord-types", "version": 1, "types": [{"name": "x", "steps": [4, 7]}]})",
             R"({"format": "lumiviz-chord-types", "version": 1, "types": [{"name": "x", "steps": [0, 7, 4]}]})",
             R"({"format": "lumiviz-chord-types", "version": 1, "types": [{"name": "x", "steps": [0, 4, 4]}]})",
             R"({"format": "lumiviz-chord-types", "version": 1, "types": [{"name": "x", "steps": [0, 60]}]})",
             R"({"format": "lumiviz-chord-types", "version": 1, "types": [{"name": "x", "steps": [0, 4]}, {"name": "x", "steps": [0, 3]}]})",
             "kein JSON"})
    {
        CAPTURE(falsch);
        CHECK_FALSE(ladeAkkordArten(schreibe(falsch), arten));
        CHECK(arten.size() == 2);
    }
    CHECK_FALSE(ladeAkkordArten(QDir(ordner.path()).filePath(QStringLiteral("fehlt.json")), arten));

    // die mitgelieferte liest das Programm: 45 Arten, Dur und Moll vorn
    REQUIRE(akkordArten().size() == 45);
    CHECK(akkordArten()[0].name == QStringLiteral("major"));
    CHECK(akkordArten()[1].name == QStringLiteral("minor"));
}

TEST_CASE("Loop Griffbild 26e-1: die Arten gegen die mitgelieferten Vorlagen")
{
    // jede Art einmal, aufsteigend, mit Grundton
    std::set<QString> namen;
    for (const AkkordArt& art : akkordArten())
    {
        CAPTURE(art.name.toStdString());
        CHECK(namen.insert(art.name).second);
        REQUIRE(!art.stufen.empty());
        CHECK(art.stufen.front() == 0);
        CHECK(std::is_sorted(art.stufen.begin(), art.stufen.end()));
    }

    const QDir basis(looptest::repoWurzel().filePath(QStringLiteral("asset/loop/chords")));
    // jede Art der Vorlagen kennt das Programm — bis auf alt und 7sg (ohne gaengige Bedeutung)
    const auto artOhneBass = [](const QString& art) {
        const QString vorn = art.section(QLatin1Char('/'), 0, 0);
        return vorn.isEmpty() ? QStringLiteral("major") : vorn == QLatin1String("m") ? QStringLiteral("minor") : vorn;
    };
    int gerechnet = 0;
    int voicings = 0;
    int alsSlash = 0;
    int vollstaendig = 0;
    int vollstaendigErkannt = 0;
    int alle = 0;
    int alleErkannt = 0;
    for (const char* ordner : {"guitar", "ukulele", "drop_d", "bass", "violin", "voicings"})
    {
        const QDir hier(basis.filePath(QLatin1String(ordner)));
        for (const QString& datei : hier.entryList({QStringLiteral("*.lvchord")}, QDir::Files))
        {
            CAPTURE(datei.toStdString());
            Akkord a;
            REQUIRE(ladeAkkord(hier.filePath(datei), a));
            const QString art = artOhneBass(a.art);
            if (art == QLatin1String("alt") || art == QLatin1String("7sg")) continue;
            const AkkordArt* bekannt = artNamens(art);
            CHECK(bekannt != nullptr);
            if (bekannt == nullptr) continue;

            std::vector<int> h;
            if (a.istGriffbild())
                h = toeneVon(a);
            else
            {
                // ein Voicing nennt seine Toene ueber dem tiefsten — welcher Ton der Art unten liegt
                // (Umkehrung, Drop 2), steht nicht in der Datei: so verschieben, dass die Klassen die Art bilden
                int unten = -1;
                for (int o = 0; o < 12 && unten < 0; ++o)
                    if (klassen(a.abstaende, -o) == klassen(bekannt->stufen)) unten = o;
                REQUIRE(unten >= 0);
                if (const auto t = toeneDesVoicings(a, 48 + a.grundton + unten))
                    for (const AkkordTon& ton : *t) h.push_back(ton.hoehe);
            }
            REQUIRE(!h.empty());
            const int bass = *std::min_element(h.begin(), h.end()) % 12;
            const std::optional<AkkordName> name = erkenneAkkord(h);

            if (a.herkunft == QLatin1String(kHerkunftGerechnet) || !a.istGriffbild())
            {
                // gerechnet (S-T4 a) und die Voicings (26b): aus denselben Abstaenden — der Name stimmt
                if (bass == a.grundton)
                {
                    (a.istGriffbild() ? gerechnet : voicings) += 1;
                    REQUIRE(name.has_value());
                    if (name->bass >= 0)
                    {
                        // ohne Quinte als Slash gehoert (C-E-A eines C6 ist Am/C): dieselben Toene
                        ++alsSlash;
                        CHECK(a.istGriffbild());
                        continue;
                    }
                    CHECK(name == AkkordName{a.grundton, art, -1});
                }
                else if (art == QLatin1String("major") || art == QLatin1String("minor"))
                    CHECK(name == AkkordName{a.grundton, art, bass});
                continue;
            }
            // chords-db: eine vollstaendige Lage mit dem Grundton im Bass heisst wie in der Quelle —
            // ausser eine Art davor bildet dieselben Klassen (7b9#5 und b13b9)
            ++alle;
            if (name && name->grundton == a.grundton && name->art == art) ++alleErkannt;
            int imGriff = 0;
            for (const int ton : h) imGriff |= 1 << (((ton - a.grundton) % 12 + 12) % 12);
            if (bass != a.grundton || imGriff != klassen(bekannt->stufen) || a.art.contains(QLatin1Char('/'))) continue;
            const AkkordArt* erste = nullptr;
            for (const AkkordArt& andere : akkordArten())
                if (klassen(andere.stufen) == imGriff)
                {
                    erste = &andere;
                    break;
                }
            if (erste != bekannt) continue;
            ++vollstaendig;
            CHECK(name == AkkordName{a.grundton, art, -1});
            if (name == AkkordName{a.grundton, art, -1}) ++vollstaendigErkannt;
        }
    }
    MESSAGE("gerechnet " << gerechnet << " (davon als Slash gehoert " << alsSlash << "), Voicings " << voicings << "; chords-db vollstaendig mit Grundton im Bass "
                         << vollstaendigErkannt << " / " << vollstaendig << ", alle Lagen mit ihrem Namen erkannt " << alleErkannt
                         << " / " << alle);
    CHECK(gerechnet > 1000);
    CHECK(voicings >= 19 * 12);
    CHECK(vollstaendig > 500);
}

TEST_CASE("Loop Griffbild 26e-1: verschoben auf dem Griffbrett")
{
    // C-Dur offen x32010, zwei Buende hoeher: x54232 — die leeren Saiten werden gegriffen
    const Akkord c = griff({-1, 3, 2, 0, 1, 0}, {0, 3, 2, 0, 1, 0});
    const std::optional<Akkord> d = verschobenesGriffbild(c, 2);
    REQUIRE(d.has_value());
    CHECK(d->buende == std::vector<int>{-1, 5, 4, 2, 3, 2});
    CHECK(d->finger == c.finger);
    CHECK(toeneVon(*d) == std::vector<int>{50, 54, 57, 62, 66});  // D-Dur
    CHECK(erkenneAkkord(toeneVon(*d)) == AkkordName{2, QStringLiteral("major"), -1});
    // das Barre wandert mit; unter den Sattel nicht
    const Akkord f = griff({1, 3, 3, 2, 1, 1}, {1, 3, 4, 2, 1, 1}, {1});
    CHECK(verschobenesGriffbild(f, 2)->barre == std::vector<int>{3});
    CHECK_FALSE(verschobenesGriffbild(f, -1).has_value());
    CHECK_FALSE(verschobenesGriffbild(c, kBuendeMax).has_value());
    Akkord v;
    v.abstaende = {0, 4, 7};
    CHECK_FALSE(verschobenesGriffbild(v, 1).has_value());
    // dieselbe Regel wie in den Noten
    Saitensatz s;
    s.leer = kGitarre;
    for (int n = 0; n < 6; ++n)
        CHECK(toeneDesGriffbilds(c, s, n).has_value() == verschobenesGriffbild(c, n).has_value());
}

TEST_CASE("Loop Griffbild 26e-1: welche Buende, wo die Punkte, was ein Klick trifft")
{
    CHECK(griffbildFenster(griff({-1, 3, 2, 0, 1, 0})) == GriffbildFenster{1, 4});
    CHECK(griffbildFenster(griff({-1, -1, -1, -1, -1, -1})) == GriffbildFenster{1, 4});
    CHECK(griffbildFenster(griff({8, 10, 10, 9, 8, 8})) == GriffbildFenster{8, 4});
    CHECK(griffbildFenster(griff({-1, 3, 5, 5, 5, 3})) == GriffbildFenster{3, 4});
    CHECK(griffbildFenster(griff({-1, 7, 0, 0, 12, 0})) == GriffbildFenster{7, 6});  // weit: so viele wie noetig

    const GriffbildLage lage(QRectF(20, 40, 100, 160), 6, GriffbildFenster{1, 4});
    CHECK(lage.xSaite(0) == doctest::Approx(20));
    CHECK(lage.xSaite(5) == doctest::Approx(120));
    CHECK(lage.yLinie(0) == doctest::Approx(40));   // der Sattel
    CHECK(lage.yLinie(4) == doctest::Approx(200));
    CHECK(lage.punkt(1, 3).y() == doctest::Approx(140));  // zwischen der Linie unter dem 2. und dem 3. Bund
    for (int saite = 0; saite < 6; ++saite)
        for (int bund = 1; bund <= 4; ++bund)
            CHECK(lage.trifft(lage.punkt(saite, bund)) == GriffbildLage::Treffer{saite, bund});
    CHECK(lage.trifft(lage.oben(2)) == GriffbildLage::Treffer{2, 0});
    CHECK_FALSE(lage.trifft(QPointF(0, 100)).has_value());
    CHECK_FALSE(lage.trifft(QPointF(70, 230)).has_value());
    const GriffbildLage hoch(QRectF(0, 0, 100, 100), 6, GriffbildFenster{8, 4});
    CHECK(hoch.trifft(hoch.punkt(0, 8)) == GriffbildLage::Treffer{0, 8});
    CHECK(hoch.trifft(hoch.punkt(5, 11)) == GriffbildLage::Treffer{5, 11});
}

TEST_CASE("Loop Griffbild 26e-1: das Bild — Punkte, Barre, × und ○ (LUMIVIZ_GRIFFBILD_BILD)")
{
    Anwendung anwendung;
    QPalette palette;
    palette.setColor(QPalette::Base, Qt::white);
    palette.setColor(QPalette::Text, Qt::black);
    palette.setColor(QPalette::Highlight, QColor(0x37, 0x8A, 0xDD));
    palette.setColor(QPalette::HighlightedText, Qt::white);
    const auto male = [&](const Akkord& a, GriffbildGroesse g, QSize groesse) {
        QImage bild(groesse, QImage::Format_ARGB32);
        bild.fill(Qt::white);
        QPainter p(&bild);
        zeichneGriffbild(p, QRectF(QPointF(0, 0), QSizeF(groesse)), a, g, palette);
        p.end();
        return bild;
    };
    const auto blau = [](QRgb farbe) { return qBlue(farbe) > 180 && qRed(farbe) < 120; };

    // C-Dur offen: Punkte auf A3, D2, B1, sonst nicht
    const Akkord c = griff({-1, 3, 2, 0, 1, 0}, {0, 3, 2, 0, 1, 0});
    for (const GriffbildGroesse g : {GriffbildGroesse::Klein, GriffbildGroesse::Gross})
    {
        const QSize groesse = g == GriffbildGroesse::Klein ? QSize(56, 64) : QSize(160, 190);
        const QImage bild = male(c, g, groesse);
        const GriffbildLage lage = griffbildLageIn(QRectF(QPointF(0, 0), QSizeF(groesse)), c, g);
        const auto bei = [&](QPointF o) { return bild.pixel(o.toPoint()); };
        // im Punkt, neben der Mitte (dort steht gross der Finger)
        const QPointF neben(-lage.halbmesser() * 0.7, 0.0);
        CHECK(blau(bei(lage.punkt(1, 3) + neben)));
        CHECK(blau(bei(lage.punkt(2, 2) + neben)));
        CHECK(blau(bei(lage.punkt(4, 1) + neben)));
        CHECK_FALSE(blau(bei(lage.punkt(3, 1) + neben)));
        CHECK_FALSE(blau(bei(lage.punkt(0, 3) + neben)));
        // × ueber der tiefen E-Saite: in der Mitte dunkel; ○ ueber der G-Saite: in der Mitte hell
        int dunkel = 255;
        for (int dx = -1; dx <= 1; ++dx)
            for (int dy = -1; dy <= 1; ++dy) dunkel = std::min(dunkel, qGray(bei(lage.oben(0) + QPointF(dx, dy))));
        CHECK(dunkel < 128);
        CHECK(qGray(bei(lage.oben(3))) > 200);
        // der Sattel ist dunkel, zwischen den Saiten
        CHECK(qGray(bei(QPointF((lage.xSaite(2) + lage.xSaite(3)) / 2.0, lage.yLinie(0)))) < 100);
    }
    // F-Dur mit Barre am 1. Bund: der Balken fuellt die Stelle zwischen den Saiten
    const Akkord f = griff({1, 3, 3, 2, 1, 1}, {1, 3, 4, 2, 1, 1}, {1});
    {
        const QSize groesse(160, 190);
        const QImage bild = male(f, GriffbildGroesse::Gross, groesse);
        const GriffbildLage lage = griffbildLageIn(QRectF(QPointF(0, 0), QSizeF(groesse)), f, GriffbildGroesse::Gross);
        const QPointF zwischen((lage.xSaite(3) + lage.xSaite(4)) / 2.0, lage.punkt(0, 1).y());
        CHECK(blau(bild.pixel(zwischen.toPoint())));
    }
    // ein Voicing zeichnet nichts
    {
        Akkord v;
        v.abstaende = {0, 4, 7};
        const QImage bild = male(v, GriffbildGroesse::Gross, QSize(60, 60));
        CHECK(bild.pixel(30, 30) == qRgb(255, 255, 255));
    }

    if (const QString ziel = qEnvironmentVariable("LUMIVIZ_GRIFFBILD_BILD"); !ziel.isEmpty())
    {
        // ein Bogen: die Lagen von C, Cm, C7, F und Bb der Gitarre — klein und gross
        std::vector<Akkord> zeigen;
        for (const Akkord& a : mitgelieferteAkkorde())
            if (a.name.startsWith(QLatin1String("guitar/")) &&
                (a.name.startsWith(QLatin1String("guitar/c_major_")) || a.name.startsWith(QLatin1String("guitar/c_minor_")) ||
                 a.name.startsWith(QLatin1String("guitar/c_7_")) || a.name.startsWith(QLatin1String("guitar/f_major_")) ||
                 a.name.startsWith(QLatin1String("guitar/bb_major_"))))
                zeigen.push_back(a);
        const int spalten = 5;
        const QSize klein(64, 74);
        const QSize gross(170, 200);
        const int zeilen = (static_cast<int>(zeigen.size()) + spalten - 1) / spalten;
        QImage bogen(spalten * (klein.width() + 8 + gross.width() + 16), zeilen * (gross.height() + 24), QImage::Format_ARGB32);
        bogen.fill(Qt::white);
        QPainter p(&bogen);
        for (std::size_t i = 0; i < zeigen.size(); ++i)
        {
            const int x = static_cast<int>(i % spalten) * (klein.width() + 8 + gross.width() + 16);
            const int y = static_cast<int>(i / spalten) * (gross.height() + 24);
            zeichneGriffbild(p, QRectF(x, y + 18, klein.width(), klein.height()), zeigen[i], GriffbildGroesse::Klein, palette);
            zeichneGriffbild(p, QRectF(x + klein.width() + 8, y + 18, gross.width(), gross.height()), zeigen[i],
                             GriffbildGroesse::Gross, palette);
            p.setPen(Qt::black);
            p.drawText(QPointF(x + 2, y + 14), zeigen[i].titel + QStringLiteral(" · ") + zeigen[i].name.section(QLatin1Char('/'), 1));
        }
        p.end();
        CHECK(bogen.save(ziel));
        MESSAGE("Griffbilder: " << ziel.toStdString() << " (" << zeigen.size() << ")");
    }
}
