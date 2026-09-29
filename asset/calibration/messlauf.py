# -*- coding: utf-8 -*-
"""Gemeinsame Bausteine der Referenzlaeufe (S81) — MilkDrop und AVS.

1. **Start ohne Konsolenfenster.** Die Skripte laufen oft aus einer Umgebung
   ohne eigene Konsole (Claude-Shell, geplante Aufgaben). Dann gibt Windows
   JEDEM gestarteten Konsolenprogramm (MilkdropRef, AvsRef, die Standalones)
   ein neues, sichtbares Konsolenfenster — am Standardort, unabhaengig von
   `LUMIVIZ_REF_BILDSCHIRM`. Die Ausgabe fangen die Skripte ohnehin ab; die
   Fenster zeigen nichts. `run()` startet deshalb IMMER mit
   `CREATE_NO_WINDOW`. Die Renderfenster (GUI) bleiben sichtbar.

2. **Formate.** Mehrere Frame-Marken trennen Phasenversatz von echtem
   Fehler; mehrere FORMATE trennen Fehler im Umgang mit dem Seitenverhaeltnis
   von allen anderen (Befund S81: Custom-Waves ohne `m_fInvAspectY` —
   quadratisch 0,003, 4:3 0,122). Die Aspekt-Reihe haelt die **Pixelzahl**
   ungefaehr gleich (~307 000 wie 640×480), damit sich nur die Form aendert,
   nicht die Groesse; alle Kanten sind Vielfache von 16 (der MilkDrop-Kern
   rundet seinen Puffer sonst, z. B. 734 → 736). Groesse ist eine EIGENE
   Achse: `monitor` = die physische Aufloesung des Monitors aus
   `LUMIVIZ_REF_BILDSCHIRM` — dort zeigen sich Fehler, die an festen
   Pixelmassen haengen (Versatz, Blur-Radius, Punktgroesse, Rundung krummer
   Kanten).
"""
from __future__ import annotations

import contextlib
import os
import subprocess
import sys

# Ohne Konsole starten (nur Windows kennt das Flag)
OHNE_KONSOLE = ({"creationflags": subprocess.CREATE_NO_WINDOW}
                if os.name == "nt" else {})


def run(cmd, **kw) -> subprocess.CompletedProcess:
    """`subprocess.run` ohne Konsolenfenster — fuer JEDEN Werkzeugstart."""
    flags = kw.pop("creationflags", 0)
    return subprocess.run(cmd, creationflags=flags | OHNE_KONSOLE.get("creationflags", 0),
                          **kw)


# --- Grafikkarte (S81) ------------------------------------------------------------------
# Referenz UND Pruefling laufen auf DERSELBEN Karte (Vorgabe Patrik). Anlass:
# der MilkDrop-Kern paart ps_3_0 mit vs_1_1; die Radeon 610M zeichnet solche
# Presets gar nicht (Helix, Beauty, Starfield-edit — „stumm" seit S75), die
# 4090 schon. Standard ist die 4090; `610m` bei Bedarf.
#   Name -> (Kennung im Adapternamen, Windows-GpuPreference)
KARTEN = {
    "4090": ("4090", 2),   # Hohe Leistung
    "610m": ("610M", 1),   # Energiesparen
}
GPU_SCHLUESSEL = r"Software\Microsoft\DirectX\UserGpuPreferences"


def pruefe_python() -> None:
    """Die Store-Fassung von Python reicht den GPU-Eintrag NICHT an gestartete
    Prozesse weiter (Paket-Kontext, S81) — die Karte waere dann Zufall."""
    if "WindowsApps" in sys.executable:
        raise SystemExit(
            "FEHLER: Store-Python (" + sys.executable + ") reicht die GPU-Wahl "
            "nicht an die Werkzeuge weiter. Mit dem normalen Python starten, "
            r"z. B. C:\Users\patri\AppData\Local\Programs\Python\Python312\python.exe")


def gpu_eintrag(exe: str) -> str | None:
    """Aktueller UserGpuPreferences-Wert fuer `exe` (None = kein Eintrag)."""
    import winreg
    try:
        with winreg.OpenKey(winreg.HKEY_CURRENT_USER, GPU_SCHLUESSEL) as k:
            return winreg.QueryValueEx(k, exe)[0]
    except OSError:
        return None


def gpu_eintrag_setzen(exe: str, wert: str | None) -> None:
    """Eintrag setzen (`"GpuPreference=2;"`) oder mit None loeschen."""
    import winreg
    with winreg.CreateKey(winreg.HKEY_CURRENT_USER, GPU_SCHLUESSEL) as k:
        if wert is None:
            with contextlib.suppress(OSError):
                winreg.DeleteValue(k, exe)
        else:
            winreg.SetValueEx(k, exe, 0, winreg.REG_SZ, wert)


@contextlib.contextmanager
def wach_halten():
    """Standby verhindern, solange der Block laeuft (S81) — wie ein
    Videoplayer, ueber SetThreadExecutionState; endet mit dem Block bzw. dem
    Prozess. Vorsorge: ein Messlauf soll nicht in den Standby fallen. (Der
    Absturz vom 29.09. 16:21 war KEIN Standby-Fall — kein Kernel-Power-506
    davor, Standby erst nach 60 min; er geschah im Betrieb unter Last.)"""
    if os.name != "nt":
        yield
        return
    import ctypes
    es_continuous, es_system_required = 0x80000000, 0x00000001
    ctypes.windll.kernel32.SetThreadExecutionState(es_continuous | es_system_required)
    try:
        yield
    finally:
        ctypes.windll.kernel32.SetThreadExecutionState(es_continuous)


# CPU-Werte aus LibreHardwareMonitor (S81): dessen Webserver liefert
# /data.json, wenn er laeuft (Options → Remote Web Server, als Administrator
# gestartet — sonst fehlen die CPU-Sensoren). Spalte → SensorId.
LHM_URL = "http://localhost:8085/data.json"
LHM_SENSOREN = {
    "cpu_tctl_C": "/amdcpu/0/temperature/2",
    "cpu_package_W": "/amdcpu/0/power/0",
    "cpu_last_pct": "/amdcpu/0/load/0",
    "gpu_hotspot_C": "/gpu-nvidia/0/temperature/2",
}
LHM_SPALTEN = list(LHM_SENSOREN)


def lhm_werte() -> list[str]:
    """Aktuelle LHM-Werte in LHM_SPALTEN-Reihenfolge; leer, wenn LHM fehlt."""
    import json
    import urllib.request
    try:
        with urllib.request.urlopen(LHM_URL, timeout=2) as antwort:
            daten = json.load(antwort)
    except Exception:  # noqa: BLE001 — LHM laeuft nicht: Spalten bleiben leer
        return [""] * len(LHM_SPALTEN)
    gefunden: dict[str, str] = {}

    def gehe(knoten):
        sid = knoten.get("SensorId")
        if sid:
            gefunden[sid] = str(knoten.get("Value", "")).split(" ")[0].replace(",", ".")
        for kind in knoten.get("Children", []):
            gehe(kind)

    gehe(daten)
    return [gefunden.get(sid, "") for sid in LHM_SENSOREN.values()]


@contextlib.contextmanager
def gpu_protokoll(ziel):
    """NVIDIA-Karte je Sekunde mitschreiben (S81), solange der Block laeuft.

    Anlass: Absturz mitten im Kalibrierlauf (29.09. 16:21, Kernel-Power 41
    ohne Bugcheck — hart abgeschaltet, kein Protokoll). Windows fuehrt keinen
    Temperaturverlauf; diese CSV ueberlebt einen Absturz bis zur letzten
    Zeile. Ohne nvidia-smi: stiller Verzicht."""
    import shutil as _sh
    import threading
    exe = _sh.which("nvidia-smi")
    if not exe:
        yield False
        return
    # Je Sekunde EINE Abfrage, Zeile sofort auf die Platte (flush + fsync) —
    # `nvidia-smi -l -f` puffert und verliert beim Beenden bzw. Absturz alles
    felder = ("timestamp,temperature.gpu,power.draw,clocks.gr,utilization.gpu,"
              "pstate,clocks_throttle_reasons.active")
    stopp = threading.Event()

    def schreiben():
        with open(ziel, "w", encoding="utf-8") as f:
            f.write(felder + "," + ",".join(LHM_SPALTEN) + "\n")
            while not stopp.is_set():
                with contextlib.suppress(Exception):
                    zeile = subprocess.run(
                        [exe, f"--query-gpu={felder}", "--format=csv,noheader"],
                        capture_output=True, text=True, timeout=5,
                        **OHNE_KONSOLE).stdout.strip()
                    if zeile:
                        f.write(zeile + "," + ",".join(lhm_werte()) + "\n")
                        f.flush()
                        os.fsync(f.fileno())
                stopp.wait(1.0)

    faden = threading.Thread(target=schreiben, daemon=True)
    faden.start()
    try:
        yield True
    finally:
        stopp.set()
        faden.join(timeout=10)


@contextlib.contextmanager
def karte(name: str, exes: list[str]):
    """Alle `exes` fuer die Dauer des Laufs auf die Karte `name` stellen und
    danach den vorherigen Stand wiederherstellen. Die Windows-Pfade muessen
    exakt stimmen (Backslashes, absoluter Pfad) — der Eintrag ist der Pfad."""
    if name not in KARTEN:
        raise ValueError(f"unbekannte Karte '{name}' (bekannt: {', '.join(KARTEN)})")
    pfade = [str(os.path.abspath(e)).replace("/", "\\") for e in exes]
    vorher = {p: gpu_eintrag(p) for p in pfade}
    for p in pfade:
        gpu_eintrag_setzen(p, f"GpuPreference={KARTEN[name][1]};")
    try:
        yield KARTEN[name][0]
    finally:
        for p, alt in vorher.items():
            gpu_eintrag_setzen(p, alt)


# Aspekt-Reihe: ~gleiche Pixelzahl, Kanten Vielfache von 16. Reihenfolge =
# Berichtsspalten; das erste Format ist die quadratische Kontrolle.
FORMATE_ASPEKT = [
    ("1:1", "560x560"),
    ("4:3", "640x480"),
    ("16:9", "736x416"),
    ("3,4:1", "1024x304"),   # Streifen wie DISPLAY2 (2560×734)
    ("3:4 hoch", "480x640"),
]


def monitor_groesse(name: str | None = None) -> str | None:
    """Physische Aufloesung „WxH" des Monitors `name` (Vorgabe:
    `LUMIVIZ_REF_BILDSCHIRM`; Geraetename `DISPLAYn` oder Index)."""
    name = name or os.environ.get("LUMIVIZ_REF_BILDSCHIRM", "")
    if os.name != "nt" or not name:
        return None
    import ctypes
    from ctypes import wintypes
    user32 = ctypes.windll.user32
    try:  # physische Pixel, nicht die DPI-virtualisierten
        user32.SetProcessDpiAwarenessContext(ctypes.c_void_p(-4))
    except (AttributeError, OSError):
        pass

    class Info(ctypes.Structure):
        _fields_ = [("cb", wintypes.DWORD), ("mon", wintypes.RECT),
                    ("arbeit", wintypes.RECT), ("flags", wintypes.DWORD),
                    ("geraet", ctypes.c_wchar * 32)]

    liste = []
    proto = ctypes.WINFUNCTYPE(ctypes.c_int, wintypes.HMONITOR, wintypes.HDC,
                               ctypes.POINTER(wintypes.RECT), wintypes.LPARAM)

    def sammeln(h, _dc, _r, _l):
        i = Info()
        i.cb = ctypes.sizeof(Info)
        if user32.GetMonitorInfoW(h, ctypes.byref(i)):
            liste.append((i.geraet.replace("\\\\.\\", "").upper(),
                          i.mon.right - i.mon.left, i.mon.bottom - i.mon.top))
        return 1

    user32.EnumDisplayMonitors(None, None, proto(sammeln), 0)
    wunsch = name.upper().replace("\\\\.\\", "")
    for geraet, w, h in liste:
        if geraet == wunsch:
            return f"{w}x{h}"
    if wunsch.isdigit() and int(wunsch) < len(liste):
        _, w, h = liste[int(wunsch)]
        return f"{w}x{h}"
    return None


def formate_aufloesen(text: str) -> list[tuple[str, str]]:
    """`--formate`-Wert → [(Bezeichnung, "WxH")].

    Bausteine (Komma-getrennt, frei kombinierbar):
    - `aspekt`  — die Aspekt-Reihe oben
    - `monitor` — physische Groesse des Monitors aus LUMIVIZ_REF_BILDSCHIRM
    - `monitor:DISPLAY1` — physische Groesse eines bestimmten Monitors
    - `WxH`     — ein festes Format
    """
    formate: list[tuple[str, str]] = []
    for teil in (t.strip() for t in text.split(",") if t.strip()):
        low = teil.lower()
        if low == "aspekt":
            formate += FORMATE_ASPEKT
        elif low.startswith("monitor"):
            name = teil.split(":", 1)[1] if ":" in teil else None
            groesse = monitor_groesse(name)
            if groesse is None:
                raise ValueError(f"--formate {teil}: Monitor nicht gefunden "
                                 "(LUMIVIZ_REF_BILDSCHIRM gesetzt?)")
            formate.append((f"Monitor {name or os.environ.get('LUMIVIZ_REF_BILDSCHIRM')}",
                            groesse))
        elif "x" in low:
            formate.append((teil, teil))
        else:
            raise ValueError(f"--formate: unbekannter Baustein '{teil}'")
    # Doppelte Formate nur einmal messen
    gesehen, eindeutig = set(), []
    for bez, groesse in formate:
        if groesse not in gesehen:
            gesehen.add(groesse)
            eindeutig.append((bez, groesse))
    return eindeutig
