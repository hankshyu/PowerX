//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/27/2025 09:47:32
//  Module Name:        uBump.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A data structure of storing micro Bump on interposers.
//                      Support grid data structure in pinMap for neighbor finding 
//                      and instance look-ups
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//
/////////////////////////////////////////////////////////////////////////////////

#ifndef __UBUMP_H__
#define __UBUMP_H__

// Dependencies
// 1. C++ STL:
#include <string>
#include <vector>
#include <unordered_map>

// 2. Boost Library:

// 3. Texo Library:
#include "rectangle.hpp"
#include "technology.hpp"
#include "signalType.hpp"
#include "ballOut.hpp"
#include "pinMap.hpp"



class UBump: public PinMap{
private:
    Rectangle m_InterposerSizeRectangle;
    std::vector<BallOut *> m_allBallouts[BALLOUT_ROTATION_COUNT];

public:
    std::unordered_map<std::string, Rectangle> instanceToRectangleMap;
    std::unordered_map<std::string, BallOut *> instanceToBallOutMap;
    std::unordered_map<std::string, BallOutRotation> instanceToRotationMap;


    UBump();
    explicit UBump(const std::string &fileName);
    ~UBump();

    friend bool visualiseUBump(const UBump &uBump, const Technology &tch, const std::string &filePath);

};

#endif // __UBUMP_H__
