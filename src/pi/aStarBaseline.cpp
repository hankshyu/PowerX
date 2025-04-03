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
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <utility>

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
    
    std::unordered_map<OrderedSegment, std::vector<Cord>> connectionToPath;
    std::unordered_map<OrderedSegment, SignalType> connectionToSignalType;

    std::unordered_map<Cord, std::unordered_set<OrderedSegment>> sameSignalOverlap;
    std::unordered_map<Cord, std::unordered_set<OrderedSegment>> diffSignalOverlap;

    

    std::unordered_map<SignalType, std::unordered_set<Cord>>::const_iterator cit;
    // signalTypeToAllCords
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
        
        // route the pin <-> pin result reflected by MST algorithm
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
                OrderedSegment connnName(from, to);

                // Overlap Rule:
                // 1. Use SignalType::OVERLAP to record same signal overlap
                // 2. Use SignalType::UNKNOWN to record different signal overlaps

                for(const Cord &c : path){
                    SignalType gridSigType = this->canvasM5[c.y()][c.x()];
                    if(gridSigType == SignalType::EMPTY){
                        this->canvasM5[c.y()][c.x()] = st;
                    }else if(gridSigType == st){
                        this->canvasM5[c.y()][c.x()] = SignalType::OVERLAP;
                        for(std::unordered_map<OrderedSegment, std::vector<Cord>>::const_iterator cit = connectionToPath.begin(); cit != connectionToPath.end(); ++cit){
                            bool foundSoleConn = false;
                            for(const Cord &oldCord : cit->second){
                                if(oldCord == c){
                                    sameSignalOverlap[c] ={cit->first, connnName};
                                    foundSoleConn = true;
                                    break;
                                }
                                if(foundSoleConn == true) break;
                            }
                        }

                    }else if(gridSigType == SignalType::OVERLAP){
                        // check if it's the same type of Signal
                        SignalType othersType = connectionToSignalType[*(sameSignalOverlap[c].begin())];
                        if(othersType== st){ // still remains OVERLAP
                            sameSignalOverlap[c].insert(connnName);
                        }else{ // Turns into Unknown
                            this->canvasM5[c.y()][c.x()] = SignalType::UNKNOWN;
                            diffSignalOverlap[c] = sameSignalOverlap[c];
                            diffSignalOverlap[c].insert(connnName);
                            sameSignalOverlap.erase(c);
                        }
                    }else if(gridSigType == SignalType::UNKNOWN){
                        diffSignalOverlap[c].insert(connnName);
                    }else{ // this is where two signals are not the same, 
                        this->canvasM5[c.y()][c.x()] = SignalType::UNKNOWN;
                        for(std::unordered_map<OrderedSegment, std::vector<Cord>>::const_iterator cit = connectionToPath.begin(); cit != connectionToPath.end(); ++cit){
                            bool foundSoleConn = false;
                            for(const Cord &oldCord : cit->second){
                                if(oldCord == c){
                                    diffSignalOverlap[c] ={cit->first, connnName};
                                    foundSoleConn = true;
                                    break;
                                }
                                if(foundSoleConn == true) break;
                            }
                        }
                    }
                }
                // insert into connection
                connectionToSignalType[connnName] = st;
                connectionToPath[connnName] = path;

            }
        }   
    }
    
    // process overlap, for diffSignalTypes overlap
    std::unordered_set<OrderedSegment> toTornconn;

    for(std::unordered_map<Cord, std::unordered_set<OrderedSegment>>::const_iterator cit = diffSignalOverlap.begin(); cit != diffSignalOverlap.end(); ++cit){
        toTornconn.insert(cit->second.begin(), cit->second.end());
    }

    for(const OrderedSegment &os : toTornconn){
        connectionToPath.erase(os);
    }

    // clean the canvas, redraw lines
    this->canvasM5.assign(this->canvasHeight, std::vector<SignalType>(this->canvasWidth, SignalType::EMPTY));
    for(std::unordered_map<OrderedSegment, std::vector<Cord>>::const_iterator cit = connectionToPath.begin(); cit != connectionToPath.end(); ++cit){
        SignalType st = connectionToSignalType[cit->first];
        for(const Cord &c : cit->second){
            this->canvasM5[c.y()][c.x()] = st;
        }
    }
}

void AStarBaseline::reconnectAStar(){
    std::vector<std::vector<int>> component;

    std::unordered_set<SignalType>allsigtpes;
    for(int j = 0; j < this->canvasHeight; ++j){
        for(int i = 0; i < this->canvasWidth; ++i){
            allsigtpes.insert(this->canvasM5[j][i]);
        }
    }
    allsigtpes.erase(SignalType::EMPTY);
    std::unordered_map<int, std::vector<Cord>> compIDToGrids;
    std::unordered_map<SignalType, std::vector<int>> sigTypeToComponents;
    for(const SignalType &cst : allsigtpes){
        sigTypeToComponents[cst] = {};
    }
    
    component.resize(this->canvasHeight, std::vector<int>(this->canvasWidth, -1));
    int componentID = 0;
    for(int j = 0; j < this->canvasHeight; ++j){
        for(int i = 0; i < this->canvasWidth; ++i){
            if(component[j][i] == -1){
                SignalType st = this->canvasM5[j][i];
                if(st != SignalType::EMPTY){
                    compIDToGrids[componentID] = {};
                    sigTypeToComponents[st].push_back(componentID);
                }
                compIDToGrids[componentID] = reconnectAStarHelperBFSLabel(component, j, i, componentID);
                componentID++;
            } 
        }
    }



    // remove if the signalType has only one island left
    for(std::unordered_map<SignalType, std::vector<int>>::iterator it = sigTypeToComponents.begin(); it != sigTypeToComponents.end();){
        if(it->second.size() == 1){
            it = sigTypeToComponents.erase(it);
        }else{
            ++it;
        }
    }


    // connect the signal islands if they are disconnected
    while(!sigTypeToComponents.empty()){
        std::unordered_map<SignalType, std::vector<int>>::iterator tgit = sigTypeToComponents.begin();
        SignalType tgSig = tgit->first;
        int rgSetIdx1 = tgit->second.at(0);
        int rgSetIdx2 = tgit->second.at(1);

        std::vector<std::vector<bool>> grid(this->canvasHeight, std::vector<bool>(this->canvasWidth, false));
        for(int j = 0; j < this->canvasHeight; ++j){
            for(int i = 0; i < this->canvasWidth; ++i){
                if((this->canvasM5[j][i] == SignalType::EMPTY) || (this->canvasM5[j][i] == tgSig)){
                    grid[j][i] = true;
                }
            }
        }
        
        std::vector<Cord> routingResult = shortestPathBetweenSets(grid, compIDToGrids[rgSetIdx1], compIDToGrids[rgSetIdx2]);

        assert(!routingResult.empty());
        for(const Cord &c : routingResult){
            this->canvasM5[c.y()][c.x()] = tgSig;
        }
        for(const Cord &c : compIDToGrids[rgSetIdx2]){
            compIDToGrids[rgSetIdx1].push_back(c);
        }

        compIDToGrids.erase(rgSetIdx2);
        
        sigTypeToComponents[tgSig].erase(sigTypeToComponents[tgSig].begin() + 1); // erase the second at rgSetIdx2

        // check if the signal has only one island left
        for(std::unordered_map<SignalType, std::vector<int>>::iterator it = sigTypeToComponents.begin(); it != sigTypeToComponents.end();){
            if(it->second.size() == 1){
                it = sigTypeToComponents.erase(it);
            }else{
                ++it;
            }

        }



    }
}

std::vector<Cord> AStarBaseline::reconnectAStarHelperBFSLabel(std::vector<std::vector<int>> &component, int j, int i, int id){
    const int dx[4] = {-1, 1, 0, 0};
    const int dy[4] = {0, 0, -1, 1};

    std::vector<Cord> cordsofThisID;
    
    SignalType st = this->canvasM5[j][i];
    std::queue<std::pair<int, int>> q;
    q.push(std::pair<int, int>(j, i));
    component[j][i] = id;
    cordsofThisID.push_back(Cord(i, j));

    while(!q.empty()){
        int x = q.front().first;
        int y = q.front().second;
        q.pop();

        for(int dir = 0; dir < 4; ++dir){
            int nx = x + dx[dir]; 
            int ny = y + dy[dir];
            if(nx >= 0 && nx < this->canvasHeight && ny >= 0 && ny < this->canvasWidth &&
                component[nx][ny] == -1 && this->canvasM5[nx][ny] == st){
                
                q.push(std::pair<int, int>(nx, ny));
                component[nx][ny] = id;
                cordsofThisID.push_back(Cord(ny, nx));
            }
        }
    }

    return cordsofThisID;
}


std::vector<Cord> AStarBaseline::shortestPathBetweenSets(const std::vector<std::vector<bool>> &grid, const std::vector<Cord> &setA, const std::vector<Cord> &setB){
    std::vector<std::vector<bool>> visisted(this->canvasHeight, std::vector<bool>(this->canvasWidth, false));
    std::queue<Cord> q;
    std::set<Cord> setBSet(setB.begin(), setB.end());
    std::unordered_map<Cord, Cord> parent; // map a cord to its parent

    for(const Cord &c : setA){
        q.push(c);
        visisted[c.y()][c.x()] = true;
        parent[c] = Cord(-1, -1); // Mark as root
    }

    std::vector<Cord> directions = {Cord(-1, 0), Cord(1, 0), Cord(0, -1), Cord(0, 1)};
    Cord goal(-1, -1);
    while(!q.empty()){
        Cord mc(q.front());
        q.pop();
        int x = mc.x();
        int y = mc.y();

        if(setBSet.count(mc)){
            goal = mc;
            break;
        }
        
        for(const Cord &c : directions){
            int nx = x + c.x();
            int ny = y + c.y();

            if(nx >= 0 && nx < canvasWidth && ny >= 0 && ny < canvasHeight && !visisted[ny][nx] && grid[ny][nx]){
                visisted[ny][nx] = true;
                parent[Cord(nx, ny)] = mc;
                q.push(Cord(nx, ny));
            }
        }
    }
    

    // if no path found
    if(goal == Cord(-1, -1)) return {};

    // reconstrut path
    std::vector<Cord> returnPath;
    for(Cord cur(goal); cur != Cord(-1, -1); cur = parent[cur]){
        returnPath.push_back(cur);
    }

    reverse(returnPath.begin(), returnPath.end());
    return returnPath;
    
}


void AStarBaseline::runKNN(){
    std::vector<std::vector<SignalType>> refMap = this->canvasM5;
    std::unordered_set<SignalType> allSignalTypes;
    for(int j = 0; j < this->canvasHeight; ++j){
        for(int i = 0; i < this->canvasWidth; ++i){
            allSignalTypes.insert(this->canvasM5[j][i]);
        }
    }
    allSignalTypes.erase(SignalType::EMPTY);
    for(int j = 0; j < this->canvasHeight; ++j){
        for(int i = 0; i < this->canvasWidth; ++i){
            std::cout << "KNN " << j << " " << i << std::endl;
            if(refMap[j][i] != SignalType::EMPTY) continue;
            // run knn
            for(int dist = 1; dist <= (this->canvasWidth + this->canvasHeight - 2); ++dist){
                std::unordered_map<SignalType, int> vote;
                std::vector<Cord> searchGridsDeltas;
                for(const SignalType &st : allSignalTypes){
                    vote[st] = 0;
                }
                for(int a = -dist; a <= dist; ++a){
                    int abs_b = dist - std::abs(a);
                    int b1 = abs_b;

                    if(abs_b == 0){
                        searchGridsDeltas.push_back(Cord(a, 0));
                    }else{
                        searchGridsDeltas.push_back(Cord(a, b1));
                        searchGridsDeltas.push_back(Cord(a, -b1));
                    }
                }
                for(const Cord &c : searchGridsDeltas){
                    int nx = c.x() + i;
                    int ny = c.y() + j;
                    if(!(nx >= 0 && nx < this->canvasWidth && ny >= 0 && ny < this->canvasHeight)) continue;

                    SignalType voteSt = refMap[ny][nx];
                    if(voteSt!= SignalType::EMPTY){
                        vote[voteSt]++;
                    }
                }
                int maxCount = 0;
                SignalType maxCountSigtype = SignalType::EMPTY;
                for(std::unordered_map<SignalType, int>::const_iterator cit = vote.begin(); cit != vote.end(); ++cit){
                    if(cit->second >= maxCount){
                        maxCount = cit->second;
                        maxCountSigtype = cit->first;
                    }
                } 
                if(maxCount != 0){
                    this->canvasM5[j][i] = maxCountSigtype;
                    break;
                } 

            }

        }
    }

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