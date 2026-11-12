# Probe 34k-5: der statistische Teil ohne Bauplan

> **Sitzung:** S195 (2026-11-12). Frage aus GZ4 a („Nicht nachgerechnet: ob der statistische Teil ohne Bauplan Moden
> liefert — erste Probe in 34k-5“). Programm [`proben/2026-11-12_S195_statistik_ohne_bauplan.py`](proben/2026-11-12_S195_statistik_ohne_bauplan.py),
> Rohausgabe [`…txt`](proben/2026-11-12_S195_statistik_ohne_bauplan.txt). Liest nur die drei mitgelieferten
> Flügelkörper und ihre Baupläne; nichts am Programm geändert.

## 1 Am Code

`schaetze` (`InstrumentBody/Statistik.hpp`) nimmt alle drei Größen des statistischen Teils — Modendichte, Streuung der
Formwerte je Stegpunkt, Betrag des Abstrahlgewichts je Hörort — **aus gerechneten Moden** im oberen Band und gibt bei
weniger als zehn Moden einen leeren Teil zurück; `ziehe` zieht aus einem leeren Teil nichts. **Ohne Bauplan gibt es
keine gerechneten Moden, also keine statistischen.** `ziehe` selbst braucht keinen Bauplan: ein von Hand gefüllter
`StatistischerTeil` würde Moden liefern. Die Frage ist deshalb, ob sich seine drei Größen aus Plattenwerten
vorhersagen lassen.

## 2 Gemessen: Plattenformeln gegen gerechnete Moden

Vorhersage aus Plattenwerten: Modendichte der dünnen orthotropen Platte n = (A / 2) · √(ρh / √(Dx·Dy)), Formwert am
Stegpunkt im quadratischen Mittel 1 / √M. Verglichen mit den gerechneten Moden je Band.

| Körper | Band (Hz) | Dichte gerechnet / Formel | Formwert gerechnet / (1/√M ganz) | Formwert je Stegpunkt, kleinster … größter |
|---|---|---|---|---|
| Steinway D (Fichte, Rippen, Steg) | 0–250 | 0,30 | 0,75 | 0,036 … 0,296 |
| | 250–500 | 0,50 | 0,59 | 0,070 … 0,275 |
| | 500–1000 | 0,60 | 0,50 | 0,087 … 0,255 |
| | 1000–2500 | 0,88 | 0,39 | 0,064 … 0,259 |
| | 2500–5000 | 1,32 | 0,30 | 0,059 … 0,158 |
| Glasboden, gleiche Dicke | 0–250 | 0,46 | 1,12 | 0,001 … 0,217 |
| | 500–1000 | 0,86 | 0,82 | 0,016 … 0,183 |
| | 2500–5000 | 1,09 | 0,80 | 0,085 … 0,193 |
| Glasboden, gleiche Masse | 0–250 | 0,33 | 0,54 | 0,050 … 0,223 |
| | 500–1000 | 0,72 | 0,31 | 0,043 … 0,179 |
| | 2500–5000 | 0,84 | 0,22 | 0,043 … 0,113 |

Für das Abstrahlgewicht gibt es keine Formel aus Plattenwerten; gerechnet fällt sein Betrag vom untersten zum obersten
Band um den Faktor 8 bis 12.

## 3 Befund

1. **Die Modendichte** trifft die Formel oberhalb von 2,5 kHz auf −16 % bis +32 %. Unter 1 kHz liegt sie um den
   Faktor 1,2 bis 3,3 zu hoch (der Rand nimmt Moden weg) — gerade dort, wo wenige einzelne Moden den Steg bestimmen.
2. **Der Formwert am Stegpunkt** liegt um den Faktor 1,3 bis 4,5 unter 1 / √M (der Steg sitzt auf einer schweren,
   steifen Leiste), und er hängt vom Ort ab: zwischen den Stegpunkten liegt im selben Band der Faktor 2 bis 8, im
   untersten Band des Glasbodens über 200. Aus Plattenwerten folgt **ein** Wert für alle Tasten.
3. **Das Abstrahlgewicht** lässt sich aus Plattenwerten nicht angeben.
4. Die Sprünge der Lautheit, um die es in GZ2 a geht, entstehen unter rund 1 kHz an einzelnen Stegmoden
   (Gegenprobe S193). Genau dieser Bereich ist nicht statistisch: er braucht einzelne Moden mit ihrem Wert am Ort der
   Saite.

**Also:** Der statistische Teil liefert ohne Bauplan keine Moden, und Plattenwerte allein geben nur die Größenordnung
— gut genug für den Teil oberhalb der gerechneten Moden, nicht für den Bereich, der die Zielgrößen trägt. Der
Haltepunkt 34k-5 greift („Bauplan je Instrument, ein gemeinsames Plattenmodell, oder Kipppunkt GZ4 a“).

## 4 Einschränkung

Die Probe steht auf drei Fassungen **eines** Bodens (Flügel, 2 m², mit Rippen und Steg). Für kleine, dünne Decken
(Laute, Oud, Gitarre; 0,1 m², 1,5–3 mm) ist nichts gerechnet. Die Richtung der Abweichungen (Rand nimmt tiefe Moden
weg, der Steg senkt den Formwert) gilt dort ebenso; die Faktoren sind nicht übertragbar.
