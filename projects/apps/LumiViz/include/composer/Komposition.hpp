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

/// Die Farbreihen der Kanaele (Entscheid Patrik S100, K1 b): Bild-Kanaele
/// bekommen kuehle Toene, Audio-Kanaele warme — die Art bleibt an der Farbe
/// erkennbar. `nr` zaehlt in der Reihe und laeuft um. Der Wert ist 0xRRGGBB.
[[nodiscard]] std::int32_t reihenFarbe(KanalArt art, int nr);
/// So viele Farben hat eine Reihe.
inline constexpr int kReihenFarben = 6;

struct Kanal
{
    KanalArt art = KanalArt::Visual;
    QString name;
    bool an = true;
    /// Die Farbe der Clips, 0xRRGGBB — sie gehoert dem Kanal und steht in der
    /// Datei (S100): er behaelt sie, wenn er verschoben oder ein anderer
    /// geloescht wird. -1 = noch keine vergeben (`Komposition::vergibFarben`).
    std::int32_t farbe = -1;
    /// Bild- oder Ton-Clips, je nach `art`; nach Start sortiert, ueberlappungsfrei.
    std::vector<Clip> clips;
};

/// Ein Stueck der Pegelkurve der GANZEN Komposition (Entscheid Patrik S100,
/// F7 c). Wie `ClipStueck`, aber an einer festen Stelle der Zeitachse (F7-1 a):
/// aendert sich die Laenge der Komposition, bleibt es dort. Liegt seine Mitte
/// hinter dem Ende, wirkt es nicht — und bleibt gespeichert.
struct GesamtStueck
{
    std::int64_t mitteMs = 0;  ///< Mitte des Stuecks, ab Anfang der Komposition
    std::int64_t weiteMs = 0;  ///< eingestellt
    bool gekippt = false;
    Pegelform form = delle();
};

/// Die Kurven ueber die ganze Komposition (S101, Entscheid Patrik AM1: a):
/// die der Komposition selbst wirkt auf Bild UND Ton (Zeile MASTER), dazu je
/// eine nur fuer das Bild (VISUAL) und nur fuer den Ton (AUDIO).
/// Dazu eine fuer die Karaoke-Ebene (KARAOKE, Entscheid Patrik KS1: a).
enum class MasterArt
{
    Gesamt,
    Bild,
    Ton,
    Karaoke
};

/// Eine Kurve ueber die ganze Komposition: Blenden mit Form, Grundpegel und
/// Stuecke an fester Zeit — dieselben Teile wie die Kurve der Komposition.
struct MasterKurve
{
    std::int64_t einblendeMs = 0;
    std::int64_t ausblendeMs = 0;
    Pegelform einblendeForm = einfacheBlende();
    Pegelform ausblendeForm = einfacheBlende();
    double grundpegel = 1.0;
    std::vector<GesamtStueck> stuecke;

    /// Wirkt sie ueberhaupt — eine Blende, ein Stueck, ein Grundpegel unter voll?
    [[nodiscard]] bool wirkt() const
    {
        return einblendeMs > 0 || ausblendeMs > 0 || grundpegel < 1.0 || !stuecke.empty();
    }
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
    // --- die Pegelkurve der ganzen Komposition, wie am Clip (Entscheide Patrik
    //     S100: F7 c, F7-1 a; Konzept §4) -------------------------------------
    /// Die Formen der zwei Blenden; Vorgabe linear.
    Pegelform einblendeForm = einfacheBlende();
    Pegelform ausblendeForm = einfacheBlende();
    /// Der Pegel zwischen den Teilen, 0…1 — Bild und Ton.
    double grundpegel = 1.0;
    /// Stuecke an festen Stellen der Zeitachse.
    std::vector<GesamtStueck> gesamtStuecke;

    /// Die Kurve als Clip ueber [0, Laenge): Grundpegel, Blenden und die
    /// Stuecke, deren Mitte vor dem Ende liegt, mit der Lage als Anteil. Damit
    /// rechnen `blendeBei`, die Zeitachse und der Curve Editor wie an einem
    /// Clip. `herkunft` (wenn gegeben): je Stueck des Clips der Index in
    /// `gesamtStuecke`.
    /// `art`: welche der drei Kurven (Vorgabe: die der Komposition, MASTER).
    [[nodiscard]] Clip kurveAlsClip(std::vector<int>* herkunft = nullptr,
                                    MasterArt art = MasterArt::Gesamt) const;
    /// Die Kurve aus einem solchen Clip zuruecknehmen (nach dem Bearbeiten):
    /// Grundpegel, Blenden und Stuecke — die Lage wird zur festen Zeit. Stuecke
    /// hinter dem Ende, die der Clip nicht trug, bleiben.
    void setzeKurveAusClip(const Clip& clip, MasterArt art = MasterArt::Gesamt);
    /// Hat die Komposition ueberhaupt eine Kurve — eine Blende, ein Stueck
    /// oder einen Grundpegel unter voll?
    [[nodiscard]] bool hatKurve(MasterArt art = MasterArt::Gesamt) const;

    // --- Visual-Master und Audio-Master (S101, Entscheid Patrik AM1: a) -------
    /// Je eine weitere Kurve nur fuer das Bild und nur fuer den Ton, mit allem,
    /// was die Kurve der Komposition hat. Vorgabe: wirkt nicht.
    MasterKurve bildKurve;
    MasterKurve tonKurve;
    /// Die Kurve der Karaoke-Ebene (S101, Entscheid Patrik KS1: a): wie stark
    /// der Songtext ueber dem Gemischten liegt. Sie steht fuer sich — MASTER
    /// und VISUAL blenden die Ebene nicht mit (wie seit S99).
    MasterKurve karaokeKurve;
    [[nodiscard]] double karaokePegelBei(std::int64_t stelleMs) const
    {
        return masterBei(MasterArt::Karaoke, stelleMs);
    }
    /// Die Kurve einer Art als Wert; fuer `Gesamt` aus den Feldern oben.
    [[nodiscard]] MasterKurve masterKurve(MasterArt art) const;
    void setzeMasterKurve(MasterArt art, MasterKurve kurve);
    /// Pegel einer der drei Kurven allein an der Stelle, 0…1.
    [[nodiscard]] double masterBei(MasterArt art, std::int64_t stelleMs) const;
    /// Was auf das Bild bzw. den Ton wirkt: die Kurve der Komposition MAL die
    /// des Bilds bzw. des Tons — so, wie ein Clip seinen Pegel mit der Kurve
    /// der Komposition malnimmt.
    [[nodiscard]] double bildPegelBei(std::int64_t stelleMs) const;
    [[nodiscard]] double tonPegelBei(std::int64_t stelleMs) const;

    /// Die Ueberlappung, die wirkt: hoechstens die halbe Laenge — die zwei
    /// Fenster beruehren sich dann, sie kreuzen sich nie.
    [[nodiscard]] std::int64_t ueberlappung() const;
    /// Die Dauer eines Umlaufs in der Schleife: Laenge weniger Ueberlappung.
    [[nodiscard]] std::int64_t umlaufMs() const;
    /// Zu einer Stelle im vorderen Fenster die Stelle im hinteren, die in der
    /// Schleife zugleich laeuft; -1 = `stelleMs` liegt nicht im vorderen Fenster.
    [[nodiscard]] std::int64_t endStelleZu(std::int64_t stelleMs) const;
    /// Pegel der Kurve der ganzen Komposition an der Stelle, 0…1 — gerechnet
    /// wie die Pegelkurve eines Clips ueber [0, Laenge): ueberdecken sich
    /// Teile, werden sie gestaucht (bis S99: linear, der kleinere Wert galt).
    [[nodiscard]] double blendeBei(std::int64_t stelleMs) const;

    // --- Kanalfarben (S100, Entscheid Patrik K1 b) ----------------------------
    /// Die erste Farbe der Reihe dieser Art, die noch kein Kanal der Art
    /// traegt; sind alle vergeben, zaehlt die Reihe von vorn weiter.
    [[nodiscard]] std::int32_t freieFarbe(KanalArt art) const;
    /// Jedem Kanal ohne Farbe eine geben, in der Reihenfolge der Kanaele.
    /// Kanaele mit Farbe bleiben, wie sie sind.
    void vergibFarben();
    /// Die Farbe, in der die Clips des Kanals gezeichnet werden: seine eigene
    /// — oder, solange er keine hat, die der Reihe nach seinem Platz unter
    /// den Kanaelen seiner Art.
    [[nodiscard]] std::int32_t farbeVon(int kanal) const;

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
