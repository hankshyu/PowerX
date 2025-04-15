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

// 2. Boost Library:

// 3. Texo Library:
#include "powerGrid.hpp"
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
            VNode *node = new VNode;
            node.sig = SignalType::EMPTY;
            node->up = nullptr;
            node->down = nullptr;
            node->left = nullptr;
            node->right = nullptr;
            this->m5NodeArr[j][i] = node;
        }
    }

    for(int j = 0; j < nodeHeight; ++j){
        for(int i = 0; i < nodeWidth; ++i){
            // todo, build edges ...
        }
    }

    for(int j = 0; j < nodeHeight; ++j){
        for(int i = 0; i < nodeWidth; ++i){
            VNode *node = new VNode;
            node.sig = SignalType::EMPTY;
            node->up = nullptr;
            node->down = nullptr;
            node->left = nullptr;
            node->right = nullptr;
            this->m7NodeArr[j][i] = node;
        }
    }

    for(const SignalType &st : this->uBump.allSignalTypes){
        if(m5IgnoreSigs.count(st) != 0) continue;
        this->m5Points[st] = {};
        this->m5Segments[st] = {};
        if(this->uBump.signalTypeToAllCords[st].size() != 0){
            for(const Cord &c : this->uBump.signalTypeToAllCords[st]){
                this->m5Points[st].push_back(c);
            }
        }
    }
    
    for(const SignalType &st : this->c4.allSignalTypes){
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

}

void VoronoiPDNGen::runFLUTERouting(){
    // Initialize LUT using POWV9.dat and PORT9.dat (POST9.dat not needed)
    Flute::FluteState *state = Flute::flute_init("./lib/flute/POWV9.dat", "./lib/flute/PORT9.dat");

    int x[4] = {10, 20, 30, 40};
    int y[4] = {10, 25, 15, 30};

    Flute::Tree tree = Flute::flute(state, 4, x, y, FLUTE_ACCURACY);

    std::cout << "Wirelength: " << tree.length << std::endl;
    // Print branches
    std::cout << "Branches:" << std::endl;
    for (int i = 0; i < tree.deg; ++i) {
        const Flute::Branch& b = tree.branch[i];
        const Flute::Branch& neighbor = tree.branch[b.n];  // The branch this one connects to

        std::cout << "  Branch " << i << ": (" << b.x << ", " << b.y << ")"
                  << " ↔ (" << neighbor.x << ", " << neighbor.y << ")"
                  << " [connected to branch " << b.n << "]" << std::endl;
    }

    // Free resources
    free_tree(state, tree);
    flute_free(state);


}