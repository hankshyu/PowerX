//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        06/07/2025 17:12:24
//  Module Name:        softBody.cpp
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


#include "softBody.hpp"

// Constructor
SoftBody::SoftBody(int layer, SignalType sig, double expectCurrent, int id): layer(layer), sigType(sig), expectCurrent(expectCurrent), id(id) {
    
    //calculate initial pressure
    pressure = 0;

}
// Getter for signal type
SignalType SoftBody::getSigType() {
    return sigType;
}

// Getter for expected current
double SoftBody::getExpectCurrent() {
    return expectCurrent;
}