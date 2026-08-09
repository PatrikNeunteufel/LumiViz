/**
 ****************************************************************************************
 * @file   AudioDateiQuelle.cpp
 * @brief  Umsetzung von AudioDateiQuelle — Dekodieren, FFT, bild-indizierte Zufuhr
 ****************************************************************************************
 */

#include "AudioDateiQuelle.hpp"

#include <QAudioBuffer>
#include <QAudioDecoder>
#include <QAudioFormat>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QTimer>
#include <QUrl>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace lumi::werkzeug
{

namespace
{

constexpr double kPi = 3.14159265358979323846;
/// Notbremse: ein kaputter Dekoder darf den Standalone nicht aufhaengen
constexpr int kDekodierZeitlimitMs = 120000;

/// Cooley-Tukey in-place, Laenge muss Zweierpotenz sein. Die Drehfaktoren
/// werden direkt gerechnet (kein Rekurrenz-Fehler) — bei 1024 Punkten
/// vernachlaessigbar teuer und dafuer bitgleich reproduzierbar.
void fftAmOrt(double* re, double* im, int n)
{
    for (int i = 1, j = 0; i < n; ++i)
    {
        int bit = n >> 1;
        for (; (j & bit) != 0; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j)
        {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }
    for (int len = 2; len <= n; len <<= 1)
    {
        const int halb = len / 2;
        for (int i = 0; i < n; i += len)
        {
            for (int k = 0; k < halb; ++k)
            {
                const double winkel = -2.0 * kPi * k / len;
                const double cr = std::cos(winkel);
                const double ci = std::sin(winkel);
                const double ur = re[i + k];
                const double ui = im[i + k];
                const double vr = re[i + k + halb] * cr - im[i + k + halb] * ci;
                const double vi = re[i + k + halb] * ci + im[i + k + halb] * cr;
                re[i + k] = ur + vr;
                im[i + k] = ui + vi;
                re[i + k + halb] = ur - vr;
                im[i + k + halb] = ui - vi;
            }
        }
    }
}

/// Hann-Fenster, einmal gerechnet (die FFT laeuft je Bild zweimal)
const std::vector<double>& hannFenster()
{
    static const std::vector<double> fenster = [] {
        std::vector<double> f(AudioDateiQuelle::kFftGroesse);
        for (int i = 0; i < AudioDateiQuelle::kFftGroesse; ++i)
        {
            f[static_cast<std::size_t>(i)] =
                0.5 - 0.5 * std::cos(2.0 * kPi * i / (AudioDateiQuelle::kFftGroesse - 1));
        }
        return f;
    }();
    return fenster;
}

/// Einen dekodierten Block als interleaved Stereo-Float anhaengen.
/// Deckt die Formate ab, die Qt-Backends liefern koennen — der FFmpeg-Backend
/// haelt sich an setAudioFormat(), andere nicht zwingend.
///
/// LEISTUNG: Basiszeiger und Formatverzweigung liegen AUSSERHALB der
/// Sample-Schleife. Mit `constData<T>()` je Sample brauchte eine Vier-Minuten-
/// Datei ~68 s zum Einlesen (~21 Mio. Aufrufe) — der Standalone sah dabei aus,
/// als haenge er beim Start.
void blockAnhaengen(std::vector<float>& ziel, const QAudioBuffer& puffer)
{
    const QAudioFormat fmt = puffer.format();
    const int kanaele = std::max(1, fmt.channelCount());
    const int rahmen = puffer.frameCount();
    if (rahmen <= 0) return;

    const std::size_t alt = ziel.size();
    ziel.resize(alt + static_cast<std::size_t>(rahmen) * 2);
    float* aus = ziel.data() + alt;
    const int rechts = (kanaele > 1) ? 1 : 0;

    const auto uebertragen = [&](const auto* quelle, auto normieren) {
        for (int f = 0; f < rahmen; ++f)
        {
            const int basis = f * kanaele;
            aus[f * 2 + 0] = normieren(quelle[basis]);
            aus[f * 2 + 1] = normieren(quelle[basis + rechts]);
        }
    };

    switch (fmt.sampleFormat())
    {
    case QAudioFormat::Float:
        uebertragen(puffer.constData<float>(), [](float v) { return v; });
        break;
    case QAudioFormat::Int16:
        uebertragen(puffer.constData<qint16>(),
                    [](qint16 v) { return v / 32768.0f; });
        break;
    case QAudioFormat::Int32:
        uebertragen(puffer.constData<qint32>(),
                    [](qint32 v) { return static_cast<float>(v / 2147483648.0); });
        break;
    case QAudioFormat::UInt8:
        uebertragen(puffer.constData<quint8>(),
                    [](quint8 v) { return (static_cast<int>(v) - 128) / 128.0f; });
        break;
    default:
        std::fill(aus, aus + static_cast<std::size_t>(rahmen) * 2, 0.0f);
        break;
    }
}

/// Lineare Umtastung auf kAbtastrate (nur noetig, wenn der Backend die
/// gewuenschte Rate ignoriert hat)
std::vector<float> umtasten(const std::vector<float>& quelle, int vonRate)
{
    if (vonRate == AudioDateiQuelle::kAbtastrate || vonRate <= 0) return quelle;
    const std::size_t rahmenQ = quelle.size() / 2;
    if (rahmenQ < 2) return quelle;
    const double verhaeltnis =
        static_cast<double>(AudioDateiQuelle::kAbtastrate) / vonRate;
    const std::size_t rahmenZ = static_cast<std::size_t>(rahmenQ * verhaeltnis);
    std::vector<float> ziel(rahmenZ * 2, 0.0f);
    for (std::size_t i = 0; i < rahmenZ; ++i)
    {
        const double pos = i / verhaeltnis;
        const std::size_t a = static_cast<std::size_t>(pos);
        const std::size_t b = std::min(a + 1, rahmenQ - 1);
        const double t = pos - a;
        for (int k = 0; k < 2; ++k)
        {
            ziel[i * 2 + k] = static_cast<float>(
                quelle[a * 2 + k] * (1.0 - t) + quelle[b * 2 + k] * t);
        }
    }
    return ziel;
}

}  // namespace

// =============================================================================================
// AudioDateiQuelle
// =============================================================================================

bool AudioDateiQuelle::laden(const QString& pfad, QString* fehler)
{
    const auto scheitern = [&](const QString& text) {
        if (fehler != nullptr) *fehler = text;
        m_pcm.clear();
        m_frames = 0;
        return false;
    };

    const QFileInfo info(pfad);
    if (!info.isFile()) return scheitern(QStringLiteral("Datei nicht gefunden: %1").arg(pfad));

    QAudioFormat wunsch;
    wunsch.setSampleRate(kAbtastrate);
    wunsch.setChannelCount(kKanaele);
    wunsch.setSampleFormat(QAudioFormat::Float);

    QAudioDecoder dekoder;
    dekoder.setAudioFormat(wunsch);
    dekoder.setSource(QUrl::fromLocalFile(info.absoluteFilePath()));

    std::vector<float> roh;
    int rohRate = kAbtastrate;
    bool rateGesehen = false;
    QString dekoderFehler;

    QEventLoop schleife;
    QObject::connect(&dekoder, &QAudioDecoder::bufferReady, &dekoder, [&] {
        const QAudioBuffer puffer = dekoder.read();
        if (!puffer.isValid()) return;
        if (!rateGesehen)
        {
            rohRate = puffer.format().sampleRate();
            rateGesehen = true;
        }
        blockAnhaengen(roh, puffer);
    });
    QObject::connect(&dekoder, &QAudioDecoder::finished, &schleife, &QEventLoop::quit);
    // Das Fehlersignal heisst in Qt6 `error` wie der gleichnamige Getter —
    // qOverload loest die Mehrdeutigkeit auf
    QObject::connect(&dekoder, qOverload<QAudioDecoder::Error>(&QAudioDecoder::error),
                     &dekoder, [&](QAudioDecoder::Error) {
                         dekoderFehler = dekoder.errorString();
                         schleife.quit();
                     });
    QTimer::singleShot(kDekodierZeitlimitMs, &schleife, &QEventLoop::quit);

    dekoder.start();
    schleife.exec();
    dekoder.stop();

    if (roh.empty())
    {
        return scheitern(dekoderFehler.isEmpty()
                             ? QStringLiteral("keine Audiodaten dekodiert: %1").arg(pfad)
                             : dekoderFehler);
    }

    m_pcm = umtasten(roh, rateGesehen ? rohRate : kAbtastrate);
    m_frames = static_cast<std::int64_t>(m_pcm.size() / 2);
    m_quelle = info.fileName();
    // Startversatz erneut klemmen — setStartSekunden() kann vor laden() gerufen
    // worden sein, als die Laenge noch unbekannt war
    m_startRahmen = std::clamp<std::int64_t>(m_startRahmen, 0, std::max<std::int64_t>(0, m_frames - 1));
    return true;
}

void AudioDateiQuelle::setStartSekunden(double sekunden)
{
    const std::int64_t rahmen =
        static_cast<std::int64_t>(std::llround(std::max(0.0, sekunden) * kAbtastrate));
    m_startRahmen = (m_frames > 0) ? std::clamp<std::int64_t>(rahmen, 0, m_frames - 1) : rahmen;
}

void AudioDateiQuelle::setVerstaerkung(double faktor)
{
    m_verstaerkung = (faktor > 0.0) ? faktor : 1.0;
}

double AudioDateiQuelle::dauerSekunden() const
{
    return static_cast<double>(m_frames) / kAbtastrate;
}

void AudioDateiQuelle::frameFuellen(std::int64_t bildIndex, float* wave, float* spec) const
{
    if (wave != nullptr) std::memset(wave, 0, sizeof(float) * kWaveFrames * 2);
    if (spec != nullptr) std::memset(spec, 0, sizeof(float) * kBins * 2);
    if (m_frames <= 0) return;

    // Leseposition ALLEIN aus dem Bild-Index — das ist der Determinismus-Vertrag
    const std::int64_t versatz =
        static_cast<std::int64_t>(std::llround(bildIndex * kAbtastrate / kBildrate));
    const std::int64_t basis = m_startRahmen + versatz;

    // Umlaufend lesen: laeuft die Datei aus, geht es am Anfang weiter
    const auto probe = [&](std::int64_t rahmen, int kanal) -> double {
        std::int64_t r = rahmen % m_frames;
        if (r < 0) r += m_frames;
        return m_pcm[static_cast<std::size_t>(r) * 2 + static_cast<std::size_t>(kanal)];
    };

    if (wave != nullptr)
    {
        for (int i = 0; i < kWaveFrames; ++i)
        {
            wave[static_cast<std::size_t>(i) * 2 + 0] =
                static_cast<float>(probe(basis + i, 0) * m_verstaerkung);
            wave[static_cast<std::size_t>(i) * 2 + 1] =
                static_cast<float>(probe(basis + i, 1) * m_verstaerkung);
        }
    }

    if (spec == nullptr) return;

    // Spektrum je Kanal: Hann-gefensterte kFftGroesse-Punkt-FFT ab derselben
    // Position wie die Wellenform. Normiert auf n/4 (Hann-Kohaerenzgewinn 0,5),
    // damit ein Vollausschlag-Sinus rund 1,0 ergibt — dieselbe Groessenordnung
    // wie die BASS-Baender der App.
    const std::vector<double>& fenster = hannFenster();
    std::vector<double> re(kFftGroesse);
    std::vector<double> im(kFftGroesse);
    for (int kanal = 0; kanal < 2; ++kanal)
    {
        for (int i = 0; i < kFftGroesse; ++i)
        {
            re[static_cast<std::size_t>(i)] =
                probe(basis + i, kanal) * fenster[static_cast<std::size_t>(i)];
            im[static_cast<std::size_t>(i)] = 0.0;
        }
        fftAmOrt(re.data(), im.data(), kFftGroesse);
        const double norm = 4.0 / kFftGroesse;
        for (int b = 0; b < kBins; ++b)
        {
            const double betrag = std::hypot(re[static_cast<std::size_t>(b)],
                                             im[static_cast<std::size_t>(b)]) *
                                  norm * m_verstaerkung;
            spec[static_cast<std::size_t>(b) * 2 + static_cast<std::size_t>(kanal)] =
                static_cast<float>(betrag);
        }
    }
}

// =============================================================================================
// Musik-Profil
// =============================================================================================

namespace
{

constexpr int kProfilBaender = 8;
constexpr double kBandUnten = 40.0;
constexpr double kBandOben = 16000.0;

}  // namespace

bool AudioDateiQuelle::profilSchreiben(const QString& zielDatei, double sekunden,
                                       QString* fehler) const
{
    const auto scheitern = [&](const QString& text) {
        if (fehler != nullptr) *fehler = text;
        return false;
    };
    if (m_frames <= 0) return scheitern(QStringLiteral("keine Audiodaten geladen"));

    const int bilder = static_cast<int>(std::llround(std::max(1.0, sekunden) * kBildrate));

    // --- Bandgrenzen, log-verteilt ------------------------------------------------------
    std::vector<double> kanten(kProfilBaender + 1);
    const double schritt = std::pow(kBandOben / kBandUnten, 1.0 / kProfilBaender);
    kanten[0] = kBandUnten;
    for (int k = 1; k <= kProfilBaender; ++k) kanten[static_cast<std::size_t>(k)] =
        kanten[static_cast<std::size_t>(k) - 1] * schritt;
    std::vector<double> mitten(kProfilBaender);
    for (int k = 0; k < kProfilBaender; ++k)
    {
        mitten[static_cast<std::size_t>(k)] =
            std::sqrt(kanten[static_cast<std::size_t>(k)] *
                      kanten[static_cast<std::size_t>(k) + 1]);
    }

    // --- Je Bild eine FFT, daraus die Bandenergien --------------------------------------
    const std::vector<double>& fenster = hannFenster();
    std::vector<double> re(kFftGroesse);
    std::vector<double> im(kFftGroesse);
    std::vector<double> roh(static_cast<std::size_t>(bilder) * kProfilBaender, 0.0);

    for (int f = 0; f < bilder; ++f)
    {
        const std::int64_t basis =
            m_startRahmen + static_cast<std::int64_t>(std::llround(f * kAbtastrate / kBildrate));
        for (int i = 0; i < kFftGroesse; ++i)
        {
            std::int64_t r = (basis + i) % m_frames;
            if (r < 0) r += m_frames;
            const double mono = 0.5 * (m_pcm[static_cast<std::size_t>(r) * 2 + 0] +
                                       m_pcm[static_cast<std::size_t>(r) * 2 + 1]);
            re[static_cast<std::size_t>(i)] = mono * fenster[static_cast<std::size_t>(i)];
            im[static_cast<std::size_t>(i)] = 0.0;
        }
        fftAmOrt(re.data(), im.data(), kFftGroesse);

        for (int k = 0; k < kProfilBaender; ++k)
        {
            const int von = std::clamp(
                static_cast<int>(kanten[static_cast<std::size_t>(k)] / (kAbtastrate * 0.5) * kBins),
                0, kBins - 1);
            const int bis = std::clamp(
                static_cast<int>(kanten[static_cast<std::size_t>(k) + 1] / (kAbtastrate * 0.5) *
                                 kBins),
                von + 1, kBins);
            double summe = 0.0;
            for (int b = von; b < bis; ++b)
            {
                summe += std::hypot(re[static_cast<std::size_t>(b)],
                                    im[static_cast<std::size_t>(b)]);
            }
            roh[static_cast<std::size_t>(f) * kProfilBaender + k] = summe / (bis - von);
        }
    }

    // --- Je Band auf sein eigenes 95er-Perzentil normieren (s. Kopfkommentar) ------------
    std::vector<double> norm(kProfilBaender, 1.0);
    for (int k = 0; k < kProfilBaender; ++k)
    {
        std::vector<double> werte(static_cast<std::size_t>(bilder));
        for (int f = 0; f < bilder; ++f)
            werte[static_cast<std::size_t>(f)] = roh[static_cast<std::size_t>(f) * kProfilBaender + k];
        std::sort(werte.begin(), werte.end());
        const double p95 = werte[static_cast<std::size_t>(bilder * 95 / 100)];
        norm[static_cast<std::size_t>(k)] = (p95 > 1e-9) ? 1.0 / p95 : 0.0;
    }

    std::vector<unsigned char> huellen(static_cast<std::size_t>(bilder) * kProfilBaender, 0);
    for (std::size_t i = 0; i < huellen.size(); ++i)
    {
        const double v = roh[i] * norm[i % kProfilBaender];
        huellen[i] = static_cast<unsigned char>(std::clamp(v, 0.0, 1.0) * 255.0 + 0.5);
    }

    // --- Beat-Spur: Spektralfluss der drei unteren Baender -------------------------------
    std::vector<double> fluss(static_cast<std::size_t>(bilder), 0.0);
    for (int f = 1; f < bilder; ++f)
    {
        double s = 0.0;
        for (int k = 0; k < 3; ++k)
        {
            const double d = huellen[static_cast<std::size_t>(f) * kProfilBaender + k] -
                             huellen[static_cast<std::size_t>(f - 1) * kProfilBaender + k];
            if (d > 0.0) s += d;
        }
        fluss[static_cast<std::size_t>(f)] = s;
    }
    // Rohe Anschlaege: lokale Spitzen ueber dem gleitenden Mittel (±0,5 s).
    // Eine feste Schwelle taugt nicht — laute und leise Passagen unterscheiden
    // sich um Groessenordnungen.
    std::vector<unsigned char> anschlaege(static_cast<std::size_t>(bilder), 0);
    for (int f = 1; f < bilder; ++f)
    {
        const int von = std::max(0, f - 30);
        const int bis = std::min(bilder, f + 31);
        double mittel = 0.0;
        for (int i = von; i < bis; ++i) mittel += fluss[static_cast<std::size_t>(i)];
        mittel /= (bis - von);
        const bool spitze =
            fluss[static_cast<std::size_t>(f)] >= fluss[static_cast<std::size_t>(f - 1)] &&
            (f + 1 >= bilder ||
             fluss[static_cast<std::size_t>(f)] >= fluss[static_cast<std::size_t>(f + 1)]);
        if (spitze && fluss[static_cast<std::size_t>(f)] > mittel * 1.5 && mittel > 0.5)
        {
            anschlaege[static_cast<std::size_t>(f)] = 1;
        }
    }

    // --- Tempo: Autokorrelation des Flusses ---------------------------------------------
    // Die rohen Anschlaege allein taugen NICHT als Beat-Spur: ein Onset-Detektor
    // sieht auch Achtel und Hi-Hats, und der Median der Abstaende landet dann
    // beim Zwei- oder Vierfachen des Tempos (erster Lauf: 257 statt ~129 BPM).
    // Darum erst die Periode bestimmen, dann die Anschlaege auf deren Raster
    // legen — was daneben liegt, ist eine Unterteilung und faellt weg.
    const int pMin = 18;  // 200 BPM
    const int pMax = 60;  // 60 BPM
    int periode = 0;
    int phase = 0;
    double bestes = -1.0;
    for (int p = pMin; p <= pMax && p < bilder; ++p)
    {
        // Tempo-Prior um 120 BPM (Sigma 0,7 Oktaven). OHNE ihn gewinnt
        // zuverlaessig eine Oktave daneben: das mittlere Raster-Mittel steigt,
        // je duenner das Raster ist (jeder zweite Schlag trifft staerker), und
        // ein Onset-Detektor liefert zusaetzlich Achtel. Erster Lauf ohne
        // Prior: 257 BPM, danach mit reiner Mittelung: 63 BPM — beides
        // Oktavfehler zum tatsaechlichen ~129er-Takt.
        const double bpmP = 60.0 * kBildrate / p;
        const double oktaven = std::log2(bpmP / 120.0);
        const double prior = std::exp(-0.5 * (oktaven / 0.7) * (oktaven / 0.7));
        for (int ph = 0; ph < p; ++ph)
        {
            double summe = 0.0;
            int zahl = 0;
            for (int f = ph; f < bilder; f += p)
            {
                summe += fluss[static_cast<std::size_t>(f)];
                ++zahl;
            }
            if (zahl < 4) continue;
            const double wert = (summe / zahl) * prior;
            if (wert > bestes)
            {
                bestes = wert;
                periode = p;
                phase = ph;
            }
        }
    }

    // --- Beat-Spur: Rasterstellen, an denen die Musik auch wirklich anschlaegt ------------
    // Reines Raster waere nur ein schoeneres --beat-period. Die Kopplung an die
    // Anschlaege haelt die Struktur der Vorlage: waehrend eines Breaks bleiben
    // Rasterstellen leer, im Refrain sitzt jede.
    std::vector<unsigned char> beats(static_cast<std::size_t>(bilder), 0);
    if (periode > 0)
    {
        for (int f = phase; f < bilder; f += periode)
        {
            for (int d = -3; d <= 3; ++d)
            {
                const int i = f + d;
                if (i >= 0 && i < bilder && anschlaege[static_cast<std::size_t>(i)] != 0)
                {
                    beats[static_cast<std::size_t>(f)] = 1;
                    break;
                }
            }
        }
    }

    // --- Schleifenlaenge auf ein Vielfaches der Periode runden ---------------------------
    int schleife = bilder;
    if (periode > 0) schleife = std::max(periode, (bilder / periode) * periode);

    // --- Schreiben ----------------------------------------------------------------------
    QFile datei(zielDatei);
    if (!datei.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        return scheitern(QStringLiteral("nicht schreibbar: %1").arg(zielDatei));

    const double bpm = (periode > 0) ? 60.0 * kBildrate / periode : 0.0;
    int beatZahl = 0;
    for (int f = 0; f < schleife; ++f) beatZahl += beats[static_cast<std::size_t>(f)];

    QTextStream aus(&datei);
    aus.setEncoding(QStringConverter::Utf8);
    aus << "/**\n"
        << " ****************************************************************************************\n"
        << " * @file   MusikProfil.hpp\n"
        << " * @brief  ERZEUGTE DATEI — Huellkurven-Profil fuer das Musik-Muster (S74)\n"
        << " *\n"
        << " * @details\n"
        << " * Nicht von Hand aendern. Neu erzeugen mit:\n"
        << " *   AvsStandalone --audio-datei <datei> --audio-start <sek>\n"
        << " *                 --audio-profil-schreiben <ziel.hpp> --audio-profil-dauer <sek>\n"
        << " *\n"
        << " * Diese Datei enthaelt KEINE Aufnahme — nur " << kProfilBaender
        << " Bandhuellkurven je Bild und\n"
        << " * eine Beat-Spur. Daraus synthetisiert SynthAudio.hpp ein Pruefsignal mit der\n"
        << " * Dynamik der Vorlage. Zurueckrechnen laesst sich die Vorlage daraus nicht.\n"
        << " *\n"
        << " * Quelle:      " << m_quelle << "\n"
        << " * Ausschnitt:  ab " << QString::number(m_startRahmen / kAbtastrate, 'f', 1)
        << " s, " << QString::number(schleife / kBildrate, 'f', 2) << " s Schleife ("
        << schleife << " Bilder)\n"
        << " * Schlaege:    " << beatZahl << " (~" << QString::number(bpm, 'f', 1) << " BPM)\n"
        << " ****************************************************************************************\n"
        << " */\n\n"
        << "#ifndef LUMI_MUSIK_PROFIL_HPP\n"
        << "#define LUMI_MUSIK_PROFIL_HPP\n\n"
        << "#include \"SynthAudio.hpp\"\n\n"
        << "namespace lumi\n{\nnamespace synth\n{\n\n"
        << "inline const MusikProfil& eingebautesProfil()\n{\n"
        << "    static const float mitten[] = {";
    for (int k = 0; k < kProfilBaender; ++k)
    {
        aus << (k > 0 ? ", " : "")
            << QString::number(mitten[static_cast<std::size_t>(k)], 'f', 1) << "f";
    }
    aus << "};\n\n    static const unsigned char huellen[] = {\n";
    for (int f = 0; f < schleife; ++f)
    {
        aus << "        ";
        for (int k = 0; k < kProfilBaender; ++k)
        {
            aus << huellen[static_cast<std::size_t>(f) * kProfilBaender + k] << ",";
        }
        aus << "\n";
    }
    aus << "    };\n\n    static const unsigned char beats[] = {\n";
    for (int f = 0; f < schleife; ++f)
    {
        if (f % 60 == 0) aus << "        ";
        aus << beats[static_cast<std::size_t>(f)] << ",";
        if (f % 60 == 59) aus << "\n";
    }
    if (schleife % 60 != 0) aus << "\n";
    aus << "    };\n\n"
        << "    static const MusikProfil profil = {" << kProfilBaender << ", " << schleife
        << ", mitten, huellen, beats,\n"
        << "                                        \"" << m_quelle << " ab "
        << QString::number(m_startRahmen / kAbtastrate, 'f', 1) << " s\"};\n"
        << "    return profil;\n}\n\n"
        << "}  // namespace synth\n}  // namespace lumi\n\n"
        << "#endif  // LUMI_MUSIK_PROFIL_HPP\n";
    aus.flush();
    datei.close();

    std::printf("[Profil] %s: %d Bilder (%.2f s), %d Schlaege (~%.1f BPM), %d Baender\n",
                qPrintable(QFileInfo(zielDatei).fileName()), schleife, schleife / kBildrate,
                beatZahl, bpm, kProfilBaender);
    std::fflush(stdout);
    return true;
}

// =============================================================================================
// PcmGeber
// =============================================================================================

PcmGeber::PcmGeber(const AudioDateiQuelle& quelle)
    : m_quelle(&quelle)
    , m_pos(quelle.startRahmen() * 2)
{
}

qint64 PcmGeber::readData(char* daten, qint64 maxGroesse)
{
    const std::vector<float>& pcm = m_quelle->pcm();
    if (pcm.empty() || maxGroesse <= 0) return 0;

    const qint64 samples = std::min<qint64>(maxGroesse / static_cast<qint64>(sizeof(float)),
                                            static_cast<qint64>(pcm.size()));
    if (samples <= 0) return 0;

    auto* ziel = reinterpret_cast<float*>(daten);
    for (qint64 i = 0; i < samples; ++i)
    {
        if (m_pos >= static_cast<std::int64_t>(pcm.size()))
        {
            m_pos = m_quelle->startRahmen() * 2;  // umlaufend, wie die Bild-Zufuhr
        }
        ziel[i] = pcm[static_cast<std::size_t>(m_pos++)];
    }
    return samples * static_cast<qint64>(sizeof(float));
}

}  // namespace lumi::werkzeug
