//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        06/05/2025 00:41:5
//  Module Name:        fbox.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A FPoint based box model in boost::geometry
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//////////////////////////////////////////////////////////////////////////////////

#ifndef __FBOX_H__
#define __FBOX_H__

// Dependencies
// 1. C++ STL:
#include <ostream>

// 2. Boost Library:
#include "boost/geometry.hpp"
#include "boost/geometry/geometries/box.hpp"
#include "boost/functional/hash.hpp"

// 3. Texo Library:
#include "units.hpp"
#include "fpoint.hpp"

typedef boost::geometry::model::box<FPoint> FBox;

std::ostream &operator<<(std::ostream &os, const FBox &b);

// Cord class hash function implementations
namespace std {
    template <>
    struct hash<FBox> {
        size_t operator()(const FBox &key) const;
    };

}  // namespace std

namespace boost {
    template <>
    struct hash<FBox> {
        size_t operator()(const FBox &key) const;
    };

}  // namespace boost

namespace fbox{
    inline flen_t getWidth(const FBox &b){
        return b.max_corner().x() - b.min_corner().x();
    }

    inline flen_t getHeight(const FBox &b){
        return b.max_corner().y() - b.min_corner().y();
    }

    inline farea_t getArea(const FBox &b){
        return boost::geometry::area(b);
    }

    // considered Touch = true
    inline bool hasIntersect(const FBox &box1, const FBox &box2){
        return boost::geometry::intersects(box1, box2);
    }
    
    inline bool isContained(const FPoint &point, const FBox &box, bool consierTouch=true){
        return (consierTouch)? boost::geometry::covered_by(point, box) : boost::geometry::within(point, box);
    }
}


#endif  // __FBOX_H__