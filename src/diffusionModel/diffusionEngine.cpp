//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        07/08/2025 18:33:15
//  Module Name:        diffusionEngine.hpp
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
// 07/08/2025           Ported from diffusionSimulator class
/////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:

// 2. Boost Library:

// 3. Texo Library:
#include "diffusionEngine.hpp"

DiffusionEngine::DiffusionEngine(const std::string &fileName): PowerDistributionNetwork(fileName) {
    this->m_cellGridLayers = m_metalLayerCount;
    this->m_cellGridWidth = m_gridWidth;
    this->m_cellGridHeight = m_gridHeight;
    this->m_cellGrid2DCount = m_gridWidth * m_gridHeight;
    this->m_cellGrid3DCount = m_cellGrid2DCount * m_metalLayerCount;

    this->m_viaGridLayers = m_viaLayerCount;
}

size_t DiffusionEngine::calMetalIdx(size_t layer, size_t height, size_t width) const {
    return layer * m_cellGrid2DCount + height * m_cellGridWidth + width;
}

size_t DiffusionEngine::calMetalIdx(const MetalCord &cc) const {
    return cc.l * m_cellGrid2DCount + cc.h * m_cellGridWidth + cc.w;
}

MetalCord DiffusionEngine::calMetalCord(size_t idx) const {
    size_t layer = idx / m_cellGrid2DCount;
    size_t rem   = idx % m_cellGrid2DCount;

    size_t y = rem / m_cellGridWidth;
    size_t x = rem % m_cellGridWidth;

    return MetalCord(layer, x, y);
}

size_t DiffusionEngine::getlMetalIdxBegin(size_t layer) const {
    return layer * m_cellGrid2DCount;
}

size_t DiffusionEngine::getMetalIdxBegin(size_t layer, size_t height) const {
    return layer * m_cellGrid2DCount + height * m_cellGridHeight;
}

size_t DiffusionEngine::getMetalIdxEnd(size_t layer) const {
    return (layer+1) * m_cellGrid2DCount;
}

size_t DiffusionEngine::getMetalIdxEnd(size_t layer, size_t height) const {
    return layer * m_cellGrid2DCount + (height+1) * m_cellGridHeight;
}

size_t DiffusionEngine::calViaIdx(size_t l, size_t w) const {
    return (l == 0)? w : m_viaGrid2DAccumlateCount[l-1] + w;
}

size_t DiffusionEngine::calViaIdx(const ViaCord &vc) const {
    return (vc.l == 0)? vc.w :  m_viaGrid2DAccumlateCount[vc.l - 1] + vc.w;
}

ViaCord DiffusionEngine::calViaCord(size_t idx) const {
    if(idx < m_viaGrid2DAccumlateCount[0]) return ViaCord(0, idx);

    // could be O(log n) calling std::upper_bound()
    for(int layer = 0; layer < m_viaLayerCount-1; ++layer){
        if(idx < m_viaGrid2DAccumlateCount[layer]){
            return ViaCord(layer+1, idx - m_viaGrid2DAccumlateCount[layer]);
        }
    }
    // not found
    return ViaCord(SIZE_T_INVALID, SIZE_T_INVALID);
}

size_t DiffusionEngine::getViaIdxBegin(size_t layer) const {
    return (layer == 0)? 0 : m_viaGrid2DAccumlateCount[layer - 1];
}

size_t DiffusionEngine::getViaIdxEnd(size_t layer) const {
    return m_viaGrid2DAccumlateCount[layer];
}

size_t DiffusionEngine::getAllViaIdxEnd() const {
    return m_viaGrid2DAccumlateCount.back();
}

void DiffusionEngine::markPreplacedAndInsertPadsOnCanvas(){

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
    
}

void DiffusionEngine::markObstaclesOnCanvas(){
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

void DiffusionEngine::initialiseGraphWithPreplaced(){

    // transfer the marking of metal layer onto chamber-related metal data structures
    // attributes to fill in
    // signal, fullDirection
    // canvasMetalLayer, canvasMetalX, canvasMetalY
    // up/down/left/right *Cell and idx
    // metalCellNeighbors[]
    
    cellGrid.resize(m_cellGrid3DCount);
    cellGridType.resize(m_cellGrid3DCount, CellType::EMPTY);

    for(int metalLayer = 0; metalLayer < m_metalLayerCount; ++metalLayer){
        for(int j = 0; j < m_gridHeight; ++j){
            for(int i = 0; i < m_gridWidth; ++i){


                size_t cellIndex = calMetalIdx(metalLayer, j, i);
                MetalCell &cell = cellGrid[cellIndex];
                SignalType st = metalLayers[metalLayer].canvas[j][i];

                
                cell.canvasMetalLayer = metalLayer;
                cell.canvasMetalY = j;
                cell.canvasMetalX = i;
                
                if(st != SignalType::EMPTY){
                    cell.signal = st;
                    cellGridType[cellIndex] = (st == SignalType::OBSTACLE)? CellType::OBSTACLES : CellType::PREPLACED;
                }

                // add neighbors
                if(j != (m_cellGridHeight - 1)){
                    SignalType upNieghborSt = metalLayers[metalLayer].canvas[j+1][i];
                    if(upNieghborSt == SignalType::EMPTY || upNieghborSt == st){
                        size_t upCellIdx = cellIndex - m_cellGridWidth;
                        MetalCell *upCellPointer = &cellGrid[upCellIdx];
                        cell.upCell = upCellPointer;
                        cell.metalCellNeighbors.push_back(upCellPointer);
                        cell.upCellIdx = upCellIdx;

                        addDirection(cell.fullDirection, DirFlagAxis::UP);
                        
                    }
                }

                if(j != 0){
                    SignalType downNieghborSt = metalLayers[metalLayer].canvas[j-1][i];
                    if(downNieghborSt == SignalType::EMPTY || downNieghborSt == st){
                        size_t downCellIdx = cellIndex + m_cellGridWidth;
                        MetalCell *downCellPointer = &cellGrid[downCellIdx];
                        cell.downCell = downCellPointer;
                        cell.metalCellNeighbors.push_back(downCellPointer);
                        cell.downCellIdx = downCellIdx;
                        addDirection(cell.fullDirection, DirFlagAxis::DOWN);
                    }
                }

                if(i != 0){
                    SignalType leftNieghborSt = metalLayers[metalLayer].canvas[j][i-1];
                    if(leftNieghborSt == SignalType::EMPTY || leftNieghborSt == st){
                        size_t leftCellIdx = cellIndex - 1;
                        MetalCell *leftCellPointer = &cellGrid[leftCellIdx];
                        cell.leftCell = leftCellPointer;
                        cell.metalCellNeighbors.push_back(leftCellPointer);
                        cell.leftCellIdx = leftCellIdx;
                        addDirection(cell.fullDirection, DirFlagAxis::LEFT);
                    }
                }

                if(i != (m_cellGridWidth - 1)){
                    SignalType rightNieghborSt = metalLayers[metalLayer].canvas[j][i+1];
                    if(rightNieghborSt == SignalType::EMPTY || rightNieghborSt == st){
                        size_t rightCellIdx = cellIndex + 1;
                        MetalCell *rightCellPointer = &cellGrid[rightCellIdx];
                        cell.rightCell = rightCellPointer;
                        cell.metalCellNeighbors.push_back(rightCellPointer);
                        cell.rightCellIdx = rightCellIdx;
                        addDirection(cell.fullDirection, DirFlagAxis::RIGHT);
                    }
                }

            }
        }
    }

    // transfer the marking of via layer onto chamber-related (mdtal/via) data structures
    // for(int viaLayer = 0; viaLayer < m_viaLayerCount; ++viaLayer){
    //     size_t layerPinCount = 0;
    //     for(int j = 0; j < m_pinHeight; ++j){
    //         for(int i = 0; i < m_pinWidth; ++i){
    //             SignalType st = viaLayers[viaLayer].canvas[j][i];
    //             if(st == SignalType::OBSTACLE) continue;

    //             layerPinCount++;
    //             viaGrid.resize(layerPinCount);
    //             ViaCell &cell = viaGrid.back();

    //             enum CellType cellType;

    //             if(st == SignalType::EMPTY){
    //                 viaGridType.push_back(CellType::EMPTY);
    //                 cellType =  CellType::EMPTY;

    //             }else{ // preplaced power signals
    //                 viaGridType.push_back(CellType::PREPLACED);
    //                 cellType = CellType::PREPLACED;
    //                 cell.signal = st;
    //             }

                
    //             size_t upLLCellIdx = calCellIdx(viaLayer, j, i);
    //             enum CellType upLLCellType = cellGridType[upLLCellIdx];
    //             if(upLLCellType != CellType::OBSTACLES){
    //                 SignalType upLLCellSt = this->metalLayer[viaLayer].canvas[j][i];
                    
    //                 if((cellType != CellType::PREPLACED) || (upLLCellType != CellType::PREPLACED) || (st == upLLCellSt)){
    //                     cell.upLLCell = &cellGrid[upLLCellIdx]
    //                     cell.upLLCellIdx = upLLCellIdx;
    //                     addDirection(cell.fullDirection, DirFlagViaAxis::UPLL);
    //                 }

    //             }
    //             // to do other 7
    //             size_t upLRCellIdx = upCellIdx + 1;
    //             enum CellType upLRCellType = cellGridTupe[upLRCellIdx];
    //             if(upLRCellType != CellType::OBSTACLES){
    //                 // todo..
    //             }

    //             cell.direction = cell.fulldirection;

    //         }
    //     }
    //     m_viaGrid2DCount.push_back(layerPinCount);
    //     m_viaGrid2DAccumlateCount.push_back((viaLayer == 0)? layerPinCount : m_viaGrid2DAccumlateCount.back() + layerPinCount);

    // }

    // // labeling of the grid
    // cellGridLabel.resize(m_cellGrid3DCount, 0);
}

void DiffusionEngine::fillEnclosedRegions(){
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







