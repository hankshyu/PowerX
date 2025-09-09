//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        09/06/2025 13:55:21
//  Module Name:        FOrderedSegment.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A floating point version of the OrderedSegment Class
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//
//////////////////////////////////////////////////////////////////////////////////


// Dependencies
// 1. C++ STL:
#include <ostream>

// 2. Boost Library:
#include "boost/functional/hash.hpp"

// 3. Texo Library:
#include "forderedSegment.hpp"
#include "units.hpp"
#include "fcord.hpp"

FOrderedSegment::FOrderedSegment(): mLow(FCord(0, 0)), mHigh(FCord(0, 0)) {

}

FOrderedSegment::FOrderedSegment(const FCord &low, const FCord &high): mLow(low), mHigh(high) {
    if(low > high){
        std::swap(mLow, mHigh);
    }
}


bool FOrderedSegment::operator == (const FOrderedSegment &comp) const{
    return (this->mLow == comp.getLow()) && (this->mHigh == comp.getHigh());
}


std::ostream &operator << (std::ostream &os, const FOrderedSegment &odsm){
    return os << "FOS[" << odsm.mLow << " -- " << odsm.mHigh << "]";
}


// OrderedSegment class hash functions
size_t std::hash<FOrderedSegment>::operator()(const FOrderedSegment &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.getLow());
    boost::hash_combine(seed, key.getHigh());

    return seed;
}


size_t boost::hash<FOrderedSegment>::operator()(const FOrderedSegment &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.getLow());
    boost::hash_combine(seed, key.getHigh());

    return seed;
}