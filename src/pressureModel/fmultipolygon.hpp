//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        06/07/2025 15:18:03
//  Module Name:        fmultipolygon.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A FPolygon clustered multipolygon system. holds the result of
//                      boolean operations between FPolygons
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//           
//////////////////////////////////////////////////////////////////////////////////

#ifndef __FMULTIPOLYGON_H__
#define __FMULTIPOLYGON_H__

// Dependencies
// 1. C++ STL:
#include <vector>

// 2. Boost Library:
#include "boost/geometry.hpp"
#include "boost/geometry/geometries/multi_polygon.hpp"

// 3. Texo Library:
#include "units.hpp"
#include "fpoint.hpp"
#include "fbox.hpp"
#include "fpolygon.hpp"

typedef boost::geometry::model::multi_polygon<FPolygon> FMultiPolygon;

namespace fmp{
    bool isEmpty(const FMultiPolygon &fmpg);
    bool isValid(const FMultiPolygon &fmpg);

    flen_t getPerimeter(const FMultiPolygon &fmpg);
    farea_t getArea(const FMultiPolygon &fmpg);
    FBox getBBox(const FMultiPolygon &fmpg);

    // considered Touch = true
    bool hasIntersect(const FMultiPolygon &fmpoly1, const FMultiPolygon &fmpoly2);
    
    bool isContained(const FPoint &point, const FMultiPolygon &fmpolygon, bool consierTouch=true);

}

#endif // __FMULTIPOLYGON_H__