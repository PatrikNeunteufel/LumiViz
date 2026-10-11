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
int neuerClip(Loop& loop, int kanal, int startTakt);

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

/// Einen leeren Kanal anhaengen („Channel N"); gibt seinen Platz zurueck.
int kanalHinzu(Loop& loop);
/// Den Kanal mit seinen Clips entfernen; Basen ohne Clip gehen mit.
bool kanalLoeschen(Loop& loop, int kanal);
bool kanalUmbenennen(Loop& loop, int kanal, const QString& name);
bool kanalSchalten(Loop& loop, int kanal, bool an);

}  // namespace lumi::loop
