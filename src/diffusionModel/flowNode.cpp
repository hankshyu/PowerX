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
std::ostream& operator<<(std::ostream& os, FlowNodeType fnt){
    switch (fnt){
        case FlowNodeType::UNKNOWN:
            return os << "FlowNodeType::UNKNOWN";
            break;
        case FlowNodeType::OBSTACLES:
            return os << "FlowNodeType::OBSTACLES";
            break;
        case FlowNodeType::EMPTY:
            return os << "FlowNodeType::EMPTY";
            break;
        case FlowNodeType::AGGREGATED:
            return os << "FlowNodeType::AGGREGATED";
            break;
        case FlowNodeType::VIA_TOP:
            return os << "FlowNodeType::VIA_TOP";
            break;
        case FlowNodeType::VIA_DOWN:
            return os << "FlowNodeType::VIA_DOWN";
            break;
        case FlowNodeType::SUPER_SOURCE:
            return os << "FlowNodeType::SUPER_SOURCE";
            break;
        case FlowNodeType::SUPER_SINK:
            return os << "FlowNodeType::SUPER_SINK";
            break;
        default:
            return os << "CellType::?";
            break;
    }
}

FlowNode::FlowNode(): FlowNode(FlowNodeType::UNKNOWN, CELL_LABEL_EMPTY, -1, SignalType::UNKNOWN) {}

FlowNode::FlowNode(FlowNodeType type): FlowNode(type, CELL_LABEL_EMPTY, -1, SignalType::UNKNOWN) {}

FlowNode::FlowNode(FlowNodeType type, CellLabel label): FlowNode(type, label, -1, SignalType::UNKNOWN) {}

FlowNode::FlowNode(FlowNodeType type, CellLabel label, int layer): FlowNode(type, label, layer, SignalType::UNKNOWN) {}

FlowNode::FlowNode(FlowNodeType type, CellLabel label, int layer, SignalType st): 
    type(type), label(label), layer(layer),
    signal(st),
    isSuperNode(false), northIsAggregated(false), southIsAggregated(false), eastIsAggregated(false), westIsAggregated(false) {}
