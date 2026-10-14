/**
 * @file FormFenster.hpp
 * @brief Klang aus einer Form: das Fenster „Edit shape" — Konturen zeichnen,
 *        schieben, ordnen, anhoeren und als Form speichern
 *        (docs/audio/Shape_Konzept.md §8a; Stufe 20a; Skizze gebilligt,
 *        Patrik S120; Entscheide Z1 a und Z2 a, Patrik S122)
 *
 * **Kopf:** „Edit shape – <Name>", daneben der Ort; rechts **Open SVG...**
 * und **Open image...** — in Stufe 20a noch grau (20b, 20c). Draw gibt es
 * nicht (Z2 a): gezeichnet wird mit Pen.
 * **Zeichenflaeche** (`FormFlaeche`): die Konturen in ihren Farben — gewaehlte
 * kraeftig, nicht gewaehlte duenn und gestrichelt, der Startpunkt gefuellt.
 * Darunter die Werkzeuge **Pen** und **Move point**; **Bridge** und **Show
 * image** sind grau (20d, 20c).
 * **Liste Contours:** Haken, Farbe, Platz und Name, der Anteil am Umlauf;
 * **Rename**, **Remove**, ↑ und ↓.
 * **Unten:** **From image** und **Use as** grau, **Preview** mit dem
 * Wellenzug eines Umlaufs (`FormVerlauf`), **Harmonics** und **Play**.
 * **Fuss:** „Used by: …", **Cancel**, **Save as...**, **Save** — wie im
 * Fenster Edit sample: Save ersetzt eine eigene Form nach einer Rueckfrage und
 * ist sonst grau; Save as... legt eine neue eigene an.
 *
 * Gespeichert wird mit `speichereEigeneZeichnung` (Kontur.hpp); danach zaehlt
 * das Fenster die Fassung der Instrumente weiter (`zaehleInstrumentFassung`),
 * damit gerechnete Toene mit der neuen Form neu entstehen. `gespeichert` nennt
 * die Form, wie eine Schicht sie nennt.
 */
#pragma once

#include "audio/IAudioEngine.hpp"
#include "loop/AufnahmeFenster.hpp"  // AusschnittQuelle
#include "shape/BildKonturen.hpp"
#include "shape/Kontur.hpp"
#include "shape/Tonform.hpp"

#include <QImage>
#include <QPainterPath>
#include <QPointF>
#include <QRectF>
#include <QTransform>
#include <QString>
#include <QWidget>

#include <memory>
#include <optional>
#include <utility>
#include <vector>

class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QPushButton;
class QRadioButton;
class QSlider;
class QToolButton;
class QTreeWidget;
class QTimer;
class QTreeWidgetItem;

namespace lumi::loop
{

/// Die Zeichenflaeche: zeigt eine Zeichnung und nimmt Pen und Move point an.
/// Sie aendert die Zeichnung nicht selbst — sie meldet, was geschehen soll.
class FormFlaeche : public QWidget
{
    Q_OBJECT

public:
    enum class Werkzeug
    {
        Stift,     ///< Pen: ziehen = frei Hand, Klick = Eckpunkt
        Schieben,  ///< Move point
        Bruecke,   ///< Bridge: ein Punkt auf einer Kontur, dann einer auf einer anderen (G2 a)
    };

    explicit FormFlaeche(QWidget* parent = nullptr);
    /// Die Zeichnung, die zu sehen ist (gehoert dem Fenster); nullptr = keine.
    void setZeichnung(const shape::Zeichnung* zeichnung);
    /// Der Ausschnitt der Zeichnung, der die Flaeche fuellt (mit gleichem Mass in x und y).
    void setAnsicht(const QRectF& ansicht);
    [[nodiscard]] const QRectF& ansicht() const { return m_ansicht; }
    void setWerkzeug(Werkzeug werkzeug);
    [[nodiscard]] Werkzeug werkzeug() const { return m_werkzeug; }
    /// Diese Kontur hervorheben (die in der Liste gewaehlte); 0 = keine.
    void setHervorgehoben(int kennung);
    /// Das Bild blass im Hintergrund (Show image): `bild` in seinen Punkten, `lage`
    /// legt es in die Zeichnung; nullptr = keins.
    void setBild(const QImage* bild, const QTransform& lage, bool zeigen);
    /// Konturen, die nur vorgeschlagen sind (From image ohne Apply live): duenn und grau.
    void setVorschau(std::vector<QPainterPath> vorschau);
    /// Die Spruenge des Stifts (One path): grau gestrichelt mit zwei Punkten.
    void setSpruenge(std::vector<std::pair<QPointF, QPointF>> spruenge);
    /// Bridge: der erste Punkt einer Bruecke, die gerade entsteht; nullopt = keiner.
    void setBrueckeAnfang(std::optional<QPointF> punkt);

    /// Ein Punkt der Zeichnung im Bild und zurueck.
    [[nodiscard]] QPointF imBild(QPointF punkt) const;
    [[nodiscard]] QPointF inZeichnung(QPointF ort) const;
    /// So viele Einheiten der Zeichnung macht ein Bildpunkt.
    [[nodiscard]] double einheitJePunkt() const;

    // --- Pen (auch fuer Tests; Punkte in den Koordinaten der Zeichnung) ---------
    /// Die Kontur, die gerade entsteht; leer = keine.
    [[nodiscard]] const QPainterPath& angefangen() const { return m_angefangen; }
    /// Ein Klick: ein Eckpunkt — oder, auf dem ersten Punkt, schliessen.
    void klicke(QPointF punkt);
    /// Ein Zug frei Hand; endet er am ersten Punkt, schliesst er die Kontur.
    void ziehe(const std::vector<QPointF>& zug);
    /// Schliessen (Doppelklick, Enter). false = daraus wird keine Kontur.
    bool schliesse();
    /// Die angefangene Kontur verwerfen (Esc).
    void verwirf();

    /// So nah (in Bildpunkten) muss die Maus an einem Punkt sein, der getroffen werden soll.
    static constexpr double kFangRadius = 8.0;
    /// So weit (in Bildpunkten) darf ein Zug frei Hand nach dem Glaetten abweichen.
    static constexpr double kGlaetten = 2.0;
    /// Weiter als das (in Bildpunkten) bewegt: kein Klick, ein Zug.
    static constexpr double kZugAb = 4.0;

Q_SIGNALS:
    /// Pen hat eine Kontur geschlossen.
    void konturFertig(const QPainterPath& pfad);
    /// Move point: der Ankerpunkt `anker` der Kontur `kennung` soll nach `ziel`.
    void ankerGezogen(int kennung, int anker, QPointF ziel);
    /// Move point: losgelassen — erst jetzt lohnt es, neu zu rechnen.
    void ankerLosgelassen();
    /// Pen hat etwas angefangen oder verworfen (der Hinweis unter der Flaeche).
    void angefangenGeaendert();
    /// Bridge: hier wurde geklickt (Koordinaten der Zeichnung).
    void brueckeGeklickt(QPointF punkt);
    /// Bridge: Esc — die angefangene Bruecke verwerfen.
    void brueckeVerworfen();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    /// Der Ankerpunkt unter dem Ort (Bildpunkte): Kennung und Platz; {0, -1} = keiner.
    [[nodiscard]] std::pair<int, int> ankerBei(QPointF ort) const;
    void stelleZeiger(QPointF ort);
    /// Das Mass: Bildpunkte je Einheit der Zeichnung, und wo ihr Ursprung liegt.
    [[nodiscard]] double mass() const;

    const shape::Zeichnung* m_zeichnung = nullptr;
    const QImage* m_bild = nullptr;
    QTransform m_bildLage;
    bool m_zeigeBild = true;
    std::vector<QPainterPath> m_vorschau;
    std::vector<std::pair<QPointF, QPointF>> m_spruenge;
    std::optional<QPointF> m_brueckeAnfang;
    QRectF m_ansicht{0.0, 0.0, 200.0, 200.0};
    Werkzeug m_werkzeug = Werkzeug::Stift;
    int m_hervorgehoben = 0;
    QPainterPath m_angefangen;
    // die Maus
    bool m_gedrueckt = false;
    QPointF m_druckOrt;              ///< Bildpunkte
    std::vector<QPointF> m_zug;      ///< Zeichnung: der Zug, solange gedrueckt
    bool m_istZug = false;
    QPointF m_maus{-1.0, -1.0};      ///< Bildpunkte; < 0 = nicht ueber der Flaeche
    int m_ziehKennung = 0;           ///< Move point: welche Kontur, welcher Punkt
    int m_ziehAnker = -1;
};

/// „Preview": ein Umlauf des Stifts ueber der Zeit — links die Bewegung zur
/// Seite, rechts die nach oben, mit einem Strich, wo die naechste Kontur beginnt.
class FormVerlauf : public QWidget
{
    Q_OBJECT

public:
    explicit FormVerlauf(QWidget* parent = nullptr);
    void setForm(const std::shared_ptr<const shape::Tonform>& form, double harmonische);
    /// Die beiden Kurven, je ein Wert zwischen −1 und 1 je Stelle (fuer Tests).
    [[nodiscard]] const std::vector<double>& links() const { return m_links; }
    [[nodiscard]] const std::vector<double>& rechts() const { return m_rechts; }
    /// Wo eine neue Kontur beginnt, als Anteil des Umlaufs.
    [[nodiscard]] const std::vector<double>& grenzen() const { return m_grenzen; }
    [[nodiscard]] QSize sizeHint() const override { return {240, 90}; }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    std::vector<double> m_links;
    std::vector<double> m_rechts;
    std::vector<double> m_grenzen;
};

class FormFenster : public QWidget
{
    Q_OBJECT

public:
    explicit FormFenster(QWidget* parent = nullptr);
    ~FormFenster() override;

    /// Die Engine, ueber die Play spielt; nullptr = keine (Play ist dann aus).
    /// Sie muss das Fenster ueberleben, oder vorher mit nullptr abgemeldet werden.
    void setEngine(IAudioEngine* engine);

    /// **New shape...:** eine leere Zeichnung, Pen gewaehlt.
    void beginneNeu();
    /// **Edit...:** eine Form oeffnen, wie eine Schicht sie nennt. false = nicht
    /// gefunden oder nicht lesbar; das Fenster bleibt, wie es war.
    bool bearbeite(const QString& nennung);

    /// Woher stammt, was offen ist?
    enum class Herkunft
    {
        Neu,           ///< New shape: noch ohne Namen
        Eigene,        ///< eine eigene Form (`user:<name>`): Save ersetzt sie
        Mitgeliefert,  ///< eine mitgelieferte Form: schreibgeschuetzt
        Datei,         ///< eine `.lvcontour` an freiem Ort: schreibgeschuetzt
    };
    [[nodiscard]] Herkunft herkunft() const;
    /// Die Form, die offen ist; leer = eine neue.
    [[nodiscard]] const QString& nennung() const { return m_nennung; }
    /// Die Zeichnung, wie sie gerade steht.
    [[nodiscard]] const shape::Zeichnung& zeichnung() const { return m_zeichnung; }
    /// Weicht sie von dem ab, was geoeffnet wurde?
    [[nodiscard]] bool geaendert() const { return !(m_zeichnung == m_vorlage); }

    // --- was die Bedienelemente ausloesen (auch fuer Tests) -------------------
    void setzeWerkzeug(FormFlaeche::Werkzeug werkzeug);
    /// Eine Kontur dazu (Pen hat sie geschlossen); sie ist danach in der Liste
    /// gewaehlt. Zurueck: ihre Kennung; 0 = die Zeichnung ist voll.
    int fuegeKonturHinzu(const QPainterPath& pfad);
    /// Der Haken in der Liste.
    void setzeGewaehlt(int kennung, bool an);
    /// **Rename.** Leer = der Name faellt weg („Contour").
    void benenne(int kennung, const QString& name);
    /// **Remove.** false = es gab sie nicht.
    bool entferne(int kennung);
    /// **↑ / ↓:** einen Platz hinauf (-1) oder hinunter (+1). false = am Rand.
    bool schiebe(int kennung, int richtung);
    /// **Move point.** false = es gibt den Punkt nicht.
    bool schiebeAnker(int kennung, int anker, QPointF ziel);
    /// Die in der Liste gewaehlte Kontur; 0 = keine.
    [[nodiscard]] int aktuelle() const;
    void waehle(int kennung);
    /// **Undo** (Ctrl+Z) und **Redo** (Ctrl+Y): jeder Schritt oben ist einer.
    bool rueckgaengig();
    bool wiederherstellen();

    // --- Open SVG... (Stufe 20b; Entscheide S1 a, O1 a, O2 a) ------------------
    enum class Einfuegen
    {
        Ersetzen,     ///< Replace: die bisherigen Konturen fallen weg; die Herkunft wird „SVG"
        Hinzufuegen,  ///< Add: ans Ende der Liste, eingepasst in den Ausschnitt; die Herkunft bleibt
    };
    /// Die Konturen eines SVG nehmen, ohne Dialog. Bei einer leeren Zeichnung
    /// gilt immer Ersetzen. Zurueck: so viele kamen dazu; 0 = das SVG hat keine
    /// sichtbare Kontur; -1 = keine SVG-Datei oder nicht lesbar. Ein Schritt
    /// fuer Undo.
    int nimmSvg(const QString& datei, Einfuegen art);
    /// So viele kuerzere Konturen sind beim letzten Holen weggefallen (die
    /// Zeile unter der Liste); 0 = keine.
    [[nodiscard]] int weggefallen() const { return m_weggefallen; }

    // --- Open image... und From image (Stufe 20c; Entscheide K1 a, K2 a, R1 a, R2 a) ---
    /// Ein Bild nehmen, ohne Dialog. Mit **Apply live** (R1 a) kommen seine
    /// Konturen gleich in die Zeichnung (bei einer leeren immer Ersetzen; bei
    /// Add eingepasst in den Ausschnitt); ohne (R2 a) nur als Vorschau, bis
    /// `uebernimmVorschau`. Zurueck: so viele Konturen kamen dazu (ohne Apply
    /// live: so viele schlaegt die Vorschau vor); -1 = kein Bild.
    int nimmBild(const QString& datei, Einfuegen art);
    int nimmBild(const QImage& bild, const QString& name, Einfuegen art);
    /// Ist ein Bild da (die Regler von From image wirken)? Nach dem Oeffnen einer
    /// Form oder New shape nicht mehr — das Bild steht nicht in der Datei.
    [[nodiscard]] bool hatBild() const { return !m_bildDaten.isNull(); }
    /// Die Regler setzen und gleich neu erkennen (ohne die Pause der Regler).
    void setzeBildEinstellung(const shape::BildEinstellung& einstellung);
    [[nodiscard]] const shape::BildEinstellung& bildEinstellung() const { return m_bildEinstellung; }
    /// **Apply live** (R2 a): an = die Regler ersetzen die Konturen aus dem Bild; aus = Vorschau und Take contours.
    void setzeLive(bool an);
    [[nodiscard]] bool live() const { return m_live; }
    /// **Take contours:** die Vorschau uebernehmen (O1 a). Zurueck: so viele kamen dazu.
    int uebernimmVorschau(Einfuegen art);
    /// Was ohne Apply live vorgeschlagen ist.
    [[nodiscard]] const shape::BildKonturen& vorschau() const { return m_vorschauKonturen; }
    /// Stammt die Kontur aus dem Bild, solange es da ist (die Regler ersetzen sie)?
    [[nodiscard]] bool ausBild(int kennung) const;
    /// Stoesst die Kontur an den Rand ihres Bilds (die Liste zeigt „edge")?
    [[nodiscard]] bool amRand(int kennung) const;

    // --- Use as und Bridge (Stufe 20d; Entscheide Z5 a, B1, G1 a, G2 a) ----------
    /// **Use as:** Layers, one per contour — oder One path. Ein Schritt fuer Undo.
    void setzeNutzung(shape::Nutzung nutzung);
    /// **Jumps** (nur bei One path): In list order, Shortest, Drawn by hand.
    void setzeSpruenge(shape::Spruenge spruenge);
    /// Eine Bruecke von Hand setzen, ohne Maus. false = dieselbe Kontur, eine
    /// gibt es nicht, oder sie schloesse einen Ring (G2 a). Sonst stellt sie
    /// Use as auf One path und Jumps auf Drawn by hand.
    bool setzeBruecke(int von, QPointF vonPunkt, int nach, QPointF nachPunkt);
    /// Die Bruecke an diesem Platz in `zeichnung().bruecken` entfernen.
    bool entferneBruecke(int platz);
    /// **Bridge** mit der Maus: der erste Klick setzt den Anfang auf eine
    /// Kontur, der zweite das Ende auf eine andere; ein Klick auf eine
    /// bestehende Bruecke entfernt sie (ohne angefangene).
    void brueckeKlick(QPointF punkt);
    /// Was Bridge zuletzt sagt (ein Ring wird nicht angenommen, ...).
    [[nodiscard]] const QString& brueckenMeldung() const { return m_brueckenMeldung; }
    /// Die Zeile unter Use as: „2 contours · 2 jumps".
    [[nodiscard]] QString zahlenZeile() const;

    /// **Harmonics** der Preview: 1 … `shape::kHarmonischeMax`.
    void setzeHarmonische(double harmonische);
    [[nodiscard]] double harmonische() const { return m_harmonische; }
    /// Was die Preview zeigt und Play spielt: alle Konturen mit Haken als ein
    /// Pfad; nullptr = keine Kontur mit Haken.
    [[nodiscard]] std::shared_ptr<const shape::Tonform> form() const { return m_form; }
    /// Der Anteil der Kontur am Umlauf (0 … 1); -1 = sie spielt nicht mit.
    [[nodiscard]] double anteil(int kennung) const;
    /// **Play:** einen Umlauf immer wieder, links die Bewegung zur Seite, rechts
    /// die nach oben (wie Stereo XY), bei `kPlayHz`. false = keine Engine oder
    /// keine Kontur mit Haken.
    bool spiele();
    void halte();
    [[nodiscard]] bool spielt() const { return m_mixer != INVALID_STREAM; }
    /// Was Play spielt: eine Sekunde, Stereo, bei `kPlayRate` (fuer Tests).
    [[nodiscard]] std::vector<float> klang() const;
    static constexpr int kPlayRate = 48000;
    static constexpr int kPlayHz = 220;  ///< A3 — ganzzahlig, damit die Sekunde nahtlos wiederholt

    /// **Save** ohne Rueckfrage: die eigene Form ersetzen, die offen ist. Leer
    /// = keine eigene offen, keine Kontur, oder nicht zu schreiben; sonst die
    /// Nennung `user:<name>` — sie geht auch als `gespeichert` hinaus, und das
    /// Fenster schliesst.
    QString speichere();
    /// **Save as...** ohne Dialog: als neue eigene Form unter diesem Titel.
    /// Leer = keine Kontur, aus dem Titel wird kein Name, den Namen gibt es
    /// schon, oder nicht zu schreiben; sonst wie `speichere`.
    QString speichereAls(const QString& titel);
    /// Was Save as... als Namen vorschlaegt: „My shape", „My shape 2", …; fuer
    /// eine geoeffnete Form „Heart 2", „Heart 3", …
    [[nodiscard]] QString vorschlagTitel() const;
    /// Die Zeile unter dem Namen im Dialog von Save as...: die Datei, die
    /// daraus wird — oder warum es so nicht geht. `geht` sagt, ob Save an ist.
    [[nodiscard]] static QString dateiZeile(const QString& titel, bool* geht = nullptr);
    /// Der Satz, mit dem Save nachfragt — mit den eigenen Instrumenten, die die
    /// Form spielen.
    [[nodiscard]] QString ersetzenFrage() const;
    /// Die Zeile links im Fuss: „Used by: …" oder leer.
    [[nodiscard]] QString nutzerZeile() const;

    /// Die Teile (fuer Tests).
    [[nodiscard]] FormFlaeche* flaeche() const { return m_flaeche; }
    [[nodiscard]] FormVerlauf* verlauf() const { return m_verlauf; }
    [[nodiscard]] QTreeWidget* liste() const { return m_liste; }

Q_SIGNALS:
    /// Die Form liegt bei den eigenen und heisst so (`user:<name>`).
    void gespeichert(const QString& nennung);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void setupUI();
    /// Vor einer Aenderung: den Stand fuer Undo merken.
    void merke();
    /// Nach einer Aenderung: nicht mehr gesichert, alles neu gezeigt, Play spielt das Neue.
    void geaendertJetzt();
    /// Titel, Ort, Liste, Preview, Knoepfe aus `m_zeichnung`.
    void zeigeAlles();
    /// Der Regler Harmonics und seine Zahl.
    void zeigeHarmonische();
    void zeigeListe();
    void zeigeKnoepfe();
    void rechneForm();
    void frageUndSpeichere();
    void frageUndSpeichereAls();
    /// Open SVG...: die Datei waehlen, bei vorhandenen Konturen Replace, Add oder Cancel (O1 a).
    void frageNachSvg();
    /// Open image...: die Datei waehlen; mit Apply live und vorhandenen Konturen Replace, Add oder Cancel.
    void frageNachBild();
    /// Die Konturen des Bilds in die Zeichnung (ans Ende); merkt sie als die aus dem Bild. Zurueck: so viele.
    int setzeBildKonturen();
    /// Die Regler haben sich bewegt: live neu erkennen, sonst die Vorschau.
    void erkenneNeu();
    void rechneVorschau();
    void zeigeBildRegler();
    QImage m_bildDaten;  ///< das Bild zum Erkennen (verkleinert); leer = keins
    QString m_bildName;
    QTransform m_bildLage;  ///< Punkte des Bilds -> Zeichnung
    shape::BildEinstellung m_bildEinstellung;
    bool m_live = true;
    shape::BildKonturen m_vorschauKonturen;
    std::vector<int> m_bildKennungen;  ///< die Konturen aus dem Bild (R1 a)
    std::vector<int> m_randKennungen;  ///< Konturen, die an den Bildrand stossen
    QGroupBox* m_ausBild = nullptr;
    QComboBox* m_teilung = nullptr;
    QSlider* m_stufen = nullptr;
    QSlider* m_glaetten = nullptr;
    QSlider* m_mindest = nullptr;
    QSlider* m_detail = nullptr;
    QLabel* m_stufenWert = nullptr;
    QLabel* m_glaettenWert = nullptr;
    QLabel* m_mindestWert = nullptr;
    QLabel* m_detailWert = nullptr;
    QCheckBox* m_liveHaken = nullptr;
    QPushButton* m_nehmen = nullptr;
    QCheckBox* m_zeigeBild = nullptr;
    QTimer* m_bildTakt = nullptr;  ///< die Regler: erst nach einer kurzen Pause neu erkennen
    int m_weggefallen = 0;
    QLabel* m_weggefallenZeile = nullptr;
    /// Die Zeichnung unter diesem Namen ablegen, melden, schliessen.
    QString schreibe(const QString& name);
    /// Den Ausschnitt so legen, dass alle Konturen mit Rand hineinpassen.
    void passeAnsichtAn();
    [[nodiscard]] QTreeWidgetItem* zeileVon(int kennung) const;

    IAudioEngine* m_engine = nullptr;
    shape::Zeichnung m_zeichnung;
    shape::Zeichnung m_vorlage;  ///< wie geoeffnet
    QString m_nennung;
    bool m_gesichert = true;
    /// Ein Schritt fuer Undo: die Zeichnung und welche ihrer Konturen aus dem Bild stammen.
    struct Stand
    {
        shape::Zeichnung zeichnung;
        std::vector<int> bildKennungen;
        std::vector<int> randKennungen;
    };
    std::vector<Stand> m_zurueck;
    std::vector<Stand> m_vor;
    void nimmStand(Stand stand);
    bool m_schiebtGerade = false;  ///< Move point: der Stand ist zu Beginn des Ziehens gemerkt
    double m_harmonische = 24.0;
    std::shared_ptr<const shape::Tonform> m_form;

    AusschnittQuelle m_quelle;
    AudioStreamHandle m_mixer = INVALID_STREAM;
    AudioStreamHandle m_quellHandle = INVALID_STREAM;

    QLabel* m_ort = nullptr;
    QPushButton* m_svg = nullptr;
    QPushButton* m_bild = nullptr;
    FormFlaeche* m_flaeche = nullptr;
    QToolButton* m_stift = nullptr;
    QToolButton* m_schieben = nullptr;
    QToolButton* m_bruecke = nullptr;
    QLabel* m_hinweis = nullptr;
    QTreeWidget* m_liste = nullptr;
    QLabel* m_listenKopf = nullptr;
    QPushButton* m_umbenennen = nullptr;
    QPushButton* m_weg = nullptr;
    QPushButton* m_hinauf = nullptr;
    QPushButton* m_hinunter = nullptr;
    std::optional<std::pair<int, QPointF>> m_brueckeAnfang;  ///< Bridge: Kontur und Punkt des ersten Klicks
    QString m_brueckenMeldung;
    QLabel* m_zahlen = nullptr;
    /// Die Kontur, die dem Punkt am naechsten liegt, und der Punkt auf ihr; {0, …} = keine im Fangkreis.
    [[nodiscard]] std::pair<int, QPointF> konturBei(QPointF punkt) const;
    QRadioButton* m_nutzungSchichten = nullptr;
    QRadioButton* m_nutzungPfad = nullptr;
    QComboBox* m_spruenge = nullptr;
    FormVerlauf* m_verlauf = nullptr;
    QSlider* m_harmonischeRegler = nullptr;
    QLabel* m_harmonischeWert = nullptr;
    QPushButton* m_play = nullptr;
    QLabel* m_nutzer = nullptr;
    QPushButton* m_sichernAls = nullptr;
    QPushButton* m_sichern = nullptr;
};

}  // namespace lumi::loop
