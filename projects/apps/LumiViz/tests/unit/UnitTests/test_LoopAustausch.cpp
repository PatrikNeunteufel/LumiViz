/**
 ****************************************************************************************
 * @file   test_LoopAustausch.cpp
 * @brief  Loop, Stufe 33: Muster, Clips und Spuren als Dateien — `.lvpattern`,
 *         `.lvclip`, `.lvtrack` (docs/audio/Loop_Konzept.md §4o; Entscheide Patrik S167)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Fenster und ohne Qt-Anwendung; Dateien in Wegwerf-Ordnern.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Austausch.hpp"
#include "loop/Loop.hpp"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>

#include <algorithm>

using namespace lumi::loop;

namespace
{

/// Ein Loop in 4/4 mit vier Schritten je Schlag: ein Drum-Muster mit einem eigenen
/// Sample neben der Datei, ein Instrument-Muster, eines ohne Clip; zwei Kanaele.
Loop beispiel(const QString& sample)
{
    Loop loop;
    loop.bpm = 96.0;
    loop.swing = 0.2;
    loop.laut = 0.7;
    loop.tags.titel = QStringLiteral("Quelle");
    Muster drums;
    drums.name = QStringLiteral("Drums");
    drums.takte = 1;
    drums.spuren = {{QStringLiteral("Kick"), QStringLiteral("kit:kick"), 1.0, 0.0, false},
                    {QStringLiteral("Clap"), sample, 0.8, -0.3, false}};
    drums.zeilen = {{1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0}, {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0.5, 0, 0, 0}};
    Muster bass;
    bass.name = QStringLiteral("Bass");
    bass.art = MusterArt::Instrument;
    bass.takte = 2;
    bass.instrument = QStringLiteral("piano");
    bass.noten = {{0, 4, 36, 1.0}, {8, 2, 43, 0.7}, {16, 8, 41, 0.9}};
    Muster frei = drums;
    frei.name = QStringLiteral("Frei");
    loop.muster = {drums, bass, frei};

    LoopClip mitEcho(QStringLiteral("Drums"), 0, 2);
    ClipEcho echo;
    echo.an = true;
    echo.feedback = 0.5;
    mitEcho.effekte.reihe = {echo};
    LoopClip ohneFx(QStringLiteral("Drums"), 4, 1);
    ohneFx.effekteAn = false;
    ohneFx.effekte.reihe = {ClipHall{}};
    LoopClip bassClip(QStringLiteral("Bass"), 2, 2);
    bassClip.expression = {{0.0, 1.0}, {1.5, 0.4}};
    Kanal k1{QStringLiteral("Beat"), true, {mitEcho, ohneFx}};
    k1.gainDb = -3.5;
    Kanal k2{QStringLiteral("Low"), false, {bassClip}};
    k2.gainDb = 2.0;
    loop.kanaele = {k1, k2};
    loop.richte();
    return loop;
}

struct Ordner
{
    QTemporaryDir dir;
    QString sample;
    Ordner()
    {
        REQUIRE(dir.isValid());
        REQUIRE(QDir(dir.path()).mkpath(QStringLiteral("samples")));
        sample = dir.filePath(QStringLiteral("samples/clap.wav"));
        QFile f(sample);
        REQUIRE(f.open(QIODevice::WriteOnly));
    }
};

}  // namespace

TEST_CASE("Loop Austausch 33a: herausloesen — Muster, Clip, Spur")
{
    const Ordner o;
    const Loop loop = beispiel(o.sample);

    const std::optional<Loop> muster = musterDatei(loop, QStringLiteral("Bass"));
    REQUIRE(muster.has_value());
    REQUIRE(muster->muster.size() == 1);
    CHECK(muster->muster[0] == *loop.findeMuster(QStringLiteral("Bass")));
    CHECK(muster->kanaele.empty());
    // Raster, Taktart, Tempo, Swing mit — Tags, Lautstaerke und Limiter nicht
    CHECK(muster->schlaegeJeTakt == 4);
    CHECK(muster->schritteJeSchlag == 4);
    CHECK(muster->bpm == 96.0);
    CHECK(muster->swing == 0.2);
    CHECK(muster->tags == LoopTags{});
    CHECK(muster->laut == Loop{}.laut);
    CHECK_FALSE(musterDatei(loop, QStringLiteral("Fehlt")).has_value());

    // der zweite Clip im Kanal Beat: ab Takt 0, Effekte und Schalter bleiben
    const std::optional<Loop> clip = clipDatei(loop, 0, 1);
    REQUIRE(clip.has_value());
    REQUIRE(clip->kanaele.size() == 1);
    REQUIRE(clip->kanaele[0].clips.size() == 1);
    const LoopClip& c = clip->kanaele[0].clips[0];
    CHECK(c.startTakt == 0);
    CHECK(c.wiederholungen == 1);
    CHECK_FALSE(c.effekteAn);
    CHECK(c.effekte == loop.kanaele[0].clips[1].effekte);
    CHECK(clip->kanaele[0].name == QStringLiteral("Beat"));
    CHECK(clip->kanaele[0].gainDb == 0.0);
    REQUIRE(clip->muster.size() == 1);
    CHECK(clip->muster[0].name == QStringLiteral("Drums"));
    CHECK_FALSE(clipDatei(loop, 0, 2).has_value());
    CHECK_FALSE(clipDatei(loop, 2, 0).has_value());

    // die Spur Low: der Kanal wie er ist, nur das Muster Bass
    const std::optional<Loop> spur = spurDatei(loop, 1);
    REQUIRE(spur.has_value());
    REQUIRE(spur->kanaele.size() == 1);
    CHECK(spur->kanaele[0] == loop.kanaele[1]);
    REQUIRE(spur->muster.size() == 1);
    CHECK(spur->muster[0].name == QStringLiteral("Bass"));
    // Beat nennt Drums zweimal — einmal mit, Frei nicht
    const std::optional<Loop> beat = spurDatei(loop, 0);
    REQUIRE(beat.has_value());
    REQUIRE(beat->muster.size() == 1);
    CHECK(beat->muster[0].name == QStringLiteral("Drums"));
    CHECK_FALSE(spurDatei(loop, -1).has_value());
}

TEST_CASE("Loop Austausch 33a: die drei Dateien — schreiben, lesen, Gestalt, Fassung")
{
    const Ordner o;
    const Loop loop = beispiel(o.sample);
    const QDir hier(o.dir.path());

    struct Fall
    {
        AustauschArt art;
        Loop teil;
        const char* datei;
        const char* format;
    };
    const std::vector<Fall> faelle = {
        {AustauschArt::Muster, *musterDatei(loop, QStringLiteral("Drums")), "drums.lvpattern", "\"lumiviz-pattern\""},
        {AustauschArt::Clip, *clipDatei(loop, 0, 0), "echo.lvclip", "\"lumiviz-clip\""},
        {AustauschArt::Spur, *spurDatei(loop, 1), "low.lvtrack", "\"lumiviz-track\""},
    };
    for (const Fall& f : faelle)
    {
        CAPTURE(f.datei);
        const QString pfad = hier.filePath(QLatin1String(f.datei));
        CHECK(austauschArtVonDatei(pfad) == f.art);
        REQUIRE(speichereAustausch(f.art, f.teil, pfad));
        QFile datei(pfad);
        REQUIRE(datei.open(QIODevice::ReadOnly));
        const QByteArray inhalt = datei.readAll();
        CHECK(inhalt.contains(f.format));
        CHECK(inhalt.contains("\"lumiviz-loop\""));  // die Huelle traegt einen Loop
        CHECK_FALSE(inhalt.contains(o.dir.path().toUtf8()));  // das eigene Sample relativ

        Loop gelesen;
        QStringList bericht;
        REQUIRE(ladeAustausch(pfad, f.art, gelesen, &bericht));
        CHECK(bericht.isEmpty());
        CHECK(gelesen == f.teil);

        // jede andere Art liest diese Datei nicht
        for (const AustauschArt andere : {AustauschArt::Muster, AustauschArt::Clip, AustauschArt::Spur})
            if (andere != f.art) CHECK_FALSE(ladeAustausch(pfad, andere, gelesen));
    }
    CHECK_FALSE(austauschArtVonDatei(QStringLiteral("a.lvloop")).has_value());

    SUBCASE("fehlt das Sample, steht es im Bericht — gelesen wird trotzdem")
    {
        REQUIRE(QFile::remove(o.sample));
        Loop gelesen;
        QStringList bericht;
        REQUIRE(ladeAustausch(hier.filePath(QStringLiteral("drums.lvpattern")), AustauschArt::Muster, gelesen, &bericht));
        REQUIRE(bericht.size() == 1);
        CHECK(bericht[0].startsWith(QStringLiteral("File not found: ")));
    }
    SUBCASE("falsche Gestalt, Fassung von morgen, kein JSON")
    {
        const auto schreibe = [&](const QString& name, const QByteArray& text) {
            const QString pfad = hier.filePath(name);
            QFile f(pfad);
            REQUIRE(f.open(QIODevice::WriteOnly));
            f.write(text);
            return pfad;
        };
        Loop gelesen;
        // ein .lvpattern mit Kanal ist keines
        Loop zwei = *musterDatei(loop, QStringLiteral("Drums"));
        zwei.kanaele = loop.kanaele;
        zwei.muster = loop.muster;
        REQUIRE(speichereAustausch(AustauschArt::Muster, zwei, hier.filePath(QStringLiteral("zwei.lvpattern"))));
        CHECK_FALSE(ladeAustausch(hier.filePath(QStringLiteral("zwei.lvpattern")), AustauschArt::Muster, gelesen));
        // ein .lvtrack ohne Kanal ist keines
        REQUIRE(speichereAustausch(AustauschArt::Spur, *musterDatei(loop, QStringLiteral("Bass")),
                                   hier.filePath(QStringLiteral("leer.lvtrack"))));
        CHECK_FALSE(ladeAustausch(hier.filePath(QStringLiteral("leer.lvtrack")), AustauschArt::Spur, gelesen));

        bool zuNeu = false;
        CHECK_FALSE(ladeAustausch(schreibe(QStringLiteral("neu.lvpattern"),
                                           R"({"format": "lumiviz-pattern", "version": 2, "loop": {}})"),
                                  AustauschArt::Muster, gelesen, nullptr, &zuNeu));
        CHECK(zuNeu);
        CHECK_FALSE(ladeAustausch(schreibe(QStringLiteral("ohne.lvpattern"), R"({"format": "lumiviz-pattern", "version": 1})"),
                                  AustauschArt::Muster, gelesen));
        CHECK_FALSE(ladeAustausch(schreibe(QStringLiteral("kaputt.lvpattern"), "kein json"), AustauschArt::Muster, gelesen));
        CHECK_FALSE(ladeAustausch(hier.filePath(QStringLiteral("gibtsnicht.lvclip")), AustauschArt::Clip, gelesen));
    }
}

TEST_CASE("Loop Austausch 33b: Umrechnung nach Schlaegen (PE2 a)")
{
    const Ordner o;
    const Loop loop = beispiel(o.sample);

    SUBCASE("gleiche Taktart und gleiches Raster: Bit fuer Bit")
    {
        Loop teil = *spurDatei(loop, 0);
        const Loop vorher = teil;
        CHECK(rechneUm(teil, 4, 4) == Umrechnung{});
        CHECK(teil == vorher);
    }

    SUBCASE("Noten aus 4/4 mit vier Schritten in 3/4 mit drei: zeitlich genau")
    {
        Loop teil = *musterDatei(loop, QStringLiteral("Bass"));
        Muster& m = teil.muster[0];
        // eine Note ein Sechzehntel nach dem Schlag, eine kuerzer als ein Schritt
        m.noten.push_back({1, 2, 48, 0.5});
        Note kurz{13, 1, 50, 0.5};
        kurz.versatzEnde = -0.5;
        m.noten.push_back(kurz);
        m.anschlaege = {{8, Anschlagart::Ab, -1.0}};
        m.zeitspuren.von(Steuerspur::Expression) = {{6.0, 0.5}, {20.0, 1.0}};
        teil.richte();
        const Muster alt = teil.muster[0];
        const auto schlagAnfang = [](const Note& n, int jeSchlag) { return (n.schritt + n.versatzAnfang) / jeSchlag; };
        const auto schlagDauer = [](const Note& n, int jeSchlag) { return (n.laenge + n.versatzEnde) / jeSchlag; };

        CHECK(rechneUm(teil, 3, 3) == Umrechnung{});
        const Muster& neu = teil.muster[0];
        CHECK(teil.schlaegeJeTakt == 3);
        CHECK(teil.schritteJeSchlag == 3);
        CHECK(neu.takte == 3);  // 8 Schlaege → 3 Takte zu 3, aufgerundet
        REQUIRE(neu.noten.size() == alt.noten.size());
        for (std::size_t i = 0; i < alt.noten.size(); ++i)
        {
            CAPTURE(i);
            CHECK(neu.noten[i].hoehe == alt.noten[i].hoehe);
            CHECK(schlagAnfang(neu.noten[i], 3) == doctest::Approx(schlagAnfang(alt.noten[i], 4)).epsilon(1e-12));
            CHECK(schlagDauer(neu.noten[i], 3) == doctest::Approx(schlagDauer(alt.noten[i], 4)).epsilon(1e-12));
            CHECK(neu.noten[i].versatzAnfang >= 0.0);
            CHECK(neu.noten[i].versatzAnfang < 1.0);
            CHECK(neu.noten[i].versatzEnde <= 0.0);
            CHECK(neu.noten[i].versatzEnde > -1.0);
        }
        // Schritt 1 (ein Viertel Schlag) → Schritt 0 mit Versatz 0,75
        const auto c48 = std::find_if(neu.noten.begin(), neu.noten.end(), [](const Note& n) { return n.hoehe == 48; });
        REQUIRE(c48 != neu.noten.end());
        CHECK(c48->schritt == 0);
        CHECK(c48->versatzAnfang == doctest::Approx(0.75));
        CHECK(c48->laenge == 2);
        CHECK(c48->versatzEnde == doctest::Approx(-0.5));
        // ein Akkord-Eintrag geht mit seinen Noten, die Spuren genau
        REQUIRE(neu.anschlaege.size() == 1);
        CHECK(neu.anschlaege[0].schritt == 6);
        REQUIRE(neu.zeitspuren.von(Steuerspur::Expression).size() == 2);
        CHECK(neu.zeitspuren.von(Steuerspur::Expression)[0].stelle == doctest::Approx(4.5));
        CHECK(neu.zeitspuren.von(Steuerspur::Expression)[1].stelle == doctest::Approx(15.0));
    }

    SUBCASE("Drum-Zeilen: auf den naechsten Schritt, zwei auf einem werden der lautere")
    {
        Loop teil = *musterDatei(loop, QStringLiteral("Drums"));
        teil.muster[0].zeilen[0] = {1, 0.4, 0.9, 0.6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        teil.richte();
        const Umrechnung u = rechneUm(teil, 4, 3);
        // 0 → 0; 1 → 0,75 → 1; 2 → 1,5 → 1 (der fruehere), zusammengelegt; 3 → 2,25 → 2
        CHECK(u.verschoben == 3);  // die zweite Zeile faellt genau: Schritt 4 → 3, Schritt 12 → 9
        CHECK(u.zusammengelegt == 1);
        const Muster& m = teil.muster[0];
        CHECK(m.takte == 1);
        REQUIRE(m.zeilen[0].size() == 12);
        CHECK(m.zeilen[0][0] == 1.0);
        CHECK(m.zeilen[0][1] == 0.9);
        CHECK(m.zeilen[0][2] == 0.6);
        CHECK(m.zeilen[1][3] == 1.0);
        CHECK(m.zeilen[1][9] == 0.5);
    }

    SUBCASE("feineres Raster, ein Vielfaches: nichts gerundet")
    {
        Loop teil = *musterDatei(loop, QStringLiteral("Drums"));
        const Muster alt = teil.muster[0];
        CHECK(rechneUm(teil, 4, 8) == Umrechnung{});
        REQUIRE(teil.muster[0].zeilen[0].size() == 32);
        for (std::size_t i = 0; i < alt.zeilen[0].size(); ++i) CHECK(teil.muster[0].zeilen[0][2 * i] == alt.zeilen[0][i]);
    }

    SUBCASE("ein Clip: Expression in Takten ueber die Schlaege, Start und Wiederholungen bleiben")
    {
        Loop teil = *spurDatei(loop, 1);
        rechneUm(teil, 3, 4);
        REQUIRE(teil.kanaele[0].clips.size() == 1);
        const LoopClip& c = teil.kanaele[0].clips[0];
        CHECK(c.startTakt == 2);
        CHECK(c.wiederholungen == 2);
        REQUIRE(c.expression.size() == 2);
        CHECK(c.expression[1].stelle == doctest::Approx(2.0));  // 1,5 Takte zu 4 = 6 Schlaege = 2 Takte zu 3
    }
}

TEST_CASE("Loop Austausch 33c: einsetzen — Namen, Ort, neuer Kanal")
{
    const Ordner o;
    const Loop quelle = beispiel(o.sample);
    const auto clipsIn = [](const Loop& loop, int kanal) { return loop.kanaele[static_cast<std::size_t>(kanal)].clips; };

    SUBCASE("ein Muster mit gleichem Inhalt wird mitbenutzt, der Clip steht am Takt")
    {
        Loop ziel = quelle;
        const Einsetzung e = setzeEin(ziel, *musterDatei(quelle, QStringLiteral("Bass")), AustauschArt::Muster, 0, 8);
        CHECK(e.kanal == 0);
        CHECK_FALSE(e.neuerKanal);
        CHECK_FALSE(e.umgerechnet);
        CHECK(e.umbenannt.isEmpty());
        CHECK(ziel.muster.size() == quelle.muster.size());
        REQUIRE(clipsIn(ziel, 0).size() == 3);
        CHECK(clipsIn(ziel, 0)[2] == LoopClip(QStringLiteral("Bass"), 8, 1));
    }

    SUBCASE("gleicher Name, anderer Inhalt: Bass_2 — ein zweites Mal wird Bass_2 mitbenutzt")
    {
        Loop teil = *musterDatei(quelle, QStringLiteral("Bass"));
        teil.muster[0].noten[0].hoehe = 38;
        Loop ziel = quelle;
        Einsetzung e = setzeEin(ziel, teil, AustauschArt::Muster, 1, 6);
        CHECK(e.umbenannt == QStringList{QStringLiteral("Bass → Bass_2")});
        REQUIRE(ziel.findeMuster(QStringLiteral("Bass_2")) != nullptr);
        CHECK(ziel.findeMuster(QStringLiteral("Bass_2"))->noten[0].hoehe == 38);
        CHECK(ziel.findeMuster(QStringLiteral("Bass"))->noten[0].hoehe == 36);
        CHECK(clipsIn(ziel, 1).back().muster == QStringLiteral("Bass_2"));
        const std::size_t zahl = ziel.muster.size();
        e = setzeEin(ziel, teil, AustauschArt::Muster, 0, 20);
        CHECK(ziel.muster.size() == zahl);
        CHECK(clipsIn(ziel, 0).back().muster == QStringLiteral("Bass_2"));
    }

    SUBCASE("kein Platz am Takt: ein neuer Kanal, benannt nach dem Muster")
    {
        Loop ziel = quelle;
        const Einsetzung e = setzeEin(ziel, *musterDatei(quelle, QStringLiteral("Bass")), AustauschArt::Muster, 0, 1);
        CHECK(e.neuerKanal);
        REQUIRE(ziel.kanaele.size() == 3);
        CHECK(e.kanal == 2);
        CHECK(ziel.kanaele[2].name == QStringLiteral("Bass"));
        CHECK(clipsIn(ziel, 2) == std::vector<LoopClip>{LoopClip(QStringLiteral("Bass"), 1, 1)});
        CHECK(clipsIn(ziel, 0) == clipsIn(quelle, 0));
        // ein Kanal, den es nicht gibt, ebenso
        CHECK(setzeEin(ziel, *musterDatei(quelle, QStringLiteral("Bass")), AustauschArt::Muster, 9, 0).neuerKanal);
    }

    SUBCASE("ein Clip: mit Effekten, Schalter und Expression an den Takt")
    {
        Loop ziel = quelle;
        const Einsetzung e = setzeEin(ziel, *clipDatei(quelle, 0, 0), AustauschArt::Clip, 1, 10);
        CHECK(e.kanal == 1);
        LoopClip erwartet = quelle.kanaele[0].clips[0];
        erwartet.startTakt = 10;
        CHECK(clipsIn(ziel, 1).back() == erwartet);
        // kein Platz: der neue Kanal heisst wie der in der Datei
        const Einsetzung voll = setzeEin(ziel, *clipDatei(quelle, 0, 0), AustauschArt::Clip, 1, 2);
        CHECK(voll.neuerKanal);
        CHECK(ziel.kanaele.back().name == QStringLiteral("Beat"));
    }

    SUBCASE("eine Spur aus 3/4: umgerechnet, ein neuer Kanal, Takte bleiben (PE3 b)")
    {
        Loop andere = quelle;
        andere.schlaegeJeTakt = 3;
        andere.richte();
        Loop ziel = quelle;
        const Einsetzung e = setzeEin(ziel, *spurDatei(andere, 1), AustauschArt::Spur, 0, 99);
        CHECK(e.umgerechnet);
        CHECK(e.neuerKanal);
        REQUIRE(ziel.kanaele.size() == 3);
        CHECK(ziel.kanaele[2].name == QStringLiteral("Low"));
        CHECK(ziel.kanaele[2].gainDb == 2.0);
        CHECK_FALSE(ziel.kanaele[2].an);
        REQUIRE(ziel.kanaele[2].clips.size() == 1);
        CHECK(ziel.kanaele[2].clips[0].startTakt == 2);
        // 2 Takte zu 3 Schlaegen = 6 Schlaege → 2 Takte zu 4; die Noten liegen in denselben
        // Schlaegen wie in Bass des Ziels — gleicher Inhalt, also mitbenutzt
        CHECK(ziel.kanaele[2].clips[0].muster == QStringLiteral("Bass"));
        CHECK(ziel.muster.size() == quelle.muster.size());
    }

    SUBCASE("eine Spur aus einem anderen Raster: die Noten anders, Bass_2")
    {
        Loop andere = quelle;
        andere.schritteJeSchlag = 3;
        andere.richte();
        Loop ziel = quelle;
        const Einsetzung e = setzeEin(ziel, *spurDatei(andere, 1), AustauschArt::Spur);
        CHECK(e.umgerechnet);
        CHECK(e.umbenannt == QStringList{QStringLiteral("Bass → Bass_2")});
        const Muster* neu = ziel.findeMuster(QStringLiteral("Bass_2"));
        REQUIRE(neu != nullptr);
        // Schritt 8 bei drei je Schlag = Schlag 2,67 → bei vier je Schlag Schritt 10 mit Versatz 0,67
        const auto g = std::find_if(neu->noten.begin(), neu->noten.end(), [](const Note& n) { return n.hoehe == 43; });
        REQUIRE(g != neu->noten.end());
        CHECK(g->schritt == 10);
        CHECK(g->versatzAnfang == doctest::Approx(2.0 / 3.0));
    }

    SUBCASE("zwei Muster der Datei, beide belegt: die Clips nennen je ihren neuen Namen")
    {
        Loop teil = *spurDatei(quelle, 0);
        Muster zweites = teil.muster[0];
        zweites.name = QStringLiteral("Drums_2");
        zweites.zeilen[0][2] = 1.0;
        teil.muster[0].zeilen[0][1] = 1.0;  // anders als Drums im Ziel
        teil.muster.push_back(zweites);
        teil.kanaele[0].clips[1].muster = QStringLiteral("Drums_2");
        teil.richte();
        Loop ziel = quelle;
        const Einsetzung e = setzeEin(ziel, teil, AustauschArt::Spur);
        CHECK(e.umbenannt == QStringList{QStringLiteral("Drums → Drums_2"), QStringLiteral("Drums_2 → Drums_2_2")});
        const std::vector<LoopClip> clips = clipsIn(ziel, 2);
        REQUIRE(clips.size() == 2);
        CHECK(clips[0].muster == QStringLiteral("Drums_2"));
        CHECK(clips[1].muster == QStringLiteral("Drums_2_2"));
        CHECK(ziel.findeMuster(QStringLiteral("Drums_2"))->zeilen[0][1] == 1.0);
        CHECK(ziel.findeMuster(QStringLiteral("Drums_2_2"))->zeilen[0][2] == 1.0);
    }

    SUBCASE("ohne die Gestalt seiner Art aendert sich nichts")
    {
        Loop ziel = quelle;
        CHECK(setzeEin(ziel, *musterDatei(quelle, QStringLiteral("Bass")), AustauschArt::Spur).kanal == -1);
        CHECK(ziel == quelle);
    }
}
