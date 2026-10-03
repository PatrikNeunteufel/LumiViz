/**
 ****************************************************************************************
 * @file   LyricsSyncPanel.hpp
 * @brief  Songtext nachsynchronisieren: Zeiten je Zeile mitklopfen und nachstellen
 *
 * @details
 * Session 87 (Idee Patrik S86): einen unsynchronen Text synchron machen —
 * oder einen synchronen nachstellen. Das Panel arbeitet am Text des laufenden
 * Titels (derselbe, den der Lyrics-Knoten zeigt):
 *
 * - **Mitklopfen:** „Tap" (Knopf, Enter oder T — seit S92 im ganzen Panel,
 *   ausser in Eingabefeldern) gibt der markierten Zeile die Abspielposition
 *   und rueckt zur naechsten Textzeile.
 * - **Sprungleiste** (S92): unter der Wellenform, ueber den ganzen Titel.
 * - **Kontextmenue** (S92): in der Liste und im Wort-Streifen, je nach Zeile.
 * - **Nachstellen:** markierte Zeilen ±100 ms, „ab hier" ±100 ms, Zeit
 *   loeschen, Zeit von Hand eintragen (F2 in der Spalte Time).
 * - **Vorhoeren:** Doppelklick auf eine Zeile springt im Titel kurz davor; der
 *   Knopf „▶" der Zeile springt genau auf ihre Zeit.
 * - **Spielzeit und Wellenform:** oben laeuft die Spielzeit mit; die Wellenform
 *   zeigt einen Ausschnitt von einigen Sekunden um die Abspielposition (Marke
 *   in der Mitte) mit den Zeiten der Zeilen als Striche — der Einsatz ist zu
 *   sehen, bevor er zu hoeren ist. Ein Klick in die Wellenform springt dorthin.
 * - **Vorschau:** jede Aenderung geht sofort an den Lyrics-Knoten — man sieht
 *   im Bild, was man tut. Gespeichert ist damit noch nichts.
 * - **Speichern** schreibt eine `.lrc` ueber `audio::speichereSongtext`: neben
 *   die Quelldatei (gleicher Name), bei eingebettetem Text nach der
 *   Einstellung „Save fetched lyrics". Die Suche findet sie ab dann von
 *   selbst (`.lrc` vor `.txt`).
 *
 * Gespeichert wird nur, wenn jede Textzeile eine Zeit hat: der LRC-Leser
 * verwirft Zeilen ohne Zeit, sobald eine andere eine traegt. Leerzeilen ohne
 * Zeit entfallen.
 *
 * Wechselt der Titel, waehrend ungespeicherte Zeiten da sind, bleibt die
 * Arbeit stehen (Hinweis im Panel) — sie wird nie stillschweigend verworfen.
 *
 * - **Track offset** (S88): verschiebt den ganzen Text dieses Titels, ohne
 *   Zeiten oder Datei anzufassen; liegt in `offsets.json`, wirkt sofort.
 *
 * - **Undo / Redo** (S88): jeder Schritt an den Zeiten laesst sich
 *   zuruecknehmen (Ctrl+Z / Ctrl+Y in der Liste), bis zum Laden oder Speichern.
 *
 * Nicht in dieser Stufe: Wort-Zeiten, automatisches Verteilen.
 ****************************************************************************************
 */

#pragma once

#include "PanelBase.hpp"

#include "audio/AudioEvents.hpp"
#include "visualizers/multieffect/LyricsModel.hpp"

#include <QString>

#include <atomic>
#include <cstdint>
#include <memory>
#include <thread>
#include <vector>

class QCheckBox;
class QLabel;
class QPushButton;
class QTableWidget;
class QTimer;

class LyricsSyncPanel : public PanelBase
{
    Q_OBJECT

public:
    explicit LyricsSyncPanel(ServiceContainer& services, QWidget* parent = nullptr);
    ~LyricsSyncPanel() override;

    /// Zur Zeile springen: genau auf ihre Zeit (`vorlaufMs` = 0, Knopf „▶" der
    /// Zeile) oder kurz davor (Doppelklick). false = die Zeile hat keine Zeit.
    bool springeZu(int reihe, int vorlaufMs);

    /// Spielzeit als Text: `1:23.4 / 3:45`.
    [[nodiscard]] static QString spielzeitText(std::int64_t posMs, int dauerMs);

    [[nodiscard]] int preferredArea() const override;

    /// Vorlauf beim Vorhoeren: so viele ms vor der Zeile setzt der Sprung an
    /// (Vorgabe; einstellbar seit S92, s. `setzeVorlaufMs`).
    static constexpr int kVorlaufMs = 1500;
    static constexpr int kVorlaufMaxMs = 10000;

    /// S92 (Wunsch Patrik): Vorlauf des Doppelklicks einstellen — Feld
    /// „Pre-roll" und Untermenue im Kontextmenue; gemerkt ueber den Neustart
    /// (QSettings `lyricsSync/prerollMs`). 0 = genau auf die Zeile.
    void setzeVorlaufMs(int ms);
    [[nodiscard]] int vorlaufMs() const { return m_vorlaufMs; }

    /// S92: zum Wort `wort` der markierten Zeile springen (`vorlaufMs` davor).
    /// false = das Wort hat keine Zeit.
    bool springeZuWort(int wort, int vorlaufMs);

    /// S92 (Wunsch Patrik): Kontextmenue der Zeilenliste — die Eintraege
    /// richten sich nach der Zeile unter dem Zeiger: mit Zeit (springen,
    /// verschieben, Zeit loeschen), ohne Zeit (zur letzten Zeile mit Zeit
    /// davor), mit Wort-Zeiten (Clear word times), Textzeile (Tap, Voice).
    /// Immer da: Pre-roll, Words, Follow playback, Undo / Redo. `reihe` < 0 =
    /// freie Flaeche (nur die allgemeinen Eintraege).
    void fuelleZeilenMenue(class QMenu& menu, int reihe);
    /// S92: Kontextmenue des Wort-Streifens fuer das Wort `wort` der
    /// markierten Zeile (springen, Tap, verschieben, Wort-Zeiten loeschen).
    void fuelleWortMenue(class QMenu& menu, int wort);
    /// Schritt der Knoepfe „−"/„+".
    static constexpr int kSchrittMs = 100;

    /// Text des Titels in die Liste nehmen (verwirft die Arbeitskopie).
    void lade(const TrackInfo& track, const lumi::multieffect::lyrics::Songtext& text);

    /// Der markierten Zeile die Position `posMs` geben und zur naechsten
    /// Textzeile ruecken. false = keine Zeile markiert.
    bool tippe(std::int64_t posMs);

    /// Zeiten verschieben: der markierten Zeilen, oder (`abHier`) aller Zeilen
    /// ab der ersten markierten. Zeilen ohne Zeit bleiben ohne.
    void verschiebe(int deltaMs, bool abHier);

    /// Zeit der markierten Zeilen loeschen.
    void loescheZeit();

    /// S90 (LB5, Duett): die Stimme der markierten Textzeilen setzen — Knoepfe
    /// „Voice 1 / 2 / Both" und die Tasten 1 / 2 / 3 in der Liste. Die Stimme
    /// bestimmt die Farbe der Zeile; gespeichert wird sie als Marke am
    /// Zeilenanfang (`v1:`, `v2:`, `v1+v2:`).
    void setzeStimme(lumi::multieffect::lyrics::Stimme stimme);

    /// S90 (LB5-2, Entscheide L6-1 a / L6-2 a): Wort-Zeiten klopfen. Mit dem
    /// Schalter „Words" zeigt ein Streifen unter der Liste die Worte der
    /// markierten Zeile; `tippe` gibt dann dem gewaehlten Wort die Zeit und
    /// rueckt zum naechsten — nach dem letzten zur naechsten Textzeile. Das
    /// erste Wort traegt die Zeit der Zeile. Gespeichert wird eine Zeile nur
    /// dann mit Wort-Zeiten, wenn ALLE ihre Worte eine haben.
    void setWortModus(bool an);
    [[nodiscard]] bool wortModus() const;
    /// Das Wort der markierten Zeile waehlen, das der naechste Tap trifft.
    void waehleWort(int index);
    [[nodiscard]] int gewaehltesWort() const { return m_wort; }
    /// Zeilen, die Wort-Zeiten haben, aber nicht fuer alle Worte.
    [[nodiscard]] int offeneWortZeilen() const;
    /// Die Wort-Zeiten der markierten Zeilen loeschen (die Zeit der Zeile bleibt).
    void loescheWortZeiten();

    /// Arbeitskopie als Songtext: nur Zeilen mit Zeit, nach Zeit geordnet.
    [[nodiscard]] lumi::multieffect::lyrics::Songtext alsSongtext() const;

    /// Textzeilen ohne Zeit (Leerzeilen zaehlen nicht).
    [[nodiscard]] int offeneZeilen() const;

    /// Wohin gespeichert wuerde; leer = es gibt nichts zu speichern.
    [[nodiscard]] QString zielPfad() const;

    /// Speichern; false = noch offene Zeilen, kein Ziel oder Schreibfehler.
    bool speichere();

    [[nodiscard]] bool geaendert() const { return m_geaendert; }

    /// S88: den letzten Schritt (Klopfen, Verschieben, Zeit loeschen, Zeit von
    /// Hand) zuruecknehmen bzw. wieder herstellen — Knoepfe und Ctrl+Z /
    /// Ctrl+Y in der Liste. Die Geschichte gilt fuer die Arbeitskopie: Laden
    /// und Speichern leeren sie. false = nichts zurueckzunehmen.
    bool rueckgaengig();
    bool wiederholen();

    /// S88: Versatz, der nur fuer den Titel des Panels gilt (`offsets.json`,
    /// `audio::setzeSongtextTitelVersatz`) — sofort wirksam und gespeichert,
    /// unabhaengig von „Save". Ohne Titel geschieht nichts.
    void setzeTitelVersatz(int ms);

    /// S91 (LB5-3, Entscheid L6-4 a): langsamer abspielen, damit sich Zeilen
    /// und Worte klopfen lassen — `prozent` 50..100 geht als Tempo an den
    /// Player (ueber die Abtastrate, der Ton wird tiefer). Die Zeiten bleiben
    /// Zeit im Titel: ein Tap bei halbem Tempo gilt bei vollem genauso.
    /// Entscheid T1 a: das Tempo gilt nur, solange das Panel sichtbar ist —
    /// verborgen spielt der Player mit 100 %, der Schieber behaelt seinen Wert
    /// und gilt wieder, sobald das Panel gezeigt wird. Ein Titelwechsel
    /// aendert nichts.
    void setzeTempo(int prozent);
    static constexpr int kTempoMin = 50;

    /// Benutzerordner ersetzen (Tests — der echte bleibt unberuehrt).
    void setBenutzerOrdner(const QString& ordner) { m_ordner = ordner; }

protected:
    void onActivate() override;
    void onDeactivate() override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void setupUI();
    void fuelleListe();
    void zeigeZeile(int reihe);
    void zeigeStand();
    void passeStandHoehe();  ///< Mindesthoehe der Statuszeile fuer die jetzige Breite
    void zeigePosition();
    void vorschau();
    void uebernimmLaufenden(bool erzwungen);
    [[nodiscard]] QList<int> markierteReihen() const;
    void zeigeTitelVersatz();
    void zeigeWorte();  ///< den Wort-Streifen aus der markierten Zeile fuellen
    /// Worte der Zeile fuer den Streifen: ihre Wort-Eintraege, sonst am
    /// Leerraum getrennt und ohne Zeit (-1).
    [[nodiscard]] static std::vector<lumi::multieffect::lyrics::Wort> worteVon(
        const lumi::multieffect::lyrics::Zeile& z);
    class QListWidget* m_wortListe = nullptr;
    QCheckBox* m_worteAn = nullptr;
    QPushButton* m_wortLeeren = nullptr;
    int m_wort = 0;  ///< Index des Worts, das der naechste Tap trifft
    /// Versatz, mit dem der Knoten rechnet: `[offset:]` der Datei + der des Titels.
    [[nodiscard]] std::int64_t versatz() const { return m_text.versatzMs + m_titelVersatzMs; }

    /// Ein Stand der Arbeitskopie fuer Rueckgaengig (S88).
    struct Stand
    {
        std::vector<lumi::multieffect::lyrics::Zeile> zeilen;
        bool geaendert = false;
        int reihe = -1;  ///< erste markierte Reihe — nach dem Zuruecknehmen wieder markiert
    };
    static constexpr std::size_t kGeschichteMax = 200;
    [[nodiscard]] Stand jetzigerStand() const;
    void merke(Stand vorher);           ///< den Stand VOR einer Aenderung festhalten
    void setzeStand(const Stand& s);
    std::vector<Stand> m_zurueck;
    std::vector<Stand> m_vor;
    lumi::multieffect::lyrics::Songtext m_geladen;  ///< der Text, wie er geladen wurde
    QPushButton* m_undo = nullptr;
    QPushButton* m_redo = nullptr;

    std::int64_t m_titelVersatzMs = 0;       ///< Versatz nur dieses Titels (S88)
    class QSpinBox* m_versatzFeld = nullptr;
    class QSlider* m_tempo = nullptr;        ///< Abspieltempo in Prozent (S91)
    QLabel* m_tempoText = nullptr;
    int m_vorlaufMs = kVorlaufMs;            ///< Vorlauf des Doppelklicks (S92)
    class QDoubleSpinBox* m_vorlauf = nullptr;
    class QSlider* m_leiste = nullptr;       ///< Sprungleiste ueber den ganzen Titel (S92)
    void wendeTempoAn();  ///< sichtbar: Wert des Schiebers an den Player; verborgen: 100 %
    TrackInfo m_track;
    lumi::multieffect::lyrics::Songtext m_text;  ///< Arbeitskopie, Reihenfolge der Datei
    bool m_geaendert = false;
    bool m_fuellt = false;  ///< Liste wird gerade gefuellt (itemChanged ignorieren)
    int m_laufend = -1;     ///< Reihe an der Abspielposition
    QString m_ordner;

    void ladeWellenform(const QString& pfad);

    QLabel* m_kopf = nullptr;
    QLabel* m_zeit = nullptr;            ///< laufende Spielzeit
    class LyricsWellenform* m_welle = nullptr;  ///< Ausschnitt um die Abspielposition
    QString m_wellePfad;                 ///< Datei, deren Huellkurve geladen ist / wird
    std::thread m_ladeFaden;             ///< dekodiert die Huellkurve
    std::shared_ptr<std::atomic<bool>> m_abbruch;
    QLabel* m_stand = nullptr;
    QTableWidget* m_liste = nullptr;
    QPushButton* m_tap = nullptr;
    QPushButton* m_speichern = nullptr;
    QCheckBox* m_folgen = nullptr;
    QTimer* m_uhr = nullptr;
};
