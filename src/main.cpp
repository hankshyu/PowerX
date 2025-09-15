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

std::string CASE_NAME;
std::string FILEPATH_TCH;
std::string FILEPATH_BUMPS;
std::string FILEPATH_CONFIG;

void setCaseFromArgs(int argc, char **argv);
void printWelcomeBanner();
void printExitBanner();
void checkSetUp();
void displayGridArrayWithPin(PowerDistributionNetwork &pdn, Technology &tch, bool upDownDisplay = false, const std::string &fileNamePrefix = "outputs/m");
void exportEquivalentCircuits(PowerDistributionNetwork &pdn, Technology &tch, EqCktExtractor &EqCktExtor, const std::string &fileNamePrefix = "outputs/");

void runVoronoiDiagramBasedAlgorithm(bool useFLUTERouting = false, bool displayIntermediateResults = false, bool displayFinalResult = true,  bool exportCircuit = false);
void runMyAlgorithm(int *, char ***, bool displayIntermediateResults = false, bool displayFinalResult = true,  bool exportCircuit = false);

int main(int argc, char **argv){
    setCaseFromArgs(argc, argv);
    printWelcomeBanner();
    
    // checkSetUp();
    // runVoronoiDiagramBasedAlgorithm(false, true, true, true);
    runMyAlgorithm(&argc, &argv, true, true, true);
    
    printExitBanner();

}

void setCaseFromArgs(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "[Error] Missing case argument. Usage: ./elf case01~case06\n";
        std::exit(EXIT_FAILURE);
    }

    std::string inputCase = argv[1];
    std::unordered_set<std::string> validCases = {"case01", "case02", "case03", "case04", "case05", "case06"};

    bool valid = validCases.count(inputCase);

    if (!valid) {
        std::cerr << "[Error] Invalid case \"" << inputCase << "\". Allowed: case01 ~ case06.\n";
        std::exit(EXIT_FAILURE);
    }

    // Assign the case
    CASE_NAME = inputCase;

    // Update file paths
    FILEPATH_TCH    = "inputs/" + CASE_NAME + "/" + CASE_NAME + ".tch";
    FILEPATH_BUMPS  = "inputs/" + CASE_NAME + "/" + CASE_NAME + ".pinout";
    FILEPATH_CONFIG = "inputs/" + CASE_NAME + "/" + CASE_NAME + ".config";
}

void printWelcomeBanner(){
    std::cout << colours::CYAN << "PowerX: A Power Plane Evaluation and Optimization Tool, running " << CASE_NAME << colours::COLORRST << std::endl;
}

void printExitBanner(){
    std::cout << colours::YELLOW << "PowerX Exists Successfully" << colours::COLORRST << std::endl; 
}

void displayGridArrayWithPin(PowerDistributionNetwork &pdn, Technology &tch, bool upDownDisplay, const std::string &fileNamePrefix){
    for(size_t layer = 0; layer < pdn.getMetalLayerCount(); ++layer){

        if(layer == 0){
            visualiseGridArrayWithPin(pdn.metalLayers[layer].canvas, pdn.viaLayers[layer].canvas, tch, fileNamePrefix+std::to_string(layer)+".txt");

        }else if(layer == (pdn.getMetalLayerCount() - 1)){
            visualiseGridArrayWithPin(pdn.metalLayers[layer].canvas, pdn.viaLayers[layer-1].canvas, tch, fileNamePrefix+std::to_string(layer)+".txt");

        }else{
            visualiseGridArrayWithPins(pdn.metalLayers[layer].canvas, pdn.viaLayers[layer-1].canvas, pdn.viaLayers[layer].canvas, tch, fileNamePrefix+std::to_string(layer)+".txt");
            if(upDownDisplay){
                visualiseGridArrayWithPin(pdn.metalLayers[layer].canvas, pdn.viaLayers[layer-1].canvas, tch, fileNamePrefix+std::to_string(layer)+"_up.txt");
                visualiseGridArrayWithPin(pdn.metalLayers[layer].canvas, pdn.viaLayers[layer].canvas, tch, fileNamePrefix+std::to_string(layer)+"_down.txt");
            }
        }
    }
}

void exportEquivalentCircuits(PowerDistributionNetwork &pdn, Technology &tch, EqCktExtractor &EqCktExtor, const std::string &fileNamePrefix){
    for(SignalType st : pdn.uBump.allSignalTypes){
        std::string cktFileName = fileNamePrefix + to_string(st) + ".sp";
        pdn.exportEquivalentCircuit(st, tch, EqCktExtor, cktFileName);
    }

    // vpg.exportEquivalentCircuit(SignalType::POWER_1, technology, EqCktExtor, "outputs/POWER1.sp");
    // vpg.exportEquivalentCircuit(SignalType::POWER_2, technology, EqCktExtor, "outputs/POWER2.sp");
    // vpg.exportEquivalentCircuit(SignalType::POWER_3, technology, EqCktExtor, "outputs/POWER3.sp");
}

void checkSetUp(){
    Technology technology(FILEPATH_TCH);
    EqCktExtractor EqCktExtor(technology);
    
    DiffusionEngine dse(FILEPATH_BUMPS, FILEPATH_CONFIG);
    dse.markPreplacedAndInsertPadsOnCanvas();

    displayGridArrayWithPin(dse, technology, true);
    dse.checkVias();

    // std::cout << colours::MAGENTA << "Pass Via validity test of " << CASE_NAME  << colours::COLORRST << std::endl;
}

void runVoronoiDiagramBasedAlgorithm(bool useFLUTERouting, bool displayIntermediateResults, bool displayFinalResult, bool exportCircuit){
    
    TimeProfiler timeProfiler;
    timeProfiler.startTimer("Preprocessing");

        Technology technology(FILEPATH_TCH);
        EqCktExtractor EqCktExtor(technology); 
        VoronoiPDNGen vpg(FILEPATH_BUMPS);


        auto displayPointsSegments = [&](std::string fileNamePrefix){
            for(int layer = 0; layer < vpg.getMetalLayerCount(); ++layer){
                std::string displayFileName = fileNamePrefix + std::to_string(layer) + ".txt";
                visualisePointsSegments(vpg, vpg.pointsOfLayers[layer], vpg.segmentsOfLayers[layer], displayFileName);
            }
        };

        auto displayFPointsSegments = [&](std::string fileNamePrefix){
            for(int layer = 0; layer < vpg.getMetalLayerCount(); ++layer){
                std::string displayFileName = fileNamePrefix + std::to_string(layer) + ".txt";
                visualisePointsSegments(vpg, vpg.fPointsOfLayers[layer], vpg.fSegmentsOfLayers[layer], displayFileName);
            }
        };

        auto displayVoronoiGraphs = [&](std::string fileNamePrefix){
            for(int layer = 0; layer < vpg.getMetalLayerCount(); ++layer){
                std::string displayFileName = fileNamePrefix + std::to_string(layer) + ".txt";
                // visualiseVoronoiGraph(vpg, vpg.pointsOfLayers[layer], vpg.voronoiCellsOfLayers[layer], displayFileName);
                visualiseVoronoiGraph(vpg, vpg.fPointsOfLayers[layer], vpg.fVoronoiCellsOfLayers[layer], displayFileName);
            }
        };

        auto displayVoronoiPolygons = [&](std::string fileNamePrefix){
            for(int layer = 0; layer < vpg.getMetalLayerCount(); ++layer){
                std::string displayFileName = fileNamePrefix + std::to_string(layer) + ".txt";
                visualiseMultiPolygons(vpg, vpg.multiPolygonsOfLayers[layer], displayFileName);

            }
        };

        auto displayPhysicalImplementation = [&](std::string fileNamePrefix){
            for(int layer = 0; layer < vpg.getMetalLayerCount(); ++layer){
                std::string displayFileName = fileNamePrefix + std::to_string(layer) + ".txt";
                visualisePhysicalImplementation(vpg, layer, displayFileName);
            }
        };

        /* Start Execution */


        vpg.markPreplacedAndInsertPads();
        if(displayIntermediateResults) displayGridArrayWithPin(vpg, technology, false, "outputs/init_gawp_m");

        vpg.initPointsAndSegments();
        if(displayIntermediateResults) displayPointsSegments("outputs/init_ps_m");

        for(int i = 0; i < (vpg.getMetalLayerCount() - 1); ++i){
            vpg.connectLayers(i, i+1);
        }
        if(displayIntermediateResults) displayPointsSegments("outputs/conn_ps_m");

    timeProfiler.pauseTimer("Preprocessing");


    timeProfiler.startTimer("Routing");
        for(int i = 0; i < vpg.getMetalLayerCount(); ++i){
            if(!useFLUTERouting) vpg.runMSTRouting(vpg.pointsOfLayers[i], vpg.segmentsOfLayers[i]);
            else vpg.runFLUTERouting(vpg.pointsOfLayers[i], vpg.segmentsOfLayers[i]);
        }
        if(displayIntermediateResults) displayPointsSegments("outputs/route_ps_m");
    timeProfiler.pauseTimer("Routing");
    

    timeProfiler.startTimer("Rip And Reroute");
        for(int i = 0; i < vpg.getMetalLayerCount(); ++i){
            vpg.ripAndReroute(vpg.pointsOfLayers[i], vpg.segmentsOfLayers[i]);
        }
        if(displayIntermediateResults) displayPointsSegments("outputs/reroute_ps_m");
    timeProfiler.pauseTimer("Rip And Reroute");


    timeProfiler.startTimer("Gen Voronoi Diagram");
        for(int i = 0; i < vpg.getMetalLayerCount(); ++i){
            vpg.generateInitialPowerPlanePointsX2(vpg.pointsOfLayers[i], vpg.segmentsOfLayers[i], vpg.fPointsOfLayers[i], vpg.fSegmentsOfLayers[i]);
        }
        
        if(displayIntermediateResults) displayFPointsSegments("outputs/addvdpoints_ps_m");

        for(int i = 0; i < vpg.getMetalLayerCount(); ++i){
            vpg.generateVoronoiDiagramX(vpg.fPointsOfLayers[i], vpg.fVoronoiCellsOfLayers[i]);
            // std::cout << "Completed Generating voronoi for metal layer " << i << std::endl;
        }

        if(displayIntermediateResults) displayVoronoiGraphs("outputs/genvd_vg_m");

        for(int i = 0; i < vpg.getMetalLayerCount(); ++i){
            // vpg.mergeVoronoiCells(vpg.pointsOfLayers[i], vpg.voronoiCellsOfLayers[i], vpg.multiPolygonsOfLayers[i]);
            vpg.mergeVoronoiCellsX(vpg.fPointsOfLayers[i], vpg.fVoronoiCellsOfLayers[i], vpg.multiPolygonsOfLayers[i]);

            vpg.exportToCanvas(vpg.metalLayers[i].canvas, vpg.multiPolygonsOfLayers[i]);
        }

        if(displayIntermediateResults) displayVoronoiPolygons("outputs/mergevp_vp_m");
        if(displayIntermediateResults) displayGridArrayWithPin(vpg, technology, false, "outputs/postvd_gawp_m");

    timeProfiler.pauseTimer("Gen Voronoi Diagram");


    timeProfiler.startTimer("Legalisation Stage");
        for(int i = 0; i < vpg.getMetalLayerCount(); ++i){
            vpg.obstacleAwareLegalisation(i);
            vpg.floatingPlaneReconnection(i);
            std::cout << "[Legalisation] Layer " << i << " Aggregation, result = " << vpg.checkOnePiece(i) << std::endl;
        }
        if(displayIntermediateResults) displayGridArrayWithPin(vpg, technology, false, "outputs/leg_gawp_m");

    timeProfiler.pauseTimer("Legalisation Stage");


    timeProfiler.startTimer("Cross-Layer PI Enhancement");
        vpg.enhanceCrossLayerPINew();
        if(displayIntermediateResults) displayGridArrayWithPin(vpg, technology, false, "outputs/enhance_gawp_m");

    timeProfiler.pauseTimer("Cross-Layer PI Enhancement");


    timeProfiler.startTimer("ReLegalisation Stage");
        for(int i = 0; i < vpg.getMetalLayerCount(); ++i){
            vpg.obstacleAwareLegalisation(i);
            vpg.floatingPlaneReconnection(i);
            std::cout << "[ReLegalisation] Layer " << i << " Aggregation, result = " << vpg.checkOnePiece(i) << std::endl;
        }
        if(displayIntermediateResults) displayGridArrayWithPin(vpg, technology, false, "outputs/releg_gawp_m");

    timeProfiler.pauseTimer("ReLegalisation Stage");

    timeProfiler.startTimer("Post-Processing");
        vpg.assignVias();
        for(int i = 0; i < vpg.getMetalLayerCount(); ++i){
            vpg.removeFloatingPlanes(i);
        }
        if(displayIntermediateResults) displayGridArrayWithPin(vpg, technology, false, "outputs/postp_gawp_m");    
    timeProfiler.pauseTimer("Post-Processing");

    vpg.writeSnapShot("snap.txt");
    // vpg.readSnapShot("snap.txt");
    if(displayIntermediateResults) displayGridArrayWithPin(vpg, technology, false, "outputs/postp_gawp_m");    

    // Start Physical Implementation
    timeProfiler.startTimer("Physcial Realisation");
    vpg.buildPhysicalImplementation();
    if(displayIntermediateResults) displayPhysicalImplementation("outputs/phyrlz_pi_m");
    vpg.connectivityAwareAssignment();
    if(displayIntermediateResults) displayPhysicalImplementation("outputs/phyrlz_pi2_m");
    timeProfiler.pauseTimer("Physcial Realisation");


    if(displayFinalResult) displayPhysicalImplementation("outputs/fnl_fnl_m");
    if(exportCircuit) vpg.exportPhysicalToCircuit(technology, EqCktExtor, "outputs/");

    timeProfiler.printTimingReport();

}

void runMyAlgorithm(int *argc, char ***argv, bool displayIntermediateResults, bool displayFinalResult,  bool exportCircuit){
    
    TimeProfiler timeProfiler;
    timeProfiler.startTimer("Preprocessing");

        Technology technology(FILEPATH_TCH);
        EqCktExtractor EqCktExtor(technology);
        DiffusionEngine dse(FILEPATH_BUMPS, FILEPATH_CONFIG);


        auto displayPhysicalImplementation = [&](std::string fileNamePrefix){
            for(int layer = 0; layer < dse.getMetalLayerCount(); ++layer){
                std::string displayFileName = fileNamePrefix + std::to_string(layer) + ".txt";
                visualisePhysicalImplementation(dse, layer, displayFileName);
            }
        };

        
        PetscInitialize(argc, argv, NULL, NULL);
        
        dse.markPreplacedAndInsertPadsOnCanvas();
        if(displayIntermediateResults) displayGridArrayWithPin(dse, technology, false, "outputs/2init_gawp_m");
        
        dse.markObstaclesOnCanvas();
        dse.initialiseGraphWithPreplaced();
        if(displayIntermediateResults) displayGridArrayWithPin(dse, technology, false, "outputs/2fillobst_gawp_m");
        
        dse.fillEnclosedRegions();
        if(displayIntermediateResults){
            dse.writeBackToPDN();
            displayGridArrayWithPin(dse, technology, false, "outputs/2fillEnclosed_gawp_m");
        } 

    timeProfiler.pauseTimer("Preprocessing");


    timeProfiler.startTimer("MCF Stage");
        dse.initialiseMCFSolver();
        dse.runMCFSolver("", 1);
        if(displayIntermediateResults){
            dse.writeBackToPDN();
            displayGridArrayWithPin(dse, technology, false, "outputs/2mcfraw_gawp_m");
        }

    timeProfiler.pauseTimer("MCF Stage");

    timeProfiler.startTimer("Post-MCF Repair & WB");

        dse.postMCFLocalRepairTop(true);
        
        if(displayIntermediateResults){
            dse.writeBackToPDN();
            displayGridArrayWithPin(dse, technology, false, "outputs/2mcffix_gawp_m");
        } 

        // dse.exportResultsToFile("outputs/result.txt");
        // dse.importResultsFromFile("outputs/result.txt");

    timeProfiler.pauseTimer("Post-MCF Repair & WB");

    dse.writeBackToPDN();
    
    size_t blankCountMetal = 0;
    size_t blankCountVia = 0;

    for(size_t layer = 0; layer < dse.getMetalLayerCount(); ++layer){
        for(size_t y = 0; y < dse.getGridHeight(); ++y){
            for(size_t x = 0; x < dse.getGridWidth(); ++x){
                if(dse.metalLayers[layer].canvas[y][x] == SignalType::EMPTY) blankCountMetal++;

            }
        }
    }

    for(size_t layer = 0; layer < dse.getViaLayerCount(); ++layer){
        for(size_t y = 0; y < dse.getPinHeight(); ++y){
            for(size_t x = 0; x < dse.getPinWidth(); ++x){
                if(dse.viaLayers[layer].canvas[y][x] == SignalType::EMPTY) blankCountVia++;

            }
        }
    }

    std::cout << "Empty Metal/Via Count = " << colours::GREEN << blankCountMetal << "/" << blankCountVia << colours::COLORRST << std::endl;


    timeProfiler.startTimer("R-based Filling Stage");
        dse.initialiseFiller();
        // dse.checkFillerInitialisation();
    
        dse.initialiseSignalTreesX();
        dse.runInitialEvaluationX();


        // dse.initialiseSignalTrees();
        // dse.runInitialEvaluation();


    timeProfiler.pauseTimer("R-based Filling Stage");

    // // timeProfiler.startTimer("R-based Filling Iterate");
    // //     dse.evaluateAndFill();
    // //     if(displayIntermediateResults){
    // //         dse.writeBackToPDN();
    // //         displayGridArrayWithPin(dse, technology, false, "outputs/2rfill_gawp_m");
    // //     }
    // // timeProfiler.pauseTimer("R-based Filling Iterate");
    
    timeProfiler.startTimer("R-based Filling Iterate");
        dse.evaluateAndFillX();
        if(displayIntermediateResults){
            dse.writeBackToPDN();
            displayGridArrayWithPin(dse, technology, false, "outputs/2rfill_gawp_m");
        }
    timeProfiler.pauseTimer("R-based Filling Iterate");




    timeProfiler.startTimer("Post-Processing");
        dse.assignVias();
        for(int i = 0; i < dse.getMetalLayerCount(); ++i){
            dse.removeFloatingPlanes(i);
        }
        if(displayIntermediateResults) displayGridArrayWithPin(dse, technology, false, "outputs/2postp_gawp_m");    
    timeProfiler.pauseTimer("Post-Processing");

    // Start Physical Implementation
    timeProfiler.startTimer("Physcial Realisation");
        dse.buildPhysicalImplementation();
        if(displayIntermediateResults) displayPhysicalImplementation("outputs/2phyrlz_pi_m");
        dse.connectivityAwareAssignment();
        if(displayIntermediateResults) displayPhysicalImplementation("outputs/2phyrlz_pi2_m");
    timeProfiler.pauseTimer("Physcial Realisation");


    if(displayFinalResult) displayPhysicalImplementation("outputs/2fnl_fnl_m");
    if(exportCircuit) dse.exportPhysicalToCircuit(technology, EqCktExtor, "outputs/");

    timeProfiler.printTimingReport();

}
