"""Hoertest: blinde A/B-Vergleiche im Browser (LumiViz, S166, Wunsch Patrik).

Aufruf aus dem Projekt-Root:

    python -X utf8 -I tools/hoertest/hoertest.py <ordner> [--port 8765]
    python -X utf8 -I tools/hoertest/hoertest.py <ordner> --auswertung

<ordner> enthaelt hoertest.json und die Klaenge, die es nennt (geschrieben von einem Werkzeug der Unit-Tests,
etwa LUMIVIZ_PV1_WAV in test_LoopSalamanderHoerproben.cpp):

    {"titel": "...",
     "fragen":     [{"id", "text", "links", "rechts"}, ...],      je Klang auf 1-5
     "klaenge":    [{"id", "datei", "taste", "name", "staerke", "fassung", "sekunden"}, ...],
     "vergleiche": [{"id", "a", "b", "kontrolle"}, ...]}          a, b: ids aus klaenge

Der Server laeuft nur auf 127.0.0.1 und liefert die Seite index.html neben diesem Skript. Die Seite stellt die
Vergleiche in zufaelliger Reihenfolge, legt A und B je Vergleich zufaellig fest und zeigt nie, welcher Klang welcher
ist. Jede Antwort wird sofort als eine Zeile an <ordner>/antworten.jsonl angehaengt; beantwortete Vergleiche kommen
beim naechsten Start nicht wieder (eine spaetere Antwort auf denselben Vergleich gilt). --auswertung liest die
Antworten, ordnet A und B den Fassungen zu und schreibt <ordner>/auswertung.txt. Nur Standardbibliothek.
"""

import argparse
import json
import statistics
import sys
from datetime import datetime
from http import HTTPStatus
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

SEITE = Path(__file__).resolve().parent / "index.html"
ANTWORTEN = "antworten.jsonl"


def lies_test(ordner: Path) -> dict:
    return json.loads((ordner / "hoertest.json").read_text(encoding="utf-8"))


def lies_antworten(ordner: Path) -> dict:
    """Die Antworten je Vergleich; die letzte Zeile zu einem Vergleich gilt."""
    antworten = {}
    pfad = ordner / ANTWORTEN
    if not pfad.exists():
        return antworten
    for zeile in pfad.read_text(encoding="utf-8").splitlines():
        if zeile.strip():
            eintrag = json.loads(zeile)
            antworten[eintrag["id"]] = eintrag
    return antworten


def handler_fuer(ordner: Path):
    class Handler(BaseHTTPRequestHandler):
        def log_message(self, format, *args):  # nur Fehler ausgeben
            pass

        def schicke(self, inhalt: bytes, art: str, status=HTTPStatus.OK):
            self.send_response(status)
            self.send_header("Content-Type", art)
            self.send_header("Content-Length", str(len(inhalt)))
            self.send_header("Cache-Control", "no-store")
            self.end_headers()
            self.wfile.write(inhalt)

        def do_GET(self):
            pfad = self.path.split("?", 1)[0]
            if pfad == "/":
                self.schicke(SEITE.read_bytes(), "text/html; charset=utf-8")
            elif pfad == "/hoertest.json":
                self.schicke((ordner / "hoertest.json").read_bytes(), "application/json; charset=utf-8")
            elif pfad == "/antworten":
                ids = sorted(lies_antworten(ordner).keys())
                self.schicke(json.dumps(ids).encode("utf-8"), "application/json; charset=utf-8")
            elif pfad.startswith("/k/"):
                datei = (ordner / pfad[3:]).resolve()
                if not datei.is_relative_to(ordner) or not datei.is_file() or datei.suffix.lower() != ".wav":
                    self.schicke(b"nicht gefunden", "text/plain", HTTPStatus.NOT_FOUND)
                    return
                self.schicke(datei.read_bytes(), "audio/wav")
            else:
                self.schicke(b"nicht gefunden", "text/plain", HTTPStatus.NOT_FOUND)

        def do_POST(self):
            if self.path != "/antwort":
                self.schicke(b"nicht gefunden", "text/plain", HTTPStatus.NOT_FOUND)
                return
            laenge = int(self.headers.get("Content-Length", "0"))
            try:
                eintrag = json.loads(self.rfile.read(laenge).decode("utf-8"))
                if not isinstance(eintrag, dict) or "id" not in eintrag:
                    raise ValueError("ohne id")
            except ValueError as fehler:
                self.schicke(str(fehler).encode("utf-8"), "text/plain", HTTPStatus.BAD_REQUEST)
                return
            eintrag["zeit"] = datetime.now().isoformat(timespec="seconds")
            with (ordner / ANTWORTEN).open("a", encoding="utf-8") as aus:
                aus.write(json.dumps(eintrag, ensure_ascii=False) + "\n")
            print(f"Antwort {eintrag['id']}", flush=True)
            self.schicke(b"ok", "text/plain")

    return Handler


def mittel(werte):
    werte = [w for w in werte if w is not None]
    return f"{statistics.mean(werte):.1f}" if werte else "-"


def auswertung(ordner: Path) -> str:
    test = lies_test(ordner)
    klaenge = {k["id"]: k for k in test["klaenge"]}
    vergleiche = {v["id"]: v for v in test["vergleiche"]}
    fragen = [f["id"] for f in test["fragen"]]
    antworten = lies_antworten(ordner)
    zeilen = [f"Auswertung {test.get('titel', '')}: {len(antworten)} von {len(vergleiche)} Vergleichen beantwortet",
              "Werte 1-5 je Frage (" + ", ".join(f"{f['id']} = {f['links']} .. {f['rechts']}" for f in test["fragen"]) + ")", ""]

    je_fassung = {}  # (taste, fassung) -> frage -> [werte]
    kontrolle = []   # |A - B| bei gleichem Klang
    vorzug = {}      # paar -> {fassung: zahl}
    zeilen.append("Je Vergleich (Fassung: Werte in der Reihenfolge der Fragen; Vorzug = klingt mehr nach echtem Fluegel)")
    for vid in sorted(antworten, key=lambda i: list(vergleiche).index(i) if i in vergleiche else 999):
        a = antworten[vid]
        v = vergleiche.get(vid)
        if v is None:
            continue
        ka, kb = klaenge[a["a"]], klaenge[a["b"]]
        werte = a.get("werte", {})
        for seite, k in (("A", ka), ("B", kb)):
            ziel = je_fassung.setdefault((k["name"], k["fassung"]), {f: [] for f in fragen})
            for f in fragen:
                ziel[f].append(werte.get(seite, {}).get(f))
        if v.get("kontrolle"):
            for f in fragen:
                wa, wb = werte.get("A", {}).get(f), werte.get("B", {}).get(f)
                if wa is not None and wb is not None:
                    kontrolle.append(abs(wa - wb))
        wahl = a.get("vorzug")
        sieger = ka["fassung"] if wahl == "A" else kb["fassung"] if wahl == "B" else "gleich"
        paar = " / ".join(sorted({ka["fassung"], kb["fassung"]}))
        vorzug.setdefault(paar, {}).setdefault(sieger, 0)
        vorzug[paar][sieger] += 1
        text = lambda seite, k: k["fassung"] + " " + " ".join(str(werte.get(seite, {}).get(f) or "?") for f in fragen)
        zeilen.append(f"  {ka['name']} {ka['staerke']:>2}: A {text('A', ka)} | B {text('B', kb)} | Vorzug {sieger}"
                      + (" (Kontrolle)" if v.get("kontrolle") else "")
                      + (f" | {a['bemerkung']}" if a.get("bemerkung") else ""))

    zeilen += ["", "Mittel je Taste und Fassung: " + " ".join(fragen)]
    for (name, fassung) in sorted(je_fassung):
        werte = je_fassung[(name, fassung)]
        zeilen.append(f"  {name} {fassung:<10} " + " ".join(f"{mittel(werte[f]):>4}" for f in fragen))
    zeilen += ["", "Vorzug je Paar (Anzahl Vergleiche)"]
    for paar, zahlen in sorted(vorzug.items()):
        zeilen.append(f"  {paar}: " + ", ".join(f"{s} {n}" for s, n in sorted(zahlen.items())))
    zeilen += ["", "Kontrolle (zweimal derselbe Klang): mittlerer Abstand A-B "
               + (f"{statistics.mean(kontrolle):.2f} Stufen ueber {len(kontrolle)} Werte" if kontrolle else "- (keine)")]
    return "\n".join(zeilen) + "\n"


def main():
    p = argparse.ArgumentParser(description="Blinde A/B-Hoervergleiche im Browser")
    p.add_argument("ordner", type=Path, help="Ordner mit hoertest.json und den Klaengen")
    p.add_argument("--port", type=int, default=8765)
    p.add_argument("--auswertung", action="store_true", help="Antworten auswerten statt den Server zu starten")
    args = p.parse_args()
    ordner = args.ordner.resolve()
    if not (ordner / "hoertest.json").is_file():
        sys.exit(f"hoertest.json fehlt in {ordner}")
    if args.auswertung:
        text = auswertung(ordner)
        (ordner / "auswertung.txt").write_text(text, encoding="utf-8")
        print(text, end="")
        return
    server = ThreadingHTTPServer(("127.0.0.1", args.port), handler_fuer(ordner))
    print(f"Hoertest: http://127.0.0.1:{args.port}/  (Ordner {ordner}; beenden mit Strg+C)", flush=True)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    main()
