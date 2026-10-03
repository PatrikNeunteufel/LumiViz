/**
 ****************************************************************************************
 * @file   test_PaletteUi.cpp
 * @brief  UI-Tests der Effekt-Palette, des Kettenbaum-Drops und des Skript-Editors
 *         (Sichttest-Protokoll S79/S80, Teile A und C) — offscreen, ohne Maus
 *
 * @author Patrik Neunteufel
 * @date   September 2026
 *
 * Die Widgets werden mit `Qt::WA_DontShowOnScreen` „gezeigt": Layout, Fokus und
 * Ereignisse laufen wie im Programm, es erscheint aber kein Fenster. Drag & Drop
 * wird als Ereignisfolge DragEnter → DragMove → Drop an den Viewport geschickt —
 * so, wie Qt sie beim echten Ziehen auch liefert.
 *
 * QSettings laufen unter eigenem Organisations-/Programmnamen und werden vor und
 * nach jedem Fall geleert: Favoriten und „Zuletzt" des Nutzers bleiben unberuehrt.
 *
 * Screenshots: `LUMIVIZ_UI_SCHNAPPSCHUSS=<ordner>` legt dort PNGs der Widgets ab
 * (Palette gefiltert, Detailzeile, Editor mit Shape-Code) — zum Ansehen, nicht
 * zum Vergleichen. Ohne die Variable entsteht nichts.
 ****************************************************************************************
 */

#include <doctest.h>

#include "UI/dialogs/LyricsAlternativesDialog.hpp"
#include "UI/panels/AudioBrowserPanel.hpp"
#include "UI/panels/EelScriptEditing.hpp"
#include "UI/panels/EffectPalette.hpp"
#include "UI/panels/EffectPaletteWindow.hpp"
#include "UI/panels/ImportBrowserPanel.hpp"
#include "UI/PresetListen.hpp"
#include "UI/panels/KaraokePanel.hpp"
#include "UI/panels/LyricsSyncPanel.hpp"
#include "audio/SongtextQuelle.hpp"
#include "UI/panels/MultiEffectPanel.hpp"
#include "UI/panels/PlayerPanel.hpp"
#include "UI/panels/PlaylistPanel.hpp"
#include "UI/panels/TagViewPanel.hpp"
#include "UI/widgets/PresetTypeIcons.hpp"
#include "UI/widgets/PresetZeile.hpp"
#include "audio/Wiederaufnahme.hpp"
#include "UI/widgets/AudioDateien.hpp"
#include "UI/widgets/DateiDrag.hpp"
#include "audio/IAudioPlayer.hpp"
#include "audio/Playlist.hpp"
#include "services/EventBus.hpp"
#include "services/ServiceContainer.hpp"
#include "services/ShortcutRegistry.hpp"
#include "services/events/UIEvents.hpp"
#include "visualizers/MultiEffectVisualizer.hpp"
#include "visualizers/multieffect/ChainSerializer.hpp"
#include "visualizers/multieffect/NodePresetStore.hpp"

#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QTimer>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFontComboBox>
#include <QTemporaryDir>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QHeaderView>
#include <QKeyEvent>
#include <QRegularExpression>
#include <QScrollBar>
#include <QBuffer>
#include <QElapsedTimer>
#include <QImage>
#include <QLabel>
#include <QSet>
#include <QThread>
#include <QLineEdit>
#include <QListWidget>
#include <QMimeData>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSettings>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QWheelEvent>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextLayout>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItemIterator>
#include <QUrl>
#include <QVBoxLayout>

#include <QAbstractButton>
#include <QMutex>

#include <filesystem>
#include <memory>
#include <optional>
#include <set>
#include <variant>

using lumi::ui::EffectPaletteWindow;
using lumi::ui::Origin;
using lumi::ui::Role;
using lumi::ui::effectPalette;

namespace
{

/// QApplication fuer diesen Fall (die GL-Tests erzeugen je Fall eine eigene
/// QGuiApplication; es kann immer nur eine geben) + abgeschottete QSettings.
class UiUmgebung
{
public:
    UiUmgebung()
    {
        if (QCoreApplication::instance() == nullptr)
            m_app = std::make_unique<QApplication>(m_argc, m_argv);
        m_org = QCoreApplication::organizationName();
        m_name = QCoreApplication::applicationName();
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("PaletteUi"));
        QSettings().clear();
    }
    ~UiUmgebung()
    {
        QSettings().clear();
        QCoreApplication::setOrganizationName(m_org);
        QCoreApplication::setApplicationName(m_name);
    }
    UiUmgebung(const UiUmgebung&) = delete;
    UiUmgebung& operator=(const UiUmgebung&) = delete;

private:
    int m_argc = 1;
    char m_arg0[10] = "UnitTests";
    char* m_argv[2] = {m_arg0, nullptr};
    std::unique_ptr<QApplication> m_app;
    QString m_org;
    QString m_name;
};

/// Ordner fuer Screenshots, leer = keine.
QString schnappschussOrdner()
{
    const QString d = qEnvironmentVariable("LUMIVIZ_UI_SCHNAPPSCHUSS");
    if (!d.isEmpty()) QDir().mkpath(d);
    return d;
}

void schnappschuss(QWidget& w, const char* name)
{
    const QString d = schnappschussOrdner();
    if (d.isEmpty()) return;
    w.grab().save(d + QLatin1Char('/') + QString::fromUtf8(name) + QStringLiteral(".png"));
}

void zeigen(QWidget& w)
{
    w.setAttribute(Qt::WA_DontShowOnScreen);
    w.show();
    QCoreApplication::processEvents();
}

// --- Palette -----------------------------------------------------------------

QLineEdit* suche(EffectPaletteWindow& w) { return w.findChild<QLineEdit*>(); }
QTreeWidget* baum(EffectPaletteWindow& w) { return w.findChild<QTreeWidget*>(); }

QToolButton* chip(EffectPaletteWindow& w, const QString& text)
{
    for (QToolButton* b : w.findChildren<QToolButton*>())
        if (b->isCheckable() && b->text() == text) return b;
    return nullptr;
}

QPushButton* knopf(EffectPaletteWindow& w, const QString& text)
{
    for (QPushButton* b : w.findChildren<QPushButton*>())
        if (b->text() == text) return b;
    return nullptr;
}

/// Sichtbare Paletten-Indizes AUSSERHALB von Favoriten/Zuletzt.
std::set<int> sichtbareEintraege(EffectPaletteWindow& w)
{
    std::set<int> out;
    QTreeWidget* t = baum(w);
    for (int s = 2; s < t->topLevelItemCount(); ++s)  // 0 = ★, 1 = Zuletzt
    {
        QTreeWidgetItem* sec = t->topLevelItem(s);
        if (sec->isHidden()) continue;
        for (int c = 0; c < sec->childCount(); ++c)
            if (!sec->child(c)->isHidden())
                out.insert(sec->child(c)->data(0, Qt::UserRole).toInt());
    }
    return out;
}

int kinderSichtbar(EffectPaletteWindow& w, int section)
{
    QTreeWidgetItem* sec = baum(w)->topLevelItem(section);
    int n = 0;
    for (int c = 0; c < sec->childCount(); ++c) n += sec->child(c)->isHidden() ? 0 : 1;
    return n;
}

void tippe(QWidget* ziel, int key)
{
    QKeyEvent druck(QEvent::KeyPress, key, Qt::NoModifier);
    QCoreApplication::sendEvent(ziel, &druck);
}

int indexVon(const char* name)
{
    const auto& p = effectPalette();
    for (size_t i = 0; i < p.size(); ++i)
        if (!p[i].isHeader() && QString::fromUtf8(p[i].name) == QString::fromUtf8(name))
            return static_cast<int>(i);
    return -1;
}

void waehle(EffectPaletteWindow& w, int paletteIndex)
{
    for (QTreeWidgetItemIterator it(baum(w)); *it != nullptr; ++it)
        if ((*it)->data(0, Qt::UserRole).toInt() == paletteIndex)
        {
            baum(w)->setCurrentItem(*it);
            return;
        }
    FAIL("Eintrag nicht im Baum: " << paletteIndex);
}

// --- Kettenbaum ----------------------------------------------------------------

struct DropErgebnis
{
    bool angenommen = false;          ///< Drop-Ereignis akzeptiert
    std::optional<int> paletteIndex;  ///< onPaletteDrop gerufen
    QTreeWidgetItem* ziel = nullptr;
    ChainDrop wo = ChainDrop::Viewport;
};

/// Kettenbaum wie im Panel: gleiche Drag-Konfiguration, drei Knoten, der
/// mittlere eine Liste. Hoch genug, dass unter den Zeilen Leerflaeche bleibt.
struct Kettenbaum
{
    ChainTreeWidget tree;
    QTreeWidgetItem* a = nullptr;
    QTreeWidgetItem* liste = nullptr;
    QTreeWidgetItem* c = nullptr;
    DropErgebnis letzter;

    Kettenbaum()
    {
        tree.setColumnCount(4);
        tree.configureDragDrop();
        const auto flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable |
                           Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
        a = new QTreeWidgetItem(&tree, {QStringLiteral("Clear")});
        liste = new QTreeWidgetItem(&tree, {QStringLiteral("Effect List")});
        c = new QTreeWidgetItem(&tree, {QStringLiteral("Milkdrop")});
        for (QTreeWidgetItem* i : {a, liste, c}) i->setFlags(flags);
        tree.onPaletteDrop = [this](int idx, QTreeWidgetItem* t, ChainDrop w) {
            letzter.paletteIndex = idx;
            letzter.ziel = t;
            letzter.wo = w;
        };
        tree.resize(420, 400);
        zeigen(tree);
    }

    /// Zieht einen Paletteneintrag an `pos` (Viewport-Koordinaten) und laesst los.
    DropErgebnis ziehePalette(int paletteIndex, QPoint pos, const QString& mime = {})
    {
        letzter = {};
        QMimeData data;
        data.setData(mime.isEmpty() ? QString::fromUtf8(lumi::ui::kPaletteMime) : mime,
                     QByteArray::number(paletteIndex));
        QWidget* vp = tree.viewport();
        QDragEnterEvent enter(pos, Qt::CopyAction, &data, Qt::LeftButton, Qt::NoModifier);
        QCoreApplication::sendEvent(vp, &enter);
        QDragMoveEvent move(pos, Qt::CopyAction, &data, Qt::LeftButton, Qt::NoModifier);
        QCoreApplication::sendEvent(vp, &move);
        if (!move.isAccepted()) return letzter;  // Qt liefert dann auch kein Drop
        QDropEvent drop(QPointF(pos), Qt::CopyAction, &data, Qt::LeftButton, Qt::NoModifier);
        QCoreApplication::sendEvent(vp, &drop);
        letzter.angenommen = drop.isAccepted();
        return letzter;
    }
};

}  // namespace

// =============================================================================
// Teil A — Palette
// =============================================================================

TEST_CASE("PaletteUi A2: Suchwort + Enter fuegt den ersten Treffer ein")
{
    UiUmgebung env;
    EffectPaletteWindow w;
    w.setInsertEnabled(true);
    zeigen(w);
    QList<int> spy;  // Qt Test ist nicht gelinkt — mitschreiben statt QSignalSpy
    QObject::connect(&w, &EffectPaletteWindow::addRequested,
                     [&spy](int i) { spy.append(i); });

    suche(w)->setText(QStringLiteral("fract"));
    tippe(suche(w), Qt::Key_Return);

    REQUIRE(spy.count() == 1);
    const int idx = spy.at(0);
    CHECK(QString::fromUtf8(effectPalette()[static_cast<size_t>(idx)].name)
              .contains(QStringLiteral("Fract"), Qt::CaseInsensitive));
    schnappschuss(w, "A2_palette_fract");
}

TEST_CASE("PaletteUi A3: Pfeil runter, anderer Treffer, Enter nimmt genau den")
{
    UiUmgebung env;
    EffectPaletteWindow w;
    w.setInsertEnabled(true);
    zeigen(w);
    QList<int> spy;  // Qt Test ist nicht gelinkt — mitschreiben statt QSignalSpy
    QObject::connect(&w, &EffectPaletteWindow::addRequested,
                     [&spy](int i) { spy.append(i); });

    suche(w)->setText(QStringLiteral("fract"));
    const int erster = baum(w)->currentItem()->data(0, Qt::UserRole).toInt();
    tippe(suche(w), Qt::Key_Down);  // Fokus in den Baum
    tippe(baum(w), Qt::Key_Down);   // naechste sichtbare Zeile
    const int zweiter = baum(w)->currentItem()->data(0, Qt::UserRole).toInt();
    REQUIRE(zweiter >= 0);
    CHECK(zweiter != erster);
    tippe(baum(w), Qt::Key_Return);

    REQUIRE(spy.count() == 1);
    CHECK(spy.at(0) == zweiter);
}

TEST_CASE("PaletteUi A4-A6: Chips — Herkunft, dann UND Rolle, dann ODER innerhalb der Zeile")
{
    UiUmgebung env;
    EffectPaletteWindow w;
    zeigen(w);
    const auto& p = effectPalette();
    const std::set<int> alle = sichtbareEintraege(w);

    QToolButton* milk = chip(w, QStringLiteral("MilkDrop"));
    QToolButton* avs = chip(w, QStringLiteral("AVS"));
    QToolButton* draws = chip(w, lumi::ui::roleText(Role::Render));
    REQUIRE(milk != nullptr);
    REQUIRE(avs != nullptr);
    REQUIRE(draws != nullptr);

    milk->click();  // A4
    const std::set<int> nurMilk = sichtbareEintraege(w);
    REQUIRE(!nurMilk.empty());
    CHECK(nurMilk.size() < alle.size());
    for (int i : nurMilk) CHECK(p[static_cast<size_t>(i)].origin == Origin::MilkDrop);

    draws->click();  // A5: UND zwischen den Zeilen
    const std::set<int> milkDraws = sichtbareEintraege(w);
    CHECK(milkDraws.size() <= nurMilk.size());
    for (int i : milkDraws)
    {
        CHECK(p[static_cast<size_t>(i)].origin == Origin::MilkDrop);
        CHECK(p[static_cast<size_t>(i)].role == Role::Render);
    }

    avs->click();  // A6: ODER innerhalb der Herkunftszeile
    const std::set<int> beide = sichtbareEintraege(w);
    CHECK(beide.size() > milkDraws.size());
    for (int i : beide)
    {
        const Origin o = p[static_cast<size_t>(i)].origin;
        CHECK((o == Origin::MilkDrop || o == Origin::Avs));
        CHECK(p[static_cast<size_t>(i)].role == Role::Render);
    }
    schnappschuss(w, "A6_palette_chips");
}

TEST_CASE("PaletteUi A7: Detailzeile nennt Name, Herkunft/Rolle und Formelfelder")
{
    UiUmgebung env;
    EffectPaletteWindow w;
    w.setInsertEnabled(true);
    zeigen(w);
    const int idx = indexVon("Clear");
    REQUIRE(idx >= 0);
    waehle(w, idx);

    QString detail;
    for (QLabel* l : w.findChildren<QLabel*>())
        if (l->text().contains(QStringLiteral("<b>Clear</b>"))) detail = l->text();
    REQUIRE(!detail.isEmpty());
    CHECK(detail.contains(QStringLiteral("AVS")));
    CHECK(detail.contains(QStringLiteral("Formula fields")));
    CHECK(detail.contains(QStringLiteral("module variable")));
    schnappschuss(w, "A7_palette_detail_clear");
}

TEST_CASE("PaletteUi A8/A9: Favorit und Zuletzt ueberleben einen Neustart des Fensters")
{
    UiUmgebung env;
    const int clear = indexVon("Clear");
    const int fractal = indexVon("Fractal 2D");
    REQUIRE(clear >= 0);
    REQUIRE(fractal >= 0);
    {
        EffectPaletteWindow w;
        zeigen(w);
        waehle(w, clear);
        chip(w, QString::fromUtf8("★"))->click();  // der ★-Knopf ist auch checkable
        CHECK(kinderSichtbar(w, 0) == 1);           // A8
        w.noteUsed(fractal);
        CHECK(kinderSichtbar(w, 1) == 1);
    }
    EffectPaletteWindow neu;  // liest QSettings wie nach einem App-Neustart
    zeigen(neu);
    REQUIRE(kinderSichtbar(neu, 0) == 1);
    CHECK(baum(neu)->topLevelItem(0)->child(0)->data(0, Qt::UserRole).toInt() == clear);
    REQUIRE(kinderSichtbar(neu, 1) == 1);
    CHECK(baum(neu)->topLevelItem(1)->child(0)->data(0, Qt::UserRole).toInt() == fractal);
}

TEST_CASE("PaletteUi A1/A17: Add-Knopf meldet, Fenster bleibt offen; ohne Multi Effect gesperrt")
{
    UiUmgebung env;
    EffectPaletteWindow w;
    zeigen(w);
    QList<int> spy;  // Qt Test ist nicht gelinkt — mitschreiben statt QSignalSpy
    QObject::connect(&w, &EffectPaletteWindow::addRequested,
                     [&spy](int i) { spy.append(i); });
    waehle(w, indexVon("Clear"));

    QPushButton* add = knopf(w, QStringLiteral("Add"));
    REQUIRE(add != nullptr);
    CHECK_FALSE(add->isEnabled());  // noch kein Multi-Effect-Visualizer
    w.setInsertEnabled(true);
    CHECK(add->isEnabled());
    add->click();
    add->click();
    CHECK(spy.count() == 2);
    CHECK(w.isVisible());  // A17: bleibt offen
    knopf(w, QStringLiteral("Close"))->click();
    CHECK_FALSE(w.isVisible());
}

TEST_CASE("PaletteUi A18: Fenstergroesse wird gemerkt")
{
    UiUmgebung env;
    {
        EffectPaletteWindow w;
        zeigen(w);
        w.resize(510, 470);
        QCoreApplication::processEvents();
        w.hide();  // hideEvent speichert die Geometrie
    }
    EffectPaletteWindow neu;
    CHECK(neu.size() == QSize(510, 470));
}

// =============================================================================
// Teil A — Drop in den Kettenbaum
// =============================================================================

TEST_CASE("PaletteUi A12: Drop AUF eine Zeile -> OnItem mit diesem Ziel")
{
    UiUmgebung env;
    Kettenbaum k;
    const QPoint mitte = k.tree.visualItemRect(k.liste).center();
    const DropErgebnis r = k.ziehePalette(7, mitte);
    CHECK(r.angenommen);
    REQUIRE(r.paletteIndex.has_value());
    CHECK(*r.paletteIndex == 7);
    CHECK(r.ziel == k.liste);
    CHECK(r.wo == ChainDrop::OnItem);
}

TEST_CASE("PaletteUi A13: Drop an die Kante zwischen zwei Zeilen -> Above/Below")
{
    UiUmgebung env;
    Kettenbaum k;
    const QRect rc = k.tree.visualItemRect(k.c);
    const DropErgebnis r = k.ziehePalette(7, QPoint(rc.center().x(), rc.top() + 1));
    CHECK(r.angenommen);
    REQUIRE(r.paletteIndex.has_value());
    const bool zwischen = (r.ziel == k.c && r.wo == ChainDrop::Above) ||
                          (r.ziel == k.liste && r.wo == ChainDrop::Below);
    CHECK(zwischen);
}

TEST_CASE("PaletteUi A14: Drop auf die Leerflaeche weit unter der letzten Zeile -> Ende")
{
    UiUmgebung env;
    Kettenbaum k;
    const QRect letzte = k.tree.visualItemRect(k.c);
    const QPoint weitUnten(letzte.center().x(), k.tree.viewport()->height() - 10);
    REQUIRE(weitUnten.y() > letzte.bottom() + 100);  // wirklich freie Flaeche

    const DropErgebnis r = k.ziehePalette(7, weitUnten);
    CHECK(r.angenommen);
    REQUIRE(r.paletteIndex.has_value());
    CHECK(r.ziel == nullptr);
    CHECK(r.wo == ChainDrop::Viewport);
}

TEST_CASE("PaletteUi A16: fremde Daten (kein Paletteneintrag) werden abgewiesen")
{
    UiUmgebung env;
    Kettenbaum k;
    const QPoint mitte = k.tree.visualItemRect(k.a).center();
    const DropErgebnis r = k.ziehePalette(7, mitte, QStringLiteral("text/plain"));
    CHECK_FALSE(r.angenommen);
    CHECK_FALSE(r.paletteIndex.has_value());
}

// =============================================================================
// Teil A — Ende zu Ende am ECHTEN Panel (Kette waechst wirklich)
// =============================================================================
//
// Der Nachbau oben prueft nur den Baum. Hier laeuft dieselbe Ereignisfolge
// gegen ein vollstaendiges MultiEffectPanel mit Host — so, wie es in der App
// haengt (Host kommt ueber das VisualizerChangedEvent). Gezaehlt wird an der
// KETTE, nicht am Baum.

namespace
{
using lumi::multieffect::ChainNode;

struct PanelUmgebung
{
    ServiceContainer services;
    MultiEffectVisualizer vis;
    QMutex mutex;
    std::unique_ptr<MultiEffectPanel> panel;
    ChainTreeWidget* tree = nullptr;

    PanelUmgebung()
    {
        services.registerInstance<IEventBus>(std::make_unique<EventBus>());
        // Startkette: Clear · Effect List (mit einem Clear) · Milkdrop
        ChainNode root;
        root.params = lumi::multieffect::ListParams{};
        root.children.push_back(knoten("Clear"));
        ChainNode liste = knoten("Effect List");
        liste.children.push_back(knoten("Clear"));
        root.children.push_back(std::move(liste));
        root.children.push_back(knoten("Milkdrop (Preset-Pipeline)"));
        vis.setChain(std::move(root));

        panel = std::make_unique<MultiEffectPanel>(services);
        panel->resize(460, 900);
        zeigen(*panel);
        services.tryResolve<IEventBus>()->publish(VisualizerChangedEvent(
            "multieffect", "Multi Effect", static_cast<IVisualizer*>(&vis), &mutex));
        QCoreApplication::processEvents();
        // ChainTreeWidget hat kein Q_OBJECT -> findChild geht nicht, also casten
        for (QTreeWidget* t : panel->findChildren<QTreeWidget*>())
            if (auto* c = dynamic_cast<ChainTreeWidget*>(t)) tree = c;
    }

    static ChainNode knoten(const char* paletteName)
    {
        const int i = indexVon(paletteName);
        REQUIRE(i >= 0);
        ChainNode n;
        n.params = effectPalette()[static_cast<size_t>(i)].make();
        return n;
    }

    [[nodiscard]] const ChainNode& root() const { return vis.chain(); }

    [[nodiscard]] QTreeWidgetItem* oben(int row) const { return tree->topLevelItem(row); }

    /// Drop wie oben, danach die Warteschlange leeren (das Panel fuegt
    /// verzoegert ein, erst nach dem dropEvent des Baums).
    bool ziehe(const char* paletteName, QPoint pos)
    {
        QMimeData data;
        data.setData(QString::fromUtf8(lumi::ui::kPaletteMime),
                     QByteArray::number(indexVon(paletteName)));
        QWidget* vp = tree->viewport();
        QDragEnterEvent enter(pos, Qt::CopyAction, &data, Qt::LeftButton, Qt::NoModifier);
        QCoreApplication::sendEvent(vp, &enter);
        QDragMoveEvent move(pos, Qt::CopyAction, &data, Qt::LeftButton, Qt::NoModifier);
        QCoreApplication::sendEvent(vp, &move);
        if (!move.isAccepted()) return false;
        QDropEvent drop(QPointF(pos), Qt::CopyAction, &data, Qt::LeftButton, Qt::NoModifier);
        QCoreApplication::sendEvent(vp, &drop);
        QCoreApplication::processEvents();
        QCoreApplication::processEvents();
        return drop.isAccepted();
    }
};

template <typename T>
bool ist(const ChainNode& n)
{
    return std::holds_alternative<T>(n.params);
}

size_t wellenIn(const ChainNode& n)
{
    const auto* mp = std::get_if<lumi::multieffect::MilkdropNodeParams>(&n.params);
    return mp == nullptr ? 0 : mp->preset.waves.size();
}
}  // namespace

TEST_CASE("PaletteUi Panel: Startzustand — Host haengt, Baum zeigt die drei Knoten")
{
    UiUmgebung env;
    PanelUmgebung p;
    REQUIRE(p.tree != nullptr);
    CHECK(p.tree->isEnabled());
    CHECK(p.tree->topLevelItemCount() == 3);
    CHECK(p.root().children.size() == 3);
    schnappschuss(*p.panel, "A_panel_start");
}

// S89, KE2 Schritt 2 (Entscheid K2 a): dasselbe Panel an den Baum der
// Karaoke-Ebene gebunden — der Baum zeigt deren Knoten, ein Edit trifft die
// Ebene und nicht die Kette, und zurueck steht wieder die Kette da.
TEST_CASE("PaletteUi Panel S89: an den Karaoke-Baum gebunden trifft ein Edit die Ebene, nicht die Kette")
{
    UiUmgebung env;
    PanelUmgebung p;
    REQUIRE(p.tree != nullptr);
    CHECK(p.panel->baum() == MultiEffectPanel::Baum::Kette);

    p.panel->setBaum(MultiEffectPanel::Baum::Karaoke);
    QCoreApplication::processEvents();
    REQUIRE(p.tree->topLevelItemCount() == 2);  // Lyrics + Trail-Liste
    CHECK(p.oben(1)->childCount() == 3);        // Drift, Fadeout, Lyrics Trail
    CHECK(ist<lumi::multieffect::LyricsParams>(p.vis.karaokeKette().children[0]));

    // Lyrics-Knoten im Baum umbenennen: die Ebene aendert sich, die Kette nicht
    const std::string ketteVorher = p.root().children[0].displayName;
    p.oben(0)->setText(0, QStringLiteral("Meine Zeile"));
    QCoreApplication::processEvents();
    CHECK(p.vis.karaokeKette().children[0].displayName == "Meine Zeile");
    CHECK(p.root().children.size() == 3);
    CHECK(p.root().children[0].displayName == ketteVorher);
    schnappschuss(*p.panel, "S89_panel_karaoke_baum");

    p.panel->setBaum(MultiEffectPanel::Baum::Kette);
    QCoreApplication::processEvents();
    CHECK(p.tree->topLevelItemCount() == 3);
}

// S89, KE2 Schritt 3: das Karaoke-Panel — Hauptschalter, Karaoke-Presets
// (.lvkar) und Reiter „Lyrics" mit dem Editor des Lyrics-Knotens der Ebene.
// Jeder Edit landet in der Ablage; ein Preset stellt den Stand wieder her.
TEST_CASE("Karaoke-Panel S89: Schalter, Editor der Ebene, Ablage und Karaoke-Presets")
{
    UiUmgebung env;
    QTemporaryDir tmp;
    REQUIRE(tmp.isValid());
    const QString ablage = tmp.filePath(QStringLiteral("karaoke/current.lvkar"));
    MultiEffectVisualizer::setKaraokeAblage(ablage);
    MultiEffectVisualizer::setKaraokeEbene(false);
    using lumi::multieffect::LyricsParams;
    {
        ServiceContainer services;
        services.registerInstance<IEventBus>(std::make_unique<EventBus>());
        MultiEffectVisualizer vis;
        QMutex mutex;
        KaraokePanel panel(services);
        panel.resize(520, 1400);
        zeigen(panel);
        services.tryResolve<IEventBus>()->publish(VisualizerChangedEvent(
            "multieffect", "Multi Effect", static_cast<IVisualizer*>(&vis), &mutex));
        QCoreApplication::processEvents();
        QCoreApplication::processEvents();
        const auto lyrics = [&]() -> const LyricsParams& {
            return std::get<LyricsParams>(vis.karaokeKette().children[0].params);
        };

        // --- Hauptschalter: schaltet die Ebene und merkt es sich
        auto* schalter = panel.findChild<QCheckBox*>(QStringLiteral("karaokeSchalter"));
        REQUIRE(schalter != nullptr);
        CHECK_FALSE(schalter->isChecked());
        schalter->setChecked(true);
        CHECK(MultiEffectVisualizer::karaokeEbene());
        CHECK(QSettings().value(QStringLiteral("karaoke/enabled")).toBool());

        // --- Reiter 1: der Editor steht auf dem Lyrics-Knoten der Ebene, ohne Baum
        MultiEffectPanel* editor = nullptr;
        for (MultiEffectPanel* e : panel.findChildren<MultiEffectPanel*>())
            if (e->baum() == MultiEffectPanel::Baum::Karaoke) editor = e;
        REQUIRE(editor != nullptr);
        for (QTreeWidget* t : editor->findChildren<QTreeWidget*>())
            if (dynamic_cast<ChainTreeWidget*>(t) != nullptr) CHECK_FALSE(t->isVisible());
        auto* yShift = editor->findChild<QSpinBox*>(QStringLiteral("yShift"));
        REQUIRE(yShift != nullptr);  // ein Feld des Lyrics-Knotens
        CHECK(editor->findChild<QComboBox*>(QStringLiteral("knotenPreset")) != nullptr);
        schnappschuss(panel, "S89_karaoke_panel");

        // --- ein Edit trifft die Ebene und landet in der Ablage
        const int vorgabe = lyrics().yShift;
        yShift->setValue(vorgabe - 20);
        QCoreApplication::processEvents();
        CHECK(lyrics().yShift == vorgabe - 20);
        editor->speichereKaraokeJetzt();
        REQUIRE(QFile::exists(ablage));
        {
            MultiEffectVisualizer neu;  // laedt die Ablage im Konstruktor
            CHECK(std::get<LyricsParams>(neu.karaokeKette().children[0].params).yShift ==
                  vorgabe - 20);
        }

        // --- Karaoke-Presets (Auftrag Patrik S89): frei waehlbarer Ort, Preset-
        //     Zeile mit laufendem Preset / zuletzt; hier ohne Dialoge
        using lumi::ui::PresetArt;
        using lumi::ui::PresetListen;
        const KaraokePanel::Ziel ziel{&vis, &mutex, services.tryResolve<IEventBus>(), nullptr};
        const QString abend = tmp.filePath(QStringLiteral("woanders/Abend.lvkar"));
        CHECK(vis.karaokeGeaendert());                              // der Edit von eben
        CHECK_FALSE(KaraokePanel::speichereKaraokeNach(ziel, ablage));  // die Ablage ist kein Preset
        REQUIRE(KaraokePanel::speichereKaraokeNach(ziel, abend));   // der Ordner entsteht
        CHECK(QFile::exists(abend));
        CHECK_FALSE(vis.karaokeGeaendert());                        // gespeichert = ungeaendert
        CHECK(QFileInfo(PresetListen::laufendesKaraokePreset()) == QFileInfo(abend));
        CHECK(PresetListen::position(PresetListen::zuletzt(PresetArt::Karaoke), abend) == 0);
        auto* zeile = panel.findChild<lumi::ui::PresetZeile*>();
        REQUIRE(zeile != nullptr);
        QCoreApplication::processEvents();
        CHECK(QFileInfo(zeile->laufend()) == QFileInfo(abend));
        auto* combo = zeile->findChild<QComboBox*>(QStringLiteral("presetZeileCombo"));
        REQUIRE(combo != nullptr);
        CHECK(combo->itemText(0) == QStringLiteral("Abend"));

        // Default (nichts ungespeichert → keine Rueckfrage): Vorgabe, kein laufendes Preset
        panel.setzeVorgabe();
        QCoreApplication::processEvents();
        CHECK(lyrics().yShift == vorgabe);
        CHECK(PresetListen::laufendesKaraokePreset().isEmpty());
        CHECK(zeile->laufend().isEmpty());

        // Laden stellt den Stand her; beide Reiter und die Zeile ziehen nach
        REQUIRE(panel.ladeDatei(abend));
        QCoreApplication::processEvents();
        CHECK(lyrics().yShift == vorgabe - 20);
        CHECK_FALSE(vis.karaokeGeaendert());
        yShift = editor->findChild<QSpinBox*>(QStringLiteral("yShift"));
        REQUIRE(yShift != nullptr);
        CHECK(yShift->value() == vorgabe - 20);
        CHECK(QFileInfo(zeile->laufend()) == QFileInfo(abend));

        // unlesbar / fremd: abgewiesen, die Ebene bleibt
        CHECK_FALSE(KaraokePanel::ladeKaraokeDatei(ziel, tmp.filePath(QStringLiteral("gibtsnicht.lvkar"))));
        CHECK(lyrics().yShift == vorgabe - 20);
        CHECK(QFile::exists(ablage));  // die laufende Ebene bleibt
    }
    MultiEffectVisualizer::setKaraokeEbene(false);
    MultiEffectVisualizer::setKaraokeAblage(QString());
}

// S89, KE3: Reiter „Lyrics Trail" — die Trail-Liste der Ebene als kurze Kette.
// Die Palette zeigt nur Erlaubtes (Rollen Transform und Color, Entscheid K1 a),
// der Lyrics Trail am Ende ist fest, „Trail on" ist das Auge der Liste.
TEST_CASE("Karaoke-Panel S89: Reiter Lyrics Trail — Liste, Paletten-Filter, fester Trail, Trail on/off")
{
    UiUmgebung env;
    MultiEffectVisualizer::setKaraokeAblage(QString());
    using lumi::multieffect::LyricsTrailParams;

    // --- der uebergeordnete Filter der Palette
    {
        EffectPaletteWindow w;
        const std::set<int> alle = sichtbareEintraege(w);
        w.setUebergeordneterFilter(&lumi::ui::karaokeTrailErlaubt, QStringLiteral("Trail"));
        const std::set<int> erlaubt = sichtbareEintraege(w);
        CHECK(erlaubt.size() > 10);
        CHECK(erlaubt.size() < alle.size());
        for (int i : erlaubt)
        {
            const auto& t = effectPalette()[static_cast<size_t>(i)];
            CAPTURE(t.name);
            CHECK((t.role == lumi::ui::Role::Transform || t.role == lumi::ui::Role::Color));
        }
        CHECK(erlaubt.count(indexVon("Blitter Feedback")) == 1);
        CHECK(erlaubt.count(indexVon("Fadeout")) == 1);
        CHECK(erlaubt.count(indexVon("Lyrics")) == 0);
        CHECK(erlaubt.count(indexVon("Lyrics Trail")) == 0);
        CHECK(erlaubt.count(indexVon("Effect List")) == 0);
        // die Chips der Rollen ohne erlaubten Eintrag sind weg
        w.show();
        CHECK_FALSE(chip(w, lumi::ui::roleText(lumi::ui::Role::Render))->isVisible());
        CHECK(chip(w, lumi::ui::roleText(lumi::ui::Role::Color))->isVisible());
    }

    ServiceContainer services;
    services.registerInstance<IEventBus>(std::make_unique<EventBus>());
    MultiEffectVisualizer vis;
    QMutex mutex;
    KaraokePanel panel(services);
    panel.resize(520, 1000);
    zeigen(panel);
    services.tryResolve<IEventBus>()->publish(VisualizerChangedEvent(
        "multieffect", "Multi Effect", static_cast<IVisualizer*>(&vis), &mutex));
    QCoreApplication::processEvents();
    auto* reiter = panel.findChild<QTabWidget*>();
    REQUIRE(reiter != nullptr);
    REQUIRE(reiter->count() == 2);
    reiter->setCurrentIndex(1);
    QCoreApplication::processEvents();

    MultiEffectPanel* trail = nullptr;
    for (MultiEffectPanel* e : panel.findChildren<MultiEffectPanel*>())
        if (e->baum() == MultiEffectPanel::Baum::KaraokeTrail) trail = e;
    REQUIRE(trail != nullptr);
    ChainTreeWidget* tree = nullptr;
    for (QTreeWidget* t : trail->findChildren<QTreeWidget*>())
        if (auto* c = dynamic_cast<ChainTreeWidget*>(t)) tree = c;
    REQUIRE(tree != nullptr);
    CHECK(tree->isVisible());
    const auto kinder = [&]() -> const std::vector<ChainNode>& {
        return vis.karaokeKette().children[1].children;
    };
    const auto istTrail = [](const ChainNode& n) {
        return std::holds_alternative<LyricsTrailParams>(n.params);
    };
    // Vorgabe: Drift, Fadeout, Lyrics Trail — der Baum zeigt genau diese drei
    REQUIRE(kinder().size() == 3);
    CHECK(tree->topLevelItemCount() == 3);
    schnappschuss(panel, "S89_karaoke_trail");

    const auto ziehe = [&](const char* name) {
        QMimeData data;
        data.setData(QString::fromUtf8(lumi::ui::kPaletteMime), QByteArray::number(indexVon(name)));
        const QPoint pos(60, tree->viewport()->height() - 10);  // Leerflaeche = anhaengen
        QWidget* vp = tree->viewport();
        QDragEnterEvent enter(pos, Qt::CopyAction, &data, Qt::LeftButton, Qt::NoModifier);
        QCoreApplication::sendEvent(vp, &enter);
        QDragMoveEvent move(pos, Qt::CopyAction, &data, Qt::LeftButton, Qt::NoModifier);
        QCoreApplication::sendEvent(vp, &move);
        QDropEvent drop(QPointF(pos), Qt::CopyAction, &data, Qt::LeftButton, Qt::NoModifier);
        QCoreApplication::sendEvent(vp, &drop);
        QCoreApplication::processEvents();
        QCoreApplication::processEvents();
    };

    // --- Erlaubtes ans Ende gezogen landet VOR dem festen Lyrics Trail
    ziehe("Invert");
    REQUIRE(kinder().size() == 4);
    CHECK(ist<lumi::multieffect::InvertParams>(kinder()[2]));
    CHECK(istTrail(kinder()[3]));
    CHECK(tree->topLevelItemCount() == 4);

    // --- nicht Erlaubtes wird abgewiesen (auch aus dem Fenster der Kette gezogen)
    ziehe("Lyrics");
    ziehe("Effect List");
    ziehe("Lyrics Trail");
    CHECK(kinder().size() == 4);

    // --- der Lyrics Trail ist nicht entfernbar, ein anderer Knoten schon
    tree->setCurrentItem(tree->topLevelItem(3));
    tree->onDeleteKey();
    QCoreApplication::processEvents();
    CHECK(kinder().size() == 4);
    tree->setCurrentItem(tree->topLevelItem(2));
    tree->onDeleteKey();
    QCoreApplication::processEvents();
    REQUIRE(kinder().size() == 3);
    CHECK(istTrail(kinder()[2]));

    // --- Trail on/off = das Auge der Trail-Liste
    auto* schalter = panel.findChild<QCheckBox*>(QStringLiteral("karaokeTrailSchalter"));
    REQUIRE(schalter != nullptr);
    CHECK(schalter->isChecked());
    schalter->setChecked(false);
    CHECK_FALSE(vis.karaokeKette().children[1].enabled);
    schalter->setChecked(true);
    CHECK(vis.karaokeKette().children[1].enabled);
    // die Zeile (Kind 0) bleibt davon unberuehrt
    CHECK(vis.karaokeKette().children[0].enabled);
}

// S89 (Auftrag Patrik): Favoriten und „zuletzt benutzt" fuer Preset-Dateien —
// je eine Liste fuer die Kette und fuer Karaoke, in den QSettings.
TEST_CASE("PresetListen S89: Favoriten und die zehn zuletzt benutzten, je Art getrennt")
{
    UiUmgebung env;  // eigene, leere QSettings
    using lumi::ui::PresetArt;
    using L = lumi::ui::PresetListen;
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const auto pfad = [&](int i) { return dir.filePath(QStringLiteral("p%1.lvfx").arg(i)); };

    CHECK(L::favoriten(PresetArt::Kette).isEmpty());
    CHECK(L::zuletzt(PresetArt::Kette).isEmpty());

    // zuletzt: juengstes zuerst, hoechstens zehn, kein Eintrag doppelt
    for (int i = 0; i < 12; ++i) L::merkeBenutzt(PresetArt::Kette, pfad(i));
    QStringList z = L::zuletzt(PresetArt::Kette);
    REQUIRE(z.size() == L::kZuletztMax);
    CHECK(QFileInfo(z.first()) == QFileInfo(pfad(11)));
    CHECK(QFileInfo(z.last()) == QFileInfo(pfad(2)));
    L::merkeBenutzt(PresetArt::Kette, pfad(5));            // schon drin: wandert nach vorn
    L::merkeBenutzt(PresetArt::Kette, pfad(5).toUpper());  // andere Schreibweise = dieselbe Datei
    z = L::zuletzt(PresetArt::Kette);
    CHECK(z.size() == L::kZuletztMax);
    CHECK(L::position(z, pfad(5)) == 0);
    L::merkeBenutzt(PresetArt::Kette, QString());           // leer zaehlt nicht
    CHECK(L::zuletzt(PresetArt::Kette).size() == L::kZuletztMax);

    // Favoriten: setzen, nehmen, kein Doppel, Reihenfolge des Setzens
    L::setzeFavorit(PresetArt::Kette, pfad(3), true);
    L::setzeFavorit(PresetArt::Kette, pfad(1), true);
    L::setzeFavorit(PresetArt::Kette, pfad(3).toUpper(), true);
    CHECK(L::favoriten(PresetArt::Kette).size() == 2);
    CHECK(L::istFavorit(PresetArt::Kette, pfad(3)));
    CHECK(L::position(L::favoriten(PresetArt::Kette), pfad(3)) == 0);
    L::setzeFavorit(PresetArt::Kette, pfad(3), false);
    CHECK_FALSE(L::istFavorit(PresetArt::Kette, pfad(3)));
    CHECK(L::istFavorit(PresetArt::Kette, pfad(1)));

    // die Arten teilen nichts
    CHECK(L::zuletzt(PresetArt::Karaoke).isEmpty());
    L::merkeBenutzt(PresetArt::Karaoke, dir.filePath(QStringLiteral("abend.lvkar")));
    L::setzeFavorit(PresetArt::Karaoke, dir.filePath(QStringLiteral("abend.lvkar")), true);
    CHECK(L::zuletzt(PresetArt::Karaoke).size() == 1);
    CHECK(L::favoriten(PresetArt::Kette).size() == 1);

    // vergiss: aus beiden Listen der Art
    L::vergiss(PresetArt::Kette, pfad(1));
    CHECK_FALSE(L::istFavorit(PresetArt::Kette, pfad(1)));
    CHECK(L::position(L::zuletzt(PresetArt::Kette), pfad(1)) < 0);
    CHECK(L::zuletzt(PresetArt::Karaoke).size() == 1);

    // die Listen ueberleben ein „neues Programm" (QSettings)
    CHECK(QSettings().value(QStringLiteral("presets/chain/recent")).toStringList().size() ==
          L::zuletzt(PresetArt::Kette).size());
}

// S89 (Auftrag Patrik): die Preset-Zeile — laufendes Preset, Favoriten, zuletzt;
// eine Auswahl laedt sofort (1a), das Mausrad laedt nichts, kein Loeschen (2a).
TEST_CASE("PresetZeile S89: laufendes Preset, Favoriten, zuletzt — Auswahl meldet Laden, Rad nicht")
{
    UiUmgebung env;
    using lumi::ui::PresetArt;
    using lumi::ui::PresetListen;
    using lumi::ui::PresetZeile;
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const auto lege = [&](const QString& name) {
        const QString pfad = dir.filePath(name);
        QFile f(pfad);
        REQUIRE(f.open(QIODevice::WriteOnly));
        f.write("{}");
        return pfad;
    };
    const QString eins = lege(QStringLiteral("Eins.lvfx"));
    const QString zwei = lege(QStringLiteral("Zwei.lvfx"));
    const QString weg = dir.filePath(QStringLiteral("Weg.lvfx"));  // gibt es nicht
    PresetListen::merkeBenutzt(PresetArt::Kette, weg);
    PresetListen::merkeBenutzt(PresetArt::Kette, zwei);
    PresetListen::merkeBenutzt(PresetArt::Kette, eins);
    PresetListen::setzeFavorit(PresetArt::Kette, zwei, true);

    PresetZeile zeile(PresetArt::Kette);
    zeigen(zeile);
    auto* combo = zeile.findChild<QComboBox*>(QStringLiteral("presetZeileCombo"));
    auto* stern = zeile.findChild<QToolButton*>(QStringLiteral("presetZeileStern"));
    REQUIRE(combo != nullptr);
    REQUIRE(stern != nullptr);
    QStringList geladen;
    int listen = 0;
    QObject::connect(&zeile, &PresetZeile::ladeDatei, [&](const QString& p) { geladen.append(p); });
    QObject::connect(&zeile, &PresetZeile::listenGeaendert, [&] { ++listen; });

    // Aufbau: laufend · Kopf Favoriten · Zwei · Kopf Zuletzt · Eins · Zwei · Weg
    REQUIRE(combo->count() == 7);
    CHECK(combo->itemText(0) == QStringLiteral("(new effect chain)"));
    CHECK(combo->itemText(2) == QStringLiteral("Zwei"));
    CHECK(combo->itemText(4) == QStringLiteral("Eins"));
    CHECK(combo->itemText(6) == QStringLiteral("Weg"));
    CHECK_FALSE(stern->isEnabled());  // ohne laufendes Preset kein Favorit
    // die Kopfzeilen sind nicht waehlbar
    const auto* modell = qobject_cast<QStandardItemModel*>(combo->model());
    REQUIRE(modell != nullptr);
    CHECK(modell->item(1)->flags() == Qt::NoItemFlags);
    CHECK(modell->item(3)->flags() == Qt::NoItemFlags);

    // laufendes Preset + Stern fuer „geaendert"
    zeile.setLaufend(eins);
    CHECK(combo->itemText(0) == QStringLiteral("Eins"));
    CHECK(stern->isEnabled());
    CHECK_FALSE(stern->isChecked());
    zeile.setGeaendert(true);
    CHECK(combo->itemText(0) == QStringLiteral("Eins *"));
    zeile.setGeaendert(false);

    // Auswahl laedt sofort — und die Zeile zeigt danach wieder das laufende Preset
    emit combo->activated(2);
    REQUIRE(geladen.size() == 1);
    CHECK(QFileInfo(geladen[0]) == QFileInfo(zwei));
    CHECK(combo->currentIndex() == 0);
    // Zeile 0 und Kopfzeilen laden nichts
    emit combo->activated(0);
    emit combo->activated(1);
    CHECK(geladen.size() == 1);

    // das Mausrad laedt nichts und wechselt nichts
    QWheelEvent rad(QPointF(5, 5), combo->mapToGlobal(QPointF(5, 5)), QPoint(), QPoint(0, -120),
                    Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QCoreApplication::sendEvent(combo, &rad);
    CHECK(combo->currentIndex() == 0);
    CHECK(geladen.size() == 1);

    // ein fehlender Eintrag faellt beim Anklicken aus den Listen (kein Laden)
    emit combo->activated(6);
    CHECK(geladen.size() == 1);
    CHECK(PresetListen::position(PresetListen::zuletzt(PresetArt::Kette), weg) < 0);
    CHECK(combo->count() == 6);
    CHECK(listen == 1);

    // ★ macht das laufende Preset zum Favoriten und nimmt es wieder
    stern->setChecked(true);
    CHECK(PresetListen::istFavorit(PresetArt::Kette, eins));
    CHECK(combo->count() == 7);
    stern->setChecked(false);
    CHECK_FALSE(PresetListen::istFavorit(PresetArt::Kette, eins));
    CHECK(listen == 3);

    // Knoepfe melden Load… / Save / Save As…
    int dialoge = 0;
    std::vector<bool> sichern;
    QObject::connect(&zeile, &PresetZeile::ladenDialog, [&] { ++dialoge; });
    QObject::connect(&zeile, &PresetZeile::speichern, [&](bool a) { sichern.push_back(a); });
    zeile.findChild<QPushButton*>(QStringLiteral("presetZeileLaden"))->click();
    zeile.findChild<QPushButton*>(QStringLiteral("presetZeileSpeichern"))->click();
    zeile.findChild<QPushButton*>(QStringLiteral("presetZeileSpeichernAls"))->click();
    CHECK(dialoge == 1);
    CHECK(sichern == std::vector<bool>{false, true});
    schnappschuss(zeile, "S89_preset_zeile");
}

// S89: die Preset-Zeile im Effect-Chain-Panel haengt an denselben Wegen wie
// Menue File und Effect Browser — sie meldet nur, das Fenster laedt/speichert.
TEST_CASE("PaletteUi Panel S89: Preset-Zeile der Kette meldet Laden je Typ, Speichern, zeigt das laufende Preset")
{
    UiUmgebung env;
    PanelUmgebung p;
    auto* zeile = p.panel->findChild<lumi::ui::PresetZeile*>();
    REQUIRE(zeile != nullptr);
    IEventBus* bus = p.services.tryResolve<IEventBus>();
    QStringList gemeldet;
    const auto a1 = bus->subscribeScoped<ImportAvsPresetEvent>([&](const ImportAvsPresetEvent& e) {
        gemeldet.append(QStringLiteral("avs:") + QString::fromStdString(e.path));
    });
    const auto a2 = bus->subscribeScoped<ImportMilkPresetEvent>([&](const ImportMilkPresetEvent& e) {
        gemeldet.append(QStringLiteral("milk:") + QString::fromStdString(e.path));
    });
    const auto a3 = bus->subscribeScoped<LoadEffectChainEvent>([&](const LoadEffectChainEvent& e) {
        gemeldet.append(QStringLiteral("lvfx:") + QString::fromStdString(e.path));
    });
    const auto a4 = bus->subscribeScoped<SaveEffectChainEvent>([&](const SaveEffectChainEvent& e) {
        gemeldet.append(e.saveAs ? QStringLiteral("saveas") : QStringLiteral("save"));
    });

    emit zeile->ladeDatei(QStringLiteral("C:/x/a.avs"));
    emit zeile->ladeDatei(QStringLiteral("C:/x/b.MILK"));
    emit zeile->ladeDatei(QStringLiteral("C:/x/c.lvfx2"));
    emit zeile->ladenDialog();
    emit zeile->speichern(false);
    emit zeile->speichern(true);
    CHECK(gemeldet == QStringList{QStringLiteral("avs:C:/x/a.avs"), QStringLiteral("milk:C:/x/b.MILK"),
                                  QStringLiteral("lvfx:C:/x/c.lvfx2"), QStringLiteral("lvfx:"),
                                  QStringLiteral("save"), QStringLiteral("saveas")});

    // das Fenster meldet das laufende Preset: die Zeile zeigt es
    bus->publish(CurrentPresetChangedEvent{"C:/x/Mein Preset.lvfx"});
    CHECK(QFileInfo(zeile->laufend()).completeBaseName() == QStringLiteral("Mein Preset"));
    bus->publish(CurrentPresetChangedEvent{std::string()});
    CHECK(zeile->laufend().isEmpty());

    // die Reiter des Karaoke-Panels haben KEINE Ketten-Zeile (es hat seine eigene)
    MultiEffectPanel lyrics(p.services, MultiEffectPanel::KaraokeLyrics{});
    CHECK(lyrics.findChild<lumi::ui::PresetZeile*>() == nullptr);
}

// S89, KE4: ein Schalter fuer Panel, Menue und Tastenkuerzel — und die
// Zustandszeile „Preset shows its own lyrics", wenn die Kette gewinnt.
TEST_CASE("Karaoke-Panel S89: gemeinsamer Schalter (Panel/Menue/Kuerzel) und Zustandszeile")
{
    UiUmgebung env;
    MultiEffectVisualizer::setKaraokeAblage(QString());
    MultiEffectVisualizer::setKaraokeEbene(false);
    ServiceContainer services;
    services.registerInstance<IEventBus>(std::make_unique<EventBus>());
    IEventBus* bus = services.tryResolve<IEventBus>();
    std::vector<bool> gemeldet;
    const auto abo = bus->subscribeScoped<KaraokeEnabledChangedEvent>(
        [&](const KaraokeEnabledChangedEvent& e) { gemeldet.push_back(e.enabled); });

    // die Aktion steht in der Tastenkuerzel-Tabelle
    const lumi::services::ShortcutAction* aktion = lumi::services::shortcutAction("view.karaoke");
    REQUIRE(aktion != nullptr);
    CHECK(aktion->wired);
    CHECK_FALSE(aktion->defaultSequence.empty());

    MultiEffectVisualizer vis;
    QMutex mutex;
    KaraokePanel panel(services);
    panel.resize(520, 700);
    zeigen(panel);
    bus->publish(VisualizerChangedEvent("multieffect", "Multi Effect",
                                        static_cast<IVisualizer*>(&vis), &mutex));
    QCoreApplication::processEvents();
    auto* schalter = panel.findChild<QCheckBox*>(QStringLiteral("karaokeSchalter"));
    auto* zustand = panel.findChild<QLabel*>(QStringLiteral("karaokeZustand"));
    REQUIRE(schalter != nullptr);
    REQUIRE(zustand != nullptr);

    // --- von aussen geschaltet (Menue, Kuerzel): Renderer, Einstellung, Haken — einmal gemeldet
    KaraokePanel::setzeKaraokeAn(bus, true);
    CHECK(MultiEffectVisualizer::karaokeEbene());
    CHECK(QSettings().value(QStringLiteral("karaoke/enabled")).toBool());
    CHECK(schalter->isChecked());
    CHECK(gemeldet == std::vector<bool>{true});  // der nachgezogene Haken meldet nicht noch einmal

    // --- im Panel geschaltet: derselbe Weg
    schalter->setChecked(false);
    CHECK_FALSE(MultiEffectVisualizer::karaokeEbene());
    CHECK(gemeldet == std::vector<bool>{true, false});

    // --- Zustandszeile: die Kette zeigt selbst Lyrics und die Ebene ist an
    const auto warteAuf = [&](const QString& text) {
        QElapsedTimer t;
        t.start();
        while (zustand->text() != text && t.elapsed() < 3000)
        {
            QCoreApplication::processEvents();
            QThread::msleep(20);
        }
        return zustand->text() == text;
    };
    const auto setzeKette = [&](bool lyricsAn) {
        ChainNode root;
        root.params = lumi::multieffect::ListParams{};
        ChainNode lyr;
        lyr.params = lumi::multieffect::LyricsParams{};
        lyr.enabled = lyricsAn;
        root.children = {lyr};
        QMutexLocker lock(&mutex);
        vis.setChain(std::move(root));
    };
    const QString hinweis = QStringLiteral("Preset shows its own lyrics");
    setzeKette(true);
    CHECK(zustand->text().isEmpty());            // Ebene aus: kein Hinweis
    schalter->setChecked(true);
    CHECK(warteAuf(hinweis));
    setzeKette(false);                           // Lyrics-Knoten aus: die Ebene uebernimmt
    CHECK(warteAuf(QString()));
    setzeKette(true);
    CHECK(warteAuf(hinweis));
    schalter->setChecked(false);                 // Ebene aus: der Hinweis geht sofort
    CHECK(zustand->text().isEmpty());

    MultiEffectVisualizer::setKaraokeEbene(false);
}

// S89, KE2 Schritt 4: Mischart und Staerke der Ebene standen bis S88 in den
// QSettings — sie wandern einmal in die Ablage, die Schluessel verschwinden.
TEST_CASE("Karaoke S89: alte Mischart-Einstellungen wandern einmal in die Ablage")
{
    UiUmgebung env;  // eigene QSettings
    QTemporaryDir tmp;
    REQUIRE(tmp.isValid());
    const QString ablage = tmp.filePath(QStringLiteral("karaoke/current.lvkar"));
    using lumi::multieffect::LyricsParams;
    using lumi::multieffect::LyricsTrailParams;
    using Teil = MultiEffectVisualizer::KaraokeTeil;
    const auto setzeAlt = [] {
        QSettings s;
        s.setValue(QStringLiteral("karaoke/LineBlend"), 4);
        s.setValue(QStringLiteral("karaoke/LineStrength"), 50);
        s.setValue(QStringLiteral("karaoke/TrailBlend"), 3);
    };

    // nichts Altes da: nichts zu tun
    MultiEffectVisualizer::setKaraokeAblage(ablage);
    CHECK_FALSE(MultiEffectVisualizer::uebernehmeAlteKaraokeMischung());
    CHECK_FALSE(QFile::exists(ablage));

    // alte Werte, noch keine Ablage: sie wird mit ihnen geschrieben
    setzeAlt();
    CHECK(MultiEffectVisualizer::uebernehmeAlteKaraokeMischung());
    REQUIRE(QFile::exists(ablage));
    CHECK_FALSE(QSettings().contains(QStringLiteral("karaoke/LineBlend")));
    CHECK_FALSE(QSettings().contains(QStringLiteral("karaoke/TrailBlend")));
    {
        MultiEffectVisualizer vis;  // laedt die Ablage
        const auto& lp = std::get<LyricsParams>(vis.karaokeKette().children[0].params);
        CHECK(lp.blend == 4);
        CHECK(lp.blendStrength == 50);
        const auto& tp =
            std::get<LyricsTrailParams>(vis.karaokeKette().children[1].children[2].params);
        CHECK(tp.blend == 3);
        CHECK(tp.blendStrength == 100);
    }
    // die prozessweite Vorgabe steht danach wieder auf dem Mitgelieferten
    CHECK(MultiEffectVisualizer::karaokeMischart(Teil::Zeile) == 0);
    CHECK(MultiEffectVisualizer::karaokeMischart(Teil::Trail) == 1);

    // alte Werte, aber die Ablage gibt es schon: sie gilt, die Schluessel gehen trotzdem
    const QByteArray vorher = [&] {
        QFile f(ablage);
        REQUIRE(f.open(QIODevice::ReadOnly));
        return f.readAll();
    }();
    QSettings().setValue(QStringLiteral("karaoke/LineBlend"), 7);
    CHECK_FALSE(MultiEffectVisualizer::uebernehmeAlteKaraokeMischung());
    CHECK_FALSE(QSettings().contains(QStringLiteral("karaoke/LineBlend")));
    QFile f(ablage);
    REQUIRE(f.open(QIODevice::ReadOnly));
    CHECK(f.readAll() == vorher);

    MultiEffectVisualizer::setKaraokeAblage(QString());
}

TEST_CASE("PaletteUi Panel A14: Drop auf die Leerflaeche haengt ans Ende der Kette")
{
    UiUmgebung env;
    PanelUmgebung p;
    REQUIRE(p.tree != nullptr);
    // unterste sichtbare Zeile (der Milkdrop-Knoten hat aufgeklappte Sektionen)
    int unten = 0;
    for (QTreeWidgetItemIterator it(p.tree); *it != nullptr; ++it)
        unten = std::max(unten, p.tree->visualItemRect(*it).bottom());
    const QPoint weitUnten(60, p.tree->viewport()->height() - 10);
    INFO("Viewport-Hoehe " << p.tree->viewport()->height() << ", letzte Zeile unten bei "
                           << unten);
    REQUIRE(weitUnten.y() > unten + 40);

    CHECK(p.ziehe("Clear", weitUnten));
    REQUIRE(p.root().children.size() == 4);
    CHECK(ist<lumi::multieffect::ClearParams>(p.root().children.back()));
}

TEST_CASE("PaletteUi Panel A14b: Custom Shape auf die Leerflaeche landet im Milkdrop-Knoten")
{
    // Befund Patrik S80: nach „Milkdrop einfuegen" eine Shape gezogen — nur
    // direkt unter der letzten Zeile (= Milkdrop-Sektion „Sprites") nahm der
    // Baum sie an, weiter unten auf der Leerflaeche passierte nichts.
    UiUmgebung env;
    PanelUmgebung p;
    REQUIRE(p.tree != nullptr);
    const auto* mp =
        std::get_if<lumi::multieffect::MilkdropNodeParams>(&p.root().children[2].params);
    REQUIRE(mp != nullptr);
    const size_t vorher = mp->preset.shapes.size();
    const QPoint weitUnten(60, p.tree->viewport()->height() - 10);

    CHECK(p.ziehe("Custom Shape", weitUnten));
    CHECK(p.root().children.size() == 3);
    mp = std::get_if<lumi::multieffect::MilkdropNodeParams>(&p.root().children[2].params);
    CHECK(mp->preset.shapes.size() == vorher + 1);
}

TEST_CASE("PaletteUi Panel B1: Eingeklapptes bleibt nach einer Strukturaenderung zu (S82)")
{
    // Befund Patrik S82: „Zu dieser Gruppe wechseln" (= Strukturaenderung)
    // klappte die ganze Kette wieder auf.
    UiUmgebung env;
    PanelUmgebung p;
    REQUIRE(p.tree != nullptr);
    REQUIRE(p.oben(1)->childCount() > 0);
    REQUIRE(p.oben(2)->childCount() > 0);  // Milkdrop: Sektionen
    // eine Milkdrop-Sektion MIT Kindern (Waves traegt immer die Basis-Wave)
    int s = 0;
    while (s < p.oben(2)->childCount() && p.oben(2)->child(s)->childCount() == 0) ++s;
    REQUIRE(s < p.oben(2)->childCount());
    p.oben(1)->setExpanded(false);             // Effect List zu
    p.oben(2)->child(s)->setExpanded(false);   // Milkdrop-Sektion zu
    const QString sektion = p.oben(2)->child(s)->text(0);

    const QPoint weitUnten(60, p.tree->viewport()->height() - 10);
    REQUIRE(p.ziehe("Clear", weitUnten));  // Neuaufbau des Baums
    REQUIRE(p.tree->topLevelItemCount() == 4);

    CHECK_FALSE(p.oben(1)->isExpanded());
    CHECK(p.oben(2)->isExpanded());  // war offen, bleibt offen
    CHECK(p.oben(2)->child(s)->text(0) == sektion);
    CHECK_FALSE(p.oben(2)->child(s)->isExpanded());
}

TEST_CASE("PaletteUi Panel A12: Drop auf die Liste legt IN die Liste")
{
    UiUmgebung env;
    PanelUmgebung p;
    REQUIRE(p.tree != nullptr);
    CHECK(p.ziehe("Clear", p.tree->visualItemRect(p.oben(1)).center()));
    CHECK(p.root().children.size() == 3);
    CHECK(p.root().children[1].children.size() == 2);
}

TEST_CASE("PaletteUi Panel A13: Drop an die Kante zwischen zwei Knoten legt dazwischen")
{
    UiUmgebung env;
    PanelUmgebung p;
    REQUIRE(p.tree != nullptr);
    const QRect zweite = p.tree->visualItemRect(p.oben(1));
    CHECK(p.ziehe("Effect List", QPoint(zweite.center().x(), zweite.top() + 1)));
    REQUIRE(p.root().children.size() == 4);
    // Zwischen Clear (0) und der alten Liste: neue Liste an Position 1, leer
    CHECK(ist<lumi::multieffect::ListParams>(p.root().children[1]));
    CHECK(p.root().children[1].children.empty());
    CHECK(p.root().children[2].children.size() == 1);
}

TEST_CASE("PaletteUi Panel A15: Custom Wave auf den Milkdrop-Knoten wird eine Wave darin")
{
    UiUmgebung env;
    PanelUmgebung p;
    REQUIRE(p.tree != nullptr);
    const size_t vorher = wellenIn(p.root().children[2]);
    CHECK(p.ziehe("Custom Wave", p.tree->visualItemRect(p.oben(2)).center()));
    CHECK(p.root().children.size() == 3);  // kein eigener Kettenknoten
    CHECK(wellenIn(p.root().children[2]) == vorher + 1);
}

TEST_CASE("PaletteUi Panel A16: Custom Wave auf einen Nicht-Milkdrop-Knoten aendert nichts")
{
    UiUmgebung env;
    PanelUmgebung p;
    REQUIRE(p.tree != nullptr);
    const size_t vorher = wellenIn(p.root().children[2]);
    p.ziehe("Custom Wave", p.tree->visualItemRect(p.oben(0)).center());
    CHECK(p.root().children.size() == 3);
    CHECK(wellenIn(p.root().children[2]) == vorher);
    CHECK(ist<lumi::multieffect::ClearParams>(p.root().children[0]));
}

TEST_CASE("PaletteUi Panel A10/A11: Add aus dem Palettenfenster — Liste gewaehlt: hinein; Knoten: darunter")
{
    UiUmgebung env;
    PanelUmgebung p;
    REQUIRE(p.tree != nullptr);
    QAbstractButton* plus = nullptr;
    for (QAbstractButton* b : p.panel->findChildren<QAbstractButton*>())
        if (b->text() == QStringLiteral("+")) plus = b;
    REQUIRE(plus != nullptr);
    plus->click();
    QCoreApplication::processEvents();
    auto* pal = p.panel->findChild<EffectPaletteWindow*>();
    REQUIRE(pal != nullptr);
    pal->setAttribute(Qt::WA_DontShowOnScreen);
    CHECK(pal->isVisible());

    // A10: Liste markiert -> hinein
    p.tree->setCurrentItem(p.oben(1));
    waehle(*pal, indexVon("Clear"));
    knopf(*pal, QStringLiteral("Add"))->click();
    QCoreApplication::processEvents();
    CHECK(p.root().children[1].children.size() == 2);

    // A11: gewoehnlicher Knoten markiert -> darunter
    p.tree->setCurrentItem(p.oben(0));
    waehle(*pal, indexVon("Effect List"));
    knopf(*pal, QStringLiteral("Add"))->click();
    QCoreApplication::processEvents();
    REQUIRE(p.root().children.size() == 4);
    CHECK(ist<lumi::multieffect::ListParams>(p.root().children[1]));
    CHECK(pal->isVisible());  // A17 am echten Panel
    schnappschuss(*p.panel, "A11_panel_nach_add");
}

// =============================================================================
// Teil C — Skript-Editor
// =============================================================================

namespace
{
/// Vordergrundfarbe, die der Highlighter dem Zeichen an `spalte` in `zeile` gab.
QColor farbeBei(QTextDocument& doc, int zeile, int spalte)
{
    const QTextBlock block = doc.findBlockByNumber(zeile);
    for (const QTextLayout::FormatRange& r : block.layout()->formats())
        if (spalte >= r.start && spalte < r.start + r.length)
            return r.format.foreground().color();
    return {};
}
}  // namespace

TEST_CASE("PaletteUi C1: im Shape-Code ist b gefaerbt wie r, g, r2, g2, b2")
{
    UiUmgebung env;
    QTextDocument doc;
    doc.setPlainText(QStringLiteral("r = red;\ng = green;\nb = blue;\nr2 = red;\nb2 = blue;"));
    lumi::scriptedit::EelHighlighter hl(&doc);
    hl.rehighlight();

    const QColor r = farbeBei(doc, 0, 0);
    REQUIRE(r.isValid());
    CHECK(farbeBei(doc, 1, 0) == r);  // g
    CHECK(farbeBei(doc, 2, 0) == r);  // b  <- der Befund S80
    CHECK(farbeBei(doc, 3, 0) == r);  // r2
    CHECK(farbeBei(doc, 4, 0) == r);  // b2

    if (!schnappschussOrdner().isEmpty())
    {
        QPlainTextEdit ed;
        ed.setPlainText(QStringLiteral(
            "x = bass * .5;\ny = (mid + treb) * .25;\nred = mid * bass * 0.2;\n"
            "r = r2 = red;\ng = g2 = green;\nb = b2 = blue;\na = a2 = 1;"));
        QFont mono(QStringLiteral("Consolas"));
        mono.setStyleHint(QFont::Monospace);
        ed.setFont(mono);
        QPalette pal = ed.palette();  // dunkel wie die App, sonst sind die Farben blass
        pal.setColor(QPalette::Base, QColor(0x1E, 0x1E, 0x1E));
        pal.setColor(QPalette::Text, QColor(0xD4, 0xD4, 0xD4));
        ed.setPalette(pal);
        lumi::scriptedit::EelHighlighter hl2(ed.document());
        ed.resize(420, 170);
        zeigen(ed);
        schnappschuss(ed, "C1_editor_shape_code");
    }
}

TEST_CASE("PaletteUi C3: die Skript-Referenz erklaert die Mehrfachzuweisung")
{
    UiUmgebung env;
    const QString html = lumi::scriptedit::builtinsHtml();
    CHECK(html.contains(QStringLiteral("Chained assignment")));
    CHECK(html.contains(QStringLiteral("r = r2 = red;")));
}

// =============================================================================
// Teil D — Knoten-Voreinstellungen im Editor (S83)
// =============================================================================

TEST_CASE("PaletteUi Panel D1: Preset-Combo behaelt die Wahl, Bearbeiten -> (unsaved) – Name")
{
    // Befund Patrik S83: nach der Auswahl sprang die Combo auf „(unchanged)"
    // zurueck (der Editor wird nach dem Laden neu gebaut) — „Delete selected"
    // konnte so nie greifen. Soll: Name bleibt stehen, erste Aenderung zeigt
    // „(unsaved) – Name" (Entscheid a), Loeschen zielt weiter auf die Quelle.
    namespace np = lumi::multieffect::nodepresets;
    UiUmgebung env;
    QTemporaryDir root;
    REQUIRE(root.isValid());
    np::setRootForTesting(root.path());
    struct Zurueck
    {
        ~Zurueck() { np::setRootForTesting(QString()); }
    } zurueck;

    const QString werkDir = QDir(root.path()).filePath(QStringLiteral("asset/clear"));
    REQUIRE(QDir().mkpath(werkDir));
    {
        QFile f(QDir(werkDir).filePath(QStringLiteral("Werk.json")));
        REQUIRE(f.open(QIODevice::WriteOnly));
        f.write(R"({"format":"lumiviz-nodepreset","formatVersion":1,)"
                R"("node":{"type":"clear","blend":2,"color":1057856,"onlyFirst":false}})");
    }
    lumi::multieffect::ClearParams eigen;
    eigen.color = 0x123456;
    eigen.onlyFirst = false;
    QString geschrieben;
    REQUIRE(np::save(QStringLiteral("Eigen"), lumi::multieffect::EffectParams{eigen}, {},
                     &geschrieben));

    PanelUmgebung p;
    REQUIRE(p.tree != nullptr);
    const auto warte = [] {
        for (int i = 0; i < 3; ++i) QCoreApplication::processEvents();
    };
    const auto combo = [&p] { return p.panel->findChild<QComboBox*>(QStringLiteral("knotenPreset")); };
    const auto loeschen = [&p] {
        return p.panel->findChild<QToolButton*>(QStringLiteral("knotenPresetLoeschen"));
    };
    const auto onlyFirst = [&p] { return p.panel->findChild<QCheckBox*>(QStringLiteral("onlyFirst")); };
    const auto clearFarbe = [&p] {
        return std::get<lumi::multieffect::ClearParams>(p.root().children[0].params).color;
    };

    p.tree->setCurrentItem(p.oben(0));  // Clear
    warte();
    REQUIRE(combo() != nullptr);
    CHECK(combo()->currentIndex() == 0);
    CHECK(combo()->currentText() == QStringLiteral("(none)"));
    CHECK_FALSE(loeschen()->isEnabled());

    // Werks-Voreinstellung laden: Name bleibt nach dem Neuaufbau stehen
    combo()->setCurrentIndex(combo()->findText(QStringLiteral("Werk")));
    warte();
    CHECK(clearFarbe() == 1057856u);
    CHECK(combo()->currentText() == QStringLiteral("Werk"));
    CHECK_FALSE(loeschen()->isEnabled());  // mitgeliefert: nicht loeschbar

    // erste Aenderung -> „(unsaved) – Werk"
    REQUIRE(onlyFirst() != nullptr);
    onlyFirst()->setChecked(true);
    warte();
    CHECK(combo()->currentIndex() == 0);
    CHECK(combo()->currentText() == QString::fromUtf8("(unsaved) – Werk"));

    // eigene Voreinstellung: Loeschen aktiv, auch nach einer Aenderung
    combo()->setCurrentIndex(combo()->findText(QStringLiteral("Eigen *")));
    warte();
    CHECK(clearFarbe() == 0x123456u);
    CHECK(combo()->currentText() == QStringLiteral("Eigen *"));
    CHECK(loeschen()->isEnabled());
    onlyFirst()->setChecked(true);
    warte();
    CHECK(combo()->currentText() == QString::fromUtf8("(unsaved) – Eigen"));
    CHECK(loeschen()->isEnabled());

    // Knotenwechsel und zurueck: die Marke ueberlebt den Neuaufbau
    p.tree->setCurrentItem(p.oben(1));
    warte();
    p.tree->setCurrentItem(p.oben(0));
    warte();
    CHECK(combo()->currentText() == QString::fromUtf8("(unsaved) – Eigen"));
    CHECK(loeschen()->isEnabled());

    // Name/Beschreibung gehoeren nicht zur Voreinstellung: kein „(unsaved)"
    combo()->setCurrentIndex(combo()->findText(QStringLiteral("Eigen *")));
    warte();
    REQUIRE(combo()->currentText() == QStringLiteral("Eigen *"));
    p.oben(0)->setText(3, QStringLiteral("nur eine Notiz"));  // Spalte Beschreibung
    warte();
    CHECK(combo()->currentText() == QStringLiteral("Eigen *"));
}

TEST_CASE("PaletteUi Panel D1b: eine Voreinstellung bringt ihr Song-Skript mit (S87)")
{
    // Der Song-Slot liegt am Knoten (ChainNode::songCode), nicht in den params.
    // Gespeichert wurde er schon immer, geladen nie — aufgefallen an der
    // Host-Switcher-Vorlage „Random Runtime", die im Song-Skript wuerfelt.
    namespace np = lumi::multieffect::nodepresets;
    UiUmgebung env;
    QTemporaryDir root;
    REQUIRE(root.isValid());
    np::setRootForTesting(root.path());
    struct Zurueck
    {
        ~Zurueck() { np::setRootForTesting(QString()); }
    } zurueck;

    const QString werkDir = QDir(root.path()).filePath(QStringLiteral("asset/clear"));
    REQUIRE(QDir().mkpath(werkDir));
    {
        QFile f(QDir(werkDir).filePath(QStringLiteral("Mit Song.json")));
        REQUIRE(f.open(QIODevice::WriteOnly));
        f.write(R"({"format":"lumiviz-nodepreset","formatVersion":1,)"
                R"("node":{"type":"clear","blend":2,"songCode":"x = 1;"}})");
    }
    {
        QFile f(QDir(werkDir).filePath(QStringLiteral("Ohne Song.json")));
        REQUIRE(f.open(QIODevice::WriteOnly));
        f.write(R"({"format":"lumiviz-nodepreset","formatVersion":1,)"
                R"("node":{"type":"clear","blend":1}})");
    }

    PanelUmgebung p;
    REQUIRE(p.tree != nullptr);
    const auto warte = [] {
        for (int i = 0; i < 3; ++i) QCoreApplication::processEvents();
    };
    const auto combo = [&p] { return p.panel->findChild<QComboBox*>(QStringLiteral("knotenPreset")); };

    p.tree->setCurrentItem(p.oben(0));  // Clear
    warte();
    REQUIRE(combo() != nullptr);
    CHECK(p.root().children[0].songCode.empty());

    combo()->setCurrentIndex(combo()->findText(QStringLiteral("Mit Song")));
    warte();
    CHECK(p.root().children[0].songCode == "x = 1;");

    // Teil-Preset ohne Song-Skript: das vorhandene bleibt stehen (Merge)
    combo()->setCurrentIndex(combo()->findText(QStringLiteral("Ohne Song")));
    warte();
    CHECK(p.root().children[0].songCode == "x = 1;");
}

TEST_CASE("PaletteUi Panel D2: Custom Wave traegt eine eigene Preset-Zeile (S83)")
{
    // Entscheid Patrik E3a: Waves/Shapes/Sprites bekommen Voreinstellungen.
    // Die Zeile sitzt in der Einzel-Ansicht des Elements; nach dem Laden
    // bleibt der Editor dort (Neuaufbau ueber den vollen Baum-Pfad).
    namespace np = lumi::multieffect::nodepresets;
    UiUmgebung env;
    QTemporaryDir root;
    REQUIRE(root.isValid());
    np::setRootForTesting(root.path());
    struct Zurueck
    {
        ~Zurueck() { np::setRootForTesting(QString()); }
    } zurueck;
    const QString dir = QDir(root.path()).filePath(QStringLiteral("asset/milkWave"));
    REQUIRE(QDir().mkpath(dir));
    {
        QFile f(QDir(dir).filePath(QStringLiteral("Linie.json")));
        REQUIRE(f.open(QIODevice::WriteOnly));
        f.write(R"({"format":"lumiviz-nodepreset","formatVersion":1,)"
                R"("node":{"type":"milkWave","samples":300,"pointCode":"x = sample;"}})");
    }

    PanelUmgebung p;
    REQUIRE(p.tree != nullptr);
    REQUIRE(p.ziehe("Custom Wave", p.tree->visualItemRect(p.oben(2)).center()));
    const auto welle = [&p]() -> const lumi::milkdrop::WaveState& {
        return std::get<lumi::multieffect::MilkdropNodeParams>(p.root().children[2].params)
            .preset.waves.front();
    };
    REQUIRE(wellenIn(p.root().children[2]) >= 1);
    const int index = welle().index;

    // Element-Item der ersten Custom Wave: Pfad {2, Sektion 1, Element 0}.
    QTreeWidgetItem* item = nullptr;
    for (QTreeWidgetItemIterator it(p.tree); *it != nullptr; ++it)
    {
        const QVariantList v = (*it)->data(0, Qt::UserRole).toList();
        if (v.size() == 3 && v[0].toInt() == 2 && v[1].toInt() == 1000000 + 1 &&
            v[2].toInt() == 0)
            item = *it;
    }
    REQUIRE(item != nullptr);
    const auto warte = [] {
        for (int i = 0; i < 3; ++i) QCoreApplication::processEvents();
    };
    const auto combo = [&p] { return p.panel->findChild<QComboBox*>(QStringLiteral("knotenPreset")); };
    p.tree->setCurrentItem(item);
    warte();
    REQUIRE(combo() != nullptr);
    CHECK(combo()->currentText() == QStringLiteral("(none)"));

    combo()->setCurrentIndex(combo()->findText(QStringLiteral("Linie")));
    warte();
    CHECK(welle().samples == 300);
    CHECK(welle().pointCode == "x = sample;");
    CHECK(welle().index == index);  // Rahmen bleibt
    REQUIRE(combo() != nullptr);    // Editor steht noch in der Element-Ansicht
    CHECK(combo()->currentText() == QStringLiteral("Linie"));

    auto* samples = p.panel->findChild<QSpinBox*>(QStringLiteral("samples"));
    REQUIRE(samples != nullptr);
    samples->setValue(128);
    warte();
    CHECK(welle().samples == 128);
    CHECK(combo()->currentText() == QString::fromUtf8("(unsaved) – Linie"));
}

TEST_CASE("PaletteUi Panel D3: Milkdrop-Parameter tragen Presets (Stellung + bisher-Schalter)")
{
    // Wunsch Patrik S83: auch Code und Parameter des Milkdrop-Nodes bekommen
    // Presets — etwa den alten LumiViz-Look (Drift + Weichzeichnen + die
    // „bisher"-Schalter) in einem Zug. Laden setzt die Felder, Waves & Co.
    // bleiben unberuehrt.
    namespace np = lumi::multieffect::nodepresets;
    UiUmgebung env;
    QTemporaryDir root;
    REQUIRE(root.isValid());
    np::setRootForTesting(root.path());
    struct Zurueck
    {
        ~Zurueck() { np::setRootForTesting(QString()); }
    } zurueck;
    const QString dir = QDir(root.path()).filePath(QStringLiteral("asset/milkParams"));
    REQUIRE(QDir().mkpath(dir));
    {
        QFile f(QDir(dir).filePath(QStringLiteral("Old Look.json")));
        REQUIRE(f.open(QIODevice::WriteOnly));
        f.write(R"({"format":"lumiviz-nodepreset","formatVersion":1,"node":{"type":"milkParams",)"
                R"("gestaltLinAn":true,"gestaltLinStaerke":0.7071,"gestaltLinRichtung":135.0,)"
                R"("gestaltWeichAn":true,"gestaltWeich":1.0,"audioOriginal":false}})");
    }

    PanelUmgebung p;
    REQUIRE(p.tree != nullptr);
    const auto preset = [&p]() -> const lumi::milkdrop::PresetState& {
        return std::get<lumi::multieffect::MilkdropNodeParams>(p.root().children[2].params)
            .preset;
    };
    const std::size_t wellen = preset().waves.size();

    QTreeWidgetItem* item = nullptr;  // Sektion Parameter: Pfad {2, Basis + 5}
    for (QTreeWidgetItemIterator it(p.tree); *it != nullptr; ++it)
    {
        const QVariantList v = (*it)->data(0, Qt::UserRole).toList();
        if (v.size() == 2 && v[0].toInt() == 2 && v[1].toInt() == 1000000 + 5) item = *it;
    }
    REQUIRE(item != nullptr);
    const auto warte = [] {
        for (int i = 0; i < 3; ++i) QCoreApplication::processEvents();
    };
    const auto combo = [&p] { return p.panel->findChild<QComboBox*>(QStringLiteral("knotenPreset")); };
    p.tree->setCurrentItem(item);
    warte();
    REQUIRE(combo() != nullptr);
    combo()->setCurrentIndex(combo()->findText(QStringLiteral("Old Look")));
    warte();
    CHECK(preset().gestaltLinAn);
    CHECK(preset().gestaltLinRichtung == doctest::Approx(135.0));
    CHECK(preset().gestaltWeichAn);
    CHECK_FALSE(preset().audioOriginal);
    CHECK(preset().waves.size() == wellen);  // der Rest des Presets bleibt
    REQUIRE(combo() != nullptr);             // Editor steht noch in der Sektion
    CHECK(combo()->currentText() == QStringLiteral("Old Look"));

    auto* decay = p.panel->findChild<QDoubleSpinBox*>(QStringLiteral("decay"));
    REQUIRE(decay != nullptr);
    decay->setValue(decay->value() > 0.5 ? 0.5 : 0.9);
    warte();
    CHECK(combo()->currentText() == QString::fromUtf8("(unsaved) – Old Look"));
}

// --- Shader-Vorlagen-Zeile im Gross-Editor (S84) ----------------------------
//
// Befund Patrik S84: die Combo sprang nach der Wahl auf „— choose a template —"
// zurueck, wie vorher die Preset-Zeile. Jetzt zeigt sie den Zustand.

namespace
{

struct VorlagenWurzel
{
    QTemporaryDir dir;
    VorlagenWurzel()
    {
        namespace sv = lumi::multieffect::shadervorlagen;
        const auto schreibe = [this](const QString& rel, const QByteArray& text) {
            const QString pfad = dir.filePath(rel);
            QDir().mkpath(QFileInfo(pfad).absolutePath());
            QFile f(pfad);
            REQUIRE(f.open(QIODevice::WriteOnly));
            f.write(text);
        };
        schreibe(QStringLiteral("asset/pixelfilter/Alpha.glsl"), "vec4 farbe(vec2 uv, vec4 src) { return src; }");
        schreibe(QStringLiteral("asset/pixelfilter/Beta.glsl"), "vec4 farbe(vec2 uv, vec4 src) { return 1.0 - src; }");
        sv::setRootForTesting(dir.path());
    }
    ~VorlagenWurzel() { lumi::multieffect::shadervorlagen::setRootForTesting(QString()); }
};

/// Text wie beim Tippen/Einfuegen ersetzen (ueber den Cursor, loest textChanged aus).
void ersetzeText(QPlainTextEdit& e, const QString& text)
{
    QTextCursor c(e.document());
    c.select(QTextCursor::Document);
    c.insertText(text);
}

}  // namespace

TEST_CASE("PaletteUi S84: Vorlagen-Zeile behaelt die Wahl und zeigt (unsaved) nach Aenderung")
{
    UiUmgebung umgebung;
    VorlagenWurzel wurzel;
    QWidget dlg;
    auto* editor = new QPlainTextEdit(&dlg);
    editor->setPlainText(QStringLiteral("// eigener Code"));
    auto* lay = new QVBoxLayout(&dlg);
    lay->addLayout(lumi::scriptedit::vorlagenZeile(&dlg, editor, QStringLiteral("pixelfilter")));
    lay->addWidget(editor);
    zeigen(dlg);
    auto* combo = dlg.findChild<QComboBox*>(QStringLiteral("shaderVorlage"));
    REQUIRE(combo != nullptr);
    CHECK(combo->currentText() == QStringLiteral("(none)"));

    const int alpha = combo->findText(QStringLiteral("Alpha"));
    REQUIRE(alpha > 0);
    combo->setCurrentIndex(alpha);
    emit combo->activated(alpha);
    CHECK(editor->toPlainText().startsWith(QStringLiteral("vec4 farbe")));
    CHECK(combo->currentText() == QStringLiteral("Alpha"));  // vorher: Rueckfall auf Index 0

    ersetzeText(*editor, editor->toPlainText() + QStringLiteral(" // x"));
    CHECK(combo->currentText() == QString::fromUtf8("(unsaved) – Alpha"));

    // Zurueck auf den Vorlagen-Text => wieder sauber.
    ersetzeText(*editor, combo->itemData(alpha).toString());
    CHECK(combo->currentText() == QStringLiteral("Alpha"));

    // Strg+Z bis zum Ausgangstext => Zustand beim Oeffnen, nicht „(unsaved) – Alpha".
    while (editor->document()->isUndoAvailable()) editor->undo();
    CHECK(editor->toPlainText() == QStringLiteral("// eigener Code"));
    CHECK(combo->currentText() == QStringLiteral("(none)"));

    // Die Zustandszeile ist keine Wahl: sie aendert den Text nicht.
    emit combo->activated(0);
    CHECK(editor->toPlainText() == QStringLiteral("// eigener Code"));
}

TEST_CASE("PaletteUi S84: Vorlagen-Zeile erkennt beim Oeffnen den Text einer Vorlage")
{
    UiUmgebung umgebung;
    VorlagenWurzel wurzel;
    QWidget dlg;
    auto* editor = new QPlainTextEdit(&dlg);
    editor->setPlainText(QStringLiteral("vec4 farbe(vec2 uv, vec4 src) { return 1.0 - src; }"));
    auto* lay = new QVBoxLayout(&dlg);
    lay->addLayout(lumi::scriptedit::vorlagenZeile(&dlg, editor, QStringLiteral("pixelfilter")));
    auto* combo = dlg.findChild<QComboBox*>(QStringLiteral("shaderVorlage"));
    REQUIRE(combo != nullptr);
    CHECK(combo->currentText() == QStringLiteral("Beta"));
    ersetzeText(*editor, QStringLiteral("x"));
    CHECK(combo->currentText() == QString::fromUtf8("(unsaved) – Beta"));
}

// --- Preset-Dateien ziehen: Effect Browser -> Effect Chain (S84) ---------------
//
// Plan S84 Punkt 1: ein Preset aus dem Effect Browser in die Kette ziehen legt
// einen Presetverweis an. Die Datei reist als Datei-URL — dieselbe Form wie aus
// dem Explorer.

namespace
{

/// Legt eine kleine, ladbare Kette als .lvfx ab (ein Clear-Knoten).
QString schreibeLvfx(const QTemporaryDir& dir, const QString& name)
{
    ChainNode root;
    root.params = lumi::multieffect::ListParams{};
    root.children.push_back(PanelUmgebung::knoten("Clear"));
    const QString pfad = dir.filePath(name);
    REQUIRE(lumi::multieffect::saveChainToFile(root, pfad));
    return pfad;
}

/// Drag-Folge an den Viewport; false = der Baum hat den Zug abgewiesen.
bool sendeDrop(QWidget* vp, QMimeData& data, QPoint pos)
{
    QDragEnterEvent enter(pos, Qt::CopyAction, &data, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(vp, &enter);
    QDragMoveEvent move(pos, Qt::CopyAction, &data, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(vp, &move);
    if (!move.isAccepted()) return false;
    QDropEvent drop(QPointF(pos), Qt::CopyAction, &data, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(vp, &drop);
    QCoreApplication::processEvents();
    QCoreApplication::processEvents();
    return drop.isAccepted();
}

const lumi::multieffect::HostGroupParams* gruppe(const ChainNode& n)
{
    return std::get_if<lumi::multieffect::HostGroupParams>(&n.params);
}

}  // namespace

TEST_CASE("PaletteUi S84: presetDateien nimmt nur lokale Preset-Dateien, in Reihenfolge")
{
    UiUmgebung env;
    QTemporaryDir dir;
    const QString lvfx = schreibeLvfx(dir, QStringLiteral("b.lvfx"));
    const QString txt = dir.filePath(QStringLiteral("notiz.txt"));
    REQUIRE(QFile::copy(lvfx, txt));
    const QString avs = dir.filePath(QStringLiteral("a.AVS"));  // Grossschreibung zaehlt nicht
    REQUIRE(QFile::copy(lvfx, avs));
    REQUIRE(QDir(dir.path()).mkdir(QStringLiteral("ordner.milk")));  // Ordner mit Preset-Endung

    QMimeData data;
    data.setUrls({QUrl::fromLocalFile(lvfx), QUrl::fromLocalFile(txt),
                  QUrl(QStringLiteral("https://example.org/x.milk")),
                  QUrl::fromLocalFile(dir.filePath(QStringLiteral("ordner.milk"))),
                  QUrl::fromLocalFile(avs)});
    const QStringList treffer = lumi::ui::presetDateien(&data);
    REQUIRE(treffer.size() == 2);
    CHECK(QFileInfo(treffer[0]) == QFileInfo(lvfx));
    CHECK(QFileInfo(treffer[1]) == QFileInfo(avs));

    QMimeData nurText;
    nurText.setUrls({QUrl::fromLocalFile(txt)});
    CHECK(lumi::ui::presetDateien(&nurText).isEmpty());
    CHECK(lumi::ui::dateiMime({}) == nullptr);
}

TEST_CASE("PaletteUi S84: Kettenbaum meldet abgelegte Preset-Dateien, weist andere ab")
{
    UiUmgebung env;
    QTemporaryDir dir;
    const QString lvfx = schreibeLvfx(dir, QStringLiteral("show.lvfx"));
    Kettenbaum k;
    QStringList gemeldet;
    QTreeWidgetItem* ziel = nullptr;
    k.tree.onFileDrop = [&](const QStringList& files, QTreeWidgetItem* t, ChainDrop) {
        gemeldet = files;
        ziel = t;
    };
    std::unique_ptr<QMimeData> data(lumi::ui::dateiMime({lvfx}));
    CHECK(sendeDrop(k.tree.viewport(), *data, k.tree.visualItemRect(k.a).center()));
    REQUIRE(gemeldet.size() == 1);
    CHECK(QFileInfo(gemeldet[0]) == QFileInfo(lvfx));
    CHECK(ziel == k.a);
    CHECK_FALSE(k.letzter.paletteIndex.has_value());  // kein Paletten-Drop

    // Gegenprobe: eine Datei ohne Preset-Endung nimmt der Baum nicht an.
    const QString txt = dir.filePath(QStringLiteral("notiz.txt"));
    REQUIRE(QFile::copy(lvfx, txt));
    gemeldet.clear();
    std::unique_ptr<QMimeData> fremd(lumi::ui::dateiMime({txt}));
    CHECK_FALSE(sendeDrop(k.tree.viewport(), *fremd, k.tree.visualItemRect(k.a).center()));
    CHECK(gemeldet.isEmpty());
}

TEST_CASE("PaletteUi S84 Panel: Preset-Datei in die Kette gezogen wird ein Presetverweis")
{
    UiUmgebung env;
    QTemporaryDir dir;
    const QString lvfx = schreibeLvfx(dir, QStringLiteral("Intro.lvfx"));
    PanelUmgebung p;
    REQUIRE(p.tree != nullptr);

    // Auf die Kante zwischen Clear und Liste: an Position 1
    const QRect zweite = p.tree->visualItemRect(p.oben(1));
    std::unique_ptr<QMimeData> data(lumi::ui::dateiMime({lvfx}));
    CHECK(sendeDrop(p.tree->viewport(), *data, QPoint(zweite.center().x(), zweite.top() + 1)));
    REQUIRE(p.root().children.size() == 4);
    const ChainNode& neu = p.root().children[1];
    const auto* hg = gruppe(neu);
    REQUIRE(hg != nullptr);
    CHECK(hg->verweis);  // Verweis, keine Kopie
    CHECK(QFileInfo(QString::fromStdString(hg->sourceFile)) == QFileInfo(lvfx));
    CHECK(neu.displayName == "Intro");
    CHECK(ist<lumi::multieffect::ListParams>(p.root().children[2]));  // alte Liste rutscht
}

TEST_CASE("PaletteUi S84 Panel: mehrere Dateien auf die Leerflaeche -> Verweise am Ende, in Reihenfolge")
{
    UiUmgebung env;
    QTemporaryDir dir;
    const QString eins = schreibeLvfx(dir, QStringLiteral("Eins.lvfx"));
    const QString zwei = schreibeLvfx(dir, QStringLiteral("Zwei.lvfx"));
    PanelUmgebung p;
    REQUIRE(p.tree != nullptr);
    std::unique_ptr<QMimeData> data(lumi::ui::dateiMime({eins, zwei}));
    CHECK(sendeDrop(p.tree->viewport(), *data, QPoint(60, p.tree->viewport()->height() - 10)));
    REQUIRE(p.root().children.size() == 5);
    CHECK(p.root().children[3].displayName == "Eins");
    CHECK(p.root().children[4].displayName == "Zwei");
    CHECK(gruppe(p.root().children[4]) != nullptr);
}

TEST_CASE("PaletteUi S84: Effect Browser gibt Presets als Datei-URL mit, Ordner nicht")
{
    UiUmgebung env;
    QTemporaryDir dir;
    const QString lvfx = schreibeLvfx(dir, QStringLiteral("Intro.lvfx"));
    REQUIRE(QDir(dir.path()).mkdir(QStringLiteral("Unterordner")));
    ServiceContainer services;
    services.registerInstance<IEventBus>(std::make_unique<EventBus>());
    ImportBrowserPanel browser(services);
    zeigen(browser);

    QLineEdit* pfad = nullptr;
    for (QLineEdit* e : browser.findChildren<QLineEdit*>())
        if (e->placeholderText().startsWith(QStringLiteral("Folder path"))) pfad = e;
    REQUIRE(pfad != nullptr);
    pfad->setText(dir.path());
    emit pfad->returnPressed();
    auto* liste = browser.findChild<QListWidget*>(QStringLiteral("effectBrowserListe"));
    REQUIRE(liste != nullptr);
    CHECK(liste->dragEnabled());

    QListWidgetItem* datei = nullptr;
    QListWidgetItem* ordner = nullptr;
    for (int i = 0; i < liste->count(); ++i)
    {
        if (liste->item(i)->text() == QStringLiteral("Intro.lvfx")) datei = liste->item(i);
        if (liste->item(i)->text() == QStringLiteral("Unterordner")) ordner = liste->item(i);
    }
    REQUIRE(datei != nullptr);
    REQUIRE(ordner != nullptr);
    CHECK(datei->flags().testFlag(Qt::ItemIsDragEnabled));
    CHECK_FALSE(ordner->flags().testFlag(Qt::ItemIsDragEnabled));
    CHECK(liste->model()->supportedDragActions() == Qt::CopyAction);  // nie verschieben

    std::unique_ptr<QMimeData> data(liste->model()->mimeData({liste->indexFromItem(datei)}));
    REQUIRE(data != nullptr);
    const QStringList treffer = lumi::ui::presetDateien(data.get());
    REQUIRE(treffer.size() == 1);
    CHECK(QFileInfo(treffer[0]) == QFileInfo(lvfx));
    // Ordner allein ergibt keine Drag-Daten.
    std::unique_ptr<QMimeData> leer(liste->model()->mimeData({liste->indexFromItem(ordner)}));
    CHECK(leer == nullptr);
}

// S89: Karaoke-Presets (.lvkar) im Effect Browser — eigener Typ: ein
// Doppelklick fordert das Laden in die Karaoke-Ebene an (nicht in die Kette),
// die Datei ist nicht in die Kette ziehbar, und das Karaoke-Panel liest die
// Ebene neu, wenn sie von aussen ersetzt wurde.
TEST_CASE("Karaoke S89: .lvkar im Effect Browser — Doppelklick laedt in die Ebene, Panel liest neu")
{
    UiUmgebung env;
    MultiEffectVisualizer::setKaraokeAblage(QString());
    using lumi::multieffect::LyricsParams;
    QTemporaryDir dir;
    const QString lvfx = schreibeLvfx(dir, QStringLiteral("Intro.lvfx"));
    const QString lvkar = dir.filePath(QStringLiteral("Abend.lvkar"));
    {
        MultiEffectVisualizer quelle;
        std::get<LyricsParams>(quelle.karaokeKette().children[0].params).yShift = -33;
        REQUIRE(quelle.speichereKaraokeDatei(lvkar));
    }

    ServiceContainer services;
    services.registerInstance<IEventBus>(std::make_unique<EventBus>());
    IEventBus* bus = services.tryResolve<IEventBus>();
    std::vector<std::string> karaoke;
    int ketten = 0;
    const auto abo1 = bus->subscribeScoped<LoadKaraokePresetEvent>(
        [&](const LoadKaraokePresetEvent& e) { karaoke.push_back(e.path); });
    const auto abo2 =
        bus->subscribeScoped<LoadEffectChainEvent>([&](const LoadEffectChainEvent&) { ++ketten; });

    // --- Zuordnung: Typ und Icon
    CHECK(QFileInfo::exists(lumi::ui::presetIconDir() + QStringLiteral("/lumiviz_karaoke.ico")));
    // beim Blaettern (preset.next) zaehlt ein Karaoke-Preset nicht als Preset
    CHECK(ImportBrowserPanel::nextPresetRow(
              {ImportBrowserPanel::Type_Karaoke, ImportBrowserPanel::Type_Lvfx}, -1, 1) == 1);

    ImportBrowserPanel browser(services);
    zeigen(browser);
    QLineEdit* pfad = nullptr;
    for (QLineEdit* e : browser.findChildren<QLineEdit*>())
        if (e->placeholderText().startsWith(QStringLiteral("Folder path"))) pfad = e;
    REQUIRE(pfad != nullptr);
    pfad->setText(dir.path());
    emit pfad->returnPressed();
    auto* liste = browser.findChild<QListWidget*>(QStringLiteral("effectBrowserListe"));
    REQUIRE(liste != nullptr);
    QListWidgetItem* eintrag = nullptr;
    for (int i = 0; i < liste->count(); ++i)
        if (liste->item(i)->text() == QStringLiteral("Abend.lvkar")) eintrag = liste->item(i);
    REQUIRE(eintrag != nullptr);  // der Filter „All" zeigt die Datei
    CHECK(eintrag->data(Qt::UserRole + 1).toInt() == ImportBrowserPanel::Type_Karaoke);

    // nicht in die Kette ziehbar
    std::unique_ptr<QMimeData> leer(liste->model()->mimeData({liste->indexFromItem(eintrag)}));
    CHECK(leer == nullptr);

    // Doppelklick: Karaoke-Ereignis mit dem Pfad, KEIN Ketten-Laden
    emit liste->itemDoubleClicked(eintrag);
    REQUIRE(karaoke.size() == 1);
    CHECK(QFileInfo(QString::fromStdString(karaoke[0])) == QFileInfo(lvkar));
    CHECK(ketten == 0);

    // --- das Karaoke-Panel liest die von aussen ersetzte Ebene neu
    MultiEffectVisualizer vis;
    QMutex mutex;
    KaraokePanel panel(services);
    panel.resize(520, 900);
    zeigen(panel);
    bus->publish(VisualizerChangedEvent("multieffect", "Multi Effect",
                                        static_cast<IVisualizer*>(&vis), &mutex));
    QCoreApplication::processEvents();
    MultiEffectPanel* editor = nullptr;
    for (MultiEffectPanel* e : panel.findChildren<MultiEffectPanel*>())
        if (e->baum() == MultiEffectPanel::Baum::Karaoke) editor = e;
    REQUIRE(editor != nullptr);
    auto* yShift = editor->findChild<QSpinBox*>(QStringLiteral("yShift"));
    REQUIRE(yShift != nullptr);
    CHECK(yShift->value() != -33);
    // so, wie es das Fenster tut: laden, dann melden
    REQUIRE(vis.ladeKaraokeDatei(lvkar));
    bus->publish(KaraokeLayerChangedEvent{});
    QCoreApplication::processEvents();
    yShift = editor->findChild<QSpinBox*>(QStringLiteral("yShift"));
    REQUIRE(yShift != nullptr);
    CHECK(yShift->value() == -33);
}

// --- Audio ziehen: Audio Browser, Playlist, Player, Visualisierung (S84) -------
//
// Entscheide Patrik S84: 1a Playlist nimmt an und gibt ab; Drop auf Player/
// Visualisierung springt zum Eintrag (fehlt er, wird er angehaengt);
// 2a Doppelklick im Audio Browser = anhaengen + abspielen; 3a Icons je Familie.

namespace
{

/// Player-Attrappe: schreibt nur mit, welcher Eintrag gespielt werden soll.
class PlayerAttrappe : public IAudioPlayer
{
public:
    QList<int> gespielt;

    bool load(const QString&) override { return true; }
    void play() override {}
    void pause() override {}
    void togglePlayPause() override {}
    void stop() override {}
    bool next() override { return false; }
    bool previous() override { return false; }
    bool playIndex(int index) override
    {
        gespielt.append(index);
        return true;
    }
    int letzterSprung = -1;  ///< S87: wohin zuletzt gesprungen wurde
    void seek(int ms) override { letzterSprung = ms; }
    void seekFraction(float) override {}
    void seekRelative(int) override {}
    [[nodiscard]] float volume() const override { return 1.0F; }
    void setVolume(float) override {}
    [[nodiscard]] bool isMuted() const override { return false; }
    void setMuted(bool) override {}
    void toggleMute() override {}
    [[nodiscard]] PlaybackState state() const override { return PlaybackState{}; }
    [[nodiscard]] bool isPlaying() const override { return false; }
    [[nodiscard]] bool isPaused() const override { return false; }
    [[nodiscard]] bool isStopped() const override { return true; }
    [[nodiscard]] int positionMs() const override { return 0; }
    [[nodiscard]] int durationMs() const override { return 0; }
    [[nodiscard]] float positionFraction() const override { return 0.0F; }
    [[nodiscard]] TrackInfo currentTrack() const override { return {}; }
    [[nodiscard]] bool hasTrack() const override { return false; }
    void setPlaylist(IPlaylist*) override {}
    [[nodiscard]] IPlaylist* playlist() const override { return nullptr; }
    [[nodiscard]] int playlistIndex() const override { return -1; }
    [[nodiscard]] RepeatMode repeatMode() const override { return RepeatMode::None; }
    void setRepeatMode(RepeatMode) override {}
    [[nodiscard]] bool shuffle() const override { return false; }
    void setShuffle(bool) override {}
    void update() override {}
};

/// Dienste mit echter Playlist und Player-Attrappe, dazu Dateien in einem
/// Temp-Ordner (Inhalt egal: die Playlist prueft nur, ob die Datei existiert).
struct AudioUmgebung
{
    ServiceContainer services;
    QTemporaryDir dir;
    IPlaylist* playlist = nullptr;
    PlayerAttrappe* player = nullptr;

    AudioUmgebung()
    {
        services.registerInstance<IEventBus>(std::make_unique<EventBus>());
        auto pl = std::make_unique<Playlist>(*services.tryResolve<IEventBus>());
        playlist = pl.get();
        services.registerInstance<IPlaylist>(std::move(pl));
        auto p = std::make_unique<PlayerAttrappe>();
        player = p.get();
        services.registerInstance<IAudioPlayer>(std::move(p));
    }

    QString datei(const QString& name)
    {
        const QString pfad = QFileInfo(dir.filePath(name)).absoluteFilePath();
        QFile f(pfad);
        REQUIRE(f.open(QIODevice::WriteOnly));
        f.write("x");
        return pfad;
    }
};

QStringList playlistPfade(const IPlaylist& pl)
{
    return pl.filePaths();
}

}  // namespace

TEST_CASE("PaletteUi S84: neueReihenfolge — Zeilen vor das Ziel, gezogene in Reihenfolge")
{
    using V = std::vector<int>;
    CHECK(PlaylistPanel::neueReihenfolge(5, {1}, 4) == V{0, 2, 3, 1, 4});
    CHECK(PlaylistPanel::neueReihenfolge(5, {1}, 5) == V{0, 2, 3, 4, 1});   // ans Ende
    CHECK(PlaylistPanel::neueReihenfolge(5, {3, 4}, 0) == V{3, 4, 0, 1, 2});
    CHECK(PlaylistPanel::neueReihenfolge(5, {0, 2}, 5) == V{1, 3, 4, 0, 2});
    CHECK(PlaylistPanel::neueReihenfolge(5, {1, 2}, 2) == V{0, 1, 2, 3, 4});  // auf sich selbst
    CHECK(PlaylistPanel::neueReihenfolge(3, {}, 1) == V{0, 1, 2});
}

TEST_CASE("PaletteUi S84: Audio-Icons — jede Familie hat ein eigenes, sichtbares Icon")
{
    UiUmgebung env;
    REQUIRE_FALSE(lumi::ui::presetIconDir().isEmpty());
    CHECK(lumi::ui::audioFamilie(QStringLiteral("MP3")) == QStringLiteral("mp3"));
    CHECK(lumi::ui::audioFamilie(QStringLiteral("opus")) == QStringLiteral("ogg"));
    CHECK(lumi::ui::audioFamilie(QStringLiteral("xyz")).isEmpty());
    QSet<qint64> gesehen;
    for (const char* e : {"mp3", "wav", "flac", "ogg", "m4a", "mid"})
    {
        INFO("Endung " << e);
        const QIcon icon = lumi::ui::audioTypIcon(QString::fromUtf8(e));
        REQUIRE_FALSE(icon.isNull());
        const QImage bild = icon.pixmap(64, 64).toImage();
        REQUIRE_FALSE(bild.isNull());
        CHECK(qAlpha(bild.pixel(32, 32)) > 0);  // gezeichnet, nicht leer
        gesehen.insert(icon.pixmap(64, 64).cacheKey());
        const QString d = schnappschussOrdner();
        if (!d.isEmpty()) bild.save(d + QStringLiteral("/audio_icon_%1.png").arg(e));
    }
    CHECK(gesehen.size() == 6);  // keine zwei Familien teilen ein Bild
}

TEST_CASE("PaletteUi S84: spieleDateien springt zu Vorhandenem, haengt Fehlendes an")
{
    UiUmgebung env;
    AudioUmgebung a;
    const QString eins = a.datei(QStringLiteral("eins.mp3"));
    const QString zwei = a.datei(QStringLiteral("zwei.wav"));
    REQUIRE(a.playlist->addTrack(eins) == 0);

    CHECK(lumi::ui::spieleDateien(a.services, {zwei}));
    CHECK(playlistPfade(*a.playlist) == QStringList{eins, zwei});
    CHECK(a.player->gespielt == QList<int>{1});

    CHECK(lumi::ui::spieleDateien(a.services, {eins}));
    CHECK(a.playlist->count() == 2);  // nicht doppelt angehaengt
    CHECK(a.player->gespielt == QList<int>{1, 0});
}

TEST_CASE("PaletteUi S84: spieleDrop — Playlist-Zeile gilt genau, fremde Dateien nichts")
{
    UiUmgebung env;
    AudioUmgebung a;
    const QString lied = a.datei(QStringLiteral("lied.mp3"));
    a.playlist->addTrack(lied);
    a.playlist->addTrack(a.datei(QStringLiteral("anderes.mp3")));
    a.playlist->addTrack(lied);  // Doppel an Zeile 2

    std::unique_ptr<QMimeData> zeile(lumi::ui::dateiMime({lied}));
    zeile->setData(QString::fromUtf8(lumi::ui::kPlaylistZeilenMime), "2");
    CHECK(lumi::ui::spieleDrop(zeile.get(), a.services));
    CHECK(a.player->gespielt == QList<int>{2});  // nicht der erste Treffer (0)

    // Gegenprobe: keine Audiodatei -> nichts abgespielt, nichts angehaengt.
    std::unique_ptr<QMimeData> fremd(lumi::ui::dateiMime({a.datei(QStringLiteral("notiz.txt"))}));
    CHECK_FALSE(lumi::ui::hatAbspielbares(fremd.get(), a.services));
    CHECK_FALSE(lumi::ui::spieleDrop(fremd.get(), a.services));
    CHECK(a.playlist->count() == 3);
    CHECK(a.player->gespielt == QList<int>{2});
}

TEST_CASE("PaletteUi S84: Drop auf das Player-Panel spielt die Datei")
{
    UiUmgebung env;
    AudioUmgebung a;
    const QString lied = a.datei(QStringLiteral("lied.flac"));
    PlayerPanel panel(a.services);
    panel.resize(400, 200);
    zeigen(panel);
    CHECK(panel.acceptDrops());

    std::unique_ptr<QMimeData> data(lumi::ui::dateiMime({lied}));
    CHECK(sendeDrop(&panel, *data, QPoint(20, 20)));
    CHECK(playlistPfade(*a.playlist) == QStringList{lied});
    CHECK(a.player->gespielt == QList<int>{0});

    std::unique_ptr<QMimeData> fremd(lumi::ui::dateiMime({a.datei(QStringLiteral("x.txt"))}));
    CHECK_FALSE(sendeDrop(&panel, *fremd, QPoint(20, 20)));
}

TEST_CASE("PaletteUi S84: Playlist — Umsortieren per Drag landet im Dienst, Dateien an der Stelle")
{
    UiUmgebung env;
    AudioUmgebung a;
    const QString s0 = a.datei(QStringLiteral("a.mp3"));
    const QString s1 = a.datei(QStringLiteral("b.mp3"));
    const QString s2 = a.datei(QStringLiteral("c.mp3"));
    for (const QString& s : {s0, s1, s2}) a.playlist->addTrack(s);
    PlaylistPanel panel(a.services);
    panel.resize(400, 400);
    zeigen(panel);
    auto* liste = panel.findChild<QListWidget*>(QStringLiteral("playlistListe"));
    REQUIRE(liste != nullptr);
    REQUIRE(liste->count() == 3);

    // Hinausziehen: Datei-URLs + Zeilennummern (fuer Doppel)
    std::unique_ptr<QMimeData> data(liste->model()->mimeData(
        {liste->model()->index(2, 0), liste->model()->index(0, 0)}));
    REQUIRE(data != nullptr);
    CHECK(lumi::ui::playlistZeilen(data.get()) == QList<int>{0, 2});
    CHECK(lumi::ui::audioDateien(data.get(), a.services) == QStringList{s0, s2});

    // Dateien von aussen: vor Zeile 1 einfuegen
    const QString neu = a.datei(QStringLiteral("neu.wav"));
    std::unique_ptr<QMimeData> aussen(lumi::ui::dateiMime({neu}));
    const QRect zweite = liste->visualItemRect(liste->item(1));
    CHECK(sendeDrop(liste->viewport(), *aussen, QPoint(zweite.center().x(), zweite.top() + 1)));
    CHECK(playlistPfade(*a.playlist) == QStringList{s0, neu, s1, s2});
    CHECK(liste->count() == 4);  // Liste aus dem Dienst neu aufgebaut
    CHECK(a.player->gespielt.isEmpty());  // in die Playlist ziehen spielt nicht

    // Umsortieren (Befund S84: vorher nur in der Anzeige): Zeile 0 ans Ende —
    // im DIENST, die Liste folgt.
    PlaylistPanel::verschiebeZeilen(*a.playlist, {0}, 4);
    CHECK(playlistPfade(*a.playlist) == QStringList{neu, s1, s2, s0});
    REQUIRE(liste->count() == 4);
    CHECK(liste->item(3)->data(Qt::UserRole).toString() == s0);
    PlaylistPanel::verschiebeZeilen(*a.playlist, {2, 3}, 0);
    CHECK(playlistPfade(*a.playlist) == QStringList{s2, s0, neu, s1});
}

TEST_CASE("PaletteUi S84: Audio Browser — Icons, Drag als Datei-URL, Doppelklick spielt")
{
    UiUmgebung env;
    AudioUmgebung a;
    const QString lied = a.datei(QStringLiteral("lied.mp3"));
    a.datei(QStringLiteral("notiz.txt"));
    REQUIRE(QDir(a.dir.path()).mkdir(QStringLiteral("Alben")));
    AudioBrowserPanel browser(a.services);
    zeigen(browser);

    QLineEdit* pfad = nullptr;
    for (QLineEdit* e : browser.findChildren<QLineEdit*>())
        if (e->placeholderText().startsWith(QStringLiteral("Folder path"))) pfad = e;
    REQUIRE(pfad != nullptr);
    pfad->setText(a.dir.path());
    emit pfad->returnPressed();
    auto* liste = browser.findChild<QListWidget*>(QStringLiteral("audioBrowserListe"));
    REQUIRE(liste != nullptr);

    QListWidgetItem* datei = nullptr;
    QListWidgetItem* ordner = nullptr;
    for (int i = 0; i < liste->count(); ++i)
    {
        const QString t = liste->item(i)->text();
        CHECK(t != QStringLiteral("notiz.txt"));  // nur Abspielbares
        if (t == QStringLiteral("lied.mp3")) datei = liste->item(i);
        if (t == QStringLiteral("Alben")) ordner = liste->item(i);
    }
    REQUIRE(datei != nullptr);
    REQUIRE(ordner != nullptr);
    CHECK_FALSE(datei->icon().isNull());
    CHECK_FALSE(ordner->flags().testFlag(Qt::ItemIsDragEnabled));
    CHECK(liste->model()->supportedDragActions() == Qt::CopyAction);

    std::unique_ptr<QMimeData> data(liste->model()->mimeData({liste->indexFromItem(datei)}));
    REQUIRE(data != nullptr);
    CHECK(lumi::ui::audioDateien(data.get(), a.services) == QStringList{lied});

    emit liste->itemDoubleClicked(datei);
    CHECK(playlistPfade(*a.playlist) == QStringList{lied});
    CHECK(a.player->gespielt == QList<int>{0});
    schnappschuss(browser, "S84_audio_browser");
}

TEST_CASE("PaletteUi S84: Audio Browser zeigt das eingebettete Cover, sonst das Typ-Icon")
{
    UiUmgebung env;
    AudioUmgebung a;
    // Rotes PNG als Frontcover in einem ID3v2.3-Tag
    QImage rot(16, 16, QImage::Format_RGB32);
    rot.fill(Qt::red);
    QByteArray png;
    {
        QBuffer puffer(&png);
        REQUIRE(puffer.open(QIODevice::WriteOnly));
        REQUIRE(rot.save(&puffer, "PNG"));
    }
    QByteArray rahmen;
    rahmen.append('\0');
    rahmen.append("image/png");
    rahmen.append('\0');
    rahmen.append(static_cast<char>(3));
    rahmen.append('\0');
    rahmen.append(png);
    const auto be32 = [](qsizetype v) {
        QByteArray b(4, '\0');
        for (int i = 0; i < 4; ++i) b[i] = static_cast<char>((v >> (24 - 8 * i)) & 0xFF);
        return b;
    };
    QByteArray koerper = QByteArray("APIC") + be32(rahmen.size()) + QByteArray(2, '\0') + rahmen;
    const qsizetype n = koerper.size();
    QByteArray tag("ID3\x03\x00\x00", 6);
    for (int s : {21, 14, 7, 0}) tag.append(static_cast<char>((n >> s) & 0x7F));
    const QString mitCover = QFileInfo(a.dir.filePath(QStringLiteral("mit.mp3"))).absoluteFilePath();
    {
        QFile f(mitCover);
        REQUIRE(f.open(QIODevice::WriteOnly));
        f.write(tag + koerper + QByteArray("MPEG"));
    }
    a.datei(QStringLiteral("ohne.mp3"));

    AudioBrowserPanel browser(a.services);
    zeigen(browser);
    QLineEdit* pfad = nullptr;
    for (QLineEdit* e : browser.findChildren<QLineEdit*>())
        if (e->placeholderText().startsWith(QStringLiteral("Folder path"))) pfad = e;
    REQUIRE(pfad != nullptr);
    pfad->setText(a.dir.path());
    emit pfad->returnPressed();
    auto* liste = browser.findChild<QListWidget*>(QStringLiteral("audioBrowserListe"));
    REQUIRE(liste != nullptr);
    CHECK(liste->iconSize() == QSize(AudioBrowserPanel::kIconKante, AudioBrowserPanel::kIconKante));

    const auto eintrag = [liste](const char* name) -> QListWidgetItem* {
        for (int i = 0; i < liste->count(); ++i)
            if (liste->item(i)->text() == QString::fromUtf8(name)) return liste->item(i);
        return nullptr;
    };
    const auto mitte = [](const QListWidgetItem* item) {
        const QImage b = item->icon().pixmap(32, 32).toImage();
        return b.pixelColor(b.width() / 2, b.height() / 2);
    };
    // Der Lader laeuft im Hintergrund: bis zu 5 s auf das Cover warten.
    QElapsedTimer uhr;
    uhr.start();
    while (uhr.elapsed() < 5000 && mitte(eintrag("mit.mp3")) != QColor(Qt::red))
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
        QThread::msleep(10);
    }
    REQUIRE(eintrag("mit.mp3") != nullptr);
    CHECK(mitte(eintrag("mit.mp3")) == QColor(Qt::red));
    // Gegenprobe: ohne Cover bleibt das Typ-Icon (nicht rot, nicht leer)
    REQUIRE(eintrag("ohne.mp3") != nullptr);
    CHECK_FALSE(eintrag("ohne.mp3")->icon().isNull());
    CHECK(mitte(eintrag("ohne.mp3")) != QColor(Qt::red));
    schnappschuss(browser, "S84_audio_browser_cover");
}

TEST_CASE("PaletteUi S84 F5: Highlighter faerbt Modulfelder nach der Modul-Referenz")
{
    // Block F (F5): seit F2 ist jedes Feld skriptbar — der Highlighter kennt
    // die Namen aus der Referenz des Moduls. Gegenprobe: ohne Modul-Kategorien
    // bleibt `onbeatrandom` ungefaerbt (unbekannter Bezeichner).
    UiUmgebung env;
    const auto kat = lumi::scriptedit::SymCat::InOut;
    QTextDocument mit;
    mit.setPlainText(QStringLiteral("onbeatrandom = 1;"));
    lumi::scriptedit::EelHighlighter hlMit(&mit, {}, {{QStringLiteral("onbeatrandom"), kat}});
    hlMit.rehighlight();
    QTextDocument ohne;
    ohne.setPlainText(QStringLiteral("onbeatrandom = 1;"));
    lumi::scriptedit::EelHighlighter hlOhne(&ohne);
    hlOhne.rehighlight();

    QTextDocument referenz;  // eine global bekannte In/Out-Variable
    referenz.setPlainText(QStringLiteral("zoom = 1;"));
    lumi::scriptedit::EelHighlighter hlRef(&referenz);
    hlRef.rehighlight();

    REQUIRE(farbeBei(referenz, 0, 0).isValid());
    CHECK(farbeBei(mit, 0, 0) == farbeBei(referenz, 0, 0));
    CHECK_FALSE(farbeBei(ohne, 0, 0).isValid());
}

// Block F, F4 (S85): „~"-Knopf an modulierbaren Feldzeilen — Dialog setzt die
// Modulation am Knoten, danach erscheint „Modulation active", Entfernen raeumt ab.
TEST_CASE("PaletteUi S85 F4: Modulation per ~-Knopf setzen, schalten, entfernen")
{
    UiUmgebung env;
    PanelUmgebung p;
    REQUIRE(p.tree != nullptr);
    const auto warte = [] {
        for (int i = 0; i < 3; ++i) QCoreApplication::processEvents();
    };
    // Erster Knoten wird ein Bloom: intensity (modulierbar), post (Schalter),
    // downsample (Struktur) — die beiden letzten bekommen keinen Knopf
    p.vis.chain().children[0].params = lumi::multieffect::BloomParams{};
    p.tree->setCurrentItem(p.oben(0));
    warte();
    auto* knopf = p.panel->findChild<QToolButton*>(QStringLiteral("mod_intensity"));
    REQUIRE(knopf != nullptr);
    CHECK_FALSE(knopf->isChecked());
    CHECK(p.panel->findChild<QToolButton*>(QStringLiteral("mod_post")) == nullptr);
    CHECK(p.panel->findChild<QToolButton*>(QStringLiteral("mod_downsample")) == nullptr);
    CHECK(p.panel->findChild<QCheckBox*>(QStringLiteral("modulationActive")) == nullptr);
    // Das Feld selbst ist weiter unter seinem Vertragsnamen zu finden
    CHECK(p.panel->findChild<QDoubleSpinBox*>(QStringLiteral("intensity")) != nullptr);

    // Dialog: Quelle LFO square, Tiefe 0.2, OK
    // Im Timer keine REQUIRE/CHECK: eine Ausnahme quer durch die Qt-Schleife
    // beendet den Prozess. Gefunden wird der Dialog ueber seinen Namen; das
    // Ergebnis prueft der Test danach am Knoten.
    static bool dialogGesehen = false;
    const auto bediene = [](int quelle, double tiefe, const char* knopfName) {
        dialogGesehen = false;
        QTimer::singleShot(50, [=] {
            QDialog* dlg = nullptr;
            for (QWidget* w : QApplication::topLevelWidgets())
                if (w->objectName() == QStringLiteral("modulationDialog") && w->isVisible())
                    dlg = qobject_cast<QDialog*>(w);
            if (dlg == nullptr) return;
            dialogGesehen = true;
            if (auto* c = dlg->findChild<QComboBox*>(QStringLiteral("modSource")); c && quelle >= 0)
                c->setCurrentIndex(quelle);
            if (auto* s = dlg->findChild<QDoubleSpinBox*>(QStringLiteral("modDepth")); s && tiefe != 0.0)
                s->setValue(tiefe);
            QAbstractButton* b = nullptr;
            if (knopfName != nullptr)
                b = dlg->findChild<QPushButton*>(QString::fromLatin1(knopfName));
            else if (auto* box = dlg->findChild<QDialogButtonBox*>())
                b = box->button(QDialogButtonBox::Ok);
            if (b != nullptr)
                b->click();
            else
                dlg->reject();
        });
    };
    bediene(static_cast<int>(lumi::multieffect::ModQuelle::LfoRechteck), 0.2, nullptr);
    knopf->click();
    warte();
    CHECK(dialogGesehen);
    {
        const auto& mods = p.root().children[0].mods;
        REQUIRE(mods.size() == 1);
        CHECK(mods[0].feld == "intensity");
        CHECK(mods[0].quelle == lumi::multieffect::ModQuelle::LfoRechteck);
        CHECK(mods[0].tiefe == doctest::Approx(0.2));
    }
    // Seite neu aufbauen (Knoten erneut waehlen): Knopf markiert, Schalter da
    p.tree->setCurrentItem(p.oben(1));
    warte();
    p.tree->setCurrentItem(p.oben(0));
    warte();
    knopf = p.panel->findChild<QToolButton*>(QStringLiteral("mod_intensity"));
    REQUIRE(knopf != nullptr);
    CHECK(knopf->isChecked());
    auto* schalter = p.panel->findChild<QCheckBox*>(QStringLiteral("modulationActive"));
    REQUIRE(schalter != nullptr);
    schalter->setChecked(false);
    warte();
    CHECK_FALSE(p.root().children[0].modsActive);

    // Entfernen (Knopf frisch suchen — der Schalter kann die Seite neu gebaut haben)
    knopf = p.panel->findChild<QToolButton*>(QStringLiteral("mod_intensity"));
    REQUIRE(knopf != nullptr);
    bediene(-1, 0.0, "modRemove");
    knopf->click();
    warte();
    CHECK(dialogGesehen);
    CHECK(p.root().children[0].mods.empty());
}

// Block F, F4c (S86): SuperScope und Effect List haben keinen Skripttraeger,
// ihre Zahlfelder tragen trotzdem den „~"-Knopf — Auswahlfelder und Schalter nicht.
TEST_CASE("PaletteUi S86 F4c: ~-Knopf an SuperScope- und Listenfeldern")
{
    UiUmgebung env;
    PanelUmgebung p;
    REQUIRE(p.tree != nullptr);
    const auto warte = [] {
        for (int i = 0; i < 3; ++i) QCoreApplication::processEvents();
    };
    const auto hatKnopf = [&](const char* feld) {
        return p.panel->findChild<QToolButton*>(QStringLiteral("mod_") +
                                                QLatin1String(feld)) != nullptr;
    };
    p.vis.chain().children[0].params = lumi::multieffect::SuperScopeParams{};
    p.tree->setCurrentItem(p.oben(1));
    warte();
    p.tree->setCurrentItem(p.oben(0));
    warte();
    for (const char* feld : {"pointCount", "lineWidth", "dotSize", "colorCycleFrames"})
    {
        CAPTURE(feld);
        CHECK(hatKnopf(feld));
    }
    CHECK_FALSE(hatKnopf("renderMode"));
    CHECK_FALSE(hatKnopf("spectrumSource"));

    p.vis.chain().children[0].params = lumi::multieffect::ListParams{};
    p.tree->setCurrentItem(p.oben(1));
    warte();
    p.tree->setCurrentItem(p.oben(0));
    warte();
    for (const char* feld :
         {"inAdjustAlpha", "outAdjustAlpha", "bufferIn", "bufferOut", "onBeatFrames"})
    {
        CAPTURE(feld);
        CHECK(hatKnopf(feld));
    }
    CHECK_FALSE(hatKnopf("blendOut"));
    CHECK_FALSE(hatKnopf("clearEveryFrame"));

    // F4b (S86): Farbfelder tragen den Knopf ebenfalls
    p.vis.chain().children[0].params = lumi::multieffect::ClearParams{};
    p.tree->setCurrentItem(p.oben(1));
    warte();
    p.tree->setCurrentItem(p.oben(0));
    warte();
    CHECK(hatKnopf("color"));
    CHECK_FALSE(hatKnopf("onlyFirst"));
}

// S85 (Wunsch Patrik): die Info-Seite eines Skript-Slots beginnt mit seinem
// Takt (einmal, je Frame, je Beat, je Punkt …). Waechter: JEDES Slot-Feld, das
// das Panel mit addScript anlegt, bekommt einen Text — ein neuer Slot ohne
// Beschreibung faellt hier auf.
// S86 (Befund Patrik): die Zeile „Lyrics" im Editor stand auf dem Stand beim
// Oeffnen der Seite und meldete „not found", waehrend laengst ein Text lief.
// Sie laeuft jetzt mit; „Open folder" ist nur bei einer eigenen Datei aktiv.
TEST_CASE("PaletteUi S86/S87: Lyrics — Tabelle laeuft mit, Abruf-Stand, Ordner-Knopf nur mit Datei")
{
    UiUmgebung env;
    PanelUmgebung p;
    REQUIRE(p.tree != nullptr);
    namespace ly = lumi::multieffect::lyrics;
    const auto warte = [](int ms) {
        QElapsedTimer t;
        t.start();
        while (t.elapsed() < ms) QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
    };
    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    p.vis.chain().children[0].params = lumi::multieffect::LyricsParams{};
    p.tree->setCurrentItem(p.oben(1));
    warte(20);
    p.tree->setCurrentItem(p.oben(0));
    warte(20);
    // S87 (Wunsch Patrik): die Angaben stehen als Tabelle — je Angabe ein Feld
    const auto feld = [&p](const char* name) {
        auto* l = p.panel->findChild<QLabel*>(QLatin1String(name));
        REQUIRE(l != nullptr);
        return l;
    };
    auto* status = feld("lyricsStatus");
    auto* quelle = feld("lyricsSource");
    auto* takt = feld("lyricsTiming");
    auto* zeilen = feld("lyricsLines");
    auto* herkunft = feld("lyricsOrigin");
    auto* datei = feld("lyricsFile");
    auto* ordner = p.panel->findChild<QToolButton*>(QStringLiteral("lyricsOpenFolder"));
    REQUIRE(ordner != nullptr);
    const QString strich = QStringLiteral("—");
    MultiEffectVisualizer::setSongtextAbruf(ly::Abruf::Ruhe);
    warte(700);
    CHECK(status->text().startsWith(QStringLiteral("No lyrics")));
    CHECK(quelle->text() == strich);
    CHECK(takt->text() == strich);
    CHECK(zeilen->text() == strich);
    CHECK(datei->text() == strich);
    CHECK_FALSE(ordner->isEnabled());

    // S87: solange ein Online-Abruf laeuft, steht das da — auch waehrend der
    // Dienst um Geduld bittet; danach das Ergebnis
    MultiEffectVisualizer::setSongtextAbruf(ly::Abruf::Laedt);
    warte(700);
    CHECK(status->text() == QString::fromUtf8("…downloading…"));
    MultiEffectVisualizer::setSongtextAbruf(ly::Abruf::Wartet);
    warte(700);
    CHECK(status->text().startsWith(QString::fromUtf8("…downloading…")));
    CHECK(status->text().contains(QStringLiteral("busy")));
    MultiEffectVisualizer::setSongtextAbruf(ly::Abruf::NichtGefunden);
    warte(700);
    CHECK(status->text().contains(QStringLiteral("not online either")));
    MultiEffectVisualizer::setSongtextAbruf(ly::Abruf::Fehlgeschlagen);
    warte(700);
    CHECK(status->text().contains(QStringLiteral("failed")));

    // der Text kommt NACH dem Oeffnen der Seite (Titelwechsel, Online-Fund)
    ly::Songtext s = ly::leseLrc("[00:01.00]eins\n[00:02.00]zwei\n");
    s.quelle = "lied.lrc";
    s.datei = "C:/Musik/lied.lrc";
    MultiEffectVisualizer::setSongtext(s);
    MultiEffectVisualizer::setSongtextAbruf(ly::Abruf::Gefunden);
    warte(700);
    CHECK(status->text() == QStringLiteral("Lyrics loaded"));
    CHECK(quelle->text() == QStringLiteral("lied.lrc"));
    CHECK(takt->text() == QStringLiteral("synced"));
    CHECK(zeilen->text() == QStringLiteral("2"));
    CHECK(herkunft->text() == strich);
    CHECK(datei->text() == QStringLiteral("lied.lrc"));
    CHECK(datei->toolTip() == QStringLiteral("C:/Musik/lied.lrc"));
    CHECK(ordner->isEnabled());

    // S88: Versatz nur dieses Titels — eigene Formularzeile, ohne Titel gesperrt,
    // folgt dem Titel und haelt eine Eingabe fest (QSettings/AppData abgeschottet)
    auto* versatz = p.panel->findChild<QSpinBox*>(QStringLiteral("lyricsTrackOffset"));
    auto* zuweisen = p.panel->findChild<QToolButton*>(QStringLiteral("lyricsAssignFile"));
    REQUIRE(versatz != nullptr);
    REQUIRE(zuweisen != nullptr);
    CHECK_FALSE(versatz->isEnabled());  // Gegenprobe: es laeuft kein Titel
    CHECK_FALSE(zuweisen->isEnabled());
    TrackInfo titel;
    titel.filePath = QStringLiteral("C:/Musik/lied.mp3");
    titel.title = QStringLiteral("Lied S88 Versatz");
    titel.artist = QStringLiteral("Wer");
    lumi::audio::setzeLaufendenTitel(titel);
    MultiEffectVisualizer::setSongtextTitelVersatz(250);  // wie beim Songwechsel
    warte(700);
    CHECK(versatz->isEnabled());
    CHECK(zuweisen->isEnabled());
    CHECK(versatz->value() == 250);
    CHECK(lumi::audio::songtextTitelVersatzMs(titel) == 0);  // nachziehen schreibt nichts
    versatz->setValue(-100);
    CHECK(MultiEffectVisualizer::songtextTitelVersatzMs() == -100);
    CHECK(lumi::audio::songtextTitelVersatzMs(titel) == -100);
    schnappschuss(*versatz->parentWidget(), "lyrics_editor_s88");  // der Editor, nicht das Panel
    versatz->setValue(0);  // raeumt den Eintrag wieder ab
    CHECK(lumi::audio::songtextTitelVersatzMs(titel) == 0);
    lumi::audio::setzeLaufendenTitel(TrackInfo{});

    // aus dem Netz geholt: der Vermerk aus der Datei hat sein eigenes Feld
    s.herkunft = "LRCLIB id 7, fetched 2026-10-01";
    MultiEffectVisualizer::setSongtext(s);
    warte(700);
    CHECK(herkunft->text() == QStringLiteral("LRCLIB id 7, fetched 2026-10-01"));
    s.herkunft.clear();

    // eingebetteter, unsynchroner Text: keine Datei, kein Ordner zu oeffnen
    s = ly::leseLrc("eins\nzwei\ndrei\n");
    s.quelle = "embedded";
    MultiEffectVisualizer::setSongtext(s);
    warte(700);
    CHECK(quelle->text() == QStringLiteral("embedded"));
    CHECK(takt->text() == QStringLiteral("not synced"));
    CHECK(zeilen->text() == QStringLiteral("3"));
    CHECK(datei->text() == strich);
    CHECK_FALSE(ordner->isEnabled());

    // ein vorhandener Text geht vor: der Stand eines Abrufs ueberschreibt ihn nicht
    MultiEffectVisualizer::setSongtextAbruf(ly::Abruf::Laedt);
    warte(700);
    CHECK(status->text() == QStringLiteral("Lyrics loaded"));

    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    MultiEffectVisualizer::setSongtextAbruf(ly::Abruf::Ruhe);
}

// S87 (Wunsch Patrik): die Playlist zeigt die Laenge je Titel
TEST_CASE("PaletteUi S87: Playlist — Dauer als m:ss, ab einer Stunde h:mm:ss")
{
    CHECK(PlaylistPanel::dauerText(0).isEmpty());   // noch nicht gelesen / nicht lesbar
    CHECK(PlaylistPanel::dauerText(-5).isEmpty());
    CHECK(PlaylistPanel::dauerText(7000) == QStringLiteral("0:07"));
    CHECK(PlaylistPanel::dauerText(187400) == QStringLiteral("3:07"));
    CHECK(PlaylistPanel::dauerText(187600) == QStringLiteral("3:08"));  // gerundet
    CHECK(PlaylistPanel::dauerText(3723000) == QStringLiteral("1:02:03"));
}

// S87 (Befund Patrik): bei schmaler Playlist stand die Dauer rechts ausserhalb
// des Sichtbaren, weil die Zeile so breit war wie der laengste Name. Die Zeile
// folgt jetzt der Breite der Liste — schmal wie breit.
TEST_CASE("PaletteUi S87: Playlist — die Zeile ist so breit wie die Liste, nie breiter")
{
    UiUmgebung env;
    ServiceContainer dienste;
    PlaylistPanel panel(dienste);
    auto* liste = panel.findChild<QListWidget*>(QStringLiteral("playlistListe"));
    REQUIRE(liste != nullptr);
    panel.show();  // baut die Liste aus dem Dienst neu auf — erst danach fuellen
    for (int i = 0; i < 3; ++i) QCoreApplication::processEvents();
    auto* item = new QListWidgetItem(
        QStringLiteral("ein sehr langer Dateiname, der in keine schmale Liste passt und "
                       "frueher die Zeile verbreitert hat.mp3"),
        liste);
    item->setData(PlaylistPanel::kDauerRolle, 187000);
    for (const int breite : {180, 520})
    {
        CAPTURE(breite);
        panel.resize(breite, 300);
        for (int i = 0; i < 3; ++i) QCoreApplication::processEvents();
        const QRect zeile = liste->visualItemRect(item);
        CHECK(zeile.width() > 0);
        CHECK(zeile.right() <= liste->viewport()->width());  // rechter Rand sichtbar
        CHECK(zeile.width() >= liste->viewport()->width() - 2);  // und sie fuellt die Breite
        CHECK_FALSE(liste->horizontalScrollBar()->isVisible());
    }
}

// S87 (Befund Patrik): der Suchbegriff blieb nach Aus- und Einblenden im Feld
// stehen, gefiltert war aber nichts mehr — die Liste wird beim Einblenden neu
// gebaut und vergass dabei den Filter.
TEST_CASE("PaletteUi S87: Playlist — der Suchfilter ueberlebt den Neuaufbau der Liste")
{
    UiUmgebung env;
    AudioUmgebung audio;
    audio.playlist->addTrack(audio.datei(QStringLiteral("alpha.mp3")));
    audio.playlist->addTrack(audio.datei(QStringLiteral("beta.mp3")));
    PlaylistPanel panel(audio.services);
    panel.show();
    for (int i = 0; i < 3; ++i) QCoreApplication::processEvents();
    auto* liste = panel.findChild<QListWidget*>(QStringLiteral("playlistListe"));
    auto* suche = panel.findChild<QLineEdit*>();
    REQUIRE(liste != nullptr);
    REQUIRE(suche != nullptr);
    REQUIRE(liste->count() == 2);
    const auto sichtbar = [liste] {
        QStringList namen;
        for (int i = 0; i < liste->count(); ++i)
            if (!liste->item(i)->isHidden()) namen << liste->item(i)->text();
        return namen;
    };

    suche->setText(QStringLiteral("bet"));
    for (int i = 0; i < 3; ++i) QCoreApplication::processEvents();
    CHECK(sichtbar() == QStringList{QStringLiteral("beta.mp3")});

    // aus- und wieder einblenden: Begriff UND Filter stehen noch
    panel.hide();
    for (int i = 0; i < 3; ++i) QCoreApplication::processEvents();
    panel.show();
    for (int i = 0; i < 3; ++i) QCoreApplication::processEvents();
    CHECK(suche->text() == QStringLiteral("bet"));
    CHECK(sichtbar() == QStringList{QStringLiteral("beta.mp3")});

    // die Playlist aendert sich (Liste wird neu gebaut): der Filter gilt weiter
    audio.playlist->addTrack(audio.datei(QStringLiteral("gamma.mp3")));
    for (int i = 0; i < 3; ++i) QCoreApplication::processEvents();
    CHECK(liste->count() == 3);
    CHECK(sichtbar() == QStringList{QStringLiteral("beta.mp3")});
}

// S87 (Idee Patrik S86): Songtext nachsynchronisieren — mitklopfen,
// nachstellen, vorhoeren, speichern.
TEST_CASE("PaletteUi S87: Lyrics Sync — klopfen, verschieben, speichern als .lrc neben der Quelle")
{
    UiUmgebung env;
    AudioUmgebung audio;
    namespace ly = lumi::multieffect::lyrics;
    QTemporaryDir tmp;
    REQUIRE(tmp.isValid());

    // ein unsynchroner Text aus einer Datei, mit einer Leerzeile dazwischen
    const QString quelle = tmp.path() + "/Wer - Lied.txt";
    {
        QFile f(quelle);
        REQUIRE(f.open(QIODevice::WriteOnly));
        f.write("eins\nzwei\n\ndrei\n");
    }
    TrackInfo t;
    t.filePath = tmp.path() + "/lied.mp3";
    t.title = "Lied";
    t.artist = "Wer";
    t.durationMs = 200000;
    ly::Songtext text = ly::leseLrc("eins\nzwei\n\ndrei\n");
    text.datei = quelle.toStdString();
    lumi::audio::setzeLaufendenTitel(t);
    MultiEffectVisualizer::setSongtext(text);
    MultiEffectVisualizer::notifyPlayState(false);

    LyricsSyncPanel panel(audio.services);
    panel.setBenutzerOrdner(tmp.path() + "/nutzer");
    panel.lade(t, text);
    auto* liste = panel.findChild<QTableWidget*>(QStringLiteral("lyricsSyncList"));
    auto* speichern = panel.findChild<QPushButton*>(QStringLiteral("lyricsSyncSave"));
    auto* stand = panel.findChild<QLabel*>(QStringLiteral("lyricsSyncStatus"));
    REQUIRE(liste != nullptr);
    REQUIRE(speichern != nullptr);
    REQUIRE(stand != nullptr);
    REQUIRE(liste->rowCount() == 4);
    CHECK(liste->item(0, 0)->text().isEmpty());
    CHECK(liste->item(0, 2)->text() == QStringLiteral("eins"));
    CHECK(liste->alternatingRowColors());
    CHECK(liste->cellWidget(0, 1) == nullptr);  // ohne Zeit kein Sprung-Knopf
    CHECK(panel.offeneZeilen() == 3);  // die Leerzeile zaehlt nicht
    CHECK(stand->text().contains(QStringLiteral("3")));
    CHECK_FALSE(speichern->isEnabled());
    CHECK(panel.zielPfad() == tmp.path() + "/Wer - Lied.lrc");  // neben der Quelle

    // klopfen: Zeit an die markierte Zeile, weiter zur naechsten TEXTzeile
    CHECK(liste->currentRow() == 0);
    REQUIRE(panel.tippe(10000));
    CHECK(liste->item(0, 0)->text() == QStringLiteral("00:10.00"));
    CHECK(liste->currentRow() == 1);
    REQUIRE(panel.tippe(20000));
    CHECK(liste->currentRow() == 3);  // die Leerzeile wird uebersprungen
    CHECK(panel.geaendert());
    CHECK_FALSE(speichern->isEnabled());  // „drei" hat noch keine Zeit
    // Vorschau: der Knoten sieht schon die beiden geklopften Zeilen
    CHECK(MultiEffectVisualizer::songtext().synchron);
    CHECK(MultiEffectVisualizer::songtext().zeilen.size() == 2);
    REQUIRE(panel.tippe(30000));
    CHECK(panel.offeneZeilen() == 0);
    CHECK(speichern->isEnabled());

    // nachstellen: eine Zeile, dann alles ab der zweiten
    liste->selectRow(0);
    panel.verschiebe(-LyricsSyncPanel::kSchrittMs, false);
    CHECK(liste->item(0, 0)->text() == QStringLiteral("00:09.90"));
    liste->selectRow(1);
    panel.verschiebe(500, true);
    CHECK(liste->item(0, 0)->text() == QStringLiteral("00:09.90"));  // davor: unberuehrt
    CHECK(liste->item(1, 0)->text() == QStringLiteral("00:20.50"));
    CHECK(liste->item(3, 0)->text() == QStringLiteral("00:30.50"));
    CHECK(liste->item(2, 0)->text().isEmpty());  // Leerzeile bleibt ohne Zeit

    // Zeit von Hand (F2): gueltig wird uebernommen, Unlesbares verworfen
    liste->item(3, 0)->setText(QStringLiteral("00:31.25"));
    CHECK(liste->item(3, 0)->text() == QStringLiteral("00:31.25"));
    liste->item(3, 0)->setText(QStringLiteral("quatsch"));
    CHECK(liste->item(3, 0)->text() == QStringLiteral("00:31.25"));

    // Zeit loeschen macht die Zeile wieder offen — Speichern geht dann nicht
    liste->selectRow(3);
    panel.loescheZeit();
    CHECK(panel.offeneZeilen() == 1);
    CHECK_FALSE(panel.speichere());
    REQUIRE(panel.tippe(31250));

    // vorhoeren: Doppelklick springt kurz vor die Zeile
    Q_EMIT liste->cellDoubleClicked(1, 1);
    CHECK(audio.player->letzterSprung == 20500 - LyricsSyncPanel::kVorlaufMs);
    // der Knopf der Zeile springt GENAU auf ihre Zeit; flach, damit die
    // (abwechselnde) Zeilenfarbe in seiner Zelle steht
    auto* sprung = qobject_cast<QToolButton*>(liste->cellWidget(1, 1));
    REQUIRE(sprung != nullptr);
    CHECK(sprung->autoRaise());
    sprung->click();
    CHECK(audio.player->letzterSprung == 20500);
    CHECK(liste->cellWidget(2, 1) == nullptr);  // die Leerzeile hat keine Zeit
    CHECK_FALSE(panel.springeZu(2, 0));

    // die Spielzeit steht als Text da
    CHECK(LyricsSyncPanel::spielzeitText(83450, 225000) == QStringLiteral("1:23.4 / 3:45"));
    CHECK(LyricsSyncPanel::spielzeitText(0, 0) == QStringLiteral("0:00.0"));
    REQUIRE(panel.findChild<QLabel*>(QStringLiteral("lyricsSyncTime")) != nullptr);
    REQUIRE(panel.findChild<QWidget*>(QStringLiteral("lyricsSyncWave")) != nullptr);
    panel.resize(460, 420);
    schnappschuss(panel, "lyrics_sync");

    // speichern: .lrc neben der .txt, die Suche findet ab jetzt die .lrc
    REQUIRE(panel.speichere());
    CHECK_FALSE(panel.geaendert());
    CHECK(QFile::exists(tmp.path() + "/Wer - Lied.lrc"));
    CHECK(QFile::exists(quelle));  // die .txt bleibt
    const ly::Songtext gefunden = lumi::audio::findeSongtext(t, tmp.path());
    REQUIRE(gefunden.synchron);
    REQUIRE(gefunden.zeilen.size() == 3);
    CHECK(gefunden.zeilen[0].zeitMs == 9900);
    CHECK(gefunden.zeilen[1].zeitMs == 20500);
    CHECK(gefunden.zeilen[2].zeitMs == 31250);
    CHECK(gefunden.zeilen[2].text == "drei");
    CHECK(gefunden.laengeMs == 200000);
    CHECK(liste->rowCount() == 3);  // wie in der Datei: ohne die Leerzeile
    CHECK(MultiEffectVisualizer::songtext().zeilen.size() == 3);

    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    lumi::audio::setzeLaufendenTitel(TrackInfo{});
}

// S90 (LB5, Entscheid L6-3 a): Duett — die Stimme je Zeile im Panel setzen.
// Zuerst der Befund, der vorher nur aus dem Code gelesen war: eine Stimme, die
// der Text schon traegt, uebersteht Klopfen und Speichern.
TEST_CASE("PaletteUi S90: Lyrics Sync — Stimmen setzen (Duett), Spalte Voice, speichern")
{
    UiUmgebung env;
    AudioUmgebung audio;
    namespace ly = lumi::multieffect::lyrics;
    QTemporaryDir tmp;
    REQUIRE(tmp.isValid());

    const QString quelle = tmp.path() + "/Zwei - Duett.txt";
    {
        QFile f(quelle);
        REQUIRE(f.open(QIODevice::WriteOnly));
        f.write("eins\nv2: zwei\ndrei\n\nvier\n");
    }
    TrackInfo t;
    t.filePath = tmp.path() + "/duett.mp3";
    t.title = "Duett";
    t.artist = "Zwei";
    t.durationMs = 200000;
    // „v2:" gilt bis zur naechsten Marke: zwei, drei und vier singt Stimme 2
    ly::Songtext text = ly::leseLrc("eins\nv2: zwei\ndrei\n\nvier\n");
    text.datei = quelle.toStdString();
    REQUIRE(text.zeilen.size() == 5);
    REQUIRE(text.zeilen[1].stimme == ly::Stimme::Zwei);
    lumi::audio::setzeLaufendenTitel(t);
    MultiEffectVisualizer::setSongtext(text);
    MultiEffectVisualizer::notifyPlayState(false);

    LyricsSyncPanel panel(audio.services);
    panel.setBenutzerOrdner(tmp.path() + "/nutzer");
    panel.lade(t, text);
    auto* liste = panel.findChild<QTableWidget*>(QStringLiteral("lyricsSyncList"));
    REQUIRE(liste != nullptr);
    REQUIRE(liste->columnCount() == 4);
    // Spalte Voice (logisch 3) steht VOR dem Text, der Text fuellt den Rest
    CHECK(liste->horizontalHeaderItem(3)->text() == QStringLiteral("Voice"));
    CHECK(liste->horizontalHeader()->visualIndex(3) < liste->horizontalHeader()->visualIndex(2));
    CHECK(liste->item(0, 3)->text().isEmpty());
    CHECK(liste->item(1, 3)->text() == QStringLiteral("2"));
    CHECK(liste->item(2, 3)->text() == QStringLiteral("2"));
    CHECK(liste->item(3, 3)->text().isEmpty());  // Leerzeile: niemand singt

    // Stimme setzen: Knopf fuer mehrere markierte Zeilen, Taste in der Liste
    auto* beide = panel.findChild<QPushButton*>(QStringLiteral("lyricsSyncVoiceBoth"));
    REQUIRE(beide != nullptr);
    REQUIRE(panel.findChild<QPushButton*>(QStringLiteral("lyricsSyncVoice1")) != nullptr);
    REQUIRE(panel.findChild<QPushButton*>(QStringLiteral("lyricsSyncVoice2")) != nullptr);
    liste->clearSelection();
    liste->setSelectionMode(QAbstractItemView::MultiSelection);
    liste->selectRow(2);
    liste->selectRow(3);  // die Leerzeile ist mit markiert — sie bleibt ohne Stimme
    liste->selectRow(4);
    liste->setSelectionMode(QAbstractItemView::ExtendedSelection);
    beide->click();
    CHECK(liste->item(2, 3)->text() == QStringLiteral("1+2"));
    CHECK(liste->item(3, 3)->text().isEmpty());
    CHECK(liste->item(4, 3)->text() == QStringLiteral("1+2"));
    CHECK(liste->item(1, 3)->text() == QStringLiteral("2"));  // nicht markiert: unberuehrt
    CHECK(panel.geaendert());
    liste->selectRow(4);
    QKeyEvent taste(QEvent::KeyPress, Qt::Key_1, Qt::NoModifier, QStringLiteral("1"));
    QCoreApplication::sendEvent(liste, &taste);
    CHECK(liste->item(4, 3)->text() == QStringLiteral("1"));
    // zuruecknehmen und wiederholen gilt auch fuer die Stimme
    REQUIRE(panel.rueckgaengig());
    CHECK(liste->item(4, 3)->text() == QStringLiteral("1+2"));
    REQUIRE(panel.wiederholen());
    CHECK(liste->item(4, 3)->text() == QStringLiteral("1"));

    // klopfen aendert die Stimmen nicht
    liste->selectRow(0);
    REQUIRE(panel.tippe(10000));
    REQUIRE(panel.tippe(20000));
    REQUIRE(panel.tippe(30000));
    REQUIRE(panel.tippe(40000));
    CHECK(panel.offeneZeilen() == 0);
    CHECK(liste->item(1, 3)->text() == QStringLiteral("2"));
    CHECK(liste->item(2, 3)->text() == QStringLiteral("1+2"));
    // die Vorschau am Knoten traegt die Stimmen; die Zeile ohne Marke zaehlt
    // in einem Duett als Stimme 1 (in der Datei gilt eine Marke bis zur naechsten)
    const ly::Songtext vorschau = MultiEffectVisualizer::songtext();
    REQUIRE(vorschau.zeilen.size() == 4);
    CHECK(vorschau.zeilen[0].stimme == ly::Stimme::Eins);
    CHECK(vorschau.zeilen[1].stimme == ly::Stimme::Zwei);
    CHECK(vorschau.zeilen[2].stimme == ly::Stimme::Beide);
    CHECK(vorschau.zeilen[3].stimme == ly::Stimme::Eins);

    // speichern und wieder lesen: jede Zeile kommt mit ihrer Stimme zurueck
    REQUIRE(panel.speichere());
    const ly::Songtext gefunden = lumi::audio::findeSongtext(t, tmp.path());
    REQUIRE(gefunden.synchron);
    REQUIRE(gefunden.zeilen.size() == 4);
    CHECK(gefunden.zeilen[0].stimme == ly::Stimme::Eins);
    CHECK(gefunden.zeilen[0].text == "eins");
    CHECK(gefunden.zeilen[1].stimme == ly::Stimme::Zwei);
    CHECK(gefunden.zeilen[2].stimme == ly::Stimme::Beide);
    CHECK(gefunden.zeilen[3].stimme == ly::Stimme::Eins);
    CHECK(gefunden.zeilen[3].text == "vier");
    CHECK(liste->item(0, 3)->text() == QStringLiteral("1"));  // nach dem Speichern steht sie da

    // ein Text ganz ohne Stimmen bleibt ohne: keine Marken in der Datei
    const QString ohne = tmp.path() + "/Wer - Solo.txt";
    {
        QFile f(ohne);
        REQUIRE(f.open(QIODevice::WriteOnly));
        f.write("eins\nzwei\n");
    }
    TrackInfo s;
    s.filePath = tmp.path() + "/solo.mp3";
    s.title = "Solo";
    s.artist = "Wer";
    ly::Songtext solo = ly::leseLrc("eins\nzwei\n");
    solo.datei = ohne.toStdString();
    lumi::audio::setzeLaufendenTitel(s);
    MultiEffectVisualizer::setSongtext(solo);
    panel.lade(s, solo);
    liste->selectRow(0);
    REQUIRE(panel.tippe(1000));
    REQUIRE(panel.tippe(2000));
    REQUIRE(panel.speichere());
    QFile datei(tmp.path() + "/Wer - Solo.lrc");
    REQUIRE(datei.open(QIODevice::ReadOnly));
    const QByteArray inhalt = datei.readAll();
    CHECK_FALSE(inhalt.contains("v1:"));
    CHECK(inhalt.contains("]eins"));

    panel.resize(460, 460);
    schnappschuss(panel, "lyrics_sync_duett");
    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    lumi::audio::setzeLaufendenTitel(TrackInfo{});
}

// S90 (LB5-2, Entscheide L6-1 a / L6-2 a): Wort-Zeiten klopfen. Schalter
// „Words", Streifen mit den Worten der markierten Zeile, Tap je Wort.
TEST_CASE("PaletteUi S90: Lyrics Sync — Wort-Zeiten klopfen, Streifen, halb geklopfte Zeilen")
{
    UiUmgebung env;
    AudioUmgebung audio;
    namespace ly = lumi::multieffect::lyrics;
    QTemporaryDir tmp;
    REQUIRE(tmp.isValid());

    const QString quelle = tmp.path() + "/Wer - Worte.txt";
    {
        QFile f(quelle);
        REQUIRE(f.open(QIODevice::WriteOnly));
        f.write("eins zwei drei\nvier fuenf\n");
    }
    TrackInfo t;
    t.filePath = tmp.path() + "/worte.mp3";
    t.title = "Worte";
    t.artist = "Wer";
    t.durationMs = 200000;
    ly::Songtext text = ly::leseLrc("eins zwei drei\nvier fuenf\n");
    text.datei = quelle.toStdString();
    lumi::audio::setzeLaufendenTitel(t);
    MultiEffectVisualizer::setSongtext(text);
    MultiEffectVisualizer::notifyPlayState(false);

    LyricsSyncPanel panel(audio.services);
    panel.setBenutzerOrdner(tmp.path() + "/nutzer");
    panel.lade(t, text);
    panel.resize(460, 520);
    zeigen(panel);
    auto* liste = panel.findChild<QTableWidget*>(QStringLiteral("lyricsSyncList"));
    auto* worte = panel.findChild<QListWidget*>(QStringLiteral("lyricsSyncWords"));
    auto* schalter = panel.findChild<QCheckBox*>(QStringLiteral("lyricsSyncWordMode"));
    auto* leeren = panel.findChild<QPushButton*>(QStringLiteral("lyricsSyncClearWords"));
    auto* stand = panel.findChild<QLabel*>(QStringLiteral("lyricsSyncStatus"));
    REQUIRE(liste != nullptr);
    REQUIRE(worte != nullptr);
    REQUIRE(schalter != nullptr);
    REQUIRE(leeren != nullptr);
    REQUIRE(stand != nullptr);

    // aus: kein Streifen, Tap gilt der Zeile (wie bisher)
    CHECK_FALSE(panel.wortModus());
    CHECK_FALSE(worte->isVisible());
    CHECK_FALSE(leeren->isVisible());

    // an: der Streifen zeigt die Worte der markierten Zeile, das erste ist dran
    schalter->setChecked(true);
    CHECK(panel.wortModus());
    CHECK(worte->isVisible());
    CHECK(leeren->isVisible());
    REQUIRE(worte->count() == 3);
    CHECK(worte->item(0)->text().startsWith(QStringLiteral("eins")));
    CHECK(worte->item(2)->text().startsWith(QStringLiteral("drei")));
    CHECK(worte->currentRow() == 0);
    CHECK(panel.gewaehltesWort() == 0);

    // Tap: das erste Wort bekommt die Zeit — und die Zeile mit ihm
    REQUIRE(panel.tippe(10000));
    CHECK(liste->item(0, 0)->text() == QStringLiteral("00:10.00"));
    CHECK(liste->currentRow() == 0);  // die Zeile bleibt, das naechste Wort ist dran
    CHECK(panel.gewaehltesWort() == 1);
    CHECK(worte->item(0)->text().contains(QStringLiteral("00:10.00")));
    CHECK(panel.offeneWortZeilen() == 1);
    CHECK(stand->text().contains(QStringLiteral("words still to tap")));
    // halb geklopft: die Vorschau am Knoten traegt die Zeile OHNE Wort-Zeiten (L6-2 a)
    REQUIRE(MultiEffectVisualizer::songtext().zeilen.size() == 1);
    CHECK(MultiEffectVisualizer::songtext().zeilen[0].worte.empty());

    REQUIRE(panel.tippe(11000));
    REQUIRE(panel.tippe(12000));  // letztes Wort: weiter zur naechsten Zeile, ihr erstes Wort
    CHECK(liste->currentRow() == 1);
    CHECK(panel.gewaehltesWort() == 0);
    REQUIRE(worte->count() == 2);
    CHECK(worte->item(0)->text().startsWith(QStringLiteral("vier")));
    CHECK(panel.offeneWortZeilen() == 0);
    // die erste Zeile ist vollstaendig: die Vorschau traegt ihre Wort-Zeiten
    {
        const ly::Songtext v = MultiEffectVisualizer::songtext();
        REQUIRE(v.zeilen.size() == 1);
        REQUIRE(v.zeilen[0].worte.size() == 3);
        CHECK(v.zeilen[0].worte[0].zeitMs == 10000);
        CHECK(v.zeilen[0].worte[1].zeitMs == 11000);
        CHECK(v.zeilen[0].worte[2].zeitMs == 12000);
        CHECK(v.zeilen[0].worte[0].text + v.zeilen[0].worte[1].text + v.zeilen[0].worte[2].text ==
              "eins zwei drei");
    }

    // zweite Zeile nur zur Haelfte
    REQUIRE(panel.tippe(20000));
    CHECK(liste->item(1, 0)->text() == QStringLiteral("00:20.00"));
    CHECK(panel.offeneZeilen() == 0);      // jede Zeile hat eine Zeit …
    CHECK(panel.offeneWortZeilen() == 1);  // … aber eine hat ein offenes Wort

    // ein Wort neu klopfen: im Streifen waehlen
    liste->selectRow(0);
    panel.waehleWort(1);
    CHECK(worte->currentRow() == 1);
    REQUIRE(panel.tippe(11400));
    CHECK(worte->item(1)->text().contains(QStringLiteral("00:11.40")));
    CHECK(liste->item(0, 0)->text() == QStringLiteral("00:10.00"));  // die Zeile bleibt
    // −100 ms gilt mit „Words" dem gewaehlten Wort
    panel.waehleWort(1);
    panel.verschiebe(-LyricsSyncPanel::kSchrittMs, false);
    CHECK(worte->item(1)->text().contains(QStringLiteral("00:11.30")));
    CHECK(liste->item(0, 0)->text() == QStringLiteral("00:10.00"));
    // das erste Wort verschieben nimmt die Zeile mit
    panel.waehleWort(0);
    panel.verschiebe(LyricsSyncPanel::kSchrittMs, false);
    CHECK(liste->item(0, 0)->text() == QStringLiteral("00:10.10"));
    // Undo nimmt den Schritt zurueck
    REQUIRE(panel.rueckgaengig());
    CHECK(liste->item(0, 0)->text() == QStringLiteral("00:10.00"));

    // „From here +" verschiebt Zeilen samt ihren Wort-Zeiten
    liste->selectRow(0);
    panel.verschiebe(500, true);
    CHECK(liste->item(0, 0)->text() == QStringLiteral("00:10.50"));
    CHECK(liste->item(1, 0)->text() == QStringLiteral("00:20.50"));
    CHECK(worte->item(2)->text().contains(QStringLiteral("00:12.50")));

    schnappschuss(panel, "lyrics_sync_worte");

    // speichern: die vollstaendige Zeile mit Wort-Zeiten, die halbe als gewoehnliche Zeile
    REQUIRE(panel.speichere());
    const ly::Songtext gefunden = lumi::audio::findeSongtext(t, tmp.path());
    REQUIRE(gefunden.synchron);
    REQUIRE(gefunden.zeilen.size() == 2);
    REQUIRE(gefunden.zeilen[0].worte.size() == 3);
    CHECK(gefunden.zeilen[0].zeitMs == 10500);
    CHECK(gefunden.zeilen[0].worte[1].zeitMs == 11800);
    CHECK(gefunden.zeilen[0].worte[2].zeitMs == 12500);
    CHECK(gefunden.zeilen[0].text == "eins zwei drei");
    CHECK(gefunden.zeilen[1].worte.empty());
    CHECK(gefunden.zeilen[1].zeitMs == 20500);
    CHECK(gefunden.zeilen[1].text == "vier fuenf");
    CHECK(panel.offeneWortZeilen() == 0);  // nach dem Speichern: wie in der Datei

    // „Clear words": die Wort-Zeiten gehen, die Zeit der Zeile bleibt
    liste->selectRow(0);
    REQUIRE(worte->count() == 3);
    CHECK(worte->item(1)->text().contains(QStringLiteral("00:11.80")));
    leeren->click();
    CHECK(worte->item(1)->text().contains(QString::fromUtf8("—")));
    CHECK(liste->item(0, 0)->text() == QStringLiteral("00:10.50"));
    CHECK(panel.geaendert());

    // ohne „Words" gilt Tap wieder der Zeile; vorhandene Wort-Zeiten gehen mit ihr
    REQUIRE(panel.rueckgaengig());  // Wort-Zeiten wieder da
    schalter->setChecked(false);
    CHECK_FALSE(worte->isVisible());
    liste->selectRow(0);
    REQUIRE(panel.tippe(9500));
    CHECK(liste->item(0, 0)->text() == QStringLiteral("00:09.50"));
    CHECK(liste->currentRow() == 1);
    {
        const ly::Songtext v = MultiEffectVisualizer::songtext();
        REQUIRE(v.zeilen[0].worte.size() == 3);
        CHECK(v.zeilen[0].worte[0].zeitMs == 9500);
        CHECK(v.zeilen[0].worte[2].zeitMs == 11500);  // um dieselbe Sekunde mitgegangen
    }

    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    lumi::audio::setzeLaufendenTitel(TrackInfo{});
}

// S88 (Wunsch Patrik): „Resume playback" — waehrend des Abspielens beendet,
// beim naechsten Start genau dort weiter. Geprueft am Ablauf mit einer
// Player-Attrappe; QSettings sind abgeschottet (UiUmgebung).
namespace
{
class ResumeAttrappe : public PlayerAttrappe
{
public:
    IPlaylist* liste = nullptr;
    TrackInfo titel;
    bool spielt = false;
    int position = 0;
    int index = -1;
    QString geladen;      ///< was `load` bekam
    int gestartet = 0;    ///< wie oft `play` lief

    bool load(const QString& pfad) override
    {
        geladen = pfad;
        return true;
    }
    void play() override { ++gestartet; }
    [[nodiscard]] bool isPlaying() const override { return spielt; }
    [[nodiscard]] int positionMs() const override { return position; }
    [[nodiscard]] TrackInfo currentTrack() const override { return titel; }
    [[nodiscard]] bool hasTrack() const override { return !titel.filePath.isEmpty(); }
    [[nodiscard]] IPlaylist* playlist() const override { return liste; }
    [[nodiscard]] int playlistIndex() const override { return index; }
};
}  // namespace

TEST_CASE("PaletteUi S88: Resume playback — Stand beim Beenden, Aufnahme beim Start")
{
    UiUmgebung env;
    AudioUmgebung audio;
    using namespace lumi::audio;
    const QString a = audio.datei(QStringLiteral("a.mp3"));
    const QString b = audio.datei(QStringLiteral("b.mp3"));
    const QString c = audio.datei(QStringLiteral("c.mp3"));
    audio.playlist->addTrack(a);
    audio.playlist->addTrack(b);

    // Vorgabe aus; der Schalter haelt
    CHECK_FALSE(wiederaufnahmeAn());
    setWiederaufnahme(true);
    CHECK(wiederaufnahmeAn());

    // --- beim Beenden: Titel b lief bei 83 s
    ResumeAttrappe alt;
    alt.liste = audio.playlist;
    alt.titel.filePath = b;
    alt.spielt = true;
    alt.position = 83000;
    alt.index = 1;
    speichereWiederaufnahme(standZumBeenden(alt));
    const WiederaufnahmeStand stand = ladeWiederaufnahme();
    CHECK(stand.datei == b);
    CHECK(stand.index == 1);
    CHECK(stand.positionMs == 83000);
    CHECK(stand.lief);

    // --- beim Start: derselbe Eintrag der Playlist, dann der Sprung
    ResumeAttrappe neu;
    neu.liste = audio.playlist;
    REQUIRE(nimmWiederAuf(neu, stand));
    CHECK(neu.gespielt == QList<int>{1});
    CHECK(neu.letzterSprung == 83000);
    CHECK(neu.geladen.isEmpty());  // ueber die Playlist, nicht direkt geladen

    // die Liste hat sich geaendert (b steht jetzt vorn): die Datei wird gesucht
    audio.playlist->clear();
    audio.playlist->addTrack(b);
    audio.playlist->addTrack(a);
    ResumeAttrappe verschoben;
    verschoben.liste = audio.playlist;
    REQUIRE(nimmWiederAuf(verschoben, stand));
    CHECK(verschoben.gespielt == QList<int>{0});

    // der Titel steht nicht in der Playlist: direkt laden und spielen
    WiederaufnahmeStand fremd = stand;
    fremd.datei = c;
    ResumeAttrappe direkt;
    direkt.liste = audio.playlist;
    REQUIRE(nimmWiederAuf(direkt, fremd));
    CHECK(direkt.gespielt.isEmpty());
    CHECK(direkt.geladen == c);
    CHECK(direkt.gestartet == 1);
    CHECK(direkt.letzterSprung == 83000);

    // Gegenproben: pausiert/gestoppt beendet, Datei weg, kein Titel -> es bleibt still
    alt.spielt = false;
    speichereWiederaufnahme(standZumBeenden(alt));
    ResumeAttrappe still;
    still.liste = audio.playlist;
    CHECK_FALSE(nimmWiederAuf(still, ladeWiederaufnahme()));
    CHECK(still.gespielt.isEmpty());
    CHECK(still.gestartet == 0);
    WiederaufnahmeStand weg = stand;
    weg.datei = audio.dir.filePath(QStringLiteral("gibt es nicht.mp3"));
    CHECK_FALSE(nimmWiederAuf(still, weg));
    ResumeAttrappe leer;  // kein Titel geladen
    speichereWiederaufnahme(standZumBeenden(leer));
    CHECK(ladeWiederaufnahme().datei.isEmpty());
    CHECK_FALSE(ladeWiederaufnahme().lief);

    // der Schalter im Player-Panel schreibt dieselbe Einstellung
    PlayerPanel panel(audio.services);
    auto* knopf = panel.findChild<QPushButton*>(QStringLiteral("playerResume"));
    REQUIRE(knopf != nullptr);
    CHECK(knopf->isCheckable());
    CHECK(knopf->isChecked());  // oben eingeschaltet
    knopf->click();
    CHECK_FALSE(wiederaufnahmeAn());
    knopf->click();
    CHECK(wiederaufnahmeAn());
}

// S88 (Entscheid Patrik): `.lvfx2` (Preset-Stapel) traegt ein eigenes Icon —
// im Effect Browser auf einen Blick von einem Einzel-Preset zu unterscheiden.
TEST_CASE("PaletteUi S88: .lvfx2 hat ein eigenes Icon, .lvfx behaelt das der Familie")
{
    UiUmgebung env;
    using lumi::ui::PresetIconKind;
    using lumi::ui::presetTypeIcon;
    using lumi::ui::presetTypeIconForSuffix;
    REQUIRE_FALSE(lumi::ui::presetIconDir().isEmpty());
    // Die Icons selbst sind prozessweite Statics — in diesem Testlauf womoeglich
    // angelegt, bevor es eine QGuiApplication gab (dann leer). Geprueft wird
    // deshalb die ZUORDNUNG (welches Objekt) und die Dateien dahinter.
    const QIcon* einzel = &presetTypeIconForSuffix(QStringLiteral("lvfx"));
    const QIcon* stapel = &presetTypeIconForSuffix(QStringLiteral("LVFX2"));  // Schreibweise egal
    CHECK(stapel != einzel);
    CHECK(einzel == &presetTypeIcon(PresetIconKind::Native));
    CHECK(stapel == &presetTypeIcon(PresetIconKind::NativeStapel));
    // die anderen Formate bleiben, was sie waren
    CHECK(&presetTypeIconForSuffix(QStringLiteral("milk")) ==
          &presetTypeIcon(PresetIconKind::MilkDrop));
    CHECK(&presetTypeIconForSuffix(QStringLiteral("avs")) == &presetTypeIcon(PresetIconKind::Avs));

    // die Datei liegt da, ist ein ICO und eine andere als das Einzel-Icon. (Das
    // Bild selbst laesst sich hier nicht pruefen: in diesem Testlauf liefert
    // QIcon(<.ico>).pixmap() ein leeres Bild, Ursache nicht geklaert — ausserhalb
    // liest Qt die Datei, geprueft mit QImageReader am 2026-10-02.)
    const QString ordner = lumi::ui::presetIconDir();
    const auto bytes = [&ordner](const char* name) {
        QFile f(ordner + QLatin1Char('/') + QLatin1String(name));
        return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
    };
    const QByteArray dateiStapel = bytes("lumiviz_lvfx2.ico");
    const QByteArray dateiEinzel = bytes("lumiviz.ico");
    REQUIRE(dateiStapel.size() > 1000);
    REQUIRE(dateiEinzel.size() > 1000);
    CHECK(dateiStapel.startsWith(QByteArray::fromHex("00000100")));  // ICO-Kopf
    CHECK(dateiStapel != dateiEinzel);
}

// S88: Lyrics Sync kannte kein Rueckgaengig — ein verklopfter Einsatz liess
// sich nur von Hand richten.
TEST_CASE("PaletteUi S88: Lyrics Sync — Undo und Redo fuer jeden Schritt an den Zeiten")
{
    UiUmgebung env;
    AudioUmgebung audio;
    namespace ly = lumi::multieffect::lyrics;
    QTemporaryDir tmp;
    REQUIRE(tmp.isValid());
    TrackInfo t;
    t.filePath = tmp.path() + "/lied.mp3";
    t.title = "Lied";
    t.artist = "Wer";
    const ly::Songtext text = ly::leseLrc("eins\nzwei\ndrei\n");
    lumi::audio::setzeLaufendenTitel(t);
    MultiEffectVisualizer::setSongtext(text);
    MultiEffectVisualizer::setSongtextTitelVersatz(0);
    MultiEffectVisualizer::notifyPlayState(false);

    LyricsSyncPanel panel(audio.services);
    panel.setBenutzerOrdner(tmp.path() + "/nutzer");
    panel.lade(t, text);
    auto* liste = panel.findChild<QTableWidget*>(QStringLiteral("lyricsSyncList"));
    auto* undo = panel.findChild<QPushButton*>(QStringLiteral("lyricsSyncUndo"));
    auto* redo = panel.findChild<QPushButton*>(QStringLiteral("lyricsSyncRedo"));
    REQUIRE(liste != nullptr);
    REQUIRE(undo != nullptr);
    REQUIRE(redo != nullptr);
    const auto zeit = [liste](int r) { return liste->item(r, 0)->text(); };

    // Gegenprobe: frisch geladen gibt es nichts zurueckzunehmen
    CHECK_FALSE(undo->isEnabled());
    CHECK_FALSE(redo->isEnabled());
    CHECK_FALSE(panel.rueckgaengig());
    CHECK_FALSE(panel.wiederholen());

    // drei Schritte: klopfen, klopfen, die erste Zeile verschieben
    REQUIRE(panel.tippe(10000));
    REQUIRE(panel.tippe(20000));
    liste->selectRow(0);
    panel.verschiebe(300, false);
    CHECK(zeit(0) == QStringLiteral("00:10.30"));
    CHECK(zeit(1) == QStringLiteral("00:20.00"));
    CHECK(undo->isEnabled());
    CHECK_FALSE(redo->isEnabled());

    // zurueck in umgekehrter Reihenfolge; die Markierung steht wieder dort,
    // wo der Schritt begann — der Einsatz laesst sich gleich neu klopfen
    undo->click();
    CHECK(zeit(0) == QStringLiteral("00:10.00"));
    CHECK(redo->isEnabled());
    REQUIRE(panel.rueckgaengig());
    CHECK(zeit(1).isEmpty());
    CHECK(liste->currentRow() == 1);
    CHECK(MultiEffectVisualizer::songtext().zeilen.size() == 1);  // das Bild folgt
    REQUIRE(panel.rueckgaengig());
    CHECK(zeit(0).isEmpty());
    CHECK(liste->currentRow() == 0);
    CHECK_FALSE(panel.geaendert());  // wieder der geladene Stand
    CHECK_FALSE(undo->isEnabled());
    // ohne jede Zeit zeigt der Knoten wieder den Text, wie er geladen wurde
    CHECK_FALSE(MultiEffectVisualizer::songtext().synchron);
    CHECK(MultiEffectVisualizer::songtext().zeilen.size() == 3);

    // wieder vor: alle drei Schritte
    REQUIRE(panel.wiederholen());
    redo->click();
    REQUIRE(panel.wiederholen());
    CHECK(zeit(0) == QStringLiteral("00:10.30"));
    CHECK(zeit(1) == QStringLiteral("00:20.00"));
    CHECK(panel.geaendert());
    CHECK_FALSE(redo->isEnabled());

    // ein neuer Schritt nach dem Zuruecknehmen verwirft das Wiederholbare
    REQUIRE(panel.rueckgaengig());
    liste->selectRow(1);
    panel.loescheZeit();
    CHECK(zeit(1).isEmpty());
    CHECK_FALSE(redo->isEnabled());
    REQUIRE(panel.rueckgaengig());
    CHECK(zeit(1) == QStringLiteral("00:20.00"));

    // Zeit von Hand zaehlt als Schritt; ein Schritt, der nichts aendert, nicht
    liste->item(1, 0)->setText(QStringLiteral("00:21.50"));
    REQUIRE(panel.rueckgaengig());
    CHECK(zeit(1) == QStringLiteral("00:20.00"));
    liste->selectRow(2);  // „drei" hat keine Zeit: Verschieben aendert nichts
    panel.verschiebe(100, false);
    REQUIRE(panel.wiederholen());  // das Wiederholbare ist noch da
    CHECK(zeit(1) == QStringLiteral("00:21.50"));

    // Ctrl+Z / Ctrl+Y in der Liste
    QKeyEvent z(QEvent::KeyPress, Qt::Key_Z, Qt::ControlModifier);
    QCoreApplication::sendEvent(liste, &z);
    CHECK(zeit(1) == QStringLiteral("00:20.00"));
    QKeyEvent y(QEvent::KeyPress, Qt::Key_Y, Qt::ControlModifier);
    QCoreApplication::sendEvent(liste, &y);
    CHECK(zeit(1) == QStringLiteral("00:21.50"));

    // Speichern und Laden leeren die Geschichte
    REQUIRE(panel.tippe(30000));
    REQUIRE(panel.speichere());
    CHECK_FALSE(undo->isEnabled());
    CHECK_FALSE(panel.rueckgaengig());
    REQUIRE(panel.tippe(5000));
    CHECK(undo->isEnabled());
    panel.lade(t, text);
    CHECK_FALSE(undo->isEnabled());

    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    lumi::audio::setzeLaufendenTitel(TrackInfo{});
}

// S88 (Entscheid Patrik a): der Versatz nur dieses Titels — im Panel
// einstellbar, sofort am Knoten, festgehalten in offsets.json; Spruenge und
// Klopfen rechnen ihn ein, die Zeiten der Zeilen bleiben, wie sie sind.
TEST_CASE("PaletteUi S88: Lyrics Sync — Versatz je Titel wirkt sofort und wird festgehalten")
{
    UiUmgebung env;
    AudioUmgebung audio;
    namespace ly = lumi::multieffect::lyrics;
    QTemporaryDir tmp;
    REQUIRE(tmp.isValid());
    const QString nutzer = tmp.path() + "/nutzer";
    TrackInfo t;
    t.filePath = tmp.path() + "/lied.mp3";
    t.title = "Lied";
    t.artist = "Wer";
    t.durationMs = 200000;
    const ly::Songtext text = ly::leseLrc("[00:10.00]eins\n[00:20.00]zwei\n");
    lumi::audio::setzeLaufendenTitel(t);
    MultiEffectVisualizer::setSongtext(text);
    MultiEffectVisualizer::setSongtextTitelVersatz(0);
    MultiEffectVisualizer::notifyPlayState(false);

    LyricsSyncPanel panel(audio.services);
    panel.setBenutzerOrdner(nutzer);
    panel.lade(t, text);
    auto* feld = panel.findChild<QSpinBox*>(QStringLiteral("lyricsSyncTrackOffset"));
    auto* liste = panel.findChild<QTableWidget*>(QStringLiteral("lyricsSyncList"));
    REQUIRE(feld != nullptr);
    REQUIRE(liste != nullptr);
    CHECK(feld->isEnabled());
    CHECK(feld->value() == 0);

    // Gegenprobe: ohne Versatz springt der Knopf genau auf die Zeit der Zeile
    REQUIRE(panel.springeZu(1, 0));
    CHECK(audio.player->letzterSprung == 20000);

    // ueber das Feld: sofort am Knoten, in der Ablage, und der Sprung folgt
    feld->setValue(500);  // Text 500 ms frueher
    CHECK(MultiEffectVisualizer::songtextTitelVersatzMs() == 500);
    CHECK(lumi::audio::songtextTitelVersatzMs(t, nutzer) == 500);
    REQUIRE(panel.springeZu(1, 0));
    CHECK(audio.player->letzterSprung == 19500);
    // die Zeiten der Zeilen bleiben, nichts gilt als ungespeichert
    CHECK(liste->item(1, 0)->text() == QStringLiteral("00:20.00"));
    CHECK_FALSE(panel.geaendert());
    // klopfen bei 30 s: die Zeile bekommt die Zeit, zu der der Knoten sie zeigt
    liste->selectRow(0);
    REQUIRE(panel.tippe(30000));
    CHECK(liste->item(0, 0)->text() == QStringLiteral("00:30.50"));

    // ein neu geladenes Panel liest den Versatz aus der Ablage
    LyricsSyncPanel zweites(audio.services);
    zweites.setBenutzerOrdner(nutzer);
    zweites.lade(t, text);
    CHECK(zweites.findChild<QSpinBox*>(QStringLiteral("lyricsSyncTrackOffset"))->value() == 500);

    // laeuft inzwischen ein anderer Titel, bleibt dessen Versatz am Knoten unberuehrt
    TrackInfo anderer = t;
    anderer.filePath = tmp.path() + "/anderes.mp3";
    anderer.title = "Anderes";
    lumi::audio::setzeLaufendenTitel(anderer);
    MultiEffectVisualizer::setSongtextTitelVersatz(0);
    panel.setzeTitelVersatz(-200);
    CHECK(MultiEffectVisualizer::songtextTitelVersatzMs() == 0);
    CHECK(lumi::audio::songtextTitelVersatzMs(t, nutzer) == -200);
    CHECK(lumi::audio::songtextTitelVersatzMs(anderer, nutzer) == 0);

    // ohne Titel ist das Feld gesperrt
    LyricsSyncPanel leer(audio.services);
    CHECK_FALSE(leer.findChild<QSpinBox*>(QStringLiteral("lyricsSyncTrackOffset"))->isEnabled());

    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    MultiEffectVisualizer::setSongtextTitelVersatz(0);
    lumi::audio::setzeLaufendenTitel(TrackInfo{});
}

// S87 (Wunsch Patrik): andere Fassungen eines Songtexts — die Kandidaten
// stehen nach Naehe der Dauer sortiert, die Vorschau zeigt den Text, und
// „Use these lyrics" speichert und zeigt die gewaehlte Fassung.
TEST_CASE("PaletteUi S87: Lyrics Alternatives — Sortierung, Vorschau, Uebernehmen")
{
    UiUmgebung env;
    namespace ly = lumi::multieffect::lyrics;
    QTemporaryDir tmp;
    REQUIRE(tmp.isValid());
    TrackInfo t;
    t.filePath = tmp.path() + "/lied.mp3";
    t.title = "Lied";
    t.artist = "Wer";
    t.durationMs = 240000;
    lumi::audio::setzeLaufendenTitel(t);
    MultiEffectVisualizer::setSongtext(ly::Songtext{});

    LyricsAlternativesDialog::setzeSucheBeimOeffnen(false);  // kein Netz im Test
    LyricsAlternativesDialog dlg(t, 11);
    LyricsAlternativesDialog::setzeSucheBeimOeffnen(true);
    dlg.setBenutzerOrdner(tmp.path() + "/nutzer");
    lumi::audio::sperreSongtext(13, "Wer - Lied", tmp.path() + "/nutzer");

    auto* titel = dlg.findChild<QLineEdit*>(QStringLiteral("lyricsAltTitle"));
    auto* tabelle = dlg.findChild<QTableWidget*>(QStringLiteral("lyricsAltTable"));
    auto* vorschau = dlg.findChild<QPlainTextEdit*>(QStringLiteral("lyricsAltPreview"));
    auto* nehmen = dlg.findChild<QPushButton*>(QStringLiteral("lyricsAltUse"));
    REQUIRE(titel != nullptr);
    REQUIRE(tabelle != nullptr);
    REQUIRE(vorschau != nullptr);
    REQUIRE(nehmen != nullptr);
    CHECK(titel->text() == QStringLiteral("Lied"));  // aus den Tags vorbelegt, aenderbar
    CHECK_FALSE(nehmen->isEnabled());

    dlg.zeigeKandidaten(lumi::audio::kandidatenAusLrclib(
        R"([{"id":11,"trackName":"Lied","artistName":"Wer","albumName":"Kurz","duration":181,)"
        R"("instrumental":false,"plainLyrics":"kurz\n","syncedLyrics":null},)"
        R"({"id":12,"trackName":"Lied","artistName":"Wer","albumName":"Lang","duration":241,)"
        R"("instrumental":false,"plainLyrics":"lang\nmehr\n","syncedLyrics":"[00:01.00] lang\n[00:03.00] mehr\n"},)"
        R"({"id":13,"trackName":"Lied","artistName":"Wer","albumName":"Instr","duration":300,)"
        R"("instrumental":true,"plainLyrics":null,"syncedLyrics":null}])"));
    REQUIRE(tabelle->rowCount() == 3);
    // die Fassung mit der passenden Dauer (241 gegen 240 s) steht oben und ist gewaehlt
    CHECK(tabelle->item(0, 2)->text() == QStringLiteral("Lang"));
    CHECK(tabelle->item(0, 3)->text() == QStringLiteral("4:01 (+1 s)"));
    CHECK(tabelle->item(0, 4)->text() == QStringLiteral("synced"));
    CHECK(tabelle->item(0, 5)->text() == QStringLiteral("2"));  // Zeilenzahl (Befund Patrik)
    CHECK(tabelle->item(1, 2)->text() == QStringLiteral("Kurz"));
    CHECK(tabelle->item(1, 5)->text() == QStringLiteral("1"));
    // die gezeigte Fassung: Vermerk UND fette Zeile
    CHECK(tabelle->item(1, 6)->text() == QStringLiteral("shown now"));
    CHECK(tabelle->item(1, 0)->font().bold());
    CHECK_FALSE(tabelle->item(0, 0)->font().bold());
    CHECK(tabelle->item(2, 4)->text() == QStringLiteral("instrumental"));
    CHECK(tabelle->item(2, 6)->text() == QStringLiteral("blocked"));
    // der Text steht MIT Zeitmarken da
    CHECK(vorschau->toPlainText() == QStringLiteral("[00:01.00] lang\n[00:03.00] mehr\n"));
    CHECK(nehmen->isEnabled());

    // Balken an der Abspielposition: 2 s liegt in der ersten Zeile, 4 s in der zweiten
    dlg.zeigePosition(2000);
    REQUIRE(vorschau->extraSelections().size() == 1);
    CHECK(vorschau->extraSelections().first().cursor.blockNumber() == 0);
    dlg.zeigePosition(4000);
    CHECK(vorschau->extraSelections().first().cursor.blockNumber() == 1);
    dlg.zeigePosition(-1);  // der Titel laeuft nicht (mehr): kein Balken
    CHECK(vorschau->extraSelections().isEmpty());

    // zwei markiert: sie stehen nebeneinander, die dritte Spalte bleibt zu
    auto* vorschau2 = dlg.findChild<QPlainTextEdit*>(QStringLiteral("lyricsAltPreview1"));
    auto* vorschau3 = dlg.findChild<QPlainTextEdit*>(QStringLiteral("lyricsAltPreview2"));
    REQUIRE(vorschau2 != nullptr);
    REQUIRE(vorschau3 != nullptr);
    CHECK(vorschau2->parentWidget()->isHidden());
    tabelle->selectionModel()->select(
        tabelle->model()->index(1, 0),
        QItemSelectionModel::Select | QItemSelectionModel::Rows);
    CHECK_FALSE(vorschau2->parentWidget()->isHidden());
    CHECK(vorschau3->parentWidget()->isHidden());
    CHECK(vorschau2->toPlainText() == QStringLiteral("kurz\n"));  // unsynchron: ohne Marken

    // ein Eintrag ohne Text laesst sich nicht uebernehmen
    tabelle->selectRow(2);
    CHECK_FALSE(nehmen->isEnabled());

    // uebernehmen: gespeichert, angezeigt — der Dialog bleibt offen und
    // kennzeichnet jetzt DIESE Fassung als die gezeigte
    tabelle->selectRow(0);
    dlg.uebernehmen();
    CHECK(QFile::exists(tmp.path() + "/nutzer/Wer - Lied.lrc"));
    const auto a = MultiEffectVisualizer::songtextAngaben();
    CHECK(a.vorhanden);
    CHECK(a.synchron);
    CHECK(lumi::audio::lrclibKennung(a.herkunft) == 12);
    CHECK(tabelle->item(0, 6)->text() == QStringLiteral("shown now"));
    CHECK(tabelle->item(0, 0)->font().bold());
    CHECK(tabelle->item(1, 6)->text().isEmpty());
    CHECK_FALSE(tabelle->item(1, 0)->font().bold());

    // S87 (Wunsch Patrik): der Kopf nennt die Laenge des Titels — der Massstab
    // fuer die Spalte Duration — und die Spielzeit
    auto* kopf = dlg.findChild<QLabel*>(QStringLiteral("lyricsAltHeader"));
    auto* wechsel = dlg.findChild<QWidget*>(QStringLiteral("lyricsAltSwitch"));
    auto* wechselKnopf = dlg.findChild<QPushButton*>(QStringLiteral("lyricsAltSwitchButton"));
    REQUIRE(kopf != nullptr);
    REQUIRE(wechsel != nullptr);
    REQUIRE(wechselKnopf != nullptr);
    MultiEffectVisualizer::notifyPlayState(false);
    MultiEffectVisualizer::notifyPlayPosition(83000);
    dlg.aktualisiere();
    CHECK(kopf->text().contains(QString::fromUtf8("Wer — Lied")));
    CHECK(kopf->text().contains(QStringLiteral("length 4:00")));
    CHECK(kopf->text().contains(QStringLiteral("playing at 1:23")));
    CHECK(wechsel->isHidden());

    // Songwechsel: nicht stillschweigend umschalten — sagen und anbieten
    TrackInfo neu;
    neu.filePath = tmp.path() + "/anderes.mp3";
    neu.title = "Anderes";
    neu.artist = "Wer";
    neu.durationMs = 185000;
    lumi::audio::setzeLaufendenTitel(neu);
    dlg.aktualisiere();
    CHECK_FALSE(wechsel->isHidden());
    CHECK(titel->text() == QStringLiteral("Lied"));           // noch der alte Titel
    CHECK(tabelle->rowCount() == 3);                           // und seine Liste
    CHECK_FALSE(kopf->text().contains(QStringLiteral("playing at")));  // er laeuft nicht mehr
    CHECK(vorschau->extraSelections().isEmpty());              // kein Balken

    // der Knopf nimmt den laufenden Titel: Begriffe neu, Liste leer, Hinweis weg
    LyricsAlternativesDialog::setzeSucheBeimOeffnen(false);
    wechselKnopf->click();
    LyricsAlternativesDialog::setzeSucheBeimOeffnen(true);
    CHECK(titel->text() == QStringLiteral("Anderes"));
    CHECK(tabelle->rowCount() == 0);
    CHECK(wechsel->isHidden());
    CHECK(kopf->text().contains(QStringLiteral("length 3:05")));

    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    MultiEffectVisualizer::setSongtextAbruf(ly::Abruf::Ruhe);
    MultiEffectVisualizer::notifyPlayPosition(0);
    lumi::audio::setzeLaufendenTitel(TrackInfo{});
}

// S86 (Wunsch Patrik): Panel „Tag View" zeigt die Tags des laufenden Titels;
// ein fehlender Tag ist als „—" zu sehen, ohne Titel steht der Hinweis da.
TEST_CASE("PaletteUi S86: Tag View zeigt die Tags des Titels")
{
    UiUmgebung env;
    ServiceContainer dienste;
    TagViewPanel panel(dienste);
    const auto text = [&](const char* name) {
        auto* l = panel.findChild<QLabel*>(QLatin1String(name));
        REQUIRE(l != nullptr);
        return l->text();
    };
    CHECK(text("tagCover") == QStringLiteral("No track loaded"));
    CHECK(text("tagTitle") == QStringLiteral("—"));

    TrackInfo t;
    t.filePath = QStringLiteral("C:/Musik/gibt es nicht.mp3");
    t.title = QStringLiteral("Blue Monday");
    t.artist = QStringLiteral("New Order");
    t.year = QStringLiteral("1983");
    t.trackNumber = QStringLiteral("7");
    t.durationMs = 449000;
    panel.zeige(t);
    CHECK(text("tagTitle") == QStringLiteral("Blue Monday"));
    CHECK(text("tagArtist") == QStringLiteral("New Order"));
    CHECK(text("tagAlbum") == QStringLiteral("—"));  // fehlt: sichtbar als fehlend
    CHECK(text("tagYear") == QStringLiteral("1983"));
    CHECK(text("tagTrack") == QStringLiteral("7"));
    CHECK(text("tagFormat").startsWith(QStringLiteral("7:29")));
    CHECK(text("tagCover") == QStringLiteral("No cover"));  // Datei existiert nicht
}

// S88: Tag View zeigt auch den Songtext, den der Lyrics-Knoten fuehrt — und
// zieht nach, wenn er spaeter kommt (online geholt, nachtraeglich gewaehlt).
TEST_CASE("PaletteUi S88: Tag View zeigt den Songtext des Titels")
{
    namespace ly = lumi::multieffect::lyrics;
    UiUmgebung env;
    ServiceContainer dienste;
    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    TagViewPanel panel(dienste);
    auto* quelle = panel.findChild<QLabel*>(QStringLiteral("tagLyricsSource"));
    auto* feld = panel.findChild<QPlainTextEdit*>(QStringLiteral("tagLyrics"));
    REQUIRE(quelle != nullptr);
    REQUIRE(feld != nullptr);
    CHECK(feld->isReadOnly());

    // Gegenprobe: ohne Text steht nichts da
    panel.zeigeSongtext();
    CHECK(quelle->text() == QStringLiteral("—"));
    CHECK(feld->toPlainText().isEmpty());

    ly::Songtext s = ly::leseLrc("[00:01.00]erste Zeile\n[00:05.00]zweite Zeile\n");
    s.quelle = "user folder: Wer - Lied.lrc";
    MultiEffectVisualizer::setSongtext(s);
    CHECK(feld->toPlainText().isEmpty());  // erst der naechste Takt der Uhr zieht nach
    panel.zeigeSongtext();
    CHECK(feld->toPlainText() == QStringLiteral("erste Zeile\nzweite Zeile"));
    CHECK(quelle->text() == QStringLiteral("user folder: Wer - Lied.lrc · synced · 2 line(s)"));

    // ein anderer (unsynchroner) Text ersetzt ihn; kein Text raeumt ab
    ly::Songtext u = ly::leseLrc("nur Text\n");
    u.quelle = "embedded";
    MultiEffectVisualizer::setSongtext(u);
    panel.zeigeSongtext();
    CHECK(feld->toPlainText() == QStringLiteral("nur Text"));
    CHECK(quelle->text().contains(QStringLiteral("not synced")));
    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    panel.zeigeSongtext();
    CHECK(feld->toPlainText().isEmpty());
    CHECK(quelle->text() == QStringLiteral("—"));
}

// S86 (Wunsch Patrik): der Text-Knoten waehlt die Schrift aus einer Liste und
// hat ein Haekchen Bold, das mit dem Zahlfeld Weight im Gleichschritt bleibt.
TEST_CASE("PaletteUi S86: Text — Schrift-Auswahlliste und Bold")
{
    UiUmgebung env;
    PanelUmgebung p;
    REQUIRE(p.tree != nullptr);
    const auto warte = [] {
        for (int i = 0; i < 3; ++i) QCoreApplication::processEvents();
    };
    lumi::multieffect::TextParams t;
    t.text = "A;B";
    t.fontFace = "Schrift Die Es Nicht Gibt";
    p.vis.chain().children[0].params = t;
    p.tree->setCurrentItem(p.oben(1));
    warte();
    p.tree->setCurrentItem(p.oben(0));
    warte();

    auto* font = p.panel->findChild<QFontComboBox*>(QStringLiteral("fontFace"));
    REQUIRE(font != nullptr);
    CHECK(font->count() > 0);  // installierte Schriften
    // ein importierter, hier nicht installierter Name bleibt stehen
    CHECK(font->currentText() == QStringLiteral("Schrift Die Es Nicht Gibt"));

    auto* bold = p.panel->findChild<QCheckBox*>(QStringLiteral("fontBold"));
    auto* gewicht = p.panel->findChild<QSpinBox*>(QStringLiteral("fontWeight"));
    REQUIRE(bold != nullptr);
    REQUIRE(gewicht != nullptr);
    CHECK_FALSE(bold->isChecked());
    bold->setChecked(true);
    warte();
    CHECK(std::get<lumi::multieffect::TextParams>(p.root().children[0].params).fontWeight == 700);
}

// S86 (Fund Patrik): Text, AVI und Comment standen nie in der Effekt-Palette —
// nur der Import konnte sie anlegen. Waechter: jeder Knotentyp der Variante
// kommt in EffectPalette.cpp vor, ausser den bewusst internen.
TEST_CASE("PaletteUi S86: jeder Knotentyp steht in der Effekt-Palette")
{
    const std::filesystem::path app =
        std::filesystem::path(__FILE__).parent_path().parent_path().parent_path().parent_path();
    const auto lies = [&](const std::filesystem::path& rel) {
        QFile f(QString::fromStdWString((app / rel).wstring()));
        REQUIRE(f.open(QIODevice::ReadOnly));
        return QString::fromUtf8(f.readAll());
    };
    const QString kette =
        lies(std::filesystem::path("include") / "visualizers" / "multieffect" / "EffectChain.hpp");
    const QString palette =
        lies(std::filesystem::path("src") / "UI" / "panels" / "EffectPalette.cpp");

    const int start = kette.indexOf(QStringLiteral("using EffectParams"));
    REQUIRE(start >= 0);
    const QString variante = kette.mid(start, kette.indexOf(QLatin1Char(';'), start) - start);
    // Import-Protokoll und Platzhalter fuer nicht umgesetzte Effekte legt nur
    // der Import an
    const QStringList intern = {QStringLiteral("ImportNotesParams"),
                                QStringLiteral("PassthroughParams")};
    static const QRegularExpression kTyp(QStringLiteral("\\b(\\w+Params)\\b"));
    int gefunden = 0;
    for (auto it = kTyp.globalMatch(variante); it.hasNext();)
    {
        const QString typ = it.next().captured(1);
        if (typ == QStringLiteral("EffectParams") || intern.contains(typ)) continue;
        ++gefunden;
        CHECK_MESSAGE(palette.contains(typ), "fehlt in der Palette: " << typ.toStdString());
    }
    CHECK(gefunden > 85);  // Schutz gegen einen Test, der still nichts prueft
}

TEST_CASE("PaletteUi S85: jeder Skript-Slot nennt auf der Info-Seite seinen Takt")
{
    using lumi::scriptedit::slotScopeHtml;
    const std::filesystem::path app =
        std::filesystem::path(__FILE__).parent_path().parent_path().parent_path().parent_path();
    QFile f(QString::fromStdWString(
        (app / "src" / "UI" / "panels" / "MultiEffectPanel.cpp").wstring()));
    REQUIRE(f.open(QIODevice::ReadOnly));
    const QString text = QString::fromUtf8(f.readAll());

    static const QRegularExpression kSlot(
        QStringLiteral("addScript\\(\"(\\w+)\",\\s*(?:t \\+ )?tr\\(\"([^\"]*)\"\\)"));
    int gefunden = 0;
    for (auto it = kSlot.globalMatch(text); it.hasNext();)
    {
        const auto m = it.next();
        const QString field = m.captured(1);
        QString label = m.captured(2);
        label.replace(QStringLiteral("%1"), QStringLiteral("1"));
        const bool milk = field == QLatin1String("milkdrop") ||
                          field.startsWith(QLatin1String("per")) ||
                          label.contains(QLatin1String("per Frame"));
        const QString typ = milk                              ? QStringLiteral("milkdrop")
                            : field == QLatin1String("code") ? QStringLiteral("movement")
                                                              : QStringLiteral("clear");
        CHECK_MESSAGE(slotScopeHtml(typ, field, label).contains(QLatin1String("Runs")),
                      "ohne Takt: " << field.toStdString() << " / " << label.toStdString());
        ++gefunden;
    }
    CHECK(gefunden > 200);  // der Scan greift (Stand S85: rund 250 Aufrufe)

    // Punkt-Slot nennt die Einheit des Moduls
    CHECK(slotScopeHtml(QStringLiteral("superScope"), QStringLiteral("pointCode"),
                        QStringLiteral("Point"))
              .contains(QLatin1String("point")));
    CHECK(slotScopeHtml(QStringLiteral("texerII"), QStringLiteral("pointCode"),
                        QStringLiteral("Point"))
              .contains(QLatin1String("sprite")));
    // Gegenprobe: ein unbekanntes Feld bekommt keinen Kopf
    CHECK(slotScopeHtml(QStringLiteral("clear"), QStringLiteral("gibtEsNicht"),
                        QStringLiteral("X"))
              .isEmpty());
}
