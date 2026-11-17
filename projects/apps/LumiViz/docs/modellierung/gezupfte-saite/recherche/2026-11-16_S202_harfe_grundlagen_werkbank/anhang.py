"""Haengt an bericht.md je Skript den Wortlaut und die Ausgabe eines frischen Laufs an."""
import os, subprocess, sys
D = r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz_Research\ergebnisse\2026-11-16_12_harfe_grundlagen"
SKRIPTE = ["r1_saiten.py", "r2_mittelwert.py", "r4_waltham_grau.py", "r6_koerper_rechnen.py",
           "r7_abstrahlung_je_schnelle.py", "r8_saiten_auf_moden.py", "r9_z1_je_term.py", "r10_lecarrou_2fhg.py"]
teile = []
for s in SKRIPTE:
    pfad = os.path.join(D, s)
    code = open(pfad, encoding="utf-8").read()
    env = dict(os.environ, PYTHONUTF8="1")
    r = subprocess.run([sys.executable, pfad], capture_output=True, text=True, encoding="utf-8", env=env, cwd=D)
    aus = r.stdout + (("\n[stderr]\n" + r.stderr) if r.stderr.strip() else "")
    teile.append(f"\n### {s}\n\n```python\n{code.rstrip()}\n```\n\nAusgabe (Exit-Code {r.returncode}):\n\n```text\n{aus.rstrip()}\n```\n")
    print(s, r.returncode)
with open(os.path.join(D, "bericht.md"), "a", encoding="utf-8", newline="\n") as f:
    f.write("".join(teile))
print("angehaengt")
