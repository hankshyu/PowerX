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
#include "signalType.hpp"
#include "objectArray.hpp"
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

        // parse PDN_START ... PDN_END part (preplaced part)
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


