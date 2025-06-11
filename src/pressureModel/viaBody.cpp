//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        06/07/2025 17:07:43
//  Module Name:        viaBody.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        An Object Representing a via on the pressure system
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
/////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:

// 2. Boost Library:

// 3. Texo Library:
#include "viaBody.hpp"


ViaBody::ViaBody(int uplayerIdx, int downlayerIdx, const FPoint &location)
    : upIdx(uplayerIdx), downIdx(downlayerIdx), location(location) {}

int ViaBody::getUpIdx() const {
    return upIdx;
}

int ViaBody::getDownIdx() const {
    return downIdx;
}

FPoint ViaBody::getLocation() const {
    return location;
}
