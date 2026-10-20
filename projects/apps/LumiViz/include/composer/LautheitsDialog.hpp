/**
 * @file LautheitsDialog.hpp
 * @brief Composer: der Dialog Match loudness (Composer_Konzept §9g,
 *        Entscheide Patrik L3 a, L4 a, G5-1 a bis G5-4 a)
 *
 * Nicht modal: der Composer spielt weiter, was im Dialog steht, hoert man
 * sofort (G5-1 a) — ueber die Komposition der Sitzung, ohne Undo-Schritt.
 * *OK* schreibt EINEN Schritt „Match loudness" (G5-2 a), *Cancel* und das
 * Schliessen stellen die Gains von vorher wieder her. Gemessen wird beim
 * Oeffnen, Clip fuer Clip in einem eigenen Thread (G5-3 a); Songs merkt
 * `SongLautheiten`, Loops werden immer neu gemessen.
 */
#pragma once

#include "composer/Abgleich.hpp"
#include "composer/ClipLautheit.hpp"

#include <QDialog>
#include <QPointer>

#include <atomic>
#include <functional>
#include <memory>
#include <thread>

class IAudioEngine;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QPushButton;
class QTableWidget;

namespace lumi::composer
{

class Sitzung;

class LautheitsDialog : public QDialog
{
    Q_OBJECT

public:
    /// Wie das Panel einen Undo-Schritt ausfuehrt (`ComposerPanel::bearbeite`).
    using Bearbeite = std::function<void(const Komposition& neu, const QString& was)>;

    /// `engine` darf fehlen — dann misst der Dialog nichts (alle Zeilen „not readable").
    LautheitsDialog(Sitzung& sitzung, IAudioEngine* engine, Bearbeite bearbeite, QWidget* parent = nullptr);
    ~LautheitsDialog() override;

    /// Rechtsklick „Match loudness to this clip..." (G5-4 a): dieser Clip ist das Ziel.
    void setzeZielClip(int kanal, int clip);
    /// Knopf neben Gain (G5-4 a): die Zeile dieses Clips markieren, das Ziel bleibt.
    void markiere(int kanal, int clip);

    // --- fuer Tests und die Knoepfe -------------------------------------------
    [[nodiscard]] const Abgleich& abgleich() const { return m_abgleich; }
    /// Laeuft noch eine Messung oder wartet eine?
    [[nodiscard]] bool misst() const;
    /// Die markierte Zeile, -1 = keine.
    [[nodiscard]] int markiert() const { return m_markiert; }
    /// Das Gain einer Zeile im Dialog setzen (wie der Schieber).
    void setzeGain(int zeile, double db);
    /// Use suggestions.
    void nimmVorschlaege();
    /// Listen A/B: `vorher` = die Gains von vor dem Dialog hoeren (A).
    void hoereVorher(bool vorher);
    [[nodiscard]] bool hoertVorher() const { return m_vorher; }
    /// ▶ einer Zeile: die Mitte des gespielten Ausschnitts, Composer spielt.
    void spieleZeile(int zeile);
    /// Measure again.
    void missNeu();
    /// Der Text der Warnungen unter der Tabelle (leer = keine).
    [[nodiscard]] QString warnText() const;

    /// Die letzte Wahl des Ziels (L3 a) steht in den Einstellungen unter
    /// `composer/matchLoudness/…`: Fixed level mit seinem Wert, sonst der lauteste Clip.
    static constexpr const char* kEinstellungFest = "composer/matchLoudness/fixedLevel";
    static constexpr const char* kEinstellungLufs = "composer/matchLoudness/fixedLufs";

Q_SIGNALS:
    /// Alle Zeilen sind gemessen (oder nicht lesbar).
    void messungFertig();

public Q_SLOTS:
    void accept() override;
    void reject() override;

private:
    void baueTabelle();
    void zeigeZeile(int zeile);
    void zeigeAlles();
    void fuelleZiel();
    void zeigeVorschau();
    /// Eine Fassung in die Sitzung geben, als Dialog (nicht „von aussen").
    void setze(const Komposition& neu);
    void wennKompositionBearbeitet();
    void wennKompositionGewechselt();
    void stelleWiederHer();
    /// Die naechste wartende Zeile messen (aus dem Speicher, sonst im Thread).
    void misseNaechste();
    void nimmErgebnis(const AbgleichZeile& was, std::optional<audio::Lautheit> ergebnis);
    void halteMessungAn();
    void merkeZielwahl() const;

    QPointer<Sitzung> m_sitzung;
    IAudioEngine* m_engine = nullptr;
    Bearbeite m_bearbeite;
    Abgleich m_abgleich;
    SongLautheiten m_gemerkt;
    Komposition m_gesetzt;          ///< was der Dialog zuletzt in die Sitzung gab
    bool m_setztSelbst = false;     ///< der Dialog gibt gerade selbst eine Fassung in die Sitzung
    bool m_vorher = false;          ///< Listen A/B: A (vorher)
    bool m_zielVonHand = false;     ///< der Benutzer hat ein Ziel gewaehlt — nicht mehr „der lauteste"
    bool m_fertig = false;          ///< OK oder Cancel ist durch
    int m_markiert = -1;
    int m_rate = 44100;

    // --- Messen ---------------------------------------------------------------
    std::vector<AbgleichZeile> m_warten;  ///< was noch gemessen wird (als Clip, nicht als Zeile)
    bool m_laeuft = false;
    int m_lauf = 0;  ///< zaehlt die Messungen — ein Ergebnis einer abgebrochenen gilt nicht
    std::thread m_faden;
    std::shared_ptr<std::atomic<bool>> m_abbruch;
    std::shared_ptr<std::atomic<int>> m_prozent;  ///< Fortschritt der laufenden Messung
    AbgleichZeile m_inArbeit;

    // --- Oberflaeche ----------------------------------------------------------
    QComboBox* m_ziel = nullptr;
    QDoubleSpinBox* m_festLufs = nullptr;
    QPushButton* m_neuMessen = nullptr;
    QTableWidget* m_tabelle = nullptr;
    QLabel* m_warnung = nullptr;
    QPushButton* m_vorschlaege = nullptr;
    QPushButton* m_ab = nullptr;
    QPushButton* m_ok = nullptr;
    bool m_fuellt = false;  ///< die Oberflaeche wird gerade gefuellt — keine Rueckmeldung
};

}  // namespace lumi::composer
