/**
 * @file ClipEffektFenster.cpp
 * @brief Loop: das Fenster „Clip effects"
 */

#include "loop/ClipEffektFenster.hpp"

#include "loop/SignalWeg.hpp"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QStackedWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include <utility>

namespace lumi::loop
{

namespace
{

struct ZeitEintrag
{
    EchoZeit zeit;
    const char* name;
};
// Namen wie in der Skizze; „Delay" kommt nicht vor (S111 als Echo gelesen)
constexpr ZeitEintrag kZeiten[] = {
    {EchoZeit::Sechzehntel, "1/16"}, {EchoZeit::Achtel, "1/8"}, {EchoZeit::AchtelPunktiert, "1/8 dotted"},
    {EchoZeit::Viertel, "1/4"},      {EchoZeit::Halbe, "1/2"},
};

/// Die Seiten unter der Zeile: je Art eines Bausteins eine, davor die ohne Regler.
/// Die Folge der Arten ist die des `ClipBaustein`: Seite = Art + 1.
constexpr int kSeiteLeer = 0;

/// Cutoff: der Schieber geht gleichmaessig in Oktaven von `kClipFilterHzMin` bis `kClipFilterHzMax`.
constexpr int kCutoffStufen = 1000;

double hzVon(double stufe)
{
    return kClipFilterHzMin *
           std::pow(kClipFilterHzMax / kClipFilterHzMin, std::clamp(stufe, 0.0, 1.0 * kCutoffStufen) / kCutoffStufen);
}

double stufeVon(double hz)
{
    const double h = std::clamp(hz, kClipFilterHzMin, kClipFilterHzMax);
    return kCutoffStufen * std::log(h / kClipFilterHzMin) / std::log(kClipFilterHzMax / kClipFilterHzMin);
}

QString prozent(double wert)
{
    return QStringLiteral("%1 %").arg(std::lround(wert * 100.0));
}

QString hertz(double hz)
{
    return hz < 1000.0 ? QStringLiteral("%1 Hz").arg(std::lround(hz)) : QStringLiteral("%1 kHz").arg(hz / 1000.0, 0, 'f', 1);
}

QString zeitName(EchoZeit zeit)
{
    for (const ZeitEintrag& eintrag : kZeiten)
        if (eintrag.zeit == zeit) return QLatin1String(eintrag.name);
    return {};
}

QString artName(FilterArt art)
{
    return art == FilterArt::Hochpass   ? ClipEffektFenster::tr("High-pass")
           : art == FilterArt::Bandpass ? ClipEffektFenster::tr("Band-pass")
                                        : ClipEffektFenster::tr("Low-pass");
}

/// Wie ein Baustein in der Zeile heisst, und sein Kurzwert.
struct Kurz
{
    QString titel;
    QString wert;
    bool an;
};

/// Steht dieses Feld des Bausteins am Platz `platz` in allen Reihen gleich? Die Reihen haben
/// dieselbe Folge wie die gezeigte — an dem Platz steht ueberall dieselbe Art.
template <class Art, class Feld>
bool ueberall(const std::vector<ClipEffekte>& alle, std::size_t platz, Feld Art::* feld, const Art& gezeigt)
{
    return std::all_of(alle.begin(), alle.end(),
                       [&](const ClipEffekte& e) { return std::get<Art>(e.reihe[platz]).*feld == gezeigt.*feld; });
}

/// Die Teile eines Kurzwerts: wo die Clips abweichen, steht statt des Werts „mixed" — sind es
/// alle Teile, nur das eine Wort.
QString fuege(const QString& trenner, std::initializer_list<std::pair<QString, bool>> teile)
{
    const QString gemischt = ClipEffektFenster::tr("mixed");
    QStringList texte;
    bool einerGleich = false;
    for (const auto& [text, gleich] : teile)
    {
        texte << (gleich ? text : gemischt);
        einerGleich = einerGleich || gleich;
    }
    return einerGleich ? texte.join(trenner) : gemischt;
}

/// Der Baustein am Platz `platz` der gezeigten Reihe, mit Blick auf alle Reihen derselben Folge.
Kurz kurz(const ClipBaustein& baustein, const std::vector<ClipEffekte>& alle, std::size_t platz)
{
    const QString punkt = QString::fromUtf8(" · ");
    if (const auto* filter = std::get_if<ClipFilter>(&baustein))
        return {ClipEffektFenster::tr("Filter"),
                fuege(QStringLiteral(" "), {{artName(filter->art), ueberall(alle, platz, &ClipFilter::art, *filter)},
                                            {hertz(filter->hz), ueberall(alle, platz, &ClipFilter::hz, *filter)}}),
                filter->an};
    if (const auto* verzerrung = std::get_if<ClipVerzerrung>(&baustein))
        return {ClipEffektFenster::tr("Distortion"),
                fuege(punkt, {{ClipEffektFenster::tr("Drive %1").arg(prozent(verzerrung->drive)),
                               ueberall(alle, platz, &ClipVerzerrung::drive, *verzerrung)}}),
                verzerrung->an};
    if (const auto* echo = std::get_if<ClipEcho>(&baustein))
        return {ClipEffektFenster::tr("Echo"),
                fuege(punkt, {{zeitName(echo->zeit), ueberall(alle, platz, &ClipEcho::zeit, *echo)},
                              {prozent(echo->mix), ueberall(alle, platz, &ClipEcho::mix, *echo)}}),
                echo->an};
    const auto& hall = std::get<ClipHall>(baustein);
    return {ClipEffektFenster::tr("Reverb"),
            fuege(punkt, {{QStringLiteral("%1 s").arg(hall.sekunden, 0, 'f', 1), ueberall(alle, platz, &ClipHall::sekunden, hall)},
                          {prozent(hall.mix), ueberall(alle, platz, &ClipHall::mix, hall)}}),
            hall.an};
}

/// Ein Baustein fuer sich: sein Name, und ob er eingeschaltet ist.
Kurz kurz(const ClipBaustein& baustein)
{
    if (const auto* filter = std::get_if<ClipFilter>(&baustein))
        return {ClipEffektFenster::tr("Filter"), QStringLiteral("%1 %2").arg(artName(filter->art), hertz(filter->hz)), filter->an};
    if (const auto* verzerrung = std::get_if<ClipVerzerrung>(&baustein))
        return {ClipEffektFenster::tr("Distortion"), ClipEffektFenster::tr("Drive %1").arg(prozent(verzerrung->drive)),
                verzerrung->an};
    if (const auto* echo = std::get_if<ClipEcho>(&baustein))
        return {ClipEffektFenster::tr("Echo"), QString::fromUtf8("%1 · %2").arg(zeitName(echo->zeit), prozent(echo->mix)), echo->an};
    const auto& hall = std::get<ClipHall>(baustein);
    return {ClipEffektFenster::tr("Reverb"), QString::fromUtf8("%1 s · %2").arg(hall.sekunden, 0, 'f', 1).arg(prozent(hall.mix)),
            hall.an};
}

/// Haben zwei Reihen dieselbe Folge von Bausteinen — gleich viele, je Platz dieselbe Art?
bool gleicheFolge(const ClipEffekte& a, const ClipEffekte& b)
{
    return a.reihe.size() == b.reihe.size() &&
           std::equal(a.reihe.begin(), a.reihe.end(), b.reihe.begin(),
                      [](const ClipBaustein& x, const ClipBaustein& y) { return x.index() == y.index(); });
}

bool& anVon(ClipBaustein& baustein)
{
    return std::visit([](auto& werte) -> bool& { return werte.an; }, baustein);
}

}  // namespace

ClipEffektFenster::ClipEffektFenster(QWidget* parent) : QWidget(parent, Qt::Tool)
{
    setObjectName(QStringLiteral("loopClipEffects"));
    setWindowTitle(tr("Clip effects"));
    setupUI();
    fuelle();
}

void ClipEffektFenster::neuerRegler(QVBoxLayout* gruppe, const QString& name, const QString& kennung, int von, int bis,
                                    double faktor, std::function<double(const ClipEffekte&)> lies,
                                    std::function<void(ClipEffekte&, double)> setze,
                                    std::function<QString(double)> text)
{
    auto* zeile = new QHBoxLayout;
    auto* schild = new QLabel(name, this);
    schild->setMinimumWidth(72);
    auto* schieber = new QSlider(Qt::Horizontal, this);
    schieber->setObjectName(kennung);
    schieber->setRange(von, bis);
    schieber->setMinimumWidth(160);
    auto* anzeige = new QLabel(this);
    anzeige->setObjectName(kennung + QStringLiteral("Value"));
    anzeige->setMinimumWidth(56);
    zeile->addWidget(schild);
    zeile->addWidget(schieber, 1);
    zeile->addWidget(anzeige);
    gruppe->addLayout(zeile);

    const std::size_t nr = m_regler.size();
    m_regler.push_back({schieber, anzeige, faktor, std::move(lies), std::move(setze), std::move(text)});
    const auto melde = [this, nr] {
        if (m_fuellt) return;
        const Regler& regler = m_regler[nr];
        const double wert = regler.schieber->value() / regler.faktor;
        (void)aendere([&regler, wert](ClipEffekte& e) { regler.setze(e, wert); });
    };
    // im Zug laeuft nur die Anzeige mit; gemeldet wird beim Loslassen — und
    // bei jeder Aenderung ohne Zug (Tasten, Mausrad, Klick in die Bahn)
    connect(schieber, &QSlider::valueChanged, this, [this, nr, melde](int stand) {
        const Regler& regler = m_regler[nr];
        if (m_fuellt) return;
        regler.anzeige->setText(regler.text(stand / regler.faktor));
        if (!regler.schieber->isSliderDown()) melde();
    });
    connect(schieber, &QSlider::sliderReleased, this, melde);
}

template <class Art>
void ClipEffektFenster::reglerFuer(QVBoxLayout* gruppe, const QString& name, const QString& kennung, int von, int bis,
                                   double faktor, double Art::* feld, std::function<QString(double)> text)
{
    neuerRegler(
        gruppe, name, kennung, von, bis, faktor,
        [this, feld](const ClipEffekte& e) {
            const Art* werte = baustein<Art>(e);
            return werte != nullptr ? werte->*feld : Art{}.*feld;
        },
        [this, feld](ClipEffekte& e, double wert) {
            if (Art* werte = baustein<Art>(e)) werte->*feld = wert;
        },
        std::move(text));
}

void ClipEffektFenster::setupUI()
{
    auto* aussen = new QVBoxLayout(this);
    const auto fett = [](QWidget* teil) {
        QFont schrift = teil->font();
        schrift.setBold(true);
        teil->setFont(schrift);
    };
    const auto knopf = [this](const QString& text, const char* kennung, const QString& hilfe) {
        auto* k = new QPushButton(text, this);
        k->setObjectName(QLatin1String(kennung));
        k->setToolTip(hilfe);
        return k;
    };

    // --- der Kopf: wie viele Clips, welche, und die Spitze
    auto* kopf = new QHBoxLayout;
    m_kopf = new QLabel(this);
    m_kopf->setObjectName(QStringLiteral("fxHead"));
    fett(m_kopf);
    m_clips = new QLabel(this);
    m_clips->setObjectName(QStringLiteral("fxClips"));
    m_clips->setEnabled(false);  // grau: eine Auskunft
    // wird es eng, kuerzt die Liste der Namen — nicht die Zahl der Clips davor
    m_kopf->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    m_clips->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    m_spitze = new QLabel(QStringLiteral("–"), this);
    m_spitze->setObjectName(QStringLiteral("fxPeak"));
    m_spitze->setMinimumWidth(36);
    m_spitze->setToolTip(tr("Peak of the mixed loop while it plays, effects included; above 1 it clips"));
    // Showing (K5 a): nur zu sehen, wenn die markierten Clips verschiedene Reihen haben
    m_zeigtName = new QLabel(tr("Showing"), this);
    m_zeigtWahl = new QComboBox(this);
    m_zeigtWahl->setObjectName(QStringLiteral("fxShowing"));
    m_zeigtWahl->setToolTip(tr("The selected clips have different paths — this one is shown"));
    kopf->addWidget(m_kopf);
    kopf->addWidget(m_clips, 1);
    kopf->addWidget(m_zeigtName);
    kopf->addWidget(m_zeigtWahl);
    kopf->addWidget(new QLabel(tr("Peak"), this));
    kopf->addWidget(m_spitze);
    aussen->addLayout(kopf);
    connect(m_zeigtWahl, &QComboBox::activated, this, [this](int platz) { zeigeClip(m_zeigtWahl->itemData(platz).toInt()); });

    // --- der Hinweis bei verschiedenen Reihen
    auto* hinweisRahmen = new QFrame(this);
    hinweisRahmen->setObjectName(QStringLiteral("fxDifferRow"));
    hinweisRahmen->setFrameShape(QFrame::StyledPanel);
    m_hinweisZeile = hinweisRahmen;
    {
        auto* zeile = new QHBoxLayout(m_hinweisZeile);
        // zwei feste Zeilen, kein Umbruch: umbrochen war der Satz im Bild aus dem Test abgeschnitten
        m_hinweis = new QLabel(m_hinweisZeile);
        m_hinweis->setObjectName(QStringLiteral("fxDiffer"));
        m_angleichen = knopf(QString(), "fxUseForAll",
                             tr("Set the shown path, with its values, on every selected clip — their own effects are replaced"));
        zeile->addWidget(m_hinweis, 1);
        zeile->addWidget(m_angleichen);
    }
    aussen->addWidget(m_hinweisZeile);
    connect(m_angleichen, &QPushButton::clicked, this, [this] { (void)gleicheAn(); });

    // --- die Zeile Signal path
    m_pfad = new SignalWeg(this);
    m_pfad->setObjectName(QStringLiteral("fxPath"));
    aussen->addWidget(m_pfad);
    connect(m_pfad, &SignalWeg::gewaehltGeaendert, this, [this](int platz) {
        const int letzter = static_cast<int>(m_pfad->kaesten().size()) - 1;
        waehleStelle(platz <= 0 ? kStelleClip : platz >= letzter ? kStelleOut : platz - 1);
    });
    // die Plaetze der Zeile zaehlen den Kasten Clip mit: der erste Baustein steht dort auf Platz 1
    connect(m_pfad, &SignalWeg::verschoben, this, [this](int von, int nach) { (void)verschiebeBaustein(von - 1, nach - 1); });
    connect(m_pfad, &SignalWeg::addVerlangt, this, &ClipEffektFenster::zeigeAddMenue);

    // --- der gewaehlte Baustein: sein Haken, Move und Remove
    {
        auto* zeile = new QHBoxLayout;
        m_an = new QCheckBox(this);
        m_an->setObjectName(QStringLiteral("fxBlockOn"));
        fett(m_an);
        m_an->setToolTip(tr("On: the block works. Off: it stays in the path and is skipped"));
        auto* hinweis = new QLabel(tr("click a block · drag to move it"), this);
        hinweis->setObjectName(QStringLiteral("fxPathHint"));
        hinweis->setEnabled(false);
        m_bausteinVor = knopf(QString::fromUtf8("← Move"), "fxBlockLeft", tr("Move the selected block one place towards Clip"));
        m_bausteinZurueck = knopf(QString::fromUtf8("Move →"), "fxBlockRight", tr("Move the selected block one place towards Out"));
        m_bausteinWeg = knopf(tr("Remove"), "fxBlockRemove", tr("Remove the selected block from the path"));
        zeile->addWidget(m_an);
        zeile->addWidget(hinweis, 1);
        zeile->addWidget(m_bausteinVor);
        zeile->addWidget(m_bausteinZurueck);
        zeile->addWidget(m_bausteinWeg);
        aussen->addLayout(zeile);
        // `clicked`, nicht `toggled`: nur der Mensch meldet. Ein Haken, der
        // teils an zeigte, geht beim Klick fuer alle an.
        connect(m_an, &QCheckBox::clicked, this, [this] {
            const bool an = m_an->checkState() != Qt::Unchecked;
            const int platz = m_stelle;
            (void)aendere([an, platz](ClipEffekte& e) {
                if (platz >= 0 && platz < static_cast<int>(e.reihe.size())) anVon(e.reihe[static_cast<std::size_t>(platz)]) = an;
            });
        });
        connect(m_bausteinVor, &QPushButton::clicked, this, [this] { (void)schiebeBaustein(-1); });
        connect(m_bausteinZurueck, &QPushButton::clicked, this, [this] { (void)schiebeBaustein(1); });
        connect(m_bausteinWeg, &QPushButton::clicked, this, [this] { (void)entferneBaustein(); });
    }

    // --- die Regler: je Art eine Seite; das Fenster ist so hoch wie die hoechste
    m_seiten = new QStackedWidget(this);
    m_seiten->setObjectName(QStringLiteral("fxPages"));
    const auto neueSeite = [this] {
        auto* seite = new QWidget(m_seiten);
        auto* inhalt = new QVBoxLayout(seite);
        inhalt->setContentsMargins(0, 0, 0, 0);
        m_seiten->addWidget(seite);
        return inhalt;
    };
    {
        QVBoxLayout* leer = neueSeite();  // kSeiteLeer
        m_leerText = new QLabel(m_seiten);
        m_leerText->setObjectName(QStringLiteral("fxEmpty"));
        m_leerText->setEnabled(false);
        m_leerText->setWordWrap(true);
        leer->addWidget(m_leerText);
        leer->addStretch(1);
    }

    // --- Filter (Stufe 16d; K3 a): die Grenze in Hertz, sie folgt keiner Tonhoehe
    {
        QVBoxLayout* filter = neueSeite();
        auto* zeile = new QHBoxLayout;
        auto* schild = new QLabel(tr("Type"), this);
        schild->setMinimumWidth(72);
        m_filterArt = new QComboBox(this);
        m_filterArt->setObjectName(QStringLiteral("fxFilterType"));
        for (const FilterArt art : {FilterArt::Tiefpass, FilterArt::Hochpass, FilterArt::Bandpass})
            m_filterArt->addItem(artName(art), static_cast<int>(art));
        m_filterArt->setPlaceholderText(tr("mixed"));
        m_filterArt->setToolTip(
            tr("Low-pass keeps what lies below Cutoff, High-pass what lies above it, Band-pass what lies around it"));
        zeile->addWidget(schild);
        zeile->addWidget(m_filterArt, 1);
        filter->addLayout(zeile);
        connect(m_filterArt, &QComboBox::activated, this, [this](int platz) {
            if (platz < 0) return;
            const auto art = static_cast<FilterArt>(m_filterArt->itemData(platz).toInt());
            (void)aendere([this, art](ClipEffekte& e) {
                if (ClipFilter* werte = baustein<ClipFilter>(e)) werte->art = art;
            });
        });
        neuerRegler(
            filter, tr("Cutoff"), QStringLiteral("fxFilterCutoff"), 0, kCutoffStufen, 1.0,
            [this](const ClipEffekte& e) {
                const ClipFilter* werte = baustein<ClipFilter>(e);
                return stufeVon(werte != nullptr ? werte->hz : ClipFilter{}.hz);
            },
            [this](ClipEffekte& e, double stufe) {
                if (ClipFilter* werte = baustein<ClipFilter>(e)) werte->hz = hzVon(stufe);
            },
            [](double stufe) { return hertz(hzVon(stufe)); });
        reglerFuer<ClipFilter>(filter, tr("Resonance"), QStringLiteral("fxFilterResonance"), 0, 100, 100.0, &ClipFilter::resonanz,
                               prozent);
        filter->addStretch(1);
    }

    // --- Distortion
    {
        QVBoxLayout* verzerrer = neueSeite();
        reglerFuer<ClipVerzerrung>(verzerrer, tr("Drive"), QStringLiteral("fxDistDrive"), 0, 100, 100.0, &ClipVerzerrung::drive,
                                   prozent);
        reglerFuer<ClipVerzerrung>(verzerrer, tr("Tone"), QStringLiteral("fxDistTone"), 0, 100, 100.0, &ClipVerzerrung::ton, prozent);
        reglerFuer<ClipVerzerrung>(verzerrer, tr("Mix"), QStringLiteral("fxDistMix"), 0, 100, 100.0, &ClipVerzerrung::mix, prozent);
        verzerrer->addStretch(1);
    }

    // --- Echo
    {
        QVBoxLayout* echo = neueSeite();
        auto* zeile = new QHBoxLayout;
        auto* schild = new QLabel(tr("Time"), this);
        schild->setMinimumWidth(72);
        m_zeit = new QComboBox(this);
        m_zeit->setObjectName(QStringLiteral("fxEchoTime"));
        for (const ZeitEintrag& eintrag : kZeiten)
            m_zeit->addItem(QLatin1String(eintrag.name), static_cast<int>(eintrag.zeit));
        m_zeit->setPlaceholderText(tr("mixed"));
        m_zeit->setToolTip(tr("Time between the repeats, as a note value at the tempo of the loop"));
        m_zeitMs = new QLabel(this);
        m_zeitMs->setObjectName(QStringLiteral("fxEchoTimeValue"));
        m_zeitMs->setMinimumWidth(56);
        zeile->addWidget(schild);
        zeile->addWidget(m_zeit, 1);
        zeile->addWidget(m_zeitMs);
        echo->addLayout(zeile);
        connect(m_zeit, &QComboBox::activated, this, [this](int platz) {
            if (platz < 0) return;
            const auto zeit = static_cast<EchoZeit>(m_zeit->itemData(platz).toInt());
            (void)aendere([this, zeit](ClipEffekte& e) {
                if (ClipEcho* werte = baustein<ClipEcho>(e)) werte->zeit = zeit;
            });
        });
        reglerFuer<ClipEcho>(echo, tr("Feedback"), QStringLiteral("fxEchoFeedback"), 0,
                             static_cast<int>(std::lround(kEchoFeedbackMax * 100)), 100.0, &ClipEcho::feedback, prozent);
        reglerFuer<ClipEcho>(echo, tr("Damping"), QStringLiteral("fxEchoDamping"), 0, 100, 100.0, &ClipEcho::daempfung, prozent);
        reglerFuer<ClipEcho>(echo, tr("Mix"), QStringLiteral("fxEchoMix"), 0, 100, 100.0, &ClipEcho::mix, prozent);
        echo->addStretch(1);
    }

    // --- Reverb
    {
        QVBoxLayout* hall = neueSeite();
        reglerFuer<ClipHall>(hall, tr("Size"), QStringLiteral("fxReverbSize"), 0, 100, 100.0, &ClipHall::groesse, prozent);
        reglerFuer<ClipHall>(hall, tr("Decay"), QStringLiteral("fxReverbDecay"), static_cast<int>(std::lround(kHallSekMin * 10)),
                             static_cast<int>(std::lround(kHallSekMax * 10)), 10.0, &ClipHall::sekunden,
                             [](double wert) { return QStringLiteral("%1 s").arg(wert, 0, 'f', 1); });
        reglerFuer<ClipHall>(hall, tr("Damping"), QStringLiteral("fxReverbDamping"), 0, 100, 100.0, &ClipHall::daempfung, prozent);
        reglerFuer<ClipHall>(hall, tr("Mix"), QStringLiteral("fxReverbMix"), 0, 100, 100.0, &ClipHall::mix, prozent);
        hall->addStretch(1);
    }
    aussen->addWidget(m_seiten, 1);

    auto* fuss = new QHBoxLayout;
    auto* merke = new QLabel(tr("Mix 0 % leaves the clip as it is"), this);
    merke->setEnabled(false);
    m_kopieren = knopf(tr("Copy"), "fxCopy", tr("Remember the path of the shown clip"));
    m_einfuegen = knopf(tr("Paste"), "fxPaste", tr("Set the remembered path on all selected clips"));
    m_zurueck = knopf(tr("Reset"), "fxReset", tr("Remove all effects of the selected clips"));
    fuss->addWidget(merke, 1);
    fuss->addWidget(m_kopieren);
    fuss->addWidget(m_einfuegen);
    fuss->addWidget(m_zurueck);
    aussen->addLayout(fuss);
    connect(m_kopieren, &QPushButton::clicked, this, [this] { (void)kopiere(); });
    connect(m_einfuegen, &QPushButton::clicked, this, [this] { (void)fuegeEin(); });
    connect(m_zurueck, &QPushButton::clicked, this, [this] { (void)setzeZurueck(); });
    setMinimumWidth(520);  // Clip, drei Bausteine, Add und Out in einer Zeile
}

const ClipEffekte* ClipEffektFenster::effekteVon(const Loop& loop, int platz) const
{
    if (platz < 0 || platz >= static_cast<int>(m_orte.size())) return nullptr;
    const ClipOrt& ort = m_orte[static_cast<std::size_t>(platz)];
    const int clip = findeClip(loop, ort);
    if (clip < 0) return nullptr;
    return &loop.kanaele[static_cast<std::size_t>(ort.kanal)].clips[static_cast<std::size_t>(clip)].effekte;
}

ClipEffekte ClipEffektFenster::gezeigteReihe() const
{
    const ClipEffekte* e = effekteVon(m_loop, m_zeigt);
    return e != nullptr ? *e : ClipEffekte{};
}

std::vector<int> ClipEffektFenster::gleiche() const
{
    std::vector<int> plaetze;
    const ClipEffekte* gezeigt = effekteVon(m_loop, m_zeigt);
    if (gezeigt == nullptr) return plaetze;
    for (int i = 0; i < static_cast<int>(m_orte.size()); ++i)
        if (const ClipEffekte* e = effekteVon(m_loop, i); e != nullptr && gleicheFolge(*e, *gezeigt)) plaetze.push_back(i);
    return plaetze;
}

std::vector<ClipEffekte> ClipEffektFenster::gleicheEffekte() const
{
    std::vector<ClipEffekte> alle;
    for (const int platz : gleiche()) alle.push_back(*effekteVon(m_loop, platz));
    return alle;
}

QString ClipEffektFenster::clipName(int platz) const
{
    if (platz < 0 || platz >= static_cast<int>(m_orte.size())) return {};
    const ClipOrt& ort = m_orte[static_cast<std::size_t>(platz)];
    const int clip = findeClip(m_loop, ort);
    if (clip < 0) return {};
    const LoopClip& eintrag = m_loop.kanaele[static_cast<std::size_t>(ort.kanal)].clips[static_cast<std::size_t>(clip)];
    return tr("%1 (bar %2)").arg(eintrag.muster).arg(eintrag.startTakt + 1);
}

void ClipEffektFenster::zeige(const Loop& loop, const std::vector<ClipOrt>& orte)
{
    m_loop = loop;
    // erst sammeln, dann setzen: `orte` kann die eigene Liste sein
    std::vector<ClipOrt> gibtEs;
    for (const ClipOrt& ort : orte)
        if (findeClip(m_loop, ort) >= 0) gibtEs.push_back(ort);
    const bool andere = !(gibtEs == m_orte);
    m_orte = std::move(gibtEs);
    if (andere)
    {
        // andere Clips: der erste wird gezeigt, gewaehlt ist sein erster Baustein
        m_zeigt = 0;
        m_stelle = gezeigteReihe().reihe.empty() ? kStelleClip : 0;
    }
    fuelle();
}

void ClipEffektFenster::waehleStelle(int stelle)
{
    const int zahl = static_cast<int>(gezeigteReihe().reihe.size());
    const int neu = stelle == kStelleOut || (stelle >= 0 && stelle < zahl) ? stelle : kStelleClip;
    if (neu == m_stelle) return;
    m_stelle = neu;
    fuelle();
}

void ClipEffektFenster::zeigeClip(int platz)
{
    if (platz < 0 || platz >= static_cast<int>(m_orte.size()) || platz == m_zeigt) return;
    m_zeigt = platz;
    m_stelle = gezeigteReihe().reihe.empty() ? kStelleClip : 0;
    fuelle();
}

void ClipEffektFenster::fuelle()
{
    m_fuellt = true;
    const bool leer = m_orte.empty();
    m_zeigt = leer ? 0 : std::clamp(m_zeigt, 0, static_cast<int>(m_orte.size()) - 1);
    const ClipEffekte gezeigt = gezeigteReihe();
    const int zahl = static_cast<int>(gezeigt.reihe.size());
    if (m_stelle >= zahl || (m_stelle < 0 && m_stelle != kStelleOut)) m_stelle = kStelleClip;
    const std::vector<int> mit = gleiche();
    const std::vector<ClipEffekte> alle = gleicheEffekte();
    const bool verschieden = mit.size() < m_orte.size();

    m_pfad->setEnabled(!leer);
    m_kopieren->setEnabled(!leer);
    m_einfuegen->setEnabled(!leer && m_gemerkt.has_value());
    m_zurueck->setEnabled(!leer);

    // --- der Kopf
    if (leer)
    {
        m_kopf->setText(tr("No clip selected"));
        m_clips->setText(tr("Select one or more clips in the Loop panel"));
    }
    else
    {
        m_kopf->setText(m_orte.size() == 1 ? tr("Signal path — 1 clip") : tr("Signal path — %1 clips").arg(m_orte.size()));
        // welche: Basis und Takt, hoechstens drei beim Namen
        QStringList namen;
        for (int i = 0; i < static_cast<int>(m_orte.size()) && i < 3; ++i) namen << clipName(i);
        if (m_orte.size() > 3) namen << tr("+ %1 more").arg(m_orte.size() - 3);
        m_clips->setText(namen.join(QString::fromUtf8(" · ")));
    }

    // --- Showing und der Hinweis: nur bei verschiedenen Reihen (K5 a)
    m_zeigtName->setVisible(verschieden);
    m_zeigtWahl->setVisible(verschieden);
    m_hinweisZeile->setVisible(verschieden);
    {
        const QSignalBlocker ruhe(m_zeigtWahl);
        m_zeigtWahl->clear();
        for (int i = 0; i < static_cast<int>(m_orte.size()); ++i) m_zeigtWahl->addItem(clipName(i), i);
        m_zeigtWahl->setCurrentIndex(m_zeigt);
    }
    if (verschieden)
    {
        QStringList andere;
        for (int i = 0; i < static_cast<int>(m_orte.size()); ++i)
            if (std::find(mit.begin(), mit.end(), i) == mit.end()) andere << clipName(i);
        const int gesamt = static_cast<int>(m_orte.size());
        const QString welche =
            andere.size() <= 2 ? andere.join(QStringLiteral(", ")) : tr("%1 other clips").arg(andere.size());
        m_hinweis->setText((mit.size() == 1 ? tr("1 of %1 clips has this path.").arg(gesamt)
                                            : tr("%1 of %2 clips have this path.").arg(mit.size()).arg(gesamt)) +
                           QLatin1Char('\n') + tr("Not changed from here: %1").arg(welche));
        m_hinweis->setToolTip(tr("Controls, Add, Move and Remove apply to the clips with the shown path. The others "
                                 "have a different path and stay as they are"));
        m_angleichen->setText(tr("Use this path for all %1").arg(gesamt));
    }

    // --- die Zeile: Clip, die Bausteine des gezeigten Clips, Add, Out
    {
        std::vector<WegKasten> kaesten;
        kaesten.push_back(WegKasten{tr("Clip"), tr("dry"), false, false, tr("The clip as its pattern plays it")});
        for (int i = 0; i < zahl; ++i)
        {
            const ClipBaustein& baustein = gezeigt.reihe[static_cast<std::size_t>(i)];
            // der Kurzwert des gezeigten Clips; wo einer mit derselben Folge abweicht, steht „mixed“
            const Kurz k = kurz(baustein, alle, static_cast<std::size_t>(i));
            kaesten.push_back(WegKasten{k.titel, k.wert, false, !k.an,
                                        k.an ? QString() : tr("Switched off: it stays in the path and is skipped")});
        }
        kaesten.push_back(
            WegKasten{tr("Out"), m_spitzenWert < 0.0 ? QString::fromUtf8("–") : tr("Peak %1").arg(m_spitzenWert, 0, 'f', 2), false,
                      false, tr("What goes into the mix of the loop")});
        const bool voll = zahl >= kBausteineMax;
        const int gewaehlt = m_stelle == kStelleOut ? zahl + 1 : m_stelle < 0 ? 0 : m_stelle + 1;
        m_pfad->setKaesten(std::move(kaesten), leer ? -1 : gewaehlt, !leer && !voll,
                           voll ? tr("%1 of %1").arg(kBausteineMax) : tr("+ Add"));
    }

    // --- der gewaehlte Baustein
    const bool gewaehlt = !leer && m_stelle >= 0;
    m_an->setVisible(gewaehlt);
    m_bausteinVor->setVisible(gewaehlt);
    m_bausteinZurueck->setVisible(gewaehlt);
    m_bausteinWeg->setVisible(gewaehlt);
    m_bausteinVor->setEnabled(gewaehlt && m_stelle > 0);
    m_bausteinZurueck->setEnabled(gewaehlt && m_stelle + 1 < zahl);
    if (gewaehlt)
    {
        const auto platz = static_cast<std::size_t>(m_stelle);
        m_an->setText(kurz(gezeigt.reihe[platz]).titel);
        int an = 0;
        for (const ClipEffekte& e : alle)
            if (kurz(e.reihe[platz]).an) ++an;
        const QSignalBlocker ruhe(m_an);
        // teils an: der Haken zeigt es; ein Klick schaltet dann alle an
        m_an->setTristate(an > 0 && an < static_cast<int>(alle.size()));
        m_an->setCheckState(an == 0 ? Qt::Unchecked : an == static_cast<int>(alle.size()) ? Qt::Checked : Qt::PartiallyChecked);
        m_seiten->setCurrentIndex(static_cast<int>(gezeigt.reihe[platz].index()) + 1);
    }
    else
    {
        m_seiten->setCurrentIndex(kSeiteLeer);
        m_leerText->setText(
            leer        ? QString()
            : zahl == 0 ? tr("No effects on this clip. + Add puts a Filter, Distortion, Echo or Reverb into the path.")
                        : tr("Click a block in the path to see its controls."));
    }

    // --- die Regler: der Wert des gezeigten Clips; weicht einer mit derselben Folge ab, steht „mixed" da
    for (const Regler& regler : m_regler)
    {
        const double erster = regler.lies(gezeigt);
        const bool gleich = std::all_of(alle.begin(), alle.end(), [&](const ClipEffekte& e) {
            return std::lround(regler.lies(e) * regler.faktor) == std::lround(erster * regler.faktor);
        });
        const QSignalBlocker ruhe(regler.schieber);
        regler.schieber->setValue(static_cast<int>(std::lround(erster * regler.faktor)));
        regler.anzeige->setText(leer ? QString() : gleich ? regler.text(erster) : tr("mixed"));
    }
    {
        const ClipEcho* echo = baustein<ClipEcho>(gezeigt);
        const EchoZeit erste = echo != nullptr ? echo->zeit : ClipEcho{}.zeit;
        const bool gleich = std::all_of(alle.begin(), alle.end(), [&](const ClipEffekte& e) {
            const ClipEcho* anderes = baustein<ClipEcho>(e);
            return anderes == nullptr || anderes->zeit == erste;
        });
        const QSignalBlocker ruhe(m_zeit);
        m_zeit->setCurrentIndex(gleich ? m_zeit->findData(static_cast<int>(erste)) : -1);
        // die Zeit in Millisekunden, im Tempo des Loops
        m_zeitMs->setText(leer || !gleich || m_loop.bpm <= 0.0
                              ? QString()
                              : tr("%1 ms").arg(std::lround(echoSchlaege(erste) * 60000.0 / m_loop.bpm)));
    }
    {
        const ClipFilter* filter = baustein<ClipFilter>(gezeigt);
        const FilterArt erste = filter != nullptr ? filter->art : FilterArt::Tiefpass;
        const bool gleich = std::all_of(alle.begin(), alle.end(), [&](const ClipEffekte& e) {
            const ClipFilter* anderer = baustein<ClipFilter>(e);
            return anderer == nullptr || anderer->art == erste;
        });
        const QSignalBlocker ruhe(m_filterArt);
        m_filterArt->setCurrentIndex(gleich ? m_filterArt->findData(static_cast<int>(erste)) : -1);
    }
    m_fuellt = false;
}

bool ClipEffektFenster::melde(const Loop& neu)
{
    if (neu == m_loop)
    {
        fuelle();  // ein Regler steht vielleicht anders, als die Clips sind
        return false;
    }
    m_loop = neu;
    fuelle();
    Q_EMIT bearbeitet(neu, tr("Clip effects"));
    return true;
}

bool ClipEffektFenster::aendere(const std::function<void(ClipEffekte&)>& was)
{
    if (m_orte.empty()) return false;
    Loop neu = m_loop;
    for (const int platz : gleiche())
    {
        const ClipOrt& ort = m_orte[static_cast<std::size_t>(platz)];
        const int clip = findeClip(neu, ort);
        if (clip < 0) continue;
        was(neu.kanaele[static_cast<std::size_t>(ort.kanal)].clips[static_cast<std::size_t>(clip)].effekte);
    }
    return melde(neu);
}

bool ClipEffektFenster::aendereAlle(const std::function<void(ClipEffekte&)>& was)
{
    if (m_orte.empty()) return false;
    Loop neu = m_loop;
    for (const ClipOrt& ort : m_orte)
    {
        const int clip = findeClip(neu, ort);
        if (clip < 0) continue;
        was(neu.kanaele[static_cast<std::size_t>(ort.kanal)].clips[static_cast<std::size_t>(clip)].effekte);
    }
    return melde(neu);
}

bool ClipEffektFenster::neuerBaustein(const ClipBaustein& baustein)
{
    if (m_orte.empty()) return false;
    const int zahl = static_cast<int>(gezeigteReihe().reihe.size());
    if (zahl >= kBausteineMax) return false;
    // hinter dem gewaehlten: bei Clip vorn, bei Out hinten
    const int platz = m_stelle == kStelleOut ? zahl : m_stelle < 0 ? 0 : std::min(m_stelle + 1, zahl);
    ClipBaustein neu = baustein;
    anVon(neu) = true;  // wer einen Baustein holt, will ihn hoeren
    m_stelle = platz;   // vor dem Aendern: die Anzeige soll schon den neuen zeigen
    return aendere([&neu, platz](ClipEffekte& e) { e.reihe.insert(e.reihe.begin() + platz, neu); });
}

bool ClipEffektFenster::verschiebeBaustein(int von, int nach)
{
    const int zahl = static_cast<int>(gezeigteReihe().reihe.size());
    if (von < 0 || von >= zahl || nach < 0 || nach >= zahl) return false;
    m_stelle = nach;
    if (von == nach)
    {
        fuelle();
        return true;
    }
    (void)aendere([von, nach](ClipEffekte& e) {
        const ClipBaustein baustein = e.reihe[static_cast<std::size_t>(von)];
        e.reihe.erase(e.reihe.begin() + von);
        e.reihe.insert(e.reihe.begin() + nach, baustein);
    });
    return true;
}

bool ClipEffektFenster::schiebeBaustein(int richtung)
{
    if (m_stelle < 0) return false;
    const int ziel = m_stelle + (richtung < 0 ? -1 : 1);
    if (ziel < 0 || ziel >= static_cast<int>(gezeigteReihe().reihe.size())) return false;
    return verschiebeBaustein(m_stelle, ziel);
}

bool ClipEffektFenster::entferneBaustein()
{
    const int zahl = static_cast<int>(gezeigteReihe().reihe.size());
    if (m_stelle < 0 || m_stelle >= zahl) return false;
    const int platz = m_stelle;
    // danach ist gewaehlt, was an seinen Platz rueckt — oder der davor, oder der Kasten Clip
    m_stelle = zahl <= 1 ? kStelleClip : std::min(platz, zahl - 2);
    return aendere([platz](ClipEffekte& e) { e.reihe.erase(e.reihe.begin() + platz); });
}

bool ClipEffektFenster::gleicheAn()
{
    if (m_orte.empty()) return false;
    const ClipEffekte gezeigt = gezeigteReihe();
    return aendereAlle([&gezeigt](ClipEffekte& e) { e = gezeigt; });
}

void ClipEffektFenster::zeigeAddMenue(QPoint unten)
{
    QMenu menue(this);
    menue.setObjectName(QStringLiteral("fxAddBlockMenu"));
    connect(menue.addAction(tr("Filter")), &QAction::triggered, this, [this] { (void)neuerBaustein(ClipFilter{}); });
    connect(menue.addAction(tr("Distortion")), &QAction::triggered, this, [this] { (void)neuerBaustein(ClipVerzerrung{}); });
    connect(menue.addAction(tr("Echo")), &QAction::triggered, this, [this] { (void)neuerBaustein(ClipEcho{}); });
    connect(menue.addAction(tr("Reverb")), &QAction::triggered, this, [this] { (void)neuerBaustein(ClipHall{}); });
    menue.exec(m_pfad->mapToGlobal(unten));
}

bool ClipEffektFenster::kopiere()
{
    if (m_orte.empty()) return false;
    m_gemerkt = gezeigteReihe();
    m_einfuegen->setEnabled(true);
    return true;
}

bool ClipEffektFenster::fuegeEin()
{
    if (!m_gemerkt.has_value()) return false;
    return aendereAlle([this](ClipEffekte& e) { e = *m_gemerkt; });
}

bool ClipEffektFenster::setzeZurueck()
{
    return aendereAlle([](ClipEffekte& e) { e = ClipEffekte{}; });
}

void ClipEffektFenster::setSpitze(double spitze)
{
    const QString text = spitze < 0.0 ? QStringLiteral("–") : QString::number(spitze, 'f', 2);
    if (text == m_spitze->text()) return;
    m_spitze->setText(text);
    // ueber 1 uebersteuert die Summe (L6 a)
    m_spitze->setStyleSheet(spitze > 1.0 ? QStringLiteral("color: #e05050; font-weight: bold;") : QString());
    // der Kasten Out zeigt sie auch — nur sein Wert wird neu gesetzt: die Spitze kommt laufend,
    // waehrend jemand zieht oder eine Auswahl offen hat
    m_spitzenWert = spitze;
    m_pfad->setKastenWert(static_cast<int>(m_pfad->kaesten().size()) - 1,
                          spitze < 0.0 ? QString::fromUtf8("–") : tr("Peak %1").arg(spitze, 0, 'f', 2));
}

}  // namespace lumi::loop
