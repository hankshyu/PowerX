//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        07/08/2025 16:50:14
//  Module Name:        diffusionChamber.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A basic cell instance in the diffusion system. holds the number of
//                      particles participating in the diffusion process
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//  2025/07/08          Divide basic diffusion chamber cell type from cell.hpp 
//////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:
#include <cassert>
#include <ostream>
#include <vector>

// 2. Boost Library:

// 3. Texo Library:
#include "diffusionChamber.hpp"
#include "signalType.hpp"
#include "dirFlags.hpp"

std::ostream& operator<<(std::ostream& os, CellType ct){
    
    switch (ct){
        case CellType::EMPTY:
            return os << "CellType::EMPTY";
            break;
        case CellType::OBSTACLES:
            return os << "CellType::OBSTACLES";
            break;
        case CellType::PREPLACED:
            return os << "CellType::PREPLACED";
            break;
        case CellType::MARKED:
            return os << "CellType::MARKED";
            break;
        default:
            return os << "CellType::UNkNOWN";
            break;
    }
}

DiffusionChamber::DiffusionChamber(): signal(SignalType::EMPTY), fullDirection(DIRFLAG_EMPTY) {}

int DiffusionChamber::getParticlesCount(CellLabel label){
    for(int i = 0; i < cellLabels.size(); ++i){
        if(label == cellLabels[i]) return cellParticles[i];
    }
    return -1;
}

void DiffusionChamber::addParticles(CellLabel label, int particleCount){
    assert(particleCount >= 0);
    for(int i = 0; i < cellLabels.size(); ++i){
        if(label == cellLabels[i]){
            cellParticles[i] += particleCount;
            return;
        }
    }
    
    // add new label
    cellLabels.push_back(label);
    cellParticles.push_back(particleCount);
}

void DiffusionChamber::removeParticles(CellLabel label, int particleCount){
    assert(particleCount >= 0);
    for (int i = 0; i < cellLabels.size(); ++i) {
        if (label == cellLabels[i]) {
            cellParticles[i] -= particleCount;
            if (cellParticles[i] <= 0) {
                // swap with back, then pop
                cellParticles[i] = cellParticles.back();
                cellLabels[i] = cellLabels.back();
                cellParticles.pop_back();
                cellLabels.pop_back();
            }
            return;
        }
    }
}

void DiffusionChamber::clearParticles(CellLabel label){
    for (int i = 0; i < cellLabels.size(); ++i) {
        if (label == cellLabels[i]) {
            // swap with back, then pop
            cellParticles[i] = cellParticles.back();
            cellLabels[i] = cellLabels.back();
            cellParticles.pop_back();
            cellLabels.pop_back();
            return;
        }
    }
}