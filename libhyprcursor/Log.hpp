#pragma once

enum eLogLevel {
    TRACE = 0,
    INFO,
    LOG,
    WARN,
    ERR,
    CRIT,
    NONE
};

#include <string>
#include <format>
#include <iostream>

namespace Debug {
    inline bool quiet   = false;
    inline bool verbose = false;

    template <typename... Args>
    void log(eLogLevel level, const std::string& fmt, Args&&... args) {

#ifndef HYPRLAND_DEBUG
        // don't log in release
        return;
#endif

        if (!verbose && level == TRACE)
            return;

        if (quiet)
            return;

        if (level != NONE) {
            std::cout << '[';

            switch (level) {
                case TRACE: std::cout << "TRACE"; break;
                case INFO: std::cout << "INFO"; break;
                case LOG: std::cout << "LOG"; break;
                case WARN: std::cout << "WARN"; break;
                case ERR: std::cout << "ERR"; break;
                case CRIT: std::cout << "CRITICAL"; break;
                default: break;
            }

            std::cout << "] ";
        }

        std::cout << std::vformat(fmt, std::make_format_args(args...)) << "\n";
    }
};