# BASS-Binaries — bewusst nicht im Repo (Lizenz)

> **Warum fehlen hier Dateien?** BASS und seine Addons sind proprietär (un4seen developments).
> Die **Binärdateien** (Import-Libs, DLLs, Linux-Libs, Beispiel-Executables) werden aus
> Lizenzgründen **nicht versioniert** — die .gitignore-Regeln `x64/`, `x86/`, `bin/` halten sie
> bewusst draußen. Header und SDK-Struktur sind getrackt.
>
> **Symptom bei frischem Klon/Checkout:** Linker-Fehler wie
> `…/externals/bass/bass24/win/c/x64/bass.lib … missing and no known rule to make it`.

## Beschaffung bei Bedarf

**Variante A — von einer bestehenden Arbeitskopie kopieren** (schnellster Weg, gleiche Struktur):

```bash
# vom alten/funktionierenden Checkout aus (Beispiel):
cd <alte-Arbeitskopie>
git ls-files -o -i --exclude-standard -z -- externals | tar --null -cf - -T - | (cd <neuer-Checkout> && tar -xf -)
```

**Variante B — Download von un4seen.com** (https://www.un4seen.com/):
Je benötigtem Paket das ZIP laden und **in die vorhandene SDK-Struktur** entpacken
(die getrackten Header zeigen, wohin). Benötigt werden je Addon typischerweise:

| Zielpfad (relativ zu `externals/bass/<addon>/`) | Inhalt |
|---|---|
| `win/c/x64/<name>.lib` | Import-Library für den MSVC/Clang-Link (das ist die Datei aus dem Linker-Fehler) |
| `win/x64/<name>.dll` | Laufzeit-DLL (wird neben die EXE deployt; Plugin-Auto-Loading lädt aus dem EXE-Verzeichnis) |
| `linux/libs/x86_64/` bzw. `linux/libs/x86/` | Linux-Shared-Libs |
| `win/c/bin/` | Beispiel-EXEs des SDK (nur Referenz, nicht nötig zum Bauen) |

**Aktuell genutzte Pakete** (Solution.json: `bass` mit `BASS_FLAC: true`): mindestens
`bass24` und `bassflac24`; weitere Addons (wasapi, mix, enc, opus, …) liegen als SDK-Struktur
bereit und brauchen ihre Binaries nur, wenn sie eingebunden werden.

Auch betroffen (kein BASS, gleiche Regel-Logik): `externals/lua54/win/bin/` (Lua-Tools).

## Automatisierung (geplant)

Das Build-System (CMakeCraft) hat eine Prefetch-Hook-Infrastruktur
(`cmake/externals/hooks/prefetch/`). Ein `Bass_PreFetch`-Hook, der fehlende Binaries erkennt und
von un4seen lädt, ist als Phase-1-Aufgabe vorgemerkt — bis dahin gilt Variante A/B.

*Lizenz: BASS ist für Freeware kostenlos nutzbar, kommerzielle Nutzung erfordert eine Lizenz —
siehe un4seen.com. Keine Weiterverbreitung der Binaries über dieses Repo.*
