//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/22/2022 14:02:43
//  Module Name:        powerPlane.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A data structure representing the power plane of the interposer
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//
//////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:
#include <iostream>
#include <cassert>
#include <string>

// 2. Boost Library:


// 3. Texo Library:
#include "powerPlane.hpp"
#include "pinout.hpp"
#include "ballout.hpp"
#include "cornerStitching.hpp"

PowerPlane::PowerPlane(const std::string &fileName): uBump(fileName), c4(fileName) {

    assert(uBump.getPinCountWidth() == c4.getPinCountWidth());
    assert(uBump.getPinCountHeight() == c4.getPinCountHeight());

    len_t planeWidth = uBump.getPinCountWidth() - 1;
    len_t planeHeight = uBump.getPinCountHeight() - 1;

    m5 = CornerStitching(planeWidth, planeHeight);
    m7 = CornerStitching(planeWidth, planeHeight);


}
