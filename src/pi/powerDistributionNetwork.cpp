//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        05/01/2025 17:36:36
//  Module Name:        powerDistributionNetwork.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A parent class that includes the bump holding data structures
//                      for ubump/C4, 2D vectors holding Metal layers and 2D verctors
//                      holding vias crossing metal layers
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//
//////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:
#include <cassert>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <queue>
// 2. Boost Library:
#include "boost/polygon/polygon.hpp"

// 3. Texo Library:
#include "powerDistributionNetwork.hpp"
#include "cord.hpp"
#include "line.hpp"
#include "signalType.hpp"
#include "objectArray.hpp"
#include "technology.hpp"
#include "eqCktExtractor.hpp"
#include "microBump.hpp"
#include "c4Bump.hpp"

// Initialize the static const unordered_map
const std::unordered_map<SignalType, SignalType> PowerDistributionNetwork::defulatuBumpSigPadMap = {
    { SignalType::POWER_1, SignalType::POWER_1},
    { SignalType::POWER_2, SignalType::POWER_2},
    { SignalType::POWER_3, SignalType::POWER_3},
    { SignalType::POWER_4, SignalType::POWER_4},
    { SignalType::POWER_5, SignalType::POWER_5},
    { SignalType::POWER_6, SignalType::POWER_6},
    { SignalType::POWER_7, SignalType::POWER_7},
    { SignalType::POWER_8, SignalType::POWER_8},
    { SignalType::POWER_9, SignalType::POWER_9},
    { SignalType::POWER_10, SignalType::POWER_10},
};

const std::unordered_map<SignalType, SignalType> PowerDistributionNetwork::defulatc4SigPadMap = {
    { SignalType::POWER_1, SignalType::POWER_1},
    { SignalType::POWER_2, SignalType::POWER_2},
    { SignalType::POWER_3, SignalType::POWER_3},
    { SignalType::POWER_4, SignalType::POWER_4},
    { SignalType::POWER_5, SignalType::POWER_5},
    { SignalType::POWER_6, SignalType::POWER_6},
    { SignalType::POWER_7, SignalType::POWER_7},
    { SignalType::POWER_8, SignalType::POWER_8},
    { SignalType::POWER_9, SignalType::POWER_9},
    { SignalType::POWER_10, SignalType::POWER_10},
    { SignalType::GROUND, SignalType::OBSTACLE},
    { SignalType::SIGNAL, SignalType::SIGNAL},
    { SignalType::OBSTACLE, SignalType::OBSTACLE}
};

PowerDistributionNetwork::PowerDistributionNetwork(const std::string &fileName): uBump(fileName), c4(fileName) {
    
    std::ifstream file(fileName);
    assert(file.is_open());

    std::string lineBuffer;
    
    bool readPDN = false;
    bool readTechnology = false;
    bool finishTechnologyParsing = false;

    while(std::getline(file, lineBuffer)){
        if (lineBuffer.empty()|| lineBuffer[0] == '#') continue; // whole line comment
        
        size_t comment_pos = lineBuffer.find_first_of("#");
        if (comment_pos != std::string::npos) {
                lineBuffer = lineBuffer.substr(0, comment_pos);  // Remove everything after "#"
        }

        // process line: std::cout << lineBuffer << std::endl;
        std::vector<std::string> splitLine;
        std::istringstream stream(lineBuffer);
        std::string wordBuffer;
    
        while (stream >> wordBuffer) { // Extract words by spaces and newlines
            splitLine.push_back(wordBuffer);
        }

        // parse TECHNOLOGY part
        if(!finishTechnologyParsing){
            if(!readTechnology){
                if(lineBuffer == "TECHNOLOGY_BEGIN") readTechnology = true;
                continue;
            }

            if(lineBuffer == "TECHNOLOGY_END"){
                assert(m_gridWidth != -1);
                assert(m_gridHeight != -1);
                assert(m_pinWidth == (m_gridWidth + 1));
                assert(m_pinHeight == (m_gridHeight + 1));
                assert(m_metalLayerCount >= 2);


                m_viaLayerCount = m_metalLayerCount - 1;
                m_ubumpConnectedMetalLayerIdx = 0;
                m_c4ConnectedMetalLayerIdx = m_metalLayerCount - 1;

                this->metalLayers = std::vector<ObjectArray>(m_metalLayerCount, ObjectArray(m_gridWidth, m_gridHeight));
                this->viaLayers = std::vector<ObjectArray>(m_viaLayerCount, ObjectArray(m_pinWidth, m_pinHeight));
                finishTechnologyParsing = true;
                continue;
            }

            
            if(splitLine[0] == "GRID_WIDTH") m_gridWidth = std::stoi(splitLine[2]);
            else if(splitLine[0] == "GRID_HEIGHT") m_gridHeight = std::stoi(splitLine[2]);
            else if(splitLine[0] == "PIN_WIDTH") m_pinWidth = std::stoi(splitLine[2]);
            else if(splitLine[0] == "PIN_HEIGHT") m_pinHeight = std::stoi(splitLine[2]);
            else if(splitLine[0] == "LAYERS") m_metalLayerCount = std::stoi(splitLine[2]);
            else{
                std::cout << "[PowerX:PDNParser] Error: Unrecognizted Technology details: " << lineBuffer << std::endl;
                exit(4);
            }
            continue;
        }

        // parse PDN_PREPLACE_START ... PDN_PREPLACE_END part (preplaced part)
        if(splitLine[0] == "PDN_PREPLACE_START"){
            readPDN = true;
            continue;
        }
        if(!readPDN) continue;
        
        if(splitLine[0] == "PDN_PREPLACE_END"){
            break;
        }

        if(splitLine[0] == "METAL_LAYER"){
            int targetLayer = std::stoi(splitLine[1]);
            if(targetLayer >= this->m_metalLayerCount){
                std::cout << "[PowerX:PDNParser] Error: PDN metal preplace layer idx: " << targetLayer << "should be less than total layers: " << this->m_metalLayerCount << std::endl;
                exit(4);
            }
            splitLine[2].erase(std::remove(splitLine[2].begin(), splitLine[2].end(), '"'), splitLine[2].end());
            if(!splitLine[2].empty()) metalLayers[targetLayer].readBlockages(splitLine[2]);

        }else if(splitLine[0] == "VIA_LAYER"){
            int targetLayer = std::stoi(splitLine[1]);
            if(targetLayer >= this->m_viaLayerCount){
                std::cout << "[PowerX:PDNParser] Error: PDN via preplace layer idx: " << targetLayer << "should be less than total layers: " << this->m_viaLayerCount << std::endl;
                exit(4);  
            }
            splitLine[2].erase(std::remove(splitLine[2].begin(), splitLine[2].end(), '"'), splitLine[2].end());
            if(!splitLine[2].empty()) viaLayers[targetLayer].readBlockages(splitLine[2]);

        }else{
            std::cout << "[PowerX:PDNParser] Error: Unrecognizted label in PDN preplace area: " << lineBuffer << std::endl;
            exit(4);
        }
    }

    file.close();

    assert(readTechnology);
    assert(finishTechnologyParsing);
}

PowerDistributionNetwork::~PowerDistributionNetwork(){
    
    for(PDNEdge *eg : pdnEdgeOwner){
        delete(eg);
    }

    for(size_t layer = 0; layer < physicalGridLayer; ++layer){
        for(size_t y = 0; y < physicalGridHeight; ++y){
            for(size_t x = 0; x < physicalGridHeight; ++x){
                delete(physicalNodes[layer][y][x]);
            }
        }
    }

}

bool PowerDistributionNetwork::checkOnePiece(int metalLayerIdx){
    std::unordered_map<SignalType, DoughnutPolygonSet> dpsMap = collectDoughnutPolygons(metalLayers[metalLayerIdx].canvas);
    for(std::unordered_map<SignalType, DoughnutPolygonSet>::const_iterator cit = dpsMap.begin(); cit != dpsMap.end(); ++cit){
        SignalType st = cit->first;
        if(POWER_SIGNAL_SET.count(st) == 0) continue;
        if(dps::getShapesCount(cit->second) > 1) return false;
    }

    return true;
}

void PowerDistributionNetwork::checkVias(){

    for(int layer = 0; layer < m_viaLayerCount; ++layer){
        int upLayer = layer;
        int downLayer = layer + 1;
        for(int j = 1; j < m_pinHeight-1; ++j){
            for(int i = 1; i < m_pinHeight-1; ++i){
                switch (viaLayers[layer].canvas[j][i]){
                    case SignalType::OBSTACLE:{
                        continue;
                        break;
                    }
                    case SignalType::SIGNAL:{

                            if(metalLayers[upLayer].canvas[j-1][i-1] != SignalType::SIGNAL){
                                printf("[Error CheckVias] Signal(layer, j, i) = (%d, %d, %d) has nonSignal Up LL: %s\n", layer, j, i, to_string(metalLayers[upLayer].canvas[j-1][i-1]));
                            }
                            if(metalLayers[upLayer].canvas[j-1][i] != SignalType::SIGNAL){
                                printf("[Error CheckVias] Signal(layer, j, i) = (%d, %d, %d) has nonSignal Up LR: %s\n", layer, j, i, to_string(metalLayers[upLayer].canvas[j-1][i]));
                            }
                            if(metalLayers[upLayer].canvas[j][i-1] != SignalType::SIGNAL){
                                printf("[Error CheckVias] Signal(layer, j, i) = (%d, %d, %d) has nonSignal Up UL: %s\n", layer, j, i, to_string(metalLayers[upLayer].canvas[j][i-1]));
                            }
                            if(metalLayers[upLayer].canvas[j][i] != SignalType::SIGNAL){
                                printf("[Error CheckVias] Signal(layer, j, i) = (%d, %d, %d) has nonSignal Up UR: %s\n", layer, j, i, to_string(metalLayers[upLayer].canvas[j][i]));
                            }


                            if(metalLayers[downLayer].canvas[j-1][i-1] != SignalType::SIGNAL){
                                printf("[Error CheckVias] Signal(layer, j, i) = (%d, %d, %d) has nonSignal Down LL: %s\n", layer, j, i, to_string(metalLayers[downLayer].canvas[j-1][i-1]));
                            }
                            if(metalLayers[downLayer].canvas[j-1][i] != SignalType::SIGNAL){
                                printf("[Error CheckVias] Signal(layer, j, i) = (%d, %d, %d) has nonSignal Down LR: %s\n", layer, j, i, to_string(metalLayers[downLayer].canvas[j-1][i]));
                            }
                            if(metalLayers[downLayer].canvas[j][i-1] != SignalType::SIGNAL){
                                printf("[Error CheckVias] Signal(layer, j, i) = (%d, %d, %d) has nonSignal Down UL: %s\n", layer, j, i, to_string(metalLayers[downLayer].canvas[j][i-1]));
                            }
                            if(metalLayers[downLayer].canvas[j][i] != SignalType::SIGNAL){
                                printf("[Error CheckVias] Signal(layer, j, i) = (%d, %d, %d) has nonSignal Down UR: %s\n", layer, j, i, to_string(metalLayers[downLayer].canvas[j][i]));
                            }
                            break;
                    }
                    case SignalType::EMPTY:{

                        bool upLLClear = (metalLayers[upLayer].canvas[j-1][i-1] != SignalType::SIGNAL) && (metalLayers[upLayer].canvas[j-1][i-1] != SignalType::OBSTACLE);
                        bool upLRClear = (metalLayers[upLayer].canvas[j-1][i] != SignalType::SIGNAL) && (metalLayers[upLayer].canvas[j-1][i] != SignalType::OBSTACLE);
                        bool upULClear = (metalLayers[upLayer].canvas[j][i-1] != SignalType::SIGNAL) && (metalLayers[upLayer].canvas[j][i-1] != SignalType::OBSTACLE);
                        bool upURClear = (metalLayers[upLayer].canvas[j][i] != SignalType::SIGNAL) && (metalLayers[upLayer].canvas[j][i] != SignalType::OBSTACLE);

                        bool downLLClear = (metalLayers[downLayer].canvas[j-1][i-1] != SignalType::SIGNAL) && (metalLayers[downLayer].canvas[j-1][i-1] != SignalType::OBSTACLE);
                        bool downLRClear = (metalLayers[downLayer].canvas[j-1][i] != SignalType::SIGNAL) && (metalLayers[downLayer].canvas[j-1][i] != SignalType::OBSTACLE);
                        bool downULClear = (metalLayers[downLayer].canvas[j][i-1] != SignalType::SIGNAL) && (metalLayers[downLayer].canvas[j][i-1] != SignalType::OBSTACLE);
                        bool downURClear = (metalLayers[downLayer].canvas[j][i] != SignalType::SIGNAL) && (metalLayers[downLayer].canvas[j][i] != SignalType::OBSTACLE);

                        if(upLLClear && upLRClear && upULClear && upURClear && downLLClear && downLRClear && downULClear && downURClear) continue;
                        else{
                            if(!upLLClear) printf("[Error CheckVias] Empty(layer, j, i) = (%d, %d, %d) has signal: %s\n", layer, j, i, to_string(metalLayers[upLayer].canvas[j-1][i-1]));
                            if(!upLRClear) printf("[Error CheckVias] Empty(layer, j, i) = (%d, %d, %d) has signal: %s\n", layer, j, i, to_string(metalLayers[upLayer].canvas[j-1][i]));
                            if(!upULClear) printf("[Error CheckVias] Empty(layer, j, i) = (%d, %d, %d) has signal: %s\n", layer, j, i, to_string(metalLayers[upLayer].canvas[j][i-1]));
                            if(!upURClear) printf("[Error CheckVias] Empty(layer, j, i) = (%d, %d, %d) has signal: %s\n", layer, j, i, to_string(metalLayers[upLayer].canvas[j][i]));
                            
                            if(!downLLClear) printf("[Error CheckVias] Empty(layer, j, i) = (%d, %d, %d) has signal: %s\n", layer, j, i, to_string(metalLayers[downLayer].canvas[j-1][i-1]));
                            if(!downLLClear) printf("[Error CheckVias] Empty(layer, j, i) = (%d, %d, %d) has signal: %s\n", layer, j, i, to_string(metalLayers[downLayer].canvas[j-1][i]));
                            if(!downLLClear) printf("[Error CheckVias] Empty(layer, j, i) = (%d, %d, %d) has signal: %s\n", layer, j, i, to_string(metalLayers[downLayer].canvas[j][i-1]));
                            if(!downLLClear) printf("[Error CheckVias] Empty(layer, j, i) = (%d, %d, %d) has signal: %s\n", layer, j, i, to_string(metalLayers[downLayer].canvas[j][i]));
                        }
                        break;
                    }
                    default:{
                        printf("[Error CheckVias] (layer, j, i) = (%d, %d, %d) has signal: %s\n", layer, j, i, to_string(viaLayers[layer].canvas[j][i]));
                        break;
                    }
                }


            }
        }

    }
}

void PowerDistributionNetwork::fillEnclosedRegionsonCanvas(){
    std::vector<std::vector<bool>> visited;
    const std::vector<Cord> directions = {Cord(-1, 0), Cord(1, 0), Cord(0, -1), Cord(0, 1)};

    auto inBounds = [&](int x, int y) {
        return y >= 0 && y < m_gridHeight && x >= 0 && x < m_gridWidth;
    };

    for (int layer = 0; layer < m_metalLayerCount ; ++layer) {
        visited.assign(m_gridHeight, std::vector<bool>(m_gridWidth, false));
        
        for (int y = 0; y < m_gridHeight; ++y) {
            for (int x = 0; x < m_gridWidth; ++x) {
                if (metalLayers[layer].canvas[y][x] != SignalType::EMPTY || visited[y][x]) continue;

                std::queue<Cord> q;
                std::vector<Cord> region;
                std::unordered_set<SignalType> borderSignals;
                bool touchesBoundary = false;

                q.push(Cord(x, y));
                visited[y][x] = true;
                region.emplace_back(x, y);

                while (!q.empty()) {
                    Cord c = q.front(); q.pop();

                    for (const Cord& d : directions) {
                        int ny = c.y() + d.y();
                        int nx = c.x() + d.x();

                        if (!inBounds(nx, ny)) {
                            continue;
                        }

                        SignalType neighborType = metalLayers[layer].canvas[ny][nx];

                        if (neighborType == SignalType::EMPTY && !visited[ny][nx]) {
                            visited[ny][nx] = true;
                            q.push(Cord(nx, ny));
                            region.emplace_back(nx, ny);
                        } else if (neighborType != SignalType::EMPTY && neighborType != SignalType::OBSTACLE) {
                            borderSignals.insert(neighborType);
                        }
                    }
                }

                if (borderSignals.size() == 1) {
                    SignalType fillType = *borderSignals.begin();
                    for (const Cord& p : region) {
                        metalLayers[layer].canvas[p.y()][p.x()] = fillType;
                    }
                }
            }
        }
    }
}

void PowerDistributionNetwork::assignVias(){
    for(int vLayer = 0; vLayer < m_viaLayerCount; ++vLayer){
        int upMetalLayerIndex = vLayer;
        int downMetalLayerIndex = vLayer + 1;
        // leave the surronding blank
        for(int j = 1; j < (m_pinHeight-1); ++j){
            for(int i = 1; i < (m_pinWidth-1); ++i){
                if(viaLayers[vLayer].canvas[j][i] != SignalType::EMPTY) continue;

                SignalType upLLst = metalLayers[upMetalLayerIndex].canvas[j-1][i-1];
                SignalType upLRst = metalLayers[upMetalLayerIndex].canvas[j-1][i];
                SignalType upULst = metalLayers[upMetalLayerIndex].canvas[j][i-1];
                SignalType upURst = metalLayers[upMetalLayerIndex].canvas[j][i];
                if((upLLst != upLRst) || (upLRst != upULst) || (upULst != upURst)) continue;

                SignalType downLLst = metalLayers[downMetalLayerIndex].canvas[j-1][i-1];
                SignalType downLRst = metalLayers[downMetalLayerIndex].canvas[j-1][i];
                SignalType downULst = metalLayers[downMetalLayerIndex].canvas[j][i-1];
                SignalType downURst = metalLayers[downMetalLayerIndex].canvas[j][i];
                if((upLLst != downLLst) || (downLLst != downLRst) || (downLRst != downULst) || (downULst != downURst)) continue;

                viaLayers[vLayer].setCanvas(j, i, upLLst);
            }
        }
    }
}

void PowerDistributionNetwork::removeFloatingPlanes(int layer){
    
    assert(layer >= m_ubumpConnectedMetalLayerIdx);
    assert(layer <= m_c4ConnectedMetalLayerIdx);


    std::unordered_map<SignalType, std::unordered_set<Cord>> requiredCords;
    
    // insert the preplaced
    for(auto cit = metalLayers[layer].preplacedCords.begin(); cit != metalLayers[layer].preplacedCords.end(); ++cit){
        requiredCords[cit->first] = std::unordered_set<Cord>(cit->second.begin(), cit->second.end());
    }
    // check the vias from up/down layers
    const std::vector<std::vector<SignalType>> &upperPinCanvas = (layer == m_ubumpConnectedMetalLayerIdx)? uBump.canvas : viaLayers[layer-1].canvas;
    const std::vector<std::vector<SignalType>> &lowerPinCanvas = (layer == m_c4ConnectedMetalLayerIdx)? c4.canvas : viaLayers[layer].canvas;
    Rectangle pinCanvasSizeRectangle(0, 0, m_pinWidth-1, m_pinHeight-1);
    for(int i = 0; i < m_pinHeight; ++i){
        for(int j = 0; j < m_pinWidth; ++j){
            SignalType ust = upperPinCanvas[j][i];
            if((ust != SignalType::EMPTY) && (ust != SignalType::OBSTACLE)){
                Cord ll(i-1, j-1);
                Cord lr(i, j-1);
                Cord ul(i-1, j);
                Cord ur(i, j);
                if(boost::polygon::contains(pinCanvasSizeRectangle, ll, true)){
                    requiredCords[ust].insert(ll);
                }
                if(boost::polygon::contains(pinCanvasSizeRectangle, lr, true)){
                    requiredCords[ust].insert(lr);
                }
                if(boost::polygon::contains(pinCanvasSizeRectangle, ul, true)){
                    requiredCords[ust].insert(ul);
                }
                if(boost::polygon::contains(pinCanvasSizeRectangle, ur, true)){
                    requiredCords[ust].insert(ur);
                }
            }

            SignalType dst = lowerPinCanvas[j][i];
            if((dst != SignalType::EMPTY) && (dst != SignalType::OBSTACLE)){
                Cord ll(i-1, j-1);
                Cord lr(i, j-1);
                Cord ul(i-1, j);
                Cord ur(i, j);
                if(boost::polygon::contains(pinCanvasSizeRectangle, ll, true)){
                    requiredCords[dst].insert(ll);
                }
                if(boost::polygon::contains(pinCanvasSizeRectangle, lr, true)){
                    requiredCords[dst].insert(lr);
                }
                if(boost::polygon::contains(pinCanvasSizeRectangle, ul, true)){
                    requiredCords[dst].insert(ul);
                }
                if(boost::polygon::contains(pinCanvasSizeRectangle, ur, true)){
                    requiredCords[dst].insert(ur);
                }
            } 
        }
    }

    std::unordered_map<SignalType, DoughnutPolygonSet> layerDPS = collectDoughnutPolygons(metalLayers[layer].canvas);
    for(std::unordered_map<SignalType, DoughnutPolygonSet>::iterator it = layerDPS.begin(); it != layerDPS.end(); ++it){
        SignalType st = it->first;
        if((st == SignalType::EMPTY) || (st == SignalType::OBSTACLE)) continue;


        for(int dpidx = 0; dpidx < dps::getShapesCount(it->second); ++dpidx){
            std::vector<Cord>fragmentGrids = dp::getContainedGrids(it->second.at(dpidx));
            if(requiredCords.count(st) == 0){
                for(const Cord &c : fragmentGrids) metalLayers[layer].setCanvas(c, SignalType::EMPTY);
            }else{
                bool isRequired = false;
                for(const Cord &c : fragmentGrids){
                    if(requiredCords[st].count(c) != 0){
                        isRequired = true;
                        break;
                    }
                }
                if(!isRequired){
                    for(const Cord &c : fragmentGrids) metalLayers[layer].setCanvas(c, SignalType::EMPTY);
                }
            }
        }
    }
    
}

void PowerDistributionNetwork::exportEquivalentCircuit(const SignalType st, const Technology &tch, const EqCktExtractor &extor, const std::string &filePath){
 
    std::ofstream ofs(filePath, std::ios::out);
    assert(ofs.is_open());

    auto pointToNode = [](int layer, const Cord &c) -> std::string {
        return "n" + std::to_string(c.x()) + "_" + std::to_string(c.y()) + "_" + std::to_string(layer);
    };

    ofs << "****** Equivalent Circuit Components ******" << std::endl;
    ofs << ".SUBCKT ubump in out" << std::endl;
    ofs << "R1 in mid " << tch.getMicrobumpResistance() << "m" << std::endl;
    ofs << "L1 mid out " << tch.getMicrobumpInductance() << "p" << std::endl;
    ofs << ".ENDS" << std::endl;
    
    ofs << std::endl;

    ofs << ".SUBCKT via in out" << std::endl;
    ofs << "R1 in mid " << extor.getInterposerViaResistance() << "m" << std::endl;
    ofs << "L1 mid out " << extor.getInterposerViaInductance() << "p" << std::endl;
    ofs << ".ENDS" << std::endl;

    ofs << std::endl;

    ofs << ".SUBCKT edge in out" << std::endl;
    ofs << "R1 in mid " << 2*extor.getInterposerResistance() << "m" << std::endl;
    ofs << "L1 mid out " << 2*extor.getInterposerInductance() << "p" << std::endl;
    ofs << ".ENDS" << std::endl;

    ofs << std::endl;

    ofs << ".SUBCKT tsv in out" << std::endl;
    ofs << "R1 in mid " << tch.getTsvResistance() << "m" << std::endl;
    ofs << "L1 mid out " << tch.getTsvInductance() << "p" << std::endl;
    ofs << ".ENDS" << std::endl;

    ofs << std::endl;

    ofs << ".SUBCKT cfour in out" << std::endl;
    ofs << "R1 in mid " << tch.getC4Resistance() << "m" << std::endl;
    ofs << "L1 mid out " << tch.getC4Inductance() << "p" << std::endl;
    ofs << ".ENDS" << std::endl;
    
    ofs << std::endl;

    ofs << "****** Equivalent Circuit ******" << std::endl;
    int chipletcount = uBump.signalTypeToInstances[st].size();
    assert(chipletcount > 0);
    std::vector<std::string> chipletTitles(uBump.signalTypeToInstances[st].begin(), uBump.signalTypeToInstances[st].end());

    int uBumpCounter = 0;
    int viaCounter = 0;
    int edgeCounter = 0;
    int tsvCounter = 0;
    int c4Counter = 0;
    int capCounter = 0;

    ofs << ".SUBCKT eqckt in ";
    for(int i = 0; i < chipletcount; ++i){
        ofs << chipletTitles[i] << "_o ";
    }

    ofs << "vss" << std::endl;
    ofs << "*** MicroBumps ***" << std::endl;
    for(int ubidx = 0; ubidx < chipletcount; ++ubidx){
        std::string instanceName = chipletTitles[ubidx];
        Cord llbase(rec::getLL(uBump.instanceToRectangleMap.at(instanceName)));
        for(const Cord &c : uBump.instanceToBallOutMap[instanceName]->SignalTypeToAllCords[st]){
            ofs << "Xubump" << uBumpCounter++ << "_" <<  pointToNode(m_ubumpConnectedMetalLayerIdx, Cord(c.x() + llbase.x(), c.y() + llbase.y())) << " ";
            ofs << chipletTitles[ubidx] << "_o " << pointToNode(m_ubumpConnectedMetalLayerIdx, Cord(c.x() + llbase.x(), c.y() + llbase.y())) << " ubump" << std::endl;
        }
    }

    ofs << "*** C4 Bumps ***" << std::endl;
    for(const C4PinCluster *cluster : c4.signalTypeToAllClusters[st]){
        std::string c4Node = pointToNode(m_metalLayerCount, cluster->representation);
        ofs << "Xcfour" << c4Counter++ << " in " << c4Node << " cfour" << std::endl;
        for(const Cord &c : cluster->pins){
            ofs << "Xtsv" << tsvCounter++ << "_" << pointToNode(m_c4ConnectedMetalLayerIdx, c) << " ";
            ofs << c4Node << " " << pointToNode(m_c4ConnectedMetalLayerIdx, c) << " tsv" << std::endl;
        }
    }

    ofs << "*** Metal Layers ***" << std::endl;
    len_t pinXMin = 0;
    len_t pinXMax = m_pinWidth-1;
    len_t pinYMin = 0;
    len_t pinYMax = m_pinHeight-1;
    // Cord borderLL(pinXMin, pinYMin);
    // Cord borderLR(pinXMax, pinYMin);
    // Cord borderUL(pinXMin, pinYMax);
    // Cord borderUR(pinXMax, pinYMax);
    for(int mLayerIdx = m_ubumpConnectedMetalLayerIdx; mLayerIdx <= m_c4ConnectedMetalLayerIdx; ++mLayerIdx){
        std::unordered_set<Line> collectedLines;
        std::unordered_set<Cord> collectedCords;
        std::unordered_map<SignalType, DoughnutPolygonSet> stDPS = collectDoughnutPolygons(metalLayers[mLayerIdx].canvas);
        for(int dpidx = 0; dpidx < dps::getShapesCount(stDPS[st]); ++dpidx){
            DoughnutPolygon fragment = stDPS[st][dpidx];
            std::vector<Cord> fragmentLLs = dp::getContainedGrids(fragment);
            for(const Cord &llCord : fragmentLLs){
                Cord ll(llCord.x(), llCord.y());
                Cord lr(llCord.x() + 1, llCord.y());
                Cord ul(llCord.x(), llCord.y() + 1);
                Cord ur(llCord.x() + 1, llCord.y() + 1);
                
                Line lup(ul, ur);
                Line ldown(ll, lr);
                Line lleft(ll, ul);
                Line lright(lr, ur);

                std::vector<Cord> candCords = {ll, lr, ul, ur};
                std::vector<Line> candLines = {lup, ldown, lleft, lright};

                // capacitors
                for(const Cord &candc : candCords){
                    if(collectedCords.count(candc) != 0) continue;
                    collectedCords.insert(candc);

                    bool xOnEdge = (candc.x() == pinXMin) || (candc.x() == pinXMax);
                    bool yOnEdge = (candc.y() == pinYMin) || (candc.y() == pinYMax);

                    if(xOnEdge && yOnEdge){
                        ofs << "C" << capCounter++ << " " << pointToNode(mLayerIdx, candc) << " vss " << extor.getInterposerCapacitanceCornerCell()/4.0 << "f" << std::endl;
                    }else if(xOnEdge || yOnEdge){
                        ofs << "C" << capCounter++ << " " << pointToNode(mLayerIdx, candc) << " vss " << extor.getInterposerCapacitanceEdgeCell()/4.0 << "f" << std::endl;
                    }else{
                        ofs << "C" << capCounter++ << " " << pointToNode(mLayerIdx, candc) << " vss " << extor.getInterposerCapacitanceCenterCell()/4.0 << "f" << std::endl;
                    }
                }

                // Edges

                for(const Line &candl : candLines){
                    if(collectedLines.count(candl) != 0) continue;
                    collectedLines.insert(candl);
                    ofs << "Xedge" << edgeCounter++ << "_" << pointToNode(mLayerIdx, candl.getLow()) << "_" << pointToNode(mLayerIdx, candl.getHigh()) << " ";
                    ofs << pointToNode(mLayerIdx, candl.getLow()) << " " << pointToNode(mLayerIdx, candl.getHigh()) << " edge" << std::endl;
                }
            }
        }
    }

    ofs << "*** Vias ***" << std::endl;
    for(int viaLayerIdx = 0; viaLayerIdx < m_viaLayerCount; ++ viaLayerIdx){
        for(int j = 0; j < m_pinHeight; ++j){
            for(int i = 0; i < m_pinWidth; ++i){
                if(viaLayers[viaLayerIdx].canvas[j][i] == st){
                    ofs << "Xvia" << viaCounter++ << "_" << pointToNode(viaLayerIdx, Cord(i, j)) << "_" << pointToNode(viaLayerIdx+1, Cord(i, j)) << " ";
                    ofs << pointToNode(viaLayerIdx, Cord(i, j)) << " " << pointToNode(viaLayerIdx+1, Cord(i, j)) << " via" << std::endl;
                }
            }
        }
    }
    // ofs << "Xubump1 in R8_o ubump" << std::endl;
    // ofs << "Xubump2 in R7_o ubump" << std::endl;
    // ofs << "Xubump3 in R6_o ubump" << std::endl;
    // ofs << "Xubump4 in R5_o ubump" << std::endl;
    // ofs << "Xubump5 in R4_o ubump" << std::endl;
    // ofs << "Xubump6 in R3_o ubump" << std::endl;
    // ofs << "Xubump7 in R2_o ubump" << std::endl;
    // ofs << "Xubump8 in R1_o ubump" << std::endl;
    ofs << ".ENDS" << std::endl;
    ofs << std::endl;

    ofs << "****** Chiplet Load Model ******" << std::endl;
    ofs << ".SUBCKT chiplet in vss Rval=50m Lval=200n Cval=30p Iload=1.0" << std::endl;
    ofs << "Rpath in n1 Rval" << std::endl;
    ofs << "Lpath n1 n2 Lval" << std::endl;
    ofs << "Cpath n2 vss Cval" << std::endl;
    ofs << "ILOAD n2 vss DC Iload" << std::endl;
    ofs << ".ENDS" << std::endl;

    ofs << std::endl;

    ofs << "****** Input PCB model Model ******" << std::endl;
    ofs << ".SUBCKT pcb vrm_low vrm_high out vss" << std::endl;
    ofs << "Lpcbh vrm_high n1 " << tch.getPCBInductance() << "p" << std::endl;
    ofs << "Rpcbh n1 out " << tch.getPCBResistance() << "u" << std::endl;
    ofs << "Ldecaph out n2 " << tch.getPCBDecapInductance() <<  "n" << std::endl;
    ofs << "Cdecap n2 n3 " << tch.getPCBDecapCapacitance() << "u" << std::endl;
    ofs << "Rdecapl n3 vss " << tch.getPCBDecapResistance() << "u" << std::endl;
    ofs << "Rpcbl n4 vss " << tch.getPCBResistance() << "u" << std::endl;
    ofs << "Lpcbl vrm_low n4 " << tch.getPCBInductance() << "p" << std::endl;
    
    ofs << "* --- Decap Bank for Broadband Z(f) Control ---" << std::endl;
    ofs << "* 1 μF decap (low freq bulk, moderately damped)" << std::endl;
    ofs << "Rdecap1u out n1u 0.05" << std::endl;
    ofs << "Cdecap1u n1u vss 1u" << std::endl;
    ofs << "* 100 nF decap (mid-low freq)" << std::endl;
    ofs << "Rdecap100n out n100n 0.1" << std::endl;
    ofs << "Cdecap100n n100n vss 100n" << std::endl;
    ofs << "* 10 nF decap (mid freq)" << std::endl;
    ofs << "Rdecap10n out n10n 0.2" << std::endl;
    ofs << "Cdecap10n n10n vss 10n" << std::endl;
    ofs << "* 1 nF decap (high freq)" << std::endl;
    ofs << "Rdecap1n out n1n 0.3" << std::endl;
    ofs << "Cdecap1n n1n vss 1n" << std::endl;
    ofs << "* 100 pF decap (GHz damping)" << std::endl;
    ofs << "Rdecap100p out n100p 0.5" << std::endl;
    ofs << "Cdecap100p n100p vss 100p" << std::endl;
    
    ofs << ".ENDS" << std::endl;

    ofs << std::endl << std::endl;
    ofs << "****** Main ******" << std::endl;
    ofs << ".param VDD=1.0" << std::endl;
    ofs << "VVRM vrm_high vrm_low DC VDD" << std::endl;
    ofs << "Xpcb vrm_low vrm_high pcb_out 0 pcb" << std::endl;
    
    ofs << "Xeqckt pcb_out ";
    for(int i = 0; i < chipletcount; ++i){
        ofs << chipletTitles[i] << "_o ";
    }
    ofs << "0" << " " << "eqckt" << std::endl;
    
    for(int i = 0; i < chipletcount; ++i){
        ofs << "Xchiplet" << i << " " << chipletTitles[i] << "_o " << "0 chiplet ";
        BallOut *bt = uBump.instanceToBallOutMap.at(chipletTitles[i]);
        assert(bt != nullptr);
        ofs << "Rval=" << bt->getSeriesResistance() << "m Lval=" << bt->getSeriesInductance() << "n Cval=" << bt->getShuntCapacitance() << "p ";
        ofs << "Iload=" << bt->getMaxCurrent() << std::endl;
    }

    ofs << std::endl;
    ofs << "****** DC IR-Drop test ******" << std::endl;
    ofs << ".OPTION POST=2 INGOLD=2 RUNLVL=6" << std::endl;
    ofs << std::endl << ".op" << std::endl;
    ofs << ".DC VDD 1.5 1.5 0.5" << std::endl;
    
    ofs << ".print V(vrm_low) V(vrm_high) V(pcb_out) ";
    for(int i = 0; i < chipletcount; ++i){
        ofs << "V(" << chipletTitles[i] << "_o" << ") ";
    }
    ofs << std::endl;

    ofs << ".measure DC irdroppcb param=\'V(vrm_high) - V(pcb_out)\'" << std::endl;
    for(int i = 0; i < chipletcount; ++i){
        Rectangle chpletPlacement = uBump.instanceToRectangleMap[chipletTitles[i]];
        Cord chipletLL = rec::getLL(chpletPlacement);

        ofs << ".measure DC IRDrop" << chipletTitles[i] << "_" <<  chipletLL.x() << "_" << chipletLL.y() << "_" << rec::getWidth(chpletPlacement) << "_" << rec::getHeight(chpletPlacement);
        ofs << " param=\'V(pcb_out) - V(" << chipletTitles[i] << "_o" << ")\'" << std::endl;
    }
    ofs << std::endl;

    // ofs << ".OPTION RELTOL=1e-4 ABSTOL=1e-8 VNTOL=1e-6" << std::endl;
    ofs << ".end" << std::endl;
    ofs.close();
}

void PowerDistributionNetwork::buildPhysicalImplementation(){
    
    physicalGridLayer = m_metalLayerCount;
    physicalGridWidth = m_gridWidth + 1;
    physicalGridHeight = m_gridHeight + 1;

    auto inPhysicalGrid = [&](size_t y, size_t x){
        return (y >= 0) && (y < physicalGridHeight) && (x >= 0) && (x < physicalGridWidth);
    };

    physicalNodes.assign(physicalGridLayer, std::vector<std::vector<PDNNode*>>(physicalGridHeight, std::vector<PDNNode*>(physicalGridWidth, nullptr)));

    auto inCanvas = [&](int cy, int cx) -> bool {
        return (0 <= cy && cy < static_cast<int>(m_gridHeight) && 0 <= cx && cx < static_cast<int>(m_gridWidth));
    };

    for (size_t layer = 0; layer < m_metalLayerCount; ++layer) {
        for (size_t y = 0; y < physicalGridHeight; ++y) {
            for (size_t x = 0; x < physicalGridWidth; ++x) {

                PDNNode* newNode = new PDNNode(layer, x, y);

                SignalType curSig = SignalType::EMPTY;
                bool mixSignal = false;
                bool isObstacle = false;

                // examine up to four adjacent canvas cells:
                auto checkCell = [&](int cy, int cx) {
                    if (!inCanvas(cy, cx)) return;  // skip out-of-range cells
                    SignalType gridSig = this->metalLayers[layer].canvas[cy][cx];

                    if (gridSig == SignalType::SIGNAL || gridSig == SignalType::OBSTACLE || gridSig == SignalType::GROUND) {
                        isObstacle = true;
                    }

                    if (POWER_SIGNAL_SET.count(gridSig) != 0) {
                        if (curSig == SignalType::EMPTY || curSig == gridSig) {
                            curSig = gridSig;
                        } else {
                            curSig = SignalType::EMPTY;
                            mixSignal = true;
                        }
                    }
                };

                // Corners of the node within the canvas cell grid
                checkCell(static_cast<int>(y) - 1, static_cast<int>(x) - 1);
                checkCell(static_cast<int>(y) - 1, static_cast<int>(x));
                checkCell(static_cast<int>(y),     static_cast<int>(x) - 1);
                checkCell(static_cast<int>(y),     static_cast<int>(x));

                if (isObstacle) {
                    newNode->signal = SignalType::OBSTACLE;   // ensure enum name matches
                } else if (!mixSignal) {
                    newNode->signal = curSig;
                } else {
                    newNode->signal = SignalType::EMPTY;
                }

                physicalNodes[layer][y][x] = newNode;
            }
        }
    }

    // initialise the edges of the metal layers
    size_t physicalGridWidthRightBorder = physicalGridWidth - 1;
    size_t physicalGridHeightUpBorder = physicalGridHeight - 1;
    for (size_t layer = 0; layer < m_metalLayerCount; ++layer){
        for (size_t y = 0; y < physicalGridHeight; ++y){
            for (size_t x = 0; x < physicalGridWidth; ++x){
                PDNNode *centreNode = physicalNodes[layer][y][x];
                SignalType centreNodeSignal = centreNode->signal;
                if(centreNodeSignal == SignalType::OBSTACLE || centreNodeSignal == SignalType::SIGNAL) continue;

                if(x != physicalGridWidthRightBorder){
                    PDNNode *rightNode = physicalNodes[layer][y][x+1];
                    SignalType rightNodeSig = rightNode->signal;
                    if(rightNodeSig != SignalType::OBSTACLE && rightNodeSig != SignalType::SIGNAL){
                        PDNEdge *newEdge = new PDNEdge(centreNode, rightNode);
                        newEdge->isVia = false;
                        centreNode->east = newEdge;
                        rightNode->west = newEdge;
                        this->pdnEdgeOwner.push_back(newEdge);
                    }
                }

                if(y != physicalGridHeightUpBorder){
                    PDNNode *upNode = physicalNodes[layer][y+1][x];
                    SignalType upNodeSig = upNode->signal;
                    if(upNodeSig != SignalType::OBSTACLE && upNodeSig != SignalType::SIGNAL){
                        PDNEdge *newEdge = new PDNEdge(centreNode, upNode);
                        newEdge->isVia = false;
                        centreNode->north = newEdge;
                        upNode->south = newEdge;
                        this->pdnEdgeOwner.push_back(newEdge);
                    }
                }
            }
        }
    }

    for(size_t viaLayer = 0; viaLayer < m_viaLayerCount; ++viaLayer){
        for(size_t y = 0; y < m_pinHeight; ++y){
            for(size_t x = 0; x < m_pinWidth; ++x){
                SignalType curSig = viaLayers[viaLayer].canvas[y][x];
                if(POWER_SIGNAL_SET.count(curSig) == 0 && curSig != SignalType::EMPTY) continue;
                
                PDNNode *downNode = this->physicalNodes[viaLayer+1][y][x];
                PDNNode *upNode = this->physicalNodes[viaLayer][y][x];

                PDNEdge *newEdge = new PDNEdge(downNode, upNode);
                newEdge->signal = curSig;
                if(curSig != SignalType::EMPTY){
                    upNode->signal = curSig;    
                    downNode->signal = curSig;
                }
                newEdge->isVia = true;
                downNode->up = newEdge;
                upNode->down = newEdge;
                this->pdnEdgeOwner.push_back(newEdge);
            }
        }
    }

    // grow the first time of the edges
    growPDNNodeEdges();

    for(const auto&[st, nameSet] : uBump.signalTypeToInstances){
        if(POWER_SIGNAL_SET.count(st) == 0) continue;
        phySOI.push_back(st);
        for(const std::string &name : nameSet){
            phyChipletNames[st].push_back(name);
            Cord chipletLL = rec::getLL(uBump.instanceToRectangleMap[name]);
            for(const Cord &c : uBump.instanceToBallOutMap[name]->SignalTypeToAllCords[st]){
                len_t mx = chipletLL.x() + c.x();
                len_t my = chipletLL.y() + c.y();
                phyChipletNodes[name].push_back(physicalNodes[0][my][mx]);
                assert(physicalNodes[0][my][mx]->signal == st);
            }
        }
    }

    for(SignalType st : phySOI){
        for(const Cord &c : c4.signalTypeToAllCords[st]){
            phySignalInNodes[st].push_back(physicalNodes[physicalGridLayer-1][c.y()][c.x()]);
            assert(physicalNodes[physicalGridLayer-1][c.y()][c.x()]->signal == st);
        }
    }

}

void PowerDistributionNetwork::growPDNNodeEdges(){
    for(PDNEdge *eg : pdnEdgeOwner){
        SignalType n0Sig = eg->n0->signal;
        SignalType n1Sig = eg->n1->signal;

        if((n0Sig == n1Sig) && (n0Sig != SignalType::EMPTY && n0Sig != SignalType::OBSTACLE)){
            eg->signal = n0Sig;
        }
    }
}

bool PowerDistributionNetwork::connectivityAwareAssignment(const std::vector<SignalType> &priority){
    bool priorityEmpty = priority.empty();

    std::vector<SignalType> modifiedPriority = priority;

    std::unordered_map<SignalType, unsigned int> sigPriority;
    unsigned int sigWeight = 2 << (phySOI.size()+3);
    for(int i = 0; i < priority.size(); ++i){
        SignalType st = priority[i];
        if(std::find(phySOI.begin(), phySOI.end(), st) == phySOI.end()) continue;
        if(sigPriority.count(st) != 0) continue;
        sigPriority[st] = sigWeight;
        sigWeight = sigWeight >> 1;
    }

    if(sigPriority.size() != phySOI.size()){
        std::unordered_map<SignalType, double> currentRequirement;
        for(SignalType st : phySOI){
            if(sigPriority.count(st) == 0){
                double totalCurrentRequirement = 0;
                for(const std::string &chipletName : uBump.signalTypeToInstances[st]){
                    totalCurrentRequirement += uBump.instanceToBallOutMap[chipletName]->getMaxCurrent();
                }
                currentRequirement[st] = totalCurrentRequirement;
            }
        }
        std::vector<SignalType> lostSigs;
        for(const auto&[st, cr] : currentRequirement){
            lostSigs.push_back(st);
        }
        std::sort(lostSigs.begin(),lostSigs.end(), [&](SignalType &st1, SignalType &st2){return currentRequirement[st1] > currentRequirement[st2]; });
        for(int i = 0; i < lostSigs.size(); ++i){
            sigPriority[lostSigs[i]] = sigWeight;
            sigWeight = sigWeight >> 1;
            modifiedPriority.push_back(lostSigs[i]);
        }
    }
    



    auto checkConnectivity = [&](SignalType st, const std::vector<PDNNode *> &destinations) -> bool {
        
        std::unordered_set<PDNNode *> destinationSet(destinations.begin(), destinations.end());
        std::unordered_set<PDNNode *> visisted;
        std::queue<PDNNode *> q;

        for(PDNNode *node : phySignalInNodes[st]){
            visisted.insert(node);
            q.push(node);
        }

        auto pushNieghbors = [&](PDNNode *node, PDNEdge *neighborEdge){
            if(neighborEdge == nullptr || neighborEdge->signal != st) return false;
            
            PDNNode *neighborNode = (neighborEdge->n0 == node)? neighborEdge->n1 : neighborEdge->n0;
            if(destinationSet.count(neighborNode) != 0) return true;
            if(visisted.count(neighborNode) == 0){
                visisted.insert(neighborNode);
                q.push(neighborNode);
            }
            return false;
        };

        while(!q.empty()){
            PDNNode *node = q.front(); q.pop();

            if(pushNieghbors(node, node->north)) return true;
            if(pushNieghbors(node, node->south)) return true;
            if(pushNieghbors(node, node->east)) return true;
            if(pushNieghbors(node, node->west)) return true;

            if(pushNieghbors(node, node->up)) return true;
            if(pushNieghbors(node, node->down)) return true;
        }

        return false;
    };

    auto fixConnectivity = [&](SignalType st, const std::vector<PDNNode *> &destinations) -> bool {
        std::unordered_set<PDNNode *> destinationSet(destinations.begin(), destinations.end());
        std::unordered_set<PDNNode *> visisted;
        std::queue<PDNNode *> q;

        for(PDNNode *node : phySignalInNodes[st]){
            visisted.insert(node);
            q.push(node);
        }

        auto pushNieghbors = [&](PDNNode *node, PDNEdge *neighborEdge){
            
            if(neighborEdge == nullptr) return false;
            if((neighborEdge->signal != st) && (neighborEdge->signal != SignalType::EMPTY)) return false;
            
            PDNNode *neighborNode = (neighborEdge->n0 == node)? neighborEdge->n1 : neighborEdge->n0;
            if((neighborNode->signal != st) && (neighborNode->signal != SignalType::EMPTY)) return false;
            
            
            if(destinationSet.count(neighborNode) != 0){
                // commit all visited nodes
                for(PDNNode *vnode : visisted){
                    vnode->signal = st;
                }
                growPDNNodeEdges();

                return true;
            }
            
            if(visisted.count(neighborNode) == 0){
                visisted.insert(neighborNode);
                q.push(neighborNode);
            }
            return false;
        };

        bool haveFix = false;
        while(!q.empty()){
            PDNNode *node = q.front(); q.pop();

            if(pushNieghbors(node, node->north)) return true;
            if(pushNieghbors(node, node->south)) return true;
            if(pushNieghbors(node, node->east)) return true;
            if(pushNieghbors(node, node->west)) return true;

            if(pushNieghbors(node, node->up)) return true;
            if(pushNieghbors(node, node->down)) return true;
        }

        return false;

    };

    for(SignalType st : modifiedPriority){
        for(const std::string &chipletName : phyChipletNames[st]){
            if(!checkConnectivity(st, phyChipletNodes[chipletName])){
                if(!fixConnectivity(st, phyChipletNodes[chipletName])){
                    std::cout << "[Physical Implementation] Chiplet " << chipletName << " of " << st << " fails to connect to current source! " << std::endl;
                    // return false;
                }
            }
        }
    }

    std::cout << "[Physical Implementation] All chiplet Passes connectivity test" << std::endl;
    


    std::unordered_set<PDNNode *> sourceReachableNodes;
    std::unordered_set<PDNNode *> sinkUsedNodes;

    std::unordered_set<PDNNode *> mustExistNodes;
    std::unordered_set<PDNNode *> notWorthGrowingNodes;

    
    auto markReachableNodes = [&](SignalType st, const std::vector<PDNNode *> &seedNodes, std::unordered_set<PDNNode *> &reachableNodes){
        std::unordered_set<PDNNode *> visisted;
        std::queue<PDNNode *> q;

        for(PDNNode *node : seedNodes){
            visisted.insert(node);
            q.push(node);
        }

        auto pushNieghbors = [&](PDNNode *node, PDNEdge *neighborEdge){
            if(neighborEdge == nullptr || neighborEdge->signal != st) return;
            
            PDNNode *neighborNode = (neighborEdge->n0 == node)? neighborEdge->n1 : neighborEdge->n0;
            if(visisted.count(neighborNode) == 0){
                visisted.insert(neighborNode);
                q.push(neighborNode);
            }
        };

        while(!q.empty()){
            PDNNode *node = q.front(); q.pop();

            pushNieghbors(node, node->north);
            pushNieghbors(node, node->south);
            pushNieghbors(node, node->east);
            pushNieghbors(node, node->west);

            pushNieghbors(node, node->up);
            pushNieghbors(node, node->down);
        }
        reachableNodes.insert(visisted.begin(), visisted.end());
    };

    for(SignalType st : modifiedPriority){

        // forward pass, from source side bfs
        markReachableNodes(st, phySignalInNodes[st], sourceReachableNodes);
        
        
        // backward pass, from sink side bfs
        std::vector<PDNNode *> signalAllSinks;
        for(const std::string &chipletName : phyChipletNames[st]){
            for(PDNNode *node : phyChipletNodes[chipletName]){
                signalAllSinks.push_back(node);
            }
        }
        markReachableNodes(st, signalAllSinks, sinkUsedNodes);

        mustExistNodes.insert(phySignalInNodes[st].begin(), phySignalInNodes[st].end());
        mustExistNodes.insert(signalAllSinks.begin(), signalAllSinks.end());
    }

    
    for(size_t layer = 0; layer < physicalGridLayer; ++layer){
        for(size_t y = 0; y < physicalGridHeight; ++y){
            for(size_t x = 0; x < physicalGridWidth; ++x){
                PDNNode *node = physicalNodes[layer][y][x];
                if(mustExistNodes.count(node)){
                    if(!sourceReachableNodes.count(node) || !sinkUsedNodes.count(node)) notWorthGrowingNodes.insert(node);
                    continue;
                }
                SignalType nodeSt = node->signal;

                if(POWER_SIGNAL_SET.count(nodeSt) && (!sourceReachableNodes.count(node) || !sinkUsedNodes.count(node))){
                    node->signal = SignalType::EMPTY;
                    if(node->north != nullptr && POWER_SIGNAL_SET.count(node->north->signal)) node->north->signal = SignalType::EMPTY;
                    if(node->south != nullptr && POWER_SIGNAL_SET.count(node->south->signal)) node->south->signal = SignalType::EMPTY;
                    if(node->east != nullptr && POWER_SIGNAL_SET.count(node->east->signal)) node->east->signal = SignalType::EMPTY;
                    if(node->west != nullptr && POWER_SIGNAL_SET.count(node->west->signal)) node->west->signal = SignalType::EMPTY;
                    if(node->up != nullptr && POWER_SIGNAL_SET.count(node->up->signal)) node->up->signal = SignalType::EMPTY;
                    if(node->down != nullptr && POWER_SIGNAL_SET.count(node->down->signal)) node->down->signal = SignalType::EMPTY;
                }
            }
        }
    }

    // monitor empty nodes
    std::unordered_set<PDNNode *> emptyNodes;
    for(size_t layer = 0; layer < physicalGridLayer; ++layer){
        for(size_t y = 0; y < physicalGridHeight; ++y){
            for(size_t x = 0; x < physicalGridWidth; ++x){
                PDNNode *node = physicalNodes[layer][y][x];
                if(node->signal == SignalType::EMPTY) emptyNodes.insert(node);
            }
        }
    }

    const int maxIteration = std::max(physicalGridWidth, physicalGridHeight);
    
    int iterationCounter = 0;
    while (!emptyNodes.empty() && (iterationCounter++ < maxIteration)) {
        // std::cout << "empty node cout = " << emptyNodes.size() << std::endl;
    
        for (auto it = emptyNodes.begin(); it != emptyNodes.end(); ) {
            PDNNode* node = *it;

            // Count neighboring power signals
            std::unordered_map<SignalType, int> edgeCount;
            edgeCount.reserve(6); // up to 6 neighbors

            auto bump = [&](PDNEdge* p) {
                if(p == nullptr) return;
                PDNNode *neighbor = (p->n0 == node)? p->n1 : p->n0;
                if(notWorthGrowingNodes.count(neighbor)) return;

                if (POWER_SIGNAL_SET.count(neighbor->signal)) {
                    ++edgeCount[neighbor->signal];
                }
            };

            bump(node->north);
            bump(node->south);
            bump(node->east);
            bump(node->west);
            bump(node->up);
            bump(node->down);

            if (!edgeCount.empty()) {
                if(priorityEmpty){
                    auto itMax = edgeCount.begin();
                    // use NN logic to fill the empty nodes
                    for (auto itEC = std::next(edgeCount.begin()); itEC != edgeCount.end(); ++itEC) {
                        if (itEC->second > itMax->second) itMax = itEC;
                    }
                    node->signal = itMax->first;

                }else{
                    // use priority logic to fill empty nodes

                    for(auto &[sig, count] : edgeCount){
                        count = count * (sigPriority[sig] + 1);
                    }

                    auto itMax = edgeCount.begin();
                    for (auto itEC = std::next(edgeCount.begin()); itEC != edgeCount.end(); ++itEC) {
                        if (itEC->second > itMax->second) itMax = itEC;
                    }
                    node->signal = itMax->first;
                }
                it = emptyNodes.erase(it);
            } else {
                ++it; // nothing to do for this node
            }
        }
        growPDNNodeEdges();
    }
    std::cout << "[Physical Implementation] Complete Physical Implementation Empty Assignments" << std::endl;

    return true;
    
}

void PowerDistributionNetwork::exportPhysicalToCircuitBySignal(SignalType st, const Technology &tch, const EqCktExtractor &extor, const std::string &filePath){
    std::ofstream ofs(filePath, std::ios::out);
    assert(ofs.is_open());

    auto pointToNode = [](int layer, const Cord &c) -> std::string {
        return "n" + std::to_string(c.x()) + "_" + std::to_string(c.y()) + "_" + std::to_string(layer);
    };

    ofs << "****** Equivalent Circuit Components ******" << std::endl;
    ofs << ".SUBCKT ubump in out" << std::endl;
    ofs << "R1 in mid " << tch.getMicrobumpResistance() << "m" << std::endl;
    ofs << "L1 mid out " << tch.getMicrobumpInductance() << "p" << std::endl;
    ofs << ".ENDS" << std::endl;
    ofs << std::endl;

    ofs << ".SUBCKT via in out" << std::endl;
    ofs << "R1 in mid " << extor.getInterposerViaResistance() << "m" << std::endl;
    ofs << "L1 mid out " << extor.getInterposerViaInductance() << "p" << std::endl;
    ofs << ".ENDS" << std::endl;
    ofs << std::endl;

    ofs << ".SUBCKT edge in out" << std::endl;
    ofs << "R1 in mid " << 2*extor.getInterposerResistance() << "m" << std::endl;
    ofs << "L1 mid out " << 2*extor.getInterposerInductance() << "p" << std::endl;
    ofs << ".ENDS" << std::endl;
    ofs << std::endl;

    ofs << ".SUBCKT tsv in out" << std::endl;
    ofs << "R1 in mid " << tch.getTsvResistance() << "m" << std::endl;
    ofs << "L1 mid out " << tch.getTsvInductance() << "p" << std::endl;
    ofs << ".ENDS" << std::endl;
    ofs << std::endl;

    ofs << ".SUBCKT cfour in out" << std::endl;
    ofs << "R1 in mid " << tch.getC4Resistance() << "m" << std::endl;
    ofs << "L1 mid out " << tch.getC4Inductance() << "p" << std::endl;
    ofs << ".ENDS" << std::endl;
    ofs << std::endl;

    ofs << "****** Equivalent Circuit ******" << std::endl;
    int chipletcount = phyChipletNames[st].size();
    assert(chipletcount > 0);
    std::vector<std::string> &chipletTitles = phyChipletNames[st];

    int uBumpCounter = 0;
    int viaCounter = 0;
    int edgeCounter = 0;
    int tsvCounter = 0;
    int c4Counter = 0;
    int capCounter = 0;

    ofs << ".SUBCKT eqckt in ";
    for(int i = 0; i < chipletcount; ++i){
        ofs << chipletTitles[i] << "_o ";
    }
    ofs << "vss" << std::endl;

    ofs << "*** MicroBumps ***" << std::endl;
    for(int ubidx = 0; ubidx < chipletcount; ++ubidx){
        std::string instanceName = chipletTitles[ubidx];
        for(PDNNode *pnode : phyChipletNodes[instanceName]){
            ofs << "Xubump" << uBumpCounter++ << "_" <<  to_string(pnode) << " " << chipletTitles[ubidx] << "_o " << to_string(pnode) << " ubump" << std::endl;
        }
    }

    ofs << "*** C4 Bumps ***" << std::endl;
    for(const C4PinCluster *cluster : c4.signalTypeToAllClusters[st]){
        std::string c4Node = pointToNode(m_metalLayerCount, cluster->representation);
        ofs << "Xcfour" << c4Counter++ << " in " << c4Node << " cfour" << std::endl;
        for(const Cord &c : cluster->pins){
            ofs << "Xtsv" << tsvCounter++ << "_" << pointToNode(m_c4ConnectedMetalLayerIdx, c) << " ";
            ofs << c4Node << " " << pointToNode(m_c4ConnectedMetalLayerIdx, c) << " tsv" << std::endl;
        }
    }

    ofs << "*** Metal and Vias ***" << std::endl;
    len_t pinXMin = 0;
    len_t pinXMax = physicalGridWidth - 1;
    len_t pinYMin = 0;
    len_t pinYMax = physicalGridHeight - 1;
    for(size_t layer = 0; layer < physicalGridLayer; ++layer){
        for(size_t y = 0; y < physicalGridHeight; ++y){
            for(size_t x = 0; x < physicalGridWidth; ++x){
                bool xOnEdge = (x == pinXMin) || (x == pinXMax);
                bool yOnEdge = (y == pinYMin) || (y == pinYMax);

                if(xOnEdge && yOnEdge){
                    ofs << "C" << capCounter++ << " " << pointToNode(layer, Cord(x, y)) << " vss " << extor.getInterposerCapacitanceCornerCell()/4.0 << "f" << std::endl;
                }else if(xOnEdge || yOnEdge){
                    ofs << "C" << capCounter++ << " " << pointToNode(layer, Cord(x, y)) << " vss " << extor.getInterposerCapacitanceEdgeCell()/4.0 << "f" << std::endl;
                }else{
                    ofs << "C" << capCounter++ << " " << pointToNode(layer, Cord(x, y)) << " vss " << extor.getInterposerCapacitanceCenterCell()/4.0 << "f" << std::endl;
                }
            }
        }
    }
    
    for(PDNEdge *edge : pdnEdgeOwner){
        if(edge->signal != st) continue;
        
        if(edge->isVia){
            ofs << "Xvia" << viaCounter++ << "_" << to_string(edge->n0) << "_" << to_string(edge->n1) << " ";
            ofs << to_string(edge->n0) << " " << to_string(edge->n1) << " via" << std::endl;
        }else{
            ofs << "Xedge" << edgeCounter++ << "_" << to_string(edge->n0) << "_" << to_string(edge->n1) << " ";
            ofs << to_string(edge->n0) << " " << to_string(edge->n1) << " edge" << std::endl;
        }
    }

    ofs << ".ENDS" << std::endl;
    ofs << std::endl;

    ofs << "****** Chiplet Load Model ******" << std::endl;
    ofs << ".SUBCKT chiplet in vss Rval=50m Lval=200n Cval=30p Iload=1.0" << std::endl;
    ofs << "Rpath in n1 Rval" << std::endl;
    ofs << "Lpath n1 n2 Lval" << std::endl;
    ofs << "Cpath n2 vss Cval" << std::endl;
    ofs << "ILOAD n2 vss DC Iload" << std::endl;
    ofs << ".ENDS" << std::endl;

    ofs << std::endl;

    ofs << "****** Input PCB model Model ******" << std::endl;
    ofs << ".SUBCKT pcb vrm_low vrm_high out vss" << std::endl;
    ofs << "Lpcbh vrm_high n1 " << tch.getPCBInductance() << "p" << std::endl;
    ofs << "Rpcbh n1 out " << tch.getPCBResistance() << "u" << std::endl;
    ofs << "Ldecaph out n2 " << tch.getPCBDecapInductance() <<  "n" << std::endl;
    ofs << "Cdecap n2 n3 " << tch.getPCBDecapCapacitance() << "u" << std::endl;
    ofs << "Rdecapl n3 vss " << tch.getPCBDecapResistance() << "u" << std::endl;
    ofs << "Rpcbl n4 vss " << tch.getPCBResistance() << "u" << std::endl;
    ofs << "Lpcbl vrm_low n4 " << tch.getPCBInductance() << "p" << std::endl;
    
    ofs << "* --- Decap Bank for Broadband Z(f) Control ---" << std::endl;
    ofs << "* 1 μF decap (low freq bulk, moderately damped)" << std::endl;
    ofs << "Rdecap1u out n1u 0.05" << std::endl;
    ofs << "Cdecap1u n1u vss 1u" << std::endl;
    ofs << "* 100 nF decap (mid-low freq)" << std::endl;
    ofs << "Rdecap100n out n100n 0.1" << std::endl;
    ofs << "Cdecap100n n100n vss 100n" << std::endl;
    ofs << "* 10 nF decap (mid freq)" << std::endl;
    ofs << "Rdecap10n out n10n 0.2" << std::endl;
    ofs << "Cdecap10n n10n vss 10n" << std::endl;
    ofs << "* 1 nF decap (high freq)" << std::endl;
    ofs << "Rdecap1n out n1n 0.3" << std::endl;
    ofs << "Cdecap1n n1n vss 1n" << std::endl;
    ofs << "* 100 pF decap (GHz damping)" << std::endl;
    ofs << "Rdecap100p out n100p 0.5" << std::endl;
    ofs << "Cdecap100p n100p vss 100p" << std::endl;
    
    ofs << ".ENDS" << std::endl;

    ofs << std::endl << std::endl;
    ofs << "****** Main ******" << std::endl;
    ofs << ".param VDD=1.0" << std::endl;
    ofs << "VVRM vrm_high vrm_low DC VDD" << std::endl;
    ofs << "Xpcb vrm_low vrm_high pcb_out 0 pcb" << std::endl;
    
    ofs << "Xeqckt pcb_out ";
    for(int i = 0; i < chipletcount; ++i){
        ofs << chipletTitles[i] << "_o ";
    }
    ofs << "0" << " " << "eqckt" << std::endl;
    
    for(int i = 0; i < chipletcount; ++i){
        ofs << "Xchiplet" << i << " " << chipletTitles[i] << "_o " << "0 chiplet ";
        BallOut *bt = uBump.instanceToBallOutMap.at(chipletTitles[i]);
        assert(bt != nullptr);
        ofs << "Rval=" << bt->getSeriesResistance() << "m Lval=" << bt->getSeriesInductance() << "n Cval=" << bt->getShuntCapacitance() << "p ";
        ofs << "Iload=" << bt->getMaxCurrent() << std::endl;
    }

    ofs << std::endl;
    ofs << "****** DC IR-Drop test ******" << std::endl;
    ofs << ".OPTION POST=2 INGOLD=2 RUNLVL=6" << std::endl;
    ofs << std::endl << ".op" << std::endl;
    ofs << ".DC VDD 1.5 1.5 0.5" << std::endl;
    
    ofs << ".print V(vrm_low) V(vrm_high) V(pcb_out) ";
    for(int i = 0; i < chipletcount; ++i){
        ofs << "V(" << chipletTitles[i] << "_o" << ") ";
    }
    ofs << std::endl;

    ofs << ".measure DC irdroppcb param=\'V(vrm_high) - V(pcb_out)\'" << std::endl;
    for(int i = 0; i < chipletcount; ++i){
        Rectangle chpletPlacement = uBump.instanceToRectangleMap[chipletTitles[i]];
        Cord chipletLL = rec::getLL(chpletPlacement);

        ofs << ".measure DC IRDrop" << chipletTitles[i] << "_" <<  chipletLL.x() << "_" << chipletLL.y() << "_" << rec::getWidth(chpletPlacement) << "_" << rec::getHeight(chpletPlacement);
        ofs << "_" << uBump.instanceToBallOutMap[chipletTitles[i]]->getMaxCurrent();
        ofs << " param=\'V(pcb_out) - V(" << chipletTitles[i] << "_o" << ")\'" << std::endl;
    }
    ofs << std::endl;

    // ofs << ".OPTION RELTOL=1e-4 ABSTOL=1e-8 VNTOL=1e-6" << std::endl;
    ofs << ".end" << std::endl;
    ofs.close();
}

void PowerDistributionNetwork::exportPhysicalToCircuit(const Technology &tch, const EqCktExtractor &extor, const std::string &filePathPrefix){
    for(SignalType st : phySOI){
        std::string filePath = filePathPrefix + to_string(st) + ".sp";
        exportPhysicalToCircuitBySignal(st, tch, extor, filePath);
    }
}

void PowerDistributionNetwork::writeSnapShot(const std::string &fileName){
    std::ofstream ofs(fileName, std::ios::out);

    assert(ofs.is_open());
    if(!ofs.is_open()) return;

    for(int layer = 0; layer < m_metalLayerCount; ++layer){
        for(int y = 0; y < m_gridHeight; ++y){
            for(int x = 0; x < m_gridWidth; ++x){
                ofs << metalLayers[layer].canvas[y][x] << std::endl;
            }
        }
    }

    for(int layer = 0; layer < m_viaLayerCount; ++layer){
        for(int y = 0; y < m_pinHeight; ++y){
            for(int x = 0; x < m_pinWidth; ++x){
                ofs << viaLayers[layer].canvas[y][x] << std::endl;
            }
        }
    }

    ofs.close();
}

void PowerDistributionNetwork::readSnapShot(const std::string &fileName){
    
    std::ifstream ifs(fileName, std::ios::in);

    assert(ifs.is_open());
    if(!ifs.is_open()) return;

    for(int layer = 0; layer < m_metalLayerCount; ++layer){
        for(int y = 0; y < m_gridHeight; ++y){
            for(int x = 0; x < m_gridWidth; ++x){
                ifs >> metalLayers[layer].canvas[y][x];
            }
        }
    }

    for(int layer = 0; layer < m_viaLayerCount; ++layer){
        for(int y = 0; y < m_pinHeight; ++y){
            for(int x = 0; x < m_pinWidth; ++x){
                ifs >> viaLayers[layer].canvas[y][x];
            }
        }
    }

    ifs.close();
}

void markPinPadsWithoutSignals(std::vector<std::vector<SignalType>> &gridCanvas, const std::vector<std::vector<SignalType>> &pinCanvas, const std::unordered_set<SignalType> &avoidSignalTypes){
    len_t gridHeight = gridCanvas.size();
    assert(gridHeight > 0);
    len_t gridWidth = gridCanvas[0].size();
    assert(gridWidth > 0);
    len_t pinHeight = pinCanvas.size();
    assert(pinHeight == (gridHeight + 1));
    len_t pinWidth = pinCanvas[0].size();
    assert(pinWidth == (gridWidth + 1));

    for(int j = 0; j < pinHeight; ++j){
        for(int i = 0; i < pinWidth; ++i){
            SignalType st = pinCanvas[j][i];
            if(avoidSignalTypes.count(st)) continue;

            if(i > 0){
                if(j > 0) gridCanvas[j-1][i-1] = st;
                if(j < (pinHeight - 1)) gridCanvas[j][i-1] = st;
            }
            if(i < (pinWidth - 1)){
                if(j > 0) gridCanvas[j-1][i] = st;
                if(j < (pinHeight - 1)) gridCanvas[j][i] = st;
            }
        }
    }
}

void markPinPadsWithSignals(std::vector<std::vector<SignalType>> &gridCanvas, const std::vector<std::vector<SignalType>> &pinCanvas, const std::unordered_set<SignalType> &signalTypes){
    len_t gridHeight = gridCanvas.size();
    assert(gridHeight > 0);
    len_t gridWidth = gridCanvas[0].size();
    assert(gridWidth > 0);
    len_t pinHeight = pinCanvas.size();
    assert(pinHeight == (gridHeight + 1));
    len_t pinWidth = pinCanvas[0].size();
    assert(pinWidth == (gridWidth + 1));

    for(int j = 0; j < pinHeight; ++j){
        for(int i = 0; i < pinWidth; ++i){
            SignalType st = pinCanvas[j][i];
            if(signalTypes.count(st) == 0) continue;

            if(i > 0){
                if(j > 0) gridCanvas[j-1][i-1] = st;
                if(j < (pinHeight - 1)) gridCanvas[j][i-1] = st;
            }
            if(i < (pinWidth - 1)){
                if(j > 0) gridCanvas[j-1][i] = st;
                if(j < (pinHeight - 1)) gridCanvas[j][i] = st;
            }
        }
    }
}

void runClustering(const std::vector<std::vector<SignalType>> &canvas, std::vector<std::vector<int>> &cluster, std::unordered_map<SignalType, std::vector<int>> &label){
    const int rows = static_cast<int>(canvas.size());
    if (rows == 0) return;
    const int cols = static_cast<int>(canvas[0].size());

    // [NEW] Clear label map
    label.clear();

    // Resize cluster to match canvas and initialize all to -1
    cluster.resize(rows);
    for (std::size_t i = 0; i < cluster.size(); ++i) {
        cluster[i].resize(cols, -1);
    }

    int currentLabel = 0;
    
    const int dRow[4] = {-1, 1, 0, 0};
    const int dCol[4] = {0, 0, -1, 1};
    
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (cluster[i][j] != -1) continue;
            
            SignalType currentType = canvas[i][j];
            
            std::queue<std::pair<int, int>> bfsQueue;
            bfsQueue.push(std::make_pair(i, j));
            cluster[i][j] = currentLabel;
            
            while (!bfsQueue.empty()) {
                std::pair<int, int> cell = bfsQueue.front();
                bfsQueue.pop();
                int x = cell.first;
                int y = cell.second;
                
                for (int dir = 0; dir < 4; ++dir) {
                    int newX = x + dRow[dir];
                    int newY = y + dCol[dir];
                    
                    if (newX < 0 || newX >= rows || newY < 0 || newY >= cols)
                        continue;
                    if (canvas[newX][newY] != currentType)
                        continue;
                    if (cluster[newX][newY] != -1)
                        continue;
                    
                    cluster[newX][newY] = currentLabel;
                    bfsQueue.push(std::make_pair(newX, newY));
                }
            }
            
            label[currentType].push_back(currentLabel);
            ++currentLabel;
        }
    }
}

std::unordered_map<SignalType, DoughnutPolygonSet> collectDoughnutPolygons(const std::vector<std::vector<SignalType>> &canvas){
    using namespace boost::polygon::operators;
    std::unordered_map<SignalType, DoughnutPolygonSet> dpSetMap;
    int canvasHeight = canvas.size();
    int canvasWidth = canvas[0].size();
    
    for(int j = 0; j < canvasHeight; ++j){
        for(int i = 0; i < canvasWidth; ++i){
            SignalType st = canvas[j][i];
            Rectangle rect(i, j, i+1, j+1);

            std::unordered_map<SignalType, DoughnutPolygonSet>::iterator it = dpSetMap.find(st);

            dpSetMap[st] += rect;
        }
    }

    return dpSetMap;
}


