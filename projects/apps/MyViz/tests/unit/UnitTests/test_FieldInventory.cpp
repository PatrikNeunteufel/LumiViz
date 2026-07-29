/**
 ****************************************************************************************
 * @file   test_FieldInventory.cpp
 * @brief  Feld-Inventar aller Knotentypen (Knoten-Parameter-Konzept, Strang E/F)
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 *
 * @details
 * Strang E (§9) braucht die Antwort auf „welche Felder gibt es ueberhaupt?" —
 * und zwar **maschinell**, nicht von Hand abgeschrieben. Diese Datei erzeugt sie
 * generisch: sie besucht JEDE Alternative von `EffectParams`, konstruiert sie
 * auf Vorgabe, serialisiert sie mit `nodeToJson` und listet die Felder mit
 * `nodepresets::fieldNames()` — also mit genau derselben Quelle, aus der auch
 * die Voreinstellungen und die Feldauswahl im Panel leben. Ein neuer Knotentyp
 * oder ein neues Feld erscheint damit ohne eine Zeile Pflege hier.
 *
 * Ergebnis ist ein **Golden** unter `asset/calibration/fields/inventory.json`:
 * | Zweck | wer liest es |
 * |---|---|
 * | Soll-Liste der Feld-Presets (§9) | `make_field_probes.py` |
 * | Gate gegen Felder ohne Tooltip (§10) | spaeter, gleiche Datei |
 * | Waechter gegen stille Feld-Aenderungen | dieser Test |
 *
 * Neu schreiben (nach einer beabsichtigten Aenderung):
 * `LUMIVIZ_UPDATE_FIELD_INVENTORY=1` setzen und die Suite einmal laufen lassen.
 * Der Vergleich normalisiert Zeilenenden, damit `core.autocrlf` ihn nicht kippt.
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/multieffect/ChainSerializer.hpp"
#include "visualizers/multieffect/EffectChain.hpp"
#include "visualizers/multieffect/NodePresetStore.hpp"

#include <QByteArray>
#include <QFile>
#include <QtGlobal>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>

#include <algorithm>
#include <filesystem>
#include <string>
#include <utility>
#include <variant>
#include <vector>

using namespace lumi::multieffect;
namespace np = lumi::multieffect::nodepresets;

namespace
{

/// Repo-Wurzel aus dem Pfad DIESER Datei (wie test_ChainSerializer/test_NodePresetStore).
std::filesystem::path repoRoot()
{
    std::filesystem::path p(__FILE__);
    for (int i = 0; i < 7; ++i) p = p.parent_path();
    return p;
}

std::filesystem::path goldenPath()
{
    return repoRoot() / "asset" / "calibration" / "fields" / "inventory.json";
}

/**
 * Kurzname der JSON-Auspraegung. Der Preset-Generator entscheidet daran, WIE er
 * ein Feld verstellt — eine Zahl bekommt einen anderen Gegenwert als ein Text
 * (Skriptfeld) oder eine Liste (Farbtafel, Kernel).
 */
const char* valueKind(const QJsonValue& v)
{
    switch (v.type())
    {
    case QJsonValue::Bool:   return "bool";
    case QJsonValue::Double: return "zahl";
    case QJsonValue::String: return "text";
    case QJsonValue::Array:  return "liste";
    case QJsonValue::Object: return "objekt";
    case QJsonValue::Null:   return "null";
    default:                 return "unbekannt";
    }
}

/// Inventar EINES Typs: Schluessel, Anzeigename, Felder mit Vorgabewert und Art.
QJsonObject inventoryFor(const EffectParams& params)
{
    ChainNode tmp;
    tmp.params = params;
    const QJsonObject node = nodeToJson(tmp);

    QJsonArray fields;
    for (const QString& name : np::fieldNames(params))
    {
        const QJsonValue v = node.value(name);
        QJsonObject f;
        f[QStringLiteral("name")] = name;
        f[QStringLiteral("art")] = QLatin1String(valueKind(v));
        f[QStringLiteral("default")] = v;
        if (v.isArray()) f[QStringLiteral("laenge")] = v.toArray().size();
        fields.append(f);
    }

    QJsonObject out;
    out[QStringLiteral("typkey")] = effectTypeKey(params);
    out[QStringLiteral("name")] = QString::fromLatin1(effectTypeName(params));
    out[QStringLiteral("felder")] = fields;
    return out;
}

/**
 * Besucht jede Alternative der Variante. Der Faltungsausdruck ueber die
 * Index-Folge ist die einzige Stelle, die „alle Typen" bedeutet — es gibt hier
 * bewusst KEINE Aufzaehlung, die man zu pflegen vergessen koennte.
 */
template <std::size_t... I>
std::vector<QJsonObject> buildAll(std::index_sequence<I...>)
{
    std::vector<QJsonObject> v;
    v.reserve(sizeof...(I));
    (v.push_back(inventoryFor(EffectParams(std::in_place_index<I>))), ...);
    std::sort(v.begin(), v.end(), [](const QJsonObject& a, const QJsonObject& b) {
        return a.value(QStringLiteral("typkey")).toString()
               < b.value(QStringLiteral("typkey")).toString();
    });
    return v;
}

std::vector<QJsonObject> allTypes()
{
    return buildAll(std::make_index_sequence<std::variant_size_v<EffectParams>>{});
}

/// Das Inventar als Datei-Inhalt (sortiert, damit der Golden-Diff etwas bedeutet).
QByteArray renderInventory(const std::vector<QJsonObject>& types)
{
    QJsonArray arr;
    int fieldCount = 0;
    for (const QJsonObject& o : types)
    {
        arr.append(o);
        fieldCount += o.value(QStringLiteral("felder")).toArray().size();
    }

    QJsonObject summe;
    summe[QStringLiteral("typen")] = static_cast<int>(types.size());
    summe[QStringLiteral("felder")] = fieldCount;

    QJsonObject doc;
    doc[QStringLiteral("schema")] = 1;
    doc[QStringLiteral("summe")] = summe;
    doc[QStringLiteral("typen")] = arr;
    return QJsonDocument(doc).toJson(QJsonDocument::Indented);
}

/// Zeilenenden angleichen — sonst entscheidet `core.autocrlf` ueber gruen/rot.
QByteArray normalizeEol(QByteArray in)
{
    in.replace("\r\n", "\n");
    return in;
}

} // namespace

TEST_CASE("Feld-Inventar: jeder Knotentyp hat einen eigenen, nicht leeren Typschluessel")
{
    const std::vector<QJsonObject> types = allTypes();
    CHECK(types.size() == std::variant_size_v<EffectParams>);

    std::set<QString> seen;
    for (const QJsonObject& o : types)
    {
        const QString key = o.value(QStringLiteral("typkey")).toString();
        const QString name = o.value(QStringLiteral("name")).toString();
        CHECK_MESSAGE(!key.isEmpty(), "Typ ohne Schluessel: " << name.toStdString());
        // Ein doppelter Schluessel heisst: zwei Typen teilen sich eine Ablage —
        // Voreinstellungen und Feld-Presets landeten im selben Ordner.
        CHECK_MESSAGE(seen.insert(key).second,
                      "Typschluessel doppelt vergeben: " << key.toStdString()
                                                         << " (" << name.toStdString() << ")");
    }
}

TEST_CASE("Feld-Inventar: jedes Feld uebersteht den Roundtrip")
{
    // Struct -> JSON -> Struct -> JSON. Weil das erste JSON JEDES Feld nennt,
    // greift kein Alt-Format-Zweig; geprueft wird allein, ob Schreiber und
    // Leser dasselbe Feld kennen. Ein Feld, das nur der Schreiber kennt,
    // faellt hier auf — und ein solches waere im Panel einstellbar, ginge beim
    // Speichern aber verloren.
    //
    // Die VORGABEN sind nicht mehr Gegenstand eines Laufzeit-Tests: seit dem
    // SSOT-Umbau (S56) bezieht der Deserialisierer sie aus dem Struct
    // (`getInt(o, "x", p.x)`), es gibt also keine zweite Quelle mehr. Bewacht
    // wird das statisch im Ernter. Ein Test mit einem LEEREN JSON waere hier
    // sogar irrefuehrend: er loest die Alt-Format-Migrationen aus, und die
    // rechnen dann aus fehlenden Werten (Befund S56).
    for (const QJsonObject& typ : allTypes())
    {
        const QString key = typ.value(QStringLiteral("typkey")).toString();

        QJsonObject erst;
        erst[QStringLiteral("type")] = key;
        for (const QJsonValue& f : typ.value(QStringLiteral("felder")).toArray())
            erst[f.toObject().value(QStringLiteral("name")).toString()] =
                f.toObject().value(QStringLiteral("default"));

        QStringList report;
        const QJsonObject zweit = nodeToJson(nodeFromJson(erst, &report));

        for (const QJsonValue& f : typ.value(QStringLiteral("felder")).toArray())
        {
            const QString name = f.toObject().value(QStringLiteral("name")).toString();
            CHECK_MESSAGE(zweit.value(name) == erst.value(name),
                          "Feld ueberlebt den Roundtrip nicht: "
                              << key.toStdString() << "." << name.toStdString());
        }
    }
}

TEST_CASE("Feld-Inventar: das Golden ist aktuell")
{
    const QByteArray now = renderInventory(allTypes());
    const std::filesystem::path path = goldenPath();
    const std::string pathStr = path.string();

    if (qEnvironmentVariableIsSet("LUMIVIZ_UPDATE_FIELD_INVENTORY"))
    {
        std::filesystem::create_directories(path.parent_path());
        QFile out(QString::fromStdString(pathStr));
        REQUIRE(out.open(QIODevice::WriteOnly | QIODevice::Truncate));
        REQUIRE(out.write(now) == now.size());
        out.close();
        MESSAGE("Feld-Inventar neu geschrieben: " << pathStr);
    }

    QFile in(QString::fromStdString(pathStr));
    REQUIRE_MESSAGE(in.open(QIODevice::ReadOnly),
                    "Feld-Inventar fehlt — einmal mit LUMIVIZ_UPDATE_FIELD_INVENTORY=1 "
                    "laufen lassen: "
                        << pathStr);
    const QByteArray golden = in.readAll();
    in.close();

    CHECK_MESSAGE(normalizeEol(golden) == normalizeEol(now),
                  "Feld-Inventar weicht vom Golden ab. Ist die Aenderung gewollt, einmal "
                  "mit LUMIVIZ_UPDATE_FIELD_INVENTORY=1 laufen lassen und die Datei "
                  "mitcommitten: "
                      << pathStr);
}
