@echo off
rem Beleg 34h-2 (S179): abdruck_trommel.cpp gegen die IC-Quellen bauen und laufen lassen.
rem
rem   abdruck_trommel.bat IC-rechnung Ausgabeordner
rem
rem Etwa: abdruck_trommel.bat ..\..\..\..\..\..\..\InstrumentsCalculation\rechnung %TEMP%\abdruck
rem Baut mit MSVC (cl /O2 /fp:precise /MD) in der Umgebung von Visual Studio 18 - die Plattform von IC
rem werte/abdruck_msvc.txt - und schreibt trommel_ic.txt in den Ausgabeordner. Das IC-Repo bleibt unberuehrt.
rem
rem Nicht mit clang++ und statischer Laufzeit (Vorgabe von clang++ ohne CMake): die Bongo weicht dort in einem Wert um
rem ein float-ULP ab (S179: Wert 7525 des Tons ff; die statische ucrt rechnet eine Funktion anders als die dynamische,
rem mit der MSVC und der LumiViz-Bau unter clang linken - beide geben dieselben Bits).
rem Nur ASCII in dieser Datei: cmd liest sie in der Codepage der Konsole.
setlocal
if "%~2"=="" (echo Aufruf: abdruck_trommel.bat IC-rechnung Ausgabeordner & exit /b 2)
set "IC=%~f1"
set "AUS=%~f2"
set "VCVARS=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
call "%VCVARS%" >nul || exit /b 2
if not exist "%AUS%" mkdir "%AUS%"
pushd "%AUS%"
cl /nologo /std:c++20 /O2 /DNDEBUG /MD /EHsc /fp:precise /utf-8 /I"%IC%" "%~dp0abdruck_trommel.cpp" "%IC%\klassen\05_membran\Trommel.cpp" "%IC%\klassen\05_membran\Vorgaben.cpp" "%IC%\gemeinsam\Membran.cpp" "%IC%\gemeinsam\Balken.cpp" "%IC%\gemeinsam\Eigenzerlegung.cpp" "%IC%\gemeinsam\Modal.cpp" "%IC%\gemeinsam\Werkzeug.cpp" /Fe:abdruck_trommel.exe >nul
set "RC=%ERRORLEVEL%"
popd
if not "%RC%"=="0" exit /b 1
"%AUS%\abdruck_trommel.exe" > "%AUS%\trommel_ic.txt" || exit /b 1
echo geschrieben: %AUS%\trommel_ic.txt
exit /b 0
