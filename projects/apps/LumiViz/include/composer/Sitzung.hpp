/**
 * @file Sitzung.hpp
 * @brief Composer: die geladene Komposition eines Fensters und ihr Transport
 *        (docs/ui/Composer_Konzept.md §7, §8, Stufe C2b)
 *
 * **Geladen ist nicht an** (Patrik S95): die Sitzung haelt die Komposition,
 * auch wenn der Composer aus ist — dann liefert die Effektkette das Bild, und
 * im Panel laesst sich vorbereiten. **Composer on** setzt die Bildquelle
 * (Naht R3), aus nimmt sie weg; Komposition und Stelle bleiben.
 *
 * Die Sitzung ist die eine Stelle, ueber die Menue, Panel und Start den
 * Composer bedienen: Laden, Composer on, Play / Pause / Stopp / Sprung, Loop,
 * das Ende (einstellbar) und der Stand ueber den Neustart (Resume).
 *
 * Threads: lebt im GUI-Thread, als Kind des `VisualizerWidget`. Die Uhr liegt
 * in der Bildquelle und wird nur unter dem Render-Mutex angefasst; fuer die
 * Anzeige liest die Sitzung den Stand, den die Bildquelle je Bild spiegelt —
 * ohne zu warten.
 *
 * Rueckbau (Konzept §12): Teil von `include/composer` / `src/composer`.
 */
#pragma once

#include "composer/Komposition.hpp"

#include <QObject>
#include <QString>
#include <QStringList>

#include <cstdint>

class QTimer;
class VisualizerWidget;

namespace lumi::composer
{

class Bildquelle;

/// Was am Ende der Komposition geschieht, wenn Loop aus ist (Entscheid Patrik
/// S95: alle drei waehlbar, Vorgabe Halten).
enum class AmEnde : int
{
    Halten = 0,       ///< die Uhr steht am Ende: schwarz, Karaoke laeuft, Composer bleibt an
    AnDenAnfang = 1,  ///< an den Anfang springen und halten, wie Stopp
    ComposerAus = 2,  ///< der Composer schaltet sich aus — die Effektkette liefert das Bild
};

class Sitzung : public QObject
{
    Q_OBJECT

public:
    /// Die Sitzung dieses Fensters; beim ersten Aufruf angelegt.
    [[nodiscard]] static Sitzung& fuer(VisualizerWidget& widget);
    /// Die Sitzung dieses Fensters, wenn es schon eine gibt.
    [[nodiscard]] static Sitzung* finde(const VisualizerWidget& widget);

    // --- Die Komposition im Panel -------------------------------------------

    /// Eine `.lvcomp` laden. Der Composer geht dabei aus, die Stelle an den
    /// Anfang. false = keine Komposition; es bleibt alles, wie es war.
    bool lade(const QString& pfad, QStringList* bericht = nullptr);
    /// Oeffnen von Hand (Menue): laden, Composer an und — ausser die
    /// Einstellung „haelt am Anfang" ist gesetzt — sofort abspielen.
    bool oeffne(const QString& pfad, QStringList* bericht = nullptr);
    /// Composer aus und die Komposition weg.
    void entlade();

    [[nodiscard]] bool geladen() const { return m_geladen; }
    [[nodiscard]] const Komposition& komposition() const { return m_komposition; }
    [[nodiscard]] const QString& datei() const { return m_datei; }

    // --- Bearbeiten und Speichern (C3a) ---------------------------------------

    /// Die geladene Komposition durch eine geaenderte Fassung ersetzen. Datei,
    /// Composer on, Stelle und Lauf bleiben; laeuft sie gerade, sieht man die
    /// Aenderung sofort (die Renderer der Clips legen sich dafuer neu an).
    void setzeKomposition(Komposition neu);
    /// Weicht die Komposition von dem ab, was in der Datei steht?
    [[nodiscard]] bool ungespeichert() const;
    /// In die geladene Datei schreiben. false = nichts geladen oder Schreibfehler.
    bool speichere();
    /// In eine andere Datei schreiben; sie ist danach die geladene.
    bool speichereNach(const QString& pfad);
    /// Zaehlt Laden und Entladen — ein Undo-Schritt einer frueheren Ladung
    /// gilt nicht mehr.
    [[nodiscard]] int ladung() const { return m_ladung; }

    // --- Composer on ----------------------------------------------------------

    /// An: die Komposition liefert das Bild, an der gemerkten Stelle,
    /// angehalten. Aus: die Effektkette liefert es wieder. Ohne geladene
    /// Komposition bleibt der Composer aus.
    void setAn(bool an);
    [[nodiscard]] bool an() const { return quelle() != nullptr; }

    // --- Transport ------------------------------------------------------------

    /// Abspielen; schaltet den Composer ein, wenn er aus ist. Am Ende
    /// stehend: von vorn.
    void spiele();
    void pausiere();
    /// Anhalten und an den Anfang.
    void stoppe();
    /// An eine Stelle springen; ob die Uhr laeuft, bleibt. Geht auch bei
    /// ausgeschaltetem Composer — die Stelle gilt dann beim Einschalten.
    void springe(std::int64_t stelleMs);

    [[nodiscard]] bool laeuft() const;
    [[nodiscard]] std::int64_t stelleMs() const;
    [[nodiscard]] std::int64_t laengeMs() const { return m_geladen ? m_komposition.laenge() : 0; }

    /// Loop ueber die ganze Komposition; bleibt ueber den Neustart
    /// (`composer/loop`).
    void setSchleife(bool an);
    [[nodiscard]] bool schleife() const { return m_schleife; }

    // --- Einstellungen (QSettings unter `composer/…`) -------------------------

    /// Resume after restart (Vorgabe aus).
    [[nodiscard]] static bool resumeAn();
    static void setResume(bool an);
    /// Open Composition haelt am Anfang, statt sofort zu spielen (Vorgabe aus).
    [[nodiscard]] static bool oeffnenHaelt();
    static void setOeffnenHaelt(bool haelt);
    [[nodiscard]] static AmEnde amEnde();
    static void setAmEnde(AmEnde art);

    // --- Ueber den Neustart (Konzept §8 „Der Start") --------------------------

    /// Datei, „Composer an", Stelle und „lief gerade" festhalten — immer, auch
    /// ohne Komposition, damit kein frueherer Stand stehen bleibt.
    void sichereStand() const;
    /// Beim Start: die zuletzt geladene Komposition wieder ins Panel; mit
    /// Resume auch ihr Stand. false = es gab keine (oder die Datei fehlt).
    bool stelleWiederHer(QStringList* bericht = nullptr);

    /// Die Messung der laufenden Komposition samt Anlauf je Clip ins Log.
    void protokolliereMessung();

Q_SIGNALS:
    /// Eine andere Komposition ist geladen — oder keine mehr.
    void kompositionGeaendert();
    /// Die geladene Komposition wurde bearbeitet (auch: Undo, Redo).
    void kompositionBearbeitet();
    /// Komposition, Composer on, Loop, Transport oder der Stand „gespeichert"
    /// haben sich geaendert.
    void geaendert();
    /// Solange der Composer an ist: etwa 30-mal je Sekunde — die Stelle neu lesen.
    void takt();

private:
    explicit Sitzung(VisualizerWidget& widget);

    [[nodiscard]] Bildquelle* quelle() const;
    void beimTakt();

    VisualizerWidget& m_widget;
    Komposition m_komposition;
    Komposition m_gespeichert;  ///< was in der Datei steht
    QString m_datei;
    int m_ladung = 0;
    bool m_geladen = false;
    bool m_schleife = false;
    std::int64_t m_stelleMs = 0;  ///< die Stelle, solange der Composer aus ist
    QTimer* m_takt = nullptr;
};

}  // namespace lumi::composer
