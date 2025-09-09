//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        09/07/2025 22:01:55
//  Module Name:        powerDistributionNetwork.cpp
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

// Dependencies
// 1. C++ STL:

// 2. Boost Library:

// 3. Texo Library:
#include "cord.hpp"
#include "signalType.hpp"
#include "pdnNode.hpp"

PDNNode::PDNNode(): layer(LEN_T_MAX), y(LEN_T_MAX), x(LEN_T_MAX), signal(SignalType::EMPTY),
    north(nullptr), south(nullptr), east(nullptr), west(nullptr), up(nullptr), down(nullptr) {}

PDNNode::PDNNode(len_t layer, len_t x, len_t y): layer(layer), y(y), x(x), signal(SignalType::EMPTY),
    north(nullptr), south(nullptr), east(nullptr), west(nullptr), up(nullptr), down(nullptr) {}

Cord PDNNode::getLayerCord() const {
    return Cord(x, y);
}

std::string to_string(PDNNode *node){
    return "n" + std::to_string(node->x) + "_" + std::to_string(node->y) + "_" + std::to_string(node->layer);
}

