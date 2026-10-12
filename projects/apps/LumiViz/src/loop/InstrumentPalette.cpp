/**
 * @file InstrumentPalette.cpp
 * @brief Loop: das Fenster „Instruments" (Loop_Konzept.md §4h, Stufe 14c)
 */

#include "loop/InstrumentPalette.hpp"

#include "loop/Loop.hpp"

#include <QComboBox>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QHideEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QShortcut>
#include <QSignalBlocker>
#include <QToolButton>
#include <QTreeWidget>
#include <QUrl>
#include <QVBoxLayout>

#include <algorithm>

namespace lumi::loop
{

namespace
{

const char* const kFavoriten = "instrumentPalette/favorites";
const char* const kZuletzt = "instrumentPalette/recent";
const char* const kGeometrie = "instrumentPalette/geometry";

constexpr int kRolleNennung = Qt::UserRole + 1;  ///< an einem Blatt: wie ein Muster das Instrument nennt
constexpr int kRolleKopf = Qt::UserRole + 2;     ///< an einem Kopf: sein Name ohne Zahl

const QString kKopfFavoriten = QStringLiteral("Favorites");
const QString kKopfZuletzt = QStringLiteral("Recently used");
const QString kKopfEigene = QStringLiteral("My files");

constexpr int kHoeheBass = 36;    // C2
constexpr int kHoeheSonst = 60;   // C4

}  // namespace

int InstrumentPalette::hoeheFuerArt(const QString& art)
{
    // ein Bass und ein Schlag fuer eine Drum-Spur (K3 a) spielen tief
    const QString schluessel = gueltigeArt(art);
    return schluessel == QLatin1String("bass") || schluessel == QLatin1String("drums") ? kHoeheBass : kHoeheSonst;
}

InstrumentPalette::InstrumentPalette(QWidget* parent) : QWidget(parent, Qt::Window)
{
    setObjectName(QStringLiteral("instrumentPalette"));
    setWindowTitle(tr("Instruments"));

    // Ctrl+Z / Ctrl+Y (Z1 a, S115): das Fenster ist ein eigenes — das Kuerzel von
    // Edit → Undo im Hauptfenster gilt hier nicht, und nach Use ist dieses Fenster
    // das aktive. In der Suche nimmt das Eingabefeld die Tasten selbst.
    auto* zurueck = new QShortcut(QKeySequence::Undo, this);
    zurueck->setObjectName(QStringLiteral("instrumentPaletteUndo"));
    connect(zurueck, &QShortcut::activated, this, &InstrumentPalette::nimmZurueck);
    auto* vor = new QShortcut(QKeySequence::Redo, this);
    vor->setObjectName(QStringLiteral("instrumentPaletteRedo"));
    connect(vor, &QShortcut::activated, this, &InstrumentPalette::stelleWiederHer);

    auto* aussen = new QVBoxLayout(this);

    // --- oben: suchen und nach Genre sichten
    m_suche = new QLineEdit(this);
    m_suche->setObjectName(QStringLiteral("instrumentPaletteSearch"));
    m_suche->setPlaceholderText(tr("Search"));
    m_suche->setClearButtonEnabled(true);
    m_suche->setToolTip(tr("Searches name, type, genres and description"));
    m_suche->installEventFilter(this);
    m_genre = new QComboBox(this);
    m_genre->setObjectName(QStringLiteral("instrumentPaletteGenre"));
    m_genre->setMinimumWidth(130);
    m_genre->setToolTip(tr("Show only instruments that carry this genre"));
    auto* kopf = new QHBoxLayout;
    kopf->addWidget(m_suche, 1);
    kopf->addWidget(new QLabel(tr("Genre"), this));
    kopf->addWidget(m_genre);
    aussen->addLayout(kopf);

    m_zielZeile = new QLabel(this);
    m_zielZeile->setObjectName(QStringLiteral("instrumentPaletteTarget"));
    m_zielZeile->setEnabled(false);  // grau: eine Auskunft
    aussen->addWidget(m_zielZeile);

    // --- der Baum
    m_baum = new QTreeWidget(this);
    m_baum->setObjectName(QStringLiteral("instrumentPaletteTree"));
    m_baum->setHeaderHidden(true);
    m_baum->setColumnCount(1);
    m_baum->setUniformRowHeights(true);
    m_baum->setContextMenuPolicy(Qt::CustomContextMenu);
    aussen->addWidget(m_baum, 1);

    // --- unten: was gewaehlt ist
    m_titel = new QLabel(this);
    m_titel->setObjectName(QStringLiteral("instrumentPaletteTitle"));
    m_titel->setTextFormat(Qt::RichText);
    m_satz = new QLabel(this);
    m_satz->setObjectName(QStringLiteral("instrumentPaletteDescription"));
    m_satz->setWordWrap(true);
    m_satz->setEnabled(false);
    m_satz->setMinimumHeight(m_satz->fontMetrics().lineSpacing() * 2);
    m_satz->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    aussen->addWidget(m_titel);
    aussen->addWidget(m_satz);

    m_hoehe = new QComboBox(this);
    m_hoehe->setObjectName(QStringLiteral("instrumentPalettePreviewNote"));
    m_hoehe->setToolTip(tr("The note a click plays; it follows the type (a bass plays low)"));
    for (int hoehe = 24; hoehe <= 84; hoehe += 12) m_hoehe->addItem(tonName(hoehe), hoehe);  // C1 … C6
    m_play = new QPushButton(tr("Play"), this);
    m_play->setObjectName(QStringLiteral("instrumentPalettePlay"));
    m_favorit = new QToolButton(this);
    m_favorit->setObjectName(QStringLiteral("instrumentPaletteFavorite"));
    m_favorit->setCheckable(true);
    m_favorit->setText(QString::fromUtf8("★"));  // wie in der Effect Palette
    m_favorit->setToolTip(tr("Favorite (shown at the top)"));
    m_use = new QPushButton(tr("Use"), this);
    m_use->setObjectName(QStringLiteral("instrumentPaletteUse"));
    m_use->setDefault(true);
    auto* fuss = new QHBoxLayout;
    fuss->addWidget(new QLabel(tr("Preview note"), this));
    fuss->addWidget(m_hoehe);
    fuss->addWidget(m_play);
    fuss->addWidget(m_favorit);
    fuss->addStretch(1);
    fuss->addWidget(m_use);
    aussen->addLayout(fuss);
    auto* hinweis = new QLabel(tr("Click plays the instrument · double-click uses it in the pattern"), this);
    hinweis->setEnabled(false);
    aussen->addWidget(hinweis);

    {
        QSettings einstellungen;
        m_favoriten = einstellungen.value(QLatin1String(kFavoriten)).toStringList();
        m_zuletzt = einstellungen.value(QLatin1String(kZuletzt)).toStringList();
        const QByteArray geometrie = einstellungen.value(QLatin1String(kGeometrie)).toByteArray();
        if (geometrie.isEmpty() || !restoreGeometry(geometrie)) resize(440, 620);
    }

    connect(m_suche, &QLineEdit::textChanged, this, [this] { filtere(); });
    connect(m_genre, &QComboBox::activated, this, [this] { filtere(); });
    connect(m_baum, &QTreeWidget::currentItemChanged, this, [this](QTreeWidgetItem* neu, QTreeWidgetItem*) {
        // ein anderes Instrument: die Tonhoehe seiner Art
        if (neu != nullptr)
            if (const Instrument* instrument = finde(neu->data(0, kRolleNennung).toString()))
                setzeVorhoerHoehe(hoeheFuerArt(instrument->art));
        zeigeDetail();
    });
    connect(m_baum, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem* eintrag) {
        if (eintrag != nullptr && !eintrag->data(0, kRolleNennung).toString().isEmpty()) (void)spieleAn();
    });
    connect(m_baum, &QTreeWidget::itemActivated, this, [this](QTreeWidgetItem* eintrag) {
        if (eintrag != nullptr && !eintrag->data(0, kRolleNennung).toString().isEmpty()) (void)verwende();
    });
    connect(m_baum, &QTreeWidget::customContextMenuRequested, this, [this](const QPoint& ort) { zeigeMenue(ort); });
    connect(m_play, &QPushButton::clicked, this, [this] { (void)spieleAn(); });
    connect(m_use, &QPushButton::clicked, this, [this] { (void)verwende(); });
    connect(m_favorit, &QToolButton::clicked, this, [this](bool an) { setzeFavorit(an); });

    liesNeu();
    setzeZiel(QString());
}

// --- der Baum ----------------------------------------------------------------

const Instrument* InstrumentPalette::finde(const QString& nennung) const
{
    if (nennung.isEmpty()) return nullptr;
    const auto treffer = std::find_if(m_alle.begin(), m_alle.end(),
                                      [&nennung](const Instrument& i) { return i.name == nennung; });
    return treffer == m_alle.end() ? nullptr : &*treffer;
}

QTreeWidgetItem* InstrumentPalette::blatt(QTreeWidgetItem* kopf, const Instrument& instrument)
{
    auto* eintrag = new QTreeWidgetItem(kopf);
    eintrag->setText(0, instrument.titel);
    eintrag->setData(0, kRolleNennung, instrument.name);
    eintrag->setToolTip(0, instrument.beschreibung);
    return eintrag;
}

void InstrumentPalette::liesNeu()
{
    {
        // Favoriten und Zuletzt, wie sie jetzt gemerkt sind: das Fenster gibt es je Editor
        // einmal (Instrument-Editor, Drum-Editor) — das andere kann sie geaendert haben
        QSettings einstellungen;
        m_favoriten = einstellungen.value(QLatin1String(kFavoriten)).toStringList();
        m_zuletzt = einstellungen.value(QLatin1String(kZuletzt)).toStringList();
    }
    m_alle = instrumente();
    for (Instrument& eigenes : eigeneInstrumente()) m_alle.push_back(std::move(eigenes));
    {
        // die Genres, die es gibt — die Wahl bleibt, wenn es sie noch gibt
        const QString gewaehltesGenre = m_genre->currentData().toString();
        const QSignalBlocker ruhe(m_genre);
        m_genre->clear();
        m_genre->addItem(tr("All"), QString());
        for (const QString& genre : bekannteGenres()) m_genre->addItem(genre, genre);
        m_genre->setCurrentIndex(std::max(0, m_genre->findData(gewaehltesGenre)));
    }
    baueBaum();
}

void InstrumentPalette::baueBaum()
{
    const QString vorher = gewaehlt();
    // welche Koepfe zugeklappt waren, bleiben es
    QStringList zu;
    for (int i = 0; i < m_baum->topLevelItemCount(); ++i)
        if (!m_baum->topLevelItem(i)->isExpanded()) zu << m_baum->topLevelItem(i)->data(0, kRolleKopf).toString();

    const QSignalBlocker ruhe(m_baum);
    m_baum->clear();
    const auto kopf = [this](const QString& name) {
        auto* eintrag = new QTreeWidgetItem(m_baum);
        eintrag->setData(0, kRolleKopf, name);
        eintrag->setFlags(Qt::ItemIsEnabled);
        QFont fett = eintrag->font(0);
        fett.setBold(true);
        eintrag->setFont(0, fett);
        return eintrag;
    };
    const auto liste = [this, &kopf](const QString& name, const QStringList& nennungen) {
        QTreeWidgetItem* eintrag = kopf(name);
        for (const QString& nennung : nennungen)
            if (const Instrument* instrument = finde(nennung)) blatt(eintrag, *instrument);
    };
    liste(kKopfFavoriten, m_favoriten);
    liste(kKopfZuletzt, m_zuletzt);
    for (const InstrumentArt& art : instrumentArten())
    {
        QTreeWidgetItem* eintrag = kopf(QLatin1String(art.titel));
        for (const Instrument& instrument : m_alle)
            if (!istEigenesInstrument(instrument.name) && gueltigeArt(instrument.art) == QLatin1String(art.schluessel))
                blatt(eintrag, instrument);
    }
    QTreeWidgetItem* eigene = kopf(kKopfEigene);
    for (const Instrument& instrument : m_alle)
        if (istEigenesInstrument(instrument.name)) blatt(eigene, instrument);
    if (eigene->childCount() == 0)
    {
        // noch nichts Eigenes: sagen, wie etwas hierher kommt
        auto* leer = new QTreeWidgetItem(eigene);
        leer->setText(0, tr("Save an instrument in the creator to see it here"));
        leer->setFlags(Qt::NoItemFlags);
    }
    for (int i = 0; i < m_baum->topLevelItemCount(); ++i)
        m_baum->topLevelItem(i)->setExpanded(!zu.contains(m_baum->topLevelItem(i)->data(0, kRolleKopf).toString()));
    filtere();
    if (!vorher.isEmpty()) (void)waehle(vorher);
    zeigeDetail();
}

bool InstrumentPalette::passt(const Instrument& instrument) const
{
    const QString genre = m_genre->currentData().toString();
    if (!genre.isEmpty() && !instrument.genres.contains(genre, Qt::CaseInsensitive)) return false;
    const QString heu = QStringList{instrument.titel, artTitel(instrument.art),
                                    instrument.genres.join(QLatin1Char(' ')), instrument.beschreibung}
                            .join(QLatin1Char(' '));
    const QStringList worte = m_suche->text().split(QLatin1Char(' '), Qt::SkipEmptyParts);
    return std::all_of(worte.begin(), worte.end(),
                       [&heu](const QString& wort) { return heu.contains(wort, Qt::CaseInsensitive); });
}

void InstrumentPalette::filtere()
{
    const bool gefiltert = !m_suche->text().trimmed().isEmpty() || !m_genre->currentData().toString().isEmpty();
    for (int i = 0; i < m_baum->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* kopf = m_baum->topLevelItem(i);
        int sichtbar = 0;
        bool platzhalter = false;
        for (int k = 0; k < kopf->childCount(); ++k)
        {
            QTreeWidgetItem* eintrag = kopf->child(k);
            const Instrument* instrument = finde(eintrag->data(0, kRolleNennung).toString());
            if (instrument == nullptr)
            {
                platzhalter = true;
                eintrag->setHidden(gefiltert);
                continue;
            }
            const bool zeigen = passt(*instrument);
            eintrag->setHidden(!zeigen);
            if (zeigen) ++sichtbar;
        }
        // die Favoriten tragen den Stern des Knopfs, wie in der Effect Palette
        const QString kopfName = kopf->data(0, kRolleKopf).toString();
        const QString name = kopfName == kKopfFavoriten ? QString::fromUtf8("★ ") + kopfName : kopfName;
        kopf->setText(0, sichtbar > 0 ? QStringLiteral("%1  (%2)").arg(name).arg(sichtbar) : name);
        // ein Kopf ohne Treffer verschwindet — nur „My files" bleibt ohne Filter stehen und sagt, wie man es fuellt
        kopf->setHidden(sichtbar == 0 && !(platzhalter && !gefiltert));
    }
    // das gewaehlte ist weggefiltert: nichts ist gewaehlt
    if (QTreeWidgetItem* aktuell = m_baum->currentItem(); aktuell != nullptr && aktuell->isHidden())
        m_baum->setCurrentItem(nullptr);
    zeigeDetail();
}

QStringList InstrumentPalette::zeilen() const
{
    QStringList zeilen;
    for (int i = 0; i < m_baum->topLevelItemCount(); ++i)
    {
        const QTreeWidgetItem* kopf = m_baum->topLevelItem(i);
        if (kopf->isHidden()) continue;
        zeilen << QStringLiteral("[%1]").arg(kopf->data(0, kRolleKopf).toString());
        for (int k = 0; k < kopf->childCount(); ++k)
        {
            const QString nennung = kopf->child(k)->data(0, kRolleNennung).toString();
            if (!kopf->child(k)->isHidden() && !nennung.isEmpty()) zeilen << nennung;
        }
    }
    return zeilen;
}

QStringList InstrumentPalette::unter(const QString& kopfName) const
{
    QStringList nennungen;
    for (int i = 0; i < m_baum->topLevelItemCount(); ++i)
    {
        const QTreeWidgetItem* kopf = m_baum->topLevelItem(i);
        if (kopf->isHidden() || kopf->data(0, kRolleKopf).toString() != kopfName) continue;
        for (int k = 0; k < kopf->childCount(); ++k)
        {
            const QString nennung = kopf->child(k)->data(0, kRolleNennung).toString();
            if (!kopf->child(k)->isHidden() && !nennung.isEmpty()) nennungen << nennung;
        }
    }
    return nennungen;
}

// --- was die Bedienelemente ausloesen ----------------------------------------

void InstrumentPalette::setzeZiel(const QString& muster, const QString& spur)
{
    m_ziel = muster;
    m_zielZeile->setText(muster.isEmpty() ? tr("Open an instrument pattern to use an instrument")
                         : spur.isEmpty() ? tr("For pattern: %1").arg(muster)
                                          : tr("For track: %1 (%2)").arg(spur, muster));
    zeigeDetail();
}

void InstrumentPalette::zeigeVorn()
{
    liesNeu();
    show();
    raise();
    activateWindow();
    m_suche->setFocus();
    m_suche->selectAll();
}

void InstrumentPalette::setzeSuche(const QString& text)
{
    m_suche->setText(text);  // filtert ueber textChanged
}

void InstrumentPalette::setzeGenre(const QString& genre)
{
    m_genre->setCurrentIndex(std::max(0, m_genre->findData(genre)));
    filtere();
}

bool InstrumentPalette::waehle(const QString& nennung)
{
    if (nennung.isEmpty()) return false;
    // lieber das Blatt unter seiner Art oder unter „My files" als das unter Favorites
    QTreeWidgetItem* gefunden = nullptr;
    for (int i = m_baum->topLevelItemCount() - 1; i >= 0 && gefunden == nullptr; --i)
    {
        QTreeWidgetItem* kopf = m_baum->topLevelItem(i);
        if (kopf->isHidden()) continue;
        for (int k = 0; k < kopf->childCount(); ++k)
            if (!kopf->child(k)->isHidden() && kopf->child(k)->data(0, kRolleNennung).toString() == nennung)
            {
                gefunden = kopf->child(k);
                break;
            }
    }
    if (gefunden == nullptr) return false;
    gefunden->parent()->setExpanded(true);
    m_baum->setCurrentItem(gefunden);
    m_baum->scrollToItem(gefunden);
    return true;
}

QString InstrumentPalette::gewaehlt() const
{
    const QTreeWidgetItem* aktuell = m_baum->currentItem();
    return aktuell == nullptr ? QString() : aktuell->data(0, kRolleNennung).toString();
}

int InstrumentPalette::vorhoerHoehe() const
{
    return m_hoehe->currentData().toInt();
}

void InstrumentPalette::setzeVorhoerHoehe(int hoehe)
{
    const int platz = m_hoehe->findData(hoehe);
    if (platz >= 0) m_hoehe->setCurrentIndex(platz);
}

bool InstrumentPalette::spieleAn()
{
    const QString nennung = gewaehlt();
    if (nennung.isEmpty()) return false;
    Q_EMIT tonVerlangt(nennung, vorhoerHoehe());
    return true;
}

bool InstrumentPalette::verwende()
{
    const QString nennung = gewaehlt();
    if (nennung.isEmpty() || m_ziel.isEmpty()) return false;
    Q_EMIT verwenden(nennung);
    m_zuletzt.removeAll(nennung);
    m_zuletzt.prepend(nennung);
    while (m_zuletzt.size() > kZuletztMax) m_zuletzt.removeLast();
    merke();
    baueBaum();
    return true;
}

bool InstrumentPalette::istFavorit(const QString& nennung) const
{
    return m_favoriten.contains(nennung);
}

void InstrumentPalette::setzeFavorit(bool an)
{
    const QString nennung = gewaehlt();
    if (nennung.isEmpty() || an == istFavorit(nennung)) return;
    if (an)
        m_favoriten.append(nennung);
    else
        m_favoriten.removeAll(nennung);
    merke();
    baueBaum();
}

bool InstrumentPalette::loescheGewaehltes()
{
    const QString nennung = gewaehlt();
    if (!istEigenesInstrument(nennung)) return false;
    if (!loescheEigenes(nennung.mid(static_cast<int>(qstrlen(kEigenVorsatz))))) return false;
    m_favoriten.removeAll(nennung);
    m_zuletzt.removeAll(nennung);
    merke();
    liesNeu();
    return true;
}

void InstrumentPalette::merke() const
{
    QSettings einstellungen;
    einstellungen.setValue(QLatin1String(kFavoriten), m_favoriten);
    einstellungen.setValue(QLatin1String(kZuletzt), m_zuletzt);
}

void InstrumentPalette::zeigeDetail()
{
    if (m_use == nullptr) return;  // waehrend des Aufbaus
    const Instrument* instrument = finde(gewaehlt());
    if (instrument == nullptr)
    {
        m_titel->setText(tr("<i>Select an instrument</i>"));
        m_satz->clear();
    }
    else
    {
        QStringList teile{artTitel(instrument->art)};
        if (!instrument->genres.isEmpty()) teile << instrument->genres.join(QStringLiteral(", "));
        if (istEigenesInstrument(instrument->name))
            teile << tr("My files (%1.lvinst)").arg(instrument->name.mid(static_cast<int>(qstrlen(kEigenVorsatz))));
        m_titel->setText(QStringLiteral("<b>%1</b> · %2")
                             .arg(instrument->titel.toHtmlEscaped(), teile.join(QStringLiteral(" · ")).toHtmlEscaped()));
        const int schichten = static_cast<int>(instrument->schichten.size());
        const QString zahl = schichten == 1 ? tr("1 layer.") : tr("%1 layers.").arg(schichten);
        m_satz->setText(instrument->beschreibung.isEmpty() ? zahl
                                                          : QStringLiteral("%1 %2").arg(instrument->beschreibung, zahl));
    }
    const bool da = instrument != nullptr;
    m_play->setEnabled(da);
    m_hoehe->setEnabled(da);
    m_favorit->setEnabled(da);
    {
        const QSignalBlocker ruhe(m_favorit);
        m_favorit->setChecked(da && istFavorit(instrument->name));
    }
    m_use->setEnabled(da && !m_ziel.isEmpty());
    m_use->setToolTip(m_ziel.isEmpty() ? tr("Open an instrument pattern first")
                                       : tr("Use this instrument in the pattern %1").arg(m_ziel));
}

void InstrumentPalette::zeigeMenue(QPoint ort)
{
    QTreeWidgetItem* eintrag = m_baum->itemAt(ort);
    if (eintrag == nullptr || eintrag->data(0, kRolleNennung).toString().isEmpty()) return;
    m_baum->setCurrentItem(eintrag);
    const QString nennung = gewaehlt();
    QMenu menue(this);
    QAction* use = menue.addAction(tr("Use"));
    use->setEnabled(!m_ziel.isEmpty());
    connect(use, &QAction::triggered, this, [this] { (void)verwende(); });
    connect(menue.addAction(tr("Play")), &QAction::triggered, this, [this] { (void)spieleAn(); });
    QAction* favorit = menue.addAction(istFavorit(nennung) ? tr("Remove from Favorites") : tr("Add to Favorites"));
    connect(favorit, &QAction::triggered, this, [this, nennung] { setzeFavorit(!istFavorit(nennung)); });
    if (istEigenesInstrument(nennung))
    {
        menue.addSeparator();
        const QString datei = eigeneDatei(nennung.mid(static_cast<int>(qstrlen(kEigenVorsatz))));
        connect(menue.addAction(tr("Show in folder")), &QAction::triggered, this, [datei] {
            QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(datei).absolutePath()));
        });
        connect(menue.addAction(tr("Delete")), &QAction::triggered, this, [this, datei] {
            const Instrument* instrument = finde(gewaehlt());
            if (QMessageBox::question(this, tr("Instruments"),
                                      tr("Delete your instrument %1?\nA pattern that uses it falls silent.\n%2")
                                          .arg(instrument != nullptr ? instrument->titel : QString(),
                                               QDir::toNativeSeparators(datei))) == QMessageBox::Yes)
                (void)loescheGewaehltes();
        });
    }
    menue.exec(m_baum->viewport()->mapToGlobal(ort));
}

void InstrumentPalette::nimmZurueck()
{
    Q_EMIT rueckgaengigVerlangt(false);
}

void InstrumentPalette::stelleWiederHer()
{
    Q_EMIT rueckgaengigVerlangt(true);
}

void InstrumentPalette::hideEvent(QHideEvent* event)
{
    QSettings().setValue(QLatin1String(kGeometrie), saveGeometry());
    QWidget::hideEvent(event);
}

bool InstrumentPalette::eventFilter(QObject* objekt, QEvent* ereignis)
{
    // aus der Suche mit Pfeil hinunter in den Baum; Enter verwendet, was gewaehlt ist
    if (objekt == m_suche && ereignis->type() == QEvent::KeyPress)
    {
        const auto* taste = static_cast<QKeyEvent*>(ereignis);
        if (taste->key() == Qt::Key_Down)
        {
            m_baum->setFocus();
            if (m_baum->currentItem() == nullptr)
                for (const QString& zeile : zeilen())
                    if (!zeile.startsWith(QLatin1Char('[')) && waehle(zeile)) break;
            return true;
        }
        if (taste->key() == Qt::Key_Return || taste->key() == Qt::Key_Enter)
        {
            (void)verwende();
            return true;
        }
    }
    return QWidget::eventFilter(objekt, ereignis);
}

}  // namespace lumi::loop
