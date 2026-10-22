/**
 * @file Taktachse.cpp
 * @brief Loop: die Achse des Loop-Panels
 */

#include "loop/Taktachse.hpp"

#include "UI/widgets/Farbreihen.hpp"  // dieselben Farbreihen wie die Kanaele des Composers (S128, N1 b)
#include "loop/Bearbeiten.hpp"

#include <QAction>
#include <QContextMenuEvent>
#include <QHelpEvent>
#include <QInputDialog>

#include "UI/widgets/Bedienhilfe.hpp"  // zeigeMenue, frageText: in Tests abfragbar
#include <QKeyEvent>
#include <QLineEdit>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QPolygonF>
#include <QToolTip>
#include <QWheelEvent>

#include <algorithm>
#include <array>
#include <cmath>

namespace lumi::loop
{

namespace
{

/// So nah an einer Kante des Clips greift die Maus die Kante statt des Clips.
constexpr int kKantenNaehe = 6;

/// Abstand der Striche im Lineal, in Ticks: die kleinste Zahl ganzer Takte,
/// bei der die Beschriftungen nicht zusammenstossen (48 Pixel).
std::int64_t strichTicks(double ticksJePixel)
{
    static constexpr std::array<int, 9> kStufen = {1, 2, 4, 8, 16, 32, 64, 128, 256};
    if (ticksJePixel <= 0.0) return Taktachse::kTicksJeTakt;
    for (const int takte : kStufen)
        if (static_cast<double>(takte * Taktachse::kTicksJeTakt) / ticksJePixel >= 48.0)
            return takte * Taktachse::kTicksJeTakt;
    return kStufen.back() * Taktachse::kTicksJeTakt;
}

/// Der naechste ganze Takt zu einer Stelle in Ticks (auch vor dem Anfang).
int naechsterTakt(std::int64_t ticks)
{
    return static_cast<int>(std::floor(static_cast<double>(ticks) / Taktachse::kTicksJeTakt + 0.5));
}

}  // namespace

Taktachse::Taktachse(QWidget* parent) : lumi::ui::Achse(0, kFeinsteTicksJePixel, parent)
{
    setObjectName(QStringLiteral("loopTaktachse"));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);  // sie rollt selbst
    setFocusPolicy(Qt::StrongFocus);                                // Entf und Esc
    setMouseTracking(true);                                         // der Zeiger an den Kanten der Clips
}

int Taktachse::anzeigeTakte(const Loop& loop)
{
    // Luft hinter dem Ende: sonst liesse sich nichts ueber das Ende hinaus
    // ziehen. Ein leerer Loop zeigt 16 Takte.
    const int takte = loop.durchlaufTakte();
    return std::max(16, takte + std::max(4, takte / 4));
}

void Taktachse::setLoop(const Loop& loop, bool geladen)
{
    m_loop = loop;
    m_geladen = geladen;
    m_griff = Griff::Keiner;
    m_zugLaeuft = false;
    setzeLaenge(geladen ? anzeigeTakte(loop) * kTicksJeTakt : 0);
    m_kanal = -1;
    m_clip = -1;
    m_gruppe.clear();
    m_rahmen = {};
    beginneNeu();
    updateGeometry();
    passeRollenAn();
    update();
    Q_EMIT auswahlGeaendert();
}

void Taktachse::zeigeBearbeitet(const Loop& loop)
{
    m_griff = Griff::Keiner;
    m_zugLaeuft = false;
    m_loop = loop;
    setzeLaenge(anzeigeTakte(loop) * kTicksJeTakt);
    m_rahmen = {};
    richteGruppe();  // was es nicht mehr gibt, faellt aus den Marken
    updateGeometry();
    passeRollenAn();
    update();
    Q_EMIT auswahlGeaendert();  // auch bei gleicher Auswahl: der Clip kann neue Takte haben
}

ClipOrt Taktachse::ortVon(int kanal, int clip) const
{
    if (kanal < 0 || kanal >= static_cast<int>(m_loop.kanaele.size())) return {};
    const auto& clips = m_loop.kanaele[static_cast<std::size_t>(kanal)].clips;
    if (clip < 0 || clip >= static_cast<int>(clips.size())) return {};
    return {kanal, clips[static_cast<std::size_t>(clip)].startTakt};
}

void Taktachse::richteGruppe()
{
    if (m_gruppe.size() <= 1)
    {
        // ein einzelner Clip: die Marke folgt dem angeklickten
        m_gruppe.clear();
        if (auswahl() != nullptr)
            m_gruppe.push_back(ortVon(m_kanal, m_clip));
        else
        {
            m_kanal = -1;
            m_clip = -1;
        }
        return;
    }
    std::vector<ClipOrt> gibtEs;
    for (const ClipOrt& ort : m_gruppe)
        if (findeClip(m_loop, ort) >= 0 && std::find(gibtEs.begin(), gibtEs.end(), ort) == gibtEs.end())
            gibtEs.push_back(ort);
    m_gruppe = std::move(gibtEs);
    // der angeklickte gehoert zu den markierten — sonst wird es der erste von ihnen
    if (auswahl() == nullptr ||
        std::find(m_gruppe.begin(), m_gruppe.end(), ortVon(m_kanal, m_clip)) == m_gruppe.end())
    {
        m_kanal = m_gruppe.empty() ? -1 : m_gruppe.front().kanal;
        m_clip = m_gruppe.empty() ? -1 : findeClip(m_loop, m_gruppe.front());
    }
}

bool Taktachse::istMarkiert(int kanal, int clip) const
{
    if (kanal < 0 || clip < 0) return false;
    const ClipOrt ort = ortVon(kanal, clip);
    return ort.kanal >= 0 && std::find(m_gruppe.begin(), m_gruppe.end(), ort) != m_gruppe.end();
}

void Taktachse::markiereUm(int kanal, int clip)
{
    const ClipOrt ort = ortVon(kanal, clip);
    if (ort.kanal < 0) return;
    const auto platz = std::find(m_gruppe.begin(), m_gruppe.end(), ort);
    if (platz == m_gruppe.end())
    {
        m_gruppe.push_back(ort);
        m_kanal = kanal;
        m_clip = clip;
    }
    else
    {
        m_gruppe.erase(platz);
        // der angeklickte ist jetzt der zuletzt dazugenommene, der noch markiert ist
        m_kanal = m_gruppe.empty() ? -1 : m_gruppe.back().kanal;
        m_clip = m_gruppe.empty() ? -1 : findeClip(m_loop, m_gruppe.back());
    }
    update();
    Q_EMIT auswahlGeaendert();
}

void Taktachse::markiere(const std::vector<ClipOrt>& orte)
{
    m_gruppe.clear();
    for (const ClipOrt& ort : orte)
        if (findeClip(m_loop, ort) >= 0 && std::find(m_gruppe.begin(), m_gruppe.end(), ort) == m_gruppe.end())
            m_gruppe.push_back(ort);
    m_kanal = m_gruppe.empty() ? -1 : m_gruppe.front().kanal;
    m_clip = m_gruppe.empty() ? -1 : findeClip(m_loop, m_gruppe.front());
    update();
    Q_EMIT auswahlGeaendert();
}

void Taktachse::markiereAlle()
{
    std::vector<ClipOrt> alle;
    for (int k = 0; k < static_cast<int>(m_loop.kanaele.size()); ++k)
        for (const LoopClip& clip : m_loop.kanaele[static_cast<std::size_t>(k)].clips)
            alle.push_back({k, clip.startTakt});
    markiere(alle);
}

std::vector<ClipOrt> Taktachse::clipsIm(const QRect& rahmen) const
{
    std::vector<ClipOrt> treffer;
    for (int k = 0; k < static_cast<int>(m_loop.kanaele.size()); ++k)
    {
        const auto& clips = m_loop.kanaele[static_cast<std::size_t>(k)].clips;
        for (int c = 0; c < static_cast<int>(clips.size()); ++c)
            if (clipRechteck(k, c).intersects(rahmen))
                treffer.push_back({k, clips[static_cast<std::size_t>(c)].startTakt});
    }
    return treffer;
}

void Taktachse::setSpielt(bool spielt)
{
    if (spielt == m_spielt) return;
    m_spielt = spielt;
    update();
}

void Taktachse::setStelleSchritte(double schritte)
{
    const int jeTakt = std::max(1, m_loop.schritteJeTakt());
    setStelle(static_cast<std::int64_t>(schritte / jeTakt * static_cast<double>(kTicksJeTakt)));
}

void Taktachse::setNebenStellenSchritte(std::vector<double> schritte)
{
    if (schritte == m_nebenSchritte) return;
    m_nebenSchritte = std::move(schritte);
    update();
}

void Taktachse::setStartmarke(double schritte)
{
    if (schritte == m_startSchritte) return;
    m_startSchritte = schritte;
    update();
}

int Taktachse::schrittBeiX(int x) const
{
    const int jeTakt = std::max(1, m_loop.schritteJeTakt());
    const double schritte = static_cast<double>(stelleVon(x)) * jeTakt / static_cast<double>(kTicksJeTakt);
    return std::clamp(static_cast<int>(std::lround(schritte)), 0, std::max(0, m_loop.durchlaufSchritte()));
}

const LoopClip* Taktachse::auswahl() const
{
    if (m_kanal < 0 || m_kanal >= static_cast<int>(m_loop.kanaele.size())) return nullptr;
    const auto& clips = m_loop.kanaele[static_cast<std::size_t>(m_kanal)].clips;
    if (m_clip < 0 || m_clip >= static_cast<int>(clips.size())) return nullptr;
    return &clips[static_cast<std::size_t>(m_clip)];
}

void Taktachse::waehle(int kanal, int clip)
{
    if (kanal == m_kanal && clip == m_clip && m_gruppe.size() <= 1) return;
    m_kanal = kanal;
    m_clip = clip;
    m_gruppe.clear();  // nur noch dieser eine
    richteGruppe();
    update();
    Q_EMIT auswahlGeaendert();
}

int Taktachse::zeilenZahl() const
{
    return std::max<int>(2, static_cast<int>(m_loop.kanaele.size()) + 1);
}

int Taktachse::taktBei(int x) const
{
    return static_cast<int>(stelleVon(x) / kTicksJeTakt);
}

int Taktachse::kanalBei(int y) const
{
    const int zeile = zeileBei(y);
    return zeile >= 0 && zeile < static_cast<int>(m_loop.kanaele.size()) ? zeile : -1;
}

QRect Taktachse::clipRechteck(int kanal, int clip) const
{
    if (kanal < 0 || kanal >= static_cast<int>(m_loop.kanaele.size())) return {};
    const auto& clips = m_loop.kanaele[static_cast<std::size_t>(kanal)].clips;
    if (clip < 0 || clip >= static_cast<int>(clips.size())) return {};
    const LoopClip& c = clips[static_cast<std::size_t>(clip)];
    const int links = xVonTakt(c.startTakt);
    const int rechts = xVonTakt(c.startTakt + m_loop.clipTakte(c));
    return {links, zeileOben(kanal) + 3, std::max(2, rechts - links), zeileHoch(kanal) - 6};
}

QColor Taktachse::basisFarbe(const QString& muster, bool an) const
{
    int nr = 0;
    for (std::size_t i = 0; i < m_loop.muster.size(); ++i)
        if (m_loop.muster[i].name == muster) nr = static_cast<int>(i);
    // zwoelf Toene: erst die kuehle, dann die warme Reihe
    const int reihen = 2 * lumi::ui::kReihenFarben;
    const bool warm = nr % reihen >= lumi::ui::kReihenFarben;
    const std::int32_t rgb = lumi::ui::reihenFarbe(warm ? lumi::ui::Farbreihe::Warm : lumi::ui::Farbreihe::Kuehl,
                                                   nr % lumi::ui::kReihenFarben);
    QColor farbe = QColor::fromRgb(static_cast<QRgb>(rgb) | 0xFF000000u);
    if (!an) farbe = QColor::fromHsv(std::max(0, farbe.hue()), std::min(farbe.saturation(), 40), 110);
    return farbe;
}

// --- Die Expression am Clip (Stufe 28e; Skizze §1a, SK8 a, SK13 a) -----------------

int Taktachse::yVonAusdruck(const QRect& clip, double wert)
{
    // vier Pixel Luft oben und unten: die Linie bei 1 liegt nicht auf dem Rand
    const int spanne = std::max(1, clip.height() - 8);
    return clip.bottom() - 4 - static_cast<int>(std::lround(std::clamp(wert, 0.0, 1.0) * spanne));
}

double Taktachse::ausdruckBei(const QRect& clip, int y)
{
    const int spanne = std::max(1, clip.height() - 8);
    const double wert = static_cast<double>(clip.bottom() - 4 - y) / spanne;
    return std::clamp(std::round(wert * 100.0) / 100.0, 0.0, 1.0);
}

double Taktachse::clipStelleBei(const LoopClip& clip, int x, bool frei) const
{
    const double takte = static_cast<double>(stelleVon(x)) / kTicksJeTakt - clip.startTakt;
    return frei ? std::round(takte * 100.0) / 100.0 : std::round(takte * 16.0) / 16.0;
}

int Taktachse::xVonClipStelle(const LoopClip& clip, double stelle) const
{
    return xVon(std::llround((clip.startTakt + stelle) * static_cast<double>(kTicksJeTakt)));
}

bool Taktachse::kurveGreifbar(int kanal, int clip) const
{
    // der angeklickte Clip, wenn er allein markiert ist (SK13 a)
    return kanal >= 0 && kanal == m_kanal && clip == m_clip && auswahl() != nullptr && m_gruppe.size() <= 1;
}

int Taktachse::ausdruckPunktBei(int kanal, int clip, QPoint ort) const
{
    if (!kurveGreifbar(kanal, clip)) return -1;
    const LoopClip& c = m_loop.kanaele[static_cast<std::size_t>(kanal)].clips[static_cast<std::size_t>(clip)];
    const QRect r = clipRechteck(kanal, clip);
    int bester = -1;
    int abstand = 6;
    for (int i = 0; i < static_cast<int>(c.expression.size()); ++i)
    {
        const SpurPunkt& p = c.expression[static_cast<std::size_t>(i)];
        const int d = std::max(std::abs(ort.x() - xVonClipStelle(c, p.stelle)), std::abs(ort.y() - yVonAusdruck(r, p.wert)));
        if (d < abstand)
        {
            abstand = d;
            bester = i;
        }
    }
    return bester;
}

bool Taktachse::aufAusdruck(int kanal, int clip, QPoint ort) const
{
    if (!kurveGreifbar(kanal, clip)) return false;
    const LoopClip& c = m_loop.kanaele[static_cast<std::size_t>(kanal)].clips[static_cast<std::size_t>(clip)];
    const QRect r = clipRechteck(kanal, clip);
    if (!r.contains(ort)) return false;
    const double stelle = static_cast<double>(stelleVon(ort.x())) / kTicksJeTakt - c.startTakt;
    return std::abs(ort.y() - yVonAusdruck(r, kurveBei(c.expression, stelle, 1.0))) <= 4;
}

int Taktachse::legeAusdruckPunkt(LoopClip& clip, SpurPunkt punkt)
{
    std::vector<SpurPunkt>& punkte = clip.expression;
    const auto gleich = std::find_if(punkte.begin(), punkte.end(), [&](const SpurPunkt& p) { return p.stelle == punkt.stelle; });
    if (gleich != punkte.end())
    {
        gleich->wert = punkt.wert;  // je Stelle ein Punkt: er ersetzt den alten
        return static_cast<int>(gleich - punkte.begin());
    }
    if (static_cast<int>(punkte.size()) >= kZeitspurPunkteMax) return -1;
    const auto neu = punkte.insert(
        std::upper_bound(punkte.begin(), punkte.end(), punkt, [](const SpurPunkt& a, const SpurPunkt& b) { return a.stelle < b.stelle; }),
        punkt);
    return static_cast<int>(neu - punkte.begin());
}

bool Taktachse::setzeAusdruckPunkt(int kanal, int clip, SpurPunkt punkt)
{
    if (clipRechteck(kanal, clip).isEmpty()) return false;
    Loop neu = m_loop;
    LoopClip& c = neu.kanaele[static_cast<std::size_t>(kanal)].clips[static_cast<std::size_t>(clip)];
    punkt.wert = std::clamp(punkt.wert, 0.0, 1.0);
    if (legeAusdruckPunkt(c, punkt) < 0 || neu == m_loop) return false;
    uebernimm(neu, kanal, clip, tr("Add expression point"));
    return true;
}

bool Taktachse::loescheAusdruckPunkt(int kanal, int clip, int index)
{
    if (clipRechteck(kanal, clip).isEmpty()) return false;
    Loop neu = m_loop;
    std::vector<SpurPunkt>& punkte = neu.kanaele[static_cast<std::size_t>(kanal)].clips[static_cast<std::size_t>(clip)].expression;
    if (index < 0 || index >= static_cast<int>(punkte.size())) return false;
    punkte.erase(punkte.begin() + index);
    uebernimm(neu, kanal, clip, tr("Delete expression point"));
    return true;
}

bool Taktachse::leereAusdruck(int kanal, int clip)
{
    if (clipRechteck(kanal, clip).isEmpty()) return false;
    Loop neu = m_loop;
    std::vector<SpurPunkt>& punkte = neu.kanaele[static_cast<std::size_t>(kanal)].clips[static_cast<std::size_t>(clip)].expression;
    if (punkte.empty()) return false;
    punkte.clear();
    uebernimm(neu, kanal, clip, tr("Clear expression"));
    return true;
}

void Taktachse::zieheAusdruck(QPoint ort, bool frei)
{
    if (m_zugKanal < 0 || m_zugClip < 0) return;
    LoopClip& c = m_loop.kanaele[static_cast<std::size_t>(m_zugKanal)].clips[static_cast<std::size_t>(m_zugClip)];
    std::vector<SpurPunkt>& punkte = c.expression;
    if (m_ausdruckIndex < 0 || m_ausdruckIndex >= static_cast<int>(punkte.size())) return;
    // zwischen den Nachbarn (je Stelle ein Punkt, die Reihenfolge bleibt) und im Clip
    const double davor = m_ausdruckIndex > 0 ? punkte[static_cast<std::size_t>(m_ausdruckIndex - 1)].stelle : -1e9;
    const double danach = m_ausdruckIndex + 1 < static_cast<int>(punkte.size())
                              ? punkte[static_cast<std::size_t>(m_ausdruckIndex + 1)].stelle
                              : 1e9;
    const double stelle = std::clamp(clipStelleBei(c, ort.x(), frei), 0.0, static_cast<double>(m_loop.clipTakte(c)));
    SpurPunkt& p = punkte[static_cast<std::size_t>(m_ausdruckIndex)];
    if (stelle > davor && stelle < danach) p.stelle = stelle;
    p.wert = ausdruckBei(clipRechteck(m_zugKanal, m_zugClip), ort.y());
    Q_EMIT hinweis(tr("Expression: %1").arg(p.wert <= 0.0 ? tr("%1 (silent)").arg(p.wert, 0, 'f', 2)
                                                            : tr("%1 (%2 dB)").arg(p.wert, 0, 'f', 2).arg(expressionDb(p.wert), 0, 'f', 1)));
    update();
}

void Taktachse::zeichneAusdruck(QPainter& p, int kanal, int clip, const QRect& r)
{
    const LoopClip& c = m_loop.kanaele[static_cast<std::size_t>(kanal)].clips[static_cast<std::size_t>(clip)];
    const bool greifbar = kurveGreifbar(kanal, clip);
    if (c.expression.empty() && !greifbar) return;  // ohne Kurve zeichnet der Clip nichts (Wert 1)
    const double laenge = m_loop.clipTakte(c);
    const QRect innen = r.adjusted(1, 1, -1, -1);
    p.save();
    p.setClipRect(innen, Qt::IntersectClip);
    if (c.expression.empty())
    {
        // der angeklickte Clip ohne Kurve: die Linie bei 1, an der man ansetzt
        p.setPen(QPen(QColor(255, 255, 255, 170), 1.0, Qt::DashLine));
        const int y = yVonAusdruck(r, 1.0);
        p.drawLine(r.left() + 2, y, r.right() - 2, y);
        if (r.height() >= 28)
        {
            QFont klein = font();
            klein.setPointSizeF(std::max(6.5, klein.pointSizeF() - 1.5));
            p.setFont(klein);
            p.drawText(QRect(r.left(), y + 1, r.width() - 6, 14), Qt::AlignRight | Qt::AlignTop, tr("Expression 1"));
            p.setFont(font());
        }
        p.restore();
        return;
    }
    // die Kurve: vor dem ersten Punkt dessen Wert, hinter dem letzten dessen — bis zum Ende des Clips
    QPolygon linie;
    linie << QPoint(r.left(), yVonAusdruck(r, kurveBei(c.expression, 0.0, 1.0)));
    for (const SpurPunkt& punkt : c.expression)
        if (punkt.stelle > 0.0 && punkt.stelle < laenge) linie << QPoint(xVonClipStelle(c, punkt.stelle), yVonAusdruck(r, punkt.wert));
    linie << QPoint(r.right(), yVonAusdruck(r, kurveBei(c.expression, laenge, 1.0)));
    p.setPen(QPen(QColor(255, 255, 255, greifbar ? 230 : 150), greifbar ? 1.6 : 1.2));
    p.drawPolyline(linie);
    p.restore();
    if (!greifbar) return;
    // die Punkte; hinter dem Ende (oder vor dem Anfang) blass — nicht zu hoeren (SK8 a)
    for (int i = 0; i < static_cast<int>(c.expression.size()); ++i)
    {
        const SpurPunkt& punkt = c.expression[static_cast<std::size_t>(i)];
        const bool hoerbar = punkt.stelle >= 0.0 && punkt.stelle <= laenge;
        const bool gezogen = m_griff == Griff::Ausdruck && m_ausdruckIndex == i;
        QColor farbe(255, 255, 255, hoerbar ? 255 : 90);
        p.setPen(QPen(farbe, 1.0));
        p.setBrush(gezogen ? QBrush(palette().color(QPalette::Highlight)) : QBrush(farbe));
        p.drawEllipse(QPoint(xVonClipStelle(c, punkt.stelle), yVonAusdruck(r, punkt.wert)), 3, 3);
    }
    p.setBrush(Qt::NoBrush);
}

void Taktachse::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    const QPalette pal = palette();
    p.fillRect(rect(), pal.color(QPalette::Base));

    if (!m_geladen)
    {
        p.setPen(pal.color(QPalette::Disabled, QPalette::Text));
        p.drawText(rect(), Qt::AlignCenter, tr("No loop loaded — New or Load..."));
        return;
    }

    const QColor text = pal.color(QPalette::Text);
    QColor linie = text;
    linie.setAlpha(50);
    const int kanaele = static_cast<int>(m_loop.kanaele.size());
    // die Unterkante der Zeile fuer einen neuen Kanal (gerollt kann sie ueber dem Rand liegen)
    const int unten = std::max(kLinealHoehe, zeileOben(kanaele + 1));
    const Flaechen ausschnitte = flaechen();

    const std::int64_t schritt = strichTicks(einheitJePixel());
    bool hatClips = false;
    for (const Kanal& kanal : m_loop.kanaele) hatClips = hatClips || !kanal.clips.empty();
    const int xEnde = xVonTakt(m_loop.durchlaufTakte());

    // Was senkrecht durch Lineal und Zeilen laeuft, wird in beiden Gaengen
    // gezeichnet; der Ausschnitt laesst jeweils nur seinen Teil durch.
    const auto unterlage = [&] {
        // --- die Striche des Lineals: jeder traegt die Nummer seines Takts
        zeichneLinealStriche(p, schritt, unten,
                             [](std::int64_t ticks) { return QString::number(ticks / kTicksJeTakt + 1); });
        // --- das Ende des Durchlaufs: dort beginnt der Loop wieder von vorn
        if (hatClips)
        {
            QColor ende = text;
            ende.setAlpha(120);
            p.setPen(QPen(ende, 1.0));
            p.drawLine(xEnde, 0, xEnde, unten);
        }
    };
    const auto auflage = [&] {
        if (!m_spielt)
        {
            // B4: wo Play beginnt — eine gruene Marke, wenn es nicht der Anfang ist
            if (m_startSchritte <= 0.0) return;
            const int jeTakt = std::max(1, m_loop.schritteJeTakt());
            const int x = xVon(static_cast<std::int64_t>(m_startSchritte / jeTakt * static_cast<double>(kTicksJeTakt)));
            const QColor gruen(80, 200, 120);
            p.setPen(QPen(gruen, 1.5));
            p.drawLine(x, 0, x, unten);
            p.setPen(Qt::NoPen);
            p.setBrush(gruen);
            const QPoint spitze[3] = {QPoint(x - 5, 0), QPoint(x + 5, 0), QPoint(x, 8)};
            p.drawPolygon(spitze, 3);
            p.setBrush(Qt::NoBrush);
            return;
        }
        // W8 a (S130): spielt der Composer diesen Loop mehrfach, die leiseren Clips blasser
        // — wie die zweite Marke im Fenster der Ueberlappung des Composers
        const int jeTakt = std::max(1, m_loop.schritteJeTakt());
        p.setPen(QPen(QColor(255, 90, 70, 120), 2.0));
        for (const double neben : m_nebenSchritte)
        {
            const int xNeben = xVon(static_cast<std::int64_t>(neben / jeTakt * static_cast<double>(kTicksJeTakt)));
            p.drawLine(xNeben, 0, xNeben, unten);
        }
        const int x = xVon(stelle());
        p.setPen(QPen(QColor(255, 90, 70), 2.0));
        p.drawLine(x, 0, x, unten);
    };

    // --- erster Gang: die Zeilen, unter dem Lineal
    p.setClipRect(ausschnitte.zeilenZeit);
    unterlage();
    p.setFont(font());
    for (int k = 0; k <= kanaele; ++k)
    {
        const int oben = zeileOben(k);
        const int hoch = zeileHoch(k);
        if (oben + hoch < kLinealHoehe || oben > height()) continue;  // aus dem Fenster gerollt
        p.setClipRect(ausschnitte.zeilen);
        p.setPen(linie);
        p.drawLine(0, oben + hoch, width(), oben + hoch);
        if (k == kanaele)
        {
            // die Zeile, auf der ein neuer Kanal entsteht
            p.setPen(pal.color(QPalette::Disabled, QPalette::Text));
            if (namePasst(hoch))
                p.drawText(QRect(8, oben, kopfBreite() - 12, hoch), Qt::AlignLeft | Qt::AlignVCenter,
                           tr("New channel"));
            continue;
        }
        const Kanal& kanal = m_loop.kanaele[static_cast<std::size_t>(k)];
        QColor name = text;
        if (!kanal.an) name.setAlpha(110);
        p.setPen(name);
        const QString titel = kanal.an ? kanal.name : tr("%1 (off)").arg(kanal.name);
        if (namePasst(hoch))
        {
            // der Fader (27e-2, LL12 a): rechts im Kopf; bei 0 dB matt, sonst gerahmt
            const QRect feld = faderFeld(k);
            const int nameBreit = std::max(0, feld.left() - 8 - 6);
            p.drawText(QRect(8, oben, nameBreit, hoch), Qt::AlignLeft | Qt::AlignVCenter,
                       fontMetrics().elidedText(titel, Qt::ElideRight, nameBreit));
            const bool aktiv = k == m_faderUnterMaus || k == m_faderZug;
            QColor schrift = kanal.gainDb == 0.0 && !aktiv ? pal.color(QPalette::Disabled, QPalette::Text) : text;
            if (!kanal.an) schrift.setAlpha(110);
            if (kanal.gainDb != 0.0 || aktiv)
            {
                QColor rand = aktiv ? pal.color(QPalette::Highlight) : linie;
                if (!kanal.an) rand.setAlpha(110);
                p.setPen(QPen(rand, aktiv ? 1.5 : 1.0));
                p.setBrush(Qt::NoBrush);
                p.drawRoundedRect(QRectF(feld).adjusted(0.5, 0.5, -0.5, -0.5), 3, 3);
            }
            p.setPen(schrift);
            p.drawText(feld, Qt::AlignCenter, faderText(kanal.gainDb));
        }

        p.setClipRect(ausschnitte.zeilenZeit);
        for (int c = 0; c < static_cast<int>(kanal.clips.size()); ++c)
        {
            const LoopClip& clip = kanal.clips[static_cast<std::size_t>(c)];
            const QRect r = clipRechteck(k, c);
            if (r.right() < kopfBreite() || r.left() > width()) continue;  // aus dem Fenster gerollt
            const bool gewaehlt = istMarkiert(k, c);
            const QColor farbe = basisFarbe(clip.muster, kanal.an);
            p.setPen(gewaehlt ? QPen(pal.color(QPalette::Highlight), 2.0) : QPen(farbe.darker(150), 1.0));
            p.setBrush(farbe);
            // hineingezoomt kann ein Clip viele tausend Pixel breit sein: gezeichnet
            // wird nur, was im Fenster liegt (und etwas mehr, fuer die runden Ecken)
            p.drawRoundedRect(
                r.intersected(QRect(kopfBreite() - 12, r.top(), width() - kopfBreite() + 24, r.height())), 4, 4);
            p.setBrush(Qt::NoBrush);
            // Stufe 28e (Skizze §1a): die Expression als Kurve auf dem Clip, unter dem Namen
            zeichneAusdruck(p, k, c, r);

            // die Wiederholungen: ein gestrichelter Strich an jeder Mustergrenze
            const Muster* basis = m_loop.findeMuster(clip.muster);
            if (basis != nullptr && clip.wiederholungen > 1)
            {
                p.setPen(QPen(QColor(255, 255, 255, 150), 1.0, Qt::DashLine));
                for (int mal = 1; mal < clip.wiederholungen; ++mal)
                {
                    const int x = xVonTakt(clip.startTakt + mal * basis->takte);
                    p.drawLine(x, r.top() + 2, x, r.bottom() - 1);
                }
            }

            p.setPen(Qt::white);
            // der Name bleibt am linken Rand des Fensters stehen, wenn der
            // Anfang des Clips hinausgerollt ist
            QRect namensFeld = r.adjusted(6, 0, -4, 0);
            namensFeld.setLeft(std::max(r.left(), kopfBreite()) + 6);
            int frei = r.right() - 3;  // bis hierher ist rechts Platz fuer Marken
            // die Marke „fx": an diesem Clip ist ein Effekt eingeschaltet (Stufe 12);
            // ist sein Schalter aus (E1 a), steht sie matt da — die Reihe bleibt
            if (clip.effekte.einerAn() && r.width() >= 48 && namePasst(r.height()))
            {
                const QString marke = QStringLiteral("fx");
                const int breit = fontMetrics().horizontalAdvance(marke) + 8;
                const int markeHoch = std::min(r.height() - 4, fontMetrics().height() + 2);
                const QRect feld(r.right() - breit - 3, r.center().y() - markeHoch / 2, breit, markeHoch);
                QColor schrift = Qt::white;
                if (!clip.effekteAn) schrift.setAlpha(90);
                p.setPen(QPen(schrift, 1.0, clip.effekteAn ? Qt::SolidLine : Qt::DotLine));
                p.setBrush(QColor(0, 0, 0, clip.effekteAn ? 70 : 30));
                p.drawRoundedRect(feld, 3, 3);
                p.setBrush(Qt::NoBrush);
                p.drawText(feld, Qt::AlignCenter, marke);
                namensFeld.setRight(feld.left() - 4);
                frei = feld.left() - 4;
                p.setPen(Qt::white);  // der Name bleibt hell, auch neben einer matten Marke
            }
            // die Marke Freeze (Stufe 30g, FZ4 a A): links vom „fx" — durchgezogen eingefroren,
            // gepunktet beim Rechnen, gestrichelt und matter, wenn der Clip live spielt; ist kein
            // Platz fuer den Text, nur die Flocke (der Text steht im Tooltip)
            const auto fm = m_freeze.find({k, clip.startTakt});
            // die Sanduhr (Stufe 30h, FZ5 a A): an der Stelle der Marke, nur ohne Marke; ohne Platz
            // fuer die Zahl nur die Sanduhr
            if (fm != m_freeze.end() && fm->second.zustand == FrozenZustand::Aus && fm->second.zeigeZeit &&
                namePasst(r.height()))
            {
                const int links = std::max(r.left(), kopfBreite()) + 6;
                const int namePlatz = std::min(fontMetrics().horizontalAdvance(clip.muster) + 8, 60);
                QString text = zeitText(fm->second);
                if (fontMetrics().horizontalAdvance(text) + 8 > frei - links - namePlatz) text = QString(QChar(0x23F3));
                const int breit = fontMetrics().horizontalAdvance(text) + 8;
                if (breit <= frei - links)
                {
                    const int markeHoch = std::min(r.height() - 4, fontMetrics().height() + 2);
                    const QRect feld(frei - breit, r.center().y() - markeHoch / 2, breit, markeHoch);
                    p.setPen(Qt::NoPen);
                    p.setBrush(QColor(0, 0, 0, 70));
                    p.drawRoundedRect(feld, 3, 3);
                    p.setBrush(Qt::NoBrush);
                    p.setPen(Qt::white);
                    p.drawText(feld, Qt::AlignCenter, text);
                    namensFeld.setRight(std::min(namensFeld.right(), feld.left() - 4));
                }
            }
            if (fm != m_freeze.end() && fm->second.zustand != FrozenZustand::Aus && namePasst(r.height()))
            {
                const FreezeMarke& marke = fm->second;
                const int links = std::max(r.left(), kopfBreite()) + 6;
                QString text = freezeText(marke);
                // dem Namen bleibt Platz fuer ein Stueck (hoechstens 60 px), der Rest gehoert der Marke
                const int namePlatz = std::min(fontMetrics().horizontalAdvance(clip.muster) + 8, 60);
                const bool kurz = fontMetrics().horizontalAdvance(text) + 8 > frei - links - namePlatz;
                if (kurz)
                    text = marke.rechnet ? QStringLiteral("%1 %").arg(std::lround(100.0 * marke.anteil))
                                         : QString(QChar(0x2744));
                const bool live = !marke.rechnet && marke.zustand != FrozenZustand::Frozen;
                // kurz und live (veraendert, fehlt): die Flocke in einem Warndreieck (Patrik S144) —
                // im Quadrat sieht sie aus wie eingefroren
                const bool dreieck = kurz && live;
                const int markeHoch = std::min(r.height() - 4, fontMetrics().height() + 2);
                const int breit = dreieck ? markeHoch + markeHoch / 3 : fontMetrics().horizontalAdvance(text) + 8;
                if (breit <= frei - links)
                {
                    const QRect feld(frei - breit, r.center().y() - markeHoch / 2, breit, markeHoch);
                    QColor schrift = Qt::white;
                    if (live && !dreieck) schrift.setAlpha(170);
                    p.setPen(QPen(schrift, 1.0,
                                  marke.rechnet ? Qt::DotLine : live && !dreieck ? Qt::DashLine : Qt::SolidLine));
                    p.setBrush(QColor(0, 0, 0, live ? 40 : 70));
                    if (dreieck)
                    {
                        const QPointF spitze(feld.center().x() + 0.5, feld.top() + 0.5);
                        const QPolygonF form({spitze, QPointF(feld.right() + 0.5, feld.bottom() + 0.5),
                                              QPointF(feld.left() + 0.5, feld.bottom() + 0.5)});
                        p.drawPolygon(form);
                        p.setBrush(Qt::NoBrush);
                        // die Flocke im unteren, breiteren Teil des Dreiecks, etwas kleiner
                        QFont klein = p.font();
                        if (klein.pointSizeF() > 0.0)
                            klein.setPointSizeF(klein.pointSizeF() * 0.8);
                        else
                            klein.setPixelSize(std::max(6, klein.pixelSize() * 4 / 5));
                        p.setFont(klein);
                        p.drawText(feld.adjusted(0, markeHoch / 4, 0, 0), Qt::AlignCenter, text);
                        p.setFont(font());
                    }
                    else
                    {
                        p.drawRoundedRect(feld, 3, 3);
                        p.setBrush(Qt::NoBrush);
                        p.drawText(feld, Qt::AlignCenter, text);
                    }
                    namensFeld.setRight(std::min(namensFeld.right(), feld.left() - 4));
                    p.setPen(Qt::white);
                }
            }
            const QString beschriftung = clip.wiederholungen > 1
                                             ? tr("%1 × %2").arg(clip.muster).arg(clip.wiederholungen)
                                             : clip.muster;
            if (namePasst(r.height()))
            {
                const QString aufschrift =
                    fontMetrics().elidedText(beschriftung, Qt::ElideRight, std::max(0, namensFeld.width()));
                // Stufe 28e: kreuzt die Kurve den Namen, bleibt er auf einem leichten Grund lesbar
                if (!clip.expression.empty() && !aufschrift.isEmpty())
                {
                    const QRect grund =
                        p.boundingRect(namensFeld, Qt::AlignLeft | Qt::AlignVCenter, aufschrift).adjusted(-3, 0, 3, 0);
                    p.setPen(Qt::NoPen);
                    p.setBrush(QColor(0, 0, 0, 70));
                    p.drawRoundedRect(grund, 3, 3);
                    p.setBrush(Qt::NoBrush);
                    p.setPen(Qt::white);
                }
                p.drawText(namensFeld, Qt::AlignLeft | Qt::AlignVCenter, aufschrift);
            }
        }
    }
    p.setClipRect(ausschnitte.zeilen);
    p.setPen(linie);
    p.drawLine(kopfBreite() - 1, 0, kopfBreite() - 1, unten);
    // --- ein Kanal wird am Namen gezogen (7b a, wie im Composer): seine Zeile
    //     hervorgehoben, ein Strich an der Kante der Zeile, an der er landet
    if (m_kanalZug >= 0 && m_kanalZiel >= 0 && m_kanalZug < kanaele)
    {
        QColor hell = pal.color(QPalette::Highlight);
        hell.setAlpha(45);
        p.fillRect(QRect(0, zeileOben(m_kanalZug), width(), zeileHoch(m_kanalZug)), hell);
        if (m_kanalZiel != m_kanalZug)
        {
            // nach unten gezogen landet er UNTER der Zielzeile, nach oben ueber ihr
            const int y = zeileOben(m_kanalZiel + (m_kanalZiel > m_kanalZug ? 1 : 0));
            p.setPen(QPen(pal.color(QPalette::Highlight), 3.0));
            p.drawLine(0, y, width(), y);
        }
    }
    p.setClipRect(ausschnitte.zeilenZeit);
    if (m_griff == Griff::Rahmen && m_zugLaeuft)
    {
        // der Rahmen, der Clips markiert
        QColor flaeche = pal.color(QPalette::Highlight);
        p.setPen(QPen(flaeche, 1.0, Qt::DashLine));
        flaeche.setAlpha(40);
        p.setBrush(flaeche);
        p.drawRect(m_rahmen);
        p.setBrush(Qt::NoBrush);
    }
    auflage();

    // --- zweiter Gang: das Lineal, ueber allem, was darunter gerollt ist
    p.setClipRect(ausschnitte.lineal);
    p.fillRect(ausschnitte.lineal, pal.color(QPalette::AlternateBase));
    p.setClipRect(ausschnitte.linealZeit);
    unterlage();
    p.setClipRect(ausschnitte.lineal);
    p.setPen(linie);
    p.drawLine(kopfBreite() - 1, 0, kopfBreite() - 1, unten);
    p.setClipRect(ausschnitte.linealZeit);
    auflage();
}

Taktachse::Griff Taktachse::griffBei(QPoint ort, int* kanal, int* clip) const
{
    *kanal = -1;
    *clip = -1;
    if (!m_geladen || ort.x() < kopfBreite()) return Griff::Keiner;
    const int k = kanalBei(ort.y());
    if (k < 0) return Griff::Keiner;
    const auto& clips = m_loop.kanaele[static_cast<std::size_t>(k)].clips;
    for (int c = 0; c < static_cast<int>(clips.size()); ++c)
    {
        const QRect r = clipRechteck(k, c);
        if (!r.contains(ort)) continue;
        *kanal = k;
        *clip = c;
        // ein schmaler Clip laesst sich nur verschieben: sonst traefe man ihn nie in der Mitte
        if (r.width() >= 4 * kKantenNaehe)
        {
            if (ort.x() - r.left() < kKantenNaehe) return Griff::KanteLinks;
            if (r.right() - ort.x() < kKantenNaehe) return Griff::KanteRechts;
        }
        return Griff::Verschieben;
    }
    return Griff::Keiner;
}

void Taktachse::mousePressEvent(QMouseEvent* event)
{
    merkeTaste(true);  // so lange laeuft der Ausschnitt nicht mit
    if (m_geladen && event->button() == Qt::MiddleButton && m_griff == Griff::Keiner)
    {
        beginneSchwenken(event->position().toPoint());
        return;
    }
    if (!m_geladen || event->button() != Qt::LeftButton) return;
    const QPoint ort = event->position().toPoint();
    if (ort.y() < kLinealHoehe)
    {
        // B4 (S124): ein Klick ins Lineal waehlt die Stelle — dorthin springen oder dort beginnen
        m_imLineal = true;
        Q_EMIT stelleGewaehlt(schrittBeiX(ort.x()));
        return;
    }
    // am Fader (27e-2, LL12 a): Ctrl+Klick setzt 0 dB, sonst zieht die Maus ihn senkrecht
    if (const int fader = faderBei(ort); fader >= 0)
    {
        if ((event->modifiers() & Qt::ControlModifier) != 0)
        {
            kanalGain(fader, 0.0);
            return;
        }
        m_faderZug = fader;
        m_faderStartY = ort.y();
        m_faderStartDb = m_loop.kanaele[static_cast<std::size_t>(fader)].gainDb;
        m_vorZug = m_loop;
        return;
    }
    // am Kanalnamen (7b a): der Kanal laesst sich an einen anderen Platz ziehen —
    // vorbereitet wird es hier, gezogen erst, wenn die Maus sich bewegt
    if (ort.x() < kopfBreite() && kanalBei(ort.y()) >= 0)
    {
        m_kanalZug = kanalBei(ort.y());
        m_kanalZiel = -1;
        m_kanalZugStart = ort;
        return;
    }
    int kanal = -1;
    int clip = -1;
    const Griff griff = griffBei(ort, &kanal, &clip);
    const bool ctrl = (event->modifiers() & Qt::ControlModifier) != 0;
    m_klickSammelt = false;
    // Stufe 28e (SK13 a): am angeklickten Clip einen Punkt der Expression greifen — oder auf
    // Linie oder Kurve einen setzen und weiterziehen; die Kanten gehen vor
    if (!ctrl && griff == Griff::Verschieben && kurveGreifbar(kanal, clip))
    {
        int punkt = ausdruckPunktBei(kanal, clip, ort);
        const bool neu = punkt < 0;
        if (neu && aufAusdruck(kanal, clip, ort))
        {
            m_vorZug = m_loop;
            LoopClip& c = m_loop.kanaele[static_cast<std::size_t>(kanal)].clips[static_cast<std::size_t>(clip)];
            const bool frei = (event->modifiers() & Qt::AltModifier) != 0;
            punkt = legeAusdruckPunkt(c, {clipStelleBei(c, ort.x(), frei), ausdruckBei(clipRechteck(kanal, clip), ort.y())});
            if (punkt < 0) m_loop = m_vorZug;
        }
        else if (!neu)
            m_vorZug = m_loop;
        if (punkt >= 0)
        {
            m_griff = Griff::Ausdruck;
            m_ausdruckIndex = punkt;
            m_ausdruckNeu = neu;
            m_zugKanal = kanal;
            m_zugClip = clip;
            m_zugLaeuft = false;
            update();
            return;
        }
    }
    if (griff == Griff::Keiner)
    {
        // freie Flaeche: von hier laesst sich ein Rahmen aufziehen; mit Ctrl
        // kommt, was er trifft, zu den Marken dazu
        if (ort.x() >= kopfBreite())
        {
            m_griff = Griff::Rahmen;
            m_zugLaeuft = false;
            m_zugStart = ort;
            m_rahmen = {};
            m_vorZug = m_loop;
            m_vorGruppe = ctrl ? m_gruppe : std::vector<ClipOrt>{};
        }
        if (!ctrl) waehle(-1, -1);
        return;
    }
    if (ctrl)
    {
        markiereUm(kanal, clip);  // dazu oder heraus; gezogen wird dabei nichts
        return;
    }
    // Ein Druck auf einen von mehreren markierten Clips laesst die Marken
    // stehen: so laesst sich die Gruppe ziehen. Eine Kante gehoert immer nur
    // einem Clip.
    if (griff == Griff::Verschieben && m_gruppe.size() > 1 && istMarkiert(kanal, clip))
    {
        m_klickSammelt = true;
        m_kanal = kanal;
        m_clip = clip;
        update();
        Q_EMIT auswahlGeaendert();
    }
    else
        waehle(kanal, clip);
    m_griff = griff;
    m_zugLaeuft = false;
    m_zugStart = ort;
    m_zugKanal = kanal;
    m_zugClip = clip;
    m_vorZug = m_loop;
    m_vorGruppe = m_gruppe;
    m_gutKanal = 0;
    m_gutTakt = 0;
    m_zugVersatz = stelleVon(ort.x()) -
                   m_loop.kanaele[static_cast<std::size_t>(kanal)].clips[static_cast<std::size_t>(clip)].startTakt *
                       kTicksJeTakt;
}

void Taktachse::zieheZu(QPoint ort)
{
    if (m_griff == Griff::Rahmen)
    {
        m_rahmen = QRect(m_zugStart, ort).normalized();
        update();
        return;
    }
    const std::int64_t maus = stelleVon(ort.x());
    if (m_griff == Griff::Verschieben && m_vorGruppe.size() > 1)
    {
        // Die Gruppe als Ganzes: alle um dieselben Takte und Kanaele. Ist an
        // der Stelle unter der Maus kein Platz, bleibt sie an der letzten, an
        // der Platz war.
        const auto& vorher = m_vorZug.kanaele[static_cast<std::size_t>(m_zugKanal)].clips;
        const ClipOrt gezogen{m_zugKanal, vorher[static_cast<std::size_t>(m_zugClip)].startTakt};
        const int kanaele = static_cast<int>(m_vorZug.kanaele.size());
        const int kanalVersatz = std::clamp(zeileBei(ort.y()), 0, kanaele - 1) - m_zugKanal;
        const int taktVersatz = naechsterTakt(maus - m_zugVersatz) - gezogen.startTakt;
        const auto versuche = [this](int kanalV, int taktV) {
            Loop probe = m_vorZug;
            std::vector<ClipOrt> orte = m_vorGruppe;
            if (kanalV == 0 && taktV == 0)
            {
                m_loop = m_vorZug;
                m_gruppe = m_vorGruppe;
                return true;
            }
            if (!versetzeClips(probe, orte, kanalV, taktV)) return false;
            m_loop = std::move(probe);
            m_gruppe = std::move(orte);
            return true;
        };
        if (versuche(kanalVersatz, taktVersatz))
        {
            m_gutKanal = kanalVersatz;
            m_gutTakt = taktVersatz;
        }
        else
            (void)versuche(m_gutKanal, m_gutTakt);
        // der gezogene Clip bleibt der angeklickte
        const auto nr = std::find(m_vorGruppe.begin(), m_vorGruppe.end(), gezogen) - m_vorGruppe.begin();
        if (nr < static_cast<std::ptrdiff_t>(m_gruppe.size()))
        {
            m_kanal = m_gruppe[static_cast<std::size_t>(nr)].kanal;
            m_clip = findeClip(m_loop, m_gruppe[static_cast<std::size_t>(nr)]);
        }
        update();
        return;
    }
    m_loop = m_vorZug;
    int ziel = m_zugKanal;
    int platz = m_zugClip;
    switch (m_griff)
    {
        case Griff::Verschieben:
        {
            // die Zeile unter der Maus: der Clip wechselt den Kanal
            const int kanaele = static_cast<int>(m_vorZug.kanaele.size());
            const int wunschKanal = std::clamp(zeileBei(ort.y()), 0, kanaele - 1);
            const int neu = versetzeClip(m_loop, m_zugKanal, m_zugClip, wunschKanal, naechsterTakt(maus - m_zugVersatz));
            if (neu >= 0)
            {
                ziel = wunschKanal;
                platz = neu;
            }
            break;
        }
        case Griff::KanteLinks:
            platz = setzeAnfang(m_loop, m_zugKanal, m_zugClip, naechsterTakt(maus));
            break;
        case Griff::KanteRechts:
            platz = setzeEnde(m_loop, m_zugKanal, m_zugClip, naechsterTakt(maus));
            break;
        case Griff::Rahmen:
        case Griff::Ausdruck:  // zieht `zieheAusdruck`
        case Griff::Keiner:
            return;
    }
    m_kanal = ziel;
    m_clip = platz;
    m_gruppe.clear();
    richteGruppe();
    // die Laenge der Achse bleibt im Zug, wie sie war: aenderte sich der
    // Massstab unter der Maus, liefe der Clip ihr davon. Neu gemessen wird
    // beim Loslassen.
    update();
}

void Taktachse::brichZugAb()
{
    if (m_griff == Griff::Keiner) return;
    if (m_griff == Griff::Rahmen)
        m_rahmen = {};
    else if (m_griff == Griff::Ausdruck)
    {
        m_loop = m_vorZug;  // Stufe 28e: der Punkt geht zurueck (ein neuer verschwindet)
        m_ausdruckIndex = -1;
    }
    else if (m_zugLaeuft)
    {
        m_loop = m_vorZug;
        m_kanal = m_zugKanal;
        m_clip = m_zugClip;
        m_gruppe = m_vorGruppe;
    }
    m_griff = Griff::Keiner;
    m_zugLaeuft = false;
    m_klickSammelt = false;
    update();
}

void Taktachse::mouseMoveEvent(QMouseEvent* event)
{
    const QPoint ort = event->position().toPoint();
    if (m_imLineal && (event->buttons() & Qt::LeftButton))
    {
        Q_EMIT stelleGewaehlt(schrittBeiX(ort.x()));  // Ziehen im Lineal: die Stelle folgt
        return;
    }
    if (schwenkt())
    {
        schwenkeZu(ort);
        return;
    }
    if (m_faderZug >= 0 && (event->buttons() & Qt::LeftButton))
    {
        // 0,1 dB je Pixel in Schritten von 0,5 dB; mit Shift 0,02 dB je Pixel in Schritten von 0,1 dB
        const bool fein = (event->modifiers() & Qt::ShiftModifier) != 0;
        const double schritt = fein ? 0.1 : 0.5;
        const double roh = m_faderStartDb + (m_faderStartY - ort.y()) * (fein ? 0.02 : 0.1);
        const double db = std::clamp(std::round(roh / schritt) * schritt, kKanalGainMinDb, kKanalGainMaxDb);
        m_loop = m_vorZug;
        m_loop.kanaele[static_cast<std::size_t>(m_faderZug)].gainDb = db;
        setCursor(Qt::SizeVerCursor);
        update();
        Q_EMIT hinweis(tr("Channel gain: %1").arg(faderText(db)));
        return;
    }
    if (m_kanalZug >= 0 && (event->buttons() & Qt::LeftButton))
    {
        // ein Kanal wird am Namen gezogen: er fiele auf die Zeile unter der Maus
        if (m_kanalZiel < 0 && (ort - m_kanalZugStart).manhattanLength() < 4) return;
        const int zeilen = static_cast<int>(m_loop.kanaele.size());
        m_kanalZiel = std::clamp(zeileBei(ort.y()), 0, std::max(0, zeilen - 1));
        setCursor(Qt::ClosedHandCursor);
        update();
        return;
    }
    if (m_griff == Griff::Ausdruck && (event->buttons() & Qt::LeftButton))
    {
        zieheAusdruck(ort, (event->modifiers() & Qt::AltModifier) != 0);
        return;
    }
    if (m_griff != Griff::Keiner && (event->buttons() & Qt::LeftButton))
    {
        if (!m_zugLaeuft && (ort - m_zugStart).manhattanLength() < 4) return;
        m_zugLaeuft = true;
        zieheZu(ort);
        return;
    }
    if (!m_geladen) return;
    int kanal = -1;
    int clip = -1;
    const Griff griff = griffBei(ort, &kanal, &clip);
    const int fader = faderBei(ort);
    setzeFaderUnterMaus(fader);
    // Stufe 28e (SK13 a): an der Kurve des angeklickten Clips sagen Zeiger und Hinweis, was geht
    const int punkt = griff == Griff::Verschieben ? ausdruckPunktBei(kanal, clip, ort) : -1;
    const bool linie = punkt < 0 && griff == Griff::Verschieben && aufAusdruck(kanal, clip, ort);
    if (punkt >= 0 || linie)
    {
        const LoopClip& c = m_loop.kanaele[static_cast<std::size_t>(kanal)].clips[static_cast<std::size_t>(clip)];
        const double wert = punkt >= 0 ? c.expression[static_cast<std::size_t>(punkt)].wert : 1.0;
        Q_EMIT hinweis(punkt >= 0 ? tr("Expression %1 dB — drag to move, double-click to delete")
                                        .arg(wert <= 0.0 ? tr("silent") : QString::number(expressionDb(wert), 'f', 1))
                                  : tr("Click to add an expression point (Alt: off the grid); right-click: Clear expression"));
        m_hinweisAusdruck = true;
        setCursor(punkt >= 0 ? Qt::SizeAllCursor : Qt::CrossCursor);
        return;
    }
    if (m_hinweisAusdruck && fader < 0)
    {
        m_hinweisAusdruck = false;
        Q_EMIT hinweis(QString());
    }
    if (fader >= 0)
        setCursor(Qt::SizeVerCursor);  // am Fader: senkrecht ziehen
    else if (ort.x() < kopfBreite() && kanalBei(ort.y()) >= 0)
        setCursor(Qt::OpenHandCursor);  // am Kanalnamen: der Kanal laesst sich verschieben
    else if (griff == Griff::KanteLinks || griff == Griff::KanteRechts)
        setCursor(Qt::SizeHorCursor);
    else
        unsetCursor();
}

void Taktachse::mouseReleaseEvent(QMouseEvent* event)
{
    merkeTaste(event->buttons() != Qt::NoButton);
    if (m_faderZug >= 0 && event->button() == Qt::LeftButton)
    {
        // ein Zug am Fader ist ein Undo-Schritt, wie weit er auch ging
        const Loop gezogen = m_loop;
        m_loop = m_vorZug;
        m_faderZug = -1;
        unsetCursor();
        Q_EMIT hinweis(faderBei(event->position().toPoint()) >= 0 ? faderHinweis() : QString());
        if (gezogen != m_vorZug)
            uebernimm(gezogen, m_kanal, m_clip, tr("Channel gain"));
        else
            update();
        return;
    }
    if (m_imLineal && event->button() == Qt::LeftButton)
    {
        m_imLineal = false;
        return;
    }
    if (schwenkt())
    {
        if (event->button() != Qt::MiddleButton) return;  // eine andere Taste: das Schwenken laeuft weiter
        beendeSchwenken();
        return;
    }
    if (m_kanalZug >= 0 && event->button() == Qt::LeftButton)
    {
        // der Kanal faellt auf seinen neuen Platz — ohne Bewegung war es nur ein Klick
        const int von = m_kanalZug;
        const int nach = m_kanalZiel;
        m_kanalZug = -1;
        m_kanalZiel = -1;
        unsetCursor();
        update();
        if (nach >= 0 && nach != von) kanalVerschieben(von, nach);
        return;
    }
    if (m_griff == Griff::Ausdruck && event->button() == Qt::LeftButton)
    {
        // Stufe 28e: ein gesetzter oder gezogener Punkt — ein Undo-Schritt
        const Loop gezogen = m_loop;
        m_loop = m_vorZug;
        m_griff = Griff::Keiner;
        m_ausdruckIndex = -1;
        if (gezogen != m_vorZug)
            uebernimm(gezogen, m_zugKanal, m_zugClip, m_ausdruckNeu ? tr("Add expression point") : tr("Move expression point"));
        else
            update();
        return;
    }
    if (m_griff == Griff::Keiner) return;
    const Griff griff = m_griff;
    const bool gezogen = m_zugLaeuft;
    const bool sammelt = m_klickSammelt;
    m_griff = Griff::Keiner;
    m_zugLaeuft = false;
    m_klickSammelt = false;
    if (griff == Griff::Rahmen)
    {
        if (gezogen)
        {
            // was der Rahmen beruehrt — mit Ctrl zu dem dazu, was schon markiert war
            std::vector<ClipOrt> orte = m_vorGruppe;
            for (const ClipOrt& ort : clipsIm(m_rahmen)) orte.push_back(ort);
            m_rahmen = {};
            markiere(orte);
        }
        update();
        return;
    }
    if (!gezogen)
    {
        // ein Klick ohne Zug auf einen von mehreren markierten: nur er bleibt
        if (sammelt) waehle(m_kanal, m_clip);
        return;
    }
    setzeLaenge(anzeigeTakte(m_loop) * kTicksJeTakt);
    passeRollenAn();
    update();
    if (m_loop == m_vorZug) return;
    Q_EMIT auswahlGeaendert();  // der Clip hat neue Takte
    Q_EMIT bearbeitet(m_loop, griff != Griff::Verschieben ? tr("Resize clip")
                              : m_gruppe.size() > 1       ? tr("Move clips")
                                                          : tr("Move clip"));
}

void Taktachse::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape && m_kanalZug >= 0)
    {
        // einen Kanal gezogen: Esc laesst ihn, wo er war
        m_kanalZug = -1;
        m_kanalZiel = -1;
        unsetCursor();
        update();
        return;
    }
    if (event->key() == Qt::Key_Escape && m_griff != Griff::Keiner)
    {
        brichZugAb();
        return;
    }
    if (m_griff == Griff::Keiner && m_geladen && event->key() == Qt::Key_A &&
        event->modifiers() == Qt::ControlModifier)
    {
        markiereAlle();
        return;
    }
    if (m_griff == Griff::Keiner && auswahl() != nullptr)
    {
        if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)
        {
            (void)loescheAuswahl();
            return;
        }
        if (event->key() == Qt::Key_D && event->modifiers() == Qt::ControlModifier)
        {
            (void)dupliziereAuswahl();
            return;
        }
    }
    QWidget::keyPressEvent(event);
}

void Taktachse::uebernimm(const Loop& neu, int kanal, int clip, const QString& was)
{
    m_loop = neu;
    setzeLaenge(anzeigeTakte(neu) * kTicksJeTakt);
    m_kanal = kanal;
    m_clip = clip;
    richteGruppe();
    updateGeometry();
    passeRollenAn();
    update();
    Q_EMIT auswahlGeaendert();
    Q_EMIT bearbeitet(neu, was);
}

bool Taktachse::neuerClip(int kanal, int takt, MusterArt art)
{
    if (!m_geladen || m_griff != Griff::Keiner) return false;
    Loop neu = m_loop;
    const int platz = lumi::loop::neuerClip(neu, kanal, takt, art);
    if (platz < 0) return false;
    m_gruppe.clear();  // markiert ist danach der neue Clip
    uebernimm(neu, kanal, platz, tr("New clip"));
    return true;
}

bool Taktachse::dupliziereAuswahl()
{
    if (auswahl() == nullptr || m_griff != Griff::Keiner) return false;
    Loop neu = m_loop;
    if (m_gruppe.size() > 1)
    {
        // die Gruppe als Ganzes; markiert sind danach die Kopien
        std::vector<ClipOrt> orte = m_gruppe;
        if (!dupliziereClips(neu, orte)) return false;
        m_gruppe = orte;
        uebernimm(neu, orte.front().kanal, findeClip(neu, orte.front()), tr("Duplicate clips"));
        return true;
    }
    const int platz = dupliziere(neu, m_kanal, m_clip);
    if (platz < 0) return false;
    m_gruppe.clear();
    uebernimm(neu, m_kanal, platz, tr("Duplicate clip"));
    return true;
}

bool Taktachse::loescheAuswahl()
{
    if (auswahl() == nullptr || m_griff != Griff::Keiner) return false;
    Loop neu = m_loop;
    if (m_gruppe.size() > 1)
    {
        if (!loescheClips(neu, m_gruppe)) return false;
        m_gruppe.clear();
        uebernimm(neu, -1, -1, tr("Delete clips"));
        return true;
    }
    if (!loescheClip(neu, m_kanal, m_clip)) return false;
    m_gruppe.clear();
    uebernimm(neu, -1, -1, tr("Delete clip"));
    return true;
}

bool Taktachse::benenneBasisUm(const QString& name)
{
    const LoopClip* clip = auswahl();
    if (clip == nullptr || m_griff != Griff::Keiner) return false;
    Loop neu = m_loop;
    if (!lumi::loop::benenneBasisUm(neu, clip->muster, name)) return false;
    uebernimm(neu, m_kanal, m_clip, tr("Rename base"));
    return true;
}

bool Taktachse::bearbeiteBasis()
{
    const LoopClip* clip = auswahl();
    if (clip == nullptr || m_griff != Griff::Keiner) return false;
    Q_EMIT editorVerlangt(clip->muster);
    return true;
}

bool Taktachse::neueBasisUndBearbeiten()
{
    const LoopClip* clip = auswahl();
    if (clip == nullptr || m_griff != Griff::Keiner) return false;
    // spielt nur dieser Clip die Basis, gibt es nichts zu loesen: sie ist schon seine eigene
    if (basisNutzer(m_loop, clip->muster) <= 1) return bearbeiteBasis();
    Loop neu = m_loop;
    const QString name = neueBasisAus(neu, m_kanal, m_clip);
    if (name.isEmpty()) return false;
    uebernimm(neu, m_kanal, m_clip, tr("Make new base"));
    Q_EMIT editorVerlangt(name);
    return true;
}

bool Taktachse::verlangeEffekte()
{
    if (m_gruppe.empty() || m_griff != Griff::Keiner) return false;
    Q_EMIT effekteVerlangt();
    return true;
}

bool Taktachse::verlangeExport()
{
    const LoopClip* clip = auswahl();
    if (clip == nullptr || m_gruppe.size() > 1 || m_griff != Griff::Keiner) return false;
    Q_EMIT exportVerlangt(m_kanal, clip->startTakt);
    return true;
}

bool Taktachse::verlangeFreeze(bool einfrieren)
{
    if (m_gruppe.empty() || m_griff != Griff::Keiner) return false;
    Q_EMIT freezeVerlangt(einfrieren);
    return true;
}

void Taktachse::setzeFreezeMarken(std::map<std::pair<int, int>, FreezeMarke> marken)
{
    if (marken == m_freeze) return;
    m_freeze = std::move(marken);
    update();
}

FreezeMarke Taktachse::freezeMarke(int kanal, int startTakt) const
{
    const auto ort = m_freeze.find({kanal, startTakt});
    return ort == m_freeze.end() ? FreezeMarke{} : ort->second;
}

QString Taktachse::freezeText(const FreezeMarke& marke)
{
    if (marke.rechnet) return tr("computing… %1 %").arg(std::lround(100.0 * marke.anteil));
    const QString flocke(QChar(0x2744));
    switch (marke.zustand)
    {
    case FrozenZustand::Aus: return {};
    case FrozenZustand::Frozen: return tr("%1 frozen").arg(flocke);
    case FrozenZustand::Veraendert: return tr("%1 frozen · changed").arg(flocke);
    case FrozenZustand::Fehlt: return tr("%1 frozen · missing").arg(flocke);
    }
    return {};
}

QString Taktachse::zeitText(const FreezeMarke& marke)
{
    if (!marke.zeigeZeit || marke.zustand != FrozenZustand::Aus) return {};
    return tr("%1 %2 s").arg(QChar(0x23F3)).arg(marke.gezeigtS(), 0, 'f', 1);
}

QString Taktachse::freezeTooltip(const FreezeMarke& marke)
{
    if (marke.zustand == FrozenZustand::Aus && marke.zeigeZeit && !marke.rechnet)
        return tr("Computing this clip takes %1 s of work (about %2 s waiting on %3 threads) — "
                  "Freeze clip to start it at once")
            .arg(marke.arbeitMs / 1000.0, 0, 'f', 1)
            .arg(marke.arbeitMs / 1000.0 / std::max(1, marke.threads), 0, 'f', 1)
            .arg(std::max(1, marke.threads));
    if (marke.rechnet)
        return tr("Freezing this clip — it plays live until the waveform is ready");
    switch (marke.zustand)
    {
    case FrozenZustand::Aus: return {};
    case FrozenZustand::Frozen: return tr("Frozen: this clip plays its computed waveform");
    case FrozenZustand::Veraendert:
        return tr("Frozen · changed: edited since it was frozen — it plays live and is frozen again when you save");
    case FrozenZustand::Fehlt: return tr("Frozen · missing: the waveform is computed again");
    }
    return {};
}

bool Taktachse::event(QEvent* event)
{
    if (event->type() == QEvent::ToolTip)
    {
        const auto* hilfe = static_cast<QHelpEvent*>(event);
        for (const ClipOrt& ort : clipsIm(QRect(hilfe->pos(), QSize(1, 1))))
        {
            const QString text = freezeTooltip(freezeMarke(ort.kanal, ort.startTakt));
            if (text.isEmpty()) continue;
            QToolTip::showText(hilfe->globalPos(), text, this);
            return true;
        }
        QToolTip::hideText();
        event->ignore();
        return true;
    }
    return Achse::event(event);
}

void Taktachse::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (!m_geladen || event->button() != Qt::LeftButton) return;
    if (const int fader = faderBei(event->position().toPoint()); fader >= 0)
    {
        // am Fader (27e-2): den Wert tippen — „-6.5", „-6,5", „+3", „-inf"
        m_faderZug = -1;
        bool ok = false;
        const double jetzt = m_loop.kanaele[static_cast<std::size_t>(fader)].gainDb;
        const QString vorgabe = jetzt <= kKanalGainMinDb ? QStringLiteral("-inf") : QString::number(jetzt, 'f', 1);
        const QString eingabe =
            lumi::ui::frageText(this, tr("Channel gain"),
                                tr("Gain of %1 in dB, %2 to +%3 (%2 = silent):")
                                    .arg(m_loop.kanaele[static_cast<std::size_t>(fader)].name)
                                    .arg(kKanalGainMinDb, 0, 'f', 0)
                                    .arg(kKanalGainMaxDb, 0, 'f', 0),
                                vorgabe, &ok)
                .trimmed();
        if (!ok) return;
        QString zahl = eingabe;
        zahl.replace(QLatin1Char(','), QLatin1Char('.')).replace(QChar(0x2212), QLatin1Char('-'));
        zahl.remove(QStringLiteral("dB"), Qt::CaseInsensitive);
        zahl = zahl.trimmed();
        if (zahl.compare(QStringLiteral("-inf"), Qt::CaseInsensitive) == 0 || zahl == QStringLiteral("-∞"))
        {
            kanalGain(fader, kKanalGainMinDb);
            return;
        }
        bool zahlOk = false;
        const double db = zahl.toDouble(&zahlOk);
        if (zahlOk) kanalGain(fader, db);
        return;
    }
    int kanal = -1;
    int clip = -1;
    if (griffBei(event->position().toPoint(), &kanal, &clip) == Griff::Keiner) return;
    // Stufe 28e (SK13 a): auf einem Punkt der Expression nimmt der Doppelklick ihn weg
    if (const int punkt = ausdruckPunktBei(kanal, clip, event->position().toPoint()); punkt >= 0)
    {
        brichZugAb();
        (void)loescheAusdruckPunkt(kanal, clip, punkt);
        return;
    }
    brichZugAb();  // der zweite Druck hat einen Zug vorbereitet
    waehle(kanal, clip);
    (void)bearbeiteBasis();
}

void Taktachse::kanalHinzu()
{
    if (!m_geladen || m_griff != Griff::Keiner) return;
    Loop neu = m_loop;
    (void)lumi::loop::kanalHinzu(neu);
    uebernimm(neu, m_kanal, m_clip, tr("Add channel"));
}

void Taktachse::kanalLoeschen(int kanal)
{
    if (!m_geladen || m_griff != Griff::Keiner) return;
    Loop neu = m_loop;
    if (!lumi::loop::kanalLoeschen(neu, kanal)) return;
    // die Marken folgen ihren Kanaelen; was im geloeschten lag, faellt
    m_gruppe.erase(std::remove_if(m_gruppe.begin(), m_gruppe.end(),
                                  [kanal](const ClipOrt& ort) { return ort.kanal == kanal; }),
                   m_gruppe.end());
    for (ClipOrt& ort : m_gruppe)
        if (ort.kanal > kanal) --ort.kanal;
    // die Auswahl folgt ihrem Kanal; lag sie im geloeschten, faellt sie
    const int gewaehlt = m_kanal == kanal ? -1 : m_kanal > kanal ? m_kanal - 1 : m_kanal;
    uebernimm(neu, gewaehlt, gewaehlt < 0 ? -1 : m_clip, tr("Delete channel"));
}

void Taktachse::kanalUmbenennen(int kanal, const QString& name)
{
    if (!m_geladen || m_griff != Griff::Keiner) return;
    Loop neu = m_loop;
    if (!lumi::loop::kanalUmbenennen(neu, kanal, name) || neu == m_loop) return;
    uebernimm(neu, m_kanal, m_clip, tr("Rename channel"));
}

void Taktachse::kanalVerschieben(int von, int nach)
{
    if (!m_geladen || m_griff != Griff::Keiner) return;
    Loop neu = m_loop;
    if (!lumi::loop::kanalVerschieben(neu, von, nach)) return;
    // Marken und Auswahl bleiben bei ihrem Kanal
    for (ClipOrt& ort : m_gruppe) ort.kanal = platzNachVerschieben(ort.kanal, von, nach);
    uebernimm(neu, m_kanal >= 0 ? platzNachVerschieben(m_kanal, von, nach) : m_kanal, m_clip, tr("Move channel"));
}

void Taktachse::kanalGain(int kanal, double db)
{
    if (!m_geladen || m_griff != Griff::Keiner || kanal < 0 || kanal >= static_cast<int>(m_loop.kanaele.size())) return;
    Loop neu = m_loop;
    neu.kanaele[static_cast<std::size_t>(kanal)].gainDb = std::clamp(db, kKanalGainMinDb, kKanalGainMaxDb);
    if (neu == m_loop) return;
    uebernimm(neu, m_kanal, m_clip, tr("Channel gain"));
}

QRect Taktachse::faderFeld(int kanal) const
{
    if (kanal < 0 || kanal >= static_cast<int>(m_loop.kanaele.size())) return {};
    const int hoch = zeileHoch(kanal);
    if (!namePasst(hoch)) return {};
    const QFontMetrics masse = fontMetrics();
    const int breit = masse.horizontalAdvance(QStringLiteral("−60.0 dB")) + 10;
    const int feldHoch = masse.height() + 4;
    return {kopfBreite() - breit - 8, zeileOben(kanal) + (hoch - feldHoch) / 2, breit, feldHoch};
}

int Taktachse::faderBei(QPoint ort) const
{
    if (!m_geladen || ort.x() >= kopfBreite() || ort.y() < kLinealHoehe) return -1;
    const int kanal = kanalBei(ort.y());
    return kanal >= 0 && faderFeld(kanal).contains(ort) ? kanal : -1;
}

QString Taktachse::faderText(double db)
{
    if (db <= kKanalGainMinDb) return QStringLiteral("−∞ dB");
    const double gerundet = std::round(db * 10.0) / 10.0;
    if (gerundet == 0.0) return QStringLiteral("0.0 dB");
    return (gerundet > 0.0 ? QStringLiteral("+") : QStringLiteral("−")) + QString::number(std::abs(gerundet), 'f', 1) +
           QStringLiteral(" dB");
}

QString Taktachse::faderHinweis()
{
    return tr("Channel gain · drag up or down or turn the wheel, with Shift in fine steps · double-click to type · "
              "Ctrl+click for 0 dB");
}

void Taktachse::setzeFaderUnterMaus(int kanal)
{
    if (kanal == m_faderUnterMaus) return;
    m_faderUnterMaus = kanal;
    update();
    Q_EMIT hinweis(kanal >= 0 ? faderHinweis() : QString());
}

void Taktachse::leaveEvent(QEvent* event)
{
    if (m_faderZug < 0) setzeFaderUnterMaus(-1);
    lumi::ui::Achse::leaveEvent(event);
}

void Taktachse::wheelEvent(QWheelEvent* event)
{
    // ueber dem Fader (27e-2): das Rad stellt ihn — 0,5 dB je Raste, mit Shift 0,1 dB
    const Qt::KeyboardModifiers tasten =
        event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier | Qt::AltModifier);
    const int fader = faderBei(event->position().toPoint());
    if (fader < 0 || (tasten != Qt::NoModifier && tasten != Qt::ShiftModifier))
    {
        lumi::ui::Achse::wheelEvent(event);
        return;
    }
    const QPoint grad = event->angleDelta();
    m_faderRadRest += grad.y() != 0 ? grad.y() : grad.x();  // mit Shift meldet das System das Rad oft waagrecht
    const int rasten = m_faderRadRest / 120;
    m_faderRadRest -= rasten * 120;
    if (rasten != 0)
    {
        const double schritt = tasten == Qt::ShiftModifier ? 0.1 : 0.5;
        const double jetzt = m_loop.kanaele[static_cast<std::size_t>(fader)].gainDb;
        kanalGain(fader, std::round((jetzt + rasten * schritt) / schritt) * schritt);
    }
    event->accept();
}

void Taktachse::kanalSchalten(int kanal, bool an)
{
    if (!m_geladen || m_griff != Griff::Keiner) return;
    Loop neu = m_loop;
    if (!lumi::loop::kanalSchalten(neu, kanal, an) || neu == m_loop) return;
    uebernimm(neu, m_kanal, m_clip, an ? tr("Channel on") : tr("Channel off"));
}

void Taktachse::contextMenuEvent(QContextMenuEvent* event)
{
    if (!m_geladen) return;
    const QPoint ort = event->pos();
    QMenu menu(this);
    if (ort.y() < kLinealHoehe)
    {
        fuelleZoomMenue(menu);
        lumi::ui::zeigeMenue(menu, event->globalPos());
        return;
    }
    const int kanal = kanalBei(ort.y());
    if (kanal >= 0 && ort.x() < kopfBreite())
    {
        // am Kanalnamen
        const Kanal& k = m_loop.kanaele[static_cast<std::size_t>(kanal)];
        menu.addAction(tr("Rename channel..."), this, [this, kanal, name = k.name] {
            bool ok = false;
            const QString neu = lumi::ui::frageText(this, tr("Rename channel"), tr("Name:"), name, &ok);
            if (ok) kanalUmbenennen(kanal, neu);
        });
        // 7b a: wie im Composer — auch durch Ziehen am Namen
        QAction* hoch = menu.addAction(tr("Move channel up"), this, [this, kanal] { kanalVerschieben(kanal, kanal - 1); });
        hoch->setObjectName(QStringLiteral("loopMoveChannelUp"));
        hoch->setEnabled(kanal > 0);
        QAction* runter =
            menu.addAction(tr("Move channel down"), this, [this, kanal] { kanalVerschieben(kanal, kanal + 1); });
        runter->setObjectName(QStringLiteral("loopMoveChannelDown"));
        runter->setEnabled(kanal + 1 < static_cast<int>(m_loop.kanaele.size()));
        menu.addAction(k.an ? tr("Turn channel off") : tr("Turn channel on"), this,
                       [this, kanal, an = k.an] { kanalSchalten(kanal, !an); });
        menu.addAction(tr("Delete channel"), this, [this, kanal] { kanalLoeschen(kanal); });
        lumi::ui::zeigeMenue(menu, event->globalPos());
        return;
    }

    int clipKanal = -1;
    int clip = -1;
    if (griffBei(ort, &clipKanal, &clip) != Griff::Keiner)
    {
        // der Rechtsklick auf einen von mehreren markierten Clips laesst die
        // Marken stehen: Duplicate und Delete gelten dann fuer alle
        const bool gruppe = m_gruppe.size() > 1 && istMarkiert(clipKanal, clip);
        if (gruppe)
        {
            m_kanal = clipKanal;
            m_clip = clip;
            update();
            Q_EMIT auswahlGeaendert();
        }
        else
            waehle(clipKanal, clip);
        const int zahl = static_cast<int>(m_gruppe.size());
        // die Basis gehoert einem Clip: bei mehreren sind diese Eintraege grau
        menu.addAction(tr("Edit base"), this, [this] { (void)bearbeiteBasis(); })->setEnabled(!gruppe);
        menu.addAction(tr("Make new base and edit"), this, [this] { (void)neueBasisUndBearbeiten(); })
            ->setEnabled(!gruppe);
        // die Effekte gelten fuer alle markierten (Entscheid Patrik S113, E2 b)
        menu.addAction(gruppe ? tr("Effects of %1 clips...").arg(zahl) : tr("Effects..."), this,
                       [this] { (void)verlangeEffekte(); });
        // Stufe 30c (WX6 a): ein Clip als WAV — nur einer
        QAction* exportieren = menu.addAction(tr("Export audio..."), this, [this] { (void)verlangeExport(); });
        exportieren->setObjectName(QStringLiteral("loopClipExportAudio"));
        exportieren->setEnabled(!gruppe);
        // Stufe 30g (FZ4 a B): sind alle markierten eingefroren, „Unfreeze"; sonst „Freeze"
        const bool alleEingefroren = std::all_of(m_gruppe.begin(), m_gruppe.end(), [this](const ClipOrt& ort) {
            const int nr = findeClip(m_loop, ort);
            return nr >= 0 && !m_loop.kanaele[static_cast<std::size_t>(ort.kanal)].clips[static_cast<std::size_t>(nr)].frozen.isEmpty();
        });
        QString freezeEintrag = alleEingefroren ? (gruppe ? tr("Unfreeze %1 clips").arg(zahl) : tr("Unfreeze clip"))
                                                : (gruppe ? tr("Freeze %1 clips").arg(zahl) : tr("Freeze clip"));
        // Stufe 30h (FZ5 a C): ein Clip mit Sanduhr nennt seine Zeit
        if (!gruppe && !alleEingefroren)
        {
            const FreezeMarke marke =
                freezeMarke(clipKanal, m_loop.kanaele[static_cast<std::size_t>(clipKanal)].clips[static_cast<std::size_t>(clip)].startTakt);
            if (marke.zeigeZeit) freezeEintrag = tr("Freeze clip (%1 s to compute)").arg(marke.gezeigtS(), 0, 'f', 1);
        }
        QAction* freeze = menu.addAction(freezeEintrag, this, [this, ein = !alleEingefroren] { (void)verlangeFreeze(ein); });
        freeze->setObjectName(QStringLiteral("loopClipFreeze"));
        menu.addSeparator();
        menu.addAction(gruppe ? tr("Duplicate %1 clips").arg(zahl) : tr("Duplicate"), this,
                       [this] { (void)dupliziereAuswahl(); });
        QAction* umbenennen = menu.addAction(tr("Rename base..."), this, [this] {
            const LoopClip* gewaehlt = auswahl();
            if (gewaehlt == nullptr) return;
            bool ok = false;
            const QString neu = lumi::ui::frageText(this, tr("Rename base"), tr("Name:"), gewaehlt->muster, &ok);
            if (ok) (void)benenneBasisUm(neu);
        });
        umbenennen->setEnabled(!gruppe);
        menu.addAction(gruppe ? tr("Delete %1 clips").arg(zahl) : tr("Delete"), this,
                       [this] { (void)loescheAuswahl(); });
        // Stufe 28e (SK13 a): die Kurve dieses Clips weg
        menu.addSeparator();
        const bool kurve =
            !m_loop.kanaele[static_cast<std::size_t>(clipKanal)].clips[static_cast<std::size_t>(clip)].expression.empty();
        menu.addAction(tr("Clear expression"), this, [this, clipKanal, clip] { (void)leereAusdruck(clipKanal, clip); })
            ->setEnabled(!gruppe && kurve);
    }
    else
    {
        if (kanal >= 0)
        {
            const int takt = taktBei(ort.x());
            QAction* neu = menu.addAction(tr("New drumkit clip"), this, [this, kanal, takt] { (void)neuerClip(kanal, takt); });
            neu->setEnabled(istFrei(m_loop, kanal, takt, kTakteNeu));
            QAction* instrument = menu.addAction(tr("New instrument clip"), this, [this, kanal, takt] {
                (void)neuerClip(kanal, takt, MusterArt::Instrument);
            });
            instrument->setEnabled(neu->isEnabled());
            menu.addSeparator();
        }
        menu.addAction(tr("Add channel"), this, [this] { kanalHinzu(); });
        menu.addAction(tr("Select all clips"), QKeySequence(Qt::CTRL | Qt::Key_A), this, [this] { markiereAlle(); });
    }
    fuelleZoomMenue(menu);
    lumi::ui::zeigeMenue(menu, event->globalPos());
}

}  // namespace lumi::loop
