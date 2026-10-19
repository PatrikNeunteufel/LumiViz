/**
 * @file Spielweise.hpp
 * @brief Loop: die Spielweisen eines Saiteninstruments — Strum und die
 *        Artikulationen einer Note (docs/audio/Saiten_Konzept.md, Stufe 24a;
 *        Entscheide Patrik S134: S-W2 a, S-W3 a, S-W4 a, S-W6 a)
 *
 * **Strum** (S-W2 a) steht als Vorgabe am Muster: der Versatz der Saiten
 * (*Spread*) und das Muster der Hand (*Hand*, S-W3 a) — je Stelle eines Takts
 * ein Anschlag nach unten oder oben. Ein neues Muster bekommt die Vorgabe des
 * Instruments (`Saitensatz::strum`). Ein Akkord — die Noten, die am selben
 * Schritt anschlagen — kann abweichen: ein `Anschlag` in der Liste des Musters
 * (S-W4 a).
 *
 * Die **Artikulationen** gehoeren der Note (S-W4 a: was eine Saite betrifft,
 * steht an der Note; was die Hand betrifft, am Zeitpunkt).
 *
 * Hier steht nur das Modell — wie es klingt und wann es anschlaegt, rechnen
 * erst die Stufen 24b und 24c. Kein Qt-Widget, keine Audio-Engine.
 */
#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QLatin1String>
#include <QString>

#include <vector>

namespace lumi::loop
{

/// Die Richtung eines Anschlags der Hand.
enum class Richtung : int
{
    Ab = 0,   ///< Down: die tiefe Saite zuerst
    Auf = 1,  ///< Up: die hohe Saite zuerst
};

/// Das Raster der Hand (S-W3 a): so dicht liegen die Stellen eines Takts, an
/// denen sie ab und auf geht.
enum class HandRaster : int
{
    Schlag = 0,        ///< *Beat*
    HalberSchlag = 1,  ///< *1/2 beat* — die Vorgabe (von Claude gesetzt: Achtel bei 4/4)
    Schritt = 2,       ///< *Step*
};

/// Spread: der Abstand zweier benachbarter Saiten beim Anschlag, in ms
/// (S-W12 a) — die Vorgabe nach Freire 2018 (SMC, Table 6, S. 547:
/// sechssaitig gestrummt 8–11 ms je Saite, „adagio"), die Grenze von Claude
/// gesetzt (ein langsam gebrochener Akkord).
inline constexpr double kSpreadVorgabeMs = 8.0;
inline constexpr double kSpreadMaxMs = 100.0;
/// So viele Stellen traegt das Muster der Hand hoechstens: ein Takt im
/// feinsten Raster (`kRasterMax` Schlaege mit je `kRasterMax` Schritten).
inline constexpr int kHandStellenMax = 256;

/// Strum: die Vorgabe eines Musters — und eines Instruments fuer neue Muster.
struct Strum
{
    double spreadMs = kSpreadVorgabeMs;  ///< Spread: von einer Saite zur naechsten, 0 … `kSpreadMaxMs`
    HandRaster raster = HandRaster::HalberSchlag;
    /// Je Stelle eines Takts die Richtung; eine Stelle dahinter wechselt ab
    /// (gerade Stellen Down, ungerade Up). Leer = *Alternate*. Gerichtet steht
    /// hier nichts, was ohnehin abwechselt (`richteStrum`).
    std::vector<Richtung> hand;

    [[nodiscard]] bool operator==(const Strum&) const = default;
};

/// Die Richtung an einer Stelle der Hand: die gesetzte oder die abwechselnde.
[[nodiscard]] inline Richtung richtungAnStelle(const Strum& strum, int stelle)
{
    if (stelle >= 0 && stelle < static_cast<int>(strum.hand.size())) return strum.hand[static_cast<std::size_t>(stelle)];
    return stelle % 2 == 0 ? Richtung::Ab : Richtung::Auf;
}
/// So viele Stellen hat ein Takt im Raster der Hand.
[[nodiscard]] int handStellen(HandRaster raster, int schlaegeJeTakt, int schritteJeSchlag);
/// Die Stelle der Hand, auf der ein Schritt des Musters anschlaegt — liegt er
/// zwischen zwei Stellen, die vorige (von Claude gesetzt, S-W3 a).
[[nodiscard]] int handStelleVon(HandRaster raster, int schritt, int schlaegeJeTakt, int schritteJeSchlag);
/// Werte in ihre Grenzen; das Muster der Hand hoechstens `stellen` lang, und
/// hinten weg, was ohnehin abwechselt.
void richteStrum(Strum& strum, int stellen = kHandStellenMax);

/// Wie das Raster in einer Datei steht: `beat`, `halfBeat`, `step`.
[[nodiscard]] QLatin1String handRasterName(HandRaster raster);
/// Fehlt es oder ist es unbekannt: `halfBeat`.
[[nodiscard]] HandRaster handRasterAusName(const QString& name);
/// Das Muster der Hand als Text, je Stelle `D` oder `U`; und zurueck — ein
/// anderes Zeichen gilt als die abwechselnde Richtung an dieser Stelle.
[[nodiscard]] QString handAlsText(const std::vector<Richtung>& hand);
[[nodiscard]] std::vector<Richtung> handAusText(const QString& text);
/// Strum in einer Datei (`.lvloop` am Muster, `.lvinst` unter `strings`):
/// `spread` in ms, `hand` das Raster, `pattern` das Muster der Hand (nur, wenn
/// es eines gibt). Gelesen wird ungerichtet — wer liest, richtet.
[[nodiscard]] QJsonObject strumAlsJson(const Strum& strum);
[[nodiscard]] Strum strumAusJson(const QJsonObject& o);

/// Wie ein Akkord angeschlagen wird, der vom Muster abweicht (S-W4 a).
enum class Anschlagart : int
{
    Hand = 0,      ///< *Follow hand*: die Richtung der Stelle — dann weicht nur der Versatz ab
    Ab = 1,        ///< *Down*
    Auf = 2,       ///< *Up*
    Zugleich = 3,  ///< *Together*: alle Saiten zugleich
};

/// Die Abweichung eines Akkords: am Schritt, an dem seine Noten anschlagen.
struct Anschlag
{
    int schritt = 0;
    Anschlagart art = Anschlagart::Hand;
    double spreadMs = -1.0;  ///< −1 = wie das Muster, sonst 0 … `kSpreadMaxMs`

    /// Weicht er ueberhaupt ab? Ein Eintrag „wie das Muster" in beidem faellt weg.
    [[nodiscard]] bool weichtAb() const { return art != Anschlagart::Hand || spreadMs >= 0.0; }
    [[nodiscard]] bool operator==(const Anschlag&) const = default;
};

/// Wie die Art in einer Datei steht: `hand`, `down`, `up`, `together`.
[[nodiscard]] QLatin1String anschlagartName(Anschlagart art);
/// Unbekannt: `hand`.
[[nodiscard]] Anschlagart anschlagartAusName(const QString& name);

/// Die Artikulationen einer Note (S-W1 a, erste Stufe) als Bits von
/// `Note::artikulationen`; frei kombinierbar (von Claude gesetzt).
enum Artikulation : int
{
    kPalmMute = 1,  ///< *Palm mute*: der Handballen daempft am Steg
    kLetRing = 2,   ///< *Let ring*: klingt bis zur naechsten Note auf derselben Saite
    kTapping = 4,   ///< *Tapping*: am Bund angeschlagen
    kHarmonic = 8,  ///< *Harmonic*: ein Flageolett (S-W7 a)
    // --- zweite Stufe (Stufe 25a, S-L1 a): an der Zielnote, sie kommt von der vorigen
    // Note auf derselben Saite und wird nicht angeschlagen
    kLegato = 16,  ///< *Legato*: Hammer-on (hoeher) oder Pull-off (tiefer) — nach der Richtung benannt
    kSlide = 32,   ///< *Slide*: gleitet von der vorigen Note auf derselben Saite her
};
/// Alle Bits, die dieses Programm kennt — andere fallen beim Lesen weg.
inline constexpr int kArtikulationenAlle = kPalmMute | kLetRing | kTapping | kHarmonic | kLegato | kSlide;
/// Die Bits, die eine Note mit der vorigen auf ihrer Saite verbinden.
inline constexpr int kVerbunden = kLegato | kSlide;

/// Ein Punkt der Kurve eines Bends (Stufe 25a, S-L2 a).
struct BendPunkt
{
    double zeit = 0.0;       ///< Anteil der Notenlaenge, 0 … 1 — wird die Note laenger, dehnt sich der Bend
    double halbtoene = 0.0;  ///< die Biegung ueber dem gegriffenen Ton, 0 … `kBendHalbtoeneMax`, in Vierteln

    [[nodiscard]] bool operator==(const BendPunkt&) const = default;
};
/// So weit biegt ein Bend hoechstens (S-L2 a: drei Halbtoene) — und in welchem
/// Raster (Vierteltoene).
inline constexpr double kBendHalbtoeneMax = 3.0;
inline constexpr double kBendRaster = 0.25;
/// So viele Punkte traegt eine Kurve hoechstens (von Claude gesetzt: die Vorlagen
/// brauchen vier, eine gezeichnete Kurve selten mehr als acht).
inline constexpr int kBendPunkteMax = 16;

/// Werte in ihre Grenzen, die Biegung ins Raster, nach der Zeit sortiert;
/// zweimal dieselbe Zeit: der spaetere Punkt gilt (er ist der Sprung); hoechstens
/// `kBendPunkteMax`. Biegt die Kurve nirgends, ist sie leer — kein Bend.
void richteBend(std::vector<BendPunkt>& bend);
/// Die Biegung zur Zeit `zeit` (Anteil der Note), linear zwischen den Punkten;
/// vor dem ersten der erste, nach dem letzten der letzte; ohne Punkte 0.
[[nodiscard]] double bendBei(const std::vector<BendPunkt>& bend, double zeit);
/// Der Bend in der `.lvloop` (seit Fassung 10): je Punkt `[zeit, halbtoene]`.
/// Gelesen wird ungerichtet — wer liest, richtet.
[[nodiscard]] QJsonArray bendAlsJson(const std::vector<BendPunkt>& bend);
[[nodiscard]] std::vector<BendPunkt> bendAusJson(const QJsonArray& werte);

}  // namespace lumi::loop
