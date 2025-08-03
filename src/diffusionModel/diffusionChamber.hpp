//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        07/08/2025 16:46:46
//  Module Name:        diffusionChamber.hpp
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

#ifndef __DIFFUSION_CHAMBER_H__
#define __DIFFUSION_CHAMBER_H__

// Dependencies
// 1. C++ STL:
#include <cstdint>
#include <iostream>
#include <vector>
#include <limits>
// 2. Boost Library:

// 3. Texo Library:
#include "units.hpp"
#include "signalType.hpp"
#include "dirFlags.hpp"
#include "flowEdge.hpp"

// 0 is empty, 1 ~ n
typedef uint16_t CellLabel;
constexpr CellLabel CELL_LABEL_EMPTY = 0;

// EMPTY for competable cells, MARKED for soft, PREPLACED as hard and OBSTACLES
enum class CellType : uint8_t{
    EMPTY = 0,
    OBSTACLES = 1,
    PREPLACED = 2,
    MARKED = 3, 

    CANDIDATE = 4
};

std::ostream& operator<<(std::ostream& os, CellType ct);
std::istream& operator>>(std::istream& is, CellType& ct);

enum class DiffusionChamberType : uint8_t{
    UNKNOWN = 0,
    METAL = 1,
    VIA = 2
};

std::ostream& operator<<(std::ostream& os, DiffusionChamberType dct);

class FlowEdge;

class DiffusionChamber{
public:
    
    DiffusionChamberType metalViaType;
    len_t canvasLayer;
    len_t canvasX;
    len_t canvasY;

    DirFlag fullDirection;

    CellType type;
    SignalType signal;

    // this is the index place in both metalGrid/metalGridLabel or ViaGrid/viaGridlabel
    size_t index;

    std::vector<DiffusionChamber *> neighbors;

    std::vector<CellLabel> cellLabels;
    std::vector<int> cellParticles;

    std::vector<CellLabel> cellLabelsCache;
    std::vector<int> cellParticlesCache;


    DiffusionChamber();

    // return -1 if lable not exist
    int getParticlesCount(CellLabel label); 

    void addParticlesToCache(CellLabel label, int particleCount);
    // void removeParticlesFromCache(CellLabel label, int particleCount);
    // void clearParticles(CellLabel label);

    void commitCache();
    void flushCache();
};

// Cord class hash function implementations
namespace std {
    template <>
    struct hash<DiffusionChamber> {
        size_t operator()(const DiffusionChamber &key) const;
    };

}  // namespace std



#endif // __DIFFUSION_CHAMBER_H__