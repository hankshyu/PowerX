//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/23/2025 15:05:54
//  Module Name:        aStarBaseline.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        Implement A* BaseLine Algorithm in 
//                      Liao, H., Patil, V., Dong, X., Shanbhag, D., Fallon, E., Hogan, T.,
//                      Spasojevic, M., and Burak Kara, L. (July 25, 2023). 
//                      "Hierarchical Automatic Multilayer Power Plane Generation With Genetic 
//                      Optimization and Multilayer Perceptron." 
//                      ASME. J. Mech. Des. October 2023; 145(10): 101706.
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//
//////////////////////////////////////////////////////////////////////////////////

#ifndef __ASTARBASELINE_H__
#define __ASTARBASELINE_H__

// Dependencies
// 1. C++ STL:
#include <vector>

// 2. Boost Library:


// 3. Texo Library:
#include "cord.hpp"
#include "signalType.hpp"
#include "microBump.hpp"
#include "c4Bump.hpp"


class AStarBaseline{
public:

    MicroBump uBump;
    C4Bump c4;

    len_t canvasWidth;
    len_t canvasHeight;
    std::vector<std::vector<SignalType>> canvasM5;
    std::vector<std::vector<SignalType>> canvasM7;

    AStarBaseline(const std::string &fileName);

    void calculateUBumpMST();

    Cord traslateIdxToCord(int idx) const;
    int translateCordToIdx(Cord cord) const;

    friend bool visualiseM5(const AStarBaseline &ast, const std::string &filePath);
    friend bool visualiseM7(const AStarBaseline &ast, const std::string &filePath);

};



#endif // __ASTARBASELINE_H__