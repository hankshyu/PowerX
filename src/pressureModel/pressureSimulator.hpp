//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        06/04/2025 16:58:37
//  Module Name:        pressureSimulator.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
/* Overall Algorithm 

// step 1 & 2 in initialize()
1. Cluster pins & preplaced grids to form initial polygon
2. Initialize bin-based spatial indexing structure

// step 3, massive parallel, 
3. Loop until growth converge or Iteration_max
	For each Polygon P: 
		For each border point P_i in P
            Calculate the force applied to P_i
                a. Pressure Force
                b. Bending Resistance (Roundness Enforcer)
                c. Attraction / Repulsion by nearby polygons
                d. Force caused by nearby vias
                e. Borders & hard vias
            Move P_i using force-directed relaxation

	Overlap Resolving / Merging 
	Remeshing, add extra points to ensure |𝑃_(𝑖+1)  −𝑃_𝑖 |< ε 
	Recalculate pressure for all polygon, 
    update via status, diffusion affect between layers
	Update Spatial indexing structure
*/

//////////////////////////////////////////////////////////////////////////////////
//  Description:        The top module of the pressure growing system
//

//////////////////////////////////////////////////////////////////////////////////
//  Revision:
/////////////////////////////////////////////////////////////////////////////////

#ifndef __PRESSURE_SIMULATOR_H__
#define __PRESSURE_SIMULATOR_H__

// Dependencies
// 1. C++ STL:
#include <vector>
#include <unordered_map>
#include <memory>

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

#include "pointBinSystem.hpp"
#include "rectangleBinSystem.hpp"

class PressureSimulator: public PowerDistributionNetwork{
private:
    // simulaittion system contour (playground size of the system)
    int m_canvasWidth;
    int m_canvasHeight;

    std::vector<std::vector<SoftBody *>> m_OwnerSoftBodies;
    std::vector<std::vector<ViaBody *>> m_OwnerViasBodies;

    struct SimHyperParams {

        // smallest distance between contour points
        flen_t pointsMaxDelta = 0.5;
        flen_t initialMargin = 0.5;
        flen_t pointBinSize = 2.0;
        flen_t rectangleBinSize = 5.0;

        int iterationMax = 10;

    } m_params;
    
public:
    // bin system of the bounding box of softBodies
    std::vector<RectangleBinSystem<flen_t, SoftBody>> softBodyRectangleBin;
    std::vector<PointBinSystem<flen_t, SoftBody>> softBodyPointBin;

    // bin system of the bounding box of vias
    std::vector<PointBinSystem<flen_t, ViaBody>> viaPointBins;

    // statistics
    std::vector<flen_t> avgViaDistance;

    double totalCurrentRequirement;
    std::unordered_map<SignalType, double> signalCurrentRequirements;

    PressureSimulator(const std::string &fileName);
    ~PressureSimulator();

    inline int getCanvasWidth() const {return m_canvasWidth;}
    inline int getCanvasHeight() const {return m_canvasHeight;}

    const std::vector<SoftBody *> &getSoftBodyOwner(int layer) const;
    const std::vector<ViaBody *> &getViaBodyOwner(int layer) const;

    
    void runAlgorithm();
    
    void initialise();
    void collectStatistics();
    void normalise();

    void updatePolygons();



    void relaxPolygons();
    void fixPolygons();


    friend bool visualiseSoftBodies(const PressureSimulator &ps, const std::vector<SoftBody *> softBodies, const std::string &filePath);
    friend bool visualiseSoftBodiesWithPin(const PressureSimulator &ps, const std::vector<SoftBody *> softBodies, const std::vector<ViaBody *> vias, const std::string &filePath);
    friend bool visualiseSoftBodiesWithPins(const PressureSimulator &ps, const std::vector<SoftBody *> softBodies, const std::vector<ViaBody *> upVias,  const std::vector<ViaBody *> downVias, const std::string &filePath);

};
#endif // __PRESSURE_SIMULATOR_H__