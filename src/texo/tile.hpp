//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        02/25/2025 00:21:49
//  Module Name:        tile.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A parent class inherited from rectangle class that serves
//                      as a building block of the corner stitching system. Contains
//                      pointers connecting to adjacent tiles
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//  02/25/2025:         Rewrite tile to be a parent class of Rectangle class
//  02/25/2025:         Add eTileType::EMPTY for initialized Tile
// 
//
//////////////////////////////////////////////////////////////////////////////////｀

#ifndef __TILE_H__
#define __TILE_H__

// Dependencies
// 1. C++ STL:
#include <ostream>

// 2. Boost Library:
#include "boost/polygon/polygon.hpp"

// 3. Texo Library:
#include "rectangle.hpp"

enum class eTileType{
    EMPTY, BLANK, BLOCK, OVERLAP
};

std::ostream &operator << (std::ostream &os, const eTileType &t);


class Tile : public Rectangle{
private:
    eTileType m_type;
    
public:
    Tile *rt, *tr, *bl, *lb;

    Tile();
    explicit Tile(const eTileType &t, const Cord &ll, len_t w, len_t h);
    explicit Tile(const eTileType &t, const Cord &ll, const Cord &ur);

    // for adapting to Rectangle parent class
    explicit Tile(len_t xl, len_t yl, len_t xh, len_t yh);
    explicit Tile(const Interval &horizontal_interval, const Interval &vertical_interval);


    bool operator == (const Tile &other) const;
    bool operator != (const Tile &other) const;
    
    eTileType getType() const;
    void setType(const eTileType &newType);

    friend void swap(Tile &first, Tile &second) noexcept;
    friend std::ostream &operator << (std::ostream &os, const Tile &t);

};

namespace boost {namespace polygon{
    template <>
    struct geometry_concept<Tile> { typedef rectangle_concept type; };

    template <>
    struct rectangle_traits<Tile> {
        typedef Cord coordinate_type;
        typedef Interval interval_type;
        static inline interval_type get(const Tile& tile, orientation_2d orient) {
            return tile.get(orient); }
    };

    template <>
    struct rectangle_mutable_traits<Tile> {

        static inline void set(Tile& tile, orientation_2d orient, const Interval& interval) {
            tile.set(orient, interval); }

        static inline Tile construct(const Interval& interval_horizontal, const Interval& interval_vertical) {
            return Tile(interval_horizontal, interval_vertical); }
    };
}}

// Tile class hash function implementations
namespace std{
    template<>
    struct hash<Tile>{
        size_t operator()(const Tile &key) const;
    };
} // namespace std

namespace boost{
    template<>
    struct hash<Tile>{
        size_t operator()(const Tile &key) const;
    };
} // namespace boost

#endif // __TILE_H__

