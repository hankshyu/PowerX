//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        07/18/2025 14:05:25
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

// Dependencies
// 1. C++ STL:

// 2. Boost Library:

// 3. Texo Library:
#include "flowNode.hpp"

// 4. Gurobi Library
#include "gurobi_c++.h"

FlowNode::FlowNode(): FlowNode(FlowNodeType::UNKNOWN, CELL_LABEL_EMPTY -1) {}

FlowNode::FlowNode(FlowNodeType type): FlowNode(type, CELL_LABEL_EMPTY -1) {}

FlowNode::FlowNode(FlowNodeType type, CellLabel label): FlowNode(type, label, -1) {}

FlowNode::FlowNode(FlowNodeType type, CellLabel label, int layer): 
    type(type), label(label), layer(layer),
    st(SignalType::UNKNOWN),
    isSuperNode(false), northIsAggregated(false), southIsAggregated(false), eastIsAggregated(false), westIsAggregated(false) {}
