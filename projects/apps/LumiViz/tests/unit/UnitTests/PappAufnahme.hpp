/**
 * @file PappAufnahme.hpp
 * @brief Eine Audio-Engine aus Pappe, die „aufnimmt" — fuer die Tests zum
 *        Fenster „Record sample" (Loop, Stufe 11b)
 *
 * Sie liefert als Aufnahme, was der Test ihr gibt, und reicht die Quelle
 * heraus, die ein Fenster in ihren Mixer haengt. Kein Ton, kein Eingang.
 */
#pragma once

#include "PappTon.hpp"

#include <QString>
#include <QStringList>

#include <algorithm>
#include <cstdint>
#include <vector>

namespace papp
{

/// `aufnahme` ist, was der Eingang liefert; `recordStop` gibt davon heraus,
/// was bis dahin „angekommen" ist (`angekommen`).
class AufnahmeEngine : public PappMischEngine
{
public:
    std::vector<AudioDeviceInfo> eingaenge;
    std::vector<float> aufnahme;   ///< Stereo
    std::int64_t angekommen = 0;   ///< so viele Abtastwerte je Seite sind schon da
    float pegel = 0.0f;
    bool laeuft = false;
    bool oeffnet = true;           ///< false = der Eingang laesst sich nicht oeffnen
    QStringList rufe;
    IAudioGenerator* generator = nullptr;

    [[nodiscard]] std::vector<AudioDeviceInfo> getRecordDevices() const override { return eingaenge; }
    bool recordStart(int deviceId, int sampleRate, double maxSeconds) override
    {
        rufe << QStringLiteral("start %1 %2 %3").arg(deviceId).arg(sampleRate).arg(maxSeconds);
        if (!oeffnet) return false;
        laeuft = true;
        angekommen = 0;
        return true;
    }
    [[nodiscard]] bool isRecording() const override { return laeuft; }
    [[nodiscard]] std::int64_t recordedFrames() const override { return angekommen; }
    [[nodiscard]] float recordLevel() override { return pegel; }
    [[nodiscard]] std::vector<float> recordStop() override
    {
        rufe << QStringLiteral("stop");
        laeuft = false;
        const auto zahl = static_cast<std::size_t>(2 * std::min<std::int64_t>(angekommen, static_cast<std::int64_t>(aufnahme.size() / 2)));
        return std::vector<float>(aufnahme.begin(), aufnahme.begin() + static_cast<std::ptrdiff_t>(zahl));
    }
    [[nodiscard]] AudioStreamHandle mixerAddGenerator(AudioStreamHandle, IAudioGenerator* quelle) override
    {
        generator = quelle;
        Stream s;
        s.datei = QStringLiteral("<generator>");
        s.dauer = 1000000;
        s.haelt = true;
        m_streams.push_back(s);
        protokoll << QStringLiteral("add generator");
        return letzter();
    }
    [[nodiscard]] int getSampleRate() const override { return 48000; }
};

/// Ein Eingang fuer `AufnahmeEngine::eingaenge`.
inline AudioDeviceInfo eingang(int id, const QString& name, bool standard, bool mitschnitt = false)
{
    AudioDeviceInfo info;
    info.id = id;
    info.name = name;
    info.isDefault = standard;
    info.isLoopback = mitschnitt;
    return info;
}

}  // namespace papp
