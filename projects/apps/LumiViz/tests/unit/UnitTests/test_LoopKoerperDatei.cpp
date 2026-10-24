/**
 ****************************************************************************************
 * @file   test_LoopKoerperDatei.cpp
 * @brief  Loop, Stufe 31c-5d: der gerechnete Koerper `.lvinstbody` und sein Waechter
 *         (docs/audio/Spielweisen_Konzept.md §4.1, Entscheide Patrik S147/S148: BM3 a, BM5 a,
 *         PL31c-5 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Qt-Anwendung.
 *
 * - Die Datei: Hin- und Rueckweg gleich (Zahlen genau), jedes Feld geschrieben, der Samen als
 *   Text auch mit 64 Bit; falsches Format, zu neue Fassung und kaputte Moden abgewiesen.
 * - Der Abdruck des Bauplans: gleich bei gleichem Bauplan, anders, sobald sich etwas aendert.
 * - Der Waechter (vom Arbeitsordner aus, die Test-Exe aus dem Projekt-Root starten): jeder
 *   mitgelieferte Koerper unter `asset/loop/instrument_bodies/` passt zu seinem Bauplan und
 *   zur Fassung der Rechnung (`kRechenFassung`) — sonst `InstrumentBodyTool` neu laufen
 *   lassen; Stegpunkte und Hoerorte wie im Bauplan, Moden aufsteigend im Fenster, der
 *   statistische Teil schliesst an, die Datei unter rund 10 MB (Kipppunkt von BM3 c).
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Koerper.hpp"
#include "loop/KoerperBauplan.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>

#include <cmath>
#include <complex>

using namespace lumi::loop;
namespace kp = lumi::koerper;

namespace
{

kp::Koerper kleinerKoerper()
{
    kp::Koerper k;
    k.titel = "Pruefkoerper";
    k.beschreibung = "von Hand";
    k.bauplan = "pruef.lvinstbodyplan";
    k.bauplanAbdruck = std::string(64, 'a');
    k.rechnung = {kp::kRechenFassung, "CDT 2.0.1", "Spectra", 3, 5000.0, 0.0, 5000.0, 1234, 12.5};
    k.stegTasten = {60, 61, 62};
    k.hoerorte = {"left", "right"};
    k.luft = {1.2, 343.0};
    k.daempfung = {1.0e-5, 0.05, 0.1};
    k.werkstoffe = {"spruce", "glass"};
    k.werkstoffVerlust = {0.0, 1.0e-3};
    for (int i = 0; i < 2; ++i)
    {
        kp::ModeImKoerper m;
        m.frequenzHz = 100.0 + 0.1 * i + 1.0 / 3.0;  // nicht in wenigen Stellen darstellbar
        m.verlustfaktor = 0.0113;
        m.werkstoffAnteil = {1.0 / 3.0, 2.0 / 3.0};
        m.amStegpunkt = {1.0 / 7.0, -2.5, 3.0e-9};
        m.abstrahlung = {{0.1, -0.2}, {1.0 / 3.0, 2.0e-12}};
        k.moden.push_back(m);
    }
    k.statistik.vonHz = 5000.0;
    k.statistik.bisHz = 20000.0;
    k.statistik.dichte = 0.238;
    k.statistik.anstieg = 2.1e-5;
    k.statistik.stegStreuung = {0.1, 0.2, 0.3};
    k.statistik.hoerBetrag = {0.0067, 0.0069};
    k.statistik.samen = 0x8000000000000005ull;  // passt nicht in eine JSON-Zahl
    k.statistik.bandVonHz = 2500.0;
    k.statistik.bandBisHz = 5000.0;
    k.statistik.bandModen = 528;
    k.statistik.werkstoffAnteil = {0.25, 0.75};
    k.statistik.werkstoffVerlust = k.werkstoffVerlust;  // die Datei traegt sie einmal, am Koerper
    return k;
}

}  // namespace

TEST_CASE("Loop Koerper 31c-5d Datei")
{
    const kp::Koerper k = kleinerKoerper();
    const QJsonObject json = alsJson(k);
    CHECK(json.value(QStringLiteral("format")).toString() == QLatin1String(kKoerperFormat));
    CHECK(json.value(QStringLiteral("version")).toInt() == kp::kKoerperFassung);
    for (const char* feld : {"title", "description", "units", "plan", "computation", "bridgePoints", "listeningPoints", "air", "damping",
                             "materials", "modes", "statistical"})
        CHECK_MESSAGE(json.contains(QLatin1String(feld)), feld);
    CHECK(json.value(QStringLiteral("statistical")).toObject().value(QStringLiteral("seed")).toString() ==
          QStringLiteral("9223372036854775813"));

    kp::Koerper zurueck;
    REQUIRE(ausJson(json, zurueck));
    CHECK(zurueck == k);

    // ein leerer Koerper geht auch hin und zurueck
    kp::Koerper leer;
    REQUIRE(ausJson(alsJson(kp::Koerper{}), leer));
    CHECK(leer == kp::Koerper{});

    bool zuNeu = true;
    kp::Koerper bleibt = kleinerKoerper();
    QJsonObject fremd = json;
    fremd[QStringLiteral("format")] = QLatin1String(kBauplanFormat);
    CHECK_FALSE(ausJson(fremd, bleibt, &zuNeu));
    CHECK_FALSE(zuNeu);
    QJsonObject neuer = json;
    neuer[QStringLiteral("version")] = kp::kKoerperFassung + 1;
    CHECK_FALSE(ausJson(neuer, bleibt, &zuNeu));
    CHECK(zuNeu);
    // eine Mode mit zu wenigen Werten am Steg
    QJsonObject kaputt = json;
    QJsonArray moden = json.value(QStringLiteral("modes")).toArray();
    QJsonObject mode = moden[0].toObject();
    mode[QStringLiteral("bridge")] = QJsonArray{1.0};
    moden[0] = mode;
    kaputt[QStringLiteral("modes")] = moden;
    CHECK_FALSE(ausJson(kaputt, bleibt));
    CHECK(bleibt == kleinerKoerper());
    // Anteile, die nicht zu den Werkstoffen passen
    QJsonObject kaputtAnteil = json;
    moden = json.value(QStringLiteral("modes")).toArray();
    mode = moden[0].toObject();
    mode[QStringLiteral("materialShares")] = QJsonArray{1.0};
    moden[0] = mode;
    kaputtAnteil[QStringLiteral("modes")] = moden;
    CHECK_FALSE(ausJson(kaputtAnteil, bleibt));

    // Fassung 1 (vor DM1 a): ohne Werkstoffe und Anteile — bleibt lesbar
    QJsonObject alt = json;
    alt[QStringLiteral("version")] = 1;
    alt.remove(QStringLiteral("materials"));
    moden = json.value(QStringLiteral("modes")).toArray();
    for (int i = 0; i < moden.size(); ++i)
    {
        QJsonObject o = moden[i].toObject();
        o.remove(QStringLiteral("materialShares"));
        moden[i] = o;
    }
    alt[QStringLiteral("modes")] = moden;
    QJsonObject altStat = alt.value(QStringLiteral("statistical")).toObject();
    altStat.remove(QStringLiteral("materialShares"));
    alt[QStringLiteral("statistical")] = altStat;
    kp::Koerper ausAlt;
    REQUIRE(ausJson(alt, ausAlt));
    CHECK(ausAlt.werkstoffe.empty());
    CHECK(ausAlt.moden.size() == 2);
    CHECK(ausAlt.moden[0].werkstoffAnteil.empty());
    CHECK(ausAlt.moden[0].verlustfaktor == k.moden[0].verlustfaktor);
    CHECK(ausAlt.statistik.werkstoffAnteil.empty());

    // ueber eine Datei
    const QString pfad = QDir::temp().filePath(QStringLiteral("lumiviz_test_koerper") + QLatin1String(kKoerperEndung));
    REQUIRE(speichereKoerper(k, pfad));
    kp::Koerper geladen;
    REQUIRE(ladeKoerper(pfad, geladen));
    CHECK(geladen == k);
    QFile::remove(pfad);
}

TEST_CASE("Loop Koerper 31c-5d Abdruck")
{
    kp::Bauplan plan;
    plan.titel = "Pruefplatte";
    plan.platte.material = "spruce";
    plan.platte.dicke = 0.008;
    plan.platte.umriss = {{0.0, 0.0}, {1.0, 0.0}, {1.0, 0.5}};
    const QString a = bauplanAbdruck(plan);
    CHECK(a.size() == 64);
    CHECK(a == bauplanAbdruck(plan));
    kp::Bauplan anders = plan;
    anders.platte.dicke = 0.009;
    CHECK(bauplanAbdruck(anders) != a);
    anders = plan;
    anders.hoerorte.push_back({"left", {0.5, 0.2}, 0.15, "von Hand"});
    CHECK(bauplanAbdruck(anders) != a);
}

TEST_CASE("Loop Koerper 31c-5d Waechter")
{
    const QDir ordner(QStringLiteral("asset/loop/instrument_bodies"));
    REQUIRE_MESSAGE(ordner.exists(), "die Test-Exe aus dem Projekt-Root starten");
    const QStringList koerper = ordner.entryList({QStringLiteral("*") + QLatin1String(kKoerperEndung)}, QDir::Files, QDir::Name);
    CHECK(koerper.contains(QStringLiteral("steinway_d") + QLatin1String(kKoerperEndung)));
    for (const QString& name : koerper)
    {
        CAPTURE(name.toStdString());
        const QString pfad = ordner.filePath(name);
        kp::Koerper k;
        REQUIRE(ladeKoerper(pfad, k));
        kp::Bauplan plan;
        REQUIRE_MESSAGE(ladeBauplan(ordner.filePath(QString::fromStdString(k.bauplan)), plan), "Bauplan ", k.bauplan, " fehlt");

        // passt der Koerper zu seinem Bauplan und zur Rechnung? Sonst neu rechnen:
        //   InstrumentBodyTool asset/loop/instrument_bodies/<name>.lvinstbodyplan --to <bis>
        CHECK_MESSAGE(k.bauplanAbdruck == bauplanAbdruck(plan).toStdString(),
                      "der Bauplan hat sich geaendert — InstrumentBodyTool neu laufen lassen");
        CHECK_MESSAGE(k.rechnung.fassung == kp::kRechenFassung, "die Rechnung hat sich geaendert (kRechenFassung) — InstrumentBodyTool neu laufen lassen");

        // Stegpunkte und Hoerorte wie im Bauplan
        REQUIRE(k.stegTasten.size() == plan.stegpunkte.size());
        for (std::size_t i = 0; i < k.stegTasten.size(); ++i) CHECK(k.stegTasten[i] == plan.stegpunkte[i].taste);
        REQUIRE(k.hoerorte.size() == plan.hoerorte.size());
        for (std::size_t i = 0; i < k.hoerorte.size(); ++i) CHECK(k.hoerorte[i] == plan.hoerorte[i].name);

        // die Werkstoffe wie im Bauplan, mit ihrem Verlustfaktor (DM1 a)
        REQUIRE(k.werkstoffe.size() == plan.materialien.size());
        REQUIRE(k.werkstoffVerlust.size() == plan.materialien.size());
        for (std::size_t w = 0; w < k.werkstoffe.size(); ++w)
        {
            CHECK(k.werkstoffe[w] == plan.materialien[w].name);
            CHECK(k.werkstoffVerlust[w] == plan.materialien[w].verlustfaktor);
        }

        // Moden aufsteigend im Fenster, je Mode Anteile je Werkstoff (Summe 1), die Daempfung daraus
        // (DM1 a); der statistische Teil schliesst an
        REQUIRE_FALSE(k.moden.empty());
        int falsch = 0;
        for (std::size_t i = 0; i < k.moden.size(); ++i)
        {
            const kp::ModeImKoerper& m = k.moden[i];
            falsch += (m.frequenzHz >= k.rechnung.vonHz && m.frequenzHz < k.rechnung.bisHz) ? 0 : 1;
            falsch += (i == 0 || m.frequenzHz >= k.moden[i - 1].frequenzHz) ? 0 : 1;
            double summe = 0.0;
            for (const double a : m.werkstoffAnteil) summe += a;
            falsch += m.werkstoffAnteil.size() == k.werkstoffe.size() && std::abs(summe - 1.0) < 1e-9 ? 0 : 1;
            const double eta = kp::verlustfaktorAusAnteilen(k.daempfung, m.frequenzHz, m.werkstoffAnteil, k.werkstoffVerlust);
            falsch += std::abs(m.verlustfaktor - eta) < 1e-12 ? 0 : 1;
        }
        CHECK(falsch == 0);
        if (k.statistik.dichte > 0.0)
        {
            CHECK(k.statistik.vonHz == k.rechnung.bisHz);
            CHECK(k.statistik.stegStreuung.size() == k.stegTasten.size());
            CHECK(k.statistik.hoerBetrag.size() == k.hoerorte.size());
        }
        const double mb = static_cast<double>(QFileInfo(pfad).size()) / 1048576.0;
        MESSAGE(name.toStdString(), ": ", k.moden.size(), " Moden bis ", k.rechnung.bisHz, " Hz, statistisch bis ", k.statistik.bisHz,
                " Hz, ", mb, " MB");
        CHECK_MESSAGE(mb < 10.0, "der Koerper waechst ueber rund 10 MB — Kipppunkt von BM3 c (binaer)");
    }
}
