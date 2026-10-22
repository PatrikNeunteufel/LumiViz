/**
 * @file FreezeAuftrag.hpp
 * @brief Loop: Clips einfrieren ohne Oberflaeche — Toene laden, rechnen, ablegen
 *        (docs/audio/Spielweisen_Konzept.md §4.4, Stufe 30g; Entscheide F2 a, F3 a, FZ4 a)
 *
 * Ein Auftrag laeuft auf einem eigenen Thread und nimmt die Clips nacheinander: je
 * Clip die Toene ueber einen `Tonauftrag` (aus dem Tonspeicher oder gerechnet, auf
 * mehreren Kernen), die Wellenform (`rechneClip`, F3 a) und die Datei im Ordner
 * `frozen` (`legeFrozen`) unter dem **jetzigen** Abdruck des Clips. Liegt sie schon
 * vor, wird nichts gerechnet. Wer ihn haelt, fragt `stand` ab (etwa je Takt einer
 * Uhr) und kann ihn abbrechen; danach steht das `ergebnis` da.
 *
 * Lebenszyklus (Bootstrap_Integration.md §6.7): der Dekodierer nutzt die Engine —
 * der Destruktor bricht ab und wartet auf den Thread.
 */
#pragma once

#include "loop/Bearbeiten.hpp"
#include "loop/Loop.hpp"
#include "loop/Tonauftrag.hpp"

#include <QByteArray>
#include <QStringList>

#include <atomic>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

namespace lumi::loop
{

struct FreezeStand
{
    int clip = 0;          ///< der wievielte gerade dran ist, ab 0
    int clips = 0;         ///< so viele sind es
    double anteil = 0.0;   ///< am laufenden Clip: Toene bis 0,5, Rechnen bis 1
    bool beendet = false;
};

struct FreezeErgebnis
{
    bool abgebrochen = false;
    std::vector<ClipOrt> gelegt;     ///< Clips, deren Wellenform jetzt vorliegt (auch die schon da war)
    std::vector<ClipOrt> fehlgeschlagen;
    QStringList bericht;             ///< Klaenge, die nicht zu laden waren; was nicht gelang
};

class FreezeAuftrag
{
public:
    /// Beginnt sofort, auf einem eigenen Thread. `loop` gerichtet.
    FreezeAuftrag(Loop loop, std::vector<ClipOrt> orte, int rate, Dekodierer dekodierer);
    ~FreezeAuftrag();
    FreezeAuftrag(const FreezeAuftrag&) = delete;
    FreezeAuftrag& operator=(const FreezeAuftrag&) = delete;

    /// Anhalten; wartet nicht. Was schon abgelegt ist, bleibt.
    void brichAb();
    /// Warten, bis der Thread fertig ist (fuer Tests).
    void warte();

    [[nodiscard]] FreezeStand stand() const;
    [[nodiscard]] bool beendet() const { return m_beendet.load(); }
    /// Erst, wenn `beendet`.
    [[nodiscard]] std::optional<FreezeErgebnis> ergebnis() const;
    /// Die Clips des Auftrags (fuer die Marke „computing…“).
    [[nodiscard]] const std::vector<ClipOrt>& orte() const { return m_orte; }

private:
    void arbeite();
    /// Einen Clip; false = abgebrochen.
    bool friere(const ClipOrt& ort, FreezeErgebnis& ergebnis);

    Loop m_loop;
    std::vector<ClipOrt> m_orte;
    int m_rate = 0;
    Dekodierer m_dekodierer;
    std::atomic<bool> m_abbruch{false};
    std::atomic<bool> m_beendet{false};
    std::atomic<int> m_clip{0};
    std::atomic<double> m_anteil{0.0};
    mutable std::mutex m_sperre;           ///< fuer das Ergebnis und den Tonauftrag
    std::optional<FreezeErgebnis> m_ergebnis;
    Tonauftrag* m_toene = nullptr;         ///< solange er laedt (unter der Sperre)
    std::thread m_thread;
};

}  // namespace lumi::loop
