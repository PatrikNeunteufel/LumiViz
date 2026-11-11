@echo off
rem Beleg 34j-1 (S189): abdruck_zunge.cpp gegen die IC-Vorlage der Zunge bauen und laufen lassen.
rem
rem   abdruck_zunge.bat Ausgabeordner [IC-rechnung]
rem
rem Ohne zweiten Ordner gilt die Vorlage im Repo: harvest\instruments-calculation\rechnung (IC 4409088).
rem Baut mit MSVC (cl /O2 /fp:precise /MD) in der Umgebung von Visual Studio 18 - die Plattform von IC
rem werte/abdruck_msvc.txt - und schreibt zunge_ic.txt in den Ausgabeordner. Die Vorlage bleibt unberuehrt.
rem Mit clang++ -O2 -ffp-contract=off (Schalter von IC) und /MD-Laufzeit kommen dieselben Bits heraus (S189 geprueft).
rem Nur ASCII in dieser Datei: cmd liest sie in der Codepage der Konsole.
setlocal
if "%~1"=="" (echo Aufruf: abdruck_zunge.bat Ausgabeordner [IC-rechnung] & exit /b 2)
set "AUS=%~f1"
if "%~2"=="" (set "IC=%~dp0..\..\..\..\..\..\harvest\instruments-calculation\rechnung") else (set "IC=%~2")
for %%I in ("%IC%") do set "IC=%%~fI"
set "PF=%IC%\..\..\..\externals\pocketfft"
for %%I in ("%PF%") do set "PF=%%~fI"
set "VCVARS=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
call "%VCVARS%" >nul || exit /b 2
if not exist "%AUS%" mkdir "%AUS%"
pushd "%AUS%"
cl /nologo /std:c++20 /O2 /DNDEBUG /MD /EHsc /fp:precise /utf-8 /I"%IC%" /I"%PF%" "%~dp0abdruck_zunge.cpp" "%IC%\klassen\10_zunge\Zunge.cpp" "%IC%\klassen\10_zunge\Vorgaben.cpp" "%IC%\gemeinsam\Kurve.cpp" "%IC%\gemeinsam\Werkzeug.cpp" "%IC%\gemeinsam\Fft.cpp" "%IC%\gemeinsam\Eigenzerlegung.cpp" "%IC%\gemeinsam\Balken.cpp" "%IC%\messung\Bausteine.cpp" /Fe:abdruck_zunge.exe > bau.log
set "RC=%ERRORLEVEL%"
popd
if not "%RC%"=="0" (type "%AUS%\bau.log" & exit /b 1)
"%AUS%\abdruck_zunge.exe" > "%AUS%\zunge_ic.txt" || exit /b 1
echo geschrieben: %AUS%\zunge_ic.txt
exit /b 0
