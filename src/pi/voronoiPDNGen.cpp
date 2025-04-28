//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        04/06/2025 18:15:19
//  Module Name:        voronoiPDNGen.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        Implementation of 
//                      Chia-Wei Lin, Jing-Yao Weng, I-Te Lin, 
//                      Ho-Chieh Hsu, Chia-Ming Liu, and Mark Po-Hung Lin. 2024.
//                      "Voronoi Diagram-based Multiple Power Plane Generation on Redistribution Layers in 3D ICs." 
//                      In Proceedings of the 61st ACM/IEEE Design Automation Conference (DAC '24).
//                      Association for Computing Machinery, New York, NY, USA, Article 19, 1–6.
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//
//////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stack>
#include <memory>
#include <assert.h>
#include <omp.h> // parallel computing

// 2. Boost Library:
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include "boost/polygon/polygon.hpp"
#include "boost/geometry.hpp"

// 3. Texo Library:
#include "units.hpp"
#include "powerGrid.hpp"
#include "segment.hpp"
#include "orderedSegment.hpp"
#include "voronoiPDNGen.hpp"

// 4. FLUTE
#include "flute.h"

// 5.GEOS
#define USE_UNSTABLE_GEOS_CPP_API  // Suppress GEOS C++ warning
#include "geos/geom/Coordinate.h"
#include "geos/geom/Envelope.h"
#include "geos/geom/GeometryFactory.h"
#include "geos/geom/Point.h"
#include "geos/geom/Geometry.h"
#include "geos/geom/MultiPoint.h"
#include "geos/triangulate/VoronoiDiagramBuilder.h"

VoronoiPDNGen::VoronoiPDNGen(const std::string &fileName): PowerGrid(fileName) {

}
/*
void VoronoiPDNGen::runSATRouting(){
    CaDiCaL::Solver solver;

    // Clause: x1 ∨ x2
    solver.add(1);
    solver.add(2);
    solver.add(0);

    // Clause: ¬x1 ∨ x2
    solver.add(-1);
    solver.add(2);
    solver.add(0);

    // Clause: x1 ∨ ¬x2
    solver.add(1);
    solver.add(-2);
    solver.add(0);

    int result = solver.solve();

    if (result == 10) {
        std::cout << "SATISFIABLE\n";
        std::cout << "x1 = " << solver.val(1) << "\n";
        std::cout << "x2 = " << solver.val(2) << "\n";
    } else if (result == 20) {
        std::cout << "UNSATISFIABLE\n";
    } else {
        std::cout << "UNKNOWN\n";
    }
}

void VoronoiPDNGen::runILPRouting(){
    constexpr int ROWS = 3;
    constexpr int COLS = 4;
    constexpr int NETS = 2;

    int arr[ROWS][COLS] = {
        {1, 2, 0, 2},
        {0, 0, 1, 0},
        {1, 0, 2, 0}
    };

    // Lambda for checking if (i,j) is a pin of a given net
    auto is_pin = [&](int i, int j, int net) -> bool {
        return arr[i][j] == net + 1;
    };

    // Lambda for mapping 3D (i,j,net) into 1D index
    auto var_index = [&](int i, int j, int net) -> int {
        return net * ROWS * COLS + i * COLS + j;
    };

    int totalVars = ROWS * COLS * NETS;
    OsiClpSolverInterface solver;

    // Set up variables: x[i][j][net]
    std::vector<double> objective(totalVars, 1.0); // Minimize total wirelength
    std::vector<double> col_lb(totalVars, 0.0);
    std::vector<double> col_ub(totalVars, 1.0);

    CoinPackedMatrix* matrix = new CoinPackedMatrix(false, 0, 0);
    matrix->setDimensions(0, totalVars); // initially no constraints

    solver.loadProblem(*matrix, &col_lb[0], &col_ub[0], &objective[0], nullptr, nullptr);
    for (int i = 0; i < totalVars; ++i) {
        solver.setInteger(i);
    }

    // Add constraints
    std::vector<CoinPackedVector> constraints;
    std::vector<double> rhs;
    std::vector<char> sense;

    // 1. No cell can be shared by both nets
    for (int i = 0; i < ROWS; ++i)
        for (int j = 0; j < COLS; ++j) {
            CoinPackedVector row;
            row.insert(var_index(i, j, 0), 1.0); // net1
            row.insert(var_index(i, j, 1), 1.0); // net2
            constraints.push_back(row);
            rhs.push_back(1.0);
            sense.push_back('L'); // x1 + x2 <= 1
        }

    // 2. Pin constraints: each net must connect its pins
    for (int net = 0; net < NETS; ++net) {
        for (int i = 0; i < ROWS; ++i)
            for (int j = 0; j < COLS; ++j)
                if (is_pin(i, j, net)) {
                    CoinPackedVector row;
                    row.insert(var_index(i, j, net), 1.0);
                    constraints.push_back(row);
                    rhs.push_back(1.0);
                    sense.push_back('E'); // Must be used
                }
    }

    // Load constraints into solver
    for (int i = 0; i < constraints.size(); ++i) {
        solver.addRow(constraints[i], sense[i], rhs[i], rhs[i]);
    }

    // Solve using CBC
    CbcModel model(solver);
    model.solver()->setHintParam(OsiDoReducePrint, true, OsiHintTry);
    model.branchAndBound();

    const double* solution = model.solver()->getColSolution();

    // Print solution
    std::cout << "Routing solution:\n";
    for (int net = 0; net < NETS; ++net) {
        std::cout << "Net " << (net + 1) << ":\n";
        for (int i = 0; i < ROWS; ++i) {
            for (int j = 0; j < COLS; ++j) {
                int idx = var_index(i, j, net);
                std::cout << (solution[idx] > 0.5 ? "#" : ".");
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }
        
}
*/

void VoronoiPDNGen::initPoints(const std::unordered_set<SignalType> &m5IgnoreSigs, const std::unordered_set<SignalType> &m7IgnoreSigs){

    this->nodeHeight = this->canvasHeight + 1;
    this->nodeWidth = this->canvasWidth + 1;

    m5NodeArr.resize(nodeHeight, std::vector<VNode *>(nodeWidth, nullptr));
    m7NodeArr.resize(nodeHeight, std::vector<VNode *>(nodeWidth, nullptr));

    for(int j = 0; j < nodeHeight; ++j){
        for(int i = 0; i < nodeWidth; ++i){
            VNode *unode = new VNode;
            unode->sig = SignalType::EMPTY;
            unode->up = nullptr;
            unode->down = nullptr;
            unode->left = nullptr;
            unode->right = nullptr;
            this->m5NodeArr[j][i] = unode;

            VNode *dnode = new VNode;
            dnode->sig = SignalType::EMPTY;
            dnode->up = nullptr;
            dnode->down = nullptr;
            dnode->left = nullptr;
            dnode->right = nullptr;
            this->m7NodeArr[j][i] = dnode;
        }
    }
    // create node and edges
    for(int j = 0; j < nodeHeight; ++j){
        for(int i = 0; i < nodeWidth; ++i){

            if(i != (nodeWidth - 1)){
                VEdge *urEdge = new VEdge;
                urEdge->sig = SignalType::EMPTY;
                urEdge->c1 = Cord(i, j);
                urEdge->c2 = Cord(i+1, j);
                this->m5NodeArr[j][i]->right = urEdge;
                this->m5NodeArr[j][i+1]->left = urEdge;

                VEdge *drEdge = new VEdge;
                drEdge->sig = SignalType::EMPTY;
                drEdge->c1 = Cord(i, j);
                drEdge->c2 = Cord(i+1, j);
                this->m7NodeArr[j][i]->right = drEdge;
                this->m7NodeArr[j][i+1]->left = drEdge;
            }

            if(j != (nodeHeight -1)){
                VEdge *uEdge = new VEdge;
                uEdge->sig = SignalType::EMPTY;
                uEdge->c1 = Cord(i, j);
                uEdge->c2 = Cord(i, j+1);
                this->m5NodeArr[j][i]->up = uEdge;
                this->m5NodeArr[j+1][i]->down = uEdge;

                VEdge *duEdge = new VEdge;
                duEdge->sig = SignalType::EMPTY;
                duEdge->c1 = Cord(i, j);
                duEdge->c1 = Cord(i, j+1);
                this->m7NodeArr[j][i]->up = duEdge;
                this->m7NodeArr[j+1][i]->down = duEdge;
            }
        }
    }

    for(const SignalType &st : this->uBump.allSignalTypes){
        if(m5IgnoreSigs.count(st) != 0) continue;

        for(const Cord &c : this->uBump.signalTypeToAllCords[st]){
            SignalType fillSig = st;
            
            int leftBorder = (c.x() != 0)? (c.x() - 1) : 0;
            int rightBorder = (c.x() != (nodeWidth - 1))? (c.x() + 1) : c.x();
            int downBorder = (c.y() != 0)? (c.y() - 1) : 0;
            int upBorder = (c.y() != (nodeHeight - 1))? (c.y() + 1) : c.y();
            for(int j = downBorder; j <= upBorder; ++j){
                for(int i = leftBorder; i <= rightBorder; ++i){
                    m7NodeArr[j][i]->sig = fillSig;
                    if(i != rightBorder) m7NodeArr[j][i]->right->sig = fillSig;
                    if(j != upBorder) m7NodeArr[j][i]->up->sig = fillSig;
                }
            }
        }

        this->m5Points[st] = {};
        this->m5Segments[st] = {};
        if(this->uBump.signalTypeToAllCords[st].size() != 0){
            for(const Cord &c : this->uBump.signalTypeToAllCords[st]){
                this->m5Points[st].push_back(c);
            }
        }
    }
    
    for(const SignalType &st : this->c4.allSignalTypes){
        for(const Cord &c : this->c4.signalTypeToAllCords[st]){
            SignalType fillSig = (m7IgnoreSigs.count(st) == 0)? st : SignalType::OBSTACLE;
            
            int leftBorder = (c.x() != 0)? (c.x() - 1) : 0;
            int rightBorder = (c.x() != (nodeWidth - 1))? (c.x() + 1) : c.x();
            int downBorder = (c.y() != 0)? (c.y() - 1) : 0;
            int upBorder = (c.y() != (nodeHeight - 1))? (c.y() + 1) : c.y();
            for(int j = downBorder; j <= upBorder; ++j){
                for(int i = leftBorder; i <= rightBorder; ++i){
                    m7NodeArr[j][i]->sig = fillSig;
                    if(i != rightBorder) m7NodeArr[j][i]->right->sig = fillSig;
                    if(j != upBorder) m7NodeArr[j][i]->up->sig = fillSig;
                }
            }
        }

        if(m7IgnoreSigs.count(st) != 0) continue;

        this->m7Points[st] = {};
        this->m7Segments[st] = {};
        if(this->c4.signalTypeToAllCords[st].size() != 0){
            for(const Cord &c : this->c4.signalTypeToAllCords[st]){
                this->m7Points[st].push_back(c);
            }
        }
    }
}

void VoronoiPDNGen::connectLayers(){
    // for each signalType. These must at least be one point overlaap between m5 and m7
    for(std::unordered_map<SignalType, std::vector<Cord>>::const_iterator cit = this->m5Points.begin(); cit != m5Points.end(); ++cit){
        SignalType st = cit->first;
        std::vector<Segment> links;

        for(const Cord &up : cit->second){
            for(const Cord &dn : this->m7Points[st]){
                links.push_back(Segment(dn, up));
            }
        }

        std::sort(links.begin(), links.end(), [](Segment o1, Segment o2){
            return calEuclideanDistance(o1.low(), o1.high()) < calEuclideanDistance(o2.low(), o2.high());
        });

        if(links[0].low() == links[0].high()) continue;

        bool connectSuccess = false;
        for(int osidx = 0; osidx < links.size(); ++osidx){
            Segment pos = links[osidx];
            Cord up(pos.high());
            Cord down(pos.low());
            SignalType downSig = this->m7NodeArr[up.y()][up.x()]->sig;
            if((downSig == st) || (downSig == SignalType::EMPTY)){
                // attempt to add a point on m7
                connectSuccess = true;
                m7NodeArr[up.y()][up.x()]->sig = st;
                m7Points[st].push_back(up);
            }

            if(connectSuccess) break;
        }

        // if there is overlap point, skip the signal
    }
    
}

void VoronoiPDNGen::runFLUTERouting(std::unordered_map<SignalType, std::vector<Cord>> &layerPoints, std::unordered_map<SignalType, std::vector<OrderedSegment>> &layerSegments){
    const int FLUTE_ACC = 9; // 0 ~ 9
    for(std::unordered_map<SignalType, std::vector<Cord>>::iterator it = layerPoints.begin(); it != layerPoints.end(); ++it){
        SignalType targetSig = it->first;
        int pointCount = it->second.size();
        if(pointCount == 0) continue;
        std::vector<len_t> xArr;
        std::vector<len_t> yArr;
        for(const Cord &c : it->second){
            xArr.push_back(c.x());
            yArr.push_back(c.y());
        }

        Flute::FluteState *state = Flute::flute_init(WIRELENGTH_VECTOR_FILE, ROUTING_TREE_FILE);
        Flute::Tree tree = Flute::flute(state, pointCount, xArr.data(), yArr.data(), FLUTE_ACC);
        // std::cout << targetSig << "Before: " << layerPoints[targetSig].size() << std::endl;
        for(int i = 0; i < tree.deg*2 - 2; ++i){
            int n = tree.branch[i].n;
            Cord c1(tree.branch[i].x, tree.branch[i].y);
            Cord c2(tree.branch[n].x, tree.branch[n].y);
            if(c1 == c2) continue;
            bool foundc1 = false;
            bool foundc2 = false;
            for(int j = 0; j < layerPoints[targetSig].size(); ++j){
                Cord m5c = layerPoints[targetSig].at(j);
                if(m5c == c1) foundc1 = true;
            }
            for(int j = 0; j < layerPoints[targetSig].size(); ++j){
                Cord m5c = layerPoints[targetSig].at(j);
                if(m5c == c2) foundc2 = true;
            }
            if(!foundc1) layerPoints[targetSig].push_back(c1);
            if(!foundc2) layerPoints[targetSig].push_back(c2);

            OrderedSegment newos(c1, c2);
            bool foundos = false;
            for(OrderedSegment os : layerSegments[targetSig]){
                if(os == newos) foundos = true;
            }
            if(!foundos) layerSegments[targetSig].push_back(newos);

        }

        // std::cout << "after: " << layerPoints[targetSig].size() << std::endl;
        // std::cout << "segs: " << layerSegments[targetSig].size() << std::endl; 

        // std::cout << "Wirelength: " << tree.length << std::endl;
        // std::cout << "Branches:" << std::endl;
        // for (int i = 0; i < tree.deg * 2 - 2; ++i) {
        //     int n = tree.branch[i].n;
        //     std::cout << "Branch " << i << " at (" << tree.branch[i].x << ", " << tree.branch[i].y << ") "
        //             << "is connected to Branch " << n << " at ("
        //             << tree.branch[n].x << ", " << tree.branch[n].y << ")\n";
        // }

        // Free resources
        free_tree(state, tree);
        flute_free(state);
    }
}

void VoronoiPDNGen::runMSTRouting(std::unordered_map<SignalType, std::vector<Cord>> &layerPoints, std::unordered_map<SignalType, std::vector<OrderedSegment>> &layerSegments){
    for(std::unordered_map<SignalType, std::vector<Cord>>::iterator it = layerPoints.begin(); it != layerPoints.end(); ++it){
        SignalType targetSig = it->first;
        int pointCount = it->second.size();
        if(pointCount == 0) continue;
        
        std::vector <Cord> pinsVector(it->second.begin(), it->second.end());


        typedef boost::adjacency_list<
            boost::vecS, 
            boost::vecS, 
            boost::undirectedS, 
            boost::no_property, 
            boost::property<boost::edge_weight_t, int>> Graph;
        typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;

        Graph graph(pinsVector.size());
        for(int i = 0; i < pinsVector.size() - 1; ++i){
            for(int j = i+1; j < pinsVector.size(); ++j){
                add_edge(i, j, calManhattanDistance(pinsVector[i], pinsVector[j]), graph);
            }
        }

        std::vector<Vertex> p(num_vertices(graph));
        prim_minimum_spanning_tree(graph, &p[0]);

        for (std::size_t i = 0; i != p.size(); ++i){
            if (p[i] == i) continue;

            Cord c1 = pinsVector[i];
            Cord c2 = pinsVector[p[i]];
            assert(c1 != c2);

            if(c1 == c2) continue;
            bool foundc1 = false;
            bool foundc2 = false;
            for(int j = 0; j < layerPoints[targetSig].size(); ++j){
                Cord m5c = layerPoints[targetSig].at(j);
                if(m5c == c1) foundc1 = true;
            }
            for(int j = 0; j < layerPoints[targetSig].size(); ++j){
                Cord m5c = layerPoints[targetSig].at(j);
                if(m5c == c2) foundc2 = true;
            }
            if(!foundc1) layerPoints[targetSig].push_back(c1);
            if(!foundc2) layerPoints[targetSig].push_back(c2);

            OrderedSegment newos(c1, c2);
            bool foundos = false;
            for(OrderedSegment os : layerSegments[targetSig]){
                if(os == newos) foundos = true;
            }
            if(!foundos) layerSegments[targetSig].push_back(newos);
        }
    }
}

void VoronoiPDNGen::ripAndReroute(std::unordered_map<SignalType, std::vector<Cord>> &layerPoints, std::unordered_map<SignalType, std::vector<OrderedSegment>> &layerSegments){
    std::vector<std::pair<OrderedSegment, OrderedSegment>> rerouteSegmentArr;
    std::unordered_map<OrderedSegment, SignalType> allSegmentMap;

    // fill all Segments
    for(std::unordered_map<SignalType, std::vector<OrderedSegment>>::const_iterator cit = layerSegments.begin(); cit != layerSegments.end(); ++cit){
        for(const OrderedSegment &os : cit->second){
            allSegmentMap[os] = cit->first;
        }
    }

    // run segment intersect checking
    for(std::unordered_map<SignalType, std::vector<OrderedSegment>>::iterator lsit = layerSegments.begin(); lsit != layerSegments.end(); ++lsit){
        for(OrderedSegment os : lsit->second){
            for(std::unordered_map<OrderedSegment, SignalType>::iterator it = allSegmentMap.begin(); it != allSegmentMap.end(); ++it){
                if(lsit->first == it->second) continue;
                OrderedSegment cos = it->first;
                if (os == cos) continue;
                if(seg::intersects(os, cos) ){
                    rerouteSegmentArr.push_back(std::pair<OrderedSegment, OrderedSegment>(os, cos));
                }
            }
        }
    }

    

    std::vector<OrderedSegment> ripArr;
    // attempt to fix
    while(!rerouteSegmentArr.empty()){
        std::pair<OrderedSegment, OrderedSegment> fixPair = rerouteSegmentArr[0];
        // copare the segment length:
        flen_t firstLength = calEuclideanDistance(fixPair.first.getLow(), fixPair.first.getHigh());
        flen_t secondLength = calEuclideanDistance(fixPair.second.getLow(), fixPair.second.getHigh());
        OrderedSegment removeTarget;
        if(firstLength > secondLength){
            removeTarget = fixPair.first;
        }else{
            removeTarget = fixPair.second;
        }

        ripArr.push_back(removeTarget);
        rerouteSegmentArr.erase(remove_if(rerouteSegmentArr.begin(), rerouteSegmentArr.end(),
                [=](const std::pair<OrderedSegment, OrderedSegment>& p) {
                    return p.first == removeTarget || p.second == removeTarget;
                }
            ),rerouteSegmentArr.end()
        );
    
    }

    // remove OrderedSegment from the layerSegments
    for(OrderedSegment dos : ripArr){
        for(std::unordered_map<SignalType, std::vector<OrderedSegment>>::iterator it = layerSegments.begin(); it != layerSegments.end(); ++it){
            it->second.erase(std::remove(it->second.begin(), it->second.end(), dos), it->second.end());
        }
    }

    // sort the ripArr according to wirelength
    std::sort(ripArr.begin(), ripArr.end(), [](OrderedSegment os1, OrderedSegment os2){
        return calEuclideanDistance(os1.getLow(), os1.getHigh()) < calEuclideanDistance(os2.getLow(), os2.getHigh());
    });

    for(const OrderedSegment &cos : ripArr){

        SignalType cosSt = allSegmentMap[cos];
        Cord start(cos.getLow());
        Cord goal(cos.getHigh());
        std::cout << "Attempt BFS Routing From " << start << " -> " << goal << "of type: " << cosSt << std::endl;

        std::vector<std::vector<int>> nodeStat(this->nodeHeight, std::vector<int>(this->nodeWidth, 0));
        
        using P45Data = boost::polygon::polygon_45_data<len_t>;
        std::vector<P45Data> blockingObjects;
        std::vector<Segment> blockingSegs;


        for(std::unordered_map<SignalType, std::vector<OrderedSegment>>::const_iterator cit = layerSegments.begin(); cit != layerSegments.end(); ++cit){
            if(cit->first == cosSt) continue;
            for(const OrderedSegment &cbos : cit->second){
                Cord c1(cbos.getLow());
                Cord c2(cbos.getHigh());
                std::vector<Cord> windings;

                if(c1.x() == c2.x()){
                    if(c1.y() > c2.y()) std::swap(c1, c2);
                    windings = {Cord(c2.x()-1, c2.y()+1), Cord(c2.x()+1, c2.y()+1), Cord(c1.x()+1, c1.y()-1), Cord(c1.x()-1, c1.y()-1)};
                }else{
                    if(c1.x() > c2.x()) std::swap(c1, c2);
                    if(c1.y() > c2.y()){
                        windings = {
                            Cord(c1.x()-1, c1.y()-1), Cord(c1.x()-1, c1.y()+1), Cord(c1.x()+1, c1.y()+1),
                            Cord(c2.x()+1, c2.y()+1), Cord(c2.x()+1, c2.y()-1), Cord(c2.x()-1, c2.y()-1)
                        };
                    }else if(c1.y() < c2.y()){
                        windings = {
                            Cord(c1.x()+1, c1.y()-1), Cord(c1.x()-1, c1.y()-1), Cord(c1.x()-1, c1.y()+1),
                            Cord(c2.x()-1, c2.y()+1), Cord(c2.x()+1, c2.y()+1), Cord(c2.x()+1, c2.y()-1)
                        };
                    }else{ // c1.y() == c2.y()
                        windings = {Cord(c1.x()-1, c1.y()+1), Cord(c2.x()+1, c2.y()+1), Cord(c2.x()+1, c2.y()-1), Cord(c1.x()-1, c1.y()-1)};
                    }
                }

                blockingObjects.emplace_back(P45Data(windings.begin(), windings.end()));
                for(int i = 0; i < windings.size(); ++i){
                    blockingSegs.emplace_back(Segment(windings[i], windings[(i+1)%windings.size()]));
                    }
            }
        }

        for(int j = 0; j < this->nodeHeight; ++j){
            for(int i = 0; i < this->nodeWidth; ++i){
                Cord c(i, j);
                for(const P45Data &dt : blockingObjects){
                    if(boost::polygon::contains(dt, c)){
                        nodeStat[j][i] = 9;
                        break;
                    } 
                }
            }
        }

        // Start Routing uisng BFS
        // 9: blockages, 1 visited
        std::unordered_map<Cord, Cord> prev;
        std::unordered_map<Cord, int> cost;
        auto comp = [&](const Cord &a, const Cord &b){
            return (cost[a]+calEuclideanDistance(a, goal)) > (cost[b]+calEuclideanDistance(b, goal));
        };

        std::priority_queue<Cord, std::vector<Cord>, decltype(comp)> q(comp);

        cost[start] = 0;
        q.push(start);
        nodeStat[start.y()][start.x()] = 1;
        while(!q.empty()){
            Cord curr = q.top();
            q.pop();

            if(curr == goal) break;
            std::vector<Cord>neighbors;

            std::vector<Cord> candidates;
            #pragma omp parallel
            {
                std::vector<Cord> local_candidates;
                #pragma omp for nowait
                for(int j = 0; j < this->nodeHeight; ++j){
                    for(int i = 0; i < this->nodeWidth; ++i){
                        if(nodeStat[j][i] != 0) continue;
                        Cord cand(i, j);
                        Segment candSeg(curr, cand);
                        bool intersects = false;
                        for(const Segment &sg : blockingSegs){
                            if(seg::intersects(candSeg, sg)){
                                intersects = true;
                                break;
                            }
                        }
                        if(!intersects){
                            local_candidates.push_back(cand);
                        }
                    }
                }
            
                #pragma omp critical
                candidates.insert(candidates.end(), local_candidates.begin(), local_candidates.end());
            }
            // sequential section: update shared structures
            for (const Cord& cand : candidates) {
                int x = cand.x(), y = cand.y();
                if (nodeStat[y][x] == 0) {
                    nodeStat[y][x] = 1;
                    prev[cand] = curr;
                    cost[cand] = cost[curr] + calManhattanDistance(cand, curr);
                    q.push(cand);
                }
            }

            // std::cout << "Done " << curr << std::endl;
        }

        std::cout << "Finish Routing!" << std::endl;
        assert(nodeStat[goal.y()][goal.x()] != 0);
        std::vector<Cord> path;

        for(Cord at = goal; at != start; at = prev[at]){
            path.push_back(at);
        }
        path.push_back(start);
        std::reverse(path.begin(), path.end());
        std::cout << "Path: ";
        for(Cord &c : path){
            std::cout <<  c << " ";
        }
        std::cout << std::endl;
        // add points
        for(int i = 1; i < path.size()-1; ++i){
            layerPoints[cosSt].push_back(path[i]);
        }
        // add paths
        for(int i = 0; i < path.size()-1; ++i){
            layerSegments[cosSt].push_back(OrderedSegment(path[i], path[i+1]));
        }

    }
    fixRepeatedPoints(layerPoints);
}

void VoronoiPDNGen::generateInitialPowerPlane(std::unordered_map<SignalType, std::vector<Cord>> &layerPoints, std::unordered_map<SignalType, std::vector<OrderedSegment>> &layerSegments){
    std::stack<std::pair<OrderedSegment, SignalType>> toFix;
    for(std::unordered_map<SignalType, std::vector<OrderedSegment>>::iterator it = layerSegments.begin(); it != layerSegments.end(); ++it){
        SignalType st = it->first;
        for(std::vector<OrderedSegment>::iterator osit = it->second.begin(); osit!= it->second.end();){
            OrderedSegment os = *osit;
            Cord osLow(os.getLow());
            Cord osHigh(os.getHigh());
            FCord centre(double(osLow.x() + osHigh.x())/2, double(osLow.y() + osHigh.y())/2);
            double radius = calEuclideanDistance(osLow, osHigh) / 2;

            bool needsRemoval = false;
            for(std::unordered_map<SignalType, std::vector<Cord>>::const_iterator pcit = layerPoints.begin(); pcit != layerPoints.end(); ++pcit){
                if(pcit->first == st) continue;
                bool foundIntersect = false;
                for(std::vector<Cord>::const_iterator cordit = pcit->second.begin(); cordit != pcit->second.end(); ++cordit){
                    Cord eCord = *cordit;
                    if(calEuclideanDistance(centre, eCord) <= radius){
                        toFix.push(std::pair<OrderedSegment, SignalType>(os, st));
                        foundIntersect = true;
                        break;
                    }
                }
                needsRemoval = foundIntersect;
                if(needsRemoval) break;
            }

            osit = (needsRemoval)? it->second.erase(osit) : osit + 1;
        }
    }

    while(!toFix.empty()){
        std::pair<OrderedSegment, SignalType> tftop = toFix.top();
        toFix.pop();

        OrderedSegment fixos = tftop.first;
        SignalType fixst = tftop.second;
        Cord osLow(fixos.getLow());
        Cord osHigh(fixos.getHigh());
        FCord centre(double(osLow.x() + osHigh.x())/2, double(osLow.y() + osHigh.y())/2);
        double radius = calEuclideanDistance(osLow, osHigh) / 2;

        bool hasIntersect = false;
        Cord c3;
        
        for(std::unordered_map<SignalType, std::vector<Cord>>::const_iterator pcit = layerPoints.begin(); pcit != layerPoints.end(); ++pcit){
            if(pcit->first == fixst) continue;
            bool foundIntersect = false;
            for(std::vector<Cord>::const_iterator cordit = pcit->second.begin(); cordit != pcit->second.end(); ++cordit){
                Cord eCord = *cordit;
                if(calEuclideanDistance(centre, eCord) <= radius){
                    foundIntersect = hasIntersect = true;
                    c3 = eCord;
                    break;
                }
            }
            if(foundIntersect) break;
        }

        if(hasIntersect){
            // calculate the projection of c3 on line(osLow, osHigh) 
            if(osLow.x() > osHigh.x()) std::swap(osLow, osHigh);

            Cord ab(osHigh.x() - osLow.x(), osHigh.y() - osLow.y());
            Cord ac(c3.x() - osLow.x(), c3.y() - osLow.y());
            double t = double(ac.x()*ab.x() + ac.y()*ab.y()) / double(ab.x()*ab.x() + ab.y()*ab.y());
            Cord projection(osLow.x() + ab.x()*t, osLow.y() + ab.y()*t);
            if(projection == osLow){
                if(osLow.y() < osHigh.y()){
                    projection.x(projection.x()+1);
                    projection.y(projection.y()+1);
                }else if(osLow.y()== osHigh.y()){
                    projection.x(projection.x()+1);
                }else{ // osLow.y() > osHigh.y()
                    projection.x(projection.x()+1);
                    projection.y(projection.y()-1);
                }
            }else if(projection == osHigh){

                if(osLow.y() < osHigh.y()){
                    projection.x(projection.x()-1);
                    projection.y(projection.y()-1);
                }else if(osLow.y() == osHigh.y()){

                    projection.x(projection.x()-1);
                }else{ // osLow.y() > osHigh.y()

                    projection.x(projection.x()-1);
                    projection.y(projection.y()+1);
                }
            }
            layerPoints[fixst].push_back(projection);

            OrderedSegment nos1(osLow, projection);
            OrderedSegment nos2(projection, osHigh);
            toFix.push(std::pair<OrderedSegment, SignalType>(nos1, fixst));
            toFix.push(std::pair<OrderedSegment, SignalType>(nos2, fixst));
        }else{
            layerSegments[fixst].push_back(fixos);
        }
    }

    fixRepeatedPoints(layerPoints);
}

void VoronoiPDNGen::fixRepeatedPoints(std::unordered_map<SignalType, std::vector<Cord>> &layerPoints){
    std::unordered_map<Cord, SignalType> table;
    for(std::unordered_map<SignalType, std::vector<Cord>>::iterator it = layerPoints.begin(); it != layerPoints.end(); ++it){
        SignalType st = it->first;
        for(const Cord &cord : it->second){
            std::unordered_map<Cord, SignalType>::iterator fit = table.find(cord);
            if(fit != table.end()){
                assert(fit->second == st);
            }else{
                table[cord] = st;
            }
        }
    }
    for(std::unordered_map<SignalType, std::vector<Cord>>::iterator it = layerPoints.begin(); it != layerPoints.end(); ++it){
        it->second.clear();
    }
    for(std::unordered_map<Cord, SignalType>::const_iterator cit = table.begin(); cit != table.end(); ++cit){
        layerPoints[cit->second].push_back(cit->first);
    }
    
}

void VoronoiPDNGen::generateVoronoiDiagram(const std::unordered_map<SignalType, std::vector<Cord>> &layerPoints, std::unordered_map<Cord, std::vector<FCord>> &voronoiCells){
    
    std::vector<Cord> voronoiCentres;
    std::vector<SignalType> voronoiCentreSignalTypes;
    std::vector<std::vector<FCord>> allWindings;
    for(std::unordered_map<SignalType, std::vector<Cord>>::const_iterator cit = layerPoints.begin(); cit != layerPoints.end(); ++cit){
        for(const Cord &c : cit->second){
            voronoiCentres.push_back(c);
            voronoiCentreSignalTypes.push_back(cit->first);
        }
    }

    using namespace geos::geom;
    using geos::triangulate::VoronoiDiagramBuilder;

    // Input coordinates
    std::vector<Coordinate> inputSites;
    for(int i = 0; i < voronoiCentres.size(); ++i){
        inputSites.push_back(Coordinate(voronoiCentres[i].x(), voronoiCentres[i].y()));
    }

    const GeometryFactory* factory = GeometryFactory::getDefaultInstance();
    auto coordSeq = std::make_unique<CoordinateSequence>();
    for (const Coordinate& c : inputSites) {
        coordSeq->add(c);
    }

    // Create MultiPoint from CoordinateSequence
    std::unique_ptr<Geometry> multipoint = factory->createMultiPoint(*coordSeq);

    // Bounding box
    Envelope clipEnv(0, this->nodeWidth - 1, 0, this->nodeHeight-1);
    std::unique_ptr<Geometry> canvas = factory->toGeometry(&clipEnv);

    // Build Voronoi
    VoronoiDiagramBuilder builder;
    builder.setSites(*multipoint);
    builder.setClipEnvelope(&clipEnv);
    std::unique_ptr<Geometry> diagram = builder.getDiagram(*factory);

    // Extract and display each clipped Voronoi cell
    for (std::size_t i = 0; i < diagram->getNumGeometries(); ++i) {
        const Geometry* cell = diagram->getGeometryN(i);

        // Clip to bounding box
        std::unique_ptr<Geometry> clipped = cell->intersection(canvas.get());
        if (clipped->isEmpty()) continue;

        const Polygon* poly = dynamic_cast<const Polygon*>(clipped.get());
        if (!poly) continue;

        // Get the site (input point) this cell corresponds to
        const Geometry* siteGeom = multipoint->getGeometryN(i);
        const CoordinateXY* coordPtr = siteGeom->getCoordinate();
        if(!coordPtr) continue;


        // Extract exterior ring points into a vector
        std::unique_ptr<CoordinateSequence> ring = poly->getExteriorRing()->getCoordinates();
        std::vector<FCord> windingVector;
        for (std::size_t j = 0; j < ring->getSize(); ++j) {
            Coordinate tmpc = ring->getAt(j);
            windingVector.push_back(FCord(tmpc.x, tmpc.y));
        }


        allWindings.push_back(windingVector);
    }

    // start linking voronoi cells to its centre point

    for(int i = 0; i < allWindings.size(); ++i){
        std::vector<FCord> winding = allWindings[i];
        FPGMPolygon poly;
        for(const FCord &fc : winding){
            boost::geometry::append(poly, FPGMPoint(fc.x(), fc.y()));
        }
        boost::geometry::correct(poly);
        bool foundcentre = false;
        for(const Cord &c :voronoiCentres){
            FPGMPoint inside(flen_t(c.x()), flen_t(c.y()));
            if(boost::geometry::within(inside, poly)){
                assert(!foundcentre);
                foundcentre = true;
                voronoiCells[c] = winding;
            }
        }
        assert(foundcentre);
        // if(!foundcentre) std::cout << "No centre found for !" << i << std::endl;
    }
}

void VoronoiPDNGen::mergeVoronoiCells(std::unordered_map<SignalType, std::vector<Cord>> &layerPoints, std::unordered_map<Cord, std::vector<FCord>> &voronoiCellMap, std::unordered_map<SignalType, FPGMMultiPolygon> &multiPolygonMap){
    // merge polygons

    auto printFPGMMultiPolygon = [&](const FPGMMultiPolygon &mp){
        std::cout << "MultiPolygon has " << mp.size() << " piece(s)\n";

        for (size_t i = 0; i < mp.size(); ++i) {
            const FPGMPolygon& poly = mp[i];
    
            // Print exterior ring
            const auto& exterior = poly.outer();
            std::cout << "\nPiece #" << i << ": Exterior contour with "
                      << exterior.size() << " point(s):\n";
            // for (const auto& pt : exterior) {
            //     std::cout << std::fixed << std::setprecision(3)
            //               << "  (" << boost::geometry::get<0>(pt)
            //               << ", " << boost::geometry::get<1>(pt) << ")\n";
            // }
    
            // Print interior rings (holes)
            const auto& holes = poly.inners();
            std::cout << "  " << holes.size() << " hole(s):\n";
            // for (size_t h = 0; h < holes.size(); ++h) {
            //     std::cout << "    Hole #" << h << " with " << holes[h].size() << " point(s):\n";
            //     for (const auto& pt : holes[h]) {
            //         std::cout << std::fixed << std::setprecision(3)
            //                   << "      (" << boost::geometry::get<0>(pt)
            //                   << ", " << boost::geometry::get<1>(pt) << ")\n";
            //     }
            // }
        }
    };


    std::vector<SignalType> allSignalTypes;
    for(std::unordered_map<SignalType, std::vector<Cord>>::const_iterator cit = layerPoints.begin(); cit != layerPoints.end(); ++cit){
        allSignalTypes.push_back(cit->first);
    }

    for(std::unordered_map<SignalType, std::vector<Cord>>::const_iterator cordit = layerPoints.begin(); cordit != layerPoints.end(); ++cordit){
        SignalType rst = cordit->first;
        FPGMMultiPolygon merged;
        for(const Cord &voronoiMidCords : cordit->second){
            std::unordered_map<Cord, std::vector<FCord>>::iterator vcmit = voronoiCellMap.find(voronoiMidCords);
            if(vcmit == voronoiCellMap.end()) continue;

            FPGMPolygon voronoiPoly;
            for(const FCord &fc : vcmit->second){
                boost::geometry::append(voronoiPoly, FPGMPoint(fc.x(), fc.y()));
            }
            boost::geometry::correct(voronoiPoly);
            FPGMMultiPolygon tmp; 
            boost::geometry::union_(merged, voronoiPoly, tmp);
            merged = std::move(tmp);
        }

        multiPolygonMap[rst]= merged;
    }

    // for(const SignalType rst : allSignalTypes){
    //     FPGMMultiPolygon fpmp = multiPolygonMap[rst];
    //     std::cout << rst << std::endl;
    //     printFPGMMultiPolygon(fpmp);
    // }


}

void VoronoiPDNGen::enhanceCrossLayerPI(std::unordered_map<SignalType, FPGMMultiPolygon> &m5PolygonMap, std::unordered_map<SignalType, FPGMMultiPolygon> &m7PolygonMap){
    
    auto calculateSignalArea = [&]() -> std::unordered_map<SignalType, farea_t> {
        
        std::unordered_map<SignalType, farea_t> sigToArea;
        for(std::unordered_map<SignalType, FPGMMultiPolygon>::const_iterator cit = m5PolygonMap.begin(); cit != m5PolygonMap.end(); ++cit){
            
            farea_t area = boost::geometry::area(cit->second);
            if(sigToArea.find(cit->first) == sigToArea.end()) sigToArea[cit->first] = area;
            else sigToArea[cit->first] += area;
        }

        for(std::unordered_map<SignalType, FPGMMultiPolygon>::const_iterator cit = m7PolygonMap.begin(); cit != m7PolygonMap.end(); ++cit){
            
            farea_t area = boost::geometry::area(cit->second);
            if(sigToArea.find(cit->first) == sigToArea.end()) sigToArea[cit->first] = area;
            else sigToArea[cit->first] += area;
        }

        return sigToArea;

    };
    
    std::unordered_set<SignalType> allSignalSet;
    for(std::unordered_map<SignalType, FPGMMultiPolygon>::const_iterator cit = m5PolygonMap.begin(); cit != m5PolygonMap.end(); ++cit){
        allSignalSet.insert(cit->first);
    }
    for(std::unordered_map<SignalType, FPGMMultiPolygon>::const_iterator cit = m7PolygonMap.begin(); cit != m7PolygonMap.end(); ++cit){
        allSignalSet.insert(cit->first);
    }
    std::vector<SignalType> allSignalIdx(allSignalSet.begin(), allSignalSet.end());
    
    for(int upidx = 0; upidx < allSignalIdx.size(); ++upidx){
        for(int dnidx = 0; dnidx < allSignalIdx.size(); ++dnidx){
            if(upidx == dnidx) continue;
            SignalType upSigType = allSignalIdx[upidx];
            SignalType dnSigType = allSignalIdx[dnidx];

            std::unordered_map<SignalType, FPGMMultiPolygon>::iterator it5 = m5PolygonMap.find(upSigType);
            if(it5 == m5PolygonMap.end()) continue;
            FPGMMultiPolygon upMP(it5->second);
            

            std::unordered_map<SignalType, FPGMMultiPolygon>::iterator it7 = m7PolygonMap.find(dnSigType);
            if(it7 == m7PolygonMap.end()) continue;
            FPGMMultiPolygon dnMP(it7->second);

            FPGMMultiPolygon overlap;
            boost::geometry::intersection(upMP, dnMP, overlap);
            if(overlap.empty()) continue;

            std::stack<FPGMMultiPolygon> overlapPieces;
            for (FPGMPolygon const &singlePolygon : overlap) {
                FPGMMultiPolygon onePiece;
                onePiece.push_back(singlePolygon);
                overlapPieces.push(onePiece);
            }
            
            while(!overlapPieces.empty()){
                FPGMMultiPolygon targetPiece(overlapPieces.top());
                FPGMMultiPolygon targetPiece2(overlapPieces.top());
                overlapPieces.pop();

                std::unordered_map<SignalType, farea_t> areaReport =  calculateSignalArea();
                
                if(areaReport[upSigType] < areaReport[dnSigType]){
                    // attempt to paint the overlap area at the M7 layer from dnSigType to upSigType
                    // 1. check whether it will disconnect the doner
                    // 2. check if the added overlap area would introduce discontinuity to donee
                    
                    FPGMMultiPolygon expFrom(m7PolygonMap[dnSigType]);
                    FPGMMultiPolygon expTo;
                    if(m7PolygonMap.find(upSigType) != m7PolygonMap.end()){
                        expTo = m7PolygonMap[upSigType];
                    }
                    FPGMMultiPolygon expFromResult;
                    FPGMMultiPolygon expToResult;

                    boost::geometry::difference(expFrom, targetPiece, expFromResult);
                    boost::geometry::union_(expTo, targetPiece, expToResult);
    
                    if((expFromResult.size() == 1) && (expToResult.size() == 1)){
                        // safe to transfer, execute
                        m7PolygonMap[dnSigType] = expFromResult;
                        m7PolygonMap[upSigType] = expToResult;
                        continue;
                    }
                }
                
                // attempt to print the overlap area at the M5 layer from upSigType to dnSigType
                FPGMMultiPolygon expFrom(m5PolygonMap[upSigType]);
                FPGMMultiPolygon expTo;
                if(m5PolygonMap.find(dnSigType) != m5PolygonMap.end()){
                    expTo = m5PolygonMap[dnSigType];
                }

                FPGMMultiPolygon expFromResult;
                FPGMMultiPolygon expToResult;

                boost::geometry::difference(expFrom, targetPiece2, expFromResult);
                boost::geometry::union_(expTo, targetPiece2, expToResult);

                if((expFromResult.size() == 1) && (expToResult.size() == 1)){
                    // safe to transfer, execute
                    m5PolygonMap[upSigType] = expFromResult;
                    m5PolygonMap[dnSigType] = expToResult;
                }
            }
        }

    }

}

void VoronoiPDNGen::exportToCanvas(std::vector<std::vector<SignalType>> &canvas, std::unordered_map<SignalType, FPGMMultiPolygon> &signalPolygon){
    // std::cout << "Export: " << std::endl;
    // std::cout << this->nodeWidth << " " << this->nodeHeight << std::endl;
    // std::cout << this->canvasWidth << " " << this->canvasHeight << std::endl;
    std::vector<SignalType> allSigType;
    std::vector<FPGMMultiPolygon> allSigPolygon;
    for(std::unordered_map<SignalType, FPGMMultiPolygon>::const_iterator cit = signalPolygon.begin(); cit != signalPolygon.end(); ++cit){
        allSigType.push_back(cit->first);
        allSigPolygon.push_back(FPGMMultiPolygon(cit->second));
    }

    for(int canvasJ = 0; canvasJ < canvasHeight; ++canvasJ){
        for(int canvasI = 0; canvasI < canvasWidth; ++canvasI){
            std::vector<farea_t> signalOverlapArea(allSigType.size(), 0);
            std::vector<FPGMMultiPolygon> polygons = allSigPolygon;
            FPGMPolygon unitBox;
            boost::geometry::append(unitBox, FPGMPoint(canvasI, canvasJ));
            boost::geometry::append(unitBox, FPGMPoint(canvasI+1, canvasJ));
            boost::geometry::append(unitBox, FPGMPoint(canvasI+1, canvasJ+1));
            boost::geometry::append(unitBox, FPGMPoint(canvasI, canvasJ+1));
            boost::geometry::append(unitBox, FPGMPoint(canvasI, canvasJ));
            boost::geometry::correct(unitBox);
            // FPGMMultiPolygon MunitBox;


            std::vector<FPGMMultiPolygon> unitBoxes(allSigType.size(), FPGMMultiPolygon({unitBox}));
            std::vector<FPGMMultiPolygon> resultPolygon(allSigType.size(), FPGMMultiPolygon());
            for(int i = 0; i < allSigType.size(); ++i){
                boost::geometry::intersection(polygons[i], unitBoxes[i], resultPolygon[i]);
                signalOverlapArea[i] = boost::geometry::area(resultPolygon[i]);
                // std::cout << i << " " << signalOverlapArea[i];
            }
            int largestIdx = 0;
            farea_t largestVal = FAREA_T_MIN;
            for(int i = 0; i < allSigType.size(); ++i){
                if(signalOverlapArea[i] > largestVal){
                    largestVal = signalOverlapArea[i];
                    largestIdx = i;
                }
            }

            canvas[canvasJ][canvasI] = allSigType[largestIdx];
        }
    }
}

void VoronoiPDNGen::fixIsolatedCells(std::vector<std::vector<SignalType>> &canvas, const std::unordered_set<SignalType> &obstacles){
   
    const int dx[4] = {-1, 1, 0, 0};
    const int dy[4] = {0, 0, -1, 1};
    
    std::vector<std::vector<int>> clusterArr;
    std::unordered_map<SignalType, std::vector<int>> labeling;
    runClustering(canvas, clusterArr, labeling);

    std::unordered_map<int, area_t> labelArea;
    for(int j = 0; j < clusterArr.size(); ++j){
        for(int i = 0; i < clusterArr[0].size(); ++i){
            int label = clusterArr[j][i];
            if(labelArea.find(label) != labelArea.end()){
                labelArea[label]++;
            }else{
                labelArea[label] = 1;
            }
        }
    }
    // std::cout << "Label Area: " << std::endl;
    // for(auto at : labelArea){
    //     std::cout << at.first << ": " << at.second << std::endl;
    // }
    
    for(std::unordered_map<SignalType, std::vector<int>>::iterator it = labeling.begin(); it != labeling.end(); ++it){
        if(obstacles.count(it->first) == 1) continue;
        if(it->second.size() <= 1) continue;
        SignalType fixingSignalType = it->first;
        std::sort(it->second.begin(), it->second.end(), [&](int x, int y){return labelArea[x] > labelArea[y];});

        for(int fixclusterIdx = 1; fixclusterIdx < it->second.size(); ++fixclusterIdx){
            // collect the coordinates
            int toFixIdx = it->second[fixclusterIdx];
            std::vector<Cord> toFix;
            for(int canvasJ = 0; canvasJ < clusterArr.size(); ++canvasJ){
                for(int canvasI = 0; canvasI < clusterArr[0].size(); ++ canvasI){
                    if(clusterArr[canvasJ][canvasI] == toFixIdx) toFix.push_back(Cord(canvasI, canvasJ));
                }
            }

            for(const Cord &c : toFix){
                std::unordered_map<SignalType, int> sigTypeCounter;
                for(int didx = 0; didx < 4; ++didx){
                    int newX = c.x() + dx[didx];
                    int newY = c.y() + dy[didx];
                    if ((newX > 0) && (newX < canvas[0].size()) && (newY > 0) && (newY < canvas.size())){
                        SignalType st = canvas[newY][newX];
                        if((st == fixingSignalType) || (obstacles.count(st))) continue;
                        if(sigTypeCounter.find(st) == sigTypeCounter.end()) sigTypeCounter[st] = 1;
                        else sigTypeCounter[st]++;
                    }
                }

                int maxsigCount = 0;
                SignalType maxsigtype = SignalType::EMPTY;
                for(std::unordered_map<SignalType, int>::const_iterator cit = sigTypeCounter.begin(); cit != sigTypeCounter.end(); ++cit){
                    if(cit->second > maxsigCount){
                        maxsigCount = cit->second;
                        maxsigtype = cit->first;
                    }
                }
                // std::cout << c << ": " << maxsigtype << std::endl;

                canvas[c.y()][c.x()] = maxsigtype;
            }
        }
    }

    // for(auto at : labeling){
    //     std::cout << at.first << ": ";
    //     for(auto lb : at.second){
    //         std::cout << lb << " ";
    //     }
    //     std::cout << std::endl;
    // }

    // fix 


}