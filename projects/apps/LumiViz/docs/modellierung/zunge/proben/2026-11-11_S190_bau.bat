@echo off
rem S190, 34j-5: den Pruefstand der Zunge gegen den Kern freeReed bauen (InstrumentModels/zunge, InstrumentCore/Kurve).
rem
rem   2026-11-11_S190_bau.bat <quelle.cpp> <ziel.exe>
rem
rem Umgebung von Visual Studio 18, clang, Schalter wie die Kern-Bibliotheken (keine Kontraktion, kein fast-math).
setlocal
if "%~2"=="" (echo Aufruf: %~nx0 ^<quelle.cpp^> ^<ziel.exe^> & exit /b 2)
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul || exit /b 2
set "R=%~dp0..\..\..\..\..\..\.."
set "L=%R%\projects\libs"
clang++ -std=c++20 -O2 -ffp-contract=off -fno-fast-math -I "%L%\InstrumentModels\include" -I "%L%\InstrumentCore\include" ^
  "%~1" "%L%\InstrumentModels\src\zunge\Zunge.cpp" "%L%\InstrumentModels\src\zunge\Vorgaben.cpp" ^
  "%L%\InstrumentCore\src\Kurve.cpp" -o "%~2"
exit /b %ERRORLEVEL%
