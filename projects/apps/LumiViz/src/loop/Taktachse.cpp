/**
 * @file Taktachse.cpp
 * @brief Loop: die Achse des Loop-Panels
 */

#include "loop/Taktachse.hpp"

#include "UI/widgets/Farbreihen.hpp"  // dieselben Farbreihen wie die Kanaele des Composers (S128, N1 b)
#include "loop/Bearbeiten.hpp"

#include <QAction>
#include <QContextMenuEvent>
#include <QInputDialog>

#include "UI/widgets/Bedienhilfe.hpp"  // zeigeMenue, frageText: in Tests abfragbar
#include <QKeyEvent>
#include <QLineEdit>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>

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
                p.drawText(QRect(8, oben, kKopfBreite - 12, hoch), Qt::AlignLeft | Qt::AlignVCenter,
                           tr("New channel"));
            continue;
        }
        const Kanal& kanal = m_loop.kanaele[static_cast<std::size_t>(k)];
        QColor name = text;
        if (!kanal.an) name.setAlpha(110);
        p.setPen(name);
        const QString titel = kanal.an ? kanal.name : tr("%1 (off)").arg(kanal.name);
        if (namePasst(hoch))
            p.drawText(QRect(8, oben, kKopfBreite - 12, hoch), Qt::AlignLeft | Qt::AlignVCenter,
                       fontMetrics().elidedText(titel, Qt::ElideRight, kKopfBreite - 12));

        p.setClipRect(ausschnitte.zeilenZeit);
        for (int c = 0; c < static_cast<int>(kanal.clips.size()); ++c)
        {
            const LoopClip& clip = kanal.clips[static_cast<std::size_t>(c)];
            const QRect r = clipRechteck(k, c);
            if (r.right() < kKopfBreite || r.left() > width()) continue;  // aus dem Fenster gerollt
            const bool gewaehlt = istMarkiert(k, c);
            const QColor farbe = basisFarbe(clip.muster, kanal.an);
            p.setPen(gewaehlt ? QPen(pal.color(QPalette::Highlight), 2.0) : QPen(farbe.darker(150), 1.0));
            p.setBrush(farbe);
            // hineingezoomt kann ein Clip viele tausend Pixel breit sein: gezeichnet
            // wird nur, was im Fenster liegt (und etwas mehr, fuer die runden Ecken)
            p.drawRoundedRect(
                r.intersected(QRect(kKopfBreite - 12, r.top(), width() - kKopfBreite + 24, r.height())), 4, 4);
            p.setBrush(Qt::NoBrush);

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
            namensFeld.setLeft(std::max(r.left(), kKopfBreite) + 6);
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
                p.setPen(Qt::white);  // der Name bleibt hell, auch neben einer matten Marke
            }
            const QString beschriftung = clip.wiederholungen > 1
                                             ? tr("%1 × %2").arg(clip.muster).arg(clip.wiederholungen)
                                             : clip.muster;
            if (namePasst(r.height()))
                p.drawText(namensFeld, Qt::AlignLeft | Qt::AlignVCenter,
                           fontMetrics().elidedText(beschriftung, Qt::ElideRight, std::max(0, namensFeld.width())));
        }
    }
    p.setClipRect(ausschnitte.zeilen);
    p.setPen(linie);
    p.drawLine(kKopfBreite - 1, 0, kKopfBreite - 1, unten);
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
    p.drawLine(kKopfBreite - 1, 0, kKopfBreite - 1, unten);
    p.setClipRect(ausschnitte.linealZeit);
    auflage();
}

Taktachse::Griff Taktachse::griffBei(QPoint ort, int* kanal, int* clip) const
{
    *kanal = -1;
    *clip = -1;
    if (!m_geladen || ort.x() < kKopfBreite) return Griff::Keiner;
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
    // am Kanalnamen (7b a): der Kanal laesst sich an einen anderen Platz ziehen —
    // vorbereitet wird es hier, gezogen erst, wenn die Maus sich bewegt
    if (ort.x() < kKopfBreite && kanalBei(ort.y()) >= 0)
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
    if (griff == Griff::Keiner)
    {
        // freie Flaeche: von hier laesst sich ein Rahmen aufziehen; mit Ctrl
        // kommt, was er trifft, zu den Marken dazu
        if (ort.x() >= kKopfBreite)
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
    if (ort.x() < kKopfBreite && kanalBei(ort.y()) >= 0)
        setCursor(Qt::OpenHandCursor);  // am Kanalnamen: der Kanal laesst sich verschieben
    else if (griff == Griff::KanteLinks || griff == Griff::KanteRechts)
        setCursor(Qt::SizeHorCursor);
    else
        unsetCursor();
}

void Taktachse::mouseReleaseEvent(QMouseEvent* event)
{
    merkeTaste(event->buttons() != Qt::NoButton);
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

void Taktachse::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (!m_geladen || event->button() != Qt::LeftButton) return;
    int kanal = -1;
    int clip = -1;
    if (griffBei(event->position().toPoint(), &kanal, &clip) == Griff::Keiner) return;
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
    if (kanal >= 0 && ort.x() < kKopfBreite)
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
