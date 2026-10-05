/**
 * @file ComposerPanel.cpp
 * @brief Composer: das Panel zum Ansehen (Stufe C2b)
 */

#include "composer/ComposerPanel.hpp"

#include "UI/widgets/DateiDrag.hpp"
#include "UI/widgets/VisualizerWidget.hpp"
#include "composer/AppAnbindung.hpp"
#include "composer/Bearbeiten.hpp"
#include "composer/FormPresets.hpp"
#include "composer/KurvenEditor.hpp"
#include "composer/Mischer.hpp"
#include "composer/Sitzung.hpp"
#include "services/ICommandBus.hpp"
#include "services/ServiceContainer.hpp"
#include "visualizers/multieffect/ChainSerializer.hpp"
#include "visualizers/multieffect/EffectChain.hpp"

#include <QAction>
#include <QApplication>
#include <QContextMenuEvent>
#include <QDragEnterEvent>
#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <QMimeData>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QDoubleSpinBox>
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
    setAcceptDrops(true);             // Presets aus dem Effect Browser (C3b)
}

void Zeitachse::setKomposition(const Komposition& komposition, bool geladen)
{
    m_komposition = komposition;
    m_geladen = geladen;
    m_laengeMs = geladen ? anzeigeLaenge(komposition) : 0;
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
    if (m_clip < 0 || m_clip >= static_cast<int>(kanal.clips.size())) return nullptr;
    return &kanal.clips[static_cast<std::size_t>(m_clip)];
}

std::int64_t Zeitachse::anzeigeLaenge(const Komposition& komposition)
{
    // Luft hinter dem Ende: sonst liesse sich nichts ueber das Ende hinaus
    // ziehen oder dahinter fallen lassen. Eine leere Komposition zeigt 30 s.
    const std::int64_t laenge = komposition.laenge();
    return std::max<std::int64_t>(30000, laenge + std::max<std::int64_t>(10000, laenge / 5));
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
    // im Bearbeiten eine Zeile mehr: die Flaeche, auf der ein neuer Kanal entsteht
    const int zeilen = std::max<int>(2, static_cast<int>(m_komposition.kanaele.size()) +
                                            (m_bearbeiten ? 1 : 0));
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
    // --- das Ende: folgt es dem letzten Clip, eine duenne Linie; ist es fest
    //     gesetzt, eine eigene Marke — und die leere Strecke davor schraffiert
    const std::int64_t endeMs = m_komposition.laenge();
    const int xEnde = xVon(endeMs);
    if (m_komposition.laengeMs > 0)
    {
        const QColor fest(240, 170, 60);
        const int xClips = xVon(std::min(m_komposition.clipEnde(), endeMs));
        if (xEnde > xClips)
        {
            QColor strich = fest;
            strich.setAlpha(90);
            p.fillRect(QRect(xClips, kLinealHoehe, xEnde - xClips, unten - kLinealHoehe),
                       QBrush(strich, Qt::BDiagPattern));
        }
        p.setPen(QPen(fest, 2.0));
        p.drawLine(xEnde, 0, xEnde, unten);
        p.setBrush(fest);
        p.drawPolygon(QPolygon({QPoint(xEnde - 6, 0), QPoint(xEnde + 6, 0), QPoint(xEnde, 9)}));
        p.setBrush(Qt::NoBrush);
    }
    else
    {
        p.setPen(linie);
        p.drawLine(xEnde, 0, xEnde, unten);
    }

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

        for (int c = 0; c < static_cast<int>(kanal.clips.size()); ++c)
        {
            const VisualClip& clip = kanal.clips[static_cast<std::size_t>(c)];
            const QRect r = clipRechteck(zeile, clip);
            const bool gewaehlt = k == m_kanal && c == m_clip;
            const QColor farbe = kanalFarbe(k, m_an && kanal.an);
            p.setPen(gewaehlt ? QPen(pal.color(QPalette::Highlight), 2.0) : QPen(farbe.darker(150), 1.0));
            p.setBrush(farbe);
            p.drawRoundedRect(r, 4, 4);

            // die Pegelkurve: Hoehe im Clip = Pegel
            if (!clip.vollerPegel() && r.width() > 8 && clip.dauerMs() > 0)
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

            // das Teil, an dem der Curve Editor haengt: ein Strich unten im Clip
            if (gewaehlt && hatTeil(clip, m_teil))
            {
                const Bereich bereich = bereichVon(clip, m_teil);
                const int x0 = std::max(r.left() + 1, xVon(clip.startMs + bereich.vonMs));
                const int x1 = std::min(r.right() - 1, xVon(clip.startMs + bereich.bisMs));
                if (x1 > x0) p.fillRect(QRect(x0, r.bottom() - 3, x1 - x0, 3), QColor(255, 220, 120));
            }

            p.setPen(Qt::white);
            p.drawText(r.adjusted(6, 0, -4, 0), Qt::AlignLeft | Qt::AlignVCenter,
                       fontMetrics().elidedText(QFileInfo(clip.datei).completeBaseName(),
                                                Qt::ElideRight, std::max(0, r.width() - 10)));

            // Griffe der Blenden (C4), nur im Bearbeiten: oben im Clip je ein
            // Quadrat fuer die Laenge; am angeklickten Clip die Bézier-Anfasser
            if (m_bearbeiten)
            {
                for (const bool ein : {true, false})
                {
                    const QPoint griff = blendenGriff(k, c, ein);
                    if (griff.x() < 0) continue;
                    p.setPen(QPen(QColor(30, 30, 30), 1.0));
                    p.setBrush(Qt::white);
                    p.drawRect(QRect(griff.x() - 3, griff.y() - 3, 7, 7));
                    if (!gewaehlt) continue;
                    const QPoint a1 = bezierGriff(k, c, ein, 0);
                    const QPoint a2 = bezierGriff(k, c, ein, 1);
                    if (a1.x() < 0) continue;
                    // die Hebel: vom Fuss der Blende (Pegel null) zum ersten, von
                    // ihrem Kopf (Grundpegel) zum zweiten Anfasser — die
                    // Ausblendung laeuft rueckwaerts, ihr Fuss liegt rechts
                    QRect br;
                    int x0 = 0;
                    int x1 = 0;
                    Blenden b;
                    if (!blendenBereich(k, c, ein, &br, &x0, &x1, &b)) continue;
                    const int unten0 = r.bottom() - 1;
                    const int oben0 =
                        r.bottom() - 1 - static_cast<int>(clip.grundpegel * (r.height() - 3) + 0.5);
                    p.setPen(QPen(QColor(255, 220, 120), 1.0));
                    p.drawLine(QPoint(ein ? x0 : x1, unten0), a1);
                    p.drawLine(QPoint(ein ? x1 : x0, oben0), a2);
                    p.setBrush(QColor(255, 220, 120));
                    p.drawEllipse(a1, 4, 4);
                    p.drawEllipse(a2, 4, 4);
                }
                // Griffe der Stuecke: eine Raute in der Mitte (verschieben), je
                // ein Strich an den Kanten (Weite)
                p.setPen(QPen(QColor(30, 30, 30), 1.0));
                p.setBrush(Qt::white);
                for (int s = 0; s < static_cast<int>(clip.stuecke.size()); ++s)
                {
                    const QPoint mitte = stueckGriff(k, c, s, 0);
                    if (mitte.x() < 0) continue;
                    p.drawPolygon(QPolygon({QPoint(mitte.x(), mitte.y() - 4), QPoint(mitte.x() + 4, mitte.y()),
                                            QPoint(mitte.x(), mitte.y() + 4), QPoint(mitte.x() - 4, mitte.y())}));
                    for (const int seite : {-1, 1})
                    {
                        const QPoint kante = stueckGriff(k, c, s, seite);
                        if (kante.x() >= 0) p.drawRect(QRect(kante.x() - 1, kante.y() - 4, 3, 8));
                    }
                }
                p.setBrush(Qt::NoBrush);
            }
        }
    }
    p.setPen(linie);
    p.drawLine(kKopfBreite - 1, 0, kKopfBreite - 1, unten);

    // --- ein Drag von aussen: die Zeile, in die er faellt, und die Stelle
    if (m_fallOrt.x() >= 0 && m_fallOrt.y() >= kLinealHoehe)
    {
        const int k = kanalBei(m_fallOrt.y());
        const int zeile = k >= 0 ? k : zeilen;  // unter der letzten Zeile: ein neuer Kanal
        const QRect ziel(kKopfBreite, kLinealHoehe + zeile * kZeilenHoehe, width() - kKopfBreite,
                         kZeilenHoehe);
        QColor hell = pal.color(QPalette::Highlight);
        hell.setAlpha(60);
        p.fillRect(ziel, hell);
        const int xFall = xVon(raste(stelleVon(m_fallOrt.x()), m_komposition, -1, -1));
        p.setPen(QPen(pal.color(QPalette::Highlight), 2.0));
        p.drawLine(xFall, ziel.top(), xFall, ziel.bottom());
        if (k < 0)
        {
            p.setPen(text);
            p.drawText(QRect(8, ziel.top(), kKopfBreite - 12, kZeilenHoehe),
                       Qt::AlignLeft | Qt::AlignVCenter, tr("New channel"));
        }
    }

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
    updateGeometry();
    update();
}

void Zeitachse::zeigeBearbeitet(const Komposition& komposition)
{
    m_griff = Griff::Keiner;
    m_zugLaeuft = false;
    m_komposition = komposition;
    m_laengeMs = anzeigeLaenge(komposition);
    m_stelleMs = std::min(m_stelleMs, komposition.laenge());
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
        const auto& clips = in.kanaele[static_cast<std::size_t>(k)].clips;
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

QPoint Zeitachse::stueckGriff(int kanal, int clip, int stueck, int welcher) const
{
    if (kanal < 0 || kanal >= static_cast<int>(m_komposition.kanaele.size())) return {-1, -1};
    const auto& clips = m_komposition.kanaele[static_cast<std::size_t>(kanal)].clips;
    if (clip < 0 || clip >= static_cast<int>(clips.size())) return {-1, -1};
    const VisualClip& c = clips[static_cast<std::size_t>(clip)];
    if (stueck < 0 || stueck >= static_cast<int>(c.stuecke.size())) return {-1, -1};
    const QRect r = clipRechteck(kanal, c);  // Zeile = Kanal (CH1 oben)
    if (r.width() < 24) return {-1, -1};     // zu schmal fuer Griffe
    // wo das Stueck gerade liegt — gestaucht, wenn der Platz nicht reicht
    const Bereich b = c.verteilung().stuecke[static_cast<std::size_t>(stueck)];
    const int x0 = xVon(c.startMs + b.vonMs);
    const int x1 = xVon(c.startMs + b.bisMs);
    if (welcher == 0) return {(x0 + x1) / 2, r.top() + 5};
    if (x1 - x0 < 28) return {-1, -1};  // die Kanten laegen auf der Raute
    return {welcher < 0 ? x0 : x1, r.top() + 5};
}

Zeitachse::Griff Zeitachse::griffBei(QPoint ort, int* kanal, int* clip, int* stueck) const
{
    *kanal = -1;
    *clip = -1;
    if (stueck != nullptr) *stueck = -1;
    const int zeile = ort.y() < kLinealHoehe ? -1 : (ort.y() - kLinealHoehe) / kZeilenHoehe;
    if (zeile < 0 || zeile >= static_cast<int>(m_komposition.kanaele.size())) return Griff::Keiner;
    const int k = kanalVonZeile(zeile);
    const auto& clips = m_komposition.kanaele[static_cast<std::size_t>(k)].clips;
    for (int c = 0; c < static_cast<int>(clips.size()); ++c)
    {
        const QRect r = clipRechteck(zeile, clips[static_cast<std::size_t>(c)]);
        if (!r.contains(ort)) continue;
        *kanal = k;
        *clip = c;
        // Blenden (C4): zuerst die Bézier-Anfasser des angeklickten Clips,
        // dann die Griffe oben im Clip — sie gehen den Kanten vor
        const auto nah = [&ort](QPoint p) { return p.x() >= 0 && (ort - p).manhattanLength() <= 8; };
        if (k == m_kanal && c == m_clip)
        {
            if (nah(bezierGriff(k, c, true, 0))) return Griff::BezierEin1;
            if (nah(bezierGriff(k, c, true, 1))) return Griff::BezierEin2;
            if (nah(bezierGriff(k, c, false, 0))) return Griff::BezierAus1;
            if (nah(bezierGriff(k, c, false, 1))) return Griff::BezierAus2;
        }
        if (ort.y() <= r.top() + 11)
        {
            // Stuecke (C4 Teil 2): die Raute in der Mitte geht vor, dann die
            // Blenden, zuletzt die Kanten der Stuecke
            const int stuecke = static_cast<int>(clips[static_cast<std::size_t>(c)].stuecke.size());
            const auto trifft = [&](int welcher, Griff griff) {
                for (int s = 0; s < stuecke; ++s)
                {
                    if (!nah(stueckGriff(k, c, s, welcher))) continue;
                    if (stueck != nullptr) *stueck = s;
                    return griff;
                }
                return Griff::Keiner;
            };
            if (trifft(0, Griff::StueckLage) != Griff::Keiner) return Griff::StueckLage;
            if (nah(blendenGriff(k, c, true))) return Griff::BlendeEin;
            if (nah(blendenGriff(k, c, false))) return Griff::BlendeAus;
            if (trifft(-1, Griff::StueckLinks) != Griff::Keiner) return Griff::StueckLinks;
            if (trifft(1, Griff::StueckRechts) != Griff::Keiner) return Griff::StueckRechts;
        }
        // Kanten: die aeusseren sechs Pixel — bei schmalen Clips nur rechts
        if (r.width() > 16 && ort.x() - r.left() <= 6) return Griff::KanteLinks;
        if (r.right() - ort.x() <= 6) return Griff::KanteRechts;
        return Griff::Verschieben;
    }
    return Griff::Keiner;
}

bool Zeitachse::blendenBereich(int kanal, int clip, bool ein, QRect* clipRect, int* x0, int* x1,
                               Blenden* blenden) const
{
    if (kanal < 0 || kanal >= static_cast<int>(m_komposition.kanaele.size())) return false;
    const auto& clips = m_komposition.kanaele[static_cast<std::size_t>(kanal)].clips;
    if (clip < 0 || clip >= static_cast<int>(clips.size())) return false;
    const VisualClip& c = clips[static_cast<std::size_t>(clip)];
    // Die Laenge laesst sich an jeder Blende ziehen — auch an einer, deren Form
    // eigene Punkte hat (dann bleibt `blenden` fuer diese Seite ohne Form).
    (void)liesBlenden(c, *blenden);
    *clipRect = clipRechteck(kanal, c);  // Zeile = Kanal (CH1 oben)
    if (clipRect->width() < 24) return false;  // zu schmal fuer Griffe
    // wo die Blende gerade liegt — gestaucht, wenn der Platz nicht reicht
    const Verteilung v = c.verteilung();
    const Bereich& bereich = ein ? v.ein : v.aus;
    *x0 = xVon(c.startMs + bereich.vonMs);
    *x1 = xVon(c.startMs + bereich.bisMs);
    return true;
}

QPoint Zeitachse::blendenGriff(int kanal, int clip, bool ein) const
{
    QRect r;
    int x0 = 0;
    int x1 = 0;
    Blenden b;
    if (!blendenBereich(kanal, clip, ein, &r, &x0, &x1, &b)) return {-1, -1};
    return {std::clamp(ein ? x1 : x0, r.left() + 4, r.right() - 4), r.top() + 5};
}

QPoint Zeitachse::bezierGriff(int kanal, int clip, bool ein, int nr) const
{
    QRect r;
    int x0 = 0;
    int x1 = 0;
    Blenden b;
    if (!blendenBereich(kanal, clip, ein, &r, &x0, &x1, &b)) return {-1, -1};
    const VisualClip& c =
        m_komposition.kanaele[static_cast<std::size_t>(kanal)].clips[static_cast<std::size_t>(clip)];
    // Anfasser hier nur fuer eine Blende mit EINEM Bézier-Abschnitt; eine Form
    // mit eigenen Punkten gehoert in den Curve Editor
    const ClipBlende& blende = ein ? c.ein : c.aus;
    if (blende.dauerMs <= 0 || !einfach(blende.form) ||
        blende.form.punkte.front().form != Kurvenform::Bezier || x1 - x0 < 8)
        return {-1, -1};
    const BezierGriffe& g = blende.form.punkte.front().bezier;
    const double bx = g[static_cast<std::size_t>(nr * 2)];
    const double by = g[static_cast<std::size_t>(nr * 2 + 1)];
    // die Ausblendung ist dieselbe Form, rueckwaerts gelesen (an der y-Achse gespiegelt)
    const double x = ein ? bx : 1.0 - bx;
    const double pegel = by * c.grundpegel;
    return {x0 + static_cast<int>(x * (x1 - x0) + 0.5),
            r.bottom() - 1 - static_cast<int>(pegel * (r.height() - 3) + 0.5)};
}

void Zeitachse::setzeBlendenForm(int kanal, int clip, bool ein, Kurvenform form)
{
    if (!m_bearbeiten || !m_geladen) return;
    QRect r;
    int x0 = 0;
    int x1 = 0;
    Blenden b;
    if (!blendenBereich(kanal, clip, ein, &r, &x0, &x1, &b)) return;
    Komposition neu = m_komposition;
    VisualClip& c = neu.kanaele[static_cast<std::size_t>(kanal)].clips[static_cast<std::size_t>(clip)];
    // nur diese Seite: die andere behaelt ihre Form, auch eine mit eigenen Punkten
    ClipBlende& blende = ein ? c.ein : c.aus;
    const BezierGriffe griffe =
        einfach(blende.form) ? blende.form.punkte.front().bezier : kBezierVorgabe;
    blende.form = einfacheBlende(form, griffe);
    if (blende.dauerMs <= 0) blende.dauerMs = std::min<std::int64_t>(2000, c.dauerMs() / 4);
    uebernimm(neu, kanal, clip);
    Q_EMIT bearbeitet(neu, tr("Change fade shape"));
}

void Zeitachse::setzeBlendenPreset(int kanal, int clip, bool ein, const QString& name)
{
    if (!m_bearbeiten || !m_geladen) return;
    if (kanal < 0 || kanal >= static_cast<int>(m_komposition.kanaele.size())) return;
    Komposition neu = m_komposition;
    auto& clips = neu.kanaele[static_cast<std::size_t>(kanal)].clips;
    if (clip < 0 || clip >= static_cast<int>(clips.size())) return;
    Pegelform form;
    if (!formpresets::lade(formpresets::finde(FormArt::Blende, name).pfad, FormArt::Blende, form)) return;
    VisualClip& c = clips[static_cast<std::size_t>(clip)];
    ClipBlende& blende = ein ? c.ein : c.aus;
    blende.form = form;
    if (blende.dauerMs <= 0) blende.dauerMs = std::min<std::int64_t>(2000, c.dauerMs() / 4);
    uebernimm(neu, kanal, clip);
    Q_EMIT bearbeitet(neu, tr("Change fade shape"));
}

void Zeitachse::setzeStueckPreset(int kanal, int clip, int stueck, const QString& name)
{
    if (!m_bearbeiten || !m_geladen) return;
    if (kanal < 0 || kanal >= static_cast<int>(m_komposition.kanaele.size())) return;
    Komposition neu = m_komposition;
    auto& clips = neu.kanaele[static_cast<std::size_t>(kanal)].clips;
    if (clip < 0 || clip >= static_cast<int>(clips.size())) return;
    VisualClip& c = clips[static_cast<std::size_t>(clip)];
    if (stueck < 0 || stueck >= static_cast<int>(c.stuecke.size())) return;
    Pegelform form;
    if (!formpresets::lade(formpresets::finde(FormArt::Stueck, name).pfad, FormArt::Stueck, form)) return;
    c.stuecke[static_cast<std::size_t>(stueck)].form = form;
    uebernimm(neu, kanal, clip);
    Q_EMIT bearbeitet(neu, tr("Change shape curve"));
}

void Zeitachse::setTeil(Teil teil)
{
    if (teil == m_teil) return;
    m_teil = teil;
    update();
}

void Zeitachse::oeffneKurve(int kanal, int clip, Teil teil)
{
    if (!m_geladen || kanal < 0 || kanal >= static_cast<int>(m_komposition.kanaele.size())) return;
    const auto& clips = m_komposition.kanaele[static_cast<std::size_t>(kanal)].clips;
    if (clip < 0 || clip >= static_cast<int>(clips.size())) return;
    if (!hatTeil(clips[static_cast<std::size_t>(clip)], teil))
    {
        // eine Blende, die es noch nicht gibt, entsteht hier — sonst gaebe es nichts zu zeichnen
        if (!m_bearbeiten || (teil.art != TeilArt::Ein && teil.art != TeilArt::Aus)) return;
        Komposition neu = m_komposition;
        VisualClip& c = neu.kanaele[static_cast<std::size_t>(kanal)].clips[static_cast<std::size_t>(clip)];
        (teil.art == TeilArt::Ein ? c.ein : c.aus).dauerMs = std::min<std::int64_t>(2000, c.dauerMs() / 4);
        uebernimm(neu, kanal, clip);
        Q_EMIT bearbeitet(neu, tr("Add fade"));
    }
    else if (kanal != m_kanal || clip != m_clip)
    {
        m_kanal = kanal;
        m_clip = clip;
        update();
        Q_EMIT auswahlGeaendert();
    }
    Q_EMIT kurveOeffnen(teil);
}

void Zeitachse::stueckHinzu(int kanal, int clip, std::int64_t zeitImClipMs)
{
    if (!m_bearbeiten || !m_geladen) return;
    if (kanal < 0 || kanal >= static_cast<int>(m_komposition.kanaele.size())) return;
    Komposition neu = m_komposition;
    auto& clips = neu.kanaele[static_cast<std::size_t>(kanal)].clips;
    if (clip < 0 || clip >= static_cast<int>(clips.size())) return;
    const int stueck = fuegeStueckEin(clips[static_cast<std::size_t>(clip)], zeitImClipMs);
    uebernimm(neu, kanal, clip);
    Q_EMIT bearbeitet(neu, tr("Add shape"));
    Q_EMIT kurveOeffnen({TeilArt::Stueck, stueck});
}

void Zeitachse::stueckLoeschen(int kanal, int clip, int stueck)
{
    if (!m_bearbeiten || !m_geladen) return;
    if (kanal < 0 || kanal >= static_cast<int>(m_komposition.kanaele.size())) return;
    Komposition neu = m_komposition;
    auto& clips = neu.kanaele[static_cast<std::size_t>(kanal)].clips;
    if (clip < 0 || clip >= static_cast<int>(clips.size())) return;
    if (!loescheStueck(clips[static_cast<std::size_t>(clip)], stueck)) return;
    uebernimm(neu, kanal, clip);
    Q_EMIT bearbeitet(neu, tr("Delete shape"));
}

void Zeitachse::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (!m_geladen || event->button() != Qt::LeftButton) return;
    const QPoint ort = event->position().toPoint();
    int kanal = -1;
    int clip = -1;
    (void)griffBei(ort, &kanal, &clip);
    if (clip < 0) return;
    // der erste Klick des Doppelklicks hat einen Zug vorbereitet — er gilt nicht
    brichZugAb();
    const VisualClip& c = m_komposition.kanaele[static_cast<std::size_t>(kanal)].clips[static_cast<std::size_t>(clip)];
    const Teil teil = teilBei(c, stelleVon(ort.x()) - c.startMs);
    if (teil.art != TeilArt::Keins) oeffneKurve(kanal, clip, teil);
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
    int stueck = -1;
    const Griff griff = griffBei(ort, &kanal, &clipIndex, &stueck);
    if (m_bearbeiten && griff != Griff::Keiner)
    {
        m_griff = griff;
        m_zugStueck = stueck;
        m_zugLaeuft = false;
        m_zugStart = ort;
        m_zugKanal = kanal;
        m_zugClip = clipIndex;
        m_vorZug = m_komposition;
        m_zugVersatzMs =
            stelleVon(ort.x()) -
            m_komposition.kanaele[static_cast<std::size_t>(kanal)].clips[static_cast<std::size_t>(clipIndex)].startMs;
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
        m_vorZug.kanaele[static_cast<std::size_t>(m_zugKanal)].clips[static_cast<std::size_t>(m_zugClip)];
    const std::int64_t maus = stelleVon(ort.x());

    m_komposition = m_vorZug;
    int platz = -1;
    int ziel = m_zugKanal;
    switch (m_griff)
    {
        case Griff::Verschieben:
        {
            // die Zeile unter der Maus: der Clip wechselt den Kanal — nur in
            // einen seiner Art (Bild zu Bild, Ton zu Ton)
            const int zeilen = static_cast<int>(m_vorZug.kanaele.size());
            const int zeile = std::clamp((ort.y() - kLinealHoehe) / kZeilenHoehe, 0, zeilen - 1);
            if (ort.y() >= kLinealHoehe &&
                m_vorZug.kanaele[static_cast<std::size_t>(kanalVonZeile(zeile))].art ==
                    m_vorZug.kanaele[static_cast<std::size_t>(m_zugKanal)].art)
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
        case Griff::BlendeEin:
        case Griff::BlendeAus:
        case Griff::BezierEin1:
        case Griff::BezierEin2:
        case Griff::BezierAus1:
        case Griff::BezierAus2:
        {
            // Entscheid Patrik S96: ein gestauchtes Teil von Hand aendern heisst,
            // erst den sichtbaren Stand festschreiben — danach bewegt sich nur,
            // was angefasst wird
            VisualClip fest = alt;
            schreibeFest(fest);
            const std::int64_t dauer = fest.dauerMs();
            // die Laenge: nur die Dauer aendert sich, die Form bleibt — auch eine mit eigenen Punkten
            if (m_griff == Griff::BlendeEin)
                fest.ein.dauerMs = std::clamp<std::int64_t>(
                    gerastet(maus) - alt.startMs, 0, std::max<std::int64_t>(0, dauer - fest.aus.dauerMs));
            else if (m_griff == Griff::BlendeAus)
                fest.aus.dauerMs = std::clamp<std::int64_t>(
                    alt.endeMs - gerastet(maus), 0, std::max<std::int64_t>(0, dauer - fest.ein.dauerMs));
            else
            {
                // ein Bézier-Anfasser: seine Lage im Rechteck der Blende, je 0…1
                const bool ein = m_griff == Griff::BezierEin1 || m_griff == Griff::BezierEin2;
                const std::size_t nr = (m_griff == Griff::BezierEin2 || m_griff == Griff::BezierAus2) ? 1 : 0;
                const QRect r = clipRechteck(m_zugKanal, alt);
                const Verteilung v = fest.verteilung();
                const Bereich& bereich = ein ? v.ein : v.aus;
                const int x0 = xVon(alt.startMs + bereich.vonMs);
                const int x1 = xVon(alt.startMs + bereich.bisMs);
                const double x = std::clamp(static_cast<double>(ort.x() - x0) / std::max(1, x1 - x0), 0.0, 1.0);
                const double hoehe = std::max(1.0, fest.grundpegel * (r.height() - 3));
                const double pegel =
                    std::clamp(static_cast<double>(r.bottom() - 1 - ort.y()) / hoehe, 0.0, 1.0);
                Pegelform& form = (ein ? fest.ein : fest.aus).form;
                if (!einfach(form)) return;
                BezierGriffe& g = form.punkte.front().bezier;
                g[nr * 2] = ein ? x : 1.0 - x;  // die Ausblendung laeuft rueckwaerts
                g[nr * 2 + 1] = pegel;
            }
            m_komposition.kanaele[static_cast<std::size_t>(m_zugKanal)]
                .clips[static_cast<std::size_t>(m_zugClip)] = fest;
            platz = m_zugClip;
            break;
        }
        case Griff::StueckLage:
        case Griff::StueckLinks:
        case Griff::StueckRechts:
        {
            // Beide schreiben zuerst den sichtbaren Stand fest (Entscheid Patrik
            // S96) und halten an den Nachbarn an: gezogen wird nie ins Stauchen.
            VisualClip fest = alt;
            const std::int64_t imClip = gerastet(maus) - alt.startMs;
            if (m_griff == Griff::StueckLage)
            {
                if (!verschiebeStueck(fest, m_zugStueck, imClip)) return;
            }
            else
            {
                // die Mitte bleibt: der Abstand der Maus zur Mitte ist die halbe Weite
                const Bereich jetzt = bereichVon(alt, {TeilArt::Stueck, m_zugStueck});
                const std::int64_t mitte = (jetzt.vonMs + jetzt.bisMs) / 2;
                const std::int64_t halb = imClip > mitte ? imClip - mitte : mitte - imClip;
                if (!setzeStueckWeite(fest, m_zugStueck, 2 * halb)) return;
            }
            m_komposition.kanaele[static_cast<std::size_t>(m_zugKanal)]
                .clips[static_cast<std::size_t>(m_zugClip)] = fest;
            platz = m_zugClip;
            break;
        }
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
        m_laengeMs = anzeigeLaenge(m_komposition);
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
        else if (griff != Griff::Keiner && griff != Griff::Verschieben)
            setCursor(Qt::PointingHandCursor);  // ein Griff der Blende
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
    m_laengeMs = anzeigeLaenge(m_komposition);
    update();
    if (gleich(m_komposition, m_vorZug)) return;
    Q_EMIT auswahlGeaendert();  // der Clip hat neue Zeiten
    QString was = tr("Change fade");
    switch (griff)
    {
        case Griff::Verschieben: was = tr("Move clip"); break;
        case Griff::KanteLinks:
        case Griff::KanteRechts: was = tr("Resize clip"); break;
        case Griff::StueckLage: was = tr("Move shape"); break;
        case Griff::StueckLinks:
        case Griff::StueckRechts: was = tr("Change shape width"); break;
        default: break;
    }
    Q_EMIT bearbeitet(m_komposition, was);
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
        loescheAuswahl();
        return;
    }
    QWidget::keyPressEvent(event);
}

void Zeitachse::loescheAuswahl()
{
    Komposition neu = m_komposition;
    if (!loescheClip(neu, m_kanal, m_clip)) return;
    uebernimm(neu, -1, -1);
    Q_EMIT bearbeitet(neu, tr("Delete clip"));
}

void Zeitachse::uebernimm(const Komposition& neu, int kanal, int clip)
{
    m_komposition = neu;
    m_laengeMs = anzeigeLaenge(neu);
    m_kanal = kanal;
    m_clip = clip;
    updateGeometry();
    update();
    Q_EMIT auswahlGeaendert();
}

// -----------------------------------------------------------------------------
// Neues hinein (C3b)
// -----------------------------------------------------------------------------

int Zeitachse::kanalBei(int y) const
{
    if (y < kLinealHoehe) return -1;
    const int zeile = (y - kLinealHoehe) / kZeilenHoehe;
    return zeile < static_cast<int>(m_komposition.kanaele.size()) ? kanalVonZeile(zeile) : -1;
}

bool Zeitachse::lasseFallen(QPoint ort, const QStringList& dateien, Qt::KeyboardModifiers tasten)
{
    if (!m_bearbeiten || !m_geladen || dateien.isEmpty() || m_griff != Griff::Keiner) return false;
    if (ort.y() < kLinealHoehe) return false;
    Komposition neu = m_komposition;
    int kanal = kanalBei(ort.y());
    if (kanal < 0) kanal = fuegeKanalEin(neu);  // unter der letzten Zeile: ein neuer Kanal
    if (neu.kanaele[static_cast<std::size_t>(kanal)].art != KanalArt::Visual) return false;

    const Stoss stoss = !(tasten & Qt::ControlModifier) ? Stoss::Halten
                        : (tasten & Qt::ShiftModifier)  ? Stoss::Schieben
                                                        : Stoss::Kuerzen;
    std::int64_t start = stelleVon(ort.x());
    if (!(tasten & Qt::AltModifier)) start = raste(start, m_komposition, -1, -1);
    int platz = -1;
    for (const QString& datei : dateien)  // mehrere Dateien: hintereinander
    {
        platz = fuegeClipEin(neu, kanal, datei, start, kNeuerClipMs, stoss);
        if (platz < 0) return false;
        start = neu.kanaele[static_cast<std::size_t>(kanal)].clips[static_cast<std::size_t>(platz)].endeMs;
    }
    uebernimm(neu, kanal, platz);
    Q_EMIT bearbeitet(neu, dateien.size() == 1 ? tr("Add clip") : tr("Add clips"));
    Q_EMIT hineingezogen(dateien);
    return true;
}

void Zeitachse::kanalHinzu()
{
    if (!m_bearbeiten || !m_geladen) return;
    Komposition neu = m_komposition;
    (void)fuegeKanalEin(neu);
    uebernimm(neu, m_kanal, m_clip);
    Q_EMIT bearbeitet(neu, tr("Add channel"));
}

void Zeitachse::kanalLoeschen(int kanal)
{
    if (!m_bearbeiten || !m_geladen) return;
    Komposition neu = m_komposition;
    if (!loescheKanal(neu, kanal)) return;
    uebernimm(neu, -1, -1);
    Q_EMIT bearbeitet(neu, tr("Delete channel"));
}

void Zeitachse::kanalUmbenennen(int kanal, const QString& name)
{
    if (!m_bearbeiten || !m_geladen || name.trimmed().isEmpty()) return;
    if (kanal < 0 || kanal >= static_cast<int>(m_komposition.kanaele.size())) return;
    Komposition neu = m_komposition;
    neu.kanaele[static_cast<std::size_t>(kanal)].name = name.trimmed();
    uebernimm(neu, m_kanal, m_clip);
    Q_EMIT bearbeitet(neu, tr("Rename channel"));
}

void Zeitachse::kanalSchalten(int kanal, bool an)
{
    if (!m_bearbeiten || !m_geladen) return;
    if (kanal < 0 || kanal >= static_cast<int>(m_komposition.kanaele.size())) return;
    Komposition neu = m_komposition;
    neu.kanaele[static_cast<std::size_t>(kanal)].an = an;
    uebernimm(neu, m_kanal, m_clip);
    Q_EMIT bearbeitet(neu, an ? tr("Enable channel") : tr("Disable channel"));
}

void Zeitachse::dragEnterEvent(QDragEnterEvent* event)
{
    if (m_bearbeiten && m_geladen && !lumi::ui::presetDateien(event->mimeData()).isEmpty())
    {
        m_fallOrt = event->position().toPoint();
        event->acceptProposedAction();
        update();
    }
}

void Zeitachse::dragMoveEvent(QDragMoveEvent* event)
{
    if (m_fallOrt.x() < 0) return;
    m_fallOrt = event->position().toPoint();
    // im Lineal faellt nichts
    if (m_fallOrt.y() < kLinealHoehe)
        event->ignore();
    else
        event->acceptProposedAction();
    update();
}

void Zeitachse::dragLeaveEvent(QDragLeaveEvent*)
{
    m_fallOrt = QPoint(-1, -1);
    update();
}

void Zeitachse::dropEvent(QDropEvent* event)
{
    m_fallOrt = QPoint(-1, -1);
    update();
    if (lasseFallen(event->position().toPoint(), lumi::ui::presetDateien(event->mimeData()),
                    event->modifiers()))
        event->acceptProposedAction();
}

void Zeitachse::contextMenuEvent(QContextMenuEvent* event)
{
    if (!m_bearbeiten || !m_geladen) return;
    const QPoint ort = event->pos();
    int kanal = -1;
    int clip = -1;
    (void)griffBei(ort, &kanal, &clip);
    const int zeile = kanalBei(ort.y());

    QMenu menu(this);
    if (clip >= 0)
    {
        if (kanal != m_kanal || clip != m_clip)
        {
            m_kanal = kanal;
            m_clip = clip;
            update();
            Q_EMIT auswahlGeaendert();
        }
        // Blenden (C4): die Form je Seite. Eine Form mit eigenen Punkten traegt
        // keinen Haken — eine Wahl hier ersetzt sie durch die einfache Form.
        {
            const VisualClip& gewaehlt =
                m_komposition.kanaele[static_cast<std::size_t>(kanal)].clips[static_cast<std::size_t>(clip)];
            // Die Liste sind die Blenden-Presets — mitgelieferte und eigene; der
            // Haken sitzt an dem, dessen Form die Blende gerade traegt.
            const std::vector<formpresets::Eintrag> presets = formpresets::liste(FormArt::Blende);
            for (const bool ein : {true, false})
            {
                const ClipBlende& blende = ein ? gewaehlt.ein : gewaehlt.aus;
                const bool zeigt = blende.dauerMs > 0 && einfach(blende.form);
                QMenu* formen = menu.addMenu(ein ? tr("Fade in") : tr("Fade out"));
                bool gefunden = false;
                for (const formpresets::Eintrag& e : presets)
                {
                    QAction* a = formen->addAction(e.name);
                    a->setCheckable(true);
                    Pegelform f;
                    if (!gefunden && blende.dauerMs > 0 && formpresets::lade(e.pfad, FormArt::Blende, f) &&
                        gleich(f, blende.form))
                    {
                        a->setChecked(true);
                        gefunden = true;
                    }
                    connect(a, &QAction::triggered, this, [this, kanal, clip, ein, name = e.name] {
                        setzeBlendenPreset(kanal, clip, ein, name);
                    });
                }
                if (!presets.empty()) continue;
                // ohne Preset-Ordner (die App findet `asset/composer` nicht): die sieben festen Formen
                for (int f = 0; f < kKurvenformen; ++f)
                {
                    const auto form = static_cast<Kurvenform>(f);
                    QAction* a = formen->addAction(QString::fromLatin1(kurvenformName(form)));
                    a->setCheckable(true);
                    a->setChecked(zeigt && blende.form.punkte.front().form == form);
                    connect(a, &QAction::triggered, this, [this, kanal, clip, ein, form] {
                        setzeBlendenForm(kanal, clip, ein, form);
                    });
                }
            }
            menu.addSeparator();
        }
        // die Kurven im Curve Editor (C4 Teil 2) und die Stuecke mitten im Clip
        {
            const VisualClip& c =
                m_komposition.kanaele[static_cast<std::size_t>(kanal)].clips[static_cast<std::size_t>(clip)];
            const std::int64_t zeit = stelleVon(ort.x()) - c.startMs;
            const Teil unter = teilBei(c, zeit);
            menu.addAction(tr("Edit fade in curve..."), this,
                           [this, kanal, clip] { oeffneKurve(kanal, clip, {TeilArt::Ein, -1}); });
            menu.addAction(tr("Edit fade out curve..."), this,
                           [this, kanal, clip] { oeffneKurve(kanal, clip, {TeilArt::Aus, -1}); });
            if (unter.art == TeilArt::Stueck)
            {
                // die Stueck-Presets, wie bei den Blenden
                const ClipStueck& stueck = c.stuecke[static_cast<std::size_t>(unter.stueck)];
                QMenu* formen = menu.addMenu(tr("Shape"));
                bool gefunden = false;
                for (const formpresets::Eintrag& e : formpresets::liste(FormArt::Stueck))
                {
                    QAction* a = formen->addAction(e.name);
                    a->setCheckable(true);
                    Pegelform f;
                    if (!gefunden && formpresets::lade(e.pfad, FormArt::Stueck, f) && gleich(f, stueck.form))
                    {
                        a->setChecked(true);
                        gefunden = true;
                    }
                    connect(a, &QAction::triggered, this, [this, kanal, clip, unter, name = e.name] {
                        setzeStueckPreset(kanal, clip, unter.stueck, name);
                    });
                }
                formen->setEnabled(!formen->isEmpty());
                menu.addAction(tr("Edit shape curve..."), this,
                               [this, kanal, clip, unter] { oeffneKurve(kanal, clip, unter); });
                menu.addAction(tr("Delete shape"), this,
                               [this, kanal, clip, unter] { stueckLoeschen(kanal, clip, unter.stueck); });
            }
            menu.addAction(tr("Add shape here"), this,
                           [this, kanal, clip, zeit] { stueckHinzu(kanal, clip, zeit); });
            menu.addSeparator();
        }
        menu.addAction(tr("Delete clip"), this, [this] { loescheAuswahl(); });
        menu.addSeparator();
    }
    if (zeile >= 0)
    {
        const Kanal& k = m_komposition.kanaele[static_cast<std::size_t>(zeile)];
        menu.addAction(tr("Rename channel..."), this, [this, zeile, alt = k.name] {
            bool ok = false;
            const QString name = QInputDialog::getText(this, tr("Rename Channel"), tr("Name:"),
                                                       QLineEdit::Normal, alt, &ok);
            if (ok) kanalUmbenennen(zeile, name);
        });
        QAction* an = menu.addAction(tr("Channel enabled"));
        an->setCheckable(true);
        an->setChecked(k.an);
        connect(an, &QAction::triggered, this, [this, zeile](bool ein) { kanalSchalten(zeile, ein); });
        menu.addAction(tr("Delete channel"), this, [this, zeile] { kanalLoeschen(zeile); });
        menu.addSeparator();
    }
    menu.addAction(tr("Add channel"), this, [this] { kanalHinzu(); });
    menu.exec(event->globalPos());
}

// =============================================================================
// ComposerPanel
// =============================================================================

ComposerPanel::ComposerPanel(ServiceContainer& services, QWidget* parent)
    : PanelBase(services, "composer", tr("Composer"), parent)
{
    // Ueber diesen Bus haengt die Sitzung Effect Chain an einen Clip (C3c).
    Sitzung::setzeBus(eventBus());
    setupUI();
    zeigeKomposition();
}

ComposerPanel::~ComposerPanel()
{
    Sitzung::setzeBus(nullptr);  // der Bus gehoert den Diensten des Panels
}

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
    m_sync = new QCheckBox(tr("Sync on Effect Chain"), this);
    m_sync->setObjectName(QStringLiteral("composerSync"));
    m_sync->setToolTip(
        tr("While a composition delivers the picture:\n\n"
           "On: the Effect Chain panel shows the chain of the clip selected here. Changes "
           "act at once, in the mixed picture.\n\n"
           "Off: the Effect Chain panel rests.\n\n"
           "With the Composer off, Effect Chain shows the chain of the app as always."));
    kopf->addWidget(m_sync);
    kopf->addStretch(1);
    root->addLayout(kopf);

    // --- Datei-Zeile (eine eigene Zeile: der Kopf waere sonst breiter als ein
    //     schmal angedocktes Panel)
    kopf = new QHBoxLayout();
    m_neu = new QPushButton(tr("New"), this);
    m_neu->setObjectName(QStringLiteral("composerNew"));
    m_neu->setToolTip(tr("Start a new, empty composition with one channel. Drag presets from "
                         "the Effect Browser into it."));
    kopf->addWidget(m_neu);
    m_laden = new QPushButton(tr("Load..."), this);
    m_laden->setObjectName(QStringLiteral("composerLoad"));
    m_laden->setToolTip(tr("Open a composition — the same as File → Open Composition..."));
    kopf->addWidget(m_laden);
    m_speichern = new QPushButton(tr("Save"), this);
    m_speichern->setObjectName(QStringLiteral("composerSave"));
    m_speichern->setToolTip(tr("Write the changes into the loaded file"));
    kopf->addWidget(m_speichern);
    m_speichernUnter = new QPushButton(tr("Save As..."), this);
    m_speichernUnter->setObjectName(QStringLiteral("composerSaveAs"));
    kopf->addWidget(m_speichernUnter);
    kopf->addSpacing(8);
    m_datei = new QLabel(this);
    m_datei->setObjectName(QStringLiteral("composerDatei"));
    kopf->addWidget(m_datei);
    kopf->addStretch(1);
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
    // Das Ende der Komposition: folgt dem letzten Clip — oder steht fest
    m_endeFest = new QCheckBox(tr("Fixed end"), this);
    m_endeFest->setObjectName(QStringLiteral("composerFixedEnd"));
    m_endeFest->setToolTip(
        tr("Off: the composition ends where its last clip ends — shorten or move the last clip "
           "and the end follows.\n\n"
           "On: the composition ends at the time on the right, even when no clip is left; the "
           "empty stretch is hatched and the end carries its own marker. Loop and the end "
           "setting act there. Can be changed while Edit is on."));
    fuss->addWidget(m_endeFest);
    m_endeZeit = new QDoubleSpinBox(this);
    m_endeZeit->setObjectName(QStringLiteral("composerEndTime"));
    m_endeZeit->setDecimals(1);
    m_endeZeit->setSingleStep(1.0);
    m_endeZeit->setRange(0.1, 86400.0);
    m_endeZeit->setSuffix(tr(" s"));
    m_endeZeit->setKeyboardTracking(false);
    fuss->addWidget(m_endeZeit);
    fuss->addSpacing(12);
    connect(m_endeFest, &QCheckBox::clicked, this, [this](bool fest) {
        if (m_sitzung == nullptr || !m_sitzung->geladen()) return;
        Komposition neu = m_sitzung->komposition();
        neu.laengeMs = fest ? neu.laenge() : 0;
        bearbeite(neu, fest ? tr("Fix the end") : tr("Let the end follow the last clip"));
        zeigeEnde();
    });
    connect(m_endeZeit, &QDoubleSpinBox::valueChanged, this, [this](double sekunden) {
        if (m_sitzung == nullptr || !m_sitzung->geladen() || m_sitzung->komposition().laengeMs <= 0)
            return;
        Komposition neu = m_sitzung->komposition();
        neu.laengeMs = static_cast<std::int64_t>(sekunden * 1000.0 + 0.5);
        bearbeite(neu, tr("Change the end"));
    });
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
        zeigeEnde();
    });
    connect(m_sync, &QCheckBox::clicked, this, [this](bool an) {
        if (m_sitzung != nullptr) m_sitzung->setSync(an);
    });
    connect(m_zeitachse, &Zeitachse::bearbeitet, this,
            [this](const Komposition& neu, const QString& was) { bearbeite(neu, was); });
    connect(m_mischart, QOverload<int>::of(&QComboBox::activated), this, [this](int index) {
        const VisualClip* clip = m_zeitachse->auswahl();
        if (clip == nullptr || m_sitzung == nullptr || !m_zeitachse->bearbeiten()) return;
        Komposition neu = m_sitzung->komposition();
        neu.kanaele[static_cast<std::size_t>(m_zeitachse->gewaehlterKanal())]
            .clips[static_cast<std::size_t>(m_zeitachse->gewaehlterClip())]
            .mischart = static_cast<Mischart>(index);
        bearbeite(neu, tr("Change blend mode"));
    });
    connect(m_speichern, &QPushButton::clicked, this, [this] {
        if (m_fenster != nullptr) (void)speichereMitDialog(*m_fenster, this, false);
    });
    connect(m_speichernUnter, &QPushButton::clicked, this, [this] {
        if (m_fenster != nullptr) (void)speichereMitDialog(*m_fenster, this, true);
    });
    connect(m_neu, &QPushButton::clicked, this, [this] {
        sucheFenster();
        if (m_fenster == nullptr || !beginneKomposition(*m_fenster, this)) return;
        m_edit->setChecked(true);  // eine leere Komposition will bearbeitet werden
    });
    connect(m_laden, &QPushButton::clicked, this, [this] {
        sucheFenster();
        if (m_fenster != nullptr) (void)oeffneKomposition(*m_fenster, this);
    });
    connect(m_zeitachse, &Zeitachse::hineingezogen, this, [this](const QStringList& dateien) {
        // Entscheid S88 (1a): eine Datei mit Host Switcher laeuft als Clip
        // unveraendert — der Composer sagt beim Hinzufuegen, dass dann zwei wechseln.
        QStringList mit;
        for (const QString& datei : dateien)
            if (hatHostSwitcher(datei)) mit << QFileInfo(datei).fileName();
        if (mit.isEmpty()) return;
        QMessageBox::information(
            this, tr("Add Clip"),
            tr("%1 contains a Host Switcher. It keeps switching inside its clip — so two "
               "things change the picture: the Composer and the switcher in the file.")
                .arg(mit.join(QStringLiteral(", "))));
    });

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
    connect(m_zeitachse, &Zeitachse::kurveOeffnen, this, [this](Teil teil) { oeffneKurve(teil); });
}

KurvenFenster* ComposerPanel::kurvenFenster() const
{
    return m_kurven;
}

void ComposerPanel::oeffneKurve(Teil teil)
{
    m_kurvenTeil = teil;
    if (m_kurven == nullptr)
    {
        // ein eigenes Fenster des Panels (Entscheid Patrik S96) — keine Naht
        m_kurven = new KurvenFenster(this);
        connect(m_kurven, &KurvenFenster::clipGeaendert, this, [this](const VisualClip& clip, const QString& was) {
            if (m_sitzung == nullptr || !m_sitzung->geladen() || !m_zeitachse->bearbeiten()) return;
            const int k = m_zeitachse->gewaehlterKanal();
            const int c = m_zeitachse->gewaehlterClip();
            Komposition neu = m_sitzung->komposition();
            if (k < 0 || k >= static_cast<int>(neu.kanaele.size())) return;
            auto& clips = neu.kanaele[static_cast<std::size_t>(k)].clips;
            if (c < 0 || c >= static_cast<int>(clips.size())) return;
            // nur die Pegelkurve: Datei, Zeiten und Mischart gehoeren der Zeitachse
            VisualClip& ziel = clips[static_cast<std::size_t>(c)];
            ziel.grundpegel = clip.grundpegel;
            ziel.ein = clip.ein;
            ziel.aus = clip.aus;
            ziel.stuecke = clip.stuecke;
            bearbeite(neu, was);
        });
    }
    m_kurven->show();
    m_kurven->raise();
    zeigeKurve();
}

void ComposerPanel::zeigeKurve()
{
    if (m_kurven == nullptr) return;
    const VisualClip* clip = m_zeitachse->auswahl();
    m_kurven->zeige(clip, m_kurvenTeil,
                    clip != nullptr ? QFileInfo(clip->datei).completeBaseName() : QString(),
                    m_zeitachse->bearbeiten());
    m_zeitachse->setTeil(m_kurven->isVisible() && m_kurven->hatKurve() ? m_kurvenTeil : Teil{});
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
            // (der Curve Editor folgt ueber `auswahlGeaendert` → `zeigeClip`, auch nach Undo)
            zeigeEnde();
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
    zeigeEnde();
}

void ComposerPanel::zeigeEnde()
{
    const bool geladen = m_sitzung != nullptr && m_sitzung->geladen();
    const bool fest = geladen && m_sitzung->komposition().laengeMs > 0;
    const bool bearbeiten = geladen && m_zeitachse->bearbeiten();
    m_endeFest->setChecked(fest);
    m_endeFest->setEnabled(bearbeiten);
    m_endeZeit->setEnabled(bearbeiten && fest);
    const QSignalBlocker ruhe(m_endeZeit);
    m_endeZeit->setValue(geladen ? static_cast<double>(m_sitzung->laengeMs()) / 1000.0 : 0.1);
}

void ComposerPanel::zeigeDatei()
{
    const bool geladen = m_sitzung != nullptr && m_sitzung->geladen();
    const bool offen = geladen && m_sitzung->ungespeichert();
    const bool ohneDatei = geladen && m_sitzung->datei().isEmpty();  // neu, noch nie gespeichert
    // der Stern wie in der Preset-Zeile: es gibt Ungespeichertes
    m_datei->setText(geladen ? (ohneDatei ? tr("untitled") : QFileInfo(m_sitzung->datei()).fileName()) +
                                   (offen ? QStringLiteral(" *") : QString())
                             : QString());
    m_datei->setToolTip(geladen && !ohneDatei ? QDir::toNativeSeparators(m_sitzung->datei()) : QString());
    m_speichern->setEnabled(offen || ohneDatei);
    m_speichernUnter->setEnabled(geladen);
    m_edit->setEnabled(geladen);
    if (!geladen && m_edit->isChecked()) m_edit->setChecked(false);
    m_sync->setEnabled(m_sitzung != nullptr);
    m_sync->setChecked(m_sitzung != nullptr && m_sitzung->sync());
}

bool ComposerPanel::hatHostSwitcher(const QString& pfad)
{
    const QString endung = QFileInfo(pfad).suffix().toLower();
    if (endung != QLatin1String("lvfx") && endung != QLatin1String("lvfx2")) return false;
    lumi::multieffect::ChainNode wurzel;
    QStringList bericht;
    if (!lumi::multieffect::loadChainFromFile(pfad, wurzel, &bericht)) return false;
    const auto sucht = [](const auto& selbst, const lumi::multieffect::ChainNode& knoten) -> bool {
        if (std::holds_alternative<lumi::multieffect::HostSwitcherParams>(knoten.params)) return true;
        for (const lumi::multieffect::ChainNode& kind : knoten.children)
            if (selbst(selbst, kind)) return true;
        return false;
    };
    return sucht(sucht, wurzel);
}

void ComposerPanel::bearbeite(const Komposition& neu, const QString& was)
{
    if (m_sitzung == nullptr || !m_sitzung->geladen()) return;
    if (auto* bus = services().tryResolve<ICommandBus>())
        (void)bus->execute(std::make_unique<KompositionsBefehl>(*m_sitzung, neu, was));
    else
        m_sitzung->setzeKomposition(neu);
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
    zeigeKurve();  // der Curve Editor folgt dem angeklickten Clip
    // die Mischart gibt es nur am Bild
    const bool bild = clip != nullptr &&
                      m_zeitachse->komposition().kanaele[static_cast<std::size_t>(m_zeitachse->gewaehlterKanal())].art ==
                          KanalArt::Visual;
    m_mischart->setEnabled(bild && m_zeitachse->bearbeiten());
    // Sync on Effect Chain (C3c) folgt dem angeklickten Clip
    if (m_sitzung != nullptr)
        m_sitzung->waehleClip(clip != nullptr ? m_zeitachse->gewaehlterKanal() : -1,
                              clip != nullptr ? m_zeitachse->gewaehlterClip() : -1);
    if (clip == nullptr || m_sitzung == nullptr)
    {
        m_clipZeile->setText(m_sitzung != nullptr && m_sitzung->geladen() ? tr("No clip selected")
                                                                          : QString());
        m_clipZeile->setToolTip(QString());
        return;
    }
    m_mischart->setCurrentIndex(static_cast<int>(clip->mischart));
    // Der Kanal aus der Fassung der ZEITACHSE: waehrend eines Schritts ist sie
    // der Sitzung einen Augenblick voraus (ein eben entstandener Kanal).
    const Kanal& kanal =
        m_zeitachse->komposition().kanaele[static_cast<std::size_t>(m_zeitachse->gewaehlterKanal())];
    m_clipZeile->setText(QStringLiteral("%1 · %2 · %3 – %4 · %5")
                             .arg(kanal.name, QFileInfo(clip->datei).fileName(),
                                  zeitText(clip->startMs), zeitText(clip->endeMs),
                                  QString::fromLatin1(mischartName(clip->mischart))));
    m_clipZeile->setToolTip(QDir::toNativeSeparators(clip->datei));
}

}  // namespace lumi::composer
