/**
 * @file Datensicherung.cpp
 * @brief Die eigenen Daten exportieren und importieren (Datenablage_Konzept.md, A4)
 */

#include "core/Datensicherung.hpp"

#include "core/ZipArchiv.hpp"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <QSet>
#include <QSettings>
#include <QStandardPaths>

#include <algorithm>

namespace lumi::core::datensicherung
{

namespace
{

const QString kManifest = QStringLiteral("manifest.json");
const QString kEinstellungen = QStringLiteral("settings.json");
const QString kDaten = QStringLiteral("data/");

/// Ein Pfad aus einem Archiv, dem man trauen darf: relativ, mit `/`, ohne
/// `.`/`..`, ohne Laufwerk, ohne leeres Stueck.
bool pfadInOrdnung(const QString& rel)
{
    if (rel.isEmpty() || rel.startsWith(QLatin1Char('/')) || rel.endsWith(QLatin1Char('/'))) return false;
    if (rel.contains(QLatin1Char('\\')) || rel.contains(QLatin1Char(':'))) return false;
    const QStringList stuecke = rel.split(QLatin1Char('/'));
    return std::none_of(stuecke.begin(), stuecke.end(), [](const QString& s) {
        return s.isEmpty() || s == QLatin1String(".") || s == QLatin1String("..") || s.trimmed() != s;
    });
}

/// Der Bereich, in dessen Ordner `rel` liegt und der die Datei auch nimmt.
const Bereich* bereichVon(const QString& rel)
{
    const QString name = rel.mid(rel.lastIndexOf(QLatin1Char('/')) + 1);
    for (const Bereich& bereich : bereiche())
        for (const QString& ordner : bereich.ordner)
            if (rel.startsWith(ordner + QLatin1Char('/'), Qt::CaseInsensitive))
                return bereich.ohne.contains(name, Qt::CaseInsensitive) ? nullptr : &bereich;
    return nullptr;
}

QByteArray lies(const QString& pfad, bool* ok)
{
    QFile datei(pfad);
    *ok = datei.open(QIODevice::ReadOnly);
    return *ok ? datei.readAll() : QByteArray();
}

struct Archiv
{
    bool ok = false;
    QString fehler;
    QJsonObject manifest;
    std::vector<ZipEintrag> eintraege;
};

Archiv liesArchiv(const QString& datei)
{
    Archiv archiv;
    if (!leseZip(datei, archiv.eintraege, &archiv.fehler)) return archiv;
    const auto manifest = std::find_if(archiv.eintraege.begin(), archiv.eintraege.end(),
                                       [](const ZipEintrag& e) { return e.name == kManifest; });
    if (manifest == archiv.eintraege.end())
    {
        archiv.fehler = QStringLiteral("This is not a LumiViz data file (no manifest).");
        return archiv;
    }
    archiv.manifest = QJsonDocument::fromJson(manifest->daten).object();
    if (archiv.manifest.value(QStringLiteral("format")).toString() != QLatin1String(kFormat))
    {
        archiv.fehler = QStringLiteral("This is not a LumiViz data file.");
        return archiv;
    }
    if (archiv.manifest.value(QStringLiteral("version")).toInt(0) > kFassung)
    {
        archiv.fehler = QStringLiteral("This file was exported with a newer LumiViz (%1). Update LumiViz to import it.")
                            .arg(archiv.manifest.value(QStringLiteral("appVersion")).toString());
        return archiv;
    }
    archiv.ok = true;
    return archiv;
}

// --- Einstellungen: Wert <-> JSON --------------------------------------------

QJsonValue alsJson(const QVariant& wert)
{
    switch (wert.typeId())
    {
        case QMetaType::Bool: return wert.toBool();
        case QMetaType::Int:
        case QMetaType::UInt:
        case QMetaType::LongLong:
        case QMetaType::ULongLong: return wert.toLongLong();
        case QMetaType::Double:
        case QMetaType::Float: return wert.toDouble();
        case QMetaType::QStringList: return QJsonArray::fromStringList(wert.toStringList());
        case QMetaType::QVariantList:
        {
            QJsonArray feld;
            for (const QVariant& stueck : wert.toList()) feld.append(alsJson(stueck));
            return feld;
        }
        case QMetaType::QByteArray:
            return QJsonObject{{QStringLiteral("base64"), QString::fromLatin1(wert.toByteArray().toBase64())}};
        default: return wert.toString();
    }
}

QVariant ausJson(const QJsonValue& wert)
{
    if (wert.isBool()) return wert.toBool();
    if (wert.isDouble())
    {
        const double zahl = wert.toDouble();
        const auto ganz = static_cast<qlonglong>(zahl);
        return static_cast<double>(ganz) == zahl ? QVariant(ganz) : QVariant(zahl);
    }
    if (wert.isArray())
    {
        const QJsonArray feld = wert.toArray();
        const bool nurText = std::all_of(feld.begin(), feld.end(), [](const QJsonValue& s) { return s.isString(); });
        if (nurText)
        {
            QStringList liste;
            for (const QJsonValue& stueck : feld) liste << stueck.toString();
            return liste;
        }
        QVariantList liste;
        for (const QJsonValue& stueck : feld) liste << ausJson(stueck);
        return liste;
    }
    if (wert.isObject())
        return QByteArray::fromBase64(wert.toObject().value(QStringLiteral("base64")).toString().toLatin1());
    return wert.toString();
}

}  // namespace

const std::vector<Bereich>& bereiche()
{
    static const std::vector<Bereich> kBereiche = {
        {QStringLiteral("visuals"), QStringLiteral("Visualizer presets"),
         {QStringLiteral("presets/visuals"), QStringLiteral("presets/smoothing"), QStringLiteral("presets/audio")},
         {}, true},
        {QStringLiteral("gradients"), QStringLiteral("Gradients"), {QStringLiteral("presets/gradients")}, {}, true},
        {QStringLiteral("nodepresets"), QStringLiteral("Module presets"), {QStringLiteral("nodepresets")}, {}, true},
        {QStringLiteral("shadertemplates"), QStringLiteral("Shader templates"), {QStringLiteral("shadertemplates")},
         {}, true},
        // `current.lvkar` ist der laufende Stand der Karaoke-Ebene, kein Preset
        {QStringLiteral("karaoke"), QStringLiteral("Karaoke presets and trails"), {QStringLiteral("karaoke")},
         {QStringLiteral("current.lvkar")}, true},
        {QStringLiteral("composer"), QStringLiteral("Composer fades, shapes, mutes"), {QStringLiteral("composer")},
         {}, true},
        {QStringLiteral("shadertoy"), QStringLiteral("Shadertoy imports"), {QStringLiteral("shadertoy")}, {}, true},
        {QStringLiteral("instruments"), QStringLiteral("Instruments"), {QStringLiteral("instruments")}, {}, true},
        // eigene Kits (Loop, Stufe 21a; Entscheid K3 a, S123): Muster nennen sie als `user:<name>`
        {QStringLiteral("kits"), QStringLiteral("Kits"), {QStringLiteral("kits")}, {}, true},
        // eigene Akkorde (Loop, Stufe 26a; Entscheid S-T3 a, S137): Griffbilder und Voicings als `user:<name>`
        {QStringLiteral("chords"), QStringLiteral("Chords"), {QStringLiteral("chords")}, {}, true},
        // eigene Muster, Clips und Spuren (Loop, Stufe 33; Entscheid PE1-1 a, S167): `.lvpattern`
        // unter `patterns`, `.lvclip` und `.lvtrack` unter `tracks`
        {QStringLiteral("patterns"), QStringLiteral("Patterns"), {QStringLiteral("patterns")}, {}, true},
        {QStringLiteral("tracks"), QStringLiteral("Tracks and clips"), {QStringLiteral("tracks")}, {}, true},
        // eigene Samples (Loop, Stufe 11a; Entscheid S1 a): Instrumente nennen sie als `user:<name>`
        {QStringLiteral("samples"), QStringLiteral("Samples"), {QStringLiteral("samples")}, {}, true},
        // eigene Formen (Klang aus einer Form, Stufe 19a; Entscheid D2 a): Schichten nennen sie als `user:<name>`
        {QStringLiteral("shapes"), QStringLiteral("Shapes"), {QStringLiteral("contours")}, {}, true},
        // geladene Songtexte mit den Versaetzen (`offsets.json`); `notfound.json`
        // merkt nur Fehlsuchen
        {QStringLiteral("lyrics"), QStringLiteral("Downloaded lyrics"), {QStringLiteral("lyrics")},
         {QStringLiteral("notfound.json")}, false},
    };
    return kBereiche;
}

const Bereich* findeBereich(const QString& id)
{
    const auto& alle = bereiche();
    const auto treffer = std::find_if(alle.begin(), alle.end(), [&id](const Bereich& b) { return b.id == id; });
    return treffer == alle.end() ? nullptr : &*treffer;
}

QString benutzerOrdner()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

std::vector<Datei> sammle(const QString& benutzerOrdner, const Bereich& bereich)
{
    std::vector<Datei> dateien;
    if (benutzerOrdner.isEmpty()) return dateien;
    const QDir wurzel(benutzerOrdner);
    for (const QString& ordner : bereich.ordner)
    {
        const QString pfad = wurzel.filePath(ordner);
        if (!QFileInfo(pfad).isDir()) continue;
        QDirIterator lauf(pfad, QDir::Files, QDirIterator::Subdirectories);
        while (lauf.hasNext())
        {
            const QFileInfo info(lauf.next());
            if (bereich.ohne.contains(info.fileName(), Qt::CaseInsensitive)) continue;
            dateien.push_back({wurzel.relativeFilePath(info.absoluteFilePath()), info.size(), info.lastModified()});
        }
    }
    std::sort(dateien.begin(), dateien.end(),
              [](const Datei& a, const Datei& b) { return QString::compare(a.rel, b.rel, Qt::CaseInsensitive) < 0; });
    return dateien;
}

// --- Einstellungen ----------------------------------------------------------

bool einstellungGehtMit(const QString& schluessel)
{
    // ganze Gruppen
    static const QStringList kGruppen = {
        QStringLiteral("shortcuts/"),
        QStringLiteral("milkdrop/"),
        QStringLiteral("configpanel/preview/"),
    };
    // einzelne Schluessel
    static const QSet<QString> kEinzeln = {
        QStringLiteral("effectPalette/favorites"),
        QStringLiteral("effectPalette/recent"),
        // das Fenster „Instruments" (S114, Stufe 14c): Namen, wie ein Muster sie nennt
        QStringLiteral("instrumentPalette/favorites"),
        QStringLiteral("instrumentPalette/recent"),
        // E1 a: Favoriten der Presets, volle Dateipfade — sie gehen mit, wie sie sind
        QStringLiteral("presets/chain/favorites"),
        QStringLiteral("presets/karaoke/favorites"),
        QStringLiteral("player/shuffle"),
        QStringLiteral("player/repeatMode"),
        QStringLiteral("player/resumePlayback"),
        QStringLiteral("composer/atEnd"),
        QStringLiteral("composer/loop"),
        QStringLiteral("composer/resume"),
        QStringLiteral("composer/playerResumes"),
        QStringLiteral("composer/resumesWhenSwitchedOn"),
        QStringLiteral("composer/newClipSeconds"),
        QStringLiteral("composer/rowHeightPercent"),
        QStringLiteral("composer/syncEffectChain"),
        QStringLiteral("composer/muteProfile"),
        QStringLiteral("import/avsRenderScaleDivisor"),
        QStringLiteral("editor/indentWidth"),
        QStringLiteral("editor/spaceAroundOperators"),
        QStringLiteral("editor/maxBlankLines"),
        QStringLiteral("lyrics/onlineSearch"),
        QStringLiteral("lyrics/saveLocation"),
        QStringLiteral("lyricsSync/prerollMs"),
        QStringLiteral("karaoke/enabled"),
        QStringLiteral("loop/drumRightClickDeletes"),
        QStringLiteral("loop/notesRightClickDeletes"),
        QStringLiteral("kamera/aufnahmeDauer"),
    };
    if (kEinzeln.contains(schluessel)) return true;
    return std::any_of(kGruppen.begin(), kGruppen.end(),
                       [&schluessel](const QString& gruppe) { return schluessel.startsWith(gruppe); });
}

QJsonObject leseEinstellungen(QSettings& einstellungen)
{
    QJsonObject werte;
    const QStringList alle = einstellungen.allKeys();
    for (const QString& schluessel : alle)
        if (einstellungGehtMit(schluessel)) werte.insert(schluessel, alsJson(einstellungen.value(schluessel)));
    return werte;
}

int schreibeEinstellungen(QSettings& einstellungen, const QJsonObject& werte)
{
    int gesetzt = 0;
    for (auto it = werte.begin(); it != werte.end(); ++it)
    {
        if (!einstellungGehtMit(it.key())) continue;
        einstellungen.setValue(it.key(), ausJson(it.value()));
        ++gesetzt;
    }
    return gesetzt;
}

// --- Export -----------------------------------------------------------------

ExportErgebnis exportiere(const QString& benutzerOrdner, const ExportWahl& wahl, const QJsonObject& einstellungen,
                          const QString& ziel, const QString& appVersion)
{
    ExportErgebnis ergebnis;
    const QDir wurzel(benutzerOrdner);
    std::vector<ZipEintrag> eintraege;
    QJsonObject gruppen;
    for (const Bereich& bereich : bereiche())
    {
        if (!wahl.bereiche.contains(bereich.id)) continue;
        const std::vector<Datei> dateien = sammle(benutzerOrdner, bereich);
        if (dateien.empty()) continue;
        for (const Datei& datei : dateien)
        {
            bool ok = false;
            QByteArray daten = lies(wurzel.filePath(datei.rel), &ok);
            if (!ok)
            {
                ergebnis.fehler = QStringLiteral("Could not read '%1'.").arg(QDir::toNativeSeparators(datei.rel));
                return ergebnis;
            }
            eintraege.push_back({kDaten + datei.rel, std::move(daten), datei.geaendert});
        }
        gruppen.insert(bereich.id, static_cast<int>(dateien.size()));
        ergebnis.dateien += static_cast<int>(dateien.size());
        ++ergebnis.bereiche;
    }

    const QJsonObject manifest{
        {QStringLiteral("format"), QLatin1String(kFormat)},
        {QStringLiteral("version"), kFassung},
        {QStringLiteral("app"), QStringLiteral("LumiViz")},
        {QStringLiteral("appVersion"), appVersion},
        {QStringLiteral("exported"), QDateTime::currentDateTime().toString(Qt::ISODate)},
        {QStringLiteral("groups"), gruppen},
        {QStringLiteral("settings"), wahl.einstellungen},
    };
    // das Manifest zuerst: wer die Datei von Hand oeffnet, sieht es oben
    eintraege.insert(eintraege.begin(),
                     {kManifest, QJsonDocument(manifest).toJson(QJsonDocument::Indented), QDateTime()});
    if (wahl.einstellungen)
        eintraege.push_back(
            {kEinstellungen, QJsonDocument(einstellungen).toJson(QJsonDocument::Indented), QDateTime()});

    ergebnis.ok = schreibeZip(ziel, eintraege, &ergebnis.fehler);
    return ergebnis;
}

// --- Import -----------------------------------------------------------------

int ImportBereich::vergeben() const
{
    return static_cast<int>(std::count_if(dateien.begin(), dateien.end(), [](const ImportDatei& d) {
        return d.zustand == Zustand::NameVergeben;
    }));
}

Inhalt oeffne(const QString& datei, const QString& benutzerOrdner)
{
    Inhalt inhalt;
    const Archiv archiv = liesArchiv(datei);
    if (!archiv.ok)
    {
        inhalt.fehler = archiv.fehler;
        return inhalt;
    }
    inhalt.fassung = archiv.manifest.value(QStringLiteral("version")).toInt(0);
    inhalt.appVersion = archiv.manifest.value(QStringLiteral("appVersion")).toString();
    inhalt.exportiert =
        QDateTime::fromString(archiv.manifest.value(QStringLiteral("exported")).toString(), Qt::ISODate);

    const QDir wurzel(benutzerOrdner);
    std::map<QString, std::vector<ImportDatei>> jeBereich;
    for (const ZipEintrag& eintrag : archiv.eintraege)
    {
        if (eintrag.name == kManifest) continue;
        if (eintrag.name == kEinstellungen)
        {
            inhalt.hatEinstellungen = true;
            inhalt.einstellungen = QJsonDocument::fromJson(eintrag.daten).object();
            continue;
        }
        const QString rel = eintrag.name.startsWith(kDaten) ? eintrag.name.mid(kDaten.size()) : QString();
        const Bereich* bereich = pfadInOrdnung(rel) ? bereichVon(rel) : nullptr;
        if (bereich == nullptr)
        {
            ++inhalt.ausgelassen;
            continue;
        }
        ImportDatei neu{rel, eintrag.daten.size(), eintrag.geaendert, Zustand::Neu, 0, {}};
        const QFileInfo eigene(wurzel.filePath(rel));
        if (eigene.isFile())
        {
            bool ok = false;
            const QByteArray vorhanden = lies(eigene.absoluteFilePath(), &ok);
            if (ok && vorhanden == eintrag.daten)
                neu.zustand = Zustand::Gleich;
            else
            {
                neu.zustand = Zustand::NameVergeben;
                neu.eigeneGroesse = eigene.size();
                neu.eigeneGeaendert = eigene.lastModified();
            }
        }
        jeBereich[bereich->id].push_back(std::move(neu));
    }
    for (const Bereich& bereich : bereiche())
    {
        const auto treffer = jeBereich.find(bereich.id);
        if (treffer == jeBereich.end()) continue;
        std::sort(treffer->second.begin(), treffer->second.end(), [](const ImportDatei& a, const ImportDatei& b) {
            return QString::compare(a.rel, b.rel, Qt::CaseInsensitive) < 0;
        });
        inhalt.bereiche.push_back({bereich.id, std::move(treffer->second)});
    }
    inhalt.ok = true;
    return inhalt;
}

QString nameMitZahl(const QString& rel, const std::function<bool(const QString&)>& istVergeben)
{
    const int schnitt = rel.lastIndexOf(QLatin1Char('/')) + 1;
    const QString ordner = rel.left(schnitt);
    const QString datei = rel.mid(schnitt);
    // die Endung ist, was nach dem letzten Punkt steht — `Neon.v2.json` -> `Neon.v2 (2).json`
    const int punkt = datei.lastIndexOf(QLatin1Char('.'));
    const QString stamm = punkt > 0 ? datei.left(punkt) : datei;
    const QString endung = punkt > 0 ? datei.mid(punkt) : QString();
    // `<stamm>_<zahl>` (S125, P6 a): ein eigener Name traegt nur a-z, Ziffern und `_`
    // und hoechstens 64 Zeichen (`istEigenerName`) — so wie `nimmZuEigenenInstrumenten`
    for (int zahl = 2;; ++zahl)
    {
        const QString versuch = ordner + QStringLiteral("%1_%2%3").arg(stamm.left(60)).arg(zahl).arg(endung);
        if (!istVergeben(versuch)) return versuch;
    }
}

ImportErgebnis importiere(const QString& datei, const QString& benutzerOrdner, const ImportWahl& wahl)
{
    ImportErgebnis ergebnis;
    if (benutzerOrdner.isEmpty())
    {
        ergebnis.fehler = QStringLiteral("There is no user data folder.");
        return ergebnis;
    }
    const Archiv archiv = liesArchiv(datei);
    if (!archiv.ok)
    {
        ergebnis.fehler = archiv.fehler;
        return ergebnis;
    }
    const QDir wurzel(benutzerOrdner);
    QSet<QString> geschrieben;  // was dieser Import schon angelegt hat (klein geschrieben)

    const auto schreibe = [&](const QString& rel, const ZipEintrag& eintrag) {
        const QString pfad = wurzel.filePath(rel);
        if (!QDir().mkpath(QFileInfo(pfad).absolutePath())) return false;
        QSaveFile ziel(pfad);
        if (!ziel.open(QIODevice::WriteOnly) || ziel.write(eintrag.daten) != eintrag.daten.size() || !ziel.commit())
            return false;
        // das Datum der Datei mitnehmen: ein spaeterer Import vergleicht damit
        if (eintrag.geaendert.isValid())
        {
            QFile gesetzt(pfad);
            if (gesetzt.open(QIODevice::ReadWrite))
                gesetzt.setFileTime(eintrag.geaendert, QFileDevice::FileModificationTime);
        }
        geschrieben.insert(rel.toLower());
        return true;
    };
    const auto scheitere = [&](const QString& rel) {
        ergebnis.fehler = QStringLiteral("Could not write '%1'.").arg(QDir::toNativeSeparators(rel));
        return ergebnis;
    };

    for (const ZipEintrag& eintrag : archiv.eintraege)
    {
        if (!eintrag.name.startsWith(kDaten)) continue;
        const QString rel = eintrag.name.mid(kDaten.size());
        const Bereich* bereich = pfadInOrdnung(rel) ? bereichVon(rel) : nullptr;
        if (bereich == nullptr || !wahl.bereiche.contains(bereich->id)) continue;

        const QFileInfo eigene(wurzel.filePath(rel));
        if (!eigene.isFile())
        {
            if (!schreibe(rel, eintrag)) return scheitere(rel);
            ++ergebnis.eingelesen;
            continue;
        }
        bool ok = false;
        const QByteArray vorhanden = lies(eigene.absoluteFilePath(), &ok);
        if (ok && vorhanden == eintrag.daten)
        {
            ++ergebnis.gleich;
            continue;
        }

        Wahl was = Wahl::Meine;
        switch (wahl.regel)
        {
            case Regel::MeineBehalten: was = Wahl::Meine; break;
            case Regel::Ersetzen: was = Wahl::Importierte; break;
            case Regel::BeideBehalten: was = Wahl::Beide; break;
            case Regel::JeDatei:
            {
                const auto treffer = wahl.jeDatei.find(rel);
                was = treffer == wahl.jeDatei.end() ? Wahl::Meine : treffer->second;
                break;
            }
        }
        if (was == Wahl::Meine)
        {
            ++ergebnis.uebersprungen;
        }
        else if (was == Wahl::Importierte)
        {
            if (!schreibe(rel, eintrag)) return scheitere(rel);
            ++ergebnis.ersetzt;
        }
        else
        {
            const QString neu = nameMitZahl(rel, [&](const QString& versuch) {
                return geschrieben.contains(versuch.toLower()) || QFileInfo::exists(wurzel.filePath(versuch));
            });
            if (!schreibe(neu, eintrag)) return scheitere(neu);
            ++ergebnis.umbenannt;
        }
    }
    ergebnis.ok = true;
    return ergebnis;
}

}  // namespace lumi::core::datensicherung
