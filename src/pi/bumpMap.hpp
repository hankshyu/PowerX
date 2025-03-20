//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/11/2025 21:20:17
//  Module Name:        bumpMap.hpp
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
//
/////////////////////////////////////////////////////////////////////////////////

#ifndef __BUMPMAP_H__
#define __BUMPMAP_H__

// Dependencies
// 1. C++ STL:
#include <string>
#include <ostream>
#include <map>
#include <unordered_set>


// 2. Boost Library:

// 3. Texo Library:
#include "units.hpp"
#include "cord.hpp"
#include "rectangle.hpp"
#include "technology.hpp"

typedef std::string bumpType;

class BumpMap{
private:
    std::string m_name;
    int bumpCountWidth;
    int bumpCountHeight;
    std::unordered_set<bumpType> m_allBumpTypes;
    std::map<Cord, bumpType> m_bumpMap;
    std::map<bumpType, std::set<Cord>> m_TypeToCords;

public:
    
    BumpMap() = delete;
    explicit BumpMap(const std::string &filePath);

    std::string getName() const;
    int getbumpCountWidth() const;
    int getbumpCountHeight() const;

    friend bool visualiseBumpMap(const BumpMap &bumpMap, const Technology &tch, const std::string &filePath);
    
};

#endif // __BUMPMAP_H__