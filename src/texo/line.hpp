//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/02/2025 20:49:55
//  Module Name:        line.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        Two coordinates(2D point) in a 90 degress rectilinear coordinate system
//                      composes of an unique line, a line could only be horizontal or vertical.
//                      The system preserves that mLow is beneath/left of mHigh
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//  03/02/2025:         Adds constructor from Segment to line, swithc to rule of zero
//  03/02/2025:         Adds boost library hashing function
//////////////////////////////////////////////////////////////////////////////////

#ifndef __LINE_H__
#define __LINE_H__

// Dependencies
// 1. C++ STL:
#include <ostream>

// 2. Boost Library:
#include "boost/functional/hash.hpp"

// 3. Texo Library:
#include "units.hpp"
#include "isotropy.hpp"
#include "segment.hpp"
#include "cord.hpp"


class Line{
private:
    Cord mLow;
    Cord mHigh;
    Orientation2D mOrient;

public:

    Line();
    explicit Line(const Cord &low, const Cord &high);


    // conversion operator that allows Line to be casted to Segment
    operator Segment() const;

    bool operator == (const Line &comp) const;

    Cord getLow() const;
    Cord getHigh() const;
    Orientation2D getOrient() const;


    // if the line "touches the tile", return yes even if it touches the right/top border
    // bool inTile(Tile *tile) const;

    friend std::ostream &operator << (std::ostream &os, const Line &line);

};

// Line class hash functions
namespace std{
    template<>
    struct hash<Line>{
        size_t operator()(const Line &key) const;
    };
}

namespace boost{
    template<>
    struct hash<Line>{
        size_t operator()(const Line &key) const;
    };
}


#endif  // #define __LINE_H__