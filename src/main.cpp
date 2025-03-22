#include <iostream>
#include <string>

#include "colours.hpp"
#include "timeProfiler.hpp"
#include "visualiser.hpp"

#include "powerPlane.hpp"
#include "eqCktExtractor.hpp"



// #include "doughnutPolygon.hpp"

namespace gtl = boost::polygon;
using namespace boost::polygon::operators;


std::string FILEPATH_TCH = "inputs/standard.tch";
std::string FILEPATH_PINOUT = "inputs/rocket64_0808.pinout";

const std::string TIMERTAG_READ_TCH = "Read Technology File";
const std::string TIMERTAG_IMPORT_PARAM = "Import Parameters";
const std::string TIMERTAG_EQCKTCOMPONENT_CAL = "CKT Components Calculations";

int main(int argc, char const *argv[]){

    std::cout << colours::CYAN << "PowerX: A Power Plane Evaluation and Optimization Tool" << colours::COLORRST << std::endl;
    std::cout << std::endl;
    
    TimeProfiler timeProfiler;

    timeProfiler.startTimer(TIMERTAG_READ_TCH);
    Technology technology(FILEPATH_TCH);
    timeProfiler.pauseTimer(TIMERTAG_READ_TCH);

    timeProfiler.startTimer(TIMERTAG_IMPORT_PARAM);
    PowerPlane powerPlane(FILEPATH_PINOUT);
    timeProfiler.pauseTimer(TIMERTAG_IMPORT_PARAM);


    timeProfiler.startTimer(TIMERTAG_EQCKTCOMPONENT_CAL);
    EqCktExtractor EqCktExtor (technology);
    timeProfiler.pauseTimer(TIMERTAG_EQCKTCOMPONENT_CAL);



    std::cout << EqCktExtor.getInterposerResistance() << std::endl; 
    std::cout << EqCktExtor.getInterposerInductance() << std::endl; 
    std::cout << EqCktExtor.getInterposerCapacitanceCenterCell() << std::endl; 
    std::cout << EqCktExtor.getInterposerCapacitanceEdgeCell() << std::endl; 
    std::cout << EqCktExtor.getInterposerCapacitanceCornerCell() << std::endl; 
    std::cout << EqCktExtor.getInterposerConductance() << std::endl; 
    std::cout << EqCktExtor.getInterposerViaResistance() << std::endl; 
    std::cout << EqCktExtor.getInterposerViaInductance() << std::endl; 


    // visualisation part
    std::vector<BumpMap> allChipletBumpMap = powerPlane.uBump.getAllChipletTypes();
    for(const BumpMap &bm : allChipletBumpMap){
        visualiseBumpMap(bm, technology, "outputs/"+bm.getName()+".bm");
    }

    visualisePinout(powerPlane.uBump, technology, "outputs/"+powerPlane.uBump.getName()+".ubump");
    visualiseBallout(powerPlane.c4, technology, "outputs/"+powerPlane.c4.getName()+".c4");

    
    timeProfiler.printTimingReport();
    




}