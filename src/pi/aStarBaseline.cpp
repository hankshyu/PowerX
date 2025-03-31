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
#include <queue>
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

AStarNode::AStarNode(const Cord &c, int g, int h, AStarNode *p): cord(c), gCost(g), hCost(h), parent(p) {

}

bool AStarNode::operator >(const AStarNode &other) const{
    return this->fCost() > other.fCost();
}

bool isValid(const Cord &c, const std::vector<std::vector<SignalType>>& grid){
    bool inGrid = (c.x() >= 0) && (c.y() >= 0) && (c.x() < grid[0].size()) && (c.y() < grid.size());
    return inGrid;
    // bool gridValid = (grid[c.y()][c.x()] == 0);
}

std::vector<Cord> reconstructPath(AStarNode *end){
    std::vector<Cord> path;
    while(end != nullptr){
        path.emplace_back(end->cord);
        end = end->parent;
    }
    std::reverse(path.begin(), path.end());
    return path;
}

std::vector<Cord> runAStarAlgorithm(const std::vector<std::vector<SignalType>>& grid, const Cord &start, const Cord &goal) {
    
    std::vector<std::vector<bool>> visited(grid.size(), std::vector<bool>(grid[0].size(), false));

    // use lambda
    auto cmp = [](AStarNode* a, AStarNode* b) { return *a > *b; };
    std::priority_queue<AStarNode*, std::vector<AStarNode*>, decltype(cmp)> openList(cmp);

    AStarNode* startNode = new AStarNode(start, 0, calManhattanDistance(start, goal));
    openList.push(startNode);

    const std::vector<Cord> directions = {Cord(0,1), Cord(1,0), Cord(0,-1), Cord(-1,0)};

    while (!openList.empty()) {
        AStarNode* current = openList.top();
        openList.pop();

        // int x = current->x, y = current->y;
        len_t x = current->cord.x();
        len_t y = current->cord.y();

        if (visited[y][x]) continue;
        visited[y][x] = true;
        if(current->cord == goal){
            std::vector<Cord> path = reconstructPath(current);

            while (!openList.empty()) {
                delete openList.top();
                openList.pop();
            }
            return path;
        }

        for (const Cord &dc : directions) {
            int nx = x + dc.x();
            int ny = y + dc.y();
            Cord ncord = Cord(nx, ny);
            if (isValid(ncord, grid) && !visited[ny][nx]) {
                int g = current->gCost + 1;
                int h = calManhattanDistance(ncord, goal);
                AStarNode* neighbor = new AStarNode(ncord, g, h, current);
                openList.push(neighbor);
            }
        }
    }

    return {}; // No path found
}


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

void AStarBaseline::pinPadInsertion(){
    for(std::unordered_map<Cord, SignalType>::const_iterator cit = uBump.cordToSignalTypeMap.begin(); cit != uBump.cordToSignalTypeMap.end(); ++cit){
        Cord c = cit->first;
        SignalType st = cit->second;
        if((st == SignalType::GROUND) || (st == SignalType::SIGNAL)) continue;

        if(c.x() != 0){
            if(c.y() != uBump.getPinMapHeight()){
                this->canvasM5[c.y()][c.x()] = st;
            }

            if(c.y() != 0){
                this->canvasM5[c.y() - 1][c.x()] = st;
            }

        }
        if(c.x() != this->uBump.getPinMapWidth()){
            if(c.y() != uBump.getPinMapHeight()){
                this->canvasM5[c.y()][c.x() - 1] = st;
            }

            if(c.y() != 0){
                this->canvasM5[c.y() - 1][c.x() - 1] = st;

            }
        }
    }
}

void AStarBaseline::calculateUBumpMST(){
    
    std::vector<Cord> hasOverlapCord;

    std::unordered_map<SignalType, std::unordered_set<Cord>>::const_iterator cit;
    // ;signalTypeToAllCords
    for(cit = this->uBump.signalTypeToAllCords.begin(); cit != this->uBump.signalTypeToAllCords.end(); ++cit){
        SignalType st = cit->first;
        if((st == SignalType::GROUND) || (st == SignalType::SIGNAL)) continue;
        std::vector<Cord> pinsVector(cit->second.begin(), cit->second.end());
        
        typedef boost::adjacency_list<
            boost::vecS, 
            boost::vecS, 
            boost::undirectedS, 
            boost::no_property, 
            boost::property<boost::edge_weight_t, int>> Graph;
        typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
        typedef boost::graph_traits<Graph>::edge_descriptor Edge;
    
        Graph graph(pinsVector.size());
        for(int i = 0; i < pinsVector.size() - 1; ++i){
            for(int j = i+1; j < pinsVector.size(); ++j){
                add_edge(i, j, calManhattanDistance(pinsVector[i], pinsVector[j]), graph);
            }
        }
    
        std::vector<Vertex> p(num_vertices(graph));
        prim_minimum_spanning_tree(graph, &p[0]);
        
    
        for (std::size_t i = 0; i != p.size(); ++i){
            if (p[i] != i){
                Cord from = pinsVector[i];
                Cord to = pinsVector[p[i]];
                assert(from != to);

                Cord RoutingStart;
                Cord RoutingEnd;

                if(from.x() > to.x()) std::swap(from, to);
                
                if(from.x() == to.x()){
                    if(from.y() > to.y()) std::swap(from, to);
                    RoutingStart.y(from.y());
                    RoutingEnd.y(to.y() - 1);
                    
                    if(from.x() != 0){
                        RoutingStart.x(from.x() - 1);
                        RoutingEnd.x(to.x() - 1);
                    }else{
                        RoutingStart.x(from.x());
                        RoutingEnd.x(to.x());
                    }
                }else{ // from.x() < to.x()
                    RoutingStart.x(from.x());
                    RoutingEnd.x(to.x() - 1);

                    if(from.y() == to.y()){
                        if(from.y() != 0){
                            RoutingStart.y(from.y() - 1);
                            RoutingEnd.y(to.y() - 1);
                        }else{
                            RoutingStart.y(from.y());
                            RoutingEnd.y(to.y());
                        }

                    }else if(from.y() > to.y()){
                        RoutingStart.y(from.y() - 1);
                        RoutingEnd.y(to.y());

                    }else{ // from.y() < to.y()
                        RoutingStart.y(from.y());
                        RoutingEnd.y(to.y() - 1);

                    }
                }

                std::vector<Cord> path = runAStarAlgorithm(this->canvasM5, RoutingStart, RoutingEnd);
                this->connectionToPath[OrderedSegment(from, to)] = path;
                for(const Cord &c : path){
                    if((this->canvasM5[c.y()][c.x()] != SignalType::EMPTY) && (this->canvasM5[c.y()][c.x()] != st)){
                        this->canvasM5[c.y()][c.x()] = SignalType::OVERLAP;
                        hasOverlapCord.push_back(c);
                    }else{
                        this->canvasM5[c.y()][c.x()] = st;
                    }
                }
            }
        }   
    }
    
    // process overlap

}

// Cord AStarBaseline::traslateIdxToCord(int idx) const {
//     const int pinCountWidth = powerPlane.uBump.getPinCountWidth();
//     len_t x = idx / pinCountWidth;
//     return Cord(x, idx - x*pinCountWidth);
// }

// int AStarBaseline::translateCordToIdx(Cord cord) const {
//     const int pinCountWidth = powerPlane.uBump.getPinCountWidth();
//     return cord.x() * pinCountWidth + cord.y();
// }