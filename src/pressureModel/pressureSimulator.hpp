//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        06/04/2025 16:58:37
//  Module Name:        pressureSimulator.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        The top module of the pressure growing system
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
/////////////////////////////////////////////////////////////////////////////////

#ifndef __PRESSURE_SIMULATOR_H__
#define __PRESSURE_SIMULATOR_H__

// Dependencies
// 1. C++ STL:
#include <vector>
#include <unordered_map>
#include <memory>

// 2. Boost Library:

// 3. Texo Library:
#include "signalType.hpp"
#include "powerDistributionNetwork.hpp"

#include "fpoint.hpp"
#include "fbox.hpp"
#include "fpolygon.hpp"
#include "fmultipolygon.hpp"
#include "softBody.hpp"
#include "viaBody.hpp"

#include "pointBinSystem.hpp"

class PressureSimulator: public PowerDistributionNetwork{
private:

    flen_t m_PointsMinDelta = 0.2;

    std::vector<std::vector<SoftBody *>> m_OwnerSoftBodies;
    std::vector<std::vector<ViaBody *>> m_OwnerViasBodies;
    
public:

    std::vector<std::vector<FBox>> softBodyBoundingBox;

    // 
    std::vector<PointBinSystem<flen_t, ViaBody>> viaBins;

   

    PressureSimulator(const std::string &fileName);
    ~PressureSimulator();

    void inflate();

};

// friend bool visualisePressureSimulator(const PressureSimulator &ps, std::string &filePath);


#endif // __PRESSURE_SIMULATOR_H__