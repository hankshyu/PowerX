//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/09/2025 21:39:46
//  Module Name:        pinout.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A data strucutre of pinout storge and lookups
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//
//////////////////////////////////////////////////////////////////////////////////

#ifndef __PINOUT_H__
#define __PINOUT_H__

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
#include "rectangle.hpp"


class Pinout{
private:
    std::string m_name;
    Rectangle m_footprint;
    Cord m_relativePositon;
    
    std::unordered_set<std::string> m_allTypes;
    std::unordered_map<std::string, std::unordered_set<Cord>> m_typeToCords;
    std::unordered_map<Cord, std::string> m_cordToType;
public:
    
    Pinout();
    explicit Pinout(const len_t footprintWidth, const len_t footprintHeight);
    explicit Pinout(const Rectangle &footprint);
    explicit Pinout(const Rectangle &footprint, const Cord &relativePosition);
    
    void setName(const std::string &name);
    void setRelativePosition(const Cord &relativePosition);

    std::string getName() const;
    Rectangle getFootprint() const;
    Cord getRelativePosition() const;

    bool readFromPinoutFile(const std::string &pinoutFile);
    void insertPin(const Cord &cord, const std::string &cordType);

    std::string getPinType(const Cord &c) const;
    bool getAllPinsofType(const std::string & pinType, std::unordered_set<Cord> &pinsOfType) const;

    bool exportPinOut(std::unordered_map<std::string, std::vector<Cord>> &allPinouts) const;
    
    void visualizePinOut(std::ostream &os) const;

};


#endif // __PINOUT_H__