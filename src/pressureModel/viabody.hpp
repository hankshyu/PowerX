//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        06/07/2025 16:56:06
//  Module Name:        viaBody.hpp
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

#ifndef __VIA_BODY_H__
#define __VIA_BODY_H__

// Dependencies
// 1. C++ STL:
#include <vector>
// 2. Boost Library:

// 3. Texo Library:
#include "signalType.hpp"

#include "fpoint.hpp"


class ViaBody{
private:
    int upIdx;
    int downIdx;
    FPoint location;

public:
    ViaBody(int uplayerIdx, int downlayerIdx, const FPoint &location);
    int getUpIdx();
    int getDownIdx();
    FPoint getLocation();

};

#endif // __VIA_BODY_H__