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

#include "UI/panels/EelScriptEditing.hpp"
#include "UI/panels/EffectPalette.hpp"
#include "UI/panels/EffectPaletteWindow.hpp"
#include "UI/panels/MultiEffectPanel.hpp"
#include "services/EventBus.hpp"
#include "services/ServiceContainer.hpp"
#include "services/events/UIEvents.hpp"
#include "visualizers/MultiEffectVisualizer.hpp"
#include "visualizers/multieffect/NodePresetStore.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMimeData>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextLayout>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItemIterator>

#include <QAbstractButton>
#include <QMutex>

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
