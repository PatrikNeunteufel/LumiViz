/**
 * @file Tonfuehrung.hpp
 * @brief Composer: der Audiokanal fuehrt den Player (docs/ui/Composer_Konzept.md
 *        §4 und §8, Stufe C5)
 *
 * Aus der Stelle der Uhr, der Komposition und dem Stand des Players
 * entscheidet die Fuehrung je Takt, welcher Song wo spielen muss — und wer
 * gerade die Zeit fuehrt (Entscheid Patrik S96):
 *
 * - **Solange ein Song spielt, fuehrt der Ton.** Weicht die Uhr von der Stelle
 *   ab, die zu hoeren ist, wird die UHR nachgestellt — auch nach einem Sprung
 *   im Player (Fortschrittsbalken).
 * - **Springt die Uhr** (Klick ins Lineal, Stop, Schleife), fuehrt sie dieses
 *   eine Mal: der Player wird an die passende Stelle gesetzt.
 * - **In Luecken** — kein Ton-Clip an der Stelle, oder der Song ist vor dem
 *   Ende seines Clips aus — fuehrt die Wanduhr, der Player schweigt.
 *
 * Solange die Fuehrung den Player hat, ruht die Playlist: sie wird am Player
 * ausgehaengt (`setPlaylist(nullptr)`) und beim Freigeben wieder eingehaengt —
 * am Titelende spielt dann nichts von selbst weiter.
 *
 * Kein Qt-Objekt, kein Thread, keine Uhr: wer taktet (die Sitzung, im
 * GUI-Thread), nennt die Stelle und stellt die Uhr nach, wenn das Ergebnis es
 * verlangt. Laeuft gegen `IAudioPlayer` — im Test gegen einen Player aus Pappe.
 *
 * Rueckbau (Konzept §12): Teil von `include/composer` / `src/composer`.
 */
#pragma once

#include "composer/Komposition.hpp"

#include <QString>

#include <cstdint>
#include <functional>

class IAudioPlayer;
class IPlaylist;

namespace lumi::composer
{

class Tonfuehrung
{
public:
    /// So weit duerfen Uhr und Ton auseinanderliegen, bevor nachgestellt wird.
    static constexpr std::int64_t kToleranzMs = 40;
    /// Ein Schritt zwischen zwei Takten, der groesser ist, gilt als Sprung.
    static constexpr std::int64_t kSprungMs = 1500;
    /// „Previous" innerhalb der ersten Sekunden eines Songs geht zum Song davor.
    static constexpr std::int64_t kZurueckMs = 3000;

    Tonfuehrung() = default;
    ~Tonfuehrung() { gibFrei(); }
    Tonfuehrung(const Tonfuehrung&) = delete;
    Tonfuehrung& operator=(const Tonfuehrung&) = delete;

    /// Den Player uebernehmen: die Playlist ruht ab jetzt. Ein zweiter Aufruf
    /// mit demselben Player tut nichts.
    void uebernimm(IAudioPlayer& player);
    /// Den Player freigeben: die Playlist haengt wieder, der Pegel steht auf
    /// voll. Was gerade spielt, spielt weiter.
    void gibFrei();
    [[nodiscard]] bool fuehrt() const { return m_player != nullptr; }

    /// Wohin der Pegel des Clips geht (0…1, je Takt). Die Lautstaerke des
    /// Players bleibt, wie sie ist — der Pegel kommt obendrauf (Naht N7a).
    void setPegelSenke(std::function<void(float)> senke) { m_pegel = std::move(senke); }

    struct Ergebnis
    {
        bool uhrStellen = false;    ///< der Ton fuehrt: die Uhr gehoert auf `stelleMs`
        std::int64_t stelleMs = 0;
    };
    /// Ein Takt: `stelleMs` ist die Stelle der Uhr, `laeuft`, ob sie laeuft.
    Ergebnis takt(const Komposition& komposition, std::int64_t stelleMs, bool laeuft);

    /// Der Song ist vor dem Ende seines Clips zu Ende gegangen (oder liess
    /// sich nicht laden): bis zum Clip-Ende fuehrt die Wanduhr.
    [[nodiscard]] bool songAus() const { return m_songAus; }

    /// Der Anfang des naechsten (`vor`) bzw. vorigen Songs der Komposition,
    /// von `stelleMs` aus — fuer Next / Previous. Zurueck: mitten in einem
    /// Song zuerst an dessen Anfang, in seinen ersten Sekunden zum Song davor.
    /// -1 = es gibt keinen.
    [[nodiscard]] static std::int64_t songAnfang(const Komposition& komposition, std::int64_t stelleMs,
                                                 bool vor);

private:
    /// Woran ein Ton-Clip wiedererkannt wird; leer = Luecke.
    struct Schluessel
    {
        QString datei;
        std::int64_t startMs = -1;
        std::int64_t einsatzMs = 0;
        [[nodiscard]] bool operator==(const Schluessel& o) const
        {
            return datei == o.datei && startMs == o.startMs && einsatzMs == o.einsatzMs;
        }
    };

    IAudioPlayer* m_player = nullptr;
    IPlaylist* m_playlist = nullptr;  ///< die ausgehaengte Playlist
    std::function<void(float)> m_pegel;

    Schluessel m_schluessel;          ///< der Clip des letzten Takts
    bool m_ersterTakt = true;
    bool m_songAus = false;
    bool m_liefZuletzt = false;       ///< Uhr lief und der Song spielte im letzten Takt
    std::int64_t m_letzteStelle = 0;  ///< Stelle der Uhr im letzten Takt
    std::int64_t m_letztePos = 0;     ///< Stelle im Song im letzten Takt
};

}  // namespace lumi::composer
