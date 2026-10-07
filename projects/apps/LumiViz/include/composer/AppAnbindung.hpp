/**
 * @file AppAnbindung.hpp
 * @brief Composer: eine Komposition in der App ein- und ausschalten
 *        (docs/ui/Composer_Konzept.md §12.2 Naht N8, Stufe C2a)
 *
 * Das Menue File schaltet den Composer: **Open composition…** laedt eine
 * `.lvcomp`, setzt die Bildquelle an die Stelle des Renderers der Kette (Naht
 * R3) und startet die Uhr; **Close composition** nimmt sie wieder weg — die
 * Kette laeuft weiter wie zuvor.
 *
 * Das Menue meldet nur die beiden Ereignisse; das Hauptfenster reicht sie an
 * die Funktionen hier weiter. Seit C2b fuehrt `composer/Sitzung` die geladene
 * Komposition, den Schalter Composer on und den Transport; die Funktionen hier
 * sind der Weg des Menues und des Starts dorthin.
 *
 * Rueckbau (Konzept §12): Teil von `include/composer` / `src/composer`.
 */
#pragma once

#include "audio/AudioEvents.hpp"
#include "audio/IAudioEngine.hpp"
#include "services/events/Event.hpp"

#include <QString>
#include <QStringList>

#include <string>

class QWidget;
class VisualizerWidget;

namespace lumi::composer
{

/// Menue File → Open composition… (`path` leer = Dateidialog).
struct OpenCompositionEvent : public Event
{
    EVENT_TYPE_NAME("OpenCompositionEvent")

    std::string path;

    OpenCompositionEvent() = default;
    explicit OpenCompositionEvent(std::string p) : path(std::move(p)) {}
};

/// Menue File → Close composition.
struct CloseCompositionEvent : public Event
{
    EVENT_TYPE_NAME("CloseCompositionEvent")
};

/// Liefert gerade eine Komposition das Bild dieses Fensters?
[[nodiscard]] bool kompositionLaeuft(const VisualizerWidget& widget);

/// Eine `.lvcomp` laden, den Composer einschalten und abspielen (ausser die
/// Einstellung „haelt am Anfang" ist gesetzt). Eine schon geladene Komposition
/// wird ersetzt. false = die Datei ist keine Komposition; es bleibt alles, wie
/// es war. `bericht` nennt, was gerichtet wurde und welche Dateien fehlen.
bool starteKomposition(VisualizerWidget& widget, const QString& pfad,
                       QStringList* bericht = nullptr);

/// Composer aus und die Komposition aus dem Panel — der Renderer der Kette
/// liefert wieder das Bild.
void schliesseKomposition(VisualizerWidget& widget);

/// Vor dem Ersetzen, Schliessen oder Beenden (Naht N10): hat die geladene
/// Komposition ungespeicherte Aenderungen, fragt ein Dialog Save / Discard /
/// Cancel (Regel S89). true = weiter. Ohne Aenderungen fragt nichts.
[[nodiscard]] bool darfKompositionErsetzen(VisualizerWidget& widget, QWidget* eltern,
                                           const QString& titel);

/// Speichern mit Meldung bei Fehlern (C3b): in die geladene Datei — oder, mit
/// `immerFragen` bzw. bei einer neuen Komposition ohne Datei, nach einem
/// Dateidialog. false = abgebrochen oder nicht geschrieben.
bool speichereMitDialog(VisualizerWidget& widget, QWidget* eltern, bool immerFragen);

/// Eine neue, leere Komposition ins Panel (C3b) — nach der Rueckfrage zu
/// ungespeicherten Aenderungen. false = abgebrochen.
bool beginneKomposition(VisualizerWidget& widget, QWidget* eltern);

/// Haengt Effect Chain gerade an einem Clip der Komposition (Sync on Effect
/// Chain)? Dann gehoert „Save Effect Chain" dem Composer (Naht N11).
[[nodiscard]] bool clipKetteGebunden(const VisualizerWidget& widget);

/// Die Kette des Clips speichern, an dem Effect Chain haengt (Naht N11,
/// Entscheid Patrik S95): immer mit Dateidialog und nie ueber das Original;
/// der Clip zeigt danach auf die neue Datei. Benutzen weitere Clips das alte
/// Preset, fragt eine Liste zum Ankreuzen, welche mitwechseln. false =
/// abgebrochen oder nicht geschrieben.
bool speichereClipKette(VisualizerWidget& widget, QWidget* eltern);

/// Beim Start der App (Naht N4): die zuletzt geladene Komposition wieder ins
/// Panel; mit „Resume after restart" auch ihr Stand (Konzept §8). false = es
/// gab keine.
bool stelleKompositionWiederHer(VisualizerWidget& widget);

/// Die Messung der laufenden Komposition als Zeilen (Zeiten, Renderer,
/// Grafikspeicher); leer, wenn keine laeuft. Dieselben Zeilen schreibt
/// `schliesseKomposition` ins Log, bevor die Bildquelle geht.
[[nodiscard]] QStringList messungDerKomposition(VisualizerWidget& widget);

/// Ein Transport-Befehl des Player-Panels, eines Hotkeys oder des Menues (Naht
/// N5, Entscheid Patrik S96): fuehrt ein Audiokanal des Composers den Player,
/// steuert der Befehl den ganzen Composer — Play / Pause, Stop (an den Anfang),
/// Next / Previous (zum Anfang des naechsten bzw. vorigen Songs). true = der
/// Composer hat ihn genommen. false = er fuehrt nicht, oder es ist Lautstaerke
/// oder Mute — dann geht der Befehl wie immer an den Player.
[[nodiscard]] bool transport(TransportCommandEvent::Action aktion);

/// Der Mixer, in dem die Songs der Komposition laufen (Naht N13, Entscheid
/// Patrik S97: die Visuals sehen die Summe mit den Pegeln): fuehrt der
/// Composer Ton, lesen Spektrum und Wellenform ihn statt des Streams des
/// Players. `INVALID_STREAM` = der Composer fuehrt keinen Ton — dann wie immer
/// der Stream des Players.
[[nodiscard]] AudioStreamHandle tonMixer();

// --- Das Player-Panel bei fuehrendem Composer (Naht N5, Anzeige; Entscheid
//     Patrik S98): es zeigt den laufenden Song als „Composer - <Song>". Titel,
//     Stelle und „laeuft" bekommt es ueber die Ereignisse, auf die es ohnehin
//     hoert — die Sitzung verschickt sie fuer den laufenden Song.

/// Fuehrt der Composer gerade den Ton?
[[nodiscard]] bool fuehrtTon();
/// Der laufende Song und sein Stand, fuer ein Panel, das erst jetzt sichtbar
/// wird. `fuehrt` false = der Composer fuehrt keinen Ton; dann gilt der Player.
struct TonStand
{
    bool fuehrt = false;
    bool laeuft = false;   ///< die Komposition laeuft
    bool hatSong = false;  ///< es wurde schon ein laufender Song gemeldet
    TrackInfo song;
    int stelleMs = 0;      ///< die Stelle in ihm
};
[[nodiscard]] TonStand tonStand();
/// Ein Klick in den Balken des Player-Panels: im laufenden Song an den
/// `anteil` (0…1) seiner Laenge — der Composer springt an die Stelle der
/// Komposition, an der das zu hoeren ist. true = der Composer fuehrt den Ton
/// und hat den Klick genommen (auch wenn es noch keinen Song gab). false = er
/// fuehrt nicht; der Klick geht wie immer an den Player.
[[nodiscard]] bool springeImSong(double anteil);

/// Meldet der Composer gerade, woran Effect Chain haengt (Naht N16, S101)?
/// Wahr nur waehrend die Sitzung ein `VisualizerChangedEvent` verschickt — an
/// einen Clip, an nichts oder zurueck an die Kette. Die Editoren der
/// Karaoke-Ebene uebergehen solche Meldungen: die Ebene gehoert der App.
[[nodiscard]] bool meldetBindung();

/// Der Weg des Menues: bei leerem Pfad nach der Datei fragen (letzter Ordner
/// unter `composer/lastDir`), laden, Meldungen zeigen. false = abgebrochen
/// oder nicht geladen.
bool oeffneKomposition(VisualizerWidget& widget, QWidget* eltern, const QString& pfad = {});

}  // namespace lumi::composer
