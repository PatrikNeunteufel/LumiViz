/**
 * @file ExportAuftrag.hpp
 * @brief Loop: *Export audio…* ohne Oberflaeche — Toene laden, rechnen, schreiben
 *        (docs/audio/Spielweisen_Konzept.md §4.4, Stufe 30c; Entscheide WX1 a bis WX9 a)
 *
 * Ein Auftrag laeuft auf einem eigenen Thread, in drei Schritten: die Toene ueber
 * einen `Tonauftrag` (aus dem Tonspeicher oder gerechnet, auf mehreren Kernen),
 * die Wellenform (`loop/Wellenform`), die Datei (`audio::schreibeWav`). Wer ihn
 * haelt, fragt `stand` ab (etwa je Takt einer Uhr) und kann ihn abbrechen; danach
 * steht das `ergebnis` da.
 *
 * Lebenszyklus (Bootstrap_Integration.md §6.7): der Dekodierer nutzt die Engine —
 * der Destruktor bricht ab und wartet auf den Thread; ein Auftrag endet also vor
 * der Engine, wenn sein Halter es tut.
 */
#pragma once

#include "audio/WavDatei.hpp"
#include "loop/Bearbeiten.hpp"
#include "loop/Loop.hpp"
#include "loop/Tonauftrag.hpp"
#include "loop/Wellenform.hpp"

#include <QString>
#include <QStringList>

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>

namespace lumi::loop
{

/// Was exportiert wird.
enum class ExportArt
{
    Clip,        ///< ein Clip im Loop-Panel (F3 a)
    Loop,        ///< der ganze Loop im Loop-Panel (WX7 a)
    Ausschnitt,  ///< ein Loop-Clip des Composers (WX3 a)
};

struct ExportWunsch
{
    ExportArt art = ExportArt::Clip;
    Loop loop;                            ///< gerichtet
    ClipOrt clip;                         ///< nur `Clip`
    LoopEnde ende = LoopEnde::Nahtlos;    ///< nur `Loop`
    int durchlaeufe = 1;                  ///< nur `Loop`
    double versatzMs = 0.0;               ///< nur `Ausschnitt`: die Stelle im Loop, an der der Clip beginnt
    double laengeMs = 0.0;                ///< nur `Ausschnitt`: die Laenge des Clips
    QString datei;
    audio::WavTiefe tiefe = audio::WavTiefe::Gleitkomma32;
    int rate = 44100;
};

enum class ExportPhase
{
    Toene,      ///< die Toene werden geladen
    Rechnen,    ///< die Wellenform entsteht
    Schreiben,  ///< die Datei
    Fertig,
    Abgebrochen,
    Fehler,
};

struct ExportStand
{
    ExportPhase phase = ExportPhase::Toene;
    int toeneErledigt = 0;
    int toeneGesamt = 0;
    double anteil = 0.0;  ///< beim Rechnen: 0 … 1
};

struct ExportErgebnis
{
    QString fehler;                      ///< leer = geschrieben
    std::int64_t frames = 0;             ///< in der Datei
    std::int64_t clipFrames = 0;         ///< davon Clip oder Durchlaeufe, der Rest ist Nachklang
    bool nachklangGekappt = false;
    int rate = 0;
    audio::WavErgebnis wav;
    qint64 bytes = 0;                    ///< so gross ist die Datei
    QStringList bericht;                 ///< Klaenge, die nicht zu laden waren
};

class ExportAuftrag
{
public:
    /// Beginnt sofort, auf einem eigenen Thread.
    ExportAuftrag(ExportWunsch wunsch, Dekodierer dekodierer);
    ~ExportAuftrag();
    ExportAuftrag(const ExportAuftrag&) = delete;
    ExportAuftrag& operator=(const ExportAuftrag&) = delete;

    /// Anhalten; wartet nicht. Danach endet der Auftrag mit `Abgebrochen`, ohne Datei.
    void brichAb();
    /// Warten, bis der Thread fertig ist (fuer Tests).
    void warte();

    [[nodiscard]] ExportStand stand() const;
    /// Fertig, abgebrochen oder fehlgeschlagen?
    [[nodiscard]] bool beendet() const;
    /// Erst, wenn `beendet`.
    [[nodiscard]] std::optional<ExportErgebnis> ergebnis() const;

private:
    void arbeite();
    void endet(ExportPhase phase, ExportErgebnis ergebnis);

    ExportWunsch m_wunsch;
    Dekodierer m_dekodierer;
    std::atomic<bool> m_abbruch{false};
    std::atomic<int> m_phase{static_cast<int>(ExportPhase::Toene)};
    std::atomic<int> m_toeneErledigt{0};
    std::atomic<int> m_toeneGesamt{0};
    std::atomic<double> m_anteil{0.0};
    mutable std::mutex m_sperre;           ///< fuer das Ergebnis und den Tonauftrag
    std::optional<ExportErgebnis> m_ergebnis;
    Tonauftrag* m_toene = nullptr;         ///< solange er laedt (unter der Sperre)
    std::thread m_thread;
};

/// Der Vorschlag fuer den Dateinamen: `<Loop>_<Basis>_bar<N>.wav` fuer einen Clip,
/// `<Loop>.wav` fuer den ganzen Loop, `<Loop>_<m-ss>.wav` fuer einen Loop-Clip des
/// Composers (die Stelle im Loop). Zeichen, die in Dateinamen nicht gehen, werden zu `_`.
[[nodiscard]] QString exportName(const QString& loopName, const ExportWunsch& wunsch);

/// Die Art als Text im Fenster und in der Rueckmeldung: „32-bit float“, „24-bit“, „16-bit“.
[[nodiscard]] QString tiefeText(audio::WavTiefe tiefe);

}  // namespace lumi::loop
