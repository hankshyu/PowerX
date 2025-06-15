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
#include "binSystem.hpp"

PressureSimulator::PressureSimulator(const std::string &fileName): PowerDistributionNetwork(fileName) {
    int softBodyIdx = 0;
    int viaBodyIdx = 0;
    const flen_t INITIAL_MARGIN = 0.5; // be a value between 0 ~ 0.5
    
    this->m_OwnerSoftBodies.resize(m_metalLayerCount);
    this->m_OwnerViasBodies.resize(m_viaLayerCount);
    
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
        for(auto cit = viaLayers[constructLayer].preplacedCords.begin(); cit != viaLayers[constructLayer].preplacedCords.end(); ++ cit){
            SignalType st = cit->first;
            if(currentRequiringSignals.count(st) == 0) continue;

            for(const Cord &c : cit->second){
                len_t CordX = c.x();
                len_t CordY = c.y();
                FPoint preplacedCentre(CordX, CordY);

                bool pointCovered = false;
                for(const FBox &existedFBox : layerInitBoxes.at(constructLayer)[st]){
                    if(fbox::isContained(preplacedCentre, existedFBox)){
                        pointCovered = true;
                        break;
                    }
                }

                if(!pointCovered){
                    FPoint preplacedBoxLL(CordX - INITIAL_MARGIN, CordY - INITIAL_MARGIN);
                    FPoint preplacedBoxUR(CordX + INITIAL_MARGIN, CordY + INITIAL_MARGIN);
                    layerInitBoxes.at(constructLayer)[st].emplace_back(preplacedBoxLL, preplacedBoxUR);
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

                m_OwnerSoftBodies.at(constructLayer).push_back(sb);
                softBodyBoundingBox.at(constructLayer).push_back(fp::getBBox(poly));
            }
        }
    
    }

    /* Insert Vias, link to softbodies*/



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

void PressureSimulator::inflate(){
    const int maxInflateIteration = 100;
    const flen_t binSize = 1.0;
    
    for(int inflateIteration = 0; inflateIteration < maxInflateIteration; ++inflateIteration){
        std::cout << "[Inflation BroadCast] Iteration " << inflateIteration << " starts" << std::endl;
        
        // up/down parallel
        for(int layer = 0; layer < getMetalLayerCount(); ++layer){
        std::cout << "[Inflation BroadCast] Layer " << layer << " starts" << std::endl;

            
            BinSystem<flen_t, SoftBody> layerPointsBin(1.0, 0, 0, m_gridWidth-1, m_gridHeight-1);
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