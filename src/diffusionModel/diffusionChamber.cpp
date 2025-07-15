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
#include "boost/functional/hash.hpp"

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

std::ostream& operator<<(std::ostream& os, DiffusionChamberType dct){
        switch (dct){
        case DiffusionChamberType::UNKNOWN:
            return os << "DiffusionChamberType::EMPTY";
            break;
        case DiffusionChamberType::METAL:
            return os << "DiffusionChamberType::OBSTACLES";
            break;
        case DiffusionChamberType::VIA:
            return os << "DiffusionChamberType::PREPLACED";
            break;
        default:
            return os;
            break;
    }
}

DiffusionChamber::DiffusionChamber(): index(SIZE_T_INVALID), metalViaType(DiffusionChamberType::UNKNOWN),
    canvasLayer(LEN_T_MIN), canvasX(LEN_T_MIN), canvasY(LEN_T_MIN),
    type(CellType::EMPTY), signal(SignalType::EMPTY), fullDirection(DIRFLAG_EMPTY) {}

int DiffusionChamber::getParticlesCount(CellLabel label){
    for(int i = 0; i < cellLabels.size(); ++i){
        if(label == cellLabels[i]) return cellParticles[i];
    }
    return -1;
}

void DiffusionChamber::addParticlesToCache(CellLabel label, int particleCount){
    if(particleCount == 0) return;
    assert(particleCount != 0);
    for(int i = 0; i < cellLabelsCache.size(); ++i){
        if(label == cellLabelsCache[i]){
            cellParticlesCache[i] += particleCount;
            if (cellParticlesCache[i] == 0) {
                // remove lable if it's == 0
                cellParticlesCache[i] = cellParticlesCache.back();
                cellLabelsCache[i] = cellLabelsCache.back();
                cellParticlesCache.pop_back();
                cellLabelsCache.pop_back();
            }
            return;
        }
    }
    // add new label
    cellLabelsCache.push_back(label);
    cellParticlesCache.push_back(particleCount);
}

// void DiffusionChamber::removeParticlesFromCache(CellLabel label, int particleCount){
//     assert(particleCount >= 0);
//     for (int i = 0; i < cellLabelsCache.size(); ++i) {
//         if (label == cellLabelsCache[i]) {
//             cellParticlesCache[i] -= particleCount;
//             if (cellParticles[i] == 0) {
//                 // remove lable if it's == 0
//                 cellParticlesCache[i] = cellParticlesCache.back();
//                 cellLabelsCache[i] = cellLabelsCache.back();
//                 cellParticlesCache.pop_back();
//                 cellLabelsCache.pop_back();
//             }
//             return;
//         }
//     }
// }

// void DiffusionChamber::clearParticles(CellLabel label){
//     for (int i = 0; i < cellLabels.size(); ++i) {
//         if (label == cellLabels[i]) {
//             // swap with back, then pop
//             cellParticles[i] = cellParticles.back();
//             cellLabels[i] = cellLabels.back();
//             cellParticles.pop_back();
//             cellLabels.pop_back();
//             return;
//         }
//     }
// }

void DiffusionChamber::commitCache(){
    cellLabels = std::move(cellLabelsCache);
    cellParticles = std::move(cellParticlesCache);

    cellLabelsCache.clear();
    cellParticlesCache.clear();
}

void DiffusionChamber::flushCache(){

    cellLabelsCache.clear();
    cellParticlesCache.clear();
}

size_t std::hash<DiffusionChamber>::operator()(const DiffusionChamber &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.metalViaType);
    boost::hash_combine(seed, key.index);
    return seed;
}