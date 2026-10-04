/**
 * @file AppAnbindung.hpp
 * @brief Composer: eine Komposition in der App ein- und ausschalten
 *        (docs/ui/Composer_Konzept.md §12.2 Naht N8, Stufe C2a)
 *
 * Solange es kein Panel gibt (C2b), schaltet das Menue File den Composer:
 * **Open composition…** laedt eine `.lvcomp`, setzt die Bildquelle an die
 * Stelle des Renderers der Kette (Naht R3) und startet die Uhr; **Close
 * composition** nimmt sie wieder weg — die Kette laeuft weiter wie zuvor.
 *
 * Das Menue meldet nur die beiden Ereignisse; das Hauptfenster reicht sie an
 * die Funktionen hier weiter. Alles Weitere ist eigener Code des Composers.
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

/// Eine `.lvcomp` laden, als Bildquelle setzen und die Uhr starten. Eine schon
/// laufende Komposition wird ersetzt. false = die Datei ist keine Komposition;
/// es bleibt alles, wie es war. `bericht` nennt, was gerichtet wurde und
/// welche Dateien fehlen.
bool starteKomposition(VisualizerWidget& widget, const QString& pfad,
                       QStringList* bericht = nullptr);

/// Die Bildquelle wegnehmen — der Renderer der Kette liefert wieder das Bild.
void schliesseKomposition(VisualizerWidget& widget);

/// Der Weg des Menues: bei leerem Pfad nach der Datei fragen (letzter Ordner
/// unter `composer/lastDir`), laden, Meldungen zeigen. false = abgebrochen
/// oder nicht geladen.
bool oeffneKomposition(VisualizerWidget& widget, QWidget* eltern, const QString& pfad = {});

}  // namespace lumi::composer
