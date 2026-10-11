/**
 * @file Loop.hpp
 * @brief Loop: das Datenmodell eines Loops und die Datei `.lvloop`
 *        (docs/audio/Loop_Konzept.md §4)
 *
 * Ein Loop ist ein **Arrangement in Takten** (Entscheide Patrik S108, L3 neu
 * und L8 a): Kanaele, in den Kanaelen Clips. Ein Clip spielt ein **Muster**
 * (die „Basis") und wiederholt es; mehrere Clips koennen dasselbe Muster
 * spielen — wer das Muster aendert, aendert sie alle.
 *
 * Ein Muster traegt alles, was sein Editor einstellt: bei einem Drumkit die
 * Spuren (das Kit) und je Spur die Schritte. Eine Spur verweist auf einen
 * Klang: einen Namen aus dem mitgelieferten Kit (`kit:<Name>`) oder eine
 * Sample-Datei — eingebettet wird nichts. Bei einem Instrument (Stufe 9,
 * Entscheid I1 c) sind es das Instrument und die Noten.
 *
 * Tempo und Raster stehen im Loop und gelten fuer alles darin (L2 b). Die
 * Datei `.lvloop` ist das ganze Arrangement. Kein Qt-Widget, keine
 * Audio-Engine: reines Modell.
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
/// Ein Muster ist mindestens so lang — die Laenge eines neuen Clips
/// (Vorgabe Patrik S108).
inline constexpr int kTakteMin = 4;
inline constexpr int kTakteMax = 64;   ///< hoechstens so viele Takte je Muster

/// Der Vorsatz eines Klangs aus dem mitgelieferten Kit: `kit:kick`.
[[nodiscard]] bool istKitKlang(const QString& klang);

/// Eine Spur eines Drumkits: ein Klang mit Lautstaerke und Panorama.
struct Spur
{
    QString name;
    QString klang;       ///< `kit:<Name>` oder der Pfad einer Sample-Datei (absolut); leer = kein Klang
    double laut = 1.0;   ///< 0…1
    double pan = 0.0;    ///< -1 (links) … 1 (rechts)
    bool stumm = false;

    [[nodiscard]] bool operator==(const Spur&) const = default;
};

/// Die Art eines Musters — sie bestimmt seinen Editor: ein Drumkit (Spuren und
/// Schritte) oder ein Instrument (Noten mit Tonhoehe und Laenge, Stufe 9).
enum class MusterArt : int
{
    Drumkit = 0,
    Instrument = 1,
};

/// Die Tonhoehen einer Note, als MIDI-Nummer: C1 bis C7, sechs Oktaven
/// (60 = C4, 69 = A4 mit 440 Hz).
inline constexpr int kHoeheMin = 24;
inline constexpr int kHoeheMax = 96;

/// Eine Note eines Instrument-Musters: ab einem Schritt, so viele Schritte
/// lang, in einer Tonhoehe.
struct Note
{
    int schritt = 0;    ///< 0 = der erste Schritt des Musters
    int laenge = 1;     ///< in Schritten, mindestens 1
    int hoehe = 60;     ///< MIDI-Nummer, `kHoeheMin` … `kHoeheMax`
    double laut = 1.0;  ///< 0…1 (groesser als 0 — eine Note ohne Lautstaerke gibt es nicht)

    [[nodiscard]] bool operator==(const Note&) const = default;
};

/// Nennt ein Muster sein Instrument beim Pfad einer eigenen Datei (`.lvinst`)
/// — statt beim Namen eines mitgelieferten?
[[nodiscard]] bool istInstrumentDatei(const QString& nennung);
/// Das Instrument eines neuen Musters und eines Musters ohne Angabe: `piano`.
[[nodiscard]] QString instrumentVorgabe();

/// Der Vorsatz eines gerechneten Tons unter den Klaengen: `inst:piano:60:500`
/// — Instrument (Name oder Pfad), Tonhoehe, Dauer in Millisekunden.
[[nodiscard]] bool istTon(const QString& klang);
/// Der Name des Klangs zu einem Ton eines Instruments.
[[nodiscard]] QString tonKlang(const QString& instrument, int hoehe, int dauerMs);
/// Den Namen wieder zerlegen. false = kein Ton.
bool zerlegeTon(const QString& klang, QString& instrument, int& hoehe, int& dauerMs);

/// Ein Muster, die „Basis" eines oder mehrerer Clips. Ein **Drumkit** traegt
/// das Kit und je Spur eine Zeile, je Schritt eine Lautstaerke (0 = aus). Ein
/// **Instrument** traegt den Namen seines Instruments, Lautstaerke und
/// Panorama und seine Noten; es spielt mehrere Toene zugleich.
struct Muster
{
    QString name;
    MusterArt art = MusterArt::Drumkit;
    int takte = kTakteMin;

    // --- Drumkit ---
    std::vector<Spur> spuren;
    /// `zeilen[spur][schritt]`, 0…1; nach `Loop::richte` je Spur eine Zeile
    /// mit `takte * schritteJeTakt()` Schritten.
    std::vector<std::vector<double>> zeilen;

    // --- Instrument ---
    /// Das Instrument (`loop/Instrument.hpp`): der Name eines mitgelieferten,
    /// etwa `piano`, oder der Pfad einer eigenen Datei `.lvinst` (absolut).
    QString instrument;
    double laut = 1.0;   ///< 0…1
    double pan = 0.0;    ///< -1 (links) … 1 (rechts)
    bool stumm = false;
    /// Nach `Loop::richte` nach Schritt, dann Tonhoehe sortiert; je Schritt
    /// und Tonhoehe hoechstens eine, keine reicht ueber das Ende des Musters.
    std::vector<Note> noten;

    [[nodiscard]] bool operator==(const Muster&) const = default;
};

/// Ein Clip im Kanal: ein Muster ab einem Takt, einmal oder mehrfach
/// hintereinander. Seine Laenge ist immer ein Vielfaches der Musterlaenge.
struct LoopClip
{
    QString muster;          ///< Name des Musters
    int startTakt = 0;       ///< 0 = der erste Takt
    int wiederholungen = 1;  ///< mindestens 1

    [[nodiscard]] bool operator==(const LoopClip&) const = default;
};

struct Kanal
{
    QString name;
    bool an = true;
    /// Nach `Loop::richte` nach Start sortiert und ueberlappungsfrei.
    std::vector<LoopClip> clips;

    [[nodiscard]] bool operator==(const Kanal&) const = default;
};

/// Die Tags eines Loops — dasselbe Schema wie bei einem Song (Vorgabe Patrik
/// S109): liegt der Loop als Clip im Composer, zaehlt er als laufender Song,
/// und Player-Panel, Text-Vorlagen und Karaoke lesen hier, was sie bei einer
/// MP3 aus der Datei lesen. Alle Felder sind freiwillig; ohne Titel gilt der
/// Dateiname.
struct LoopTags
{
    QString titel;
    QString kuenstler;
    QString album;
    QString jahr;
    QString genre;
    QString nummer;  ///< Titelnummer
    /// Songtext wie in den Tags einer Datei abgelegt — er darf selbst
    /// LRC-Zeitmarken tragen.
    QString text;

    [[nodiscard]] bool operator==(const LoopTags&) const = default;
};

struct Loop
{
    LoopTags tags;
    double bpm = 120.0;
    int schlaegeJeTakt = 4;
    int schritteJeSchlag = 4;
    /// 0…1: verzoegert jeden zweiten Schritt eines Schlags, bei 1 um einen halben Schritt.
    double swing = 0.0;
    /// 0…1, die Lautstaerke des ganzen Loops. Die Vorgabe laesst Luft nach
    /// oben: die Spuren addieren sich, begrenzt wird nichts (Entscheid Patrik
    /// S108, L6 a).
    double laut = 0.5;

    std::vector<Muster> muster;
    std::vector<Kanal> kanaele;

    [[nodiscard]] bool operator==(const Loop&) const = default;

    [[nodiscard]] int schritteJeTakt() const { return schlaegeJeTakt * schritteJeSchlag; }
    /// Das erste Muster dieses Namens; nullptr = keins.
    [[nodiscard]] const Muster* findeMuster(const QString& name) const;
    /// Laenge eines Clips in Takten: Wiederholungen mal Musterlaenge; 0, wenn
    /// es sein Muster nicht gibt.
    [[nodiscard]] int clipTakte(const LoopClip& clip) const;
    /// Takte eines Durchlaufs: das Ende des letzten Clips, gleich ob sein
    /// Kanal an ist. Ohne Clips ein Takt (Stille).
    [[nodiscard]] int durchlaufTakte() const;
    [[nodiscard]] int durchlaufSchritte() const { return durchlaufTakte() * schritteJeTakt(); }
    /// Dauer eines Durchlaufs in Millisekunden (ohne Swing gerechnet — der
    /// letzte Schritt endet davon unberuehrt am Durchlauf).
    [[nodiscard]] double durchlaufMs() const;
    /// Die Klaenge aller Spuren aller Muster, jeder einmal, in der
    /// Reihenfolge ihres ersten Auftretens; Spuren ohne Klang zaehlen nicht.
    /// Dazu je Instrument-Muster die Toene seiner Noten (`tonKlang`): je
    /// Tonhoehe und Dauer einer — die Dauer haengt am Tempo.
    [[nodiscard]] QStringList klangNamen() const;
    /// Die Dauer einer Note in Millisekunden, gerundet — wie sie im Namen
    /// ihres Tons steht.
    [[nodiscard]] int notenMs(const Note& note) const;

    /// Unsinn richten: Werte in ihre Grenzen, je Muster eine Zeile je Spur in
    /// voller Laenge, Muster ohne Namen benennen; Clips nach Start sortieren,
    /// Ueberlappung im selben Kanal aufloesen (der spaetere Clip beginnt, wo
    /// der fruehere endet), Clips ohne Muster entfernen. Was auffaellt, steht
    /// danach im `bericht`.
    void richte(QStringList* bericht = nullptr);
};

/// Der Loop als JSON. Pfade von Sample-Dateien werden relativ zu
/// `basisOrdner` abgelegt, wenn sie darunter liegen — sonst absolut.
[[nodiscard]] QJsonObject alsJson(const Loop& loop, const QString& basisOrdner = {});
/// Liest einen Loop und richtet ihn. false = kein Loop (falsches Format oder
/// eine Fassung vor 2); `loop` bleibt dann, wie er war. Fehlt eine
/// Sample-Datei, steht das im `bericht`.
bool ausJson(const QJsonObject& wurzel, Loop& loop, const QString& basisOrdner = {},
             QStringList* bericht = nullptr);

bool speichereLoop(const Loop& loop, const QString& pfad);
bool ladeLoop(const QString& pfad, Loop& loop, QStringList* bericht = nullptr);

}  // namespace lumi::loop
