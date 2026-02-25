/**
 * @file Logger.hpp
 * @brief A simple logger class for accumulating and outputting debug log messages.
 */

#pragma once

#include "UtilsCommon.h"

/**
 * @brief A simple logger class that accumulates and outputs debug log messages.
 * @note Usage: Logger() << "Message part 1" << "Message part 2";
 */
class Logger {
public:
    Logger() = default;
    ~Logger() {
#ifdef _DEBUG
        std::cout << m_buffer.str() << std::endl;
#endif
    }

    template<typename T>
    Logger& operator<<(const T& msg) {
#ifdef _DEBUG
        m_buffer << msg;
#endif
        return *this;
    }
    Logger& operator<<(std::ostream& (*manip)(std::ostream&)) {
#ifdef _DEBUG
        manip(m_buffer);
#endif
        return *this;
    }

private:
    std::stringstream m_buffer; // Buffer to hold log messages
};
