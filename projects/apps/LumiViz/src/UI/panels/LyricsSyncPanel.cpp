/**
 ****************************************************************************************
 * @file   LyricsSyncPanel.cpp
 * @brief  Songtext nachsynchronisieren: Zeiten je Zeile mitklopfen und nachstellen
 ****************************************************************************************
 */

#include "UI/panels/LyricsSyncPanel.hpp"

#include "audio/IAudioPlayer.hpp"
#include "audio/SongtextOnline.hpp"
#include "audio/SongtextQuelle.hpp"
#include "services/IEventBus.hpp"
#include "services/ServiceContainer.hpp"
#include "visualizers/MultiEffectVisualizer.hpp"

#include <BasicLogger.h>

#include <QCheckBox>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>

namespace
{
namespace ly = lumi::multieffect::lyrics;

enum Spalte { kZeit, kText };

QString zeitText(std::int64_t ms)
{
    return ms < 0 ? QString() : QString::fromStdString(ly::detail::zeitText(ms));
}
}  // namespace

LyricsSyncPanel::LyricsSyncPanel(ServiceContainer& services, QWidget* parent)
    : PanelBase(services, QStringLiteral("lyrics_sync"), tr("Lyrics Sync"), parent)
{
    setupUI();
    lade(TrackInfo{}, ly::Songtext{});
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

    m_liste = new QTableWidget(0, 2, this);
    m_liste->setObjectName(QStringLiteral("lyricsSyncList"));
    m_liste->setHorizontalHeaderLabels({tr("Time"), tr("Text")});
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
    connect(m_liste, &QTableWidget::cellDoubleClicked, this, [this](int reihe, int) {
        if (reihe < 0 || reihe >= static_cast<int>(m_text.zeilen.size())) return;
        const std::int64_t zeit = m_text.zeilen[static_cast<std::size_t>(reihe)].zeitMs;
        if (zeit < 0) return;
        if (auto* player = this->services().tryResolve<IAudioPlayer>())
            player->seek(static_cast<int>(std::max<std::int64_t>(0, zeit - kVorlaufMs)));
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
        m_text.zeilen[static_cast<std::size_t>(reihe)].zeitMs = zeit;
        m_geaendert = true;
        zeigeZeile(reihe);
        zeigeStand();
        vorschau();
    });

    m_uhr = new QTimer(this);
    m_uhr->setInterval(200);
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
    // die Zeiten stehen in der Liste so, wie sie in der Datei stuenden —
    // der Versatz der Datei bleibt als `[offset:]` erhalten
    m_geaendert = false;
    m_laufend = -1;
    fuelleListe();
    if (m_liste->rowCount() > 0) m_liste->selectRow(0);
    zeigeStand();
}

void LyricsSyncPanel::fuelleListe()
{
    m_fuellt = true;
    m_liste->setRowCount(static_cast<int>(m_text.zeilen.size()));
    for (int r = 0; r < m_liste->rowCount(); ++r)
    {
        m_liste->setItem(r, kZeit, new QTableWidgetItem());
        auto* text = new QTableWidgetItem();
        text->setFlags(text->flags() & ~Qt::ItemIsEditable);
        m_liste->setItem(r, kText, text);
        zeigeZeile(r);
    }
    m_liste->resizeColumnToContents(kZeit);
    if (m_liste->columnWidth(kZeit) < 80) m_liste->setColumnWidth(kZeit, 80);
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
    for (int s = 0; s < 2; ++s)
    {
        QFont f = m_liste->item(reihe, s)->font();
        f.setBold(reihe == m_laufend);  // die Zeile an der Abspielposition
        m_liste->item(reihe, s)->setFont(f);
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

    m_tap->setEnabled(!m_text.leer() && !fremd);
    m_speichern->setEnabled(m_geaendert && !m_text.leer() && offen == 0 && !zielPfad().isEmpty());
}

bool LyricsSyncPanel::tippe(std::int64_t posMs)
{
    const QList<int> reihen = markierteReihen();
    if (reihen.isEmpty() || m_text.leer()) return false;
    const int reihe = reihen.first();
    // wie der Knoten rechnet: wirksame Zeit = Position + Versatz der Datei
    m_text.zeilen[static_cast<std::size_t>(reihe)].zeitMs =
        std::max<std::int64_t>(0, posMs + m_text.versatzMs);
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
    m_geaendert = true;
    zeigeStand();
    vorschau();
}

void LyricsSyncPanel::loescheZeit()
{
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
    if (m_text.leer() || lumi::audio::laufenderTitel().filePath != m_track.filePath) return;

    // die Zeile an der Abspielposition: die spaeteste, deren Zeit erreicht ist
    const std::int64_t jetzt = MultiEffectVisualizer::playPositionMs() + m_text.versatzMs;
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
