/**
 * @file AkkordFenster.hpp
 * @brief Loop: das Fenster *Chords* — Griffbilder suchen, anhoeren und ins Muster
 *        setzen (docs/audio/Saiten_Konzept.md S-T2 a, S-T8 a bis S-T10 a, S-T12 b;
 *        Stufe 26e-2, S138)
 *
 * Nicht modal, wie das Fenster *Instruments*. **A** oben *For:* — Instrument, Muster,
 * Stimmung und Capo; gezeigt wird, was nach `passtZu` dazu passt. **B** *Search* und
 * *Type*. **C** links der Baum der Ordner: ★ *Favorites*, *Recently used*, die zwoelf
 * Grundtoene mit ihren Arten, *Your chords*. **D** rechts die Galerie: ein kleines
 * Griffbild je Lage unter Koepfen nach Art; ein Klick waehlt und spielt an, ein
 * Doppelklick setzt an die Einfuegemarke, ein Bild auf Tab-Linien oder Piano-Roll
 * ziehen setzt dort. **E** unten das gewaehlte gross, mit Lage, Toenen, *computed*
 * (S-T4 a) und *unusual* (S-T12 b); **F** *Move −1 / +1 fret*; **G** vier Symbole
 * zum Anhoeren (S-T9 a — die gewaehlte Spielart bleibt und wird gemerkt, S-T10 a),
 * ★ und **Use**.
 *
 * Das Fenster setzt selbst nichts: **Use** meldet `verwenden`, das Anhoeren
 * `vorhoerenVerlangt` — der Instrument-Editor fuehrt beides aus.
 * Favoriten, *Recently used*, die Spielart und die Lage des Fensters merkt die App
 * (`chordWindow/…`); die Spielart nicht in der Test-Exe (`setzeSpielartMerken`).
 */
#pragma once

#include "loop/Akkord.hpp"

#include <QStringList>
#include <QWidget>

#include <optional>
#include <vector>

class QButtonGroup;
class QComboBox;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QScrollArea;
class QToolButton;
class QTreeWidget;

namespace lumi::loop
{

class GriffbildFeld;

/// G (S-T9 a) und M (S-T11 a): die vier Symbole zum Anhoeren ↓ ↑ ↓↑ = in `reihe`,
/// je ein Knopf mit der Spielart als id der Gruppe; Namen `chordPlayDown`, `chordPlayUp`,
/// `chordPlayDownUp`, `chordPlayTogether`.
QButtonGroup* baueSpielartKnoepfe(QWidget* eltern, QHBoxLayout* reihe);

/// Die Galerie (D): kleine Griffbilder in Reihen, unter Koepfen.
class AkkordGalerie : public QWidget
{
    Q_OBJECT

public:
    struct Gruppe
    {
        QString titel;
        std::vector<Akkord> akkorde;
    };
    explicit AkkordGalerie(QWidget* parent = nullptr);
    void setzeGruppen(std::vector<Gruppe> gruppen);
    [[nodiscard]] const std::vector<Gruppe>& gruppen() const { return m_gruppen; }
    void setzeGewaehlt(const QString& nennung);
    [[nodiscard]] const QString& gewaehlt() const { return m_gewaehlt; }
    /// Voicings (S-T14 a D): ab welcher Oktave ihr tiefster Ton gezeichnet wird.
    void setzeOktavBeginn(int ton) { m_oktav = ton; update(); }
    /// Die Zelle eines Akkords (fuer Tests und das Rollen); leer = nicht zu sehen.
    [[nodiscard]] QRect zelle(const QString& nennung) const;
    /// Der Akkord unter dem Punkt; leer = keiner.
    [[nodiscard]] QString nennungBei(QPoint ort) const;

    static constexpr int kBildBreite = 62;
    static constexpr int kBildHoehe = 72;
    static constexpr int kZelleBreite = 78;
    static constexpr int kZelleHoehe = kBildHoehe + 32;  ///< mit zwei Zeilen Schrift
    static constexpr int kKopfHoehe = 24;
    static constexpr int kRand = 6;

Q_SIGNALS:
    void angeklickt(const QString& nennung);
    void aktiviert(const QString& nennung);
    /// Die Maus zieht ein Bild (D: auf Tab-Linien oder Piano-Roll).
    void ziehen(const QString& nennung);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    bool event(QEvent* event) override;

private:
    struct Platz
    {
        QRect rechteck;
        int gruppe = -1;
        int akkord = -1;  ///< −1 = der Kopf
    };
    void ordne();
    [[nodiscard]] const Akkord* akkordVon(const QString& nennung) const;

    std::vector<Gruppe> m_gruppen;
    std::vector<Platz> m_plaetze;
    QString m_gewaehlt;
    int m_oktav = 48;
    QPoint m_druck;
    QString m_druckNennung;
};

class AkkordFenster : public QWidget
{
    Q_OBJECT

public:
    explicit AkkordFenster(QWidget* parent = nullptr);

    /// Fuer welches Muster das Fenster waehlt: Name, Titel des Instruments, seine
    /// Saiten (die Stimmung entscheidet, was passt), Strum des Musters (Spread beim
    /// Anhoeren) und die Schritte je Schlag (Down-up). Ohne Muster ist **Use** aus.
    /// Ohne Saiten zeigt es die Voicings (S-T14 a); `oktavBeginn` ist dann die Oktave
    /// ihres tiefsten Tons (48 = C3, ein Bass 36).
    void setzeZiel(const QString& muster, const QString& instrumentTitel, const Saitensatz& saiten, const Strum& strum,
                   int schritteJeSchlag, int oktavBeginn = 48);
    /// Zeigt das Fenster Voicings (das Instrument hat keine Saiten)?
    [[nodiscard]] bool zeigtVoicings() const { return !m_saiten.istSaiteninstrument(); }
    /// Die Akkorde neu lesen (eigene aendern sich), dazu Favoriten und Zuletzt.
    void liesNeu();
    void zeigeVorn();

    // --- was die Bedienelemente ausloesen (auch fuer Tests) -------------------
    void setzeSuche(const QString& text);
    /// Leer = *All types*.
    void setzeArt(const QString& art);
    /// Einen Ordner des Baums waehlen: `favorites`, `recent`, `own`, `root:<0…11>`,
    /// `root:<0…11>:<art>`. false = es gibt ihn gerade nicht.
    bool waehleOrdner(const QString& ordner);
    [[nodiscard]] QString ordner() const;
    /// Einen Akkord waehlen, ohne ihn anzuspielen; die Verschiebung beginnt bei 0.
    bool waehle(const QString& nennung);
    [[nodiscard]] QString gewaehlt() const;
    /// F: die gewaehlte Lage um `buende` verschieben. false = es geht nicht weiter.
    bool verschiebe(int buende);
    [[nodiscard]] int verschiebung() const { return m_verschiebung; }
    /// F bei Voicings (S-T14 a): um `oktaven` Oktaven. false = es geht nicht weiter.
    bool verschiebeOktave(int oktaven);
    /// Der tiefste Ton des gewaehlten Voicings.
    [[nodiscard]] int tiefster() const { return m_tiefster; }
    /// Was mit der Lage zu den Noten gehoert (`akkordNoten`): ein Griffbild ist schon
    /// verschoben (0), ein Voicing traegt seinen tiefsten Ton.
    [[nodiscard]] int bezug() const { return zeigtVoicings() ? m_tiefster : 0; }
    /// Die gewaehlte Lage mit ihrer Verschiebung; nichts = keine gewaehlt.
    [[nodiscard]] std::optional<Akkord> lage() const;
    /// G: die Spielart; `anspielen` spielt gleich (wie ein Klick auf das Symbol).
    void setzeSpielart(AkkordSpielart art, bool anspielen = false);
    [[nodiscard]] AkkordSpielart spielart() const { return m_spielart; }
    bool spieleAn();
    bool verwende();
    void setzeFavorit(bool an);
    [[nodiscard]] bool istFavorit(const QString& nennung) const;
    [[nodiscard]] QStringList favoriten() const { return m_favoriten; }
    [[nodiscard]] QStringList zuletzt() const { return m_zuletzt; }
    /// Nach einem Ziehen ins Raster: unter *Recently used* merken.
    void merkeVerwendet(const QString& nennung);
    void nimmZurueck();
    void stelleWiederHer();

    // --- H (Stufe 26e-3, S-T8 a H, S-T11 a): eigene Griffbilder ------------------
    /// *New...*: der Dialog *Edit chord* mit einem leeren Griffbild dieser Stimmung.
    /// true = gespeichert (es steht dann gewaehlt unter *Your chords*).
    bool neu();
    /// *Edit...*: die gewaehlte Lage (mit Verschiebung) im Dialog — ein eigenes
    /// ersetzt sich, eine Vorlage wird eine eigene Kopie.
    bool bearbeite();
    /// *Duplicate*: die gewaehlte Lage als eigene Kopie, ohne Dialog (`_2` wie *Keep both*).
    bool dupliziere();
    /// *Delete*: das gewaehlte eigene nach Rueckfrage loeschen (`chordDeleteQuestion`,
    /// ueber `fuehreDialogAus`). false = kein eigenes oder abgelehnt.
    bool loescheGewaehlten();

    /// Was die Galerie zeigt, von oben nach unten: die Koepfe als `[C major]`,
    /// dazwischen die Nennungen.
    [[nodiscard]] QStringList galerieZeilen() const;
    /// Die Ordner des Baums, wie sie zu sehen sind: je Zeile `Text` (eingerueckt mit zwei Leerzeichen).
    [[nodiscard]] QStringList baumZeilen() const;
    /// Was die Leiste ueber die gewaehlte Lage sagt (E).
    [[nodiscard]] QString detailText() const;
    [[nodiscard]] QString ungewoehnlichText() const;
    [[nodiscard]] AkkordGalerie* galerie() const { return m_galerie; }
    [[nodiscard]] GriffbildFeld* feld() const { return m_feld; }

    /// Die Spielart ueber einen Neustart merken (S-T10 a); die Test-Exe schaltet es ab.
    static void setzeSpielartMerken(bool an);
    static constexpr int kZuletztMax = 12;
    /// Was ein gezogenes Bild traegt: `<nennung>\n<bezug>` (Griffbild: die
    /// Verschiebung; Voicing: der tiefste Ton, falls es nicht auf einer Zeile landet).
    static constexpr const char* kMimeTyp = "application/x-lumiviz-chord";

Q_SIGNALS:
    /// Diese Lage (ein Griffbild schon verschoben) an die Einfuegemarke setzen; `bezug`
    /// wie `akkordNoten` (beim Voicing der tiefste Ton).
    void verwenden(const lumi::loop::Akkord& lage, int bezug);
    /// Diese Noten mit dem Instrument des Musters anspielen.
    void vorhoerenVerlangt(const lumi::loop::Muster& noten);
    void rueckgaengigVerlangt(bool wiederholen);

protected:
    void hideEvent(QHideEvent* event) override;

private:
    void baueBaum();
    void zeigeGalerie();
    void zeigeDetail();
    void merke() const;
    void zieheAb(const QString& nennung);
    /// Den Dialog *Edit chord* fuer `vorlage` zeigen; true = gespeichert und gewaehlt.
    bool oeffneDialog(const Akkord& vorlage);
    /// Nach Speichern oder Loeschen: neu lesen, den eigenen waehlen.
    void zeigeEigenen(const QString& nennung);
    [[nodiscard]] const Akkord* finde(const QString& nennung) const;
    [[nodiscard]] bool passt(const Akkord& akkord) const;
    /// Die Akkorde eines Ordners als Gruppen der Galerie.
    [[nodiscard]] std::vector<AkkordGalerie::Gruppe> gruppenVon(const QString& ordner) const;

    QString m_muster;
    Saitensatz m_saiten;
    Strum m_strum;
    int m_jeSchlag = 4;
    std::vector<Akkord> m_alle;  ///< was zur Stimmung passt: mitgelieferte, dann eigene
    QStringList m_favoriten;
    QStringList m_zuletzt;
    int m_verschiebung = 0;
    int m_tiefster = 48;     ///< Voicing: der tiefste Ton
    int m_oktavBeginn = 48;  ///< Voicing: wo er zuerst liegt
    AkkordSpielart m_spielart = AkkordSpielart::Ab;

    QLabel* m_zielZeile = nullptr;
    QLineEdit* m_suche = nullptr;
    QComboBox* m_art = nullptr;
    QTreeWidget* m_baum = nullptr;
    QScrollArea* m_rolle = nullptr;
    AkkordGalerie* m_galerie = nullptr;
    GriffbildFeld* m_feld = nullptr;
    QLabel* m_titel = nullptr;
    QLabel* m_toene = nullptr;
    QLabel* m_gerechnet = nullptr;
    QLabel* m_ungewoehnlich = nullptr;
    QPushButton* m_tiefer = nullptr;
    QPushButton* m_hoeher = nullptr;
    QLabel* m_schiebenText = nullptr;
    QPushButton* m_oktTiefer = nullptr;  ///< Voicing: −8va
    QPushButton* m_oktHoeher = nullptr;  ///< Voicing: +8va
    QPushButton* m_play = nullptr;       ///< Voicing: ein Knopf statt der Spielarten
    QButtonGroup* m_spielarten = nullptr;
    QToolButton* m_favorit = nullptr;
    QPushButton* m_use = nullptr;
    QPushButton* m_neu = nullptr;
    QPushButton* m_bearbeiten = nullptr;
    QPushButton* m_duplizieren = nullptr;
    QPushButton* m_loeschen = nullptr;
};

}  // namespace lumi::loop
