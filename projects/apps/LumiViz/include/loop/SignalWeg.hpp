/**
 * @file SignalWeg.hpp
 * @brief Loop: die Zeile „Signal path" — der Weg des Tons als Reihe von
 *        Kaesten (docs/audio/Loop_Konzept.md §4j, Stufe 16c; Skizze gebilligt,
 *        Patrik S118, Entscheid S1 a)
 *
 * Von links nach rechts Kaesten, mit Pfeilen verbunden: vorn und hinten je ein
 * fester (im Creator Source und Level), dazwischen die Bausteine, vor dem
 * letzten der Knopf **+ Add**. Jeder Kasten traegt einen Namen und eine Zeile
 * Kurzwert. Ein Klick waehlt einen Kasten, Ziehen ordnet die Bausteine um.
 * Passt die Reihe nicht in die Breite, bricht sie in eine weitere Zeile um.
 *
 * Das Widget kennt weder Schicht noch Clip: wer es benutzt, nennt ihm die
 * Kaesten und hoert auf seine Signale — der Creator fuer die Schicht, das
 * Fenster Clip effects fuer den Clip (Stufe 16d).
 */
#pragma once

#include <QFont>
#include <QPoint>
#include <QRect>
#include <QString>
#include <QWidget>

#include <vector>

namespace lumi::loop
{

/// Ein Kasten der Zeile.
struct WegKasten
{
    QString titel;             ///< „Filter"
    QString wert;              ///< die Zeile darunter: „High-pass ×2.3"
    bool gestrichelt = false;  ///< gestrichelt umrandet: es gibt ihn genau einmal (Envelope)
    bool matt = false;         ///< grau geschrieben: er tut gerade nichts (ausgeschaltet, uebersprungen)
    QString hilfe;             ///< Tooltip; leer = der allgemeine

    [[nodiscard]] bool operator==(const WegKasten&) const = default;
};

class SignalWeg : public QWidget
{
    Q_OBJECT

public:
    explicit SignalWeg(QWidget* parent = nullptr);

    /// Die Kaesten setzen: der erste und der letzte sind fest (grau hinterlegt,
    /// lassen sich nicht ziehen), die dazwischen sind die Bausteine. `gewaehlt`
    /// ist der Platz des gewaehlten Kastens (-1 = keiner). `addText` steht auf
    /// dem Knopf vor dem letzten Kasten; `addMoeglich` = false zeigt ihn grau.
    void setKaesten(std::vector<WegKasten> kaesten, int gewaehlt, bool addMoeglich, const QString& addText);
    [[nodiscard]] const std::vector<WegKasten>& kaesten() const { return m_kaesten; }
    /// Nur den Kurzwert eines Kastens neu setzen — fuer einen Wert, der sich laufend aendert
    /// (die Spitze im Kasten Out), ohne Auswahl und Zug anzufassen.
    void setKastenWert(int platz, const QString& wert);
    [[nodiscard]] int gewaehlt() const { return m_gewaehlt; }
    [[nodiscard]] bool addMoeglich() const { return m_addMoeglich; }
    [[nodiscard]] const QString& addText() const { return m_addText; }

    /// Wo ein Kasten steht, und wo der Knopf Add.
    [[nodiscard]] QRect kastenRect(int platz) const;
    [[nodiscard]] QRect addRect() const;
    /// Der Kasten unter einem Punkt; -1 = keiner.
    [[nodiscard]] int kastenBei(QPoint ort) const;
    /// Laesst sich dieser Kasten ziehen? Der erste und der letzte nicht.
    [[nodiscard]] bool ziehbar(int platz) const;
    /// So viele Zeilen braucht die Reihe in der heutigen Breite.
    [[nodiscard]] int zeilen() const { return m_zeilen; }

    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSizeHint() const override;

Q_SIGNALS:
    /// Ein Kasten wurde angeklickt.
    void gewaehltGeaendert(int platz);
    /// Ein Baustein wurde gezogen: er soll von `von` an den Platz `nach`
    /// (beides Plaetze unter den Kaesten, die festen mitgezaehlt).
    void verschoben(int von, int nach);
    /// Der Knopf Add wurde gedrueckt; `unten` ist die Stelle fuer ein Menue.
    void addVerlangt(QPoint unten);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    bool event(QEvent* event) override;

private:
    /// Kaesten und Knopf in die Breite legen; die Hoehe folgt der Zahl der Zeilen.
    void lege();
    /// Der Platz, an den ein gezogener Baustein unter diesem Punkt kaeme.
    [[nodiscard]] int zielBei(QPoint ort) const;
    void zeigeZeiger(QPoint ort);
    [[nodiscard]] QFont titelSchrift() const;  ///< fett: der Name eines Kastens
    [[nodiscard]] QFont wertSchrift() const;   ///< etwas kleiner: sein Kurzwert

    std::vector<WegKasten> m_kaesten;
    std::vector<QRect> m_rects;  ///< je Kasten
    QRect m_add;
    int m_gewaehlt = -1;
    bool m_addMoeglich = true;
    QString m_addText;
    int m_zeilen = 1;

    int m_gedrueckt = -1;  ///< der Kasten unter der gedrueckten Maus
    QPoint m_druckOrt;
    bool m_zieht = false;
    int m_ziel = -1;       ///< waehrend des Ziehens: hierhin kaeme er
};

}  // namespace lumi::loop
