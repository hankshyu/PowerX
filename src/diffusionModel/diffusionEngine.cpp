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
// 2. Boost Library:

// 3. Texo Library:
#include "diffusionEngine.hpp"

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
                if(st == SignalType::OBSTACLE || st == SignalType::SIGNAL) continue;
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
                size_t upLLCellIdx = calMetalIdx(viaLayer, j, i);
                SignalType upLLCellSt = metalLayers[viaLayer].canvas[j][i];
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
                size_t upLRCellIdx = upLLCellIdx + 1;
                SignalType upLRCellSt = metalLayers[viaLayer].canvas[j][i+1];
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
                size_t upULCellIdx = upLLCellIdx + m_metalGridWidth;
                SignalType upULCellSt = metalLayers[viaLayer].canvas[j+1][i];
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
                size_t upURCellIdx = upULCellIdx + 1;
                SignalType upURCellSt = metalLayers[viaLayer].canvas[j+1][i+1];
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
                size_t downLLCellIdx = upLLCellIdx + m_metalGrid2DCount;
                SignalType downLLCellSt = metalLayers[viaLayer+1].canvas[j][i];
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
                size_t downLRCellIdx = downLLCellIdx + 1;
                SignalType downLRCellSt = metalLayers[viaLayer+1].canvas[j][i+1];
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
                size_t downULCellIdx = downLLCellIdx + m_metalGridWidth;
                SignalType downULCellSt = metalLayers[viaLayer+1].canvas[j+1][i];
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
                size_t downURCellIdx = downULCellIdx + 1;
                SignalType downURCellSt = metalLayers[viaLayer+1].canvas[j+1][i+1];
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

void DiffusionEngine::fillEnclosedRegions(){
    
    for(int layer = 0; layer < m_metalGridLayers; ++layer){
        std::vector<std::vector<bool>> visited (m_metalGridHeight, std::vector<bool>(m_metalGridWidth, false));
        for(int y = 0; y < m_metalGridHeight; ++y){
            for(int x = 0; x < m_metalGridWidth; ++x){
                size_t cellIdx = calMetalIdx(layer, y, x);
                MetalCell *cellPointer = &metalGrid[cellIdx];
                
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
    
    // a link goes from empty -> empty
    for(MetalCell &mc : this->metalGrid){
        mc.metalCellNeighbors.clear();
        mc.viaCellNeighbors.clear();
    }
    for(ViaCell &vc : this->viaGrid){
        vc.neighbors.clear();
    }

    // link 2D metal layer linkings
    for(MetalCell &mc : this->metalGrid){
        if(mc.type != CellType::EMPTY) continue;

        MetalCell *mcPointer = &mc;
        if((mc.northCell != nullptr) && (mc.northCell->type == CellType::EMPTY)){
            mc.metalCellNeighbors.push_back(mc.northCell);
            mc.northCell->metalCellNeighbors.push_back(mcPointer);
        }
        if((mc.southCell != nullptr) && (mc.southCell->type == CellType::EMPTY)){
            mc.metalCellNeighbors.push_back(mc.southCell);
            mc.southCell->metalCellNeighbors.push_back(mcPointer);
        }
        if((mc.eastCell != nullptr) && (mc.eastCell->type == CellType::EMPTY)){
            mc.metalCellNeighbors.push_back(mc.eastCell);
            mc.eastCell->metalCellNeighbors.push_back(mcPointer);
        }
        if((mc.westCell != nullptr) && (mc.westCell->type == CellType::EMPTY)){
            mc.metalCellNeighbors.push_back(mc.westCell);
            mc.westCell->metalCellNeighbors.push_back(mcPointer);
        }

    }

    // link via related linkings
    for(ViaCell &vc : this->viaGrid){
        if(vc.type != CellType::EMPTY) continue;

        ViaCell *vcPointer = &vc;
        
        if((vc.upLLCell != nullptr) && (vc.upLLCell->type == CellType::EMPTY)){
            vc.neighbors.push_back(vc.upLLCell);
            vc.upLLCell->viaCellNeighbors.push_back(vcPointer);
        }
        if((vc.upULCell != nullptr) && (vc.upULCell->type != CellType::EMPTY)){
            vc.neighbors.push_back(vc.upULCell);
            vc.upULCell->viaCellNeighbors.push_back(vcPointer);
        }
        if((vc.upLRCell != nullptr) && (vc.upLRCell->type != CellType::EMPTY)){
            vc.neighbors.push_back(vc.upLRCell);
            vc.upLRCell->viaCellNeighbors.push_back(vcPointer);
        }
        if((vc.upURCell != nullptr) && (vc.upURCell->type != CellType::EMPTY)){
            vc.neighbors.push_back(vc.upURCell);
            vc.upURCell->viaCellNeighbors.push_back(vcPointer);
        }

        if((vc.downLLCell != nullptr) && (vc.downLLCell->type == CellType::EMPTY)){
            vc.neighbors.push_back(vc.downLLCell);
            vc.downLLCell->viaCellNeighbors.push_back(vcPointer);
        }
        if((vc.downULCell != nullptr) && (vc.downULCell->type != CellType::EMPTY)){
            vc.neighbors.push_back(vc.downULCell);
            vc.downULCell->viaCellNeighbors.push_back(vcPointer);
        }
        if((vc.downLRCell != nullptr) && (vc.downLRCell->type != CellType::EMPTY)){
            vc.neighbors.push_back(vc.downLRCell);
            vc.downLRCell->viaCellNeighbors.push_back(vcPointer);
        }
        if((vc.downURCell != nullptr) && (vc.downURCell->type != CellType::EMPTY)){
            vc.neighbors.push_back(vc.downURCell);
            vc.downURCell->viaCellNeighbors.push_back(vcPointer);
        } 
    }
}

void DiffusionEngine::initialiseIndexing(){
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
            if((mc.type == CellType::EMPTY) || (mc.type == CellType::OBSTACLES) || metalVisited[cellIdx]) continue;
            dcPointer = &mc;
            paintingType = mc.signal;
        }else{
            ViaCell &vc = this->viaGrid[viaIdx];
            if((vc.type == CellType::EMPTY) || (vc.type == CellType::OBSTACLES) || viaVisited[viaIdx]) continue;
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
            dc->addParticles(cl, 1000);
        }
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