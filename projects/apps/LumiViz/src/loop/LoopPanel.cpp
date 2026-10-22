/**
 * @file LoopPanel.cpp
 * @brief Loop: das Panel — das Arrangement eines Loops (Stufe 6a)
 */

#include "loop/LoopPanel.hpp"

#include "UI/dialogs/PaketDialoge.hpp"
#include "UI/widgets/Achse.hpp"
#include "UI/widgets/Bedienhilfe.hpp"  // zeigeMenue, frageText: in Tests abfragbar
#include "UI/widgets/PresetTypeIcons.hpp"
#include "audio/IAudioEngine.hpp"
#include "audio/IAudioPlayer.hpp"
#include "loop/Bearbeiten.hpp"
#include "loop/ClipEffektFenster.hpp"
#include "loop/ComposerAbfragen.hpp"  // der Composer steht ueber dem Loop (S128, N1 b)
#include "loop/DrumEditor.hpp"
#include "loop/DrumKit.hpp"  // merkeGeoeffnetenLoop
#include "loop/ExportFenster.hpp"
#include "loop/Instrument.hpp"  // zaehleInstrumentFassung
#include "loop/InstrumentEditor.hpp"
#include "loop/Kit.hpp"
#include "loop/Offen.hpp"
#include "loop/Sample.hpp"
#include "loop/Taktachse.hpp"
#include "loop/Tonspeicher.hpp"
#include "shape/Kontur.hpp"  // zaehleZeichnungFassung
#include "services/ICommandBus.hpp"
#include "services/ServiceContainer.hpp"
#include "services/events/UIEvents.hpp"
#include "visualizers/MultiEffectVisualizer.hpp"  // notifyPlayPosition, notifyPlayState (7a)

#include <DockWidget.h>

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QSettings>
#include <QShortcut>
#include <QSignalBlocker>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>

namespace lumi::loop
{

namespace
{

const char* const kLetzterOrdner = "loop/lastDir";
const char* const kLetzteDatei = "loop/lastFile";
/// Einstellung: die Hoehe der Zeilen in der Achse, in Prozent.
const char* const kZeilenProzent = "loop/rowHeightPercent";
constexpr int kTaktMs = 33;
/// Einstellung: Scroll with playhead (S127, W3 a), Vorgabe an.
const char* const kMitlaufen = "loop/scrollWithPlayhead";

void setzeText(QLabel* label, const QString& text)
{
    if (label->text() != text) label->setText(text);
}

/// Ein Schritt des Bearbeitens fuer Undo / Redo: die Fassung davor und danach.
/// Gilt nur fuer die Ladung, in der er entstand — nach New oder Load tut er
/// nichts mehr.
class LoopBefehl : public ICommand
{
public:
    LoopBefehl(LoopPanel& panel, Loop nachher, const QString& was)
        : m_panel(&panel),
          m_ladung(panel.ladung()),
          m_vorher(panel.loop()),
          m_nachher(std::move(nachher)),
          m_was(was.toStdString())
    {
    }

    bool execute() override
    {
        if (!gilt() || m_vorher == m_nachher) return false;
        m_panel->setzeLoop(m_nachher);
        return true;
    }
    void undo() override
    {
        if (gilt()) m_panel->setzeLoop(m_vorher);
    }
    [[nodiscard]] std::string description() const override { return m_was; }

private:
    [[nodiscard]] bool gilt() const { return m_panel != nullptr && m_panel->ladung() == m_ladung; }

    QPointer<LoopPanel> m_panel;
    int m_ladung;
    Loop m_vorher;
    Loop m_nachher;
    std::string m_was;
};

}  // namespace

LoopPanel::LoopPanel(ServiceContainer& services, QWidget* parent)
    : PanelBase(services, "loop", tr("Loop"), parent),
      // Rangfolge (Entscheid Patrik S108, L4): der Composer steht ueber dem Loop
      m_spieler([] { return composerFuehrtTon(); })
{
    setupUI();
    // der zuletzt geladene Loop steht wieder im Panel (gespielt wird nicht von selbst)
    const QString letzte = QSettings().value(QLatin1String(kLetzteDatei)).toString();
    if (!letzte.isEmpty() && QFileInfo::exists(letzte)) lade(letzte);
    zeigeDatei();
    zeigeClip();
    zeigeStand();

    if (auto* bus = eventBus())
        m_ladeSubscription = bus->subscribeScoped<LoadLoopEvent>(
            [this](const LoadLoopEvent& e) { (void)oeffne(QString::fromStdString(e.path)); });
    // nach Import data oder Import package (S125): eigene Dateien koennen ersetzt sein —
    // gerechnete Toene, Samples und Formen vergessen, offene Listen neu lesen
    if (auto* bus = eventBus())
        m_importSubscription = bus->subscribeScoped<UserDataImportedEvent>([this](const UserDataImportedEvent&) {
            zaehleInstrumentFassung();
            vergissSampleDaten();
            tonspeicher().leere();  // wie die Samples: ein Archiv kann Dateien mit altem Stand ersetzen
            lumi::shape::zaehleZeichnungFassung();
            if (m_editor) m_editor->liesEigeneNeu();
            if (m_instrumentEditor) m_instrumentEditor->liesEigeneNeu();
            if (m_vorhoeren == 0) m_spieler.setzeLoop(spielFassung());
            meldeStand();
        });
    // ▶ im Dialog Import package (Publish, S125): ein Sample einen Schlag lang, ein Instrument C4
    if (auto* bus = eventBus())
        m_vorhoerSubscription = bus->subscribeScoped<PreviewSoundFileEvent>([this](const PreviewSoundFileEvent& e) {
            const QString pfad = QString::fromStdString(e.path);
            if (e.instrument)
                (void)spieleInstrumentTon(pfad, 60);
            else
                (void)spieleSchlaege({Spur(QString(), pfad)});
        });
    // vor dem Beenden der App fragt das Hauptfenster hier nach (MainWindow::closeEvent)
    meldeBeendenFrage(this, [this](const QString& titel) { return darfErsetzen(titel); });

    m_takt = new QTimer(this);
    m_takt->setInterval(kTaktMs);
    connect(m_takt, &QTimer::timeout, this, &LoopPanel::beimTakt);
    m_takt->start();
}

LoopPanel::~LoopPanel()
{
    // die Audio-Engine lebt laenger als die Panels: hier noch freigeben
    m_spieler.halte(false);
    m_freeze.reset();  // Stufe 30g: sein Dekodierer nutzt die Engine (Bootstrap_Integration.md §6.7)
    if (m_meldetSong) meldeLaufendenLoop(nullptr);
    vergissBeendenFrage(this);
    meldeOffen({}, {});  // niemand hat mehr einen Loop offen
}

void LoopPanel::meldeStand()
{
    // Wer dieselbe Datei spielt — ein Clip im Composer —, hoert die Fassung
    // des Panels, auch die ungespeicherte (Entscheid Patrik S108, L7 a). Ein
    // Loop ohne Datei kann nirgends als Clip liegen.
    meldeOffen(m_geladen ? m_datei : QString(), m_loop);
}

void LoopPanel::setzeTags(const LoopTags& tags)
{
    if (!m_geladen) return;
    Loop neuer = m_loop;
    neuer.tags = tags;
    bearbeite(neuer, tr("Edit tags"));
}

void LoopPanel::frageNachTags()
{
    if (!m_geladen) return;
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Loop tags"));
    dialog.setObjectName(QStringLiteral("loopTagsDialog"));
    auto* formular = new QFormLayout(&dialog);
    const auto feld = [&](const QString& name, const QString& wert) {
        auto* eingabe = new QLineEdit(wert, &dialog);
        formular->addRow(name, eingabe);
        return eingabe;
    };
    QLineEdit* titel = feld(tr("Title"), m_loop.tags.titel);
    titel->setPlaceholderText(m_datei.isEmpty() ? QString() : QFileInfo(m_datei).completeBaseName());
    QLineEdit* kuenstler = feld(tr("Artist"), m_loop.tags.kuenstler);
    QLineEdit* album = feld(tr("Album"), m_loop.tags.album);
    QLineEdit* jahr = feld(tr("Year"), m_loop.tags.jahr);
    QLineEdit* genre = feld(tr("Genre"), m_loop.tags.genre);
    QLineEdit* nummer = feld(tr("Track"), m_loop.tags.nummer);
    auto* text = new QPlainTextEdit(m_loop.tags.text, &dialog);
    text->setPlaceholderText(tr("Lyrics — plain text or LRC with time tags"));
    formular->addRow(tr("Lyrics"), text);
    auto* knoepfe = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    formular->addRow(knoepfe);
    connect(knoepfe, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(knoepfe, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (lumi::ui::fuehreDialogAus(dialog) != QDialog::Accepted) return;
    LoopTags tags;
    tags.titel = titel->text().trimmed();
    tags.kuenstler = kuenstler->text().trimmed();
    tags.album = album->text().trimmed();
    tags.jahr = jahr->text().trimmed();
    tags.genre = genre->text().trimmed();
    tags.nummer = nummer->text().trimmed();
    tags.text = text->toPlainText();
    setzeTags(tags);
}

void LoopPanel::setupUI()
{
    auto* aussen = new QVBoxLayout(this);

    auto* dateiReihe = new QHBoxLayout;
    // das Datei-Icon der Loops vor dem Namen, wie im Panel Composer (S120,
    // Entscheid Patrik A2 a); zu sehen, solange ein Loop geladen ist
    m_dateiIcon = new QLabel(this);
    m_dateiIcon->setObjectName(QStringLiteral("loopFileIcon"));
    if (const QIcon marke = lumi::ui::dateiArtIcon(QStringLiteral("lvloop")); !marke.isNull())
        m_dateiIcon->setPixmap(marke.pixmap(18, 18));
    m_dateiIcon->setVisible(false);
    m_dateiZeile = new QLabel(this);
    m_dateiZeile->setObjectName(QStringLiteral("loopFile"));
    m_dateiZeile->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_neu = new QPushButton(tr("New"), this);
    m_neu->setObjectName(QStringLiteral("loopNew"));
    m_neu->setToolTip(tr("Start an empty loop"));
    m_laden = new QPushButton(tr("Load..."), this);
    m_laden->setObjectName(QStringLiteral("loopLoad"));
    m_laden->setToolTip(tr("Load a loop file (.lvloop)"));
    m_speichern = new QPushButton(tr("Save"), this);
    m_speichern->setObjectName(QStringLiteral("loopSave"));
    m_speichernUnter = new QPushButton(tr("Save As..."), this);
    m_speichernUnter->setObjectName(QStringLiteral("loopSaveAs"));
    m_publish = new QPushButton(tr("Publish..."), this);
    m_publish->setObjectName(QStringLiteral("loopPublish"));
    m_publish->setToolTip(tr("Pass the loop on as one file, with the instruments, kits, shapes and samples "
                             "of your own it uses"));
    // Stufe 30c (WX8 a): der ganze Loop als WAV, zwischen Save As... und Publish...
    m_exportieren = new QPushButton(tr("Export audio..."), this);
    m_exportieren->setObjectName(QStringLiteral("loopExportAudio"));
    m_exportieren->setToolTip(tr("Write the whole loop as you hear it here — with faders, loop volume and limiter — "
                                 "into a WAV file"));
    m_tags = new QPushButton(tr("Tags..."), this);
    m_tags->setObjectName(QStringLiteral("loopTags"));
    m_tags->setToolTip(tr("Title, artist and the other tags of the loop — what the player shows "
                          "while the loop runs as a clip in the Composer"));
    dateiReihe->addWidget(m_dateiIcon);
    dateiReihe->addWidget(m_dateiZeile, 1);
    dateiReihe->addWidget(m_tags);
    dateiReihe->addWidget(m_neu);
    dateiReihe->addWidget(m_laden);
    dateiReihe->addWidget(m_speichern);
    dateiReihe->addWidget(m_speichernUnter);
    dateiReihe->addWidget(m_exportieren);
    dateiReihe->addWidget(m_publish);
    aussen->addLayout(dateiReihe);

    auto* spielReihe = new QHBoxLayout;
    m_play = new QPushButton(tr("Play"), this);
    m_play->setObjectName(QStringLiteral("loopPlay"));
    auto* bpmName = new QLabel(tr("BPM"), this);
    m_bpm = new QDoubleSpinBox(this);
    m_bpm->setObjectName(QStringLiteral("loopBpm"));
    m_bpm->setRange(kBpmMin, kBpmMax);
    m_bpm->setDecimals(1);
    m_bpm->setKeyboardTracking(false);  // erst die fertige Zahl gilt — nicht jede Ziffer ein Schritt fuer Undo
    m_bpm->setToolTip(tr("Tempo of the whole loop, in beats per minute"));
    m_info = new QLabel(this);
    m_info->setObjectName(QStringLiteral("loopInfo"));
    m_stelle = new QLabel(this);
    m_stelle->setObjectName(QStringLiteral("loopPosition"));
    m_fit = new QPushButton(tr("Fit"), this);
    m_fit->setObjectName(QStringLiteral("loopFit"));
    m_fit->setToolTip(tr("Show the whole loop, rows in normal height (Ctrl+0)") + QLatin1Char('\n') +
                      lumi::ui::Achse::zoomGesten().join(QLatin1Char('\n')));
    spielReihe->addWidget(m_play);
    spielReihe->addWidget(bpmName);
    spielReihe->addWidget(m_bpm);
    spielReihe->addWidget(m_info, 1);
    spielReihe->addWidget(m_stelle);
    // Mitlaufen (S127, Entscheid Patrik W3 a): wie im Composer, Vorgabe an
    m_mitlaufen = new QCheckBox(tr("Scroll with playhead"), this);
    m_mitlaufen->setObjectName(QStringLiteral("loopScrollWithPlayhead"));
    m_mitlaufen->setChecked(QSettings().value(QLatin1String(kMitlaufen), true).toBool());
    m_mitlaufen->setToolTip(
        tr("While something plays — Play here, Play pattern or the Composer — the arrangement and the "
           "pattern editors follow the playhead when they are zoomed in or longer than the window. "
           "Continuous or page by page as set for the Composer (Settings). While you drag a scroll "
           "bar, nothing moves."));
    connect(m_mitlaufen, &QCheckBox::toggled, this, [](bool an) { QSettings().setValue(QLatin1String(kMitlaufen), an); });
    // der Begrenzer am Ausgang (27e-3, Entscheide Patrik LL7 a, LL13 a): gehoert zum Loop, Vorgabe an
    m_limiter = new QCheckBox(tr("Limiter"), this);
    m_limiter->setObjectName(QStringLiteral("loopLimiter"));
    m_limiter->setChecked(true);
    m_limiter->setToolTip(
        tr("Keeps the sum of all channels below 0 dB while this panel plays the loop.\n\n"
           "On: peaks above 0 dB — for example from a channel fader above 0 dB — are turned down "
           "smoothly instead of clipping. Everything below passes unchanged.\n\n"
           "Off: the sum goes out as it is; peaks above 0 dB clip at the output.\n\n"
           "In the Composer the limiter of the composition applies instead. Saved with the loop."));
    connect(m_limiter, &QCheckBox::clicked, this, [this](bool an) {
        if (!m_geladen) return;
        Loop neuer = m_loop;
        neuer.limiter = an;
        bearbeite(neuer, an ? tr("Turn on limiter") : tr("Turn off limiter"));
    });
    spielReihe->addWidget(m_limiter);
    spielReihe->addWidget(m_mitlaufen);
    spielReihe->addWidget(m_fit);
    aussen->addLayout(spielReihe);

    m_achse = new Taktachse(this);
    m_achse->setZeilenProzent(QSettings().value(QLatin1String(kZeilenProzent), 100).toInt());
    aussen->addWidget(new lumi::ui::AchsenRahmen(m_achse, this), 1);

    m_clipZeile = new QLabel(this);
    m_clipZeile->setObjectName(QStringLiteral("loopClip"));
    aussen->addWidget(m_clipZeile);

    m_hinweis = new QLabel(tr("The Composer plays the audio — start it in the Composer panel"), this);
    m_hinweis->setObjectName(QStringLiteral("loopHint"));
    m_hinweis->setWordWrap(true);
    m_hinweis->hide();
    aussen->addWidget(m_hinweis);

    m_bericht = new QLabel(this);
    m_bericht->setObjectName(QStringLiteral("loopReport"));
    m_bericht->setWordWrap(true);
    m_bericht->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_bericht->hide();
    aussen->addWidget(m_bericht);

    connect(m_neu, &QPushButton::clicked, this, [this] {
        if (darfErsetzen(tr("New Loop"))) neu();
    });
    connect(m_laden, &QPushButton::clicked, this, &LoopPanel::frageNachDatei);
    connect(m_tags, &QPushButton::clicked, this, &LoopPanel::frageNachTags);
    connect(m_speichern, &QPushButton::clicked, this, [this] { (void)speichereMitDialog(false); });
    connect(m_speichernUnter, &QPushButton::clicked, this, [this] { (void)speichereMitDialog(true); });
    // Publish (Publish_Konzept.md P2 a): die gespeicherte Datei; ungespeichert bietet es Speichern an
    connect(m_publish, &QPushButton::clicked, this, [this] {
        lumi::ui::veroeffentlicheAusFenster(this, lumi::paket::Art::Loop, m_datei, m_datei.isEmpty() || ungespeichert(),
                                            [this] { return speichereMitDialog(m_datei.isEmpty()) ? m_datei : QString(); });
    });
    connect(m_play, &QPushButton::clicked, this, [this] {
        if (m_spieler.spielt() && !hoertVor())
            halte();
        else
            spiele();
    });
    connect(m_bpm, &QDoubleSpinBox::valueChanged, this, [this](double bpm) {
        if (!m_geladen) return;
        Loop neuer = m_loop;
        neuer.bpm = bpm;
        bearbeite(neuer, tr("Change tempo"));
    });
    connect(m_fit, &QPushButton::clicked, m_achse, &Taktachse::setzeZoomZurueck);
    auto* fitKuerzel = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_0), this);
    fitKuerzel->setContext(Qt::WidgetWithChildrenShortcut);
    connect(fitKuerzel, &QShortcut::activated, m_achse, &Taktachse::setzeZoomZurueck);

    connect(m_achse, &Taktachse::bearbeitet, this, &LoopPanel::bearbeite);
    connect(m_achse, &Taktachse::auswahlGeaendert, this, &LoopPanel::zeigeClip);
    // am Fader (27e-2): die Zeile unter der Achse nennt die Gesten; ohne Hinweis wieder den Clip
    connect(m_achse, &Taktachse::hinweis, this, [this](const QString& text) {
        if (text.isEmpty())
            zeigeClip();
        else
            m_clipZeile->setText(text);
    });
    connect(m_achse, &Taktachse::editorVerlangt, this, [this](const QString& muster) { (void)oeffneEditor(muster); });
    connect(m_achse, &Taktachse::effekteVerlangt, this, [this] { (void)oeffneEffekte(); });
    // Stufe 30c (WX6 a, WX8 a): ein Clip oder der ganze Loop als WAV
    connect(m_achse, &Taktachse::exportVerlangt, this,
            [this](int kanal, int startTakt) { (void)oeffneExport(ClipOrt{kanal, startTakt}); });
    connect(m_exportieren, &QPushButton::clicked, this, [this] { (void)oeffneExport(); });
    // Stufe 30g (FZ4 a B): Freeze clip / Unfreeze clip
    connect(m_achse, &Taktachse::freezeVerlangt, this, [this](bool einfrieren) { (void)friereEin(einfrieren); });
    // B4 (S124): ein Klick ins Lineal — dorthin springen, oder dort beginnen
    connect(m_achse, &Taktachse::stelleGewaehlt, this, &LoopPanel::setzeStartstelle);
    connect(m_achse, &Taktachse::zeilenProzentGeaendert, this,
            [](int prozent) { QSettings().setValue(QLatin1String(kZeilenProzent), prozent); });
}

bool LoopPanel::darfErsetzen(const QString& titel, const std::function<Rueckfrage()>& frage)
{
    if (!ungespeichert()) return true;
    Rueckfrage wahl = Rueckfrage::Bleiben;
    if (frage)
        wahl = frage();
    else
    {
        // dieselbe Frage wie im Composer (`composer::darfKompositionErsetzen`)
        const QString name = m_datei.isEmpty() ? tr("untitled") : QFileInfo(m_datei).fileName();
        const QMessageBox::StandardButton knopf =
            QMessageBox::question(this, titel, tr("The loop '%1' has unsaved changes.").arg(name),
                                  QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
        if (knopf == QMessageBox::Save) wahl = Rueckfrage::Speichern;
        else if (knopf == QMessageBox::Discard) wahl = Rueckfrage::Verwerfen;
    }
    if (wahl == Rueckfrage::Bleiben) return false;
    if (wahl == Rueckfrage::Verwerfen) return true;
    return speichereMitDialog(false);
}

void LoopPanel::frageNachDatei()
{
    if (!darfErsetzen(tr("Load Loop"))) return;
    QSettings einstellungen;
    QString ordner = einstellungen.value(QLatin1String(kLetzterOrdner)).toString();
    // beim ersten Mal: der Ordner mit dem mitgelieferten Beispiel
    if (ordner.isEmpty() && !kitOrdner().isEmpty()) ordner = QDir::cleanPath(kitOrdner() + QStringLiteral("/.."));
    const QString pfad =
        QFileDialog::getOpenFileName(this, tr("Load loop"), ordner, tr("Loop (*.lvloop);;All files (*)"));
    if (pfad.isEmpty()) return;
    einstellungen.setValue(QLatin1String(kLetzterOrdner), QFileInfo(pfad).absolutePath());
    if (!lade(pfad))
    {
        // was geladen war, bleibt; die Meldung steht, bis wieder etwas geladen wird
        m_ladeBericht = {tr("Not a loop file: %1").arg(QDir::toNativeSeparators(pfad))};
        zeigeStand();
    }
}

bool LoopPanel::speichereMitDialog(bool unter)
{
    if (!m_geladen) return false;
    QString pfad = m_datei;
    if (unter || pfad.isEmpty())
    {
        QSettings einstellungen;
        const QString ordner = m_datei.isEmpty() ? einstellungen.value(QLatin1String(kLetzterOrdner)).toString()
                                                 : m_datei;
        pfad = QFileDialog::getSaveFileName(this, tr("Save loop"), ordner, tr("Loop (*.lvloop)"));
        if (pfad.isEmpty()) return false;
        if (QFileInfo(pfad).suffix().isEmpty()) pfad += QStringLiteral(".lvloop");
        einstellungen.setValue(QLatin1String(kLetzterOrdner), QFileInfo(pfad).absolutePath());
    }
    if (speichere(pfad)) return true;
    QMessageBox::warning(this, tr("Loop"), tr("Could not write %1").arg(QDir::toNativeSeparators(pfad)));
    return false;
}

void LoopPanel::neu()
{
    beendeVorhoeren();
    Loop leer;
    (void)kanalHinzu(leer);
    m_loop = leer;
    m_gespeichert = leer;
    m_datei.clear();
    m_geladen = true;
    ++m_ladung;
    m_ladeBericht.clear();
    m_solo.clear();
    m_spieler.setzeLoop(m_loop);
    m_achse->setLoop(m_loop, true);
    if (m_editor != nullptr) m_editor->setLoop(m_loop);  // seine Basis gibt es nicht mehr: er schliesst sich
    if (m_instrumentEditor != nullptr) m_instrumentEditor->setLoop(m_loop);
    bestimmeFreeze();
    zeigeDatei();
    zeigeClip();
    zeigeStand();
}

bool LoopPanel::lade(const QString& pfad)
{
    Loop loop;
    QStringList bericht;
    if (!ladeLoop(pfad, loop, &bericht)) return false;
    beendeVorhoeren();
    m_loop = loop;
    m_gespeichert = loop;
    m_datei = pfad;
    m_geladen = true;
    ++m_ladung;
    m_ladeBericht = bericht;
    m_solo.clear();
    m_spieler.setzeLoop(m_loop);
    m_achse->setLoop(m_loop, true);
    if (m_editor != nullptr) m_editor->setLoop(m_loop);  // gibt es seine Basis hier nicht, schliesst er sich
    if (m_instrumentEditor != nullptr) m_instrumentEditor->setLoop(m_loop);
    // H1 a: die Toene schon jetzt im Hintergrund rechnen — Play findet sie dann im Speicher
    if (IAudioEngine* engine = services().tryResolve<IAudioEngine>()) m_spieler.ladeVor(*engine);
    QSettings().setValue(QLatin1String(kLetzteDatei), pfad);
    merkeGeoeffnetenLoop(pfad);  // fuer „Used by" im Fenster Kits
    setzeStartstelle(0);         // ein anderer Loop beginnt wieder vorn
    bestimmeFreeze();            // Stufe 30g: fehlt eine Wellenform, wird sie gerechnet (F5 a)
    zeigeDatei();
    zeigeClip();
    zeigeStand();
    return true;
}

bool LoopPanel::oeffne(const QString& pfad)
{
    if (!darfErsetzen(tr("Load Loop"))) return false;
    const bool geladen = lade(pfad);
    if (!geladen)
    {
        // was geladen war, bleibt; die Meldung steht, bis wieder etwas geladen wird
        m_ladeBericht = {tr("Not a loop file: %1").arg(QDir::toNativeSeparators(pfad))};
        zeigeStand();
    }
    else
        QSettings().setValue(QLatin1String(kLetzterOrdner), QFileInfo(pfad).absolutePath());
    // das Dock, in dem das Panel sitzt (auch unter einer Rollflaeche): nach vorn
    for (QWidget* w = this; w != nullptr; w = w->parentWidget())
        if (auto* dock = qobject_cast<ads::CDockWidget*>(w))
        {
            dock->toggleView(true);
            dock->raise();
            break;
        }
    return geladen;
}

bool LoopPanel::speichere(const QString& pfad)
{
    if (!m_geladen || pfad.isEmpty()) return false;
    // F2 a: eingefrorene Clips gelten mit dem Stand, der jetzt gespeichert wird — ihre Marken
    // bekommen den jetzigen Abdruck, was fehlt, rechnet danach im Hintergrund
    Loop gespeichert = m_loop;
    const std::vector<ClipOrt> zuRechnen = frischeFrozenMarken(gespeichert, freezeRate(), saitenAusInstrumenten());
    if (!speichereLoop(gespeichert, pfad)) return false;
    if (!(gespeichert == m_loop))
    {
        m_loop = gespeichert;
        m_achse->zeigeBearbeitet(m_loop);
    }
    starteFreeze(zuRechnen);
    bestimmeFreeze();
    m_datei = pfad;
    m_gespeichert = m_loop;
    QSettings().setValue(QLatin1String(kLetzteDatei), pfad);
    merkeGeoeffnetenLoop(pfad);
    zeigeDatei();
    return true;
}

void LoopPanel::bearbeite(const Loop& neu, const QString& was)
{
    if (!m_geladen) return;
    if (auto* bus = services().tryResolve<ICommandBus>())
        (void)bus->execute(std::make_unique<LoopBefehl>(*this, neu, was));
    else
        setzeLoop(neu);
}

void LoopPanel::setzeLoop(const Loop& loop)
{
    beendeVorhoeren();  // sonst spielte statt des angespielten Tons ploetzlich der Loop
    m_loop = loop;
    m_spieler.setzeLoop(spielFassung());  // spielt er, gilt die Fassung ab dem naechsten Block
    m_spitze = 0.0f;                      // die Spitze der Fassung davor sagt nichts mehr
    m_achse->zeigeBearbeitet(m_loop);
    if (m_editor != nullptr) m_editor->setLoop(m_loop);
    if (m_instrumentEditor != nullptr) m_instrumentEditor->setLoop(m_loop);
    bestimmeFreeze();  // Stufe 30g: veraendert, oder (nach Undo) wieder eingefroren
    zeigeDatei();
    zeigeClip();
    zeigeStand();
}

Loop LoopPanel::spielFassung()
{
    const Muster* solo = m_solo.isEmpty() ? nullptr : m_loop.findeMuster(m_solo);
    if (solo == nullptr)
    {
        m_solo.clear();  // das Muster gibt es nicht mehr: wieder der ganze Loop
        return m_loop;
    }
    // E1 a (S124): mit den Effekten des Bezugsclips — der markierte, sonst der erste der Basis
    return soloFassung(m_loop, m_solo, markierterClip());
}

bool LoopPanel::spiele()
{
    if (!m_geladen) return false;
    IAudioEngine* engine = services().tryResolve<IAudioEngine>();
    IAudioPlayer* player = services().tryResolve<IAudioPlayer>();
    if (engine == nullptr || player == nullptr) return false;
    m_vorhoeren = 0;  // ein angespielter Ton weicht: sein Zeitgeber haelt nichts mehr an
    m_solo.clear();
    m_spieler.setzeLoop(m_loop);
    m_spitze = 0.0f;
    const bool spielt = m_spieler.starte(*engine, *player);
    if (spielt && m_startSchritt > 0) (void)m_spieler.springe(m_startSchritt);  // B4: dort beginnen
    zeigeStand();
    return spielt;
}

void LoopPanel::setzeStartstelle(int schritt)
{
    m_startSchritt = std::max(0, schritt);
    m_achse->setStartmarke(m_startSchritt);
    // spielt der ganze Loop, springt er; ein Muster allein gehoert seinem Editor
    if (m_spieler.spielt() && m_solo.isEmpty() && m_vorhoeren == 0) (void)m_spieler.springe(m_startSchritt);
}

bool LoopPanel::spieleMuster(const QString& muster)
{
    if (!m_geladen || m_loop.findeMuster(muster) == nullptr) return false;
    IAudioEngine* engine = services().tryResolve<IAudioEngine>();
    IAudioPlayer* player = services().tryResolve<IAudioPlayer>();
    if (engine == nullptr || player == nullptr) return false;
    m_vorhoeren = 0;
    m_solo = muster;
    m_spieler.setzeLoop(spielFassung());
    m_spitze = 0.0f;
    const bool spielt = m_spieler.starte(*engine, *player);
    if (!spielt)
    {
        m_solo.clear();
        m_spieler.setzeLoop(m_loop);
    }
    zeigeStand();
    return spielt;
}

void LoopPanel::halte()
{
    m_spieler.halte(true);
    if (!m_solo.isEmpty() || m_vorhoeren != 0)
    {
        m_solo.clear();
        m_vorhoeren = 0;
        m_spieler.setzeLoop(m_loop);
    }
    zeigeStand();
}

DrumEditor* LoopPanel::editor() const
{
    return m_editor;
}

InstrumentEditor* LoopPanel::instrumentEditor() const
{
    return m_instrumentEditor;
}

ClipEffektFenster* LoopPanel::effektFenster() const
{
    return m_effekte;
}

bool LoopPanel::oeffneExport(std::optional<ClipOrt> clip)
{
    if (!m_geladen) return false;
    IAudioEngine* engine = services().tryResolve<IAudioEngine>();
    ExportWunsch vorlage;
    vorlage.art = clip ? ExportArt::Clip : ExportArt::Loop;
    vorlage.loop = m_loop;
    vorlage.loop.richte();
    if (clip) vorlage.clip = *clip;
    vorlage.rate = engine != nullptr && engine->getSampleRate() > 0 ? engine->getSampleRate() : 44100;
    const QString name = !m_datei.isEmpty()          ? QFileInfo(m_datei).completeBaseName()
                         : !m_loop.tags.titel.isEmpty() ? m_loop.tags.titel
                                                        : tr("untitled");
    QString text = name;
    if (clip)
    {
        const int nr = findeClip(vorlage.loop, *clip);
        if (nr < 0) return false;
        const Kanal& kanal = vorlage.loop.kanaele[static_cast<std::size_t>(clip->kanal)];
        const LoopClip& c = kanal.clips[static_cast<std::size_t>(nr)];
        text = tr("%1 · %2, bars %3–%4").arg(kanal.name, c.muster).arg(c.startTakt + 1).arg(c.startTakt + vorlage.loop.clipTakte(c));
    }
    // dekodiert wird ueber die Engine (TV3 a); ohne Engine bleiben Samples still
    Dekodierer dekodierer = [engine](const QString& datei, int rate, double maxSekunden) {
        return engine != nullptr ? engine->decodeStereo(datei, rate, maxSekunden) : std::vector<float>{};
    };
    ExportFenster fenster(std::move(vorlage), name, text, std::move(dekodierer), this);
    (void)lumi::ui::fuehreDialogAus(fenster);
    return true;
}

bool LoopPanel::oeffneEffekte(bool zeigen)
{
    if (!m_geladen) return false;
    if (m_effekte == nullptr)
    {
        m_effekte = new ClipEffektFenster(this);
        connect(m_effekte, &ClipEffektFenster::bearbeitet, this, &LoopPanel::bearbeite);
    }
    m_effekte->zeige(m_achse->loop(), m_achse->markierte());
    if (zeigen)
    {
        m_effekte->show();
        m_effekte->raise();
        m_effekte->activateWindow();
    }
    zeigeStand();
    return true;
}

bool LoopPanel::spieleTon(const QString& muster, int hoehe)
{
    const Muster* basis = m_geladen ? m_loop.findeMuster(muster) : nullptr;
    if (basis == nullptr || basis->art != MusterArt::Instrument) return false;
    // mit Lautstaerke und Panorama des Musters
    return spieleVor(*basis, hoehe);
}

bool LoopPanel::spieleInstrumentTon(const QString& nennung, int hoehe, int klingtMs)
{
    if (!m_geladen || nennung.isEmpty()) return false;
    Muster ton;
    ton.art = MusterArt::Instrument;
    ton.instrument = nennung;
    return spieleVor(ton, hoehe, klingtMs);
}

bool LoopPanel::spieleInstrumentNoten(const QString& nennung, const Muster& noten, int klingtMs)
{
    if (!m_geladen || nennung.isEmpty() || noten.noten.empty()) return false;
    Muster ton = noten;
    ton.art = MusterArt::Instrument;
    ton.instrument = nennung;
    return spieleVorNoten(ton, klingtMs);
}

bool LoopPanel::spieleAkkordVor(const QString& nennung, const Muster& noten)
{
    if (!m_geladen || nennung.isEmpty() || noten.noten.empty()) return false;
    Muster ton = noten;
    ton.art = MusterArt::Instrument;
    ton.instrument = nennung;
    ton.stumm = false;
    int ende = 0;  // in Schritten: wo die letzte Note aufhoert
    for (const Note& note : ton.noten)
    {
        if (note.hoehe < kHoeheMin || note.hoehe > kHoeheMax || note.schritt < 0) return false;
        ende = std::max(ende, note.schritt + std::max(1, note.laenge));
    }
    const double schrittMs = 60000.0 / m_loop.bpm / std::max(1, m_loop.schritteJeSchlag);
    const int dauerMs = static_cast<int>(std::lround(ende * schrittMs)) + 600;
    const double taktMs = 60000.0 / m_loop.bpm * m_loop.schlaegeJeTakt;
    ton.takte = std::clamp(static_cast<int>(std::ceil(dauerMs / taktMs)) + 1, kTakteMin, kTakteMax);
    return spieleAllein(ton, dauerMs);
}

bool LoopPanel::spieleCreatorNoten(const QString& nennung, const Muster& noten, int klingtMs)
{
    const bool folge = std::any_of(noten.noten.begin(), noten.noten.end(),
                                   [&](const Note& n) { return n.schritt != noten.noten.front().schritt; });
    return folge ? spieleAkkordVor(nennung, noten) : spieleInstrumentNoten(nennung, noten, klingtMs);
}

bool LoopPanel::spieleVor(const Muster& vorlage, int hoehe, int klingtMs)
{
    if (hoehe < kHoeheMin || hoehe > kHoeheMax) return false;
    Muster ton = vorlage;
    ton.noten = {{0, 1, hoehe, 1.0}};
    ton.anschlaege.clear();  // die Abweichungen der Akkorde des Musters gelten fuer diesen einen Ton nicht
    return spieleVorNoten(ton, klingtMs);
}

bool LoopPanel::spieleVorNoten(const Muster& vorlage, int klingtMs)
{
    // die Noten des Musters, jede an Schritt 0 und einen Schlag lang (ein Akkord bleibt einer)
    Muster ton = vorlage;
    ton.stumm = false;
    for (Note& note : ton.noten)
    {
        if (note.hoehe < kHoeheMin || note.hoehe > kHoeheMax) return false;
        note.schritt = 0;
        note.laenge = m_loop.schritteJeSchlag;
    }
    for (Anschlag& anschlag : ton.anschlaege) anschlag.schritt = 0;
    if (ton.noten.empty()) return false;
    // der Ton und sein Ausklang — oder so lang, wie „Repeat note" ihn klingen laesst
    const int dauerMs = klingtMs > 0 ? klingtMs : m_loop.notenMs(ton.noten.front()) + 600;
    // so viele Takte, dass das Muster nicht von vorn beginnt, bevor die Zeit um ist
    const double taktMs = 60000.0 / m_loop.bpm * m_loop.schlaegeJeTakt;
    ton.takte = std::clamp(static_cast<int>(std::ceil(dauerMs / taktMs)) + 1, kTakteMin, kTakteMax);
    return spieleAllein(ton, dauerMs);
}

bool LoopPanel::spieleSpur(const QString& muster, int spur)
{
    const Muster* basis = m_geladen ? m_loop.findeMuster(muster) : nullptr;
    if (basis == nullptr || basis->art != MusterArt::Drumkit || spur < 0 || spur >= static_cast<int>(basis->spuren.size()))
        return false;
    return spieleSchlaege({basis->spuren[static_cast<std::size_t>(spur)]});
}

bool LoopPanel::spieleSchlaege(const std::vector<Spur>& spuren)
{
    if (!m_geladen) return false;
    std::vector<Spur> spielen;
    for (const Spur& spur : spuren)
        if (!spur.spielt().isEmpty()) spielen.push_back(spur);
    if (spielen.empty()) return false;
    for (Spur& spur : spielen) spur.stumm = false;
    // ein Instrument klingt Length lang und klingt dann aus; ein Klang ist so lang wie seine Datei —
    // angespielt werden davon hoechstens zwei Sekunden; der letzte Schlag kommt nach den anderen
    const double schlagMs = 60000.0 / m_loop.bpm;
    const Spur& letzte = spielen.back();
    const int dauerMs = static_cast<int>(std::ceil(schlagMs * static_cast<double>(spielen.size() - 1))) +
                        (letzte.instrument.isEmpty() ? 2000 : letzte.dauerMs) + 600;
    // ein Muster mit diesen Spuren, je Spur ein Schlag, einen Schlag nach dem anderen — so lang,
    // dass es nicht von vorn beginnt, bevor der letzte verklungen ist
    const double taktMs = schlagMs * m_loop.schlaegeJeTakt;
    Muster schlag;
    schlag.art = MusterArt::Drumkit;
    schlag.takte = std::clamp(static_cast<int>(std::ceil(dauerMs / taktMs)) + 1, kTakteMin, kTakteMax);
    const int schritte = schlag.takte * m_loop.schritteJeTakt();
    // mehr Schlaege, als das laengste Muster fasst, faellt weg
    while (static_cast<int>(spielen.size() - 1) * m_loop.schritteJeSchlag >= schritte) spielen.pop_back();
    schlag.spuren = spielen;
    schlag.zeilen.assign(spielen.size(), std::vector<double>(static_cast<std::size_t>(schritte), 0.0));
    for (std::size_t s = 0; s < spielen.size(); ++s)
        schlag.zeilen[s][s * static_cast<std::size_t>(m_loop.schritteJeSchlag)] = 1.0;
    return spieleAllein(schlag, dauerMs);
}

bool LoopPanel::spieleAllein(const Muster& muster, int dauerMs)
{
    // spielt der Loop oder ein Muster, hoert man es schon — das Anspielen wuerde es unterbrechen
    if (m_spieler.spielt() && m_vorhoeren == 0) return false;
    IAudioEngine* engine = services().tryResolve<IAudioEngine>();
    IAudioPlayer* player = services().tryResolve<IAudioPlayer>();
    if (engine == nullptr || player == nullptr) return false;

    Muster ton = muster;
    ton.name = QStringLiteral("Preview");
    Loop allein = m_loop;
    allein.muster = {ton};
    allein.kanaele = {Kanal{QStringLiteral("Preview"), true, {LoopClip{ton.name, 0, 1}}}};
    m_solo.clear();
    m_spieler.setzeLoop(allein);
    m_spitze = 0.0f;
    if (!m_spieler.starte(*engine, *player))
    {
        m_vorhoeren = 0;
        m_spieler.setzeLoop(m_loop);
        zeigeStand();
        return false;
    }
    const int nummer = ++m_vorhoerNr;
    m_vorhoeren = nummer;
    // wird sein Klang noch gerechnet (TV6 a), laeuft die Zeit erst, wenn er da ist
    m_vorhoerDauerMs = 0;
    if (m_spieler.wartet())
        m_vorhoerDauerMs = std::max(1, dauerMs);
    else
        beendeVorhoerenNach(dauerMs);
    zeigeStand();
    return true;
}

void LoopPanel::beendeVorhoerenNach(int dauerMs)
{
    // nach dem Ton und seinem Ausklang haelt der Spieler wieder — ausser ein neuerer Ton laeuft
    const int nummer = m_vorhoeren;
    QTimer::singleShot(dauerMs, this, [this, nummer] {
        if (m_vorhoeren == nummer) beendeVorhoeren();
    });
}

void LoopPanel::beendeVorhoeren()
{
    if (m_vorhoeren == 0) return;
    m_vorhoeren = 0;
    m_spieler.halte(true);
    m_spieler.setzeLoop(m_loop);
    zeigeStand();
}

bool LoopPanel::oeffneEditor(const QString& muster, bool zeigen)
{
    const Muster* basis = m_geladen ? m_loop.findeMuster(muster) : nullptr;
    if (basis == nullptr) return false;
    // die Art der Basis bestimmt den Editor (Stufe 9b): je Art ein Fenster
    if (basis->art == MusterArt::Instrument)
    {
        if (m_instrumentEditor == nullptr)
        {
            m_instrumentEditor = new InstrumentEditor(this);
            connect(m_instrumentEditor, &InstrumentEditor::bearbeitet, this, &LoopPanel::bearbeite);
            connect(m_instrumentEditor, &InstrumentEditor::spielenVerlangt, this, [this](bool spielen) {
                if (spielen && m_instrumentEditor != nullptr)
                {
                    // B4 b: Play pattern beginnt an der Abspielstelle des Editors
                    if (spieleMuster(m_instrumentEditor->muster()))
                        (void)m_spieler.springe(m_instrumentEditor->raster()->abspielstelle());
                }
                else
                    halte();
            });
            connect(m_instrumentEditor, &InstrumentEditor::abspielstelleGewaehlt, this, [this](int schritt) {
                if (m_instrumentEditor != nullptr && m_spieler.spielt() && m_solo == m_instrumentEditor->muster())
                    (void)m_spieler.springe(schritt);
            });
            connect(m_instrumentEditor, &InstrumentEditor::tonVerlangt, this, [this](int hoehe) {
                if (m_instrumentEditor != nullptr) (void)spieleTon(m_instrumentEditor->muster(), hoehe);
            });
            // der Instrument-Creator (Stufe 10b): er spielt sein Instrument an, auch ungespeichert
            connect(m_instrumentEditor, &InstrumentEditor::instrumentTonVerlangt, this,
                    [this](const QString& nennung, int hoehe) { (void)spieleInstrumentTon(nennung, hoehe); });
            // der Creator mit den Noten nach Play as (Stufe 24d); mit „Repeat note" bis zum naechsten
            connect(m_instrumentEditor, &InstrumentEditor::instrumentVorhoerenVerlangt, this,
                    [this](const QString& nennung, const Muster& noten, int klingtMs) {
                        (void)spieleCreatorNoten(nennung, noten, klingtMs);
                    });
            // das Fenster Chords spielt ein Griffbild an (Stufe 26e-2)
            connect(m_instrumentEditor, &InstrumentEditor::akkordVorhoerenVerlangt, this,
                    [this](const QString& nennung, const Muster& noten) { (void)spieleAkkordVor(nennung, noten); });
            // eine Instrument-Datei ist neu geschrieben: wer sie spielt, rechnet ihre Toene neu
            connect(m_instrumentEditor, &InstrumentEditor::instrumentGespeichert, this, [this] {
                if (m_vorhoeren == 0) m_spieler.setzeLoop(spielFassung());
                meldeStand();  // auch ein Clip im Composer, der diesen Loop spielt
            });
            // der Creator zeigt die Aufnahme einer Sample-Schicht (Stufe 11a): hier wird sie dekodiert
            connect(m_instrumentEditor, &InstrumentEditor::sampleVerlangt, this, [this](const QString& nennung) {
                IAudioEngine* engine = services().tryResolve<IAudioEngine>();
                const QString datei = sampleDatei(nennung);
                if (engine != nullptr && !datei.isEmpty())
                    merkeSampleDaten(nennung, kSampleRate, engine->decodeStereo(datei, kSampleRate, kKlangMaxSekunden));
            });
            // „Record..." im Creator (Stufe 11b): die Engine, wie sie beim Oeffnen des Fensters ist
            m_instrumentEditor->setEngineGeber([this] { return services().tryResolve<IAudioEngine>(); });
            // Ctrl+Z / Ctrl+Y im Fenster „Instruments" (Z1 a, S115): derselbe Undo wie Edit → Undo
            connect(m_instrumentEditor, &InstrumentEditor::rueckgaengigVerlangt, this, [this](bool wiederholen) {
                if (auto* bus = services().tryResolve<ICommandBus>())
                    (void)(wiederholen ? bus->redo() : bus->undo());
            });
        }
        m_instrumentEditor->setMarkierterClip(markierterClip());
        if (!m_instrumentEditor->zeige(m_loop, muster)) return false;
        if (zeigen)
        {
            m_instrumentEditor->show();
            m_instrumentEditor->raise();
            m_instrumentEditor->activateWindow();
        }
        zeigeStand();
        return true;
    }
    if (m_editor == nullptr)
    {
        m_editor = new DrumEditor(this);
        m_editor->setKitKlaenge(kitKlaenge());
        connect(m_editor, &DrumEditor::bearbeitet, this, &LoopPanel::bearbeite);
        connect(m_editor, &DrumEditor::spielenVerlangt, this, [this](bool spielen) {
            if (spielen && m_editor != nullptr)
            {
                // B4 b: Play pattern beginnt an der Abspielstelle des Editors
                if (spieleMuster(m_editor->muster())) (void)m_spieler.springe(m_editor->raster()->abspielstelle());
            }
            else
                halte();
        });
        // B4 b: ein Klick in die Taktzeile — spielt das Muster allein, springt es
        connect(m_editor, &DrumEditor::abspielstelleGewaehlt, this, [this](int schritt) {
            if (m_editor != nullptr && m_spieler.spielt() && m_solo == m_editor->muster()) (void)m_spieler.springe(schritt);
        });
        // der Klang einer Spur (Stufe 11c, Schritt 2): Play im Feld hinter „…", das Fenster
        // Instruments, der Creator und „Record sample" — dieselben Wege wie beim Instrument-Editor
        connect(m_editor, &DrumEditor::spurTonVerlangt, this,
                [this](const QString& basisName, int spur) { (void)spieleSpur(basisName, spur); });
        // ▶ und Play all im Fenster Kits (Stufe 21c)
        connect(m_editor, &DrumEditor::kitAnspielenVerlangt, this,
                [this](const std::vector<Spur>& spuren) { (void)spieleSchlaege(spuren); });
        connect(m_editor, &DrumEditor::instrumentTonVerlangt, this,
                [this](const QString& nennung, int hoehe) { (void)spieleInstrumentTon(nennung, hoehe); });
        connect(m_editor, &DrumEditor::instrumentVorhoerenVerlangt, this,
                [this](const QString& nennung, const Muster& noten, int klingtMs) {
                    (void)spieleCreatorNoten(nennung, noten, klingtMs);
                });
        connect(m_editor, &DrumEditor::klaengeGeaendert, this, [this] {
            if (m_vorhoeren == 0) m_spieler.setzeLoop(spielFassung());
            meldeStand();
        });
        connect(m_editor, &DrumEditor::sampleVerlangt, this, [this](const QString& nennung) {
            IAudioEngine* engine = services().tryResolve<IAudioEngine>();
            const QString datei = sampleDatei(nennung);
            if (engine != nullptr && !datei.isEmpty())
                merkeSampleDaten(nennung, kSampleRate, engine->decodeStereo(datei, kSampleRate, kKlangMaxSekunden));
        });
        m_editor->setEngineGeber([this] { return services().tryResolve<IAudioEngine>(); });
        connect(m_editor, &DrumEditor::rueckgaengigVerlangt, this, [this](bool wiederholen) {
            if (auto* bus = services().tryResolve<ICommandBus>()) (void)(wiederholen ? bus->redo() : bus->undo());
        });
    }
    m_editor->setMarkierterClip(markierterClip());
    if (!m_editor->zeige(m_loop, muster)) return false;
    if (zeigen)
    {
        m_editor->show();
        m_editor->raise();
        m_editor->activateWindow();
    }
    zeigeStand();
    return true;
}

void LoopPanel::meldeSong()
{
    IEventBus* bus = eventBus();
    // ein Loop ohne Datei (noch nie gespeichert) meldet sich nicht: ein Song ohne
    // Datei heisst fuer Player-Panel und Tag View „kein Titel“
    const bool meldet =
        m_geladen && !m_datei.isEmpty() && m_spieler.spielt() && m_solo.isEmpty() && m_vorhoeren == 0;
    if (meldet)
    {
        const IAudioEngine* engine = services().tryResolve<IAudioEngine>();
        const TrackInfo song = loopAlsSong(m_loop, m_datei, engine != nullptr ? engine->getSampleRate() : 0);
        // ein neuer Song: zum ersten Mal, oder Tags, Datei oder Laenge haben sich geaendert
        const bool neu = !m_meldetSong || song.filePath != m_gemeldet.filePath || song.title != m_gemeldet.title ||
                         song.artist != m_gemeldet.artist || song.album != m_gemeldet.album ||
                         song.lyricsPlain != m_gemeldet.lyricsPlain || song.durationMs != m_gemeldet.durationMs;
        if (neu)
        {
            const bool warSchon = m_meldetSong;
            m_meldetSong = true;
            m_gemeldet = song;
            meldeLaufendenLoop(&m_gemeldet);
            if (bus != nullptr)
            {
                TrackChangedEvent ereignis;
                ereignis.track = song;
                bus->publish(ereignis);
                if (!warSchon)
                {
                    PlaybackStateEvent zustand;
                    zustand.state = PlaybackState::Playing;
                    zustand.previousState = PlaybackState::Paused;
                    bus->publish(zustand);
                }
            }
            MultiEffectVisualizer::notifyPlayState(true);
            m_gemeldeteStelle = -1;
        }
        // die Stelle im Durchlauf: an den Renderer in jedem Takt, an den Bus etwa
        // zehnmal je Sekunde und nach einem Sprung — wie der Composer
        const double msJeSchritt =
            m_loop.durchlaufSchritte() > 0 ? m_loop.durchlaufMs() / m_loop.durchlaufSchritte() : 0.0;
        const int stelle = static_cast<int>(std::lround(m_spieler.stelleSchritte() * msJeSchritt));
        MultiEffectVisualizer::notifyPlayPosition(stelle);
        ++m_stelleTakte;
        if (bus != nullptr && (m_gemeldeteStelle < 0 || m_stelleTakte >= 3 || std::abs(stelle - m_gemeldeteStelle) > 500))
        {
            m_stelleTakte = 0;
            m_gemeldeteStelle = stelle;
            PlaybackPositionEvent ort;
            ort.positionMs = stelle;
            ort.durationMs = song.durationMs;
            ort.progress = song.durationMs > 0 ? static_cast<float>(stelle) / static_cast<float>(song.durationMs) : 0.0f;
            bus->publish(ort);
        }
        return;
    }
    if (!m_meldetSong) return;
    m_meldetSong = false;
    meldeLaufendenLoop(nullptr);
    // hat der Composer den Ton uebernommen, meldet er seinen Song selbst
    if (composerFuehrtTon()) return;
    // sonst gilt wieder der Song des Players — wie nach dem Composer (Sitzung::gibTonFrei)
    IAudioPlayer* player = services().tryResolve<IAudioPlayer>();
    const bool spielt = player != nullptr && player->isPlaying();
    if (bus != nullptr)
    {
        if (player != nullptr && player->hasTrack())
        {
            TrackChangedEvent ereignis;
            ereignis.track = player->currentTrack();
            ereignis.playlistIndex = player->playlistIndex();
            bus->publish(ereignis);
            PlaybackPositionEvent ort;
            ort.positionMs = player->positionMs();
            ort.durationMs = player->durationMs();
            ort.progress = player->positionFraction();
            bus->publish(ort);
        }
        else
            bus->publish(TrackChangedEvent{});  // kein Titel: das Player-Panel ruht
        PlaybackStateEvent zustand;
        zustand.state = spielt ? PlaybackState::Playing : PlaybackState::Paused;
        zustand.previousState = PlaybackState::Playing;
        bus->publish(zustand);
    }
    if (player != nullptr && player->hasTrack()) MultiEffectVisualizer::notifyPlayPosition(player->positionMs());
    MultiEffectVisualizer::notifyPlayState(spielt);
}

void LoopPanel::beimTakt()
{
    m_spieler.takt();
    // ein angespielter Ton, dessen Klang gerade fertig wurde: jetzt laeuft seine Zeit (TV6 a)
    if (m_vorhoeren != 0 && m_vorhoerDauerMs > 0 && !m_spieler.wartet())
    {
        beendeVorhoerenNach(m_vorhoerDauerMs);
        m_vorhoerDauerMs = 0;
    }
    meldeSong();
    // der Composer kann den Ton uebernommen haben: dann spielt auch kein Muster mehr allein
    if (!m_spieler.spielt() && (!m_solo.isEmpty() || m_vorhoeren != 0))
    {
        m_solo.clear();
        m_vorhoeren = 0;
        m_spieler.setzeLoop(m_loop);
    }
    m_spitze = std::max(m_spitze, m_spieler.nimmSpitze());
    pruefeFreeze();  // Stufe 30g
    if (isVisible() || (m_editor != nullptr && m_editor->isVisible()) ||
        (m_instrumentEditor != nullptr && m_instrumentEditor->isVisible()) ||
        (m_effekte != nullptr && m_effekte->isVisible()))
        zeigeStand();
}

void LoopPanel::zeigeDatei()
{
    m_speichern->setEnabled(m_geladen && (ungespeichert() || m_datei.isEmpty()));
    m_speichernUnter->setEnabled(m_geladen);
    m_tags->setEnabled(m_geladen);
    m_bpm->setEnabled(m_geladen);
    meldeStand();  // jede Aenderung an Loop oder Datei kommt hier vorbei
    m_dateiIcon->setVisible(m_geladen && !m_dateiIcon->pixmap().isNull());
    if (!m_geladen)
    {
        m_dateiZeile->setText(tr("No loop loaded"));
        m_dateiZeile->setToolTip({});
        m_info->clear();
        return;
    }
    // der Stern wie im Composer: es gibt Ungespeichertes
    m_dateiZeile->setText((m_datei.isEmpty() ? tr("untitled") : QFileInfo(m_datei).fileName()) +
                          (ungespeichert() ? QStringLiteral(" *") : QString()));
    m_dateiZeile->setToolTip(m_datei.isEmpty() ? QString() : QDir::toNativeSeparators(m_datei));
    {
        const QSignalBlocker ruhe(m_bpm);
        m_bpm->setValue(m_loop.bpm);
    }
    {
        const QSignalBlocker ruhe(m_limiter);
        m_limiter->setChecked(m_loop.limiter);
    }
    m_info->setText(tr("%1 BPM · channels: %2 · bars: %3 · %4 s")
                        .arg(m_loop.bpm, 0, 'f', m_loop.bpm == std::floor(m_loop.bpm) ? 0 : 1)
                        .arg(m_loop.kanaele.size())
                        .arg(m_loop.durchlaufTakte())
                        .arg(m_loop.durchlaufMs() / 1000.0, 0, 'f', 1));
}

std::optional<ClipOrt> LoopPanel::markierterClip() const
{
    const int kanal = m_achse->gewaehlterKanal();
    const LoopClip* clip = m_achse->auswahl();
    if (kanal < 0 || clip == nullptr) return std::nullopt;
    return ClipOrt{kanal, clip->startTakt};
}

void LoopPanel::zeigeClip()
{
    // E1 a: der markierte Clip ist der Bezugsclip der Editoren — sie zeigen seinen Schalter,
    // und ein Muster, das allein spielt, spielt mit seinen Effekten
    if (m_editor != nullptr) m_editor->setMarkierterClip(markierterClip());
    if (m_instrumentEditor != nullptr) m_instrumentEditor->setMarkierterClip(markierterClip());
    if (!m_solo.isEmpty() && m_vorhoeren == 0) m_spieler.setzeLoop(spielFassung());
    // das Fenster der Effekte folgt den Marken — und jeder neuen Fassung
    if (m_effekte != nullptr) m_effekte->zeige(m_achse->loop(), m_achse->markierte());
    const LoopClip* clip = m_achse->auswahl();
    if (clip == nullptr)
    {
        // sagt auch, wie man mehrere markiert — sonst findet es niemand
        m_clipZeile->setText(m_geladen ? tr("Right-click a channel to add a clip · Ctrl+click or drag a frame on "
                                            "empty space to select several clips")
                                       : QString());
        return;
    }
    if (const auto zahl = m_achse->markierte().size(); zahl > 1)
    {
        int mitEffekt = 0;
        for (const ClipOrt& ort : m_achse->markierte())
        {
            const int platz = findeClip(m_achse->loop(), ort);
            if (platz >= 0 && m_achse->loop().kanaele[static_cast<std::size_t>(ort.kanal)]
                                  .clips[static_cast<std::size_t>(platz)]
                                  .effekte.einerAn())
                ++mitEffekt;
        }
        m_clipZeile->setText(tr("%1 clips selected · %2 with effects · drag moves them together, Del deletes, "
                                "Ctrl+D duplicates")
                                 .arg(zahl)
                                 .arg(mitEffekt));
        return;
    }
    const Loop& gezeigt = m_achse->loop();
    const Muster* basis = gezeigt.findeMuster(clip->muster);
    const int takte = basis != nullptr ? basis->takte : 0;
    m_clipZeile->setText(tr("Clip: %1 · bars %2 to %3 · %4 × %5 bars · base used by %6 clips")
                             .arg(clip->muster)
                             .arg(clip->startTakt + 1)
                             .arg(clip->startTakt + gezeigt.clipTakte(*clip))
                             .arg(clip->wiederholungen)
                             .arg(takte)
                             .arg(basisNutzer(gezeigt, clip->muster)));
}

void LoopPanel::zeigeStand()
{
    // ein angespielter Ton zaehlt nicht als Spielen: Play bleibt Play, keine Marke laeuft
    const bool spielt = m_spieler.spielt() && m_vorhoeren == 0;
    const bool darf = m_spieler.darfSpielen();
    // die Stelle im Arrangement (Schritte), -1 = keine: das eigene Play — oder der
    // Composer, der diesen Loop gerade spielt (S127, Entscheide Patrik W1 a, W2 a)
    double arrangement = -1.0;
    // spielt der Composer diesen Loop mehrfach zugleich, die Stellen der leiseren Clips
    // (S130, Entscheid Patrik W8 a) — blassere Marken; der lauteste fuehrt
    std::vector<double> neben;
    if (spielt && m_solo.isEmpty())
        arrangement = m_spieler.stelleSchritte();
    else if (!spielt && m_geladen && !m_datei.isEmpty())
    {
        const std::vector<double> stellen = composerLoopStellenMs(m_datei);
        if (!stellen.empty()) arrangement = m_loop.schritteVonMs(stellen.front());
        for (std::size_t i = 1; i < stellen.size(); ++i) neben.push_back(m_loop.schritteVonMs(stellen[i]));
    }
    // TV5 a: was gerechnet wird, steht an der Stelle von „Bar · Step"
    const bool bereitetVor = spielt && m_solo.isEmpty() && m_spieler.wartet();
    const QString vorbereitet = QStringLiteral("%1 / %2").arg(m_spieler.vorbereitet()).arg(m_spieler.vorzubereiten());
    // Stufe 30g (FZ4 a D): was gerade eingefroren wird
    const QString friertText = freezeHinweis();
    setzeText(m_stelle, [&]() -> QString {
        if (bereitetVor) return tr("Preparing sounds… %1").arg(vorbereitet);
        if (arrangement < 0.0) return friertText;
        const int schritt = static_cast<int>(arrangement);
        const QString stelle =
            tr("Bar %1 · Step %2").arg(schritt / m_loop.schritteJeTakt() + 1).arg(schritt % m_loop.schritteJeTakt() + 1);
        const QString mitToenen =
            spielt && m_spieler.bereitet() ? stelle + tr(" · updating sounds %1").arg(vorbereitet) : stelle;
        return friertText.isEmpty() ? mitToenen : mitToenen + QStringLiteral(" · ") + friertText;
    }());
    const bool akzent = bereitetVor || (spielt && m_spieler.bereitet()) || !friertText.isEmpty();
    if (m_stelle->property("akzent").toBool() != akzent)
    {
        m_stelle->setProperty("akzent", akzent);
        m_stelle->setStyleSheet(akzent ? QStringLiteral("color: palette(highlight);") : QString());
    }
    const QString knopf = spielt ? tr("Stop") : tr("Play");
    if (m_play->text() != knopf) m_play->setText(knopf);
    m_play->setEnabled(m_geladen && darf);
    m_play->setToolTip(darf ? tr("Play the loop from the start; the player pauses meanwhile")
                            : tr("The Composer plays the audio — start it in the Composer panel"));
    m_hinweis->setVisible(!darf);
    m_fit->setEnabled(m_achse->zoomVerstellt());

    // die Abspielmarke; mit Scroll with playhead folgt ihr der Ausschnitt (W3 a), in
    // der Art, die fuer den Composer eingestellt ist
    using Mitlaufen = lumi::ui::Achse::Mitlaufen;
    const Mitlaufen art = !m_mitlaufen->isChecked()                  ? Mitlaufen::Aus
                          : mitlaufenSeitenweise()                 ? Mitlaufen::Seitenweise
                                                                   : Mitlaufen::Stetig;
    // spielt ein Muster allein, laeuft die Marke im Editor — im Arrangement gibt es dafuer keine Stelle
    m_achse->setMitlaufen(arrangement >= 0.0 ? art : Mitlaufen::Aus);
    m_achse->setSpielt(arrangement >= 0.0);
    if (arrangement >= 0.0) m_achse->setStelleSchritte(arrangement);
    m_achse->setNebenStellenSchritte(neben);
    // die Kaskade in die Editoren: allein gespielt die Stelle im Muster, sonst die
    // Stelle des Arrangements in einem Clip dieses Musters (W1 a, Zusatz Patrik) —
    // liegt das Muster dort mehrfach, die des zuletzt begonnenen Clips (W8-2 a)
    const auto imMuster = [&](const QString& muster, bool allein) {
        if (allein) return m_spieler.stelleSchritte();
        return arrangement >= 0.0 ? m_loop.stelleImMuster(arrangement, muster) : -1.0;
    };
    // ... und blasser die uebrigen: die frueher begonnenen Clips dieses Musters an derselben
    // Stelle (W8-2 a) und alle Clips dieses Musters an den Stellen der leiseren Composer-Clips (W8 a)
    const auto nebenImMuster = [&](const QString& muster, bool allein) {
        std::vector<double> imClip;
        if (allein) return imClip;
        if (arrangement >= 0.0)
        {
            const std::vector<double> hier = m_loop.stellenImMuster(arrangement, muster);
            if (hier.size() > 1) imClip.insert(imClip.end(), hier.begin() + 1, hier.end());
        }
        for (const double n : neben)
        {
            const std::vector<double> dort = m_loop.stellenImMuster(n, muster);
            imClip.insert(imClip.end(), dort.begin(), dort.end());
        }
        return imClip;
    };
    if (m_effekte != nullptr) m_effekte->setSpitze(spielt ? static_cast<double>(m_spitze) : -1.0);
    if (m_editor != nullptr)
    {
        const bool allein = spielt && !m_solo.isEmpty() && m_solo == m_editor->muster();
        m_editor->setSpielt(allein, m_spieler.stelleSchritte());
        m_editor->zeigeMarke(imMuster(m_editor->muster(), allein), art, nebenImMuster(m_editor->muster(), allein));
        m_editor->setSpitze(spielt ? static_cast<double>(m_spitze) : -1.0);
        // warum Play pattern still ist — oder noch (TV5 a, D; Patrik S142: wie im Instrument-Editor)
        const bool tonWartet = m_spieler.wartet() && m_vorhoeren != 0;
        const bool musterWartet = m_spieler.wartet() && m_solo == m_editor->muster();
        m_editor->setHinweis(!darf          ? tr("The Composer plays the audio — Play pattern is silent")
                             : tonWartet    ? tr("Preparing sound…")
                             : musterWartet ? tr("Preparing sounds… %1").arg(vorbereitet)
                             : allein && m_spieler.bereitet() ? tr("Updating sounds %1").arg(vorbereitet)
                                                              : QString());
    }
    if (m_instrumentEditor != nullptr)
    {
        const bool allein = spielt && !m_solo.isEmpty() && m_solo == m_instrumentEditor->muster();
        m_instrumentEditor->setSpielt(allein, m_spieler.stelleSchritte());
        m_instrumentEditor->zeigeMarke(imMuster(m_instrumentEditor->muster(), allein), art,
                                       nebenImMuster(m_instrumentEditor->muster(), allein));
        m_instrumentEditor->setSpitze(spielt ? static_cast<double>(m_spitze) : -1.0);
        // warum ein Klick auf eine Taste gerade nichts hoeren laesst — oder noch nicht (TV5 a, D; TV6 a)
        const bool tonWartet = m_spieler.wartet() && m_vorhoeren != 0;
        const bool musterWartet = m_spieler.wartet() && m_solo == m_instrumentEditor->muster();
        m_instrumentEditor->setHinweis(!darf          ? tr("The Composer plays the audio — keys and Play pattern are silent")
                                       : tonWartet    ? tr("Preparing sound…")
                                       : musterWartet ? tr("Preparing sounds… %1").arg(vorbereitet)
                                       : allein && m_spieler.bereitet() ? tr("Updating sounds %1").arg(vorbereitet)
                                       : spielt       ? tr("Keys are silent while the loop plays")
                                                      : QString());
    }

    QStringList zeilen = m_ladeBericht;
    zeilen << m_spieler.bericht();
    zeilen << m_freezeBericht;
    const QString text = zeilen.join(QLatin1Char('\n'));
    setzeText(m_bericht, text);
    m_bericht->setVisible(!text.isEmpty());
}

// --- Freeze (Stufe 30g) -------------------------------------------------------

int LoopPanel::freezeRate()
{
    const IAudioEngine* engine = services().tryResolve<IAudioEngine>();
    return engine != nullptr && engine->getSampleRate() > 0 ? engine->getSampleRate() : 44100;
}

bool LoopPanel::friereEin(bool einfrieren)
{
    if (!m_geladen) return false;
    const std::vector<ClipOrt> orte = m_achse->markierte();
    if (orte.empty()) return false;
    Loop neu = m_loop;
    std::vector<ClipOrt> zuRechnen;
    if (einfrieren)
        zuRechnen = setzeFrozenMarken(neu, orte, freezeRate(), saitenAusInstrumenten());
    else
        loeseFrozenMarken(neu, orte);
    if (neu == m_loop) return false;
    const bool mehrere = orte.size() > 1;
    bearbeite(neu, einfrieren ? (mehrere ? tr("Freeze clips") : tr("Freeze clip"))
                              : (mehrere ? tr("Unfreeze clips") : tr("Unfreeze clip")));
    starteFreeze(std::move(zuRechnen));  // FZ4 a C: gleich, nicht erst beim Speichern
    zeigeFreezeMarken();
    return true;
}

void LoopPanel::starteFreeze(std::vector<ClipOrt> orte)
{
    if (orte.empty()) return;
    if (m_freeze != nullptr)
    {
        for (const ClipOrt& ort : orte)
            if (std::find(m_freezeWarte.begin(), m_freezeWarte.end(), ort) == m_freezeWarte.end())
                m_freezeWarte.push_back(ort);
        return;
    }
    IAudioEngine* engine = services().tryResolve<IAudioEngine>();
    // dekodiert wird ueber die Engine (TV3 a); ohne Engine bleiben Samples still
    Dekodierer dekodierer = [engine](const QString& datei, int rate, double maxSekunden) {
        return engine != nullptr ? engine->decodeStereo(datei, rate, maxSekunden) : std::vector<float>{};
    };
    Loop gerichtet = m_loop;
    gerichtet.richte();
    m_freezeBericht.clear();
    m_freeze = std::make_unique<FreezeAuftrag>(std::move(gerichtet), std::move(orte), freezeRate(), std::move(dekodierer));
}

void LoopPanel::pruefeFreeze()
{
    if (m_freeze == nullptr) return;
    if (!m_freeze->beendet())
    {
        zeigeFreezeMarken();  // „computing… n %"
        return;
    }
    if (const std::optional<FreezeErgebnis> ergebnis = m_freeze->ergebnis())
    {
        for (const QString& zeile : ergebnis->bericht)
            if (!m_freezeBericht.contains(zeile)) m_freezeBericht << zeile;
        m_freezeMisslungen.insert(m_freezeMisslungen.end(), ergebnis->fehlgeschlagen.begin(),
                                  ergebnis->fehlgeschlagen.end());
    }
    m_freeze.reset();
    // der Spieler plant von selbst neu (`frozenFassung`), der Composer ebenso
    std::vector<ClipOrt> naechste;
    naechste.swap(m_freezeWarte);
    starteFreeze(std::move(naechste));
    bestimmeFreeze(false);
    zeigeStand();
}

void LoopPanel::warteAufFreeze()
{
    while (m_freeze != nullptr)
    {
        m_freeze->warte();
        pruefeFreeze();
    }
}

QString LoopPanel::freezeHinweis() const
{
    if (m_freeze == nullptr) return {};
    const FreezeStand stand = m_freeze->stand();
    return tr("Freezing clip %1 of %2 … %3 %")
        .arg(std::min(stand.clip + 1, stand.clips))
        .arg(stand.clips)
        .arg(std::lround(100.0 * stand.anteil));
}

void LoopPanel::bestimmeFreeze(bool nachAenderung)
{
    if (nachAenderung) m_freezeMisslungen.clear();
    m_freezeZustand.clear();
    std::vector<ClipOrt> fehlend;
    if (m_geladen)
    {
        const int rate = freezeRate();
        const SaitenVon saiten = saitenAusInstrumenten();
        for (std::size_t k = 0; k < m_loop.kanaele.size(); ++k)
            for (const LoopClip& clip : m_loop.kanaele[k].clips)
            {
                if (clip.frozen.isEmpty()) continue;
                const ClipOrt ort{static_cast<int>(k), clip.startTakt};
                const FrozenZustand zustand = frozenZustand(m_loop, ort, rate, saiten);
                m_freezeZustand[{ort.kanal, ort.startTakt}] = zustand;
                if (zustand == FrozenZustand::Fehlt) fehlend.push_back(ort);
            }
    }
    // F5 a: unveraendert, aber die Datei fehlt — neu rechnen (nicht, was schon dran ist,
    // und nicht, was gerade misslang)
    std::erase_if(fehlend, [this](const ClipOrt& ort) {
        return std::find(m_freezeMisslungen.begin(), m_freezeMisslungen.end(), ort) != m_freezeMisslungen.end();
    });
    if (m_freeze != nullptr)
        std::erase_if(fehlend, [this](const ClipOrt& ort) {
            const std::vector<ClipOrt>& laufend = m_freeze->orte();
            return std::find(laufend.begin(), laufend.end(), ort) != laufend.end();
        });
    starteFreeze(std::move(fehlend));
    zeigeFreezeMarken();
}

void LoopPanel::zeigeFreezeMarken()
{
    std::map<std::pair<int, int>, FreezeMarke> marken;
    for (const auto& [ort, zustand] : m_freezeZustand) marken[ort].zustand = zustand;
    if (m_freeze != nullptr)
    {
        const FreezeStand stand = m_freeze->stand();
        const std::vector<ClipOrt>& orte = m_freeze->orte();
        for (std::size_t i = 0; i < orte.size(); ++i)
        {
            if (static_cast<int>(i) < stand.clip) continue;  // schon fertig
            const auto ort = marken.find({orte[i].kanal, orte[i].startTakt});
            if (ort == marken.end()) continue;  // inzwischen geloest
            ort->second.rechnet = true;
            ort->second.anteil = static_cast<int>(i) == stand.clip ? stand.anteil : 0.0;
        }
    }
    m_achse->setzeFreezeMarken(std::move(marken));
}

}  // namespace lumi::loop
