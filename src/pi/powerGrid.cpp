//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        04/06/2025 16:10:52
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

// Dependencies
// 1. C++ STL:
#include <cassert>
#include <vector>
#include <map>

// 2. Boost Library:


// 3. Texo Library:
#include "cord.hpp"
#include "signalType.hpp"
#include "microBump.hpp"
#include "c4Bump.hpp"
#include "orderedSegment.hpp"
#include "powerGrid.hpp"

// Initialize the static const unordered_map
const std::unordered_map<SignalType, SignalType> PowerGrid::defulatM5SigPadMap = {
    { SignalType::POWER_1, SignalType::POWER_1},
    { SignalType::POWER_2, SignalType::POWER_2},
    { SignalType::POWER_3, SignalType::POWER_3},
    { SignalType::POWER_4, SignalType::POWER_4},
    { SignalType::POWER_5, SignalType::POWER_5},
    { SignalType::POWER_6, SignalType::POWER_6},
    { SignalType::POWER_7, SignalType::POWER_7},
    { SignalType::POWER_8, SignalType::POWER_8},
    { SignalType::POWER_9, SignalType::POWER_9},
    { SignalType::POWER_10, SignalType::POWER_10},
};

const std::unordered_map<SignalType, SignalType> PowerGrid::defulatM7SigPadMap = {
    { SignalType::POWER_1, SignalType::POWER_1},
    { SignalType::POWER_2, SignalType::POWER_2},
    { SignalType::POWER_3, SignalType::POWER_3},
    { SignalType::POWER_4, SignalType::POWER_4},
    { SignalType::POWER_5, SignalType::POWER_5},
    { SignalType::POWER_6, SignalType::POWER_6},
    { SignalType::POWER_7, SignalType::POWER_7},
    { SignalType::POWER_8, SignalType::POWER_8},
    { SignalType::POWER_9, SignalType::POWER_9},
    { SignalType::POWER_10, SignalType::POWER_10},
    { SignalType::GROUND, SignalType::OBSTACLE},
    { SignalType::SIGNAL, SignalType::OBSTACLE},
    { SignalType::OBSTACLE, SignalType::OBSTACLE},
};

PowerGrid::PowerGrid(const std::string &fileName): uBump(fileName), c4(fileName) {
    assert(uBump.getPinMapWidth() == c4.getPinMapWidth());
    assert(uBump.getPinMapHeight() == c4.getPinMapHeight());
    assert(uBump.getPinMapWidth() > 1);
    assert(uBump.getPinMapHeight() > 1);

    this->canvasWidth = uBump.getPinMapWidth() - 1;
    this->canvasHeight = uBump.getPinMapHeight() - 1;
    
    this->canvasM5.resize(this->canvasHeight, std::vector<SignalType>(this->canvasWidth, SignalType::EMPTY));
    this->canvasM7.resize(this->canvasHeight, std::vector<SignalType>(this->canvasWidth, SignalType::EMPTY));
}

void PowerGrid::insertPinPads(const PinMap &pm, std::vector<std::vector<SignalType>> &canvas, const std::unordered_map<SignalType, SignalType> &padTypeMap){
    for(std::unordered_map<Cord, SignalType>::const_iterator cit = pm.cordToSignalTypeMap.begin(); cit != pm.cordToSignalTypeMap.end(); ++cit){
        SignalType st = cit->second;
        std::unordered_map<SignalType, SignalType>::const_iterator ptcit = padTypeMap.find(st);
        // skip the kind of signal where doesn't exist in padTypeMap
        if(ptcit == padTypeMap.end()) continue;
        
        Cord c = cit->first;
        SignalType padst = ptcit->second;

        // if((st == SignalType::GROUND) || (st == SignalType::SIGNAL)) continue;

        if(c.x() != 0){
            if(c.y() != pm.getPinMapHeight()){
                canvas[c.y()][c.x() - 1] = padst;
            }

            if(c.y() != 0){
                canvas[c.y() - 1][c.x() - 1] = padst;
            }

        }

        if(c.x() != pm.getPinMapWidth()){
            if(c.y() != pm.getPinMapHeight()){
                canvas[c.y()][c.x()] = padst;
            }

            if(c.y() != 0){
                canvas[c.y() - 1][c.x()] = padst;

            }
        }
    }
}

void PowerGrid::reportOverlaps() const {
    std::map<SignalType, int> m5Stats;
    std::map<SignalType, int> m7Stats;
    std::map<SignalType, int> ovStats;

    for(int j = 0; j < canvasHeight; ++j){
        for(int i = 0; i < canvasWidth; ++i){
            SignalType stup = canvasM5[j][i];
            if(m5Stats.find(stup) == m5Stats.end()) m5Stats[stup] = 0;
            m5Stats[stup]++;

            SignalType stdn = canvasM7[j][i];
            if(m7Stats.find(stdn) == m7Stats.end()) m7Stats[stdn] = 0;
            m7Stats[stdn]++;

            if(stup == stdn){
                if(ovStats.find(stdn) == ovStats.end()) ovStats[stdn] = 0;
                ovStats[stdn]++;
            }
        }
    }

    // for(auto at : m5Stats){
    //     std::cout << at.first << " " << at.second << std::endl;
    // }
    // std::cout << std::endl;
    
    // for(auto at : m7Stats){
    //     std::cout << at.first << " " << at.second << std::endl;
    // }

    std::cout << "Overlap Report: " << std::endl;
    for(auto at : ovStats){
        SignalType st = at.first;
        double ovpct = 100*2*double(at.second) / (double(m7Stats[st]) + double(m5Stats[st]));
        std::cout << at.first << " " << at.second << "(" << ovpct << "%)" << std::endl;    
    }
}