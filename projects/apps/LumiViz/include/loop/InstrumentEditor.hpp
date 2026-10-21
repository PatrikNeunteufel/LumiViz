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
 * **Spielweisen (Stufe 24e; Skizze gebilligt, Patrik S134, S-W5 a):** unter der
 * Instrument-Zeile die Zeile *Strum* des Musters (H).
 * **Spuren ueber der Zeit (Stufe 28d; Skizze §1a gebilligt, Patrik S140):** darunter die
 * Zeile *Lanes* — je Spur, die das Instrument traegt, ein Haken, dazu *Tab*; im Raster
 * die eingeblendeten Spuren unter der Spur Strum.
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

#include <array>
#include <cmath>
#include <functional>
#include <optional>
#include <utility>
#include <vector>

class IAudioEngine;
class QCheckBox;
class QContextMenuEvent;
class QHelpEvent;
class QPainter;
class QComboBox;
class QLabel;
class QPushButton;
class QScrollBar;
class QSlider;
class QSpinBox;
class QToolButton;

namespace lumi::loop
{

class AkkordFenster;
struct Akkord;
class InstrumentCreator;
class InstrumentPalette;
class StrumZeile;

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

    // --- Spielweisen (Stufe 24e; Skizze gebilligt, Patrik S134, S-W5 a) --------
    // I: die Noten eines Akkords stehen um ihren Versatz verschoben, ein feiner
    // Strich verbindet ihre Anschlaege (der Punkt an der zuerst angeschlagenen);
    // K: die Artikulationen als Kuerzel hinter der Nummer der Saite.
    /// Das Tempo des Loops: es macht aus dem Versatz in ms eine Strecke.
    void setTempo(double bpm);
    /// Je Note des Musters (in seiner Reihenfolge) der Anschlag, wie die
    /// Planung ihn setzt (`schlageAn`); ohne Saiten lauter Down ohne Versatz.
    [[nodiscard]] std::vector<Anschlagplan> anschlaege() const;
    /// So weit steht eine Note mit diesem Versatz rechts von ihrem Schritt, in Pixeln.
    [[nodiscard]] int versatzPixel(double versatzMs) const;
    /// Die Kuerzel einer Note: PM (Palm mute), LR (Let ring), T (Tapping), ◇ (Harmonic).
    [[nodiscard]] static QString artikulationsKuerzel(int artikulationen);
    /// J: die Spur *Strum* zwischen der Zeile der Takte und den Noten (S-W14 a;
    /// nur mit Saiten) — an jedem Schritt, an dem eine Note beginnt, ein Pfeil
    /// (S-W15 a), grau aus der Hand, blau mit Nadel aus *Anschlaege*, „≡" fuer
    /// Together, „T", wo nur getippt wird. Ein Klick schaltet bei einem Akkord
    /// Follow hand → Down → Up → Together, bei einer einzelnen Note Follow hand
    /// → Down → Up. false = keine Saiten oder keine Note an diesem Schritt.
    bool schalteAnschlag(int schritt);
    /// L: im Menue der Noten das Untermenue *Strum* (fuer die Schritte, an denen
    /// markierte Noten anschlagen) und die Haken der Artikulationen (fuer die
    /// markierten Noten) — je Wahl ein Schritt fuer Undo. false = nichts
    /// markiert, keine Saiten, oder es aendert nichts.
    bool setzeAnschlagart(Anschlagart art);
    /// *Spread...*: der Abstand zweier Saiten fuer diese Akkorde; −1 = wie das Muster.
    bool setzeAnschlagSpread(double ms);
    /// Tragen alle markierten Noten die Artikulation, geht sie weg — sonst bekommen sie alle.
    bool schalteArtikulation(int bit);
    /// Der Name einer Artikulation im Menue: Palm mute, Let ring, Tapping, Harmonic.
    [[nodiscard]] static QString artikulationsName(int bit);
    /// Der Schritt unter dem Punkt in der Spur Strum, an dem eine Note beginnt; -1 = keiner.
    [[nodiscard]] int strumBei(QPoint ort) const;
    /// Oberkante der Spur Strum: direkt unter der Zeile der Takte (S-W14 a).
    [[nodiscard]] int strumOben() const { return kKopfHoehe; }
    [[nodiscard]] int strumHoehe() const { return m_saiten.istSaiteninstrument() ? kStrumHoehe : 0; }

    // --- Tab-Linien (Stufe 26c; Skizze gebilligt, Patrik S137, S-T5 a A, B, F) --
    // Zwischen der Spur Strum und den Noten je Saite eine Linie, oben die hoechste;
    // links Nummer und Ton der leeren Saite. An jeder Note die Bundzahl (ohne Buende
    // der Tonname), ein blasser Strich zeigt die Dauer. Ein Klick auf eine Linie setzt
    // die Tab-Marke; eine getippte Zahl setzt dort einen Bund (zwei Ziffern kurz
    // hintereinander: zweistellig). Eine Zahl auf eine andere Linie ziehen: derselbe
    // Ton auf der anderen Saite. Nur mit Saiten, und solange der Haken *Tab* steht.
    /// Der Haken *Tab* (F): die Linien zeigen oder nicht.
    void setTabAn(bool an);
    [[nodiscard]] bool tabAn() const { return m_tabAn; }
    [[nodiscard]] int tabOben() const { return spurenOben() + spurenHoehe(); }
    [[nodiscard]] int tabHoehe() const
    {
        return m_saiten.istSaiteninstrument() && m_tabAn ? m_saiten.anzahl() * kTabZeile + 2 * kTabRand : 0;
    }
    /// Die Mitte der Linie einer Saite (Stelle in `Saitensatz::leer`); oben die hoechste.
    [[nodiscard]] int tabLinieY(int stelle) const
    {
        return tabOben() + kTabRand + (m_saiten.anzahl() - 1 - stelle) * kTabZeile + kTabZeile / 2;
    }
    /// Die Saite (Stelle) unter `y` in den Tab-Linien; -1 = keine.
    [[nodiscard]] int tabSaiteBei(int y) const;
    /// Die Note, deren Zahl unter dem Punkt steht (Platz in `muster().noten`); -1 = keine.
    [[nodiscard]] int tabNoteBei(QPoint ort) const;
    /// Die Tab-Marke: dort landet ein getippter Bund. Schritt -1 = keine.
    void setzeTabMarke(int schritt, int stelle);
    [[nodiscard]] int tabSchritt() const { return m_tabSchritt; }
    [[nodiscard]] int tabSaite() const { return m_tabSaite; }
    /// B: an der Tab-Marke den Bund `bund` setzen — liegt dort schon eine Note auf
    /// dieser Saite, bekommt sie ihn; sonst eine neue Note von einem Schritt. Die
    /// Saite steht danach an der Note (S-M2 a). false = keine Marke, ausserhalb der
    /// Buende oder der Toene, oder dort klingt derselbe Ton schon.
    bool setzeBund(int bund);
    /// B: eine Note auf eine andere Saite legen (derselbe Ton). false = es aendert nichts.
    bool legeAufSaite(int platz, int stelle);
    /// D (Stufe 26e-2, S-T5 a, S-T8 a): ein Griffbild am Schritt `schritt` setzen — jede
    /// Note auf ihrer Saite, einen Schritt lang (wie eine getippte Zahl, B). Was an
    /// diesem Schritt auf einer Saite des Griffbilds liegt oder einen seiner Toene
    /// spielt, weicht; markiert sind danach die neuen Noten. Ein Schritt fuer Undo.
    /// false = ausserhalb des Musters, oder das Griffbild passt nicht. Ein Voicing
    /// (Stufe 26f, S-T14 a) setzt seinen tiefsten Ton auf `bezug`, ohne Saiten.
    bool setzeAkkord(const Akkord& lage, int schritt, int bezug = 0);
    /// Was in den Tab-Linien an einer Note steht: der Bund — ohne Buende der
    /// Tonname, nicht greifbar „!".
    [[nodiscard]] QString tabText(int platz) const;
    // --- Legato, Slide, Bend (Stufe 26d; S-L3 a A–G, S-T5 a C, E) --------------
    // Die Haken *Legato (hammer-on / pull-off)* und *Slide* im Menue (E) setzen die
    // Bits an den markierten Noten (sie schliessen sich aus); gezeigt als Bogen mit h/p,
    // als schraege Linie mit Pfeil (Piano-Roll) oder „/" (Tab); ohne vorige Note auf der
    // Saite ein rotes „!". Der Bend (D, F, G) als Kurve in der Note bis zum erreichten
    // Ton, gestrichelt das Ziel, beschriftet in Ganztoenen; in den Tab-Linien ein Pfeil.
    /// F: die Vorlage fuer die markierten Noten — mit ihrer Hoehe, ohne Bend „full";
    /// `Keine` nimmt ihn weg.
    bool setzeBendVorlage(BendVorlage art);
    /// F: die Hoehe des Bends der markierten Noten: eine Vorlage behaelt ihre Form,
    /// eine eigene Kurve wird gestreckt, ohne Bend wird es ein *Bend*.
    bool setzeBendHoehe(double halbtoene);
    /// G: die Kurve fuer die markierten Noten (Edit bend...).
    bool setzeBend(std::vector<BendPunkt> punkte);
    /// Die Hoehen, die das Menue Bend anbietet, in Halbtoenen (½, full, 1½, 2).
    [[nodiscard]] static const std::vector<double>& bendHoehen();
    /// Der Name einer Vorlage im Menue.
    [[nodiscard]] static QString bendVorlageName(BendVorlage art);
    static constexpr int kTabZeile = 14;  ///< je Saite
    static constexpr int kTabRand = 4;    ///< oben und unten
    /// So lange nach einer Ziffer zaehlt die naechste noch zum selben Bund (zweistellig).
    static constexpr int kTabZifferMs = 800;

    // --- Spuren ueber der Zeit (Stufe 28d; Skizze §1a, SK1 a, SK2 a) ------------
    // Unter der Spur Strum (oder der Zeile der Takte) je eingeblendeter Spur eine Leiste:
    // Sustain, Sostenuto, Expression, Vibrato, Pitch, danach die Schalter des Musters.
    // Links das Kuerzel (der volle Name im Tooltip). Pedale und Schalter als Flaeche von
    // unten, Expression, Vibrato und Pitch als Kurve mit Punkten, Pitch um eine
    // Mittellinie; eine leere Spur gestrichelt mit „Instrument default". Ein Klick setzt
    // einen Punkt auf den Schritt (mit Alt frei) und zieht ihn weiter, ein Punkt laesst
    // sich ziehen (zwischen seinen Nachbarn), ein Doppelklick nimmt ihn weg; im Menue
    // *Delete point* und *Clear lane*. Je Zug ein Schritt fuer Undo.
    /// Eine Spur der Leiste: eine Steuer-Spur — oder mit `schalter` ein Schalter beim Namen.
    struct LeistenSpur
    {
        Steuerspur steuer = Steuerspur::Sustain;
        QString schalter;  ///< leer = die Steuer-Spur

        [[nodiscard]] bool operator==(const LeistenSpur&) const = default;
    };
    /// Welche Steuer-Spuren das Instrument traegt (`traegtSpur`) und wie weit Pitch
    /// reicht (`pitchBereichHt`; die Spur zeigt ±so viel, 0 = 2).
    void setzeGetrageneSpuren(const std::array<bool, kSteuerspuren>& traegt, double pitchBereichHt);
    [[nodiscard]] bool traegt(Steuerspur spur) const { return m_traegt[static_cast<std::size_t>(spur)]; }
    /// Die Spuren, die die Zeile *Lanes* anbietet: die getragenen und die, die Werte
    /// tragen (sie klingen nur, wo das Instrument sie traegt), dann die Schalter des Musters.
    [[nodiscard]] std::vector<LeistenSpur> angeboteneSpuren() const;
    /// SK1 a: ein- und ausblenden ist nur Ansicht.
    void setzeSpurSichtbar(const LeistenSpur& spur, bool sichtbar);
    [[nodiscard]] bool spurSichtbar(const LeistenSpur& spur) const;
    /// SK1 a beim Oeffnen eines Musters: die Spuren mit Werten ein, die uebrigen aus.
    void zeigeSpurenMitWerten();
    [[nodiscard]] std::vector<LeistenSpur> sichtbareSpuren() const;
    [[nodiscard]] bool hatWerte(const LeistenSpur& spur) const;
    /// SK2 a: das Kuerzel in der linken Spalte (Sus, Sost, Expr, Vib, Pitch; ein Schalter
    /// die ersten sechs Zeichen seines Namens) und der volle Name.
    [[nodiscard]] static QString spurKuerzel(const LeistenSpur& spur);
    [[nodiscard]] static QString spurName(const LeistenSpur& spur);
    [[nodiscard]] int spurenOben() const { return strumOben() + strumHoehe(); }
    [[nodiscard]] int spurenHoehe() const { return static_cast<int>(sichtbareSpuren().size()) * kSpurHoehe; }
    [[nodiscard]] int spurOben(int nr) const { return spurenOben() + nr * kSpurHoehe; }
    /// Die Spur (Platz in `sichtbareSpuren`) unter `y`; -1 = keine.
    [[nodiscard]] int spurBei(int y) const;
    /// Die Grenzen der Werte einer Spur, wie sie gezeichnet wird.
    [[nodiscard]] std::pair<double, double> spurBereich(const LeistenSpur& spur) const;
    [[nodiscard]] int yVonWert(int nr, double wert) const;
    /// Der Wert unter `y` in der Spur `nr`, in ihren Grenzen (ein Schalter: 0 oder 1).
    [[nodiscard]] double wertBei(int nr, int y) const;
    /// Die Stelle unter `x` in Schritten: auf den naechsten Schritt gerundet, `frei` mit Bruchteil.
    [[nodiscard]] double stelleBei(int x, bool frei) const;
    [[nodiscard]] int xVonStelle(double stelle) const
    {
        return kTastenBreite + static_cast<int>(std::lround(stelle * m_schrittBreite)) - m_rollX;
    }
    /// Der Punkt der Spur `nr` unter dem Ort (bis `kPunktFang` Pixel daneben); -1 = keiner.
    [[nodiscard]] int spurPunktBei(int nr, QPoint ort) const;
    /// Einen Punkt setzen — an derselben Stelle ersetzt er den alten. false = es aendert nichts.
    bool setzeSpurPunkt(const LeistenSpur& spur, SpurPunkt punkt);
    bool loescheSpurPunkt(const LeistenSpur& spur, int index);
    /// *Clear lane*: alle Punkte der Spur weg. false = sie war leer.
    bool leereSpur(const LeistenSpur& spur);
    /// Ein Wert der Spur als Text: Pedale in Prozent, Expression mit dem Pegel in dB
    /// (SK9 a), Vibrato in Cent, Pitch in Halbtoenen, ein Schalter on/off.
    [[nodiscard]] static QString wertText(const LeistenSpur& spur, double wert);
    static constexpr int kSpurHoehe = 36;  ///< je eingeblendeter Spur
    static constexpr int kPunktFang = 6;   ///< so nah faengt die Maus einen Punkt

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
    [[nodiscard]] int sichtHoehe() const { return height() - rasterOben() - kLautHoehe; }
    /// So rollen, dass diese Tonhoehe in der Mitte liegt.
    void zeigeHoehe(int hoehe);

    // --- Zoom (Entscheid Patrik E-Z1 a, S135) — dieselben Gesten wie die Zeitachse (S102) --
    // Ctrl+Rad zoomt die Zeit, Alt+Rad die Hoehe der Zeilen, Ctrl+Alt+Rad beides, um die
    // Stelle unter der Maus; Faktor und Grenzen der Hoehe von `lumi::ui::Achse`.
    // „Fit" (`passeEin`): das ganze Muster in der Breite, die Noten in der Hoehe.
    static constexpr double kSchrittBreiteMin = 4.0;   ///< so eng wird ein Schritt hoechstens
    static constexpr double kSchrittBreiteMax = 96.0;  ///< und so breit
    /// Die Breite eines Schritts und die Hoehe einer Zeile, wie der Zoom sie gerade setzt.
    [[nodiscard]] int schrittBreite() const { return static_cast<int>(m_schrittBreite + 0.5); }
    [[nodiscard]] int zeilenHoehe() const { return kZeilenHoehe * m_zeilenProzent / 100; }
    [[nodiscard]] int zeilenProzent() const { return m_zeilenProzent; }
    /// Die Zeit zoomen: `faktor` ueber 1 = hinein; der Schritt unter `xFest` bleibt stehen.
    void zoomeZeit(double faktor, int xFest);
    /// Die Hoehe der Zeilen in Prozent (`Achse::kHoeheMin` … `kHoeheMax`); die
    /// Tonhoehe unter `yFest` bleibt stehen, -1 = die Oberkante der Noten.
    void setZeilenProzent(int prozent, int yFest = -1);
    /// **Fit:** das ganze Muster in der Breite, die Noten in der Hoehe (ohne Noten:
    /// 100 % um C4).
    void passeEin();
    /// Weicht der Zoom von der Normalgroesse ab (16 px je Schritt, 100 %)?
    [[nodiscard]] bool zoomVerstellt() const { return schrittBreite() != kSchrittBreite || m_zeilenProzent != 100; }

    // --- Geometrie (auch fuer Tests) ------------------------------------------
    static constexpr int kTastenBreite = 44;  ///< die Klaviatur links
    static constexpr int kKopfHoehe = 18;     ///< die Zeile mit den Takten
    static constexpr int kZeilenHoehe = 14;   ///< je Tonhoehe, ohne Zoom
    static constexpr int kSchrittBreite = 16;  ///< ohne Zoom
    static constexpr int kLautHoehe = 56;     ///< die Lautstaerke je Note, unten
    static constexpr int kStrumHoehe = 20;    ///< Stufe 24e (J, S-W14 a): die Spur Strum unter dem Kopf, nur mit Saiten
    static constexpr int kKante = 5;          ///< so breit ist der Griff am Ende einer Note mindestens
    static constexpr int kKanteMax = 9;       ///< und hoechstens (ein Drittel der Note, dazwischen)
    /// Liegt `x` auf dem Griff am Ende dieser Note? Dort zieht man ihre Laenge;
    /// der Mauszeiger zeigt es.
    [[nodiscard]] bool anKante(const Note& note, int x) const;
    [[nodiscard]] int xVonSchritt(int schritt) const { return kTastenBreite + schritt * schrittBreite() - m_rollX; }
    /// Der Schritt unter `x`; -1 = keiner (die Klaviatur, hinter dem Ende).
    [[nodiscard]] int schrittBei(int x) const;
    [[nodiscard]] int yVonHoehe(int hoehe) const { return rasterOben() + (kHoeheMax - hoehe) * zeilenHoehe() - m_rollY; }
    /// Oberkante der Noten: unter der Zeile der Takte, (mit Saiten) der Spur Strum, den
    /// eingeblendeten Spuren (Stufe 28d) und den Tab-Linien.
    [[nodiscard]] int rasterOben() const { return tabOben() + tabHoehe(); }
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
    /// Stufe 28d: ein Doppelklick auf einen Punkt einer Spur nimmt ihn weg.
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    /// D (Stufe 26e-2): ein Griffbild aus dem Fenster *Chords* — es landet am Schritt
    /// unter der Maus, in Tab-Linien und Piano-Roll.
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
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
        TabSaite,     ///< Stufe 26c: eine Zahl in den Tab-Linien auf eine andere Saite
        SpurPunkt,    ///< Stufe 28d: ein Punkt einer Spur ueber der Zeit
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
    double m_bpm = 120.0;  ///< Stufe 24e: fuer den Versatz der Akkordnoten
    double m_schrittBreite = kSchrittBreite;  ///< E-Z1 a: der Zoom der Zeit (mit Bruchteil, damit er nicht wandert)
    int m_zeilenProzent = 100;                ///< E-Z1 a: der Zoom der Hoehe
    int m_radRest = 0;  ///< was vom Mausrad noch keine ganze Raste ergab (Hoehen-Zoom)
    /// Griffe und Anschlaege je Note in der Reihenfolge des Musters — geplant auf
    /// einer sortierten Kopie (waehrend eines Zugs ist das Muster nicht gerichtet).
    void plane(std::vector<Griff>* griffe, std::vector<Anschlagplan>* anschlaege) const;
    /// Die Schritte, an denen markierte Noten anschlagen, je einmal, aufsteigend.
    [[nodiscard]] std::vector<int> markierteSchritte() const;
    /// *Spread...*: der Dialog `strumSpreadQuestion` (ueber `fuehreDialogAus`).
    void frageSpread(double vorgabe);
    // --- Stufe 26c: die Tab-Linien
    bool m_tabAn = true;
    int m_tabSchritt = -1;   ///< die Tab-Marke; -1 = keine
    int m_tabSaite = -1;
    int m_tabZiel = -1;      ///< beim Ziehen einer Zahl: die Saite unter der Maus
    int m_tabZiffer = -1;    ///< die zuletzt getippte Ziffer an dieser Marke; -1 = keine
    qint64 m_tabZifferZeit = 0;
    /// G: *Edit bend...* — der Dialog `bendDialog` (ueber `fuehreDialogAus`).
    void frageBend();
    /// Die markierten Noten mit dieser Aenderung, ein Schritt fuer Undo; false = es aendert nichts.
    bool aendereMarkierte(const std::function<void(Note&)>& aendere, const QString& was);
    /// Die Tab-Linien zeichnen (aus `paintEvent`).
    void zeichneTab(QPainter& p, const std::vector<Griff>& griffe, const std::vector<Anschlagplan>& anschlaege);
    // --- Stufe 28d: die Spuren ueber der Zeit (InstrumentEditorSpuren.cpp)
    std::array<bool, kSteuerspuren> m_traegt{true, true, true, true, true};
    double m_pitchBereich = kPitchHalbtoeneMax;
    std::array<bool, kSteuerspuren> m_steuerSichtbar{};
    std::vector<QString> m_schalterSichtbar;  ///< die eingeblendeten Schalter, beim Namen
    LeistenSpur m_spurZug;                    ///< beim Ziehen eines Punkts: seine Spur
    int m_spurIndex = -1;                     ///< und sein Platz darin
    /// Die Punkte einer Spur im Muster; nullptr = ein Schalter, den das Muster nicht traegt.
    [[nodiscard]] static std::vector<lumi::loop::SpurPunkt>* punkteVon(Muster& muster, const LeistenSpur& spur);
    [[nodiscard]] static const std::vector<lumi::loop::SpurPunkt>* punkteVon(const Muster& muster, const LeistenSpur& spur);
    /// Einen Punkt ins Muster legen, ohne zu melden (an derselben Stelle ersetzt er den
    /// alten); sein Platz in der Spur, -1 = nicht moeglich.
    int legeSpurPunkt(const LeistenSpur& spur, lumi::loop::SpurPunkt punkt);
    /// Den gegriffenen Punkt an diese Stelle ziehen: zwischen seinen Nachbarn, im Muster.
    void zieheSpurPunkt(QPoint ort, bool frei);
    /// Eine Aenderung an den Spuren melden (ein Schritt fuer Undo).
    void meldeSpuren(const QString& was);
    /// Die Spuren zeichnen (aus `paintEvent`).
    void zeichneSpuren(QPainter& p);
    /// Tooltip, Menue und Zeiger in den Spuren; false = der Ort liegt nicht in ihnen.
    bool spurTooltip(const QHelpEvent& hilfe);
    bool spurMenue(const QContextMenuEvent& event);
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
    /// H (Stufe 24e, S-W5 a): Strum dieses Musters — Spread, Hand und ihr Muster
    /// je Stelle eines Takts. Die Zeile steht nur da, wenn das Instrument Saiten traegt.
    void setzeStrum(const Strum& strum);
    /// Die Zeile Strum (fuer Tests).
    [[nodiscard]] StrumZeile* strumZeile() const { return m_strumZeile; }
    /// Stufe 28d: die Haken der Zeile *Lanes* (fuer Tests), in der Reihenfolge der Spuren.
    [[nodiscard]] const std::vector<QCheckBox*>& spurHaken() const { return m_spurHaken; }

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

    // --- das Fenster *Chords* (Stufe 26e-2, S-T2 a, S-T8 a) --------------------
    /// D: „Chords..." — das Fenster oeffnen (es entsteht beim ersten Mal), fuer die
    /// Stimmung dieses Musters. **Use** dort setzt das Griffbild an die
    /// Einfuegemarke. `zeigen` false fuellt es nur (fuer Tests).
    void oeffneAkkorde(bool zeigen = true);
    /// Das Fenster; nullptr, solange es noch nie geoeffnet wurde.
    [[nodiscard]] AkkordFenster* akkordFenster() const;

Q_SIGNALS:
    /// Der Creator will einen Ton hoeren: das Instrument beim Pfad seiner
    /// Vorhoer-Datei.
    void instrumentTonVerlangt(const QString& nennung, int hoehe);
    /// Der Creator spielt an (seit Stufe 24d mit den Noten nach *Play as*):
    /// `klingtMs` > 0 von „Repeat note" — so lang darf es klingen.
    void instrumentVorhoerenVerlangt(const QString& nennung, const Muster& noten, int klingtMs);
    /// Das Fenster *Chords* spielt ein Griffbild an (S-T9 a): diese Noten mit dem
    /// Instrument `nennung`, an ihren Schritten (Down-up schlaegt zweimal an).
    void akkordVorhoerenVerlangt(const QString& nennung, const Muster& noten);
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
    QPushButton* m_akkorde = nullptr;  ///< D (Stufe 26e-2): „Chords...", nur mit Saiten
    QPointer<AkkordFenster> m_akkordFenster;
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
    StrumZeile* m_strumZeile = nullptr;  ///< H (Stufe 24e, S-W5 a)
    QCheckBox* m_tabHaken = nullptr;     ///< F (Stufe 26c, S-T5 a): die Tab-Linien zeigen; der Editor merkt die Stellung
    // --- Stufe 28d (Skizze §1a, SK1 a): die Zeile Lanes
    QWidget* m_spurZeile = nullptr;
    QLabel* m_spurTitel = nullptr;
    std::vector<QCheckBox*> m_spurHaken;  ///< je angebotener Spur einer, in ihrer Reihenfolge
    /// Die Haken der Zeile Lanes nach dem Raster richten (Namen, Punkt bei Werten, Stellung).
    void zeigeSpurZeile();
};

}  // namespace lumi::loop
