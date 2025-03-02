//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/02/2025 21:01:51
//  Module Name:        line.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        Two coordinates(2D point) composes of an unique line, different
//                      from Segment where two coordinates exist no order, Line preservers
//                      the order of line.low <= line.high
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//  03/02/2025:         Add constructor implentations
//////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:
#include <ostream>
#include <cassert>

// 2. Boost Library:
#include "boost/polygon/polygon.hpp"
#include "boost/functional/hash.hpp"

// 3. Texo Library:
#include "line.hpp"
#include "units.hpp"
#include "isotropy.hpp"
#include "segment.hpp"
#include "cord.hpp"


Line::Line()
    : mLow(Cord(0, 0)), mHigh(Cord(LEN_T_MAX, 0)), mOrient(eOrientation2D::HORIZONTAL){
}

Line::Line(const Cord &low, const Cord &high) {
    
    // Line could only be vertical or horizontal
    assert((low.x() == high.x()) || (low.y() == high.y()));
    
    if(low.x() == high.x()){
        mOrient = eOrientation2D::VERTICAL;
        if(low.y() <= high.y()){
            mLow = low;
            mHigh = high;
        }else{
            mLow = high;
            mHigh = low;
        }
    }else{ // low.y() == high.y()
        mOrient = eOrientation2D::HORIZONTAL;
        if(low.x() <= high.x()){
            mLow = low;
            mHigh = high;
        }else {
            mLow = high;
            mHigh = low;
        }
    }
}

Line::operator Segment() const {
    return Segment(mLow, mHigh);
}

bool Line::operator == (const Line &comp) const {
    return (this->mLow == comp.getLow()) && (this->mHigh == comp.getHigh());
}

Cord Line::getLow() const {
    return this->mLow;
}

Cord Line::getHigh() const {
    return this->mHigh;
}

Orientation2D Line::getOrient() const {
    return this->mOrient;
}

// bool Line::inTile(Tile *tile) const{
//     if(mOrient == eOrientation2D::HORIZONTAL){
//         len_t lineY = mLow.y(); 
// 		len_t lineXLow = mLow.x();
// 		len_t lineXHigh = mHigh.x();

// 		len_t tileXLow = tile->getXLow();
// 		len_t tileXHigh = tile->getXHigh();
// 		len_t tileYLow = tile->getYLow();
// 		len_t tileYHigh = tile->getYHigh();

//         bool yDownInRange = (lineY >= tileYLow);
//         bool yUpInRange = (lineY <= tileYHigh);
//         bool yCordInTile = (yDownInRange && yUpInRange);

//         bool lineToTileLeft = (lineXHigh <= tileXLow);
//         bool lineToTileRight =(lineXLow >= tileXHigh);

//         return yCordInTile && (!(lineToTileLeft || lineToTileRight));

//     }else{ // eOrientation2D::VERTICAL
//         len_t lineX = mLow.x();
//         len_t lineYLow = mLow.y();
//         len_t lineYHigh = mHigh.y();

// 		len_t tileXLow = tile->getXLow();
// 		len_t tileXHigh = tile->getXHigh();
// 		len_t tileYLow = tile->getYLow();
// 		len_t tileYHigh = tile->getYHigh();

//         bool xDownInRange = (lineX >= tileXLow);
//         bool xUpInRange = (lineX <= tileXHigh);
//         bool xCordInTile = (xDownInRange && xUpInRange);

//         bool linetoTileDown = (lineYHigh <= tileYLow);
//         bool linetoTileUp = (lineYLow >= tileYHigh);

//         return xCordInTile && (!(linetoTileDown || linetoTileUp));
//     }

// }

std::ostream &operator<<(std::ostream &os, const Line &line) {
    os << "L[" << line.mLow << " -- ";
    os << ((line.mOrient == eOrientation2D::HORIZONTAL)? "H" : "V");
    os << " -- " << line.mHigh << "]";
    
    return os;
}

size_t std::hash<Line>::operator()(const Line &key) const {
    
    std::size_t seed = 0;
    boost::hash_combine(seed, key.getLow());
    boost::hash_combine(seed, key.getHigh());

    return seed;
}

size_t boost::hash<Line>::operator()(const Line &key) const {
    
    std::size_t seed = 0;
    boost::hash_combine(seed, key.getLow());
    boost::hash_combine(seed, key.getHigh());

    return seed;
}
