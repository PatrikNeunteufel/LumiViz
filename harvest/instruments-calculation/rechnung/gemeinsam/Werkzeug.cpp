/**
 * @file Werkzeug.cpp
 * @brief Gemeinsam: WAV, Pegel, Abdruck, Rechenzeit, Pool
 */
#include "gemeinsam/Werkzeug.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstring>
#include <fstream>
#include <numbers>
#include <thread>

namespace ic
{

namespace
{

void schreibe32(std::ofstream& aus, std::uint32_t w)
{
    const char b[4] = {static_cast<char>(w & 0xff), static_cast<char>((w >> 8) & 0xff), static_cast<char>((w >> 16) & 0xff),
                       static_cast<char>((w >> 24) & 0xff)};
    aus.write(b, 4);
}

void schreibe16(std::ofstream& aus, std::uint16_t w)
{
    const char b[2] = {static_cast<char>(w & 0xff), static_cast<char>((w >> 8) & 0xff)};
    aus.write(b, 2);
}

std::uint32_t lies32(const unsigned char* p)
{
    return static_cast<std::uint32_t>(p[0]) | (static_cast<std::uint32_t>(p[1]) << 8) | (static_cast<std::uint32_t>(p[2]) << 16) |
           (static_cast<std::uint32_t>(p[3]) << 24);
}

std::uint16_t lies16(const unsigned char* p)
{
    return static_cast<std::uint16_t>(p[0] | (p[1] << 8));
}

double filtere(const Biquad& f, double x, double& z1, double& z2)
{
    // direkte Form II transponiert
    const double y = f.b0 * x + z1;
    z1 = f.b1 * x - f.a1 * y + z2;
    z2 = f.b2 * x - f.a2 * y;
    return y;
}

// --- SHA-256 nach FIPS 180-4 ---
constexpr std::array<std::uint32_t, 64> kK = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01,
    0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
    0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116, 0x1e376c08,
    0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

std::uint32_t dreh(std::uint32_t x, int n)
{
    return (x >> n) | (x << (32 - n));
}

void block(std::array<std::uint32_t, 8>& h, const std::uint8_t* p)
{
    std::array<std::uint32_t, 64> w{};
    for (int i = 0; i < 16; ++i)
        w[i] = (static_cast<std::uint32_t>(p[4 * i]) << 24) | (static_cast<std::uint32_t>(p[4 * i + 1]) << 16) |
               (static_cast<std::uint32_t>(p[4 * i + 2]) << 8) | static_cast<std::uint32_t>(p[4 * i + 3]);
    for (int i = 16; i < 64; ++i)
    {
        const std::uint32_t s0 = dreh(w[i - 15], 7) ^ dreh(w[i - 15], 18) ^ (w[i - 15] >> 3);
        const std::uint32_t s1 = dreh(w[i - 2], 17) ^ dreh(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }
    std::uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];
    for (int i = 0; i < 64; ++i)
    {
        const std::uint32_t s1 = dreh(e, 6) ^ dreh(e, 11) ^ dreh(e, 25);
        const std::uint32_t ch = (e & f) ^ (~e & g);
        const std::uint32_t t1 = hh + s1 + ch + kK[i] + w[i];
        const std::uint32_t s0 = dreh(a, 2) ^ dreh(a, 13) ^ dreh(a, 22);
        const std::uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        const std::uint32_t t2 = s0 + maj;
        hh = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }
    h[0] += a;
    h[1] += b;
    h[2] += c;
    h[3] += d;
    h[4] += e;
    h[5] += f;
    h[6] += g;
    h[7] += hh;
}

}  // namespace

bool schreibeWav(const std::string& pfad, std::span<const float> werte, int rate, int kanaele)
{
    std::ofstream aus(pfad, std::ios::binary);
    if (!aus) return false;
    const auto daten = static_cast<std::uint32_t>(werte.size() * 4);
    aus.write("RIFF", 4);
    schreibe32(aus, 36 + daten);
    aus.write("WAVE", 4);
    aus.write("fmt ", 4);
    schreibe32(aus, 16);
    schreibe16(aus, 3);  // IEEE float
    schreibe16(aus, static_cast<std::uint16_t>(kanaele));
    schreibe32(aus, static_cast<std::uint32_t>(rate));
    schreibe32(aus, static_cast<std::uint32_t>(rate * kanaele * 4));
    schreibe16(aus, static_cast<std::uint16_t>(kanaele * 4));
    schreibe16(aus, 32);
    aus.write("data", 4);
    schreibe32(aus, daten);
    for (const float w : werte)
    {
        std::uint32_t bits = 0;
        std::memcpy(&bits, &w, 4);
        schreibe32(aus, bits);
    }
    return static_cast<bool>(aus);
}

bool leseWav(const std::string& pfad, std::vector<float>& werte, int& rate, int& kanaele)
{
    std::ifstream ein(pfad, std::ios::binary);
    if (!ein) return false;
    std::vector<unsigned char> alles((std::istreambuf_iterator<char>(ein)), std::istreambuf_iterator<char>());
    if (alles.size() < 12 || std::memcmp(alles.data(), "RIFF", 4) != 0 || std::memcmp(alles.data() + 8, "WAVE", 4) != 0)
        return false;
    int format = 0;
    int bits = 0;
    std::size_t pos = 12;
    while (pos + 8 <= alles.size())
    {
        const std::uint32_t groesse = lies32(alles.data() + pos + 4);
        const unsigned char* inhalt = alles.data() + pos + 8;
        if (pos + 8 + groesse > alles.size()) return false;
        if (std::memcmp(alles.data() + pos, "fmt ", 4) == 0 && groesse >= 16)
        {
            format = lies16(inhalt);
            kanaele = lies16(inhalt + 2);
            rate = static_cast<int>(lies32(inhalt + 4));
            bits = lies16(inhalt + 14);
            if (format == 0xFFFE && groesse >= 26) format = lies16(inhalt + 24);  // WAVE_FORMAT_EXTENSIBLE
        }
        else if (std::memcmp(alles.data() + pos, "data", 4) == 0)
        {
            werte.clear();
            const int bytes = bits / 8;
            if (bytes <= 0) return false;
            for (std::size_t i = 0; i + static_cast<std::size_t>(bytes) <= groesse; i += static_cast<std::size_t>(bytes))
            {
                const unsigned char* p = inhalt + i;
                if (format == 3 && bits == 32)
                {
                    const std::uint32_t b = lies32(p);
                    float f = 0.0f;
                    std::memcpy(&f, &b, 4);
                    werte.push_back(f);
                }
                else if (format == 1 && bits == 16)
                    werte.push_back(static_cast<float>(static_cast<std::int16_t>(lies16(p)) / 32768.0));
                else if (format == 1 && bits == 24)
                {
                    std::int32_t v = p[0] | (p[1] << 8) | (p[2] << 16);
                    if (v & 0x800000) v -= 0x1000000;
                    werte.push_back(static_cast<float>(v / 8388608.0));
                }
                else
                    return false;
            }
            return true;
        }
        pos += 8 + groesse + (groesse & 1);
    }
    return false;
}

double spitze(std::span<const float> werte)
{
    double s = 0.0;
    for (const float w : werte) s = std::max(s, static_cast<double>(std::abs(w)));
    return s;
}

double rms(std::span<const float> werte)
{
    if (werte.empty()) return 0.0;
    double s = 0.0;
    for (const float w : werte) s += static_cast<double>(w) * w;
    return std::sqrt(s / static_cast<double>(werte.size()));
}

double dB(double x)
{
    return x > 0.0 ? 20.0 * std::log10(x) : -300.0;
}

Biquad kFilterKugel(int rate)
{
    const double f0 = 1681.974450955533;
    const double g = 3.999843853973347;
    const double q = 0.7071752369554196;
    const double k = std::tan(std::numbers::pi * f0 / rate);
    const double vh = std::pow(10.0, g / 20.0);
    const double vb = std::pow(vh, 0.4996667741545416);
    const double a0 = 1.0 + k / q + k * k;
    Biquad f;
    f.b0 = (vh + vb * k / q + k * k) / a0;
    f.b1 = 2.0 * (k * k - vh) / a0;
    f.b2 = (vh - vb * k / q + k * k) / a0;
    f.a1 = 2.0 * (k * k - 1.0) / a0;
    f.a2 = (1.0 - k / q + k * k) / a0;
    return f;
}

Biquad kFilterHochpass(int rate)
{
    const double f0 = 38.13547087602444;
    const double q = 0.5003270373238773;
    const double k = std::tan(std::numbers::pi * f0 / rate);
    const double a0 = 1.0 + k / q + k * k;
    Biquad f;
    f.b0 = 1.0;
    f.b1 = -2.0;
    f.b2 = 1.0;
    f.a1 = 2.0 * (k * k - 1.0) / a0;
    f.a2 = (1.0 - k / q + k * k) / a0;
    return f;
}

double lufsNaeherung(std::span<const float> mono, int rate)
{
    if (mono.empty() || rate <= 0) return -300.0;
    const Biquad kugel = kFilterKugel(rate);
    const Biquad hoch = kFilterHochpass(rate);
    double z1 = 0.0, z2 = 0.0, w1 = 0.0, w2 = 0.0, summe = 0.0;
    for (const float x : mono)
    {
        const double y = filtere(hoch, filtere(kugel, x, z1, z2), w1, w2);
        summe += y * y;
    }
    const double mittel = summe / static_cast<double>(mono.size());
    return mittel > 0.0 ? -0.691 + 10.0 * std::log10(mittel) : -300.0;
}

double lautheit(std::span<const float> mono, int rate)
{
    if (mono.empty() || rate <= 0) return -300.0;
    const Biquad kugel = kFilterKugel(rate);
    const Biquad hoch = kFilterHochpass(rate);
    std::vector<double> quadrat(mono.size());
    double z1 = 0.0, z2 = 0.0, w1 = 0.0, w2 = 0.0;
    for (std::size_t i = 0; i < mono.size(); ++i)
    {
        const double y = filtere(hoch, filtere(kugel, mono[i], z1, z2), w1, w2);
        quadrat[i] = y * y;
    }
    const auto lufs = [](double mittel) { return mittel > 0.0 ? -0.691 + 10.0 * std::log10(mittel) : -300.0; };
    const auto block = static_cast<std::size_t>(std::llround(0.4 * rate));
    const auto schritt = static_cast<std::size_t>(std::llround(0.1 * rate));
    if (mono.size() < block)
    {
        double summe = 0.0;
        for (const double q : quadrat) summe += q;
        return lufs(summe / static_cast<double>(quadrat.size()));
    }
    // Mittel je Block ueber eine laufende Summe der Quadrate
    std::vector<double> kumuliert(quadrat.size() + 1, 0.0);
    for (std::size_t i = 0; i < quadrat.size(); ++i) kumuliert[i + 1] = kumuliert[i] + quadrat[i];
    std::vector<double> bloecke;
    for (std::size_t ab = 0; ab + block <= quadrat.size(); ab += schritt)
        bloecke.push_back((kumuliert[ab + block] - kumuliert[ab]) / static_cast<double>(block));
    const auto mittelUeber = [&](double grenzeLufs) {
        double summe = 0.0;
        std::size_t n = 0;
        for (const double b : bloecke)
            if (lufs(b) > grenzeLufs)
            {
                summe += b;
                ++n;
            }
        return n > 0 ? summe / static_cast<double>(n) : 0.0;
    };
    const double absolut = mittelUeber(-70.0);
    if (absolut <= 0.0) return -300.0;
    return lufs(mittelUeber(lufs(absolut) - 10.0));
}

std::vector<double> dbJeBlock(std::span<const float> mono, int rate, double blockS)
{
    std::vector<double> db;
    const auto n = static_cast<std::size_t>(std::llround(blockS * rate));
    if (n == 0) return db;
    for (std::size_t ab = 0; ab < mono.size(); ab += n) db.push_back(dB(rms(mono.subspan(ab, std::min(n, mono.size() - ab)))));
    return db;
}

std::string sha256(std::span<const std::uint8_t> bytes)
{
    std::array<std::uint32_t, 8> h = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
    const std::size_t ganz = bytes.size() / 64;
    for (std::size_t i = 0; i < ganz; ++i) block(h, bytes.data() + 64 * i);
    std::array<std::uint8_t, 128> rest{};
    const std::size_t r = bytes.size() - 64 * ganz;
    if (r > 0) std::memcpy(rest.data(), bytes.data() + 64 * ganz, r);
    rest[r] = 0x80;
    const std::size_t laenge = r + 1 + 8 <= 64 ? 64 : 128;
    const std::uint64_t bits = static_cast<std::uint64_t>(bytes.size()) * 8;
    for (int i = 0; i < 8; ++i) rest[laenge - 1 - i] = static_cast<std::uint8_t>((bits >> (8 * i)) & 0xff);
    block(h, rest.data());
    if (laenge == 128) block(h, rest.data() + 64);
    static const char* ziffern = "0123456789abcdef";
    std::string hex;
    for (const std::uint32_t w : h)
        for (int i = 7; i >= 0; --i) hex.push_back(ziffern[(w >> (4 * i)) & 0xf]);
    return hex;
}

Sha256::Sha256() : m_h{0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19}, m_puffer{} {}

void Sha256::dazu(const std::uint8_t* daten, std::size_t n)
{
    std::array<std::uint32_t, 8> h;
    std::copy(m_h, m_h + 8, h.begin());
    m_laenge += n;
    while (n > 0)
    {
        const std::size_t nimm = std::min(n, 64 - m_imPuffer);
        std::memcpy(m_puffer + m_imPuffer, daten, nimm);
        m_imPuffer += nimm;
        daten += nimm;
        n -= nimm;
        if (m_imPuffer == 64)
        {
            block(h, m_puffer);
            m_imPuffer = 0;
        }
    }
    std::copy(h.begin(), h.end(), m_h);
}

std::string Sha256::fertig()
{
    std::array<std::uint32_t, 8> h;
    std::copy(m_h, m_h + 8, h.begin());
    std::array<std::uint8_t, 128> rest{};
    std::memcpy(rest.data(), m_puffer, m_imPuffer);
    rest[m_imPuffer] = 0x80;
    const std::size_t laenge = m_imPuffer + 1 + 8 <= 64 ? 64 : 128;
    const std::uint64_t bits = m_laenge * 8;
    for (int i = 0; i < 8; ++i) rest[laenge - 1 - static_cast<std::size_t>(i)] = static_cast<std::uint8_t>((bits >> (8 * i)) & 0xff);
    block(h, rest.data());
    if (laenge == 128) block(h, rest.data() + 64);
    static const char* ziffern = "0123456789abcdef";
    std::string hex;
    for (const std::uint32_t w : h)
        for (int i = 7; i >= 0; --i) hex.push_back(ziffern[(w >> (4 * i)) & 0xf]);
    return hex;
}

std::string abdruck(std::span<const float> werte)
{
    std::vector<std::uint8_t> bytes(werte.size() * 4);
    for (std::size_t i = 0; i < werte.size(); ++i)
    {
        std::uint32_t b = 0;
        std::memcpy(&b, &werte[i], 4);
        for (int k = 0; k < 4; ++k) bytes[4 * i + static_cast<std::size_t>(k)] = static_cast<std::uint8_t>((b >> (8 * k)) & 0xff);
    }
    return sha256(bytes);
}

double echtzeitAnteil(const std::function<void()>& rechne, double tonDauerS, int wiederholungen)
{
    if (tonDauerS <= 0.0 || wiederholungen <= 0) return 0.0;
    double summe = 0.0;
    for (int i = 0; i < wiederholungen; ++i)
    {
        const Stoppuhr uhr;
        rechne();
        summe += uhr.sekunden();
    }
    return summe / wiederholungen / tonDauerS;
}

void parallelJe(std::size_t anzahl, const std::function<void(std::size_t)>& aufgabe, unsigned threads)
{
    if (anzahl == 0) return;
    if (threads == 0) threads = std::clamp(std::thread::hardware_concurrency(), 1u, 11u);
    threads = static_cast<unsigned>(std::min<std::size_t>(threads, anzahl));
    if (threads <= 1)
    {
        for (std::size_t i = 0; i < anzahl; ++i) aufgabe(i);
        return;
    }
    std::atomic<std::size_t> naechste{0};
    std::vector<std::thread> pool;
    for (unsigned t = 0; t < threads; ++t)
        pool.emplace_back([&] {
            for (std::size_t i = naechste++; i < anzahl; i = naechste++) aufgabe(i);
        });
    for (std::thread& t : pool) t.join();
}

}  // namespace ic
