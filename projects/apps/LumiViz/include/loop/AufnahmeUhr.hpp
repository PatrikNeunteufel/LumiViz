/**
 * @file AufnahmeUhr.hpp
 * @brief Loop: eine Aufnahme an der Wiedergabe — wo ein aufgenommener Abtastwert in der Uhr
 *        der Wiedergabe liegt, die Latenz, der Take und der Loop, der dazu spielt
 *        (docs/audio/Loop_Konzept.md §4p, Stufe 15k-1; Entscheide Patrik WV10 a, WV10-1 a,
 *        WR1 a, Skizze SK15k-1 a)
 *
 * **Die Uhr.** Waehrend aufgenommen wird, liest die Engine bei jedem Block eine Uhr der
 * Wiedergabe (`IAudioEngine::setRecordClock`) — beim Loop die Stelle des Klangerzeugers.
 * Aus den Marken folgt ein **Versatz**: Abtastwert k der Aufnahme kam an, als die Uhr bei
 * k + Versatz stand (der Median ueber alle Bloecke: das Ruckeln der Bloecke faellt heraus).
 *
 * **Die Latenz.** Was die Uhr gerade spielt, hoert man spaeter, und was man singt, kommt
 * spaeter an — beides zusammen ist die Latenz L in Abtastwerten. Gespielt wurde zu
 * Abtastwert k also die Stelle k + Versatz − L. Gemessen wird sie mit derselben Uhr:
 * `KlickGeber` spielt Klicks an bekannten Stellen, `messeLatenz` findet sie in der Aufnahme.
 * Was die Uhr an festem Abstand zum Hoeren hat, steckt so mit in L — der Wert gilt fuer
 * jeden Generator im Mixer der Engine, also auch fuer den Loop.
 *
 * Die Uhr haengt nicht an Wave-Clips: Noten nutzen sie spaeter (Live-Loop-Modus, WV10 a).
 * Kein Qt-Widget.
 */
#pragma once

#include "audio/IAudioEngine.hpp"
#include "loop/Loop.hpp"

#include <QString>

#include <atomic>
#include <cstdint>
#include <optional>
#include <vector>

namespace lumi::loop
{

/// Der Versatz der Aufnahme gegen die Uhr: der Median von Uhr − aufgenommen ueber die
/// Marken, an denen die Uhr seit der vorigen weiterlief (vorher steht sie still). Leer =
/// keine solche Marke.
[[nodiscard]] std::optional<std::int64_t> uhrVersatz(const std::vector<IAudioEngine::RecordClockMark>& marken);

// --- die Latenz messen (*Measure latency*, SK15k-1 a B) ------------------------------

/// So viele Klicks, in diesem Abstand, der erste nach diesem Vorlauf (von Claude gesetzt).
inline constexpr int kLatenzKlicks = 5;
inline constexpr double kKlickAbstandS = 0.5;
inline constexpr double kKlickVorlaufS = 0.3;
/// Bis so weit hinter einem Klick wird er in der Aufnahme gesucht.
inline constexpr double kLatenzHoechstensS = 0.4;
/// So lang ist ein Klick: ein abklingender Ton von 1 kHz.
inline constexpr double kKlickS = 0.01;

/// Wo die Klicks in der Uhr des `KlickGeber` liegen, in Abtastwerten.
[[nodiscard]] std::vector<std::int64_t> klickStellen(int rate);
/// So lang laeuft eine Messung: bis nach dem letzten Klick Zeit fuer die Latenz bleibt.
[[nodiscard]] std::int64_t messungFrames(int rate);
/// Der Wert eines Klicks `i` Abtastwerte nach seinem Anfang; 0 danach.
[[nodiscard]] float klickWert(std::int64_t i, int rate);

/// Spielt die Klicks einer Messung — ein Generator im Mixer der Engine. Seine Stelle ist
/// die Uhr der Messung.
class KlickGeber final : public IAudioGenerator
{
public:
    explicit KlickGeber(int rate);
    void render(float* out, int frames) override;
    /// Die Uhr: so viele Abtastwerte sind gerechnet.
    [[nodiscard]] std::int64_t uhr() const { return m_stelle.load(); }
    /// Alle Klicks gespielt und Zeit fuer ihre Latenz vergangen?
    [[nodiscard]] bool fertig() const { return uhr() >= messungFrames(m_rate); }
    [[nodiscard]] int rate() const { return m_rate; }

private:
    int m_rate;
    std::vector<std::int64_t> m_klicks;
    std::atomic<std::int64_t> m_stelle{0};
};

/// Was eine Messung fand.
struct Latenzmessung
{
    std::optional<double> ms;  ///< der Median der gefundenen Klicks; leer = zu wenige gefunden
    double streuungMs = 0.0;   ///< groesste minus kleinste
    int gefunden = 0;
    int erwartet = 0;
};

/// Die Klicks `klicks` (Stellen in der Uhr) in der Aufnahme `stereo` suchen, die mit
/// `versatz` an der Uhr liegt: je Klick der erste Abtastwert, der die Haelfte der Spitze in
/// seinem Fenster (bis `kLatenzHoechstensS`) erreicht — nur, wenn die Spitze deutlich ueber
/// dem Rauschen vor dem ersten Klick liegt. Gefunden sein muessen mindestens drei.
[[nodiscard]] Latenzmessung messeLatenz(const std::vector<float>& stereo, std::int64_t versatz,
                                        const std::vector<std::int64_t>& klicks, int rate);

/// Die gemerkte Latenz fuer dieses Paar aus Eingang und Ausgang (Namen der Geraete), in ms;
/// leer = noch keine.
[[nodiscard]] std::optional<double> gemerkteLatenzMs(const QString& eingang, const QString& ausgang);
void merkeLatenzMs(const QString& eingang, const QString& ausgang, double ms);

// --- der Take ------------------------------------------------------------------------

/// Aus der Aufnahme `stereo` (mit `versatz` an der Uhr, Latenz `latenz`) das Stueck, das zur
/// Wiedergabe von `vonUhr` bis `bisUhr` gehoert. Was vor dem Beginn der Aufnahme laege, ist
/// still — so bleibt der Take an seinem Platz.
[[nodiscard]] std::vector<float> schneideTake(const std::vector<float>& stereo, std::int64_t versatz,
                                              std::int64_t latenz, std::int64_t vonUhr, std::int64_t bisUhr);

// --- der Loop, der zur Aufnahme spielt -------------------------------------------------

/// So lang wird ein Take hoechstens (von Claude gesetzt).
inline constexpr double kTakeHoechstensSekunden = 600.0;

/// *Play while recording* (Entscheide Patrik MH1 a, MH2 a, S169): was waehrend des Takes zu
/// hoeren ist. Ohne Kopfhoerer hoert das Mikrofon die Lautsprecher — dann nichts. Die Werte
/// stehen so in den Einstellungen (`loop/recordPlayWhile`).
enum class Mithoeren
{
    LoopUndKlick = 0,  ///< *Loop and click*: einzaehlen, der Take sitzt im Takt (Kopfhoerer)
    NurKlick = 1,      ///< *Click only*: einzaehlen, nur der Klick spielt weiter (Kopfhoerer)
    Frei = 2,          ///< *Nothing — record right away*: wie ein Sample, nichts spielt (MH2 a)
};

/// Was aufgenommen wird (SK15k-1 a A).
struct WaveAufnahmePlan
{
    int startTakt = 0;   ///< *Starts at bar*: hier beginnt der Take (0 = der erste Takt)
    int einzaehlen = 1;  ///< *Count-in* in Takten, 0 … 2
    int takte = 0;       ///< *Length* in Takten; 0 = bis *Stop*
    Mithoeren mithoeren = Mithoeren::Frei;  ///< *Play while recording* (Vorgabe MH2 a)

    [[nodiscard]] bool operator==(const WaveAufnahmePlan&) const = default;
};

/// Der Loop, den der Spieler zur Aufnahme spielt, und wo darin gespielt und aufgenommen wird.
struct Aufnahmefassung
{
    Loop loop;
    int abspielTakt = 0;   ///< hier beginnt die Wiedergabe (mit dem Einzaehlen)
    int aufnahmeTakt = 0;  ///< hier beginnt der Take
};

/// Die Fassung zum Aufnehmen: reicht das Einzaehlen vor Takt 0, ruecken alle Clips um so viele
/// Takte nach hinten; dazu ein Kanal mit dem Klick (`kit:hat_closed` auf jedem Schlag, die
/// Eins lauter) — beim Einzaehlen und waehrend der Aufnahme. Bei *Loop and click* bis zum
/// Ende des Takes, bei *Length* „bis Stop“ bis zum Ende des Loops. Bei *Click only* endet der
/// Loop am Aufnahmetakt (`schneideAb`), und der Klick reicht bis zum Ende des Takes — bei „bis
/// Stop“ bis `kTakeHoechstensSekunden`, damit der Spieler nicht an den Anfang zurueckspringt.
/// `Mithoeren::Frei` spielt keine Fassung (das Fenster nimmt dann wie ein Sample auf).
[[nodiscard]] Aufnahmefassung aufnahmeFassung(const Loop& loop, const WaveAufnahmePlan& plan);

/// Der Loop endet am Takt `takt`: Clips ab dort fallen weg, eine Wiederholung, die ueber ihn
/// reicht, spielt eine auf die Takte davor gekuerzte Kopie ihres Musters (`<Name> (count-in)`,
/// mit Effekten und Kurve des Clips, nicht eingefroren). Danach `richte`.
void schneideAb(Loop& loop, int takt);

/// Ein Takt in Abtastwerten bei `rate` (wie der Rechenkern ihn rechnet).
[[nodiscard]] double taktFrames(const Loop& loop, int rate);

}  // namespace lumi::loop
