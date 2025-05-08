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

// 3. Texo Library:
#include "cornerStitching.hpp"
#include "floorplan.hpp"
#include "technology.hpp"
#include "ballOut.hpp"
#include "objectArray.hpp"
#include "microBump.hpp"
#include "voronoiPDNGen.hpp"

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
// bool visualiseVoronoiGraph(const VoronoiPDNGen &vpg, const )
// bool VisualisePolygons
/*
bool visualiseM5VoronoiGraph(const VoronoiPDNGen &vpg, const std::string &filePath);
bool visualiseM7VoronoiGraph(const VoronoiPDNGen &vpg, const std::string &filePath);
bool visualiseM5VoronoiPolygons(const VoronoiPDNGen &vpg, const std::string &filePath);
bool visualiseM7voronoiPolygons(const VoronoiPDNGen &vpg, const std::string &filePath);
*/

#endif // __VISUALIZER_H__