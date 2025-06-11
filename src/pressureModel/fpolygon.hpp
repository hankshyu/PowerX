//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        06/24/2025 17:25:41
//  Module Name:        fpolygon.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A FPoint based polygon class in booost::geometry
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//           
//////////////////////////////////////////////////////////////////////////////////

#ifndef __FPOLYGON_H__
#define __FPOLYGON_H__

// Dependencies
// 1. C++ STL:
#include <vector>

// 2. Boost Library:
#include "boost/geometry.hpp"
#include "boost/geometry/geometries/polygon.hpp"

// 3. Texo Library:
#include "units.hpp"
#include "fpoint.hpp"
#include "fbox.hpp"

typedef boost::geometry::model::polygon<FPoint> FPolygon;

namespace fp{

    FPolygon createFPolygon(const std::vector<FPoint> &pts);
    void addHole(FPolygon &poly, const std::vector<FPoint> &pts);

    bool isEmpty(const FPolygon &fpg);
    bool isValid(const FPolygon &fpg);

    FPolygon simplify(FPolygon &fpg, flen_t tolerance = 0.05);
    flen_t getPerimeter(const FPolygon &fpg);
    farea_t getArea(const FPolygon &fpg);
    FBox getBBox(const FPolygon &fpg);

    // count unique points
    size_t getOuterEdgesCount(const FPolygon &fpg);
    std::vector<FPoint> getOuter(const FPolygon &fpg);
    size_t getHoleCount(const FPolygon &fpg);
    FPoint getCentroidIgnoreHoles(const FPolygon &fpg);
    flen_t getShortestDistance(const FPolygon &fpoly1, const FPolygon &fpoly2);

    // considered Touch = true
    bool hasIntersect(const FPolygon &fpoly1, const FPolygon &fpoly2);
    
    bool isContained(const FPoint &point, const FPolygon &fpolygon, bool consierTouch=true);

}

#endif // __FPOLYGON_H__
