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

VoronoiPDNGen::VoronoiPDNGen(const std::string &fileName): PowerGrid(fileName) {

}

void VoronoiPDNGen::runILPRouting(){
    // Set up LP solver
    OsiClpSolverInterface solver;

    // Variables: x and y
    const int numCols = 2; // two variables 
    const int numRows = 1; // one constraints
    double objective[] = {1.0, 1.0}; // Objective: minimize x + y
    double colLB[] = {0.0, 0.0};
    double colUB[] = {1e20, 1e20};

    // Constraint: x + 2y ≥ 4
    double rowLB[] = {4.0};
    double rowUB[] = {1e20};

    // Matrix for constraint: one row, two columns
    // Column-major storage (compressed sparse column format)
    int starts[] = {0, 1, 2};        // 2 columns → 3 entries
    int indices[] = {0, 0};          // row 0 has two entries (x and y)
    double elements[] = {1.0, 2.0};  // x has coef 1, y has coef 2

    // Load the problem into solver
    solver.loadProblem(numCols, numRows, starts, indices, elements,
                        colLB, colUB, objective, rowLB, rowUB);

    // Make variables integer
    solver.setInteger(0); // x
    solver.setInteger(1); // y

    // Create CBC model and solve
    std::cout << "Try to build model" << std::endl;
    CbcModel model(solver);
    model.branchAndBound();

    // Output results
    const double* solution = model.solver()->getColSolution();
    std::cout << "Solution:\n";
    std::cout << "x = " << solution[0] << "\n";
    std::cout << "y = " << solution[1] << "\n";
        
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