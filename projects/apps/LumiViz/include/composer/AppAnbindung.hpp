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

/// Beim Start der App (Naht N4): die zuletzt geladene Komposition wieder ins
/// Panel; mit „Resume after restart" auch ihr Stand (Konzept §8). false = es
/// gab keine.
bool stelleKompositionWiederHer(VisualizerWidget& widget);

/// Die Messung der laufenden Komposition als Zeilen (Zeiten, Renderer,
/// Grafikspeicher); leer, wenn keine laeuft. Dieselben Zeilen schreibt
/// `schliesseKomposition` ins Log, bevor die Bildquelle geht.
[[nodiscard]] QStringList messungDerKomposition(VisualizerWidget& widget);

/// Der Weg des Menues: bei leerem Pfad nach der Datei fragen (letzter Ordner
/// unter `composer/lastDir`), laden, Meldungen zeigen. false = abgebrochen
/// oder nicht geladen.
bool oeffneKomposition(VisualizerWidget& widget, QWidget* eltern, const QString& pfad = {});

}  // namespace lumi::composer
