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

// class SignalTree{
// public:

//     SignalType signal;
//     int chipletCount;
//     double currentBudget; // in pctg;

//     std::unordered_set<DiffusionChamber *> preplacedNodes;
//     std::unordered_set<DiffusionChamber *> preplacedOrMarkedNodes;
//     std::unordered_set<DiffusionChamber *> candidateNodes;

//     // std::vector<CandVertex> candidates;
//     // node 0 = input node
//     // node 1 ~ n is chiplet output Node

//     size_t pinInIdxBegin;
//     size_t pinInIdxEnd;

//     size_t pinOutIdxBegin;
//     size_t pinOutIdxEnd;

//     std::vector<DiffusionChamber *> GIdxToNode;
//     std::unordered_map<DiffusionChamber *, size_t> nodeToGIdx;


//     PetscInt n_size;
//     PetscInt exp_size;

//     Mat G_n;          // Conductance matrix
//     KSP ksp_n;        // Cached solver (e.g., with Cholesky preconditioner)
//     bool kspPrepared = false;

//     Mat V_n;
//     Mat I_n;

//     size_t resultIdxBegin;
//     size_t resultIdxEnd;

//     SignalTree();
//     SignalTree(SignalType signal, int chipletCount, double budget);

// };

class SignalTree {
public:
    SignalType signal;
    int        chipletCount;
    double     currentBudget; // %

    std::unordered_set<DiffusionChamber *> preplacedNodes;
    std::unordered_set<DiffusionChamber *> preplacedOrMarkedNodes;
    std::unordered_set<DiffusionChamber *> candidateNodes;

    size_t pinInIdxBegin  = 0;
    size_t pinInIdxEnd    = 0;
    size_t pinOutIdxBegin = 0;
    size_t pinOutIdxEnd   = 0;

    std::vector<DiffusionChamber *>               GIdxToNode;
    std::unordered_map<DiffusionChamber*, size_t> nodeToGIdx;

    // Sizes
    PetscInt n_size       = 0;   // active full size
    PetscInt exp_size     = 0;   // #sinks
    PetscInt n_size_red   = 0;   // active reduced size

    // NEW: capacity fields (never change after init)
    PetscInt n_size_cap     = 0; // capacity full size
    PetscInt n_size_red_cap = 0; // capacity reduced size

    // Solver objects
    Mat G_n   = nullptr;
    KSP ksp_n = nullptr;
    bool kspPrepared = false;

    Mat V_n = nullptr;
    Mat I_n = nullptr;

    size_t resultIdxBegin = 0;
    size_t resultIdxEnd   = 0;

    // --- reduced indexing ---
    PetscInt ground_full_idx = -1;
    std::vector<PetscInt> full2red;   // map full -> reduced
    std::vector<PetscInt> red2full;   // map reduced -> full

    // --- Green’s columns ---
    Mat W  = nullptr; // minimal (supernodes only)
    Mat WT = nullptr; // expanded (supernodes + neighbors)
    std::unordered_map<PetscInt, PetscInt> WTIndex;
    bool haveWT = false;

    int aij_nnz_per_row_hint = 12; // tuning knob

    // Constructors / destructor
    SignalTree();
    SignalTree(SignalType signal, int chipletCount, double budget);
    ~SignalTree();

    // Helpers
    void clearKSP();
    void clearDenseHelpers();
    void clearW();
    void clearWT();
    void clearMatrices();

    // --- NEW: helpers for capacity-aware logic ---
    PetscInt activeFullSize()  const { return n_size; }
    PetscInt activeRedSize()   const { return n_size_red; }
    PetscInt capacityFullSize() const { return n_size_cap; }
    PetscInt capacityRedSize()  const { return n_size_red_cap; }

    inline void setActiveSizes(PetscInt full, PetscInt red) {
        n_size     = full;
        n_size_red = red;
    }
};





#endif // __SIGNAL_TREE_H__


