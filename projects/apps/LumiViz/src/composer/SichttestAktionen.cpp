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

Ergebnis kompositionSchliessen(VisualizerWidget& fenster)
{
    (void)panelFuer(fenster);
    if (Sitzung* sitzung = Sitzung::finde(fenster)) sitzung->entlade();
    return ok();
}

Ergebnis abspielmarke(VisualizerWidget& fenster, const QJsonObject& argumente)
{
    const std::optional<qint64> zeit =
        lumi::sichttest::zeitMs(lumi::sichttest::argument(argumente, QStringLiteral("zeit")));
    if (!zeit.has_value()) return ungueltig(QStringLiteral("Expected: abspielmarke zeit=<m:ss>"));
    Sitzung& sitzung = Sitzung::fuer(fenster);
    if (!sitzung.geladen()) return fehler(QStringLiteral("No composition is loaded."));
    sitzung.springe(*zeit);
    return *zeit > sitzung.laengeMs()
               ? ok(QStringLiteral("The composition ends at %1; the playhead stands there.")
                        .arg(ComposerPanel::zeitText(sitzung.laengeMs())))
               : ok();
}

Ergebnis wiedergabe(VisualizerWidget& fenster, const QJsonObject& argumente)
{
    const QString was = lumi::sichttest::argument(argumente, QStringLiteral("wert")).toLower();
    Sitzung& sitzung = Sitzung::fuer(fenster);
    if (was == QLatin1String("start"))
    {
        if (!sitzung.geladen()) return fehler(QStringLiteral("No composition is loaded."));
        sitzung.spiele();
        return sitzung.laeuft() ? ok() : fehler(QStringLiteral("The composition does not play."));
    }
    if (was == QLatin1String("pause"))
    {
        sitzung.pausiere();
        return ok();
    }
    if (was == QLatin1String("stopp") || was == QLatin1String("stop"))
    {
        sitzung.stoppe();
        return ok();
    }
    return ungueltig(QStringLiteral("Expected: wiedergabe start|pause|stopp"));
}

Ergebnis schleife(VisualizerWidget& fenster, const QJsonObject& argumente)
{
    const std::optional<bool> an = lumi::sichttest::anOderAus(argumente);
    if (!an.has_value()) return ungueltig(QStringLiteral("Expected: schleife an|aus"));
    (void)panelFuer(fenster);
    Sitzung::fuer(fenster).setSchleife(*an);
    return ok();
}

Ergebnis syncKette(VisualizerWidget& fenster, const QJsonObject& argumente)
{
    const std::optional<bool> an = lumi::sichttest::anOderAus(argumente);
    if (!an.has_value()) return ungueltig(QStringLiteral("Expected: sync_kette an|aus"));
    (void)panelFuer(fenster);
    Sitzung::fuer(fenster).setSync(*an);
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

    // --- die weiteren Schalter und der Transport (S105)
    using Mit = Ergebnis (*)(VisualizerWidget&, const QJsonObject&);
    const auto melde = [&anbindung, ziel, ohneFenster](const char* name, const char* beschreibung,
                                                        const char* parameter, Mit aktion) {
        anbindung.aktion(QString::fromLatin1(name), QString::fromLatin1(beschreibung),
                         QString::fromLatin1(parameter),
                         [ziel, ohneFenster, aktion](const QJsonObject& argumente) -> Ergebnis {
                             return ziel != nullptr ? aktion(*ziel, argumente) : ohneFenster();
                         });
    };
    melde("abspielmarke", "Moves the playhead of the Composer", "zeit=<m:ss>",
          &sichttestaktion::abspielmarke);
    melde("wiedergabe", "Transport of the Composer", "start|pause|stopp", &sichttestaktion::wiedergabe);
    melde("schleife", "Switch Loop of the Composer", "an|aus", &sichttestaktion::schleife);
    melde("sync_kette", "Switch Sync on Effect Chain", "an|aus", &sichttestaktion::syncKette);
    anbindung.aktion(
        QStringLiteral("komposition_schliessen"),
        QStringLiteral("Switches the Composer off and removes the composition, without any dialog"),
        QString(),
        [ziel, ohneFenster](const QJsonObject&) -> Ergebnis {
            return ziel != nullptr ? sichttestaktion::kompositionSchliessen(*ziel) : ohneFenster();
        },
        lumi::sichttest::FragtNach | lumi::sichttest::NurErstlauf);
}

}  // namespace lumi::composer
