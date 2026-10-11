@echo off
rem Release-clang bauen — in der Umgebung von Visual Studio 18 (2026).
rem
rem   tools\bau_clang.bat [Ziel]
rem
rem Ohne Ziel alles, sonst etwa: tools\bau_clang.bat LumiViz.UnitTests
rem
rem Warum so (S105, CLAUDE.md): ohne diese Umgebung scheitert LumiViz.rc ohne
rem Meldung, mit der von Visual Studio 2022 das Linken. Vier Auftraege zugleich:
rem mit mehr bricht clang beim Bau aller Tests mit "out of memory" ab.
rem Ein offener Tester oder ein laufendes LumiViz sperrt Dateien neben der Exe —
rem dann wird nicht gebaut (Rueckgabe 3).
setlocal
tasklist /FI "IMAGENAME eq Sichttest.exe" | find /I "Sichttest.exe" >nul && (echo LAEUFT: Sichttest - nicht gebaut & exit /b 3)
tasklist /FI "IMAGENAME eq LumiViz.exe" | find /I "LumiViz.exe" >nul && (echo LAEUFT: LumiViz - nicht gebaut & exit /b 3)
set "VCVARS=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
if not exist "%VCVARS%" (echo Nicht gefunden: %VCVARS% & exit /b 2)
call "%VCVARS%" >nul || exit /b 2
cd /d "%~dp0.." || exit /b 2
if "%~1"=="" (
    cmake --build --preset build-ninja-release-clang --parallel 4
) else (
    cmake --build --preset build-ninja-release-clang --parallel 4 --target %1
)
exit /b %ERRORLEVEL%
