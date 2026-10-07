/**
 ****************************************************************************************
 * @file   test_ComposerMute.cpp
 * @brief  Composer: Mute fuer spontane Durchsagen — das Mute-Profil, sein
 *         Verlauf ueber der Zeit und die Profile als Dateien
 *         (Vorgabe Patrik S101; docs/ui/Composer_Konzept.md §4)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Nur das Modell: kein Ton, kein Fenster. Die Zeit reicht der Test herein.
 *
 * Rueckbau (Konzept §12): diese Datei loeschen, Zeile in Source.cmake nehmen.
 ****************************************************************************************
 */

#include <doctest.h>

#include "PappTon.hpp"
#include "composer/AppAnbindung.hpp"
#include "composer/Mute.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QSettings>
#include <QStringList>
#include <QTemporaryDir>

using namespace lumi::composer;

namespace
{

struct Wurzel
{
    explicit Wurzel(const QString& pfad) { muteprofile::setzeWurzelFuerTests(pfad); }
    ~Wurzel() { muteprofile::setzeWurzelFuerTests({}); }
    Wurzel(const Wurzel&) = delete;
    Wurzel& operator=(const Wurzel&) = delete;
};

QStringList namen()
{
    QStringList aus;
    for (const muteprofile::Eintrag& e : muteprofile::liste())
        aus << (e.mitgeliefert ? QStringLiteral("[%1]").arg(e.name) : e.name);
    return aus;
}

}  // namespace

TEST_SUITE("ComposerMute")
{
    TEST_CASE("Der Verlauf: nach dem Profil hinunter auf den Zielpegel und wieder hinauf")
    {
        MuteProfil p;
        p.ausblendeMs = 400;
        p.einblendeMs = 1000;
        p.zielPegel = 0.2;
        MuteVerlauf v;
        v.setProfil(p);

        // am Anfang: voll, nichts blendet
        CHECK_FALSE(v.stumm());
        CHECK(v.faktorBei(0) == doctest::Approx(1.0));
        CHECK_FALSE(v.blendet(0));

        // stumm: linear in 400 ms von 1 auf 0,2 — und dort bleibt es
        v.schalte(true, 10000);
        CHECK(v.stumm());
        CHECK(v.faktorBei(10000) == doctest::Approx(1.0));
        CHECK(v.faktorBei(10100) == doctest::Approx(0.8));
        CHECK(v.faktorBei(10200) == doctest::Approx(0.6));
        CHECK(v.blendet(10399));
        CHECK(v.faktorBei(10400) == doctest::Approx(0.2));
        CHECK_FALSE(v.blendet(10400));
        CHECK(v.faktorBei(99999) == doctest::Approx(0.2));

        // derselbe Zustand noch einmal: nichts beginnt neu
        v.schalte(true, 50000);
        CHECK(v.faktorBei(50000) == doctest::Approx(0.2));

        // zurueck: in 1000 ms von 0,2 auf 1
        v.schalte(false, 60000);
        CHECK_FALSE(v.stumm());
        CHECK(v.faktorBei(60000) == doctest::Approx(0.2));
        CHECK(v.faktorBei(60500) == doctest::Approx(0.6));
        CHECK(v.faktorBei(61000) == doctest::Approx(1.0));
        CHECK(v.faktorBei(70000) == doctest::Approx(1.0));
    }

    TEST_CASE("Mitten in der Blende umgeschaltet: kein Sprung, der Rest dauert anteilig")
    {
        MuteProfil p;
        p.ausblendeMs = 400;
        p.einblendeMs = 1000;
        p.zielPegel = 0.0;
        MuteVerlauf v;
        v.setProfil(p);

        v.schalte(true, 0);
        CHECK(v.faktorBei(100) == doctest::Approx(0.75));
        // nach einem Viertel des Wegs wieder zurueck: von 0,75 aus, ein Viertel der Einblendzeit
        v.schalte(false, 100);
        CHECK(v.faktorBei(100) == doctest::Approx(0.75));
        CHECK(v.faktorBei(225) == doctest::Approx(0.875));
        CHECK(v.blendet(349));
        CHECK(v.faktorBei(350) == doctest::Approx(1.0));
        CHECK_FALSE(v.blendet(350));

        // und umgekehrt: mitten im Einblenden wieder stumm
        v.schalte(true, 1000);
        v.schalte(false, 1400);  // ganz unten
        CHECK(v.faktorBei(1900) == doctest::Approx(0.5));
        v.schalte(true, 1900);  // von 0,5 hinunter: die halbe Ausblendzeit
        CHECK(v.faktorBei(1900) == doctest::Approx(0.5));
        CHECK(v.faktorBei(2000) == doctest::Approx(0.25));
        CHECK(v.faktorBei(2100) == doctest::Approx(0.0));
    }

    TEST_CASE("Die Kurvenform bestimmt den Weg; ohne Dauer springt es; ein neues Profil gilt ab dem naechsten Umschalten")
    {
        MuteProfil weich;
        weich.ausblendeMs = 1000;
        weich.einblendeMs = 1000;
        weich.zielPegel = 0.0;
        weich.ausblendeForm = einfacheBlende(Kurvenform::SKurve);
        weich.einblendeForm = einfacheBlende(Kurvenform::EaseIn);
        MuteVerlauf v;
        v.setProfil(weich);
        v.schalte(true, 0);
        // die S-Kurve: am Anfang langsamer als die Gerade, in der Mitte gleich, am Ende wieder langsamer
        CHECK(v.faktorBei(100) > 0.9);
        CHECK(v.faktorBei(500) == doctest::Approx(0.5));
        CHECK(v.faktorBei(900) < 0.1);
        // die Ausblendung liest ihre Form rueckwaerts — wie am Clip
        CHECK(v.faktorBei(250) == doctest::Approx(blendenPegel(weich.ausblendeForm, 0.25, true)));
        v.schalte(false, 5000);
        CHECK(v.faktorBei(5250) == doctest::Approx(blendenPegel(weich.einblendeForm, 0.25, false)));
        CHECK(v.faktorBei(5500) < 0.5);  // Ease in: kommt spaet

        // ein hartes Profil, mitten im Lauf gesetzt: die laufende Blende laeuft zu Ende
        MuteProfil hart;
        hart.ausblendeMs = 0;
        hart.einblendeMs = 0;
        hart.zielPegel = 0.0;
        v.setProfil(hart);
        CHECK(v.blendet(5500));
        CHECK(v.faktorBei(6000) == doctest::Approx(1.0));
        // ab dem naechsten Umschalten gilt es: sofort unten, sofort oben
        v.schalte(true, 7000);
        CHECK(v.faktorBei(7000) == doctest::Approx(0.0));
        CHECK_FALSE(v.blendet(7000));
        v.schalte(false, 7001);
        CHECK(v.faktorBei(7001) == doctest::Approx(1.0));

        // „nur leiser": Zielpegel 100 % aendert nichts
        MuteProfil nichts;
        nichts.zielPegel = 1.0;
        v.setProfil(nichts);
        v.schalte(true, 8000);
        CHECK(v.faktorBei(8100) == doctest::Approx(1.0));
        CHECK_FALSE(v.blendet(8100));
    }

    TEST_CASE("Das Profil: richten, als Datei und zurueck")
    {
        MuteProfil p;
        p.ausblendeMs = -5;
        p.einblendeMs = 2500;
        p.zielPegel = 3.0;
        p.einblendeForm = einfacheBlende(Kurvenform::Bezier, {0.2, 0.9, 0.5, 1.0});
        p.richte();
        CHECK(p.ausblendeMs == 0);
        CHECK(p.zielPegel == doctest::Approx(1.0));

        p.zielPegel = 0.35;
        p.ausblendeMs = 300;
        const QJsonObject json = profilAlsJson(p);
        CHECK(json.value(QStringLiteral("format")).toString() == QStringLiteral("lumiviz-mute"));
        CHECK(json.value(QStringLiteral("level")).toDouble() == doctest::Approx(0.35));
        MuteProfil gelesen;
        REQUIRE(profilAusJson(json, gelesen));
        CHECK(gleich(gelesen, p));
        CHECK(gelesen.einblendeMs == 2500);
        CHECK(gelesen.einblendeForm.punkte.front().form == Kurvenform::Bezier);

        // etwas anderes ist kein Profil — das Ziel bleibt, wie es war
        QJsonObject fremd = json;
        fremd.insert(QStringLiteral("format"), QStringLiteral("lumiviz-curve"));
        MuteProfil unberuehrt = p;
        CHECK_FALSE(profilAusJson(fremd, unberuehrt));
        CHECK(gleich(unberuehrt, p));
        // ohne Formen: linear
        QJsonObject knapp;
        knapp.insert(QStringLiteral("format"), QStringLiteral("lumiviz-mute"));
        knapp.insert(QStringLiteral("level"), 0.5);
        REQUIRE(profilAusJson(knapp, gelesen));
        CHECK(gleich(gelesen.ausblendeForm, einfacheBlende()));
        CHECK(gelesen.ausblendeMs == 0);
    }

    TEST_CASE("Profile speichern, listen, laden, loeschen — eigene verdraengen mitgelieferte, die geschuetzt sind")
    {
        QTemporaryDir dir;
        REQUIRE(dir.isValid());
        const Wurzel wurzel(dir.path());
        CHECK(muteprofile::liste().empty());

        MuteProfil p;
        p.zielPegel = 0.1;
        p.ausblendeMs = 250;
        QString pfad;
        REQUIRE(muteprofile::speichere(QStringLiteral("Meine Ansage"), p, &pfad));
        CHECK(pfad.endsWith(QStringLiteral("user/composer/mutes/Meine Ansage.lvmute")));
        CHECK(namen() == QStringList{QStringLiteral("Meine Ansage")});
        MuteProfil gelesen;
        REQUIRE(muteprofile::lade(pfad, gelesen));
        CHECK(gleich(gelesen, p));
        CHECK_FALSE(muteprofile::speichere(QStringLiteral("   "), p));

        // ein mitgeliefertes, von Hand in den Asset-Ordner der Test-Wurzel
        const QString asset = muteprofile::assetOrdner();
        REQUIRE(QDir().mkpath(asset));
        {
            MuteProfil hart;
            hart.ausblendeMs = 0;
            hart.einblendeMs = 0;
            QFile f(QDir(asset).filePath(QStringLiteral("meine ansage.lvmute")));
            REQUIRE(f.open(QIODevice::WriteOnly));
            f.write(QJsonDocument(profilAlsJson(hart)).toJson());
            QFile g(QDir(asset).filePath(QStringLiteral("Hart.lvmute")));
            REQUIRE(g.open(QIODevice::WriteOnly));
            g.write(QJsonDocument(profilAlsJson(hart)).toJson());
        }
        // das eigene verdraengt das gleichnamige mitgelieferte
        CHECK(namen() == QStringList{QStringLiteral("[Hart]"), QStringLiteral("Meine Ansage")});
        const muteprofile::Eintrag hart = muteprofile::finde(QStringLiteral("HART"));
        REQUIRE(hart.mitgeliefert);
        CHECK_FALSE(muteprofile::loesche(hart));
        CHECK(QFile::exists(hart.pfad));

        // unlesbar: abgelehnt
        QFile kaputt(QDir(asset).filePath(QStringLiteral("Kaputt.lvmute")));
        REQUIRE(kaputt.open(QIODevice::WriteOnly));
        kaputt.write("{ kein json");
        kaputt.close();
        CHECK_FALSE(muteprofile::lade(kaputt.fileName(), gelesen));

        // das gewaehlte Profil ueber den Neustart: der Name; fehlt es, gilt die Vorgabe
        const QString org = QCoreApplication::organizationName();
        const QString app = QCoreApplication::applicationName();
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("ComposerMute"));
        QSettings().clear();
        CHECK(muteprofile::gewaehlt().isEmpty());
        CHECK(gleich(muteprofile::gewaehltesProfil(), MuteProfil{}));
        muteprofile::setGewaehlt(QStringLiteral("Meine Ansage"));
        CHECK(gleich(muteprofile::gewaehltesProfil(), p));
        muteprofile::setGewaehlt(QStringLiteral("Gibt es nicht"));
        CHECK(gleich(muteprofile::gewaehltesProfil(), MuteProfil{}));
        muteprofile::setGewaehlt(QString());
        CHECK(muteprofile::gewaehlt().isEmpty());
        QSettings().clear();
        QCoreApplication::setOrganizationName(org);
        QCoreApplication::setApplicationName(app);

        // loeschen
        CHECK(muteprofile::loesche(muteprofile::finde(QStringLiteral("Meine Ansage"))));
        // das eigene ist weg, das mitgelieferte gleichen Namens wieder zu sehen
        CHECK_FALSE(namen().contains(QStringLiteral("Meine Ansage")));
        CHECK(namen().contains(QStringLiteral("[meine ansage]")));
    }

    TEST_CASE("Der Schalter der App: setzt den Pegel am Player nach dem Profil — und sagt, was die Visuals sehen")
    {
        struct PegelPlayer : papp::PappPlayer
        {
            float p = 1.0f;
            int gesetzt = 0;
            [[nodiscard]] float pegel() const override { return p; }
            void setPegel(float neu) override
            {
                p = neu;
                ++gesetzt;
            }
        } player;
        QTemporaryDir dir;
        REQUIRE(dir.isValid());
        const Wurzel wurzel(dir.path());
        const QString org = QCoreApplication::organizationName();
        const QString name = QCoreApplication::applicationName();
        {
            int argc = 1;
            char arg0[10] = "UnitTests";
            char* argv[2] = {arg0, nullptr};
            QCoreApplication app(argc, argv);
            QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
            QCoreApplication::setApplicationName(QStringLiteral("ComposerMute"));
            QSettings().clear();

            MuteProfil profil;
            profil.ausblendeMs = 400;
            profil.einblendeMs = 1000;
            profil.zielPegel = 0.2;
            REQUIRE(muteprofile::speichere(QStringLiteral("Ansage"), profil));
            muteprofile::setGewaehlt(QStringLiteral("Ansage"));

            CHECK(MuteSchalter::vorhanden() == nullptr);
            CHECK(muteFaktorFuerVisuals() == 1.0f);  // ohne Schalter: nichts wird angefasst
            MuteSchalter& schalter = MuteSchalter::instanz();
            CHECK(MuteSchalter::vorhanden() == &schalter);
            std::int64_t zeit = 0;
            schalter.setUhrFuerTests([&zeit] { return zeit; });
            schalter.ladeProfil();
            schalter.setPlayer(&player);
            CHECK(player.p == doctest::Approx(1.0f));
            int gemeldet = 0;
            QObject::connect(&schalter, &MuteSchalter::geschaltet, &app, [&gemeldet](bool) { ++gemeldet; });

            // stumm: der Pegel des Players geht nach dem Profil hinunter
            schalter.schalte(true);
            CHECK(schalter.stumm());
            CHECK(gemeldet == 1);
            zeit = 200;
            schalter.takt();
            CHECK(player.p == doctest::Approx(0.6f));
            zeit = 400;
            schalter.takt();
            CHECK(player.p == doctest::Approx(0.2f));
            // die Visuals sehen davon nichts (Vorgabe des Profils)
            CHECK(schalter.faktorFuerVisuals() == 1.0);
            CHECK(muteFaktorFuerVisuals() == 1.0f);
            schalter.schalte(true);  // derselbe Zustand: nichts geschieht
            CHECK(gemeldet == 1);

            // und wieder hinauf
            schalter.schalte(false);
            CHECK(gemeldet == 2);
            zeit = 900;
            schalter.takt();
            CHECK(player.p == doctest::Approx(0.6f));
            zeit = 1400;
            schalter.takt();
            CHECK(player.p == doctest::Approx(1.0f));

            // ein Profil mit „Visuals follow": Spektrum und Wellenform gehen mit —
            // die ganze Absenkung lang, auch waehrend der Ton zurueckkommt
            profil.visualsFolgen = true;
            REQUIRE(muteprofile::speichere(QStringLiteral("Ansage"), profil));
            schalter.ladeProfil();
            zeit = 5000;
            schalter.schalte(true);
            zeit = 5200;
            CHECK(schalter.faktorFuerVisuals() == doctest::Approx(0.6));
            CHECK(muteFaktorFuerVisuals() == doctest::Approx(0.6f));
            zeit = 6000;
            CHECK(schalter.faktorFuerVisuals() == doctest::Approx(0.2));
            schalter.schalte(false);
            zeit = 6500;
            CHECK(schalter.faktorFuerVisuals() == doctest::Approx(0.6));
            zeit = 7000;
            CHECK(schalter.faktorFuerVisuals() == doctest::Approx(1.0));

            // ein Player, der spaeter dazukommt, bekommt den Pegel, der gerade gilt
            schalter.schalte(true);
            zeit = 9000;
            PegelPlayer spaeter;
            schalter.setPlayer(&spaeter);
            CHECK(spaeter.p == doctest::Approx(0.2f));
            schalter.setPlayer(nullptr);
            schalter.setUhrFuerTests(nullptr);
            QSettings().clear();
        }
        // mit der QApplication geht auch der Schalter
        CHECK(MuteSchalter::vorhanden() == nullptr);
        QCoreApplication::setOrganizationName(org);
        QCoreApplication::setApplicationName(name);
    }

    TEST_CASE("Jedes mitgelieferte Mute-Profil laedt")
    {
        const QString ordner = muteprofile::assetOrdner();
        REQUIRE_MESSAGE(!ordner.isEmpty(), "asset/composer/mutes nicht gefunden");
        int mitgeliefert = 0;
        for (const muteprofile::Eintrag& e : muteprofile::liste())
        {
            if (!e.mitgeliefert) continue;
            ++mitgeliefert;
            CAPTURE(e.name.toStdString());
            MuteProfil profil;
            REQUIRE(muteprofile::lade(e.pfad, profil));
            CHECK(profil.zielPegel >= 0.0);
            CHECK(profil.zielPegel < 1.0);  // sonst taete der Knopf nichts
            CHECK(profil.ausblendeForm.wert(1.0) == doctest::Approx(1.0));
            CHECK(profil.einblendeForm.wert(1.0) == doctest::Approx(1.0));
        }
        CHECK(mitgeliefert == 3);
        // die drei, mit ihren Werten
        MuteProfil ansage;
        REQUIRE(muteprofile::lade(muteprofile::finde(QStringLiteral("Announcement")).pfad, ansage));
        CHECK(ansage.zielPegel == doctest::Approx(0.2));
        CHECK(ansage.ausblendeMs == 400);
        CHECK(ansage.einblendeMs == 1500);
        MuteProfil hart;
        REQUIRE(muteprofile::lade(muteprofile::finde(QStringLiteral("Hard mute")).pfad, hart));
        CHECK(hart.zielPegel == doctest::Approx(0.0));
        CHECK(hart.ausblendeMs == 0);
    }
}
