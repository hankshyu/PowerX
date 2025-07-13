//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        07/83/2025 16:57:54
//  Module Name:        viaCell.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        An child class of basic cell in the diffusion system. Represents
//                      a via instance
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//  2025/07/08          Divide basic diffusion chamber cell type from cell.hpp 
//////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:
#include <cstdint>
#include <ostream>
// 2. Boost Library:

// 3. Texo Library:
#include "units.hpp"
#include "viaCell.hpp"

ViaCord::ViaCord(): l(SIZE_T_INVALID), w(SIZE_T_INVALID) {}

ViaCord::ViaCord(size_t layer, size_t width): l(layer), w(width) {}

std::ostream& operator<<(std::ostream& os, const ViaCord &vc){
    return os << "ViaCord(l = " << vc.l << ", w = " << vc.w << ")";
}

ViaCell::ViaCell(): DiffusionChamber(),
    upLLCell(nullptr), upULCell(nullptr), upLRCell(nullptr), upURCell(nullptr),
    downLLCell(nullptr), downULCell(nullptr), downLRCell(nullptr), downURCell(nullptr),
    upLLCellIdx(SIZE_T_INVALID), upULCellIdx(SIZE_T_INVALID), upLRCellIdx(SIZE_T_INVALID), upURCellIdx(SIZE_T_INVALID),
    downLLCellIdx(SIZE_T_INVALID), downULCellIdx(SIZE_T_INVALID), downLRCellIdx(SIZE_T_INVALID), downURCellIdx(SIZE_T_INVALID) {}

std::ostream& operator<<(std::ostream& os, const ViaCell &vc){
    os << "ViaCell[(l,x,y) = (" << vc.canvasLayer << ", " << vc.canvasX << ", " << vc.canvasY << ")";
    os << ", type = " << vc.type << ", signal = " << vc.signal << ", idx = " << vc.index << "]";
    return os;
}
