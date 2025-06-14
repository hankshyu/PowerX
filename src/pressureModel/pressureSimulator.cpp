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
    
    this->ownerSoftBodies.resize(m_metalLayerCount);
    this->ownerViasBodies.resize(m_viaLayerCount);
    
    // start gathering softBodies, 
    std::unordered_set<SignalType> currentRequiringSignals;
    std::unordered_map<SignalType, double> signalCurrentRequirements;
    std::unordered_map<SignalType, farea_t> signalTotalArea;
    std::unordered_map<SignalType, std::vector<FBox>> layer0InitBallons;
    
    for(auto cit = uBump.signalTypeToInstances.begin(); cit != uBump.signalTypeToInstances.end(); ++cit){
        
        SignalType st = cit->first;
        if(st == SignalType::EMPTY || st == SignalType::GROUND || st == SignalType::SIGNAL || st == SignalType::OBSTACLE) continue;

        currentRequiringSignals.insert(st);
        std::cout << "Processing signal type: " << st << std::endl;
        
        for(std::string instance : cit->second){
            std::cout << "Instance: " << instance << std::endl;
            BallOut *ballout = uBump.instanceToBallOutMap.at(instance);
            
            double balloutCurrent = ballout->getMaxCurrent();
            signalCurrentRequirements[st] += balloutCurrent;

            for(auto at : ballout->SignalTypeToAllCords){
                if(at.first == SignalType::SIGNAL || at.first == SignalType::GROUND) continue;
                len_t xMin = LEN_T_MAX, xMax = LEN_T_MIN;
                len_t yMin = LEN_T_MAX, yMax = LEN_T_MIN;
                for(Cord c : at.second){
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

                layer0InitBallons[cit->first].emplace_back(FBox(FPoint(xMin, yMin), FPoint(xMax, yMax)));
            }
        }
        
    }

    // add preplaced metal into planning
    for(auto cit = metalLayers[m_ubumpConnectedMetalLayerIdx].preplacedCords.begin(); cit != metalLayers[m_ubumpConnectedMetalLayerIdx].preplacedCords.end(); ++cit){
        SignalType st = cit->first;
        if(currentRequiringSignals.find(st) == currentRequiringSignals.end()) continue;

        std::vector<FBox> addedBox;
        for(const Cord &c : cit->second){
            len_t CordX = c.x();
            len_t CordY = c.y();
            FPoint preplacedBoxLL(CordX, CordY);
            FPoint preplacedBoxUR(CordX + 1, CordY + 1);

            FBox preplacedBox(preplacedBoxLL, preplacedBoxUR);
            bool completelyWithin = false;
            for(const FBox &existedFBox : layer0InitBallons[st]){
                if(fbox::isContained(preplacedBoxLL, existedFBox) && fbox::isContained(preplacedBoxUR, existedFBox)){
                    completelyWithin = true;
                    break;
                }
            }

            if(!completelyWithin) addedBox.emplace_back(FBox(preplacedBoxLL, preplacedBoxUR));
        }
        layer0InitBallons[st].insert(layer0InitBallons[st].end(), std::make_move_iterator(addedBox.begin()), std::make_move_iterator(addedBox.end()));

    }

    // add preplaced via into planning
    for(auto cit = viaLayers[m_ubumpConnectedMetalLayerIdx].preplacedCords.begin(); cit != viaLayers[m_ubumpConnectedMetalLayerIdx].preplacedCords.end(); ++ cit){
        SignalType st = cit->first;
        if(currentRequiringSignals.find(st) == currentRequiringSignals.end()) continue;

        std::vector<FBox> addedPoints;
        for(const Cord &c : cit->second){
            len_t CordX = c.x();
            len_t CordY = c.y();
            FPoint preplacedCentre(CordX, CordY);

            bool completelyWithin = false;
            for(const FBox &existedFBox : layer0InitBallons[st]){
                if(fbox::isContained(preplacedCentre, existedFBox)){
                    completelyWithin = true;
                    break;
                }
            }

            if(!completelyWithin){
                FPoint preplacedBoxLL(CordX - INITIAL_MARGIN, CordY - INITIAL_MARGIN);
                FPoint preplacedBoxUR(CordX + INITIAL_MARGIN, CordY + INITIAL_MARGIN);
                std::cout << "New point added: " << preplacedCentre << std::endl;
                addedPoints.emplace_back(FBox(preplacedBoxLL, preplacedBoxUR));

            }
        }
        layer0InitBallons[st].insert(layer0InitBallons[st].end(), std::make_move_iterator(addedPoints.begin()), std::make_move_iterator(addedPoints.end()));
    }

    // remove completely within boxes
    for(auto it = layer0InitBallons.begin(); it != layer0InitBallons.end(); ++it){
        SignalType st = it->first;

        // Track which boxes to keep
        std::vector<bool> toRemove(it->second.size(), false);
        
        for (size_t i = 0; i < it->second.size()-1; ++i) {
            for (size_t j = i+1; j < it->second.size(); ++j) {
                if(fbox::isContained(it->second.at(i), it->second.at(j)) || fbox::isContained(it->second.at(j), it->second.at(i))){
                    toRemove[i] = true;
                    continue;
                }
            }
        }

        // Read/Write pointer loop compacts the vector in-place by moving unremoved elements forward and then resizing to discard the rest.
        size_t writeIdx = 0;
        for (size_t readIdx = 0; readIdx < it->second.size(); ++readIdx) {
            if (!toRemove[readIdx]){
                signalTotalArea[st] += fbox::getArea(it->second[readIdx]);
                it->second[writeIdx++] = std::move(it->second[readIdx]);
            }
        }

        it->second.resize(writeIdx);

    }

    // insert into ownerSoftBodies vector
    for(auto it = layer0InitBallons.begin(); it != layer0InitBallons.end(); ++it){
        SignalType st = it->first;
        double perUnitCurrent = signalCurrentRequirements.at(st) / signalTotalArea.at(st);
        for(const FBox &fb : it->second){
            farea_t fbArea = fbox::getArea(fb);
            double initCurrent = perUnitCurrent * fbArea;

            SoftBody *sb = new SoftBody(softBodyIdx++, st, initCurrent, fbArea);
            
            // wind clockwise, pushing in points using linear interpolation keeping delta < 0.2
            // start pusing in
            FPoint boxLL(fb.min_corner());
            FPoint boxUR(fb.max_corner());

            flen_t minX = boxLL.x();
            flen_t minY = boxLL.y();
            flen_t maxX = boxUR.x();
            flen_t maxY = boxUR.y();

            sb->contour.emplace_back(FPoint(minX, minY));
            sb->contour.emplace_back(FPoint(minX, maxY));
            sb->contour.emplace_back(FPoint(maxX, maxY));
            sb->contour.emplace_back(FPoint(maxX, minY));


            ownerSoftBodies[m_ubumpConnectedMetalLayerIdx].push_back(sb);

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
            for(int softBodyIdx = 0; softBodyIdx < ownerSoftBodies.size(); ++softBodyIdx){
                SoftBody *sbTarget = ownerSoftBodies.at(layer).at(softBodyIdx);
                
                for(int pointIdx = 0; pointIdx < sbTarget->contour.size(); ++pointIdx){
                    FPoint &fp = sbTarget->contour.at(pointIdx);
                    layerPointsBin.insert(fp.x(), fp.y(), sbTarget);
                }
            }

            // parallel

            for(int softBodyIdx = 0; softBodyIdx < ownerSoftBodies.size(); ++softBodyIdx){
                SoftBody *sbTarget = ownerSoftBodies.at(layer).at(softBodyIdx);
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