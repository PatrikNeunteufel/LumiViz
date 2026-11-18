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
 *   statistische Teil schliesst an, die Datei unter rund 10 MB (Kipppunkt von BM3 c). Mit Hohlraum
 *   (F-K1 a) traegt jede Mode ihren Anteil der Luft; die Daempfung ist die der Platte mal ihrem
 *   Anteil plus Anteil der Luft / Guete, die Anteile der Luft summieren sich zu hoechstens 1.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Koerper.hpp"
#include "loop/KoerperBauplan.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
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
        m.luftAnteil = i == 0 ? 2.0 / 3.0 : 0.0;
        m.werkstoffAnteil = {1.0 / 3.0, 2.0 / 3.0};
        m.amStegpunkt = {1.0 / 7.0, -2.5, 3.0e-9};
        m.abstrahlung = {{0.1, -0.2}, {1.0 / 3.0, 2.0e-12}};
        m.stegWinkelGrad = i == 0 ? -12.0 - 1.0 / 3.0 : 90.0;  // KQ35-2 a
        k.moden.push_back(m);
    }
    k.statistik.vonHz = 5000.0;
    k.statistik.bisHz = 20000.0;
    k.statistik.dichte = 0.238;
    k.statistik.anstieg = 2.1e-5;
    k.statistik.stegStreuung = {0.1, 0.2, 0.3};
    k.statistik.hoerBetrag = {0.0067, 0.0069};
    k.statistik.stegWinkelGrad = 12.0 + 2.0 / 3.0;  // KQ35-2 a
    k.statistik.parallelAnteil = 0.4 + 1.0 / 300.0;  // KQ35-1 a
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
    for (const char* feld : {"title", "description", "units", "origin", "plan", "measurement", "computation", "bridgePoints",
                             "listeningPoints", "air", "damping", "materials", "modes", "statistical"})
        CHECK_MESSAGE(json.contains(QLatin1String(feld)), feld);
    // KQ25 a (Fassung 4): die Herkunft steht immer in der Datei; ein gerechneter Koerper nennt `plan`
    CHECK(json.value(QStringLiteral("origin")).toString() == QStringLiteral("plan"));
    {
        kp::Koerper gemessen = k;
        gemessen.herkunft = kp::Herkunft::Messung;
        gemessen.bauplan.clear();
        gemessen.bauplanAbdruck.clear();
        gemessen.messung = "pruef.lvinstbodyfit";
        gemessen.messungAbdruck = std::string(64, 'b');
        gemessen.fitFassung = kp::kFitFassung;
        const QJsonObject mj = alsJson(gemessen);
        CHECK(mj.value(QStringLiteral("origin")).toString() == QStringLiteral("measurement"));
        CHECK(mj.value(QStringLiteral("measurement")).toObject().value(QStringLiteral("fitVersion")).toInt() == kp::kFitFassung);
        CHECK(mj.value(QStringLiteral("units")).toString().contains(QStringLiteral("measured")));
        kp::Koerper mz;
        REQUIRE(ausJson(mj, mz));
        CHECK(mz == gemessen);
        CHECK(mz.herkunft == kp::Herkunft::Messung);
        // eine unbekannte Herkunft macht die Datei unlesbar
        QJsonObject fremdeHerkunft = mj;
        fremdeHerkunft[QStringLiteral("origin")] = QStringLiteral("guess");
        CHECK_FALSE(ausJson(fremdeHerkunft, mz));
        // Fassung 3 (vor KQ25 a): ohne `origin` und `measurement` — ein Koerper aus einem Bauplan
        QJsonObject drei = json;
        drei[QStringLiteral("version")] = 3;
        drei.remove(QStringLiteral("origin"));
        drei.remove(QStringLiteral("measurement"));
        kp::Koerper ausDrei;
        REQUIRE(ausJson(drei, ausDrei));
        CHECK(ausDrei == k);
        // der Abdruck einer Beschreibung haengt nicht an den Zeilenenden
        CHECK(messungAbdruck(QByteArray("{\n \"a\": 1\n}\n")) == messungAbdruck(QByteArray("{\r\n \"a\": 1\r\n}\r\n")));
        CHECK(messungAbdruck(QByteArray("{\n \"a\": 1\n}\n")) != messungAbdruck(QByteArray("{\n \"a\": 2\n}\n")));
    }
    CHECK(json.value(QStringLiteral("statistical")).toObject().value(QStringLiteral("seed")).toString() ==
          QStringLiteral("9223372036854775813"));

    kp::Koerper zurueck;
    REQUIRE(ausJson(json, zurueck));
    CHECK(zurueck == k);

    // KQ35-2 a (Fassung 5): die Richtung am Steg steht immer in der Datei, je Mode und im statistischen Teil
    {
        const QJsonObject stJson = json.value(QStringLiteral("statistical")).toObject();
        CHECK(stJson.contains(QStringLiteral("bridgeAngle")));
        CHECK(stJson.contains(QStringLiteral("parallelFactor")));
        QJsonArray ohne;
        for (const QJsonValue& e : json.value(QStringLiteral("modes")).toArray())
        {
            QJsonObject o = e.toObject();
            CHECK(o.contains(QStringLiteral("bridgeAngle")));
            o.remove(QStringLiteral("bridgeAngle"));
            ohne.append(o);
        }
        // Fassung 4: ohne die Felder — jede Mode senkrecht, keine Moden parallel zur Decke
        QJsonObject vier = json;
        vier[QStringLiteral("version")] = 4;
        vier[QStringLiteral("modes")] = ohne;
        QJsonObject stOhne = stJson;
        stOhne.remove(QStringLiteral("bridgeAngle"));
        stOhne.remove(QStringLiteral("parallelFactor"));
        vier[QStringLiteral("statistical")] = stOhne;
        kp::Koerper ausVier;
        REQUIRE(ausJson(vier, ausVier));
        kp::Koerper senkrecht = k;
        for (kp::ModeImKoerper& m : senkrecht.moden) m.stegWinkelGrad = 0.0;
        senkrecht.statistik.stegWinkelGrad = 0.0;
        senkrecht.statistik.parallelAnteil = 0.0;
        CHECK(ausVier == senkrecht);
        // ein Winkel ueber 90 Grad macht die Datei unlesbar
        QJsonArray schief = json.value(QStringLiteral("modes")).toArray();
        QJsonObject erste = schief[0].toObject();
        erste[QStringLiteral("bridgeAngle")] = 91.0;
        schief[0] = erste;
        QJsonObject kaputterWinkel = json;
        kaputterWinkel[QStringLiteral("modes")] = schief;
        kp::Koerper nicht;
        CHECK_FALSE(ausJson(kaputterWinkel, nicht));
    }

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

    // Fassung 2 (vor F-K1 a): ohne den Anteil der Luft — bleibt lesbar, der Anteil ist 0
    for (const QJsonValue& e : json.value(QStringLiteral("modes")).toArray()) CHECK(e.toObject().contains(QStringLiteral("airShare")));
    QJsonObject zwei = json;
    zwei[QStringLiteral("version")] = 2;
    moden = json.value(QStringLiteral("modes")).toArray();
    for (int i = 0; i < moden.size(); ++i)
    {
        QJsonObject o = moden[i].toObject();
        o.remove(QStringLiteral("airShare"));
        moden[i] = o;
    }
    zwei[QStringLiteral("modes")] = moden;
    kp::Koerper ausZwei;
    REQUIRE(ausJson(zwei, ausZwei));
    CHECK(ausZwei.moden[0].luftAnteil == 0.0);
    CHECK(ausZwei.moden[0].verlustfaktor == k.moden[0].verlustfaktor);

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
    CHECK(koerper.contains(QStringLiteral("oud") + QLatin1String(kKoerperEndung)));
    CHECK(koerper.contains(QStringLiteral("lute") + QLatin1String(kKoerperEndung)));
    CHECK(koerper.contains(QStringLiteral("harpsichord") + QLatin1String(kKoerperEndung)));
    CHECK(koerper.contains(QStringLiteral("concert_harp") + QLatin1String(kKoerperEndung)));
    CHECK(koerper.contains(QStringLiteral("classical_guitar") + QLatin1String(kKoerperEndung)));  // gemessen (KQ25 a)
    for (const QString& name : koerper)
    {
        CAPTURE(name.toStdString());
        const QString pfad = ordner.filePath(name);
        kp::Koerper k;
        REQUIRE(ladeKoerper(pfad, k));
        if (k.herkunft == kp::Herkunft::Messung)
        {
            // KQ25 a: ein gemessener Koerper haengt an seiner Beschreibung `.lvinstbodyfit` und an der Fassung des
            // Fits. Sonst neu schreiben: python asset/loop/instrument_bodies/make_gitarre.py <Messdatei>
            QFile fitDatei(ordner.filePath(QString::fromStdString(k.messung)));
            REQUIRE_MESSAGE(fitDatei.open(QIODevice::ReadOnly), "Beschreibung ", k.messung, " fehlt");
            const QByteArray inhalt = fitDatei.readAll();
            CHECK_MESSAGE(k.messungAbdruck == messungAbdruck(inhalt).toStdString(),
                          "die Beschreibung hat sich geaendert — make_gitarre.py neu laufen lassen");
            CHECK_MESSAGE(k.fitFassung == kp::kFitFassung, "der Fit hat sich geaendert (kFitFassung) — make_gitarre.py neu laufen lassen");
            const QJsonObject fit = QJsonDocument::fromJson(inhalt).object();
            CHECK(fit.value(QStringLiteral("format")).toString() == QLatin1String(kFitFormat));
            CHECK(fit.value(QStringLiteral("fitVersion")).toInt() == kp::kFitFassung);
            CHECK_FALSE(fit.value(QStringLiteral("source")).toString().isEmpty());
            const QJsonArray punkte = fit.value(QStringLiteral("bridgePoints")).toArray();
            REQUIRE(static_cast<std::size_t>(punkte.size()) == k.stegTasten.size());
            for (int i = 0; i < punkte.size(); ++i)
            {
                CHECK(punkte[i].toObject().value(QStringLiteral("key")).toInt() == k.stegTasten[static_cast<std::size_t>(i)]);
                CHECK_FALSE(punkte[i].toObject().value(QStringLiteral("source")).toString().isEmpty());
            }
            const QJsonArray orte = fit.value(QStringLiteral("listeningPoints")).toArray();
            REQUIRE(static_cast<std::size_t>(orte.size()) == k.hoerorte.size());
            for (int i = 0; i < orte.size(); ++i)
                CHECK(orte[i].toObject().value(QStringLiteral("name")).toString().toStdString() == k.hoerorte[static_cast<std::size_t>(i)]);
            // kein Bauplan, keine Werkstoffe: die Daempfung ist je Mode gemessen
            CHECK(k.bauplan.empty());
            CHECK(k.werkstoffe.empty());
            REQUIRE_FALSE(k.moden.empty());
            int falsch = 0;
            for (std::size_t i = 0; i < k.moden.size(); ++i)
            {
                const kp::ModeImKoerper& m = k.moden[i];
                falsch += (m.frequenzHz >= k.rechnung.vonHz && m.frequenzHz < k.rechnung.bisHz) ? 0 : 1;
                falsch += (i == 0 || m.frequenzHz >= k.moden[i - 1].frequenzHz) ? 0 : 1;
                falsch += (m.verlustfaktor > 0.0 && m.verlustfaktor < 0.5) ? 0 : 1;
                falsch += (m.werkstoffAnteil.empty() && m.luftAnteil == 0.0) ? 0 : 1;
                // KQ35-1 a: eine Mode nur parallel zur Decke strahlt nicht ab
                if (std::abs(m.stegWinkelGrad) == 90.0)
                    for (const std::complex<double>& g : m.abstrahlung) falsch += g == std::complex<double>{0.0, 0.0} ? 0 : 1;
            }
            CHECK(falsch == 0);
            // KQ26 a: ein statistischer Teil schliesst an, seine Groessen stammen aus der Messung. Die Beschreibung
            // nennt die Moden, mit denen das Skript den Pegel gestellt hat — es muessen die von `ziehe` sein.
            if (k.statistik.dichte > 0.0)
            {
                CHECK(k.statistik.vonHz == k.rechnung.bisHz);
                CHECK(k.statistik.bisHz > k.statistik.vonHz);
                CHECK(k.statistik.stegStreuung.size() == k.stegTasten.size());
                CHECK(k.statistik.hoerBetrag.size() == k.hoerorte.size());
                CHECK(k.statistik.werkstoffAnteil.empty());
                CHECK(k.statistik.daempfung.verlustfaktor(k.statistik.vonHz) > 0.0);
                const std::vector<kp::ModeImKoerper> gezogen = kp::ziehe(k.statistik);
                const QJsonObject st = fit.value(QStringLiteral("check")).toObject().value(QStringLiteral("statistical")).toObject();
                REQUIRE(static_cast<int>(gezogen.size()) == st.value(QStringLiteral("drawnModes")).toInt());
                const QJsonArray erste = st.value(QStringLiteral("drawnFirstHz")).toArray();
                REQUIRE(static_cast<std::size_t>(erste.size()) <= gezogen.size());
                for (int i = 0; i < erste.size(); ++i)
                    CHECK(gezogen[static_cast<std::size_t>(i)].frequenzHz == doctest::Approx(erste[i].toDouble()).epsilon(1e-9));
                CHECK(gezogen.back().frequenzHz == doctest::Approx(st.value(QStringLiteral("drawnLastHz")).toDouble()).epsilon(1e-9));
                int daneben = 0;
                for (const kp::ModeImKoerper& m : gezogen) daneben += (m.verlustfaktor > 0.0 && m.verlustfaktor < 0.5) ? 0 : 1;
                CHECK(daneben == 0);
            }
            MESSAGE(name.toStdString(), ": ", k.moden.size(), " gemessene Moden bis ", k.rechnung.bisHz, " Hz, statistisch bis ",
                    k.statistik.bisHz, " Hz");
            continue;
        }
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
        double luftSumme = 0.0;
        for (std::size_t i = 0; i < k.moden.size(); ++i)
        {
            const kp::ModeImKoerper& m = k.moden[i];
            falsch += (m.frequenzHz >= k.rechnung.vonHz && m.frequenzHz < k.rechnung.bisHz) ? 0 : 1;
            falsch += (i == 0 || m.frequenzHz >= k.moden[i - 1].frequenzHz) ? 0 : 1;
            double summe = 0.0;
            for (const double a : m.werkstoffAnteil) summe += a;
            falsch += m.werkstoffAnteil.size() == k.werkstoffe.size() && std::abs(summe - 1.0) < 1e-9 ? 0 : 1;
            double eta = kp::verlustfaktorAusAnteilen(k.daempfung, m.frequenzHz, m.werkstoffAnteil, k.werkstoffVerlust);
            // mit Hohlraum (F-K1 a): die Platte mal ihrem Anteil plus Anteil der Luft / Guete
            falsch += m.luftAnteil >= 0.0 && m.luftAnteil <= 1.0 && (plan.hohlraum.an() || m.luftAnteil == 0.0) ? 0 : 1;
            if (plan.hohlraum.an()) eta = (1.0 - m.luftAnteil) * eta + m.luftAnteil / plan.hohlraum.guete;
            luftSumme += m.luftAnteil;
            falsch += std::abs(m.verlustfaktor - eta) < 1e-12 ? 0 : 1;
        }
        CHECK(falsch == 0);
        // die Luft in den Loechern ist ein Freiheitsgrad: ueber alle Moden hoechstens 1, im Fenster fast alles
        if (plan.hohlraum.an())
        {
            CHECK(luftSumme <= 1.0 + 1e-9);
            CHECK(luftSumme > 0.9);
        }
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
