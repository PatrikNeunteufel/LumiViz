/**
 * @file InstrumentCreator.cpp
 * @brief Loop: der Instrument-Creator (Stufe 10b)
 */

#include "loop/InstrumentCreator.hpp"

#include "audio/IAudioEngine.hpp"
#include "loop/AufnahmeFenster.hpp"
#include "loop/FormFenster.hpp"
#include "loop/Kit.hpp"  // bibliotheksKlaenge
#include "loop/SampleFrage.hpp"
#include "loop/SignalWeg.hpp"

#include <QAction>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QColorDialog>
#include <QCompleter>
#include <QDialog>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QSignalBlocker>
#include <QSlider>
#include <QStyle>
#include <QStyleOptionViewItem>
#include <QStringListModel>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>
#include <array>
#include <cmath>
#include <utility>

namespace lumi::loop
{

namespace
{

const char* const kLetzterOrdner = "loop/lastInstrumentDir";
const char* const kLetzterSampleOrdner = "loop/lastSampleDir";
const char* const kLetzterFormOrdner = "loop/lastShapeDir";
const char* const kGruppeZu = "loop/creatorFolded/";  ///< dahinter der Schluessel der Gruppe

/// Wird die Stellung der Gruppen gelesen und gemerkt? (Tests schalten es ab.)
bool& gruppenMerken()
{
    static bool an = true;
    return an;
}

const QColor kGriffRot(216, 90, 48);    ///< Sample start
const QColor kGriffBlau(55, 138, 221);  ///< Sustain loop

/// Vorschlaege fuer das Feld Genres: es traegt mehrere, mit Kommas getrennt —
/// vorgeschlagen wird fuer das Stueck hinter dem letzten Komma, und die Wahl
/// ersetzt nur dieses Stueck.
class GenreVorschlag : public QCompleter
{
public:
    explicit GenreVorschlag(QLineEdit* feld) : QCompleter(feld), m_feld(feld)
    {
        setModel(new QStringListModel(bekannteGenres(), this));
        setCaseSensitivity(Qt::CaseInsensitive);
    }
    void liesNeu() { static_cast<QStringListModel*>(model())->setStringList(bekannteGenres()); }

protected:
    [[nodiscard]] QStringList splitPath(const QString& pfad) const override
    {
        return {pfad.mid(pfad.lastIndexOf(QLatin1Char(',')) + 1).trimmed()};
    }
    [[nodiscard]] QString pathFromIndex(const QModelIndex& index) const override
    {
        const QString text = m_feld->text();
        const int komma = text.lastIndexOf(QLatin1Char(','));
        return (komma < 0 ? QString() : text.left(komma + 1) + QLatin1Char(' ')) + QCompleter::pathFromIndex(index);
    }

private:
    QLineEdit* m_feld;
};
constexpr int kBildRate = 44100;
constexpr int kObertonRegler = 8;         ///< der 2. bis 9. Teilton als Regler
constexpr double kDaempfungVon = 0.98;    ///< der Regler Damping reicht von hier …
constexpr double kDaempfungBis = 0.9999;  ///< … bis hier

int gerundet(double wert)
{
    return static_cast<int>(std::lround(wert));
}

/// Pan als Text: „C" in der Mitte, sonst „L 20" oder „R 100" (in Hundertsteln).
QString panText(double pan)
{
    const int wert = gerundet(pan * 100.0);
    if (wert == 0) return QStringLiteral("C");
    return QStringLiteral("%1 %2").arg(wert < 0 ? QStringLiteral("L") : QStringLiteral("R")).arg(std::abs(wert));
}

bool schwarz(int hoehe)
{
    const int stufe = ((hoehe % 12) + 12) % 12;
    return stufe == 1 || stufe == 3 || stufe == 6 || stufe == 8 || stufe == 10;
}

}  // namespace

// =============================================================================
// KurvenBild
// =============================================================================

KurvenBild::KurvenBild(QWidget* parent) : QWidget(parent)
{
    setMinimumHeight(80);
}

void KurvenBild::setWerte(std::vector<float> werte, bool nurOben, double marke, const QString& markenText)
{
    m_werte = std::move(werte);
    m_nurOben = nurOben;
    m_marke = marke;
    m_markenText = markenText;
    update();
}

void KurvenBild::setZweite(std::vector<float> werte)
{
    m_zweite = std::move(werte);
    update();
}

QPolygonF KurvenBild::linie(const std::vector<float>& werte) const
{
    const int rand = 6;
    const double breite = width() - 2.0 * rand;
    const double hoch = height() - 2.0 * rand;
    const double null = m_nurOben ? rand + hoch : rand + hoch / 2.0;
    QPolygonF punkte;
    for (std::size_t i = 0; i < werte.size(); ++i)
    {
        const double x = rand + breite * static_cast<double>(i) / static_cast<double>(werte.size() - 1);
        const double wert = std::clamp(static_cast<double>(werte[i]), -1.0, 1.0);
        punkte << QPointF(x, m_nurOben ? null - wert * hoch : null - wert * hoch / 2.0);
    }
    return punkte;
}

void KurvenBild::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QPalette pal = palette();
    p.fillRect(rect(), pal.color(QPalette::Base));
    QColor leise = pal.color(QPalette::Text);
    leise.setAlpha(90);
    p.setPen(leise);
    p.drawRect(rect().adjusted(0, 0, -1, -1));
    const int rand = 6;
    const double breite = width() - 2.0 * rand;
    const double hoch = height() - 2.0 * rand;
    const double null = m_nurOben ? rand + hoch : rand + hoch / 2.0;
    p.drawLine(QPointF(rand, null), QPointF(rand + breite, null));
    if (m_marke >= 0.0 && m_marke <= 1.0)
    {
        const double x = rand + m_marke * breite;
        p.setPen(QPen(leise, 1.0, Qt::DashLine));
        p.drawLine(QPointF(x, rand), QPointF(x, rand + hoch));
        p.setPen(leise);
        p.drawText(QRectF(x + 4, rand, 120, 16), Qt::AlignLeft | Qt::AlignVCenter, m_markenText);
    }
    if (m_zweite.size() >= 2)
    {
        // die gewaehlte Schicht: gestrichelt, unter der Summe
        QColor zweite = pal.color(QPalette::Text);
        zweite.setAlpha(170);
        p.setPen(QPen(zweite, 1.2, Qt::DashLine));
        p.drawPolyline(linie(m_zweite));
    }
    // der Bereich liegt unter der Kurve, die Griffe darueber
    const bool bereich = m_griffe[GriffVon] >= 0.0 && m_griffe[GriffBis] > m_griffe[GriffVon];
    if (bereich && !m_aussenGrau)
    {
        QColor flaeche = kGriffBlau;
        flaeche.setAlpha(46);
        p.fillRect(QRectF(QPointF(xVon(m_griffe[GriffVon]), rand), QPointF(xVon(m_griffe[GriffBis]), rand + hoch)), flaeche);
    }
    if (m_werte.size() >= 2 && m_gespiegelt)
    {
        // eine Aufnahme: die Betraege als Flaeche um die Nulllinie
        QPolygonF flaeche = linie(m_werte);
        for (qsizetype i = flaeche.size() - 1; i >= 0; --i) flaeche << QPointF(flaeche[i].x(), 2.0 * null - flaeche[i].y());
        QColor farbe = pal.color(QPalette::Highlight);
        farbe.setAlpha(190);
        p.setPen(Qt::NoPen);
        p.setBrush(farbe);
        p.drawPolygon(flaeche);
        p.setBrush(Qt::NoBrush);
    }
    else if (m_werte.size() >= 2)
    {
        p.setPen(QPen(pal.color(QPalette::Highlight), 1.8));
        p.drawPolyline(linie(m_werte));
    }
    const auto zeichneGriff = [&](Griff welcher, const QColor& farbe) {
        if (m_griffe[welcher] < 0.0) return;
        const double x = xVon(m_griffe[welcher]);
        p.setPen(QPen(farbe, 2.0));
        p.drawLine(QPointF(x, rand), QPointF(x, rand + hoch));
        // eine Fahne oben: daran sieht man, dass sich der Strich greifen laesst
        p.fillRect(QRectF(x - 5.0, 1.0, 10.0, 8.0), farbe);
    };
    if (bereich && m_aussenGrau)
    {
        // ein Zuschnitt: was ausserhalb liegt, faellt weg — grau ueber der Kurve
        QColor grau = pal.color(QPalette::Base);
        grau.setAlpha(175);
        p.fillRect(QRectF(QPointF(rand, rand), QPointF(xVon(m_griffe[GriffVon]), rand + hoch)), grau);
        p.fillRect(QRectF(QPointF(xVon(m_griffe[GriffBis]), rand), QPointF(rand + breite, rand + hoch)), grau);
    }
    if (m_zeiger >= 0.0 && m_zeiger <= 1.0)
    {
        p.setPen(QPen(kGriffBlau, 1.2));
        p.drawLine(QPointF(xVon(m_zeiger), rand), QPointF(xVon(m_zeiger), rand + hoch));
    }
    if (bereich)
    {
        zeichneGriff(GriffVon, m_aussenGrau ? kGriffRot : kGriffBlau);
        zeichneGriff(GriffBis, m_aussenGrau ? kGriffRot : kGriffBlau);
    }
    zeichneGriff(GriffStart, kGriffRot);
}

void KurvenBild::setAussenGrau(bool an)
{
    if (m_aussenGrau == an) return;
    m_aussenGrau = an;
    update();
}

void KurvenBild::setZeiger(double stelle)
{
    if (m_zeiger == stelle) return;
    m_zeiger = stelle;
    update();
}

void KurvenBild::setGespiegelt(bool an)
{
    if (m_gespiegelt == an) return;
    m_gespiegelt = an;
    update();
}

void KurvenBild::setGriffe(double start, double von, double bis)
{
    m_griffe[GriffStart] = start;
    m_griffe[GriffVon] = von;
    m_griffe[GriffBis] = bis;
    // die Maus zeigt, wo sich greifen laesst — auch ohne gedrueckte Taste
    setMouseTracking(start >= 0.0 || von >= 0.0);
    if (start < 0.0 && von < 0.0) unsetCursor();
    update();
}

double KurvenBild::griff(Griff welcher) const
{
    return welcher >= GriffStart && welcher <= GriffBis ? m_griffe[welcher] : -1.0;
}

double KurvenBild::stelleBei(int x) const
{
    const int rand = 6;
    const double breite = std::max(1.0, width() - 2.0 * rand);
    return std::clamp((x - rand) / breite, 0.0, 1.0);
}

int KurvenBild::xVon(double stelle) const
{
    const int rand = 6;
    return gerundet(rand + std::clamp(stelle, 0.0, 1.0) * (width() - 2.0 * rand));
}

KurvenBild::Griff KurvenBild::griffBei(int x) const
{
    Griff naechster = KeinGriff;
    int abstand = kGriffWeite + 1;
    const bool bereich = m_griffe[GriffVon] >= 0.0 && m_griffe[GriffBis] > m_griffe[GriffVon];
    for (const Griff welcher : {GriffStart, GriffVon, GriffBis})
    {
        if (m_griffe[welcher] < 0.0 || (welcher != GriffStart && !bereich)) continue;
        const int weit = std::abs(x - xVon(m_griffe[welcher]));
        if (weit < abstand)  // bei gleichem Abstand gilt der Anfang
        {
            abstand = weit;
            naechster = welcher;
        }
    }
    return naechster;
}

void KurvenBild::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) return;
    m_gezogen = griffBei(event->position().toPoint().x());
}

void KurvenBild::mouseMoveEvent(QMouseEvent* event)
{
    const int x = event->position().toPoint().x();
    if (m_gezogen == KeinGriff)
    {
        if (griffBei(x) != KeinGriff)
            setCursor(Qt::SizeHorCursor);
        else
            unsetCursor();
        return;
    }
    Q_EMIT griffGezogen(m_gezogen, stelleBei(x));
}

void KurvenBild::mouseReleaseEvent(QMouseEvent*)
{
    m_gezogen = KeinGriff;
}

// =============================================================================
// Tastenleiste
// =============================================================================

Tastenleiste::Tastenleiste(QWidget* parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("creatorKeys"));
    setMinimumHeight(44);
    setCursor(Qt::PointingHandCursor);
}

QRect Tastenleiste::tastenRect(int hoehe) const
{
    // 15 weisse Tasten nebeneinander; eine schwarze sitzt auf der Kante davor
    int weisse = 0;
    for (int h = kTief; h < hoehe; ++h)
        if (!schwarz(h)) ++weisse;
    const double breite = width() / 15.0;
    if (schwarz(hoehe))
    {
        const double b = breite * 0.6;
        return QRect(gerundet(weisse * breite - b / 2.0), 0, gerundet(b), gerundet(height() * 0.6));
    }
    return QRect(gerundet(weisse * breite), 0, gerundet(breite) - 1, height() - 1);
}

int Tastenleiste::hoeheBei(QPoint ort) const
{
    for (int h = kTief; h <= kHoch; ++h)
        if (schwarz(h) && tastenRect(h).contains(ort)) return h;
    for (int h = kTief; h <= kHoch; ++h)
        if (!schwarz(h) && tastenRect(h).contains(ort)) return h;
    return -1;
}

QPoint Tastenleiste::ortVon(int hoehe) const
{
    const QRect r = tastenRect(hoehe);
    // eine weisse Taste unten greifen: oben liegen die schwarzen darueber
    return schwarz(hoehe) ? r.center() : QPoint(r.center().x(), r.bottom() - 4);
}

void Tastenleiste::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    const QColor an = palette().color(QPalette::Highlight);
    QFont klein = font();
    klein.setPointSizeF(std::max(6.5, klein.pointSizeF() - 1.0));
    p.setFont(klein);
    for (int h = kTief; h <= kHoch; ++h)
    {
        if (schwarz(h)) continue;
        const QRect r = tastenRect(h);
        p.setPen(QColor(120, 120, 120));
        // gedrueckt: die Farbe der Auswahl; hervorgehoben (Repeat note): dieselbe, heller
        p.setBrush(h == m_gedrueckt ? an : h == m_gewaehlt ? an.lighter(150) : QColor(232, 232, 228));
        p.drawRect(r);
        if (h % 12 == 0)
        {
            p.setPen(QColor(40, 40, 44));
            p.drawText(r.adjusted(2, 0, 0, -2), Qt::AlignLeft | Qt::AlignBottom, tonName(h));
        }
    }
    for (int h = kTief; h <= kHoch; ++h)
    {
        if (!schwarz(h)) continue;
        p.setPen(Qt::NoPen);
        p.setBrush(h == m_gedrueckt ? an : h == m_gewaehlt ? an.lighter(150) : QColor(40, 40, 44));
        p.drawRect(tastenRect(h));
    }
}

void Tastenleiste::setGewaehlt(int hoehe)
{
    const int neu = hoehe >= kTief && hoehe <= kHoch ? hoehe : -1;
    if (neu == m_gewaehlt) return;
    m_gewaehlt = neu;
    update();
}

void Tastenleiste::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) return;
    const int hoehe = hoeheBei(event->position().toPoint());
    if (hoehe < 0) return;
    m_gedrueckt = hoehe;
    update();
    Q_EMIT taste(hoehe);
}

void Tastenleiste::mouseReleaseEvent(QMouseEvent*)
{
    if (m_gedrueckt < 0) return;
    m_gedrueckt = -1;
    update();
}

// =============================================================================
// InstrumentCreator
// =============================================================================

InstrumentCreator::InstrumentCreator(QWidget* parent) : QWidget(parent, Qt::Tool)
{
    setObjectName(QStringLiteral("loopInstrumentCreator"));
    setupUI();
}

QString InstrumentCreator::vorhoerDatei()
{
    return QDir::temp().filePath(QStringLiteral("lumiviz_instrument_preview.lvinst"));
}

InstrumentCreator::Regler& InstrumentCreator::regler(QGridLayout* gitter, const QString& name, const char* kennung,
                                                     int von, int bis, std::function<int(const Schicht&)> lese,
                                                     std::function<void(Schicht&, int)> schreibe,
                                                     std::function<QString(const Schicht&)> text)
{
    Regler r;
    r.name = new QLabel(name, this);
    r.schieber = new QSlider(Qt::Horizontal, this);
    r.schieber->setObjectName(QLatin1String(kennung));
    r.schieber->setRange(von, bis);
    r.wert = new QLabel(this);
    r.wert->setMinimumWidth(58);
    r.wert->setEnabled(false);  // grau: eine Auskunft
    r.lese = std::move(lese);
    r.text = std::move(text);
    const int zeile = gitter->rowCount();
    gitter->addWidget(r.name, zeile, 0);
    gitter->addWidget(r.schieber, zeile, 1);
    gitter->addWidget(r.wert, zeile, 2);
    connect(r.schieber, &QSlider::valueChanged, this, [this, schreibe = std::move(schreibe)](int wert) {
        aendereSchicht([&](Schicht& s) { schreibe(s, wert); });
    });
    m_regler.push_back(std::move(r));
    return m_regler.back();
}

void InstrumentCreator::setupUI()
{
    auto* aussen = new QVBoxLayout(this);

    auto* kopf = new QHBoxLayout;
    m_titel = new QLineEdit(this);
    m_titel->setObjectName(QStringLiteral("creatorName"));
    m_titel->setMaximumWidth(200);
    m_herkunftZeile = new QLabel(this);
    m_herkunftZeile->setObjectName(QStringLiteral("creatorBasedOn"));
    m_herkunftZeile->setEnabled(false);
    m_zurueck = new QPushButton(tr("Reset"), this);
    m_zurueck->setObjectName(QStringLiteral("creatorReset"));
    m_zurueck->setToolTip(tr("Back to the instrument as it was opened or last saved"));
    m_speichern = new QPushButton(tr("Save"), this);
    m_speichern->setObjectName(QStringLiteral("creatorSave"));
    m_speichernUnter = new QPushButton(tr("Save as..."), this);
    m_speichernUnter->setObjectName(QStringLiteral("creatorSaveAs"));
    m_speichernUnter->setToolTip(tr("Save a copy to My files under a new name"));
    m_spitze = new QLabel(this);
    m_spitze->setObjectName(QStringLiteral("creatorPeak"));
    m_spitze->setToolTip(tr("Peak of one note (C4) with these settings"));
    kopf->addWidget(new QLabel(tr("Name"), this));
    kopf->addWidget(m_titel);
    kopf->addWidget(m_herkunftZeile);
    kopf->addStretch(1);
    kopf->addWidget(m_zurueck);
    kopf->addWidget(m_speichern);
    kopf->addWidget(m_speichernUnter);
    m_gesamt = new QSlider(Qt::Horizontal, this);
    m_gesamt->setObjectName(QStringLiteral("creatorMasterLevel"));
    m_gesamt->setRange(0, 100);
    m_gesamt->setFixedWidth(90);
    m_gesamt->setToolTip(tr("Level of the whole instrument"));
    m_gesamtWert = new QLabel(this);
    m_gesamtWert->setMinimumWidth(40);
    m_gesamtWert->setEnabled(false);
    kopf->addWidget(new QLabel(tr("Level"), this));
    kopf->addWidget(m_gesamt);
    kopf->addWidget(m_gesamtWert);
    kopf->addWidget(new QLabel(tr("Peak"), this));
    kopf->addWidget(m_spitze);
    aussen->addLayout(kopf);

    // --- die zweite Zeile (Stufe 14b; Skizze gebilligt, Patrik S114): was das
    // Fenster „Instruments" von einem Instrument zeigt und wonach es filtert
    auto* angaben = new QHBoxLayout;
    m_art = new QComboBox(this);
    m_art->setObjectName(QStringLiteral("creatorType"));
    m_art->setToolTip(tr("Where the instrument is listed in the Instruments window"));
    for (const InstrumentArt& art : instrumentArten())
        m_art->addItem(QLatin1String(art.titel), QLatin1String(art.schluessel));
    m_genres = new QLineEdit(this);
    m_genres->setObjectName(QStringLiteral("creatorGenres"));
    m_genres->setMinimumWidth(150);
    m_genres->setPlaceholderText(tr("Funk, Disco"));
    m_genres->setToolTip(tr("Genres, separated by commas — the Instruments window filters by them"));
    m_genres->setCompleter(new GenreVorschlag(m_genres));
    m_beschreibung = new QLineEdit(this);
    m_beschreibung->setObjectName(QStringLiteral("creatorDescription"));
    m_beschreibung->setPlaceholderText(tr("One sentence about the instrument"));
    m_beschreibung->setMaxLength(200);
    angaben->addWidget(new QLabel(tr("Type"), this));
    angaben->addWidget(m_art);
    angaben->addWidget(new QLabel(tr("Genres"), this));
    angaben->addWidget(m_genres);
    angaben->addWidget(new QLabel(tr("Description"), this));
    angaben->addWidget(m_beschreibung, 1);
    aussen->addLayout(angaben);

    auto* koerper = new QHBoxLayout;

    // --- links: die Schichten
    auto* links = new QVBoxLayout;
    {
        auto* zeile = new QLabel(tr("Layers"), this);
        QFont fett = zeile->font();
        fett.setBold(true);
        zeile->setFont(fett);
        links->addWidget(zeile);
    }
    m_liste = new QListWidget(this);
    m_liste->setObjectName(QStringLiteral("creatorLayers"));
    m_liste->setToolTip(tr("Click a layer to edit it; the check mark mutes it; click the colour dot to choose its colour"));
    // der Farbpunkt (Stufe 20d): ein Klick darauf oder ein Rechtsklick auf die Schicht oeffnet die Farbwahl
    m_liste->viewport()->installEventFilter(this);
    m_liste->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_liste, &QListWidget::customContextMenuRequested, this, [this](QPoint wo) {
        if (QListWidgetItem* eintrag = m_liste->itemAt(wo)) zeigeFarbMenue(m_liste->row(eintrag), m_liste->viewport()->mapToGlobal(wo));
    });
    links->addWidget(m_liste, 1);
    const auto knopf = [this](const QString& text, const char* kennung, const QString& hilfe) {
        auto* k = new QPushButton(text, this);
        k->setObjectName(QLatin1String(kennung));
        k->setToolTip(hilfe);
        return k;
    };
    m_neu = knopf(tr("Add layer"), "creatorAddLayer", tr("A new layer: sine, one voice (up to %1 layers)").arg(kSchichtenMax));
    m_neuAus = knopf(tr("Add layer from instrument..."), "creatorAddLayerFrom",
                     tr("Copy the layers of another instrument into this one"));
    m_doppelt = knopf(tr("Duplicate"), "creatorDuplicateLayer", tr("The selected layer once more"));
    m_weg = knopf(tr("Remove"), "creatorRemoveLayer", tr("Remove the selected layer"));
    m_hinauf = knopf(QStringLiteral("▲"), "creatorLayerUp", tr("Move the selected layer up"));
    m_hinunter = knopf(QStringLiteral("▼"), "creatorLayerDown", tr("Move the selected layer down"));
    m_hinauf->setFixedWidth(30);
    m_hinunter->setFixedWidth(30);
    links->addWidget(m_neu);
    links->addWidget(m_neuAus);
    {
        auto* zeile = new QHBoxLayout;
        zeile->addWidget(m_doppelt);
        zeile->addWidget(m_weg);
        zeile->addWidget(m_hinauf);
        zeile->addWidget(m_hinunter);
        links->addLayout(zeile);
    }
    koerper->addLayout(links, 3);

    auto* gitter = new QGridLayout;
    m_gitter = gitter;
    gitter->setColumnStretch(1, 1);
    // Die Ueberschrift einer Gruppe ist ein Knopf: ein Klick klappt ihre Zeilen ein und aus
    // (Wunsch Patrik S116 — das Fenster ist mit jeder Stufe hoeher geworden). Zur Gruppe
    // gehoert, was im Gitter zwischen ihr und der naechsten Ueberschrift steht.
    const auto titel = [&](const QString& text, const char* schluessel) {
        auto* kopf = new QToolButton(this);
        kopf->setObjectName(QStringLiteral("creatorGroup_") + QLatin1String(schluessel));
        kopf->setText(text);
        kopf->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        kopf->setArrowType(Qt::DownArrow);
        kopf->setAutoRaise(true);
        kopf->setCursor(Qt::PointingHandCursor);
        kopf->setToolTip(tr("Click to fold or unfold this group"));
        QFont fett = kopf->font();
        fett.setBold(true);
        kopf->setFont(fett);
        const int zeile = gitter->rowCount();
        gitter->addWidget(kopf, zeile, 0, 1, 3, Qt::AlignLeft);
        const QString name = QLatin1String(schluessel);
        const bool zu = gruppenMerken() && QSettings().value(QLatin1String(kGruppeZu) + name, false).toBool();
        m_gruppen.push_back(Gruppe{name, kopf, zeile, zu});
        connect(kopf, &QToolButton::clicked, this, [this, name] { setzeGruppeZu(name, !gruppeZu(name)); });
        return kopf;
    };
    const auto ms = [](double wert) { return QStringLiteral("%1 ms").arg(gerundet(wert)); };

    // --- die gewaehlte Schicht: was nur sie betrifft
    // Seit Stufe 16c (Skizze gebilligt, Patrik S118, S1 a) steht ueber den Reglern die Zeile
    // Signal path; das Gitter darunter zeigt nur, was zum gewaehlten Kasten gehoert.
    auto* mitte = new QVBoxLayout;
    m_schichtKopf = new QLabel(this);
    m_schichtKopf->setObjectName(QStringLiteral("creatorLayerTitle"));
    {
        QFont fett = m_schichtKopf->font();
        fett.setBold(true);
        m_schichtKopf->setFont(fett);
        m_schichtName = new QLineEdit(this);
        m_schichtName->setObjectName(QStringLiteral("creatorLayerName"));
        auto* zeile = new QHBoxLayout;
        zeile->addWidget(m_schichtKopf);
        zeile->addSpacing(8);
        zeile->addWidget(new QLabel(tr("Name"), this));
        zeile->addWidget(m_schichtName, 1);
        mitte->addLayout(zeile);
    }
    m_wegTitel = new QLabel(this);
    m_wegTitel->setObjectName(QStringLiteral("creatorPathTitle"));
    {
        QFont fett = m_wegTitel->font();
        fett.setBold(true);
        m_wegTitel->setFont(fett);
    }
    {
        // sonst findet niemand, dass sich die Kaesten anklicken und ziehen lassen — neben dem
        // Titel, nicht darunter: jede Zeile hier macht das Fenster hoeher
        auto* hinweis = new QLabel(tr("click a block · drag to move it"), this);
        hinweis->setObjectName(QStringLiteral("creatorPathHint"));
        hinweis->setEnabled(false);
        auto* zeile = new QHBoxLayout;
        zeile->addWidget(m_wegTitel);
        zeile->addStretch(1);
        zeile->addWidget(hinweis);
        mitte->addLayout(zeile);
    }
    m_pfad = new SignalWeg(this);
    m_pfad->setObjectName(QStringLiteral("creatorPath"));
    mitte->addWidget(m_pfad);
    {
        auto* zeile = new QHBoxLayout;
        m_stellenTitel = new QLabel(this);
        m_stellenTitel->setObjectName(QStringLiteral("creatorBlockTitle"));
        QFont fett = m_stellenTitel->font();
        fett.setBold(true);
        m_stellenTitel->setFont(fett);
        m_bausteinVor = knopf(QString::fromUtf8("← Move"), "creatorBlockLeft", tr("Move the selected block one place towards Source"));
        m_bausteinZurueck = knopf(QString::fromUtf8("Move →"), "creatorBlockRight", tr("Move the selected block one place towards Level"));
        m_bausteinWeg = knopf(tr("Remove"), "creatorBlockRemove", QString());
        zeile->addWidget(m_stellenTitel, 1);
        zeile->addWidget(m_bausteinVor);
        zeile->addWidget(m_bausteinZurueck);
        zeile->addWidget(m_bausteinWeg);
        mitte->addLayout(zeile);
    }
    connect(m_pfad, &SignalWeg::gewaehltGeaendert, this, [this](int platz) {
        const int letzter = static_cast<int>(m_pfad->kaesten().size()) - 1;
        waehleStelle(platz <= 0 ? kStelleSource : platz >= letzter ? kStelleLevel : platz - 1);
    });
    // die Plaetze der Zeile zaehlen Source mit: der erste Baustein steht dort auf Platz 1
    connect(m_pfad, &SignalWeg::verschoben, this, [this](int von, int nach) { (void)verschiebeBaustein(von - 1, nach - 1); });
    connect(m_pfad, &SignalWeg::addVerlangt, this, &InstrumentCreator::zeigeAddMenue);
    connect(m_bausteinVor, &QPushButton::clicked, this, [this] { (void)schiebeBaustein(-1); });
    connect(m_bausteinZurueck, &QPushButton::clicked, this, [this] { (void)schiebeBaustein(1); });
    connect(m_bausteinWeg, &QPushButton::clicked, this, [this] { (void)entferneBaustein(); });

    // --- Level: der feste Kasten am Ende der Zeile
    m_zeilenLevel.von = gitter->rowCount();
    regler(gitter, tr("Level"), "creatorLevel", 0, 100, [](const Schicht& i) { return gerundet(i.laut * 100.0); },
           [](Schicht& i, int w) { i.laut = w / 100.0; },
           [](const Schicht& i) { return QStringLiteral("%1 %").arg(gerundet(i.laut * 100.0)); });
    // Pan je Schicht (Stufe 18, Entscheid Z3 a; Skizze gebilligt S120): fuer jede Waveform
    regler(gitter, tr("Pan"), "creatorPan", -100, 100, [](const Schicht& i) { return gerundet(i.pan * 100.0); },
           [](Schicht& i, int w) { i.pan = w / 100.0; }, [](const Schicht& i) { return panText(i.pan); })
        .schieber->setToolTip(tr("Where this layer sits between left and right. Towards one side, the other side gets quieter"));
    // --- Source: der feste Kasten am Anfang — Transpose, Start offset, Oscillator, Vibrato
    m_zeilenLevel.bis = m_zeilenSource.von = gitter->rowCount();
    regler(gitter, tr("Transpose"), "creatorTranspose", -24, 24, [](const Schicht& i) { return i.versatzHalbtoene; },
           [](Schicht& i, int w) { i.versatzHalbtoene = w; },
           [](const Schicht& i) {
               return QStringLiteral("%1%2 st").arg(i.versatzHalbtoene > 0 ? QStringLiteral("+") : QString()).arg(i.versatzHalbtoene);
           })
        .schieber->setToolTip(tr("The layer sounds this many semitones above or below the note"));
    regler(gitter, tr("Start offset"), "creatorStartOffset", 0, 200, [](const Schicht& i) { return gerundet(i.einsatzMs / 5.0); },
           [](Schicht& i, int w) { i.einsatzMs = w * 5.0; }, [ms](const Schicht& i) { return ms(i.einsatzMs); })
        .schieber->setToolTip(tr("The layer starts this long after the note begins (not an echo)"));

    // --- Oscillator
    titel(tr("Oscillator"), "oscillator");
    m_welle = new QComboBox(this);
    m_welle->setObjectName(QStringLiteral("creatorWaveform"));
    m_welle->addItem(tr("Sine"), static_cast<int>(Schwingung::Sinus));
    m_welle->addItem(tr("Triangle"), static_cast<int>(Schwingung::Dreieck));
    m_welle->addItem(tr("Saw"), static_cast<int>(Schwingung::Saege));
    m_welle->addItem(tr("Square"), static_cast<int>(Schwingung::Rechteck));
    m_welle->addItem(tr("Noise"), static_cast<int>(Schwingung::Rauschen));  // Stufe 11d
    m_welle->addItem(tr("Plucked string"), static_cast<int>(Schwingung::Saite));
    m_welle->addItem(tr("Sample"), static_cast<int>(Schwingung::Sample));
    m_welle->addItem(tr("Shape"), static_cast<int>(Schwingung::Form));  // Stufe 19e
    {
        const int zeile = gitter->rowCount();
        gitter->addWidget(new QLabel(tr("Waveform"), this), zeile, 0);
        gitter->addWidget(m_welle, zeile, 1, 1, 2);
    }
    connect(m_welle, &QComboBox::activated, this, [this](int index) {
        const auto welle = static_cast<Schwingung>(m_welle->itemData(index).toInt());
        aendereSchicht([welle](Schicht& s) {
            // zum ersten Mal eine Aufnahme: sie klingt, wie sie ist — voll gehalten
            if (welle == Schwingung::Sample && s.schwingung != Schwingung::Sample && s.sample.isEmpty()) s.halten = 1.0;
            // zum ersten Mal eine Form: eine mitgelieferte, damit gleich etwas klingt
            if (welle == Schwingung::Form && s.form.isEmpty()) s.form = QStringLiteral("heart");
            s.schwingung = welle;
        });
    });

    // --- nur Shape (Stufe 19e; Skizze gebilligt, Patrik S120)
    {
        auto* name = new QLabel(tr("Shape"), this);
        m_formName = new QLabel(this);
        m_formName->setObjectName(QStringLiteral("creatorShapeName"));
        m_formName->setEnabled(false);  // grau: eine Auskunft
        m_formWahl = new QPushButton(tr("Browse..."), this);
        m_formWahl->setObjectName(QStringLiteral("creatorShapeBrowse"));
        m_formWahl->setToolTip(tr("Choose the shape: a built-in one, one of your own, or a shape file"));
        // Edit... (Stufe 20a): die Form im Fenster Edit shape
        m_formBearbeiten = new QPushButton(tr("Edit..."), this);
        m_formBearbeiten->setObjectName(QStringLiteral("creatorShapeEdit"));
        auto* formKnoepfe = new QWidget(this);
        auto* formReihe = new QHBoxLayout(formKnoepfe);
        formReihe->setContentsMargins(0, 0, 0, 0);
        formReihe->addWidget(m_formWahl);
        formReihe->addWidget(m_formBearbeiten);
        const int zeile = gitter->rowCount();
        gitter->addWidget(name, zeile, 0);
        gitter->addWidget(m_formName, zeile, 1);
        gitter->addWidget(formKnoepfe, zeile, 2);
        auto* kontur = new QLabel(tr("Contour"), this);
        m_formKontur = new QComboBox(this);
        m_formKontur->setObjectName(QStringLiteral("creatorShapeContour"));
        m_formKontur->setToolTip(tr("What this layer plays: one contour of the shape, or all ticked contours one after "
                                    "the other in one round of the pen"));
        const int zweite = gitter->rowCount();
        gitter->addWidget(kontur, zweite, 0);
        gitter->addWidget(m_formKontur, zweite, 1, 1, 2);
        m_formZeilen = {name, m_formName, formKnoepfe, m_formWahl, m_formBearbeiten, kontur, m_formKontur};
        connect(m_formWahl, &QPushButton::clicked, this, &InstrumentCreator::zeigeFormWahl);
        connect(m_formBearbeiten, &QPushButton::clicked, this, [this] { (void)oeffneFormBearbeiten(true); });
        connect(m_formKontur, &QComboBox::activated, this, [this](int index) {
            const int kennung = m_formKontur->itemData(index).toInt();
            aendereSchicht([kennung](Schicht& s) { s.formKontur = kennung; });
        });
    }
    // Harmonics: der Regler laeuft in gleichen Verhaeltnissen (40 Schritte je Verdopplung) — unten,
    // wo sich der Klang am staerksten aendert, stellt er fein
    regler(gitter, tr("Harmonics"), "creatorShapeHarmonics", 0, 360,
           [](const Schicht& i) { return gerundet(40.0 * std::log2(std::clamp(i.harmonische, 1.0, kFormHarmonischeMax))); },
           [](Schicht& i, int w) { i.harmonische = std::clamp(std::exp2(w / 40.0), 1.0, kFormHarmonischeMax); },
           [](const Schicht& i) { return i.harmonische < 10.0 ? QString::number(i.harmonische, 'f', 1) : QString::number(gerundet(i.harmonische)); })
        .schieber->setToolTip(tr("How many partials draw the shape: 1 is an ellipse and sounds like a sine, more follow "
                                 "the drawing more closely and sound brighter. High notes get fewer by themselves"));
    m_nurForm.push_back(m_regler.size() - 1);
    regler(gitter, tr("Harmonics envelope"), "creatorShapeHarmonicsEnvelope", -static_cast<int>(kFormHuelleMax),
           static_cast<int>(kFormHuelleMax), [](const Schicht& i) { return gerundet(i.harmonischeHuelle); },
           [](Schicht& i, int w) { i.harmonischeHuelle = w; },
           [](const Schicht& i) {
               const int w = gerundet(i.harmonischeHuelle);
               return w == 0 ? tr("off") : QStringLiteral("%1%2").arg(w > 0 ? QStringLiteral("+ ") : QString::fromUtf8("− ")).arg(std::abs(w));
           })
        .schieber->setToolTip(tr("The envelope of the layer adds this many partials on top of Harmonics (or takes them "
                                 "away): to the right a note starts bright and gets rounder, to the left it starts round"));
    m_nurForm.push_back(m_regler.size() - 1);
    regler(gitter, tr("Angle"), "creatorShapeAngle", 0, 360, [](const Schicht& i) { return gerundet(i.winkelGrad); },
           [](Schicht& i, int w) { i.winkelGrad = w; },
           [](const Schicht& i) { return QString::fromUtf8("%1°").arg(gerundet(i.winkelGrad)); })
        .schieber->setToolTip(tr("Turns the shape before the tone is taken from it: 0° is the sideways movement of the "
                                 "pen, 90° its up-and-down movement. Changes the sound; with Stereo XY it turns the picture"));
    m_nurForm.push_back(m_regler.size() - 1);
    regler(gitter, tr("Start"), "creatorShapeStart", 0, 100, [](const Schicht& i) { return gerundet(i.formStartProzent); },
           [](Schicht& i, int w) { i.formStartProzent = w; },
           [](const Schicht& i) { return QStringLiteral("%1 %").arg(gerundet(i.formStartProzent)); })
        .schieber->setToolTip(tr("Where on the round of the pen every note begins. On its own you hardly hear it — it "
                                 "matters behind a Drive, next to a second layer with the same shape, and in a scope"));
    m_nurForm.push_back(m_regler.size() - 1);
    {
        m_formStereo = new QCheckBox(tr("Stereo XY"), this);
        m_formStereo->setObjectName(QStringLiteral("creatorShapeStereoXY"));
        m_formStereo->setToolTip(tr("Left plays the sideways movement of the pen, right its up-and-down movement — an "
                                    "XY scope then draws the shape"));
        m_formRueckwaerts = new QCheckBox(tr("Reverse"), this);
        m_formRueckwaerts->setObjectName(QStringLiteral("creatorShapeReverse"));
        m_formRueckwaerts->setToolTip(tr("The pen runs the other way round. On its own you hardly hear it — it matters "
                                         "behind a Drive, next to a second layer with the same shape, and in a scope"));
        auto* haken = new QWidget(this);
        auto* reihe = new QHBoxLayout(haken);
        reihe->setContentsMargins(0, 0, 0, 0);
        reihe->addWidget(m_formStereo);
        reihe->addWidget(m_formRueckwaerts);
        reihe->addStretch(1);
        gitter->addWidget(haken, gitter->rowCount(), 1, 1, 2);
        m_formZeilen.push_back(haken);
        m_formZeilen.push_back(m_formStereo);
        m_formZeilen.push_back(m_formRueckwaerts);
        connect(m_formStereo, &QCheckBox::toggled, this, [this](bool an) { aendereSchicht([an](Schicht& s) { s.stereoXY = an; }); });
        connect(m_formRueckwaerts, &QCheckBox::toggled, this,
                [this](bool an) { aendereSchicht([an](Schicht& s) { s.rueckwaerts = an; }); });
    }

    // --- nur Sample (Stufe 11a; Skizze gebilligt, Patrik S115)
    {
        auto* name = new QLabel(tr("File"), this);
        m_sampleName = new QLabel(this);
        m_sampleName->setObjectName(QStringLiteral("creatorSampleFile"));
        m_sampleName->setEnabled(false);  // grau: eine Auskunft
        m_sampleWahl = new QPushButton(tr("File..."), this);
        m_sampleWahl->setObjectName(QStringLiteral("creatorSampleBrowse"));
        m_sampleWahl->setToolTip(tr("Choose the recording: one of your samples, or a sound file"));
        // Record... (Stufe 11b): eine Aufnahme statt einer Datei
        m_sampleAufnahme = new QPushButton(tr("Record..."), this);
        m_sampleAufnahme->setObjectName(QStringLiteral("creatorSampleRecord"));
        m_sampleAufnahme->setToolTip(tr("Record a sample, trim it and use it for this layer"));
        const int zeile = gitter->rowCount();
        gitter->addWidget(name, zeile, 0);
        gitter->addWidget(m_sampleName, zeile, 1, 1, 2);
        auto* knoepfe = new QWidget(this);
        auto* reihe = new QHBoxLayout(knoepfe);
        reihe->setContentsMargins(0, 0, 0, 0);
        reihe->addWidget(m_sampleWahl);
        reihe->addWidget(m_sampleAufnahme);
        // Edit... (Stufe 17b; Entscheide M1 a und M4 a, Patrik S119): was die Schicht spielt, zuschneiden
        m_sampleBearbeiten = new QPushButton(tr("Edit..."), this);
        m_sampleBearbeiten->setObjectName(QStringLiteral("creatorSampleEdit"));
        m_sampleBearbeiten->setToolTip(tr("Trim the sample this layer plays and save it"));
        reihe->addWidget(m_sampleBearbeiten);
        reihe->addStretch(1);
        gitter->addWidget(knoepfe, gitter->rowCount(), 1, 1, 2);
        // auch die Knoepfe selbst: wer nach ihnen fragt, soll sehen, dass sie weg sind
        m_sampleZeilen = {name, m_sampleName, knoepfe, m_sampleWahl, m_sampleAufnahme, m_sampleBearbeiten};
        connect(m_sampleWahl, &QPushButton::clicked, this, &InstrumentCreator::zeigeSampleWahl);
        connect(m_sampleAufnahme, &QPushButton::clicked, this, [this] { (void)oeffneAufnahme(true); });
        connect(m_sampleBearbeiten, &QPushButton::clicked, this, [this] {
            if (oeffneBearbeiten(true)) return;
            // der Knopf ist nur an, wenn es ein Sample gibt: dann ist es zu lang
            if (const QString grund = bearbeitenHindernis(); !grund.isEmpty()) QMessageBox::information(this, tr("Edit sample"), grund);
        });

        auto* grundton = new QLabel(tr("Root note"), this);
        m_grundton = new QComboBox(this);
        m_grundton->setObjectName(QStringLiteral("creatorRootNote"));
        m_grundton->setToolTip(tr("The note the recording plays — at this note it sounds as recorded"));
        m_grundton->setMaxVisibleItems(24);
        for (int hoehe = 0; hoehe <= 127; ++hoehe) m_grundton->addItem(tonName(hoehe), hoehe);
        const int zweite = gitter->rowCount();
        gitter->addWidget(grundton, zweite, 0);
        gitter->addWidget(m_grundton, zweite, 1);
        m_sampleZeilen.push_back(grundton);
        m_sampleZeilen.push_back(m_grundton);
        // Detect (Schritt 3): messen, vorschlagen — uebernommen wird erst mit Apply
        m_detect = new QPushButton(tr("Detect"), this);
        m_detect->setObjectName(QStringLiteral("creatorDetect"));
        m_detect->setToolTip(tr("Measure the pitch of the recording, from Sample start on, and suggest Root note and Fine tune"));
        gitter->addWidget(m_detect, zweite, 2);
        m_sampleZeilen.push_back(m_detect);
        m_detectText = new QLabel(this);
        m_detectText->setObjectName(QStringLiteral("creatorDetected"));
        m_detectText->setEnabled(false);  // grau: eine Auskunft
        m_detectApply = new QPushButton(tr("Apply"), this);
        m_detectApply->setObjectName(QStringLiteral("creatorDetectApply"));
        m_detectApply->setToolTip(tr("Set Root note and Fine tune to the measured pitch"));
        const int dritte = gitter->rowCount();
        gitter->addWidget(m_detectText, dritte, 1);
        gitter->addWidget(m_detectApply, dritte, 2);
        connect(m_detect, &QPushButton::clicked, this, [this] { (void)messe(); });
        connect(m_detectApply, &QPushButton::clicked, this, [this] { (void)uebernimmGemessenes(); });
        connect(m_grundton, &QComboBox::activated, this, [this](int index) {
            const int hoehe = m_grundton->itemData(index).toInt();
            aendereSchicht([hoehe](Schicht& s) { s.grundton = hoehe; });
        });
    }
    regler(gitter, tr("Fine tune"), "creatorFineTune", -100, 100, [](const Schicht& i) { return gerundet(i.feinCent); },
           [](Schicht& i, int w) { i.feinCent = w; },
           [](const Schicht& i) {
               return QStringLiteral("%1%2 ct").arg(i.feinCent > 0.0 ? QStringLiteral("+") : QString()).arg(gerundet(i.feinCent));
           })
        .schieber->setToolTip(tr("How far the recording lies above (+) or below the root note"));
    m_nurSample.push_back(m_regler.size() - 1);
    // die drei Stellen in der Aufnahme: ihr Bereich ist die Laenge der Aufnahme (`zeigeAlles`)
    regler(gitter, tr("Sample start"), "creatorSampleStart", 0, 1, [](const Schicht& i) { return gerundet(i.sampleStartMs); },
           [](Schicht& i, int w) { i.sampleStartMs = w; }, [ms](const Schicht& i) { return ms(i.sampleStartMs); })
        .schieber->setToolTip(tr("A note starts this far into the recording — the red marker in the picture"));
    m_reglerStart = m_regler.size() - 1;
    m_nurSample.push_back(m_reglerStart);
    {
        auto* name = new QLabel(tr("Sustain loop"), this);
        m_schleife = new QCheckBox(tr("repeat a part while the note is held"), this);
        m_schleife->setObjectName(QStringLiteral("creatorSustainLoop"));
        m_schleife->setToolTip(tr("A recording gets shorter at higher notes. With a sustain loop the part between "
                                  "Loop from and Loop to repeats as long as the note lasts — the blue range in the picture"));
        const int zeile = gitter->rowCount();
        gitter->addWidget(name, zeile, 0);
        gitter->addWidget(m_schleife, zeile, 1, 1, 2);
        m_sampleZeilen.push_back(name);
        m_sampleZeilen.push_back(m_schleife);
        connect(m_schleife, &QCheckBox::toggled, this, &InstrumentCreator::setzeSchleife);
    }
    const auto sekunden = [](double wertMs) { return QStringLiteral("%1 s").arg(wertMs / 1000.0, 0, 'f', 2); };
    regler(gitter, tr("Loop from"), "creatorLoopFrom", 0, 1, [](const Schicht& i) { return gerundet(i.schleifeVonMs); },
           [](Schicht& i, int w) { i.schleifeVonMs = std::min<double>(w, std::max(0.0, i.schleifeBisMs - 1.0)); },
           [sekunden](const Schicht& i) { return sekunden(i.schleifeVonMs); });
    m_reglerVon = m_regler.size() - 1;
    m_nurSample.push_back(m_reglerVon);
    regler(gitter, tr("Loop to"), "creatorLoopTo", 0, 1, [](const Schicht& i) { return gerundet(i.schleifeBisMs); },
           [](Schicht& i, int w) { i.schleifeBisMs = std::max<double>(w, i.schleifeVonMs + 1.0); },
           [sekunden](const Schicht& i) { return sekunden(i.schleifeBisMs); });
    m_reglerBis = m_regler.size() - 1;
    m_nurSample.push_back(m_reglerBis);

    regler(gitter, tr("Voices"), "creatorVoices", 1, 3, [](const Schicht& i) { return i.stimmen; },
           [](Schicht& i, int w) { i.stimmen = w; }, [](const Schicht& i) { return QString::number(i.stimmen); });
    m_nichtSaite.push_back(m_regler.size() - 1);
    m_nichtSample.push_back(m_regler.size() - 1);
    regler(gitter, tr("Detune"), "creatorDetune", 0, 50, [](const Schicht& i) { return gerundet(i.verstimmungCent); },
           [](Schicht& i, int w) { i.verstimmungCent = w; },
           [](const Schicht& i) { return QStringLiteral("%1 ct").arg(gerundet(i.verstimmungCent)); });
    m_nichtSaite.push_back(m_regler.size() - 1);
    m_nichtSample.push_back(m_regler.size() - 1);
    // --- Pitch sweep (Stufe 11d; Skizze gebilligt, Entscheide N1 und N2, Patrik S116): jede Note
    //     beginnt neben ihrer Tonhoehe und kommt auf sie — darueber faellt sie, darunter steigt sie
    regler(gitter, tr("Pitch sweep"), "creatorPitchSweep", static_cast<int>(kTonhoeheStartMin), static_cast<int>(kTonhoeheStartMax),
           [](const Schicht& i) { return gerundet(i.tonhoeheStartHt); }, [](Schicht& i, int w) { i.tonhoeheStartHt = w; },
           [](const Schicht& i) {
               // ein Pfeil sagt die Richtung — die Worte dazu waren fuer die Spalte zu breit
               const int st = gerundet(i.tonhoeheStartHt);
               return st > 0   ? QString::fromUtf8("↓ %1 st").arg(st)
                      : st < 0 ? QString::fromUtf8("↑ %1 st").arg(-st)
                               : tr("off");
           })
        .schieber->setToolTip(tr("Every note starts this many semitones away from its pitch and glides onto it: to the "
                                 "right it starts above and falls (↓ — a kick, a tom), to the left it starts below and "
                                 "rises (↑)"));
    m_nurTonweg.push_back(m_regler.size() - 1);
    regler(gitter, tr("Sweep time"), "creatorSweepTime", 0, 100, [](const Schicht& i) { return gerundet(i.tonhoeheZeitMs / 5.0); },
           [](Schicht& i, int w) { i.tonhoeheZeitMs = w * 5.0; }, [ms](const Schicht& i) { return ms(i.tonhoeheZeitMs); })
        .schieber->setToolTip(tr("After this time the note has reached its pitch"));
    m_reglerWegZeit = m_regler.size() - 1;
    m_nurTonweg.push_back(m_reglerWegZeit);
    for (int k = 0; k < kObertonRegler; ++k)
    {
        const auto platz = static_cast<std::size_t>(k);
        const auto anteil = [platz](const Schicht& i) { return platz < i.obertoene.size() ? i.obertoene[platz] : 0.0; };
        const QByteArray kennung = QByteArrayLiteral("creatorHarmonic") + QByteArray::number(k + 2);
        regler(gitter, tr("Harmonic %1").arg(k + 2), kennung.constData(), 0, 100,
               [anteil](const Schicht& i) { return gerundet(anteil(i) * 100.0); },
               [platz](Schicht& i, int w) {
                   if (i.obertoene.size() <= platz) i.obertoene.resize(platz + 1, 0.0);
                   i.obertoene[platz] = w / 100.0;
               },
               [anteil](const Schicht& i) { return QStringLiteral("%1 %").arg(gerundet(anteil(i) * 100.0)); });
        m_nurSinus.push_back(m_regler.size() - 1);
    }
    regler(gitter, tr("Damping"), "creatorDamping", 0, 100,
           [](const Schicht& i) {
               return gerundet((i.saiteDaempfung - kDaempfungVon) / (kDaempfungBis - kDaempfungVon) * 100.0);
           },
           [](Schicht& i, int w) { i.saiteDaempfung = kDaempfungVon + (kDaempfungBis - kDaempfungVon) * w / 100.0; },
           [](const Schicht& i) { return QString::number(i.saiteDaempfung, 'f', 4); });
    m_daempfung = m_regler.size() - 1;

    // --- Vibrato: gehoert zu Source und steht deshalb vor den Bausteinen
    m_vibratoTitel = titel(tr("Vibrato"), "vibrato");
    regler(gitter, tr("Rate"), "creatorVibratoRate", 0, 120, [](const Schicht& i) { return gerundet(i.vibratoHz * 10.0); },
           [](Schicht& i, int w) { i.vibratoHz = w / 10.0; },
           [](const Schicht& i) { return QStringLiteral("%1 Hz").arg(i.vibratoHz, 0, 'f', 1); });
    m_nichtSaite.push_back(m_regler.size() - 1);
    m_nichtSample.push_back(m_regler.size() - 1);
    regler(gitter, tr("Depth"), "creatorVibratoDepth", 0, 50, [](const Schicht& i) { return gerundet(i.vibratoCent); },
           [](Schicht& i, int w) { i.vibratoCent = w; },
           [](const Schicht& i) { return QStringLiteral("%1 ct").arg(gerundet(i.vibratoCent)); });
    m_nichtSaite.push_back(m_regler.size() - 1);
    m_nichtSample.push_back(m_regler.size() - 1);
    regler(gitter, tr("Delay"), "creatorVibratoDelay", 0, 100,
           [](const Schicht& i) { return gerundet(i.vibratoEinsatzMs / 10.0); },
           [](Schicht& i, int w) { i.vibratoEinsatzMs = w * 10.0; },
           [ms](const Schicht& i) { return ms(i.vibratoEinsatzMs); });
    m_nichtSaite.push_back(m_regler.size() - 1);
    m_nichtSample.push_back(m_regler.size() - 1);
    m_gruppen.back().ende = gitter->rowCount();
    m_zeilenSource.bis = gitter->rowCount();

    // --- Envelope: seit Stufe 16c ein Baustein der Zeile, ohne eigene Ueberschrift
    m_zeilenHuelle.von = gitter->rowCount();
    regler(gitter, tr("Attack"), "creatorAttack", 0, 1000, [](const Schicht& i) { return gerundet(i.anstiegMs); },
           [](Schicht& i, int w) { i.anstiegMs = w; }, [ms](const Schicht& i) { return ms(i.anstiegMs); });
    regler(gitter, tr("Decay"), "creatorDecay", 0, 500, [](const Schicht& i) { return gerundet(i.abfallMs / 10.0); },
           [](Schicht& i, int w) { i.abfallMs = w * 10.0; }, [ms](const Schicht& i) { return ms(i.abfallMs); });
    m_nichtSaite.push_back(m_regler.size() - 1);
    regler(gitter, tr("Sustain"), "creatorSustain", 0, 100, [](const Schicht& i) { return gerundet(i.halten * 100.0); },
           [](Schicht& i, int w) { i.halten = w / 100.0; },
           [](const Schicht& i) { return QStringLiteral("%1 %").arg(gerundet(i.halten * 100.0)); });
    m_nichtSaite.push_back(m_regler.size() - 1);
    regler(gitter, tr("Release"), "creatorRelease", 0, 300, [](const Schicht& i) { return gerundet(i.ausklangMs / 10.0); },
           [](Schicht& i, int w) { i.ausklangMs = w * 10.0; }, [ms](const Schicht& i) { return ms(i.ausklangMs); });

    // --- Filter: der Baustein Filter — die Regler stellen den, der in der Zeile gewaehlt ist
    m_zeilenHuelle.bis = m_zeilenFilter.von = gitter->rowCount();
    const auto filterWert = [this](const Schicht& s) {
        const SchichtFilter* filter = gewaehlterFilter(s);
        return filter != nullptr ? *filter : SchichtFilter{};
    };
    const auto driveWert = [this](const Schicht& s) {
        const SchichtDrive* drive = gewaehlterDrive(s);
        return drive != nullptr ? drive->wert : 0.0;
    };
    // --- Type (Stufe 11e; Skizze gebilligt, Patrik S116): Low-pass ist der Filter von frueher
    {
        m_filterArtName = new QLabel(tr("Type"), this);
        m_filterArt = new QComboBox(this);
        m_filterArt->setObjectName(QStringLiteral("creatorFilterType"));
        m_filterArt->addItem(tr("Low-pass"), static_cast<int>(FilterArt::Tiefpass));
        m_filterArt->addItem(tr("High-pass"), static_cast<int>(FilterArt::Hochpass));
        m_filterArt->addItem(tr("Band-pass"), static_cast<int>(FilterArt::Bandpass));
        m_filterArt->setToolTip(tr("Low-pass keeps what lies below Cutoff, High-pass what lies above it, Band-pass what "
                                   "lies around it. With Cutoff at \"open\" there is no filter"));
        const int zeile = gitter->rowCount();
        gitter->addWidget(m_filterArtName, zeile, 0);
        gitter->addWidget(m_filterArt, zeile, 1, 1, 2);
        connect(m_filterArt, &QComboBox::activated, this, [this](int index) {
            const auto art = static_cast<FilterArt>(m_filterArt->itemData(index).toInt());
            aendereSchicht([this, art](Schicht& s) {
                if (SchichtFilter* filter = gewaehlterFilter(s)) filter->art = art;
            });
        });
    }
    regler(gitter, tr("Cutoff"), "creatorCutoff", 0, 320, [filterWert](const Schicht& i) { return gerundet(filterWert(i).teilton * 10.0); },
           [this](Schicht& i, int w) {
               if (SchichtFilter* filter = gewaehlterFilter(i)) filter->teilton = w / 10.0;
           },
           [filterWert](const Schicht& i) {
               return filterWert(i).teilton <= 0.0 ? tr("open") : QStringLiteral("× %1").arg(filterWert(i).teilton, 0, 'f', 1);
           });
    m_nichtSaite.push_back(m_regler.size() - 1);
    // Resonance (Entscheid H1 b): an allen drei Arten
    regler(gitter, tr("Resonance"), "creatorResonance", 0, 100, [filterWert](const Schicht& i) { return gerundet(filterWert(i).resonanz * 100.0); },
           [this](Schicht& i, int w) {
               if (SchichtFilter* filter = gewaehlterFilter(i)) filter->resonanz = w / 100.0;
           },
           [filterWert](const Schicht& i) { return QStringLiteral("%1 %").arg(gerundet(filterWert(i).resonanz * 100.0)); })
        .schieber->setToolTip(tr("Emphasis at Cutoff: further right the filter rings at its edge. With Band-pass it sets "
                                 "the width — further right the band gets narrower"));
    m_reglerResonanz = m_regler.size() - 1;
    m_nichtSaite.push_back(m_reglerResonanz);
    regler(gitter, tr("Filter envelope"), "creatorFilterEnvelope", 0, 320,
           [filterWert](const Schicht& i) { return gerundet(filterWert(i).huelle * 10.0); },
           [this](Schicht& i, int w) {
               if (SchichtFilter* filter = gewaehlterFilter(i)) filter->huelle = w / 10.0;
           },
           [filterWert](const Schicht& i) { return QStringLiteral("+ %1").arg(filterWert(i).huelle, 0, 'f', 1); })
        .schieber->setToolTip(tr("The envelope of the layer opens the filter by this much on top of Cutoff"));
    m_nichtSaite.push_back(m_regler.size() - 1);

    // --- Drive: der Baustein Drive
    m_zeilenFilter.bis = m_zeilenDrive.von = gitter->rowCount();
    regler(gitter, tr("Drive"), "creatorDrive", 0, 100, [driveWert](const Schicht& i) { return gerundet(driveWert(i) * 100.0); },
           [this](Schicht& i, int w) {
               if (SchichtDrive* drive = gewaehlterDrive(i)) drive->wert = w / 100.0;
           },
           [driveWert](const Schicht& i) {
               return driveWert(i) <= 0.0 ? tr("off") : QStringLiteral("%1 %").arg(gerundet(driveWert(i) * 100.0));
           })
        .schieber->setToolTip(tr("Distortion at this place of the path"));
    m_zeilenDrive.bis = gitter->rowCount();
    gitter->setRowStretch(gitter->rowCount(), 1);
    // Die Regler der Schicht stehen in einem Feld, das rollt (Befund Patrik S116: mit jeder
    // aufgeklappten Gruppe wuchs das Fenster, bis ueber die Hoehe des Bildschirms). So bestimmt
    // die Spalte die Hoehe des Fensters nicht mehr; was nicht hineinpasst, erreicht der Rollbalken.
    {
        auto* inhalt = new QWidget;
        inhalt->setObjectName(QStringLiteral("creatorLayerControls"));
        gitter->setContentsMargins(0, 0, 6, 0);  // etwas Luft vor dem Rollbalken
        inhalt->setLayout(gitter);
        m_rollen = new QScrollArea(this);
        m_rollen->setObjectName(QStringLiteral("creatorLayerScroll"));
        m_rollen->setFrameShape(QFrame::NoFrame);
        m_rollen->setWidgetResizable(true);
        m_rollen->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_rollen->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        m_rollen->setWidget(inhalt);
        m_rollen->setMinimumWidth(380);  // Name, Regler und Wert einer Zeile
        // wie das Fenster, nicht wie ein Eingabefeld
        m_rollen->viewport()->setAutoFillBackground(false);
        inhalt->setAutoFillBackground(false);
        mitte->addWidget(m_rollen, 1);
        koerper->addLayout(mitte, 5);
        // das Mausrad rollt das Feld — es verstellt keinen Regler und keine Auswahl, ueber die
        // der Zeiger gerade faehrt (`eventFilter`)
        for (QSlider* schieber : inhalt->findChildren<QSlider*>()) schieber->installEventFilter(this);
        for (QComboBox* auswahl : inhalt->findChildren<QComboBox*>()) auswahl->installEventFilter(this);
    }

    // --- rechts: die Bilder und die Klaviatur
    auto* rechts = new QVBoxLayout;
    m_wellenTitel = new QLabel(tr("One cycle"), this);
    m_wellenTitel->setObjectName(QStringLiteral("creatorCycleTitle"));
    rechts->addWidget(m_wellenTitel);
    m_wellenBild = new KurvenBild(this);
    m_wellenBild->setObjectName(QStringLiteral("creatorCycle"));
    rechts->addWidget(m_wellenBild, 1);
    // nur bei einer Sample-Schicht: sonst findet niemand, dass sich die Marken ziehen lassen
    m_bildHinweis = new QLabel(tr("Red: sample start · blue: sustain loop — drag them"), this);
    m_bildHinweis->setObjectName(QStringLiteral("creatorSampleHint"));
    m_bildHinweis->setEnabled(false);
    rechts->addWidget(m_bildHinweis);
    // nur bei einer Shape-Schicht (Stufe 19e): die Zeichnung, die Form, wie sie spielt, und Angle
    m_formBildTitel = new QLabel(tr("Shape, as it plays"), this);
    m_formBildTitel->setObjectName(QStringLiteral("creatorShapeTitle"));
    rechts->addWidget(m_formBildTitel);
    m_formBild = new FormBild(this);
    m_formBild->setObjectName(QStringLiteral("creatorShapePicture"));
    rechts->addWidget(m_formBild, 2);
    m_formLegende = new QLabel(this);
    m_formLegende->setObjectName(QStringLiteral("creatorShapeLegend"));
    m_formLegende->setEnabled(false);
    rechts->addWidget(m_formLegende);
    connect(m_wellenBild, &KurvenBild::griffGezogen, this, [this](int welcher, double stelle) {
        const double laenge = sampleLaengeMs();
        if (laenge <= 0.0 || schicht().schwingung != Schwingung::Sample) return;
        const double ort = std::round(stelle * laenge);
        aendereSchicht([welcher, ort](Schicht& s) {
            if (welcher == KurvenBild::GriffStart) s.sampleStartMs = ort;
            if (welcher == KurvenBild::GriffVon) s.schleifeVonMs = std::min(ort, std::max(0.0, s.schleifeBisMs - 1.0));
            if (welcher == KurvenBild::GriffBis) s.schleifeBisMs = std::max(ort, s.schleifeVonMs + 1.0);
        });
    });
    rechts->addWidget(new QLabel(tr("Envelope of a note"), this));
    m_huellBild = new KurvenBild(this);
    m_huellBild->setObjectName(QStringLiteral("creatorEnvelope"));
    rechts->addWidget(m_huellBild, 1);
    m_legende = new QLabel(tr("Solid: all layers · dashed: selected layer"), this);
    m_legende->setObjectName(QStringLiteral("creatorLegend"));
    m_legende->setEnabled(false);
    rechts->addWidget(m_legende);
    rechts->addWidget(new QLabel(tr("Click a key to hear it"), this));
    m_tasten = new Tastenleiste(this);
    rechts->addWidget(m_tasten);
    // Repeat note (Wunsch Patrik S116): der angeklickte Ton spielt immer wieder
    m_wiederholen = new QCheckBox(tr("Repeat note"), this);
    m_wiederholen->setObjectName(QStringLiteral("creatorRepeat"));
    m_wiederholen->setToolTip(tr("Plays the note again and again while you turn the controls. Click a key to choose "
                                 "the note — it stays highlighted while this is on"));
    // every n beats: so weit liegen die Toene auseinander — und so lang darf einer klingen
    m_wiederholAbstand = new QComboBox(this);
    m_wiederholAbstand->setObjectName(QStringLiteral("creatorRepeatEvery"));
    for (const int schlaege : {1, 2, 4, 8, 16})
        m_wiederholAbstand->addItem(schlaege == 1 ? tr("every beat") : tr("every %1 beats").arg(schlaege), schlaege);
    m_wiederholAbstand->setCurrentIndex(m_wiederholAbstand->findData(m_wiederholSchlaege));
    m_wiederholAbstand->setToolTip(tr("How far apart the repeated notes are, in beats of the loop. Each note may ring "
                                      "until the next one — choose more beats to hear a long release to its end"));
    {
        auto* reihe = new QHBoxLayout;
        reihe->addWidget(m_wiederholen);
        reihe->addWidget(m_wiederholAbstand);
        reihe->addStretch(1);
        rechts->addLayout(reihe);
    }
    connect(m_wiederholAbstand, &QComboBox::activated, this,
            [this](int index) { setzeWiederholSchlaege(m_wiederholAbstand->itemData(index).toInt()); });
    m_wiederholTakt = new QTimer(this);
    connect(m_wiederholTakt, &QTimer::timeout, this, [this] { (void)spieleAn(m_wiederholHoehe); });
    connect(m_wiederholen, &QCheckBox::toggled, this, &InstrumentCreator::setzeWiederholen);
    m_nurWahl = new QCheckBox(tr("Selected layer only"), this);
    m_nurWahl->setObjectName(QStringLiteral("creatorSelectedOnly"));
    m_nurWahl->setToolTip(tr("The keys play the selected layer alone, even if it is muted"));
    rechts->addWidget(m_nurWahl);
    // nur bei einer Sample-Schicht (Schritt 3): zum Stimmen nach Gehoer
    m_referenz = new QCheckBox(tr("Reference tone"), this);
    m_referenz->setObjectName(QStringLiteral("creatorReferenceTone"));
    m_referenz->setToolTip(tr("The keys play the selected sample layer alone, and a sine of the same note next to it — "
                              "tune Root note and Fine tune until both sound as one"));
    rechts->addWidget(m_referenz);
    rechts->addStretch(1);
    koerper->addLayout(rechts, 4);
    aussen->addLayout(koerper, 1);

    connect(m_titel, &QLineEdit::editingFinished, this, [this] {
        const QString titel = m_titel->text().trimmed();
        if (!titel.isEmpty() && titel != m_instrument.titel) aendere([&titel](Instrument& i) { i.titel = titel; });
    });
    connect(m_art, &QComboBox::activated, this, [this](int index) {
        const QString art = m_art->itemData(index).toString();
        if (art != gueltigeArt(m_instrument.art)) aendere([&art](Instrument& i) { i.art = art; });
    });
    connect(m_genres, &QLineEdit::editingFinished, this, [this] {
        const QStringList genres = genresAus(m_genres->text());
        if (genres != m_instrument.genres) aendere([&genres](Instrument& i) { i.genres = genres; });
    });
    connect(m_beschreibung, &QLineEdit::editingFinished, this, [this] {
        const QString text = m_beschreibung->text().trimmed();
        if (text != m_instrument.beschreibung) aendere([&text](Instrument& i) { i.beschreibung = text; });
    });
    connect(m_zurueck, &QPushButton::clicked, this, &InstrumentCreator::setzeZurueck);
    connect(m_speichern, &QPushButton::clicked, this, [this] {
        // ein mitgeliefertes oder neues: eine eigene Kopie unter einem Namen
        if (m_datei.isEmpty())
        {
            (void)frageNachNamen();
            return;
        }
        if (!speichere())
            QMessageBox::warning(this, tr("Instrument"), tr("Could not write %1").arg(QDir::toNativeSeparators(m_datei)));
    });
    // Save as...: immer eine Kopie unter neuem Namen in den Benutzerordner — kein Dateidialog (Entscheid E1 a, S117)
    connect(m_speichernUnter, &QPushButton::clicked, this, [this] { (void)frageNachNamen(); });
    connect(m_tasten, &Tastenleiste::taste, this, [this](int hoehe) {
        m_wiederholHoehe = hoehe;  // auch ohne Repeat: mit ihm beginnt es spaeter
        (void)spieleAn(hoehe);
        if (!wiederholt()) return;
        // mit Repeat: die Taste ist die neue Tonhoehe, und der Takt beginnt mit ihr von vorn
        m_tasten->setGewaehlt(hoehe);
        m_wiederholTakt->start(wiederholMs());
    });
    connect(m_gesamt, &QSlider::valueChanged, this, [this](int wert) {
        aendere([wert](Instrument& i) { i.laut = wert / 100.0; });
    });
    connect(m_schichtName, &QLineEdit::editingFinished, this, [this] {
        const QString name = m_schichtName->text().trimmed();
        if (name != schicht().name) aendereSchicht([&name](Schicht& s) { s.name = name; });
    });
    connect(m_liste, &QListWidget::currentRowChanged, this, [this](int zeile) { waehle(zeile); });
    connect(m_liste, &QListWidget::itemChanged, this, [this](QListWidgetItem* eintrag) {
        // der Haken: die Schicht an oder stumm
        const int zeile = m_liste->row(eintrag);
        const bool an = eintrag->checkState() == Qt::Checked;
        aendere([zeile, an](Instrument& i) {
            if (zeile >= 0 && zeile < static_cast<int>(i.schichten.size())) i.schichten[static_cast<std::size_t>(zeile)].an = an;
        });
    });
    connect(m_neu, &QPushButton::clicked, this, [this] { (void)neueSchicht(); });
    connect(m_neuAus, &QPushButton::clicked, this, &InstrumentCreator::zeigeQuellen);
    connect(m_doppelt, &QPushButton::clicked, this, [this] { (void)verdopple(); });
    connect(m_weg, &QPushButton::clicked, this, [this] { (void)entferne(); });
    connect(m_hinauf, &QPushButton::clicked, this, [this] { (void)schiebe(-1); });
    connect(m_hinunter, &QPushButton::clicked, this, [this] { (void)schiebe(1); });
    connect(m_nurWahl, &QCheckBox::toggled, this, [this](bool an) { m_nurGewaehlte = an; });
    connect(m_referenz, &QCheckBox::toggled, this, [this](bool an) { m_referenzton = an; });
    resize(1000, 680);
}

// =============================================================================
// Die Schichten
// =============================================================================

QString InstrumentCreator::schichtTitel(const Schicht& schicht, int platz)
{
    return schicht.name.isEmpty() ? tr("Layer %1").arg(platz + 1) : QStringLiteral("%1 %2").arg(platz + 1).arg(schicht.name);
}

void InstrumentCreator::waehle(int platz)
{
    if (platz < 0 || platz >= static_cast<int>(m_instrument.schichten.size()) || platz == m_gewaehlt) return;
    m_gewaehlt = platz;
    zeigeAlles();
}

void InstrumentCreator::aendereSchicht(const std::function<void(Schicht&)>& tu)
{
    const auto platz = static_cast<std::size_t>(m_gewaehlt);
    aendere([&tu, platz](Instrument& i) { tu(i.schichten[platz]); });
}

bool InstrumentCreator::neueSchicht()
{
    if (m_instrument.schichten.size() >= static_cast<std::size_t>(kSchichtenMax)) return false;
    m_gewaehlt = static_cast<int>(m_instrument.schichten.size());
    aendere([](Instrument& i) { i.schichten.push_back(Schicht{}); });
    return true;
}

int InstrumentCreator::schichtenAus(const Instrument& quelle)
{
    const std::size_t vorher = m_instrument.schichten.size();
    const std::size_t frei = static_cast<std::size_t>(kSchichtenMax) - std::min(vorher, static_cast<std::size_t>(kSchichtenMax));
    const std::size_t zahl = std::min(frei, quelle.schichten.size());
    if (zahl == 0) return 0;
    m_gewaehlt = static_cast<int>(vorher);
    aendere([&quelle, zahl](Instrument& i) {
        for (std::size_t s = 0; s < zahl; ++s)
        {
            Schicht neu = quelle.schichten[s];
            if (neu.name.isEmpty()) neu.name = quelle.titel;
            // die Lautstaerke des ganzen Instruments wandert in die Schicht
            neu.laut = std::clamp(neu.laut * quelle.laut, 0.0, 1.0);
            i.schichten.push_back(std::move(neu));
        }
    });
    return static_cast<int>(zahl);
}

bool InstrumentCreator::verdopple()
{
    if (m_instrument.schichten.size() >= static_cast<std::size_t>(kSchichtenMax)) return false;
    const int platz = m_gewaehlt;
    m_gewaehlt = platz + 1;
    aendere([platz](Instrument& i) { i.schichten.insert(i.schichten.begin() + platz + 1, i.schichten[static_cast<std::size_t>(platz)]); });
    return true;
}

bool InstrumentCreator::entferne()
{
    if (m_instrument.schichten.size() <= 1) return false;
    const int platz = m_gewaehlt;
    m_gewaehlt = std::min(platz, static_cast<int>(m_instrument.schichten.size()) - 2);
    aendere([platz](Instrument& i) { i.schichten.erase(i.schichten.begin() + platz); });
    return true;
}

bool InstrumentCreator::schiebe(int richtung)
{
    const int platz = m_gewaehlt;
    const int ziel = platz + (richtung < 0 ? -1 : 1);
    if (ziel < 0 || ziel >= static_cast<int>(m_instrument.schichten.size())) return false;
    m_gewaehlt = ziel;
    aendere([platz, ziel](Instrument& i) {
        std::swap(i.schichten[static_cast<std::size_t>(platz)], i.schichten[static_cast<std::size_t>(ziel)]);
    });
    if (m_gewaehlt == ziel) zeigeAlles();  // zwei gleiche Schichten getauscht: nur die Wahl wandert
    return true;
}

void InstrumentCreator::setNurGewaehlte(bool an)
{
    m_nurWahl->setChecked(an);
    m_nurGewaehlte = an;
}

void InstrumentCreator::zeigeQuellen()
{
    QMenu menue(this);
    for (const Instrument& quelle : instrumente())
        connect(menue.addAction(quelle.titel), &QAction::triggered, this, [this, name = quelle.name] {
            if (const Instrument* gefunden = findeInstrument(name)) (void)schichtenAus(*gefunden);
        });
    menue.addSeparator();
    connect(menue.addAction(tr("From file...")), &QAction::triggered, this, [this] {
        QSettings einstellungen;
        const QString pfad = QFileDialog::getOpenFileName(this, tr("Add layer from instrument"),
                                                          einstellungen.value(QLatin1String(kLetzterOrdner)).toString(),
                                                          tr("Instrument (*.lvinst)"));
        if (pfad.isEmpty()) return;
        Instrument quelle;
        if (!ladeInstrument(pfad, quelle))
        {
            QMessageBox::warning(this, tr("Instrument"), tr("Could not read %1").arg(QDir::toNativeSeparators(pfad)));
            return;
        }
        einstellungen.setValue(QLatin1String(kLetzterOrdner), QFileInfo(pfad).absolutePath());
        (void)schichtenAus(quelle);
    });
    menue.exec(m_neuAus->mapToGlobal(QPoint(0, m_neuAus->height())));
}

void InstrumentCreator::oeffne(const Instrument& instrument)
{
    m_instrument = instrument;
    if (m_instrument.schichten.empty()) m_instrument.schichten.push_back(Schicht{});
    m_vorlage = m_instrument;
    m_gewaehlt = 0;
    m_stelle = kStelleSource;
    m_stelleFuer = 0;
    m_eigenerName.clear();
    // die Vorschlaege fuer Genres: auch was seit dem letzten Oeffnen dazukam
    static_cast<GenreVorschlag*>(m_genres->completer())->liesNeu();
    if (istEigenesInstrument(instrument.name))
    {
        // ein eigenes aus dem Benutzerordner: Save schreibt an seinen Platz
        m_eigenerName = instrument.name.mid(static_cast<int>(qstrlen(kEigenVorsatz)));
        m_datei = eigeneDatei(m_eigenerName);
        m_herkunft = tr("My files: %1").arg(QFileInfo(m_datei).fileName());
    }
    else if (istInstrumentDatei(instrument.name))
    {
        m_datei = instrument.name;
        m_herkunft = QFileInfo(m_datei).fileName();
    }
    else
    {
        // ein mitgeliefertes: es bleibt, wie es ist — gespeichert wird eine eigene Kopie
        m_datei.clear();
        m_herkunft = tr("based on %1").arg(instrument.titel);
    }
    zeigeAlles();
}

QString InstrumentCreator::nennung() const
{
    if (!m_eigenerName.isEmpty()) return QLatin1String(kEigenVorsatz) + m_eigenerName;
    return m_datei;
}

QStringList InstrumentCreator::genresAus(const QString& text)
{
    QStringList genres;
    for (const QString& stueck : text.split(QLatin1Char(',')))
    {
        const QString genre = stueck.trimmed();
        if (!genre.isEmpty() && !genres.contains(genre, Qt::CaseInsensitive)) genres << genre;
    }
    return genres;
}

QString InstrumentCreator::vorschlagTitel() const
{
    const QString titel = m_instrument.titel.trimmed();
    // schon umbenannt — oder ein neues ohne Vorlage (aus „Edit sound..." im Drum-Editor) — und
    // der Name ist frei: so heisst es
    if ((titel != m_vorlage.titel || m_vorlage.name.isEmpty()) && !QFileInfo::exists(eigeneDatei(eigenerName(titel))))
        return titel;
    for (int zahl = 2;; ++zahl)
    {
        const QString versuch = QStringLiteral("%1 %2").arg(titel).arg(zahl);
        if (!QFileInfo::exists(eigeneDatei(eigenerName(versuch)))) return versuch;
    }
}

bool InstrumentCreator::geaendert() const
{
    return !(m_instrument == m_vorlage);
}

void InstrumentCreator::aendere(const std::function<void(Instrument&)>& tu)
{
    Instrument neu = m_instrument;
    tu(neu);
    if (neu.schichten.empty()) neu.schichten.push_back(Schicht{});
    if (neu == m_instrument) return;
    m_instrument = std::move(neu);
    zeigeAlles();
}

void InstrumentCreator::setzeZurueck()
{
    if (!geaendert()) return;
    m_instrument = m_vorlage;
    m_gewaehlt = std::min(m_gewaehlt, static_cast<int>(m_instrument.schichten.size()) - 1);
    zeigeAlles();
}

bool InstrumentCreator::speichere()
{
    if (m_datei.isEmpty()) return false;
    // ein eigenes bleibt ein eigenes: es heisst weiter `user:<name>`, nicht wie sein Pfad
    if (!m_eigenerName.isEmpty())
    {
        if (!speichereEigenes(m_instrument, m_eigenerName)) return false;
        m_vorlage = m_instrument;
        zeigeAlles();
        Q_EMIT gespeichert(nennung());
        return true;
    }
    return speichereUnter(m_datei);
}

bool InstrumentCreator::speichereUnter(const QString& pfad)
{
    if (pfad.isEmpty() || !speichereInstrument(m_instrument, pfad)) return false;
    m_datei = pfad;
    m_eigenerName.clear();
    m_instrument.name = pfad;  // es heisst jetzt wie seine Datei
    m_vorlage = m_instrument;
    m_herkunft = QFileInfo(pfad).fileName();
    zeigeAlles();
    Q_EMIT gespeichert(pfad);
    return true;
}

bool InstrumentCreator::speichereAlsEigenes(const QString& titel)
{
    const QString sauber = titel.trimmed();
    const QString name = eigenerName(sauber);
    if (name.isEmpty()) return false;
    Instrument kopie = m_instrument;
    kopie.titel = sauber;
    if (!speichereEigenes(kopie, name)) return false;
    m_instrument = std::move(kopie);  // heisst jetzt `user:<name>`
    m_eigenerName = name;
    m_datei = eigeneDatei(name);
    m_vorlage = m_instrument;
    m_herkunft = tr("My files: %1").arg(QFileInfo(m_datei).fileName());
    zeigeAlles();
    Q_EMIT gespeichert(nennung());
    return true;
}

QString InstrumentCreator::kopieSatz() const
{
    // ein neues ohne Vorlage (aus „Edit sound..." im Drum-Editor)
    if (m_vorlage.name.isEmpty()) return tr("Your instrument is saved to My files and used by this pattern.");
    // schon eine eigene Datei — im Benutzerordner oder an freiem Ort: Save as... legt eine Kopie daneben
    if (!m_datei.isEmpty())
        return tr("A copy is saved to My files under this name and used by this pattern; %1 stays as it is.")
            .arg(QFileInfo(m_datei).fileName());
    // ein mitgeliefertes: nur der Hinweis — ersetzen laesst es sich nicht (Entscheid E1b c, S117)
    return tr("%1 is an original LumiViz instrument. Originals are not overwritten: your copy is saved to "
              "My files and used by this pattern. Keep the suggested name or type your own.")
        .arg(m_vorlage.titel);
}

bool InstrumentCreator::frageNachNamen()
{
    QDialog dialog(this);
    dialog.setObjectName(QStringLiteral("creatorSaveDialog"));
    dialog.setWindowTitle(tr("Save instrument"));
    auto* aussen = new QVBoxLayout(&dialog);
    auto* satz = new QLabel(kopieSatz(), &dialog);
    satz->setWordWrap(true);
    satz->setEnabled(false);
    aussen->addWidget(satz);
    auto* name = new QLineEdit(vorschlagTitel(), &dialog);
    name->setObjectName(QStringLiteral("creatorSaveName"));
    auto* zeile = new QHBoxLayout;
    zeile->addWidget(new QLabel(tr("Name"), &dialog));
    zeile->addWidget(name, 1);
    aussen->addLayout(zeile);
    auto* datei = new QLabel(&dialog);
    datei->setObjectName(QStringLiteral("creatorSaveFile"));
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
        const QString kurz = eigenerName(name->text());
        datei->setText(kurz.isEmpty() ? tr("Type a name with letters or digits")
                                      : tr("File: %1.lvinst%2")
                                            .arg(kurz, QFileInfo::exists(eigeneDatei(kurz)) ? tr(" — replaces the one in My files")
                                                                                         : QString()));
        sichern->setEnabled(!kurz.isEmpty());
    };
    connect(name, &QLineEdit::textChanged, &dialog, zeige);
    connect(abbrechen, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(sichern, &QPushButton::clicked, &dialog, &QDialog::accept);
    zeige();
    dialog.resize(420, dialog.sizeHint().height());
    if (dialog.exec() != QDialog::Accepted) return false;

    const QString titel = name->text().trimmed();
    if (QFileInfo::exists(eigeneDatei(eigenerName(titel))) &&
        QMessageBox::question(this, tr("Save instrument"),
                              tr("My files already holds %1.lvinst.\nReplace it?").arg(eigenerName(titel))) !=
            QMessageBox::Yes)
        return false;
    if (!speichereAlsEigenes(titel))
    {
        QMessageBox::warning(this, tr("Instrument"),
                             tr("Could not write %1").arg(QDir::toNativeSeparators(eigeneDatei(eigenerName(titel)))));
        return false;
    }
    return true;
}


bool InstrumentCreator::spieleAn(int hoehe)
{
    // die Kopie, wie sie gerade steht — auch ungespeichert — in die Vorhoer-Datei
    const QString datei = vorhoerDatei();
    Instrument zuHoeren = m_instrument;
    // Reference tone gilt nur fuer eine Sample-Schicht: sie allein, dazu ein Sinus derselben Note
    const bool referenz = m_referenzton && schicht().schwingung == Schwingung::Sample;
    if (m_nurGewaehlte || referenz)
    {
        // die gewaehlte Schicht allein — auch wenn sie in der Liste stumm ist
        zuHoeren.schichten = {schicht()};
        zuHoeren.schichten[0].an = true;
    }
    if (referenz)
    {
        Schicht ton;
        ton.name = QStringLiteral("Reference tone");
        ton.versatzHalbtoene = schicht().versatzHalbtoene;  // die Note, die die Schicht spielen soll
        ton.halten = 1.0;
        ton.laut = 0.5;
        zuHoeren.schichten.push_back(ton);
    }
    if (!speichereInstrument(zuHoeren, datei)) return false;
    // mit Repeat note darf der Ton bis zum naechsten klingen
    if (wiederholt())
        Q_EMIT tonLangVerlangt(datei, hoehe, wiederholMs());
    else
        Q_EMIT tonVerlangt(datei, hoehe);
    return true;
}

void InstrumentCreator::closeEvent(QCloseEvent* event)
{
    if (geaendert())
    {
        const QMessageBox::StandardButton wahl = QMessageBox::question(
            this, tr("Instrument"), tr("The instrument '%1' has unsaved changes.").arg(m_instrument.titel),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
        if (wahl == QMessageBox::Cancel)
        {
            event->ignore();
            return;
        }
        if (wahl == QMessageBox::Save)
        {
            if (m_datei.isEmpty())
                (void)frageNachNamen();  // eine eigene Kopie unter einem Namen (Stufe 14b)
            else
                (void)speichere();
            if (geaendert())  // abgebrochen oder nicht geschrieben: das Fenster bleibt
            {
                event->ignore();
                return;
            }
        }
        else
        {
            m_instrument = m_vorlage;
            m_gewaehlt = std::min(m_gewaehlt, static_cast<int>(m_instrument.schichten.size()) - 1);
        }
    }
    QWidget::closeEvent(event);
}

void InstrumentCreator::zeigeListe()
{
    // die Zeilen an Ort und Stelle nachziehen: die Liste meldet gerade
    // vielleicht selbst einen Haken, ihre Eintraege muessen bleiben
    const QSignalBlocker ruhe(m_liste);
    const int zahl = static_cast<int>(m_instrument.schichten.size());
    while (m_liste->count() > zahl) delete m_liste->takeItem(m_liste->count() - 1);
    while (m_liste->count() < zahl)
    {
        auto* eintrag = new QListWidgetItem(m_liste);
        eintrag->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
    }
    for (int platz = 0; platz < zahl; ++platz)
    {
        const Schicht& s = m_instrument.schichten[static_cast<std::size_t>(platz)];
        QListWidgetItem* eintrag = m_liste->item(platz);
        eintrag->setText(tr("%1 · %2\nLevel %3 % · %4%5 st")
                             .arg(schichtTitel(s, platz), m_welle->itemText(m_welle->findData(static_cast<int>(s.schwingung))))
                             .arg(gerundet(s.laut * 100.0))
                             .arg(s.versatzHalbtoene > 0 ? QStringLiteral("+") : QString())
                             .arg(s.versatzHalbtoene));
        eintrag->setCheckState(s.an ? Qt::Checked : Qt::Unchecked);
        // die Farbe der Schicht (F1 a): ihre eigene oder die abgeleitete
        const QColor farbe = schichtFarbe(m_instrument, platz);
        QPixmap punkt(12, 12);
        punkt.fill(Qt::transparent);
        {
            QPainter p(&punkt);
            p.setRenderHint(QPainter::Antialiasing);
            p.setPen(Qt::NoPen);
            p.setBrush(farbe);
            p.drawEllipse(QRectF(1.0, 1.0, 10.0, 10.0));
        }
        eintrag->setIcon(QIcon(punkt));
        eintrag->setData(Qt::UserRole, farbe.name(QColor::HexRgb));
    }
    m_liste->setCurrentRow(m_gewaehlt);
    const bool platzFrei = zahl < kSchichtenMax;
    m_neu->setEnabled(platzFrei);
    m_neuAus->setEnabled(platzFrei);
    m_doppelt->setEnabled(platzFrei);
    m_weg->setEnabled(zahl > 1);
    m_hinauf->setEnabled(m_gewaehlt > 0);
    m_hinunter->setEnabled(m_gewaehlt < zahl - 1);
}

Instrument InstrumentCreator::fuerBild(bool nurGewaehlte) const
{
    Instrument bild = m_instrument;
    if (nurGewaehlte)
    {
        bild.schichten = {schicht()};
        bild.schichten[0].an = true;
    }
    for (Schicht& s : bild.schichten)
    {
        s.stimmen = 1;
        s.vibratoCent = 0.0;
        s.tonhoeheStartHt = 0.0;  // ohne Pitch sweep: das Bild zeigt den Ton, auf dem die Note ankommt
    }
    return bild;
}

void InstrumentCreator::zeigeAlles()
{
    setWindowTitle(tr("Instrument: %1").arg(m_instrument.titel) + (geaendert() ? QStringLiteral(" *") : QString()));
    if (!m_titel->hasFocus() || m_titel->text().trimmed().isEmpty()) m_titel->setText(m_instrument.titel);
    m_herkunftZeile->setText(m_herkunft);
    m_herkunftZeile->setToolTip(m_datei.isEmpty() ? QString() : QDir::toNativeSeparators(m_datei));
    m_zurueck->setEnabled(geaendert());
    // Save ist waehlbar, sobald etwas geaendert ist (Stufe 14b): bei einem
    // mitgelieferten fragt es nach dem Namen der eigenen Kopie
    m_speichern->setEnabled(geaendert());
    m_speichern->setToolTip(m_datei.isEmpty()
                                ? tr("Save your copy to My files under a name — the built-in instrument stays as it is")
                                : QDir::toNativeSeparators(m_datei));
    {
        const QSignalBlocker ruhe(m_art);
        m_art->setCurrentIndex(m_art->findData(gueltigeArt(m_instrument.art)));
    }
    if (!m_genres->hasFocus()) m_genres->setText(m_instrument.genres.join(QStringLiteral(", ")));
    if (!m_beschreibung->hasFocus()) m_beschreibung->setText(m_instrument.beschreibung);
    m_gewaehlt = std::clamp(m_gewaehlt, 0, static_cast<int>(m_instrument.schichten.size()) - 1);
    const Schicht& gewaehlte = schicht();
    // eine andere Schicht beginnt bei Source; einen Baustein, den es nicht mehr gibt, auch
    if (m_gewaehlt != m_stelleFuer || m_stelle >= static_cast<int>(gewaehlte.reihe.size()) ||
        (m_stelle < 0 && m_stelle != kStelleLevel))
        m_stelle = kStelleSource;
    m_stelleFuer = m_gewaehlt;
    {
        const QSignalBlocker ruhe(m_gesamt);
        m_gesamt->setValue(gerundet(m_instrument.laut * 100.0));
        m_gesamtWert->setText(QStringLiteral("%1 %").arg(gerundet(m_instrument.laut * 100.0)));
    }
    zeigeListe();
    m_schichtKopf->setText(tr("Layer %1").arg(m_gewaehlt + 1));
    if (!m_schichtName->hasFocus()) m_schichtName->setText(gewaehlte.name);
    m_schichtName->setPlaceholderText(tr("Layer %1").arg(m_gewaehlt + 1));
    m_legende->setVisible(m_instrument.schichten.size() > 1);
    m_nurWahl->setEnabled(m_instrument.schichten.size() > 1);
    {
        const QSignalBlocker ruhe(m_welle);
        m_welle->setCurrentIndex(m_welle->findData(static_cast<int>(gewaehlte.schwingung)));
    }
    // --- nur Sample: die Datei, der Grundton, und die drei Stellen reichen so weit wie die Aufnahme
    const bool sample = gewaehlte.schwingung == Schwingung::Sample;
    if (sample)
    {
        const double laengeMs = sampleLaengeMs();
        const QString datei = sampleDatei(gewaehlte.sample);
        QString text = tr("(none)");
        if (!gewaehlte.sample.isEmpty())
        {
            if (datei.isEmpty() || !QFileInfo::exists(datei))
                text = tr("%1 (not found)").arg(sampleTitel(gewaehlte.sample));
            else
            {
                text = QFileInfo(datei).fileName();
                if (laengeMs > 0.0) text += QStringLiteral(" · %1 s").arg(laengeMs / 1000.0, 0, 'f', 1);
                if (istEigenesSample(gewaehlte.sample)) text += tr(" · My samples");
                if (istMitgelieferterKlang(gewaehlte.sample)) text += tr(" · built-in");
            }
        }
        m_sampleName->setText(text);
        m_sampleName->setToolTip(datei.isEmpty() ? QString() : QDir::toNativeSeparators(datei));
        // Edit... (Stufe 17b): nur, wenn es etwas zu oeffnen gibt
        m_sampleBearbeiten->setEnabled(laengeMs > 0.0);
        {
            const QSignalBlocker ruhe(m_grundton);
            m_grundton->setCurrentIndex(m_grundton->findData(std::clamp(gewaehlte.grundton, 0, 127)));
        }
        {
            const QSignalBlocker ruhe(m_schleife);
            m_schleife->setChecked(gewaehlte.schleife);
            m_schleife->setEnabled(laengeMs > 0.0);
        }
        const int bis = std::max(1, gerundet(laengeMs));
        for (const std::size_t platz : {m_reglerStart, m_reglerVon, m_reglerBis})
        {
            const QSignalBlocker ruhe(m_regler[platz].schieber);
            m_regler[platz].schieber->setRange(0, bis);
            m_regler[platz].schieber->setEnabled(laengeMs > 0.0 && (platz == m_reglerStart || gewaehlte.schleife));
        }
    }
    for (Regler& r : m_regler)
    {
        // den Regler nur setzen, wenn er etwas anderes zeigt, als der Wert verlangt —
        // ein Wert zwischen zwei Stellungen bleibt, bis jemand am Regler zieht
        const QSignalBlocker ruhe(r.schieber);
        r.schieber->setValue(std::clamp(r.lese(gewaehlte), r.schieber->minimum(), r.schieber->maximum()));
        r.wert->setText(r.text(gewaehlte));
    }
    // erst alles in den Gruppen zeigen — dann blendet aus, was die Schwingung nicht kennt,
    // und zuletzt, was eingeklappt ist (`klappeGruppen`)
    for (int i = 0; i < m_gitter->count() && !m_gruppen.empty(); ++i)
    {
        int zeile = 0;
        int spalte = 0;
        int hoch = 0;
        int breit = 0;
        m_gitter->getItemPosition(i, &zeile, &spalte, &hoch, &breit);
        if (QWidget* teil = m_gitter->itemAt(i)->widget(); teil != nullptr && zeile >= m_gruppen.front().zeile)
            teil->setVisible(true);
    }
    // was die Schwingung nicht kennt, bleibt weg
    const bool saite = gewaehlte.schwingung == Schwingung::Saite;
    const bool sinus = gewaehlte.schwingung == Schwingung::Sinus;
    const auto zeige = [this](std::size_t platz, bool an) {
        Regler& r = m_regler[platz];
        r.name->setVisible(an);
        r.schieber->setVisible(an);
        r.wert->setVisible(an);
    };
    // Rauschen (Stufe 11d) hat keine Tonhoehe: Voices, Detune, Vibrato und Pitch sweep fehlen
    const bool rauschen = gewaehlte.schwingung == Schwingung::Rauschen;
    for (const std::size_t platz : m_nichtSaite) zeige(platz, !saite);
    for (const std::size_t platz : m_nichtSample) zeige(platz, !saite && !sample && !rauschen);
    for (const std::size_t platz : m_nurSinus) zeige(platz, sinus);
    for (const std::size_t platz : m_nurSample) zeige(platz, sample);
    for (const std::size_t platz : m_nurTonweg) zeige(platz, !saite && !rauschen);
    // --- nur Shape (Stufe 19e): die Form, ihre Konturen, die vier Regler und die zwei Haken
    const bool form = gewaehlte.schwingung == Schwingung::Form;
    for (const std::size_t platz : m_nurForm) zeige(platz, form);
    for (QWidget* teil : m_formZeilen) teil->setVisible(form);
    if (form)
    {
        liesForm(gewaehlte.form);
        m_formName->setText(gewaehlte.form.isEmpty() ? tr("(none)")
                            : !m_formGefunden        ? tr("%1 (not found)").arg(gewaehlte.form)
                                                     : m_formZeichnung.titel);
        m_formName->setToolTip(shape::istKonturDatei(gewaehlte.form) ? QDir::toNativeSeparators(gewaehlte.form) : gewaehlte.form);
        m_formBearbeiten->setEnabled(m_formGefunden);
        m_formBearbeiten->setToolTip(m_formGefunden ? tr("Draw, move and order the contours of this shape and save it")
                                                    : tr("The shape of this layer is not found — choose one with Browse..."));
        const QSignalBlocker ruhe(m_formKontur);
        m_formKontur->clear();
        m_formKontur->addItem(tr("All, one path"), 0);
        for (std::size_t n = 0; n < m_formZeichnung.konturen.size(); ++n)
        {
            const shape::Kontur& k = m_formZeichnung.konturen[n];
            m_formKontur->addItem(QStringLiteral("%1 %2").arg(n + 1).arg(k.name.isEmpty() ? tr("Contour") : k.name), k.kennung);
        }
        int platz = m_formKontur->findData(gewaehlte.formKontur);
        if (platz < 0)
        {
            // die Kontur gibt es in dieser Form nicht (mehr): die Schicht schweigt, das steht da
            m_formKontur->addItem(tr("Contour %1 (missing)").arg(gewaehlte.formKontur), gewaehlte.formKontur);
            platz = m_formKontur->count() - 1;
        }
        m_formKontur->setCurrentIndex(platz);
        const QSignalBlocker ruheStereo(m_formStereo);
        const QSignalBlocker ruheZurueck(m_formRueckwaerts);
        m_formStereo->setChecked(gewaehlte.stereoXY);
        m_formRueckwaerts->setChecked(gewaehlte.rueckwaerts);
    }
    // ohne Weg gibt es keine Zeit zu stellen
    m_regler[m_reglerWegZeit].schieber->setEnabled(gerundet(gewaehlte.tonhoeheStartHt) != 0);
    // der gewaehlte Filter (Stufe 11e, 16c): die Saite hat keinen; steht Cutoff auf „open", ist nichts zu stellen
    const SchichtFilter* const filter = gewaehlterFilter(gewaehlte);
    {
        const QSignalBlocker ruhe(m_filterArt);
        m_filterArt->setCurrentIndex(m_filterArt->findData(static_cast<int>(filter != nullptr ? filter->art : FilterArt::Tiefpass)));
    }
    m_filterArtName->setVisible(!saite);
    m_filterArt->setVisible(!saite);
    m_filterArt->setEnabled(filter != nullptr && filter->teilton > 0.0);
    m_regler[m_reglerResonanz].schieber->setEnabled(filter != nullptr && filter->teilton > 0.0);
    for (QWidget* teil : m_sampleZeilen) teil->setVisible(sample);
    {
        // Detect: das Ergebnis gilt fuer die Schicht und die Aufnahme, an der gemessen wurde
        const bool gemessenDa = sample && !m_gemessenFuer.isEmpty() && m_gemessenFuer == messSchluessel();
        m_detect->setEnabled(sample && sampleLaengeMs() > 0.0);
        m_detectText->setVisible(gemessenDa);
        m_detectApply->setVisible(gemessenDa && m_gemessen.gefunden);
        if (gemessenDa)
            m_detectText->setText(m_gemessen.gefunden ? tr("Detected: %1").arg(grundtonText(m_gemessen))
                                                      : tr("No clear pitch found"));
        m_referenz->setVisible(sample);
    }
    m_vibratoTitel->setVisible(!saite && !sample && !rauschen);  // eine Ueberschrift ohne Regler darunter bleibt weg
    zeige(m_daempfung, saite);
    zeigeWeg();
    klappeGruppen();
    zeigeBilder();
}

// --- Repeat note (Wunsch Patrik S116) ---------------------------------------------

void InstrumentCreator::setzeWiederholen(bool an)
{
    {
        const QSignalBlocker ruhe(m_wiederholen);
        m_wiederholen->setChecked(an);
    }
    if (an == m_wiederholTakt->isActive()) return;
    if (!an)
    {
        m_wiederholTakt->stop();
        m_tasten->setGewaehlt(-1);  // hervorgehoben ist die Taste nur, solange es laeuft
        return;
    }
    m_tasten->setGewaehlt(m_wiederholHoehe);
    m_wiederholTakt->start(wiederholMs());
    (void)spieleAn(m_wiederholHoehe);  // gleich, nicht erst nach dem ersten Abstand
}

bool InstrumentCreator::wiederholt() const
{
    return m_wiederholTakt->isActive();
}

void InstrumentCreator::setzeSchlagMs(double ms)
{
    if (!std::isfinite(ms) || ms <= 0.0) return;
    m_schlagMs = ms;
    if (wiederholt()) m_wiederholTakt->setInterval(wiederholMs());
}

void InstrumentCreator::setzeWiederholSchlaege(int schlaege)
{
    const int index = m_wiederholAbstand->findData(schlaege);
    if (index < 0) return;  // nur, was die Auswahl nennt
    m_wiederholSchlaege = schlaege;
    {
        const QSignalBlocker ruhe(m_wiederholAbstand);
        m_wiederholAbstand->setCurrentIndex(index);
    }
    if (wiederholt()) m_wiederholTakt->setInterval(wiederholMs());
}

int InstrumentCreator::wiederholMs() const
{
    return std::max(250, gerundet(m_wiederholSchlaege * m_schlagMs));
}

void InstrumentCreator::hideEvent(QHideEvent* event)
{
    setzeWiederholen(false);
    QWidget::hideEvent(event);
}

bool InstrumentCreator::eventFilter(QObject* objekt, QEvent* ereignis)
{
    // ein Klick auf den Farbpunkt einer Schicht: die Farbwahl
    if (m_liste != nullptr && objekt == m_liste->viewport() && ereignis->type() == QEvent::MouseButtonPress)
    {
        const auto* maus = static_cast<QMouseEvent*>(ereignis);
        if (QListWidgetItem* eintrag = m_liste->itemAt(maus->position().toPoint());
            eintrag != nullptr && maus->button() == Qt::LeftButton && farbPunktRect(m_liste->row(eintrag)).contains(maus->position().toPoint()))
        {
            zeigeFarbMenue(m_liste->row(eintrag), m_liste->viewport()->mapToGlobal(maus->position().toPoint()));
            return true;
        }
    }
    // im rollenden Feld gehoert das Mausrad dem Rollen: der Regler nimmt es nicht, das Ereignis
    // geht an das Feld weiter
    if (ereignis->type() == QEvent::Wheel && m_rollen != nullptr)
        if (const auto* teil = qobject_cast<QWidget*>(objekt); teil != nullptr && m_rollen->widget()->isAncestorOf(teil))
        {
            ereignis->ignore();
            return true;
        }
    return QWidget::eventFilter(objekt, ereignis);
}

// --- Gruppen ein- und ausklappen (Wunsch Patrik S116) ---------------------------

void InstrumentCreator::klappeGruppen()
{
    for (std::size_t g = 0; g < m_gruppen.size(); ++g)
    {
        const Gruppe& gruppe = m_gruppen[g];
        gruppe.kopf->setArrowType(gruppe.zu ? Qt::RightArrow : Qt::DownArrow);
        if (!gruppe.zu) continue;
        // ihre Zeilen: bis zur naechsten Ueberschrift, die letzte bis ans Ende des Gitters
        const int bis = gruppe.ende >= 0 ? gruppe.ende : g + 1 < m_gruppen.size() ? m_gruppen[g + 1].zeile : m_gitter->rowCount();
        for (int i = 0; i < m_gitter->count(); ++i)
        {
            int zeile = 0;
            int spalte = 0;
            int hoch = 0;
            int breit = 0;
            m_gitter->getItemPosition(i, &zeile, &spalte, &hoch, &breit);
            if (QWidget* teil = m_gitter->itemAt(i)->widget(); teil != nullptr && zeile > gruppe.zeile && zeile < bis)
                teil->setVisible(false);
        }
    }
    zeigeStelle();
    // Das rollende Feld behaelt die Hoehe, die seine Zeilen brauchen: sonst drueckte das Fenster
    // sie zusammen, bevor der Rollbalken kaeme (im Bild aus dem Test gesehen).
    if (m_rollen != nullptr)
    {
        m_gitter->invalidate();
        m_rollen->widget()->setMinimumHeight(m_gitter->sizeHint().height());
    }
}

bool InstrumentCreator::gruppeZu(const QString& schluessel) const
{
    for (const Gruppe& gruppe : m_gruppen)
        if (gruppe.schluessel == schluessel) return gruppe.zu;
    return false;
}

void InstrumentCreator::setzeGruppeZu(const QString& schluessel, bool zu)
{
    for (Gruppe& gruppe : m_gruppen)
    {
        if (gruppe.schluessel != schluessel || gruppe.zu == zu) continue;
        gruppe.zu = zu;
        if (gruppenMerken()) QSettings().setValue(QLatin1String(kGruppeZu) + schluessel, zu);
        zeigeAlles();
        return;
    }
}

void InstrumentCreator::setzeGruppenMerken(bool an)
{
    gruppenMerken() = an;
}

// =============================================================================
// Signal path (Stufe 16c; Skizze gebilligt, Patrik S118, Entscheid S1 a)
// =============================================================================

const SchichtFilter* InstrumentCreator::gewaehlterFilter(const Schicht& schicht) const
{
    if (m_stelle < 0 || m_stelle >= static_cast<int>(schicht.reihe.size())) return nullptr;
    return std::get_if<SchichtFilter>(&schicht.reihe[static_cast<std::size_t>(m_stelle)]);
}

SchichtFilter* InstrumentCreator::gewaehlterFilter(Schicht& schicht) const
{
    if (m_stelle < 0 || m_stelle >= static_cast<int>(schicht.reihe.size())) return nullptr;
    return std::get_if<SchichtFilter>(&schicht.reihe[static_cast<std::size_t>(m_stelle)]);
}

const SchichtDrive* InstrumentCreator::gewaehlterDrive(const Schicht& schicht) const
{
    if (m_stelle < 0 || m_stelle >= static_cast<int>(schicht.reihe.size())) return nullptr;
    return std::get_if<SchichtDrive>(&schicht.reihe[static_cast<std::size_t>(m_stelle)]);
}

SchichtDrive* InstrumentCreator::gewaehlterDrive(Schicht& schicht) const
{
    if (m_stelle < 0 || m_stelle >= static_cast<int>(schicht.reihe.size())) return nullptr;
    return std::get_if<SchichtDrive>(&schicht.reihe[static_cast<std::size_t>(m_stelle)]);
}

const InstrumentCreator::Abschnitt& InstrumentCreator::gewaehlterAbschnitt() const
{
    if (m_stelle == kStelleLevel) return m_zeilenLevel;
    const std::vector<SchichtBaustein>& reihe = schicht().reihe;
    if (m_stelle < 0 || m_stelle >= static_cast<int>(reihe.size())) return m_zeilenSource;
    const SchichtBaustein& baustein = reihe[static_cast<std::size_t>(m_stelle)];
    if (std::holds_alternative<SchichtFilter>(baustein)) return m_zeilenFilter;
    if (std::holds_alternative<SchichtDrive>(baustein)) return m_zeilenDrive;
    return m_zeilenHuelle;
}

void InstrumentCreator::waehleStelle(int stelle)
{
    const int zahl = static_cast<int>(schicht().reihe.size());
    const int neu = stelle == kStelleLevel || (stelle >= 0 && stelle < zahl) ? stelle : kStelleSource;
    if (neu == m_stelle) return;
    m_stelle = neu;
    zeigeAlles();
}

bool InstrumentCreator::fuegeEin(const SchichtBaustein& baustein)
{
    const int zahl = static_cast<int>(schicht().reihe.size());
    if (zahl >= kBausteineMax) return false;
    // hinter dem gewaehlten: bei Source vorn, bei Level hinten
    const int platz = m_stelle == kStelleLevel ? zahl : m_stelle < 0 ? 0 : std::min(m_stelle + 1, zahl);
    m_stelle = platz;  // vor dem Aendern: die Anzeige soll schon den neuen zeigen
    aendereSchicht([&baustein, platz](Schicht& s) { s.reihe.insert(s.reihe.begin() + platz, baustein); });
    return true;
}

bool InstrumentCreator::fuegeFilterEin()
{
    // ein neuer Filter soll zu hoeren sein: nicht offen
    return fuegeEin(SchichtFilter{FilterArt::Tiefpass, 4.0, 0.0, 0.0});
}

bool InstrumentCreator::fuegeDriveEin()
{
    return fuegeEin(SchichtDrive{0.3});
}

bool InstrumentCreator::verschiebeBaustein(int von, int nach)
{
    const int zahl = static_cast<int>(schicht().reihe.size());
    if (von < 0 || von >= zahl || nach < 0 || nach >= zahl) return false;
    m_stelle = nach;
    if (von == nach)
    {
        zeigeAlles();
        return true;
    }
    const Instrument vorher = m_instrument;
    aendereSchicht([von, nach](Schicht& s) {
        const SchichtBaustein baustein = s.reihe[static_cast<std::size_t>(von)];
        s.reihe.erase(s.reihe.begin() + von);
        s.reihe.insert(s.reihe.begin() + nach, baustein);
    });
    if (m_instrument == vorher) zeigeAlles();  // zwei gleiche Bausteine getauscht: nur die Wahl wandert
    return true;
}

bool InstrumentCreator::schiebeBaustein(int richtung)
{
    if (m_stelle < 0) return false;
    const int ziel = m_stelle + (richtung < 0 ? -1 : 1);
    if (ziel < 0 || ziel >= static_cast<int>(schicht().reihe.size())) return false;
    return verschiebeBaustein(m_stelle, ziel);
}

bool InstrumentCreator::entferneBaustein()
{
    const std::vector<SchichtBaustein>& reihe = schicht().reihe;
    if (m_stelle < 0 || m_stelle >= static_cast<int>(reihe.size())) return false;
    if (std::holds_alternative<SchichtHuelle>(reihe[static_cast<std::size_t>(m_stelle)])) return false;
    const int platz = m_stelle;
    // danach ist gewaehlt, was an seinen Platz rueckt — oder der davor
    m_stelle = std::min(platz, static_cast<int>(reihe.size()) - 2);
    aendereSchicht([platz](Schicht& s) { s.reihe.erase(s.reihe.begin() + platz); });
    return true;
}

void InstrumentCreator::zeigeAddMenue(QPoint unten)
{
    QMenu menue(this);
    menue.setObjectName(QStringLiteral("creatorAddBlockMenu"));
    connect(menue.addAction(tr("Filter")), &QAction::triggered, this, [this] { (void)fuegeFilterEin(); });
    connect(menue.addAction(tr("Drive")), &QAction::triggered, this, [this] { (void)fuegeDriveEin(); });
    // jede Schicht hat genau eine Envelope: sie steht da, laesst sich aber nicht noch einmal holen
    menue.addAction(tr("Envelope — already in the path"))->setEnabled(false);
    menue.exec(m_pfad->mapToGlobal(unten));
}

void InstrumentCreator::zeigeWeg()
{
    const Schicht& s = schicht();
    const bool saite = s.schwingung == Schwingung::Saite;
    const int zahl = static_cast<int>(s.reihe.size());
    std::vector<WegKasten> kaesten;
    // --- Source: die Schwingung, und ob die Note faellt oder steigt
    QString quelle = m_welle->itemText(m_welle->findData(static_cast<int>(s.schwingung)));
    if (const int st = gerundet(s.tonhoeheStartHt);
        st != 0 && s.tonhoeheZeitMs > 0.0 && !saite && s.schwingung != Schwingung::Rauschen)
        quelle += st > 0 ? QString::fromUtf8(" ↓ %1 st").arg(st) : QString::fromUtf8(" ↑ %1 st").arg(-st);
    // eine Shape-Schicht nennt ihre Form: „Shape · Heart"
    if (s.schwingung == Schwingung::Form && !s.form.isEmpty())
        quelle += QString::fromUtf8(" · %1").arg(m_formGelesen == s.form && m_formGefunden ? m_formZeichnung.titel : s.form);
    kaesten.push_back(WegKasten{tr("Source"), quelle, false, false, tr("What makes the tone: waveform, transpose, pitch sweep, vibrato")});
    for (const SchichtBaustein& baustein : s.reihe)
    {
        if (const auto* filter = std::get_if<SchichtFilter>(&baustein))
        {
            const QString art = m_filterArt->itemText(m_filterArt->findData(static_cast<int>(filter->art)));
            const bool offen = filter->teilton <= 0.0;
            kaesten.push_back(WegKasten{tr("Filter"),
                                        saite   ? tr("skipped")
                                        : offen ? tr("open")
                                                : QString::fromUtf8("%1 ×%2").arg(art).arg(filter->teilton, 0, 'f', 1),
                                        false, saite || offen,
                                        saite ? tr("A plucked string has no filter — this block is skipped") : QString()});
        }
        else if (const auto* drive = std::get_if<SchichtDrive>(&baustein))
            kaesten.push_back(WegKasten{tr("Drive"),
                                        drive->wert <= 0.0 ? tr("off") : QStringLiteral("%1 %").arg(gerundet(drive->wert * 100.0)),
                                        false, drive->wert <= 0.0, QString()});
        else
            kaesten.push_back(WegKasten{tr("Envelope"), tr("Decay %1 ms").arg(gerundet(s.abfallMs)), true, false,
                                        tr("Where the envelope shapes the tone — every layer has exactly one. Drag it to move it")});
    }
    // der Kasten nennt Pan nur neben der Mitte: „80 % · Pan L 20"
    QString pegel = QStringLiteral("%1 %").arg(gerundet(s.laut * 100.0));
    if (gerundet(s.pan * 100.0) != 0) pegel += tr(" · Pan %1").arg(panText(s.pan));
    kaesten.push_back(WegKasten{tr("Level"), pegel, false, false, tr("The level and the pan of this layer")});
    const bool voll = zahl >= kBausteineMax;
    const int gewaehlt = m_stelle == kStelleLevel ? zahl + 1 : m_stelle < 0 ? 0 : m_stelle + 1;
    m_pfad->setKaesten(std::move(kaesten), gewaehlt, !voll,
                      voll ? tr("%1 of %1").arg(kBausteineMax) : tr("+ Add"));

    QString titel = tr("Signal path — Layer %1").arg(m_gewaehlt + 1);
    if (!s.name.isEmpty()) titel += QStringLiteral(" \"%1\"").arg(s.name);
    m_wegTitel->setText(titel);

    // --- ueber den Reglern: wie der gewaehlte Kasten heisst, und was sich mit ihm tun laesst
    const bool baustein = m_stelle >= 0;
    const bool huelle = baustein && std::holds_alternative<SchichtHuelle>(s.reihe[static_cast<std::size_t>(m_stelle)]);
    m_stellenTitel->setText(m_pfad->kaesten()[static_cast<std::size_t>(gewaehlt)].titel);
    m_bausteinVor->setVisible(baustein);
    m_bausteinZurueck->setVisible(baustein);
    m_bausteinWeg->setVisible(baustein);
    m_bausteinVor->setEnabled(baustein && m_stelle > 0);
    m_bausteinZurueck->setEnabled(baustein && m_stelle + 1 < zahl);
    m_bausteinWeg->setEnabled(baustein && !huelle);
    m_bausteinWeg->setToolTip(huelle ? tr("The envelope stays in the path — every layer has exactly one")
                                     : tr("Remove the selected block from the path"));
}

void InstrumentCreator::zeigeStelle()
{
    // im Gitter nur, was zum gewaehlten Kasten gehoert; was davor schon ausgeblendet wurde
    // (die Schwingung kennt es nicht, die Gruppe ist eingeklappt), bleibt weg
    const Abschnitt& gewaehlt = gewaehlterAbschnitt();
    const int ersteGruppe = m_gruppen.empty() ? 0 : m_gruppen.front().zeile;
    for (int i = 0; i < m_gitter->count(); ++i)
    {
        int zeile = 0;
        int spalte = 0;
        int hoch = 0;
        int breit = 0;
        m_gitter->getItemPosition(i, &zeile, &spalte, &hoch, &breit);
        QWidget* teil = m_gitter->itemAt(i)->widget();
        if (teil == nullptr) continue;
        const bool dabei = zeile >= gewaehlt.von && zeile < gewaehlt.bis;
        if (!dabei)
            teil->setVisible(false);
        else if (zeile < ersteGruppe)
            teil->setVisible(true);  // vor der ersten Gruppe blendet sonst niemand ein
    }
}

// =============================================================================
// Die Aufnahme einer Sample-Schicht (Stufe 11a)
// =============================================================================

std::shared_ptr<const SampleDaten> InstrumentCreator::holeSample(const QString& nennung)
{
    if (nennung.isEmpty()) return nullptr;
    std::shared_ptr<const SampleDaten> daten = sampleDaten(nennung);
    if (daten == nullptr && nennung != m_sampleVersucht)
    {
        // wer eine Audio-Engine hat, dekodiert und merkt — noch im Signal
        m_sampleVersucht = nennung;
        Q_EMIT sampleVerlangt(nennung);
        daten = sampleDaten(nennung);
    }
    if (daten != nullptr && nennung == m_sampleVersucht) m_sampleVersucht.clear();
    return daten;
}

double InstrumentCreator::sampleLaengeMs()
{
    if (schicht().schwingung != Schwingung::Sample) return 0.0;
    const std::shared_ptr<const SampleDaten> daten = holeSample(schicht().sample);
    return daten != nullptr && daten->rate > 0 ? 1000.0 * static_cast<double>(daten->klang.frames()) / daten->rate : 0.0;
}

void InstrumentCreator::setEngineGeber(std::function<IAudioEngine*()> geber)
{
    m_engineGeber = std::move(geber);
}

void InstrumentCreator::richteAufnahmeEin()
{
    if (m_aufnahme == nullptr)
    {
        m_aufnahme = new AufnahmeFenster(this);
        // Save und Save as...: das Sample ist das der Schicht, die jetzt gewaehlt ist
        connect(m_aufnahme, &AufnahmeFenster::gespeichert, this, [this](const QString& nennung, double vornWegMs) {
            if (schicht().schwingung == Schwingung::Sample) uebernimmSample(nennung, vornWegMs);
        });
    }
    // die Engine, wie sie jetzt ist — sie kann seit dem letzten Mal eine andere sein
    m_aufnahme->setEngine(m_engineGeber ? m_engineGeber() : nullptr);
}

void InstrumentCreator::zeigeAufnahme()
{
    m_aufnahme->show();
    m_aufnahme->raise();
    m_aufnahme->activateWindow();
}

bool InstrumentCreator::oeffneAufnahme(bool zeigen)
{
    if (schicht().schwingung != Schwingung::Sample) return false;
    richteAufnahmeEin();
    m_bearbeitet.clear();
    m_aufnahme->beginne();
    if (zeigen) zeigeAufnahme();
    return true;
}

QString InstrumentCreator::bearbeitenHindernis()
{
    if (schicht().schwingung != Schwingung::Sample || schicht().sample.isEmpty()) return {};
    if (sampleLaengeMs() <= 1000.0 * AufnahmeFenster::kHoechstensSekunden) return {};
    return tr("This sample is longer than %1 s. Edit... works on samples up to %1 s.").arg(AufnahmeFenster::kHoechstensSekunden, 0, 'f', 0);
}

bool InstrumentCreator::oeffneBearbeiten(bool zeigen)
{
    if (schicht().schwingung != Schwingung::Sample) return false;
    const QString nennung = schicht().sample;
    const std::shared_ptr<const SampleDaten> daten = holeSample(nennung);
    if (daten == nullptr || daten->rate <= 0 || daten->klang.frames() <= 0) return false;
    if (!bearbeitenHindernis().isEmpty()) return false;
    richteAufnahmeEin();
    m_bearbeitet = nennung;
    m_aufnahme->bearbeite(nennung, daten->klang.daten, daten->rate);
    if (zeigen) zeigeAufnahme();
    return true;
}

void InstrumentCreator::uebernimmSample(const QString& nennung, double vornWegMs)
{
    // zugeschnitten ist, was diese Schicht spielte, als Edit... gedrueckt wurde — sonst ist es ein anderer Klang
    const bool zugeschnitten = vornWegMs >= 0.0 && !m_bearbeitet.isEmpty() && schicht().sample == m_bearbeitet;
    m_bearbeitet.clear();
    if (!zugeschnitten)
    {
        const bool derselbeName = schicht().sample == nennung;
        setzeSample(nennung);
        // unter demselben Namen neu aufgenommen: auch das beginnt von vorn
        if (derselbeName)
            aendereSchicht([](Schicht& s) {
                s.sampleStartMs = 0.0;
                s.schleife = false;
                s.schleifeVonMs = 0.0;
                s.schleifeBisMs = 0.0;
            });
        return;
    }
    m_sampleVersucht.clear();
    m_gemessenFuer.clear();
    aendereSchicht([&nennung, vornWegMs](Schicht& s) {
        s.sample = nennung;
        // was vorn weg ist, fehlt jeder Stelle im Sample: der Einsatz und die Schleife ruecken mit (M3 a)
        s.sampleStartMs = std::max(0.0, s.sampleStartMs - vornWegMs);
        s.schleifeVonMs = std::max(0.0, s.schleifeVonMs - vornWegMs);
        s.schleifeBisMs = std::max(0.0, s.schleifeBisMs - vornWegMs);
        if (s.schleifeBisMs <= s.schleifeVonMs) s.schleife = false;
    });
    zeigeAlles();
}

void InstrumentCreator::setzeSample(const QString& nennung)
{
    m_sampleVersucht.clear();  // ein neuer Versuch, auch mit derselben Nennung
    m_gemessenFuer.clear();    // was gemessen wurde, galt der Aufnahme davor — auch unter demselben Namen
    aendereSchicht([&nennung](Schicht& s) {
        if (s.sample == nennung) return;
        s.sample = nennung;
        // eine andere Aufnahme beginnt von vorn
        s.sampleStartMs = 0.0;
        s.schleife = false;
        s.schleifeVonMs = 0.0;
        s.schleifeBisMs = 0.0;
    });
    zeigeAlles();  // dieselbe Nennung, aber vielleicht jetzt zu laden
}

bool InstrumentCreator::nimmSampleDatei(const QString& pfad, bool kopieren)
{
    const QString nennung = kopieren ? nimmZuEigenen(pfad) : pfad;
    if (nennung.isEmpty()) return false;
    setzeSample(nennung);
    return true;
}

QString InstrumentCreator::messSchluessel() const
{
    return QStringLiteral("%1|%2").arg(m_gewaehlt).arg(schicht().sample);
}

bool InstrumentCreator::messe()
{
    if (schicht().schwingung != Schwingung::Sample) return false;
    const std::shared_ptr<const SampleDaten> daten = holeSample(schicht().sample);
    if (daten == nullptr) return false;
    const auto ab = static_cast<std::int64_t>(std::max(0.0, schicht().sampleStartMs) * daten->rate / 1000.0);
    m_gemessen = messeGrundton(daten->klang.daten, daten->rate, ab);
    m_gemessenFuer = messSchluessel();
    zeigeAlles();
    return m_gemessen.gefunden;
}

Grundton InstrumentCreator::gemessen() const
{
    return !m_gemessenFuer.isEmpty() && m_gemessenFuer == messSchluessel() ? m_gemessen : Grundton{};
}

bool InstrumentCreator::uebernimmGemessenes()
{
    const Grundton ton = gemessen();
    if (!ton.gefunden) return false;
    aendereSchicht([&ton](Schicht& s) {
        s.grundton = std::clamp(ton.hoehe, 0, 127);
        s.feinCent = std::round(ton.cent);
    });
    return true;
}

void InstrumentCreator::setReferenzton(bool an)
{
    m_referenz->setChecked(an);
    m_referenzton = an;
}

void InstrumentCreator::setzeSchleife(bool an)
{
    const double laenge = sampleLaengeMs();
    aendereSchicht([an, laenge](Schicht& s) {
        s.schleife = an;
        // zum ersten Mal an: der mittlere Teil der Aufnahme
        if (an && s.schleifeBisMs <= s.schleifeVonMs && laenge > 0.0)
        {
            s.schleifeVonMs = std::round(laenge * 0.25);
            s.schleifeBisMs = std::round(laenge * 0.75);
        }
    });
}

// =============================================================================
// Die Form einer Shape-Schicht (Stufe 19e)
// =============================================================================

void InstrumentCreator::liesForm(const QString& nennung)
{
    // eine geschriebene Form (Edit shape) ist neu zu lesen, auch bei gleicher Nennung
    if (m_formGelesenDa && m_formGelesen == nennung && m_formFassung == shape::zeichnungFassung()) return;
    m_formGelesen = nennung;
    m_formGelesenDa = true;
    m_formFassung = shape::zeichnungFassung();
    m_formZeichnung = shape::Zeichnung{};
    m_formGefunden = !nennung.isEmpty() && shape::holeZeichnung(nennung, m_formZeichnung);
    if (!m_formGefunden) m_formZeichnung = shape::Zeichnung{};
}

void InstrumentCreator::setzeForm(const QString& nennung)
{
    m_formGelesenDa = false;  // auch dieselbe Nennung noch einmal lesen: die Datei kann eine andere sein
    aendereSchicht([&nennung](Schicht& s) {
        s.form = nennung;
        s.formKontur = 0;  // eine andere Form hat andere Konturen
    });
}

void InstrumentCreator::zeigeFormWahl()
{
    QMenu menue(this);
    menue.addSection(tr("Built-in shapes"));
    for (const shape::Zeichnung& z : shape::zeichnungen())
        connect(menue.addAction(z.titel), &QAction::triggered, this, [this, nennung = z.name] { setzeForm(nennung); });
    const std::vector<shape::Zeichnung> eigene = shape::eigeneZeichnungen();
    if (!eigene.empty())
    {
        menue.addSection(tr("My shapes"));
        for (const shape::Zeichnung& z : eigene)
            connect(menue.addAction(z.titel), &QAction::triggered, this, [this, nennung = z.name] { setzeForm(nennung); });
    }
    menue.addSeparator();
    connect(menue.addAction(tr("From file...")), &QAction::triggered, this, [this] {
        QSettings einstellungen;
        const QString pfad = QFileDialog::getOpenFileName(this, tr("Choose a shape"),
                                                          einstellungen.value(QLatin1String(kLetzterFormOrdner)).toString(),
                                                          tr("LumiViz shapes (*%1)").arg(QLatin1String(shape::kKonturEndung)));
        if (pfad.isEmpty()) return;
        einstellungen.setValue(QLatin1String(kLetzterFormOrdner), QFileInfo(pfad).absolutePath());
        setzeForm(pfad);  // die Schicht nennt den Pfad, die Datei bleibt, wo sie ist
    });
    menue.addSeparator();
    connect(menue.addAction(tr("New shape...")), &QAction::triggered, this, [this] { (void)oeffneNeueForm(true); });
    menue.exec(m_formWahl->mapToGlobal(QPoint(0, m_formWahl->height())));
}

void InstrumentCreator::richteFormFensterEin()
{
    if (m_formFenster == nullptr)
    {
        m_formFenster = new FormFenster(this);
        // Save und Save as...: die Form ist die der Schicht, die jetzt gewaehlt ist
        connect(m_formFenster, &FormFenster::gespeichert, this, &InstrumentCreator::uebernimmForm);
    }
    // die Engine, wie sie jetzt ist — sie kann seit dem letzten Mal eine andere sein
    m_formFenster->setEngine(m_engineGeber ? m_engineGeber() : nullptr);
}

bool InstrumentCreator::oeffneFormBearbeiten(bool zeigen)
{
    if (schicht().schwingung != Schwingung::Form) return false;
    richteFormFensterEin();
    if (!m_formFenster->bearbeite(schicht().form)) return false;
    if (zeigen)
    {
        m_formFenster->show();
        m_formFenster->raise();
        m_formFenster->activateWindow();
    }
    return true;
}

bool InstrumentCreator::oeffneNeueForm(bool zeigen)
{
    if (schicht().schwingung != Schwingung::Form) return false;
    richteFormFensterEin();
    m_formFenster->beginneNeu();
    if (zeigen)
    {
        m_formFenster->show();
        m_formFenster->raise();
        m_formFenster->activateWindow();
    }
    return true;
}

void InstrumentCreator::uebernimmForm(const QString& nennung)
{
    if (schicht().schwingung != Schwingung::Form) return;
    m_formGelesenDa = false;  // auch dieselbe Nennung neu lesen
    m_formMeldung.clear();
    shape::Zeichnung zeichnung;
    std::vector<int> mitHaken;
    if (shape::holeZeichnung(nennung, zeichnung))
        for (const shape::Kontur& k : zeichnung.konturen)
            if (k.gewaehlt) mitHaken.push_back(k.kennung);

    // One path (G1 a) — oder keine Kontur mit Haken: die offene Schicht spielt alle als einen Pfad
    if (mitHaken.empty() || zeichnung.nutzung == shape::Nutzung::EinPfad)
    {
        aendereSchicht([&nennung](Schicht& s) {
            s.form = nennung;
            s.formKontur = 0;
        });
        zeigeAlles();
        return;
    }

    // Layers (L1 a): die offene Schicht spielt eine angehakte Kontur — ihre bisherige, wenn sie noch einen
    // Haken hat, sonst die erste, die keine Schicht spielt —, jede weitere angehakte bekommt dahinter
    // eine Kopie der offenen Schicht; eine Kontur, die schon eine Schicht mit dieser Form spielt, keine zweite
    const int platz = m_gewaehlt;
    const Schicht vorlage = schicht();
    int fehlen = 0;
    aendere([&](Instrument& instrument) {
        const auto spielt = [&instrument, &nennung](int kontur, int ausser) {
            for (std::size_t i = 0; i < instrument.schichten.size(); ++i)
            {
                const Schicht& s = instrument.schichten[i];
                if (static_cast<int>(i) != ausser && s.schwingung == Schwingung::Form && s.form == nennung && s.formKontur == kontur)
                    return true;
            }
            return false;
        };
        Schicht& offen = instrument.schichten[static_cast<std::size_t>(platz)];
        const bool behaelt = offen.form == nennung && std::find(mitHaken.begin(), mitHaken.end(), offen.formKontur) != mitHaken.end();
        if (!behaelt)
        {
            int eigene = mitHaken.front();
            for (const int k : mitHaken)
                if (!spielt(k, platz))
                {
                    eigene = k;
                    break;
                }
            offen.form = nennung;
            offen.formKontur = eigene;
        }
        Schicht kopie = offen;
        kopie.farbe.clear();  // jede Kopie traegt die Farbe ihrer Kontur
        const QString wer = vorlage.name.isEmpty() ? tr("Layer %1").arg(platz + 1) : vorlage.name;
        int hinter = platz;
        for (const int k : mitHaken)
        {
            if (spielt(k, -1)) continue;
            if (instrument.schichten.size() >= static_cast<std::size_t>(kSchichtenMax))
            {
                ++fehlen;
                continue;
            }
            const shape::Kontur* kontur = zeichnung.finde(k);
            kopie.formKontur = k;
            kopie.name = QStringLiteral("%1 · %2").arg(wer, kontur != nullptr && !kontur->name.isEmpty() ? kontur->name : tr("Contour"));
            instrument.schichten.insert(instrument.schichten.begin() + (++hinter), kopie);
        }
    });
    if (fehlen > 0)
    {
        m_formMeldung = (fehlen == 1 ? tr("1 ticked contour did not fit — an instrument holds %1 layers").arg(kSchichtenMax)
                                     : tr("%1 ticked contours did not fit — an instrument holds %2 layers").arg(fehlen).arg(kSchichtenMax));
        if (isVisible()) QMessageBox::information(this, tr("Edit shape"), m_formMeldung);
    }
    zeigeAlles();
}

void InstrumentCreator::setzeSchichtFarbe(int platz, const QString& farbe)
{
    if (platz < 0 || platz >= static_cast<int>(m_instrument.schichten.size())) return;
    const QColor gelesen(farbe);
    const QString neu = farbe.isEmpty() || !gelesen.isValid() ? QString() : gelesen.name(QColor::HexRgb);
    aendere([platz, &neu](Instrument& i) { i.schichten[static_cast<std::size_t>(platz)].farbe = neu; });
}

void InstrumentCreator::zeigeFarbMenue(int platz, QPoint wo)
{
    if (platz < 0 || platz >= static_cast<int>(m_instrument.schichten.size())) return;
    QMenu menue(this);
    connect(menue.addAction(tr("Colour...")), &QAction::triggered, this, [this, platz] {
        const QColor gewaehlt = QColorDialog::getColor(schichtFarbe(m_instrument, platz), this, tr("Layer colour"));
        if (gewaehlt.isValid()) setzeSchichtFarbe(platz, gewaehlt.name(QColor::HexRgb));
    });
    QAction* automatisch = menue.addAction(tr("Automatic"));
    automatisch->setCheckable(true);
    automatisch->setChecked(m_instrument.schichten[static_cast<std::size_t>(platz)].farbe.isEmpty());
    automatisch->setToolTip(tr("The colour of its waveform family — a Shape layer takes the colour of its contour"));
    connect(automatisch, &QAction::triggered, this, [this, platz] { setzeSchichtFarbe(platz, QString()); });
    menue.exec(wo);
}

QRect InstrumentCreator::farbPunktRect(int platz) const
{
    QListWidgetItem* eintrag = m_liste->item(platz);
    if (eintrag == nullptr) return {};
    QStyleOptionViewItem option;
    option.initFrom(m_liste);
    option.rect = m_liste->visualItemRect(eintrag);
    option.features = QStyleOptionViewItem::HasCheckIndicator | QStyleOptionViewItem::HasDecoration | QStyleOptionViewItem::HasDisplay;
    option.decorationSize = m_liste->iconSize().isValid() ? m_liste->iconSize() : QSize(16, 16);
    option.icon = eintrag->icon();
    option.text = eintrag->text();
    option.checkState = eintrag->checkState();
    return m_liste->style()->subElementRect(QStyle::SE_ItemViewItemDecoration, &option, m_liste).adjusted(-2, -2, 2, 2);
}

// =============================================================================
// FormBild — „Shape, as it plays"
// =============================================================================

FormBild::FormBild(QWidget* parent) : QWidget(parent)
{
    setMinimumHeight(120);
}

void FormBild::setForm(const std::shared_ptr<const shape::Tonform>& form, double harmonische, double winkelGrad)
{
    m_zeichnung.clear();
    m_umriss.clear();
    m_winkelGrad = winkelGrad;
    if (form != nullptr && !form->leer())
    {
        // ein Umlauf des Stifts — bei mehreren Konturen mit den Spruengen dazwischen
        constexpr int kPunkte = 720;
        for (int n = 0; n <= kPunkte; ++n)
        {
            const double umlauf = static_cast<double>(n % kPunkte) / kPunkte;
            const shape::Punkt gezeichnet = shape::tonwert(*form, umlauf, shape::kHarmonischeMax);
            const shape::Punkt gespielt = shape::tonwert(*form, umlauf, harmonische);
            m_zeichnung << QPointF(gezeichnet.real(), gezeichnet.imag());
            m_umriss << QPointF(gespielt.real(), gespielt.imag());
        }
    }
    update();
}

void FormBild::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QPalette pal = palette();
    p.fillRect(rect(), pal.color(QPalette::Base));
    QColor leise = pal.color(QPalette::Text);
    leise.setAlpha(90);
    p.setPen(leise);
    p.drawRect(rect().adjusted(0, 0, -1, -1));
    if (m_umriss.size() < 2) return;
    // die Form fuellt mit etwas Rand ein Quadrat in der Mitte; y zeigt in der Form nach oben
    const double rand = 12.0;
    const double halb = std::max(1.0, std::min(width(), height()) / 2.0 - rand);
    const QPointF mitte(width() / 2.0, height() / 2.0);
    const auto imBild = [&](const QPolygonF& zug) {
        QPolygonF bild;
        bild.reserve(zug.size());
        for (const QPointF& punkt : zug) bild << QPointF(mitte.x() + punkt.x() * halb, mitte.y() - punkt.y() * halb);
        return bild;
    };
    QColor gestrichelt = pal.color(QPalette::Text);
    gestrichelt.setAlpha(150);
    p.setPen(QPen(gestrichelt, 1.0, Qt::DashLine));
    p.drawPolyline(imBild(m_zeichnung));
    p.setPen(QPen(pal.color(QPalette::Highlight), 1.8));
    p.drawPolyline(imBild(m_umriss));
    // der Pfeil fuer Angle: aus dieser Richtung wird der Ton abgenommen (0° nach rechts, 90° nach oben)
    const double winkel = m_winkelGrad * 3.14159265358979323846 / 180.0;
    const QPointF richtung(std::cos(winkel), -std::sin(winkel));
    const QPointF spitze = mitte + richtung * (halb + rand - 3.0);
    p.setPen(QPen(kGriffRot, 1.6));
    p.drawLine(mitte, spitze);
    const QPointF quer(-richtung.y(), richtung.x());
    p.setBrush(kGriffRot);
    p.drawPolygon(QPolygonF{spitze, spitze - richtung * 9.0 + quer * 4.0, spitze - richtung * 9.0 - quer * 4.0});
}

void InstrumentCreator::zeigeSampleWahl()
{
    const QStringList eigene = eigeneSamples();
    const QStringList bibliothek = bibliotheksKlaenge();  // die mitgelieferten (`lib:`, S123)
    if (eigene.isEmpty() && bibliothek.isEmpty())
    {
        frageNachSampleDatei();
        return;
    }
    QMenu menue(this);
    const auto gruppe = [this, &menue](const QString& titel, const QStringList& nennungen) {
        if (nennungen.isEmpty()) return;
        menue.addSection(titel);
        for (const QString& nennung : nennungen)
            connect(menue.addAction(sampleTitel(nennung)), &QAction::triggered, this, [this, nennung] { setzeSample(nennung); });
    };
    gruppe(tr("Samples"), bibliothek);
    gruppe(tr("My samples"), eigene);
    menue.addSeparator();
    connect(menue.addAction(tr("From file...")), &QAction::triggered, this, &InstrumentCreator::frageNachSampleDatei);
    menue.exec(m_sampleWahl->mapToGlobal(QPoint(0, m_sampleWahl->height())));
}

void InstrumentCreator::frageNachSampleDatei()
{
    QSettings einstellungen;
    QStringList muster;
    for (const QString& endung : sampleEndungen()) muster << QStringLiteral("*.") + endung;
    const QString pfad = QFileDialog::getOpenFileName(this, tr("Choose a sample"),
                                                      einstellungen.value(QLatin1String(kLetzterSampleOrdner)).toString(),
                                                      tr("Sound files (%1)").arg(muster.join(QLatin1Char(' '))));
    if (pfad.isEmpty()) return;
    einstellungen.setValue(QLatin1String(kLetzterSampleOrdner), QFileInfo(pfad).absolutePath());

    // Entscheid S2 a: kopieren oder am Ort lassen — gefragt wird, bis die Wahl gemerkt ist
    // (dieselbe Frage stellt der Drum-Editor, `SampleFrage.hpp`)
    const SampleKopie wahl = waehleSampleKopie(this, pfad, tr("Without a copy the instrument names the file at"),
                                               QStringLiteral("creatorSampleCopyDialog"));
    if (wahl == SampleKopie::Fragen) return;  // abgebrochen
    if (!nimmSampleDatei(pfad, wahl == SampleKopie::Kopieren))
        QMessageBox::warning(this, tr("Sample"), tr("Could not copy %1 to My samples").arg(QDir::toNativeSeparators(pfad)));
}

void InstrumentCreator::zeigeBilder()
{
    // Die Bilder zeigen eine Stimme ohne Vibrato: mehrere verstimmte Stimmen
    // schweben, und im Bild saehe man die Schwebung statt Wellenzug und Huellkurve.
    // Die Linie ist die Summe aller eingeschalteten Schichten; bei mehreren
    // kommt die gewaehlte Schicht gestrichelt dazu, im selben Massstab.
    const Instrument bild = fuerBild(false);
    const bool mitGewaehlter = m_instrument.schichten.size() > 1;
    const Instrument einzeln = mitGewaehlter ? fuerBild(true) : Instrument{};
    const auto teile = [](std::vector<float>& werte, float durch) {
        if (durch > 0.0f)
            for (float& wert : werte) wert /= durch;
    };
    // --- eine Sample-Schicht ist gewaehlt: oben die Aufnahme selbst, mit den Griffen fuer
    //     Sample start (rot) und den Sustain loop (blau)
    const Schicht& gewaehlte = schicht();
    const bool sample = gewaehlte.schwingung == Schwingung::Sample;
    m_bildHinweis->setVisible(sample);
    // --- eine Shape-Schicht ist gewaehlt: unter One cycle die Zeichnung, die Form und Angle
    const bool form = gewaehlte.schwingung == Schwingung::Form;
    m_formBildTitel->setVisible(form);
    m_formBild->setVisible(form);
    m_formLegende->setVisible(form);
    if (form)
    {
        const double teiltoene = std::clamp(gewaehlte.harmonische, 1.0, kFormHarmonischeMax);
        m_formBild->setForm(shape::tonformFuer(gewaehlte.form, gewaehlte.formKontur), teiltoene, gewaehlte.winkelGrad);
        m_formLegende->setText(tr("Dashed: the drawing · solid: with %1 harmonics · arrow: Angle")
                                   .arg(teiltoene < 10.0 ? QString::number(teiltoene, 'f', 1) : QString::number(gerundet(teiltoene))));
    }
    if (sample)
    {
        const std::shared_ptr<const SampleDaten> daten = holeSample(gewaehlte.sample);
        std::vector<float> werte;
        double start = -1.0;
        double von = -1.0;
        double bis = -1.0;
        if (daten != nullptr && daten->klang.frames() > 1)
        {
            // je Stueck der groesste Betrag beider Seiten
            constexpr std::int64_t kStuecke = 400;
            const std::int64_t frames = daten->klang.frames();
            werte.assign(static_cast<std::size_t>(kStuecke), 0.0f);
            float groesster = 0.0f;
            for (std::int64_t f = 0; f < frames; ++f)
            {
                const auto stueck = static_cast<std::size_t>(f * kStuecke / frames);
                const float wert = std::max(std::abs(daten->klang.daten[static_cast<std::size_t>(2 * f)]),
                                            std::abs(daten->klang.daten[static_cast<std::size_t>(2 * f + 1)]));
                werte[stueck] = std::max(werte[stueck], wert);
                groesster = std::max(groesster, wert);
            }
            teile(werte, groesster);
            const double laengeMs = 1000.0 * static_cast<double>(frames) / daten->rate;
            start = std::clamp(gewaehlte.sampleStartMs / laengeMs, 0.0, 1.0);
            if (gewaehlte.schleife && gewaehlte.schleifeBisMs > gewaehlte.schleifeVonMs)
            {
                von = std::clamp(gewaehlte.schleifeVonMs / laengeMs, 0.0, 1.0);
                bis = std::clamp(gewaehlte.schleifeBisMs / laengeMs, 0.0, 1.0);
            }
        }
        m_wellenTitel->setText(daten != nullptr           ? tr("The sample")
                               : gewaehlte.sample.isEmpty() ? tr("The sample — choose a file")
                                                            : tr("The sample (not found)"));
        m_wellenBild->setGespiegelt(true);
        m_wellenBild->setWerte(std::move(werte), false);
        m_wellenBild->setZweite({});
        m_wellenBild->setGriffe(start, von, bis);
    }
    else
    {
        // Rauschen hat keine Schwingung, die sich wiederholt: das Bild zeigt ein kurzes Stueck davon
        m_wellenTitel->setText(gewaehlte.schwingung == Schwingung::Rauschen ? tr("Noise") : tr("One cycle"));
        m_wellenBild->setGespiegelt(false);
        m_wellenBild->setGriffe(-1.0, -1.0, -1.0);
    }
    // --- eine Schwingung: aus einem gerechneten Ton (C3), nach dem Einsatz
    if (!sample)
    {
        const int hoehe = 48;
        const Klang ton = rendereTon(bild, hoehe, 400.0, kBildRate);
        const auto periode = static_cast<std::int64_t>(std::lround(kBildRate / tonHz(hoehe)));
        // hinter Einsatz und Anstieg der spaetesten Schicht, aber noch in der Note
        double hinterMs = 0.0;
        for (const Schicht& s : m_instrument.schichten)
            if (s.an) hinterMs = std::max(hinterMs, s.einsatzMs + std::min(s.anstiegMs, 250.0));
        const std::int64_t ab = std::min<std::int64_t>(static_cast<std::int64_t>((hinterMs + 20.0) * kBildRate / 1000.0),
                                                       std::max<std::int64_t>(0, ton.frames() - periode - 1));
        const auto ausschnitt = [ab, periode](const Klang& klang) {
            std::vector<float> werte;
            for (std::int64_t f = ab; f < ab + periode; ++f)
                werte.push_back(f < klang.frames() ? klang.daten[static_cast<std::size_t>(2 * f)] : 0.0f);
            return werte;
        };
        std::vector<float> werte = ausschnitt(ton);
        float groesster = 0.0f;
        for (const float wert : werte) groesster = std::max(groesster, std::abs(wert));
        std::vector<float> zweite;
        if (mitGewaehlter)
        {
            zweite = ausschnitt(rendereTon(einzeln, hoehe, 400.0, kBildRate));
            for (const float wert : zweite) groesster = std::max(groesster, std::abs(wert));
            teile(zweite, groesster);
        }
        teile(werte, groesster);
        m_wellenBild->setWerte(std::move(werte), false);
        m_wellenBild->setZweite(std::move(zweite));
    }
    // --- die Huellkurve einer Note (C4, 600 ms): der groesste Betrag je Stueck
    {
        const double dauerMs = 600.0;
        const Klang ton = rendereTon(bild, 60, dauerMs, kBildRate);
        constexpr int kStuecke = 160;
        const std::int64_t frames = ton.frames();
        const auto stuecke = [frames](const Klang& klang) {
            std::vector<float> werte(kStuecke, 0.0f);
            for (std::int64_t f = 0; f < klang.frames() && f < frames; ++f)
            {
                const auto stueck = static_cast<std::size_t>(f * kStuecke / std::max<std::int64_t>(1, frames));
                werte[stueck] = std::max(werte[stueck], std::abs(klang.daten[static_cast<std::size_t>(2 * f)]));
            }
            return werte;
        };
        std::vector<float> werte = stuecke(ton);
        float groesster = 0.0f;
        for (const float wert : werte) groesster = std::max(groesster, wert);
        std::vector<float> zweite;
        if (mitGewaehlter)
        {
            zweite = stuecke(rendereTon(einzeln, 60, dauerMs, kBildRate));
            for (const float wert : zweite) groesster = std::max(groesster, wert);
            teile(zweite, groesster);
        }
        // Peak dagegen gilt fuer den Ton, wie er klingt — mit allen Stimmen
        float spitze = 0.0f;
        for (const float wert : rendereTon(m_instrument, 60, dauerMs, kBildRate).daten)
            spitze = std::max(spitze, std::abs(wert));
        m_spitze->setText(QString::number(spitze, 'f', 2));
        teile(werte, groesster);
        const double ende = frames > 0 ? dauerMs * kBildRate / 1000.0 / static_cast<double>(frames) : -1.0;
        m_huellBild->setWerte(std::move(werte), true, ende, tr("note ends"));
        m_huellBild->setZweite(std::move(zweite));
    }
}

}  // namespace lumi::loop
