/**
 * @file InstrumentEditorSpuren.cpp
 * @brief Loop: die Spuren ueber der Zeit im Raster des Instrument-Editors —
 *        Pedale, Expression, Vibrato, Pitch und die Schalter (Stufe 28d;
 *        docs/audio/Spielweisen_Konzept.md §1a, SK1 a, SK2 a)
 */

#include "loop/InstrumentEditor.hpp"

#include "UI/widgets/Bedienhilfe.hpp"  // zeigeMenue: in Tests abfragbar

#include <QContextMenuEvent>
#include <QHelpEvent>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QToolTip>

#include <algorithm>
#include <cmath>

namespace lumi::loop
{

namespace
{

/// Ohne Pitch-Bereich am Instrument zeigt die Spur so weit (Vorgabe der *Pitch range*).
constexpr double kPitchBereichOhne = 2.0;

}  // namespace

void NotenRaster::setzeGetrageneSpuren(const std::array<bool, kSteuerspuren>& traegt, double pitchBereichHt)
{
    if (traegt == m_traegt && pitchBereichHt == m_pitchBereich) return;
    m_traegt = traegt;
    m_pitchBereich = pitchBereichHt;
    setRollen(m_rollX, m_rollY);  // die Noten koennen mehr oder weniger Hoehe haben
    update();
}

std::vector<NotenRaster::LeistenSpur> NotenRaster::angeboteneSpuren() const
{
    std::vector<LeistenSpur> spuren;
    for (const Steuerspur s : kAlleSteuerspuren)
        if (traegt(s) || !m_muster.zeitspuren.von(s).empty()) spuren.push_back({s, {}});
    for (const SchalterSpur& schalter : m_muster.zeitspuren.schalter) spuren.push_back({Steuerspur::Sustain, schalter.art});
    return spuren;
}

void NotenRaster::setzeSpurSichtbar(const LeistenSpur& spur, bool sichtbar)
{
    if (spurSichtbar(spur) == sichtbar) return;
    if (spur.schalter.isEmpty())
        m_steuerSichtbar[static_cast<std::size_t>(spur.steuer)] = sichtbar;
    else if (sichtbar)
        m_schalterSichtbar.push_back(spur.schalter);
    else
        m_schalterSichtbar.erase(std::remove(m_schalterSichtbar.begin(), m_schalterSichtbar.end(), spur.schalter),
                                 m_schalterSichtbar.end());
    setRollen(m_rollX, m_rollY);
    update();
}

bool NotenRaster::spurSichtbar(const LeistenSpur& spur) const
{
    if (spur.schalter.isEmpty()) return m_steuerSichtbar[static_cast<std::size_t>(spur.steuer)];
    return std::find(m_schalterSichtbar.begin(), m_schalterSichtbar.end(), spur.schalter) != m_schalterSichtbar.end();
}

void NotenRaster::zeigeSpurenMitWerten()
{
    for (const Steuerspur s : kAlleSteuerspuren)
        m_steuerSichtbar[static_cast<std::size_t>(s)] = !m_muster.zeitspuren.von(s).empty();
    m_schalterSichtbar.clear();
    for (const SchalterSpur& schalter : m_muster.zeitspuren.schalter)
        if (!schalter.punkte.empty()) m_schalterSichtbar.push_back(schalter.art);
    setRollen(m_rollX, m_rollY);
    update();
}

std::vector<NotenRaster::LeistenSpur> NotenRaster::sichtbareSpuren() const
{
    std::vector<LeistenSpur> spuren = angeboteneSpuren();
    spuren.erase(std::remove_if(spuren.begin(), spuren.end(), [this](const LeistenSpur& s) { return !spurSichtbar(s); }),
                 spuren.end());
    return spuren;
}

bool NotenRaster::hatWerte(const LeistenSpur& spur) const
{
    const std::vector<lumi::loop::SpurPunkt>* punkte = punkteVon(m_muster, spur);
    return punkte != nullptr && !punkte->empty();
}

QString NotenRaster::spurKuerzel(const LeistenSpur& spur)
{
    if (!spur.schalter.isEmpty()) return spur.schalter.left(6);
    switch (spur.steuer)
    {
        case Steuerspur::Sustain: return tr("Sus");
        case Steuerspur::Sostenuto: return tr("Sost");
        case Steuerspur::Expression: return tr("Expr");
        case Steuerspur::Vibrato: return tr("Vib");
        case Steuerspur::Pitch: return tr("Pitch");
    }
    return {};
}

QString NotenRaster::spurName(const LeistenSpur& spur)
{
    if (!spur.schalter.isEmpty()) return spur.schalter;
    switch (spur.steuer)
    {
        case Steuerspur::Sustain: return tr("Sustain");
        case Steuerspur::Sostenuto: return tr("Sostenuto");
        case Steuerspur::Expression: return tr("Expression");
        case Steuerspur::Vibrato: return tr("Vibrato");
        case Steuerspur::Pitch: return tr("Pitch");
    }
    return {};
}

int NotenRaster::spurBei(int y) const
{
    if (y < spurenOben()) return -1;
    const int nr = (y - spurenOben()) / kSpurHoehe;
    return nr < static_cast<int>(sichtbareSpuren().size()) ? nr : -1;
}

std::pair<double, double> NotenRaster::spurBereich(const LeistenSpur& spur) const
{
    if (!spur.schalter.isEmpty()) return {0.0, 1.0};
    if (spur.steuer == Steuerspur::Pitch)
    {
        // SK3 a: der Bereich des Instruments — was dahinter liegt, wird gekappt gezeichnet
        const double bereich = m_pitchBereich > 0.0 ? m_pitchBereich : kPitchBereichOhne;
        return {-bereich, bereich};
    }
    return {spurMin(spur.steuer), spurMax(spur.steuer)};
}

int NotenRaster::yVonWert(int nr, double wert) const
{
    const std::vector<LeistenSpur> spuren = sichtbareSpuren();
    if (nr < 0 || nr >= static_cast<int>(spuren.size())) return -1;
    const auto [unten, oben] = spurBereich(spuren[static_cast<std::size_t>(nr)]);
    const double anteil = std::clamp((wert - unten) / (oben - unten), 0.0, 1.0);
    // drei Pixel Luft oben und unten
    return spurOben(nr) + kSpurHoehe - 4 - static_cast<int>(std::lround(anteil * (kSpurHoehe - 8)));
}

double NotenRaster::wertBei(int nr, int y) const
{
    const std::vector<LeistenSpur> spuren = sichtbareSpuren();
    if (nr < 0 || nr >= static_cast<int>(spuren.size())) return 0.0;
    const LeistenSpur& spur = spuren[static_cast<std::size_t>(nr)];
    const auto [unten, oben] = spurBereich(spur);
    const double anteil = std::clamp(static_cast<double>(spurOben(nr) + kSpurHoehe - 4 - y) / (kSpurHoehe - 8), 0.0, 1.0);
    if (!spur.schalter.isEmpty()) return anteil >= 0.5 ? 1.0 : 0.0;
    double wert = unten + anteil * (oben - unten);
    // auf eine lesbare Stufe: Hundertstel, Vibrato ganze Cent, Pitch Hundertstel eines Halbtons
    wert = spur.steuer == Steuerspur::Vibrato ? std::round(wert) : std::round(wert * 100.0) / 100.0;
    return std::clamp(wert, unten, oben);
}

double NotenRaster::stelleBei(int x, bool frei) const
{
    const double stelle = (x - kTastenBreite + m_rollX) / m_schrittBreite;
    return frei ? std::round(stelle * 100.0) / 100.0 : std::round(stelle);
}

namespace
{

/// Die Punkte einer Spur im Muster, fuer ein veraenderliches und ein festes Muster.
template <class M, class Spur>
auto punkteIn(M& muster, const Spur& spur) -> decltype(&muster.zeitspuren.von(spur.steuer))
{
    if (spur.schalter.isEmpty()) return &muster.zeitspuren.von(spur.steuer);
    for (auto& schalter : muster.zeitspuren.schalter)
        if (schalter.art == spur.schalter) return &schalter.punkte;
    return nullptr;
}

}  // namespace

std::vector<lumi::loop::SpurPunkt>* NotenRaster::punkteVon(Muster& muster, const LeistenSpur& spur)
{
    return punkteIn(muster, spur);
}

const std::vector<lumi::loop::SpurPunkt>* NotenRaster::punkteVon(const Muster& muster, const LeistenSpur& spur)
{
    return punkteIn(muster, spur);
}

int NotenRaster::spurPunktBei(int nr, QPoint ort) const
{
    const std::vector<LeistenSpur> spuren = sichtbareSpuren();
    if (nr < 0 || nr >= static_cast<int>(spuren.size())) return -1;
    const std::vector<lumi::loop::SpurPunkt>* punkte = punkteVon(m_muster, spuren[static_cast<std::size_t>(nr)]);
    if (punkte == nullptr) return -1;
    int bester = -1;
    int abstand = kPunktFang + 1;
    for (int i = 0; i < static_cast<int>(punkte->size()); ++i)
    {
        const lumi::loop::SpurPunkt& p = (*punkte)[static_cast<std::size_t>(i)];
        const int d = std::max(std::abs(ort.x() - xVonStelle(p.stelle)), std::abs(ort.y() - yVonWert(nr, p.wert)));
        if (d < abstand)
        {
            abstand = d;
            bester = i;
        }
    }
    return bester;
}

void NotenRaster::meldeSpuren(const QString& was)
{
    update();
    Q_EMIT geaendert(m_muster, was);
}

int NotenRaster::legeSpurPunkt(const LeistenSpur& spur, lumi::loop::SpurPunkt punkt)
{
    std::vector<lumi::loop::SpurPunkt>* punkte = punkteVon(m_muster, spur);
    if (punkte == nullptr || punkt.stelle < 0.0 || punkt.stelle >= schritte()) return -1;
    const auto ort = std::find_if(punkte->begin(), punkte->end(),
                                  [&](const lumi::loop::SpurPunkt& p) { return p.stelle == punkt.stelle; });
    if (ort != punkte->end())
    {
        ort->wert = punkt.wert;  // je Stelle ein Punkt: er ersetzt den alten
        return static_cast<int>(ort - punkte->begin());
    }
    if (static_cast<int>(punkte->size()) >= kZeitspurPunkteMax) return -1;
    const auto neu = punkte->insert(std::upper_bound(punkte->begin(), punkte->end(), punkt,
                                                     [](const lumi::loop::SpurPunkt& a, const lumi::loop::SpurPunkt& b) {
                                                         return a.stelle < b.stelle;
                                                     }),
                                    punkt);
    return static_cast<int>(neu - punkte->begin());
}

bool NotenRaster::setzeSpurPunkt(const LeistenSpur& spur, lumi::loop::SpurPunkt punkt)
{
    const Muster vorher = m_muster;
    if (legeSpurPunkt(spur, punkt) < 0 || m_muster == vorher) return false;
    meldeSpuren(tr("Add lane point"));
    return true;
}

bool NotenRaster::loescheSpurPunkt(const LeistenSpur& spur, int index)
{
    std::vector<lumi::loop::SpurPunkt>* punkte = punkteVon(m_muster, spur);
    if (punkte == nullptr || index < 0 || index >= static_cast<int>(punkte->size())) return false;
    punkte->erase(punkte->begin() + index);
    meldeSpuren(tr("Delete lane point"));
    return true;
}

bool NotenRaster::leereSpur(const LeistenSpur& spur)
{
    std::vector<lumi::loop::SpurPunkt>* punkte = punkteVon(m_muster, spur);
    if (punkte == nullptr || punkte->empty()) return false;
    punkte->clear();
    meldeSpuren(tr("Clear lane"));
    return true;
}

QString NotenRaster::wertText(const LeistenSpur& spur, double wert)
{
    if (!spur.schalter.isEmpty()) return wert >= 0.5 ? tr("on") : tr("off");
    switch (spur.steuer)
    {
        case Steuerspur::Sustain:
        case Steuerspur::Sostenuto:
            return tr("%1 %").arg(std::lround(wert * 100.0));
        case Steuerspur::Expression:
            // SK9 a: der Pegel e², in dB 40 · lg e
            return wert <= 0.0 ? tr("%1 (silent)").arg(wert, 0, 'f', 2)
                               : tr("%1 (%2 dB)").arg(wert, 0, 'f', 2).arg(expressionDb(wert), 0, 'f', 1);
        case Steuerspur::Vibrato:
            return tr("%1 cents").arg(std::lround(wert));
        case Steuerspur::Pitch:
            return tr("%1%2 semitones").arg(wert > 0.0 ? QStringLiteral("+") : QString()).arg(wert, 0, 'f', 2);
    }
    return {};
}

void NotenRaster::zieheSpurPunkt(QPoint ort, bool frei)
{
    std::vector<lumi::loop::SpurPunkt>* punkte = punkteVon(m_muster, m_spurZug);
    if (punkte == nullptr || m_spurIndex < 0 || m_spurIndex >= static_cast<int>(punkte->size())) return;
    const std::vector<LeistenSpur> spuren = sichtbareSpuren();
    const auto nr = static_cast<int>(std::find(spuren.begin(), spuren.end(), m_spurZug) - spuren.begin());
    if (nr >= static_cast<int>(spuren.size())) return;
    lumi::loop::SpurPunkt& p = (*punkte)[static_cast<std::size_t>(m_spurIndex)];
    // zwischen den Nachbarn: je Stelle ein Punkt (Modell 28a), die Reihenfolge bleibt
    const double davor = m_spurIndex > 0 ? (*punkte)[static_cast<std::size_t>(m_spurIndex - 1)].stelle : -1.0;
    const double danach = m_spurIndex + 1 < static_cast<int>(punkte->size())
                              ? (*punkte)[static_cast<std::size_t>(m_spurIndex + 1)].stelle
                              : static_cast<double>(schritte());
    const double stelle = stelleBei(ort.x(), frei);
    if (stelle > davor && stelle < danach && stelle >= 0.0) p.stelle = stelle;
    p.wert = wertBei(nr, ort.y());
    update();
}

void NotenRaster::zeichneSpuren(QPainter& p)
{
    const std::vector<LeistenSpur> spuren = sichtbareSpuren();
    if (spuren.empty()) return;
    const QPalette pal = palette();
    const QColor text = pal.color(QPalette::Text);
    const QColor an = pal.color(QPalette::Highlight);
    // die Schalter anders gefaerbt als die Steuer-Spuren (Skizze §1a)
    const QColor schalterFarbe = pal.color(QPalette::Link);
    QColor leise = text;
    leise.setAlpha(130);
    QColor blass = text;
    blass.setAlpha(60);
    QFont klein = font();
    klein.setPointSizeF(std::max(6.5, klein.pointSizeF() - 1.0));
    const int zahl = schritte();
    const int jeTakt = m_schlaege * m_jeSchlag;
    const int links = kTastenBreite;
    const int rechts = std::min(width(), xVonSchritt(zahl));

    for (int nr = 0; nr < static_cast<int>(spuren.size()); ++nr)
    {
        const LeistenSpur& spur = spuren[static_cast<std::size_t>(nr)];
        const int y0 = spurOben(nr);
        const bool getragen = !spur.schalter.isEmpty() || traegt(spur.steuer);
        p.setClipping(false);
        p.setPen(leise);
        p.drawLine(0, y0 + kSpurHoehe - 1, width(), y0 + kSpurHoehe - 1);
        p.setFont(klein);
        p.setPen(getragen ? leise : blass);
        p.drawText(QRect(2, y0, kTastenBreite - 4, kSpurHoehe), Qt::AlignLeft | Qt::AlignVCenter, spurKuerzel(spur));
        p.setClipRect(QRect(links, y0, sichtBreite(), kSpurHoehe - 1));
        // die Schlaege leise, die Takte kraeftiger
        for (int i = 0; i <= zahl; i += m_jeSchlag)
        {
            const int x = xVonSchritt(i);
            if (x < links || x > width()) continue;
            QColor linie = text;
            linie.setAlpha(i % jeTakt == 0 ? 90 : 30);
            p.setPen(linie);
            p.drawLine(x, y0, x, y0 + kSpurHoehe - 2);
        }
        const std::vector<lumi::loop::SpurPunkt>* punkte = punkteVon(m_muster, spur);
        if (punkte == nullptr || punkte->empty())
        {
            // eine leere Spur: die Vorgabe des Instruments (SF4 a, SF5 d)
            QPen strich(blass);
            strich.setStyle(Qt::DashLine);
            p.setPen(strich);
            const int mitte = y0 + kSpurHoehe / 2;
            p.drawLine(links, mitte, rechts, mitte);
            p.setPen(leise);
            p.drawText(QRect(std::max(links, xVonSchritt(0)) + 4, y0, 160, kSpurHoehe / 2), Qt::AlignLeft | Qt::AlignVCenter,
                       tr("Instrument default"));
            continue;
        }
        const QColor farbe = !getragen ? blass : !spur.schalter.isEmpty() ? schalterFarbe : an;
        const bool stufen = !spur.schalter.isEmpty() || istStufenspur(spur.steuer);
        const int boden = yVonWert(nr, spurBereich(spur).first);
        if (stufen)
        {
            // als Flaeche von unten: der Wert gilt bis zum naechsten Punkt, vor dem ersten oben (aus)
            QColor flaeche = farbe;
            flaeche.setAlpha(110);
            for (std::size_t i = 0; i < punkte->size(); ++i)
            {
                const double von = (*punkte)[i].stelle;
                const double bis = i + 1 < punkte->size() ? (*punkte)[i + 1].stelle : static_cast<double>(zahl);
                const int oben = yVonWert(nr, (*punkte)[i].wert);
                if (oben >= boden) continue;
                p.fillRect(QRect(QPoint(xVonStelle(von), oben), QPoint(xVonStelle(bis) - 1, boden)), flaeche);
            }
        }
        else
        {
            // Pitch: die Mittellinie ist die gespielte Hoehe
            if (spur.steuer == Steuerspur::Pitch)
            {
                p.setPen(blass);
                const int null = yVonWert(nr, 0.0);
                p.drawLine(links, null, rechts, null);
            }
            // als Kurve: vor dem ersten Punkt sein Wert, nach dem letzten dessen
            QPainterPath pfad;
            pfad.moveTo(xVonSchritt(0), yVonWert(nr, punkte->front().wert));
            for (const lumi::loop::SpurPunkt& punkt : *punkte) pfad.lineTo(xVonStelle(punkt.stelle), yVonWert(nr, punkt.wert));
            pfad.lineTo(xVonSchritt(zahl), yVonWert(nr, punkte->back().wert));
            p.setPen(QPen(farbe, 1.5));
            p.setBrush(Qt::NoBrush);
            p.drawPath(pfad);
        }
        // die Punkte; einer jenseits des Bereichs (Pitch, SK3 a) hohl und rot — gekappt, nicht geaendert
        const auto [unten, oben] = spurBereich(spur);
        for (int i = 0; i < static_cast<int>(punkte->size()); ++i)
        {
            const lumi::loop::SpurPunkt& punkt = (*punkte)[static_cast<std::size_t>(i)];
            const bool gekappt = punkt.wert < unten || punkt.wert > oben;
            const bool gezogen = m_zug == Zug::SpurPunkt && m_spurZug == spur && m_spurIndex == i;
            p.setPen(gekappt ? QColor(Qt::red) : farbe);
            p.setBrush(gekappt ? QBrush(Qt::NoBrush) : QBrush(gezogen ? text : farbe));
            p.drawEllipse(QPoint(xVonStelle(punkt.stelle), yVonWert(nr, punkt.wert)), 3, 3);
        }
        p.setBrush(Qt::NoBrush);
    }
    p.setClipping(false);
}

bool NotenRaster::spurTooltip(const QHelpEvent& hilfe)
{
    const QPoint ort = hilfe.pos();
    const int nr = spurBei(ort.y());
    if (nr < 0 || ort.y() >= tabOben()) return false;
    const LeistenSpur spur = sichtbareSpuren()[static_cast<std::size_t>(nr)];
    QString text;
    const bool getragen = !spur.schalter.isEmpty() || traegt(spur.steuer);
    if (ort.x() < kTastenBreite)
        text = spurName(spur);  // SK2 a: der volle Name ueber dem Kuerzel
    else if (const int punkt = spurPunktBei(nr, ort); punkt >= 0)
        text = tr("%1: %2 — drag to move, double-click to delete")
                   .arg(spurName(spur), wertText(spur, (*punkteVon(m_muster, spur))[static_cast<std::size_t>(punkt)].wert));
    else if (!hatWerte(spur))
        text = tr("%1: instrument default — click to add a point").arg(spurName(spur));
    else
        text = tr("%1: click to add a point on the step (Alt: off the grid), right-click: Clear lane").arg(spurName(spur));
    if (!getragen) text += tr("\nThis instrument does not use it — the values stay in the pattern");
    QToolTip::showText(hilfe.globalPos(), text, this);
    return true;
}

bool NotenRaster::spurMenue(const QContextMenuEvent& event)
{
    const QPoint ort = event.pos();
    const int nr = spurBei(ort.y());
    if (nr < 0 || ort.y() >= tabOben() || ort.x() < kTastenBreite) return false;
    const LeistenSpur spur = sichtbareSpuren()[static_cast<std::size_t>(nr)];
    const int punkt = spurPunktBei(nr, ort);
    QMenu menu(this);
    menu.setObjectName(QStringLiteral("instrumentLaneMenu"));
    menu.addAction(tr("Delete point"), this, [this, spur, punkt] { (void)loescheSpurPunkt(spur, punkt); })
        ->setEnabled(punkt >= 0);
    menu.addAction(tr("Clear lane"), this, [this, spur] { (void)leereSpur(spur); })->setEnabled(hatWerte(spur));
    lumi::ui::zeigeMenue(menu, event.globalPos());
    return true;
}

}  // namespace lumi::loop
