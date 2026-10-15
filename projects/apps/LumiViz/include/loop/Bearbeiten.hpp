/**
 * @file Bearbeiten.hpp
 * @brief Loop: das Arrangement bearbeiten — Clips anlegen, verschieben,
 *        verlaengern, kuerzen, kopieren, loeschen; Kanaele
 *        (docs/audio/Loop_Konzept.md §4c, Stufe 6a)
 *
 * Reine Funktionen auf dem Modell, ohne Fenster: das Loop-Panel ruft sie aus
 * seinen Gesten, die Tests direkt. Jede haelt die Regeln des Modells ein:
 *
 * - Clips liegen auf **ganzen Takten** und ueberlappen sich im Kanal nicht.
 * - Ein Clip ist immer ein **Vielfaches seines Musters** lang (mindestens eins).
 * - Kopien bleiben **verbunden** (Entscheid Patrik S108, L8 a): sie nennen
 *   dasselbe Muster, die „Basis".
 *
 * Nach jeder Aenderung sind die Clips des Kanals nach Start sortiert; wo eine
 * Funktion einen Clip zurueckgibt, ist es sein Platz danach.
 */
#pragma once

#include "loop/Loop.hpp"

#include <optional>
#include <QString>
#include <QStringList>

#include <vector>

namespace lumi::loop
{

/// Liegen die Takte [`startTakt`, `startTakt + takte`) im Kanal frei? Der Clip
/// `ausser` zaehlt nicht mit (der, der gerade bewegt wird).
[[nodiscard]] bool istFrei(const Loop& loop, int kanal, int startTakt, int takte, int ausser = -1);
/// So viele Clips aller Kanaele spielen dieses Muster.
[[nodiscard]] int basisNutzer(const Loop& loop, const QString& muster);
/// Wo steht die Basis `name` aus `alt` in der Fassung `neu`? Ihr Name, wenn es
/// sie dort noch gibt; der neue Name, wenn sie umbenannt wurde (eine Basis,
/// die `alt` nicht kennt, mit demselben Inhalt); leer = es gibt sie nicht mehr.
/// So folgt ein Editor-Fenster seiner Basis.
[[nodiscard]] QString folgeBasis(const Loop& alt, const QString& name, const Loop& neu);

/// Einen leeren Drumkit-Clip anlegen (Vorgabe Patrik S108): eine neue Basis
/// „Pattern N", `kTakteMin` Takte lang, ohne Spuren. Liegt dort schon ein Clip
/// oder reicht der Platz bis zum naechsten nicht: -1, nichts geaendert.
/// Mit `MusterArt::Instrument` ein leerer Instrument-Clip (Stufe 9): dieselbe
/// Laenge, ohne Noten, das Instrument ist das erste der Auswahl.
int neuerClip(Loop& loop, int kanal, int startTakt, MusterArt art = MusterArt::Drumkit);

/// Den Clip nach `zielKanal` an `startTakt` legen. Ist dort kein Platz, rueckt
/// er an die naechste freie Stelle — an die Kante des Clips, der im Weg liegt,
/// davor oder dahinter, was naeher am Wunsch ist. -1 = kein Platz, nichts
/// geaendert.
int versetzeClip(Loop& loop, int kanal, int clip, int zielKanal, int startTakt);
/// Die rechte Kante ziehen: der Clip wiederholt sein Muster so oft, dass er
/// moeglichst nah an `endeTakt` endet — mindestens einmal, hoechstens bis zum
/// naechsten Clip.
int setzeEnde(Loop& loop, int kanal, int clip, int endeTakt);
/// Die linke Kante ziehen: das Ende bleibt, der Anfang rueckt in ganzen
/// Mustern — hoechstens bis zum Clip davor oder bis Takt 0.
int setzeAnfang(Loop& loop, int kanal, int clip, int startTakt);
/// Eine verbundene Kopie hinter den Clip legen, an die erste Stelle, an die
/// sie passt. Gibt ihren Platz zurueck.
int dupliziere(Loop& loop, int kanal, int clip);
/// Den Clip entfernen. Spielt danach kein Clip mehr seine Basis, geht sie mit
/// (sie waere nicht mehr zu erreichen).
bool loescheClip(Loop& loop, int kanal, int clip);
/// Die Basis umbenennen — fuer alle Clips, die sie spielen. false = der Name
/// ist leer oder schon vergeben.
bool benenneBasisUm(Loop& loop, const QString& alt, const QString& neu);

// --- Mehrere Clips zugleich (Stufe 12c) --------------------------------------

/// Ein Clip, benannt nach Kanal und Starttakt — im Kanal eindeutig, und
/// anders als sein Platz in der Liste bleibt es gueltig, wenn andere Clips
/// dazukommen oder gehen.
struct ClipOrt
{
    int kanal = -1;
    int startTakt = 0;

    [[nodiscard]] bool operator==(const ClipOrt&) const = default;
};

/// Der Platz des Clips in seinem Kanal; -1 = es gibt ihn nicht.
[[nodiscard]] int findeClip(const Loop& loop, ClipOrt ort);
/// Der Clip, dessen Effekte ein Editor beim Abspielen seines Musters nimmt
/// (Entscheid E1 a, S124): `markiert`, wenn er dieses Muster spielt — sonst
/// der erste Clip des Musters (Kanaele und Clips der Reihe nach). Leer = das
/// Muster hat keinen Clip.
[[nodiscard]] std::optional<ClipOrt> bezugsclip(const Loop& loop, const QString& muster, std::optional<ClipOrt> markiert);
/// „Drums, bar 5": wie ein Clip in Texten heisst.
[[nodiscard]] QString clipText(const Loop& loop, ClipOrt ort);
/// Was „Play pattern" spielt: ein Loop aus einem Kanal „Solo" mit einem Clip
/// dieses Musters ab Takt 1 — mit den Effekten und dem Schalter des
/// Bezugsclips (`bezugsclip`, E1 a). Ohne das Muster: der Loop, wie er ist.
[[nodiscard]] Loop soloFassung(const Loop& loop, const QString& muster, std::optional<ClipOrt> markiert);
/// Den Schalter der Effekte eines Clips setzen. false = es gibt ihn nicht, oder er stand schon so.
bool setzeEffekteAn(Loop& loop, ClipOrt ort, bool an);
/// Alle genannten Clips um dieselben Takte und Kanaele versetzen — ihre Lage
/// zueinander bleibt. Der Versatz wird so weit gekuerzt, dass kein Clip vor
/// Takt 0 oder ausserhalb der Kanaele landet. Anders als ein einzelner Clip
/// rueckt die Gruppe nicht an eine freie Kante: liegt an einem der Ziele ein
/// anderer Clip, geschieht nichts. true = versetzt, `orte` nennt die neuen
/// Stellen; false = nichts geaendert (auch bei Versatz null).
bool versetzeClips(Loop& loop, std::vector<ClipOrt>& orte, int kanalVersatz, int taktVersatz);
/// Verbundene Kopien aller genannten Clips anlegen, als Gruppe nach hinten
/// versetzt: an die erste Stelle hinter der Gruppe, an der alle Kopien Platz
/// haben. `orte` nennt danach die Kopien. Die Effekte eines Clips gehen mit.
bool dupliziereClips(Loop& loop, std::vector<ClipOrt>& orte);
/// Alle genannten Clips entfernen; Basen ohne Clip gehen mit.
bool loescheClips(Loop& loop, const std::vector<ClipOrt>& orte);

// --- Muster: was der Drum-Editor aendert (Stufe 6b) --------------------------

/// Den Clip von seiner Basis loesen (Entscheid Patrik S108, L8 a): das Muster
/// wird unter einem neuen Namen kopiert („<Name> 2", die erste freie Zahl),
/// der Clip spielt die Kopie, das Original bleibt den anderen. Gibt den neuen
/// Namen zurueck; leer = den Clip gibt es nicht.
QString neueBasisAus(Loop& loop, int kanal, int clip);
/// Die Laenge des Musters setzen (`kTakteMin` bis `kTakteMax`): kuerzer
/// schneidet die hinteren Schritte ab, laenger haengt leere an — oder, mit
/// `wiederholen` (Entscheid B2 a), wiederholt in den neuen Takten die
/// bisherigen der Reihe nach (aus 1 Takt vier gleiche, aus 3 auf 4 die Takte
/// 1, 2, 3, 1): Schritte mit ihrer Lautstaerke, Noten; eine Note, die ueber
/// das Ende reichte, wird gekuerzt. Seine Clips werden mit ihm laenger oder
/// kuerzer; ein Clip dahinter, in den einer hineinwuechse, rueckt nach hinten.
bool setzeMusterTakte(Loop& loop, const QString& muster, int takte, bool wiederholen = false);
/// Der Haken „Repeat when lengthening" beider Editoren (B2 a): das Programm
/// merkt ihn, zuerst ist er an.
[[nodiscard]] bool wiederholenBeimVerlaengern();
void setzeWiederholenBeimVerlaengern(bool an);
/// Das Raster des ganzen Loops setzen (je 1 bis `kRasterMax`). Jeder Schritt
/// bleibt an seiner Stelle im Takt, wo das neue Raster sie kennt: ein Schlag,
/// den es nicht mehr gibt, und ein Schritt, der zwischen zwei neue faellt,
/// gehen verloren.
bool setzeRaster(Loop& loop, int schlaegeJeTakt, int schritteJeSchlag);
/// Eine Spur an das Muster haengen („Track N"), mit einer leeren Zeile. Ihr
/// Klang ist der erste aus `kitKlaenge`, den das Muster noch nicht spielt —
/// sind alle vergeben oder ist die Liste leer, bleibt sie ohne Klang. Gibt
/// ihren Platz zurueck, -1 = das Muster gibt es nicht.
int spurHinzu(Loop& loop, const QString& muster, const QStringList& kitKlaenge);
bool spurLoeschen(Loop& loop, const QString& muster, int spur);
/// Die Spur `von` an den Platz `nach` legen, mit ihrer Zeile (S127, Entscheid
/// Patrik 7b a); die anderen ruecken nach. false = gibt es nicht, oder nichts zu tun.
bool spurVerschieben(Loop& loop, const QString& muster, int von, int nach);
/// Wo ein Platz `platz` liegt, nachdem einer von `von` nach `nach` gelegt
/// wurde (fuer Auswahl und Marken, die ihrem Kanal oder ihrer Spur folgen).
[[nodiscard]] int platzNachVerschieben(int platz, int von, int nach);
/// Das Muster zum Aendern; nullptr = es gibt keins dieses Namens.
[[nodiscard]] Muster* musterIn(Loop& loop, const QString& name);

/// Einen leeren Kanal anhaengen („Channel N"); gibt seinen Platz zurueck.
int kanalHinzu(Loop& loop);
/// Den Kanal mit seinen Clips entfernen; Basen ohne Clip gehen mit.
bool kanalLoeschen(Loop& loop, int kanal);
bool kanalUmbenennen(Loop& loop, int kanal, const QString& name);
bool kanalSchalten(Loop& loop, int kanal, bool an);
/// Den Kanal `von` an den Platz `nach` legen, mit seinen Clips (S127, 7b a). Der
/// Klang aendert sich nicht. false = gibt es nicht, oder nichts zu tun.
bool kanalVerschieben(Loop& loop, int von, int nach);

}  // namespace lumi::loop
