/**
 * @file GriffbildBild.cpp
 * @brief Loop: ein Griffbild zeichnen (Stufe 26e, S138)
 */

#include "loop/GriffbildBild.hpp"

#include <QFont>
#include <QPainter>
#include <QPalette>

#include <algorithm>
#include <cmath>

namespace lumi::loop
{

GriffbildFenster griffbildFenster(const Akkord& akkord)
{
    int tief = kBuendeMax + 1;
    int hoch = 0;
    for (const int bund : akkord.buende)
        if (bund > 0)
        {
            tief = std::min(tief, bund);
            hoch = std::max(hoch, bund);
        }
    if (hoch <= 4) return {1, 4};  // passt an den Sattel (auch: nichts gegriffen)
    return {tief, std::max(4, hoch - tief + 1)};
}

GriffbildLage::GriffbildLage(const QRectF& feld, int saiten, GriffbildFenster fenster)
    : m_brett(feld), m_saiten(std::max(1, saiten)), m_fenster(fenster)
{
    m_fenster.anzahl = std::max(1, m_fenster.anzahl);
    m_fenster.erster = std::max(1, m_fenster.erster);
    m_obenHoehe = std::max(6.0, feld.height() / (m_fenster.anzahl + 1.6) * 0.8);
}

double GriffbildLage::xSaite(int saite) const
{
    if (m_saiten < 2) return m_brett.center().x();
    return m_brett.left() + saite * m_brett.width() / (m_saiten - 1);
}

double GriffbildLage::yLinie(int bund) const
{
    return m_brett.top() + (bund - (m_fenster.erster - 1)) * m_brett.height() / m_fenster.anzahl;
}

QPointF GriffbildLage::punkt(int saite, int bund) const
{
    return {xSaite(saite), (yLinie(bund - 1) + yLinie(bund)) / 2.0};
}

QPointF GriffbildLage::oben(int saite) const
{
    return {xSaite(saite), m_brett.top() - m_obenHoehe / 2.0};
}

std::optional<GriffbildLage::Treffer> GriffbildLage::trifft(QPointF ort) const
{
    const double abstand = m_saiten < 2 ? m_brett.width() + 20.0 : m_brett.width() / (m_saiten - 1);
    const int saite = m_saiten < 2 ? 0 : static_cast<int>(std::lround((ort.x() - m_brett.left()) / abstand));
    if (saite < 0 || saite >= m_saiten || std::abs(ort.x() - xSaite(saite)) > abstand / 2.0) return std::nullopt;
    if (ort.y() < m_brett.top() - m_obenHoehe || ort.y() > m_brett.bottom()) return std::nullopt;
    if (ort.y() < m_brett.top()) return Treffer{saite, 0};
    const double hoehe = m_brett.height() / m_fenster.anzahl;
    const int innen = std::clamp(static_cast<int>(std::floor((ort.y() - m_brett.top()) / hoehe)), 0, m_fenster.anzahl - 1);
    return Treffer{saite, m_fenster.erster + innen};
}

double GriffbildLage::halbmesser() const
{
    const double breit = m_saiten < 2 ? m_brett.width() : m_brett.width() / (m_saiten - 1);
    return std::max(1.5, std::min(breit, m_brett.height() / m_fenster.anzahl) * 0.36);
}

GriffbildLage griffbildLageIn(const QRectF& aussen, const Akkord& akkord, GriffbildGroesse groesse,
                              std::optional<GriffbildFenster> fenster)
{
    const GriffbildFenster f = fenster ? *fenster : griffbildFenster(akkord);
    const int saiten = std::max(1, static_cast<int>(akkord.leer.size()));
    // Raender (von Claude gesetzt): links Platz fuer die Bundzahl, oben fuer × / ○
    const double links = aussen.width() * (groesse == GriffbildGroesse::Klein ? 0.22 : 0.2);
    const double rechts = aussen.width() * 0.08;
    const double oben = aussen.height() * (groesse == GriffbildGroesse::Klein ? 0.16 : 0.12);
    const double unten = aussen.height() * 0.04;
    const QRectF brett(aussen.left() + links, aussen.top() + oben, aussen.width() - links - rechts,
                       aussen.height() - oben - unten);
    return GriffbildLage(brett, saiten, f);
}

void zeichneGriffbild(QPainter& p, const QRectF& aussen, const Akkord& akkord, GriffbildGroesse groesse,
                      const QPalette& palette, std::optional<GriffbildFenster> fenster)
{
    if (!akkord.istGriffbild()) return;
    const GriffbildLage lage = griffbildLageIn(aussen, akkord, groesse, fenster);
    const GriffbildFenster& f = lage.fenster();
    const QRectF& brett = lage.griffbrett();
    const int saiten = static_cast<int>(akkord.leer.size());
    const bool gross = groesse == GriffbildGroesse::Gross;
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    QColor linie = palette.color(QPalette::Text);
    linie.setAlpha(150);
    const QColor punkt = palette.color(QPalette::Highlight);
    const QColor aufPunkt = palette.color(QPalette::HighlightedText);

    // Saiten und Buende; am Sattel die erste Linie dick
    p.setPen(QPen(linie, 1.0));
    for (int s = 0; s < saiten; ++s) p.drawLine(QPointF(lage.xSaite(s), brett.top()), QPointF(lage.xSaite(s), brett.bottom()));
    for (int b = f.erster; b <= f.erster - 1 + f.anzahl; ++b)
        p.drawLine(QPointF(brett.left(), lage.yLinie(b)), QPointF(brett.right(), lage.yLinie(b)));
    if (f.amSattel())
    {
        p.setPen(QPen(palette.color(QPalette::Text), gross ? 4.0 : 2.5, Qt::SolidLine, Qt::FlatCap));
        p.drawLine(QPointF(brett.left(), brett.top()), QPointF(brett.right(), brett.top()));
    }
    else
    {
        p.setPen(QPen(linie, 1.0));
        p.drawLine(QPointF(brett.left(), brett.top()), QPointF(brett.right(), brett.top()));
        QFont schrift = p.font();
        schrift.setPixelSize(std::max(8, static_cast<int>(brett.height() / f.anzahl * (gross ? 0.42 : 0.6))));
        p.setFont(schrift);
        p.setPen(palette.color(QPalette::Text));
        const double y = lage.punkt(0, f.erster).y();
        // links vom Punkt (und vom Balken eines Barres), nicht darunter
        const double rand = brett.left() - lage.halbmesser() - 2.0;
        p.drawText(QRectF(aussen.left(), y - brett.height(), std::max(1.0, rand - aussen.left()), 2 * brett.height()),
                   Qt::AlignRight | Qt::AlignVCenter, QString::number(f.erster));
    }

    // ueber dem Sattel: × nicht gespielt, ○ leer
    const double zeichen = std::max(2.0, lage.halbmesser() * 0.75);
    p.setPen(QPen(palette.color(QPalette::Text), gross ? 1.4 : 1.0));
    p.setBrush(Qt::NoBrush);
    for (int s = 0; s < saiten && s < static_cast<int>(akkord.buende.size()); ++s)
    {
        const QPointF o = lage.oben(s);
        if (akkord.buende[static_cast<std::size_t>(s)] < 0)
        {
            p.drawLine(o + QPointF(-zeichen, -zeichen), o + QPointF(zeichen, zeichen));
            p.drawLine(o + QPointF(-zeichen, zeichen), o + QPointF(zeichen, -zeichen));
        }
        else if (akkord.buende[static_cast<std::size_t>(s)] == 0)
            p.drawEllipse(o, zeichen, zeichen);
    }

    // Barres als Balken von der ersten bis zur letzten Saite auf diesem Bund
    const double r = lage.halbmesser();
    p.setPen(Qt::NoPen);
    p.setBrush(punkt);
    std::vector<std::pair<int, int>> imBalken;   // (Saite, Bund), vom Balken gezeichnet
    std::vector<std::pair<int, int>> balkenAnfang;  // dort steht der Finger des Balkens
    for (const int bund : akkord.barre)
    {
        if (bund < f.erster || bund > f.erster - 1 + f.anzahl) continue;
        int von = -1;
        int bis = -1;
        for (int s = 0; s < static_cast<int>(akkord.buende.size()); ++s)
            if (akkord.buende[static_cast<std::size_t>(s)] == bund)
            {
                if (von < 0) von = s;
                bis = s;
            }
        if (von < 0 || bis == von) continue;
        const double y = lage.punkt(von, bund).y();
        p.drawRoundedRect(QRectF(lage.xSaite(von) - r, y - r, lage.xSaite(bis) - lage.xSaite(von) + 2 * r, 2 * r), r, r);
        balkenAnfang.emplace_back(von, bund);
        for (int s = von; s <= bis; ++s)
            if (akkord.buende[static_cast<std::size_t>(s)] == bund) imBalken.emplace_back(s, bund);
    }

    // die Punkte, gross mit dem Finger
    QFont schrift = p.font();
    schrift.setPixelSize(std::max(8, static_cast<int>(r * 1.3)));
    p.setFont(schrift);
    for (int s = 0; s < static_cast<int>(akkord.buende.size()); ++s)
    {
        const int bund = akkord.buende[static_cast<std::size_t>(s)];
        if (bund <= 0 || bund < f.erster || bund > f.erster - 1 + f.anzahl) continue;
        const bool balken = std::find(imBalken.begin(), imBalken.end(), std::make_pair(s, bund)) != imBalken.end();
        const QPointF mitte = lage.punkt(s, bund);
        if (!balken)
        {
            p.setPen(Qt::NoPen);
            p.setBrush(punkt);
            p.drawEllipse(mitte, r, r);
        }
        const int finger = s < static_cast<int>(akkord.finger.size()) ? akkord.finger[static_cast<std::size_t>(s)] : 0;
        const bool anfang = std::find(balkenAnfang.begin(), balkenAnfang.end(), std::make_pair(s, bund)) != balkenAnfang.end();
        if (gross && finger > 0 && (!balken || anfang))
        {
            p.setPen(aufPunkt);
            p.drawText(QRectF(mitte.x() - r, mitte.y() - r, 2 * r, 2 * r), Qt::AlignCenter, QString::number(finger));
        }
    }
    p.restore();
}

}  // namespace lumi::loop
