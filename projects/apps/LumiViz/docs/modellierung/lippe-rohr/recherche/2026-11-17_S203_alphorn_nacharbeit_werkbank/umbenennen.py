"""Zwei als .pdf abgelegte HTML-Antworten umbenennen und INDEX.md nachfuehren (nichts loeschen)."""
from pathlib import Path

ABR = Path(__file__).resolve().parent / "abrufe"
PAARE = [
    ("Eveno_2012_Acoustics_resonance_playing_freq_trumpet.pdf", "hal_anubis_sperrseite_hal-00810906.html"),
    ("Wey_2024_Arts_19th_century_alphorn_timbre.pdf", "bfh_arbor_dspace_huelle_wey2024.html"),
]
idx = ABR / "INDEX.md"
text = idx.read_text(encoding="utf-8")
for alt, neu in PAARE:
    (ABR / alt).rename(ABR / neu)
    text = text.replace(f"| {alt} |", f"| {neu} (abgelegt als {alt}; HTML statt PDF) |")
    print(alt, "->", neu)
idx.write_text(text, encoding="utf-8")
