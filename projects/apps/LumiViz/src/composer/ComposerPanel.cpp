/**
 * @file ComposerPanel.cpp
 * @brief Composer: das Panel zum Ansehen (Stufe C2b)
 */

#include "composer/ComposerPanel.hpp"

#include "UI/widgets/VisualizerWidget.hpp"
#include "composer/Bearbeiten.hpp"
#include "composer/Mischer.hpp"
#include "composer/Sitzung.hpp"
#include "services/ICommandBus.hpp"
#include "services/ServiceContainer.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QSettings>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QStyle>
#include <QVBoxLayout>

#include <algorithm>
#include <array>

namespace lumi::composer
{

namespace
{
const QString kKnopfAn = QStringLiteral("QPushButton { background-color: #4a6fa5; border-radius: 4px; }");

/// Farbe der Clips eines Kanals — je Kanal ein eigener Ton.
QColor kanalFarbe(int kanal, bool an)
{
    QColor f = QColor::fromHsv((205 + kanal * 67) % 360, 120, 165);
    if (!an) f = QColor::fromHsv(f.hue(), 40, 110);
    return f;
}

/// Abstand der Striche im Lineal, in Sekunden: der kleinste, bei dem die
/// Beschriftungen nicht zusammenstossen.
int strichSekunden(std::int64_t laengeMs, int breite)
{
    static constexpr std::array<int, 12> kStufen = {1, 2, 5, 10, 15, 30, 60, 120, 300, 600, 1800, 3600};
    if (laengeMs <= 0 || breite <= 0) return 1;
    for (int s : kStufen)
        if (static_cast<double>(s) * 1000.0 / static_cast<double>(laengeMs) * breite >= 64.0) return s;
    return kStufen.back();
}

/// Ein Schritt des Bearbeitens fuer Undo / Redo: die Fassung davor und danach.
/// Gilt nur fuer die Ladung, in der er entstand — nach Open oder Close
/// Composition tut er nichts mehr.
class KompositionsBefehl : public ICommand
{
public:
    KompositionsBefehl(Sitzung& sitzung, Komposition nachher, QString was)
        : m_sitzung(&sitzung),
          m_ladung(sitzung.ladung()),
          m_vorher(sitzung.komposition()),
          m_nachher(std::move(nachher)),
          m_was(was.toStdString())
    {
    }

    bool execute() override
    {
        if (!gilt() || gleich(m_vorher, m_nachher)) return false;
        m_sitzung->setzeKomposition(m_nachher);
        return true;
    }
    void undo() override
    {
        if (gilt()) m_sitzung->setzeKomposition(m_vorher);
    }
    [[nodiscard]] std::string description() const override { return m_was; }

private:
    [[nodiscard]] bool gilt() const { return m_sitzung != nullptr && m_sitzung->ladung() == m_ladung; }

    QPointer<Sitzung> m_sitzung;
    int m_ladung;
    Komposition m_vorher;
    Komposition m_nachher;
    std::string m_was;
};
}  // namespace

// =============================================================================
// Zeitachse
// =============================================================================

Zeitachse::Zeitachse(QWidget* parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("composerZeitachse"));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    setFocusPolicy(Qt::StrongFocus);  // Entf und Esc im Bearbeiten
}

void Zeitachse::setKomposition(const Komposition& komposition, bool geladen)
{
    m_komposition = komposition;
    m_geladen = geladen;
    m_laengeMs = geladen ? komposition.laenge() : 0;
    m_kanal = -1;
    m_clip = -1;
    m_stelleMs = 0;
    updateGeometry();
    update();
    Q_EMIT auswahlGeaendert();
}

void Zeitachse::setStelle(std::int64_t stelleMs)
{
    if (stelleMs == m_stelleMs) return;
    m_stelleMs = stelleMs;
    update();
}

void Zeitachse::setAn(bool an)
{
    if (an == m_an) return;
    m_an = an;
    update();
}

const VisualClip* Zeitachse::auswahl() const
{
    if (m_kanal < 0 || m_kanal >= static_cast<int>(m_komposition.kanaele.size())) return nullptr;
    const Kanal& kanal = m_komposition.kanaele[static_cast<std::size_t>(m_kanal)];
    if (m_clip < 0 || m_clip >= static_cast<int>(kanal.visual.size())) return nullptr;
    return &kanal.visual[static_cast<std::size_t>(m_clip)];
}

int Zeitachse::zeitBreite() const
{
    return std::max(1, width() - kKopfBreite - kRand);
}

int Zeitachse::xVon(std::int64_t stelleMs) const
{
    if (m_laengeMs <= 0) return kKopfBreite;
    return kKopfBreite + static_cast<int>(static_cast<double>(stelleMs) /
                                          static_cast<double>(m_laengeMs) * zeitBreite() + 0.5);
}

std::int64_t Zeitachse::stelleVon(int x) const
{
    if (m_laengeMs <= 0) return 0;
    const double anteil = static_cast<double>(x - kKopfBreite) / static_cast<double>(zeitBreite());
    return std::clamp<std::int64_t>(static_cast<std::int64_t>(anteil * static_cast<double>(m_laengeMs) + 0.5),
                                    0, m_laengeMs);
}

int Zeitachse::yVonZeile(int zeile) const
{
    return kLinealHoehe + zeile * kZeilenHoehe + kZeilenHoehe / 2;
}

int Zeitachse::kanalVonZeile(int zeile) const
{
    return zeile;  // CH1 oben (Entscheid Patrik S95) — im Stapel liegt er zuunterst
}

QRect Zeitachse::clipRechteck(int zeile, const VisualClip& clip) const
{
    const int links = xVon(clip.startMs);
    const int rechts = xVon(clip.endeMs);
    return {links, kLinealHoehe + zeile * kZeilenHoehe + 3, std::max(2, rechts - links),
            kZeilenHoehe - 6};
}

QSize Zeitachse::sizeHint() const
{
    return {640, minimumSizeHint().height()};
}

QSize Zeitachse::minimumSizeHint() const
{
    const int zeilen = std::max<int>(2, static_cast<int>(m_komposition.kanaele.size()));
    return {kKopfBreite + 160, kLinealHoehe + zeilen * kZeilenHoehe + 4};
}

void Zeitachse::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    const QPalette pal = palette();
    p.fillRect(rect(), pal.color(QPalette::Base));

    if (!m_geladen)
    {
        p.setPen(pal.color(QPalette::Disabled, QPalette::Text));
        p.drawText(rect(), Qt::AlignCenter,
                   tr("No composition loaded — File → Open Composition..."));
        return;
    }

    const QColor text = pal.color(QPalette::Text);
    QColor linie = text;
    linie.setAlpha(50);
    const int zeilen = static_cast<int>(m_komposition.kanaele.size());
    const int unten = kLinealHoehe + zeilen * kZeilenHoehe;

    // --- Lineal
    p.fillRect(QRect(0, 0, width(), kLinealHoehe), pal.color(QPalette::AlternateBase));
    const int schritt = strichSekunden(m_laengeMs, zeitBreite());
    QFont klein = font();
    klein.setPointSizeF(std::max(6.5, klein.pointSizeF() - 1.0));
    p.setFont(klein);
    for (std::int64_t ms = 0; ms <= m_laengeMs; ms += static_cast<std::int64_t>(schritt) * 1000)
    {
        const int x = xVon(ms);
        p.setPen(linie);
        p.drawLine(x, kLinealHoehe - 6, x, unten);
        p.setPen(text);
        const QString marke = ComposerPanel::zeitText(ms).section(QLatin1Char('.'), 0, 0);
        // am rechten Rand steht die Beschriftung links vom Strich, sonst waere sie abgeschnitten
        if (x + 6 + p.fontMetrics().horizontalAdvance(marke) > width())
            p.drawText(QRect(x - 83, 0, 80, kLinealHoehe - 4), Qt::AlignRight | Qt::AlignVCenter, marke);
        else
            p.drawText(QRect(x + 3, 0, 80, kLinealHoehe - 4), Qt::AlignLeft | Qt::AlignVCenter, marke);
    }
    p.setPen(linie);
    p.drawLine(xVon(m_laengeMs), 0, xVon(m_laengeMs), unten);  // das Ende

    // --- Kanaele: in der Reihenfolge der Datei, CH1 oben
    p.setFont(font());
    for (int zeile = 0; zeile < zeilen; ++zeile)
    {
        const int k = kanalVonZeile(zeile);
        const Kanal& kanal = m_komposition.kanaele[static_cast<std::size_t>(k)];
        const int oben = kLinealHoehe + zeile * kZeilenHoehe;
        p.setPen(linie);
        p.drawLine(0, oben + kZeilenHoehe, width(), oben + kZeilenHoehe);

        QColor name = text;
        if (!kanal.an) name.setAlpha(110);
        p.setPen(name);
        const QString titel = kanal.an ? kanal.name : tr("%1 (off)").arg(kanal.name);
        p.drawText(QRect(8, oben, kKopfBreite - 12, kZeilenHoehe), Qt::AlignLeft | Qt::AlignVCenter,
                   fontMetrics().elidedText(titel, Qt::ElideRight, kKopfBreite - 12));

        for (int c = 0; c < static_cast<int>(kanal.visual.size()); ++c)
        {
            const VisualClip& clip = kanal.visual[static_cast<std::size_t>(c)];
            const QRect r = clipRechteck(zeile, clip);
            const bool gewaehlt = k == m_kanal && c == m_clip;
            const QColor farbe = kanalFarbe(k, m_an && kanal.an);
            p.setPen(gewaehlt ? QPen(pal.color(QPalette::Highlight), 2.0) : QPen(farbe.darker(150), 1.0));
            p.setBrush(farbe);
            p.drawRoundedRect(r, 4, 4);

            // die Pegelkurve: Hoehe im Clip = Pegel
            if (!clip.kurve.empty() && r.width() > 8 && clip.dauerMs() > 0)
            {
                QPainterPath kurve;
                const int stuetzen = std::clamp(r.width() / 3, 8, 200);
                for (int i = 0; i <= stuetzen; ++i)
                {
                    const double anteil = static_cast<double>(i) / stuetzen;
                    const double pegel = clip.pegelBei(
                        static_cast<std::int64_t>(anteil * static_cast<double>(clip.dauerMs())));
                    const QPointF punkt(r.left() + anteil * r.width(),
                                        r.bottom() - 1 - pegel * (r.height() - 3));
                    if (i == 0)
                        kurve.moveTo(punkt);
                    else
                        kurve.lineTo(punkt);
                }
                p.setBrush(Qt::NoBrush);
                p.setPen(QPen(QColor(255, 255, 255, 170), 1.2));
                p.drawPath(kurve);
            }

            p.setPen(Qt::white);
            p.drawText(r.adjusted(6, 0, -4, 0), Qt::AlignLeft | Qt::AlignVCenter,
                       fontMetrics().elidedText(QFileInfo(clip.datei).completeBaseName(),
                                                Qt::ElideRight, std::max(0, r.width() - 10)));
        }
    }
    p.setPen(linie);
    p.drawLine(kKopfBreite - 1, 0, kKopfBreite - 1, unten);

    // --- Abspielmarke
    const int x = xVon(m_stelleMs);
    QPen marke(QColor(255, 90, 70), 2.0);
    if (!m_an) marke.setStyle(Qt::DashLine);
    p.setPen(marke);
    p.drawLine(x, 0, x, unten);
}

void Zeitachse::setBearbeiten(bool an)
{
    if (an == m_bearbeiten) return;
    brichZugAb();
    m_bearbeiten = an;
    setMouseTracking(an);  // fuer den Zeiger an den Kanten
    if (!an) unsetCursor();
    update();
}

void Zeitachse::zeigeBearbeitet(const Komposition& komposition)
{
    m_griff = Griff::Keiner;
    m_zugLaeuft = false;
    m_komposition = komposition;
    m_laengeMs = komposition.laenge();
    m_stelleMs = std::min(m_stelleMs, m_laengeMs);
    if (auswahl() == nullptr)  // den Clip gibt es nicht mehr
    {
        m_kanal = -1;
        m_clip = -1;
    }
    updateGeometry();
    update();
    Q_EMIT auswahlGeaendert();  // auch bei gleicher Auswahl: der Clip kann neue Zeiten haben
}

std::int64_t Zeitachse::raste(std::int64_t zeitMs, const Komposition& in, int ausserKanal,
                              int ausserClip) const
{
    // so viele Millisekunden sind acht Pixel
    const std::int64_t nah =
        m_laengeMs > 0 ? static_cast<std::int64_t>(8.0 * static_cast<double>(m_laengeMs) / zeitBreite())
                       : 0;
    std::int64_t beste = -1;
    std::int64_t abstand = nah + 1;
    const auto pruefe = [&](std::int64_t kante) {
        const std::int64_t a = kante > zeitMs ? kante - zeitMs : zeitMs - kante;
        if (a < abstand)
        {
            abstand = a;
            beste = kante;
        }
    };
    for (int k = 0; k < static_cast<int>(in.kanaele.size()); ++k)
    {
        const auto& clips = in.kanaele[static_cast<std::size_t>(k)].visual;
        for (int c = 0; c < static_cast<int>(clips.size()); ++c)
        {
            if (k == ausserKanal && c == ausserClip) continue;
            pruefe(clips[static_cast<std::size_t>(c)].startMs);
            pruefe(clips[static_cast<std::size_t>(c)].endeMs);
        }
    }
    pruefe(m_stelleMs);
    if (beste >= 0) return beste;
    const auto raster = static_cast<std::int64_t>(in.rasterSekunden * 1000.0 + 0.5);
    if (raster <= 0) return zeitMs;
    return (zeitMs + raster / 2) / raster * raster;
}

Zeitachse::Griff Zeitachse::griffBei(QPoint ort, int* kanal, int* clip) const
{
    *kanal = -1;
    *clip = -1;
    const int zeile = ort.y() < kLinealHoehe ? -1 : (ort.y() - kLinealHoehe) / kZeilenHoehe;
    if (zeile < 0 || zeile >= static_cast<int>(m_komposition.kanaele.size())) return Griff::Keiner;
    const int k = kanalVonZeile(zeile);
    const auto& clips = m_komposition.kanaele[static_cast<std::size_t>(k)].visual;
    for (int c = 0; c < static_cast<int>(clips.size()); ++c)
    {
        const QRect r = clipRechteck(zeile, clips[static_cast<std::size_t>(c)]);
        if (!r.contains(ort)) continue;
        *kanal = k;
        *clip = c;
        // Kanten: die aeusseren sechs Pixel — bei schmalen Clips nur rechts
        if (r.width() > 16 && ort.x() - r.left() <= 6) return Griff::KanteLinks;
        if (r.right() - ort.x() <= 6) return Griff::KanteRechts;
        return Griff::Verschieben;
    }
    return Griff::Keiner;
}

void Zeitachse::mousePressEvent(QMouseEvent* event)
{
    if (!m_geladen || event->button() != Qt::LeftButton) return;
    const QPoint ort = event->position().toPoint();
    if (ort.y() < kLinealHoehe)
    {
        m_zieht = true;
        Q_EMIT sprung(stelleVon(ort.x()));
        return;
    }
    int kanal = -1;
    int clipIndex = -1;
    const Griff griff = griffBei(ort, &kanal, &clipIndex);
    if (m_bearbeiten && griff != Griff::Keiner)
    {
        m_griff = griff;
        m_zugLaeuft = false;
        m_zugStart = ort;
        m_zugKanal = kanal;
        m_zugClip = clipIndex;
        m_vorZug = m_komposition;
        m_zugVersatzMs =
            stelleVon(ort.x()) -
            m_komposition.kanaele[static_cast<std::size_t>(kanal)].visual[static_cast<std::size_t>(clipIndex)].startMs;
    }
    if (kanal == m_kanal && clipIndex == m_clip) return;
    m_kanal = kanal;
    m_clip = clipIndex;
    update();
    Q_EMIT auswahlGeaendert();
}

void Zeitachse::zieheZu(QPoint ort, Qt::KeyboardModifiers tasten)
{
    // Entscheid Patrik S95: ohne Taste anhalten, Ctrl kuerzen, Ctrl+Shift schieben
    const Stoss stoss = !(tasten & Qt::ControlModifier) ? Stoss::Halten
                        : (tasten & Qt::ShiftModifier)  ? Stoss::Schieben
                                                        : Stoss::Kuerzen;
    const bool frei = (tasten & Qt::AltModifier) != 0;
    const auto gerastet = [&](std::int64_t zeit) {
        return frei ? zeit : raste(zeit, m_vorZug, m_zugKanal, m_zugClip);
    };
    const VisualClip& alt =
        m_vorZug.kanaele[static_cast<std::size_t>(m_zugKanal)].visual[static_cast<std::size_t>(m_zugClip)];
    const std::int64_t maus = stelleVon(ort.x());

    m_komposition = m_vorZug;
    int platz = -1;
    int ziel = m_zugKanal;
    switch (m_griff)
    {
        case Griff::Verschieben:
        {
            // die Zeile unter der Maus: der Clip wechselt den Kanal (nur Visual-Kanaele)
            const int zeilen = static_cast<int>(m_vorZug.kanaele.size());
            const int zeile = std::clamp((ort.y() - kLinealHoehe) / kZeilenHoehe, 0, zeilen - 1);
            if (ort.y() >= kLinealHoehe &&
                m_vorZug.kanaele[static_cast<std::size_t>(kanalVonZeile(zeile))].art == KanalArt::Visual)
                ziel = kanalVonZeile(zeile);
            // einrasten darf der Anfang oder das Ende — was weniger weit weg liegt
            const std::int64_t wunsch = maus - m_zugVersatzMs;
            const std::int64_t vorn = gerastet(wunsch);
            const std::int64_t hinten = gerastet(wunsch + alt.dauerMs()) - alt.dauerMs();
            const auto weg = [wunsch](std::int64_t s) { return s > wunsch ? s - wunsch : wunsch - s; };
            platz = versetzeClip(m_komposition, m_zugKanal, m_zugClip, ziel,
                                 weg(hinten) < weg(vorn) ? hinten : vorn, stoss);
            break;
        }
        case Griff::KanteLinks:
            platz = zieheKante(m_komposition, m_zugKanal, m_zugClip, true, gerastet(maus), stoss);
            break;
        case Griff::KanteRechts:
            platz = zieheKante(m_komposition, m_zugKanal, m_zugClip, false, gerastet(maus), stoss);
            break;
        case Griff::Keiner:
            return;
    }
    m_kanal = ziel;
    m_clip = platz;
    // m_laengeMs bleibt im Zug, wie es war: aenderte sich der Massstab unter
    // der Maus, liefe der Clip ihr davon. Neu gemessen wird beim Loslassen.
    update();
}

void Zeitachse::brichZugAb()
{
    if (m_griff == Griff::Keiner) return;
    if (m_zugLaeuft)
    {
        m_komposition = m_vorZug;
        m_kanal = m_zugKanal;
        m_clip = m_zugClip;
        m_laengeMs = m_komposition.laenge();
    }
    m_griff = Griff::Keiner;
    m_zugLaeuft = false;
    update();
}

void Zeitachse::mouseMoveEvent(QMouseEvent* event)
{
    const QPoint ort = event->position().toPoint();
    if (m_zieht)
    {
        Q_EMIT sprung(stelleVon(ort.x()));
        return;
    }
    if (m_griff != Griff::Keiner && (event->buttons() & Qt::LeftButton))
    {
        if (!m_zugLaeuft && (ort - m_zugStart).manhattanLength() < 4) return;
        m_zugLaeuft = true;
        zieheZu(ort, event->modifiers());
        return;
    }
    if (m_bearbeiten && m_geladen)
    {
        int kanal = -1;
        int clip = -1;
        const Griff griff = griffBei(ort, &kanal, &clip);
        if (griff == Griff::KanteLinks || griff == Griff::KanteRechts)
            setCursor(Qt::SizeHorCursor);
        else
            unsetCursor();
    }
}

void Zeitachse::mouseReleaseEvent(QMouseEvent*)
{
    m_zieht = false;
    if (m_griff == Griff::Keiner) return;
    const Griff griff = m_griff;
    const bool gezogen = m_zugLaeuft;
    m_griff = Griff::Keiner;
    m_zugLaeuft = false;
    if (!gezogen) return;
    m_laengeMs = m_komposition.laenge();
    update();
    if (gleich(m_komposition, m_vorZug)) return;
    Q_EMIT auswahlGeaendert();  // der Clip hat neue Zeiten
    Q_EMIT bearbeitet(m_komposition, griff == Griff::Verschieben ? tr("Move clip") : tr("Resize clip"));
}

void Zeitachse::keyPressEvent(QKeyEvent* event)
{
    if (m_bearbeiten && event->key() == Qt::Key_Escape && m_griff != Griff::Keiner)
    {
        brichZugAb();
        return;
    }
    if (m_bearbeiten && m_griff == Griff::Keiner && auswahl() != nullptr &&
        (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace))
    {
        Komposition neu = m_komposition;
        if (loescheClip(neu, m_kanal, m_clip))
        {
            m_komposition = neu;
            m_kanal = -1;
            m_clip = -1;
            update();
            Q_EMIT auswahlGeaendert();
            Q_EMIT bearbeitet(neu, tr("Delete clip"));
        }
        return;
    }
    QWidget::keyPressEvent(event);
}

// =============================================================================
// ComposerPanel
// =============================================================================

ComposerPanel::ComposerPanel(ServiceContainer& services, QWidget* parent)
    : PanelBase(services, "composer", tr("Composer"), parent)
{
    setupUI();
    zeigeKomposition();
}

ComposerPanel::~ComposerPanel() = default;

Sitzung* ComposerPanel::sitzung() const
{
    return m_sitzung;
}

QString ComposerPanel::zeitText(std::int64_t ms)
{
    ms = std::max<std::int64_t>(0, ms);
    const std::int64_t zehntel = ms / 100;
    return QStringLiteral("%1:%2.%3")
        .arg(zehntel / 600)
        .arg((zehntel / 10) % 60, 2, 10, QLatin1Char('0'))
        .arg(zehntel % 10);
}

void ComposerPanel::setupUI()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(6, 6, 6, 6);

    // --- Kopf
    auto* kopf = new QHBoxLayout();
    m_an = new QCheckBox(tr("Composer on"), this);
    m_an->setObjectName(QStringLiteral("composerAn"));
    m_an->setToolTip(tr("On: the composition delivers the picture. Off: the effect chain does — "
                        "the composition stays loaded here and can be prepared."));
    kopf->addWidget(m_an);
    kopf->addSpacing(8);

    const auto knopf = [this, kopf](const char* name, QStyle::StandardPixmap bild) {
        auto* k = new QPushButton(this);
        k->setObjectName(QLatin1String(name));
        k->setIcon(style()->standardIcon(bild));
        k->setFixedSize(32, 32);
        kopf->addWidget(k);
        return k;
    };
    m_spielen = knopf("composerPlay", QStyle::SP_MediaPlay);
    m_stopp = knopf("composerStop", QStyle::SP_MediaStop);
    m_stopp->setToolTip(tr("Stop and return to the start"));
    m_schleife = knopf("composerLoop", QStyle::SP_BrowserReload);
    m_schleife->setCheckable(true);
    m_resume = knopf("composerResume", QStyle::SP_MediaSeekForward);
    m_resume->setCheckable(true);

    m_zeit = new QLabel(this);
    m_zeit->setObjectName(QStringLiteral("composerZeit"));
    kopf->addSpacing(8);
    kopf->addWidget(m_zeit);
    kopf->addSpacing(12);
    m_edit = new QCheckBox(tr("Edit"), this);
    m_edit->setObjectName(QStringLiteral("composerEdit"));
    m_edit->setToolTip(
        tr("On: clips can be changed.\n"
           "Drag a clip to move it, drag its left or right edge to change its length, "
           "Del removes the selected clip. Edit → Undo takes a step back.\n\n"
           "When the dragged clip meets a neighbour:\n"
           "no key — it stops there\n"
           "Ctrl — the neighbour is shortened\n"
           "Ctrl+Shift — the neighbours are pushed along\n\n"
           "Dragging snaps to the grid, to the edges of other clips and to the play "
           "position; hold Alt to drag freely. Esc cancels a drag.\n\n"
           "Off: the panel only shows and plays the composition."));
    kopf->addWidget(m_edit);
    kopf->addStretch(1);
    m_datei = new QLabel(this);
    m_datei->setObjectName(QStringLiteral("composerDatei"));
    kopf->addWidget(m_datei);
    m_speichern = new QPushButton(tr("Save"), this);
    m_speichern->setObjectName(QStringLiteral("composerSave"));
    m_speichern->setToolTip(tr("Write the changes into the loaded file"));
    kopf->addWidget(m_speichern);
    m_speichernUnter = new QPushButton(tr("Save As..."), this);
    m_speichernUnter->setObjectName(QStringLiteral("composerSaveAs"));
    kopf->addWidget(m_speichernUnter);
    root->addLayout(kopf);

    // --- Zeitachse (scrollt bei vielen Kanaelen)
    m_zeitachse = new Zeitachse(this);
    auto* rolle = new QScrollArea(this);
    rolle->setWidgetResizable(true);
    rolle->setFrameShape(QFrame::NoFrame);
    rolle->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    rolle->setWidget(m_zeitachse);
    root->addWidget(rolle, 1);

    // --- der angeklickte Clip
    m_clipZeile = new QLabel(this);
    m_clipZeile->setObjectName(QStringLiteral("composerClip"));
    m_clipZeile->setTextInteractionFlags(Qt::TextSelectableByMouse);
    auto* fuss = new QHBoxLayout();
    fuss->addWidget(m_clipZeile, 1);
    auto* mischTitel = new QLabel(tr("Blend:"), this);
    fuss->addWidget(mischTitel);
    m_mischart = new QComboBox(this);
    m_mischart->setObjectName(QStringLiteral("composerMischart"));
    for (int i = 0; i < kMischarten; ++i)
        m_mischart->addItem(QString::fromLatin1(mischartName(static_cast<Mischart>(i))));
    m_mischart->setToolTip(tr("How the selected clip is mixed with what lies below it. "
                              "Can be changed while Edit is on."));
    fuss->addWidget(m_mischart);
    root->addLayout(fuss);

    connect(m_edit, &QCheckBox::toggled, this, [this](bool an) {
        m_zeitachse->setBearbeiten(an);
        zeigeClip();
    });
    connect(m_zeitachse, &Zeitachse::bearbeitet, this,
            [this](const Komposition& neu, const QString& was) { bearbeite(neu, was); });
    connect(m_mischart, QOverload<int>::of(&QComboBox::activated), this, [this](int index) {
        const VisualClip* clip = m_zeitachse->auswahl();
        if (clip == nullptr || m_sitzung == nullptr || !m_zeitachse->bearbeiten()) return;
        Komposition neu = m_sitzung->komposition();
        neu.kanaele[static_cast<std::size_t>(m_zeitachse->gewaehlterKanal())]
            .visual[static_cast<std::size_t>(m_zeitachse->gewaehlterClip())]
            .mischart = static_cast<Mischart>(index);
        bearbeite(neu, tr("Change blend mode"));
    });
    connect(m_speichern, &QPushButton::clicked, this, [this] {
        if (m_sitzung == nullptr || m_sitzung->speichere()) return;
        QMessageBox::warning(this, tr("Save Composition"),
                             tr("Could not save the composition:\n%1")
                                 .arg(QDir::toNativeSeparators(m_sitzung->datei())));
    });
    connect(m_speichernUnter, &QPushButton::clicked, this, [this] { speichereUnter(); });

    connect(m_an, &QCheckBox::toggled, this, [this](bool an) {
        if (m_sitzung != nullptr) m_sitzung->setAn(an);
        m_gezeigt = -1;  // liess es sich nicht schalten, springt der Haken zurueck
        zeigeStand();
    });
    connect(m_spielen, &QPushButton::clicked, this, [this] {
        if (m_sitzung == nullptr) return;
        if (m_sitzung->laeuft())
            m_sitzung->pausiere();
        else
            m_sitzung->spiele();
    });
    connect(m_stopp, &QPushButton::clicked, this, [this] {
        if (m_sitzung != nullptr) m_sitzung->stoppe();
    });
    connect(m_schleife, &QPushButton::clicked, this, [this](bool an) {
        if (m_sitzung != nullptr) m_sitzung->setSchleife(an);
        zeigeStand();
    });
    connect(m_resume, &QPushButton::clicked, this, [this](bool an) {
        Sitzung::setResume(an);
        zeigeStand();
    });
    connect(m_zeitachse, &Zeitachse::sprung, this, [this](qint64 stelle) {
        if (m_sitzung != nullptr) m_sitzung->springe(stelle);
    });
    connect(m_zeitachse, &Zeitachse::auswahlGeaendert, this, [this] { zeigeClip(); });
}

void ComposerPanel::setFenster(VisualizerWidget* fenster)
{
    if (fenster == m_fenster) return;
    if (m_sitzung != nullptr) disconnect(m_sitzung, nullptr, this, nullptr);
    m_fenster = fenster;
    m_sitzung = fenster != nullptr ? &Sitzung::fuer(*fenster) : nullptr;
    if (m_sitzung != nullptr)
    {
        connect(m_sitzung, &Sitzung::kompositionGeaendert, this, [this] { zeigeKomposition(); });
        connect(m_sitzung, &Sitzung::kompositionBearbeitet, this, [this] {
            m_zeitachse->zeigeBearbeitet(m_sitzung->komposition());
        });
        connect(m_sitzung, &Sitzung::geaendert, this, [this] {
            zeigeStand();
            zeigeDatei();
        });
        connect(m_sitzung, &Sitzung::takt, this, [this] { zeigeStand(); });
    }
    zeigeKomposition();
}

void ComposerPanel::sucheFenster()
{
    if (m_fenster != nullptr) return;
    VisualizerWidget* gefunden = nullptr;
    if (QWidget* fenster = window()) gefunden = fenster->findChild<VisualizerWidget*>();
    // abgedockt ist das Panel sein eigenes Fenster — dann ueber alle suchen
    if (gefunden == nullptr)
    {
        const QWidgetList alle = QApplication::topLevelWidgets();
        for (QWidget* w : alle)
        {
            gefunden = qobject_cast<VisualizerWidget*>(w);
            if (gefunden == nullptr) gefunden = w->findChild<VisualizerWidget*>();
            if (gefunden != nullptr) break;
        }
    }
    if (gefunden != nullptr) setFenster(gefunden);
}

void ComposerPanel::showEvent(QShowEvent* event)
{
    PanelBase::showEvent(event);
    sucheFenster();
    zeigeStand();
}

void ComposerPanel::zeigeKomposition()
{
    const bool geladen = m_sitzung != nullptr && m_sitzung->geladen();
    m_zeitachse->setKomposition(geladen ? m_sitzung->komposition() : Komposition{}, geladen);
    zeigeDatei();
    zeigeStand();
    zeigeClip();
}

void ComposerPanel::zeigeDatei()
{
    const bool geladen = m_sitzung != nullptr && m_sitzung->geladen();
    const bool offen = geladen && m_sitzung->ungespeichert();
    // der Stern wie in der Preset-Zeile: es gibt Ungespeichertes
    m_datei->setText(geladen ? QFileInfo(m_sitzung->datei()).fileName() +
                                   (offen ? QStringLiteral(" *") : QString())
                             : QString());
    m_datei->setToolTip(geladen ? QDir::toNativeSeparators(m_sitzung->datei()) : QString());
    m_speichern->setEnabled(offen);
    m_speichernUnter->setEnabled(geladen);
    m_edit->setEnabled(geladen);
}

void ComposerPanel::bearbeite(const Komposition& neu, const QString& was)
{
    if (m_sitzung == nullptr || !m_sitzung->geladen()) return;
    if (auto* bus = services().tryResolve<ICommandBus>())
        (void)bus->execute(std::make_unique<KompositionsBefehl>(*m_sitzung, neu, was));
    else
        m_sitzung->setzeKomposition(neu);
}

void ComposerPanel::speichereUnter()
{
    if (m_sitzung == nullptr || !m_sitzung->geladen()) return;
    const QString titel = tr("Save Composition As");
    QString pfad = QFileDialog::getSaveFileName(this, titel, m_sitzung->datei(),
                                                tr("LumiViz Composition (*.lvcomp)"));
    if (pfad.isEmpty()) return;
    if (!pfad.endsWith(QStringLiteral(".lvcomp"), Qt::CaseInsensitive)) pfad += QStringLiteral(".lvcomp");
    if (!m_sitzung->speichereNach(pfad))
        QMessageBox::warning(this, titel,
                             tr("Could not save the composition:\n%1").arg(QDir::toNativeSeparators(pfad)));
}

void ComposerPanel::zeigeStand()
{
    const bool geladen = m_sitzung != nullptr && m_sitzung->geladen();
    const bool an = geladen && m_sitzung->an();
    const bool laeuft = an && m_sitzung->laeuft();
    const bool schleife = m_sitzung != nullptr ? m_sitzung->schleife() : false;

    // Schalter und Knoepfe nur anfassen, wenn sich etwas geaendert hat — der
    // Takt kommt 30-mal je Sekunde, und ein Stylesheet zu setzen ist teuer.
    const int zustand = (geladen ? 1 : 0) | (an ? 2 : 0) | (laeuft ? 4 : 0) | (schleife ? 8 : 0);
    if (zustand != m_gezeigt)
    {
        m_gezeigt = zustand;
        {
            const QSignalBlocker ruhe(m_an);
            m_an->setChecked(an);
        }
        m_an->setEnabled(geladen);
        m_spielen->setEnabled(geladen);
        m_stopp->setEnabled(geladen);
        m_spielen->setIcon(
            style()->standardIcon(laeuft ? QStyle::SP_MediaPause : QStyle::SP_MediaPlay));
        m_spielen->setToolTip(laeuft ? tr("Pause") : tr("Play"));
        m_schleife->setChecked(schleife);
        m_schleife->setStyleSheet(schleife ? kKnopfAn : QString());
        m_schleife->setToolTip(schleife ? tr("Loop the whole composition (ON)")
                                        : tr("Loop the whole composition"));
        m_zeitachse->setAn(an);
    }
    const bool resume = Sitzung::resumeAn();
    if (resume != m_resume->isChecked() || m_resume->toolTip().isEmpty())
    {
        m_resume->setChecked(resume);
        m_resume->setStyleSheet(resume ? kKnopfAn : QString());
        m_resume->setToolTip(
            resume ? tr("Resume after restart (ON): closing LumiViz brings the composition back "
                        "the way it was — Composer on or off, same position, playing or paused")
                   : tr("Resume after restart (off: the composition comes back loaded, with "
                        "the Composer off)"));
    }

    const std::int64_t stelle = geladen ? m_sitzung->stelleMs() : 0;
    m_zeit->setText(geladen ? QStringLiteral("%1 / %2").arg(zeitText(stelle),
                                                             zeitText(m_sitzung->laengeMs()))
                            : QString());
    m_zeitachse->setStelle(stelle);
}

void ComposerPanel::zeigeClip()
{
    const VisualClip* clip = m_zeitachse->auswahl();
    m_mischart->setEnabled(clip != nullptr && m_zeitachse->bearbeiten());
    if (clip == nullptr || m_sitzung == nullptr)
    {
        m_clipZeile->setText(m_sitzung != nullptr && m_sitzung->geladen() ? tr("No clip selected")
                                                                          : QString());
        m_clipZeile->setToolTip(QString());
        return;
    }
    m_mischart->setCurrentIndex(static_cast<int>(clip->mischart));
    const Kanal& kanal =
        m_sitzung->komposition().kanaele[static_cast<std::size_t>(m_zeitachse->gewaehlterKanal())];
    m_clipZeile->setText(QStringLiteral("%1 · %2 · %3 – %4 · %5")
                             .arg(kanal.name, QFileInfo(clip->datei).fileName(),
                                  zeitText(clip->startMs), zeitText(clip->endeMs),
                                  QString::fromLatin1(mischartName(clip->mischart))));
    m_clipZeile->setToolTip(QDir::toNativeSeparators(clip->datei));
}

}  // namespace lumi::composer
