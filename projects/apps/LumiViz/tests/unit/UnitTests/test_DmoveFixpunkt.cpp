/**
 ****************************************************************************************
 * @file   test_DmoveFixpunkt.cpp
 * @brief  Gate fuer den r_dmove-Fixpunkt-Warp (S49): der Shader rechnet die
 *         separable GANZZAHL-Interpolation des Originals in geschlossener Form
 *         (Band, Restweg, trunkierte Bandsteigung) — hier gegen eine
 *         zeilengetreue CPU-Nachbildung der Originalschleife geprueft
 *         (r_dmove.cpp:372-578) inkl. BLEND4_16/BLEND_ADJ.
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/MultiEffectVisualizer.hpp"
#include "visualizers/modules/scripting/ScriptGridModule.hpp"

#include <QGuiApplication>
#include <QImage>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QSurfaceFormat>

#include <algorithm>
#include <vector>

using namespace lumi::multieffect;
using lumi::modules::GridNodeFx;

namespace {

// Nicht quadratisch (Aspekt-Fehler verstecken sich in quadratischen Gates) und
// beide Kanten >= 64 — kleiner klemmt MultiEffectVisualizer die Surface hoch.
constexpr int kW = 96;
constexpr int kH = 72;
constexpr int kXres = 5;
constexpr int kYres = 4;

/// Ein Bildpunkt als drei Kanaele (AVS haelt sie in EINEM int; die Kanalfolge
/// ist fuer die Mischung egal, solange sie durchgehend gleich bleibt).
struct Rgb
{
    int r = 0, g = 0, b = 0;
};

/// BLEND_ADJ (r_defs.h, MMX-Zweig): (a*v + b*(255-v)) >> 8 — EINE Trunkierung.
Rgb blendAdj(const Rgb& a, const Rgb& b, int v)
{
    return {(a.r * v + b.r * (255 - v)) >> 8, (a.g * v + b.g * (255 - v)) >> 8,
            (a.b * v + b.b * (255 - v)) >> 8};
}

/// BLEND4_16 (r_defs.h, MMX-Zweig): Gewichte (p>>8)&0xff gegen 255-w
/// (mmx_blend4_revn = 0x00ff), drei trunkierende Stufen.
Rgb blend4_16(const std::vector<Rgb>& in, int w, int addr, int xp, int yp)
{
    const int xw = (xp >> 8) & 0xff;
    const int yw = (yp >> 8) & 0xff;
    auto at = [&](int i) { return in[static_cast<std::size_t>(std::clamp(
                              i, 0, static_cast<int>(in.size()) - 1))]; };
    const Rgb c00 = at(addr), c10 = at(addr + 1);
    const Rgb c01 = at(addr + w), c11 = at(addr + w + 1);
    auto mix = [](int a, int b, int t) { return (a * (255 - t) + b * t) >> 8; };
    const Rgb top{mix(c00.r, c10.r, xw), mix(c00.g, c10.g, xw), mix(c00.b, c10.b, xw)};
    const Rgb bot{mix(c01.r, c11.r, xw), mix(c01.g, c11.g, xw), mix(c01.b, c11.b, xw)};
    return {mix(top.r, bot.r, yw), mix(top.g, bot.g, yw), mix(top.b, bot.b, yw)};
}

/**
 * @brief Zeilengetreue Nachbildung von C_THISCLASS::smp_render (r_dmove.cpp:372-578)
 *
 * Ein Thread (start_l = 0, end_l = h), Reihenfolge und Ganzzahl-Divisionen wie
 * im Original. Die Eingabe liegt in AVS-Zeilenordnung (0 = oben).
 */
std::vector<Rgb> dmoveReference(const std::vector<Rgb>& in, int w, int h,
                                const std::vector<GridNodeFx>& tab, int XRES,
                                int YRES, bool wrap, bool blend, bool subpixel)
{
    std::vector<Rgb> out(in.size());
    const int wAdj = subpixel ? (w - 2) << 16 : (w - 1) << 16;
    const int hAdj = subpixel ? (h - 2) << 16 : (h - 1) << 16;

    // Tabelle wie smp_begin: ohne wrap wird schon hier geklemmt (r_dmove:349-355)
    std::vector<int> rd(static_cast<std::size_t>(XRES) * YRES * 3);
    for (std::size_t i = 0; i < static_cast<std::size_t>(XRES) * YRES; ++i)
    {
        int tx = tab[i].x;
        int ty = tab[i].y;
        if (!wrap)
        {
            tx = std::clamp(tx, 0, wAdj);
            ty = std::clamp(ty, 0, hAdj);
        }
        rd[i * 3 + 0] = tx;
        rd[i * 3 + 1] = ty;
        rd[i * 3 + 2] = tab[i].a;
    }

    std::vector<int> interp(static_cast<std::size_t>(XRES) * 6 + 6, 0);
    const int xcDpos = (w << 16) / (XRES - 1);
    const int ycDpos = (h << 16) / (YRES - 1);
    int rdOff = 0;      // Index in rd (Originale: rdtab)
    int outPos = 0;     // Schreibposition (Originale: out/blendin)
    int ycPos = 0, lypos = 0, yl = h;
    while (yl > 0)
    {
        ycPos += ycDpos;
        int yseek = (ycPos >> 16) - lypos;
        if (yseek == 0) break;  // Original bricht ab (Band der Breite 0)
        lypos = ycPos >> 16;
        const int xr3 = XRES * 3;
        for (int l = 0; l < XRES; ++l)
        {
            const int t1 = rd[rdOff + 0], t2 = rd[rdOff + 1], t3 = rd[rdOff + 2];
            interp[l * 6 + 0] = t1;
            interp[l * 6 + 1] = t2;
            interp[l * 6 + 2] = (rd[rdOff + xr3 + 0] - t1) / yseek;
            interp[l * 6 + 3] = (rd[rdOff + xr3 + 1] - t2) / yseek;
            interp[l * 6 + 4] = t3;
            interp[l * 6 + 5] = (rd[rdOff + xr3 + 2] - t3) / yseek;
            rdOff += 3;
        }
        if (yseek > yl) yseek = yl;
        yl -= yseek;

        while (yseek-- > 0)
        {
            int stab = 0, l = w, lpos = 0, xcPos = 0;
            while (l > 0)
            {
                xcPos += xcDpos;
                int seek = (xcPos >> 16) - lpos;
                if (seek == 0) return out;
                lpos = xcPos >> 16;
                int xp = interp[stab + 0];
                int yp = interp[stab + 1];
                int ap = interp[stab + 4];
                const int dA = (interp[stab + 10] - ap) / seek;
                int dX = (interp[stab + 6] - xp) / seek;
                int dY = (interp[stab + 7] - yp) / seek;
                interp[stab + 0] += interp[stab + 2];
                interp[stab + 1] += interp[stab + 3];
                interp[stab + 4] += interp[stab + 5];
                stab += 6;

                if (seek > l) seek = l;
                l -= seek;
                if (wrap)
                {
                    xp %= wAdj;
                    yp %= hAdj;
                    if (xp < 0) xp += wAdj;
                    if (yp < 0) yp += hAdj;
                    // r_dmove.cpp:561-565 — genau diese Schranke macht die EINE
                    // Korrektur je Schritt zum vollen Modulo (Argument wie Roto).
                    dX = std::clamp(dX, -wAdj + 1, wAdj - 1);
                    dY = std::clamp(dY, -hAdj + 1, hAdj - 1);
                }
                else
                {
                    xp = std::clamp(xp, 0, wAdj - 1);
                    yp = std::clamp(yp, 0, hAdj - 1);
                }
                while (seek-- > 0)
                {
                    if (wrap)
                    {
                        if (xp < 0) xp += wAdj;
                        else if (xp >= wAdj) xp -= wAdj;
                        if (yp < 0) yp += hAdj;
                        else if (yp >= hAdj) yp -= hAdj;
                    }
                    else
                    {
                        xp = std::clamp(xp, 0, wAdj - 1);
                        yp = std::clamp(yp, 0, hAdj - 1);
                    }
                    const int addr = (xp >> 16) + (yp >> 16) * w;
                    const Rgb moved =
                        subpixel ? blend4_16(in, w, addr, xp, yp)
                                 : in[static_cast<std::size_t>(std::clamp(
                                       addr, 0, static_cast<int>(in.size()) - 1))];
                    out[static_cast<std::size_t>(outPos)] =
                        blend ? blendAdj(moved, in[static_cast<std::size_t>(outPos)],
                                         ap >> 16)
                              : moved;
                    ++outPos;
                    xp += dX;
                    yp += dY;
                    ap += dA;
                }
            }
            interp[stab + 0] += interp[stab + 2];
            interp[stab + 1] += interp[stab + 3];
            interp[stab + 4] += interp[stab + 5];
        }
    }
    return out;
}

/// Bandzerlegung wie im Shader: (Index, Rest im Band, Bandbreite)
struct Band
{
    int idx, rest, width;
};
Band bandOf(int p, int dpos, int n)
{
    int b = std::clamp((p << 16) / dpos, 0, n - 2);
    if (((b + 1) * dpos) >> 16 <= p) b = std::min(b + 1, n - 2);
    if ((b * dpos) >> 16 > p) b = std::max(b - 1, 0);
    const int lo = (b * dpos) >> 16;
    const int hi = ((b + 1) * dpos) >> 16;
    return {b, p - lo, std::max(hi - lo, 1)};
}

/// Die geschlossene Form des Shaders, in C++ nachgezogen (Formel-Gate ohne GL).
std::vector<Rgb> dmoveClosedForm(const std::vector<Rgb>& in, int w, int h,
                                 const std::vector<GridNodeFx>& tab, int XRES,
                                 int YRES, bool wrap, bool blend, bool subpixel)
{
    std::vector<Rgb> out(in.size());
    const int wAdj = subpixel ? (w - 2) << 16 : (w - 1) << 16;
    const int hAdj = subpixel ? (h - 2) << 16 : (h - 1) << 16;
    const int xcDpos = (w << 16) / (XRES - 1);
    const int ycDpos = (h << 16) / (YRES - 1);
    auto node = [&](int col, int row, int comp) {
        const GridNodeFx& n = tab[static_cast<std::size_t>(row) * XRES + col];
        int v = comp == 0 ? n.x : (comp == 1 ? n.y : n.a);
        if (!wrap && comp < 2) v = std::clamp(v, 0, comp == 0 ? wAdj : hAdj);
        return v;
    };
    for (int dy = 0; dy < h; ++dy)
    {
        const Band by = bandOf(dy, ycDpos, YRES);
        for (int dx = 0; dx < w; ++dx)
        {
            const Band bx = bandOf(dx, xcDpos, XRES);
            int v[3];
            for (int c = 0; c < 3; ++c)
            {
                const int t00 = node(bx.idx, by.idx, c);
                const int t01 = node(bx.idx, by.idx + 1, c);
                const int t10 = node(bx.idx + 1, by.idx, c);
                const int t11 = node(bx.idx + 1, by.idx + 1, c);
                const int left = t00 + by.rest * ((t01 - t00) / by.width);
                const int right = t10 + by.rest * ((t11 - t10) / by.width);
                v[c] = left + bx.rest * ((right - left) / bx.width);
            }
            int xp = v[0], yp = v[1];
            if (wrap)
            {
                xp %= wAdj;
                yp %= hAdj;
                if (xp < 0) xp += wAdj;
                if (yp < 0) yp += hAdj;
            }
            else
            {
                xp = std::clamp(xp, 0, wAdj - 1);
                yp = std::clamp(yp, 0, hAdj - 1);
            }
            const int addr = (xp >> 16) + (yp >> 16) * w;
            const Rgb moved = subpixel ? blend4_16(in, w, addr, xp, yp)
                                       : in[static_cast<std::size_t>(std::clamp(
                                             addr, 0, static_cast<int>(in.size()) - 1))];
            const std::size_t o = static_cast<std::size_t>(dy) * w + dx;
            out[o] = blend ? blendAdj(moved, in[o], std::clamp(v[2] >> 16, 0, 255))
                           : moved;
        }
    }
    return out;
}

/// QImage (GL, y+ = oben) -> AVS-Zeilenordnung
std::vector<Rgb> toAvs(const QImage& img)
{
    std::vector<Rgb> px(static_cast<std::size_t>(img.width()) * img.height());
    for (int y = 0; y < img.height(); ++y)
    {
        for (int x = 0; x < img.width(); ++x)
        {
            const QColor c = img.pixelColor(x, y);  // QImage-Zeile 0 = oben
            px[static_cast<std::size_t>(y) * img.width() + x] = {c.red(), c.green(),
                                                                 c.blue()};
        }
    }
    return px;
}

}  // namespace

TEST_CASE("MovementTabelle: r_trans-Packung (Offset + 5-Bit-Subpixel) je Pixel")
{
    // r_trans hat kein Gitter: die Tabelle haelt je PIXEL einen Eintrag. Geprueft
    // wird die Packung an einer Verschiebung, deren Ziel sich von Hand ausrechnen
    // laesst (rect-Modus, x/y um ein Achtel nach rechts/unten).
    constexpr int w = 64, h = 48;
    lumi::modules::ScriptGridModule grid;
    grid.setPointCode("x=x+0.25;y=y+0.25");
    grid.setRectCoords(true);

    std::vector<int> tab;
    REQUIRE(grid.buildTransTable(w, h, /*wrap*/ false, /*subpixel*/ false, tab));
    REQUIRE(tab.size() == static_cast<std::size_t>(w) * h);
    // r_trans.cpp:478-479 + 505-508: ow = (int)((x+1)*w2 + 0.5) mit w2 = w/2
    for (int y = 4; y < h - 8; ++y)
    {
        for (int x = 4; x < w - 12; ++x)
        {
            const int want = (x + w / 8) + (y + h / 8) * w;
            CHECK(tab[static_cast<std::size_t>(y) * w + x] == want);
        }
    }

    // Mit Subpixel liegen die 5-Bit-Anteile in den oberen Bits; die Verschiebung
    // trifft hier exakt Pixelmitten, die Anteile sind also 0.
    REQUIRE(grid.buildTransTable(w, h, /*wrap*/ false, /*subpixel*/ true, tab));
    const int probe = tab[static_cast<std::size_t>(10) * w + 10];
    CHECK((probe & ((1 << 22) - 1)) == (10 + w / 8) + (10 + h / 8) * w);
    CHECK(((probe >> 24) & (31 << 3)) == 0);
    CHECK(((probe >> 19) & (31 << 3)) == 0);

    // Ohne Punkt-Slot gibt es keine Tabelle (Aufrufer faellt zurueck).
    lumi::modules::ScriptGridModule leer;
    std::vector<int> none;
    CHECK_FALSE(leer.buildTransTable(w, h, false, true, none));
}

TEST_CASE("DmoveFixpunkt: Shader deckt sich mit der r_dmove-Originalschleife")
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
        MESSAGE("Kein GL-Kontext moeglich (Headless?) — Gate uebersprungen");
        return;
    }
    QOffscreenSurface surface;
    surface.setFormat(ctx.format());
    surface.create();
    REQUIRE(surface.isValid());
    REQUIRE(ctx.makeCurrent(&surface));

    // Quellbild: ein SuperScope-Streckenzug (kontrastreich, nicht symmetrisch —
    // ein glattes Bild wuerde Interpolationsfehler verstecken).
    const char* kPoint = "d=d*0.87;r=r+0.35";

    ChainNode root;
    root.params = ListParams{};
    ChainNode clear;
    clear.params = ClearParams{};
    ChainNode scope;
    SuperScopeParams sp;
    sp.initCode = "n=64";
    sp.pointCode = "x=cos(i*7)*0.8;y=sin(i*11)*0.8;red=i;green=1-i;blue=0.5";
    sp.renderMode = 0;
    sp.dotSize = 5.0f;
    scope.params = sp;
    ChainNode dmove;
    DynamicMovementParams dp;
    dp.pointCode = kPoint;
    dp.xres = kXres;
    dp.yres = kYres;
    dp.subpixel = true;
    dp.wrap = false;
    dp.blend = false;
    dmove.params = dp;
    dmove.enabled = false;

    root.children.push_back(std::move(clear));
    root.children.push_back(std::move(scope));
    root.children.push_back(std::move(dmove));

    MultiEffectVisualizer vis;
    vis.setChain(std::move(root));
    vis.initialize();
    vis.resize(QSize(kW, kH));
    ctx.functions()->glViewport(0, 0, kW, kH);

    const float dt = 1.0f / 60.0f;
    vis.render(dt);
    const QImage base = vis.debugGrabRootSurface();  // Quellbild ohne Warp
    REQUIRE(!base.isNull());
    MESSAGE("Surface: " << base.width() << "x" << base.height());
    REQUIRE(base.width() == kW);
    REQUIRE(base.height() == kH);
    const std::vector<Rgb> src = toAvs(base);
    // Das Quellbild muss ueber die Frames stabil sein, sonst vergleicht das Gate
    // Aepfel mit Birnen (Clear + rein i-abhaengiges Punkt-Skript = deterministisch).
    vis.render(dt);
    const std::vector<Rgb> src2 = toAvs(vis.debugGrabRootSurface());
    long long baseDrift = 0;
    for (std::size_t i = 0; i < src.size(); ++i)
        if (src[i].r != src2[i].r || src[i].g != src2[i].g || src[i].b != src2[i].b)
            ++baseDrift;
    REQUIRE(baseDrift == 0);

    // Dieselbe Gittertabelle wie die Chain (gleiches Modul, gleiche Groesse).
    lumi::modules::ScriptGridModule grid;
    grid.setPointCode(kPoint);
    grid.setGridSize(kXres, kYres);
    grid.setRectCoords(false);
    grid.setAvsGridPositions(true);  // wie der Dynamic-Movement-Knoten
    grid.execute(static_cast<float>(kW), static_cast<float>(kH), false, dt);
    const std::vector<GridNodeFx> tab = grid.fieldFx();
    REQUIRE(tab.size() == static_cast<std::size_t>(kXres) * kYres);

    // Diagnose: reine Translation im rect-Modus. Eine Spiegelung der
    // Zeilenordnung faellt hier sofort auf (Inhalt wandert nach unten statt oben).
    {
        auto& p = std::get<DynamicMovementParams>(vis.chain().children[2].params);
        p.rectCoords = true;
        p.pointCode = "x=x+0.25;y=y+0.25";
        p.wrap = false;
        p.blend = false;
        p.subpixel = false;
        vis.chain().children[2].enabled = true;
        vis.recompileChain();
        vis.render(dt);
        const std::vector<Rgb> got = toAvs(vis.debugGrabRootSurface());
        // Erwartet: Quelle = dest + (8, 6). Welche Verschiebung passt wirklich?
        int bestOx = 99, bestOy = 99;
        long long best = -1;
        for (int oy = -12; oy <= 12; ++oy)
            for (int ox = -12; ox <= 12; ++ox)
            {
                long long hit = 0;
                for (int y = 12; y < kH - 12; ++y)
                    for (int x = 12; x < kW - 12; ++x)
                    {
                        const Rgb& a = got[static_cast<std::size_t>(y) * kW + x];
                        const Rgb& b = src[static_cast<std::size_t>(y + oy) * kW + (x + ox)];
                        if (a.r == b.r && a.g == b.g && a.b == b.b) ++hit;
                    }
                if (hit > best) { best = hit; bestOx = ox; bestOy = oy; }
            }
        CHECK(bestOx == kW / 8);
        CHECK(bestOy == kH / 8);
        MESSAGE("Translations-Diagnose: beste Verschiebung (" << bestOx << ","
                << bestOy << "), Treffer " << best << " von "
                << (kW - 24) * (kH - 24) << " (erwartet " << kW / 8 << ","
                << kH / 8 << ")");
        p.rectCoords = false;
        p.pointCode = kPoint;
    }

    struct Variant
    {
        bool wrap, blend, subpixel;
        const char* name;
    };
    const Variant variants[] = {
        {false, false, true, "clamp+subpixel"},
        {false, false, false, "clamp+nearest"},
        {true, false, true, "wrap+subpixel"},
        {true, true, true, "wrap+blend"},
    };

    for (const Variant& v : variants)
    {
        CAPTURE(v.name);
        auto& p = std::get<DynamicMovementParams>(vis.chain().children[2].params);
        p.wrap = v.wrap;
        p.blend = v.blend;
        p.subpixel = v.subpixel;
        vis.chain().children[2].enabled = true;
        vis.recompileChain();
        vis.render(dt);
        const QImage warped = vis.debugGrabRootSurface();
        REQUIRE(!warped.isNull());
        const std::vector<Rgb> got = toAvs(warped);
        const std::vector<Rgb> want =
            dmoveReference(src, kW, kH, tab, kXres, kYres, v.wrap, v.blend, v.subpixel);
        // Zwischenstufe: dieselbe Rechnung geschlossen (= was der Shader tut).
        const std::vector<Rgb> closed =
            dmoveClosedForm(src, kW, kH, tab, kXres, kYres, v.wrap, v.blend, v.subpixel);
        long long closedDiff = 0;
        for (std::size_t i = 0; i < closed.size(); ++i)
            if (closed[i].r != want[i].r || closed[i].g != want[i].g ||
                closed[i].b != want[i].b)
                ++closedDiff;
        CHECK(closedDiff == 0);

        int worst = 0;
        long long diffPixels = 0;
        for (std::size_t i = 0; i < got.size(); ++i)
        {
            const int d = std::max({std::abs(got[i].r - want[i].r),
                                    std::abs(got[i].g - want[i].g),
                                    std::abs(got[i].b - want[i].b)});
            worst = std::max(worst, d);
            if (d > 1) ++diffPixels;
        }
        // 8-Bit-Readback: +-1 deckt die Rundung des Textur-Fetches ab, mehr
        // waere ein echter Rechenunterschied zur Originalschleife.
        CHECK(worst <= 1);
        CHECK(diffPixels == 0);
    }
}
