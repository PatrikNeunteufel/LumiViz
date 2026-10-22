/**
 * @file test_PublishFreeze.cpp
 * @brief Publish, Stufe 30i: die Wellenformen eingefrorener Clips gehen mit
 *        (docs/audio/Spielweisen_Konzept.md §4.4; Entscheide F5 a, FP1 a, FP3 a)
 *
 * Ohne Ton, ohne Fenster. Der Absender ist der Bestand aus `PaketBestand.hpp`,
 * dazu Kanaele mit zwei eingefrorenen Clips; der Empfaenger hat eigene Ordner und
 * einen eigenen Ordner `frozen` im selben Wegwerf-Ordner. Die Wellenformen sind
 * kurze Stuecke ohne Klang — es geht um Abdruck und Ablage, nicht um das Rechnen.
 */

#include "PaketBestand.hpp"

#include "core/ZipArchiv.hpp"
#include "loop/Freeze.hpp"
#include "paket/Import.hpp"
#include "paket/Packen.hpp"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>

#include <functional>

namespace
{

namespace ds = lumi::core::datensicherung;

constexpr int kRate = 44100;

/// Eigene Einstellungen (`LumiVizTest` / `PublishFreeze`) — die Grenze des Ordners `frozen`.
struct FreezeEinstellungen
{
    QString org = QCoreApplication::organizationName();
    QString name = QCoreApplication::applicationName();
    FreezeEinstellungen()
    {
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("PublishFreeze"));
        QSettings().clear();
    }
    ~FreezeEinstellungen()
    {
        QSettings().clear();
        QCoreApplication::setOrganizationName(org);
        QCoreApplication::setApplicationName(name);
    }
};

/// Solange er lebt, ist `pfad` der Ordner `frozen`.
struct FrozenOrdner
{
    QString pfad;
    explicit FrozenOrdner(const QString& p) : pfad(p) { lp::setzeFrozenOrdnerFuerTests(pfad); }
    ~FrozenOrdner() { lp::setzeFrozenOrdnerFuerTests({}); }
};

QByteArray liesBytes(const QString& pfad)
{
    QFile datei(pfad);
    return datei.open(QIODevice::ReadOnly) ? datei.readAll() : QByteArray("<fehlt>");
}

/// Der Bestand mit Kanaelen: Bass (Muster B, ab Takt 5) und Drums (Muster D) eingefroren,
/// Piano (Muster P) live.
struct FrozenBestand : Bestand
{
    QByteArray bass, drums;  ///< die Abdruecke beim Absender
    QString absender = QDir(tmp.path()).filePath(QStringLiteral("frozen_absender"));
    QString empfaenger = QDir(tmp.path()).filePath(QStringLiteral("frozen_empfaenger"));

    FrozenBestand()
    {
        lp::Loop loop;
        REQUIRE(lp::ladeLoop(loopDatei, loop));
        const auto kanal = [](const QString& name, const QString& muster, int start) {
            lp::Kanal k;
            k.name = name;
            lp::LoopClip clip;
            clip.muster = muster;
            clip.startTakt = start;
            k.clips = {clip};
            return k;
        };
        loop.kanaele = {kanal(QStringLiteral("Bass"), QStringLiteral("B"), 4), kanal(QStringLiteral("Drums"), QStringLiteral("D"), 0),
                        kanal(QStringLiteral("Piano"), QStringLiteral("P"), 0)};
        loop.richte();
        const FrozenOrdner ordner(absender);
        const lp::SaitenVon saiten = lp::saitenAusInstrumenten();
        const std::vector<lp::ClipOrt> zuRechnen =
            lp::setzeFrozenMarken(loop, {lp::ClipOrt{0, 4}, lp::ClipOrt{1, 0}}, kRate, saiten);
        CHECK(zuRechnen.size() == 2);
        bass = lp::clipAbdruck(loop, lp::ClipOrt{0, 4}, kRate, saiten);
        drums = lp::clipAbdruck(loop, lp::ClipOrt{1, 0}, kRate, saiten);
        REQUIRE(bass.size() == 40);
        REQUIRE(drums.size() == 40);
        REQUIRE(bass != drums);
        REQUIRE(lp::speichereLoop(loop, loopDatei));
    }
};

/// Packt den Loop mit den Wellenformen zu `abdruecke`.
QString packeMit(const Bestand& b, const std::vector<QByteArray>& abdruecke, const QString& name,
                 pb::PackErgebnis* ergebnis = nullptr)
{
    const pb::Fund fund = pb::sammle(b.loopDatei);
    REQUIRE(fund.ok);
    const QString ziel = QDir(b.tmp.path()).filePath(name);
    const pb::PackErgebnis e = pb::packe(fund, ziel, QStringLiteral("1.2.3"), abdruecke);
    REQUIRE_MESSAGE(e.ok, e.fehler.toStdString());
    if (ergebnis != nullptr) *ergebnis = e;
    return ziel;
}

/// Eine Wellenform von `n` Abtastwerten je Kanal mit dem Wert `wert`.
std::vector<float> welle(float wert, std::size_t n = 1000)
{
    return std::vector<float>(2 * n, wert);
}

/// Das Paket umbauen: Manifest und Eintraege.
void aenderePaket(const QString& paket, const std::function<void(QJsonObject&, std::vector<lumi::core::ZipEintrag>&)>& aendere)
{
    std::vector<lumi::core::ZipEintrag> eintraege;
    REQUIRE(lumi::core::leseZip(paket, eintraege));
    QJsonObject manifest;
    for (const auto& e : eintraege)
        if (e.name == QStringLiteral("manifest.json")) manifest = QJsonDocument::fromJson(e.daten).object();
    aendere(manifest, eintraege);
    for (auto& e : eintraege)
        if (e.name == QStringLiteral("manifest.json")) e.daten = QJsonDocument(manifest).toJson();
    REQUIRE(lumi::core::schreibeZip(paket, eintraege));
}

}  // namespace

TEST_SUITE("Publish")
{
    TEST_CASE("Publish Freeze 30i: die Wellenformen gehen mit und kommen an, wo sie passen (FP1 a, FP3 a)")
    {
        const PublishApp app;
        const FreezeEinstellungen einstellungen;
        const FrozenBestand b;
        const QString& absender = b.absender;
        const QString& empfaenger = b.empfaenger;
        const lp::SaitenVon saiten = lp::saitenAusInstrumenten();

        // beim Absender: zwei Clips mit Marke, die Wellenformen fehlen noch
        std::vector<pb::FrozenTeil> teile;
        {
            const FrozenOrdner ordner(absender);
            teile = pb::frozenTeile(b.loopDatei, kRate, saiten);
            REQUIRE(teile.size() == 2);
            CHECK(teile[0].etikett == QStringLiteral("Bass · bar 5"));
            CHECK(teile[0].abdruck == b.bass);
            CHECK(teile[0].datei.isEmpty());
            CHECK(teile[1].etikett == QStringLiteral("Drums · bar 1"));
            CHECK(teile[1].abdruck == b.drums);

            REQUIRE(lp::legeFrozen(b.bass, welle(0.25f), kRate));
            REQUIRE(lp::legeFrozen(b.drums, welle(-0.5f, 1500), kRate));
            teile = pb::frozenTeile(b.loopDatei, kRate, saiten);
            REQUIRE(teile.size() == 2);
            CHECK(teile[0].datei == lp::frozenDatei(b.bass));
            CHECK(teile[0].groesse == QFileInfo(lp::frozenDatei(b.bass)).size());
            CHECK(teile[1].groesse > teile[0].groesse);
            // eine andere Rate: ein anderer Abdruck, keine Datei
            const std::vector<pb::FrozenTeil> anders = pb::frozenTeile(b.loopDatei, 48000, saiten);
            REQUIRE(anders.size() == 2);
            CHECK(anders[0].abdruck != b.bass);
            CHECK(anders[0].datei.isEmpty());
        }

        // packen: ohne und mit Wellenformen
        pb::PackErgebnis ohne, mit;
        (void)packeMit(b, {}, QStringLiteral("ohne.lvloop.lvdata"), &ohne);
        QString paket;
        {
            const FrozenOrdner ordner(absender);
            paket = packeMit(b, {b.bass, b.drums, b.bass}, QStringLiteral("titan.lvloop.lvdata"), &mit);
        }
        CHECK(ohne.frozen == 0);
        CHECK(mit.frozen == 2);  // jeder Abdruck einmal
        CHECK(mit.dateien == ohne.dateien + 2);
        CHECK(mit.groesse == ohne.groesse + teile[0].groesse + teile[1].groesse);
        CHECK(pb::frozenPfad(b.bass) == QStringLiteral("frozen/") + QString::fromLatin1(b.bass) + QStringLiteral(".wav"));

        const Empfaenger e(b.tmp.path());
        const FrozenOrdner ordner(empfaenger);
        const pb::PaketInhalt inhalt = pb::oeffnePaket(paket);
        REQUIRE_MESSAGE(inhalt.ok, inhalt.fehler.toStdString());
        CHECK(inhalt.frozen == 2);
        CHECK(inhalt.frozenGroesse == teile[0].groesse + teile[1].groesse);
        CHECK(inhalt.dateien.size() == 7);  // die Wellenformen sind keine Dateien des Benutzerordners

        const QString loopZiel = QDir(e.loops).filePath(QStringLiteral("titan.lvloop"));
        pb::PaketWahl wahl{{}, loopZiel};
        wahl.rate = kRate;
        const pb::PaketErgebnis ergebnis = pb::importierePaket(paket, wahl);
        REQUIRE_MESSAGE(ergebnis.ok, ergebnis.fehler.toStdString());
        CHECK(ergebnis.frozenClips == 2);
        CHECK(ergebnis.frozenGelegt == 2);
        CHECK(ergebnis.frozenBereit == 2);
        CHECK(liesBytes(lp::frozenDatei(b.bass)) == liesBytes(QDir(absender).filePath(QString::fromLatin1(b.bass) + ".wav")));
        CHECK(lp::holeFrozen(b.drums, kRate).has_value());

        // der Loop beim Empfaenger nennt andere Dateien — und ergibt dieselben Abdruecke
        lp::Loop loop;
        REQUIRE(lp::ladeLoop(loopZiel, loop));
        loop.richte();
        CHECK(lp::clipAbdruck(loop, lp::ClipOrt{0, 4}, kRate, lp::saitenAusInstrumenten()) == b.bass);
        CHECK(lp::eingefrorenVon(loop, kRate, lp::saitenAusInstrumenten()).size() == 2);

        // ein zweiter Import: die Dateien liegen schon da und bleiben
        const pb::PaketErgebnis zweiter = pb::importierePaket(paket, wahl);
        REQUIRE(zweiter.ok);
        CHECK(zweiter.frozenGelegt == 0);
        CHECK(zweiter.frozenBereit == 2);

        // ohne Rate (0): nichts abgelegt, nichts gezaehlt
        pb::PaketWahl ohneRate{{}, QDir(e.loops).filePath(QStringLiteral("titan_3.lvloop"))};
        const pb::PaketErgebnis still = pb::importierePaket(paket, ohneRate);
        REQUIRE(still.ok);
        CHECK(still.frozenClips == 0);
        CHECK(still.frozenGelegt == 0);
    }

    TEST_CASE("Publish Freeze 30i: Keep both, Use mine, andere Rate, vorhandene Datei, fremde Wellenform (FP3 a)")
    {
        const PublishApp app;
        const FreezeEinstellungen einstellungen;
        const FrozenBestand b;
        const QString& absender = b.absender;
        const QString& empfaenger = b.empfaenger;
        QString paket;
        {
            const FrozenOrdner ordner(absender);
            REQUIRE(lp::legeFrozen(b.bass, welle(0.25f), kRate));
            REQUIRE(lp::legeFrozen(b.drums, welle(-0.5f), kRate));
            paket = packeMit(b, {b.bass, b.drums}, QStringLiteral("titan.lvloop.lvdata"));
        }
        const Empfaenger e(b.tmp.path());
        const FrozenOrdner ordner(empfaenger);
        // beim Empfaenger ein anderer bass: der Name ist vergeben
        lp::Instrument seiner;
        seiner.titel = QStringLiteral("Empfaenger Bass");
        REQUIRE(lp::speichereInstrument(seiner, QDir(e.instrumente).filePath(QStringLiteral("bass.lvinst"))));
        const QString loopZiel = QDir(e.loops).filePath(QStringLiteral("titan.lvloop"));
        const QString bassPfad = QStringLiteral("data/instruments/bass.lvinst");

        SUBCASE("Keep both: bass_2 heisst anders, klingt gleich — beide Wellenformen passen")
        {
            pb::PaketWahl wahl{{}, loopZiel};
            wahl.rate = kRate;
            const pb::PaketErgebnis ergebnis = pb::importierePaket(paket, wahl);
            REQUIRE(ergebnis.ok);
            CHECK(ergebnis.umbenannt == 1);
            CHECK(ergebnis.frozenGelegt == 2);
            CHECK(ergebnis.frozenBereit == 2);
        }
        SUBCASE("Use mine: der eigene bass klingt anders — der Clip Bass wird hier gerechnet")
        {
            pb::PaketWahl wahl{{{bassPfad, ds::Wahl::Meine}}, loopZiel};
            wahl.rate = kRate;
            const pb::PaketErgebnis ergebnis = pb::importierePaket(paket, wahl);
            REQUIRE(ergebnis.ok);
            CHECK(ergebnis.eigeneGenommen == 1);
            CHECK(ergebnis.frozenClips == 2);
            CHECK(ergebnis.frozenGelegt == 1);
            CHECK(ergebnis.frozenBereit == 1);
            CHECK_FALSE(QFileInfo::exists(lp::frozenDatei(b.bass)));
            CHECK(QFileInfo::exists(lp::frozenDatei(b.drums)));
        }
        SUBCASE("eine andere Ausgaberate: kein Abdruck passt, nichts wird abgelegt")
        {
            pb::PaketWahl wahl{{}, loopZiel};
            wahl.rate = 48000;
            const pb::PaketErgebnis ergebnis = pb::importierePaket(paket, wahl);
            REQUIRE(ergebnis.ok);
            CHECK(ergebnis.frozenClips == 2);
            CHECK(ergebnis.frozenGelegt == 0);
            CHECK(ergebnis.frozenBereit == 0);
            CHECK(lp::frozenStand().dateien == 0);
        }
        SUBCASE("eine eigene Datei gleichen Namens bleibt")
        {
            REQUIRE(lp::legeFrozen(b.drums, welle(0.75f, 3000), kRate));
            const QByteArray eigene = liesBytes(lp::frozenDatei(b.drums));
            pb::PaketWahl wahl{{}, loopZiel};
            wahl.rate = kRate;
            const pb::PaketErgebnis ergebnis = pb::importierePaket(paket, wahl);
            REQUIRE(ergebnis.ok);
            CHECK(ergebnis.frozenGelegt == 1);
            CHECK(ergebnis.frozenBereit == 2);
            CHECK(liesBytes(lp::frozenDatei(b.drums)) == eigene);
        }
        SUBCASE("eine Wellenform, die zu keinem Clip gehoert, bleibt draussen; eine kaputte auch")
        {
            const QByteArray fremd(40, 'a');
            const QByteArray kaputt = b.drums;
            aenderePaket(paket, [&](QJsonObject& manifest, std::vector<lumi::core::ZipEintrag>& eintraege) {
                QJsonObject p = manifest.value(QStringLiteral("package")).toObject();
                QJsonArray frozen = p.value(QStringLiteral("frozen")).toArray();
                frozen.append(QJsonObject{{QStringLiteral("path"), pb::frozenPfad(fremd)},
                                          {QStringLiteral("fingerprint"), QString::fromLatin1(fremd)}});
                p.insert(QStringLiteral("frozen"), frozen);
                manifest.insert(QStringLiteral("package"), p);
                QByteArray wav;
                for (const auto& e : eintraege)
                    if (e.name == pb::frozenPfad(b.bass)) wav = e.daten;
                eintraege.push_back({pb::frozenPfad(fremd), wav, QDateTime()});
                for (auto& e : eintraege)
                    if (e.name == pb::frozenPfad(kaputt)) e.daten = "keine WAV";
            });
            const pb::PaketInhalt inhalt = pb::oeffnePaket(paket);
            REQUIRE_MESSAGE(inhalt.ok, inhalt.fehler.toStdString());
            CHECK(inhalt.frozen == 3);
            pb::PaketWahl wahl{{}, loopZiel};
            wahl.rate = kRate;
            const pb::PaketErgebnis ergebnis = pb::importierePaket(paket, wahl);
            REQUIRE(ergebnis.ok);
            CHECK(ergebnis.frozenGelegt == 1);  // nur Bass
            CHECK(ergebnis.frozenBereit == 1);
            CHECK_FALSE(QFileInfo::exists(lp::frozenDatei(fremd)));
            CHECK_FALSE(QFileInfo::exists(lp::frozenDatei(kaputt)));
            CHECK(lp::frozenStand().dateien == 1);
            CHECK(QDir(empfaenger).entryList(QDir::Files).size() == 1);  // kein `.part` liegt herum
        }
        SUBCASE("ein Eintrag, dessen Pfad nicht zum Abdruck passt: das Paket ist beschaedigt")
        {
            aenderePaket(paket, [&](QJsonObject& manifest, std::vector<lumi::core::ZipEintrag>&) {
                QJsonObject p = manifest.value(QStringLiteral("package")).toObject();
                QJsonArray frozen = p.value(QStringLiteral("frozen")).toArray();
                QJsonObject erster = frozen.at(0).toObject();
                erster.insert(QStringLiteral("path"), QStringLiteral("data/samples/kick.wav"));
                frozen.replace(0, erster);
                p.insert(QStringLiteral("frozen"), frozen);
                manifest.insert(QStringLiteral("package"), p);
            });
            const pb::PaketInhalt inhalt = pb::oeffnePaket(paket);
            CHECK_FALSE(inhalt.ok);
            CHECK(inhalt.fehler.contains(QStringLiteral("damaged")));
        }
        SUBCASE("ein Paket ohne Wellenformen (wie vor 30i): die Clips werden hier gerechnet")
        {
            aenderePaket(paket, [&](QJsonObject& manifest, std::vector<lumi::core::ZipEintrag>&) {
                QJsonObject p = manifest.value(QStringLiteral("package")).toObject();
                p.remove(QStringLiteral("frozen"));
                manifest.insert(QStringLiteral("package"), p);
            });
            const pb::PaketInhalt inhalt = pb::oeffnePaket(paket);
            REQUIRE(inhalt.ok);
            CHECK(inhalt.frozen == 0);
            pb::PaketWahl wahl{{}, loopZiel};
            wahl.rate = kRate;
            const pb::PaketErgebnis ergebnis = pb::importierePaket(paket, wahl);
            REQUIRE(ergebnis.ok);
            CHECK(ergebnis.frozenClips == 2);
            CHECK(ergebnis.frozenGelegt == 0);
            CHECK(ergebnis.frozenBereit == 0);
        }
    }

    TEST_CASE("Publish Freeze 30i: eine Wellenform, die beim Packen fehlt, bleibt draussen; nur ein Loop traegt welche")
    {
        const PublishApp app;
        const FreezeEinstellungen einstellungen;
        const FrozenBestand b;
        const FrozenOrdner ordner(b.absender);
        REQUIRE(lp::legeFrozen(b.drums, welle(-0.5f), kRate));
        pb::PackErgebnis ergebnis;
        (void)packeMit(b, {b.bass, b.drums}, QStringLiteral("titan.lvloop.lvdata"), &ergebnis);
        CHECK(ergebnis.frozen == 1);

        // ein Instrument-Paket nimmt keine Wellenform mit
        const pb::Fund fund = pb::sammle(QDir(b.instrumente).filePath(QStringLiteral("bass.lvinst")));
        REQUIRE(fund.ok);
        const pb::PackErgebnis instrument =
            pb::packe(fund, QDir(b.tmp.path()).filePath(QStringLiteral("bass.lvinst.lvdata")), QStringLiteral("1.2.3"), {b.drums});
        REQUIRE(instrument.ok);
        CHECK(instrument.frozen == 0);
    }
}
