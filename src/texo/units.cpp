//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        02/21/2025 22:53:12
//  Module Name:        units.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        Defines the basic datatypes and some global definitions
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//  02/21/2025:         Remove sector struct related functions
//////////////////////////////////////////////////////////////////////////////////

/*
Header File Dependencies
//  1. C++ STL:         cstdint, climits, ostream
//  2. Boost Library:   boost/polygon/polygon.hpp
//  3. Texo Library:    None
*/

// Dependencies
// 1. C++ STL:
#include <cmath>

// 2. Boost Library:

// 3. Texo Library:
#include "units.hpp"

std::ostream &operator << (std::ostream &os, const quadrant &q){
    switch (q){
        case quadrant::I:
            os << "Quadrant::I";
            break;
        case quadrant::II:
            os << "Quadrant::II";
            break;
        case quadrant::III:
            os << "Quadrant::III";
            break;
        case quadrant::IV:
            os << "quadrant::IV";
            break;
        default:
            break;
    }
    
    return os;
}

angle_t flipAngle(angle_t angle){
    return (angle > 0)? (angle - M_PI) : (angle + M_PI);
}
quadrant translateAngleToQuadrant(angle_t angle){

	if(angle >= 0){
		if(angle <= (M_PI / 2.0)) return quadrant::I;
		else return quadrant::II;
	}else{
		if(angle >= (- M_PI / 2.0)) return quadrant::IV;
		else return quadrant::III;
	}
}
