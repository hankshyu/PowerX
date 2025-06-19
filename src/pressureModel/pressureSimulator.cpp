//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        06/04/2025 13:34:29
//  Module Name:        pressureSimulator.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        The top module of the pressure growing system
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
/////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:
#include <vector>
#include <unordered_map>


// 2. Boost Library:
#include "boost/geometry.hpp"

// 3. Texo Library:
#include "signalType.hpp"
#include "powerDistributionNetwork.hpp"

#include "fpoint.hpp"
#include "fbox.hpp"
#include "fpolygon.hpp"
#include "fmultipolygon.hpp"
#include "softBody.hpp"
#include "viaBody.hpp"
#include "pressureSimulator.hpp"
#include "pointBinSystem.hpp"

PressureSimulator::PressureSimulator(const std::string &fileName): PowerDistributionNetwork(fileName) {
    // the canvas size is the same width the pin canvas
    m_canvasWidth = m_pinWidth;
    m_canvasHeight = m_pinHeight;
    
    int softBodyIdx = 0;
    int viaBodyIdx = 0;
    const flen_t INITIAL_MARGIN = 0.5; // be a value between 0 ~ 0.5
    const flen_t POINT_BIN_SIZE = 2.0;
    const flen_t RECTANGLE_BIN_SIZE = 5.0;

    
    this->m_OwnerSoftBodies.resize(m_metalLayerCount);

    this->softBodyRectangleBin.resize(m_metalLayerCount, RectangleBinSystem<flen_t, SoftBody>(RECTANGLE_BIN_SIZE, 0, 0, m_gridWidth, m_gridHeight));
    this->softBodyPointBin.resize(m_metalLayerCount, PointBinSystem<flen_t, SoftBody>(POINT_BIN_SIZE, 0, 0, m_gridWidth, m_gridHeight));

    /* Insert SoftBodies */

    std::unordered_set<SignalType> currentRequiringSignals;
    std::unordered_map<SignalType, double> signalCurrentRequirements;
    
    std::vector<std::unordered_map<SignalType, std::vector<FBox>>> layerInitBoxes(m_metalLayerCount);

    // Porcess Ubump related informations
    for(auto cit = uBump.signalTypeToInstances.begin(); cit != uBump.signalTypeToInstances.end(); ++cit){
        
        SignalType st = cit->first;
        if(st == SignalType::EMPTY || st == SignalType::GROUND || st == SignalType::SIGNAL || st == SignalType::OBSTACLE) continue;
        currentRequiringSignals.insert(st);
        
        for(const std::string &instance : cit->second){
            BallOut *ballout = uBump.instanceToBallOutMap.at(instance);
            
            double balloutCurrent = ballout->getMaxCurrent();
            signalCurrentRequirements[st] += balloutCurrent;

            for(auto at : ballout->SignalTypeToAllCords){
                if(at.first == SignalType::SIGNAL || at.first == SignalType::GROUND) continue;
                len_t xMin = LEN_T_MAX, xMax = LEN_T_MIN;
                len_t yMin = LEN_T_MAX, yMax = LEN_T_MIN;
                for(const Cord &c : at.second){
                    len_t x = c.x();
                    len_t y = c.y();
                    if(x < xMin) xMin = x;
                    if(x > xMax) xMax = x;
                    if(y < yMin) yMin = y;
                    if(y > yMax) yMax = y;
                }
                assert(xMin != LEN_T_MAX);
                assert(xMax != LEN_T_MIN);
                assert(yMin != LEN_T_MAX);
                assert(yMax != LEN_T_MIN);
                
                //give little margin to the bounding box
                xMin -= INITIAL_MARGIN;
                xMax += INITIAL_MARGIN;
                yMin -= INITIAL_MARGIN;
                yMax += INITIAL_MARGIN;

                layerInitBoxes.at(m_ubumpConnectedMetalLayerIdx)[st].emplace_back(FPoint(xMin, yMin), FPoint(xMax, yMax));
            }
        }
    }

    // Process C4 related information
    for(auto cit = c4.signalTypeToAllClusters.begin(); cit != c4.signalTypeToAllClusters.end(); ++cit){
        SignalType st = cit->first;
        if(currentRequiringSignals.count(st) == 0) continue;

        for(const C4PinCluster *c4cluster : cit->second){
            len_t xMin = LEN_T_MAX, xMax = LEN_T_MIN;
            len_t yMin = LEN_T_MAX, yMax = LEN_T_MIN;
            for(const Cord &c : c4cluster->pins){
                len_t x = c.x();
                len_t y = c.y();
                if(x < xMin) xMin = x;
                if(x > xMax) xMax = x;
                if(y < yMin) yMin = y;
                if(y > yMax) yMax = y;
            }
            assert(xMin != LEN_T_MAX);
            assert(xMax != LEN_T_MIN);
            assert(yMin != LEN_T_MAX);
            assert(yMax != LEN_T_MIN);
            
            //give little margin to the bounding box
            xMin -= INITIAL_MARGIN;
            xMax += INITIAL_MARGIN;
            yMin -= INITIAL_MARGIN;
            yMax += INITIAL_MARGIN;

            layerInitBoxes.at(m_c4ConnectedMetalLayerIdx)[st].emplace_back(FPoint(xMin, yMin), FPoint(xMax, yMax));
        }
    }

    for(int constructLayer = m_ubumpConnectedMetalLayerIdx; constructLayer <= m_c4ConnectedMetalLayerIdx; ++constructLayer){

        // add preplaced metal into planning (doesn't check overlap)
        for(auto cit = metalLayers[constructLayer].preplacedCords.begin(); cit != metalLayers[constructLayer].preplacedCords.end(); ++cit){
            SignalType st = cit->first;
            if(currentRequiringSignals.count(st) == 0) continue;

            for(const Cord &c : cit->second){
                len_t CordX = c.x();
                len_t CordY = c.y();
                FPoint preplacedBoxLL(CordX, CordY);
                FPoint preplacedBoxUR(CordX + 1, CordY + 1);

                layerInitBoxes.at(constructLayer)[st].emplace_back(preplacedBoxLL, preplacedBoxUR);
            }
        }

        // add preplaced via into planning (check insertion before insertion)
        // check the preplace via above this metal layer
        for(int viaLayer = constructLayer-1; viaLayer <= constructLayer; ++viaLayer){
            // skip if the metal layer does not exist
            if((viaLayer < 0) || (viaLayer >= m_viaLayerCount)) continue;

            for(auto cit = viaLayers[viaLayer].preplacedCords.begin(); cit != viaLayers[viaLayer].preplacedCords.end(); ++ cit){
                SignalType st = cit->first;
                if(currentRequiringSignals.count(st) == 0) continue;

                for(const Cord &c : cit->second){
                    len_t CordX = c.x();
                    len_t CordY = c.y();
                    FPoint preplacedCentre(CordX, CordY);

                    bool pointCovered = false;
                    for(const FBox &existedFBox : layerInitBoxes.at(viaLayer)[st]){
                        if(fbox::isContained(preplacedCentre, existedFBox)){
                            pointCovered = true;
                            break;
                        }
                    }

                    if(!pointCovered){
                        FPoint preplacedBoxLL(CordX - INITIAL_MARGIN, CordY - INITIAL_MARGIN);
                        FPoint preplacedBoxUR(CordX + INITIAL_MARGIN, CordY + INITIAL_MARGIN);
                        layerInitBoxes.at(viaLayer)[st].emplace_back(preplacedBoxLL, preplacedBoxUR);
                    }
                }
            }
        }






        // merge collected layerInitBoxes, union them and create corresponding Softbody object
        for(auto it = layerInitBoxes.at(constructLayer).begin(); it != layerInitBoxes.at(constructLayer).end(); ++it){
            SignalType st = it->first;
            std::vector<FPolygon> polygons;
            for (const FBox& box : it->second) {
                FPolygon poly;
                boost::geometry::convert(box, poly);
                boost::geometry::correct(poly);
                polygons.push_back(std::move(poly));
            }
            
            FMultiPolygon unionResult;
            for (const FPolygon& poly : polygons) {
                FMultiPolygon temp;
                boost::geometry::union_(unionResult, poly, temp);
                unionResult = std::move(temp);
            }

            farea_t signalArea = fmp::getArea(unionResult);
            double perUnitCurrent = signalCurrentRequirements.at(st) / signalArea;

            for (const FPolygon& poly : unionResult) {
                double polyArea = fp::getArea(poly);
                double initCurrent = perUnitCurrent * polyArea;
                

                SoftBody *sb = new SoftBody(softBodyIdx++, st, initCurrent, polyArea);
                
                // fill in points bt. A(ax, ay), B(bx, by) s.t. |A-B| < delta, use only interpolation
                auto interpolationFillPoints = [&](const flen_t &ax, const flen_t &ay, const flen_t &bx, const flen_t &by){

                    flen_t dx = bx - ax;
                    flen_t dy = by - ay;
                    flen_t distance = std::hypot(dx, dy);
                    if(distance <= m_PointsMinDelta) return;

                    int newPoints = int(distance / m_PointsMinDelta) + 1;
                    int newSegments = newPoints + 1; // 1 points -> 2 segments

                    dx /= newSegments;
                    dy /= newSegments;

                    flen_t movex = ax;
                    flen_t movey = ay;

                    for(int i = 0; i < newPoints; ++i){
                        movex += dx;
                        movey += dy;
                        sb->contour.emplace_back(movex, movey);

                    }
                };

                for(int i = 0; i < fp::getOuterEdgesCount(poly); ++i){
                    sb->contour.emplace_back(poly.outer().at(i));
                    FPoint mainPoint = poly.outer().at(i);
                    FPoint nextPoint = poly.outer().at(i+1);
                    interpolationFillPoints(mainPoint.x(), mainPoint.y(), nextPoint.x(), nextPoint.y());
                }
                sb->shape = fp::createFPolygon(sb->contour);
                m_OwnerSoftBodies[constructLayer].push_back(sb);
                
                // push Bounding box into the Rectangle bin system
                FBox boundingBox = fp::getBBox(poly);
                const FPoint &bbMin = boundingBox.min_corner();
                const FPoint &bbMax = boundingBox.max_corner();

                softBodyRectangleBin[constructLayer].insert(bbMin.x(), bbMin.y(), bbMax.x(), bbMax.y(), sb);
                for(const FPoint &contourPoints : sb->contour){
                    softBodyPointBin[constructLayer].insert(contourPoints.x(), contourPoints.y(), sb);
                }
            }
        }
    
    }


    /* Create Vias, link to softbodies*/
    this->m_OwnerViasBodies.resize(m_viaLayerCount);
    this->viaPointBins.resize(m_viaLayerCount, PointBinSystem<flen_t, ViaBody>(POINT_BIN_SIZE, 0, 0, m_pinWidth, m_pinHeight));

    for(int layer = 0; layer < m_viaLayerCount; ++layer){
        
        std::vector<std::vector<bool>> isPreplacedVia(m_pinHeight, std::vector<bool>(m_pinWidth, false));

        // process viaCondidatew that is preplaced
        const ObjectArray &oa = viaLayers.at(layer);
        for(const auto &[st, cords] : oa.preplacedCords){
            if(st == SignalType::OBSTACLE || st == SignalType::SIGNAL){
                for(const Cord &c : cords){
                    isPreplacedVia[c.y()][c.x()] = true;
                }
            }else{
                // preplaced cords for specific signals
                for(const Cord &c : cords){
                    flen_t cX = static_cast<flen_t>(c.x());
                    flen_t cY = static_cast<flen_t>(c.y());
                    
                    // check whether if it touches any existing softbodies, this via would count as hard via
                    // check top layer
                    std::vector<SoftBody *> upLayerSoftBodyCandidate = softBodyRectangleBin[layer].queryPoint(cX, cY);
                    SoftBody *upIntersectSoftBody = nullptr;
                    for(SoftBody *upSBIndex : upLayerSoftBodyCandidate){
                        // lookup cache if softbody exists
                        if(fp::isContained(FPoint(cX, cY), upSBIndex->shape)){
                            upIntersectSoftBody = upSBIndex;
                            break;
                        }
                    }

                    // check bottom layer
                    std::vector<SoftBody *> downLayerSoftBodyCandidate = softBodyRectangleBin.at(layer+1).queryPoint(cX, cY);
                    SoftBody *downIntersectSoftBody = nullptr;
                    for(SoftBody *downSBIndex : downLayerSoftBodyCandidate){
                        // lookup cache if softbody exists
                        if(fp::isContained(FPoint(cX, cY), downSBIndex->shape)){
                            upIntersectSoftBody = downSBIndex;
                            break;
                        }
                    }
                    
                    // create the ViaBody, link the info of vias and softBodies
                    ViaBody *vb = new ViaBody(layer, cX, cY, st);
                    
                    bool upIntersectsSoftBodyisEmpty = (upIntersectSoftBody == nullptr);
                    bool downIntersectsSoftBodyisEmpty = (downIntersectSoftBody == nullptr);

                    if(upIntersectsSoftBodyisEmpty && downIntersectsSoftBodyisEmpty){
                        vb->status = ViaBodyStatus::EMPTY;
                        viaPointBins[layer].insert(cX, cY, vb);

                    }else if((!upIntersectsSoftBodyisEmpty) && downIntersectsSoftBodyisEmpty){
                        vb->upIsFixed = true;
                        vb->upSoftBody = upIntersectSoftBody;
                        vb->status = ViaBodyStatus::TOP_OCCUPIED;
                        viaPointBins[layer].insert(cX, cY, vb);

                        upIntersectSoftBody->hardVias.push_back(vb);

                    }else if(upIntersectsSoftBodyisEmpty && (!downIntersectsSoftBodyisEmpty)){
                        vb->downIsFixed = true;
                        vb->downSoftBody = downIntersectSoftBody;
                        vb->status = ViaBodyStatus::DOWN_OCCUPIED;
                        viaPointBins[layer].insert(cX, cY, vb);

                        upIntersectSoftBody->hardVias.push_back(vb);

                    }else{
                        vb->upIsFixed = true;
                        vb->upSoftBody = upIntersectSoftBody;
                        vb->downIsFixed = true;
                        vb->downSoftBody = downIntersectSoftBody;
                        vb->status = ViaBodyStatus::BROKEN;
                        // do not insert to viaPointBins, useless

                        upIntersectSoftBody->hardVias.push_back(vb);
                        downIntersectSoftBody->hardVias.push_back(vb);
                    }
                
                    isPreplacedVia[c.y()][c.x()] = true;
                    m_OwnerViasBodies[layer].push_back(vb);
                }
            }
        }
        
        // process those viaCandidates which is not preplaced
        for(int j = 0; j < m_pinHeight; ++j){
            for(int i = 0; i < m_pinWidth; ++i){
                if(isPreplacedVia[j][i]) continue;
                    
                flen_t cX = static_cast<flen_t>(i);
                flen_t cY = static_cast<flen_t>(j);
                
                // check whether if it touches any existing softbodies, this via would count as hard via
                // check top layer
                std::vector<SoftBody *> upLayerSoftBodyCandidate = softBodyRectangleBin[layer].queryPoint(cX, cY);
                SoftBody *upIntersectSoftBody = nullptr;
                for(SoftBody *upSBIndex : upLayerSoftBodyCandidate){
                    // lookup cache if softbody exists
                    if(fp::isContained(FPoint(cX, cY), upSBIndex->shape)){
                        upIntersectSoftBody = upSBIndex;
                        break;
                    }
                }

                // check bottom layer
                std::vector<SoftBody *> downLayerSoftBodyCandidate = softBodyRectangleBin.at(layer+1).queryPoint(cX, cY);
                SoftBody *downIntersectSoftBody = nullptr;
                for(SoftBody *downSBIndex : downLayerSoftBodyCandidate){
                    // lookup cache if softbody exists
                    if(fp::isContained(FPoint(cX, cY), downSBIndex->shape)){
                        upIntersectSoftBody = downSBIndex;
                        break;
                    }
                }

                // create the ViaBody, link the info of vias and softBodies
                ViaBody *vb = new ViaBody(layer, cX, cY);
                
                bool upIntersectsSoftBodyisEmpty = (upIntersectSoftBody == nullptr);
                bool downIntersectsSoftBodyisEmpty = (downIntersectSoftBody == nullptr);

                if(upIntersectsSoftBodyisEmpty && downIntersectsSoftBodyisEmpty){
                    vb->status = ViaBodyStatus::EMPTY;
                    viaPointBins[layer].insert(cX, cY, vb);

                }else if((!upIntersectsSoftBodyisEmpty) && downIntersectsSoftBodyisEmpty){
                    vb->upIsFixed = true;
                    vb->upSoftBody = upIntersectSoftBody;
                    vb->status = ViaBodyStatus::TOP_OCCUPIED;
                    viaPointBins[layer].insert(cX, cY, vb);

                    upIntersectSoftBody->hardVias.push_back(vb);

                }else if(upIntersectsSoftBodyisEmpty && (!downIntersectsSoftBodyisEmpty)){
                    vb->downIsFixed = true;
                    vb->downSoftBody = downIntersectSoftBody;
                    vb->status = ViaBodyStatus::DOWN_OCCUPIED;
                    viaPointBins[layer].insert(cX, cY, vb);

                    upIntersectSoftBody->hardVias.push_back(vb);

                }else{
                    vb->upIsFixed = true;
                    vb->upSoftBody = upIntersectSoftBody;
                    vb->downIsFixed = true;
                    vb->downSoftBody = downIntersectSoftBody;
                    vb->status = ViaBodyStatus::BROKEN;
                    // do not insert to viaPointBins, useless

                    upIntersectSoftBody->hardVias.push_back(vb);
                    downIntersectSoftBody->hardVias.push_back(vb);
                }

                m_OwnerViasBodies[layer].push_back(vb);

            }
        }
    }

    // calculate pressure and force for each softbody & viaBody

}

PressureSimulator::~PressureSimulator(){
    
    for (auto &row : m_OwnerSoftBodies) {
        for (SoftBody* ptr : row) {
            delete ptr;
        }
    }

    for (auto &row : m_OwnerViasBodies) {
        for (ViaBody* ptr : row) {
            delete ptr;
        }
    }
}

const std::vector<SoftBody *> &PressureSimulator::getSoftBodyOwner(int layer) const {
    assert(layer >= 0 && layer < m_metalLayerCount);
    return m_OwnerSoftBodies[layer];
}

const std::vector<ViaBody *> &PressureSimulator::getViaBodyOwner(int layer) const {
    assert(layer >= 0 && layer < m_viaLayerCount);
    return m_OwnerViasBodies[layer];
}

void PressureSimulator::inflate(){

    const int maxInflateIteration = 100;
    const flen_t binSize = 2.0;
    
    for(int inflateIteration = 0; inflateIteration < maxInflateIteration; ++inflateIteration){
        std::cout << "[Inflation BroadCast] Iteration " << inflateIteration << " starts" << std::endl;
        
        // up/down parallel
        for(int layer = 0; layer < getMetalLayerCount(); ++layer){
        std::cout << "[Inflation BroadCast] Layer " << layer << " starts" << std::endl;

            
            PointBinSystem<flen_t, SoftBody> layerPointsBin(binSize, 0, 0, m_gridWidth-1, m_gridHeight-1);
            for(int softBodyIdx = 0; softBodyIdx < m_OwnerSoftBodies.size(); ++softBodyIdx){
                SoftBody *sbTarget = m_OwnerSoftBodies.at(layer).at(softBodyIdx);
                
                for(int pointIdx = 0; pointIdx < sbTarget->contour.size(); ++pointIdx){
                    FPoint &fp = sbTarget->contour.at(pointIdx);
                    layerPointsBin.insert(fp.x(), fp.y(), sbTarget);
                }
            }

            // parallel

            for(int softBodyIdx = 0; softBodyIdx < m_OwnerSoftBodies.size(); ++softBodyIdx){
                SoftBody *sbTarget = m_OwnerSoftBodies.at(layer).at(softBodyIdx);
                const int sbCounterSize = sbTarget->contour.size();
                double sbPressure = sbTarget->pressure;

                // move the contour points
                // parallel
                int sbCounterSizeMinusOne = sbCounterSize - 1;
                for(int pointIdx = 0; pointIdx < sbCounterSize; ++pointIdx){
                    FPoint &fpLast = (pointIdx == 0)? sbTarget->contour.at(sbCounterSizeMinusOne) : sbTarget->contour.at(pointIdx-1);
                    FPoint &fp = sbTarget->contour.at(pointIdx);
                    FPoint &fpNext = (pointIdx == sbCounterSizeMinusOne)? sbTarget->contour.at(0) : sbTarget->contour.at(pointIdx+1);
                

                    // calculate normal force (-> P * ni)
                    flen_t normalX = fpLast.y() - fpNext.y();
                    flen_t normalY = fpNext.x() - fpLast.x();
                    flen_t normalMagnitude = std::sqrt(normalX * normalX + normalY * normalY);

                    normalX = (normalX / normalMagnitude);
                    normalY = (normalY / normalMagnitude);
                    
                    // calculate Curvature Restoration force

                }

            }
        }
    }
}