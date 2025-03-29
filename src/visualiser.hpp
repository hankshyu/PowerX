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
#include "uBump.hpp"
#include "cornerStitching.hpp"
#include "floorplan.hpp"


// use "renderPinMap.py" to render ballOut data structure
bool visualiseBallOut(const BallOut &bumpMap, const Technology &tch, const std::string &filePath);

// use "renderPinout.py" to render UBump data structure
bool visualiseUBump(const UBump &uBump, const Technology &tch, const std::string &filePath);

// // use "renderPinout.py" to render Ballout data structure
// bool visualiseBallout(const Ballout &pinout, const Technology &tch, const std::string &filePath);

// use "renderCornerStitching.py" tor render class cornerStitching, which composed of Tiles with pointers
bool visualiseCornerStitching(const CornerStitching &cs, const std::string &filePath);

// use "renderFloorplan.py" for render class floorplan 
bool visualiseFloorplan(const Floorplan &fp, const std::string &filePath);





#endif // __VISUALIZER_H__