/**
 ****************************************************************************************
 * @file   PaketDialoge.cpp
 * @brief  Publish, Stufe P-A: die Dialoge Publish und Import package
 *         (docs/core-services/Publish_Konzept.md §4b–§4d)
 ****************************************************************************************
 */

#include "UI/dialogs/PaketDialoge.hpp"

#include "AppInfo.hpp"
#include "UI/dialogs/DatensicherungDialoge.hpp"
#include "UI/widgets/Bedienhilfe.hpp"
#include "UI/widgets/PresetTypeIcons.hpp"
#include "audio/IAudioEngine.hpp"
#include "loop/DrumKit.hpp"
#include "loop/Freeze.hpp"
#include "loop/FreezeAuftrag.hpp"
#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"
#include "loop/SignalWeg.hpp"
#include "loop/Sample.hpp"
#include "services/IEventBus.hpp"
#include "services/events/UIEvents.hpp"
#include "shape/Kontur.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QSettings>
#include <QStandardPaths>
#include <QTimer>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace lumi::ui
{

namespace pk = lumi::paket;
namespace ds = lumi::core::datensicherung;
namespace lp = lumi::loop;

namespace
{

const char* const kLetzterPublishOrdner = "data/lastPublishDir";  // ein Pfad: geht nie in einen Export
const char* const kLetzterImportOrdner = "data/lastPackageDir";
const char* const kLetzterQuellOrdner = "data/lastPublishSourceDir";
const char* const kFrozenMitnehmen = "data/publishFrozen";  // der Haken Include frozen clips (FP2 a)

QString startOrdner(QSettings& einstellungen, const char* schluessel)
{
    const QString gemerkt = einstellungen.value(QLatin1String(schluessel)).toString();
    if (!gemerkt.isEmpty() && QFileInfo(gemerkt).isDir()) return gemerkt;
    const QString dokumente = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    return dokumente.isEmpty() ? QDir::homePath() : dokumente;
}

QString grossAnfang(const QString& wort)
{
    return wort.isEmpty() ? wort : wort.left(1).toUpper() + wort.mid(1);
}

/// „1 file", „2 files" — `tr()` mit `%n` setzt ohne Uebersetzung keine Mehrzahl ein.
QString zahlWort(int zahl, const QString& eins, const QString& mehr)
{
    return QStringLiteral("%1 %2").arg(zahl).arg(zahl == 1 ? eins : mehr);
}

QString gruppenTitel(pk::Art art)
{
    switch (art)
    {
        case pk::Art::Instrument: return QObject::tr("Instruments");
        case pk::Art::Kit: return QObject::tr("Kits");
        case pk::Art::Kontur: return QObject::tr("Shapes");
        case pk::Art::Sample: return QObject::tr("Samples");
        case pk::Art::Akkord: return QObject::tr("Chords");
        case pk::Art::Loop: return QObject::tr("Loops");
    }
    return {};
}

QString endungDer(pk::Art art, const QString& datei)
{
    return art == pk::Art::Sample ? QFileInfo(datei).suffix().toLower() : pk::artText(art);
}

QColor hinweisFarbe(const QWidget* w)
{
    return lp::SignalWeg::abweichFarbe(w->palette());  // das Bernstein der Hinweise, wie im Drum-Editor (K11 a)
}

QColor grau(const QWidget* w)
{
    return w->palette().color(QPalette::Disabled, QPalette::Text);
}

/// Ein kleines Bild einer Zeichnung: ihre Konturen, auf `kante` Pixel eingepasst.
QPixmap formBild(const lumi::shape::Zeichnung& zeichnung, int kante, const QColor& farbe)
{
    QPixmap bild(kante, kante);
    bild.fill(Qt::transparent);
    QPainterPath alle;
    for (const lumi::shape::Kontur& k : zeichnung.konturen) alle.addPath(k.pfad);
    const QRectF rahmen = alle.boundingRect();
    if (rahmen.isEmpty()) return bild;
    QPainter p(&bild);
    p.setRenderHint(QPainter::Antialiasing);
    const double mass = (kante - 4) / std::max(rahmen.width(), rahmen.height());
    p.translate(kante / 2.0, kante / 2.0);
    p.scale(mass, mass);
    p.translate(-rahmen.center());
    p.setPen(QPen(farbe, 1.5 / mass));
    p.drawPath(alle);
    return bild;
}

/// Wie viele Dinge beim Empfaenger eine eigene Datei nennen — fuer „used by".
int nutzerZahl(pk::Art art, const QString& name)
{
    const QString nennung = QStringLiteral("user:") + name;
    switch (art)
    {
        case pk::Art::Sample: return static_cast<int>(lp::eigeneInstrumenteMitSample(nennung).size());
        case pk::Art::Kontur: return static_cast<int>(lp::eigeneInstrumenteMitForm(nennung).size());
        case pk::Art::Kit: return static_cast<int>(lp::loopsMitKit(nennung, lp::zuletztGeoeffneteLoops()).size());
        case pk::Art::Instrument:
        {
            int zahl = 0;
            for (const QString& datei : lp::zuletztGeoeffneteLoops())
            {
                lp::Loop loop;
                if (!lp::ladeLoop(datei, loop)) continue;
                const bool nennt = std::any_of(loop.muster.begin(), loop.muster.end(), [&nennung](const lp::Muster& m) {
                    return m.instrument == nennung ||
                           std::any_of(m.spuren.begin(), m.spuren.end(),
                                       [&nennung](const lp::Spur& s) { return s.instrument == nennung; });
                });
                zahl += nennt ? 1 : 0;
            }
            return zahl;
        }
        case pk::Art::Akkord:  // ein Akkord setzt nur Noten — niemand nennt ihn (S-T1 a)
        case pk::Art::Loop: break;
    }
    return 0;
}

}  // namespace

QString paketArtWort(pk::Art art)
{
    switch (art)
    {
        case pk::Art::Instrument: return QObject::tr("instrument");
        case pk::Art::Kit: return QObject::tr("kit");
        case pk::Art::Loop: return QObject::tr("loop");
        case pk::Art::Kontur: return QObject::tr("shape");
        case pk::Art::Sample: return QObject::tr("sample");
        case pk::Art::Akkord: return QObject::tr("chord");
    }
    return {};
}

FreezeTon freezeTonVon(IAudioEngine* engine)
{
    FreezeTon ton;
    ton.rate = engine != nullptr && engine->getSampleRate() > 0 ? engine->getSampleRate() : 44100;
    // dekodiert wird ueber die Engine (TV3 a); ohne Engine bleiben Samples still
    ton.dekodierer = [engine](const QString& datei, int rate, double maxSekunden) {
        return engine != nullptr ? engine->decodeStereo(datei, rate, maxSekunden) : std::vector<float>{};
    };
    return ton;
}

// =============================================================================
// Publish <art>
// =============================================================================

PublishDialog::PublishDialog(pk::Fund fund, QWidget* parent, QSettings* einstellungen, FreezeTon ton)
    : QDialog(parent), m_fund(std::move(fund)), m_einstellungen(einstellungen), m_ton(std::move(ton))
{
    setObjectName(QStringLiteral("publishDialog"));
    setWindowTitle(tr("Publish %1").arg(paketArtWort(m_fund.art)));
    auto* aussen = new QVBoxLayout(this);

    // eingefrorene Clips eines Loops (Stufe 30i, FP1 a): der gespeicherte Stand, der jetzige Abdruck
    if (m_fund.art == pk::Art::Loop && m_ton.rate > 0)
    {
        lp::Loop loop;
        if (lp::ladeLoop(m_fund.hauptdatei, loop))
        {
            loop.richte();
            const lp::InstrumentVon saiten = lp::instrumentAuskunft();
            m_teile = pk::frozenTeile(loop, m_ton.rate, saiten);
            for (const pk::FrozenTeil& teil : m_teile)
            {
                const std::optional<double> ms =
                    teil.datei.isEmpty() ? lp::clipRechenzeitMs(loop, teil.ort, m_ton.rate, saiten) : std::nullopt;
                m_rechenzeit.push_back(ms ? QStringLiteral("%1 s").arg(*ms / 1000.0, 0, 'f', 1) : QString());
            }
            if (!m_teile.empty()) m_loop = std::move(loop);
        }
    }

    auto* nameZeile = new QHBoxLayout;
    nameZeile->addWidget(new QLabel(tr("Package"), this));
    m_name = new QLineEdit(pk::paketName(m_fund), this);
    m_name->setObjectName(QStringLiteral("publishName"));
    nameZeile->addWidget(m_name, 1);
    aussen->addLayout(nameZeile);

    m_baum = new QTreeWidget(this);
    m_baum->setObjectName(QStringLiteral("publishContents"));
    m_baum->setColumnCount(2);
    m_baum->setHeaderHidden(true);
    m_baum->setRootIsDecorated(true);
    m_baum->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_baum->header()->setStretchLastSection(true);
    m_baum->setTextElideMode(Qt::ElideMiddle);  // ein langer Pfad: Anfang und Dateiname bleiben
    aussen->addWidget(m_baum, 1);
    baueBaum();

    auto* medienZeile = new QHBoxLayout;
    m_medien = new QCheckBox(tr("Include media files"), this);
    m_medien->setObjectName(QStringLiteral("publishMedia"));
    m_medien->setEnabled(false);  // Songs und Videos kommen mit Komposition und Effect chain (Stufe P-B)
    medienZeile->addWidget(m_medien);
    auto* keine = new QLabel(tr("— none in this %1").arg(paketArtWort(m_fund.art)), this);
    keine->setEnabled(false);
    medienZeile->addWidget(keine);
    medienZeile->addStretch(1);
    aussen->addLayout(medienZeile);

    QSettings appEinstellungen;
    QSettings& e = m_einstellungen != nullptr ? *m_einstellungen : appEinstellungen;
    if (!m_teile.empty())
    {
        auto* frozenZeile = new QHBoxLayout;
        m_frozenHaken = new QCheckBox(tr("Include frozen clips"), this);
        m_frozenHaken->setObjectName(QStringLiteral("publishFrozen"));
        m_frozenHaken->setChecked(e.value(QLatin1String(kFrozenMitnehmen), true).toBool());
        m_frozenHaken->setToolTip(tr("The waveforms of the frozen clips go along: the receiver plays them at once "
                                     "instead of computing them. They make the package larger."));
        frozenZeile->addWidget(m_frozenHaken);
        auto* wozu = new QLabel(tr("— the receiver plays them at once"), this);
        wozu->setEnabled(false);
        frozenZeile->addWidget(wozu);
        frozenZeile->addStretch(1);
        aussen->addLayout(frozenZeile);
        connect(m_frozenHaken, &QCheckBox::toggled, this, [this](bool an) {
            QSettings appEinstellungen;
            QSettings& e = m_einstellungen != nullptr ? *m_einstellungen : appEinstellungen;
            e.setValue(QLatin1String(kFrozenMitnehmen), an);
            aktualisiereSumme();
        });
    }
    auto* zielZeile = new QHBoxLayout;
    zielZeile->addWidget(new QLabel(tr("Save to"), this));
    m_ziel = new QLineEdit(QDir::toNativeSeparators(startOrdner(e, kLetzterPublishOrdner)), this);
    m_ziel->setObjectName(QStringLiteral("publishTarget"));
    zielZeile->addWidget(m_ziel, 1);
    auto* waehlen = new QPushButton(tr("Browse..."), this);
    waehlen->setObjectName(QStringLiteral("publishBrowse"));
    connect(waehlen, &QPushButton::clicked, this, [this] {
        const QString ordner = QFileDialog::getExistingDirectory(this, tr("Save to"), zielOrdner());
        if (!ordner.isEmpty()) setzeZielOrdner(ordner);
    });
    zielZeile->addWidget(waehlen);
    aussen->addLayout(zielZeile);

    auto* fuss = new QHBoxLayout;
    m_summe = new QLabel(this);
    m_summe->setObjectName(QStringLiteral("publishSum"));
    fuss->addWidget(m_summe, 1);
    m_publish = new QPushButton(tr("Publish"), this);
    m_publish->setObjectName(QStringLiteral("publishGo"));
    m_publish->setDefault(true);
    connect(m_publish, &QPushButton::clicked, this, &PublishDialog::veroeffentliche);
    auto* abbrechen = new QPushButton(tr("Cancel"), this);
    connect(abbrechen, &QPushButton::clicked, this, &QDialog::reject);
    fuss->addWidget(m_publish);
    fuss->addWidget(abbrechen);
    aussen->addLayout(fuss);
    aktualisiereSumme();
    resize(600, 580);

    // fehlende Wellenformen rechnen ab dem Oeffnen (FP2 a); Cancel bricht ab (der Auftrag
    // wartet im Destruktor), Gerechnetes bleibt im Ordner `frozen`
    if (m_loop)
    {
        for (const pk::FrozenTeil& teil : m_teile)
            if (teil.datei.isEmpty()) m_freezeOrte.push_back(teil.ort);
        if (!m_freezeOrte.empty())
        {
            m_freeze = std::make_unique<lp::FreezeAuftrag>(*m_loop, m_freezeOrte, m_ton.rate, m_ton.dekodierer);
            m_uhr = new QTimer(this);
            m_uhr->setInterval(100);
            connect(m_uhr, &QTimer::timeout, this, &PublishDialog::pruefeFreeze);
            m_uhr->start();
        }
        fuelleFrozenZeilen();
    }
}

PublishDialog::~PublishDialog() = default;

bool PublishDialog::frozenMitnehmen() const
{
    return m_frozenHaken != nullptr && m_frozenHaken->isChecked();
}

void PublishDialog::fuelleFrozenZeilen()
{
    if (m_frozenGruppe == nullptr) return;
    const lp::FreezeStand stand = m_freeze != nullptr ? m_freeze->stand() : lp::FreezeStand{};
    for (std::size_t i = 0; i < m_teile.size() && static_cast<int>(i) < m_frozenGruppe->childCount(); ++i)
    {
        const pk::FrozenTeil& teil = m_teile[i];
        QTreeWidgetItem* zeile = m_frozenGruppe->child(static_cast<int>(i));
        QString text;
        if (!teil.datei.isEmpty())
            text = tr("ready · %1").arg(groesseAlsText(teil.groesse));
        else if (m_freeze != nullptr)
        {
            const auto platz = std::find(m_freezeOrte.begin(), m_freezeOrte.end(), teil.ort);
            const int nr = static_cast<int>(platz - m_freezeOrte.begin());
            if (nr == stand.clip)
                text = tr("computing… %1 %").arg(static_cast<int>(stand.anteil * 100.0));
            else
                text = m_rechenzeit[i].isEmpty() ? tr("to compute") : tr("to compute · %1").arg(m_rechenzeit[i]);
        }
        else
            text = tr("could not compute — the receiver computes it");
        zeile->setText(1, text);
        zeile->setForeground(1, teil.datei.isEmpty() && m_freeze == nullptr ? hinweisFarbe(this) : grau(this));
    }
}

void PublishDialog::aktualisiereSumme()
{
    if (m_wartet && m_freeze != nullptr)
    {
        // Publish wartet: im Fuss statt der Summe der Stand (FP2 a)
        const lp::FreezeStand stand = m_freeze->stand();
        m_summe->setText(tr("Freezing clip %1 of %2 … %3 %")
                             .arg(std::min(stand.clip + 1, std::max(stand.clips, 1)))
                             .arg(std::max(stand.clips, 1))
                             .arg(static_cast<int>(stand.anteil * 100.0)));
        return;
    }
    int zahl = 1 + static_cast<int>(m_fund.zugehoerige.size());
    qint64 groesse = QFileInfo(m_fund.hauptdatei).size();
    for (const pk::Zugehoerige& z : m_fund.zugehoerige) groesse += QFileInfo(z.datei).size();
    if (frozenMitnehmen())
        for (const pk::FrozenTeil& teil : m_teile)
        {
            ++zahl;
            groesse += teil.groesse;
        }
    m_summe->setText(QStringLiteral("%1, %2").arg(zahlWort(zahl, tr("file"), tr("files")), groesseAlsText(groesse)));
}

void PublishDialog::pruefeFreeze()
{
    if (m_freeze == nullptr) return;
    if (!m_freeze->beendet())
    {
        fuelleFrozenZeilen();
        aktualisiereSumme();
        return;
    }
    m_freeze.reset();
    if (m_uhr != nullptr) m_uhr->stop();
    // jetzt liegen die Dateien: neu nachsehen (was misslang, rechnet der Empfaenger)
    m_teile = pk::frozenTeile(*m_loop, m_ton.rate, lp::instrumentAuskunft());
    fuelleFrozenZeilen();
    aktualisiereSumme();
    if (m_wartet)
    {
        m_wartet = false;
        m_publish->setEnabled(true);
        veroeffentliche();
    }
}

void PublishDialog::warteAufFreeze()
{
    if (m_freeze == nullptr) return;
    m_freeze->warte();
    pruefeFreeze();
}

void PublishDialog::baueBaum()
{
    const QString artWort = paketArtWort(m_fund.art);
    auto* haupt = new QTreeWidgetItem(m_baum, {m_fund.titel, tr("%1 · main file").arg(grossAnfang(artWort))});
    haupt->setIcon(0, dateiIconFuerEndung(endungDer(m_fund.art, m_fund.hauptdatei)));
    const QString hauptOrdner = QFileInfo(m_fund.hauptdatei).absolutePath();

    for (const pk::Art art : {pk::Art::Instrument, pk::Art::Kit, pk::Art::Kontur, pk::Art::Sample})
    {
        std::vector<const pk::Zugehoerige*> dieser;
        for (const pk::Zugehoerige& z : m_fund.zugehoerige)
            if (z.art == art) dieser.push_back(&z);
        if (dieser.empty()) continue;
        auto* gruppe = new QTreeWidgetItem(haupt, {QStringLiteral("%1 (%2)").arg(gruppenTitel(art)).arg(dieser.size())});
        for (const pk::Zugehoerige* z : dieser)
        {
            QString herkunft;
            const QString neu = QStringLiteral("user:") + z->name;
            if (z->herkunft == pk::Herkunft::Eigen)
                herkunft = tr("yours · %1").arg(neu);
            else if (QFileInfo(z->datei).absolutePath() == hauptOrdner)
                herkunft = tr("next to the %1 → %2").arg(artWort, neu);
            else
                herkunft = QStringLiteral("%1 → %2").arg(QDir::toNativeSeparators(z->datei), neu);
            auto* zeile = new QTreeWidgetItem(gruppe, {z->name, herkunft});
            zeile->setIcon(0, dateiIconFuerEndung(endungDer(art, z->datei)));
            zeile->setForeground(1, grau(this));
            zeile->setToolTip(1, QDir::toNativeSeparators(z->datei));
            zeile->setToolTip(0, tr("Named by: %1").arg(z->genanntVon.join(QStringLiteral(", "))));
        }
        gruppe->setExpanded(true);
    }
    // eingefrorene Clips (FP2 a): je Clip eine Zeile, den Stand fuellt `fuelleFrozenZeilen`
    if (!m_teile.empty())
    {
        m_frozenGruppe = new QTreeWidgetItem(haupt, {tr("Frozen clips (%1)").arg(m_teile.size())});
        m_frozenGruppe->setToolTip(0, tr("Waveforms of the frozen clips — the receiver plays them at once"));
        for (const pk::FrozenTeil& teil : m_teile)
        {
            auto* zeile = new QTreeWidgetItem(m_frozenGruppe, {teil.etikett});
            zeile->setIcon(0, dateiIconFuerEndung(QStringLiteral("wav")));
        }
        m_frozenGruppe->setExpanded(true);
    }
    haupt->setExpanded(true);

    if (!m_fund.mitgeliefert.empty())
    {
        auto* mit = new QTreeWidgetItem(
            m_baum, {tr("Bundled, not included (%1)").arg(m_fund.mitgeliefert.size()),
                     tr("every LumiViz has them")});
        for (int spalte = 0; spalte < 2; ++spalte) mit->setForeground(spalte, grau(this));
        for (const pk::Nennung& n : m_fund.mitgeliefert)
        {
            auto* zeile = new QTreeWidgetItem(mit, {n.text, paketArtWort(n.art)});
            for (int spalte = 0; spalte < 2; ++spalte) zeile->setForeground(spalte, grau(this));
        }
        mit->setExpanded(false);
    }
    if (!m_fund.fehlt.empty())
    {
        auto* fehlt = new QTreeWidgetItem(m_baum, {tr("Not found (%1)").arg(m_fund.fehlt.size()),
                                                   tr("the package names them; they can't come along")});
        fehlt->setForeground(0, hinweisFarbe(this));
        fehlt->setIcon(0, style()->standardIcon(QStyle::SP_MessageBoxWarning));
        for (const pk::Fehlend& f : m_fund.fehlt)
        {
            auto* zeile = new QTreeWidgetItem(fehlt, {f.nennung, tr("named by %1").arg(f.genanntVon)});
            zeile->setForeground(0, hinweisFarbe(this));
        }
        fehlt->setExpanded(true);
    }
}

void PublishDialog::setzeName(const QString& dateiname)
{
    m_name->setText(dateiname);
}

void PublishDialog::setzeZielOrdner(const QString& ordner)
{
    m_ziel->setText(QDir::toNativeSeparators(ordner));
}

QString PublishDialog::zielOrdner() const
{
    return QDir::fromNativeSeparators(m_ziel->text().trimmed());
}

QString PublishDialog::paketDatei() const
{
    QString name = m_name->text().trimmed();
    if (name.isEmpty()) name = pk::paketName(m_fund);
    if (!name.endsWith(QLatin1String(ds::kEndung), Qt::CaseInsensitive)) name += QLatin1String(ds::kEndung);
    return QDir(zielOrdner()).filePath(name);
}

QString PublishDialog::summe() const
{
    return m_summe->text();
}

pk::PackErgebnis PublishDialog::fuehreAus()
{
    std::vector<QByteArray> abdruecke;
    if (frozenMitnehmen())
        for (const pk::FrozenTeil& teil : m_teile)
            if (!teil.datei.isEmpty()) abdruecke.push_back(teil.abdruck);
    const pk::PackErgebnis ergebnis = pk::packe(m_fund, paketDatei(), lumi::appVersion(), abdruecke);
    if (ergebnis.ok)
    {
        QSettings appEinstellungen;
        QSettings& e = m_einstellungen != nullptr ? *m_einstellungen : appEinstellungen;
        e.setValue(QLatin1String(kLetzterPublishOrdner), zielOrdner());
    }
    return ergebnis;
}

void PublishDialog::veroeffentliche()
{
    // rechnet noch eine Wellenform, die mitgehen soll, wartet Publish darauf (FP2 a)
    if (frozenMitnehmen() && m_freeze != nullptr)
    {
        m_wartet = true;
        m_publish->setEnabled(false);
        aktualisiereSumme();
        return;
    }
    // die Meldungen ueber `fuehreDialogAus`: ein Test kann sie beantworten
    const QString datei = paketDatei();
    if (QFileInfo::exists(datei))
    {
        QMessageBox frage(QMessageBox::Question, windowTitle(),
                          tr("%1 exists. Replace it?").arg(QDir::toNativeSeparators(datei)),
                          QMessageBox::Yes | QMessageBox::No, this);
        frage.setObjectName(QStringLiteral("publishReplaceQuestion"));
        if (fuehreDialogAus(frage) != QMessageBox::Yes) return;
    }
    const pk::PackErgebnis ergebnis = fuehreAus();
    if (!ergebnis.ok)
    {
        QMessageBox warnung(QMessageBox::Warning, windowTitle(), tr("Could not publish.\n%1").arg(ergebnis.fehler),
                            QMessageBox::Ok, this);
        warnung.setObjectName(QStringLiteral("publishFailed"));
        (void)fuehreDialogAus(warnung);
        return;
    }
    QMessageBox fertig(QMessageBox::Information, windowTitle(),
                       tr("Published %1 — %2.").arg(QDir::toNativeSeparators(datei),
                                                    zahlWort(ergebnis.dateien, tr("file"), tr("files"))),
                       QMessageBox::Ok, this);
    fertig.setObjectName(QStringLiteral("publishDone"));
    (void)fuehreDialogAus(fertig);
    accept();
}

// =============================================================================
// Import package
// =============================================================================

ImportPaketDialog::ImportPaketDialog(const QString& datei, QWidget* parent, QSettings* einstellungen, int rate)
    : QDialog(parent), m_datei(datei), m_einstellungen(einstellungen), m_rate(rate), m_inhalt(pk::oeffnePaket(datei))
{
    setObjectName(QStringLiteral("importPackageDialog"));
    setWindowTitle(tr("Import package"));
    auto* aussen = new QVBoxLayout(this);
    baueKopf(aussen, datei);

    auto* fuss = new QHBoxLayout;
    if (m_inhalt.ok)
    {
        // --- Contents
        auto* titel = new QLabel(tr("Contents"), this);
        QFont fett = titel->font();
        fett.setBold(true);
        titel->setFont(fett);
        aussen->addWidget(titel);
        m_inhaltListe = new QTreeWidget(this);
        m_inhaltListe->setObjectName(QStringLiteral("importPackageContents"));
        m_inhaltListe->setColumnCount(2);
        m_inhaltListe->setHeaderHidden(true);
        m_inhaltListe->setRootIsDecorated(false);
        m_inhaltListe->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        m_inhaltListe->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
        m_inhaltListe->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        if (m_inhalt.brauchtZiel())
            new QTreeWidgetItem(m_inhaltListe, {m_inhalt.titel, tr("%1 → Save to (below)").arg(grossAnfang(paketArtWort(m_inhalt.art)))});
        for (const pk::PaketDatei& d : m_inhalt.dateien)
        {
            QString wohin = QStringLiteral("→ %1, ").arg(gruppenTitel(d.art));
            switch (d.zustand)
            {
                case ds::Zustand::Neu: wohin += tr("new"); break;
                case ds::Zustand::Gleich: wohin += tr("already here"); break;
                case ds::Zustand::NameVergeben: wohin += tr("same name (see below)"); break;
            }
            auto* zeile = new QTreeWidgetItem(m_inhaltListe, {d.name, wohin});
            zeile->setIcon(0, dateiIconFuerEndung(endungDer(d.art, d.pfad)));
            zeile->setForeground(1, grau(this));
        }
        if (m_inhalt.frozen > 0)
        {
            // die Wellenformen eingefrorener Clips (FP3 a): der Import prueft sie
            auto* zeile = new QTreeWidgetItem(m_inhaltListe, {tr("Frozen clips (%1)").arg(m_inhalt.frozen),
                                                              tr("→ frozen clips, %1").arg(groesseAlsText(m_inhalt.frozenGroesse))});
            zeile->setIcon(0, dateiIconFuerEndung(QStringLiteral("wav")));
            zeile->setForeground(1, grau(this));
        }
        aussen->addWidget(m_inhaltListe);

        // --- Same names
        if (m_inhalt.vergeben() > 0) baueGleiche();
        if (m_gleiche != nullptr)
        {
            auto* t = new QLabel(tr("Same names (%1)").arg(m_inhalt.vergeben()), this);
            t->setFont(fett);
            aussen->addWidget(t);
            aussen->addWidget(m_gleiche);
            auto* unter = new QHBoxLayout;
            m_summe = new QLabel(this);
            m_summe->setObjectName(QStringLiteral("importPackageSum"));
            m_summe->setWordWrap(true);
            unter->addWidget(m_summe, 1);
            m_alle = new QComboBox(this);
            m_alle->setObjectName(QStringLiteral("importPackageAll"));
            m_alle->addItem(tr("All: Keep both"), static_cast<int>(ds::Wahl::Beide));
            m_alle->addItem(tr("All: Replace mine"), static_cast<int>(ds::Wahl::Importierte));
            m_alle->addItem(tr("All: Use mine"), static_cast<int>(ds::Wahl::Meine));
            connect(m_alle, &QComboBox::activated, this,
                    [this](int platz) { setzeAlle(static_cast<ds::Wahl>(m_alle->itemData(platz).toInt())); });
            unter->addWidget(m_alle);
            aussen->addLayout(unter);
            aktualisiereSumme();
        }

        // --- Notes
        for (const pk::Fehlend& f : m_inhalt.fehlt)
            m_hinweise << tr("Not in the package (the sender didn't have it): %1 — named by %2").arg(f.nennung, f.genanntVon);
        QStringList fehltHier;
        for (const pk::Nennung& n : m_inhalt.mitgeliefertFehltHier) fehltHier << n.text;
        if (!fehltHier.isEmpty())
            m_hinweise << tr("Not in this LumiViz: %1 — the package needs a newer LumiViz for them")
                              .arg(fehltHier.join(QStringLiteral(", ")));
        const int warnungen = static_cast<int>(m_hinweise.size());
        if (m_inhalt.frozen > 0) m_hinweise << tr("Frozen clips: the receiver checks them on import");
        if (!m_inhalt.mitgeliefert.empty() && fehltHier.isEmpty())
        {
            QStringList namen;
            for (const pk::Nennung& n : m_inhalt.mitgeliefert) namen << n.text;
            QString liste = namen.mid(0, 6).join(QStringLiteral(", "));
            if (namen.size() > 6) liste += QStringLiteral(" …");
            m_hinweise << tr("Bundled with LumiViz, all present: %1").arg(liste);
        }
        if (!m_hinweise.isEmpty())
        {
            auto* t = new QLabel(tr("Notes"), this);
            t->setFont(fett);
            aussen->addWidget(t);
            for (int i = 0; i < m_hinweise.size(); ++i)
            {
                auto* zeile = new QLabel(m_hinweise[i], this);
                zeile->setWordWrap(true);
                QPalette farben = zeile->palette();
                farben.setColor(QPalette::WindowText, i < warnungen ? hinweisFarbe(this) : grau(this));
                zeile->setPalette(farben);
                aussen->addWidget(zeile);
            }
        }

        // --- Save to
        QSettings appEinstellungen;
        QSettings& e = m_einstellungen != nullptr ? *m_einstellungen : appEinstellungen;
        if (m_inhalt.brauchtZiel())
        {
            auto* zielZeile = new QHBoxLayout;
            zielZeile->addWidget(new QLabel(tr("Save to"), this));
            m_ziel = new QLineEdit(
                QDir::toNativeSeparators(QDir(startOrdner(e, kLetzterImportOrdner)).filePath(m_inhalt.dateiname())), this);
            m_ziel->setObjectName(QStringLiteral("importPackageTarget"));
            zielZeile->addWidget(m_ziel, 1);
            auto* waehlen = new QPushButton(tr("Browse..."), this);
            connect(waehlen, &QPushButton::clicked, this, [this] {
                const QString pfad = QFileDialog::getSaveFileName(
                    this, tr("Save to"), ziel(), tr("LumiViz loop (*.%1)").arg(pk::artText(m_inhalt.art)));
                if (!pfad.isEmpty()) setzeZiel(pfad);
            });
            zielZeile->addWidget(waehlen);
            aussen->addLayout(zielZeile);
        }

        // nur ein Loop wird geoeffnet; Instrumente, Kits, Formen und Samples stehen danach in ihren Listen
        if (m_inhalt.brauchtZiel())
        {
            m_oeffnen = new QCheckBox(tr("Open the %1 after import").arg(paketArtWort(m_inhalt.art)), this);
            m_oeffnen->setObjectName(QStringLiteral("importPackageOpen"));
            m_oeffnen->setChecked(true);
            fuss->addWidget(m_oeffnen);
        }
    }
    fuss->addStretch(1);
    m_import = new QPushButton(tr("Import"), this);
    m_import->setObjectName(QStringLiteral("importPackageGo"));
    m_import->setDefault(true);
    m_import->setEnabled(m_inhalt.ok);
    connect(m_import, &QPushButton::clicked, this, &ImportPaketDialog::importiere);
    auto* abbrechen = new QPushButton(tr("Cancel"), this);
    connect(abbrechen, &QPushButton::clicked, this, &QDialog::reject);
    fuss->addWidget(m_import);
    fuss->addWidget(abbrechen);
    aussen->addLayout(fuss);
    resize(660, sizeHint().height());
}

void ImportPaketDialog::baueKopf(QVBoxLayout* aussen, const QString& datei)
{
    auto* kopf = new QHBoxLayout;
    auto* icon = new QLabel(this);
    const QIcon marke = dateiIconFuerEndung(m_inhalt.ok ? pk::artText(m_inhalt.art) : QStringLiteral("lvdata"));
    if (!marke.isNull()) icon->setPixmap(marke.pixmap(32, 32));
    kopf->addWidget(icon);
    auto* texte = new QVBoxLayout;
    auto* titel = new QLabel(m_inhalt.ok ? tr("Import package — %1").arg(m_inhalt.titel) : QFileInfo(datei).fileName(), this);
    titel->setObjectName(QStringLiteral("importPackageTitle"));
    QFont gross = titel->font();
    gross.setBold(true);
    gross.setPointSizeF(gross.pointSizeF() * 1.15);
    titel->setFont(gross);
    texte->addWidget(titel);
    auto* unter = new QLabel(this);
    unter->setObjectName(QStringLiteral("importPackageInfo"));
    unter->setWordWrap(true);
    if (m_inhalt.ok)
    {
        qint64 groesse = 0;
        for (const pk::PaketDatei& d : m_inhalt.dateien) groesse += d.groesse;
        int zahl = static_cast<int>(m_inhalt.dateien.size()) + m_inhalt.frozen;
        groesse += m_inhalt.frozenGroesse;
        if (m_inhalt.brauchtZiel())
        {
            groesse += pk::paketDaten(datei, m_inhalt.hauptPfad).size();
            ++zahl;
        }
        unter->setText(tr("%1 · published %2 with LumiViz %3 · %4, %5")
                           .arg(grossAnfang(paketArtWort(m_inhalt.art)),
                                m_inhalt.exportiert.date().toString(QStringLiteral("yyyy-MM-dd")),
                                m_inhalt.appVersion.isEmpty() ? QStringLiteral("?") : m_inhalt.appVersion,
                                zahlWort(zahl, tr("file"), tr("files")), groesseAlsText(groesse)));
        unter->setEnabled(false);
    }
    else
    {
        unter->setText(tr("%1 cannot be imported.\n%2").arg(QDir::toNativeSeparators(datei), m_inhalt.fehler));
    }
    texte->addWidget(unter);
    kopf->addLayout(texte, 1);
    aussen->addLayout(kopf);
}

void ImportPaketDialog::baueGleiche()
{
    m_gleiche = new QTreeWidget(this);
    m_gleiche->setObjectName(QStringLiteral("importPackageSameNames"));
    m_gleiche->setColumnCount(4);
    m_gleiche->setHeaderLabels({tr("Name"), tr("Yours"), tr("In the package"), tr("Use")});
    m_gleiche->setRootIsDecorated(false);
    m_gleiche->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_gleiche->header()->setStretchLastSection(true);
    m_gleiche->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_gleiche->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    for (const pk::PaketDatei& d : m_inhalt.dateien)
    {
        if (d.zustand != ds::Zustand::NameVergeben) continue;
        auto* zeile = new QTreeWidgetItem(m_gleiche, {d.name + QLatin1Char('\n') + paketArtWort(d.art)});
        zeile->setIcon(0, dateiIconFuerEndung(endungDer(d.art, d.pfad)));

        const auto seite = [this, &d](bool eigene) {
            auto* feld = new QWidget(m_gleiche);
            auto* reihe = new QHBoxLayout(feld);
            reihe->setContentsMargins(2, 2, 2, 2);
            if (d.art == pk::Art::Kontur)
            {
                lumi::shape::Zeichnung z;
                const bool da = eigene ? lumi::shape::holeZeichnung(QStringLiteral("user:") + d.name, z)
                                       : lumi::shape::ausJson(QJsonDocument::fromJson(pk::paketDaten(m_datei, d.pfad)).object(), z);
                auto* bild = new QLabel(feld);
                if (da) bild->setPixmap(formBild(z, 36, palette().color(QPalette::Text)));
                reihe->addWidget(bild);
            }
            else if (d.art == pk::Art::Sample || d.art == pk::Art::Instrument)
            {
                auto* spiel = new QToolButton(feld);
                spiel->setText(QStringLiteral("▶"));
                spiel->setObjectName(eigene ? QStringLiteral("playMine_") + d.name : QStringLiteral("playPackage_") + d.name);
                spiel->setToolTip(eigene ? tr("Play yours") : tr("Play the one in the package"));
                connect(spiel, &QToolButton::clicked, this, [this, d, eigene] { spieleAn(d, eigene); });
                reihe->addWidget(spiel);
            }
            QString text = eigene ? QStringLiteral("%1, %2").arg(d.eigeneGeaendert.date().toString(QStringLiteral("yyyy-MM-dd")),
                                                                  groesseAlsText(d.eigeneGroesse))
                                  : QStringLiteral("%1").arg(groesseAlsText(d.groesse));
            if (eigene)
            {
                const int nutzer = nutzerZahl(d.art, d.name);
                if (nutzer > 0) text += QLatin1Char('\n') + tr("used by %1").arg(nutzer);
            }
            else if (!d.genanntVon.isEmpty())
            {
                text += QLatin1Char('\n') + tr("used by %1").arg(d.genanntVon.join(QStringLiteral(", ")));
            }
            reihe->addWidget(new QLabel(text, feld), 1);
            return feld;
        };
        m_gleiche->setItemWidget(zeile, 1, seite(true));
        m_gleiche->setItemWidget(zeile, 2, seite(false));

        auto* wahl = new QComboBox(m_gleiche);
        wahl->setObjectName(QStringLiteral("use_") + d.pfad);
        wahl->addItem(tr("Keep both → %1").arg(pk::keepBothName(m_inhalt, d)), static_cast<int>(ds::Wahl::Beide));
        wahl->addItem(tr("Replace mine"), static_cast<int>(ds::Wahl::Importierte));
        wahl->addItem(tr("Use mine"), static_cast<int>(ds::Wahl::Meine));
        wahl->setToolTip(tr("Keep both: the package's file gets a new name, and everything in the package names it. "
                            "Use mine: the package uses yours."));
        connect(wahl, &QComboBox::currentIndexChanged, this, [this] { aktualisiereSumme(); });
        m_gleiche->setItemWidget(zeile, 3, wahl);
        m_wahl[d.pfad] = wahl;
    }
}

void ImportPaketDialog::spieleAn(const pk::PaketDatei& d, bool eigene)
{
    if (eigene)
    {
        emit anspielenVerlangt(static_cast<int>(d.art), d.eigeneDatei);
        return;
    }
    if (!m_vorhoeren) m_vorhoeren = std::make_unique<QTemporaryDir>();
    if (!m_vorhoeren->isValid()) return;
    const QString pfad = QDir(m_vorhoeren->path()).filePath(QFileInfo(d.pfad).fileName());
    QFile datei(pfad);
    if (!datei.open(QIODevice::WriteOnly)) return;
    datei.write(pk::paketDaten(m_datei, d.pfad));
    datei.close();
    emit anspielenVerlangt(static_cast<int>(d.art), pfad);
}

void ImportPaketDialog::aktualisiereSumme()
{
    if (m_summe == nullptr) return;
    int beide = 0, ersetzen = 0, meine = 0;
    for (const auto& [pfad, box] : m_wahl)
    {
        switch (static_cast<ds::Wahl>(box->currentData().toInt()))
        {
            case ds::Wahl::Beide: ++beide; break;
            case ds::Wahl::Importierte: ++ersetzen; break;
            case ds::Wahl::Meine: ++meine; break;
        }
    }
    QStringList teile;
    if (beide > 0)
        teile << tr("Keep both renames %1; the package's %2 will name them")
                     .arg(zahlWort(beide, tr("file"), tr("files")), paketArtWort(m_inhalt.art));
    if (ersetzen > 0) teile << tr("Replace mine overwrites %1 of yours").arg(zahlWort(ersetzen, tr("file"), tr("files")));
    if (meine > 0) teile << tr("Use mine keeps %1 of yours").arg(zahlWort(meine, tr("file"), tr("files")));
    m_summe->setText(teile.join(QStringLiteral(". ")) + QLatin1Char('.'));
}

QStringList ImportPaketDialog::hinweise() const
{
    return m_hinweise;
}

void ImportPaketDialog::setzeWahl(const QString& pfad, ds::Wahl wahl)
{
    const auto it = m_wahl.find(pfad);
    if (it == m_wahl.end()) return;
    it->second->setCurrentIndex(it->second->findData(static_cast<int>(wahl)));
}

ds::Wahl ImportPaketDialog::wahl(const QString& pfad) const
{
    const auto it = m_wahl.find(pfad);
    return it == m_wahl.end() ? ds::Wahl::Beide : static_cast<ds::Wahl>(it->second->currentData().toInt());
}

void ImportPaketDialog::setzeAlle(ds::Wahl wahl)
{
    for (const auto& [pfad, box] : m_wahl) box->setCurrentIndex(box->findData(static_cast<int>(wahl)));
    if (m_alle != nullptr) m_alle->setCurrentIndex(m_alle->findData(static_cast<int>(wahl)));
    aktualisiereSumme();
}

QString ImportPaketDialog::summe() const
{
    return m_summe != nullptr ? m_summe->text() : QString();
}

void ImportPaketDialog::setzeZiel(const QString& pfad)
{
    if (m_ziel != nullptr) m_ziel->setText(QDir::toNativeSeparators(pfad));
}

QString ImportPaketDialog::ziel() const
{
    return m_ziel != nullptr ? QDir::fromNativeSeparators(m_ziel->text().trimmed()) : QString();
}

void ImportPaketDialog::setzeOeffnen(bool an)
{
    if (m_oeffnen != nullptr) m_oeffnen->setChecked(an);
}

bool ImportPaketDialog::oeffnen() const
{
    return m_oeffnen != nullptr && m_oeffnen->isChecked();
}

pk::PaketErgebnis ImportPaketDialog::fuehreAus()
{
    pk::PaketWahl wahl;
    for (const auto& [pfad, box] : m_wahl) wahl.jeDatei[pfad] = static_cast<ds::Wahl>(box->currentData().toInt());
    wahl.ziel = ziel();
    wahl.rate = m_rate;
    const pk::PaketErgebnis ergebnis = pk::importierePaket(m_datei, wahl);
    if (ergebnis.ok && m_inhalt.brauchtZiel())
    {
        QSettings appEinstellungen;
        QSettings& e = m_einstellungen != nullptr ? *m_einstellungen : appEinstellungen;
        e.setValue(QLatin1String(kLetzterImportOrdner), QFileInfo(wahl.ziel).absolutePath());
    }
    return ergebnis;
}

QString ImportPaketDialog::frozenMeldung(const pk::PaketErgebnis& ergebnis)
{
    if (ergebnis.frozenClips <= 0) return {};
    const QString bereit = tr("Frozen clips: %1 of %2 ready to play").arg(ergebnis.frozenBereit).arg(ergebnis.frozenClips);
    const int rechnen = ergebnis.frozenClips - ergebnis.frozenBereit;
    return rechnen > 0 ? bereit + tr(" — %1 will be computed here").arg(rechnen) : bereit;
}

void ImportPaketDialog::importiere()
{
    if (m_inhalt.brauchtZiel() && QFileInfo::exists(ziel()) &&
        QMessageBox::question(this, windowTitle(), tr("%1 exists. Replace it?").arg(QDir::toNativeSeparators(ziel()))) !=
            QMessageBox::Yes)
        return;
    const pk::PaketErgebnis ergebnis = fuehreAus();
    if (!ergebnis.ok)
    {
        QMessageBox::warning(this, windowTitle(), tr("Could not import.\n%1").arg(ergebnis.fehler));
        return;
    }
    QStringList teile;
    if (ergebnis.neu > 0) teile << tr("%1 new").arg(ergebnis.neu);
    if (ergebnis.umbenannt > 0)
        teile << tr("%1 renamed (%2)").arg(ergebnis.umbenannt).arg(ergebnis.umbenennungen.join(QStringLiteral(", ")));
    if (ergebnis.ersetzt > 0) teile << tr("%1 replaced").arg(ergebnis.ersetzt);
    if (ergebnis.eigeneGenommen > 0) teile << tr("%1 of yours used").arg(ergebnis.eigeneGenommen);
    if (ergebnis.gleich > 0) teile << tr("%1 already here").arg(ergebnis.gleich);
    QString meldung = tr("Imported %1: %2.").arg(m_inhalt.titel, teile.join(QStringLiteral(", ")));
    if (const QString frozen = frozenMeldung(ergebnis); !frozen.isEmpty()) meldung += QLatin1Char('\n') + frozen;
    QMessageBox::information(this, windowTitle(), meldung);
    if (oeffnen() && !ergebnis.hauptDatei.isEmpty()) emit oeffnenVerlangt(static_cast<int>(m_inhalt.art), ergebnis.hauptDatei);
    accept();
}

// =============================================================================

void zeigePublish(QWidget* parent, const QString& datei, const FreezeTon& ton)
{
    pk::Fund fund = pk::sammle(datei);
    if (!fund.ok)
    {
        QMessageBox::warning(parent, QObject::tr("Publish"),
                             QObject::tr("%1 cannot be published.\n%2").arg(QDir::toNativeSeparators(datei), fund.fehler));
        return;
    }
    PublishDialog dialog(std::move(fund), parent, nullptr, ton);
    dialog.exec();
}

void zeigePublishDatei(QWidget* parent, const FreezeTon& ton)
{
    QSettings einstellungen;
    QStringList samples;
    for (const QString& endung : lp::sampleEndungen()) samples << QStringLiteral("*.") + endung;
    const QString datei = QFileDialog::getOpenFileName(
        parent, QObject::tr("Publish a file"), startOrdner(einstellungen, kLetzterQuellOrdner),
        QObject::tr("Loops, instruments, kits, shapes (*.lvloop *.lvinst *.lvkit *.lvcontour);;Samples (%1);;All files (*)")
            .arg(samples.join(QLatin1Char(' '))));
    if (datei.isEmpty()) return;
    einstellungen.setValue(QLatin1String(kLetzterQuellOrdner), QFileInfo(datei).absolutePath());
    zeigePublish(parent, datei, ton);
}

void veroeffentlicheAusFenster(QWidget* parent, pk::Art art, const QString& datei, bool ungespeichert,
                               const std::function<QString()>& speichere, const FreezeTon& ton)
{
    const QString titel = QObject::tr("Publish %1").arg(paketArtWort(art));
    QString pfad = datei;
    if (ungespeichert || pfad.isEmpty())
    {
        if (speichere)
        {
            const auto antwort = QMessageBox::question(
                parent, titel,
                pfad.isEmpty() ? QObject::tr("Publish takes the saved %1. Save it now?").arg(paketArtWort(art))
                               : QObject::tr("Publish takes the saved %1 — your latest changes aren't in it. Save them now?")
                                     .arg(paketArtWort(art)),
                pfad.isEmpty() ? QMessageBox::Save | QMessageBox::Cancel
                               : QMessageBox::Save | QMessageBox::Ignore | QMessageBox::Cancel);
            if (antwort == QMessageBox::Cancel) return;
            if (antwort == QMessageBox::Save)
            {
                pfad = speichere();
                if (pfad.isEmpty()) return;
            }
        }
        else if (pfad.isEmpty())
        {
            QMessageBox::information(parent, titel,
                                     QObject::tr("This %1 has no file of its own: it is built in, or not saved yet. "
                                                 "Built-in ones come with every LumiViz. To pass on a changed one, "
                                                 "save a copy first (Save as...).")
                                         .arg(paketArtWort(art)));
            return;
        }
        else if (QMessageBox::question(parent, titel,
                                       QObject::tr("Publish takes the saved %1 — your latest changes aren't in it. "
                                                   "Publish the saved one?")
                                           .arg(paketArtWort(art))) != QMessageBox::Yes)
        {
            return;
        }
    }
    zeigePublish(parent, pfad, ton);
}

void zeigeImportPaket(QWidget* parent, const QString& gewaehlt, IEventBus* bus, int rate)
{
    QString datei = gewaehlt;
    if (datei.isEmpty())
    {
        QSettings einstellungen;
        datei = QFileDialog::getOpenFileName(parent, QObject::tr("Import package"),
                                             startOrdner(einstellungen, kLetzterImportOrdner),
                                             QObject::tr("LumiViz package (*%1);;All files (*)").arg(QLatin1String(ds::kEndung)));
        if (datei.isEmpty()) return;
        einstellungen.setValue(QLatin1String(kLetzterImportOrdner), QFileInfo(datei).absolutePath());
    }
    if (!pk::istPaket(datei))
    {
        // eine Datensicherung: der Dialog Import data (P8 a)
        const QString ordner = ds::benutzerOrdner();
        DatenImportDialog dialog(datei, ordner, parent);
        if (!dialog.inhalt().ok)
        {
            QMessageBox::warning(parent, QObject::tr("Import package"),
                                 QObject::tr("%1 cannot be imported.\n%2")
                                     .arg(QDir::toNativeSeparators(datei), dialog.inhalt().fehler));
            return;
        }
        if (dialog.exec() == QDialog::Accepted && bus != nullptr) bus->publish(UserDataImportedEvent{});
        return;
    }
    ImportPaketDialog dialog(datei, parent, nullptr, rate);
    if (bus != nullptr)
    {
        QObject::connect(&dialog, &ImportPaketDialog::anspielenVerlangt, &dialog, [bus](int art, const QString& pfad) {
            bus->publish(PreviewSoundFileEvent(static_cast<pk::Art>(art) == pk::Art::Instrument, pfad.toStdString()));
        });
        QObject::connect(&dialog, &ImportPaketDialog::oeffnenVerlangt, &dialog, [bus](int art, const QString& pfad) {
            if (static_cast<pk::Art>(art) == pk::Art::Loop) bus->publish(LoadLoopEvent(pfad.toStdString()));
        });
    }
    // danach lesen offene Listen neu, Zwischenspeicher fallen weg (S125); ein Loop, den
    // oeffnenVerlangt schon geladen hat, las die neuen Dateien ohnehin
    if (dialog.exec() == QDialog::Accepted && bus != nullptr) bus->publish(UserDataImportedEvent{});
}

}  // namespace lumi::ui
