/**
 ****************************************************************************************
 * @file   test_NodePresetStore.cpp
 * @brief  Knoten-Voreinstellungen: Roundtrip, Typwaechter, Benutzer-vor-Asset
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 *
 * @details
 * Bewacht die Zusagen aus dem Knoten-Parameter-Konzept §3/§8.2:
 * - eine Voreinstellung traegt die `params` **einschliesslich der EEL-Formeln**,
 * - aber weder `children` noch `displayName`,
 * - eine Datei fremden Typs wird ABGELEHNT (nicht still zum Passthrough),
 * - bei Namensgleichheit gewinnt der Benutzer-Ordner.
 *
 * Der Store schreibt in ein Wegwerf-Verzeichnis (`setRootForTesting`), nicht in
 * AppData und nicht ins Repo.
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/modules/SuperscopeModule.hpp"
#include "visualizers/multieffect/ChainSerializer.hpp"
#include "visualizers/multieffect/NodePresetStore.hpp"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>

#include <filesystem>

using namespace lumi::multieffect;
namespace np = lumi::multieffect::nodepresets;

namespace
{

/// Repo-Wurzel aus dem Pfad DIESER Datei (wie test_ChainSerializer).
std::filesystem::path repoRoot()
{
    std::filesystem::path p(__FILE__);
    for (int i = 0; i < 7; ++i) p = p.parent_path();
    return p;
}

/// Setzt die Store-Wurzel auf ein Wegwerf-Verzeichnis und raeumt am Ende auf.
struct ScopedRoot
{
    QTemporaryDir dir;
    explicit ScopedRoot() { np::setRootForTesting(dir.path()); }
    ~ScopedRoot() { np::setRootForTesting(QString()); }
};

SuperScopeParams richScope()
{
    SuperScopeParams p;
    p.initCode = "n=800; t=0";
    p.frameCode = "t=t+0.01";
    p.beatCode = "t=t+0.5";
    p.pointCode = "x=cos(i*6.28)*0.7; y=sin(i*6.28)*0.7; red=1";
    p.pointCount = 800;
    p.colorCycleFrames = 42;
    p.colors = {0x112233, 0x445566};
    return p;
}

/// Legt eine Asset-Datei von Hand an (der Store schreibt nur in user/).
void writeAsset(const QString& root, const QString& typeKey, const QString& name,
                const QJsonObject& node)
{
    const QString dir = QDir(root).filePath(QStringLiteral("asset/") + typeKey);
    QDir().mkpath(dir);
    QJsonObject doc;
    doc[QStringLiteral("format")] = QStringLiteral("lumiviz-nodepreset");
    doc[QStringLiteral("formatVersion")] = 1;
    doc[QStringLiteral("node")] = node;
    QFile f(QDir(dir).filePath(name + QStringLiteral(".json")));
    REQUIRE(f.open(QIODevice::WriteOnly));
    f.write(QJsonDocument(doc).toJson());
    f.close();
}

} // namespace

TEST_CASE("NodePresetStore: Roundtrip traegt die EEL-Formeln mit")
{
    ScopedRoot root;
    const EffectParams saved{richScope()};

    QString written;
    REQUIRE(np::save(QStringLiteral("Kreis"), saved, {}, &written));
    CHECK(!written.isEmpty());

    const auto entries = np::list(effectTypeKey(saved));
    REQUIRE(entries.size() == 1);
    CHECK(entries[0].name == QStringLiteral("Kreis"));
    CHECK(entries[0].builtin == false);

    EffectParams loaded{SuperScopeParams{}};
    QStringList report;
    REQUIRE(np::load(entries[0].path, effectTypeKey(saved), loaded, &report));

    const auto* a = std::get_if<SuperScopeParams>(&saved);
    const auto* b = std::get_if<SuperScopeParams>(&loaded);
    REQUIRE(a != nullptr);
    REQUIRE(b != nullptr);
    CHECK(b->initCode == a->initCode);
    CHECK(b->frameCode == a->frameCode);
    CHECK(b->beatCode == a->beatCode);
    CHECK(b->pointCode == a->pointCode);  // §8.2: die Formeln gehoeren dazu
    CHECK(b->pointCount == a->pointCount);
    CHECK(b->colorCycleFrames == a->colorCycleFrames);
    REQUIRE(b->colors.size() == a->colors.size());
    CHECK(b->colors[0] == a->colors[0]);
    CHECK(b->colors[1] == a->colors[1]);
}

TEST_CASE("NodePresetStore: Rahmen des Knotens bleibt draussen")
{
    ScopedRoot root;
    ListParams list;
    list.clearEveryFrame = true;
    const EffectParams params{list};

    QString written;
    REQUIRE(np::save(QStringLiteral("Leer"), params, {}, &written));

    QFile f(written);
    REQUIRE(f.open(QIODevice::ReadOnly));
    const QJsonObject doc = QJsonDocument::fromJson(f.readAll()).object();
    f.close();

    const QJsonObject node = doc.value(QStringLiteral("node")).toObject();
    CHECK(node.contains(QStringLiteral("type")));
    CHECK_FALSE(node.contains(QStringLiteral("name")));
    CHECK_FALSE(node.contains(QStringLiteral("description")));
    CHECK_FALSE(node.contains(QStringLiteral("enabled")));
    CHECK_FALSE(node.contains(QStringLiteral("children")));  // §8.2
}

TEST_CASE("NodePresetStore: fremder Typ wird abgelehnt, nicht zum Passthrough")
{
    ScopedRoot root;
    QString written;
    REQUIRE(np::save(QStringLiteral("Kreis"), EffectParams{richScope()}, {}, &written));

    // Dieselbe Datei unter einem anderen erwarteten Typ laden: muss scheitern.
    EffectParams out{ListParams{}};
    QStringList report;
    CHECK_FALSE(np::load(written, QStringLiteral("movement"), out, &report));
    CHECK(!report.isEmpty());
    // Unveraendert — ein abgelehntes Preset darf nichts anfassen.
    CHECK(std::holds_alternative<ListParams>(out));
}

TEST_CASE("NodePresetStore: Teil-Preset laesst die uebrigen Felder stehen")
{
    ScopedRoot root;

    // Nur die Formeln sichern — Farbe, Breite und Blend NICHT (Vorgabe Patrik
    // S53: beim Speichern Felder abwaehlen; so arbeitet auch eine Figur).
    QString written;
    REQUIRE(np::save(QStringLiteral("NurFormeln"), EffectParams{richScope()},
                     {QStringLiteral("initCode"), QStringLiteral("frameCode"),
                      QStringLiteral("beatCode"), QStringLiteral("pointCode"),
                      QStringLiteral("pointCount")},
                     &written));

    // Die abgewaehlten Felder stehen gar nicht erst in der Datei.
    QFile f(written);
    REQUIRE(f.open(QIODevice::ReadOnly));
    const QJsonObject node =
        QJsonDocument::fromJson(f.readAll()).object().value(QStringLiteral("node")).toObject();
    f.close();
    CHECK(node.contains(QStringLiteral("pointCode")));
    CHECK_FALSE(node.contains(QStringLiteral("colors")));
    CHECK_FALSE(node.contains(QStringLiteral("lineWidth")));
    CHECK(node.contains(QStringLiteral("type")));  // der Typwaechter bleibt IMMER

    // Auf einen Knoten mit eigener Farbe/Breite laden: Formeln kommen, der Rest
    // bleibt unangetastet.
    SuperScopeParams host;
    host.colors = {0xABCDEF};
    host.lineWidth = 7.5f;
    host.pointCode = "x=0; y=0";
    EffectParams target{host};

    QStringList report;
    REQUIRE(np::load(written, QStringLiteral("superScope"), target, &report));
    const auto* t = std::get_if<SuperScopeParams>(&target);
    REQUIRE(t != nullptr);
    CHECK(t->pointCode == richScope().pointCode);   // uebernommen
    CHECK(t->pointCount == richScope().pointCount);
    REQUIRE(t->colors.size() == 1);
    CHECK(t->colors[0] == 0xABCDEFu);               // geblieben
    CHECK(t->lineWidth == doctest::Approx(7.5f));   // geblieben
}

TEST_CASE("NodePresetStore: Benutzer schlaegt Asset bei gleichem Namen")
{
    ScopedRoot root;
    const EffectParams params{richScope()};
    const QString key = effectTypeKey(params);

    QJsonObject assetNode;
    assetNode[QStringLiteral("type")] = key;
    writeAsset(root.dir.path(), key, QStringLiteral("Kreis"), assetNode);
    writeAsset(root.dir.path(), key, QStringLiteral("Nurasset"), assetNode);

    auto entries = np::list(key);
    REQUIRE(entries.size() == 2);
    CHECK(entries[0].builtin);
    CHECK(entries[1].builtin);

    // Gleichnamige Benutzer-Datei verdraengt das Asset, die andere bleibt.
    QString written;
    REQUIRE(np::save(QStringLiteral("Kreis"), params, {}, &written));
    entries = np::list(key);
    REQUIRE(entries.size() == 2);
    for (const np::Entry& e : entries)
    {
        if (e.name == QStringLiteral("Kreis")) CHECK_FALSE(e.builtin);
        if (e.name == QStringLiteral("Nurasset")) CHECK(e.builtin);
    }
}

TEST_CASE("NodePresetStore: Asset-Voreinstellungen sind nicht loeschbar")
{
    ScopedRoot root;
    const QString key = QStringLiteral("superScope");
    QJsonObject node;
    node[QStringLiteral("type")] = key;
    writeAsset(root.dir.path(), key, QStringLiteral("Mitgeliefert"), node);

    const auto entries = np::list(key);
    REQUIRE(entries.size() == 1);
    REQUIRE(entries[0].builtin);
    CHECK_FALSE(np::remove(entries[0]));
    CHECK(QFile::exists(entries[0].path));  // wirklich noch da
}

TEST_CASE("NodePresetStore: jede MITGELIEFERTE Voreinstellung ist ladbar")
{
    // Ohne Test-Wurzel: die echte Aufwaertssuche muss `asset/nodepresets` finden,
    // und JEDE Datei darin muss durch `load()` gehen. Sonst wandert eine kaputte
    // Beispieldatei unbemerkt ins Repo.
    const std::filesystem::path root = repoRoot() / "asset" / "nodepresets";
    REQUIRE(std::filesystem::exists(root / "README.md"));  // der Suchanker

    int checked = 0;
    for (const auto& typeDir : std::filesystem::directory_iterator(root))
    {
        if (!typeDir.is_directory()) continue;
        const QString typeKey =
            QString::fromStdString(typeDir.path().filename().string());
        const auto entries = np::list(typeKey);
        CAPTURE(typeKey.toStdString());
        CHECK(!entries.empty());  // ein Ordner ohne ladbare Datei ist ein Fehler
        for (const np::Entry& e : entries)
        {
            CAPTURE(e.name.toStdString());
            CHECK(e.builtin);  // im Repo gibt es nur Mitgeliefertes
            EffectParams out;
            QStringList report;
            CHECK(np::load(e.path, typeKey, out, &report));
            CHECK(effectTypeKey(out) == typeKey);  // Typ wirklich getroffen
            ++checked;
        }
    }
    CHECK(checked > 0);
}

TEST_CASE("NodePresetStore: die SuperScope-Figuren decken die Modul-Bibliothek")
{
    // Etappe 1b (S53): die Figuren liegen als Dateien, das Panel-Dropdown ist
    // weg. Das Modul bleibt aber Quelle fuer den Standalone-Visualizer und ist
    // Qt-frei — es kann die Dateien nicht selbst lesen. Dieser Waechter haelt
    // beide Seiten zusammen: JEDE Figur braucht ihre Datei, mit demselben EEL.
    using P = lumi::modules::SuperscopePreset;
    const std::vector<P> figures = {
        P::HorizontalScope, P::VerticalScope,    P::Circle,     P::Spiral,
        P::Lissajous,       P::Flower,           P::Star,       P::Starburst,
        P::Heart,           P::SpectrumBars,     P::CircularSpectrum,
        P::Butterfly,       P::Hypocycloid};

    const auto entries = np::list(QStringLiteral("superScope"));
    for (P fig : figures)
    {
        const QString name =
            QString::fromUtf8(lumi::modules::SuperscopeModule::presetName(fig));
        CAPTURE(name.toStdString());
        CHECK(name != QStringLiteral("Unknown"));  // Luecke in presetName (S53)

        const auto it = std::find_if(entries.cbegin(), entries.cend(),
                                     [&name](const np::Entry& e) { return e.name == name; });
        REQUIRE(it != entries.cend());

        lumi::modules::SuperscopeModule tmp;
        tmp.loadPresetCode(fig);

        // Basis absichtlich anders: was die Datei nicht traegt, bleibt stehen.
        SuperScopeParams host;
        host.lineWidth = 6.25f;
        EffectParams target{host};
        QStringList report;
        REQUIRE(np::load(it->path, QStringLiteral("superScope"), target, &report));

        const auto* t = std::get_if<SuperScopeParams>(&target);
        REQUIRE(t != nullptr);
        CHECK(t->initCode == tmp.initCode());
        CHECK(t->frameCode == tmp.frameCode());
        CHECK(t->beatCode == tmp.beatCode());
        CHECK(t->pointCode == tmp.pointCode());
        CHECK(t->pointCount == tmp.pointCount());
        CHECK(t->lineWidth == doctest::Approx(6.25f));  // Figur = Teil-Preset
    }
}

TEST_CASE("NodePresetStore: Dateinamen werden bereinigt")
{
    CHECK(np::sanitize(QStringLiteral("Spiral 2")) == QStringLiteral("Spiral 2"));
    CHECK(np::sanitize(QStringLiteral("a/b:c")) == QStringLiteral("a_b_c"));
    CHECK(np::sanitize(QStringLiteral("  Rand  ")) == QStringLiteral("Rand"));
    CHECK(np::sanitize(QStringLiteral("")).isEmpty());
    CHECK(np::sanitize(QStringLiteral("///")).isEmpty());  // nur Ersatzzeichen
}
