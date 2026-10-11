/**
 * @file InstrumentEditor.cpp
 * @brief Loop: der Instrument-Editor — die Noten einer Basis ueber einer
 *        Klaviatur (Stufe 9b)
 */

#include "loop/InstrumentEditor.hpp"

#include "loop/Bearbeiten.hpp"
#include "loop/Instrument.hpp"

#include <QComboBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QPushButton>
#include <QScrollBar>
#include <QShowEvent>
#include <QSignalBlocker>
#include <QSlider>
#include <QSpinBox>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>

namespace lumi::loop
{

namespace
{

constexpr int kHoehen = kHoeheMax - kHoeheMin + 1;
constexpr double kLautMin = 0.05;  ///< leiser wird eine Note nicht — ohne Lautstaerke gaebe es sie nicht

bool schwarzeTaste(int hoehe)
{
    const int stufe = ((hoehe % 12) + 12) % 12;
    return stufe == 1 || stufe == 3 || stufe == 6 || stufe == 8 || stufe == 10;
}

int teileAb(int wert, int durch)
{
    return wert >= 0 ? wert / durch : -((-wert + durch - 1) / durch);
}

}  // namespace

// =============================================================================
// NotenRaster
// =============================================================================

NotenRaster::NotenRaster(QWidget* parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("loopNotes"));
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);  // der Mauszeiger zeigt, was ein Klick taete
}

bool NotenRaster::anKante(const Note& note, int x) const
{
    const int griff = std::clamp(note.laenge * kSchrittBreite / 3, kKante, kKanteMax);
    return x >= xVonSchritt(note.schritt + note.laenge) - griff;
}

void NotenRaster::zeigeZeiger(QPoint ort)
{
    Qt::CursorShape form = Qt::ArrowCursor;
    if (const int platz = noteBei(ort); platz >= 0)
        form = anKante(m_muster.noten[static_cast<std::size_t>(platz)], ort.x()) ? Qt::SizeHorCursor : Qt::SizeAllCursor;
    else if (ort.x() < kTastenBreite && hoeheBei(ort.y()) >= 0)
        form = Qt::PointingHandCursor;
    if (cursor().shape() != form) setCursor(form);
}

void NotenRaster::setMuster(const Muster& muster, int schlaegeJeTakt, int schritteJeSchlag)
{
    m_muster = muster;
    m_schlaege = std::max(1, schlaegeJeTakt);
    m_jeSchlag = std::max(1, schritteJeSchlag);
    m_zug = Zug::Keiner;
    if (auswahl() < 0) m_wahlSchritt = m_wahlHoehe = -1;
    setRollen(m_rollX, m_rollY);  // das Muster kann kuerzer geworden sein
    update();
}

void NotenRaster::setMarke(double schritt)
{
    if (schritt == m_marke) return;
    m_marke = schritt;
    update();
}

int NotenRaster::auswahl() const
{
    if (m_wahlSchritt < 0) return -1;
    for (std::size_t i = 0; i < m_muster.noten.size(); ++i)
        if (m_muster.noten[i].schritt == m_wahlSchritt && m_muster.noten[i].hoehe == m_wahlHoehe)
            return static_cast<int>(i);
    return -1;
}

bool NotenRaster::loescheAuswahl()
{
    const int platz = auswahl();
    if (platz < 0 || m_zug != Zug::Keiner) return false;
    m_muster.noten.erase(m_muster.noten.begin() + platz);
    m_wahlSchritt = m_wahlHoehe = -1;
    update();
    Q_EMIT geaendert(m_muster, tr("Delete note"));
    return true;
}

int NotenRaster::rollXMax() const
{
    return std::max(0, schritte() * kSchrittBreite - sichtBreite());
}

int NotenRaster::rollYMax() const
{
    return std::max(0, kHoehen * kZeilenHoehe - sichtHoehe());
}

void NotenRaster::setRollen(int x, int y)
{
    const int neuX = std::clamp(x, 0, rollXMax());
    const int neuY = std::clamp(y, 0, rollYMax());
    if (neuX == m_rollX && neuY == m_rollY) return;
    m_rollX = neuX;
    m_rollY = neuY;
    update();
}

void NotenRaster::zeigeHoehe(int hoehe)
{
    setRollen(m_rollX, (kHoeheMax - hoehe) * kZeilenHoehe + kZeilenHoehe / 2 - sichtHoehe() / 2);
}

int NotenRaster::schrittBei(int x) const
{
    if (x < kTastenBreite) return -1;
    const int schritt = (x - kTastenBreite + m_rollX) / kSchrittBreite;
    return schritt < schritte() ? schritt : -1;
}

int NotenRaster::hoeheBei(int y) const
{
    if (y < kKopfHoehe || y >= lautOben()) return -1;
    const int hoehe = kHoeheMax - (y - kKopfHoehe + m_rollY) / kZeilenHoehe;
    return hoehe >= kHoeheMin && hoehe <= kHoeheMax ? hoehe : -1;
}

int NotenRaster::noteBei(QPoint ort) const
{
    const int schritt = schrittBei(ort.x());
    const int hoehe = hoeheBei(ort.y());
    if (schritt < 0 || hoehe < 0) return -1;
    for (int i = static_cast<int>(m_muster.noten.size()) - 1; i >= 0; --i)
    {
        const Note& note = m_muster.noten[static_cast<std::size_t>(i)];
        if (note.hoehe == hoehe && schritt >= note.schritt && schritt < note.schritt + note.laenge) return i;
    }
    return -1;
}

bool NotenRaster::belegt(int schritt, int hoehe, int ausser) const
{
    for (int i = 0; i < static_cast<int>(m_muster.noten.size()); ++i)
    {
        const Note& note = m_muster.noten[static_cast<std::size_t>(i)];
        if (i != ausser && note.schritt == schritt && note.hoehe == hoehe) return true;
    }
    return false;
}

void NotenRaster::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    const QPalette pal = palette();
    const QColor grund = pal.color(QPalette::Base);
    const QColor text = pal.color(QPalette::Text);
    const QColor an = pal.color(QPalette::Highlight);
    p.fillRect(rect(), grund);
    QColor leise = text;
    leise.setAlpha(130);
    const int zahl = schritte();
    const int jeTakt = m_schlaege * m_jeSchlag;
    const int oben = kKopfHoehe;
    const int unten = lautOben();
    const QRect feld(kTastenBreite, oben, sichtBreite(), unten - oben);
    const int rechts = std::min(width(), xVonSchritt(zahl));

    QFont klein = font();
    klein.setPointSizeF(std::max(6.5, klein.pointSizeF() - 1.0));

    // --- die Zeilen der Tonhoehen: die der schwarzen Tasten abgesetzt, jedes C mit einer Linie
    p.setClipRect(feld);
    for (int hoehe = kHoeheMin; hoehe <= kHoeheMax; ++hoehe)
    {
        const int y = yVonHoehe(hoehe);
        if (y + kZeilenHoehe < oben || y > unten) continue;
        if (schwarzeTaste(hoehe))
        {
            QColor dunkel = text;
            dunkel.setAlpha(22);
            p.fillRect(QRect(kTastenBreite, y, rechts - kTastenBreite, kZeilenHoehe), dunkel);
        }
        if (hoehe % 12 == 0)
        {
            QColor linie = text;
            linie.setAlpha(70);
            p.setPen(linie);
            p.drawLine(kTastenBreite, y + kZeilenHoehe - 1, rechts, y + kZeilenHoehe - 1);
        }
    }
    // --- die Schritte: jeder Schlag kraeftiger, jeder Takt am kraeftigsten
    for (int i = 0; i <= zahl; ++i)
    {
        const int x = xVonSchritt(i);
        if (x < kTastenBreite || x > width()) continue;
        QColor linie = text;
        linie.setAlpha(i % jeTakt == 0 ? 110 : i % m_jeSchlag == 0 ? 55 : 22);
        p.setPen(linie);
        p.drawLine(x, oben, x, unten);
    }
    // --- die Noten: so kraeftig, wie sie laut sind; die gewaehlte mit Rand
    const int gewaehlt = auswahl();
    for (int i = 0; i < static_cast<int>(m_muster.noten.size()); ++i)
    {
        const Note& note = m_muster.noten[static_cast<std::size_t>(i)];
        const QRect r(xVonSchritt(note.schritt) + 1, yVonHoehe(note.hoehe) + 1, note.laenge * kSchrittBreite - 1,
                      kZeilenHoehe - 2);
        QColor farbe = m_muster.stumm ? text : an;
        farbe.setAlphaF((m_muster.stumm ? 0.2 : 0.4) + (m_muster.stumm ? 0.25 : 0.6) * note.laut);
        p.setPen(i == gewaehlt ? QPen(text, 1.0) : Qt::NoPen);
        p.setBrush(farbe);
        p.drawRoundedRect(r, 2, 2);
    }
    p.setBrush(Qt::NoBrush);

    // --- die Klaviatur
    p.setClipRect(QRect(0, oben, kTastenBreite, unten - oben));
    p.setFont(klein);
    for (int hoehe = kHoeheMin; hoehe <= kHoeheMax; ++hoehe)
    {
        const int y = yVonHoehe(hoehe);
        if (y + kZeilenHoehe < oben || y > unten) continue;
        const QRect taste(0, y, kTastenBreite - 1, kZeilenHoehe);
        const bool schwarz = schwarzeTaste(hoehe);
        p.fillRect(taste, hoehe == m_taste ? an : schwarz ? QColor(40, 40, 44) : QColor(232, 232, 228));
        p.setPen(QColor(120, 120, 120));
        p.drawLine(0, y + kZeilenHoehe - 1, kTastenBreite - 1, y + kZeilenHoehe - 1);
        if (hoehe % 12 == 0)
        {
            p.setPen(QColor(40, 40, 44));
            p.drawText(taste.adjusted(4, 0, -2, 0), Qt::AlignLeft | Qt::AlignVCenter, tonName(hoehe));
        }
    }

    // --- der Kopf: die Takte
    p.setClipRect(QRect(kTastenBreite, 0, sichtBreite(), kKopfHoehe));
    p.setPen(leise);
    for (int takt = 0; takt < m_muster.takte; ++takt)
        p.drawText(QRect(xVonSchritt(takt * jeTakt) + 3, 0, 80, kKopfHoehe), Qt::AlignLeft | Qt::AlignVCenter,
                   tr("Bar %1").arg(takt + 1));

    // --- unten: die Lautstaerke je Note, an ihrem Einsatz
    p.setClipRect(QRect(kTastenBreite, unten, sichtBreite(), kLautHoehe));
    p.setPen(leise);
    p.drawLine(kTastenBreite, unten, width(), unten);
    p.setPen(Qt::NoPen);
    for (int i = 0; i < static_cast<int>(m_muster.noten.size()); ++i)
    {
        const Note& note = m_muster.noten[static_cast<std::size_t>(i)];
        const int hoch = std::max(2, static_cast<int>(note.laut * (kLautHoehe - 6) + 0.5));
        QColor farbe = an;
        if (i != gewaehlt) farbe.setAlpha(150);
        p.setBrush(farbe);
        p.drawRect(QRect(xVonSchritt(note.schritt) + 2, height() - 1 - hoch, kSchrittBreite - 5, hoch));
    }
    p.setClipping(false);
    p.setFont(font());
    p.setPen(leise);
    p.setFont(klein);
    // (die Spalte der Klaviatur ist schmal: „Note volume" passt nicht hinein)
    p.drawText(QRect(2, unten, kTastenBreite - 4, kLautHoehe), Qt::AlignLeft | Qt::AlignVCenter, tr("Note\nvol."));

    // --- die Abspielmarke
    if (m_marke >= 0.0 && m_marke < zahl)
    {
        const int x = kTastenBreite + static_cast<int>(m_marke * kSchrittBreite) - m_rollX;
        if (x >= kTastenBreite)
        {
            p.setPen(QPen(QColor(255, 90, 70), 2.0));
            p.drawLine(x, 0, x, height());
        }
    }
}

void NotenRaster::zieheBei(QPoint ort)
{
    const int zahl = schritte();
    if (zahl <= 0) return;
    const int schrittRoh = std::clamp(teileAb(ort.x() - kTastenBreite + m_rollX, kSchrittBreite), 0, zahl - 1);
    if (m_zug == Zug::Laut)
    {
        const int schritt = schrittBei(ort.x());
        if (schritt < 0) return;
        const double hoehe = 1.0 - static_cast<double>(ort.y() - lautOben() - 3) / (kLautHoehe - 6);
        const double laut = std::clamp(std::round(hoehe * 100.0) / 100.0, kLautMin, 1.0);
        // die gewaehlte Note, wenn sie hier beginnt — sonst alle, die hier beginnen
        const int gewaehlt = auswahl();
        const bool nurGewaehlte =
            gewaehlt >= 0 && m_muster.noten[static_cast<std::size_t>(gewaehlt)].schritt == schritt;
        for (int i = 0; i < static_cast<int>(m_muster.noten.size()); ++i)
        {
            Note& note = m_muster.noten[static_cast<std::size_t>(i)];
            if (note.schritt == schritt && (!nurGewaehlte || i == gewaehlt)) note.laut = laut;
        }
        update();
        return;
    }
    const int platz = auswahl();
    if (platz < 0) return;
    Note& note = m_muster.noten[static_cast<std::size_t>(platz)];
    if (m_zug == Zug::Laenge)
    {
        note.laenge = std::clamp(schrittRoh - note.schritt + 1, 1, zahl - note.schritt);
    }
    else if (m_zug == Zug::Verschieben)
    {
        const int schritt = std::clamp(schrittRoh - m_griffSchritt, 0, zahl - note.laenge);
        const int hoehe = std::clamp(kHoeheMax - teileAb(ort.y() - kKopfHoehe + m_rollY, kZeilenHoehe), kHoeheMin, kHoeheMax);
        if (belegt(schritt, hoehe, platz)) return;  // dort beginnt schon eine Note dieser Tonhoehe
        note.schritt = schritt;
        note.hoehe = hoehe;
        m_wahlSchritt = schritt;
        m_wahlHoehe = hoehe;
    }
    update();
}

void NotenRaster::mousePressEvent(QMouseEvent* event)
{
    const QPoint ort = event->position().toPoint();
    setFocus(Qt::MouseFocusReason);
    if (event->button() == Qt::RightButton)
    {
        const int platz = noteBei(ort);
        if (platz < 0 || m_zug != Zug::Keiner) return;
        const Note& note = m_muster.noten[static_cast<std::size_t>(platz)];
        m_wahlSchritt = note.schritt;
        m_wahlHoehe = note.hoehe;
        (void)loescheAuswahl();
        return;
    }
    if (event->button() != Qt::LeftButton) return;
    if (ort.y() >= lautOben())
    {
        if (ort.x() < kTastenBreite) return;
        m_zug = Zug::Laut;
        m_neu = false;
        m_vorZug = m_muster;
        zieheBei(ort);
        return;
    }
    const int hoehe = hoeheBei(ort.y());
    if (hoehe < 0) return;
    if (ort.x() < kTastenBreite)
    {
        m_taste = hoehe;  // sie leuchtet, solange die Maus gedrueckt ist
        update();
        Q_EMIT tonVerlangt(hoehe);
        return;
    }
    const int schritt = schrittBei(ort.x());
    if (schritt < 0) return;
    m_vorZug = m_muster;
    const int platz = noteBei(ort);
    if (platz >= 0)
    {
        const Note& note = m_muster.noten[static_cast<std::size_t>(platz)];
        m_wahlSchritt = note.schritt;
        m_wahlHoehe = note.hoehe;
        m_neu = false;
        m_griffSchritt = schritt - note.schritt;
        m_zug = anKante(note, ort.x()) ? Zug::Laenge : Zug::Verschieben;
    }
    else
    {
        // eine neue Note von einem Schritt; weitergezogen wird sie laenger
        m_muster.noten.push_back({schritt, 1, hoehe, 1.0});
        m_wahlSchritt = schritt;
        m_wahlHoehe = hoehe;
        m_neu = true;
        m_zug = Zug::Laenge;
    }
    update();
}

void NotenRaster::mouseMoveEvent(QMouseEvent* event)
{
    if (m_zug != Zug::Keiner && (event->buttons() & Qt::LeftButton))
        zieheBei(event->position().toPoint());
    else if (event->buttons() == Qt::NoButton)
        zeigeZeiger(event->position().toPoint());
}

void NotenRaster::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_taste >= 0)
    {
        m_taste = -1;
        update();
    }
    if (event->button() != Qt::LeftButton || m_zug == Zug::Keiner) return;
    const Zug zug = m_zug;
    m_zug = Zug::Keiner;
    if (m_muster == m_vorZug) return;
    const QString was = m_neu                       ? tr("Add note")
                        : zug == Zug::Laenge        ? tr("Change note length")
                        : zug == Zug::Verschieben   ? tr("Move note")
                                                    : tr("Change note volume");
    Q_EMIT geaendert(m_muster, was);
}

void NotenRaster::keyPressEvent(QKeyEvent* event)
{
    if ((event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) && loescheAuswahl()) return;
    QWidget::keyPressEvent(event);
}

void NotenRaster::wheelEvent(QWheelEvent* event)
{
    // das Rad rollt die Tonhoehen, mit Shift die Zeit
    const int rasten = event->angleDelta().y() != 0 ? event->angleDelta().y() : event->angleDelta().x();
    if (rasten == 0) return;
    if (event->modifiers() & Qt::ShiftModifier)
        setRollen(m_rollX - rasten * kSchrittBreite * 4 / 120, m_rollY);
    else
        setRollen(m_rollX, m_rollY - rasten * kZeilenHoehe * 3 / 120);
    Q_EMIT gerollt();
    event->accept();
}

void NotenRaster::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    setRollen(m_rollX, m_rollY);
    Q_EMIT gerollt();
}

// =============================================================================
// InstrumentEditor
// =============================================================================

InstrumentEditor::InstrumentEditor(QWidget* parent) : QWidget(parent, Qt::Tool)
{
    setObjectName(QStringLiteral("loopInstrumentEditor"));
    setupUI();
}

void InstrumentEditor::setupUI()
{
    auto* aussen = new QVBoxLayout(this);

    auto* kopf = new QHBoxLayout;
    m_basisName = new QLabel(this);
    m_basisName->setObjectName(QStringLiteral("instrumentBase"));
    m_nutzer = new QLabel(this);
    m_nutzer->setObjectName(QStringLiteral("instrumentUsers"));
    m_nutzer->setEnabled(false);  // grau: eine Auskunft
    m_takte = new QSpinBox(this);
    m_takte->setObjectName(QStringLiteral("instrumentBars"));
    m_takte->setRange(kTakteMin, kTakteMax);
    m_takte->setKeyboardTracking(false);
    m_takte->setToolTip(tr("Length of the pattern in bars; its clips grow and shrink with it"));
    m_play = new QPushButton(tr("Play pattern"), this);
    m_play->setObjectName(QStringLiteral("instrumentPlay"));
    m_play->setToolTip(tr("Play only this pattern, in a loop"));
    m_spitze = new QLabel(QStringLiteral("–"), this);
    m_spitze->setObjectName(QStringLiteral("instrumentPeak"));
    m_spitze->setMinimumWidth(36);
    m_spitze->setToolTip(tr("Peak of the mixed loop while it plays; above 1 it clips"));
    kopf->addWidget(new QLabel(tr("Base"), this));
    kopf->addWidget(m_basisName);
    kopf->addWidget(m_nutzer);
    kopf->addStretch(1);
    kopf->addWidget(new QLabel(tr("Bars"), this));
    kopf->addWidget(m_takte);
    kopf->addWidget(m_play);
    kopf->addWidget(new QLabel(tr("Peak"), this));
    kopf->addWidget(m_spitze);
    aussen->addLayout(kopf);

    auto* zeile = new QHBoxLayout;
    m_instrument = new QComboBox(this);
    m_instrument->setObjectName(QStringLiteral("instrumentSound"));
    m_instrument->setMinimumWidth(130);
    m_laut = new QSlider(Qt::Horizontal, this);
    m_laut->setObjectName(QStringLiteral("instrumentVolume"));
    m_laut->setRange(0, 100);
    m_laut->setFixedWidth(110);
    m_laut->setToolTip(tr("Volume of the pattern"));
    m_pan = new QSlider(Qt::Horizontal, this);
    m_pan->setObjectName(QStringLiteral("instrumentPan"));
    m_pan->setRange(-100, 100);
    m_pan->setFixedWidth(110);
    m_pan->setToolTip(tr("Pan: left to right"));
    m_stumm = new QToolButton(this);
    m_stumm->setObjectName(QStringLiteral("instrumentMute"));
    m_stumm->setText(QStringLiteral("M"));
    m_stumm->setCheckable(true);
    m_stumm->setToolTip(tr("Mute this pattern"));
    zeile->addWidget(new QLabel(tr("Instrument"), this));
    zeile->addWidget(m_instrument);
    zeile->addWidget(new QLabel(tr("Volume"), this));
    zeile->addWidget(m_laut);
    zeile->addWidget(new QLabel(tr("Pan"), this));
    zeile->addWidget(m_pan);
    zeile->addWidget(m_stumm);
    m_hinweis = new QLabel(this);
    m_hinweis->setObjectName(QStringLiteral("instrumentHint"));
    m_hinweis->setEnabled(false);  // grau: eine Auskunft
    zeile->addSpacing(12);
    zeile->addWidget(m_hinweis);
    zeile->addStretch(1);
    aussen->addLayout(zeile);

    // --- das Raster mit seinen Rollbalken
    auto* koerper = new QGridLayout;
    koerper->setSpacing(0);
    m_raster = new NotenRaster(this);
    m_rollY = new QScrollBar(Qt::Vertical, this);
    m_rollX = new QScrollBar(Qt::Horizontal, this);
    koerper->addWidget(m_raster, 0, 0);
    koerper->addWidget(m_rollY, 0, 1);
    koerper->addWidget(m_rollX, 1, 0);
    koerper->setRowStretch(0, 1);
    koerper->setColumnStretch(0, 1);
    aussen->addLayout(koerper, 1);

    connect(m_takte, &QSpinBox::valueChanged, this, [this](int takte) { setzeTakte(takte); });
    connect(m_play, &QPushButton::clicked, this, [this] { Q_EMIT spielenVerlangt(!m_spielt); });
    connect(m_instrument, &QComboBox::activated, this,
            [this](int index) { setzeInstrument(m_instrument->itemData(index).toString()); });
    connect(m_stumm, &QToolButton::toggled, this, [this](bool stumm) { setzeStumm(stumm); });
    // ein Schieber meldet beim Loslassen — nicht jeden Pixel als Schritt fuer Undo
    const auto schieber = [this](QSlider* regler, bool pan) {
        const auto melde = [this, regler, pan] {
            if (pan)
                setzePan(regler->value() / 100.0);
            else
                setzeLaut(regler->value() / 100.0);
        };
        connect(regler, &QSlider::sliderReleased, this, melde);
        connect(regler, &QSlider::valueChanged, this, [regler, melde] {
            if (!regler->isSliderDown()) melde();
        });
    };
    schieber(m_laut, false);
    schieber(m_pan, true);

    connect(m_raster, &NotenRaster::geaendert, this, [this](const Muster& neu, const QString& was) {
        Loop fassung = m_loop;
        Muster* ziel = musterIn(fassung, m_muster);
        if (ziel == nullptr) return;
        ziel->noten = neu.noten;
        fassung.richte();  // sortiert die Noten, kuerzt am Ende des Musters
        if (fassung == m_loop)
        {
            zeigeAlles();  // nichts geaendert: das Raster zeigt wieder, was gilt
            return;
        }
        Q_EMIT bearbeitet(fassung, was);
    });
    connect(m_raster, &NotenRaster::tonVerlangt, this, &InstrumentEditor::tonVerlangt);
    connect(m_raster, &NotenRaster::gerollt, this, &InstrumentEditor::passeRollbalkenAn);
    connect(m_rollX, &QScrollBar::valueChanged, this, [this](int x) { m_raster->setRollen(x, m_raster->rollY()); });
    connect(m_rollY, &QScrollBar::valueChanged, this, [this](int y) { m_raster->setRollen(m_raster->rollX(), y); });
    resize(1100, 560);
}

void InstrumentEditor::passeRollbalkenAn()
{
    const QSignalBlocker a(m_rollX);
    const QSignalBlocker b(m_rollY);
    m_rollX->setRange(0, m_raster->rollXMax());
    m_rollX->setPageStep(std::max(1, m_raster->sichtBreite()));
    m_rollX->setSingleStep(NotenRaster::kSchrittBreite);
    m_rollX->setValue(m_raster->rollX());
    m_rollY->setRange(0, m_raster->rollYMax());
    m_rollY->setPageStep(std::max(1, m_raster->sichtHoehe()));
    m_rollY->setSingleStep(NotenRaster::kZeilenHoehe);
    m_rollY->setValue(m_raster->rollY());
    // ein Balken, der nichts zu rollen hat, bleibt weg
    m_rollX->setVisible(m_raster->rollXMax() > 0);
}

void InstrumentEditor::zeigeAusschnitt()
{
    // auf die Noten — ohne Noten um C4 (bei der Hoehe des Fensters rund C3 bis C5)
    int mitte = 60;
    if (const Muster* ziel = basis(); ziel != nullptr && !ziel->noten.empty())
    {
        int tief = kHoeheMax;
        int hoch = kHoeheMin;
        for (const Note& note : ziel->noten)
        {
            tief = std::min(tief, note.hoehe);
            hoch = std::max(hoch, note.hoehe);
        }
        mitte = (tief + hoch) / 2;
    }
    m_raster->setRollen(0, m_raster->rollY());
    m_raster->zeigeHoehe(mitte);
    passeRollbalkenAn();
}

void InstrumentEditor::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    if (m_gezeigt) return;
    m_gezeigt = true;
    zeigeAusschnitt();
}

bool InstrumentEditor::zeige(const Loop& loop, const QString& muster)
{
    const Muster* ziel = loop.findeMuster(muster);
    if (ziel == nullptr || ziel->art != MusterArt::Instrument) return false;
    const bool andere = muster != m_muster;
    m_loop = loop;
    m_muster = muster;
    zeigeAlles();
    if (andere) zeigeAusschnitt();
    return true;
}

void InstrumentEditor::setLoop(const Loop& loop)
{
    if (m_muster.isEmpty()) return;
    const QString folge = folgeBasis(m_loop, m_muster, loop);
    m_loop = loop;
    if (folge.isEmpty())
    {
        // die Basis gibt es nicht mehr (ihr letzter Clip ist geloescht, ein anderer Loop geladen)
        m_muster.clear();
        close();
        return;
    }
    m_muster = folge;
    zeigeAlles();
}

void InstrumentEditor::zeigeAlles()
{
    const Muster* ziel = basis();
    if (ziel == nullptr) return;
    setWindowTitle(tr("Instrument pattern: %1").arg(m_muster));
    m_basisName->setText(m_muster);
    m_nutzer->setText(tr("· used by %1 clips").arg(basisNutzer(m_loop, m_muster)));
    {
        const QSignalBlocker a(m_takte);
        const QSignalBlocker b(m_instrument);
        const QSignalBlocker c(m_laut);
        const QSignalBlocker d(m_pan);
        const QSignalBlocker e(m_stumm);
        m_takte->setValue(ziel->takte);
        m_instrument->clear();
        for (const Instrument& instrument : instrumente()) m_instrument->addItem(instrument.titel, instrument.name);
        // ein Instrument, das dieses Programm nicht kennt, bleibt stehen — es schweigt
        if (m_instrument->findData(ziel->instrument) < 0)
            m_instrument->addItem(tr("%1 (not found)").arg(ziel->instrument), ziel->instrument);
        m_instrument->setCurrentIndex(m_instrument->findData(ziel->instrument));
        if (!m_laut->isSliderDown()) m_laut->setValue(static_cast<int>(std::lround(ziel->laut * 100.0)));
        if (!m_pan->isSliderDown()) m_pan->setValue(static_cast<int>(std::lround(ziel->pan * 100.0)));
        m_stumm->setChecked(ziel->stumm);
    }
    m_raster->setMuster(*ziel, m_loop.schlaegeJeTakt, m_loop.schritteJeSchlag);
    passeRollbalkenAn();
}

void InstrumentEditor::setSpielt(bool spielt, double schritt)
{
    if (spielt != m_spielt)
    {
        m_spielt = spielt;
        m_play->setText(spielt ? tr("Stop") : tr("Play pattern"));
    }
    m_raster->setMarke(spielt ? schritt : -1.0);
}

void InstrumentEditor::setHinweis(const QString& text)
{
    if (m_hinweis->text() != text) m_hinweis->setText(text);
}

void InstrumentEditor::setSpitze(double spitze)
{
    const QString text = spitze < 0.0 ? QStringLiteral("–") : QString::number(spitze, 'f', 2);
    if (text == m_spitze->text()) return;
    m_spitze->setText(text);
    // ueber 1 uebersteuert die Summe (L6 a)
    m_spitze->setStyleSheet(spitze > 1.0 ? QStringLiteral("color: #e05050; font-weight: bold;") : QString());
}

// --- die Aenderungen: jede meldet eine neue Fassung des Loops ----------------

void InstrumentEditor::setzeTakte(int takte)
{
    Loop neu = m_loop;
    if (!setzeMusterTakte(neu, m_muster, takte) || neu == m_loop) return;
    Q_EMIT bearbeitet(neu, tr("Change pattern length"));
}

void InstrumentEditor::setzeInstrument(const QString& name)
{
    Loop neu = m_loop;
    Muster* ziel = musterIn(neu, m_muster);
    if (ziel == nullptr || name.isEmpty()) return;
    ziel->instrument = name;
    if (neu == m_loop) return;
    Q_EMIT bearbeitet(neu, tr("Change instrument"));
}

void InstrumentEditor::setzeLaut(double laut)
{
    Loop neu = m_loop;
    Muster* ziel = musterIn(neu, m_muster);
    if (ziel == nullptr) return;
    ziel->laut = std::clamp(laut, 0.0, 1.0);
    if (neu == m_loop) return;
    Q_EMIT bearbeitet(neu, tr("Change pattern volume"));
}

void InstrumentEditor::setzePan(double pan)
{
    Loop neu = m_loop;
    Muster* ziel = musterIn(neu, m_muster);
    if (ziel == nullptr) return;
    ziel->pan = std::clamp(pan, -1.0, 1.0);
    if (neu == m_loop) return;
    Q_EMIT bearbeitet(neu, tr("Change pattern pan"));
}

void InstrumentEditor::setzeStumm(bool stumm)
{
    Loop neu = m_loop;
    Muster* ziel = musterIn(neu, m_muster);
    if (ziel == nullptr) return;
    ziel->stumm = stumm;
    if (neu == m_loop) return;
    Q_EMIT bearbeitet(neu, tr("Mute pattern"));
}

}  // namespace lumi::loop
