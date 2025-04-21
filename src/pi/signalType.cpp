//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        02/28/2025 23:06:02
//  Module Name:        signalType.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        An enum class for signal types
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//
//////////////////////////////////////////////////////////////////////////////////｀

// Dependencies
// 1. C++ STL:
#include <cstdint>
#include <cctype>
#include <string>
#include <ostream>
#include <unordered_map>
#include <algorithm>

// 2. Boost Library:

// 3. Texo Library:
#include "signalType.hpp"

std::ostream& operator<<(std::ostream& os, SignalType st) {
    return os << to_string(st);
}

SignalType convertToSignalType (const std::string &str){
    
    // Convert to uppercase for case-insensitive comparison
    std::string input = str;
    transform(input.begin(), input.end(), input.begin(), [](unsigned char c){ return std::toupper(c); });

    // Remove prefix if exists
    std::string prefix1 = "SIGNALTYPE::";
    std::string prefix2 = "SIGNALTYPE_";

    if (input.rfind(prefix1, 0) == 0) {
        input = input.substr(prefix1.length());
    } else if (input.rfind(prefix2, 0) == 0) {
        input = input.substr(prefix2.length());
    }

    static const std::unordered_map<std::string, SignalType> strToSignalTypeMap = {
        {"EMPTY", SignalType::EMPTY},
        {"POWER_1", SignalType::POWER_1}, {"PWR_1", SignalType::PWR_1}, {"P1", SignalType::P1},
        {"POWER_2", SignalType::POWER_2}, {"PWR_2", SignalType::PWR_2}, {"P2", SignalType::P2},
        {"POWER_3", SignalType::POWER_3}, {"PWR_3", SignalType::PWR_3}, {"P3", SignalType::P3},
        {"POWER_4", SignalType::POWER_4}, {"PWR_4", SignalType::PWR_4}, {"P4", SignalType::P4},
        {"POWER_5", SignalType::POWER_5}, {"PWR_5", SignalType::PWR_5}, {"P5", SignalType::P5},
        {"POWER_6", SignalType::POWER_6}, {"PWR_6", SignalType::PWR_6}, {"P6", SignalType::P6},
        {"POWER_7", SignalType::POWER_7}, {"PWR_7", SignalType::PWR_7}, {"P7", SignalType::P7},
        {"POWER_8", SignalType::POWER_8}, {"PWR_8", SignalType::PWR_8}, {"P8", SignalType::P8},
        {"POWER_9", SignalType::POWER_9}, {"PWR_9", SignalType::PWR_9}, {"P9", SignalType::P9},
        {"POWER_10", SignalType::POWER_10}, {"PWR_10", SignalType::PWR_10}, {"P10", SignalType::P10},
        {"GROUND", SignalType::GROUND}, {"GND", SignalType::GND},
        {"SIGNAL", SignalType::SIGNAL}, {"SIG", SignalType::SIG},
        {"OBSTACLE", SignalType::OBSTACLE}, {"OBSTACLES", SignalType::OBSTACLES}, {"OBST", SignalType::OBST},
        {"OVERLAP", SignalType::OVERLAP},
        {"UNKNOWN", SignalType::UNKNOWN}
    };

    std::unordered_map<std::string, SignalType>::const_iterator cit = strToSignalTypeMap.find(input);
    return cit != strToSignalTypeMap.end() ? cit->second : SignalType::UNKNOWN;
}
