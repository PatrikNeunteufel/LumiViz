/**
 * @file BounceAuftrag.hpp
 * @brief Loop: einen Clip fuer *Bounce* rechnen, ohne Oberflaeche (Stufe 15k-2; Entscheid WV7 a)
 *
 * Wie ein Clip in `FreezeAuftrag`: auf einem eigenen Thread die Toene ueber einen `Tonauftrag`
 * (aus dem Tonspeicher oder gerechnet), dann die Wellenform mit Nachklang (`rechneClip`, F3 a).
 * Wer ihn haelt, fragt `beendet` ab und nimmt danach die `welle`; die Datei legt er selbst ab
 * (auf dem Thread der Oberflaeche, `speichereAufnahme`).
 *
 * Lebenszyklus (Bootstrap_Integration.md §6.7): der Dekodierer nutzt die Engine — der Destruktor
 * bricht ab und wartet auf den Thread.
 */
#pragma once

#include "loop/Bearbeiten.hpp"
#include "loop/Loop.hpp"
#include "loop/Tonauftrag.hpp"
#include "loop/Wellenform.hpp"

#include <QString>
#include <QStringList>

#include <atomic>
#include <mutex>
#include <optional>
#include <thread>

namespace lumi::loop
{

class BounceAuftrag
{
public:
    /// Beginnt sofort. `loop` gerichtet; `ort` der Drum- oder Instrument-Clip darin.
    BounceAuftrag(Loop loop, ClipOrt ort, int rate, Dekodierer dekodierer);
    ~BounceAuftrag();
    BounceAuftrag(const BounceAuftrag&) = delete;
    BounceAuftrag& operator=(const BounceAuftrag&) = delete;

    void brichAb();
    void warte();
    [[nodiscard]] bool beendet() const { return m_beendet.load(); }
    /// 0 … 1: Toene bis 0,5, Rechnen bis 1.
    [[nodiscard]] double anteil() const;
    /// Erst, wenn `beendet`: die Wellenform; leer = nicht gelungen oder abgebrochen.
    [[nodiscard]] std::optional<Wellenform> welle() const;
    [[nodiscard]] QStringList bericht() const;
    /// Der Loop, in dem gerechnet wurde, und der Clip darin.
    [[nodiscard]] const Loop& loop() const { return m_loop; }
    [[nodiscard]] ClipOrt ort() const { return m_ort; }

private:
    void arbeite();

    Loop m_loop;
    ClipOrt m_ort;
    int m_rate = 0;
    Dekodierer m_dekodierer;
    std::atomic<bool> m_abbruch{false};
    std::atomic<bool> m_beendet{false};
    std::atomic<double> m_anteil{0.0};
    mutable std::mutex m_sperre;  ///< fuer Ergebnis, Bericht und den Tonauftrag
    std::optional<Wellenform> m_welle;
    QStringList m_bericht;
    Tonauftrag* m_toene = nullptr;  ///< solange er laedt (unter der Sperre)
    std::thread m_thread;
};

}  // namespace lumi::loop
