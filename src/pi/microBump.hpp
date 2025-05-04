//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/27/2025 09:47:32
//  Module Name:        microBump.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A data structure of storing micro Bump on interposers
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//  03/29/2025          Rename from uBump to microBump
//  05/04/2025          Change parent class from PinMap to ObjectArray
//
/////////////////////////////////////////////////////////////////////////////////

#ifndef __MICROBUMP_H__
#define __MICROBUMP_H__

// Dependencies
// 1. C++ STL:
#include <string>
#include <vector>
#include <unordered_map>

// 2. Boost Library:

// 3. Texo Library:
#include "cord.hpp"
#include "rectangle.hpp"
#include "technology.hpp"
#include "signalType.hpp"
#include "ballOut.hpp"
#include "objectArray.hpp"

class MicroBump: public ObjectArray{
private:
    Rectangle m_interposerSizeRectangle;
    std::vector<BallOut *> m_allBallouts[BALLOUT_ROTATION_COUNT];

public:
    std::unordered_set<SignalType> allSignalTypes;
    std::unordered_map<SignalType, std::unordered_set<Cord>> signalTypeToAllCords;

    std::unordered_map<SignalType, std::unordered_set<std::string>> signalTypeToInstances;
    
    std::unordered_map<std::string, Rectangle> instanceToRectangleMap;
    std::unordered_map<std::string, BallOut *> instanceToBallOutMap;
    std::unordered_map<std::string, BallOutRotation> instanceToRotationMap;

    MicroBump();
    explicit MicroBump(const std::string &fileName);
    ~MicroBump();

    friend bool visualiseMicroBump(const MicroBump &microBump, const Technology &tch, const std::string &filePath);

};

#endif // __MICROBUMP_H__
