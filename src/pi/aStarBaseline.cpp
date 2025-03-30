//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/23/2025 15:05:54
//  Module Name:        aStarBaseline.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        Implement A* BaseLine Algorithm in 
//                      Liao, H., Patil, V., Dong, X., Shanbhag, D., Fallon, E., Hogan, T.,
//                      Spasojevic, M., and Burak Kara, L. (July 25, 2023). 
//                      "Hierarchical Automatic Multilayer Power Plane Generation With Genetic 
//                      Optimization and Multilayer Perceptron." 
//                      ASME. J. Mech. Des. October 2023; 145(10): 101706.
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//
//////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:
#include <cassert>
#include <vector>
#include <iostream>
#include <set>

// 2. Boost Library:
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>

// 3. Texo Library:
#include "aStarBaseline.hpp"
#include "signalType.hpp"
#include "ballOut.hpp"
#include "microBump.hpp"
#include "c4Bump.hpp"

#include "cord.hpp"



AStarBaseline::AStarBaseline(const std::string &fileName): uBump(fileName), c4(fileName) {
    assert(uBump.getPinMapWidth() == c4.getPinMapWidth());
    assert(uBump.getPinMapHeight() == c4.getPinMapHeight());
    assert(uBump.getPinMapWidth() > 1);
    assert(uBump.getPinMapHeight() > 1);

    this->canvasWidth = uBump.getPinMapWidth() - 1;
    this->canvasHeight = uBump.getPinMapHeight() - 1;
    
    this->canvasM5.resize(this->canvasHeight, std::vector<SignalType>(this->canvasWidth, SignalType::EMPTY));
    this->canvasM7.resize(this->canvasHeight, std::vector<SignalType>(this->canvasWidth, SignalType::EMPTY));
}

// void AStarBaseline::calculateUBumpMST(){
//     std::vector<bumpType> allPinTypes = powerPlane.uBump.getAllPinTypes();
//     for(int i = 0; i < allPinTypes.size(); ++i){
//         bumpType bt = allPinTypes[i];
//         if((bt == "GROUND") || (bt == "SIG")) continue;

//         std::set<Cord> allCordsOfTypeSet;
//         powerPlane.uBump.getAllPinsofType(bt, allCordsOfTypeSet);
//         std::vector<Cord> pinsVector(allCordsOfTypeSet.begin(), allCordsOfTypeSet.end());

        
//         typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, boost::no_property, boost::property<boost::edge_weight_t, int>> Graph;
//         typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
    
//         Graph graph(pinsVector.size());
//         for(int i = 0; i < pinsVector.size() - 1; ++i){
//             for(int j = i+1; j < pinsVector.size(); ++j){
                
//                 add_edge(i, j, calManhattanDistance(pinsVector[i], pinsVector[j]), graph);
//             }
//         }
    
//         std::vector<Vertex> p(num_vertices(graph));
//         prim_minimum_spanning_tree(graph, &p[0]);

//         // powerPlane.m5.addBlockTile(Rectangle())
    
//         // for (std::size_t i = 0; i != p.size(); ++i){
//         //     if (p[i] != i)
//         //         std::cout << "parent[" << i << "] = " << p[i] << std::endl;
//         //     else
//         //         std::cout << "parent[" << i << "] = no parent" << std::endl;
//         // }
        
        
//     }
// }

// Cord AStarBaseline::traslateIdxToCord(int idx) const {
//     const int pinCountWidth = powerPlane.uBump.getPinCountWidth();
//     len_t x = idx / pinCountWidth;
//     return Cord(x, idx - x*pinCountWidth);
// }

// int AStarBaseline::translateCordToIdx(Cord cord) const {
//     const int pinCountWidth = powerPlane.uBump.getPinCountWidth();
//     return cord.x() * pinCountWidth + cord.y();
// }