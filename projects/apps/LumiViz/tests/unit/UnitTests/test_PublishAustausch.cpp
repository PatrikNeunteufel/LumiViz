/**
 ****************************************************************************************
 * @file   test_PublishAustausch.cpp
 * @brief  Publish und Datensicherung, Loop Stufe 33e: Muster, Clips und Spuren als
 *         Dateien (`.lvpattern`, `.lvclip`, `.lvtrack`; Entscheid PE1-1 a, S167)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Der Bestand aus `PaketBestand.hpp` (eigene Dinge, Dateien an freiem Ort, Mitgeliefertes);
 * die Ordner `patterns` und `tracks` von Absender und Empfaenger in Wegwerf-Ordnern
 * (`setzeAustauschBasisFuerTests`).
 ****************************************************************************************
 */

#include "PaketBestand.hpp"

#include "loop/Austausch.hpp"
#include "paket/Import.hpp"
#include "paket/Packen.hpp"

#include <optional>

namespace
{

/// Der Ordner `patterns`/`tracks` unter `basis`, solange er lebt.
struct AustauschBasis
{
    explicit AustauschBasis(const QString& basis) { lp::setzeAustauschBasisFuerTests(basis); }
    ~AustauschBasis() { lp::setzeAustauschBasisFuerTests({}); }
};

}  // namespace

TEST_SUITE("Publish")
{
    TEST_CASE("Publish Austausch 33e: die Arten — Endung, Text, Bereich")
    {
        CHECK(pb::artVonDatei(QStringLiteral("a.lvpattern")) == pb::Art::Muster);
        CHECK(pb::artVonDatei(QStringLiteral("a.LVCLIP")) == pb::Art::Clip);
        CHECK(pb::artVonDatei(QStringLiteral("a.lvtrack")) == pb::Art::Spur);
        CHECK(pb::artText(pb::Art::Muster) == QStringLiteral("lvpattern"));
        CHECK(pb::artAusText(QStringLiteral("lvtrack")) == pb::Art::Spur);
        CHECK(pb::austauschArt(pb::Art::Clip) == lp::AustauschArt::Clip);
        CHECK_FALSE(pb::austauschArt(pb::Art::Loop).has_value());
        CHECK(pb::bereichOrdner(pb::Art::Muster) == QStringLiteral("patterns"));
        CHECK(pb::bereichOrdner(pb::Art::Clip) == QStringLiteral("tracks"));
        CHECK(pb::bereichId(pb::Art::Spur) == QStringLiteral("tracks"));
    }

    TEST_CASE("Publish Austausch 33e: ein eigenes Muster — gesammelt, gepackt, beim Empfaenger in patterns")
    {
        const PublishApp app;
        const Bestand b;
        const QString absender = QDir(b.tmp.path()).filePath(QStringLiteral("absender"));
        std::optional<AustauschBasis> basis(std::in_place, absender);

        // das Drum-Muster D des Bestands als eigenes .lvpattern
        lp::Loop loop;
        REQUIRE(lp::ladeLoop(b.loopDatei, loop));
        const QString datei = QDir(lp::eigenerAustauschOrdner(lp::AustauschArt::Muster)).filePath(QStringLiteral("drums.lvpattern"));
        REQUIRE(QDir().mkpath(QFileInfo(datei).absolutePath()));
        REQUIRE(lp::speichereAustausch(lp::AustauschArt::Muster, *lp::musterDatei(loop, QStringLiteral("D")), datei));

        const pb::Fund fund = pb::sammle(datei);
        REQUIRE_MESSAGE(fund.ok, fund.fehler.toStdString());
        CHECK(fund.art == pb::Art::Muster);
        CHECK(fund.hauptName == QStringLiteral("drums"));
        // es nennt, was D nennt: das Kit mein (und darin bass mit kick und herz), pump_bass mit
        // snare_hit, den Kick auf der Platte, den eigenen kick
        CHECK(fund.finde(pb::Art::Kit, QStringLiteral("mein")) != nullptr);
        CHECK(fund.finde(pb::Art::Instrument, QStringLiteral("pump_bass")) != nullptr);
        CHECK(fund.finde(pb::Art::Sample, QStringLiteral("snare_hit")) != nullptr);
        REQUIRE(fund.fehlt.size() == 1);
        CHECK(fund.fehlt.front().nennung == QStringLiteral("user:gibt_es_nicht"));

        const QString paket = QDir(b.tmp.path()).filePath(pb::paketName(fund));
        CHECK(QFileInfo(paket).fileName() == QStringLiteral("drums.lvpattern.lvdata"));
        REQUIRE(pb::packe(fund, paket, QStringLiteral("1.2.3")).ok);

        // der Empfaenger: eigene Ordner, ein eigenes patterns
        const Empfaenger e(b.tmp.path());
        basis.reset();
        const AustauschBasis beimEmpfaenger(QDir(b.tmp.path()).filePath(QStringLiteral("empfaenger")));
        const pb::PaketInhalt inhalt = pb::oeffnePaket(paket);
        REQUIRE_MESSAGE(inhalt.ok, inhalt.fehler.toStdString());
        CHECK(inhalt.art == pb::Art::Muster);
        CHECK_FALSE(inhalt.brauchtZiel());
        const pb::PaketErgebnis ergebnis = pb::importierePaket(paket, {});
        REQUIRE_MESSAGE(ergebnis.ok, ergebnis.fehler.toStdString());
        const QString ziel = QDir(lp::eigenerAustauschOrdner(lp::AustauschArt::Muster)).filePath(QStringLiteral("drums.lvpattern"));
        CHECK(QFileInfo(ergebnis.hauptDatei) == QFileInfo(ziel));

        // das Muster nennt nur noch eigene Dinge des Empfaengers
        lp::Loop teil;
        REQUIRE(lp::ladeAustausch(ziel, lp::AustauschArt::Muster, teil));
        const lp::Muster& d = teil.muster.at(0);
        CHECK(d.kit == QStringLiteral("user:mein"));
        CHECK(d.spuren.at(1).instrument == QStringLiteral("user:pump_bass"));
        CHECK(d.spuren.at(4).klang == QStringLiteral("user:kick_2"));
        lp::Instrument pump;
        REQUIRE(lp::holeInstrument(QStringLiteral("user:pump_bass"), pump));
        CHECK(pump.schichten.at(0).sample == QStringLiteral("user:snare_hit"));
    }
}
