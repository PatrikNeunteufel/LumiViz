/**
 * @file KernAnsicht.cpp
 * @brief Loop: eine Kern-Schicht im Creator ansehen (FU3 a, Stufe 34d)
 */

#include "loop/KernAnsicht.hpp"

#include "InstrumentModels/Kerne.hpp"
#include "loop/Boden.hpp"

#include <QFont>
#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QScrollArea>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include <variant>

namespace lumi::loop
{

namespace
{

const kern::KernTyp* typVon(const Schicht& schicht)
{
    return istKernSchicht(schicht) ? kern::kerne().typ(schicht.kernName.toStdString()) : nullptr;
}

QString qs(const std::string& text)
{
    return QString::fromStdString(text);
}

/// Eine Zahl kurz: drei gueltige Stellen, ohne Exponent zwischen 0,001 und 99 999.
QString zahlText(double wert)
{
    const double betrag = std::abs(wert);
    if (betrag != 0.0 && (betrag < 1e-3 || betrag >= 1e5)) return QString::number(wert, 'g', 3);
    if (betrag >= 1000.0) return QString::number(wert, 'f', 0);  // 9600, nicht 9599.69 und nicht 9.6e+03
    return QString::number(wert, 'g', 3);
}

QString mitEinheit(const QString& wert, const std::string& einheit)
{
    return einheit.empty() ? wert : wert + QLatin1Char(' ') + qs(einheit);
}

/// Die Farbe einer Marke der Belegkraft: gemessen gruen, Quelle blau, Rechnung grau, offen bernstein.
QColor belegFarbe(kern::Belegkraft b, const QPalette& farben)
{
    const bool dunkel = farben.color(QPalette::Window).lightness() < 128;
    switch (b)
    {
    case kern::Belegkraft::Gemessen: return dunkel ? QColor(110, 200, 120) : QColor(30, 130, 50);
    case kern::Belegkraft::Quelle: return dunkel ? QColor(120, 170, 240) : QColor(30, 90, 190);
    case kern::Belegkraft::Rechnung: return farben.color(QPalette::PlaceholderText);
    case kern::Belegkraft::Offen: return dunkel ? QColor(240, 180, 80) : QColor(170, 100, 0);
    }
    return farben.color(QPalette::WindowText);
}

/// Ein Layout leeren: seine Widgets und die verschachtelter Layouts loeschen.
void leere(QLayout* layout)
{
    while (QLayoutItem* teil = layout->takeAt(0))
    {
        if (QLayout* innen = teil->layout()) leere(innen);
        delete teil->widget();
        delete teil;
    }
}

QString tastenText(int note)
{
    return QStringLiteral("%1 (%2)").arg(tonName(note)).arg(note);
}

}  // namespace

// =============================================================================
// KernKurvenBild
// =============================================================================

KernKurvenBild::KernKurvenBild(QWidget* parent) : QWidget(parent)
{
    setMinimumSize(90, 18);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void KernKurvenBild::setze(const kern::Kurve& kurve, kern::Skala skala, int noteMin, int noteMax)
{
    m_werte.clear();
    m_eigene.clear();
    for (int note = noteMin; note <= noteMax; ++note)
    {
        m_werte.push_back(kern::wertAn(kurve, note, skala));
        if (std::any_of(kurve.abweichungen.begin(), kurve.abweichungen.end(), [note](const kern::NotenWert& a) { return a.note == note; }))
            m_eigene.push_back(note - noteMin);
    }
    update();
}

void KernKurvenBild::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QRectF feld = QRectF(rect()).adjusted(1.5, 2.5, -1.5, -2.5);
    p.setPen(QPen(palette().color(QPalette::Mid), 1.0));
    p.drawLine(feld.bottomLeft(), feld.bottomRight());
    if (m_werte.size() < 2) return;
    const auto [klein, gross] = std::minmax_element(m_werte.begin(), m_werte.end());
    const double spanne = *gross - *klein;
    const auto punkt = [&](std::size_t i) {
        const double x = feld.left() + feld.width() * static_cast<double>(i) / static_cast<double>(m_werte.size() - 1);
        const double anteil = spanne > 0.0 ? (m_werte[i] - *klein) / spanne : 0.5;
        return QPointF(x, feld.bottom() - anteil * feld.height());
    };
    QPainterPath weg(punkt(0));
    for (std::size_t i = 1; i < m_werte.size(); ++i) weg.lineTo(punkt(i));
    p.setPen(QPen(palette().color(QPalette::Highlight), 1.4));
    p.drawPath(weg);
    p.setPen(Qt::NoPen);
    p.setBrush(palette().color(QPalette::WindowText));
    for (const int i : m_eigene) p.drawEllipse(punkt(static_cast<std::size_t>(i)), 2.0, 2.0);
}

// =============================================================================
// KernAnsicht
// =============================================================================

KernAnsicht::KernAnsicht(QWidget* parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("creatorCoreView"));
    auto* aussen = new QVBoxLayout(this);
    aussen->setContentsMargins(0, 0, 0, 0);
    auto* kopfZeile = new QHBoxLayout;
    m_kopf = new QLabel(this);
    m_kopf->setObjectName(QStringLiteral("creatorCoreTitle"));
    QFont fett = m_kopf->font();
    fett.setBold(true);
    m_kopf->setFont(fett);
    m_ungemessen = new QLabel(tr("Unmeasured"), this);
    m_ungemessen->setObjectName(QStringLiteral("creatorCoreUnmeasured"));
    m_ungemessen->setToolTip(tr("At least one value that shapes the sound has not been measured on a recording yet — "
                                "it comes from a source, a calculation, or is open"));
    m_ungemessen->setStyleSheet(QStringLiteral("QLabel { border: 1px solid palette(mid); border-radius: 3px; padding: 0 4px; }"));
    kopfZeile->addWidget(m_kopf);
    kopfZeile->addWidget(m_ungemessen);
    kopfZeile->addStretch(1);
    aussen->addLayout(kopfZeile);
    m_unterzeile = new QLabel(this);
    m_unterzeile->setObjectName(QStringLiteral("creatorCoreInfo"));
    m_unterzeile->setEnabled(false);
    m_unterzeile->setWordWrap(true);
    aussen->addWidget(m_unterzeile);
    m_hinweis = new QLabel(tr("View only — editing the values of a core comes later"), this);
    m_hinweis->setObjectName(QStringLiteral("creatorCoreHint"));
    m_hinweis->setEnabled(false);
    aussen->addWidget(m_hinweis);

    m_inhalt = new QWidget;
    m_inhalt->setObjectName(QStringLiteral("creatorCoreValues"));
    m_gitter = new QGridLayout(m_inhalt);
    m_gitter->setContentsMargins(0, 0, 6, 0);
    m_gitter->setColumnStretch(1, 1);
    m_rollen = new QScrollArea(this);
    m_rollen->setObjectName(QStringLiteral("creatorCoreScroll"));
    m_rollen->setFrameShape(QFrame::NoFrame);
    m_rollen->setWidgetResizable(true);
    m_rollen->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_rollen->setWidget(m_inhalt);
    m_rollen->viewport()->setAutoFillBackground(false);
    m_inhalt->setAutoFillBackground(false);
    aussen->addWidget(m_rollen, 1);
}

QString KernAnsicht::kernTitel(const Schicht& schicht)
{
    if (const kern::KernTyp* typ = typVon(schicht)) return qs(typ->tafel.titel);
    return schicht.kernName;
}

QString KernAnsicht::belegkraftTitel(kern::Belegkraft belegkraft)
{
    switch (belegkraft)
    {
    case kern::Belegkraft::Gemessen: return tr("Measured");
    case kern::Belegkraft::Quelle: return tr("Source");
    case kern::Belegkraft::Rechnung: return tr("Computed");
    case kern::Belegkraft::Offen: return tr("Open");
    }
    return tr("Open");
}

bool KernAnsicht::ungemessen(const Schicht& schicht)
{
    const kern::KernTyp* typ = typVon(schicht);
    return typ != nullptr && kern::ungemessen(typ->tafel, schicht.kernWerte);
}

QString KernAnsicht::kopf() const
{
    return m_kopf->text();
}

bool KernAnsicht::markeUngemessen() const
{
    return !m_ungemessen->isHidden();
}

void KernAnsicht::setzeMarkeHier(bool an)
{
    m_markeHier = an;
    baue();
}

void KernAnsicht::setzeGruppeZu(const QString& gruppe, bool zu)
{
    if (zu)
        m_zu.insert(gruppe);
    else
        m_zu.remove(gruppe);
    baue();
}

void KernAnsicht::zeige(const Schicht& schicht, Teil teil)
{
    m_schicht = schicht;
    m_teil = teil;
    baue();
}

void KernAnsicht::baue()
{
    // das Gitter von vorn: eine Tafel hat wenige Dutzend Zeilen (eine Zeile mit Kurve ist ein eigenes Layout)
    leere(m_gitter);
    m_zeilen.clear();
    m_gruppenNamen.clear();
    const kern::KernTyp* typ = typVon(m_schicht);
    const Schicht& s = m_schicht;

    if (typ == nullptr)
    {
        m_kopf->setText(tr("Core · %1").arg(s.kernName));
        m_unterzeile->setText(tr("This program does not know the core \"%1\"").arg(s.kernName));
        m_ungemessen->setVisible(false);
        m_hinweis->setVisible(false);
        return;
    }
    const kern::Parametertafel& tafel = typ->tafel;
    // die Marke steht im Creator neben Type (§6a 2); hier nur, wo die Ansicht allein steht
    m_ungemessen->setVisible(m_markeHier && m_teil == Teil::Kern && kern::ungemessen(tafel, s.kernWerte));
    m_hinweis->setVisible(m_teil == Teil::Kern);

    if (m_teil == Teil::Koerper)
    {
        // FU1 a: der Koerper an der Schicht — die App faltet die Kraft des Kerns mit seiner Antwort
        m_kopf->setText(tr("Body"));
        m_unterzeile->setText(tr("The resonance body the force of the core sounds through, heard at two places above it"));
        QString wert = tr("none — the force at the bridge, both sides alike");
        if (!s.koerper.isEmpty()) wert = bodenVon(s.koerper) ? s.koerper : tr("%1 (not found)").arg(s.koerper);
        auto* name = new QLabel(tr("Body"), m_inhalt);
        auto* text = new QLabel(wert, m_inhalt);
        text->setObjectName(QStringLiteral("creatorCoreBody"));
        text->setTextInteractionFlags(Qt::TextSelectableByMouse);
        m_gitter->addWidget(name, 0, 0);
        m_gitter->addWidget(text, 0, 1, 1, 2);
        m_gitter->setRowStretch(1, 1);
        m_zeilen << QStringLiteral("Body: %1").arg(wert);
        return;
    }

    m_kopf->setText(tr("Core · %1").arg(qs(tafel.titel)));
    m_unterzeile->setText(tr("Core %1 · keys %2 to %3").arg(qs(tafel.kern), tastenText(tafel.noteMin), tastenText(tafel.noteMax)));

    int zeile = 0;
    for (const std::string& gruppeStd : tafel.gruppen)
    {
        const QString gruppe = qs(gruppeStd);
        m_gruppenNamen << gruppe;
        const bool zu = m_zu.contains(gruppe);
        auto* kopf = new QToolButton(m_inhalt);
        kopf->setObjectName(QStringLiteral("creatorCoreGroup_") + gruppe);
        kopf->setText(gruppe);
        kopf->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        kopf->setArrowType(zu ? Qt::RightArrow : Qt::DownArrow);
        kopf->setAutoRaise(true);
        kopf->setCursor(Qt::PointingHandCursor);
        kopf->setToolTip(tr("Click to fold or unfold this group"));
        QFont fett = kopf->font();
        fett.setBold(true);
        kopf->setFont(fett);
        // verzoegert: das Neubauen loescht den Knopf, der gerade meldet
        connect(kopf, &QToolButton::clicked, this, [this, gruppe] { setzeGruppeZu(gruppe, !gruppeZu(gruppe)); }, Qt::QueuedConnection);
        m_gitter->addWidget(kopf, zeile++, 0, 1, 3, Qt::AlignLeft);
        for (const kern::ParameterAngabe& a : tafel.werte)
        {
            if (a.gruppe != gruppeStd) continue;
            const kern::Wert* w = s.kernWerte.finde(a.name);
            QString text = tr("missing");
            kern::Belegkraft beleg = a.belegkraft;
            QString herkunft = qs(a.herkunft);
            KernKurvenBild* bild = nullptr;
            if (w != nullptr)
            {
                if (const auto* k = std::get_if<kern::Kurve>(w))
                {
                    beleg = k->belegkraft;
                    herkunft = qs(k->herkunft);
                    double klein = 0.0;
                    double gross = 0.0;
                    for (int note = tafel.noteMin; note <= tafel.noteMax; ++note)
                    {
                        const double v = kern::wertAn(*k, note, a.skala);
                        klein = note == tafel.noteMin ? v : std::min(klein, v);
                        gross = note == tafel.noteMin ? v : std::max(gross, v);
                    }
                    text = klein == gross ? mitEinheit(zahlText(klein), a.einheit)
                                          : mitEinheit(QStringLiteral("%1 … %2").arg(zahlText(klein), zahlText(gross)), a.einheit);
                    if (!k->abweichungen.empty()) text += tr(" · %1 own").arg(k->abweichungen.size());
                    if (!zu)
                    {
                        bild = new KernKurvenBild(m_inhalt);
                        bild->setObjectName(QStringLiteral("creatorCoreCurve_") + qs(a.name));
                        bild->setze(*k, a.skala, tafel.noteMin, tafel.noteMax);
                    }
                }
                else if (const auto* d = std::get_if<double>(w))
                    text = mitEinheit(zahlText(*d), a.einheit);
                else if (const auto* g = std::get_if<std::int64_t>(w))
                    text = a.einheit == "MIDI" ? tastenText(static_cast<int>(*g)) : mitEinheit(QString::number(*g), a.einheit);
                else if (const auto* b = std::get_if<bool>(w))
                    text = *b ? tr("on") : tr("off");
                else if (const auto* t = std::get_if<std::string>(w))
                    text = qs(*t);
                else if (const auto* l = std::get_if<kern::Liste>(w))
                    text = tr("%1 entries").arg(l->eintraege.size());
            }
            const QString belegText = belegkraftTitel(beleg);
            m_zeilen << QStringLiteral("%1 / %2: %3 [%4]").arg(gruppe, qs(a.titel), text, belegText);
            if (zu) continue;
            const QString hilfe = herkunft.isEmpty() ? belegText : belegText + QStringLiteral(": ") + herkunft;
            auto* name = new QLabel(qs(a.titel), m_inhalt);
            name->setToolTip(hilfe);
            auto* wert = new QLabel(text, m_inhalt);
            wert->setObjectName(QStringLiteral("creatorCoreValue_") + qs(a.name));
            wert->setToolTip(hilfe);
            auto* marke = new QLabel(belegText, m_inhalt);
            marke->setObjectName(QStringLiteral("creatorCoreEvidence_") + qs(a.name));
            marke->setToolTip(hilfe);
            QPalette farben = marke->palette();
            farben.setColor(QPalette::WindowText, belegFarbe(beleg, palette()));
            marke->setPalette(farben);
            m_gitter->addWidget(name, zeile, 0);
            if (bild != nullptr)
            {
                auto* spalte = new QHBoxLayout;
                spalte->addWidget(bild, 1);
                spalte->addWidget(wert);
                m_gitter->addLayout(spalte, zeile, 1);
            }
            else
                m_gitter->addWidget(wert, zeile, 1);
            m_gitter->addWidget(marke, zeile, 2);
            ++zeile;
        }
    }
    m_gitter->setRowStretch(zeile, 1);
}

}  // namespace lumi::loop
