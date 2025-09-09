//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        09/07/2025 22:12:23
//  Module Name:        pdnNode.hpp
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

#ifndef __PDN_EDGE_H__
#define __PDN_EDGE_H__

// Dependencies
// 1. C++ STL:

// 2. Boost Library:

// 3. Texo Library:
#include "cord.hpp"
#include "signalType.hpp"

class PDNNode;

class PDNEdge{
public:

    PDNNode *n0;
    PDNNode *n1;

    SignalType signal;
    bool isVia;

    PDNEdge();
    PDNEdge(PDNNode *n0, PDNNode *n1);

};

#endif // __PDN_EDGE_H__