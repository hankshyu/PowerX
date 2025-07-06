//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        07/03/2025 15:08:40
//  Module Name:        dirFlags.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A bitmap system for directional flags
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//
//////////////////////////////////////////////////////////////////////////////////｀

#ifndef __DIRFLAG_H__
#define __DIRFLAG_H__

#include <cstdint>

typedef uint8_t DirFlag;

enum class DirFlagAxis : uint8_t {
    UP      = 0x01,
    DOWN    = 0x02,
    LEFT    = 0x04,
    RIGHT   = 0x08,
    TOP     = 0x10,
    BOTTOM  = 0x20
};

enum class DirFlagViaAxis : uint8_t {
    UPLL    = 0x01,
    UPUL    = 0x02,
    UPLR    = 0x04,
    UPUR    = 0x08,
    DOWNLL  = 0x10,
    DOWNUL  = 0x20,
    DOWNLR  = 0x40,
    DOWNUR  = 0x80
};

// Helper to convert enum classes to raw DirFlag
inline constexpr DirFlag toFlag(DirFlagAxis dir) {
    return static_cast<DirFlag>(dir);
}
inline constexpr DirFlag toFlag(DirFlagViaAxis dir) {
    return static_cast<DirFlag>(dir);
}

// Grouped masks
constexpr DirFlag DIRFLAG_EMPTY = 0x00;
constexpr DirFlag CELL_ALL_2D_DIR = toFlag(DirFlagAxis::UP) | toFlag(DirFlagAxis::DOWN) |toFlag(DirFlagAxis::LEFT) | toFlag(DirFlagAxis::RIGHT);
constexpr DirFlag CELL_ALL_3D_DIR = CELL_ALL_2D_DIR | toFlag(DirFlagAxis::TOP) | toFlag(DirFlagAxis::BOTTOM);

constexpr DirFlag VIA_ALL_UP_DIR = toFlag(DirFlagViaAxis::UPLL) | toFlag(DirFlagViaAxis::UPUL) | toFlag(DirFlagViaAxis::UPLR) | toFlag(DirFlagViaAxis::UPUR);
constexpr DirFlag VIA_ALL_DOWN_DIR = toFlag(DirFlagViaAxis::DOWNLL) | toFlag(DirFlagViaAxis::DOWNUL) | toFlag(DirFlagViaAxis::DOWNLR) | toFlag(DirFlagViaAxis::DOWNUR);
constexpr DirFlag VIA_ALL_DIR = VIA_ALL_UP_DIR | VIA_ALL_DOWN_DIR;

// DirFlags Utility Function
inline void addDirection(DirFlag& flag, DirFlagAxis dir) {
    flag |= toFlag(dir);
}
inline void addDirection(DirFlag& flag, DirFlagViaAxis dir) {
    flag |= toFlag(dir);
}

// Remove a direction from the flag
inline void removeDirection(DirFlag& flag, DirFlagAxis dir) {
    flag &= ~toFlag(dir);
}
inline void removeDirection(DirFlag& flag, DirFlagViaAxis dir) {
    flag &= ~toFlag(dir);
}

// Check if a direction is set
inline bool hasDirection(DirFlag flag, DirFlagAxis dir) {
    return (flag & toFlag(dir)) != DIRFLAG_EMPTY;
}
inline bool hasDirection(DirFlag flag, DirFlagViaAxis dir) {
    return (flag & toFlag(dir)) != DIRFLAG_EMPTY;
}

// Reset all directions
inline void clearAllDirections(DirFlag& flag) {
    flag = DIRFLAG_EMPTY;
}

#endif // __DIRFLAG_H__