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

bool PowerDistributionNetwork::checkOnePiece(int metalLayerIdx){
    std::unordered_map<SignalType, DoughnutPolygonSet> dpsMap = collectDoughnutPolygons(metalLayers[metalLayerIdx].canvas);
    for(std::unordered_map<SignalType, DoughnutPolygonSet>::const_iterator cit = dpsMap.begin(); cit != dpsMap.end(); ++cit){
        SignalType st = cit->first;
        if(POWER_SIGNAL_SET.count(st) == 0) continue;
        if(dps::getShapesCount(cit->second) > 1) return false;
    }

    return true;
}

void PowerDistributionNetwork::fillEnclosedRegionsonCanvas(){
    std::vector<std::vector<bool>> visited;
    const std::vector<Cord> directions = {Cord(-1, 0), Cord(1, 0), Cord(0, -1), Cord(0, 1)};

    auto inBounds = [&](int y, int x) {
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

                        if (!inBounds(ny, nx)) {
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
                    ofs << "Xedge" << edgeCounter++ << "_" << pointToNode(mLayerIdx, candl.low()) << "_" << pointToNode(mLayerIdx, candl.high()) << " ";
                    ofs << pointToNode(mLayerIdx, candl.low()) << " " << pointToNode(mLayerIdx, candl.high()) << " edge" << std::endl;
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


