/**
 * @file Zeitspur.hpp
 * @brief Loop: die Spuren ueber der Zeit — Pedale, Expression, Vibrato, Pitch
 *        und die Schalter der Klang-Spielweisen (docs/audio/Spielweisen_Konzept.md,
 *        Stufe 28a; Entscheide Patrik S139/S140: SF5 d, SK1 a bis SK8 a)
 *
 * Ein Instrument-Muster traegt fuenf **Steuer-Spuren** (SF3 a, SF5 d) und die
 * **Schalter** seiner Klang-Spielweisen (SF8 a) — die Regler eines Keyboards
 * ueber der Zeit, in Schritten des Musters. Ein Clip traegt dazu einen Verlauf
 * **Expression** in Takten ab seinem Anfang (SK8 a), mit der Spur des Musters
 * malgenommen.
 *
 * Eine leere Spur ist die Vorgabe des Instruments: kein Pedal, Expression 1,
 * Vibrato und Pitch von der Schicht. Hier steht nur das Modell — wie es klingt,
 * rechnen die Stufen 28b und 28c. Kein Qt-Widget, keine Audio-Engine.
 *
 * `Spur` heisst im Loop schon die Spur eines Drumkits; daher `Zeitspur`.
 */
#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QLatin1String>
#include <QString>

#include <array>
#include <vector>

namespace lumi::loop
{

/// Ein Punkt einer Spur: an einer Stelle ein Wert. Die Stelle zaehlt im Muster
/// in Schritten, am Clip in Takten, jeweils mit Bruchteil.
struct SpurPunkt
{
    double stelle = 0.0;
    double wert = 0.0;

    [[nodiscard]] bool operator==(const SpurPunkt&) const = default;
};

/// So viele Punkte traegt eine Spur hoechstens (von Claude gesetzt: eine Grenze
/// fuer die Datei — ein Muster von 64 Takten mit 16 Punkten je Takt braucht 1024).
inline constexpr int kZeitspurPunkteMax = 4096;

/// Die fuenf Steuer-Spielweisen (SF3 a), in der Reihenfolge der Spuren im Editor
/// (Skizze §1a).
enum class Steuerspur : int
{
    Sustain = 0,    ///< Pedal: Noten klingen weiter, bis es losgelassen wird
    Sostenuto = 1,  ///< Pedal: nur die beim Treten klingenden Noten werden gehalten
    Expression = 2, ///< Staerke ueber der Zeit, 0 … 1 (SK4 a)
    Vibrato = 3,    ///< Tiefe ueber der Zeit, in Cent (SF4 a)
    Pitch = 4,      ///< Tonhoehe ueber der Zeit, in Halbtoenen (SK3 a)
};
inline constexpr int kSteuerspuren = 5;
inline constexpr std::array<Steuerspur, kSteuerspuren> kAlleSteuerspuren{
    Steuerspur::Sustain, Steuerspur::Sostenuto, Steuerspur::Expression, Steuerspur::Vibrato, Steuerspur::Pitch};

/// Die Grenzen der Werte: Pedale und Expression 0 … 1, Vibrato in Cent wie
/// `vibratoDepth` einer Schicht, Pitch so weit, wie die groesste *Pitch range*
/// reicht (SK3 a: die Grenze des Instruments gilt erst beim Rechnen).
inline constexpr double kVibratoCentMax = 100.0;
inline constexpr double kPitchHalbtoeneMax = 24.0;
[[nodiscard]] double spurMin(Steuerspur spur);
[[nodiscard]] double spurMax(Steuerspur spur);
/// Stufen (die Pedale): der Wert gilt bis zum naechsten Punkt. Sonst eine Kurve,
/// linear zwischen den Punkten.
[[nodiscard]] bool istStufenspur(Steuerspur spur);
/// Wie die Spur in einer Datei heisst: `sustain`, `sostenuto`, `expression`,
/// `vibrato`, `pitch`.
[[nodiscard]] QLatin1String steuerspurName(Steuerspur spur);

/// Ein Schalter einer Klang-Spielweise ueber der Zeit (SF8 a), etwa *Una corda*:
/// an (1) oder aus (0) ab jeder Stelle. Beim Namen genannt; welche es gibt,
/// bringen die Bauarten (ab der Hammer-Saite).
struct SchalterSpur
{
    QString art;  ///< der Name in der Datei, etwa `unaCorda`
    std::vector<SpurPunkt> punkte;

    [[nodiscard]] bool operator==(const SchalterSpur&) const = default;
};

/// Die Spuren eines Instrument-Musters.
struct Zeitspuren
{
    std::array<std::vector<SpurPunkt>, kSteuerspuren> steuer{};
    std::vector<SchalterSpur> schalter;

    [[nodiscard]] std::vector<SpurPunkt>& von(Steuerspur spur) { return steuer[static_cast<std::size_t>(spur)]; }
    [[nodiscard]] const std::vector<SpurPunkt>& von(Steuerspur spur) const
    {
        return steuer[static_cast<std::size_t>(spur)];
    }
    /// Traegt keine Spur einen Punkt?
    [[nodiscard]] bool leer() const;

    [[nodiscard]] bool operator==(const Zeitspuren&) const = default;
};

/// Punkte in ihre Grenzen: Werte auf `min` … `max`; nach der Stelle sortiert, je
/// Stelle ein Punkt (der spaetere gilt, wie beim Bend); hoechstens
/// `kZeitspurPunkteMax`. Mit `ende` > 0 (im Muster) liegen die Stellen in
/// 0 … `ende`, Punkte ab dem Ende gehen, wie Noten. Am Clip (`ende` = 0) bleiben
/// Punkte vor dem Anfang und hinter dem Ende stehen (SK8 a) — sie sind nur nicht
/// zu hoeren.
void richtePunkte(std::vector<SpurPunkt>& punkte, double min, double max, double ende = 0.0);
/// Alle Spuren eines Musters von `schritte` Schritten richten; Schalter ohne
/// Namen gehen, je Name bleibt der erste, sortiert nach dem Namen (wie in der Datei).
void richteZeitspuren(Zeitspuren& spuren, double schritte);

/// Der Wert einer Stufenspur an `stelle`: der des letzten Punkts davor (oder an
/// ihr); vor dem ersten `vorher`. Ohne Punkte `vorher`.
[[nodiscard]] double stufeBei(const std::vector<SpurPunkt>& punkte, double stelle, double vorher = 0.0);
/// Der Wert einer Kurve an `stelle`, linear zwischen den Punkten; vor dem ersten
/// der erste, nach dem letzten der letzte. Ohne Punkte `vorgabe`.
[[nodiscard]] double kurveBei(const std::vector<SpurPunkt>& punkte, double stelle, double vorgabe);

/// In einer Datei je Punkt `[stelle, wert]`. Gelesen wird ungerichtet — wer
/// liest, richtet.
[[nodiscard]] QJsonArray punkteAlsJson(const std::vector<SpurPunkt>& punkte);
[[nodiscard]] std::vector<SpurPunkt> punkteAusJson(const QJsonArray& werte);
/// Die Spuren eines Musters in der `.lvloop` (seit Fassung 12, immer geschrieben):
/// je Steuer-Spur ihr Name, dazu `switches` mit je Schalter seinem Namen.
[[nodiscard]] QJsonObject zeitspurenAlsJson(const Zeitspuren& spuren);
[[nodiscard]] Zeitspuren zeitspurenAusJson(const QJsonObject& o);

}  // namespace lumi::loop
