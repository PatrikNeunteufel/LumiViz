/**
 ****************************************************************************************
 * @file   JsonPresetParser.hpp
 * @brief  Minimal flat-JSON value extraction for module preset files (5.6)
 *
 * Consolidates the three hand-rolled preset parsers (AudioSourceModule,
 * SmoothingModule, ColorGradientModule). This is deliberately NOT a general
 * JSON parser — it covers the known, flat module-preset format written by the
 * modules themselves (scalar values plus simple/nested arrays), with the same
 * find-based semantics the individual parsers used.
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#pragma once

#include <fstream>
#include <optional>
#include <sstream>
#include <string>

namespace lumi::modules {

/**
 * @class JsonPresetParser
 * @brief Key-based value extraction from a module preset JSON document
 */
class JsonPresetParser
{
public:
    explicit JsonPresetParser(std::string content) : m_content(std::move(content)) {}

    /// @brief Load a preset file; nullopt if it cannot be opened
    [[nodiscard]] static std::optional<JsonPresetParser> fromFile(const std::string& filePath)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            return std::nullopt;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return JsonPresetParser(buffer.str());
    }

    [[nodiscard]] std::string getString(const std::string& key,
                                        const std::string& fallback = {}) const
    {
        const std::string searchKey = "\"" + key + "\":";
        size_t pos = m_content.find(searchKey);
        if (pos == std::string::npos) return fallback;
        pos = m_content.find('"', pos + searchKey.length());
        if (pos == std::string::npos) return fallback;
        const size_t end = m_content.find('"', pos + 1);
        if (end == std::string::npos) return fallback;
        return m_content.substr(pos + 1, end - pos - 1);
    }

    [[nodiscard]] float getFloat(const std::string& key, float fallback = 0.0f) const
    {
        const size_t pos = findValueStart(key);
        if (pos == std::string::npos) return fallback;
        try
        {
            return std::stof(m_content.substr(pos));
        }
        catch (...)
        {
            return fallback;
        }
    }

    [[nodiscard]] int getInt(const std::string& key, int fallback = 0) const
    {
        const size_t pos = findValueStart(key);
        if (pos == std::string::npos) return fallback;
        try
        {
            return std::stoi(m_content.substr(pos));
        }
        catch (...)
        {
            return fallback;
        }
    }

    [[nodiscard]] bool getBool(const std::string& key, bool fallback = false) const
    {
        const std::string searchKey = "\"" + key + "\":";
        const size_t pos = m_content.find(searchKey);
        if (pos == std::string::npos) return fallback;
        return m_content.find("true", pos) < m_content.find(',', pos);
    }

    /**
     * @brief Raw text of an array value without the outer brackets
     *
     * Handles nested arrays via bracket counting (e.g. gradient stops
     * "[[pos,r,g,b,a],...]"). Empty string if the key or brackets are missing.
     */
    [[nodiscard]] std::string getArrayContent(const std::string& key) const
    {
        const size_t keyPos = m_content.find("\"" + key + "\"");
        if (keyPos == std::string::npos) return {};
        const size_t arrStart = m_content.find('[', keyPos);
        if (arrStart == std::string::npos) return {};

        int bracketCount = 1;
        size_t arrEnd = arrStart + 1;
        while (arrEnd < m_content.size() && bracketCount > 0)
        {
            if (m_content[arrEnd] == '[') ++bracketCount;
            else if (m_content[arrEnd] == ']') --bracketCount;
            ++arrEnd;
        }
        if (bracketCount != 0) return {};

        return m_content.substr(arrStart + 1, arrEnd - arrStart - 2);
    }

private:
    /// Index of the first value character after "key": (skipping blanks), or npos
    [[nodiscard]] size_t findValueStart(const std::string& key) const
    {
        const std::string searchKey = "\"" + key + "\":";
        size_t pos = m_content.find(searchKey);
        if (pos == std::string::npos) return std::string::npos;
        pos += searchKey.length();
        while (pos < m_content.size()
               && (m_content[pos] == ' ' || m_content[pos] == '\t'))
        {
            ++pos;
        }
        return pos < m_content.size() ? pos : std::string::npos;
    }

    std::string m_content;
};

} // namespace lumi::modules
