/**
 * @file KitFenster.cpp
 * @brief Loop: das Fenster Kits (Stufe 21c)
 */

#include "loop/KitFenster.hpp"

#include "UI/dialogs/PaketDialoge.hpp"
#include "loop/Instrument.hpp"  // holeInstrument, eigenerName, kEigenVorsatz
#include "loop/InstrumentPalette.hpp"
#include "loop/Kit.hpp"  // bibliotheksKlaenge
#include "loop/KlangAuswahl.hpp"
#include "loop/Sample.hpp"
#include "loop/SampleFrage.hpp"

#include <QCloseEvent>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QSettings>
#include <QShortcut>
#include <QSignalBlocker>
#include <QSlider>
#include <QTableWidget>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include <map>
#include <optional>

namespace lumi::loop
{

namespace
{

const char* const kSampleOrdner = "loop/lastSampleDir";  // derselbe Ordner wie im Drum-Editor
constexpr int kListenBreite = 210;
constexpr int kNutzerBeimNamen = 4;  ///< so viele Loops stehen bei „Used by" beim Namen

enum Spalte : int
{
    kSpalteRolle = 0,
    kSpalteKlang,
    kSpalteLaut,
    kSpaltePan,
    kSpalteSpielen,
    kSpalten
};

/// Ein Schloss fuer die mitgelieferten Kits, in der Farbe des Texts gezeichnet.
QIcon schlossIcon(const QColor& farbe)
{
    QPixmap bild(16, 16);
    bild.fill(Qt::transparent);
    QPainter maler(&bild);
    maler.setRenderHint(QPainter::Antialiasing);
    maler.setPen(QPen(farbe, 1.6));
    maler.setBrush(Qt::NoBrush);
    maler.drawArc(QRectF(4.8, 2.0, 6.4, 8.0), 0, 180 * 16);  // der Buegel
    maler.drawLine(QPointF(4.8, 6.0), QPointF(4.8, 8.0));
    maler.drawLine(QPointF(11.2, 6.0), QPointF(11.2, 8.0));
    maler.setPen(Qt::NoPen);
    maler.setBrush(farbe);
    maler.drawRoundedRect(QRectF(3.0, 7.5, 10.0, 7.0), 1.5, 1.5);  // der Koerper
    return QIcon(bild);
}

/// `user:mein_kit` → `mein_kit`; leer = kein eigenes Kit.
QString eigenerKern(const QString& nennung)
{
    return nennung.startsWith(QLatin1String(kEigenVorsatz)) ? nennung.mid(static_cast<int>(qstrlen(kEigenVorsatz)))
                                                            : QString();
}

/// Der Platz einer Rolle in der Reihenfolge der Rollen: Art, dann Nummer.
std::pair<int, int> platzVon(const QString& rolle)
{
    const std::optional<Rolle> gelesen = leseRolle(rolle);
    if (!gelesen) return {1000, 0};
    return {static_cast<int>(rollenArten().indexOf(gelesen->art)), gelesen->nummer};
}

}  // namespace

KitFenster::KitFenster(QWidget* parent) : QWidget(parent, Qt::Tool)
{
    setObjectName(QStringLiteral("loopKits"));
    setWindowTitle(tr("Kits"));
    setupUI();
}

void KitFenster::setupUI()
{
    auto* haupt = new QVBoxLayout(this);

    // --- Kopf: „Kits", rechts Undo und Redo
    auto* kopf = new QHBoxLayout;
    auto* name = new QLabel(tr("Kits"), this);
    QFont fett = name->font();
    fett.setBold(true);
    fett.setPointSizeF(fett.pointSizeF() * 1.2);
    name->setFont(fett);
    kopf->addWidget(name);
    kopf->addStretch(1);
    m_undo = new QToolButton(this);
    m_undo->setObjectName(QStringLiteral("kitsUndo"));
    m_undo->setText(tr("Undo"));
    m_undo->setToolTip(tr("Undo the last change to this kit (Ctrl+Z)"));
    m_redo = new QToolButton(this);
    m_redo->setObjectName(QStringLiteral("kitsRedo"));
    m_redo->setText(tr("Redo"));
    m_redo->setToolTip(tr("Redo (Ctrl+Y)"));
    kopf->addWidget(m_undo);
    kopf->addWidget(m_redo);
    haupt->addLayout(kopf);

    auto* mitte = new QHBoxLayout;
    // --- links die Liste
    auto* links = new QVBoxLayout;
    m_liste = new QTreeWidget(this);
    m_liste->setObjectName(QStringLiteral("kitsList"));
    m_liste->setHeaderHidden(true);
    m_liste->setRootIsDecorated(false);
    m_liste->setFixedWidth(kListenBreite);
    links->addWidget(m_liste, 1);
    auto* listenKnoepfe = new QHBoxLayout;
    m_duplizieren = new QPushButton(tr("Duplicate"), this);
    m_duplizieren->setObjectName(QStringLiteral("kitsDuplicate"));
    m_duplizieren->setToolTip(tr("Makes your own copy of this kit, as it is now"));
    m_umbenennen = new QPushButton(tr("Rename"), this);
    m_umbenennen->setObjectName(QStringLiteral("kitsRename"));
    m_umbenennen->setToolTip(tr("Changes the name shown for your kit. Loops that use it still find it"));
    m_loeschen = new QPushButton(tr("Delete"), this);
    m_loeschen->setObjectName(QStringLiteral("kitsDelete"));
    m_loeschen->setToolTip(tr("Deletes your kit. Loops that used it keep their sounds"));
    listenKnoepfe->addWidget(m_duplizieren);
    listenKnoepfe->addWidget(m_umbenennen);
    listenKnoepfe->addWidget(m_loeschen);
    links->addLayout(listenKnoepfe);
    mitte->addLayout(links);

    // --- rechts das gewaehlte Kit
    auto* rechts = new QVBoxLayout;
    m_titel = new QLabel(this);
    m_titel->setObjectName(QStringLiteral("kitsTitle"));
    QFont titelSchrift = m_titel->font();
    titelSchrift.setBold(true);
    m_titel->setFont(titelSchrift);
    m_hinweis = new QLabel(this);
    m_hinweis->setObjectName(QStringLiteral("kitsReadOnly"));
    m_hinweis->setWordWrap(true);
    {
        QPalette grau = m_hinweis->palette();
        grau.setColor(QPalette::WindowText, grau.color(QPalette::PlaceholderText));
        m_hinweis->setPalette(grau);
    }
    rechts->addWidget(m_titel);
    rechts->addWidget(m_hinweis);
    m_tabelle = new QTableWidget(0, kSpalten, this);
    m_tabelle->setObjectName(QStringLiteral("kitsRoles"));
    m_tabelle->setHorizontalHeaderLabels({tr("Role"), tr("Sound"), tr("Volume"), tr("Pan"), QString()});
    m_tabelle->verticalHeader()->setVisible(false);
    m_tabelle->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tabelle->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tabelle->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // Anmerkung Patrik zu K8 a: die Spalte Sound nimmt den freien Platz
    QHeaderView* spalten = m_tabelle->horizontalHeader();
    spalten->setSectionResizeMode(kSpalteRolle, QHeaderView::Fixed);
    spalten->setSectionResizeMode(kSpalteKlang, QHeaderView::Stretch);
    spalten->setSectionResizeMode(kSpalteLaut, QHeaderView::Fixed);
    spalten->setSectionResizeMode(kSpaltePan, QHeaderView::Fixed);
    spalten->setSectionResizeMode(kSpalteSpielen, QHeaderView::Fixed);
    m_tabelle->setColumnWidth(kSpalteRolle, 76);
    m_tabelle->setColumnWidth(kSpalteLaut, 92);
    m_tabelle->setColumnWidth(kSpaltePan, 92);
    m_tabelle->setColumnWidth(kSpalteSpielen, 32);
    m_tabelle->verticalHeader()->setDefaultSectionSize(28);
    rechts->addWidget(m_tabelle, 1);
    auto* rollenKnoepfe = new QHBoxLayout;
    m_rolleNeu = new QPushButton(tr("Add role"), this);
    m_rolleNeu->setObjectName(QStringLiteral("kitsAddRole"));
    m_rolleNeu->setToolTip(tr("Adds a role to the kit — choose its sound in the row"));
    // ein Knopf mit Menue zeichnet den Pfeil selbst (wie der Knopf Kit im Drum-Editor)
    auto* rollenMenue = new QMenu(m_rolleNeu);
    m_rolleNeu->setMenu(rollenMenue);
    connect(rollenMenue, &QMenu::aboutToShow, this, [this, rollenMenue] { fuelleRollenMenue(rollenMenue); });
    m_rolleWeg = new QPushButton(tr("Remove role"), this);
    m_rolleWeg->setObjectName(QStringLiteral("kitsRemoveRole"));
    m_alle = new QPushButton(tr("Play all"), this);
    m_alle->setObjectName(QStringLiteral("kitsPlayAll"));
    m_alle->setToolTip(tr("Plays every role once, one per beat"));
    rollenKnoepfe->addWidget(m_rolleNeu);
    rollenKnoepfe->addWidget(m_rolleWeg);
    rollenKnoepfe->addStretch(1);
    rollenKnoepfe->addWidget(m_alle);
    rechts->addLayout(rollenKnoepfe);
    mitte->addLayout(rechts, 1);
    haupt->addLayout(mitte, 1);

    // --- Fuss
    auto* fuss = new QHBoxLayout;
    m_nutzer = new QLabel(this);
    m_nutzer->setObjectName(QStringLiteral("kitsUsedBy"));
    m_nutzer->setWordWrap(true);
    m_schliessen = new QPushButton(tr("Close"), this);
    m_sichern = new QPushButton(tr("Save"), this);
    m_sichern->setObjectName(QStringLiteral("kitsSave"));
    m_sichern->setDefault(true);
    m_publish = new QPushButton(tr("Publish..."), this);
    m_publish->setObjectName(QStringLiteral("kitsPublish"));
    m_publish->setToolTip(tr("Pass the kit on as one file, with the instruments and samples of your own it uses"));
    fuss->addWidget(m_nutzer, 1);
    fuss->addWidget(m_schliessen);
    fuss->addWidget(m_sichern);
    fuss->addWidget(m_publish);
    haupt->addLayout(fuss);

    connect(m_undo, &QToolButton::clicked, this, [this] { (void)rueckgaengig(); });
    connect(m_redo, &QToolButton::clicked, this, [this] { (void)wiederherstellen(); });
    auto* zurueck = new QShortcut(QKeySequence::Undo, this);
    connect(zurueck, &QShortcut::activated, this, [this] { (void)rueckgaengig(); });
    auto* vor = new QShortcut(QKeySequence::Redo, this);
    connect(vor, &QShortcut::activated, this, [this] { (void)wiederherstellen(); });
    connect(m_liste, &QTreeWidget::currentItemChanged, this, [this](QTreeWidgetItem* neu) {
        const QString nennung = neu != nullptr ? neu->data(0, Qt::UserRole).toString() : QString();
        if (nennung.isEmpty() || nennung == m_kit.name) return;
        if (darfVerlassen())
            (void)waehle(nennung);
        else
            baueListe();  // die Liste steht wieder auf dem offenen Kit
    });
    connect(m_duplizieren, &QPushButton::clicked, this, [this] { (void)dupliziere(); });
    connect(m_umbenennen, &QPushButton::clicked, this, &KitFenster::frageUndBenenne);
    connect(m_loeschen, &QPushButton::clicked, this, &KitFenster::frageUndLoesche);
    connect(m_rolleWeg, &QPushButton::clicked, this, [this] { (void)entferneRolle(zeile()); });
    connect(m_alle, &QPushButton::clicked, this, [this] { (void)spieleAlle(); });
    connect(m_tabelle, &QTableWidget::itemSelectionChanged, this, &KitFenster::zeigeKnoepfe);
    connect(m_schliessen, &QPushButton::clicked, this, &QWidget::close);
    connect(m_sichern, &QPushButton::clicked, this, [this] {
        if (speichere().isEmpty())
            QMessageBox::warning(this, tr("Kits"), tr("Could not save the kit %1").arg(m_kit.titel));
    });
    // Publish (Publish_Konzept.md P2 a): das gewaehlte Kit, wie es gespeichert ist
    connect(m_publish, &QPushButton::clicked, this, [this] {
        const QString datei = lumi::paket::dateiDerNennung(lumi::paket::Art::Kit, m_kit.name, {});
        if (datei.isEmpty())  // ein mitgeliefertes: die Meldung, kein Speichern
        {
            lumi::ui::veroeffentlicheAusFenster(this, lumi::paket::Art::Kit, datei, false);
            return;
        }
        lumi::ui::veroeffentlicheAusFenster(this, lumi::paket::Art::Kit, datei, geaendert(), [this, datei] {
            return speichere().isEmpty() ? QString() : datei;
        });
    });
    resize(880, 520);
}

// --- die Liste ---------------------------------------------------------------

void KitFenster::liesListeNeu()
{
    baueListe();
}

void KitFenster::baueListe()
{
    const QSignalBlocker ruhe(m_liste);
    m_liste->clear();
    const QIcon schloss = schlossIcon(QWidget::palette().color(QPalette::PlaceholderText));
    QTreeWidgetItem* offen = nullptr;
    const auto abschnitt = [&](const QString& titel, const std::vector<DrumKit>& kits, bool mitgeliefert) {
        if (kits.empty()) return;
        auto* kopf = new QTreeWidgetItem(m_liste, {titel});
        kopf->setFlags(Qt::ItemIsEnabled);
        QFont fett = kopf->font(0);
        fett.setBold(true);
        kopf->setFont(0, fett);
        for (const DrumKit& kit : kits)
        {
            auto* eintrag = new QTreeWidgetItem(m_liste, {kit.titel});
            eintrag->setData(0, Qt::UserRole, kit.name);
            eintrag->setToolTip(0, kit.beschreibung);
            if (mitgeliefert) eintrag->setIcon(0, schloss);
            if (kit.name == m_kit.name) offen = eintrag;
        }
    };
    abschnitt(tr("Built-in kits"), mitgelieferteKits(), true);
    abschnitt(tr("My kits"), eigeneKits(), false);
    if (offen != nullptr)
    {
        m_liste->setCurrentItem(offen);
        m_liste->scrollToItem(offen);
    }
}

QStringList KitFenster::listenZeilen() const
{
    QStringList zeilen;
    for (int i = 0; i < m_liste->topLevelItemCount(); ++i)
    {
        const QTreeWidgetItem* eintrag = m_liste->topLevelItem(i);
        const QString nennung = eintrag->data(0, Qt::UserRole).toString();
        if (nennung.isEmpty())
            zeilen << QStringLiteral("[%1]").arg(eintrag->text(0));
        else
            zeilen << (nennung == m_kit.name ? QStringLiteral("*") : QString()) + nennung;
    }
    return zeilen;
}

void KitFenster::oeffne(const QString& nennung)
{
    if (!nennung.isEmpty() && nennung == m_kit.name)
    {
        baueListe();  // eigene koennen dazugekommen sein
        zeigeAlles();
        return;
    }
    if (!darfVerlassen())
    {
        baueListe();
        return;
    }
    if (waehle(nennung)) return;
    // das Kit gibt es nicht (oder keins genannt): das erste der Liste
    if (!mitgelieferteKits().empty())
        (void)waehle(mitgelieferteKits().front().name);
    else if (const std::vector<DrumKit> eigene = eigeneKits(); !eigene.empty())
        (void)waehle(eigene.front().name);
    else
    {
        baueListe();
        zeigeAlles();
    }
}

bool KitFenster::waehle(const QString& nennung)
{
    DrumKit kit;
    if (nennung.isEmpty() || !holeKit(nennung, kit)) return false;
    m_kit = kit;
    m_vorlage = kit;
    m_zurueck.clear();
    m_vor.clear();
    m_tabelle->setRowCount(0);  // ein anderes Kit: die Zeilen von vorn
    m_zeilen.clear();
    rechneNutzer();
    baueListe();
    zeigeAlles();
    if (!m_kit.klaenge.empty()) waehleZeile(0);
    return true;
}

void KitFenster::setKitKlaenge(const QStringList& klaenge)
{
    m_kitKlaenge = klaenge;
    zeigeAlles();
}

void KitFenster::setOffenerLoop(const Loop& loop)
{
    m_offen = loop;
    m_hatOffen = true;
    rechneNutzer();
    m_nutzer->setText(m_nutzerText);
}

QString KitFenster::dupliziere()
{
    if (m_kit.name.isEmpty()) return {};
    // „808 copy", „808 copy 2", …: der erste Titel, dessen Datei es noch nicht gibt
    const QString basis = tr("%1 copy").arg(m_kit.titel);
    QString titel = basis;
    QString name = eigenerName(titel);
    for (int n = 2; n < 1000 && (eigeneKitDatei(name).isEmpty() || QFileInfo::exists(eigeneKitDatei(name))); ++n)
    {
        titel = QStringLiteral("%1 %2").arg(basis).arg(n);
        name = eigenerName(titel);
    }
    const QString datei = eigeneKitDatei(name);
    if (datei.isEmpty() || QFileInfo::exists(datei)) return {};
    DrumKit kopie = m_kit;
    kopie.titel = titel;
    kopie.name = QLatin1String(kEigenVorsatz) + name;
    if (!speichereKit(datei, kopie)) return {};
    // die Kopie ist offen, wie sie geschrieben ist (Pfade, wie sie wieder gelesen werden)
    Q_EMIT kitsGeaendert();
    (void)waehle(kopie.name);
    return kopie.name;
}

bool KitFenster::benenne(const QString& titel)
{
    const QString kern = eigenerKern(m_kit.name);
    const QString sauber = titel.trimmed();
    if (kern.isEmpty() || sauber.isEmpty()) return false;
    // geschrieben wird, was gespeichert war — mit dem neuen Titel; Aenderungen von Hand bleiben offen
    DrumKit datei = m_vorlage;
    datei.titel = sauber;
    if (!speichereKit(eigeneKitDatei(kern), datei)) return false;
    m_vorlage.titel = sauber;
    m_kit.titel = sauber;
    for (DrumKit& stand : m_zurueck) stand.titel = sauber;
    for (DrumKit& stand : m_vor) stand.titel = sauber;
    Q_EMIT gespeichert(m_kit.name);
    baueListe();
    zeigeAlles();
    return true;
}

bool KitFenster::loesche()
{
    const QString kern = eigenerKern(m_kit.name);
    const QString datei = kern.isEmpty() ? QString() : eigeneKitDatei(kern);
    if (datei.isEmpty() || !QFile::remove(datei)) return false;
    m_kit = DrumKit{};
    m_vorlage = DrumKit{};
    Q_EMIT kitsGeaendert();
    oeffne(QString());
    return true;
}

void KitFenster::frageUndBenenne()
{
    bool ok = false;
    const QString titel = QInputDialog::getText(this, tr("Rename kit"), tr("Name of the kit:"), QLineEdit::Normal,
                                                m_kit.titel, &ok);
    if (!ok || titel.trimmed().isEmpty() || titel.trimmed() == m_kit.titel) return;
    if (!benenne(titel)) QMessageBox::warning(this, tr("Kits"), tr("Could not rename the kit %1").arg(m_kit.titel));
}

void KitFenster::frageUndLoesche()
{
    if (eigenerKern(m_kit.name).isEmpty()) return;
    if (QMessageBox::question(this, tr("Delete kit"),
                              tr("Delete your kit %1? Loops that use it keep their sounds; their Kit button then "
                                 "shows (not found).")
                                  .arg(m_kit.titel)) != QMessageBox::Yes)
        return;
    if (!loesche()) QMessageBox::warning(this, tr("Kits"), tr("Could not delete the kit %1").arg(m_kit.titel));
}

bool KitFenster::darfVerlassen()
{
    if (!geaendert() || schreibgeschuetzt()) return true;
    const QMessageBox::StandardButton wahl =
        QMessageBox::question(this, tr("Kits"), tr("Save the changes to %1?").arg(m_kit.titel),
                              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
    if (wahl == QMessageBox::Cancel) return false;
    if (wahl == QMessageBox::Save) return !speichere().isEmpty();
    m_kit = m_vorlage;  // verworfen
    m_zurueck.clear();
    m_vor.clear();
    zeigeAlles();
    return true;
}

void KitFenster::closeEvent(QCloseEvent* event)
{
    if (!darfVerlassen())
    {
        event->ignore();
        return;
    }
    QWidget::closeEvent(event);
}

// --- das offene Kit ------------------------------------------------------------

bool KitFenster::schreibgeschuetzt() const
{
    return eigenerKern(m_kit.name).isEmpty();
}

QString KitFenster::hinweisZeile() const
{
    if (m_kit.name.isEmpty()) return tr("No kit is open");
    return schreibgeschuetzt() ? tr("Built-in, read-only — Duplicate makes your own copy") : QString();
}

int KitFenster::zeile() const
{
    const QList<QTableWidgetSelectionRange> bereiche = m_tabelle->selectedRanges();
    return bereiche.isEmpty() ? -1 : bereiche.front().topRow();
}

void KitFenster::waehleZeile(int zeile)
{
    if (zeile < 0 || zeile >= m_tabelle->rowCount())
        m_tabelle->clearSelection();
    else
        m_tabelle->selectRow(zeile);
    zeigeKnoepfe();
}

void KitFenster::zeigeAlles()
{
    m_titel->setText(m_kit.titel);
    m_titel->setToolTip(m_kit.beschreibung);
    const QString hinweis = hinweisZeile();
    m_hinweis->setText(hinweis);
    m_hinweis->setVisible(!hinweis.isEmpty());
    m_nutzer->setText(m_nutzerText);
    setWindowTitle(geaendert() ? tr("Kits — %1 *").arg(m_kit.titel) : tr("Kits — %1").arg(m_kit.titel));
    zeigeTabelle();
    zeigeKnoepfe();
}

void KitFenster::zeigeTabelle()
{
    const int gewaehlt = zeile();
    const int anzahl = static_cast<int>(m_kit.klaenge.size());
    // Dieselben Rollen in derselben Reihenfolge: die Zeilen an Ort und Stelle
    // nachstellen. Neu gebaut wird nur, wenn sich die Rollen aendern — eine
    // Auswahl, die gerade meldet, darf nicht unter ihrem Aufruf geloescht werden.
    bool gleich = m_tabelle->rowCount() == anzahl && static_cast<int>(m_zeilen.size()) == anzahl;
    for (int z = 0; gleich && z < anzahl; ++z)
        gleich = m_tabelle->item(z, kSpalteRolle) != nullptr &&
                 m_tabelle->item(z, kSpalteRolle)->text() == m_kit.klaenge[static_cast<std::size_t>(z)].rolle;
    if (!gleich)
    {
        const QSignalBlocker ruhe(m_tabelle);
        m_tabelle->setRowCount(0);
        m_zeilen.clear();
        m_tabelle->setRowCount(anzahl);
        for (int z = 0; z < anzahl; ++z)
        {
            Zeile teile;
            auto* rolle = new QTableWidgetItem(m_kit.klaenge[static_cast<std::size_t>(z)].rolle);
            m_tabelle->setItem(z, kSpalteRolle, rolle);
            for (const int spalte : {kSpalteKlang, kSpalteLaut, kSpaltePan, kSpalteSpielen})
                m_tabelle->setItem(z, spalte, new QTableWidgetItem);  // damit ein Klick daneben die Zeile waehlt
            teile.klang = new KlangAuswahl;
            teile.klang->setObjectName(QStringLiteral("kitsSound%1").arg(z));
            teile.laut = new QSlider(Qt::Horizontal);
            teile.laut->setRange(0, 100);
            teile.laut->setToolTip(tr("Volume of this role"));
            teile.pan = new QSlider(Qt::Horizontal);
            teile.pan->setRange(-100, 100);
            teile.pan->setToolTip(tr("Pan: left to right"));
            teile.spielen = new QToolButton;
            teile.spielen->setObjectName(QStringLiteral("kitsPlay%1").arg(z));
            teile.spielen->setText(QString::fromUtf8("\xE2\x96\xB6"));  // ▶
            teile.spielen->setAutoRaise(true);
            teile.spielen->setToolTip(tr("Play this role"));
            m_tabelle->setCellWidget(z, kSpalteKlang, teile.klang);
            m_tabelle->setCellWidget(z, kSpalteLaut, teile.laut);
            m_tabelle->setCellWidget(z, kSpaltePan, teile.pan);
            m_tabelle->setCellWidget(z, kSpalteSpielen, teile.spielen);
            connect(teile.klang, &QComboBox::activated, this, [this, z, auswahl = teile.klang](int index) {
                waehleZeile(z);
                klangGewaehlt(z, auswahl->itemData(index).toString());
            });
            connect(teile.spielen, &QToolButton::clicked, this, [this, z] {
                waehleZeile(z);
                (void)spieleRolle(z);
            });
            // ein Schieber meldet beim Loslassen — nicht jeden Pixel als Schritt fuer Undo
            const auto schieber = [this, z](QSlider* regler, bool pan) {
                const auto melde = [this, z, regler, pan] {
                    if (pan)
                        (void)setzePan(z, regler->value() / 100.0);
                    else
                        (void)setzeLaut(z, regler->value() / 100.0);
                };
                connect(regler, &QSlider::sliderPressed, this, [this, z] { waehleZeile(z); });
                connect(regler, &QSlider::sliderReleased, this, melde);
                connect(regler, &QSlider::valueChanged, this, [regler, melde] {
                    if (!regler->isSliderDown()) melde();
                });
            };
            schieber(teile.laut, false);
            schieber(teile.pan, true);
            m_zeilen.push_back(teile);
        }
        if (gewaehlt >= 0 && anzahl > 0) m_tabelle->selectRow(std::min(gewaehlt, anzahl - 1));
    }
    const bool aenderbar = !schreibgeschuetzt();
    const QStringList eigene = eigeneSamples();  // einmal gelesen, fuer alle Zeilen
    const QStringList bibliothek = bibliotheksKlaenge();
    for (int z = 0; z < anzahl; ++z)
    {
        const KitKlang& klang = m_kit.klaenge[static_cast<std::size_t>(z)];
        Zeile& teile = m_zeilen[static_cast<std::size_t>(z)];
        teile.klang->fuelle(alsSpur(klang), m_kitKlaenge, eigene, bibliothek, false);
        teile.klang->setEnabled(aenderbar);
        const QSignalBlocker a(teile.laut);
        const QSignalBlocker b(teile.pan);
        if (!teile.laut->isSliderDown()) teile.laut->setValue(static_cast<int>(std::lround(klang.laut * 100.0)));
        if (!teile.pan->isSliderDown()) teile.pan->setValue(static_cast<int>(std::lround(klang.pan * 100.0)));
        teile.laut->setEnabled(aenderbar);
        teile.pan->setEnabled(aenderbar);
        teile.spielen->setEnabled(!alsSpur(klang).spielt().isEmpty());
    }
}

void KitFenster::zeigeKnoepfe()
{
    const bool offen = !m_kit.name.isEmpty();
    const bool aenderbar = offen && !schreibgeschuetzt();
    m_undo->setEnabled(kannRueckgaengig());
    m_redo->setEnabled(kannWiederherstellen());
    m_duplizieren->setEnabled(offen);
    m_umbenennen->setEnabled(aenderbar);
    m_loeschen->setEnabled(aenderbar);
    m_rolleNeu->setEnabled(aenderbar);
    m_rolleWeg->setEnabled(aenderbar && zeile() >= 0 && zeile() < static_cast<int>(m_kit.klaenge.size()));
    m_alle->setEnabled(std::any_of(m_kit.klaenge.begin(), m_kit.klaenge.end(),
                                   [](const KitKlang& k) { return !alsSpur(k).spielt().isEmpty(); }));
    m_sichern->setEnabled(aenderbar && geaendert());
    const int gewaehlt = zeile();
    m_rolleWeg->setToolTip(gewaehlt >= 0 && gewaehlt < static_cast<int>(m_kit.klaenge.size())
                               ? tr("Removes %1 from the kit").arg(m_kit.klaenge[static_cast<std::size_t>(gewaehlt)].rolle)
                               : tr("Choose a row first"));
}

bool KitFenster::aendere(const std::function<void(DrumKit&)>& tu)
{
    if (schreibgeschuetzt()) return false;
    DrumKit neu = m_kit;
    tu(neu);
    if (neu == m_kit)
    {
        zeigeAlles();  // eine Auswahl, die dasselbe noch einmal nennt, steht wieder richtig
        return false;
    }
    m_zurueck.push_back(m_kit);
    m_vor.clear();
    m_kit = std::move(neu);
    zeigeAlles();
    return true;
}

bool KitFenster::aendereZeile(int zeile, const std::function<void(KitKlang&)>& tu)
{
    if (zeile < 0 || zeile >= static_cast<int>(m_kit.klaenge.size())) return false;
    return aendere([&](DrumKit& kit) { tu(kit.klaenge[static_cast<std::size_t>(zeile)]); });
}

bool KitFenster::setzeKlang(int zeile, const QString& klang)
{
    return aendereZeile(zeile, [&klang](KitKlang& k) {
        k.klang = klang;
        // die Rolle spielt wieder einen Klang: kein Instrument, Note und Length auf der Vorgabe
        k.instrument.clear();
        k.hoehe = KitKlang().hoehe;
        k.dauerMs = kSpurDauerMs;
    });
}

bool KitFenster::setzeInstrument(int zeile, const QString& nennung)
{
    if (nennung.isEmpty()) return false;
    return aendereZeile(zeile, [&nennung](KitKlang& k) {
        const bool erstes = k.instrument.isEmpty();
        k.instrument = nennung;
        if (!erstes) return;  // ein anderes Instrument: Note und Length bleiben
        Instrument instrument;
        k.hoehe = holeInstrument(nennung, instrument) ? InstrumentPalette::hoeheFuerArt(instrument.art) : KitKlang().hoehe;
        k.dauerMs = kSpurDauerMs;
    });
}

bool KitFenster::setzeLaut(int zeile, double laut)
{
    return aendereZeile(zeile, [laut](KitKlang& k) { k.laut = std::clamp(laut, 0.0, 1.0); });
}

bool KitFenster::setzePan(int zeile, double pan)
{
    return aendereZeile(zeile, [pan](KitKlang& k) { k.pan = std::clamp(pan, -1.0, 1.0); });
}

QStringList KitFenster::freieRollen() const
{
    // je Art die naechste Nummer hinter der hoechsten vergebenen (K5 a)
    std::map<QString, int> hoechste;
    for (const KitKlang& k : m_kit.klaenge)
        if (const std::optional<Rolle> r = leseRolle(k.rolle)) hoechste[r->art] = std::max(hoechste[r->art], r->nummer);
    QStringList frei;
    for (const QString& art : rollenArten()) frei << rollenText({art, hoechste[art] + 1});
    return frei;
}

int KitFenster::fuegeRolleHinzu(const QString& rolle)
{
    const std::optional<Rolle> gelesen = leseRolle(rolle);
    if (!gelesen || schreibgeschuetzt()) return -1;
    const QString text = rollenText(*gelesen);
    if (std::any_of(m_kit.klaenge.begin(), m_kit.klaenge.end(), [&text](const KitKlang& k) { return k.rolle == text; }))
        return -1;
    // an ihren Platz: nach der Reihenfolge der Arten, dann der Nummer
    const auto platz = std::find_if(m_kit.klaenge.begin(), m_kit.klaenge.end(),
                                    [&text](const KitKlang& k) { return platzVon(text) < platzVon(k.rolle); });
    const int zeile = static_cast<int>(platz - m_kit.klaenge.begin());
    KitKlang neu;
    neu.rolle = text;
    if (!aendere([&](DrumKit& kit) { kit.klaenge.insert(kit.klaenge.begin() + zeile, neu); })) return -1;
    waehleZeile(zeile);
    return zeile;
}

bool KitFenster::entferneRolle(int zeile)
{
    if (zeile < 0 || zeile >= static_cast<int>(m_kit.klaenge.size())) return false;
    const bool weg = aendere([zeile](DrumKit& kit) { kit.klaenge.erase(kit.klaenge.begin() + zeile); });
    if (weg) waehleZeile(std::min(zeile, static_cast<int>(m_kit.klaenge.size()) - 1));
    return weg;
}

void KitFenster::fuelleRollenMenue(QMenu* menue)
{
    menue->clear();
    for (const QString& rolle : freieRollen()) menue->addAction(rolle, this, [this, rolle] { (void)fuegeRolleHinzu(rolle); });
}

bool KitFenster::rueckgaengig()
{
    if (m_zurueck.empty()) return false;
    m_vor.push_back(m_kit);
    m_kit = m_zurueck.back();
    m_zurueck.pop_back();
    zeigeAlles();
    return true;
}

bool KitFenster::wiederherstellen()
{
    if (m_vor.empty()) return false;
    m_zurueck.push_back(m_kit);
    m_kit = m_vor.back();
    m_vor.pop_back();
    zeigeAlles();
    return true;
}

QString KitFenster::speichere()
{
    const QString kern = eigenerKern(m_kit.name);
    const QString datei = kern.isEmpty() ? QString() : eigeneKitDatei(kern);
    if (datei.isEmpty() || !speichereKit(datei, m_kit)) return {};
    m_vorlage = m_kit;
    Q_EMIT gespeichert(m_kit.name);
    zeigeAlles();
    return m_kit.name;
}

// --- Klang, Anspielen, Used by -------------------------------------------------

Spur KitFenster::alsSpur(const KitKlang& klang)
{
    Spur spur;
    spur.name = klang.rolle;
    spur.rolle = klang.rolle;
    spur.klang = klang.klang;
    spur.instrument = klang.instrument;
    spur.hoehe = klang.hoehe;
    spur.dauerMs = klang.dauerMs;
    spur.laut = klang.laut;
    spur.pan = klang.pan;
    return spur;
}

bool KitFenster::spieleRolle(int zeile)
{
    if (zeile < 0 || zeile >= static_cast<int>(m_kit.klaenge.size())) return false;
    const Spur spur = alsSpur(m_kit.klaenge[static_cast<std::size_t>(zeile)]);
    if (spur.spielt().isEmpty()) return false;
    Q_EMIT anspielenVerlangt({spur});
    return true;
}

bool KitFenster::spieleAlle()
{
    std::vector<Spur> spuren;
    for (const KitKlang& klang : m_kit.klaenge)
        if (Spur spur = alsSpur(klang); !spur.spielt().isEmpty()) spuren.push_back(std::move(spur));
    if (spuren.empty()) return false;
    Q_EMIT anspielenVerlangt(spuren);
    return true;
}

QStringList KitFenster::klangZeilen(int zeile) const
{
    const KlangAuswahl* auswahl = klangAuswahl(zeile);
    return auswahl != nullptr ? auswahl->zeilen() : QStringList{};
}

KlangAuswahl* KitFenster::klangAuswahl(int zeile) const
{
    return zeile >= 0 && zeile < static_cast<int>(m_zeilen.size()) ? m_zeilen[static_cast<std::size_t>(zeile)].klang : nullptr;
}

void KitFenster::klangGewaehlt(int zeile, const QString& wahl)
{
    if (wahl == kKlangWahlJetzt || wahl == kKlangWahlKopf) return;
    if (wahl == kKlangWahlDatei)
    {
        frageNachSampleDatei(zeile);
        return;
    }
    if (wahl == kKlangWahlInstrument)
    {
        zeigeTabelle();  // die Auswahl zeigt wieder, was die Rolle spielt — geaendert wird im Fenster
        (void)oeffnePalette(zeile);
        return;
    }
    (void)setzeKlang(zeile, wahl);
}

void KitFenster::frageNachSampleDatei(int zeile)
{
    QSettings einstellungen;
    const QString pfad =
        QFileDialog::getOpenFileName(this, tr("Choose sample"), einstellungen.value(QLatin1String(kSampleOrdner)).toString(),
                                     tr("Audio (*.wav *.mp3 *.flac *.ogg);;All files (*)"));
    if (pfad.isEmpty())
    {
        zeigeTabelle();  // die Auswahl steht wieder auf dem Klang der Rolle
        return;
    }
    einstellungen.setValue(QLatin1String(kSampleOrdner), QFileInfo(pfad).absolutePath());
    // Entscheid S2 a: kopieren oder am Ort lassen — dieselbe Frage wie im Drum-Editor
    const SampleKopie wahl = waehleSampleKopie(this, pfad, tr("Without a copy the kit names the file at"),
                                               QStringLiteral("kitSampleCopyDialog"));
    if (wahl == SampleKopie::Fragen)
    {
        zeigeTabelle();  // abgebrochen
        return;
    }
    if (!nimmSampleDatei(zeile, pfad, wahl == SampleKopie::Kopieren))
    {
        QMessageBox::warning(this, tr("Sample"), tr("Could not copy %1 to My samples").arg(QDir::toNativeSeparators(pfad)));
        zeigeTabelle();
    }
}

bool KitFenster::nimmSampleDatei(int zeile, const QString& pfad, bool kopieren)
{
    const QString nennung = kopieren ? nimmZuEigenen(pfad) : pfad;
    if (nennung.isEmpty()) return false;
    (void)setzeKlang(zeile, nennung);
    return true;
}

bool KitFenster::oeffnePalette(int zeile, bool zeigen)
{
    if (schreibgeschuetzt() || zeile < 0 || zeile >= static_cast<int>(m_kit.klaenge.size())) return false;
    if (m_palette == nullptr)
    {
        m_palette = new InstrumentPalette(this);
        // Use: das Instrument kommt in die Rolle, fuer die das Fenster gerade waehlt
        connect(m_palette, &InstrumentPalette::verwenden, this,
                [this](const QString& nennung) { (void)setzeInstrument(m_paletteZeile, nennung); });
        connect(m_palette, &InstrumentPalette::tonVerlangt, this, &KitFenster::instrumentTonVerlangt);
        // Ctrl+Z / Ctrl+Y im Fenster Instruments: hier gilt das Undo des Kits
        connect(m_palette, &InstrumentPalette::rueckgaengigVerlangt, this, [this](bool wiederholen) {
            (void)(wiederholen ? wiederherstellen() : rueckgaengig());
        });
    }
    m_paletteZeile = zeile;
    const KitKlang& klang = m_kit.klaenge[static_cast<std::size_t>(zeile)];
    m_palette->setzeZiel(tr("kit %1").arg(m_kit.titel), klang.rolle);
    if (zeigen)
        m_palette->zeigeVorn();
    else
        m_palette->liesNeu();
    if (!klang.instrument.isEmpty())
        (void)m_palette->waehle(klang.instrument);
    else
        (void)m_palette->zeigeArt(QStringLiteral("drums"));
    return true;
}

void KitFenster::rechneNutzer()
{
    if (m_kit.name.isEmpty())
    {
        m_nutzerText.clear();
        return;
    }
    // die Loops, die LumiViz kennt: die mitgelieferten, die zuletzt geoeffneten und der offene
    QStringList nutzer = loopsMitKit(m_kit.name, mitgelieferteLoopDateien() + zuletztGeoeffneteLoops());
    if (m_hatOffen && nenntKit(m_offen, m_kit.name)) nutzer.prepend(tr("the open loop"));
    const QString gezaehlt = tr("built-in loops, the open one and the last %1 opened").arg(kZuletztLoopsMax);
    if (nutzer.isEmpty())
    {
        m_nutzerText = tr("Not used by any loop LumiViz knows (%1)").arg(gezaehlt);
        return;
    }
    QString liste = nutzer.mid(0, kNutzerBeimNamen).join(QStringLiteral(", "));
    if (nutzer.size() > kNutzerBeimNamen) liste = tr("%1 and %2 more").arg(liste).arg(nutzer.size() - kNutzerBeimNamen);
    m_nutzerText = tr("Used by: %1 · counted: %2").arg(liste, gezaehlt);
}

}  // namespace lumi::loop
