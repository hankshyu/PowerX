//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        07/03/2025 14:30:47
//  Module Name:        cell.cpp
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
//////////////////////////////////////////////////////////////////////////////////｀

// Dependencies
// 1. C++ STL:
#include <cassert>
// 2. Boost Library:

// 3. Texo Library:
#include "metalCell.hpp"
#include "units.hpp"

MetalCord::MetalCord(): l(SIZE_T_INVALID), w(SIZE_T_INVALID), h(SIZE_T_INVALID) {}

MetalCord::MetalCord(size_t layer, size_t width, size_t height): l(layer), w(width), h(height) {}

std::ostream& operator<<(std::ostream& os, const MetalCord &mc){
    return os << "MetalCord(l = " << mc.l << ", w = " << mc.w <<  ", h = " << mc.h << ")";
}

MetalCell::MetalCell(): DiffusionChamber(),
    upCell(nullptr), downCell(nullptr), leftCell(nullptr), rightCell(nullptr), 
    topCell(nullptr), bottomCell(nullptr),
    upCellIdx(SIZE_T_INVALID), downCellIdx(SIZE_T_INVALID), leftCellIdx(SIZE_T_INVALID), rightCellIdx(SIZE_T_INVALID),
    topCellIdx(SIZE_T_INVALID), bottomCellIdx(SIZE_T_INVALID) {}
