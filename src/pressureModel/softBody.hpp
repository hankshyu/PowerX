//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        06/04/2025 17:09:56
//  Module Name:        softBody.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        The object that expands, shrink and merge due to
//                      pressure-driven forces
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
/////////////////////////////////////////////////////////////////////////////////

#ifndef __SOFT_BODY_H__
#define __SOFT_BODY_H__

// Dependencies
// 1. C++ STL:
#include <vector>
// 2. Boost Library:

// 3. Texo Library:
#include "signalType.hpp"

#include "fpoint.hpp"
#include "fbox.hpp"
#include "fpolygon.hpp"
#include "fmultipolygon.hpp"

class SoftBody{
private:
    int layer;
    SignalType sigType;
    double expectCurrent;
    int id;


public:
    double pressure;
    std::vector<FPoint> contour;
    
    std::vector<FPoint> hardVias;
    FPolygon hardViaFPolygon;

    SoftBody(int layer, SignalType sig, double expectCurrent, int id);

    SignalType getSigType();
    double getExpectCurrent();

};

#endif // __SOFT_BODY_H__