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

#include <QString>
#include <QStringList>

namespace lumi::loop
{

/// Liegen die Takte [`startTakt`, `startTakt + takte`) im Kanal frei? Der Clip
/// `ausser` zaehlt nicht mit (der, der gerade bewegt wird).
[[nodiscard]] bool istFrei(const Loop& loop, int kanal, int startTakt, int takte, int ausser = -1);
/// So viele Clips aller Kanaele spielen dieses Muster.
[[nodiscard]] int basisNutzer(const Loop& loop, const QString& muster);

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

// --- Muster: was der Drum-Editor aendert (Stufe 6b) --------------------------

/// Den Clip von seiner Basis loesen (Entscheid Patrik S108, L8 a): das Muster
/// wird unter einem neuen Namen kopiert („<Name> 2", die erste freie Zahl),
/// der Clip spielt die Kopie, das Original bleibt den anderen. Gibt den neuen
/// Namen zurueck; leer = den Clip gibt es nicht.
QString neueBasisAus(Loop& loop, int kanal, int clip);
/// Die Laenge des Musters setzen (`kTakteMin` bis `kTakteMax`): kuerzer
/// schneidet die hinteren Schritte ab, laenger haengt leere an. Seine Clips
/// werden mit ihm laenger oder kuerzer; ein Clip dahinter, in den einer
/// hineinwuechse, rueckt nach hinten.
bool setzeMusterTakte(Loop& loop, const QString& muster, int takte);
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
/// Das Muster zum Aendern; nullptr = es gibt keins dieses Namens.
[[nodiscard]] Muster* musterIn(Loop& loop, const QString& name);

/// Einen leeren Kanal anhaengen („Channel N"); gibt seinen Platz zurueck.
int kanalHinzu(Loop& loop);
/// Den Kanal mit seinen Clips entfernen; Basen ohne Clip gehen mit.
bool kanalLoeschen(Loop& loop, int kanal);
bool kanalUmbenennen(Loop& loop, int kanal, const QString& name);
bool kanalSchalten(Loop& loop, int kanal, bool an);

}  // namespace lumi::loop
