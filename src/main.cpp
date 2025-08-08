#include <iostream>
#include <string>
#include <utility>


#include "colours.hpp"
#include "timeProfiler.hpp"
#include "visualiser.hpp"

#include "technology.hpp"
#include "eqCktExtractor.hpp"
#include "signalType.hpp"

#include "diffusionEngine.hpp"
#include "circuitSolver.hpp"

#include "gurobi_c++.h"

std::string FILEPATH_TCH = "inputs/standard.tch";
std::string FILEPATH_BUMPS = "inputs/rocket64_0808.pinout";

const std::string TIMERTAG_ASTAR_M5 = "Run A* Baseline Algo on M5";
const std::string TIMERTAG_ASTAR_M7 = "Run A* Baseline Algo on M7";

void printWelcomeBanner();
void printExitBanner();

int main(int argc, char **argv){
    // PetscInitialize(&argc, &argv, NULL, NULL);
    std::vector<std::string> timeSpan = {
        "Initialize",
        "Mark PP Pads Canvas",
        "Mark Obstacles Canvas",
        "Init Graph with PP",
        "Fill Enclosed Region",
        
        "Init MCF Solver",
        "Run MCF Solver",
        

        "Init Filler",
        "Init SignalTrees",
        "Initial Evaluation"
        
    };

    printWelcomeBanner();
    TimeProfiler timeProfiler;

    Technology technology(FILEPATH_TCH);
    EqCktExtractor EqCktExtor(technology);
    
    timeProfiler.startTimer(timeSpan[0]);
    DiffusionEngine dse(FILEPATH_BUMPS);
    timeProfiler.pauseTimer(timeSpan[0]);
    
    timeProfiler.startTimer(timeSpan[1]);
    dse.markPreplacedAndInsertPadsOnCanvas();
    timeProfiler.pauseTimer(timeSpan[1]);

    timeProfiler.startTimer(timeSpan[2]);
    dse.markObstaclesOnCanvas();
    timeProfiler.pauseTimer(timeSpan[2]);

    timeProfiler.startTimer(timeSpan[3]);
    dse.initialiseGraphWithPreplaced();
    timeProfiler.pauseTimer(timeSpan[3]);
    
    /*
        // timeProfiler.startTimer(timeSpan[4]);
        // dse.fillEnclosedRegions();
        // timeProfiler.pauseTimer(timeSpan[4]);
    */

    timeProfiler.startTimer(timeSpan[5]);
    dse.initialiseMCFSolver();
    timeProfiler.pauseTimer(timeSpan[5]);

    timeProfiler.startTimer(timeSpan[6]);
    dse.runMCFSolver("", 1);
    dse.verifyAndFixMCFResult(true);
    timeProfiler.pauseTimer(timeSpan[6]);


    dse.exportResultsToFile("outputs/result.txt");

    // dse.importResultsFromFile("outputs/result.txt");


    // timeProfiler.startTimer(timeSpan[7]);
    // dse.initialiseFiller();
    // timeProfiler.pauseTimer(timeSpan[7]);
    // dse.checkFillerInitialisation();

    // timeProfiler.startTimer(timeSpan[8]);
    // dse.initialiseSignalTrees();
    // timeProfiler.pauseTimer(timeSpan[8]);

    // timeProfiler.startTimer(timeSpan[9]);
    // dse.runInitialEvaluation();
    // timeProfiler.pauseTimer(timeSpan[9]);
    
    dse.writeBackToPDN();
    
    visualiseDiffusionEngineMetalAndVia(dse, 0, 0, "outputs/dse_m0_v0.txt");
    visualiseDiffusionEngineMetalAndVia(dse, 1, 0, "outputs/dse_m1_v0.txt");
    visualiseDiffusionEngineMetalAndVia(dse, 1, 1, "outputs/dse_m1_v1.txt");
    visualiseDiffusionEngineMetalAndVia(dse, 2, 1, "outputs/dse_m2_v1.txt");

    visualiseGridArrayWithPin(dse.metalLayers[0].canvas, dse.viaLayers[0].canvas, technology, "outputs/m0.txt");
    visualiseGridArrayWithPins(dse.metalLayers[1].canvas, dse.viaLayers[0].canvas, dse.viaLayers[1].canvas, technology, "outputs/m1.txt");
    visualiseGridArrayWithPin(dse.metalLayers[2].canvas, dse.viaLayers[1].canvas, technology, "outputs/m2.txt");



    // VoronoiPDNGen vpg(FILEPATH_BUMPS);
    // vpg.markPreplacedAndInsertPads();
    // vpg.initPointsAndSegments();

    // for(int i = 0; i < (vpg.getMetalLayerCount() - 1); ++i){
    //     vpg.connectLayers(i, i+1);
    // }

    // for(int i = 0; i < vpg.getMetalLayerCount(); ++i){
    //     // vpg.runMSTRouting(vpg.pointsOfLayers[i], vpg.segmentsOfLayers[i]);
    //     vpg.runFLUTERouting(vpg.pointsOfLayers[i], vpg.segmentsOfLayers[i]);
    // }
    // for(int i = 0; i < vpg.getMetalLayerCount(); ++i){
    //     vpg.ripAndReroute(vpg.pointsOfLayers[i], vpg.segmentsOfLayers[i]);
    // }
    // for(int i = 0; i < vpg.getMetalLayerCount(); ++i){
    //     vpg.generateInitialPowerPlanePoints(vpg.pointsOfLayers[i], vpg.segmentsOfLayers[i]);
    // }
    
    // // visualisePointsSegments(vpg, vpg.pointsOfLayers[0], vpg.segmentsOfLayers[0], "outputs/ps0.txt");
    // // visualisePointsSegments(vpg, vpg.pointsOfLayers[1], vpg.segmentsOfLayers[1], "outputs/ps1.txt");
    // // visualisePointsSegments(vpg, vpg.pointsOfLayers[2], vpg.segmentsOfLayers[2], "outputs/ps2.txt");
    
    // for(int i = 0; i < vpg.getMetalLayerCount(); ++i){
    //     vpg.generateVoronoiDiagram(vpg.pointsOfLayers[i], vpg.voronoiCellsOfLayers[i]);
    // }


    // visualiseVoronoiGraph(vpg, vpg.pointsOfLayers[0], vpg.voronoiCellsOfLayers[0], "outputs/vd0.txt");
    // visualiseVoronoiGraph(vpg, vpg.pointsOfLayers[1], vpg.voronoiCellsOfLayers[1], "outputs/vd1.txt");
    // visualiseVoronoiGraph(vpg, vpg.pointsOfLayers[2], vpg.voronoiCellsOfLayers[2], "outputs/vd2.txt");

    // for(int i = 0; i < vpg.getMetalLayerCount(); ++i){
    //     vpg.mergeVoronoiCells(vpg.pointsOfLayers[i], vpg.voronoiCellsOfLayers[i], vpg.multiPolygonsOfLayers[i]);
    //     vpg.exportToCanvas(vpg.metalLayers[i].canvas, vpg.multiPolygonsOfLayers[i]);
    // }

    // for(int i = 0; i < vpg.getMetalLayerCount(); ++i){
    //     vpg.obstacleAwareLegalisation(i);
    //     vpg.floatingPlaneReconnection(i);
    //     std::cout << "Legalize Checking of layer " << i << ", result = " << vpg.checkOnePiece(i) << std::endl;
    // }

    // vpg.enhanceCrossLayerPI();


    // for(int i = 0; i < vpg.getMetalLayerCount(); ++i){
    //     vpg.obstacleAwareLegalisation(i);
    //     vpg.floatingPlaneReconnection(i);
    //     std::cout << "Legalize Checking of layer " << i << ", result = " << vpg.checkOnePiece(i) << std::endl;
    // }

    // vpg.assignVias();

    // for(int i = 0; i < vpg.getMetalLayerCount(); ++i){
    //     vpg.removeFloatingPlanes(i);
    // }

    
    // vpg.exportEquivalentCircuit(SignalType::POWER_1, technology, EqCktExtor, "outputs/POWER1.sp");
    // vpg.exportEquivalentCircuit(SignalType::POWER_2, technology, EqCktExtor, "outputs/POWER2.sp");
    // vpg.exportEquivalentCircuit(SignalType::POWER_3, technology, EqCktExtor, "outputs/POWER3.sp");
    // vpg.exportEquivalentCircuit(SignalType::POWER_4, technology, EqCktExtor, "outputs/POWER4.sp");


    // visualiseGridArrayWithPin(vpg.metalLayers[0].canvas, vpg.viaLayers[0].canvas, technology, "outputs/m0.txt");
    // visualiseGridArrayWithPins(vpg.metalLayers[1].canvas, vpg.viaLayers[0].canvas, vpg.viaLayers[1].canvas, technology, "outputs/m1.txt");
    // visualiseGridArrayWithPin(vpg.metalLayers[2].canvas, vpg.viaLayers[1].canvas, technology, "outputs/m2.txt");


    // visualiseMultiPolygons(vpg, vpg.multiPolygonsOfLayers[0], "outputs/mp0.txt");
    // visualiseMultiPolygons(vpg, vpg.multiPolygonsOfLayers[1], "outputs/mp1.txt");
    // visualiseMultiPolygons(vpg, vpg.multiPolygonsOfLayers[2], "outputs/mp2.txt");
    
    timeProfiler.printTimingReport();
    return 0;
    

}

void printWelcomeBanner(){
    std::cout << colours::CYAN << "PowerX: A Power Plane Evaluation and Optimization Tool" << colours::COLORRST << std::endl;

}

void printExitBanner(){
    std::cout << colours::YELLOW << "PowerX Exists Successfully" << colours::COLORRST << std::endl;
    
}