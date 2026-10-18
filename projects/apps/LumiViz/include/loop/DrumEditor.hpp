/**
 * @file DrumEditor.hpp
 * @brief Loop: der Drum-Editor — das Schritt-Raster einer Basis, als eigenes
 *        Fenster (docs/audio/Loop_Konzept.md §4d, Stufe 6b)
 *
 * Oeffnet sich ueber „Edit base" am Clip (Entscheid Patrik S108, L8 a: die
 * Aenderung gilt fuer alle Clips der Basis). **Kopf:** Basis und Zahl ihrer
 * Clips, Takte des Musters, „Play pattern", die Spitze der Summe (L6 a).
 * **Raster des ganzen Loops:** Schlaege je Takt, Schritte je Schlag, Swing.
 * **Spuren:** je Spur Stumm, Name, Klang, Lautstaerke, Panorama und ihre
 * Schritte; darunter die Lautstaerke je Schritt der angeklickten Spur.
 *
 * **Der Klang einer Spur** (Stufe 11c, Schritt 2; Entscheid K1 a, Skizze
 * gebilligt S115): die Auswahl nennt die Klaenge des **Kit**, die eigenen
 * Samples unter **My samples**, und darunter **Instrument...** (das Fenster
 * Instruments), **Sample file...** (mit der Frage aus S2 a) und **Record
 * sample...** (das Fenster Record sample). Spielt die Spur ein Instrument,
 * steht sein Titel in der Auswahl, und der Knopf **…** daneben oeffnet ein
 * kleines Feld mit **Note**, **Length**, **Play**, **Browse...** und **Edit
 * instrument...**.
 *
 * Der Editor aendert den Loop nicht selbst: er meldet die neue Fassung mit
 * `bearbeitet`, das Loop-Panel fuehrt sie ueber Undo aus und gibt sie mit
 * `setLoop` zurueck.
 */
#pragma once

#include "loop/Auswahl.hpp"
#include "loop/Bearbeiten.hpp"  // ClipOrt
#include "loop/DrumKit.hpp"  // KitOptionen
#include "loop/Loop.hpp"
#include "UI/widgets/Achse.hpp"  // Mitlaufen

#include <QPoint>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QWidget>

#include <functional>
#include <optional>
#include <vector>

class IAudioEngine;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QMenu;
class QPushButton;
class QScrollArea;
class QSlider;
class QSpinBox;
class QToolButton;
class QVBoxLayout;
class QWheelEvent;

namespace lumi::loop
{

class AufnahmeFenster;
struct DrumKit;
class InstrumentCreator;
class InstrumentPalette;
class KitFenster;
class KlangAuswahl;
struct Instrument;
struct Schicht;

/// Das Raster: je Spur eine Zeile Schritte, darunter die Lautstaerke je Schritt
/// der angeklickten Spur. Ein Klick schaltet einen Schritt an (volle
/// Lautstaerke) oder aus; mit gedrueckter Taste weitergezogen bekommen die
/// ueberfahrenen Schritte derselben Spur denselben Zustand. In der unteren
/// Zeile zieht man die Lautstaerke. Gemeldet wird beim Loslassen, einmal.
class SchrittRaster : public QWidget
{
    Q_OBJECT

public:
    explicit SchrittRaster(QWidget* parent = nullptr);

    void setMuster(const Muster& muster, int schlaegeJeTakt, int schritteJeSchlag);
    [[nodiscard]] const Muster& muster() const { return m_muster; }
    void setSpur(int spur);
    [[nodiscard]] int spur() const { return m_spur; }
    /// Die Abspielmarke: der Schritt im Muster (mit Bruchteil); unter null = keine.
    void setMarke(double schritt);
    /// Weitere Stellen im Muster, an denen der Composer diesen Loop zugleich
    /// spielt — die leiseren Clips, blasser (S130, Entscheid Patrik W8 a).
    void setNebenMarken(std::vector<double> schritte);
    [[nodiscard]] const std::vector<double>& nebenMarken() const { return m_neben; }

    // --- Mehrere Schritte zugleich (Stufe 13b, Skizze §4i) -------------------
    // Markiert wird mit Shift+Ziehen (ein Bereich, auch ueber mehrere Spuren)
    // und Ctrl+Klick (ein Schritt dazu oder heraus). Ein Klick in die Zeile
    // der Takte setzt die Einfuegemarke. Alles Folgende loesen auch Tasten und
    // das Menue des Rechtsklicks aus.
    [[nodiscard]] const std::vector<SchrittOrt>& markierte() const { return m_marken; }
    [[nodiscard]] bool istMarkiert(int spur, int schritt) const;
    void markiere(const std::vector<SchrittOrt>& orte);
    void markiereUm(int spur, int schritt);
    void markiereAlle();
    /// Der Schritt, an dem Ctrl+V einfuegt (0 bis zur Laenge des Musters).
    /// Gesetzt mit Shift+Klick in die Zeile der Takte (Entscheid Patrik B4 b, S124).
    void setzeEinfuegemarke(int schritt);
    [[nodiscard]] int einfuegemarke() const { return m_einfuegen; }
    /// Wo **Play pattern** beginnt und wohin ein Klick in die Zeile der Takte
    /// springt, solange das Muster spielt (B4 b); 0 = der Anfang.
    void setzeAbspielstelle(int schritt);
    [[nodiscard]] int abspielstelle() const { return m_abspiel; }
    [[nodiscard]] bool hatKopie() const { return !m_kopie.leer(); }
    /// Die markierten Schritte merken. false = nichts markiert.
    bool kopiere();
    /// Merken und leeren.
    bool schneideAus();
    /// Das Gemerkte an der Einfuegemarke einfuegen, in der Spur, in der die
    /// kopierte Auswahl begann; die Marke rueckt um die eingefuegte Laenge
    /// weiter. `ersetzen` (Entscheid Patrik S113, V1 a) schreibt auch die
    /// leeren Schritte der Kopie; false legt nur dazu.
    bool fuegeEin(bool ersetzen = true);
    /// Dasselbe an einer bestimmten Stelle („Paste here"); die Marke bleibt.
    bool fuegeEinBei(SchrittOrt ziel, bool ersetzen = true);
    /// Die markierten Schritte direkt hinter sich selbst noch einmal einfuegen.
    bool dupliziere();
    bool leereMarkierte();
    /// Die markierten versetzen; am Rand haelt die ganze Auswahl an (V3 a).
    /// Mehrere Schritte hintereinander gehen von der Stelle aus, an der die
    /// Auswahl beim ersten stand: was sie unterwegs ueberfaehrt, bleibt.
    bool versetze(int spurVersatz, int schrittVersatz);
    /// Die markierten in der Zeit im Kreis versetzen (Alt+Pfeil).
    bool drehe(int schrittVersatz);
    /// Was beim letzten Einfuegen nicht gepasst hat; leer = alles.
    [[nodiscard]] const QString& meldung() const { return m_meldung; }
    /// Der Haken „Right-click deletes" (Wunsch Patrik S113): an = der
    /// Rechtsklick auf einen gesetzten Schritt schaltet ihn sofort aus; aus =
    /// er oeffnet immer das Menue, in dem es dafuer einen Eintrag gibt.
    void setRechtsklickLoescht(bool an) { m_rechtsLoescht = an; }
    [[nodiscard]] bool rechtsklickLoescht() const { return m_rechtsLoescht; }
    /// Einen Schritt ausschalten (das Menue: „Clear this step"). false = er war aus.
    bool leereSchritt(SchrittOrt ort);

    // --- Zoom (Entscheid Patrik E-Z1 a, S135) — dieselben Gesten wie die Zeitachse (S102) --
    // Ctrl+Rad zoomt die Zeit, Alt+Rad die Hoehe der Spuren, Ctrl+Alt+Rad beides, um die
    // Stelle unter der Maus. Die Hoehe geht nicht unter 100 %: links stehen in jeder
    // Spur Bedienelemente, die so hoch sind (von Claude gesetzt, S135). „Fit": das
    // ganze Muster in der Breite, die Spuren in Normalhoehe — sie sind immer alle zu sehen.
    static constexpr double kSchrittBreiteMin = 4.0;
    static constexpr double kSchrittBreiteMax = 96.0;
    static constexpr int kZeilenProzentMin = 100;
    [[nodiscard]] int schrittBreite() const { return static_cast<int>(m_schrittBreite + 0.5); }
    [[nodiscard]] int zeilenHoehe() const { return kZeilenHoehe * m_zeilenProzent / 100; }
    [[nodiscard]] int zeilenProzent() const { return m_zeilenProzent; }
    /// Die Zeit zoomen: `faktor` ueber 1 = hinein. `xFest` im Raster; die Stelle dort
    /// meldet `gezoomt` vorher und nachher — das Fenster rollt so, dass sie stehen bleibt.
    void zoomeZeit(double faktor, int xFest);
    /// Die Hoehe der Spuren in Prozent (100 … `Achse::kHoeheMax`); meldet `hoeheGeaendert`.
    void setZeilenProzent(int prozent);
    /// **Fit:** das ganze Muster auf `sichtBreite`, die Spuren in Normalhoehe.
    void passeEin(int sichtBreite);
    [[nodiscard]] bool zoomVerstellt() const { return schrittBreite() != kSchrittBreite || m_zeilenProzent != 100; }

    // --- Geometrie (auch fuer Tests) ----------------------------------------
    static constexpr int kKopfHoehe = 18;    ///< die Zeile mit den Takten
    static constexpr int kZeilenHoehe = 26;  ///< je Spur ohne Zoom — so hoch wie ihre Kopfzeile links
    static constexpr int kLuftHoehe = 30;    ///< zwischen den Spuren und der Lautstaerke (links: „Add track")
    static constexpr int kLautHoehe = 52;    ///< die Lautstaerke je Schritt
    static constexpr int kSchrittBreite = 12;  ///< ohne Zoom
    static constexpr int kSchrittLuft = 2;
    static constexpr int kSchlagLuft = 5;    ///< zusaetzlich zwischen zwei Schlaegen
    [[nodiscard]] int xVonSchritt(int schritt) const;
    /// Die Abspielmarke in Schritten des Musters; -1 = keine.
    [[nodiscard]] double marke() const { return m_marke; }
    /// Der Schritt unter `x`; -1 = keiner (eine Luecke, ausserhalb).
    [[nodiscard]] int schrittBei(int x) const;
    [[nodiscard]] int yVonSpur(int spur) const { return kKopfHoehe + spur * zeilenHoehe(); }
    /// Oberkante der Zeile mit der Lautstaerke je Schritt.
    [[nodiscard]] int lautOben() const;
    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSizeHint() const override { return sizeHint(); }

Q_SIGNALS:
    void spurGewaehlt(int spur);
    /// Die Schritte sind geaendert: das Muster danach und ein Wort fuer Undo.
    void geaendert(const lumi::loop::Muster& neu, const QString& was);
    /// Marken, Einfuegemarke oder Meldung haben sich geaendert.
    void auswahlGeaendert();
    /// Ein Klick in die Zeile der Takte hat die Abspielstelle gesetzt (B4 b).
    void abspielstelleGewaehlt(int schritt);
    /// E-Z1 a: die Zeit ist gezoomt — die Stelle unter der Maus lag bei `xVorher`
    /// im Raster und liegt jetzt bei `xNachher`.
    void gezoomt(int xVorher, int xNachher);
    /// E-Z1 a: die Spuren sind hoeher oder niedriger geworden.
    void hoeheGeaendert();

protected:
    void wheelEvent(QWheelEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    [[nodiscard]] int schritte() const;
    void maleBei(QPoint ort);
    /// Der Schritt, der `x` am naechsten liegt — auch in einer Luecke und am Rand.
    [[nodiscard]] int schrittNahe(int x) const;
    /// Die Spur unter `y`, auf die vorhandenen begrenzt; -1 = es gibt keine.
    [[nodiscard]] int spurNahe(int y) const;
    void rahmeBis(QPoint ort);
    /// Eine eigene Aenderung uebernehmen: zeigen, melden, die Marken setzen.
    void uebernimm(const Muster& neu, const std::vector<SchrittOrt>& marken, const QString& was);
    bool bewege(int spurVersatz, int schrittVersatz, bool imKreis);

    Muster m_muster;
    double m_schrittBreite = kSchrittBreite;  ///< E-Z1 a: der Zoom der Zeit
    int m_zeilenProzent = 100;                ///< E-Z1 a: der Zoom der Hoehe
    int m_radRest = 0;  ///< was vom Mausrad noch keine ganze Raste ergab (Hoehen-Zoom)
    /// Wie weit ein Schritt im Schnitt reicht (Breite, Luft, Anteil der Schlag-Luft).
    [[nodiscard]] double schrittMass() const;
    int m_schlaege = 4;
    int m_jeSchlag = 4;
    int m_spur = -1;
    double m_marke = -1.0;
    std::vector<double> m_neben;  ///< W8 a: die blasseren Marken

    /// Was die gedrueckte Maus gerade tut.
    enum class Zug
    {
        Keiner,
        Setzen,   ///< Schritte anschalten
        Loeschen, ///< Schritte ausschalten
        Laut,     ///< die Lautstaerke je Schritt ziehen
        Rahmen,   ///< mit Shift: einen Bereich markieren
    };
    Zug m_zug = Zug::Keiner;
    int m_zugSpur = -1;
    Muster m_vorZug;

    bool m_rechtsLoescht = false;
    bool m_rechtsGeloescht = false;  ///< der Rechtsklick hat eben einen Schritt geleert: kein Menue dazu
    std::vector<SchrittOrt> m_marken;
    std::vector<SchrittOrt> m_vorRahmen;  ///< die Marken beim Druck (mit Ctrl bleiben sie)
    SchrittOrt m_rahmenStart;
    int m_einfuegen = 0;
    int m_abspiel = 0;  ///< B4 b
    SchrittKopie m_kopie;
    int m_kopieSpur = 0;  ///< die Spur, in der die kopierte Auswahl begann
    QString m_meldung;
    // Mehrere Pfeiltasten hintereinander: gerechnet wird vom Stand beim ersten
    // Druck, mit dem ganzen Versatz seither.
    Muster m_schiebeBasis;
    std::vector<SchrittOrt> m_schiebeOrte;
    Muster m_schiebeErgebnis;
    std::vector<SchrittOrt> m_schiebeMarken;
    int m_schiebeSpur = 0;
    int m_schiebeSchritt = 0;
    bool m_schiebeImKreis = false;
    bool m_schiebeGilt = false;
};

class DrumEditor : public QWidget
{
    Q_OBJECT

public:
    explicit DrumEditor(QWidget* parent = nullptr);

    /// Diese Basis des Loops zeigen. false = es gibt sie nicht (oder sie ist
    /// kein Drumkit) — der Editor bleibt, wie er war.
    bool zeige(const Loop& loop, const QString& muster);
    /// Eine andere Fassung desselben Loops (nach einer Aenderung, nach Undo).
    /// Heisst die Basis jetzt anders, folgt der Editor ihr; gibt es sie nicht
    /// mehr, schliesst er sich.
    void setLoop(const Loop& loop);
    [[nodiscard]] const QString& muster() const { return m_muster; }
    [[nodiscard]] const Loop& loop() const { return m_loop; }
    [[nodiscard]] SchrittRaster* raster() const { return m_raster; }

    /// Die Klaenge des Kits fuer die Auswahl je Spur (`kit:<Name>`).
    void setKitKlaenge(const QStringList& klaenge);
    /// Spielt gerade dieses Muster allein — und wo steht es (Schritt im Muster)?
    void setSpielt(bool spielt, double schritt);
    /// Die Marke allein, ohne den Knopf Play pattern (S127, W1 a): das Arrangement
    /// oder der Composer spielt dieses Muster gerade; -1 = keine. Mit `art` folgt
    /// der Ausschnitt ihr (W3 a) — nicht, solange am Rollbalken gezogen wird.
    /// `neben`: die Stellen der leiseren Clips desselben Loops im Composer,
    /// blasser gezeichnet (S130, W8 a); der Ausschnitt folgt nur `schritt`.
    void zeigeMarke(double schritt, lumi::ui::Achse::Mitlaufen art, std::vector<double> neben = {});
    /// Wo das Raster gerade steht (der Rollbalken), in Pixeln — fuer Tests.
    [[nodiscard]] int rollX() const;
    /// Die Spitze der Summe, solange etwas spielt; unter null = keine Anzeige.
    void setSpitze(double spitze);

    // --- was die Bedienelemente ausloesen (auch fuer Tests) -----------------
    void setzeTakte(int takte);
    void setzeRaster(int schlaegeJeTakt, int schritteJeSchlag);
    void setzeSwing(double swing);
    void spurHinzu();
    void spurLoeschen(int spur);
    void spurUmbenennen(int spur, const QString& name);
    /// Die Spur an einen anderen Platz legen (S127, Entscheid Patrik 7b a) — auch
    /// durch Ziehen am Namen oder Rechtsklick „Move track up/down". Schritte,
    /// Klang und Auswahl gehen mit; ein Undo-Schritt.
    void spurVerschieben(int von, int nach);
    /// Die Zeile einer Spur (Name, Klang, Regler); nullptr = gibt es nicht. Fuer Tests.
    [[nodiscard]] QWidget* spurZeile(int spur) const;
    /// Die Spur, die gerade am Namen gezogen wird, und wo sie landete (-1 = keine).
    [[nodiscard]] int spurImZug() const { return m_spurZug; }
    [[nodiscard]] int spurZiel() const { return m_spurZiel; }
    /// Die Spur spielt diesen Klang: `kit:<Name>`, `user:<name>` oder der Pfad
    /// einer Tondatei. Spielte sie ein Instrument, spielt sie es danach nicht
    /// mehr — Note und Length stehen wieder auf ihrer Vorgabe.
    void setzeKlang(int spur, const QString& klang);
    void setzeLaut(int spur, double laut);
    void setzePan(int spur, double pan);
    void setzeStumm(int spur, bool stumm);

    // --- der Klang einer Spur: Instrument, Datei, Aufnahme (Stufe 11c, Schritt 2) ---
    /// Die Spur spielt dieses Instrument (genannt wie in einem Muster). Spielte
    /// sie bisher keins, steht die Note nach seiner Art (Bass und Drums C2,
    /// sonst C4) und Length auf 250 ms (K2 a); spielte sie schon eins, bleiben
    /// beide. Der Klang der Spur bleibt daneben stehen.
    void setzeSpurInstrument(int spur, const QString& nennung);
    /// **Note** und **Length** einer Instrument-Spur; bei einer anderen Spur geschieht nichts.
    void setzeNote(int spur, int hoehe);
    void setzeDauer(int spur, int ms);
    /// Eine gewaehlte Tondatei nehmen (Entscheid S2 a): zu den eigenen Samples
    /// kopiert (`user:<name>`) oder am Ort gelassen (ihr Pfad). false = die
    /// Kopie gelang nicht.
    bool nimmSampleDatei(int spur, const QString& pfad, bool kopieren);
    /// **Play** im Feld hinter „…": einen Schlag der Spur anspielen
    /// (`spurTonVerlangt`). false = die Spur spielt kein Instrument.
    bool spieleSpur(int spur);
    /// **Instrument...** und **Browse...**: das Fenster Instruments fuer diese
    /// Spur oeffnen (es entsteht beim ersten Mal); **Use** dort setzt das
    /// Instrument in die Spur. `zeigen` false fuellt es nur (fuer Tests).
    /// false = die Spur gibt es nicht.
    bool oeffnePalette(int spur, bool zeigen = true);
    /// Das Fenster; nullptr, solange es noch nie geoeffnet wurde.
    [[nodiscard]] InstrumentPalette* palette() const;
    /// **Edit instrument...**: den Creator mit dem Instrument der Spur oeffnen.
    /// Speichert er, nennt die Spur danach, was er gespeichert hat. false =
    /// die Spur spielt kein Instrument, oder es laesst sich nicht finden.
    bool oeffneCreator(int spur, bool zeigen = true);
    [[nodiscard]] InstrumentCreator* creator() const;
    /// **Edit sound...** (Stufe 11c, Schritt 3; Entscheid K1 a): aus dem, was
    /// die Spur spielt, ein Instrument machen und im Creator oeffnen. Ein
    /// Klang des Kits, ein eigenes Sample oder eine Tondatei wird ein neues
    /// Instrument der Art Drums mit einer Sample-Schicht (`instrumentAusKlang`)
    /// — am Loop aendert sich nichts, bis der Creator speichert: dann spielt
    /// die Spur das Instrument, die Note auf dem Root note der Schicht, Length
    /// so lang wie die Aufnahme (K2 a). Spielt die Spur schon ein Instrument,
    /// ist es dasselbe wie Edit instrument.... false = die Spur spielt nichts.
    bool bearbeiteKlang(int spur, bool zeigen = true);
    /// Das Instrument, das „Edit sound..." aus einem Klang macht: eine
    /// Sample-Schicht, die die Aufnahme bei C4 spielt, wie sie ist.
    [[nodiscard]] static Instrument instrumentAusKlang(const QString& klang);
    /// Wer die Audio-Engine nennt, ueber die „Record sample" aufnimmt — hier
    /// und im Creator.
    void setEngineGeber(std::function<IAudioEngine*()> geber);
    /// **Record sample...**: das Fenster oeffnen (von vorn); was dort mit Save
    /// and use gespeichert wird, ist danach der Klang der Spur. false = die
    /// Spur gibt es nicht.
    bool oeffneAufnahme(int spur, bool zeigen = true);
    [[nodiscard]] AufnahmeFenster* aufnahmeFenster() const;
    /// Der Knopf **…** einer Instrument-Spur: das Feld mit Note, Length, Play,
    /// Browse..., Edit instrument.... false = die Spur spielt kein Instrument.
    bool oeffneDetail(int spur, bool zeigen = true);
    /// Das Feld; nullptr, solange es noch nie geoeffnet wurde.
    [[nodiscard]] QWidget* detail() const { return m_detail; }
    /// Die Spur, fuer die das Feld gerade gilt; -1 = keine.
    [[nodiscard]] int detailSpur() const { return m_detailSpur; }
    /// Was die Auswahl „Sound" einer Spur anbietet, von oben nach unten: je
    /// Klang seine Nennung, die Ueberschriften als `[Kit]`, `[My samples]`,
    /// die drei Wege darunter bei ihrem Text; das Instrument einer Spur als
    /// `instrument:<Nennung>`. Trennlinien fehlen.
    [[nodiscard]] QStringList klangZeilen(int spur) const;

    // --- Kits (Stufe 21b; Entscheide K1 a bis K5 a, Skizze gezeigt S123) -----
    // Im Kopf der Knopf **Kit: <Titel> ▾** (nach einer Aenderung von Hand mit
    // „(changed)", K4 a); sein Menue: Built-in kits, My kits, Save as kit...,
    // Edit kits... (das Fenster Kits, Stufe 21c). Darunter zwei Haken (K10 a, S124),
    // die das Programm merkt: Add all roles of the kit, Remove empty tracks the
    // kit doesn't have. Eine Spur, deren Rolle das Kit nicht hat, traegt im
    // Etikett ein * in der Hinweisfarbe; eine Zeile unter dem Raster sagt, was es
    // heisst (K11 a). Je Spur das Etikett **Role**: ein
    // Klick waehlt die Rolle; eine geratene steht in Klammern, eine, die das
    // Kit nicht hat, durchgestrichen (K2 a). **Add track** bietet die Rollen
    // des Kits an, die keine Spur hat.

    /// Das Kit anwenden (`wendeKitAn`, DrumKit.hpp): jede Spur bekommt den Klang
    /// ihrer Rolle, die Schritte bleiben; mit den Haken (`kitOptionen`) kommen
    /// fehlende Rollen dazu und leere fremde Spuren fallen weg — ein Schritt
    /// fuer Undo. false = kein Kit dieses Namens.
    bool wendeKitAn(const QString& nennung);
    /// Die Rolle einer Spur setzen; leer = keine. Eine ungueltige wird nicht gesetzt.
    void setzeRolle(int spur, const QString& rolle);
    /// Eine Spur fuer eine Rolle des Kits anlegen, mit dessen Klang und leeren
    /// Schritten (Add track, K2 a). -1 = das Muster nennt kein Kit, oder es hat die Rolle nicht.
    int spurAusKit(const QString& rolle);
    /// **Save as kit...**: die Spuren als eigenes Kit mit diesem Titel speichern
    /// (`kitAusMuster`); danach nennt das Muster es. Leer = nicht gespeichert
    /// (kein gueltiger Name, oder vergeben und `ueberschreiben` aus); sonst die Nennung.
    QString speichereAlsKit(const QString& titel, bool ueberschreiben = false);
    /// Was der Knopf Kit zeigt.
    [[nodiscard]] QString kitText() const;
    /// Die Rollen des Kits, die keine Spur hat (die Eintraege oben in Add track).
    [[nodiscard]] QStringList fehlendeRollen() const;
    /// Das Etikett Role einer Spur, wie es dasteht.
    [[nodiscard]] QString rollenEtikett(int spur) const;
    /// Steht das Etikett durchgestrichen (die Rolle fehlt im Kit)?
    [[nodiscard]] bool rolleFehltImKit(int spur) const;
    /// Das Menue des Knopfs Kit von oben nach unten: Ueberschriften als
    /// `[Built-in kits]`, `[My kits]`, je Kit seine Nennung (das gewaehlte mit
    /// `*` davor), dann die Eintraege bei ihrem Text.
    [[nodiscard]] QStringList kitZeilen() const;
    /// Was der letzte Kitwechsel getan hat (die Zeile unter dem Kopf); leer = keiner.
    [[nodiscard]] QString kitMeldung() const;
    /// Die beiden Haken im Menue (K10 a); das Programm merkt sie, beide sind zuerst an.
    [[nodiscard]] static KitOptionen kitOptionen();
    static void setzeKitOptionen(const KitOptionen& optionen);
    /// Die Zeile unter dem Raster, die das * erklaert; leer = keine Spur ohne Gegenstueck im Kit.
    [[nodiscard]] QString fremdZeile() const;
    // --- Groesse und Lage (S125, Offene_Punkte §6 Loop) ---------------------
    /// Das Fenster merkt Groesse und Lage beim Verstecken und stellt sie beim
    /// Bauen wieder her. Die Test-Exe schaltet das ab (sonst hinge der Ausgang
    /// der Tests an dem, was im Programm zuletzt stand).
    static void setzeLageMerken(bool an);
    /// Groesse und Lage jetzt merken (das Verstecken ruft es; fuer Tests).
    void merkeLage() const;

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
    /// **Edit kits...** (Stufe 21c, K7 a): das Fenster Kits oeffnen, auf dem Kit
    /// des Musters (sonst dem ersten). `zeigen` false fuellt es nur (fuer Tests).
    void oeffneKits(bool zeigen = true);
    /// Das Fenster; nullptr, solange es noch nie geoeffnet wurde.
    [[nodiscard]] KitFenster* kitFenster() const;
    /// Das Menue des Rechtsklicks auf eine Spur (Edit sound, Rename track..., Remove
    /// track); oeffentlich fuer Tests (S125), gezeigt ueber `lumi::ui::zeigeMenue`.
    void spurMenue(int spur, QPoint ort);
    /// Nach einem Import (S125): die Spuren mit ihrer Auswahl Sound, das Fenster
    /// Instruments und die Liste im Fenster Kits neu lesen.
    void liesEigeneNeu();

Q_SIGNALS:
    /// Eine Aenderung ist fertig: die neue Fassung des Loops und ein Wort fuer Undo.
    void bearbeitet(const lumi::loop::Loop& neu, const QString& was);
    /// „Play pattern" / „Stop": dieses Muster allein spielen, oder anhalten.
    void spielenVerlangt(bool spielen);
    /// Einen Schlag dieser Spur des Musters anspielen (Play im Feld hinter „…").
    void spurTonVerlangt(const QString& muster, int spur);
    /// Ein Klick in die Zeile der Takte (B4 b): spielt das Muster allein, springt es dorthin.
    void abspielstelleGewaehlt(int schritt);
    /// Creator und Fenster Instruments wollen einen Ton eines Instruments hoeren.
    void instrumentTonVerlangt(const QString& nennung, int hoehe);
    /// Der Creator spielt an (seit Stufe 24d mit den Noten nach *Play as*):
    /// `klingtMs` > 0 von „Repeat note" — so lang darf es klingen.
    void instrumentVorhoerenVerlangt(const QString& nennung, const Muster& noten, int klingtMs);
    /// Eine Datei, die ein Klang nennt, ist neu geschrieben (ein Instrument im
    /// Creator, eine Aufnahme unter demselben Namen): wer spielt, laedt neu.
    void klaengeGeaendert();
    /// Der Creator braucht die Daten einer Aufnahme: dekodieren und merken, noch im Signal.
    void sampleVerlangt(const QString& nennung);
    /// Das Fenster Instruments bittet um Undo (false) oder Redo (true).
    void rueckgaengigVerlangt(bool wiederholen);
    /// Das Fenster Kits will Schlaege hoeren: je Spur einen, einen Schlag nach dem anderen.
    void kitAnspielenVerlangt(const std::vector<lumi::loop::Spur>& spuren);

protected:
    /// Ein Klick in die Kopfzeile einer Spur waehlt sie.
    bool eventFilter(QObject* objekt, QEvent* ereignis) override;
    void hideEvent(QHideEvent* event) override;
    /// Esc bricht das Ziehen einer Spur ab.
    void keyPressEvent(QKeyEvent* event) override;

private:
    /// Die Bedienelemente einer Spur, links neben ihrer Zeile im Raster.
    struct SpurZeile
    {
        QWidget* reihe = nullptr;
        QToolButton* stumm = nullptr;
        QLabel* name = nullptr;
        QToolButton* rolle = nullptr;  ///< das Etikett Role (Stufe 21b)
        KlangAuswahl* klang = nullptr;
        QToolButton* mehr = nullptr;  ///< „…": nur an einer Instrument-Spur zu sehen
        QSlider* laut = nullptr;
        QSlider* pan = nullptr;
    };
    void setupUI();
    /// Kopf, Raster und Spuren aus `m_loop` neu fuellen.
    void zeigeAlles();
    void baueSpuren(const Muster& muster);
    /// E-Z1 a: die Kopfzeilen der Spuren so hoch wie die Spuren im Raster, die Flaeche darum mit.
    void passeSpurHoeheAn();
    /// Was in der Auswahl gewaehlt wurde: ein Klang — oder einer der drei Wege darunter.
    void klangGewaehlt(int spur, const QString& wahl);
    void frageNachSampleDatei(int spur);
    void waehleSpur(int spur);
    /// Das Feld hinter „…" aus der Spur fuellen; spielt sie kein Instrument mehr, schliesst es.
    void zeigeDetail();
    [[nodiscard]] const Muster* basis() const { return m_loop.findeMuster(m_muster); }
    /// Die Spur des gezeigten Musters; nullptr = es gibt sie nicht.
    [[nodiscard]] const Spur* spurBei(int spur) const;
    /// Eine Spur aendern und die neue Fassung melden. false = es gibt sie
    /// nicht, oder es hat sich nichts geaendert.
    bool aendereSpur(int spur, const QString& was, const std::function<void(Spur&)>& tu);

    /// Das Ziehen einer Spur am Namen (7b a): das Ziel unter der Maus bestimmen
    /// und den Strich setzen; `ende` = losgelassen (verschieben) oder abgebrochen.
    void zieheSpurZu(QPoint global);
    void beendeSpurZug(bool verschieben);

    Loop m_loop;
    QString m_muster;
    QStringList m_kitKlaenge;
    bool m_spielt = false;
    int m_spurZug = -1;            ///< die Spur, die am Namen gezogen wird (7b)
    int m_spurZiel = -1;           ///< der Platz, auf den sie fiele
    QPoint m_spurZugStart;         ///< wo gedrueckt wurde (global)
    QWidget* m_spurStrich = nullptr;  ///< der Strich, der zeigt, wo sie landet
    std::function<IAudioEngine*()> m_engineGeber;
    QPointer<InstrumentPalette> m_palette;
    QPointer<InstrumentCreator> m_creator;
    QPointer<AufnahmeFenster> m_aufnahme;
    QPointer<KitFenster> m_kits;
    int m_paletteSpur = -1;   ///< fuer diese Spur waehlt das Fenster Instruments; -1 = keine
    int m_creatorSpur = -1;   ///< das Instrument dieser Spur ist im Creator offen
    /// „Edit sound..." hat fuer diese Spur ein neues Instrument geoeffnet: beim
    /// ersten Save wird sie zur Instrument-Spur — wenn sie dann noch
    /// `m_wandelKlang` spielt. -1 = nichts steht aus.
    int m_wandelSpur = -1;
    QString m_wandelKlang;
    /// Den Creator anlegen, wenn es ihn noch nicht gibt.
    void stelleCreator();
    /// So lang ist die Aufnahme einer Sample-Schicht ab Sample start, in den Grenzen von Length.
    [[nodiscard]] int aufnahmeMs(const Schicht& schicht);
    int m_aufnahmeSpur = -1;  ///< fuer diese Spur nimmt „Record sample" auf
    QWidget* m_detail = nullptr;  ///< das Feld hinter „…"
    int m_detailSpur = -1;
    QComboBox* m_detailNote = nullptr;
    QSpinBox* m_detailDauer = nullptr;
    QLabel* m_detailTitel = nullptr;

    // --- Kits (Stufe 21b)
    QPushButton* m_kitKnopf = nullptr;
    QLabel* m_kitZeile = nullptr;
    QString m_kitMeldung;
    /// Das Kit, das das gezeigte Muster nennt. false = keines, oder es fehlt.
    bool aktuellesKit(DrumKit& kit) const;
    void zeigeKit();
    void fuelleKitMenue(QMenu* menue);
    void rollenMenue(int spur);
    void spurNeuGeklickt();

    QLabel* m_basisName = nullptr;
    QLabel* m_nutzer = nullptr;
    QSpinBox* m_takte = nullptr;
    QPushButton* m_play = nullptr;
    QLabel* m_spitze = nullptr;
    QSpinBox* m_schlaege = nullptr;
    QSpinBox* m_jeSchlag = nullptr;
    QDoubleSpinBox* m_swing = nullptr;
    QVBoxLayout* m_spurSpalte = nullptr;
    std::vector<SpurZeile> m_spuren;
    QPushButton* m_spurNeu = nullptr;
    QLabel* m_lautName = nullptr;
    QLabel* m_leerHinweis = nullptr;  ///< steht da, solange das Muster keine Spur hat
    QLabel* m_fremdZeile = nullptr;   ///< „* not in the kit …" (K11 a)
    QLabel* m_auswahlZeile = nullptr; ///< wie man markiert — oder was markiert ist und was die Tasten tun
    QCheckBox* m_rechtsLoescht = nullptr;  ///< „Right-click deletes"
    QCheckBox* m_wiederholen = nullptr;    ///< „Repeat when lengthening" (B2 a)
    std::optional<ClipOrt> m_markiert;  ///< E1 a: der im Loop markierte Clip
    QPushButton* m_fx = nullptr;         ///< E1 a: der Knopf FX
    void zeigeFx();
    void zeigeAuswahl();
    QScrollArea* m_rollen = nullptr;
    SchrittRaster* m_raster = nullptr;
};

}  // namespace lumi::loop
