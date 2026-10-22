/**
 * @file Aufnahme.hpp
 * @brief Loop: eine Aufnahme zuschneiden und als eigenes Sample speichern —
 *        der Kern hinter dem Fenster „Record sample"
 *        (docs/audio/Loop_Konzept.md §4h, Stufe 11b; Skizze gebilligt,
 *        Patrik S115)
 *
 * Aufgenommen wird ueber die Audio-Engine (`IAudioEngine::recordStart`). Hier
 * steht, was danach mit den Abtastwerten geschieht: die Stille an den Raendern
 * finden, einen Ausschnitt herausnehmen (mit kurzer Blende an beiden Enden,
 * auf Wunsch auf einen festen Pegel gehoben), als WAV schreiben und zu den
 * eigenen Samples legen (Sample.hpp, Entscheid S1 a).
 *
 * Alle Aufnahmen sind Stereo, links und rechts im Wechsel. Derselbe Aufruf
 * ergibt dieselben Werte. Kein Qt-Widget, keine Audio-Engine.
 */
#pragma once

#include <QString>

#include <cstdint>
#include <vector>

namespace lumi::loop
{

/// Ein Ausschnitt einer Aufnahme, in Abtastwerten je Seite: `von` gehoert
/// dazu, `bis` nicht mehr.
struct Ausschnitt
{
    std::int64_t von = 0;
    std::int64_t bis = 0;

    [[nodiscard]] std::int64_t frames() const { return bis > von ? bis - von : 0; }
    [[nodiscard]] bool operator==(const Ausschnitt&) const = default;
};

/// Was leiser ist als dieser Teil der Spitze, gilt als Stille (rund -34 dB).
inline constexpr double kStilleAnteil = 0.02;
/// So viel bleibt vor dem ersten und nach dem letzten Ton stehen, in Millisekunden.
inline constexpr double kStilleVorMs = 5.0;
inline constexpr double kStilleNachMs = 30.0;
/// So lang blendet ein Ausschnitt an beiden Enden, damit er nicht knackt.
inline constexpr double kSchnittBlendeMs = 2.0;
/// Auf diesen groessten Betrag hebt Normalize (wie die Klaenge des Kits).
inline constexpr double kNormalSpitze = 0.9;

/// Der Ausschnitt ohne die Stille an beiden Enden („Trim silence at both
/// ends"). Ist alles still (oder die Aufnahme leer), bleibt sie ganz.
[[nodiscard]] Ausschnitt ohneStille(const std::vector<float>& stereo, int rate);

/// Der groesste Betrag im Ausschnitt.
[[nodiscard]] float spitzeIn(const std::vector<float>& stereo, Ausschnitt ausschnitt);

/// An welchen Enden ein Ausschnitt kurz auf null geblendet wird. Ein Ende, an
/// dem nicht geschnitten wurde, bleibt ungeblendet (Stufe 17: ein geoeffnetes
/// Sample behaelt seinen Anschlag).
struct Blenden
{
    bool vorn = true;
    bool hinten = true;
};

/// Den Ausschnitt herausnehmen: an beiden Enden kurz geblendet (`blenden`),
/// mit `normalisieren` so laut, dass der groesste Betrag `kNormalSpitze` ist
/// (Stille bleibt Stille). Der Ausschnitt wird auf die Aufnahme begrenzt;
/// leer = nichts darin.
[[nodiscard]] std::vector<float> schneideAus(const std::vector<float>& stereo, int rate, Ausschnitt ausschnitt,
                                             bool normalisieren, Blenden blenden = {});

/// Als WAV schreiben: Stereo, 16 Bit — das liest jedes Programm (ueber `audio/WavDatei`, ohne
/// Dither, seit Stufe 30b). Werte ueber 1
/// werden begrenzt. false = leer, `rate` nicht groesser als 0, oder nicht zu schreiben.
bool schreibeWav(const QString& pfad, const std::vector<float>& stereo, int rate);

/// Der Titel, den „Record sample" vorschlaegt: „Recording", sonst „Recording
/// 2", … — der erste, den es unter den eigenen Samples noch nicht gibt.
[[nodiscard]] QString vorschlagAufnahmeTitel();
/// Der Titel, den „Save as..." fuer die Kopie eines Samples vorschlaegt
/// (Stufe 17): „kick 2", sonst „kick 3", … — der erste, den es unter den
/// eigenen Samples noch nicht gibt. Ohne Titel wie `vorschlagAufnahmeTitel`.
[[nodiscard]] QString vorschlagKopieTitel(const QString& titel);

/// Eine Aufnahme unter diesem Titel zu den eigenen Samples legen: als
/// `<name>.wav` mit `name = eigenerName(titel)`; ein vorhandenes Sample mit
/// dem Namen wird ersetzt (auch eines mit anderer Endung). Was dazu gemerkt
/// war, ist vergessen; Toene, die es spielen, werden neu gerechnet.
/// Leer = aus dem Titel wird kein Name, die Aufnahme ist leer, oder das
/// Schreiben gelang nicht; sonst die Nennung `user:<name>`.
[[nodiscard]] QString speichereAufnahme(const std::vector<float>& stereo, int rate, const QString& titel);

}  // namespace lumi::loop
