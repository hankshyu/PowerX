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
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <ostream>

// 2. Boost Library:


// 3. Texo Library:
#include "units.hpp"
#include "cord.hpp"
#include "technology.hpp"
#include "bumpMap.hpp"

typedef std::string chipletType;

class Pinout{
private:
    std::string m_name;
    int m_pinCountWidth;
    int m_pinCountHeight;
    
    std::map<chipletType, BumpMap> m_allChipletTypes;
    std::map<std::string, chipletType> m_instanceToType;
    std::map<std::string, Cord> m_instanceToLL;


    std::unordered_set<bumpType> m_allPinTypes;
    std::unordered_map<bumpType, std::set<Cord>> m_typeToCords;
    std::unordered_map<Cord, bumpType> m_cordToType;

public:
    
    Pinout();
    explicit Pinout(const std::string &fileName);

    inline std::string getName() const {return this->m_name;}
    inline int getPinCountWidth() const {return this->m_pinCountWidth;}
    inline int getPinCountHeight() const {return this->m_pinCountHeight;}

    chipletType getInstanceType(const std::string &chipletName) const;
    std::vector<BumpMap> getAllChipletTypes() const;
    
    // retur the bumpType related to the Cord, if unmatch return empty string
    bumpType getPinType(const Cord &c) const;
    bool getAllPinsofType(const bumpType & pinType, std::set<Cord> &locations) const;

    // bool exportPinOut(std::unordered_map<std::string, std::vector<Cord>> &allPinouts) const;
    
    friend bool visualisePinout(const Pinout &pinout, const Technology &tch, const std::string &filePath);

};
#endif // __PINOUT_H__