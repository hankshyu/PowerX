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
#include "rectangleBinSystem.hpp"

class PressureSimulator: public PowerDistributionNetwork{
private:
    // simulaittion system contour (playground size of the system)
    int m_canvasWidth;
    int m_canvasHeight;

    flen_t m_PointsMinDelta = 0.2;

    std::vector<std::vector<SoftBody *>> m_OwnerSoftBodies;
    std::vector<std::vector<ViaBody *>> m_OwnerViasBodies;
    
public:
    // bin system of the bounding box of softBodies
    std::vector<RectangleBinSystem<flen_t, SoftBody>> softBodyRectangleBin;
    std::vector<PointBinSystem<flen_t, SoftBody>> softBodyPointBin;

    // bin system of the bounding box of vias
    std::vector<PointBinSystem<flen_t, ViaBody>> viaPointBins;

   

    PressureSimulator(const std::string &fileName);
    ~PressureSimulator();

    inline int getCanvasWidth() const {return m_canvasWidth;}
    inline int getCanvasHeight() const {return m_canvasHeight;}

    const std::vector<SoftBody *> &getSoftBodyOwner(int layer) const;
    const std::vector<ViaBody *> &getViaBodyOwner(int layer) const;

    void inflate();

    friend bool visualiseSoftBodies(const PressureSimulator &ps, const std::vector<SoftBody *> softBodies, const std::string &filePath);
    friend bool visualiseSoftBodiesWithPin(const PressureSimulator &ps, const std::vector<SoftBody *> softBodies, const std::vector<ViaBody *> vias, const std::string &filePath);
    friend bool visualiseSoftBodiesWithPins(const PressureSimulator &ps, const std::vector<SoftBody *> softBodies, const std::vector<ViaBody *> upVias,  const std::vector<ViaBody *> downVias, const std::string &filePath);

};
#endif // __PRESSURE_SIMULATOR_H__