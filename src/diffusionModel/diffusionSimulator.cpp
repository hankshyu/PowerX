//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        07/03/2025 13:26:38
//  Module Name:        diffusionSimulator.chpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        The top module of the diffusion system
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//
/////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <cstdlib>

// 2. Boost Library:

// 3. Texo Library:
#include "diffusionSimulator.hpp"
#include "cell.hpp"
#include "dirFlags.hpp"
#include "cord.hpp"


CellCord DiffusionSimulator::calCellCord(size_t idx) {
    size_t layer = idx / m_cellGrid2DCount;
    size_t rem   = idx % m_cellGrid2DCount;

    size_t y = rem / m_cellGridWidth;
    size_t x = rem % m_cellGridWidth;

    return CellCord(layer, x, y);
}

ViaCord DiffusionSimulator::calViaCord(size_t idx){
    assert(idx != SIZET_INVALID);
    if(idx < m_viaGrid2DAccumlateCount[0]) return ViaCord(0, idx);
    for(int i = 1; i < m_viaGridLayers; ++i){
        if(idx < m_viaGrid2DAccumlateCount[i]) return ViaCord(i, idx - m_viaGrid2DAccumlateCount[i-1]);
    }

    assert(idx == SIZET_INVALID);
    return ViaCord(SIZET_INVALID, SIZET_INVALID);
}


DiffusionSimulator::DiffusionSimulator(const std::string &fileName): PowerDistributionNetwork(fileName){
    // put preplaced signals to the canvas
    for(int i = 0; i < m_metalLayerCount; ++i){
        this->metalLayers[i].markPreplacedToCanvas();
    }

    for(int i = 0; i < m_viaLayerCount; ++i){
        this->viaLayers[i].markPreplacedToCanvas();
    } 

    std::unordered_set<SignalType> viaToMetalSignalTypes(POWER_SIGNAL_SET.begin(), POWER_SIGNAL_SET.end());
    viaToMetalSignalTypes.insert(SignalType::SIGNAL);
    // insert pads for the uBump connecting metal layer
    markPinPadsWithSignals(this->metalLayers[m_ubumpConnectedMetalLayerIdx].canvas, this->uBump.canvas, POWER_SIGNAL_SET);
    markPinPadsWithSignals(this->metalLayers[m_ubumpConnectedMetalLayerIdx].canvas, this->viaLayers[0].canvas, viaToMetalSignalTypes);

    for(int mLayer = m_ubumpConnectedMetalLayerIdx + 1; mLayer < m_c4ConnectedMetalLayerIdx; ++mLayer){
        markPinPadsWithSignals(this->metalLayers[mLayer].canvas, this->viaLayers[mLayer-1].canvas,viaToMetalSignalTypes);
        markPinPadsWithSignals(this->metalLayers[mLayer].canvas, this->viaLayers[mLayer].canvas, viaToMetalSignalTypes);
    }

    markPinPadsWithSignals(this->metalLayers[m_c4ConnectedMetalLayerIdx].canvas, this->viaLayers[m_viaLayerCount-1].canvas, viaToMetalSignalTypes);
    markPinPadsWithoutSignals(this->metalLayers[m_c4ConnectedMetalLayerIdx].canvas, this->c4.canvas, {SignalType::EMPTY, SignalType::OBSTACLE});
    

    // turn signals that isn't POWER_SIGNAL_SET on metal/via layer into obstacles
    for(int viaLayer = 0; viaLayer < m_viaLayerCount; ++viaLayer){
        for(int j = 0; j < m_pinHeight; ++j){
                for(int i = 0; i < m_pinWidth; ++i){
                SignalType st = viaLayers[viaLayer].canvas[j][i];
                if((st == SignalType::GROUND) || (st == SignalType::SIGNAL) || (st == SignalType::OVERLAP)){
                    viaLayers[viaLayer].canvas[j][i] = SignalType::OBSTACLE;
                }
            }
        }
    }

    for(int metalLayer = 0; metalLayer < m_metalLayerCount; ++metalLayer){
        for(int j = 0; j < m_gridHeight; ++j){
            for(int i = 0; i < m_gridWidth; ++i){
                SignalType st = metalLayers[metalLayer].canvas[j][i];
                if((st == SignalType::GROUND) || (st == SignalType::SIGNAL) || (st == SignalType::OVERLAP)){
                    metalLayers[metalLayer].canvas[j][i] = SignalType::OBSTACLE;
                }
            }
        }
    }
}

void DiffusionSimulator::fillCanvasConfinedSpace() {
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

void DiffusionSimulator::initialise(){

    // transfer the marking metal/via layer onto chamber-related data structures
    
    // m_cellGridLayers = m_metalLayerCount;
    // m_cellGridWidth = m_gridWidth;
    // m_cellGridHeight = m_gridHeight;
    // m_cellGrid2DCount = m_gridWidth * m_gridHeight;
    // m_cellGrid3DCount = m_cellGrid2DCount * m_metalLayerCount;
    
    // cellGrid.resize(m_cellGrid3DCount);
    // cellGridType.resize(m_cellGrid3DCount, CellType::EMPTY);
    // cellGridLabel.resize(m_cellGrid3DCount, 0);

    // for(int metalLayer = 0; metalLayer < m_metalLayerCount; ++metalLayer){
    //     for(int j = 0; j < m_gridHeight; ++j){
    //         for(int i = 0; i < m_gridWidth; ++i){
    //             size_t cellIndex = calCellIdx(metalLayer, j, i);
    //             MetalCell &cell = cellGrid[cellIndex];
    //             SignalType st = etalLayers[metalLayer].canvas[j][i];
    //             if(st != SignalType::EMPTY){
    //                 cell.signal = st;
                    
    //                 if(st == SignalType::OBSTACLE) cellGridType[cellIndex] = CellType::OBSTACLES;
    //                 else cellGridType[cellIndex] = CellType::PREPLACED;
    //             }
    //             if(j != 0){
    //                 cell.downCell = cellGrid[calCellIndex(metalLayer, j-1, i)];
    //                 addDirection(cell.fullDirection, DirFlagAxis::DOWN);
    //             }
    //         }
    //     }
    // }


}
