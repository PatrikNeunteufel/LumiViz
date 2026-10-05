/**
 * @file KurvenEditor.hpp
 * @brief Composer: das Fenster Curve Editor — die Form einer Blende oder eines
 *        Stuecks zeichnen, aus Presets laden, als Preset speichern
 *        (docs/ui/Composer_Konzept.md §9, Stufe C4 Teil 2)
 *
 * **Formflaeche:** die Form in Anteilen (x 0…1 ueber die Weite, y = Pegel).
 * Punkte ziehen, Doppelklick setzt einen neuen Punkt oder nimmt einen weg,
 * Rechtsklick waehlt die Form des Abschnitts; ein Bézier-Abschnitt zeigt zwei
 * Anfasser. Die Flaeche kennt keine Zeit.
 *
 * **KurvenFenster:** ein eigenes, nicht modales Fenster (Entscheid Patrik S96,
 * Rueckfrage 2: a). Oben die Preset-Zeile, in der Mitte die Formflaeche, unten
 * die Parameter des Teils am Clip — Dauer bzw. Lage, Weite und „Flipped" —
 * und der Grundpegel des Clips. Das Fenster aendert nichts selbst: es meldet
 * den geaenderten Clip, das Composer-Panel fuehrt ihn ueber Undo aus und zeigt
 * dem Fenster danach den neuen Stand.
 *
 * Rueckbau (Konzept §12): Teil von `include/composer` / `src/composer`. Keine
 * Naht — das Fenster gehoert dem Composer-Panel.
 */
#pragma once

#include "composer/Bearbeiten.hpp"
#include "composer/Komposition.hpp"
#include "composer/Pegelform.hpp"

#include <QPoint>
#include <QString>
#include <QWidget>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QPushButton;

namespace lumi::composer
{

class Formflaeche : public QWidget
{
    Q_OBJECT

public:
    explicit Formflaeche(QWidget* parent = nullptr);

    /// Die Form, die gezeigt wird; eine laufende Bewegung der Maus endet.
    void setForm(const Pegelform& form);
    [[nodiscard]] const Pegelform& form() const { return m_form; }
    /// Die Flaeche zeigt die Wirkung am Clip (Entscheid Patrik S96): `rueckwaerts`
    /// = als Ausblendung, an der y-Achse gespiegelt; `gekippt` = oben und unten
    /// getauscht. Die Form selbst bleibt, wie sie ist — nur die Ansicht wechselt.
    void setAnsicht(bool rueckwaerts, bool gekippt);
    [[nodiscard]] bool rueckwaerts() const { return m_rueckwaerts; }
    [[nodiscard]] bool gekippt() const { return m_gekippt; }
    /// Aus: nur ansehen.
    void setBearbeitbar(bool an);
    [[nodiscard]] bool bearbeitbar() const { return m_bearbeitbar; }

    // --- Schritte (auch fuer Tests); jeder meldet `geaendert` ----------------
    /// Einen Punkt setzen; er uebernimmt die Form des Abschnitts, in den er faellt.
    void fuegePunktEin(double x, double y);
    /// Einen Punkt wegnehmen — nicht den ersten, nicht den letzten.
    void loeschePunkt(int punkt);
    /// Die Form des Abschnitts, der bei `punkt` beginnt.
    void setzeAbschnittForm(int punkt, Kurvenform form);

    // --- Geometrie (auch fuer Tests) ----------------------------------------
    /// Wo der Punkt (x, y) DER FORM auf der Flaeche liegt — in der Ansicht.
    [[nodiscard]] QPoint ortVon(double x, double y) const;
    [[nodiscard]] QPoint punktOrt(int punkt) const;
    /// Einer der zwei Bézier-Anfasser (`nr` 0 oder 1) des Abschnitts, der bei
    /// `punkt` beginnt; (-1,-1) = der Abschnitt ist keine Bézier-Kurve.
    [[nodiscard]] QPoint anfasserOrt(int punkt, int nr) const;
    /// Der Punkt unter `ort`, -1 = keiner.
    [[nodiscard]] int punktBei(QPoint ort) const;
    [[nodiscard]] int gewaehlterPunkt() const { return m_gewaehlt; }

Q_SIGNALS:
    /// Ein Schritt ist fertig (Maus losgelassen, Punkt gesetzt, Form gewaehlt).
    void geaendert(const lumi::composer::Pegelform& form, const QString& was);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    [[nodiscard]] QSize sizeHint() const override { return {380, 240}; }
    [[nodiscard]] QSize minimumSizeHint() const override { return {200, 140}; }

private:
    [[nodiscard]] QRect feld() const;
    [[nodiscard]] double xVon(int px) const;
    [[nodiscard]] double yVon(int py) const;
    /// Der Abschnitt, in den `x` faellt: der Punkt, bei dem er beginnt.
    [[nodiscard]] int abschnittBei(double x) const;
    /// Darf der Punkt in dieser Richtung bewegt werden?
    [[nodiscard]] bool festX(int punkt) const;
    [[nodiscard]] bool festY(int punkt) const;

    Pegelform m_form;
    bool m_bearbeitbar = true;
    bool m_rueckwaerts = false;
    bool m_gekippt = false;
    int m_gewaehlt = -1;

    enum class Zug
    {
        Keiner,
        Punkt,
        Anfasser,
    };
    Zug m_zug = Zug::Keiner;
    int m_zugPunkt = -1;
    int m_zugNr = 0;       ///< welcher Anfasser
    bool m_bewegt = false;
    Pegelform m_vorZug;
};

class KurvenFenster : public QWidget
{
    Q_OBJECT

public:
    explicit KurvenFenster(QWidget* parent = nullptr);

    /// Das Teil eines Clips zeigen. `clip` nullptr oder ein Teil, das der Clip
    /// nicht hat: das Fenster ist leer und sagt, wie man eine Kurve oeffnet.
    void zeige(const VisualClip* clip, Teil teil, const QString& name, bool bearbeitbar);
    [[nodiscard]] Teil teil() const { return m_teil; }
    [[nodiscard]] bool hatKurve() const { return m_hat; }
    [[nodiscard]] Formflaeche* flaeche() const { return m_flaeche; }

    // --- Presets (auch fuer Tests) -------------------------------------------
    /// Die Form eines Presets dieser Art laden. false = gibt es nicht.
    bool ladePreset(const QString& name);
    /// Die gezeigte Form als eigenes Preset speichern.
    bool speicherePreset(const QString& name);
    /// Das eigene Preset loeschen. false = mitgeliefert oder gibt es nicht.
    bool loeschePreset(const QString& name);

Q_SIGNALS:
    /// Der Clip mit der Aenderung — das Panel fuehrt sie aus.
    void clipGeaendert(const lumi::composer::VisualClip& clip, const QString& was);

private:
    void setupUI();
    /// Die Liste der Presets neu lesen und das waehlen, das der Form gleicht.
    void zeigePresets();
    /// Die Felder unten aus `m_clip`.
    void zeigeWerte();
    [[nodiscard]] Pegelform* form();
    [[nodiscard]] FormArt art() const;
    /// Eine Aenderung an Dauer, Weite oder Lage: erst den sichtbaren Stand
    /// festschreiben (Entscheid Patrik S96), dann `tu`, dann melden.
    template <typename Tu>
    void aendereWert(const QString& was, Tu tu);

    VisualClip m_clip;
    Teil m_teil;
    bool m_hat = false;
    bool m_bearbeitbar = false;

    QLabel* m_titel = nullptr;
    QComboBox* m_presets = nullptr;
    QPushButton* m_speichern = nullptr;
    QPushButton* m_loeschen = nullptr;
    Formflaeche* m_flaeche = nullptr;
    QLabel* m_hinweis = nullptr;
    QLabel* m_ansicht = nullptr;        ///< sagt, wenn die Flaeche gespiegelt zeigt
    QLabel* m_dauerTitel = nullptr;
    QDoubleSpinBox* m_dauer = nullptr;  ///< Dauer der Blende bzw. Weite des Stuecks
    QLabel* m_jetzt = nullptr;          ///< „now 2.1 s", wenn das Teil gestaucht ist
    QLabel* m_lageTitel = nullptr;
    QDoubleSpinBox* m_lage = nullptr;
    QCheckBox* m_gekippt = nullptr;
    QDoubleSpinBox* m_grund = nullptr;
};

}  // namespace lumi::composer
