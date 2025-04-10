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

#ifndef __ASTARBASELINE_H__
#define __ASTARBASELINE_H__

// Dependencies
// 1. C++ STL:
#include <vector>

// 2. Boost Library:


// 3. Texo Library:
#include "cord.hpp"
#include "powerGrid.hpp"
#include "orderedSegment.hpp"

struct AStarNode{
    Cord cord;
    int gCost, hCost;
    AStarNode *parent;

    AStarNode(const Cord &c, int g, int h, AStarNode *p = nullptr);

    inline int fCost() const {return gCost + hCost;}
    bool operator >(const AStarNode &other) const;
};

std::vector<Cord> reconstructPath(AStarNode *end);
std::vector<Cord> runAStarAlgorithm(const std::vector<std::vector<SignalType>> &canvas, const Cord &start, const Cord &goal, const SignalType &st);

class AStarBaseline: public PowerGrid{
private:
    std::vector<Cord> reconnectAStarHelperBFSLabel(const std::vector<std::vector<SignalType>> &canvas, std::vector<std::vector<int>> &component, int j, int i, int id);
    std::vector<Cord> shortestPathBetweenSets(const std::vector<std::vector<bool>> &grid, const std::vector<Cord> &setA, const std::vector<Cord> &setB);
public:

    AStarBaseline(const std::string &fileName);

    void calculateMST(const PinMap &pm, std::vector<std::vector<SignalType>> &canvas, const std::unordered_set<SignalType> &ignoreSt);
    void reconnectIslands(std::vector<std::vector<SignalType>> &canvas, const std::unordered_set<SignalType> &ignoreSt);
    void runKNearestNeighbor(std::vector<std::vector<SignalType>> &canvas, const std::unordered_set<SignalType> &ignoreSt);

    
    // Cord traslateIdxToCord(int idx) const;
    // int translateCordToIdx(Cord cord) const;

};



#endif // __ASTARBASELINE_H__