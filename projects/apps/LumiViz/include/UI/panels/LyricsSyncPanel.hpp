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
 * - **Mitklopfen:** „Tap" (Knopf, Enter oder T in der Liste) gibt der
 *   markierten Zeile die Abspielposition und rueckt zur naechsten Textzeile.
 * - **Nachstellen:** markierte Zeilen ±100 ms, „ab hier" ±100 ms, Zeit
 *   loeschen, Zeit von Hand eintragen (F2 in der Spalte Time).
 * - **Vorhoeren:** Doppelklick auf eine Zeile springt im Titel kurz davor.
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
 * Nicht in dieser Stufe: Wort-Zeiten, automatisches Verteilen.
 ****************************************************************************************
 */

#pragma once

#include "PanelBase.hpp"

#include "audio/AudioEvents.hpp"
#include "visualizers/multieffect/LyricsModel.hpp"

#include <QString>

#include <cstdint>

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
    ~LyricsSyncPanel() override = default;

    [[nodiscard]] int preferredArea() const override;

    /// Vorlauf beim Vorhoeren: so viele ms vor der Zeile setzt der Sprung an.
    static constexpr int kVorlaufMs = 1500;
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

    /// Arbeitskopie als Songtext: nur Zeilen mit Zeit, nach Zeit geordnet.
    [[nodiscard]] lumi::multieffect::lyrics::Songtext alsSongtext() const;

    /// Textzeilen ohne Zeit (Leerzeilen zaehlen nicht).
    [[nodiscard]] int offeneZeilen() const;

    /// Wohin gespeichert wuerde; leer = es gibt nichts zu speichern.
    [[nodiscard]] QString zielPfad() const;

    /// Speichern; false = noch offene Zeilen, kein Ziel oder Schreibfehler.
    bool speichere();

    [[nodiscard]] bool geaendert() const { return m_geaendert; }

    /// Benutzerordner ersetzen (Tests — der echte bleibt unberuehrt).
    void setBenutzerOrdner(const QString& ordner) { m_ordner = ordner; }

protected:
    void onActivate() override;
    void onDeactivate() override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void setupUI();
    void fuelleListe();
    void zeigeZeile(int reihe);
    void zeigeStand();
    void zeigePosition();
    void vorschau();
    void uebernimmLaufenden(bool erzwungen);
    [[nodiscard]] QList<int> markierteReihen() const;

    TrackInfo m_track;
    lumi::multieffect::lyrics::Songtext m_text;  ///< Arbeitskopie, Reihenfolge der Datei
    bool m_geaendert = false;
    bool m_fuellt = false;  ///< Liste wird gerade gefuellt (itemChanged ignorieren)
    int m_laufend = -1;     ///< Reihe an der Abspielposition
    QString m_ordner;

    QLabel* m_kopf = nullptr;
    QLabel* m_stand = nullptr;
    QTableWidget* m_liste = nullptr;
    QPushButton* m_tap = nullptr;
    QPushButton* m_speichern = nullptr;
    QCheckBox* m_folgen = nullptr;
    QTimer* m_uhr = nullptr;
};
