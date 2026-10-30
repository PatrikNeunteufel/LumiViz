/**
 * @file Austausch.cpp
 * @brief Loop: Muster, Clips und Spuren als Dateien (Stufe 33, S167)
 */

#include "loop/Austausch.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>

#include <algorithm>
#include <cmath>
#include <map>

namespace lumi::loop
{

namespace
{

QString formatVon(AustauschArt art)
{
    switch (art)
    {
    case AustauschArt::Muster: return QStringLiteral("lumiviz-pattern");
    case AustauschArt::Clip: return QStringLiteral("lumiviz-clip");
    case AustauschArt::Spur: return QStringLiteral("lumiviz-track");
    }
    return {};
}

/// Ein leerer Loop mit Tempo, Taktart, Raster und Swing von `loop`.
Loop rahmenVon(const Loop& loop)
{
    Loop teil;
    teil.bpm = loop.bpm;
    teil.schlaegeJeTakt = loop.schlaegeJeTakt;
    teil.schritteJeSchlag = loop.schritteJeSchlag;
    teil.swing = loop.swing;
    return teil;
}

/// Hat der kleine Loop die Gestalt seiner Art?
bool hatGestalt(AustauschArt art, const Loop& teil)
{
    switch (art)
    {
    case AustauschArt::Muster: return teil.muster.size() == 1 && teil.kanaele.empty();
    case AustauschArt::Clip:
        return teil.kanaele.size() == 1 && teil.kanaele[0].clips.size() == 1 &&
               teil.findeMuster(teil.kanaele[0].clips[0].muster) != nullptr;
    case AustauschArt::Spur: return teil.kanaele.size() == 1;
    }
    return false;
}

}  // namespace

QString austauschEndung(AustauschArt art)
{
    switch (art)
    {
    case AustauschArt::Muster: return QLatin1String(kMusterDateiEndung);
    case AustauschArt::Clip: return QLatin1String(kClipDateiEndung);
    case AustauschArt::Spur: return QLatin1String(kSpurDateiEndung);
    }
    return {};
}

std::optional<AustauschArt> austauschArtVonDatei(const QString& datei)
{
    for (const AustauschArt art : {AustauschArt::Muster, AustauschArt::Clip, AustauschArt::Spur})
        if (datei.endsWith(austauschEndung(art), Qt::CaseInsensitive)) return art;
    return std::nullopt;
}

namespace
{

QString& basisImTest()
{
    static QString basis;
    return basis;
}

}  // namespace

QString eigenerAustauschOrdner(AustauschArt art)
{
    const QString basis = !basisImTest().isEmpty() ? basisImTest()
                                                   : QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (basis.isEmpty()) return {};
    return QDir(basis).filePath(art == AustauschArt::Muster ? QStringLiteral("patterns") : QStringLiteral("tracks"));
}

void setzeAustauschBasisFuerTests(const QString& basis)
{
    basisImTest() = basis;
}

std::optional<Loop> musterDatei(const Loop& loop, const QString& name)
{
    const Muster* muster = loop.findeMuster(name);
    if (muster == nullptr) return std::nullopt;
    Loop teil = rahmenVon(loop);
    teil.muster = {*muster};
    return teil;
}

std::optional<Loop> clipDatei(const Loop& loop, int kanal, int clip)
{
    if (kanal < 0 || kanal >= static_cast<int>(loop.kanaele.size())) return std::nullopt;
    const Kanal& k = loop.kanaele[static_cast<std::size_t>(kanal)];
    if (clip < 0 || clip >= static_cast<int>(k.clips.size())) return std::nullopt;
    LoopClip c = k.clips[static_cast<std::size_t>(clip)];
    const Muster* muster = loop.findeMuster(c.muster);
    if (muster == nullptr) return std::nullopt;
    c.startTakt = 0;
    Loop teil = rahmenVon(loop);
    teil.muster = {*muster};
    Kanal nur;
    nur.name = k.name;
    nur.clips = {std::move(c)};
    teil.kanaele = {std::move(nur)};
    return teil;
}

std::optional<Loop> spurDatei(const Loop& loop, int kanal)
{
    if (kanal < 0 || kanal >= static_cast<int>(loop.kanaele.size())) return std::nullopt;
    const Kanal& k = loop.kanaele[static_cast<std::size_t>(kanal)];
    Loop teil = rahmenVon(loop);
    for (const Muster& m : loop.muster)
        if (std::any_of(k.clips.begin(), k.clips.end(), [&m](const LoopClip& c) { return c.muster == m.name; }))
            teil.muster.push_back(m);
    teil.kanaele = {k};
    return teil;
}

QByteArray austauschText(AustauschArt art, const Loop& teil, const QString& basisOrdner)
{
    QJsonObject huelle;
    huelle[QStringLiteral("format")] = formatVon(art);
    huelle[QStringLiteral("version")] = kAustauschFassung;
    huelle[QStringLiteral("loop")] = alsJson(teil, basisOrdner);
    return loopDateiText(huelle);
}

bool speichereAustausch(AustauschArt art, const Loop& teil, const QString& pfad)
{
    QSaveFile datei(pfad);
    if (!datei.open(QIODevice::WriteOnly)) return false;
    return datei.write(austauschText(art, teil, QFileInfo(pfad).absolutePath())) >= 0 && datei.commit();
}

bool ladeAustausch(const QString& pfad, AustauschArt art, Loop& teil, QStringList* bericht, bool* zuNeu)
{
    if (zuNeu != nullptr) *zuNeu = false;
    QFile datei(pfad);
    if (!datei.open(QIODevice::ReadOnly)) return false;
    return austauschAusText(datei.readAll(), art, teil, QFileInfo(pfad).absolutePath(), bericht, zuNeu);
}

bool austauschAusText(const QByteArray& text, AustauschArt art, Loop& teil, const QString& basisOrdner,
                      QStringList* bericht, bool* zuNeu)
{
    if (zuNeu != nullptr) *zuNeu = false;
    QJsonParseError fehler;
    const QJsonDocument doc = QJsonDocument::fromJson(text, &fehler);
    if (fehler.error != QJsonParseError::NoError || !doc.isObject()) return false;
    const QJsonObject huelle = doc.object();
    if (huelle.value(QStringLiteral("format")).toString() != formatVon(art)) return false;
    if (huelle.value(QStringLiteral("version")).toInt(1) > kAustauschFassung)
    {
        if (zuNeu != nullptr) *zuNeu = true;  // eine Fassung von morgen: lieber nichts als etwas Falsches
        return false;
    }
    Loop gelesen;
    QStringList hier;
    if (!ausJson(huelle.value(QStringLiteral("loop")).toObject(), gelesen, basisOrdner, &hier))
        return false;
    if (!hatGestalt(art, gelesen)) return false;
    if (bericht != nullptr) *bericht = hier;
    teil = std::move(gelesen);
    return true;
}

Umrechnung rechneUm(Loop& teil, int schlaegeJeTakt, int schritteJeSchlag)
{
    // Gleitkomma: ein Rest unter dieser Grenze gilt als genau auf dem Schritt
    constexpr double kGenau = 1e-9;
    Umrechnung u;
    schlaegeJeTakt = std::clamp(schlaegeJeTakt, 1, kRasterMax);
    schritteJeSchlag = std::clamp(schritteJeSchlag, 1, kRasterMax);
    if (teil.schlaegeJeTakt == schlaegeJeTakt && teil.schritteJeSchlag == schritteJeSchlag) return u;

    const double schritt = static_cast<double>(schritteJeSchlag) / teil.schritteJeSchlag;  // alt → neu
    const double takt = static_cast<double>(teil.schlaegeJeTakt) / schlaegeJeTakt;        // alt → neu
    for (Muster& m : teil.muster)
    {
        const int schlaege = m.takte * teil.schlaegeJeTakt;
        const int takte = std::clamp(static_cast<int>(std::ceil(static_cast<double>(schlaege) / schlaegeJeTakt - kGenau)),
                                     kTakteMin, kTakteMax);
        const int schritte = takte * schlaegeJeTakt * schritteJeSchlag;

        for (Note& n : m.noten)
        {
            const double anfang = (n.schritt + n.versatzAnfang) * schritt;
            n.schritt = static_cast<int>(std::floor(anfang + kGenau));
            n.versatzAnfang = std::max(0.0, anfang - n.schritt);
            if (n.versatzAnfang < kGenau) n.versatzAnfang = 0.0;
            const double dauer = (n.laenge + n.versatzEnde) * schritt;
            n.laenge = std::max(1, static_cast<int>(std::ceil(dauer - kGenau)));
            n.versatzEnde = std::min(0.0, dauer - n.laenge);
            if (n.versatzEnde > -kGenau) n.versatzEnde = 0.0;
        }
        for (Anschlag& a : m.anschlaege) a.schritt = static_cast<int>(std::floor(a.schritt * schritt + kGenau));
        for (std::vector<SpurPunkt>& spur : m.zeitspuren.steuer)
            for (SpurPunkt& p : spur) p.stelle *= schritt;
        for (SchalterSpur& spur : m.zeitspuren.schalter)
            for (SpurPunkt& p : spur.punkte) p.stelle *= schritt;

        for (std::vector<double>& zeile : m.zeilen)
        {
            std::vector<double> neu(static_cast<std::size_t>(schritte), 0.0);
            for (std::size_t i = 0; i < zeile.size(); ++i)
            {
                if (zeile[i] <= 0.0) continue;
                const double ziel = static_cast<double>(i) * schritt;
                const int j = std::clamp(static_cast<int>(std::floor(ziel + 0.5 - kGenau)), 0, schritte - 1);
                if (std::abs(ziel - j) > kGenau) ++u.verschoben;
                double& dort = neu[static_cast<std::size_t>(j)];
                if (dort > 0.0) ++u.zusammengelegt;
                dort = std::max(dort, zeile[i]);
            }
            zeile = std::move(neu);
        }
        m.takte = takte;
    }
    for (Kanal& k : teil.kanaele)
        for (LoopClip& c : k.clips)
            for (SpurPunkt& p : c.expression) p.stelle *= takt;
    teil.schlaegeJeTakt = schlaegeJeTakt;
    teil.schritteJeSchlag = schritteJeSchlag;
    teil.richte();
    return u;
}

namespace
{

/// Der Name, unter dem `muster` in `ziel` steht: der eigene, `_2`, `_3`, … — der erste,
/// der frei ist oder ein Muster gleichen Inhalts traegt. `true` = es steht schon dort.
std::pair<QString, bool> nameIn(const Loop& ziel, const Muster& muster)
{
    for (int n = 1;; ++n)
    {
        const QString name = n == 1 ? muster.name : QStringLiteral("%1_%2").arg(muster.name).arg(n);
        const Muster* dort = ziel.findeMuster(name);
        if (dort == nullptr) return {name, false};
        Muster gleich = muster;
        gleich.name = name;
        if (*dort == gleich) return {name, true};
    }
}

/// Hat der Kanal ab `takt` fuer `takte` Takte Platz?
bool hatPlatz(const Loop& loop, const Kanal& kanal, int takt, int takte)
{
    return std::none_of(kanal.clips.begin(), kanal.clips.end(), [&](const LoopClip& c) {
        const int anfang = c.startTakt;
        const int ende = anfang + loop.clipTakte(c);
        return anfang < takt + takte && takt < ende;
    });
}

}  // namespace

Einsetzung setzeEin(Loop& ziel, Loop teil, AustauschArt art, int kanal, int takt)
{
    Einsetzung e;
    if (!hatGestalt(art, teil)) return e;
    e.umgerechnet = teil.schlaegeJeTakt != ziel.schlaegeJeTakt || teil.schritteJeSchlag != ziel.schritteJeSchlag;
    e.umrechnung = rechneUm(teil, ziel.schlaegeJeTakt, ziel.schritteJeSchlag);

    // die Muster: mitbenutzen oder unter freiem Namen; danach nennen die Clips den neuen
    // (erst alle Namen, dann die Clips — sonst traefe ein neuer Name einen alten der Datei)
    std::map<QString, QString> neuerName;
    for (Muster& m : teil.muster)
    {
        const auto [name, steht] = nameIn(ziel, m);
        neuerName[m.name] = name;
        if (name != m.name) e.umbenannt << QStringLiteral("%1 → %2").arg(m.name, name);
        m.name = name;
        if (!steht) ziel.muster.push_back(m);
    }
    for (Kanal& k : teil.kanaele)
        for (LoopClip& c : k.clips)
            if (const auto it = neuerName.find(c.muster); it != neuerName.end()) c.muster = it->second;

    if (art == AustauschArt::Spur)
    {
        ziel.kanaele.push_back(teil.kanaele[0]);
        e.kanal = static_cast<int>(ziel.kanaele.size()) - 1;
        e.neuerKanal = true;
        ziel.richte();
        return e;
    }

    LoopClip clip = art == AustauschArt::Clip ? teil.kanaele[0].clips[0] : LoopClip(teil.muster[0].name, 0, 1);
    clip.startTakt = std::max(0, takt);
    const int takte = ziel.clipTakte(clip);
    if (kanal >= 0 && kanal < static_cast<int>(ziel.kanaele.size()) &&
        hatPlatz(ziel, ziel.kanaele[static_cast<std::size_t>(kanal)], clip.startTakt, takte))
    {
        ziel.kanaele[static_cast<std::size_t>(kanal)].clips.push_back(std::move(clip));
        e.kanal = kanal;
    }
    else
    {
        Kanal neu;
        neu.name = art == AustauschArt::Clip ? teil.kanaele[0].name : teil.muster[0].name;
        neu.clips = {std::move(clip)};
        ziel.kanaele.push_back(std::move(neu));
        e.kanal = static_cast<int>(ziel.kanaele.size()) - 1;
        e.neuerKanal = true;
    }
    ziel.richte();
    return e;
}

}  // namespace lumi::loop
