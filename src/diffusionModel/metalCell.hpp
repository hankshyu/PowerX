//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        07/03/2025 14:07:50
//  Module Name:        metalCell.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        An child class of the basic cell in the diffusion system. Represents
//                      a metal cell instance
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//  2025/07/08          Divide basic diffusion chamber cell type from cell.hpp 
//////////////////////////////////////////////////////////////////////////////////

#ifndef __METAL_CELL_H__
#define __METAL_CELL_H__

// Dependencies
// 1. C++ STL:
#include <cstddef>
#include <ostream>

// 2. Boost Library:

// 3. Texo Library:
#include "diffusionChamber.hpp"

struct MetalCord{
    size_t l;
    size_t w;
    size_t h;

    MetalCord();
    MetalCord(size_t layer, size_t width, size_t height);
};

std::ostream& operator<<(std::ostream& os, const MetalCord &mc);

class ViaCell;

class MetalCell : public DiffusionChamber{
public:

    std::vector<MetalCell *> metalCellNeighbors;
    std::vector<ViaCell *> viaCellNeighbors;

    MetalCell *northCell;
    MetalCell *southCell;
    MetalCell *eastCell;
    MetalCell *westCell;

    ViaCell *upCell;
    ViaCell *downCell;

    size_t northCellIdx;
    size_t southCellIdx;
    size_t eastCellIdx;
    size_t westCellIdx;

    size_t upCellIdx;
    size_t downCellIdx;

    MetalCell();
};

std::ostream& operator<<(std::ostream& os, const MetalCell &mc);

#endif // __METAL_CELL_H__