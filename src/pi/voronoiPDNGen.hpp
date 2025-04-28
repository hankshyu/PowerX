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
#include "segment.hpp"
#include "orderedSegment.hpp"
#include "powerGrid.hpp"
#include "signalType.hpp"

struct VEdge{
    SignalType sig;
    Cord c1, c2;

};

struct VNode{
    SignalType sig;
    VEdge *up;
    VEdge *down;
    VEdge *left;
    VEdge *right;

};

class VoronoiPDNGen: public PowerGrid{
private:
    const char *WIRELENGTH_VECTOR_FILE = "./lib/flute/POWV9.dat";
    const char *ROUTING_TREE_FILE = "./lib/flute/PORT9.dat";

    void fixRepeatedPoints(std::unordered_map<SignalType, std::vector<Cord>> &layerPoints);
public:

    // Boost::geometry definitions

    typedef boost::geometry::model::d2::point_xy<flen_t> FPGMPoint;
    typedef boost::geometry::model::polygon<FPGMPoint> FPGMPolygon;
    typedef boost::geometry::model::multi_polygon<FPGMPolygon> FPGMMultiPolygon;


    int nodeHeight, nodeWidth;
    std::unordered_map<SignalType, std::vector<Cord>> m5Points;
    std::unordered_map<SignalType, std::vector<OrderedSegment>> m5Segments;
    std::unordered_map<Cord, std::vector<FCord>> m5VoronoiCells;
    std::unordered_map<SignalType, FPGMMultiPolygon> m5MultiPolygons;

    std::unordered_map<SignalType, std::vector<Cord>> m7Points;
    std::unordered_map<SignalType, std::vector<OrderedSegment>> m7Segments;
    std::unordered_map<Cord, std::vector<FCord>> m7VoronoiCells;
    std::unordered_map<SignalType, FPGMMultiPolygon> m7MultiPolygons;

    std::vector<std::vector<VNode *>> m5NodeArr;
    std::vector<std::vector<VNode *>> m7NodeArr;

    VoronoiPDNGen(const std::string &fileName);

    void runSATRouting();
    void runILPRouting();

    void initPoints(const std::unordered_set<SignalType> &m5IgnoreSigs, const std::unordered_set<SignalType> &m7IgnoreSigs);
    void connectLayers();

    void runFLUTERouting(std::unordered_map<SignalType, std::vector<Cord>> &layerPoints, std::unordered_map<SignalType, std::vector<OrderedSegment>> &layerSegments);
    void runMSTRouting(std::unordered_map<SignalType, std::vector<Cord>> &layerPoints, std::unordered_map<SignalType, std::vector<OrderedSegment>> &layerSegments);
    void ripAndReroute(std::unordered_map<SignalType, std::vector<Cord>> &layerPoints, std::unordered_map<SignalType, std::vector<OrderedSegment>> &layerSegments);
    
    void generateInitialPowerPlane(std::unordered_map<SignalType, std::vector<Cord>> &layerPoints, std::unordered_map<SignalType, std::vector<OrderedSegment>> &layerSegments);
    void generateVoronoiDiagram(const std::unordered_map<SignalType, std::vector<Cord>> &layerPoints, std::unordered_map<Cord, std::vector<FCord>> &voronoiCells);
    void mergeVoronoiCells(std::unordered_map<SignalType, std::vector<Cord>> &layerPoints, std::unordered_map<Cord, std::vector<FCord>> &voronoiCellMap, std::unordered_map<SignalType, FPGMMultiPolygon> &multiPolygonMap);
    void enhanceCrossLayerPI(std::unordered_map<SignalType, FPGMMultiPolygon> &m5PolygonMap, std::unordered_map<SignalType, FPGMMultiPolygon> &PolygonMap);
    
    void exportToCanvas(std::vector<std::vector<SignalType>> &canvas, std::unordered_map<SignalType, FPGMMultiPolygon> &signalPolygon);
    void fixIsolatedCells(std::vector<std::vector<SignalType>> &canvas, const std::unordered_set<SignalType> &obstacles);
    
    
    friend bool visualiseM5VoronoiPointsSegments(const VoronoiPDNGen &vpg, const std::string &filePath);
    friend bool visualiseM7VoronoiPointsSegments(const VoronoiPDNGen &vpg, const std::string &filePath);

    friend bool visualiseM5VoronoiGraph(const VoronoiPDNGen &vpg, const std::string &filePath);
    friend bool visualiseM7VoronoiGraph(const VoronoiPDNGen &vpg, const std::string &filePath);

    friend bool visualiseM5VoronoiPolygons(const VoronoiPDNGen &vpg, const std::string &filePath);
    friend bool visualiseM7VoronoiPolygons(const VoronoiPDNGen &vpg, const std::string &filePath);

};

#endif // __VORONOIPDNGEN_H__