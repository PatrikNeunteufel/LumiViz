/**
 ****************************************************************************************
 * @file   AvsParserReader.hpp
 * @brief  Bounds-checked little-endian cursor over a .avs config blob
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 *
 * @details
 * Mirrors the reading idioms of the original loaders (ref vis_avs, GET_INT +
 * "if (len-pos >= 4)" guards + C_RBASE::load_string) so decoders can be written
 * as 1:1 transcriptions of the respective load_config. Where the original reads
 * out of bounds into zeroed slack (e.g. the version-byte peek), this reader
 * returns 0 instead — observable behavior on valid files is identical.
 ****************************************************************************************
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace lumi::avs::detail {

class Reader
{
public:
    Reader(const std::uint8_t* data, std::size_t size) : m_data(data), m_size(size) {}

    [[nodiscard]] std::size_t pos() const { return m_pos; }
    [[nodiscard]] std::size_t remaining() const { return m_size - m_pos; }
    [[nodiscard]] bool atEnd() const { return m_pos >= m_size; }

    /// @brief Peek one byte without consuming (0 when out of bounds — zeroed slack)
    [[nodiscard]] std::uint8_t peekByte(std::size_t ahead = 0) const
    {
        return (m_pos + ahead < m_size) ? m_data[m_pos + ahead] : 0;
    }

    void skip(std::size_t n) { m_pos += (n <= remaining()) ? n : remaining(); }

    /// @brief Unconditional little-endian int32 (mirrors GET_INT; 0 when short)
    std::int32_t i32()
    {
        if (remaining() < 4)
        {
            m_pos = m_size;
            return 0;
        }
        const std::uint32_t v = static_cast<std::uint32_t>(m_data[m_pos]) |
                                (static_cast<std::uint32_t>(m_data[m_pos + 1]) << 8) |
                                (static_cast<std::uint32_t>(m_data[m_pos + 2]) << 16) |
                                (static_cast<std::uint32_t>(m_data[m_pos + 3]) << 24);
        m_pos += 4;
        return static_cast<std::int32_t>(v);
    }

    /// @brief Guarded field read: "if (len-pos >= 4) { x=GET_INT(); pos+=4; }"
    bool tryI32(std::int32_t& out)
    {
        if (remaining() < 4) return false;
        out = i32();
        return true;
    }

    /// @brief Raw byte run ("" if not enough data — nothing consumed then)
    [[nodiscard]] bool tryBytes(std::size_t n, const std::uint8_t*& out)
    {
        if (remaining() < n) return false;
        out = m_data + m_pos;
        m_pos += n;
        return true;
    }

    /**
     * @brief Length-prefixed string (mirrors C_RBASE::load_string)
     *
     * int32 size, then size bytes (which include the NUL terminator when written
     * by AVS). size<=0 or truncated data yields "" — the size field is consumed
     * either way, the payload only when valid.
     */
    std::string loadString()
    {
        if (remaining() < 4) { m_pos = m_size; return {}; }
        const std::int32_t size = i32();
        if (size <= 0 || remaining() < static_cast<std::size_t>(size)) return {};
        const char* begin = reinterpret_cast<const char*>(m_data + m_pos);
        m_pos += static_cast<std::size_t>(size);
        return toCString(begin, static_cast<std::size_t>(size));
    }

    /// @brief Fixed-size block treated as C string (old fixed-256/1024 formats)
    std::string fixedString(const std::uint8_t* block, std::size_t size) const
    {
        return toCString(reinterpret_cast<const char*>(block), size);
    }

private:
    /// Cut at the first NUL — AVS strings are C strings inside sized blocks.
    static std::string toCString(const char* begin, std::size_t maxLen)
    {
        std::size_t n = 0;
        while (n < maxLen && begin[n] != '\0') ++n;
        return std::string(begin, n);
    }

    const std::uint8_t* m_data = nullptr;
    std::size_t m_size = 0;
    std::size_t m_pos = 0;
};

} // namespace lumi::avs::detail
