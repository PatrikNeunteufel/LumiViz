/**
 * @file test_PublishSammler.cpp
 * @brief Publish, Stufe P-A: was ein Ding nennt — der Sammler
 *        (docs/core-services/Publish_Konzept.md §2a, §3; Entscheide P1 a, P3 a)
 *
 * Ohne Ton, ohne Fenster. Die eigenen Instrumente, Samples, Kits und Formen
 * liegen in Wegwerf-Ordnern; das Mitgelieferte liest der Test vom Arbeitsordner
 * aus — die Test-Exe aus dem Projekt-Root starten.
 */

#include "PaketBestand.hpp"

TEST_SUITE("Publish")
{
    TEST_CASE("Publish Sammler: die Art einer Datei")
    {
        CHECK(pb::artVonDatei(QStringLiteral("a/b.lvinst")) == pb::Art::Instrument);
        CHECK(pb::artVonDatei(QStringLiteral("a/b.LVKIT")) == pb::Art::Kit);
        CHECK(pb::artVonDatei(QStringLiteral("b.lvloop")) == pb::Art::Loop);
        CHECK(pb::artVonDatei(QStringLiteral("b.lvcontour")) == pb::Art::Kontur);
        CHECK(pb::artVonDatei(QStringLiteral("b.flac")) == pb::Art::Sample);
        CHECK(pb::artVonDatei(QStringLiteral("b.wav")) == pb::Art::Sample);
        CHECK_FALSE(pb::artVonDatei(QStringLiteral("b.lvfx")).has_value());  // Stufe P-B
        CHECK_FALSE(pb::artVonDatei(QStringLiteral("b.txt")).has_value());
        CHECK(pb::artText(pb::Art::Instrument) == QStringLiteral("lvinst"));
        CHECK(pb::artText(pb::Art::Sample) == QStringLiteral("sample"));
    }

    TEST_CASE("Publish Sammler: ein Loop — Eigenes, freie Orte, Mitgeliefertes, Fehlendes, jede Datei einmal")
    {
        const PublishApp app;
        const Bestand b;
        const pb::Fund fund = pb::sammle(b.loopDatei);
        REQUIRE_MESSAGE(fund.ok, fund.fehler.toStdString());
        CHECK(fund.art == pb::Art::Loop);
        CHECK(fund.titel == QStringLiteral("titan"));
        CHECK(fund.hauptName.isEmpty());  // ein Loop kommt dorthin, wo der Empfaenger ihn ablegt

        QStringList gefunden;
        for (const pb::Zugehoerige& z : fund.zugehoerige) gefunden << pb::artText(z.art) + QLatin1Char(':') + z.name;
        // nach Art, darin nach Name; bass dreimal genannt, kick viermal — je einmal da
        CHECK(gefunden == QStringList{QStringLiteral("lvinst:bass"), QStringLiteral("lvinst:pump_bass"),
                                      QStringLiteral("lvkit:mein"), QStringLiteral("lvcontour:herz"),
                                      QStringLiteral("sample:kick"), QStringLiteral("sample:kick_2"),
                                      QStringLiteral("sample:snare_hit")});

        const pb::Zugehoerige* kick = fund.finde(pb::Art::Sample, QStringLiteral("kick"));
        REQUIRE(kick != nullptr);
        CHECK(kick->herkunft == pb::Herkunft::Eigen);  // die eigene behaelt ihren Namen
        CHECK(kick->nennungen == QStringList{QStringLiteral("user:kick")});
        CHECK(kick->genanntVon.contains(QStringLiteral("titan")));
        CHECK(kick->genanntVon.contains(QStringLiteral("Mein Bass")));
        CHECK(kick->genanntVon.contains(QStringLiteral("Mein Kit")));

        const pb::Zugehoerige* kick2 = fund.finde(pb::Art::Sample, QStringLiteral("kick_2"));
        REQUIRE(kick2 != nullptr);
        CHECK(kick2->herkunft == pb::Herkunft::FreierOrt);  // auf der Platte, gleicher Dateiname
        CHECK(QFileInfo(kick2->datei) == QFileInfo(b.freiKick));

        const pb::Zugehoerige* pump = fund.finde(pb::Art::Instrument, QStringLiteral("pump_bass"));
        REQUIRE(pump != nullptr);
        CHECK(pump->herkunft == pb::Herkunft::FreierOrt);
        CHECK(pump->genanntVon == QStringList{QStringLiteral("titan")});

        // was das Instrument neben dem Loop nennt, kommt mit: das Sample von der Platte
        const pb::Zugehoerige* snare = fund.finde(pb::Art::Sample, QStringLiteral("snare_hit"));
        REQUIRE(snare != nullptr);
        CHECK(snare->genanntVon == QStringList{QStringLiteral("Pump Bass")});
        CHECK(QFileInfo(snare->nennungen.value(0)) == QFileInfo(b.snareDatei));

        const pb::Zugehoerige* herz = fund.finde(pb::Art::Kontur, QStringLiteral("herz"));
        REQUIRE(herz != nullptr);
        CHECK(herz->genanntVon == QStringList{QStringLiteral("Mein Bass")});

        // Mitgeliefertes nur beim Namen; kit:snare spielt nicht (das Instrument spielt) und fehlt hier
        CHECK(fund.mitgeliefert == std::vector<pb::Nennung>{{pb::Art::Kontur, QStringLiteral("heart")},
                                                            {pb::Art::Sample, QStringLiteral("kit:hat_closed")},
                                                            {pb::Art::Sample, QStringLiteral("lib:clap_707")},
                                                            {pb::Art::Instrument, QStringLiteral("piano")}});

        REQUIRE(fund.fehlt.size() == 1);
        CHECK(fund.fehlt.front() == pb::Fehlend{pb::Art::Sample, QStringLiteral("user:gibt_es_nicht"),
                                                QStringLiteral("titan")});
    }

    TEST_CASE("Publish Sammler: ein Instrument — sein Name, was es nennt; als Pfad im eigenen Ordner zaehlt eigen")
    {
        const PublishApp app;
        const Bestand b;

        const pb::Fund pump = pb::sammle(b.pumpDatei);
        REQUIRE(pump.ok);
        CHECK(pump.art == pb::Art::Instrument);
        CHECK(pump.titel == QStringLiteral("Pump Bass"));
        CHECK(pump.hauptName == QStringLiteral("pump_bass"));
        REQUIRE(pump.zugehoerige.size() == 1);
        CHECK(pump.zugehoerige.front().name == QStringLiteral("snare_hit"));
        CHECK(pump.mitgeliefert == std::vector<pb::Nennung>{{pb::Art::Kontur, QStringLiteral("heart")}});

        // das eigene Instrument, als Datei gewaehlt: es heisst weiter bass
        const pb::Fund bass = pb::sammle(QDir(b.instrumente).filePath(QStringLiteral("bass.lvinst")));
        REQUIRE(bass.ok);
        CHECK(bass.hauptName == QStringLiteral("bass"));
        REQUIRE(bass.zugehoerige.size() == 2);
        CHECK(bass.zugehoerige[0].art == pb::Art::Kontur);
        CHECK(bass.zugehoerige[1].art == pb::Art::Sample);

        // ein eigenes Sample, als Pfad genannt, zaehlt als eigen und behaelt seinen Namen
        lp::Instrument perPfad;
        perPfad.titel = QStringLiteral("Per Pfad");
        perPfad.schichten = {sampleSchicht(QDir(b.samples).filePath(QStringLiteral("kick.wav")))};
        const QString datei = QDir(b.frei).filePath(QStringLiteral("per_pfad.lvinst"));
        REQUIRE(lp::speichereInstrument(perPfad, datei));
        const pb::Fund fund = pb::sammle(datei);
        REQUIRE(fund.zugehoerige.size() == 1);
        CHECK(fund.zugehoerige.front().herkunft == pb::Herkunft::Eigen);
        CHECK(fund.zugehoerige.front().name == QStringLiteral("kick"));
    }

    TEST_CASE("Publish Sammler: ein Kit, eine Form, ein Sample; was sich nicht veroeffentlichen laesst")
    {
        const PublishApp app;
        const Bestand b;

        const pb::Fund kit = pb::sammle(QDir(b.kits).filePath(QStringLiteral("mein.lvkit")));
        REQUIRE(kit.ok);
        CHECK(kit.titel == QStringLiteral("Mein Kit"));
        CHECK(kit.hauptName == QStringLiteral("mein"));
        QStringList namen;
        for (const pb::Zugehoerige& z : kit.zugehoerige) namen << z.name;
        CHECK(namen == QStringList{QStringLiteral("bass"), QStringLiteral("herz"), QStringLiteral("kick")});

        const pb::Fund herz = pb::sammle(QDir(b.konturen).filePath(QStringLiteral("herz.lvcontour")));
        REQUIRE(herz.ok);
        CHECK(herz.titel == QStringLiteral("Herz"));
        CHECK(herz.zugehoerige.empty());

        const pb::Fund snare = pb::sammle(b.snareDatei);
        REQUIRE(snare.ok);
        CHECK(snare.art == pb::Art::Sample);
        CHECK(snare.hauptName == QStringLiteral("snare_hit"));

        const pb::Fund fehlt = pb::sammle(QDir(b.frei).filePath(QStringLiteral("gibt_es_nicht.lvloop")));
        CHECK_FALSE(fehlt.ok);
        CHECK(fehlt.fehler.contains(QStringLiteral("not found")));

        const pb::Fund fremd = pb::sammle(QDir(b.frei).filePath(QStringLiteral("notiz.txt")));
        CHECK_FALSE(fremd.ok);
        CHECK_FALSE(fremd.fehler.isEmpty());
    }
}
