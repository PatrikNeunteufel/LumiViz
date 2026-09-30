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
#include "visualizers/milkdrop/MilkdropSerializer.hpp"

#include <EelTranspiler.hpp>

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>

#include <cstdlib>
#include <filesystem>
#include <vector>

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
        // Milkdrop-Elemente und -Sektionen (S83) sind keine Knoten: JSON-Ebene,
        // jedes Feld muss der Serializer kennen (sonst ist es ein stiller
        // Tippfehler), und jeder Code-Slot muss als MilkDrop-EEL uebersetzbar sein.
        QStringList bekannt;
        if (typeKey == QLatin1String(np::kMilkWave))
            bekannt = lumi::milkdrop::waveToJson({}).keys();
        else if (typeKey == QLatin1String(np::kMilkShape))
            bekannt = lumi::milkdrop::shapeToJson({}).keys();
        else if (typeKey == QLatin1String(np::kMilkSprite))
            bekannt = lumi::milkdrop::spriteToJson({}).keys();
        else if (typeKey == QLatin1String(np::kMilkCode))
            bekannt = {QStringLiteral("perFrameInit"), QStringLiteral("perFrame"),
                       QStringLiteral("perPixel")};
        else if (typeKey == QLatin1String(np::kMilkParams))
        {
            bekannt = lumi::milkdrop::presetToJson({})
                          .value(QStringLiteral("preset"))
                          .toObject()
                          .keys();
            for (const char* k : {"perFrameInit", "perFrame", "perPixel", "warpShader",
                                  "compShader", "waves", "shapes", "sprites", "name",
                                  "generation", "psVersion"})
                bekannt.removeAll(QLatin1String(k));
        }
        const bool element = !bekannt.isEmpty();
        for (const np::Entry& e : entries)
        {
            CAPTURE(e.name.toStdString());
            CHECK(e.builtin);  // im Repo gibt es nur Mitgeliefertes
            if (element)
            {
                QJsonObject felder;
                QStringList report;
                REQUIRE(np::loadJson(e.path, typeKey, felder, &report));
                CHECK_FALSE(felder.contains(QStringLiteral("type")));
                for (const QString& k : felder.keys())
                    CHECK_MESSAGE(bekannt.contains(k), "unbekanntes Feld: " << k.toStdString());
                for (const char* slot : {"initCode", "frameCode", "pointCode", "code",
                                         "perFrameInit", "perFrame", "perPixel"})
                {
                    const std::string code =
                        felder.value(QLatin1String(slot)).toString().toStdString();
                    const auto r = lumi::eel::transpile(code, lumi::eel::Dialect::Milkdrop);
                    CHECK_MESSAGE(r.ok, slot << ": " << r.error);
                }
                ++checked;
                continue;
            }
            EffectParams out;
            QStringList report;
            CHECK(np::load(e.path, typeKey, out, &report));
            CHECK(effectTypeKey(out) == typeKey);  // Typ wirklich getroffen
            ++checked;
        }
    }
    CHECK(checked > 0);
}

TEST_CASE("NodePresetStore: Element-Preset (Custom Wave) — Rahmen bleibt, Teil-Preset mischt")
{
    // S83, Entscheid Patrik E3a: Waves/Shapes/Sprites tragen Voreinstellungen
    // ueber die JSON-Ebene. `index`/`enabled` sind der Rahmen des Elements
    // und gehoeren nicht in die Datei.
    ScopedRoot root;
    lumi::milkdrop::WaveState w;
    w.index = 3;
    w.enabled = true;
    w.r = 0.25;
    w.samples = 256;
    w.pointCode = "x = sample; y = 0.5;";
    QJsonObject felder = lumi::milkdrop::waveToJson(w);
    felder.remove(QStringLiteral("index"));
    felder.remove(QStringLiteral("enabled"));

    QString written;
    REQUIRE(np::saveJson(QStringLiteral("Linie"), QString::fromLatin1(np::kMilkWave), felder,
                         {QStringLiteral("samples"), QStringLiteral("pointCode")}, &written));
    const auto entries = np::list(QString::fromLatin1(np::kMilkWave));
    REQUIRE(entries.size() == 1);

    // Laden auf eine ANDERE Wave: nur samples + pointCode wandern, Farbe bleibt.
    lumi::milkdrop::WaveState ziel;
    ziel.index = 7;
    ziel.enabled = false;
    ziel.r = 0.9;
    QJsonObject z = lumi::milkdrop::waveToJson(ziel);
    REQUIRE(np::loadJson(entries[0].path, QString::fromLatin1(np::kMilkWave), z, nullptr));
    const lumi::milkdrop::WaveState out = lumi::milkdrop::waveFromJson(z);
    CHECK(out.index == 7);
    CHECK_FALSE(out.enabled);
    CHECK(out.r == doctest::Approx(0.9));
    CHECK(out.samples == 256);
    CHECK(out.pointCode == "x = sample; y = 0.5;");

    // Falscher Typ wird abgelehnt, `inOut` bleibt unberuehrt.
    QJsonObject shape = lumi::milkdrop::shapeToJson({});
    const QJsonObject vorher = shape;
    CHECK_FALSE(np::loadJson(entries[0].path, QString::fromLatin1(np::kMilkShape), shape,
                             nullptr));
    CHECK(shape == vorher);
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

namespace
{

/// Die Liste bekannter deutscher Namen/Woerter (S83) — neben dieser Datei.
struct DeutscheWoerter
{
    QSet<QString> namen;    ///< alte Preset-/Beispielnamen, klein geschrieben
    QSet<QString> woerter;  ///< deutsche Woerter ohne englische Doppelgaenger
};

DeutscheWoerter ladeDeutscheWoerter()
{
    DeutscheWoerter d;
    const std::filesystem::path pfad =
        std::filesystem::path(__FILE__).parent_path() / "deutsche_woerter.txt";
    QFile f(QString::fromStdWString(pfad.wstring()));
    REQUIRE(f.open(QIODevice::ReadOnly | QIODevice::Text));
    QSet<QString>* ziel = nullptr;
    for (const QString& roh : QString::fromUtf8(f.readAll()).split(QLatin1Char('\n')))
    {
        const QString z = roh.trimmed();
        if (z.isEmpty() || z.startsWith(QLatin1Char('#'))) continue;
        if (z == QLatin1String("[namen]")) ziel = &d.namen;
        else if (z == QLatin1String("[woerter]")) ziel = &d.woerter;
        else if (ziel != nullptr) ziel->insert(z.toLower());
    }
    return d;
}

/// Kommentar-Anteile eines Codes: alles hinter `//` und in `/* … */`.
QString kommentareIn(const QString& code)
{
    QString out;
    static const QRegularExpression kBlock(QStringLiteral("/\\*.*?\\*/"),
                                           QRegularExpression::DotMatchesEverythingOption);
    for (auto it = kBlock.globalMatch(code); it.hasNext();)
        out += it.next().captured(0) + QLatin1Char('\n');
    for (const QString& zeile : code.split(QLatin1Char('\n')))
    {
        const int pos = zeile.indexOf(QStringLiteral("//"));
        if (pos >= 0) out += zeile.mid(pos + 2) + QLatin1Char('\n');
    }
    return out;
}

/// Menschenlesbarer Text einer Preset-/Kettendatei: `name`/`description`
/// ganz, alle anderen Strings nur mit ihren Kommentaren.
void sammleText(const QJsonValue& v, const QString& schluessel, QString& out)
{
    if (v.isObject())
    {
        const QJsonObject o = v.toObject();
        for (auto it = o.constBegin(); it != o.constEnd(); ++it)
            sammleText(it.value(), it.key(), out);
    }
    else if (v.isArray())
    {
        for (const QJsonValue& e : v.toArray()) sammleText(e, schluessel, out);
    }
    else if (v.isString())
    {
        const QString s = v.toString();
        out += (schluessel == QLatin1String("name") || schluessel == QLatin1String("description"))
                   ? s
                   : kommentareIn(s);
        out += QLatin1Char('\n');
    }
}

/// Deutsche Treffer in einem Text: Umlaute/ß und Woerter der Liste.
QStringList deutschIn(const QString& text, const DeutscheWoerter& d)
{
    QStringList treffer;
    for (const QChar c : text)
        if (QStringLiteral("äöüÄÖÜß").contains(c)) treffer << QString(c);
    static const QRegularExpression kWort(QStringLiteral("[A-Za-zÄÖÜäöüß]+"));
    for (auto it = kWort.globalMatch(text); it.hasNext();)
    {
        const QString w = it.next().captured(0).toLower();
        if (d.woerter.contains(w)) treffer << w;
    }
    treffer.removeDuplicates();
    return treffer;
}

}  // namespace

TEST_CASE("Mitgelieferte Namen und Texte sind englisch (S83, Liste deutscher Woerter)")
{
    // Entscheid Patrik S83: alle mitgelieferten Presets, Beispiele und
    // Shader-Vorlagen heissen englisch und sind englisch kommentiert. Die
    // Sprache selbst kann ein Test nicht erkennen — dieser prueft gegen eine
    // Liste bekannter deutscher Namen und Woerter (deutsche_woerter.txt) und
    // gegen Umlaute. Neue deutsche Woerter, die auffallen, gehoeren in die Liste.
    const DeutscheWoerter d = ladeDeutscheWoerter();
    REQUIRE(d.namen.size() > 100);
    REQUIRE(d.woerter.size() > 100);

    const std::filesystem::path asset = repoRoot() / "asset";
    int dateien = 0;
    int texte = 0;
    for (const char* ordner : {"nodepresets", "examples", "shadertemplates"})
    {
        REQUIRE(std::filesystem::exists(asset / ordner));
        for (const auto& e : std::filesystem::recursive_directory_iterator(asset / ordner))
        {
            if (!e.is_regular_file() || e.path().filename() == "README.md") continue;
            const QString datei = QString::fromStdWString(e.path().filename().wstring());
            const QString stamm = QString::fromStdWString(e.path().stem().wstring());
            CAPTURE(datei.toStdString());
            // (1) Dateiname
            CHECK_MESSAGE(!d.namen.contains(stamm.toLower()), "alter deutscher Name");
            const QString nameOhneTyp = stamm.section(QStringLiteral(" - "), -1);
            CHECK_MESSAGE(!d.namen.contains(nameOhneTyp.toLower()), "alter deutscher Name");
            const QStringList imNamen = deutschIn(stamm, d);
            CHECK_MESSAGE(imNamen.isEmpty(), "deutsch im Dateinamen: "
                                                 << imNamen.join(QLatin1Char(',')).toStdString());
            ++dateien;

            // (2)/(3) Inhalt: Kommentare, Beschreibungen, Knotennamen
            const QString ext = QString::fromStdWString(e.path().extension().wstring());
            if (ext == QLatin1String(".avs")) continue;  // binaer
            QFile f(QString::fromStdWString(e.path().wstring()));
            REQUIRE(f.open(QIODevice::ReadOnly));
            const QByteArray roh = f.readAll();
            QString text;
            if (ext == QLatin1String(".json") || ext == QLatin1String(".lvfx"))
            {
                const QJsonDocument doc = QJsonDocument::fromJson(roh);
                REQUIRE(doc.isObject());
                sammleText(doc.object(), QString(), text);
            }
            else
            {
                text = kommentareIn(QString::fromUtf8(roh));
            }
            const QStringList imText = deutschIn(text, d);
            CHECK_MESSAGE(imText.isEmpty(), "deutsch im Text: "
                                                << imText.join(QLatin1Char(',')).toStdString());
            ++texte;
        }
    }
    CHECK(dateien > 300);  // Schutz gegen einen Test, der still nichts prueft
    CHECK(texte > 150);
}

namespace
{

/// Die Zeichenketten-Literale eines C++-Quelltexts, Kommentare uebersprungen.
/// `vorher` = der Code direkt vor dem Literal (fuer die tr()-Erkennung).
struct Literal
{
    QString text;
    QString vorher;
    int zeile = 0;
};

std::vector<Literal> literaleIn(const QString& s)
{
    std::vector<Literal> out;
    int zeile = 1;
    for (int i = 0; i < s.size(); ++i)
    {
        const QChar c = s[i];
        if (c == QLatin1Char('\n')) ++zeile;
        if (c == QLatin1Char('/') && i + 1 < s.size() && s[i + 1] == QLatin1Char('/'))
        {
            while (i < s.size() && s[i] != QLatin1Char('\n')) ++i;
            --i;  // das \n zaehlt die naechste Runde
            continue;
        }
        if (c == QLatin1Char('/') && i + 1 < s.size() && s[i + 1] == QLatin1Char('*'))
        {
            const int ende = s.indexOf(QStringLiteral("*/"), i + 2);
            const int bis = ende < 0 ? s.size() : ende + 2;
            zeile += s.mid(i, bis - i).count(QLatin1Char('\n'));
            i = bis - 1;
            continue;
        }
        if (c == QLatin1Char('\'') && i + 2 < s.size())
        {
            // Zeichenliteral ('"', '\'') ueberspringen
            i += (s[i + 1] == QLatin1Char('\\')) ? 3 : 2;
            continue;
        }
        if (c == QLatin1Char('"'))
        {
            // Rohstring R"(...)" — Inhalt ist Code (GLSL-Wrapper), kein UI-Text
            if (i > 0 && s[i - 1] == QLatin1Char('R'))
            {
                const int ende = s.indexOf(QStringLiteral(")\""), i);
                const int bis = ende < 0 ? s.size() : ende + 2;
                zeile += s.mid(i, bis - i).count(QLatin1Char('\n'));
                i = bis - 1;
                continue;
            }
            Literal l;
            l.zeile = zeile;
            l.vorher = s.mid(std::max(0, i - 60), std::min(60, i));
            int j = i + 1;
            while (j < s.size() && s[j] != QLatin1Char('"'))
            {
                if (s[j] == QLatin1Char('\\')) ++j;
                ++j;
            }
            l.text = s.mid(i + 1, j - i - 1);
            out.push_back(l);
            i = j;
        }
    }
    return out;
}

}  // namespace

TEST_CASE("Oberflaechentexte sind englisch (S83, tr() + Referenzseiten)")
{
    // Entscheid Patrik S83: die Oberflaeche ist englisch. Geprueft werden alle
    // tr()-Texte der UI-Quellen und die HTML-Referenzseiten (ⓘ) gegen die
    // Liste bekannter deutscher Woerter und gegen Umlaute. Nicht geprueft:
    // Kommentare (bleiben deutsch, Projekt-Konvention), Log-Meldungen und die
    // Feld-Tooltips (FieldDocs.cpp — erzeugt aus EffectChain.hpp).
    const DeutscheWoerter d = ladeDeutscheWoerter();
    const std::filesystem::path app = repoRoot() / "projects" / "apps" / "LumiViz";
    static const QRegularExpression kTrVorher(QStringLiteral("\\btr\\(\\s*(\"[^\"]*\"\\s*)*$"));
    static const QRegularExpression kHtml(QStringLiteral("<(p|h2|h3|td|th|tr|b|code|table)[ >]"));
    int geprueft = 0;
    for (const char* ordner : {"src/UI", "include/UI"})
    {
        for (const auto& e : std::filesystem::recursive_directory_iterator(app / ordner))
        {
            const auto ext = e.path().extension();
            if (!e.is_regular_file() || (ext != ".cpp" && ext != ".hpp")) continue;
            if (e.path().filename() == "FieldDocs.cpp") continue;  // eigener Punkt (§8)
            QFile f(QString::fromStdWString(e.path().wstring()));
            REQUIRE(f.open(QIODevice::ReadOnly));
            const QString s = QString::fromUtf8(f.readAll());
            const QString datei = QString::fromStdWString(e.path().filename().wstring());
            // Referenzseiten bestehen aus vielen Teil-Literalen: im Umfeld eines
            // HTML-Literals zaehlen die Nachbarn mit (dieselbe Funktion).
            const std::vector<Literal> lits = literaleIn(s);
            for (std::size_t k = 0; k < lits.size(); ++k)
            {
                const Literal& l = lits[k];
                const bool istTr = kTrVorher.match(l.vorher).hasMatch();
                bool istHtml = kHtml.match(l.text).hasMatch();
                for (std::size_t n = (k >= 3 ? k - 3 : 0); !istHtml && n < std::min(lits.size(), k + 4); ++n)
                    istHtml = kHtml.match(lits[n].text).hasMatch() &&
                              std::abs(lits[n].zeile - l.zeile) <= 3;
                if (!istTr && !istHtml) continue;
                // Log-Meldungen sind keine Oberflaeche, auch neben einer Referenzseite
                static const QRegularExpression kLog(
                    QStringLiteral("\\b(log(Info|Warning|Error|Debug)|f?printf)\\s*\\([^;]*$"));
                if (!istTr && kLog.match(l.vorher).hasMatch()) continue;
                const QStringList treffer = deutschIn(l.text, d);
                CHECK_MESSAGE(treffer.isEmpty(),
                              datei.toStdString() << ":" << l.zeile << " deutsch: "
                                                  << treffer.join(QLatin1Char(',')).toStdString()
                                                  << " in '" << l.text.left(80).toStdString() << "'");
                ++geprueft;
            }
        }
    }
    CHECK(geprueft > 1000);  // Schutz gegen einen Test, der still nichts prueft
}

TEST_CASE("NodePresetStore: Dateinamen werden bereinigt")
{
    CHECK(np::sanitize(QStringLiteral("Spiral 2")) == QStringLiteral("Spiral 2"));
    CHECK(np::sanitize(QStringLiteral("a/b:c")) == QStringLiteral("a_b_c"));
    CHECK(np::sanitize(QStringLiteral("  Rand  ")) == QStringLiteral("Rand"));
    CHECK(np::sanitize(QStringLiteral("")).isEmpty());
    CHECK(np::sanitize(QStringLiteral("///")).isEmpty());  // nur Ersatzzeichen
}
