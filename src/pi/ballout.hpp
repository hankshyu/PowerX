//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/20/2025 15:14:29
//  Module Name:        ballout.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A data structure for C4 bumpball storage and lookups
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//
//////////////////////////////////////////////////////////////////////////////////

#ifndef __BALLOUT_H__
#define __BALLOUT_H__

// Dependencies
// 1. C++ STL:
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <ostream>

// 2. Boost Library:


// 3. Texo Library:
#include "units.hpp"
#include "cord.hpp"
#include "technology.hpp"

typedef std::string ballType;

struct Cluster {
    Cord representation;
    Cord lowerLeftBall;
    ballType clusterType;

    int ballMapX;
    int ballmapY;
    std::unordered_set<Cord> ballouts;
};

class Ballout{
private:
    
    std::string m_name;

    int m_pinCountWidth;
    int m_pinCountHeight;

    int m_ballWidth;
    int m_ballHeight;

    int m_ballPitchWidth;
    int m_ballPitchHeight;

    int m_ballCountWidth;
    int m_ballCountHeight;

    int m_leftBorder;
    int m_rightBorder;
    int m_upBorder;
    int m_downBorder;


    std::unordered_map<ballType, std::set<Cluster *>> m_ballTypeToClusters;
    std::unordered_map<Cord, ballType> m_cordToBallType;
    std::unordered_map<Cord, Cluster *> m_cordToCluster;

    public:

    std::vector<std::vector<Cluster>> m_ballMap;
    
    Ballout();
    explicit Ballout(const std::string &fileName);

    inline std::string getName() const {return this->m_name;}
    inline int getBallCountWidth() const {return this->m_ballCountWidth;}
    inline int getBallCountHeight() const {return this->m_ballCountHeight;}
    inline int getBallWidth() const {return this->m_ballWidth;}
    inline int getBallHeight() const {return this->m_ballHeight;}

    inline int getPinCountWidth() const {return this->m_pinCountWidth;}
    inline int getPinCountHeight() const {return this->m_pinCountHeight;}

    const Cluster &getCluster(const Cord &cord) const;
    ballType getBallType(const Cord &cord) const;
    Cord getRepresentation(const Cord &cord) const;

    friend bool visualiseBallout(const Ballout &ballout, const Technology &tch, const std::string &filePath);
};

#endif // __BALLOUT_H__