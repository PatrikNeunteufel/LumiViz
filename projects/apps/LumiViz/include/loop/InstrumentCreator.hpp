/**
 * @file InstrumentCreator.hpp
 * @brief Loop: der Instrument-Creator — die Bausteine eines gerechneten
 *        Instruments als Regler, als eigenes Fenster
 *        (docs/audio/Loop_Konzept.md §4f, Stufe 10b; Skizze gebilligt,
 *        Patrik S110; Entscheide I2 b und I3 a)
 *
 * **Kopf:** Name, die Vorlage, Reset, Save, Save as..., Level des ganzen
 * Instruments, Peak; darunter Type, Genres, Description und die Zeile
 * **Tuning** (`SaitenZeile`, Stufe 23e) — die Saiten des Instruments.
 * **Links (Stufe 10c; Skizze gebilligt, Patrik S111):** die Liste der
 * Schichten — Haken (an oder stumm), Name, Schwingung, Lautstaerke und
 * Versatz; darunter Add layer, Add layer from instrument..., Duplicate,
 * Remove, hinauf und hinunter.
 * **Mitte (seit Stufe 16c; Skizze gebilligt, Patrik S118, Entscheid S1 a):**
 * die gewaehlte Schicht — ihr Name, darunter die Zeile **Signal path**
 * (`SignalWeg`): fest Source, dann die Bausteine der Reihe (Filter, Drive,
 * Envelope), der Knopf + Add, fest Level. Unter der Zeile stehen die Regler
 * des gewaehlten Kastens und, bei einem Baustein, ← Move, Move →, Remove.
 * Source zeigt Transpose, Start offset, Oscillator (Waveform, Voices, Detune,
 * Pitch sweep — bei Sine die Obertoene, bei Plucked string Damping) und
 * Vibrato; Level den Regler Level. Ziehen ordnet die Bausteine um.
 * **Plucked string (Stufe 23e; Skizze gebilligt, Patrik S132):** unter
 * Waveform die Auswahl Model (Classic, Extended KS, Waveguide). Bei Classic
 * steht Damping wie bisher; sonst die Gruppen **String** (Pluck position,
 * Pluck, Ring time, High damping, Stiffness, String variation) und **Body**
 * (Body, Body amount). Der Kasten Source nennt das Verfahren.
 * **Rechts:** das Bild einer Schwingung und die Huellkurve einer Note — die
 * Summe aller Schichten als Linie, die gewaehlte gestrichelt — und eine
 * kleine Klaviatur zum Anspielen; „Selected layer only" laesst die gewaehlte
 * allein hoeren.
 * **Eine Schicht der Art Sample (Stufe 11a; Skizze gebilligt, Patrik S115):**
 * unter Waveform stehen File mit **File...**, Root note, Fine tune, Sample
 * start und der Sustain loop mit Loop from und Loop to; Voices, Detune und
 * Vibrato fehlen. Rechts zeigt das obere Bild die Aufnahme, mit einer roten
 * Marke fuer Sample start und einem blauen Bereich fuer den Sustain loop —
 * beide lassen sich im Bild ziehen.
 *
 * **Eine Schicht der Art Shape (Stufe 19e; Skizze gebilligt, Patrik S120):**
 * unter Waveform stehen Shape mit **Browse...**, Contour (eine Kontur der
 * Datei oder „All, one path"), Harmonics, Harmonics envelope, Angle, Start und
 * die Haken Stereo XY und Reverse; darunter wie sonst Voices, Detune, Pitch
 * sweep. Rechts steht unter One cycle das Bild „Shape, as it plays".
 *
 * Der Creator arbeitet an einer Kopie. Erst **Save** schreibt die Datei
 * `.lvinst` — ein mitgeliefertes Instrument laesst sich nur unter neuem Namen
 * speichern (Save as...). Zum Anspielen schreibt er die Kopie in eine
 * Vorhoer-Datei und nennt sie dem, der spielt.
 */
#pragma once

#include "loop/Grundton.hpp"
#include "loop/Instrument.hpp"
#include "loop/Sample.hpp"
#include "shape/Kontur.hpp"
#include "shape/Tonform.hpp"

#include <QPoint>
#include <QPolygonF>
#include <QString>
#include <QWidget>

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

class IAudioEngine;
class QCheckBox;
class QComboBox;
class QGridLayout;
class QLabel;
class QLineEdit;
class QListWidget;
class QHideEvent;
class QPushButton;
class QScrollArea;
class QSlider;
class QTimer;
class QToolButton;

namespace lumi::loop
{

class AufnahmeFenster;
class FormFenster;
class SaitenZeile;
class SignalWeg;

/// Eine Kurve als Bild: Werte von -1 bis 1 (oder 0 bis 1), ueber die Breite
/// verteilt; auf Wunsch ein senkrechter Strich an einer Stelle.
class KurvenBild : public QWidget
{
    Q_OBJECT

public:
    explicit KurvenBild(QWidget* parent = nullptr);
    /// `nurOben`: die Werte sind Betraege (0 bis 1), die Nulllinie liegt unten.
    /// `marke`: 0 bis 1 = ein Strich an dieser Stelle der Breite; unter null = keiner.
    void setWerte(std::vector<float> werte, bool nurOben, double marke = -1.0, const QString& markenText = {});
    [[nodiscard]] const std::vector<float>& werte() const { return m_werte; }
    /// Eine zweite Kurve, gestrichelt unter der ersten; leer = keine.
    void setZweite(std::vector<float> werte);
    [[nodiscard]] const std::vector<float>& zweite() const { return m_zweite; }
    [[nodiscard]] QSize sizeHint() const override { return {260, 96}; }

    /// Die Werte als Flaeche zeichnen, gespiegelt um die Nulllinie — so sieht
    /// eine Aufnahme aus (die Werte sind dann Betraege, 0 bis 1).
    void setGespiegelt(bool an);
    [[nodiscard]] bool gespiegelt() const { return m_gespiegelt; }

    // --- Griffe zum Ziehen (Stufe 11a: das Bild einer Aufnahme) ---------------
    /// Welcher Griff: der Anfang (rot), der Bereich von und bis (blau).
    enum Griff : int
    {
        KeinGriff = -1,
        GriffStart = 0,
        GriffVon,
        GriffBis,
    };
    /// Die Griffe als Stellen der Breite, 0 bis 1; unter null = diesen gibt es
    /// nicht. Der Bereich zwischen `von` und `bis` ist hinterlegt.
    void setGriffe(double start, double von, double bis);
    [[nodiscard]] double griff(Griff welcher) const;
    /// Die Stelle (0 bis 1) unter einem x, und das x einer Stelle.
    [[nodiscard]] double stelleBei(int x) const;
    [[nodiscard]] int xVon(double stelle) const;
    /// Der Griff unter einem x; `KeinGriff` = keiner in Reichweite.
    [[nodiscard]] Griff griffBei(int x) const;
    /// So weit neben einem Griff greift die Maus ihn noch.
    static constexpr int kGriffWeite = 6;
    /// Der Bereich als Zuschnitt (Fenster „Record sample", Stufe 11b): nicht
    /// das Innere ist hinterlegt, sondern was ausserhalb liegt ist grau, und
    /// die beiden Griffe sind rot.
    void setAussenGrau(bool an);
    [[nodiscard]] bool aussenGrau() const { return m_aussenGrau; }
    /// Eine blaue Linie an einer Stelle (0 bis 1) — wo gerade gespielt wird;
    /// unter null = keine. Sie laesst sich nicht greifen.
    void setZeiger(double stelle);
    [[nodiscard]] double zeiger() const { return m_zeiger; }

Q_SIGNALS:
    /// Ein Griff wird gezogen: er soll an diese Stelle (0 bis 1).
    void griffGezogen(int welcher, double stelle);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    [[nodiscard]] QPolygonF linie(const std::vector<float>& werte) const;

    std::vector<float> m_werte;
    std::vector<float> m_zweite;
    bool m_nurOben = false;
    double m_marke = -1.0;
    QString m_markenText;
    bool m_gespiegelt = false;
    bool m_aussenGrau = false;
    double m_zeiger = -1.0;
    double m_griffe[3] = {-1.0, -1.0, -1.0};
    Griff m_gezogen = KeinGriff;
};

/// „Shape, as it plays" (Stufe 19e; Skizze gebilligt, Patrik S120): die
/// Zeichnung gestrichelt, darueber die Form mit der eingestellten Zahl von
/// Harmonischen, und ein Pfeil fuer Angle — die Richtung, aus der der Ton von
/// der Form abgenommen wird.
class FormBild : public QWidget
{
    Q_OBJECT

public:
    explicit FormBild(QWidget* parent = nullptr);
    /// `form` = nullptr: nichts zu zeigen (keine Form genannt oder nicht gefunden).
    void setForm(const std::shared_ptr<const shape::Tonform>& form, double harmonische, double winkelGrad);
    /// Die Zeichnung und die Form, wie sie spielt — als geschlossene Zuege in
    /// den Koordinaten der Form (−1 … 1, y nach oben); leer = nichts zu zeigen.
    [[nodiscard]] const QPolygonF& zeichnung() const { return m_zeichnung; }
    [[nodiscard]] const QPolygonF& umriss() const { return m_umriss; }
    [[nodiscard]] double winkelGrad() const { return m_winkelGrad; }
    [[nodiscard]] QSize sizeHint() const override { return {260, 150}; }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QPolygonF m_zeichnung;
    QPolygonF m_umriss;
    double m_winkelGrad = 0.0;
};

/// Zwei Oktaven Tasten, C3 bis C5: ein Klick verlangt den Ton.
class Tastenleiste : public QWidget
{
    Q_OBJECT

public:
    explicit Tastenleiste(QWidget* parent = nullptr);
    static constexpr int kTief = 48;  ///< C3
    static constexpr int kHoch = 72;  ///< C5
    /// Die Tonhoehe unter dem Punkt; -1 = keine.
    [[nodiscard]] int hoeheBei(QPoint ort) const;
    /// Ein Punkt auf der Taste dieser Tonhoehe (fuer Tests).
    [[nodiscard]] QPoint ortVon(int hoehe) const;
    [[nodiscard]] QSize sizeHint() const override { return {260, 48}; }
    /// Eine Taste hervorheben — die Tonhoehe, die „Repeat note" gerade spielt
    /// (Wunsch Patrik S116); -1 = keine.
    void setGewaehlt(int hoehe);
    [[nodiscard]] int gewaehlt() const { return m_gewaehlt; }

Q_SIGNALS:
    void taste(int hoehe);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    [[nodiscard]] QRect tastenRect(int hoehe) const;
    int m_gedrueckt = -1;
    int m_gewaehlt = -1;
};

class InstrumentCreator : public QWidget
{
    Q_OBJECT

public:
    explicit InstrumentCreator(QWidget* parent = nullptr);

    /// Dieses Instrument zeigen — eine Kopie. Sein `name` sagt, woher es
    /// kommt: ein mitgeliefertes (Name) oder eine eigene Datei (Pfad).
    void oeffne(const Instrument& instrument);
    /// Die Kopie, wie die Regler sie gerade stellen.
    [[nodiscard]] const Instrument& instrument() const { return m_instrument; }
    /// Weicht die Kopie von dem ab, was geoeffnet oder zuletzt gespeichert wurde?
    [[nodiscard]] bool geaendert() const;
    /// Die eigene Datei, in die Save schreibt; leer = es gibt keine (ein
    /// mitgeliefertes Instrument: nur Save as...).
    [[nodiscard]] const QString& datei() const { return m_datei; }

    // --- was die Bedienelemente ausloesen (auch fuer Tests) -------------------
    /// Die Kopie aendern; Bilder, Regler und Titel ziehen nach.
    void aendere(const std::function<void(Instrument&)>& tu);

    // --- die Schichten (Stufe 10c) --------------------------------------------
    /// Der Platz der gewaehlten Schicht; ihre Regler stehen in der Mitte.
    [[nodiscard]] int gewaehlt() const { return m_gewaehlt; }
    [[nodiscard]] const Schicht& schicht() const { return m_instrument.schichten[static_cast<std::size_t>(m_gewaehlt)]; }
    void waehle(int platz);
    /// Die gewaehlte Schicht aendern.
    void aendereSchicht(const std::function<void(Schicht&)>& tu);
    /// Eine neue Schicht mit Grundeinstellung, sie ist danach gewaehlt.
    /// false = das Instrument traegt schon `kSchichtenMax`.
    bool neueSchicht();
    /// Die Schichten eines anderen Instruments dazunehmen — kopiert, nicht
    /// verbunden; eine ohne Namen heisst wie das Instrument. So viele kamen
    /// dazu (was ueber `kSchichtenMax` geht, bleibt weg); die erste davon ist
    /// danach gewaehlt.
    int schichtenAus(const Instrument& quelle);
    /// Die gewaehlte Schicht noch einmal, gleich dahinter. false = kein Platz.
    bool verdopple();
    /// Die gewaehlte Schicht entfernen. false = es ist die einzige.
    bool entferne();
    /// Die gewaehlte Schicht einen Platz hinauf (-1) oder hinunter (+1).
    bool schiebe(int richtung);
    /// Anspielen nur mit der gewaehlten Schicht — auch wenn sie stumm ist.
    void setNurGewaehlte(bool an);
    [[nodiscard]] bool nurGewaehlte() const { return m_nurGewaehlte; }
    /// Wie eine Schicht in der Liste heisst: „2 Shimmer" oder „Layer 2".
    [[nodiscard]] static QString schichtTitel(const Schicht& schicht, int platz);
    /// Zurueck auf das, was geoeffnet oder zuletzt gespeichert wurde.
    void setzeZurueck();
    /// In die eigene Datei schreiben — ein eigenes Instrument an seinen Platz
    /// im Benutzerordner, eine Datei an freiem Ort dorthin. false = es gibt
    /// keine (ein mitgeliefertes: `speichereAlsEigenes`), oder das Schreiben
    /// gelang nicht.
    bool speichere();
    /// Als neue Datei an freiem Ort schreiben; sie ist danach die Datei des
    /// Creators. Seit S117 fuehrt kein Knopf mehr hierher (Save as... fragt
    /// nach dem Namen); `speichere` schreibt so eine geladene Datei an ihren Ort.
    bool speichereUnter(const QString& pfad);
    /// Der Satz im Dialog „Save instrument": wohin die Kopie kommt und was
    /// bleibt, wie es ist — je nachdem, ob das Instrument neu, mitgeliefert
    /// oder schon eine eigene Datei ist.
    [[nodiscard]] QString kopieSatz() const;
    /// Unter diesem Titel als eigenes Instrument in den Benutzerordner
    /// schreiben (Stufe 14b, Entscheid P2 a): die Datei heisst
    /// `eigenerName(titel)`, das Instrument danach `user:<name>`; ein
    /// vorhandenes mit dem Namen wird ersetzt. false = aus dem Titel wird
    /// kein Name, oder das Schreiben gelang nicht.
    bool speichereAlsEigenes(const QString& titel);
    /// Der Titel, den Save fuer eine eigene Kopie vorschlaegt: der eigene —
    /// oder mit einer Zahl dahinter, wenn er noch der der Vorlage ist
    /// („House Piano 2", die erste, die es unter den eigenen nicht gibt).
    [[nodiscard]] QString vorschlagTitel() const;
    /// Wie das Instrument nach dem Speichern heisst: `user:<name>` oder der
    /// Pfad seiner Datei; leer = noch nie gespeichert.
    [[nodiscard]] QString nennung() const;
    /// Die Genres aus einem Text mit Kommas — ohne Leeres, ohne Doppelte.
    [[nodiscard]] static QStringList genresAus(const QString& text);
    /// Einen Ton mit den Einstellungen anspielen, wie sie gerade stehen: die
    /// Kopie kommt in die Vorhoer-Datei, `tonVerlangt` nennt sie. false = die
    /// Datei liess sich nicht schreiben.
    bool spieleAn(int hoehe);
    /// Die Vorhoer-Datei (im Ordner fuer Zwischendateien).
    [[nodiscard]] static QString vorhoerDatei();

    // --- die Aufnahme einer Sample-Schicht (Stufe 11a) -------------------------
    /// Die Aufnahme der gewaehlten Schicht setzen: ein eigenes Sample
    /// (`user:<name>`) oder der Pfad einer Tondatei. Eine andere Aufnahme
    /// beginnt von vorn: Sample start 0, kein Sustain loop.
    void setzeSample(const QString& nennung);
    /// Eine gewaehlte Tondatei nehmen (Entscheid S2 a): zu den eigenen Samples
    /// kopiert (`user:<name>`) oder am Ort gelassen (ihr Pfad). false = die
    /// Kopie gelang nicht.
    bool nimmSampleDatei(const QString& pfad, bool kopieren);
    /// So lang ist die Aufnahme der gewaehlten Schicht, in Millisekunden;
    /// 0 = keine genannt oder nicht zu laden.
    [[nodiscard]] double sampleLaengeMs();
    /// Der Sustain loop der gewaehlten Schicht an oder aus; beim ersten
    /// Einschalten liegt er auf dem mittleren Teil der Aufnahme.
    void setzeSchleife(bool an);

    // --- die Form einer Shape-Schicht (Stufe 19e) --------------------------------
    /// Die Form der gewaehlten Schicht setzen: eine mitgelieferte beim Namen,
    /// eine eigene (`user:<name>`) oder der Pfad einer `.lvcontour`. Eine
    /// andere Form hat andere Konturen: Contour steht danach auf „All, one path".
    void setzeForm(const QString& nennung);
    /// Das Bild „Shape, as it plays" (fuer Tests).
    [[nodiscard]] FormBild* formBild() const { return m_formBild; }
    /// **Edit...** neben Shape (Stufe 20a): die Form der gewaehlten Schicht im
    /// Fenster Edit shape oeffnen. Nach Save oder Save as... spielt die Schicht
    /// das Ergebnis. false = keine Shape-Schicht, oder ihre Form ist nicht zu lesen.
    bool oeffneFormBearbeiten(bool zeigen = true);
    /// **New shape...** im Menue hinter Browse...: das Fenster leer oeffnen;
    /// was dort mit Save as... gespeichert wird, spielt danach die gewaehlte
    /// Schicht. false = keine Shape-Schicht.
    bool oeffneNeueForm(bool zeigen = true);
    /// Das Fenster Edit shape; nullptr, solange es nie geoeffnet wurde.
    [[nodiscard]] FormFenster* formFenster() const { return m_formFenster; }
    /// Was der Creator nach dem letzten Speichern aus Edit shape sagt („2
    /// contours did not fit ..."); leer = nichts.
    [[nodiscard]] const QString& formMeldung() const { return m_formMeldung; }

    // --- die Farbe einer Schicht (Stufe 20d, F1 a) --------------------------------
    /// Die eigene Farbe der Schicht auf `platz` setzen (`#rrggbb`); leer =
    /// Automatic, die abgeleitete.
    void setzeSchichtFarbe(int platz, const QString& farbe);

    // --- Detect und Reference tone (Stufe 11a, Schritt 3) ----------------------
    /// **Detect:** den Grundton der Aufnahme der gewaehlten Schicht messen, ab
    /// Sample start. Das Ergebnis steht danach unter Root note („Detected: A3
    /// +12 ct" oder „No clear pitch found") — uebernommen wird erst mit Apply.
    /// false = kein klarer Ton, oder keine Aufnahme geladen.
    bool messe();
    /// Was zuletzt fuer die gewaehlte Schicht und ihre Aufnahme gemessen wurde;
    /// `gefunden` ist false, wenn nichts gemessen oder nichts gefunden ist.
    [[nodiscard]] Grundton gemessen() const;
    /// **Apply:** das Gemessene uebernehmen — Root note und Fine tune. false =
    /// es gibt nichts zu uebernehmen.
    bool uebernimmGemessenes();
    /// **Reference tone:** die Klaviatur spielt die gewaehlte Sample-Schicht
    /// allein und dazu einen Sinus derselben Note — zum Stimmen nach Gehoer.
    void setReferenzton(bool an);
    [[nodiscard]] bool referenzton() const { return m_referenzton; }

    // --- Repeat note (Wunsch Patrik S116) ----------------------------------------
    /// **Repeat note:** an = der zuletzt angeklickte Ton (anfangs C4) spielt
    /// immer wieder, mit den Einstellungen, wie sie gerade stehen — so hoert
    /// man jede Aenderung beim naechsten Schlag. Ein Klick auf eine Taste
    /// wechselt die Tonhoehe; sie ist auf der Klaviatur hervorgehoben, solange
    /// es laeuft. Endet, wenn das Fenster zugeht.
    void setzeWiederholen(bool an);
    [[nodiscard]] bool wiederholt() const;
    /// Die Tonhoehe, die wiederholt wird (auch die zuletzt angeklickte ohne Repeat).
    [[nodiscard]] int wiederholHoehe() const { return m_wiederholHoehe; }
    /// So lang ist ein Schlag im Tempo des Loops, in Millisekunden — der
    /// Editor nennt es.
    void setzeSchlagMs(double ms);
    /// **every n beats** (Wunsch Patrik S116: „bei delay usw will man etwas
    /// mehr hoeren"): der Abstand zwischen zwei Toenen in Schlaegen — 1, 2, 4,
    /// 8 oder 16, vorbelegt 2. So lang darf der Ton auch klingen.
    void setzeWiederholSchlaege(int schlaege);
    [[nodiscard]] int wiederholSchlaege() const { return m_wiederholSchlaege; }
    /// Der Abstand in Millisekunden: die Schlaege im Tempo des Loops, mindestens 250 ms.
    [[nodiscard]] int wiederholMs() const;

    // --- Signal path (Stufe 16c) -------------------------------------------------
    /// Was in der Zeile gewaehlt ist — seine Regler stehen darunter: Source,
    /// Level oder der Platz eines Bausteins in `schicht().reihe` (ab 0).
    static constexpr int kStelleSource = -1;
    static constexpr int kStelleLevel = -2;
    [[nodiscard]] int stelle() const { return m_stelle; }
    /// Einen Kasten waehlen; was es nicht gibt, waehlt Source.
    void waehleStelle(int stelle);
    /// **+ Add:** ein neuer Filter (Low-pass, Cutoff × 4) oder Drive (30 %)
    /// hinter dem gewaehlten Baustein — bei Source vorn, bei Level hinten; er
    /// ist danach gewaehlt. false = die Reihe traegt schon `kBausteineMax`.
    bool fuegeFilterEin();
    bool fuegeDriveEin();
    /// **← Move / Move →:** den gewaehlten Baustein einen Platz nach vorn (-1)
    /// oder hinten (+1). false = kein Baustein gewaehlt, oder er steht am Rand.
    bool schiebeBaustein(int richtung);
    /// Ziehen: den Baustein vom Platz `von` an den Platz `nach` der Reihe; er
    /// ist danach gewaehlt. false = einen der Plaetze gibt es nicht.
    bool verschiebeBaustein(int von, int nach);
    /// **Remove:** den gewaehlten Baustein entfernen. false = keiner gewaehlt —
    /// oder es ist die Envelope: sie bleibt, jede Schicht hat genau eine.
    bool entferneBaustein();
    /// Die Zeile selbst (fuer Tests).
    [[nodiscard]] SignalWeg* signalWeg() const { return m_pfad; }

    // --- Tuning (Stufe 23e) --------------------------------------------------------
    /// Die Zeile Tuning unter Type / Genres / Description: die Saiten des
    /// Instruments (`Instrument::saiten`).
    [[nodiscard]] SaitenZeile* saitenZeile() const { return m_saitenZeile; }

    // --- Gruppen ein- und ausklappen (Wunsch Patrik S116) -----------------------
    /// Die Gruppen Oscillator und Vibrato (unter Source) lassen sich an
    /// ihrer Ueberschrift ein- und ausklappen; das Programm merkt die Stellung
    /// (`loop/creatorFolded/<schluessel>`), sie gilt fuer jede Schicht und jedes
    /// Instrument. `schluessel`: `oscillator`, `string`, `body` (Stufe 23e), `vibrato` — Envelope und Filter
    /// sind seit Stufe 16c Bausteine der Zeile Signal path, keine Gruppen mehr.
    void setzeGruppeZu(const QString& schluessel, bool zu);
    [[nodiscard]] bool gruppeZu(const QString& schluessel) const;
    /// Nur fuer Tests: aus = die Stellung wird weder gelesen noch gemerkt.
    static void setzeGruppenMerken(bool an);

    // --- Record... (Stufe 11b, Schritt 3) --------------------------------------
    /// Wer die Audio-Engine nennt, ueber die das Fenster „Record sample"
    /// aufnimmt und spielt — gefragt wird, wenn es sich oeffnet. Ohne ihn (oder
    /// nennt er keine) sagt das Fenster, dass es nicht aufnehmen kann.
    void setEngineGeber(std::function<IAudioEngine*()> geber);
    /// **Record...:** das Fenster „Record sample" oeffnen (von vorn: keine
    /// Aufnahme). Was dort mit Save as... gespeichert wird, ist danach die
    /// Aufnahme der gewaehlten Schicht. false = die gewaehlte Schicht ist
    /// keine Sample-Schicht.
    bool oeffneAufnahme(bool zeigen = true);
    /// **Edit...** (Stufe 17b; Entscheide M1 a, M3 a, M4 a, Patrik S119): was
    /// die gewaehlte Schicht spielt, im selben Fenster oeffnen — ein eigenes
    /// Sample, einen Klang des Kits, eine Datei an freiem Ort. Nach Save oder
    /// Save as... spielt die Schicht das Ergebnis; ihr Sample start und ihre
    /// Schleife ruecken um das vorn Weggeschnittene mit. false = keine
    /// Sample-Schicht, kein Sample, seine Daten sind nicht zu laden, oder es
    /// ist zu lang (`bearbeitenHindernis`).
    bool oeffneBearbeiten(bool zeigen = true);
    /// Warum Edit... das Sample der gewaehlten Schicht nicht oeffnet, obwohl
    /// es da ist: es ist laenger als `AufnahmeFenster::kHoechstensSekunden`.
    /// Leer = kein Hindernis (oder nichts zu oeffnen).
    [[nodiscard]] QString bearbeitenHindernis();
    /// Das Fenster „Record sample"; nullptr, solange Record... noch nie gedrueckt wurde.
    [[nodiscard]] AufnahmeFenster* aufnahmeFenster() const { return m_aufnahme; }

Q_SIGNALS:
    /// Die Daten dieser Aufnahme werden gebraucht (fuer das Bild und die
    /// Laenge): wer eine Audio-Engine hat, dekodiert sie und merkt sie
    /// (`merkeSampleDaten`, Sample.hpp) — noch waehrend des Signals.
    void sampleVerlangt(const QString& nennung);
    /// Das Instrument ist gespeichert und heisst jetzt so: `user:<name>` oder
    /// der Pfad seiner Datei (Save, Save as...).
    void gespeichert(const QString& nennung);
    /// Einen Ton anspielen: das Instrument beim Pfad seiner Vorhoer-Datei.
    void tonVerlangt(const QString& nennung, int hoehe);
    /// Dasselbe von „Repeat note": der Ton darf `klingtMs` lang klingen — bis
    /// der naechste kommt —, damit ein langer Ausklang ganz zu hoeren ist.
    void tonLangVerlangt(const QString& nennung, int hoehe, int klingtMs);

protected:
    /// Das Mausrad ueber einem Regler der Schicht rollt das Feld, statt ihn zu verstellen.
    bool eventFilter(QObject* objekt, QEvent* ereignis) override;
    /// Das Fenster ist zu: „Repeat note" endet.
    void hideEvent(QHideEvent* event) override;
    /// Ungespeichertes: Save, Discard oder Cancel.
    void closeEvent(QCloseEvent* event) override;

private:
    /// Ein Regler mit seinem Wert daneben.
    struct Regler
    {
        QLabel* name = nullptr;
        QSlider* schieber = nullptr;
        QLabel* wert = nullptr;
        std::function<int(const Schicht&)> lese;
        std::function<QString(const Schicht&)> text;
    };
    void setupUI();
    /// Ein Regler der gewaehlten Schicht.
    Regler& regler(QGridLayout* gitter, const QString& name, const char* kennung, int von, int bis,
                   std::function<int(const Schicht&)> lese, std::function<void(Schicht&, int)> schreibe,
                   std::function<QString(const Schicht&)> text);
    /// Save bei einem mitgelieferten oder neuen Instrument und Save as... bei
    /// jedem (Entscheid E1 a, S117): nach dem Namen der eigenen Kopie fragen
    /// und sie in den Benutzerordner schreiben. false = abgebrochen.
    bool frageNachNamen();
    /// Das Menue hinter „Add layer from instrument...".
    void zeigeQuellen();
    void zeigeListe();
    /// Die Kopie fuer die Bilder: je Schicht eine Stimme ohne Vibrato;
    /// `nurGewaehlte` = nur die gewaehlte Schicht, eingeschaltet.
    [[nodiscard]] Instrument fuerBild(bool nurGewaehlte) const;
    /// Regler, Sichtbarkeit, Bilder, Titel aus `m_instrument`.
    void zeigeAlles();
    void zeigeBilder();
    /// Das Menue hinter „File...": die eigenen Samples und „From file...".
    void zeigeSampleWahl();
    /// „From file...": die Datei waehlen und nach S2 a fragen.
    void frageNachSampleDatei();
    /// Die gemerkten Daten einer Aufnahme; fehlen sie, einmal verlangen.
    [[nodiscard]] std::shared_ptr<const SampleDaten> holeSample(const QString& nennung);

    Instrument m_instrument;
    Instrument m_vorlage;   ///< wie geoeffnet oder zuletzt gespeichert
    QString m_datei;
    QString m_eigenerName;  ///< der Name im Benutzerordner; leer = kein eigenes (`user:<name>`)
    QString m_herkunft;     ///< „based on …"
    int m_gewaehlt = 0;
    bool m_nurGewaehlte = false;

    QListWidget* m_liste = nullptr;
    QPushButton* m_neu = nullptr;
    QPushButton* m_neuAus = nullptr;
    QPushButton* m_doppelt = nullptr;
    QPushButton* m_weg = nullptr;
    QPushButton* m_hinauf = nullptr;
    QPushButton* m_hinunter = nullptr;
    QLabel* m_schichtKopf = nullptr;
    QLineEdit* m_schichtName = nullptr;
    QSlider* m_gesamt = nullptr;      ///< Level des ganzen Instruments
    QLabel* m_gesamtWert = nullptr;
    QLabel* m_legende = nullptr;
    QCheckBox* m_nurWahl = nullptr;
    QLineEdit* m_titel = nullptr;
    QLabel* m_herkunftZeile = nullptr;
    QPushButton* m_zurueck = nullptr;
    QPushButton* m_speichern = nullptr;
    QPushButton* m_speichernUnter = nullptr;
    QPushButton* m_publish = nullptr;
    QLabel* m_spitze = nullptr;
    // die zweite Zeile im Kopf (Stufe 14b): fuer das Fenster „Instruments"
    QComboBox* m_art = nullptr;
    QLineEdit* m_genres = nullptr;
    QLineEdit* m_beschreibung = nullptr;
    QComboBox* m_welle = nullptr;
    std::vector<Regler> m_regler;
    std::vector<std::size_t> m_nurSinus;     ///< Plaetze in `m_regler`: die Obertoene
    /// Was die gezupfte Saite nicht kennt: Voices, Detune, Decay, Sustain, Filter, Vibrato.
    std::vector<std::size_t> m_nichtSaite;
    std::size_t m_daempfung = 0;             ///< Platz des Reglers Damping (nur Saite)
    // --- nur Sample (Stufe 11a)
    std::vector<std::size_t> m_nurSample;    ///< Plaetze: Fine tune, Sample start, Loop from, Loop to
    std::vector<std::size_t> m_nichtSample;  ///< was eine Aufnahme nicht kennt: Voices, Detune, Vibrato
    std::vector<std::size_t> m_nurTonweg;    ///< Pitch sweep, Sweep time: nicht bei Saite und Rauschen (Stufe 11d)
    std::size_t m_reglerWegZeit = 0;         ///< Platz von Sweep time: grau, solange Pitch sweep auf 0 steht
    QLabel* m_filterArtName = nullptr;       ///< „Type" in der Gruppe Filter (Stufe 11e)
    QComboBox* m_filterArt = nullptr;        ///< Low-pass, High-pass, Band-pass
    std::size_t m_reglerResonanz = 0;        ///< Platz von Resonance: grau, solange Cutoff auf „open" steht
    std::size_t m_reglerStart = 0;
    std::size_t m_reglerVon = 0;
    std::size_t m_reglerBis = 0;
    std::vector<QWidget*> m_sampleZeilen;    ///< was sonst nur bei Sample zu sehen ist
    QLabel* m_sampleName = nullptr;
    QPushButton* m_sampleWahl = nullptr;
    QComboBox* m_grundton = nullptr;
    QCheckBox* m_schleife = nullptr;
    // --- nur Shape (Stufe 19e)
    std::vector<std::size_t> m_nurForm;      ///< Plaetze: Harmonics, Harmonics envelope, Angle, Start
    std::vector<QWidget*> m_formZeilen;      ///< was sonst nur bei Shape zu sehen ist
    QLabel* m_formName = nullptr;
    QPushButton* m_formWahl = nullptr;
    QPushButton* m_formBearbeiten = nullptr;  ///< Edit... (Stufe 20a)
    FormFenster* m_formFenster = nullptr;
    void richteFormFensterEin();
    /// Was das Fenster gespeichert hat, in die gewaehlte Schicht setzen — mit
    /// Use as: Layers je weiterer angehakter Kontur eine Kopie (L1 a).
    void uebernimmForm(const QString& nennung);
    QString m_formMeldung;
    /// Das Menue der Farbe einer Schicht: Colour... und Automatic.
    void zeigeFarbMenue(int platz, QPoint wo);
    /// Wo in der Liste der Farbpunkt einer Schicht steht (in Punkten des Viewports).
    [[nodiscard]] QRect farbPunktRect(int platz) const;
    QComboBox* m_formKontur = nullptr;
    QCheckBox* m_formStereo = nullptr;
    QCheckBox* m_formRueckwaerts = nullptr;
    /// Die Zeichnung der gewaehlten Schicht, einmal je Nennung gelesen (fuer
    /// die Zeile Contour); `m_formGefunden` = sie liess sich lesen.
    QString m_formGelesen;
    bool m_formGelesenDa = false;
    std::uint64_t m_formFassung = 0;  ///< `shape::zeichnungFassung` beim Lesen
    bool m_formGefunden = false;
    shape::Zeichnung m_formZeichnung;
    void liesForm(const QString& nennung);
    /// Das Menue hinter „Browse...": die mitgelieferten und die eigenen Formen, „From file...".
    void zeigeFormWahl();
    FormBild* m_formBild = nullptr;
    QLabel* m_formBildTitel = nullptr;
    QLabel* m_formLegende = nullptr;
    QWidget* m_vibratoTitel = nullptr;       ///< die Ueberschrift Vibrato: weg, wenn kein Regler darunter steht
    /// Eine Gruppe von Zeilen unter einer Ueberschrift, die sich einklappen laesst.
    struct Gruppe
    {
        QString schluessel;         ///< `oscillator`, `vibrato`
        QToolButton* kopf = nullptr;
        int zeile = 0;              ///< die Zeile der Ueberschrift im Gitter
        bool zu = false;
        int ende = -1;              ///< die erste Zeile, die nicht mehr dazugehoert; -1 = bis zur naechsten Ueberschrift
    };
    std::vector<Gruppe> m_gruppen;
    QGridLayout* m_gitter = nullptr;
    QScrollArea* m_rollen = nullptr;  ///< die Regler der Schicht: rollt, wenn sie nicht ins Fenster passen
    QCheckBox* m_wiederholen = nullptr;  ///< „Repeat note"
    QComboBox* m_wiederholAbstand = nullptr;  ///< „every n beats"
    int m_wiederholSchlaege = 2;
    QTimer* m_wiederholTakt = nullptr;
    int m_wiederholHoehe = 60;           ///< C4, bis eine Taste angeklickt wird
    double m_schlagMs = 500.0;           ///< ein Schlag bei 120 BPM
    /// Was eingeklappt ist, ausblenden; die Pfeile der Ueberschriften stellen.
    void klappeGruppen();
    // --- Signal path (Stufe 16c)
    int m_stelle = kStelleSource;
    int m_stelleFuer = 0;  ///< fuer diese Schicht gilt `m_stelle`; eine andere beginnt bei Source
    SignalWeg* m_pfad = nullptr;
    QLabel* m_wegTitel = nullptr;      ///< „Signal path — Layer 1 "Body"“
    QLabel* m_stellenTitel = nullptr;  ///< der Name des gewaehlten Kastens ueber seinen Reglern
    QPushButton* m_bausteinVor = nullptr;
    QPushButton* m_bausteinZurueck = nullptr;
    QPushButton* m_bausteinWeg = nullptr;
    /// Zeilen des Gitters, die zu einem Kasten gehoeren: von (einschliesslich) bis (ausschliesslich).
    struct Abschnitt
    {
        int von = 0;
        int bis = 0;
    };
    Abschnitt m_zeilenLevel;
    Abschnitt m_zeilenSource;
    Abschnitt m_zeilenHuelle;
    Abschnitt m_zeilenFilter;
    Abschnitt m_zeilenDrive;
    /// Der gewaehlte Baustein dieser Schicht, wenn er ein Filter (ein Drive) ist; sonst nullptr.
    [[nodiscard]] const SchichtFilter* gewaehlterFilter(const Schicht& schicht) const;
    [[nodiscard]] SchichtFilter* gewaehlterFilter(Schicht& schicht) const;
    [[nodiscard]] const SchichtDrive* gewaehlterDrive(const Schicht& schicht) const;
    [[nodiscard]] SchichtDrive* gewaehlterDrive(Schicht& schicht) const;
    /// Die Zeilen des Gitters, deren Regler zum gewaehlten Kasten gehoeren.
    [[nodiscard]] const Abschnitt& gewaehlterAbschnitt() const;
    /// Einen Baustein hinter dem gewaehlten einsetzen (`fuegeFilterEin`, `fuegeDriveEin`).
    bool fuegeEin(const SchichtBaustein& baustein);
    /// Die Kaesten der Zeile, ihr Titel, die Knoepfe Move und Remove.
    void zeigeWeg();
    /// Im Gitter nur zeigen, was zum gewaehlten Kasten gehoert.
    void zeigeStelle();
    /// Das Menue hinter + Add.
    void zeigeAddMenue(QPoint unten);
    QLabel* m_wellenTitel = nullptr;
    QLabel* m_bildHinweis = nullptr;
    QString m_sampleVersucht;                ///< diese Aufnahme liess sich nicht laden: nicht bei jedem Zug neu fragen
    // --- Detect und Reference tone (Schritt 3)
    /// Fuer welche Schicht und Aufnahme `m_gemessen` gilt (`messSchluessel`); leer = nichts gemessen.
    QString m_gemessenFuer;
    Grundton m_gemessen;
    [[nodiscard]] QString messSchluessel() const;
    QPushButton* m_detect = nullptr;
    QLabel* m_detectText = nullptr;
    QPushButton* m_detectApply = nullptr;
    QCheckBox* m_referenz = nullptr;
    bool m_referenzton = false;
    // --- Record... (Stufe 11b)
    QPushButton* m_sampleAufnahme = nullptr;
    AufnahmeFenster* m_aufnahme = nullptr;
    // --- Edit... (Stufe 17b)
    QPushButton* m_sampleBearbeiten = nullptr;
    QString m_bearbeitet;  ///< das Sample, fuer das Edit... das Fenster geoeffnet hat; leer = Record...
    void richteAufnahmeEin();
    void zeigeAufnahme();
    /// Was das Fenster gespeichert hat, in die gewaehlte Schicht setzen.
    void uebernimmSample(const QString& nennung, double vornWegMs);
    std::function<IAudioEngine*()> m_engineGeber;
    KurvenBild* m_wellenBild = nullptr;
    KurvenBild* m_huellBild = nullptr;
    Tastenleiste* m_tasten = nullptr;
    SaitenZeile* m_saitenZeile = nullptr;  ///< die Zeile Tuning (Stufe 23e)
    // --- die Saite nach der Physik (Stufe 23e): Model, die Gruppen String und Body
    QComboBox* m_saitenModell = nullptr;     ///< Model: Classic, Extended KS, Waveguide
    std::vector<QWidget*> m_modellZeile;     ///< Name und Auswahl Model: nur bei Plucked string
    QWidget* m_saitenTitel = nullptr;        ///< die Ueberschrift String
    QWidget* m_korpusTitel = nullptr;        ///< die Ueberschrift Body
    QComboBox* m_korpus = nullptr;           ///< Body: None, Acoustic guitar (Parlor), Classical guitar
    std::vector<QWidget*> m_korpusZeile;     ///< Name und Auswahl Body
    std::vector<std::size_t> m_nurPhysik;    ///< Plaetze in `m_regler`: die Regler von String und Body
    std::size_t m_reglerKorpusAnteil = 0;    ///< Body amount: grau ohne Korpus
};

}  // namespace lumi::loop
