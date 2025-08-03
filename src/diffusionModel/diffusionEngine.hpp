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
#include <unordered_set>
#include <unordered_map>

// 2. Boost Library:

// 3. Texo Library:
#include "signalType.hpp"
#include "powerDistributionNetwork.hpp"
#include "dirFlags.hpp"
#include "diffusionChamber.hpp"
#include "metalCell.hpp"
#include "viaCell.hpp"
#include "units.hpp"

#include "flowNode.hpp"
#include "flowEdge.hpp"

#include "candVertex.hpp"
#include "signalTree.hpp"

// 4. Gurobi Library
#include "gurobi_c++.h"

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
    std::unordered_map<SignalType, double> currentBudget; // normalized to sum = 1

    std::vector<SignalType> cellLabelToSigType;
    std::unordered_map<SignalType, std::vector<CellLabel>> sigTypeToAllCellLabels;

    std::vector<MetalCell> metalGrid;
    std::vector<CellLabel> metalGridLabel;

    std::vector<ViaCell> viaGrid;
    std::vector<CellLabel> viaGridLabel;

    // MCF related attributes
    double normalMetalEdgeLB = 0.0;
    double normalMetalEdgeUB = 1.0;
    double normalMetalEdgeWeight = 1.0;

    double aggrMetalEdgeLB = 0.0;
    double aggrMetalEdgeUB = 2.0;
    double aggrMetalEdgeWeight = 0.2;
    
    double ViaEdgeLB = 0.0;
    double ViaEdgeUB = 1.75;
    double subViaEdgeUB = ViaEdgeUB / 2.5;
    double viaEdgeWeight = 0.1;

    double ViaBudgetAvgQuota = 0.2;
    double viaBudgetCurrentQuota = 0.7;

    double minChipletBudgetAvgPctg = 0.75;

    // sorting by ascending order of current requirement
    std::vector<SignalType> flowSOIIdxToSig;
    std::unordered_map<SignalType, int> flowSOISigToIdx;
    std::vector<double> SOIBudget;
    
    std::vector<FlowNode> superSource;
    std::vector<FlowNode> superSink;

    std::vector<std::vector<FlowNode>> viaFlowTopNodeArr;
    std::vector<std::vector<FlowNode>> viaFlowDownNodeArr;
    
    std::vector<FlowNode> metalFlowNodeOwnership;
    std::vector<std::vector<std::vector<FlowNode *>>> metalFlowNodeArr;
    std::unordered_map<SignalType, std::vector<FlowNode *>> superSourceConnectedNodes;
    std::unordered_map<SignalType, std::vector<FlowNode *>> superSinkConnectedNodes;

    std::vector<FlowEdge *> flowEdgeOwnership;

    /* Filler related attributes */
    std::unordered_set<DiffusionChamber *> allPreplacedNodes;
    std::unordered_set<DiffusionChamber *> allPreplacedOrMarkedNodes;
    std::unordered_set<DiffusionChamber *> allCandidateNodes;

    std::unordered_map<DiffusionChamber *, std::vector<SignalType>> overlapNodes;
    std::unordered_map<SignalType, SignalTree> signalTrees;



    DiffusionEngine(const std::string &fileName);

    size_t calMetalIdx(size_t layer, size_t height, size_t width) const;
    size_t calMetalIdx(const MetalCord &cc) const;
    MetalCord calMetalCord(size_t idx) const;

    // returns the index of the first element of the metal layer
    size_t getMetalIdxBegin(size_t layer) const;
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

    void writeBackToPDN();
    void exportResultsToFile(const std::string &filePath);
    void importResultsFromFile(const std::string &filePath);
    
    /* These are functions for multi-source DFS (Diffusion)*/
    void runDiffusionTop(double diffusionRate);
    // returns the number of labels (0 is reserved for empty)
    int initialiseIndexing();
    void placeDiffusionParticles();
    void diffuse(double diffusionRate);
    void stage();
    void commit();


    /* These are functions for MCF (Multi-commodity Flow)*/
    void initialiseMCFSolver();
    void runMCFSolver(std::string logFile, int outputLevel);

    /* These are functions for Resistor Network Solving to fill empty spaces */
    void initialiseFiller();
    void initialiseSignalTrees();


    // make sure the connections are correct, only for verification
    void checkConnections();
    void checkNeighbors();

    friend bool visualiseDiffusionEngineMetal(const DiffusionEngine &dfe, size_t layer, const std::string &filePath);
    friend bool visualiseDiffusionEngineVia(const DiffusionEngine &dfe, size_t layer, const std::string &filePath);
    friend bool visualiseDiffusionEngineMetalAndVia(const DiffusionEngine &dfe, size_t metalLayer, size_t viaLayer, const std::string &filePath);
    
};

#endif // __DIFFUSION_ENGINE_H__