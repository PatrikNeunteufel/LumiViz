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

**Qt Advanced Docking System wird STATISCH gelinkt** — es steckt also in der
Exe, nicht in einer eigenen DLL. Die LGPL-2.1 verlangt in dem Fall, dass
Empfänger die Bibliothek austauschen können. Das ist erfüllt, weil der gesamte
LumiViz-Quelltext öffentlich ist und jeder neu bauen kann.

### CMakeCraft — das Build-System

> Copyright (c) 2026 Patrik Neunteufel
> <https://github.com/PatrikNeunteufel/CMakeCraft>

Wird beim Konfigurieren in der in `cmakecraft.pin` festgelegten Version geholt.

---

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
Skripten unter `asset/calibration/` erzeugt.
