/**
 * @file Tonhoehenspeicher.hpp
 * @brief Loop: den Tonhoehenverlauf einer Datei einmal messen und merken (docs/audio/Loop_Konzept.md
 *        §4p, WV8 a, Stufe 15g)
 *
 * pYIN braucht die ganze Datei (`messeTonhoehenverlauf`). Gemessen wird je Datei einmal, gemerkt
 * unter dem **Abdruck ihres Inhalts** (SHA-1 der Bytes und der Fassung der Messung) — im
 * Arbeitsspeicher und als kleine Datei im Ordner `pitch` des Benutzerbereichs, neben `frozen`.
 * Ein Speicher, kein Werk: jederzeit neu zu messen, nicht in der Datensicherung. Eine
 * umbenannte Datei hat denselben Abdruck, eine geaenderte einen neuen.
 *
 * Gemessen wird in der Rate, die der Aufrufer verlangt und in der der Dekodierer liefert; der
 * Verlauf nennt seine Rate und seinen Schritt, seine Zeiten haengen nicht an der Rate.
 * Thread-sicher.
 */
#pragma once

#include "loop/PYin.hpp"
#include "loop/Tonauftrag.hpp"

#include <QByteArray>
#include <QString>

#include <optional>

namespace lumi::loop
{

/// Die Fassung der Messung: aendert sich die Rechnung, kommt eine neue — alte Dateien gelten nicht mehr.
inline constexpr int kTonhoehenFassung = 1;

/// Der Ordner `pitch` im Benutzerbereich; leer = keiner.
[[nodiscard]] QString tonhoehenOrdner();
/// Nur fuer Tests: ein Wegwerf-Ordner statt des Benutzerbereichs; leer = wieder der echte.
void setzeTonhoehenOrdnerFuerTests(const QString& ordner);

/// Der Abdruck einer Datei: SHA-1 ueber `kTonhoehenFassung` und ihren Inhalt, 40 Hex-Zeichen.
/// Leer = nicht zu lesen.
[[nodiscard]] QByteArray tonhoehenAbdruck(const QString& pfad);

/// Den Verlauf einer Datei holen: aus dem Arbeitsspeicher, aus `pitch/<abdruck>.lvpitch` oder neu
/// gemessen (mit `dekodierer` in `rate`, dann gemerkt). Leer, wenn die Datei nicht zu lesen ist.
[[nodiscard]] std::optional<Tonhoehenverlauf> tonhoehenverlaufVon(const QString& pfad, const Dekodierer& dekodierer, int rate);

/// Die Datei eines Verlaufs schreiben und lesen (fuer Tests auch einzeln): Kopf `LVPITCH`, Fassung,
/// Rate, Schritt, Zahl der Rahmen, dann je Rahmen hz und stimmhaft als float.
bool schreibeTonhoehenverlauf(const QString& datei, const Tonhoehenverlauf& verlauf);
[[nodiscard]] std::optional<Tonhoehenverlauf> liesTonhoehenverlauf(const QString& datei);

/// Den Arbeitsspeicher leeren (Tests); die Dateien bleiben.
void vergissTonhoehenverlaeufe();

}  // namespace lumi::loop
