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

#include <QHash>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QWidget>

#include <atomic>
#include <cstdint>
#include <memory>
#include <thread>
#include <vector>

class QCheckBox;
class QMimeData;
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
    /// Ueberlappung in der Schleife (S99, Entscheid Patrik: beide Fenster
    /// zeigen). `schleife`: Loop ist an — dann sind die ersten und die letzten
    /// Sekunden der Komposition getoent, so weit ihre Ueberlappung reicht.
    /// `zweiteMs`: die Stelle im hinteren Fenster, die gerade mitlaeuft — eine
    /// zweite, blassere Marke; -1 = keine.
    void setSchleife(bool schleife, std::int64_t zweiteMs);
    [[nodiscard]] bool schleife() const { return m_schleife; }
    [[nodiscard]] std::int64_t zweiteStelleMs() const { return m_zweiteMs; }
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

    // --- Audiokanal (C5) — Songs als Clips -----------------------------------
    //
    // Songs fallen nur in Audio-Kanaele, Presets nur in Visual-Kanaele; unter
    // der letzten Zeile entsteht ein Kanal der Art der ersten Datei. Die
    // Zeitachse kennt keine Audio-Engine: Endungen, Song-Laengen und
    // Huellkurven reicht das Panel herein.

    /// Einen leeren Audio-Kanal anhaengen (heisst A1, A2 ...).
    void audioKanalHinzu();
    /// Welche Endungen als Song gelten (klein, ohne Punkt).
    void setAudioEndungen(const QStringList& endungen) { m_audioEndungen = endungen; }
    /// Woher die Laenge eines Songs kommt; ohne Auskunft ist ein neuer Ton-Clip 20 s lang.
    void setSongLaenge(SongLaenge laengeVon) { m_songLaenge = std::move(laengeVon); }
    /// Die Huellkurve eines Songs (je `msProWert` Millisekunden ein Wert 0…1)
    /// — die Wellenform im Ton-Clip. Leer = nicht lesbar; auch das wird gemerkt.
    void setHuelle(const QString& datei, std::vector<float> huelle, int msProWert);
    [[nodiscard]] bool hatHuelle(const QString& datei) const { return m_huellen.contains(datei); }
    /// Die Dateien, die in einem Drag als Clips in Frage kommen: Presets und Songs.
    [[nodiscard]] QStringList fallDateien(const QMimeData* daten) const;
    /// Wo die Dateien eines Drags an `ort` landen wuerden — je Datei Kanal,
    /// Anfang und Ende. Daraus zeichnet die Achse beim Hineinziehen den
    /// Schatten der Clips (Wunsch Patrik S98). Leer = dort faellt nichts
    /// (Lineal, Zeile der anderen Art, kein Bearbeiten). Ein Kanal gleich der
    /// Zahl der Kanaele heisst: unter der letzten Zeile entstuende ein neuer.
    struct Schatten
    {
        int kanal = -1;
        std::int64_t startMs = 0;
        std::int64_t endeMs = 0;
    };
    [[nodiscard]] std::vector<Schatten> fallSchatten(QPoint ort, const QStringList& dateien,
                                                     Qt::KeyboardModifiers tasten = Qt::NoModifier) const;

    // --- Einfuegemarke, Kopieren / Ausschneiden / Einfuegen (S98) ------------
    //
    // Entscheid Patrik S98 (a): ein Klick auf eine freie Stelle einer
    // Kanalzeile setzt dort die Einfuegemarke — Kanal und Stelle. „Einfuegen"
    // legt den Clip aus der Ablage mit seinem ANFANG an die Marke; die
    // Abspielmarke bleibt, wo sie ist. Die Ablage gilt fuer alle Zeitachsen
    // des Programms und traegt einen Clip samt Kurve, Mischart und Einsatz.

    /// Kanal der Einfuegemarke, -1 = es gibt keine.
    [[nodiscard]] int markeKanal() const { return m_markeKanal; }
    [[nodiscard]] std::int64_t markeMs() const { return m_markeMs; }
    /// Die Marke setzen (`kanal` -1 = wegnehmen). Nur im Bearbeiten.
    void setMarke(int kanal, std::int64_t stelleMs);
    /// Den angeklickten Clip in die Ablage legen. false = keiner angeklickt.
    bool kopiere();
    /// Kopieren und den Clip entfernen (nur im Bearbeiten).
    bool schneideAus();
    /// Den Clip aus der Ablage an die Einfuegemarke legen — nur in einen Kanal
    /// seiner Art; liegt dort schon ein Clip, in die naechste Luecke, in die
    /// er passt. Danach steht die Marke hinter ihm. Ohne Marke: hinter den
    /// angeklickten Clip, in dessen Kanal. false = nichts eingefuegt.
    bool fuegeEin();
    /// Liegt ein Clip in der Ablage — und ist es ein Song (`ton`)?
    [[nodiscard]] static bool ablageVoll(bool* ton = nullptr);
    static void leereAblage();

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

    /// Die Form einer Blende aus einem Blenden-Preset setzen (Rechtsklick);
    /// Dauer und die andere Seite bleiben. Eine Blende der Laenge null wird
    /// dabei zwei Sekunden lang. Ein Name, den es nicht gibt, aendert nichts.
    void setzeBlendenPreset(int kanal, int clip, bool ein, const QString& name);
    /// Die Form eines Stuecks aus einem Stueck-Preset setzen; Lage, Weite
    /// und „Flipped" bleiben.
    void setzeStueckPreset(int kanal, int clip, int stueck, const QString& name);

    // --- Teile der Pegelkurve (C4 Teil 2) ------------------------------------
    /// Die Kurve eines Teils oeffnen: der Clip wird angeklickt, `kurveOeffnen`
    /// gemeldet. Eine Blende ohne Dauer bekommt im Bearbeiten zwei Sekunden.
    void oeffneKurve(int kanal, int clip, Teil teil);
    /// Ein Stueck mit der Mitte bei `zeitImClipMs` in den Clip legen und
    /// seine Kurve oeffnen (nur im Bearbeiten).
    void stueckHinzu(int kanal, int clip, std::int64_t zeitImClipMs);
    void stueckLoeschen(int kanal, int clip, int stueck);
    /// Das Teil, an dem der Curve Editor haengt — am angeklickten Clip markiert.
    void setTeil(Teil teil);
    /// Mitte eines Griffs des Stuecks oben im Clip: `welcher` 0 = die Raute in
    /// der Mitte (verschieben), -1 / +1 = der Strich an der linken / rechten
    /// Kante (Weite). (-1,-1) = keinen — das Stueck gibt es nicht, oder es ist
    /// fuer Kanten-Griffe zu schmal.
    [[nodiscard]] QPoint stueckGriff(int kanal, int clip, int stueck, int welcher) const;

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
    /// Die Kurve dieses Teils des angeklickten Clips soll in den Curve Editor.
    void kurveOeffnen(lumi::composer::Teil teil);

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
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    bool event(QEvent* event) override;
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
    bool m_schleife = false;         ///< Loop ist an: die Fenster der Ueberlappung zeigen
    std::int64_t m_zweiteMs = -1;    ///< die Stelle im hinteren Fenster, -1 = keine
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
        StueckLage,   ///< ein Stueck verschieben
        StueckLinks,  ///< seine Weite, an der linken
        StueckRechts, ///< oder rechten Kante gezogen
    };
    /// Der Zeitbereich einer Blende des Clips in Pixeln; false = keine.
    [[nodiscard]] bool blendenBereich(int kanal, int clip, bool ein, QRect* clipRect, int* x0,
                                      int* x1, Blenden* blenden) const;
    [[nodiscard]] Griff griffBei(QPoint ort, int* kanal, int* clip, int* stueck = nullptr) const;
    void zieheZu(QPoint ort, Qt::KeyboardModifiers tasten);
    void brichZugAb();

    Griff m_griff = Griff::Keiner;
    bool m_zugLaeuft = false;        ///< die Maus hat sich seit dem Druck weit genug bewegt
    QPoint m_zugStart;
    std::int64_t m_zugVersatzMs = 0; ///< Abstand Maus – Clip-Start beim Druck
    int m_zugKanal = -1;             ///< Kanal und
    int m_zugClip = -1;              ///< Platz des Clips in `m_vorZug`
    int m_zugStueck = -1;            ///< das gezogene Stueck (Griffe der Stuecke)
    Komposition m_vorZug;            ///< der Stand beim Druck
    QPoint m_fallOrt{-1, -1};        ///< wohin ein Drag von aussen gerade zeigt (-1 = keiner)
    bool m_fallTon = false;          ///< der Drag traegt nur Songs
    QStringList m_fallDateien;       ///< was der Drag traegt
    Qt::KeyboardModifiers m_fallTasten = Qt::NoModifier;
    int m_markeKanal = -1;           ///< die Einfuegemarke: Kanal (-1 = keine)
    std::int64_t m_markeMs = 0;      ///< und Stelle

    /// Die Dateien an `ort` in `neu` legen — fuer das Fallenlassen und, auf
    /// einer Kopie, fuer den Schatten. false = dort faellt nichts.
    bool legeDateien(Komposition& neu, QPoint ort, const QStringList& dateien,
                     Qt::KeyboardModifiers tasten, int* kanal, int* platz, QStringList* passend,
                     std::vector<Schatten>* gelegt) const;
    [[nodiscard]] std::int64_t songLaenge(const QString& datei) const;
    mutable QHash<QString, std::int64_t> m_songLaengen;  ///< schon erfragt (der Schatten fragt je Bewegung)
    Teil m_teil;                     ///< das Teil im Curve Editor (markiert)

    /// Die Huellkurve eines Songs, wie die Audio-Engine sie liefert.
    struct Huelle
    {
        std::vector<float> werte;
        int msProWert = 0;
    };
    QStringList m_audioEndungen;
    SongLaenge m_songLaenge;
    QHash<QString, Huelle> m_huellen;
};

class KurvenFenster;

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
    [[nodiscard]] QSize sizeHint() const override { return {900, 300}; }

    /// Das Fenster, dessen Komposition das Panel zeigt. Ohne Aufruf sucht das
    /// Panel es beim Zeigen selbst.
    void setFenster(VisualizerWidget* fenster);
    [[nodiscard]] Sitzung* sitzung() const;
    [[nodiscard]] Zeitachse* zeitachse() const { return m_zeitachse; }
    /// Das Fenster Curve Editor; nullptr, solange noch keine Kurve geoeffnet wurde.
    [[nodiscard]] KurvenFenster* kurvenFenster() const;

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
    /// Den Curve Editor zeigen (er entsteht beim ersten Mal) und an `teil` haengen.
    void oeffneKurve(Teil teil);
    /// Dem Curve Editor den Stand des angeklickten Clips geben.
    void zeigeKurve();
    /// Fuer jeden Song der Komposition, dessen Wellenform noch fehlt, die
    /// Huellkurve holen — einen nach dem anderen, in einem eigenen Thread (die
    /// Engine dekodiert dafuer die ganze Datei).
    void ladeHuellen();
    void starteHuelle();

    QStringList m_huellWarten;                         ///< Songs, deren Huellkurve noch aussteht
    QString m_huellLaeuft;                             ///< der, an dem der Thread arbeitet
    std::thread m_huellFaden;
    std::shared_ptr<std::atomic<bool>> m_huellAbbruch;

    QPointer<VisualizerWidget> m_fenster;
    QPointer<Sitzung> m_sitzung;
    QPointer<KurvenFenster> m_kurven;
    Teil m_kurvenTeil;

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
    QCheckBox* m_sync = nullptr;
    QPushButton* m_neu = nullptr;
    QPushButton* m_laden = nullptr;
    QPushButton* m_speichern = nullptr;
    QPushButton* m_speichernUnter = nullptr;
    QComboBox* m_mischart = nullptr;
    QCheckBox* m_endeFest = nullptr;
    QDoubleSpinBox* m_endeZeit = nullptr;
    QDoubleSpinBox* m_ueberlappung = nullptr;  ///< Loop overlap (S99)
    QCheckBox* m_songsDoppelt = nullptr;       ///< Songs overlap themselves (S99)
    int m_gezeigt = -1;  ///< was Schalter und Knoepfe zuletzt zeigten
};

}  // namespace lumi::composer
