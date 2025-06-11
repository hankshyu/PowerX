//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/22/2022 12:27:37
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

#ifndef __POWERPLANE_H__
#define __POWERPLANE_H__

// Dependencies
// 1. C++ STL:
#include <iostream>
#include <cassert>
#include <string>

// 2. Boost Library:


// 3. Texo Library:
#include "pinout.hpp"
#include "ballout.hpp"
#include "cornerStitching.hpp"

class PowerPlane{
public:
    Pinout uBump;
    CornerStitching m5;
    CornerStitching m7;
    Ballout c4;

    explicit PowerPlane(const std::string &fileName);

};

#endif // __POWERPLANE_H__

