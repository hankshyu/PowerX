//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        04/06/2025 15:29:02
//  Module Name:        voronoiPDNGen.hpp
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

#ifndef __VORONOIPDNGEN_H__
#define __VORONOIPDNGEN_H__

// Dependencies
// 1. C++ STL:
#include <unordered_map>
#include <unordered_set>

// 2. Boost Library:
#include "boost/geometry.hpp"
#include "boost/geometry/geometries/point_xy.hpp"
#include "boost/geometry/geometries/polygon.hpp"

// 3. Texo Library:
#include "units.hpp"
#include "cord.hpp"
#include "orderedSegment.hpp"
#include "signalType.hpp"
#include "powerDistributionNetwork.hpp"


class VoronoiPDNGen: public PowerDistributionNetwork{
private:
    const char *WIRELENGTH_VECTOR_FILE = "./lib/flute/POWV9.dat";
    const char *ROUTING_TREE_FILE = "./lib/flute/PORT9.dat";

    void fixRepeatedPoints(std::unordered_map<SignalType, std::vector<Cord>> &layerPoints);
    void fixRepeatedSegments(std::unordered_map<SignalType, std::vector<OrderedSegment>> &layerSegments);

public:

    // Boost::geometry definitions
    typedef boost::geometry::model::d2::point_xy<flen_t> FPGMPoint;
    typedef boost::geometry::model::polygon<FPGMPoint> FPGMPolygon;
    typedef boost::geometry::model::multi_polygon<FPGMPolygon> FPGMMultiPolygon;

    std::vector<std::vector<std::vector<SignalType>>> preplaceOfLayers;
    std::vector<std::unordered_map<SignalType, std::vector<Cord>>> pointsOfLayers;
    std::vector<std::unordered_map<SignalType, std::vector<OrderedSegment>>> segmentsOfLayers;
    std::vector<std::unordered_map<Cord, std::vector<FCord>>> voronoiCellsOfLayers;
    std::vector<std::unordered_map<SignalType, FPGMMultiPolygon>> multiPolygonsOfLayers;

    VoronoiPDNGen(const std::string &fileName);
    
    
    /* Mark preplaced points to metal or via layer, insert pads to metal*/
    void markPreplacedAndInsertPads();

    /* Fill pointsOfLayers by points of interest: */ 
    // in uBump and c4 connecting layer by corresponding interconnect pins and ignore sigs
    // in other layers according to to preplaced signals (not obstacles)
    void initPointsAndSegments();

    void connectLayers(int upLayerIdx, int downLayerIdx);

    void runFLUTERouting(std::unordered_map<SignalType, std::vector<Cord>> &layerPoints, std::unordered_map<SignalType, std::vector<OrderedSegment>> &layerSegments);
    void runMSTRouting(std::unordered_map<SignalType, std::vector<Cord>> &layerPoints, std::unordered_map<SignalType, std::vector<OrderedSegment>> &layerSegments);
    
    void ripAndReroute(std::unordered_map<SignalType, std::vector<Cord>> &layerPoints, std::unordered_map<SignalType, std::vector<OrderedSegment>> &layerSegments);
    void generateInitialPowerPlanePoints(std::unordered_map<SignalType, std::vector<Cord>> &layerPoints, std::unordered_map<SignalType, std::vector<OrderedSegment>> &layerSegments);
   
    void generateVoronoiDiagram(const std::unordered_map<SignalType, std::vector<Cord>> &layerPoints, std::unordered_map<Cord, std::vector<FCord>> &voronoiCells);
    void mergeVoronoiCells(std::unordered_map<SignalType, std::vector<Cord>> &layerPoints, std::unordered_map<Cord, std::vector<FCord>> &voronoiCellMap, std::unordered_map<SignalType, FPGMMultiPolygon> &multiPolygonMap);
    
    void exportToCanvas(std::vector<std::vector<SignalType>> &canvas, std::unordered_map<SignalType, FPGMMultiPolygon> &signalPolygon, bool overlayEmtpyGrids = true);
    void obstacleAwareLegalisation(int layerIdx);
    void floatingPlaneReconnection(int layerIdx);
    
    void enhanceCrossLayerPI();


    /*
    void enhanceCrossLayerPI(std::unordered_map<SignalType, FPGMMultiPolygon> &m5PolygonMap, std::unordered_map<SignalType, FPGMMultiPolygon> &PolygonMap);    
    void fixIsolatedCells(std::vector<std::vector<SignalType>> &canvas, const std::unordered_set<SignalType> &obstacles);
    */
    
    friend bool visualisePointsSegments(const VoronoiPDNGen &vpg, const std::unordered_map<SignalType, std::vector<Cord>> &points, const std::unordered_map<SignalType, std::vector<OrderedSegment>> &segments, const std::string &filePath);
    friend bool visualiseVoronoiGraph(const VoronoiPDNGen &vpg, const std::unordered_map<SignalType, std::vector<Cord>> &points, const std::unordered_map<Cord, std::vector<FCord>> &cells, const std::string &filePath);
    friend bool visualiseMultiPolygons(const VoronoiPDNGen &vpg, const std::unordered_map<SignalType, FPGMMultiPolygon> &multiPolygons, const std::string &filePath);

};

#endif // __VORONOIPDNGEN_H__