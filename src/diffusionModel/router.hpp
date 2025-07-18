//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        07/017/2025 18:50:02
//  Module Name:        router.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        For testing gurobi and run flow based routing algorithm
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
// 07/08/2025           Ported from diffusionSimulator class
/////////////////////////////////////////////////////////////////////////////////

#ifndef _ROUTER_H__
#define _ROUTER_H__

// Dependencies
// 1. C++ STL:

#include <vector>

// 2. Boost Library:

// 3. Texo Library:
#include "signalType.hpp"
#include "powerDistributionNetwork.hpp"
#include "dirFlags.hpp"
#include "diffusionChamber.hpp"
#include "metalCell.hpp"
#include "viaCell.hpp"
#include "units.hpp"

// 4. Gurobi
#include "gurobi_c++.h"

struct GNode {
    bool isMetal;
    int z, y, x;
    GNode(bool metal, int z, int y, int x): isMetal(metal), z(z), y(y), x(x) {}

    bool operator<(const GNode& other) const {
        return std::tie(z, y, x) < std::tie(other.z, other.y, other.x);
    }
};

struct GEdge {
    GNode *u;
    GNode *v;
    GEdge(GNode *u, GNode *v): u(u), v(v) {}
    bool operator<(const GEdge& other) const {
        return std::tie(u, v) < std::tie(other.u, other.v);
    }
};

const int LAYERS = 3;
const int WIDTH = 4;
const int HEIGHT = 4;

const int NUM_SIGNALS = 2;

void testRoute();


#endif // _ROUTER_H__