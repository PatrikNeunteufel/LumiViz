/**
 * @file ExportFenster.hpp
 * @brief Loop: das Fenster *Export audio* (docs/audio/Spielweisen_Konzept.md §4.4,
 *        Stufe 30c; Entscheide WX2 a, WX6 a, WX9 a)
 *
 * Drei Zustaende in einem Fenster (WX6 a): **vor dem Export** *Clip*, *Contains*,
 * *File* mit **Browse...**, *Format* (32-bit float, 24-bit, 16-bit mit Dither) und
 * beim ganzen Loop *Ending* (Seamless loop, With tail) und *Passes*; **beim
 * Export** „Preparing sounds… n / m“, dann „Rendering… n %“, mit **Cancel**;
 * **am Ende** Dauer, Nachklang, Format und Groesse, eine Zeile in Warnfarbe bei
 * Kappung, **Show in folder** und **Close**. Gemerkt werden Format, Ordner,
 * Ending und Passes (in den Einstellungen unter `loop/export…`).
 *
 * Die Arbeit tut ein `ExportAuftrag`; das Fenster fragt ihn je Takt einer Uhr ab.
 * Geschlossen bricht es ab und wartet — der Auftrag endet vor dem Fenster.
 */
#pragma once

#include "loop/ExportAuftrag.hpp"

#include <QDialog>

#include <functional>
#include <memory>

class QButtonGroup;
class QLabel;
class QLineEdit;
class QProgressBar;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QTimer;
class QWidget;

namespace lumi::loop
{

class ExportFenster : public QDialog
{
    Q_OBJECT
public:
    /// `vorlage` sagt, was exportiert wird (Art, Loop, Clip, Versatz, Laenge, Rate);
    /// Datei, Format, Ending und Passes waehlt das Fenster. `loopName` fuer den Vorschlag
    /// des Dateinamens, `clipText` fuer die Zeile *Clip*.
    ExportFenster(ExportWunsch vorlage, QString loopName, QString clipText, Dekodierer dekodierer,
                  QWidget* parent = nullptr);
    ~ExportFenster() override;

    /// Den Export beginnen, wie **Export** (fuer Tests). false = keine Datei angegeben.
    bool starte();
    /// Laeuft ein Export?
    [[nodiscard]] bool laeuft() const { return m_auftrag != nullptr && !m_auftrag->beendet(); }
    /// Das Ergebnis des letzten Exports; leer, solange keiner fertig ist.
    [[nodiscard]] std::optional<ExportErgebnis> ergebnis() const { return m_ergebnis; }
    /// Warten, bis der Export fertig ist, und den Stand zeigen (fuer Tests).
    void warteAufEnde();

    /// Was **Show in folder** tut (Vorgabe: den Ordner im Dateimanager oeffnen) — fuer Tests.
    static void setzeOrdnerZeigerFuerTests(std::function<void(const QString& datei)> zeiger);

    /// Die Texte der Zeile *Contains* (WX6 a).
    [[nodiscard]] static QString enthaltenText(const ExportWunsch& wunsch);

protected:
    void reject() override;

private:
    void baue();
    void zeigeWahl();
    void zeigeStand();
    void zeigeEnde();
    void waehleDatei();
    void pruefeDatei();
    [[nodiscard]] ExportWunsch wunschJetzt() const;
    void merke() const;

    ExportWunsch m_vorlage;
    QString m_loopName;
    QString m_clipText;
    Dekodierer m_dekodierer;
    std::unique_ptr<ExportAuftrag> m_auftrag;
    std::optional<ExportErgebnis> m_ergebnis;

    QLabel* m_enthalten = nullptr;
    QLineEdit* m_datei = nullptr;
    QPushButton* m_waehlen = nullptr;
    QRadioButton* m_float = nullptr;
    QRadioButton* m_pcm24 = nullptr;
    QRadioButton* m_pcm16 = nullptr;
    QRadioButton* m_nahtlos = nullptr;
    QRadioButton* m_mitNachklang = nullptr;
    QSpinBox* m_durchlaeufe = nullptr;
    QWidget* m_wahl = nullptr;           ///< alles, was vor dem Export gewaehlt wird
    QProgressBar* m_balken = nullptr;
    QLabel* m_status = nullptr;
    QLabel* m_warnung = nullptr;
    QPushButton* m_export = nullptr;
    QPushButton* m_abbrechen = nullptr;
    QPushButton* m_ordner = nullptr;
    QPushButton* m_schliessen = nullptr;
    QTimer* m_uhr = nullptr;
};

}  // namespace lumi::loop
