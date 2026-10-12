/**
 * @file ZipArchiv.hpp
 * @brief Ein Zip schreiben und lesen — die einzige Stelle, die miniz kennt
 *        (docs/core-services/Datenablage_Konzept.md, Stufe A4; Entscheide
 *        Patrik S114: D5 a, E2 b)
 *
 * Fuer den Export und Import der eigenen Daten (`.lvdata`). Bewusst klein:
 * eine Liste von Eintraegen hinein, eine Liste heraus, alles im Speicher —
 * die Datei selbst liest und schreibt Qt. So spielen Umlaute im Pfad keine
 * Rolle (miniz oeffnete mit `fopen`), und eine halb geschriebene Datei gibt
 * es nicht (`QSaveFile`).
 *
 * Grenzen, mit Absicht: das ganze Archiv liegt im Speicher (`kZipMaxBytes`);
 * keine Ordner-Eintraege, kein Passwort.
 */
#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QString>

#include <vector>

namespace lumi::core
{

/// Groesser wird ein Archiv nicht geschrieben und nicht gelesen.
inline constexpr qint64 kZipMaxBytes = 512LL * 1024 * 1024;

struct ZipEintrag
{
    /// Pfad im Archiv, mit `/` getrennt (`data/presets/visuals/eq/Neon.json`).
    QString name;
    QByteArray daten;
    /// Wann die Datei zuletzt geaendert wurde; ungueltig = jetzt. Ein Zip
    /// haelt die Zeit auf zwei Sekunden genau.
    QDateTime geaendert;

    [[nodiscard]] bool operator==(const ZipEintrag&) const = default;
};

/// Die Eintraege als Zip, komprimiert. Leer = Fehler, `fehler` sagt welcher.
[[nodiscard]] QByteArray alsZip(const std::vector<ZipEintrag>& eintraege, QString* fehler = nullptr);

/// Ein Zip lesen (auch eines, das ein anderes Programm geschrieben hat).
/// Ordner-Eintraege fallen weg. false = kein lesbares Zip; `eintraege`
/// bleibt dann, wie es war.
bool ausZip(const QByteArray& zip, std::vector<ZipEintrag>& eintraege, QString* fehler = nullptr);

/// `alsZip` in eine Datei — ganz oder gar nicht.
bool schreibeZip(const QString& pfad, const std::vector<ZipEintrag>& eintraege, QString* fehler = nullptr);
/// Eine Datei mit `ausZip` lesen.
bool leseZip(const QString& pfad, std::vector<ZipEintrag>& eintraege, QString* fehler = nullptr);

}  // namespace lumi::core
