//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        02/25/2025 21:11:57
//  Module Name:        doughnutPolygon.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A len_t data type polygon_90_with_holes_data. Uses Boost
//                      Library provided data structure
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//  02/25/2025:         Change function acquireWinding and acquireBoundingBox's I/O
//                      For easier access
//
//  02/28/2025:         Add often used utility functions to dp namespace, replace bounding
//                      box calculating function to Boost library native functions
//////////////////////////////////////////////////////////////////////////////////

#ifndef __DOUGHNUTPOLYGON_H__
#define __DOUGHNUTPOLYGON_H__

// Dependencies
// 1. C++ STL:
#include <ostream>

// 2. Boost Library:
#include "boost/polygon/polygon.hpp"

// 3. Texo Library:
#include "units.hpp"
#include "cord.hpp"
#include "isotropy.hpp"
#include "rectangle.hpp"

typedef boost::polygon::polygon_90_with_holes_data<len_t> DoughnutPolygon;

std::ostream &operator << (std::ostream &os, const DoughnutPolygon &dp);

namespace dp{
    // Returns the number of edges in the outer shell, excludes sizes of the holes
    inline size_t getEdgeCount(const DoughnutPolygon &dp){
        return boost::polygon::size<DoughnutPolygon>(dp);
    }

    inline len_t getPerimeter(const DoughnutPolygon &dp){
        return boost::polygon::perimeter<DoughnutPolygon>(dp);
    }

    inline area_t getArea(const DoughnutPolygon &dp){
        return boost::polygon::area<DoughnutPolygon>(dp);
    }

    inline size_t getHoleCount(const DoughnutPolygon &dp){
        return dp.size_holes();
    }

    // inline bool isContained(const DoughnutPolygon &dp, const Cord &c, bool considerTouch = true){
    //     return boost::polygon::contains<DoughnutPolygon, Cord>(dp, c, considerTouch);
    // }

    inline Rectangle getBoundingBox(const DoughnutPolygon &dp){
        Rectangle bbox;
        boost::polygon::extents(bbox, dp);
        return bbox;
    }

    void acquireWinding(const DoughnutPolygon &rectilinearShape, std::vector<Cord> &winding, Direction1D direction = eDirection1D::CLOCKWISE);

}

#endif