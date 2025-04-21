//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        02/21/2025 23:00:39
//  Module Name:        segment.hpp
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
//  02/28/2025:         Change class name from line to segment. Using typedef implementation
//////////////////////////////////////////////////////////////////////////////////｀｀
#ifndef __SEGMENT_H__
#define __SEGMENT_H__

// Dependencies
// 1. C++ STL:
#include <ostream>

// 2. Boost Library:
#include "boost/polygon/polygon.hpp"

// 3. Texo Library:
#include "cord.hpp"

typedef boost::polygon::segment_data<len_t> Segment;

std::ostream &operator<<(std::ostream &os, const Segment &seg);

// Segment class hash function implementations
namespace std{
    template<>
    struct hash<Segment>{
        size_t operator()(const Segment &key) const;
    };
} // namespace std

namespace boost{
    template<>
    struct hash<Segment>{
        size_t operator()(const Segment &key) const;
    };
} // namespace boost

namespace seg{
    inline bool intersects(const Segment& segment1, const Segment& segment2, bool considerTouch = true){
        return boost::polygon::intersects(segment1, segment2, considerTouch);
    }
}
#endif  // #define __SEGMENT_H__