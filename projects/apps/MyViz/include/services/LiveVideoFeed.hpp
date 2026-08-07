/**
 ****************************************************************************************
 * @file   LiveVideoFeed.hpp
 * @brief  Live-Videoquellen je Chain-Node: Datei-Streaming + Kamera (Qt Multimedia)
 *
 * @author Patrik Neunteufel
 * @date   August 2026
 * @version 1.3.0
 *
 * @details
 * Gegenstueck zum VideoFrameCache (der dekodiert deterministisch KOMPLETT in den
 * Speicher): dieser Dienst liefert UHRZEITGETRIEBENE Frames — Datei-Streaming
 * ueber QMediaPlayer (lange Videos ohne RAM-Kosten) und Kamera ueber
 * QCamera/QMediaCaptureSession. Beide muenden in einen QVideoSink je Feed; das
 * jeweils letzte Bild liegt als RGBX8888-QImage unter Mutex bereit
 * (`letztesBild`), der Render-Thread holt es je Frame ab. Live heisst nicht
 * reproduzierbar — Sonden/Standalone nutzen den VideoFrameCache-Weg des
 * videoSource-Knotens (streaming aus).
 *
 * KAMERA-VERTRAG (Sondierung Offene_Punkte §7): ein Kamerageraet startet NIE
 * automatisch — `starteKamera()` ist ein No-op, solange nicht `erlaubeKamera()`
 * in DIESEM App-Lauf durch eine ausdrueckliche Nutzeraktion (Panel-Knopf,
 * Settings-Testaufnahme) gesetzt wurde. Ein Preset-Laden alleine oeffnet damit
 * nie den Windows-Berechtigungsdialog.
 *
 * Threading: starteDatei/starteKamera/stopp sind render-thread-tauglich (Queued-
 * Invoke auf den Main-Thread, dessen Event-Loop Qt Multimedia braucht —
 * dasselbe Muster wie VideoFrameCache). `letztesBild` kopiert unter Mutex.
 *
 * ABSCHALT-VERTRAG (Befund S71, Kern des Teardown-Deadlocks): ein Feed wird
 * NIE gestoppt oder zerstoert, solange der Wandler-Thread noch einen seiner
 * Frames anfasst. `feedStilllegen()` ist die EINZIGE erlaubte Reihenfolge —
 * Totflagge, Senke abklemmen, Wandler-Barriere, DANN stop/zerstoeren. Wird sie
 * verletzt, mappt der Wandler ein Frame einer bereits sterbenden MF-/D3D-
 * Pipeline, blockiert im Treiber und haelt den Pipeline-Puffer fuer immer —
 * genau daran starben die nvwgf2umx-Worker (Log-Beweis: `wait(5000)` des
 * Wandlers lief in JEDEM Lauf mit Feed in den Timeout).
 ****************************************************************************************
 */

#pragma once

#include <QImage>
#include <QMutex>
#include <QObject>
#include <QString>

#include <atomic>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

class QThread;
class QVideoSink;

namespace lumi::services {

/**
 * @class LiveVideoFeed
 * @brief Haelt je Chain-Node einen laufenden Video-/Kamera-Feed (Main-Thread)
 */
class LiveVideoFeed : public QObject
{
    // bewusst OHNE Q_OBJECT: keine eigenen Signals/Slots — Queued-Invoke
    // laeuft ueber das Lambda-Overload (Muster VideoFrameCache)
public:
    static LiveVideoFeed& instance();

    /// Kamera-Freigabe fuer diesen App-Lauf — nur aus einer ausdruecklichen
    /// Nutzeraktion heraus setzen (Panel-Knopf/Settings), nie beim Laden.
    void erlaubeKamera() { m_kameraErlaubt.store(true, std::memory_order_release); }
    [[nodiscard]] bool kameraErlaubt() const
    {
        return m_kameraErlaubt.load(std::memory_order_acquire);
    }

    /// Benutzerlokaler Ablageordner der Kamera-Testaufnahmen (wird angelegt);
    /// bewusst AUSSERHALB des Repos (Kameramaterial ist persoenlich).
    static QString testaufnahmenOrdner();

    /// Datei-Streaming starten bzw. weiterlaufen lassen (idempotent je Frame
    /// aufrufbar; nur ein Quellen-Wechsel baut den Feed neu). `tempo` wirkt
    /// als Abspielrate und darf sich laufend aendern.
    void starteDatei(std::uint64_t nodeId, const QString& pfad, bool loop,
                     double tempo);

    /// Kamera starten (idempotent). NO-OP ohne `erlaubeKamera()` in diesem
    /// App-Lauf. `geraeteId` = QCameraDevice::id() als UTF-8-String; eine
    /// unbekannte ID startet nichts (kein Retry-Spam bis zum ID-Wechsel).
    void starteKamera(std::uint64_t nodeId, const QString& geraeteId);

    /// Feed des Knotens beenden und freigeben (idempotent).
    void stopp(std::uint64_t nodeId);

    /// ALLE Feeds SYNCHRON beenden (Main-Thread). Setzt zugleich den
    /// Shutdown-Riegel (keine Neustarts mehr).
    void alleStoppen();

    /// Shutdown-Riegel setzen OHNE zu stoppen — haengt an aboutToQuit:
    /// ab hier startet nichts mehr, aber die laufende Kamera bleibt am
    /// Leben, bis der GL-/Fenster-Abbau durch ist (S70: eine STERBENDE
    /// MF-/D3D-Pipeline waehrend des GL-Teardowns verklemmte den GPU-Kanal,
    /// Haupt-Thread hing in nvoglv64).
    void beendenVorbereiten() { m_beendet.store(true, std::memory_order_release); }

    /// Kompletter Dienst-Abbau NACH dem Fenster-/GL-Teardown (Aufruf aus
    /// Application::shutdown, zwischen MainWindow- und QApplication-Abbau):
    /// alleStoppen + Wandler-Thread geordnet beenden.
    void herunterfahren();

    /// Lief in diesem App-Lauf IRGENDEIN Qt-Multimedia-Feed (Kamera ODER
    /// Datei-Streaming ODER eine Settings-Testaufnahme)? Steuert den
    /// Notausgang in Application::shutdown.
    /// BEFUND S71: das frueher benutzte Kriterium „Kamera lief" war zu eng —
    /// ein Lauf mit reinem Datei-Feed hing genauso (Log 00:28:55), weil die
    /// Wandler-Blockade nicht kameraspezifisch ist. Die Testaufnahme des
    /// Settings-Panels meldet sich ueber `merkeFremdenFeed()`.
    [[nodiscard]] bool feedGelaufen() const
    {
        return m_feedLief.load(std::memory_order_acquire);
    }

    /// Qt-Multimedia-Lauf ausserhalb dieses Dienstes melden (Settings-
    /// Testaufnahme, s. feedGelaufen()).
    void merkeFremdenFeed() { m_feedLief.store(true, std::memory_order_release); }

    /// Laufende Bild-Nummer des Feeds (0 = noch keins) — der billige
    /// Vorab-Check: erst wenn sie sich geaendert hat, lohnt `letztesBild`
    /// (die Wandlung dort kostet; Lag-Befund S70).
    std::uint64_t bildNummer(std::uint64_t nodeId);

    /**
     * @brief Letztes Bild des Feeds (RGBX8888, top-down)
     *
     * Wandelt das zuletzt abgelegte ROHE Frame auf dem AUFRUFER-Thread
     * (Render-Thread) — nie auf dem GUI-Thread (Lag-Befund S70). Vorher
     * per `bildNummer` pruefen, ob es ueberhaupt ein neues Frame gibt.
     * @param nummer laufende Bild-Nummer (0 = noch keins)
     * @return Null-Image, solange kein Frame ankam oder die Wandlung scheitert
     */
    QImage letztesBild(std::uint64_t nodeId, std::uint64_t* nummer = nullptr);

private:
    explicit LiveVideoFeed() = default;

    struct Feed;

    /// Laufen auf dem Main-Thread (Queued-Invoke aus starteDatei/-Kamera/stopp)
    void baueDatei(std::uint64_t nodeId, const QString& pfad, bool loop,
                   double tempo);
    void baueKamera(std::uint64_t nodeId, const QString& geraeteId);
    /// Entsorgt gestoppte Feeds (m_friedhof) auf dem Main-Thread. Das
    /// Queued-Lambda von stopp() traegt bewusst KEIN Eigentum — landet das
    /// Event nie (Shutdown), raeumt alleStoppen() synchron ab. Sonst stirbt
    /// die QCamera beim Entsorgen der Event-Queue MITTEN im ~QApplication
    /// (S70-Befund #2: Haupt-Thread haengt in Application::shutdown).
    void friedhofLeeren();
    /**
     * @brief Einen bestehenden Feed dieses Knotens vertragsgemaess abbauen
     *
     * BEFUND S71: `m_feeds[nodeId] = neu` zerstoerte einen Alt-Feed als
     * Nebenwirkung der Zuweisung — UNTER dem Mutex und ohne Abschalt-Vertrag.
     * Genau der Gifttrigger: die QCamera stirbt, waehrend der Wandler noch
     * ihre Frames mappt, und er haelt zugleich denselben Mutex an. Jeder
     * baue*-Pfad raeumt den Vorgaenger deshalb ZUERST, ohne Mutex.
     */
    void altenFeedRaeumen(std::uint64_t nodeId);
    /// Bauauftrag vormerken; false = fuer diesen Knoten laeuft schon einer
    /// auf dieselbe Quelle (dann NICHT nochmal queuen). S. `m_imBau`.
    bool bauVormerken(std::uint64_t nodeId, const QString& quelle);
    /// Bauauftrag austragen (jeder Ausgang von baueDatei/baueKamera).
    void bauAustragen(std::uint64_t nodeId);
    /**
     * @brief EINEN Feed vertragsgemaess abschalten (Main-Thread)
     *
     * Die einzige erlaubte Abschalt-Reihenfolge (s. ABSCHALT-VERTRAG oben):
     * 1. Totflagge setzen — der Wandler laesst ab jetzt jedes Frame liegen,
     * 2. Senke abklemmen — es kommen keine neuen Frames mehr nach,
     * 3. `wandlerBarriere()` — alle bereits zugestellten Frames sind durch
     *    und ihre Pipeline-Puffer zurueckgegeben,
     * 4. erst DANN stop() und Zerstoerung der Qt-Multimedia-Objekte.
     */
    void feedStilllegen(const std::shared_ptr<Feed>& feed);
    /**
     * @brief Warten, bis der Wandler-Thread alle bisherigen Frames durch hat
     *
     * Postet ein leeres Lambda HINTER die schon zugestellten Frame-Events und
     * wartet auf dessen Ausfuehrung (Semaphore, kein verschachtelter
     * Event-Loop). Timeout, damit ein blockierter Wandler den Abbau nur
     * verzoegert statt ihn zu verklemmen — die Warnung im Log ist dann der
     * Beleg, dass ein Frame im Treiber steckt.
     */
    void wandlerBarriere(int maxMs);
    /// Sink-Anschluss: jedes Bild nach RGBX8888 wandeln und unter `mutex`
    /// in den Feed legen. `kontext` lebt auf dem Wandler-Thread — die
    /// Zustellung ist queued, der Handler laeuft DORT (Lag-Befund S70:
    /// GUI-Thread fror ein; Render-Thread haengt bei Hardware-Frames).
    void verbindeSenke(QVideoSink* senke, QMutex* mutex, QObject* kontext,
                       const std::shared_ptr<Feed>& feed);

    QMutex m_mutex;
    std::unordered_map<std::uint64_t, std::shared_ptr<Feed>> m_feeds;
    /// BAU-RIEGEL (Befund S71): laufender Bauauftrag je Knoten (Wert =
    /// Zielquelle). Der Render-Thread ruft `starte*` in JEDEM Frame; bis der
    /// gequeuete Bau auf dem Main-Thread durch ist, steht der Feed noch nicht
    /// in `m_feeds` — ohne diesen Riegel wuerde jedes Frame einen weiteren
    /// Bauauftrag queuen, und jeder davon raeumt (altenFeedRaeumen) die eben
    /// gebaute Kamera wieder ab: eine Endlosschleife aus Auf- und Abbau, die
    /// den Main-Thread blockiert und die Kamera nie hochkommen laesst.
    std::unordered_map<std::uint64_t, QString> m_imBau;
    /// Gestoppte Feeds bis zur Main-Thread-Entsorgung (s. friedhofLeeren)
    std::vector<std::shared_ptr<Feed>> m_friedhof;
    /// Wandler-Thread (Lag-Befund S70): die Frame-Verbindungen werden queued
    /// an m_wandlerKontext zugestellt — die RGBX-Wandlung laeuft dort, weder
    /// GUI- noch Render-Thread fassen QVideoFrames an.
    QThread* m_wandler = nullptr;
    QObject* m_wandlerKontext = nullptr;  ///< Empfaenger-Kontext auf m_wandler
    std::atomic<bool> m_kameraErlaubt{false};
    std::atomic<bool> m_feedLief{false};  ///< s. feedGelaufen()
    /// Shutdown-Riegel (S70): nach alleStoppen() darf NICHTS mehr starten —
    /// der Render-Thread ruft starte*() bis zum Fenster-Abbau weiter auf und
    /// wuerde sonst mitten im Teardown eine frische Kamera-Pipeline queuen
    /// (Main-Thread hing in nvoglv64: MF-Start gegen GL-Abbau).
    std::atomic<bool> m_beendet{false};
};

} // namespace lumi::services
