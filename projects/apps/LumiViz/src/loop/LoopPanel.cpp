/**
 * @file LoopPanel.cpp
 * @brief Loop: das Panel — das Arrangement eines Loops (Stufe 6a)
 */

#include "loop/LoopPanel.hpp"

#include "UI/widgets/Achse.hpp"
#include "audio/IAudioEngine.hpp"
#include "audio/IAudioPlayer.hpp"
#include "composer/AppAnbindung.hpp"
#include "loop/Bearbeiten.hpp"
#include "loop/ClipEffektFenster.hpp"
#include "loop/DrumEditor.hpp"
#include "loop/InstrumentEditor.hpp"
#include "loop/Kit.hpp"
#include "loop/Offen.hpp"
#include "loop/Taktachse.hpp"
#include "services/ICommandBus.hpp"
#include "services/ServiceContainer.hpp"
#include "services/events/UIEvents.hpp"

#include <DockWidget.h>

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
      m_spieler([] { return lumi::composer::fuehrtTon(); })
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
    if (dialog.exec() != QDialog::Accepted) return;
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
    m_tags = new QPushButton(tr("Tags..."), this);
    m_tags->setObjectName(QStringLiteral("loopTags"));
    m_tags->setToolTip(tr("Title, artist and the other tags of the loop — what the player shows "
                          "while the loop runs as a clip in the Composer"));
    dateiReihe->addWidget(m_dateiZeile, 1);
    dateiReihe->addWidget(m_tags);
    dateiReihe->addWidget(m_neu);
    dateiReihe->addWidget(m_laden);
    dateiReihe->addWidget(m_speichern);
    dateiReihe->addWidget(m_speichernUnter);
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
    connect(m_achse, &Taktachse::editorVerlangt, this, [this](const QString& muster) { (void)oeffneEditor(muster); });
    connect(m_achse, &Taktachse::effekteVerlangt, this, [this] { (void)oeffneEffekte(); });
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
    QSettings().setValue(QLatin1String(kLetzteDatei), pfad);
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
    if (!m_geladen || pfad.isEmpty() || !speichereLoop(m_loop, pfad)) return false;
    m_datei = pfad;
    m_gespeichert = m_loop;
    QSettings().setValue(QLatin1String(kLetzteDatei), pfad);
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
    Loop allein = m_loop;
    allein.muster = {*solo};
    allein.kanaele = {Kanal{QStringLiteral("Solo"), true, {LoopClip{m_solo, 0, 1}}}};
    return allein;
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
    zeigeStand();
    return spielt;
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

bool LoopPanel::spieleInstrumentTon(const QString& nennung, int hoehe)
{
    if (!m_geladen || nennung.isEmpty()) return false;
    Muster ton;
    ton.art = MusterArt::Instrument;
    ton.instrument = nennung;
    return spieleVor(ton, hoehe);
}

bool LoopPanel::spieleVor(const Muster& vorlage, int hoehe)
{
    if (hoehe < kHoeheMin || hoehe > kHoeheMax) return false;
    // spielt der Loop oder ein Muster, hoert man es schon — das Anspielen wuerde es unterbrechen
    if (m_spieler.spielt() && m_vorhoeren == 0) return false;
    IAudioEngine* engine = services().tryResolve<IAudioEngine>();
    IAudioPlayer* player = services().tryResolve<IAudioPlayer>();
    if (engine == nullptr || player == nullptr) return false;

    // ein Loop aus einem Muster mit dieser einen Note, einen Schlag lang
    Muster ton = vorlage;
    ton.name = QStringLiteral("Preview");
    ton.takte = kTakteMin;
    ton.stumm = false;
    ton.noten = {{0, m_loop.schritteJeSchlag, hoehe, 1.0}};
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
    // nach dem Ton und seinem Ausklang haelt der Spieler wieder — ausser ein neuerer Ton laeuft
    const int dauerMs = allein.notenMs(ton.noten.front()) + 600;
    QTimer::singleShot(dauerMs, this, [this, nummer] {
        if (m_vorhoeren == nummer) beendeVorhoeren();
    });
    zeigeStand();
    return true;
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
                    (void)spieleMuster(m_instrumentEditor->muster());
                else
                    halte();
            });
            connect(m_instrumentEditor, &InstrumentEditor::tonVerlangt, this, [this](int hoehe) {
                if (m_instrumentEditor != nullptr) (void)spieleTon(m_instrumentEditor->muster(), hoehe);
            });
            // der Instrument-Creator (Stufe 10b): er spielt sein Instrument an, auch ungespeichert
            connect(m_instrumentEditor, &InstrumentEditor::instrumentTonVerlangt, this,
                    [this](const QString& nennung, int hoehe) { (void)spieleInstrumentTon(nennung, hoehe); });
            // eine Instrument-Datei ist neu geschrieben: wer sie spielt, rechnet ihre Toene neu
            connect(m_instrumentEditor, &InstrumentEditor::instrumentGespeichert, this, [this] {
                if (m_vorhoeren == 0) m_spieler.setzeLoop(spielFassung());
                meldeStand();  // auch ein Clip im Composer, der diesen Loop spielt
            });
        }
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
                (void)spieleMuster(m_editor->muster());
            else
                halte();
        });
    }
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

void LoopPanel::beimTakt()
{
    m_spieler.takt();
    // der Composer kann den Ton uebernommen haben: dann spielt auch kein Muster mehr allein
    if (!m_spieler.spielt() && (!m_solo.isEmpty() || m_vorhoeren != 0))
    {
        m_solo.clear();
        m_vorhoeren = 0;
        m_spieler.setzeLoop(m_loop);
    }
    m_spitze = std::max(m_spitze, m_spieler.nimmSpitze());
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
    m_info->setText(tr("%1 BPM · channels: %2 · bars: %3 · %4 s")
                        .arg(m_loop.bpm, 0, 'f', m_loop.bpm == std::floor(m_loop.bpm) ? 0 : 1)
                        .arg(m_loop.kanaele.size())
                        .arg(m_loop.durchlaufTakte())
                        .arg(m_loop.durchlaufMs() / 1000.0, 0, 'f', 1));
}

void LoopPanel::zeigeClip()
{
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
    setzeText(m_stelle, [&]() -> QString {
        if (!spielt) return {};
        const int schritt = static_cast<int>(m_spieler.stelleSchritte());
        return tr("Bar %1 · Step %2").arg(schritt / m_loop.schritteJeTakt() + 1).arg(schritt % m_loop.schritteJeTakt() + 1);
    }());
    const QString knopf = spielt ? tr("Stop") : tr("Play");
    if (m_play->text() != knopf) m_play->setText(knopf);
    m_play->setEnabled(m_geladen && darf);
    m_play->setToolTip(darf ? tr("Play the loop from the start; the player pauses meanwhile")
                            : tr("The Composer plays the audio — start it in the Composer panel"));
    m_hinweis->setVisible(!darf);
    m_fit->setEnabled(m_achse->zoomVerstellt());

    // die Abspielmarke; hineingezoomt blaettert der Ausschnitt mit ihr weiter
    m_achse->setMitlaufen(spielt ? lumi::ui::Achse::Mitlaufen::Seitenweise : lumi::ui::Achse::Mitlaufen::Aus);
    // spielt ein Muster allein, laeuft die Marke im Editor — im Arrangement gibt es dafuer keine Stelle
    m_achse->setSpielt(spielt && m_solo.isEmpty());
    if (spielt && m_solo.isEmpty()) m_achse->setStelleSchritte(m_spieler.stelleSchritte());
    if (m_effekte != nullptr) m_effekte->setSpitze(spielt ? static_cast<double>(m_spitze) : -1.0);
    if (m_editor != nullptr)
    {
        m_editor->setSpielt(spielt && !m_solo.isEmpty() && m_solo == m_editor->muster(), m_spieler.stelleSchritte());
        m_editor->setSpitze(spielt ? static_cast<double>(m_spitze) : -1.0);
    }
    if (m_instrumentEditor != nullptr)
    {
        m_instrumentEditor->setSpielt(spielt && !m_solo.isEmpty() && m_solo == m_instrumentEditor->muster(),
                                      m_spieler.stelleSchritte());
        m_instrumentEditor->setSpitze(spielt ? static_cast<double>(m_spitze) : -1.0);
        // warum ein Klick auf eine Taste gerade nichts hoeren laesst
        m_instrumentEditor->setHinweis(!darf     ? tr("The Composer plays the audio — keys and Play pattern are silent")
                                       : spielt ? tr("Keys are silent while the loop plays")
                                                : QString());
    }

    QStringList zeilen = m_ladeBericht;
    zeilen << m_spieler.bericht();
    const QString text = zeilen.join(QLatin1Char('\n'));
    setzeText(m_bericht, text);
    m_bericht->setVisible(!text.isEmpty());
}

}  // namespace lumi::loop
