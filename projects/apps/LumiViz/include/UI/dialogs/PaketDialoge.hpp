/**
 ****************************************************************************************
 * @file   PaketDialoge.hpp
 * @brief  Publish, Stufe P-A: die Dialoge **Publish <art>** und **Import package**
 *         (docs/core-services/Publish_Konzept.md §4b, §4c, §4d; Entscheide P7 a,
 *         P8 a, P6 a, Patrik S125)
 *
 * @details
 *  - `PublishDialog`: der Paketname, ein Baum des Inhalts (das Hauptding, die
 *    Zugehoerigen nach Art mit Herkunft und kuenftigem Namen, grau und
 *    eingeklappt das Mitgelieferte, in der Hinweisfarbe was fehlt), der Haken
 *    Include media files (bis Stufe P-B aus und grau), Save to, die Summe.
 *  - `ImportPaketDialog`: Kopf, Contents (wohin jede Datei kommt), Same names
 *    (je gleichem Namen eine Zeile mit Vergleich und der Wahl Keep both /
 *    Replace mine / Use mine, dazu All:), Notes, Save to (nur fuer einen
 *    Loop), der Haken Open after import.
 *
 * Seit Stufe 30i (FP2 a, FP3 a) die Wellenformen eingefrorener Clips: im Publish-Dialog
 * die Gruppe *Frozen clips* mit einer Zeile je Clip und der Haken *Include frozen clips*;
 * fehlende rechnen ab dem Oeffnen, **Publish** wartet darauf. Im Import-Dialog eine Zeile
 * in Contents, ein Hinweis und die Zahl in der Meldung.
 *
 * Die Regeln stehen in `paket/`; hier ist nur die Oberflaeche. Jeder Dialog
 * laesst sich ohne gezeigtes Fenster bedienen (`fuehreAus`).
 ****************************************************************************************
 */

#pragma once

#include "loop/Loop.hpp"
#include "loop/Tonauftrag.hpp"
#include "paket/Import.hpp"
#include "paket/Packen.hpp"
#include "paket/Sammler.hpp"

#include <QDialog>
#include <QTemporaryDir>

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <vector>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSettings;
class QTimer;
class QTreeWidget;
class QTreeWidgetItem;
class QVBoxLayout;
class IAudioEngine;
class IEventBus;

namespace lumi::loop
{
class FreezeAuftrag;
}

namespace lumi::ui
{

/// „loop", „instrument", „kit", „shape", „sample" — wie die Art in Titeln steht.
[[nodiscard]] QString paketArtWort(lumi::paket::Art art);

/// Was Publish und Import fuer eingefrorene Clips brauchen (Stufe 30i): die Ausgaberate,
/// mit der Abdruecke gerechnet werden, und fuer Publish den Weg zum Dekodieren, wenn eine
/// Wellenform erst zu rechnen ist (FP1 a). `rate` 0 = ohne Wellenformen.
struct FreezeTon
{
    int rate = 0;
    lumi::loop::Dekodierer dekodierer;
};
/// Wie das Loop-Panel: die Rate der Engine (ohne eine: 44100), dekodiert ueber sie.
[[nodiscard]] FreezeTon freezeTonVon(IAudioEngine* engine);

class PublishDialog : public QDialog
{
    Q_OBJECT

public:
    /// `fund` muss `ok` sein. `einstellungen` = nullptr: die der App. Mit `ton.rate` zeigt
    /// ein Loop seine eingefrorenen Clips und rechnet fehlende Wellenformen (FP1 a, FP2 a).
    explicit PublishDialog(lumi::paket::Fund fund, QWidget* parent = nullptr, QSettings* einstellungen = nullptr,
                           FreezeTon ton = {});
    ~PublishDialog() override;

    [[nodiscard]] const lumi::paket::Fund& fund() const { return m_fund; }
    [[nodiscard]] QTreeWidget* inhaltsBaum() const { return m_baum; }
    void setzeName(const QString& dateiname);
    void setzeZielOrdner(const QString& ordner);
    [[nodiscard]] QString zielOrdner() const;
    /// Der ganze Pfad des Pakets; endet immer auf `.lvdata`.
    [[nodiscard]] QString paketDatei() const;
    /// Die Zeile unten links („8 files, 1.2 KB").
    [[nodiscard]] QString summe() const;

    /// Die Gruppe *Frozen clips* im Baum; nullptr = der Loop hat keine eingefrorenen Clips.
    [[nodiscard]] QTreeWidgetItem* frozenGruppe() const { return m_frozenGruppe; }
    /// Der Haken *Include frozen clips*; nullptr = keine eingefrorenen Clips.
    [[nodiscard]] QCheckBox* frozenHaken() const { return m_frozenHaken; }
    [[nodiscard]] const std::vector<lumi::paket::FrozenTeil>& frozenTeile() const { return m_teile; }
    /// Rechnet noch eine Wellenform (bis die Uhr das Ende sieht)?
    [[nodiscard]] bool rechnet() const { return m_freeze != nullptr; }
    /// Hat **Publish** auf das Rechnen zu warten begonnen?
    [[nodiscard]] bool wartet() const { return m_wartet; }
    /// Fuer Tests: bis das Rechnen fertig ist, dann wie die Uhr (Zeilen, Summe, ein wartendes Publish).
    void warteAufFreeze();

    /// Packen, ohne Rueckfrage und ohne Meldung, mit den Wellenformen, die vorliegen.
    /// Der Knopf **Publish** wartet erst auf das Rechnen, fragt, ob eine vorhandene Datei
    /// ersetzt werden soll, und meldet danach (ueber `fuehreDialogAus`).
    lumi::paket::PackErgebnis fuehreAus();

private:
    void baueBaum();
    void fuelleFrozenZeilen();
    void aktualisiereSumme();
    void pruefeFreeze();
    [[nodiscard]] bool frozenMitnehmen() const;
    void veroeffentliche();

    lumi::paket::Fund m_fund;
    QSettings* m_einstellungen = nullptr;
    FreezeTon m_ton;
    QLineEdit* m_name = nullptr;
    QTreeWidget* m_baum = nullptr;
    QCheckBox* m_medien = nullptr;
    QLineEdit* m_ziel = nullptr;
    QLabel* m_summe = nullptr;
    QPushButton* m_publish = nullptr;

    // Stufe 30i (FP2 a)
    std::optional<lumi::loop::Loop> m_loop;  ///< der gespeicherte, gerichtet — nur mit eingefrorenen Clips
    std::vector<lumi::paket::FrozenTeil> m_teile;
    std::vector<QString> m_rechenzeit;  ///< je Teil „3.2 s" oder leer
    QTreeWidgetItem* m_frozenGruppe = nullptr;
    QCheckBox* m_frozenHaken = nullptr;
    std::unique_ptr<lumi::loop::FreezeAuftrag> m_freeze;
    std::vector<lumi::loop::ClipOrt> m_freezeOrte;  ///< was der Auftrag rechnet
    QTimer* m_uhr = nullptr;
    bool m_wartet = false;
};

class ImportPaketDialog : public QDialog
{
    Q_OBJECT

public:
    /// Oeffnet `datei` und haelt sie gegen den Benutzerordner. Laesst sie sich
    /// nicht lesen, ist `inhalt().ok` false und **Import** aus. `rate`: die Ausgaberate,
    /// mit der der Import Wellenformen prueft (FP3 a); 0 = keine ablegen.
    explicit ImportPaketDialog(const QString& datei, QWidget* parent = nullptr, QSettings* einstellungen = nullptr,
                               int rate = 0);

    /// Die Zeile der Meldung nach dem Import zu eingefrorenen Clips („Frozen clips: 2 of 3
    /// ready to play — 1 will be computed here"); leer = der Loop hat keine.
    [[nodiscard]] static QString frozenMeldung(const lumi::paket::PaketErgebnis& ergebnis);

    [[nodiscard]] const lumi::paket::PaketInhalt& inhalt() const { return m_inhalt; }
    [[nodiscard]] QTreeWidget* inhaltsListe() const { return m_inhaltListe; }
    [[nodiscard]] QTreeWidget* gleicheNamen() const { return m_gleiche; }
    /// Die Zeilen unter Notes, als Text.
    [[nodiscard]] QStringList hinweise() const;

    /// Die Wahl einer Zeile unter Same names (`pfad` wie in `PaketDatei`).
    void setzeWahl(const QString& pfad, lumi::core::datensicherung::Wahl wahl);
    [[nodiscard]] lumi::core::datensicherung::Wahl wahl(const QString& pfad) const;
    /// Das Menue **All:**.
    void setzeAlle(lumi::core::datensicherung::Wahl wahl);
    /// Die Zeile unter Same names („Keep both renames 2 files; …").
    [[nodiscard]] QString summe() const;

    void setzeZiel(const QString& pfad);
    [[nodiscard]] QString ziel() const;
    void setzeOeffnen(bool an);
    [[nodiscard]] bool oeffnen() const;

    /// Importieren, ohne Meldung. Der Knopf **Import** ruft das, meldet und
    /// sendet danach `oeffnenVerlangt`, wenn der Haken sitzt.
    lumi::paket::PaketErgebnis fuehreAus();

signals:
    /// ▶ in Same names: die Datei anspielen — die eigene oder eine Kopie der
    /// aus dem Paket in einem Wegwerf-Ordner. `art` ist `paket::Art`.
    void anspielenVerlangt(int art, const QString& datei);
    /// Nach dem Import, wenn **Open … after import** sitzt: das Hauptding oeffnen.
    void oeffnenVerlangt(int art, const QString& datei);

private:
    void baueKopf(QVBoxLayout* aussen, const QString& datei);
    void baueGleiche();
    void aktualisiereSumme();
    void spieleAn(const lumi::paket::PaketDatei& d, bool eigene);
    void importiere();

    QString m_datei;
    QSettings* m_einstellungen = nullptr;
    int m_rate = 0;
    lumi::paket::PaketInhalt m_inhalt;
    QTreeWidget* m_inhaltListe = nullptr;
    QTreeWidget* m_gleiche = nullptr;
    std::map<QString, QComboBox*> m_wahl;  ///< je Pfad im Paket
    QComboBox* m_alle = nullptr;
    QLabel* m_summe = nullptr;
    QStringList m_hinweise;
    QLineEdit* m_ziel = nullptr;
    QCheckBox* m_oeffnen = nullptr;
    QPushButton* m_import = nullptr;
    std::unique_ptr<QTemporaryDir> m_vorhoeren;  ///< Kopien aus dem Paket zum Anspielen
};

/// Die Knoepfe **Publish...**: den Dialog fuer `datei` zeigen (nach einem
/// Fehler eine Meldung). `ton`: fuer die Wellenformen eingefrorener Clips (Stufe 30i).
void zeigePublish(QWidget* parent, const QString& datei, const FreezeTon& ton = {});
/// File → **Publish a file...**: nach einer Datei der Loop-Familie fragen, dann `zeigePublish`.
void zeigePublishDatei(QWidget* parent, const FreezeTon& ton = {});
/// **Publish...** in einem Fenster (Publish_Konzept.md §5e, von Claude gesetzt):
/// ohne eigene Datei (mitgeliefert, neu) eine Meldung; mit ungespeicherten
/// Aenderungen die Frage, ob die gespeicherte Fassung gehen soll — gibt es
/// `speichere`, bietet sie Speichern an. Dann `zeigePublish`.
void veroeffentlicheAusFenster(QWidget* parent, lumi::paket::Art art, const QString& datei, bool ungespeichert,
                               const std::function<QString()>& speichere = {}, const FreezeTon& ton = {});
/// Eine `.lvdata` importieren: ein Paket im Dialog Import package, eine
/// Datensicherung im Dialog Import data (P2 a, P8 a). `datei` leer = erst fragen.
/// Mit `bus` spielen ▶ und das Oeffnen nach dem Import ueber das Panel Loop
/// (`PreviewSoundFileEvent`, `LoadLoopEvent`). `rate` wie bei `ImportPaketDialog`.
void zeigeImportPaket(QWidget* parent, const QString& datei = {}, IEventBus* bus = nullptr, int rate = 0);

}  // namespace lumi::ui
