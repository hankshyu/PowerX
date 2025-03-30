//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        02/28/2025 23:04:29
//  Module Name:        signalType.hpp
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

#ifndef __SIGNALTYPE_H__
#define __SIGNALTYPE_H__

// Dependencies
// 1. C++ STL:
#include <cstdint>
#include <string>
#include <ostream>

// 2. Boost Library:

// 3. Texo Library:

enum class SignalType : uint8_t{
    
    EMPTY = 0,
    POWER_1 = 1, PWR_1 = 1, P1 = 1,
    POWER_2 = 2, PWR_2 = 2, P2 = 2,
    POWER_3 = 3, PWR_3 = 3, P3 = 3,
    POWER_4 = 4, PWR_4 = 4, P4 = 4,
    POWER_5 = 5, PWR_5 = 5, P5 = 5,
    POWER_6 = 6, PWR_6 = 6, P6 = 6,
    POWER_7 = 7, PWR_7 = 7, P7 = 7,
    POWER_8 = 8, PWR_8 = 8, P8 = 8,
    POWER_9 = 9, PWR_9 = 9, P9 = 9,
    POWER_10 = 10, PWR_10 = 10, P10 = 10,
    GROUND = 11, GND = 11,
    SIGNAL = 12, SIG = 12,
    OBSTACLE = 13, OBSTACLES = 13, OBST = 13,
    OVERLAP = 14,
    UNKNOWN = 15,
};

constexpr inline const char* to_string(SignalType st) {
    switch (st) {
        case SignalType::EMPTY:     return "EMPTY";
        case SignalType::POWER_1:   return "POWER_1";
        case SignalType::POWER_2:   return "POWER_2";
        case SignalType::POWER_3:   return "POWER_3";
        case SignalType::POWER_4:   return "POWER_4";
        case SignalType::POWER_5:   return "POWER_5";
        case SignalType::POWER_6:   return "POWER_6";
        case SignalType::POWER_7:   return "POWER_7";
        case SignalType::POWER_8:   return "POWER_8";
        case SignalType::POWER_9:   return "POWER_9";
        case SignalType::POWER_10:  return "POWER_10";
        case SignalType::GROUND:    return "GROUND";
        case SignalType::SIGNAL:    return "SIGNAL";
        case SignalType::OBSTACLE:  return "OBSTACLE";
        case SignalType::OVERLAP:   return "OVERLAP";
        case SignalType::UNKNOWN:   return "UNKNOWN";
        default:                    return "UNKNOWN";
    }
};

std::ostream& operator<<(std::ostream& os, SignalType st);
SignalType convertToSignalType (const std::string &str);
constexpr uint8_t toIdx(SignalType id) {return static_cast<uint8_t>(id);}

#endif // __SIGNALTYPE_H__