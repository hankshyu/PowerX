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
                    cell.type = (st == SignalType::OBSTACLE)? CellType::OBSTACLES : CellType::PREPLACED;
                }

                // add neighbors
                if(j != (m_cellGridHeight - 1)){
                    SignalType northNieghborSt = metalLayers[metalLayer].canvas[j+1][i];
                    size_t northCellIdx = cellIndex + m_cellGridWidth;
                    MetalCell *northCellPointer = &cellGrid[northCellIdx];
                    cell.northCell = northCellPointer;
                    cell.northCellIdx = northCellIdx;
                    addDirection(cell.fullDirection, DirFlagAxis::NORTH);
                    if(northNieghborSt == SignalType::EMPTY || northNieghborSt == st){
                        cell.metalCellNeighbors.push_back(northCellPointer);
                    }
                }

                if(j != 0){
                    SignalType southNieghborSt = metalLayers[metalLayer].canvas[j-1][i];
                    size_t southCellIdx = cellIndex - m_cellGridWidth;
                    MetalCell *southCellPointer = &cellGrid[southCellIdx];
                    cell.southCell = southCellPointer;
                    cell.southCellIdx = southCellIdx;
                    addDirection(cell.fullDirection, DirFlagAxis::SOUTH);
                    if(southNieghborSt == SignalType::EMPTY || southNieghborSt == st){
                        cell.metalCellNeighbors.push_back(southCellPointer);
                    }
                }

                if(i != 0){
                    SignalType westNieghborSt = metalLayers[metalLayer].canvas[j][i-1];
                    size_t westCellIdx = cellIndex - 1;
                    MetalCell *westCellPointer = &cellGrid[westCellIdx];
                    cell.westCell = westCellPointer;
                    cell.westCellIdx = westCellIdx;
                    addDirection(cell.fullDirection, DirFlagAxis::WEST);
                    if(westNieghborSt == SignalType::EMPTY || westNieghborSt == st){
                        cell.metalCellNeighbors.push_back(westCellPointer);
                    }
                }

                if(i != (m_cellGridWidth - 1)){
                    SignalType eastNieghborSt = metalLayers[metalLayer].canvas[j][i+1];
                    size_t eastCellIdx = cellIndex + 1;
                    MetalCell *eastCellPointer = &cellGrid[eastCellIdx];
                    cell.eastCell = eastCellPointer;
                    cell.eastCellIdx = eastCellIdx;
                    addDirection(cell.fullDirection, DirFlagAxis::EAST);
                    if(eastNieghborSt == SignalType::EMPTY || eastNieghborSt == st){
                        cell.metalCellNeighbors.push_back(eastCellPointer);
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
                    cellType = CellType::EMPTY;

                }else{ // preplaced power signals
                    cellType = CellType::PREPLACED;
                    cell.signal = st;
                }
                cell.type = cellType;

                // Link Up direction LL cell
                size_t upLLCellIdx = calMetalIdx(viaLayer, j, i);
                SignalType upLLCellSt = metalLayers[viaLayer].canvas[j][i];
                MetalCell *upLLCellPointer = &cellGrid[upLLCellIdx];
                CellType upLLCellType = upLLCellPointer->type;
                
                // modify this via cell
                cell.upLLCell = upLLCellPointer;
                cell.upLLCellIdx = upLLCellIdx; 
                addDirection(cell.fullDirection, DirFlagViaAxis::UPLL);

                // modify the metal cell via cell is linking to
                upLLCellPointer->downCell = &cell;
                upLLCellPointer->downCellIdx = cellIndex;
                addDirection(upLLCellPointer->fullDirection, DirFlagAxis::DOWN);
                
                if((upLLCellType != CellType::OBSTACLES) && ((cellType != CellType::PREPLACED) || (upLLCellType != CellType::PREPLACED) || (st == upLLCellSt))){
                    cell.neighbors.push_back(upLLCellPointer);
                    upLLCellPointer->viaCellNeighbors.push_back(&cell);
                }


                // Link Up direction LR cell
                size_t upLRCellIdx = upLLCellIdx + 1;
                SignalType upLRCellSt = metalLayers[viaLayer].canvas[j][i+1];
                MetalCell *upLRCellPointer = &cellGrid[upLRCellIdx];
                CellType upLRCellType = upLRCellPointer->type;

                // modify this via cell
                cell.upLRCell = upLRCellPointer;
                cell.upLRCellIdx = upLRCellIdx;
                addDirection(cell.fullDirection, DirFlagViaAxis::UPLR);

                // modify the metal cell via cell is linking to
                upLRCellPointer->downCell = &cell;
                upLRCellPointer->downCellIdx = cellIndex;
                addDirection(upLRCellPointer->fullDirection, DirFlagAxis::DOWN);

                if((upLRCellType != CellType::OBSTACLES) && ((cellType != CellType::PREPLACED) || (upLLCellType != CellType::PREPLACED) || (st == upLRCellSt))){
                    cell.neighbors.push_back(upLRCellPointer);
                    upLRCellPointer->viaCellNeighbors.push_back(&cell);
                }


                // Link Up direction UL cell
                size_t upULCellIdx = upLLCellIdx + m_cellGridWidth;
                SignalType upULCellSt = metalLayers[viaLayer].canvas[j+1][i];
                MetalCell *upULCellPointer = &cellGrid[upULCellIdx];
                CellType upULCellType = upULCellPointer->type;

                // modify this via cell
                cell.upULCell = upULCellPointer;
                cell.upULCellIdx = upULCellIdx;
                addDirection(cell.fullDirection, DirFlagViaAxis::UPUL);

                // modify the metal cell via cell is linking to
                upULCellPointer->downCell = &cell;
                upULCellPointer->downCellIdx = cellIndex;
                addDirection(upULCellPointer->fullDirection, DirFlagAxis::DOWN);

                if((upULCellType != CellType::OBSTACLES) && ((cellType != CellType::PREPLACED) || (upULCellType != CellType::PREPLACED) || (st == upULCellSt))){
                    cell.neighbors.push_back(upULCellPointer);
                    upULCellPointer->viaCellNeighbors.push_back(&cell);
                }


                // Link Up direction UR cell
                size_t upURCellIdx = upULCellIdx + 1;
                SignalType upURCellSt = metalLayers[viaLayer].canvas[j+1][i+1];
                MetalCell *upURCellPointer = &cellGrid[upURCellIdx];
                CellType upURCellType = upURCellPointer->type;

                // modify this via cell
                cell.upURCell = upURCellPointer;
                cell.upURCellIdx = upURCellIdx;
                addDirection(cell.fullDirection, DirFlagViaAxis::UPUR);

                // modify the metal cell via cell is linking to
                upURCellPointer->downCell = &cell;
                upURCellPointer->downCellIdx = cellIndex;
                addDirection(upURCellPointer->fullDirection, DirFlagAxis::DOWN);

                if((upURCellType != CellType::OBSTACLES) && ((cellType != CellType::PREPLACED) || (upURCellType != CellType::PREPLACED) || (st == upURCellSt))){
                    cell.neighbors.push_back(upURCellPointer);
                    upURCellPointer->viaCellNeighbors.push_back(&cell);
                }


                // Link Down direction LL cell
                size_t downLLCellIdx = upLLCellIdx + m_cellGrid2DCount;
                SignalType downLLCellSt = metalLayers[viaLayer+1].canvas[j][i];
                MetalCell *downLLCellPointer = &cellGrid[downLLCellIdx];
                CellType downLLCellType = downLLCellPointer->type;

                // modify this via cell
                cell.downLLCell = downLLCellPointer;
                cell.downLLCellIdx = downLLCellIdx;
                addDirection(cell.fullDirection, DirFlagViaAxis::DOWNLL);

                // modify the metal cell via cell is linking to
                downLLCellPointer->upCell = &cell;
                downLLCellPointer->upCellIdx = cellIndex;
                addDirection(downLLCellPointer->fullDirection, DirFlagAxis::UP);

                if((downLLCellType != CellType::OBSTACLES) && ((cellType != CellType::PREPLACED) || (downLLCellType != CellType::PREPLACED) || (st == downLLCellSt))){
                    cell.neighbors.push_back(downLLCellPointer);
                    downLLCellPointer->viaCellNeighbors.push_back(&cell);
                }


                // Link Down direction LR cell
                size_t downLRCellIdx = downLLCellIdx + 1;
                SignalType downLRCellSt = metalLayers[viaLayer+1].canvas[j][i+1];
                MetalCell *downLRCellPointer = &cellGrid[downLRCellIdx];
                CellType downLRCellType = downLRCellPointer->type;

                // modify this via cell
                cell.downLRCell = downLRCellPointer;
                cell.downLRCellIdx = downLRCellIdx;
                addDirection(cell.fullDirection, DirFlagViaAxis::DOWNLR);

                // modify the metal cell via cell is linking to
                downLRCellPointer->upCell = &cell;
                downLRCellPointer->upCellIdx = cellIndex;
                addDirection(downLRCellPointer->fullDirection, DirFlagAxis::UP);

                if((downLRCellType != CellType::OBSTACLES) && ((cellType != CellType::PREPLACED) || (downLRCellType != CellType::PREPLACED) || (st == downLRCellSt))){
                    downLRCellPointer->viaCellNeighbors.push_back(&cell);
                    cell.neighbors.push_back(downLRCellPointer);
                }



                // Link Down direction UL cell
                size_t downULCellIdx = downLLCellIdx + m_cellGridWidth;
                SignalType downULCellSt = metalLayers[viaLayer+1].canvas[j+1][i];
                MetalCell *downULCellPointer = &cellGrid[downULCellIdx];
                CellType downULCellType = downULCellPointer->type;

                // modify this via cell
                cell.downULCell = downULCellPointer;
                cell.downULCellIdx = downULCellIdx;
                addDirection(cell.fullDirection, DirFlagViaAxis::DOWNUL);

                // modify the metal cell via cell is linking to
                downULCellPointer->upCell = &cell;
                downULCellPointer->upCellIdx = cellIndex;
                addDirection(downULCellPointer->fullDirection, DirFlagAxis::UP);

                if((downULCellType != CellType::OBSTACLES) && ((cellType != CellType::PREPLACED) || (downULCellType != CellType::PREPLACED) || (st == downULCellSt))){
                    cell.neighbors.push_back(downULCellPointer);
                    downULCellPointer->viaCellNeighbors.push_back(&cell);
                }

                // Link Down direction UR cell
                size_t downURCellIdx = downULCellIdx + 1;
                SignalType downURCellSt = metalLayers[viaLayer+1].canvas[j+1][i+1];
                MetalCell *downURCellPointer = &cellGrid[downURCellIdx];
                CellType downURCellType = downURCellPointer->type;

                // modify this via cell
                cell.downURCell = downURCellPointer;
                cell.downURCellIdx = downURCellIdx;
                addDirection(cell.fullDirection, DirFlagViaAxis::DOWNUR);

                // modify the metal cell via cell is linking to
                downURCellPointer->upCell = &cell;
                downURCellPointer->upCellIdx = cellIndex;
                addDirection(downURCellPointer->fullDirection, DirFlagAxis::UP);


                if((downURCellType != CellType::OBSTACLES) && ((cellType != CellType::PREPLACED) || (downURCellType != CellType::PREPLACED) || (st == downURCellSt))){
                    cell.neighbors.push_back(downURCellPointer);
                    downURCellPointer->viaCellNeighbors.push_back(&cell);
                }

                // increment cellIndex for the next available
                cellIndex++;
            }
        }
        
        m_viaGrid2DAccumlateCount.push_back(cellIndex);
        m_viaGrid2DCount.push_back((viaLayer == 0)? cellIndex : (cellIndex - m_viaGrid2DAccumlateCount.back()));
    }
}

void DiffusionEngine::fillEnclosedRegions(){
    
    for(int layer = 0; layer < m_cellGridLayers; ++layer){
        std::vector<std::vector<bool>> visited (m_cellGridHeight, std::vector<bool>(m_cellGridWidth, false));
        for(int y = 0; y < m_cellGridHeight; ++y){
            for(int x = 0; x < m_cellGridWidth; ++x){
                size_t cellIdx = calMetalIdx(layer, y, x);
                MetalCell *cellPointer = &cellGrid[cellIdx];
                
                if(cellPointer->type != CellType::EMPTY || visited[y][x]) continue;
                
                SignalType cellSignalType = cellPointer->signal;

                std::queue<Cord> q;
                std::vector<Cord> region;
                std::unordered_set<SignalType> borderSignals;
                bool touchesBoundary = false;

                q.push(Cord(x, y));
                visited[y][x] = true;
                region.emplace_back(x, y);

                while(!q.empty()){
                    Cord c = q.front(); 
                    q.pop();

                    std::vector<MetalCell *> neighbors;
                    if(cellPointer->northCell != nullptr) neighbors.push_back(cellPointer->northCell);
                    if(cellPointer->southCell != nullptr) neighbors.push_back(cellPointer->southCell);
                    if(cellPointer->eastCell != nullptr) neighbors.push_back(cellPointer->eastCell);
                    if(cellPointer->westCell != nullptr) neighbors.push_back(cellPointer->westCell);

                    for(MetalCell *mc : neighbors){
                        SignalType mcType = mc->signal;
                        if((mcType == SignalType::EMPTY) && (!visited[mc->canvasY][mc->canvasX])){
                            visited[mc->canvasY][mc->canvasX] = true;
                            q.emplace(mc->canvasX, mc->canvasY);
                            region.emplace_back(mc->canvasX, mc->canvasY);

                        }else if(cellSignalType != SignalType::EMPTY && mcType != SignalType::OBSTACLE){
                            borderSignals.insert(mcType);
                        }
                    }
                }

                if(borderSignals.size() == 1){
                    SignalType fillType = *borderSignals.begin();
                    // fill in the borders
                    for(const Cord &p : region){
                        size_t fillCellIdx = calMetalIdx(layer, p.y(), p.x());
                        MetalCell &fillCell = cellGrid[fillCellIdx];
                        fillCell.type = CellType::MARKED;
                        fillCell.signal = fillType;
                    }
                }
            }
        }
    }
}

void DiffusionEngine::markHalfOccupiedMetalsAndPins(){

    for(ViaCell &vc : this->viaGrid){
        SignalType vcSignal = vc.signal;
        CellType vcCellType = vc.type;
        if(vcSignal == SignalType::OBSTACLES) continue;

        std::vector<MetalCell *> neighbors = {
            vc.upLLCell, vc.upULCell, vc.upLRCell, vc.upURCell,
            vc.downLLCell, vc.downULCell, vc.downLRCell, vc.downURCell
        };

        if(vcCellType == CellType::EMPTY){
            // check if there is only one type of preplace signal
            SignalType preplacedSig = SignalType::EMPTY;
            for(MetalCell *mc : neighbors){
                assert(mc != nullptr);
                if(mc->type == CellType::PREPLACED){
                   if(preplacedSig != SignalType::EMPTY){
                    preplacedSig = SignalType::EMPTY;
                    break;
                   } 
                   preplacedSig = mc->signal;
                }
            }
            if(preplacedSig == SignalType::EMPTY) continue;

            // process those which has only one prepace signal type
            vc.signal = preplacedSig; 
            vc.type = CellType::MARKED;
            for(MetalCell *mc : neighbors){
                if(mc->type == CellType::EMPTY){
                    mc->type = CellType::MARKED;
                    mc->signal = vcSignal;
                }
            }

        }else if(POWER_SIGNAL_SET.count(vcSignal) != 0){ // the via has as signal type
            for(MetalCell *mc : neighbors){
                if(mc->type == CellType::EMPTY){
                    mc->type = CellType::MARKED;
                    mc->signal = vcSignal;
                }
            }
        }   

    }
}

void DiffusionEngine::linkNeighbors(){
    // link neighbors of those metal cells
    for(MetalCell &mc : this->cellGrid){

    }


    // link neighbors of those via cells
}



