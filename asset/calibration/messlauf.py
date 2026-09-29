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

import os
import subprocess

# Ohne Konsole starten (nur Windows kennt das Flag)
OHNE_KONSOLE = ({"creationflags": subprocess.CREATE_NO_WINDOW}
                if os.name == "nt" else {})


def run(cmd, **kw) -> subprocess.CompletedProcess:
    """`subprocess.run` ohne Konsolenfenster — fuer JEDEN Werkzeugstart."""
    flags = kw.pop("creationflags", 0)
    return subprocess.run(cmd, creationflags=flags | OHNE_KONSOLE.get("creationflags", 0),
                          **kw)


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
