//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        07/83/2025 16:57:54
//  Module Name:        viaCell.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        An child class of the basic cell in the diffusion system. Represents
//                      a via instance
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//  2025/07/08          Divide basic diffusion chamber cell type from cell.hpp 
//////////////////////////////////////////////////////////////////////////////////

#ifndef __VIA_CELL_H__
#define __VIA_CELL_H__

// Dependencies
// 1. C++ STL:
#include <cstddef>
#include <ostream>

// 2. Boost Library:

// 3. Texo Library:
#include "diffusionChamber.hpp"
#include "metalCell.hpp"

struct ViaCord{
    size_t l;
    size_t w;

    ViaCord();
    ViaCord(size_t layer, size_t width);
};

std::ostream& operator<<(std::ostream& os, const ViaCord &vc);

class ViaCell : public DiffusionChamber{
public:
    len_t canvasViaLayer;
    len_t canvasViaX;
    len_t canvasViaY;

    std::vector<MetalCell *> neighbors;

    MetalCell *upLLCell;
    MetalCell *upULCell;
    MetalCell *upLRCell;
    MetalCell *upURCell;

    MetalCell *downLLCell;
    MetalCell *downULCell;
    MetalCell *downLRCell;
    MetalCell *downURCell;

    size_t upLLCellIdx;
    size_t upULCellIdx;
    size_t upLRCellIdx;
    size_t upURCellIdx;

    size_t downLLCellIdx;
    size_t downULCellIdx;
    size_t downLRCellIdx;
    size_t downURCellIdx;

    ViaCell();

};


#endif // __VIA_CELL_H__