/**
 * @file Abgleich.cpp
 * @brief Composer: Modell des Dialogs Match loudness (§9g, G5)
 */

#include "composer/Abgleich.hpp"

#include <QFileInfo>

#include <algorithm>
#include <cmath>

namespace lumi::composer
{

QString AbgleichZeile::name() const
{
    return QFileInfo(datei).completeBaseName();
}

std::vector<AbgleichZeile> Abgleich::lies(const Komposition& komposition)
{
    std::vector<AbgleichZeile> zeilen;
    for (std::size_t k = 0; k < komposition.kanaele.size(); ++k)
    {
        const Kanal& kanal = komposition.kanaele[k];
        if (kanal.art != KanalArt::Audio) continue;
        for (std::size_t c = 0; c < kanal.clips.size(); ++c)
        {
            const Clip& clip = kanal.clips[c];
            AbgleichZeile z;
            z.kanal = static_cast<int>(k);
            z.clip = static_cast<int>(c);
            z.datei = clip.datei;
            z.loop = istLoop(clip.datei);
            // ein Loop: ein Durchlauf, gleich wo der Clip beginnt (L3b a)
            z.einsatzMs = z.loop ? 0 : clip.einsatzMs;
            z.dauerMs = z.loop ? 0 : clip.dauerMs();
            z.kanalName = kanal.name;
            z.farbe = kanal.farbe;
            z.spielt = kanal.an && clip.an;
            z.gainVorherDb = clip.gainDb;
            z.gainDb = clip.gainDb;
            zeilen.push_back(std::move(z));
        }
    }
    return zeilen;
}

Abgleich::Abgleich(const Komposition& komposition) : m_zeilen(lies(komposition)) {}

int Abgleich::zeileVon(int kanal, int clip) const
{
    for (std::size_t i = 0; i < m_zeilen.size(); ++i)
        if (m_zeilen[i].kanal == kanal && m_zeilen[i].clip == clip) return static_cast<int>(i);
    return -1;
}

void Abgleich::setzeZiel(int zeile)
{
    m_ziel = zeile >= 0 && zeile < static_cast<int>(m_zeilen.size()) ? zeile : -1;
}

int Abgleich::lauteste() const
{
    int beste = -1;
    for (std::size_t i = 0; i < m_zeilen.size(); ++i)
    {
        const auto& m = m_zeilen[i].messung;
        if (!m.has_value() || m->stille) continue;
        if (beste < 0 || m->lufs > m_zeilen[static_cast<std::size_t>(beste)].messung->lufs) beste = static_cast<int>(i);
    }
    return beste;
}

std::optional<double> Abgleich::vorschlag(int zeile) const
{
    if (zeile < 0 || zeile >= static_cast<int>(m_zeilen.size())) return std::nullopt;
    const auto& eigen = m_zeilen[static_cast<std::size_t>(zeile)].messung;
    if (!eigen.has_value() || eigen->stille) return std::nullopt;
    double bezug = m_festLufs;
    if (m_ziel >= 0)
    {
        if (zeile == m_ziel) return 0.0;  // der Bezugsclip bekommt 0 dB
        const auto& z = m_zeilen[static_cast<std::size_t>(m_ziel)].messung;
        if (!z.has_value() || z->stille) return std::nullopt;
        bezug = z->lufs;
    }
    const double roh = bezug - eigen->lufs;
    // auf ein Zehntel, wie das Feld Gain
    return std::clamp(std::round(roh * 10.0) / 10.0, kGainMinDb, kGainMaxDb);
}

std::optional<double> Abgleich::spitzeMitVorschlag(int zeile) const
{
    const std::optional<double> v = vorschlag(zeile);
    if (!v.has_value()) return std::nullopt;
    return m_zeilen[static_cast<std::size_t>(zeile)].messung->spitzeDb + *v;
}

int Abgleich::uebernimmVorschlaege()
{
    int anders = 0;
    for (std::size_t i = 0; i < m_zeilen.size(); ++i)
    {
        const std::optional<double> v = vorschlag(static_cast<int>(i));
        if (!v.has_value() || m_zeilen[i].gainDb == *v) continue;
        m_zeilen[i].gainDb = *v;
        ++anders;
    }
    return anders;
}

Komposition Abgleich::mitGains(const Komposition& in, bool imDialog) const
{
    Komposition aus = in;
    for (const AbgleichZeile& z : m_zeilen)
    {
        if (z.kanal < 0 || z.kanal >= static_cast<int>(aus.kanaele.size())) continue;
        auto& clips = aus.kanaele[static_cast<std::size_t>(z.kanal)].clips;
        if (z.clip < 0 || z.clip >= static_cast<int>(clips.size())) continue;
        clips[static_cast<std::size_t>(z.clip)].gainDb = imDialog ? z.gainDb : z.gainVorherDb;
    }
    return aus;
}

bool Abgleich::geaendert() const
{
    return std::any_of(m_zeilen.begin(), m_zeilen.end(),
                       [](const AbgleichZeile& z) { return z.gainDb != z.gainVorherDb; });
}

std::vector<int> Abgleich::liesNeu(const Komposition& neu)
{
    const AbgleichZeile* zielVorher = m_ziel >= 0 ? &m_zeilen[static_cast<std::size_t>(m_ziel)] : nullptr;
    const AbgleichZeile zielKopie = zielVorher != nullptr ? *zielVorher : AbgleichZeile{};
    std::vector<AbgleichZeile> neue = lies(neu);
    std::vector<bool> vergeben(m_zeilen.size(), false);
    std::vector<int> messen;
    for (std::size_t i = 0; i < neue.size(); ++i)
    {
        AbgleichZeile& n = neue[i];
        // derselbe Clip: zuerst am selben Platz, sonst der erste freie
        int treffer = -1;
        for (std::size_t j = 0; j < m_zeilen.size(); ++j)
        {
            if (vergeben[j] || !m_zeilen[j].gleicherClip(n)) continue;
            if (treffer < 0 || (m_zeilen[j].kanal == n.kanal && m_zeilen[j].clip == n.clip)) treffer = static_cast<int>(j);
        }
        if (treffer < 0)
        {
            messen.push_back(static_cast<int>(i));
            continue;
        }
        vergeben[static_cast<std::size_t>(treffer)] = true;
        const AbgleichZeile& alt = m_zeilen[static_cast<std::size_t>(treffer)];
        // „vorher" ist, was die Komposition jetzt sagt (etwa nach Ctrl+Z); was
        // im Dialog geaendert war, bleibt im Dialog
        if (alt.gainDb != alt.gainVorherDb) n.gainDb = alt.gainDb;
        n.messung = alt.messung;
        n.gemessenFehlgeschlagen = alt.gemessenFehlgeschlagen;
        if (!n.messung.has_value() && !n.gemessenFehlgeschlagen) messen.push_back(static_cast<int>(i));
    }
    m_zeilen = std::move(neue);
    m_ziel = -1;
    if (zielVorher != nullptr)
    {
        for (std::size_t i = 0; i < m_zeilen.size(); ++i)
            if (m_zeilen[i].gleicherClip(zielKopie)) { m_ziel = static_cast<int>(i); break; }
        if (m_ziel < 0) m_ziel = lauteste();
    }
    return messen;
}

}  // namespace lumi::composer
