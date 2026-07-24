/**
 ****************************************************************************************
 * @file   avsref_stubs.cpp
 * @brief  Stubs fuer die vis_avs-Globals aus main/wnd/draw/cfgwin/render, die
 *         AvsRef nicht mitkompiliert (Session 46). Werte = Original-Defaults.
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#include <windows.h>

// r_defs.h liefert die extern-Deklarationen — noetig, damit die const-MMX-
// Konstanten hier EXTERNE Bindung bekommen (C++-const ist sonst TU-intern)
#include "r_defs.h"

// --- render.cpp: Blendtabelle + MMX-Konstanten (Werte identisch uebernommen) ---
unsigned char g_blendtable[256][256];
unsigned int const mmx_blend4_revn[2] = {0xff00ff, 0xff00ff};
int const mmx_blendadj_mask[2] = {0xff00ff, 0xff00ff};
int const mmx_blend4_zero = 0;

// --- main.cpp ---
char g_path[1024];
HINSTANCE g_hInstance = NULL;
CRITICAL_SECTION g_render_cs;

// --- r_list.cpp: SMP aus (deterministisch, single-threaded) ---
int g_config_smp_mt = 0;
int g_config_smp = 0;
int config_reuseonresize = 1;

// --- bpm.cpp: Titel-Puffer + Fake-Init (wnd.cpp/main.cpp) ---
CRITICAL_SECTION g_title_cs;
char g_title[2048];
int g_fakeinit = 0;
namespace
{
struct CritSecInit
{
    CritSecInit()
    {
        InitializeCriticalSection(&g_title_cs);
        InitializeCriticalSection(&g_render_cs);
    }
} s_critSecInit;
} // namespace

// --- r_text.cpp / avs_eelif.cpp: kein Winamp-Fenster vorhanden ---
HWND hwnd_WinampParent = NULL;

// --- avs_eelif.cpp: Mauskoordinaten-Umrechnung (draw.cpp) — kein Fenster ---
double DDraw_translatePoint(POINT p, int isY)
{
    return 0.0;
}

// --- cfgwin.cpp ---
int g_reset_vars_on_recompile = 0;

// --- undo.cpp: r_list.cpp referenziert nur C_UndoItem::set (undo.cpp selbst
// zoege Transition + zweite Renderliste nach) — Implementierung woertlich
// aus undo.cpp:83-93 uebernommen ---
#include "undo.h"
void C_UndoItem::set(void* _data, int _length, bool _isdirty)
{
    length = _length;
    isdirty = _isdirty;
    if (data)
    {
        GlobalFree(data);
    }
    data = GlobalAlloc(GPTR, length);
    memcpy(data, _data, length);
}
