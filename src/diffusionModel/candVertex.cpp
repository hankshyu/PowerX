//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        01/01/2025 14:11:05
//  Module Name:        candVertex.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A holder data structure that logs the pointer to data structure,
//                      gain value and refresh date of the grid
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
/////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:
#include <ostream>

// 2. Boost Library:
#include "boost/functional/hash.hpp"

// 3. Texo Library:
#include "diffusionChamber.hpp"
#include "candVertex.hpp"

CandVertex::CandVertex(): dc(nullptr), gain(-1) {}

CandVertex::CandVertex(DiffusionChamber *diffChamber): dc(diffChamber), gain(-1) {}

CandVertex::CandVertex(DiffusionChamber *diffChamber, double gain): dc(diffChamber), gain(gain) {}

bool CandVertex::operator<(const CandVertex &other) const {
    return gain < other.gain;
}

size_t std::hash<CandVertex>::operator()(const CandVertex &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.dc);
    return seed;
}