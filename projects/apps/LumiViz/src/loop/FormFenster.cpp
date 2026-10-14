/**
 * @file FormFenster.cpp
 * @brief Klang aus einer Form: das Fenster „Edit shape" (Stufe 20a)
 */

#include "loop/FormFenster.hpp"

#include "loop/Instrument.hpp"  // eigeneInstrumenteMitForm, zaehleInstrumentFassung
#include "shape/BildKonturen.hpp"
#include "shape/SvgKonturen.hpp"
#include "shape/Zeichnen.hpp"

#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDialog>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QImageReader>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QLineF>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPushButton>
#include <QRadioButton>
#include <QSettings>
#include <QShortcut>
#include <QSignalBlocker>
#include <QSlider>
#include <QTimer>
#include <QSpinBox>
#include <QToolButton>
#include <QTransform>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

namespace lumi::loop
{

namespace
{

/// So viele Schritte merkt Undo.
constexpr std::size_t kSchritteMax = 100;
/// So gross ist eine leere Zeichnung — wie die mitgelieferten Formen (rund 200 Einheiten).
const QRectF kLeereAnsicht(0.0, 0.0, 200.0, 200.0);
/// Mit diesem Pegel spielt Play (der fernste Punkt der Form hat den Abstand 1).
constexpr float kPlayPegel = 0.4f;
/// Hier merkt sich Open SVG... den Ordner der letzten Datei.
const char* const kLetzterSvgOrdner = "shape/lastSvgFolder";
/// Und Open image... den seinen.
const char* const kLetzterBildOrdner = "shape/lastImageFolder";
/// Hier merkt sich das Programm den Haken Apply live (R2 a).
const char* const kBildLive = "shape/imageApplyLive";
/// So lange nach der letzten Bewegung eines Reglers wird erkannt.
constexpr int kBildPauseMs = 150;
/// So deckend steht das Bild hinter den Konturen.
constexpr double kBildDeckung = 0.35;

QString konturTitel(const shape::Kontur& kontur, int platz)
{
    return QStringLiteral("%1 %2").arg(platz + 1).arg(kontur.name.isEmpty() ? FormFenster::tr("Contour") : kontur.name);
}

/// Ein Farbpunkt fuer die Liste.
QIcon farbPunkt(const QString& farbe)
{
    QPixmap bild(12, 12);
    bild.fill(Qt::transparent);
    QPainter p(&bild);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(farbe));
    p.drawEllipse(QRectF(1.0, 1.0, 10.0, 10.0));
    return QIcon(bild);
}

}  // namespace

// =============================================================================
// FormFlaeche — die Zeichenflaeche
// =============================================================================

FormFlaeche::FormFlaeche(QWidget* parent) : QWidget(parent)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(280, 240);
    setCursor(Qt::CrossCursor);
}

void FormFlaeche::setZeichnung(const shape::Zeichnung* zeichnung)
{
    m_zeichnung = zeichnung;
    update();
}

void FormFlaeche::setAnsicht(const QRectF& ansicht)
{
    if (ansicht.width() > 0.0 && ansicht.height() > 0.0) m_ansicht = ansicht;
    update();
}

void FormFlaeche::setWerkzeug(Werkzeug werkzeug)
{
    if (werkzeug == m_werkzeug) return;
    // wer Pen verlaesst, behaelt, was sich schliessen laesst; der Rest ist zu wenig fuer eine Kontur
    if (!m_angefangen.isEmpty() && !schliesse()) verwirf();
    m_werkzeug = werkzeug;
    m_gedrueckt = false;
    m_ziehAnker = -1;
    setCursor(werkzeug == Werkzeug::Stift ? Qt::CrossCursor : Qt::ArrowCursor);
    update();
}

void FormFlaeche::setHervorgehoben(int kennung)
{
    m_hervorgehoben = kennung;
    update();
}

void FormFlaeche::setBild(const QImage* bild, const QTransform& lage, bool zeigen)
{
    m_bild = bild;
    m_bildLage = lage;
    m_zeigeBild = zeigen;
    update();
}

void FormFlaeche::setVorschau(std::vector<QPainterPath> vorschau)
{
    m_vorschau = std::move(vorschau);
    update();
}

double FormFlaeche::mass() const
{
    const double sx = width() / m_ansicht.width();
    const double sy = height() / m_ansicht.height();
    return std::max(1e-9, std::min(sx, sy));
}

QPointF FormFlaeche::imBild(QPointF punkt) const
{
    return QPointF(width() / 2.0, height() / 2.0) + (punkt - m_ansicht.center()) * mass();
}

QPointF FormFlaeche::inZeichnung(QPointF ort) const
{
    return m_ansicht.center() + (ort - QPointF(width() / 2.0, height() / 2.0)) / mass();
}

double FormFlaeche::einheitJePunkt() const
{
    return 1.0 / mass();
}

void FormFlaeche::klicke(QPointF punkt)
{
    // auf dem ersten Punkt: schliessen
    const std::vector<QPointF> anker = shape::ankerpunkte(m_angefangen);
    if (!anker.empty() && QLineF(imBild(anker.front()), imBild(punkt)).length() <= kFangRadius && schliesse()) return;
    if (shape::haengeEckeAn(m_angefangen, punkt))
    {
        update();
        Q_EMIT angefangenGeaendert();
    }
}

void FormFlaeche::ziehe(const std::vector<QPointF>& zug)
{
    if (zug.empty()) return;
    (void)shape::haengeFreihandAn(m_angefangen, zug, kGlaetten * einheitJePunkt());
    // ein Zug, der am ersten Punkt endet, schliesst die Kontur
    const std::vector<QPointF> anker = shape::ankerpunkte(m_angefangen);
    if (anker.size() > 1 && QLineF(imBild(anker.front()), imBild(zug.back())).length() <= kFangRadius && schliesse()) return;
    update();
    Q_EMIT angefangenGeaendert();
}

bool FormFlaeche::schliesse()
{
    if (!shape::laesstSichSchliessen(m_angefangen)) return false;
    QPainterPath pfad = m_angefangen;
    pfad.closeSubpath();
    m_angefangen = QPainterPath();
    update();
    Q_EMIT konturFertig(pfad);
    Q_EMIT angefangenGeaendert();
    return true;
}

void FormFlaeche::verwirf()
{
    if (m_angefangen.isEmpty()) return;
    m_angefangen = QPainterPath();
    update();
    Q_EMIT angefangenGeaendert();
}

std::pair<int, int> FormFlaeche::ankerBei(QPointF ort) const
{
    if (m_zeichnung == nullptr) return {0, -1};
    const QPointF punkt = inZeichnung(ort);
    const double radius = kFangRadius * einheitJePunkt();
    // die zuletzt gezeichnete liegt oben
    for (auto it = m_zeichnung->konturen.rbegin(); it != m_zeichnung->konturen.rend(); ++it)
        if (const int anker = shape::naechsterAnker(it->pfad, punkt, radius); anker >= 0) return {it->kennung, anker};
    return {0, -1};
}

void FormFlaeche::stelleZeiger(QPointF ort)
{
    if (m_werkzeug == Werkzeug::Stift)
        setCursor(Qt::CrossCursor);
    else if (m_ziehAnker >= 0)
        setCursor(Qt::ClosedHandCursor);
    else
        setCursor(ankerBei(ort).second >= 0 ? Qt::OpenHandCursor : Qt::ArrowCursor);
}

void FormFlaeche::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
    {
        QWidget::mousePressEvent(event);
        return;
    }
    setFocus();
    m_gedrueckt = true;
    m_druckOrt = event->position();
    m_istZug = false;
    if (m_werkzeug == Werkzeug::Stift)
    {
        m_zug = {inZeichnung(m_druckOrt)};
    }
    else
    {
        const auto [kennung, anker] = ankerBei(m_druckOrt);
        m_ziehKennung = kennung;
        m_ziehAnker = anker;
        stelleZeiger(m_druckOrt);
    }
}

void FormFlaeche::mouseMoveEvent(QMouseEvent* event)
{
    m_maus = event->position();
    if (m_gedrueckt && m_werkzeug == Werkzeug::Stift)
    {
        if (!m_istZug && QLineF(m_druckOrt, m_maus).length() > kZugAb) m_istZug = true;
        if (m_istZug) m_zug.push_back(inZeichnung(m_maus));
    }
    else if (m_gedrueckt && m_ziehAnker >= 0)
    {
        Q_EMIT ankerGezogen(m_ziehKennung, m_ziehAnker, inZeichnung(m_maus));
    }
    else
    {
        stelleZeiger(m_maus);
    }
    update();
}

void FormFlaeche::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton || !m_gedrueckt)
    {
        QWidget::mouseReleaseEvent(event);
        return;
    }
    m_gedrueckt = false;
    if (m_werkzeug == Werkzeug::Stift)
    {
        std::vector<QPointF> zug;
        zug.swap(m_zug);
        if (m_istZug)
            ziehe(zug);
        else
            klicke(inZeichnung(m_druckOrt));
    }
    else if (m_ziehAnker >= 0)
    {
        m_ziehAnker = -1;
        Q_EMIT ankerLosgelassen();
    }
    m_istZug = false;
    stelleZeiger(event->position());
    update();
}

void FormFlaeche::mouseDoubleClickEvent(QMouseEvent* event)
{
    // der erste Klick hat seinen Punkt schon gesetzt; der zweite schliesst
    if (event->button() == Qt::LeftButton && m_werkzeug == Werkzeug::Stift)
    {
        (void)schliesse();
        return;
    }
    QWidget::mouseDoubleClickEvent(event);
}

void FormFlaeche::keyPressEvent(QKeyEvent* event)
{
    if (m_werkzeug == Werkzeug::Stift && !m_angefangen.isEmpty())
    {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
        {
            (void)schliesse();
            return;
        }
        if (event->key() == Qt::Key_Escape)
        {
            verwirf();
            return;
        }
    }
    QWidget::keyPressEvent(event);
}

void FormFlaeche::leaveEvent(QEvent* event)
{
    m_maus = QPointF(-1.0, -1.0);
    update();
    QWidget::leaveEvent(event);
}

void FormFlaeche::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QPalette pal = palette();
    p.fillRect(rect(), pal.color(QPalette::Base));
    QColor leise = pal.color(QPalette::Text);
    leise.setAlpha(60);
    p.setPen(leise);
    p.drawRect(rect().adjusted(0, 0, -1, -1));

    QTransform bild;
    bild.translate(width() / 2.0, height() / 2.0);
    bild.scale(mass(), mass());
    bild.translate(-m_ansicht.center().x(), -m_ansicht.center().y());

    // das Bild blass dahinter (Show image)
    if (m_bild != nullptr && !m_bild->isNull() && m_zeigeBild)
    {
        p.save();
        p.setOpacity(kBildDeckung);
        p.setTransform(m_bildLage * bild, true);
        p.drawImage(QPointF(0.0, 0.0), *m_bild);
        p.restore();
    }
    // was From image ohne Apply live vorschlaegt: duenn und grau
    if (!m_vorschau.empty())
    {
        QColor grau = pal.color(QPalette::Text);
        grau.setAlpha(150);
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(grau, 1.2));
        for (const QPainterPath& v : m_vorschau) p.drawPath(bild.map(v));
    }

    const bool leer = (m_zeichnung == nullptr || m_zeichnung->konturen.empty()) && m_angefangen.isEmpty() && m_zug.empty() &&
                      (m_bild == nullptr || m_bild->isNull());
    if (leer)
    {
        QColor hinweis = pal.color(QPalette::Text);
        hinweis.setAlpha(110);
        p.setPen(hinweis);
        p.drawText(rect(), Qt::AlignCenter, tr("Drag or click to draw a contour"));
    }

    if (m_zeichnung != nullptr)
    {
        for (const shape::Kontur& k : m_zeichnung->konturen)
        {
            const QColor farbe(k.farbe);
            const QPainterPath zug = bild.map(k.pfad);
            p.setBrush(Qt::NoBrush);
            if (k.gewaehlt)
                p.setPen(QPen(farbe, k.kennung == m_hervorgehoben ? 3.2 : 2.0));
            else
                p.setPen(QPen(farbe, k.kennung == m_hervorgehoben ? 2.0 : 1.0, Qt::DashLine));
            p.drawPath(zug);
            const std::vector<QPointF> anker = shape::ankerpunkte(k.pfad);
            if (anker.empty()) continue;
            // der Startpunkt gefuellt; bei Move point jeder Punkt als Griff
            if (m_werkzeug == Werkzeug::Schieben)
            {
                p.setPen(QPen(farbe, 1.2));
                p.setBrush(pal.color(QPalette::Base));
                for (const QPointF& a : anker) p.drawRect(QRectF(imBild(a) - QPointF(3.5, 3.5), QSizeF(7.0, 7.0)));
            }
            p.setPen(Qt::NoPen);
            p.setBrush(farbe);
            p.drawEllipse(imBild(anker.front()), 4.0, 4.0);
        }
    }

    // was Pen gerade zeichnet
    const QColor stift = pal.color(QPalette::Highlight);
    if (!m_angefangen.isEmpty())
    {
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(stift, 2.0));
        p.drawPath(bild.map(m_angefangen));
        const std::vector<QPointF> anker = shape::ankerpunkte(m_angefangen);
        p.setPen(Qt::NoPen);
        p.setBrush(stift);
        for (const QPointF& a : anker) p.drawEllipse(imBild(a), 2.5, 2.5);
        // der erste Punkt als Ring: dort schliesst ein Klick
        if (!anker.empty())
        {
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(stift, 1.5));
            p.drawEllipse(imBild(anker.front()), kFangRadius, kFangRadius);
        }
        // die Strecke, die der naechste Klick setzen wuerde
        if (!m_gedrueckt && m_maus.x() >= 0.0 && m_werkzeug == Werkzeug::Stift)
        {
            QColor gummi = stift;
            gummi.setAlpha(140);
            p.setPen(QPen(gummi, 1.0, Qt::DashLine));
            p.drawLine(imBild(m_angefangen.currentPosition()), m_maus);
        }
    }
    if (m_istZug && m_zug.size() > 1)
    {
        QPolygonF zug;
        for (const QPointF& z : m_zug) zug << imBild(z);
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(stift, 1.5));
        p.drawPolyline(zug);
    }
}

// =============================================================================
// FormVerlauf — Preview
// =============================================================================

FormVerlauf::FormVerlauf(QWidget* parent) : QWidget(parent)
{
    setMinimumHeight(70);
}

void FormVerlauf::setForm(const std::shared_ptr<const shape::Tonform>& form, double harmonische)
{
    m_links.clear();
    m_rechts.clear();
    m_grenzen.clear();
    if (form != nullptr && !form->leer())
    {
        constexpr int kPunkte = 400;
        m_links.reserve(kPunkte + 1);
        m_rechts.reserve(kPunkte + 1);
        for (int n = 0; n <= kPunkte; ++n)
        {
            const shape::Punkt punkt = shape::tonwert(*form, static_cast<double>(n % kPunkte) / kPunkte, harmonische);
            m_links.push_back(punkt.real());
            m_rechts.push_back(punkt.imag());
        }
        if (form->konturen.size() > 1)
        {
            double stelle = 0.0;
            for (std::size_t i = 0; i + 1 < form->konturen.size(); ++i)
            {
                stelle += form->konturen[i].anteil + form->sprung;
                m_grenzen.push_back(stelle);
            }
        }
    }
    update();
}

void FormVerlauf::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QPalette pal = palette();
    p.fillRect(rect(), pal.color(QPalette::Base));
    QColor leise = pal.color(QPalette::Text);
    leise.setAlpha(60);
    p.setPen(leise);
    p.drawRect(rect().adjusted(0, 0, -1, -1));
    const double mitte = height() / 2.0;
    p.drawLine(QPointF(0.0, mitte), QPointF(width(), mitte));
    if (m_links.size() < 2)
    {
        leise.setAlpha(110);
        p.setPen(leise);
        p.drawText(rect(), Qt::AlignCenter, tr("Tick a contour to hear it"));
        return;
    }
    QColor grenze = pal.color(QPalette::Text);
    grenze.setAlpha(120);
    p.setPen(QPen(grenze, 1.0, Qt::DashLine));
    for (const double g : m_grenzen) p.drawLine(QPointF(g * width(), 2.0), QPointF(g * width(), height() - 2.0));
    const double halb = height() / 2.0 - 4.0;
    const auto kurve = [&](const std::vector<double>& werte) {
        QPolygonF zug;
        const double schritt = static_cast<double>(width()) / static_cast<double>(werte.size() - 1);
        for (std::size_t i = 0; i < werte.size(); ++i) zug << QPointF(i * schritt, mitte - werte[i] * halb);
        return zug;
    };
    const QColor links = pal.color(QPalette::Highlight);
    // R in der Textfarbe: keine Farbe einer Kontur, damit die Kurve keiner zu gehoeren scheint
    const QColor rechts = pal.color(QPalette::Text);
    p.setPen(QPen(links, 1.5));
    p.drawPolyline(kurve(m_links));
    p.setPen(QPen(rechts, 1.5));
    p.drawPolyline(kurve(m_rechts));
    p.setPen(links);
    p.drawText(QPointF(6.0, 13.0), tr("L"));
    p.setPen(rechts);
    p.drawText(QPointF(18.0, 13.0), tr("R"));
}

// =============================================================================
// FormFenster
// =============================================================================

FormFenster::FormFenster(QWidget* parent) : QWidget(parent, Qt::Tool)
{
    setObjectName(QStringLiteral("loopEditShape"));
    setupUI();
    beginneNeu();
}

FormFenster::~FormFenster()
{
    halte();
    // die Kinder werden nach diesem Rumpf geloescht; was sie dabei melden (die Liste
    // ihre aktuelle Zeile), traefe ein halb abgebautes Fenster
    for (QObject* kind : findChildren<QObject*>()) kind->blockSignals(true);
}

void FormFenster::setupUI()
{
    auto* aussen = new QVBoxLayout(this);

    // --- Kopf: der Ort, die Wege von aussen (Z2 a: kein Draw)
    m_ort = new QLabel(this);
    m_ort->setObjectName(QStringLiteral("shapeLocation"));
    m_ort->setEnabled(false);
    m_svg = new QPushButton(tr("Open SVG..."), this);
    m_svg->setObjectName(QStringLiteral("shapeOpenSvg"));
    m_svg->setToolTip(tr("Take the contours of an SVG file — every closed or open path becomes a contour"));
    m_bild = new QPushButton(tr("Open image..."), this);
    m_bild->setObjectName(QStringLiteral("shapeOpenImage"));
    m_bild->setToolTip(tr("Find contours in a picture (PNG, JPEG, ...) — the picture is split into areas, the edge of each area is a contour"));
    auto* kopf = new QHBoxLayout;
    kopf->addWidget(m_ort, 1);
    kopf->addWidget(m_svg);
    kopf->addWidget(m_bild);
    aussen->addLayout(kopf);

    // --- Mitte: die Zeichenflaeche links, die Liste rechts
    auto* mitte = new QHBoxLayout;
    auto* links = new QVBoxLayout;
    m_flaeche = new FormFlaeche(this);
    m_flaeche->setObjectName(QStringLiteral("shapeCanvas"));
    m_flaeche->setZeichnung(&m_zeichnung);
    links->addWidget(m_flaeche, 1);
    const auto werkzeug = [this](const QString& text, const char* kennung, const QString& tipp) {
        auto* knopf = new QToolButton(this);
        knopf->setText(text);
        knopf->setObjectName(QLatin1String(kennung));
        knopf->setCheckable(true);
        knopf->setAutoExclusive(true);
        knopf->setToolButtonStyle(Qt::ToolButtonTextOnly);
        knopf->setToolTip(tipp);
        return knopf;
    };
    m_stift = werkzeug(tr("Pen"), "shapePen", tr("Draw a contour: drag to draw freehand, click to set corners"));
    m_schieben = werkzeug(tr("Move point"), "shapeMovePoint", tr("Drag a point of a contour to move it"));
    m_bruecke = werkzeug(tr("Bridge"), "shapeBridge", tr("Not available yet"));
    m_bruecke->setEnabled(false);
    m_zeigeBild = new QCheckBox(tr("Show image"), this);
    m_zeigeBild->setObjectName(QStringLiteral("shapeShowImage"));
    m_zeigeBild->setChecked(true);
    m_zeigeBild->setToolTip(tr("Show the picture faintly behind the contours"));
    auto* werkzeuge = new QHBoxLayout;
    werkzeuge->addWidget(m_stift);
    werkzeuge->addWidget(m_schieben);
    werkzeuge->addWidget(m_bruecke);
    werkzeuge->addStretch(1);
    werkzeuge->addWidget(m_zeigeBild);
    links->addLayout(werkzeuge);
    m_hinweis = new QLabel(this);
    m_hinweis->setObjectName(QStringLiteral("shapeHint"));
    m_hinweis->setEnabled(false);
    m_hinweis->setWordWrap(true);
    links->addWidget(m_hinweis);
    mitte->addLayout(links, 3);

    auto* rechts = new QVBoxLayout;
    m_listenKopf = new QLabel(this);
    m_listenKopf->setObjectName(QStringLiteral("shapeContoursTitle"));
    rechts->addWidget(m_listenKopf);
    m_liste = new QTreeWidget(this);
    m_liste->setObjectName(QStringLiteral("shapeContours"));
    m_liste->setColumnCount(2);
    m_liste->setHeaderHidden(true);
    m_liste->setRootIsDecorated(false);
    m_liste->header()->setStretchLastSection(false);
    m_liste->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_liste->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_liste->setToolTip(tr("Ticked contours make up the shape; the share is how much of one round of the pen each one takes"));
    rechts->addWidget(m_liste, 1);
    m_umbenennen = new QPushButton(tr("Rename"), this);
    m_umbenennen->setObjectName(QStringLiteral("shapeRename"));
    m_weg = new QPushButton(tr("Remove"), this);
    m_weg->setObjectName(QStringLiteral("shapeRemove"));
    m_hinauf = new QPushButton(QStringLiteral("↑"), this);
    m_hinauf->setObjectName(QStringLiteral("shapeUp"));
    m_hinauf->setToolTip(tr("Move up — the order in which the pen plays the contours"));
    m_hinunter = new QPushButton(QStringLiteral("↓"), this);
    m_hinunter->setObjectName(QStringLiteral("shapeDown"));
    m_hinunter->setToolTip(tr("Move down — the order in which the pen plays the contours"));
    for (QPushButton* knopf : {m_hinauf, m_hinunter}) knopf->setFixedWidth(30);
    auto* listenKnoepfe = new QHBoxLayout;
    listenKnoepfe->addWidget(m_umbenennen);
    listenKnoepfe->addWidget(m_weg);
    listenKnoepfe->addStretch(1);
    listenKnoepfe->addWidget(m_hinauf);
    listenKnoepfe->addWidget(m_hinunter);
    rechts->addLayout(listenKnoepfe);
    m_weggefallenZeile = new QLabel(this);
    m_weggefallenZeile->setObjectName(QStringLiteral("shapeLeftOut"));
    m_weggefallenZeile->setEnabled(false);
    m_weggefallenZeile->setToolTip(tr("A shape holds at most %1 contours: the longest ones are kept").arg(shape::kKonturenMax));
    rechts->addWidget(m_weggefallenZeile);
    mitte->addLayout(rechts, 2);
    aussen->addLayout(mitte, 1);

    // --- drei Gruppen: From image und Use as kommen mit 20c und 20d, Preview gilt
    auto* gruppen = new QHBoxLayout;
    {
        m_ausBild = new QGroupBox(tr("From image"), this);
        m_ausBild->setObjectName(QStringLiteral("shapeFromImage"));
        auto* g = new QGridLayout(m_ausBild);
        // die Reihenfolge wie shape::Teilung
        m_teilung = new QComboBox(m_ausBild);
        m_teilung->setObjectName(QStringLiteral("shapeSplitBy"));
        m_teilung->addItems({tr("Colour"), tr("Brightness"), tr("Transparency")});
        m_teilung->setToolTip(tr("Split the picture into areas by colour, by brightness, or by how opaque it is (for a cut-out PNG)"));
        g->addWidget(new QLabel(tr("Split by"), m_ausBild), 0, 0);
        g->addWidget(m_teilung, 0, 1, 1, 2);
        const auto regler = [this, g](int zeile, const QString& name, const char* kennung, int von, int bis, const QString& tipp,
                                      QSlider*& schieber, QLabel*& wert) {
            schieber = new QSlider(Qt::Horizontal, m_ausBild);
            schieber->setObjectName(QLatin1String(kennung));
            schieber->setRange(von, bis);
            schieber->setToolTip(tipp);
            wert = new QLabel(m_ausBild);
            wert->setMinimumWidth(36);
            g->addWidget(new QLabel(name, m_ausBild), zeile, 0);
            g->addWidget(schieber, zeile, 1);
            g->addWidget(wert, zeile, 2);
        };
        regler(1, tr("Levels"), "shapeLevels", 2, 8, tr("How many brightness steps, colours or steps of transparency"), m_stufen, m_stufenWert);
        regler(2, tr("Smooth"), "shapeSmooth", 0, 10, tr("Blur the picture before splitting it: smoother edges, fewer high partials"),
               m_glaetten, m_glaettenWert);
        regler(3, tr("Min size"), "shapeMinSize", 0, 50, tr("Areas smaller than this share of the picture merge into their largest neighbour"),
               m_mindest, m_mindestWert);
        regler(4, tr("Detail"), "shapeDetail", 0, 100, tr("How closely the edge is followed: more detail, more high partials"), m_detail,
               m_detailWert);
        m_liveHaken = new QCheckBox(tr("Apply live"), m_ausBild);
        m_liveHaken->setObjectName(QStringLiteral("shapeApplyLive"));
        m_liveHaken->setToolTip(tr("On: every change finds the contours again and replaces those from the picture. "
                                   "Off: the contours are only proposed in grey, Take contours takes them"));
        m_nehmen = new QPushButton(tr("Take contours"), m_ausBild);
        m_nehmen->setObjectName(QStringLiteral("shapeTakeContours"));
        m_nehmen->setToolTip(tr("Take the proposed contours into the shape"));
        g->addWidget(m_liveHaken, 5, 0, 1, 2);
        g->addWidget(m_nehmen, 5, 2);
        gruppen->addWidget(m_ausBild, 1);
    }
    {
        auto* nutzung = new QGroupBox(tr("Use as"), this);
        nutzung->setObjectName(QStringLiteral("shapeUseAs"));
        nutzung->setEnabled(false);
        nutzung->setToolTip(tr("Not available yet"));
        auto* g = new QVBoxLayout(nutzung);
        // grau, aber mit dem, was in der Datei steht (die Reihenfolge wie `shape::Nutzung`, `shape::Spruenge`)
        m_nutzungSchichten = new QRadioButton(tr("Layers, one per contour"), nutzung);
        m_nutzungSchichten->setObjectName(QStringLiteral("shapeUseAsLayers"));
        m_nutzungPfad = new QRadioButton(tr("One path"), nutzung);
        m_nutzungPfad->setObjectName(QStringLiteral("shapeUseAsOnePath"));
        g->addWidget(m_nutzungSchichten);
        g->addWidget(m_nutzungPfad);
        m_spruenge = new QComboBox(nutzung);
        m_spruenge->setObjectName(QStringLiteral("shapeJumps"));
        m_spruenge->addItems({tr("In list order"), tr("Shortest"), tr("Drawn by hand")});
        auto* zeile = new QHBoxLayout;
        zeile->addWidget(new QLabel(tr("Jumps"), nutzung));
        zeile->addWidget(m_spruenge, 1);
        g->addLayout(zeile);
        g->addStretch(1);
        gruppen->addWidget(nutzung, 1);
    }
    {
        auto* vorschau = new QGroupBox(tr("Preview"), this);
        vorschau->setObjectName(QStringLiteral("shapePreview"));
        auto* g = new QVBoxLayout(vorschau);
        m_verlauf = new FormVerlauf(vorschau);
        m_verlauf->setObjectName(QStringLiteral("shapePreviewWave"));
        m_verlauf->setToolTip(tr("One round of the pen over time: L is its sideways movement, R its up-and-down movement; "
                                 "a dashed line marks where the next contour begins"));
        g->addWidget(m_verlauf, 1);
        m_harmonischeRegler = new QSlider(Qt::Horizontal, vorschau);
        m_harmonischeRegler->setObjectName(QStringLiteral("shapeHarmonics"));
        m_harmonischeRegler->setRange(0, 360);  // wie im Creator: 40 Schritte je Verdopplung
        m_harmonischeRegler->setToolTip(tr("How many partials draw the shape — only for trying it out here"));
        m_harmonischeWert = new QLabel(vorschau);
        m_harmonischeWert->setMinimumWidth(32);
        m_play = new QPushButton(tr("▶ Play"), vorschau);
        m_play->setObjectName(QStringLiteral("shapePlay"));
        m_play->setCheckable(true);
        m_play->setToolTip(tr("Play the shape at A3, left sideways and right up-and-down — again and again"));
        auto* zeile = new QHBoxLayout;
        zeile->addWidget(new QLabel(tr("Harmonics"), vorschau));
        zeile->addWidget(m_harmonischeRegler, 1);
        zeile->addWidget(m_harmonischeWert);
        g->addLayout(zeile);
        g->addWidget(m_play);
        gruppen->addWidget(vorschau, 1);
    }
    aussen->addLayout(gruppen);

    // --- Fuss: wer die Form spielt, Cancel, Save as..., Save
    m_nutzer = new QLabel(this);
    m_nutzer->setObjectName(QStringLiteral("shapeUsedBy"));
    m_nutzer->setEnabled(false);
    auto* abbrechen = new QPushButton(tr("Cancel"), this);
    abbrechen->setObjectName(QStringLiteral("shapeCancel"));
    m_sichernAls = new QPushButton(tr("Save as..."), this);
    m_sichernAls->setObjectName(QStringLiteral("shapeSaveAs"));
    m_sichernAls->setToolTip(tr("Save as a new shape in My shapes and use it"));
    m_sichern = new QPushButton(tr("Save"), this);
    m_sichern->setObjectName(QStringLiteral("shapeSave"));
    auto* fuss = new QHBoxLayout;
    fuss->addWidget(m_nutzer, 1);
    fuss->addWidget(abbrechen);
    fuss->addWidget(m_sichernAls);
    fuss->addWidget(m_sichern);
    aussen->addLayout(fuss);

    connect(m_stift, &QToolButton::clicked, this, [this] { setzeWerkzeug(FormFlaeche::Werkzeug::Stift); });
    connect(m_schieben, &QToolButton::clicked, this, [this] { setzeWerkzeug(FormFlaeche::Werkzeug::Schieben); });
    connect(m_flaeche, &FormFlaeche::konturFertig, this, [this](const QPainterPath& pfad) { (void)fuegeKonturHinzu(pfad); });
    connect(m_flaeche, &FormFlaeche::angefangenGeaendert, this, &FormFenster::zeigeKnoepfe);
    connect(m_flaeche, &FormFlaeche::ankerGezogen, this, [this](int kennung, int anker, QPointF ziel) {
        // waehrend des Ziehens nur das Bild; gemerkt wird einmal, gerechnet beim Loslassen
        if (!m_schiebtGerade)
        {
            merke();
            m_schiebtGerade = true;
        }
        for (shape::Kontur& k : m_zeichnung.konturen)
            if (k.kennung == kennung) (void)shape::schiebeAnker(k.pfad, anker, ziel);
        m_flaeche->update();
    });
    connect(m_flaeche, &FormFlaeche::ankerLosgelassen, this, [this] {
        if (!m_schiebtGerade) return;
        m_schiebtGerade = false;
        geaendertJetzt();
    });
    connect(m_liste, &QTreeWidget::itemChanged, this, [this](QTreeWidgetItem* zeile, int spalte) {
        if (spalte != 0) return;
        setzeGewaehlt(zeile->data(0, Qt::UserRole).toInt(), zeile->checkState(0) == Qt::Checked);
    });
    connect(m_liste, &QTreeWidget::currentItemChanged, this, [this] {
        m_flaeche->setHervorgehoben(aktuelle());
        zeigeKnoepfe();
    });
    const auto umbenennen = [this] {
        const int kennung = aktuelle();
        const shape::Kontur* k = m_zeichnung.finde(kennung);
        if (k == nullptr) return;
        bool ok = false;
        const QString name = QInputDialog::getText(this, tr("Rename contour"), tr("Name"), QLineEdit::Normal, k->name, &ok);
        if (ok) benenne(kennung, name);
    };
    connect(m_umbenennen, &QPushButton::clicked, this, umbenennen);
    connect(m_liste, &QTreeWidget::itemDoubleClicked, this, umbenennen);
    connect(m_weg, &QPushButton::clicked, this, [this] { (void)entferne(aktuelle()); });
    connect(m_hinauf, &QPushButton::clicked, this, [this] { (void)schiebe(aktuelle(), -1); });
    connect(m_hinunter, &QPushButton::clicked, this, [this] { (void)schiebe(aktuelle(), +1); });
    connect(m_harmonischeRegler, &QSlider::valueChanged, this, [this](int w) { setzeHarmonische(std::exp2(w / 40.0)); });
    connect(m_play, &QPushButton::clicked, this, [this](bool an) {
        if (an)
            (void)spiele();
        else
            halte();
        zeigeKnoepfe();
    });
    connect(abbrechen, &QPushButton::clicked, this, &QWidget::close);
    connect(m_svg, &QPushButton::clicked, this, &FormFenster::frageNachSvg);
    connect(m_bild, &QPushButton::clicked, this, &FormFenster::frageNachBild);
    connect(m_zeigeBild, &QCheckBox::toggled, this, [this](bool an) { m_flaeche->setBild(&m_bildDaten, m_bildLage, an); });
    m_bildTakt = new QTimer(this);
    m_bildTakt->setSingleShot(true);
    m_bildTakt->setInterval(kBildPauseMs);
    connect(m_bildTakt, &QTimer::timeout, this, &FormFenster::erkenneNeu);
    const auto reglerBewegt = [this] {
        m_bildEinstellung.teilung = static_cast<shape::Teilung>(m_teilung->currentIndex());
        m_bildEinstellung.stufen = m_stufen->value();
        m_bildEinstellung.glaetten = m_glaetten->value();
        m_bildEinstellung.mindestProzent = m_mindest->value() / 10.0;
        m_bildEinstellung.detail = m_detail->value();
        zeigeBildRegler();
        m_bildTakt->start();
    };
    connect(m_teilung, QOverload<int>::of(&QComboBox::currentIndexChanged), this, reglerBewegt);
    for (QSlider* s : {m_stufen, m_glaetten, m_mindest, m_detail}) connect(s, &QSlider::valueChanged, this, reglerBewegt);
    connect(m_liveHaken, &QCheckBox::clicked, this, [this](bool an) {
        QSettings().setValue(QLatin1String(kBildLive), an);
        setzeLive(an);
    });
    connect(m_nehmen, &QPushButton::clicked, this, [this] {
        Einfuegen art = Einfuegen::Ersetzen;
        if (!m_zeichnung.konturen.empty())
        {
            QMessageBox frage(QMessageBox::Question, windowTitle(),
                              tr("The shape already holds %n contour(s). Replace them with the contours from the picture, or add those?",
                                 nullptr, static_cast<int>(m_zeichnung.konturen.size())),
                              QMessageBox::NoButton, this);
            QPushButton* ersetzen = frage.addButton(tr("Replace"), QMessageBox::DestructiveRole);
            QPushButton* hinzu = frage.addButton(tr("Add"), QMessageBox::AcceptRole);
            frage.addButton(QMessageBox::Cancel);
            frage.setDefaultButton(hinzu);
            frage.exec();
            if (frage.clickedButton() == hinzu)
                art = Einfuegen::Hinzufuegen;
            else if (frage.clickedButton() != ersetzen)
                return;
        }
        (void)uebernimmVorschau(art);
    });
    m_live = QSettings().value(QLatin1String(kBildLive), true).toBool();
    connect(m_sichern, &QPushButton::clicked, this, &FormFenster::frageUndSpeichere);
    connect(m_sichernAls, &QPushButton::clicked, this, &FormFenster::frageUndSpeichereAls);
    auto* zurueck = new QShortcut(QKeySequence::Undo, this);
    zurueck->setContext(Qt::WidgetWithChildrenShortcut);
    connect(zurueck, &QShortcut::activated, this, [this] { (void)rueckgaengig(); });
    auto* vor = new QShortcut(QKeySequence::Redo, this);
    vor->setContext(Qt::WidgetWithChildrenShortcut);
    connect(vor, &QShortcut::activated, this, [this] { (void)wiederherstellen(); });

    {
        const QSignalBlocker ruhe(m_harmonischeRegler);
        m_harmonischeRegler->setValue(static_cast<int>(std::lround(40.0 * std::log2(m_harmonische))));
    }
    resize(820, 600);
}

void FormFenster::setEngine(IAudioEngine* engine)
{
    if (engine == m_engine) return;
    halte();
    m_engine = engine;
    zeigeKnoepfe();
}

void FormFenster::beginneNeu()
{
    halte();
    m_flaeche->verwirf();
    m_zeichnung = shape::Zeichnung{};
    m_vorlage = m_zeichnung;
    m_nennung.clear();
    m_zurueck.clear();
    m_vor.clear();
    m_weggefallen = 0;
    m_bildDaten = QImage();
    m_bildKennungen.clear();
    m_randKennungen.clear();
    m_vorschauKonturen = {};
    m_gesichert = true;
    passeAnsichtAn();
    setzeWerkzeug(FormFlaeche::Werkzeug::Stift);
    zeigeAlles();
}

bool FormFenster::bearbeite(const QString& nennung)
{
    shape::Zeichnung neu;
    if (nennung.isEmpty() || !shape::holeZeichnung(nennung, neu)) return false;
    halte();
    m_flaeche->verwirf();
    m_zeichnung = std::move(neu);
    m_vorlage = m_zeichnung;
    m_nennung = nennung;
    m_zurueck.clear();
    m_vor.clear();
    m_weggefallen = 0;
    m_bildDaten = QImage();
    m_bildKennungen.clear();
    m_randKennungen.clear();
    m_vorschauKonturen = {};
    m_gesichert = true;
    passeAnsichtAn();
    setzeWerkzeug(FormFlaeche::Werkzeug::Stift);
    zeigeAlles();
    if (!m_zeichnung.konturen.empty()) waehle(m_zeichnung.konturen.front().kennung);
    return true;
}

FormFenster::Herkunft FormFenster::herkunft() const
{
    if (m_nennung.isEmpty()) return Herkunft::Neu;
    if (shape::istEigeneZeichnung(m_nennung)) return Herkunft::Eigene;
    if (shape::istKonturDatei(m_nennung)) return Herkunft::Datei;
    return Herkunft::Mitgeliefert;
}

void FormFenster::passeAnsichtAn()
{
    QRectF alles;
    for (const shape::Kontur& k : m_zeichnung.konturen) alles = alles.isNull() ? k.pfad.boundingRect() : alles.united(k.pfad.boundingRect());
    if (hatBild())
    {
        const QRectF bild = m_bildLage.mapRect(QRectF(QPointF(0.0, 0.0), QSizeF(m_bildDaten.size())));
        alles = alles.isNull() ? bild : alles.united(bild);
    }
    if (alles.width() <= 0.0 && alles.height() <= 0.0)
    {
        m_flaeche->setAnsicht(kLeereAnsicht);
        return;
    }
    // mit Rand, und nie schmaler als ein Viertel der laengeren Seite (eine Linie bleibt zeichenbar)
    const double seite = std::max(alles.width(), alles.height());
    const double rand = 0.25 * seite;
    const double breite = std::max(alles.width(), 0.25 * seite) + 2.0 * rand;
    const double hoehe = std::max(alles.height(), 0.25 * seite) + 2.0 * rand;
    m_flaeche->setAnsicht(QRectF(alles.center().x() - breite / 2.0, alles.center().y() - hoehe / 2.0, breite, hoehe));
}

void FormFenster::setzeWerkzeug(FormFlaeche::Werkzeug werkzeug)
{
    m_flaeche->setWerkzeug(werkzeug);
    (werkzeug == FormFlaeche::Werkzeug::Stift ? m_stift : m_schieben)->setChecked(true);
    zeigeKnoepfe();
}

void FormFenster::merke()
{
    m_zurueck.push_back({m_zeichnung, m_bildKennungen, m_randKennungen});
    if (m_zurueck.size() > kSchritteMax) m_zurueck.erase(m_zurueck.begin());
    m_vor.clear();
}

void FormFenster::nimmStand(Stand stand)
{
    m_zeichnung = std::move(stand.zeichnung);
    m_bildKennungen = std::move(stand.bildKennungen);
    m_randKennungen = std::move(stand.randKennungen);
}

bool FormFenster::rueckgaengig()
{
    if (m_zurueck.empty()) return false;
    m_vor.push_back({m_zeichnung, m_bildKennungen, m_randKennungen});
    nimmStand(std::move(m_zurueck.back()));
    m_zurueck.pop_back();
    geaendertJetzt();
    return true;
}

bool FormFenster::wiederherstellen()
{
    if (m_vor.empty()) return false;
    m_zurueck.push_back({m_zeichnung, m_bildKennungen, m_randKennungen});
    nimmStand(std::move(m_vor.back()));
    m_vor.pop_back();
    geaendertJetzt();
    return true;
}

int FormFenster::fuegeKonturHinzu(const QPainterPath& pfad)
{
    if (pfad.isEmpty() || m_zeichnung.konturen.size() >= static_cast<std::size_t>(shape::kKonturenMax)) return 0;
    merke();
    const int kennung = m_zeichnung.neueKontur(pfad);
    if (kennung == 0)
    {
        m_zurueck.pop_back();
        return 0;
    }
    geaendertJetzt();
    waehle(kennung);
    return kennung;
}

void FormFenster::setzeGewaehlt(int kennung, bool an)
{
    for (shape::Kontur& k : m_zeichnung.konturen)
        if (k.kennung == kennung && k.gewaehlt != an)
        {
            merke();
            k.gewaehlt = an;
            geaendertJetzt();
            return;
        }
}

void FormFenster::benenne(int kennung, const QString& name)
{
    for (shape::Kontur& k : m_zeichnung.konturen)
        if (k.kennung == kennung && k.name != name.trimmed())
        {
            merke();
            k.name = name.trimmed();
            geaendertJetzt();
            return;
        }
}

bool FormFenster::entferne(int kennung)
{
    if (m_zeichnung.finde(kennung) == nullptr) return false;
    const auto& alle = m_zeichnung.konturen;
    const auto platz = std::find_if(alle.begin(), alle.end(), [kennung](const shape::Kontur& k) { return k.kennung == kennung; }) - alle.begin();
    merke();
    (void)m_zeichnung.entferne(kennung);
    geaendertJetzt();
    // die naechste in der Liste ist danach gewaehlt
    if (!m_zeichnung.konturen.empty())
        waehle(m_zeichnung.konturen[std::min(static_cast<std::size_t>(platz), m_zeichnung.konturen.size() - 1)].kennung);
    return true;
}

bool FormFenster::schiebe(int kennung, int richtung)
{
    auto& alle = m_zeichnung.konturen;
    const auto it = std::find_if(alle.begin(), alle.end(), [kennung](const shape::Kontur& k) { return k.kennung == kennung; });
    if (it == alle.end() || richtung == 0) return false;
    const auto platz = it - alle.begin();
    const auto ziel = platz + (richtung < 0 ? -1 : 1);
    if (ziel < 0 || ziel >= static_cast<std::ptrdiff_t>(alle.size())) return false;
    merke();
    std::swap(alle[static_cast<std::size_t>(platz)], alle[static_cast<std::size_t>(ziel)]);
    geaendertJetzt();
    waehle(kennung);
    return true;
}

bool FormFenster::schiebeAnker(int kennung, int anker, QPointF ziel)
{
    for (shape::Kontur& k : m_zeichnung.konturen)
    {
        if (k.kennung != kennung) continue;
        QPainterPath neu = k.pfad;
        if (!shape::schiebeAnker(neu, anker, ziel)) return false;
        merke();
        k.pfad = neu;
        geaendertJetzt();
        return true;
    }
    return false;
}

int FormFenster::aktuelle() const
{
    const QTreeWidgetItem* zeile = m_liste->currentItem();
    return zeile == nullptr ? 0 : zeile->data(0, Qt::UserRole).toInt();
}

QTreeWidgetItem* FormFenster::zeileVon(int kennung) const
{
    for (int i = 0; i < m_liste->topLevelItemCount(); ++i)
        if (m_liste->topLevelItem(i)->data(0, Qt::UserRole).toInt() == kennung) return m_liste->topLevelItem(i);
    return nullptr;
}

void FormFenster::waehle(int kennung)
{
    if (QTreeWidgetItem* zeile = zeileVon(kennung)) m_liste->setCurrentItem(zeile);
}

void FormFenster::setzeHarmonische(double harmonische)
{
    m_harmonische = std::clamp(harmonische, 1.0, static_cast<double>(shape::kHarmonischeMax));
    zeigeHarmonische();
    m_verlauf->setForm(m_form, m_harmonische);
    if (spielt())
    {
        halte();
        (void)spiele();
    }
}

void FormFenster::zeigeHarmonische()
{
    {
        const QSignalBlocker ruhe(m_harmonischeRegler);
        m_harmonischeRegler->setValue(static_cast<int>(std::lround(40.0 * std::log2(m_harmonische))));
    }
    m_harmonischeWert->setText(m_harmonische < 10.0 ? QString::number(m_harmonische, 'f', 1)
                                                    : QString::number(std::lround(m_harmonische)));
}

void FormFenster::rechneForm()
{
    shape::Tonform form = shape::tonform(m_zeichnung, 0);
    m_form = form.leer() ? nullptr : std::make_shared<const shape::Tonform>(std::move(form));
    m_verlauf->setForm(m_form, m_harmonische);
}

double FormFenster::anteil(int kennung) const
{
    if (m_form == nullptr) return -1.0;
    // die Form enthaelt die Konturen mit Haken, in der Reihenfolge der Liste
    std::size_t platz = 0;
    for (const shape::Kontur& k : m_zeichnung.konturen)
    {
        if (!k.gewaehlt) continue;
        if (k.kennung == kennung) return platz < m_form->konturen.size() ? m_form->konturen[platz].anteil : -1.0;
        ++platz;
    }
    return -1.0;
}

std::vector<float> FormFenster::klang() const
{
    std::vector<float> stereo;
    if (m_form == nullptr) return stereo;
    stereo.resize(static_cast<std::size_t>(2 * kPlayRate));
    for (int n = 0; n < kPlayRate; ++n)
    {
        const double umlauf = static_cast<double>(static_cast<long long>(n) * kPlayHz % kPlayRate) / kPlayRate;
        const shape::Punkt punkt = shape::tonwert(*m_form, umlauf, m_harmonische);
        stereo[static_cast<std::size_t>(2 * n)] = kPlayPegel * static_cast<float>(punkt.real());
        stereo[static_cast<std::size_t>(2 * n + 1)] = kPlayPegel * static_cast<float>(punkt.imag());
    }
    return stereo;
}

bool FormFenster::spiele()
{
    if (m_engine == nullptr || m_form == nullptr) return false;
    halte();
    m_quelle.setzeDaten(std::make_shared<const std::vector<float>>(klang()), kPlayRate, m_engine->getSampleRate());
    m_quelle.setzeAusschnitt(Ausschnitt{0, kPlayRate});
    m_quelle.setzeBlenden(Blenden{false, false});  // eine Sekunde aus ganzen Umlaeufen: sie wiederholt ohne Naht
    m_quelle.setzeWiederholen(true);
    const AudioStreamHandle mixer = m_engine->createMixer();
    if (mixer == INVALID_STREAM) return false;
    const AudioStreamHandle quelle = m_engine->mixerAddGenerator(mixer, &m_quelle);
    if (quelle == INVALID_STREAM)
    {
        m_engine->freeStream(mixer);
        return false;
    }
    m_mixer = mixer;
    m_quellHandle = quelle;
    m_quelle.starte();
    m_engine->setSourcePaused(m_quellHandle, false);
    m_engine->play(m_mixer);
    zeigeKnoepfe();
    return true;
}

void FormFenster::halte()
{
    if (m_mixer == INVALID_STREAM) return;
    if (m_engine != nullptr)
    {
        m_engine->mixerRemoveSource(m_quellHandle);
        m_engine->freeStream(m_mixer);
    }
    m_mixer = INVALID_STREAM;
    m_quellHandle = INVALID_STREAM;
    zeigeKnoepfe();
}

void FormFenster::geaendertJetzt()
{
    m_gesichert = false;
    zeigeAlles();
    if (spielt())
    {
        halte();
        (void)spiele();
    }
}

void FormFenster::zeigeAlles()
{
    rechneForm();
    zeigeHarmonische();
    zeigeListe();
    m_flaeche->setHervorgehoben(aktuelle());
    m_flaeche->update();
    const QString titel = herkunft() == Herkunft::Neu && m_zeichnung.titel.isEmpty() ? tr("New shape") : m_zeichnung.titel;
    setWindowTitle(tr("Edit shape – %1").arg(titel));
    QString ort = tr("My shapes");
    if (herkunft() == Herkunft::Neu) ort = tr("Not saved yet");
    if (herkunft() == Herkunft::Mitgeliefert) ort = tr("Built-in, read-only");
    if (herkunft() == Herkunft::Datei) ort = tr("File, read-only: %1").arg(QDir::toNativeSeparators(m_nennung));
    m_ort->setText(ort);
    m_nutzer->setText(nutzerZeile());
    m_flaeche->setBild(&m_bildDaten, m_bildLage, m_zeigeBild->isChecked());
    if (!hatBild()) m_flaeche->setVorschau({});
    zeigeBildRegler();
    (m_zeichnung.nutzung == shape::Nutzung::EinPfad ? m_nutzungPfad : m_nutzungSchichten)->setChecked(true);
    m_spruenge->setCurrentIndex(static_cast<int>(m_zeichnung.spruenge));
    zeigeKnoepfe();
}

void FormFenster::zeigeListe()
{
    const int vorher = aktuelle();
    const QSignalBlocker ruhe(m_liste);
    // Dieselben Konturen in derselben Reihenfolge: die Zeilen an Ort und Stelle
    // nachstellen. Neu gebaut wird nur, wenn sich Zahl oder Reihenfolge aendert —
    // ein Haken meldet sich aus `setData` der Zeile; wer sie dort loescht, loescht
    // sie unter dem laufenden Aufruf weg.
    bool gleich = m_liste->topLevelItemCount() == static_cast<int>(m_zeichnung.konturen.size());
    for (int n = 0; gleich && n < m_liste->topLevelItemCount(); ++n)
        gleich = m_liste->topLevelItem(n)->data(0, Qt::UserRole).toInt() == m_zeichnung.konturen[static_cast<std::size_t>(n)].kennung;
    if (!gleich) m_liste->clear();
    int gewaehlt = 0;
    for (std::size_t n = 0; n < m_zeichnung.konturen.size(); ++n)
    {
        const shape::Kontur& k = m_zeichnung.konturen[n];
        QTreeWidgetItem* zeile = gleich ? m_liste->topLevelItem(static_cast<int>(n)) : new QTreeWidgetItem(m_liste);
        zeile->setText(0, konturTitel(k, static_cast<int>(n)));
        zeile->setIcon(0, farbPunkt(k.farbe));
        zeile->setData(0, Qt::UserRole, k.kennung);
        zeile->setFlags(zeile->flags() | Qt::ItemIsUserCheckable);
        zeile->setCheckState(0, k.gewaehlt ? Qt::Checked : Qt::Unchecked);
        const double a = anteil(k.kennung);
        zeile->setText(1, amRand(k.kennung) ? tr("edge") : a < 0.0 ? QString::fromUtf8("–") : QStringLiteral("%1 %").arg(std::lround(100.0 * a)));
        zeile->setTextAlignment(1, Qt::AlignRight | Qt::AlignVCenter);
        if (k.gewaehlt) ++gewaehlt;
    }
    m_listenKopf->setText(tr("Contours — %1 of %2 ticked").arg(gewaehlt).arg(m_zeichnung.konturen.size()));
    m_weggefallenZeile->setText(m_weggefallen > 0 ? tr("%1 smaller ones left out").arg(m_weggefallen) : QString());
    m_weggefallenZeile->setVisible(m_weggefallen > 0);
    if (QTreeWidgetItem* zeile = zeileVon(vorher)) m_liste->setCurrentItem(zeile);
}

void FormFenster::zeigeKnoepfe()
{
    const bool stift = m_flaeche->werkzeug() == FormFlaeche::Werkzeug::Stift;
    if (!stift)
        m_hinweis->setText(tr("Drag a point to move it · Ctrl+Z undoes"));
    else if (!m_flaeche->angefangen().isEmpty())
        m_hinweis->setText(tr("Go on drawing · click the first point, double-click or press Enter to close · Esc discards"));
    else
        m_hinweis->setText(tr("Drag to draw freehand · click to set corners · both mix in one contour"));

    const int kennung = aktuelle();
    const auto& alle = m_zeichnung.konturen;
    const auto it = std::find_if(alle.begin(), alle.end(), [kennung](const shape::Kontur& k) { return k.kennung == kennung; });
    const bool eine = it != alle.end();
    m_umbenennen->setEnabled(eine);
    m_weg->setEnabled(eine);
    m_hinauf->setEnabled(eine && it != alle.begin());
    m_hinunter->setEnabled(eine && it + 1 != alle.end());

    m_play->setEnabled(m_engine != nullptr && m_form != nullptr);
    {
        const QSignalBlocker ruhe(m_play);
        m_play->setChecked(spielt());
        m_play->setText(spielt() ? tr("■ Stop playing") : tr("▶ Play"));
    }
    if (m_engine == nullptr) m_play->setToolTip(tr("No audio engine — Play is not available"));

    // From image (R1 a, R2 a): nur mit einem Bild; Take contours nur ohne Apply live
    m_ausBild->setEnabled(hatBild());
    m_ausBild->setToolTip(hatBild() ? QString() : tr("Open a picture with Open image... — the picture is not kept in the shape file"));
    m_zeigeBild->setEnabled(hatBild());
    {
        const QSignalBlocker ruhe(m_liveHaken);
        m_liveHaken->setChecked(m_live);
    }
    m_nehmen->setVisible(!m_live);
    m_nehmen->setEnabled(hatBild() && !m_vorschauKonturen.konturen.empty());

    const bool etwas = !alle.empty();
    const bool eigene = herkunft() == Herkunft::Eigene;
    m_sichernAls->setEnabled(etwas);
    m_sichern->setEnabled(etwas && eigene);
    if (eigene)
        m_sichern->setToolTip(tr("Replace %1 in My shapes").arg(m_vorlage.titel));
    else if (herkunft() == Herkunft::Neu)
        m_sichern->setToolTip(tr("A new shape has no name yet — use Save as..."));
    else
        m_sichern->setToolTip(tr("Built-in shapes and files stay as they are — use Save as..."));
}

int FormFenster::nimmSvg(const QString& datei, Einfuegen art)
{
    const shape::SvgKonturen svg = shape::liesSvgKonturen(datei);
    if (!svg.gelesen) return -1;
    if (svg.teile.empty()) return 0;
    merke();
    const bool ersetzen = art == Einfuegen::Ersetzen || m_zeichnung.konturen.empty();
    QTransform lage;
    int weg = svg.weggefallen;
    if (ersetzen)
    {
        // wie eine leere Zeichnung: die Kennungen laufen weiter, damit keine Schicht eine neue Kontur fuer eine alte haelt
        m_zeichnung.konturen.clear();
        m_zeichnung.bruecken.clear();
        m_zeichnung.herkunft = shape::Herkunft{shape::HerkunftArt::Svg, QFileInfo(datei).fileName(), {}};
    }
    else
    {
        // Add: das SVG fuellt den Ausschnitt ohne seinen Rand (O1 a), mit gleichem Mass in x und y
        QRectF quelle;
        for (const shape::SvgTeil& t : svg.teile) quelle = quelle.isNull() ? t.pfad.boundingRect() : quelle.united(t.pfad.boundingRect());
        const QRectF ansicht = m_flaeche->ansicht();
        const QSizeF innen = ansicht.size() / 1.5;  // passeAnsichtAn legt ein Viertel der laengeren Seite je Seite als Rand dazu
        const double mass = std::min(innen.width() / std::max(quelle.width(), 1e-9), innen.height() / std::max(quelle.height(), 1e-9));
        lage.translate(ansicht.center().x(), ansicht.center().y());
        lage.scale(mass, mass);
        lage.translate(-quelle.center().x(), -quelle.center().y());
    }
    int dazu = 0;
    int erste = 0;
    for (const shape::SvgTeil& t : svg.teile)
    {
        const int kennung = m_zeichnung.neueKontur(lage.map(t.pfad), QString(), t.farbe);
        if (kennung == 0)
        {
            ++weg;  // die Zeichnung ist voll
            continue;
        }
        m_zeichnung.konturen.back().gewaehlt = t.gewaehlt;
        if (erste == 0) erste = kennung;
        ++dazu;
    }
    m_weggefallen = ersetzen ? weg : m_weggefallen + weg;
    passeAnsichtAn();
    geaendertJetzt();
    if (erste != 0) waehle(erste);
    return dazu;
}

void FormFenster::frageNachSvg()
{
    QSettings einstellungen;
    const QString datei = QFileDialog::getOpenFileName(this, tr("Open SVG"), einstellungen.value(QLatin1String(kLetzterSvgOrdner)).toString(),
                                                       tr("SVG files (*.svg)"));
    if (datei.isEmpty()) return;
    einstellungen.setValue(QLatin1String(kLetzterSvgOrdner), QFileInfo(datei).absolutePath());
    Einfuegen art = Einfuegen::Ersetzen;
    if (!m_zeichnung.konturen.empty())
    {
        // O1 a: stehen schon Konturen da, entscheidet der Nutzer
        QMessageBox frage(QMessageBox::Question, windowTitle(),
                          tr("The shape already holds %n contour(s). Replace them with the contours of %1, or add those?", nullptr,
                             static_cast<int>(m_zeichnung.konturen.size()))
                              .arg(QFileInfo(datei).fileName()),
                          QMessageBox::NoButton, this);
        QPushButton* ersetzen = frage.addButton(tr("Replace"), QMessageBox::DestructiveRole);
        QPushButton* hinzu = frage.addButton(tr("Add"), QMessageBox::AcceptRole);
        frage.addButton(QMessageBox::Cancel);
        frage.setDefaultButton(hinzu);
        frage.exec();
        if (frage.clickedButton() == ersetzen)
            art = Einfuegen::Ersetzen;
        else if (frage.clickedButton() == hinzu)
            art = Einfuegen::Hinzufuegen;
        else
            return;
    }
    const int dazu = nimmSvg(datei, art);
    if (dazu < 0)
        QMessageBox::warning(this, windowTitle(), tr("Could not read %1 as an SVG file").arg(QDir::toNativeSeparators(datei)));
    else if (dazu == 0)
        QMessageBox::information(this, windowTitle(), tr("%1 holds no visible path").arg(QFileInfo(datei).fileName()));
}

// =============================================================================
// Open image... und From image (Stufe 20c)
// =============================================================================

bool FormFenster::ausBild(int kennung) const
{
    return std::find(m_bildKennungen.begin(), m_bildKennungen.end(), kennung) != m_bildKennungen.end();
}

bool FormFenster::amRand(int kennung) const
{
    return std::find(m_randKennungen.begin(), m_randKennungen.end(), kennung) != m_randKennungen.end();
}

int FormFenster::nimmBild(const QString& datei, Einfuegen art)
{
    QImageReader leser(datei);
    leser.setAutoTransform(true);  // ein Foto steht, wie es aufgenommen ist
    const QImage bild = leser.read();
    if (bild.isNull()) return -1;
    return nimmBild(bild, QFileInfo(datei).fileName(), art);
}

int FormFenster::nimmBild(const QImage& bild, const QString& name, Einfuegen art)
{
    const QImage vorbereitet = shape::bildZumErkennen(bild);
    if (vorbereitet.isNull()) return -1;
    const bool ersetzen = art == Einfuegen::Ersetzen || m_zeichnung.konturen.empty();
    m_bildDaten = vorbereitet;
    m_bildName = name;
    m_bildLage = QTransform();
    if (!ersetzen)
    {
        // Add: das Bild fuellt den Ausschnitt ohne seinen Rand, wie ein SVG (O1 a)
        const QRectF ansicht = m_flaeche->ansicht();
        const QSizeF innen = ansicht.size() / 1.5;
        const double mass = std::min(innen.width() / m_bildDaten.width(), innen.height() / m_bildDaten.height());
        m_bildLage.translate(ansicht.center().x(), ansicht.center().y());
        m_bildLage.scale(mass, mass);
        m_bildLage.translate(-m_bildDaten.width() / 2.0, -m_bildDaten.height() / 2.0);
    }
    m_flaeche->setBild(&m_bildDaten, m_bildLage, m_zeigeBild->isChecked());
    if (!m_live)
    {
        // R2 a: nur vorschlagen — uebernommen wird mit Take contours
        m_bildKennungen.clear();
        rechneVorschau();
        if (ersetzen) passeAnsichtAn();
        zeigeAlles();
        return static_cast<int>(m_vorschauKonturen.konturen.size());
    }
    merke();
    if (ersetzen)
    {
        m_zeichnung.konturen.clear();
        m_zeichnung.bruecken.clear();
        m_randKennungen.clear();
        m_zeichnung.herkunft = shape::Herkunft{shape::HerkunftArt::Bild, name, m_bildEinstellung.alsJson()};
    }
    m_bildKennungen.clear();  // was aus einem frueheren Bild kam, bleibt, wie es ist
    const int dazu = setzeBildKonturen();
    passeAnsichtAn();
    geaendertJetzt();
    if (!m_bildKennungen.empty()) waehle(m_bildKennungen.front());
    return dazu;
}

int FormFenster::setzeBildKonturen()
{
    const shape::BildKonturen gefunden = shape::bildKonturen(m_bildDaten, m_bildEinstellung);
    int weg = gefunden.weggefallen;
    int dazu = 0;
    for (const shape::BildKontur& k : gefunden.konturen)
    {
        const int kennung = m_zeichnung.neueKontur(m_bildLage.map(k.pfad));
        if (kennung == 0)
        {
            ++weg;  // die Zeichnung ist voll
            continue;
        }
        m_zeichnung.konturen.back().gewaehlt = k.gewaehlt;
        m_bildKennungen.push_back(kennung);
        if (k.rand) m_randKennungen.push_back(kennung);
        ++dazu;
    }
    m_weggefallen = weg;
    // die Herkunft traegt die Regler, solange sie dieses Bild nennt
    if (m_zeichnung.herkunft.art == shape::HerkunftArt::Bild && m_zeichnung.herkunft.quelle == m_bildName)
        m_zeichnung.herkunft.einstellungen = m_bildEinstellung.alsJson();
    return dazu;
}

void FormFenster::erkenneNeu()
{
    m_bildTakt->stop();
    if (!hatBild()) return;
    if (!m_live)
    {
        rechneVorschau();
        return;
    }
    // R1 a: genau die Konturen aus dem Bild ersetzen; gezeichnete und SVG-Konturen bleiben
    merke();
    for (const int kennung : m_bildKennungen)
    {
        (void)m_zeichnung.entferne(kennung);
        std::erase(m_randKennungen, kennung);
    }
    m_bildKennungen.clear();
    (void)setzeBildKonturen();
    geaendertJetzt();
}

void FormFenster::rechneVorschau()
{
    m_vorschauKonturen = m_live || !hatBild() ? shape::BildKonturen{} : shape::bildKonturen(m_bildDaten, m_bildEinstellung);
    std::vector<QPainterPath> zuege;
    for (const shape::BildKontur& k : m_vorschauKonturen.konturen) zuege.push_back(m_bildLage.map(k.pfad));
    m_flaeche->setVorschau(std::move(zuege));
    zeigeKnoepfe();
}

int FormFenster::uebernimmVorschau(Einfuegen art)
{
    if (!hatBild() || m_live) return 0;
    merke();
    if (art == Einfuegen::Ersetzen || m_zeichnung.konturen.empty())
    {
        m_zeichnung.konturen.clear();
        m_zeichnung.bruecken.clear();
        m_randKennungen.clear();
        m_zeichnung.herkunft = shape::Herkunft{shape::HerkunftArt::Bild, m_bildName, m_bildEinstellung.alsJson()};
    }
    m_bildKennungen.clear();
    const int dazu = setzeBildKonturen();
    passeAnsichtAn();
    geaendertJetzt();
    if (!m_bildKennungen.empty()) waehle(m_bildKennungen.front());
    return dazu;
}

void FormFenster::setzeBildEinstellung(const shape::BildEinstellung& einstellung)
{
    m_bildEinstellung = einstellung;
    m_bildEinstellung.ausJson(einstellung.alsJson());  // in die Grenzen
    zeigeBildRegler();
    erkenneNeu();
}

void FormFenster::setzeLive(bool an)
{
    m_live = an;
    {
        const QSignalBlocker ruhe(m_liveHaken);
        m_liveHaken->setChecked(an);
    }
    rechneVorschau();  // aus: vorschlagen; an: die Vorschau verschwindet
}

void FormFenster::zeigeBildRegler()
{
    const shape::BildEinstellung& e = m_bildEinstellung;
    for (QWidget* w : std::initializer_list<QWidget*>{m_teilung, m_stufen, m_glaetten, m_mindest, m_detail}) w->blockSignals(true);
    m_teilung->setCurrentIndex(static_cast<int>(e.teilung));
    m_stufen->setValue(e.stufen);
    m_glaetten->setValue(e.glaetten);
    m_mindest->setValue(static_cast<int>(std::lround(e.mindestProzent * 10.0)));
    m_detail->setValue(e.detail);
    for (QWidget* w : std::initializer_list<QWidget*>{m_teilung, m_stufen, m_glaetten, m_mindest, m_detail}) w->blockSignals(false);
    m_stufenWert->setText(QString::number(e.stufen));
    m_glaettenWert->setText(e.glaetten == 0 ? tr("off") : QString::number(e.glaetten));
    m_mindestWert->setText(QStringLiteral("%1 %").arg(e.mindestProzent, 0, 'f', 1));
    m_detailWert->setText(QStringLiteral("%1 %").arg(e.detail));
}

void FormFenster::frageNachBild()
{
    QSettings einstellungen;
    const QString datei = QFileDialog::getOpenFileName(this, tr("Open image"), einstellungen.value(QLatin1String(kLetzterBildOrdner)).toString(),
                                                       tr("Pictures (*.png *.jpg *.jpeg *.bmp *.gif *.webp)"));
    if (datei.isEmpty()) return;
    einstellungen.setValue(QLatin1String(kLetzterBildOrdner), QFileInfo(datei).absolutePath());
    Einfuegen art = Einfuegen::Ersetzen;
    if (m_live && !m_zeichnung.konturen.empty())
    {
        // O1 a: stehen schon Konturen da, entscheidet der Nutzer (ohne Apply live erst bei Take contours)
        QMessageBox frage(QMessageBox::Question, windowTitle(),
                          tr("The shape already holds %n contour(s). Replace them with the contours of %1, or add those?", nullptr,
                             static_cast<int>(m_zeichnung.konturen.size()))
                              .arg(QFileInfo(datei).fileName()),
                          QMessageBox::NoButton, this);
        QPushButton* ersetzen = frage.addButton(tr("Replace"), QMessageBox::DestructiveRole);
        QPushButton* hinzu = frage.addButton(tr("Add"), QMessageBox::AcceptRole);
        frage.addButton(QMessageBox::Cancel);
        frage.setDefaultButton(hinzu);
        frage.exec();
        if (frage.clickedButton() == hinzu)
            art = Einfuegen::Hinzufuegen;
        else if (frage.clickedButton() != ersetzen)
            return;
    }
    else if (!m_zeichnung.konturen.empty())
    {
        art = Einfuegen::Hinzufuegen;  // ohne Apply live: das Bild liegt eingepasst neben den Konturen, Take contours fragt
    }
    if (nimmBild(datei, art) < 0)
        QMessageBox::warning(this, windowTitle(), tr("Could not read %1 as a picture").arg(QDir::toNativeSeparators(datei)));
}

QString FormFenster::nutzerZeile() const
{
    if (m_nennung.isEmpty()) return {};
    const QStringList nutzer = eigeneInstrumenteMitForm(m_nennung);
    if (nutzer.isEmpty()) return tr("Not used by any of your instruments");
    constexpr int kBeimNamen = 3;
    QString liste = nutzer.mid(0, kBeimNamen).join(QStringLiteral(", "));
    if (nutzer.size() > kBeimNamen) liste = tr("%1 and %2 more").arg(liste).arg(nutzer.size() - kBeimNamen);
    return tr("Used by: %1").arg(liste);
}

QString FormFenster::ersetzenFrage() const
{
    QString frage = tr("Replace %1 in My shapes?").arg(m_vorlage.titel);
    const QStringList nutzer = eigeneInstrumenteMitForm(m_nennung);
    if (!nutzer.isEmpty())
    {
        constexpr int kBeimNamen = 3;
        QString liste = nutzer.mid(0, kBeimNamen).join(QStringLiteral(", "));
        if (nutzer.size() > kBeimNamen) liste = tr("%1 and %2 more").arg(liste).arg(nutzer.size() - kBeimNamen);
        frage += QLatin1Char('\n') + tr("Used by: %1 — they will play the new version.").arg(liste);
    }
    return frage;
}

QString FormFenster::dateiZeile(const QString& titel, bool* geht)
{
    const QString kurz = shape::eigenerKonturName(titel);
    const QString datei = kurz.isEmpty() ? QString() : shape::eigeneKonturDatei(kurz);
    const bool vergeben = !datei.isEmpty() && QFileInfo::exists(datei);
    if (geht != nullptr) *geht = !kurz.isEmpty() && !datei.isEmpty() && !vergeben;
    if (kurz.isEmpty()) return tr("Type a name with letters or digits");
    if (vergeben) return tr("My shapes already holds %1 — choose another name").arg(QFileInfo(datei).fileName());
    return tr("File: My files · contours · %1%2").arg(kurz, QLatin1String(shape::kKonturEndung));
}

QString FormFenster::vorschlagTitel() const
{
    const QString basis = m_nennung.isEmpty() ? tr("My shape") : m_vorlage.titel;
    const auto frei = [](const QString& titel) {
        const QString datei = shape::eigeneKonturDatei(shape::eigenerKonturName(titel));
        return !datei.isEmpty() && !QFileInfo::exists(datei);
    };
    if (m_nennung.isEmpty() && frei(basis)) return basis;
    for (int n = 2; n < 1000; ++n)
    {
        const QString titel = QStringLiteral("%1 %2").arg(basis).arg(n);
        if (frei(titel)) return titel;
    }
    return basis;
}

QString FormFenster::schreibe(const QString& name)
{
    if (m_zeichnung.konturen.empty() || name.isEmpty()) return {};
    halte();
    shape::Zeichnung neu = m_zeichnung;
    if (!shape::speichereEigeneZeichnung(neu, name)) return {};
    zaehleInstrumentFassung();  // wer Toene mit dieser Form gerechnet hat, rechnet neu
    m_zeichnung = neu;
    m_vorlage = neu;
    m_nennung = neu.name;
    m_gesichert = true;
    zeigeAlles();
    Q_EMIT gespeichert(m_nennung);
    close();
    return m_nennung;
}

QString FormFenster::speichere()
{
    if (herkunft() != Herkunft::Eigene) return {};
    return schreibe(m_nennung.mid(static_cast<int>(qstrlen(shape::kEigenVorsatz))));
}

QString FormFenster::speichereAls(const QString& titel)
{
    bool geht = false;
    (void)dateiZeile(titel, &geht);
    if (!geht || m_zeichnung.konturen.empty()) return {};
    const QString vorher = m_zeichnung.titel;
    m_zeichnung.titel = titel.trimmed();
    const QString nennung = schreibe(shape::eigenerKonturName(titel));
    if (nennung.isEmpty()) m_zeichnung.titel = vorher;
    return nennung;
}

void FormFenster::frageUndSpeichere()
{
    if (herkunft() != Herkunft::Eigene) return;
    if (QMessageBox::question(this, windowTitle(), ersetzenFrage()) != QMessageBox::Yes) return;
    if (speichere().isEmpty())
        QMessageBox::warning(this, windowTitle(),
                             tr("Could not write %1").arg(QDir::toNativeSeparators(shape::eigeneKonturDatei(
                                 m_nennung.mid(static_cast<int>(qstrlen(shape::kEigenVorsatz)))))));
}

void FormFenster::frageUndSpeichereAls()
{
    if (m_zeichnung.konturen.empty()) return;
    QDialog dialog(this);
    dialog.setObjectName(QStringLiteral("shapeSaveAsDialog"));
    dialog.setWindowTitle(tr("Save shape as"));
    auto* aussen = new QVBoxLayout(&dialog);
    QString satzText = tr("This saves a new shape to My shapes and uses it.");
    if (herkunft() == Herkunft::Mitgeliefert) satzText = tr("Built-in shapes stay as they are. This saves a copy to My shapes and uses it.");
    if (herkunft() == Herkunft::Datei) satzText = tr("The file stays as it is. This saves a copy to My shapes and uses it.");
    if (herkunft() == Herkunft::Eigene) satzText = tr("%1 stays as it is. This saves a copy to My shapes and uses it.").arg(m_vorlage.titel);
    auto* satz = new QLabel(satzText, &dialog);
    satz->setWordWrap(true);
    satz->setEnabled(false);
    aussen->addWidget(satz);
    auto* name = new QLineEdit(vorschlagTitel(), &dialog);
    name->setObjectName(QStringLiteral("shapeSaveAsName"));
    auto* zeile = new QHBoxLayout;
    zeile->addWidget(new QLabel(tr("Name"), &dialog));
    zeile->addWidget(name, 1);
    aussen->addLayout(zeile);
    auto* datei = new QLabel(&dialog);
    datei->setObjectName(QStringLiteral("shapeSaveAsFile"));
    datei->setEnabled(false);
    aussen->addWidget(datei);
    auto* abbrechen = new QPushButton(tr("Cancel"), &dialog);
    auto* sichern = new QPushButton(tr("Save"), &dialog);
    sichern->setDefault(true);
    auto* fuss = new QHBoxLayout;
    fuss->addStretch(1);
    fuss->addWidget(abbrechen);
    fuss->addWidget(sichern);
    aussen->addLayout(fuss);
    const auto zeige = [name, datei, sichern] {
        bool geht = false;
        datei->setText(dateiZeile(name->text(), &geht));
        sichern->setEnabled(geht);
    };
    connect(name, &QLineEdit::textChanged, &dialog, zeige);
    connect(abbrechen, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(sichern, &QPushButton::clicked, &dialog, &QDialog::accept);
    zeige();
    dialog.resize(420, dialog.sizeHint().height());
    if (dialog.exec() != QDialog::Accepted) return;

    const QString titel = name->text().trimmed();
    if (speichereAls(titel).isEmpty())
        QMessageBox::warning(this, windowTitle(),
                             tr("Could not write %1").arg(QDir::toNativeSeparators(shape::eigeneKonturDatei(shape::eigenerKonturName(titel)))));
}

void FormFenster::closeEvent(QCloseEvent* event)
{
    if (!m_gesichert && geaendert() &&
        QMessageBox::question(this, windowTitle(), tr("Your changes to the shape are not saved.\nDiscard them?")) != QMessageBox::Yes)
    {
        event->ignore();
        return;
    }
    halte();
    QWidget::closeEvent(event);
}

}  // namespace lumi::loop
