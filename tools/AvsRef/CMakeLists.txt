# AvsRef — Referenz-Renderer um den originalen vis_avs-Kern (Session 46).
# Eigenstaendiges 32-bit-MSVC-Projekt (EEL-JIT + MMX-Inline-Asm sind x86),
# bewusst NICHT Teil der CMakeCraft-Solution (die baut x64).
#
# Konfigurieren:  cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
# Bauen:          cmake --build build --config Release
cmake_minimum_required(VERSION 3.21)
project(AvsRef C CXX)

if(NOT MSVC)
    message(FATAL_ERROR "AvsRef braucht MSVC (SEH, __asm, altes Win32-C++).")
endif()
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "AvsRef muss 32-bit gebaut werden: -A Win32 (EEL-JIT/MMX).")
endif()

# Referenz-Quellbaum: ../ref neben dem LumiViz-Repo (Repo-Landkarte)
set(AVS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../ref/vis_avs/avs")
if(NOT EXISTS "${AVS_ROOT}/vis_avs/r_list.cpp")
    message(FATAL_ERROR "ref/vis_avs nicht gefunden unter ${AVS_ROOT}")
endif()

set(VIS "${AVS_ROOT}/vis_avs")
set(NSEEL "${AVS_ROOT}/ns-eel")

# Render-Kern: Effekte + Liste + Registry + EEL — OHNE main/wnd/draw/cfgwin/
# render/undo/r_transition (Winamp, DDraw, UI, Transitions)
set(AVS_SOURCES
    ${VIS}/r_avi.cpp
    ${VIS}/r_blit.cpp
    ${VIS}/r_blur.cpp
    ${VIS}/r_bpm.cpp
    ${VIS}/r_bright.cpp
    ${VIS}/r_bspin.cpp
    ${VIS}/r_bump.cpp
    ${VIS}/r_chanshift.cpp
    ${VIS}/r_clear.cpp
    ${VIS}/r_colorfade.cpp
    ${VIS}/r_colorreduction.cpp
    ${VIS}/r_comment.cpp
    ${VIS}/r_contrast.cpp
    ${VIS}/r_dcolormod.cpp
    ${VIS}/r_ddm.cpp
    ${VIS}/r_dmove.cpp
    ${VIS}/r_dotfnt.cpp
    ${VIS}/r_dotgrid.cpp
    ${VIS}/r_dotpln.cpp
    ${VIS}/r_fadeout.cpp
    ${VIS}/r_fastbright.cpp
    ${VIS}/r_grain.cpp
    ${VIS}/r_interf.cpp
    ${VIS}/r_interleave.cpp
    ${VIS}/r_invert.cpp
    ${VIS}/r_linemode.cpp
    # r_list: UNVERAENDERTE Kopie in patched/ — nur damit der Quote-Include
    # dort das gepatchte rlib.h (via render.h-Kopie) findet
    patched/r_list.cpp
    ${VIS}/r_mirror.cpp
    ${VIS}/r_mosaic.cpp
    ${VIS}/r_multidelay.cpp
    ${VIS}/r_multiplier.cpp
    ${VIS}/r_nfclr.cpp
    ${VIS}/r_onetone.cpp
    ${VIS}/r_oscring.cpp
    ${VIS}/r_oscstar.cpp
    ${VIS}/r_parts.cpp
    ${VIS}/r_picture.cpp
    ${VIS}/r_rotblit.cpp
    ${VIS}/r_rotstar.cpp
    ${VIS}/r_scat.cpp
    ${VIS}/r_shift.cpp
    ${VIS}/r_simple.cpp
    ${VIS}/r_sscope.cpp
    ${VIS}/r_stack.cpp
    ${VIS}/r_stars.cpp
    ${VIS}/r_svp.cpp
    ${VIS}/r_text.cpp
    ${VIS}/r_timescope.cpp
    ${VIS}/r_trans.cpp
    ${VIS}/r_unkn.cpp
    ${VIS}/r_videodelay.cpp
    ${VIS}/r_water.cpp
    ${VIS}/r_waterbump.cpp
    # rlib/bpm: minimal gepatchte Kopien (modernes MSVC, s. patched/*)
    patched/rlib.cpp
    ${VIS}/linedraw.cpp
    ${VIS}/matrix.cpp
    ${VIS}/util.cpp
    patched/bpm.cpp
    ${VIS}/TIMING.C
    ${VIS}/avs_eelif.cpp
    ${NSEEL}/megabuf.c
    ${NSEEL}/nseel-caltab.c
    patched/nseel-cfunc.c
    patched/nseel-compiler.c
    ${NSEEL}/nseel-eval.c
    ${NSEEL}/nseel-lextab.c
    ${NSEEL}/nseel-yylex.c
)

add_executable(AvsRef
    src/avsref_main.cpp
    src/avsref_stubs.cpp
    ${AVS_SOURCES}
)

# patched/ VOR dem Referenzbaum: gepatchtes rlib.h gewinnt fuer alle TUs
target_include_directories(AvsRef PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/patched ${VIS} ${NSEEL}
)
# Defines wie im Original-.dsp (ohne WA2_EMBED — kein Winamp)
target_compile_definitions(AvsRef PRIVATE
    WIN32
    _MBCS
    NSEEL_LOOPFUNC_SUPPORT
    AVS_MEGABUF_SUPPORT
    _CRT_SECURE_NO_WARNINGS
    _CRT_NONSTDC_NO_DEPRECATE
)
# Alt-Code: Warnungen stummschalten, damit echte Fehler sichtbar bleiben
target_compile_options(AvsRef PRIVATE /W1 /wd4996)

target_link_libraries(AvsRef PRIVATE
    user32 gdi32 comctl32 comdlg32 shell32 ole32 vfw32 winmm
)

# EEL-JIT-Vertraege (ns-eel misst Fragmentgroessen als Funktionsadress-
# Differenzen und kopiert/patcht x86-Code zur Laufzeit):
#  - /OPT:NOICF: ICF faltet die identischen leeren nseel_asm_*_end-Funktionen
#    zusammen -> Fragmentgroessen werden Muell -> Heap-Korruption
#  - /INCREMENTAL:NO: ILT-Thunks zerstoeren die Adress-Arithmetik
#  - /NXCOMPAT:NO: der JIT-Code liegt in GlobalAlloc-Speicher (nicht
#    ausfuehrbar unter DEP) — lokales Werkzeug, bewusst deaktiviert
#  - /SAFESEH:NO: die alten Asm-Objekte liefern keine SEH-Tabellen
target_link_options(AvsRef PRIVATE
    /INCREMENTAL:NO /OPT:NOICF /NXCOMPAT:NO /SAFESEH:NO
)
