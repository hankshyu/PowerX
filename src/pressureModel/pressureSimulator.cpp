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

#include "visualiser.hpp"

PressureSimulator::PressureSimulator(const std::string &fileName): PowerDistributionNetwork(fileName) {
    this->m_canvasWidth = m_pinWidth;
    this->m_canvasHeight = m_pinHeight;
}

void PressureSimulator::runAlgorithm(){
    initialise();
    collectStatistics();
    normalise();
    
    visualiseSoftBodiesWithPin(*this, getSoftBodyOwner(0), getViaBodyOwner(0), "vid/m0_0.txt");
    visualiseSoftBodiesWithPins(*this, getSoftBodyOwner(1), getViaBodyOwner(0), getViaBodyOwner(1), "vid/m1_0.txt");
    visualiseSoftBodiesWithPin(*this, getSoftBodyOwner(2), getViaBodyOwner(1), "vid/m2_0.txt");
    
    for(int algorithmIteration = 1; algorithmIteration <= 20; ++algorithmIteration){
        relaxPolygons();
        fixPolygons();

        visualiseSoftBodiesWithPin(*this, getSoftBodyOwner(0), getViaBodyOwner(0), "vid/m0_" + std::to_string(algorithmIteration) + ".txt");
        visualiseSoftBodiesWithPins(*this, getSoftBodyOwner(1), getViaBodyOwner(0), getViaBodyOwner(1), "vid/m1_"+std::to_string(algorithmIteration)+".txt");
        visualiseSoftBodiesWithPin(*this, getSoftBodyOwner(2), getViaBodyOwner(1), "vid/m2_"+std::to_string(algorithmIteration)+".txt");

    }
}
    
void PressureSimulator::initialise(){

    int softBodyIdx = 0;
    int viaBodyIdx = 0;

    // the canvas size is the same width the pin canvas

    this->m_OwnerSoftBodies.resize(m_metalLayerCount);

    this->softBodyRectangleBin.resize(m_metalLayerCount, RectangleBinSystem<flen_t, SoftBody>(m_params.rectangleBinSize, 0, 0, m_gridWidth, m_gridHeight));
    this->softBodyPointBin.resize(m_metalLayerCount, PointBinSystem<flen_t, SoftBody>(m_params.pointBinSize, 0, 0, m_gridWidth, m_gridHeight));

    /* Insert SoftBodies */

    std::unordered_set<SignalType> currentRequiringSignals;
    
    std::vector<std::unordered_map<SignalType, std::vector<FBox>>> layerInitBoxes(m_metalLayerCount);

    // Porcess Ubump related informations
    for(auto cit = uBump.signalTypeToInstances.begin(); cit != uBump.signalTypeToInstances.end(); ++cit){
        
        SignalType st = cit->first;
        if(st == SignalType::EMPTY || st == SignalType::GROUND || st == SignalType::SIGNAL || st == SignalType::OBSTACLE) continue;
        currentRequiringSignals.insert(st);
        
        for(const std::string &instance : cit->second){
            BallOut *ballout = uBump.instanceToBallOutMap[instance];
            double balloutCurrent = ballout->getMaxCurrent();
            signalCurrentRequirements[st] += balloutCurrent;



            Rectangle balloutRectangle = uBump.instanceToRectangleMap[instance];
            flen_t fxl = static_cast<flen_t>(rec::getXL(balloutRectangle));
            flen_t fxh = static_cast<flen_t>(rec::getXH(balloutRectangle));
            flen_t fyl = static_cast<flen_t>(rec::getYL(balloutRectangle));
            flen_t fyh = static_cast<flen_t>(rec::getYH(balloutRectangle));

            layerInitBoxes[m_ubumpConnectedMetalLayerIdx][st].emplace_back(FPoint(fxl, fyl), FPoint(fxh, fyh));

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
            xMin -= m_params.initialMargin;
            xMax += m_params.initialMargin;
            yMin -= m_params.initialMargin;
            yMax += m_params.initialMargin;

            layerInitBoxes[m_c4ConnectedMetalLayerIdx][st].emplace_back(FPoint(xMin, yMin), FPoint(xMax, yMax));
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

                layerInitBoxes[constructLayer][st].emplace_back(preplacedBoxLL, preplacedBoxUR);
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
                    for(const FBox &existedFBox : layerInitBoxes[viaLayer][st]){
                        if(fbox::isContained(preplacedCentre, existedFBox)){
                            pointCovered = true;
                            break;
                        }
                    }

                    if(!pointCovered){
                        FPoint preplacedBoxLL(CordX - m_params.initialMargin, CordY - m_params.initialMargin);
                        FPoint preplacedBoxUR(CordX + m_params.initialMargin, CordY + m_params.initialMargin);
                        layerInitBoxes[viaLayer][st].emplace_back(preplacedBoxLL, preplacedBoxUR);
                    }
                }
            }
        }

        // merge collected layerInitBoxes, union them and create corresponding Softbody object
        for(auto it = layerInitBoxes[constructLayer].begin(); it != layerInitBoxes[constructLayer].end(); ++it){
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
            double perUnitCurrent = signalCurrentRequirements[st] / signalArea;

            for (const FPolygon& poly : unionResult) {
                double polyArea = fp::getArea(poly);
                double initCurrent = perUnitCurrent * polyArea;
                

                SoftBody *sb = new SoftBody(softBodyIdx++, st, initCurrent, polyArea);
                
                // avoid pushing repeated points
                // fill in points bt. A(ax, ay), B(bx, by) s.t. |A-B| < delta, use only interpolation
                for(int i = 0; i < fp::getOuterEdgesCount(poly); ++i){
                    FPoint mainPoint = poly.outer()[i];
                    FPoint nextPoint = poly.outer()[i+1];
                    flen_t ax = mainPoint.x();
                    flen_t ay = mainPoint.y();
                    flen_t bx = nextPoint.x();
                    flen_t by = nextPoint.y();

                    flen_t dx = bx - ax;
                    flen_t dy = by - ay;
                    flen_t distance = std::hypot(dx, dy);

                    if(distance <= m_params.pointsMaxDelta){
                        sb->contour.emplace_back(ax, ay);
                        sb->cacheContourDX.push_back(dx);
                        sb->cacheContourDY.push_back(dy);
                        sb->cacheContourDistance.push_back(distance);
                        continue;
                    }

                    int newSegments = ceil(distance / m_params.pointsMaxDelta);
                    int newPoints = newSegments - 1;

                    dx /= newSegments;
                    dy /= newSegments;
                    distance /= newSegments;

                    flen_t movex = ax;
                    flen_t movey = ay;

                    for(int i = 0; i < newPoints; ++i){
                        movex += dx;
                        movey += dy;
                        sb->contour.emplace_back(movex, movey);
                        sb->cacheContourDX.push_back(dx);
                        sb->cacheContourDY.push_back(dy);
                        sb->cacheContourDistance.push_back(distance);
                    }
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
    this->viaPointBins.resize(m_viaLayerCount, PointBinSystem<flen_t, ViaBody>(m_params.pointBinSize, 0, 0, m_pinWidth, m_pinHeight));

    for(int layer = 0; layer < m_viaLayerCount; ++layer){
        
        std::vector<std::vector<bool>> isPreplacedVia(m_pinHeight, std::vector<bool>(m_pinWidth, false));

        // process viaCondidatew that is preplaced
        const ObjectArray &oa = viaLayers[layer];
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
                    std::vector<SoftBody *> downLayerSoftBodyCandidate = softBodyRectangleBin[layer+1].queryPoint(cX, cY);
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
                    vb->setIsPeplaced();
                    vb->activSigType = st;
                    
                    bool upIntersectsSoftBodyisEmpty = (upIntersectSoftBody == nullptr);
                    bool downIntersectsSoftBodyisEmpty = (downIntersectSoftBody == nullptr);

                    if(upIntersectsSoftBodyisEmpty && downIntersectsSoftBodyisEmpty){
                        vb->status = ViaBodyStatus::EMPTY;
                        vb->activSigType = SignalType::EMPTY;
                        viaPointBins[layer].insert(cX, cY, vb);

                    }else if((!upIntersectsSoftBodyisEmpty) && downIntersectsSoftBodyisEmpty){
                        vb->setUpIsFixed();
                        vb->upSoftBody = upIntersectSoftBody;
                        vb->status = ViaBodyStatus::TOP_OCCUPIED;
                        vb->activSigType = upIntersectSoftBody->getSigType();
                        viaPointBins[layer].insert(cX, cY, vb);

                        upIntersectSoftBody->hardVias.push_back(vb);

                    }else if(upIntersectsSoftBodyisEmpty && (!downIntersectsSoftBodyisEmpty)){
                        vb->setDownIsFixed();
                        vb->downSoftBody = downIntersectSoftBody;
                        vb->status = ViaBodyStatus::DOWN_OCCUPIED;
                        vb->activSigType = downIntersectSoftBody->getSigType();
                        viaPointBins[layer].insert(cX, cY, vb);

                        upIntersectSoftBody->hardVias.push_back(vb);

                    }else{
                        // do not insert to viaPointBins, both up and down are preplaced
                        vb->setUpIsFixed();
                        vb->setDownIsFixed();
                        vb->upSoftBody = upIntersectSoftBody;
                        vb->downSoftBody = downIntersectSoftBody;
                        
                        vb->status = ViaBodyStatus::STABLE;

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
                std::vector<SoftBody *> downLayerSoftBodyCandidate = softBodyRectangleBin[layer+1].queryPoint(cX, cY);
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
                    vb->setUpIsFixed();
                    vb->upSoftBody = upIntersectSoftBody;
                    vb->status = ViaBodyStatus::TOP_OCCUPIED;
                    vb->activSigType = upIntersectSoftBody->getSigType();
                    viaPointBins[layer].insert(cX, cY, vb);

                    upIntersectSoftBody->hardVias.push_back(vb);

                }else if(upIntersectsSoftBodyisEmpty && (!downIntersectsSoftBodyisEmpty)){
                    vb->setDownIsFixed();
                    vb->downSoftBody = downIntersectSoftBody;
                    vb->status = ViaBodyStatus::DOWN_OCCUPIED;
                    vb->activSigType = downIntersectSoftBody->getSigType();
                    viaPointBins[layer].insert(cX, cY, vb);

                    upIntersectSoftBody->hardVias.push_back(vb);

                }else{
                    vb->setUpIsFixed();
                    vb->setDownIsFixed();
                    vb->upSoftBody = upIntersectSoftBody;
                    vb->downSoftBody = downIntersectSoftBody;
                    viaPointBins[layer].insert(cX, cY, vb);


                    SignalType upSigType = vb->upSoftBody->getSigType();
                    SignalType downSigType = vb->downSoftBody->getSigType();

                    if(upSigType == downSigType){
                        upIntersectSoftBody->hardVias.push_back(vb);
                        downIntersectSoftBody->hardVias.push_back(vb);
                        vb->status = ViaBodyStatus::STABLE;
                    }else{ // not coherent, do not set as fixed 
                        vb->status = ViaBodyStatus::UNSTABLE;

                    }
                }

                m_OwnerViasBodies[layer].push_back(vb);

            }
        }
    }
    

}

void PressureSimulator::collectStatistics(){

    // collect statistical values
    avgViaDistance.resize(getViaLayerCount());

    for(int layer = 0; layer < getViaLayerCount(); ++layer){
        flen_t distance = 0;
        
        for(ViaBody *vb : m_OwnerViasBodies[layer]){
            flen_t vbX = vb->x();
            flen_t vbY = vb->y();

            PointDistanceEntry<flen_t, ViaBody> neighbor = viaPointBins[layer].queryNearestNeighbor(vbX, vbY);
            
            distance += neighbor.distance;
        }

        int layerViasCount = m_OwnerViasBodies[layer].size();
        avgViaDistance[layer] = distance / layerViasCount;

    }

    // calculate total current requirements

    this->totalCurrentRequirement = 0;
    for(const auto&[st, sum] : signalCurrentRequirements){
        totalCurrentRequirement += sum;
    }


}

void PressureSimulator::normalise(){

    for(int layer = 0; layer < getMetalLayerCount(); ++layer){
        for(SoftBody *sb : m_OwnerSoftBodies[layer]){
            sb->setExpectedCurrent(sb->getRawExpectedCurrent() / this->totalCurrentRequirement);
        }
    }

    // initialize softbody attributes
    for(int layer = 0; layer < getMetalLayerCount(); ++layer){
        for(SoftBody *sb : m_OwnerSoftBodies[layer]){
            flen_t avgViaDistance = 0;
            if(layer == 0) avgViaDistance = this->avgViaDistance[0];
            else if(layer == m_c4ConnectedMetalLayerIdx) avgViaDistance = this->avgViaDistance[m_c4ConnectedMetalLayerIdx-1];
            else avgViaDistance = (this->avgViaDistance[layer-1] + this->avgViaDistance[layer])/2;
            sb->initialiseParameters(this->m_canvasWidth, this->m_canvasHeight, avgViaDistance);
        }
    }
}

void PressureSimulator::relaxPolygons(){

    // up/down metal layer parallel
    for(int layer = 0; layer < getMetalLayerCount(); ++layer){
        std::cout << "Layer = " << layer << "are softbody count = " << m_OwnerSoftBodies[layer].size() << std::endl;
        
        // softbody parallel
        for(int softBodyIdx = 0; softBodyIdx < m_OwnerSoftBodies[layer].size(); ++softBodyIdx){
            std::cout << "Doing softbodyIdx " << softBodyIdx << std::endl;
            SoftBody *sbTarget = m_OwnerSoftBodies[layer][softBodyIdx];
            SignalType sbTargetSigType = sbTarget->getSigType();
            const int sbCounterSize = sbTarget->contour.size();
            
            
            double sbPressure = sbTarget->pressure;
            double sbSurfaceTension = sbTarget->surfaceTension;
            
            double sbAttractionStrength = sbTarget->attractionStrength;
            flen_t sbAttractionRadius = sbTarget->attractionRadius;
            double sbRepulsionStrength = sbTarget->repulsionStrength;
            flen_t sbRepulsionRadius = sbTarget->repulsionRadius;

            double sbAttractEmptyViaStrength = sbTarget->attractEmptyViaStrength;
            flen_t sbAttractEmptyViaRadius= sbTarget->attractEmptyViaRadius;
            double sbAttractSameViaStrength= sbTarget->attractSameViaStrength;
            flen_t sbAttractsameViaRadius= sbTarget->attractsameViaRadius;


            // contour points parallel
            int sbCounterSizeMinusOne = sbCounterSize - 1;
            // std::cout << "Probing SoftBody signal type = " << sbTargetSigType << std::endl;
            for(int pointIdx = 0; pointIdx < sbCounterSize; ++pointIdx){
                
                
                int fpLastIdx = (pointIdx == 0)? sbCounterSizeMinusOne : pointIdx-1;
                int fpNextIdx = (pointIdx == sbCounterSizeMinusOne)? 0 : pointIdx+1;
                FPoint &fpLast = sbTarget->contour[fpLastIdx];
                FPoint &fp = sbTarget->contour[pointIdx];
                flen_t fpX = fp.x();
                flen_t fpY = fp.y();
                FPoint &fpNext = sbTarget->contour[fpNextIdx];
                // std::cout << "Point = " << fpLastIdx << fpLast << ", " << pointIdx << fp << ", " << fpNextIdx << fpNext << std::endl;
                // fpLast -> A, fp -> B, fpNext -> C 
                flen_t BAdx = (fpX - fpLast.x());
                flen_t BAdy = (fpY - fpLast.y());
                flen_t CBdx = (fpNext.x() - fpX);
                flen_t CBdy = (fpNext.y() - fpY);

                flen_t distanceAB = std::hypot(BAdx, BAdy);
                flen_t distanceBC = std::hypot(CBdx, CBdy);

                // flen_t BAdx = sbTarget->cacheContourDX[fpLastIdx];
                // flen_t BAdy = sbTarget->cacheContourDY[fpLastIdx];
                // flen_t CBdx = sbTarget->cacheContourDX[pointIdx];
                // flen_t CBdy = sbTarget->cacheContourDY[pointIdx];
                
                // flen_t distanceAB = sbTarget->cacheContourDistance[fpLastIdx];
                // flen_t distanceBC = sbTarget->cacheContourDistance[pointIdx];
                /* Calculate pressure force */

                // flen_t pressureForce = (0.5 * (distanceAB + distanceBC)) * sbTarget->pressure;
                flen_t pressureForce = (m_params.pointsMaxDelta) * sbTarget->pressure;
                
                /* Calculate Curvature Restoration force */

                // Cross: (Ax - Bx)(Cy - By) - (Ay - By)(Cx - Bx)
                // If cross < 0, you’re in a convex (bulging) region -> positive curvature
	            // If cross > 0, you’re in a concave (dented) region -> negative curvature
                flen_t cross = BAdx * CBdy - BAdy * CBdx;
                
                farea_t area = 0.5 * std::abs(cross);

                flen_t CAdx = fpNext.x() - fpLast.x();
                flen_t CAdy = fpNext.y() - fpLast.y();
                flen_t distanceCA = hypot(CAdx, CAdy);
                
                flen_t normaldx = -CAdy/distanceCA;
                flen_t normaldy = CAdx/distanceCA;

                flen_t minusMengerKappa = (4.0 * area) / (distanceAB * distanceBC * distanceCA);
                minusMengerKappa *= m_params.pointsMaxDelta; // normalise to 0-1
                if(cross < 0) minusMengerKappa = -minusMengerKappa;
                flen_t curvatureRestorationForce = 0.5 * pressureForce * sbSurfaceTension * minusMengerKappa; // normalise to ~40%

                flen_t resultantForceX =  pressureForce + curvatureRestorationForce;
                flen_t resultantForceY = resultantForceX * normaldy;
                resultantForceX = resultantForceX * normaldx;

                bool isShow = curvatureRestorationForce != 0;
                sbTarget->cacheContour.emplace_back(fpX + 0.05*resultantForceX, fpY + 0.05*resultantForceY);

                // std::cout << "[Pressure]" << FPoint(fpX, fpY) << "P = " << pressureForce << ", K = " << curvatureRestorationForce << ", @" << normaldx << ", " << normaldy << std::endl;


                

                // if(true){
                //     std::cout << "Display Pont forces: " << FPoint(fpX, fpY) << std::endl;
                //     std::cout << "Normal Related Force" << std::endl;
                //     std::cout << "PressureForce = " << pressureForce << std::endl;
                //     std::cout << "Curvature Restoration = " << curvatureRestorationForce << std::endl;
                //     std::cout << "Total dx = " << normaldx << ", dy = " << normaldy << std::endl;
                // } 
                
                // /* Calculate Force by nearby Softbodies */
                // using pointAnsSB = PointDistanceEntry<flen_t, SoftBody>;
                // if(isShow){
                //     std::cout << std::endl << std::endl;
                //     std::cout << "SoftBody Related Forces" << std::endl;
                //     std::cout << "Attraction Forces by same type" << std::endl;
                // }
                // // Attraction Force by same types of Softbodies
                // // F = -Ka x (1 - d/Ra) x r, for d < Ra
                // flen_t totalSBBorderAttractionForceX = 0;
                // flen_t totalSBBorderAttractionForceY = 0;

                // std::vector<pointAnsSB>attractionCandidates = softBodyPointBin[layer].queryDistance(fpX, fpY, sbAttractionRadius);
                // for(const pointAnsSB& pa : attractionCandidates){
                //     if(pa.object == sbTarget) continue;
                //     if(pa.object->getSigType() != sbTargetSigType) continue;

                //     flen_t paDistance = pa.distance;
                //     flen_t paX = pa.x;
                //     flen_t paY = pa.y;

                //     flen_t attractionDx = 1 - (paDistance / sbAttractionRadius);
                //     flen_t attractionDy = (attractionDx * (paY - fpY)) / paDistance;
                //     attractionDx = (attractionDx * (paX - fpX)) / paDistance;
                //     if(isShow) std::cout << "[Attract SB] " << "(" << paX << ", " << paY << ")" << " with attraction force = " << attractionDx << ", " << attractionDy << std::endl;
                //     totalSBBorderAttractionForceX += attractionDx;
                //     totalSBBorderAttractionForceY += attractionDy;
                // }

                // totalSBBorderAttractionForceX *= sbAttractionStrength;
                // totalSBBorderAttractionForceY *= sbAttractionStrength;
                // if(isShow) std::cout << "Attract SB Sum = " << totalSBBorderAttractionForceX << ", " << totalSBBorderAttractionForceY << std::endl;

                // // Repulsion Force by different types of Softbodies
                // // F = +Kr x (1- d/Rr)^2 x r, for d < Rr

                // if(isShow){
                //     std::cout << std::endl;
                //     std::cout << "Repulsion Forces by diff type" << std::endl;
                // }
                // flen_t totalSBBorderRepulsionForceX = 0;
                // flen_t totalSBBorderRepulsionForceY = 0;


                // std::vector<pointAnsSB>repulsionCandidates = softBodyPointBin[layer].queryDistance(fpX, fpY, sbRepulsionRadius);
                // for(const pointAnsSB& pa : repulsionCandidates){
                //     if(pa.object == sbTarget) continue;
                //     if(pa.object->getSigType() == sbTargetSigType) continue;
                    
                //     flen_t paDistance = pa.distance;
                //     flen_t paX = pa.x;
                //     flen_t paY = pa.y;

                //     flen_t repulsionDx = 1 - (paDistance/sbRepulsionRadius);
                //     repulsionDx = repulsionDx * repulsionDx;
                //     flen_t repulsionDy = (repulsionDx * (fpY - paY)) / paDistance;
                //     repulsionDx = (repulsionDx * (fpX - paX)) / paDistance;

                //     if(isShow) std::cout << "[Repulsion SB] " << "(" << paX << ", " << paY << ")" << " with attraction force = " << repulsionDx << ", " << repulsionDy << std::endl;
                //     totalSBBorderRepulsionForceX += repulsionDx;
                //     totalSBBorderRepulsionForceY += repulsionDy;
                // }

                // totalSBBorderRepulsionForceX *= sbRepulsionStrength;
                // totalSBBorderRepulsionForceY *= sbRepulsionStrength;

                // if(isShow) std::cout << "Repulsion SB Sum = " << totalSBBorderRepulsionForceX << ", " << totalSBBorderRepulsionForceY << std::endl;

                // /* Calculate Forces by Nearby Vias */
                // // F = K x (1 - d/R)^2 x r, for d < R

                // if(isShow){
                //     std::cout << std::endl << std::endl;
                //     std::cout << "Via Related Force" << std::endl;
                // }
                // flen_t totalEmptyViaForceX = 0;
                // flen_t totalEmptyViaForceY = 0;

                // flen_t totalSameViaForceX = 0;
                // flen_t totalSameViaForceY = 0;

                // flen_t viaBinQueryDistance = (sbAttractEmptyViaRadius > sbAttractsameViaRadius)? sbAttractEmptyViaRadius : sbAttractsameViaRadius;
                // using pointAnsVB = PointDistanceEntry<flen_t, ViaBody>;

                // if(isShow) std::cout << "Layer up:" << std::endl;
                // // explore the via layer above the metal layer
                // if(layer != m_ubumpConnectedMetalLayerIdx){
                //     int viaLayer = layer - 1;
                //     std::vector<pointAnsVB> viaCandidates = viaPointBins[viaLayer].queryDistance(fpX, fpY, viaBinQueryDistance);
                //     for(const pointAnsVB& pa : viaCandidates){
                //         ViaBody *vb = pa.object;
                //         switch (vb->status){
                //             case ViaBodyStatus::EMPTY: {
                //                 // already preplaced as another signal but not attracted
                //                 if(vb->getIsPreplaced() && (vb->activSigType != sbTargetSigType)) continue;
                //                 flen_t paDistance = pa.distance;
                //                 if(paDistance > sbAttractEmptyViaRadius) continue;

                //                 flen_t paX = pa.x;
                //                 flen_t paY = pa.y;

                //                 flen_t emptyViaAttractionDx = 1 - (paDistance / sbAttractionRadius);
                //                 emptyViaAttractionDx = emptyViaAttractionDx * emptyViaAttractionDx;

                //                 flen_t emptyViaAttractionDy = (emptyViaAttractionDx * (paY - fpY)) / paDistance;
                //                 emptyViaAttractionDx = (emptyViaAttractionDx * (paX - fpX)) / paDistance;

                //                 if(isShow) std::cout << "[Empty Via] " << "(" << paX << ", " << paY << ")" << " with force = " << emptyViaAttractionDx << ", " << emptyViaAttractionDy << std::endl;
                //                 totalEmptyViaForceX += emptyViaAttractionDx;
                //                 totalEmptyViaForceY += emptyViaAttractionDy;
                                
                //             }
                //             case ViaBodyStatus::TOP_OCCUPIED: {
                //                 if(vb->activSigType != sbTargetSigType) continue;
                //                 flen_t paDistance = pa.distance;
                //                 if(paDistance > sbAttractsameViaRadius) continue;
                                
                //                 flen_t paX = pa.x;
                //                 flen_t paY = pa.y;

                //                 flen_t sameViaAttractionDx = 1 - (paDistance / sbAttractionRadius);
                //                 sameViaAttractionDx = sameViaAttractionDx * sameViaAttractionDx;

                //                 flen_t sameViaAttractionDy = (sameViaAttractionDx * (paY - fpY)) / paDistance;
                //                 sameViaAttractionDx = (sameViaAttractionDx * (paX - fpX)) / paDistance;

                //                 if(isShow) std::cout << "[Top occ] " << "(" << paX << ", " << paY << ")" << " with force = " << sameViaAttractionDx << ", " << sameViaAttractionDy << std::endl;
                //                 totalSameViaForceX += sameViaAttractionDx;
                //                 totalSameViaForceY += sameViaAttractionDy;
                //             }
                //             default: // uninterested type, next via
                //                 continue;
                //         }


                //     }   
                // }

                // if(isShow) std::cout << "Layer down:" << std::endl;
                // // explore the via layer below the metal layer
                // if(layer != m_c4ConnectedMetalLayerIdx){
                //     int viaLayer = layer;
                //     std::vector<pointAnsVB> viaCandidates = viaPointBins[viaLayer].queryDistance(fpX, fpY, viaBinQueryDistance);
                //     for(const pointAnsVB& pa : viaCandidates){
                //         ViaBody *vb = pa.object;
                //         switch (vb->status){
                //             case ViaBodyStatus::EMPTY: {
                //                 // already preplaced as another signal but not attracted
                //                 if(vb->getIsPreplaced() && (vb->activSigType != sbTargetSigType)) continue;
                //                 flen_t paDistance = pa.distance;
                //                 if(paDistance > sbAttractEmptyViaRadius) continue;

                //                 flen_t paX = pa.x;
                //                 flen_t paY = pa.y;

                //                 flen_t emptyViaAttractionDx = 1 - (paDistance / sbAttractionRadius);
                //                 emptyViaAttractionDx = emptyViaAttractionDx * emptyViaAttractionDx;

                //                 flen_t emptyViaAttractionDy = (emptyViaAttractionDx * (paY - fpY)) / paDistance;
                //                 emptyViaAttractionDx = (emptyViaAttractionDx * (paX - fpX)) / paDistance;

                //                 if(isShow) std::cout << "[Empty Via] " << "(" << paX << ", " << paY << ")" << " with force = " << emptyViaAttractionDx << ", " << emptyViaAttractionDy << std::endl;
                //                 totalEmptyViaForceX += emptyViaAttractionDx;
                //                 totalEmptyViaForceY += emptyViaAttractionDy;
                                
                //             }
                //             case ViaBodyStatus::DOWN_OCCUPIED: {
                //                 if(vb->activSigType != sbTargetSigType) continue;
                //                 flen_t paDistance = pa.distance;
                //                 if(paDistance > sbAttractsameViaRadius) continue;
                                
                //                 flen_t paX = pa.x;
                //                 flen_t paY = pa.y;

                //                 flen_t sameViaAttractionDx = 1 - (paDistance / sbAttractionRadius);
                //                 sameViaAttractionDx = sameViaAttractionDx * sameViaAttractionDx;

                //                 flen_t sameViaAttractionDy = (sameViaAttractionDx * (paY - fpY)) / paDistance;
                //                 sameViaAttractionDx = (sameViaAttractionDx * (paX - fpX)) / paDistance;

                //                 if(isShow) std::cout << "[Top occ] " << "(" << paX << ", " << paY << ")" << " with force = " << sameViaAttractionDx << ", " << sameViaAttractionDy << std::endl;
                //                 totalSameViaForceX += sameViaAttractionDx;
                //                 totalSameViaForceY += sameViaAttractionDy;
                //             }
                //             default: // uninterested type, next via
                //                 continue;
                //         }

                //     }

                // }

                // flen_t totalViaForceX = sbAttractEmptyViaStrength * totalEmptyViaForceX + sbAttractSameViaStrength * totalSameViaForceX;
                // flen_t totalViaForceY = sbAttractEmptyViaStrength * totalEmptyViaForceY + sbAttractSameViaStrength * totalSameViaForceY;
                // if(isShow){

                //     std::cout << "Via Sum = " << totalViaForceX << ", " << totalViaForceY << std::endl;
                // }
                



            }
        }

    }
}

void PressureSimulator::fixPolygons(){
    // the updated contour is in cacheContour, process them.

    // constraint shape within canvas
    for(int layer = 0; layer < getMetalLayerCount(); ++layer){
        std::unordered_map<SignalType, std::vector<FMultiPolygon>> groups;
    }
    // detect overlaps & merging

    for(int layer = 0; layer < getMetalLayerCount(); ++layer){
        std::cout << "fixing for layer " << layer << std::endl;
        for(int softBodyIdx = 0; softBodyIdx < m_OwnerSoftBodies[layer].size(); ++softBodyIdx){
            SoftBody *sb = m_OwnerSoftBodies[layer][softBodyIdx];
            sb->contour = std::move(sb->cacheContour);
            std::cout << "Remeshing for: " << sb->getID() << std::endl;
            sb->remeshContour(m_params.pointsMaxDelta);
        }
    }

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
