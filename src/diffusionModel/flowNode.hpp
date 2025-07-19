//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        07/18/2025 13:48:19
//  Module Name:        flowNode.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A data structure that represents a MCF node
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
/////////////////////////////////////////////////////////////////////////////////

#ifndef __FLOW_NODE_H__
#define __FLOW_NODE_H__

// Dependencies
// 1. C++ STL:

// 2. Boost Library:

// 3. Texo Library:
#include "signalType.hpp"
#include "diffusionChamber.hpp"

// 4. Gurobi Library
#include "gurobi_c++.h"

enum class FlowNodeType : uint8_t{
    UNKNOWN,
    OBSTACLES,
    EMPTY,
    AGGREGATED,
    VIA_TOP,
    VIA_DOWN,
    SUPER_SOURCE,
    SUPER_SINK,
};

std::ostream& operator<<(std::ostream& os, FlowNodeType dct);



class FlowEdge;

class FlowNode{
public:
    FlowNodeType type;
    CellLabel label;
    int layer;

    SignalType st;
    bool isSuperNode;
    
    std::vector<FlowEdge *> inEdges;
    std::vector<FlowEdge *> outEdges;

    FlowNode();
    FlowNode(FlowNodeType type);
    FlowNode(FlowNodeType type, CellLabel label);
    FlowNode(FlowNodeType type, CellLabel label, int layer);
    
};


#endif // __FLOW_NODE_H__