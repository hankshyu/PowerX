//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        02/21/2025 23:19:18
//  Module Name:        cord.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A len_t data type of boost::point_concept implementation
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//  02/21/2025:         Reimplement Cord member functions, using rule of zero
//
//  02/28/2025:         Switch back to typedef implementation, remove class-based
//                      attributes and functions
//////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:

// 2. Boost Library:
#include "boost/functional/hash.hpp"

// 3. Texo Library:
#include "cord.hpp"

std::ostream &operator<<(std::ostream &os, const Cord &c) {
    return os << "(" << c.x() << ", " << c.y() << ")";
}

size_t std::hash<Cord>::operator()(const Cord &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.x());
    boost::hash_combine(seed, key.y());
    return seed;
}

size_t boost::hash<Cord>::operator()(const Cord &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.x());
    boost::hash_combine(seed, key.y());
    return seed;
}

len_t calManhattanDistance(const Cord &c1, const Cord &c2) noexcept {
    return boost::polygon::manhattan_distance<Cord, Cord>(c1, c2);
}

flen_t calEuclideanDistance(const Cord &c1, const Cord &c2) noexcept{
    return boost::polygon::euclidean_distance<Cord, Cord>(c1, c2);
}

len_t calDistanceSquared(const Cord &c1, const Cord &c2) noexcept{
    return boost::polygon::distance_squared<Cord, Cord>(c1, c2);
}






