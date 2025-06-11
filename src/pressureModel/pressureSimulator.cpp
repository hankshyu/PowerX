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