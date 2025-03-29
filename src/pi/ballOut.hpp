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
//  2025/03/27          Add rotations to class
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
#include "rectangle.hpp"
#include "technology.hpp"
#include "signalType.hpp"

enum BallOutRotation : uint8_t{

    R0 = 0,
    R90 = 1, 
    R180 = 2, 
    R270 = 3,
    EMPTY = 4,
    UNKNOWN = 5,
};
static const size_t BALLOUT_ROTATION_COUNT = 4;

constexpr inline const char* to_string(BallOutRotation bor) {
    switch (bor) {
        case BallOutRotation::EMPTY:    return "EMPTY";
        case BallOutRotation::R0:       return "R0";
        case BallOutRotation::R90:      return "R90";
        case BallOutRotation::R180:     return "R180";
        case BallOutRotation::R270:     return "R270";
        case BallOutRotation::UNKNOWN:  return "UNKNOWN";
        default:                        return "UNKNOWN";
    }
};

std::ostream& operator<<(std::ostream& os, BallOutRotation bor);
BallOutRotation convertToBallOutRotation (const std::string &str);



class BallOut{
private:
    std::string m_name;
    int m_ballOutWidth;
    int m_ballOutHeight;
    enum BallOutRotation m_rotation;

public:
    std::unordered_set<SignalType> allSignalTypes;
    std::vector<std::vector<SignalType>> ballOutArray;
    std::unordered_map<SignalType, std::unordered_set<Cord>> SignalTypeToAllCords;


    BallOut();
    explicit BallOut(const std::string &filePath);
    explicit BallOut(const BallOut &ref, enum BallOutRotation rotation);

    inline std::string getName() const {return this->m_name;}
    inline int getBallOutWidth() const {return this->m_ballOutWidth;}
    inline int getBallOutHeight() const {return this->m_ballOutHeight;}
    inline Rectangle getBallOutSizeRectangle() const {return Rectangle(0, 0, (m_ballOutWidth > 1)? m_ballOutWidth-1 : 0, (m_ballOutHeight > 1)? m_ballOutHeight-1 : 0);}
    inline BallOutRotation getRotation() const {return this->m_rotation;}
    inline std::vector<SignalType> getAllSignalTypes() const {return std::vector<SignalType>(allSignalTypes.begin(), allSignalTypes.end());}
    
    friend bool visualiseBallOut(const BallOut &bumpMap, const Technology &tch, const std::string &filePath);
    
};

Cord CSVCellToCord(const std::string &CSVCell);

#endif // __BALLOUT_H__