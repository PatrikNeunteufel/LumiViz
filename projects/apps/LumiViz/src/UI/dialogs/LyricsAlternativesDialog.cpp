/**
 ****************************************************************************************
 * @file   LyricsAlternativesDialog.cpp
 * @brief  Andere Fassungen eines Songtexts suchen, vergleichen und uebernehmen (S87)
 ****************************************************************************************
 */

#include "UI/dialogs/LyricsAlternativesDialog.hpp"

#include "visualizers/MultiEffectVisualizer.hpp"

#include <BasicLogger.h>

#include <QCheckBox>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QSplitter>
#include <QSyntaxHighlighter>
#include <QTableWidget>
#include <QTextBlock>
#include <QVBoxLayout>

#include <algorithm>
#include <cstdlib>
#include <utility>

namespace
{
using lumi::audio::SongtextKandidat;
namespace ly = lumi::multieffect::lyrics;

enum SpaltenNr { kTitel, kInterpret, kAlbum, kDauer, kTakt, kZeilen, kVermerk, kAnzahl };

bool& sucheBeimOeffnen()
{
    static bool an = true;
    return an;
}

QString dauerText(int sekunden)
{
    return QStringLiteral("%1:%2").arg(sekunden / 60).arg(sekunden % 60, 2, 10, QLatin1Char('0'));
}

/// Zeitmarke wie in der Datei: `[mm:ss.xx]`.
QString markenText(std::int64_t ms)
{
    return QStringLiteral("[%1:%2.%3]")
        .arg(ms / 60000, 2, 10, QLatin1Char('0'))
        .arg((ms / 1000) % 60, 2, 10, QLatin1Char('0'))
        .arg((ms % 1000) / 10, 2, 10, QLatin1Char('0'));
}

/// Der Text, wie er in der Datei stuende — eine Zeile je Textzeile, synchron
/// mit Zeitmarke davor (Block-Nummer = Zeilen-Index).
QString lrcText(const ly::Songtext& s)
{
    QString out;
    for (const ly::Zeile& z : s.zeilen)
    {
        if (z.zeitMs >= 0) out += markenText(z.zeitMs) + QLatin1Char(' ');
        out += QString::fromStdString(z.text) + QLatin1Char('\n');
    }
    return out;
}

/// Zeitmarken gedaempft, damit der Text lesbar bleibt und die Zeiten doch
/// zu sehen sind.
class LrcHervorhebung : public QSyntaxHighlighter
{
public:
    using QSyntaxHighlighter::QSyntaxHighlighter;

protected:
    void highlightBlock(const QString& text) override
    {
        static const QRegularExpression zeit(QStringLiteral("^\\[\\d+:\\d+(?:[.:]\\d+)?\\]"));
        static const QRegularExpression wort(QStringLiteral("<\\d+:\\d+(?:[.:]\\d+)?>"));
        static const QRegularExpression stimme(
            QStringLiteral("(?<=\\] |^)(?:v1\\+v2|v1|v2|M|F|D):"),
            QRegularExpression::CaseInsensitiveOption);
        QTextCharFormat fZeit;
        fZeit.setForeground(QColor(0x5A, 0x8F, 0xD6));
        QTextCharFormat fStimme;
        fStimme.setForeground(QColor(0xD6, 0x8A, 0x3C));
        fStimme.setFontWeight(QFont::Bold);
        const auto faerbe = [&](const QRegularExpression& re, const QTextCharFormat& f) {
            auto it = re.globalMatch(text);
            while (it.hasNext())
            {
                const auto m = it.next();
                setFormat(static_cast<int>(m.capturedStart()),
                          static_cast<int>(m.capturedLength()), f);
            }
        };
        faerbe(zeit, fZeit);
        faerbe(wort, fZeit);
        faerbe(stimme, fStimme);
    }
};
}  // namespace

void LyricsAlternativesDialog::setzeSucheBeimOeffnen(bool an)
{
    sucheBeimOeffnen() = an;
}

LyricsAlternativesDialog::LyricsAlternativesDialog(const TrackInfo& track, qint64 aktuelleId,
                                                   QWidget* parent)
    : QDialog(parent), m_track(track), m_aktuelleId(aktuelleId)
{
    setWindowTitle(tr("Lyrics Alternatives — %1").arg(track.title));
    resize(980, 680);
    auto* vl = new QVBoxLayout(this);

    // --- Kopf: fuer welchen Titel, wie lang er ist, wo er gerade spielt
    m_kopf = new QLabel(this);
    m_kopf->setObjectName(QStringLiteral("lyricsAltHeader"));
    m_kopf->setWordWrap(true);
    QFont kopfSchrift = m_kopf->font();
    kopfSchrift.setBold(true);
    m_kopf->setFont(kopfSchrift);
    vl->addWidget(m_kopf);

    // --- Songwechsel: nie stillschweigend umschalten, sondern anbieten
    m_wechsel = new QWidget(this);
    m_wechsel->setObjectName(QStringLiteral("lyricsAltSwitch"));
    auto* wl = new QHBoxLayout(m_wechsel);
    wl->setContentsMargins(0, 0, 0, 0);
    m_wechselText = new QLabel(m_wechsel);
    m_wechselText->setWordWrap(true);
    auto* wechseln = new QPushButton(tr("Switch to the playing track"), m_wechsel);
    wechseln->setObjectName(QStringLiteral("lyricsAltSwitchButton"));
    wechseln->setToolTip(tr("Searches the lyrics of the track that is playing now. Until then "
                            "this window keeps working for the track it was opened for."));
    wl->addWidget(m_wechselText, 1);
    wl->addWidget(wechseln);
    m_wechsel->setVisible(false);
    vl->addWidget(m_wechsel);
    connect(wechseln, &QPushButton::clicked, this, &LyricsAlternativesDialog::nimmLaufendenTitel);

    // --- Suchbegriffe: aus den Tags vorbelegt, frei aenderbar
    auto* form = new QFormLayout();
    m_titel = new QLineEdit(track.title, this);
    m_titel->setObjectName(QStringLiteral("lyricsAltTitle"));
    m_interpret = new QLineEdit(track.artist, this);
    m_interpret->setObjectName(QStringLiteral("lyricsAltArtist"));
    m_titel->setToolTip(tr("Search term for the title — change it when the tag carries "
                           "additions such as \"(Feat. …)\" or \"(album version)\"."));
    form->addRow(tr("Title"), m_titel);
    form->addRow(tr("Artist"), m_interpret);
    vl->addLayout(form);

    auto* suchZeile = new QHBoxLayout();
    m_suchen = new QPushButton(tr("Search"), this);
    m_suchen->setObjectName(QStringLiteral("lyricsAltSearch"));
    m_suchen->setToolTip(tr("Sends the two search terms to lrclib.net."));
    m_status = new QLabel(this);
    m_status->setObjectName(QStringLiteral("lyricsAltStatus"));
    m_status->setWordWrap(true);
    m_folgen = new QCheckBox(tr("Follow playback"), this);
    m_folgen->setObjectName(QStringLiteral("lyricsAltFollow"));
    m_folgen->setChecked(true);
    m_folgen->setToolTip(tr("Keeps the line at the playback position in view. The bar marks "
                            "it in every version shown."));
    suchZeile->addWidget(m_suchen);
    suchZeile->addWidget(m_status, 1);
    suchZeile->addWidget(m_folgen);
    vl->addLayout(suchZeile);

    // --- Kandidaten oben, Texte unten; der Griff dazwischen teilt die Hoehe
    auto* teiler = new QSplitter(Qt::Vertical, this);
    teiler->setObjectName(QStringLiteral("lyricsAltSplitter"));
    teiler->setChildrenCollapsible(false);
    m_tabelle = new QTableWidget(0, kAnzahl, teiler);
    m_tabelle->setObjectName(QStringLiteral("lyricsAltTable"));
    m_tabelle->setHorizontalHeaderLabels({tr("Title"), tr("Artist"), tr("Album"), tr("Duration"),
                                          tr("Timing"), tr("Lines"), tr("Note")});
    m_tabelle->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tabelle->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_tabelle->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tabelle->verticalHeader()->setVisible(false);
    m_tabelle->horizontalHeader()->setStretchLastSection(true);
    m_tabelle->setToolTip(tr("Select up to three entries (Ctrl or Shift) to compare them side "
                             "by side. The version shown now is bold."));

    auto* texte = new QSplitter(Qt::Horizontal, teiler);
    texte->setChildrenCollapsible(false);
    for (int i = 0; i < kSpalten; ++i)
    {
        Spalte& sp = m_spalten[static_cast<std::size_t>(i)];
        sp.rahmen = new QWidget(texte);
        auto* sl = new QVBoxLayout(sp.rahmen);
        sl->setContentsMargins(0, 0, 0, 0);
        auto* kopfZeile = new QHBoxLayout();
        sp.kopf = new QLabel(sp.rahmen);
        sp.kopf->setObjectName(QStringLiteral("lyricsAltHead%1").arg(i));
        sp.kopf->setWordWrap(true);
        sp.nehmen = new QPushButton(tr("Use these lyrics"), sp.rahmen);
        // die erste Spalte behaelt die Namen aus der einspaltigen Fassung
        sp.nehmen->setObjectName(i == 0 ? QStringLiteral("lyricsAltUse")
                                        : QStringLiteral("lyricsAltUse%1").arg(i));
        sp.nehmen->setToolTip(tr("Replaces the saved lyrics of this track. The previous "
                                 "version is not blocked."));
        sp.nehmen->setEnabled(false);
        kopfZeile->addWidget(sp.kopf, 1);
        kopfZeile->addWidget(sp.nehmen);
        sl->addLayout(kopfZeile);
        sp.text = new QPlainTextEdit(sp.rahmen);
        sp.text->setObjectName(i == 0 ? QStringLiteral("lyricsAltPreview")
                                      : QStringLiteral("lyricsAltPreview%1").arg(i));
        sp.text->setReadOnly(true);
        sp.text->setLineWrapMode(QPlainTextEdit::NoWrap);
        new LrcHervorhebung(sp.text->document());
        sl->addWidget(sp.text, 1);
        connect(sp.nehmen, &QPushButton::clicked, this, [this, i]() { uebernehmen(i); });
        sp.rahmen->setVisible(i == 0);
    }
    m_spalten[0].text->setPlaceholderText(tr("Select an entry to read its lyrics."));
    teiler->setStretchFactor(0, 2);
    teiler->setStretchFactor(1, 3);
    vl->addWidget(teiler, 1);

    auto* knoepfe = new QDialogButtonBox(QDialogButtonBox::Close, this);
    vl->addWidget(knoepfe);

    connect(m_suchen, &QPushButton::clicked, this, &LyricsAlternativesDialog::suche);
    connect(m_titel, &QLineEdit::returnPressed, this, &LyricsAlternativesDialog::suche);
    connect(m_interpret, &QLineEdit::returnPressed, this, &LyricsAlternativesDialog::suche);
    connect(m_tabelle, &QTableWidget::itemSelectionChanged, this,
            &LyricsAlternativesDialog::auswahlGeaendert);
    connect(knoepfe, &QDialogButtonBox::rejected, this, &QDialog::reject);
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &LyricsAlternativesDialog::sende);
    // Kopfzeile, Balken und Titelwechsel laufen mit dem Player
    m_marke.setInterval(200);
    connect(&m_marke, &QTimer::timeout, this, &LyricsAlternativesDialog::aktualisiere);
    m_marke.start();
    aktualisiere();

    if (sucheBeimOeffnen() && !track.title.trimmed().isEmpty()) suche();
}

void LyricsAlternativesDialog::aktualisiere()
{
    const TrackInfo laufend = lumi::audio::laufenderTitel();
    const bool dieser = laufend.filePath == m_track.filePath;
    const auto name = [](const TrackInfo& t) {
        return t.artist.isEmpty() ? t.title : QStringLiteral("%1 — %2").arg(t.artist, t.title);
    };

    // Kopf: Titel · Laenge · Spielzeit (nur solange dieser Titel laeuft)
    QString kopf = name(m_track);
    const int soll = (m_track.durationMs + 500) / 1000;
    if (soll > 0) kopf += QStringLiteral("  ·  ") + tr("length %1").arg(dauerText(soll));
    if (dieser)
    {
        const int pos = static_cast<int>(
            std::max<std::int64_t>(0, MultiEffectVisualizer::playPositionMs()) / 1000);
        kopf += QStringLiteral("  ·  ") + tr("playing at %1").arg(dauerText(pos));
    }
    if (m_kopf->text() != kopf) m_kopf->setText(kopf);

    // ein anderer Titel laeuft: sagen und den Wechsel anbieten
    const bool fremd = !dieser && !laufend.filePath.isEmpty();
    if (fremd)
    {
        const QString text = tr("Now playing: %1 — this window still shows the lyrics "
                                "search for the track above.")
                                 .arg(name(laufend));
        if (m_wechselText->text() != text) m_wechselText->setText(text);
    }
    if (m_wechsel->isVisibleTo(this) != fremd) m_wechsel->setVisible(fremd);

    zeigePosition(dieser ? MultiEffectVisualizer::playPositionMs() : -1);
}

void LyricsAlternativesDialog::nimmLaufendenTitel()
{
    const TrackInfo laufend = lumi::audio::laufenderTitel();
    if (laufend.filePath.isEmpty() || laufend.filePath == m_track.filePath) return;
    m_track = laufend;
    m_aktuelleId =
        lumi::audio::lrclibKennung(MultiEffectVisualizer::songtextAngaben().herkunft);
    setWindowTitle(tr("Lyrics Alternatives — %1").arg(m_track.title));
    m_titel->setText(m_track.title);
    m_interpret->setText(m_track.artist);
    zeigeKandidaten({});  // die Liste gehoerte zum vorigen Titel
    m_status->clear();
    aktualisiere();
    if (sucheBeimOeffnen() && !m_track.title.trimmed().isEmpty()) suche();
}

QString LyricsAlternativesDialog::ordner() const
{
    return m_ordner.isEmpty() ? lumi::audio::songtextOrdner() : m_ordner;
}

void LyricsAlternativesDialog::suche()
{
    m_timer.stop();
    if (QNetworkReply* alt = std::exchange(m_laufend, nullptr)) alt->abort();
    m_versuch = 0;
    if (lumi::audio::lrclibSuchUrl(m_titel->text(), m_interpret->text()).isEmpty())
    {
        m_status->setText(tr("Enter a title to search for."));
        return;
    }
    sende();
}

void LyricsAlternativesDialog::sende()
{
    const QUrl url = lumi::audio::lrclibSuchUrl(m_titel->text(), m_interpret->text());
    if (url.isEmpty()) return;
    if (m_net == nullptr) m_net = new QNetworkAccessManager(this);
    QNetworkRequest req(url);
    // verlangt: Programmname, Version und Projektseite (lrclib.net/docs)
    req.setRawHeader("User-Agent",
                     QStringLiteral("LumiViz v%1 (https://github.com/PatrikNeunteufel/LumiViz)")
                         .arg(QCoreApplication::applicationVersion())
                         .toUtf8());
    req.setTransferTimeout(15000);
    QNetworkReply* reply = m_net->get(req);
    m_laufend = reply;
    m_status->setText(m_versuch == 0 ? tr("…searching…")
                                     : tr("…searching… (service busy, trying again)"));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { fertig(reply); });
}

void LyricsAlternativesDialog::fertig(QNetworkReply* reply)
{
    reply->deleteLater();
    if (reply != m_laufend) return;  // ueberholt von einer neueren Suche
    m_laufend = nullptr;
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status == 429 || status == 503)  // wie der selbsttaetige Abruf: von selbst wieder
    {
        const int retry =
            reply->hasRawHeader("Retry-After") ? reply->rawHeader("Retry-After").toInt() : -1;
        const int warteMs = lumi::audio::songtextWartezeitMs(++m_versuch, retry);
        if (warteMs >= 0)
        {
            m_status->setText(tr("…searching… (service busy, trying again)"));
            m_timer.start(warteMs);
            return;
        }
        m_status->setText(tr("The service stays busy — please try again later."));
        return;
    }
    if (status != 200)
    {
        m_status->setText(tr("The search failed (HTTP %1).").arg(status));
        return;
    }
    auto kandidaten = lumi::audio::kandidatenAusLrclib(reply->readAll());
    // Nichts zum vollen Titel (S87, Befund Patrik: „Try (Video Version)"):
    // einmal von selbst ohne die Zusaetze in Klammern und ohne Gaeste suchen.
    // Die Felder zeigen, womit gesucht wurde, und bleiben aenderbar.
    if (kandidaten.empty())
    {
        const QString titel = lumi::audio::songtextTitelOhneZusatz(m_titel->text());
        const QString interpret = lumi::audio::songtextInterpretOhneZusatz(m_interpret->text());
        if (titel != m_titel->text().simplified() || interpret != m_interpret->text().simplified())
        {
            m_titel->setText(titel);
            m_interpret->setText(interpret);
            suche();
            m_status->setText(tr("Nothing for the full title — searching without additions…"));
            return;
        }
    }
    zeigeKandidaten(std::move(kandidaten));
}

bool LyricsAlternativesDialog::istAktuell(const SongtextKandidat& k) const
{
    if (m_aktuelleId > 0) return k.id == m_aktuelleId;
    // ohne Kennung (aeltere Datei ohne Vermerk): am Wortlaut erkennen
    const ly::Songtext jetzt = MultiEffectVisualizer::songtext();
    return !jetzt.leer() && jetzt.zeilen == k.text.zeilen;
}

void LyricsAlternativesDialog::zeigeKandidaten(std::vector<SongtextKandidat> kandidaten)
{
    // die Fassung, deren Dauer der Datei am naechsten kommt, zuerst — bei zwei
    // Versionen eines Songs ist das meist die richtige
    const int soll = (m_track.durationMs + 500) / 1000;
    std::stable_sort(kandidaten.begin(), kandidaten.end(),
                     [soll](const SongtextKandidat& a, const SongtextKandidat& b) {
                         return std::abs(a.dauerSek - soll) < std::abs(b.dauerSek - soll);
                     });
    m_kandidaten = std::move(kandidaten);

    m_tabelle->setRowCount(0);
    for (const SongtextKandidat& k : m_kandidaten)
    {
        const int reihe = m_tabelle->rowCount();
        m_tabelle->insertRow(reihe);
        const int diff = k.dauerSek - soll;
        QString dauer = dauerText(k.dauerSek);
        if (soll > 0 && diff != 0)
            dauer += QStringLiteral(" (%1%2 s)").arg(diff > 0 ? QStringLiteral("+") : QString()).arg(diff);
        const QString takt = k.text.leer() ? (k.instrumental ? tr("instrumental") : tr("no text"))
                             : k.text.synchron ? tr("synced")
                                               : tr("not synced");
        const QStringList zellen = {k.titel,
                                    k.interpret,
                                    k.album,
                                    dauer,
                                    takt,
                                    k.text.leer() ? QString()
                                                  : QString::number(k.text.zeilen.size()),
                                    QString()};
        for (int s = 0; s < zellen.size(); ++s)
        {
            auto* zelle = new QTableWidgetItem(zellen[s]);
            if (s == kZeilen) zelle->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            m_tabelle->setItem(reihe, s, zelle);
        }
    }
    vermerke();
    m_tabelle->resizeColumnsToContents();
    m_status->setText(m_kandidaten.empty()
                          ? tr("Nothing found — try other search terms.")
                          : tr("Entries found: %1").arg(m_kandidaten.size()));
    if (!m_kandidaten.empty()) m_tabelle->selectRow(0);
    auswahlGeaendert();
}

void LyricsAlternativesDialog::vermerke()
{
    const QString pfad = ordner();
    for (int reihe = 0; reihe < m_tabelle->rowCount(); ++reihe)
    {
        const SongtextKandidat& k = m_kandidaten[static_cast<std::size_t>(reihe)];
        const bool aktuell = istAktuell(k);
        QStringList vermerk;
        if (aktuell) vermerk << tr("shown now");
        if (lumi::audio::songtextGesperrt(k.id, pfad)) vermerk << tr("blocked");
        for (int s = 0; s < kAnzahl; ++s)
        {
            QTableWidgetItem* zelle = m_tabelle->item(reihe, s);
            if (zelle == nullptr) continue;
            if (s == kVermerk) zelle->setText(vermerk.join(QStringLiteral(", ")));
            QFont f = zelle->font();
            f.setBold(aktuell);  // die gezeigte Fassung faellt als ganze Zeile auf
            zelle->setFont(f);
        }
    }
}

void LyricsAlternativesDialog::auswahlGeaendert()
{
    // markierte Reihen, in Tabellen-Reihenfolge; die ersten drei werden gezeigt
    std::vector<int> reihen;
    for (const QModelIndex& i : m_tabelle->selectionModel()->selectedRows())
        if (i.row() >= 0 && i.row() < static_cast<int>(m_kandidaten.size()))
            reihen.push_back(i.row());
    std::sort(reihen.begin(), reihen.end());
    if (static_cast<int>(reihen.size()) > kSpalten)
    {
        m_status->setText(tr("Up to three versions are shown side by side."));
        reihen.resize(kSpalten);
    }

    for (int i = 0; i < kSpalten; ++i)
    {
        Spalte& sp = m_spalten[static_cast<std::size_t>(i)];
        sp.kandidat = i < static_cast<int>(reihen.size()) ? reihen[static_cast<std::size_t>(i)] : -1;
        sp.zeile = -1;
        sp.text->setExtraSelections({});
        sp.rahmen->setVisible(i == 0 || sp.kandidat >= 0);
        if (sp.kandidat < 0)
        {
            sp.kopf->clear();
            sp.text->clear();
            sp.nehmen->setEnabled(false);
            continue;
        }
        const SongtextKandidat& k = m_kandidaten[static_cast<std::size_t>(sp.kandidat)];
        sp.kopf->setText(QStringLiteral("%1 · %2 · %3")
                             .arg(k.album.isEmpty() ? k.titel : k.album, dauerText(k.dauerSek),
                                  tr("%1 lines").arg(k.text.zeilen.size())));
        sp.text->setPlainText(lrcText(k.text));
        sp.nehmen->setEnabled(!k.text.leer());
    }
}

void LyricsAlternativesDialog::zeigePosition(std::int64_t posMs)
{
    for (Spalte& sp : m_spalten)
    {
        int zeile = -1;
        if (posMs >= 0 && sp.kandidat >= 0)
            zeile = ly::zeileBei(m_kandidaten[static_cast<std::size_t>(sp.kandidat)].text, posMs);
        if (zeile == sp.zeile) continue;
        sp.zeile = zeile;
        QList<QTextEdit::ExtraSelection> marken;
        const QTextBlock block = sp.text->document()->findBlockByNumber(zeile);
        if (zeile >= 0 && block.isValid())
        {
            QTextEdit::ExtraSelection marke;
            QColor farbe = sp.text->palette().color(QPalette::Highlight);
            farbe.setAlpha(110);
            marke.format.setBackground(farbe);
            marke.format.setProperty(QTextFormat::FullWidthSelection, true);
            marke.cursor = QTextCursor(block);
            marken.append(marke);
            if (m_folgen->isChecked())
            {
                sp.text->setTextCursor(marke.cursor);
                sp.text->centerCursor();
            }
        }
        sp.text->setExtraSelections(marken);
    }
}

void LyricsAlternativesDialog::uebernehmen(int spalte)
{
    if (spalte < 0 || spalte >= kSpalten) return;
    const int index = m_spalten[static_cast<std::size_t>(spalte)].kandidat;
    if (index < 0 || index >= static_cast<int>(m_kandidaten.size())) return;
    const SongtextKandidat& k = m_kandidaten[static_cast<std::size_t>(index)];
    if (k.text.leer()) return;

    const QString pfad = ordner();
    // wer einen gesperrten Eintrag ausdruecklich waehlt, will ihn wieder haben
    lumi::audio::entsperreSongtext(k.id, pfad);
    const auto text = lumi::audio::uebernehmeSongtext(m_track, k.text, pfad);
    BasicLogger::logInfo("Lyrics: alternative chosen (LRCLIB id " + std::to_string(k.id) +
                         ") — " + m_track.title.toStdString());
    // nur anzeigen, wenn der Titel noch laeuft; gespeichert ist der Text so oder so
    if (lumi::audio::laufenderTitel().filePath == m_track.filePath)
    {
        MultiEffectVisualizer::setSongtext(text);
        MultiEffectVisualizer::setSongtextAbruf(lumi::audio::SongtextAbruf::Gefunden);
    }
    // offen bleiben: weiter vergleichen, waehrend der Titel laeuft
    m_aktuelleId = k.id;
    vermerke();
    m_status->setText(tr("Saved: %1").arg(QFileInfo(QString::fromStdString(text.datei)).fileName()));
}

void zeigeLyricsSperrliste(QWidget* parent, const QString& benutzerOrdner)
{
    const QString ordner =
        benutzerOrdner.isEmpty() ? lumi::audio::songtextOrdner() : benutzerOrdner;
    QDialog dlg(parent);
    dlg.setWindowTitle(QObject::tr("Blocked Lyrics"));
    dlg.resize(460, 360);
    auto* vl = new QVBoxLayout(&dlg);
    auto* hinweis = new QLabel(
        QObject::tr("Entries marked as wrong. They are not fetched again; remove one to "
                    "allow it again."),
        &dlg);
    hinweis->setWordWrap(true);
    vl->addWidget(hinweis);
    auto* liste = new QListWidget(&dlg);
    liste->setObjectName(QStringLiteral("lyricsBlockedList"));
    vl->addWidget(liste, 1);
    const auto fuelle = [liste, ordner]() {
        liste->clear();
        for (const lumi::audio::SongtextSperre& s : lumi::audio::songtextSperrliste(ordner))
        {
            auto* item = new QListWidgetItem(
                QStringLiteral("%1  (LRCLIB id %2)").arg(s.titel).arg(s.id), liste);
            item->setData(Qt::UserRole, s.id);
        }
    };
    fuelle();
    auto* knoepfe = new QDialogButtonBox(QDialogButtonBox::Close, &dlg);
    auto* frei = knoepfe->addButton(QObject::tr("Unblock"), QDialogButtonBox::ActionRole);
    frei->setObjectName(QStringLiteral("lyricsBlockedUnblock"));
    vl->addWidget(knoepfe);
    QObject::connect(frei, &QPushButton::clicked, &dlg, [liste, ordner, fuelle]() {
        if (const QListWidgetItem* item = liste->currentItem())
        {
            lumi::audio::entsperreSongtext(item->data(Qt::UserRole).toLongLong(), ordner);
            fuelle();
        }
    });
    QObject::connect(knoepfe, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    dlg.exec();
}
