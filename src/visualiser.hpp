//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/14/2025 19:03:56
//  Module Name:        visualiser.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        All classes that writes out to a certain file is presented
//                      Use render program to render the output file for visualization purposes
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//
/////////////////////////////////////////////////////////////////////////////////

#ifndef __VISUALIZER_H__
#define __VISUALIZER_H__

// Dependencies
// 1. C++ STL:
#include <iostream>
#include <fstream>

// 2. Boost Library:
// 2. Boost Library:
#include "boost/geometry.hpp"
#include "boost/geometry/geometries/point_xy.hpp"
#include "boost/geometry/geometries/polygon.hpp"

// 3. Texo Library:
#include "cornerStitching.hpp"
#include "floorplan.hpp"
#include "technology.hpp"
#include "ballOut.hpp"
#include "objectArray.hpp"
#include "microBump.hpp"
#include "voronoiPDNGen.hpp"
#include "pressureSimulator.hpp"
#include "diffusionEngine.hpp"
#include "powerDistributionNetwork.hpp"

using FPGMPoint = boost::geometry::model::d2::point_xy<flen_t>;
using FPGMPolygon = boost::geometry::model::polygon<FPGMPoint>;
using FPGMMultiPolygon = boost::geometry::model::multi_polygon<FPGMPolygon>;

// use "renderCornerStitching.py" tor render class cornerStitching, which composed of Tiles with pointers
bool visualiseCornerStitching(const CornerStitching &cs, const std::string &filePath);

// use "renderFloorplan.py" for render class floorplan 
bool visualiseFloorplan(const Floorplan &fp, const std::string &filePath);

// use "renderBallOut.py" to render ballOut data structure
bool visualiseBallOut(const BallOut &ballout, const Technology &tch, const std::string &filePath);

// use "renderObjectArray" to render pin Arrays, Grid Arrays or integrated visualisation mode
bool visualisePinArray(const std::vector<std::vector<SignalType>> &pinArr, const Technology &tch, const std::string &filePath);
bool visualiseGridArray(const std::vector<std::vector<SignalType>> &gridArr, const Technology &tch, const std::string &filePath);
bool visualiseGridArrayWithPin(const std::vector<std::vector<SignalType>> &gridArr, const std::vector<std::vector<SignalType>> &pinArr, const Technology &tch, const std::string &filePath);
bool visualiseGridArrayWithPins(const std::vector<std::vector<SignalType>> &gridArr,const std::vector<std::vector<SignalType>> &upPinArr, const std::vector<std::vector<SignalType>> &downPinArr, const Technology &tch, const std::string &filePath);
bool visualiseMicroBump(const MicroBump &microBump, const Technology &tch, const std::string &filePath);

// use "renderVoronoiPointsSegments" to render points and segments structures in the voronoi algorithm
bool visualisePointsSegments(const VoronoiPDNGen &vpg, const std::unordered_map<SignalType, std::vector<Cord>> &points, const std::unordered_map<SignalType, std::vector<OrderedSegment>> &segments, const std::string &filePath);
bool visualisePointsSegments(const VoronoiPDNGen &vpg, const std::unordered_map<SignalType, std::vector<FCord>> &fpoints, const std::unordered_map<SignalType, std::vector<FOrderedSegment>> &fsegments, const std::string &filePath);
bool visualiseVoronoiGraph(const VoronoiPDNGen &vpg, const std::unordered_map<SignalType, std::vector<Cord>> &points, const std::unordered_map<Cord, std::vector<FCord>> &cells, const std::string &filePath);
bool visualiseMultiPolygons(const VoronoiPDNGen &vpg, const std::unordered_map<SignalType, FPGMMultiPolygon> &multiPolygons, const std::string &filePath);

// use "renderPressureSimulator" to render SoftBody status, viaBody status or integrated visualisation mode
bool visualiseSoftBodies(const PressureSimulator &ps, const std::vector<SoftBody *> softBodies, const std::string &filePath);
bool visualiseSoftBodiesWithPin(const PressureSimulator &ps, const std::vector<SoftBody *> softBodies, const std::vector<ViaBody *> vias, const std::string &filePath);
bool visualiseSoftBodiesWithPins(const PressureSimulator &ps, const std::vector<SoftBody *> softBodies, const std::vector<ViaBody *> upVias,  const std::vector<ViaBody *> downVias, const std::string &filePath);

// use "renderDissusionEngine" to render metalCell/viaCell status
bool visualiseDiffusionEngineMetal(const DiffusionEngine &dfe, size_t layer, const std::string &filePath);
bool visualiseDiffusionEngineVia(const DiffusionEngine &dfe, size_t layer, const std::string &filePath);
bool visualiseDiffusionEngineMetalAndVia(const DiffusionEngine &dfe, size_t metalLayer, size_t viaLayer, const std::string &filePath);

// user "renderPhysicalImplementation" to render PDNNode/PDNEdge 
bool visualisePhysicalImplementation(const PowerDistributionNetwork &pdn, int layer, const std::string &filePath);


#endif // __VISUALIZER_H__