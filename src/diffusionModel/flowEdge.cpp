//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        07/18/2025 13:28:43
//  Module Name:        flowEdge.cpp
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

// Dependencies
// 1. C++ STL:

// 2. Boost Library:
#include "boost/functional/hash.hpp"

// 3. Texo Library:
#include "flowEdge.hpp"
#include "flowNode.hpp"

// 4. Gurobi Library
#include "gurobi_c++.h"

FlowEdge::FlowEdge(SignalType st, FlowNode* u, FlowNode* v):
    st(st), u(u), v(v) {}

FlowEdge::FlowEdge(SignalType st, FlowNode* u, FlowNode* v, GRBVar var): 
    st(st), u(u), v(v), var(var) {}

std::ostream &operator << (std::ostream &os, const FlowEdge &fe){
    return os << "FlowEdge[" << fe.u->label << " -- " << fe.st << " -- " << fe.v->label << "]";
}

size_t std::hash<FlowEdge>::operator()(const FlowEdge &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.st);
    boost::hash_combine(seed, key.u);
    boost::hash_combine(seed, key.v);
    return seed;
}