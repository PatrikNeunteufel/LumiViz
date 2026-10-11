/**
 * @file InstrumentCreator.cpp
 * @brief Loop: der Instrument-Creator (Stufe 10b)
 */

#include "loop/InstrumentCreator.hpp"

#include <QCloseEvent>
#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QPushButton>
#include <QSettings>
#include <QSignalBlocker>
#include <QSlider>
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
    if (m_werte.size() < 2) return;
    QPolygonF linie;
    for (std::size_t i = 0; i < m_werte.size(); ++i)
    {
        const double x = rand + breite * static_cast<double>(i) / static_cast<double>(m_werte.size() - 1);
        const double wert = std::clamp(static_cast<double>(m_werte[i]), -1.0, 1.0);
        linie << QPointF(x, m_nurOben ? null - wert * hoch : null - wert * hoch / 2.0);
    }
    p.setPen(QPen(pal.color(QPalette::Highlight), 1.8));
    p.drawPolyline(linie);
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
                                                     int von, int bis, std::function<int(const Instrument&)> lese,
                                                     std::function<void(Instrument&, int)> schreibe,
                                                     std::function<QString(const Instrument&)> text)
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
        aendere([&](Instrument& i) { schreibe(i, wert); });
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
    kopf->addWidget(new QLabel(tr("Peak"), this));
    kopf->addWidget(m_spitze);
    aussen->addLayout(kopf);

    auto* koerper = new QHBoxLayout;
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
        aendere([welle](Instrument& i) { i.schwingung = welle; });
    });
    regler(gitter, tr("Voices"), "creatorVoices", 1, 3, [](const Instrument& i) { return i.stimmen; },
           [](Instrument& i, int w) { i.stimmen = w; }, [](const Instrument& i) { return QString::number(i.stimmen); });
    m_nichtSaite.push_back(m_regler.size() - 1);
    regler(gitter, tr("Detune"), "creatorDetune", 0, 50, [](const Instrument& i) { return gerundet(i.verstimmungCent); },
           [](Instrument& i, int w) { i.verstimmungCent = w; },
           [](const Instrument& i) { return QStringLiteral("%1 ct").arg(gerundet(i.verstimmungCent)); });
    m_nichtSaite.push_back(m_regler.size() - 1);
    for (int k = 0; k < kObertonRegler; ++k)
    {
        const auto platz = static_cast<std::size_t>(k);
        const auto anteil = [platz](const Instrument& i) { return platz < i.obertoene.size() ? i.obertoene[platz] : 0.0; };
        const QByteArray kennung = QByteArrayLiteral("creatorHarmonic") + QByteArray::number(k + 2);
        regler(gitter, tr("Harmonic %1").arg(k + 2), kennung.constData(), 0, 100,
               [anteil](const Instrument& i) { return gerundet(anteil(i) * 100.0); },
               [platz](Instrument& i, int w) {
                   if (i.obertoene.size() <= platz) i.obertoene.resize(platz + 1, 0.0);
                   i.obertoene[platz] = w / 100.0;
               },
               [anteil](const Instrument& i) { return QStringLiteral("%1 %").arg(gerundet(anteil(i) * 100.0)); });
        m_nurSinus.push_back(m_regler.size() - 1);
    }
    regler(gitter, tr("Damping"), "creatorDamping", 0, 100,
           [](const Instrument& i) {
               return gerundet((i.saiteDaempfung - kDaempfungVon) / (kDaempfungBis - kDaempfungVon) * 100.0);
           },
           [](Instrument& i, int w) { i.saiteDaempfung = kDaempfungVon + (kDaempfungBis - kDaempfungVon) * w / 100.0; },
           [](const Instrument& i) { return QString::number(i.saiteDaempfung, 'f', 4); });
    m_daempfung = m_regler.size() - 1;

    // --- Envelope
    titel(tr("Envelope"));
    regler(gitter, tr("Attack"), "creatorAttack", 0, 1000, [](const Instrument& i) { return gerundet(i.anstiegMs); },
           [](Instrument& i, int w) { i.anstiegMs = w; }, [ms](const Instrument& i) { return ms(i.anstiegMs); });
    regler(gitter, tr("Decay"), "creatorDecay", 0, 500, [](const Instrument& i) { return gerundet(i.abfallMs / 10.0); },
           [](Instrument& i, int w) { i.abfallMs = w * 10.0; }, [ms](const Instrument& i) { return ms(i.abfallMs); });
    m_nichtSaite.push_back(m_regler.size() - 1);
    regler(gitter, tr("Sustain"), "creatorSustain", 0, 100, [](const Instrument& i) { return gerundet(i.halten * 100.0); },
           [](Instrument& i, int w) { i.halten = w / 100.0; },
           [](const Instrument& i) { return QStringLiteral("%1 %").arg(gerundet(i.halten * 100.0)); });
    m_nichtSaite.push_back(m_regler.size() - 1);
    regler(gitter, tr("Release"), "creatorRelease", 0, 300, [](const Instrument& i) { return gerundet(i.ausklangMs / 10.0); },
           [](Instrument& i, int w) { i.ausklangMs = w * 10.0; }, [ms](const Instrument& i) { return ms(i.ausklangMs); });

    // --- Filter
    titel(tr("Filter"));
    regler(gitter, tr("Cutoff"), "creatorCutoff", 0, 320, [](const Instrument& i) { return gerundet(i.filterTeilton * 10.0); },
           [](Instrument& i, int w) { i.filterTeilton = w / 10.0; },
           [this](const Instrument& i) {
               return i.filterTeilton <= 0.0 ? tr("open") : QStringLiteral("× %1").arg(i.filterTeilton, 0, 'f', 1);
           });
    m_nichtSaite.push_back(m_regler.size() - 1);
    regler(gitter, tr("Envelope"), "creatorFilterEnvelope", 0, 320,
           [](const Instrument& i) { return gerundet(i.filterHuelle * 10.0); },
           [](Instrument& i, int w) { i.filterHuelle = w / 10.0; },
           [](const Instrument& i) { return QStringLiteral("+ %1").arg(i.filterHuelle, 0, 'f', 1); });
    m_nichtSaite.push_back(m_regler.size() - 1);

    // --- Vibrato
    titel(tr("Vibrato"));
    regler(gitter, tr("Rate"), "creatorVibratoRate", 0, 120, [](const Instrument& i) { return gerundet(i.vibratoHz * 10.0); },
           [](Instrument& i, int w) { i.vibratoHz = w / 10.0; },
           [](const Instrument& i) { return QStringLiteral("%1 Hz").arg(i.vibratoHz, 0, 'f', 1); });
    m_nichtSaite.push_back(m_regler.size() - 1);
    regler(gitter, tr("Depth"), "creatorVibratoDepth", 0, 50, [](const Instrument& i) { return gerundet(i.vibratoCent); },
           [](Instrument& i, int w) { i.vibratoCent = w; },
           [](const Instrument& i) { return QStringLiteral("%1 ct").arg(gerundet(i.vibratoCent)); });
    m_nichtSaite.push_back(m_regler.size() - 1);
    regler(gitter, tr("Delay"), "creatorVibratoDelay", 0, 100,
           [](const Instrument& i) { return gerundet(i.vibratoEinsatzMs / 10.0); },
           [](Instrument& i, int w) { i.vibratoEinsatzMs = w * 10.0; },
           [ms](const Instrument& i) { return ms(i.vibratoEinsatzMs); });
    m_nichtSaite.push_back(m_regler.size() - 1);
    regler(gitter, tr("Level"), "creatorLevel", 0, 100, [](const Instrument& i) { return gerundet(i.laut * 100.0); },
           [](Instrument& i, int w) { i.laut = w / 100.0; },
           [](const Instrument& i) { return QStringLiteral("%1 %").arg(gerundet(i.laut * 100.0)); });
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
    rechts->addWidget(new QLabel(tr("Click a key to hear it"), this));
    m_tasten = new Tastenleiste(this);
    rechts->addWidget(m_tasten);
    rechts->addStretch(1);
    koerper->addLayout(rechts, 4);
    aussen->addLayout(koerper, 1);

    connect(m_titel, &QLineEdit::editingFinished, this, [this] {
        const QString titel = m_titel->text().trimmed();
        if (!titel.isEmpty() && titel != m_instrument.titel) aendere([&titel](Instrument& i) { i.titel = titel; });
    });
    connect(m_zurueck, &QPushButton::clicked, this, &InstrumentCreator::setzeZurueck);
    connect(m_speichern, &QPushButton::clicked, this, [this] {
        if (!speichere())
            QMessageBox::warning(this, tr("Instrument"), tr("Could not write %1").arg(QDir::toNativeSeparators(m_datei)));
    });
    connect(m_speichernUnter, &QPushButton::clicked, this, &InstrumentCreator::frageNachDatei);
    connect(m_tasten, &Tastenleiste::taste, this, [this](int hoehe) { (void)spieleAn(hoehe); });
    resize(760, 620);
}

void InstrumentCreator::oeffne(const Instrument& instrument)
{
    m_instrument = instrument;
    m_vorlage = instrument;
    if (istInstrumentDatei(instrument.name))
    {
        m_datei = instrument.name;
        m_herkunft = QFileInfo(m_datei).fileName();
    }
    else
    {
        // ein mitgeliefertes: es bleibt, wie es ist — gespeichert wird eine eigene Datei
        m_datei.clear();
        m_herkunft = tr("based on %1").arg(instrument.titel);
    }
    zeigeAlles();
}

bool InstrumentCreator::geaendert() const
{
    return !(m_instrument == m_vorlage);
}

void InstrumentCreator::aendere(const std::function<void(Instrument&)>& tu)
{
    Instrument neu = m_instrument;
    tu(neu);
    if (neu == m_instrument) return;
    m_instrument = std::move(neu);
    zeigeAlles();
}

void InstrumentCreator::setzeZurueck()
{
    if (!geaendert()) return;
    m_instrument = m_vorlage;
    zeigeAlles();
}

bool InstrumentCreator::speichere()
{
    return !m_datei.isEmpty() && speichereUnter(m_datei);
}

bool InstrumentCreator::speichereUnter(const QString& pfad)
{
    if (pfad.isEmpty() || !speichereInstrument(m_instrument, pfad)) return false;
    m_datei = pfad;
    m_instrument.name = pfad;  // es heisst jetzt wie seine Datei
    m_vorlage = m_instrument;
    m_herkunft = QFileInfo(pfad).fileName();
    zeigeAlles();
    Q_EMIT gespeichert(pfad);
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
    if (!speichereInstrument(m_instrument, datei)) return false;
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
                frageNachDatei();
            else
                (void)speichere();
            if (geaendert())  // abgebrochen oder nicht geschrieben: das Fenster bleibt
            {
                event->ignore();
                return;
            }
        }
        else
            m_instrument = m_vorlage;
    }
    QWidget::closeEvent(event);
}

void InstrumentCreator::zeigeAlles()
{
    setWindowTitle(tr("Instrument: %1").arg(m_instrument.titel) + (geaendert() ? QStringLiteral(" *") : QString()));
    if (!m_titel->hasFocus() || m_titel->text().trimmed().isEmpty()) m_titel->setText(m_instrument.titel);
    m_herkunftZeile->setText(m_herkunft);
    m_herkunftZeile->setToolTip(m_datei.isEmpty() ? QString() : QDir::toNativeSeparators(m_datei));
    m_zurueck->setEnabled(geaendert());
    m_speichern->setEnabled(!m_datei.isEmpty() && geaendert());
    m_speichern->setToolTip(m_datei.isEmpty() ? tr("A built-in instrument cannot be overwritten — use Save as...")
                                              : QDir::toNativeSeparators(m_datei));
    {
        const QSignalBlocker ruhe(m_welle);
        m_welle->setCurrentIndex(m_welle->findData(static_cast<int>(m_instrument.schwingung)));
    }
    for (Regler& r : m_regler)
    {
        // den Regler nur setzen, wenn er etwas anderes zeigt, als der Wert verlangt —
        // ein Wert zwischen zwei Stellungen bleibt, bis jemand am Regler zieht
        const QSignalBlocker ruhe(r.schieber);
        r.schieber->setValue(std::clamp(r.lese(m_instrument), r.schieber->minimum(), r.schieber->maximum()));
        r.wert->setText(r.text(m_instrument));
    }
    // was die Schwingung nicht kennt, bleibt weg
    const bool saite = m_instrument.schwingung == Schwingung::Saite;
    const bool sinus = m_instrument.schwingung == Schwingung::Sinus;
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
    Instrument bild = m_instrument;
    bild.stimmen = 1;
    bild.vibratoCent = 0.0;
    // --- eine Schwingung: aus einem gerechneten Ton (C3), nach dem Einsatz
    {
        const int hoehe = 48;
        const Klang ton = rendereTon(bild, hoehe, 400.0, kBildRate);
        const auto periode = static_cast<std::int64_t>(std::lround(kBildRate / tonHz(hoehe)));
        // hinter dem Anstieg, aber noch in der Note
        const std::int64_t ab = std::min<std::int64_t>(
            static_cast<std::int64_t>((std::min(m_instrument.anstiegMs, 250.0) + 20.0) * kBildRate / 1000.0),
            std::max<std::int64_t>(0, ton.frames() - periode - 1));
        std::vector<float> werte;
        float groesster = 0.0f;
        for (std::int64_t f = ab; f < ab + periode && f < ton.frames(); ++f)
        {
            werte.push_back(ton.daten[static_cast<std::size_t>(2 * f)]);
            groesster = std::max(groesster, std::abs(werte.back()));
        }
        if (groesster > 0.0f)
            for (float& wert : werte) wert /= groesster;
        m_wellenBild->setWerte(std::move(werte), false);
    }
    // --- die Huellkurve einer Note (C4, 600 ms): der groesste Betrag je Stueck
    {
        const double dauerMs = 600.0;
        const Klang ton = rendereTon(bild, 60, dauerMs, kBildRate);
        constexpr int kStuecke = 160;
        std::vector<float> werte(kStuecke, 0.0f);
        float groesster = 0.0f;
        const std::int64_t frames = ton.frames();
        for (std::int64_t f = 0; f < frames; ++f)
        {
            const auto stueck = static_cast<std::size_t>(f * kStuecke / std::max<std::int64_t>(1, frames));
            werte[stueck] = std::max(werte[stueck], std::abs(ton.daten[static_cast<std::size_t>(2 * f)]));
        }
        for (const float wert : werte) groesster = std::max(groesster, wert);
        // Peak dagegen gilt fuer den Ton, wie er klingt — mit allen Stimmen
        float spitze = 0.0f;
        for (const float wert : rendereTon(m_instrument, 60, dauerMs, kBildRate).daten)
            spitze = std::max(spitze, std::abs(wert));
        m_spitze->setText(QString::number(spitze, 'f', 2));
        if (groesster > 0.0f)
            for (float& wert : werte) wert /= groesster;
        const double ende = frames > 0 ? dauerMs * kBildRate / 1000.0 / static_cast<double>(frames) : -1.0;
        m_huellBild->setWerte(std::move(werte), true, ende, tr("note ends"));
    }
}

}  // namespace lumi::loop
