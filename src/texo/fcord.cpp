//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        02/22/2025 22:53:53
//  Module Name:        fcord.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A flen_t data type of boost::point_concept implementation
//                      Note that boost::polygon library doest not well support
//                      floating point type coordinate systems in most data structures
//                      This implementation merely serves for calculation purposes
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//  02/21/2025:         Reimplement FFCord member functions, using rule of zero
//
//  02/28/2025:         Switch back to typedef implementation, remove class-based
//                      attributes and functions
//////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:

// 2. Boost Library:
#include "boost/functional/hash.hpp"

// 3. Texo Library:
#include "fcord.hpp"


std::ostream &operator<<(std::ostream &os, const FCord &c) {
    return os << "f'(" << c.x() << ", " << c.y() << ")";
}

size_t std::hash<FCord>::operator()(const FCord &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.x());
    boost::hash_combine(seed, key.y());
    return seed;
}

size_t boost::hash<FCord>::operator()(const FCord &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.x());
    boost::hash_combine(seed, key.y());
    return seed;
}

flen_t calManhattanDistance(const FCord &c1, const FCord &c2) noexcept {
    return boost::polygon::manhattan_distance<FCord, FCord>(c1, c2);
}

flen_t calEuclideanDistance(const FCord &c1, const FCord &c2) noexcept{
    return boost::polygon::euclidean_distance<FCord, FCord>(c1, c2);
}

flen_t calDistanceSquared(const FCord &c1, const FCord &c2) noexcept{
    return boost::polygon::distance_squared<FCord, FCord>(c1, c2);
}
