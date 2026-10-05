/**
 * @file ComposerPanel.cpp
 * @brief Composer: das Panel zum Ansehen (Stufe C2b)
 */

#include "composer/ComposerPanel.hpp"

#include "UI/widgets/VisualizerWidget.hpp"
#include "composer/Mischer.hpp"
#include "composer/Sitzung.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
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
}  // namespace

// =============================================================================
// Zeitachse
// =============================================================================

Zeitachse::Zeitachse(QWidget* parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("composerZeitachse"));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
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
    const int zeile = (ort.y() - kLinealHoehe) / kZeilenHoehe;
    if (zeile >= 0 && zeile < static_cast<int>(m_komposition.kanaele.size()))
    {
        const int k = kanalVonZeile(zeile);
        const auto& clips = m_komposition.kanaele[static_cast<std::size_t>(k)].visual;
        for (int c = 0; c < static_cast<int>(clips.size()); ++c)
        {
            if (clipRechteck(zeile, clips[static_cast<std::size_t>(c)]).contains(ort))
            {
                kanal = k;
                clipIndex = c;
                break;
            }
        }
    }
    if (kanal == m_kanal && clipIndex == m_clip) return;
    m_kanal = kanal;
    m_clip = clipIndex;
    update();
    Q_EMIT auswahlGeaendert();
}

void Zeitachse::mouseMoveEvent(QMouseEvent* event)
{
    if (m_zieht) Q_EMIT sprung(stelleVon(event->position().toPoint().x()));
}

void Zeitachse::mouseReleaseEvent(QMouseEvent*)
{
    m_zieht = false;
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
    kopf->addStretch(1);
    m_datei = new QLabel(this);
    m_datei->setObjectName(QStringLiteral("composerDatei"));
    kopf->addWidget(m_datei);
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
    root->addWidget(m_clipZeile);

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
        connect(m_sitzung, &Sitzung::geaendert, this, [this] { zeigeStand(); });
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
    m_datei->setText(geladen ? QFileInfo(m_sitzung->datei()).fileName() : QString());
    m_datei->setToolTip(geladen ? QDir::toNativeSeparators(m_sitzung->datei()) : QString());
    zeigeStand();
    zeigeClip();
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
    if (clip == nullptr || m_sitzung == nullptr)
    {
        m_clipZeile->setText(m_sitzung != nullptr && m_sitzung->geladen() ? tr("No clip selected")
                                                                          : QString());
        m_clipZeile->setToolTip(QString());
        return;
    }
    const Kanal& kanal =
        m_sitzung->komposition().kanaele[static_cast<std::size_t>(m_zeitachse->gewaehlterKanal())];
    m_clipZeile->setText(QStringLiteral("%1 · %2 · %3 – %4 · %5")
                             .arg(kanal.name, QFileInfo(clip->datei).fileName(),
                                  zeitText(clip->startMs), zeitText(clip->endeMs),
                                  QString::fromLatin1(mischartName(clip->mischart))));
    m_clipZeile->setToolTip(QDir::toNativeSeparators(clip->datei));
}

}  // namespace lumi::composer
