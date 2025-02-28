//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        02/23/2025 12:31:54
//  Module Name:        interval.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A len_t data type of boost::interval_concept implementation
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//  02/21/2025:         First implementation Interval member functions, using rule of zero
//
//  02/28/2025:         Switch back to typedef implementation, remove class-based
//                      attributes and functions
//////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:

// 2. Boost Library:
#include "boost/functional/hash.hpp"

// 3. Texo Library:
#include "units.hpp"
#include "interval.hpp"


std::ostream &operator<<(std::ostream &os, const Interval &intv){
    return os << "I[L: " << intv.low() << " H: " << intv.high() << "]";
}

size_t std::hash<Interval>::operator()(const Interval &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.low());
    boost::hash_combine(seed, key.high());
    return seed;
}

size_t boost::hash<Interval>::operator()(const Interval &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.low());
    boost::hash_combine(seed, key.high());
    return seed;
}
