/**
 * @file LautheitsDialog.cpp
 * @brief Composer: der Dialog Match loudness (§9g, G5)
 */

#include "composer/LautheitsDialog.hpp"

#include "audio/IAudioEngine.hpp"
#include "composer/Bearbeiten.hpp"
#include "composer/Sitzung.hpp"
#include "loop/Klangerzeuger.hpp"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QSettings>
#include <QSlider>
#include <QTableWidget>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

namespace lumi::composer
{

namespace
{

enum Spalte : int
{
    kSpalteKanal = 0,
    kSpalteClip,
    kSpalteLautheit,
    kSpalteSpitze,
    kSpalteGain,
    kSpalteVorschlag,
    kSpalteSpielen,
    kSpalten
};

/// Ein Wert in dB mit Vorzeichen, wie die Marke im Clip („+6.0 dB").
QString dbText(double db)
{
    const double gerundet = std::round(db * 10.0) / 10.0;
    return (gerundet > 0.0 ? QStringLiteral("+") : QString()) + QString::number(gerundet, 'f', 1) +
           QStringLiteral(" dB");
}

QIcon farbIcon(std::int32_t farbe)
{
    QPixmap bild(12, 12);
    bild.fill(Qt::transparent);
    QPainter p(&bild);
    p.setPen(Qt::NoPen);
    p.setBrush(farbe >= 0 ? QColor::fromRgb(static_cast<QRgb>(farbe)) : QColor(Qt::gray));
    p.drawRoundedRect(QRectF(1, 1, 10, 10), 2, 2);
    return QIcon(bild);
}

}  // namespace

LautheitsDialog::LautheitsDialog(Sitzung& sitzung, IAudioEngine* engine, Bearbeite bearbeite, QWidget* parent)
    : QDialog(parent),
      m_sitzung(&sitzung),
      m_engine(engine),
      m_bearbeite(std::move(bearbeite)),
      m_abgleich(sitzung.komposition()),
      m_prozent(std::make_shared<std::atomic<int>>(0))
{
    setObjectName(QStringLiteral("composerMatchLoudness"));
    setModal(false);
    const QString name = sitzung.datei().isEmpty() ? tr("untitled") : QFileInfo(sitzung.datei()).completeBaseName();
    setWindowTitle(tr("Match loudness – %1").arg(name));
    if (m_engine != nullptr && m_engine->getSampleRate() > 0) m_rate = m_engine->getSampleRate();
    m_gesetzt = sitzung.komposition();

    auto* wurzel = new QVBoxLayout(this);

    // --- oben: Target und Measure again
    auto* oben = new QHBoxLayout();
    oben->addWidget(new QLabel(tr("Target:"), this));
    m_ziel = new QComboBox(this);
    m_ziel->setObjectName(QStringLiteral("matchTarget"));
    m_ziel->setToolTip(tr("What the others are matched to. A clip: it keeps 0 dB, every other clip gets "
                          "the difference of its loudness. Fixed level: every clip is matched to the "
                          "loudness on the right."));
    oben->addWidget(m_ziel, 1);
    m_festLufs = new QDoubleSpinBox(this);
    m_festLufs->setObjectName(QStringLiteral("matchFixedLufs"));
    m_festLufs->setRange(-40.0, -5.0);
    m_festLufs->setDecimals(1);
    m_festLufs->setSingleStep(0.5);
    m_festLufs->setSuffix(tr(" LUFS"));
    m_festLufs->setKeyboardTracking(false);
    oben->addWidget(m_festLufs);
    m_neuMessen = new QPushButton(tr("Measure again"), this);
    m_neuMessen->setObjectName(QStringLiteral("matchMeasureAgain"));
    m_neuMessen->setToolTip(tr("Measure every clip again, also songs LumiViz remembered."));
    oben->addWidget(m_neuMessen);
    wurzel->addLayout(oben);

    // --- die Zeilen
    m_tabelle = new QTableWidget(this);
    m_tabelle->setObjectName(QStringLiteral("matchTable"));
    m_tabelle->setColumnCount(kSpalten);
    m_tabelle->setHorizontalHeaderLabels(
        {tr("Channel"), tr("Clip"), tr("Loudness"), tr("Peak"), tr("Gain"), tr("Suggested"), QString()});
    m_tabelle->verticalHeader()->hide();
    // die markierte Zeile ist fett (eine Auswahl zeichnet sich ueber den Schiebern unsauber)
    m_tabelle->setSelectionMode(QAbstractItemView::NoSelection);
    m_tabelle->setFocusPolicy(Qt::NoFocus);
    m_tabelle->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QHeaderView* kopf = m_tabelle->horizontalHeader();
    for (int s : {kSpalteKanal, kSpalteLautheit, kSpalteSpitze, kSpalteVorschlag})
        kopf->setSectionResizeMode(s, QHeaderView::ResizeToContents);
    kopf->setSectionResizeMode(kSpalteClip, QHeaderView::Stretch);
    kopf->setSectionResizeMode(kSpalteGain, QHeaderView::Fixed);
    kopf->setSectionResizeMode(kSpalteSpielen, QHeaderView::Fixed);
    m_tabelle->setColumnWidth(kSpalteGain, 190);
    m_tabelle->setColumnWidth(kSpalteSpielen, 36);
    m_tabelle->horizontalHeaderItem(kSpalteLautheit)
        ->setToolTip(tr("Loudness of what the clip plays, after EBU R128, without its level curve and gain"));
    m_tabelle->horizontalHeaderItem(kSpalteSpitze)->setToolTip(tr("Highest sample of what the clip plays"));
    wurzel->addWidget(m_tabelle, 1);
    m_warnung = new QLabel(this);
    m_warnung->setObjectName(QStringLiteral("matchWarning"));
    m_warnung->setWordWrap(true);
    m_warnung->setStyleSheet(QStringLiteral("color: #d8b400;"));
    m_warnung->hide();
    wurzel->addWidget(m_warnung);

    // --- unten
    auto* unten = new QHBoxLayout();
    m_vorschlaege = new QPushButton(tr("Use suggestions"), this);
    m_vorschlaege->setObjectName(QStringLiteral("matchUseSuggestions"));
    m_vorschlaege->setToolTip(tr("Set every measured clip to its suggested gain. Nothing is kept before OK."));
    unten->addWidget(m_vorschlaege);
    m_ab = new QPushButton(this);
    m_ab->setObjectName(QStringLiteral("matchListenAB"));
    m_ab->setCheckable(true);
    m_ab->setToolTip(tr("Switch what you hear while the Composer plays: A = the gains from before this "
                        "dialog, B = the gains in the dialog."));
    unten->addWidget(m_ab);
    unten->addStretch(1);
    auto* abbrechen = new QPushButton(tr("Cancel"), this);
    abbrechen->setObjectName(QStringLiteral("matchCancel"));
    unten->addWidget(abbrechen);
    m_ok = new QPushButton(tr("OK"), this);
    m_ok->setObjectName(QStringLiteral("matchOk"));
    m_ok->setDefault(true);
    unten->addWidget(m_ok);
    wurzel->addLayout(unten);

    connect(m_ok, &QPushButton::clicked, this, &LautheitsDialog::accept);
    connect(abbrechen, &QPushButton::clicked, this, &LautheitsDialog::reject);
    connect(m_vorschlaege, &QPushButton::clicked, this, &LautheitsDialog::nimmVorschlaege);
    connect(m_ab, &QPushButton::toggled, this, [this](bool a) { hoereVorher(a); });
    connect(m_neuMessen, &QPushButton::clicked, this, &LautheitsDialog::missNeu);
    connect(m_ziel, QOverload<int>::of(&QComboBox::activated), this, [this](int index) {
        m_zielVonHand = true;
        m_abgleich.setzeZiel(index < static_cast<int>(m_abgleich.zeilen().size()) ? index : -1);
        merkeZielwahl();
        zeigeAlles();
    });
    connect(m_festLufs, &QDoubleSpinBox::valueChanged, this, [this](double lufs) {
        if (m_fuellt) return;
        m_abgleich.setzeFestLufs(lufs);
        merkeZielwahl();
        zeigeAlles();
    });
    connect(&sitzung, &Sitzung::kompositionBearbeitet, this, &LautheitsDialog::wennKompositionBearbeitet);
    connect(&sitzung, &Sitzung::kompositionGeaendert, this, &LautheitsDialog::wennKompositionGewechselt);

    // Fortschritt der laufenden Messung, zehnmal je Sekunde
    auto* uhr = new QTimer(this);
    uhr->setInterval(100);
    connect(uhr, &QTimer::timeout, this, [this] {
        if (!m_laeuft) return;
        for (std::size_t i = 0; i < m_abgleich.zeilen().size(); ++i)
            if (m_abgleich.zeilen()[i].gleicherClip(m_inArbeit) && !m_abgleich.zeilen()[i].messung.has_value())
                zeigeZeile(static_cast<int>(i));
    });
    uhr->start();

    // die letzte Wahl (L3 a): Fixed level mit Wert — sonst der lauteste Clip
    const QSettings einstellungen;
    const bool fest = einstellungen.value(QLatin1String(kEinstellungFest), false).toBool();
    m_abgleich.setzeFestLufs(einstellungen.value(QLatin1String(kEinstellungLufs), kAbgleichFestLufs).toDouble());
    m_abgleich.setzeZiel(fest ? -1 : 0);
    m_zielVonHand = fest;

    baueTabelle();
    resize(760, std::min(560, 170 + 30 * static_cast<int>(m_abgleich.zeilen().size())));

    // messen: alles, was noch keinen Wert hat
    for (const AbgleichZeile& z : m_abgleich.zeilen())
        if (std::none_of(m_warten.begin(), m_warten.end(), [&](const AbgleichZeile& w) { return w.gleicherClip(z); }))
            m_warten.push_back(z);
    QTimer::singleShot(0, this, &LautheitsDialog::misseNaechste);
}

LautheitsDialog::~LautheitsDialog()
{
    halteMessungAn();
    if (!m_fertig) stelleWiederHer();  // geschlossen, ohne OK und ohne Cancel
    (void)m_gemerkt.schreibe();
}

// =============================================================================
// Oberflaeche
// =============================================================================

void LautheitsDialog::baueTabelle()
{
    m_fuellt = true;
    const auto& zeilen = m_abgleich.zeilen();
    m_tabelle->setRowCount(static_cast<int>(zeilen.size()));
    for (int i = 0; i < static_cast<int>(zeilen.size()); ++i)
    {
        const AbgleichZeile& z = zeilen[static_cast<std::size_t>(i)];
        auto* kanal = new QTableWidgetItem(farbIcon(z.farbe), z.kanalName);
        auto* clip = new QTableWidgetItem(z.name() + (z.loop ? tr(" (loop)") : QString()));
        clip->setToolTip(QFileInfo(z.datei).absoluteFilePath());
        m_tabelle->setItem(i, kSpalteKanal, kanal);
        m_tabelle->setItem(i, kSpalteClip, clip);
        m_tabelle->setItem(i, kSpalteLautheit, new QTableWidgetItem);
        m_tabelle->setItem(i, kSpalteSpitze, new QTableWidgetItem);
        m_tabelle->setItem(i, kSpalteVorschlag, new QTableWidgetItem);

        // Gain: Schieber in Zehnteln, daneben der Wert
        auto* feld = new QWidget(m_tabelle);
        auto* reihe = new QHBoxLayout(feld);
        reihe->setContentsMargins(4, 0, 4, 0);
        auto* schieber = new QSlider(Qt::Horizontal, feld);
        schieber->setObjectName(QStringLiteral("matchGainSlider"));
        schieber->setRange(static_cast<int>(kGainMinDb * 10), static_cast<int>(kGainMaxDb * 10));
        schieber->setMinimumWidth(110);
        auto* wert = new QLabel(feld);
        wert->setObjectName(QStringLiteral("matchGainValue"));
        wert->setMinimumWidth(62);
        wert->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        reihe->addWidget(schieber, 1);
        reihe->addWidget(wert);
        connect(schieber, &QSlider::valueChanged, this, [this, i](int zehntel) {
            if (!m_fuellt) setzeGain(i, zehntel / 10.0);
        });
        m_tabelle->setCellWidget(i, kSpalteGain, feld);

        auto* spielen = new QToolButton(m_tabelle);
        spielen->setObjectName(QStringLiteral("matchPlay"));
        spielen->setText(QStringLiteral("▶"));
        spielen->setToolTip(tr("Play from the middle of this clip"));
        connect(spielen, &QToolButton::clicked, this, [this, i] { spieleZeile(i); });
        m_tabelle->setCellWidget(i, kSpalteSpielen, spielen);
    }
    m_fuellt = false;
    zeigeAlles();
}

void LautheitsDialog::fuelleZiel()
{
    m_fuellt = true;
    m_ziel->clear();
    for (const AbgleichZeile& z : m_abgleich.zeilen())
    {
        QString text = z.kanalName + QStringLiteral(" · ") + z.name();
        if (z.messung.has_value() && !z.messung->stille)
            text += QStringLiteral(" (%1 LUFS)").arg(QString::number(z.messung->lufs, 'f', 1));
        m_ziel->addItem(farbIcon(z.farbe), text);
    }
    m_ziel->addItem(tr("Fixed level"));
    const int ziel = m_abgleich.ziel();
    m_ziel->setCurrentIndex(ziel >= 0 ? ziel : m_ziel->count() - 1);
    m_festLufs->setValue(m_abgleich.festLufs());
    m_festLufs->setVisible(ziel < 0);
    m_fuellt = false;
}

void LautheitsDialog::zeigeZeile(int i)
{
    if (i < 0 || i >= m_tabelle->rowCount()) return;
    const AbgleichZeile& z = m_abgleich.zeilen()[static_cast<std::size_t>(i)];
    const bool wartet = std::any_of(m_warten.begin(), m_warten.end(),
                                    [&](const AbgleichZeile& w) { return w.gleicherClip(z); });
    QString laut;
    QString spitze;
    if (z.messung.has_value())
    {
        laut = z.messung->stille ? tr("silent") : QStringLiteral("%1 LUFS").arg(QString::number(z.messung->lufs, 'f', 1));
        spitze = z.messung->stille && z.messung->spitzeDb <= audio::kLufsStille ? QStringLiteral("–") : dbText(z.messung->spitzeDb);
    }
    else if (z.gemessenFehlgeschlagen)
        laut = tr("not readable");
    else if (m_laeuft && z.gleicherClip(m_inArbeit))
        laut = tr("Measuring… %1 %").arg(m_prozent->load());
    else if (wartet)
        laut = tr("Waiting…");
    m_tabelle->item(i, kSpalteLautheit)->setText(laut);
    m_tabelle->item(i, kSpalteSpitze)->setText(spitze);

    // der Vorschlag; gelb mit Warnzeichen, wenn die Spitze ueber 0 dB kaeme (L4 a)
    QTableWidgetItem* vorschlag = m_tabelle->item(i, kSpalteVorschlag);
    const std::optional<double> v = m_abgleich.vorschlag(i);
    const std::optional<double> spitzeDanach = m_abgleich.spitzeMitVorschlag(i);
    const bool warnt = spitzeDanach.has_value() && *spitzeDanach > 0.0;
    QString text = v.has_value() ? dbText(*v) : QString();
    if (v.has_value() && i == m_abgleich.ziel()) text += tr(" (target)");
    if (warnt) text = QStringLiteral("⚠ ") + text;
    vorschlag->setText(text);
    vorschlag->setForeground(warnt ? QBrush(QColor(0xd8, 0xb4, 0x00)) : QBrush());

    // grau: Kanal oder Clip aus — gemessen wird trotzdem
    const QBrush schrift = z.spielt ? QBrush() : QBrush(palette().color(QPalette::Disabled, QPalette::Text));
    for (int s : {kSpalteKanal, kSpalteClip, kSpalteLautheit, kSpalteSpitze})
    {
        QTableWidgetItem* zelle = m_tabelle->item(i, s);
        zelle->setForeground(schrift);
        QFont f = m_tabelle->font();
        f.setBold(i == m_markiert);  // der Clip, neben dessen Gain der Dialog geoeffnet wurde
        zelle->setFont(f);
    }
    m_tabelle->item(i, kSpalteClip)->setToolTip(QFileInfo(z.datei).absoluteFilePath() +
                                                (z.spielt ? QString() : tr("\nThe clip or its channel is off.")));

    if (auto* feld = m_tabelle->cellWidget(i, kSpalteGain))
    {
        auto* schieber = feld->findChild<QSlider*>();
        auto* wert = feld->findChild<QLabel*>();
        const bool war = m_fuellt;
        m_fuellt = true;
        schieber->setValue(static_cast<int>(std::lround(z.gainDb * 10.0)));
        m_fuellt = war;
        wert->setText(dbText(z.gainDb));
        QFont f = wert->font();
        f.setBold(z.gainDb != z.gainVorherDb);  // geaendert im Dialog
        wert->setFont(f);
        wert->setToolTip(z.gainDb != z.gainVorherDb ? tr("Before: %1").arg(dbText(z.gainVorherDb)) : QString());
    }
}

void LautheitsDialog::zeigeAlles()
{
    fuelleZiel();
    for (int i = 0; i < m_tabelle->rowCount(); ++i) zeigeZeile(i);
    const QString warn = warnText();
    m_warnung->setText(warn);
    m_warnung->setVisible(!warn.isEmpty());
    m_ab->setText(m_vorher ? tr("Hearing: A (before)") : tr("Hearing: B (dialog)"));
    bool vorschlagDa = false;
    for (int i = 0; i < m_tabelle->rowCount(); ++i) vorschlagDa = vorschlagDa || m_abgleich.vorschlag(i).has_value();
    m_vorschlaege->setEnabled(vorschlagDa);
}

QString LautheitsDialog::warnText() const
{
    QStringList saetze;
    const bool limiter = m_sitzung == nullptr || m_sitzung->komposition().limiter;
    for (int i = 0; i < static_cast<int>(m_abgleich.zeilen().size()); ++i)
    {
        const std::optional<double> spitze = m_abgleich.spitzeMitVorschlag(i);
        if (!spitze.has_value() || *spitze <= 0.0) continue;
        const QString name = m_abgleich.zeilen()[static_cast<std::size_t>(i)].name();
        saetze << (limiter ? tr("%1: peak would reach %2 – the limiter keeps it below 0 dB.")
                           : tr("%1: peak would reach %2 – with Limiter off it will clip."))
                      .arg(name, dbText(*spitze));
    }
    return saetze.join(QLatin1Char('\n'));
}

// =============================================================================
// Bedienen
// =============================================================================

void LautheitsDialog::setzeZielClip(int kanal, int clip)
{
    const int zeile = m_abgleich.zeileVon(kanal, clip);
    if (zeile < 0) return;
    m_abgleich.setzeZiel(zeile);
    m_zielVonHand = true;
    markiere(kanal, clip);
}

void LautheitsDialog::markiere(int kanal, int clip)
{
    m_markiert = m_abgleich.zeileVon(kanal, clip);
    if (m_markiert >= 0) m_tabelle->scrollToItem(m_tabelle->item(m_markiert, kSpalteClip));
    zeigeAlles();
}

bool LautheitsDialog::misst() const
{
    return m_laeuft || !m_warten.empty();
}

void LautheitsDialog::setzeGain(int zeile, double db)
{
    if (zeile < 0 || zeile >= static_cast<int>(m_abgleich.zeilen().size())) return;
    AbgleichZeile& z = m_abgleich.zeilen()[static_cast<std::size_t>(zeile)];
    const double neu = std::clamp(std::round(db * 10.0) / 10.0, kGainMinDb, kGainMaxDb);
    if (neu == z.gainDb) return;
    z.gainDb = neu;
    zeigeZeile(zeile);
    zeigeVorschau();
}

void LautheitsDialog::nimmVorschlaege()
{
    if (m_abgleich.uebernimmVorschlaege() == 0) return;
    zeigeAlles();
    zeigeVorschau();
}

void LautheitsDialog::hoereVorher(bool vorher)
{
    if (m_ab->isChecked() != vorher)
    {
        const QSignalBlocker still(m_ab);
        m_ab->setChecked(vorher);
    }
    if (m_vorher == vorher) return;
    m_vorher = vorher;
    zeigeAlles();
    zeigeVorschau();
}

void LautheitsDialog::spieleZeile(int zeile)
{
    if (m_sitzung == nullptr || !m_sitzung->geladen()) return;
    if (zeile < 0 || zeile >= static_cast<int>(m_abgleich.zeilen().size())) return;
    const AbgleichZeile& z = m_abgleich.zeilen()[static_cast<std::size_t>(zeile)];
    const Komposition& k = m_sitzung->komposition();
    if (z.kanal >= static_cast<int>(k.kanaele.size())) return;
    const auto& clips = k.kanaele[static_cast<std::size_t>(z.kanal)].clips;
    if (z.clip >= static_cast<int>(clips.size())) return;
    const Clip& clip = clips[static_cast<std::size_t>(z.clip)];
    m_sitzung->springe(clip.startMs + clip.dauerMs() / 2);
    if (!m_sitzung->laeuft()) m_sitzung->spiele();
}

void LautheitsDialog::missNeu()
{
    halteMessungAn();
    m_warten.clear();
    for (AbgleichZeile& z : m_abgleich.zeilen())
    {
        if (!z.loop) m_gemerkt.vergiss(z.datei);
        z.messung.reset();
        z.gemessenFehlgeschlagen = false;
        if (std::none_of(m_warten.begin(), m_warten.end(), [&](const AbgleichZeile& w) { return w.gleicherClip(z); }))
            m_warten.push_back(z);
    }
    zeigeAlles();
    misseNaechste();
}

void LautheitsDialog::merkeZielwahl() const
{
    QSettings einstellungen;
    einstellungen.setValue(QLatin1String(kEinstellungFest), m_abgleich.ziel() < 0);
    einstellungen.setValue(QLatin1String(kEinstellungLufs), m_abgleich.festLufs());
}

// =============================================================================
// Vorhoeren, OK, Cancel
// =============================================================================

void LautheitsDialog::setze(const Komposition& neu)
{
    // die Sitzung meldet „bearbeitet" noch waehrend des Aufrufs — das ist
    // dann der Dialog selbst, keine Aenderung von aussen
    m_setztSelbst = true;
    m_sitzung->setzeKomposition(neu);
    m_setztSelbst = false;
    m_gesetzt = m_sitzung->komposition();
}

void LautheitsDialog::zeigeVorschau()
{
    if (m_fertig || m_sitzung == nullptr || !m_sitzung->geladen()) return;
    setze(m_abgleich.mitGains(m_sitzung->komposition(), !m_vorher));
}

void LautheitsDialog::stelleWiederHer()
{
    if (m_sitzung == nullptr || !m_sitzung->geladen()) return;
    setze(m_abgleich.mitGains(m_sitzung->komposition(), false));
}

void LautheitsDialog::wennKompositionBearbeitet()
{
    if (m_setztSelbst || m_fertig || m_sitzung == nullptr || gleich(m_sitzung->komposition(), m_gesetzt)) return;
    // von aussen (etwa Ctrl+Z): die Liste neu lesen (G5-2 a)
    const std::vector<int> neu = m_abgleich.liesNeu(m_sitzung->komposition());
    for (int i : neu)
    {
        const AbgleichZeile& z = m_abgleich.zeilen()[static_cast<std::size_t>(i)];
        const bool schonDa = (m_laeuft && z.gleicherClip(m_inArbeit)) ||
                             std::any_of(m_warten.begin(), m_warten.end(),
                                         [&](const AbgleichZeile& w) { return w.gleicherClip(z); });
        if (!schonDa) m_warten.push_back(z);
    }
    if (m_markiert >= static_cast<int>(m_abgleich.zeilen().size())) m_markiert = -1;
    m_gesetzt = m_sitzung->komposition();
    baueTabelle();
    zeigeVorschau();
    if (!m_laeuft) misseNaechste();
}

void LautheitsDialog::wennKompositionGewechselt()
{
    // eine andere Komposition (oder keine): diese gibt es nicht mehr
    m_fertig = true;
    halteMessungAn();
    QDialog::reject();
}

void LautheitsDialog::accept()
{
    if (m_fertig) return;
    halteMessungAn();
    merkeZielwahl();
    if (m_sitzung != nullptr && m_sitzung->geladen())
    {
        // erst still zurueck auf vorher — dann EIN Schritt mit allen Gains
        stelleWiederHer();
        m_fertig = true;
        if (m_abgleich.geaendert() && m_bearbeite)
            m_bearbeite(m_abgleich.mitGains(m_sitzung->komposition(), true), tr("Match loudness"));
    }
    m_fertig = true;
    (void)m_gemerkt.schreibe();
    QDialog::accept();
}

void LautheitsDialog::reject()
{
    if (m_fertig)
    {
        QDialog::reject();
        return;
    }
    halteMessungAn();
    merkeZielwahl();
    stelleWiederHer();
    m_fertig = true;
    (void)m_gemerkt.schreibe();
    QDialog::reject();
}

// =============================================================================
// Messen (G5-3 a)
// =============================================================================

void LautheitsDialog::misseNaechste()
{
    if (m_laeuft || m_fertig) return;
    while (!m_warten.empty())
    {
        const AbgleichZeile was = m_warten.front();
        m_warten.erase(m_warten.begin());
        // ein Song, den LumiViz schon kennt
        if (!was.loop)
            if (const std::optional<audio::Lautheit> da = m_gemerkt.hole(was.datei, was.einsatzMs, was.dauerMs, m_rate))
            {
                nimmErgebnis(was, da);
                continue;
            }
        if (m_engine == nullptr)
        {
            nimmErgebnis(was, std::nullopt);
            continue;
        }
        // ein Loop: im Thread der Oberflaeche vorbereiten, im eigenen Thread rechnen
        std::shared_ptr<const loop::Programm> programm;
        if (was.loop)
        {
            programm = bereiteLoopVor(was.datei, *m_engine, m_rate);
            if (programm == nullptr)
            {
                nimmErgebnis(was, std::nullopt);
                continue;
            }
        }
        m_inArbeit = was;
        m_laeuft = true;
        const int nr = ++m_lauf;
        m_prozent->store(0);
        m_abbruch = std::make_shared<std::atomic<bool>>(false);
        IAudioEngine* engine = m_engine;
        const int rate = m_rate;
        auto abbruch = m_abbruch;
        auto prozent = m_prozent;
        m_faden = std::thread([this, nr, was, programm, engine, rate, abbruch, prozent] {
            const MessFortschritt melde = [abbruch, prozent](double anteil) {
                prozent->store(static_cast<int>(anteil * 100.0));
                return !abbruch->load();
            };
            std::optional<audio::Lautheit> ergebnis = was.loop
                                                   ? messeLoop(programm, rate, melde)
                                                   : messeSong(*engine, was.datei, was.einsatzMs, was.dauerMs, rate, melde);
            if (abbruch->load()) return;
            QMetaObject::invokeMethod(
                this,
                [this, nr, was, ergebnis] {
                    // abgebrochen und schon eine neue Messung unterwegs: gilt nicht
                    if (nr != m_lauf || !m_laeuft) return;
                    if (m_faden.joinable()) m_faden.join();
                    m_laeuft = false;
                    nimmErgebnis(was, ergebnis);
                    misseNaechste();
                },
                Qt::QueuedConnection);
        });
        zeigeAlles();
        return;
    }
    (void)m_gemerkt.schreibe();
    zeigeAlles();
    Q_EMIT messungFertig();
}

void LautheitsDialog::nimmErgebnis(const AbgleichZeile& was, std::optional<audio::Lautheit> ergebnis)
{
    if (ergebnis.has_value() && !was.loop) m_gemerkt.merke(was.datei, was.einsatzMs, was.dauerMs, m_rate, *ergebnis);
    for (AbgleichZeile& z : m_abgleich.zeilen())
    {
        if (!z.gleicherClip(was)) continue;
        z.messung = ergebnis;
        z.gemessenFehlgeschlagen = !ergebnis.has_value();
    }
    // solange niemand ein Ziel gewaehlt hat, ist es der lauteste (L3 a)
    if (!m_zielVonHand)
    {
        const int laut = m_abgleich.lauteste();
        if (laut >= 0) m_abgleich.setzeZiel(laut);
    }
    zeigeAlles();
}

void LautheitsDialog::halteMessungAn()
{
    if (m_abbruch != nullptr) m_abbruch->store(true);
    if (m_faden.joinable()) m_faden.join();
    m_laeuft = false;
    ++m_lauf;  // ein schon geschicktes Ergebnis gilt nicht mehr
}

}  // namespace lumi::composer
