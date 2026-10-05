/**
 * @file ComposerPanel.hpp
 * @brief Composer: das Panel zum Ansehen (docs/ui/Composer_Konzept.md §7, Stufe C2b)
 *
 * **Kopf:** Schalter Composer on, Play / Pause, Stopp, Loop, Resume after
 * restart, die Stelle als Zeit, der Name der Datei. **Zeitachse:** Lineal mit
 * Abspielmarke (Klick und Ziehen springen), die Kanaele als Zeilen, die Clips
 * als Bloecke mit ihrer Pegelkurve. **Clip-Zeile:** Datei, Start, Ende und
 * Mischart des angeklickten Clips.
 *
 * Bearbeitet wird hier noch nichts (C3). Das Panel bedient nur die
 * `composer/Sitzung` des Fensters — dieselbe, ueber die Menue und Start gehen.
 *
 * Rueckbau (Konzept §12): Teil von `include/composer` / `src/composer`; die
 * Anmeldung in `PanelAutoReg.cpp` ist Naht N8.
 */
#pragma once

#include "UI/panels/PanelBase.hpp"
#include "composer/Bearbeiten.hpp"
#include "composer/Komposition.hpp"

#include <QPointer>
#include <QString>
#include <QStringList>
#include <QWidget>

#include <cstdint>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QPushButton;
class VisualizerWidget;

namespace lumi::composer
{

class Sitzung;

/// Die Zeitachse: Lineal, Abspielmarke, Kanaele und Clips. Zeigt die ganze
/// Laenge der Komposition auf ihrer Breite (ein Zoom kommt mit C3).
class Zeitachse : public QWidget
{
    Q_OBJECT

public:
    explicit Zeitachse(QWidget* parent = nullptr);

    /// Was gezeigt wird; `geladen` false = der Hinweis „keine Komposition".
    void setKomposition(const Komposition& komposition, bool geladen);
    void setStelle(std::int64_t stelleMs);
    /// Der Composer ist aus: die Clips blasser, die Marke gestrichelt.
    void setAn(bool an);
    /// Bearbeiten (C3a, Schalter Edit): Clips lassen sich verschieben, an den
    /// Kanten ziehen und mit Entf loeschen. Aus = nur ansehen und anklicken.
    void setBearbeiten(bool an);
    [[nodiscard]] bool bearbeiten() const { return m_bearbeiten; }
    /// Die bearbeitete Fassung derselben Komposition zeigen (auch nach Undo);
    /// Stelle und — wo es den Clip noch gibt — Auswahl bleiben.
    void zeigeBearbeitet(const Komposition& komposition);
    /// Eine Zeit beim Ziehen einrasten: auf Kanten anderer Clips und die
    /// Abspielmarke, sonst auf das Raster der Datei. `ausser` = der gezogene
    /// Clip (Kanal, Clip) in `in`.
    [[nodiscard]] std::int64_t raste(std::int64_t zeitMs, const Komposition& in, int ausserKanal,
                                     int ausserClip) const;

    // --- Neues hinein (C3b) — alles nur im Bearbeiten -----------------------
    /// Preset-Dateien an der Stelle `ort` fallen lassen: in die Zeile darunter
    /// als neue Clips hintereinander; unterhalb der letzten Zeile entsteht ein
    /// neuer Kanal. `tasten` wie beim Ziehen (Ctrl kuerzen, Ctrl+Shift
    /// schieben, Alt frei). false = nichts eingefuegt.
    bool lasseFallen(QPoint ort, const QStringList& dateien,
                     Qt::KeyboardModifiers tasten = Qt::NoModifier);
    /// Einen leeren Kanal anhaengen.
    void kanalHinzu();
    void kanalLoeschen(int kanal);
    void kanalUmbenennen(int kanal, const QString& name);
    void kanalSchalten(int kanal, bool an);
    /// Der Kanal der Zeile unter `y`, -1 = keine Zeile (Lineal, freie Flaeche).
    [[nodiscard]] int kanalBei(int y) const;

    // --- Ein- und Ausblenden (C4) — nur im Bearbeiten ------------------------
    /// Mitte des Blenden-Griffs oben im Clip: am Ende der Einblendung
    /// (`ein`) bzw. am Anfang der Ausblendung. (-1,-1) = der Clip hat keine
    /// Griffe (es gibt ihn nicht, oder seine Kurve ist keine einfache Blende).
    [[nodiscard]] QPoint blendenGriff(int kanal, int clip, bool ein) const;
    /// Mitte eines der zwei Bézier-Anfasser (`nr` 0 oder 1) einer Blende.
    [[nodiscard]] QPoint bezierGriff(int kanal, int clip, bool ein, int nr) const;
    /// Die Form einer Blende setzen (Rechtsklick). Eine Blende der Laenge
    /// null wird dabei zwei Sekunden lang — sonst saehe man die Wahl nicht.
    void setzeBlendenForm(int kanal, int clip, bool ein, Kurvenform form);

    /// Die Fassung, die die Achse gerade zeigt.
    [[nodiscard]] const Komposition& komposition() const { return m_komposition; }
    [[nodiscard]] int gewaehlterKanal() const { return m_kanal; }
    [[nodiscard]] int gewaehlterClip() const { return m_clip; }
    /// Der angeklickte Clip, nullptr = keiner.
    [[nodiscard]] const VisualClip* auswahl() const;

    // --- Geometrie (auch fuer Tests) ----------------------------------------
    [[nodiscard]] int xVon(std::int64_t stelleMs) const;
    [[nodiscard]] std::int64_t stelleVon(int x) const;
    /// Mitte der Zeile des Kanals (0 = oberste Zeile).
    [[nodiscard]] int yVonZeile(int zeile) const;
    /// Kanal einer Zeile: die Reihenfolge der Datei, der erste Kanal zuoberst
    /// (Entscheid Patrik S95) — im Bild liegt er zuunterst, spaetere darueber.
    [[nodiscard]] int kanalVonZeile(int zeile) const;

    static constexpr int kKopfBreite = 96;   ///< Spalte mit den Kanalnamen
    static constexpr int kRand = 10;         ///< frei rechts neben dem Ende
    static constexpr int kLinealHoehe = 24;
    static constexpr int kZeilenHoehe = 36;

Q_SIGNALS:
    /// Klick oder Ziehen im Lineal.
    void sprung(qint64 stelleMs);
    /// Ein Clip wurde angeklickt — oder die freie Flaeche (dann -1, -1).
    void auswahlGeaendert();
    /// Ein Zug ist losgelassen oder ein Clip geloescht: die neue Fassung und
    /// ein Wort dazu fuer Undo („Move clip").
    void bearbeitet(const lumi::composer::Komposition& neu, const QString& was);
    /// Diese Dateien wurden gerade als Clips eingefuegt.
    void hineingezogen(const QStringList& dateien);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSizeHint() const override;

public:
    /// So viel Zeit zeigt die Achse fuer diese Komposition: ihre Laenge und
    /// etwas Luft dahinter, damit sich ueber das Ende hinaus ziehen laesst.
    [[nodiscard]] static std::int64_t anzeigeLaenge(const Komposition& komposition);

private:
    void loescheAuswahl();
    /// Eine selbst erzeugte Fassung zeigen und die Auswahl setzen.
    void uebernimm(const Komposition& neu, int kanal, int clip);
    [[nodiscard]] int zeitBreite() const;
    [[nodiscard]] QRect clipRechteck(int zeile, const VisualClip& clip) const;

    Komposition m_komposition;
    bool m_geladen = false;
    bool m_an = false;
    std::int64_t m_laengeMs = 0;
    std::int64_t m_stelleMs = 0;
    int m_kanal = -1;
    int m_clip = -1;
    bool m_zieht = false;  ///< Maus im Lineal gedrueckt
    bool m_bearbeiten = false;

    /// Ein Zug an einem Clip (nur im Bearbeiten).
    enum class Griff
    {
        Keiner,
        Verschieben,
        KanteLinks,
        KanteRechts,
        BlendeEin,   ///< Laenge der Einblendung
        BlendeAus,
        BezierEin1,  ///< die zwei Anfasser der Einblendung
        BezierEin2,
        BezierAus1,
        BezierAus2,
    };
    /// Der Zeitbereich einer Blende des Clips in Pixeln; false = keine.
    [[nodiscard]] bool blendenBereich(int kanal, int clip, bool ein, QRect* clipRect, int* x0,
                                      int* x1, Blenden* blenden) const;
    [[nodiscard]] Griff griffBei(QPoint ort, int* kanal, int* clip) const;
    void zieheZu(QPoint ort, Qt::KeyboardModifiers tasten);
    void brichZugAb();

    Griff m_griff = Griff::Keiner;
    bool m_zugLaeuft = false;        ///< die Maus hat sich seit dem Druck weit genug bewegt
    QPoint m_zugStart;
    std::int64_t m_zugVersatzMs = 0; ///< Abstand Maus – Clip-Start beim Druck
    int m_zugKanal = -1;             ///< Kanal und
    int m_zugClip = -1;              ///< Platz des Clips in `m_vorZug`
    Komposition m_vorZug;            ///< der Stand beim Druck
    QPoint m_fallOrt{-1, -1};        ///< wohin ein Drag von aussen gerade zeigt (-1 = keiner)
};

class ComposerPanel : public PanelBase
{
    Q_OBJECT

public:
    explicit ComposerPanel(ServiceContainer& services, QWidget* parent = nullptr);
    ~ComposerPanel() override;

    /// Die Zeitachse scrollt selbst (viele Kanaele).
    [[nodiscard]] bool scrollsItself() const override { return true; }
    [[nodiscard]] int preferredArea() const override { return Qt::BottomDockWidgetArea; }
    /// Breit und flach — nicht das hohe Format der uebrigen Panels.
    [[nodiscard]] QSize sizeHint() const override { return {900, 260}; }

    /// Das Fenster, dessen Komposition das Panel zeigt. Ohne Aufruf sucht das
    /// Panel es beim Zeigen selbst.
    void setFenster(VisualizerWidget* fenster);
    [[nodiscard]] Sitzung* sitzung() const;
    [[nodiscard]] Zeitachse* zeitachse() const { return m_zeitachse; }

    /// Eine Stelle als Text: Minuten, Sekunden, Zehntel („1:23.4").
    [[nodiscard]] static QString zeitText(std::int64_t ms);
    /// Enthaelt die Kettendatei einen Host Switcher? Dann wechseln im Composer
    /// zwei Wechsler — davor warnt das Panel beim Hinzufuegen (Entscheid S88).
    [[nodiscard]] static bool hatHostSwitcher(const QString& pfad);

protected:
    void showEvent(QShowEvent* event) override;

private:
    void setupUI();
    void sucheFenster();
    /// Alles neu: eine andere Komposition (oder keine).
    void zeigeKomposition();
    /// Schalter, Knoepfe, Stelle — bei jeder Aenderung und im Takt.
    void zeigeStand();
    /// Dateiname mit Stern und die Knoepfe Save / Save As.
    void zeigeDatei();
    void zeigeClip();
    /// Der Schalter „Fixed end" und seine Zeit.
    void zeigeEnde();
    /// Eine Aenderung ausfuehren — ueber den CommandBus der App (Undo / Redo),
    /// ohne ihn direkt.
    void bearbeite(const Komposition& neu, const QString& was);

    QPointer<VisualizerWidget> m_fenster;
    QPointer<Sitzung> m_sitzung;

    QCheckBox* m_an = nullptr;
    QPushButton* m_spielen = nullptr;
    QPushButton* m_stopp = nullptr;
    QPushButton* m_schleife = nullptr;
    QPushButton* m_resume = nullptr;
    QLabel* m_zeit = nullptr;
    QLabel* m_datei = nullptr;
    Zeitachse* m_zeitachse = nullptr;
    QLabel* m_clipZeile = nullptr;
    QCheckBox* m_edit = nullptr;
    QPushButton* m_neu = nullptr;
    QPushButton* m_laden = nullptr;
    QPushButton* m_speichern = nullptr;
    QPushButton* m_speichernUnter = nullptr;
    QComboBox* m_mischart = nullptr;
    QCheckBox* m_endeFest = nullptr;
    QDoubleSpinBox* m_endeZeit = nullptr;
    int m_gezeigt = -1;  ///< was Schalter und Knoepfe zuletzt zeigten
};

}  // namespace lumi::composer
