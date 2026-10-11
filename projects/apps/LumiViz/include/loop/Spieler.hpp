/**
 * @file Spieler.hpp
 * @brief Loop: ein Loop spielt allein, ohne Composer
 *        (docs/audio/Loop_Konzept.md, Entscheide L4 a und L7 a, Stufe 5b)
 *
 * Der Spieler uebernimmt den Ton fuer das Loop-Panel: ein eigener Mixer in der
 * Audio-Engine, darin der `Klangerzeuger`. Wer spielt, hat den Ton —
 * **Rangfolge Composer, Loop-Panel, Player** (Entscheid Patrik S108):
 *
 * - Fuehrt der Composer den Ton, spielt der Spieler nicht: `starte` lehnt ab,
 *   und ein laufender haelt im naechsten Takt von selbst an.
 * - Solange der Spieler spielt, schweigt der Player. Beim Anhalten spielt er
 *   weiter, wenn er vorher spielte.
 * - Lautstaerke und Mute des Players gelten fuer den Mixer des Loops (wie
 *   beim Composer).
 *
 * Kein Qt-Objekt, kein Thread, kein Zeitgeber: wer ihn haelt, ruft `takt`.
 * Laeuft gegen `IAudioEngine` und `IAudioPlayer` — im Test gegen beide aus
 * Pappe.
 */
#pragma once

#include "audio/IAudioEngine.hpp"
#include "loop/Klangerzeuger.hpp"
#include "loop/Loop.hpp"

#include <QStringList>

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

class IAudioPlayer;

namespace lumi::loop
{

class Spieler
{
public:
    /// Hat ein Hoeherer den Ton (der Composer)? Leer = es gibt keinen.
    using Vorrang = std::function<bool()>;

    explicit Spieler(Vorrang vorrang = {});
    ~Spieler();
    Spieler(const Spieler&) = delete;
    Spieler& operator=(const Spieler&) = delete;

    /// Den Loop setzen, der gespielt wird (er wird gerichtet). Spielt der
    /// Spieler, gilt er ab dem naechsten Block; die Klaenge werden nur neu
    /// geladen, wenn sich die Klaenge der Spuren geaendert haben.
    void setzeLoop(const Loop& loop);
    [[nodiscard]] const Loop& loop() const { return m_loop; }

    /// Den Ton uebernehmen und von vorn spielen. false = der Composer hat
    /// den Ton, oder die Engine hat keinen Mixer — dann bleibt alles, wie es
    /// war. Spielt er schon, beginnt er von vorn.
    bool starte(IAudioEngine& engine, IAudioPlayer& player);
    /// Anhalten und den Ton freigeben. Mit `playerWeiter` spielt der Player
    /// weiter, wenn er beim Uebernehmen spielte.
    void halte(bool playerWeiter = true);
    [[nodiscard]] bool spielt() const { return m_engine != nullptr; }
    /// Darf er gerade spielen — oder hat ein Hoeherer den Ton?
    [[nodiscard]] bool darfSpielen() const { return !(m_vorrang && m_vorrang()); }

    /// Ein Takt (rund 30-mal je Sekunde): haelt an, wenn der Composer den Ton
    /// uebernommen hat; haelt den Player still; fuehrt die Lautstaerke nach.
    void takt();

    /// Der Mixer des Loops — Spektrum und Wellenform fuer die Visuals.
    /// INVALID_STREAM = er spielt nicht.
    [[nodiscard]] AudioStreamHandle mixer() const { return m_mixer; }
    /// Die Stelle im laufenden Durchlauf, in Schritten (mit Bruchteil); 0,
    /// wenn er nicht spielt.
    [[nodiscard]] double stelleSchritte() const;
    /// Der groesste Betrag der Summe seit dem letzten Aufruf — vor der
    /// Lautstaerke des Players. Ueber 1 uebersteuert der Loop (L6 a).
    [[nodiscard]] float nimmSpitze() { return m_erzeuger.nimmSpitze(); }
    /// Was beim letzten Laden der Klaenge auffiel (`Sound not loaded: …`).
    [[nodiscard]] const QStringList& bericht() const { return m_bericht; }

    /// Der Spieler, der gerade spielt — es gibt nur einen Ton, also
    /// hoechstens einen. nullptr = keiner.
    [[nodiscard]] static Spieler* spielender();

private:
    void ladeProgramm();

    Vorrang m_vorrang;
    Loop m_loop;
    Klangerzeuger m_erzeuger;  ///< lebt laenger als seine Quelle im Mixer
    std::shared_ptr<const std::vector<Klang>> m_klaenge;
    QStringList m_klangNamen;  ///< die Klaenge der Spuren, zu denen `m_klaenge` gehoert
    int m_klangRate = 0;
    std::uint64_t m_instrumentFassung = 0;  ///< `instrumentFassung()` beim Laden der Klaenge
    QStringList m_bericht;

    IAudioEngine* m_engine = nullptr;
    IAudioPlayer* m_player = nullptr;
    AudioStreamHandle m_mixer = INVALID_STREAM;
    AudioStreamHandle m_quelle = INVALID_STREAM;
    bool m_playerSpielte = false;
    float m_laut = -1.0f;  ///< zuletzt am Mixer gesetzt
};

/// Der Mixer des spielenden Loops; INVALID_STREAM = es spielt keiner. Fuer
/// die Stelle, an der die Visuals ihren Ton holen (wie `composer::tonMixer`).
[[nodiscard]] AudioStreamHandle tonMixer();

}  // namespace lumi::loop
