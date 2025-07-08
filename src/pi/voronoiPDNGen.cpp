//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        04/06/2025 18:15:19
//  Module Name:        voronoiPDNGen.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        Implementation of 
//                      Chia-Wei Lin, Jing-Yao Weng, I-Te Lin, 
//                      Ho-Chieh Hsu, Chia-Ming Liu, and Mark Po-Hung Lin. 2024.
//                      "Voronoi Diagram-based Multiple Power Plane Generation on Redistribution Layers in 3D ICs." 
//                      In Proceedings of the 61st ACM/IEEE Design Automation Conference (DAC '24).
//                      Association for Computing Machinery, New York, NY, USA, Article 19, 1–6.
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//  05/04/2025          Swap parent class to powerDistributionNetwork, support multi-layer plane and preplaced signals
//
//////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>
#include <stack>
#include <memory>
#include <assert.h>
#include <omp.h> // parallel computing

// 2. Boost Library:
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include "boost/polygon/polygon.hpp"
#include "boost/geometry.hpp"

// 3. Texo Library:
#include "units.hpp"
#include "cord.hpp"
#include "fcord.hpp"
#include "line.hpp"
#include "orderedSegment.hpp"
#include "voronoiPDNGen.hpp"
#include "doughnutPolygon.hpp"
#include "doughnutPolygonSet.hpp"

// 4. FLUTE
#include "flute.h"

// 5.GEOS
#define USE_UNSTABLE_GEOS_CPP_API  // Suppress GEOS C++ warning
#include "geos/geom/Coordinate.h"
#include "geos/geom/Envelope.h"
#include "geos/geom/GeometryFactory.h"
#include "geos/geom/Point.h"
#include "geos/geom/Geometry.h"
#include "geos/geom/MultiPoint.h"
#include "geos/triangulate/VoronoiDiagramBuilder.h"

void VoronoiPDNGen::fixRepeatedPoints(std::unordered_map<SignalType, std::vector<Cord>> &layerPoints){
    bool haveFix = false;
    std::unordered_map<Cord, SignalType> table;
    for(std::unordered_map<SignalType, std::vector<Cord>>::iterator it = layerPoints.begin(); it != layerPoints.end(); ++it){
        SignalType st = it->first;
        for(const Cord &cord : it->second){
            std::unordered_map<Cord, SignalType>::iterator fit = table.find(cord);
            if(fit != table.end()){
                assert(fit->second == st);
                haveFix = true;
                // std::cout << "[PowerX:FixRepeatedPoints] Repeated points found, fixed " << cord << std::endl;

            }else{
                table[cord] = st;
            }
        }
    }
    for(std::unordered_map<SignalType, std::vector<Cord>>::iterator it = layerPoints.begin(); it != layerPoints.end(); ++it){
        it->second.clear();
    }
    for(std::unordered_map<Cord, SignalType>::const_iterator cit = table.begin(); cit != table.end(); ++cit){
        layerPoints[cit->second].push_back(cit->first);
    }

    // if(!haveFix) std::cout << "[PowerX:FixRepeatedPoints] Repeated points not found" << std::endl;
}

void VoronoiPDNGen::fixRepeatedSegments(std::unordered_map<SignalType, std::vector<OrderedSegment>> &layerSegments){
    bool haveFix = false;
    std::unordered_map<OrderedSegment, SignalType> table;
    for(std::unordered_map<SignalType, std::vector<OrderedSegment>>::iterator it = layerSegments.begin(); it != layerSegments.end(); ++it){
        SignalType st = it->first;
        for(const OrderedSegment &os : it->second){
            std::unordered_map<OrderedSegment, SignalType>::iterator fit = table.find(os);
            if(fit != table.end()){
                assert(fit->second == st);
                haveFix = true;
                // std::cout << "[PowerX:fixRepeatedSegments] Repeated Segment found, fixed " << os << std::endl;

            }else{
                table[os] = st;
            }
        }
    }
    for(std::unordered_map<SignalType, std::vector<OrderedSegment>>::iterator it = layerSegments.begin(); it != layerSegments.end(); ++it){
        it->second.clear();
    }
    for(std::unordered_map<OrderedSegment, SignalType>::const_iterator cit = table.begin(); cit != table.end(); ++cit){
        layerSegments[cit->second].push_back(cit->first);
    }

    // if(!haveFix) std::cout << "[PowerX:fixRepeatedSegments] Repeated segments not found" << std::endl;
}

VoronoiPDNGen::VoronoiPDNGen(const std::string &fileName): PowerDistributionNetwork(fileName) {
    this->pointsOfLayers.resize(this->m_metalLayerCount);
    this->segmentsOfLayers.resize(this->m_metalLayerCount);
    this->voronoiCellsOfLayers.resize(this->m_metalLayerCount);
    this->multiPolygonsOfLayers.resize(this->m_metalLayerCount);
}

void VoronoiPDNGen::markPreplacedAndInsertPads(){
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
    
    for(int mLayer = m_ubumpConnectedMetalLayerIdx; mLayer <= m_c4ConnectedMetalLayerIdx; ++mLayer){
        this->preplaceOfLayers.push_back(this->metalLayers[mLayer].canvas);
    }
}

void VoronoiPDNGen::initPointsAndSegments(){
    
    const std::unordered_set<SignalType> uBumpSOI = {
        SignalType::POWER_1, SignalType::POWER_2, SignalType::POWER_3, SignalType::POWER_4, SignalType::POWER_5,
        SignalType::POWER_6, SignalType::POWER_7, SignalType::POWER_8, SignalType::POWER_9, SignalType::POWER_10
    };
    const std::unordered_set<SignalType> c4SOI = {
        SignalType::POWER_1, SignalType::POWER_2, SignalType::POWER_3, SignalType::POWER_4, SignalType::POWER_5,
        SignalType::POWER_6, SignalType::POWER_7, SignalType::POWER_8, SignalType::POWER_9, SignalType::POWER_10
    };
    const std::unordered_set<SignalType> viaSOI = {
        SignalType::POWER_1, SignalType::POWER_2, SignalType::POWER_3, SignalType::POWER_4, SignalType::POWER_5,
        SignalType::POWER_6, SignalType::POWER_7, SignalType::POWER_8, SignalType::POWER_9, SignalType::POWER_10
};
    const std::unordered_set<SignalType> metalSOI = {
        SignalType::POWER_1, SignalType::POWER_2, SignalType::POWER_3, SignalType::POWER_4, SignalType::POWER_5,
        SignalType::POWER_6, SignalType::POWER_7, SignalType::POWER_8, SignalType::POWER_9, SignalType::POWER_10
    };


    // prepare the points for uBump connecting layer
    std::unordered_map<SignalType, std::unordered_set<Cord>> ubumpcheckMap;
    
    // points on uBump that is POI
    for(std::unordered_map<SignalType, std::unordered_set<Cord>>::const_iterator cit = uBump.signalTypeToAllCords.begin(); cit != uBump.signalTypeToAllCords.end(); ++cit){
        SignalType st = cit->first;
        if(uBumpSOI.count(st) == 0) continue;

        ubumpcheckMap[st].insert(cit->second.begin(), cit->second.end());
    }

    // points on the via below uBump connecting layer that's POI
    for(std::unordered_map<SignalType, std::vector<Cord>>::const_iterator cit = viaLayers[m_ubumpConnectedMetalLayerIdx].preplacedCords.begin(); cit != viaLayers[m_ubumpConnectedMetalLayerIdx].preplacedCords.end(); ++cit){
        SignalType st = cit->first;
        if(viaSOI.count(st) == 0) continue;
        ubumpcheckMap[st].insert(cit->second.begin(), cit->second.end());
    }

    // points on the metal layer itself that is POI
    for(std::unordered_map<SignalType, std::vector<Cord>>::const_iterator cit = metalLayers[m_ubumpConnectedMetalLayerIdx].preplacedCords.begin(); cit != metalLayers[m_ubumpConnectedMetalLayerIdx].preplacedCords.end(); ++cit){
        SignalType st = cit->first;
        if(metalSOI.count(st) == 0) continue;
        for(const Cord &c : cit->second){
            Cord llPointOfBox(c);
            Cord lrPointOfBox(c.x() + 1, c.y());
            Cord ulPointOfBox(c.x(), c.y() + 1);
            Cord urPointOfBox(c.x() + 1, c.y() + 1);
            ubumpcheckMap[st].insert(llPointOfBox);
            ubumpcheckMap[st].insert(lrPointOfBox);
            ubumpcheckMap[st].insert(ulPointOfBox);
            ubumpcheckMap[st].insert(urPointOfBox);
        }
    }

    // push ubumpCheckMap points into pointsOfLayers and initialise segmentsOflayers
    std::unordered_set<Cord> ubumpSeenCords;
    for(std::unordered_map<SignalType, std::unordered_set<Cord>>::const_iterator cit = ubumpcheckMap.begin(); cit != ubumpcheckMap.end(); ++cit){
        
        if(cit->second.empty()) continue;
        SignalType st = cit->first;

        for(const Cord &c : cit->second){
            if(ubumpSeenCords.count(c) != 0){
                std::cout << "[PowerX:VoronoiPDNGen] Warning: Repeated Point of interest found when processing uBump connecting metal layer: " << c << std::endl;
                exit(4);
            }else{
                ubumpSeenCords.insert(c);
            }
        }

        this->pointsOfLayers[m_ubumpConnectedMetalLayerIdx][st] = std::vector<Cord>(cit->second.begin(), cit->second.end());
        this->segmentsOfLayers[m_ubumpConnectedMetalLayerIdx][st] = {};
    }


    // prepare the points for c4 connecting layer
    std::unordered_map<SignalType, std::unordered_set<Cord>> c4checkMap;

    // points on c4 that is POI
    for(std::unordered_map<SignalType, std::unordered_set<Cord>>::const_iterator cit = c4.signalTypeToAllCords.begin(); cit != c4.signalTypeToAllCords.end(); ++cit){
        SignalType st = cit->first;
        if(c4SOI.count(st) == 0) continue;
        c4checkMap[st].insert(cit->second.begin(), cit->second.end());
    }

    // points on the via above c4 connecting layer that's POI
    for(std::unordered_map<SignalType, std::vector<Cord>>::const_iterator cit = viaLayers[m_c4ConnectedMetalLayerIdx-1].preplacedCords.begin(); cit != viaLayers[m_c4ConnectedMetalLayerIdx-1].preplacedCords.end(); ++cit){
        SignalType st = cit->first;
        if(viaSOI.count(st) == 0) continue;
        c4checkMap[st].insert(cit->second.begin(), cit->second.end());
    }

    // points on the metal layer itself that is POI
    for(std::unordered_map<SignalType, std::vector<Cord>>::const_iterator cit = metalLayers[m_c4ConnectedMetalLayerIdx].preplacedCords.begin(); cit != metalLayers[m_c4ConnectedMetalLayerIdx].preplacedCords.end(); ++cit){
        SignalType st = cit->first;
        if(metalSOI.count(st) == 0) continue;
        for(const Cord &c : cit->second){
            Cord llPointOfBox(c);
            Cord lrPointOfBox(c.x() + 1, c.y());
            Cord ulPointOfBox(c.x(), c.y() + 1);
            Cord urPointOfBox(c.x() + 1, c.y() + 1);
            c4checkMap[st].insert(llPointOfBox);
            c4checkMap[st].insert(lrPointOfBox);
            c4checkMap[st].insert(ulPointOfBox);
            c4checkMap[st].insert(urPointOfBox);
        }
    }

    // push c4checkMap points into pointsOfLayers and initialise segmentsOflayers
    std::unordered_set<Cord> c4seenCords;
    for(std::unordered_map<SignalType, std::unordered_set<Cord>>::const_iterator cit = c4checkMap.begin(); cit != c4checkMap.end(); ++cit){
        
        if(cit->second.empty()) continue;
        SignalType st = cit->first;
        
        for(const Cord &c : cit->second){
            if(c4seenCords.count(c) != 0){
                std::cout << "[PowerX:VoronoiPDNGen] Warning: Repeated Point of interest found when processing c4 connecting metal layer: " << c << std::endl;
                exit(4);
            }else{
                c4seenCords.insert(c);
            }
        }
        this->pointsOfLayers[m_c4ConnectedMetalLayerIdx][st] = std::vector<Cord>(cit->second.begin(), cit->second.end());
        this->segmentsOfLayers[m_c4ConnectedMetalLayerIdx][st] = {};
    }

    // prepare points other layers, which comes from:
    // 1. the preplaced grids from such layer
    // 2. the up and down via points
    // TODO

    for(int mLayer = m_ubumpConnectedMetalLayerIdx+1; mLayer < m_c4ConnectedMetalLayerIdx; ++mLayer){
        std::unordered_map<SignalType, std::unordered_set<Cord>> processPoints;

        // points on the top via that is POI
        for(std::unordered_map<SignalType, std::vector<Cord>>::const_iterator cit = viaLayers[mLayer-1].preplacedCords.begin(); cit != viaLayers[mLayer-1].preplacedCords.end(); ++cit){
            SignalType st = cit->first;
            if(viaSOI.count(st) == 0) continue;
            processPoints[st].insert(cit->second.begin(), cit->second.end());
        }

        // points on the bottom that that is POI
        for(std::unordered_map<SignalType, std::vector<Cord>>::const_iterator cit = viaLayers[mLayer].preplacedCords.begin(); cit != viaLayers[mLayer].preplacedCords.end(); ++cit){
            SignalType st = cit->first;
            if(viaSOI.count(st) == 0) continue;
            processPoints[st].insert(cit->second.begin(), cit->second.end());
        }

        // points on the metal layer itself that is POI
        for(std::unordered_map<SignalType, std::vector<Cord>>::const_iterator cit = metalLayers[mLayer].preplacedCords.begin(); cit != metalLayers[mLayer].preplacedCords.end(); ++cit){
            SignalType st = cit->first;
            if(metalSOI.count(st) == 0) continue;
            for(const Cord &c : cit->second){
                Cord llPointOfBox(c);
                Cord lrPointOfBox(c.x() + 1, c.y());
                Cord ulPointOfBox(c.x(), c.y() + 1);
                Cord urPointOfBox(c.x() + 1, c.y() + 1);
                processPoints[st].insert(llPointOfBox);
                processPoints[st].insert(lrPointOfBox);
                processPoints[st].insert(ulPointOfBox);
                processPoints[st].insert(urPointOfBox);
            }
        }

        // push ubumpCheckMap points into pointsOfLayers and initialise segmentsOflayers
        std::unordered_set<Cord> seenCords;
        for(std::unordered_map<SignalType, std::unordered_set<Cord>>::const_iterator cit = processPoints.begin(); cit != processPoints.end(); ++cit){
            
            if(cit->second.empty()) continue;
            SignalType st = cit->first;

            for(const Cord &c : cit->second){
                if(seenCords.count(c) != 0){
                    std::cout << "[PowerX:VoronoiPDNGen] Warning: Repeated Point of interest(" << c << ") found when processing metal layer: " << mLayer << std::endl;
                    exit(4);
                }else{
                    seenCords.insert(c);
                }
            }

            this->pointsOfLayers[mLayer][st] = std::vector<Cord>(cit->second.begin(), cit->second.end());
            this->segmentsOfLayers[mLayer][st] = {};
        }
    }
}

void VoronoiPDNGen::connectLayers(int upLayerIdx, int downLayerIdx){
    assert(upLayerIdx >= m_ubumpConnectedMetalLayerIdx);
    assert(downLayerIdx <= m_c4ConnectedMetalLayerIdx);
    assert(downLayerIdx == (upLayerIdx+1));

    const std::unordered_set<SignalType> metalSOI = {
        SignalType::POWER_1, SignalType::POWER_2, SignalType::POWER_3, SignalType::POWER_4, SignalType::POWER_5,
        SignalType::POWER_6, SignalType::POWER_7, SignalType::POWER_8, SignalType::POWER_9, SignalType::POWER_10
    };

    // collect all signal of interests
    std::unordered_set<SignalType> allSigTypes;
    for(auto at : this->metalLayers[upLayerIdx].preplacedCords){
        if(!at.second.empty()) allSigTypes.insert(at.first);
    }
    for(auto at : this->metalLayers[downLayerIdx].preplacedCords){
        if(!at.second.empty()) allSigTypes.insert(at.first);
    }
    for(std::unordered_set<SignalType>::iterator it = allSigTypes.begin(); it != allSigTypes.end(); ){
        // erease signal if it's not SOI, or it already apperas in the via (valid)
        if(metalSOI.count(*it) == 0) it = allSigTypes.erase(it);
        else if(this->viaLayers[upLayerIdx].preplacedCords.count(*it) != 0) it = allSigTypes.erase(it);
        else ++it;
    }



    for(const SignalType &st : allSigTypes){
        // check if existing points already exists in the upLayerIdx and downLayerIdx
        FCord upCordAvg(0, 0);
        FCord downCordAvg(0, 0);

        if(pointsOfLayers[upLayerIdx].count(st) != 0){
            for(const Cord &c : pointsOfLayers[upLayerIdx][st]){
                upCordAvg.x(upCordAvg.x() + c.x());
                upCordAvg.y(upCordAvg.y() + c.y());
            }
            upCordAvg.x(upCordAvg.x() / pointsOfLayers[upLayerIdx][st].size());
            upCordAvg.y(upCordAvg.y() / pointsOfLayers[upLayerIdx][st].size());
        }else{
            upCordAvg = FCord(-1, -1);
        }

        if(pointsOfLayers[downLayerIdx].count(st) != 0){
            for(const Cord &c : pointsOfLayers[downLayerIdx][st]){
                downCordAvg.x(downCordAvg.x() + c.x());
                downCordAvg.y(downCordAvg.y() + c.y());
            }
            downCordAvg.x(downCordAvg.x() / pointsOfLayers[downLayerIdx][st].size());
            downCordAvg.y(downCordAvg.y() / pointsOfLayers[downLayerIdx][st].size());
        }else{
            downCordAvg = FCord(-1, -1);
        } 

        // select candidates that should not be occupied and pass the pad test

        std::unordered_set<Cord> upLayerBlockingPoints;
        std::unordered_set<Cord> downLayerBlockingPoints;
        std::unordered_set<Cord> allEmptyVias;
        
        for(auto cit = pointsOfLayers[upLayerIdx].begin(); cit != pointsOfLayers[upLayerIdx].end(); ++cit){
            if(cit->first == st) continue;
            upLayerBlockingPoints.insert(cit->second.begin(), cit->second.end());
        }

        for(auto cit = pointsOfLayers[downLayerIdx].begin(); cit != pointsOfLayers[downLayerIdx].end(); ++cit){
            if(cit->first == st) continue;
            downLayerBlockingPoints.insert(cit->second.begin(), cit->second.end());
        }

        std::unordered_set<Cord> occupiedVias;
        for(auto cit = viaLayers[upLayerIdx].preplacedCords.begin(); cit != viaLayers[upLayerIdx].preplacedCords.end(); ++cit){
            occupiedVias.insert(cit->second.begin(), cit->second.end());
        }

        for(int j = 0; j < getPinHeight(); ++j){
            for(int i = 0; i < getPinHeight(); ++i){
                Cord c(i, j);
                if(occupiedVias.count(c) != 0) continue;
                if(upLayerBlockingPoints.count(c) != 0) continue;
                if(downLayerBlockingPoints.count(c) != 0) continue;
                
                allEmptyVias.insert(Cord(i, j));
            }
        }

        for(std::unordered_set<Cord>::iterator it = allEmptyVias.begin(); it != allEmptyVias.end(); ){
            // test pads are valid
            Cord pin(*it);
            len_t xHigh = pin.x();
            len_t xLow = (xHigh > 0)? xHigh-1 : xHigh;
            len_t yHigh = pin.y();
            len_t yLow = (yHigh > 0)? yHigh-1 : yHigh;

            Cord ll(xLow, yLow);
            Cord lr(xHigh, yLow);
            Cord ul(xLow, yHigh);
            Cord ur(xHigh, yHigh);

            bool pinValid = true;
            for(auto cit = metalLayers[upLayerIdx].preplacedCords.begin(); cit != metalLayers[upLayerIdx].preplacedCords.end(); ++cit){
                SignalType findingSt = cit->first;
                if(findingSt == st){
                    continue;
                }else if(metalLayers[upLayerIdx].canvas[ll.y()][ll.x()] != st && metalLayers[upLayerIdx].canvas[ll.y()][ll.x()] != SignalType::EMPTY){
                    pinValid = false;
                    break;
                }else if(metalLayers[upLayerIdx].canvas[lr.y()][lr.x()] != st && metalLayers[upLayerIdx].canvas[lr.y()][lr.x()] != SignalType::EMPTY){
                    pinValid = false;
                    break;
                }else if(metalLayers[upLayerIdx].canvas[ul.y()][ul.x()] != st && metalLayers[upLayerIdx].canvas[ul.y()][ul.x()] != SignalType::EMPTY){
                    pinValid = false;
                    break;
                }else if(metalLayers[upLayerIdx].canvas[ur.y()][ur.x()] != st && metalLayers[upLayerIdx].canvas[ur.y()][ur.x()] != SignalType::EMPTY){
                    pinValid = false;
                    break;
                }
            }

            if(pinValid){
                for(auto cit = metalLayers[downLayerIdx].preplacedCords.begin(); cit != metalLayers[downLayerIdx].preplacedCords.end(); ++cit){
                    SignalType findingSt = cit->first;
                    if(findingSt == st){
                        continue;
                    }else if(metalLayers[downLayerIdx].canvas[ll.y()][ll.x()] != st && metalLayers[downLayerIdx].canvas[ll.y()][ll.x()] != SignalType::EMPTY){
                        pinValid = false;
                        break;
                    }else if(metalLayers[downLayerIdx].canvas[lr.y()][lr.x()] != st && metalLayers[downLayerIdx].canvas[lr.y()][lr.x()] != SignalType::EMPTY){
                        pinValid = false;
                        break;
                    }else if(metalLayers[downLayerIdx].canvas[ul.y()][ul.x()] != st && metalLayers[downLayerIdx].canvas[ul.y()][ul.x()] != SignalType::EMPTY){
                        pinValid = false;
                        break;
                    }else if(metalLayers[downLayerIdx].canvas[ur.y()][ur.x()] != st && metalLayers[downLayerIdx].canvas[ur.y()][ur.x()] != SignalType::EMPTY){
                        pinValid = false;
                        break;
                    }
                }
            }

            // erase or increment
            if(!pinValid) it = allEmptyVias.erase(it);
            else ++it;
        }
        
        // select the candidates that is closest to upCord
        flen_t minDistance = FLEN_T_MAX;
        Cord currBest(-1, -1);

        for(const Cord &c : allEmptyVias){
            flen_t distance = 0;
            if(upCordAvg != FCord(-1, -1)) distance += calEuclideanDistance(upCordAvg, c);
            if(downCordAvg != FCord(-1, -1)) distance += calEuclideanDistance(downCordAvg, c);

            if(distance < minDistance){
                minDistance = distance;
                currBest = c;
            }
        }

        // ccurrBest is selected to bridge up and down metal layer, push to preplaced
        viaLayers[upLayerIdx].preplacedCords[st].push_back(currBest);
        viaLayers[upLayerIdx].setCanvas(currBest, st);

        Cord llPad(currBest.x() - 1, currBest.y() - 1);
        Cord lrPad(currBest.x(), currBest.y() - 1);
        Cord ulPad(currBest.x() - 1, currBest.y());
        Cord urPad(currBest.x(), currBest.y());

        metalLayers[upLayerIdx].preplacedCords[st].push_back(llPad);
        metalLayers[upLayerIdx].preplacedCords[st].push_back(lrPad);
        metalLayers[upLayerIdx].preplacedCords[st].push_back(ulPad);
        metalLayers[upLayerIdx].preplacedCords[st].push_back(urPad);
        metalLayers[upLayerIdx].setCanvas(llPad, st);
        metalLayers[upLayerIdx].setCanvas(lrPad, st);
        metalLayers[upLayerIdx].setCanvas(ulPad, st);
        metalLayers[upLayerIdx].setCanvas(urPad, st);

        metalLayers[downLayerIdx].preplacedCords[st].push_back(llPad);
        metalLayers[downLayerIdx].preplacedCords[st].push_back(lrPad);
        metalLayers[downLayerIdx].preplacedCords[st].push_back(ulPad);
        metalLayers[downLayerIdx].preplacedCords[st].push_back(urPad);
        metalLayers[downLayerIdx].setCanvas(llPad, st);
        metalLayers[downLayerIdx].setCanvas(lrPad, st);
        metalLayers[downLayerIdx].setCanvas(ulPad, st);
        metalLayers[downLayerIdx].setCanvas(urPad, st);

        // push to points
        this->pointsOfLayers[upLayerIdx][st].push_back(currBest);
        this->pointsOfLayers[downLayerIdx][st].push_back(currBest);
        
    }
}

void VoronoiPDNGen::runFLUTERouting(std::unordered_map<SignalType, std::vector<Cord>> &layerPoints, std::unordered_map<SignalType, std::vector<OrderedSegment>> &layerSegments){
    const int FLUTE_ACC = 9; // 0 ~ 9
    for(std::unordered_map<SignalType, std::vector<Cord>>::iterator it = layerPoints.begin(); it != layerPoints.end(); ++it){
        SignalType targetSig = it->first;
        int pointCount = it->second.size();
        if(pointCount == 0) continue;
        std::vector<len_t> xArr;
        std::vector<len_t> yArr;
        for(const Cord &c : it->second){
            xArr.push_back(c.x());
            yArr.push_back(c.y());
        }

        Flute::FluteState *state = Flute::flute_init(WIRELENGTH_VECTOR_FILE, ROUTING_TREE_FILE);
        Flute::Tree tree = Flute::flute(state, pointCount, xArr.data(), yArr.data(), FLUTE_ACC);
        // std::cout << targetSig << "Before: " << layerPoints[targetSig].size() << std::endl;
        for(int i = 0; i < tree.deg*2 - 2; ++i){
            int n = tree.branch[i].n;
            Cord c1(tree.branch[i].x, tree.branch[i].y);
            Cord c2(tree.branch[n].x, tree.branch[n].y);
            if(c1 == c2) continue;
            bool foundc1 = false;
            bool foundc2 = false;
            for(int j = 0; j < layerPoints[targetSig].size(); ++j){
                Cord m5c = layerPoints[targetSig].at(j);
                if(m5c == c1) foundc1 = true;
            }
            for(int j = 0; j < layerPoints[targetSig].size(); ++j){
                Cord m5c = layerPoints[targetSig].at(j);
                if(m5c == c2) foundc2 = true;
            }
            if(!foundc1) layerPoints[targetSig].push_back(c1);
            if(!foundc2) layerPoints[targetSig].push_back(c2);

            OrderedSegment newos(c1, c2);
            bool foundos = false;
            for(OrderedSegment os : layerSegments[targetSig]){
                if(os == newos) foundos = true;
            }
            if(!foundos) layerSegments[targetSig].push_back(newos);
        }

        // Free resources
        free_tree(state, tree);
        flute_free(state);
    }
}

void VoronoiPDNGen::runMSTRouting(std::unordered_map<SignalType, std::vector<Cord>> &layerPoints, std::unordered_map<SignalType, std::vector<OrderedSegment>> &layerSegments){
    
    for(std::unordered_map<SignalType, std::vector<Cord>>::iterator it = layerPoints.begin(); it != layerPoints.end(); ++it){
        SignalType targetSig = it->first;
        int pointCount = it->second.size();
        if(pointCount == 0) continue;
        
        std::vector <Cord> pinsVector(it->second.begin(), it->second.end());


        typedef boost::adjacency_list<
            boost::vecS, 
            boost::vecS, 
            boost::undirectedS, 
            boost::no_property, 
            boost::property<boost::edge_weight_t, int>> Graph;
        typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;

        Graph graph(pinsVector.size());
        for(int i = 0; i < pinsVector.size() - 1; ++i){
            for(int j = i+1; j < pinsVector.size(); ++j){
                add_edge(i, j, calManhattanDistance(pinsVector[i], pinsVector[j]), graph);
            }
        }

        std::vector<Vertex> p(num_vertices(graph));
        prim_minimum_spanning_tree(graph, &p[0]);

        for (std::size_t i = 0; i != p.size(); ++i){
            if (p[i] == i) continue;

            Cord c1 = pinsVector[i];
            Cord c2 = pinsVector[p[i]];
            assert(c1 != c2);

            if(c1 == c2) continue;
            bool foundc1 = false;
            bool foundc2 = false;
            for(int j = 0; j < layerPoints[targetSig].size(); ++j){
                Cord m5c = layerPoints[targetSig].at(j);
                if(m5c == c1) foundc1 = true;
            }
            for(int j = 0; j < layerPoints[targetSig].size(); ++j){
                Cord m5c = layerPoints[targetSig].at(j);
                if(m5c == c2) foundc2 = true;
            }
            if(!foundc1) layerPoints[targetSig].push_back(c1);
            if(!foundc2) layerPoints[targetSig].push_back(c2);

            OrderedSegment newos(c1, c2);
            bool foundos = false;
            for(OrderedSegment os : layerSegments[targetSig]){
                if(os == newos) foundos = true;
            }
            if(!foundos) layerSegments[targetSig].push_back(newos);
        }
    }
}

void VoronoiPDNGen::ripAndReroute(std::unordered_map<SignalType, std::vector<Cord>> &layerPoints, std::unordered_map<SignalType, std::vector<OrderedSegment>> &layerSegments){
    std::vector<std::pair<OrderedSegment, OrderedSegment>> rerouteSegmentArr;
    std::unordered_map<OrderedSegment, SignalType> allSegmentMap;

    // fill all Segments
    for(std::unordered_map<SignalType, std::vector<OrderedSegment>>::const_iterator cit = layerSegments.begin(); cit != layerSegments.end(); ++cit){
        for(const OrderedSegment &os : cit->second){
            allSegmentMap[os] = cit->first;
        }
    }

    // run segment intersect checking
    for(std::unordered_map<SignalType, std::vector<OrderedSegment>>::iterator lsit = layerSegments.begin(); lsit != layerSegments.end(); ++lsit){
        for(OrderedSegment os : lsit->second){
            for(std::unordered_map<OrderedSegment, SignalType>::iterator it = allSegmentMap.begin(); it != allSegmentMap.end(); ++it){
                if(lsit->first == it->second) continue;
                OrderedSegment cos = it->first;
                if (os == cos) continue;
                if(seg::intersects(os, cos) ){
                    rerouteSegmentArr.push_back(std::pair<OrderedSegment, OrderedSegment>(os, cos));
                }
            }
        }
    }

    

    std::vector<OrderedSegment> ripArr;
    // attempt to fix
    while(!rerouteSegmentArr.empty()){
        std::pair<OrderedSegment, OrderedSegment> fixPair = rerouteSegmentArr[0];
        // copare the segment length:
        flen_t firstLength = calEuclideanDistance(fixPair.first.getLow(), fixPair.first.getHigh());
        flen_t secondLength = calEuclideanDistance(fixPair.second.getLow(), fixPair.second.getHigh());
        OrderedSegment removeTarget;
        if(firstLength > secondLength){
            removeTarget = fixPair.first;
        }else{
            removeTarget = fixPair.second;
        }

        ripArr.push_back(removeTarget);
        rerouteSegmentArr.erase(remove_if(rerouteSegmentArr.begin(), rerouteSegmentArr.end(),
                [=](const std::pair<OrderedSegment, OrderedSegment>& p) {
                    return p.first == removeTarget || p.second == removeTarget;
                }
            ),rerouteSegmentArr.end()
        );
    
    }

    // remove OrderedSegment from the layerSegments
    for(OrderedSegment dos : ripArr){
        for(std::unordered_map<SignalType, std::vector<OrderedSegment>>::iterator it = layerSegments.begin(); it != layerSegments.end(); ++it){
            it->second.erase(std::remove(it->second.begin(), it->second.end(), dos), it->second.end());
        }
    }

    // sort the ripArr according to wirelength
    std::sort(ripArr.begin(), ripArr.end(), [](OrderedSegment os1, OrderedSegment os2){
        return calEuclideanDistance(os1.getLow(), os1.getHigh()) < calEuclideanDistance(os2.getLow(), os2.getHigh());
    });

    for(const OrderedSegment &cos : ripArr){

        SignalType cosSt = allSegmentMap[cos];
        Cord start(cos.getLow());
        Cord goal(cos.getHigh());
        std::cout << "Attempt BFS Routing From " << start << " -> " << goal << " of type: " << cosSt << " ";

        std::vector<std::vector<int>> nodeStat(getPinHeight(), std::vector<int>(getPinWidth(), 0));
        
        using P45Data = boost::polygon::polygon_45_data<len_t>;
        std::vector<P45Data> blockingObjects;
        std::vector<Segment> blockingSegs;


        for(std::unordered_map<SignalType, std::vector<OrderedSegment>>::const_iterator cit = layerSegments.begin(); cit != layerSegments.end(); ++cit){
            if(cit->first == cosSt) continue;
            for(const OrderedSegment &cbos : cit->second){
                Cord c1(cbos.getLow());
                Cord c2(cbos.getHigh());
                std::vector<Cord> windings;

                if(c1.x() == c2.x()){
                    if(c1.y() > c2.y()) std::swap(c1, c2);
                    windings = {Cord(c2.x()-1, c2.y()+1), Cord(c2.x()+1, c2.y()+1), Cord(c1.x()+1, c1.y()-1), Cord(c1.x()-1, c1.y()-1)};
                }else{
                    if(c1.x() > c2.x()) std::swap(c1, c2);
                    if(c1.y() > c2.y()){
                        windings = {
                            Cord(c1.x()-1, c1.y()-1), Cord(c1.x()-1, c1.y()+1), Cord(c1.x()+1, c1.y()+1),
                            Cord(c2.x()+1, c2.y()+1), Cord(c2.x()+1, c2.y()-1), Cord(c2.x()-1, c2.y()-1)
                        };
                    }else if(c1.y() < c2.y()){
                        windings = {
                            Cord(c1.x()+1, c1.y()-1), Cord(c1.x()-1, c1.y()-1), Cord(c1.x()-1, c1.y()+1),
                            Cord(c2.x()-1, c2.y()+1), Cord(c2.x()+1, c2.y()+1), Cord(c2.x()+1, c2.y()-1)
                        };
                    }else{ // c1.y() == c2.y()
                        windings = {Cord(c1.x()-1, c1.y()+1), Cord(c2.x()+1, c2.y()+1), Cord(c2.x()+1, c2.y()-1), Cord(c1.x()-1, c1.y()-1)};
                    }
                }

                blockingObjects.emplace_back(P45Data(windings.begin(), windings.end()));
                for(int i = 0; i < windings.size(); ++i){
                    blockingSegs.emplace_back(Segment(windings[i], windings[(i+1)%windings.size()]));
                    }
            }
        }

        for(int j = 0; j < getPinHeight(); ++j){
            for(int i = 0; i <getPinWidth(); ++i){
                Cord c(i, j);
                for(const P45Data &dt : blockingObjects){
                    if(boost::polygon::contains(dt, c)){
                        nodeStat[j][i] = 9;
                        break;
                    } 
                }
            }
        }

        // Start Routing uisng BFS
        // 9: blockages, 1 visited
        std::unordered_map<Cord, Cord> prev;
        std::unordered_map<Cord, int> cost;
        auto comp = [&](const Cord &a, const Cord &b){
            return (cost[a]+calEuclideanDistance(a, goal)) > (cost[b]+calEuclideanDistance(b, goal));
        };

        std::priority_queue<Cord, std::vector<Cord>, decltype(comp)> q(comp);

        cost[start] = 0;
        q.push(start);
        nodeStat[start.y()][start.x()] = 1;
        while(!q.empty()){
            Cord curr = q.top();
            q.pop();

            if(curr == goal) break;
            std::vector<Cord>neighbors;

            std::vector<Cord> candidates;
            #pragma omp parallel
            {
                std::vector<Cord> local_candidates;
                #pragma omp for nowait
                for(int j = 0; j < getPinHeight(); ++j){
                    for(int i = 0; i < getPinWidth(); ++i){
                        if(nodeStat[j][i] != 0) continue;
                        Cord cand(i, j);
                        Segment candSeg(curr, cand);
                        bool intersects = false;
                        for(const Segment &sg : blockingSegs){
                            if(seg::intersects(candSeg, sg)){
                                intersects = true;
                                break;
                            }
                        }
                        if(!intersects){
                            local_candidates.push_back(cand);
                        }
                    }
                }
            
                #pragma omp critical
                candidates.insert(candidates.end(), local_candidates.begin(), local_candidates.end());
            }
            // sequential section: update shared structures
            for (const Cord& cand : candidates) {
                int x = cand.x(), y = cand.y();
                if (nodeStat[y][x] == 0) {
                    nodeStat[y][x] = 1;
                    prev[cand] = curr;
                    cost[cand] = cost[curr] + calManhattanDistance(cand, curr);
                    q.push(cand);
                }
            }

            // std::cout << "Done " << curr << std::endl;
        }

        assert(nodeStat[goal.y()][goal.x()] != 0);
        std::vector<Cord> path;

        for(Cord at = goal; at != start; at = prev[at]){
            path.push_back(at);
        }
        path.push_back(start);
        std::reverse(path.begin(), path.end());
        std::cout << ", chosen path: ";
        for(Cord &c : path){
            std::cout <<  c << " ";
        }
        std::cout << std::endl;
        // add points
        for(int i = 1; i < path.size()-1; ++i){
            layerPoints[cosSt].push_back(path[i]);
        }
        // add paths
        for(int i = 0; i < path.size()-1; ++i){
            layerSegments[cosSt].push_back(OrderedSegment(path[i], path[i+1]));
        }

    }
    
    fixRepeatedPoints(layerPoints);
    fixRepeatedSegments(layerSegments);
}

void VoronoiPDNGen::generateInitialPowerPlanePoints(std::unordered_map<SignalType, std::vector<Cord>> &layerPoints, std::unordered_map<SignalType, std::vector<OrderedSegment>> &layerSegments){
    std::stack<std::pair<OrderedSegment, SignalType>> toFix;
    for(std::unordered_map<SignalType, std::vector<OrderedSegment>>::iterator it = layerSegments.begin(); it != layerSegments.end(); ++it){
        SignalType st = it->first;
        for(std::vector<OrderedSegment>::iterator osit = it->second.begin(); osit!= it->second.end();){
            OrderedSegment os = *osit;
            Cord osLow(os.getLow());
            Cord osHigh(os.getHigh());
            FCord centre(double(osLow.x() + osHigh.x())/2, double(osLow.y() + osHigh.y())/2);
            double radius = calEuclideanDistance(osLow, osHigh) / 2;

            bool needsRemoval = false;
            for(std::unordered_map<SignalType, std::vector<Cord>>::const_iterator pcit = layerPoints.begin(); pcit != layerPoints.end(); ++pcit){
                if(pcit->first == st) continue;
                bool foundIntersect = false;
                for(std::vector<Cord>::const_iterator cordit = pcit->second.begin(); cordit != pcit->second.end(); ++cordit){
                    Cord eCord = *cordit;
                    if(calEuclideanDistance(centre, eCord) <= radius){
                        toFix.push(std::pair<OrderedSegment, SignalType>(os, st));
                        foundIntersect = true;
                        break;
                    }
                }
                needsRemoval = foundIntersect;
                if(needsRemoval) break;
            }

            osit = (needsRemoval)? it->second.erase(osit) : osit + 1;
        }
    }

    while(!toFix.empty()){
        std::pair<OrderedSegment, SignalType> tftop = toFix.top();
        toFix.pop();

        OrderedSegment fixos = tftop.first;
        SignalType fixst = tftop.second;
        Cord osLow(fixos.getLow());
        Cord osHigh(fixos.getHigh());
        FCord centre(double(osLow.x() + osHigh.x())/2, double(osLow.y() + osHigh.y())/2);
        double radius = calEuclideanDistance(osLow, osHigh) / 2;

        bool hasIntersect = false;
        Cord c3;
        
        for(std::unordered_map<SignalType, std::vector<Cord>>::const_iterator pcit = layerPoints.begin(); pcit != layerPoints.end(); ++pcit){
            if(pcit->first == fixst) continue;
            bool foundIntersect = false;
            for(std::vector<Cord>::const_iterator cordit = pcit->second.begin(); cordit != pcit->second.end(); ++cordit){
                Cord eCord = *cordit;
                if(calEuclideanDistance(centre, eCord) <= radius){
                    foundIntersect = hasIntersect = true;
                    c3 = eCord;
                    break;
                }
            }
            if(foundIntersect) break;
        }

        if(hasIntersect){
            // calculate the projection of c3 on line(osLow, osHigh) 
            if(osLow.x() > osHigh.x()) std::swap(osLow, osHigh);

            Cord ab(osHigh.x() - osLow.x(), osHigh.y() - osLow.y());
            Cord ac(c3.x() - osLow.x(), c3.y() - osLow.y());
            double t = double(ac.x()*ab.x() + ac.y()*ab.y()) / double(ab.x()*ab.x() + ab.y()*ab.y());
            Cord projection(osLow.x() + ab.x()*t, osLow.y() + ab.y()*t);
            if(projection == osLow){
                if(osLow.y() < osHigh.y()){
                    projection.x(projection.x()+1);
                    projection.y(projection.y()+1);
                }else if(osLow.y()== osHigh.y()){
                    projection.x(projection.x()+1);
                }else{ // osLow.y() > osHigh.y()
                    projection.x(projection.x()+1);
                    projection.y(projection.y()-1);
                }
            }else if(projection == osHigh){

                if(osLow.y() < osHigh.y()){
                    projection.x(projection.x()-1);
                    projection.y(projection.y()-1);
                }else if(osLow.y() == osHigh.y()){

                    projection.x(projection.x()-1);
                }else{ // osLow.y() > osHigh.y()

                    projection.x(projection.x()-1);
                    projection.y(projection.y()+1);
                }
            }
            layerPoints[fixst].push_back(projection);

            OrderedSegment nos1(osLow, projection);
            OrderedSegment nos2(projection, osHigh);
            toFix.push(std::pair<OrderedSegment, SignalType>(nos1, fixst));
            toFix.push(std::pair<OrderedSegment, SignalType>(nos2, fixst));
        }else{
            layerSegments[fixst].push_back(fixos);
        }
    }

    fixRepeatedPoints(layerPoints);
    fixRepeatedSegments(layerSegments);
}

void VoronoiPDNGen::generateVoronoiDiagram(const std::unordered_map<SignalType, std::vector<Cord>> &layerPoints, std::unordered_map<Cord, std::vector<FCord>> &voronoiCells){
    
    std::vector<Cord> voronoiCentres;
    std::vector<SignalType> voronoiCentreSignalTypes;
    std::vector<std::vector<FCord>> allWindings;
    for(std::unordered_map<SignalType, std::vector<Cord>>::const_iterator cit = layerPoints.begin(); cit != layerPoints.end(); ++cit){
        for(const Cord &c : cit->second){
            voronoiCentres.push_back(c);
            voronoiCentreSignalTypes.push_back(cit->first);
        }
    }

    using namespace geos::geom;
    using geos::triangulate::VoronoiDiagramBuilder;

    // Input coordinates
    std::vector<Coordinate> inputSites;
    for(int i = 0; i < voronoiCentres.size(); ++i){
        inputSites.push_back(Coordinate(voronoiCentres[i].x(), voronoiCentres[i].y()));
    }

    const GeometryFactory* factory = GeometryFactory::getDefaultInstance();
    auto coordSeq = std::make_unique<CoordinateSequence>();
    for (const Coordinate& c : inputSites) {
        coordSeq->add(c);
    }

    // Create MultiPoint from CoordinateSequence
    std::unique_ptr<Geometry> multipoint = factory->createMultiPoint(*coordSeq);

    // Bounding box
    Envelope clipEnv(0, (getPinWidth()-1), 0, (getPinHeight()-1));
    std::unique_ptr<Geometry> canvas = factory->toGeometry(&clipEnv);

    // Build Voronoi
    VoronoiDiagramBuilder builder;
    builder.setSites(*multipoint);
    builder.setClipEnvelope(&clipEnv);
    std::unique_ptr<Geometry> diagram = builder.getDiagram(*factory);

    // Extract and display each clipped Voronoi cell
    for (std::size_t i = 0; i < diagram->getNumGeometries(); ++i) {
        const Geometry* cell = diagram->getGeometryN(i);

        // Clip to bounding box
        std::unique_ptr<Geometry> clipped = cell->intersection(canvas.get());
        if (clipped->isEmpty()) continue;

        const Polygon* poly = dynamic_cast<const Polygon*>(clipped.get());
        if (!poly) continue;

        // Get the site (input point) this cell corresponds to
        const Geometry* siteGeom = multipoint->getGeometryN(i);
        const CoordinateXY* coordPtr = siteGeom->getCoordinate();
        if(!coordPtr) continue;


        // Extract exterior ring points into a vector
        std::unique_ptr<CoordinateSequence> ring = poly->getExteriorRing()->getCoordinates();
        std::vector<FCord> windingVector;
        for (std::size_t j = 0; j < ring->getSize(); ++j) {
            Coordinate tmpc = ring->getAt(j);
            windingVector.push_back(FCord(tmpc.x, tmpc.y));
        }


        allWindings.push_back(windingVector);
    }

    // start linking voronoi cells to its centre point

    for(int i = 0; i < allWindings.size(); ++i){
        std::vector<FCord> winding = allWindings[i];
        FPGMPolygon poly;
        for(const FCord &fc : winding){
            boost::geometry::append(poly, FPGMPoint(fc.x(), fc.y()));
        }
        boost::geometry::correct(poly);
        bool foundcentre = false;
        for(const Cord &c :voronoiCentres){
            FPGMPoint inside(flen_t(c.x()), flen_t(c.y()));
            if(boost::geometry::within(inside, poly)){
                assert(!foundcentre);
                foundcentre = true;
                voronoiCells[c] = winding;
            }
        }
        assert(foundcentre);
        // if(!foundcentre) std::cout << "No centre found for !" << i << std::endl;
    }
}

void VoronoiPDNGen::mergeVoronoiCells(std::unordered_map<SignalType, std::vector<Cord>> &layerPoints, std::unordered_map<Cord, std::vector<FCord>> &voronoiCellMap, std::unordered_map<SignalType, FPGMMultiPolygon> &multiPolygonMap){

    for(std::unordered_map<SignalType, std::vector<Cord>>::const_iterator cordit = layerPoints.begin(); cordit != layerPoints.end(); ++cordit){
        SignalType rst = cordit->first;
        FPGMMultiPolygon merged;

        for(const Cord &voronoiMidCords : cordit->second){
            std::unordered_map<Cord, std::vector<FCord>>::iterator vcmit = voronoiCellMap.find(voronoiMidCords);
            if(vcmit == voronoiCellMap.end()) continue;

            FPGMPolygon voronoiPoly;
            for(const FCord &fc : vcmit->second){
                boost::geometry::append(voronoiPoly, FPGMPoint(fc.x(), fc.y()));
            }
            boost::geometry::correct(voronoiPoly);
            FPGMMultiPolygon voronoiMPoly;
            voronoiMPoly.push_back(voronoiPoly);
            
            FPGMMultiPolygon tmp;
            boost::geometry::union_(merged, voronoiMPoly, tmp);
            merged = std::move(tmp);
        }
        
        multiPolygonMap[rst]= merged;
    }
}

void VoronoiPDNGen::exportToCanvas(std::vector<std::vector<SignalType>> &canvas, std::unordered_map<SignalType, FPGMMultiPolygon> &signalPolygon, bool overlayEmtpyGrids){
    // std::cout << "Export: " << std::endl;
    // std::cout << this->nodeWidth << " " << this->nodeHeight << std::endl;
    // std::cout << this->canvasWidth << " " << this->canvasHeight << std::endl;
    std::vector<SignalType> allSigType;
    std::vector<FPGMMultiPolygon> allSigPolygon;
    for(std::unordered_map<SignalType, FPGMMultiPolygon>::const_iterator cit = signalPolygon.begin(); cit != signalPolygon.end(); ++cit){
        allSigType.push_back(cit->first);
        allSigPolygon.push_back(FPGMMultiPolygon(cit->second));
    }

    for(int canvasJ = 0; canvasJ < getGridHeight(); ++canvasJ){
        for(int canvasI = 0; canvasI < getGridWidth(); ++canvasI){
            
            if(overlayEmtpyGrids){
                if(canvas[canvasJ][canvasI] != SignalType::EMPTY) continue;
            }

            std::vector<farea_t> signalOverlapArea(allSigType.size(), 0);
            std::vector<FPGMMultiPolygon> polygons = allSigPolygon;
            FPGMPolygon unitBox;
            boost::geometry::append(unitBox, FPGMPoint(canvasI, canvasJ));
            boost::geometry::append(unitBox, FPGMPoint(canvasI+1, canvasJ));
            boost::geometry::append(unitBox, FPGMPoint(canvasI+1, canvasJ+1));
            boost::geometry::append(unitBox, FPGMPoint(canvasI, canvasJ+1));
            boost::geometry::append(unitBox, FPGMPoint(canvasI, canvasJ));
            boost::geometry::correct(unitBox);
            // FPGMMultiPolygon MunitBox;


            std::vector<FPGMMultiPolygon> unitBoxes(allSigType.size(), FPGMMultiPolygon({unitBox}));
            std::vector<FPGMMultiPolygon> resultPolygon(allSigType.size(), FPGMMultiPolygon());
            for(int i = 0; i < allSigType.size(); ++i){
                boost::geometry::intersection(polygons[i], unitBoxes[i], resultPolygon[i]);
                signalOverlapArea[i] = boost::geometry::area(resultPolygon[i]);
                // std::cout << i << " " << signalOverlapArea[i];
            }

            int largestIdx = 0;
            farea_t largestVal = FAREA_T_MIN;
            for(int i = 0; i < allSigType.size(); ++i){
                if(signalOverlapArea[i] > largestVal){
                    largestVal = signalOverlapArea[i];
                    largestIdx = i;
                }
            }

            canvas[canvasJ][canvasI] = allSigType[largestIdx];
        }
    }
}

void VoronoiPDNGen::obstacleAwareLegalisation(int layerIdx){
    std::unordered_set<SignalType> ignoreSignalType = {SignalType::OBSTACLE, SignalType::SIGNAL, SignalType::GROUND, SignalType::EMPTY};
    std::unordered_map<SignalType, DoughnutPolygonSet> dpSetMap = collectDoughnutPolygons(metalLayers[layerIdx].canvas);

    for(std::unordered_map<SignalType, DoughnutPolygonSet>::iterator it = dpSetMap.begin(); it != dpSetMap.end(); ++it){
        SignalType st = it->first;
        if(ignoreSignalType.count(st)) continue;
        if(dps::getShapesCount(it->second) <= 1) continue;

        int splitSize = dps::getShapesCount(it->second);
        for(int sidx = 0; sidx < splitSize; ++sidx){
            DoughnutPolygon fragdp = it->second.at(sidx);
            std::vector<Cord> containGrids = dp::getContainedGrids(fragdp);
            bool mustKeep = false;
            for(const Cord &c : containGrids){
                if(this->preplaceOfLayers[layerIdx][c.y()][c.x()] != SignalType::EMPTY){
                    mustKeep = true;
                    break;
                }
            }
            if(!mustKeep){
                // std::cout << "Remove one fragment" << std::endl;
                // empty out the part
                for(const Cord &c : containGrids){
                    this->metalLayers[layerIdx].setCanvas(c, SignalType::EMPTY);
                }
            }
        }
    }

}

void VoronoiPDNGen::floatingPlaneReconnection(int layerIdx){
    std::unordered_set<SignalType> ignoreSignalType = {SignalType::OBSTACLE, SignalType::SIGNAL, SignalType::GROUND, SignalType::EMPTY};
    std::unordered_map<SignalType, DoughnutPolygonSet> dpSetMap = collectDoughnutPolygons(metalLayers[layerIdx].canvas);
    if(dpSetMap.count(SignalType::EMPTY) == 0) return;

    DoughnutPolygonSet targetdps = dpSetMap.at(SignalType::EMPTY);
    for(int fragidx = 0; fragidx < dps::getShapesCount(targetdps); ++fragidx){
        DoughnutPolygon frag = targetdps.at(fragidx);
        std::unordered_map<SignalType, int> poll;
        std::vector<Cord> winding;
        for(auto it = frag.begin(); it != frag.end(); ++it){
            winding.push_back(Cord(*it));
        }
        winding.push_back(winding[0]);
        for(int widx = 0; widx < (winding.size() - 1); ++widx){
            Line longLine(winding.at(widx), winding.at(widx + 1));
            if(longLine.getOrient() == eOrientation2D::HORIZONTAL){
                len_t sy = longLine.getLow().y();
                for(len_t vx = longLine.getLow().x(); vx < longLine.getHigh().x(); ++vx){
                    Cord upCord(vx, sy);
                    Cord downCord(vx, sy-1);

                    bool upCordValid = ((vx >= 0) && (vx < m_gridWidth) && (sy >= 0) && (sy < m_gridHeight));
                    bool downCordValid =  ((vx >= 0) && (vx < m_gridWidth) && ((sy-1) >= 0) && ((sy-1) < m_gridHeight));

                    if(!upCordValid || !downCordValid) continue;

                    SignalType uptype = metalLayers[layerIdx].canvas[upCord.y()][upCord.x()];
                    SignalType downtype = metalLayers[layerIdx].canvas[downCord.y()][downCord.x()];

                    poll[uptype] = (poll.count(uptype) == 0)? 1 : (poll[uptype] + 1);
                    poll[downtype] = (poll.count(downtype) == 0)? 1 : (poll[downtype] + 1);
                }

            }else{ // eOrientation2D::VERTICAL
                len_t sx = longLine.getLow().x();
                for(len_t vy = longLine.getLow().y(); vy < longLine.getHigh().y(); ++vy){
                    Cord rightCord(sx, vy);
                    Cord leftCord(sx-1, vy);

                    bool rightCordValid = ((sx >= 0) && (sx < m_gridWidth) && (vy >= 0) && (vy < m_gridHeight));
                    bool leftCordValid = (((sx-1) >= 0) && ((sx-1) < m_gridWidth) && (vy >= 0) && (vy < m_gridHeight));

                    if(!rightCordValid || ! leftCordValid) continue;

                    SignalType righttype = metalLayers[layerIdx].canvas[rightCord.y()][rightCord.x()];
                    SignalType lefttype = metalLayers[layerIdx].canvas[leftCord.y()][leftCord.x()];

                    poll[righttype] = (poll.count(righttype) == 0)? 1 : (poll[righttype] + 1) ;
                    poll[lefttype] = (poll.count(lefttype) == 0)? 1 : (poll[lefttype] + 1);

                }

            }
        }
        
        // start painting to new color
        for(const SignalType st : ignoreSignalType){
            poll.erase(st);
        }

        if(poll.empty()) continue;

        int maxVote = 0;
        SignalType maxVoteSig = SignalType::EMPTY;

        for(std::unordered_map<SignalType, int>::const_iterator cit = poll.begin(); cit != poll.end(); ++cit){
            if(cit->second > maxVote){
                maxVote = cit->second;
                maxVoteSig = cit->first;
            }
        }

        std::vector<Cord> paintTarget = dp::getContainedGrids(frag);
        for(const Cord & c : paintTarget){
            metalLayers[layerIdx].setCanvas(c, maxVoteSig);
        }
    }

}

void VoronoiPDNGen::enhanceCrossLayerPI(){
    
    enum class PowerPlaneType{
        PREPLACED, STACKED, HARD, SOFT 
    };
    
    // initialise marking to PowerPlaneType::SOFT
    std::vector<std::vector<std::vector<PowerPlaneType>>> marking(
        m_metalLayerCount, 
        std::vector<std::vector<PowerPlaneType>>(m_gridHeight, std::vector<PowerPlaneType>(m_gridWidth, PowerPlaneType::SOFT))
    );

    for(int mLayer = 0; mLayer < m_metalLayerCount; ++mLayer){
        for(int j = 0; j < m_gridHeight; ++j){
            for(int i = 0; i < m_gridWidth; ++i){

                if(marking[mLayer][j][i] != PowerPlaneType::SOFT) continue;
                if(this->preplaceOfLayers[mLayer][j][i] != SignalType::EMPTY){
                    marking[mLayer][j][i] = PowerPlaneType::PREPLACED;
                    continue;
                }
                // check if could mark as STACKED or HARD
                SignalType targetSt = metalLayers[mLayer].canvas[j][i];
                
                int upSearchLayer = mLayer;
                int downSearchLayer = mLayer;
                while(true){
                    // std::cout << "ups: " << upSearchLayer << std::endl;
                    if(metalLayers[upSearchLayer].canvas[j][i] != targetSt){
                        upSearchLayer++;
                        break;
                    }
                    if(upSearchLayer == 0) break;
                    else upSearchLayer--;
                }

                while(true){
                    // std::cout << "dps: " << upSearchLayer << std::endl;

                    if(metalLayers[downSearchLayer].canvas[j][i] != targetSt){
                        downSearchLayer--;
                        break;
                    }
                    if(downSearchLayer == (m_metalLayerCount-1)) break;
                    else downSearchLayer++;
                }

                assert(upSearchLayer >= 0);
                assert(upSearchLayer < m_metalLayerCount);

                assert(downSearchLayer >= 0);
                assert(downSearchLayer < m_metalLayerCount);

                assert(downSearchLayer >= upSearchLayer);

                int overlapLayerCount = (downSearchLayer - upSearchLayer + 1);
                
                if(overlapLayerCount == 1) marking[mLayer][j][i] = PowerPlaneType::SOFT;
                else if(overlapLayerCount == 2) marking[mLayer][j][i] = PowerPlaneType::HARD;
                else marking[mLayer][j][i] = PowerPlaneType::STACKED;

            }
        }
    }

    

    // start trading
    using namespace boost::polygon::operators;
    for(int upLayerIdx = 0; upLayerIdx < (m_metalLayerCount-1); ++upLayerIdx){
        int downLayerIdx = upLayerIdx + 1;
        std::cout << "Trading Layer = " << upLayerIdx << " & " << downLayerIdx  << std::endl;
        // calculate the signal area as a trading reference
        std::unordered_map<SignalType, int> upOccurence = countSignalTypeOccurrences(metalLayers[upLayerIdx].canvas);
        std::unordered_map<SignalType, int> downOccurence = countSignalTypeOccurrences(metalLayers[downLayerIdx].canvas);

        std::unordered_map<SignalType, int> totalOccurrence = upOccurence;
        for (std::unordered_map<SignalType, int>::const_iterator cit = downOccurence.begin(); cit != downOccurence.end(); ++cit) {
            const SignalType& sig = cit->first;
            int count = cit->second;

            // Add count to totalOccurrence (insert with default 0 if not present)
            totalOccurrence[sig] += count;
        }
        std::unordered_map<SignalType, DoughnutPolygonSet> upLayerDPS = collectDoughnutPolygons(metalLayers[upLayerIdx].canvas);
        std::unordered_map<SignalType, DoughnutPolygonSet> downLayerDPS = collectDoughnutPolygons(metalLayers[downLayerIdx].canvas);

        for(int j = 0; j < m_gridHeight; ++j){
            // std::cout << "Tradinng j = " << j << std::endl;
            for(int i = 0; i < m_gridWidth; ++i){
                SignalType upSigType = metalLayers[upLayerIdx].canvas[j][i];
                SignalType downSigType =  metalLayers[downLayerIdx].canvas[j][i];
                if(upSigType == downSigType) continue;

                
                PowerPlaneType upPPType = marking[upLayerIdx][j][i];
                PowerPlaneType downPPType = marking[downLayerIdx][j][i];
                bool upSoftDownTradable = (upPPType == PowerPlaneType::SOFT) && ((downPPType == PowerPlaneType::SOFT)||(downPPType == PowerPlaneType::STACKED));
                bool downSoftUpTradable = (downPPType == PowerPlaneType::SOFT) && ((upPPType == PowerPlaneType::SOFT)||(upPPType == PowerPlaneType::STACKED));
                if((!upSoftDownTradable) && (!downSoftUpTradable)) continue;

                DoughnutPolygonSet dpsUpLayerUpSigType = (upLayerDPS.count(upSigType))? upLayerDPS[upSigType] : DoughnutPolygonSet();
                DoughnutPolygonSet dpsDownLayerDownSigType = (downLayerDPS.count(downSigType))? downLayerDPS[downSigType] : DoughnutPolygonSet();
                Rectangle rect(i, j, i+1, j+1);
                
                int origSize = dps::getShapesCount(dpsUpLayerUpSigType);
                dpsUpLayerUpSigType -= rect;
                bool canMakeUpDown = (dps::getShapesCount(dpsUpLayerUpSigType) <= origSize);
                
                origSize = dps::getShapesCount(dpsDownLayerDownSigType);
                dpsDownLayerDownSigType -= rect;
                bool canMakeDownUp = (dps::getShapesCount(dpsDownLayerDownSigType) <= origSize);
                if((!canMakeUpDown) && (!canMakeDownUp)) continue;
                

                // start trading
                if((totalOccurrence[upSigType] >= totalOccurrence[downSigType]) && canMakeUpDown){
                    --totalOccurrence[upSigType];
                    metalLayers[upLayerIdx].setCanvas(j, i, downSigType);
                    upLayerDPS[upSigType] = dpsUpLayerUpSigType;
                    upLayerDPS[downSigType] += rect;
                }else{
                    --totalOccurrence[downSigType];
                    metalLayers[downLayerIdx].setCanvas(j, i, upSigType);
                    downLayerDPS[downSigType] = dpsDownLayerDownSigType;
                    downLayerDPS[upSigType] += rect;
                }

                // refresh the markings of the two place
                marking[upLayerIdx][j][i] = marking[downLayerIdx][j][i] = PowerPlaneType::SOFT;


                // check if could mark as STACKED or HARD
                SignalType targetSt = metalLayers[upLayerIdx].canvas[j][i];
                // search up
                for(int mLayer = upLayerIdx; mLayer <= downLayerIdx; ++mLayer){
                    int upSearchLayer = mLayer;
                    int downSearchLayer = mLayer;
                    while(true){
                        if(metalLayers[upSearchLayer].canvas[j][i] != targetSt){
                            upSearchLayer++;
                            break;
                        }
                        if(upSearchLayer == 0) break;
                        else upSearchLayer--;
                    }

                    while(true){
                        if(metalLayers[downSearchLayer].canvas[j][i] != targetSt){
                            downSearchLayer--;
                            break;
                        }
                        if(downSearchLayer == (m_metalLayerCount-1)) break;
                        else downSearchLayer++;
                    }

                    assert(upSearchLayer >= 0);
                    assert(upSearchLayer < m_metalLayerCount);
                    assert(downSearchLayer >= 0);
                    assert(downSearchLayer < m_metalLayerCount);
                    assert(downSearchLayer >= upSearchLayer);

                    int overlapLayerCount = (downSearchLayer - upSearchLayer + 1);

                    if(overlapLayerCount == 1) marking[mLayer][j][i] = PowerPlaneType::SOFT;
                    else if(overlapLayerCount == 2) marking[mLayer][j][i] = PowerPlaneType::HARD;
                    else marking[mLayer][j][i] = PowerPlaneType::STACKED;
                }

            }
        }
    }
}
    // metallayers[1].canvas[]
void VoronoiPDNGen::handCraft(){
    // fill enclosed region
    
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
    


    for(int layer = 0; layer <= 1; ++layer){
        metalLayers[layer].canvas[74][37] = SignalType::POWER_2;
        metalLayers[layer].canvas[75][37] = SignalType::POWER_2;
        metalLayers[layer].canvas[74][38] = SignalType::POWER_2;
        metalLayers[layer].canvas[75][38] = SignalType::POWER_2;
    }

    for(int layer = 0; layer <= 1; ++layer){
        metalLayers[layer].canvas[74][55] = SignalType::POWER_2;
        metalLayers[layer].canvas[75][55] = SignalType::POWER_2;
        metalLayers[layer].canvas[74][56] = SignalType::POWER_2;
        metalLayers[layer].canvas[75][56] = SignalType::POWER_2;
    }


    for(int layer = 0; layer <= 1; ++layer){
        metalLayers[layer].canvas[74][55] = SignalType::POWER_2;
        metalLayers[layer].canvas[75][55] = SignalType::POWER_2;
        metalLayers[layer].canvas[74][56] = SignalType::POWER_2;
        metalLayers[layer].canvas[75][56] = SignalType::POWER_2;
    }



    for(int j = 68; j <= 69; ++j){
        for(int i = 40; i <= 53; ++i){
            metalLayers[0].canvas[j][i] = SignalType::POWER_2;
            metalLayers[1].canvas[j][i] = SignalType::POWER_2;
        }
    }
    for(int i = 31; i <= 40; ++i){
        metalLayers[1].canvas[65][i] = SignalType::POWER_2;
    }


    for(int layer = 0; layer <= 1; ++layer){
        metalLayers[layer].canvas[63][85] = SignalType::POWER_4;
        metalLayers[layer].canvas[62][85] = SignalType::POWER_4;
        metalLayers[layer].canvas[63][86] = SignalType::POWER_4;
        metalLayers[layer].canvas[62][86] = SignalType::POWER_4;
        for(int j = 56; j <= 57; ++j){
            for(int i = 78; i <= 92; ++i){
                metalLayers[layer].canvas[j][i] = SignalType::POWER_4;

            }
        }
    }
    
    for(int layer = 1; layer <= 2; ++layer){
        for(int j = 62; j <= 64; ++j){
            for(int i = 42; i <= 49; ++i){
                metalLayers[layer].canvas[j][i] = SignalType::POWER_2;
            }
        }
    }

    for(int layer = 1; layer <= 2; ++layer){
        for(int j = 62; j <= 64; ++j){
            for(int i = 77; i <= 79; ++i){
                metalLayers[layer].canvas[j][i] = SignalType::POWER_4;
            }
        }
    }

    for(int layer = 1; layer <= 2; ++layer){
        for(int j = 62; j <= 64; ++j){
            for(int i = 77; i <= 79; ++i){
                metalLayers[layer].canvas[j][i] = SignalType::POWER_4;
            }
            for(int i = 83; i <= 85; ++i){
                metalLayers[layer].canvas[j][i] = SignalType::POWER_4;
            }
        }
    }

    for(int j = 65; j <= 67; ++j){
        for(int i = 40; i <= 49; ++i){
            metalLayers[1].canvas[j][i] = SignalType::POWER_2;
        }
    }
    for(int j = 70; j <= 75; ++j){
        for(int i = 40; i <= 41; ++i){
            metalLayers[1].canvas[j][i] = SignalType::POWER_2;
        }
    }
    for(int j = 74; j <= 75; ++j){
        for(int i = 36; i <= 39; ++i){
            metalLayers[1].canvas[j][i] = SignalType::POWER_2;
        }
        for(int i = 52; i <= 54; ++i){
            metalLayers[1].canvas[j][i] = SignalType::POWER_2;
        }
    }
    for(int j = 70; j <= 73; ++j){
        for(int i = 52; i <= 53; ++i){
            metalLayers[1].canvas[j][i] = SignalType::POWER_2;
        }
    }
    for(int j = 70; j <= 76; ++j){
        for(int i = 46; i <= 47; ++i){
            metalLayers[1].canvas[j][i] = SignalType::POWER_2;
        }
    }

    for(int j = 59; j <= 61; ++j){
        for(int i = 26; i <= 49; ++i){
            metalLayers[2].canvas[j][i] = SignalType::POWER_2;
        }
    }

    for(int j = 44; j <= 61; ++j){
        for(int i = 47; i <= 49; ++i){
            metalLayers[2].canvas[j][i] = SignalType::POWER_2;
        }
    }


    for(int j = 52; j <= 55; ++j){
        for(int i = 91; i <= 92; ++i){
            metalLayers[1].canvas[j][i] = SignalType::POWER_4;
        }
    }


    for(int j = 58; j <= 61; ++j){
        for(int i = 78; i <= 86; ++i){
            metalLayers[1].canvas[j][i] = SignalType::POWER_4;
        }
    }
    for(int j = 65; j <= 68; ++j){
        for(int i = 84; i <= 85; ++i){
            metalLayers[1].canvas[j][i] = SignalType::POWER_4;
        }
    }
    for(int j = 50; j <= 54; ++j){
        for(int i = 70; i <= 71; ++i){
            metalLayers[1].canvas[j][i] = SignalType::POWER_4;
        }
    }
    for(int j = 53; j <= 55; ++j){
        for(int i = 72; i <= 79; ++i){
            metalLayers[1].canvas[j][i] = SignalType::POWER_4;
        }
    }


    for(int j = 53; j <= 61; ++j){
        for(int i = 77; i <= 79; ++i){
            metalLayers[2].canvas[j][i] = SignalType::POWER_4;
        }
    }
    for(int j = 53; j <= 55; ++j){
        for(int i = 68; i <= 76; ++i){
            metalLayers[2].canvas[j][i] = SignalType::POWER_4;
        }
    }
    for(int j = 56; j <= 61; ++j){
        for(int i = 83; i <= 85; ++i){
            metalLayers[2].canvas[j][i] = SignalType::POWER_4;
        }
    }
    for(int j = 59; j <= 73; ++j){
        for(int i = 77; i <= 85; ++i){
            metalLayers[2].canvas[j][i] = SignalType::POWER_4;
        }
    }

    for(int j = 24; j <= 96; ++j){
        for(int i = 20; i <= 100; ++i){
            if(metalLayers[0].canvas[j][i] == SignalType::EMPTY){
                metalLayers[0].canvas[j][i] = SignalType::POWER_3;
            }
        }
    }
    for(int j = 0; j < m_gridHeight; ++j){
        for(int i = 0; i < m_gridWidth; ++i){
            if(metalLayers[0].canvas[j][i] == SignalType::EMPTY){
                metalLayers[0].canvas[j][i] = SignalType::POWER_1;
            } 
        }
    }

    for(int j = 24; j <= 96; ++j){
        for(int i = 20; i <= 100; ++i){
            if(metalLayers[1].canvas[j][i] == SignalType::EMPTY){
                metalLayers[1].canvas[j][i] = SignalType::POWER_3;
            }
        }
    }

    for(int j = 20; j <= 23; ++j){
        for(int i = 34; i <= 35; ++i){
            metalLayers[1].canvas[j][i] = SignalType::POWER_3;
        }
    }
    for(int j = 22; j <= 23; ++j){
        for(int i = 46; i <= 47; ++i){
            metalLayers[1].canvas[j][i] = SignalType::POWER_3;

        }
    }
    for(int j = 19; j <= 23; ++j){
        for(int i = 91; i <= 92; ++i){
            metalLayers[1].canvas[j][i] = SignalType::POWER_3;

        }
    }
    for(int j = 0; j < m_gridHeight; ++j){
        for(int i = 0; i < m_gridWidth; ++i){
            if(metalLayers[1].canvas[j][i] == SignalType::EMPTY){
                metalLayers[1].canvas[j][i] = SignalType::POWER_1;
            } 
        }
    }

    for(int j = 24; j <= 96; ++j){
        for(int i = 20; i <= 100; ++i){
            if(metalLayers[2].canvas[j][i] == SignalType::EMPTY){
                metalLayers[2].canvas[j][i] = SignalType::POWER_3;
            }
        }
    }

    for(int j = 29; j <= 37; ++j){
        for(int i = 17; i <= 19; ++i){
            metalLayers[2].canvas[j][i] = SignalType::POWER_3;
        }
    }
    for(int j = 89; j <= 96; ++j){
        for(int i = 17; i <= 19; ++i){
            metalLayers[2].canvas[j][i] = SignalType::POWER_3;
        }
    }

    for(int j = 83; j <= 85; ++j){
        for(int i = 101; i <= 118; ++i){
            metalLayers[2].canvas[j][i] = SignalType::POWER_3;
        }
    }
    for(int j = 29; j <= 31; ++j){
        for(int i = 101; i <= 118; ++i){
            metalLayers[2].canvas[j][i] = SignalType::POWER_3;
        }
    }
    for(int j = 0; j < m_gridHeight; ++j){
        for(int i = 0; i < m_gridWidth; ++i){
            if(metalLayers[2].canvas[j][i] == SignalType::EMPTY){
                metalLayers[2].canvas[j][i] = SignalType::POWER_1;
            } 
        }
    }


    // for(int j = 0; j < m_gridHeight; ++j){
    //     for(int i = 0; i < m_gridWidth; ++i){
    //         if(metalLayers[1].canvas[j][i] != SignalType::EMPTY){
    //             metalLayers[0].canvas[j][i] = metalLayers[1].canvas[j][i];
    //         }
    //     }
    // }

    // for(int j = 0; j < m_gridHeight; ++j){
    //     for(int i = 0; i < m_gridWidth; ++i){
    //         if(metalLayers[1].canvas[j][i] != SignalType::EMPTY){
    //             metalLayers[2].canvas[j][i] = metalLayers[1].canvas[j][i];
    //         }
    //     }
    // }
}