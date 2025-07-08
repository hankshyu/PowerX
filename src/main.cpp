#include <iostream>
#include <string>
#include <utility>


#include "colours.hpp"
#include "timeProfiler.hpp"
#include "visualiser.hpp"

#include "technology.hpp"
#include "eqCktExtractor.hpp"
#include "signalType.hpp"

#include "diffusionSimulator.hpp"


std::string FILEPATH_TCH = "inputs/standard.tch";
std::string FILEPATH_BUMPS = "inputs/rocket64_0808.pinout";

const std::string TIMERTAG_ASTAR_M5 = "Run A* Baseline Algo on M5";
const std::string TIMERTAG_ASTAR_M7 = "Run A* Baseline Algo on M7";

void printWelcomeBanner();
void printExitBanner();

int main(int argc, char const *argv[]){

    std::vector<std::string> timeSpan = {
       "Canvas Initialize",
       "Transform Signals",
       "Fill Confined Space"
    };

    printWelcomeBanner();
    TimeProfiler timeProfiler;
    
    Technology technology(FILEPATH_TCH);
    EqCktExtractor EqCktExtor(technology);

    /*
    timeProfiler.startTimer(timeSpan[0]);
    DiffusionSimulator diffsim(FILEPATH_BUMPS);
    timeProfiler.pauseTimer(timeSpan[0]);
    
    // timeProfiler.startTimer(timeSpan[1]);
    // diffsim.transformSignals();
    // timeProfiler.pauseTimer(timeSpan[1]);


    // timeProfiler.startTimer(timeSpan[2]);
    // diffsim.fillEnclosedRegions();
    // timeProfiler.pauseTimer(timeSpan[2]);


    // dffs.initialise();

    visualiseGridArrayWithPin(diffsim.metalLayers[0].canvas, diffsim.viaLayers[0].canvas, technology, "outputs/m0.txt");
    visualiseGridArrayWithPins(diffsim.metalLayers[1].canvas, diffsim.viaLayers[0].canvas, diffsim.viaLayers[1].canvas, technology, "outputs/m1.txt");
    visualiseGridArrayWithPin(diffsim.metalLayers[2].canvas, diffsim.viaLayers[1].canvas, technology, "outputs/m2.txt");

    */


    VoronoiPDNGen vpg(FILEPATH_BUMPS);

    vpg.markPreplacedAndInsertPads();


    // vpg.initPointsAndSegments();

    // for(int i = 0; i < (vpg.getMetalLayerCount() - 1); ++i){
    //     vpg.connectLayers(i, i+1);
    // }

    // for(int i = 0; i < vpg.getMetalLayerCount(); ++i){
    //     // vpg.runMSTRouting(vpg.pointsOfLayers[i], vpg.segmentsOfLayers[i]);
    //     vpg.runFLUTERouting(vpg.pointsOfLayers[i], vpg.segmentsOfLayers[i]);
    //     vpg.ripAndReroute(vpg.pointsOfLayers[i], vpg.segmentsOfLayers[i]);
    //     vpg.generateInitialPowerPlanePoints(vpg.pointsOfLayers[i], vpg.segmentsOfLayers[i]);
    //     vpg.generateVoronoiDiagram(vpg.pointsOfLayers[i], vpg.voronoiCellsOfLayers[i]);
    //     vpg.mergeVoronoiCells(vpg.pointsOfLayers[i], vpg.voronoiCellsOfLayers[i], vpg.multiPolygonsOfLayers[i]);
    //     vpg.exportToCanvas(vpg.metalLayers[i].canvas, vpg.multiPolygonsOfLayers[i]);
    //     vpg.obstacleAwareLegalisation(i);
    //     std::cout << "Legalize Checking of layer " << i << ", result = " << vpg.checkOnePiece(i) << std::endl;
    //     vpg.floatingPlaneReconnection(i);
    // }

    // vpg.enhanceCrossLayerPI();


    // for(int i = 0; i < vpg.getMetalLayerCount(); ++i){
    //     vpg.obstacleAwareLegalisation(i);
    //     std::cout << "Legalize Checking of layer " << i << ", result = " << vpg.checkOnePiece(i) << std::endl;
        
    //     vpg.floatingPlaneReconnection(i);
    // }

    vpg.handCraft();
    vpg.assignVias();

    for(int i = 0; i < vpg.getMetalLayerCount(); ++i){
        vpg.removeFloatingPlanes(i);
    }

    
    vpg.exportEquivalentCircuit(SignalType::POWER_1, technology, EqCktExtor, "outputs/POWER1.sp");
    vpg.exportEquivalentCircuit(SignalType::POWER_2, technology, EqCktExtor, "outputs/POWER2.sp");
    vpg.exportEquivalentCircuit(SignalType::POWER_3, technology, EqCktExtor, "outputs/POWER3.sp");
    vpg.exportEquivalentCircuit(SignalType::POWER_4, technology, EqCktExtor, "outputs/POWER4.sp");


    visualiseGridArrayWithPin(vpg.metalLayers[0].canvas, vpg.viaLayers[0].canvas, technology, "outputs/m0.txt");
    visualiseGridArrayWithPins(vpg.metalLayers[1].canvas, vpg.viaLayers[0].canvas, vpg.viaLayers[1].canvas, technology, "outputs/m1.txt");
    visualiseGridArrayWithPin(vpg.metalLayers[2].canvas, vpg.viaLayers[1].canvas, technology, "outputs/m2.txt");

    // visualisePointsSegments(vpg, vpg.pointsOfLayers[0], vpg.segmentsOfLayers[0], "outputs/ps0.txt");
    // visualisePointsSegments(vpg, vpg.pointsOfLayers[1], vpg.segmentsOfLayers[1], "outputs/ps1.txt");
    // visualisePointsSegments(vpg, vpg.pointsOfLayers[2], vpg.segmentsOfLayers[2], "outputs/ps2.txt");

    // visualiseVoronoiGraph(vpg, vpg.pointsOfLayers[0], vpg.voronoiCellsOfLayers[0], "outputs/vd0.txt");
    // visualiseVoronoiGraph(vpg, vpg.pointsOfLayers[1], vpg.voronoiCellsOfLayers[1], "outputs/vd1.txt");
    // visualiseVoronoiGraph(vpg, vpg.pointsOfLayers[2], vpg.voronoiCellsOfLayers[2], "outputs/vd2.txt");
    
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