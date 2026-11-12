@echo off
rem S195 (34k-4, Ziele): das Messprogramm fuer die Aufnahmen der Klasse 2 bauen — gegen AudioCore aus dem Repo
rem (Lautheit, WAV), ohne die App. Muster: 2026-11-12_S192_bau.bat.
rem
rem   2026-11-12_S195_bau.bat <quelle.cpp> <ziel.exe>
rem
rem Umgebung von Visual Studio 18, clang.
setlocal
if "%~2"=="" (echo Aufruf: %~nx0 ^<quelle.cpp^> ^<ziel.exe^> & exit /b 2)
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul || exit /b 2
set "R=%~dp0..\..\..\..\..\..\.."
set "A=%R%\projects\libs\AudioCore"
clang++ -std=c++20 -O2 -ffp-contract=off -fno-fast-math -I "%A%\include" ^
  "%~1" "%A%\src\Lautheit.cpp" "%A%\src\WavFormat.cpp" -o "%~2"
exit /b %ERRORLEVEL%
