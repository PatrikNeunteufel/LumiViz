@echo off
rem S188: ein Messprogramm gegen die IC-Vorlage der Zunge bauen (harvest/instruments-calculation, wird sonst nicht gebaut).
rem Seit S192 (34j-10) ist klassen/10_zunge dort geloescht - aus dem Git-Verlauf holen (Commit 6e9f4756, siehe
rem docs/modellierung/anbindung-ic/abdruck_zunge.bat) und R unten auf jenen Ordner setzen.
rem
rem   2026-11-11_S188_bau.bat <quelle.cpp> <ziel.exe>
rem
rem Umgebung von Visual Studio 18, clang, Schalter wie IC (E1 a: keine Kontraktion, kein fast-math).
setlocal
if "%~2"=="" (echo Aufruf: %~nx0 ^<quelle.cpp^> ^<ziel.exe^> & exit /b 2)
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul || exit /b 2
set "R=%~dp0..\..\..\..\..\..\.."
set "IC=%R%\harvest\instruments-calculation\rechnung"
clang++ -std=c++20 -O2 -ffp-contract=off -fno-fast-math -I "%IC%" -isystem "%R%\externals\pocketfft" ^
  "%~1" "%IC%\klassen\10_zunge\Zunge.cpp" "%IC%\klassen\10_zunge\Vorgaben.cpp" ^
  "%IC%\gemeinsam\Kurve.cpp" "%IC%\gemeinsam\Werkzeug.cpp" "%IC%\gemeinsam\Fft.cpp" "%IC%\gemeinsam\Eigenzerlegung.cpp" ^
  "%IC%\gemeinsam\Balken.cpp" "%IC%\messung\Bausteine.cpp" -o "%~2"
exit /b %ERRORLEVEL%
