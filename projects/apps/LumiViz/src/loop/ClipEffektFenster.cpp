/**
 * @file ClipEffektFenster.cpp
 * @brief Loop: das Fenster „Clip effects"
 */

#include "loop/ClipEffektFenster.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSlider>
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

QString prozent(double wert)
{
    return QStringLiteral("%1 %").arg(std::lround(wert * 100.0));
}

}  // namespace

ClipEffektFenster::ClipEffektFenster(QWidget* parent) : QWidget(parent, Qt::Tool)
{
    setObjectName(QStringLiteral("loopClipEffects"));
    setWindowTitle(tr("Clip effects"));
    setupUI();
    fuelle();
}

QCheckBox* ClipEffektFenster::neuerSchalter(QVBoxLayout* gruppe, const QString& titel, const QString& kennung,
                                            std::function<bool(const ClipEffekte&)> lies,
                                            std::function<void(ClipEffekte&, bool)> setze)
{
    auto* haken = new QCheckBox(titel, this);
    haken->setObjectName(kennung);
    QFont fett = haken->font();
    fett.setBold(true);
    haken->setFont(fett);
    haken->setToolTip(tr("Turns %1 on or off for the selected clips").arg(titel));
    gruppe->addWidget(haken);
    const std::size_t nr = m_schalter.size();
    m_schalter.push_back({haken, std::move(lies), std::move(setze)});
    // `clicked`, nicht `toggled`: nur der Mensch meldet. Ein Haken, der
    // „teils an" zeigte, geht beim Klick fuer alle an.
    connect(haken, &QCheckBox::clicked, this, [this, nr] {
        const Schalter& schalter = m_schalter[nr];
        const bool an = schalter.haken->checkState() != Qt::Unchecked;
        (void)aendere([&schalter, an](ClipEffekte& e) { schalter.setze(e, an); });
    });
    return haken;
}

void ClipEffektFenster::neuerRegler(QVBoxLayout* gruppe, const QString& name, const QString& kennung, int von, int bis,
                                    double faktor, std::function<double(const ClipEffekte&)> lies,
                                    std::function<void(ClipEffekte&, double)> setze,
                                    std::function<QString(double)> text)
{
    auto* zeile = new QHBoxLayout;
    auto* schild = new QLabel(name, this);
    schild->setMinimumWidth(64);
    auto* schieber = new QSlider(Qt::Horizontal, this);
    schieber->setObjectName(kennung);
    schieber->setRange(von, bis);
    schieber->setMinimumWidth(90);
    auto* anzeige = new QLabel(this);
    anzeige->setObjectName(kennung + QStringLiteral("Value"));
    anzeige->setMinimumWidth(52);
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

void ClipEffektFenster::setupUI()
{
    auto* aussen = new QVBoxLayout(this);

    auto* kopf = new QHBoxLayout;
    m_kopf = new QLabel(this);
    m_kopf->setObjectName(QStringLiteral("fxHead"));
    QFont fett = m_kopf->font();
    fett.setBold(true);
    m_kopf->setFont(fett);
    m_clips = new QLabel(this);
    m_clips->setObjectName(QStringLiteral("fxClips"));
    m_clips->setEnabled(false);  // grau: eine Auskunft
    m_spitze = new QLabel(QStringLiteral("–"), this);
    m_spitze->setObjectName(QStringLiteral("fxPeak"));
    m_spitze->setMinimumWidth(36);
    m_spitze->setToolTip(tr("Peak of the mixed loop while it plays, effects included; above 1 it clips"));
    kopf->addWidget(m_kopf);
    kopf->addWidget(m_clips, 1);
    kopf->addWidget(new QLabel(tr("Peak"), this));
    kopf->addWidget(m_spitze);
    aussen->addLayout(kopf);

    m_gruppen = new QWidget(this);
    auto* gruppen = new QHBoxLayout(m_gruppen);
    gruppen->setContentsMargins(0, 0, 0, 0);
    const auto neueGruppe = [this, gruppen] {
        auto* rahmen = new QFrame(m_gruppen);
        rahmen->setFrameShape(QFrame::StyledPanel);
        auto* inhalt = new QVBoxLayout(rahmen);
        gruppen->addWidget(rahmen, 1);
        return inhalt;
    };

    // --- Distortion
    QVBoxLayout* verzerrer = neueGruppe();
    neuerSchalter(
        verzerrer, tr("Distortion"), QStringLiteral("fxDistOn"), [](const ClipEffekte& e) { return e.verzerrung.an; },
        [](ClipEffekte& e, bool an) { e.verzerrung.an = an; });
    neuerRegler(
        verzerrer, tr("Drive"), QStringLiteral("fxDistDrive"), 0, 100, 100.0,
        [](const ClipEffekte& e) { return e.verzerrung.drive; }, [](ClipEffekte& e, double w) { e.verzerrung.drive = w; },
        prozent);
    neuerRegler(
        verzerrer, tr("Tone"), QStringLiteral("fxDistTone"), 0, 100, 100.0,
        [](const ClipEffekte& e) { return e.verzerrung.ton; }, [](ClipEffekte& e, double w) { e.verzerrung.ton = w; },
        prozent);
    neuerRegler(
        verzerrer, tr("Mix"), QStringLiteral("fxDistMix"), 0, 100, 100.0,
        [](const ClipEffekte& e) { return e.verzerrung.mix; }, [](ClipEffekte& e, double w) { e.verzerrung.mix = w; },
        prozent);
    verzerrer->addStretch(1);

    // --- Echo
    QVBoxLayout* echo = neueGruppe();
    neuerSchalter(
        echo, tr("Echo"), QStringLiteral("fxEchoOn"), [](const ClipEffekte& e) { return e.echo.an; },
        [](ClipEffekte& e, bool an) { e.echo.an = an; });
    {
        auto* zeile = new QHBoxLayout;
        auto* schild = new QLabel(tr("Time"), this);
        schild->setMinimumWidth(64);
        m_zeit = new QComboBox(this);
        m_zeit->setObjectName(QStringLiteral("fxEchoTime"));
        for (const ZeitEintrag& eintrag : kZeiten)
            m_zeit->addItem(QLatin1String(eintrag.name), static_cast<int>(eintrag.zeit));
        m_zeit->setPlaceholderText(tr("mixed"));
        m_zeit->setToolTip(tr("Time between the repeats, as a note value at the tempo of the loop"));
        m_zeitMs = new QLabel(this);
        m_zeitMs->setObjectName(QStringLiteral("fxEchoTimeValue"));
        m_zeitMs->setMinimumWidth(52);
        zeile->addWidget(schild);
        zeile->addWidget(m_zeit, 1);
        zeile->addWidget(m_zeitMs);
        echo->addLayout(zeile);
        connect(m_zeit, &QComboBox::activated, this, [this](int platz) {
            if (platz < 0) return;
            const auto zeit = static_cast<EchoZeit>(m_zeit->itemData(platz).toInt());
            (void)aendere([zeit](ClipEffekte& e) { e.echo.zeit = zeit; });
        });
    }
    neuerRegler(
        echo, tr("Feedback"), QStringLiteral("fxEchoFeedback"), 0, static_cast<int>(std::lround(kEchoFeedbackMax * 100)),
        100.0, [](const ClipEffekte& e) { return e.echo.feedback; }, [](ClipEffekte& e, double w) { e.echo.feedback = w; },
        prozent);
    neuerRegler(
        echo, tr("Damping"), QStringLiteral("fxEchoDamping"), 0, 100, 100.0,
        [](const ClipEffekte& e) { return e.echo.daempfung; }, [](ClipEffekte& e, double w) { e.echo.daempfung = w; },
        prozent);
    neuerRegler(
        echo, tr("Mix"), QStringLiteral("fxEchoMix"), 0, 100, 100.0, [](const ClipEffekte& e) { return e.echo.mix; },
        [](ClipEffekte& e, double w) { e.echo.mix = w; }, prozent);

    // --- Reverb
    QVBoxLayout* hall = neueGruppe();
    neuerSchalter(
        hall, tr("Reverb"), QStringLiteral("fxReverbOn"), [](const ClipEffekte& e) { return e.hall.an; },
        [](ClipEffekte& e, bool an) { e.hall.an = an; });
    neuerRegler(
        hall, tr("Size"), QStringLiteral("fxReverbSize"), 0, 100, 100.0,
        [](const ClipEffekte& e) { return e.hall.groesse; }, [](ClipEffekte& e, double w) { e.hall.groesse = w; },
        prozent);
    neuerRegler(
        hall, tr("Decay"), QStringLiteral("fxReverbDecay"), static_cast<int>(std::lround(kHallSekMin * 10)),
        static_cast<int>(std::lround(kHallSekMax * 10)), 10.0, [](const ClipEffekte& e) { return e.hall.sekunden; },
        [](ClipEffekte& e, double w) { e.hall.sekunden = w; },
        [](double wert) { return QStringLiteral("%1 s").arg(wert, 0, 'f', 1); });
    neuerRegler(
        hall, tr("Damping"), QStringLiteral("fxReverbDamping"), 0, 100, 100.0,
        [](const ClipEffekte& e) { return e.hall.daempfung; }, [](ClipEffekte& e, double w) { e.hall.daempfung = w; },
        prozent);
    neuerRegler(
        hall, tr("Mix"), QStringLiteral("fxReverbMix"), 0, 100, 100.0, [](const ClipEffekte& e) { return e.hall.mix; },
        [](ClipEffekte& e, double w) { e.hall.mix = w; }, prozent);
    aussen->addWidget(m_gruppen);

    auto* fuss = new QHBoxLayout;
    auto* reihenfolge = new QLabel(tr("Order: Distortion, Echo, Reverb · Mix 0 % leaves the clip as it is"), this);
    reihenfolge->setEnabled(false);
    m_kopieren = new QPushButton(tr("Copy"), this);
    m_kopieren->setObjectName(QStringLiteral("fxCopy"));
    m_kopieren->setToolTip(tr("Remember the effects of the first selected clip"));
    m_einfuegen = new QPushButton(tr("Paste"), this);
    m_einfuegen->setObjectName(QStringLiteral("fxPaste"));
    m_einfuegen->setToolTip(tr("Set the remembered effects on all selected clips"));
    m_zurueck = new QPushButton(tr("Reset"), this);
    m_zurueck->setObjectName(QStringLiteral("fxReset"));
    m_zurueck->setToolTip(tr("Turn all effects of the selected clips off and back to their defaults"));
    fuss->addWidget(reihenfolge, 1);
    fuss->addWidget(m_kopieren);
    fuss->addWidget(m_einfuegen);
    fuss->addWidget(m_zurueck);
    aussen->addLayout(fuss);
    connect(m_kopieren, &QPushButton::clicked, this, [this] { (void)kopiere(); });
    connect(m_einfuegen, &QPushButton::clicked, this, [this] { (void)fuegeEin(); });
    connect(m_zurueck, &QPushButton::clicked, this, [this] { (void)setzeZurueck(); });
}

std::vector<ClipEffekte> ClipEffektFenster::effekte() const
{
    std::vector<ClipEffekte> alle;
    for (const ClipOrt& ort : m_orte)
    {
        const int platz = findeClip(m_loop, ort);
        if (platz >= 0)
            alle.push_back(
                m_loop.kanaele[static_cast<std::size_t>(ort.kanal)].clips[static_cast<std::size_t>(platz)].effekte);
    }
    return alle;
}

void ClipEffektFenster::zeige(const Loop& loop, const std::vector<ClipOrt>& orte)
{
    m_loop = loop;
    // erst sammeln, dann setzen: `orte` kann die eigene Liste sein
    std::vector<ClipOrt> gibtEs;
    for (const ClipOrt& ort : orte)
        if (findeClip(m_loop, ort) >= 0) gibtEs.push_back(ort);
    m_orte = std::move(gibtEs);
    fuelle();
}

void ClipEffektFenster::fuelle()
{
    m_fuellt = true;
    const std::vector<ClipEffekte> alle = effekte();
    const bool leer = alle.empty();
    m_gruppen->setEnabled(!leer);
    m_kopieren->setEnabled(!leer);
    m_einfuegen->setEnabled(!leer && m_gemerkt.has_value());
    m_zurueck->setEnabled(!leer);

    if (leer)
    {
        m_kopf->setText(tr("No clip selected"));
        m_clips->setText(tr("Select one or more clips in the Loop panel"));
    }
    else
    {
        m_kopf->setText(alle.size() == 1 ? tr("1 clip") : tr("%1 clips").arg(alle.size()));
        // welche: Basis und Takt, hoechstens drei beim Namen
        QStringList namen;
        for (std::size_t i = 0; i < m_orte.size() && i < 3; ++i)
        {
            const int platz = findeClip(m_loop, m_orte[i]);
            const LoopClip& clip =
                m_loop.kanaele[static_cast<std::size_t>(m_orte[i].kanal)].clips[static_cast<std::size_t>(platz)];
            namen << tr("%1 (bar %2)").arg(clip.muster).arg(clip.startTakt + 1);
        }
        if (m_orte.size() > 3) namen << tr("+ %1 more").arg(m_orte.size() - 3);
        m_clips->setText(namen.join(QStringLiteral(" · ")));
    }

    for (const Schalter& schalter : m_schalter)
    {
        int an = 0;
        for (const ClipEffekte& e : alle)
            if (schalter.lies(e)) ++an;
        const QSignalBlocker ruhe(schalter.haken);
        // teils an: der Haken zeigt es; ein Klick schaltet dann alle an
        schalter.haken->setTristate(an > 0 && an < static_cast<int>(alle.size()));
        schalter.haken->setCheckState(an == 0 ? Qt::Unchecked
                                      : an == static_cast<int>(alle.size()) ? Qt::Checked
                                                                           : Qt::PartiallyChecked);
    }
    for (const Regler& regler : m_regler)
    {
        const double erster = leer ? regler.lies(ClipEffekte{}) : regler.lies(alle.front());
        const bool gleich = std::all_of(alle.begin(), alle.end(), [&](const ClipEffekte& e) {
            return std::lround(regler.lies(e) * regler.faktor) == std::lround(erster * regler.faktor);
        });
        const QSignalBlocker ruhe(regler.schieber);
        regler.schieber->setValue(static_cast<int>(std::lround(erster * regler.faktor)));
        regler.anzeige->setText(leer ? QString() : gleich ? regler.text(erster) : tr("mixed"));
    }
    {
        const EchoZeit erste = leer ? ClipEffekte{}.echo.zeit : alle.front().echo.zeit;
        const bool gleich =
            std::all_of(alle.begin(), alle.end(), [erste](const ClipEffekte& e) { return e.echo.zeit == erste; });
        const QSignalBlocker ruhe(m_zeit);
        m_zeit->setCurrentIndex(gleich ? m_zeit->findData(static_cast<int>(erste)) : -1);
        // die Zeit in Millisekunden, im Tempo des Loops
        m_zeitMs->setText(leer || !gleich || m_loop.bpm <= 0.0
                              ? QString()
                              : tr("%1 ms").arg(std::lround(echoSchlaege(erste) * 60000.0 / m_loop.bpm)));
    }
    m_fuellt = false;
}

bool ClipEffektFenster::aendere(const std::function<void(ClipEffekte&)>& was)
{
    if (m_orte.empty()) return false;
    Loop neu = m_loop;
    for (const ClipOrt& ort : m_orte)
    {
        const int platz = findeClip(neu, ort);
        if (platz < 0) continue;
        was(neu.kanaele[static_cast<std::size_t>(ort.kanal)].clips[static_cast<std::size_t>(platz)].effekte);
    }
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

bool ClipEffektFenster::kopiere()
{
    const std::vector<ClipEffekte> alle = effekte();
    if (alle.empty()) return false;
    m_gemerkt = alle.front();
    m_einfuegen->setEnabled(true);
    return true;
}

bool ClipEffektFenster::fuegeEin()
{
    if (!m_gemerkt.has_value()) return false;
    return aendere([this](ClipEffekte& e) { e = *m_gemerkt; });
}

bool ClipEffektFenster::setzeZurueck()
{
    return aendere([](ClipEffekte& e) { e = ClipEffekte{}; });
}

void ClipEffektFenster::setSpitze(double spitze)
{
    const QString text = spitze < 0.0 ? QStringLiteral("–") : QString::number(spitze, 'f', 2);
    if (text == m_spitze->text()) return;
    m_spitze->setText(text);
    // ueber 1 uebersteuert die Summe (L6 a)
    m_spitze->setStyleSheet(spitze > 1.0 ? QStringLiteral("color: #e05050; font-weight: bold;") : QString());
}

}  // namespace lumi::loop
