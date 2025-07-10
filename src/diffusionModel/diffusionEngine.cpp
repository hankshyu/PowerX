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

    // transfer the marking of metal layer onto chamber-related metal data structures, attributes to fill:
    // signal, fullDirection
    // cellGridType[idx]
    // canvasMetalLayer, canvasMetalX, canvasMetalY
    // up/down/left/right *MetalCell and idx
    // metalCellNeighbors[]
    
    cellGrid.resize(m_cellGrid3DCount);
    cellGridType.resize(m_cellGrid3DCount, CellType::EMPTY);

    for(int metalLayer = 0; metalLayer < m_metalLayerCount; ++metalLayer){
        for(int j = 0; j < m_gridHeight; ++j){
            for(int i = 0; i < m_gridWidth; ++i){

                size_t cellIndex = calMetalIdx(metalLayer, j, i);
                MetalCell &cell = cellGrid[cellIndex];
                SignalType st = metalLayers[metalLayer].canvas[j][i];
                
                cell.index = cellIndex;
                cell.metalViaType = DiffusionChamberType::METAL;
                cell.canvasLayer = metalLayer;
                cell.canvasY = j;
                cell.canvasX = i;
                
                if(st != SignalType::EMPTY){
                    cell.signal = st;
                    cellGridType[cellIndex] = (st == SignalType::OBSTACLE)? CellType::OBSTACLES : CellType::PREPLACED;
                }

                // add neighbors
                if(j != (m_cellGridHeight - 1)){
                    SignalType northNieghborSt = metalLayers[metalLayer].canvas[j+1][i];
                    if(northNieghborSt == SignalType::EMPTY || northNieghborSt == st){
                        size_t northCellIdx = cellIndex + m_cellGridWidth;
                        MetalCell *northCellPointer = &cellGrid[northCellIdx];
                        cell.northCell = northCellPointer;
                        cell.metalCellNeighbors.push_back(northCellPointer);
                        cell.northCellIdx = northCellIdx;
                        addDirection(cell.fullDirection, DirFlagAxis::NORTH);
                    }
                }

                if(j != 0){
                    SignalType southNieghborSt = metalLayers[metalLayer].canvas[j-1][i];
                    if(southNieghborSt == SignalType::EMPTY || southNieghborSt == st){
                        size_t southCellIdx = cellIndex - m_cellGridWidth;
                        MetalCell *southCellPointer = &cellGrid[southCellIdx];
                        cell.southCell = southCellPointer;
                        cell.metalCellNeighbors.push_back(southCellPointer);
                        cell.southCellIdx = southCellIdx;
                        addDirection(cell.fullDirection, DirFlagAxis::SOUTH);
                    }
                }

                if(i != 0){
                    SignalType westNieghborSt = metalLayers[metalLayer].canvas[j][i-1];
                    if(westNieghborSt == SignalType::EMPTY || westNieghborSt == st){
                        size_t westCellIdx = cellIndex - 1;
                        MetalCell *westCellPointer = &cellGrid[westCellIdx];
                        cell.westCell = westCellPointer;
                        cell.metalCellNeighbors.push_back(westCellPointer);
                        cell.westCellIdx = westCellIdx;
                        addDirection(cell.fullDirection, DirFlagAxis::WEST);
                    }
                }

                if(i != (m_cellGridWidth - 1)){
                    SignalType eastNieghborSt = metalLayers[metalLayer].canvas[j][i+1];
                    if(eastNieghborSt == SignalType::EMPTY || eastNieghborSt == st){
                        size_t eastCellIdx = cellIndex + 1;
                        MetalCell *eastCellPointer = &cellGrid[eastCellIdx];
                        cell.eastCell = eastCellPointer;
                        cell.metalCellNeighbors.push_back(eastCellPointer);
                        cell.eastCellIdx = eastCellIdx;
                        addDirection(cell.fullDirection, DirFlagAxis::EAST);
                    }
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

                // use default constructor
                viaGrid.emplace_back();
                ViaCell &cell = viaGrid.back();
                
                cell.index = cellIndex;
                cell.metalViaType = DiffusionChamberType::VIA;
                cell.canvasLayer = viaLayer;
                cell.canvasY = j;
                cell.canvasX = i;

                CellType cellType;
                if(st == SignalType::EMPTY){
                    viaGridType.push_back(CellType::EMPTY);
                    cellType =  CellType::EMPTY;

                }else{ // preplaced power signals
                    viaGridType.push_back(CellType::PREPLACED);
                    cellType = CellType::PREPLACED;
                    cell.signal = st;
                }

                // Link Up direction LL cell
                size_t upLLCellIdx = calMetalIdx(viaLayer, j, i);
                CellType upLLCellType = cellGridType[upLLCellIdx];
                if(upLLCellType != CellType::OBSTACLES){
                    SignalType upLLCellSt = metalLayers[viaLayer].canvas[j][i];
                    if((cellType != CellType::PREPLACED) || (upLLCellType != CellType::PREPLACED) || (st == upLLCellSt)){
                        MetalCell *upLLCellPointer = &cellGrid[upLLCellIdx];

                        // modify this via cell
                        cell.upLLCell = upLLCellPointer;
                        cell.neighbors.push_back(upLLCellPointer);
                        cell.upLLCellIdx = upLLCellIdx; 
                        addDirection(cell.fullDirection, DirFlagViaAxis::UPLL);

                        // modify the metal cell via cell is linking to
                        upLLCellPointer->downCell = &cell;
                        upLLCellPointer->viaCellNeighbors.push_back(&cell);
                        upLLCellPointer->downCellIdx = cellIndex;
                        addDirection(upLLCellPointer->fullDirection, DirFlagAxis::DOWN);
                    }
                }

                // Link Up direction LR cell
                size_t upLRCellIdx = upLLCellIdx + 1;
                CellType upLRCellType = cellGridType[upLRCellIdx];
                if(upLRCellType != CellType::OBSTACLES){
                    SignalType upLRCellSt = metalLayers[viaLayer].canvas[j][i+1];
                    if((cellType != CellType::PREPLACED) || (upLLCellType != CellType::PREPLACED) || (st == upLRCellSt)){
                        MetalCell *upLRCellPointer = &cellGrid[upLRCellIdx];

                        // modify this via cell
                        cell.upLRCell = upLRCellPointer;
                        cell.neighbors.push_back(upLRCellPointer);
                        cell.upLRCellIdx = upLRCellIdx;
                        addDirection(cell.fullDirection, DirFlagViaAxis::UPLR);

                        // modify the metal cell via cell is linking to
                        upLRCellPointer->downCell = &cell;
                        upLRCellPointer->viaCellNeighbors.push_back(&cell);
                        upLRCellPointer->downCellIdx = cellIndex;
                        addDirection(upLRCellPointer->fullDirection, DirFlagAxis::DOWN);
                    }
                }

                // Link Up direction UL cell
                size_t upULCellIdx = upLLCellIdx + m_cellGridWidth;
                CellType upULCellType = cellGridType[upULCellIdx];
                if(upULCellType != CellType::OBSTACLES){
                    SignalType upULCellSt = metalLayers[viaLayer].canvas[j+1][i];
                    if((cellType != CellType::PREPLACED) || (upULCellType != CellType::PREPLACED) || (st == upULCellSt)){
                        MetalCell *upULCellPointer = &cellGrid[upULCellIdx];

                        // modify this via cell
                        cell.upULCell = upULCellPointer;
                        cell.neighbors.push_back(upULCellPointer);
                        cell.upLRCellIdx = upULCellIdx;
                        addDirection(cell.fullDirection, DirFlagViaAxis::UPUL);

                        // modify the metal cell via cell is linking to
                        upULCellPointer->downCell = &cell;
                        upULCellPointer->viaCellNeighbors.push_back(&cell);
                        upULCellPointer->downCellIdx = cellIndex;
                        addDirection(upULCellPointer->fullDirection, DirFlagAxis::DOWN);
                    }
                }

                // Link Up direction UR cell
                size_t upURCellIdx = upULCellIdx + 1;
                CellType upURCellType = cellGridType[upURCellIdx];
                if(upURCellType != CellType::OBSTACLES){
                    SignalType upURCellSt = metalLayers[viaLayer].canvas[j+1][i+1];
                    if((cellType != CellType::PREPLACED) || (upURCellType != CellType::PREPLACED) || (st == upURCellSt)){
                        MetalCell *upURCellPointer = &cellGrid[upURCellIdx];

                        // modify this via cell
                        cell.upURCell = upURCellPointer;
                        cell.neighbors.push_back(upURCellPointer);
                        cell.upLRCellIdx = upURCellIdx;
                        addDirection(cell.fullDirection, DirFlagViaAxis::UPUR);

                        // modify the metal cell via cell is linking to
                        upURCellPointer->downCell = &cell;
                        upURCellPointer->viaCellNeighbors.push_back(&cell);
                        upURCellPointer->downCellIdx = cellIndex;
                        addDirection(upURCellPointer->fullDirection, DirFlagAxis::DOWN);
                    }
                }

                // Link Down direction LL cell
                size_t downLLCellIdx = upLLCellIdx + m_cellGrid2DCount;
                CellType downLLCellType = cellGridType[downLLCellIdx];
                if(downLLCellType != CellType::OBSTACLES){
                    SignalType downLLCellSt = metalLayers[viaLayer+1].canvas[j][i];
                    if((cellType != CellType::PREPLACED) || (downLLCellType != CellType::PREPLACED) || (st == downLLCellSt)){
                        MetalCell *downLLCellPointer = &cellGrid[downLLCellIdx];

                        // modify this via cell
                        cell.downLLCell = downLLCellPointer;
                        cell.neighbors.push_back(downLLCellPointer);
                        cell.downLLCellIdx = downLLCellIdx;
                        addDirection(cell.fullDirection, DirFlagViaAxis::DOWNLL);

                        // modify the metal cell via cell is linking to
                        downLLCellPointer->upCell = &cell;
                        downLLCellPointer->viaCellNeighbors.push_back(&cell);
                        downLLCellPointer->upCellIdx = cellIndex;
                        addDirection(downLLCellPointer->fullDirection, DirFlagAxis::UP);
                    }
                }

                // Link Down direction LR cell
                size_t downLRCellIdx = downLLCellIdx + 1;
                CellType downLRCellType = cellGridType[downLRCellIdx];
                if(downLRCellType != CellType::OBSTACLES){
                    SignalType downLRCellSt = metalLayers[viaLayer+1].canvas[j][i+1];
                    if((cellType != CellType::PREPLACED) || (downLRCellType != CellType::PREPLACED) || (st == downLRCellSt)){
                        MetalCell *downLRCellPointer = &cellGrid[downLRCellIdx];

                        // modify this via cell
                        cell.downLRCell = downLRCellPointer;
                        cell.neighbors.push_back(downLRCellPointer);
                        cell.downLRCellIdx = downLRCellIdx;
                        addDirection(cell.fullDirection, DirFlagViaAxis::DOWNLR);

                        // modify the metal cell via cell is linking to
                        downLRCellPointer->upCell = &cell;
                        downLRCellPointer->viaCellNeighbors.push_back(&cell);
                        downLRCellPointer->upCellIdx = cellIndex;
                        addDirection(downLRCellPointer->fullDirection, DirFlagAxis::UP);
                    }
                }

                // Link Down direction UL cell
                size_t downULCellIdx = downLLCellIdx + m_cellGridWidth;
                CellType downULCellType = cellGridType[downULCellIdx];
                if(downULCellType != CellType::OBSTACLES){
                    SignalType downULCellSt = metalLayers[viaLayer+1].canvas[j+1][i];
                    if((cellType != CellType::PREPLACED) || (downULCellType != CellType::PREPLACED) || (st == downULCellSt)){
                        MetalCell *downULCellPointer = &cellGrid[downULCellIdx];

                        // modify this via cell
                        cell.downULCell = downULCellPointer;
                        cell.neighbors.push_back(downULCellPointer);
                        cell.downULCellIdx = downULCellIdx;
                        addDirection(cell.fullDirection, DirFlagViaAxis::DOWNUL);

                        // modify the metal cell via cell is linking to
                        downULCellPointer->upCell = &cell;
                        downULCellPointer->viaCellNeighbors.push_back(&cell);
                        downULCellPointer->upCellIdx = cellIndex;
                        addDirection(downULCellPointer->fullDirection, DirFlagAxis::UP);
                    }
                }

                // Link Down direction UR cell
                size_t downURCellIdx = downULCellIdx + 1;
                CellType downURCellType = cellGridType[downURCellIdx];
                if(downURCellType != CellType::OBSTACLES){
                    SignalType downURCellSt = metalLayers[viaLayer+1].canvas[j+1][i+1];
                    if((cellType != CellType::PREPLACED) || (downURCellType != CellType::PREPLACED) || (st == downURCellSt)){
                        MetalCell *downURCellPointer = &cellGrid[downURCellIdx];

                        // modify this via cell
                        cell.downURCell = downURCellPointer;
                        cell.neighbors.push_back(downURCellPointer);
                        cell.downURCellIdx = downURCellIdx;
                        addDirection(cell.fullDirection, DirFlagViaAxis::DOWNUR);

                        // modify the metal cell via cell is linking to
                        downURCellPointer->upCell = &cell;
                        downURCellPointer->viaCellNeighbors.push_back(&cell);
                        downURCellPointer->upCellIdx = cellIndex;
                        addDirection(downURCellPointer->fullDirection, DirFlagAxis::UP);
                    }
                }

                // increment cellIndex for the next available
                cellIndex++;

            }
        }
        
        m_viaGrid2DAccumlateCount.push_back(cellIndex);
        m_viaGrid2DCount.push_back((viaLayer == 0)? cellIndex : (cellIndex - m_viaGrid2DAccumlateCount.back()));
    }
}

void fillEnclosedRegions();





