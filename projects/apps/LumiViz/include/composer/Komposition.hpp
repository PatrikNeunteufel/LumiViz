/**
 * @file Komposition.hpp
 * @brief Composer: das Datenmodell einer Komposition und die Datei `.lvcomp`
 *        (docs/ui/Composer_Konzept.md §6, Stufe C1b)
 *
 * Eine Komposition ist eine Zeitachse in Millisekunden mit Kanaelen; in den
 * Kanaelen liegen Clips. Ein Clip im Visual-Kanal verweist auf eine
 * Preset-Datei (`.avs`, `.milk`, `.lvfx`, `.lvfx2`), einer im Audio-Kanal auf
 * einen Song (C5) — eingebettet wird nichts (Entscheid Patrik S60). Kein
 * Qt-Widget, kein GL, keine Audio-Engine: reines Modell.
 *
 * Rueckbau (Konzept §12): Teil von `include/composer` / `src/composer`.
 */
#pragma once

#include "composer/Mischer.hpp"
#include "composer/Pegelform.hpp"

#include <QJsonObject>
#include <QString>
#include <QStringList>

#include <array>
#include <cstdint>
#include <functional>
#include <vector>

namespace lumi::composer
{

/// Eine Blende am Clip: die eingestellte Dauer und die Form. Die Seite
/// (Anfang oder Ende) bestimmt die Richtung.
struct ClipBlende
{
    std::int64_t dauerMs = 0;  ///< eingestellt; 0 = keine Blende an dieser Seite
    Pegelform form = einfacheBlende();
};

/// Ein Stueck mitten im Clip: Lage und Weite wie eingestellt, dazu die Form.
struct ClipStueck
{
    double lage = 0.5;         ///< Mitte des Stuecks als Anteil der Clip-Laenge, ab Clip-Anfang
    std::int64_t weiteMs = 0;  ///< eingestellt
    bool gekippt = false;      ///< an der x-Achse gespiegelt: oben und unten getauscht
    Pegelform form = delle();
};

/// Ein Punkt der Pegelkurve, wie `.lvcomp` sie bis S95 trug (Feld `levels`):
/// Zeit im Clip, Pegel, Form des Abschnitts bis zum naechsten Punkt.
struct AltPunkt
{
    std::int64_t zeitMs = 0;
    double pegel = 1.0;
    Kurvenform form = Kurvenform::Linear;
    BezierGriffe bezier = kBezierVorgabe;
};

/// Ein Clip auf der Zeitachse — EIN Typ fuer Bild und Ton (Entscheid Patrik
/// S96, C5): beide haben Start und Ende, beide dieselbe Pegelkurve mit
/// denselben Presets. Was er ist, sagt der Kanal, in dem er liegt
/// (`Kanal::art`): im Visual-Kanal eine Preset-Datei, deren Bild mit
/// `mischart` gemischt wird; im Audio-Kanal ein Song, der ab `einsatzMs`
/// spielt und dessen Pegel die Lautstaerke ist.
struct Clip
{
    QString datei;  ///< Preset bzw. Song, absolut (in der Datei relativ zur `.lvcomp`, wo moeglich)
    std::int64_t startMs = 0;
    std::int64_t endeMs = 0;  ///< ausschliesslich: der Clip laeuft in [start, ende)
    Mischart mischart = Mischart::Normal;  ///< nur Bild

    // --- nur Ton ---------------------------------------------------------------
    /// Ab dieser Stelle im Song beginnt der Clip. Wird die linke Kante
    /// gezogen, wandert der Einsatz mit — der Rest bleibt, wo er zu hoeren war.
    std::int64_t einsatzMs = 0;
    /// Laenge des Songs; 0 = unbekannt. Ueber sie hinaus laesst sich der Clip
    /// nicht ziehen.
    std::int64_t quellMs = 0;

    // --- Pegelkurve (Entscheide Patrik S96): alle Teile sind freiwillig ------
    /// Der Pegel zwischen den Teilen, 0…1. Die Blenden laufen von null bis
    /// hierher; eine Delle senkt von hier in Richtung null, ein Buckel hebt
    /// von hier in Richtung voll.
    double grundpegel = 1.0;
    ClipBlende ein;
    ClipBlende aus;
    std::vector<ClipStueck> stuecke;

    [[nodiscard]] std::int64_t dauerMs() const { return endeMs - startMs; }
    /// Wo die Teile bei der jetzigen Clip-Laenge liegen — gestaucht, wo sie
    /// sich ueberdecken. Die eingestellten Werte bleiben, wie sie sind.
    [[nodiscard]] Verteilung verteilung() const;
    /// Pegel an der Stelle `zeitImClipMs` (0 = Clip-Start).
    [[nodiscard]] double pegelBei(std::int64_t zeitImClipMs) const;
    /// Ein- und Ausblenden mit einer einfachen Form setzen: von 0 auf den
    /// Grundpegel in `einMs`, zurueck auf 0 in den letzten `ausMs`. 0 = kein
    /// Blenden an diesem Ende. Die Stuecke bleiben.
    void setzeBlenden(std::int64_t einMs, std::int64_t ausMs,
                      Kurvenform form = Kurvenform::Linear);
    /// Laeuft der Clip durchweg auf vollem Pegel — keine Blende, kein Stueck,
    /// Grundpegel 1?
    [[nodiscard]] bool vollerPegel() const;
    /// Die Stelle im Song, die an `zeitImClipMs` zu hoeren ist (nur Ton).
    [[nodiscard]] std::int64_t songStelle(std::int64_t zeitImClipMs) const { return einsatzMs + zeitImClipMs; }
};

/// Die Namen aus der Zeit, als Bild und Ton zwei Typen waren.
using VisualClip = Clip;
using AudioClip = Clip;

/// Eine alte Punktkurve in die Teile des Clips uebersetzen (ersetzt Blenden,
/// Stuecke und Grundpegel). Das Muster einer einfachen Ein- / Ausblendung wird
/// zu Blenden; jede andere Kurve zu EINEM Stueck ueber die ganze Clip-Laenge —
/// der Pegelverlauf bleibt derselbe.
void ausAltenPunkten(Clip& clip, std::vector<AltPunkt> punkte);

enum class KanalArt : int
{
    Visual = 0,
    Audio = 1,
};

struct Kanal
{
    KanalArt art = KanalArt::Visual;
    QString name;
    bool an = true;
    /// Bild- oder Ton-Clips, je nach `art`; nach Start sortiert, ueberlappungsfrei.
    std::vector<Clip> clips;
};

/// Liefert die Laenge eines Songs in Millisekunden, 0 = unbekannt. Das Modell
/// kennt keine Audio-Engine — wer laedt, reicht die Auskunft herein.
using SongLaenge = std::function<std::int64_t(const QString& datei)>;

struct Komposition
{
    /// Reihenfolge = Stapelreihenfolge der Visual-Kanaele, unten zuerst.
    std::vector<Kanal> kanaele;
    /// 0 = das Ende folgt dem letzten Clip (Vorgabe); groesser = festes Ende —
    /// die Komposition laeuft bis dorthin, auch wenn kein Clip mehr liegt.
    std::int64_t laengeMs = 0;
    double rasterSekunden = 1.0;  ///< Einrasten beim Ziehen (Bedienung)

    // --- Schleife und Blende der ganzen Komposition (Entscheide Patrik S99,
    //     Konzept §4) — alle Werte gehoeren zur Komposition, Vorgabe „nichts" ---
    /// Eingestellt: in der Schleife beginnt der Anfang um diese Zeit vor dem
    /// Ende; im Fenster laufen die letzten und die ersten Millisekunden
    /// zugleich. Was davon wirkt, sagt `ueberlappung()`.
    std::int64_t ueberlappungMs = 0;
    /// Ein Song, der im vorderen UND im hinteren Fenster liegt, bekommt im
    /// Fenster eine zweite Quelle: sein Ende klingt aus, waehrend sein Anfang spielt.
    bool songsDoppelt = true;
    /// Dasselbe fuer einen Bild-Clip: ein zweites, frisch gestartetes Exemplar.
    /// Aus = er laeuft einmal durch, im Fenster gilt der hoehere der zwei Pegel.
    bool bilderDoppelt = false;
    /// Ein- und Ausblenden der GANZEN Komposition, Bild und Ton; 0 = keins.
    /// Gilt immer — auch beim ersten Start und am Ende ohne Schleife.
    std::int64_t einblendeMs = 0;
    std::int64_t ausblendeMs = 0;

    /// Die Ueberlappung, die wirkt: hoechstens die halbe Laenge — die zwei
    /// Fenster beruehren sich dann, sie kreuzen sich nie.
    [[nodiscard]] std::int64_t ueberlappung() const;
    /// Die Dauer eines Umlaufs in der Schleife: Laenge weniger Ueberlappung.
    [[nodiscard]] std::int64_t umlaufMs() const;
    /// Zu einer Stelle im vorderen Fenster die Stelle im hinteren, die in der
    /// Schleife zugleich laeuft; -1 = `stelleMs` liegt nicht im vorderen Fenster.
    [[nodiscard]] std::int64_t endStelleZu(std::int64_t stelleMs) const;
    /// Pegel der Blende der ganzen Komposition an der Stelle, 0…1 (linear).
    /// Ueberdecken sich Ein- und Ausblendung, gilt der kleinere Wert.
    [[nodiscard]] double blendeBei(std::int64_t stelleMs) const;

    /// Die gesetzte Laenge, sonst das Ende des letzten Clips — Bild oder Ton.
    [[nodiscard]] std::int64_t laenge() const;
    /// Das Ende des letzten Clips, gleich ob eine Laenge gesetzt ist.
    [[nodiscard]] std::int64_t clipEnde() const;
    /// Der erste eingeschaltete Audio-Kanal; -1 = keiner — dann fuehrt die
    /// Komposition keinen Ton.
    [[nodiscard]] int tonKanal() const;
    /// Die Ton-Clips, die an der Stelle `stelleMs` spielen — ALLE
    /// eingeschalteten Audio-Kanaele zaehlen, die Songs ueberlagern sich (C5b).
    /// Reihenfolge: der oberste Kanal zuerst. Mit `vorlaufMs` auch die Clips,
    /// die innerhalb dieser Zeit beginnen. Leer = dort ist Stille.
    [[nodiscard]] std::vector<const Clip*> toeneBei(std::int64_t stelleMs, std::int64_t vorlaufMs = 0) const;
    /// Fuer Ton-Clips ohne bekannte Song-Laenge die Laenge erfragen; ein Clip,
    /// der laenger ist als sein Song ab dem Einsatz, wird auf ihn gekuerzt.
    void ergaenzeTon(const SongLaenge& laengeVon);
    /// Clips je Kanal nach Start sortieren, Unsinn richten (Ende vor Start,
    /// Ueberlappung im selben Kanal: der spaetere Clip beginnt, wo der
    /// fruehere endet), Blenden und Stuecke klemmen. Meldet, was gerichtet
    /// wurde.
    void richte(QStringList* bericht = nullptr);
};

/// Ueberlappung in der Schleife (S99): was Bild (`Wiedergabe::rendere`) und Ton
/// (`Tonfuehrung::takt`) ausser der Stelle der Uhr ueber die Schleife wissen
/// muessen. Im Fenster laufen die Clips der vorderen UND der hinteren Stelle;
/// vor dem Umschlag werden die ersten Clips der naechsten Runde vorbereitet.
/// (Eine eigene Struktur im Namensraum: als eingeschachtelte mit
/// Mitglieds-Vorgaben taugt sie nicht zum Vorgabewert eines Parameters ihrer
/// Klasse — clang lehnt das ab.)
struct Schleifenlage
{
    /// Die Stelle im hinteren Fenster, die zugleich laeuft; -1 = keine.
    std::int64_t zweiteMs = -1;
    /// So lange dauert es, bis die Schleife umschlaegt und der Anfang
    /// wieder beginnt; -1 = sie schlaegt nicht um (keine Schleife).
    std::int64_t umschlagInMs = -1;
};

/// Die Komposition als JSON. `basisOrdner` nicht leer: Dateien darunter
/// werden relativ geschrieben.
[[nodiscard]] QJsonObject alsJson(const Komposition& komposition, const QString& basisOrdner = {});
/// JSON lesen; relative Dateien werden gegen `basisOrdner` aufgeloest.
/// false = kein Composer-Dokument. Der Bericht nennt, was gerichtet wurde
/// und welche Dateien fehlen (die Clips bleiben trotzdem erhalten).
/// `laengeVon` (optional): die Laenge der Songs — ein Ton-Clip ohne Ende in
/// der Datei (bis S95) reicht dann bis zum Schluss seines Songs.
bool ausJson(const QJsonObject& wurzel, Komposition& komposition, const QString& basisOrdner = {},
             QStringList* bericht = nullptr, const SongLaenge& laengeVon = {});

bool speichereKomposition(const Komposition& komposition, const QString& pfad);
bool ladeKomposition(const QString& pfad, Komposition& komposition, QStringList* bericht = nullptr,
                     const SongLaenge& laengeVon = {});

}  // namespace lumi::composer
