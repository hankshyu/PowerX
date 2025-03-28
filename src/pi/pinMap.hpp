//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/26/2025 22:16:46
//  Module Name:        pinMap.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A data structure wil be extended for micro-bumps an c4
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//  03/28/2025          Remove 2D grid data structure
//  03/29/2025          Remove Signal Type ID counting mechanics, use SignalType
//
/////////////////////////////////////////////////////////////////////////////////

#ifndef __PINMAP_H__
#define __PINMAP_H__

// Dependencies
// 1. C++ STL:
#include <string>
#include <unordered_map>
#include <unordered_set>

// 2. Boost Library:

// 3. Texo Library:
#include "cord.hpp"
#include "signalType.hpp"

typedef std::string pinType;
typedef unsigned char pinTypeId;

class PinMap{
protected:
    std::string m_name;
    int m_pinMapWidth;
    int m_pinMapHeight;

public:

    PinMap();
    explicit PinMap(const std::string name, int width, int height);

    inline std::string getName() const {return this->m_name;}
    inline int getPinMapWidth() const {return this->m_pinMapWidth;}
    inline int getPinMapHeight() const {return this->m_pinMapHeight;}

    std::unordered_map<Cord, SignalType> cordToSignalTypeMap;
    std::unordered_map<SignalType, std::unordered_set<Cord>> signalTypeToAllCords;

};

#endif // __PINMAP_H__
