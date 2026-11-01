/**
 * @file AufnahmeFenster.cpp
 * @brief Loop: das Fenster „Record sample" (Stufe 11b, Schritt 2)
 */

#include "loop/AufnahmeFenster.hpp"

#include "UI/dialogs/PaketDialoge.hpp"
#include "loop/Instrument.hpp"
#include "loop/InstrumentCreator.hpp"  // KurvenBild
#include "loop/Kit.hpp"
#include "loop/LatenzFenster.hpp"  // Stufe 15k-1
#include "loop/Loop.hpp"  // istKitKlang
#include "loop/Sample.hpp"

#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDialog>
#include <QDir>
#include <QFileInfo>
#include <QFrame>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QSettings>
#include <QSignalBlocker>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

namespace lumi::loop
{

namespace
{

const char* const kEingangGemerkt = "loop/recordInput";
constexpr int kTaktMs = 30;

QString sekunden(double wert, int stellen)
{
    return QString::number(wert, 'f', stellen);
}

}  // namespace

// =============================================================================
// AusschnittQuelle
// =============================================================================

void AusschnittQuelle::setzeDaten(std::shared_ptr<const std::vector<float>> stereo, int rate, int ausgabeRate)
{
    m_daten = std::move(stereo);
    m_rate = rate;
    m_schritt = rate > 0 && ausgabeRate > 0 ? static_cast<double>(rate) / ausgabeRate : 1.0;
    m_fertig.store(true);
}

void AusschnittQuelle::setzeAusschnitt(Ausschnitt ausschnitt)
{
    m_von.store(ausschnitt.von);
    m_bis.store(ausschnitt.bis);
}

void AusschnittQuelle::starte()
{
    m_fertig.store(false);
    m_neu.store(true);
}

void AusschnittQuelle::render(float* out, int frames)
{
    std::fill(out, out + 2 * static_cast<std::ptrdiff_t>(frames), 0.0f);
    if (m_daten == nullptr || m_rate <= 0 || m_fertig.load()) return;
    const std::vector<float>& roh = *m_daten;
    const auto alle = static_cast<std::int64_t>(roh.size() / 2);
    // der Ausschnitt gilt, wie er in diesem Block steht — er darf sich unter dem Spielen aendern
    const auto von = static_cast<double>(std::clamp<std::int64_t>(m_von.load(), 0, alle));
    const auto bis = static_cast<double>(std::clamp<std::int64_t>(m_bis.load(), 0, alle));
    if (bis - von < 2.0) return;
    if (m_neu.exchange(false)) m_ort = von;
    const bool wiederholen = m_wiederholen.load();
    const double faktor = m_faktor.load();
    const double blende = std::min(kSchnittBlendeMs * m_rate / 1000.0, (bis - von) / 2.0);
    const bool blendeVorn = m_blendeVorn.load();
    const bool blendeHinten = m_blendeHinten.load();

    for (int i = 0; i < frames; ++i)
    {
        if (m_ort < von) m_ort = von;  // der Anfang wurde hinter die Stelle gezogen
        if (m_ort >= bis - 1.0)
        {
            if (!wiederholen)
            {
                m_fertig.store(true);
                break;
            }
            m_ort = von;
        }
        const auto f = static_cast<std::size_t>(m_ort);
        const double rest = m_ort - static_cast<double>(f);
        // an beiden Enden kurz auf null, wie beim Speichern
        const double rand = std::min(blendeVorn ? m_ort - von : blende, blendeHinten ? bis - 1.0 - m_ort : blende);
        const double g = faktor * (blende > 0.0 ? std::min(1.0, rand / blende) : 1.0);
        for (std::size_t seite = 0; seite < 2; ++seite)
        {
            const double a = roh[2 * f + seite];
            const double b = roh[2 * f + 2 + seite];
            out[2 * i + static_cast<int>(seite)] = static_cast<float>((a + (b - a) * rest) * g);
        }
        m_ort += m_schritt;
    }
    m_stelle.store(static_cast<std::int64_t>(m_ort));
}

// =============================================================================
// AufnahmeFenster
// =============================================================================

AufnahmeFenster::AufnahmeFenster(QWidget* parent) : QWidget(parent, Qt::Tool)
{
    setObjectName(QStringLiteral("loopRecordSample"));
    setWindowTitle(tr("Record sample"));
    setupUI();
    zeigeAlles();
}

AufnahmeFenster::~AufnahmeFenster()
{
    halte();
    if (m_engine != nullptr && m_nimmtAuf) (void)m_engine->recordStop();
}

void AufnahmeFenster::setupUI()
{
    auto* aussen = new QVBoxLayout(this);

    // --- Input und Pegel
    m_eingang = new QComboBox(this);
    m_eingang->setObjectName(QStringLiteral("recordInput"));
    m_eingang->setToolTip(tr("Where the recording comes from"));
    m_eingang->setPlaceholderText(tr("(no input found)"));
    m_pegel = new QProgressBar(this);
    m_pegel->setObjectName(QStringLiteral("recordLevel"));
    m_pegel->setRange(0, 100);
    m_pegel->setTextVisible(false);
    m_pegel->setFixedWidth(110);
    m_pegel->setFixedHeight(10);
    m_pegel->setToolTip(tr("Level of the input while recording"));
    auto* oben = new QHBoxLayout;
    oben->addWidget(new QLabel(tr("Input"), this));
    oben->addWidget(m_eingang, 1);
    oben->addWidget(new QLabel(tr("Level"), this));
    oben->addWidget(m_pegel);
    aussen->addLayout(oben);

    // --- Stufe 15k-1 (SK15k-1 a A): die Zeile fuer Wave-Clips, nur in diesem Modus
    m_waveZeile = new QWidget(this);
    m_waveZeile->setObjectName(QStringLiteral("recordWaveRow"));
    {
        auto* spalten = new QVBoxLayout(m_waveZeile);
        spalten->setContentsMargins(0, 0, 0, 0);
        auto* zeile = new QHBoxLayout;
        m_startTakt = new QSpinBox(m_waveZeile);
        m_startTakt->setObjectName(QStringLiteral("recordStartBar"));
        m_startTakt->setRange(1, 9999);
        m_startTakt->setToolTip(tr("The take begins at this bar"));
        m_einzaehlen = new QComboBox(m_waveZeile);
        m_einzaehlen->setObjectName(QStringLiteral("recordCountIn"));
        m_einzaehlen->addItem(tr("none"), 0);
        m_einzaehlen->addItem(tr("1 bar"), 1);
        m_einzaehlen->addItem(tr("2 bars"), 2);
        m_einzaehlen->setToolTip(tr("Bars the loop plays, with a click, before the take begins"));
        m_laenge = new QComboBox(m_waveZeile);
        m_laenge->setObjectName(QStringLiteral("recordLength"));
        m_laenge->addItem(tr("until Stop"), 0);
        for (const int takte : {1, 2, 4, 8, 16, 32}) m_laenge->addItem(takte == 1 ? tr("1 bar") : tr("%1 bars").arg(takte), takte);
        m_laenge->setToolTip(tr("Until Stop: Stop ends the take at the end of the bar"));
        m_klick = new QCheckBox(tr("Click while recording"), m_waveZeile);
        m_klick->setObjectName(QStringLiteral("recordClick"));
        zeile->addWidget(new QLabel(tr("Starts at bar"), m_waveZeile));
        zeile->addWidget(m_startTakt);
        zeile->addWidget(new QLabel(tr("Count-in"), m_waveZeile));
        zeile->addWidget(m_einzaehlen);
        zeile->addWidget(new QLabel(tr("Length"), m_waveZeile));
        zeile->addWidget(m_laenge);
        zeile->addWidget(m_klick);
        zeile->addStretch(1);
        spalten->addLayout(zeile);
        auto* latenz = new QHBoxLayout;
        m_latenz = new QSpinBox(m_waveZeile);
        m_latenz->setObjectName(QStringLiteral("recordLatency"));
        m_latenz->setRange(0, 1000);
        m_latenz->setSuffix(tr(" ms"));
        m_latenz->setToolTip(tr("How late a sound arrives: from the loop to the speaker, and from the microphone back"));
        m_latenzStand = new QLabel(m_waveZeile);
        m_latenzStand->setObjectName(QStringLiteral("recordLatencyState"));
        m_latenzStand->setEnabled(false);
        auto* messen = new QPushButton(tr("Measure latency..."), m_waveZeile);
        messen->setObjectName(QStringLiteral("recordMeasureLatency"));
        latenz->addWidget(new QLabel(tr("Latency"), m_waveZeile));
        latenz->addWidget(m_latenz);
        latenz->addWidget(m_latenzStand);
        latenz->addWidget(messen);
        latenz->addStretch(1);
        spalten->addLayout(latenz);
        m_streifen = new QLabel(m_waveZeile);
        m_streifen->setObjectName(QStringLiteral("recordStrip"));
        m_streifen->setFrameShape(QFrame::StyledPanel);
        spalten->addWidget(m_streifen);
        connect(messen, &QPushButton::clicked, this, [this] { (void)oeffneLatenzmessung(); });
        connect(m_latenz, &QSpinBox::valueChanged, this, [this] { zeigeAlles(); });
        for (QComboBox* wahl : {m_einzaehlen, m_laenge})
            connect(wahl, QOverload<int>::of(&QComboBox::activated), this, [this] { zeigeAlles(); });
        connect(m_startTakt, &QSpinBox::valueChanged, this, [this] { zeigeAlles(); });
    }
    m_waveZeile->hide();
    aussen->addWidget(m_waveZeile);

    // --- Record, Stop, Stand
    m_record = new QPushButton(tr("● Record"), this);
    m_record->setObjectName(QStringLiteral("recordStart"));
    m_record->setToolTip(tr("Start a new recording — it replaces the one in the picture"));
    m_stop = new QPushButton(tr("Stop"), this);
    m_stop->setObjectName(QStringLiteral("recordStop"));
    m_stand = new QLabel(this);
    m_stand->setObjectName(QStringLiteral("recordState"));
    m_stand->setEnabled(false);
    auto* knoepfe = new QHBoxLayout;
    knoepfe->addWidget(m_record);
    knoepfe->addWidget(m_stop);
    knoepfe->addWidget(m_stand, 1);
    aussen->addLayout(knoepfe);

    // --- das Bild der Aufnahme mit den beiden Griffen
    m_bild = new KurvenBild(this);
    m_bild->setObjectName(QStringLiteral("recordPicture"));
    m_bild->setGespiegelt(true);
    m_bild->setAussenGrau(true);
    m_bild->setMinimumHeight(110);
    aussen->addWidget(m_bild, 1);
    m_hinweis = new QLabel(tr("Drag the two handles to trim · grey parts are cut off on save · blue: play position"), this);
    m_hinweis->setObjectName(QStringLiteral("recordHint"));
    m_hinweis->setEnabled(false);
    aussen->addWidget(m_hinweis);

    // --- Play, Repeat, der Ausschnitt in Zahlen
    m_play = new QPushButton(tr("▶ Play"), this);
    m_play->setObjectName(QStringLiteral("recordPlay"));
    m_play->setCheckable(true);
    m_play->setToolTip(tr("Play what would be saved"));
    m_wiederholen = new QCheckBox(tr("Repeat"), this);
    m_wiederholen->setObjectName(QStringLiteral("recordRepeat"));
    m_wiederholen->setToolTip(tr("Play the trimmed part again and again — drag the handles while it plays"));
    m_schnitt = new QLabel(this);
    m_schnitt->setObjectName(QStringLiteral("recordTrimmed"));
    m_schnitt->setEnabled(false);
    auto* spiel = new QHBoxLayout;
    spiel->addWidget(m_play);
    spiel->addWidget(m_wiederholen);
    spiel->addWidget(m_schnitt, 1);
    aussen->addLayout(spiel);

    // --- Trim silence, Normalize
    m_ohneStille = new QCheckBox(tr("Trim silence at both ends"), this);
    m_ohneStille->setObjectName(QStringLiteral("recordTrimSilence"));
    m_ohneStille->setChecked(true);
    m_ohneStille->setToolTip(tr("Set the handles to where the sound begins and ends"));
    m_normal = new QCheckBox(tr("Normalize level"), this);
    m_normal->setObjectName(QStringLiteral("recordNormalize"));
    m_normal->setToolTip(tr("Make the saved sample as loud as the built-in sounds"));
    auto* haken = new QHBoxLayout;
    haken->addWidget(m_ohneStille);
    haken->addWidget(m_normal);
    haken->addStretch(1);
    aussen->addLayout(haken);

    // --- Cancel, Save as..., Save (Stufe 17, M2 b und M2b a: kein Namensfeld mehr)
    auto* abbrechen = new QPushButton(tr("Cancel"), this);
    abbrechen->setObjectName(QStringLiteral("recordCancel"));
    m_sichernAls = new QPushButton(tr("Save as..."), this);
    m_sichernAls->setObjectName(QStringLiteral("recordSaveAs"));
    m_sichernAls->setToolTip(tr("Save as a new sample in My samples and use it"));
    m_sichern = new QPushButton(tr("Save"), this);
    m_sichern->setObjectName(QStringLiteral("recordSave"));
    m_publish = new QPushButton(tr("Publish..."), this);
    m_publish->setObjectName(QStringLiteral("recordPublish"));
    m_publish->setToolTip(tr("Pass the sample on as one file"));
    // Stufe 15k-1: im Modus fuer Wave-Clips statt Save
    m_verwerfen = new QPushButton(tr("Discard"), this);
    m_verwerfen->setObjectName(QStringLiteral("recordDiscard"));
    m_verwerfen->setToolTip(tr("Throw this take away and record again"));
    m_behalten = new QPushButton(tr("Keep take"), this);
    m_behalten->setObjectName(QStringLiteral("recordKeepTake"));
    m_behalten->setToolTip(tr("Save the take to My samples and put it into the loop as a wave clip"));
    m_verwerfen->hide();
    m_behalten->hide();
    auto* fuss = new QHBoxLayout;
    fuss->addStretch(1);
    fuss->addWidget(abbrechen);
    fuss->addWidget(m_sichernAls);
    fuss->addWidget(m_sichern);
    fuss->addWidget(m_publish);
    fuss->addWidget(m_verwerfen);
    fuss->addWidget(m_behalten);
    aussen->addLayout(fuss);
    connect(m_verwerfen, &QPushButton::clicked, this, &AufnahmeFenster::verwirfTake);
    connect(m_behalten, &QPushButton::clicked, this, [this] {
        if (behalteTake().isEmpty() && takeTakte() > 0)
            QMessageBox::warning(this, windowTitle(), tr("Could not write the take to My samples"));
    });

    m_takt = new QTimer(this);
    m_takt->setInterval(kTaktMs);
    connect(m_takt, &QTimer::timeout, this, &AufnahmeFenster::takt);

    connect(m_record, &QPushButton::clicked, this, [this] { (void)starteAufnahme(); });
    connect(m_stop, &QPushButton::clicked, this, [this] { (void)stoppeAufnahme(); });
    connect(m_play, &QPushButton::clicked, this, [this](bool an) {
        if (an)
            (void)spiele();
        else
            halte();
        zeigeAlles();
    });
    connect(m_wiederholen, &QCheckBox::toggled, this, &AufnahmeFenster::setzeWiederholen);
    connect(m_ohneStille, &QCheckBox::toggled, this, &AufnahmeFenster::setzeOhneStille);
    connect(m_normal, &QCheckBox::toggled, this, &AufnahmeFenster::setzeNormalisieren);
    connect(m_eingang, QOverload<int>::of(&QComboBox::activated), this, [this](int index) {
        QSettings().setValue(QLatin1String(kEingangGemerkt), m_eingang->itemData(index, Qt::UserRole + 1).toString());
    });
    connect(abbrechen, &QPushButton::clicked, this, &QWidget::close);
    connect(m_sichern, &QPushButton::clicked, this, &AufnahmeFenster::frageUndSpeichere);
    connect(m_sichernAls, &QPushButton::clicked, this, &AufnahmeFenster::frageUndSpeichereAls);
    // Publish (Publish_Konzept.md P2 a): das gespeicherte Sample — Save schliesst das Fenster
    connect(m_publish, &QPushButton::clicked, this, [this] {
        lumi::ui::veroeffentlicheAusFenster(this, lumi::paket::Art::Sample,
                                            lumi::paket::dateiDerNennung(lumi::paket::Art::Sample, m_nennung, {}),
                                            geaendert());
    });
    connect(m_bild, &KurvenBild::griffGezogen, this, [this](int welcher, double stelle) {
        const auto ort = static_cast<std::int64_t>(std::llround(stelle * static_cast<double>(frames())));
        Ausschnitt neu = m_ausschnitt;
        if (welcher == KurvenBild::GriffVon) neu.von = std::min(ort, neu.bis - minFrames());
        if (welcher == KurvenBild::GriffBis) neu.bis = std::max(ort, neu.von + minFrames());
        setzeAusschnitt(neu);
    });
    resize(600, 390);
}

void AufnahmeFenster::setEngine(IAudioEngine* engine)
{
    if (engine == m_engine) return;
    halte();
    if (m_engine != nullptr && m_nimmtAuf) (void)m_engine->recordStop();
    m_nimmtAuf = false;
    m_engine = engine;
    liesEingaenge();
    zeigeAlles();
}

void AufnahmeFenster::liesEingaenge()
{
    const QSignalBlocker ruhe(m_eingang);
    m_eingang->clear();
    if (m_engine == nullptr) return;
    const QString gemerkt = QSettings().value(QLatin1String(kEingangGemerkt)).toString();
    int vorgabe = -1;
    for (const AudioDeviceInfo& eingang : m_engine->getRecordDevices())
    {
        if (!eingang.isEnabled) continue;
        // der Mitschnitt nimmt auf, was der Rechner gerade spielt — das soll man der Liste ansehen
        m_eingang->addItem(eingang.isLoopback ? tr("%1 — what the computer plays").arg(eingang.name) : eingang.name, eingang.id);
        const int index = m_eingang->count() - 1;
        m_eingang->setItemData(index, eingang.name, Qt::UserRole + 1);
        if (!gemerkt.isEmpty() && eingang.name == gemerkt) vorgabe = index;
        if (vorgabe < 0 && gemerkt.isEmpty() && eingang.isDefault && !eingang.isLoopback) vorgabe = index;
    }
    if (m_eingang->count() > 0) m_eingang->setCurrentIndex(std::max(0, vorgabe));
}

void AufnahmeFenster::beginne()
{
    halte();
    if (m_engine != nullptr && m_nimmtAuf) (void)m_engine->recordStop();
    if (m_wave && m_wave->halte && m_phase != WavePhase::Bereit) m_wave->halte();
    m_wave.reset();  // Stufe 15k-1: wieder ein Sample
    m_phase = WavePhase::Bereit;
    m_takeTakte = 0;
    m_nimmtAuf = false;
    m_daten.reset();
    m_rate = 0;
    m_ausschnitt = {};
    m_gesichert = true;
    m_nennung.clear();
    m_frisch = false;
    m_meldung.clear();
    {
        // nach einer Aufnahme stehen die Griffe am Ton — auch wenn vorher ein Sample offen war
        const QSignalBlocker ruhe(m_ohneStille);
        m_ohneStille->setChecked(true);
    }
    liesEingaenge();
    zeigeBild();
    zeigeAlles();
}

void AufnahmeFenster::bearbeite(const QString& nennung, std::vector<float> stereo, int rate)
{
    halte();
    if (m_engine != nullptr && m_nimmtAuf) (void)m_engine->recordStop();
    m_nimmtAuf = false;
    m_meldung.clear();
    m_nennung = nennung;
    m_frisch = false;
    {
        // oeffnen und speichern aendert nichts: kein Schnitt, kein Anheben
        const QSignalBlocker ruheStille(m_ohneStille);
        const QSignalBlocker ruheNormal(m_normal);
        m_ohneStille->setChecked(false);
        m_normal->setChecked(false);
    }
    m_daten = std::make_shared<const std::vector<float>>(std::move(stereo));
    m_rate = rate;
    m_ausschnitt = Ausschnitt{0, frames()};
    m_gesichert = false;  // ob es etwas zu verlieren gibt, sagt `geaendert`
    m_quelle.setzeVerstaerkung(1.0f);
    liesEingaenge();
    zeigeBild();
    zeigeAlles();
}

AufnahmeFenster::Herkunft AufnahmeFenster::herkunft() const
{
    if (m_nennung.isEmpty()) return Herkunft::Aufnahme;
    if (istEigenesSample(m_nennung)) return Herkunft::Eigenes;
    return istMitgelieferterKlang(m_nennung) ? Herkunft::Mitgeliefert : Herkunft::Datei;
}

bool AufnahmeFenster::geaendert() const
{
    if (frames() <= 0) return false;
    return m_frisch || m_ausschnitt != Ausschnitt{0, frames()} || m_normal->isChecked();
}

std::int64_t AufnahmeFenster::frames() const
{
    return m_daten != nullptr ? static_cast<std::int64_t>(m_daten->size() / 2) : 0;
}

std::int64_t AufnahmeFenster::minFrames() const
{
    return std::max<std::int64_t>(2, static_cast<std::int64_t>(kAusschnittMinMs * m_rate / 1000.0));
}

bool AufnahmeFenster::starteAufnahme()
{
    if (m_wave) return starteWave();
    if (m_engine == nullptr || m_eingang->count() == 0 || m_nimmtAuf) return false;
    halte();
    m_meldung.clear();
    if (!m_engine->recordStart(m_eingang->currentData().toInt(), kSampleRate, kHoechstensSekunden))
    {
        // keine Meldung im eigenen Fenster: die Zeile sagt, warum — und was man tun kann
        m_meldung = tr("Could not open this input. Is the microphone allowed for desktop apps in the Windows privacy settings?");
        zeigeAlles();
        return false;
    }
    m_nimmtAuf = true;
    m_takt->start();
    zeigeAlles();
    return true;
}

bool AufnahmeFenster::stoppeAufnahme()
{
    if (m_wave)
    {
        if (m_phase == WavePhase::Bereit) return false;
        stoppeWave();
        return true;
    }
    if (m_engine == nullptr || !m_nimmtAuf) return false;
    m_nimmtAuf = false;
    std::vector<float> daten = m_engine->recordStop();
    if (!spielt()) m_takt->stop();
    m_pegel->setValue(0);
    if (daten.size() < 4)
    {
        m_meldung = tr("Nothing arrived from this input.");
        zeigeAlles();
        return true;
    }
    setzeAufnahme(std::move(daten), kSampleRate);
    return true;
}

void AufnahmeFenster::setzeAufnahme(std::vector<float> stereo, int rate)
{
    halte();
    m_meldung.clear();
    m_daten = std::make_shared<const std::vector<float>>(std::move(stereo));
    m_rate = rate;
    m_gesichert = false;
    if (!m_nennung.isEmpty() && !m_frisch)
    {
        // die erste Aufnahme ueber einem geoeffneten Sample: Trim silence wie nach jeder Aufnahme
        const QSignalBlocker ruhe(m_ohneStille);
        m_ohneStille->setChecked(true);
    }
    // ueber einem Klang des Kits oder einer Datei aufgenommen: mit ihnen hat das Bild nichts mehr zu tun.
    // Ein eigenes Sample bleibt offen — Save schreibt die Aufnahme unter seinem Namen (M4 a).
    if (herkunft() != Herkunft::Eigenes) m_nennung.clear();
    m_frisch = true;
    m_ausschnitt = m_ohneStille->isChecked() ? ohneStille(*m_daten, m_rate) : Ausschnitt{0, frames()};
    zeigeBild();
    zeigeAlles();
}

void AufnahmeFenster::setzeAusschnitt(Ausschnitt ausschnitt)
{
    const std::int64_t alle = frames();
    if (alle <= 0) return;
    ausschnitt.von = std::clamp<std::int64_t>(ausschnitt.von, 0, std::max<std::int64_t>(0, alle - minFrames()));
    ausschnitt.bis = std::clamp<std::int64_t>(ausschnitt.bis, std::min(alle, ausschnitt.von + minFrames()), alle);
    if (ausschnitt == m_ausschnitt) return;
    m_ausschnitt = ausschnitt;
    // was gerade spielt, folgt sofort
    m_quelle.setzeAusschnitt(m_ausschnitt);
    m_quelle.setzeBlenden(blenden());
    setzeNormalisieren(m_normal->isChecked());
    zeigeAlles();
}

void AufnahmeFenster::setzeOhneStille(bool an)
{
    {
        const QSignalBlocker ruhe(m_ohneStille);
        m_ohneStille->setChecked(an);
    }
    if (m_daten != nullptr) setzeAusschnitt(an ? ohneStille(*m_daten, m_rate) : Ausschnitt{0, frames()});
}

void AufnahmeFenster::setzeNormalisieren(bool an)
{
    {
        const QSignalBlocker ruhe(m_normal);
        m_normal->setChecked(an);
    }
    // auch beim Anhoeren: so laut, wie es gespeichert wuerde
    float faktor = 1.0f;
    if (an && m_daten != nullptr)
    {
        const float spitze = spitzeIn(*m_daten, m_ausschnitt);
        if (spitze > 0.0f) faktor = static_cast<float>(kNormalSpitze / spitze);
    }
    m_quelle.setzeVerstaerkung(faktor);
}

void AufnahmeFenster::setzeWiederholen(bool an)
{
    {
        const QSignalBlocker ruhe(m_wiederholen);
        m_wiederholen->setChecked(an);
    }
    m_quelle.setzeWiederholen(an);
}

bool AufnahmeFenster::spiele()
{
    if (m_engine == nullptr || m_daten == nullptr || m_nimmtAuf) return false;
    halte();
    m_quelle.setzeDaten(m_daten, m_rate, m_engine->getSampleRate());
    m_quelle.setzeAusschnitt(m_ausschnitt);
    m_quelle.setzeBlenden(blenden());
    m_quelle.setzeWiederholen(m_wiederholen->isChecked());
    setzeNormalisieren(m_normal->isChecked());
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
    m_takt->start();
    zeigeAlles();
    return true;
}

void AufnahmeFenster::halte()
{
    if (m_mixer == INVALID_STREAM) return;
    if (m_engine != nullptr)
    {
        m_engine->mixerRemoveSource(m_quellHandle);
        m_engine->freeStream(m_mixer);
    }
    m_mixer = INVALID_STREAM;
    m_quellHandle = INVALID_STREAM;
    if (!m_nimmtAuf) m_takt->stop();
    m_bild->setZeiger(-1.0);
    zeigeAlles();
}

std::vector<float> AufnahmeFenster::ergebnis() const
{
    if (m_daten == nullptr) return {};
    // ein Take sitzt im Takt und bleibt, wie er ankam: Blenden und Gain traegt das Wave-Muster (15c),
    // Normalize und Trim silence sind in diesem Modus nicht zu sehen
    if (m_wave) return *m_daten;
    return schneideAus(*m_daten, m_rate, m_ausschnitt, m_normal->isChecked(), blenden());
}

Blenden AufnahmeFenster::blenden() const
{
    if (m_frisch || m_nennung.isEmpty()) return {};
    return {m_ausschnitt.von > 0, m_ausschnitt.bis < frames()};
}

QString AufnahmeFenster::schreibe(const QString& name)
{
    const std::vector<float> daten = ergebnis();
    if (daten.empty() || name.isEmpty()) return {};
    halte();
    // eine frische Aufnahme ist ein anderer Klang: dort gibt es keinen Einsatz mitzuziehen
    const double vornWegMs = m_frisch ? -1.0 : 1000.0 * static_cast<double>(m_ausschnitt.von) / m_rate;
    const QString nennung = speichereAufnahme(daten, m_rate, name);
    if (nennung.isEmpty()) return {};
    m_gesichert = true;
    Q_EMIT gespeichert(nennung, vornWegMs);
    close();
    return nennung;
}

QString AufnahmeFenster::speichere()
{
    if (herkunft() != Herkunft::Eigenes) return {};
    return schreibe(m_nennung.mid(static_cast<int>(qstrlen(kSampleVorsatz))));
}

QString AufnahmeFenster::speichereAls(const QString& titel)
{
    const QString name = eigenerName(titel);
    // einen vergebenen Namen nimmt Save as... nicht — ersetzen kann nur Save (M2 b)
    if (name.isEmpty() || !eigeneSampleDatei(name).isEmpty()) return {};
    return schreibe(name);
}

QString AufnahmeFenster::vorschlagTitel() const
{
    return m_nennung.isEmpty() ? vorschlagAufnahmeTitel() : vorschlagKopieTitel(sampleTitel(m_nennung));
}

QString AufnahmeFenster::ersetzenFrage() const
{
    const QString datei = QFileInfo(sampleDatei(m_nennung)).fileName();
    QString frage = m_frisch ? tr("Replace %1 with the new recording?").arg(datei)
                             : tr("Replace %1?\nWhat you cut away is gone.").arg(datei);
    const QStringList nutzer = eigeneInstrumenteMitSample(m_nennung);
    if (!nutzer.isEmpty())
    {
        constexpr int kBeimNamen = 3;
        QString liste = nutzer.mid(0, kBeimNamen).join(QStringLiteral(", "));
        if (nutzer.size() > kBeimNamen) liste = tr("%1 and %2 more").arg(liste).arg(nutzer.size() - kBeimNamen);
        frage += QLatin1Char('\n') + tr("Used by: %1 — they will play the new version.").arg(liste);
    }
    return frage;
}

QString AufnahmeFenster::dateiZeile(const QString& titel, bool* geht)
{
    const QString kurz = eigenerName(titel);
    const bool vergeben = !kurz.isEmpty() && !eigeneSampleDatei(kurz).isEmpty();
    if (geht != nullptr) *geht = !kurz.isEmpty() && !vergeben;
    if (kurz.isEmpty()) return tr("Type a name with letters or digits");
    if (vergeben) return tr("My samples already holds %1 — choose another name").arg(QFileInfo(eigeneSampleDatei(kurz)).fileName());
    return tr("File: My files · samples · %1.wav").arg(kurz);
}

void AufnahmeFenster::frageUndSpeichere()
{
    if (herkunft() != Herkunft::Eigenes) return;
    const QString datei = sampleDatei(m_nennung);
    if (QMessageBox::question(this, windowTitle(), ersetzenFrage()) != QMessageBox::Yes) return;
    if (speichere().isEmpty())
        QMessageBox::warning(this, windowTitle(), tr("Could not write %1").arg(QDir::toNativeSeparators(datei)));
}

void AufnahmeFenster::frageUndSpeichereAls()
{
    if (frames() <= 0) return;
    QDialog dialog(this);
    dialog.setObjectName(QStringLiteral("recordSaveAsDialog"));
    dialog.setWindowTitle(tr("Save sample as"));
    auto* aussen = new QVBoxLayout(&dialog);
    QString satzText = tr("This saves a new sample to My samples and uses it.");
    if (herkunft() == Herkunft::Mitgeliefert) satzText = tr("Built-in sounds stay as they are. This saves a copy to My samples and uses it.");
    if (herkunft() == Herkunft::Datei) satzText = tr("The file stays as it is. This saves a copy to My samples and uses it.");
    if (herkunft() == Herkunft::Eigenes) satzText = tr("%1 stays as it is. This saves a copy to My samples and uses it.").arg(sampleTitel(m_nennung));
    auto* satz = new QLabel(satzText, &dialog);
    satz->setWordWrap(true);
    satz->setEnabled(false);
    aussen->addWidget(satz);
    auto* name = new QLineEdit(vorschlagTitel(), &dialog);
    name->setObjectName(QStringLiteral("recordSaveAsName"));
    auto* zeile = new QHBoxLayout;
    zeile->addWidget(new QLabel(tr("Name"), &dialog));
    zeile->addWidget(name, 1);
    aussen->addLayout(zeile);
    auto* datei = new QLabel(&dialog);
    datei->setObjectName(QStringLiteral("recordSaveAsFile"));
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
                             tr("Could not write %1")
                                 .arg(QDir::toNativeSeparators(QDir(eigenerSampleOrdner()).filePath(eigenerName(titel) + QStringLiteral(".wav")))));
}

void AufnahmeFenster::takt()
{
    if (m_engine == nullptr) return;
    if (m_wave && m_phase != WavePhase::Bereit)
    {
        taktWave();
        return;
    }
    if (m_nimmtAuf)
    {
        // der Pegel: der groesste Betrag seit dem letzten Takt, in Dezibel auf die Breite gelegt (-48 dB bis 0)
        const float spitze = m_engine->recordLevel();
        const double db = spitze > 0.0f ? 20.0 * std::log10(static_cast<double>(spitze)) : -100.0;
        m_pegel->setValue(static_cast<int>(std::clamp((db + 48.0) / 48.0, 0.0, 1.0) * 100.0));
        if (!m_engine->isRecording())
        {
            (void)stoppeAufnahme();  // die hoechste Laenge ist erreicht
            return;
        }
        zeigeAlles();
    }
    if (spielt())
    {
        if (m_quelle.fertig())
        {
            halte();
            return;
        }
        const std::int64_t alle = frames();
        m_bild->setZeiger(alle > 0 ? static_cast<double>(m_quelle.stelle()) / static_cast<double>(alle) : -1.0);
    }
}

void AufnahmeFenster::zeigeBild()
{
    std::vector<float> werte;
    if (m_daten != nullptr && frames() > 1)
    {
        // je Stueck der groesste Betrag beider Seiten, auf die Spitze bezogen — auch eine leise Aufnahme ist zu sehen
        constexpr std::int64_t kStuecke = 600;
        const std::int64_t alle = frames();
        werte.assign(static_cast<std::size_t>(kStuecke), 0.0f);
        float groesster = 0.0f;
        for (std::int64_t f = 0; f < alle; ++f)
        {
            const auto stueck = static_cast<std::size_t>(f * kStuecke / alle);
            const float wert = std::max(std::abs((*m_daten)[static_cast<std::size_t>(2 * f)]),
                                        std::abs((*m_daten)[static_cast<std::size_t>(2 * f + 1)]));
            werte[stueck] = std::max(werte[stueck], wert);
            groesster = std::max(groesster, wert);
        }
        if (groesster > 0.0f)
            for (float& wert : werte) wert /= groesster;
    }
    m_bild->setWerte(std::move(werte), false);
}

void AufnahmeFenster::zeigeAlles()
{
    const bool engineDa = m_engine != nullptr;
    const bool eingangDa = engineDa && m_eingang->count() > 0;
    const std::int64_t alle = frames();
    const bool aufnahmeDa = alle > 0;

    m_eingang->setEnabled(eingangDa && !m_nimmtAuf);
    m_record->setEnabled(eingangDa && !m_nimmtAuf);
    m_stop->setEnabled(m_nimmtAuf);

    QString stand;
    const QString hoechstens = tr("up to %1 s").arg(sekunden(kHoechstensSekunden, 0));
    if (!m_meldung.isEmpty())
        stand = m_meldung;
    else if (m_nimmtAuf)
        stand = tr("%1 s recorded · %2").arg(sekunden(static_cast<double>(m_engine->recordedFrames()) / kSampleRate, 1), hoechstens);
    else if (aufnahmeDa && m_frisch)
        stand = tr("%1 s recorded").arg(sekunden(static_cast<double>(alle) / m_rate, 1));
    else if (aufnahmeDa)
    {
        // ein geoeffnetes Sample: wie lang es ist und woher es kommt
        QString woher = tr("My samples");
        if (herkunft() == Herkunft::Mitgeliefert) woher = tr("Built-in, read-only");
        if (herkunft() == Herkunft::Datei) woher = tr("File, read-only");
        stand = tr("%1 s · %2").arg(sekunden(static_cast<double>(alle) / m_rate, 2), woher);
    }
    else if (!engineDa)
        stand = tr("No audio engine — recording is not available");
    else if (!eingangDa)
        stand = tr("No input found");
    else
        stand = tr("Press Record — %1").arg(hoechstens);
    m_stand->setText(stand);
    m_stand->setWordWrap(!m_meldung.isEmpty());

    // die Griffe und der Ausschnitt in Zahlen
    if (aufnahmeDa && !m_nimmtAuf)
    {
        m_bild->setGriffe(-1.0, static_cast<double>(m_ausschnitt.von) / static_cast<double>(alle),
                          static_cast<double>(m_ausschnitt.bis) / static_cast<double>(alle));
        m_schnitt->setText(tr("Trimmed: %1 s to %2 s · %3 s")
                               .arg(sekunden(static_cast<double>(m_ausschnitt.von) / m_rate, 2),
                                    sekunden(static_cast<double>(m_ausschnitt.bis) / m_rate, 2),
                                    sekunden(static_cast<double>(m_ausschnitt.frames()) / m_rate, 2)));
    }
    else
    {
        m_bild->setGriffe(-1.0, -1.0, -1.0);
        m_schnitt->clear();
    }
    m_hinweis->setVisible(aufnahmeDa && !m_nimmtAuf);
    m_play->setEnabled(engineDa && aufnahmeDa && !m_nimmtAuf);
    {
        const QSignalBlocker ruhe(m_play);
        m_play->setChecked(spielt());
        m_play->setText(spielt() ? tr("■ Stop playing") : tr("▶ Play"));
    }
    m_ohneStille->setEnabled(aufnahmeDa && !m_nimmtAuf);
    m_normal->setEnabled(aufnahmeDa && !m_nimmtAuf);

    // der Titel sagt, was offen ist; Save gibt es nur fuer ein eigenes Sample
    setWindowTitle(m_nennung.isEmpty() ? tr("Record sample") : tr("Edit sample – %1").arg(sampleTitel(m_nennung)));
    const bool eigenes = herkunft() == Herkunft::Eigenes;
    m_sichernAls->setEnabled(aufnahmeDa && !m_nimmtAuf);
    m_sichern->setEnabled(aufnahmeDa && !m_nimmtAuf && eigenes);
    if (eigenes)
        m_sichern->setToolTip(tr("Replace %1 in My samples").arg(sampleTitel(m_nennung)));
    else if (herkunft() == Herkunft::Aufnahme)
        m_sichern->setToolTip(tr("A new recording has no name yet — use Save as..."));
    else
        m_sichern->setToolTip(tr("Built-in sounds and files stay as they are — use Save as..."));
    zeigeWave();
}

void AufnahmeFenster::closeEvent(QCloseEvent* event)
{
    if (!m_gesichert && geaendert() &&
        QMessageBox::question(this, windowTitle(),
                              m_frisch ? tr("The recording is not saved.\nDiscard it?") : tr("Your changes are not saved.\nDiscard them?")) !=
            QMessageBox::Yes)
    {
        event->ignore();
        return;
    }
    halte();
    if (m_engine != nullptr && m_nimmtAuf) (void)m_engine->recordStop();
    if (m_wave && m_phase != WavePhase::Bereit)
    {
        // Stufe 15k-1: der Loop spielte zur Aufnahme — er haelt mit dem Fenster
        if (m_engine != nullptr) m_engine->setRecordClock({});
        if (m_wave->halte) m_wave->halte();
        m_phase = WavePhase::Bereit;
    }
    m_nimmtAuf = false;
    m_takt->stop();
    QWidget::closeEvent(event);
}

// --- Stufe 15k-1: Aufnahme in einen Wave-Clip --------------------------------------------

void AufnahmeFenster::beginneWaveClip(WaveAufnahme auftrag)
{
    beginne();
    const WaveAufnahmePlan plan = auftrag.plan;
    m_wave = std::move(auftrag);
    {
        // ein Take bleibt, wie er aufgenommen ist: kein Schnitt an der Stille, kein Anheben — er sitzt im Takt
        const QSignalBlocker a(m_ohneStille);
        const QSignalBlocker b(m_normal);
        m_ohneStille->setChecked(false);
        m_normal->setChecked(false);
    }
    setzePlan(plan);
    // die gemerkte Latenz fuer dieses Paar aus Eingang und Ausgang (WV10-1 a)
    const std::optional<double> gemerkt =
        gemerkteLatenzMs(m_eingang->currentData(Qt::UserRole + 1).toString(), m_wave->ausgang);
    setzeLatenzMs(gemerkt.value_or(0.0));
    m_latenzStand->setText(gemerkt ? tr("measured · %1 → %2").arg(m_eingang->currentText(), m_wave->ausgang)
                                   : tr("not measured yet"));
    zeigeAlles();
}

WaveAufnahmePlan AufnahmeFenster::plan() const
{
    WaveAufnahmePlan p;
    p.startTakt = m_startTakt->value() - 1;
    p.einzaehlen = m_einzaehlen->currentData().toInt();
    p.takte = m_laenge->currentData().toInt();
    p.klick = m_klick->isChecked();
    return p;
}

void AufnahmeFenster::setzePlan(const WaveAufnahmePlan& plan)
{
    const QSignalBlocker a(m_startTakt);
    const QSignalBlocker b(m_einzaehlen);
    const QSignalBlocker c(m_laenge);
    const QSignalBlocker d(m_klick);
    m_startTakt->setValue(plan.startTakt + 1);
    m_einzaehlen->setCurrentIndex(std::max(0, m_einzaehlen->findData(std::clamp(plan.einzaehlen, 0, 2))));
    // eine Laenge, die die Liste nicht kennt (die eines Musters), kommt dazu
    int index = m_laenge->findData(plan.takte);
    if (index < 0)
    {
        m_laenge->addItem(plan.takte == 1 ? tr("1 bar") : tr("%1 bars").arg(plan.takte), plan.takte);
        index = m_laenge->count() - 1;
    }
    m_laenge->setCurrentIndex(index);
    m_klick->setChecked(plan.klick);
    zeigeAlles();
}

double AufnahmeFenster::latenzMs() const
{
    return m_latenz->value();
}

void AufnahmeFenster::setzeLatenzMs(double ms)
{
    m_latenz->setValue(static_cast<int>(std::lround(ms)));
}

bool AufnahmeFenster::oeffneLatenzmessung(bool zeigen)
{
    if (!m_wave) return false;
    if (m_latenzFenster == nullptr)
    {
        m_latenzFenster = new LatenzFenster(this);
        connect(m_latenzFenster, &LatenzFenster::latenzGewaehlt, this, [this](double ms) {
            setzeLatenzMs(ms);
            m_latenzStand->setText(tr("measured · %1 → %2").arg(m_eingang->currentText(), m_wave ? m_wave->ausgang : QString()));
        });
    }
    m_latenzFenster->setze(m_engine, m_eingang->currentData().toInt(), m_eingang->currentData(Qt::UserRole + 1).toString(),
                           m_wave->ausgang);
    if (zeigen)
    {
        m_latenzFenster->show();
        m_latenzFenster->raise();
        m_latenzFenster->activateWindow();
    }
    return true;
}

LatenzFenster* AufnahmeFenster::latenzFenster() const
{
    return m_latenzFenster;
}

double AufnahmeFenster::taktFramesJetzt() const
{
    return m_wave ? taktFrames(m_fassung.loop, m_wave->rate) : 0.0;
}

std::int64_t AufnahmeFenster::latenzFrames() const
{
    return m_wave ? std::llround(latenzMs() * m_wave->rate / 1000.0) : 0;
}

bool AufnahmeFenster::starteWave()
{
    if (!m_wave || m_engine == nullptr || m_eingang->count() == 0 || m_phase != WavePhase::Bereit) return false;
    if (!m_wave->spiele || !m_wave->uhr || m_wave->rate <= 0) return false;
    halte();
    m_meldung.clear();
    m_daten.reset();
    m_takeTakte = 0;
    m_fassung = aufnahmeFassung(m_wave->loop, plan());
    // die Stelle, an der der Take beginnt — wie der Rechenkern einen Takt rechnet
    const Loop& f = m_fassung.loop;
    const double schrittFrames = f.bpm > 0.0 ? m_wave->rate * 60.0 / f.bpm / f.schritteJeSchlag : 0.0;
    m_vonUhr = std::llround(static_cast<double>(m_fassung.aufnahmeTakt) * f.schritteJeTakt() * schrittFrames);
    const int takte = plan().takte;
    m_bisUhr = takte > 0 ? m_vonUhr + std::llround(static_cast<double>(takte) * f.schritteJeTakt() * schrittFrames) : -1;
    if (!m_wave->spiele(f, m_fassung.abspielTakt))
    {
        m_meldung = tr("The loop can't play now — the Composer has the audio.");
        zeigeAlles();
        return false;
    }
    m_phase = WavePhase::Wartet;
    m_takt->start();
    zeigeAlles();
    return true;
}

void AufnahmeFenster::taktWave()
{
    const std::int64_t uhr = m_wave->uhr();
    if (m_phase == WavePhase::Wartet)
    {
        // erst wenn der Loop hoerbar spielt: dann laeuft die Uhr, und die Aufnahme haengt an ihr
        if (m_wave->bereit && !m_wave->bereit()) return;
        m_engine->setRecordClock(m_wave->uhr);
        if (!m_engine->recordStart(m_eingang->currentData().toInt(), m_wave->rate, kWaveHoechstensSekunden))
        {
            m_engine->setRecordClock({});
            if (m_wave->halte) m_wave->halte();
            m_phase = WavePhase::Bereit;
            m_takt->stop();
            m_meldung = tr("Could not open this input. Is the microphone allowed for desktop apps in the Windows privacy settings?");
            zeigeAlles();
            return;
        }
        m_nimmtAuf = true;
        m_phase = WavePhase::Nimmt;
    }
    // der Pegel wie beim Sample
    const float spitze = m_engine->recordLevel();
    const double db = spitze > 0.0f ? 20.0 * std::log10(static_cast<double>(spitze)) : -100.0;
    m_pegel->setValue(static_cast<int>(std::clamp((db + 48.0) / 48.0, 0.0, 1.0) * 100.0));
    // zu Ende: nach dem Ende des Takes und der Latenz, und einem Rest fuer die Bloecke
    const auto rest = static_cast<std::int64_t>(0.1 * m_wave->rate);
    if (m_bisUhr >= 0 && uhr >= m_bisUhr + latenzFrames() + rest)
    {
        beendeWave();
        return;
    }
    if (!m_engine->isRecording())
    {
        // die hoechste Laenge ist erreicht: der Take endet, wo er ist
        if (m_bisUhr < 0) m_bisUhr = std::max(m_vonUhr + 1, uhr - latenzFrames());
        beendeWave();
        return;
    }
    zeigeAlles();
}

void AufnahmeFenster::stoppeWave()
{
    const std::int64_t gehoert = m_wave->uhr() - latenzFrames();
    if (m_phase == WavePhase::Wartet || gehoert < m_vonUhr)
    {
        // noch vor dem Take (beim Einzaehlen): nichts behalten
        if (m_nimmtAuf) (void)m_engine->recordStop();
        m_engine->setRecordClock({});
        m_nimmtAuf = false;
        if (m_wave->halte) m_wave->halte();
        m_phase = WavePhase::Bereit;
        m_takt->stop();
        zeigeAlles();
        return;
    }
    if (m_phase != WavePhase::Nimmt) return;
    if (m_bisUhr < 0)
    {
        // bis Stop: der Take endet am Ende des Takts, in dem Stop gehoert wurde
        const double takt = taktFramesJetzt();
        const auto takte = std::max<std::int64_t>(1, static_cast<std::int64_t>(std::ceil((gehoert - m_vonUhr) / takt)));
        m_bisUhr = m_vonUhr + std::llround(static_cast<double>(takte) * takt);
    }
    m_phase = WavePhase::Endet;
    zeigeAlles();
}

void AufnahmeFenster::beendeWave()
{
    std::vector<float> daten = m_engine->recordStop();
    const std::vector<IAudioEngine::RecordClockMark> marken = m_engine->recordClockMarks();
    m_engine->setRecordClock({});
    m_nimmtAuf = false;
    if (m_wave->halte) m_wave->halte();
    m_phase = WavePhase::Bereit;
    m_takt->stop();
    m_pegel->setValue(0);
    const std::optional<std::int64_t> versatz = uhrVersatz(marken);
    if (!versatz || daten.size() < 4)
    {
        m_meldung = tr("Nothing arrived from this input.");
        zeigeAlles();
        return;
    }
    const double takt = taktFramesJetzt();
    m_takeTakte = takt > 0.0 ? std::max(1, static_cast<int>(std::lround(static_cast<double>(m_bisUhr - m_vonUhr) / takt))) : 0;
    setzeAufnahme(schneideTake(daten, *versatz, latenzFrames(), m_vonUhr, m_bisUhr), m_wave->rate);
    // ein Take wird nicht an der Stille geschnitten: er sitzt im Takt
    m_ausschnitt = Ausschnitt{0, frames()};
    zeigeAlles();
}

QString AufnahmeFenster::behalteTake()
{
    if (!m_wave || m_takeTakte <= 0 || frames() <= 0) return {};
    // der erste freie Name „Take 1", „Take 2" …
    QString name;
    for (int n = 1; n < 10000 && name.isEmpty(); ++n)
    {
        const QString kandidat = eigenerName(tr("Take %1").arg(n));
        if (!kandidat.isEmpty() && eigeneSampleDatei(kandidat).isEmpty()) name = kandidat;
    }
    const QString nennung = name.isEmpty() ? QString() : speichereAufnahme(ergebnis(), m_rate, name);
    if (nennung.isEmpty()) return {};
    m_gesichert = true;
    Q_EMIT waveTakeBehalten(nennung, plan().startTakt, m_takeTakte);
    close();
    return nennung;
}

void AufnahmeFenster::verwirfTake()
{
    m_daten.reset();
    m_takeTakte = 0;
    m_gesichert = true;
    m_ausschnitt = {};
    zeigeBild();
    zeigeAlles();
}

QString AufnahmeFenster::streifenText() const
{
    if (!m_wave) return {};
    const WaveAufnahmePlan p = plan();
    const int start = p.startTakt + 1;
    QStringList teile;
    if (p.einzaehlen > 0) teile << tr("bar %1 · count-in").arg(start - p.einzaehlen < 1 ? QStringLiteral("…") : QString::number(start - p.einzaehlen));
    if (m_phase == WavePhase::Nimmt || m_phase == WavePhase::Endet)
    {
        const double takt = taktFramesJetzt();
        const std::int64_t gehoert = m_wave->uhr() - latenzFrames();
        if (gehoert < m_vonUhr)
            teile << tr("counting in…");
        else
            teile << tr("recording bar %1").arg(start + static_cast<int>((gehoert - m_vonUhr) / std::max(1.0, takt)));
        if (m_phase == WavePhase::Endet) teile << tr("ends at the end of the bar");
    }
    else if (m_phase == WavePhase::Wartet)
        teile << tr("preparing the loop…");
    else if (m_takeTakte > 0)
        teile << tr("take: bar %1 – %2 (%3 bars)").arg(start).arg(start + m_takeTakte - 1).arg(m_takeTakte);
    else
        teile << (p.takte > 0 ? tr("bar %1 – %2 · recording").arg(start).arg(start + p.takte - 1)
                              : tr("from bar %1 · recording until Stop, to the end of that bar").arg(start));
    return teile.join(QStringLiteral(" | "));
}

void AufnahmeFenster::zeigeWave()
{
    const bool wave = m_wave.has_value();
    m_waveZeile->setVisible(wave);
    m_behalten->setVisible(wave);
    m_verwerfen->setVisible(wave);
    m_sichern->setVisible(!wave);
    m_sichernAls->setVisible(!wave);
    m_publish->setVisible(!wave);
    m_ohneStille->setVisible(!wave);
    m_normal->setVisible(!wave);
    if (!wave) return;
    setWindowTitle(tr("Record wave clip"));
    const bool laeuft = m_phase != WavePhase::Bereit;
    m_record->setEnabled(m_engine != nullptr && m_eingang->count() > 0 && !laeuft);
    m_stop->setEnabled(laeuft);
    for (QWidget* feld : std::initializer_list<QWidget*>{m_startTakt, m_einzaehlen, m_laenge, m_klick, m_latenz, m_eingang})
        feld->setEnabled(!laeuft);
    m_streifen->setText(streifenText());
    m_behalten->setEnabled(!laeuft && m_takeTakte > 0 && frames() > 0);
    m_verwerfen->setEnabled(!laeuft && frames() > 0);
    if (laeuft) m_stand->setText(m_phase == WavePhase::Wartet ? tr("Preparing the loop…") : tr("Recording…"));
}

}  // namespace lumi::loop
