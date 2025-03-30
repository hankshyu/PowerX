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

#ifndef __ORDEREDSEGMENT_H__
#define __ORDEREDSEGMENT_H__

// Dependencies
// 1. C++ STL:
#include <ostream>

// 2. Boost Library:
#include "boost/functional/hash.hpp"

// 3. Texo Library:
#include "units.hpp"
#include "segment.hpp"
#include "cord.hpp"

class OrderedSegment{
private:
    Cord mLow;
    Cord mHigh;

public:
    OrderedSegment();
    explicit OrderedSegment(const Cord &low, const Cord &high);

    // conversion operator that allows Line to be casted to Segment
    operator Segment() const;
    bool operator == (const OrderedSegment &comp) const;
    inline Cord getLow() const {return mLow;}
    inline Cord getHigh() const {return mHigh;}

    friend std::ostream &operator << (std::ostream &os, const OrderedSegment &odsm);


};

// OrderedSegment class hash functions
namespace std{
    template<>
    struct hash<OrderedSegment>{
        size_t operator()(const OrderedSegment &key) const;
    };
}

namespace boost{
    template<>
    struct hash<OrderedSegment>{
        size_t operator()(const OrderedSegment &key) const;
    };
}

#endif // __ORDEREDSEGMENT_H__