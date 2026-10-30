/**
 * @file Taktachse.hpp
 * @brief Loop: die Achse des Loop-Panels — Lineal in Takten, Kanaele als
 *        Zeilen, Clips als Bloecke (docs/audio/Loop_Konzept.md §4c, Stufe 6a)
 *
 * Massstab, Zoom, Rollen, Mausrad, Schwenken und Mitlaufen kommen aus der
 * Basis `lumi::ui::Achse` — dieselbe Bedienung wie in der Zeitachse des
 * Composers (Entscheid Patrik S109, W1 b). Die Einheit der Basis ist hier ein
 * **Tick**, der 960. Teil eines Takts: Clips liegen auf ganzen Takten, die
 * Abspielmarke laeuft dazwischen.
 *
 * Bearbeitet wird immer (kein Schalter Edit): ein Clip laesst sich
 * verschieben — auch in einen anderen Kanal — und an beiden Kanten in ganzen
 * Mustern verlaengern und kuerzen; der Rechtsklick legt Clips und Kanaele an.
 * Die Achse aendert den Loop nicht selbst: sie meldet die neue Fassung mit
 * `bearbeitet`, das Panel fuehrt sie ueber Undo aus und zeigt sie zurueck.
 */
#pragma once

#include "UI/widgets/Achse.hpp"
#include "loop/Austausch.hpp"
#include "loop/Bearbeiten.hpp"
#include "loop/Freeze.hpp"
#include "loop/Loop.hpp"

#include <QColor>
#include <QPoint>
#include <QRect>
#include <QString>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <map>
#include <utility>
#include <vector>

class QPainter;

namespace lumi::loop
{

/// Wie ein Clip zu Freeze steht, fuer die Marke (Stufe 30g, FZ4 a A) — das Panel rechnet es aus.
struct FreezeMarke
{
    FrozenZustand zustand = FrozenZustand::Aus;
    bool rechnet = false;  ///< ein Auftrag friert ihn gerade ein: „computing… n %“
    double anteil = 0.0;   ///< dabei: 0 … 1
    // --- die Sanduhr (Stufe 30h; F4 a, FZ5 a bis FZ7 a) ---
    bool zeigeZeit = false;  ///< ueber der Schwelle und nicht eingefroren
    double arbeitMs = 0.0;   ///< die Rechenzeit, die Arbeit auf einem Kern (FZ6 a)
    int threads = 1;         ///< so viele rechnen — die Wartezeit ist `arbeitMs / threads`
    bool warten = false;     ///< die Sanduhr zeigt die Wartezeit statt der Arbeit (FZ7 a)

    [[nodiscard]] double gezeigtS() const { return (warten ? arbeitMs / std::max(1, threads) : arbeitMs) / 1000.0; }
    [[nodiscard]] bool operator==(const FreezeMarke&) const = default;
};

class Taktachse : public lumi::ui::Achse
{
    Q_OBJECT

public:
    explicit Taktachse(QWidget* parent = nullptr);

    /// So viele Ticks hat ein Takt.
    static constexpr std::int64_t kTicksJeTakt = 960;
    /// Die Spalte der Kanalnamen ist hier breiter als in der Basis: rechts steht
    /// der Fader des Kanals (Stufe 27e-2, Entscheid Patrik LL12 a).
    static constexpr int kKopfBreiteLoop = 150;
    [[nodiscard]] int kopfBreite() const override { return kKopfBreiteLoop; }
    /// Der feinste Massstab: vier Ticks je Pixel, ein Takt ist dann 240 Pixel breit.
    static constexpr double kFeinsteTicksJePixel = 4.0;

    /// Einen anderen Loop zeigen (`geladen` false = der Hinweis „kein Loop"):
    /// die Auswahl faellt, der Zoom beginnt von vorn.
    void setLoop(const Loop& loop, bool geladen);
    /// Die bearbeitete Fassung desselben Loops zeigen (auch nach Undo): Zoom,
    /// Rollen und — wo es den Clip noch gibt — die Auswahl bleiben.
    void zeigeBearbeitet(const Loop& loop);
    [[nodiscard]] const Loop& loop() const { return m_loop; }

    /// Die Abspielmarke: gezeichnet wird sie nur, solange der Loop spielt.
    void setSpielt(bool spielt);
    /// Steht die Abspielmarke (das Panel spielt, oder der Composer spielt diesen Loop)?
    [[nodiscard]] bool spielt() const { return m_spielt; }
    /// Die Stelle im Durchlauf, in Schritten (wie `Spieler::stelleSchritte`).
    void setStelleSchritte(double schritte);
    /// Weitere Stellen, an denen der Composer diesen Loop zugleich spielt — die
    /// leiseren Clips, blasser gezeichnet (S130, Entscheid Patrik W8 a). Leer = keine.
    void setNebenStellenSchritte(std::vector<double> schritte);
    [[nodiscard]] const std::vector<double>& nebenStellenSchritte() const { return m_nebenSchritte; }
    /// Wo Play beginnt (Entscheid Patrik B4, S124), in Schritten; gezeichnet im
    /// Lineal, solange der Loop nicht spielt und sie nicht am Anfang steht.
    void setStartmarke(double schritte);
    [[nodiscard]] double startmarke() const { return m_startSchritte; }
    /// Die Stelle unter `x`, in ganzen Schritten des Durchlaufs (fuer das Lineal).
    [[nodiscard]] int schrittBeiX(int x) const;

    [[nodiscard]] int gewaehlterKanal() const { return m_kanal; }
    [[nodiscard]] int gewaehlterClip() const { return m_clip; }
    /// Der angeklickte Clip, nullptr = keiner.
    [[nodiscard]] const LoopClip* auswahl() const;
    /// Genau diesen Clip markieren (-1, -1 = keinen).
    void waehle(int kanal, int clip);

    // --- Mehrere Clips markieren (Stufe 12c) --------------------------------
    // Markiert wird mit Klick (einer), Ctrl+Klick (dazu oder heraus) und einem
    // Rahmen auf freier Flaeche. Der zuletzt angeklickte ist der Clip, den
    // `auswahl` nennt — fuer ihn gelten Edit base und Rename base.
    /// Alle markierten Clips; der angeklickte ist darunter.
    [[nodiscard]] const std::vector<ClipOrt>& markierte() const { return m_gruppe; }
    [[nodiscard]] bool istMarkiert(int kanal, int clip) const;
    /// Den Clip dazunehmen oder herausnehmen (Ctrl+Klick).
    void markiereUm(int kanal, int clip);
    /// Genau diese Clips markieren; der erste wird der angeklickte.
    void markiere(const std::vector<ClipOrt>& orte);
    void markiereAlle();
    /// Die Clips, die der Rahmen beruehrt (in Fensterpunkten).
    [[nodiscard]] std::vector<ClipOrt> clipsIm(const QRect& rahmen) const;

    // --- Bearbeiten: was Rechtsklick und Tasten ausloesen (auch fuer Tests) ---
    /// Einen leeren Clip (4 Takte) an den Takt legen — ein Drumkit oder, mit
    /// `MusterArt::Instrument`, ein Instrument. false = kein Platz.
    bool neuerClip(int kanal, int takt, MusterArt art = MusterArt::Drumkit);
    /// Eine verbundene Kopie hinter den angeklickten Clip legen — sind mehrere
    /// markiert, Kopien von allen, als Gruppe dahinter.
    bool dupliziereAuswahl();
    /// Den angeklickten Clip loeschen — oder alle markierten.
    bool loescheAuswahl();
    /// Die Basis des angeklickten Clips umbenennen (fuer alle seine Kopien).
    bool benenneBasisUm(const QString& name);
    /// „Edit base": den Editor der Basis des angeklickten Clips verlangen
    /// (auch der Doppelklick auf einen Clip).
    bool bearbeiteBasis();
    /// „Make new base and edit" (Entscheid Patrik S108, L8 a): den angeklickten
    /// Clip von seiner Basis loesen — er spielt eine Kopie unter neuem Namen —
    /// und deren Editor verlangen.
    bool neueBasisUndBearbeiten();
    /// „Effects...": das Fenster der Effekte fuer die markierten Clips
    /// verlangen (Stufe 12d). false = kein Clip markiert.
    bool verlangeEffekte();
    /// „Export audio..." (Stufe 30c, WX6 a): den Export des angeklickten Clips verlangen.
    /// false = keiner oder mehrere markiert.
    bool verlangeExport();
    /// *Freeze clip* / *Unfreeze clip* (Stufe 30g, FZ4 a B) fuer die markierten Clips verlangen.
    /// false = keiner markiert.
    bool verlangeFreeze(bool einfrieren);

    // --- Stufe 33 (Loop_Konzept §4o, PE1, LC1 a): Muster, Clips und Spuren als Dateien ---
    /// Wie die Menues nach einer Datei fragen: `speichern` = Speichern-Dialog; der Pfad
    /// oder leer (abgebrochen). Ohne Ersatz `QFileDialog`; in Tests ersetzbar.
    using Dateiwahl =
        std::function<QString(bool speichern, const QString& titel, const QString& vorschlag, const QString& filter)>;
    static void setzeDateiwahlFuerTests(Dateiwahl wahl);
    /// *Export pattern...* (die Basis des Clips), *Export clip...*, *Export track...*: fragt
    /// nach der Datei — vorgeschlagen im Benutzerordner — und schreibt sie. false =
    /// abgebrochen oder nicht geschrieben (dann mit Meldung).
    bool exportiereMuster(int kanal, int clip);
    bool exportiereClip(int kanal, int clip);
    bool exportiereSpur(int kanal);
    /// *Import pattern...* und *Import clip...* ab `takt` in `kanal`, *Import track...* als
    /// neuer Kanal: fragt nach der Datei, setzt ein (mit Undo) und meldet, was dabei
    /// geschah (umgerechnet, umbenannt, neuer Kanal, fehlende Dateien). false = abgebrochen
    /// oder keine solche Datei (dann mit Meldung).
    bool importiere(AustauschArt art, int kanal = -1, int takt = 0);

    /// Die Marken (FZ4 a A), je Clip — Kanal, Starttakt —; ein Clip ohne Eintrag hat keine.
    void setzeFreezeMarken(std::map<std::pair<int, int>, FreezeMarke> marken);
    [[nodiscard]] FreezeMarke freezeMarke(int kanal, int startTakt) const;
    /// Was die Marke sagt: „❄ frozen“, „computing… 40 %“, „❄ frozen · changed“; leer = keine.
    [[nodiscard]] static QString freezeText(const FreezeMarke& marke);
    /// Der Tooltip dazu, mit Erklaerung — an der Sanduhr beide Zahlen (FZ7 a).
    [[nodiscard]] static QString freezeTooltip(const FreezeMarke& marke);
    /// Was die Sanduhr sagt: „⏳ 3.2 s“; leer = keine.
    [[nodiscard]] static QString zeitText(const FreezeMarke& marke);
    void kanalHinzu();
    void kanalLoeschen(int kanal);
    void kanalUmbenennen(int kanal, const QString& name);
    void kanalSchalten(int kanal, bool an);
    /// Den Fader des Kanals stellen (Stufe 27e-2): begrenzt auf `kKanalGainMinDb` …
    /// `kKanalGainMaxDb`, ein Undo-Schritt. Auch Ziehen, Rad, Doppelklick und Ctrl+Klick am Feld.
    void kanalGain(int kanal, double db);
    /// Das Feld des Faders im Kopf eines Kanals; leer = es gibt den Kanal nicht
    /// oder seine Zeile ist zu niedrig fuer Schrift.
    [[nodiscard]] QRect faderFeld(int kanal) const;
    /// Der Kanal, dessen Fader gerade gezogen wird (-1 = keiner).
    [[nodiscard]] int faderImZug() const { return m_faderZug; }
    /// Wie der Fader im Kopf steht: „+4.0 dB", „0.0 dB", „−6.5 dB", „−∞ dB".
    [[nodiscard]] static QString faderText(double db);
    /// Was die Statuszeile ueber dem Fader sagt.
    [[nodiscard]] static QString faderHinweis();
    /// Den Kanal an einen anderen Platz legen (S127, Entscheid Patrik 7b a) —
    /// auch durch Ziehen am Namen oder Rechtsklick „Move channel up/down".
    /// Clips, Marken und Auswahl gehen mit; ein Undo-Schritt.
    void kanalVerschieben(int von, int nach);
    /// Der Kanal, der gerade am Namen gezogen wird, und wo er landete (-1 = keiner).
    [[nodiscard]] int kanalImZug() const { return m_kanalZug; }
    [[nodiscard]] int kanalZiel() const { return m_kanalZiel; }

    // --- Geometrie (auch fuer Tests) ----------------------------------------
    [[nodiscard]] int xVonTakt(int takt) const { return xVon(takt * kTicksJeTakt); }
    /// Der Takt, in dem `x` liegt (0 = der erste).
    [[nodiscard]] int taktBei(int x) const;
    /// Der Kanal der Zeile unter `y`; -1 = keine (Lineal, die Zeile fuer einen
    /// neuen Kanal, freie Flaeche).
    [[nodiscard]] int kanalBei(int y) const;
    /// Das Rechteck eines Clips im Fenster; leer = es gibt ihn nicht.
    [[nodiscard]] QRect clipRechteck(int kanal, int clip) const;
    /// So viele Takte zeigt die Achse fuer diesen Loop: seine Laenge und Luft
    /// dahinter, damit sich ueber das Ende hinaus ziehen laesst.
    [[nodiscard]] static int anzeigeTakte(const Loop& loop);
    /// Die Farbe der Clips einer Basis: je Basis ein eigener Ton, damit man
    /// verbundene Kopien erkennt (S109). `an` false = Kanal aus, grau.
    [[nodiscard]] QColor basisFarbe(const QString& muster, bool an) const;

    // --- Die Expression am Clip (Stufe 28e; Skizze §1a, SK8 a, SK13 a) ---------
    // Jeder Clip mit Kurve zeichnet sie ueber seine ganze Laenge. Der angeklickte
    // (allein markierte) zeigt dazu die Punkte, ohne Kurve eine gestrichelte Linie bei
    // 1 („Expression 1"): ein Klick auf Linie oder Kurve setzt einen Punkt und zieht ihn
    // (in Sechzehnteln eines Takts, mit Alt frei), ein Punkt laesst sich ziehen, ein
    // Doppelklick nimmt ihn weg; im Menue *Clear expression*. Punkte hinter dem Ende des
    // Clips sind blass (SK8 a).
    /// Die Hoehe eines Werts 0 … 1 im Rechteck eines Clips (1 oben, 0 unten).
    [[nodiscard]] static int yVonAusdruck(const QRect& clip, double wert);
    /// Der Wert unter `y` im Rechteck eines Clips, auf Hundertstel.
    [[nodiscard]] static double ausdruckBei(const QRect& clip, int y);
    /// Die Stelle unter `x` in Takten ab dem Anfang des Clips: auf Sechzehntel eines
    /// Takts, `frei` auf Hundertstel.
    [[nodiscard]] double clipStelleBei(const LoopClip& clip, int x, bool frei) const;
    [[nodiscard]] int xVonClipStelle(const LoopClip& clip, double stelle) const;
    /// Bearbeitet die Maus gerade die Kurve dieses Clips (er ist allein markiert)?
    [[nodiscard]] bool kurveGreifbar(int kanal, int clip) const;
    /// Der Punkt der Kurve unter dem Ort (bis 5 Pixel daneben); -1 = keiner.
    [[nodiscard]] int ausdruckPunktBei(int kanal, int clip, QPoint ort) const;
    /// Liegt der Ort auf der Linie oder der Kurve (bis 4 Pixel daneben)?
    [[nodiscard]] bool aufAusdruck(int kanal, int clip, QPoint ort) const;
    /// Einen Punkt setzen (an derselben Stelle ersetzt er den alten) — ein Undo-Schritt.
    bool setzeAusdruckPunkt(int kanal, int clip, SpurPunkt punkt);
    bool loescheAusdruckPunkt(int kanal, int clip, int index);
    /// *Clear expression*. false = der Clip hat keine Kurve.
    bool leereAusdruck(int kanal, int clip);

Q_SIGNALS:
    /// Ein Clip wurde angeklickt — oder die freie Flaeche (dann -1, -1).
    void auswahlGeaendert();
    /// Eine Aenderung ist fertig: die neue Fassung und ein Wort dazu fuer Undo.
    void bearbeitet(const lumi::loop::Loop& neu, const QString& was);
    /// Der Editor dieser Basis soll sich oeffnen.
    void editorVerlangt(const QString& muster);
    /// Das Fenster „Clip effects" soll sich fuer die markierten Clips oeffnen.
    void effekteVerlangt();
    /// Das Fenster „Export audio" soll sich fuer diesen Clip oeffnen (Stufe 30c).
    void exportVerlangt(int kanal, int startTakt);
    /// Die markierten Clips sollen einfrieren (`true`) oder ihre Marke verlieren (Stufe 30g).
    void freezeVerlangt(bool einfrieren);
    /// Ein Klick oder Zug im Lineal (B4): dorthin springen, oder dort beginnen.
    void stelleGewaehlt(int schritt);
    /// Ein Hinweis zur Bedienung unter der Maus (Stufe 27e-2: am Fader, auch sein
    /// Wert waehrend des Ziehens); leer = keiner mehr.
    void hinweis(const QString& text);

protected:
    bool event(QEvent* event) override;  ///< der Tooltip der Marke Freeze
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    [[nodiscard]] bool istGeladen() const override { return m_geladen; }
    /// Die Kanaele und eine Zeile mehr: die Flaeche, auf der ein neuer Kanal entsteht.
    [[nodiscard]] int zeilenZahl() const override;
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    /// Der Kanal, auf dessen Fader `ort` zeigt; -1 = keiner.
    [[nodiscard]] int faderBei(QPoint ort) const;
    /// Die Maus steht ueber dem Fader von `kanal` (-1 = ueber keinem): Hervorhebung und Hinweis.
    void setzeFaderUnterMaus(int kanal);
    /// Ein Zug an einem Clip.
    enum class Griff
    {
        Keiner,
        Verschieben,
        KanteLinks,
        KanteRechts,
        Rahmen,  ///< auf freier Flaeche: ein Rahmen, der Clips markiert
        Ausdruck,  ///< Stufe 28e: ein Punkt der Expression am angeklickten Clip
    };
    /// Einen Punkt in die Kurve legen, ohne zu melden; sein Platz, -1 = nicht moeglich.
    static int legeAusdruckPunkt(LoopClip& clip, SpurPunkt punkt);
    /// Den gegriffenen Punkt an diese Stelle ziehen: zwischen seinen Nachbarn, im Clip.
    void zieheAusdruck(QPoint ort, bool frei);
    /// Die Kurve eines Clips zeichnen (aus `paintEvent`).
    void zeichneAusdruck(QPainter& p, int kanal, int clip, const QRect& r);
    int m_ausdruckIndex = -1;    ///< der gegriffene Punkt
    bool m_ausdruckNeu = false;  ///< der Zug begann mit einem neuen Punkt
    bool m_hinweisAusdruck = false;  ///< der Hinweis spricht gerade von der Kurve
    /// Die Marken in Ordnung bringen, nachdem sich der Loop oder der
    /// angeklickte Clip geaendert hat: was es nicht mehr gibt, faellt heraus;
    /// der angeklickte gehoert immer dazu.
    void richteGruppe();
    [[nodiscard]] ClipOrt ortVon(int kanal, int clip) const;
    [[nodiscard]] Griff griffBei(QPoint ort, int* kanal, int* clip) const;
    void zieheZu(QPoint ort);
    void brichZugAb();
    /// Eine selbst erzeugte Fassung zeigen, die Auswahl setzen und sie melden.
    void uebernimm(const Loop& neu, int kanal, int clip, const QString& was);

    Loop m_loop;
    bool m_geladen = false;
    bool m_spielt = false;
    int m_kanal = -1;
    int m_clip = -1;

    Griff m_griff = Griff::Keiner;
    bool m_zugLaeuft = false;  ///< die Maus hat sich seit dem Druck weit genug bewegt
    QPoint m_zugStart;
    std::int64_t m_zugVersatz = 0;  ///< Abstand Maus – Clip-Start beim Druck, in Ticks
    int m_zugKanal = -1;            ///< Kanal und
    int m_zugClip = -1;             ///< Platz des Clips in `m_vorZug`
    Loop m_vorZug;                  ///< der Stand beim Druck

    std::vector<ClipOrt> m_gruppe;     ///< alle markierten Clips
    std::map<std::pair<int, int>, FreezeMarke> m_freeze;  ///< die Marken Freeze (Stufe 30g)
    std::vector<ClipOrt> m_vorGruppe;  ///< die Marken beim Druck
    /// Der Druck traf einen von mehreren markierten Clips: wird nicht gezogen,
    /// bleibt beim Loslassen nur er markiert.
    bool m_klickSammelt = false;
    int m_gutKanal = 0;  ///< der letzte Versatz der Gruppe, an dem Platz war
    int m_gutTakt = 0;
    QRect m_rahmen;      ///< der Rahmen, solange er gezogen wird
    double m_startSchritte = 0.0;  ///< B4: wo Play beginnt
    std::vector<double> m_nebenSchritte;  ///< W8 a: die blasseren Marken der leiseren Clips
    bool m_imLineal = false;       ///< die Maus zieht im Lineal
    int m_kanalZug = -1;           ///< der Kanal, der am Namen gezogen wird (7b)
    int m_kanalZiel = -1;          ///< der Platz, auf den er fiele
    QPoint m_kanalZugStart;
    int m_faderZug = -1;         ///< der Kanal, dessen Fader gezogen wird (27e-2)
    int m_faderStartY = 0;
    double m_faderStartDb = 0.0;
    int m_faderUnterMaus = -1;   ///< hervorgehoben, solange die Maus darueber steht
    int m_faderRadRest = 0;      ///< ein feines Rad sammelt bis zur Raste
};

}  // namespace lumi::loop
