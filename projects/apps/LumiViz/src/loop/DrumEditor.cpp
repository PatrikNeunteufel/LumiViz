/**
 * @file DrumEditor.cpp
 * @brief Loop: der Drum-Editor — das Schritt-Raster einer Basis (Stufe 6b)
 */

#include "loop/DrumEditor.hpp"

#include "loop/Bearbeiten.hpp"

#include <QComboBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSettings>
#include <QSignalBlocker>
#include <QSlider>
#include <QSpinBox>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

namespace lumi::loop
{

namespace
{

/// Der Eintrag der Klang-Auswahl, der nach einer Sample-Datei fragt.
const QString kDateiWahl = QStringLiteral("?file");
const char* const kSampleOrdner = "loop/lastSampleDir";
const char* const kSpurNr = "spur";
constexpr int kSpaltenBreite = 316;  ///< die Kopfzeilen der Spuren, links vom Raster
constexpr int kRand = 4;             ///< vor dem ersten und hinter dem letzten Schritt

}  // namespace

// =============================================================================
// SchrittRaster
// =============================================================================

SchrittRaster::SchrittRaster(QWidget* parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("loopSteps"));
}

void SchrittRaster::setMuster(const Muster& muster, int schlaegeJeTakt, int schritteJeSchlag)
{
    m_muster = muster;
    m_schlaege = std::max(1, schlaegeJeTakt);
    m_jeSchlag = std::max(1, schritteJeSchlag);
    m_zug = Zug::Keiner;
    if (m_spur >= static_cast<int>(m_muster.spuren.size())) m_spur = static_cast<int>(m_muster.spuren.size()) - 1;
    if (m_spur < 0 && !m_muster.spuren.empty()) m_spur = 0;
    resize(sizeHint());
    updateGeometry();
    update();
}

void SchrittRaster::setSpur(int spur)
{
    if (spur < 0 || spur >= static_cast<int>(m_muster.spuren.size()) || spur == m_spur) return;
    m_spur = spur;
    update();
}

void SchrittRaster::setMarke(double schritt)
{
    if (schritt == m_marke) return;
    m_marke = schritt;
    update();
}

int SchrittRaster::schritte() const
{
    return m_muster.takte * m_schlaege * m_jeSchlag;
}

int SchrittRaster::xVonSchritt(int schritt) const
{
    return kRand + schritt * (kSchrittBreite + kSchrittLuft) + (schritt / m_jeSchlag) * kSchlagLuft;
}

int SchrittRaster::schrittBei(int x) const
{
    const int schlagBreite = m_jeSchlag * (kSchrittBreite + kSchrittLuft) + kSchlagLuft;
    if (x < kRand) return -1;
    const int schlag = (x - kRand) / schlagBreite;
    const int rest = (x - kRand) - schlag * schlagBreite;
    const int teil = rest / (kSchrittBreite + kSchrittLuft);
    if (teil >= m_jeSchlag || rest % (kSchrittBreite + kSchrittLuft) >= kSchrittBreite) return -1;  // eine Luecke
    const int schritt = schlag * m_jeSchlag + teil;
    return schritt < schritte() ? schritt : -1;
}

int SchrittRaster::lautOben() const
{
    return kKopfHoehe + static_cast<int>(m_muster.spuren.size()) * kZeilenHoehe + kLuftHoehe;
}

QSize SchrittRaster::sizeHint() const
{
    return {xVonSchritt(schritte()) + kRand, lautOben() + kLautHoehe};
}

void SchrittRaster::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    const QPalette pal = palette();
    p.fillRect(rect(), pal.color(QPalette::Base));
    const QColor text = pal.color(QPalette::Text);
    QColor leise = text;
    leise.setAlpha(130);
    QColor zelle = text;
    QColor an = pal.color(QPalette::Highlight);
    const int zahl = schritte();
    const int jeTakt = m_schlaege * m_jeSchlag;
    const int spuren = static_cast<int>(m_muster.spuren.size());

    // --- der Kopf: die Takte
    QFont klein = font();
    klein.setPointSizeF(std::max(6.5, klein.pointSizeF() - 1.0));
    p.setFont(klein);
    p.setPen(leise);
    for (int takt = 0; takt < m_muster.takte; ++takt)
        p.drawText(QRect(xVonSchritt(takt * jeTakt), 0, 80, kKopfHoehe), Qt::AlignLeft | Qt::AlignVCenter,
                   tr("Bar %1").arg(takt + 1));
    p.setFont(font());

    // --- die angeklickte Spur: ihre Zeile leicht hervorgehoben
    if (m_spur >= 0 && m_spur < spuren)
    {
        QColor zeile = an;
        zeile.setAlpha(28);
        p.fillRect(QRect(0, yVonSpur(m_spur), width(), kZeilenHoehe), zeile);
    }

    // --- die Schritte: jeder zweite Schlag heller; ein gesetzter Schritt so
    //     kraeftig, wie er laut ist
    p.setPen(Qt::NoPen);
    for (int s = 0; s < spuren; ++s)
    {
        const std::vector<double>& zeile = m_muster.zeilen[static_cast<std::size_t>(s)];
        const bool stumm = m_muster.spuren[static_cast<std::size_t>(s)].stumm;
        for (int i = 0; i < zahl && i < static_cast<int>(zeile.size()); ++i)
        {
            const QRect r(xVonSchritt(i), yVonSpur(s) + 4, kSchrittBreite, kZeilenHoehe - 8);
            const double wert = zeile[static_cast<std::size_t>(i)];
            if (wert > 0.0)
            {
                QColor farbe = stumm ? text : an;
                farbe.setAlphaF((stumm ? 0.2 : 0.35) + (stumm ? 0.25 : 0.65) * wert);
                p.setBrush(farbe);
            }
            else
            {
                zelle.setAlpha((i / m_jeSchlag) % 2 == 0 ? 58 : 24);
                p.setBrush(zelle);
            }
            p.drawRoundedRect(r, 2, 2);
        }
    }

    // --- die Lautstaerke je Schritt der angeklickten Spur
    const int oben = lautOben();
    p.setPen(leise);
    p.drawLine(0, oben + kLautHoehe - 1, width(), oben + kLautHoehe - 1);
    p.setPen(Qt::NoPen);
    if (m_spur >= 0 && m_spur < spuren)
    {
        const std::vector<double>& zeile = m_muster.zeilen[static_cast<std::size_t>(m_spur)];
        for (int i = 0; i < zahl && i < static_cast<int>(zeile.size()); ++i)
        {
            const double wert = zeile[static_cast<std::size_t>(i)];
            if (wert <= 0.0) continue;
            const int hoch = std::max(2, static_cast<int>(wert * (kLautHoehe - 6) + 0.5));
            p.setBrush(an);
            p.drawRect(QRect(xVonSchritt(i), oben + kLautHoehe - 1 - hoch, kSchrittBreite, hoch));
        }
    }

    // --- die Abspielmarke
    if (m_marke >= 0.0 && m_marke < zahl)
    {
        const int ganz = static_cast<int>(m_marke);
        const int x = xVonSchritt(ganz) + static_cast<int>((m_marke - ganz) * (kSchrittBreite + kSchrittLuft));
        p.setPen(QPen(QColor(255, 90, 70), 2.0));
        p.drawLine(x, 0, x, height());
    }
}

void SchrittRaster::maleBei(QPoint ort)
{
    const int schritt = schrittBei(ort.x());
    if (schritt < 0 || m_zugSpur < 0 || m_zugSpur >= static_cast<int>(m_muster.zeilen.size())) return;
    std::vector<double>& zeile = m_muster.zeilen[static_cast<std::size_t>(m_zugSpur)];
    if (schritt >= static_cast<int>(zeile.size())) return;
    double& wert = zeile[static_cast<std::size_t>(schritt)];
    switch (m_zug)
    {
        case Zug::Setzen:
            if (wert <= 0.0) wert = 1.0;  // ein neuer Schritt beginnt mit voller Lautstaerke
            break;
        case Zug::Loeschen:
            wert = 0.0;
            break;
        case Zug::Laut:
        {
            const double hoehe = 1.0 - static_cast<double>(ort.y() - lautOben() - 3) / (kLautHoehe - 6);
            // ganz unten ist der Schritt aus
            wert = hoehe < 0.04 ? 0.0 : std::clamp(std::round(hoehe * 100.0) / 100.0, 0.0, 1.0);
            break;
        }
        case Zug::Keiner:
            return;
    }
    update();
}

void SchrittRaster::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) return;
    const QPoint ort = event->position().toPoint();
    const int spuren = static_cast<int>(m_muster.spuren.size());
    if (ort.y() >= lautOben())
    {
        if (m_spur < 0 || m_spur >= spuren) return;
        m_zug = Zug::Laut;
        m_zugSpur = m_spur;
        m_vorZug = m_muster;
        maleBei(ort);
        return;
    }
    if (ort.y() < kKopfHoehe) return;
    const int spur = (ort.y() - kKopfHoehe) / kZeilenHoehe;
    if (spur >= spuren) return;
    if (spur != m_spur)
    {
        m_spur = spur;
        update();
        Q_EMIT spurGewaehlt(spur);
    }
    const int schritt = schrittBei(ort.x());
    if (schritt < 0) return;
    const std::vector<double>& zeile = m_muster.zeilen[static_cast<std::size_t>(spur)];
    if (schritt >= static_cast<int>(zeile.size())) return;
    m_zug = zeile[static_cast<std::size_t>(schritt)] > 0.0 ? Zug::Loeschen : Zug::Setzen;
    m_zugSpur = spur;
    m_vorZug = m_muster;
    maleBei(ort);
}

void SchrittRaster::mouseMoveEvent(QMouseEvent* event)
{
    if (m_zug != Zug::Keiner && (event->buttons() & Qt::LeftButton)) maleBei(event->position().toPoint());
}

void SchrittRaster::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton || m_zug == Zug::Keiner) return;
    const Zug zug = m_zug;
    m_zug = Zug::Keiner;
    if (m_muster == m_vorZug) return;
    Q_EMIT geaendert(m_muster, zug == Zug::Laut ? tr("Change step volume") : tr("Edit steps"));
}

// =============================================================================
// DrumEditor
// =============================================================================

DrumEditor::DrumEditor(QWidget* parent) : QWidget(parent, Qt::Tool)
{
    setObjectName(QStringLiteral("loopDrumEditor"));
    setupUI();
}

void DrumEditor::setupUI()
{
    auto* aussen = new QVBoxLayout(this);

    auto* kopf = new QHBoxLayout;
    m_basisName = new QLabel(this);
    m_basisName->setObjectName(QStringLiteral("drumBase"));
    m_nutzer = new QLabel(this);
    m_nutzer->setObjectName(QStringLiteral("drumUsers"));
    m_nutzer->setEnabled(false);  // grau: eine Auskunft
    m_takte = new QSpinBox(this);
    m_takte->setObjectName(QStringLiteral("drumBars"));
    m_takte->setRange(kTakteMin, kTakteMax);
    m_takte->setKeyboardTracking(false);
    m_takte->setToolTip(tr("Length of the pattern in bars; its clips grow and shrink with it"));
    m_play = new QPushButton(tr("Play pattern"), this);
    m_play->setObjectName(QStringLiteral("drumPlay"));
    m_play->setToolTip(tr("Play only this pattern, in a loop"));
    m_spitze = new QLabel(QStringLiteral("–"), this);
    m_spitze->setObjectName(QStringLiteral("drumPeak"));
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

    auto* raster = new QHBoxLayout;
    m_schlaege = new QSpinBox(this);
    m_schlaege->setObjectName(QStringLiteral("drumBeats"));
    m_schlaege->setRange(1, kRasterMax);
    m_schlaege->setKeyboardTracking(false);
    m_jeSchlag = new QSpinBox(this);
    m_jeSchlag->setObjectName(QStringLiteral("drumStepsPerBeat"));
    m_jeSchlag->setRange(1, kRasterMax);
    m_jeSchlag->setKeyboardTracking(false);
    m_swing = new QDoubleSpinBox(this);
    m_swing->setObjectName(QStringLiteral("drumSwing"));
    m_swing->setRange(0.0, 100.0);
    m_swing->setDecimals(0);
    m_swing->setSuffix(QStringLiteral(" %"));
    m_swing->setKeyboardTracking(false);
    m_swing->setToolTip(tr("Delays every second step of a beat; at 100 % by half a step"));
    raster->addWidget(new QLabel(tr("Grid of the whole loop:"), this));
    raster->addWidget(new QLabel(tr("Beats per bar"), this));
    raster->addWidget(m_schlaege);
    raster->addWidget(new QLabel(tr("Steps per beat"), this));
    raster->addWidget(m_jeSchlag);
    raster->addWidget(new QLabel(tr("Swing"), this));
    raster->addWidget(m_swing);
    raster->addStretch(1);
    aussen->addLayout(raster);

    // --- links die Kopfzeilen der Spuren, rechts das Raster; es rollt nur das Raster
    auto* koerper = new QHBoxLayout;
    koerper->setSpacing(0);
    auto* spalte = new QWidget(this);
    spalte->setFixedWidth(kSpaltenBreite);
    m_spurSpalte = new QVBoxLayout(spalte);
    m_spurSpalte->setContentsMargins(0, 0, 0, 0);
    m_spurSpalte->setSpacing(0);
    auto* titel = new QWidget(spalte);
    titel->setFixedHeight(SchrittRaster::kKopfHoehe);
    auto* titelReihe = new QHBoxLayout(titel);
    titelReihe->setContentsMargins(26, 0, 0, 0);
    titelReihe->setSpacing(6);
    const auto titelFeld = [&](const QString& text, int breite) {
        auto* feld = new QLabel(text, titel);
        feld->setEnabled(false);
        feld->setFixedWidth(breite);
        titelReihe->addWidget(feld);
    };
    titelFeld(tr("Track"), 70);
    titelFeld(tr("Sound"), 100);
    titelFeld(tr("Vol"), 48);
    titelFeld(tr("Pan"), 48);
    titelReihe->addStretch(1);
    m_spurSpalte->addWidget(titel);
    // (die Zeilen der Spuren haengt `baueSpuren` hier ein)
    auto* neuReihe = new QWidget(spalte);
    neuReihe->setFixedHeight(SchrittRaster::kLuftHoehe);
    auto* neuLayout = new QHBoxLayout(neuReihe);
    neuLayout->setContentsMargins(0, 2, 0, 2);
    m_spurNeu = new QPushButton(tr("Add track"), neuReihe);
    m_spurNeu->setObjectName(QStringLiteral("drumAddTrack"));
    neuLayout->addWidget(m_spurNeu);
    neuLayout->addStretch(1);
    m_spurSpalte->addWidget(neuReihe);
    m_lautName = new QLabel(spalte);
    m_lautName->setObjectName(QStringLiteral("drumStepVolume"));
    m_lautName->setFixedHeight(SchrittRaster::kLautHoehe);
    m_lautName->setEnabled(false);
    m_spurSpalte->addWidget(m_lautName);
    m_spurSpalte->addStretch(1);

    m_raster = new SchrittRaster;
    m_rollen = new QScrollArea(this);
    m_rollen->setFrameShape(QFrame::NoFrame);
    m_rollen->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_rollen->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);  // lange Muster rollen, sie stauchen nicht
    m_rollen->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_rollen->setWidget(m_raster);
    koerper->addWidget(spalte, 0, Qt::AlignTop);
    koerper->addWidget(m_rollen, 1);
    aussen->addLayout(koerper, 1);

    connect(m_takte, &QSpinBox::valueChanged, this, [this](int takte) { setzeTakte(takte); });
    connect(m_schlaege, &QSpinBox::valueChanged, this, [this](int schlaege) { setzeRaster(schlaege, m_jeSchlag->value()); });
    connect(m_jeSchlag, &QSpinBox::valueChanged, this, [this](int jeSchlag) { setzeRaster(m_schlaege->value(), jeSchlag); });
    connect(m_swing, &QDoubleSpinBox::valueChanged, this, [this](double prozent) { setzeSwing(prozent / 100.0); });
    connect(m_play, &QPushButton::clicked, this, [this] { Q_EMIT spielenVerlangt(!m_spielt); });
    connect(m_spurNeu, &QPushButton::clicked, this, [this] { spurHinzu(); });
    connect(m_raster, &SchrittRaster::spurGewaehlt, this, [this](int spur) { waehleSpur(spur); });
    connect(m_raster, &SchrittRaster::geaendert, this, [this](const Muster& neu, const QString& was) {
        Loop fassung = m_loop;
        Muster* ziel = musterIn(fassung, m_muster);
        if (ziel == nullptr) return;
        ziel->zeilen = neu.zeilen;
        Q_EMIT bearbeitet(fassung, was);
    });
    resize(820, 320);
}

void DrumEditor::setKitKlaenge(const QStringList& klaenge)
{
    m_kitKlaenge = klaenge;
    if (basis() != nullptr) zeigeAlles();
}

bool DrumEditor::zeige(const Loop& loop, const QString& muster)
{
    const Muster* ziel = loop.findeMuster(muster);
    if (ziel == nullptr || ziel->art != MusterArt::Drumkit) return false;
    m_loop = loop;
    m_muster = muster;
    zeigeAlles();
    return true;
}

void DrumEditor::setLoop(const Loop& loop)
{
    if (m_muster.isEmpty()) return;
    // umbenannt? Dann gibt es im neuen Loop eine Basis mit neuem Namen und demselben Inhalt
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

void DrumEditor::zeigeAlles()
{
    const Muster* ziel = basis();
    if (ziel == nullptr) return;
    setWindowTitle(tr("Drum pattern: %1").arg(m_muster));
    m_basisName->setText(m_muster);
    m_nutzer->setText(tr("· used by %1 clips").arg(basisNutzer(m_loop, m_muster)));
    {
        const QSignalBlocker a(m_takte);
        const QSignalBlocker b(m_schlaege);
        const QSignalBlocker c(m_jeSchlag);
        const QSignalBlocker d(m_swing);
        m_takte->setValue(ziel->takte);
        m_schlaege->setValue(m_loop.schlaegeJeTakt);
        m_jeSchlag->setValue(m_loop.schritteJeSchlag);
        m_swing->setValue(m_loop.swing * 100.0);
    }
    m_raster->setMuster(*ziel, m_loop.schlaegeJeTakt, m_loop.schritteJeSchlag);
    // das Raster in voller Hoehe, darunter Platz fuer den Rollbalken
    m_rollen->setFixedHeight(m_raster->sizeHint().height() + style()->pixelMetric(QStyle::PM_ScrollBarExtent) + 2);
    baueSpuren(*ziel);
    waehleSpur(m_raster->spur());
}

void DrumEditor::fuelleKlang(QComboBox* auswahl, const QString& klang) const
{
    const QSignalBlocker ruhe(auswahl);
    auswahl->clear();
    if (klang.isEmpty()) auswahl->addItem(tr("(none)"), QString());
    for (const QString& kit : m_kitKlaenge) auswahl->addItem(kit.mid(4), kit);  // ohne den Vorsatz „kit:"
    if (!klang.isEmpty() && auswahl->findData(klang) < 0)
        auswahl->addItem(istKitKlang(klang) ? klang.mid(4) : QFileInfo(klang).fileName(), klang);
    auswahl->addItem(tr("Sample file..."), kDateiWahl);
    auswahl->setCurrentIndex(auswahl->findData(klang));
    auswahl->setToolTip(istKitKlang(klang) ? klang : QDir::toNativeSeparators(klang));
}

void DrumEditor::baueSpuren(const Muster& muster)
{
    if (m_spuren.size() != muster.spuren.size())
    {
        for (SpurZeile& zeile : m_spuren) delete zeile.reihe;
        m_spuren.clear();
        for (int s = 0; s < static_cast<int>(muster.spuren.size()); ++s)
        {
            SpurZeile zeile;
            zeile.reihe = new QWidget;
            zeile.reihe->setFixedHeight(SchrittRaster::kZeilenHoehe);
            zeile.reihe->setProperty(kSpurNr, s);
            zeile.reihe->setContextMenuPolicy(Qt::CustomContextMenu);
            zeile.reihe->installEventFilter(this);
            auto* reihe = new QHBoxLayout(zeile.reihe);
            reihe->setContentsMargins(0, 0, 0, 0);
            reihe->setSpacing(6);
            zeile.stumm = new QToolButton(zeile.reihe);
            zeile.stumm->setText(QStringLiteral("M"));
            zeile.stumm->setCheckable(true);
            zeile.stumm->setFixedSize(20, 20);
            zeile.stumm->setToolTip(tr("Mute this track"));
            zeile.name = new QLabel(zeile.reihe);
            zeile.name->setFixedWidth(70);
            zeile.name->setProperty(kSpurNr, s);
            zeile.name->installEventFilter(this);
            zeile.klang = new QComboBox(zeile.reihe);
            zeile.klang->setFixedWidth(100);
            zeile.laut = new QSlider(Qt::Horizontal, zeile.reihe);
            zeile.laut->setRange(0, 100);
            zeile.laut->setFixedWidth(48);
            zeile.laut->setToolTip(tr("Volume of the track"));
            zeile.pan = new QSlider(Qt::Horizontal, zeile.reihe);
            zeile.pan->setRange(-100, 100);
            zeile.pan->setFixedWidth(48);
            zeile.pan->setToolTip(tr("Pan: left to right"));
            reihe->addWidget(zeile.stumm);
            reihe->addWidget(zeile.name);
            reihe->addWidget(zeile.klang);
            reihe->addWidget(zeile.laut);
            reihe->addWidget(zeile.pan);
            reihe->addStretch(1);
            m_spurSpalte->insertWidget(1 + s, zeile.reihe);  // hinter der Titelzeile

            connect(zeile.reihe, &QWidget::customContextMenuRequested, this,
                    [this, s, reiheWidget = zeile.reihe](QPoint ort) { spurMenue(s, reiheWidget->mapToGlobal(ort)); });
            connect(zeile.stumm, &QToolButton::toggled, this, [this, s](bool stumm) { setzeStumm(s, stumm); });
            connect(zeile.klang, &QComboBox::activated, this, [this, s, auswahl = zeile.klang](int index) {
                const QString wahl = auswahl->itemData(index).toString();
                if (wahl != kDateiWahl)
                {
                    setzeKlang(s, wahl);
                    return;
                }
                QSettings einstellungen;
                const QString pfad = QFileDialog::getOpenFileName(
                    this, tr("Choose sample"), einstellungen.value(QLatin1String(kSampleOrdner)).toString(),
                    tr("Audio (*.wav *.mp3 *.flac *.ogg);;All files (*)"));
                if (pfad.isEmpty())
                {
                    zeigeAlles();  // die Auswahl steht wieder auf dem Klang der Spur
                    return;
                }
                einstellungen.setValue(QLatin1String(kSampleOrdner), QFileInfo(pfad).absolutePath());
                setzeKlang(s, pfad);
            });
            // ein Schieber meldet beim Loslassen — nicht jeden Pixel als Schritt fuer Undo
            const auto schieber = [this, s](QSlider* regler, bool pan) {
                const auto melde = [this, s, regler, pan] {
                    if (pan)
                        setzePan(s, regler->value() / 100.0);
                    else
                        setzeLaut(s, regler->value() / 100.0);
                };
                connect(regler, &QSlider::sliderReleased, this, melde);
                connect(regler, &QSlider::valueChanged, this, [regler, melde] {
                    if (!regler->isSliderDown()) melde();
                });
            };
            schieber(zeile.laut, false);
            schieber(zeile.pan, true);
            m_spuren.push_back(zeile);
        }
    }
    for (std::size_t s = 0; s < m_spuren.size(); ++s)
    {
        const Spur& spur = muster.spuren[s];
        SpurZeile& zeile = m_spuren[s];
        const QSignalBlocker a(zeile.stumm);
        const QSignalBlocker b(zeile.laut);
        const QSignalBlocker c(zeile.pan);
        zeile.stumm->setChecked(spur.stumm);
        zeile.name->setText(zeile.name->fontMetrics().elidedText(spur.name, Qt::ElideRight, 70));
        zeile.name->setToolTip(spur.name);
        fuelleKlang(zeile.klang, spur.klang);
        if (!zeile.laut->isSliderDown()) zeile.laut->setValue(static_cast<int>(std::lround(spur.laut * 100.0)));
        if (!zeile.pan->isSliderDown()) zeile.pan->setValue(static_cast<int>(std::lround(spur.pan * 100.0)));
    }
}

void DrumEditor::waehleSpur(int spur)
{
    m_raster->setSpur(spur);
    const Muster* ziel = basis();
    const int gewaehlt = m_raster->spur();
    for (std::size_t s = 0; s < m_spuren.size(); ++s)
    {
        QFont schrift = m_spuren[s].name->font();
        schrift.setBold(static_cast<int>(s) == gewaehlt);
        m_spuren[s].name->setFont(schrift);
    }
    m_lautName->setText(ziel != nullptr && gewaehlt >= 0 && gewaehlt < static_cast<int>(ziel->spuren.size())
                            ? tr("Step volume: %1").arg(ziel->spuren[static_cast<std::size_t>(gewaehlt)].name)
                            : QString());
}

bool DrumEditor::eventFilter(QObject* objekt, QEvent* ereignis)
{
    if (ereignis->type() == QEvent::MouseButtonPress && objekt->property(kSpurNr).isValid())
        waehleSpur(objekt->property(kSpurNr).toInt());
    return QWidget::eventFilter(objekt, ereignis);
}

void DrumEditor::spurMenue(int spur, QPoint ort)
{
    const Muster* ziel = basis();
    if (ziel == nullptr || spur < 0 || spur >= static_cast<int>(ziel->spuren.size())) return;
    waehleSpur(spur);
    QMenu menu(this);
    menu.addAction(tr("Rename track..."), this, [this, spur, name = ziel->spuren[static_cast<std::size_t>(spur)].name] {
        bool ok = false;
        const QString neu = QInputDialog::getText(this, tr("Rename track"), tr("Name:"), QLineEdit::Normal, name, &ok);
        if (ok) spurUmbenennen(spur, neu);
    });
    menu.addAction(tr("Remove track"), this, [this, spur] { spurLoeschen(spur); });
    menu.exec(ort);
}

void DrumEditor::setSpielt(bool spielt, double schritt)
{
    if (spielt != m_spielt)
    {
        m_spielt = spielt;
        m_play->setText(spielt ? tr("Stop") : tr("Play pattern"));
    }
    m_raster->setMarke(spielt ? schritt : -1.0);
}

void DrumEditor::setSpitze(double spitze)
{
    const QString text = spitze < 0.0 ? QStringLiteral("–") : QString::number(spitze, 'f', 2);
    if (text == m_spitze->text()) return;
    m_spitze->setText(text);
    // ueber 1 uebersteuert die Summe (L6 a)
    m_spitze->setStyleSheet(spitze > 1.0 ? QStringLiteral("color: #e05050; font-weight: bold;") : QString());
}

// --- die Aenderungen: jede meldet eine neue Fassung des Loops ----------------

void DrumEditor::setzeTakte(int takte)
{
    Loop neu = m_loop;
    if (!setzeMusterTakte(neu, m_muster, takte) || neu == m_loop) return;
    Q_EMIT bearbeitet(neu, tr("Change pattern length"));
}

void DrumEditor::setzeRaster(int schlaegeJeTakt, int schritteJeSchlag)
{
    Loop neu = m_loop;
    if (!lumi::loop::setzeRaster(neu, schlaegeJeTakt, schritteJeSchlag)) return;
    Q_EMIT bearbeitet(neu, tr("Change grid"));
}

void DrumEditor::setzeSwing(double swing)
{
    Loop neu = m_loop;
    neu.swing = std::clamp(swing, 0.0, 1.0);
    if (neu == m_loop) return;
    Q_EMIT bearbeitet(neu, tr("Change swing"));
}

void DrumEditor::spurHinzu()
{
    Loop neu = m_loop;
    const int spur = lumi::loop::spurHinzu(neu, m_muster, m_kitKlaenge);
    if (spur < 0) return;
    Q_EMIT bearbeitet(neu, tr("Add track"));
    waehleSpur(spur);
}

void DrumEditor::spurLoeschen(int spur)
{
    Loop neu = m_loop;
    if (!lumi::loop::spurLoeschen(neu, m_muster, spur)) return;
    Q_EMIT bearbeitet(neu, tr("Remove track"));
}

void DrumEditor::spurUmbenennen(int spur, const QString& name)
{
    Loop neu = m_loop;
    Muster* ziel = musterIn(neu, m_muster);
    if (ziel == nullptr || spur < 0 || spur >= static_cast<int>(ziel->spuren.size()) || name.trimmed().isEmpty()) return;
    ziel->spuren[static_cast<std::size_t>(spur)].name = name.trimmed();
    if (neu == m_loop) return;
    Q_EMIT bearbeitet(neu, tr("Rename track"));
}

void DrumEditor::setzeKlang(int spur, const QString& klang)
{
    Loop neu = m_loop;
    Muster* ziel = musterIn(neu, m_muster);
    if (ziel == nullptr || spur < 0 || spur >= static_cast<int>(ziel->spuren.size())) return;
    ziel->spuren[static_cast<std::size_t>(spur)].klang = klang;
    if (neu == m_loop) return;
    Q_EMIT bearbeitet(neu, tr("Change sound"));
}

void DrumEditor::setzeLaut(int spur, double laut)
{
    Loop neu = m_loop;
    Muster* ziel = musterIn(neu, m_muster);
    if (ziel == nullptr || spur < 0 || spur >= static_cast<int>(ziel->spuren.size())) return;
    ziel->spuren[static_cast<std::size_t>(spur)].laut = std::clamp(laut, 0.0, 1.0);
    if (neu == m_loop) return;
    Q_EMIT bearbeitet(neu, tr("Change track volume"));
}

void DrumEditor::setzePan(int spur, double pan)
{
    Loop neu = m_loop;
    Muster* ziel = musterIn(neu, m_muster);
    if (ziel == nullptr || spur < 0 || spur >= static_cast<int>(ziel->spuren.size())) return;
    ziel->spuren[static_cast<std::size_t>(spur)].pan = std::clamp(pan, -1.0, 1.0);
    if (neu == m_loop) return;
    Q_EMIT bearbeitet(neu, tr("Change track pan"));
}

void DrumEditor::setzeStumm(int spur, bool stumm)
{
    Loop neu = m_loop;
    Muster* ziel = musterIn(neu, m_muster);
    if (ziel == nullptr || spur < 0 || spur >= static_cast<int>(ziel->spuren.size())) return;
    ziel->spuren[static_cast<std::size_t>(spur)].stumm = stumm;
    if (neu == m_loop) return;
    Q_EMIT bearbeitet(neu, tr("Mute track"));
}

}  // namespace lumi::loop
