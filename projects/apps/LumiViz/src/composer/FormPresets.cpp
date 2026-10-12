/**
 * @file FormPresets.cpp
 * @brief Composer: Kurvenformen als Presets (Stufe C4 Teil 2)
 */

#include "composer/FormPresets.hpp"

#include "core/Mitgeliefert.hpp"
#include "visualizers/multieffect/NodePresetStore.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>

#include <algorithm>

namespace lumi::composer::formpresets
{

namespace
{

const QString kFormat = QStringLiteral("lumiviz-curve");
constexpr int kVersion = 1;

/// Test-Wurzel (leer = normales Verhalten). Nur ueber setzeWurzelFuerTests gesetzt.
QString& testWurzel()
{
    static QString wurzel;
    return wurzel;
}

QString unterordner(FormArt art)
{
    return art == FormArt::Stueck ? QStringLiteral("composer/shapes") : QStringLiteral("composer/fades");
}

/// Alle Presets eines Verzeichnisses als Eintraege, nach Namen sortiert.
void sammle(const QString& ordner, FormArt art, bool mitgeliefert, std::vector<Eintrag>& aus)
{
    if (ordner.isEmpty()) return;
    const QDir dir(ordner);
    if (!dir.exists()) return;
    const QStringList dateien = dir.entryList({QLatin1Char('*') + QLatin1String(endung(art))},
                                              QDir::Files, QDir::Name | QDir::IgnoreCase);
    for (const QString& datei : dateien)
        aus.push_back({QFileInfo(datei).completeBaseName(), dir.filePath(datei), mitgeliefert});
}

/// Neben der Exe oder im Projektordner (core/Mitgeliefert.hpp, S114).
QString sucheAsset(const QString& unter)
{
    return lumi::core::mitgeliefert::ordner(unter);
}

}  // namespace

const char* endung(FormArt art)
{
    return art == FormArt::Stueck ? ".lvshape" : ".lvfade";
}

QString assetOrdner(FormArt art)
{
    if (!testWurzel().isEmpty())
        return QDir(testWurzel()).filePath(QStringLiteral("asset/") + unterordner(art));
    // einmal aufgeloest und gemerkt
    static const QString kBlenden = sucheAsset(unterordner(FormArt::Blende));
    static const QString kStuecke = sucheAsset(unterordner(FormArt::Stueck));
    return art == FormArt::Stueck ? kStuecke : kBlenden;
}

QString benutzerOrdner(FormArt art)
{
    if (!testWurzel().isEmpty())
        return QDir(testWurzel()).filePath(QStringLiteral("user/") + unterordner(art));
    const QString basis = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (basis.isEmpty()) return {};
    return QDir(basis).filePath(unterordner(art));
}

std::vector<Eintrag> liste(FormArt art)
{
    std::vector<Eintrag> eigene;
    sammle(benutzerOrdner(art), art, false, eigene);
    std::vector<Eintrag> aus;
    sammle(assetOrdner(art), art, true, aus);
    // ein eigenes mit dem Namen eines mitgelieferten verdraengt es
    aus.erase(std::remove_if(aus.begin(), aus.end(),
                             [&eigene](const Eintrag& e) {
                                 return std::any_of(eigene.cbegin(), eigene.cend(), [&e](const Eintrag& o) {
                                     return QString::compare(o.name, e.name, Qt::CaseInsensitive) == 0;
                                 });
                             }),
              aus.end());
    aus.insert(aus.end(), eigene.begin(), eigene.end());
    return aus;
}

Eintrag finde(FormArt art, const QString& name)
{
    for (const Eintrag& e : liste(art))
        if (QString::compare(e.name, name, Qt::CaseInsensitive) == 0) return e;
    return {};
}

bool lade(const QString& pfad, FormArt art, Pegelform& form)
{
    QFile datei(pfad);
    if (!datei.open(QIODevice::ReadOnly)) return false;
    QJsonParseError fehler;
    const QJsonDocument doc = QJsonDocument::fromJson(datei.readAll(), &fehler);
    if (fehler.error != QJsonParseError::NoError || !doc.isObject()) return false;
    const QJsonObject wurzel = doc.object();
    if (wurzel.value(QStringLiteral("format")).toString() != kFormat) return false;
    Pegelform gelesen;
    if (!formAusJson(wurzel, gelesen) || gelesen.art != art) return false;
    form = std::move(gelesen);
    return true;
}

bool speichere(const QString& name, const Pegelform& form, QString* pfad)
{
    const QString sauber = lumi::multieffect::nodepresets::sanitize(name);
    const QString ordner = benutzerOrdner(form.art);
    if (sauber.isEmpty() || ordner.isEmpty()) return false;
    if (!QDir().mkpath(ordner)) return false;

    Pegelform gerichtet = form;
    gerichtet.richte();
    QJsonObject wurzel = formAlsJson(gerichtet);
    wurzel.insert(QStringLiteral("format"), kFormat);
    wurzel.insert(QStringLiteral("version"), kVersion);

    const QString ziel = QDir(ordner).filePath(sauber + QLatin1String(endung(form.art)));
    QSaveFile datei(ziel);
    if (!datei.open(QIODevice::WriteOnly)) return false;
    if (datei.write(QJsonDocument(wurzel).toJson(QJsonDocument::Indented)) < 0 || !datei.commit())
        return false;
    if (pfad != nullptr) *pfad = ziel;
    return true;
}

bool loesche(FormArt art, const Eintrag& eintrag)
{
    if (eintrag.mitgeliefert || eintrag.pfad.isEmpty()) return false;
    // nur, was wirklich im Benutzerordner liegt
    const QString ordner = QDir::cleanPath(benutzerOrdner(art));
    if (ordner.isEmpty() || QDir::cleanPath(QFileInfo(eintrag.pfad).absolutePath())
                                    .compare(ordner, Qt::CaseInsensitive) != 0)
        return false;
    return QFile::remove(eintrag.pfad);
}

void setzeWurzelFuerTests(const QString& wurzel)
{
    testWurzel() = wurzel;
}

}  // namespace lumi::composer::formpresets
