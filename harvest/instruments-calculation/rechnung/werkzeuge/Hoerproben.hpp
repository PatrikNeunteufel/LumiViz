/**
 * @file Hoerproben.hpp
 * @brief Werkzeug: die Sammlung der Hoerproben (`hoerproben.cpp`) und das Register fuer weitere Klassen — abgelegt nach
 *        HoerTest_Helper `docs/Konzept.md` 0.3.0 §11/§12 (Sync Hoertest HT-1, HT10–HT14 a; IC-1, IC-2)
 *
 * Je Klasse eine Liste `<wurzel>/listen/<klasse>/`: `liste.json` (§12), `klaenge/<id>.wav` (mono, 48 kHz, 32-bit float;
 * ignoriert) und `klaenge.sha256` (Abdruck jeder Datei und der Befehl, der sie schreibt). Die Achsen jedes Klangs kommen
 * aus seinem Namen: `instrument`, `ton` (erstes Glied, wenn es eine Note ist), `staerke` (`a<dB-Stufe>` oder
 * `e<Staerke×100>`), `spielweise` (der Rest). Ein Name mit `mitschwingen` bzw. `ohne` ist die Fassung `mit` bzw. `ohne`;
 * beide zusammen ergeben einen Punkt `paar`, alle anderen Klaenge einen Punkt `einzel` mit ihrer Frage (HT11 a).
 */
#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace ic::hoer
{

/// Ein abgelegter Klang einer Liste.
struct Klang
{
    std::string id, datei, fassung = "vorgabe";
    std::vector<std::pair<std::string, std::string>> achsen;
    std::string frage;
    double lufsVorher = 0.0;    ///< BS.1770-4 mit Gating, vor dem Angleichen
    double spitzeVorher = 0.0;  ///< vor dem Angleichen
    std::string abdruck;        ///< SHA-256 der WAV-Datei (nach `schreibeListen`)
    std::vector<float> ton;     ///< bis `schreibeListen` (dort angeglichen und geschrieben)
    int rate = 48000;
};

struct Sammlung
{
    std::string wurzel = "hoertest";
    std::string befehl;  ///< so wurden die Klaenge geschrieben (Kopf von `klaenge.sha256`)
    int zahl = 0;
    std::map<std::string, std::vector<Klang>> listen;  ///< je Klasse
    std::vector<std::string> kaputt;                   ///< Toene mit NaN/Inf — nicht abgelegt, gemeldet
    /// Nimmt eine Hoerprobe auf (Lautheit nach BS.1770-4 und Spitze gemessen); geschrieben wird mit `schreibeListen`.
    void lege(const std::string& klasse, const std::string& instrument, const std::string& name, std::vector<float> ton, int rate,
              const std::string& frage);
    /// Je Klasse: alle Klaenge auf einen gemeinsamen Pegel — −23 LUFS, oder so viel tiefer, dass keiner ueber die Spitze
    /// 0,99 kommt (`pegel.lufs` der Liste) —, die WAV-Dateien, `liste.json` und `klaenge.sha256`.
    void schreibeListen();
};

/// Die Achsen und die Fassung aus Instrument und Name (s. Dateikopf); oeffentlich fuer den Test.
void achsenAus(const std::string& instrument, const std::string& name, Klang& k);

/// Weitere Klassen tragen sich mit einer Funktion ein (je Klasse eine Datei `werkzeuge/hoerproben_<nn>.cpp`).
struct Eintrag
{
    int klasse = 0;
    std::function<void(Sammlung&)> lege;
};

std::vector<Eintrag>& weitere();

}  // namespace ic::hoer
