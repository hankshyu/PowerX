//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        01/01/2025 14:11:05
//  Module Name:        candVertex.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        Holds the pointer to data structure and it's gain value
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
/////////////////////////////////////////////////////////////////////////////////

#ifndef __CAND_VERTEX_H__
#define __CAND_VERTEX_H__

// Dependencies
// 1. C++ STL:

// 2. Boost Library:

// 3. Texo Library:
#include "diffusionChamber.hpp"


class CandVertex{
public:
    DiffusionChamber *dc;
    double gain;
    
    CandVertex();
    CandVertex(DiffusionChamber *diffChamber);
    CandVertex(DiffusionChamber *diffChamber, double gain);

    bool operator<(const CandVertex& other) const;
};

namespace std {
    template <>
    struct hash<CandVertex> {
        size_t operator()(const CandVertex &key) const;
    };

}  // namespace std


#endif // __CAND_VERTEX_H__