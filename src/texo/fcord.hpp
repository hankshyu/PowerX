//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        02/22/2025 22:47:43
//  Module Name:        fcord.hpp
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
//  02/22/2025:         Change to suggested way to register as as a model of point concept
//                      in boost library. Full implementation of documented members.
//
//  02/28/2025:         Switch back to typedef implementation
//////////////////////////////////////////////////////////////////////////////////

#ifndef __FCORD_H__
#define __FCORD_H__

// Dependencies
// 1. C++ STL:
#include <ostream>

// 2. Boost Library:
#include "boost/polygon/polygon.hpp"
#include "boost/functional/hash.hpp"

// 3. Texo Library:
#include "units.hpp"

typedef boost::polygon::point_data<flen_t> FCord;

std::ostream &operator<<(std::ostream &os, const FCord &c);

    
// Cord class hash function implementations
namespace std {
    template <>
    struct hash<FCord> {
        size_t operator()(const FCord &key) const;
    };

}  // namespace std

namespace boost {
    template <>
    struct hash<FCord> {
        size_t operator()(const FCord &key) const;
    };

}  // namespace boost

flen_t calManhattanDistance(const FCord &c1, const FCord &c2) noexcept;
flen_t calEuclideanDistance(const FCord &c1, const FCord &c2) noexcept;
flen_t calDistanceSquared(const FCord &c1, const FCord &c2) noexcept;

#endif  // __FCORD_H__