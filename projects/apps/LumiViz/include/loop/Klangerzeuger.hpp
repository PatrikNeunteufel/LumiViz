/**
 * @file Klangerzeuger.hpp
 * @brief Loop: die lebende Quelle im Mixer (docs/audio/Loop_Konzept.md,
 *        Entscheid L1 a, Stufe 5)
 *
 * Der Klangerzeuger haengt als `IAudioGenerator` im Mixer der Audio-Engine und
 * rechnet im Ton-Thread, Block fuer Block, was sein **Programm** an seiner
 * **Stelle** hoeren laesst. Beides wird von aussen gesetzt, ohne den Ton-Thread
 * anzuhalten:
 *
 * - Das Programm ist ein fertig uebersetzter Loop mit seinen Klaengen und
 *   aendert sich nie. Wer am Loop dreht, baut ein neues und setzt es; es gilt
 *   ab dem naechsten Block.
 * - Die Stelle fuehrt der Klangerzeuger selbst (Beleg §6: BASSmix setzt sie
 *   nicht). Sie zaehlt Abtastwerte seit dem Anfang und laeuft ueber die
 *   Durchlaeufe hinweg weiter.
 *
 * Ob er laeuft, bestimmt der Mixer: eine angehaltene Quelle wird nicht gerufen.
 */
#pragma once

#include "audio/IAudioEngine.hpp"
#include "loop/Effekte.hpp"
#include "loop/Rechenkern.hpp"

#include <atomic>
#include <cstdint>
#include <memory>
#include <vector>

namespace lumi::loop
{

/// So viele Abtastwerte rechnet der Klangerzeuger am Stueck, wenn Clips
/// Effekte haben — der Mixer holt Bloecke bis ueber 8000 (Beleg §6), die
/// Bloecke der Busse sind aber fest.
inline constexpr int kEffektBlock = 1024;
/// Springt die Stelle weiter als das, vergessen Echo und Hall, was sie
/// gehoert haben; darunter (das Nachsetzen nach T1 a) klingen sie weiter
/// (Entscheid Patrik S113, G1 b — der Wert ist von Claude gesetzt).
inline constexpr int kSprungMs = 200;

/// Das Gedaechtnis der Effekte eines Programms (G1 b): je Bus des Ablaufs
/// eine Kette und ein Block fuer seinen unbearbeiteten Ton. Angelegt mit dem
/// Programm, danach **nur vom Ton-Thread** angefasst.
struct EffektZustand
{
    std::vector<EffektKette> ketten;
    std::vector<std::int64_t> nachklang;  ///< je Bus: so lange klingt er nach seinem letzten Ton, in Abtastwerten
    std::vector<std::int64_t> stille;     ///< je Bus: so lange kam kein Ton mehr hinein
    std::vector<float> puffer;            ///< je Bus `2 * kEffektBlock` Werte
    std::vector<float*> zeiger;           ///< je Bus der Anfang seines Blocks
};

/// Ein uebersetzter Loop mit seinen Klaengen — unveraenderlich. Die Klaenge
/// werden geteilt: aendert sich nur ein Muster, bleibt der Satz derselbe.
struct Programm
{
    Ablauf ablauf;
    std::shared_ptr<const Klaenge> klaenge;
    /// nullptr, wenn kein Clip Effekte hat. Das Programm bleibt, wie es ist —
    /// was sich hier aendert, ist nur, was die Effekte gehoert haben.
    std::shared_ptr<EffektZustand> effekte;
};

/// Einen gerichteten Loop fuer `rate` uebersetzen und mit seinen Klaengen
/// (je Spur einer, in derselben Rate) zu einem Programm fuegen. `saiten` wie
/// bei `plane` (Stufe 23b): dieselbe Auskunft, mit der die Klaenge zu
/// `loop.klangNamen(saiten)` geladen wurden.
[[nodiscard]] std::shared_ptr<const Programm> baueProgramm(const Loop& loop, int rate,
                                                           std::shared_ptr<const Klaenge> klaenge,
                                                           const SaitenVon& saiten = {});

class Klangerzeuger final : public IAudioGenerator
{
public:
    Klangerzeuger() = default;
    Klangerzeuger(const Klangerzeuger&) = delete;
    Klangerzeuger& operator=(const Klangerzeuger&) = delete;

    /// Ein neues Programm setzen (nullptr = Stille). Die Stelle bleibt
    /// **musikalisch** dieselbe: aendert sich das Tempo, wird sie auf die neue
    /// Schrittlaenge umgerechnet — der Loop springt nicht im Takt.
    void setzeProgramm(std::shared_ptr<const Programm> programm);
    [[nodiscard]] std::shared_ptr<const Programm> programm() const { return m_programm.load(); }

    /// Die Stelle in Abtastwerten seit dem Anfang.
    void setzeStelle(std::int64_t frame) { m_stelle.store(frame < 0 ? 0 : frame); }
    [[nodiscard]] std::int64_t stelle() const { return m_stelle.load(); }
    /// Die Stelle im laufenden Durchlauf; 0 ohne Programm.
    [[nodiscard]] std::int64_t stelleImDurchlauf() const;

    /// Im Ton-Thread (`IAudioGenerator`): einen Block rechnen und die Stelle
    /// weiterzaehlen. Ohne Programm Stille; die Stelle bleibt dann stehen.
    void render(float* out, int frames) override;

    /// Der groesste Betrag der Summe seit dem letzten Aufruf (Entscheid Patrik
    /// S108, L6 a: der Kern begrenzt nicht — ueber 1 uebersteuert sie); setzt
    /// ihn auf null zurueck.
    [[nodiscard]] float nimmSpitze() { return m_spitze.exchange(0.0f); }

private:
    std::atomic<float> m_spitze{0.0f};
    std::atomic<std::shared_ptr<const Programm>> m_programm;
    /// Das zuletzt abgeloeste Programm: haelt es am Leben, damit sein Speicher
    /// nicht im Ton-Thread frei wird, der es noch einen Block lang benutzt.
    std::shared_ptr<const Programm> m_davor;
    std::atomic<std::int64_t> m_stelle{0};

    // --- nur im Ton-Thread ---
    /// Das Programm, das der Ton-Thread zuletzt gerechnet hat: von ihm
    /// uebernimmt das naechste das Gedaechtnis seiner Effekte.
    std::shared_ptr<const Programm> m_imTon;
    /// Die Stelle, an der der naechste Block beginnt, wenn niemand springt.
    std::int64_t m_erwartet = -1;

    // --- Uebergabe vom Ton-Thread zurueck ---
    /// Der Ton-Thread gibt ein Programm, das er nicht mehr braucht, hier ab,
    /// statt es selbst loszulassen (sein Speicher soll nicht im Ton-Thread
    /// frei werden); `setzeProgramm` raeumt es weg. Ist der Platz belegt,
    /// laesst der Ton-Thread es doch selbst los — dazu muessten zwei Wechsel
    /// ankommen, ohne dass dazwischen `setzeProgramm` lief.
    std::shared_ptr<const Programm> m_abgegeben;
    std::atomic<bool> m_abgegebenVoll{false};
};

}  // namespace lumi::loop
