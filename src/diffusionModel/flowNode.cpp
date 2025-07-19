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

FlowNode::FlowNode(): type(FlowNodeType::UNKNOWN), label(CELL_LABEL_EMPTY), layer(-1), isSuperNode(false) {}

FlowNode::FlowNode(FlowNodeType type): type(type), label(CELL_LABEL_EMPTY), layer(-1), isSuperNode(false) {}

FlowNode::FlowNode(FlowNodeType type, CellLabel label): type(type), label(label), layer(-1), isSuperNode(false) {}

FlowNode::FlowNode(FlowNodeType type, CellLabel label, int layer): type(type), label(label), layer(layer), isSuperNode(false) {}