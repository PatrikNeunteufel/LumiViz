/**
 ****************************************************************************************
 * @file   DatensicherungDialoge.cpp
 * @brief  Die eigenen Daten exportieren und importieren — die drei Dialoge
 *         (docs/core-services/Datenablage_Konzept.md §4a, Stufe A4b)
 ****************************************************************************************
 */

#include "UI/dialogs/DatensicherungDialoge.hpp"

#include "AppInfo.hpp"
#include "UI/dialogs/PaketDialoge.hpp"
#include "UI/widgets/PresetTypeIcons.hpp"
#include "paket/Import.hpp"
#include "services/IEventBus.hpp"
#include "services/events/UIEvents.hpp"

#include <QButtonGroup>
#include <QCheckBox>
#include <QDate>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSettings>
#include <QStandardPaths>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace lumi::ui
{

namespace ds = lumi::core::datensicherung;

namespace
{

const char* const kLetzterOrdner = "data/lastExportDir";  // ein Pfad: geht selbst nie in einen Export
const QString kSettingsId = QStringLiteral("settings");

QFrame* strich(QWidget* parent)
{
    auto* linie = new QFrame(parent);
    linie->setFrameShape(QFrame::HLine);
    linie->setFrameShadow(QFrame::Sunken);
    return linie;
}

QString datumUndGroesse(const QDateTime& zeit, qint64 bytes)
{
    return QStringLiteral("%1 · %2").arg(zeit.toString(QStringLiteral("yyyy-MM-dd")), groesseAlsText(bytes));
}

/// „Visualizer presets · equalizer": der Bereich und der Unterordner darin.
QString gruppenTitel(const ds::Bereich& bereich, const QString& rel)
{
    const QString ordner = rel.left(rel.lastIndexOf(QLatin1Char('/')));
    QString unter = ordner;
    for (const QString& wurzel : bereich.ordner)
        if (ordner.compare(wurzel, Qt::CaseInsensitive) == 0 ||
            ordner.startsWith(wurzel + QLatin1Char('/'), Qt::CaseInsensitive))
            unter = ordner.mid(wurzel.size() + 1);
    return unter.isEmpty() ? bereich.titel : QStringLiteral("%1 · %2").arg(bereich.titel, unter);
}

QString startOrdner(QSettings& einstellungen)
{
    const QString gemerkt = einstellungen.value(QLatin1String(kLetzterOrdner)).toString();
    if (!gemerkt.isEmpty() && QFileInfo(gemerkt).isDir()) return gemerkt;
    const QString dokumente = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    return dokumente.isEmpty() ? QDir::homePath() : dokumente;
}

}  // namespace

QString groesseAlsText(qint64 bytes)
{
    if (bytes < 1024) return QStringLiteral("%1 B").arg(bytes);
    if (bytes < 1024 * 1024) return QStringLiteral("%1 KB").arg((bytes + 512) / 1024);
    return QStringLiteral("%1 MB").arg(static_cast<double>(bytes) / (1024.0 * 1024.0), 0, 'f', 1);
}

// =============================================================================
// Export your data
// =============================================================================

DatenExportDialog::DatenExportDialog(const QString& benutzerOrdner, QWidget* parent, QSettings* einstellungen)
    : QDialog(parent), m_ordner(benutzerOrdner), m_einstellungen(einstellungen)
{
    setObjectName(QStringLiteral("dataExportDialog"));
    setWindowTitle(tr("Export your data"));

    auto* aussen = new QVBoxLayout(this);

    auto* alle = new QPushButton(tr("Select all"), this);
    alle->setObjectName(QStringLiteral("dataExportAll"));
    auto* keine = new QPushButton(tr("Select none"), this);
    keine->setObjectName(QStringLiteral("dataExportNone"));
    auto* kopf = new QHBoxLayout;
    kopf->addWidget(alle);
    kopf->addWidget(keine);
    kopf->addStretch(1);
    aussen->addLayout(kopf);

    // --- die Bereiche: oben das Eigene mit Namen, abgesetzt Settings und die Songtexte
    auto* oben = new QGridLayout;
    auto* unten = new QGridLayout;
    oben->setColumnStretch(0, 1);
    unten->setColumnStretch(0, 1);
    // Settings steht in der Skizze vor den Songtexten: Zeile 0 des unteren Blocks
    m_settings = new QCheckBox(tr("Settings (favorites, recent lists, options)"), this);
    m_settings->setObjectName(QStringLiteral("dataExport_settings"));
    m_settings->setToolTip(tr("Shortcuts, favorites and options. Window layout, remembered folders and the "
                              "Shadertoy API key are never exported."));
    connect(m_settings, &QCheckBox::toggled, this, [this] { aktualisiereSumme(); });
    unten->addWidget(m_settings, 0, 0);
    int reiheOben = 0;
    int reiheUnten = 1;
    for (const ds::Bereich& bereich : ds::bereiche())
    {
        const int zahl = static_cast<int>(ds::sammle(m_ordner, bereich).size());
        m_zahl[bereich.id] = zahl;
        auto* haken = new QCheckBox(bereich.titel, this);
        haken->setObjectName(QStringLiteral("dataExport_") + bereich.id);
        haken->setChecked(bereich.anfangsAn);
        auto* anzahl = new QLabel(QString::number(zahl), this);
        anzahl->setEnabled(false);  // grau: eine Auskunft
        QGridLayout* gitter = bereich.anfangsAn ? oben : unten;
        int& reihe = bereich.anfangsAn ? reiheOben : reiheUnten;
        gitter->addWidget(haken, reihe, 0);
        gitter->addWidget(anzahl, reihe, 1, Qt::AlignRight);
        ++reihe;
        connect(haken, &QCheckBox::toggled, this, [this] { aktualisiereSumme(); });
        m_haken[bereich.id] = haken;
    }

    aussen->addLayout(oben);
    aussen->addWidget(strich(this));
    aussen->addLayout(unten);
    aussen->addWidget(strich(this));

    // --- wohin
    QSettings eigene;
    QSettings& s = m_einstellungen != nullptr ? *m_einstellungen : eigene;
    m_ziel = new QLineEdit(this);
    m_ziel->setObjectName(QStringLiteral("dataExportPath"));
    m_ziel->setMinimumWidth(360);
    m_ziel->setText(QDir::toNativeSeparators(
        QDir(startOrdner(s)).filePath(QStringLiteral("LumiViz_%1%2")
                                          .arg(QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd")),
                                               QLatin1String(ds::kEndung)))));
    auto* suchen = new QPushButton(tr("Browse..."), this);
    suchen->setObjectName(QStringLiteral("dataExportBrowse"));
    auto* pfad = new QHBoxLayout;
    pfad->addWidget(m_ziel, 1);
    pfad->addWidget(suchen);
    aussen->addLayout(pfad);

    m_summe = new QLabel(this);
    m_summe->setObjectName(QStringLiteral("dataExportSummary"));
    m_summe->setEnabled(false);
    auto* abbrechen = new QPushButton(tr("Cancel"), this);
    m_export = new QPushButton(tr("Export"), this);
    m_export->setObjectName(QStringLiteral("dataExportRun"));
    m_export->setDefault(true);
    auto* fuss = new QHBoxLayout;
    fuss->addWidget(m_summe, 1);
    fuss->addWidget(abbrechen);
    fuss->addWidget(m_export);
    aussen->addLayout(fuss);

    connect(alle, &QPushButton::clicked, this, [this] {
        for (const auto& [id, haken] : m_haken) haken->setChecked(true);
        m_settings->setChecked(true);
    });
    connect(keine, &QPushButton::clicked, this, [this] {
        for (const auto& [id, haken] : m_haken) haken->setChecked(false);
        m_settings->setChecked(false);
    });
    connect(suchen, &QPushButton::clicked, this, [this] { waehleZiel(); });
    connect(m_ziel, &QLineEdit::textChanged, this, [this] { aktualisiereSumme(); });
    connect(abbrechen, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_export, &QPushButton::clicked, this, [this] { exportiere(); });

    aktualisiereSumme();
}

void DatenExportDialog::setzeAn(const QString& id, bool an)
{
    if (id == kSettingsId)
        m_settings->setChecked(an);
    else if (const auto treffer = m_haken.find(id); treffer != m_haken.end())
        treffer->second->setChecked(an);
}

bool DatenExportDialog::istAn(const QString& id) const
{
    if (id == kSettingsId) return m_settings->isChecked();
    const auto treffer = m_haken.find(id);
    return treffer != m_haken.end() && treffer->second->isChecked();
}

void DatenExportDialog::setzeZiel(const QString& pfad)
{
    m_ziel->setText(QDir::toNativeSeparators(pfad));
}

QString DatenExportDialog::ziel() const
{
    return QDir::fromNativeSeparators(m_ziel->text().trimmed());
}

ds::ExportWahl DatenExportDialog::wahl() const
{
    ds::ExportWahl wahl;
    for (const ds::Bereich& bereich : ds::bereiche())
        if (istAn(bereich.id)) wahl.bereiche << bereich.id;
    wahl.einstellungen = m_settings->isChecked();
    return wahl;
}

void DatenExportDialog::aktualisiereSumme()
{
    int dateien = 0;
    int gruppen = 0;
    for (const auto& [id, haken] : m_haken)
    {
        if (!haken->isChecked() || m_zahl[id] == 0) continue;
        dateien += m_zahl[id];
        ++gruppen;
    }
    QString text = tr("%1 file(s) in %2 group(s)").arg(dateien).arg(gruppen);
    if (m_settings->isChecked()) text += tr(", and settings");
    m_summe->setText(text);
    m_export->setEnabled((dateien > 0 || m_settings->isChecked()) && !ziel().isEmpty());
}

void DatenExportDialog::waehleZiel()
{
    QString pfad = QFileDialog::getSaveFileName(this, tr("Export your data"), ziel(),
                                                tr("LumiViz data (*%1)").arg(QLatin1String(ds::kEndung)));
    if (pfad.isEmpty()) return;
    if (!pfad.endsWith(QLatin1String(ds::kEndung), Qt::CaseInsensitive)) pfad += QLatin1String(ds::kEndung);
    setzeZiel(pfad);
}

ds::ExportErgebnis DatenExportDialog::fuehreAus()
{
    QSettings eigene;
    QSettings& s = m_einstellungen != nullptr ? *m_einstellungen : eigene;
    const ds::ExportWahl gewaehlt = wahl();
    const QJsonObject werte = gewaehlt.einstellungen ? ds::leseEinstellungen(s) : QJsonObject();
    QString pfad = ziel();
    if (!pfad.endsWith(QLatin1String(ds::kEndung), Qt::CaseInsensitive)) pfad += QLatin1String(ds::kEndung);
    const ds::ExportErgebnis ergebnis = ds::exportiere(m_ordner, gewaehlt, werte, pfad, lumi::appVersion());
    if (ergebnis.ok) s.setValue(QLatin1String(kLetzterOrdner), QFileInfo(pfad).absolutePath());
    return ergebnis;
}

void DatenExportDialog::exportiere()
{
    QString pfad = ziel();
    if (!pfad.endsWith(QLatin1String(ds::kEndung), Qt::CaseInsensitive)) pfad += QLatin1String(ds::kEndung);
    if (QFileInfo::exists(pfad) &&
        QMessageBox::question(this, windowTitle(),
                              tr("%1 already exists.\nReplace it?").arg(QDir::toNativeSeparators(pfad))) !=
            QMessageBox::Yes)
        return;
    const ds::ExportErgebnis ergebnis = fuehreAus();
    if (!ergebnis.ok)
    {
        QMessageBox::warning(this, windowTitle(), tr("The export failed.\n%1").arg(ergebnis.fehler));
        return;
    }
    QMessageBox::information(this, windowTitle(),
                             tr("Exported %1 file(s) in %2 group(s)%3 to\n%4")
                                 .arg(ergebnis.dateien)
                                 .arg(ergebnis.bereiche)
                                 .arg(m_settings->isChecked() ? tr(" and your settings") : QString(),
                                      QDir::toNativeSeparators(pfad)));
    accept();
}

// =============================================================================
// Let me decide for each file
// =============================================================================

namespace
{
constexpr int kSpalteName = 0;
constexpr int kSpalteMeine = 1;
constexpr int kSpalteImportierte = 2;
constexpr int kRolleRel = Qt::UserRole + 1;
}  // namespace

DatenJeDateiDialog::DatenJeDateiDialog(const std::vector<ds::ImportBereich>& bereiche, QWidget* parent)
    : QDialog(parent)
{
    setObjectName(QStringLiteral("dataImportDecideDialog"));

    auto* aussen = new QVBoxLayout(this);
    auto* satz = new QLabel(
        tr("Tick what you want to keep. Both ticked: the imported file gets a number added to its name. "
           "Nothing ticked: yours stays."),
        this);
    satz->setWordWrap(true);
    satz->setEnabled(false);
    aussen->addWidget(satz);

    // ganze Spalten setzen — in der Skizze Haken im Kopf der Tabelle; ein Kopf
    // traegt in Qt keine Haken, deshalb zwei Knoepfe darueber
    auto* alleMeine = new QPushButton(tr("Keep all mine"), this);
    alleMeine->setObjectName(QStringLiteral("dataDecideAllMine"));
    auto* alleImportierten = new QPushButton(tr("Take all imported"), this);
    alleImportierten->setObjectName(QStringLiteral("dataDecideAllImported"));
    auto* beide = new QPushButton(tr("Keep both everywhere"), this);
    beide->setObjectName(QStringLiteral("dataDecideAllBoth"));
    auto* knoepfe = new QHBoxLayout;
    knoepfe->addWidget(alleMeine);
    knoepfe->addWidget(alleImportierten);
    knoepfe->addWidget(beide);
    knoepfe->addStretch(1);
    aussen->addLayout(knoepfe);

    m_liste = new QTreeWidget(this);
    m_liste->setObjectName(QStringLiteral("dataDecideList"));
    m_liste->setColumnCount(3);
    m_liste->setHeaderLabels({tr("Name"), tr("Mine"), tr("Imported")});
    m_liste->setRootIsDecorated(false);
    m_liste->setSelectionMode(QAbstractItemView::NoSelection);
    m_liste->setUniformRowHeights(true);
    m_liste->header()->setStretchLastSection(false);
    m_liste->header()->setSectionResizeMode(kSpalteName, QHeaderView::Stretch);
    m_liste->header()->setSectionResizeMode(kSpalteMeine, QHeaderView::ResizeToContents);
    m_liste->header()->setSectionResizeMode(kSpalteImportierte, QHeaderView::ResizeToContents);
    aussen->addWidget(m_liste, 1);

    QString gruppe;
    QTreeWidgetItem* kopf = nullptr;
    for (const ds::ImportBereich& importBereich : bereiche)
    {
        const ds::Bereich* bereich = ds::findeBereich(importBereich.id);
        if (bereich == nullptr) continue;
        for (const ds::ImportDatei& datei : importBereich.dateien)
        {
            if (datei.zustand != ds::Zustand::NameVergeben) continue;
            const QString titel = gruppenTitel(*bereich, datei.rel);
            if (kopf == nullptr || titel != gruppe)
            {
                gruppe = titel;
                kopf = new QTreeWidgetItem(m_liste, {titel});
                kopf->setFirstColumnSpanned(true);
                kopf->setFlags(Qt::ItemIsEnabled);
                kopf->setForeground(kSpalteName, palette().color(QPalette::Disabled, QPalette::Text));
            }
            const bool importJuenger = datei.geaendert > datei.eigeneGeaendert;
            const bool eigeneJuenger = datei.eigeneGeaendert > datei.geaendert;
            auto* zeile = new QTreeWidgetItem(kopf);
            zeile->setText(kSpalteName, QFileInfo(datei.rel).completeBaseName());
            // das Icon der Dateiart, wo es eines gibt (S120, Entscheid Patrik
            // A2 a): hier stehen Dateien verschiedener Art untereinander. Ein
            // Verlauf ist eine `.json` — er traegt das Icon seines Bereichs.
            QIcon art = dateiIconFuerEndung(QFileInfo(datei.rel).suffix());
            if (art.isNull() && bereich->id == QLatin1String("gradients")) art = dateiArtIcon(QStringLiteral("grad"));
            // ohne Icon ein leeres von gleicher Groesse: die Namen stehen untereinander
            if (art.isNull())
            {
                QPixmap leer(m_liste->iconSize().isValid() ? m_liste->iconSize() : QSize(16, 16));
                leer.fill(Qt::transparent);
                art = QIcon(leer);
            }
            zeile->setIcon(kSpalteName, art);
            zeile->setToolTip(kSpalteName, QDir::toNativeSeparators(datei.rel));
            zeile->setData(kSpalteName, kRolleRel, datei.rel);
            zeile->setText(kSpalteMeine, datumUndGroesse(datei.eigeneGeaendert, datei.eigeneGroesse) +
                                             (eigeneJuenger ? tr("  newer") : QString()));
            zeile->setText(kSpalteImportierte, datumUndGroesse(datei.geaendert, datei.groesse) +
                                                   (importJuenger ? tr("  newer") : QString()));
            zeile->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
            zeile->setCheckState(kSpalteMeine, Qt::Checked);
            zeile->setCheckState(kSpalteImportierte, Qt::Unchecked);
            m_zeilen.push_back(zeile);
        }
    }
    m_liste->expandAll();
    setWindowTitle(tr("%1 name(s) already exist").arg(zeilen()));

    m_summe = new QLabel(this);
    m_summe->setObjectName(QStringLiteral("dataDecideSummary"));
    m_summe->setEnabled(false);
    auto* zurueck = new QPushButton(tr("Back"), this);
    auto* weiter = new QPushButton(tr("Import"), this);
    weiter->setObjectName(QStringLiteral("dataDecideImport"));
    weiter->setDefault(true);
    auto* fuss = new QHBoxLayout;
    fuss->addWidget(m_summe, 1);
    fuss->addWidget(zurueck);
    fuss->addWidget(weiter);
    aussen->addLayout(fuss);

    connect(alleMeine, &QPushButton::clicked, this, [this] { setzeAlle(true, false); });
    connect(alleImportierten, &QPushButton::clicked, this, [this] { setzeAlle(false, true); });
    connect(beide, &QPushButton::clicked, this, [this] { setzeAlle(true, true); });
    connect(m_liste, &QTreeWidget::itemChanged, this, [this] { aktualisiereSumme(); });
    connect(zurueck, &QPushButton::clicked, this, &QDialog::reject);
    connect(weiter, &QPushButton::clicked, this, &QDialog::accept);

    resize(640, 420);
    aktualisiereSumme();
}

int DatenJeDateiDialog::zeilen() const
{
    return static_cast<int>(m_zeilen.size());
}

void DatenJeDateiDialog::setze(const QString& rel, bool meine, bool importierte)
{
    for (QTreeWidgetItem* zeile : m_zeilen)
    {
        if (zeile->data(kSpalteName, kRolleRel).toString() != rel) continue;
        zeile->setCheckState(kSpalteMeine, meine ? Qt::Checked : Qt::Unchecked);
        zeile->setCheckState(kSpalteImportierte, importierte ? Qt::Checked : Qt::Unchecked);
    }
}

void DatenJeDateiDialog::setzeAlle(bool meine, bool importierte)
{
    for (QTreeWidgetItem* zeile : m_zeilen)
    {
        zeile->setCheckState(kSpalteMeine, meine ? Qt::Checked : Qt::Unchecked);
        zeile->setCheckState(kSpalteImportierte, importierte ? Qt::Checked : Qt::Unchecked);
    }
}

std::map<QString, ds::Wahl> DatenJeDateiDialog::wahl() const
{
    std::map<QString, ds::Wahl> wahl;
    for (const QTreeWidgetItem* zeile : m_zeilen)
    {
        const bool meine = zeile->checkState(kSpalteMeine) == Qt::Checked;
        const bool importierte = zeile->checkState(kSpalteImportierte) == Qt::Checked;
        // beide aus: es wird nichts geloescht — die eigene bleibt
        wahl[zeile->data(kSpalteName, kRolleRel).toString()] =
            importierte ? (meine ? ds::Wahl::Beide : ds::Wahl::Importierte) : ds::Wahl::Meine;
    }
    return wahl;
}

QString DatenJeDateiDialog::summe() const
{
    return m_summe->text();
}

void DatenJeDateiDialog::aktualisiereSumme()
{
    if (m_summe == nullptr) return;  // waehrend des Aufbaus
    int behaelt = 0;
    int ersetzt = 0;
    int beide = 0;
    for (const auto& [rel, was] : wahl())
    {
        if (was == ds::Wahl::Meine)
            ++behaelt;
        else if (was == ds::Wahl::Importierte)
            ++ersetzt;
        else
            ++beide;
    }
    m_summe->setText(
        tr("Keeps %1 of yours, replaces %2, adds %3 under a new name").arg(behaelt).arg(ersetzt).arg(beide));
}

// =============================================================================
// Import data
// =============================================================================

DatenImportDialog::DatenImportDialog(const QString& datei, const QString& benutzerOrdner, QWidget* parent,
                                     QSettings* einstellungen)
    : QDialog(parent), m_datei(datei), m_ordner(benutzerOrdner), m_einstellungen(einstellungen),
      m_inhalt(ds::oeffne(datei, benutzerOrdner))
{
    setObjectName(QStringLiteral("dataImportDialog"));
    setWindowTitle(tr("Import data"));

    auto* aussen = new QVBoxLayout(this);
    auto* kopf = new QLabel(this);
    kopf->setObjectName(QStringLiteral("dataImportHead"));
    kopf->setEnabled(false);
    kopf->setWordWrap(true);
    // das Datei-Icon des Exports vor dem Namen (S120, Entscheid Patrik A2 a)
    auto* kopfReihe = new QHBoxLayout;
    if (const QIcon marke = dateiArtIcon(QStringLiteral("lvdata")); !marke.isNull())
    {
        auto* icon = new QLabel(this);
        icon->setObjectName(QStringLiteral("dataImportHeadIcon"));
        icon->setPixmap(marke.pixmap(18, 18));
        kopfReihe->addWidget(icon);  // mittig wie der Text daneben
    }
    kopfReihe->addWidget(kopf, 1);
    aussen->addLayout(kopfReihe);

    auto* abbrechen = new QPushButton(tr("Cancel"), this);
    m_import = new QPushButton(tr("Import"), this);
    m_import->setObjectName(QStringLiteral("dataImportRun"));
    m_import->setDefault(true);
    m_hinweis = new QLabel(this);
    m_hinweis->setObjectName(QStringLiteral("dataImportHint"));
    connect(abbrechen, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_import, &QPushButton::clicked, this, [this] { importiere(); });

    if (!m_inhalt.ok)
    {
        kopf->setText(tr("%1\n%2").arg(QFileInfo(datei).fileName(), m_inhalt.fehler));
        m_import->setEnabled(false);
    }
    else
    {
        QString text = tr("%1 · exported with LumiViz %2").arg(QFileInfo(datei).fileName(), m_inhalt.appVersion);
        if (m_inhalt.exportiert.isValid())
            text += tr(" on %1").arg(m_inhalt.exportiert.toString(QStringLiteral("yyyy-MM-dd")));
        if (m_inhalt.ausgelassen > 0)
            text += tr("\n%1 entr(ies) do not belong to any group and are left out.").arg(m_inhalt.ausgelassen);
        kopf->setText(text);

        // --- nur die Bereiche, die in der Datei stecken
        auto* gitter = new QGridLayout;
        gitter->setColumnStretch(0, 1);
        int reihe = 0;
        for (const ds::ImportBereich& importBereich : m_inhalt.bereiche)
        {
            const ds::Bereich* bereich = ds::findeBereich(importBereich.id);
            if (bereich == nullptr) continue;
            auto* haken = new QCheckBox(bereich->titel, this);
            haken->setObjectName(QStringLiteral("dataImport_") + bereich->id);
            haken->setChecked(true);
            const int vergeben = importBereich.vergeben();
            auto* zahl = new QLabel(vergeben > 0 ? tr("%1 · %2 exist").arg(importBereich.dateien.size()).arg(vergeben)
                                                 : QString::number(importBereich.dateien.size()),
                                    this);
            zahl->setEnabled(false);
            gitter->addWidget(haken, reihe, 0);
            gitter->addWidget(zahl, reihe, 1, Qt::AlignRight);
            ++reihe;
            m_haken[bereich->id] = haken;
            connect(haken, &QCheckBox::toggled, this, [this] { aktualisiere(); });
        }
        if (m_inhalt.hatEinstellungen)
        {
            m_settings = new QCheckBox(tr("Settings"), this);
            m_settings->setObjectName(QStringLiteral("dataImport_settings"));
            m_settings->setToolTip(tr("Shortcuts, favorites and options from the file replace yours. "
                                      "Everything else in your settings stays."));
            auto* ersetzt = new QLabel(tr("replaces yours"), this);
            ersetzt->setEnabled(false);
            gitter->addWidget(m_settings, reihe, 0);
            gitter->addWidget(ersetzt, reihe, 1, Qt::AlignRight);
            connect(m_settings, &QCheckBox::toggled, this, [this] { aktualisiere(); });
        }
        aussen->addLayout(gitter);
        aussen->addWidget(strich(this));

        // --- die Regel fuer gleiche Namen (D7 a)
        auto* frage = new QLabel(tr("If a name already exists"), this);
        aussen->addWidget(frage);
        auto* gruppe = new QButtonGroup(this);
        const auto regel = [&](ds::Regel r, const QString& text, const char* name) {
            auto* knopf = new QRadioButton(text, this);
            knopf->setObjectName(QLatin1String(name));
            gruppe->addButton(knopf);
            aussen->addWidget(knopf);
            m_regeln[r] = knopf;
        };
        regel(ds::Regel::MeineBehalten, tr("Keep mine, skip the imported one"), "dataImportKeepMine");
        regel(ds::Regel::Ersetzen, tr("Replace mine"), "dataImportReplace");
        regel(ds::Regel::BeideBehalten, tr("Keep both, add a number to the imported name"), "dataImportKeepBoth");
        regel(ds::Regel::JeDatei, tr("Let me decide for each file"), "dataImportDecide");
        m_regeln[ds::Regel::MeineBehalten]->setChecked(true);
        aussen->addWidget(strich(this));
    }

    auto* fuss = new QHBoxLayout;
    fuss->addWidget(m_hinweis, 1);
    fuss->addWidget(abbrechen);
    fuss->addWidget(m_import);
    aussen->addLayout(fuss);

    aktualisiere();
}

void DatenImportDialog::setzeAn(const QString& id, bool an)
{
    if (id == kSettingsId)
    {
        if (m_settings != nullptr) m_settings->setChecked(an);
    }
    else if (const auto treffer = m_haken.find(id); treffer != m_haken.end())
        treffer->second->setChecked(an);
}

bool DatenImportDialog::istAn(const QString& id) const
{
    if (id == kSettingsId) return m_settings != nullptr && m_settings->isChecked();
    const auto treffer = m_haken.find(id);
    return treffer != m_haken.end() && treffer->second->isChecked();
}

void DatenImportDialog::setzeRegel(ds::Regel regel)
{
    if (const auto treffer = m_regeln.find(regel); treffer != m_regeln.end()) treffer->second->setChecked(true);
}

ds::Regel DatenImportDialog::regel() const
{
    for (const auto& [regel, knopf] : m_regeln)
        if (knopf->isChecked()) return regel;
    return ds::Regel::MeineBehalten;
}

int DatenImportDialog::vergeben() const
{
    int zahl = 0;
    for (const ds::ImportBereich& bereich : m_inhalt.bereiche)
        if (istAn(bereich.id)) zahl += bereich.vergeben();
    return zahl;
}

void DatenImportDialog::aktualisiere()
{
    if (!m_inhalt.ok)
    {
        m_hinweis->clear();
        return;
    }
    const int zahl = vergeben();
    m_hinweis->setText(zahl > 0 ? tr("%1 name(s) already exist").arg(zahl) : tr("No name exists yet"));
    // ohne gleichen Namen gibt es nichts zu regeln
    for (const auto& [regel, knopf] : m_regeln) knopf->setEnabled(zahl > 0);
    bool etwas = istAn(kSettingsId);
    for (const auto& [id, haken] : m_haken) etwas = etwas || haken->isChecked();
    m_import->setEnabled(etwas);
}

ds::ImportErgebnis DatenImportDialog::fuehreAus()
{
    m_einstellungenGesetzt = 0;
    ds::ImportWahl wahl;
    for (const ds::ImportBereich& bereich : m_inhalt.bereiche)
        if (istAn(bereich.id)) wahl.bereiche << bereich.id;
    wahl.regel = regel();
    wahl.jeDatei = m_jeDatei;
    ds::ImportErgebnis ergebnis = ds::importiere(m_datei, m_ordner, wahl);
    if (ergebnis.ok && istAn(kSettingsId))
    {
        QSettings eigene;
        QSettings& s = m_einstellungen != nullptr ? *m_einstellungen : eigene;
        m_einstellungenGesetzt = ds::schreibeEinstellungen(s, m_inhalt.einstellungen);
    }
    return ergebnis;
}

void DatenImportDialog::importiere()
{
    if (regel() == ds::Regel::JeDatei && vergeben() > 0)
    {
        std::vector<ds::ImportBereich> gewaehlt;
        for (const ds::ImportBereich& bereich : m_inhalt.bereiche)
            if (istAn(bereich.id)) gewaehlt.push_back(bereich);
        DatenJeDateiDialog liste(gewaehlt, this);
        if (liste.exec() != QDialog::Accepted) return;  // Back: hier weiter
        m_jeDatei = liste.wahl();
    }
    const ds::ImportErgebnis ergebnis = fuehreAus();
    if (!ergebnis.ok)
    {
        QMessageBox::warning(this, windowTitle(), tr("The import failed.\n%1").arg(ergebnis.fehler));
        return;
    }
    QStringList zeilen;
    zeilen << tr("%1 new file(s)").arg(ergebnis.eingelesen);
    if (ergebnis.ersetzt > 0) zeilen << tr("%1 of yours replaced").arg(ergebnis.ersetzt);
    if (ergebnis.umbenannt > 0) zeilen << tr("%1 added under a new name").arg(ergebnis.umbenannt);
    if (ergebnis.uebersprungen > 0) zeilen << tr("%1 skipped — the name exists").arg(ergebnis.uebersprungen);
    if (ergebnis.gleich > 0) zeilen << tr("%1 already there, identical").arg(ergebnis.gleich);
    if (m_einstellungenGesetzt > 0) zeilen << tr("%1 setting(s) replaced").arg(m_einstellungenGesetzt);
    QMessageBox::information(
        this, windowTitle(),
        tr("Import finished.\n\n%1\n\nA list that is open right now shows new entries the next time it is "
           "filled; settings apply after a restart.")
            .arg(zeilen.join(QLatin1Char('\n'))));
    accept();
}

// =============================================================================
// Aus dem Settings-Panel
// =============================================================================

void zeigeDatenExport(QWidget* parent)
{
    const QString ordner = ds::benutzerOrdner();
    if (ordner.isEmpty())
    {
        QMessageBox::warning(parent, QObject::tr("User Data"), QObject::tr("Qt reports no writable data location."));
        return;
    }
    DatenExportDialog dialog(ordner, parent);
    dialog.exec();
}

void zeigeDatenImport(QWidget* parent, IEventBus* bus, int rate)
{
    const QString ordner = ds::benutzerOrdner();
    if (ordner.isEmpty())
    {
        QMessageBox::warning(parent, QObject::tr("User Data"), QObject::tr("Qt reports no writable data location."));
        return;
    }
    QSettings einstellungen;
    const QString datei =
        QFileDialog::getOpenFileName(parent, QObject::tr("Import data"), startOrdner(einstellungen),
                                     QObject::tr("LumiViz data (*%1);;All files (*)").arg(QLatin1String(ds::kEndung)));
    if (datei.isEmpty()) return;
    einstellungen.setValue(QLatin1String(kLetzterOrdner), QFileInfo(datei).absolutePath());
    if (lumi::paket::istPaket(datei))
    {
        // ein Paket aus Publish: der Dialog Import package (Publish_Konzept.md P2 a, P8 a)
        zeigeImportPaket(parent, datei, bus, rate);
        return;
    }
    DatenImportDialog dialog(datei, ordner, parent);
    if (!dialog.inhalt().ok)
    {
        QMessageBox::warning(parent, QObject::tr("Import data"),
                             QObject::tr("%1 cannot be imported.\n%2")
                                 .arg(QDir::toNativeSeparators(datei), dialog.inhalt().fehler));
        return;
    }
    // offene Listen lesen neu, Zwischenspeicher fallen weg (S125)
    if (dialog.exec() == QDialog::Accepted && bus != nullptr) bus->publish(UserDataImportedEvent{});
}

}  // namespace lumi::ui
