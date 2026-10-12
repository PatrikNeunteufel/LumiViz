/**
 * @file InstrumentPalette.hpp
 * @brief Loop: das Fenster „Instruments" — ein Instrument suchen, anhoeren und
 *        ins Muster setzen (docs/audio/Loop_Konzept.md §4h, Stufe 14c;
 *        Entscheid P1 a, Skizze gebilligt, Patrik S114)
 *
 * Ersetzt die Dropdown-Liste im Instrument-Editor: neunundvierzig Instrumente
 * in einer flachen Liste liessen sich weder suchen noch nach Genre sichten.
 * Vorbild ist das Palettenfenster der Effektkette (`EffectPaletteWindow`).
 *
 * Das Fenster ist nicht modal und bleibt offen. Oben **Search** und **Genre**,
 * darunter „For pattern: <Muster>". Der Baum: **Favorites**, **Recently used**,
 * die acht Arten mit ihrer Zahl, **My files** (die eigenen aus dem
 * Benutzerordner, `user:<name>`). Unten Titel, Art und Genres, ein Satz
 * Beschreibung, **Preview note**, **Play**, **Favorite** und **Use**.
 *
 *  - ein Klick spielt einen Ton (`tonVerlangt`);
 *  - Doppelklick, Enter oder **Use** setzt das Instrument ins Muster
 *    (`verwenden`) — das Setzen selbst macht der Instrument-Editor.
 *
 * Favoriten und „Recently used" merkt die App (`instrumentPalette/…`), als
 * Namen, wie ein Muster sie nennt — sie gelten auch auf einem anderen Rechner.
 */
#pragma once

#include "loop/Instrument.hpp"

#include <QStringList>
#include <QWidget>

#include <vector>

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QToolButton;
class QTreeWidget;
class QTreeWidgetItem;

namespace lumi::loop
{

class InstrumentPalette : public QWidget
{
    Q_OBJECT

public:
    explicit InstrumentPalette(QWidget* parent = nullptr);

    /// Fuer welches Muster das Fenster gerade waehlt; leer = es ist kein
    /// Instrument-Muster offen, **Use** ist dann aus.
    void setzeZiel(const QString& muster);
    [[nodiscard]] const QString& ziel() const { return m_ziel; }
    /// Die Instrumente neu lesen — die eigenen aendern sich, waehrend das
    /// Programm laeuft (Save im Creator, ein Import).
    void liesNeu();
    /// Oeffnen bzw. nach vorn holen, die Schreibmarke in der Suche.
    void zeigeVorn();

    // --- was die Bedienelemente ausloesen (auch fuer Tests) -------------------
    void setzeSuche(const QString& text);
    /// Leer = „All".
    void setzeGenre(const QString& genre);
    /// Ein Instrument waehlen, ohne es anzuspielen. false = es steht nicht im
    /// Fenster (oder ist gerade weggefiltert).
    bool waehle(const QString& nennung);
    /// Das gewaehlte Instrument, wie ein Muster es nennt; leer = keins.
    [[nodiscard]] QString gewaehlt() const;
    /// Die Tonhoehe, mit der angespielt wird.
    [[nodiscard]] int vorhoerHoehe() const;
    void setzeVorhoerHoehe(int hoehe);
    /// Das gewaehlte anspielen. false = keins gewaehlt.
    bool spieleAn();
    /// Das gewaehlte ins Muster setzen und unter „Recently used" merken.
    /// false = keins gewaehlt oder kein Muster offen.
    bool verwende();
    void setzeFavorit(bool an);
    [[nodiscard]] bool istFavorit(const QString& nennung) const;
    /// Das gewaehlte eigene Instrument loeschen (ohne Rueckfrage — die stellt
    /// das Menue). false = es ist kein eigenes.
    bool loescheGewaehltes();

    /// Was der Baum gerade zeigt, von oben nach unten: je Zeile die Nennung;
    /// die Koepfe als `[Favorites]`, `[Keys]`, … Was der Filter verbirgt, fehlt.
    [[nodiscard]] QStringList zeilen() const;
    /// Die Instrumente unter einem Kopf, wie sie gerade zu sehen sind.
    [[nodiscard]] QStringList unter(const QString& kopf) const;
    [[nodiscard]] QStringList favoriten() const { return m_favoriten; }
    [[nodiscard]] QStringList zuletzt() const { return m_zuletzt; }

    /// Die Tonhoehe, mit der ein Instrument dieser Art zuerst angespielt wird:
    /// ein Bass tief (C2), alles andere C4.
    [[nodiscard]] static int hoeheFuerArt(const QString& art);
    /// So viele stehen hoechstens unter „Recently used".
    static constexpr int kZuletztMax = 8;

Q_SIGNALS:
    /// Dieses Instrument ins Muster setzen.
    void verwenden(const QString& nennung);
    /// Einen Ton dieses Instruments anspielen.
    void tonVerlangt(const QString& nennung, int hoehe);

protected:
    void hideEvent(QHideEvent* event) override;
    bool eventFilter(QObject* objekt, QEvent* ereignis) override;

private:
    void baueBaum();
    void filtere();
    void zeigeDetail();
    void zeigeMenue(QPoint ort);
    [[nodiscard]] const Instrument* finde(const QString& nennung) const;
    [[nodiscard]] bool passt(const Instrument& instrument) const;
    QTreeWidgetItem* blatt(QTreeWidgetItem* kopf, const Instrument& instrument);
    void merke() const;

    QString m_ziel;
    std::vector<Instrument> m_alle;  ///< mitgelieferte, dann eigene
    QStringList m_favoriten;         ///< Nennungen
    QStringList m_zuletzt;           ///< juengstes zuerst

    QLineEdit* m_suche = nullptr;
    QComboBox* m_genre = nullptr;
    QLabel* m_zielZeile = nullptr;
    QTreeWidget* m_baum = nullptr;
    QLabel* m_titel = nullptr;
    QLabel* m_satz = nullptr;
    QComboBox* m_hoehe = nullptr;
    QPushButton* m_play = nullptr;
    QToolButton* m_favorit = nullptr;
    QPushButton* m_use = nullptr;
};

}  // namespace lumi::loop
