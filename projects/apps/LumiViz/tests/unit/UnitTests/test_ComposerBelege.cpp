/**
 ****************************************************************************************
 * @file   test_ComposerBelege.cpp
 * @brief  Composer, Stufe C0: Belege zum Renderweg ueber Host-Gruppen
 *         (docs/ui/Composer_Konzept.md §5, Entscheid C3)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Diese Tests bauen NICHTS am Composer. Sie halten fest, was die Host-Gruppen
 * heute koennen und was nicht — die vier Luecken aus dem Konzept §5 (Pegel
 * von aussen, Mischart mit Pegel, Laden nach Bedarf, Tiefenregel). Wo ein
 * Test eine LUECKE festnagelt, steht das am Test: er wird mit der Naht, die
 * die Luecke schliesst, bewusst umgeschrieben.
 *
 * Rueckbau (Konzept §12): diese Datei loeschen, Zeile in Source.cmake nehmen.
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/MultiEffectVisualizer.hpp"
#include "visualizers/multieffect/ChainSerializer.hpp"

#include <QColor>
#include <QDir>
#include <QDirIterator>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QSurfaceFormat>
#include <QTemporaryDir>

#include <algorithm>
#include <memory>

using namespace lumi::multieffect;

namespace
{

constexpr int kGroesse = 64;

/// Host-Gruppe, die ihr Bild einfarbig fuellt.
ChainNode farbGruppe(const std::string& name, uint32_t farbe, BlendMode misch,
                     double ueberblendSekunden = 0.0, bool an = true)
{
    ChainNode g;
    HostGroupParams hp;
    hp.blendOut = misch;
    hp.crossfadeSeconds = ueberblendSekunden;
    g.params = hp;
    g.displayName = name;
    g.enabled = an;
    ChainNode clear;
    ClearParams cp;
    cp.color = farbe;
    clear.params = cp;
    g.children.push_back(std::move(clear));
    return g;
}

ChainNode wurzel()
{
    ChainNode root;
    root.params = ListParams{};
    return root;
}

/// GL-Kontext fuer einen Testfall; `gut` = false ohne GL (Headless).
struct Gl
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QGuiApplication app{argc, argv};
    QOpenGLContext ctx;
    QOffscreenSurface surface;
    bool gut = false;

    Gl()
    {
        QSurfaceFormat fmt;
        fmt.setVersion(3, 3);
        fmt.setProfile(QSurfaceFormat::CoreProfile);
        ctx.setFormat(fmt);
        if (!ctx.create()) return;
        surface.setFormat(ctx.format());
        surface.create();
        gut = surface.isValid() && ctx.makeCurrent(&surface);
    }
};

/// Kette `frames` Bilder laufen lassen, Farbe der Bildmitte.
QColor laufe(MultiEffectVisualizer& vis, int frames, int groesse = kGroesse)
{
    vis.initialize();
    vis.resize(QSize(groesse, groesse));
    QOpenGLContext::currentContext()->functions()->glViewport(0, 0, groesse, groesse);
    for (int i = 0; i < frames; ++i) vis.render(1.0f / 60.0f);
    const QImage img = vis.debugGrabRootSurface();
    REQUIRE(!img.isNull());
    return img.pixelColor(groesse / 2, groesse / 2);
}

QString farbText(const QColor& c)
{
    return QStringLiteral("r %1 g %2 b %3")
        .arg(c.redF(), 0, 'f', 3).arg(c.greenF(), 0, 'f', 3).arg(c.blueF(), 0, 'f', 3);
}

/// Mitgelieferte Presets (asset/presets), vom Ort dieser Datei aus.
QStringList mitgelieferte(int hoechstens)
{
    const QString ordner = QDir::cleanPath(QFileInfo(QString::fromUtf8(__FILE__)).absolutePath() +
                                           QStringLiteral("/../../../../../../asset/presets"));
    QStringList pfade;
    QDirIterator it(ordner, {QStringLiteral("*.avs"), QStringLiteral("*.milk")}, QDir::Files,
                    QDirIterator::Subdirectories);
    while (it.hasNext() && pfade.size() < hoechstens) pfade.append(it.next());
    pfade.sort();
    return pfade;
}

}  // namespace

TEST_SUITE("ComposerBelege")
{
    TEST_CASE("C0 GL: zwei Host-Gruppen laufen zugleich, ein Teilpegel geht heute nur ueber Adjustable")
    {
        Gl gl;
        if (!gl.gut)
        {
            MESSAGE("Kein GL-Kontext moeglich (Headless?) — uebersprungen");
            return;
        }

        SUBCASE("Adjustable mit halbem Pegel mischt Rot und Blau haelftig")
        {
            ChainNode root = wurzel();
            root.children.push_back(farbGruppe("Rot", 0xFF0000, BlendMode::Replace));
            ChainNode blau = farbGruppe("Blau", 0x0000FF, BlendMode::Adjustable);
            std::get<HostGroupParams>(blau.params).outAdjustAlpha = 128;
            root.children.push_back(std::move(blau));
            MultiEffectVisualizer vis;
            vis.setChain(std::move(root));
            const QColor c = laufe(vis, 3);
            MESSAGE("Adjustable 128: " << farbText(c).toStdString());
            CHECK(c.redF() == doctest::Approx(0.5).epsilon(0.05));
            CHECK(c.blueF() == doctest::Approx(0.5).epsilon(0.05));
        }
        SUBCASE("der Pegel laesst sich je Bild per Skript setzen")
        {
            ChainNode root = wurzel();
            root.children.push_back(farbGruppe("Rot", 0xFF0000, BlendMode::Replace));
            ChainNode blau = farbGruppe("Blau", 0x0000FF, BlendMode::Adjustable);
            std::get<HostGroupParams>(blau.params).frameCode = "outadjustalpha = 64;";
            root.children.push_back(std::move(blau));
            MultiEffectVisualizer vis;
            vis.setChain(std::move(root));
            const QColor c = laufe(vis, 3);
            MESSAGE("Adjustable per Skript 64: " << farbText(c).toStdString());
            CHECK(c.blueF() == doctest::Approx(0.25).epsilon(0.08));
            CHECK(c.redF() == doctest::Approx(0.75).epsilon(0.05));
        }
        SUBCASE("drei Gruppen zugleich stapeln sich in Kettenreihenfolge")
        {
            ChainNode root = wurzel();
            root.children.push_back(farbGruppe("Rot", 0x800000, BlendMode::Replace));
            root.children.push_back(farbGruppe("Gruen", 0x008000, BlendMode::Additive));
            root.children.push_back(farbGruppe("Blau", 0x000080, BlendMode::Additive));
            MultiEffectVisualizer vis;
            vis.setChain(std::move(root));
            const QColor c = laufe(vis, 3);
            MESSAGE("drei Gruppen: " << farbText(c).toStdString());
            CHECK(c.redF() == doctest::Approx(0.5).epsilon(0.05));
            CHECK(c.greenF() == doctest::Approx(0.5).epsilon(0.05));
            CHECK(c.blueF() == doctest::Approx(0.5).epsilon(0.05));
        }
    }

    TEST_CASE("C0 GL: das Gewicht wirkt als Pegel — LUECKE: nur zeitgefuehrt, und die Mischart kennt keinen Pegel")
    {
        Gl gl;
        if (!gl.gut)
        {
            MESSAGE("Kein GL-Kontext moeglich (Headless?) — uebersprungen");
            return;
        }

        SUBCASE("Additive mit outAdjustAlpha 64")
        {
            ChainNode root = wurzel();
            root.children.push_back(farbGruppe("Rot", 0x800000, BlendMode::Replace));
            ChainNode blau = farbGruppe("Blau", 0x0000FF, BlendMode::Additive);
            std::get<HostGroupParams>(blau.params).outAdjustAlpha = 64;
            root.children.push_back(std::move(blau));
            MultiEffectVisualizer vis;
            vis.setChain(std::move(root));
            const QColor c = laufe(vis, 3);
            MESSAGE("Additive, Pegel 64: " << farbText(c).toStdString());
            // LUECKE (Naht N2): der Pegel wirkt nur bei Adjustable — Additive
            // legt Blau VOLL dazu, obwohl 64 von 255 verlangt sind.
            CHECK(c.blueF() > 0.95);
            CHECK(c.redF() == doctest::Approx(0.5).epsilon(0.05));
        }
        SUBCASE("eine Gruppe blendet ueber einer stehenden ein (2 s, nach 1 s)")
        {
            for (BlendMode misch : {BlendMode::Replace, BlendMode::Additive})
            {
                ChainNode root = wurzel();
                root.children.push_back(farbGruppe("Rot", 0x800000, BlendMode::Replace));
                root.children.push_back(farbGruppe("Blau", 0x0000FF, misch, 2.0));
                MultiEffectVisualizer vis;
                vis.setChain(std::move(root));
                const QColor c = laufe(vis, 60);
                MESSAGE("Einblenden ueber stehender Gruppe, Mischart "
                        << static_cast<int>(misch) << ", halbe Zeit: " << farbText(c).toStdString());
                // BELEG: das Gewicht (hier 0,5 nach halber Zeit) wirkt als Pegel
                // UEBER dem, was darunter steht — genau die Ueberlagerung.
                CHECK(c.blueF() == doctest::Approx(0.5).epsilon(0.05));
                CHECK(c.redF() == doctest::Approx(0.25).epsilon(0.08));
                // LUECKE (Naht N2): fuer Replace und Additive dasselbe Bild — unter
                // vollem Gewicht wird immer ueberblendet, die Mischart gilt nicht.
                // LUECKE (Naht N1): das Gewicht laeuft mit der ZEIT auf sein Ziel;
                // setzen laesst es sich nicht.
            }
        }
        SUBCASE("Wechsel A -> B (A aus, B an, 2 s, nach 1 s)")
        {
            // Beide starten mit Gewicht 0 bzw. laufen auf ihr Ziel: A war nie an,
            // deshalb zuerst 3 Bilder mit A an, dann umschalten.
            ChainNode root = wurzel();
            root.children.push_back(farbGruppe("Rot", 0xFF0000, BlendMode::Replace, 2.0));
            root.children.push_back(farbGruppe("Blau", 0x0000FF, BlendMode::Replace, 2.0, false));
            MultiEffectVisualizer vis;
            vis.setChain(std::move(root));
            vis.initialize();
            vis.resize(QSize(kGroesse, kGroesse));
            for (int i = 0; i < 180; ++i) vis.render(1.0f / 60.0f);  // A voll da
            // umschalten wie das Auge im Panel: an Ort und Stelle, ohne neue Kette
            vis.chain().children[0].enabled = false;
            vis.chain().children[1].enabled = true;
            for (int i = 0; i < 60; ++i) vis.render(1.0f / 60.0f);
            const QImage img = vis.debugGrabRootSurface();
            REQUIRE(!img.isNull());
            const QColor c = img.pixelColor(kGroesse / 2, kGroesse / 2);
            MESSAGE("Wechsel A->B, halbe Zeit: " << farbText(c).toStdString());
            // BELEG: der Uebergang zweier Gruppen ist ein exakter 50/50-Mix
            CHECK(c.redF() == doctest::Approx(0.5).epsilon(0.05));
            CHECK(c.blueF() == doctest::Approx(0.5).epsilon(0.05));
        }
    }

    TEST_CASE("C0 GL: Tiefenregel — LUECKE: ein .lvfx2 mit Host Switcher bleibt als Clip schwarz")
    {
        Gl gl;
        if (!gl.gut)
        {
            MESSAGE("Kein GL-Kontext moeglich (Headless?) — uebersprungen");
            return;
        }

        // Eine Kette, wie ein .lvfx2 sie traegt: Switcher + zwei Gruppen
        ChainNode stapel = wurzel();
        {
            ChainNode sw;
            HostSwitcherParams sp;
            sp.groups = {"Hell", "Grau"};
            sp.startIndex = 2;
            sw.params = sp;
            stapel.children.push_back(std::move(sw));
            stapel.children.push_back(farbGruppe("Hell", 0xFFFFFF, BlendMode::Replace, 0.0, false));
            stapel.children.push_back(farbGruppe("Grau", 0x404040, BlendMode::Replace, 0.0, false));
        }
        QTemporaryDir tmp;
        REQUIRE(tmp.isValid());
        const QString datei = tmp.filePath(QStringLiteral("Stapel.lvfx2"));
        REQUIRE(saveChainToFile(stapel, datei));

        SUBCASE("allein geladen schaltet der Switcher auf Grau")
        {
            MultiEffectVisualizer vis;
            REQUIRE(vis.loadChainFile(datei));
            const QColor c = laufe(vis, 3);
            MESSAGE(".lvfx2 allein: " << farbText(c).toStdString());
            CHECK(c.redF() == doctest::Approx(0x40 / 255.0).epsilon(0.1));
        }
        SUBCASE("als Inhalt einer Host-Gruppe (= Clip)")
        {
            ChainNode baum;
            QStringList report;
            REQUIRE(MultiEffectVisualizer::buildPresetTree(datei, baum, &report));
            ChainNode clip;
            HostGroupParams hp;
            hp.crossfadeSeconds = 0.0;
            clip.params = hp;
            clip.displayName = "Clip";
            clip.children = MultiEffectVisualizer::presetGroupChildren(std::move(baum));
            ChainNode root = wurzel();
            root.children.push_back(std::move(clip));
            MultiEffectVisualizer vis;
            vis.setChain(std::move(root));
            const QColor c = laufe(vis, 3);
            const ChainNode& drin = vis.chain().children[0];
            int gruppen = 0, listen = 0, switcher = 0;
            for (const ChainNode& k : drin.children)
            {
                if (std::holds_alternative<HostGroupParams>(k.params)) ++gruppen;
                if (std::holds_alternative<ListParams>(k.params)) ++listen;
                if (std::holds_alternative<HostSwitcherParams>(k.params)) ++switcher;
            }
            // LUECKE (Tiefenregel): die Gruppen der Datei werden zu Effect Lists,
            // der Switcher findet nichts mehr zu schalten, und weil er die
            // Gruppen zur Laufzeit einschaltete, ruhen die Listen — schwarz,
            // und der Bericht sagt nichts dazu.
            CHECK(gruppen == 0);
            CHECK(listen == 2);
            CHECK(switcher == 1);
            CHECK(c.redF() + c.greenF() + c.blueF() < 0.05);
            CHECK(report.isEmpty());
            MESSAGE(".lvfx2 als Clip: " << farbText(c).toStdString() << " | Kinder "
                    << drin.children.size() << ", Host-Gruppen " << gruppen << ", Listen "
                    << listen << ", Switcher " << switcher << " | Bericht: "
                    << report.join(QStringLiteral(" / ")).toStdString());
        }
    }

    TEST_CASE("C0 GL: ruhende Clips kosten nichts; Messung der Lade- und Anlaufzeiten")
    {
        Gl gl;
        if (!gl.gut)
        {
            MESSAGE("Kein GL-Kontext moeglich (Headless?) — uebersprungen");
            return;
        }
        const QStringList presets = mitgelieferte(32);
        if (presets.isEmpty())
        {
            MESSAGE("asset/presets nicht gefunden — Messung uebersprungen");
            return;
        }
        constexpr int kBild = 640;

        // Jedes Preset einmal als Baum (das tut ein Presetverweis beim Laden)
        std::vector<ChainNode> clips;
        QElapsedTimer uhr;
        uhr.start();
        for (const QString& p : presets)
        {
            ChainNode baum;
            if (!MultiEffectVisualizer::buildPresetTree(p, baum, nullptr)) continue;
            ChainNode clip;
            HostGroupParams hp;
            hp.crossfadeSeconds = 0.0;
            clip.params = hp;
            clip.displayName = QFileInfo(p).completeBaseName().toStdString();
            clip.enabled = false;  // ruht
            clip.children = MultiEffectVisualizer::presetGroupChildren(std::move(baum));
            clips.push_back(std::move(clip));
        }
        const qint64 msLesen = uhr.elapsed();
        REQUIRE(!clips.empty());
        MESSAGE("MESSUNG " << clips.size() << " Presets lesen und uebersetzen: " << msLesen
                << " ms (" << msLesen / static_cast<qint64>(clips.size()) << " ms je Preset)");

        const auto messe = [&](int anzahl, int laufende) -> std::size_t {
            ChainNode root = wurzel();
            for (int i = 0; i < anzahl; ++i)
            {
                ChainNode c = clips[static_cast<size_t>(i) % clips.size()];
                c.displayName += " #" + std::to_string(i);
                c.enabled = i < laufende;
                root.children.push_back(std::move(c));
            }
            MultiEffectVisualizer vis;
            QElapsedTimer t;
            t.start();
            vis.setChain(std::move(root));
            const qint64 msSetzen = t.restart();
            vis.initialize();
            vis.resize(QSize(kBild, kBild));
            QOpenGLContext::currentContext()->functions()->glViewport(0, 0, kBild, kBild);
            vis.render(1.0f / 60.0f);
            QOpenGLContext::currentContext()->functions()->glFinish();
            const qint64 msErstes = t.restart();
            for (int i = 0; i < 30; ++i) vis.render(1.0f / 60.0f);
            QOpenGLContext::currentContext()->functions()->glFinish();
            const qint64 msDreissig = t.elapsed();
            MESSAGE("MESSUNG " << anzahl << " Clips, " << laufende << " laufen: setChain "
                    << msSetzen << " ms, erstes Bild " << msErstes << " ms, danach "
                    << (static_cast<double>(msDreissig) / 30.0) << " ms je Bild, Laufzeiten "
                    << vis.debugLaufzeitAnzahl());
            return vis.debugLaufzeitAnzahl();
        };
        const std::size_t einer = messe(1, 1);
        CHECK(einer > 0);
        // BELEG: ein ruhender Clip legt keine Laufzeit an — nichts wird
        // kompiliert, bis er laeuft. Viele Clips kosten nur das Lesen.
        CHECK(messe(8, 0) == 0);
        CHECK(messe(32, 0) == 0);
        CHECK(messe(8, 1) == einer);
        messe(8, 2);
        // BELEG: die Zahl der ruhenden Clips aendert nichts an den laufenden
        CHECK(messe(32, 2) == messe(8, 2));

        // LUECKE (Naht N3, Entladen): was ein Clip angelegt hat, bleibt nach
        // seinem Ende liegen — ueber ein langes Programm waechst der Bestand.
        {
            const int anzahl = std::min<int>(8, static_cast<int>(clips.size()));
            ChainNode root = wurzel();
            for (int i = 0; i < anzahl; ++i) root.children.push_back(clips[static_cast<size_t>(i)]);
            MultiEffectVisualizer vis;
            vis.setChain(std::move(root));
            vis.initialize();
            vis.resize(QSize(kBild, kBild));
            std::size_t nachErstem = 0;
            for (int i = 0; i < anzahl; ++i)
            {
                for (int k = 0; k < anzahl; ++k) vis.chain().children[static_cast<size_t>(k)].enabled = k == i;
                for (int f = 0; f < 3; ++f) vis.render(1.0f / 60.0f);
                if (i == 0) nachErstem = vis.debugLaufzeitAnzahl();
            }
            MESSAGE("MESSUNG " << anzahl << " Clips nacheinander: Laufzeiten nach dem ersten "
                    << nachErstem << ", nach allen " << vis.debugLaufzeitAnzahl());
            CHECK(vis.debugLaufzeitAnzahl() > nachErstem);
        }
    }
}
