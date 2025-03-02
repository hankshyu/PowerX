//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/01/2025 23:23:30
//  Module Name:        tile.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        Basic element for corner stitching, a rectangle with 4 pointers
//                      pointing at neighboring tiles
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//  03/01/2025:         Remove copy constructor, use default
//  03/01/2025:         Remove checkCordInTile method, which exists under rec namespace,
//                      added several utility functions
//////////////////////////////////////////////////////////////////////////////////

#ifndef __TILE_H__
#define __TILE_H__

// Dependencies
// 1. C++ STL:
#include <ostream>

// 2. Boost Library:
#include "boost/polygon/polygon.hpp"

// 3. Texo Library:
#include "units.hpp"
#include "cord.hpp"
#include "rectangle.hpp"

enum class tileType{
    BLANK, BLOCK, OVERLAP
};

std::ostream &operator << (std::ostream &os, const enum tileType &t);

class Tile{
private:
    tileType mType;
    Rectangle mRectangle;

public:
    Tile *rt, *tr, *bl, *lb;

    explicit Tile();
    explicit Tile(tileType t, const Rectangle &rect);
    explicit Tile(tileType t, const Cord &ll, len_t w, len_t h);
    explicit Tile(tileType t, const Cord &ll, const Cord &ur);

    bool operator == (const Tile &comp) const;
    
    tileType getType() const;
    Rectangle getRectangle() const;

    void setType(tileType type);
    void setWidth(len_t width);
    void setHeight(len_t height);
    void setLowerLeft(const Cord &lowerLeft);
    void setRectangle(const Rectangle &rec);

    len_t getWidth() const;
    len_t getHeight() const;
    area_t getArea() const;
    double calculateAspectRatio() const;
    FCord calculateCentre() const;

    len_t getXLow() const;
    len_t getXHigh() const; 
    len_t getYLow() const;
    len_t getYHigh() const;
        
    Cord getLowerLeft() const;
    Cord getUpperLeft() const;
    Cord getLowerRight() const;
    Cord getUpperRight() const;
    
    friend std::ostream &operator << (std::ostream &os, const Tile &t);


};
    
// Implement hash function for map and set data structure
namespace std{
    template<>
    struct hash<Tile>{
        size_t operator()(const Tile &key) const;
    };
}  // namespace std

namespace boost {
    template<>
    struct hash<Tile>{
        size_t operator()(const Tile &key) const;
    };

} // namespace boost




#endif // __TILE_H__