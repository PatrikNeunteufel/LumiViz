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
 * Kein Qt-Objekt, kein Zeitgeber: wer ihn haelt, ruft `takt`. Laeuft gegen
 * `IAudioEngine` und `IAudioPlayer` — im Test gegen beide aus Pappe.
 *
 * Seit Stufe 29d (Entscheide H2 a, TV5 a, TV6 a) laedt er seine Klaenge ueber
 * einen `Tonauftrag`: liegt alles im Tonspeicher, spielt er sofort; sonst
 * rechnet der Auftrag im Hintergrund. `starte` uebernimmt dann schon den Ton,
 * spielt aber still (`wartet`), bis die Toene da sind — `takt` schaltet um und
 * beginnt von vorn (oder an der Stelle, zu der inzwischen gesprungen wurde).
 * Aendert sich der Loop waehrend er spielt, klingt das bisherige Programm, bis
 * die neuen Toene fertig sind (`bereitet`). `ladeVor` rechnet die Toene eines
 * geoeffneten Loops vorab, ohne zu spielen; Play uebernimmt einen solchen
 * Auftrag. Seine Threads enden mit dem Spieler — er stirbt vor der Engine
 * (Bootstrap_Integration.md §6.7).
 */
#pragma once

#include "audio/IAudioEngine.hpp"
#include "loop/Klangerzeuger.hpp"
#include "loop/Loop.hpp"
#include "loop/Tonauftrag.hpp"

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
    /// war. Spielt er schon, beginnt er von vorn. Fehlen Toene, spielt er still,
    /// bis sie da sind (`wartet`).
    bool starte(IAudioEngine& engine, IAudioPlayer& player);
    /// Die Toene des gesetzten Loops vorab rechnen, ohne zu spielen (H1 a: beim
    /// Oeffnen eines Loops). Spielt er schon, tut es nichts.
    void ladeVor(IAudioEngine& engine);
    /// Er hat den Ton, aber noch kein Programm: die Toene werden gerechnet (TV5 a, A).
    [[nodiscard]] bool wartet() const { return spielt() && m_wartet; }
    /// Ein Auftrag rechnet fuer das Spielen — beim Warten oder nach einer Bearbeitung
    /// (TV5 a, A und C). Nicht beim Vorladen.
    [[nodiscard]] bool bereitet() const { return spielt() && m_auftrag != nullptr; }
    /// Rechnet ein Auftrag aus `ladeVor`?
    [[nodiscard]] bool laedtVor() const { return m_vorab != nullptr; }
    /// Wie weit der Auftrag ist: fertige und alle Klaenge; 0 und 0 ohne Auftrag.
    [[nodiscard]] int vorbereitet() const { return m_auftrag != nullptr ? m_auftrag->erledigt() : 0; }
    [[nodiscard]] int vorzubereiten() const { return m_auftrag != nullptr ? m_auftrag->gesamt() : 0; }
    /// Warten, bis der laufende Auftrag fertig ist, und umschalten — blockiert. Fuer
    /// Tests und Messungen.
    void warteAufToene();
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
    /// An diese Stelle des Durchlaufs springen, in Schritten (Entscheid Patrik
    /// B4, S124). Echo und Hall vergessen dabei, was sie gehoert haben
    /// (`kSprungMs`). false = er spielt nicht.
    bool springe(double schritt);
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
    /// Was ein Auftrag laedt — und wozu ein fertiger Satz Klaenge gehoert.
    struct Satz
    {
        QStringList namen;
        int rate = 0;
        std::uint64_t fassung = 0;  ///< `instrumentFassung()` beim Anstossen
        bool operator==(const Satz&) const = default;
    };
    /// `vonVorn`: er beginnt gerade — fehlen Toene, spielt er still bis dahin.
    void ladeProgramm(bool vonVorn);
    /// Den Satz, den der Loop jetzt braucht.
    [[nodiscard]] Satz satzJetzt(int rate) const;
    /// Einen fertigen Auftrag uebernehmen: seine Klaenge, sein Bericht.
    void nimmKlaenge(Tonauftrag& auftrag, const Satz& satz);
    /// Ist der Auftrag fertig: umschalten (und beim Warten von vorn beginnen).
    void schalteUm();
    /// Einen Auftrag abbrechen; er ruht, sobald sein letzter Name geladen ist.
    void verwirf(std::unique_ptr<Tonauftrag>& auftrag);
    void raeumeAuf();

    Vorrang m_vorrang;
    Loop m_loop;
    Klangerzeuger m_erzeuger;  ///< lebt laenger als seine Quelle im Mixer
    std::shared_ptr<const Klaenge> m_klaenge;
    Satz m_satz;               ///< wozu `m_klaenge` gehoert
    QStringList m_bericht;
    std::unique_ptr<Tonauftrag> m_auftrag;  ///< rechnet fuer das Spielen
    Satz m_auftragSatz;
    std::unique_ptr<Tonauftrag> m_vorab;    ///< rechnet vorab (`ladeVor`)
    Satz m_vorabSatz;
    std::vector<std::unique_ptr<Tonauftrag>> m_verworfen;  ///< abgebrochen, ihre Threads laufen noch aus
    bool m_wartet = false;
    double m_startSchritt = 0.0;  ///< wohin er beim Umschalten nach dem Warten springt

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
