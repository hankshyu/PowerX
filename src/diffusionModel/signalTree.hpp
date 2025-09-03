//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        08/01/2025 16:21:50
//  Module Name:        signalTree.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        Holds the datastructure to run the filler algorithm of one signal type
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
/////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:
#include <vector>
#include <unordered_set>
#include <unordered_map>

// 2. Boost Library:

// 3. Texo Library:
#include "signalType.hpp"
#include "powerDistributionNetwork.hpp"
#include "diffusionChamber.hpp"

#include "candVertex.hpp"

// 4. PETSc Library:
#include "petscmat.h"
#include "petscksp.h"

#ifndef __SIGNAL_TREE_H__
#define __SIGNAL_TREE_H__

class SignalTree{
public:

    SignalType signal;
    int chipletCount;
    double currentBudget; // in pctg;

    std::unordered_set<DiffusionChamber *> preplacedNodes;
    std::unordered_set<DiffusionChamber *> preplacedOrMarkedNodes;
    std::unordered_set<DiffusionChamber *> candidateNodes;

    // std::vector<CandVertex> candidates;
    // node 0 = input node
    // node 1 ~ n is chiplet output Node

    size_t pinInIdxBegin;
    size_t pinInIdxEnd;

    size_t pinOutIdxBegin;
    size_t pinOutIdxEnd;

    std::vector<DiffusionChamber *> GIdxToNode;
    std::unordered_map<DiffusionChamber *, size_t> nodeToGIdx;


    PetscInt n_size;
    PetscInt exp_size;

    Mat G_n;          // Conductance matrix
    KSP ksp_n;        // Cached solver (e.g., with Cholesky preconditioner)
    bool kspPrepared = false;

    Mat V_n;
    Mat I_n;

    size_t resultIdxBegin;
    size_t resultIdxEnd;

    SignalTree();
    SignalTree(SignalType signal, int chipletCount, double budget);

};

#endif // __SIGNAL_TREE_H__


