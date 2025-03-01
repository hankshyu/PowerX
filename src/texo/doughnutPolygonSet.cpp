//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        02/25/2025 22:13:17
//  Module Name:        doughnutPolygonSet.cpp
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

// Dependencies
// 1. C++ STL:
#include <ostream>

// 2. Boost Library:
#include "boost/polygon/polygon.hpp"

// 3. Texo Library:
#include "doughnutPolygonSet.hpp"

std::ostream &operator << (std::ostream &os, const doughnutPolygonSetIllegalType &t){
    switch (t)
    {
    case doughnutPolygonSetIllegalType::DPS_LEGAL:
        os << "doughnutPolygonSetIllegalType::DPS_LEGAL";
        break;
    case doughnutPolygonSetIllegalType::DPS_HOLE:
        os << "doughnutPolygonSetIllegalType::DPS_HOLE";
        break;
    case doughnutPolygonSetIllegalType::DPS_TWO_SHAPE:
        os << "doughnutPolygonSetIllegalType::DPS_TWO_SHAPE";
        break;
    case doughnutPolygonSetIllegalType::DPS_INNER_WIDTH:
        os << "doughnutPolygonSetIllegalType::DPS_INNER_WIDTH";
        break;
    default:
        break;
    }
   
    return os;
}

len_t dps::calMinInnerWidth(const DoughnutPolygonSet &dpSet){
    
    len_t minInnerWidth = LEN_T_MAX;
    len_t tmpInnerWidth = LEN_T_MAX;
    
    if (dpSet.empty()) return LEN_T_MIN;

    using namespace boost::polygon::operators;
    
    // dice the rectangle horizontally and measure the width 
    std::vector<Rectangle> fragments;
    boost::polygon::get_rectangles(fragments, dpSet, eOrientation2D::HORIZONTAL);
    for(const Rectangle &hrec : fragments){
        tmpInnerWidth = rec::getWidth(hrec);
        if(tmpInnerWidth < minInnerWidth) minInnerWidth = tmpInnerWidth;
    }

    fragments.clear();
    // dice the rectangle vertically and measure the height
    boost::polygon::get_rectangles(fragments, dpSet, eOrientation2D::VERTICAL);
    for(const Rectangle &vrec : fragments){
        tmpInnerWidth = rec::getHeight(vrec);
        if(tmpInnerWidth < minInnerWidth) minInnerWidth = tmpInnerWidth;
    }

    return minInnerWidth;
}