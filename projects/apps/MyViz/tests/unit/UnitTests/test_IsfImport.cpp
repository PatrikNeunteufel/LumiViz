/**
 ****************************************************************************************
 * @file   test_IsfImport.cpp
 * @brief  Tests fuer den ISF-Kopf-Leser des isfFilter-Knotens (S72)
 *
 * Zwei Ebenen, absichtlich beide:
 *   1. **Fixtures** (`asset/testdata/isf/`) — von Hand gegen die Spec
 *      geschrieben, decken jede Sorte ab (Filter, Generator, Uebergang, alle
 *      INPUT-Typen, .vs-Begleiter) und laufen IMMER, auch ohne Klon.
 *   2. **Korpus** gegen die echte Bibliothek unter `../ref/isf` (Vidvox,
 *      MIT) — dieselbe Bauart wie der AvsParser-Korpus. Fehlt der Ordner,
 *      meldet der Fall das und ist gruen; nichts Fremdes liegt im Repo.
 *
 * KERNZUSAGE seit dem Entscheid Patrik S72 (eigener Knoten statt Abbildung
 * auf den pixelFilter): **es wird nichts mehr abgelehnt und nichts mehr
 * umgeschrieben.** Sorte, Geometrie und Zahl der Bildquellen sind
 * Eigenschaften, keine Ausschlussgruende — genau das pruefen die Faelle hier.
 *
 * @author Patrik Neunteufel
 * @date   August 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/multieffect/ChainSerializer.hpp"
#include "visualizers/multieffect/IsfImport.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>

#include <algorithm>
#include <filesystem>
#include <variant>

using lumi::isf::IsfTyp;
using lumi::multieffect::ChainNode;
using lumi::multieffect::IsfFilterParams;

namespace {

/// Repo-Wurzel aus dem Pfad DIESER Datei (Muster: test_FieldInventory).
[[nodiscard]] QString repoRoot()
{
    std::filesystem::path p(__FILE__);
    for (int i = 0; i < 7; ++i) p = p.parent_path();
    return QString::fromStdString(p.string());
}

[[nodiscard]] QString liesDatei(const QString& pfad)
{
    QFile f(pfad);
    if (!f.open(QIODevice::ReadOnly)) return {};
    return QString::fromUtf8(f.readAll());
}

/// Die Fixtures im Repo — deterministisch, lizenzfrei, immer da.
[[nodiscard]] QString fixture(const QString& name)
{
    return liesDatei(repoRoot() + QStringLiteral("/asset/testdata/isf/") + name);
}

}  // namespace

TEST_CASE("IsfImport: Filter — ein Bild-Eingang, Code unveraendert")
{
    const QString quelle = fixture(QStringLiteral("posterize.fs"));
    REQUIRE_MESSAGE(!quelle.isEmpty(), "Fixture asset/testdata/isf/posterize.fs fehlt");

    const auto r = lumi::isf::importiereIsf(quelle, QStringLiteral("posterize"));
    REQUIRE_MESSAGE(r.ok, r.error.toStdString());

    // EIN Bild-Eingang => Filter.
    REQUIRE(r.bildInputs.size() == 1);
    CHECK(r.bildInputs.first() == QStringLiteral("inputImage"));
    CHECK(lumi::isf::sortenName(1).contains(QStringLiteral("Filter")));

    // Der Code wird DURCHGEREICHT, nicht uebersetzt: das ISF-Vokabular bleibt
    // stehen (der Wrapper stellt es bereit), der JSON-Kopf ist ab.
    CHECK(r.fragCode.contains(QStringLiteral("void main")));
    CHECK(r.fragCode.contains(QStringLiteral("IMG_THIS_PIXEL(inputImage)")));
    CHECK(r.fragCode.contains(QStringLiteral("gl_FragColor")));
    CHECK_FALSE(r.fragCode.contains(QStringLiteral("\"INPUTS\"")));
    CHECK_FALSE(r.fragCode.startsWith(QStringLiteral("/*")));
    // Kein .vs mitgegeben => leer, nicht etwa geraten.
    CHECK(r.vertexCode.isEmpty());

    // Regler mit Bereich; Herkunft aus CREDIT + Dateiname (ISF hat kein
    // Titelfeld) — Lizenz-Pflicht S72.
    REQUIRE(r.parameter.size() == 1);
    CHECK(r.parameter.first().name == QStringLiteral("levels"));
    CHECK(r.parameter.first().typ == IsfTyp::Float);
    CHECK(r.parameter.first().hatBereich);
    CHECK(r.parameter.first().min == doctest::Approx(2.0));
    CHECK(r.parameter.first().max == doctest::Approx(30.0));
    CHECK(r.herkunft.name == "posterize");
    CHECK(r.herkunft.author == "by zoidberg");
    CHECK(r.herkunft.license.empty());  // keine im Kopf => Hinweis im Report
}

TEST_CASE("IsfImport: Generator und Uebergang werden ANGENOMMEN, nicht abgelehnt")
{
    // Genau die Faelle, die die Vorfassung (Abbildung auf den pixelFilter)
    // zurueckweisen MUSSTE. Mit eigenem Knoten sind sie normale Sorten.
    const QString gen = fixture(QStringLiteral("generator.fs"));
    REQUIRE(!gen.isEmpty());
    const auto rg = lumi::isf::importiereIsf(gen, QStringLiteral("generator"));
    REQUIRE_MESSAGE(rg.ok, rg.error.toStdString());
    CHECK(rg.bildInputs.isEmpty());  // null Quellen = Generator
    CHECK(lumi::isf::sortenName(0).contains(QStringLiteral("Generator")));
    CHECK_FALSE(rg.fragCode.isEmpty());

    const QString ueb = fixture(QStringLiteral("uebergang.fs"));
    REQUIRE(!ueb.isEmpty());
    const auto ru = lumi::isf::importiereIsf(ueb, QStringLiteral("uebergang"));
    REQUIRE_MESSAGE(ru.ok, ru.error.toStdString());
    REQUIRE(ru.bildInputs.size() == 2);  // zwei Quellen = Uebergang
    CHECK(ru.bildInputs.at(0) == QStringLiteral("startImage"));
    CHECK(ru.bildInputs.at(1) == QStringLiteral("endImage"));
    CHECK(lumi::isf::sortenName(2).contains(QString::fromUtf8("Übergang")));
    // `progress` ist ein gewoehnlicher Regler, kein Sonderfall.
    const bool hatProgress =
        std::any_of(ru.parameter.begin(), ru.parameter.end(),
                    [](const lumi::isf::IsfParam& p) {
                        return p.name == QStringLiteral("progress");
                    });
    CHECK(hatProgress);

    // Die Sorte steht auch als Klartext im Report (Info-Zeile im Panel).
    const bool sorteGemeldet =
        std::any_of(ru.report.begin(), ru.report.end(), [](const QString& z) {
            return z.contains(QStringLiteral("Erkannt als"));
        });
    CHECK(sorteGemeldet);
}

TEST_CASE("IsfImport: nur was gar kein ISF ist, wird zurueckgewiesen")
{
    // Kein Kopf.
    const auto rk = lumi::isf::importiereIsf(
        QStringLiteral("void main() { gl_FragColor = vec4(1.0); }"));
    CHECK_FALSE(rk.ok);
    CHECK(rk.error.contains(QStringLiteral("JSON-Kopf")));

    // Kopf da, aber kaputtes JSON.
    const auto rj = lumi::isf::importiereIsf(
        QStringLiteral("/*{ \"INPUTS\": [ }*/\nvoid main(){}"));
    CHECK_FALSE(rj.ok);
    CHECK(rj.error.contains(QStringLiteral("JSON")));
}

TEST_CASE("IsfImport: alle INPUT-Typen kommen typrichtig an")
{
    const QString quelle = fixture(QStringLiteral("alle_typen.fs"));
    REQUIRE(!quelle.isEmpty());
    const auto r = lumi::isf::importiereIsf(quelle, QStringLiteral("alle_typen"));
    REQUIRE_MESSAGE(r.ok, r.error.toStdString());

    // Zwei Bild-Eingaenge => zwei Quell-Zeilen im Panel, KEIN Sonderfall.
    REQUIRE(r.bildInputs.size() == 2);
    CHECK(r.bildInputs.at(0) == QStringLiteral("inputImage"));
    CHECK(r.bildInputs.at(1) == QStringLiteral("zweitesBild"));

    const auto suche = [&r](const char* name) {
        return std::find_if(r.parameter.begin(), r.parameter.end(),
                            [name](const lumi::isf::IsfParam& p) {
                                return p.name == QLatin1String(name);
                            });
    };
    REQUIRE(suche("zahl") != r.parameter.end());
    CHECK(suche("zahl")->typ == IsfTyp::Float);
    REQUIRE(suche("schalter") != r.parameter.end());
    CHECK(suche("schalter")->typ == IsfTyp::Bool);
    CHECK(suche("schalter")->zahl == doctest::Approx(1.0));
    REQUIRE(suche("punkt") != r.parameter.end());
    CHECK(suche("punkt")->vektor[0] == doctest::Approx(0.25));
    REQUIRE(suche("tonung") != r.parameter.end());
    CHECK(suche("tonung")->vektor[3] == doctest::Approx(1.0));
    REQUIRE(suche("ausloeser") != r.parameter.end());
    CHECK(suche("ausloeser")->typ == IsfTyp::Event);

    // `long` mit LABELS/VALUES wird ein Klartext-Dropdown (Parameter-Baum).
    const auto modus = suche("modus");
    REQUIRE(modus != r.parameter.end());
    CHECK(modus->typ == IsfTyp::Long);
    CHECK(modus->auswahlLabels.size() == 3);
    CHECK(modus->auswahlLabels.at(1) == QStringLiteral("Mittel"));
    CHECK(modus->auswahlWerte.size() == 3);

    // Audio-Inputs kann der Knoten nicht bedienen — gemeldet, nicht still als
    // Regler ausgegeben.
    CHECK(std::any_of(r.report.begin(), r.report.end(), [](const QString& z) {
        return z.contains(QStringLiteral("Audio-Input"));
    }));
    CHECK_FALSE(std::any_of(r.parameter.begin(), r.parameter.end(),
                            [](const lumi::isf::IsfParam& p) {
                                return p.typ == IsfTyp::Audio ||
                                       p.typ == IsfTyp::AudioFft;
                            }));
}

TEST_CASE("IsfImport: der .vs bleibt ein EIGENER Shader")
{
    // Bis zum Entscheid S72 wurde der Vertex-Shader in die Filterfunktion
    // gefaltet — das erzwang Ablehnungen fuer alles Geometrische. Jetzt ist
    // er ein eigenes Feld und kommt unveraendert an.
    const QString fs = fixture(QStringLiteral("kanten.fs"));
    const QString vs = fixture(QStringLiteral("kanten.vs"));
    REQUIRE(!fs.isEmpty());
    REQUIRE(!vs.isEmpty());

    const auto r = lumi::isf::importiereIsf(fs, QStringLiteral("kanten"), vs);
    REQUIRE_MESSAGE(r.ok, r.error.toStdString());
    CHECK(r.vertexCode.contains(QStringLiteral("isf_vertShaderInit")));
    CHECK(r.vertexCode.contains(QStringLiteral("left_coord = ")));
    // Die Varying-Deklarationen bleiben, wo sie hingehoeren: `out` im
    // Vertex-, `in` im Fragment-Shader. Nichts wird gespiegelt oder entfernt.
    CHECK(r.vertexCode.contains(QStringLiteral("out vec2 left_coord;")));
    CHECK(r.fragCode.contains(QStringLiteral("in vec2 left_coord;")));

    // OHNE .vs kommt der Fragment-Teil trotzdem an (der Nutzer kann den
    // Vertex-Teil auch von Hand schreiben) — kein Abbruch mehr.
    const auto ohne = lumi::isf::importiereIsf(fs, QStringLiteral("kanten"));
    CHECK(ohne.ok);
    CHECK(ohne.vertexCode.isEmpty());
}

TEST_CASE("IsfImport: istIsf erkennt das Format streng")
{
    CHECK(lumi::isf::istIsf(QStringLiteral("/*{ \"INPUTS\": [] }*/\nvoid main(){}")));
    CHECK(lumi::isf::istIsf(QStringLiteral("\n\n  /*{}*/\nvoid main(){}")));
    // Ein gewoehnlicher Kommentar ist kein ISF-Kopf …
    CHECK_FALSE(lumi::isf::istIsf(
        QStringLiteral("/* nur ein Kommentar */\nvoid main(){}")));
    // … und kaputtes JSON auch nicht.
    CHECK_FALSE(lumi::isf::istIsf(QStringLiteral("/*{ \"A\": }*/\nvoid main(){}")));
    CHECK_FALSE(lumi::isf::istIsf(QStringLiteral("void main(){}")));
}

TEST_CASE("IsfImport: Quellen-Zeilen bekommen die AVS-Vorgabe")
{
    // AVS-Konvention (Entscheid Patrik S72): nichts ausgewaehlt = normale
    // Pipeline, also das Ketten-Bild.
    const auto q = lumi::isf::alsBildQuellen(
        {QStringLiteral("startImage"), QStringLiteral("endImage")});
    REQUIRE(q.size() == 2);
    CHECK(q[0].name == "startImage");
    CHECK(q[0].bindung == lumi::multieffect::isffilter::kQuelleKette);
    CHECK(q[1].bindung == lumi::multieffect::isffilter::kQuelleKette);
    CHECK(lumi::isf::alsBildQuellen({}).empty());  // Generator
}

TEST_CASE("ParamGruppe: Roundtrip mit Verschachtelung und allen Typen")
{
    using namespace lumi::multieffect;

    ParamGruppe wurzel;
    wurzel.label = "Wurzel";
    {
        ParamWert w;
        w.key = "zahl";
        w.label = "Zahl";
        w.typ = ParamTyp::Zahl;
        w.zahl = 0.5;
        w.min = 0.0;
        w.max = 1.0;
        w.hatBereich = true;
        wurzel.werte.push_back(w);
    }
    {
        ParamWert w;
        w.key = "ja";
        w.typ = ParamTyp::Bool;
        w.ja = true;
        wurzel.werte.push_back(w);
    }
    {
        ParamWert w;
        w.key = "modus";
        w.typ = ParamTyp::Auswahl;
        w.zahl = 2;
        w.auswahlLabels = {"Grob", "Mittel", "Fein"};
        w.auswahlWerte = {0, 1, 2};
        wurzel.werte.push_back(w);
    }
    {
        ParamWert w;
        w.key = "wort";
        w.typ = ParamTyp::Text;
        w.text = "hallo";
        wurzel.werte.push_back(w);
    }
    // Verschachtelung ist von Anfang an dabei (Entwurf Patrik) — genau
    // deshalb wird sie hier auch geprueft und nicht erst spaeter.
    ParamGruppe unter;
    unter.key = "farben";
    unter.label = "Farben";
    {
        ParamWert w;
        w.key = "tonung";
        w.typ = ParamTyp::Farbe;
        w.vektor = {{1.0, 0.5, 0.25, 1.0}};
        unter.werte.push_back(w);
    }
    {
        ParamWert w;
        w.key = "mitte";
        w.typ = ParamTyp::Punkt2D;
        w.vektor = {{0.25, 0.75, 0.0, 0.0}};
        unter.werte.push_back(w);
    }
    wurzel.gruppen.push_back(unter);

    IsfFilterParams p;
    p.fragCode = "void main() { gl_FragColor = vec4(1.0); }";
    p.parameter = wurzel;

    ChainNode node;
    node.params = p;
    QStringList report;
    const ChainNode back =
        lumi::multieffect::nodeFromJson(lumi::multieffect::nodeToJson(node), &report);
    const auto* q = std::get_if<IsfFilterParams>(&back.params);
    REQUIRE(q != nullptr);

    REQUIRE(q->parameter.werte.size() == 4);
    CHECK(q->parameter.werte[0].key == "zahl");
    CHECK(q->parameter.werte[0].zahl == doctest::Approx(0.5));
    CHECK(q->parameter.werte[0].hatBereich);
    CHECK(q->parameter.werte[1].typ == ParamTyp::Bool);
    CHECK(q->parameter.werte[1].ja);
    CHECK(q->parameter.werte[2].typ == ParamTyp::Auswahl);
    CHECK(q->parameter.werte[2].auswahlLabels.size() == 3);
    CHECK(q->parameter.werte[2].auswahlLabels[1] == "Mittel");
    CHECK(q->parameter.werte[3].text == "hallo");

    REQUIRE(q->parameter.gruppen.size() == 1);
    CHECK(q->parameter.gruppen[0].label == "Farben");
    REQUIRE(q->parameter.gruppen[0].werte.size() == 2);
    CHECK(q->parameter.gruppen[0].werte[0].vektor[1] == doctest::Approx(0.5));
    CHECK(q->parameter.gruppen[0].werte[1].vektor[0] == doctest::Approx(0.25));

    // Leere Ablage schreibt KEINEN Schluessel — deshalb taucht sie im
    // Feld-Inventar nicht auf, das einen Vorgabe-Knoten serialisiert.
    ChainNode leerNode;
    leerNode.params = IsfFilterParams{};
    CHECK_FALSE(lumi::multieffect::nodeToJson(leerNode).contains("parameter"));
}

TEST_CASE("ParamGruppe: der Leser klemmt kaputte Werte")
{
    using namespace lumi::multieffect;
    QJsonObject wert;
    wert["key"] = "x";
    wert["typ"] = 99;      // gibt es nicht
    wert["zahl"] = 500.0;  // weit ausserhalb
    wert["min"] = 10.0;
    wert["max"] = 1.0;  // verdreht
    QJsonArray werte;
    werte.append(wert);
    QJsonObject baum;
    baum["werte"] = werte;
    QJsonObject o;
    o["type"] = "isfFilter";
    o["parameter"] = baum;

    QStringList report;
    const ChainNode n = lumi::multieffect::nodeFromJson(o, &report);
    const auto* q = std::get_if<IsfFilterParams>(&n.params);
    REQUIRE(q != nullptr);
    REQUIRE(q->parameter.werte.size() == 1);
    const ParamWert& w = q->parameter.werte[0];
    // Typ auf die Aufzaehlung geklemmt (sonst waehlt das Panel einen Editor,
    // den es nicht gibt), Grenzen entdreht, Wert hineingeklemmt.
    CHECK(static_cast<int>(w.typ) <= static_cast<int>(ParamTyp::Auswahl));
    CHECK(w.min == doctest::Approx(1.0));
    CHECK(w.max == doctest::Approx(10.0));
    CHECK(w.zahl == doctest::Approx(10.0));
}

TEST_CASE("isfFilter: Knoten roundtrippt inkl. Quellen und Geometrie")
{
    using namespace lumi::multieffect;
    IsfFilterParams p;
    p.fragCode = "void main() { gl_FragColor = IMG_THIS_PIXEL(inputImage); }";
    p.vertexCode = "void main() { isf_vertShaderInit(); }";
    p.quellen = {{"startImage", isffilter::kQuelleKette},
                 {"endImage", isffilter::kQuelleBufferBasis + 2}};
    p.geometrie = 2;
    p.gridX = 128;
    p.gridY = 96;
    p.blend = 1;
    p.mixAmount = 0.5;
    p.herkunft.author = "Vidvox";

    ChainNode node;
    node.params = p;
    QStringList report;
    const ChainNode back =
        lumi::multieffect::nodeFromJson(lumi::multieffect::nodeToJson(node), &report);
    const auto* q = std::get_if<IsfFilterParams>(&back.params);
    REQUIRE(q != nullptr);
    CHECK(q->fragCode == p.fragCode);
    CHECK(q->vertexCode == p.vertexCode);
    REQUIRE(q->quellen.size() == 2);
    CHECK(q->quellen[0].name == "startImage");
    CHECK(q->quellen[0].bindung == isffilter::kQuelleKette);
    CHECK(q->quellen[1].name == "endImage");
    CHECK(q->quellen[1].bindung == isffilter::kQuelleBufferBasis + 2);
    CHECK(q->geometrie == 2);
    CHECK(q->gridX == 128);
    CHECK(q->blend == 1);
    CHECK(q->mixAmount == doctest::Approx(0.5));
    CHECK(q->herkunft.author == "Vidvox");

    // Klemmen: Geometrie, Gitter und Bindung duerfen nie ausserhalb landen —
    // der Renderer waehlt sonst eine Bauart oder einen Slot, den es nicht gibt.
    QJsonObject o;
    o["type"] = "isfFilter";
    o["geometrie"] = 99;
    o["gridX"] = 9999;
    o["gridY"] = 0;
    QJsonObject quelle;
    quelle["name"] = "x";
    quelle["bindung"] = 42;
    QJsonArray quellen;
    quellen.append(quelle);
    o["quellen"] = quellen;
    const ChainNode k = lumi::multieffect::nodeFromJson(o, &report);
    const auto* qk = std::get_if<IsfFilterParams>(&k.params);
    REQUIRE(qk != nullptr);
    CHECK(qk->geometrie == isffilter::kGeometrieMax);
    CHECK(qk->gridX == isffilter::kGridMax);
    CHECK(qk->gridY == isffilter::kGridMin);
    REQUIRE(qk->quellen.size() == 1);
    CHECK(qk->quellen[0].bindung == isffilter::kQuelleMax);
}

TEST_CASE("IsfImport: Referenz-Korpus (Vidvox ISF-Files)")
{
    // Muster des AvsParser-Korpus: die Bibliothek liegt AUSSERHALB des Repos
    // (`../ref/isf`, MIT-Lizenz). Fehlt sie, ist der Fall gruen und sagt das.
    const QString dir = repoRoot() + QStringLiteral("/../ref/isf/ISF");
    if (!QFileInfo(dir).isDir())
    {
        MESSAGE("ISF-Korpus nicht vorhanden (" << dir.toStdString()
                                               << ") — uebersprungen. Klon: "
                                                  "git clone --depth 1 "
                                                  "https://github.com/Vidvox/"
                                                  "ISF-Files.git ../ref/isf");
        return;
    }

    int gesamt = 0;
    int generator = 0;
    int filter = 0;
    int uebergang = 0;
    int mehrfach = 0;
    int mitVs = 0;
    QStringList fehler;
    const QFileInfoList dateien =
        QDir(dir).entryInfoList({QStringLiteral("*.fs")}, QDir::Files, QDir::Name);
    for (const QFileInfo& fi : dateien)
    {
        ++gesamt;
        const QString inhalt = liesDatei(fi.absoluteFilePath());
        const QString vsPfad = fi.absolutePath() + QLatin1Char('/') +
                               fi.completeBaseName() + QStringLiteral(".vs");
        const QString vs =
            QFileInfo::exists(vsPfad) ? liesDatei(vsPfad) : QString();
        if (!vs.isEmpty()) ++mitVs;

        const auto r = lumi::isf::importiereIsf(inhalt, fi.completeBaseName(), vs);
        // KERNZUSAGE: jede Datei der Bibliothek wird ANGENOMMEN. Ein Fehler
        // hier heisst, dass der Kopf-Leser eine echte ISF-Datei nicht versteht.
        if (!r.ok)
        {
            if (fehler.size() < 10)
                fehler << (fi.fileName() + QStringLiteral(": ") + r.error);
            continue;
        }
        switch (r.bildInputs.size())
        {
            case 0: ++generator; break;
            case 1: ++filter; break;
            case 2: ++uebergang; break;
            default: ++mehrfach; break;
        }
        // Der Kopf muss ab sein, der Code da.
        if (r.fragCode.isEmpty() || r.fragCode.contains(QStringLiteral("\"ISFVSN\"")))
        {
            if (fehler.size() < 10)
                fehler << (fi.fileName() + QStringLiteral(": Kopf nicht sauber getrennt"));
        }
        // Ein vorhandener .vs muss auch ankommen.
        if (!vs.isEmpty() && r.vertexCode.isEmpty())
        {
            if (fehler.size() < 10)
                fehler << (fi.fileName() + QStringLiteral(": .vs verschluckt"));
        }
    }

    MESSAGE("ISF-Korpus: " << gesamt << " Dateien angenommen — " << generator
                           << " Generatoren, " << filter << " Filter, " << uebergang
                           << " Uebergaenge, " << mehrfach << " mehrquellig; " << mitVs
                           << " mit .vs-Begleiter");
    for (const QString& f : fehler) MESSAGE("  " << f.toStdString());
    CHECK(gesamt > 0);
    CHECK(fehler.isEmpty());
}
