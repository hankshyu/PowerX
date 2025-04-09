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

// 2. Boost Library:

// 3. Texo Library:
#include "powerGrid.hpp"



class VoronoiPDNGen: public PowerGrid{
public:

    VoronoiPDNGen(const std::string &fileName);

    void runILPRouting();
    void runSATRouting();

};

#endif // __VORONOIPDNGEN_H__