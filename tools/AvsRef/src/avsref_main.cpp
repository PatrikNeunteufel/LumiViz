/**
 ****************************************************************************************
 * @file   avsref_main.cpp
 * @brief  AvsRef — Referenz-Renderer um den ORIGINALEN vis_avs-Render-Kern
 *         (C_RenderListClass::render auf Speicher-Framebuffer, ohne Winamp/
 *         Fenster/DDraw). Gegenstueck zu AvsStandalone fuer Seite-an-Seite-
 *         und Diff-Vergleiche (Session 46).
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 * @version 1.0.0
 *
 * @details
 * Aufruf:
 *   AvsRef <presetDateiOderOrdner> [--frames N] [--out DIR] [--size WxH]
 *          [--save-every M] [--beat-period N] [--tick-hz N]
 *
 * - Audio kommt synthetisch und ist BYTE-IDENTISCH zur LumiViz-Seite:
 *   Signal wie AvsStandalone::feedSyntheticAudio (Sinus 220 Hz + Beat-Puls
 *   ~120 BPM, frame-getaktet mit t = frame/60) und Float->Byte-Abbildung wie
 *   MultiEffectVisualizer::buildVisData (kSpecGain=8 + AVS-Log-Kurve,
 *   Waveform int(w*127)&0xFF).
 * - Beat: Original-Onset-Logik aus main.cpp:290-329 (Byte-Waveform) +
 *   refineBeat (bpm.cpp, cfg_smartbeat=0 = Original-Default, deterministisch).
 *   --beat-period N erzwingt stattdessen exakt alle N Frames einen Beat.
 * - Ausgabe je Preset: letzter Frame als 32-bpp-BMP (Speicherlayout 0x00RRGGBB
 *   = BGRX, top-down) + Luma-Statistik im AvsStandalone-Format.
 ****************************************************************************************
 */

#include <windows.h>

#include "r_defs.h"
#include "rlib.h"
#include "r_list.h"
#include "avs_eelif.h"
#include "bpm.h"
#include "timing.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// windows.h-Makros min/max wieder loswerden — die AVS-Header oben sind damit
// bereits geparst, ab hier gilt std::
#undef min
#undef max

#include <algorithm>
#include <string>
#include <vector>

// Der synthetische Klang kommt seit S74 aus der GEMEINSAMEN Quelle, die auch
// die beiden Standalones einbinden — vorher stand die Formel hier als Kopie
// mit dem Kommentar „formelgleich zu …". Der Header ist bewusst Qt-frei und
// C++14-tauglich, damit dieses 32-bit-Alt-Projekt ihn uebersetzen kann.
#include "SynthAudio.hpp"

namespace
{
// std::clamp braeuchte C++17 — der Alt-Code bleibt auf dem MSVC-Default
inline float clampF(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
inline int clampI(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

/// Fester Startwert des CRT-Zufallsstroms je Preset — macht die Referenz
/// reproduzierbar (siehe Kommentar an der Aufrufstelle nach __LoadPreset).
constexpr unsigned int kRandSeed = 1u;
} // namespace

// Render-Kern-Globals (Pendant zu render.cpp, das wir nicht mitkompilieren)
C_RenderListClass* g_render_effects = NULL;
C_RLibrary* g_render_library = NULL;

namespace
{

// --- Synthetisches Audio: aus der gemeinsamen SynthAudio.hpp (S74) -----------------------
// Der Geschmack ist `Avs` — ein Beat-Faktor ueber alle Bins, so wie
// AvsStandalone es fuettert. Bit fuer Bit dasselbe Signal wie vor S74.
typedef lumi::synth::Frame SyntheticFrame;

// --- Float->Byte: EXAKT MultiEffectVisualizer::buildVisData ------------------------------
unsigned char specByte(const float* chan, int i)
{
    constexpr float kSpecGain = 8.0f;
    constexpr int kWinampRealBands = 512;
    if (i >= kWinampRealBands) return 0;
    const float s = chan[static_cast<size_t>(i) * 512 / kWinampRealBands];
    const float lin = clampF(s * kSpecGain, 0.0f, 1.0f);
    const float logv = logf(lin * 60.0f + 1.0f) / logf(60.0f);
    return static_cast<unsigned char>(clampF(logv, 0.0f, 1.0f) * 255.0f);
}

unsigned char waveByte(const float* chan, int i)
{
    const float w = chan[i];  // 576 Eintraege je Kanal -> Index 1:1
    const int sw = clampI(static_cast<int>(w * 127.0f), -128, 127);
    return static_cast<unsigned char>(sw & 0xFF);
}

/// visdata[0]=Spektrum L/R (log-skaliert), visdata[1]=Waveform L/R (signed bytes)
void buildVisData(const SyntheticFrame& in, char visdata[2][2][576])
{
    float chanW[2][576];
    float chanS[2][512];
    for (int i = 0; i < 576; ++i)
    {
        chanW[0][i] = in.wave[i * 2 + 0];
        chanW[1][i] = in.wave[i * 2 + 1];
    }
    for (int b = 0; b < 512; ++b)
    {
        chanS[0][b] = in.spec[b * 2 + 0];
        chanS[1][b] = in.spec[b * 2 + 1];
    }
    for (int ch = 0; ch < 2; ++ch)
    {
        for (int i = 0; i < 576; ++i)
        {
            visdata[0][ch][i] = static_cast<char>(specByte(chanS[ch], i));
            visdata[1][ch][i] = static_cast<char>(waveByte(chanW[ch], i));
        }
    }
}

// --- Beat-Onset: EXAKT vis_avs main.cpp:290-329 (auf den Waveform-BYTES) -----------------
struct BeatState
{
    int peak1 = 0, peak2 = 0, cnt = 0, peak1_peak = 0;

    int onset(const char visdata[2][2][576])
    {
        int lt[2] = {0, 0};
        for (int ch = 0; ch < 2; ++ch)
        {
            const unsigned char* f = reinterpret_cast<const unsigned char*>(&visdata[1][ch][0]);
            for (int x = 0; x < 576; ++x)
            {
                int r = *f++ ^ 128;
                r -= 128;
                if (r < 0) r = -r;
                lt[ch] += r;
            }
        }
        lt[0] = std::max(lt[0], lt[1]);

        peak1 = (peak1 * 125 + peak2 * 3) / 128;
        ++cnt;
        int avs_beat = 0;
        if (lt[0] >= (peak1 * 34) / 32 && lt[0] > (576 * 16))
        {
            if (cnt > 0)
            {
                cnt = 0;
                avs_beat = 1;
            }
            peak1 = (lt[0] + peak1_peak) / 2;
            peak1_peak = lt[0];
        }
        else if (lt[0] > peak2)
        {
            peak2 = lt[0];
        }
        else
        {
            peak2 = (peak2 * 14) / 16;
        }
        return avs_beat;
    }
};

// --- Ausgabe -----------------------------------------------------------------------------
struct FrameStats
{
    double meanR = 0.0, meanG = 0.0, meanB = 0.0;
    double minLuma = 1.0, maxLuma = 0.0;
};

FrameStats computeStats(const int* fb, int w, int h)
{
    FrameStats st;
    double sumR = 0.0, sumG = 0.0, sumB = 0.0;
    const size_t pixels = static_cast<size_t>(w) * h;
    for (size_t p = 0; p < pixels; ++p)
    {
        const unsigned int px = static_cast<unsigned int>(fb[p]);
        const double r = ((px >> 16) & 0xFF) / 255.0;
        const double g = ((px >> 8) & 0xFF) / 255.0;
        const double b = (px & 0xFF) / 255.0;
        sumR += r;
        sumG += g;
        sumB += b;
        const double luma = 0.2126 * r + 0.7152 * g + 0.0722 * b;
        st.minLuma = std::min(st.minLuma, luma);
        st.maxLuma = std::max(st.maxLuma, luma);
    }
    st.meanR = sumR / pixels;
    st.meanG = sumG / pixels;
    st.meanB = sumB / pixels;
    return st;
}

/// 32-bpp-BMP, top-down (negatives biHeight); Speicher-Ints sind direkt BGRX
bool writeBmp(const char* path, const int* fb, int w, int h)
{
    BITMAPFILEHEADER bfh = {};
    BITMAPINFOHEADER bih = {};
    const DWORD imageSize = static_cast<DWORD>(w) * h * 4;
    bfh.bfType = 0x4D42;  // 'BM'
    bfh.bfOffBits = sizeof(bfh) + sizeof(bih);
    bfh.bfSize = bfh.bfOffBits + imageSize;
    bih.biSize = sizeof(bih);
    bih.biWidth = w;
    bih.biHeight = -h;  // top-down wie der AVS-Framebuffer
    bih.biPlanes = 1;
    bih.biBitCount = 32;
    bih.biCompression = BI_RGB;
    bih.biSizeImage = imageSize;

    FILE* f = fopen(path, "wb");
    if (!f) return false;
    fwrite(&bfh, sizeof(bfh), 1, f);
    fwrite(&bih, sizeof(bih), 1, f);
    // Alpha-Byte nullen (FBO-Alpha ist kein Bildinhalt — Merkregel S45)
    std::vector<int> row(static_cast<size_t>(w));
    for (int y = 0; y < h; ++y)
    {
        memcpy(row.data(), fb + static_cast<size_t>(y) * w, static_cast<size_t>(w) * 4);
        for (int x = 0; x < w; ++x) row[static_cast<size_t>(x)] &= 0x00FFFFFF;
        fwrite(row.data(), 4, static_cast<size_t>(w), f);
    }
    fclose(f);
    return true;
}

std::string baseNameWithSuffix(const std::string& path)
{
    size_t slash = path.find_last_of("\\/");
    std::string name = (slash == std::string::npos) ? path : path.substr(slash + 1);
    // Punkt -> Unterstrich, wie AvsStandalone (Endung bleibt Teil des Namens)
    for (char& c : name)
        if (c == '.') c = '_';
    return name;
}

} // namespace

int main(int argc, char** argv)
{
    // Ungepuffert: bei Crash/Umleitung gehen sonst alle printf-Zeilen verloren
    setvbuf(stdout, NULL, _IONBF, 0);

    // --- Argumente -----------------------------------------------------------------------
    std::string target;
    std::string outDir = ".";
    int frames = 120;
    int width = 800, height = 600;
    int saveEvery = 0;    // 0 = nur letzter Frame
    int beatPeriod = 0;   // 0 = Original-Detektor
    int tickHz = 0;       // --tick-hz N: gettime() als Frame-Uhr (0 = Wanduhr)
    bool silence = false; // --silence: Stille statt Kalibrier-Signal
    std::string apeDir;   // --ape-dir: echte APE-Sammlung (leer = keine APEs)
    // --audio-muster (S74): klassisch = Sinus+Beat-Puls wie bisher, musik =
    // aus einer Aufnahme abgeleitete Huellkurven samt Beat-Spur. Beides kommt
    // aus SynthAudio.hpp, die LumiViz genauso einbindet — deshalb bleiben die
    // Vergleiche in BEIDEN Mustern gueltig.
    lumi::synth::Muster muster = lumi::synth::Muster::Klassisch;
    for (int i = 1; i < argc; ++i)
    {
        const std::string a = argv[i];
        auto next = [&](int& v) { if (i + 1 < argc) v = atoi(argv[++i]); };
        if (a == "--frames") next(frames);
        else if (a == "--save-every") next(saveEvery);
        else if (a == "--beat-period") next(beatPeriod);
        else if (a == "--tick-hz") next(tickHz);
        else if (a == "--silence") silence = true;
        else if (a == "--out" && i + 1 < argc) outDir = argv[++i];
        else if (a == "--ape-dir" && i + 1 < argc) apeDir = argv[++i];
        else if (a == "--audio-muster" && i + 1 < argc)
        {
            if (!lumi::synth::musterAusText(argv[++i], muster))
            {
                fprintf(stderr, "FEHLER: --audio-muster erwartet klassisch|musik\n");
                return 2;
            }
        }
        else if (a == "--size" && i + 1 < argc)
        {
            if (sscanf(argv[++i], "%dx%d", &width, &height) != 2 || width < 64 || height < 64)
            {
                fprintf(stderr, "FEHLER: --size erwartet WxH (min 64x64)\n");
                return 2;
            }
        }
        else if (target.empty()) target = a;
        else
        {
            fprintf(stderr, "FEHLER: unbekanntes Argument '%s'\n", a.c_str());
            return 2;
        }
    }
    if (target.empty())
    {
        fprintf(stderr,
                "Aufruf: AvsRef <preset.avs|ordner> [--frames N] [--out DIR]\n"
                "        [--size WxH] [--save-every M] [--beat-period N]\n"
                "        [--tick-hz N: gettime() als Frame-Uhr statt Wanduhr]\n");
        return 2;
    }

    // --- Preset-Liste ----------------------------------------------------------------------
    std::vector<std::string> presets;
    const DWORD attr = GetFileAttributesA(target.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        fprintf(stderr, "FEHLER: '%s' nicht gefunden\n", target.c_str());
        return 2;
    }
    if (attr & FILE_ATTRIBUTE_DIRECTORY)
    {
        WIN32_FIND_DATAA fd;
        const std::string mask = target + "\\*.avs";
        HANDLE h = FindFirstFileA(mask.c_str(), &fd);
        if (h != INVALID_HANDLE_VALUE)
        {
            do
            {
                presets.push_back(target + "\\" + fd.cFileName);
            } while (FindNextFileA(h, &fd));
            FindClose(h);
        }
        std::sort(presets.begin(), presets.end());
    }
    else
    {
        presets.push_back(target);
    }
    if (presets.empty())
    {
        fprintf(stderr, "FEHLER: keine .avs-Presets unter '%s'\n", target.c_str());
        return 2;
    }
    printf("[AvsRef] %d Preset(s), %dx%d, %d Frames\n",
           static_cast<int>(presets.size()), width, height, frames);

    CreateDirectoryA(outDir.c_str(), NULL);

    // --- Kern-Init (Pendant zu Render_Init, ohne Winamp) -----------------------------------
    // g_path: APE-Verzeichnis fuer den Registry-Scan. Default ist ein (i. d. R.
    // nicht existentes) Unterverzeichnis, damit keine Fremd-DLLs geladen werden;
    // --ape-dir zeigt bewusst auf eine echte Sammlung, damit Presets MIT APEs
    // vergleichbar werden (sonst misst man unsere APE-Nachbauten gegen nichts).
    if (!apeDir.empty())
    {
        strncpy(g_path, apeDir.c_str(), 1023);
        g_path[1023] = 0;
        printf("[AvsRef] APE-Verzeichnis: %s\n", g_path);
    }
    else
    {
        GetModuleFileNameA(NULL, g_path, 1024);  // g_path[1024], r_defs.h unbounded
        char* p = g_path + strlen(g_path);
        while (p > g_path && *p != '\\') --p;
        *p = 0;
        strcat(g_path, "\\ape");
    }

    timingInit();
    {
        for (int j = 0; j < 256; ++j)
            for (int i = 0; i < 256; ++i)
                g_blendtable[i][j] = static_cast<unsigned char>((i / 255.0) * (float)j);
    }
    AVS_EEL_IF_init();
    initBpm();
    g_render_library = new C_RLibrary();
    g_render_effects = new C_RenderListClass(1);

    // EEL-Selbsttest (nur AVSREF_DEBUG): Compile + JIT-Execute eines trivialen
    // Ausdrucks — prueft den kompletten ns-eel-Pfad isoliert vom Renderer
    if (getenv("AVSREF_DEBUG"))
    {
        NSEEL_VMCTX vm = NSEEL_VM_alloc();
        double* nvar = NSEEL_VM_regvar(vm, "n");
        const int handle = AVS_EEL_IF_Compile((int)vm, "n=200");
        if (handle == 0)
        {
            fprintf(stderr, "[dbg] EEL-Compile FEHLER: %s\n", last_error_string);
        }
        else
        {
            char zeroVis[2][2][576] = {};
            AVS_EEL_IF_Execute((void*)handle, zeroVis);
            fprintf(stderr, "[dbg] EEL-Selbsttest: n=%f (erwartet 200)\n", *nvar);
            NSEEL_code_free((NSEEL_CODEHANDLE)handle);
        }
        AVS_EEL_IF_VM_free(vm);
    }

    // --- Render-Schleife -------------------------------------------------------------------
    // Kleines Sicherheitspolster hinter w*h (Alt-Code; gelesen wird nur w*h)
    std::vector<int> fb1(static_cast<size_t>(width) * (height + 4), 0);
    std::vector<int> fb2(static_cast<size_t>(width) * (height + 4), 0);
    int rc = 0;

    for (const std::string& preset : presets)
    {
        printf("\n[AvsRef] === %s ===\n", preset.c_str());
        g_render_effects->__LoadPreset(const_cast<char*>(preset.c_str()), 1);
        // Zufallsstrom NACH dem Laden auf einen festen Wert setzen (S52).
        // Grund: `r_chanshift.cpp:340` ruft `srand((unsigned int)time(0))` in
        // seinem load_config — jedes Preset mit einem Channel Shift saet den
        // globalen CRT-Strom also mit der aktuellen SEKUNDE neu, und die
        // Referenz wird damit von Lauf zu Lauf ein anderes Bild liefern.
        // Gemessen (vier Laeufe, jeder Renderer mit sich selbst verglichen):
        // AvsStandalone MAE 0,0000, AvsRef 0,055-0,064 — die Streuung kam
        // vollstaendig von hier. Ein Vergleich gegen eine nicht reproduzierbare
        // Referenz hat eine Rauschgrenze, unter die niemand messen kann.
        // Der Wert selbst ist beliebig, nur konstant muss er sein; er steht
        // NACH dem Laden, damit er jedes load_config ueberschreibt.
        srand(kRandSeed);
        if (getenv("AVSREF_DEBUG")) fprintf(stderr, "[dbg] __LoadPreset zurueck\n");
        if (g_render_effects->getNumRenders() == 0)
        {
            fprintf(stderr, "[AvsRef] LADEN FEHLGESCHLAGEN (leere Renderliste)\n");
            rc = 1;
            continue;
        }

        // Frischer Zustand je Preset (Framebuffer + Beat); EEL-reg/megabuf
        // bleiben wie im Original prozessweit bestehen
        std::fill(fb1.begin(), fb1.end(), 0);
        std::fill(fb2.begin(), fb2.end(), 0);
        BeatState beatState;
        int s = 0;
        char visdata[2][2][576];

        for (int frame = 0; frame < frames; ++frame)
        {
            // Virtuelle Frame-Uhr fuer EEL-gettime() (patched/avs_eelif.cpp):
            // ohne sie misst ein selbstvermessendes Preset (gettime-FPS-
            // Zaehlfenster) die Batch-Rendergeschwindigkeit statt einer
            // Bildrate — gegen keinen anderen Renderer vergleichbar (S59).
            if (tickHz > 0)
            {
                extern int g_avsref_tick_ms;
                g_avsref_tick_ms = (int)((long long)frame * 1000 / tickHz);
            }
            lumi::synth::Optionen synthOpt;
            synthOpt.muster = muster;
            synthOpt.geschmack = lumi::synth::Geschmack::Avs;
            synthOpt.stille = silence;
            SyntheticFrame synth;
            lumi::synth::erzeuge(frame, synthOpt, synth);
            buildVisData(synth, visdata);

            // Rangfolge: ausdrueckliches --beat-period schlaegt alles. Nur so
            // laesst sich der Beat zwischen den Mustern KONSTANT halten — und
            // genau das braucht die Frage, ob ein Befund am Renderpfad haengt
            // oder bloss am Pruefsignal (S74).
            int isBeat;
            if (beatPeriod > 0)
                isBeat = (frame % beatPeriod) == 0 ? 1 : 0;
            else if (muster == lumi::synth::Muster::Musik)
                // Beat-SPUR der Vorlage — dieselben Flags, die LumiViz per
                // setBeatTrackOverride bekommt (S74)
                isBeat = lumi::synth::istBeat(frame, synthOpt) ? 1 : 0;
            else
                isBeat = refineBeat(beatState.onset(visdata));

            int* fb = s ? fb2.data() : fb1.data();
            int* fbout = s ? fb1.data() : fb2.data();
            if (getenv("AVSREF_DEBUG")) fprintf(stderr, "[dbg] render frame %d...\n", frame);
            const int t = g_render_effects->render(visdata, isBeat, fb, fbout, width, height);
            if (t & 1) s ^= 1;

            const bool last = frame == frames - 1;
            if (last || (saveEvery > 0 && frame % saveEvery == 0))
            {
                const int* result = s ? fb2.data() : fb1.data();
                char file[MAX_PATH * 2];
                _snprintf(file, sizeof(file), "%s\\%s_f%04d_ref.bmp", outDir.c_str(),
                          baseNameWithSuffix(preset).c_str(), frame + 1);
                const FrameStats st = computeStats(result, width, height);
                if (!writeBmp(file, result, width, height))
                {
                    fprintf(stderr, "[AvsRef] BMP-Schreibfehler: %s\n", file);
                    rc = 1;
                }
                printf("[AvsRef] Frame %d: %s — mean RGB=(%.3f, %.3f, %.3f), "
                       "Luma min=%.3f max=%.3f\n",
                       frame + 1, file, st.meanR, st.meanG, st.meanB, st.minLuma, st.maxLuma);
            }
        }
    }

    delete g_render_effects;
    g_render_effects = NULL;
    delete g_render_library;
    g_render_library = NULL;
    AVS_EEL_IF_quit();
    printf("\n[AvsRef] fertig, rc=%d\n", rc);
    return rc;
}
