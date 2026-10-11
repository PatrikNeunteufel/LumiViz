/**
 * @file Achse.cpp
 * @brief Die gemeinsame Basis der Achsen mit Lineal, Zeilen und Abspielmarke (S109)
 */

#include "UI/widgets/Achse.hpp"

#include <QAction>
#include <QCoreApplication>
#include <QFont>
#include <QFontMetrics>
#include <QGridLayout>
#include <QKeySequence>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QResizeEvent>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>

namespace lumi::ui
{

// =============================================================================
// Achse
// =============================================================================

Achse::Achse(int kopfZeilen, double feinsteJePixel, QWidget* parent)
    : QWidget(parent), m_kopfZeilen(kopfZeilen), m_feinsteJePixel(feinsteJePixel)
{
}

void Achse::setStelle(std::int64_t stelle)
{
    if (stelle == m_stelle) return;
    m_stelle = stelle;
    folgeDerMarke();
    update();
}

void Achse::folgeDerMarke()
{
    if (m_mitlaufen == Mitlaufen::Aus || zeitGanz() || m_tasteUnten || m_balkenGehalten) return;
    const int breite = zeitBreite();
    const int imInhalt = static_cast<int>(std::floor(static_cast<double>(m_stelle) / einheitJePixel() + 0.5));
    if (m_mitlaufen == Mitlaufen::Stetig)
    {
        // die Marke in der Mitte — am Anfang und am Ende laeuft sie selbst
        setRollX(imInhalt - breite / 2);
        return;
    }
    // seitenweise: erst wenn die Marke den Ausschnitt verlaesst, dann steht sie links
    const int imFenster = imInhalt - m_rollX;
    if (imFenster < 0 || imFenster >= breite) setRollX(imInhalt);
}

void Achse::beginneNeu()
{
    m_stelle = 0;
    m_rollY = 0;  // ein anderer Inhalt beginnt oben —
    m_rollX = 0;  // und mit der ganzen Laenge auf der Breite (Entscheid Patrik ZO9: a)
    m_einheitJePixel = 0.0;
}

int Achse::zeitBreite() const
{
    return std::max(1, width() - kKopfBreite - kRand);
}

int Achse::xVon(std::int64_t stelle) const
{
    if (m_laenge <= 0) return kKopfBreite;
    if (zeitGanz())
        return kKopfBreite + static_cast<int>(static_cast<double>(stelle) / static_cast<double>(m_laenge) *
                                                  zeitBreite() + 0.5);
    // hineingezoomt (S102): der feste Massstab, um `rollX` nach links versetzt
    return kKopfBreite + static_cast<int>(std::floor(static_cast<double>(stelle) / einheitJePixel() + 0.5)) -
           m_rollX;
}

std::int64_t Achse::stelleVon(int x) const
{
    if (m_laenge <= 0) return 0;
    if (zeitGanz())
    {
        const double anteil = static_cast<double>(x - kKopfBreite) / static_cast<double>(zeitBreite());
        return std::clamp<std::int64_t>(
            static_cast<std::int64_t>(anteil * static_cast<double>(m_laenge) + 0.5), 0, m_laenge);
    }
    const double einheiten = static_cast<double>(x - kKopfBreite + m_rollX) * einheitJePixel();
    return std::clamp<std::int64_t>(static_cast<std::int64_t>(std::floor(einheiten + 0.5)), 0, m_laenge);
}

double Achse::einheitJePixelGanz() const
{
    return m_laenge > 0 ? static_cast<double>(m_laenge) / zeitBreite() : 1.0;
}

double Achse::einheitJePixel() const
{
    const double ganz = einheitJePixelGanz();
    return m_einheitJePixel > 0.0 ? std::min(m_einheitJePixel, ganz) : ganz;
}

int Achse::rollXMax() const
{
    if (zeitGanz() || m_laenge <= 0) return 0;
    const int inhalt = static_cast<int>(std::ceil(static_cast<double>(m_laenge) / einheitJePixel()));
    return std::max(0, inhalt - zeitBreite());
}

void Achse::setRollX(int x)
{
    x = std::clamp(x, 0, rollXMax());
    if (x == m_rollX) return;
    m_rollX = x;
    update();
    Q_EMIT rollenGeaendert();
}

void Achse::zoomeZeit(double faktor, int xFest)
{
    if (!istGeladen() || m_laenge <= 0 || faktor <= 0.0) return;
    const double ganz = einheitJePixelGanz();
    const double alt = einheitJePixel();
    // der Festpunkt: die Zeit unter `xFest` — ueber der Spalte der Namen die
    // an deren rechter Kante
    const int x = std::max(xFest, kKopfBreite);
    const double zeit = static_cast<double>(x - kKopfBreite + m_rollX) * alt;
    const double neu = std::max(m_feinsteJePixel, alt / faktor);
    if (neu >= ganz * 0.999)
    {
        // weiter heraus als die ganze Laenge geht es nicht
        m_einheitJePixel = 0.0;
        m_rollX = 0;
    }
    else
    {
        m_einheitJePixel = neu;
        m_rollX = std::clamp(static_cast<int>(std::floor(zeit / neu + 0.5)) - (x - kKopfBreite), 0, rollXMax());
    }
    update();
    Q_EMIT rollenGeaendert();
}

void Achse::setzeZoomZurueck()
{
    zeigeZeitGanz();
    setZeilenProzent(100);
}

QStringList Achse::zoomGesten()
{
    return {tr("Ctrl+Wheel — zoom the time"),
            tr("Alt+Wheel — zoom the row height"),
            tr("Ctrl+Alt+Wheel — zoom both"),
            tr("Wheel — scroll up and down"),
            tr("Shift+Wheel — scroll sideways"),
            tr("Middle mouse button — drag the view")};
}

void Achse::fuelleZoomMenue(QMenu& menu)
{
    QMenu* zoom = menu.addMenu(tr("Zoom"));
    QAction* fit = zoom->addAction(tr("Fit"), this, [this] { setzeZoomZurueck(); });
    fit->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));  // nur als Anzeige — das Kuerzel fuehrt das Panel
    fit->setShortcutVisibleInContextMenu(true);
    fit->setEnabled(zoomVerstellt());
    zoom->addSeparator();
    // die Gesten zum Nachlesen (Entscheid Patrik ZO8: a) — grau, nichts zum Anklicken
    for (const QString& geste : zoomGesten()) zoom->addAction(geste)->setEnabled(false);
}

void Achse::zeigeZeitGanz()
{
    if (zeitGanz()) return;
    m_einheitJePixel = 0.0;
    m_rollX = 0;
    update();
    Q_EMIT rollenGeaendert();
}

int Achse::yVonZeile(int zeile) const
{
    return zeileOben(zeile) + zeileHoch(zeile) / 2;
}

QSize Achse::sizeHint() const
{
    return {640, inhaltHoehe()};
}

QSize Achse::minimumSizeHint() const
{
    // das Lineal und zwei Zeilen: was nicht hineinpasst, rollt (S102)
    return {kKopfBreite + 160, kLinealHoehe + 2 * kZeilenHoehe};
}

int Achse::inhaltHoehe() const
{
    return kLinealHoehe + m_kopfZeilen * kopfHoch() + zeilenZahl() * kanalHoch() + 4;
}

int Achse::zeileBei(int y) const
{
    if (y < kLinealHoehe) return kKeineZeile;
    const int imInhalt = y + m_rollY;
    const int zeilenOben = kLinealHoehe + m_kopfZeilen * kopfHoch();
    if (imInhalt < zeilenOben)
        return std::clamp((imInhalt - kLinealHoehe) / kopfHoch(), 0, m_kopfZeilen - 1) - m_kopfZeilen;
    return (imInhalt - zeilenOben) / kanalHoch();
}

bool Achse::namePasst(int hoch) const
{
    return hoch >= fontMetrics().ascent();  // die Unterlaengen duerfen ueberstehen
}

void Achse::setZeilenProzent(int prozent, int yFest)
{
    prozent = std::clamp(prozent, kHoeheMin, kHoeheMax);
    if (prozent == m_zeilenProzent) return;
    // der Festpunkt: welche Zeile unter `yFest` liegt, und wie weit in ihr
    const int y = std::max(yFest, kLinealHoehe);
    const int zeile = zeileBei(y);
    const double anteil = static_cast<double>(y - zeileOben(zeile)) / zeileHoch(zeile);
    m_zeilenProzent = prozent;
    m_rollY = 0;  // `zeileOben` zaehlt jetzt vom Anfang des Inhalts
    m_rollY = std::clamp(zeileOben(zeile) + static_cast<int>(anteil * zeileHoch(zeile) + 0.5) - y, 0,
                         rollYMax());
    updateGeometry();
    update();
    Q_EMIT rollenGeaendert();
    Q_EMIT zeilenProzentGeaendert(m_zeilenProzent);
}

int Achse::rollYMax() const
{
    return std::max(0, inhaltHoehe() - height());
}

void Achse::setRollY(int y)
{
    y = std::clamp(y, 0, rollYMax());
    if (y == m_rollY) return;
    m_rollY = y;
    update();
    Q_EMIT rollenGeaendert();
}

void Achse::passeRollenAn()
{
    m_rollY = std::clamp(m_rollY, 0, rollYMax());
    // passt die ganze Laenge beim festen Massstab wieder auf die Breite (das
    // Fenster ist breiter, der Inhalt kuerzer geworden), ist sie „ganz"
    if (m_einheitJePixel > 0.0 && m_einheitJePixel >= einheitJePixelGanz() * 0.999) m_einheitJePixel = 0.0;
    m_rollX = std::clamp(m_rollX, 0, rollXMax());
    Q_EMIT rollenGeaendert();
}

void Achse::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    passeRollenAn();
}

void Achse::wheelEvent(QWheelEvent* event)
{
    // Entscheid Patrik ZO1: a (S102) — ohne Taste senkrecht rollen, Shift+Rad
    // waagrecht, Ctrl+Rad die Zeit zoomen, Alt+Rad die Zeilenhoehe,
    // Ctrl+Alt+Rad beides.
    const QPoint grad = event->angleDelta();
    const Qt::KeyboardModifiers tasten =
        event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier | Qt::AltModifier);
    // mit einer Taste kann das System das Rad als waagrecht melden: es zaehlt die Achse, die sich bewegt
    const int rad = grad.y() != 0 ? grad.y() : grad.x();

    const bool zeit = tasten == Qt::ControlModifier || tasten == (Qt::ControlModifier | Qt::AltModifier);
    const bool hoehe = tasten == Qt::AltModifier || tasten == (Qt::ControlModifier | Qt::AltModifier);
    if (zeit || hoehe)
    {
        if (!istGeladen() || rad == 0)
        {
            event->ignore();
            return;
        }
        if (zeit) zoomeZeit(std::pow(kZoomSchritt, rad / 120.0), static_cast<int>(event->position().x()));
        if (hoehe)
        {
            // die Hoehe geht in ganzen Schritten: ein feines Rad sammelt, bis es eine Raste ist
            m_radRest += rad;
            const int rasten = m_radRest / 120;
            m_radRest -= rasten * 120;
            if (rasten != 0)
                setZeilenProzent(m_zeilenProzent + rasten * kHoeheSchritt,
                                 static_cast<int>(event->position().y()));
            // Alt allein losgelassen schaltet unter Windows die Menueleiste auf
            // die Tastatur. Qt laesst das, sobald sich zwischen Druck und
            // Loslassen die Maus bewegt hat — das Rad zaehlt dabei nicht. Eine
            // Mausbewegung an ein Objekt ohne Wirkung holt es nach.
            QObject niemand;
            QMouseEvent bewegung(QEvent::MouseMove, event->position(), event->globalPosition(), Qt::NoButton,
                                 event->buttons(), event->modifiers());
            QCoreApplication::sendEvent(&niemand, &bewegung);
        }
        event->accept();  // auch am Anschlag: das Rad soll dann nichts anderes rollen
        return;
    }
    // 60 Pixel je Raste — etwa so weit wie die rollende Flaeche, in der die Achse bis S101 lag
    const bool quer = tasten == Qt::ShiftModifier ||
                      (tasten == Qt::NoModifier && std::abs(grad.x()) > std::abs(grad.y()));
    if (quer)
    {
        const int weg = tasten == Qt::ShiftModifier ? rad : grad.x();
        const int vorher = m_rollX;
        setRollX(m_rollX - weg * 60 / 120);
        event->setAccepted(m_rollX != vorher);
        return;
    }
    if (tasten != Qt::NoModifier || rollYMax() <= 0 || grad.y() == 0)
    {
        event->ignore();
        return;
    }
    const int vorher = m_rollY;
    setRollY(m_rollY - grad.y() * 60 / 120);
    event->setAccepted(m_rollY != vorher);
}

void Achse::beginneSchwenken(QPoint ort)
{
    m_schwenkt = true;
    m_schwenkStart = ort;
    m_schwenkRoll = QPoint(m_rollX, m_rollY);
    setCursor(Qt::ClosedHandCursor);
}

void Achse::schwenkeZu(QPoint ort)
{
    // der Inhalt folgt der Maus: nach rechts gezogen rollt er zurueck
    setRollX(m_schwenkRoll.x() - (ort.x() - m_schwenkStart.x()));
    setRollY(m_schwenkRoll.y() - (ort.y() - m_schwenkStart.y()));
}

void Achse::beendeSchwenken()
{
    m_schwenkt = false;
    unsetCursor();
}

Achse::Flaechen Achse::flaechen() const
{
    Flaechen f;
    f.lineal = QRect(0, 0, width(), kLinealHoehe);
    f.zeilen = QRect(0, kLinealHoehe, width(), std::max(0, height() - kLinealHoehe));
    f.linealZeit = f.lineal.adjusted(kKopfBreite - 1, 0, 0, 0);
    f.zeilenZeit = f.zeilen.adjusted(kKopfBreite - 1, 0, 0, 0);
    return f;
}

void Achse::zeichneLinealStriche(QPainter& p, std::int64_t schritt, int unten,
                                 const std::function<QString(std::int64_t)>& beschriftung) const
{
    const QColor text = palette().color(QPalette::Text);
    QColor linie = text;
    linie.setAlpha(50);
    // die Striche nur im sichtbaren Stueck Zeit: hineingezoomt waeren es sonst Tausende
    const std::int64_t von = stelleVon(kKopfBreite) / schritt * schritt;
    const std::int64_t bis = std::min(m_laenge, stelleVon(width()) + schritt);
    QFont klein = font();
    klein.setPointSizeF(std::max(6.5, klein.pointSizeF() - 1.0));
    p.setFont(klein);
    for (std::int64_t stelle = von; stelle <= bis; stelle += schritt)
    {
        const int x = xVon(stelle);
        p.setPen(linie);
        p.drawLine(x, kLinealHoehe - 6, x, unten);
        p.setPen(text);
        const QString marke = beschriftung(stelle);
        // am rechten Rand steht die Beschriftung links vom Strich, sonst waere sie abgeschnitten
        if (x + 6 + p.fontMetrics().horizontalAdvance(marke) > width())
            p.drawText(QRect(x - 83, 0, 80, kLinealHoehe - 4), Qt::AlignRight | Qt::AlignVCenter, marke);
        else
            p.drawText(QRect(x + 3, 0, 80, kLinealHoehe - 4), Qt::AlignLeft | Qt::AlignVCenter, marke);
    }
    p.setFont(font());
}

// =============================================================================
// AchsenRahmen
// =============================================================================

AchsenRahmen::AchsenRahmen(Achse* achse, QWidget* parent) : QWidget(parent), m_achse(achse)
{
    m_senkrecht = new QScrollBar(Qt::Vertical, this);
    m_senkrecht->setSingleStep(20);
    m_waagrecht = new QScrollBar(Qt::Horizontal, this);
    m_waagrecht->setSingleStep(20);
    auto* gitter = new QGridLayout(this);
    gitter->setContentsMargins(0, 0, 0, 0);
    gitter->setSpacing(0);
    gitter->addWidget(achse, 0, 0);
    gitter->addWidget(m_senkrecht, 0, 1);
    gitter->addWidget(m_waagrecht, 1, 0);
    gitter->setRowStretch(0, 1);
    gitter->setColumnStretch(0, 1);
    connect(achse, &Achse::rollenGeaendert, this, &AchsenRahmen::gleicheAb);
    connect(m_senkrecht, &QScrollBar::valueChanged, achse, &Achse::setRollY);
    connect(m_waagrecht, &QScrollBar::valueChanged, achse, &Achse::setRollX);
    // ein gezogener Balken haelt das Mitlaufen an — sonst zoege die Abspielmarke ihn zurueck
    for (QScrollBar* balken : {m_senkrecht, m_waagrecht})
    {
        connect(balken, &QScrollBar::sliderPressed, achse, [achse] { achse->halteMitlaufen(true); });
        connect(balken, &QScrollBar::sliderReleased, achse, [achse] { achse->halteMitlaufen(false); });
    }
    gleicheAb();
}

void AchsenRahmen::gleicheAb()
{
    const int max = m_achse->rollYMax();
    {
        const QSignalBlocker ruhe(m_senkrecht);
        m_senkrecht->setRange(0, max);
        // eine Seite: was unter dem Lineal zu sehen ist
        m_senkrecht->setPageStep(std::max(1, m_achse->height() - Achse::kLinealHoehe));
        m_senkrecht->setValue(m_achse->rollY());
    }
    m_senkrecht->setVisible(max > 0);  // nur, wenn nicht alle Zeilen ins Fenster passen

    const int maxX = m_achse->rollXMax();
    {
        const QSignalBlocker ruhe(m_waagrecht);
        m_waagrecht->setRange(0, maxX);
        // eine Seite: die Zeit, die rechts der Namensspalte zu sehen ist
        m_waagrecht->setPageStep(std::max(1, m_achse->width() - Achse::kKopfBreite));
        m_waagrecht->setValue(m_achse->rollX());
    }
    m_waagrecht->setVisible(maxX > 0);  // nur, wenn in die Zeit hineingezoomt ist
}

}  // namespace lumi::ui
