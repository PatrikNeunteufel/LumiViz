/**
 ****************************************************************************************
 * @file   test_ComposerVorlagen.cpp
 * @brief  Composer: die festen Vorlagen der Sichttests (die `.lvcomp` unter
 *         `asset/sichttest`) laden ohne Meldung und tragen, was die Sichttest-Listen zusagen (S103)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Die Sichttest-Listen nennen Zeiten und Kanaele der Vorlagen beim Wort („CH2
 * mit einem Clip von 0:10 bis 0:50"). Dieser Test haelt Liste und Vorlage
 * beisammen: aendert sich das Format oder eine Vorlage, faellt er um, bevor
 * ein Sichttest an falschem Material scheitert. Nur das Modell — kein Ton,
 * kein Fenster.
 *
 * Rueckbau (Konzept §12): diese Datei loeschen, Zeile in Source.cmake nehmen;
 * der Ordner `asset/sichttest` geht mit.
 ****************************************************************************************
 */

#include <doctest.h>

#include "composer/Komposition.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>

#include <cstdint>
#include <vector>

using namespace lumi::composer;

namespace
{

struct ClipZusage
{
    const char* datei;  ///< Name ohne Ordner und Endung
    std::int64_t startMs;
    std::int64_t endeMs;
};

struct KanalZusage
{
    const char* name;
    KanalArt art;
    std::vector<ClipZusage> clips;
};

struct VorlagenZusage
{
    const char* datei;
    std::int64_t laengeMs;
    std::vector<KanalZusage> kanaele;
};

constexpr KanalArt B = KanalArt::Visual;
constexpr KanalArt T = KanalArt::Audio;

/// Was die Listen `.claude/handover/Sichttest_Composer_*.md` ueber die Vorlagen sagen.
std::vector<VorlagenZusage> zusagen()
{
    return {
        {"composer_A_kurz.lvcomp", 60000,
         {{"CH1", B, {{"Stern", 0, 60000}}},
          {"CH2", B, {{"Sternenfeld", 10000, 50000}}},
          {"A1", T, {{"15_pseudo_musik_120bpm_60s", 0, 50000}}}}},
        {"composer_A_master.lvcomp", 60000,
         {{"CH1", B, {{"Stern", 0, 60000}}},
          {"CH2", B, {{"Sternenfeld", 10000, 50000}}},
          {"A1", T, {{"15_pseudo_musik_120bpm_60s", 0, 50000}}}}},
        {"composer_B_viele.lvcomp", 360000,
         {{"CH1", B, {{"Stern", 0, 180000}, {"Ring", 190000, 360000}}},
          {"CH2", B, {{"Sternenfeld", 0, 180000}}},
          {"A1", T,
           {{"14_amplituden_rampe_60s", 0, 60000},
            {"05_sweep_log_20-20k_60s", 60000, 120000},
            {"15_pseudo_musik_120bpm_60s", 120000, 180000},
            {"14_amplituden_rampe_60s", 180000, 240000},
            {"05_sweep_log_20-20k_60s", 240000, 300000},
            {"15_pseudo_musik_120bpm_60s", 300000, 360000}}},
          {"CH3", B, {{"Dreiecke", 30000, 90000}}},
          {"CH4", B, {{"Fontaene", 60000, 120000}}},
          {"CH5", B, {{"Punktgitter", 120000, 240000}}},
          {"CH6", B, {{"Teilchen", 240000, 300000}}},
          {"A2", T, {{"07_kick_60bpm_30s", 10000, 40000}}},
          {"A3", T, {{"08_kick_120bpm_30s", 60000, 90000}}},
          {"A4", T, {{"02_sinus440_mono_30s", 120000, 150000}}},
          {"A5", T, {{"13_rauschen_rosa_30s", 180000, 210000}}},
          {"A6", T, {{"07_kick_60bpm_30s", 300000, 330000}}}}},
        {"composer_C_songwechsel.lvcomp", 120000,
         {{"CH1", B, {{"Stern", 0, 120000}}},
          {"A1", T, {{"02_sinus440_mono_30s", 10000, 40000}, {"13_rauschen_rosa_30s", 70000, 100000}}},
          {"A2", T, {{"08_kick_120bpm_30s", 30000, 60000}}}}},
        {"composer_D_schleife.lvcomp", 30000,
         {{"CH1", B, {{"Fontaene", 0, 30000}}}, {"A1", T, {{"07_kick_60bpm_30s", 0, 30000}}}}},
        {"composer_E_karaoke.lvcomp", 60000,
         {{"CH1", B, {{"Sternenfeld", 0, 60000}}},
          {"A1", T, {{"15_pseudo_musik_120bpm_60s", 0, 60000}}}}},
    };
}

QString vorlagenOrdner()
{
    return QDir::cleanPath(QFileInfo(QString::fromUtf8(__FILE__)).absolutePath() +
                           QStringLiteral("/../../../../../../asset/sichttest"));
}

Komposition lade(const QString& name)
{
    Komposition k;
    QStringList bericht;
    const QString pfad = QDir(vorlagenOrdner()).filePath(name);
    REQUIRE_MESSAGE(ladeKomposition(pfad, k, &bericht), pfad.toStdString());
    CHECK_MESSAGE(bericht.isEmpty(), bericht.join(QStringLiteral(" | ")).toStdString());
    return k;
}

}  // namespace

TEST_SUITE("ComposerVorlagen")
{
    TEST_CASE("S103 Vorlagen der Sichttests: laden ohne Meldung und halten die Zusagen der Listen")
    {
        const QDir ordner(vorlagenOrdner());
        REQUIRE_MESSAGE(ordner.exists(), "asset/sichttest nicht gefunden");
        const std::vector<VorlagenZusage> alle = zusagen();

        SUBCASE("jede Vorlage im Ordner hat eine Zusage, jede Zusage eine Vorlage")
        {
            QStringList erwartet;
            for (const VorlagenZusage& z : alle) erwartet << QString::fromLatin1(z.datei);
            erwartet.sort();
            // nur die Vorlagen: ein Sichttest darf daneben `test_*.lvcomp` ablegen
            QStringList vorhanden = ordner.entryList({QStringLiteral("composer_*.lvcomp")}, QDir::Files);
            vorhanden.sort();
            CHECK(vorhanden == erwartet);
        }

        SUBCASE("Laenge, Kanaele und Clips")
        {
            for (const VorlagenZusage& z : alle)
            {
                CAPTURE(z.datei);
                const Komposition k = lade(QString::fromLatin1(z.datei));
                CHECK(k.laenge() == z.laengeMs);
                CHECK(k.laengeMs == 0);  // kein festes Ende: es folgt dem letzten Clip
                REQUIRE(k.kanaele.size() == z.kanaele.size());
                for (std::size_t i = 0; i < z.kanaele.size(); ++i)
                {
                    const Kanal& kanal = k.kanaele[i];
                    const KanalZusage& kz = z.kanaele[i];
                    CAPTURE(kz.name);
                    CHECK(kanal.name == QString::fromLatin1(kz.name));
                    CHECK(kanal.art == kz.art);
                    CHECK(kanal.an);
                    CHECK(kanal.farbe == -1);  // keine gespeicherte Farbe
                    REQUIRE(kanal.clips.size() == kz.clips.size());
                    for (std::size_t c = 0; c < kz.clips.size(); ++c)
                    {
                        const Clip& clip = kanal.clips[c];
                        CAPTURE(kz.clips[c].datei);
                        CHECK(QFileInfo(clip.datei).completeBaseName() == QString::fromLatin1(kz.clips[c].datei));
                        CHECK(QFileInfo::exists(clip.datei));
                        CHECK(clip.startMs == kz.clips[c].startMs);
                        CHECK(clip.endeMs == kz.clips[c].endeMs);
                    }
                }
            }
        }

        SUBCASE("A kurz: nur CH1 reicht bis ans Ende, CH2 blendet je 5 s, keine Master-Kurve")
        {
            const Komposition k = lade(QStringLiteral("composer_A_kurz.lvcomp"));
            CHECK(k.kanaele[0].clips[0].vollerPegel());
            const Clip& ch2 = k.kanaele[1].clips[0];
            CHECK(ch2.ein.dauerMs == 5000);
            CHECK(ch2.aus.dauerMs == 5000);
            CHECK(ch2.mischart == Mischart::Additive);
            for (const MasterArt art : {MasterArt::Gesamt, MasterArt::Bild, MasterArt::Ton, MasterArt::Karaoke})
                CHECK_FALSE(k.hatKurve(art));
        }

        SUBCASE("A master: ein shape der Zeile MASTER, Mitte 0:30, Weite 10 s")
        {
            const Komposition k = lade(QStringLiteral("composer_A_master.lvcomp"));
            REQUIRE(k.gesamtStuecke.size() == 1);
            CHECK(k.gesamtStuecke[0].mitteMs == 30000);
            CHECK(k.gesamtStuecke[0].weiteMs == 10000);
            CHECK_FALSE(k.gesamtStuecke[0].gekippt);
            CHECK(k.grundpegel == doctest::Approx(1.0));
            CHECK(k.blendeBei(20000) == doctest::Approx(1.0));
            CHECK(k.blendeBei(30000) == doctest::Approx(0.0).epsilon(0.01));
            CHECK(k.blendeBei(40000) == doctest::Approx(1.0));
            CHECK_FALSE(k.hatKurve(MasterArt::Bild));
            CHECK_FALSE(k.hatKurve(MasterArt::Ton));
        }

        SUBCASE("B viele: CH1 und CH2 ueberdecken sich voll, die unteren Songs sind leiser")
        {
            const Komposition k = lade(QStringLiteral("composer_B_viele.lvcomp"));
            CHECK(k.kanaele[0].clips[0].mischart == Mischart::Normal);
            CHECK(k.kanaele[1].clips[0].mischart == Mischart::Normal);
            const Clip& ch3 = k.kanaele[3].clips[0];
            CHECK(ch3.ein.dauerMs == 10000);
            CHECK(ch3.aus.dauerMs == 10000);
            for (std::size_t i = 7; i < k.kanaele.size(); ++i)
                CHECK(k.kanaele[i].clips[0].grundpegel == doctest::Approx(0.3));
        }

        SUBCASE("C Songwechsel: Ueberlagerung 0:30 bis 0:40, Kreuzung bei 0:35, Luecke 1:00 bis 1:10")
        {
            const Komposition k = lade(QStringLiteral("composer_C_songwechsel.lvcomp"));
            const Clip& sinus = k.kanaele[1].clips[0];
            const Clip& kick = k.kanaele[2].clips[0];
            CHECK(sinus.aus.dauerMs == 10000);
            CHECK(kick.ein.dauerMs == 10000);
            CHECK(k.toeneBei(35000).size() == 2);
            CHECK(sinus.pegelBei(35000 - sinus.startMs) == doctest::Approx(0.5).epsilon(0.02));
            CHECK(kick.pegelBei(35000 - kick.startMs) == doctest::Approx(0.5).epsilon(0.02));
            CHECK(k.kanaele[1].clips[1].ein.dauerMs == 10000);  // der dritte Song blendet allein ein
            CHECK(k.toeneBei(5000).empty());    // der erste Song beginnt bei 0:10
            CHECK(k.toeneBei(65000).empty());   // Luecke
            CHECK(k.toeneBei(110000).empty());  // Bild laeuft ueber den letzten Song hinaus
        }

        SUBCASE("D Schleife: Loop overlap 5 s, Songs doppelt, Bilder nicht")
        {
            const Komposition k = lade(QStringLiteral("composer_D_schleife.lvcomp"));
            CHECK(k.ueberlappungMs == 5000);
            CHECK(k.ueberlappung() == 5000);
            CHECK(k.songsDoppelt);
            CHECK_FALSE(k.bilderDoppelt);
            CHECK(k.kanaele[0].clips[0].ein.dauerMs == 5000);
            CHECK(k.kanaele[0].clips[0].aus.dauerMs == 5000);
        }

        SUBCASE("E Karaoke: der Songtext liegt neben dem Song, elf Zeilen im Abstand von 5 s")
        {
            const Komposition k = lade(QStringLiteral("composer_E_karaoke.lvcomp"));
            const QFileInfo song(k.kanaele[1].clips[0].datei);
            QFile lrc(song.absolutePath() + QLatin1Char('/') + song.completeBaseName() + QStringLiteral(".lrc"));
            REQUIRE(lrc.open(QIODevice::ReadOnly | QIODevice::Text));
            const QStringList zeilen = QString::fromUtf8(lrc.readAll()).split(QLatin1Char('\n'), Qt::SkipEmptyParts);
            REQUIRE(zeilen.size() == 11);
            CHECK(zeilen.first() == QStringLiteral("[00:05.00]Zeile 1 beginnt bei 0:05"));
            CHECK(zeilen.last() == QStringLiteral("[00:55.00]Zeile 11 beginnt bei 0:55"));
            CHECK_FALSE(k.hatKurve(MasterArt::Karaoke));
        }
    }
}
