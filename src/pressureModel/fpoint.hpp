//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        06/04/2025 17:27:19
//  Module Name:        fpoint.hpp
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

#ifndef __FPOINT_H__
#define __FPOINT_H__

// Dependencies
// 1. C++ STL:
#include <ostream>

// 2. Boost Library:
#include "boost/geometry/geometries/point_xy.hpp"
#include "boost/functional/hash.hpp"

// 3. Texo Library:
#include "units.hpp"

typedef boost::geometry::model::d2::point_xy<flen_t> FPoint;

std::ostream &operator<<(std::ostream &os, const FPoint &c);

// Cord class hash function implementations
namespace std {
    template <>
    struct hash<FPoint> {
        size_t operator()(const FPoint &key) const;
    };

}  // namespace std

namespace boost {
    template <>
    struct hash<FPoint> {
        size_t operator()(const FPoint &key) const;
    };

}  // namespace boost

flen_t calManhattanDistance(const FPoint &c1, const FPoint &c2) noexcept;
flen_t calEuclideanDistance(const FPoint &c1, const FPoint &c2) noexcept;
flen_t calDistanceSquared(const FPoint &c1, const FPoint &c2) noexcept;

#endif  // __FPOINT_H__