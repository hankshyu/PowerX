//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        04/06/2025 16:04:51
//  Module Name:        powerGrid.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A parent class that includes the bump holding data structures
//                      for ubump/C4, and the m5 and m7 grids
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//
//////////////////////////////////////////////////////////////////////////////////

#ifndef __POWERGRID_H__
#define __POWERGRID_H__

// Dependencies
// 1. C++ STL:
#include <vector>
#include <unordered_map>

// 2. Boost Library:

// 3. Texo Library:
#include "signalType.hpp"
#include "microBump.hpp"
#include "c4Bump.hpp"

class PowerGrid{
public:
    MicroBump uBump;
    C4Bump c4;

    len_t canvasWidth;
    len_t canvasHeight;
    std::vector<std::vector<SignalType>> canvasM5;
    std::vector<std::vector<SignalType>> canvasM7;

    const static std::unordered_map<SignalType, SignalType> defulatM5SigPadMap;
    const static std::unordered_map<SignalType, SignalType> defulatM7SigPadMap;

    PowerGrid(const std::string &fileName);
    void insertPinPads(const PinMap &pm, std::vector<std::vector<SignalType>> &canvas, const std::unordered_map<SignalType, SignalType> &padTypeMap);

    friend bool visualisePGM5(const PowerGrid &pg, const std::string &filePath, bool overlayOverlaps, bool overlayM5uBump, bool overlayM7C4);
    friend bool visualisePGM7(const PowerGrid &pg, const std::string &filePath, bool overlayOverlaps, bool overlayM5uBump, bool overlayM7C4);
    friend bool visualisePGOverlap(const PowerGrid &pg, const std::string &filePath, bool overlayM5uBump, bool overlayM7C4);
};

#endif // __POWERGRID_H__