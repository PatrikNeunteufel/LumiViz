/**
 * @file Aufnahme.cpp
 * @brief Loop: eine Aufnahme zuschneiden und speichern (Stufe 11b)
 */

#include "loop/Aufnahme.hpp"

#include "audio/WavDatei.hpp"
#include "loop/Instrument.hpp"
#include "loop/Sample.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>

#include <algorithm>
#include <cmath>

namespace lumi::loop
{

namespace
{

std::int64_t framesVon(const std::vector<float>& stereo)
{
    return static_cast<std::int64_t>(stereo.size() / 2);
}

Ausschnitt begrenzt(Ausschnitt ausschnitt, std::int64_t frames)
{
    ausschnitt.von = std::clamp<std::int64_t>(ausschnitt.von, 0, frames);
    ausschnitt.bis = std::clamp<std::int64_t>(ausschnitt.bis, ausschnitt.von, frames);
    return ausschnitt;
}

float betrag(const std::vector<float>& stereo, std::int64_t frame)
{
    const auto i = static_cast<std::size_t>(2 * frame);
    return std::max(std::abs(stereo[i]), std::abs(stereo[i + 1]));
}

}  // namespace

float spitzeIn(const std::vector<float>& stereo, Ausschnitt ausschnitt)
{
    const Ausschnitt a = begrenzt(ausschnitt, framesVon(stereo));
    float groesster = 0.0f;
    for (std::int64_t f = a.von; f < a.bis; ++f) groesster = std::max(groesster, betrag(stereo, f));
    return groesster;
}

Ausschnitt ohneStille(const std::vector<float>& stereo, int rate)
{
    const std::int64_t frames = framesVon(stereo);
    const Ausschnitt ganz{0, frames};
    const float spitze = spitzeIn(stereo, ganz);
    if (frames == 0 || rate <= 0 || spitze <= 0.0f) return ganz;
    const auto schwelle = static_cast<float>(spitze * kStilleAnteil);
    std::int64_t erster = 0;
    while (erster < frames && betrag(stereo, erster) < schwelle) ++erster;
    std::int64_t letzter = frames - 1;
    while (letzter > erster && betrag(stereo, letzter) < schwelle) --letzter;
    const auto vor = static_cast<std::int64_t>(kStilleVorMs * rate / 1000.0);
    const auto nach = static_cast<std::int64_t>(kStilleNachMs * rate / 1000.0);
    return begrenzt({erster - vor, letzter + 1 + nach}, frames);
}

std::vector<float> schneideAus(const std::vector<float>& stereo, int rate, Ausschnitt ausschnitt, bool normalisieren, Blenden blenden)
{
    const Ausschnitt a = begrenzt(ausschnitt, framesVon(stereo));
    const std::int64_t frames = a.frames();
    if (frames <= 0 || rate <= 0) return {};
    std::vector<float> aus(stereo.begin() + 2 * a.von, stereo.begin() + 2 * a.bis);

    if (normalisieren)
    {
        const float spitze = spitzeIn(aus, {0, frames});
        if (spitze > 0.0f)
        {
            const auto faktor = static_cast<float>(kNormalSpitze / spitze);
            for (float& wert : aus) wert *= faktor;
        }
    }
    // an beiden Enden kurz auf null: ein Schnitt mitten im Ton knackt sonst
    const std::int64_t blende = std::min<std::int64_t>(static_cast<std::int64_t>(kSchnittBlendeMs * rate / 1000.0), frames / 2);
    for (std::int64_t f = 0; f < blende; ++f)
    {
        const float anteil = static_cast<float>(f) / static_cast<float>(blende);
        for (const std::int64_t stelle : {f, frames - 1 - f})
        {
            // ein Ende, an dem nicht geschnitten wurde, bleibt wie es ist — der Anschlag eines Schlags etwa
            if (stelle == f ? !blenden.vorn : !blenden.hinten) continue;
            aus[static_cast<std::size_t>(2 * stelle)] *= anteil;
            aus[static_cast<std::size_t>(2 * stelle + 1)] *= anteil;
        }
    }
    return aus;
}

bool schreibeWav(const QString& pfad, const std::vector<float>& stereo, int rate)
{
    // derselbe Schreiber wie Export audio… und Freeze (Stufe 30b) — 16 Bit ohne Dither, wie seit 11b
    if (framesVon(stereo) <= 0 || rate <= 0) return false;
    return audio::schreibeWav(pfad, stereo, rate, audio::WavTiefe::Pcm16).geschrieben;
}

QString vorschlagAufnahmeTitel()
{
    const QString grund = QStringLiteral("Recording");
    for (int zahl = 1;; ++zahl)
    {
        const QString titel = zahl == 1 ? grund : QStringLiteral("%1 %2").arg(grund).arg(zahl);
        if (eigeneSampleDatei(eigenerName(titel)).isEmpty()) return titel;
    }
}

QString vorschlagKopieTitel(const QString& titel)
{
    // der Name ohne Endung: aus „cello.mp3" wird „cello 2", nicht „cello.mp3 2"
    const QString grund = QFileInfo(titel).completeBaseName();
    if (eigenerName(grund).isEmpty()) return vorschlagAufnahmeTitel();
    for (int zahl = 2;; ++zahl)
    {
        const QString kopie = QStringLiteral("%1 %2").arg(grund).arg(zahl);
        if (eigeneSampleDatei(eigenerName(kopie)).isEmpty()) return kopie;
    }
}

QString speichereAufnahme(const std::vector<float>& stereo, int rate, const QString& titel)
{
    const QString name = eigenerName(titel);
    const QString ordner = eigenerSampleOrdner();
    if (name.isEmpty() || ordner.isEmpty() || framesVon(stereo) <= 0 || rate <= 0) return {};
    if (!QDir().mkpath(ordner)) return {};
    const QString nennung = QLatin1String(kSampleVorsatz) + name;
    const QString ziel = QDir(ordner).filePath(name + QStringLiteral(".wav"));

    // was unter dem Namen gemerkt ist, gilt nicht mehr — solange die alte Datei noch da ist: ihr Schluessel
    vergissSampleDaten(nennung);
    if (!schreibeWav(ziel, stereo, rate)) return {};
    // derselbe Name mit anderer Endung waere ein zweites Sample dieses Namens: weg damit
    for (const QString& endung : sampleEndungen())
        if (endung != QLatin1String("wav")) QFile::remove(QDir(ordner).filePath(name + QLatin1Char('.') + endung));
    vergissSampleDaten(nennung);
    zaehleInstrumentFassung();  // wer Toene mit diesem Sample gerechnet hat, rechnet neu
    return nennung;
}

}  // namespace lumi::loop
