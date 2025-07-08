//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        07/03/2025 13:26:38
//  Module Name:        diffusionSimulator.hpp
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
//
/////////////////////////////////////////////////////////////////////////////////

#ifndef __DIFFUSION_SIMULATOR_H__
#define __DIFFUSION_SIMULATOR_H__

// Dependencies
// 1. C++ STL:
#include <string>
#include <vector>
#include <unordered_map>
// 2. Boost Library:

// 3. Texo Library:
#include "signalType.hpp"
#include "powerDistributionNetwork.hpp"
#include "dirFlags.hpp"
#include "cell.hpp"


class DiffusionSimulator: public PowerDistributionNetwork{
private:
    size_t m_cellGridLayers = SIZET_INVALID;
    size_t m_cellGridWidth = SIZET_INVALID;
    size_t m_cellGridHeight = SIZET_INVALID;
    size_t m_cellGrid2DCount = SIZET_INVALID;
    size_t m_cellGrid3DCount = SIZET_INVALID;

    size_t m_viaGridLayers = SIZET_INVALID;
    std::vector<size_t> m_viaGrid2DCount;
    std::vector<size_t> m_viaGrid2DAccumlateCount;
    
    inline size_t calCellIdx(size_t l, size_t h, size_t w) {return l * m_cellGrid2DCount + h * m_cellGridWidth + w;}
    inline size_t calCellIdx(const CellCord &cc){return cc.l * m_cellGrid2DCount + cc.h * m_cellGridWidth + cc.w;}
    CellCord calCellCord(size_t idx);

    inline size_t calViaIdx(size_t l, size_t w) {return m_viaGrid2DAccumlateCount[l] + w;}
    inline size_t calViaIdx(const ViaCord &vc) {return m_viaGrid2DAccumlateCount[vc.l] + vc.w;}
    ViaCord calViaCord(size_t idx);

public:
    
    int labelCount;
    std::vector<SignalType> cellLabelToSigType;
    std::unordered_map<SignalType, std::vector<CellLabel>> sigTypeToAllCellLabels;

    std::vector<MetalCell> cellGrid;
    std::vector<CellType> cellGridType;
    std::vector<CellLabel> cellGridLabel;

    std::vector<ViaCell> viaGrid;
    std::vector<CellType> viaGridType;
    std::vector<CellLabel> viaGridlabel;

    DiffusionSimulator(const std::string &fileName);


    // top module for executing the algorithm
    // void runAlgorithm();
    void transformSignals();
    void fillEnclosedRegions();
    void initialise();

};

#endif // __DIFFUSION_SIMULATOR_H__