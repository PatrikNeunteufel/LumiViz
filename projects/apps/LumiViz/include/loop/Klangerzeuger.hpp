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
#include "loop/Rechenkern.hpp"

#include <atomic>
#include <cstdint>
#include <memory>
#include <vector>

namespace lumi::loop
{

/// Ein uebersetzter Loop mit seinen Klaengen — unveraenderlich. Die Klaenge
/// werden geteilt: aendert sich nur ein Muster, bleibt der Satz derselbe.
struct Programm
{
    Ablauf ablauf;
    std::shared_ptr<const std::vector<Klang>> klaenge;
};

/// Einen gerichteten Loop fuer `rate` uebersetzen und mit seinen Klaengen
/// (je Spur einer, in derselben Rate) zu einem Programm fuegen.
[[nodiscard]] std::shared_ptr<const Programm> baueProgramm(const Loop& loop, int rate,
                                                           std::shared_ptr<const std::vector<Klang>> klaenge);

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

private:
    std::atomic<std::shared_ptr<const Programm>> m_programm;
    /// Das zuletzt abgeloeste Programm: haelt es am Leben, damit sein Speicher
    /// nicht im Ton-Thread frei wird, der es noch einen Block lang benutzt.
    std::shared_ptr<const Programm> m_davor;
    std::atomic<std::int64_t> m_stelle{0};
};

}  // namespace lumi::loop
