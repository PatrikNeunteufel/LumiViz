/**
 ****************************************************************************************
 * @file   ref_msgbox.h
 * @brief  MilkdropRef: MessageBox → stderr (S67) — die Original-Fehlerdialoge
 *         ("MILKDROP ERROR", z. B. "Unable to read the data file") blockierten
 *         Batch-Läufe bei falscher Preset-Basis. Wird per /FI in ALLE
 *         Übersetzungseinheiten gezwungen; die Boxen werden zu stderr-Zeilen,
 *         Rückgabewert IDOK, das Verhalten danach bleibt das der Referenz.
 *
 * @author LumiPulse Team
 * @date   August 2026
 * @version 1.0.0
 ****************************************************************************************
 */
#pragma once
#include <windows.h>
#include <cstdio>

static inline int lumiRefMsgBoxW(HWND, const wchar_t* text, const wchar_t* title,
                                 UINT)
{
    fwprintf(stderr, L"[MilkdropRef] %ls: %ls\n", title ? title : L"",
             text ? text : L"");
    fflush(stderr);
    return IDOK;
}
static inline int lumiRefMsgBoxA(HWND, const char* text, const char* title, UINT)
{
    fprintf(stderr, "[MilkdropRef] %s: %s\n", title ? title : "",
            text ? text : "");
    fflush(stderr);
    return IDOK;
}

// OBJEKT-artige Makros: die Referenz hat #ifdef-Direktiven INNERHALB von
// MessageBox-Argumentlisten (utility.cpp:307) — in funktionsartigen Makro-
// Argumenten waere das illegal (C2121), als einfacher Namens-Ersatz nicht.
#undef MessageBox
#undef MessageBoxW
#undef MessageBoxA
#define MessageBoxW lumiRefMsgBoxW
#define MessageBoxA lumiRefMsgBoxA
#define MessageBox lumiRefMsgBoxA
