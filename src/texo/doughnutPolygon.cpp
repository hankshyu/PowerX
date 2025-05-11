//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        02/25/2025 21:11:57
//  Module Name:        doughnutPolygon.cpp
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
//  02/28/2025:         Remove Bounding Box calculating implementations, switch to
//                      Boost Library function
//////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:
#include <ostream>
#include <unordered_set>
#include <algorithm>

// 2. Boost Library:
#include "boost/polygon/polygon.hpp"

// 3. Texo Library:
#include "doughnutPolygon.hpp"
#include "cord.hpp"
#include "isotropy.hpp"

std::ostream &operator << (std::ostream &os, const DoughnutPolygon &dp){

    boost::polygon::direction_1d direction = boost::polygon::winding(dp);
    
    
    if(direction == boost::polygon::direction_1d_enum::CLOCKWISE){
        for(auto it = dp.begin(); it != dp.end(); ++it){
            os << *it << " ";
        }
    }else{
        std::vector<Cord> buffer;
        for(auto it = dp.begin(); it != dp.end(); ++it){
            buffer.push_back(*it);
        }
        for(std::vector<Cord>::reverse_iterator it = buffer.rbegin(); it != buffer.rend(); ++it){
            os << *it << " ";
        }
    }

    return os;

}

void dp::acquireWinding(const DoughnutPolygon &rectilinearShape, std::vector<Cord> &winding, Direction1D direction){
    
    boost::polygon::direction_1d actualDirection = boost::polygon::winding(rectilinearShape);
    
    if(direction == actualDirection){
        for(auto it = rectilinearShape.begin(); it != rectilinearShape.end(); ++it){
            winding.push_back(*it);
        }
    }else{
        std::vector<Cord> buffer;
        for(auto it = rectilinearShape.begin(); it != rectilinearShape.end(); ++it){
            buffer.push_back(*it);
        }
        for(std::vector<Cord>::reverse_iterator it = buffer.rbegin(); it != buffer.rend(); ++it){
            winding.push_back(*it);
        }
    }
}
std::vector<Cord> dp::getContainedCords(const DoughnutPolygon &dp){
    std::vector<Cord> containedCords;

    Rectangle bbox = getBoundingBox(dp);
    for(int j = rec::getYL(bbox); j <= rec::getYH(bbox); ++j){
        for(int i = rec::getXL(bbox); i <= rec::getXH(bbox); ++i){
            Cord c(i, j);
            if(boost::polygon::contains(dp, c, true)) containedCords.emplace_back(c);
        }
    }

    return containedCords;
}

std::vector<Cord> dp::getContainedGrids(const DoughnutPolygon &dp){
    std::vector<Cord> containedGrids;

    // using namespace boost::polygon::operators;
    
    std::vector<DoughnutPolygon> dpset;
    dpset.push_back(dp);
    // dpset += dp;
    std::vector<Rectangle> fragments;
    boost::polygon::get_rectangles(fragments, dpset);

    for(const Rectangle &rect : fragments){
        Cord rectll(rec::getLL(rect));
        for(int j = 0; j < rec::getHeight(rect); ++j){
            for(int i = 0; i < rec::getWidth(rect); ++i){
                containedGrids.emplace_back(Cord(rectll.x() + i, rectll.y() + j));
            }
        }
    }

    return containedGrids;
}