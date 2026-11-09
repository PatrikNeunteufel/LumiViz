/**
 * @file Json.cpp
 * @brief Gemeinsam: der kleine JSON-Leser und -Schreiber
 */
#include "gemeinsam/Json.hpp"

#include <cctype>
#include <charconv>
#include <cstdio>
#include <fstream>
#include <sstream>

namespace ic
{

namespace
{

class Leser
{
public:
    explicit Leser(const std::string& t) : m_t(t) {}

    bool wert(JsonWert& w)
    {
        leer();
        if (m_i >= m_t.size()) return fehler("Ende");
        const char c = m_t[m_i];
        if (c == '{') return objekt(w);
        if (c == '[') return liste(w);
        if (c == '"')
        {
            w.art = JsonWert::Art::Text;
            return text(w.text);
        }
        if (m_t.compare(m_i, 4, "true") == 0)
        {
            m_i += 4;
            w.art = JsonWert::Art::Wahrheit;
            w.wahrheit = true;
            return true;
        }
        if (m_t.compare(m_i, 5, "false") == 0)
        {
            m_i += 5;
            w.art = JsonWert::Art::Wahrheit;
            return true;
        }
        if (m_t.compare(m_i, 4, "null") == 0)
        {
            m_i += 4;
            w.art = JsonWert::Art::Null;
            return true;
        }
        return zahl(w);
    }

    bool ende()
    {
        leer();
        return m_i == m_t.size();
    }

    std::string meldung;

private:
    bool fehler(const char* was)
    {
        meldung = std::string(was) + " an Stelle " + std::to_string(m_i);
        return false;
    }

    void leer()
    {
        while (m_i < m_t.size() && std::isspace(static_cast<unsigned char>(m_t[m_i]))) ++m_i;
    }

    bool zahl(JsonWert& w)
    {
        const std::size_t ab = m_i;
        while (m_i < m_t.size() && (std::isdigit(static_cast<unsigned char>(m_t[m_i])) || m_t[m_i] == '-' || m_t[m_i] == '+' ||
                                    m_t[m_i] == '.' || m_t[m_i] == 'e' || m_t[m_i] == 'E'))
            ++m_i;
        if (m_i == ab) return fehler("unerwartetes Zeichen");
        double x = 0.0;
        const auto r = std::from_chars(m_t.data() + ab, m_t.data() + m_i, x);
        if (r.ec != std::errc() || r.ptr != m_t.data() + m_i) return fehler("Zahl");
        w.art = JsonWert::Art::Zahl;
        w.zahl = x;
        return true;
    }

    static void utf8(std::string& s, unsigned code)
    {
        if (code < 0x80)
            s.push_back(static_cast<char>(code));
        else if (code < 0x800)
        {
            s.push_back(static_cast<char>(0xC0 | (code >> 6)));
            s.push_back(static_cast<char>(0x80 | (code & 0x3F)));
        }
        else
        {
            s.push_back(static_cast<char>(0xE0 | (code >> 12)));
            s.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
            s.push_back(static_cast<char>(0x80 | (code & 0x3F)));
        }
    }

    bool text(std::string& s)
    {
        ++m_i;  // "
        s.clear();
        while (m_i < m_t.size() && m_t[m_i] != '"')
        {
            char c = m_t[m_i++];
            if (c == '\\' && m_i < m_t.size())
            {
                c = m_t[m_i++];
                switch (c)
                {
                    case 'n': s.push_back('\n'); break;
                    case 't': s.push_back('\t'); break;
                    case 'r': s.push_back('\r'); break;
                    case 'b': s.push_back('\b'); break;
                    case 'f': s.push_back('\f'); break;
                    case 'u':
                    {
                        if (m_i + 4 > m_t.size()) return fehler("\\u");
                        utf8(s, static_cast<unsigned>(std::stoul(m_t.substr(m_i, 4), nullptr, 16)));
                        m_i += 4;
                        break;
                    }
                    default: s.push_back(c);
                }
            }
            else
                s.push_back(c);
        }
        if (m_i >= m_t.size()) return fehler("Text ohne Ende");
        ++m_i;
        return true;
    }

    bool liste(JsonWert& w)
    {
        ++m_i;
        w.art = JsonWert::Art::Liste;
        leer();
        if (m_i < m_t.size() && m_t[m_i] == ']')
        {
            ++m_i;
            return true;
        }
        for (;;)
        {
            JsonWert e;
            if (!wert(e)) return false;
            w.liste.push_back(std::move(e));
            leer();
            if (m_i < m_t.size() && m_t[m_i] == ',')
            {
                ++m_i;
                continue;
            }
            if (m_i < m_t.size() && m_t[m_i] == ']')
            {
                ++m_i;
                return true;
            }
            return fehler("Liste");
        }
    }

    bool objekt(JsonWert& w)
    {
        ++m_i;
        w.art = JsonWert::Art::Objekt;
        leer();
        if (m_i < m_t.size() && m_t[m_i] == '}')
        {
            ++m_i;
            return true;
        }
        for (;;)
        {
            leer();
            if (m_i >= m_t.size() || m_t[m_i] != '"') return fehler("Schluessel");
            std::string schluessel;
            if (!text(schluessel)) return false;
            leer();
            if (m_i >= m_t.size() || m_t[m_i] != ':') return fehler(":");
            ++m_i;
            JsonWert e;
            if (!wert(e)) return false;
            w.schluessel.push_back(std::move(schluessel));
            w.objekt.push_back(std::move(e));
            leer();
            if (m_i < m_t.size() && m_t[m_i] == ',')
            {
                ++m_i;
                continue;
            }
            if (m_i < m_t.size() && m_t[m_i] == '}')
            {
                ++m_i;
                return true;
            }
            return fehler("Objekt");
        }
    }

    const std::string& m_t;
    std::size_t m_i = 0;
};

void textAlsJson(std::ostringstream& aus, const std::string& s)
{
    aus << '"';
    for (const char c : s)
    {
        if (c == '"' || c == '\\')
            aus << '\\' << c;
        else if (c == '\n')
            aus << "\\n";
        else
            aus << c;
    }
    aus << '"';
}

void schreibe(std::ostringstream& aus, const JsonWert& w, int einzug)
{
    const std::string innen(static_cast<std::size_t>(einzug + 2), ' ');
    const std::string aussen(static_cast<std::size_t>(einzug), ' ');
    switch (w.art)
    {
        case JsonWert::Art::Null: aus << "null"; break;
        case JsonWert::Art::Wahrheit: aus << (w.wahrheit ? "true" : "false"); break;
        case JsonWert::Art::Zahl:
        {
            char puffer[32];
            std::snprintf(puffer, sizeof puffer, "%.17g", w.zahl);
            aus << puffer;
            break;
        }
        case JsonWert::Art::Text: textAlsJson(aus, w.text); break;
        case JsonWert::Art::Liste:
        {
            aus << '[';
            for (std::size_t i = 0; i < w.liste.size(); ++i)
            {
                aus << (i ? ", " : "");
                schreibe(aus, w.liste[i], einzug + 2);
            }
            aus << ']';
            break;
        }
        case JsonWert::Art::Objekt:
        {
            aus << "{\n";
            for (std::size_t i = 0; i < w.objekt.size(); ++i)
            {
                aus << innen;
                textAlsJson(aus, w.schluessel[i]);
                aus << ": ";
                schreibe(aus, w.objekt[i], einzug + 2);
                aus << (i + 1 < w.objekt.size() ? ",\n" : "\n");
            }
            aus << aussen << '}';
            break;
        }
    }
}

}  // namespace

const JsonWert& JsonWert::operator[](const std::string& name) const
{
    static const JsonWert leer;
    for (std::size_t i = 0; i < schluessel.size(); ++i)
        if (schluessel[i] == name) return objekt[i];
    return leer;
}

bool JsonWert::hat(const std::string& name) const
{
    for (const std::string& k : schluessel)
        if (k == name) return true;
    return false;
}

void JsonWert::setze(std::string name, JsonWert wert)
{
    art = Art::Objekt;
    schluessel.push_back(std::move(name));
    objekt.push_back(std::move(wert));
}

bool leseJson(const std::string& text, JsonWert& wert, std::string* fehler)
{
    Leser leser(text);
    JsonWert w;
    if (!leser.wert(w) || !leser.ende())
    {
        if (fehler) *fehler = leser.meldung.empty() ? "Rest nach dem Wert" : leser.meldung;
        return false;
    }
    wert = std::move(w);
    return true;
}

bool leseJsonDatei(const std::string& pfad, JsonWert& wert, std::string* fehler)
{
    std::ifstream ein(pfad, std::ios::binary);
    if (!ein)
    {
        if (fehler) *fehler = "Datei nicht lesbar: " + pfad;
        return false;
    }
    std::stringstream puffer;
    puffer << ein.rdbuf();
    return leseJson(puffer.str(), wert, fehler);
}

std::string alsJson(const JsonWert& wert, int einzug)
{
    std::ostringstream aus;
    schreibe(aus, wert, einzug);
    return aus.str();
}

}  // namespace ic
