/**
 * @file test_PublishImport.cpp
 * @brief Publish, Stufe P-A: ein Paket ansehen und einlesen
 *        (docs/core-services/Publish_Konzept.md §3; Entscheide P3 a, P5 a, P6 a)
 *
 * Ohne Ton, ohne Fenster. Der Absender ist der Bestand aus `PaketBestand.hpp`;
 * der Empfaenger hat eigene Ordner im selben Wegwerf-Ordner.
 */

#include "PaketBestand.hpp"

#include "core/Datensicherung.hpp"
#include "core/ZipArchiv.hpp"
#include "paket/Import.hpp"
#include "paket/Packen.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <functional>

namespace
{

namespace ds = lumi::core::datensicherung;

QByteArray liesBytes(const QString& pfad)
{
    QFile datei(pfad);
    return datei.open(QIODevice::ReadOnly) ? datei.readAll() : QByteArray("<fehlt>");
}

const pb::PaketDatei* findeDatei(const pb::PaketInhalt& inhalt, const QString& pfad)
{
    for (const pb::PaketDatei& d : inhalt.dateien)
        if (d.pfad == pfad) return &d;
    return nullptr;
}

/// Das Manifest eines Pakets aendern und das Paket neu schreiben.
void aendereManifest(const QString& paket, const std::function<void(QJsonObject&)>& aendere)
{
    std::vector<lumi::core::ZipEintrag> eintraege;
    REQUIRE(lumi::core::leseZip(paket, eintraege));
    for (auto& e : eintraege)
    {
        if (e.name != QStringLiteral("manifest.json")) continue;
        QJsonObject m = QJsonDocument::fromJson(e.daten).object();
        aendere(m);
        e.daten = QJsonDocument(m).toJson();
    }
    REQUIRE(lumi::core::schreibeZip(paket, eintraege));
}

}  // namespace

TEST_SUITE("Publish")
{
    TEST_CASE("Publish Import: ein Loop in einen leeren Benutzerordner — alles kommt an und klingt, wie es soll")
    {
        const PublishApp app;
        const Bestand b;
        const QString paket = packeTitan(b);
        CHECK(pb::istPaket(paket));
        const Empfaenger e(b.tmp.path());

        const pb::PaketInhalt inhalt = pb::oeffnePaket(paket);
        REQUIRE_MESSAGE(inhalt.ok, inhalt.fehler.toStdString());
        CHECK(inhalt.art == pb::Art::Loop);
        CHECK(inhalt.brauchtZiel());
        CHECK(inhalt.dateiname() == QStringLiteral("titan.lvloop"));
        CHECK(inhalt.appVersion == QStringLiteral("1.2.3"));
        CHECK(inhalt.dateien.size() == 7);
        CHECK(inhalt.vergeben() == 0);
        CHECK(inhalt.mitgeliefert.size() == 4);
        CHECK(inhalt.mitgeliefertFehltHier.empty());
        REQUIRE(inhalt.fehlt.size() == 1);
        CHECK(inhalt.fehlt.front().nennung == QStringLiteral("user:gibt_es_nicht"));
        const pb::PaketDatei* pump = findeDatei(inhalt, QStringLiteral("data/instruments/pump_bass.lvinst"));
        REQUIRE(pump != nullptr);
        CHECK_FALSE(pump->eigenBeimAbsender);
        CHECK(pump->genanntVon == QStringList{QStringLiteral("titan")});

        // ohne Ablageort fuer den Loop geht nichts
        const pb::PaketErgebnis ohneZiel = pb::importierePaket(paket, {});
        CHECK_FALSE(ohneZiel.ok);
        CHECK(QDir(e.instrumente).isEmpty());

        const QString loopZiel = QDir(e.loops).filePath(QStringLiteral("titan.lvloop"));
        const pb::PaketErgebnis ergebnis = pb::importierePaket(paket, {{}, loopZiel});
        REQUIRE_MESSAGE(ergebnis.ok, ergebnis.fehler.toStdString());
        CHECK(ergebnis.neu == 7);
        CHECK(ergebnis.umbenannt + ergebnis.ersetzt + ergebnis.gleich + ergebnis.eigeneGenommen == 0);
        CHECK(ergebnis.hauptDatei == loopZiel);

        // der Loop nennt nur noch eigene Dinge des Empfaengers — und die gibt es
        lp::Loop loop;
        REQUIRE(lp::ladeLoop(loopZiel, loop));
        CHECK(loop.muster.at(0).kit == QStringLiteral("user:mein"));
        CHECK(loop.muster.at(0).spuren.at(1).instrument == QStringLiteral("user:pump_bass"));
        CHECK(loop.muster.at(2).instrument == QStringLiteral("user:bass"));
        lp::Instrument pumpInstrument;
        REQUIRE(lp::holeInstrument(QStringLiteral("user:pump_bass"), pumpInstrument));
        CHECK(pumpInstrument.schichten.at(0).sample == QStringLiteral("user:snare_hit"));
        CHECK(liesBytes(lp::sampleDatei(QStringLiteral("user:snare_hit"))) == "snare");
        CHECK(liesBytes(lp::sampleDatei(QStringLiteral("user:kick_2"))) == "ein anderer kick");
        lp::DrumKit kit;
        REQUIRE(lp::holeKit(QStringLiteral("user:mein"), kit));
        CHECK(kit.klaenge.at(1).instrument == QStringLiteral("user:bass"));
        lumi::shape::Zeichnung herz;
        CHECK(lumi::shape::holeZeichnung(QStringLiteral("user:herz"), herz));

        // ein zweiter Import: alles ist gleich, nichts wird geschrieben
        const pb::PaketInhalt nochmal = pb::oeffnePaket(paket);
        REQUIRE(nochmal.ok);
        CHECK(nochmal.vergeben() == 0);
        const pb::PaketErgebnis zweiter = pb::importierePaket(paket, {{}, loopZiel});
        REQUIRE(zweiter.ok);
        CHECK(zweiter.gleich == 7);
        CHECK(zweiter.umbenannt == 0);
    }

    TEST_CASE("Publish Import: gleiche Namen — Keep both benennt um und schreibt mit, Use mine und Replace mine (P6 a)")
    {
        const PublishApp app;
        const Bestand b;
        const QString paket = packeTitan(b);
        const Empfaenger e(b.tmp.path());
        // beim Empfaenger: ein anderer bass, derselbe kick, eine andere snare_hit unter anderer Endung
        lp::Instrument seiner;
        seiner.titel = QStringLiteral("Empfaenger Bass");
        REQUIRE(lp::speichereInstrument(seiner, QDir(e.instrumente).filePath(QStringLiteral("bass.lvinst"))));
        lege(QDir(e.samples).filePath(QStringLiteral("kick.wav")), "eigener kick");
        lege(QDir(e.samples).filePath(QStringLiteral("snare_hit.flac")), "andere snare");

        const pb::PaketInhalt inhalt = pb::oeffnePaket(paket);
        REQUIRE(inhalt.ok);
        CHECK(inhalt.vergeben() == 2);
        CHECK(findeDatei(inhalt, QStringLiteral("data/samples/kick.wav"))->zustand == ds::Zustand::Gleich);
        const pb::PaketDatei* snare = findeDatei(inhalt, QStringLiteral("data/samples/snare_hit.wav"));
        REQUIRE(snare != nullptr);
        CHECK(snare->zustand == ds::Zustand::NameVergeben);
        CHECK(QFileInfo(snare->eigeneDatei).fileName() == QStringLiteral("snare_hit.flac"));

        const QString loopZiel = QDir(e.loops).filePath(QStringLiteral("titan.lvloop"));
        SUBCASE("Keep both (Vorgabe): bass_2 und snare_hit_2, das Paket nennt sie")
        {
            const pb::PaketErgebnis ergebnis = pb::importierePaket(paket, {{}, loopZiel});
            REQUIRE_MESSAGE(ergebnis.ok, ergebnis.fehler.toStdString());
            CHECK(ergebnis.umbenannt == 2);
            CHECK(ergebnis.gleich == 1);
            CHECK(ergebnis.neu == 4);
            CHECK(ergebnis.umbenennungen.contains(QStringLiteral("bass → bass_2")));

            lp::Instrument eigener;
            REQUIRE(lp::holeInstrument(QStringLiteral("user:bass"), eigener));
            CHECK(eigener.titel == QStringLiteral("Empfaenger Bass"));  // die eigene bleibt
            lp::Instrument mitgebracht;
            REQUIRE(lp::holeInstrument(QStringLiteral("user:bass_2"), mitgebracht));
            CHECK(mitgebracht.titel == QStringLiteral("Mein Bass"));
            CHECK(mitgebracht.schichten.at(0).sample == QStringLiteral("user:kick"));  // gleich: die eigene

            lp::Loop loop;
            REQUIRE(lp::ladeLoop(loopZiel, loop));
            CHECK(loop.muster.at(2).instrument == QStringLiteral("user:bass_2"));
            lp::DrumKit kit;
            REQUIRE(lp::holeKit(QStringLiteral("user:mein"), kit));
            CHECK(kit.klaenge.at(1).instrument == QStringLiteral("user:bass_2"));
            lp::Instrument pump;
            REQUIRE(lp::holeInstrument(QStringLiteral("user:pump_bass"), pump));
            CHECK(pump.schichten.at(0).sample == QStringLiteral("user:snare_hit_2"));
            CHECK(liesBytes(QDir(e.samples).filePath(QStringLiteral("snare_hit.flac"))) == "andere snare");
            CHECK(liesBytes(lp::sampleDatei(QStringLiteral("user:snare_hit_2"))) == "snare");
        }
        SUBCASE("Use mine fuer bass, Replace mine fuer snare_hit")
        {
            pb::PaketWahl wahl{{}, loopZiel};
            wahl.jeDatei[QStringLiteral("data/instruments/bass.lvinst")] = ds::Wahl::Meine;
            wahl.jeDatei[QStringLiteral("data/samples/snare_hit.wav")] = ds::Wahl::Importierte;
            const pb::PaketErgebnis ergebnis = pb::importierePaket(paket, wahl);
            REQUIRE(ergebnis.ok);
            CHECK(ergebnis.eigeneGenommen == 1);
            CHECK(ergebnis.ersetzt == 1);
            CHECK(ergebnis.umbenannt == 0);

            lp::Loop loop;
            REQUIRE(lp::ladeLoop(loopZiel, loop));
            CHECK(loop.muster.at(2).instrument == QStringLiteral("user:bass"));
            lp::Instrument eigener;
            REQUIRE(lp::holeInstrument(QStringLiteral("user:bass"), eigener));
            CHECK(eigener.titel == QStringLiteral("Empfaenger Bass"));
            CHECK_FALSE(QFileInfo::exists(QDir(e.samples).filePath(QStringLiteral("snare_hit.flac"))));
            CHECK(liesBytes(lp::sampleDatei(QStringLiteral("user:snare_hit"))) == "snare");
        }
    }

    TEST_CASE("Publish Import: abgewiesen — eine Datensicherung, ein verdorbener Pfad; Mitgeliefertes, das hier fehlt")
    {
        const PublishApp app;
        const Bestand b;
        const QString paket = packeTitan(b);
        const Empfaenger e(b.tmp.path());

        // eine Datensicherung (Fassung 1) ist kein Paket
        const QString sicherung = QDir(b.tmp.path()).filePath(QStringLiteral("sicherung.lvdata"));
        REQUIRE(ds::exportiere(QDir(b.tmp.path()).filePath(QStringLiteral("user")), {{QStringLiteral("samples")}, false},
                               {}, sicherung, QStringLiteral("1.2.3"))
                    .ok);
        CHECK_FALSE(pb::istPaket(sicherung));
        const pb::PaketInhalt keins = pb::oeffnePaket(sicherung);
        CHECK_FALSE(keins.ok);
        CHECK(keins.fehler.contains(QStringLiteral("backup")));

        SUBCASE("ein Pfad, der hinausfuehrt")
        {
            aendereManifest(paket, [](QJsonObject& m) {
                QJsonObject p = m.value(QStringLiteral("package")).toObject();
                QJsonArray dateien = p.value(QStringLiteral("files")).toArray();
                QJsonObject erste = dateien.at(0).toObject();
                erste.insert(QStringLiteral("path"), QStringLiteral("data/instruments/../../raus.lvinst"));
                dateien.replace(0, erste);
                p.insert(QStringLiteral("files"), dateien);
                m.insert(QStringLiteral("package"), p);
            });
            const pb::PaketInhalt inhalt = pb::oeffnePaket(paket);
            CHECK_FALSE(inhalt.ok);
            CHECK(inhalt.fehler.contains(QStringLiteral("damaged")));
            CHECK_FALSE(pb::importierePaket(paket, {{}, QDir(e.loops).filePath(QStringLiteral("t.lvloop"))}).ok);
            CHECK(QDir(e.instrumente).isEmpty());
        }
        SUBCASE("eine neuere Fassung")
        {
            aendereManifest(paket, [](QJsonObject& m) { m.insert(QStringLiteral("version"), 3); });
            const pb::PaketInhalt inhalt = pb::oeffnePaket(paket);
            CHECK_FALSE(inhalt.ok);
            CHECK(inhalt.fehler.contains(QStringLiteral("newer")));
        }
        SUBCASE("Mitgeliefertes, das es hier nicht gibt, wird genannt")
        {
            aendereManifest(paket, [](QJsonObject& m) {
                QJsonObject p = m.value(QStringLiteral("package")).toObject();
                QJsonArray mit = p.value(QStringLiteral("bundled")).toArray();
                mit.append(QJsonObject{{QStringLiteral("type"), QStringLiteral("sample")},
                                       {QStringLiteral("reference"), QStringLiteral("lib:gibt_es_hier_nicht")}});
                mit.append(QJsonObject{{QStringLiteral("type"), QStringLiteral("lvinst")},
                                       {QStringLiteral("reference"), QStringLiteral("gibt_es_hier_nicht")}});
                p.insert(QStringLiteral("bundled"), mit);
                m.insert(QStringLiteral("package"), p);
            });
            const pb::PaketInhalt inhalt = pb::oeffnePaket(paket);
            REQUIRE(inhalt.ok);
            CHECK(inhalt.mitgeliefertFehltHier ==
                  std::vector<pb::Nennung>{{pb::Art::Sample, QStringLiteral("lib:gibt_es_hier_nicht")},
                                           {pb::Art::Instrument, QStringLiteral("gibt_es_hier_nicht")}});
        }
    }
}
