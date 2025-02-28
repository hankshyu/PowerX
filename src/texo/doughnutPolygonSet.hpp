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
        boost::polygon::extents<DoughnutPolygonSet, Rectangle>(bbox, dp);
        return bbox;
    }

    inline size_t getShapesCount(const DoughnutPolygonSet &dpSet){
        return dpSet.size();
    }

    inline size_t getHolesCount(const DoughnutPolygonSet &dpSet){
        return dpSet.size_holes();
    }

    // len_t calMinInnerWidth(const DoughnutPolygonSet &dpSet);


    inline void diceIntoRectangles(const DoughnutPolygonSet &dpSet, std::vector<Rectangle> &fragments){
        boost::polygon::get_rectangles(fragments, dpSet);
    }

    // inline void diceIntoMaxRectangles(const DoughnutPolygonSet &dpSet, std::vector<Rectangle> &fragments){
    //     boost::polygon::get_max_rectangles(fragments, dpSet);
    // }

    // inline bool oneShape(const DoughnutPolygonSet &dpSet){
    //     return (dpSet.size() == 1);
    // }

    // inline bool noHole(const DoughnutPolygonSet &dpSet){

    //     for(int i = 0; i < dpSet.size(); ++i){
    //         DoughnutPolygon curSegment = dpSet[i];
    //         if(curSegment.begin_holes() != curSegment.end_holes()) return false;
    //     }

    //     return true;
    // }






}

#endif // __DOUGHNUTPOLYGONSET_H__