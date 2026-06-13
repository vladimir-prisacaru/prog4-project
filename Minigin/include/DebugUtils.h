#pragma once

#include <iostream>
#include <fstream>
#include <format>
#include <string>



inline std::ofstream& getLogFile()
{
    static std::ofstream logFile("log_latest.txt", std::ios::out | std::ios::trunc);
    return logFile;
}



template<typename... Args>
inline void logMsg([[maybe_unused]] std::format_string<Args...> fmt,
    [[maybe_unused]] Args&&... args)
{
    #ifdef _DEBUG

    getLogFile() << "LOG: "
        << std::format(fmt, std::forward<Args>(args)...)
        << '\n';

    #endif
}

template<typename... Args>
inline void logWarning([[maybe_unused]] std::format_string<Args...> fmt,
    [[maybe_unused]] Args&&... args)
{
    #ifdef _DEBUG

    getLogFile() << "WARNING: "
        << std::format(fmt, std::forward<Args>(args)...)
        << '\n';

    #endif
}

template<typename... Args>
inline void logError([[maybe_unused]] std::format_string<Args...> fmt,
    [[maybe_unused]] Args&&... args)
{
    #ifdef _DEBUG

    getLogFile() << "ERROR: "
        << std::format(fmt, std::forward<Args>(args)...)
        << '\n';

    #endif
}

template<typename... Args>
inline void logCaughtException([[maybe_unused]] std::format_string<Args...> fmt,
    [[maybe_unused]] Args&&... args)
{
    #ifdef _DEBUG

    getLogFile() << "EXCEPTION CAUGHT: "
        << std::format(fmt, std::forward<Args>(args)...)
        << '\n';

    #endif
}