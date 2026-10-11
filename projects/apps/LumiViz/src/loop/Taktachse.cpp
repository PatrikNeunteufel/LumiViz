/**
 * @file Taktachse.cpp
 * @brief Loop: die Achse des Loop-Panels
 */

#include "loop/Taktachse.hpp"

#include "composer/Komposition.hpp"  // reihenFarbe: dieselben Farbreihen wie die Kanaele des Composers
#include "loop/Bearbeiten.hpp"

#include <QAction>
#include <QContextMenuEvent>
#include <QInputDialog>
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
    if (auswahl() == nullptr)  // den Clip gibt es nicht mehr
    {
        m_kanal = -1;
        m_clip = -1;
    }
    updateGeometry();
    passeRollenAn();
    update();
    Q_EMIT auswahlGeaendert();  // auch bei gleicher Auswahl: der Clip kann neue Takte haben
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

const LoopClip* Taktachse::auswahl() const
{
    if (m_kanal < 0 || m_kanal >= static_cast<int>(m_loop.kanaele.size())) return nullptr;
    const auto& clips = m_loop.kanaele[static_cast<std::size_t>(m_kanal)].clips;
    if (m_clip < 0 || m_clip >= static_cast<int>(clips.size())) return nullptr;
    return &clips[static_cast<std::size_t>(m_clip)];
}

void Taktachse::waehle(int kanal, int clip)
{
    if (kanal == m_kanal && clip == m_clip) return;
    m_kanal = kanal;
    m_clip = clip;
    if (auswahl() == nullptr)
    {
        m_kanal = -1;
        m_clip = -1;
    }
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
    // zwoelf Toene: erst die kuehle, dann die warme Reihe des Composers
    const int reihen = 2 * lumi::composer::kReihenFarben;
    const bool warm = nr % reihen >= lumi::composer::kReihenFarben;
    const std::int32_t rgb = lumi::composer::reihenFarbe(
        warm ? lumi::composer::KanalArt::Audio : lumi::composer::KanalArt::Visual, nr % lumi::composer::kReihenFarben);
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
        if (!m_spielt) return;
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
            const bool gewaehlt = k == m_kanal && c == m_clip;
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
    p.setClipRect(ausschnitte.zeilenZeit);
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
    if (ort.y() < kLinealHoehe) return;  // Play beginnt immer von vorn: im Lineal gibt es nichts zu setzen
    int kanal = -1;
    int clip = -1;
    const Griff griff = griffBei(ort, &kanal, &clip);
    if (griff != Griff::Keiner)
    {
        m_griff = griff;
        m_zugLaeuft = false;
        m_zugStart = ort;
        m_zugKanal = kanal;
        m_zugClip = clip;
        m_vorZug = m_loop;
        m_zugVersatz = stelleVon(ort.x()) -
                       m_loop.kanaele[static_cast<std::size_t>(kanal)].clips[static_cast<std::size_t>(clip)].startTakt *
                           kTicksJeTakt;
    }
    waehle(kanal, clip);
}

void Taktachse::zieheZu(QPoint ort)
{
    m_loop = m_vorZug;
    int ziel = m_zugKanal;
    int platz = m_zugClip;
    const std::int64_t maus = stelleVon(ort.x());
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
        case Griff::Keiner:
            return;
    }
    m_kanal = ziel;
    m_clip = platz;
    // die Laenge der Achse bleibt im Zug, wie sie war: aenderte sich der
    // Massstab unter der Maus, liefe der Clip ihr davon. Neu gemessen wird
    // beim Loslassen.
    update();
}

void Taktachse::brichZugAb()
{
    if (m_griff == Griff::Keiner) return;
    if (m_zugLaeuft)
    {
        m_loop = m_vorZug;
        m_kanal = m_zugKanal;
        m_clip = m_zugClip;
    }
    m_griff = Griff::Keiner;
    m_zugLaeuft = false;
    update();
}

void Taktachse::mouseMoveEvent(QMouseEvent* event)
{
    const QPoint ort = event->position().toPoint();
    if (schwenkt())
    {
        schwenkeZu(ort);
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
    if (griff == Griff::KanteLinks || griff == Griff::KanteRechts)
        setCursor(Qt::SizeHorCursor);
    else
        unsetCursor();
}

void Taktachse::mouseReleaseEvent(QMouseEvent* event)
{
    merkeTaste(event->buttons() != Qt::NoButton);
    if (schwenkt())
    {
        if (event->button() != Qt::MiddleButton) return;  // eine andere Taste: das Schwenken laeuft weiter
        beendeSchwenken();
        return;
    }
    if (m_griff == Griff::Keiner) return;
    const Griff griff = m_griff;
    const bool gezogen = m_zugLaeuft;
    m_griff = Griff::Keiner;
    m_zugLaeuft = false;
    if (!gezogen) return;
    setzeLaenge(anzeigeTakte(m_loop) * kTicksJeTakt);
    passeRollenAn();
    update();
    if (m_loop == m_vorZug) return;
    Q_EMIT auswahlGeaendert();  // der Clip hat neue Takte
    Q_EMIT bearbeitet(m_loop, griff == Griff::Verschieben ? tr("Move clip") : tr("Resize clip"));
}

void Taktachse::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape && m_griff != Griff::Keiner)
    {
        brichZugAb();
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
    if (auswahl() == nullptr)
    {
        m_kanal = -1;
        m_clip = -1;
    }
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
    uebernimm(neu, kanal, platz, tr("New clip"));
    return true;
}

bool Taktachse::dupliziereAuswahl()
{
    if (auswahl() == nullptr || m_griff != Griff::Keiner) return false;
    Loop neu = m_loop;
    const int platz = dupliziere(neu, m_kanal, m_clip);
    if (platz < 0) return false;
    uebernimm(neu, m_kanal, platz, tr("Duplicate clip"));
    return true;
}

bool Taktachse::loescheAuswahl()
{
    if (auswahl() == nullptr || m_griff != Griff::Keiner) return false;
    Loop neu = m_loop;
    if (!loescheClip(neu, m_kanal, m_clip)) return false;
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
        menu.exec(event->globalPos());
        return;
    }
    const int kanal = kanalBei(ort.y());
    if (kanal >= 0 && ort.x() < kKopfBreite)
    {
        // am Kanalnamen
        const Kanal& k = m_loop.kanaele[static_cast<std::size_t>(kanal)];
        menu.addAction(tr("Rename channel..."), this, [this, kanal, name = k.name] {
            bool ok = false;
            const QString neu = QInputDialog::getText(this, tr("Rename channel"), tr("Name:"), QLineEdit::Normal, name, &ok);
            if (ok) kanalUmbenennen(kanal, neu);
        });
        menu.addAction(k.an ? tr("Turn channel off") : tr("Turn channel on"), this,
                       [this, kanal, an = k.an] { kanalSchalten(kanal, !an); });
        menu.addAction(tr("Delete channel"), this, [this, kanal] { kanalLoeschen(kanal); });
        menu.exec(event->globalPos());
        return;
    }

    int clipKanal = -1;
    int clip = -1;
    if (griffBei(ort, &clipKanal, &clip) != Griff::Keiner)
    {
        waehle(clipKanal, clip);
        menu.addAction(tr("Edit base"), this, [this] { (void)bearbeiteBasis(); });
        menu.addAction(tr("Make new base and edit"), this, [this] { (void)neueBasisUndBearbeiten(); });
        menu.addSeparator();
        menu.addAction(tr("Duplicate"), this, [this] { (void)dupliziereAuswahl(); });
        menu.addAction(tr("Rename base..."), this, [this] {
            const LoopClip* gewaehlt = auswahl();
            if (gewaehlt == nullptr) return;
            bool ok = false;
            const QString neu = QInputDialog::getText(this, tr("Rename base"), tr("Name:"), QLineEdit::Normal,
                                                      gewaehlt->muster, &ok);
            if (ok) (void)benenneBasisUm(neu);
        });
        menu.addAction(tr("Delete"), this, [this] { (void)loescheAuswahl(); });
    }
    else
    {
        if (kanal >= 0)
        {
            const int takt = taktBei(ort.x());
            QAction* neu = menu.addAction(tr("New drumkit clip"), this, [this, kanal, takt] { (void)neuerClip(kanal, takt); });
            neu->setEnabled(istFrei(m_loop, kanal, takt, kTakteMin));
            QAction* instrument = menu.addAction(tr("New instrument clip"), this, [this, kanal, takt] {
                (void)neuerClip(kanal, takt, MusterArt::Instrument);
            });
            instrument->setEnabled(neu->isEnabled());
            menu.addSeparator();
        }
        menu.addAction(tr("Add channel"), this, [this] { kanalHinzu(); });
    }
    fuelleZoomMenue(menu);
    menu.exec(event->globalPos());
}

}  // namespace lumi::loop
