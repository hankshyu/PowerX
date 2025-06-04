//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        06/04/2025 17:35:06
//  Module Name:        fpoint.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A flen_t data type of boost::geometry::model::d2::point_xy implementation
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
#include "fpoint.hpp"

std::ostream &operator<<(std::ostream &os, const FPoint &c) {
    return os << "fp(" << c.x() << ", " << c.y() << ")";
}

size_t std::hash<FPoint>::operator()(const FPoint &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.x());
    boost::hash_combine(seed, key.y());
    return seed;
}

size_t boost::hash<FPoint>::operator()(const FPoint &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.x());
    boost::hash_combine(seed, key.y());
    return seed;
}

flen_t calManhattanDistance(const FPoint &c1, const FPoint &c2) noexcept {
    return std::abs(c1.x() - c2.x()) + std::abs(c1.y() - c2.y());
}

flen_t calEuclideanDistance(const FPoint &c1, const FPoint &c2) noexcept{
    flen_t dx = c1.x() - c2.x();
    flen_t dy = c1.y() - c2.y();
    return std::sqrt(dx * dx + dy * dy);
}

flen_t calDistanceSquared(const FPoint &c1, const FPoint &c2) noexcept{
    flen_t dx = c1.x() - c2.x();
    flen_t dy = c1.y() - c2.y();
    return dx * dx + dy * dy;
}
