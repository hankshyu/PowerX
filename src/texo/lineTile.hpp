//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/02/2025 22:26:56
//  Module Name:        lineTile.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A combination of Line(horizontal or vertical) and Tile data
//                      structure
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:           
//  03/02/2025:         Change impelentation to rule of zero, use defualt copy constructor
//                      and copy assignment operator
//
//////////////////////////////////////////////////////////////////////////////////

#ifndef __LINETILE_H__
#define __LINETILE_H__

// Dependencies
// 1. C++ STL:
#include <ostream>

// 2. Boost Library:

// 3. Texo Library:
#include "line.hpp"
#include "tile.hpp"

enum class lineTileDirection{
    LEFT, RIGHT, DOWN, UP, CENTRE
};

std::ostream &operator << (std::ostream &os, const lineTileDirection &ltd);

class LineTile{
private:
    Line mLine;
    Tile *mTile;
    lineTileDirection mDirection;

public:
    LineTile();
    explicit LineTile(const Line &line, Tile *tile);

    bool operator == (const LineTile &comp) const;
    
    Line getLine() const;
    Tile *getTile() const;
    lineTileDirection getDirection() const;
    
    friend std::ostream &operator << (std::ostream &os, const LineTile &t);
};

// Cord class hash function implementations
namespace std{
    template<>
    struct hash<LineTile>{
        size_t operator()(const LineTile &key) const;
    };
} // namespace std

namespace boost{
    template<>
    struct hash<LineTile>{
        size_t operator()(const LineTile &key) const;
    };
} // namespace boost

#endif // __LINETILE_H__



