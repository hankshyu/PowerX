#include <iostream>
#include <string>

#include "colours.hpp"
#include "timeProfiler.hpp"
#include "visualiser.hpp"
#include "orderedSegment.hpp"

#include "technology.hpp"
#include "eqCktExtractor.hpp"
#include "signalType.hpp"
#include "objectArray.hpp"
#include "c4Bump.hpp"
#include "microBump.hpp"
#include "voronoiPDNGen.hpp"

// #include "ballOut.hpp"
// #include "aStarBaseline.hpp"



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
    
    VoronoiPDNGen vpg(FILEPATH_BUMPS);

    timeProfiler.startTimer("Voronoi Diagram Based P/G");

    vpg.markPreplacedAndInsertPads();
    vpg.initPointsAndSegments();

    for(int i = 0; i < (vpg.getMetalLayerCount() - 1); ++i){
        vpg.connectLayers(i, i+1);
    }

    for(int i = 0; i < vpg.getMetalLayerCount(); ++i){
        vpg.runFLUTERouting(vpg.pointsOfLayers[i], vpg.segmentsOfLayers[i]);
    }

    for(int i = 0; i < vpg.getMetalLayerCount(); ++i){
        vpg.ripAndReroute(vpg.pointsOfLayers[i], vpg.segmentsOfLayers[i]);
    }

    // visualiseGridArrayWithPin(vpg.metalLayers[0].canvas, vpg.viaLayers[0].canvas, technology, "outputs/m0.txt");
    // visualiseGridArrayWithPins(vpg.metalLayers[1].canvas, vpg.viaLayers[0].canvas, vpg.viaLayers[1].canvas, technology, "outputs/m1.txt");
    // visualiseGridArrayWithPin(vpg.metalLayers[2].canvas, vpg.viaLayers[1].canvas, technology, "outputs/m2.txt");

    visualisePointsSegments(vpg, vpg.pointsOfLayers[0], vpg.segmentsOfLayers[0], "outputs/ps0.txt");
    visualisePointsSegments(vpg, vpg.pointsOfLayers[1], vpg.segmentsOfLayers[1], "outputs/ps1.txt");
    visualisePointsSegments(vpg, vpg.pointsOfLayers[2], vpg.segmentsOfLayers[2], "outputs/ps2.txt");
    timeProfiler.pauseTimer("Voronoi Diagram Based P/G");
    timeProfiler.printTimingReport();
    
    /*
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
}

void printWelcomeBanner(){
    std::cout << colours::CYAN << "PowerX: A Power Plane Evaluation and Optimization Tool" << colours::COLORRST << std::endl;

}

void printExitBanner(){
    std::cout << colours::YELLOW << "PowerX Exists Successfully" << colours::COLORRST << std::endl;
    
}