/**
 * @file test_PublishPacken.cpp
 * @brief Publish, Stufe P-A: ein Fund als Paket `.lvdata` Fassung 2
 *        (docs/core-services/Publish_Konzept.md §3; Entscheide P3 a, P5 a)
 *
 * Ohne Ton, ohne Fenster; Bestand wie in `test_PublishSammler`. Der Test
 * liest das Paket als Zip wieder und prueft, dass jede Nennung einer
 * Zugehoerigen schon `user:<name>` heisst.
 */

#include "PaketBestand.hpp"

#include "core/Datensicherung.hpp"
#include "core/ZipArchiv.hpp"
#include "paket/Packen.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <map>

namespace
{

/// Das Paket als Name -> Bytes.
std::map<QString, QByteArray> liesPaket(const QString& datei)
{
    std::vector<lumi::core::ZipEintrag> eintraege;
    QString fehler;
    REQUIRE_MESSAGE(lumi::core::leseZip(datei, eintraege, &fehler), fehler.toStdString());
    std::map<QString, QByteArray> inhalt;
    for (auto& e : eintraege) inhalt[e.name] = e.daten;
    return inhalt;
}

QJsonObject json(const QByteArray& daten)
{
    return QJsonDocument::fromJson(daten).object();
}

QStringList namen(const std::map<QString, QByteArray>& inhalt)
{
    QStringList alle;
    for (const auto& [name, daten] : inhalt) alle << name;
    return alle;
}

}  // namespace

TEST_SUITE("Publish")
{
    TEST_CASE("Publish Packen: ein Loop — Fassung 2, Manifest, Dateien unter data/ und package/, Nennungen umgeschrieben")
    {
        const PublishApp app;
        const Bestand b;
        const pb::Fund fund = pb::sammle(b.loopDatei);
        REQUIRE(fund.ok);
        CHECK(pb::paketName(fund) == QStringLiteral("titan.lvloop.lvdata"));

        const QString ziel = QDir(b.tmp.path()).filePath(pb::paketName(fund));
        const pb::PackErgebnis e = pb::packe(fund, ziel, QStringLiteral("1.2.3"));
        REQUIRE_MESSAGE(e.ok, e.fehler.toStdString());
        CHECK(e.dateien == 8);  // der Loop und sieben Zugehoerige

        const auto inhalt = liesPaket(ziel);
        CHECK(namen(inhalt) == QStringList{QStringLiteral("data/contours/herz.lvcontour"),
                                           QStringLiteral("data/instruments/bass.lvinst"),
                                           QStringLiteral("data/instruments/pump_bass.lvinst"),
                                           QStringLiteral("data/kits/mein.lvkit"),
                                           QStringLiteral("data/samples/kick.wav"),
                                           QStringLiteral("data/samples/kick_2.wav"),
                                           QStringLiteral("data/samples/snare_hit.wav"),
                                           QStringLiteral("manifest.json"),
                                           QStringLiteral("package/titan.lvloop")});

        // --- das Manifest
        const QJsonObject manifest = json(inhalt.at(QStringLiteral("manifest.json")));
        CHECK(manifest.value(QStringLiteral("format")).toString() == QLatin1String(lumi::core::datensicherung::kFormat));
        CHECK(manifest.value(QStringLiteral("version")).toInt() == 2);
        CHECK(manifest.value(QStringLiteral("appVersion")).toString() == QStringLiteral("1.2.3"));
        const QJsonObject gruppen = manifest.value(QStringLiteral("groups")).toObject();
        CHECK(gruppen.value(QStringLiteral("instruments")).toInt() == 2);
        CHECK(gruppen.value(QStringLiteral("samples")).toInt() == 3);
        CHECK(gruppen.value(QStringLiteral("kits")).toInt() == 1);
        CHECK(gruppen.value(QStringLiteral("shapes")).toInt() == 1);
        const QJsonObject paket = manifest.value(QStringLiteral("package")).toObject();
        CHECK(paket.value(QStringLiteral("type")).toString() == QStringLiteral("lvloop"));
        CHECK(paket.value(QStringLiteral("main")).toString() == QStringLiteral("package/titan.lvloop"));
        CHECK(paket.value(QStringLiteral("files")).toArray().size() == 7);
        CHECK(paket.value(QStringLiteral("bundled")).toArray().size() == 4);
        const QJsonArray fehlt = paket.value(QStringLiteral("missing")).toArray();
        REQUIRE(fehlt.size() == 1);
        CHECK(fehlt.at(0).toObject().value(QStringLiteral("reference")).toString() == QStringLiteral("user:gibt_es_nicht"));
        const QJsonObject pump = paket.value(QStringLiteral("files")).toArray().at(1).toObject();
        CHECK(pump.value(QStringLiteral("name")).toString() == QStringLiteral("pump_bass"));
        CHECK(pump.value(QStringLiteral("origin")).toString() == QStringLiteral("elsewhere"));
        CHECK(pump.value(QStringLiteral("namedBy")).toArray().at(0).toString() == QStringLiteral("titan"));

        // --- der Loop nennt seine Zugehoerigen beim Namen, Mitgeliefertes und Fehlendes wie vorher
        lp::Loop loop;
        QStringList bericht;
        REQUIRE(lp::ausJson(json(inhalt.at(QStringLiteral("package/titan.lvloop"))), loop, {}, &bericht));
        const lp::Muster& drums = loop.muster.at(0);
        CHECK(drums.kit == QStringLiteral("user:mein"));
        QStringList spielt;
        for (const lp::Spur& s : drums.spuren) spielt << (s.instrument.isEmpty() ? s.klang : s.instrument);
        CHECK(spielt == QStringList{QStringLiteral("user:kick"), QStringLiteral("user:pump_bass"),
                                    QStringLiteral("lib:clap_1"), QStringLiteral("kit:hat"),
                                    QStringLiteral("user:kick_2"), QStringLiteral("user:gibt_es_nicht")});
        CHECK(loop.muster.at(1).instrument == QStringLiteral("piano"));
        CHECK(loop.muster.at(2).instrument == QStringLiteral("user:bass"));
        // kein Pfad des Absenders steht mehr darin
        CHECK_FALSE(inhalt.at(QStringLiteral("package/titan.lvloop")).contains(QDir::fromNativeSeparators(b.tmp.path()).toUtf8()));

        // --- das Instrument neben dem Loop nennt das Sample von der Platte als user:snare_hit
        lp::Instrument pumpInstrument;
        REQUIRE(lp::ausJson(json(inhalt.at(QStringLiteral("data/instruments/pump_bass.lvinst"))), pumpInstrument));
        CHECK(pumpInstrument.titel == QStringLiteral("Pump Bass"));
        CHECK(pumpInstrument.schichten.at(0).sample == QStringLiteral("user:snare_hit"));
        CHECK(pumpInstrument.schichten.at(1).form == QStringLiteral("heart"));

        lp::DrumKit kit;
        REQUIRE(lp::ausJson(json(inhalt.at(QStringLiteral("data/kits/mein.lvkit"))), kit));
        REQUIRE(kit.klaenge.size() == 2);
        CHECK(kit.klaenge[0].klang == QStringLiteral("user:kick"));
        CHECK(kit.klaenge[1].instrument == QStringLiteral("user:bass"));

        // Samples und Formen gehen Byte fuer Byte mit
        CHECK(inhalt.at(QStringLiteral("data/samples/kick.wav")) == "eigener kick");
        CHECK(inhalt.at(QStringLiteral("data/samples/kick_2.wav")) == "ein anderer kick");
        CHECK(inhalt.at(QStringLiteral("data/samples/snare_hit.wav")) == "snare");

        // --- ein Programm, das nur Fassung 1 kennt, weist das Paket ab und meldet die neuere Fassung
        const auto alt = lumi::core::datensicherung::oeffne(ziel, QDir(b.tmp.path()).filePath(QStringLiteral("anderer")));
        CHECK_FALSE(alt.ok);
        CHECK(alt.fehler.contains(QStringLiteral("1.2.3")));
    }

    TEST_CASE("Publish Packen: ein Instrument — es liegt selbst unter data/, sein Name im Paketnamen")
    {
        const PublishApp app;
        const Bestand b;
        const pb::Fund fund = pb::sammle(b.pumpDatei);
        REQUIRE(fund.ok);
        CHECK(pb::paketName(fund) == QStringLiteral("pump_bass.lvinst.lvdata"));
        CHECK(pb::hauptPfad(fund) == QStringLiteral("data/instruments/pump_bass.lvinst"));
        const QString ziel = QDir(b.tmp.path()).filePath(pb::paketName(fund));
        const pb::PackErgebnis e = pb::packe(fund, ziel, QStringLiteral("1.2.3"));
        REQUIRE(e.ok);
        CHECK(e.dateien == 2);
        const auto inhalt = liesPaket(ziel);
        CHECK(namen(inhalt) == QStringList{QStringLiteral("data/instruments/pump_bass.lvinst"),
                                           QStringLiteral("data/samples/snare_hit.wav"),
                                           QStringLiteral("manifest.json")});
        lp::Instrument pump;
        REQUIRE(lp::ausJson(json(inhalt.at(QStringLiteral("data/instruments/pump_bass.lvinst"))), pump));
        CHECK(pump.schichten.at(0).sample == QStringLiteral("user:snare_hit"));

        // ein Fund, der nicht ok ist, schreibt nichts
        const pb::Fund kaputt = pb::sammle(QDir(b.frei).filePath(QStringLiteral("gibt_es_nicht.lvinst")));
        const QString nichts = QDir(b.tmp.path()).filePath(QStringLiteral("nichts.lvdata"));
        CHECK_FALSE(pb::packe(kaputt, nichts, QStringLiteral("1.2.3")).ok);
        CHECK_FALSE(QFileInfo::exists(nichts));
    }
}
