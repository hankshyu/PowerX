//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        02/23/2025 00:15:48
//  Module Name:        interval.hpp
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
//  02/21/2025:         First implementation of interval concept in Boost Library using 
//                      self constructed class. Full implementation of documented members
//
//  02/28/2025:         Switch back to typedef implementation
//
//////////////////////////////////////////////////////////////////////////////////

#ifndef __INTERVAL_H__
#define __INTERVAL_H__

// Dependencies
// 1. C++ STL:
#include <ostream>

// 2. Boost Library:
#include "boost/polygon/polygon.hpp"

// 3. Texo Library:
#include "units.hpp"


typedef boost::polygon::interval_data<len_t> Interval;

std::ostream &operator<<(std::ostream &os, const Interval &intv);

// Interval class hash function implementations
namespace std {
    template <>
    struct hash<Interval> {
        size_t operator()(const Interval &key) const;
    };

}  // namespace std

namespace boost {
    template <>
    struct hash<Interval> {
        size_t operator()(const Interval &key) const;
    };

}  // namespace boost


#endif  // __INTERVAL_H__