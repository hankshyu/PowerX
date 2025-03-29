#include <iostream>
#include <string>

#include "colours.hpp"
#include "timeProfiler.hpp"
#include "visualiser.hpp"

#include "eqCktExtractor.hpp"
#include "signalType.hpp"
#include "ballOut.hpp"
#include "microBump.hpp"
#include "c4Bump.hpp"



// #include "doughnutPolygon.hpp"

std::string FILEPATH_TCH = "inputs/standard.tch";
std::string FILEPATH_UBUMP = "inputs/rocket64_0808.pinout";

const std::string TIMERTAG_READ_TCH = "Read Technology File";
const std::string TIMERTAG_EQCKTCOMPONENT_CAL = "CKT Components Calculations";
const std::string TIMERTAG_IMPORT_PARAM = "Import Parameters";
const std::string TIMERTAG_MST = "Calculate MST";

void printWelcomeBanner();
void printExitBanner();


int main(int argc, char const *argv[]){

    printWelcomeBanner();
    TimeProfiler timeProfiler;

    timeProfiler.startTimer(TIMERTAG_READ_TCH);
    Technology technology(FILEPATH_TCH);
    timeProfiler.pauseTimer(TIMERTAG_READ_TCH);

    timeProfiler.startTimer(TIMERTAG_EQCKTCOMPONENT_CAL);
    EqCktExtractor EqCktExtor (technology);
    timeProfiler.pauseTimer(TIMERTAG_EQCKTCOMPONENT_CAL);



    MicroBump ub(FILEPATH_UBUMP);
    visualiseMicroBump(ub, technology, "outputs/rocket64_0808.ubump");


    C4Bump c4(FILEPATH_UBUMP);
    visualiseC4Bump(c4, technology, "outputs/rocket64_0808.c4");



    // timeProfiler.startTimer(TIMERTAG_IMPORT_PARAM);
    // AStarBaseline AStarBaseline(FILEPATH_PINOUT);
    // timeProfiler.pauseTimer(TIMERTAG_IMPORT_PARAM);

    // timeProfiler.startTimer(TIMERTAG_MST);
    // AStarBaseline.calculateUBumpMST();
    // timeProfiler.pauseTimer(TIMERTAG_MST);


    // PowerPlane pp(FILEPATH_PINOUT);
    // std::cout << pp.c4.m_ballMap[2][3].representation << std::endl;
    // for(const Cord &c : pp.c4.m_ballMap[2][3].ballouts){
    //     std::cout << c << std::endl;
    // }





    // visualisation part
    // std::vector<BumpMap> allChipletBumpMap = powerPlane.uBump.getAllChipletTypes();
    // for(const BumpMap &bm : allChipletBumpMap){
    //     visualiseBumpMap(bm, technology, "outputs/"+bm.getName()+".bm");
    // }

    // visualisePinout(powerPlane.uBump, technology, "outputs/"+powerPlane.uBump.getName()+".ubump");
    // visualiseBallout(powerPlane.c4, technology, "outputs/"+powerPlane.c4.getName()+".c4");

    // BumpMap c4bm("inputs/c4.csv");
    // visualiseBumpMap(c4bm, technology, "outputs/c4.bm");
    
    
    timeProfiler.printTimingReport();
    printExitBanner();


}

void printWelcomeBanner(){
    std::cout << colours::CYAN << "PowerX: A Power Plane Evaluation and Optimization Tool" << colours::COLORRST << std::endl;

}

void printExitBanner(){
    std::cout << colours::YELLOW << "PowerX Exists Successfully" << colours::COLORRST << std::endl;
    
}