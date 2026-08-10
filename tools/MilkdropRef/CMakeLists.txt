# MilkdropRef — Referenz-Renderer um den originalen MilkDrop3-Kern (Session 63).
# Eigenstaendiges 32-bit-MSVC-Projekt nach AvsRef-Vorbild (das Original-Projekt
# plugin.vcxproj baut Win32; ns-eel2 nutzt x86-JIT), bewusst NICHT Teil der
# CMakeCraft-Solution.
#
# Konfigurieren:  cmake -S tools/MilkdropRef -B tools/MilkdropRef/build -G "Visual Studio 17 2022" -A Win32
# Bauen:          cmake --build tools/MilkdropRef/build --config Release
#
# d3dx9 kommt aus dem NuGet-Paket Microsoft.DXSDK.D3DX (third_party/dxsdk-d3dx,
# lokal entpackt, untracked) — keine System-Installation noetig.
cmake_minimum_required(VERSION 3.21)
project(MilkdropRef CXX C)

if(NOT MSVC)
    message(FATAL_ERROR "MilkdropRef braucht MSVC (Win32-C++, D3D9).")
endif()
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "MilkdropRef muss 32-bit gebaut werden: -A Win32 (ns-eel2-JIT).")
endif()

# Referenz-Quellbaum: ../ref neben dem LumiViz-Repo (Repo-Landkarte)
set(MD3_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../ref/MilkDrop3/code")
if(NOT EXISTS "${MD3_ROOT}/vis_milk2/plugin.cpp")
    message(FATAL_ERROR "ref/MilkDrop3 nicht gefunden unter ${MD3_ROOT}")
endif()

set(VIS "${MD3_ROOT}/vis_milk2")
set(NSEEL "${MD3_ROOT}/ns-eel2")
set(DXSDK "${CMAKE_CURRENT_SOURCE_DIR}/third_party/dxsdk-d3dx/build/native")
if(NOT EXISTS "${DXSDK}/include/d3dx9.h")
    message(FATAL_ERROR "d3dx9 fehlt: NuGet Microsoft.DXSDK.D3DX nach third_party/dxsdk-d3dx entpacken (SETUP in README.md)")
endif()

# Render-Kern wie plugin.vcxproj — OHNE Milkdrop2PcmVisualizer (ersetzt durch
# src/milkdropref_main.cpp) und OHNE audio/* (WASAPI-Loopback; wir fuettern
# synthetisches PCM byte-identisch zur LumiViz-Seite)
set(MD3_SOURCES
    ${VIS}/dxcontext.cpp
    ${VIS}/fft.cpp
    ${VIS}/menu.cpp
    ${VIS}/milkdropfs.cpp
    ${VIS}/plugin.cpp
    ${VIS}/pluginshell.cpp
    ${VIS}/state.cpp
    ${VIS}/support.cpp
    ${VIS}/texmgr.cpp
    ${VIS}/textmgr.cpp
    ${VIS}/utility.cpp
    ${VIS}/wasabi.cpp
    ${NSEEL}/nseel-caltab.c
    ${NSEEL}/nseel-cfunc.c
    # nseel-compiler: gepatchte Kopie — __floor-Intrinsic-Kollision (s. patched/)
    patched/nseel-compiler.c
    ${NSEEL}/nseel-eval.c
    ${NSEEL}/nseel-lextab.c
    ${NSEEL}/nseel-ram.c
    ${NSEEL}/nseel-yylex.c
)

add_executable(MilkdropRef
    src/milkdropref_main.cpp
    ${MD3_SOURCES}
    ${VIS}/plugin.rc
)

# patched/ VOR dem Referenzbaum, falls Kopien noetig werden (AvsRef-Muster).
# projects/exec/common: der GEMEINSAME synthetische Klang (SynthAudio.hpp, S74)
# — dieselbe Datei, die MilkdropStandalone einbindet. Genau darauf beruht die
# Vergleichbarkeit; vorher war die Formel hier eine Kopie.
# Dazu die GEMEINSAME Kaltstart-Saat (KaltstartSaat.hpp, S75) aus dem
# App-Include-Baum: nur wenn beide Seiten dieselbe Saat einstreuen, beginnen
# sie im selben Startzustand — sonst zeichnen wir und die Referenz bleibt
# schwarz (belegt an Starfield/Helix/Beauty).
target_include_directories(MilkdropRef PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/patched ${VIS} ${NSEEL} ${DXSDK}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../projects/exec/common
    ${CMAKE_CURRENT_SOURCE_DIR}/../../projects/apps/LumiViz/include
)
# Defines wie plugin.vcxproj Release|Win32
target_compile_definitions(MilkdropRef PRIVATE
    WIN32
    _WINDOWS
    USE_VIS_HDR_HWND
    STRSAFE_NO_DEPRECATE
    NSEEL_REENTRANT_EXECUTION
    _CRT_SECURE_NO_WARNINGS
    _CRT_NON_CONFORMING_SWPRINTFS
    _CRT_NONSTDC_NO_DEPRECATE
)
# Alt-Code: Warnungen stummschalten, damit echte Fehler sichtbar bleiben;
# /Zc:forScope-: VC6-Schleifen-Scope (i lebt nach der for-Schleife weiter);
# /FI ref_msgbox.h: MessageBox → stderr (S67) — Fehlerdialoge blockierten
# Batch-Läufe (z. B. "Unable to read the data file" bei falscher Preset-Basis)
target_compile_options(MilkdropRef PRIVATE /W1 /wd4996 /Zc:forScope-
    "/FI${CMAKE_CURRENT_SOURCE_DIR}/patched/ref_msgbox.h")
# Nur der eigene Host braucht C++17 (std::filesystem) — der Referenzbaum
# bleibt auf dem MSVC-Default, wie ihn plugin.vcxproj baute
set_source_files_properties(src/milkdropref_main.cpp PROPERTIES
    COMPILE_OPTIONS "/std:c++17"
)

target_link_libraries(MilkdropRef PRIVATE
    d3d9
    "${DXSDK}/release/lib/x86/d3dx9.lib"
    Shlwapi user32 gdi32 comctl32 comdlg32 shell32 ole32 winmm dwmapi
)

# Konsolen-App (CLI-Batch), aber WinMain-frei — Einstieg ist main()
set_target_properties(MilkdropRef PROPERTIES WIN32_EXECUTABLE FALSE)

# ns-eel2-JIT-Vertraege (Fragmentgroessen als Funktionsadress-Differenzen,
# Code wird zur Laufzeit kopiert/gepatcht) — Lehren aus AvsRef:
target_link_options(MilkdropRef PRIVATE
    /INCREMENTAL:NO /OPT:NOICF /NXCOMPAT:NO /SAFESEH:NO
)

# d3dx9_43.dll neben die Exe (NuGet-Runtime, x86)
add_custom_command(TARGET MilkdropRef POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DXSDK}/release/bin/x86/D3DX9_43.dll"
        "$<TARGET_FILE_DIR:MilkdropRef>/D3DX9_43.dll"
)
