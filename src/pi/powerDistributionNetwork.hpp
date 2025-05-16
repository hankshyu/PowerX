//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        05/01/2025 16:54:29
//  Module Name:        powerDistributionNetwork.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A parent class that includes the bump holding data structures
//                      for ubump/C4, 2D vectors holding Metal layers and 2D verctors
//                      holding vias crossing metal layers
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//
//////////////////////////////////////////////////////////////////////////////////

#ifndef __POWER_DISTRIBUTION_NETWORK_H__
#define __POWER_DISTRIBUTION_NETWORK_H__

// Dependencies
// 1. C++ STL:
#include <cassert>
#include <vector>
#include <unordered_map>

// 2. Boost Library:


// 3. Texo Library:
#include "doughnutPolygon.hpp"
#include "doughnutPolygonSet.hpp"
#include "technology.hpp"
#include "eqCktExtractor.hpp"
#include "signalType.hpp"
#include "objectArray.hpp"
#include "microBump.hpp"
#include "c4Bump.hpp"

class PowerDistributionNetwork{
protected:
    int m_gridWidth;
    int m_gridHeight;
    int m_pinWidth;
    int m_pinHeight;
    int m_metalLayerCount;
    int m_viaLayerCount;

    int m_ubumpConnectedMetalLayerIdx;
    int m_c4ConnectedMetalLayerIdx;

public:
    MicroBump uBump;
    C4Bump c4;

    std::vector<ObjectArray> metalLayers;
    std::vector<ObjectArray> viaLayers;

    const static std::unordered_map<SignalType, SignalType> defulatuBumpSigPadMap;
    const static std::unordered_map<SignalType, SignalType> defulatc4SigPadMap;

    PowerDistributionNetwork(const std::string &fileName);

    inline int getGridWidth() const {return this->m_gridWidth;}
    inline int getGridHeight() const {return this->m_gridHeight;}
    inline int getPinWidth() const {return this->m_pinWidth;}
    inline int getPinHeight() const {return this->m_pinHeight;}
    inline int getMetalLayerCount() const {return this->m_metalLayerCount;}
    inline int getViaLayerCount() const {return this->m_viaLayerCount;}
    inline int getuBumpConnectedMetalLayerIdx() const {return this->m_ubumpConnectedMetalLayerIdx;}
    inline int getc4ConnectedmetalLayerIdx() const {return this->m_c4ConnectedMetalLayerIdx;}

    
    bool checkOnePiece(int layer);
    bool checkPinPadValid(int layer);


    void assignVias();
    void removeFloatingPlanes(int layer);
    void exportEquivalentCircuit(const SignalType st, const Technology &tch, const EqCktExtractor &extor, const std::string &filePath);
};

void markPinPadsWithoutSignals(std::vector<std::vector<SignalType>> &gridCanvas, const std::vector<std::vector<SignalType>> &pinCanvas, const std::unordered_set<SignalType> &avoidSignalTypes);
void markPinPadsWithSignals(std::vector<std::vector<SignalType>> &gridCanvas, const std::vector<std::vector<SignalType>> &pinCanvas, const std::unordered_set<SignalType> &signalTypes);

void runClustering(const std::vector<std::vector<SignalType>> &canvas, std::vector<std::vector<int>> &cluster, std::unordered_map<SignalType, std::vector<int>> &label);

// void insertPinPads(const PinMap &pm, std::vector<std::vector<SignalType>> &canvas, const std::unordered_map<SignalType, SignalType> &padTypeMap);

std::unordered_map<SignalType, DoughnutPolygonSet> collectDoughnutPolygons(const std::vector<std::vector<SignalType>> &canvas);




#endif // __POWER_DISTRIBUTION_NETWORK_H__