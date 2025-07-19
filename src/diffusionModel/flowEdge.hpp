//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        07/18/2025 13:17:41
//  Module Name:        flowEdge.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A data structure that holds two metalCell* and maps to
//                      a Gurobi decision variable
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
/////////////////////////////////////////////////////////////////////////////////

#ifndef __FLOW_EDGE_H__
#define __FLOW_EDGE_H__

// Dependencies
// 1. C++ STL:

// 2. Boost Library:

// 3. Texo Library:
#include "signalType.hpp"

// 4. Gurobi Library
#include "gurobi_c++.h"

class FlowNode;

class FlowEdge{
public:
    FlowNode *u;
    FlowNode *v;
    SignalType st;

    GRBVar var;
    
    FlowEdge(SignalType st, FlowNode* u, FlowNode* v);
    FlowEdge(SignalType st, FlowNode* u, FlowNode* v, GRBVar var);
};

std::ostream &operator << (std::ostream &os, const FlowEdge &fe);

namespace std {
    template <>
    struct hash<FlowEdge> {
        size_t operator()(const FlowEdge &key) const;
    };

}  // namespace std

#endif // __FLOW_EDGE_H__