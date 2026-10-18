/**
 * @file SaitenBild.cpp
 * @brief Das Saitenbild der Gruppe String (Stufe 23e)
 */
#include "loop/SaitenBild.hpp"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

#include <algorithm>
#include <cmath>
#include <vector>

namespace lumi::loop
{

namespace
{

constexpr int kRand = 14;            ///< links und rechts: Platz fuer Sattel, Steg und Beschriftung
constexpr int kAnschlagFarbe = 0xe8a33d;
constexpr double kLageMin = 0.002;   ///< wie die Abnahme sie begrenzt (Instrument.cpp)
constexpr double kLageMax = 0.998;

QColor farbe(int rgb)
{
    return QColor::fromRgb(static_cast<QRgb>(rgb));
}

}  // namespace

SaitenBild::SaitenBild(QWidget* parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("creatorStringPicture"));
    setMouseTracking(true);  // der Zeiger wechselt schon ueber einer Marke, nicht erst beim Druecken
    setToolTip(tr("Nut left, bridge right. Drag pluck to change where the string is plucked;\n"
                  "with Waveguide, drag neck and bridge to move the pickups"));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void SaitenBild::setze(double anschlagOrt, double mensurMm, const std::array<Abnehmer, 2>& abnehmer, bool mitAbnehmern)
{
    m_anschlag = std::clamp(anschlagOrt, kAnschlagOrtMin, kAnschlagOrtMax);
    m_mensur = mensurMm > 0.0 ? mensurMm : kMensurVorgabeMm;
    m_abnehmer = abnehmer;
    m_mitAbnehmern = mitAbnehmern;
    update();
}

int SaitenBild::links() const
{
    return kRand;
}

int SaitenBild::rechts() const
{
    return std::max(links() + 1, width() - kRand);
}

int SaitenBild::saiteY() const
{
    return 36;
}

int SaitenBild::xVon(double anteilVomSteg) const
{
    return static_cast<int>(std::lround(rechts() - anteilVomSteg * (rechts() - links())));
}

double SaitenBild::anteilBei(int x) const
{
    return static_cast<double>(rechts() - x) / static_cast<double>(rechts() - links());
}

bool SaitenBild::ziehbar(int platz) const
{
    const AbnehmerArt art = m_abnehmer[static_cast<std::size_t>(platz)].art;
    return m_mitAbnehmern && art != AbnehmerArt::Keiner && art != AbnehmerArt::Piezo;
}

QPoint SaitenBild::ortVon(Griff griff) const
{
    if (griff == GriffAnschlag) return {xVon(m_anschlag), saiteY() - 8};
    if (griff == GriffHals || griff == GriffSteg)
    {
        const int platz = griff == GriffHals ? 0 : 1;
        if (!ziehbar(platz)) return {-1, -1};
        return {xVon(m_abnehmer[static_cast<std::size_t>(platz)].mmVomSteg / m_mensur), saiteY() + 14};
    }
    return {-1, -1};
}

SaitenBild::Griff SaitenBild::griffBei(QPoint ort) const
{
    // ueber der Saite die Marke pluck, darunter die Tonabnehmer
    if (ort.y() <= saiteY() + 2)
        return std::abs(ort.x() - xVon(m_anschlag)) <= kGriffWeite ? GriffAnschlag : KeinGriff;
    Griff bester = KeinGriff;
    int naechst = kGriffWeite + 1;
    for (const Griff griff : {GriffHals, GriffSteg})
    {
        const int platz = griff == GriffHals ? 0 : 1;
        if (!ziehbar(platz)) continue;
        const Abnehmer& a = m_abnehmer[static_cast<std::size_t>(platz)];
        int abstand = std::abs(ort.x() - xVon(a.mmVomSteg / m_mensur));
        // ein Humbucker greift man auch an seiner zweiten Spule
        if (a.art == AbnehmerArt::Humbucker)
            abstand = std::min(abstand, std::abs(ort.x() - xVon((a.mmVomSteg + kHumbuckerAbstandMm) / m_mensur)));
        if (abstand < naechst)
        {
            naechst = abstand;
            bester = griff;
        }
    }
    return bester;
}

void SaitenBild::zeigeZeiger(Griff griff)
{
    if (griff == KeinGriff)
        unsetCursor();
    else
        setCursor(Qt::SizeHorCursor);
}

void SaitenBild::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) return QWidget::mousePressEvent(event);
    m_gezogen = griffBei(event->position().toPoint());
    if (m_gezogen != KeinGriff) mouseMoveEvent(event);
}

void SaitenBild::mouseMoveEvent(QMouseEvent* event)
{
    const QPoint ort = event->position().toPoint();
    if (m_gezogen == KeinGriff)
    {
        zeigeZeiger(griffBei(ort));
        return;
    }
    const double anteil = anteilBei(ort.x());
    if (m_gezogen == GriffAnschlag)
    {
        const double neu = std::clamp(anteil, kAnschlagOrtMin, kAnschlagOrtMax);
        m_anschlag = neu;
        update();
        Q_EMIT anschlagGezogen(neu);
        return;
    }
    const int platz = m_gezogen == GriffHals ? 0 : 1;
    const double mm = std::round(std::clamp(anteil, kLageMin, kLageMax) * m_mensur);
    m_abnehmer[static_cast<std::size_t>(platz)].mmVomSteg = mm;
    update();
    Q_EMIT abnehmerGezogen(platz, mm);
}

void SaitenBild::mouseReleaseEvent(QMouseEvent* event)
{
    m_gezogen = KeinGriff;
    zeigeZeiger(griffBei(event->position().toPoint()));
}

void SaitenBild::leaveEvent(QEvent* event)
{
    if (m_gezogen == KeinGriff) unsetCursor();
    QWidget::leaveEvent(event);
}

void SaitenBild::paintEvent(QPaintEvent* /*event*/)
{
    QPainter maler(this);
    maler.setRenderHint(QPainter::Antialiasing);
    const QColor text = palette().color(QPalette::WindowText);
    QColor matt = text;
    matt.setAlphaF(0.55);
    const QColor abnehmerFarbe = palette().color(QPalette::Highlight);
    const int y = saiteY();
    QFont klein = font();
    klein.setPointSizeF(klein.pointSizeF() * 0.85);
    maler.setFont(klein);
    const QFontMetrics masse(klein);
    // eine Beschriftung mittig unter oder ueber x, aber im Bild
    const auto beschrifte = [&](int x, int grundlinie, const QString& wort, const QColor& f) {
        const int breite = masse.horizontalAdvance(wort);
        const int links = std::clamp(x - breite / 2, 0, std::max(0, width() - breite));
        maler.setPen(f);
        maler.drawText(links, grundlinie, wort);
    };

    // Sattel und Steg
    maler.setPen(QPen(text, 4, Qt::SolidLine, Qt::FlatCap));
    maler.drawLine(links(), y - 9, links(), y + 9);
    maler.drawLine(rechts(), y - 9, rechts(), y + 9);
    // die Saite
    maler.setPen(QPen(matt, 1.6));
    maler.drawLine(links(), y, rechts(), y);

    // die Tonabnehmer: unter der Saite, ein Strich je Spule
    if (m_mitAbnehmern)
        for (int platz = 0; platz < 2; ++platz)
        {
            const Abnehmer& a = m_abnehmer[static_cast<std::size_t>(platz)];
            if (a.art == AbnehmerArt::Keiner) continue;
            const QString name = platz == 0 ? tr("neck") : tr("bridge");
            if (a.art == AbnehmerArt::Piezo)
            {
                // fest am Steg, nicht zu ziehen
                maler.fillRect(QRect(rechts() - 6, y + 4, 6, 10), matt);
                beschrifte(rechts() - 3, y + 30, tr("piezo"), matt);
                continue;
            }
            std::vector<double> spulen = {a.mmVomSteg};
            if (a.art == AbnehmerArt::Humbucker) spulen.push_back(a.mmVomSteg + kHumbuckerAbstandMm);
            for (const double mm : spulen)
                maler.fillRect(QRect(xVon(mm / m_mensur) - 2, y + 4, 5, 18), abnehmerFarbe);
            beschrifte(xVon(a.mmVomSteg / m_mensur), y + 36,
                       QStringLiteral("%1 %2 mm").arg(name).arg(static_cast<int>(std::lround(a.mmVomSteg))), abnehmerFarbe);
        }

    // die Marke pluck: ein Dreieck ueber der Saite, die Spitze auf ihr
    {
        const int x = xVon(m_anschlag);
        QPainterPath dreieck;
        dreieck.moveTo(x, y - 2);
        dreieck.lineTo(x - 7, y - 16);
        dreieck.lineTo(x + 7, y - 16);
        dreieck.closeSubpath();
        maler.fillPath(dreieck, farbe(kAnschlagFarbe));
        beschrifte(x, y - 20, QStringLiteral("pluck %1 %").arg(static_cast<int>(std::lround(m_anschlag * 100.0))),
                   farbe(kAnschlagFarbe));
    }

    // unten: was wo ist, und dass sich etwas ziehen laesst
    const int unten = height() - 4;
    maler.setPen(matt);
    maler.drawText(links() - 4, unten, tr("nut"));
    const QString steg = tr("bridge");
    maler.drawText(rechts() + 4 - masse.horizontalAdvance(steg), unten, steg);
    const QString hinweis = tr("drag the marks · scale %1 mm").arg(static_cast<int>(std::lround(m_mensur)));
    maler.drawText((width() - masse.horizontalAdvance(hinweis)) / 2, unten, hinweis);
}

}  // namespace lumi::loop
