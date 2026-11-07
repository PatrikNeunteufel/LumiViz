/**
 * @file Tastatur.hpp
 * @brief Loop: der Block *Keyboard* eines Instruments — Umfang, Pedale auf ihren Plaetzen, Grenze von *Bass sustain*
 *        (docs/audio/Spielweisen_Konzept.md §4.1, HA2 a, HA4 a; Instrumente_IC_Konzept.md FU2 a)
 *
 * Bis S175 stand das mit der Hammer-Saite in `loop/HammerSaite.hpp`. Seit 34d (FU2 a) traegt der Block nur, was die
 * Planung und der Editor brauchen; was die Rechnung eines Kerns bestimmt (`undampedFrom`, Soft, Moderator), steht in
 * seiner Tafel (`InstrumentModels/hammer/HammerKern.hpp`). *Bass sustain* und seine Grenze kommen aus IC (Klasse 1 F-2 a).
 *
 * Kein Qt-Widget, keine Audio-Engine.
 */
#pragma once

#include <QJsonObject>
#include <QLatin1String>
#include <QString>

namespace lumi::loop
{

/// Der Umfang einer Tastatur: A0 bis C8 (wie `kHoeheMin`, `kHoeheMax` in Loop.hpp).
inline constexpr int kTastaturMin = 21;
inline constexpr int kTastaturMax = 108;

/// Die Art eines Pedals; je Platz sind nur bestimmte erlaubt (`pedalErlaubt`).
enum class PedalArt : int
{
    Keins = 0,
    UnaCorda = 1,     ///< links: die Mechanik verschiebt sich, der Hammer trifft weniger Saiten
    Leise = 2,        ///< links, *Soft*: die Haemmer ruecken naeher an die Saiten
    Sostenuto = 3,    ///< Mitte: haelt die Daempfer, die beim Treten oben sind
    Moderator = 4,    ///< Mitte, *Practice*: ein Filz zwischen Hammer und Saite
    Sustain = 5,      ///< rechts: hebt alle Daempfer
    BassSustain = 6,  ///< Mitte, *Bass sustain* (aus IC, Klasse 1 F-2 a): hebt die Daempfer unter `bassSustainBis`
};

/// Der Platz eines Pedals.
enum class PedalPlatz : int
{
    Links = 0,
    Mitte = 1,
    Rechts = 2,
};

/// Darf diese Art auf diesem Platz stehen (HA4 a)? Keins ueberall.
[[nodiscard]] bool pedalErlaubt(PedalPlatz platz, PedalArt art);
/// Wie die Art in einer Datei steht: `none`, `unaCorda`, `soft`, `sostenuto`, `practice`, `sustain`, `bassSustain`.
[[nodiscard]] QLatin1String pedalName(PedalArt art);
/// Fehlt die Art oder ist sie unbekannt: keins.
[[nodiscard]] PedalArt pedalAusName(const QString& name);

/// Die Ausstattung einer Tastatur (HA4 a). Die Vorgaben sind die des Fluegels.
struct Tastatur
{
    int tiefste = kTastaturMin;   ///< `range`: die tiefste Taste
    int hoechste = kTastaturMax;  ///< die hoechste
    PedalArt links = PedalArt::UnaCorda;
    PedalArt mitte = PedalArt::Sostenuto;
    PedalArt rechts = PedalArt::Sustain;
    /// *Bass sustain*: die erste Taste, deren Daempfer nicht gehoben wird (IC F-2 a; offen, Vorgabe C4).
    int bassSustainBis = 60;

    [[nodiscard]] bool operator==(const Tastatur&) const = default;
};

/// In die Grenzen: der Umfang innerhalb `kTastaturMin` … `kTastaturMax` (tiefste nicht ueber hoechste),
/// `bassSustainBis` im Umfang bis hoechste + 1, ein Pedal, das auf seinem Platz nicht erlaubt ist, wird keins.
void richteTastatur(Tastatur& tastatur);
/// Der Block `keyboard`: `{"range": [tiefste, hoechste], "pedals": {"left": …, "middle": …, "right": …},
/// "bassSustainBelow": …}` — und zurueck (gerichtet). Ein `undampedFrom` aus Fassung 20 wird ueberlesen (es steht seit
/// Fassung 21 in der Tafel des Kerns).
[[nodiscard]] QJsonObject tastaturAlsJson(const Tastatur& tastatur);
[[nodiscard]] Tastatur tastaturAusJson(const QJsonObject& o);

}  // namespace lumi::loop
