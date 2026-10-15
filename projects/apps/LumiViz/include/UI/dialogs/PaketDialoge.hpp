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
 * Die Regeln stehen in `paket/`; hier ist nur die Oberflaeche. Jeder Dialog
 * laesst sich ohne gezeigtes Fenster bedienen (`fuehreAus`).
 ****************************************************************************************
 */

#pragma once

#include "paket/Import.hpp"
#include "paket/Packen.hpp"
#include "paket/Sammler.hpp"

#include <QDialog>
#include <QTemporaryDir>

#include <functional>
#include <map>
#include <memory>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSettings;
class QTreeWidget;
class QTreeWidgetItem;
class QVBoxLayout;
class IEventBus;

namespace lumi::ui
{

/// „loop", „instrument", „kit", „shape", „sample" — wie die Art in Titeln steht.
[[nodiscard]] QString paketArtWort(lumi::paket::Art art);

class PublishDialog : public QDialog
{
    Q_OBJECT

public:
    /// `fund` muss `ok` sein. `einstellungen` = nullptr: die der App.
    explicit PublishDialog(lumi::paket::Fund fund, QWidget* parent = nullptr, QSettings* einstellungen = nullptr);

    [[nodiscard]] const lumi::paket::Fund& fund() const { return m_fund; }
    [[nodiscard]] QTreeWidget* inhaltsBaum() const { return m_baum; }
    void setzeName(const QString& dateiname);
    void setzeZielOrdner(const QString& ordner);
    [[nodiscard]] QString zielOrdner() const;
    /// Der ganze Pfad des Pakets; endet immer auf `.lvdata`.
    [[nodiscard]] QString paketDatei() const;
    /// Die Zeile unten links („8 files, 1.2 KB").
    [[nodiscard]] QString summe() const;

    /// Packen, ohne Rueckfrage und ohne Meldung. Der Knopf **Publish** fragt
    /// vorher, ob eine vorhandene Datei ersetzt werden soll, und meldet danach.
    lumi::paket::PackErgebnis fuehreAus();

private:
    void baueBaum();
    void veroeffentliche();

    lumi::paket::Fund m_fund;
    QSettings* m_einstellungen = nullptr;
    QLineEdit* m_name = nullptr;
    QTreeWidget* m_baum = nullptr;
    QCheckBox* m_medien = nullptr;
    QLineEdit* m_ziel = nullptr;
    QLabel* m_summe = nullptr;
    QPushButton* m_publish = nullptr;
};

class ImportPaketDialog : public QDialog
{
    Q_OBJECT

public:
    /// Oeffnet `datei` und haelt sie gegen den Benutzerordner. Laesst sie sich
    /// nicht lesen, ist `inhalt().ok` false und **Import** aus.
    explicit ImportPaketDialog(const QString& datei, QWidget* parent = nullptr, QSettings* einstellungen = nullptr);

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
/// Fehler eine Meldung).
void zeigePublish(QWidget* parent, const QString& datei);
/// File → **Publish a file...**: nach einer Datei der Loop-Familie fragen, dann `zeigePublish`.
void zeigePublishDatei(QWidget* parent);
/// **Publish...** in einem Fenster (Publish_Konzept.md §5e, von Claude gesetzt):
/// ohne eigene Datei (mitgeliefert, neu) eine Meldung; mit ungespeicherten
/// Aenderungen die Frage, ob die gespeicherte Fassung gehen soll — gibt es
/// `speichere`, bietet sie Speichern an. Dann `zeigePublish`.
void veroeffentlicheAusFenster(QWidget* parent, lumi::paket::Art art, const QString& datei, bool ungespeichert,
                               const std::function<QString()>& speichere = {});
/// Eine `.lvdata` importieren: ein Paket im Dialog Import package, eine
/// Datensicherung im Dialog Import data (P2 a, P8 a). `datei` leer = erst fragen.
/// Mit `bus` spielen ▶ und das Oeffnen nach dem Import ueber das Panel Loop
/// (`PreviewSoundFileEvent`, `LoadLoopEvent`).
void zeigeImportPaket(QWidget* parent, const QString& datei = {}, IEventBus* bus = nullptr);

}  // namespace lumi::ui
