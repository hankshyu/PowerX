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
#include "voronoiPDNGen.hpp"


// #include "doughnutPolygon.hpp"

std::string FILEPATH_TCH = "inputs/standard.tch";
std::string FILEPATH_BUMPS = "inputs/rocket64_0808.pinout";

const std::string TIMERTAG_ASTAR_M5 = "Run A* Baseline Algo on M5";
const std::string TIMERTAG_ASTAR_M7 = "Run A* Baseline Algo on M7";


void printWelcomeBanner();
void printExitBanner();


int main(int argc, char const *argv[]){

    printWelcomeBanner();
    TimeProfiler timeProfiler;

    Technology technology(FILEPATH_TCH);
    EqCktExtractor EqCktExtor (technology);


    /*
    AStarBaseline AStarBL(FILEPATH_BUMPS);
    visualiseMicroBump(AStarBL.uBump, technology, "outputs/uBump.ub");
    visualiseC4Bump(AStarBL.c4, technology, "outputs/c4.c4");


    timeProfiler.startTimer(TIMERTAG_ASTAR_M5);
    AStarBL.insertPinPads(AStarBL.uBump, AStarBL.canvasM5, AStarBL.defulatM5SigPadMap);
    AStarBL.calculateMST(AStarBL.uBump, AStarBL.canvasM5, {SignalType::GROUND, SignalType::SIGNAL});
    AStarBL.reconnectIslands(AStarBL.canvasM5, {SignalType::EMPTY, SignalType::GROUND, SignalType::SIGNAL, SignalType::OBSTACLE});
    AStarBL.runKNearestNeighbor(AStarBL.canvasM5, {SignalType::EMPTY, SignalType::GROUND, SignalType::SIGNAL, SignalType::OBSTACLE});

    timeProfiler.pauseTimer(TIMERTAG_ASTAR_M5);


    timeProfiler.startTimer(TIMERTAG_ASTAR_M7);
    AStarBL.insertPinPads(AStarBL.c4, AStarBL.canvasM7, AStarBL.defulatM7SigPadMap);
    AStarBL.calculateMST(AStarBL.c4, AStarBL.canvasM7, {SignalType::GROUND, SignalType::SIGNAL});
    AStarBL.reconnectIslands(AStarBL.canvasM7, {SignalType::EMPTY, SignalType::GROUND, SignalType::SIGNAL, SignalType::OBSTACLE});
    AStarBL.runKNearestNeighbor(AStarBL.canvasM7, {SignalType::EMPTY, SignalType::GROUND, SignalType::SIGNAL, SignalType::OBSTACLE});

    timeProfiler.pauseTimer(TIMERTAG_ASTAR_M7);

    AStarBL.reportOverlaps();

    EqCktExtor.exportEquivalentCircuit(AStarBL, SignalType::POWER_4, "outputs/eqckt.sp");


    visualisePGM5(AStarBL, "outputs/rocket64_m5.m5",false, true, false);
    visualisePGM7(AStarBL, "outputs/rocket64_m7.m7",false, false, true);
    visualisePGOverlap(AStarBL, "outputs/rocket64_ov.ov", true, true);
    */
    
    timeProfiler.startTimer("Test ILP");
    VoronoiPDNGen vpg(FILEPATH_BUMPS);
    vpg.initPoints({}, {});
    vpg.connectLayers();
    // vpg.runFLUTERouting();
    
    timeProfiler.pauseTimer("Test ILP");


    timeProfiler.printTimingReport();
    printExitBanner();

    Segment s1(Cord(12, 3), Cord(12, 6));
    Segment s2(Cord(12, 6), Cord(12, 3));
    Segment s3(Cord(12, 5), Cord(15, 4));
    Segment s4(Cord(14, 4), Cord(12, 8));
    Segment s5(Cord(16, 7), Cord(13, 5));
    using namespace boost::polygon::operators;

    std::cout << boost::polygon::intersects(s5, s4, true);
    

    
}

void printWelcomeBanner(){
    std::cout << colours::CYAN << "PowerX: A Power Plane Evaluation and Optimization Tool" << colours::COLORRST << std::endl;

}

void printExitBanner(){
    std::cout << colours::YELLOW << "PowerX Exists Successfully" << colours::COLORRST << std::endl;
    
}