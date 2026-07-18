/**
 ****************************************************************************************
 * @file   CommandResult.hpp
 * @brief  
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once
#include <string>

#include "../../common/Attributes.hpp"

namespace viz::core
{

    /**
     * @brief Result type for command operations.
     */
    struct CommandResult
    {
        bool ok{true};         ///< true if operation succeeded
        std::string message{}; ///< human-readable message
        int errorCode{0};      ///< 0 = success; >0 = domain-specific

        static VIZ_NODISCARD CommandResult Ok(std::string msg = {})
        {
            return {true, std::move(msg), 0};
        }
        static VIZ_NODISCARD CommandResult Fail(std::string msg, int code = 1)
        {
            return {false, std::move(msg), code};
        }
        VIZ_NODISCARD explicit operator bool() const noexcept
        {
            return ok;
        }
    };

} // namespace viz::core
