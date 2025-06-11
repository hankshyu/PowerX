//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        06/07/2025 15:36:00
//  Module Name:        fmultipolygon.cpp
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

// Dependencies
// 1. C++ STL:

// 2. Boost Library:
#include "boost/geometry.hpp"
#include <boost/geometry/geometries/polygon.hpp>


// 3. Texo Library:
#include "units.hpp"
#include "fpoint.hpp"
#include "fbox.hpp"
#include "fpolygon.hpp"
#include "fmultipolygon.hpp"

bool fmp::isEmpty(const FMultiPolygon& fmpg) {
    return fmpg.empty();
}

bool fmp::isValid(const FMultiPolygon& fmpg) {
    return boost::geometry::is_valid(fmpg);
}

flen_t fmp::getPerimeter(const FMultiPolygon& fmpg) {
    return boost::geometry::perimeter(fmpg);
}

farea_t fmp::getArea(const FMultiPolygon& fmpg) {
    return boost::geometry::area(fmpg);
}

FBox fmp::getBBox(const FMultiPolygon& fmpg) {
    FBox box;
    boost::geometry::envelope(fmpg, box);
    return box;
}

bool fmp::hasIntersect(const FMultiPolygon& fmpoly1, const FMultiPolygon& fmpoly2) {
    return boost::geometry::intersects(fmpoly1, fmpoly2);
}

bool fmp::isContained(const FPoint& point, const FMultiPolygon& fmpolygon, bool considerTouch) {
    return considerTouch
        ? boost::geometry::covered_by(point, fmpolygon)
        : boost::geometry::within(point, fmpolygon);
}
