/**
 * @file Pruefsumme.cpp
 * @brief InstrumentCore: SHA-256 nach FIPS 180-4 (aus IC `rechnung/gemeinsam/Werkzeug.cpp` `bb7e6d6`, wortgleich;
 *        dazu die Eingabe als Text)
 */
#include "InstrumentCore/Pruefsumme.hpp"

#include <algorithm>
#include <array>
#include <cstring>
#include <vector>

namespace lumi::kern
{

namespace
{
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

void Sha256::dazu(std::string_view text)
{
    dazu(reinterpret_cast<const std::uint8_t*>(text.data()), text.size());
}

std::string sha256(std::string_view text)
{
    return sha256(std::span<const std::uint8_t>(reinterpret_cast<const std::uint8_t*>(text.data()), text.size()));
}

}  // namespace lumi::kern
