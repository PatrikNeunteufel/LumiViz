/**
 * @file InstrumentCreator.hpp
 * @brief Loop: der Instrument-Creator — die Bausteine eines gerechneten
 *        Instruments als Regler, als eigenes Fenster
 *        (docs/audio/Loop_Konzept.md §4f, Stufe 10b; Skizze gebilligt,
 *        Patrik S110; Entscheide I2 b und I3 a)
 *
 * **Kopf:** Name, die Vorlage, Reset, Save, Save as..., Level des ganzen
 * Instruments, Peak.
 * **Links (Stufe 10c; Skizze gebilligt, Patrik S111):** die Liste der
 * Schichten — Haken (an oder stumm), Name, Schwingung, Lautstaerke und
 * Versatz; darunter Add layer, Add layer from instrument..., Duplicate,
 * Remove, hinauf und hinunter.
 * **Mitte:** die gewaehlte Schicht — Name, Level, Transpose, Start offset;
 * Oscillator (Waveform, Voices, Detune — bei Sine die Obertoene, bei Plucked
 * string Damping), Envelope, Filter, Drive, Vibrato.
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
 * Der Creator arbeitet an einer Kopie. Erst **Save** schreibt die Datei
 * `.lvinst` — ein mitgeliefertes Instrument laesst sich nur unter neuem Namen
 * speichern (Save as...). Zum Anspielen schreibt er die Kopie in eine
 * Vorhoer-Datei und nennt sie dem, der spielt.
 */
#pragma once

#include "loop/Grundton.hpp"
#include "loop/Instrument.hpp"
#include "loop/Sample.hpp"

#include <QPoint>
#include <QPolygonF>
#include <QString>
#include <QWidget>

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
class QPushButton;
class QScrollArea;
class QSlider;
class QToolButton;

namespace lumi::loop
{

class AufnahmeFenster;

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

Q_SIGNALS:
    void taste(int hoehe);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    [[nodiscard]] QRect tastenRect(int hoehe) const;
    int m_gedrueckt = -1;
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
    /// Als neue Datei an freiem Ort schreiben; sie ist danach die Datei des Creators.
    bool speichereUnter(const QString& pfad);
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

    // --- Gruppen ein- und ausklappen (Wunsch Patrik S116) -----------------------
    /// Die Gruppen Oscillator, Envelope, Filter und Vibrato lassen sich an
    /// ihrer Ueberschrift ein- und ausklappen; das Programm merkt die Stellung
    /// (`loop/creatorFolded/<schluessel>`), sie gilt fuer jede Schicht und jedes
    /// Instrument. `schluessel`: `oscillator`, `envelope`, `filter`, `vibrato`.
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
    /// Aufnahme, ein neuer Name). Was dort mit Save and use gespeichert wird,
    /// ist danach die Aufnahme der gewaehlten Schicht. false = die gewaehlte
    /// Schicht ist keine Sample-Schicht.
    bool oeffneAufnahme(bool zeigen = true);
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

protected:
    /// Das Mausrad ueber einem Regler der Schicht rollt das Feld, statt ihn zu verstellen.
    bool eventFilter(QObject* objekt, QEvent* ereignis) override;
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
    void frageNachDatei();
    /// Save bei einem mitgelieferten oder neuen Instrument: nach dem Namen
    /// der eigenen Kopie fragen und sie schreiben. false = abgebrochen.
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
    QWidget* m_vibratoTitel = nullptr;       ///< die Ueberschrift Vibrato: weg, wenn kein Regler darunter steht
    /// Eine Gruppe von Zeilen unter einer Ueberschrift, die sich einklappen laesst.
    struct Gruppe
    {
        QString schluessel;         ///< `oscillator`, `envelope`, `filter`, `vibrato`
        QToolButton* kopf = nullptr;
        int zeile = 0;              ///< die Zeile der Ueberschrift im Gitter
        bool zu = false;
    };
    std::vector<Gruppe> m_gruppen;
    QGridLayout* m_gitter = nullptr;
    QScrollArea* m_rollen = nullptr;  ///< die Regler der Schicht: rollt, wenn sie nicht ins Fenster passen
    /// Was eingeklappt ist, ausblenden; die Pfeile der Ueberschriften stellen.
    void klappeGruppen();
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
    std::function<IAudioEngine*()> m_engineGeber;
    KurvenBild* m_wellenBild = nullptr;
    KurvenBild* m_huellBild = nullptr;
    Tastenleiste* m_tasten = nullptr;
};

}  // namespace lumi::loop
