# Eigenzerlegung — Eigenwerte kleiner komplexer Matrizen

> **Version:** 0.1.0  
> **Datum:** 2026-10-25  
> **Typ:** CppModuleDoc  
> **Status:** gebaut (Stufe 31d-7 A, Punkt 2)  
> **Modul:** lumi::loop (App LumiViz)  
> **Dateien:** `include/loop/Eigenzerlegung.hpp`, `src/loop/Eigenzerlegung.cpp`  
> **Namespace:** lumi::loop  
> **Abhängigkeiten:** nur die Standardbibliothek; kein Qt, kein `windows.h` (IK1)  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## 1. Zweck

Die Moden der Hammer-Saite je Teilton (Stufe 31d-7, Entscheide P2 a, P6 a in
`docs/audio/Spielweisen_Konzept.md` §4.1): N ≤ 3 Saiten mit je zwei Schwingungsebenen geben eine allgemein
komplexe Matrix bis 6 × 6. Der frühere Kunstgriff für eine Störung vom Rang eins (`chorModen`, Durand–Kerner)
trägt nicht mehr, sobald der Steg in zwei Richtungen verschieden nimmt; Eigen ist nur an die Tests gebunden.

## 2. Schnittstelle

- `KleineMatrix` — n × n, zeilenweise, `a(zeile, spalte)`; n bis `kKleineMatrixMax` (16).
- `Eigenzerlegung zerlegeKlein(const KleineMatrix&)` — `werte` (λ_m), `rechts` (Spalte m: Eigenvektor, Länge 1),
  `links` (Zeile m, `links · rechts` = Einheit), also A = rechts · diag(λ) · links. `vollstaendig` ist falsch,
  wenn die QR-Iteration nicht zusammenlief oder `rechts` nicht umkehrbar war.

Für eine Antwort c·e^(At)·b zählt je Mode (c·v_m)(w_m·b) — so rechnet die Hammer-Saite die Anteile ihrer Moden.

## 3. Verfahren

1. Ist die Matrix schon diagonal, werden Werte und Einheit unverändert übernommen (Bit für Bit — darauf baut die
   Zusage, dass eine Saite mit γ = 0 wie vorher rechnet).
2. Householder auf Hessenberg-Form, dann QR mit Verschiebung nach Wilkinson (alle zehn Schritte eine
   Ausnahmeverschiebung) bis zur Schur-Form T, die Drehungen gesammelt.
3. Eigenvektoren von T rückwärts eingesetzt (ein Nenner unter ε·‖A‖ wird ε·‖A‖), zurückgedreht, auf Länge 1;
   ein isolierter Eigenwert (Abstand über 10⁻⁸·‖A‖) bekommt einen Schritt inverser Iteration.
4. `links` als Inverse von `rechts` mit Pivotsuche.

## 4. Tests

`tests/unit/UnitTests/test_LoopKleinEigen.cpp`: diagonal Bit für Bit; 160 zufällige Matrizen 1 … 8 gegen Eigen
(Eigenwerte und Residuum auf 10⁻¹²·‖A‖, `links · rechts` auf 10⁻¹⁰); der Chor in einer Ebene gegen `chorModen`
(Eigenwerte 10⁻¹²·‖A‖, Anteile 10⁻⁹); zwei Ebenen mit zusammenfallenden und eng verstimmten Eigenwerten
(Übertragung aus den Moden gegen direkt gelöst auf 10⁻⁹, Projektoren isolierter Eigenwerte gegen Eigen auf 10⁻⁹).

## 5. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 0.1.0 | 2026-10-25 | Session 153. Angelegt (31d-7 A, Punkt 2; P6 a). |
