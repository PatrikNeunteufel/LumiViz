/**
 ****************************************************************************************
 * @file   test_IsfGlSmoke.cpp
 * @brief  GL-Smoke-Test des ISF-Filters (S72): kompiliert und LINKT das
 *         Programm-Paar in einem echten 3.3-Core-Kontext
 *
 * Der Wrapper-Test prueft den TEXT. Ob der Treiber ihn auch annimmt, sagt nur
 * ein echter Kontext — und genau dort schlagen die Dinge zu, die man im Text
 * nicht sieht: `gl_FragColor` gibt es in 330 Core nicht mehr, `#define main`
 * muss sauber aufgehen, und ein Fragment-Shader linkt nur, wenn die Varyings
 * der Vertex-Stufe wirklich passen (Befund S70: `filter` ist reserviert und
 * fiel erst am Treiber auf).
 *
 * Zwei Ebenen wie beim Kopf-Leser: Fixtures aus dem Repo (laufen immer) und
 * der Vidvox-Korpus unter `../ref/isf`, falls geklont.
 *
 * @author Patrik Neunteufel
 * @date   August 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/multieffect/IsfFilterWrapper.hpp"
#include "visualizers/multieffect/IsfImport.hpp"

#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLShaderProgram>
#include <QString>
#include <QSurfaceFormat>

#include <filesystem>
#include <string>

namespace {

std::filesystem::path repoRoot()
{
    std::filesystem::path p(__FILE__);
    for (int i = 0; i < 7; ++i) p = p.parent_path();
    return p;
}

[[nodiscard]] QString liesDatei(const QString& pfad)
{
    QFile f(pfad);
    if (!f.open(QIODevice::ReadOnly)) return {};
    return QString::fromUtf8(f.readAll());
}

/// Uebersetzt eine ISF-Datei und linkt das Programm-Paar. Leerer Rueckgabe-
/// wert = alles gut, sonst der Treiber-Log.
[[nodiscard]] std::string linkeIsf(const QString& fsInhalt, const QString& name,
                                   const QString& vsInhalt)
{
    const auto r = lumi::isf::importiereIsf(fsInhalt, name, vsInhalt);
    if (!r.ok) return "Import: " + r.error.toStdString();

    const auto quellen = lumi::isf::alsBildQuellen(r.bildInputs, r.audioWaveInputs, r.audioFftInputs);
    const auto parameter = lumi::isf::alsParamGruppe(r.parameter);
    const std::string vert =
        lumi::isffilter::wrapVertex(r.vertexCode.toStdString(), parameter);
    const std::string frag = lumi::isffilter::wrapFragment(
        r.fragCode.toStdString(), quellen, parameter);

    QOpenGLShaderProgram program;
    if (!program.addShaderFromSourceCode(QOpenGLShader::Vertex, vert.c_str()))
        return "Vertex: " + program.log().toStdString();
    if (!program.addShaderFromSourceCode(QOpenGLShader::Fragment, frag.c_str()))
        return "Fragment: " + program.log().toStdString();
    if (!program.link()) return "Link: " + program.log().toStdString();
    return {};
}

}  // namespace

TEST_CASE("IsfGlSmoke: die Praelude linkt im 3.3-Core-Kontext")
{
    static int argc = 1;
    static char arg0[] = "UnitTests";
    static char* argv[] = {arg0, nullptr};
    QGuiApplication app(argc, argv);

    QSurfaceFormat fmt;
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);

    QOpenGLContext ctx;
    ctx.setFormat(fmt);
    if (!ctx.create())
    {
        MESSAGE("Kein GL-Kontext moeglich (Headless?) — Smoke uebersprungen");
        return;
    }
    QOffscreenSurface surface;
    surface.setFormat(ctx.format());
    surface.create();
    REQUIRE(surface.isValid());
    REQUIRE(ctx.makeCurrent(&surface));

    // (1) Leerer Knoten — ein frisch eingefuegter muss linken, sonst haette
    //     der Nutzer sofort einen Fehler im Panel stehen.
    {
        QOpenGLShaderProgram leer;
        CHECK(leer.addShaderFromSourceCode(
            QOpenGLShader::Vertex, lumi::isffilter::wrapVertex("", {}).c_str()));
        CHECK(leer.addShaderFromSourceCode(
            QOpenGLShader::Fragment,
            lumi::isffilter::wrapFragment("", {}, {}).c_str()));
        CHECK_MESSAGE(leer.link(), leer.log().toStdString());
    }

    // (2) Der Starter-Shader des Knotens.
    {
        const std::vector<lumi::multieffect::IsfBildQuelle> q = {
            {"inputImage", lumi::multieffect::isffilter::kQuelleKette}};
        QOpenGLShaderProgram st;
        CHECK(st.addShaderFromSourceCode(
            QOpenGLShader::Vertex, lumi::isffilter::wrapVertex("", {}).c_str()));
        CHECK(st.addShaderFromSourceCode(
            QOpenGLShader::Fragment,
            lumi::isffilter::wrapFragment(lumi::isffilter::starterFragment(), q, {})
                .c_str()));
        CHECK_MESSAGE(st.link(), st.log().toStdString());
    }

    // (3) Die Fixtures — je eine Sorte, inkl. .vs-Begleiter.
    const QString fixDir =
        QString::fromStdString((repoRoot() / "asset" / "testdata" / "isf").string());
    for (const char* name : {"posterize", "generator", "uebergang", "alle_typen"})
    {
        const QString fs = liesDatei(fixDir + QStringLiteral("/") +
                                     QLatin1String(name) + QStringLiteral(".fs"));
        REQUIRE_MESSAGE(!fs.isEmpty(), name);
        const std::string log = linkeIsf(fs, QLatin1String(name), {});
        CHECK_MESSAGE(log.empty(), name, ": ", log);
    }
    {
        const QString fs = liesDatei(fixDir + QStringLiteral("/kanten.fs"));
        const QString vs = liesDatei(fixDir + QStringLiteral("/kanten.vs"));
        REQUIRE(!fs.isEmpty());
        REQUIRE(!vs.isEmpty());
        // Der .vs bleibt eine EIGENE Stufe — hier zeigt sich, ob die Varyings
        // zwischen beiden Stufen wirklich zusammenpassen.
        const std::string log = linkeIsf(fs, QStringLiteral("kanten"), vs);
        CHECK_MESSAGE(log.empty(), "kanten: ", log);
    }

    ctx.doneCurrent();
}

TEST_CASE("IsfGlSmoke: Referenz-Korpus linkt (Vidvox ISF-Files)")
{
    const std::filesystem::path dir = repoRoot() / ".." / "ref" / "isf" / "ISF";
    if (!std::filesystem::is_directory(dir))
    {
        MESSAGE("ISF-Korpus nicht vorhanden — uebersprungen.");
        return;
    }

    static int argc = 1;
    static char arg0[] = "UnitTests";
    static char* argv[] = {arg0, nullptr};
    QGuiApplication app(argc, argv);

    QSurfaceFormat fmt;
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    QOpenGLContext ctx;
    ctx.setFormat(fmt);
    if (!ctx.create())
    {
        MESSAGE("Kein GL-Kontext moeglich (Headless?) — Smoke uebersprungen");
        return;
    }
    QOffscreenSurface surface;
    surface.setFormat(ctx.format());
    surface.create();
    REQUIRE(surface.isValid());
    REQUIRE(ctx.makeCurrent(&surface));

    int gesamt = 0;
    int ok = 0;
    std::vector<std::string> fehler;
    for (const auto& e : std::filesystem::directory_iterator(dir))
    {
        if (!e.is_regular_file() || e.path().extension() != ".fs") continue;
        ++gesamt;
        const QString fs =
            liesDatei(QString::fromStdString(e.path().string()));
        std::filesystem::path vsPfad = e.path();
        vsPfad.replace_extension(".vs");
        const QString vs = std::filesystem::exists(vsPfad)
                               ? liesDatei(QString::fromStdString(vsPfad.string()))
                               : QString();
        const std::string log =
            linkeIsf(fs, QString::fromStdString(e.path().stem().string()), vs);
        if (log.empty())
        {
            ++ok;
        }
        else if (fehler.size() < 12)
        {
            fehler.push_back(e.path().filename().string() + ": " + log.substr(0, 220));
        }
    }
    MESSAGE("ISF-GL-Smoke: ", ok, " von ", gesamt, " Dateien kompiliert und gelinkt");
    for (const auto& f : fehler) MESSAGE("  ", f);
    CHECK(gesamt > 0);
    // Kein 100-%-Anspruch: die Bibliothek enthaelt Multipass- und
    // IMPORTED-Dateien, die ohne ihre zusaetzlichen Puffer/Texturen nicht
    // linken KOENNEN. Die Zahl ist der Messwert, an dem sich Fortschritt und
    // Rueckschritt ablesen lassen — ein Einbruch faellt sofort auf.
    CHECK(ok * 100 / gesamt >= 60);
    ctx.doneCurrent();
}
