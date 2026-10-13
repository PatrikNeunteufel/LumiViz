/**
 * @file SignalWeg.cpp
 * @brief Loop: die Zeile „Signal path"
 */

#include "loop/SignalWeg.hpp"

#include <QApplication>
#include <QHelpEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>

#include <algorithm>
#include <cstdlib>
#include <limits>
#include <utility>

namespace lumi::loop
{

namespace
{

constexpr int kKastenBreite = 64;  ///< mindestens; sonst so breit wie sein Text
constexpr int kKastenHoehe = 38;
constexpr int kKastenInnen = 7;    ///< Luft links und rechts vom Text
constexpr int kAddBreite = 56;
constexpr int kPfeil = 14;         ///< der Platz zwischen zwei Kaesten: dort steht der Pfeil
constexpr int kZeilenAbstand = 6;
constexpr int kRand = 1;           ///< Luft fuer den kraeftigen Rand des gewaehlten Kastens

}  // namespace

SignalWeg::SignalWeg(QWidget* parent) : QWidget(parent)
{
    setMouseTracking(true);  // der Zeiger sagt, was sich greifen laesst
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void SignalWeg::setKaesten(std::vector<WegKasten> kaesten, int gewaehlt, bool addMoeglich, const QString& addText)
{
    m_kaesten = std::move(kaesten);
    m_gewaehlt = gewaehlt >= 0 && gewaehlt < static_cast<int>(m_kaesten.size()) ? gewaehlt : -1;
    m_addMoeglich = addMoeglich;
    m_addText = addText;
    lege();
    update();
}

void SignalWeg::lege()
{
    m_rects.assign(m_kaesten.size(), QRect());
    m_add = QRect();
    const int breite = std::max(width(), kKastenBreite + 2 * kRand);
    int x = kRand;
    int zeile = 0;
    const auto setze = [&](int b) {
        if (x > kRand && x + b > breite - kRand)
        {
            x = kRand;
            ++zeile;
        }
        const QRect r(x, kRand + zeile * (kKastenHoehe + kZeilenAbstand), b, kKastenHoehe);
        x += b + kPfeil;
        return r;
    };
    // jeder Kasten ist so breit wie sein Text: ein abgeschnittener Kurzwert sagt nichts
    // (im Bild aus dem Test gesehen: „High-pass …“)
    const QFontMetrics fett(titelSchrift());
    const QFontMetrics klein(wertSchrift());
    for (std::size_t i = 0; i < m_kaesten.size(); ++i)
    {
        // der Knopf Add steht vor dem letzten Kasten
        if (i + 1 == m_kaesten.size() && m_kaesten.size() >= 2)
            m_add = setze(std::max(kAddBreite, klein.horizontalAdvance(m_addText) + 2 * kKastenInnen));
        const int text = std::max(fett.horizontalAdvance(m_kaesten[i].titel), klein.horizontalAdvance(m_kaesten[i].wert));
        m_rects[i] = setze(std::max(kKastenBreite, text + 2 * kKastenInnen + 2));
    }
    const int zeilen = zeile + 1;
    if (zeilen != m_zeilen)
    {
        m_zeilen = zeilen;
        updateGeometry();
    }
}

QFont SignalWeg::titelSchrift() const
{
    QFont fett = font();
    fett.setBold(true);
    return fett;
}

QFont SignalWeg::wertSchrift() const
{
    QFont klein = font();
    if (klein.pointSizeF() > 0.0) klein.setPointSizeF(std::max(6.5, klein.pointSizeF() - 1.0));
    return klein;
}

QSize SignalWeg::sizeHint() const
{
    return {4 * (kKastenBreite + kPfeil), 2 * kRand + m_zeilen * kKastenHoehe + (m_zeilen - 1) * kZeilenAbstand};
}

QSize SignalWeg::minimumSizeHint() const
{
    return {kKastenBreite + 2 * kRand, sizeHint().height()};
}

QRect SignalWeg::kastenRect(int platz) const
{
    return platz >= 0 && platz < static_cast<int>(m_rects.size()) ? m_rects[static_cast<std::size_t>(platz)] : QRect();
}

QRect SignalWeg::addRect() const
{
    return m_add;
}

int SignalWeg::kastenBei(QPoint ort) const
{
    for (std::size_t i = 0; i < m_rects.size(); ++i)
        if (m_rects[i].contains(ort)) return static_cast<int>(i);
    return -1;
}

bool SignalWeg::ziehbar(int platz) const
{
    return platz > 0 && platz + 1 < static_cast<int>(m_kaesten.size());
}

int SignalWeg::zielBei(QPoint ort) const
{
    // der Baustein, dessen Mitte dem Punkt am naechsten liegt
    int bester = -1;
    int abstand = std::numeric_limits<int>::max();
    for (int i = 1; i + 1 < static_cast<int>(m_rects.size()); ++i)
    {
        const QPoint mitte = m_rects[static_cast<std::size_t>(i)].center();
        // die Zeile zaehlt mehr als die Spalte: wer in der zweiten Zeile zieht, meint sie
        const int d = std::abs(mitte.x() - ort.x()) + 4 * std::abs(mitte.y() - ort.y());
        if (d < abstand)
        {
            abstand = d;
            bester = i;
        }
    }
    return bester;
}

void SignalWeg::zeigeZeiger(QPoint ort)
{
    if (m_zieht)
    {
        setCursor(Qt::ClosedHandCursor);
        return;
    }
    const int platz = kastenBei(ort);
    if (ziehbar(platz) && static_cast<int>(m_kaesten.size()) > 3)
        setCursor(Qt::OpenHandCursor);  // er laesst sich ziehen
    else if (platz >= 0 || (m_add.contains(ort) && m_addMoeglich))
        setCursor(Qt::PointingHandCursor);
    else
        unsetCursor();
}

void SignalWeg::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    lege();
}

void SignalWeg::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) return;
    const QPoint ort = event->position().toPoint();
    if (m_add.contains(ort))
    {
        if (m_addMoeglich) Q_EMIT addVerlangt(m_add.bottomLeft());
        return;
    }
    const int platz = kastenBei(ort);
    if (platz < 0) return;
    m_gedrueckt = platz;
    m_druckOrt = ort;
    if (platz != m_gewaehlt)
    {
        m_gewaehlt = platz;
        update();
        Q_EMIT gewaehltGeaendert(platz);
    }
}

void SignalWeg::mouseMoveEvent(QMouseEvent* event)
{
    const QPoint ort = event->position().toPoint();
    if (m_gedrueckt >= 0 && ziehbar(m_gedrueckt) && !m_zieht &&
        (ort - m_druckOrt).manhattanLength() >= QApplication::startDragDistance())
        m_zieht = true;
    if (m_zieht)
    {
        const int ziel = zielBei(ort);
        if (ziel != m_ziel)
        {
            m_ziel = ziel;
            update();
        }
    }
    zeigeZeiger(ort);
}

void SignalWeg::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) return;
    const int von = m_gedrueckt;
    const int nach = m_zieht ? m_ziel : -1;
    m_gedrueckt = -1;
    m_zieht = false;
    m_ziel = -1;
    update();
    zeigeZeiger(event->position().toPoint());
    if (von >= 0 && nach >= 0 && nach != von) Q_EMIT verschoben(von, nach);
}

void SignalWeg::leaveEvent(QEvent* event)
{
    QWidget::leaveEvent(event);
    if (!m_zieht) unsetCursor();
}

bool SignalWeg::event(QEvent* event)
{
    if (event->type() == QEvent::ToolTip)
    {
        const auto* hilfe = static_cast<QHelpEvent*>(event);
        const int platz = kastenBei(hilfe->pos());
        QString text;
        if (m_add.contains(hilfe->pos()))
            text = m_addMoeglich ? tr("Add a block behind the selected one") : tr("The path is full");
        else if (platz >= 0)
        {
            text = m_kaesten[static_cast<std::size_t>(platz)].hilfe;
            if (text.isEmpty())
                text = ziehbar(platz) ? tr("Click to show its controls — drag to move it along the path")
                                      : tr("Click to show its controls");
        }
        if (text.isEmpty())
            QToolTip::hideText();
        else
            QToolTip::showText(hilfe->globalPos(), text, this);
        return true;
    }
    return QWidget::event(event);
}

void SignalWeg::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    const QPalette farben = palette();
    // der Rand aus der Schriftfarbe: `Mid` war im dunklen Design kaum zu sehen — und mit ihm
    // nicht, dass die Envelope gestrichelt umrandet ist (im Bild aus dem Test gesehen)
    QColor rand = farben.color(QPalette::WindowText);
    rand.setAlpha(120);
    const QColor wahl = farben.color(QPalette::Highlight);
    const QColor schrift = farben.color(QPalette::WindowText);
    const QColor matt = farben.color(QPalette::Disabled, QPalette::WindowText);
    const QFont fett = titelSchrift();
    const QFont klein = wertSchrift();

    // --- die Pfeile: von jedem Stueck zum naechsten in derselben Zeile
    std::vector<QRect> folge;
    for (std::size_t i = 0; i < m_rects.size(); ++i)
    {
        if (i + 1 == m_rects.size() && !m_add.isNull()) folge.push_back(m_add);
        folge.push_back(m_rects[i]);
    }
    p.setPen(QPen(matt, 1.2));
    for (std::size_t i = 0; i + 1 < folge.size(); ++i)
    {
        if (folge[i].top() != folge[i + 1].top()) continue;  // Umbruch: kein Pfeil ueber die Zeile
        const int y = folge[i].center().y();
        const int von = folge[i].right() + 3;
        const int bis = folge[i + 1].left() - 3;
        p.drawLine(von, y, bis, y);
        p.drawLine(bis - 4, y - 3, bis, y);
        p.drawLine(bis - 4, y + 3, bis, y);
    }

    // --- die Kaesten
    for (std::size_t i = 0; i < m_kaesten.size(); ++i)
    {
        const WegKasten& kasten = m_kaesten[i];
        const QRect r = m_rects[i];
        const bool fest = !ziehbar(static_cast<int>(i));
        const bool gewaehlt = static_cast<int>(i) == m_gewaehlt;
        QPen stift(gewaehlt ? wahl : rand, gewaehlt ? 2.0 : kasten.gestrichelt ? 1.4 : 1.0);
        if (kasten.gestrichelt) stift.setStyle(Qt::DashLine);
        p.setPen(stift);
        p.setBrush(fest ? QBrush(Qt::NoBrush) : QBrush(farben.color(QPalette::Base)));  // fest: ohne Flaeche, grau geschrieben
        p.setOpacity(m_zieht && static_cast<int>(i) == m_gedrueckt ? 0.5 : 1.0);
        p.drawRoundedRect(r, 5, 5);
        const QRect innen = r.adjusted(kKastenInnen, 3, -kKastenInnen, -3);
        p.setPen(fest || kasten.matt ? matt : schrift);
        p.setFont(fett);
        p.drawText(QRect(innen.left(), innen.top(), innen.width(), innen.height() / 2), Qt::AlignLeft | Qt::AlignVCenter,
                   p.fontMetrics().elidedText(kasten.titel, Qt::ElideRight, innen.width()));
        p.setPen(fest || kasten.matt ? matt : schrift);
        p.setFont(klein);
        p.drawText(QRect(innen.left(), innen.center().y(), innen.width(), innen.height() / 2), Qt::AlignLeft | Qt::AlignVCenter,
                   p.fontMetrics().elidedText(kasten.wert, Qt::ElideRight, innen.width()));
        p.setOpacity(1.0);
    }

    // --- der Knopf Add
    if (!m_add.isNull())
    {
        p.setPen(QPen(m_addMoeglich ? rand : matt, 1.0, m_addMoeglich ? Qt::SolidLine : Qt::DotLine));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(m_add.adjusted(0, 6, 0, -6), 5, 5);
        p.setPen(m_addMoeglich ? schrift : matt);
        p.setFont(klein);
        p.drawText(m_add, Qt::AlignCenter, m_addText);
    }

    // --- beim Ziehen: ein Strich, wo der Baustein hinkaeme
    if (m_zieht && m_ziel >= 0 && m_ziel != m_gedrueckt)
    {
        const QRect ziel = m_rects[static_cast<std::size_t>(m_ziel)];
        const int x = m_ziel < m_gedrueckt ? ziel.left() - kPfeil / 2 : ziel.right() + kPfeil / 2;
        p.setPen(QPen(wahl, 3.0));
        p.drawLine(x, ziel.top(), x, ziel.bottom());
    }
}

}  // namespace lumi::loop
