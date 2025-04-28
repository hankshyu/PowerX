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
#include "technology.hpp"
#include "ballOut.hpp"
#include "microBump.hpp"
#include "c4Bump.hpp"
#include "cornerStitching.hpp"
#include "floorplan.hpp"
#include "powerGrid.hpp"
#include "voronoiPDNGen.hpp"


// use "renderPinMap.py" to render ballOut data structure
bool visualiseBallOut(const BallOut &bumpMap, const Technology &tch, const std::string &filePath);

// use "renderPinMap.py" to render UBump data structure
bool visualiseMicroBump(const MicroBump &microBump, const Technology &tch, const std::string &filePath);

// // use "renderPinMap.py" to render C4 data structure
bool visualiseC4Bump(const C4Bump &c4, const Technology &tch, const std::string &filePath);

// use "renderCornerStitching.py" tor render class cornerStitching, which composed of Tiles with pointers
bool visualiseCornerStitching(const CornerStitching &cs, const std::string &filePath);

// use "renderFloorplan.py" for render class floorplan 
bool visualiseFloorplan(const Floorplan &fp, const std::string &filePath);

bool visualisePGM5(const PowerGrid &pg, const std::string &filePath, bool overlayOverlaps = false, bool overlayM5uBump = false, bool overlayM7C4 = false);
bool visualisePGM7(const PowerGrid &pg, const std::string &filePath, bool overlayOverlaps = false, bool overlayM5uBump = false, bool overlayM7C4 = false);
bool visualisePGOverlap(const PowerGrid &pg, const std::string &filePath, bool overlayM5uBump = false, bool overlayM7C4 = false);

bool visualiseM5VoronoiPointsSegments(const VoronoiPDNGen &vpg, const std::string &filePath);
bool visualiseM7VoronoiPointsSegments(const VoronoiPDNGen &vpg, const std::string &filePath);
bool visualiseM5VoronoiGraph(const VoronoiPDNGen &vpg, const std::string &filePath);
bool visualiseM7VoronoiGraph(const VoronoiPDNGen &vpg, const std::string &filePath);
bool visualiseM5VoronoiPolygons(const VoronoiPDNGen &vpg, const std::string &filePath);
bool visualiseM7voronoiPolygons(const VoronoiPDNGen &vpg, const std::string &filePath);

#endif // __VISUALIZER_H__