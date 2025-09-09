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

#ifndef __FORDEREDSEGMENT_H__
#define __FORDEREDSEGMENT_H__

// Dependencies
// 1. C++ STL:
#include <ostream>

// 2. Boost Library:
#include "boost/functional/hash.hpp"

// 3. Texo Library:
#include "units.hpp"
#include "fcord.hpp"

class FOrderedSegment{
private:
    FCord mLow;
    FCord mHigh;

public:
    FOrderedSegment();
    explicit FOrderedSegment(const FCord &low, const FCord &high);


    bool operator == (const FOrderedSegment &comp) const;
    inline FCord getLow() const {return mLow;}
    inline FCord getHigh() const {return mHigh;}

    friend std::ostream &operator << (std::ostream &os, const FOrderedSegment &odsm);
};

// OrderedSegment class hash functions
namespace std{
    template<>
    struct hash<FOrderedSegment>{
        size_t operator()(const FOrderedSegment &key) const;
    };
}

namespace boost{
    template<>
    struct hash<FOrderedSegment>{
        size_t operator()(const FOrderedSegment &key) const;
    };
}

#endif // __FORDEREDSEGMENT_H__