/**
 ****************************************************************************************
 * @file   DatensicherungDialoge.hpp
 * @brief  Die eigenen Daten exportieren und importieren — die drei Dialoge
 *         (docs/core-services/Datenablage_Konzept.md §4a, Stufe A4b; Skizzen
 *         gebilligt, Patrik S114)
 *
 * @details
 * Settings → Panels → **User data**: **Export...** und **Import...**.
 *
 *  - `DatenExportDialog` („Export your data"): die Bereiche des Benutzerordners
 *    mit Haken und Zahl, abgesetzt darunter Settings und Downloaded lyrics
 *    (anfangs aus), der Zielpfad, eine Zeile mit der Summe.
 *  - `DatenImportDialog` („Import data"): nur die Bereiche, die in der Datei
 *    stecken, je mit Zahl und „n exist"; darunter die Regel fuer gleiche
 *    Namen (D7 a).
 *  - `DatenJeDateiDialog`: die Liste, die „Let me decide for each file"
 *    oeffnet — je gleichem Namen die eigene und die importierte Datei mit
 *    Haken, Datum und Groesse.
 *
 * Die Regeln selbst stehen in `core/Datensicherung`; hier ist nur die
 * Oberflaeche. Jeder Dialog laesst sich ohne gezeigtes Fenster bedienen
 * (`fuehreAus`) — der Knopf ruft dasselbe und zeigt danach die Meldung.
 ****************************************************************************************
 */

#pragma once

#include "core/Datensicherung.hpp"

#include <QDialog>

#include <map>
#include <vector>

class IEventBus;
class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSettings;
class QTreeWidget;
class QTreeWidgetItem;

namespace lumi::ui
{

/// Groesse fuer die Anzeige: „812 B", „3 KB", „1.4 MB".
[[nodiscard]] QString groesseAlsText(qint64 bytes);

class DatenExportDialog : public QDialog
{
    Q_OBJECT

public:
    /// `einstellungen` = nullptr: die der App.
    explicit DatenExportDialog(const QString& benutzerOrdner, QWidget* parent = nullptr,
                               QSettings* einstellungen = nullptr);

    /// Den Haken eines Bereichs setzen (`settings` = die Einstellungen).
    void setzeAn(const QString& id, bool an);
    [[nodiscard]] bool istAn(const QString& id) const;
    void setzeZiel(const QString& pfad);
    [[nodiscard]] QString ziel() const;
    /// Was der Dialog gerade exportieren wuerde.
    [[nodiscard]] lumi::core::datensicherung::ExportWahl wahl() const;

    /// Exportieren, ohne Meldung. Der Knopf **Export** ruft das und zeigt das Ergebnis.
    lumi::core::datensicherung::ExportErgebnis fuehreAus();

private:
    void aktualisiereSumme();
    void waehleZiel();
    void exportiere();

    QString m_ordner;
    QSettings* m_einstellungen = nullptr;
    std::map<QString, QCheckBox*> m_haken;  ///< je Bereich
    std::map<QString, int> m_zahl;          ///< Dateien je Bereich
    QCheckBox* m_settings = nullptr;
    QLineEdit* m_ziel = nullptr;
    QLabel* m_summe = nullptr;
    QPushButton* m_export = nullptr;
};

class DatenJeDateiDialog : public QDialog
{
    Q_OBJECT

public:
    /// `bereiche`: nur Dateien mit `Zustand::NameVergeben` erscheinen.
    explicit DatenJeDateiDialog(const std::vector<lumi::core::datensicherung::ImportBereich>& bereiche,
                                QWidget* parent = nullptr);

    /// Die Haken einer Zeile setzen (`rel` wie in `ImportDatei`).
    void setze(const QString& rel, bool meine, bool importierte);
    /// Alle Zeilen: nur die eigene / nur die importierte.
    void setzeAlle(bool meine, bool importierte);
    [[nodiscard]] int zeilen() const;
    /// Je Datei, was die Haken sagen. Beide aus = die eigene bleibt.
    [[nodiscard]] std::map<QString, lumi::core::datensicherung::Wahl> wahl() const;
    /// Die Zeile unter der Liste („Keeps 5 of yours, …").
    [[nodiscard]] QString summe() const;

private:
    void aktualisiereSumme();

    QTreeWidget* m_liste = nullptr;
    std::vector<QTreeWidgetItem*> m_zeilen;
    QLabel* m_summe = nullptr;
};

class DatenImportDialog : public QDialog
{
    Q_OBJECT

public:
    /// Oeffnet `datei` und haelt sie gegen `benutzerOrdner`. Laesst sie sich
    /// nicht lesen: `inhalt().ok` ist false, der Dialog zeigt die Meldung und
    /// **Import** ist aus. `einstellungen` = nullptr: die der App.
    DatenImportDialog(const QString& datei, const QString& benutzerOrdner, QWidget* parent = nullptr,
                      QSettings* einstellungen = nullptr);

    [[nodiscard]] const lumi::core::datensicherung::Inhalt& inhalt() const { return m_inhalt; }
    void setzeAn(const QString& id, bool an);
    [[nodiscard]] bool istAn(const QString& id) const;
    void setzeRegel(lumi::core::datensicherung::Regel regel);
    [[nodiscard]] lumi::core::datensicherung::Regel regel() const;
    /// Nur fuer `Regel::JeDatei`: die Wahl, die sonst die Liste liefert.
    void setzeJeDatei(std::map<QString, lumi::core::datensicherung::Wahl> wahl) { m_jeDatei = std::move(wahl); }
    /// So viele Namen gibt es schon — in den angehakten Bereichen.
    [[nodiscard]] int vergeben() const;

    /// Importieren, ohne Liste und ohne Meldung; setzt auch die Einstellungen,
    /// wenn ihr Haken sitzt. Der Knopf **Import** oeffnet bei „Let me decide"
    /// vorher die Liste und zeigt danach das Ergebnis.
    lumi::core::datensicherung::ImportErgebnis fuehreAus();
    /// So viele Einstellungen hat der letzte `fuehreAus` gesetzt.
    [[nodiscard]] int einstellungenGesetzt() const { return m_einstellungenGesetzt; }

private:
    void aktualisiere();
    void importiere();

    QString m_datei;
    QString m_ordner;
    QSettings* m_einstellungen = nullptr;
    lumi::core::datensicherung::Inhalt m_inhalt;
    std::map<QString, QCheckBox*> m_haken;
    QCheckBox* m_settings = nullptr;
    std::map<lumi::core::datensicherung::Regel, QRadioButton*> m_regeln;
    std::map<QString, lumi::core::datensicherung::Wahl> m_jeDatei;
    QLabel* m_hinweis = nullptr;
    QPushButton* m_import = nullptr;
    int m_einstellungenGesetzt = 0;
};

/// Settings → User data → **Import...**: nach der Datei fragen, den Dialog zeigen
/// — ein Paket aus Publish im Dialog Import package (mit `bus`: ▶ und Oeffnen; `rate`:
/// die Ausgaberate, mit der es Wellenformen eingefrorener Clips prueft, Stufe 30i).
void zeigeDatenImport(QWidget* parent, IEventBus* bus = nullptr, int rate = 0);
/// Settings → User data → **Export...**.
void zeigeDatenExport(QWidget* parent);

}  // namespace lumi::ui
