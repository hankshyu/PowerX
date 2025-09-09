//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        09/07/2025 22:01:55
//  Module Name:        pdnNode.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        Represents a physical node of the pdn realisation structure
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//
//////////////////////////////////////////////////////////////////////////////////

#ifndef __PDN_NODE_H__
#define __PDN_NODE_H__

// Dependencies
// 1. C++ STL:

// 2. Boost Library:

// 3. Texo Library:
#include "cord.hpp"
#include "signalType.hpp"

class PDNEdge;

class PDNNode{
public:

    len_t layer;
    len_t y;
    len_t x;

    SignalType signal;

    PDNEdge *north;
    PDNEdge *south;
    PDNEdge *east;
    PDNEdge *west;

    PDNEdge *up;
    PDNEdge *down;

    PDNNode();
    PDNNode(len_t layer, len_t x, len_t y);

    Cord getLayerCord() const;
    

};

std::string to_string(PDNNode *node);

#endif // __PDN_NODE_H__