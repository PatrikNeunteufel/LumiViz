/**
 * @file InstrumentEditor.hpp
 * @brief Loop: der Instrument-Editor — die Noten einer Basis ueber einer
 *        Klaviatur, als eigenes Fenster (docs/audio/Loop_Konzept.md §4f,
 *        Stufe 9b; Skizze gebilligt, Patrik S110)
 *
 * Oeffnet sich ueber „Edit base" an einem Instrument-Clip. **Kopf:** Basis und
 * Zahl ihrer Clips, Takte des Musters, „Play pattern", die Spitze der Summe.
 * **Instrument-Zeile:** Instrument, Lautstaerke, Panorama, Stumm. **Raster:**
 * links die Klaviatur (A0 bis C8, `kHoeheMin` … `kHoeheMax`), rechts die Schritte; eine Note ist ein
 * Balken in der Zeile ihrer Tonhoehe. Darunter die Lautstaerke je Note.
 * **Saiten (Stufe 23f; Skizze gebilligt, Patrik S133, S-U2 a):** traegt das
 * Instrument eine Tuning, steht in der Instrument-Zeile *String choice*, jede
 * Note zeigt ihre Saite, und unter dem Raster erklaert eine Zeile die Zeichen.
 *
 * Wie der Drum-Editor aendert er den Loop nicht selbst: er meldet die neue
 * Fassung mit `bearbeitet`, das Loop-Panel fuehrt sie ueber Undo aus und gibt
 * sie mit `setLoop` zurueck.
 */
#pragma once

#include "loop/Auswahl.hpp"
#include "loop/Bearbeiten.hpp"  // ClipOrt
#include "loop/Loop.hpp"
#include "loop/Saitenplan.hpp"  // Griff (Stufe 23f)
#include "UI/widgets/Achse.hpp"  // Mitlaufen

#include <QPoint>
#include <QPointer>
#include <QRect>
#include <QString>
#include <QWidget>

#include <functional>
#include <optional>
#include <utility>
#include <vector>

class IAudioEngine;
class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;
class QScrollBar;
class QSlider;
class QSpinBox;
class QToolButton;

namespace lumi::loop
{

class InstrumentCreator;
class InstrumentPalette;

/// Das Raster der Noten. Es rollt selbst — waagrecht (lange Muster) und
/// senkrecht (sechs Oktaven): Klaviatur, Kopf und die Zeile der Lautstaerken
/// bleiben stehen.
///
/// Ein Klick auf eine freie Stelle setzt eine Note von einem Schritt; mit
/// gedrueckter Taste nach rechts gezogen wird sie laenger. Eine Note zieht man
/// an ihrer rechten Kante laenger und kuerzer, sonst verschiebt man sie in
/// Zeit und Tonhoehe. Rechtsklick oder Entf loescht. Ein Klick auf eine Taste
/// verlangt ihren Ton. In der unteren Zeile zieht man die Lautstaerke der
/// Noten, die an dem Schritt beginnen. Gemeldet wird beim Loslassen, einmal.
class NotenRaster : public QWidget
{
    Q_OBJECT

public:
    explicit NotenRaster(QWidget* parent = nullptr);

    void setMuster(const Muster& muster, int schlaegeJeTakt, int schritteJeSchlag);
    [[nodiscard]] const Muster& muster() const { return m_muster; }
    /// Die Abspielmarke: der Schritt im Muster (mit Bruchteil); unter null = keine.
    void setMarke(double schritt);
    /// Weitere Stellen im Muster, an denen der Composer diesen Loop zugleich
    /// spielt — die leiseren Clips, blasser (S130, Entscheid Patrik W8 a).
    void setNebenMarken(std::vector<double> schritte);
    [[nodiscard]] const std::vector<double>& nebenMarken() const { return m_neben; }
    /// Die gewaehlte Note (Platz in `muster().noten`); -1 = keine.
    [[nodiscard]] int auswahl() const;
    /// Die gewaehlte Note loeschen — sind mehrere markiert, alle.
    bool loescheAuswahl();

    // --- Mehrere Noten zugleich (Stufe 13c, Skizze §4i) -----------------------
    // Markiert wird mit Shift+Ziehen (ein Rahmen) und Ctrl+Klick (eine Note
    // dazu oder heraus); ein Klick auf eine Note markiert nur sie. Zieht man
    // eine von mehreren markierten, gehen alle mit. Ein Klick in die Zeile der
    // Takte setzt die Einfuegemarke.
    [[nodiscard]] const std::vector<NotenOrt>& markierte() const { return m_marken; }
    [[nodiscard]] bool istMarkiert(const Note& note) const;
    void markiere(const std::vector<NotenOrt>& orte);
    void markiereUm(NotenOrt ort);
    void markiereAlle();
    void setzeEinfuegemarke(int schritt);
    [[nodiscard]] int einfuegemarke() const { return m_einfuegen; }
    /// Wo **Play pattern** beginnt (Entscheid Patrik B4 b, S124): ein Klick in
    /// die Zeile der Takte setzt sie, Shift+Klick die Einfuegemarke.
    void setzeAbspielstelle(int schritt);
    [[nodiscard]] int abspielstelle() const { return m_abspiel; }
    /// Der Zeitraum, den Ctrl+C mitnimmt (B3 a): die Kanten des Rahmens, mit
    /// Ctrl+A das ganze Muster; leer = die Noten selbst (Ctrl+Klick).
    [[nodiscard]] std::optional<std::pair<int, int>> zeitraum() const;
    [[nodiscard]] bool hatKopie() const { return !m_kopie.leer(); }
    bool kopiere();
    bool schneideAus();
    /// Das Gemerkte an der Einfuegemarke einfuegen, in seinen Tonhoehen; die
    /// Marke rueckt um die eingefuegte Laenge weiter. Markiert sind danach
    /// die eingefuegten Noten.
    bool fuegeEin();
    /// Dasselbe ab einem bestimmten Schritt („Paste here"); die Marke bleibt.
    bool fuegeEinBei(int schritt);
    /// Die markierten direkt hinter sich selbst noch einmal einfuegen.
    bool dupliziere();
    /// Die markierten versetzen; am Rand haelt die ganze Auswahl an
    /// (Entscheid Patrik S113, V3 a). Mehrere Schritte hintereinander gehen
    /// von der Stelle aus, an der die Auswahl beim ersten stand.
    bool versetze(int schrittVersatz, int hoeheVersatz);
    /// Die markierten in der Zeit im Kreis versetzen (Alt+Pfeil).
    bool drehe(int schrittVersatz);
    /// Was beim letzten Einfuegen oder Drehen nicht ganz passte; leer = alles.
    [[nodiscard]] const QString& meldung() const { return m_meldung; }
    /// Der Haken „Right-click deletes" (Wunsch Patrik S113): an = der
    /// Rechtsklick auf eine Note loescht sie sofort (so war es seit S110
    /// immer); aus = er markiert sie und oeffnet das Menue, in dem „Delete"
    /// steht.
    void setRechtsklickLoescht(bool an) { m_rechtsLoescht = an; }
    [[nodiscard]] bool rechtsklickLoescht() const { return m_rechtsLoescht; }

    // --- Saiten (Stufe 23f; Skizze gebilligt, Patrik S133: S-U2 a, S-U2b a) -----
    // Traegt das Instrument eine Tuning, zeigt jede Note die Nummer ihrer Saite
    // (1 = die hohe), eine selbst gesetzte eine Nadel davor, eine nicht
    // greifbare ist rot schraffiert mit „!"; die Tasten der leeren Saiten
    // tragen einen Punkt. Das Menue der Noten hat ein Untermenue String.
    /// Die Saiten des Instruments; leer = keine Saiten, nichts davon zu sehen.
    void setSaiten(const Saitensatz& saiten);
    [[nodiscard]] const Saitensatz& saiten() const { return m_saiten; }
    /// Je Note des Musters (in seiner Reihenfolge) der Griff, wie die Planung
    /// ihn waehlt (`greifeNoten`); ohne Saiten lauter leere.
    [[nodiscard]] std::vector<Griff> griffe() const;
    /// Wie eine Saite heisst (S-U2b a): die letzte der Liste ist 1.
    [[nodiscard]] static int saitenNummer(const Saitensatz& saiten, int stelle) { return saiten.anzahl() - stelle; }
    /// Die markierten Noten auf diese Saite setzen (Stelle in `Saitensatz::leer`),
    /// −1 = Auto. false = nichts markiert, keine Saiten, oder es aendert nichts.
    bool setzeSaite(int stelle);

    // --- Rollen ---------------------------------------------------------------
    void setRollen(int x, int y);
    [[nodiscard]] int rollX() const { return m_rollX; }
    /// Die Abspielmarke in Schritten des Musters; -1 = keine.
    [[nodiscard]] double marke() const { return m_marke; }
    [[nodiscard]] int rollY() const { return m_rollY; }
    /// So weit laesst sich rollen (0 = alles ist zu sehen).
    [[nodiscard]] int rollXMax() const;
    [[nodiscard]] int rollYMax() const;
    /// Breite und Hoehe des Ausschnitts, in dem die Noten liegen.
    [[nodiscard]] int sichtBreite() const { return width() - kTastenBreite; }
    [[nodiscard]] int sichtHoehe() const { return height() - kKopfHoehe - kLautHoehe; }
    /// So rollen, dass diese Tonhoehe in der Mitte liegt.
    void zeigeHoehe(int hoehe);

    // --- Geometrie (auch fuer Tests) ------------------------------------------
    static constexpr int kTastenBreite = 44;  ///< die Klaviatur links
    static constexpr int kKopfHoehe = 18;     ///< die Zeile mit den Takten
    static constexpr int kZeilenHoehe = 14;   ///< je Tonhoehe
    static constexpr int kSchrittBreite = 16;
    static constexpr int kLautHoehe = 56;     ///< die Lautstaerke je Note, unten
    static constexpr int kKante = 5;          ///< so breit ist der Griff am Ende einer Note mindestens
    static constexpr int kKanteMax = 9;       ///< und hoechstens (ein Drittel der Note, dazwischen)
    /// Liegt `x` auf dem Griff am Ende dieser Note? Dort zieht man ihre Laenge;
    /// der Mauszeiger zeigt es.
    [[nodiscard]] bool anKante(const Note& note, int x) const;
    [[nodiscard]] int xVonSchritt(int schritt) const { return kTastenBreite + schritt * kSchrittBreite - m_rollX; }
    /// Der Schritt unter `x`; -1 = keiner (die Klaviatur, hinter dem Ende).
    [[nodiscard]] int schrittBei(int x) const;
    [[nodiscard]] int yVonHoehe(int hoehe) const { return kKopfHoehe + (kHoeheMax - hoehe) * kZeilenHoehe - m_rollY; }
    /// Die Tonhoehe unter `y`; -1 = keine (der Kopf, die Zeile der Lautstaerken).
    [[nodiscard]] int hoeheBei(int y) const;
    /// Oberkante der Zeile mit der Lautstaerke je Note.
    [[nodiscard]] int lautOben() const { return height() - kLautHoehe; }
    /// Die Note unter dem Punkt (Platz in `muster().noten`); -1 = keine.
    [[nodiscard]] int noteBei(QPoint ort) const;
    [[nodiscard]] QSize sizeHint() const override { return {kTastenBreite + 64 * kSchrittBreite, 420}; }
    [[nodiscard]] QSize minimumSizeHint() const override { return {kTastenBreite + 8 * kSchrittBreite, 200}; }

Q_SIGNALS:
    /// Die Noten sind geaendert: das Muster danach und ein Wort fuer Undo.
    void geaendert(const lumi::loop::Muster& neu, const QString& was);
    /// Eine Taste der Klaviatur ist angeklickt.
    void tonVerlangt(int hoehe);
    /// Das Rad hat gerollt (die Rollbalken des Fensters ziehen nach).
    void gerollt();
    /// Marken, Einfuegemarke oder Meldung haben sich geaendert.
    void auswahlGeaendert();
    /// Ein Klick in die Zeile der Takte hat die Abspielstelle gesetzt (B4 b).
    void abspielstelleGewaehlt(int schritt);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    /// Der Tooltip einer Note: ihre Saite und ihr Bund — oder warum sie nicht greifbar ist.
    bool event(QEvent* event) override;

private:
    [[nodiscard]] int schritte() const { return m_muster.takte * m_schlaege * m_jeSchlag; }
    [[nodiscard]] bool belegt(int schritt, int hoehe, int ausser) const;
    void zieheBei(QPoint ort);

    Muster m_muster;
    int m_schlaege = 4;
    int m_jeSchlag = 4;
    double m_marke = -1.0;
    std::vector<double> m_neben;  ///< W8 a: die blasseren Marken
    int m_rollX = 0;
    int m_rollY = 0;
    /// Die gewaehlte Note, an ihrem Einsatz erkannt (die Plaetze aendern sich
    /// beim Sortieren); Schritt -1 = keine.
    int m_wahlSchritt = -1;
    int m_wahlHoehe = -1;

    /// Was die gedrueckte Maus gerade tut.
    enum class Zug
    {
        Keiner,
        Laenge,       ///< die rechte Kante der gewaehlten Note
        Verschieben,  ///< die gewaehlte Note in Zeit und Tonhoehe
        Laut,         ///< die Lautstaerke der Noten unter der Maus
        Rahmen,       ///< mit Shift: ein Rahmen, der Noten markiert
        Gruppe,       ///< mehrere markierte Noten zusammen in Zeit und Tonhoehe
    };
    /// Eine eigene Aenderung uebernehmen: zeigen, die Marken setzen, melden.
    void uebernimm(const Muster& neu, const std::vector<NotenOrt>& marken, const QString& was);
    bool bewege(int schrittVersatz, int hoeheVersatz, bool imKreis);
    /// Die Marken in Ordnung bringen: was es nicht mehr gibt, faellt heraus;
    /// die gewaehlte Note gehoert dazu.
    void richteMarken();
    void setzeMeldung(const NotenRest& rest);
    /// Schritt und Tonhoehe unter dem Punkt, auch ausserhalb des Rasters.
    [[nodiscard]] int schrittUnter(int x) const;
    [[nodiscard]] int hoeheUnter(int y) const;
    bool m_rechtsGeloescht = false;  ///< der Rechtsklick hat eben eine Note geloescht: kein Menue dazu
    bool m_rechtsLoescht = false;

    std::vector<NotenOrt> m_marken;
    std::vector<NotenOrt> m_vorMarken;  ///< die Marken beim Druck
    std::optional<std::pair<int, int>> m_vorZeitraum;  ///< der Zeitraum beim Druck (Ctrl+Rahmen erweitert ihn)
    /// B3 a: der Zeitraum des Rahmens — er gilt, solange genau diese Noten markiert sind.
    std::optional<std::pair<int, int>> m_zeitraum;
    std::vector<NotenOrt> m_zeitraumMarken;
    QPoint m_rahmenStart;
    QRect m_rahmen;          ///< der Rahmen, solange er gezogen wird
    bool m_klickSammelt = false;  ///< der Druck traf eine von mehreren markierten: ohne Zug bleibt nur sie
    int m_griffHoehe = 0;    ///< beim Ziehen der Gruppe: die Tonhoehe unter der Maus beim Druck
    int m_griffStelle = 0;   ///< und der Schritt
    int m_gutSchritt = 0;    ///< der letzte Versatz der Gruppe, der Platz hatte
    int m_gutHoehe = 0;
    int m_einfuegen = 0;
    int m_abspiel = 0;  ///< B4 b
    NotenKopie m_kopie;
    QString m_meldung;
    // mehrere Pfeiltasten hintereinander: gerechnet wird vom Stand beim ersten Druck
    Muster m_schiebeBasis;
    std::vector<NotenOrt> m_schiebeOrte;
    Muster m_schiebeErgebnis;
    std::vector<NotenOrt> m_schiebeMarken;
    int m_schiebeSchritt = 0;
    int m_schiebeHoehe = 0;
    bool m_schiebeImKreis = false;
    bool m_schiebeGilt = false;
    Zug m_zug = Zug::Keiner;
    int m_griffSchritt = 0;  ///< beim Verschieben: so weit hinter dem Einsatz wurde die Note gegriffen
    Muster m_vorZug;
    bool m_neu = false;      ///< der Zug begann mit einer neuen Note
    int m_taste = -1;        ///< die Taste der Klaviatur, die gerade gedrueckt ist; -1 = keine
    /// Den Mauszeiger setzen: an der Kante einer Note der Doppelpfeil, auf ihr
    /// das Kreuz, auf der Klaviatur die Hand.
    void zeigeZeiger(QPoint ort);
    Saitensatz m_saiten;  ///< Stufe 23f: die Saiten des Instruments; leer = keine
};

class InstrumentEditor : public QWidget
{
    Q_OBJECT

public:
    explicit InstrumentEditor(QWidget* parent = nullptr);

    /// Diese Basis des Loops zeigen. false = es gibt sie nicht (oder sie ist
    /// kein Instrument) — der Editor bleibt, wie er war.
    bool zeige(const Loop& loop, const QString& muster);
    /// Eine andere Fassung desselben Loops (nach einer Aenderung, nach Undo).
    /// Heisst die Basis jetzt anders, folgt der Editor ihr; gibt es sie nicht
    /// mehr, schliesst er sich.
    void setLoop(const Loop& loop);
    [[nodiscard]] const QString& muster() const { return m_muster; }
    [[nodiscard]] const Loop& loop() const { return m_loop; }
    [[nodiscard]] NotenRaster* raster() const { return m_raster; }

    /// Spielt gerade dieses Muster allein — und wo steht es (Schritt im Muster)?
    void setSpielt(bool spielt, double schritt);
    /// Die Marke allein, ohne den Knopf Play pattern (S127, W1 a): das Arrangement
    /// oder der Composer spielt dieses Muster gerade; -1 = keine. Mit `art` folgt
    /// der Ausschnitt ihr (W3 a) — nicht, solange am Rollbalken gezogen wird.
    /// `neben`: die Stellen der leiseren Clips desselben Loops im Composer,
    /// blasser gezeichnet (S130, W8 a); der Ausschnitt folgt nur `schritt`.
    void zeigeMarke(double schritt, lumi::ui::Achse::Mitlaufen art, std::vector<double> neben = {});
    /// Die Spitze der Summe, solange etwas spielt; unter null = keine Anzeige.
    void setSpitze(double spitze);
    /// Ein Satz unter der Instrument-Zeile: warum die Tasten gerade stumm
    /// bleiben (der Composer hat den Ton, der Loop spielt). Leer = kein Hinweis.
    void setHinweis(const QString& text);

    // --- was die Bedienelemente ausloesen (auch fuer Tests) -------------------
    void setzeTakte(int takte);
    void setzeInstrument(const QString& name);
    /// Was Load instrument... nach der Rueckfrage tut (Entscheid E3 a, S117):
    /// die Datei zu den eigenen kopieren und als `user:<name>` ins Muster
    /// setzen — oder sie am Ort lassen, dann steht ihr Pfad im Muster.
    /// false = nicht zu lesen oder nicht zu kopieren; das Muster bleibt.
    bool nimmInstrumentDatei(const QString& pfad, bool kopieren);
    void setzeLaut(double laut);
    void setzePan(double pan);
    void setzeStumm(bool stumm);
    /// *String choice* (Stufe 23f, S-U2 a): die Regel dieses Musters fuer Noten
    /// ohne eigene Saite. Die Auswahl steht nur da, wenn das Instrument Saiten traegt.
    void setzeSaitenwahl(Saitenwahl wahl);

    // --- der Instrument-Creator (Stufe 10b) -----------------------------------
    /// „Edit instrument...": den Creator mit dem Instrument dieser Basis
    /// oeffnen (er entsteht beim ersten Mal). Speichert er, nennt die Basis
    /// danach seine Datei. false = das Instrument laesst sich nicht finden.
    /// `zeigen` false fuellt ihn nur (fuer Tests ohne Fenster).
    bool oeffneCreator(bool zeigen = true);
    /// Das Fenster des Creators; nullptr, solange er noch nie geoeffnet wurde.
    [[nodiscard]] InstrumentCreator* creator() const;
    /// Wer die Audio-Engine nennt, ueber die der Creator ein Sample aufnimmt
    /// („Record...", Stufe 11b) — der Editor reicht es an den Creator weiter.
    void setEngineGeber(std::function<IAudioEngine*()> geber);

    // --- das Fenster „Instruments" (Stufe 14c) --------------------------------
    /// „Browse...": das Fenster oeffnen (es entsteht beim ersten Mal) und das
    /// Instrument dieser Basis darin waehlen. **Use** dort setzt das
    /// Instrument ins Muster. `zeigen` false fuellt es nur (fuer Tests).
    void oeffnePalette(bool zeigen = true);
    /// Das Fenster; nullptr, solange es noch nie geoeffnet wurde.
    [[nodiscard]] InstrumentPalette* palette() const;
    /// Nach einem Import (S125): das Fenster Instruments neu lesen.
    void liesEigeneNeu();
    /// Wie das Instrument einer Basis in der Zeile steht: der Titel; bei einem
    /// eigenen „Titel (My files)", bei einer Datei „Titel (datei.lvinst)",
    /// sonst „name (not found)".
    [[nodiscard]] static QString instrumentAnzeige(const QString& nennung);

Q_SIGNALS:
    /// Der Creator will einen Ton hoeren: das Instrument beim Pfad seiner
    /// Vorhoer-Datei.
    void instrumentTonVerlangt(const QString& nennung, int hoehe);
    /// Dasselbe von „Repeat note" im Creator: der Ton darf `klingtMs` lang klingen.
    void instrumentTonLangVerlangt(const QString& nennung, int hoehe, int klingtMs);
    /// Der Creator hat eine Instrument-Datei geschrieben: wer spielt, rechnet neu.
    void instrumentGespeichert();
    /// Der Creator braucht die Daten einer Aufnahme (Sample-Schicht, Stufe
    /// 11a): dekodieren und merken (`merkeSampleDaten`), noch im Signal.
    void sampleVerlangt(const QString& nennung);
    /// Das Fenster „Instruments" bittet um Undo (false) oder Redo (true) — es
    /// ist ein eigenes Fenster, das Kuerzel des Hauptfensters gilt dort nicht.
    void rueckgaengigVerlangt(bool wiederholen);
    /// Eine Aenderung ist fertig: die neue Fassung des Loops und ein Wort fuer Undo.
    void bearbeitet(const lumi::loop::Loop& neu, const QString& was);
    /// „Play pattern" / „Stop": dieses Muster allein spielen, oder anhalten.
    void spielenVerlangt(bool spielen);
    /// Ein Klick in die Zeile der Takte (B4 b): spielt das Muster allein, springt es dorthin.
    void abspielstelleGewaehlt(int schritt);
    /// Eine Taste der Klaviatur: diesen Ton des Instruments anspielen.
    void tonVerlangt(int hoehe);

public:
    // --- Effekte beim Abspielen (Entscheid Patrik E1 a, S124) ------------------
    // Play pattern spielt mit den Effekten des Bezugsclips: der im Loop
    // markierte, wenn er dieses Muster spielt, sonst der erste der Basis. Der
    // Knopf **FX** im Kopf schaltet dessen Reihe an oder aus — derselbe Schalter
    // wie im Loop (die Marke „fx" steht dann matt).
    /// Welcher Clip im Loop markiert ist (das Panel sagt es); leer = keiner.
    void setMarkierterClip(std::optional<ClipOrt> ort);
    /// Der Clip, dessen Effekte gelten; leer = die Basis hat keinen Clip.
    [[nodiscard]] std::optional<ClipOrt> bezugsclip() const;
    /// Der Knopf FX: den Schalter des Bezugsclips setzen (ein Schritt fuer Undo).
    /// false = kein Bezugsclip, oder er stand schon so.
    bool setzeEffekteAn(bool an);

    // --- Groesse und Lage (S125, Offene_Punkte §6 Loop) ---------------------
    /// Das Fenster merkt Groesse und Lage beim Verstecken und stellt sie beim
    /// Bauen wieder her. Die Test-Exe schaltet das ab (sonst hinge der Ausgang
    /// der Tests an dem, was im Programm zuletzt stand).
    static void setzeLageMerken(bool an);
    /// Groesse und Lage jetzt merken (das Verstecken ruft es; fuer Tests).
    void merkeLage() const;

protected:
    /// Beim ersten Zeigen: der Ausschnitt liegt auf den Noten (ohne Noten um C4).
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    void setupUI();
    void zeigeAusschnitt();
    void zeigeAlles();
    void passeRollbalkenAn();
    [[nodiscard]] const Muster* basis() const { return m_loop.findeMuster(m_muster); }

    Loop m_loop;
    QString m_muster;
    bool m_spielt = false;
    bool m_gezeigt = false;  ///< der Ausschnitt ist schon einmal gesetzt

    QLabel* m_basisName = nullptr;
    QLabel* m_nutzer = nullptr;
    QSpinBox* m_takte = nullptr;
    QPushButton* m_play = nullptr;
    QLabel* m_spitze = nullptr;
    QLabel* m_instrument = nullptr;  ///< der Name des Instruments (bis S113 eine Dropdown-Liste)
    QPushButton* m_instrumentSuchen = nullptr;  ///< „Browse..."
    QPushButton* m_instrumentBearbeiten = nullptr;
    QPushButton* m_instrumentLaden = nullptr;
    QPointer<InstrumentCreator> m_creator;
    std::function<IAudioEngine*()> m_engineGeber;
    QPointer<InstrumentPalette> m_palette;
    QSlider* m_laut = nullptr;
    QSlider* m_pan = nullptr;
    QToolButton* m_stumm = nullptr;
    QLabel* m_hinweis = nullptr;
    QLabel* m_auswahlZeile = nullptr;  ///< wie man markiert — oder was markiert ist und was die Tasten tun
    QCheckBox* m_rechtsLoescht = nullptr;  ///< „Right-click deletes"
    QCheckBox* m_wiederholen = nullptr;    ///< „Repeat when lengthening" (B2 a)
    std::optional<ClipOrt> m_markiert;  ///< E1 a: der im Loop markierte Clip
    QPushButton* m_fx = nullptr;         ///< E1 a: der Knopf FX
    void zeigeFx();
    void zeigeAuswahl();
    NotenRaster* m_raster = nullptr;
    QScrollBar* m_rollX = nullptr;
    QScrollBar* m_rollY = nullptr;
    // --- Stufe 23f (S-U2 a): String choice und die Hinweiszeile zu den Saiten
    QLabel* m_saitenWahlName = nullptr;
    QComboBox* m_saitenWahl = nullptr;
    QLabel* m_saitenHinweis = nullptr;
};

}  // namespace lumi::loop
