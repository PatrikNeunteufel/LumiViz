/**
 * @file GriffbildBild.cpp
 * @brief Loop: ein Griffbild zeichnen (Stufe 26e, S138)
 */

#include "loop/GriffbildBild.hpp"

#include "loop/Instrument.hpp"  // tonName

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

// --- Voicings: eine Klaviatur ---------------------------------------------------

namespace
{

/// Die Stelle einer weissen Taste in der Oktave (C = 0 … B = 6); −1 = eine schwarze.
int weissPlatz(int klasse)
{
    static constexpr int kPlatz[12] = {0, -1, 1, -1, 2, 3, -1, 4, -1, 5, -1, 6};
    return kPlatz[((klasse % 12) + 12) % 12];
}

/// Wo die Klaviatur in einem Bild liegt: in der Mitte, so hoch wie es ihr steht.
QRectF klaviaturFeld(const QRectF& aussen, GriffbildGroesse groesse)
{
    const double rand = groesse == GriffbildGroesse::Klein ? 2.0 : 6.0;
    const double breite = aussen.width() - 2 * rand;
    const double hoehe = std::min(aussen.height() - 2 * rand, breite * (groesse == GriffbildGroesse::Klein ? 0.5 : 0.42));
    return {aussen.left() + rand, aussen.center().y() - hoehe / 2.0, breite, hoehe};
}

}  // namespace

KlaviaturLage::KlaviaturLage(const QRectF& feld, int von, int bis) : m_feld(feld), m_von(von), m_bis(std::max(von, bis))
{
    int weisse = 0;
    for (int t = m_von; t <= m_bis; ++t) weisse += schwarz(t) ? 0 : 1;
    m_weiss = feld.width() / std::max(1, weisse);
}

bool KlaviaturLage::schwarz(int ton)
{
    return weissPlatz(ton) < 0;
}

QRectF KlaviaturLage::taste(int ton) const
{
    if (ton < m_von || ton > m_bis) return {};
    // die weissen Tasten vor diesem Ton: eine weisse beginnt dort, eine schwarze sitzt auf der Kante
    int weissVor = 0;
    for (int t = m_von; t < ton; ++t) weissVor += schwarz(t) ? 0 : 1;
    if (!schwarz(ton)) return {m_feld.left() + weissVor * m_weiss, m_feld.top(), m_weiss, m_feld.height()};
    const double breit = m_weiss * 0.6;
    return {m_feld.left() + weissVor * m_weiss - breit / 2.0, m_feld.top(), breit, m_feld.height() * 0.62};
}

QPointF KlaviaturLage::punkt(int ton) const
{
    const QRectF r = taste(ton);
    return {r.center().x(), schwarz(ton) ? r.top() + r.height() * 0.72 : r.top() + r.height() * 0.84};
}

std::optional<int> KlaviaturLage::tonBei(QPointF ort) const
{
    for (int t = m_von; t <= m_bis; ++t)
        if (schwarz(t) && taste(t).contains(ort)) return t;
    for (int t = m_von; t <= m_bis; ++t)
        if (!schwarz(t) && taste(t).contains(ort)) return t;
    return std::nullopt;
}

std::pair<int, int> klaviaturBereich(const std::vector<int>& toene, int oktaven, int ohneToene)
{
    const int tief = toene.empty() ? ohneToene : *std::min_element(toene.begin(), toene.end());
    const int hoch = toene.empty() ? ohneToene : *std::max_element(toene.begin(), toene.end());
    const int von = std::max(0, tief - tief % 12);
    return {von, std::max(von + 12 * std::max(1, oktaven) - 1, hoch - hoch % 12 + 11)};
}

void zeichneKlaviatur(QPainter& p, const QRectF& feld, int von, int bis, const std::vector<int>& toene,
                      GriffbildGroesse groesse, const QPalette& palette)
{
    const KlaviaturLage lage(feld, von, bis);
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    // die Tasten in ihren eigenen Farben, gleich in hellem und dunklem Design — sonst
    // kehrt ein dunkles Design Weiss und Schwarz um (von Claude gesetzt, S138)
    const QColor linie(0x88, 0x87, 0x80);
    const QColor schwarzeTaste(0x2C, 0x2C, 0x2A);
    p.setPen(QPen(linie, 0.8));
    p.setBrush(QColor(0xF7, 0xF6, 0xF2));
    for (int t = von; t <= bis; ++t)
        if (!KlaviaturLage::schwarz(t)) p.drawRect(lage.taste(t));
    p.setPen(Qt::NoPen);
    p.setBrush(schwarzeTaste);
    for (int t = von; t <= bis; ++t)
        if (KlaviaturLage::schwarz(t)) p.drawRect(lage.taste(t));
    // die Punkte; gross mit einem Ring um das C, damit man die Oktave findet
    const double r = std::max(1.5, std::min(lage.taste(von).width() * 0.32, feld.height() * 0.12));
    p.setBrush(palette.color(QPalette::Highlight));
    for (const int t : toene)
        if (t >= von && t <= bis) p.drawEllipse(lage.punkt(t), r, r);
    if (groesse == GriffbildGroesse::Gross)
    {
        QFont schrift = p.font();
        schrift.setPixelSize(std::max(8, static_cast<int>(lage.taste(von).width() * 0.42)));
        p.setFont(schrift);
        p.setPen(palette.color(QPalette::Text));
        for (int t = von; t <= bis; t += 12)
        {
            const QRectF r2 = lage.taste(t);
            p.drawText(QRectF(r2.left(), r2.bottom() + 1, r2.width() * 2, schrift.pixelSize() + 4), Qt::AlignLeft | Qt::AlignTop,
                       tonName(t));
        }
    }
    p.restore();
}

void zeichneAkkordbild(QPainter& p, const QRectF& aussen, const Akkord& akkord, int tiefster, GriffbildGroesse groesse,
                       const QPalette& palette)
{
    if (akkord.istGriffbild())
    {
        zeichneGriffbild(p, aussen, akkord, groesse, palette);
        return;
    }
    std::vector<int> toene;
    if (const auto t = toeneDesVoicings(akkord, tiefster))
        for (const AkkordTon& ton : *t) toene.push_back(ton.hoehe);
    const auto [von, bis] = klaviaturBereich(toene, groesse == GriffbildGroesse::Klein ? 2 : 3, tiefster);
    zeichneKlaviatur(p, klaviaturFeld(aussen, groesse), von, bis, toene, groesse, palette);
}

GriffbildFeld::GriffbildFeld(QWidget* parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("chordDiagram"));
}

void GriffbildFeld::setzeAkkord(const Akkord& akkord, int tiefster)
{
    m_akkord = akkord;
    m_tiefster = tiefster;
    update();
}

std::vector<int> GriffbildFeld::toene() const
{
    std::vector<int> toene;
    if (const auto t = toeneDesVoicings(m_akkord, m_tiefster))
        for (const AkkordTon& ton : *t) toene.push_back(ton.hoehe);
    return toene;
}

void GriffbildFeld::setzeKlaviatur(std::optional<std::pair<int, int>> bereich)
{
    m_klaviatur = bereich;
    update();
}

KlaviaturLage GriffbildFeld::klaviaturLage() const
{
    const auto [von, bis] = m_klaviatur ? *m_klaviatur : klaviaturBereich(toene(), 3, m_tiefster);
    return KlaviaturLage(klaviaturFeld(QRectF(rect()), GriffbildGroesse::Gross), von, bis);
}

void GriffbildFeld::setzeFenster(std::optional<GriffbildFenster> fenster)
{
    m_fenster = fenster;
    update();
}

GriffbildLage GriffbildFeld::lage() const
{
    return griffbildLageIn(QRectF(rect()), m_akkord, GriffbildGroesse::Gross, m_fenster);
}

void GriffbildFeld::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.fillRect(rect(), palette().color(QPalette::Base));
    if (m_akkord.istGriffbild())
    {
        zeichneGriffbild(p, QRectF(rect()), m_akkord, GriffbildGroesse::Gross, palette(), m_fenster);
        return;
    }
    if (m_akkord.abstaende.empty() && !m_klaviatur && m_akkord.titel.isEmpty() && m_akkord.art.isEmpty()) return;  // nichts gewaehlt
    const KlaviaturLage lage = klaviaturLage();
    zeichneKlaviatur(p, klaviaturFeld(QRectF(rect()), GriffbildGroesse::Gross), lage.von(), lage.bis(), toene(),
                     GriffbildGroesse::Gross, palette());
}

}  // namespace lumi::loop
