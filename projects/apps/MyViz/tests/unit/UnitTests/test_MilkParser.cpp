/**
 ****************************************************************************************
 * @file   test_MilkParser.cpp
 * @brief  Tests fuer den .milk-Preset-Parser (Import-Phase Roadmap 6, M1):
 *         synthetische Text-Fixtures (Scalars, Code-Konkatenation, Luecken-Abbruch,
 *         Backtick-Shader, Waves/Shapes, MD3-Superset, Sprites, Fehlertoleranz)
 *         + Korpus-Lauf ueber beide Preset-Packs (falls lokal vorhanden)
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include <MilkParser.hpp>

#include <filesystem>
#include <string>
#include <vector>

using namespace lumi::milk;

namespace {

/// Pfad zum Repo-Root (Datei -> UnitTests -> unit -> tests -> MyViz -> apps -> projects -> <repo>)
std::filesystem::path repoRoot()
{
    std::filesystem::path p(__FILE__);
    for (int i = 0; i < 7; ++i) p = p.parent_path();
    return p;
}

/// Korpus 1: asset/Milkdrop3/presets (lokal, unversioniert — Entscheid Session 39)
std::filesystem::path milkdrop3CorpusDir()
{
    return repoRoot() / "asset" / "Milkdrop3" / "presets";
}

/// Korpus 2: Winamp-MD2-Pack unter ../ref (liegt NEBEN dem Repo, lokal unversioniert)
std::filesystem::path winampCorpusDir()
{
    return repoRoot().parent_path() / "ref" / "winamp_orig" / "Src" / "resources" / "data" /
           "Milkdrop2" / "presets";
}

} // namespace

// =============================================================================
// Grundgeruest: Scalars, Sektionen, Versionen
// =============================================================================

TEST_CASE("MilkParser: MD1-Preset — Scalars + Code, keine Versions-Header")
{
    const ParseResult r = parse("[preset00]\n"
                                "fRating=3.000000\n"
                                "fDecay=0.980000\n"
                                "nWaveMode=6\n"
                                "zoom=1.001600\n"
                                "per_frame_1=q1=bass;\n"
                                "per_pixel_1=zoom=zoom+0.01;\n");
    REQUIRE(r.ok);
    CHECK(r.generation() == 1);
    CHECK(r.presetVersion == 0);
    CHECK(r.psVersion == -1);
    CHECK(r.hadPresetSection);
    CHECK(r.value("fRating", 0.0) == doctest::Approx(3.0));
    CHECK(r.value("fDecay", 0.0) == doctest::Approx(0.98));
    CHECK(r.valueInt("nWaveMode", -1) == 6);
    CHECK(r.value("zoom", 0.0) == doctest::Approx(1.0016));
    CHECK(r.perFrameCode == "q1=bass;");
    CHECK(r.perPixelCode == "zoom=zoom+0.01;");
    CHECK(r.warnings.empty());
}

TEST_CASE("MilkParser: MD2/MD3-Versions-Header vor [preset00]")
{
    const ParseResult md2 = parse("MILKDROP_PRESET_VERSION=201\n"
                                  "PSVERSION=2\n"
                                  "PSVERSION_WARP=2\n"
                                  "PSVERSION_COMP=3\n"
                                  "[preset00]\n"
                                  "fRating=1.0\n");
    REQUIRE(md2.ok);
    CHECK(md2.generation() == 2);
    CHECK(md2.presetVersion == 201);
    CHECK(md2.psVersion == 2);
    CHECK(md2.psVersionWarp == 2);
    CHECK(md2.psVersionComp == 3);

    const ParseResult md3 = parse("MILKDROP_PRESET_VERSION=300\n[preset00]\nfRating=1.0\n");
    REQUIRE(md3.ok);
    CHECK(md3.generation() == 3);

    // PSVERSION ohne MILKDROP_PRESET_VERSION (alte MD2-Schreibweise) -> Generation 2
    const ParseResult ps = parse("PSVERSION=2\n[preset00]\nfRating=1.0\n");
    REQUIRE(ps.ok);
    CHECK(ps.generation() == 2);
}

TEST_CASE("MilkParser: Lookup case-insensitiv, Erst-Treffer gewinnt bei Duplikaten")
{
    const ParseResult r = parse("[preset00]\nfGammaAdj=2.5\nfGammaAdj=9.9\n");
    REQUIRE(r.ok);
    CHECK(r.value("fgammaadj", 0.0) == doctest::Approx(2.5));
    CHECK(r.value("FGAMMAADJ", 0.0) == doctest::Approx(2.5));
    CHECK(r.rawValue("nixda") == nullptr);
    CHECK(r.value("nixda", 42.0) == doctest::Approx(42.0));
}

// =============================================================================
// Code-Konkatenation (Original-ReadCode-Verhalten)
// =============================================================================

TEST_CASE("MilkParser: mehrzeiliger Ausdruck wird OHNE Zeilenumbruch konkateniert")
{
    // Original-Verhalten: per_frame-Zeilen duerfen mitten im Ausdruck umbrechen
    const ParseResult r = parse("[preset00]\n"
                                "per_frame_1=bth=if(above(le,bth),le+114/(le+10)-7.407,\n"
                                "per_frame_2=bth+bth*.07/(bth-12));\n"
                                "per_frame_3=\n"
                                "per_frame_4=q1=bth;\n");
    REQUIRE(r.ok);
    CHECK(r.perFrameCode == "bth=if(above(le,bth),le+114/(le+10)-7.407,"
                            "bth+bth*.07/(bth-12));q1=bth;");
}

TEST_CASE("MilkParser: Luecken-Abbruch mit Warnung fuer verwaiste Zeilen")
{
    const ParseResult r = parse("[preset00]\n"
                                "per_frame_1=a=1;\n"
                                "per_frame_2=b=2;\n"
                                "per_frame_4=c=3;\n");
    REQUIRE(r.ok);
    CHECK(r.perFrameCode == "a=1;b=2;");
    REQUIRE(r.warnings.size() == 1);
    CHECK(r.warnings[0].find("per_frame") != std::string::npos);
    CHECK(r.warnings[0].find("Index 3") != std::string::npos);
}

TEST_CASE("MilkParser: doppelte Code-Zeile — erste gewinnt, Warnung")
{
    const ParseResult r = parse("[preset00]\n"
                                "per_pixel_1=zoom=1;\n"
                                "per_pixel_1=zoom=2;\n");
    REQUIRE(r.ok);
    CHECK(r.perPixelCode == "zoom=1;");
    REQUIRE(r.warnings.size() == 1);
    CHECK(r.warnings[0].find("doppelte Code-Zeile") != std::string::npos);
}

TEST_CASE("MilkParser: Backtick-Zeilen behalten Zeilenumbrueche (Shader)")
{
    const ParseResult r = parse("[preset00]\n"
                                "warp_1=`shader_body\n"
                                "warp_2=`{\n"
                                "warp_3=`    ret = tex2D( sampler_main, uv ).xyz;\n"
                                "warp_4=`}\n"
                                "comp_1=`shader_body\n"
                                "comp_2=`{ ret = float3(1,0,0); }\n");
    REQUIRE(r.ok);
    CHECK(r.warpShader == "shader_body\n{\n    ret = tex2D( sampler_main, uv ).xyz;\n}\n");
    CHECK(r.compShader == "shader_body\n{ ret = float3(1,0,0); }\n");
    CHECK(r.warnings.empty());
}

TEST_CASE("MilkParser: Inline-Kommentare enden am ORIGINAL-Zeilenende (state.cpp:1525)")
{
    // Ohne zeilenweises Strippen wuerde '//rad' beim Konkatenieren den ganzen
    // Rest-Code auffressen (18 Milkdrop3-Presets, Befund Session 39)
    const ParseResult r = parse("[preset00]\n"
                                "per_frame_1=a=1; //rad\n"
                                "per_frame_2=b=2; \\\\ sides\n"
                                "per_frame_3=c=3;\n");
    REQUIRE(r.ok);
    CHECK(r.perFrameCode == "a=1; b=2; c=3;");

    // Backtick-Zeilen (Shader) behalten Kommentare — sie haben echte Umbrueche
    const ParseResult shader = parse("[preset00]\n"
                                     "warp_1=`ret = a; // comment\n"
                                     "warp_2=`ret += b;\n");
    REQUIRE(shader.ok);
    CHECK(shader.warpShader == "ret = a; // comment\nret += b;\n");
}

TEST_CASE("MilkParser: per_frame_init_ wird nicht von per_frame_ geschluckt")
{
    const ParseResult r = parse("[preset00]\n"
                                "per_frame_init_1=n=0;\n"
                                "per_frame_1=n=n+1;\n");
    REQUIRE(r.ok);
    CHECK(r.perFrameInitCode == "n=0;");
    CHECK(r.perFrameCode == "n=n+1;");
}

// =============================================================================
// Custom Waves / Shapes (inkl. MD3-Superset)
// =============================================================================

TEST_CASE("MilkParser: Wave-Params + Code-Slots je Index")
{
    const ParseResult r = parse("[preset00]\n"
                                "wavecode_0_enabled=1\n"
                                "wavecode_0_samples=512\n"
                                "wavecode_0_r=0.6\n"
                                "wave_0_init1=t=0;\n"
                                "wave_0_per_frame1=t=t+0.1;\n"
                                "wave_0_per_point1=x=sample;\n"
                                "wave_0_per_point2=y=value1;\n"
                                "wavecode_3_enabled=0\n");
    REQUIRE(r.ok);
    REQUIRE(r.waves.size() == 2);

    const CustomWave* w0 = r.wave(0);
    REQUIRE(w0 != nullptr);
    CHECK(w0->param("enabled", 0.0) == doctest::Approx(1.0));
    CHECK(w0->param("samples", 0.0) == doctest::Approx(512.0));
    CHECK(w0->param("r", 0.0) == doctest::Approx(0.6));
    CHECK(w0->initCode == "t=0;");
    CHECK(w0->frameCode == "t=t+0.1;");
    CHECK(w0->pointCode == "x=sample;y=value1;");

    const CustomWave* w3 = r.wave(3);
    REQUIRE(w3 != nullptr);
    CHECK(w3->param("enabled", 1.0) == doctest::Approx(0.0));
    CHECK(r.wave(1) == nullptr);
}

TEST_CASE("MilkParser: Shape-Params + Code (kein per_point)")
{
    const ParseResult r = parse("[preset00]\n"
                                "shapecode_1_enabled=1\n"
                                "shapecode_1_sides=32\n"
                                "shape_1_init1=t=0;\n"
                                "shape_1_per_frame1=ang=time;\n"
                                "shape_1_per_frame2=r=0.5+0.1*sin(t);\n");
    REQUIRE(r.ok);
    REQUIRE(r.shapes.size() == 1);
    const CustomShape* s1 = r.shape(1);
    REQUIRE(s1 != nullptr);
    CHECK(s1->param("sides", 0.0) == doctest::Approx(32.0));
    CHECK(s1->initCode == "t=0;");
    CHECK(s1->frameCode == "ang=time;r=0.5+0.1*sin(t);");
}

TEST_CASE("MilkParser: MD3-Superset — Wave-Index 15 ok, 16 ignoriert mit Warnung")
{
    const ParseResult r = parse("[preset00]\n"
                                "wavecode_15_enabled=1\n"
                                "wavecode_16_enabled=1\n");
    REQUIRE(r.ok);
    REQUIRE(r.waves.size() == 1);
    CHECK(r.waves[0].index == 15);
    REQUIRE(r.warnings.size() == 1);
    CHECK(r.warnings[0].find("Superset-Cap") != std::string::npos);
}

// =============================================================================
// MD3-Sprites
// =============================================================================

TEST_CASE("MilkParser: Sprite-Sektion [SPRITE1_BEGIN]..[SPRITE1_END]")
{
    const ParseResult r = parse("[preset00]\n"
                                "fRating=1.0\n"
                                "[SPRITE1_BEGIN]\n"
                                "SpriteName=sprites\\Jello1.png\n"
                                "SpriteColorKey=0x00FF00\n"
                                "SpriteBlend=7\n"
                                "code_1=a=1.0;\n"
                                "code_2=x=0.5;\n"
                                "[SPRITE1_END]\n"
                                "fGammaAdj=2.0\n");
    REQUIRE(r.ok);
    REQUIRE(r.sprites.size() == 1);
    const Sprite& s = r.sprites[0];
    CHECK(s.index == 1);
    CHECK(*findParam(s.params, "SpriteName") == "sprites\\Jello1.png");
    CHECK(paramInt(s.params, "SpriteColorKey", -1) == 0x00FF00);
    CHECK(paramInt(s.params, "SpriteBlend", -1) == 7);
    CHECK(s.code == "a=1.0;x=0.5;");
    // nach [SPRITE1_END] geht es im Preset-Rumpf weiter
    CHECK(r.value("fGammaAdj", 0.0) == doctest::Approx(2.0));
}

TEST_CASE("MilkParser: Sprite-Kurzform [SPRITE1] (Korpus-Variante)")
{
    const ParseResult r = parse("[preset00]\n"
                                "fRating=1.0\n"
                                "[SPRITE1]\n"
                                "SpriteBlend=3\n"
                                "[SPRITE1_END]\n");
    REQUIRE(r.ok);
    REQUIRE(r.sprites.size() == 1);
    CHECK(paramInt(r.sprites[0].params, "SpriteBlend", -1) == 3);
}

// =============================================================================
// Fehlertoleranz (Import-Analyse §4.3: nie hart scheitern)
// =============================================================================

TEST_CASE("MilkParser: handgeschriebene Datei ohne [preset00] + //-Kommentare")
{
    const ParseResult r = parse("// Per Frame\n"
                                "per_frame_1=cx=0.5; // Zentrum\n"
                                "\n"
                                "// Per Vertex\n"
                                "per_pixel_1=zoom=1.01;\n");
    REQUIRE(r.ok);
    CHECK_FALSE(r.hadPresetSection);
    CHECK(r.perFrameCode == "cx=0.5; ");  // Inline-Kommentar zeilenweise gestrippt
    CHECK(r.perPixelCode == "zoom=1.01;");
    REQUIRE(r.warnings.size() == 1);
    CHECK(r.warnings[0].find("[preset00]") != std::string::npos);
}

TEST_CASE("MilkParser: leere Datei / reiner Muell -> ok=false")
{
    const ParseResult empty = parse("");
    CHECK_FALSE(empty.ok);
    CHECK_FALSE(empty.error.empty());

    const ParseResult junk = parse("dies ist kein preset\nnur text ohne gleichheitszeichen\n");
    CHECK_FALSE(junk.ok);
}

TEST_CASE("MilkParser: Zeilen ohne '=' werden gezaehlt gemeldet, Rest parst weiter")
{
    const ParseResult r = parse("[preset00]\n"
                                "kaputte zeile eins\n"
                                "fRating=2.0\n"
                                "kaputte zeile zwei\n");
    REQUIRE(r.ok);
    CHECK(r.value("fRating", 0.0) == doctest::Approx(2.0));
    REQUIRE(r.warnings.size() == 1);
    CHECK(r.warnings[0].find("2 Zeile(n) ohne '='") != std::string::npos);
}

TEST_CASE("MilkParser: unbekannte Sektion wird uebersprungen, danach geht es weiter")
{
    const ParseResult r = parse("[preset00]\n"
                                "fRating=1.0\n"
                                "[komischer_block]\n"
                                "geheim=42\n"
                                "[preset00]\n"
                                "fDecay=0.9\n");
    REQUIRE(r.ok);
    CHECK(r.rawValue("geheim") == nullptr);
    CHECK(r.value("fDecay", 0.0) == doctest::Approx(0.9));
    REQUIRE(r.warnings.size() == 1);
    CHECK(r.warnings[0].find("unbekannter Abschnitt") != std::string::npos);
}

TEST_CASE("MilkParser: CRLF und UTF-8-BOM")
{
    const ParseResult r = parse("\xEF\xBB\xBFMILKDROP_PRESET_VERSION=201\r\n"
                                "[preset00]\r\n"
                                "fRating=4.0\r\n"
                                "per_frame_1=a=1;\r\n");
    REQUIRE(r.ok);
    CHECK(r.presetVersion == 201);
    CHECK(r.value("fRating", 0.0) == doctest::Approx(4.0));
    CHECK(r.perFrameCode == "a=1;");
    CHECK(r.warnings.empty());
}

TEST_CASE("MilkParser: parseFile auf nicht existierende Datei -> ok=false")
{
    const ParseResult r = parseFile("Z:/gibts/nicht/nie.milk");
    CHECK_FALSE(r.ok);
    CHECK(r.error.find("nicht lesbar") != std::string::npos);
}

// =============================================================================
// Korpus-Laeufe (lokal unversioniert — Smoke, umgebungsabhaengig)
// =============================================================================

namespace {

struct CorpusStats
{
    int files = 0;
    int failed = 0;
    int gen1 = 0;
    int gen2 = 0;
    int gen3 = 0;
    int withWarnings = 0;
    int totalWarnings = 0;
};

CorpusStats runCorpus(const std::filesystem::path& dir)
{
    CorpusStats stats;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(dir))
    {
        if (!entry.is_regular_file() || entry.path().extension() != ".milk") continue;
        ++stats.files;
        CAPTURE(entry.path().filename().string());
        const ParseResult r = parseFile(entry.path());
        CHECK(r.ok);
        if (!r.ok)
        {
            ++stats.failed;
            continue;
        }
        switch (r.generation())
        {
            case 1: ++stats.gen1; break;
            case 2: ++stats.gen2; break;
            default: ++stats.gen3; break;
        }
        if (!r.warnings.empty())
        {
            ++stats.withWarnings;
            stats.totalWarnings += static_cast<int>(r.warnings.size());
        }
    }
    return stats;
}

} // namespace

TEST_CASE("MilkParser: Korpus asset/Milkdrop3 parst ohne Fehler")
{
    const std::filesystem::path dir = milkdrop3CorpusDir();
    if (!std::filesystem::exists(dir))
    {
        MESSAGE("Korpus nicht vorhanden (", dir.string(),
                ") — Smoke-Teil uebersprungen, synthetische Tests decken das Format");
        return;
    }
    const CorpusStats s = runCorpus(dir);
    CHECK(s.files >= 352); // Bestand Session 39; darf wachsen
    CHECK(s.failed == 0);
    CHECK(s.gen3 > 0);     // der MD3-Anteil ist der Grund fuer dieses Pack
    MESSAGE("Milkdrop3-Korpus: ", s.files, " Dateien, Generationen 1/2/3 = ", s.gen1, "/", s.gen2,
            "/", s.gen3, ", ", s.withWarnings, " mit Warnungen (", s.totalWarnings, " gesamt)");
}

TEST_CASE("MilkParser: Korpus winamp_orig (MD2-Pack) parst ohne Fehler")
{
    const std::filesystem::path dir = winampCorpusDir();
    if (!std::filesystem::exists(dir))
    {
        MESSAGE("Korpus nicht vorhanden (", dir.string(),
                ") — Smoke-Teil uebersprungen, synthetische Tests decken das Format");
        return;
    }
    const CorpusStats s = runCorpus(dir);
    CHECK(s.files == 558); // fixer Referenzbestand
    CHECK(s.failed == 0);
    CHECK(s.gen1 > 0);     // der MD1-Anteil ist der Grund fuer dieses Pack
    MESSAGE("Winamp-Korpus: ", s.files, " Dateien, Generationen 1/2/3 = ", s.gen1, "/", s.gen2,
            "/", s.gen3, ", ", s.withWarnings, " mit Warnungen (", s.totalWarnings, " gesamt)");
}
