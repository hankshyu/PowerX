//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        06/24/2025 13:50:24
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

// Dependencies
// 1. C++ STL:
#include <ostream>
#include <vector>

// 2. Boost Library:
#include "boost/geometry.hpp"
#include <boost/geometry/geometries/polygon.hpp>


// 3. Texo Library:
#include "units.hpp"
#include "fpoint.hpp"
#include "fbox.hpp"
#include "fpolygon.hpp"

FPolygon fp::createFPolygon(const std::vector<FPoint> &pts){
    FPolygon poly;
    poly.outer().assign(pts.begin(), pts.end());
    boost::geometry::correct(poly);
    return poly;
}

void fp::addHole(FPolygon &poly, const std::vector<FPoint> &pts){
    FPolygon::ring_type hole;
    hole.assign(pts.begin(), pts.end());
    boost::geometry::correct(hole);  // ensures clockwise winding and closure
    poly.inners().push_back(hole);
}

bool fp::isEmpty(const FPolygon &fpg){
    return boost::geometry::is_empty(fpg);
}

bool fp::isValid(const FPolygon &fpg){
    return boost::geometry::is_valid(fpg);
}

FPolygon fp::simplify(FPolygon &fpg, flen_t tolerance){
    FPolygon simplified;
    boost::geometry::simplify(fpg, simplified, tolerance);
    return simplified;
}

flen_t fp::getPerimeter(const FPolygon &fpg){
    return boost::geometry::perimeter(fpg);
}

farea_t fp::getArea(const FPolygon &fpg){
    return boost::geometry::area(fpg);
}

FBox fp::getBBox(const FPolygon &fpg){
    FBox fbbox;
    boost::geometry::envelope(fpg, fbbox);
    return fbbox;
}

// count unique points
size_t fp::getOuterEdgesCount(const FPolygon &fpg){
    return fpg.outer().size() - 1;
}

std::vector<FPoint> fp::getOuter(const FPolygon &fpg){
    return fpg.outer();
}

size_t fp::getHoleCount(const FPolygon &fpg){
    return fpg.inners().size();
}

FPoint fp::getCentroidIgnoreHoles(const FPolygon &fpg){
    FPoint centre;
    boost::geometry::centroid(fpg, centre);
    return centre;
}

flen_t fp::getShortestDistance(const FPolygon &fpoly1, const FPolygon &fpoly2){
    return boost::geometry::distance(fpoly1, fpoly2);
}

// considered Touch = true
bool fp::hasIntersect(const FPolygon &fpoly1, const FPolygon &fpoly2){
    return boost::geometry::intersects(fpoly1, fpoly2);
}

bool fp::isContained(const FPoint &point, const FPolygon &fpolygon, bool consierTouch){
    return (consierTouch)? boost::geometry::covered_by(point, fpolygon) : boost::geometry::within(point, fpolygon);
}