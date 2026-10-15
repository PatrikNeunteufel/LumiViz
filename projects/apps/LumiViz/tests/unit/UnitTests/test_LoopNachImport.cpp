/**
 * @file test_LoopNachImport.cpp
 * @brief Nach einem Import lesen offene Listen neu (S125, Offene_Punkte §6 Loop):
 *        das Fenster Kits, das Panel Loop auf `UserDataImportedEvent` — ohne Ton,
 *        ohne gezeigtes Fenster, eigene Kits in einem Wegwerf-Ordner, eigene
 *        Einstellungen (`LumiVizTest` / `LoopNachImport`).
 */

#include <doctest.h>

#include "loop/DrumKit.hpp"
#include "loop/Instrument.hpp"
#include "loop/InstrumentPalette.hpp"  // KitFenster haelt sie in einem QPointer
#include "loop/KitFenster.hpp"
#include "loop/LoopPanel.hpp"
#include "services/EventBus.hpp"
#include "services/IEventBus.hpp"
#include "services/ServiceContainer.hpp"
#include "services/events/UIEvents.hpp"
#include "shape/Kontur.hpp"

#include <QApplication>
#include <QDir>
#include <QSettings>
#include <QTemporaryDir>

using namespace lumi::loop;

namespace
{

struct EigeneEinstellungen
{
    QString org = QCoreApplication::organizationName();
    QString name = QCoreApplication::applicationName();
    EigeneEinstellungen()
    {
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("LoopNachImport"));
        QSettings().clear();
    }
    ~EigeneEinstellungen()
    {
        QSettings().clear();
        QCoreApplication::setOrganizationName(org);
        QCoreApplication::setApplicationName(name);
    }
};

struct KitOrdner
{
    QTemporaryDir tmp;
    QString ordner;
    KitOrdner()
    {
        REQUIRE(tmp.isValid());
        ordner = QDir(tmp.path()).filePath(QStringLiteral("kits"));
        setzeEigenenKitOrdnerFuerTests(ordner);
    }
    ~KitOrdner() { setzeEigenenKitOrdnerFuerTests(QString()); }
};

}  // namespace

TEST_SUITE("Loop nach Import")
{
    TEST_CASE("Loop nach Import: das Fenster Kits liest die Liste neu, das offene Kit und seine Aenderungen bleiben")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;
        KitOrdner kits;

        KitFenster fenster;  // nicht gezeigt
        fenster.oeffne(QStringLiteral("808"));
        const QString eigenes = fenster.dupliziere();
        REQUIRE_FALSE(eigenes.isEmpty());
        REQUIRE(fenster.setzeLaut(0, 0.25));
        REQUIRE(fenster.geaendert());

        // ein Import legt ein Kit in den Ordner — die Liste kennt es noch nicht
        DrumKit fremd;
        fremd.titel = QStringLiteral("Aus dem Import");
        REQUIRE(QDir().mkpath(kits.ordner));
        REQUIRE(speichereKit(eigeneKitDatei(QStringLiteral("aus_dem_import")), fremd));
        CHECK_FALSE(fenster.listenZeilen().contains(QStringLiteral("user:aus_dem_import")));

        fenster.liesListeNeu();
        CHECK(fenster.listenZeilen().contains(QStringLiteral("user:aus_dem_import")));
        CHECK(fenster.nennung() == eigenes);  // das offene Kit bleibt offen
        CHECK(fenster.geaendert());           // mit seiner ungesicherten Aenderung
    }

    TEST_CASE("Loop nach Import: das Panel Loop vergisst auf UserDataImportedEvent, was es gerechnet hat")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;

        ServiceContainer dienste;
        dienste.registerSingleton<IEventBus>([](ServiceContainer&) { return std::make_unique<EventBus>(); });
        IEventBus* bus = dienste.tryResolve<IEventBus>();
        REQUIRE(bus != nullptr);
        LoopPanel panel(dienste);  // nicht gezeigt

        const std::uint64_t toene = instrumentFassung();
        const std::uint64_t formen = lumi::shape::zeichnungFassung();
        bus->publish(UserDataImportedEvent{});
        CHECK(instrumentFassung() > toene);              // gerechnete Toene gelten nicht mehr
        CHECK(lumi::shape::zeichnungFassung() > formen);  // gelesene Formen auch nicht
    }
}
