/**
 ****************************************************************************************
 * @file   LyricsSyncPanel.cpp
 * @brief  Songtext nachsynchronisieren: Zeiten je Zeile mitklopfen und nachstellen
 ****************************************************************************************
 */

#include "UI/panels/LyricsSyncPanel.hpp"

#include "audio/IAudioEngine.hpp"
#include "audio/IAudioPlayer.hpp"
#include "audio/SongtextOnline.hpp"
#include "audio/SongtextQuelle.hpp"
#include "services/IEventBus.hpp"
#include "services/ServiceContainer.hpp"
#include "visualizers/MultiEffectVisualizer.hpp"

#include <BasicLogger.h>

#include <QApplication>
#include <QCheckBox>
#include <QFileInfo>
#include <QMouseEvent>
#include <QPainter>
#include <QPointer>
#include <QToolButton>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>
#include <functional>
#include <vector>

namespace
{
namespace ly = lumi::multieffect::lyrics;

enum Spalte { kZeit, kSprung, kText };

constexpr int kMsProWert = 10;      ///< Aufloesung der Huellkurve
constexpr int kFensterMs = 4000;    ///< so viel zeigt die Wellenform vor und nach der Marke

QString zeitText(std::int64_t ms)
{
    return ms < 0 ? QString() : QString::fromStdString(ly::detail::zeitText(ms));
}
}  // namespace

/**
 * Ausschnitt der Wellenform um die Abspielposition (S87, Wunsch Patrik): die
 * Marke steht in der Mitte, das Bild laeuft darunter durch. Senkrechte Striche
 * sind die Zeiten der Zeilen (die markierte hervorgehoben) — so sieht man, ob
 * ein Strich auf dem Einsatz sitzt. Klick = dorthin springen.
 */
class LyricsWellenform : public QWidget
{
public:
    explicit LyricsWellenform(QWidget* parent) : QWidget(parent)
    {
        setMinimumHeight(64);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setToolTip(tr("Waveform around the playback position (marker in the middle). The "
                      "lines are the times of the lyrics lines. Click to jump there."));
    }

    std::function<void(std::int64_t)> onSprung;

    void setHuelle(std::vector<float> huelle)
    {
        m_huelle = std::move(huelle);
        update();
    }
    void setPosition(std::int64_t posMs)
    {
        if (posMs == m_pos) return;
        m_pos = posMs;
        update();
    }
    void setMarken(std::vector<std::int64_t> marken, std::int64_t gewaehlt)
    {
        m_marken = std::move(marken);
        m_gewaehlt = gewaehlt;
        update();
    }
    [[nodiscard]] bool hatHuelle() const { return !m_huelle.empty(); }

    /// Zeit unter der x-Koordinate (Mitte = Abspielposition).
    [[nodiscard]] std::int64_t zeitBei(int x) const
    {
        const double proPixel = 2.0 * kFensterMs / std::max(1, width());
        return m_pos + static_cast<std::int64_t>((x - width() / 2.0) * proPixel);
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.fillRect(rect(), palette().color(QPalette::Base));
        const int w = width();
        const int h = height();
        const double mitteY = h / 2.0;
        const double msProPixel = 2.0 * kFensterMs / std::max(1, w);
        // Textfarbe, zurueckgenommen: `Mid` ist im dunklen Thema kaum vom
        // Grund zu unterscheiden (am Schnappschuss gesehen)
        QColor welle = palette().color(QPalette::Text);
        welle.setAlpha(150);

        // Huellkurve: je Pixelspalte der groesste Wert ihres Zeitabschnitts
        if (!m_huelle.empty())
        {
            p.setPen(welle);
            for (int x = 0; x < w; ++x)
            {
                const double t0 = m_pos + (x - w / 2.0) * msProPixel;
                const auto i0 = static_cast<std::int64_t>(t0 / kMsProWert);
                const auto i1 = static_cast<std::int64_t>((t0 + msProPixel) / kMsProWert);
                float spitze = 0.0f;
                for (std::int64_t i = std::max<std::int64_t>(0, i0);
                     i <= i1 && i < static_cast<std::int64_t>(m_huelle.size()); ++i)
                    spitze = std::max(spitze, m_huelle[static_cast<std::size_t>(i)]);
                if (i0 < 0 && i1 < 0) continue;  // vor dem Anfang des Titels
                const double hoehe = std::max(0.5, static_cast<double>(spitze) * (h / 2.0 - 2.0));
                p.drawLine(QPointF(x, mitteY - hoehe), QPointF(x, mitteY + hoehe));
            }
        }
        else
        {
            p.setPen(welle);
            p.drawText(rect(), Qt::AlignCenter, tr("No waveform"));
        }

        // Zeiten der Zeilen
        for (const std::int64_t t : m_marken)
        {
            const double x = w / 2.0 + static_cast<double>(t - m_pos) / msProPixel;
            if (x < 0.0 || x > w) continue;
            const bool gewaehlt = t == m_gewaehlt;
            p.setPen(QPen(palette().color(gewaehlt ? QPalette::Highlight : QPalette::Text),
                          gewaehlt ? 2.0 : 1.0));
            p.drawLine(QPointF(x, 0.0), QPointF(x, h));
        }
        // Abspielposition
        p.setPen(QPen(QColor(0xE0, 0x40, 0x40), 2.0));
        p.drawLine(QPointF(w / 2.0, 0.0), QPointF(w / 2.0, h));
    }

    void mousePressEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton && onSprung)
            onSprung(std::max<std::int64_t>(0, zeitBei(static_cast<int>(event->position().x()))));
    }

private:
    std::vector<float> m_huelle;
    std::vector<std::int64_t> m_marken;
    std::int64_t m_pos = 0;
    std::int64_t m_gewaehlt = -1;
};

LyricsSyncPanel::LyricsSyncPanel(ServiceContainer& services, QWidget* parent)
    : PanelBase(services, QStringLiteral("lyrics_sync"), tr("Lyrics Sync"), parent)
{
    setupUI();
    lade(TrackInfo{}, ly::Songtext{});
}

LyricsSyncPanel::~LyricsSyncPanel()
{
    // der Dekodier-Thread haelt nur den Abbruch-Schalter und die Engine;
    // er wird hier beendet, bevor das Panel (und danach die Dienste) gehen
    if (m_abbruch) m_abbruch->store(true);
    if (m_ladeFaden.joinable()) m_ladeFaden.join();
}

QString LyricsSyncPanel::spielzeitText(std::int64_t posMs, int dauerMs)
{
    const auto teil = [](std::int64_t ms, bool zehntel) {
        ms = std::max<std::int64_t>(0, ms);
        QString s = QStringLiteral("%1:%2").arg(ms / 60000).arg((ms / 1000) % 60, 2, 10,
                                                                QLatin1Char('0'));
        if (zehntel) s += QStringLiteral(".%1").arg((ms % 1000) / 100);
        return s;
    };
    return dauerMs > 0 ? teil(posMs, true) + QStringLiteral(" / ") + teil(dauerMs, false)
                       : teil(posMs, true);
}

void LyricsSyncPanel::ladeWellenform(const QString& pfad)
{
    if (pfad == m_wellePfad) return;
    m_wellePfad = pfad;
    if (m_abbruch) m_abbruch->store(true);
    if (m_ladeFaden.joinable()) m_ladeFaden.join();
    m_welle->setHuelle({});
    auto* engine = services().tryResolve<IAudioEngine>();
    if (pfad.isEmpty() || engine == nullptr) return;

    // Die ganze Datei zu dekodieren dauert spuerbar — in einem eigenen Thread;
    // das Ergebnis kommt ueber die Ereignisschleife zurueck und gilt nur, wenn
    // inzwischen kein anderer Titel verlangt wurde.
    m_abbruch = std::make_shared<std::atomic<bool>>(false);
    const auto abbruch = m_abbruch;
    const QPointer<LyricsSyncPanel> panel(this);
    m_ladeFaden = std::thread([engine, pfad, abbruch, panel]() {
        std::vector<float> huelle = engine->probePeaks(pfad, kMsProWert, abbruch.get());
        if (abbruch->load()) return;
        QMetaObject::invokeMethod(
            qApp,
            [panel, pfad, abbruch, huelle = std::move(huelle)]() mutable {
                if (panel && !abbruch->load() && panel->m_wellePfad == pfad)
                    panel->m_welle->setHuelle(std::move(huelle));
            },
            Qt::QueuedConnection);
    });
}

bool LyricsSyncPanel::springeZu(int reihe, int vorlaufMs)
{
    if (reihe < 0 || reihe >= static_cast<int>(m_text.zeilen.size())) return false;
    const std::int64_t zeit = m_text.zeilen[static_cast<std::size_t>(reihe)].zeitMs;
    if (zeit < 0) return false;
    // Zeiten stehen wie in der Datei: Abspielposition = Zeit − Versatz
    const std::int64_t ziel = std::max<std::int64_t>(0, zeit - versatz() - vorlaufMs);
    if (auto* player = services().tryResolve<IAudioPlayer>()) player->seek(static_cast<int>(ziel));
    return true;
}

LyricsSyncPanel::Stand LyricsSyncPanel::jetzigerStand() const
{
    const QList<int> reihen = markierteReihen();
    return {m_text.zeilen, m_geaendert, reihen.isEmpty() ? -1 : reihen.first()};
}

void LyricsSyncPanel::merke(Stand vorher)
{
    m_zurueck.push_back(std::move(vorher));
    if (m_zurueck.size() > kGeschichteMax) m_zurueck.erase(m_zurueck.begin());
    m_vor.clear();  // ein neuer Schritt: was zurueckgenommen war, ist nicht wiederholbar
}

void LyricsSyncPanel::setzeStand(const Stand& s)
{
    m_text.zeilen = s.zeilen;
    m_geaendert = s.geaendert;
    m_laufend = -1;
    fuelleListe();
    if (s.reihe >= 0 && s.reihe < m_liste->rowCount())
    {
        m_liste->selectRow(s.reihe);
        m_liste->scrollToItem(m_liste->item(s.reihe, kText));
    }
    zeigeStand();
    // im Bild: der zurueckgenommene Stand — ohne jede Zeit wieder der Text,
    // wie er geladen wurde (vorschau() liesse sonst die letzte Zeit stehen)
    if (lumi::audio::laufenderTitel().filePath != m_track.filePath) return;
    const ly::Songtext jetzt = alsSongtext();
    MultiEffectVisualizer::setSongtext(jetzt.leer() ? m_geladen : jetzt);
}

bool LyricsSyncPanel::rueckgaengig()
{
    if (m_zurueck.empty()) return false;
    m_vor.push_back(jetzigerStand());
    const Stand s = std::move(m_zurueck.back());
    m_zurueck.pop_back();
    setzeStand(s);
    return true;
}

bool LyricsSyncPanel::wiederholen()
{
    if (m_vor.empty()) return false;
    m_zurueck.push_back(jetzigerStand());
    const Stand s = std::move(m_vor.back());
    m_vor.pop_back();
    setzeStand(s);
    return true;
}

void LyricsSyncPanel::setzeTitelVersatz(int ms)
{
    if (m_track.filePath.isEmpty()) return;
    m_titelVersatzMs = ms;
    lumi::audio::setzeSongtextTitelVersatz(
        m_track, ms, m_ordner.isEmpty() ? lumi::audio::songtextOrdner() : m_ordner);
    // sofort im Bild — aber nur, wenn dieser Titel auch der laufende ist
    if (lumi::audio::laufenderTitel().filePath == m_track.filePath)
        MultiEffectVisualizer::setSongtextTitelVersatz(ms);
    zeigeTitelVersatz();
    zeigeStand();  // die Striche in der Wellenform wandern mit
}

void LyricsSyncPanel::zeigeTitelVersatz()
{
    m_versatzFeld->setEnabled(!m_track.filePath.isEmpty());
    if (m_versatzFeld->value() == m_titelVersatzMs) return;
    const QSignalBlocker ruhe(m_versatzFeld);
    m_versatzFeld->setValue(static_cast<int>(m_titelVersatzMs));
}

int LyricsSyncPanel::preferredArea() const
{
    return Qt::RightDockWidgetArea;
}

void LyricsSyncPanel::setupUI()
{
    auto* vl = new QVBoxLayout(this);

    m_kopf = new QLabel(this);
    m_kopf->setObjectName(QStringLiteral("lyricsSyncHead"));
    m_kopf->setWordWrap(true);
    vl->addWidget(m_kopf);

    // laufende Spielzeit und Wellenform um die Abspielposition (S87)
    m_zeit = new QLabel(this);
    m_zeit->setObjectName(QStringLiteral("lyricsSyncTime"));
    QFont zeitSchrift = m_zeit->font();
    zeitSchrift.setPointSizeF(zeitSchrift.pointSizeF() * 1.3);
    zeitSchrift.setBold(true);
    m_zeit->setFont(zeitSchrift);
    m_zeit->setToolTip(tr("Playback position / length of the track."));
    m_zeit->setText(QStringLiteral("—"));
    vl->addWidget(m_zeit);
    m_welle = new LyricsWellenform(this);
    m_welle->setObjectName(QStringLiteral("lyricsSyncWave"));
    m_welle->onSprung = [this](std::int64_t ms) {
        if (auto* player = this->services().tryResolve<IAudioPlayer>())
            player->seek(static_cast<int>(ms));
    };
    vl->addWidget(m_welle);

    m_liste = new QTableWidget(0, 3, this);
    m_liste->setObjectName(QStringLiteral("lyricsSyncList"));
    m_liste->setHorizontalHeaderLabels({tr("Time"), tr("Go"), tr("Text")});
    // abwechselnd gefaerbte Zeilen; die Sprung-Knoepfe sind flach, damit die
    // Farbe der Zeile auch in ihrer Zelle steht
    m_liste->setAlternatingRowColors(true);
    m_liste->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_liste->setSelectionMode(QAbstractItemView::ExtendedSelection);
    // Doppelklick gehoert dem Vorhoeren; die Zeit aendert man mit F2
    m_liste->setEditTriggers(QAbstractItemView::EditKeyPressed);
    m_liste->verticalHeader()->setVisible(false);
    m_liste->horizontalHeader()->setStretchLastSection(true);
    m_liste->setToolTip(
        tr("Enter or T: give the selected line the current playback position and move on. "
           "Double-click: jump to just before the line. F2 in the Time column: type a time "
           "(mm:ss.xx)."));
    m_liste->installEventFilter(this);
    vl->addWidget(m_liste, 1);

    const auto knopf = [this](const QString& text, const char* name, const QString& tipp) {
        auto* k = new QPushButton(text, this);
        k->setObjectName(QLatin1String(name));
        k->setToolTip(tipp);
        return k;
    };
    auto* reihe1 = new QHBoxLayout();
    m_tap = knopf(tr("Tap"), "lyricsSyncTap",
                  tr("Gives the selected line the current playback position and moves to the "
                     "next line (Enter or T in the list)."));
    auto* minus = knopf(tr("−100 ms"), "lyricsSyncMinus", tr("Selected lines 100 ms earlier."));
    auto* plus = knopf(tr("+100 ms"), "lyricsSyncPlus", tr("Selected lines 100 ms later."));
    auto* abMinus = knopf(tr("From here −"), "lyricsSyncFromMinus",
                          tr("All lines from the first selected one 100 ms earlier."));
    auto* abPlus = knopf(tr("From here +"), "lyricsSyncFromPlus",
                         tr("All lines from the first selected one 100 ms later."));
    reihe1->addWidget(m_tap);
    reihe1->addWidget(minus);
    reihe1->addWidget(plus);
    reihe1->addWidget(abMinus);
    reihe1->addWidget(abPlus);
    reihe1->addStretch(1);
    vl->addLayout(reihe1);

    auto* reihe2 = new QHBoxLayout();
    auto* leeren = knopf(tr("Clear time"), "lyricsSyncClear",
                         tr("Removes the time of the selected lines."));
    auto* neu = knopf(tr("Reload"), "lyricsSyncReload",
                      tr("Takes the lyrics of the playing track again — unsaved times are "
                         "discarded."));
    m_speichern = knopf(tr("Save"), "lyricsSyncSave",
                        tr("Writes a .lrc next to the lyrics file (or to the folder for fetched "
                           "lyrics). Possible once every text line has a time."));
    m_folgen = new QCheckBox(tr("Follow playback"), this);
    m_folgen->setObjectName(QStringLiteral("lyricsSyncFollow"));
    m_folgen->setToolTip(tr("Keeps the line at the playback position in view."));
    reihe2->addWidget(leeren);
    reihe2->addWidget(neu);
    reihe2->addWidget(m_speichern);
    // S88: Rueckgaengig / Wiederholen fuer die Schritte an den Zeiten
    m_undo = knopf(tr("Undo"), "lyricsSyncUndo",
                   tr("Takes back the last step (tap, shift, clear, typed time). Ctrl+Z in "
                      "the list."));
    m_redo = knopf(tr("Redo"), "lyricsSyncRedo",
                   tr("Restores the step taken back last. Ctrl+Y in the list."));
    reihe2->addWidget(m_undo);
    reihe2->addWidget(m_redo);
    connect(m_undo, &QPushButton::clicked, this, &LyricsSyncPanel::rueckgaengig);
    connect(m_redo, &QPushButton::clicked, this, &LyricsSyncPanel::wiederholen);
    reihe2->addStretch(1);
    vl->addLayout(reihe2);
    reihe2 = new QHBoxLayout();  // dritte Reihe: Versatz und Mitlaufen
    // S88 (Entscheid Patrik a): Versatz nur dieses Titels — verschiebt den
    // ganzen Text, ohne die Zeiten oder die Datei anzufassen; wirkt sofort
    m_versatzFeld = new QSpinBox(this);
    m_versatzFeld->setObjectName(QStringLiteral("lyricsSyncTrackOffset"));
    m_versatzFeld->setRange(-60000, 60000);
    m_versatzFeld->setSingleStep(50);
    m_versatzFeld->setSuffix(tr(" ms"));
    m_versatzFeld->setToolTip(
        tr("Timing offset for this track only: positive = the lyrics appear earlier, negative "
           "= later. Shifts the whole text without changing the times or the file; remembered "
           "per track, applied at once (no Save needed)."));
    reihe2->addWidget(new QLabel(tr("Track offset:"), this));
    reihe2->addWidget(m_versatzFeld);
    connect(m_versatzFeld, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &LyricsSyncPanel::setzeTitelVersatz);
    reihe2->addStretch(1);
    reihe2->addWidget(m_folgen);
    vl->addLayout(reihe2);

    m_stand = new QLabel(this);
    m_stand->setObjectName(QStringLiteral("lyricsSyncStatus"));
    m_stand->setWordWrap(true);
    vl->addWidget(m_stand);

    connect(m_tap, &QPushButton::clicked, this,
            [this]() { tippe(MultiEffectVisualizer::playPositionMs()); });
    connect(minus, &QPushButton::clicked, this, [this]() { verschiebe(-kSchrittMs, false); });
    connect(plus, &QPushButton::clicked, this, [this]() { verschiebe(kSchrittMs, false); });
    connect(abMinus, &QPushButton::clicked, this, [this]() { verschiebe(-kSchrittMs, true); });
    connect(abPlus, &QPushButton::clicked, this, [this]() { verschiebe(kSchrittMs, true); });
    connect(leeren, &QPushButton::clicked, this, &LyricsSyncPanel::loescheZeit);
    connect(neu, &QPushButton::clicked, this, [this]() { uebernimmLaufenden(true); });
    connect(m_speichern, &QPushButton::clicked, this, &LyricsSyncPanel::speichere);

    // Vorhoeren: kurz vor die Zeile springen
    connect(m_liste, &QTableWidget::cellDoubleClicked, this,
            [this](int reihe, int) { springeZu(reihe, kVorlaufMs); });
    // die markierte Zeile in der Wellenform hervorheben
    connect(m_liste, &QTableWidget::itemSelectionChanged, this, [this]() {
        if (!m_fuellt) zeigeStand();
    });
    // Zeit von Hand (F2): mm:ss.xx; leer = ohne Zeit; Unlesbares wird verworfen
    connect(m_liste, &QTableWidget::itemChanged, this, [this](QTableWidgetItem* item) {
        if (m_fuellt || item == nullptr || item->column() != kZeit) return;
        const int reihe = item->row();
        if (reihe < 0 || reihe >= static_cast<int>(m_text.zeilen.size())) return;
        const QString eingabe = item->text().trimmed();
        const std::int64_t zeit =
            eingabe.isEmpty() ? -1 : ly::detail::zeit(eingabe.toStdString());
        if (!eingabe.isEmpty() && zeit < 0)
        {
            zeigeZeile(reihe);  // unlesbar: alter Wert zurueck
            return;
        }
        if (m_text.zeilen[static_cast<std::size_t>(reihe)].zeitMs == zeit)
        {
            zeigeZeile(reihe);  // nichts geaendert: nur die Schreibweise richten
            return;
        }
        merke(jetzigerStand());
        m_text.zeilen[static_cast<std::size_t>(reihe)].zeitMs = zeit;
        m_geaendert = true;
        zeigeZeile(reihe);
        zeigeStand();
        vorschau();
    });

    m_uhr = new QTimer(this);
    m_uhr->setInterval(50);  // die Wellenform soll fliessen, nicht ruckeln
    connect(m_uhr, &QTimer::timeout, this, &LyricsSyncPanel::zeigePosition);
}

QList<int> LyricsSyncPanel::markierteReihen() const
{
    QList<int> reihen;
    for (const QModelIndex& i : m_liste->selectionModel()->selectedRows()) reihen.append(i.row());
    std::sort(reihen.begin(), reihen.end());
    return reihen;
}

void LyricsSyncPanel::lade(const TrackInfo& track, const ly::Songtext& text)
{
    m_track = track;
    m_text = text;
    m_geladen = text;
    m_zurueck.clear();  // die Geschichte gehoert zur Arbeitskopie
    m_vor.clear();
    // die Zeiten stehen in der Liste so, wie sie in der Datei stuenden —
    // der Versatz der Datei bleibt als `[offset:]` erhalten
    m_geaendert = false;
    m_laufend = -1;
    // S88: der Versatz, der nur fuer diesen Titel gilt (offsets.json)
    m_titelVersatzMs = lumi::audio::songtextTitelVersatzMs(
        track, m_ordner.isEmpty() ? lumi::audio::songtextOrdner() : m_ordner);
    zeigeTitelVersatz();
    fuelleListe();
    if (m_liste->rowCount() > 0) m_liste->selectRow(0);
    zeigeStand();
    ladeWellenform(track.filePath);
}

void LyricsSyncPanel::fuelleListe()
{
    m_fuellt = true;
    m_liste->setRowCount(static_cast<int>(m_text.zeilen.size()));
    for (int r = 0; r < m_liste->rowCount(); ++r)
    {
        m_liste->removeCellWidget(r, kSprung);  // Knoepfe der vorigen Fuellung
        m_liste->setItem(r, kZeit, new QTableWidgetItem());
        auto* sprung = new QTableWidgetItem();
        sprung->setFlags(sprung->flags() & ~Qt::ItemIsEditable);
        m_liste->setItem(r, kSprung, sprung);
        auto* text = new QTableWidgetItem();
        text->setFlags(text->flags() & ~Qt::ItemIsEditable);
        m_liste->setItem(r, kText, text);
        zeigeZeile(r);
    }
    m_liste->resizeColumnToContents(kZeit);
    if (m_liste->columnWidth(kZeit) < 80) m_liste->setColumnWidth(kZeit, 80);
    m_liste->setColumnWidth(kSprung, 34);
    m_fuellt = false;
}

void LyricsSyncPanel::zeigeZeile(int reihe)
{
    if (reihe < 0 || reihe >= static_cast<int>(m_text.zeilen.size())) return;
    const ly::Zeile& z = m_text.zeilen[static_cast<std::size_t>(reihe)];
    const bool war = m_fuellt;
    m_fuellt = true;
    m_liste->item(reihe, kZeit)->setText(zeitText(z.zeitMs));
    m_liste->item(reihe, kText)->setText(QString::fromStdString(z.text));
    for (const int s : {static_cast<int>(kZeit), static_cast<int>(kText)})
    {
        QFont f = m_liste->item(reihe, s)->font();
        f.setBold(reihe == m_laufend);  // die Zeile an der Abspielposition
        m_liste->item(reihe, s)->setFont(f);
    }
    // Sprung-Knopf: nur wo es eine Zeit gibt, zu der man springen kann
    const bool hatKnopf = m_liste->cellWidget(reihe, kSprung) != nullptr;
    if (z.zeitMs >= 0 && !hatKnopf)
    {
        auto* knopf = new QToolButton(m_liste);
        knopf->setObjectName(QStringLiteral("lyricsSyncJump"));
        knopf->setText(QString::fromUtf8("▶"));
        knopf->setAutoRaise(true);  // flach: die Zeilenfarbe scheint durch
        knopf->setToolTip(tr("Jump to the time of this line."));
        connect(knopf, &QToolButton::clicked, this, [this, reihe]() { springeZu(reihe, 0); });
        m_liste->setCellWidget(reihe, kSprung, knopf);
    }
    else if (z.zeitMs < 0 && hatKnopf)
    {
        m_liste->removeCellWidget(reihe, kSprung);
    }
    m_fuellt = war;
}

int LyricsSyncPanel::offeneZeilen() const
{
    return static_cast<int>(std::count_if(m_text.zeilen.begin(), m_text.zeilen.end(),
                                          [](const ly::Zeile& z) {
                                              return z.zeitMs < 0 && !z.text.empty();
                                          }));
}

void LyricsSyncPanel::zeigeStand()
{
    const bool titel = !m_track.filePath.isEmpty();
    const bool fremd = titel && lumi::audio::laufenderTitel().filePath != m_track.filePath;
    m_kopf->setText(!titel ? tr("No track loaded.")
                    : m_track.artist.isEmpty()
                          ? m_track.title
                          : QStringLiteral("%1 — %2").arg(m_track.artist, m_track.title));

    const int offen = offeneZeilen();
    QString stand;
    if (m_text.leer())
        stand = titel ? tr("No lyrics for this track.") : QString();
    else if (fremd)
        stand = tr("Another track is playing now. Save these times, or Reload to take the "
                   "new track.");
    else if (offen > 0)
        stand = tr("Lines without a time: %1").arg(offen);
    else if (m_geaendert)
        stand = tr("All lines have a time — not saved yet.");
    else
        stand = tr("All lines have a time.");
    m_stand->setText(stand);

    // Striche in der Wellenform: Zeiten als Abspielpositionen (Zeit − Versatz)
    std::vector<std::int64_t> marken;
    for (const ly::Zeile& z : m_text.zeilen)
        if (z.zeitMs >= 0) marken.push_back(z.zeitMs - versatz());
    const QList<int> gewaehlt = markierteReihen();
    std::int64_t gewaehlteZeit = -1;
    if (!gewaehlt.isEmpty() && gewaehlt.first() < static_cast<int>(m_text.zeilen.size()) &&
        m_text.zeilen[static_cast<std::size_t>(gewaehlt.first())].zeitMs >= 0)
        gewaehlteZeit =
            m_text.zeilen[static_cast<std::size_t>(gewaehlt.first())].zeitMs - versatz();
    m_welle->setMarken(std::move(marken), gewaehlteZeit);

    m_tap->setEnabled(!m_text.leer() && !fremd);
    m_undo->setEnabled(!m_zurueck.empty());
    m_redo->setEnabled(!m_vor.empty());
    m_speichern->setEnabled(m_geaendert && !m_text.leer() && offen == 0 && !zielPfad().isEmpty());
}

bool LyricsSyncPanel::tippe(std::int64_t posMs)
{
    const QList<int> reihen = markierteReihen();
    if (reihen.isEmpty() || m_text.leer()) return false;
    const int reihe = reihen.first();
    merke(jetzigerStand());
    // wie der Knoten rechnet: wirksame Zeit = Position + Versatz (Datei + Titel)
    m_text.zeilen[static_cast<std::size_t>(reihe)].zeitMs =
        std::max<std::int64_t>(0, posMs + versatz());
    m_geaendert = true;
    zeigeZeile(reihe);

    // weiter zur naechsten TEXTzeile (Leerzeilen brauchen keine Zeit)
    int naechste = reihe + 1;
    while (naechste < static_cast<int>(m_text.zeilen.size()) &&
           m_text.zeilen[static_cast<std::size_t>(naechste)].text.empty())
        ++naechste;
    if (naechste < static_cast<int>(m_text.zeilen.size()))
    {
        m_liste->selectRow(naechste);
        m_liste->scrollToItem(m_liste->item(naechste, kText), QAbstractItemView::PositionAtCenter);
    }
    zeigeStand();
    vorschau();
    return true;
}

void LyricsSyncPanel::verschiebe(int deltaMs, bool abHier)
{
    QList<int> reihen = markierteReihen();
    if (reihen.isEmpty()) return;
    if (abHier)
    {
        const int start = reihen.first();
        reihen.clear();
        for (int r = start; r < static_cast<int>(m_text.zeilen.size()); ++r) reihen.append(r);
    }
    Stand vorher = jetzigerStand();
    bool etwas = false;
    for (const int r : reihen)
    {
        ly::Zeile& z = m_text.zeilen[static_cast<std::size_t>(r)];
        if (z.zeitMs < 0) continue;
        z.zeitMs = std::max<std::int64_t>(0, z.zeitMs + deltaMs);
        zeigeZeile(r);
        etwas = true;
    }
    if (!etwas) return;
    merke(std::move(vorher));
    m_geaendert = true;
    zeigeStand();
    vorschau();
}

void LyricsSyncPanel::loescheZeit()
{
    Stand vorher = jetzigerStand();
    bool etwas = false;
    for (const int r : markierteReihen())
    {
        ly::Zeile& z = m_text.zeilen[static_cast<std::size_t>(r)];
        if (z.zeitMs < 0) continue;
        z.zeitMs = -1;
        z.worte.clear();  // Wort-Zeiten ohne Zeilen-Zeit sind wertlos
        zeigeZeile(r);
        etwas = true;
    }
    if (!etwas) return;
    merke(std::move(vorher));
    m_geaendert = true;
    zeigeStand();
    vorschau();
}

ly::Songtext LyricsSyncPanel::alsSongtext() const
{
    ly::Songtext out = m_text;
    out.zeilen.clear();
    for (const ly::Zeile& z : m_text.zeilen)
        if (z.zeitMs >= 0) out.zeilen.push_back(z);
    std::stable_sort(out.zeilen.begin(), out.zeilen.end(),
                     [](const ly::Zeile& a, const ly::Zeile& b) { return a.zeitMs < b.zeitMs; });
    out.synchron = !out.zeilen.empty();
    return out;
}

void LyricsSyncPanel::vorschau()
{
    // sofort im Bild: was man klopft, sieht man — gespeichert ist es damit nicht
    if (lumi::audio::laufenderTitel().filePath != m_track.filePath) return;
    const ly::Songtext s = alsSongtext();
    if (s.leer()) return;  // noch keine Zeit: der unsynchrone Block bleibt stehen
    MultiEffectVisualizer::setSongtext(s);
}

QString LyricsSyncPanel::zielPfad() const
{
    if (m_track.filePath.isEmpty() || m_text.leer()) return QString();
    // neben die Quelldatei, gleicher Name: die Suche nimmt `.lrc` vor `.txt`
    if (!m_text.datei.empty())
    {
        const QFileInfo quelle(QString::fromStdString(m_text.datei));
        return quelle.absolutePath() + QLatin1Char('/') + quelle.completeBaseName() +
               QStringLiteral(".lrc");
    }
    // eingebettet / ohne Datei: wie ein geholter Text
    return lumi::audio::songtextZiel(m_track, true, lumi::audio::songtextAblage(),
                                     m_ordner.isEmpty() ? lumi::audio::songtextOrdner()
                                                        : m_ordner);
}

bool LyricsSyncPanel::speichere()
{
    if (offeneZeilen() > 0) return false;
    const QString ziel = zielPfad();
    ly::Songtext s = alsSongtext();
    if (ziel.isEmpty() || s.leer()) return false;
    if (m_track.durationMs > 0) s.laengeMs = m_track.durationMs;  // fuer welche Fassung
    if (!lumi::audio::speichereSongtext(s, ziel))
    {
        m_stand->setText(tr("Could not write %1").arg(QFileInfo(ziel).fileName()));
        return false;
    }
    BasicLogger::logInfo("Lyrics: synced lyrics saved — " + ziel.toStdString());
    s.datei = ziel.toStdString();
    s.quelle = "synced in LumiViz: " + QFileInfo(ziel).fileName().toStdString();
    m_text = s;  // die Leerzeilen ohne Zeit sind damit weg, wie in der Datei
    m_geladen = s;
    m_zurueck.clear();  // die Reihen sind jetzt andere: aeltere Staende passen nicht mehr
    m_vor.clear();
    m_geaendert = false;
    fuelleListe();
    if (lumi::audio::laufenderTitel().filePath == m_track.filePath)
        MultiEffectVisualizer::setSongtext(s);
    zeigeStand();
    m_stand->setText(tr("Saved: %1").arg(QFileInfo(ziel).fileName()));
    return true;
}

void LyricsSyncPanel::zeigePosition()
{
    // ein neuer Titel: uebernehmen, solange hier nichts Ungespeichertes liegt
    uebernimmLaufenden(false);
    // Spielzeit und Wellenform laufen immer mit dem Player
    const std::int64_t pos = MultiEffectVisualizer::playPositionMs();
    const TrackInfo laufend = lumi::audio::laufenderTitel();
    const QString zeit =
        laufend.filePath.isEmpty() ? QStringLiteral("—") : spielzeitText(pos, laufend.durationMs);
    if (m_zeit->text() != zeit) m_zeit->setText(zeit);
    m_welle->setPosition(laufend.filePath == m_track.filePath ? pos : 0);
    if (m_text.leer() || laufend.filePath != m_track.filePath) return;

    // die Zeile an der Abspielposition: die spaeteste, deren Zeit erreicht ist
    // S88: der Lyrics-Editor kann den Versatz dieses Titels verstellt haben
    const std::int64_t titelVersatz = MultiEffectVisualizer::songtextTitelVersatzMs();
    if (titelVersatz != m_titelVersatzMs)
    {
        m_titelVersatzMs = titelVersatz;
        zeigeTitelVersatz();
        zeigeStand();
    }
    const std::int64_t jetzt = MultiEffectVisualizer::playPositionMs() + versatz();
    int reihe = -1;
    std::int64_t beste = -1;
    for (int r = 0; r < static_cast<int>(m_text.zeilen.size()); ++r)
    {
        const std::int64_t t = m_text.zeilen[static_cast<std::size_t>(r)].zeitMs;
        if (t >= 0 && t <= jetzt && t >= beste)
        {
            beste = t;
            reihe = r;
        }
    }
    if (reihe == m_laufend) return;
    const int alt = m_laufend;
    m_laufend = reihe;
    zeigeZeile(alt);
    zeigeZeile(reihe);
    if (m_folgen->isChecked() && reihe >= 0)
        m_liste->scrollToItem(m_liste->item(reihe, kText), QAbstractItemView::PositionAtCenter);
}

void LyricsSyncPanel::uebernimmLaufenden(bool erzwungen)
{
    const TrackInfo titel = lumi::audio::laufenderTitel();
    const ly::Songtext text = MultiEffectVisualizer::songtext();
    if (!erzwungen)
    {
        if (m_geaendert)  // Arbeit nie stillschweigend verwerfen
        {
            zeigeStand();
            return;
        }
        // nichts Neues: derselbe Titel und derselbe Text (ein Online-Fund
        // kommt nach dem Titelwechsel — deshalb zaehlt auch der Text)
        if (titel.filePath == m_track.filePath && text.zeilen == m_text.zeilen) return;
    }
    lade(titel, text);
}

void LyricsSyncPanel::onActivate()
{
    uebernimmLaufenden(false);
    m_uhr->start();
}

void LyricsSyncPanel::onDeactivate()
{
    m_uhr->stop();
}

bool LyricsSyncPanel::eventFilter(QObject* watched, QEvent* event)
{
    // Tasten der LISTE — waehrend F2 die Zeit bearbeitet, hat das Eingabefeld
    // den Fokus und die Tasten kommen hier gar nicht an
    // S88: Ctrl+Z / Ctrl+Y (auch Ctrl+Shift+Z) gehoeren in der Liste dem Panel.
    // ShortcutOverride annehmen, sonst nimmt das fensterweite Kuerzel
    // (Edit → Undo, CommandBus) die Taste, bevor sie als KeyPress hier ankommt.
    if (watched == m_liste &&
        (event->type() == QEvent::ShortcutOverride || event->type() == QEvent::KeyPress))
    {
        auto* taste = static_cast<QKeyEvent*>(event);
        const bool ctrl = (taste->modifiers() & Qt::ControlModifier) != 0;
        const bool shift = (taste->modifiers() & Qt::ShiftModifier) != 0;
        const bool zurueck = ctrl && !shift && taste->key() == Qt::Key_Z;
        const bool vor = ctrl && (taste->key() == Qt::Key_Y || (shift && taste->key() == Qt::Key_Z));
        if (zurueck || vor)
        {
            if (event->type() == QEvent::ShortcutOverride)
            {
                event->accept();
                return true;
            }
            if (zurueck)
                rueckgaengig();
            else
                wiederholen();
            return true;
        }
    }
    if (watched == m_liste && event->type() == QEvent::KeyPress)
    {
        const auto* taste = static_cast<QKeyEvent*>(event);
        if (taste->key() == Qt::Key_Return || taste->key() == Qt::Key_Enter ||
            taste->key() == Qt::Key_T)
        {
            if (m_tap->isEnabled()) tippe(MultiEffectVisualizer::playPositionMs());
            return true;
        }
    }
    return PanelBase::eventFilter(watched, event);
}
