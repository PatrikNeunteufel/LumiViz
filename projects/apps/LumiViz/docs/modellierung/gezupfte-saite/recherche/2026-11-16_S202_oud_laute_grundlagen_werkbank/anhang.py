"""Haengt die Rechenskripte und ihre Ausgaben an bericht.md an (Auftrag 2026-11-16_15).
Aufruf: python -B -I <dieses Skript>   (einmal; prueft, ob der Anhang schon da ist)"""
import pathlib
import subprocess
import sys

HIER = pathlib.Path(r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz_Research\ergebnisse\2026-11-16_15_oud_laute_grundlagen")
BASSCHOR_AUSGABE = pathlib.Path(r"C:\Users\patri\AppData\Local\Temp\claude\0ef534e7da2728c8\5cc20833-6961-4984-9db8-473311cec840"
                                r"\tasks\bmi6jjqu8.output")
bericht = HIER / "bericht.md"
text = bericht.read_text(encoding="utf-8")
if "### A1 " in text:
    sys.exit("Anhang schon vorhanden")
teile = []
for n, (skript, flags) in enumerate([("rechnung_saiten.py", ["-I"]), ("rechnung_choere.py", ["-B", "-I"]),
                                      ("rechnung_basschor.py", None), ("rechnung_bauplan.py", ["-I"]), ("rechnung_rest.py", ["-I"]),
                                      ("rauwolf_steg.py", "nicht")], 1):
    code = (HIER / skript).read_text(encoding="utf-8")
    if flags == "nicht":
        teile.append(f"\n### A{n} `{skript}`\n\nVersuch, die Masszahlen am Steg ueber `pdftotext -bbox` zu finden; die "
                     "installierte Xpdf-Fassung kennt `-bbox` nicht. Der Ausschnitt `seiten/rauwolf_steg.png` entstand "
                     "stattdessen direkt mit `pdftoppm -r 120 -x 3960 -y 3170 -W 1040 -H 530` nach einer Uebersicht bei 20 dpi "
                     "(`seiten/rauwolf_ganz20.png`). Das Skript bleibt als Beleg des Versuchs liegen, ohne Ausgabe.\n")
        continue
    if flags is None:
        aus = BASSCHOR_AUSGABE.read_text(encoding="utf-8").replace("[exited with code 0]", "").rstrip()
        wie = "Ausgabe des Laufs in diesem Auftrag (Hintergrund, rund 5 Minuten)"
    else:
        r = subprocess.run([sys.executable, *flags, str(HIER / skript)], capture_output=True, text=True, encoding="utf-8")
        aus = (r.stdout + r.stderr).rstrip()
        wie = "Ausgabe"
    teile.append(f"\n### A{n} `{skript}`\n\n```python\n{code.rstrip()}\n```\n\n{wie}:\n\n```text\n{aus}\n```\n")
bericht.write_text(text.rstrip() + "\n" + "".join(teile), encoding="utf-8", newline="\n")
print("angehaengt:", len(teile), "Teile")
