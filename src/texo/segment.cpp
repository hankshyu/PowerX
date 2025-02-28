//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        02/23/2025 23:12:09
//  Module Name:        line.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A len_t (Cord) data type of boost::segment_concept implementation
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//
//////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:

// 2. Boost Library:
#include "boost/functional/hash.hpp"

// 3. Texo Library:
#include "segment.hpp"
#include "units.hpp"


std::ostream &operator<<(std::ostream &os, const Segment &seg){
    return os << "S[" << seg.low() << " -- " << seg.high() << "]";
}

size_t std::hash<Segment>::operator()(const Segment &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.low());
    boost::hash_combine(seed, key.high());
    return seed;
}

size_t boost::hash<Segment>::operator()(const Segment &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.low());
    boost::hash_combine(seed, key.high());
    return seed;
}
