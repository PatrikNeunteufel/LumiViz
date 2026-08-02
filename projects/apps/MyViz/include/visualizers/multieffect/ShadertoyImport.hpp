/**
 ****************************************************************************************
 * @file   ShadertoyImport.hpp
 * @brief  URL-/ID-Import über die offizielle Shadertoy-API (Strang S, S3)
 *
 * @author LumiPulse Team
 * @date   August 2026
 * @version 1.0.0
 *
 * @details
 * Die netz- und GL-freie Hälfte des Imports (Plan §S3): ID aus URL oder
 * Roh-Eingabe ziehen, API-Request-URL bauen, JSON-Antwort in ShadertoyParams
 * übersetzen — voll unit-testbar; das eigentliche GET macht das Panel
 * (Qt Network, NUR auf Knopfdruck, kein Auto-Fetch).
 *
 * API: `https://www.shadertoy.com/api/v1/shaders/<ID>?key=<AppKey>` liefert
 * `{Shader:{info:{name,username,...}, renderpass:[{type,code,inputs,...}]}}`
 * bzw. `{Error:"..."}`. Die API sieht nur Shader mit Sichtbarkeit
 * „public+api" — Fallback bleibt Code-Einfügen (Grenze im Report).
 *
 * Stufe-1-Grenzen (Report, kein Hard-Fail): nur der image-Pass wird
 * übernommen (ein `common`-Pass wird vorangestellt — er ist per Definition
 * Bestandteil aller Pässe); Buffer-A–D/Sound = S4; Textur-/Video-/Cubemap-/
 * Keyboard-Inputs ⇒ Platzhalter-Meldung (iChannel bleibt schwarz). Ein
 * music-/musicstream-Input wird auf unseren Audio-iChannel gemappt.
 *
 * Lizenz-Vorbehalt (Entscheid Plan §S3): Shadertoy-Default ist CC BY-NC-SA —
 * die Metadaten (Name/Autor/URL/Lizenz) reisen im Node mit, Inhalte bleiben
 * lokal (VisualsPresets extern), nichts davon ins Repo.
 ****************************************************************************************
 */

#pragma once

#include "visualizers/multieffect/EffectChain.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QUrlQuery>

#include <algorithm>
#include <array>
#include <vector>

namespace lumi::shadertoy {

/**
 * @brief Shader-ID aus einer Shadertoy-URL oder Roh-Eingabe ziehen
 *
 * Akzeptiert `https://www.shadertoy.com/view/<ID>` (mit/ohne Schema/www,
 * Query/Fragment egal) oder die blanke ID (alphanumerisch, ≥5 Zeichen).
 * Leerstring = nicht erkennbar.
 */
[[nodiscard]] inline QString extractShaderId(const QString& urlOrId)
{
    const QString s = urlOrId.trimmed();
    static const QRegularExpression kView(
        QStringLiteral("shadertoy\\.com/view/([A-Za-z0-9]+)"));
    if (const auto m = kView.match(s); m.hasMatch()) return m.captured(1);
    static const QRegularExpression kBare(QStringLiteral("^[A-Za-z0-9]{5,12}$"));
    if (kBare.match(s).hasMatch()) return s;
    return {};
}

/// API-Request-URL (der Key kommt aus den lokalen Settings — NIE ins Preset)
[[nodiscard]] inline QUrl apiRequestUrl(const QString& shaderId, const QString& apiKey)
{
    QUrl url(QStringLiteral("https://www.shadertoy.com/api/v1/shaders/") + shaderId);
    QUrlQuery q;
    q.addQueryItem(QStringLiteral("key"), apiKey);
    url.setQuery(q);
    return url;
}

/**
 * @brief Query-URL des Browsers (Browser-Panel, S3): Suche + Sortierung
 *
 * `sort` ∈ popular|newest|hot|love (API-Vertrag); leerer Suchtext listet
 * die Top-Ergebnisse der Sortierung. Auch hier gilt: nur „public+api".
 */
[[nodiscard]] inline QUrl queryRequestUrl(const QString& searchText, const QString& sort,
                                          const QString& apiKey, int maxResults = 24)
{
    QString path = QStringLiteral("https://www.shadertoy.com/api/v1/shaders/query");
    const QString text = searchText.trimmed();
    if (!text.isEmpty())
        path += QLatin1Char('/') + QString::fromLatin1(QUrl::toPercentEncoding(text));
    QUrl url(path);
    QUrlQuery q;
    q.addQueryItem(QStringLiteral("sort"), sort);
    q.addQueryItem(QStringLiteral("from"), QStringLiteral("0"));
    q.addQueryItem(QStringLiteral("num"), QString::number(maxResults));
    q.addQueryItem(QStringLiteral("key"), apiKey);
    url.setQuery(q);
    return url;
}

/// Offizielles Vorschaubild eines Shaders (kein API-Key nötig)
[[nodiscard]] inline QUrl thumbnailUrl(const QString& shaderId)
{
    return QUrl(QStringLiteral("https://www.shadertoy.com/media/shaders/") + shaderId +
                QStringLiteral(".jpg"));
}

/// Ergebnis der Browser-Suche: Shader-IDs in Sortier-Reihenfolge
struct QueryResult
{
    bool ok = false;
    int total = 0;       ///< Gesamttreffer laut API ("Shaders")
    QStringList ids;     ///< die gelieferte Seite (from/num)
    QString error;
};

/// Antwort der Query-API parsen: `{"Shaders":N,"Results":["id",...]}`
[[nodiscard]] inline QueryResult parseQueryReply(const QByteArray& bytes)
{
    QueryResult r;
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(bytes, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
    {
        r.error = QStringLiteral("Antwort ist kein JSON (%1)").arg(err.errorString());
        return r;
    }
    const QJsonObject root = doc.object();
    if (root.contains(QStringLiteral("Error")))
    {
        r.error = root.value(QStringLiteral("Error")).toString();
        return r;
    }
    r.total = root.value(QStringLiteral("Shaders")).toInt(0);
    for (const QJsonValue& v : root.value(QStringLiteral("Results")).toArray())
    {
        const QString id = v.toString();
        if (!id.isEmpty()) r.ids << id;
    }
    r.ok = true;
    return r;
}

/// Ergebnis des Antwort-Parsers: ok + Params + Grenzen-Report ODER error
struct ImportResult
{
    bool ok = false;
    lumi::multieffect::ShadertoyParams params;
    QStringList report;  ///< Grenzen/Hinweise (Import-Report-Stil, "ℹ "-Präfix)
    QString error;       ///< harter Fehler (API-Error, kaputtes JSON, kein image-Pass)
};

/**
 * @brief JSON-Antwort der API in ShadertoyParams übersetzen (netz-/GL-frei)
 */
[[nodiscard]] inline ImportResult parseApiReply(const QByteArray& bytes,
                                                const QString& shaderId)
{
    ImportResult r;
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(bytes, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
    {
        r.error = QStringLiteral("Antwort ist kein JSON (%1)").arg(err.errorString());
        return r;
    }
    const QJsonObject root = doc.object();
    if (root.contains(QStringLiteral("Error")))
    {
        // typisch: "Shader not found" — auch bei Sichtbarkeit ohne "+api"
        r.error = QStringLiteral("API: %1 (Shader existiert nicht oder ist nicht "
                                 "'public+api' — dann bleibt Code-Einfügen)")
                      .arg(root.value(QStringLiteral("Error")).toString());
        return r;
    }
    const QJsonObject shader = root.value(QStringLiteral("Shader")).toObject();
    const QJsonObject info = shader.value(QStringLiteral("info")).toObject();
    const QJsonArray passes = shader.value(QStringLiteral("renderpass")).toArray();
    if (passes.isEmpty())
    {
        r.error = QStringLiteral("Antwort ohne renderpass-Liste");
        return r;
    }

    QString imageCode;
    QString commonCode;
    QStringList skippedPasses;
    QJsonArray imageInputs;
    struct RawBuffer
    {
        QString code;
        QJsonArray inputs;
        QString outputId;  ///< Asset-Id des Ausgangs (Konsumenten referenzieren sie)
    };
    std::vector<RawBuffer> rawBuffers;
    // Ids kommen je nach API-Stand als Zahl ODER String — vereinheitlichen
    const auto idToString = [](const QJsonValue& v) {
        return v.isDouble() ? QString::number(v.toInt()) : v.toString();
    };
    for (const QJsonValue& v : passes)
    {
        const QJsonObject pass = v.toObject();
        const QString type = pass.value(QStringLiteral("type")).toString().toLower();
        if (type == QLatin1String("image"))
        {
            imageCode = pass.value(QStringLiteral("code")).toString();
            imageInputs = pass.value(QStringLiteral("inputs")).toArray();
        }
        else if (type == QLatin1String("common"))
        {
            commonCode = pass.value(QStringLiteral("code")).toString();
        }
        else if (type == QLatin1String("buffer") && rawBuffers.size() < 4)
        {
            // Buffer-Pass (S4): Reihenfolge in der Antwort = A..D
            RawBuffer b;
            b.code = pass.value(QStringLiteral("code")).toString();
            b.inputs = pass.value(QStringLiteral("inputs")).toArray();
            const QJsonArray outputs = pass.value(QStringLiteral("outputs")).toArray();
            if (!outputs.isEmpty())
                b.outputId =
                    idToString(outputs.first().toObject().value(QStringLiteral("id")));
            rawBuffers.push_back(std::move(b));
        }
        else
        {
            skippedPasses << (type.isEmpty() ? QStringLiteral("?") : type);
        }
    }
    if (imageCode.isEmpty())
    {
        r.error = QStringLiteral("kein image-Pass in der Antwort (Sound-only?)");
        return r;
    }

    lumi::multieffect::ShadertoyParams& p = r.params;
    // common ist per Definition Bestandteil ALLER Pässe — überall voranstellen
    const auto withCommon = [&commonCode](const QString& code) {
        return commonCode.isEmpty() ? code.toStdString()
                                    : (commonCode + QStringLiteral("\n") + code)
                                          .toStdString();
    };
    p.code = withCommon(imageCode);
    p.name = info.value(QStringLiteral("name")).toString().toStdString();
    p.author = info.value(QStringLiteral("username")).toString().toStdString();
    p.url = (QStringLiteral("https://www.shadertoy.com/view/") + shaderId).toStdString();
    // Die API liefert keine Lizenz — Shadertoy-Default gilt (Plan §S3)
    p.license = "CC BY-NC-SA 3.0 (Shadertoy-Default)";

    // Kanal-Bindungen: music → Audio, buffer → Buffer-Index (über die
    // Output-Id), alles andere = Platzhalter-Meldung
    const auto bufferIndexForId = [&rawBuffers](const QString& id) {
        for (std::size_t i = 0; i < rawBuffers.size(); ++i)
        {
            if (!id.isEmpty() && rawBuffers[i].outputId == id)
                return static_cast<int>(i);
        }
        return -1;
    };
    const auto applyInputs = [&](const QJsonArray& inputs, std::array<int, 4>& out,
                                 const QString& passLabel) {
        for (const QJsonValue& v : inputs)
        {
            const QJsonObject input = v.toObject();
            // API-Feldname variiert historisch: "ctype" (v1) bzw. "type"
            QString ctype = input.value(QStringLiteral("ctype")).toString();
            if (ctype.isEmpty()) ctype = input.value(QStringLiteral("type")).toString();
            ctype = ctype.toLower();
            const int channel =
                std::clamp(input.value(QStringLiteral("channel")).toInt(0), 0, 3);
            if (ctype == QLatin1String("music") ||
                ctype == QLatin1String("musicstream") || ctype == QLatin1String("mic"))
            {
                out[static_cast<std::size_t>(channel)] =
                    lumi::multieffect::kShadertoyInputAudio;
                r.report << QStringLiteral("ℹ %1: Audio-Input auf iChannel%2 — mit "
                                           "der LumiViz-Audio-Textur belegt")
                                .arg(passLabel)
                                .arg(channel);
            }
            else if (ctype == QLatin1String("buffer"))
            {
                const int idx =
                    bufferIndexForId(idToString(input.value(QStringLiteral("id"))));
                if (idx >= 0)
                {
                    out[static_cast<std::size_t>(channel)] = idx;
                }
                else
                {
                    r.report << QStringLiteral("ℹ %1: iChannel%2-Buffer-Referenz "
                                               "nicht auflösbar — Platzhalter")
                                    .arg(passLabel)
                                    .arg(channel);
                }
            }
            else if (!ctype.isEmpty())
            {
                r.report << QStringLiteral("ℹ %1: iChannel%2-Input '%3' wird noch "
                                           "nicht unterstützt — Platzhalter (schwarz)")
                                .arg(passLabel)
                                .arg(channel)
                                .arg(ctype);
            }
        }
    };

    p.imageInput = {-1, -1, -1, -1};
    applyInputs(imageInputs, p.imageInput, QStringLiteral("Image"));
    for (std::size_t i = 0; i < rawBuffers.size(); ++i)
    {
        lumi::multieffect::ShadertoyPass pass;
        pass.code = withCommon(rawBuffers[i].code);
        applyInputs(rawBuffers[i].inputs, pass.input,
                    QStringLiteral("Buffer %1")
                        .arg(QChar(static_cast<char16_t>('A' + i))));
        p.buffers.push_back(std::move(pass));
    }
    if (!p.buffers.empty())
    {
        r.report << QStringLiteral("ℹ Multipass: %1 Buffer-Pass/Pässe übernommen "
                                   "(S4; Selbst-Referenz liest das Vorframe)")
                        .arg(p.buffers.size());
    }
    if (!skippedPasses.isEmpty())
    {
        r.report << QStringLiteral("ℹ Nicht unterstützte Pässe übersprungen: %1")
                        .arg(skippedPasses.join(QStringLiteral(", ")));
    }

    r.ok = true;
    return r;
}

} // namespace lumi::shadertoy
