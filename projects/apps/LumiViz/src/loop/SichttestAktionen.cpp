/**
 * @file SichttestAktionen.cpp
 * @brief Loop: die Aktionen der Sichttest-Steuerung im Panel Loop
 */

#include "loop/SichttestAktionen.hpp"

#include "composer/AppAnbindung.hpp"
#include "loop/LoopPanel.hpp"

#include <QApplication>
#include <QDir>
#include <QStringList>

namespace lumi::loop
{

namespace sichttestaktion
{

using lumi::sichttest::Ergebnis;
using lumi::sichttest::fehler;
using lumi::sichttest::ok;
using lumi::sichttest::ungueltig;

Ergebnis loopLaden(LoopPanel& panel, const QString& root, const QJsonObject& argumente)
{
    const QString angabe = lumi::sichttest::argument(argumente, QStringLiteral("datei"));
    if (angabe.isEmpty()) return ungueltig(QStringLiteral("Argument datei=<path> is missing."));
    const QString datei = lumi::sichttest::loeseAuf(angabe, root);
    if (datei.isEmpty())
        return ungueltig(QStringLiteral("File not found: %1 (relative paths start at %2)")
                             .arg(QDir::toNativeSeparators(angabe), QDir::toNativeSeparators(root)));
    panel.halte();  // jeder Abschnitt beginnt im selben Zustand: nichts spielt
    if (!panel.lade(datei))
        return fehler(QStringLiteral("Not a loop file: %1").arg(QDir::toNativeSeparators(datei)));
    return ok();
}

Ergebnis wiedergabe(LoopPanel& panel, const QJsonObject& argumente)
{
    const QString was = lumi::sichttest::argument(argumente, QStringLiteral("wert")).toLower();
    if (was == QLatin1String("start"))
    {
        if (!panel.geladen()) return fehler(QStringLiteral("No loop is loaded."));
        if (lumi::composer::fuehrtTon())
            return fehler(QStringLiteral("The Composer has the sound; the Loop panel cannot play."));
        if (!panel.spiele()) return fehler(QStringLiteral("The loop could not start (no audio engine or no mixer)."));
        return ok();
    }
    if (was == QLatin1String("stopp") || was == QLatin1String("stop"))
    {
        panel.halte();
        return ok();
    }
    return ungueltig(QStringLiteral("Expected: loop_wiedergabe start|stopp"));
}

Ergebnis editor(LoopPanel& panel, const QJsonObject& argumente, bool zeigen)
{
    const QString basis = lumi::sichttest::argument(argumente, QStringLiteral("basis"));
    if (basis.isEmpty()) return ungueltig(QStringLiteral("Expected: loop_editor basis=<name>"));
    if (!panel.geladen()) return fehler(QStringLiteral("No loop is loaded."));
    if (panel.oeffneEditor(basis, zeigen)) return ok();
    QStringList namen;
    for (const Muster& muster : panel.loop().muster) namen << muster.name;
    return fehler(QStringLiteral("No base named '%1'. Known: %2").arg(basis, namen.join(QStringLiteral(", "))));
}

LoopPanel* panel()
{
    const QWidgetList alle = QApplication::topLevelWidgets();
    for (QWidget* oben : alle)
    {
        if (auto* selbst = qobject_cast<LoopPanel*>(oben)) return selbst;
        if (auto* innen = oben->findChild<LoopPanel*>()) return innen;
    }
    return nullptr;
}

}  // namespace sichttestaktion

void meldeSichttestAktionen(lumi::sichttest::Anbindung& anbindung)
{
    using lumi::sichttest::Ergebnis;
    const QString root = anbindung.root();
    // das Panel wird bei jedem Aufruf gesucht: es kann spaeter entstehen als die Anmeldung
    const auto mitPanel = [](const auto& tu) -> Ergebnis {
        LoopPanel* panel = sichttestaktion::panel();
        return panel != nullptr ? tu(*panel) : lumi::sichttest::fehler(QStringLiteral("There is no Loop panel."));
    };

    anbindung.aktion(
        QStringLiteral("loop_laden"),
        QStringLiteral("Loads a loop (.lvloop) into the Loop panel and stops it, without asking about unsaved changes"),
        QStringLiteral("datei=<path>"),
        [root, mitPanel](const QJsonObject& argumente) -> Ergebnis {
            return mitPanel([&](LoopPanel& panel) { return sichttestaktion::loopLaden(panel, root, argumente); });
        },
        lumi::sichttest::FragtNach);
    anbindung.aktion(QStringLiteral("loop_wiedergabe"), QStringLiteral("Play and Stop of the Loop panel"),
                     QStringLiteral("start|stopp"), [mitPanel](const QJsonObject& argumente) -> Ergebnis {
                         return mitPanel([&](LoopPanel& panel) { return sichttestaktion::wiedergabe(panel, argumente); });
                     });
    anbindung.aktion(QStringLiteral("loop_editor"), QStringLiteral("Opens the editor of a base of the loaded loop (drum editor or instrument editor)"),
                     QStringLiteral("basis=<name>"), [mitPanel](const QJsonObject& argumente) -> Ergebnis {
                         return mitPanel([&](LoopPanel& panel) { return sichttestaktion::editor(panel, argumente); });
                     });
}

}  // namespace lumi::loop
