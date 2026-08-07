/**
 ****************************************************************************************
 * @file   IsfImport.hpp
 * @brief  Import von ISF-Filtern (Interactive Shader Format) auf den
 *         `pixelFilter`-Vertrag — Stufe 1 des Filter-Strangs (S72)
 *
 * @author LumiPulse Team
 * @date   August 2026
 * @version 1.0.0
 *
 * @details
 * ISF ist „Shadertoy für Filter" (Vidvox, isf.video): GLSL plus eine
 * JSON-Parameterdeklaration. Die Kategorie **FX** hat exakt unseren
 * `pixelFilter`-Vertrag — Bild rein, Bild raus — und macht damit eine
 * gepflegte Online-Bibliothek nutzbar.
 *
 * **Format (gegen die Spec geprüft, S71):** Das JSON steckt als
 * Blockkommentar `/*{ … }* /` am DATEIANFANG, es gibt KEINE Nachbardatei;
 * Endung `.fs`. Eine ISF-Datei ist für sich vollständig.
 *
 * **Kategorie-Erkennung ist maschinell eindeutig:** ein Filter hat einen
 * Input `inputImage` vom Typ `image`. Generatoren haben keinen; Übergänge
 * tragen stattdessen `startImage`/`endImage`/`progress`. Beide werden mit
 * klarer Meldung ABGELEHNT statt still halb übersetzt — ein Generator ohne
 * Quellbild ergäbe im Filter-Knoten nur Unsinn.
 *
 * **Übersetzung auf unseren Vertrag:**
 * | ISF | LumiViz `pixelFilter` |
 * |---|---|
 * | `void main()` + `gl_FragColor` | Rumpf von `vec4 farbe(vec2 uv, vec4 src)` |
 * | `IMG_THIS_PIXEL(inputImage)` | `src` |
 * | `IMG_NORM_PIXEL(inputImage, p)` | `texture(uTex, p)` |
 * | `IMG_PIXEL(inputImage, p)` | `texture(uTex, (p) / uResolution)` |
 * | `IMG_SIZE(inputImage)` | `uResolution` |
 * | `isf_FragNormCoord` / `vv_FragNormCoord` | `uv` |
 * | `RENDERSIZE` · `TIME` · `TIMEDELTA` · `FRAMEINDEX` | `uResolution` · `uTime` · `uDelta` · `uFrame` |
 *
 * **`gl_FragColor` wird NICHT direkt zu `return`**, sondern zu einer lokalen
 * Variablen mit `return` am Ende. Nur so überlebt ein vorzeitiges `return;`
 * im Original: in einer `vec4`-Funktion wäre das ein Compilerfehler, und
 * viele ISF-Filter benutzen genau dieses Muster für ihren Passthrough-Zweig.
 *
 * **INPUTS in Stufe 1:** als `const`-Block mit den Vorgabewerten vor dem
 * Code — damit kompiliert und rendert der Import SOFORT. Der Block ist von
 * Sentinel-Kommentaren eingefasst; Stufe 2/3 (Parameter-Ablage + -Baum)
 * erzeugt ihn aus den Nutzerwerten neu, statt ihn zu suchen.
 *
 * **Grenzen (als Report-Zeilen, kein Hard-Fail):** `PASSES` (Multipass —
 * das läuft über den Shadertoy-Knoten), fremde `image`-Inputs (bleiben
 * schwarz), `audio`/`audioFFT` (der Filter-Knoten hat die Audio-Skalare
 * bass/mid/treb/vol/beat, aber keine Audio-Textur), `IMPORTED`-Texturen.
 *
 * Vertragsgrenze wie `PixelFilterWrapper`: Text → Text, kein GL, keine
 * GUI — voll unit-testbar.
 ****************************************************************************************
 */

#pragma once

#include "visualizers/multieffect/EffectChain.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QRegularExpression>
#include <QString>
#include <QStringList>

#include <algorithm>
#include <array>

namespace lumi::isf {

/// Sentinel um den erzeugten Parameter-Block — Stufe 2/3 erzeugt ihn neu.
inline constexpr const char* kParamBlockStart = "// --- ISF-Parameter (erzeugt) ---";
inline constexpr const char* kParamBlockEnde = "// --- Ende ISF-Parameter ---";

/// Die INPUT-Typen der ISF-Spec. `Unbekannt` = etwas Neues, wird gemeldet.
enum class IsfTyp
{
    Event,
    Bool,
    Long,
    Float,
    Point2D,
    Color,
    Image,
    Audio,
    AudioFft,
    Unbekannt,
};

/// Eine Groesse, die der `.vs`-Begleiter je Vertex vorrechnet und der
/// Fragment-Shader als `in`/`varying` liest (z. B. `left_coord`).
struct IsfVarying
{
    QString typ;   ///< GLSL-Typ, z. B. "vec2"
    QString name;  ///< Bezeichner, z. B. "left_coord"
};

/// Ein deklarierter Regler des Shaders (`INPUTS[i]`).
struct IsfParam
{
    QString name;   ///< GLSL-Bezeichner (`NAME`) — so heisst er im Code
    QString label;  ///< Klartext (`LABEL`), sonst `NAME`
    IsfTyp typ = IsfTyp::Unbekannt;

    double zahl = 0.0;   ///< DEFAULT fuer float/long/event/bool (bool: 0/1)
    double min = 0.0;    ///< MIN — nur gueltig bei `hatBereich`
    double max = 1.0;    ///< MAX — nur gueltig bei `hatBereich`
    bool hatBereich = false;

    std::array<double, 4> vektor{{0.0, 0.0, 0.0, 0.0}};  ///< point2D (xy) / color (rgba)

    QStringList auswahlLabels;  ///< `LABELS` eines `long` — Klartext im Dropdown
    QList<int> auswahlWerte;    ///< `VALUES` eines `long` — die echten Zahlen
};

/// Was der Import gefunden hat.
struct ImportErgebnis
{
    bool ok = false;
    QString error;  ///< harter Fehler (kein ISF, Generator, kaputtes JSON)

    QString code;  ///< fertiger Rumpf: definiert `vec4 farbe(vec2 uv, vec4 src)`
    QList<IsfParam> parameter;
    lumi::multieffect::Herkunft herkunft;
    QStringList report;  ///< Grenzen/Hinweise, "ℹ "-Praefix (Import-Report-Stil)
};

namespace detail {

/// Argumente eines Makro-Aufrufs ab der oeffnenden Klammer, auf OBERSTER Ebene
/// getrennt. `IMG_NORM_PIXEL(img, vec2(a, b))` hat zwei Argumente, nicht drei —
/// ohne Klammer-Zaehlung zerlegt eine naive Trennung den zweiten mitten drin.
/// `ende` bekommt die Position HINTER der schliessenden Klammer.
[[nodiscard]] inline QStringList makroArgumente(const QString& s, int klammerAuf,
                                                int& ende)
{
    QStringList args;
    QString cur;
    int tiefe = 0;
    for (int i = klammerAuf; i < s.size(); ++i)
    {
        const QChar c = s.at(i);
        if (c == QLatin1Char('('))
        {
            ++tiefe;
            if (tiefe == 1) continue;  // die oeffnende Klammer selbst
        }
        else if (c == QLatin1Char(')'))
        {
            --tiefe;
            if (tiefe == 0)
            {
                args << cur.trimmed();
                ende = i + 1;
                return args;
            }
        }
        else if (c == QLatin1Char(',') && tiefe == 1)
        {
            args << cur.trimmed();
            cur.clear();
            continue;
        }
        cur += c;
    }
    ende = s.size();  // unbalanciert — der Aufrufer laesst den Text stehen
    return {};
}

/// Ganze Woerter ersetzen (`\bNAME\b`) — `TIME` in `TIMEDELTA` bleibt heil.
inline void ersetzeWort(QString& s, const QString& wort, const QString& durch)
{
    static const QString kMuster = QStringLiteral("\\b%1\\b");
    const QRegularExpression re(kMuster.arg(QRegularExpression::escape(wort)));
    s.replace(re, durch);
}

/**
 * Einen funktionsartigen Makro-Aufruf ersetzen: `NAME(a, b)` -> `bau(args)`.
 *
 * Von HINTEN nach vorne, damit die schon berechneten Positionen durch das
 * Einsetzen nicht verrutschen; und ueber die Klammer-Zaehlung oben, damit
 * verschachtelte Aufrufe (`IMG_NORM_PIXEL(img, IMG_SIZE(img))`) halten.
 */
template <typename Bauer>
inline void ersetzeMakro(QString& s, const QString& name, Bauer bau)
{
    const QRegularExpression re(
        QStringLiteral("\\b%1\\s*\\(").arg(QRegularExpression::escape(name)));
    QList<QPair<int, int>> stellen;  // (start, klammerAuf)
    auto it = re.globalMatch(s);
    while (it.hasNext())
    {
        const auto m = it.next();
        stellen.append({m.capturedStart(), m.capturedEnd() - 1});
    }
    for (int i = stellen.size() - 1; i >= 0; --i)
    {
        int ende = 0;
        const QStringList args = makroArgumente(s, stellen[i].second, ende);
        if (args.isEmpty()) continue;  // unbalanciert — unangetastet lassen
        const QString ersatz = bau(args);
        if (ersatz.isNull()) continue;  // der Bauer will nicht — stehen lassen
        s.replace(stellen[i].first, ende - stellen[i].first, ersatz);
    }
}

/// Position des Rumpfes von `void main()`: `{` und das passende `}`.
/// Liefert false, wenn es kein `main` gibt oder die Klammern nicht aufgehen.
[[nodiscard]] inline bool findeMain(const QString& s, int& sigStart, int& auf,
                                    int& zu)
{
    static const QRegularExpression re(
        QStringLiteral("\\bvoid\\s+main\\s*\\(\\s*(?:void\\s*)?\\)\\s*\\{"));
    const auto m = re.match(s);
    if (!m.hasMatch()) return false;
    sigStart = m.capturedStart();
    auf = m.capturedEnd() - 1;
    int tiefe = 0;
    for (int i = auf; i < s.size(); ++i)
    {
        if (s.at(i) == QLatin1Char('{')) ++tiefe;
        else if (s.at(i) == QLatin1Char('}'))
        {
            --tiefe;
            if (tiefe == 0)
            {
                zu = i;
                return true;
            }
        }
    }
    return false;
}

[[nodiscard]] inline IsfTyp typVonText(const QString& t)
{
    const QString k = t.trimmed().toLower();
    if (k == QLatin1String("event")) return IsfTyp::Event;
    if (k == QLatin1String("bool")) return IsfTyp::Bool;
    if (k == QLatin1String("long")) return IsfTyp::Long;
    if (k == QLatin1String("float")) return IsfTyp::Float;
    if (k == QLatin1String("point2d")) return IsfTyp::Point2D;
    if (k == QLatin1String("color")) return IsfTyp::Color;
    if (k == QLatin1String("image")) return IsfTyp::Image;
    if (k == QLatin1String("audio")) return IsfTyp::Audio;
    if (k == QLatin1String("audiofft")) return IsfTyp::AudioFft;
    return IsfTyp::Unbekannt;
}

/// GLSL-Zahlliteral, das IMMER ein float ist (`1` waere ein int und brichte
/// `float x = 1;` in strengen Treibern).
[[nodiscard]] inline QString glslFloat(double v)
{
    QString s = QString::number(v, 'g', 9);
    if (!s.contains(QLatin1Char('.')) && !s.contains(QLatin1Char('e')) &&
        !s.contains(QLatin1Char('E')))
        s += QStringLiteral(".0");
    return s;
}

/// Regex fuer eine globale Varying-Deklaration in BEIDEN GLSL-Schreibweisen.
/// ISF-Dateien tragen sie doppelt, in einem `#if __VERSION__ <= 120`-Paar:
/// `varying` fuer alt, `in`/`out` fuer neu.
[[nodiscard]] inline QRegularExpression varyingRegex(const QString& richtungen,
                                                     const QString& name = {})
{
    const QString n = name.isEmpty() ? QStringLiteral("(\\w+)")
                                     : QRegularExpression::escape(name);
    return QRegularExpression(
        QStringLiteral("^[ \\t]*(?:%1)[ \\t]+(\\w+)[ \\t]+%2[ \\t]*;[ \\t]*\\r?\\n?")
            .arg(richtungen, n),
        QRegularExpression::MultilineOption);
}

/// Kommentare entfernen — fuer PRUEFUNGEN, nicht fuer die Ausgabe. Zwei der
/// Vidvox-`.vs` tragen ein auskommentiertes `gl_Position = ftransform();`;
/// ohne diesen Schritt wuerde die Geometrie-Erkennung unten sie faelschlich
/// ablehnen.
[[nodiscard]] inline QString ohneKommentare(QString s)
{
    static const QRegularExpression kBlock(QStringLiteral("/\\*.*?\\*/"),
                                           QRegularExpression::DotMatchesEverythingOption);
    static const QRegularExpression kZeile(QStringLiteral("//[^\\n]*"));
    s.remove(kBlock);
    s.remove(kZeile);
    return s;
}

/// Die Groessen, die der `.vs` an den Fragment-Shader weiterreicht.
[[nodiscard]] inline QList<IsfVarying> sammleVaryings(const QString& vs)
{
    QList<IsfVarying> out;
    auto it = varyingRegex(QStringLiteral("out|varying")).globalMatch(vs);
    while (it.hasNext())
    {
        const auto m = it.next();
        const IsfVarying v{m.captured(1), m.captured(2)};
        // Das `#if`-Paar deklariert JEDE Groesse zweimal — einmal reicht.
        const bool schonDa = std::any_of(
            out.begin(), out.end(),
            [&v](const IsfVarying& x) { return x.name == v.name; });
        if (!schonDa) out.append(v);
    }
    return out;
}

}  // namespace detail

/**
 * @brief Trennt den ISF-JSON-Kopf vom Shader-Code
 * @param inhalt kompletter Dateiinhalt
 * @param rumpf  bekommt alles NACH dem Kopfkommentar
 * @return der JSON-Text (ohne die Kommentarzeichen), leer wenn kein Kopf da ist
 *
 * Der Kopf MUSS das Erste in der Datei sein (Spec) — davor sind nur
 * Leerzeichen/Zeilenumbrueche erlaubt.
 */
[[nodiscard]] inline QString trenneKopf(const QString& inhalt, QString& rumpf)
{
    int i = 0;
    while (i < inhalt.size() && inhalt.at(i).isSpace()) ++i;
    if (i + 1 >= inhalt.size() || inhalt.at(i) != QLatin1Char('/') ||
        inhalt.at(i + 1) != QLatin1Char('*'))
    {
        rumpf = inhalt;
        return {};
    }
    const int zu = inhalt.indexOf(QStringLiteral("*/"), i + 2);
    if (zu < 0)
    {
        rumpf = inhalt;
        return {};
    }
    rumpf = inhalt.mid(zu + 2);
    return inhalt.mid(i + 2, zu - (i + 2)).trimmed();
}

/**
 * @brief Ist das eine ISF-Datei? (SSOT der Formaterkennung)
 *
 * Streng: der Kopf muss da sein UND gueltiges JSON-Objekt enthalten. Der
 * Import-Dialog benutzt dieselbe Antwort wie der Importeur — sonst warnt der
 * eine vor einer Datei, die der andere gar nicht annimmt.
 */
[[nodiscard]] inline bool istIsf(const QString& inhalt)
{
    QString rumpf;
    const QString kopf = trenneKopf(inhalt, rumpf);
    if (kopf.isEmpty() || !kopf.startsWith(QLatin1Char('{'))) return false;
    QJsonParseError err{};
    const QJsonDocument doc = QJsonDocument::fromJson(kopf.toUtf8(), &err);
    return err.error == QJsonParseError::NoError && doc.isObject();
}

/**
 * @brief Eine ISF-Datei (Kategorie FX) auf den `pixelFilter`-Vertrag uebersetzen
 * @param inhalt    kompletter Dateiinhalt der `.fs`
 * @param dateiName Dateiname ohne Endung — ISF kennt kein Titelfeld, der
 *                  Dateiname IST der Titel (Konvention der Bibliothek)
 * @param vertexInhalt Inhalt der gleichnamigen `.vs`, falls vorhanden.
 *        38 der 327 Dateien der Vidvox-Bibliothek haben einen — und zwar
 *        genau die interessante Klasse (Kanten, Blur, Sharpen): der
 *        Vertex-Shader rechnet die Nachbar-Koordinaten vor und reicht sie
 *        als Varying weiter. Ohne ihn LINKT der Fragment-Shader nicht.
 *        Da unser Filter ohnehin je Pixel laeuft, wandert diese Rechnung
 *        unveraendert in `farbe()` hinein — dieselben Werte, nur spaeter
 *        ausgewertet.
 */
[[nodiscard]] inline ImportErgebnis importiereIsf(const QString& inhalt,
                                                  const QString& dateiName = {},
                                                  const QString& vertexInhalt = {})
{
    ImportErgebnis r;

    QString rumpf;
    const QString kopfText = trenneKopf(inhalt, rumpf);
    if (kopfText.isEmpty() || !kopfText.startsWith(QLatin1Char('{')))
    {
        r.error = QStringLiteral(
            "Kein ISF-Shader: der JSON-Kopf `/*{ … }*/` am Dateianfang fehlt.");
        return r;
    }
    QJsonParseError perr{};
    const QJsonDocument doc = QJsonDocument::fromJson(kopfText.toUtf8(), &perr);
    if (perr.error != QJsonParseError::NoError || !doc.isObject())
    {
        r.error = QStringLiteral("ISF-Kopf ist kein gültiges JSON: %1")
                      .arg(perr.errorString());
        return r;
    }
    const QJsonObject kopf = doc.object();

    // --- INPUTS einlesen + Kategorie bestimmen -------------------------------
    bool hatInputImage = false;
    bool hatUebergang = false;
    QStringList fremdeBilder;
    for (const QJsonValue& v : kopf.value(QStringLiteral("INPUTS")).toArray())
    {
        if (!v.isObject()) continue;
        const QJsonObject in = v.toObject();
        IsfParam p;
        p.name = in.value(QStringLiteral("NAME")).toString();
        if (p.name.isEmpty()) continue;
        p.typ = detail::typVonText(in.value(QStringLiteral("TYPE")).toString());
        p.label = in.value(QStringLiteral("LABEL")).toString();
        if (p.label.isEmpty()) p.label = p.name;

        if (p.typ == IsfTyp::Image)
        {
            if (p.name == QLatin1String("inputImage"))
            {
                hatInputImage = true;
                continue;  // DAS ist unser `src` — kein Regler
            }
            if (p.name == QLatin1String("startImage") ||
                p.name == QLatin1String("endImage"))
                hatUebergang = true;
            fremdeBilder << p.name;
            continue;
        }
        if (p.name == QLatin1String("progress")) hatUebergang = true;

        const QJsonValue def = in.value(QStringLiteral("DEFAULT"));
        const QJsonValue mn = in.value(QStringLiteral("MIN"));
        const QJsonValue mx = in.value(QStringLiteral("MAX"));
        if (mn.isDouble() && mx.isDouble())
        {
            p.min = mn.toDouble();
            p.max = mx.toDouble();
            p.hatBereich = true;
        }
        if (def.isArray())
        {
            const QJsonArray a = def.toArray();
            for (int i = 0; i < 4 && i < a.size(); ++i)
                p.vektor[static_cast<std::size_t>(i)] = a.at(i).toDouble();
        }
        else if (def.isBool())
        {
            p.zahl = def.toBool() ? 1.0 : 0.0;
        }
        else
        {
            p.zahl = def.toDouble();
        }
        for (const QJsonValue& l : in.value(QStringLiteral("LABELS")).toArray())
            p.auswahlLabels << l.toString();
        for (const QJsonValue& w : in.value(QStringLiteral("VALUES")).toArray())
            p.auswahlWerte << w.toInt();

        if (p.typ == IsfTyp::Audio || p.typ == IsfTyp::AudioFft)
        {
            r.report << QStringLiteral(
                            "ℹ Audio-Input „%1\" nicht übernommen — der "
                            "Filter-Knoten hat die Audio-Werte bass/mid/treb/"
                            "vol/beat, aber keine Audio-Textur.")
                            .arg(p.name);
            continue;
        }
        if (p.typ == IsfTyp::Unbekannt)
        {
            r.report << QStringLiteral("ℹ Unbekannter INPUT-Typ bei „%1\" — "
                                       "übersprungen.")
                            .arg(p.name);
            continue;
        }
        r.parameter << p;
    }

    // --- Kategorie: nur FX passt auf den pixelFilter-Vertrag ------------------
    if (!hatInputImage)
    {
        r.error =
            hatUebergang
                ? QStringLiteral(
                      "Das ist ein ISF-ÜBERGANG (startImage/endImage/progress) "
                      "— er blendet zwischen zwei Bildern. Der Filter-Knoten "
                      "hat nur ein Quellbild.")
                : QStringLiteral(
                      "Das ist ein ISF-GENERATOR — er erzeugt ein Bild aus dem "
                      "Nichts und hat kein Quellbild (`inputImage`). Der "
                      "Filter-Knoten braucht eines.");
        return r;
    }

    // --- Herkunft (Lizenz-Pflicht S72) ---------------------------------------
    r.herkunft.name = dateiName.trimmed().toStdString();
    r.herkunft.author =
        kopf.value(QStringLiteral("CREDIT")).toString().trimmed().toStdString();
    r.herkunft.license =
        kopf.value(QStringLiteral("LICENSE")).toString().trimmed().toStdString();
    if (r.herkunft.license.empty())
    {
        r.report << QStringLiteral(
            "ℹ Die Datei nennt keine Lizenz. Vor dem Weitergeben des Presets "
            "die Lizenz der Quelle nachtragen.");
    }

    // --- Grenzen melden ------------------------------------------------------
    if (kopf.contains(QStringLiteral("PASSES")) &&
        kopf.value(QStringLiteral("PASSES")).toArray().size() > 1)
    {
        r.report << QStringLiteral(
            "ℹ Multipass-Shader (PASSES): nur der letzte Durchgang wird "
            "übersetzt. Mehrpass-Looks laufen über den Shadertoy-Knoten.");
    }
    if (kopf.contains(QStringLiteral("IMPORTED")))
    {
        r.report << QStringLiteral(
            "ℹ IMPORTED-Texturen werden nicht geladen — die betroffenen "
            "Abtastungen liefern Schwarz.");
    }
    for (const QString& b : fremdeBilder)
    {
        r.report << QStringLiteral(
                        "ℹ Zusätzlicher Bild-Input „%1\" bleibt schwarz — der "
                        "Filter-Knoten kennt nur das Kettenbild.")
                        .arg(b);
    }

    // --- Code uebersetzen ----------------------------------------------------
    QString code = rumpf;

    // Der `.vs`-Begleiter (s. @param): seine Varyings werden zu LOKALEN
    // Variablen in `farbe()`, seine main()-Rechnung laeuft dort als erstes.
    QString vsRumpf;
    QList<IsfVarying> varyings;
    if (!vertexInhalt.isEmpty())
    {
        QString vsCode;
        trenneKopf(vertexInhalt, vsCode);  // .vs traegt meist gar keinen Kopf
        varyings = detail::sammleVaryings(vsCode);
        int vSig = 0;
        int vAuf = 0;
        int vZu = 0;
        if (detail::findeMain(vsCode, vSig, vAuf, vZu))
        {
            vsRumpf = vsCode.mid(vAuf + 1, vZu - vAuf - 1);
            // GEOMETRIE-RIEGEL (Befund S72, Frage Patrik): 2 der 38
            // Vidvox-`.vs` bewegen echte Geometrie (`3d Rotate`,
            // `Vertex Manipulator`). Das laesst sich NICHT in den
            // Fragment-Shader falten — `gl_Position` gibt es dort gar nicht.
            // Ohne diesen Riegel wanderte die Zeile still hinein und ergaebe
            // einen Compilerfehler tief im Treiber. Kommentare vorher weg:
            // zwei weitere Dateien tragen ein auskommentiertes
            // `gl_Position = ftransform();`, das nichts tut.
            if (detail::ohneKommentare(vsRumpf)
                    .contains(QStringLiteral("gl_Position")))
            {
                r.error = QStringLiteral(
                    "Dieser Shader verformt GEOMETRIE im Vertex-Shader "
                    "(`gl_Position`) — der Filter-Knoten färbt nur Pixel um. "
                    "Für Verformungen ist der Mesh-Warp-Knoten zuständig.");
                return r;
            }
            // `isf_vertShaderInit()` setzt im Original gl_Position und die
            // ISF-Koordinaten — beides erledigt bei uns der geteilte
            // Quad-Vertex-Shader des Hosts.
            vsRumpf.remove(QRegularExpression(
                QStringLiteral("\\bisf_vertShaderInit\\s*\\(\\s*\\)\\s*;")));
        }
        else
        {
            r.report << QStringLiteral(
                "ℹ Der .vs-Begleiter hat kein `void main()` — seine Varyings "
                "bleiben auf 0.");
        }
    }
    // Die Deklarationen der uebernommenen Groessen fliegen aus dem
    // Fragment-Shader (dort waeren es Eingaenge einer Stufe, die es bei uns
    // nicht gibt) — sie werden gleich als lokale Variablen neu gesetzt.
    for (const IsfVarying& v : varyings)
    {
        code.remove(detail::varyingRegex(QStringLiteral("in|varying"), v.name));
    }
    // Gegenprobe: was jetzt NOCH als Eingang dasteht, kann niemand fuellen.
    // Lieber ein klarer Abbruch als ein Linkerfehler tief im Treiber.
    const auto uebrig =
        detail::varyingRegex(QStringLiteral("in|varying")).match(code);
    if (uebrig.hasMatch())
    {
        r.error =
            QStringLiteral(
                "Der Shader erwartet die Größe „%1\" vom Vertex-Shader. Lade "
                "die gleichnamige `.vs`-Datei mit — ohne sie fehlt die "
                "Berechnung.")
                .arg(uebrig.captured(2));
        return r;
    }

    // Dieselbe Uebersetzung gilt fuer BEIDE Teile — der `.vs`-Rumpf rechnet
    // mit denselben ISF-Groessen (`isf_FragNormCoord`, `RENDERSIZE`) und
    // landet gleich im selben Funktionskoerper.
    const auto uebersetzeBegriffe = [&fremdeBilder](QString& s) {
        // Fremde Bilder ZUERST: ihre Abtastungen werden zu Schwarz, damit die
        // allgemeine IMG_*-Ersetzung darunter sie nicht auf `uTex` umbiegt —
        // das waere das KETTENBILD, der Shader saehe also ein falsches Bild
        // statt gar keines.
        for (const QString& b : fremdeBilder)
        {
            for (const char* makro : {"IMG_THIS_PIXEL", "IMG_THIS_NORM_PIXEL",
                                      "IMG_NORM_PIXEL", "IMG_PIXEL"})
            {
                detail::ersetzeMakro(
                    s, QString::fromLatin1(makro),
                    [&b](const QStringList& a) -> QString {
                        return (!a.isEmpty() && a.first() == b)
                                   ? QStringLiteral("vec4(0.0)")
                                   : QString();  // null = nicht meine Baustelle
                    });
            }
            detail::ersetzeMakro(s, QStringLiteral("IMG_SIZE"),
                                 [&b](const QStringList& a) -> QString {
                                     return (!a.isEmpty() && a.first() == b)
                                                ? QStringLiteral("uResolution")
                                                : QString();
                                 });
        }

        detail::ersetzeMakro(s, QStringLiteral("IMG_THIS_PIXEL"),
                             [](const QStringList&) { return QStringLiteral("src"); });
        detail::ersetzeMakro(s, QStringLiteral("IMG_THIS_NORM_PIXEL"),
                             [](const QStringList&) { return QStringLiteral("src"); });
        detail::ersetzeMakro(
            s, QStringLiteral("IMG_NORM_PIXEL"), [](const QStringList& a) {
                return a.size() >= 2
                           ? QStringLiteral("texture(uTex, %1)").arg(a.at(1))
                           : QStringLiteral("src");
            });
        detail::ersetzeMakro(
            s, QStringLiteral("IMG_PIXEL"), [](const QStringList& a) {
                // IMG_PIXEL rechnet in PIXELN, texture() in 0..1.
                return a.size() >= 2
                           ? QStringLiteral("texture(uTex, (%1) / uResolution)")
                                 .arg(a.at(1))
                           : QStringLiteral("src");
            });
        detail::ersetzeMakro(s, QStringLiteral("IMG_SIZE"), [](const QStringList&) {
            return QStringLiteral("uResolution");
        });

        detail::ersetzeWort(s, QStringLiteral("isf_FragNormCoord"),
                            QStringLiteral("uv"));
        detail::ersetzeWort(s, QStringLiteral("vv_FragNormCoord"),
                            QStringLiteral("uv"));
        detail::ersetzeWort(s, QStringLiteral("RENDERSIZE"),
                            QStringLiteral("uResolution"));
        detail::ersetzeWort(s, QStringLiteral("TIME"), QStringLiteral("uTime"));
        detail::ersetzeWort(s, QStringLiteral("TIMEDELTA"), QStringLiteral("uDelta"));
        detail::ersetzeWort(s, QStringLiteral("FRAMEINDEX"),
                            QStringLiteral("uFrame"));
    };
    uebersetzeBegriffe(code);
    uebersetzeBegriffe(vsRumpf);

    if (code.contains(QStringLiteral("isf_FragCoord")) ||
        code.contains(QRegularExpression(QStringLiteral("\\bDATE\\b"))))
    {
        r.report << QStringLiteral(
            "ℹ `DATE`/`isf_FragCoord` gibt es hier nicht — die Stellen bitte "
            "im Code prüfen.");
    }

    // --- main() -> farbe() ---------------------------------------------------
    int sigStart = 0;
    int auf = 0;
    int zu = 0;
    if (!detail::findeMain(code, sigStart, auf, zu))
    {
        r.error = QStringLiteral(
            "Kein `void main()` gefunden — die Datei ist kein vollständiger "
            "ISF-Shader.");
        return r;
    }
    QString koerper = code.mid(auf + 1, zu - auf - 1);
    if (!koerper.contains(QStringLiteral("gl_FragColor")))
    {
        r.report << QStringLiteral(
            "ℹ `main()` schreibt kein `gl_FragColor` — der Filter gibt das "
            "Quellbild unverändert weiter.");
    }
    // gl_FragColor wird eine LOKALE Variable, nicht direkt der Rueckgabewert:
    // sonst zerbricht jedes vorzeitige `return;` (in einer vec4-Funktion ein
    // Compilerfehler), und genau so schreiben viele ISF-Filter ihren
    // Passthrough-Zweig.
    detail::ersetzeWort(koerper, QStringLiteral("gl_FragColor"),
                        QStringLiteral("_lumi_frag"));
    static const QRegularExpression kNacktesReturn(
        QStringLiteral("\\breturn\\s*;"));
    koerper.replace(kNacktesReturn, QStringLiteral("return _lumi_frag;"));

    const QString vorMain = code.left(sigStart);
    const QString nachMain = code.mid(zu + 1);

    QString out;
    if (!r.parameter.isEmpty())
    {
        out += QString::fromLatin1(kParamBlockStart) + QLatin1Char('\n');
        for (const IsfParam& p : r.parameter)
        {
            switch (p.typ)
            {
                case IsfTyp::Bool:
                case IsfTyp::Event:
                    out += QStringLiteral("const bool %1 = %2;\n")
                               .arg(p.name, p.zahl != 0.0 ? QStringLiteral("true")
                                                          : QStringLiteral("false"));
                    break;
                case IsfTyp::Long:
                    out += QStringLiteral("const int %1 = %2;\n")
                               .arg(p.name)
                               .arg(static_cast<int>(p.zahl));
                    break;
                case IsfTyp::Point2D:
                    out += QStringLiteral("const vec2 %1 = vec2(%2, %3);\n")
                               .arg(p.name, detail::glslFloat(p.vektor[0]),
                                    detail::glslFloat(p.vektor[1]));
                    break;
                case IsfTyp::Color:
                    out += QStringLiteral("const vec4 %1 = vec4(%2, %3, %4, %5);\n")
                               .arg(p.name, detail::glslFloat(p.vektor[0]),
                                    detail::glslFloat(p.vektor[1]),
                                    detail::glslFloat(p.vektor[2]),
                                    detail::glslFloat(p.vektor[3]));
                    break;
                default:
                    out += QStringLiteral("const float %1 = %2;\n")
                               .arg(p.name, detail::glslFloat(p.zahl));
                    break;
            }
        }
        out += QString::fromLatin1(kParamBlockEnde) + QStringLiteral("\n\n");
    }
    out += vorMain.trimmed();
    if (!out.endsWith(QLatin1Char('\n'))) out += QLatin1Char('\n');
    out += QStringLiteral("\nvec4 farbe(vec2 uv, vec4 src)\n{\n"
                          "    vec4 _lumi_frag = src;\n");
    if (!varyings.isEmpty())
    {
        out += QStringLiteral("    // aus dem .vs-Begleiter — dort je Vertex, "
                              "hier je Pixel gerechnet\n");
        for (const IsfVarying& v : varyings)
            out += QStringLiteral("    %1 %2;\n").arg(v.typ, v.name);
        out += vsRumpf;
        out += QLatin1Char('\n');
    }
    out += koerper;
    out += QStringLiteral("\n    return _lumi_frag;\n}\n");
    const QString rest = nachMain.trimmed();
    if (!rest.isEmpty()) out += QLatin1Char('\n') + rest + QLatin1Char('\n');

    r.code = out;
    r.ok = true;
    return r;
}

/**
 * @brief Die ISF-Regler in die generische Ablage überführen (Stufe 2)
 *
 * `IsfParam` ist die Sicht des Importeurs, `ParamGruppe` die des Knotens und
 * des Parameter-Baums. Bewusst zwei Typen: die Ablage ist NICHT ISF-spezifisch
 * (Vorgabe Patrik S71) und trägt später die Parameter jedes Moduls.
 */
[[nodiscard]] inline lumi::multieffect::ParamGruppe alsParamGruppe(
    const QList<IsfParam>& params)
{
    using lumi::multieffect::ParamTyp;
    lumi::multieffect::ParamGruppe g;
    for (const IsfParam& p : params)
    {
        lumi::multieffect::ParamWert w;
        w.key = p.name.toStdString();
        w.label = p.label.toStdString();
        w.min = p.min;
        w.max = p.max;
        w.hatBereich = p.hatBereich;
        switch (p.typ)
        {
            case IsfTyp::Bool:
            case IsfTyp::Event:
                w.typ = ParamTyp::Bool;
                w.ja = p.zahl != 0.0;
                w.hatBereich = false;
                break;
            case IsfTyp::Long:
                // Mit LABELS/VALUES wird daraus ein KLARTEXT-Dropdown, sonst
                // ein gewoehnlicher Ganzzahl-Zaehler.
                w.typ = p.auswahlLabels.isEmpty() ? ParamTyp::Ganzzahl
                                                  : ParamTyp::Auswahl;
                w.zahl = p.zahl;
                for (const QString& l : p.auswahlLabels)
                    w.auswahlLabels.push_back(l.toStdString());
                for (int v : p.auswahlWerte) w.auswahlWerte.push_back(v);
                break;
            case IsfTyp::Point2D:
                w.typ = ParamTyp::Punkt2D;
                w.vektor = p.vektor;
                break;
            case IsfTyp::Color:
                w.typ = ParamTyp::Farbe;
                w.vektor = p.vektor;
                break;
            default:
                w.typ = ParamTyp::Zahl;
                w.zahl = p.zahl;
                break;
        }
        g.werte.push_back(std::move(w));
    }
    return g;
}

/**
 * @brief Den `const`-Block aus den AKTUELLEN Werten erzeugen
 *
 * Die Gegenrichtung zu `alsParamGruppe`: was der Nutzer im Parameter-Baum
 * eingestellt hat, wird wieder GLSL. Nur die Wurzel-Werte — Untergruppen
 * haben im Shader keinen Namensraum.
 */
[[nodiscard]] inline QString erzeugeParamBlock(
    const lumi::multieffect::ParamGruppe& g)
{
    using lumi::multieffect::ParamTyp;
    if (g.werte.empty()) return {};
    QString out = QString::fromLatin1(kParamBlockStart) + QLatin1Char('\n');
    for (const auto& w : g.werte)
    {
        const QString name = QString::fromStdString(w.key);
        switch (w.typ)
        {
            case ParamTyp::Bool:
                out += QStringLiteral("const bool %1 = %2;\n")
                           .arg(name, w.ja ? QStringLiteral("true")
                                           : QStringLiteral("false"));
                break;
            case ParamTyp::Ganzzahl:
            case ParamTyp::Auswahl:
                out += QStringLiteral("const int %1 = %2;\n")
                           .arg(name)
                           .arg(static_cast<int>(w.zahl));
                break;
            case ParamTyp::Punkt2D:
                out += QStringLiteral("const vec2 %1 = vec2(%2, %3);\n")
                           .arg(name, detail::glslFloat(w.vektor[0]),
                                detail::glslFloat(w.vektor[1]));
                break;
            case ParamTyp::Farbe:
                out += QStringLiteral("const vec4 %1 = vec4(%2, %3, %4, %5);\n")
                           .arg(name, detail::glslFloat(w.vektor[0]),
                                detail::glslFloat(w.vektor[1]),
                                detail::glslFloat(w.vektor[2]),
                                detail::glslFloat(w.vektor[3]));
                break;
            case ParamTyp::Text:
                continue;  // GLSL kennt keine Zeichenketten
            default:
                out += QStringLiteral("const float %1 = %2;\n")
                           .arg(name, detail::glslFloat(w.zahl));
                break;
        }
    }
    out += QString::fromLatin1(kParamBlockEnde) + QLatin1Char('\n');
    return out;
}

/**
 * @brief Den erzeugten Block im Code gegen einen neuen tauschen
 *
 * Die Sentinel sind die Nahtstelle: der Nutzer darf ausserhalb frei
 * schreiben, INNERHALB gehoert der Text dem Parameter-Baum. Ohne Block im
 * Code passiert nichts — dann hat der Nutzer ihn geloescht und meint es so.
 */
[[nodiscard]] inline QString ersetzeParamBlock(const QString& code,
                                               const QString& neuerBlock)
{
    const int a = code.indexOf(QString::fromLatin1(kParamBlockStart));
    if (a < 0) return code;
    const int e = code.indexOf(QString::fromLatin1(kParamBlockEnde), a);
    if (e < 0) return code;
    const int ende = e + static_cast<int>(qstrlen(kParamBlockEnde)) + 1;
    QString out = code;
    out.replace(a, ende - a, neuerBlock);
    return out;
}

}  // namespace lumi::isf
