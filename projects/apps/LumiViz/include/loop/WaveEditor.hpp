/**
 * @file WaveEditor.hpp
 * @brief Loop: der **Wave editor** — Datei, Ausschnitt, Lage, Gain, Blenden und die Spur
 *        Expression eines Wave-Musters, als eigenes Fenster (docs/audio/Loop_Konzept.md
 *        §4p, Stufe 15d; Skizze SK15-1 a gebilligt, Patrik S168)
 *
 * Wie Drum- und Instrument-Editor: ein Fenster, die Basis wechselt darin. **Kopf:** Base
 * mit der Zahl ihrer Clips, Fit, Bars, Play pattern, FX, Peak. **Zeile Datei:** File (die
 * Nennung, Laenge, Rate, Kanaele), Browse..., Record..., Edit sample..., Gain, Fade in, Fade
 * out. **Zeile Tempo:** Reference tempo, Set to loop tempo, Follow tempo, Pitch method.
 * **Zeile Raster:** das Raster des Loops (ohne Swing offset). **Flaeche** (`WaveFlaeche`)
 * ueber einem Taktlineal: oben die Spur Expression, in der Mitte die Welle, unten die Spur
 * Autotune.
 *
 * **Spur Autotune** (Stufe 15i, WV4 a): die gemessene Tonhoehe (pYIN, im Hintergrund gemessen und
 * je Datei gemerkt) gestrichelt, die korrigierte fein darueber, die Zielnoten als Kaesten; unter
 * der Flaeche Key, Scale, *Detect notes*, Strength, Speed und *Selected section only*. Findet
 * *Detect notes* eine Stimme, schlaegt der Editor *Voice* vor (WV6 a).
 *
 * *Follow tempo* (Stufe 15j) dehnt auf das Tempo des Loops; die Welle zeigt die Datei dann
 * gedehnt, die Griffe und Zielnoten folgen.
 *
 * Er aendert den Loop nicht selbst: er meldet die neue Fassung mit `bearbeitet`, das
 * Loop-Panel fuehrt sie ueber Undo aus und gibt sie mit `setLoop` zurueck.
 */
#pragma once

#include "loop/Bearbeiten.hpp"  // ClipOrt
#include "loop/Loop.hpp"
#include "loop/PYin.hpp"        // Tonhoehenverlauf
#include "loop/Tonauftrag.hpp"  // Dekodierer

#include <QPointer>
#include <QRectF>
#include <QString>
#include <QWidget>

#include <functional>
#include <memory>
#include <optional>
#include <vector>

class IAudioEngine;
class QPainter;
class QTimer;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QPushButton;
class QScrollBar;
class QSpinBox;

namespace lumi::loop
{

class AufnahmeFenster;
class RasterZeile;

/// Was von einer Tondatei zu zeigen ist: ihre Spitzen und was sie ist.
struct WaveBild
{
    /// Je `kSpitzenFrames` Abtastwerte der Betrag der groessten Spitze (beide Seiten), 0 … 1.
    std::vector<float> spitzen;
    double sekunden = 0.0;  ///< die Laenge der Datei; 0 = nicht zu lesen
    int rate = 0;           ///< ihre Abtastrate (aus dem Kopf einer WAV); 0 = unbekannt
    int kanaele = 0;        ///< ihre Kanaele (aus dem Kopf einer WAV); 0 = unbekannt

    /// So viele Abtastwerte bei `kBildRate` fasst eine Spitze.
    static constexpr int kSpitzenFrames = 256;
    /// In dieser Rate wird zum Zeigen dekodiert.
    static constexpr int kBildRate = 22050;
};

/// Das Bild einer Datei rechnen: dekodiert ueber `dekodierer` (bis `maxSekunden`), Rate und
/// Kanaele aus dem Kopf, wenn es eine WAV ist. Leer (sekunden 0) = nicht zu lesen.
[[nodiscard]] WaveBild waveBildVon(const QString& datei, const Dekodierer& dekodierer, double maxSekunden = 3600.0);
/// Die groesste Spitze zwischen den Sekunden `von` und `bis` der Datei; 0 ausserhalb.
[[nodiscard]] float waveSpitze(const WaveBild& bild, double von, double bis);

/// Die Flaeche: Taktlineal, Spuren Expression und Pitch, Welle, Spur Autotune. Sie rollt
/// selbst waagrecht (`setRoll`); ihr Muster haelt sie bis zum Loslassen, dann meldet sie es
/// einmal.
///
/// Welle: der Ausschnitt farbig, der Rest der Datei grau, ausserhalb des Musters
/// schraffiert. An seinen Kanten zieht man den Ausschnitt (der Ton bleibt, wo er ist); in
/// ihm verschiebt man die ganze Welle — sie rastet in Schritten, mit Shift in Bruchteilen.
/// Spuren wie im Instrument-Editor: ein Klick setzt einen Punkt auf den Schritt (mit Alt
/// frei) und zieht ihn weiter, ein Punkt laesst sich ziehen, ein Doppelklick nimmt ihn weg;
/// im Menue *Delete point* und *Clear lane*. Pitch (Stufe 15f) zeigt ±`kPitchBereich`
/// Halbtoene und rastet auf ganze Halbtoene, mit Shift auf Hundertstel. Ein Klick ins Lineal
/// waehlt die Stelle, an der *Play pattern* beginnt.
///
/// Spur Autotune (Stufe 15i): ein Klick waehlt eine Zielnote; ziehen verschiebt sie in der Zeit
/// und in Halbtoenen (mit Shift in Cent), an ihren Kanten kuerzt oder verlaengert man sie;
/// Ziehen auf freier Flaeche legt eine neue an (Ton unter der Maus, Strength und Speed der
/// Vorgabe); Ctrl+Klick teilt sie, ein Doppelklick oder Entf nimmt sie weg; im Menue *Split
/// here*, *Join with next*, *Delete note*, *Clear notes*.
class WaveFlaeche : public QWidget
{
    Q_OBJECT

public:
    explicit WaveFlaeche(QWidget* parent = nullptr);

    void setMuster(const Muster& muster, const Loop& loop);
    [[nodiscard]] const Muster& muster() const { return m_muster; }
    void setBild(WaveBild bild);
    [[nodiscard]] const WaveBild& bild() const { return m_bild; }
    /// Der gemessene Tonhoehenverlauf der Datei (Stufe 15i); leer = (noch) keiner.
    void setVerlauf(Tonhoehenverlauf verlauf);
    [[nodiscard]] const Tonhoehenverlauf& verlauf() const { return m_verlauf; }
    /// Die gewaehlte Zielnote (Platz in `autotune.abschnitte`); −1 = keine.
    [[nodiscard]] int gewaehlteNote() const { return m_note; }
    void waehleNote(int note);
    /// Die Abspielmarke im Muster (Schritt mit Bruchteil); unter null = keine.
    void setMarke(double schritt);
    [[nodiscard]] double marke() const { return m_marke; }

    // --- Lage und Masse (auch fuer Tests) ----------------------------------------
    [[nodiscard]] double schrittBreite() const { return m_schrittBreite; }
    void setSchrittBreite(double pixel);
    /// Das ganze Muster in der Breite.
    void passeEin();
    /// Der erste sichtbare Schritt.
    [[nodiscard]] double roll() const { return m_roll; }
    void setRoll(double schritt);
    [[nodiscard]] double schritte() const;
    [[nodiscard]] double xVonSchritt(double schritt) const;
    [[nodiscard]] double schrittVonX(double x) const;
    /// Wo die Sekunde `sekunde` der Datei im Muster liegt, in Schritten.
    [[nodiscard]] double schrittVonDatei(double sekunde) const;
    /// Das Ende des Ausschnitts in Sekunden der Datei (bei `ende` 0 ihr Ende).
    [[nodiscard]] double ausschnittEnde() const;

    static constexpr int kLinealHoehe = 18;
    static constexpr int kSpurHoehe = 40;
    static constexpr int kAutotuneHoehe = 120;
    /// Die Spur Autotune zeigt mindestens so viele Halbtoene (von Claude gesetzt, 15i).
    static constexpr int kAutotuneTonMin = 12;
    static constexpr int kGriff = 5;  ///< so weit neben einer Kante greift man sie
    /// Die Spur Pitch zeigt so viele Halbtoene nach oben und unten (von Claude gesetzt, 15f);
    /// was weiter reicht, steht gekappt am Rand.
    static constexpr double kPitchBereich = 12.0;
    [[nodiscard]] int spurOben(Steuerspur spur = Steuerspur::Expression) const
    {
        return kLinealHoehe + (spur == Steuerspur::Pitch ? kSpurHoehe : 0);
    }
    [[nodiscard]] int welleOben() const { return kLinealHoehe + 2 * kSpurHoehe; }
    [[nodiscard]] int welleUnten() const { return height() - kAutotuneHoehe; }
    /// Die Spur unter `y`: Expression oder Pitch; liegt `y` in keiner, ist `*getroffen` false.
    [[nodiscard]] Steuerspur spurBei(int y, bool* getroffen = nullptr) const;

    /// Der Wert der Spur unter `y` (Pitch auf ganze Halbtoene, mit `fein` auf Hundertstel); der
    /// Punkt unter dem Ort (−1 = keiner).
    [[nodiscard]] double spurWertVonY(int y, Steuerspur spur = Steuerspur::Expression, bool fein = false) const;
    [[nodiscard]] int spurPunktBei(QPoint ort, Steuerspur spur = Steuerspur::Expression) const;

    // --- Spur Autotune (Stufe 15i) -------------------------------------------------
    [[nodiscard]] int autotuneOben() const { return welleUnten(); }
    /// Der tiefste und hoechste gezeigte Ton (MIDI): aus Messung und Zielnoten, mindestens
    /// `kAutotuneTonMin` Halbtoene.
    [[nodiscard]] int tonUnten() const { return m_tonUnten; }
    [[nodiscard]] int tonOben() const { return m_tonOben; }
    [[nodiscard]] double yVonMidi(double midi) const;
    [[nodiscard]] double midiVonY(double y) const;
    [[nodiscard]] double xVonDatei(double sekunde) const { return xVonSchritt(schrittVonDatei(sekunde)); }
    [[nodiscard]] double dateiVonX(double x) const;
    /// Die Zielnote unter dem Ort (−1 = keine).
    [[nodiscard]] int noteBei(QPoint ort) const;
    /// Findet der Verlauf im Ausschnitt (oder zwischen `von` und `bis`, Sekunden der Datei)
    /// mindestens einen gesungenen Rahmen? Ohne Verlauf false.
    [[nodiscard]] bool gesungenImAusschnitt() const;
    [[nodiscard]] bool gesungenZwischen(double von, double bis) const;

Q_SIGNALS:
    /// Ein Zug ist fertig: das neue Muster und ein Wort fuer Undo.
    void geaendert(const lumi::loop::Muster& neu, const QString& was);
    /// Ein Klick ins Lineal: dort beginnt *Play pattern*.
    void abspielstelleGewaehlt(int schritt);
    /// Die Breite eines Schritts hat sich geaendert (Ctrl+Rad, Fit) — der Rollbalken folgt.
    void massGeaendert();
    /// Eine andere Zielnote ist gewaehlt (−1 = keine) — Strength und Speed zeigen sie.
    void noteGewaehlt(int note);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    bool event(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    enum class Zug
    {
        Keiner,
        Anfang,  ///< die linke Kante des Ausschnitts
        Ende,    ///< die rechte Kante
        Lage,    ///< die ganze Welle
        Punkt,   ///< ein Punkt einer Spur (`m_spur`)
        Note,        ///< eine Zielnote, in Zeit und Ton
        NoteAnfang,  ///< ihre linke Kante
        NoteEnde,    ///< ihre rechte Kante
        NeueNote,    ///< eine neue, aufgezogen auf freier Flaeche
    };
    /// Was in der Spur Autotune unter dem Ort zu greifen ist.
    [[nodiscard]] Zug notenGriffBei(QPoint ort, int* note) const;
    void zieheNote(QPoint ort, bool fein);
    void berechneTonbereich();
    void zeichneAutotune(QPainter& p, const QRectF& spur);
    void notenMenue(QContextMenuEvent* event);
    /// Wert und Lage einer Spur: unten, oben, der Wert ohne Punkte.
    [[nodiscard]] static double spurUnten(Steuerspur spur);
    [[nodiscard]] static double spurOberGrenze(Steuerspur spur);
    [[nodiscard]] double yVonWert(Steuerspur spur, double wert) const;
    /// Was unter dem Ort in der Welle zu greifen ist.
    [[nodiscard]] Zug griffBei(QPoint ort) const;
    void zeigeZeiger(QPoint ort);
    void zieheWelle(QPoint ort, bool frei);
    void zieheSpurPunkt(QPoint ort, bool frei, bool fein);
    [[nodiscard]] double schrittMs() const { return m_schrittMs; }
    /// So viele Sekunden der Datei liegen in einem Schritt — mit *Follow tempo* mal den Faktor (15j).
    [[nodiscard]] double dateiJeSchritt() const { return m_schrittMs / 1000.0 * m_tempo; }
    void melde(const QString& was);

    Muster m_muster;
    Muster m_vorher;  ///< vor dem Zug — gemeldet wird nur, was sich geaendert hat
    double m_schrittMs = 125.0;  ///< ein Schritt beim Tempo des Loops
    double m_tempo = 1.0;        ///< Stufe 15j: `waveTempoFaktor` des Musters
    int m_schritteJeTakt = 16;
    WaveBild m_bild;
    double m_marke = -1.0;
    double m_schrittBreite = 12.0;
    double m_roll = 0.0;
    Zug m_zug = Zug::Keiner;
    int m_punkt = -1;          ///< beim Ziehen eines Punkts: sein Platz in der Spur
    Steuerspur m_spur = Steuerspur::Expression;  ///< die Spur dieses Punkts
    double m_zugSchritt = 0.0;  ///< wo der Zug begann, in Schritten
    // Stufe 15i
    Tonhoehenverlauf m_verlauf;
    int m_note = -1;            ///< die gewaehlte Zielnote
    int m_tonUnten = 48;
    int m_tonOben = 72;
    double m_zugSekunde = 0.0;  ///< wo ein Zug in der Spur Autotune begann, in Sekunden der Datei
    double m_zugMidi = 0.0;     ///< und auf welchem Ton
};

class WaveEditor : public QWidget
{
    Q_OBJECT

public:
    explicit WaveEditor(QWidget* parent = nullptr);
    ~WaveEditor() override;

    /// Diese Basis des Loops zeigen. false = es gibt sie nicht (oder sie ist kein
    /// Wave-Muster) — der Editor bleibt, wie er war.
    bool zeige(const Loop& loop, const QString& muster);
    /// Eine andere Fassung desselben Loops (nach einer Aenderung, nach Undo). Heisst die
    /// Basis jetzt anders, folgt der Editor ihr; gibt es sie nicht mehr, schliesst er sich.
    void setLoop(const Loop& loop);
    [[nodiscard]] const QString& muster() const { return m_muster; }
    [[nodiscard]] const Loop& loop() const { return m_loop; }
    [[nodiscard]] WaveFlaeche* flaeche() const { return m_flaeche; }
    [[nodiscard]] RasterZeile* rasterZeile() const { return m_rasterZeile; }

    /// Womit die Datei zum Zeigen dekodiert wird (das Loop-Panel reicht den der Engine).
    /// Ohne ihn zeigt die Flaeche keine Welle.
    void setDekodierer(Dekodierer dekodierer);
    /// Wer die Engine nennt, ueber die das Fenster *Record sample* spielt (Stufe 15e).
    void setEngineGeber(std::function<IAudioEngine*()> geber);
    /// *Edit sample...*: das eigene Sample im Fenster *Record sample* oeffnen (wie im
    /// Instrument-Creator). Speichert es, gilt danach die neue Fassung — unter neuem Namen
    /// (*Save as...*) nennt das Muster diesen; was vorn weggeschnitten wurde, rueckt den
    /// Ausschnitt mit, der Ton bleibt, wo er ist. false = kein eigenes Sample, nicht zu lesen
    /// oder laenger als das Fenster fasst. `zeigen` false fuellt es nur (fuer Tests).
    bool oeffneSampleBearbeiten(bool zeigen = true);
    [[nodiscard]] AufnahmeFenster* aufnahmeFenster() const;
    /// Ein Satz in der Zeile Datei: warum Play pattern gerade still ist. Leer = keiner.
    void setHinweis(const QString& text);

    /// Spielt gerade dieses Muster allein — und wo steht es (Schritt im Muster)?
    void setSpielt(bool spielt, double schritt);
    /// Die Marke allein, ohne den Knopf Play pattern; −1 = keine.
    void zeigeMarke(double schritt);
    /// Die Spitze der Summe, solange etwas spielt; unter null = keine Anzeige.
    void setSpitze(double spitze);
    /// Welcher Clip im Loop markiert ist (E1 a); leer = keiner.
    void setMarkierterClip(std::optional<ClipOrt> ort);
    [[nodiscard]] std::optional<ClipOrt> bezugsclip() const;
    /// Der Knopf FX: der Schalter des Bezugsclips. false = kein Bezugsclip, oder er stand schon so.
    bool setzeEffekteAn(bool an);

    // --- was die Bedienelemente ausloesen (auch fuer Tests) ------------------------
    void setzeTakte(int takte);
    /// Eine Nennung als Datei setzen (`user:`, `lib:` oder ein Pfad).
    void setzeDatei(const QString& nennung);
    /// Was Browse... nach der Rueckfrage S2 a tut: zu den eigenen Samples kopieren und
    /// `user:<name>` setzen, oder den Pfad. false = nicht zu kopieren; das Muster bleibt.
    bool nimmDatei(const QString& pfad, bool kopieren);
    void setzeGain(double db);
    void setzeFadeIn(double ms);
    void setzeFadeOut(double ms);
    void setzeBezugBpm(double bpm);
    /// *Set to loop tempo*.
    void setzeBezugAufLoop();
    /// *Follow tempo* (Stufe 15j): auf das Tempo des Loops gedehnt, die Tonhoehe bleibt.
    void setzeFolgtTempo(bool an);
    void setzeTonhoehenArt(TonhoehenArt art);

    // --- Spur Autotune (Stufe 15i) ---------------------------------------------------
    /// Key und Scale fuer *Detect notes*.
    void setzeTonart(int grundton, Skala skala);
    /// Strength (0 … 1) und Speed (ms): der gewaehlten Zielnote, ohne Wahl die Vorgabe am Muster.
    void setzeStaerke(double staerke);
    void setzeSpeed(double ms);
    /// *Detect notes*: im Ausschnitt — mit *Selected section only* und einer gewaehlten Note nur in
    /// ihrer Zeit — die Zielnoten neu. false = noch nichts gemessen oder nichts gefunden.
    bool erkenneNoten();
    void setzeNurGewaehlte(bool an);
    /// Ist der Verlauf gemessen? Und bis er es ist warten (Tests).
    [[nodiscard]] bool tonhoeheBereit() const;
    void warteAufTonhoehe();
    /// Hat *Detect notes* eine Stimme gefunden, waehrend *General* gilt? Dann steht *Use Voice* da.
    [[nodiscard]] bool stimmeVorgeschlagen() const;
    [[nodiscard]] QString autotuneHinweis() const;

    /// Wie die Datei in der Zeile steht: Titel, Laenge, Rate, Kanaele (fuer Tests).
    [[nodiscard]] QString dateiText() const;
    [[nodiscard]] QString dateiInfoText() const;
    /// Der Hinweis hinter Follow tempo: „plays 1.25× as fast when on“.
    [[nodiscard]] QString tempoHinweis() const;

    /// Groesse und Lage merken (wie die anderen Editoren); in der Test-Exe aus.
    static void setzeLageMerken(bool an);

Q_SIGNALS:
    /// Eine Aenderung ist fertig: die neue Fassung des Loops und ein Wort fuer Undo.
    void bearbeitet(const lumi::loop::Loop& neu, const QString& was);
    /// „Play pattern" / „Stop".
    void spielenVerlangt(bool spielen);
    /// Ein Klick ins Lineal: spielt das Muster allein, springt es dorthin.
    void abspielstelleGewaehlt(int schritt);
    /// *Edit sample...* hat die Datei neu geschrieben: wer spielt, laedt den Klang neu.
    void klaengeGeaendert();
    /// *Record...* (Stufe 15k-1): in dieses Muster aufnehmen, am laufenden Loop.
    void aufnahmeVerlangt();

protected:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    void setupUI();
    void zeigeAlles();
    void zeigeFx();
    void ladeBild();
    void passeRollbalkenAn();
    [[nodiscard]] const Muster* basis() const { return m_loop.findeMuster(m_muster); }
    /// Das Muster aendern und melden; false = es aenderte sich nichts.
    bool aendere(const QString& was, const std::function<void(Muster&)>& aenderung);
    /// Was das Fenster *Record sample* gespeichert hat.
    void nimmBearbeitetes(const QString& nennung, double vornWegMs);
    /// Stufe 15i: den Verlauf der Datei messen (im Hintergrund, gemerkt je Inhalt).
    void starteTonhoehe(const QString& datei, const QString& stand);
    void sieheNachTonhoehe();
    void zeigeAutotune();

    struct Messung;
    std::unique_ptr<Messung> m_messung;  ///< nullptr = es wird nicht gemessen
    QTimer* m_messFertig = nullptr;
    QString m_verlaufVon;  ///< fuer welche Datei (und welchen Stand) der Verlauf gilt
    bool m_stimmeVorschlag = false;
    QComboBox* m_grundton = nullptr;
    QComboBox* m_skala = nullptr;
    QPushButton* m_erkennen = nullptr;
    QSpinBox* m_staerke = nullptr;
    QSpinBox* m_speed = nullptr;
    QCheckBox* m_nurGewaehlte = nullptr;
    QLabel* m_autotuneHinweis = nullptr;
    QPushButton* m_stimmeNehmen = nullptr;

    std::function<IAudioEngine*()> m_engineGeber;
    QPointer<AufnahmeFenster> m_aufnahme;
    QString m_bearbeitet;  ///< das Sample, das *Edit sample...* geoeffnet hat
    QLabel* m_hinweis = nullptr;

    Loop m_loop;
    QString m_muster;
    bool m_spielt = false;
    bool m_gezeigt = false;
    std::optional<ClipOrt> m_markiert;
    Dekodierer m_dekodierer;
    QString m_bildVon;  ///< fuer welche Datei (und welchen Stand) das Bild gilt

    QLabel* m_basisName = nullptr;
    QLabel* m_nutzer = nullptr;
    QSpinBox* m_takte = nullptr;
    QPushButton* m_play = nullptr;
    QPushButton* m_fx = nullptr;
    QLabel* m_spitze = nullptr;
    QLabel* m_datei = nullptr;
    QLabel* m_dateiInfo = nullptr;
    QPushButton* m_suchen = nullptr;
    QPushButton* m_aufnehmen = nullptr;
    QPushButton* m_sampleBearbeiten = nullptr;
    QDoubleSpinBox* m_gain = nullptr;
    QSpinBox* m_fadeIn = nullptr;
    QSpinBox* m_fadeOut = nullptr;
    QDoubleSpinBox* m_bezug = nullptr;
    QPushButton* m_bezugAufLoop = nullptr;
    QCheckBox* m_folgt = nullptr;
    QLabel* m_folgtHinweis = nullptr;
    QComboBox* m_tonhoehe = nullptr;
    RasterZeile* m_rasterZeile = nullptr;
    WaveFlaeche* m_flaeche = nullptr;
    QScrollBar* m_rollX = nullptr;
    QLabel* m_hilfe = nullptr;
};

}  // namespace lumi::loop
