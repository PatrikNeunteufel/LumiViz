/**
 ****************************************************************************************
 * @file   ShaderVorlagen.hpp
 * @brief  Vorlagen fuer die Shader-Felder der Effektkette (S83, Entscheid Patrik E4a)
 *
 * @details
 * Jedes Shader-Feld erwartet einen festen Einstieg (`mainImage`, `warp`,
 * `farbe`, `kraft`, ISF-`main`, MilkDrop-`shader_body`). Wer ein Feld neu
 * fuellt, bekommt hier einen Startpunkt: **„Empty"** (nur der Funktionsrumpf —
 * kommt aus dem Code und fehlt darum nie) und einfache Vorlagen als Dateien.
 *
 * Schluessel = Vertrag, bei Feldern mit zwei Stufen um die Stufe ergaenzt:
 * `shadertoy`, `meshwarp`, `pixelfilter`, `gpuparticles`, `isffilter`,
 * `isffilter/vs`, `milkdrop/warp`, `milkdrop/comp`. Der Schluessel ist
 * zugleich der Unterordner:
 *
 *     asset/shadertemplates/<schluessel>/<Name>.<endung>   (mitgeliefert)
 *     <AppData>/shadertemplates/<schluessel>/<Name>.<endung> (eigene)
 *
 * Bei gleichem Namen gewinnt die eigene Datei (wie `NodePresetStore`).
 * Ausgewaehlt wird nur im Groß-Editor (Entscheid Patrik S83), nicht im Panel.
 * Kein GL, keine GUI — voll unit-testbar.
 ****************************************************************************************
 */

#pragma once

#include <QString>
#include <QStringList>

#include <vector>

namespace lumi::multieffect::shadervorlagen {

struct Eintrag
{
    QString name;        ///< Anzeigename (= Dateiname ohne Endung; „Empty" fuer den Rumpf)
    QString text;        ///< Shader-Text
    QString pfad;        ///< Datei (leer beim Rumpf aus dem Code)
    bool builtin{true};  ///< false = eigene Vorlage im Benutzerordner
};

/// Alle bekannten Schluessel (SSOT fuer Tests und Panel).
[[nodiscard]] const QStringList& schluessel();

/// Nur der Funktionsrumpf des Vertrags — leer bei unbekanntem Schluessel.
[[nodiscard]] QString leererRumpf(const QString& key);

/// Dateiendung der Vorlagen eines Schluessels (`glsl`, `hlsl`, `fs`, `vs`).
[[nodiscard]] QString endung(const QString& key);

/**
 * Vorlagen eines Schluessels: zuerst „Empty" (aus dem Code), dann die Dateien
 * nach Namen sortiert. Unbekannter Schluessel → leere Liste.
 */
[[nodiscard]] std::vector<Eintrag> liste(const QString& key);

/**
 * Speichert `text` als eigene Vorlage (Name wird bereinigt). Ueberschreibt eine
 * gleichnamige eigene Datei. false = Schreibfehler oder unbekannter Schluessel.
 */
[[nodiscard]] bool speichere(const QString& key, const QString& name, const QString& text,
                             QString* outPfad = nullptr);

/// Verzeichnis der mitgelieferten Vorlagen eines Schluessels (leer = nicht gefunden).
[[nodiscard]] QString assetDir(const QString& key);

/// Verzeichnis der eigenen Vorlagen eines Schluessels (wird nicht angelegt).
[[nodiscard]] QString userDir(const QString& key);

/// Nur fuer Tests: Wurzel mit `asset/` und `user/` darunter (leer = normal).
void setRootForTesting(const QString& root);

}  // namespace lumi::multieffect::shadervorlagen
