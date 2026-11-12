# 34k-5, erster Fall nach KQ6 a: Ersatz-Bauplan der Oud gegen vier gemessene Moden

> **Sitzung:** S195 (2026-11-12). Quelle: Abdallah u. a. 2026 (Meccanica 61:89, CC BY 4.0), am Original gelesen.
> Programme unter `proben/`: `2026-11-12_S195_oud_umriss.py` (liest Umriss, Balkenlagen und Löcher aus Abb. 3),
> `…_oud_umriss.json` (das Gelesene), `…_oud_bauplan.py` (schreibt den Bauplan; Aufruf
> `<umriss.json> <ordner> <dicke mm> [<steg cm> <breite mm> <höhe mm>]`), `…_oud_moden.py`. Gerechnet mit
> `InstrumentBodyTool <plan> --to 500` (Release-clang, unter einer Sekunde je Lauf). **Nichts am Programm geändert, kein
> Bauplan unter `asset/` abgelegt.**

## 1 Was in den Bauplan ging

| Größe | Wert | Woher |
|---|---|---|
| Umriss | 50 cm lang, größte Breite 37,0 cm in 18,4 cm Höhe | Abb. 3 ausgelesen; die Breite trifft die Maßzahl 37 der Abbildung |
| Balken 1–7 | Höhe über dem unteren Ende 4,1 / 17,5 / 23,7 / 30,1 (4 und 5) / 36,4 / 39,8 cm | Abb. 3 ausgelesen; die gelesenen Längen treffen Tab. 2 (27,0 gegen 26; 34,8 / 34; 33,9 / 33,7; 9,6–10,0 / 9,8; 21,8 / 22). Balken 6 aus der Lage zwischen 5 und 7 |
| Balkenmaße | Länge a, Höhe 16,5 mm in der Mitte, c am Ende, Breite („Thickness“) 4,2–6,6 mm | Tab. 2 |
| Holz | Sitka: ρ 460, E 10,34 / 0,80 GPa, G 0,63 / 0,66 / 0,03 GPa, ν 0,467; Faser längs | Tab. 1 |
| Rand | eingespannt | wie die Rechnung der Quelle (Schale und Hals fest, Abb. 7) |
| **Deckendicke** | **Annahme, 1,5–4 mm durchgerechnet** | in der Quelle nicht gefunden |
| **Schalllöcher** | **fehlen** (gelesen: Ø 10,5 cm in 30,0 cm Höhe, zwei Ø 3,9 cm in 20,4 cm Höhe, ±10,3 cm) | `InstrumentBody` kennt keine freien Innenränder |
| **Steg** | fehlt; als Gegenprobe 13 cm × 12 mm × 8 mm und 18 cm × 15 mm × 10 mm in 9 cm Höhe | Maße in der Quelle nicht angegeben |

## 2 Ergebnis

Gemessen (Tab. 3): **142 / 226 / 242 / 280 Hz**. Rechnung der Quelle mit Balken 140 / 210 / 239 / 303 Hz, ohne Balken
117 / 182 / 204 / 275 Hz.

| Decke | mit Balken, ohne Steg: erste Moden (Hz) | Abstand zu 142 / 226 / 242 / 280 |
|---|---|---|
| 1,5 mm | 138 / 162 / 260 / 279 | −3 % / −28 % / +7 % / 0 % |
| 1,75 mm | 143 / 178 / 290 / 306 | +1 % / −21 % / +20 % / +9 % |
| 2,0 mm | 147 / 194 / 319 / 330 | +4 % / −14 % / +32 % / +18 % |
| 2,5 mm | 154 / 225 / 368 / 371 | +8 % / 0 % / +52 % / +33 % |
| 3,0 mm | 163 / 255 / 398 / 420 | +15 % / +13 % / +64 % / +50 % |

Ohne Balken: 2 mm 54 / 90 / 125 / 146 Hz; 4 mm 107 / 178 / 248 / 286 Hz (die Quelle: 117 / 182 / 204 / 275).
Mit Steg (13 cm × 12 mm × 8 mm, 1,75 mm Decke): 139 / 172 / 276 / 301 — der Steg senkt alles um 2–5 %, er hebt die
zweite Mode nicht.

**Keine Deckendicke trifft alle vier Moden innerhalb von 10 %.** Die erste Mode passt bei 1,5–2,5 mm, die zweite nur
bei 2,5 mm, die dritte und vierte nur bei 1,5 mm.

## 3 Woran es liegen kann

1. **Die Reihenfolge der Formen ist anders.** In der Rechnung der Quelle ist die zweite Mode (210 Hz) ein einzelner
   Bauch im unteren Feld (links und rechts gleich), die dritte (239 Hz) zwei Bäuche nebeneinander. Hier strahlt die
   zweite Mode kaum (|G| 0,004 gegen 0,10 der ersten) — sie ist die mit zwei Bäuchen nebeneinander, und die nächste
   gleichseitige liegt erst bei 260–320 Hz. Ein Fehler in der Dicke verschiebt alle Moden gemeinsam und tauscht keine.
2. **Die Schalllöcher fehlen.** In den Bildern der Quelle sitzt die Bewegung der ersten Mode — mit wie ohne Balken —
   am großen Loch; ohne Balken liegt ihr Bauch dort und nicht in der breitesten Stelle der Decke. Ein freier Rand von
   10,5 cm Durchmesser macht die Mitte weich. Das ist eine einzelne benannte Annahme, und sie ist nicht nachgerechnet:
   `InstrumentBody` kann sie heute nicht rechnen.
3. **Die Quelle rechnet einen ganzen Körper** (Decke an einer festgehaltenen Schale, mit Steg und Hals); ihre
   Platte ohne Balken verhält sich wie eine von rund 4 mm, die Balken heben ihre erste Mode nur um 20 % (hier bei 4 mm
   um 70 %). Dicke und Verbindung von Balken und Decke stehen nicht im Text.
4. **Die Messung** stammt von einer bespielbaren Oud mit Luftraum und Saitenzug; die Zuordnung der vier gemessenen
   Spitzen zu Formen macht die Quelle nur über die Frequenz. Sie nennt ihre eigene Rechnung „moderate agreement“ (7 und
   8 % bei Mode 2 und 4).

## 4 Befund

Der Weg rechnet — ein Bauplan aus der Abbildung gibt in unter einer Sekunde Moden, Werte am Steg und
Abstrahlgewichte, und die erste Mode liegt bei jeder plausiblen Dicke nahe der gemessenen. Die Schwelle aus KQ6 a
(alle vier innerhalb 10 %) ist **verfehlt**. Ob das an der einen Annahme „keine Löcher“ liegt, lässt sich erst sagen,
wenn `InstrumentBody` freie Innenränder rechnet. Nach KQ6 a kommt der Weg damit erneut zur Vorlage.
