//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        07/03/2025 14:07:50
//  Module Name:        cell.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A basic cell in the diffusion system. holds the number of
//                      particles participating in the diffusion process
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//
//////////////////////////////////////////////////////////////////////////////////

#ifndef __CELL_H__
#define __CELL_H__

// Dependencies
// 1. C++ STL:
#include <cstdint>
#include <ostream>
#include <vector>
#include <limits>
// 2. Boost Library:

// 3. Texo Library:
#include "signalType.hpp"
#include "dirFlags.hpp"

// 0 is empty, 1 ~ n
typedef uint8_t CellLabel;

enum class CellType : uint8_t{
    EMPTY = 0,
    OBSTACLES = 1,
    PREPLACED = 2,
    MARKED = 3, 
};

std::ostream& operator<<(std::ostream& os, CellType ct);

class DiffusionChamber{
public:

    SignalType signal;

    DirFlag fullDirection;
    DirFlag direction;

    std::vector<CellLabel> cellLabels;
    std::vector<int> cellParticles;

    DiffusionChamber(): signal(SignalType::EMPTY), fullDirection(DIRFLAG_EMPTY), direction(DIRFLAG_EMPTY) {}
    
    // return -1 if lable not exist
    int getParticlesCount(CellLabel label); 

    void addParticles(CellLabel label, int particleCount);
    void removeParticles(CellLabel label, int particleCount);
    void clearParticles(CellLabel label);
};

constexpr size_t SIZET_INVALID = std::numeric_limits<size_t>::max();

struct CellCord{
    size_t l;
    size_t w;
    size_t h;

    CellCord(size_t l, size_t w, size_t h): l(l), w(w), h(h) {}
};

class ViaCell;

class MetalCell : public DiffusionChamber{
public:

    MetalCell *upCell = nullptr;
    MetalCell *downCell = nullptr;
    MetalCell *leftCell = nullptr;
    MetalCell *rightCell = nullptr;

    ViaCell *topCell = nullptr;
    ViaCell *bottomCell = nullptr;

    size_t upCellIdx = SIZET_INVALID;
    size_t downCellIdx = SIZET_INVALID;
    size_t leftCellIdx = SIZET_INVALID;
    size_t rightCellIdx = SIZET_INVALID;

    size_t topCellIdx = SIZET_INVALID;
    size_t bottomCellIdx = SIZET_INVALID;

};

struct ViaCord{
    size_t l;
    size_t w;

    ViaCord(size_t l, size_t w): l(l), w(w) {}
};

class ViaCell : public DiffusionChamber{
public:

    MetalCell *upLLCell = nullptr;
    MetalCell *upULCell = nullptr;
    MetalCell *upLRCell = nullptr;
    MetalCell *upURCell = nullptr;

    MetalCell *downLLCell = nullptr;
    MetalCell *downULCell = nullptr;
    MetalCell *downLRCell = nullptr;
    MetalCell *downURCell = nullptr;

    size_t upLLCellIdx = SIZET_INVALID;
    size_t upULCellIdx = SIZET_INVALID;
    size_t upLRCellIdx = SIZET_INVALID;
    size_t upURCellIdx = SIZET_INVALID;

    size_t downLLCellIdx = SIZET_INVALID;
    size_t downULCellIdx = SIZET_INVALID;
    size_t downLRCellIdx = SIZET_INVALID;
    size_t downURCellIdx = SIZET_INVALID;
};

std::ostream& operator<<(std::ostream& os, const MetalCell &ct);

#endif // __CELL_H__