/**
 ****************************************************************************************
 * @file   milkdropref_main.cpp
 * @brief  MilkdropRef — Ground-Truth-Renderer um den ORIGINALEN MilkDrop3-Kern
 *         (Session 63, AvsRef-Vorbild): rendert .milk-Presets mit vis_milk2
 *         (D3D9) in ein unsichtbares Fenster, dumpt den letzten Frame als BMP
 *         und meldet Pixel-Statistik im AvsStandalone-Format.
 *
 * @author Patrik Neunteufel
 * @date   August 2026
 * @version 1.0.0
 *
 * @details
 * Aufruf:
 *   MilkdropRef <preset.milk|ordner> [--frames N] [--out DIR] [--size WxH]
 *               [--silence] [--show]
 *
 * - Audio: synthetisch, formelgleich zu MilkdropStandalone::feedSyntheticAudio
 *   (t = frame/60, 576 Samples 8-bit-PCM, 128-Mitte) — `--silence` liefert
 *   Null-Signal (Hunger-Test).
 * - Zeit: der Kern misst WANDUHR (DoTime); EnforceMaxFPS taktet windowed auf
 *   60 fps, damit Sim-Zeit ~ frame/60 bleibt. Vergleich deshalb IMMER ueber
 *   Statistik/Montagen, nie Pixelgleichheit (§9, GPU + Wanduhr).
 * - Texturen/Sprites: m_szMilkdrop2Path wird auf den GROSSELTERN-Ordner des
 *   Presets gesetzt (Asset-Layout asset/Milkdrop3/{presets,textures,sprites}).
 ****************************************************************************************
 */

#include <windows.h>
#include <d3d9.h>

#include "plugin.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

// Der synthetische Klang kommt seit S74 aus der GEMEINSAMEN Quelle, die auch
// die beiden LumiViz-Standalones und AvsRef einbinden — vorher stand die
// Formel hier als Kopie mit dem Kommentar "formelgleich zu ...".
#include "SynthAudio.hpp"

// --- Globals, die der Kern erwartet (Vorbild Milkdrop2PcmVisualizer.cpp) ---------------
CPlugin g_plugin;
HINSTANCE api_orig_hinstance = nullptr;
_locale_t g_use_C_locale;
char keyMappings[8];

namespace
{

constexpr double kPi = 3.14159265358979323846;
constexpr int kSamples = 576;

IDirect3D9* g_d3d9 = nullptr;
IDirect3DDevice9* g_device = nullptr;
D3DPRESENT_PARAMETERS g_pp;

/// D3D9-Device wie in der Vorlage (windowed, COPY-SwapEffect: Backbuffer
/// bleibt nach Present lesbar — Grundlage fuer den Readback)
bool initD3d(HWND hwnd, int width, int height)
{
    g_d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
    if (g_d3d9 == nullptr) return false;

    D3DDISPLAYMODE mode;
    g_d3d9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode);

    ZeroMemory(&g_pp, sizeof(g_pp));
    g_pp.BackBufferCount = 1;
    g_pp.BackBufferFormat = mode.Format;
    g_pp.BackBufferWidth = width;
    g_pp.BackBufferHeight = height;
    g_pp.SwapEffect = D3DSWAPEFFECT_COPY;
    g_pp.EnableAutoDepthStencil = TRUE;
    g_pp.AutoDepthStencilFormat = D3DFMT_D24X8;
    g_pp.Windowed = TRUE;
    g_pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    g_pp.MultiSampleType = D3DMULTISAMPLE_NONE;
    g_pp.hDeviceWindow = hwnd;

    const HRESULT hr = g_d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
                                            D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_pp,
                                            &g_device);
    if (FAILED(hr))
    {
        std::fprintf(stderr, "FEHLER: CreateDevice 0x%08lx\n", hr);
        return false;
    }
    return true;
}

/// Synthetisches Audio aus der GEMEINSAMEN SynthAudio.hpp (S74) als 8-bit-PCM
/// mit 128-Mitte. Bis S74 stand die Formel hier als Kopie mit dem Kommentar
/// „formelgleich zu MilkdropStandalone::feedSyntheticAudio" — jetzt binden
/// beide dieselbe Datei ein. Vorgabe ergibt Bit fuer Bit dasselbe PCM.
///
/// MilkDrop rechnet seine FFT SELBST aus diesem PCM; das Spektrum, das
/// LumiViz seinem Milkdrop-Knoten fertig hinlegt, kommt hier also gar nicht
/// vor. Im Muster `musik` faellt das weniger ins Gewicht als im klassischen:
/// dort stammen Wellenform und Spektrum aus denselben Bandhuellkurven, eine
/// FFT ueber die Wellenform ergibt also dasselbe Bild.
void buildPcm(int frame, const lumi::synth::Optionen& opt, unsigned char* left,
              unsigned char* right)
{
    if (opt.stille)
    {
        std::memset(left, 128, kSamples);
        std::memset(right, 128, kSamples);
        return;
    }
    static lumi::synth::Frame klang;
    lumi::synth::erzeuge(frame, opt, klang);
    for (int i = 0; i < kSamples; ++i)
    {
        left[i] = static_cast<unsigned char>(
            std::lround(klang.wave[i * 2 + 0] * 127.0) + 128);
        right[i] = static_cast<unsigned char>(
            std::lround(klang.wave[i * 2 + 1] * 127.0) + 128);
    }
}

struct FrameStats
{
    double meanR = 0.0, meanG = 0.0, meanB = 0.0;
    double minLuma = 1.0, maxLuma = 0.0;
};

/// Backbuffer -> Systemspeicher -> BMP (32 bpp, top-down, BGRX) + Statistik
bool dumpBackbuffer(const std::wstring& bmpPath, FrameStats* outStats)
{
    IDirect3DSurface9* back = nullptr;
    if (FAILED(g_device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &back)))
        return false;
    D3DSURFACE_DESC desc;
    back->GetDesc(&desc);

    IDirect3DSurface9* sys = nullptr;
    if (FAILED(g_device->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format,
                                                     D3DPOOL_SYSTEMMEM, &sys, nullptr)))
    {
        back->Release();
        return false;
    }
    bool ok = SUCCEEDED(g_device->GetRenderTargetData(back, sys));
    back->Release();

    D3DLOCKED_RECT lr;
    if (ok) ok = SUCCEEDED(sys->LockRect(&lr, nullptr, D3DLOCK_READONLY));
    if (ok)
    {
        const int w = static_cast<int>(desc.Width);
        const int h = static_cast<int>(desc.Height);
        std::vector<unsigned char> pixels(static_cast<size_t>(w) * h * 4);
        for (int y = 0; y < h; ++y)
        {
            const unsigned char* src =
                static_cast<const unsigned char*>(lr.pBits) + static_cast<size_t>(y) * lr.Pitch;
            unsigned char* dst = pixels.data() + static_cast<size_t>(y) * w * 4;
            std::memcpy(dst, src, static_cast<size_t>(w) * 4);
        }
        sys->UnlockRect();

        // Statistik (BGRX)
        FrameStats st;
        double sumR = 0.0, sumG = 0.0, sumB = 0.0;
        const size_t n = static_cast<size_t>(w) * h;
        for (size_t p = 0; p < n; ++p)
        {
            const double b = pixels[p * 4 + 0] / 255.0;
            const double g = pixels[p * 4 + 1] / 255.0;
            const double r = pixels[p * 4 + 2] / 255.0;
            pixels[p * 4 + 3] = 0;  // Alpha nullen wie AvsRef
            sumR += r;
            sumG += g;
            sumB += b;
            const double luma = 0.2126 * r + 0.7152 * g + 0.0722 * b;
            st.minLuma = (std::min)(st.minLuma, luma);
            st.maxLuma = (std::max)(st.maxLuma, luma);
        }
        st.meanR = sumR / n;
        st.meanG = sumG / n;
        st.meanB = sumB / n;
        if (outStats != nullptr) *outStats = st;

        BITMAPFILEHEADER bfh;
        BITMAPINFOHEADER bih;
        ZeroMemory(&bfh, sizeof(bfh));
        ZeroMemory(&bih, sizeof(bih));
        bfh.bfType = 0x4D42;
        bfh.bfOffBits = sizeof(bfh) + sizeof(bih);
        bfh.bfSize = bfh.bfOffBits + static_cast<DWORD>(pixels.size());
        bih.biSize = sizeof(bih);
        bih.biWidth = w;
        bih.biHeight = -h;  // top-down
        bih.biPlanes = 1;
        bih.biBitCount = 32;
        bih.biCompression = BI_RGB;

        FILE* f = _wfopen(bmpPath.c_str(), L"wb");
        if (f != nullptr)
        {
            std::fwrite(&bfh, sizeof(bfh), 1, f);
            std::fwrite(&bih, sizeof(bih), 1, f);
            std::fwrite(pixels.data(), 1, pixels.size(), f);
            std::fclose(f);
        }
        else
        {
            ok = false;
        }
    }
    sys->Release();
    return ok;
}

LRESULT CALLBACK refWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    return DefWindowProcW(hwnd, msg, wp, lp);
}

void pumpMessages()
{
    MSG msg;
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE) != 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

} // namespace

int main(int argc, char** argv)
{
    namespace fs = std::filesystem;

    std::string target;
    int frames = 240;
    fs::path outDir = "out_milkdropref";
    int width = 640, height = 480;
    bool silence = false;
    bool show = false;
    // --audio-muster (S74): klassisch = Sinus+Beat-Puls wie bisher, musik =
    // aus einer Aufnahme abgeleitete Huellkurven. Beides aus SynthAudio.hpp,
    // die MilkdropStandalone genauso einbindet.
    lumi::synth::Muster muster = lumi::synth::Muster::Klassisch;

    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];
        if (arg == "--frames" && i + 1 < argc) frames = std::atoi(argv[++i]);
        else if (arg == "--out" && i + 1 < argc) outDir = argv[++i];
        else if (arg == "--silence") silence = true;
        else if (arg == "--show") show = true;
        else if (arg == "--audio-muster" && i + 1 < argc)
        {
            if (!lumi::synth::musterAusText(argv[++i], muster))
            {
                std::fprintf(stderr,
                             "FEHLER: --audio-muster erwartet klassisch|musik\n");
                return 2;
            }
        }
        else if (arg == "--size" && i + 1 < argc)
        {
            const std::string wh = argv[++i];
            const size_t x = wh.find('x');
            if (x != std::string::npos)
            {
                width = (std::max)(64, std::atoi(wh.substr(0, x).c_str()));
                height = (std::max)(64, std::atoi(wh.substr(x + 1).c_str()));
            }
        }
        else if (target.empty()) target = arg;
    }
    if (target.empty())
    {
        std::fprintf(stderr,
                     "Aufruf: MilkdropRef <preset.milk|ordner> [--frames N] [--out DIR] "
                     "[--size WxH] [--silence] [--show] "
                     "[--audio-muster klassisch|musik]\n");
        return 2;
    }

    // --- Preset-Liste ----------------------------------------------------------------
    std::vector<fs::path> presets;
    const fs::path targetPath = fs::absolute(target);
    if (fs::is_directory(targetPath))
    {
        for (const auto& e : fs::directory_iterator(targetPath))
        {
            if (e.is_regular_file() && e.path().extension() == L".milk")
                presets.push_back(e.path());
        }
        std::sort(presets.begin(), presets.end());
    }
    else if (fs::is_regular_file(targetPath))
    {
        presets.push_back(targetPath);
    }
    if (presets.empty())
    {
        std::fprintf(stderr, "FEHLER: keine .milk-Presets unter '%s'\n", target.c_str());
        return 2;
    }
    fs::create_directories(outDir);

    std::printf("[MilkdropRef] %d Preset(s), %d Frames, %dx%d%s\n",
                static_cast<int>(presets.size()), frames, width, height,
                silence ? ", STILLE" : "");

    // --- Fenster (unsichtbar, exakte Client-Groesse) ----------------------------------
    const HINSTANCE inst = GetModuleHandleW(nullptr);
    api_orig_hinstance = inst;
    WNDCLASSW wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc = refWndProc;
    wc.hInstance = inst;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    wc.lpszClassName = L"MilkdropRefWindow";
    RegisterClassW(&wc);

    RECT rc;
    SetRect(&rc, 0, 0, width, height);
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    HWND hwnd = CreateWindowW(L"MilkdropRefWindow", L"MilkdropRef", WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left,
                              rc.bottom - rc.top, nullptr, nullptr, inst, nullptr);
    if (hwnd == nullptr)
    {
        std::fprintf(stderr, "FEHLER: CreateWindow\n");
        return 3;
    }
    if (show) ShowWindow(hwnd, SW_SHOWNOACTIVATE);

    // --- Kern hochfahren ---------------------------------------------------------------
    // Shell-Settings (protected, per ini gesteuert): "Press F1"-Overlay aus,
    // sonst stuende Text in den Screenshots. Versions-Gate noetig, sonst
    // verwirft ReadConfig die ini (INT_VERSION/INT_SUBVERSION, defines.h)
    {
        wchar_t exePath[MAX_PATH];
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        std::wstring ini(exePath);
        ini = ini.substr(0, ini.find_last_of(L'\\') + 1) + L"milk2.ini";
        WritePrivateProfileStringW(L"settings", L"version", L"200", ini.c_str());
        WritePrivateProfileStringW(L"settings", L"subversion", L"5", ini.c_str());
        WritePrivateProfileStringW(L"settings", L"show_press_f1_msg", L"0", ini.c_str());
        WritePrivateProfileStringW(L"settings", L"max_fps_w", L"60", ini.c_str());
    }
    g_plugin.PluginPreInitialize(nullptr, nullptr);

    // Batch-Verhalten erzwingen (Werte NACH PreInitialize ueberschreiben — die
    // milk2.ini neben der Exe bleibt damit irrelevant fuer den Messlauf)
    g_plugin.m_bPresetLockedByUser = true;   // kein Auto-Wechsel
    g_plugin.m_fTimeBetweenPresets = 1e9f;
    g_plugin.m_fTimeBetweenPresetsRand = 0.0f;
    g_plugin.m_bHardCutsDisabled = true;
    g_plugin.m_bSequentialPresetOrder = true;

    // Asset-Layout: <basis>/presets/*.milk mit <basis>/{textures,sprites} —
    // der Kern sucht unter m_szMilkdrop2Path\textures und m_szPresetDir
    const fs::path presetDir = presets.front().parent_path();
    const fs::path baseDir = presetDir.parent_path();
    swprintf(g_plugin.m_szMilkdrop2Path, L"%s\\", baseDir.wstring().c_str());
    swprintf(g_plugin.m_szPresetDir, L"%s\\", presetDir.wstring().c_str());

    if (!initD3d(hwnd, width, height)) return 3;
    if (g_plugin.PluginInitialize(g_device, &g_pp, hwnd, width, height) == 0)
    {
        std::fprintf(stderr, "FEHLER: PluginInitialize\n");
        return 3;
    }

    // --- Batch -------------------------------------------------------------------------
    unsigned char pcmL[kSamples];
    unsigned char pcmR[kSamples];
    int rc2 = 0;
    for (const fs::path& preset : presets)
    {
        std::printf("\n[MilkdropRef] === %s ===\n", preset.filename().string().c_str());
        std::fflush(stdout);
        g_plugin.m_bInitialPresetSelected = true;  // kein Zufalls-Startpreset
        g_plugin.LoadPreset(preset.wstring().c_str(), 0.0f);

        for (int f = 0; f < frames; ++f)
        {
            lumi::synth::Optionen synthOpt;
            synthOpt.muster = muster;
            synthOpt.geschmack = lumi::synth::Geschmack::Milkdrop;
            synthOpt.stille = silence;
            buildPcm(f, synthOpt, pcmL, pcmR);
            if (g_plugin.PluginRender(pcmL, pcmR) == 0)
            {
                std::fprintf(stderr, "[MilkdropRef] RENDER-ABBRUCH bei Frame %d\n", f);
                rc2 = 4;
                break;
            }
            pumpMessages();
        }

        FrameStats st;
        const std::wstring bmp =
            (fs::absolute(outDir) / (preset.stem().wstring() + L"_ref.bmp")).wstring();
        if (dumpBackbuffer(bmp, &st))
        {
            std::printf("[MilkdropRef] Screenshot: %ls — mean RGB=(%.3f, %.3f, %.3f), "
                        "Luma min=%.3f max=%.3f\n",
                        bmp.c_str(), st.meanR, st.meanG, st.meanB, st.minLuma, st.maxLuma);
        }
        else
        {
            std::fprintf(stderr, "[MilkdropRef] READBACK FEHLGESCHLAGEN\n");
            rc2 = 4;
        }
        std::fflush(stdout);
    }

    g_plugin.PluginQuit();
    if (g_device != nullptr) g_device->Release();
    if (g_d3d9 != nullptr) g_d3d9->Release();
    DestroyWindow(hwnd);
    return rc2;
}
