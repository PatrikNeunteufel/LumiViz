/**
 * @file SichttestAktionen.cpp
 * @brief Composer: die Aktionen der Sichttest-Steuerung (Konzept §12.2 Naht N21)
 */

#include "composer/SichttestAktionen.hpp"

#include "UI/widgets/VisualizerWidget.hpp"
#include "composer/ComposerPanel.hpp"
#include "composer/Sitzung.hpp"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QPointer>
#include <QStringList>

namespace lumi::composer
{

namespace sichttestaktion
{

using lumi::sichttest::Ergebnis;
using lumi::sichttest::fehler;
using lumi::sichttest::ok;
using lumi::sichttest::ungueltig;

ComposerPanel* panelFuer(VisualizerWidget& fenster)
{
    ComposerPanel* panel = nullptr;
    if (QWidget* oben = fenster.window()) panel = oben->findChild<ComposerPanel*>();
    if (panel == nullptr)
    {
        // abgedockt ist das Panel sein eigenes Fenster
        const QWidgetList alle = QApplication::topLevelWidgets();
        for (QWidget* w : alle)
        {
            panel = qobject_cast<ComposerPanel*>(w);
            if (panel == nullptr) panel = w->findChild<ComposerPanel*>();
            if (panel != nullptr) break;
        }
    }
    // ein Panel, das noch nie gezeigt wurde, kennt sein Fenster noch nicht
    if (panel != nullptr) panel->setFenster(&fenster);
    return panel;
}

Ergebnis kompositionLaden(VisualizerWidget& fenster, const QString& root, const QJsonObject& argumente)
{
    QString angabe = argumente.value(QStringLiteral("datei")).toString();
    if (angabe.isEmpty()) angabe = argumente.value(QStringLiteral("wert")).toString();
    if (angabe.trimmed().isEmpty()) return ungueltig(QStringLiteral("Argument datei=<path> is missing."));
    const QString datei = lumi::sichttest::loeseAuf(angabe, root);
    if (datei.isEmpty())
        return ungueltig(QStringLiteral("File not found: %1 (relative paths start at %2)")
                             .arg(QDir::toNativeSeparators(angabe), QDir::toNativeSeparators(root)));

    (void)panelFuer(fenster);  // das Panel folgt der Sitzung, auch wenn es noch nie gezeigt wurde
    Sitzung& sitzung = Sitzung::fuer(fenster);
    QStringList bericht;
    if (!sitzung.lade(datei, &bericht))
        return fehler(QStringLiteral("Could not load the composition: %1")
                          .arg(QDir::toNativeSeparators(datei)));
    if (Sitzung::sichttestSpieltNachLaden()) sitzung.spiele();  // schaltet den Composer ein
    return ok(bericht.isEmpty()
                  ? QString()
                  : QStringLiteral("Loaded with remarks: %1").arg(bericht.join(QStringLiteral(" | "))));
}

Ergebnis composer(VisualizerWidget& fenster, const QJsonObject& argumente)
{
    const std::optional<bool> an = lumi::sichttest::anOderAus(argumente);
    if (!an.has_value()) return ungueltig(QStringLiteral("Expected: composer an|aus"));
    Sitzung& sitzung = Sitzung::fuer(fenster);
    if (*an && !sitzung.geladen()) return fehler(QStringLiteral("No composition is loaded."));
    sitzung.setAn(*an);
    if (sitzung.an() != *an)
        return fehler(QStringLiteral("The Composer could not be switched %1.")
                          .arg(*an ? QStringLiteral("on") : QStringLiteral("off")));
    return ok();
}

Ergebnis edit(VisualizerWidget& fenster, const QJsonObject& argumente)
{
    const std::optional<bool> an = lumi::sichttest::anOderAus(argumente);
    if (!an.has_value()) return ungueltig(QStringLiteral("Expected: edit an|aus"));
    ComposerPanel* panel = panelFuer(fenster);
    if (panel == nullptr) return fehler(QStringLiteral("There is no Composer panel."));
    panel->setBearbeiten(*an);
    return ok();
}

Ergebnis fit(VisualizerWidget& fenster)
{
    ComposerPanel* panel = panelFuer(fenster);
    if (panel == nullptr) return fehler(QStringLiteral("There is no Composer panel."));
    panel->zeitachse()->setzeZoomZurueck();
    return ok();
}

}  // namespace sichttestaktion

void meldeSichttestAktionen(lumi::sichttest::Anbindung& anbindung, VisualizerWidget& fenster)
{
    using lumi::sichttest::Ergebnis;
    const QPointer<VisualizerWidget> ziel(&fenster);
    const QString root = anbindung.root();
    const auto ohneFenster = [] {
        return lumi::sichttest::fehler(QStringLiteral("The visualizer window is gone."));
    };

    anbindung.aktion(
        QStringLiteral("komposition_laden"),
        QStringLiteral("Loads a composition (.lvcomp) into the Composer, without any dialog"),
        QStringLiteral("datei=<path>"),
        [ziel, root, ohneFenster](const QJsonObject& argumente) -> Ergebnis {
            return ziel != nullptr ? sichttestaktion::kompositionLaden(*ziel, root, argumente)
                                   : ohneFenster();
        },
        lumi::sichttest::FragtNach);
    anbindung.aktion(QStringLiteral("composer"), QStringLiteral("Switch Composer on"),
                     QStringLiteral("an|aus"),
                     [ziel, ohneFenster](const QJsonObject& argumente) -> Ergebnis {
                         return ziel != nullptr ? sichttestaktion::composer(*ziel, argumente)
                                                : ohneFenster();
                     });
    anbindung.aktion(QStringLiteral("edit"), QStringLiteral("Switch Edit of the Composer panel"),
                     QStringLiteral("an|aus"),
                     [ziel, ohneFenster](const QJsonObject& argumente) -> Ergebnis {
                         return ziel != nullptr ? sichttestaktion::edit(*ziel, argumente)
                                                : ohneFenster();
                     });
    anbindung.aktion(QStringLiteral("fit"),
                     QStringLiteral("Button Fit: whole length, rows at normal height"), QString(),
                     [ziel, ohneFenster](const QJsonObject&) -> Ergebnis {
                         return ziel != nullptr ? sichttestaktion::fit(*ziel) : ohneFenster();
                     });
}

}  // namespace lumi::composer
