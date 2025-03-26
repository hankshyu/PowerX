//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/11/2025 21:20:17
//  Module Name:        ballOut.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A container for buump map files. Could import and export
//                      from .csv standard format to PowerX interanl pin data structures
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//  2025/03/26          Change name from bumpMap to ballOut. Reconstruct class
//
/////////////////////////////////////////////////////////////////////////////////

#ifndef __BALLOUT_H__
#define __BALLOUT_H__

// Dependencies
// 1. C++ STL:
#include <string>
#include <ostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>


// 2. Boost Library:

// 3. Texo Library:
#include "cord.hpp"
#include "technology.hpp"

typedef std::string ballType;
typedef unsigned char ballTypeId;

class BallOut{
private:
    std::string m_name;
    int m_ballOutWidth;
    int m_ballOutHeight;
    int m_ballTypeIdCounter;

public:
    std::vector<std::vector<ballTypeId>> ballOutArray;

    std::unordered_map<ballType, ballTypeId> ballTypeToIdMap;
    std::unordered_map<ballTypeId, ballType> IdToBallTypeMap;

    std::unordered_map<ballTypeId, std::unordered_set<Cord>> IdToAllCords;


    
    BallOut();
    explicit BallOut(const std::string &filePath);

    inline std::string getName() const {return this->m_name;}
    inline int getBallOutWidth() const {return this->m_ballOutWidth;}
    inline int getBallOutHeight() const {return this->m_ballOutHeight;}

    std::vector<ballType> getAllBallTypes() const;
    
    friend bool visualiseBallOut(const BallOut &bumpMap, const Technology &tch, const std::string &filePath);
    
};

Cord CSVCellToCord(const std::string &CSVCell);

#endif // __BALLOUT_H__