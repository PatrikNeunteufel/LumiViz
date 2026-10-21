/**
 * @file Tonauftrag.hpp
 * @brief Loop: die Klaenge eines Loops laden — im Hintergrund auf mehreren Kernen
 *        (docs/audio/Spielweisen_Konzept.md §4.5, Stufe 29c; Entscheide H1 a, TV3 a)
 *
 * Ein Auftrag bekommt Klangnamen (`Loop::klangNamen`, `Ablauf::klangNamen`) und
 * eine Rate und liefert je Name einen Klang: aus dem Tonspeicher, gerechnet
 * (`rendereTon`) oder dekodiert. Jeder Name wird einmal geladen, auch wenn er
 * mehrfach vorkommt. Dekodiert wird ueber einen `Dekodierer` — eine Funktion,
 * nicht die ganze Engine (TV3 a); er muss von mehreren Threads zugleich gerufen
 * werden duerfen.
 *
 * Zwei Wege mit derselben Arbeit: `rechneHier` laedt auf dem rufenden Thread
 * (so laedt `ladeKlaenge`), `starte` auf einem Pool von Threads (Kerne − 1);
 * dann zeigen `erledigt`/`gesamt` den Fortschritt, `brichAb` haelt an, `fertig`
 * sagt, ob alles da ist. Der Bericht steht in der Reihenfolge der Namen, gleich
 * welcher Thread was zuerst fand.
 *
 * Lebenszyklus (Bootstrap_Integration.md §6.7): ein Auftrag, dessen Dekodierer
 * eine Engine nutzt, wird abgebrochen und zerstoert, bevor die Engine herunterfaehrt;
 * der Destruktor bricht ab und wartet auf seine Threads.
 */
#pragma once

#include "loop/Rechenkern.hpp"

#include <QString>
#include <QStringList>

#include <atomic>
#include <functional>
#include <memory>
#include <thread>
#include <vector>

namespace lumi::loop
{

/// Eine Datei dekodieren: ganz, Stereo, in `rate`, hoechstens `maxSekunden`. Leer = nicht zu lesen.
using Dekodierer = std::function<std::vector<float>(const QString& datei, int rate, double maxSekunden)>;

class Tonauftrag
{
public:
    Tonauftrag(QStringList namen, int rate, Dekodierer dekodierer);
    ~Tonauftrag();
    Tonauftrag(const Tonauftrag&) = delete;
    Tonauftrag& operator=(const Tonauftrag&) = delete;

    /// Nur nachsehen (TV6 a): liegt jeder Klang schon im Tonspeicher, ist der Auftrag danach
    /// fertig, ohne dass etwas gerechnet oder dekodiert wurde — true. Sonst false; was sich
    /// fand, ist gemerkt, und `rechneHier` oder `starte` laden den Rest. Holt die
    /// Instrumente (ihre Dateien), dekodiert aber nichts.
    [[nodiscard]] bool ausSpeicher();
    /// Alles auf dem rufenden Thread laden; danach ist der Auftrag fertig.
    void rechneHier();
    /// Im Hintergrund laden, auf `threads` Threads (0 = Kerne − 1, mindestens einer).
    void starte(int threads = 0);
    /// Anhalten: kein neuer Name wird mehr begonnen. Wartet nicht.
    void brichAb();
    /// Warten, bis alle Threads fertig oder abgebrochen sind.
    void warte();

    [[nodiscard]] bool fertig() const { return m_erledigt.load() == gesamt(); }
    /// Kein Thread des Auftrags arbeitet mehr (fertig oder nach `brichAb` angehalten) —
    /// dann blockiert der Destruktor nicht.
    [[nodiscard]] bool ruht() const { return m_laufend.load() == 0; }
    [[nodiscard]] bool abgebrochen() const { return m_abbruch.load(); }
    [[nodiscard]] int erledigt() const { return m_erledigt.load(); }
    /// Die verschiedenen Namen — so viel ist zu tun.
    [[nodiscard]] int gesamt() const { return static_cast<int>(m_einzeln.size()); }

    /// Je Name ein Klang, in der Reihenfolge der Namen; `nullptr` = nicht zu laden
    /// (oder noch nicht da). Erst nach `warte` oder `rechneHier` vollstaendig.
    [[nodiscard]] Klaenge klaenge() const;
    /// Was nicht zu laden war, in der Reihenfolge der Namen, jede Zeile einmal.
    [[nodiscard]] QStringList bericht() const;

private:
    struct Gemeinsam;  ///< je Instrument einmal geholt, unter einer Sperre
    struct Geholt;
    void arbeite();
    void lade(std::size_t einer);
    /// Den Klang aus dem Speicher holen, ohne zu rechnen; der Schluessel fuer `lade`.
    /// `geholt` = das Instrument eines Tons (nullptr bei einer Datei).
    bool ausSpeicher(std::size_t einer, QString& schluessel, Geholt*& geholt);

    QStringList m_namen;
    int m_rate = 0;
    Dekodierer m_dekodierer;
    std::vector<QString> m_einzeln;           ///< jeder Name einmal
    std::vector<std::size_t> m_platz;         ///< je Name: sein Index in `m_einzeln`
    std::vector<std::shared_ptr<const Klang>> m_ergebnis;  ///< je einzelnem Namen
    std::vector<QStringList> m_zeilen;        ///< je einzelnem Namen sein Bericht
    std::unique_ptr<Gemeinsam> m_gemeinsam;
    std::atomic<std::size_t> m_naechster{0};
    std::atomic<int> m_erledigt{0};
    std::atomic<bool> m_abbruch{false};
    std::atomic<int> m_laufend{0};            ///< Threads, die noch arbeiten
    std::vector<std::thread> m_threads;
};

}  // namespace lumi::loop
