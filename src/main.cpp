#include <iostream>
#include <string>
#include <utility>


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

#include "line.hpp"
#include "rectangle.hpp"
#include "doughnutPolygon.hpp"
#include "doughnutPolygonSet.hpp"

//

#include "fpoint.hpp"
#include "fbox.hpp"
#include "fpolygon.hpp"
#include "fmultipolygon.hpp"

#include "pointBinSystem.hpp"
#include "pressureSimulator.hpp"
#include "rectangleBinSystem.hpp"


std::string FILEPATH_TCH = "inputs/standard.tch";
std::string FILEPATH_BUMPS = "inputs/rocket64_0808.pinout";

const std::string TIMERTAG_ASTAR_M5 = "Run A* Baseline Algo on M5";
const std::string TIMERTAG_ASTAR_M7 = "Run A* Baseline Algo on M7";

void printWelcomeBanner();
void printExitBanner();

int main(int argc, char const *argv[]){

    printWelcomeBanner();
    TimeProfiler timeProfiler;
    timeProfiler.startTimer("My Algorithm");

    // Technology technology(FILEPATH_TCH);
    // EqCktExtractor EqCktExtor(technology);
    // PressureSimulator pressureSim(FILEPATH_BUMPS);

    FBox b1(FPoint(3, 3), FPoint(6.5, 7));
    FBox b2(FPoint(7.5, 3.4), FPoint(11.5, 4.4));
    FBox b3(FPoint(8.2, 7), FPoint(9.0, 9.0));

    FBox red(FPoint(6, 6.5), FPoint(8, 9));
    FBox green(FPoint(5.5, 4.4), FPoint(8.3, 8));

    RectangleBinSystem<flen_t, FBox> rbs(5, 0, 0, 15, 10);
    rbs.insert(3, 3, 6.5, 7, &b1);
    rbs.insert(7.5, 3.4, 11.5, 4.4, &b2);
    rbs.insert(8.2, 7, 9, 9, &b3);

    rbs.insert(6, 6.5, 8, 9, &red);
    rbs.insert(5.5, 4.4, 8.3, 8, &green);

    rbs.remove(&green);
    rbs.insert(5.5, 4.4, 8.3, 8, &green);


    std::vector<FBox *> answer = rbs.queryPoint(6.3, 6.9);
    for(auto at : answer){
        std::cout << *at << std::endl;
    }

    std::vector<std::pair<FBox *, FBox *>> ans = rbs.reportOverlap();
    for(const auto &[at, bt] : ans){
        std::cout << *at << " <-> " << *bt << std::endl; 
    }




    
    timeProfiler.pauseTimer("My Algorithm");
    timeProfiler.printTimingReport();
    
    return 0;

    /*
    VoronoiPDNGen vpg(FILEPATH_BUMPS);

    vpg.markPreplacedAndInsertPads();
    vpg.initPointsAndSegments();

    for(int i = 0; i < (vpg.getMetalLayerCount() - 1); ++i){
        vpg.connectLayers(i, i+1);
    }

    for(int i = 0; i < vpg.getMetalLayerCount(); ++i){
        vpg.runMSTRouting(vpg.pointsOfLayers[i], vpg.segmentsOfLayers[i]);
        vpg.ripAndReroute(vpg.pointsOfLayers[i], vpg.segmentsOfLayers[i]);
        vpg.generateInitialPowerPlanePoints(vpg.pointsOfLayers[i], vpg.segmentsOfLayers[i]);
        vpg.generateVoronoiDiagram(vpg.pointsOfLayers[i], vpg.voronoiCellsOfLayers[i]);
        vpg.mergeVoronoiCells(vpg.pointsOfLayers[i], vpg.voronoiCellsOfLayers[i], vpg.multiPolygonsOfLayers[i]);
        vpg.exportToCanvas(vpg.metalLayers[i].canvas, vpg.multiPolygonsOfLayers[i]);
        vpg.obstacleAwareLegalisation(i);
        std::cout << "Legalize Checking of layer " << i << ", result = " << vpg.checkOnePiece(i) << std::endl;
        vpg.floatingPlaneReconnection(i);
    }

    vpg.enhanceCrossLayerPI();


    for(int i = 0; i < vpg.getMetalLayerCount(); ++i){
        vpg.obstacleAwareLegalisation(i);
        std::cout << "Legalize Checking of layer " << i << ", result = " << vpg.checkOnePiece(i) << std::endl;
        
        vpg.floatingPlaneReconnection(i);
    }
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

    visualisePointsSegments(vpg, vpg.pointsOfLayers[0], vpg.segmentsOfLayers[0], "outputs/ps0.txt");
    visualisePointsSegments(vpg, vpg.pointsOfLayers[1], vpg.segmentsOfLayers[1], "outputs/ps1.txt");
    visualisePointsSegments(vpg, vpg.pointsOfLayers[2], vpg.segmentsOfLayers[2], "outputs/ps2.txt");

    visualiseVoronoiGraph(vpg, vpg.pointsOfLayers[0], vpg.voronoiCellsOfLayers[0], "outputs/vd0.txt");
    visualiseVoronoiGraph(vpg, vpg.pointsOfLayers[1], vpg.voronoiCellsOfLayers[1], "outputs/vd1.txt");
    visualiseVoronoiGraph(vpg, vpg.pointsOfLayers[2], vpg.voronoiCellsOfLayers[2], "outputs/vd2.txt");
    
    visualiseMultiPolygons(vpg, vpg.multiPolygonsOfLayers[0], "outputs/mp0.txt");
    visualiseMultiPolygons(vpg, vpg.multiPolygonsOfLayers[1], "outputs/mp1.txt");
    visualiseMultiPolygons(vpg, vpg.multiPolygonsOfLayers[2], "outputs/mp2.txt");
    */
    

}

void printWelcomeBanner(){
    std::cout << colours::CYAN << "PowerX: A Power Plane Evaluation and Optimization Tool" << colours::COLORRST << std::endl;

}

void printExitBanner(){
    std::cout << colours::YELLOW << "PowerX Exists Successfully" << colours::COLORRST << std::endl;
    
}