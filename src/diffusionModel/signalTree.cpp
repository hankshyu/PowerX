//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        08/03/2025 21:27:56 
//  Module Name:        signalTree.cpp
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
#include "diffusionChamber.hpp"
#include "candVertex.hpp"
#include "signalTree.hpp"

// SignalTree::SignalTree(): signal(SignalType::UNKNOWN), chipletCount(0), currentBudget(0) {}

// SignalTree::SignalTree(SignalType signal, int chipletCount, double budget): signal(signal), chipletCount(chipletCount), currentBudget(budget) {
//     assert(chipletCount >= 1);

// }

SignalTree::SignalTree()
: signal(SignalType::UNKNOWN), chipletCount(0), currentBudget(0) {}

SignalTree::SignalTree(SignalType signal, int chipletCount, double budget)
: signal(signal), chipletCount(chipletCount), currentBudget(budget) {
    assert(chipletCount >= 1);
}

SignalTree::~SignalTree() {
    clearWT();
    clearW();
    clearDenseHelpers();
    clearMatrices();
    clearKSP();
}

void SignalTree::clearKSP() {
    if (ksp_n) { KSPDestroy(&ksp_n); ksp_n = nullptr; }
    kspPrepared = false;
}

void SignalTree::clearDenseHelpers() {
    if (I_n) { MatDestroy(&I_n); I_n = nullptr; }
    if (V_n) { MatDestroy(&V_n); V_n = nullptr; }
}

void SignalTree::clearW() {
    if (W) { MatDestroy(&W); W = nullptr; }
}

void SignalTree::clearWT() {
    if (WT) { MatDestroy(&WT); WT = nullptr; }
    WTIndex.clear();
    haveWT = false;
}

void SignalTree::clearMatrices() {
    if (G_n) { MatDestroy(&G_n); G_n = nullptr; }
}



