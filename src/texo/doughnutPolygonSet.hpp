//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        02/25/2025 22:12:45
//  Module Name:        doughnutPolygonSet.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A std::vector containers of DoughnutPolygon to form a 
//                      Polygon 90 Set Concept
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:           
//  02/28/2025:         Add legality checking and utility functions to dps namespace
//
//////////////////////////////////////////////////////////////////////////////////

#ifndef __DOUGHNUTPOLYGONSET_H__
#define __DOUGHNUTPOLYGONSET_H__



// Dependencies
// 1. C++ STL:
#include <ostream>
#include <vector>

// 2. Boost Library:
#include "boost/polygon/polygon.hpp"

// 3. Texo Library:
#include "doughnutPolygon.hpp"
#include "isotropy.hpp"

typedef std::vector<DoughnutPolygon> DoughnutPolygonSet;

enum class doughnutPolygonSetIllegalType{
    DPS_LEGAL, DPS_HOLE, DPS_TWO_SHAPE, DPS_INNER_WIDTH
};

std::ostream &operator << (std::ostream &os, const doughnutPolygonSetIllegalType &t);

namespace dps{

    inline area_t getArea(const DoughnutPolygonSet &dpSet){
        return area_t(boost::polygon::area(dpSet));
    }

    inline Rectangle getBoundingBox(const DoughnutPolygonSet &dpSet){
        Rectangle bbox;
        boost::polygon::extents(bbox, dpSet);
        return bbox;
    }

    inline size_t getShapesCount(const DoughnutPolygonSet &dpSet){
        return dpSet.size();
    }


    len_t calMinInnerWidth(const DoughnutPolygonSet &dpSet);

    // dice geometry of an object into non overlapping rectangles, default vertical orientation 
    inline void diceIntoRectangles(const DoughnutPolygonSet &dpSet, std::vector<Rectangle> &fragments, Orientation2D orient = eOrientation2D::VERTICAL){
        boost::polygon::get_rectangles(fragments, dpSet, orient);
    }

    inline bool isOneShape(const DoughnutPolygonSet &dpSet){
        return (dpSet.size() == 1);
    }

    inline bool isHoleFree(const DoughnutPolygonSet &dpSet){

        for(int i = 0; i < dpSet.size(); ++i){
            DoughnutPolygon curSegment = dpSet[i];
            if(curSegment.begin_holes() != curSegment.end_holes()) return false;
        }

        return true;
    }

}

#endif // __DOUGHNUTPOLYGONSET_H__