//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        09/07/2025 22:26:57
//  Module Name:        pdnEdge.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        Represents a physical edge of the pdn realisation structure
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//
//////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:

// 2. Boost Library:

// 3. Texo Library:
#include "signalType.hpp"
#include "pdnEdge.hpp"

PDNEdge::PDNEdge(): n0(nullptr), n1(nullptr), signal(SignalType::EMPTY), isVia(false) {}

PDNEdge::PDNEdge(PDNNode *n0, PDNNode *n1): n0(n0), n1(n1), signal(SignalType::EMPTY), isVia(false) {}

