//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        06/05/2025 00:55:19
//  Module Name:        fbox.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:
#include <cmath>
// 2. Boost Library:
#include "boost/functional/hash.hpp"

// 3. Texo Library:
#include "fbox.hpp"

std::ostream &operator<<(std::ostream &os, const FBox &b) {
    os << "fbox[" << b.min_corner();
    os << " W=" << fbox::getWidth(b);
    os << " H=" << fbox::getHeight(b);
    os << " " << b.max_corner() <<"]";

    return os;
}

size_t std::hash<FBox>::operator()(const FBox &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.min_corner());
    boost::hash_combine(seed, key.max_corner());
    return seed;
}

size_t boost::hash<FBox>::operator()(const FBox &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.min_corner());
    boost::hash_combine(seed, key.max_corner());
    return seed;
}