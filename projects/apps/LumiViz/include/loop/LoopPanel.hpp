/**
 * @file LoopPanel.hpp
 * @brief Loop: das Panel — das Arrangement eines Loops
 *        (docs/audio/Loop_Konzept.md §4c, Stufe 6a; Entscheide L4 a und L7 a)
 *
 * **Datei-Zeile:** Name mit Stern, New, Load…, Save, Save As…. **Abspiel-Zeile:**
 * Play / Stop, BPM, eine Zeile mit Kanaelen, Takten und Dauer, die Stelle als
 * Bar und Step, Fit. **Achse:** die `Taktachse` mit ihren Rollbalken — Kanaele
 * als Zeilen, Clips als Bloecke. **Clip-Zeile:** Basis, Takte und Kopien des
 * angeklickten Clips.
 *
 * Das Panel haelt den Loop; jede Aenderung laeuft ueber den CommandBus der App
 * (Undo / Redo) und gilt beim Spielen ab dem naechsten Block. Die Editoren der
 * Muster (Stufe 6b) gibt es noch nicht.
 *
 * Play spielt den Loop allein, von vorn, im Kreis; der Player schweigt so
 * lange, die Visuals reagieren auf den Loop. Fuehrt der Composer den Ton, ist
 * Play gesperrt und sagt, warum (L7 a); spielte der Loop gerade, haelt er an.
 */
#pragma once

#include "UI/panels/PanelBase.hpp"
#include "loop/Bearbeiten.hpp"  // ClipOrt
#include "loop/Loop.hpp"
#include "loop/Spieler.hpp"
#include "services/IEventBus.hpp"

#include <QPointer>
#include <QString>
#include <QStringList>

#include <functional>
#include <optional>

class QDoubleSpinBox;
class QLabel;
class QPushButton;
class QTimer;

namespace lumi::loop
{

class ClipEffektFenster;
class DrumEditor;
class InstrumentEditor;
class Taktachse;

/// Die Antwort auf die Rueckfrage bei Ungespeichertem.
enum class Rueckfrage
{
    Speichern,
    Verwerfen,
    Bleiben
};

class LoopPanel : public PanelBase
{
    Q_OBJECT

public:
    explicit LoopPanel(ServiceContainer& services, QWidget* parent = nullptr);
    ~LoopPanel() override;

    /// Die Achse rollt selbst (viele Kanaele).
    [[nodiscard]] bool scrollsItself() const override { return true; }
    [[nodiscard]] int preferredArea() const override { return Qt::BottomDockWidgetArea; }
    /// Breit und flach — nicht das hohe Format der uebrigen Panels.
    [[nodiscard]] QSize sizeHint() const override { return {760, 260}; }

    /// Ein leerer Loop mit einem Kanal, noch ohne Datei.
    void neu();
    /// Eine `.lvloop` laden. false = die Datei ist kein Loop; es bleibt alles,
    /// wie es war. Spielt gerade ein Loop, spielt der neue weiter.
    bool lade(const QString& pfad);
    /// Eine `.lvloop` von aussen oeffnen (Doppelklick im Audio Browser,
    /// `LoadLoopEvent`): erst die Rueckfrage zu Ungespeichertem, dann laden —
    /// und das Panel kommt nach vorn. false = abgebrochen oder kein Loop (das
    /// steht dann im Bericht des Panels).
    bool oeffne(const QString& pfad);
    /// Den Loop in die Datei schreiben; sie ist danach die Datei des Panels.
    bool speichere(const QString& pfad);
    [[nodiscard]] const QString& datei() const { return m_datei; }
    [[nodiscard]] bool geladen() const { return m_geladen; }
    /// Gibt es Aenderungen, die nicht in der Datei stehen?
    [[nodiscard]] bool ungespeichert() const { return m_geladen && !(m_loop == m_gespeichert); }
    /// Bevor der Loop des Panels verloren ginge — New, Load, das Beenden der App
    /// (`loop/Offen.hpp`, `darfBeenden`): gibt es Ungespeichertes, fragt das
    /// Panel mit Save, Discard und Cancel, wie der Composer. false = bleiben
    /// (Cancel, oder das Speichern gelang nicht). `frage` ersetzt den Dialog
    /// (fuer Tests ohne Fenster).
    [[nodiscard]] bool darfErsetzen(const QString& titel, const std::function<Rueckfrage()>& frage = {});

    /// Der Loop, den das Panel zeigt und spielt.
    [[nodiscard]] const Loop& loop() const { return m_loop; }
    /// Eine Aenderung ausfuehren — ueber den CommandBus der App (Undo / Redo),
    /// ohne ihn direkt.
    void bearbeite(const Loop& neu, const QString& was);
    /// Die Fassung setzen, ohne Undo (das tun die Befehle des CommandBus).
    void setzeLoop(const Loop& loop);
    /// Die Tags des Loops setzen (Titel, Kuenstler … — dasselbe Schema wie bei
    /// einem Song), ueber Undo.
    void setzeTags(const LoopTags& tags);
    /// Zaehlt jedes New und Load: ein Schritt fuer Undo gilt nur fuer die
    /// Ladung, in der er entstand.
    [[nodiscard]] int ladung() const { return m_ladung; }

    /// Den geladenen Loop von vorn spielen. false = nichts geladen, der
    /// Composer hat den Ton, oder es gibt keine Audio-Engine.
    bool spiele();
    void halte();
    [[nodiscard]] const Spieler& spieler() const { return m_spieler; }
    [[nodiscard]] Taktachse* taktachse() const { return m_achse; }

    // --- der Drum-Editor (Stufe 6b) -------------------------------------------
    /// Den Editor dieser Basis zeigen (er entsteht beim ersten Mal). Es gibt
    /// ein Editor-Fenster: eine andere Basis wechselt in ihm. false = die
    /// Basis gibt es nicht. `zeigen` false fuellt ihn nur (fuer Tests ohne Fenster).
    bool oeffneEditor(const QString& muster, bool zeigen = true);
    /// Das Fenster des Editors; nullptr, solange noch keine Basis geoeffnet wurde.
    [[nodiscard]] DrumEditor* editor() const;
    /// Das Fenster des Instrument-Editors (Stufe 9b) — `oeffneEditor` waehlt
    /// nach der Art der Basis; nullptr, solange noch kein Instrument-Muster
    /// geoeffnet wurde. Je Art gibt es ein Fenster.
    [[nodiscard]] InstrumentEditor* instrumentEditor() const;
    /// Einen Ton des Instruments dieser Basis anspielen, einen Schlag lang
    /// (ein Klick auf eine Taste des Instrument-Editors). Spielt gerade der
    /// Loop oder ein Muster, geschieht nichts — man hoert es schon. false =
    /// nicht angespielt.
    bool spieleTon(const QString& muster, int hoehe);
    /// Dasselbe mit einem Instrument, das beim Namen oder beim Pfad seiner
    /// Datei genannt wird — so spielt der Instrument-Creator an, was er gerade
    /// einstellt (Stufe 10b).
    /// `klingtMs` groesser als null („Repeat note" im Creator): so lang darf der
    /// Ton im Ganzen klingen, statt einen Schlag und 600 ms.
    bool spieleInstrumentTon(const QString& nennung, int hoehe, int klingtMs = 0);
    /// Einen Schlag einer Spur eines Drum-Musters anspielen, mit Lautstaerke
    /// und Panorama der Spur (Play im Feld hinter „…" des Drum-Editors, Stufe
    /// 11c): ihr Klang — oder der Ton ihres Instruments, mit Note und Length.
    /// false = die Spur gibt es nicht, sie spielt nichts, oder der Loop spielt.
    bool spieleSpur(const QString& muster, int spur);
    /// Je Spur einen Schlag, einen Schlag (des Loops) nach dem anderen — mit
    /// Lautstaerke und Panorama der Spuren (▶ und Play all im Fenster Kits,
    /// Stufe 21c). false = keine Spur spielt etwas, oder der Loop spielt.
    bool spieleSchlaege(const std::vector<Spur>& spuren);
    /// B4 (S124): die Stelle, an der Play beginnt, in Schritten; spielt der Loop, springt er dorthin.
    void setzeStartstelle(int schritt);
    /// Der im Loop markierte Clip; leer = keiner (E1 a: der Bezugsclip der Editoren).
    [[nodiscard]] std::optional<ClipOrt> markierterClip() const;
    [[nodiscard]] int startstelle() const { return m_startSchritt; }
    /// Wird gerade ein Ton angespielt?
    [[nodiscard]] bool hoertVor() const { return m_vorhoeren != 0; }
    /// „Play pattern": nur dieses Muster spielen, im Kreis — damit man hoert,
    /// was man gerade aendert. Play im Panel spielt wieder den ganzen Loop.
    bool spieleMuster(const QString& muster);
    /// Das Muster, das gerade allein spielt; leer = keins.
    [[nodiscard]] const QString& soloMuster() const { return m_solo; }

    // --- die Effekte am Clip (Stufe 12d) --------------------------------------
    /// Das Fenster „Clip effects" fuer die markierten Clips zeigen (es
    /// entsteht beim ersten Mal und folgt danach den Marken). false = kein
    /// Loop geladen. `zeigen` false fuellt es nur (fuer Tests ohne Fenster).
    bool oeffneEffekte(bool zeigen = true);
    /// Das Fenster; nullptr, solange es noch nie geoeffnet wurde.
    [[nodiscard]] ClipEffektFenster* effektFenster() const;

private:
    void setupUI();
    void frageNachDatei();
    void frageNachTags();
    /// Datei und Fassung dem Rest des Programms melden (`loop/Offen.hpp`).
    void meldeStand();
    /// false = abgebrochen oder nicht geschrieben (dann mit Meldung).
    bool speichereMitDialog(bool unter);
    void zeigeDatei();
    void zeigeClip();
    void zeigeStand();
    void beimTakt();
    /// Das Anspielen beenden (falls eines laeuft): der Spieler haelt und bekommt den Loop zurueck.
    void beendeVorhoeren();
    /// Einen Ton einen Schlag lang spielen: ein Loop aus einem Muster wie
    /// `vorlage` mit dieser einen Note.
    bool spieleVor(const Muster& vorlage, int hoehe, int klingtMs = 0);
    /// Einen Loop aus einem Muster „Preview" einmal anspielen; nach `dauerMs`
    /// haelt der Spieler wieder und bekommt den Loop des Panels zurueck.
    bool spieleAllein(const Muster& muster, int dauerMs);
    /// Was der Spieler spielt: der ganze Loop — oder, bei „Play pattern", ein
    /// Loop aus einem Kanal mit einem Clip dieses Musters.
    [[nodiscard]] Loop spielFassung();

    /// Permanent: ein Loop aus dem Audio Browser kommt auch an, wenn das Panel nie sichtbar war.
    IEventBus::SubscriberHandle m_ladeSubscription;
    IEventBus::SubscriberHandle m_vorhoerSubscription;  ///< ▶ im Dialog Import package (Publish, S125)
    IEventBus::SubscriberHandle m_importSubscription;   ///< nach einem Import neu lesen (S125)
    QPointer<DrumEditor> m_editor;
    QPointer<InstrumentEditor> m_instrumentEditor;
    QPointer<ClipEffektFenster> m_effekte;
    int m_vorhoeren = 0;    ///< die Nummer des Tons, der gerade angespielt wird; 0 = keiner
    int m_vorhoerNr = 0;
    QString m_solo;        ///< das Muster, das allein spielt; leer = der ganze Loop
    float m_spitze = 0.0f;  ///< der groesste Betrag der Summe seit Play oder der letzten Aenderung

    Spieler m_spieler;
    Loop m_loop;         ///< was das Panel zeigt und spielt
    Loop m_gespeichert;  ///< was in der Datei steht (oder beim New entstand)
    QString m_datei;     ///< leer = noch nie gespeichert
    bool m_geladen = false;
    int m_ladung = 0;
    QStringList m_ladeBericht;  ///< was beim Lesen der Datei auffiel

    QLabel* m_dateiIcon = nullptr;  ///< das Datei-Icon der Loops vor dem Namen (S120, A2 a)
    QLabel* m_dateiZeile = nullptr;
    QPushButton* m_neu = nullptr;
    QPushButton* m_laden = nullptr;
    QPushButton* m_speichern = nullptr;
    QPushButton* m_speichernUnter = nullptr;
    QPushButton* m_publish = nullptr;
    QPushButton* m_tags = nullptr;
    QPushButton* m_play = nullptr;
    QDoubleSpinBox* m_bpm = nullptr;
    QLabel* m_info = nullptr;
    QLabel* m_stelle = nullptr;
    QPushButton* m_fit = nullptr;
    Taktachse* m_achse = nullptr;
    QLabel* m_clipZeile = nullptr;
    QLabel* m_hinweis = nullptr;
    QLabel* m_bericht = nullptr;
    QTimer* m_takt = nullptr;
    int m_startSchritt = 0;  ///< B4: wo Play beginnt
};

}  // namespace lumi::loop
