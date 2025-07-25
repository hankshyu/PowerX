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
#include <queue>
#include <algorithm>
#include <limits>
// 2. Boost Library:

// 3. Texo Library:
#include "diffusionEngine.hpp"
#include "timeProfiler.hpp"


DiffusionEngine::DiffusionEngine(const std::string &fileName): PowerDistributionNetwork(fileName) {
    this->m_metalGridLayers = m_metalLayerCount;
    this->m_metalGridWidth = m_gridWidth;
    this->m_metalGridHeight = m_gridHeight;
    this->m_metalGrid2DCount = m_gridWidth * m_gridHeight;
    this->m_metalGrid3DCount = m_metalGrid2DCount * m_metalLayerCount;

    this->m_viaGridLayers = m_viaLayerCount;

    // calculate current budget
    double totalCurrentBudget = 0;
    for(auto &[st , us] : this->uBump.signalTypeToInstances){
        if(POWER_SIGNAL_SET.count(st) == 0) continue;
        for(const std::string &inst  : us){
            double currentRequirement = this->uBump.instanceToBallOutMap[inst]->getMaxCurrent();
            totalCurrentBudget += currentRequirement;
            this->currentBudget[st] += currentRequirement;
        }
    }
    for(auto &[st, value] : this->currentBudget){
        value /= totalCurrentBudget;
    }

}

size_t DiffusionEngine::calMetalIdx(size_t layer, size_t height, size_t width) const {
    return layer * m_metalGrid2DCount + height * m_metalGridWidth + width;
}

size_t DiffusionEngine::calMetalIdx(const MetalCord &cc) const {
    return cc.l * m_metalGrid2DCount + cc.h * m_metalGridWidth + cc.w;
}

MetalCord DiffusionEngine::calMetalCord(size_t idx) const {
    size_t layer = idx / m_metalGrid2DCount;
    size_t rem   = idx % m_metalGrid2DCount;

    size_t y = rem / m_metalGridWidth;
    size_t x = rem % m_metalGridWidth;

    return MetalCord(layer, x, y);
}

size_t DiffusionEngine::getMetalIdxBegin(size_t layer) const {
    return layer * m_metalGrid2DCount;
}

size_t DiffusionEngine::getMetalIdxBegin(size_t layer, size_t height) const {
    return layer * m_metalGrid2DCount + height * m_metalGridWidth;
}

size_t DiffusionEngine::getMetalIdxEnd(size_t layer) const {
    return (layer+1) * m_metalGrid2DCount;
}

size_t DiffusionEngine::getMetalIdxEnd(size_t layer, size_t height) const {
    return layer * m_metalGrid2DCount + (height+1) * m_metalGridWidth;
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

    auto inMetalCanvas = [&](int y, int x){
        return (y >= 0) && (y < m_metalGridHeight) && (x >= 0) && (x < m_metalGridWidth);
    };

    for(int viaLayer = 0; viaLayer < m_viaLayerCount; ++viaLayer){
        for(int j = 0; j < m_pinHeight; ++j){
            for(int i = 0; i < m_pinWidth; ++i){
                SignalType st = viaLayers[viaLayer].canvas[j][i];
                if((st == SignalType::GROUND) || (st == SignalType::SIGNAL) || (st == SignalType::OVERLAP) || (st == SignalType::OBSTACLE)){
                    viaLayers[viaLayer].canvas[j][i] = SignalType::OBSTACLE;
                    continue;
                }

                bool LLinCanvas = inMetalCanvas(j-1, i-1);
                bool LRinCanvas = inMetalCanvas(j-1, i);
                bool ULinCanvas = inMetalCanvas(j, i-1);
                bool URinCanvas = inMetalCanvas(j, i);

                bool topLLIsObstacles = (LLinCanvas && metalLayers[viaLayer].canvas[j-1][i-1] == SignalType::OBSTACLE);
                bool topLRIsObstacles = (LRinCanvas && metalLayers[viaLayer].canvas[j-1][i] == SignalType::OBSTACLE);
                bool topULIsObstacles = (ULinCanvas && metalLayers[viaLayer].canvas[j][i-1] == SignalType::OBSTACLE);
                bool topURIsObstacles = (URinCanvas && metalLayers[viaLayer].canvas[j][i] == SignalType::OBSTACLE);
                
                if(topLLIsObstacles || topLRIsObstacles || topULIsObstacles || topURIsObstacles){
                    viaLayers[viaLayer].canvas[j][i] = SignalType::OBSTACLE;
                    continue;
                }
                
                bool downLLIsObstacles = (LLinCanvas && metalLayers[viaLayer+1].canvas[j-1][i-1] == SignalType::OBSTACLE);
                bool downLRIsObstacles = (LRinCanvas && metalLayers[viaLayer+1].canvas[j-1][i] == SignalType::OBSTACLE);
                bool downULIsObstacles = (ULinCanvas && metalLayers[viaLayer+1].canvas[j][i-1] == SignalType::OBSTACLE);
                bool downURIsObstacles = (URinCanvas && metalLayers[viaLayer+1].canvas[j][i] == SignalType::OBSTACLE);
                
                if(downLLIsObstacles || downLRIsObstacles || downULIsObstacles || downURIsObstacles){
                    viaLayers[viaLayer].canvas[j][i] = SignalType::OBSTACLE;
                    continue;
                }
            }
        }
    }


}

void DiffusionEngine::initialiseGraphWithPreplaced(){

    // transfer the marking of metal layer onto chamber-related metal data structures, attributes to fill:
    // signal, fullDirection
    // metalGridType[idx]
    // canvasMetalLayer, canvasMetalX, canvasMetalY
    // up/down/left/right *MetalCell and idx
    // metalCellNeighbors[]
    
    metalGrid.resize(m_metalGrid3DCount);

    for(int metalLayer = 0; metalLayer < m_metalLayerCount; ++metalLayer){
        for(int j = 0; j < m_gridHeight; ++j){
            for(int i = 0; i < m_gridWidth; ++i){

                size_t cellIndex = calMetalIdx(metalLayer, j, i);
                MetalCell &cell = metalGrid[cellIndex];
                SignalType st = metalLayers[metalLayer].canvas[j][i];
                
                cell.index = cellIndex;
                cell.metalViaType = DiffusionChamberType::METAL;
                cell.canvasLayer = metalLayer;
                cell.canvasY = j;
                cell.canvasX = i;
                
                if(st != SignalType::EMPTY){
                    cell.signal = st;
                    cell.type = (st == SignalType::OBSTACLE)? CellType::OBSTACLES : CellType::PREPLACED;
                }

                // add neighbors
                if(j != (m_metalGridHeight - 1)){
                    SignalType northNieghborSt = metalLayers[metalLayer].canvas[j+1][i];
                    size_t northCellIdx = cellIndex + m_metalGridWidth;
                    MetalCell *northCellPointer = &metalGrid[northCellIdx];
                    cell.northCell = northCellPointer;
                    cell.northCellIdx = northCellIdx;
                    addDirection(cell.fullDirection, DirFlagAxis::NORTH);

                }

                if(j != 0){
                    SignalType southNieghborSt = metalLayers[metalLayer].canvas[j-1][i];
                    size_t southCellIdx = cellIndex - m_metalGridWidth;
                    MetalCell *southCellPointer = &metalGrid[southCellIdx];
                    cell.southCell = southCellPointer;
                    cell.southCellIdx = southCellIdx;
                    addDirection(cell.fullDirection, DirFlagAxis::SOUTH);

                }

                if(i != 0){
                    SignalType westNieghborSt = metalLayers[metalLayer].canvas[j][i-1];
                    size_t westCellIdx = cellIndex - 1;
                    MetalCell *westCellPointer = &metalGrid[westCellIdx];
                    cell.westCell = westCellPointer;
                    cell.westCellIdx = westCellIdx;
                    addDirection(cell.fullDirection, DirFlagAxis::WEST);

                }

                if(i != (m_metalGridWidth - 1)){
                    SignalType eastNieghborSt = metalLayers[metalLayer].canvas[j][i+1];
                    size_t eastCellIdx = cellIndex + 1;
                    MetalCell *eastCellPointer = &metalGrid[eastCellIdx];
                    cell.eastCell = eastCellPointer;
                    cell.eastCellIdx = eastCellIdx;
                    addDirection(cell.fullDirection, DirFlagAxis::EAST);

                }
            }
        }
    }

    // transfer the marking of via layer onto chamber-related (mdtal/via) data structures, attributes to fill:
    // signal, fullDirection
    // viaGridType[idx]
    // canvasViaLayer, canvasViaX, canvasViaY
    // upLLCell, upULCell upLRCell upURCell             *MetalCell and idx
    // downLLCell, downULCell, downLRCell, downURCell   *MetalCell and idx
    // neighbors[]
    
    
    size_t cellIndex = 0;
    for(int viaLayer = 0; viaLayer < m_viaLayerCount; ++viaLayer){
        for(int j = 0; j < m_pinHeight; ++j){
            for(int i = 0; i < m_pinWidth; ++i){
                SignalType st = viaLayers[viaLayer].canvas[j][i];
                if(st == SignalType::OBSTACLE) continue;
                
                SignalType occupiedSignal = st;
                bool hasOccupiedSignal = (st != SignalType::EMPTY);

                // check if this via "must be created"
                size_t upLLCellIdx = calMetalIdx(viaLayer, j-1, i-1);
                SignalType upLLCellSt = metalLayers[viaLayer].canvas[j-1][i-1];
                if(upLLCellSt == SignalType::OBSTACLE) continue;
                bool upLLCellStNotEmpty = (upLLCellSt != SignalType::EMPTY && upLLCellSt != occupiedSignal);
                if(hasOccupiedSignal && upLLCellStNotEmpty) continue;
                else if(upLLCellStNotEmpty){
                    hasOccupiedSignal = true;
                    occupiedSignal = upLLCellSt;
                }

                size_t upLRCellIdx = upLLCellIdx + 1;
                SignalType upLRCellSt = metalLayers[viaLayer].canvas[j-1][i];
                if(upLRCellSt == SignalType::OBSTACLE) continue;
                bool upLRCellStNotEmpty = (upLRCellSt != SignalType::EMPTY && upLRCellSt != occupiedSignal);
                if(hasOccupiedSignal && upLRCellStNotEmpty) continue;
                else if(upLRCellStNotEmpty){
                    hasOccupiedSignal = true;
                    occupiedSignal = upLRCellSt;
                }

                size_t upULCellIdx = upLLCellIdx + m_metalGridWidth;
                SignalType upULCellSt = metalLayers[viaLayer].canvas[j][i-1];
                if(upULCellSt == SignalType::OBSTACLE) continue;
                bool upULCellStNotEmpty = (upULCellSt != SignalType::EMPTY && upULCellSt != occupiedSignal);
                if(hasOccupiedSignal && upULCellStNotEmpty) continue;
                else if(upULCellStNotEmpty){
                    hasOccupiedSignal = true;
                    occupiedSignal = upULCellSt;
                }

                size_t upURCellIdx = upULCellIdx + 1;
                SignalType upURCellSt = metalLayers[viaLayer].canvas[j][i];
                if(upURCellSt == SignalType::OBSTACLE) continue;
                bool upURCellStNotEmpty = (upURCellSt != SignalType::EMPTY && upURCellSt != occupiedSignal);
                if(hasOccupiedSignal && upURCellStNotEmpty) continue;
                else if(upURCellStNotEmpty){
                    hasOccupiedSignal = true;
                    occupiedSignal = upURCellSt;
                }


                size_t downLLCellIdx = upLLCellIdx + m_metalGrid2DCount;
                SignalType downLLCellSt = metalLayers[viaLayer+1].canvas[j-1][i-1];
                if(downLLCellSt == SignalType::OBSTACLE) continue;
                bool downLLCellStNotEmpty = (downLLCellSt != SignalType::EMPTY && downLLCellSt != occupiedSignal);
                if(hasOccupiedSignal && downLLCellStNotEmpty) continue;
                else if(downLLCellStNotEmpty){
                    hasOccupiedSignal = true;
                    occupiedSignal = downLLCellSt;
                }

                size_t downLRCellIdx = downLLCellIdx + 1;
                SignalType downLRCellSt = metalLayers[viaLayer+1].canvas[j-1][i];
                if(downLRCellSt == SignalType::OBSTACLE) continue;
                bool downLRCellStNotEmpty = (downLRCellSt != SignalType::EMPTY && downLRCellSt != occupiedSignal);
                if(hasOccupiedSignal && downLRCellStNotEmpty) continue;
                else if(downLRCellStNotEmpty){
                    hasOccupiedSignal = true;
                    occupiedSignal = downLRCellSt;
                }

                size_t downULCellIdx = downLLCellIdx + m_metalGridWidth;
                SignalType downULCellSt = metalLayers[viaLayer+1].canvas[j][i-1];
                if(downULCellSt == SignalType::OBSTACLE) continue;
                bool downULCellStNotEmpty = (downULCellSt != SignalType::EMPTY && downULCellSt != occupiedSignal);
                if(hasOccupiedSignal && downULCellStNotEmpty) continue;
                else if(downULCellStNotEmpty){
                    hasOccupiedSignal = true;
                    occupiedSignal = downULCellSt;
                }

                size_t downURCellIdx = downULCellIdx + 1;
                SignalType downURCellSt = metalLayers[viaLayer+1].canvas[j][i];
                if(downURCellSt == SignalType::OBSTACLE) continue;
                bool downURCellStNotEmpty = (downURCellSt != SignalType::EMPTY && downURCellSt != occupiedSignal);
                if(hasOccupiedSignal && downURCellStNotEmpty) continue;
                else if(downURCellStNotEmpty){
                    hasOccupiedSignal = true;
                    occupiedSignal = downURCellSt;
                } 
                
                // use default constructor
                viaGrid.emplace_back();
                ViaCell &cell = viaGrid.back();
                
                cell.index = cellIndex;
                cell.metalViaType = DiffusionChamberType::VIA;
                cell.canvasLayer = static_cast<len_t>(viaLayer);
                cell.canvasY = static_cast<len_t>(j);
                cell.canvasX = static_cast<len_t>(i);

                CellType cellType;
                if(st == SignalType::EMPTY){
                    cellType = CellType::EMPTY;

                }else{ // preplaced power signals
                    cellType = CellType::PREPLACED;
                    cell.signal = st;
                }
                cell.type = cellType;

                /* Link Up direction LL cell */

                MetalCell *upLLCellPointer = &metalGrid[upLLCellIdx];
                CellType upLLCellType = upLLCellPointer->type;
                
                // modify this via cell
                cell.upLLCell = upLLCellPointer;
                cell.upLLCellIdx = upLLCellIdx; 
                addDirection(cell.fullDirection, DirFlagViaAxis::UPLL);

                // modify the metal cell via cell is linking to
                upLLCellPointer->downCellIdx = cellIndex;
                addDirection(upLLCellPointer->fullDirection, DirFlagAxis::DOWN);


                /* Link Up direction LR cell */

                MetalCell *upLRCellPointer = &metalGrid[upLRCellIdx];
                CellType upLRCellType = upLRCellPointer->type;

                // modify this via cell
                cell.upLRCell = upLRCellPointer;
                cell.upLRCellIdx = upLRCellIdx;
                addDirection(cell.fullDirection, DirFlagViaAxis::UPLR);

                // modify the metal cell via cell is linking to
                upLRCellPointer->downCellIdx = cellIndex;
                addDirection(upLRCellPointer->fullDirection, DirFlagAxis::DOWN);


                /* Link Up direction UL cell */

                MetalCell *upULCellPointer = &metalGrid[upULCellIdx];
                CellType upULCellType = upULCellPointer->type;

                // modify this via cell
                cell.upULCell = upULCellPointer;
                cell.upULCellIdx = upULCellIdx;
                addDirection(cell.fullDirection, DirFlagViaAxis::UPUL);

                // modify the metal cell via cell is linking to
                upULCellPointer->downCellIdx = cellIndex;
                addDirection(upULCellPointer->fullDirection, DirFlagAxis::DOWN);

                /* Link Up direction UR cell */

                MetalCell *upURCellPointer = &metalGrid[upURCellIdx];
                CellType upURCellType = upURCellPointer->type;

                // modify this via cell
                cell.upURCell = upURCellPointer;
                cell.upURCellIdx = upURCellIdx;
                addDirection(cell.fullDirection, DirFlagViaAxis::UPUR);

                // modify the metal cell via cell is linking to
                upURCellPointer->downCellIdx = cellIndex;
                addDirection(upURCellPointer->fullDirection, DirFlagAxis::DOWN);


                /* Link Down direction LL cell */

                MetalCell *downLLCellPointer = &metalGrid[downLLCellIdx];
                CellType downLLCellType = downLLCellPointer->type;

                // modify this via cell
                cell.downLLCell = downLLCellPointer;
                cell.downLLCellIdx = downLLCellIdx;
                addDirection(cell.fullDirection, DirFlagViaAxis::DOWNLL);

                // modify the metal cell via cell is linking to
                downLLCellPointer->upCellIdx = cellIndex;
                addDirection(downLLCellPointer->fullDirection, DirFlagAxis::UP);


                /* Link Down direction LR cell */

                MetalCell *downLRCellPointer = &metalGrid[downLRCellIdx];
                CellType downLRCellType = downLRCellPointer->type;

                // modify this via cell
                cell.downLRCell = downLRCellPointer;
                cell.downLRCellIdx = downLRCellIdx;
                addDirection(cell.fullDirection, DirFlagViaAxis::DOWNLR);

                // modify the metal cell via cell is linking to
                downLRCellPointer->upCellIdx = cellIndex;
                addDirection(downLRCellPointer->fullDirection, DirFlagAxis::UP);


                /* Link Down direction UL cell */

                MetalCell *downULCellPointer = &metalGrid[downULCellIdx];
                CellType downULCellType = downULCellPointer->type;

                // modify this via cell
                cell.downULCell = downULCellPointer;
                cell.downULCellIdx = downULCellIdx;
                addDirection(cell.fullDirection, DirFlagViaAxis::DOWNUL);

                // modify the metal cell via cell is linking to
                downULCellPointer->upCellIdx = cellIndex;
                addDirection(downULCellPointer->fullDirection, DirFlagAxis::UP);


                /* Link Down direction UR cell */

                MetalCell *downURCellPointer = &metalGrid[downURCellIdx];
                CellType downURCellType = downURCellPointer->type;

                // modify this via cell
                cell.downURCell = downURCellPointer;
                cell.downURCellIdx = downURCellIdx;
                addDirection(cell.fullDirection, DirFlagViaAxis::DOWNUR);

                // modify the metal cell via cell is linking to
                downURCellPointer->upCellIdx = cellIndex;
                addDirection(downURCellPointer->fullDirection, DirFlagAxis::UP);


                // increment cellIndex for the next available
                cellIndex++;
            }
        }
        
        size_t prevAccum = (viaLayer == 0) ? 0 : m_viaGrid2DAccumlateCount[viaLayer - 1];
        m_viaGrid2DAccumlateCount.push_back(cellIndex);
        m_viaGrid2DCount.push_back(cellIndex - prevAccum);
    }

    // Debug 2025/07/13: only link MetalCell* or ViaCell* when their holder array's size concludes
    for(ViaCell &vc : viaGrid){
        ViaCell *vcPointer = &vc;
        
        vc.upLLCell->downCell = vcPointer;
        vc.upLRCell->downCell = vcPointer;
        vc.upULCell->downCell = vcPointer;
        vc.upURCell->downCell = vcPointer;

        vc.downLLCell->upCell = vcPointer;
        vc.downLRCell->upCell = vcPointer;
        vc.downULCell->upCell = vcPointer;
        vc.downURCell->upCell = vcPointer;
    }
}

void DiffusionEngine::fillEnclosedRegions() {
    for (int layer = 0; layer < m_metalGridLayers; ++layer) {
        std::vector<std::vector<bool>> visited(m_metalGridHeight, std::vector<bool>(m_metalGridWidth, false));

        for (int y = 0; y < m_metalGridHeight; ++y) {
            for (int x = 0; x < m_metalGridWidth; ++x) {
                size_t cellIdx = calMetalIdx(layer, y, x);
                MetalCell *cellPointer = &metalGrid[cellIdx];

                if (cellPointer->type != CellType::EMPTY || visited[y][x])
                    continue;

                SignalType cellSignalType = cellPointer->signal;

                std::queue<Cord> q;
                std::vector<Cord> region;
                std::unordered_set<SignalType> borderSignals;

                q.push(Cord(x, y));
                visited[y][x] = true;
                region.emplace_back(x, y);

                while (!q.empty()) {
                    Cord c = q.front(); q.pop();
                    int cx = c.x();
                    int cy = c.y();

                    MetalCell &mtc = metalGrid[calMetalIdx(layer, cy, cx)];

                    // Unrolled neighbor checks
                    if (mtc.northCell) {
                        MetalCell *mc = mtc.northCell;
                        SignalType mcSignal = mc->signal;
                        int nx = mc->canvasX;
                        int ny = mc->canvasY;
                        if (mcSignal == SignalType::EMPTY && !visited[ny][nx]) {
                            visited[ny][nx] = true;
                            q.emplace(nx, ny);
                            region.emplace_back(nx, ny);
                        } else if (mcSignal != SignalType::EMPTY && mcSignal != SignalType::OBSTACLE) {
                            borderSignals.insert(mcSignal);
                        }
                    }

                    if (mtc.southCell) {
                        MetalCell *mc = mtc.southCell;
                        SignalType mcSignal = mc->signal;
                        int nx = mc->canvasX;
                        int ny = mc->canvasY;
                        if (mcSignal == SignalType::EMPTY && !visited[ny][nx]) {
                            visited[ny][nx] = true;
                            q.emplace(nx, ny);
                            region.emplace_back(nx, ny);
                        } else if (mcSignal != SignalType::EMPTY && mcSignal != SignalType::OBSTACLE) {
                            borderSignals.insert(mcSignal);
                        }
                    }

                    if (mtc.eastCell) {
                        MetalCell *mc = mtc.eastCell;
                        SignalType mcSignal = mc->signal;
                        int nx = mc->canvasX;
                        int ny = mc->canvasY;
                        if (mcSignal == SignalType::EMPTY && !visited[ny][nx]) {
                            visited[ny][nx] = true;
                            q.emplace(nx, ny);
                            region.emplace_back(nx, ny);
                        } else if (mcSignal != SignalType::EMPTY && mcSignal != SignalType::OBSTACLE) {
                            borderSignals.insert(mcSignal);
                        }
                    }

                    if (mtc.westCell) {
                        MetalCell *mc = mtc.westCell;
                        SignalType mcSignal = mc->signal;
                        int nx = mc->canvasX;
                        int ny = mc->canvasY;
                        if (mcSignal == SignalType::EMPTY && !visited[ny][nx]) {
                            visited[ny][nx] = true;
                            q.emplace(nx, ny);
                            region.emplace_back(nx, ny);
                        } else if (mcSignal != SignalType::EMPTY && mcSignal != SignalType::OBSTACLE) {
                            borderSignals.insert(mcSignal);
                        }
                    }
                }

                // Only fill if not leaking and surrounded by one signal type
                if (borderSignals.size() == 1) {
                    SignalType fillType = *borderSignals.begin();
                    for (const Cord &p : region) {
                        size_t fillCellIdx = calMetalIdx(layer, p.y(), p.x());
                        MetalCell &fillCell = metalGrid[fillCellIdx];
                        fillCell.type = CellType::MARKED;
                        fillCell.signal = fillType;
                    }
                }
            }
        }
    }
}

void DiffusionEngine::markHalfOccupiedMetalsAndPins(){
    // no vias at this stage would have obstacles at it's edge
    // and no marked pins
    for(ViaCell &vc : this->viaGrid){
        SignalType vcSignal = vc.signal;
        CellType vcCellType = vc.type;
        
        MetalCell *vcUpLLCell = vc.upLLCell;
        MetalCell *vcUpULCell = vc.upULCell;
        MetalCell *vcUpLRCell = vc.upLRCell;
        MetalCell *vcUpURCell = vc.upURCell;
        MetalCell *vcDownLLCell = vc.downLLCell;
        MetalCell *vcDownULCell = vc.downULCell;
        MetalCell *vcDownLRCell = vc.downLRCell;
        MetalCell *vcDownURCell = vc.downURCell;


        std::unordered_set<SignalType> allSignalTypes = {
            vcSignal, 
            vcUpLLCell->signal, vcUpULCell->signal, vcUpLRCell->signal, vcUpURCell->signal,
            vcDownLLCell->signal, vcDownULCell->signal, vcDownLRCell->signal, vcDownURCell->signal
        };
        allSignalTypes.erase(SignalType::EMPTY);

        bool hasPreplaced = (vcCellType == CellType::PREPLACED);
        bool vcUpLLCellIsPreplaced = vcUpLLCell->type == CellType::PREPLACED;
        bool vcUpULCellIsPreplaced = vcUpULCell->type == CellType::PREPLACED;
        bool vcUpLRCellIsPreplaced = vcUpLRCell->type == CellType::PREPLACED;
        bool vcUpURCellIsPreplaced = vcUpURCell->type == CellType::PREPLACED;
        bool vcDownLLCellIsPreplaced = vcDownLLCell->type == CellType::PREPLACED;
        bool vcDownULCellIsPreplaced = vcDownULCell->type == CellType::PREPLACED;
        bool vcDownLRCellIsPreplaced = vcDownLRCell->type == CellType::PREPLACED;
        bool vcDownURCellIsPreplaced = vcDownURCell->type == CellType::PREPLACED;

        hasPreplaced |= (vcUpLLCellIsPreplaced || vcUpULCellIsPreplaced || vcUpLRCellIsPreplaced || vcUpURCellIsPreplaced ||
                        vcDownLLCellIsPreplaced || vcDownULCellIsPreplaced || vcDownLRCellIsPreplaced || vcDownURCellIsPreplaced);
        
        bool unoSignalType =( allSignalTypes.size() == 1);
        if(hasPreplaced && unoSignalType){
            SignalType fillSig = *allSignalTypes.begin();
            if(!vcUpLLCellIsPreplaced){
                vcUpLLCell->signal = fillSig;
                vcUpLLCell->type = CellType::MARKED;
            }
            if(!vcUpULCellIsPreplaced){
                vcUpULCell->signal = fillSig;
                vcUpULCell->type = CellType::MARKED;
            }
            if(!vcUpLRCellIsPreplaced){
                vcUpLRCell->signal = fillSig;
                vcUpLRCell->type = CellType::MARKED;
            }
            if(!vcUpURCellIsPreplaced){
                vcUpURCell->signal = fillSig;
                vcUpURCell->type = CellType::MARKED;
            }

            if(!vcDownLLCellIsPreplaced){
                vcDownLLCell->signal = fillSig;
                vcDownLLCell->type = CellType::MARKED;
            }
            if(!vcDownULCellIsPreplaced){
                vcDownULCell->signal = fillSig;
                vcDownULCell->type = CellType::MARKED;
            }
            if(!vcDownLRCellIsPreplaced){
                vcDownLRCell->signal = fillSig;
                vcDownLRCell->type = CellType::MARKED;
            }
            if(!vcDownURCellIsPreplaced){
                vcDownURCell->signal = fillSig;
                vcDownURCell->type = CellType::MARKED;
            }

        }


    }


}

void DiffusionEngine::linkNeighbors(){
    
    // a link goes from empty -> empty
    for(MetalCell &mc : this->metalGrid){
        mc.neighbors.clear();
    }
    for(ViaCell &vc : this->viaGrid){
        vc.neighbors.clear();
    }

    // link 2D metal layer linkings
    for(MetalCell &mc : this->metalGrid){
        if(mc.type != CellType::EMPTY) continue;

        MetalCell *mcPointer = &mc;

        MetalCell *mcNorthCell = mc.northCell;
        MetalCell *mcSouthCell = mc.southCell;
        MetalCell *mcEastCell = mc.eastCell;
        MetalCell *mcWestCell = mc.westCell;

        ViaCell *mcUpCell = mc.upCell;
        ViaCell *mcDownCell = mc.downCell;

        if((mcNorthCell != nullptr) && (mcNorthCell->type == CellType::EMPTY)){
            mc.neighbors.push_back(mcNorthCell);
        }
        if((mcSouthCell != nullptr) && (mcSouthCell->type == CellType::EMPTY)){
            mc.neighbors.push_back(mcSouthCell);
        }
        if((mcEastCell != nullptr) && (mcEastCell->type == CellType::EMPTY)){
            mc.neighbors.push_back(mcEastCell);
        }
        if((mcWestCell != nullptr) && (mcWestCell->type == CellType::EMPTY)){
            mc.neighbors.push_back(mcWestCell);
        }

    }

    // link via related linkings
    for(ViaCell &vc : this->viaGrid){
        if(vc.type != CellType::EMPTY) continue;
        ViaCell *vcPointer = &vc;

        MetalCell *vcUpLLCell = vc.upLLCell;
        MetalCell *vcUpULCell = vc.upULCell;
        MetalCell *vcUpLRCell = vc.upLRCell;
        MetalCell *vcUpURCell = vc.upURCell;
        MetalCell *vcDownLLCell = vc.downLLCell;
        MetalCell *vcDownULCell = vc.downULCell;
        MetalCell *vcDownLRCell = vc.downLRCell;
        MetalCell *vcDownURCell = vc.downURCell;
        
        if((vcUpLLCell != nullptr) && (vcUpLLCell->type == CellType::EMPTY)){
            vc.neighbors.push_back(vcUpLLCell);
            vcUpLLCell->neighbors.push_back(vcPointer);
        }
        if((vcUpULCell != nullptr) && (vcUpULCell->type == CellType::EMPTY)){
            vc.neighbors.push_back(vcUpULCell);
            vcUpULCell->neighbors.push_back(vcPointer);
        }
        if((vcUpLRCell != nullptr) && (vcUpLRCell->type == CellType::EMPTY)){
            vc.neighbors.push_back(vcUpLRCell);
            vcUpLRCell->neighbors.push_back(vcPointer);
        }
        if((vcUpURCell != nullptr) && (vcUpURCell->type == CellType::EMPTY)){
            vc.neighbors.push_back(vcUpURCell);
            vcUpURCell->neighbors.push_back(vcPointer);
        }

        if((vcDownLLCell != nullptr) && (vcDownLLCell->type == CellType::EMPTY)){
            vc.neighbors.push_back(vcDownLLCell);
            vcDownLLCell->neighbors.push_back(vcPointer);
        }
        if((vcDownULCell != nullptr) && (vcDownULCell->type == CellType::EMPTY)){
            vc.neighbors.push_back(vcDownULCell);
            vcDownULCell->neighbors.push_back(vcPointer);
        }
        if((vcDownLRCell != nullptr) && (vcDownLRCell->type == CellType::EMPTY)){
            vc.neighbors.push_back(vcDownLRCell);
            vcDownLRCell->neighbors.push_back(vcPointer);
        }
        if((vcDownURCell != nullptr) && (vcDownURCell->type == CellType::EMPTY)){
            vc.neighbors.push_back(vcDownURCell);
            vcDownURCell->neighbors.push_back(vcPointer);
        } 
    }
}

void DiffusionEngine::writeBackToPDN(){
    for(size_t layer = 0; layer < m_metalGridLayers; ++layer){
        for(size_t y = 0; y < m_metalGridHeight; ++y){
            for(size_t x = 0; x < m_metalGridWidth; ++x){
                if(metalLayers[layer].canvas[y][x] == SignalType::EMPTY){
                    metalLayers[layer].canvas[y][x] = metalGrid[calMetalIdx(layer, y, x)].signal;
                }
            }
        }
    }

    for(size_t layer = 0; layer < m_viaLayerCount; ++ layer){
        for(int idx = 0; idx < m_viaGrid2DCount[layer]; ++idx){
            ViaCell &vc = viaGrid[calViaIdx(layer, idx)];
            if(viaLayers[layer].canvas[vc.canvasY][vc.canvasX] == SignalType::EMPTY){
                viaLayers[layer].canvas[vc.canvasY][vc.canvasX] = vc.signal;
            }
        }
    }




}

void DiffusionEngine::runDiffusionTop(double diffusionRate){

    std::vector<std::string> timeSpan = {
       "Initialize",
       "Mark PP Pads Canvas",
       "Mark Obstacles Canvas",
       "Init Graph with PP",
       "Fill Enclosed Region",
       "Mark h-Occupied Pins",
       "Link Neighbors",
       "Initialize Index",
       "Place particles",
       "Diffuse"
    };

    TimeProfiler timeProfiler;

    timeProfiler.startTimer(timeSpan[1]);
    markPreplacedAndInsertPadsOnCanvas();
    timeProfiler.pauseTimer(timeSpan[1]);

    timeProfiler.startTimer(timeSpan[2]);
    markObstaclesOnCanvas();
    timeProfiler.pauseTimer(timeSpan[2]);

    timeProfiler.startTimer(timeSpan[3]);
    initialiseGraphWithPreplaced();
    timeProfiler.pauseTimer(timeSpan[3]);

    timeProfiler.startTimer(timeSpan[4]);
    fillEnclosedRegions();
    timeProfiler.pauseTimer(timeSpan[4]);

    timeProfiler.startTimer(timeSpan[5]);
    markHalfOccupiedMetalsAndPins();
    timeProfiler.pauseTimer(timeSpan[5]);

    timeProfiler.startTimer(timeSpan[6]);
    linkNeighbors();
    timeProfiler.pauseTimer(timeSpan[6]);

    timeProfiler.startTimer(timeSpan[7]);
    initialiseIndexing();
    timeProfiler.pauseTimer(timeSpan[7]);

    timeProfiler.startTimer(timeSpan[8]);
    placeDiffusionParticles();
    timeProfiler.pauseTimer(timeSpan[8]);

    timeProfiler.startTimer(timeSpan[9]);
    diffuse(diffusionRate);
    timeProfiler.pauseTimer(timeSpan[9]);

    // visualiseDiffusionEngineMetal(dse, 0, "outputs/dse_m0.txt");
    // visualiseDiffusionEngineMetal(dse, 1, "outputs/dse_m1.txt");
    // visualiseDiffusionEngineMetal(dse, 2, "outputs/dse_m2.txt");

    // visualiseDiffusionEngineVia(dse, 0, "outputs/dse_v0.txt");
    // visualiseDiffusionEngineVia(dse, 1, "outputs/dse_v1.txt");

    // visualiseDiffusionEngineMetalAndVia(dse, 0, 0, "outputs/dse_m0_v0.txt");
    // visualiseDiffusionEngineMetalAndVia(dse, 1, 0, "outputs/dse_m1_v0.txt");
    // visualiseDiffusionEngineMetalAndVia(dse, 1, 1, "outputs/dse_m1_v1.txt");
    // visualiseDiffusionEngineMetalAndVia(dse, 2, 1, "outputs/dse_m2_v1.txt");
}

int DiffusionEngine::initialiseIndexing(){
    cellLabelToSigType.push_back(SignalType::EMPTY);
    CellLabel labelIdx = 1;

    size_t metalGridSize = metalGrid.size();
    size_t viaGridsize = viaGrid.size();

    this->metalGridLabel = std::vector<CellLabel>(metalGridSize, CELL_LABEL_EMPTY);
    this->viaGridLabel = std::vector<CellLabel>(viaGridsize, CELL_LABEL_EMPTY);
    
    // a connected component with the same signal type (!emtpy) would own the same idx,

    std::vector<bool> metalVisited(metalGridSize, false);
    std::vector<bool> viaVisited(viaGridsize, false);
    // a pass of cellidx is sufficient to iterate all nodes, because no cell 
    for(size_t cellIdx = 0; cellIdx < (metalGridSize + viaGridsize); ++cellIdx){
        
        bool processingMetal = (cellIdx < metalGridSize);

        size_t viaIdx = cellIdx - metalGridSize;
        
        SignalType paintingType;
        DiffusionChamber *dcPointer;
        
        if(processingMetal){
            MetalCell &mc = this->metalGrid[cellIdx];
            if((mc.type == CellType::EMPTY) || metalVisited[cellIdx]) continue;
            dcPointer = &mc;
            paintingType = mc.signal;
        }else{
            ViaCell &vc = this->viaGrid[viaIdx];
            if((vc.type == CellType::EMPTY) || viaVisited[viaIdx]) continue;
            dcPointer = &vc;
            paintingType = vc.signal;
        }
        

        std::queue<DiffusionChamber *> q;
        q.emplace(dcPointer);

        if(processingMetal){
            metalVisited[cellIdx] = true;
            this->metalGridLabel[cellIdx] = labelIdx;
        }else{
            viaVisited[viaIdx] = true;
            this->viaGridLabel[viaIdx] = labelIdx;
        }


        while(!q.empty()){
            DiffusionChamber *dc = q.front(); q.pop();
            assert(dc->metalViaType != DiffusionChamberType::UNKNOWN);
            
            if(dc->metalViaType == DiffusionChamberType::METAL){
                MetalCell *mcPointer = static_cast<MetalCell *>(dc);
                
                MetalCell *mcNorthCell = mcPointer->northCell;
                MetalCell *mcSouthCell = mcPointer->southCell;
                MetalCell *mcEastCell = mcPointer->eastCell;
                MetalCell *mcWestCell = mcPointer->westCell;

                size_t mcNorthCellIdx = mcPointer->northCellIdx;
                size_t mcSouthCellIdx = mcPointer->southCellIdx;
                size_t mcEastCellIdx = mcPointer->eastCellIdx;
                size_t mcWestCellIdx = mcPointer->westCellIdx;

                ViaCell *mcUpCell = mcPointer->upCell;
                ViaCell *mcDownCell = mcPointer->downCell;

                size_t mcUpCellIdx = mcPointer->upCellIdx;
                size_t mcDownCellIdx = mcPointer->downCellIdx;

                if((mcNorthCell != nullptr) && (!metalVisited[mcNorthCellIdx]) && (mcNorthCell->signal == paintingType)){
                    metalVisited[mcNorthCellIdx] = true;
                    this->metalGridLabel[mcNorthCellIdx] = labelIdx;
                    q.emplace(mcNorthCell);
                }

                if((mcSouthCell != nullptr) && (!metalVisited[mcSouthCellIdx]) && (mcSouthCell->signal == paintingType)){
                    metalVisited[mcSouthCellIdx] = true;
                    this->metalGridLabel[mcSouthCellIdx] = labelIdx;
                    q.emplace(mcSouthCell);
                }

                if((mcEastCell != nullptr) && (!metalVisited[mcEastCellIdx]) && (mcEastCell->signal == paintingType)){
                    metalVisited[mcEastCellIdx] = true;
                    this->metalGridLabel[mcEastCellIdx] = labelIdx;
                    q.emplace(mcEastCell);
                }

                if((mcWestCell != nullptr) && (!metalVisited[mcWestCellIdx]) && (mcWestCell->signal == paintingType)){
                    metalVisited[mcWestCellIdx] = true;
                    this->metalGridLabel[mcWestCellIdx] = labelIdx;
                    q.emplace(mcWestCell);
                }

                if((mcUpCell != nullptr) && (!viaVisited[mcUpCellIdx]) && (mcUpCell->signal == paintingType)){
                    viaVisited[mcUpCellIdx] = true;
                    this->viaGridLabel[mcUpCellIdx] = labelIdx;
                    q.emplace(mcUpCell);
                }

                if((mcDownCell != nullptr) && (!viaVisited[mcDownCellIdx]) && (mcDownCell->signal == paintingType)){
                    viaVisited[mcDownCellIdx] = true;
                    this->viaGridLabel[mcDownCellIdx] = labelIdx;
                    q.emplace(mcDownCell);
                }

            }else{ // DiffusionChamberType::VIA
                ViaCell *vcPointer = static_cast<ViaCell *>(dc);

                MetalCell *vcUpLLCell = vcPointer->upLLCell;
                MetalCell *vcUpULCell = vcPointer->upULCell;
                MetalCell *vcUpLRCell = vcPointer->upLRCell;
                MetalCell *vcUpURCell = vcPointer->upURCell;

                MetalCell *vcDownLLCell = vcPointer->downLLCell;
                MetalCell *vcDownULCell = vcPointer->downULCell;
                MetalCell *vcDownLRCell = vcPointer->downLRCell;
                MetalCell *vcDownURCell = vcPointer->downURCell;

                size_t vcUpLLCellIdx = vcPointer->upLLCellIdx;
                size_t vcUpULCellIdx = vcPointer->upULCellIdx;
                size_t vcUpLRCellIdx = vcPointer->upLRCellIdx;
                size_t vcUpURCellIdx = vcPointer->upURCellIdx;

                size_t vcDownLLCellIdx = vcPointer->downLLCellIdx;
                size_t vcDownULCellIdx = vcPointer->downULCellIdx;
                size_t vcDownLRCellIdx = vcPointer->downLRCellIdx;
                size_t vcDownURCellIdx = vcPointer->downURCellIdx;

                if((vcUpLLCell != nullptr) && (!metalVisited[vcUpLLCellIdx]) && (vcUpLLCell->signal == paintingType)){
                    metalVisited[vcUpLLCellIdx] = true;
                    this->metalGridLabel[vcUpLLCellIdx] = labelIdx;
                    q.emplace(vcUpLLCell);
                }
                if((vcUpULCell != nullptr) && (!metalVisited[vcUpULCellIdx]) && (vcUpULCell->signal == paintingType)){
                    metalVisited[vcUpULCellIdx] = true;
                    this->metalGridLabel[vcUpULCellIdx] = labelIdx;
                    q.emplace(vcUpULCell);
                }
                if((vcUpLRCell != nullptr) && (!metalVisited[vcUpLRCellIdx]) && (vcUpLRCell->signal == paintingType)){
                    metalVisited[vcUpLRCellIdx] = true;
                    this->metalGridLabel[vcUpLRCellIdx] = labelIdx;
                    q.emplace(vcUpLRCell);
                }
                if((vcUpURCell != nullptr) && (!metalVisited[vcUpURCellIdx]) && (vcUpURCell->signal == paintingType)){
                    metalVisited[vcUpURCellIdx] = true;
                    this->metalGridLabel[vcUpURCellIdx] = labelIdx;
                    q.emplace(vcUpURCell);
                }

                if((vcDownLLCell != nullptr) && (!metalVisited[vcDownLLCellIdx]) && (vcDownLLCell->signal == paintingType)){
                    metalVisited[vcDownLLCellIdx] = true;
                    this->metalGridLabel[vcDownLLCellIdx] = labelIdx;
                    q.emplace(vcDownLLCell);
                }
                if((vcDownULCell != nullptr) && (!metalVisited[vcDownULCellIdx]) && (vcDownULCell->signal == paintingType)){
                    metalVisited[vcDownULCellIdx] = true;
                    this->metalGridLabel[vcDownULCellIdx] = labelIdx;
                    q.emplace(vcDownULCell);
                }
                if((vcDownLRCell != nullptr) && (!metalVisited[vcDownLRCellIdx]) && (vcDownLRCell->signal == paintingType)){
                    metalVisited[vcDownLRCellIdx] = true;
                    this->metalGridLabel[vcDownLRCellIdx] = labelIdx;
                    q.emplace(vcDownLRCell);
                }
                if((vcDownURCell != nullptr) && (!metalVisited[vcDownURCellIdx]) && (vcDownURCell->signal == paintingType)){
                    metalVisited[vcDownURCellIdx] = true;
                    this->metalGridLabel[vcDownURCellIdx] = labelIdx;
                    q.emplace(vcDownURCell);
                }
            }
        }
        
        cellLabelToSigType.push_back(paintingType);
        sigTypeToAllCellLabels[paintingType].emplace_back(labelIdx);

        ++labelIdx;
    }

    return static_cast<int>(labelIdx);
}

void DiffusionEngine::placeDiffusionParticles(){

    size_t metalGridSize = metalGrid.size();
    size_t viaGridSize = viaGrid.size();

    // put particles to the surrounded points;
    std::unordered_map<CellLabel, std::unordered_set<DiffusionChamber *>> whiteSpaceMap;

    // collect for white space map by iterate all cells
    for(size_t i = 0; i < (metalGridSize + viaGridSize); ++i){
        bool isMetalCell = (i < metalGridSize);

        if(isMetalCell){
            MetalCell &mc = metalGrid[i];
            MetalCell *mcPointer = &mc;
            CellLabel metalCellLabel = metalGridLabel[i];
            CellType metalCellType = mc.type;
            if(metalCellType == CellType::EMPTY || metalCellType == CellType::OBSTACLES) continue;

            MetalCell *mcNorthCell = mc.northCell;
            MetalCell *mcSouthCell = mc.southCell;
            MetalCell *mcEastCell = mc.eastCell;
            MetalCell *mcWestCell = mc.westCell;

            ViaCell *mcUpCell = mc.upCell;
            ViaCell *mcDownCell = mc.downCell;

            if((mcNorthCell != nullptr) && (mcNorthCell->type == CellType::EMPTY)){
                whiteSpaceMap[metalCellLabel].insert(mcNorthCell);
            }
            if((mcSouthCell != nullptr) && (mcSouthCell->type == CellType::EMPTY)){
                whiteSpaceMap[metalCellLabel].insert(mcSouthCell);
            }
            if((mcEastCell != nullptr) && (mcEastCell->type == CellType::EMPTY)){
                whiteSpaceMap[metalCellLabel].insert(mcEastCell);
            }
            if((mcWestCell != nullptr) && (mcWestCell->type == CellType::EMPTY)){
                whiteSpaceMap[metalCellLabel].insert(mcWestCell);
            }

            if((mcUpCell != nullptr) && (mcUpCell->type == CellType::EMPTY)){
                whiteSpaceMap[metalCellLabel].insert(mcUpCell);
            }
            if((mcDownCell != nullptr) && (mcDownCell->type == CellType::EMPTY)){
                whiteSpaceMap[metalCellLabel].insert(mcDownCell);
            }

        }else{ // is Via Cell
            size_t viaRealIdx = i - metalGridSize;
            ViaCell &vc = viaGrid[viaRealIdx];
            ViaCell *vcPointer = &vc;
            CellLabel viaCellLabel =  viaGridLabel[viaRealIdx];
            CellType viaCellType = vc.type;

            if(viaCellType == CellType::EMPTY) continue;


            MetalCell *vcUpLLCell = vc.upLLCell;
            MetalCell *vcUpULCell = vc.upULCell;
            MetalCell *vcUpLRCell = vc.upLRCell;
            MetalCell *vcUpURCell = vc.upURCell;

            MetalCell *vcDownLLCell = vc.downLLCell;
            MetalCell *vcDownULCell = vc.downULCell;
            MetalCell *vcDownLRCell = vc.downLRCell;
            MetalCell *vcDownURCell = vc.downURCell;

            if((vcUpLLCell != nullptr) && (vcUpLLCell->type != CellType::EMPTY)){
                whiteSpaceMap[viaCellLabel].insert(vcUpLLCell);
            }
            if((vcUpULCell != nullptr) && (vcUpULCell->type != CellType::EMPTY)){
                whiteSpaceMap[viaCellLabel].insert(vcUpULCell);
            }
            if((vcUpLRCell != nullptr) && (vcUpLRCell->type != CellType::EMPTY)){
                whiteSpaceMap[viaCellLabel].insert(vcUpLRCell);
            }
            if((vcUpURCell != nullptr) && (vcUpURCell->type != CellType::EMPTY)){
                whiteSpaceMap[viaCellLabel].insert(vcUpURCell);
            }

            if((vcDownLLCell != nullptr) && (vcDownLLCell->type != CellType::EMPTY)){
                whiteSpaceMap[viaCellLabel].insert(vcDownLLCell);
            }
            if((vcDownULCell != nullptr) && (vcDownULCell->type != CellType::EMPTY)){
                whiteSpaceMap[viaCellLabel].insert(vcDownULCell);
            }
            if((vcDownLRCell != nullptr) && (vcDownLRCell->type != CellType::EMPTY)){
                whiteSpaceMap[viaCellLabel].insert(vcDownLRCell);
            }
            if((vcDownURCell != nullptr) && (vcDownURCell->type != CellType::EMPTY)){
                whiteSpaceMap[viaCellLabel].insert(vcDownURCell);
            }
        }

    }

    for(auto &[cl, dcset] : whiteSpaceMap){
        SignalType st = this->cellLabelToSigType[cl];
        if(POWER_SIGNAL_SET.count(st) == 0) continue;

        for(DiffusionChamber *dc : dcset){
            dc->addParticlesToCache(cl, 1000);
            dc->commitCache();
        }
    }

}

void DiffusionEngine::diffuse(double diffusionRate){
    assert(diffusionRate > 0 && diffusionRate < 0.5);

    for(MetalCell &cell : metalGrid){
        size_t neighborSize = cell.neighbors.size();
        if(neighborSize == 0) continue;

        cell.cellLabelsCache = cell.cellLabels;
        cell.cellParticlesCache = cell.cellParticles;


        for (size_t i = 0; i < cell.cellParticlesCache.size(); ++i) {
            cell.cellParticlesCache[i] = -cell.cellParticles[i] * neighborSize;
        }

        for(DiffusionChamber *diffc : cell.neighbors){
            for(size_t i = 0; i < diffc->cellLabels.size(); ++i){
                cell.addParticlesToCache(diffc->cellLabels[i], diffc->cellParticles[i]);
            }
        }

        for (size_t i = 0; i < cell.cellParticlesCache.size(); ++i) {
            cell.cellParticlesCache[i] = diffusionRate * cell.cellParticlesCache[i] + cell.getParticlesCount(cell.cellLabelsCache[i]);
        }

    }

    for(ViaCell &cell : viaGrid){
        size_t neighborSize = cell.neighbors.size();
        if(neighborSize == 0) continue;

        cell.cellLabelsCache = cell.cellLabels;
        cell.cellParticlesCache = cell.cellParticles;


        for (size_t i = 0; i < cell.cellParticlesCache.size(); ++i) {
            cell.cellParticlesCache[i] = -cell.cellParticles[i] * neighborSize;
        }

        for(DiffusionChamber *diffc : cell.neighbors){
            for(size_t i = 0; i < diffc->cellLabels.size(); ++i){
                cell.addParticlesToCache(diffc->cellLabels[i], diffc->cellParticles[i]);
            }
        }
        
        for (size_t i = 0; i < cell.cellParticlesCache.size(); ++i) {
            cell.cellParticlesCache[i] = diffusionRate * cell.cellParticlesCache[i] + cell.getParticlesCount(cell.cellLabelsCache[i]);
        }
    }

    for(MetalCell &cell : metalGrid){
        cell.commitCache();
    }
    for(ViaCell &cell : viaGrid){
        cell.commitCache();
    }
    
}

void DiffusionEngine::stage(){

    for(MetalCell &cell : this->metalGrid){
        if((cell.type != CellType::EMPTY)  || (cell.cellLabels.empty())) continue;
        size_t maxIdx = 0;
        int maxParticles = cell.cellParticles[0];

        for(size_t i = 1; i < cell.cellParticles.size(); ++i){
            int particlesCount = cell.cellParticles[i];
            if(particlesCount > maxParticles){
                maxParticles = particlesCount;
                maxIdx = i;
            }
        }

        cell.signal = cellLabelToSigType[cell.cellLabels[maxIdx]];
    }

    for(ViaCell &cell : this->viaGrid){
        if((cell.type != CellType::EMPTY)  || (cell.cellLabels.empty())) continue;
        size_t maxIdx = 0;
        int maxParticles = cell.cellParticles[0];

        for(size_t i = 1; i < cell.cellParticles.size(); ++i){
            int particlesCount = cell.cellParticles[i];
            if(particlesCount > maxParticles){
                maxParticles = particlesCount;
                maxIdx = i;
            }
        }

        cell.signal = cellLabelToSigType[cell.cellLabels[maxIdx]];
    }
}

void DiffusionEngine::initialiseMCFSolver(){
    
    // initialise flowSOIIdxToSig, flowSOISigToIdx
    // initialise superSource, superSink

    // create flowSOIIdxToSig using signals in currentBudget and in ascending order of current budget
    for (const auto& [sig, _] : currentBudget) {
        flowSOIIdxToSig.push_back(sig);
    }
    std::sort(flowSOIIdxToSig.begin(), flowSOIIdxToSig.end(), 
        [&](const SignalType& a, const SignalType& b) {return currentBudget[a] < currentBudget[b];}
    );

    for(int flowSOIIdx = 0; flowSOIIdx < flowSOIIdxToSig.size(); ++flowSOIIdx){
        SignalType sig = flowSOIIdxToSig[flowSOIIdx];
        this->flowSOISigToIdx[sig] = flowSOIIdx;
        
        superSource.emplace_back(FlowNodeType::SUPER_SOURCE, CELL_LABEL_EMPTY, -1, sig);
        superSink.emplace_back(FlowNodeType::SUPER_SINK, CELL_LABEL_EMPTY, -1, sig);
    }

    // initialise viaFlowTopNodeArr, viaFlowDownNodeArr
    viaFlowTopNodeArr.resize(m_viaGridLayers);
    viaFlowDownNodeArr.resize(m_viaGridLayers);
    for(int viaLayer = 0; viaLayer < m_viaGridLayers; ++viaLayer){
        size_t layerViaCount = m_viaGrid2DCount[viaLayer];
        viaFlowTopNodeArr[viaLayer].resize(layerViaCount, FlowNode(FlowNodeType::VIA_TOP, CELL_LABEL_EMPTY, viaLayer));
        viaFlowDownNodeArr[viaLayer].resize(layerViaCount, FlowNode(FlowNodeType::VIA_DOWN, CELL_LABEL_EMPTY, viaLayer));
    }

    // initialise metalFlowNodeOwnership first, than like the pointer to metalFlowNodeArr
    int metalNodeCount = static_cast<int>(initialiseIndexing());
    std::vector<bool> aggregatedNodesProcessed(metalNodeCount, false);
    for(int i = 0; i < metalNodeCount; ++i){
        this->metalFlowNodeOwnership.emplace_back(FlowNodeType::AGGREGATED, i);
    }

    
    std::vector<std::vector<std::vector<CellLabel>>> tmpMetalLabel(
        m_metalGridLayers, std::vector<std::vector<CellLabel>>(
            m_metalGridHeight, std::vector<CellLabel>(m_metalGridWidth, CELL_LABEL_EMPTY)
        )
    );

    auto in2DRange = [&](int y, int x){
        return (y >= 0) && (y < m_metalGridHeight) && (x >= 0) && (x >= m_metalGridWidth);
    };

    std::unordered_map<SignalType, std::vector<CellLabel>> superSourceConnIdx;
    std::unordered_map<SignalType, std::vector<CellLabel>> superSinkConnIdx;

    for(size_t layer = 0; layer < m_metalGridLayers; ++layer){
        for(size_t y = 0; y < m_metalGridHeight; ++y){
            for(size_t x = 0; x < m_metalGridWidth; ++x){
                size_t mcIdx = calMetalIdx(layer, y, x);
                MetalCell &mc = this->metalGrid[mcIdx];
                
                CellLabel mcCellLabel = this->metalGridLabel[mcIdx];


                if(mc.type == CellType::EMPTY){
                    int newIdx = metalFlowNodeOwnership.size();
                    this->metalFlowNodeOwnership.emplace_back(FlowNodeType::EMPTY, newIdx, layer);
                    FlowNode &fn = metalFlowNodeOwnership.back();
                    tmpMetalLabel[layer][y][x] = newIdx;

                    bool LLIsSpecial = in2DRange(y-1, x-1) && (metalGrid[calMetalIdx(layer, y-1, x-1)].type != CellType::EMPTY);
                    bool ULIsSpecial = in2DRange(y+1, x-1) && (metalGrid[calMetalIdx(layer, y+1, x-1)].type != CellType::EMPTY);
                    bool LRIsSpecial = in2DRange(y-1, x+1) && (metalGrid[calMetalIdx(layer, y-1, x+1)].type != CellType::EMPTY);
                    bool URIsSpecial = in2DRange(y+1, x+1) && (metalGrid[calMetalIdx(layer, y+1, x+1)].type != CellType::EMPTY);
                    
                    bool NorthIsSpecial = false;
                    bool SouthIsSpecial = false;
                    bool EastIsSpecial = false;
                    bool WestIsSpecial = false;

                    if(in2DRange(y+1, x)){
                        CellType northType = metalGrid[calMetalIdx(layer, y+1, x)].type;
                        NorthIsSpecial = (northType != CellType::EMPTY);
                        if(northType == CellType::PREPLACED || northType == CellType::MARKED){
                            fn.northIsAggregated = true;
                        }
                    }

                    if(in2DRange(y-1, x)){
                        CellType southType = metalGrid[calMetalIdx(layer, y-1, x)].type;
                        SouthIsSpecial = (southType != CellType::EMPTY);
                        if(southType == CellType::PREPLACED || southType == CellType::MARKED){
                            fn.southIsAggregated = true;
                        }
                    }
                    
                    if(in2DRange(y, x+1)){
                        CellType eastType = metalGrid[calMetalIdx(layer, y, x+1)].type;
                        EastIsSpecial = (eastType != CellType::EMPTY);
                        if(eastType == CellType::PREPLACED || eastType == CellType::MARKED){
                            fn.eastIsAggregated = true;
                        }
                        
                    }
                    
                    if(in2DRange(y, x-1)){
                        CellType westType = metalGrid[calMetalIdx(layer, y, x-1)].type;
                        WestIsSpecial = (westType != CellType::EMPTY);
                        if(westType == CellType::PREPLACED || westType == CellType::MARKED){
                            fn.westIsAggregated = true;
                        }
                    }

                    fn.isSuperNode = LLIsSpecial || WestIsSpecial || ULIsSpecial 
                        || SouthIsSpecial || NorthIsSpecial 
                        || LRIsSpecial || EastIsSpecial || URIsSpecial;
                    
                    
                }else if(mc.type == CellType::MARKED || mc.type == CellType::PREPLACED){
                    SignalType mcSignal = mc.signal;
                    int newIdx = mcCellLabel;
                    tmpMetalLabel[layer][y][x] = newIdx;
                    if(aggregatedNodesProcessed[newIdx]) continue;

                    aggregatedNodesProcessed[newIdx] = true;
                    FlowNode &fn = metalFlowNodeOwnership[newIdx];
                    fn.layer = layer;
                    fn.signal = mcSignal;
                    if(layer == m_ubumpConnectedMetalLayerIdx){
                        superSinkConnIdx[mcSignal].push_back(mcCellLabel);
                    }else if(layer == m_c4ConnectedMetalLayerIdx){
                        superSourceConnIdx[mcSignal].push_back(mcCellLabel);
                    }

                    
                }else if(mc.type == CellType::OBSTACLES){
                    int newIdx = mcCellLabel;
                    tmpMetalLabel[layer][y][x] = newIdx;  
                    if(aggregatedNodesProcessed[newIdx]) continue;

                    aggregatedNodesProcessed[newIdx] = true;
                    FlowNode &fn = metalFlowNodeOwnership[newIdx];
                    fn.layer = layer;
                    fn.type = FlowNodeType::OBSTACLES;
                }
            }
        }
    }
    // transform superSource and superSink idx to FlowNode*

    for (const auto& [signal, connVec] : superSourceConnIdx) {
        for (const CellLabel& label : connVec) {
            this->superSourceConnectedNodes[signal].push_back(&metalFlowNodeOwnership[label]);
        }
    }

    for (const auto& [signal, connVec] : superSinkConnIdx) {
        for (const CellLabel& label : connVec) {
            this->superSinkConnectedNodes[signal].push_back(&metalFlowNodeOwnership[label]);
        }
    }


    // paint the via pads as special nodes as well
    for(int viaIdx = 0; viaIdx < getAllViaIdxEnd(); ++viaIdx){
        ViaCell &vc = this->viaGrid[viaIdx];
        
        len_t viaLayer = vc.canvasLayer;
        len_t viaY = vc.canvasY;
        len_t viaX = vc.canvasX;

        CellLabel topLLLabel = tmpMetalLabel[viaLayer][viaY-1][viaX-1];
        CellLabel topLRLabel = tmpMetalLabel[viaLayer][viaY-1][viaX];
        CellLabel topULLabel = tmpMetalLabel[viaLayer][viaY][viaX-1];
        CellLabel topURLabel = tmpMetalLabel[viaLayer][viaY][viaX];

        CellLabel downLLLabel = tmpMetalLabel[viaLayer+1][viaY-1][viaX-1];
        CellLabel downLRLabel = tmpMetalLabel[viaLayer+1][viaY-1][viaX];
        CellLabel downULLabel = tmpMetalLabel[viaLayer+1][viaY+1][viaX-1];
        CellLabel downURLabel = tmpMetalLabel[viaLayer+1][viaY+1][viaX];

        if(metalFlowNodeOwnership[topLLLabel].type == FlowNodeType::EMPTY){
            metalFlowNodeOwnership[topLLLabel].isSuperNode = true;
        }
        if(metalFlowNodeOwnership[topLRLabel].type == FlowNodeType::EMPTY){
            metalFlowNodeOwnership[topLRLabel].isSuperNode = true;
        }
        if(metalFlowNodeOwnership[topULLabel].type == FlowNodeType::EMPTY){
            metalFlowNodeOwnership[topULLabel].isSuperNode = true;
        }
        if(metalFlowNodeOwnership[topURLabel].type == FlowNodeType::EMPTY){
            metalFlowNodeOwnership[topURLabel].isSuperNode = true;
        }

        if(metalFlowNodeOwnership[downLLLabel].type == FlowNodeType::EMPTY){
            metalFlowNodeOwnership[downLLLabel].isSuperNode = true;
        }
        if(metalFlowNodeOwnership[downLRLabel].type == FlowNodeType::EMPTY){
            metalFlowNodeOwnership[downLRLabel].isSuperNode = true;
        }
        if(metalFlowNodeOwnership[downULLabel].type == FlowNodeType::EMPTY){
            metalFlowNodeOwnership[downULLabel].isSuperNode = true;
        }
        if(metalFlowNodeOwnership[downURLabel].type == FlowNodeType::EMPTY){
            metalFlowNodeOwnership[downURLabel].isSuperNode = true;
        }
    }

    this->metalFlowNodeArr = std::vector<std::vector<std::vector<FlowNode *>>>(
        m_metalGridLayers, std::vector<std::vector<FlowNode *>>(
            m_metalGridHeight, std::vector<FlowNode *>(m_metalGridWidth, nullptr)
        )
    );

    for(size_t layer = 0; layer < m_metalGridLayers; ++layer){
        for(size_t y = 0; y < m_metalGridHeight; ++y){
            for(size_t x = 0; x < m_metalGridWidth; ++x){
                CellLabel label = tmpMetalLabel[layer][y][x];
                this->metalFlowNodeArr[layer][y][x] = &(this->metalFlowNodeOwnership[label]);
            }
        }
    }

    // calculate SOIBudget according to viadistribution & currentBudget array

    SOIBudget.resize(flowSOIIdxToSig.size(), std::numeric_limits<double>::max());
    double minViaLayerSumBudget = std::numeric_limits<double>::max();
    for(size_t viaLayer = 0; viaLayer < m_viaGridLayers; ++viaLayer){
        double emptyViaCount = static_cast<double>(m_viaGrid2DCount[viaLayer]);
        
        for(size_t viaIdx = 0; viaIdx < m_viaGrid2DCount[viaLayer]; ++ viaIdx){
            ViaCell &vc = this->viaGrid[calViaIdx(viaLayer, viaIdx)];
            if(vc.type == CellType::OBSTACLES) emptyViaCount -= 1;
            else if(vc.type == CellType::PREPLACED || vc.type == CellType::MARKED){
                emptyViaCount -= 0.5;
            }
        }
        if(emptyViaCount < minViaLayerSumBudget) minViaLayerSumBudget = emptyViaCount;
    }

    for(int i = 0; i < SOIBudget.size(); ++i){
        SOIBudget[i] = minViaLayerSumBudget * ((ViaBudgetAvgQuota / SOIBudget.size()) + viaBudgetCurrentQuota * currentBudget[flowSOIIdxToSig[i]]);
    }

    // for displaying
    std::cout << "Display requrests: " << std::endl;
    for(int i = 0; i < flowSOIIdxToSig.size(); ++i){
        std::cout << flowSOIIdxToSig[i] << " " << SOIBudget[i] << std::endl;
    }
    std::cout << "Display superSource" << std::endl;
    for(FlowNode &fn : superSource){
        std::cout << fn.type << " " << fn.label << " " << fn.layer << " " << fn.signal << std::endl;
    }

    std::cout << "Display superSink" << std::endl;
    for(FlowNode &fn : superSink){
        std::cout << fn.type << " " << fn.label << " " << fn.layer << " " << fn.signal << std::endl;
    }

}

void DiffusionEngine::runMCFSolver(std::string logFile, int outputLevel){
   
    auto in2DRange = [&](int y, int x){
        return (y >= 0) && (y < m_metalGridHeight) && (x >= 0) && (x < m_metalGridWidth);
    };

    try {
        /* Initialise Gubobi solver*/
        GRBEnv GRBenv = GRBEnv(true);
        GRBenv.set("LogFile", logFile);
        GRBenv.set(GRB_IntParam_OutputFlag, outputLevel);
        GRBenv.start();
        GRBModel GRBmodel = GRBModel(GRBenv);

        /* construct the flow decision variables */
        // STEP 1. build metal layer decision variables, use the initialized markings
        std::vector<bool> metalFlowNodeIdxProcessed(metalFlowNodeOwnership.size(), false);
        
        for(size_t layer = m_ubumpConnectedMetalLayerIdx; layer <= m_c4ConnectedMetalLayerIdx; ++layer){
            for(int y = 0; y < m_metalGridHeight; ++y){
                for(int x = 0; x < m_metalGridWidth; ++x){
                    FlowNode *fnPointer = this->metalFlowNodeArr[layer][y][x];
                    if(fnPointer->type != FlowNodeType::EMPTY) continue;
                    
                    std::vector<Cord>fourNeighbors = {Cord(1, 0), Cord(-1, 0), Cord(0, 1), Cord(0, -1)};
                    bool UpDownDir = ((x + y) %2 == 0);

                    for(const Cord &neighborDir : fourNeighbors){
                        int nx = x + neighborDir.x();
                        int ny = y + neighborDir.y();

                        if(in2DRange(ny, nx)){
                            FlowNode *neighborFnPointer = this->metalFlowNodeArr[layer][ny][nx];

                            if(neighborFnPointer->type == FlowNodeType::EMPTY){
                                // create two directions,
                                // from this -> north
                                // from node -> this
                                if((UpDownDir && (neighborDir.y() != 0)) || (!UpDownDir && (neighborDir.x() != 0)) || (fnPointer->isSuperNode)){
                                    GRBLinExpr varsExclusiveLinExpr = 0.0;
                                    for(SignalType &st : this->flowSOIIdxToSig){
                                        GRBVar var = GRBmodel.addVar(normalMetalEdgeLB, normalMetalEdgeUB, normalMetalEdgeWeight, GRB_CONTINUOUS);
                                        FlowEdge *fe = new FlowEdge(st, fnPointer, neighborFnPointer, var);
                                        this->flowEdgeOwnership.push_back(fe);
                                        fnPointer->outEdges.push_back(fe);
                                        neighborFnPointer->inEdges.push_back(fe);
                                        varsExclusiveLinExpr += var;
                                    }
                                    // add exclusiveness
                                    GRBmodel.addConstr(varsExclusiveLinExpr <= normalMetalEdgeUB);
                                }

                                


                            }else if(neighborFnPointer->type == FlowNodeType::AGGREGATED){
                                if(layer == m_ubumpConnectedMetalLayerIdx){
                                    // only goes from this -> neighbor(aggregated)
                                    SignalType targetSt = neighborFnPointer->signal;
                                    GRBVar var = GRBmodel.addVar(aggrMetalEdgeLB, aggrMetalEdgeUB, aggrMetalEdgeWeight, GRB_CONTINUOUS);
                                    FlowEdge *fe = new FlowEdge(targetSt, fnPointer, neighborFnPointer, var);
                                    this->flowEdgeOwnership.push_back(fe);
                                    fnPointer->outEdges.push_back(fe);
                                    neighborFnPointer->inEdges.push_back(fe);
                                    
                                }else if(layer == m_c4ConnectedMetalLayerIdx){
                                    // only goes from neighbor(aggregated) -> this
                                    SignalType targetSt = neighborFnPointer->signal;
                                    GRBVar var = GRBmodel.addVar(aggrMetalEdgeLB, aggrMetalEdgeUB, aggrMetalEdgeWeight, GRB_CONTINUOUS);
                                    FlowEdge *fe = new FlowEdge(targetSt, neighborFnPointer, fnPointer, var);
                                    this->flowEdgeOwnership.push_back(fe);
                                    neighborFnPointer->outEdges.push_back(fe);
                                    fnPointer->inEdges.push_back(fe);
                                
                                }else{
                                    // goes both ways
                                    SignalType targetSt = neighborFnPointer->signal;
                                    
                                    GRBVar var0 = GRBmodel.addVar(aggrMetalEdgeLB, aggrMetalEdgeUB, aggrMetalEdgeWeight, GRB_CONTINUOUS);
                                    FlowEdge *fe0 = new FlowEdge(targetSt, fnPointer, neighborFnPointer, var0);
                                    this->flowEdgeOwnership.push_back(fe0);
                                    fnPointer->outEdges.push_back(fe0);
                                    neighborFnPointer->inEdges.push_back(fe0);

                                    GRBVar var1 = GRBmodel.addVar(aggrMetalEdgeLB, aggrMetalEdgeUB, aggrMetalEdgeWeight, GRB_CONTINUOUS);
                                    FlowEdge *fe1 = new FlowEdge(targetSt, neighborFnPointer, fnPointer, var1);
                                    this->flowEdgeOwnership.push_back(fe1);
                                    neighborFnPointer->outEdges.push_back(fe1);
                                    fnPointer->inEdges.push_back(fe1);

                                }
                            }


                        }
                    }

                }
            }
        }

        // STEP 2. build via layer diecision variables
        for(size_t viaLayer = 0; viaLayer < m_viaGridLayers; ++viaLayer){
            for(size_t viaIdx = 0; viaIdx < m_viaGrid2DCount[viaLayer]; ++ viaIdx){
                ViaCell &vc = this->viaGrid[calViaIdx(viaLayer, viaIdx)];
                size_t vcCanvasY = static_cast<size_t>(vc.canvasY);
                size_t vcCanvasX = static_cast<size_t>(vc.canvasX);

                FlowNode *topFNPointer = &viaFlowTopNodeArr[viaLayer][viaIdx];
                FlowNode *downFNPointer = &viaFlowDownNodeArr[viaLayer][viaIdx];

                // add vars from downVia -> topvia
                GRBLinExpr varsExclusiveLinExpr = 0.0;
                for(SignalType &st : this->flowSOIIdxToSig){
                    GRBVar var = GRBmodel.addVar(ViaEdgeLB, ViaEdgeUB, viaEdgeWeight, GRB_CONTINUOUS);
                    GRBVar bin = GRBmodel.addVar(0.0, 1.0, 0.0, GRB_BINARY);
                    
                    FlowEdge *fe = new FlowEdge(st, downFNPointer, topFNPointer, var);
                    this->flowEdgeOwnership.push_back(fe);
                    downFNPointer->outEdges.push_back(fe);
                    topFNPointer->inEdges.push_back(fe);
                    
                    GRBmodel.addConstr(var <= (ViaEdgeUB * bin));
                    // varsExclusiveLinExpr += var;
                    varsExclusiveLinExpr += bin;

                }
                // add exclusiveness
                // GRBmodel.addConstr(varsExclusiveLinExpr <= ViaEdgeUB);
                GRBmodel.addConstr(varsExclusiveLinExpr <= 1);

                // add vars from down nodes -> downVia
                std::unordered_map<FlowNode *, int> downOccurence;
                ++downOccurence[metalFlowNodeArr[viaLayer+1][vcCanvasY-1][vcCanvasX-1]];
                ++downOccurence[metalFlowNodeArr[viaLayer+1][vcCanvasY-1][vcCanvasX]];
                ++downOccurence[metalFlowNodeArr[viaLayer+1][vcCanvasY][vcCanvasX-1]];
                ++downOccurence[metalFlowNodeArr[viaLayer+1][vcCanvasY][vcCanvasX]];

                for(const auto &[key, value] : downOccurence){
                    if(key->type == FlowNodeType::OBSTACLES){
                        continue;
                    }else if(key->type == FlowNodeType::EMPTY){
                        GRBLinExpr varsExclusiveLinExpr = 0.0;
                        for(SignalType &st : this->flowSOIIdxToSig){
                            GRBVar var = GRBmodel.addVar(ViaEdgeLB, subViaEdgeUB, viaEdgeWeight, GRB_CONTINUOUS);
                            FlowEdge *fe = new FlowEdge(st, key, downFNPointer, var);
                            this->flowEdgeOwnership.push_back(fe);
                            key->outEdges.push_back(fe);
                            downFNPointer->inEdges.push_back(fe);
                            varsExclusiveLinExpr += var;
                        }
                        // add exclusiveness
                        GRBmodel.addConstr(varsExclusiveLinExpr <= subViaEdgeUB);

                    }else if(key->type == FlowNodeType::AGGREGATED){
                        double finlalSubViaUB = (value == 1)? subViaEdgeUB : ViaEdgeUB;
                        SignalType aggregateSt = key->signal;
                        GRBVar var = GRBmodel.addVar(ViaEdgeLB, finlalSubViaUB, viaEdgeWeight, GRB_CONTINUOUS);
                        FlowEdge *fe = new FlowEdge(aggregateSt, key, downFNPointer, var);
                        this->flowEdgeOwnership.push_back(fe);
                        key->outEdges.push_back(fe);
                        downFNPointer->inEdges.push_back(fe);
                        varsExclusiveLinExpr += var;
                    }
                }

                // add vars from upVia -> up nodes
                std::unordered_map<FlowNode *, int> topOccurence;
                ++topOccurence[metalFlowNodeArr[viaLayer][vcCanvasY-1][vcCanvasX-1]];
                ++topOccurence[metalFlowNodeArr[viaLayer][vcCanvasY-1][vcCanvasX]];
                ++topOccurence[metalFlowNodeArr[viaLayer][vcCanvasY][vcCanvasX-1]];
                ++topOccurence[metalFlowNodeArr[viaLayer][vcCanvasY][vcCanvasX]];
                
                for(const auto &[key, value] : topOccurence){
                    if(key->type == FlowNodeType::OBSTACLES){
                        continue;
                    }else if(key->type == FlowNodeType::EMPTY){
                        GRBLinExpr varsExclusiveLinExpr = 0.0;
                        for(SignalType &st : this->flowSOIIdxToSig){
                            GRBVar var = GRBmodel.addVar(ViaEdgeLB, subViaEdgeUB, viaEdgeWeight, GRB_CONTINUOUS);
                            FlowEdge *fe = new FlowEdge(st, topFNPointer, key, var);
                            this->flowEdgeOwnership.push_back(fe);
                            topFNPointer->outEdges.push_back(fe);
                            key->inEdges.push_back(fe);
                            varsExclusiveLinExpr += var;
                        }
                        // add exclusiveness
                        GRBmodel.addConstr(varsExclusiveLinExpr <= subViaEdgeUB);

                    }else if(key->type == FlowNodeType::AGGREGATED){
                        double finlalSubViaUB = (value == 1)? subViaEdgeUB : ViaEdgeUB;
                        SignalType aggregateSt = key->signal;
                        GRBVar var = GRBmodel.addVar(ViaEdgeLB, finlalSubViaUB, viaEdgeWeight, GRB_CONTINUOUS);
                        FlowEdge *fe = new FlowEdge(aggregateSt, topFNPointer , key, var);
                        this->flowEdgeOwnership.push_back(fe);
                        topFNPointer->outEdges.push_back(fe);
                        key->inEdges.push_back(fe);
                    }
                }

            }
        }

        // STEP 3. build super-source decision variables
        for(auto &[st, nodes] : this->superSourceConnectedNodes){
            FlowNode *spSource = &superSource[flowSOISigToIdx[st]];

            for(FlowNode *fn : nodes){
                GRBVar var = GRBmodel.addVar(0, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
                FlowEdge *fe = new FlowEdge(st, spSource, fn, var);
                this->flowEdgeOwnership.push_back(fe);
                spSource->outEdges.push_back(fe);
                fn->inEdges.push_back(fe);
            }
        }
        
        // STEP 4. build super-sink decision varaibles
        for(const auto&[st, nodes] : this->superSinkConnectedNodes){
            FlowNode *spSink = &superSink[flowSOISigToIdx[st]];
            double signalLowerBound = minChipletBudgetAvgPctg * (SOIBudget[flowSOISigToIdx[st]] / nodes.size());

            for(FlowNode *fn : nodes){
                GRBVar var = GRBmodel.addVar(signalLowerBound, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
                FlowEdge *fe = new FlowEdge(st, fn, spSink, var);
                this->flowEdgeOwnership.push_back(fe);
                fn->outEdges.push_back(fe);
                spSink->inEdges.push_back(fe);
            }
        }

        // step 5. add flow constraints for each signal for each cell
        for(FlowNode &fn : metalFlowNodeOwnership){
            if(fn.inEdges.empty() && fn.outEdges.empty()) continue;

            std::vector<std::vector<FlowEdge *>> nodeInEdges(flowSOIIdxToSig.size());
            std::vector<std::vector<FlowEdge *>> nodeOutEdges(flowSOIIdxToSig.size());

            for(FlowEdge *fe : fn.inEdges){
                nodeInEdges[flowSOISigToIdx[fe->signal]].push_back(fe);
            }
            for(FlowEdge *fe : fn.outEdges){
                nodeOutEdges[flowSOISigToIdx[fe->signal]].push_back(fe);
            }

            for(int i = 0; i < flowSOIIdxToSig.size(); ++i){
                if(nodeInEdges[i].empty() && nodeOutEdges[i].empty()) continue;
                else if(nodeInEdges[i].empty()){
                    GRBLinExpr varsLinExpr = 0.0;
                    for(FlowEdge *fe : nodeOutEdges[i]){
                        varsLinExpr += fe->var;
                    }
                    GRBmodel.addConstr(varsLinExpr == 0.0); 
                }else if(nodeOutEdges[i].empty()){
                    GRBLinExpr varsLinExpr = 0.0;
                    for(FlowEdge *fe : nodeInEdges[i]){
                        varsLinExpr += fe->var;
                    }
                    GRBmodel.addConstr(varsLinExpr == 0.0); 
                }else{
                    GRBLinExpr inExpr = 0.0;
                    GRBLinExpr outExpr = 0.0;
                    for (FlowEdge* fe : nodeInEdges[i]){
                        inExpr += fe->var;
                    } 
                    for (FlowEdge* fe : nodeOutEdges[i]){
                        outExpr += fe->var; 
                    }
                    GRBmodel.addConstr(inExpr == outExpr);
                }
            }
        } 

        for(int i = 0; i < viaFlowTopNodeArr.size(); ++i){
            for(FlowNode &fn : viaFlowTopNodeArr[i]){

                if(fn.inEdges.empty() && fn.outEdges.empty()) continue;

                std::vector<std::vector<FlowEdge *>> nodeInEdges(flowSOIIdxToSig.size());
                std::vector<std::vector<FlowEdge *>> nodeOutEdges(flowSOIIdxToSig.size());

                for(FlowEdge *fe : fn.inEdges){
                    nodeInEdges[flowSOISigToIdx[fe->signal]].push_back(fe);
                }
                for(FlowEdge *fe : fn.outEdges){
                    nodeOutEdges[flowSOISigToIdx[fe->signal]].push_back(fe);
                }

                for(int i = 0; i < flowSOIIdxToSig.size(); ++i){
                    if(nodeInEdges[i].empty() && nodeOutEdges[i].empty()) continue;
                    else if(nodeInEdges[i].empty()){
                        GRBLinExpr varsLinExpr = 0.0;
                        for(FlowEdge *fe : nodeOutEdges[i]){
                            varsLinExpr += fe->var;
                        }
                        GRBmodel.addConstr(varsLinExpr == 0.0); 
                    }else if(nodeOutEdges[i].empty()){
                        GRBLinExpr varsLinExpr = 0.0;
                        for(FlowEdge *fe : nodeInEdges[i]){
                            varsLinExpr += fe->var;
                        }
                        GRBmodel.addConstr(varsLinExpr == 0.0); 
                    }else{
                        GRBLinExpr inExpr = 0.0;
                        GRBLinExpr outExpr = 0.0;
                        for (FlowEdge* fe : nodeInEdges[i]){
                            inExpr += fe->var;
                        } 
                        for (FlowEdge* fe : nodeOutEdges[i]){
                            outExpr += fe->var; 
                        }
                        GRBmodel.addConstr(inExpr == outExpr);
                    }
                }
            } 
        }

        for(int i = 0; i < viaFlowDownNodeArr.size(); ++i){
            for(FlowNode &fn : viaFlowDownNodeArr[i]){

                if(fn.inEdges.empty() && fn.outEdges.empty()) continue;

                std::vector<std::vector<FlowEdge *>> nodeInEdges(flowSOIIdxToSig.size());
                std::vector<std::vector<FlowEdge *>> nodeOutEdges(flowSOIIdxToSig.size());

                for(FlowEdge *fe : fn.inEdges){
                    nodeInEdges[flowSOISigToIdx[fe->signal]].push_back(fe);
                }
                for(FlowEdge *fe : fn.outEdges){
                    nodeOutEdges[flowSOISigToIdx[fe->signal]].push_back(fe);
                }

                for(int i = 0; i < flowSOIIdxToSig.size(); ++i){
                    if(nodeInEdges[i].empty() && nodeOutEdges[i].empty()) continue;
                    else if(nodeInEdges[i].empty()){
                        GRBLinExpr varsLinExpr = 0.0;
                        for(FlowEdge *fe : nodeOutEdges[i]){
                            varsLinExpr += fe->var;
                        }
                        GRBmodel.addConstr(varsLinExpr == 0.0); 
                    }else if(nodeOutEdges[i].empty()){
                        GRBLinExpr varsLinExpr = 0.0;
                        for(FlowEdge *fe : nodeInEdges[i]){
                            varsLinExpr += fe->var;
                        }
                        GRBmodel.addConstr(varsLinExpr == 0.0); 
                    }else{
                        GRBLinExpr inExpr = 0.0;
                        GRBLinExpr outExpr = 0.0;
                        for (FlowEdge* fe : nodeInEdges[i]){
                            inExpr += fe->var;
                        } 
                        for (FlowEdge* fe : nodeOutEdges[i]){
                            outExpr += fe->var; 
                        }
                        GRBmodel.addConstr(inExpr == outExpr);
                    }
                }
            } 
        }

        // STEP 6: Add special flow constraints for superSource and superSink
        for(int i = 0; i < superSource.size(); ++i){
            double budget = SOIBudget[i];

            GRBLinExpr sourceLinExpr = 0.0;
            for(FlowEdge *fe : superSource[i].outEdges){
                sourceLinExpr += fe->var;
            }
            GRBmodel.addConstr(sourceLinExpr == budget);

            GRBLinExpr sinkLinExpr = 0.0;
            for(FlowEdge *fe : superSink[i].inEdges){
                sinkLinExpr += fe->var;
            }
            GRBmodel.addConstr(sinkLinExpr == budget);
        }

        // STEP 7. run the solver
        GRBmodel.optimize();
        int status = GRBmodel.get(GRB_IntAttr_Status);
        std::cout << "Gurobi Optimization Status: " << status << " — ";

        switch (status) {
            case GRB_OPTIMAL:
                std::cout << "Optimal solution found." << std::endl;
                std::cout << "Objective value: " << GRBmodel.get(GRB_DoubleAttr_ObjVal) << std::endl;
                break;

            case GRB_INFEASIBLE:
                std::cout << "Model is infeasible." << std::endl;
                std::cout << "Computing IIS (Irreducible Inconsistent Subsystem) for analysis..." << std::endl;
                GRBmodel.computeIIS();
                GRBmodel.write("infeasible_model.ilp");  // Save conflicting constraints
                std::cout << "IIS written to 'infeasible_model.ilp'." << std::endl;
                break;

            case GRB_UNBOUNDED:
                std::cout << "Model is unbounded." << std::endl;
                std::cout << "Check for missing constraints or variables with no bounds." << std::endl;
                break;

            case GRB_INF_OR_UNBD:
                std::cout << "Model is either infeasible or unbounded." << std::endl;
                std::cout << "Consider disabling dual reductions: model.set(GRB_IntParam_DualReductions, 0);" << std::endl;
                break;

            case GRB_TIME_LIMIT:
                std::cout << "Time limit reached before finding an optimal solution." << std::endl;
                break;

            case GRB_INTERRUPTED:
                std::cout << "Optimization was interrupted." << std::endl;
                break;

            case GRB_CUTOFF:
                std::cout << "Optimization stopped due to reaching the cutoff parameter." << std::endl;
                break;

            default:
                std::cout << "Unhandled status code. Refer to Gurobi documentation for details." << std::endl;
                break;
        }
       
        /* Extract Results*/
        // write the results back:
        for(int layer = 0; layer < m_metalGridLayers; ++layer){
            for(int y = 0; y < m_metalGridHeight; ++y){
                for(int x = 0; x < m_metalGridWidth; ++x){
                    
                    FlowNode *fn = metalFlowNodeArr[layer][y][x];
                    if(fn->type != FlowNodeType::EMPTY) continue;

                    std::unordered_map<SignalType, double> vote;
                    for(FlowEdge *fe : fn->inEdges){
                        double result = fe->var.get(GRB_DoubleAttr_X);
                        if(result > 1e-6){
                            vote[fe->signal] += result;
                        }
                    }

                    if(!vote.empty()){
                        SignalType winner;
                        double winnerValue = -1;
                        for(const auto &[st, sum] : vote){
                            if(sum > winnerValue){
                                winner = st;
                                winnerValue = sum;
                            }
                        }

                        MetalCell &mc = this->metalGrid[calMetalIdx(layer, y, x)];
                        mc.type = CellType::MARKED;
                        mc.signal = winner;
                        // std::cout << "Winner of (" << layer << ", " << y << ", " << x << ") is " << winner << std::endl;
                    }
                }
            }
        }

        for(int layer = 0; layer < m_viaGridLayers; ++layer){
            for(int idx = 0; idx < m_viaGrid2DCount[layer]; ++idx){
                ViaCell &vc = this->viaGrid[calViaIdx(layer, idx)];
                if(vc.type != CellType::EMPTY) continue;

                FlowNode &fn = viaFlowDownNodeArr[layer][idx];


                std::unordered_map<SignalType, double> vote;
                for(FlowEdge *fe : fn.outEdges){
                    double result = fe->var.get(GRB_DoubleAttr_X);
                    if(result > 1e-6){
                        vote[fe->signal] += result;
                    }
                }

                if(!vote.empty()){
                    SignalType winner;
                    double winnerValue = -1;
                    for(const auto &[st, sum] : vote){
                        if(sum > winnerValue){
                            winner = st;
                            winnerValue = sum;
                        }
                    }
                    
                    vc.type = CellType::MARKED;
                    vc.signal = winner;
                }
            }
        }

    } catch (GRBException &e) {
        std::cerr << "Gurobi error: " << e.getMessage() << std::endl;
    }
}

void DiffusionEngine::checkConnections(){
    for(int layer = 0; layer < m_metalGridLayers; ++layer){
        for(int y = 0; y < m_gridHeight; ++y){
            for(int x = 0; x < m_gridWidth; ++x){
                size_t idx = calMetalIdx(layer, y, x);
                MetalCell &mc = this->metalGrid[idx];
                assert(mc.canvasLayer == layer);
                assert(mc.canvasY == y);
                assert(mc.canvasX == x);

                if(y == 0) assert((mc.southCell == nullptr) && (mc.southCellIdx == SIZE_T_INVALID));
                if(y == (m_gridHeight-1)) assert((mc.northCell == nullptr) && (mc.northCellIdx == SIZE_T_INVALID));
                if(x == 0) assert((mc.westCell == nullptr) && (mc.westCellIdx == SIZE_T_INVALID));
                if(x == (m_gridWidth-1)) assert((mc.eastCell == nullptr) && (mc.eastCellIdx == SIZE_T_INVALID));

                if(mc.northCell != nullptr){
                    assert(mc.northCell->canvasLayer == layer);
                    assert(mc.northCell->canvasX == x);
                    assert(mc.northCell->canvasY == (y+1));
                }
                if(mc.southCell != nullptr){
                    assert(mc.southCell->canvasLayer == layer);
                    assert(mc.southCell->canvasX == x);
                    assert(mc.southCell->canvasY == (y-1));
                }

                if(mc.eastCell != nullptr){
                    assert(mc.eastCell->canvasLayer == layer);
                    assert(mc.eastCell->canvasX == (x+1));
                    assert(mc.eastCell->canvasY == y);
                }
                if(mc.westCell != nullptr){
                    assert(mc.westCell->canvasLayer == layer);
                    assert(mc.westCell->canvasX == (x-1));
                    assert(mc.westCell->canvasY == y);
                }
            }
        }
    }

    std::cout << "Pass metal connection check!" << std::endl;

    for(int layer = 0; layer < m_viaGridLayers; ++layer){
        for(int idx = getViaIdxBegin(layer); idx < getViaIdxEnd(layer); ++idx){
            ViaCell &vc = this->viaGrid[idx];
            len_t viaLayer = vc.canvasLayer;
            len_t viaY = vc.canvasY;
            len_t viaX = vc.canvasX;
            assert(layer == viaLayer);

            assert(vc.upLLCell != nullptr);
            assert(vc.downLLCell != nullptr);
            assert(vc.upLRCell != nullptr);
            assert(vc.downLRCell != nullptr);
            assert(vc.upULCell != nullptr);
            assert(vc.downULCell != nullptr);
            assert(vc.upURCell != nullptr);
            assert(vc.downURCell != nullptr);

            if(vc.upLLCell != nullptr){
                assert(vc.upLLCell == &metalGrid[vc.upLLCellIdx]);
                assert(vc.upLLCell->canvasLayer == layer);
                assert(vc.upLLCell->canvasX == viaX);
                assert(vc.upLLCell->canvasY == viaY);

                assert(vc.upLLCell->downCell == &viaGrid[vc.upLLCell->downCellIdx]);
                assert(vc.upLLCell->downCell == &vc);
            }

            if(vc.downLLCell != nullptr){
                assert(vc.downLLCell == &metalGrid[vc.downLLCellIdx]);
                assert(vc.downLLCell->canvasLayer == (layer+1));
                assert(vc.downLLCell->canvasX == viaX);
                assert(vc.downLLCell->canvasY == viaY);

                assert(vc.downLLCell->upCell == &viaGrid[vc.downLLCell->upCellIdx]);
                assert(vc.downLLCell->upCell == &vc);
            }

            if(vc.upLRCell != nullptr){
                assert(vc.upLRCell == &metalGrid[vc.upLRCellIdx]);
                assert(vc.upLRCell->canvasLayer == layer);
                assert(vc.upLRCell->canvasX == (viaX+1));
                assert(vc.upLRCell->canvasY == viaY);

                assert(vc.upLRCell->downCell == &viaGrid[vc.upLRCell->downCellIdx]);
                assert(vc.upLRCell->downCell == &vc);
            }

            if(vc.downLRCell != nullptr){
                assert(vc.downLRCell == &metalGrid[vc.downLRCellIdx]);
                assert(vc.downLRCell->canvasLayer == (layer+1));
                assert(vc.downLRCell->canvasX == (viaX+1));
                assert(vc.downLRCell->canvasY == viaY);

                assert(vc.downLRCell->upCell == &viaGrid[vc.downLRCell->upCellIdx]);
                assert(vc.downLRCell->upCell == &vc);
            }

            if(vc.upULCell != nullptr){
                assert(vc.upULCell == &metalGrid[vc.upULCellIdx]);
                assert(vc.upULCell->canvasLayer == layer);
                assert(vc.upULCell->canvasX == viaX);
                assert(vc.upULCell->canvasY == (viaY+1));

                assert(vc.upULCell->downCell == &viaGrid[vc.upULCell->downCellIdx]);
                assert(vc.upULCell->downCell == &vc);
            }

            if(vc.downULCell != nullptr){
                assert(vc.downULCell == &metalGrid[vc.downULCellIdx]);
                assert(vc.downULCell->canvasLayer == (layer+1));
                assert(vc.downULCell->canvasX == viaX);
                assert(vc.downULCell->canvasY == (viaY+1));


                assert(vc.downULCell->upCell == &viaGrid[vc.downULCell->upCellIdx]);
                assert(vc.downULCell->upCell == &vc);

            }

            if(vc.upURCell != nullptr){
                assert(vc.upURCell == &metalGrid[vc.upURCellIdx]);
                assert(vc.upURCell->canvasLayer == layer);
                assert(vc.upURCell->canvasX == (viaX+1));
                assert(vc.upURCell->canvasY == (viaY+1));

                assert(vc.upURCell->downCell == &viaGrid[vc.upURCell->downCellIdx]);
                assert(vc.upURCell->downCell == &vc);
            }

            if(vc.downURCell != nullptr){
                assert(vc.downURCell == &metalGrid[vc.downURCellIdx]);
                assert(vc.downURCell->canvasLayer == (layer+1));
                assert(vc.downURCell->canvasX == (viaX+1));
                assert(vc.downURCell->canvasY == (viaY+1));

                assert(vc.downURCell->upCell == &viaGrid[vc.downULCell->upCellIdx]);
                assert(vc.downURCell->upCell == &vc);
            }
        }
    }
    
    std::cout << "Pass via connection check!" << std::endl;
}

void DiffusionEngine::checkNeighbors(){
    for(MetalCell &mc : metalGrid){
        
        assert(mc.neighbors.size() <= 6 && mc.neighbors.size() >= 0);
        if(mc.type != CellType::EMPTY){
            assert(mc.neighbors.empty());
        }else{
            if(mc.northCell != nullptr && mc.northCell->type == CellType::EMPTY){
                assert(std::count(mc.neighbors.begin(), mc.neighbors.end(), mc.northCell) == 1);
            }
            if(mc.southCell != nullptr && mc.southCell->type == CellType::EMPTY){
                assert(std::count(mc.neighbors.begin(), mc.neighbors.end(), mc.southCell) == 1);
            }
            if(mc.eastCell != nullptr && mc.eastCell->type == CellType::EMPTY){
                assert(std::count(mc.neighbors.begin(), mc.neighbors.end(), mc.eastCell) == 1);
            }
            if(mc.westCell != nullptr && mc.westCell->type == CellType::EMPTY){
                assert(std::count(mc.neighbors.begin(), mc.neighbors.end(), mc.westCell) == 1);
            }

            if(mc.upCell != nullptr && mc.upCell->type == CellType::EMPTY){
                assert(std::count(mc.neighbors.begin(), mc.neighbors.end(), mc.upCell) == 1);
            }
            if(mc.downCell != nullptr && mc.downCell->type == CellType::EMPTY){
                assert(std::count(mc.neighbors.begin(), mc.neighbors.end(), mc.downCell) == 1);
            }


        }

        for(DiffusionChamber *dc : mc.neighbors){
            MetalCell *nmc = static_cast<MetalCell *>(dc);
            ViaCell *nvc = static_cast<ViaCell *>(dc);
            assert((nmc == mc.northCell) || (nmc == mc.southCell) || (nmc == mc.eastCell) || (nmc == mc.westCell) || (nvc == mc.upCell) || (nvc == mc.downCell));
        }


    }
    std::cout << "Pass Metal Cells Check Neighbors test" << std::endl;

    for(ViaCell &vc : viaGrid){
        assert(vc.neighbors.size() <= 8 && vc.neighbors.size() >= 0); 
        for(DiffusionChamber *dc : vc.neighbors){
            MetalCell *nmc = static_cast<MetalCell *>(dc);
            assert((nmc == vc.upLLCell) || (dc == vc.upULCell) || (dc == vc.upLRCell) || (dc == vc.upURCell) ||
                   (nmc == vc.downLLCell) || (dc == vc.downULCell) || (dc == vc.downLRCell) || (dc == vc.downURCell) 
                );
        }
        if(vc.type != CellType::EMPTY){
            assert(vc.neighbors.empty());
        }else{
            if(vc.upLLCell->type == CellType::EMPTY){
                assert(std::count(vc.neighbors.begin(), vc.neighbors.end(), vc.upLLCell) == 1);
            }
            if(vc.upULCell->type == CellType::EMPTY){
                assert(std::count(vc.neighbors.begin(), vc.neighbors.end(), vc.upULCell) == 1);
            }
            if(vc.upLRCell->type == CellType::EMPTY){
                assert(std::count(vc.neighbors.begin(), vc.neighbors.end(), vc.upLRCell) == 1);
            }
            if(vc.upURCell->type == CellType::EMPTY){
                assert(std::count(vc.neighbors.begin(), vc.neighbors.end(), vc.upURCell) == 1);
            }

            if(vc.downLLCell->type == CellType::EMPTY){
                assert(std::count(vc.neighbors.begin(), vc.neighbors.end(), vc.downLLCell) == 1);
            }
            if(vc.downULCell->type == CellType::EMPTY){
                assert(std::count(vc.neighbors.begin(), vc.neighbors.end(), vc.downULCell) == 1);
            }
            if(vc.downLRCell->type == CellType::EMPTY){
                assert(std::count(vc.neighbors.begin(), vc.neighbors.end(), vc.downLRCell) == 1);
            }
            if(vc.downURCell->type == CellType::EMPTY){
                assert(std::count(vc.neighbors.begin(), vc.neighbors.end(), vc.downURCell) == 1);
            }
        }


    }
    std::cout << "Pass Via Cells Check Neighbors test" << std::endl;

    
}
