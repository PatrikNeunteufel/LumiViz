/**
 * @file KitFenster.hpp
 * @brief Loop: das Fenster **Kits** — Kits ansehen, kopieren, bearbeiten
 *        (docs/audio/Loop_Konzept.md §4l, Entscheide K7 a, K8 a, Patrik S123;
 *        Stufe 21c)
 *
 * Geoeffnet mit **Edit kits...** im Menue des Knopfs Kit im Drum-Editor.
 *
 * - **Kopf:** „Kits", rechts **Undo** und **Redo** (Ctrl+Z, Ctrl+Y) — sie
 *   gelten den Aenderungen am offenen Kit, bis es gespeichert oder ein anderes
 *   gewaehlt ist.
 * - **Links die Liste:** *Built-in kits* (mit Schloss) und *My kits*; darunter
 *   **Duplicate** (eine eigene Kopie, sofort geschrieben), **Rename** und
 *   **Delete** (nur eigene, sofort geschrieben). Rename aendert den Titel, die
 *   Datei behaelt ihren Namen — Loops, die das Kit nennen, finden es weiter.
 * - **Rechts das gewaehlte Kit:** Titel; bei einem mitgelieferten der Satz
 *   „Built-in, read-only — Duplicate makes your own copy". Die Tabelle: je
 *   Rolle **Role**, **Sound** (die Auswahl aus `KlangAuswahl`), **Volume**,
 *   **Pan**, ▶. Darunter **Add role ▾**, **Remove role**, rechts **Play all**.
 * - **Fuss:** „Used by: …" (die mitgelieferten Loops, die zuletzt geoeffneten
 *   und der offene), **Close**, **Save**.
 *
 * Das Fenster spielt nicht selbst: es bittet mit `anspielenVerlangt`, und wer
 * den Loop spielt, laesst die Schlaege hoeren.
 */
#pragma once

#include "loop/DrumKit.hpp"
#include "loop/Loop.hpp"

#include <QPointer>
#include <QString>
#include <QStringList>
#include <QWidget>

#include <functional>
#include <vector>

class QLabel;
class QMenu;
class QPushButton;
class QSlider;
class QTableWidget;
class QToolButton;
class QTreeWidget;
class QTreeWidgetItem;

namespace lumi::loop
{

class InstrumentPalette;
class KlangAuswahl;

class KitFenster : public QWidget
{
    Q_OBJECT

public:
    explicit KitFenster(QWidget* parent = nullptr);

    /// Die Liste neu lesen (eigene Kits koennen sich geaendert haben) und
    /// dieses Kit oeffnen; gibt es es nicht, das erste der Liste.
    void oeffne(const QString& nennung);
    /// Nur die Liste neu lesen (nach einem Import, S125); das offene Kit und
    /// seine ungesicherten Aenderungen bleiben.
    void liesListeNeu();
    /// Die Klaenge des Kits fuer die Auswahl Sound (`kit:<Name>`).
    void setKitKlaenge(const QStringList& klaenge);
    /// Der Loop, der gerade offen ist — er zaehlt bei „Used by" mit.
    void setOffenerLoop(const Loop& loop);

    // --- die Liste ------------------------------------------------------------
    /// Ein anderes Kit oeffnen; ungesicherte Aenderungen fallen weg (die Frage
    /// danach stellt die Liste). false = es gibt es nicht.
    bool waehle(const QString& nennung);
    /// Die Liste von oben nach unten: `[Built-in kits]`, `[My kits]`, je Kit
    /// seine Nennung, das offene mit `*` davor.
    [[nodiscard]] QStringList listenZeilen() const;
    /// **Duplicate:** das offene Kit, wie es dasteht, als eigenes Kit „<Titel>
    /// copy" schreiben und oeffnen. Leer = nicht geschrieben; sonst die Nennung.
    QString dupliziere();
    /// **Rename:** den Titel eines eigenen Kits aendern und gleich schreiben.
    /// false = kein eigenes offen, oder der Titel ist leer.
    bool benenne(const QString& titel);
    /// **Delete:** das offene eigene Kit loeschen und das erste der Liste
    /// oeffnen. false = kein eigenes offen.
    bool loesche();

    // --- das offene Kit -------------------------------------------------------
    [[nodiscard]] const DrumKit& kit() const { return m_kit; }
    [[nodiscard]] const QString& nennung() const { return m_kit.name; }
    /// Mitgeliefert: nichts zu aendern (Duplicate macht eine eigene Kopie).
    [[nodiscard]] bool schreibgeschuetzt() const;
    /// Weicht es von dem ab, was geoeffnet oder zuletzt gespeichert wurde?
    [[nodiscard]] bool geaendert() const { return !(m_kit == m_vorlage); }
    /// Die Zeile der Tabelle, die gewaehlt ist; -1 = keine.
    [[nodiscard]] int zeile() const;
    void waehleZeile(int zeile);

    // --- was die Bedienelemente ausloesen (auch fuer Tests) -------------------
    // Jede Aenderung ist ein Schritt fuer Undo; an einem mitgelieferten Kit geschieht nichts.
    /// Die Rolle in dieser Zeile spielt diesen Klang (wie `DrumEditor::setzeKlang`).
    bool setzeKlang(int zeile, const QString& klang);
    /// Die Rolle spielt dieses Instrument; Note nach seiner Art, Length 250 ms,
    /// wenn sie vorher keins spielte.
    bool setzeInstrument(int zeile, const QString& nennung);
    bool setzeLaut(int zeile, double laut);
    bool setzePan(int zeile, double pan);
    /// Was **Add role ▾** anbietet: je Art die Nummer hinter der hoechsten
    /// vergebenen (K5 a) — „kick", wenn es keinen gibt, sonst „kick 2".
    [[nodiscard]] QStringList freieRollen() const;
    /// Eine Rolle ohne Klang dazu, an ihren Platz in der Reihenfolge der
    /// Rollen; sie ist danach gewaehlt. -1 = schon da, unbekannt oder schreibgeschuetzt.
    int fuegeRolleHinzu(const QString& rolle);
    /// **Remove role.** false = keine solche Zeile, oder schreibgeschuetzt.
    bool entferneRolle(int zeile);
    bool rueckgaengig();
    bool wiederherstellen();
    [[nodiscard]] bool kannRueckgaengig() const { return !m_zurueck.empty(); }
    [[nodiscard]] bool kannWiederherstellen() const { return !m_vor.empty(); }
    /// **Save:** das eigene Kit schreiben. Leer = schreibgeschuetzt oder nicht
    /// zu schreiben; sonst die Nennung, die auch als `gespeichert` hinausgeht.
    QString speichere();

    /// ▶ einer Zeile: ihre Rolle einmal anspielen. false = sie spielt nichts.
    bool spieleRolle(int zeile);
    /// **Play all:** die Rollen nacheinander, je Schlag eine. false = keine spielt etwas.
    bool spieleAlle();

    /// Die Auswahl Sound einer Zeile, wie `KlangAuswahl::zeilen`.
    [[nodiscard]] QStringList klangZeilen(int zeile) const;
    /// Die Auswahl selbst (fuer Tests); nullptr = keine solche Zeile.
    [[nodiscard]] KlangAuswahl* klangAuswahl(int zeile) const;
    /// Die Zeile links im Fuss: „Used by: …".
    [[nodiscard]] QString nutzerZeile() const { return m_nutzerText; }
    /// Die Zeile unter dem Titel: leer bei einem eigenen Kit.
    [[nodiscard]] QString hinweisZeile() const;
    [[nodiscard]] QTreeWidget* liste() const { return m_liste; }
    [[nodiscard]] QTableWidget* tabelle() const { return m_tabelle; }
    /// Das Fenster Instruments, das Instrument... oeffnet; nullptr = noch nie.
    [[nodiscard]] InstrumentPalette* instrumentFenster() const { return m_palette; }
    /// **Instrument...** in der Auswahl einer Zeile: das Fenster Instruments
    /// fuer diese Rolle oeffnen. `zeigen` false fuellt es nur (fuer Tests).
    bool oeffnePalette(int zeile, bool zeigen = true);
    /// **Sample file...** ohne Dialog: kopiert (`user:<name>`) oder am Ort.
    bool nimmSampleDatei(int zeile, const QString& pfad, bool kopieren);

Q_SIGNALS:
    /// Diese Schlaege anspielen: je Spur einen, einen Schlag nach dem anderen.
    void anspielenVerlangt(const std::vector<lumi::loop::Spur>& spuren);
    /// Ein Ton eines Instruments (aus dem Fenster Instruments).
    void instrumentTonVerlangt(const QString& nennung, int hoehe);
    /// Ein eigenes Kit ist geschrieben (Save, Rename) — wer es nennt, zeigt es neu.
    void gespeichert(const QString& nennung);
    /// Eigene Kits sind dazugekommen oder weggefallen (Duplicate, Delete).
    void kitsGeaendert();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    /// Eine Zeile der Tabelle.
    struct Zeile
    {
        QLabel* rolle = nullptr;
        KlangAuswahl* klang = nullptr;
        QSlider* laut = nullptr;
        QSlider* pan = nullptr;
        QToolButton* spielen = nullptr;
    };
    void setupUI();
    void baueListe();
    /// Titel, Hinweis, Tabelle, Knoepfe aus `m_kit`.
    void zeigeAlles();
    void zeigeTabelle();
    void zeigeKnoepfe();
    /// Eine Aenderung am offenen Kit: merken fuer Undo, ausfuehren, zeigen.
    bool aendere(const std::function<void(DrumKit&)>& tu);
    bool aendereZeile(int zeile, const std::function<void(KitKlang&)>& tu);
    void klangGewaehlt(int zeile, const QString& wahl);
    void frageNachSampleDatei(int zeile);
    void rechneNutzer();
    /// Vor dem Wechsel: ungesicherte Aenderungen speichern, verwerfen oder bleiben? false = bleiben.
    bool darfVerlassen();
    void frageUndBenenne();
    void frageUndLoesche();
    /// Das Menue von Add role: beim Aufklappen gefuellt, mit den Rollen aus `freieRollen`.
    void fuelleRollenMenue(QMenu* menue);
    /// Eine Rolle als Schlag einer Spur.
    [[nodiscard]] static Spur alsSpur(const KitKlang& klang);

    DrumKit m_kit;
    DrumKit m_vorlage;  ///< wie geoeffnet oder zuletzt gespeichert
    std::vector<DrumKit> m_zurueck;
    std::vector<DrumKit> m_vor;
    QStringList m_kitKlaenge;
    Loop m_offen;
    bool m_hatOffen = false;
    QString m_nutzerText;
    std::vector<Zeile> m_zeilen;
    QPointer<InstrumentPalette> m_palette;
    int m_paletteZeile = -1;

    QToolButton* m_undo = nullptr;
    QToolButton* m_redo = nullptr;
    QTreeWidget* m_liste = nullptr;
    QPushButton* m_duplizieren = nullptr;
    QPushButton* m_umbenennen = nullptr;
    QPushButton* m_loeschen = nullptr;
    QLabel* m_titel = nullptr;
    QLabel* m_hinweis = nullptr;
    QTableWidget* m_tabelle = nullptr;
    QPushButton* m_rolleNeu = nullptr;
    QPushButton* m_rolleWeg = nullptr;
    QPushButton* m_alle = nullptr;
    QLabel* m_nutzer = nullptr;
    QPushButton* m_schliessen = nullptr;
    QPushButton* m_sichern = nullptr;
    QPushButton* m_publish = nullptr;
};

}  // namespace lumi::loop
