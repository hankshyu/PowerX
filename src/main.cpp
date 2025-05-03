#include <iostream>
#include <string>

#include "colours.hpp"
#include "timeProfiler.hpp"
#include "visualiser.hpp"
#include "orderedSegment.hpp"

#include "technology.hpp"
#include "eqCktExtractor.hpp"
#include "signalType.hpp"
// #include "ballOut.hpp"
// #include "microBump.hpp"
// #include "c4Bump.hpp"
// #include "aStarBaseline.hpp"
// #include "voronoiPDNGen.hpp"



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
    EqCktExtractor EqCktExtor(technology);

    BallOut c4("inputs/l2.csv");
    visualiseBallOut(c4, technology, "outputs/c4_test.ballout");

    ObjectArray m0(5, 5);
    ObjectArray v0(6, 6);
    ObjectArray v1(6, 6);
   
    m0.readBlockages("inputs/preplaced_m0.csv");
    v0.readBlockages("inputs/preplaced_v0.csv");
    v1.readBlockages("inputs/preplaced_v1.csv");


    m0.markPreplacedToCanvas();
    v0.markPreplacedToCanvas();
    v1.markPreplacedToCanvas();

    // visualiseGridArray(m0.canvas, technology, "outputs/test.txt");
    visualiseGridArrayWithPins(m0.canvas, v0.canvas, v1.canvas, technology, "outputs/test.txt");

    for(auto at : v0.preplacedCords){
        std::cout << at.first << ": " << std::endl;
        for(Cord &c : at.second){
            std::cout << c << " ";
        }
        std::cout << std::endl;
    }

    /*
    timeProfiler.startTimer("Voronoi Diagram Based P/G");
    VoronoiPDNGen vpg(FILEPATH_BUMPS);
    vpg.initPoints({SignalType::GROUND, SignalType::SIGNAL}, {SignalType::GROUND, SignalType::SIGNAL, SignalType::OBSTACLE});
    vpg.connectLayers();

    // vpg.runFLUTERouting(vpg.m5Points, vpg.m5Segments);
    // vpg.runFLUTERouting(vpg.m7Points, vpg.m7Segments);
    
    vpg.runMSTRouting(vpg.m5Points, vpg.m5Segments);
    vpg.runMSTRouting(vpg.m7Points, vpg.m7Segments);


    vpg.ripAndReroute(vpg.m5Points, vpg.m5Segments);
    vpg.ripAndReroute(vpg.m7Points, vpg.m7Segments);


    vpg.generateInitialPowerPlane(vpg.m5Points, vpg.m5Segments);
    vpg.generateInitialPowerPlane(vpg.m7Points, vpg.m7Segments);

    
    vpg.generateVoronoiDiagram(vpg.m5Points, vpg.m5VoronoiCells);
    vpg.generateVoronoiDiagram(vpg.m7Points, vpg.m7VoronoiCells);

    vpg.mergeVoronoiCells(vpg.m5Points, vpg.m5VoronoiCells, vpg.m5MultiPolygons);
    vpg.mergeVoronoiCells(vpg.m7Points, vpg.m7VoronoiCells, vpg.m7MultiPolygons);

    // visualiseM5VoronoiGraph(vpg, "outputs/m5graph.psg");
    // visualiseM7VoronoiGraph(vpg, "outputs/m7graph.psg");

    vpg.enhanceCrossLayerPI(vpg.m5MultiPolygons, vpg.m7MultiPolygons);

    // visualiseM5VoronoiPolygons(vpg, "outputs/m5polygon.polg");
    // visualiseM7VoronoiPolygons(vpg, "outputs/m7polygon.polg");

    vpg.exportToCanvas(vpg.canvasM5, vpg.m5MultiPolygons);
    vpg.insertPinPads(vpg.uBump, vpg.canvasM5, vpg.defulatM5SigPadMap);

    vpg.exportToCanvas(vpg.canvasM7, vpg.m7MultiPolygons);
    vpg.insertPinPads(vpg.c4, vpg.canvasM7, vpg.defulatM7SigPadMap);

    vpg.fixIsolatedCells(vpg.canvasM5, {});
    vpg.fixIsolatedCells(vpg.canvasM7, {SignalType::OBSTACLE});

    visualisePGM5(vpg, "outputs/rocket64_m5.m5",false, true, false);
    visualisePGM7(vpg, "outputs/rocket64_m7.m7",false, false, true);

    // visualiseM5VoronoiPointsSegments(vpg, "outputs/m5.ps");
    // visualiseM7VoronoiPointsSegments(vpg, "outputs/m7.ps");

    timeProfiler.pauseTimer("Voronoi Diagram Based P/G");
    EqCktExtor.exportEquivalentCircuit(vpg, SignalType::POWER_1, "outputs/voroMST.sp")
    */
    /*
    AStarBaseline AStarBL(FILEPATH_BUMPS);
    visualiseMicroBump(AStarBL.uBump, technology, "outputs/uBump.ub");
    visualiseC4Bump(AStarBL.c4, technology, "outputs/c4.c4");


    timeProfiler.startTimer(TIMERTAG_ASTAR_M5);
    AStarBL.insertPinPads(AStarBL.uBump, AStarBL.canvasM5, AStarBL.defulatM5SigPadMap);
    AStarBL.calculateMST(AStarBL.uBump, AStarBL.canvasM5, {SignalType::GROUND, SignalType::SIGNAL});
    AStarBL.reconnectIslands(AStarBL.canvasM5, {SignalType::EMPTY, SignalType::GROUND, SignalType::SIGNAL, SignalType::OBSTACLE});
    AStarBL.runKNearestNeighbor(AStarBL.canvasM5, {SignalType::EMPTY, SignalType::GROUND, SignalType::SIGNAL, SignalType::OBSTACLE});

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


    timeProfiler.printTimingReport();

   
}

void printWelcomeBanner(){
    std::cout << colours::CYAN << "PowerX: A Power Plane Evaluation and Optimization Tool" << colours::COLORRST << std::endl;

}

void printExitBanner(){
    std::cout << colours::YELLOW << "PowerX Exists Successfully" << colours::COLORRST << std::endl;
    
}