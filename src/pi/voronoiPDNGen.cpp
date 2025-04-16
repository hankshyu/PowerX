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
// 2. Boost Library:

// 3. Texo Library:
#include "units.hpp"
#include "powerGrid.hpp"
#include "segment.hpp"
#include "orderedSegment.hpp"
#include "voronoiPDNGen.hpp"

// 4. Cadical SAT solver
#include "cadical.hpp"

// 5. ILP library COIN-OR (CBC)
#include "coin/CbcModel.hpp"
#include "OsiClpSolverInterface.hpp"

// 6. FLUTE
#include "flute.h"

VoronoiPDNGen::VoronoiPDNGen(const std::string &fileName): PowerGrid(fileName) {

}

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

void VoronoiPDNGen::runFLUTERouting(const std::string &wirelengthVectorFile, const std::string &RoutingTreeFile){
    const int FLUTE_ACC = 9; // 0 ~ 9
    for(std::unordered_map<SignalType, std::vector<Cord>>::iterator it = this->m5Points.begin(); it != this->m5Points.end(); ++it){
        SignalType targetSig = it->first;
        int pointCount = it->second.size();
        if(pointCount == 0) continue;
        std::vector<len_t> xArr;
        std::vector<len_t> yArr;
        for(const Cord &c : it->second){
            xArr.push_back(c.x());
            yArr.push_back(c.y());
        }

        Flute::FluteState *state = Flute::flute_init(wirelengthVectorFile.c_str(), RoutingTreeFile.c_str());
        Flute::Tree tree = Flute::flute(state, pointCount, xArr.data(), yArr.data(), FLUTE_ACC);
        std::cout << targetSig << "Before: " << m5Points[targetSig].size() << std::endl;
        for(int i = 0; i < tree.deg*2 - 2; ++i){
            int n = tree.branch[i].n;
            Cord c1(tree.branch[i].x, tree.branch[i].y);
            Cord c2(tree.branch[n].x, tree.branch[n].y);
            bool foundc1 = false;
            bool foundc2 = false;
            for(int j = 0; j < this->m5Points[targetSig].size(); ++j){
                Cord m5c = m5Points[targetSig].at(j);
                if(m5c == c1) foundc1 = true;
            }
            for(int j = 0; j < this->m5Points[targetSig].size(); ++j){
                Cord m5c = m5Points[targetSig].at(j);
                if(m5c == c2) foundc2 = true;
            }
            if(!foundc1) m5Points[targetSig].push_back(c1);
            if(!foundc2) m5Points[targetSig].push_back(c2);

            OrderedSegment newos(c1, c2);
            bool foundos = false;
            for(OrderedSegment os : m5Segments[targetSig]){
                if(os == newos) foundos = true;
            }
            if(!foundos) m5Segments[targetSig].push_back(newos);


        }

        std::cout << "after: " << m5Points[targetSig].size() << std::endl;
        std::cout << "segs: " << m5Segments[targetSig].size() << std::endl; 

        std::cout << "Wirelength: " << tree.length << std::endl;
        // Print branches
        std::cout << "Branches:" << std::endl;
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


