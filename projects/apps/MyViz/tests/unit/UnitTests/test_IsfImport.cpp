/**
 ****************************************************************************************
 * @file   test_IsfImport.cpp
 * @brief  Tests fuer den ISF-Import auf den pixelFilter-Vertrag (Stufe 1, S72)
 *
 * Zwei Ebenen, absichtlich beide:
 *   1. **Fixtures** (`asset/testdata/isf/`) — von Hand gegen die Spec
 *      geschrieben, decken jeden Zweig ab (FX, Generator, Uebergang, alle
 *      INPUT-Typen, .vs-Begleiter) und laufen IMMER, auch ohne Klon.
 *   2. **Korpus** gegen die echte Bibliothek unter `../ref/isf` (Vidvox,
 *      MIT) — dieselbe Bauart wie der AvsParser-Korpus. Fehlt der Ordner,
 *      meldet der Fall das und ist gruen; nichts Fremdes liegt im Repo.
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

TEST_CASE("IsfImport: FX-Filter wird auf den farbe()-Vertrag uebersetzt")
{
    const QString quelle = fixture(QStringLiteral("posterize.fs"));
    REQUIRE_MESSAGE(!quelle.isEmpty(), "Fixture asset/testdata/isf/posterize.fs fehlt");

    const auto r = lumi::isf::importiereIsf(quelle, QStringLiteral("posterize"));
    REQUIRE_MESSAGE(r.ok, r.error.toStdString());

    // Der Vertrag steht drin, das ISF-Vokabular ist restlos weg.
    CHECK(r.code.contains(QStringLiteral("vec4 farbe(vec2 uv, vec4 src)")));
    CHECK_FALSE(r.code.contains(QStringLiteral("void main")));
    CHECK_FALSE(r.code.contains(QStringLiteral("IMG_")));
    CHECK_FALSE(r.code.contains(QStringLiteral("gl_FragColor")));
    CHECK_FALSE(r.code.contains(QStringLiteral("isf_FragNormCoord")));
    CHECK_FALSE(r.code.contains(QStringLiteral("RENDERSIZE")));
    // WAECHTER S70: `filter` ist in GLSL ein RESERVIERTES Wort — es darf nie
    // als Bezeichner im erzeugten Code stehen (AMD lehnt den Shader sonst ab).
    CHECK_FALSE(r.code.contains(QRegularExpression(QStringLiteral("\\bfilter\\b"))));

    // IMG_THIS_PIXEL(inputImage) ist unser `src`.
    CHECK(r.code.contains(QStringLiteral("src")));
    // Der Parameter steht als const-Block mit seiner Vorgabe da.
    CHECK(r.code.contains(QString::fromLatin1(lumi::isf::kParamBlockStart)));
    CHECK(r.code.contains(QStringLiteral("const float levels = 30.0;")));

    // Herkunft: CREDIT wird zum Autor, der Dateiname zum Titel (ISF hat kein
    // Titelfeld) — Lizenz-Pflicht S72.
    REQUIRE(r.parameter.size() == 1);
    CHECK(r.parameter.first().name == QStringLiteral("levels"));
    CHECK(r.parameter.first().typ == IsfTyp::Float);
    CHECK(r.parameter.first().hatBereich);
    CHECK(r.parameter.first().min == doctest::Approx(2.0));
    CHECK(r.parameter.first().max == doctest::Approx(30.0));
    CHECK(r.herkunft.name == "posterize");
    CHECK(r.herkunft.author == "by zoidberg");
    // Keine Lizenz im Kopf => Hinweis, nicht stilles Schweigen.
    CHECK(r.herkunft.license.empty());
    CHECK(!r.report.isEmpty());
}

TEST_CASE("IsfImport: Generator und Uebergang werden ABGELEHNT, nicht halb uebersetzt")
{
    const QString gen = fixture(QStringLiteral("generator.fs"));
    REQUIRE(!gen.isEmpty());
    const auto rg = lumi::isf::importiereIsf(gen, QStringLiteral("generator"));
    CHECK_FALSE(rg.ok);
    CHECK(rg.error.contains(QStringLiteral("GENERATOR")));
    CHECK(rg.code.isEmpty());

    const QString ueb = fixture(QStringLiteral("uebergang.fs"));
    REQUIRE(!ueb.isEmpty());
    const auto ru = lumi::isf::importiereIsf(ueb, QStringLiteral("uebergang"));
    CHECK_FALSE(ru.ok);
    CHECK(ru.error.contains(QString::fromUtf8("ÜBERGANG")));

    // Kein ISF-Kopf ueberhaupt.
    const auto rk = lumi::isf::importiereIsf(
        QStringLiteral("void main() { gl_FragColor = vec4(1.0); }"));
    CHECK_FALSE(rk.ok);
    CHECK(rk.error.contains(QStringLiteral("JSON-Kopf")));

    // Kopf da, aber kaputtes JSON.
    const auto rj = lumi::isf::importiereIsf(
        QStringLiteral("/*{ \"INPUTS\": [ }*/\nvoid main(){}"));
    CHECK_FALSE(rj.ok);
}

TEST_CASE("IsfImport: alle INPUT-Typen landen typrichtig im const-Block")
{
    const QString quelle = fixture(QStringLiteral("alle_typen.fs"));
    REQUIRE(!quelle.isEmpty());
    const auto r = lumi::isf::importiereIsf(quelle, QStringLiteral("alle_typen"));
    REQUIRE_MESSAGE(r.ok, r.error.toStdString());

    CHECK(r.code.contains(QStringLiteral("const float zahl = 0.5;")));
    CHECK(r.code.contains(QStringLiteral("const bool schalter = true;")));
    CHECK(r.code.contains(QStringLiteral("const int modus = 2;")));
    CHECK(r.code.contains(QStringLiteral("const vec2 punkt = vec2(0.25, 0.75);")));
    CHECK(r.code.contains(QStringLiteral("const vec4 tonung = vec4(1.0, 0.5, 0.0, 1.0);")));

    // `long` mit LABELS/VALUES wird spaeter ein Klartext-Dropdown (Stufe 3).
    const auto modus = std::find_if(
        r.parameter.begin(), r.parameter.end(),
        [](const lumi::isf::IsfParam& p) { return p.name == QStringLiteral("modus"); });
    REQUIRE(modus != r.parameter.end());
    CHECK(modus->typ == IsfTyp::Long);
    CHECK(modus->auswahlLabels.size() == 3);
    CHECK(modus->auswahlLabels.at(1) == QStringLiteral("Mittel"));
    CHECK(modus->auswahlWerte.size() == 3);

    // Audio-Inputs kann der Filter-Knoten nicht bedienen — gemeldet, nicht
    // still als Regler ausgegeben.
    const bool audioGemeldet =
        std::any_of(r.report.begin(), r.report.end(), [](const QString& z) {
            return z.contains(QStringLiteral("Audio-Input"));
        });
    CHECK(audioGemeldet);
    const bool audioAlsRegler =
        std::any_of(r.parameter.begin(), r.parameter.end(),
                    [](const lumi::isf::IsfParam& p) {
                        return p.typ == IsfTyp::Audio || p.typ == IsfTyp::AudioFft;
                    });
    CHECK_FALSE(audioAlsRegler);

    // Ein zweiter Bild-Input bleibt schwarz — und wird NICHT auf uTex
    // umgebogen, sonst saehe der Shader das falsche Bild.
    CHECK(r.code.contains(QStringLiteral("vec4(0.0)")));
}

TEST_CASE("IsfImport: der .vs-Begleiter wandert in farbe() hinein")
{
    // 38 der 327 Vidvox-Dateien rechnen ihre Nachbar-Koordinaten im
    // Vertex-Shader vor. Ohne Uebersetzung LINKT der Fragment-Shader nicht.
    const QString fs = fixture(QStringLiteral("kanten.fs"));
    const QString vs = fixture(QStringLiteral("kanten.vs"));
    REQUIRE(!fs.isEmpty());
    REQUIRE(!vs.isEmpty());

    // OHNE .vs: klarer Abbruch statt Linkerfehler tief im Treiber.
    const auto ohne = lumi::isf::importiereIsf(fs, QStringLiteral("kanten"));
    CHECK_FALSE(ohne.ok);
    CHECK(ohne.error.contains(QStringLiteral("left_coord")));
    CHECK(ohne.error.contains(QStringLiteral(".vs")));

    // MIT .vs: die Varyings werden lokale Variablen, die Rechnung laeuft mit.
    const auto mit = lumi::isf::importiereIsf(fs, QStringLiteral("kanten"), vs);
    REQUIRE_MESSAGE(mit.ok, mit.error.toStdString());
    CHECK(mit.code.contains(QStringLiteral("vec2 left_coord;")));
    CHECK(mit.code.contains(QStringLiteral("left_coord = ")));
    // Die Deklaration als Shader-EINGANG ist weg (die gaebe es bei uns nicht).
    CHECK_FALSE(mit.code.contains(QStringLiteral("in vec2 left_coord;")));
    CHECK_FALSE(mit.code.contains(QStringLiteral("varying vec2 left_coord;")));
    // isf_vertShaderInit() erledigt bei uns der geteilte Quad-Vertex-Shader.
    CHECK_FALSE(mit.code.contains(QStringLiteral("isf_vertShaderInit")));
    // Die Varying-Rechnung steht INNERHALB von farbe(), vor dem .fs-Rumpf.
    CHECK(mit.code.indexOf(QStringLiteral("vec4 farbe(")) <
          mit.code.indexOf(QStringLiteral("left_coord = ")));
}

TEST_CASE("IsfImport: ein vorzeitiges return; ueberlebt die Uebersetzung")
{
    // In einer vec4-Funktion ist ein nacktes `return;` ein Compilerfehler —
    // und genau so schreiben viele ISF-Filter ihren Passthrough-Zweig.
    // Deshalb wird gl_FragColor eine LOKALE Variable, nicht der Rueckgabewert.
    const QString quelle = QStringLiteral(R"(/*{
    "CREDIT": "test",
    "INPUTS": [ { "NAME": "inputImage", "TYPE": "image" } ]
}*/
void main() {
    gl_FragColor = IMG_THIS_PIXEL(inputImage);
    if (gl_FragColor.a < 0.5)
        return;
    gl_FragColor.rgb = vec3(1.0) - gl_FragColor.rgb;
}
)");
    const auto r = lumi::isf::importiereIsf(quelle, QStringLiteral("frueh"));
    REQUIRE_MESSAGE(r.ok, r.error.toStdString());
    CHECK(r.code.contains(QStringLiteral("return _lumi_frag;")));
    CHECK_FALSE(r.code.contains(QRegularExpression(QStringLiteral("return\\s*;"))));
}

TEST_CASE("IsfImport: verschachtelte Makro-Argumente bleiben heil")
{
    // `IMG_NORM_PIXEL(inputImage, vec2(a, b))` hat ZWEI Argumente, nicht drei.
    // Eine naive Trennung am Komma zerlegt den vec2 mitten drin.
    const QString quelle = QStringLiteral(R"(/*{
    "INPUTS": [ { "NAME": "inputImage", "TYPE": "image" } ]
}*/
void main() {
    gl_FragColor = IMG_NORM_PIXEL(inputImage, vec2(isf_FragNormCoord.x, 0.5));
}
)");
    const auto r = lumi::isf::importiereIsf(quelle, QStringLiteral("nested"));
    REQUIRE_MESSAGE(r.ok, r.error.toStdString());
    CHECK(r.code.contains(QStringLiteral("texture(uTex, vec2(uv.x, 0.5))")));
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

TEST_CASE("IsfImport: Parameter-Block laesst sich aus den Nutzerwerten neu bauen")
{
    // Die Sentinel sind die Nahtstelle zwischen Parameter-Baum und Code:
    // INNERHALB gehoert der Text dem Baum, ausserhalb dem Nutzer.
    const QString quelle = fixture(QStringLiteral("posterize.fs"));
    REQUIRE(!quelle.isEmpty());
    const auto r = lumi::isf::importiereIsf(quelle, QStringLiteral("posterize"));
    REQUIRE(r.ok);

    auto gruppe = lumi::isf::alsParamGruppe(r.parameter);
    REQUIRE(gruppe.werte.size() == 1);
    CHECK(gruppe.werte[0].key == "levels");
    CHECK(gruppe.werte[0].typ == lumi::multieffect::ParamTyp::Zahl);
    CHECK(gruppe.werte[0].hatBereich);

    gruppe.werte[0].zahl = 4.0;  // der Nutzer dreht den Regler
    const QString neu =
        lumi::isf::ersetzeParamBlock(r.code, lumi::isf::erzeugeParamBlock(gruppe));
    CHECK(neu.contains(QStringLiteral("const float levels = 4.0;")));
    CHECK_FALSE(neu.contains(QStringLiteral("const float levels = 30.0;")));
    // Der Code AUSSERHALB des Blocks bleibt unangetastet.
    CHECK(neu.contains(QStringLiteral("vec4 farbe(vec2 uv, vec4 src)")));
    CHECK(neu.contains(QStringLiteral("amountPerLevel")));
    // Genau EIN Block, nicht zwei.
    CHECK(neu.count(QString::fromLatin1(lumi::isf::kParamBlockStart)) == 1);

    // Ohne Block im Code passiert nichts — der Nutzer hat ihn geloescht und
    // meint es so.
    const QString ohne = QStringLiteral("vec4 farbe(vec2 uv, vec4 src){return src;}");
    CHECK(lumi::isf::ersetzeParamBlock(ohne, QStringLiteral("X")) == ohne);
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
    int fx = 0;
    int abgelehnt = 0;
    int mitVs = 0;
    QStringList fehler;
    const QFileInfoList dateien =
        QDir(dir).entryInfoList({QStringLiteral("*.fs")}, QDir::Files, QDir::Name);
    for (const QFileInfo& fi : dateien)
    {
        ++gesamt;
        const QString inhalt = liesDatei(fi.absoluteFilePath());
        const QString vsPfad =
            fi.absolutePath() + QLatin1Char('/') + fi.completeBaseName() +
            QStringLiteral(".vs");
        const QString vs =
            QFileInfo::exists(vsPfad) ? liesDatei(vsPfad) : QString();
        if (!vs.isEmpty()) ++mitVs;

        const auto r = lumi::isf::importiereIsf(inhalt, fi.completeBaseName(), vs);
        if (!r.ok)
        {
            // Generatoren und Uebergaenge SOLLEN abgelehnt werden — das ist
            // kein Fehler, sondern die Kategorie-Erkennung bei der Arbeit.
            ++abgelehnt;
            const bool erwartet = r.error.contains(QStringLiteral("GENERATOR")) ||
                                  r.error.contains(QString::fromUtf8("ÜBERGANG")) ||
                                  r.error.contains(QStringLiteral("GEOMETRIE"));
            if (!erwartet && fehler.size() < 10)
                fehler << (fi.fileName() + QStringLiteral(": ") + r.error);
            continue;
        }
        ++fx;
        // Jeder uebersetzte Filter muss den Vertrag tragen und darf KEIN
        // ISF-Vokabular mehr enthalten — das ist die eigentliche Zusage.
        // `gl_Position` steht bewusst MIT in dieser Liste: es gibt ihn im
        // Fragment-Shader nicht. Ohne den Eintrag hat der Waechter die zwei
        // Geometrie-Dateien still durchgelassen (Befund S72, Frage Patrik).
        //
        // Geprueft wird der Code OHNE Kommentare — dieselbe Sicht, die auch
        // der Riegel im Importeur benutzt: `v002 Dilate/Erode` tragen ein
        // auskommentiertes `gl_Position = ftransform();`, das nichts tut und
        // den Waechter sonst grundlos rot faerbt.
        const QString wirksam = lumi::isf::detail::ohneKommentare(r.code);
        if (!wirksam.contains(QStringLiteral("vec4 farbe(vec2 uv, vec4 src)")) ||
            wirksam.contains(QStringLiteral("IMG_")) ||
            wirksam.contains(QStringLiteral("gl_FragColor")) ||
            wirksam.contains(QStringLiteral("gl_Position")) ||
            wirksam.contains(QStringLiteral("RENDERSIZE")))
        {
            if (fehler.size() < 10)
                fehler << (fi.fileName() + QStringLiteral(": ISF-Reste im Ergebnis"));
        }
    }

    MESSAGE("ISF-Korpus: " << gesamt << " Dateien, " << fx << " als Filter uebersetzt, "
                           << abgelehnt << " abgelehnt (Generator/Uebergang), "
                           << mitVs << " mit .vs-Begleiter");
    for (const QString& f : fehler) MESSAGE("  " << f.toStdString());
    CHECK(gesamt > 0);
    CHECK(fehler.isEmpty());
}
