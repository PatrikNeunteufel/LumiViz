/**
 * @file Loop.hpp
 * @brief Loop: das Datenmodell eines Loops und die Datei `.lvloop`
 *        (docs/audio/Loop_Konzept.md §4, Stufe 3)
 *
 * Ein Loop ist ein Kit aus Spuren, eine Liste benannter Muster und eine
 * Abfolge von Mustern (Entscheid Patrik S108, L3 c). Das Tempo steht im Loop
 * (L2 b). Eine Spur verweist auf einen Klang: einen Namen aus dem
 * mitgelieferten Kit (`kit:<Name>`) oder eine Sample-Datei — eingebettet wird
 * nichts. Kein Qt-Widget, keine Audio-Engine: reines Modell.
 */
#pragma once

#include <QJsonObject>
#include <QString>
#include <QStringList>

#include <vector>

namespace lumi::loop
{

inline constexpr double kBpmMin = 20.0;
inline constexpr double kBpmMax = 400.0;
inline constexpr int kRasterMax = 16;  ///< hoechstens so viele Schlaege je Takt und Schritte je Schlag
inline constexpr int kTakteMax = 16;   ///< hoechstens so viele Takte je Muster

/// Der Vorsatz eines Klangs aus dem mitgelieferten Kit: `kit:kick`.
[[nodiscard]] bool istKitKlang(const QString& klang);

/// Eine Spur des Kits: ein Klang mit Lautstaerke und Panorama.
struct Spur
{
    QString name;
    QString klang;       ///< `kit:<Name>` oder der Pfad einer Sample-Datei (absolut)
    double laut = 1.0;   ///< 0…1
    double pan = 0.0;    ///< -1 (links) … 1 (rechts)
    bool stumm = false;

    [[nodiscard]] bool operator==(const Spur&) const = default;
};

/// Ein Muster: je Spur eine Zeile, je Schritt eine Lautstaerke (0 = aus).
struct Muster
{
    QString name;
    int takte = 1;
    /// `zeilen[spur][schritt]`, 0…1 — in der Reihenfolge der Spuren des Loops;
    /// nach `Loop::richte` hat jede Zeile `takte * schritteJeTakt()` Schritte.
    std::vector<std::vector<double>> zeilen;

    [[nodiscard]] bool operator==(const Muster&) const = default;
};

struct Loop
{
    double bpm = 120.0;
    int schlaegeJeTakt = 4;
    int schritteJeSchlag = 4;
    /// 0…1: verzoegert jeden zweiten Schritt eines Schlags, bei 1 um einen halben Schritt.
    double swing = 0.0;
    double laut = 1.0;  ///< 0…1, die Lautstaerke des ganzen Loops

    std::vector<Spur> spuren;
    std::vector<Muster> muster;
    /// Namen von Mustern, der Reihe nach — ein Durchlauf. Leer = das erste Muster.
    QStringList abfolge;

    [[nodiscard]] bool operator==(const Loop&) const = default;

    [[nodiscard]] int schritteJeTakt() const { return schlaegeJeTakt * schritteJeSchlag; }
    /// Das erste Muster dieses Namens; nullptr = keins.
    [[nodiscard]] const Muster* findeMuster(const QString& name) const;
    /// Die Abfolge als Muster. nullptr steht fuer einen Namen, den es nicht
    /// gibt: ein Takt Stille. Ohne Abfolge das erste Muster; ohne Muster ein
    /// Takt Stille. Nie leer.
    [[nodiscard]] std::vector<const Muster*> ablaufMuster() const;
    /// Schritte eines Durchlaufs der Abfolge.
    [[nodiscard]] int durchlaufSchritte() const;
    /// Dauer eines Durchlaufs in Millisekunden (ohne Swing gerechnet — der
    /// letzte Schritt endet davon unberuehrt am Durchlauf).
    [[nodiscard]] double durchlaufMs() const;

    /// Unsinn richten: Werte in ihre Grenzen, je Muster eine Zeile je Spur in
    /// voller Laenge, Muster ohne Namen benennen. Was auffaellt (ein Name
    /// doppelt, ein Muster der Abfolge fehlt), steht danach im `bericht`.
    void richte(QStringList* bericht = nullptr);
};

/// Der Loop als JSON. Pfade von Sample-Dateien werden relativ zu
/// `basisOrdner` abgelegt, wenn sie darunter liegen — sonst absolut.
[[nodiscard]] QJsonObject alsJson(const Loop& loop, const QString& basisOrdner = {});
/// Liest einen Loop und richtet ihn. false = kein Loop (falsches Format);
/// `loop` bleibt dann, wie er war. Fehlt eine Sample-Datei, steht das im `bericht`.
bool ausJson(const QJsonObject& wurzel, Loop& loop, const QString& basisOrdner = {},
             QStringList* bericht = nullptr);

bool speichereLoop(const Loop& loop, const QString& pfad);
bool ladeLoop(const QString& pfad, Loop& loop, QStringList* bericht = nullptr);

}  // namespace lumi::loop
