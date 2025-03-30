//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/30/2025 13:46:01
//  Module Name:        orderedSegment.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        Different from the Boost provided Segment, orderedSegment 
//                      assures that low < high
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//
//////////////////////////////////////////////////////////////////////////////////


// Dependencies
// 1. C++ STL:
#include <ostream>

// 2. Boost Library:
#include "segment.hpp"

// 3. Texo Library:
#include "orderedSegment.hpp"
#include "units.hpp"
#include "segment.hpp"
#include "cord.hpp"

OrderedSegment::OrderedSegment(): mLow(Cord(0, 0)), mHigh(Cord(0, 0)) {

}
OrderedSegment::OrderedSegment(const Cord &low, const Cord &high): mLow(low), mHigh(high) {
    if(low > high){
        std::swap(mLow, mHigh);
    }
}

OrderedSegment::operator Segment() const{
    return Segment(mLow, mHigh);
}

bool OrderedSegment::operator == (const OrderedSegment &comp) const{
    return (this->mLow == comp.getLow()) && (this->mHigh == comp.getHigh());
}


std::ostream &operator << (std::ostream &os, const OrderedSegment &odsm){
    return os << "OS[" << odsm.mLow << " -- " << odsm.mHigh << "]";
}


// OrderedSegment class hash functions
size_t std::hash<OrderedSegment>::operator()(const OrderedSegment &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.getLow());
    boost::hash_combine(seed, key.getHigh());

    return seed;
}


size_t boost::hash<OrderedSegment>::operator()(const OrderedSegment &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.getLow());
    boost::hash_combine(seed, key.getHigh());

    return seed;
}


