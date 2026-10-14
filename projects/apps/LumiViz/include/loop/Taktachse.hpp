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
#include "loop/Bearbeiten.hpp"
#include "loop/Loop.hpp"

#include <QColor>
#include <QPoint>
#include <QRect>
#include <QString>

#include <cstdint>
#include <vector>

namespace lumi::loop
{

class Taktachse : public lumi::ui::Achse
{
    Q_OBJECT

public:
    explicit Taktachse(QWidget* parent = nullptr);

    /// So viele Ticks hat ein Takt.
    static constexpr std::int64_t kTicksJeTakt = 960;
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
    /// Die Stelle im Durchlauf, in Schritten (wie `Spieler::stelleSchritte`).
    void setStelleSchritte(double schritte);
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
    void kanalHinzu();
    void kanalLoeschen(int kanal);
    void kanalUmbenennen(int kanal, const QString& name);
    void kanalSchalten(int kanal, bool an);

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

Q_SIGNALS:
    /// Ein Clip wurde angeklickt — oder die freie Flaeche (dann -1, -1).
    void auswahlGeaendert();
    /// Eine Aenderung ist fertig: die neue Fassung und ein Wort dazu fuer Undo.
    void bearbeitet(const lumi::loop::Loop& neu, const QString& was);
    /// Der Editor dieser Basis soll sich oeffnen.
    void editorVerlangt(const QString& muster);
    /// Das Fenster „Clip effects" soll sich fuer die markierten Clips oeffnen.
    void effekteVerlangt();
    /// Ein Klick oder Zug im Lineal (B4): dorthin springen, oder dort beginnen.
    void stelleGewaehlt(int schritt);

protected:
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

private:
    /// Ein Zug an einem Clip.
    enum class Griff
    {
        Keiner,
        Verschieben,
        KanteLinks,
        KanteRechts,
        Rahmen,  ///< auf freier Flaeche: ein Rahmen, der Clips markiert
    };
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
    std::vector<ClipOrt> m_vorGruppe;  ///< die Marken beim Druck
    /// Der Druck traf einen von mehreren markierten Clips: wird nicht gezogen,
    /// bleibt beim Loslassen nur er markiert.
    bool m_klickSammelt = false;
    int m_gutKanal = 0;  ///< der letzte Versatz der Gruppe, an dem Platz war
    int m_gutTakt = 0;
    QRect m_rahmen;      ///< der Rahmen, solange er gezogen wird
    double m_startSchritte = 0.0;  ///< B4: wo Play beginnt
    bool m_imLineal = false;       ///< die Maus zieht im Lineal
};

}  // namespace lumi::loop
