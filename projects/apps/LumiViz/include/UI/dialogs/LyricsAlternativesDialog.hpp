/**
 ****************************************************************************************
 * @file   LyricsAlternativesDialog.hpp
 * @brief  Andere Fassungen eines Songtexts suchen, vergleichen und uebernehmen (S87)
 *
 * @details
 * Wunsch Patrik S87: der automatische Abruf erwischt mitunter die falsche
 * Fassung (zwei Versionen eines Songs, Titel mit Zusaetzen). Der Dialog fragt
 * `/api/search` von lrclib.net mit **frei waehlbarem** Titel und Interpreten
 * und zeigt die Eintraege mit Album, Dauer (und Abweichung zur Datei),
 * Zeilenzahl und Vermerk. Die gerade gezeigte Fassung steht fett.
 *
 * **Vergleichen:** bis zu drei markierte Eintraege stehen nebeneinander, als
 * LRC-Text MIT Zeitmarken (hervorgehoben). Laeuft der Titel, zeigt ein Balken
 * in jeder Spalte, wo der Song gerade ist — so sieht man, welche Fassung zum
 * Gesang passt. Tabelle und Texte teilen sich die Hoehe ueber einen Griff.
 *
 * **Uebernehmen** ersetzt die gespeicherte Fassung, sperrt die bisherige aber
 * nicht, und laesst den Dialog offen (weiter vergleichen). Wer einen
 * gesperrten Eintrag uebernimmt, hebt die Sperre auf.
 *
 * Die Suche ist eine ausdrueckliche Handlung: sie sendet die beiden
 * Suchbegriffe an den Dienst, auch wenn die selbsttaetige Online-Suche in den
 * Einstellungen aus ist. Der Dialog sagt das. Er ist nicht modal — der Player
 * bleibt bedienbar.
 ****************************************************************************************
 */

#pragma once

#include "audio/SongtextOnline.hpp"

#include <QDialog>
#include <QTimer>

#include <array>
#include <vector>

class QCheckBox;
class QLabel;
class QLineEdit;
class QNetworkAccessManager;
class QNetworkReply;
class QPlainTextEdit;
class QPushButton;
class QTableWidget;

class LyricsAlternativesDialog : public QDialog
{
    Q_OBJECT

public:
    static constexpr int kSpalten = 3;  ///< so viele Fassungen stehen nebeneinander

    /// `aktuelleId` = Kennung des gerade gezeigten Texts (0 = keine/unbekannt;
    /// dann wird die gezeigte Fassung am Wortlaut erkannt).
    LyricsAlternativesDialog(const TrackInfo& track, qint64 aktuelleId,
                             QWidget* parent = nullptr);

    /// Kandidaten anzeigen — von der Netzantwort gerufen; oeffentlich fuer
    /// Tests ohne Netz. Sortiert nach der Naehe der Dauer zur Datei.
    void zeigeKandidaten(std::vector<lumi::audio::SongtextKandidat> kandidaten);

    /// Die Fassung in Vergleichsspalte `spalte` uebernehmen (ihr Knopf „Use").
    void uebernehmen(int spalte = 0);

    /// Balken auf die Zeile bei `posMs` setzen (vom Zeitgeber gerufen;
    /// oeffentlich fuer Tests).
    void zeigePosition(std::int64_t posMs);

    /// Benutzerordner ersetzen (Tests — der echte bleibt unberuehrt).
    void setBenutzerOrdner(const QString& ordner) { m_ordner = ordner; }

    /// Keine Suche beim Oeffnen (Tests).
    static void setzeSucheBeimOeffnen(bool an);

private:
    struct Spalte
    {
        QWidget* rahmen = nullptr;
        QLabel* kopf = nullptr;
        QPushButton* nehmen = nullptr;
        QPlainTextEdit* text = nullptr;
        int kandidat = -1;   ///< Index in m_kandidaten, -1 = leer
        int zeile = -1;      ///< zuletzt markierte Zeile
    };

    void suche();
    void sende();
    void fertig(QNetworkReply* reply);
    void auswahlGeaendert();
    void vermerke();
    [[nodiscard]] bool istAktuell(const lumi::audio::SongtextKandidat& k) const;
    [[nodiscard]] QString ordner() const;

    TrackInfo m_track;
    qint64 m_aktuelleId = 0;
    QString m_ordner;
    std::vector<lumi::audio::SongtextKandidat> m_kandidaten;

    QLineEdit* m_titel = nullptr;
    QLineEdit* m_interpret = nullptr;
    QPushButton* m_suchen = nullptr;
    QLabel* m_status = nullptr;
    QTableWidget* m_tabelle = nullptr;
    QCheckBox* m_folgen = nullptr;
    std::array<Spalte, kSpalten> m_spalten;

    QNetworkAccessManager* m_net = nullptr;
    QNetworkReply* m_laufend = nullptr;
    QTimer m_timer;   ///< Wiederholung der Suche nach 429/503
    QTimer m_marke;   ///< Abspielposition in den Texten
    int m_versuch = 0;
};

/// Sperrliste zeigen und Eintraege wieder freigeben (Settings → Audio).
void zeigeLyricsSperrliste(QWidget* parent, const QString& benutzerOrdner = QString());
