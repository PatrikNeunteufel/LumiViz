/**
 * @file Abgleich.hpp
 * @brief Composer: das Modell des Dialogs Match loudness, ohne Fenster
 *        (Composer_Konzept §9g, Entscheide Patrik L3 a, G5-1 a bis G5-4 a)
 *
 * Je Ton-Clip eine Zeile: was er spielt, sein Gain vor dem Dialog und im
 * Dialog, seine Messung. Daraus der Vorschlag: der Bezugsclip bekommt 0 dB,
 * jeder andere den Abstand seiner Lautheit (oder den Abstand zu Fixed level).
 * Aendert sich die Komposition bei offenem Dialog (etwa Ctrl+Z), liest das
 * Modell die Liste neu und behaelt, was zu Clips mit derselben Datei und
 * demselben Ausschnitt gehoert (G5-2 a) — Clips haben keine eigene Kennung.
 */
#pragma once

#include "composer/Komposition.hpp"
#include "audio/Lautheit.hpp"

#include <QString>

#include <cstdint>
#include <optional>
#include <vector>

namespace lumi::composer
{

/// Fixed level: Vorgabe nach L3 a.
inline constexpr double kAbgleichFestLufs = -14.0;

struct AbgleichZeile
{
    int kanal = -1;
    int clip = -1;
    QString datei;
    bool loop = false;
    std::int64_t einsatzMs = 0;  ///< Song: ab hier im Song; Loop: unbeachtet (ein Durchlauf)
    std::int64_t dauerMs = 0;    ///< Song: so lange spielt der Clip
    QString kanalName;
    std::int32_t farbe = -1;
    bool spielt = true;          ///< Kanal und Clip an — sonst grau, gemessen wird trotzdem
    double gainVorherDb = 0.0;   ///< beim Oeffnen des Dialogs (bzw. als die Zeile dazukam)
    double gainDb = 0.0;         ///< im Dialog
    std::optional<audio::Lautheit> messung;
    bool gemessenFehlgeschlagen = false;  ///< nicht lesbar: keine Werte, kein Vorschlag

    /// Derselbe Clip im Sinn des Neu-Lesens: dieselbe Datei, derselbe Ausschnitt.
    [[nodiscard]] bool gleicherClip(const AbgleichZeile& o) const
    {
        return datei == o.datei && loop == o.loop && einsatzMs == o.einsatzMs && dauerMs == o.dauerMs;
    }
    /// Der Name des Clips: der Dateiname ohne Endung.
    [[nodiscard]] QString name() const;
};

class Abgleich
{
public:
    /// Je Ton-Clip aller Audio-Kanaele eine Zeile, in der Reihenfolge der Kanaele.
    explicit Abgleich(const Komposition& komposition);

    [[nodiscard]] const std::vector<AbgleichZeile>& zeilen() const { return m_zeilen; }
    [[nodiscard]] std::vector<AbgleichZeile>& zeilen() { return m_zeilen; }
    /// Die Zeile eines Clips, -1 = keine.
    [[nodiscard]] int zeileVon(int kanal, int clip) const;

    // --- Ziel (L3 a) ----------------------------------------------------------
    /// Eine Zeile als Ziel; -1 = Fixed level.
    void setzeZiel(int zeile);
    [[nodiscard]] int ziel() const { return m_ziel; }
    void setzeFestLufs(double lufs) { m_festLufs = lufs; }
    [[nodiscard]] double festLufs() const { return m_festLufs; }
    /// Die lauteste gemessene Zeile, -1 = noch keine gemessen.
    [[nodiscard]] int lauteste() const;

    // --- Vorschlag ------------------------------------------------------------
    /// Das vorgeschlagene Gain einer Zeile in dB, geklemmt auf
    /// `kGainMinDb`…`kGainMaxDb`. nullopt = sie oder das Ziel ist nicht
    /// gemessen, oder still.
    [[nodiscard]] std::optional<double> vorschlag(int zeile) const;
    /// Die Spitze, die sie mit dem Vorschlag haette, in dBFS; nullopt = keiner.
    [[nodiscard]] std::optional<double> spitzeMitVorschlag(int zeile) const;
    /// Jede Zeile mit Vorschlag bekommt ihn als Gain (Use suggestions).
    /// Liefert, wie viele sich geaendert haben.
    int uebernimmVorschlaege();

    // --- Komposition ----------------------------------------------------------
    /// `in` mit den Gains des Dialogs (`imDialog`) oder denen von vorher.
    [[nodiscard]] Komposition mitGains(const Komposition& in, bool imDialog) const;
    /// Hat eine Zeile ein anderes Gain als vorher?
    [[nodiscard]] bool geaendert() const;
    /// Die Komposition hat sich geaendert: Liste neu lesen. `neu` traegt keine
    /// Vorschau (sie kommt von aussen, etwa Ctrl+Z): ihr Gain ist das neue
    /// „vorher" jeder Zeile. Zeilen desselben Clips behalten ihre Messung und,
    /// wenn im Dialog geaendert, ihr Gain im Dialog; sonst folgt es `neu`. Das Ziel folgt seinem Clip; fehlt er, gilt
    /// die lauteste. Liefert die Zeilen, die neu gemessen werden muessen.
    std::vector<int> liesNeu(const Komposition& neu);

private:
    static std::vector<AbgleichZeile> lies(const Komposition& komposition);
    std::vector<AbgleichZeile> m_zeilen;
    int m_ziel = -1;
    double m_festLufs = kAbgleichFestLufs;
};

}  // namespace lumi::composer
