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

    printWelcomeBanner();
    TimeProfiler timeProfiler;

    Technology technology(FILEPATH_TCH);
    EqCktExtractor EqCktExtor (technology);



    timeProfiler.startTimer(TIMERTAG_ASTAR);
    AStarBaseline AStarBL(FILEPATH_BUMPS);
    AStarBL.insertPinPads(AStarBL.uBump, AStarBL.canvasM5, AStarBL.defulatM5SigPadMap);
    AStarBL.insertPinPads(AStarBL.c4, AStarBL.canvasM7, AStarBL.defulatM7SigPadMap);

    // AStarBL.calculateUBumpMST();
    // AStarBL.pinPadInsertion();
    // AStarBL.reconnectAStar();
    // AStarBL.runKNN();
    timeProfiler.pauseTimer(TIMERTAG_ASTAR);

    visualisePGM5(AStarBL, "outputs/rocket64_m5.m5",true, true, true);
    visualisePGM7(AStarBL, "outputs/rocket64_m7.m7",true, false, true);
    visualisePGOverlap(AStarBL, "outputs/rocket64_ov.ov", true, true);
    
    timeProfiler.printTimingReport();
    printExitBanner();


    // int arr[3][4] = { {1, 2, 0, 2},
    //                 {0, 0, 1, 0},
    //                 {1, 0, 2, 0} };
    
}

void printWelcomeBanner(){
    std::cout << colours::CYAN << "PowerX: A Power Plane Evaluation and Optimization Tool" << colours::COLORRST << std::endl;

}

void printExitBanner(){
    std::cout << colours::YELLOW << "PowerX Exists Successfully" << colours::COLORRST << std::endl;
    
}