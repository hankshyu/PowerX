#include <iostream>
#include <string>

#include "colours.hpp"
#include "timeProfiler.hpp"
#include "visualiser.hpp"

#include "eqCktExtractor.hpp"
#include "ballOut.hpp"



// #include "doughnutPolygon.hpp"

std::string FILEPATH_TCH = "inputs/standard.tch";
std::string FILEPATH_PINOUT = "inputs/rocket64_0808.pinout";

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

    BallOut b1 ("inputs/c4.csv");
    for(int j = b1.getBallOutHeight() - 1; j >=0 ; --j){
        for(int i = 0; i < b1.getBallOutWidth(); ++i){
            ballTypeId id = b1.ballOutArray[i][j];
            ballType type = b1.IdToBallTypeMap[id];
            std::cout << type << "(" << int(id) << "), ";
        }
        std::cout << std::endl;
    }

    std::cout << "name to id" << std::endl;

    for(std::unordered_map<ballType, ballTypeId>::const_iterator cit = b1.ballTypeToIdMap.begin(); cit != b1.ballTypeToIdMap.end(); ++cit){
        std::cout << cit->first << " -> " << int(cit->second) << std::endl;
    }

    for(std::unordered_map<ballTypeId, std::unordered_set<Cord>>::const_iterator cit = b1.IdToAllCords.begin(); cit != b1.IdToAllCords.end(); ++cit){
        std::cout << b1.IdToBallTypeMap[cit->first] << " (" << int(cit->first) << ")" << std::endl;
        for(const Cord &c : cit->second){
            std::cout << c << ", ";
        }
        std::cout << std::endl;
    }

    for(ballType bt : b1.getAllBallTypes()){
        std::cout << bt << std::endl;
    }


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