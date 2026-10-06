/**
 * @file Tonfuehrung.hpp
 * @brief Composer: die Audio-Kanaele im Mixer (docs/ui/Composer_Konzept.md
 *        §4 und §8, Stufe C5b)
 *
 * Die Songs aller eingeschalteten Audio-Kanaele laufen als Quellen in EINEM
 * Mixer der Audio-Engine und ueberlagern sich, jeder mit seiner Pegelkurve
 * (Entscheide Patrik S97):
 *
 * - **Die Uhr fuehrt — immer.** Je Takt nennt die Sitzung die Stelle der Uhr;
 *   die Fuehrung haelt die Quellen daran: an die Stelle gesetzt wird eine
 *   Quelle beim Einstieg in ihren Clip, nach einem Sprung der Uhr und wenn sie
 *   der Uhr davongelaufen ist. Haelt die Uhr, haelt der Mixer.
 * - **Der Pegel** eines Clips ist die Lautstaerke seiner Quelle im Mix, je Takt.
 *   Lautstaerke und Mute des Players gelten fuer den ganzen Mixer.
 * - **Der laufende Song** ist der Clip mit dem hoechsten Wert der Pegelkurve
 *   an der Stelle; bei Gleichstand bleibt der bisherige, sonst gilt der
 *   oberste Kanal. Die Fuehrung nennt ihn im Ergebnis des Takts — wer taktet,
 *   meldet ihn weiter (Karaoke, Tags, Tag View …).
 *
 * Solange die Fuehrung den Ton hat, schweigt der Player und die Playlist
 * ruht: sie wird am Player ausgehaengt (`setPlaylist(nullptr)`) und beim
 * Freigeben wieder eingehaengt.
 *
 * Kein Qt-Objekt, kein Thread, keine Uhr, kein Ereignis-Bus. Laeuft gegen
 * `IAudioEngine` und `IAudioPlayer` — im Test gegen beide aus Pappe.
 *
 * Rueckbau (Konzept §12): Teil von `include/composer` / `src/composer`.
 */
#pragma once

#include "audio/AudioEvents.hpp"
#include "audio/IAudioEngine.hpp"
#include "composer/Komposition.hpp"

#include <QString>

#include <cstdint>
#include <vector>

class IAudioPlayer;
class IPlaylist;

namespace lumi::composer
{

class Tonfuehrung
{
public:
    /// So weit duerfen Uhr und Quelle auseinanderliegen, bevor die Quelle
    /// nachgesetzt wird. Von mir gesetzt (S98), nicht entschieden: jedes
    /// Nachsetzen ist zu hoeren.
    static constexpr std::int64_t kToleranzMs = 80;
    /// Nach einem Setzen wird die Quelle so lange nicht auf Abweichung geprueft.
    static constexpr std::int64_t kSchonfristMs = 500;
    /// Ein Schritt der Uhr zwischen zwei Takten, der groesser ist, gilt als Sprung.
    static constexpr std::int64_t kSprungMs = 1500;
    /// So lange vor seinem Anfang wird ein Song geoeffnet (angehalten), damit
    /// das Oeffnen der Datei den Einsatz nicht verzoegert. Von mir gesetzt (S98).
    static constexpr std::int64_t kVorlaufMs = 2000;
    /// „Previous" innerhalb der ersten Sekunden eines Songs geht zum Song davor.
    static constexpr std::int64_t kZurueckMs = 3000;

    Tonfuehrung() = default;
    ~Tonfuehrung() { gibFrei(); }
    Tonfuehrung(const Tonfuehrung&) = delete;
    Tonfuehrung& operator=(const Tonfuehrung&) = delete;

    /// Den Ton uebernehmen: ein Mixer entsteht, der Player wird angehalten,
    /// die Playlist ruht. false = die Engine hat keinen Mixer — dann bleibt
    /// alles, wie es war. Ein zweiter Aufruf bei laufender Fuehrung tut nichts.
    bool uebernimm(IAudioEngine& engine, IAudioPlayer& player);
    /// Den Ton freigeben: Quellen und Mixer gehen, die Playlist haengt wieder.
    /// Der Player bleibt angehalten.
    void gibFrei();
    [[nodiscard]] bool fuehrt() const { return m_engine != nullptr; }
    /// Der Mixer — Spektrum und Wellenform der Summe (Naht N13). INVALID_STREAM = keiner.
    [[nodiscard]] AudioStreamHandle mixer() const { return m_mixer; }

    struct Ergebnis
    {
        bool hatSong = false;          ///< an der Stelle laeuft ein Song
        bool songGewechselt = false;   ///< ein anderer als im letzten Takt — `song` ist neu
        TrackInfo song;                ///< der laufende Song (nur bei `songGewechselt` gefuellt)
        std::int64_t songStelleMs = 0; ///< die Stelle im laufenden Song
        int songDauerMs = 0;           ///< seine Laenge
        bool songLaeuft = false;       ///< die Uhr laeuft und der Song ist nicht zu Ende
        /// Der Player spielte doch (jemand hat ihn gestartet) und wurde in
        /// diesem Takt angehalten — seine Meldungen haben die des laufenden
        /// Songs ueberschrieben, wer taktet, meldet ihn noch einmal.
        bool playerAngehalten = false;
    };
    /// Ein Takt: `stelleMs` ist die Stelle der Uhr, `laeuft`, ob sie laeuft.
    Ergebnis takt(const Komposition& komposition, std::int64_t stelleMs, bool laeuft);

    /// Die Stelle der Komposition, an der im zuletzt gemeldeten laufenden Song
    /// der `anteil` (0…1) seiner Laenge zu hoeren ist — fuer den Klick in den
    /// Balken des Player-Panels. Bleibt im Clip des Songs (geklemmt). -1 = es
    /// gab noch keinen laufenden Song, oder sein Clip ist nicht mehr da.
    [[nodiscard]] std::int64_t stelleImLaufenden(const Komposition& komposition, double anteil) const;

    /// Wie oft eine Quelle nachgesetzt wurde, weil sie der Uhr davonlief —
    /// seit dem Uebernehmen.
    [[nodiscard]] int nachgesetzt() const { return m_nachgesetzt; }
    /// Wie viele Quellen der Mixer gerade hat (auch die im Vorlauf).
    [[nodiscard]] int quellen() const;

    /// Der Anfang des naechsten (`vor`) bzw. vorigen Songs der Komposition,
    /// von `stelleMs` aus — fuer Next / Previous. Zurueck: mitten in einem
    /// Song zuerst an dessen Anfang, in seinen ersten Sekunden zum Song davor.
    /// -1 = es gibt keinen.
    [[nodiscard]] static std::int64_t songAnfang(const Komposition& komposition, std::int64_t stelleMs,
                                                 bool vor);

private:
    /// Woran ein Ton-Clip wiedererkannt wird; leer = keiner.
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
    /// Ein Ton-Clip im Mixer.
    struct Quelle
    {
        Schluessel schluessel;
        AudioStreamHandle handle = INVALID_STREAM;  ///< INVALID_STREAM = liess sich nicht oeffnen
        int dauerMs = 0;
        bool imClip = false;             ///< die Uhr stand im letzten Takt in seinem Clip
        float pegel = -1.0f;             ///< zuletzt gesetzt
        std::int64_t gesetztBeiMs = 0;   ///< Stelle der Uhr beim letzten Setzen
        bool gesehen = false;            ///< in diesem Takt noch gebraucht
    };

    IAudioEngine* m_engine = nullptr;
    IAudioPlayer* m_player = nullptr;
    IPlaylist* m_playlist = nullptr;  ///< die ausgehaengte Playlist
    AudioStreamHandle m_mixer = INVALID_STREAM;
    std::vector<Quelle> m_quellen;
    float m_laut = -1.0f;             ///< zuletzt am Mixer gesetzt

    Schluessel m_laufend;             ///< der laufende Song des letzten Takts
    int m_laufendDauerMs = 0;         ///< seine Laenge
    bool m_ersterTakt = true;
    bool m_liefZuletzt = false;       ///< die Uhr lief im letzten Takt
    std::int64_t m_letzteStelle = 0;  ///< Stelle der Uhr im letzten Takt
    int m_nachgesetzt = 0;
};

}  // namespace lumi::composer
