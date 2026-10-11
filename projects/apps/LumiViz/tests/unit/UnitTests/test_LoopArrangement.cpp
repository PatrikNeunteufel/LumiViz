/**
 * @file test_LoopArrangement.cpp
 * @brief Loop, Stufe 6a: das Arrangement bearbeiten — die Funktionen auf dem
 *        Modell (`loop/Bearbeiten`), die Achse des Panels (`loop/Taktachse`)
 *        und das Panel mit Datei-Zeile und Undo
 *        (docs/audio/Loop_Konzept.md §4c)
 *
 * Ohne Ton und ohne Fenster: Achse und Panel werden nicht gezeigt, die Maus
 * kommt als Ereignis.
 */

#include <doctest.h>

#include "loop/Bearbeiten.hpp"
#include "loop/Loop.hpp"
#include "loop/LoopPanel.hpp"
#include "loop/Taktachse.hpp"
#include "services/CommandBus.hpp"
#include "services/ICommandBus.hpp"
#include "services/ServiceContainer.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QDoubleSpinBox>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QSettings>
#include <QTemporaryDir>

#include <memory>

using namespace lumi::loop;

namespace
{

/// Zwei Basen (A: 4 Takte, B: 8 Takte), zwei Kanaele:
/// Drums: A ab Takt 0 zweimal (0 bis 8), B ab Takt 8 (8 bis 16)
/// Perc:  A ab Takt 4 (4 bis 8)
Loop probe()
{
    Loop loop;
    Muster a;
    a.name = QStringLiteral("A");
    a.takte = 4;
    Muster b;
    b.name = QStringLiteral("B");
    b.takte = 8;
    loop.muster = {a, b};
    loop.kanaele = {{QStringLiteral("Drums"), true, {{QStringLiteral("A"), 0, 2}, {QStringLiteral("B"), 8, 1}}},
                    {QStringLiteral("Perc"), true, {{QStringLiteral("A"), 4, 1}}}};
    return loop;
}

const LoopClip& clipIn(const Loop& loop, int kanal, int clip)
{
    return loop.kanaele[static_cast<std::size_t>(kanal)].clips[static_cast<std::size_t>(clip)];
}

/// Die Regeln des Modells gelten noch: `richte` findet nichts zu richten.
bool inOrdnung(const Loop& loop)
{
    Loop gerichtet = loop;
    QStringList bericht;
    gerichtet.richte(&bericht);
    return bericht.isEmpty() && gerichtet == loop;
}

void maus(QWidget& ziel, QEvent::Type art, QPoint ort, Qt::MouseButton taste, Qt::MouseButtons tasten)
{
    QMouseEvent ereignis(art, QPointF(ort), QPointF(ziel.mapToGlobal(ort)), taste, tasten, Qt::NoModifier);
    QCoreApplication::sendEvent(&ziel, &ereignis);
}

void ziehe(QWidget& ziel, QPoint von, QPoint nach, bool loslassen = true)
{
    maus(ziel, QEvent::MouseButtonPress, von, Qt::LeftButton, Qt::LeftButton);
    maus(ziel, QEvent::MouseMove, (von + nach) / 2, Qt::NoButton, Qt::LeftButton);
    maus(ziel, QEvent::MouseMove, nach, Qt::NoButton, Qt::LeftButton);
    if (loslassen) maus(ziel, QEvent::MouseButtonRelease, nach, Qt::LeftButton, Qt::NoButton);
}

void taste(QWidget& ziel, int key, Qt::KeyboardModifiers tasten = Qt::NoModifier)
{
    QKeyEvent ereignis(QEvent::KeyPress, key, tasten);
    QCoreApplication::sendEvent(&ziel, &ereignis);
}

struct EigeneEinstellungen
{
    QString org = QCoreApplication::organizationName();
    QString name = QCoreApplication::applicationName();
    EigeneEinstellungen()
    {
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("LoopArrangement"));
        QSettings().clear();
    }
    ~EigeneEinstellungen()
    {
        QSettings().clear();
        QCoreApplication::setOrganizationName(org);
        QCoreApplication::setApplicationName(name);
    }
};

}  // namespace

TEST_SUITE("LoopArrangement")
{
    TEST_CASE("Loop bearbeiten: ein neuer Clip ist eine leere Basis von vier Takten und braucht Platz")
    {
        Loop loop = probe();
        REQUIRE(inOrdnung(loop));
        CHECK(istFrei(loop, 1, 0, 4));
        CHECK_FALSE(istFrei(loop, 1, 2, 4));  // reicht in den Clip ab Takt 4
        CHECK_FALSE(istFrei(loop, 0, 4, 4));
        CHECK(istFrei(loop, 0, 4, 4, 0));     // der Clip selbst zaehlt nicht
        CHECK_FALSE(istFrei(loop, 5, 0, 4));  // den Kanal gibt es nicht
        CHECK_FALSE(istFrei(loop, 1, -1, 4));

        CHECK(neuerClip(loop, 1, 2) == -1);
        CHECK(loop == probe());
        CHECK(neuerClip(loop, 1, 8) == 1);
        CHECK(neuerClip(loop, 1, 0) == 0);  // sortiert: er liegt jetzt vorn
        REQUIRE(loop.muster.size() == 4);
        CHECK(clipIn(loop, 1, 0).muster == QStringLiteral("Pattern 2"));
        CHECK(clipIn(loop, 1, 2).muster == QStringLiteral("Pattern 1"));
        const Muster* neu = loop.findeMuster(QStringLiteral("Pattern 1"));
        REQUIRE(neu != nullptr);
        CHECK(neu->takte == kTakteMin);
        CHECK(neu->art == MusterArt::Drumkit);
        CHECK(neu->spuren.empty());
        CHECK(loop.clipTakte(clipIn(loop, 1, 0)) == 4);
        CHECK(inOrdnung(loop));
    }

    TEST_CASE("Loop bearbeiten: verschieben — an den Wunsch, sonst an die naechste freie Kante")
    {
        Loop loop = probe();
        // im selben Kanal, frei: B von 8 nach 10
        CHECK(versetzeClip(loop, 0, 1, 0, 10) == 1);
        CHECK(clipIn(loop, 0, 1).startTakt == 10);
        // in den Clip davor hinein: er haelt an dessen Ende
        CHECK(versetzeClip(loop, 0, 1, 0, 6) == 1);
        CHECK(clipIn(loop, 0, 1).startTakt == 8);
        // vor den Anfang: Takt 0 ist besetzt, also wieder ans Ende von A
        CHECK(versetzeClip(loop, 0, 1, 0, -5) == 1);
        CHECK(clipIn(loop, 0, 1).startTakt == 8);
        CHECK(loop == probe());

        // in den anderen Kanal, wo kein Platz ist: hinter den letzten Clip
        CHECK(versetzeClip(loop, 1, 0, 0, 4) == 2);
        CHECK(loop.kanaele[1].clips.empty());
        REQUIRE(loop.kanaele[0].clips.size() == 3);
        CHECK(clipIn(loop, 0, 2).startTakt == 16);
        CHECK(clipIn(loop, 0, 2).muster == QStringLiteral("A"));
        // und zurueck, an den Anfang des leeren Kanals
        CHECK(versetzeClip(loop, 0, 2, 1, 0) == 0);
        CHECK(clipIn(loop, 1, 0).startTakt == 0);
        CHECK(inOrdnung(loop));

        // A (zweimal, 8 Takte) ueber B hinweg: ab der Mitte liegt die Kante dahinter naeher
        CHECK(versetzeClip(loop, 0, 0, 0, 7) == 0);
        CHECK(clipIn(loop, 0, 0).startTakt == 0);
        CHECK(versetzeClip(loop, 0, 0, 0, 9) == 1);
        CHECK(clipIn(loop, 0, 1).startTakt == 16);
        CHECK(clipIn(loop, 0, 0).muster == QStringLiteral("B"));
        CHECK(inOrdnung(loop));

        CHECK(versetzeClip(loop, 0, 7, 0, 0) == -1);  // den Clip gibt es nicht
        CHECK(versetzeClip(loop, 0, 0, 9, 0) == -1);  // den Kanal nicht
    }

    TEST_CASE("Loop bearbeiten: an den Kanten wird der Clip in ganzen Mustern laenger und kuerzer")
    {
        Loop loop = probe();
        // rechte Kante von A (zweimal, der naechste Clip beginnt bei 8)
        CHECK(setzeEnde(loop, 0, 0, 13) == 0);
        CHECK(clipIn(loop, 0, 0).wiederholungen == 2);  // weiter geht es nicht
        CHECK(setzeEnde(loop, 0, 0, 5) == 0);
        CHECK(clipIn(loop, 0, 0).wiederholungen == 1);
        CHECK(setzeEnde(loop, 0, 0, -3) == 0);
        CHECK(clipIn(loop, 0, 0).wiederholungen == 1);  // mindestens einmal
        CHECK(setzeEnde(loop, 0, 0, 6) == 0);
        CHECK(clipIn(loop, 0, 0).wiederholungen == 2);  // 6 liegt naeher an 8 als an 4
        // ohne Nachbarn: so oft, wie die Maus reicht
        CHECK(setzeEnde(loop, 1, 0, 17) == 0);
        CHECK(clipIn(loop, 1, 0).wiederholungen == 3);
        CHECK(loop.clipTakte(clipIn(loop, 1, 0)) == 12);
        CHECK(setzeEnde(loop, 1, 0, 8) == 0);
        CHECK(loop == probe());

        // linke Kante: das Ende bleibt
        CHECK(setzeAnfang(loop, 1, 0, 0) == 0);
        CHECK(clipIn(loop, 1, 0).startTakt == 0);
        CHECK(clipIn(loop, 1, 0).wiederholungen == 2);
        CHECK(setzeAnfang(loop, 1, 0, -9) == 0);
        CHECK(clipIn(loop, 1, 0).startTakt == 0);  // vor Takt 0 geht es nicht
        CHECK(setzeAnfang(loop, 1, 0, 7) == 0);
        CHECK(clipIn(loop, 1, 0).startTakt == 4);
        CHECK(clipIn(loop, 1, 0).wiederholungen == 1);
        // B (8 Takte) stoesst links an A: es bleibt, wie es ist
        CHECK(setzeAnfang(loop, 0, 1, 0) == 1);
        CHECK(loop == probe());
        CHECK(inOrdnung(loop));
    }

    TEST_CASE("Loop bearbeiten: Kopien bleiben verbunden, eine Basis ohne Clip geht")
    {
        Loop loop = probe();
        CHECK(basisNutzer(loop, QStringLiteral("A")) == 2);
        // hinter A liegt B: die Kopie kommt hinter B
        CHECK(dupliziere(loop, 0, 0) == 2);
        CHECK(clipIn(loop, 0, 2).startTakt == 16);
        CHECK(clipIn(loop, 0, 2).wiederholungen == 2);
        CHECK(clipIn(loop, 0, 2).muster == QStringLiteral("A"));
        CHECK(loop.muster.size() == 2);  // keine neue Basis
        CHECK(basisNutzer(loop, QStringLiteral("A")) == 3);
        CHECK(loop.durchlaufTakte() == 24);

        // umbenennen gilt fuer alle Kopien
        CHECK_FALSE(benenneBasisUm(loop, QStringLiteral("A"), QStringLiteral("B")));
        CHECK_FALSE(benenneBasisUm(loop, QStringLiteral("A"), QStringLiteral("  ")));
        CHECK_FALSE(benenneBasisUm(loop, QStringLiteral("gibt es nicht"), QStringLiteral("X")));
        CHECK(benenneBasisUm(loop, QStringLiteral("A"), QStringLiteral(" Grund ")));
        CHECK(basisNutzer(loop, QStringLiteral("Grund")) == 3);
        CHECK(basisNutzer(loop, QStringLiteral("A")) == 0);
        CHECK(loop.findeMuster(QStringLiteral("A")) == nullptr);
        CHECK(inOrdnung(loop));

        // der letzte Clip einer Basis nimmt sie mit
        CHECK(loescheClip(loop, 0, 1));
        CHECK(loop.findeMuster(QStringLiteral("B")) == nullptr);
        CHECK(loop.muster.size() == 1);
        CHECK(loescheClip(loop, 0, 0));
        CHECK(loop.findeMuster(QStringLiteral("Grund")) != nullptr);  // zwei Kopien spielen sie noch
        CHECK_FALSE(loescheClip(loop, 0, 5));
        CHECK(inOrdnung(loop));
    }

    TEST_CASE("Loop bearbeiten: Kanaele anlegen, umbenennen, schalten, loeschen")
    {
        Loop loop = probe();
        CHECK(kanalHinzu(loop) == 2);
        CHECK(loop.kanaele[2].name == QStringLiteral("Channel 1"));
        CHECK(loop.kanaele[2].an);
        CHECK(kanalHinzu(loop) == 3);
        CHECK(loop.kanaele[3].name == QStringLiteral("Channel 2"));
        CHECK(kanalUmbenennen(loop, 3, QStringLiteral(" Bass ")));
        CHECK(loop.kanaele[3].name == QStringLiteral("Bass"));
        CHECK_FALSE(kanalUmbenennen(loop, 3, QString()));
        CHECK_FALSE(kanalUmbenennen(loop, 9, QStringLiteral("X")));
        CHECK(kanalSchalten(loop, 0, false));
        CHECK_FALSE(loop.kanaele[0].an);
        CHECK(loop.durchlaufTakte() == 16);  // ein Kanal, der aus ist, zaehlt fuer die Laenge

        // Drums geht: B spielte nur dort, A lebt in Perc weiter
        CHECK(kanalLoeschen(loop, 0));
        REQUIRE(loop.kanaele.size() == 3);
        CHECK(loop.kanaele[0].name == QStringLiteral("Perc"));
        CHECK(loop.findeMuster(QStringLiteral("B")) == nullptr);
        CHECK(loop.findeMuster(QStringLiteral("A")) != nullptr);
        CHECK_FALSE(kanalLoeschen(loop, 3));
        CHECK(inOrdnung(loop));
    }

    TEST_CASE("Taktachse: Geometrie in Takten, Auswahl, Ziehen und die Aktionen des Rechtsklicks")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};

        Taktachse achse;  // nicht gezeigt: kein Fenster
        achse.resize(900, 260);
        achse.setLoop(probe(), true);
        CHECK_FALSE(achse.grab().isNull());

        int aenderungen = 0;
        QString zuletzt;
        QObject::connect(&achse, &Taktachse::bearbeitet, &achse, [&](const Loop& neu, const QString& was) {
            ++aenderungen;
            zuletzt = was;
            achse.zeigeBearbeitet(neu);  // wie das Panel: die Fassung kommt zurueck
        });

        SUBCASE("Geometrie: 16 Takte und Luft dahinter, ein Clip liegt auf seinen Takten")
        {
            CHECK(Taktachse::anzeigeTakte(probe()) == 20);
            CHECK(Taktachse::anzeigeTakte(Loop{}) == 16);
            CHECK(achse.zeitGanz());
            CHECK(achse.xVonTakt(0) == Taktachse::kKopfBreite);
            CHECK(achse.xVonTakt(20) == achse.width() - Taktachse::kRand);
            CHECK(achse.taktBei(achse.xVonTakt(5) + 3) == 5);
            const QRect a = achse.clipRechteck(0, 0);
            CHECK(a.left() == achse.xVonTakt(0));
            CHECK(a.right() + 1 == achse.xVonTakt(8));
            CHECK(a.top() == achse.zeileOben(0) + 3);
            CHECK(achse.clipRechteck(0, 9).isNull());
            CHECK(achse.kanalBei(achse.yVonZeile(1)) == 1);
            CHECK(achse.kanalBei(achse.yVonZeile(2)) == -1);  // die Zeile fuer einen neuen Kanal
            CHECK(achse.kanalBei(10) == -1);                  // das Lineal
            // keine Kopfzeilen: die erste Zeile beginnt unter dem Lineal
            CHECK(achse.zeileOben(0) == Taktachse::kLinealHoehe);
            // verbundene Kopien tragen eine Farbe, eine andere Basis eine andere
            CHECK(achse.basisFarbe(QStringLiteral("A"), true) != achse.basisFarbe(QStringLiteral("B"), true));
            CHECK(achse.basisFarbe(QStringLiteral("A"), true) != achse.basisFarbe(QStringLiteral("A"), false));
        }

        SUBCASE("ein Klick waehlt den Clip, einer daneben nimmt die Auswahl weg")
        {
            int gewaehlt = 0;
            QObject::connect(&achse, &Taktachse::auswahlGeaendert, &achse, [&] { ++gewaehlt; });
            const QPoint imClip = achse.clipRechteck(1, 0).center();
            maus(achse, QEvent::MouseButtonPress, imClip, Qt::LeftButton, Qt::LeftButton);
            maus(achse, QEvent::MouseButtonRelease, imClip, Qt::LeftButton, Qt::NoButton);
            CHECK(achse.gewaehlterKanal() == 1);
            CHECK(achse.gewaehlterClip() == 0);
            REQUIRE(achse.auswahl() != nullptr);
            CHECK(achse.auswahl()->muster == QStringLiteral("A"));
            CHECK(gewaehlt == 1);
            CHECK(aenderungen == 0);  // ein Klick aendert nichts
            const QPoint daneben(achse.xVonTakt(12) + 4, achse.yVonZeile(1));
            maus(achse, QEvent::MouseButtonPress, daneben, Qt::LeftButton, Qt::LeftButton);
            maus(achse, QEvent::MouseButtonRelease, daneben, Qt::LeftButton, Qt::NoButton);
            CHECK(achse.auswahl() == nullptr);
            CHECK(gewaehlt == 2);
        }

        SUBCASE("verschieben: der Clip folgt der Maus in ganzen Takten, auch in den anderen Kanal")
        {
            const QPoint von = achse.clipRechteck(1, 0).center();
            ziehe(achse, von, von + QPoint(achse.xVonTakt(8) - achse.xVonTakt(4), 0));
            CHECK(aenderungen == 1);
            CHECK(zuletzt == QStringLiteral("Move clip"));
            CHECK(clipIn(achse.loop(), 1, 0).startTakt == 8);
            CHECK(achse.gewaehlterKanal() == 1);

            // nach oben in Drums, hinter B (ab Takt 16)
            const QPoint jetzt = achse.clipRechteck(1, 0).center();
            ziehe(achse, jetzt, QPoint(jetzt.x() + achse.xVonTakt(8) - achse.xVonTakt(0), achse.yVonZeile(0)));
            CHECK(aenderungen == 2);
            CHECK(achse.loop().kanaele[1].clips.empty());
            REQUIRE(achse.loop().kanaele[0].clips.size() == 3);
            CHECK(clipIn(achse.loop(), 0, 2).startTakt == 16);
            CHECK(achse.gewaehlterKanal() == 0);
            CHECK(achse.gewaehlterClip() == 2);
            CHECK(inOrdnung(achse.loop()));
        }

        SUBCASE("Kanten: rechts laenger in ganzen Mustern, links bleibt das Ende; Esc bricht ab")
        {
            const QRect r = achse.clipRechteck(1, 0);
            ziehe(achse, QPoint(r.right() - 1, r.center().y()), QPoint(achse.xVonTakt(16), r.center().y()));
            CHECK(aenderungen == 1);
            CHECK(zuletzt == QStringLiteral("Resize clip"));
            CHECK(clipIn(achse.loop(), 1, 0).wiederholungen == 3);
            CHECK(clipIn(achse.loop(), 1, 0).startTakt == 4);

            const QRect lang = achse.clipRechteck(1, 0);
            ziehe(achse, QPoint(lang.left() + 1, lang.center().y()), QPoint(achse.xVonTakt(0), lang.center().y()));
            CHECK(aenderungen == 2);
            CHECK(clipIn(achse.loop(), 1, 0).startTakt == 0);
            CHECK(clipIn(achse.loop(), 1, 0).wiederholungen == 4);

            // mitten im Zug Esc: es bleibt, wie es war, und nichts wird gemeldet
            const Loop vorher = achse.loop();
            const QRect voll = achse.clipRechteck(1, 0);
            ziehe(achse, QPoint(voll.right() - 1, voll.center().y()), QPoint(achse.xVonTakt(8), voll.center().y()),
                  false);
            CHECK(clipIn(achse.loop(), 1, 0).wiederholungen == 2);  // waehrend des Zugs schon zu sehen
            taste(achse, Qt::Key_Escape);
            maus(achse, QEvent::MouseButtonRelease, QPoint(achse.xVonTakt(8), voll.center().y()), Qt::LeftButton,
                 Qt::NoButton);
            CHECK(achse.loop() == vorher);
            CHECK(aenderungen == 2);
        }

        SUBCASE("neuer Clip, Kopie, umbenennen, loeschen — und die Tasten Entf und Ctrl+D")
        {
            CHECK_FALSE(achse.neuerClip(1, 2));  // kein Platz
            CHECK(aenderungen == 0);
            CHECK(achse.neuerClip(1, 12));
            CHECK(zuletzt == QStringLiteral("New clip"));
            CHECK(achse.gewaehlterKanal() == 1);
            CHECK(achse.gewaehlterClip() == 1);
            REQUIRE(achse.auswahl() != nullptr);
            CHECK(achse.auswahl()->muster == QStringLiteral("Pattern 1"));

            CHECK(achse.benenneBasisUm(QStringLiteral("Fill")));
            CHECK(achse.auswahl()->muster == QStringLiteral("Fill"));
            CHECK_FALSE(achse.benenneBasisUm(QStringLiteral("A")));  // vergeben

            CHECK(achse.dupliziereAuswahl());
            CHECK(zuletzt == QStringLiteral("Duplicate clip"));
            CHECK(achse.gewaehlterClip() == 2);
            CHECK(clipIn(achse.loop(), 1, 2).startTakt == 16);
            CHECK(basisNutzer(achse.loop(), QStringLiteral("Fill")) == 2);

            taste(achse, Qt::Key_D, Qt::ControlModifier);
            CHECK(basisNutzer(achse.loop(), QStringLiteral("Fill")) == 3);
            taste(achse, Qt::Key_Delete);
            CHECK(zuletzt == QStringLiteral("Delete clip"));
            CHECK(basisNutzer(achse.loop(), QStringLiteral("Fill")) == 2);
            CHECK(achse.auswahl() == nullptr);
            CHECK_FALSE(achse.loescheAuswahl());  // nichts angeklickt
            CHECK(inOrdnung(achse.loop()));
            CHECK_FALSE(achse.grab().isNull());
        }

        SUBCASE("Kanaele: anlegen, umbenennen, schalten, loeschen — die Auswahl folgt ihrem Kanal")
        {
            achse.waehle(1, 0);
            achse.kanalHinzu();
            CHECK(zuletzt == QStringLiteral("Add channel"));
            REQUIRE(achse.loop().kanaele.size() == 3);
            CHECK(achse.kanalBei(achse.yVonZeile(2)) == 2);
            achse.kanalUmbenennen(2, QStringLiteral("Bass"));
            CHECK(achse.loop().kanaele[2].name == QStringLiteral("Bass"));
            const int vorher = aenderungen;
            achse.kanalUmbenennen(2, QStringLiteral("Bass"));  // derselbe Name: kein Schritt fuer Undo
            achse.kanalSchalten(2, true);                       // schon an
            CHECK(aenderungen == vorher);
            achse.kanalSchalten(0, false);
            CHECK(zuletzt == QStringLiteral("Channel off"));
            CHECK_FALSE(achse.loop().kanaele[0].an);
            achse.kanalLoeschen(0);
            CHECK(zuletzt == QStringLiteral("Delete channel"));
            REQUIRE(achse.loop().kanaele.size() == 2);
            CHECK(achse.gewaehlterKanal() == 0);  // Perc ist jetzt die erste Zeile
            CHECK(achse.gewaehlterClip() == 0);
            achse.kanalLoeschen(0);
            CHECK(achse.auswahl() == nullptr);
            CHECK(achse.loop().muster.empty());
            CHECK_FALSE(achse.grab().isNull());
        }

        SUBCASE("die Abspielmarke: Schritte werden zu Ticks, und der Zoom der Basis gilt auch hier")
        {
            achse.setSpielt(true);
            achse.setStelleSchritte(24.0);  // 16 Schritte je Takt: anderthalb Takte
            CHECK(achse.stelle() == Taktachse::kTicksJeTakt * 3 / 2);
            achse.zoomeZeit(4.0, achse.xVonTakt(4));
            CHECK_FALSE(achse.zeitGanz());
            CHECK(achse.rollXMax() > 0);
            achse.zoomeZeit(1e9, achse.xVonTakt(4));
            CHECK(achse.einheitJePixel() == doctest::Approx(Taktachse::kFeinsteTicksJePixel));
            CHECK(achse.xVonTakt(5) - achse.xVonTakt(4) == 240);
            CHECK_FALSE(achse.grab().isNull());
            achse.setzeZoomZurueck();
            CHECK(achse.zeitGanz());
        }

        SUBCASE("ohne Loop: der Hinweis, und nichts laesst sich anlegen")
        {
            achse.setLoop({}, false);
            CHECK_FALSE(achse.grab().isNull());
            CHECK_FALSE(achse.neuerClip(0, 0));
            achse.kanalHinzu();
            CHECK(aenderungen == 0);
        }
    }

    TEST_CASE("Loop Panel: New, bearbeiten mit Undo, der Stern, Save und wieder laden")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;

        ServiceContainer dienste;
        dienste.registerSingleton<ICommandBus>([](ServiceContainer&) { return std::make_unique<CommandBus>(); });
        ICommandBus* bus = dienste.tryResolve<ICommandBus>();
        REQUIRE(bus != nullptr);

        LoopPanel panel(dienste);  // nicht gezeigt: kein Fenster
        auto* datei = panel.findChild<QLabel*>(QStringLiteral("loopFile"));
        auto* clipZeile = panel.findChild<QLabel*>(QStringLiteral("loopClip"));
        auto* speichern = panel.findChild<QPushButton*>(QStringLiteral("loopSave"));
        auto* speichernUnter = panel.findChild<QPushButton*>(QStringLiteral("loopSaveAs"));
        auto* bpm = panel.findChild<QDoubleSpinBox*>(QStringLiteral("loopBpm"));
        Taktachse* achse = panel.taktachse();
        REQUIRE(datei != nullptr);
        REQUIRE(clipZeile != nullptr);
        REQUIRE(speichern != nullptr);
        REQUIRE(speichernUnter != nullptr);
        REQUIRE(bpm != nullptr);
        REQUIRE(achse != nullptr);

        // ohne Loop
        CHECK_FALSE(panel.geladen());
        CHECK_FALSE(speichern->isEnabled());
        CHECK_FALSE(speichernUnter->isEnabled());
        CHECK_FALSE(bpm->isEnabled());

        // New: ein leerer Loop mit einem Kanal, noch ohne Datei
        panel.neu();
        CHECK(panel.geladen());
        CHECK(datei->text() == QStringLiteral("untitled"));
        CHECK_FALSE(panel.ungespeichert());
        CHECK(speichern->isEnabled());  // noch nie gespeichert
        REQUIRE(panel.loop().kanaele.size() == 1);
        CHECK(panel.loop().kanaele[0].name == QStringLiteral("Channel 1"));
        CHECK(bpm->value() == doctest::Approx(120.0));

        // ein Clip ueber die Achse: das Panel fuehrt ihn ueber den CommandBus aus
        REQUIRE(achse->neuerClip(0, 0));
        CHECK(panel.loop().kanaele[0].clips.size() == 1);
        CHECK(panel.spieler().loop() == panel.loop());  // was gespielt wuerde, ist die neue Fassung
        CHECK(datei->text() == QStringLiteral("untitled *"));
        CHECK(bus->undoDescription() == "New clip");
        CHECK(clipZeile->text().startsWith(QStringLiteral("Clip: Pattern 1 · bars 1 to 4 · 1 × 4 bars")));
        CHECK(clipZeile->text().endsWith(QStringLiteral("used by 1 clips")));

        REQUIRE(achse->dupliziereAuswahl());
        CHECK(panel.loop().kanaele[0].clips.size() == 2);
        CHECK(clipZeile->text().contains(QStringLiteral("bars 5 to 8")));
        CHECK(clipZeile->text().endsWith(QStringLiteral("used by 2 clips")));

        // Undo und Redo
        REQUIRE(bus->undo());
        CHECK(panel.loop().kanaele[0].clips.size() == 1);
        CHECK(achse->loop() == panel.loop());
        REQUIRE(bus->undo());
        CHECK(panel.loop().kanaele[0].clips.empty());
        CHECK(panel.loop().muster.empty());
        CHECK(datei->text() == QStringLiteral("untitled"));  // wieder wie beim New: kein Stern
        REQUIRE(bus->redo());
        REQUIRE(bus->redo());
        CHECK(panel.loop().kanaele[0].clips.size() == 2);

        // das Tempo
        bpm->setValue(140.0);
        CHECK(panel.loop().bpm == doctest::Approx(140.0));
        CHECK(bus->undoDescription() == "Change tempo");
        REQUIRE(bus->undo());
        CHECK(panel.loop().bpm == doctest::Approx(120.0));
        CHECK(bpm->value() == doctest::Approx(120.0));
        REQUIRE(bus->redo());

        // Save: die Datei gehoert jetzt zum Panel, der Stern geht
        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());
        const QString pfad = ordner.filePath(QStringLiteral("probe.lvloop"));
        REQUIRE(panel.speichere(pfad));
        CHECK(panel.datei() == pfad);
        CHECK(datei->text() == QStringLiteral("probe.lvloop"));
        CHECK_FALSE(speichern->isEnabled());
        CHECK(speichernUnter->isEnabled());
        const Loop gespeichert = panel.loop();

        achse->kanalHinzu();
        CHECK(datei->text() == QStringLiteral("probe.lvloop *"));
        CHECK(speichern->isEnabled());
        REQUIRE(bus->undo());
        CHECK(datei->text() == QStringLiteral("probe.lvloop"));  // zurueck auf dem Stand der Datei

        // laden: dieselbe Fassung, und ein alter Schritt fuer Undo gilt nicht mehr
        const std::size_t schritte = bus->undoCount();
        REQUIRE(panel.lade(pfad));
        CHECK(panel.loop() == gespeichert);
        CHECK(achse->loop() == gespeichert);
        CHECK_FALSE(panel.ungespeichert());
        if (schritte > 0)
        {
            (void)bus->undo();  // ein Schritt aus der Ladung davor: er tut nichts
            CHECK(panel.loop() == gespeichert);
        }

        // ein neues Panel: der zuletzt gespeicherte Loop steht wieder da
        LoopPanel wieder(dienste);
        CHECK(wieder.datei() == pfad);
        CHECK(wieder.loop() == gespeichert);

        // `LUMIVIZ_LOOP_PANEL_BILD=<datei.png>` legt ein Bild des Panels mit dem Probe-Loop ab
        if (const QString bild = qEnvironmentVariable("LUMIVIZ_LOOP_PANEL_BILD"); !bild.isEmpty())
        {
            Loop gezeigt = probe();
            (void)kanalHinzu(gezeigt);
            (void)kanalSchalten(gezeigt, 2, false);
            gezeigt.kanaele[2].clips = {{QStringLiteral("A"), 0, 1}};
            wieder.setzeLoop(gezeigt);
            wieder.taktachse()->waehle(0, 0);
            wieder.resize(900, 300);
            CHECK(wieder.grab().save(bild));
        }
    }
}
