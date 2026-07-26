/**
 ****************************************************************************************
 * @file   test_AvsParser.cpp
 * @brief  Tests fuer den .avs-Container-Parser (Import-Phase Roadmap 3):
 *         synthetische Binaer-Fixtures (Signatur, TLV, Extended-Mode, EEL-Slots,
 *         Altformate, Fehlertoleranz) + Korpus-Lauf ueber die 35 Referenz-Presets
 *         aus ref/vis_avs (falls lokal vorhanden)
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include <AvsParser.hpp>

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

using lumi::avs::EffectNode;
using lumi::avs::ParseResult;
using lumi::avs::parse;
using lumi::avs::parseFile;

namespace
{

/// Kleiner Little-Endian-Byte-Builder fuer synthetische .avs-Fixtures
class Bytes
{
public:
    Bytes& u8(int v)
    {
        m_v.push_back(static_cast<std::uint8_t>(v));
        return *this;
    }
    Bytes& i32(std::int32_t v)
    {
        const auto u = static_cast<std::uint32_t>(v);
        m_v.push_back(static_cast<std::uint8_t>(u & 0xFF));
        m_v.push_back(static_cast<std::uint8_t>((u >> 8) & 0xFF));
        m_v.push_back(static_cast<std::uint8_t>((u >> 16) & 0xFF));
        m_v.push_back(static_cast<std::uint8_t>((u >> 24) & 0xFF));
        return *this;
    }
    Bytes& raw(const void* data, std::size_t n)
    {
        const auto* p = static_cast<const std::uint8_t*>(data);
        m_v.insert(m_v.end(), p, p + n);
        return *this;
    }
    Bytes& text(const char* s) { return raw(s, std::strlen(s)); }

    /// Laengen-praefixierter String wie von save_string geschrieben (inkl. NUL)
    Bytes& lpString(const char* s)
    {
        const auto len = static_cast<std::int32_t>(std::strlen(s) + 1);
        i32(len);
        raw(s, static_cast<std::size_t>(len));
        return *this;
    }

    /// 32-Byte-APE-ID-Block (NUL-gepolstert)
    Bytes& apeId(const char* s)
    {
        char block[32] = {};
        std::strncpy(block, s, sizeof(block) - 1);
        return raw(block, sizeof(block));
    }

    /// Preset-Signatur "Nullsoft AVS Preset 0.<ver>\x1a"
    Bytes& signature(char version = '2')
    {
        text("Nullsoft AVS Preset 0.");
        u8(version);
        u8(0x1a);
        return *this;
    }

    /// Effekt-Eintrag: [id][len][blob]
    Bytes& effect(std::int32_t id, const Bytes& blob)
    {
        i32(id);
        i32(static_cast<std::int32_t>(blob.m_v.size()));
        return raw(blob.m_v.data(), blob.m_v.size());
    }

    /// APE-Effekt-Eintrag: [id>=16384][32-Byte-ID][len][blob]
    Bytes& apeEffect(const char* idString, const Bytes& blob)
    {
        i32(lumi::avs::kApeIdBase);
        apeId(idString);
        i32(static_cast<std::int32_t>(blob.m_v.size()));
        return raw(blob.m_v.data(), blob.m_v.size());
    }

    [[nodiscard]] const std::vector<std::uint8_t>& vec() const { return m_v; }

private:
    std::vector<std::uint8_t> m_v;
};

/// Pfad zum Referenz-Korpus (lokal, unversioniert): <repo>/../ref/vis_avs/...
std::filesystem::path corpusDir()
{
    std::filesystem::path p(__FILE__);
    // Datei -> UnitTests -> unit -> tests -> MyViz -> apps -> projects -> <repo>
    for (int i = 0; i < 7; ++i) p = p.parent_path();
    // ref/ liegt NEBEN dem Repo (cmake/ref, lokal unversioniert)
    return p.parent_path() / "ref" / "vis_avs" / "avs" / "vis_avs" / "presets";
}

} // namespace

// =============================================================================
// Signatur
// =============================================================================

TEST_CASE("AvsParser: Signatur 0.2 wird akzeptiert, Version erkannt")
{
    Bytes b;
    b.signature('2').u8(0x01);   // Root-Mode: clear every frame
    const ParseResult r = parse(b.vec());
    REQUIRE(r.ok);
    CHECK(r.formatVersion == 2);
    CHECK(r.root.isList);
    CHECK(r.root.name == "Main");
    CHECK(r.root.list.clearEveryFrame());
    CHECK(r.effectCount() == 0);
}

TEST_CASE("AvsParser: Signatur 0.1 wird akzeptiert")
{
    Bytes b;
    b.signature('1').u8(0x00);
    const ParseResult r = parse(b.vec());
    REQUIRE(r.ok);
    CHECK(r.formatVersion == 1);
    CHECK_FALSE(r.root.list.clearEveryFrame());
}

TEST_CASE("AvsParser: falsche Signatur und Kurz-Dateien -> ok=false")
{
    SUBCASE("fremder Inhalt")
    {
        const char* junk = "MilkDrop preset ... nicht AVS, aber lang genug ......";
        const ParseResult r =
            parse(reinterpret_cast<const std::uint8_t*>(junk), std::strlen(junk));
        CHECK_FALSE(r.ok);
        CHECK_FALSE(r.error.empty());
    }
    SUBCASE("falsche Versionsziffer")
    {
        Bytes b;
        b.signature('3').u8(0).i32(0).i32(0);
        CHECK_FALSE(parse(b.vec()).ok);
    }
    SUBCASE("abgeschnittene Signatur")
    {
        const char* stub = "Nullsoft AVS Preset 0.2";   // ohne 0x1a-Terminator
        CHECK_FALSE(
            parse(reinterpret_cast<const std::uint8_t*>(stub), std::strlen(stub)).ok);
    }
    SUBCASE("nur Signatur -> leeres Preset (tolerant)")
    {
        Bytes b;
        b.signature('2');
        const ParseResult r = parse(b.vec());
        CHECK(r.ok);
        CHECK(r.effectCount() == 0);
    }
    SUBCASE("leer") { CHECK_FALSE(parse(nullptr, 0).ok); }
}

// =============================================================================
// TLV-Grundformen
// =============================================================================

TEST_CASE("AvsParser: einzelner Builtin-Effekt (Invert)")
{
    Bytes blob;
    blob.i32(1);   // enabled
    Bytes b;
    b.signature().u8(0x00).effect(37, blob);

    const ParseResult r = parse(b.vec());
    REQUIRE(r.ok);
    REQUIRE(r.effectCount() == 1);
    const EffectNode& fx = r.root.children[0];
    CHECK(fx.name == "Invert");
    CHECK(fx.decoded);
    CHECK(fx.field("enabled") == 1);
    CHECK(fx.rawConfig.size() == 4);
    CHECK(r.warnings.empty());
}

TEST_CASE("AvsParser: mehrere Effekte in Reihenfolge")
{
    Bytes clear;
    clear.i32(1).i32(0x123456).i32(0).i32(0).i32(1);
    Bytes blur;
    blur.i32(1).i32(2);
    Bytes b;
    b.signature().u8(0x00).effect(25, clear).effect(6, blur);

    const ParseResult r = parse(b.vec());
    REQUIRE(r.ok);
    REQUIRE(r.effectCount() == 2);
    CHECK(r.root.children[0].name == "Clear Screen");
    CHECK(r.root.children[0].field("color") == 0x123456);
    CHECK(r.root.children[0].field("onlyfirst") == 1);
    CHECK(r.root.children[1].name == "Blur");
    CHECK(r.root.children[1].field("roundmode") == 2);
}

TEST_CASE("AvsParser: Picture (id 34) — NUL-Dateiname zwischen Feldern")
{
    Bytes pic;
    pic.i32(1).i32(0).i32(1).i32(0).i32(6);  // enabled, blend, blendavg, adapt, persist
    pic.text("bg.bmp").u8(0);                // NUL-terminated filename
    pic.i32(1).i32(0);                       // ratio, axis_ratio
    Bytes b;
    b.signature().u8(0x00).effect(34, pic);

    const ParseResult r = parse(b.vec());
    REQUIRE(r.ok);
    REQUIRE(r.effectCount() == 1);
    const EffectNode& fx = r.root.children[0];
    CHECK(fx.decoded);
    CHECK(fx.name == "Picture");
    CHECK(fx.field("blendavg") == 1);
    CHECK(fx.slot("filename") == "bg.bmp");
    CHECK(fx.field("ratio") == 1);
}

TEST_CASE("AvsParser: Jheriko Global APE — reserved-Skip + NUL-Codes")
{
    Bytes jg;
    jg.i32(1);                          // load = once
    for (int i = 0; i < 6; ++i) jg.i32(0);  // null0: 6 reserved int32
    jg.text("reg00=5").u8(0);           // init (NUL-terminated)
    jg.text("reg00=reg00+1").u8(0);     // frame
    jg.u8(0);                           // beat (empty)
    jg.u8(0).u8(0).u8(0);               // file, saveRegRange, saveBufRange (empty)

    Bytes b;
    b.signature().u8(0x00).apeEffect("Jheriko: Global", jg);

    const ParseResult r = parse(b.vec());
    REQUIRE(r.ok);
    REQUIRE(r.effectCount() == 1);
    const EffectNode& fx = r.root.children[0];
    CHECK(fx.decoded);
    CHECK(fx.field("load") == 1);
    CHECK(fx.slot("init") == "reg00=5");
    CHECK(fx.slot("frame") == "reg00=reg00+1");
    CHECK(fx.slot("beat") == "");
}

TEST_CASE("AvsParser: Buffer blend APE — vier Felder")
{
    Bytes bb;
    bb.i32(1).i32(8).i32(3).i32(2);  // enabled, bufferB=current, bufferA=4, mode=max
    Bytes b;
    b.signature().u8(0x00).apeEffect("Misc: Buffer blend", bb);

    const ParseResult r = parse(b.vec());
    REQUIRE(r.ok);
    const EffectNode& fx = r.root.children[0];
    CHECK(fx.decoded);
    CHECK(fx.field("bufferB") == 8);
    CHECK(fx.field("bufferA") == 3);
    CHECK(fx.field("mode") == 2);
}

TEST_CASE("AvsParser: Color Map APE — Header + erste aktive Map dekodiert")
{
    const char name48[48] = {};
    Bytes cm;
    cm.i32(2)    // key = blue
        .i32(1)  // blendMode = additive
        .i32(0)  // mapCycleMode = single
        .u8(150).u8(0).u8(0).u8(0);  // adjustBlend=150 + 3 padding bytes
    // 8 fixed 60-byte headers: map0 disabled (num=1), map1 enabled (num=2), rest empty
    cm.i32(0).i32(1).i32(0).raw(name48, 48);
    cm.i32(1).i32(2).i32(0).raw(name48, 48);
    for (int i = 0; i < 6; ++i) cm.i32(0).i32(0).i32(0).raw(name48, 48);
    // colour data per map (position, colour 0x00RRGGBB, id)
    cm.i32(50).i32(0x111111).i32(0);          // map0's single entry (skipped)
    cm.i32(0).i32(0x000000).i32(0);           // map1 entry 0
    cm.i32(255).i32(0x00FF00).i32(0);         // map1 entry 1

    Bytes b;
    b.signature().u8(0x00).apeEffect("Color Map", cm);

    const ParseResult r = parse(b.vec());
    REQUIRE(r.ok);
    REQUIRE(r.effectCount() == 1);
    const EffectNode& fx = r.root.children[0];
    CHECK(fx.decoded);
    CHECK(fx.apeId == "Color Map");
    CHECK(fx.field("key") == 2);
    CHECK(fx.field("blendMode") == 1);
    CHECK(fx.field("adjustBlend") == 150);
    CHECK(fx.field("cmcount") == 2);             // captured map1, not map0
    REQUIRE(fx.colors.size() == 2);
    CHECK(fx.colors[1] == 0x00FF00u);
    CHECK(fx.field("cmpos1") == 255);
}

TEST_CASE("AvsParser: unbekannte Builtin-ID -> Roh-Blob + Warnung")
{
    Bytes blob;
    blob.i32(42).i32(43);
    Bytes b;
    b.signature().u8(0x00).effect(300, blob);

    const ParseResult r = parse(b.vec());
    REQUIRE(r.ok);
    REQUIRE(r.effectCount() == 1);
    CHECK_FALSE(r.root.children[0].decoded);
    CHECK(r.root.children[0].rawConfig.size() == 8);
    REQUIRE(r.warnings.size() == 1);
    CHECK(r.warnings[0].find("300") != std::string::npos);
}

// =============================================================================
// EEL-Code-Formate (SuperScope als Traeger des Quartett-Musters)
// =============================================================================

TEST_CASE("AvsParser: SuperScope neues Format (laengen-praefixierte Slots)")
{
    Bytes blob;
    blob.u8(1)                       // Versions-Byte
        .lpString("x=i*2-1; y=v")    // [0] Point
        .lpString("t=t+0.01")        // [1] Frame
        .lpString("b=b+1")           // [2] Beat
        .lpString("n=100; t=0")      // [3] Init
        .i32(0)                      // which_ch
        .i32(2)                      // num_colors
        .i32(0xFF0000)
        .i32(0x00FF00)
        .i32(1);                     // drawmode
    Bytes b;
    b.signature().u8(0x00).effect(36, blob);

    const ParseResult r = parse(b.vec());
    REQUIRE(r.ok);
    REQUIRE(r.effectCount() == 1);
    const EffectNode& fx = r.root.children[0];
    CHECK(fx.name == "SuperScope");
    REQUIRE(fx.code.size() == 4);
    CHECK(fx.slot("point") == "x=i*2-1; y=v");
    CHECK(fx.slot("frame") == "t=t+0.01");
    CHECK(fx.slot("beat") == "b=b+1");
    CHECK(fx.slot("init") == "n=100; t=0");
    REQUIRE(fx.colors.size() == 2);
    CHECK(fx.colors[0] == 0xFF0000);
    CHECK(fx.colors[1] == 0x00FF00);
    CHECK(fx.field("drawmode") == 1);
}

TEST_CASE("AvsParser: SuperScope Altformat (4x256-Byte-Bloecke)")
{
    char block[1024] = {};
    std::strcpy(block + 0, "x=i");       // Point
    std::strcpy(block + 256, "t=t+1");   // Frame
    std::strcpy(block + 512, "");        // Beat (leer)
    std::strcpy(block + 768, "n=50");    // Init
    Bytes blob;
    blob.raw(block, sizeof(block)).i32(0).i32(0).i32(0);
    Bytes b;
    b.signature().u8(0x00).effect(36, blob);

    const ParseResult r = parse(b.vec());
    REQUIRE(r.ok);
    const EffectNode& fx = r.root.children[0];
    CHECK(fx.slot("point") == "x=i");
    CHECK(fx.slot("frame") == "t=t+1");
    CHECK(fx.slot("beat").empty());
    CHECK(fx.slot("init") == "n=50");
    CHECK(fx.colors.empty());
}

TEST_CASE("AvsParser: Movement mit !rect-Marker und Expression")
{
    Bytes blob;
    blob.i32(32767)             // User-Expression-Effekt
        .text("!rect ")
        .u8(1)                  // neues String-Format
        .lpString("x=x*0.9; y=y*0.9")
        .i32(1)                 // blend
        .i32(0)                 // sourcemapped
        .i32(1)                 // rectangular
        .i32(1)                 // subpixel
        .i32(0);                // wrap
    Bytes b;
    b.signature().u8(0x00).effect(15, blob);

    const ParseResult r = parse(b.vec());
    REQUIRE(r.ok);
    const EffectNode& fx = r.root.children[0];
    CHECK(fx.name == "Movement");
    CHECK(fx.slot("point") == "x=x*0.9; y=y*0.9");
    CHECK(fx.field("effect") == 32767);
    CHECK(fx.field("blend") == 1);
    CHECK(fx.field("rectangular") == 1);
    CHECK(fx.field("subpixel") == 1);
}

TEST_CASE("AvsParser: Dynamic Movement + Color Modifier Slot-Namen")
{
    Bytes dmove;
    dmove.u8(1)
        .lpString("x=x")    // point
        .lpString("")       // frame
        .lpString("")       // beat
        .lpString("")       // init
        .i32(1)             // subpixel
        .i32(0)             // rectcoords
        .i32(16)            // xres
        .i32(12)            // yres
        .i32(0)             // blend
        .i32(1);            // wrap
    Bytes dcolor;
    dcolor.u8(1)
        .lpString("red=1-red")   // level
        .lpString("")            // frame
        .lpString("")            // beat
        .lpString("")            // init
        .i32(1);                 // recompute
    Bytes b;
    b.signature().u8(0x00).effect(43, dmove).effect(45, dcolor);

    const ParseResult r = parse(b.vec());
    REQUIRE(r.ok);
    REQUIRE(r.effectCount() == 2);
    CHECK(r.root.children[0].slot("point") == "x=x");
    CHECK(r.root.children[0].field("xres") == 16);
    CHECK(r.root.children[0].field("nomove") == 0);
    CHECK(r.root.children[1].slot("level") == "red=1-red");
    CHECK(r.root.children[1].field("recompute") == 1);
}

// =============================================================================
// Verschachtelte Effektlisten
// =============================================================================

TEST_CASE("AvsParser: verschachtelte Liste mit Extended-Data und EEL-Code")
{
    // Listen-Mode: Extended-Size 36, blendin=2, blendout=3 (XOR-1-codiert)
    const std::int32_t mode = (36 << 24) | (2 << 8) | ((3 ^ 1) << 16);

    Bytes listCode;
    listCode.i32(1)                    // use_code
        .lpString("n=0")               // init
        .lpString("alphain=alphain");  // frame

    Bytes inner;
    inner.u8((mode & 0xFF) | 0x80).i32(mode);
    inner.i32(120).i32(200).i32(3).i32(4).i32(0).i32(1).i32(1).i32(15);   // Extended
    inner.apeEffect("AVS 2.8+ Effect List Config", listCode);
    Bytes invert;
    invert.i32(1);
    inner.effect(37, invert);

    Bytes b;
    b.signature().u8(0x00).effect(lumi::avs::kListId, inner);

    const ParseResult r = parse(b.vec());
    REQUIRE(r.ok);
    REQUIRE(r.root.children.size() == 1);
    const EffectNode& list = r.root.children[0];
    CHECK(list.isList);
    CHECK(list.name == "Effect List");
    CHECK(list.list.blendIn() == 2);
    CHECK(list.list.blendOut() == 3);
    CHECK(list.list.inBlendVal == 120);
    CHECK(list.list.outBlendVal == 200);
    CHECK(list.list.bufferIn == 3);
    CHECK(list.list.beatRender == 1);
    CHECK(list.list.beatRenderFrames == 15);
    CHECK(list.list.useCode == 1);
    CHECK(list.list.initCode == "n=0");
    CHECK(list.list.frameCode == "alphain=alphain");
    // der Code-Pseudo-Eintrag ist KEIN Kind
    REQUIRE(list.children.size() == 1);
    CHECK(list.children[0].name == "Invert");
    CHECK(r.effectCount() == 2);
}

TEST_CASE("AvsParser: APE-Alias wird auf Builtin gemappt (Nullsoft MIRROR v1)")
{
    Bytes mirror;
    mirror.i32(1).i32(3).i32(0).i32(1).i32(4);
    Bytes b;
    b.signature().u8(0x00).apeEffect("Nullsoft MIRROR v1", mirror);

    const ParseResult r = parse(b.vec());
    REQUIRE(r.ok);
    REQUIRE(r.effectCount() == 1);
    const EffectNode& fx = r.root.children[0];
    CHECK(fx.name == "Mirror");
    CHECK(fx.decoded);
    CHECK(fx.field("mode") == 3);
    CHECK(fx.field("slower") == 4);
    CHECK(fx.apeId == "Nullsoft MIRROR v1");
}

TEST_CASE("AvsParser: Texer II APE — fester Namenspuffer, dann Flags + Skripte")
{
    // Layout an 579 Blobs der Preset-Sammlung gepinnt (S50): der Bildname ist
    // ein FESTER MAX_PATH-Puffer mit NUL-Terminierung, KEIN laengenpraefixierter
    // String. Vor dem Fix las der Decoder die ersten vier Namensbytes als
    // Laenge — Name, Flags und alle vier Skript-Slots kamen leer heraus, der
    // Renderer zeichnete dann einen einzigen Default-Punkt.
    char nameBuf[260] = {};
    std::strncpy(nameBuf, "avsres_texer_circle_sharp_19x19.bmp", sizeof(nameBuf) - 1);
    // Der Puffer wird beim Speichern nicht geleert: Rest eines laengeren
    // Vornamens hinter dem NUL, der beim Lesen verschwinden muss.
    std::strncpy(nameBuf + 40, "bmp", 4);

    Bytes t2;
    t2.i32(0)                                   // null0
        .raw(nameBuf, sizeof(nameBuf))          // Bildname (fest, NUL-terminiert)
        .i32(1).i32(0).i32(1)                   // resizing, wrapAround, colorFiltering
        .i32(0)                                 // null1
        .lpString("n=500").lpString("t=t+1")
        .lpString("").lpString("x=i*2-1;y=0;");

    Bytes b;
    b.signature().u8(0x00).apeEffect("Acko.net: Texer II", t2);

    const ParseResult r = parse(b.vec());
    REQUIRE(r.ok);
    REQUIRE(r.effectCount() == 1);
    const EffectNode& fx = r.root.children[0];
    CHECK(fx.decoded);
    CHECK(fx.slot("filename") == "avsres_texer_circle_sharp_19x19.bmp");
    CHECK(fx.field("resizing") == 1);
    CHECK(fx.field("wrapAround") == 0);
    CHECK(fx.field("colorFiltering") == 1);
    CHECK(fx.slot("init") == "n=500");
    CHECK(fx.slot("frame") == "t=t+1");
    CHECK(fx.slot("beat") == "");
    CHECK(fx.slot("point") == "x=i*2-1;y=0;");
}

TEST_CASE("AvsParser: unbekannter APE -> Roh-Blob + Warnung")
{
    Bytes blob;
    blob.i32(7).i32(8).i32(9);
    Bytes b;
    b.signature().u8(0x00).apeEffect("Texer II", blob);

    const ParseResult r = parse(b.vec());
    REQUIRE(r.ok);
    REQUIRE(r.effectCount() == 1);
    CHECK_FALSE(r.root.children[0].decoded);
    CHECK(r.root.children[0].name == "Texer II");
    CHECK(r.root.children[0].rawConfig.size() == 12);
    REQUIRE(r.warnings.size() == 1);
    CHECK(r.warnings[0].find("Texer II") != std::string::npos);
}

// =============================================================================
// Fehlertoleranz (nie hart abbrechen)
// =============================================================================

TEST_CASE("AvsParser: abgeschnittener Blob -> Warnung, vorherige Effekte bleiben")
{
    Bytes invert;
    invert.i32(1);
    Bytes b;
    b.signature().u8(0x00).effect(37, invert);
    b.i32(6);         // naechster Effekt: Blur ...
    b.i32(100);       // ... behauptet 100 Bytes ...
    b.i32(1);         // ... liefert aber nur 4
    const ParseResult r = parse(b.vec());
    REQUIRE(r.ok);
    CHECK(r.effectCount() == 1);
    CHECK(r.root.children[0].name == "Invert");
    REQUIRE_FALSE(r.warnings.empty());
    CHECK(r.warnings[0].find("abgeschnitten") != std::string::npos);
}

TEST_CASE("AvsParser: negative Blob-Laenge -> Warnung, kein Absturz")
{
    Bytes b;
    b.signature().u8(0x00);
    b.i32(37).i32(-5);
    const ParseResult r = parse(b.vec());
    REQUIRE(r.ok);
    CHECK(r.effectCount() == 0);
    CHECK_FALSE(r.warnings.empty());
}

TEST_CASE("AvsParser: leerer Effekt-Blob dekodiert zu Default-Feldern")
{
    Bytes empty;
    Bytes b;
    b.signature().u8(0x00).effect(44, empty);   // Fast Brightness ohne Daten
    const ParseResult r = parse(b.vec());
    REQUIRE(r.ok);
    REQUIRE(r.effectCount() == 1);
    CHECK(r.root.children[0].decoded);
    CHECK(r.root.children[0].field("dir") == 0);
}

// =============================================================================
// Korpus-Lauf (35 Referenz-Presets, lokal unter ../ref — Smoke, umgebungsabhaengig)
// =============================================================================

TEST_CASE("AvsParser: Referenz-Korpus parst ohne Fehler")
{
    const std::filesystem::path dir = corpusDir();
    if (!std::filesystem::exists(dir))
    {
        MESSAGE("Korpus nicht vorhanden (", dir.string(),
                ") — Smoke-Teil uebersprungen, synthetische Tests decken das Format");
        return;
    }

    int files = 0;
    int effects = 0;
    int undecodedNodes = 0;
    std::vector<std::string> allWarnings;

    for (const auto& entry : std::filesystem::directory_iterator(dir))
    {
        if (!entry.is_regular_file() || entry.path().extension() != ".avs") continue;
        ++files;
        CAPTURE(entry.path().filename().string());
        const ParseResult r = parseFile(entry.path());
        CHECK(r.ok);
        if (!r.ok) continue;
        CHECK(r.effectCount() > 0);
        effects += r.effectCount();
        for (const auto& w : r.warnings)
            allWarnings.push_back(entry.path().filename().string() + ": " + w);

        // Kein Knoten darf leer ausgehen: entweder dekodiert oder Roh-Blob da
        const auto walk = [&](const auto& self, const EffectNode& node) -> void {
            for (const EffectNode& c : node.children)
            {
                if (!c.decoded)
                {
                    ++undecodedNodes;
                    CHECK((c.isList || !c.rawConfig.empty() || c.name == "Comment"));
                }
                self(self, c);
            }
        };
        walk(walk, r.root);
    }

    CHECK(files == 35);
    CHECK(effects > 0);
    MESSAGE("Korpus: ", files, " Dateien, ", effects, " Effekte, ", undecodedNodes,
            " nicht dekodierte Knoten, ", allWarnings.size(), " Warnungen");
    for (const std::string& w : allWarnings) MESSAGE("  Report: ", w);
}
