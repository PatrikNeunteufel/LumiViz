/**
 * @file Achse.hpp
 * @brief Die gemeinsame Basis der Achsen mit Lineal, Zeilen und Abspielmarke:
 *        Massstab, Zoom, Rollen, Mausrad, Schwenken, Mitlaufen (S109)
 *
 * Herausgezogen aus der `Zeitachse` des Composers (dort gebaut in S102,
 * Entscheide Patrik ZO1 bis ZO9), damit das Loop-Panel dieselbe Bedienung
 * bekommt (Entscheid Patrik S109, W1 b). Die Basis rechnet in einer
 * **neutralen ganzzahligen Einheit** — im Composer die Millisekunde — und kennt
 * weder Komposition noch Loop: was in den Zeilen liegt, zeichnet und bedient
 * die abgeleitete Achse.
 *
 * Aufbau des Fensters: oben das Lineal (bleibt beim Rollen stehen), links die
 * Spalte der Zeilennamen (bleibt beim waagrechten Rollen stehen). Unter dem
 * Lineal koennen **Kopfzeilen** liegen (im Composer die vier Master-Zeilen),
 * niedriger als eine Zeile; sie tragen negative Zeilennummern. Darunter die
 * Zeilen ab 0.
 */
#pragma once

#include <QPoint>
#include <QRect>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QWidget>

#include <cstdint>
#include <functional>

class QMenu;
class QPainter;
class QScrollBar;
class QWheelEvent;

namespace lumi::ui
{

class Achse : public QWidget
{
    Q_OBJECT

public:
    /// `kopfZeilen`: so viele Kopfzeilen liegen unter dem Lineal.
    /// `feinsteJePixel`: der feinste Massstab, in Einheiten je Pixel.
    explicit Achse(int kopfZeilen, double feinsteJePixel, QWidget* parent = nullptr);

    /// Die Abspielmarke setzen; mit Mitlaufen folgt ihr der Ausschnitt.
    void setStelle(std::int64_t stelle);
    /// Die Stelle, an der die Abspielmarke gerade gezeichnet ist.
    [[nodiscard]] std::int64_t stelle() const { return m_stelle; }

    // --- Geometrie (auch fuer Tests) ----------------------------------------
    [[nodiscard]] int xVon(std::int64_t stelle) const;
    [[nodiscard]] std::int64_t stelleVon(int x) const;
    /// Mitte der Zeile (0 = die oberste Zeile unter den Kopfzeilen).
    [[nodiscard]] int yVonZeile(int zeile) const;

    static constexpr int kKopfBreite = 96;   ///< Spalte mit den Zeilennamen — die Vorgabe von `kopfBreite()`
    static constexpr int kRand = 10;         ///< frei rechts neben dem Ende
    static constexpr int kLinealHoehe = 24;
    static constexpr int kZeilenHoehe = 36;

    /// Die Breite der Spalte mit den Zeilennamen. Eine Ableitung kann sie breiter
    /// machen (das Loop-Panel traegt dort den Fader, Stufe 27e-2); der Composer bleibt bei `kKopfBreite`.
    [[nodiscard]] virtual int kopfBreite() const { return kKopfBreite; }

    [[nodiscard]] int kopfZeilen() const { return m_kopfZeilen; }
    /// Oberkante und Hoehe einer Zeile; negativ = eine Kopfzeile unter dem
    /// Lineal (die oberste traegt `-kopfZeilen()`). Im Fenster, also um `rollY`
    /// nach oben versetzt: eine Zeile kann unter dem Lineal oder unter dem
    /// Rand liegen. Die Konstanten oben gelten fuer die nicht gerollte Achse
    /// in Normalhoehe (100 %).
    [[nodiscard]] int zeileOben(int zeile) const
    {
        return (zeile < 0 ? kLinealHoehe + (zeile + m_kopfZeilen) * kopfHoch()
                          : kLinealHoehe + m_kopfZeilen * kopfHoch() + zeile * kanalHoch()) -
               m_rollY;
    }
    [[nodiscard]] int zeileHoch(int zeile) const { return zeile < 0 ? kopfHoch() : kanalHoch(); }

    // --- Hoehen-Zoom (S102, Entscheide Patrik ZO1: a, ZO2: a, ZO3: a, ZO9: a) --
    //
    // Die Hoehe der Zeilen in Prozent der Normalhoehe, 50 bis 300. Die
    // Kopfzeilen sind immer 60 % so hoch wie eine Zeile (Urteil Patrik,
    // Sichttest S100 M17). Ein Name, der nicht mehr in seine Zeile passt, wird
    // nicht gezeichnet.
    static constexpr int kHoeheMin = 50;
    static constexpr int kHoeheMax = 300;
    static constexpr int kHoeheSchritt = 10;  ///< je Raste des Mausrads
    [[nodiscard]] int zeilenProzent() const { return m_zeilenProzent; }
    /// Die Hoehe setzen (auf 50…300 begrenzt). Die Stelle des Inhalts unter
    /// `yFest` bleibt im Fenster stehen (ZO5: der Mauszeiger); -1 = die
    /// Oberkante unter dem Lineal.
    void setZeilenProzent(int prozent, int yFest = -1);
    /// Die Hoehe einer Zeile und einer Kopfzeile in Pixeln.
    [[nodiscard]] int kanalHoch() const { return kZeilenHoehe * m_zeilenProzent / 100; }
    [[nodiscard]] int kopfHoch() const { return kanalHoch() * 60 / 100; }

    // --- Zuruecksetzen und Schwenken (S102, Entscheide Patrik ZO7: a, ZO8: a) --
    //
    // Mit gedrueckter mittlerer Maustaste laesst sich der Ausschnitt in beide
    // Richtungen verschieben. „Fit" setzt den Zoom zurueck.

    /// Die ganze Laenge auf der Breite, die Zeilen in Normalhoehe.
    void setzeZoomZurueck();
    /// Ist der Zoom irgendwo verstellt?
    [[nodiscard]] bool zoomVerstellt() const { return !zeitGanz() || m_zeilenProzent != 100; }
    /// Die Gesten der Achse, eine je Zeile — fuer den Tooltip von „Fit".
    [[nodiscard]] static QStringList zoomGesten();
    /// Nach Alt+Rad: verhindert, dass das Loslassen von Alt unter Windows die
    /// Menueleiste auf die Tastatur schaltet (auch fuer die Editoren des Loops, E-Z1 a).
    static void loeseAltVomMenue(const QWheelEvent& event);

    // --- Mitlaufen mit der Abspielmarke (S102, Entscheid Patrik ZO6) ----------
    //
    // Hineingezoomt zeigt die Achse nur einen Ausschnitt; laeuft die
    // Wiedergabe, wandert die Abspielmarke hinaus. Mit Mitlaufen folgt der
    // Ausschnitt ihr: stetig (die Marke bleibt in der Mitte) oder seitenweise
    // (verlaesst die Marke den Ausschnitt, steht sie danach an seinem linken
    // Rand). Solange eine Maustaste in der Achse oder an einem ihrer
    // Rollbalken gedrueckt ist, steht der Ausschnitt.
    enum class Mitlaufen
    {
        Aus,
        Stetig,
        Seitenweise,
    };
    void setMitlaufen(Mitlaufen art) { m_mitlaufen = art; }
    [[nodiscard]] Mitlaufen mitlaufen() const { return m_mitlaufen; }
    /// Ein Rollbalken wird gezogen (oder losgelassen): so lange laeuft nichts mit.
    void halteMitlaufen(bool halten) { m_balkenGehalten = halten; }
    /// Die Regel des Mitlaufens fuer jede rollende Ansicht (S127, auch die
    /// Editoren des Loops): die Marke liegt bei `imInhalt` Pixeln vom Anfang des
    /// Inhalts, sichtbar sind `breite` Pixel ab `rollJetzt`. Liefert, wohin
    /// gerollt werden soll (ungeklemmt; `rollJetzt` = bleiben).
    [[nodiscard]] static int rollFuerMarke(Mitlaufen art, int imInhalt, int breite, int rollJetzt);

    /// Die Zeile unter `y`: negativ = eine Kopfzeile, ab 0 eine Zeile — auch
    /// eine Zahl hinter der letzten (die freie Flaeche darunter).
    /// `kKeineZeile` = das Lineal; es bleibt beim Rollen stehen und deckt die
    /// Zeilen, die darunter geraten.
    static constexpr int kKeineZeile = -1000;
    [[nodiscard]] int zeileBei(int y) const;

    // --- Rollen (S102, Entscheid Patrik ZO4: a) ------------------------------
    //
    // Die Achse fuehrt ihr Rollen selbst: das Lineal bleibt oben stehen, die
    // Zeilen rollen darunter. Den Rollbalken stellt der `AchsenRahmen`; ohne
    // ihn rollt das Mausrad trotzdem.

    /// So viele Pixel ist der Inhalt nach oben gerollt (0 = ganz oben).
    [[nodiscard]] int rollY() const { return m_rollY; }
    /// Der groesste Wert fuer `rollY`: so viel Inhalt passt nicht ins Fenster.
    [[nodiscard]] int rollYMax() const;
    /// Rollen; der Wert wird auf 0…`rollYMax` begrenzt.
    void setRollY(int y);
    /// Lineal, Kopfzeilen und alle Zeilen, nicht gerollt.
    [[nodiscard]] int inhaltHoehe() const;

    // --- Zeit-Zoom (S102, Entscheide Patrik ZO1: a, ZO5: a, ZO9: a) -----------
    //
    // Ohne Zoom zeigt die Achse die ganze Laenge auf ihrer Breite („ganz") —
    // weiter heraus geht es nicht. Hineingezoomt steht der Massstab fest
    // (Einheiten je Pixel), auch wenn der Inhalt waechst oder das Fenster
    // breiter wird; passt dann wieder alles hinein, ist sie wieder „ganz".

    /// Ein Schritt des Mausrads: Faktor 1,25.
    static constexpr double kZoomSchritt = 1.25;
    /// Der Massstab, der gerade gilt.
    [[nodiscard]] double einheitJePixel() const;
    /// Zeigt die Achse die ganze Laenge auf ihrer Breite?
    [[nodiscard]] bool zeitGanz() const { return m_einheitJePixel <= 0.0; }
    /// Die Zeit zoomen: `faktor` ueber 1 = hinein. Die Zeit unter `xFest`
    /// bleibt an ihrer Stelle im Fenster (ZO5: der Mauszeiger).
    void zoomeZeit(double faktor, int xFest);
    /// Zurueck auf die ganze Laenge.
    void zeigeZeitGanz();
    /// So viele Pixel ist die Zeit nach links gerollt (0 = der Anfang steht links).
    [[nodiscard]] int rollX() const { return m_rollX; }
    [[nodiscard]] int rollXMax() const;
    void setRollX(int x);

Q_SIGNALS:
    /// `rollX`, `rollY` oder ihre Grenzen haben sich geaendert (fuer die Rollbalken).
    void rollenGeaendert();
    /// Die Hoehe der Zeilen ist anders (das Panel merkt sie sich).
    void zeilenProzentGeaendert(int prozent);

protected:
    /// Zeigt die Achse etwas? Sonst zoomt das Mausrad nicht.
    [[nodiscard]] virtual bool istGeladen() const = 0;
    /// So viele Zeilen liegen unter den Kopfzeilen (fuer `inhaltHoehe`).
    [[nodiscard]] virtual int zeilenZahl() const = 0;

    /// Die Laenge, die die Achse zeigt, in Einheiten. Die abgeleitete Achse
    /// setzt sie selbst — und laesst sie waehrend eines Zugs stehen: aenderte
    /// sich der Massstab unter der Maus, liefe der Clip ihr davon.
    [[nodiscard]] std::int64_t laenge() const { return m_laenge; }
    void setzeLaenge(std::int64_t laenge) { m_laenge = laenge; }
    /// Die Abspielmarke setzen, ohne dass der Ausschnitt folgt und ohne neu zu zeichnen.
    void setzeStelleStill(std::int64_t stelle) { m_stelle = stelle; }
    /// Ein anderer Inhalt: die Marke am Anfang, nichts gerollt, die ganze
    /// Laenge auf der Breite. Die Hoehe der Zeilen bleibt.
    void beginneNeu();
    /// Nach einer Aenderung von Inhalt oder Groesse: das Rollen in seine
    /// Grenzen holen und `rollenGeaendert` melden.
    void passeRollenAn();

    /// Die Breite, auf der die Zeit liegt: ohne Namensspalte und Rand.
    [[nodiscard]] int zeitBreite() const;
    /// Passt eine Zeile Schrift in diese Hoehe? Sonst bleibt der Name weg.
    [[nodiscard]] bool namePasst(int hoch) const;
    /// Das Untermenue „Zoom" an `menu` haengen.
    void fuelleZoomMenue(QMenu& menu);

    // --- Maus: was die abgeleitete Achse aus ihren Ereignissen meldet --------
    /// Ist irgendeine Maustaste in der Achse gedrueckt? So lange laeuft der
    /// Ausschnitt nicht mit (ZO6).
    void merkeTaste(bool unten) { m_tasteUnten = unten; }
    /// Schwenken mit der mittleren Maustaste: beginnen, der Maus folgen, enden.
    void beginneSchwenken(QPoint ort);
    [[nodiscard]] bool schwenkt() const { return m_schwenkt; }
    void schwenkeZu(QPoint ort);
    void beendeSchwenken();

    // --- Zeichnen -------------------------------------------------------------
    //
    // Das Lineal bleibt beim Rollen stehen, die Zeilen rollen darunter; die
    // Namensspalte bleibt beim waagrechten Rollen stehen. Gezeichnet wird
    // deshalb mit Ausschnitten: was an einer Zeit haengt, nur rechts der
    // Namensspalte (ab ihrer Trennlinie — dort steht die Abspielmarke bei null).
    struct Flaechen
    {
        QRect lineal;      ///< das Lineal, ganze Breite
        QRect zeilen;      ///< alles unter dem Lineal, ganze Breite
        QRect linealZeit;  ///< das Lineal rechts der Namensspalte
        QRect zeilenZeit;  ///< die Zeilen rechts der Namensspalte
    };
    [[nodiscard]] Flaechen flaechen() const;
    /// Die Striche des Lineals mit ihrer Beschriftung, im Abstand `schritt` —
    /// nur im sichtbaren Stueck Zeit. Jeder Strich reicht bis `unten`.
    void zeichneLinealStriche(QPainter& p, std::int64_t schritt, int unten,
                              const std::function<QString(std::int64_t)>& beschriftung) const;

    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSizeHint() const override;

private:
    /// Der Massstab, bei dem die ganze Laenge auf die Breite passt.
    [[nodiscard]] double einheitJePixelGanz() const;
    /// Den Ausschnitt der Abspielmarke nachfuehren, wenn Mitlaufen es verlangt.
    void folgeDerMarke();

    int m_kopfZeilen = 0;
    double m_feinsteJePixel = 1.0;
    std::int64_t m_laenge = 0;
    std::int64_t m_stelle = 0;
    int m_rollY = 0;
    int m_rollX = 0;
    int m_zeilenProzent = 100;
    double m_einheitJePixel = 0.0;  ///< der feste Massstab; 0 = die ganze Laenge auf der Breite
    int m_radRest = 0;  ///< was vom Mausrad noch keine ganze Raste ergab (Hoehen-Zoom)
    Mitlaufen m_mitlaufen = Mitlaufen::Aus;
    bool m_tasteUnten = false;      ///< irgendeine Maustaste ist in der Achse gedrueckt
    bool m_balkenGehalten = false;  ///< ein Rollbalken wird gezogen
    bool m_schwenkt = false;        ///< die mittlere Maustaste ist gedrueckt
    QPoint m_schwenkStart;          ///< die Maus beim Druck
    QPoint m_schwenkRoll;           ///< `rollX` / `rollY` beim Druck
};

/// Eine Achse mit ihren Rollbalken (S102, Entscheid Patrik ZO4: a): der
/// senkrechte steht rechts neben der Achse und erscheint nur, wenn nicht alle
/// Zeilen ins Fenster passen; der waagrechte liegt darunter und erscheint nur,
/// wenn in die Zeit hineingezoomt ist. Balken und Achse halten einander im
/// Gleichstand.
class AchsenRahmen : public QWidget
{
    Q_OBJECT

public:
    explicit AchsenRahmen(Achse* achse, QWidget* parent = nullptr);
    [[nodiscard]] QScrollBar* balkenSenkrecht() const { return m_senkrecht; }
    [[nodiscard]] QScrollBar* balkenWaagrecht() const { return m_waagrecht; }

private:
    void gleicheAb();

    Achse* m_achse = nullptr;
    QScrollBar* m_senkrecht = nullptr;
    QScrollBar* m_waagrecht = nullptr;
};

}  // namespace lumi::ui
