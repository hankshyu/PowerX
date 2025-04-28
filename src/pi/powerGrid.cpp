//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        04/06/2025 16:10:52
//  Module Name:        powerGrid.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A parent class that includes the bump holding data structures
//                      for ubump/C4, and the m5 and m7 grids
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//
//////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:
#include <cassert>
#include <vector>
#include <queue>
#include <map>

// 2. Boost Library:


// 3. Texo Library:
#include "cord.hpp"
#include "signalType.hpp"
#include "microBump.hpp"
#include "c4Bump.hpp"
#include "orderedSegment.hpp"
#include "powerGrid.hpp"

// Initialize the static const unordered_map
const std::unordered_map<SignalType, SignalType> PowerGrid::defulatM5SigPadMap = {
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

const std::unordered_map<SignalType, SignalType> PowerGrid::defulatM7SigPadMap = {
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
    { SignalType::SIGNAL, SignalType::OBSTACLE},
    { SignalType::OBSTACLE, SignalType::OBSTACLE},
};

PowerGrid::PowerGrid(const std::string &fileName): uBump(fileName), c4(fileName) {
    assert(uBump.getPinMapWidth() == c4.getPinMapWidth());
    assert(uBump.getPinMapHeight() == c4.getPinMapHeight());
    assert(uBump.getPinMapWidth() > 1);
    assert(uBump.getPinMapHeight() > 1);

    this->canvasWidth = uBump.getPinMapWidth() - 1;
    this->canvasHeight = uBump.getPinMapHeight() - 1;
    
    this->canvasM5.resize(this->canvasHeight, std::vector<SignalType>(this->canvasWidth, SignalType::EMPTY));
    this->canvasM7.resize(this->canvasHeight, std::vector<SignalType>(this->canvasWidth, SignalType::EMPTY));
}

void PowerGrid::insertPinPads(const PinMap &pm, std::vector<std::vector<SignalType>> &canvas, const std::unordered_map<SignalType, SignalType> &padTypeMap){
    for(std::unordered_map<Cord, SignalType>::const_iterator cit = pm.cordToSignalTypeMap.begin(); cit != pm.cordToSignalTypeMap.end(); ++cit){
        SignalType st = cit->second;
        std::unordered_map<SignalType, SignalType>::const_iterator ptcit = padTypeMap.find(st);
        // skip the kind of signal where doesn't exist in padTypeMap
        if(ptcit == padTypeMap.end()) continue;
        
        Cord c = cit->first;
        SignalType padst = ptcit->second;

        // if((st == SignalType::GROUND) || (st == SignalType::SIGNAL)) continue;

        if(c.x() != 0){
            if(c.y() != pm.getPinMapHeight()){
                canvas[c.y()][c.x() - 1] = padst;
            }

            if(c.y() != 0){
                canvas[c.y() - 1][c.x() - 1] = padst;
            }

        }

        if(c.x() != pm.getPinMapWidth()){
            if(c.y() != pm.getPinMapHeight()){
                canvas[c.y()][c.x()] = padst;
            }

            if(c.y() != 0){
                canvas[c.y() - 1][c.x()] = padst;

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


void PowerGrid::reportOverlaps() const {
    std::map<SignalType, int> m5Stats;
    std::map<SignalType, int> m7Stats;
    std::map<SignalType, int> ovStats;

    for(int j = 0; j < canvasHeight; ++j){
        for(int i = 0; i < canvasWidth; ++i){
            SignalType stup = canvasM5[j][i];
            if(m5Stats.find(stup) == m5Stats.end()) m5Stats[stup] = 0;
            m5Stats[stup]++;

            SignalType stdn = canvasM7[j][i];
            if(m7Stats.find(stdn) == m7Stats.end()) m7Stats[stdn] = 0;
            m7Stats[stdn]++;

            if(stup == stdn){
                if(ovStats.find(stdn) == ovStats.end()) ovStats[stdn] = 0;
                ovStats[stdn]++;
            }
        }
    }

    // for(auto at : m5Stats){
    //     std::cout << at.first << " " << at.second << std::endl;
    // }
    // std::cout << std::endl;
    
    // for(auto at : m7Stats){
    //     std::cout << at.first << " " << at.second << std::endl;
    // }

    std::cout << "Overlap Report: " << std::endl;
    for(auto at : ovStats){
        SignalType st = at.first;
        double ovpct = 100*2*double(at.second) / (double(m7Stats[st]) + double(m5Stats[st]));
        std::cout << at.first << " " << at.second << "(" << ovpct << "%)" << std::endl;    
    }
}