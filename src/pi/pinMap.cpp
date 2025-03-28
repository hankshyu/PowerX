//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/27/2025 09:38:16
//  Module Name:        pinMap.cpp
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

// Dependencies
// 1. C++ STL:
#include <string>
#include <unordered_map>
#include <unordered_set>

// 2. Boost Library:

// 3. Texo Library:
#include "cord.hpp"
#include "pinMap.hpp"

PinMap::PinMap(): m_name(""), m_pinMapWidth(0), m_pinMapHeight(0) {

}

PinMap::PinMap(const std::string name, int width, int height)
    :m_name(name), m_pinMapWidth(width), m_pinMapHeight(height) {

}

