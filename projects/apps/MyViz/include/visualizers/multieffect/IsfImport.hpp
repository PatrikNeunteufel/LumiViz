/**
 ****************************************************************************************
 * @file   IsfImport.hpp
 * @brief  Kopf-Leser für ISF-Dateien (Interactive Shader Format) — die pure
 *         Hälfte des `isfFilter`-Knotens (S72)
 *
 * @author LumiPulse Team
 * @date   August 2026
 * @version 2.0.0
 *
 * @details
 * ISF ist „Shadertoy für Filter" (Vidvox, isf.video): GLSL plus eine
 * JSON-Parameterdeklaration. Der `isfFilter`-Knoten führt solche Shader
 * **unverändert** aus — die ISF-Sprache (`IMG_*`-Makros, `RENDERSIZE`,
 * `TIME`, `isf_FragNormCoord` …) stellt der Wrapper als Makros und Uniforms
 * bereit. Dieses Modul liest deshalb nur den **Kopf** und reicht den Code
 * durch; es übersetzt nichts.
 *
 * > **Entscheid Patrik S72 — und warum die Vorfassung (1.0.0) anders aussah:**
 * > Zuerst sollte ISF auf den bestehenden `pixelFilter` abgebildet werden.
 * > Das erzwang, den Code auf `vec4 farbe(vec2, vec4)` umzuschreiben und den
 * > `.vs`-Begleiter in die Filterfunktion zu FALTEN — und damit Ablehnungen
 * > für alles, was nicht Bild-rein/Bild-raus ist. Mit einem eigenen Knoten,
 * > der beide Shader-Stufen hat, entfällt das ersatzlos: **nichts wird mehr
 * > abgelehnt und nichts mehr umgeschrieben.**
 *
 * **Format (gegen die Spec und den Korpus geprüft):** Das JSON steckt als
 * Blockkommentar `/*{ … }* /` am DATEIANFANG, es gibt KEINE Nachbardatei für
 * die Deklaration; Endung `.fs`, optional ein gleichnamiger `.vs`. Im ganzen
 * Vidvox-Korpus kommen nur diese beiden Shader-Endungen vor.
 *
 * **Es gibt keine Kategorien, nur unterschiedlich viele Bildquellen:**
 * | Bild-Eingänge | Sorte | typische Namen |
 * |---|---|---|
 * | 0 | Generator | — |
 * | 1 | Filter | `inputImage` |
 * | 2 | Übergang | `startImage`, `endImage` |
 *
 * Jede dieser Sorten darf zusätzlich einen `.vs` haben — `Rotate` ist ein
 * Übergang mit, `Life` ein Generator mit Vertex-Shader. Die Namen sind für
 * uns bloß Sampler-Bezeichner; welches Bild daran hängt, entscheidet die
 * Quellen-Liste im Knoten.
 *
 * Vertragsgrenze wie `PixelFilterWrapper`: Text → Text, kein GL, keine GUI —
 * voll unit-testbar.
 ****************************************************************************************
 */

#pragma once

#include "visualizers/multieffect/EffectChain.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>
#include <QStringList>

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace lumi::isf {

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

/// Ein deklarierter Regler des Shaders (`INPUTS[i]`).
struct IsfParam
{
    QString name;   ///< GLSL-Bezeichner (`NAME`) — so heisst das Uniform
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
    QString error;  ///< harter Fehler (kein ISF-Kopf, kaputtes JSON)

    QString fragCode;    ///< die `.fs` OHNE den JSON-Kopf, sonst unveraendert
    QString vertexCode;  ///< die `.vs`, unveraendert (leer = keine vorhanden)

    /// Bild-Eingaenge in DEKLARATIONS-Reihenfolge. Die Anzahl bestimmt die
    /// Sorte (s. `sortenName`), die Namen sind die Sampler im Shader.
    QStringList bildInputs;
    /// `audio`-Eingaenge: die **Waveform** (Zeitsignal). In ISF sind das
    /// GEWOEHNLICHE SAMPLER, die per `IMG_NORM_PIXEL` abgetastet werden — sie
    /// gehoeren deshalb in die Quellen-Liste, nicht in die Regler (Befund S72
    /// aus dem GL-Smoke: ohne ihre Sampler-Deklaration bricht der Shader mit
    /// „undeclared identifier" ab). Sie zaehlen NICHT zur Sorte: ein Filter
    /// mit Audio-Eingang ist ein Filter, kein Uebergang.
    QStringList audioWaveInputs;
    /// `audioFFT`-Eingaenge: das **Spektrum**. Getrennt gefuehrt, weil beide
    /// Sorten verschiedene Texturen bekommen — sie zusammenzuwerfen war der
    /// urspruengliche Fehler (s. `isffilter::kQuelleAudioWave`).
    QStringList audioFftInputs;

    /// Durchgaenge aus `PASSES` (leer = ein Durchgang). Ihre `TARGET`-Namen
    /// sind im Shader gewoehnliche Sampler.
    std::vector<lumi::multieffect::IsfPassZiel> passes;

    QList<IsfParam> parameter;
    lumi::multieffect::Herkunft herkunft;
    QStringList report;  ///< Grenzen/Hinweise, "ℹ "-Praefix (Import-Report-Stil)
};

/// Klartext der Sorte fuer die Info-Zeile im Panel (Wunsch Patrik S72).
/// Leitet sich AUS DER ANZAHL der Bildquellen ab — nicht aus ihrer Bindung:
/// eine Quelle auf „schwarz" ist ein Filter, der Schwarz filtert; KEINE
/// Quelle ist ein Generator.
[[nodiscard]] inline QString sortenName(int quellen)
{
    if (quellen <= 0) return QStringLiteral("Generator (erzeugt ein Bild)");
    if (quellen == 1) return QStringLiteral("Filter (ein Quellbild)");
    if (quellen == 2) return QStringLiteral("Übergang (zwei Quellbilder)");
    return QStringLiteral("Mehrfach-Quelle (%1 Bilder)").arg(quellen);
}

namespace detail {

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

/**
 * Winziger Rechner fuer die `WIDTH`/`HEIGHT`-Ausdruecke der PASSES.
 *
 * ISF erlaubt dort Ausdruecke ueber `$WIDTH`/`$HEIGHT` und die INPUT-Namen —
 * `"$WIDTH/16"` macht aus einem Pass eine Reduktionsstufe, und genau davon
 * haengt ab, ob ein Histogramm-Shader ueberhaupt funktioniert. Unterstuetzt
 * werden Zahlen, `$name`, die vier Grundrechenarten mit Vorrang, Klammern und
 * `floor`/`ceil`/`min`/`max`.
 *
 * Bewusst KEIN vollstaendiger Ausdrucks-Parser: was hier nicht aufgeht,
 * faellt auf die volle Bildgroesse zurueck — ein zu grosser Puffer rechnet
 * langsamer, aber richtig, waehrend ein geratener falscher stumm falsche
 * Bilder liefert.
 */
class MaszRechner
{
public:
    MaszRechner(const QString& text, double breite, double hoehe,
                const QList<IsfParam>& params)
        : m_t(text), m_w(breite), m_h(hoehe), m_p(params)
    {
    }

    /// Wert des Ausdrucks; `ok` bleibt false, wenn etwas nicht aufging.
    [[nodiscard]] double rechne(bool& ok)
    {
        m_i = 0;
        const double v = ausdruck();
        ueberspringe();
        ok = m_ok && m_i >= m_t.size();
        return v;
    }

private:
    void ueberspringe()
    {
        while (m_i < m_t.size() && m_t.at(m_i).isSpace()) ++m_i;
    }
    [[nodiscard]] bool schluck(QChar c)
    {
        ueberspringe();
        if (m_i < m_t.size() && m_t.at(m_i) == c)
        {
            ++m_i;
            return true;
        }
        return false;
    }
    double ausdruck()
    {
        double v = term();
        for (;;)
        {
            if (schluck(QLatin1Char('+'))) v += term();
            else if (schluck(QLatin1Char('-'))) v -= term();
            else return v;
        }
    }
    double term()
    {
        double v = faktor();
        for (;;)
        {
            if (schluck(QLatin1Char('*'))) v *= faktor();
            else if (schluck(QLatin1Char('/')))
            {
                const double d = faktor();
                // Teilen durch 0 waere ein Puffer der Groesse unendlich —
                // lieber als Fehler melden und auf Vollbild zurueckfallen.
                if (d == 0.0) { m_ok = false; return v; }
                v /= d;
            }
            else return v;
        }
    }
    double faktor()
    {
        ueberspringe();
        if (schluck(QLatin1Char('(')))
        {
            const double v = ausdruck();
            if (!schluck(QLatin1Char(')'))) m_ok = false;
            return v;
        }
        if (schluck(QLatin1Char('-'))) return -faktor();
        if (m_i < m_t.size() && m_t.at(m_i) == QLatin1Char('$'))
        {
            ++m_i;
            return groesse(bezeichner());
        }
        if (m_i < m_t.size() && (m_t.at(m_i).isDigit() || m_t.at(m_i) == QLatin1Char('.')))
        {
            const int a = m_i;
            while (m_i < m_t.size() &&
                   (m_t.at(m_i).isDigit() || m_t.at(m_i) == QLatin1Char('.')))
                ++m_i;
            bool zahlOk = false;
            const double v = m_t.mid(a, m_i - a).toDouble(&zahlOk);
            if (!zahlOk) m_ok = false;
            return v;
        }
        const QString name = bezeichner();
        if (name.isEmpty()) { m_ok = false; return 0.0; }
        // Funktionsaufruf?
        if (schluck(QLatin1Char('(')))
        {
            const double a = ausdruck();
            double b = 0.0;
            const bool zwei = schluck(QLatin1Char(','));
            if (zwei) b = ausdruck();
            if (!schluck(QLatin1Char(')'))) m_ok = false;
            if (name == QLatin1String("floor")) return std::floor(a);
            if (name == QLatin1String("ceil")) return std::ceil(a);
            if (name == QLatin1String("min")) return std::min(a, b);
            if (name == QLatin1String("max")) return std::max(a, b);
            m_ok = false;
            return a;
        }
        return groesse(name);
    }
    [[nodiscard]] QString bezeichner()
    {
        ueberspringe();
        const int a = m_i;
        while (m_i < m_t.size() &&
               (m_t.at(m_i).isLetterOrNumber() || m_t.at(m_i) == QLatin1Char('_')))
            ++m_i;
        return m_t.mid(a, m_i - a);
    }
    /// `$WIDTH`/`$HEIGHT` sowie die Vorgabewerte der INPUTS — mehr kann ein
    /// Groessen-Ausdruck sinnvoll brauchen.
    [[nodiscard]] double groesse(const QString& name)
    {
        if (name.compare(QLatin1String("WIDTH"), Qt::CaseInsensitive) == 0)
            return m_w;
        if (name.compare(QLatin1String("HEIGHT"), Qt::CaseInsensitive) == 0)
            return m_h;
        for (const IsfParam& p : m_p)
        {
            if (p.name == name) return p.zahl;
        }
        m_ok = false;
        return 0.0;
    }

    QString m_t;
    int m_i = 0;
    double m_w = 0.0;
    double m_h = 0.0;
    QList<IsfParam> m_p;
    bool m_ok = true;
};

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
 * Streng: der Kopf muss da sein UND ein gueltiges JSON-Objekt enthalten. Der
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
 * @brief Eine ISF-Datei einlesen — Kopf auswerten, Code durchreichen
 * @param inhalt       kompletter Dateiinhalt der `.fs`
 * @param dateiName    Dateiname ohne Endung — ISF kennt kein Titelfeld, der
 *                     Dateiname IST der Titel (Konvention der Bibliothek)
 * @param vertexInhalt Inhalt der gleichnamigen `.vs`, falls vorhanden. Sie
 *                     wird NICHT eingefaltet, sondern als eigener
 *                     Vertex-Shader weitergereicht — der `isfFilter`-Knoten
 *                     hat dafuer ein eigenes Feld.
 *
 * Abgelehnt wird nur, was gar keine ISF-Datei ist. Sorte, Geometrie und Zahl
 * der Bildquellen sind Eigenschaften, keine Ausschlussgruende.
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

    // --- INPUTS einlesen -----------------------------------------------------
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
            // Bild-Eingaenge sind keine Regler, sondern Quellen. Reihenfolge
            // bleibt erhalten — sie ist die einzige Zuordnung, die der
            // Nutzer im Panel wiedererkennt.
            r.bildInputs << p.name;
            continue;
        }

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

        if (p.typ == IsfTyp::Audio)
        {
            r.audioWaveInputs << p.name;  // Sampler, kein Regler
            continue;
        }
        if (p.typ == IsfTyp::AudioFft)
        {
            r.audioFftInputs << p.name;
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

    // --- Herkunft (Lizenz-Pflicht S72) ---------------------------------------
    r.herkunft.name = dateiName.trimmed().toStdString();
    // CREDIT steht in der Bibliothek meist als „by <Name>". Das Panel setzt
    // selbst ein „von" davor — ohne diesen Schnitt laese sich „von by Carter
    // Rosenberg" (Sichttest Patrik S72).
    QString credit = kopf.value(QStringLiteral("CREDIT")).toString().trimmed();
    if (credit.startsWith(QStringLiteral("by "), Qt::CaseInsensitive))
        credit = credit.mid(3).trimmed();
    r.herkunft.author = credit.toStdString();
    r.herkunft.license =
        kopf.value(QStringLiteral("LICENSE")).toString().trimmed().toStdString();
    // KEINE Report-Zeile fuer eine fehlende Lizenz (Entscheid Patrik S72:
    // "message wegen Lizenz kann in einem Lizenz-Feld angezeigt werden") —
    // das Herkunfts-Feld im Panel zeigt sie ohnehin als „Lizenz: unbekannt".
    // Eine zweite Meldung im Dialog waere nur Laerm.

    // --- PASSES: die Durchgaenge eines Multipass-Shaders ---------------------
    for (const QJsonValue& v : kopf.value(QStringLiteral("PASSES")).toArray())
    {
        if (!v.isObject()) continue;
        const QJsonObject pj = v.toObject();
        lumi::multieffect::IsfPassZiel pz;
        pz.target = pj.value(QStringLiteral("TARGET")).toString().toStdString();
        // WIDTH/HEIGHT sind AUSDRUECKE, keine Zahlen — die Reduktionsstufen
        // eines Histogramms haengen genau daran (`$WIDTH/16`). Zahl wie Text
        // annehmen, damit `"WIDTH": 16` nicht durchfaellt.
        const auto masz = [&pj](const char* schluessel) {
            const QJsonValue w = pj.value(QLatin1String(schluessel));
            if (w.isString()) return w.toString().trimmed().toStdString();
            if (w.isDouble())
                return QString::number(w.toDouble(), 'g', 9).toStdString();
            return std::string();
        };
        pz.breite = masz("WIDTH");
        pz.hoehe = masz("HEIGHT");
        pz.gleitkomma = pj.value(QStringLiteral("FLOAT")).toBool();
        pz.bestaendig = pj.value(QStringLiteral("PERSISTENT")).toBool();
        r.passes.push_back(std::move(pz));
    }
    if (kopf.contains(QStringLiteral("IMPORTED")))
    {
        r.report << QStringLiteral(
            "ℹ IMPORTED-Texturen werden nicht geladen — die betroffenen "
            "Abtastungen liefern Schwarz.");
    }
    // Ebenso KEINE Zeile fuer die Sorte — die steht als Info-Zeile im Panel
    // (`sortenName`) und aendert sich dort mit, wenn Quellen dazukommen.
    // Der Report bleibt damit dem vorbehalten, was der Nutzer SONST nicht
    // sieht: Multipass, IMPORTED-Texturen, unbekannte INPUT-Typen. Hat eine
    // Datei nichts davon, erscheint gar kein Dialog.

    // --- Code: unveraendert durchreichen -------------------------------------
    // Kein Umschreiben, kein Falten: die ISF-Sprache liefert der Wrapper.
    r.fragCode = rumpf.trimmed();
    // Auch der `.vs` kann einen Kopf tragen (selten, aber die Spec erlaubt es).
    if (!vertexInhalt.isEmpty())
    {
        QString vsRumpf;
        trenneKopf(vertexInhalt, vsRumpf);
        r.vertexCode = vsRumpf.trimmed();
    }
    r.ok = true;
    return r;
}

/**
 * @brief Die ISF-Regler in die generische Ablage überführen
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
 * @brief Bild-Eingänge in Quellen-Zeilen des Knotens überführen
 *
 * Vorgabe je Zeile ist das **Ketten-Bild** — AVS-Konvention: nichts
 * ausgewählt heißt normale Pipeline (Entscheid Patrik S72).
 */
[[nodiscard]] inline std::vector<lumi::multieffect::IsfBildQuelle> alsBildQuellen(
    const QStringList& bild, const QStringList& audioWave = {},
    const QStringList& audioFft = {})
{
    namespace ic = lumi::multieffect::isffilter;
    std::vector<lumi::multieffect::IsfBildQuelle> out;
    const auto anhaengen = [&out](const QStringList& namen, int bindung) {
        for (const QString& n : namen)
        {
            lumi::multieffect::IsfBildQuelle q;
            q.name = n.toStdString();
            q.bindung = bindung;
            out.push_back(std::move(q));
        }
    };
    anhaengen(bild, ic::kQuelleKette);
    // Audio-Eingaenge sind ebenfalls Sampler — dieselbe Liste, aber je nach
    // ISF-Typ die Waveform- oder die Spektrum-Textur.
    anhaengen(audioWave, ic::kQuelleAudioWave);
    anhaengen(audioFft, ic::kQuelleAudioFft);
    return out;
}

/**
 * @brief Größe eines Pass-Ziels in Pixeln
 * @param ausdruck  `WIDTH`/`HEIGHT` der Pass-Deklaration (leer = Vollbild)
 * @param vollbild  die Kantenlänge des Kettenbilds in dieser Richtung
 *
 * Geht der Ausdruck nicht auf, kommt die **volle Bildgröße** zurück: ein zu
 * großer Puffer rechnet langsamer, aber richtig — ein geratener falscher
 * lieferte stumm falsche Bilder.
 */
[[nodiscard]] inline int passGroesse(const std::string& ausdruck, int vollbild,
                                     int breite, int hoehe,
                                     const QList<IsfParam>& params = {})
{
    if (ausdruck.empty()) return vollbild;
    detail::MaszRechner rechner(QString::fromStdString(ausdruck), breite, hoehe,
                                params);
    bool ok = false;
    const double v = rechner.rechne(ok);
    if (!ok || !(v >= 1.0)) return vollbild;
    // Nach oben klemmen: ein Ausdruck wie `$WIDTH*100` wuerde sonst den
    // Grafikspeicher sprengen.
    return static_cast<int>(std::min(v, 8192.0));
}

}  // namespace lumi::isf
