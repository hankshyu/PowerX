//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        07/08/2025 18:10:01
//  Module Name:        diffusionEngine.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        The top module of the diffusion system
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
// 07/08/2025           Ported from diffusionSimulator class
/////////////////////////////////////////////////////////////////////////////////

#ifndef __DIFFUSION_ENGINE_H__
#define __DIFFUSION_ENGINE_H__

// Dependencies
// 1. C++ STL:
#include <cstddef>
#include <vector>

// 2. Boost Library:

// 3. Texo Library:
#include "signalType.hpp"
#include "powerDistributionNetwork.hpp"
#include "dirFlags.hpp"
#include "diffusionChamber.hpp"
#include "metalCell.hpp"
#include "viaCell.hpp"
#include "units.hpp"

class DiffusionEngine : public PowerDistributionNetwork{
    size_t m_metalGridLayers;
    size_t m_metalGridWidth;
    size_t m_metalGridHeight;
    size_t m_metalGrid2DCount;
    size_t m_metalGrid3DCount;

    size_t m_viaGridLayers;

    std::vector<size_t> m_viaGrid2DCount;
    std::vector<size_t> m_viaGrid2DAccumlateCount; // [2] = count[0] +..+count[2]

public:

    std::vector<SignalType> cellLabelToSigType;
    std::unordered_map<SignalType, std::vector<CellLabel>> sigTypeToAllCellLabels;

    std::vector<MetalCell> metalGrid;
    std::vector<CellLabel> metalGridLabel;

    std::vector<ViaCell> viaGrid;
    std::vector<CellLabel> viaGridlabel;

    DiffusionEngine(const std::string &fileName);

    size_t calMetalIdx(size_t layer, size_t height, size_t width) const;
    size_t calMetalIdx(const MetalCord &cc) const;
    MetalCord calMetalCord(size_t idx) const;

    // returns the index of the first element of the metal layer
    size_t getlMetalIdxBegin(size_t layer) const;
    // returns the index of the first element of the metal layer/height
    size_t getMetalIdxBegin(size_t layer, size_t height) const;
    // returns the index+1 of the last element of the metal layer
    size_t getMetalIdxEnd(size_t layer) const;
    // returns the index+1 of the last element of the metal layer/height
    size_t getMetalIdxEnd(size_t layer, size_t height) const;


    size_t calViaIdx(size_t l, size_t w) const;
    size_t calViaIdx(const ViaCord &vc) const;
    ViaCord calViaCord(size_t idx) const;
    
    // returns the index of the first element of the via layer
    size_t getViaIdxBegin(size_t layer) const;
    // returns the index+1 of the last element of the via layer/height
    size_t getViaIdxEnd(size_t layer) const;
     // returns the index+1 of the last element of the all vias
    size_t getAllViaIdxEnd() const;

    void markPreplacedAndInsertPadsOnCanvas();
    void markObstaclesOnCanvas();
    void initialiseGraphWithPreplaced();

    void fillEnclosedRegions();
    void markHalfOccupiedMetalsAndPins();
    void linkNeighbors();
    void initialiseIndexing();
    
};

#endif // __DIFFUSION_ENGINE_H__