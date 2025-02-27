//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        02/21/2025 23:19:18
//  Module Name:        cord.hpp
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
//  02/21/2025:         Change to suggested way to register as as a model of point concept
//                      in boost library. Full implementation of documented members.
//
//  02/22/2025:         Add conversion operator from Cord to FCord
//
//
//  02/28/2025:         Switch back to typedef implementation
//////////////////////////////////////////////////////////////////////////////////

#ifndef __CORD_H__
#define __CORD_H__

// Dependencies
// 1. C++ STL:
#include <ostream>

// 2. Boost Library:
#include "boost/polygon/polygon.hpp"
#include "boost/functional/hash.hpp"

// 3. Texo Library:
#include "units.hpp"
#include "fcord.hpp"

typedef boost::polygon::point_data<len_t> Cord;

std::ostream &operator<<(std::ostream &os, const Cord &c);

// Cord class hash function implementations
namespace std {
    template <>
    struct hash<Cord> {
        size_t operator()(const Cord &key) const;
    };

}  // namespace std

namespace boost {
    template<>
    struct hash<Cord>{
        size_t operator()(const Cord &key) const;
    };

} // namespace boost

len_t calManhattanDistance(const Cord &c1, const Cord &c2) noexcept;
flen_t calEuclideanDistance(const Cord &c1, const Cord &c2) noexcept;
len_t calDistanceSquared(const Cord &c1, const Cord &c2) noexcept;

#endif  // __CORD_H__
