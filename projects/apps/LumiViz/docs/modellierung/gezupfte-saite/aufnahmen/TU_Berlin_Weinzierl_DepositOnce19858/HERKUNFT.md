# Herkunft — TU Berlin, reflexionsarme Einzeltöne mit 32 Mikrofonen: Gitarre und Doppelpedalharfe

- **Quelle:** S. Weinzierl, M. Vorländer, D. Ackermann u. a., „A Database of Anechoic Microphone Array Measurements of
  Musical Instruments“ / „Musical Instruments – A data base with recordings, directivities, and features of classical
  musical instruments“, DepositOnce (TU Berlin), Version 3 vom 2024-02-12, DOI 10.14279/depositonce-19858 (die in der
  Doku genannte DOI `…-5861.3` antwortet mit 404); Seite
  `depositonce.tu-berlin.de/items/b194da8b-e98c-4761-9100-2b0bb6ba93e8`. Geladen am **2026-11-17** (LumiViz S203,
  Freigabe Patrik, KQ33 a) mit `curl`, nur diese zwei Dateien.
- **Lizenz:** die Dokumentation v3 S. 2 (Textebene, in S203 gelesen; Ablage `…\cmake\quellen\koerper\Weinzierl_et_al_2017_…
  _DepositOnce19858.pdf`): „The database is provided under a Creative Commons BY-NC-SA licence“ mit Verweis auf
  creativecommons.org/licenses/by-nc-sa/4.0. Die Seite von DepositOnce zeigt nur das Zeichen „In Copyright“
  (rightsstatements.org InC 1.0) — Widerspruch vermerkt; es gilt die Doku. Nicht kommerziell, Weitergabe unter gleicher
  Lizenz; nicht in ein Repo und nicht in eine Auslieferung.
- **Inhalt:** je Note und Stärke eine SOFA-Datei (`<Instrument>_<pp|ff>_<Note>_recordings.sofa`, 32 Kanäle, 44,1 kHz,
  Note und Stärke in `GLOBAL_Description`, MIDI in `MidiNote`; Doku S. 2–3). Gitarre (Masaru Kohno 1985 „Concert“, Diskant
  Nylon, Bass umsponnen) 88 Einträge, Umfang pp C2–B5, ff E2–B5; Doppelpedalharfe 156 Einträge, C1–F4 in pp und ff
  (Doku Tab. 1). Aufgenommen im reflexionsarmen Raum, Mikrofone auf einer Kugel (Durchmesser 2,1 m laut Ackermann u. a.
  2023; die Doku nennt S1 = 54,63 m², das sind 5,96 dB mehr als die Kugel durch die Mikrofone — für absolute Leistung
  klären).
- **Größe:** `2_Acoustic_guitar_modern_recordings.zip` 1 095 042 381 B, `2_Double_action_harp_modern_recordings.zip`
  2 621 077 413 B. Prüfsummen: `SHA256SUMS`, Adressen: `LADELISTE.tsv`. Nicht entpackt.
- **Zweck:** Schwellen für Spanne und Sprünge der Lautheit je Taste ohne Raum (KQ33 a, ändert die Herkunft von GZ14 a),
  je Ton die Leistungssumme der 32 Mikrofone; Gitarre und Harfe bis F4.
