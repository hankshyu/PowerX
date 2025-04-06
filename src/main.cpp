#include <iostream>
#include <string>

#include "colours.hpp"
#include "timeProfiler.hpp"
#include "visualiser.hpp"

#include "orderedSegment.hpp"


#include "eqCktExtractor.hpp"
#include "signalType.hpp"
#include "ballOut.hpp"
#include "microBump.hpp"
#include "c4Bump.hpp"
#include "aStarBaseline.hpp"

#include "cadical.hpp"
#include "coin/CbcModel.hpp"
#include "OsiClpSolverInterface.hpp"


// #include "doughnutPolygon.hpp"

std::string FILEPATH_TCH = "inputs/standard.tch";
std::string FILEPATH_BUMPS = "inputs/rocket64_0808.pinout";

const std::string TIMERTAG_ASTAR = "Run A* Baseline Algo.";


void printWelcomeBanner();
void printExitBanner();


int main(int argc, char const *argv[]){

    // printWelcomeBanner();
    // TimeProfiler timeProfiler;

    // Technology technology(FILEPATH_TCH);
    // EqCktExtractor EqCktExtor (technology);



    // timeProfiler.startTimer(TIMERTAG_ASTAR);
    // AStarBaseline AStarBL(FILEPATH_BUMPS);
    // AStarBL.calculateUBumpMST();
    // AStarBL.pinPadInsertion();
    // AStarBL.reconnectAStar();
    // AStarBL.runKNN();
    // timeProfiler.pauseTimer(TIMERTAG_ASTAR);

    // visualiseM5(AStarBL, "outputs/rocket64_m5.m5");

    
    // timeProfiler.printTimingReport();
    // printExitBanner();


    // int arr[3][4] = { {1, 2, 0, 2},
    //                 {0, 0, 1, 0},
    //                 {1, 0, 2, 0} };
    

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
    
        return 0;

}

void printWelcomeBanner(){
    std::cout << colours::CYAN << "PowerX: A Power Plane Evaluation and Optimization Tool" << colours::COLORRST << std::endl;

}

void printExitBanner(){
    std::cout << colours::YELLOW << "PowerX Exists Successfully" << colours::COLORRST << std::endl;
    
}