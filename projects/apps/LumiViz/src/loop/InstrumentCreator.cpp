/**
 * @file InstrumentCreator.cpp
 * @brief Loop: der Instrument-Creator (Stufe 10b)
 */

#include "loop/InstrumentCreator.hpp"

#include <QAction>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
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
#include <QSettings>
#include <QSignalBlocker>
#include <QSlider>
#include <QStringListModel>
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
    if (m_werte.size() < 2) return;
    p.setPen(QPen(pal.color(QPalette::Highlight), 1.8));
    p.drawPolyline(linie(m_werte));
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
        p.setBrush(h == m_gedrueckt ? an : QColor(232, 232, 228));
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
        p.setBrush(h == m_gedrueckt ? an : QColor(40, 40, 44));
        p.drawRect(tastenRect(h));
    }
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
    m_liste->setToolTip(tr("Click a layer to edit it; the check mark mutes it"));
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
    gitter->setColumnStretch(1, 1);
    const auto titel = [&](const QString& text) {
        auto* zeile = new QLabel(text, this);
        QFont fett = zeile->font();
        fett.setBold(true);
        zeile->setFont(fett);
        gitter->addWidget(zeile, gitter->rowCount(), 0, 1, 3);
    };
    const auto ms = [](double wert) { return QStringLiteral("%1 ms").arg(gerundet(wert)); };

    // --- die gewaehlte Schicht: was nur sie betrifft
    m_schichtKopf = new QLabel(this);
    m_schichtKopf->setObjectName(QStringLiteral("creatorLayerTitle"));
    {
        QFont fett = m_schichtKopf->font();
        fett.setBold(true);
        m_schichtKopf->setFont(fett);
        gitter->addWidget(m_schichtKopf, gitter->rowCount(), 0, 1, 3);
        m_schichtName = new QLineEdit(this);
        m_schichtName->setObjectName(QStringLiteral("creatorLayerName"));
        const int zeile = gitter->rowCount();
        gitter->addWidget(new QLabel(tr("Name"), this), zeile, 0);
        gitter->addWidget(m_schichtName, zeile, 1, 1, 2);
    }
    regler(gitter, tr("Level"), "creatorLevel", 0, 100, [](const Schicht& i) { return gerundet(i.laut * 100.0); },
           [](Schicht& i, int w) { i.laut = w / 100.0; },
           [](const Schicht& i) { return QStringLiteral("%1 %").arg(gerundet(i.laut * 100.0)); });
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
    titel(tr("Oscillator"));
    m_welle = new QComboBox(this);
    m_welle->setObjectName(QStringLiteral("creatorWaveform"));
    m_welle->addItem(tr("Sine"), static_cast<int>(Schwingung::Sinus));
    m_welle->addItem(tr("Triangle"), static_cast<int>(Schwingung::Dreieck));
    m_welle->addItem(tr("Saw"), static_cast<int>(Schwingung::Saege));
    m_welle->addItem(tr("Square"), static_cast<int>(Schwingung::Rechteck));
    m_welle->addItem(tr("Plucked string"), static_cast<int>(Schwingung::Saite));
    {
        const int zeile = gitter->rowCount();
        gitter->addWidget(new QLabel(tr("Waveform"), this), zeile, 0);
        gitter->addWidget(m_welle, zeile, 1, 1, 2);
    }
    connect(m_welle, &QComboBox::activated, this, [this](int index) {
        const auto welle = static_cast<Schwingung>(m_welle->itemData(index).toInt());
        aendereSchicht([welle](Schicht& s) { s.schwingung = welle; });
    });
    regler(gitter, tr("Voices"), "creatorVoices", 1, 3, [](const Schicht& i) { return i.stimmen; },
           [](Schicht& i, int w) { i.stimmen = w; }, [](const Schicht& i) { return QString::number(i.stimmen); });
    m_nichtSaite.push_back(m_regler.size() - 1);
    regler(gitter, tr("Detune"), "creatorDetune", 0, 50, [](const Schicht& i) { return gerundet(i.verstimmungCent); },
           [](Schicht& i, int w) { i.verstimmungCent = w; },
           [](const Schicht& i) { return QStringLiteral("%1 ct").arg(gerundet(i.verstimmungCent)); });
    m_nichtSaite.push_back(m_regler.size() - 1);
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

    // --- Envelope
    titel(tr("Envelope"));
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

    // --- Filter
    titel(tr("Filter"));
    regler(gitter, tr("Cutoff"), "creatorCutoff", 0, 320, [](const Schicht& i) { return gerundet(i.filterTeilton * 10.0); },
           [](Schicht& i, int w) { i.filterTeilton = w / 10.0; },
           [this](const Schicht& i) {
               return i.filterTeilton <= 0.0 ? tr("open") : QStringLiteral("× %1").arg(i.filterTeilton, 0, 'f', 1);
           });
    m_nichtSaite.push_back(m_regler.size() - 1);
    regler(gitter, tr("Envelope"), "creatorFilterEnvelope", 0, 320,
           [](const Schicht& i) { return gerundet(i.filterHuelle * 10.0); },
           [](Schicht& i, int w) { i.filterHuelle = w / 10.0; },
           [](const Schicht& i) { return QStringLiteral("+ %1").arg(i.filterHuelle, 0, 'f', 1); });
    m_nichtSaite.push_back(m_regler.size() - 1);
    regler(gitter, tr("Drive"), "creatorDrive", 0, 100, [](const Schicht& i) { return gerundet(i.verzerrung * 100.0); },
           [](Schicht& i, int w) { i.verzerrung = w / 100.0; },
           [this](const Schicht& i) {
               return i.verzerrung <= 0.0 ? tr("off") : QStringLiteral("%1 %").arg(gerundet(i.verzerrung * 100.0));
           })
        .schieber->setToolTip(tr("Distortion of this layer"));

    // --- Vibrato
    titel(tr("Vibrato"));
    regler(gitter, tr("Rate"), "creatorVibratoRate", 0, 120, [](const Schicht& i) { return gerundet(i.vibratoHz * 10.0); },
           [](Schicht& i, int w) { i.vibratoHz = w / 10.0; },
           [](const Schicht& i) { return QStringLiteral("%1 Hz").arg(i.vibratoHz, 0, 'f', 1); });
    m_nichtSaite.push_back(m_regler.size() - 1);
    regler(gitter, tr("Depth"), "creatorVibratoDepth", 0, 50, [](const Schicht& i) { return gerundet(i.vibratoCent); },
           [](Schicht& i, int w) { i.vibratoCent = w; },
           [](const Schicht& i) { return QStringLiteral("%1 ct").arg(gerundet(i.vibratoCent)); });
    m_nichtSaite.push_back(m_regler.size() - 1);
    regler(gitter, tr("Delay"), "creatorVibratoDelay", 0, 100,
           [](const Schicht& i) { return gerundet(i.vibratoEinsatzMs / 10.0); },
           [](Schicht& i, int w) { i.vibratoEinsatzMs = w * 10.0; },
           [ms](const Schicht& i) { return ms(i.vibratoEinsatzMs); });
    m_nichtSaite.push_back(m_regler.size() - 1);
    gitter->setRowStretch(gitter->rowCount(), 1);
    koerper->addLayout(gitter, 5);

    // --- rechts: die Bilder und die Klaviatur
    auto* rechts = new QVBoxLayout;
    rechts->addWidget(new QLabel(tr("One cycle"), this));
    m_wellenBild = new KurvenBild(this);
    m_wellenBild->setObjectName(QStringLiteral("creatorCycle"));
    rechts->addWidget(m_wellenBild, 1);
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
    m_nurWahl = new QCheckBox(tr("Selected layer only"), this);
    m_nurWahl->setObjectName(QStringLiteral("creatorSelectedOnly"));
    m_nurWahl->setToolTip(tr("The keys play the selected layer alone, even if it is muted"));
    rechts->addWidget(m_nurWahl);
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
    connect(m_speichernUnter, &QPushButton::clicked, this, &InstrumentCreator::frageNachDatei);
    connect(m_tasten, &Tastenleiste::taste, this, [this](int hoehe) { (void)spieleAn(hoehe); });
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
    // schon umbenannt und der Name ist frei: so heisst die Kopie
    if (titel != m_vorlage.titel && !QFileInfo::exists(eigeneDatei(eigenerName(titel)))) return titel;
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

bool InstrumentCreator::frageNachNamen()
{
    QDialog dialog(this);
    dialog.setObjectName(QStringLiteral("creatorSaveDialog"));
    dialog.setWindowTitle(tr("Save instrument"));
    auto* aussen = new QVBoxLayout(&dialog);
    auto* satz = new QLabel(
        m_vorlage.name.isEmpty()
            ? tr("Your instrument is saved to My files and used by this pattern.")
            : tr("%1 is a built-in instrument. Your copy is saved to My files and used by this pattern; "
                 "the built-in one stays as it is.")
                  .arg(m_vorlage.titel),
        &dialog);
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

void InstrumentCreator::frageNachDatei()
{
    QSettings einstellungen;
    const QString vorschlag =
        !m_datei.isEmpty() ? m_datei
                           : einstellungen.value(QLatin1String(kLetzterOrdner)).toString() + QLatin1Char('/') +
                                 m_instrument.titel.toLower().replace(QLatin1Char(' '), QLatin1Char('_')) +
                                 QStringLiteral(".lvinst");
    QString pfad = QFileDialog::getSaveFileName(this, tr("Save instrument"), vorschlag, tr("Instrument (*.lvinst)"));
    if (pfad.isEmpty()) return;
    if (!pfad.endsWith(QStringLiteral(".lvinst"), Qt::CaseInsensitive)) pfad += QStringLiteral(".lvinst");
    einstellungen.setValue(QLatin1String(kLetzterOrdner), QFileInfo(pfad).absolutePath());
    if (!speichereUnter(pfad))
        QMessageBox::warning(this, tr("Instrument"), tr("Could not write %1").arg(QDir::toNativeSeparators(pfad)));
}

bool InstrumentCreator::spieleAn(int hoehe)
{
    // die Kopie, wie sie gerade steht — auch ungespeichert — in die Vorhoer-Datei
    const QString datei = vorhoerDatei();
    Instrument zuHoeren = m_instrument;
    if (m_nurGewaehlte)
    {
        // die gewaehlte Schicht allein — auch wenn sie in der Liste stumm ist
        zuHoeren.schichten = {schicht()};
        zuHoeren.schichten[0].an = true;
    }
    if (!speichereInstrument(zuHoeren, datei)) return false;
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
    for (Regler& r : m_regler)
    {
        // den Regler nur setzen, wenn er etwas anderes zeigt, als der Wert verlangt —
        // ein Wert zwischen zwei Stellungen bleibt, bis jemand am Regler zieht
        const QSignalBlocker ruhe(r.schieber);
        r.schieber->setValue(std::clamp(r.lese(gewaehlte), r.schieber->minimum(), r.schieber->maximum()));
        r.wert->setText(r.text(gewaehlte));
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
    for (const std::size_t platz : m_nichtSaite) zeige(platz, !saite);
    for (const std::size_t platz : m_nurSinus) zeige(platz, sinus);
    zeige(m_daempfung, saite);
    zeigeBilder();
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
    // --- eine Schwingung: aus einem gerechneten Ton (C3), nach dem Einsatz
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
