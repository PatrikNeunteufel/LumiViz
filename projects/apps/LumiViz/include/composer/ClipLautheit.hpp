/**
 * @file ClipLautheit.hpp
 * @brief Composer: die Lautheit eines Ton-Clips messen und gemessene Songs
 *        merken (Composer_Konzept §9g, Entscheide Patrik L3b a, G5-3 a)
 *
 * Gemessen wird, was der Clip spielt, ohne Pegelkurve (L3b a): bei einem Song
 * der Ausschnitt ab `einsatzMs`, stueckweise dekodiert; bei einem Loop ein
 * Durchlauf, nachdem einer vorgerechnet und verworfen ist (damit Echo und Hall
 * des vorigen mitklingen). Das Messen selbst darf in einem eigenen Thread
 * laufen; das Vorbereiten eines Loops nicht (es laedt Klaenge ueber die Engine
 * und liest, was das Loop-Panel offen hat).
 */
#pragma once

#include "composer/Lautheit.hpp"

#include <QString>

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>

class IAudioEngine;

namespace lumi::loop
{
struct Programm;
}

namespace lumi::composer
{

/// Meldet den Fortschritt einer Messung, 0…1; false = abbrechen.
using MessFortschritt = std::function<bool(double anteil)>;

/// Einen Song messen: `dauerMs` ab `einsatzMs` (0 = bis zum Ende), in `rate`.
/// nullopt = nicht lesbar oder abgebrochen. Darf in einem eigenen Thread laufen.
[[nodiscard]] std::optional<Lautheit> messeSong(IAudioEngine& engine, const QString& datei, std::int64_t einsatzMs,
                                                std::int64_t dauerMs, int rate, const MessFortschritt& fortschritt);

/// Einen Loop zum Messen vorbereiten: die Fassung, die das Loop-Panel offen
/// hat, sonst die Datei; mit seinen Klaengen in `rate`. nullptr = kein Loop
/// oder nichts zu spielen. **Nur im Thread der Oberflaeche.**
[[nodiscard]] std::shared_ptr<const loop::Programm> bereiteLoopVor(const QString& datei, IAudioEngine& engine,
                                                                   int rate);

/// Einen vorbereiteten Loop messen: zwei Durchlaeufe, gemessen der zweite.
/// Ohne Engine; darf in einem eigenen Thread laufen. Ein Programm aus
/// `bereiteLoopVor` misst **einmal** — das Gedaechtnis seiner Effekte gehoert
/// dieser Messung; fuer eine neue Messung neu vorbereiten.
[[nodiscard]] std::optional<Lautheit> messeLoop(const std::shared_ptr<const loop::Programm>& programm, int rate,
                                                const MessFortschritt& fortschritt);

/// Gemessene Songs, gemerkt in einer Datei (G5-3 a) — auch ueber einen
/// Neustart. Ein Eintrag gilt, solange Groesse und Aenderungszeit der Datei
/// gleich sind; Loops werden nie gemerkt. Nur im Thread der Oberflaeche.
class SongLautheiten
{
public:
    /// Die Datei im Cache-Ordner des Benutzers.
    [[nodiscard]] static QString standardDatei();
    /// Fuer Tests eine andere Datei; leer = wieder die Vorgabe.
    static void setzeDateiFuerTests(const QString& datei);

    SongLautheiten();
    explicit SongLautheiten(QString datei);

    [[nodiscard]] std::optional<Lautheit> hole(const QString& song, std::int64_t einsatzMs, std::int64_t dauerMs,
                                               int rate) const;
    void merke(const QString& song, std::int64_t einsatzMs, std::int64_t dauerMs, int rate, const Lautheit& wert);
    /// Die Eintraege zu diesem Song vergessen (Measure again).
    void vergiss(const QString& song);
    /// Schreiben, wenn sich etwas geaendert hat. false = nicht zu schreiben.
    bool schreibe();
    [[nodiscard]] int anzahl() const { return static_cast<int>(m_werte.size()); }

private:
    [[nodiscard]] static QString schluessel(const QString& song, std::int64_t einsatzMs, std::int64_t dauerMs,
                                            int rate);
    QString m_datei;
    std::map<QString, Lautheit> m_werte;
    bool m_geaendert = false;
};

}  // namespace lumi::composer
