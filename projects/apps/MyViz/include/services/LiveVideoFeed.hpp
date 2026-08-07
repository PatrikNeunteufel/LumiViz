/**
 ****************************************************************************************
 * @file   LiveVideoFeed.hpp
 * @brief  Live-Videoquellen je Chain-Node: Datei-Streaming + Kamera (Qt Multimedia)
 *
 * @author Patrik Neunteufel
 * @date   August 2026
 * @version 1.4.0
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
 * EIN-THREAD-BESITZ (Umbau S71, Entscheid Patrik — der Kern der Loesung):
 * ALLE Qt-Multimedia-Objekte eines Feeds (QCamera, QMediaCaptureSession,
 * QMediaPlayer, QVideoSink) werden auf dem MEDIEN-THREAD erzeugt, benutzt und
 * zerstoert. Der Main-Thread erteilt nur Auftraege ("Feed anlegen", "Feed
 * beenden") und wartet beim Herunterfahren auf die Bestaetigung.
 *
 * Bis S71 lagen Pipeline (Main-Thread) und Frame-Verarbeitung (eigener Thread)
 * AUSEINANDER — das war die eigentliche Krankheit: die Pipeline starb auf dem
 * einen Thread, waehrend der andere noch ihre Frames hielt. Jede Absicherung
 * an dieser Naht (Totflagge, Barriere, Bau-Riegel, Quit-Schleife) kurierte nur
 * Symptome. Auf EINEM Thread koennen Frame-Verarbeitung und Abbau einander
 * nicht mehr ueberholen; die Wandlung laeuft zudem wieder im Lieferkontext,
 * dem laut S70 einzig sicheren Ort fuer Hardware-Frames. Zusaetzlich bindet
 * Media Foundation (COM) seine Objekte an das Apartment des erzeugenden
 * Threads — auf dem Medien-Thread haengen sie nicht mehr am GUI-Apartment,
 * das ~QGuiApplication abraeumt.
 *
 * Threading: starteDatei/starteKamera/stopp sind render-thread-tauglich
 * (Queued-Invoke auf den Medien-Thread). `letztesBild`/`bildNummer` kopieren
 * unter Mutex und sind von jedem Thread aus zulaessig.
 *
 * ABSCHALT-VERTRAG (Befund S71): ein Feed wird NIE gestoppt oder zerstoert,
 * solange noch eines seiner Frames verarbeitet wird. `feedStilllegen()` ist
 * die EINZIGE Abbau-Stelle — Totflagge, Senke abklemmen, Barriere, DANN
 * stop/zerstoeren. Seit dem Ein-Thread-Besitz ist die Barriere auf dem
 * Medien-Thread ein No-op (dort kann per Definition kein Frame parallel
 * laufen); sie bleibt als Absicherung fuer Aufrufe von aussen stehen.
 ****************************************************************************************
 */

#pragma once

#include <QImage>
#include <QMutex>
#include <QObject>
#include <QString>

#include <atomic>
#include <cstdint>
#include <functional>
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

    /// Laufen auf dem MEDIEN-THREAD (Queued-Invoke aus starteDatei/-Kamera):
    /// dort entstehen alle Qt-Multimedia-Objekte und dort sterben sie auch.
    void baueDatei(std::uint64_t nodeId, const QString& pfad, bool loop,
                   double tempo);
    void baueKamera(std::uint64_t nodeId, const QString& geraeteId);
    /// Entsorgt gestoppte Feeds (m_friedhof) auf dem Medien-Thread. Das
    /// Queued-Lambda von stopp() traegt bewusst KEIN Eigentum — landet das
    /// Event nie (Shutdown), raeumt alleStoppen() ab. Sonst stirbt die
    /// QCamera beim Entsorgen der Event-Queue (S70-Befund #2).
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
     * @brief EINEN Feed vertragsgemaess abschalten (MEDIEN-THREAD)
     *
     * Die einzige erlaubte Abschalt-Reihenfolge (s. ABSCHALT-VERTRAG oben):
     * 1. Totflagge setzen — ab jetzt bleibt jedes Frame liegen,
     * 2. Senke abklemmen — es kommen keine neuen Frames mehr nach,
     * 3. `wandlerBarriere()` — auf dem Medien-Thread ein No-op (dort kann
     *    kein Frame parallel laufen), Absicherung fuer Aufrufe von aussen,
     * 4. erst DANN stop() und Zerstoerung der Qt-Multimedia-Objekte.
     */
    void feedStilllegen(const std::shared_ptr<Feed>& feed);
    /**
     * @brief Sicherstellen, dass kein Frame mehr in Verarbeitung ist
     *
     * Auf dem Medien-Thread selbst sofort erfuellt (der Handler laeuft dort,
     * er kann nicht gleichzeitig laufen). Von aussen: leeres Lambda hinter
     * die Queue posten und darauf warten (Semaphore mit Timeout, kein
     * verschachtelter Event-Loop).
     */
    void wandlerBarriere(int maxMs);
    /// Sink-Anschluss: jedes Bild nach RGBX8888 wandeln und unter `mutex` in
    /// den Feed legen. Senke und Kontext leben auf dem Medien-Thread — die
    /// Zustellung ist damit DIRECT und die Wandlung laeuft im Lieferkontext
    /// (S70: der einzig sichere Ort fuer Hardware-Frames).
    void verbindeSenke(QVideoSink* senke, QMutex* mutex, QObject* kontext,
                       const std::shared_ptr<Feed>& feed);
    /// Auf dem Medien-Thread ausfuehren und auf Fertigmeldung warten
    /// (Semaphore + Timeout, NICHT BlockingQueued: ein haengender
    /// Medien-Thread darf den Abbau verzoegern, nicht verklemmen).
    /// @return false = Timeout, die Arbeit ist NICHT bestaetigt
    bool aufMedienThread(const std::function<void()>& arbeit, int maxMs);

    QMutex m_mutex;
    std::unordered_map<std::uint64_t, std::shared_ptr<Feed>> m_feeds;
    /// BAU-RIEGEL (Befund S71): laufender Bauauftrag je Knoten (Wert =
    /// Zielquelle). Der Render-Thread ruft `starte*` in JEDEM Frame; bis der
    /// gequeuete Bau durch ist, steht der Feed noch nicht in `m_feeds` — ohne
    /// diesen Riegel wuerde jedes Frame einen weiteren Bauauftrag queuen, und
    /// jeder davon raeumt (altenFeedRaeumen) die eben gebaute Kamera wieder
    /// ab: eine Endlosschleife aus Auf- und Abbau, die den Medien-Thread
    /// blockiert und die Kamera nie hochkommen laesst.
    std::unordered_map<std::uint64_t, QString> m_imBau;
    /// Gestoppte Feeds bis zur Entsorgung auf dem Medien-Thread
    std::vector<std::shared_ptr<Feed>> m_friedhof;
    /// MEDIEN-THREAD (Umbau S71): traegt die KOMPLETTE Qt-Multimedia-Seite —
    /// Aufbau, Frame-Verarbeitung und Abbau aller Feeds. Der Main-Thread
    /// erteilt nur Auftraege (s. EIN-THREAD-BESITZ im Dateikopf).
    QThread* m_medien = nullptr;
    QObject* m_medienKontext = nullptr;  ///< Auftrags-Kontext auf m_medien
    std::atomic<bool> m_kameraErlaubt{false};
    std::atomic<bool> m_feedLief{false};  ///< s. feedGelaufen()
    /// Shutdown-Riegel (S70): nach alleStoppen() darf NICHTS mehr starten —
    /// der Render-Thread ruft starte*() bis zum Fenster-Abbau weiter auf und
    /// wuerde sonst mitten im Teardown eine frische Kamera-Pipeline queuen
    /// (Main-Thread hing in nvoglv64: MF-Start gegen GL-Abbau).
    std::atomic<bool> m_beendet{false};
};

} // namespace lumi::services
