#pragma once

#include <iostream>
#include <fstream>
#include <format>
#include <string>



inline std::ofstream& getLogFile()
{
    static std::ofstream logFile("log.txt", std::ios::out | std::ios::trunc);
    return logFile;
}



template<typename... Args>
inline void logMsg(std::format_string<Args...> fmt, Args&&... args)
{
    getLogFile() << "LOG: "
        << std::format(fmt, std::forward<Args>(args)...)
        << '\n';
}

template<typename... Args>
inline void logWarning(std::format_string<Args...> fmt, Args&&... args)
{
    getLogFile() << "WARNING: "
        << std::format(fmt, std::forward<Args>(args)...)
        << '\n';
}

template<typename... Args>
inline void logError(std::format_string<Args...> fmt, Args&&... args)
{
    getLogFile() << "ERROR: "
        << std::format(fmt, std::forward<Args>(args)...)
        << '\n';
}

template<typename... Args>
inline void logCaughtException(std::format_string<Args...> fmt, Args&&... args)
{
    getLogFile() << "EXCEPTION CAUGHT: "
        << std::format(fmt, std::forward<Args>(args)...)
        << '\n';
}