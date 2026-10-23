# Fremdkomponenten und ihre Lizenzen

LumiViz selbst steht unter MIT **oder** Apache-2.0 (siehe [LICENSE](LICENSE)).
Die hier aufgeführten Komponenten stehen unter **eigenen** Lizenzen. Wer LumiViz
weitergibt — als Quelltext oder als fertiges Programm — muss die zugehörigen
Copyright-Vermerke und Haftungsausschlüsse mitliefern.

Die Übersicht ist in drei Teile geteilt:

1. **Mitgelieferter Fremdcode** — liegt in diesem Repository
2. **Portierter Fremdcode** — LumiViz-Dateien, deren Algorithmen aus fremden
   Quellen stammen
3. **Extern bezogene Abhängigkeiten** — werden beim Bauen geholt oder müssen
   selbst beschafft werden

---

## 1. Mitgelieferter Fremdcode

### AVS-Referenzrenderer — `tools/AvsRef/patched/`

Quelldateien aus **Winamp AVS (vis_avs)**, gepatcht für den Offscreen-Betrieb
als Vergleichsmaßstab der Kalibrierung.

> Copyright 2005 Nullsoft, Inc. — **BSD-3-Clause**

```
Copyright 2005 Nullsoft, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  * Neither the name of Nullsoft nor the names of its contributors may be used to
    endorse or promote products derived from this software without specific prior
    written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
DAMAGE.
```

Betroffen: `avs_eelif.cpp`, `bpm.cpp`, `nseel-cfunc.c`, `nseel-compiler.c`,
`r_avi.cpp`, `r_list.cpp`, `render.h`, `rlib.cpp`, `rlib.h`.
Die Lizenzköpfe sind in den Dateien unverändert erhalten.

### NS-EEL2 — `tools/MilkdropRef/patched/nseel-compiler.c`

Ausdrucksauswerter aus der WDL-Sammlung.

> Copyright (C) 2004-2008 Cockos Incorporated · Copyright (C) 1999-2003 Nullsoft, Inc.
> — **zlib-artige Lizenz**

```
This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
```

### doctest — `externals/doctest/doctest.h`

Test-Rahmenwerk, nur für die Test-Targets.

> Copyright (c) 2016-2023 Viktor Kirilov — **MIT**
> <https://github.com/doctest/doctest>

### glad — `externals/glad/`

OpenGL-Lader, generiert mit glad 0.1.36 (gl 3.3 compatibility).
Der Generator steht unter MIT; der erzeugte Ladecode ist gemeinfrei.

> <https://github.com/Dav1dde/glad>

`include/KHR/khrplatform.h`:

> Copyright (c) 2008-2018 The Khronos Group Inc. — **MIT-artig (Khronos)**

### miniz 3.1.2 — `externals/miniz/`

Zip lesen und schreiben, für den Export und Import der eigenen Daten
(`.lvdata`). `miniz.c` und `miniz.h` liegen unverändert im Repository und
werden **in die Exe gelinkt** — der Vermerk unten gehört deshalb zu jedem
weitergegebenen Binärpaket; diese Datei liegt nach jedem Bau neben der Exe.
Herkunft, Fassung und Prüfsumme: `externals/miniz/README.md`; der Lizenztext
im Original: `externals/miniz/LICENSE`.

> <https://github.com/richgel999/miniz>

```
Copyright 2013-2014 RAD Game Tools and Valve Software
Copyright 2010-2014 Rich Geldreich and Tenacious Software LLC

All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
```

Der Wortlaut ist der der MIT-Lizenz; der Name steht in der Lizenzdatei nicht.
Im Kopf von `miniz.h` steht zusätzlich „public domain … See ‚unlicense'
statement at the end of this file".

### CDT 2.0.1 — `externals/CDT/`

Constrained Delaunay Triangulation mit Ruppert-Verfeinerung, für das Netz des
Instrumentkörpers (Bibliothek `InstrumentBody`, Stufe 31c-2). Die Header liegen
unverändert im Repository und werden **nicht in die App** gebunden, nur in die
Unit-Tests und das Werkzeug `InstrumentBodyTool`. Herkunft, Fassung und
Prüfsumme: `externals/CDT/README.md`; die Lizenztexte im Original:
`externals/CDT/LICENSE`, `LICENSE-predicates.txt`, `LICENSE-LLVM.txt`.

> <https://github.com/artem-ogre/CDT> — **MPL-2.0**

Darin mitgeliefert:

- `include/portable_nth_element.hpp` — aus dem LLVM-Projekt, **Apache-2.0 mit
  LLVM-Ausnahme**.
- `include/predicates.h` — robuste geometrische Prädikate, **BSD-3-Clause**; der
  Vermerk gehört zu jeder weitergegebenen Binärdatei, die sie enthält:

```
Copyright (c) 2019, William C. Lenthe
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```

### Eigen 3.4.1 — `externals/Eigen/`

Lineare Algebra für Platte und Eigenlöser des Instrumentkörpers (Bibliothek `InstrumentBody`, Stufen 31c-3/31c-4).
Der Ordner `Eigen/` liegt unverändert im Repository und wird **nicht in die App** gebunden, nur in die Unit-Tests
und das Werkzeug `InstrumentBodyTool`. Herkunft, Fassung, Prüfsumme und die Lizenzen im Einzelnen:
`externals/Eigen/README.md`; die Lizenztexte im Original: `externals/Eigen/COPYING.*`.

> <https://gitlab.com/libeigen/eigen> — **MPL-2.0** („Eigen is primarily MPL2 licensed")

Übersetzt wird mit `EIGEN_MPL2_ONLY`: Dateien mit LGPL-Code lassen sich nicht einbinden. Darin mitgeliefert und
möglicherweise mitübersetzt:

- `Eigen/src/Core/arch/Default/BFloat16.h` — Copyright 2017 The TensorFlow Authors, **Apache-2.0**
  (`externals/Eigen/COPYING.APACHE`).
- `Eigen/src/Core/arch/Default/Half.h` — die Umrechnungen Copyright (c) Fabian Giesen 2016, „Redistribution and use
  in source and binary forms, with or without modification, are permitted."

Die übrigen Teile unter BSD (Anbindungen an MKL, BLAS, LAPACKE; `Geometry/AlignedBox.h`) werden nicht eingebunden.

### Spectra 1.2.0 — `externals/Spectra/`

Eigenwerte großer dünn besetzter Matrizen für den Eigenlöser des Instrumentkörpers (Bibliothek `InstrumentBody`,
Stufe 31c-4); braucht Eigen. Der Ordner `Spectra/` liegt unverändert im Repository (ohne `contrib/`) und wird
**nicht in die App** gebunden, nur in die Unit-Tests und das Werkzeug `InstrumentBodyTool`. Herkunft, Fassung und
Prüfsumme: `externals/Spectra/README.md`; der Lizenztext im Original: `externals/Spectra/LICENSE`.

> <https://github.com/yixuan/spectra> — **MPL-2.0** — Copyright (C) 2016-2025 Yixuan Qiu; einzelne Dateien
> Copyright (C) 2020 Netherlands eScience Center, Copyright (C) 2008-2010 Gael Guennebaud, Copyright (C) 2010 Jitse
> Niesen (aus Eigen übernommen, ebenfalls MPL-2.0)

---

## 2. Portierter Fremdcode in LumiViz-Dateien

Diese Dateien sind LumiViz-Code, ihre **Algorithmen** stammen aber nachweislich
aus fremden Quellen. Die Herkunft ist jeweils im Dateikopf vermerkt. Es gelten
die Bedingungen der Ursprungslizenz — bei den Nullsoft-Quellen also die
oben abgedruckte **BSD-3-Clause**-Klausel samt Haftungsausschluss.

| LumiViz-Datei | Ursprung | Lizenz |
|---|---|---|
| `src/visualizers/MilkdropVisualizer.cpp` | MilkDrop3 (`milkdropfs.cpp`) — Warp-Mesh, Waveform, MD1-Composite, Borders | BSD, Nullsoft |
| `include/visualizers/modules/processing/MilkLoudness.hpp` | MilkDrop3 (`milkdropfs.cpp`, `utility.cpp` `AdjustRateToFPS`) | BSD, Nullsoft |
| `include/visualizers/modules/processing/BeatEstimator.hpp` | vis_avs `bpm.cpp` | BSD-3, Nullsoft 2005 |
| `projects/libs/AvsParser/` | vis_avs `r_*.cpp` — Dateiformat und Feldreihenfolge | BSD-3, Nullsoft 2005 |
| `projects/libs/EelTranspiler/`, `MilkParser`, `HlslTranspiler` | EEL-/MilkDrop-Sprachverhalten als Verhaltensvorlage | s. o. |

Die AVS-Effektmodule in `src/visualizers/modules/` sind **Neuimplementierungen**
gegen den Referenzrenderer, keine Codeübernahmen.

---

## 3. Extern bezogene Abhängigkeiten

Diese Komponenten sind **nicht** im Repository enthalten.

### BASS 2.4 + BASSFLAC — proprietär, muss selbst beschafft werden

> Copyright (c) 1999-2022 Un4seen Developments Ltd. — **proprietär**
> <https://www.un4seen.com/>

**BASS ist kostenlos für nicht-kommerzielle Nutzung. Kommerzielle Nutzung
erfordert eine Lizenz von un4seen.** Weiterverkauf und Unterlizenzierung von
BASS sind ausgeschlossen — deshalb liegt weder das SDK noch eine Binärdatei in
diesem Repository. Beschaffung: [`externals/bass/SETUP.md`](externals/bass/SETUP.md).

Wer ein gebautes LumiViz weitergibt, gibt `bass.dll`/`bassflac.dll` mit weiter
und unterliegt damit den un4seen-Bedingungen.

### Qt 6 — LGPLv3 oder kommerziell

> Copyright (c) The Qt Company Ltd. — **LGPL-3.0** (Open-Source-Variante)
> <https://www.qt.io/>

LumiViz linkt Qt **dynamisch**. Wer ein gebautes LumiViz weitergibt, muss die
LGPL-Pflichten erfüllen: Lizenztext beilegen, auf den Qt-Quelltext verweisen und
den Austausch der Qt-Bibliotheken möglich lassen (dynamisches Linken genügt).

### Qt Advanced Docking System 5.0.0 — LGPL-2.1

> Copyright (c) Uwe Kindler — **LGPL-2.1**
> <https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System>

Wird beim Konfigurieren automatisch geholt (Solution.json), dynamisch gelinkt.
Dieselben LGPL-Pflichten wie bei Qt.

### Lua 5.4 — MIT

> Copyright (C) 1994-2020 Lua.org, PUC-Rio — **MIT**
> <https://www.lua.org/>

### Was sonst noch im Binärpaket liegt

Ein gebautes LumiViz bringt weitere Bibliotheken mit, die Qt beistellt oder das
System verlangt. Sie tauchen im Quelltext nicht auf, wohl aber in jedem ZIP des
Build-Ordners:

| Datei | Herkunft | Lizenz |
|---|---|---|
| `avcodec-*.dll`, `avformat-*.dll`, `avutil-*.dll` | **FFmpeg**, von Qt Multimedia beigestellt | LGPL — <https://ffmpeg.org/> |
| `icuuc.dll` | **ICU**, von Qt beigestellt | ICU/Unicode-Lizenz |
| `dxcompiler.dll`, `dxil.dll` | **DirectX Shader Compiler** (Microsoft) | Microsoft-Redistributable |
| `Qt6*.dll` und die Plugin-Ordner | **Qt 6** | LGPL-3.0 (s. o.) |
| `bass.dll`, `bassflac.dll` | **BASS** (un4seen) | proprietär, nicht-kommerziell frei (s. u.) |
| `lua54.dll` | **Lua 5.4** | MIT |
| (in `LumiViz.exe`) | **miniz 3.1.2**, statisch gelinkt | MIT-Wortlaut, Vermerk in §1 |

**Qt Advanced Docking System wird STATISCH gelinkt** — es steckt also in der
Exe, nicht in einer eigenen DLL. Die LGPL-2.1 verlangt in dem Fall, dass
Empfänger die Bibliothek austauschen können. Das ist erfüllt, weil der gesamte
LumiViz-Quelltext öffentlich ist und jeder neu bauen kann.

### CMakeCraft — das Build-System

> Copyright (c) 2026 Patrik Neunteufel
> <https://github.com/PatrikNeunteufel/CMakeCraft>

Wird beim Konfigurieren in der in `cmakecraft.pin` festgelegten Version geholt.

---

## Mitgelieferte Klänge — `asset/loop/samples/` (CC BY 3.0, CC BY 4.0, CC0)

Vierzig Aufnahmen aus den **OLPC Sound Samples** (One Laptop per Child
music library, zusammengestellt von Richard Boulanger und vielen Beitragenden
am Berklee College of Music), veröffentlicht auf archive.org unter **Creative
Commons Attribution 3.0** (<https://creativecommons.org/licenses/by/3.0/>);
die Lizenz steht je Eintrag in dessen Metadaten (geprüft S123, für die zwei
Cowbells aus TamTam Drum Kits S128). LumiViz hat sie bearbeitet: Stille an den
Rändern weg, am Ende ausgeblendet, Pegel angehoben, Mono, 16 Bit, 44100 Hz.

Ein Schlag Claves (`clave`), geschnitten aus *Clave son 2-3* der Escola
Superior de Música de Catalunya (ESMUC) auf Wikimedia Commons
(<https://commons.wikimedia.org/wiki/File:Clave_son_2-3.wav>), freigegeben
unter **CC0 1.0** (<https://creativecommons.org/publicdomain/zero/1.0/>; Feld
der Dateiseite, geprüft S128). Bearbeitet wie die OLPC-Aufnahmen.

Siebenunddreißig Anschläge (`drs_*`, `crocell_*`) aus zwei Schlagzeug-Kits für
**DrumGizmo** (<https://drumgizmo.org>): **DRSKit 2.1** (DrumGizmo-Team und
Jes Eiler, DRSDrums) und **CrocellKit 1.1** (DrumGizmo-Team, aufgenommen bei
JBOSound), beide unter **Creative Commons Attribution 4.0 International**
(<https://creativecommons.org/licenses/by/4.0/>; Wortlaut der Kit-Seiten,
geprüft S124). Wer die Klänge in einer Komposition verwendet, nennt sie überall
dort, wo auch andere Mitwirkende genannt werden. LumiViz hat sie bearbeitet:
die Mikrofone je Anschlag zu Stereo gemischt, Stille vorn weg, die letzten
10 ms ausgeblendet, Pegel angehoben, 16 Bit, 44100 Hz
(`asset/loop/samples/make_drumgizmo.py`).

Urheber, Eintrag und Datei stehen je Sample in
**`asset/loop/samples/CREDITS.md`** — diese Datei geht mit neben die Exe und
muss bei jeder Weitergabe dabei sein. Neben ihnen sind die Griffbilder unter
`asset/loop/chords/` fremde Inhalte (unten; Entscheid Patrik S123, 2 a, und
S137, S-T4 a).

## Mitgelieferte Akkorde — `asset/loop/chords/` (MIT)

Die Griffbilder für Gitarre (Standardstimmung) und Ukulele unter
`asset/loop/chords/guitar/` und `ukulele/` stammen aus **chords-db** von David
Rubert (<https://github.com/tombatossals/chords-db>, npm-Paket
`@tombatossals/chords-db` 0.5.1), unter der **MIT License**
(„Copyright (c) 2016 David Rubert“; geprüft S137). LumiViz hat sie umgerechnet:
je Lage eine Datei `.lvchord`, die Bünde ab dem Sattel, gegen die Töne der
Quelle geprüft (`asset/loop/chords/make_chords.py`). Der Lizenztext steht in
**`asset/loop/chords/CREDITS.md`** — diese Datei geht mit neben die Exe. Die
Griffbilder unter `drop_d/`, `bass/`, `violin/` und die Voicings unter
`voicings/` sind von LumiViz gerechnet.

## Nicht enthaltene Inhalte

Bewusst **nicht** in diesem Repository, weil die Rechte bei Dritten liegen oder
ungeklärt sind:

- **AVS-Preset-Sammlungen** der Community (GreatWho-Packs, Community Picks u. ä.)
- **MilkDrop-Preset- und Textur-Sammlungen**
  (Herkunft: <https://github.com/milkdrop2077/MilkDrop3>)
- **ISF-Shader-Bibliothek** von Vidvox (<https://isf.video/>)
- **Shadertoy-Shader** fremder Autoren
- **Logos und Marken** von Winamp/AVS und MilkDrop

LumiViz kann diese Dateien **laden**; mitgeliefert werden sie nicht. Die in
`asset/` enthaltenen Presets, Ketten und Shader sind Eigenwerk bzw. von den
Skripten unter `asset/calibration/` erzeugt — ausgenommen die Samples unter
`asset/loop/samples/` und die Griffbilder aus chords-db unter
`asset/loop/chords/` (oben).
