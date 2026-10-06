/**
 * @file Bearbeiten.hpp
 * @brief Composer: Clips einer Komposition verschieben, an den Kanten ziehen,
 *        loeschen (docs/ui/Composer_Konzept.md §6, Stufe C3a)
 *
 * Reine Operationen auf dem Modell — kein Widget, kein GL. Jede haelt die
 * Regel „Clips eines Kanals ueberlappen nicht" ein; was geschieht, wenn der
 * gezogene Clip auf einen Nachbarn trifft, bestimmt `Stoss` (Entscheid Patrik
 * S95): anhalten, den Nachbarn kuerzen oder die Nachbarn schieben.
 *
 * Rueckbau (Konzept §12): Teil von `include/composer` / `src/composer`.
 */
#pragma once

#include "composer/Komposition.hpp"

#include <cstdint>

namespace lumi::composer
{

/// Was geschieht, wenn der gezogene Clip auf einen Nachbarn im selben Kanal trifft.
enum class Stoss : int
{
    Halten = 0,    ///< der gezogene Clip haelt am Nachbarn an (ohne Taste)
    Kuerzen = 1,   ///< der gezogene Clip gewinnt, der Nachbar wird gekuerzt (Ctrl)
    Schieben = 2,  ///< die Nachbarn ruecken mit, ihre Laenge bleibt (Ctrl+Shift)
};

/// Kuerzer wird kein Clip gezogen; ein Nachbar, der beim Kuerzen darunter
/// fiele, verschwindet.
inline constexpr std::int64_t kMinClipMs = 100;

/// Den Clip auf `neuerStartMs` verschieben; seine Laenge und Kurve bleiben.
/// Liefert den Platz des Clips im Kanal danach (er kann sich aendern), -1 =
/// es gibt den Clip nicht.
int verschiebeClip(Komposition& komposition, int kanal, int clip, std::int64_t neuerStartMs,
                   Stoss stoss = Stoss::Halten);

/// Wie `verschiebeClip`, und der Clip wechselt dabei in den Kanal `nachKanal`
/// — nur in einen seiner Art (Bild zu Bild, Ton zu Ton). Liefert seinen Platz
/// im Zielkanal, -1 = Clip oder Zielkanal gibt es nicht oder er ist von der
/// anderen Art — dann bleibt alles, wie es war.
int versetzeClip(Komposition& komposition, int vonKanal, int clip, int nachKanal,
                 std::int64_t neuerStartMs, Stoss stoss = Stoss::Halten);

/// Die linke (`links` = true) oder rechte Kante des Clips auf `zeitMs` ziehen.
/// Die Blenden bleiben an ihren Kanten, die Stuecke an ihrem Anteil; alle
/// behalten ihre eingestellte Weite. Ton-Clip: die linke Kante aendert den
/// Einsatz im Song (was bleibt, ist an derselben Stelle zu hoeren), und der
/// Clip reicht nie ueber Anfang oder Schluss des Songs hinaus. Liefert den
/// Platz des Clips danach, -1 = gibt es nicht.
int zieheKante(Komposition& komposition, int kanal, int clip, bool links, std::int64_t zeitMs,
               Stoss stoss = Stoss::Halten);

/// Den Clip entfernen. false = es gibt ihn nicht.
bool loescheClip(Komposition& komposition, int kanal, int clip);

// --- Neues hinein (C3b) -------------------------------------------------------

/// Laenge eines hineingezogenen Clips, wenn nichts im Weg ist.
inline constexpr std::int64_t kNeuerClipMs = 20000;

/// Einen neuen Clip mit vollem Pegel in den Kanal legen. Mit `Stoss::Halten`
/// passt er sich der Luecke an, in die er faellt: er reicht hoechstens bis zum
/// naechsten Clip; faellt er auf einen Clip, landet er in der naechsten Luecke.
/// Kuerzen und Schieben wie beim Verschieben. Liefert seinen Platz, -1 = den
/// Kanal gibt es nicht (oder er ist kein Visual-Kanal).
int fuegeClipEin(Komposition& komposition, int kanal, const QString& datei, std::int64_t startMs,
                 std::int64_t dauerMs = kNeuerClipMs, Stoss stoss = Stoss::Halten);

/// Einen Song als Ton-Clip in einen Audio-Kanal legen (C5): so lang wie der
/// Song (`songMs`; 0 = unbekannt, dann `kNeuerClipMs`), Einsatz am Anfang des
/// Songs, voller Pegel. Am Nachbarn wie `fuegeClipEin`. Liefert seinen Platz,
/// -1 = den Kanal gibt es nicht (oder er ist kein Audio-Kanal).
int fuegeSongEin(Komposition& komposition, int kanal, const QString& datei, std::int64_t startMs,
                 std::int64_t songMs, Stoss stoss = Stoss::Halten);

/// Die Kopie eines Clips in den Kanal legen (Kopieren / Einfuegen, S98): mit
/// allem, was er traegt — Laenge, Pegelkurve, Mischart, Einsatz im Song —, sein
/// **Anfang** bei `startMs`. Anders als ein neuer Clip passt er sich keiner
/// Luecke an: mit `Stoss::Halten` landet er in der naechsten Luecke, in die er
/// ganz passt. `art` ist die Art des Clips (Bild oder Ton); er geht nur in
/// einen Kanal dieser Art. Liefert seinen Platz, -1 = den Kanal gibt es nicht
/// oder er ist von der anderen Art.
int fuegeKopieEin(Komposition& komposition, int kanal, KanalArt art, const Clip& vorlage,
                  std::int64_t startMs, Stoss stoss = Stoss::Halten);

/// Einen leeren Kanal anhaengen; ohne Namen heisst ein Visual-Kanal „CH<n>",
/// ein Audio-Kanal „A<n>", je mit der kleinsten freien Nummer. Liefert seinen
/// Platz.
int fuegeKanalEin(Komposition& komposition, const QString& name = {}, KanalArt art = KanalArt::Visual);

/// Den Kanal samt seinen Clips entfernen. false = es gibt ihn nicht.
bool loescheKanal(Komposition& komposition, int kanal);

/// Eine neue, leere Komposition: ein Kanal „CH1", das Ende folgt den Clips.
[[nodiscard]] Komposition neueKomposition();

/// Start und Ende eines Clips aendern. Blenden und Stuecke behalten ihre
/// eingestellten Werte.
void setzeGrenzen(VisualClip& clip, std::int64_t startMs, std::int64_t endeMs);

// --- Ein- und Ausblenden (C4) -------------------------------------------------

/// Ein- und Ausblendung eines Clips: Laenge, Form und — bei Bézier — Anfasser.
struct Blenden
{
    std::int64_t einMs = 0;  ///< 0 = keine Einblendung
    std::int64_t ausMs = 0;
    Kurvenform einForm = Kurvenform::Linear;
    Kurvenform ausForm = Kurvenform::Linear;
    BezierGriffe einBezier = kBezierVorgabe;
    BezierGriffe ausBezier = kBezierVorgabe;
};

/// Die Blenden des Clips lesen — die eingestellten Dauern, nicht die
/// gestauchten. false = eine Blende traegt eine Form mit eigenen Punkten (aus
/// dem Curve Editor oder einem Preset) — dann bietet die Zeitachse keine
/// Griffe fuer Form und Anfasser an.
[[nodiscard]] bool liesBlenden(const VisualClip& clip, Blenden& blenden);

/// Beide Blenden des Clips setzen, je mit einer einfachen Form. Die Dauern
/// gelten wie eingestellt, auch wenn sie zusammen laenger sind als der Clip
/// (gestaucht wird beim Rechnen). Stuecke und Grundpegel bleiben.
void setzeBlenden(VisualClip& clip, const Blenden& blenden);

/// Den sichtbaren Stand festschreiben (Entscheid Patrik S96): jedes
/// gestauchte Teil bekommt als eingestellten Wert, was es gerade einnimmt —
/// Dauer bzw. Weite, ein Stueck auch seine Lage. Teile, die nicht gestaucht
/// sind, bleiben. Gedacht fuer den Augenblick, bevor ein gestauchtes Teil von
/// Hand geaendert wird: danach bewegt sich nur, was angefasst wird.
void schreibeFest(VisualClip& clip);

// --- Teile der Pegelkurve (C4 Teil 2) -----------------------------------------

enum class TeilArt : int
{
    Keins = 0,
    Ein = 1,     ///< die Einblendung
    Aus = 2,     ///< die Ausblendung
    Stueck = 3,  ///< ein Stueck mitten im Clip
};

/// Ein Teil der Pegelkurve eines Clips — das, woran der Curve Editor haengt.
struct Teil
{
    TeilArt art = TeilArt::Keins;
    int stueck = -1;  ///< nur bei `Stueck`: sein Platz in `VisualClip::stuecke`

    [[nodiscard]] bool operator==(const Teil& o) const { return art == o.art && stueck == o.stueck; }
};

/// Gibt es das Teil am Clip — eine Blende mit Dauer bzw. das Stueck?
[[nodiscard]] bool hatTeil(const VisualClip& clip, Teil teil);
/// Das Teil, das an der Stelle `zeitImClipMs` liegt (nach der jetzigen
/// Verteilung); `Keins` = dort gilt nur der Grundpegel.
[[nodiscard]] Teil teilBei(const VisualClip& clip, std::int64_t zeitImClipMs);
/// Wo das Teil gerade liegt; ein leerer Bereich, wenn es das Teil nicht gibt.
[[nodiscard]] Bereich bereichVon(const VisualClip& clip, Teil teil);

/// Weite eines neuen Stuecks, wenn der Clip lang genug ist.
inline constexpr std::int64_t kNeuesStueckMs = 2000;

/// Ein Stueck (eine weiche Delle) mit der Mitte bei `mitteMs` in den Clip
/// legen; seine Weite ist `kNeuesStueckMs`, hoechstens ein Viertel des Clips.
/// Liefert seinen Platz in `stuecke`.
int fuegeStueckEin(VisualClip& clip, std::int64_t mitteMs);
/// Das Stueck entfernen. false = es gibt es nicht.
bool loescheStueck(VisualClip& clip, int stueck);

/// Schmaler wird kein Stueck gezogen.
inline constexpr std::int64_t kMinStueckMs = 100;

/// Der Platz, den ein Stueck einnehmen darf, ohne ein anderes Teil zu
/// beruehren: vom Ende des Teils davor bis zum Anfang des Teils dahinter (nach
/// der jetzigen Verteilung; ohne Nachbarn die Kanten des Clips).
[[nodiscard]] Bereich spielraumVon(const VisualClip& clip, int stueck);

/// Das Stueck mit seiner Mitte auf `mitteMs` schieben (Griff auf der
/// Zeitachse). Zuerst wird der sichtbare Stand festgeschrieben; dann haelt das
/// Stueck an seinen Nachbarn an — wie ein Clip am Nachbarn —, es wird also nie
/// durch das Schieben gestaucht. false = es gibt es nicht.
bool verschiebeStueck(VisualClip& clip, int stueck, std::int64_t mitteMs);

/// Die Weite des Stuecks setzen, seine Mitte bleibt. Zuerst wird der sichtbare
/// Stand festgeschrieben; breiter als bis an die Nachbarn wird es nicht.
bool setzeStueckWeite(VisualClip& clip, int stueck, std::int64_t weiteMs);

/// Name der Kurvenform fuer die Oberflaeche (englisch, wie die App).
[[nodiscard]] const char* kurvenformName(Kurvenform form);

/// Sind zwei Kompositionen gleich (Kanaele, Clips, Kurven, Laenge, Raster)?
[[nodiscard]] bool gleich(const Komposition& a, const Komposition& b);

}  // namespace lumi::composer
