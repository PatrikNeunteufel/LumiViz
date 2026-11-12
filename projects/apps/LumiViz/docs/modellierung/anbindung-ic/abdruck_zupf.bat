@echo off
rem Beleg 34k-1 (S194): abdruck_zupf.cpp gegen die IC-Vorlage der gezupften Saite bauen und laufen lassen.
rem
rem   abdruck_zupf.bat Ausgabeordner [IC-rechnung]
rem
rem Ohne zweiten Ordner gilt die Vorlage im Repo: harvest\instruments-calculation\rechnung (IC 4409088).
rem Wird klassen\02_zupf_saite dort geloescht (34k-11), die Vorlage aus dem Git-Verlauf holen, etwa
rem   git archive <commit> harvest/instruments-calculation/rechnung | tar -x -C <ordner>
rem und <ordner>\harvest\instruments-calculation\rechnung als zweiten Ordner angeben.
rem Baut mit MSVC (cl /O2 /fp:precise /MD) in der Umgebung von Visual Studio 18 - die Plattform von IC
rem werte/abdruck_msvc.txt - und schreibt zupf_ic.txt in den Ausgabeordner. Die Vorlage bleibt unberuehrt.
rem Nur ASCII in dieser Datei: cmd liest sie in der Codepage der Konsole.
setlocal
if "%~1"=="" (echo Aufruf: abdruck_zupf.bat Ausgabeordner [IC-rechnung] & exit /b 2)
set "AUS=%~f1"
if "%~2"=="" (set "IC=%~dp0..\..\..\..\..\..\harvest\instruments-calculation\rechnung") else (set "IC=%~2")
for %%I in ("%IC%") do set "IC=%%~fI"
set "PF=%IC%\..\..\..\externals\pocketfft"
for %%I in ("%PF%") do set "PF=%%~fI"
set "VCVARS=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
call "%VCVARS%" >nul || exit /b 2
if not exist "%AUS%" mkdir "%AUS%"
pushd "%AUS%"
cl /nologo /std:c++20 /O2 /DNDEBUG /MD /EHsc /fp:precise /utf-8 /I"%IC%" /I"%PF%" "%~dp0abdruck_zupf.cpp" "%IC%\klassen\02_zupf_saite\Zupf.cpp" "%IC%\klassen\02_zupf_saite\Vorgaben.cpp" "%IC%\klassen\02_zupf_saite\Pedale.cpp" "%IC%\gemeinsam\Kurve.cpp" "%IC%\gemeinsam\Werkzeug.cpp" "%IC%\gemeinsam\Fft.cpp" "%IC%\gemeinsam\Eigenzerlegung.cpp" "%IC%\gemeinsam\Balken.cpp" "%IC%\messung\Bausteine.cpp" /Fe:abdruck_zupf.exe > bau.log
set "RC=%ERRORLEVEL%"
popd
if not "%RC%"=="0" (type "%AUS%\bau.log" & exit /b 1)
"%AUS%\abdruck_zupf.exe" > "%AUS%\zupf_ic.txt" || exit /b 1
echo geschrieben: %AUS%\zupf_ic.txt
exit /b 0
