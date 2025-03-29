//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/29/2025 20:53:04
//  Module Name:        c4Bump.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A data structure of storing C4 Bump under interposers.
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//
/////////////////////////////////////////////////////////////////////////////////

#ifndef __C4BUMP_H__
#define __C4BUMP_H__

// Dependencies
// 1. C++ STL:
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

// 2. Boost Library:

// 3. Texo Library:
#include "technology.hpp"
#include "signalType.hpp"
#include "ballOut.hpp"
#include "pinMap.hpp"

struct C4PinCluster {
    Cord representation; // usually the center of the cluster
    // Cord lowerLeftBall;
    SignalType clusterSignalType;

    std::unordered_set<Cord> pins;

    C4PinCluster();
    explicit C4PinCluster(const Cord &rep, const SignalType &sigType);

};

class C4Bump : public PinMap{
private:

    BallOut *m_c4BallOut;

    int m_clusterPinCountWidth;
    int m_clusterPinCountHeight;

    int m_clusterPitchWidth;
    int m_clusterPitchHeight;

    int m_clusterCountWidth;
    int m_clusterCountHeight;

    int m_leftBorder;
    int m_rightBorder;
    int m_upBorder;
    int m_downBorder;

public:
    C4Bump();
    explicit C4Bump(const std::string &fileName);
    ~C4Bump();

    std::vector<C4PinCluster *> allClusters;
    std::unordered_map<SignalType, std::unordered_set<C4PinCluster *>> signalTypeToAllClusters;
    std::unordered_map<Cord, C4PinCluster *> cordToClusterMap;

    friend bool visualiseC4Bump(const C4Bump &c4, const Technology &tch, const std::string &filePath);

};

#endif // __C4BUMP_H__