//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/11/2025 22:19:09
//  Module Name:        bumpMap.cpp
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

// Dependencies
// 1. C++ STL:
#include <string>
#include <fstream>
#include <unordered_map>
#include <unordered_set>


// 2. Boost Library:

// 3. Texo Library:
#include "units.hpp"
#include "cord.hpp"
#include "rectangle.hpp"
#include "bumpMap.hpp"

BumpMap::BumpMap()
    :m_name(""), m_footprint(Rectangle(0, 0, 0, 0)) {

}

BumpMap::BumpMap(const std::string &name, const len_t footprintWidth, const len_t footprintHeight)
    :m_name(name), m_footprint(Rectangle(0, 0, footprintWidth, footprintHeight)) {

}

BumpMap::BumpMap(const std::string &name, const Rectangle &footprint)
    :m_name(name), m_footprint(Rectangle(footprint)) {

}
BumpMap::BumpMap(const std::string &filePath){
    importBumpMap(filePath);
}

bool BumpMap::importBumpMap(const std::string &filePath){
    std::ifstream file(filePath);
    assert(file.is_open());
    if(!file.is_open()) return false;
    
    std::string buffer;
    len_t chipletWidth, chipletHieght;

    file >> buffer >> this->m_name >> chipletWidth >> chipletHieght;
    
    m_footprint = Rectangle(0, 0, chipletWidth, chipletHieght);
    
    std::string pinType;
    for(int j = 0; j < chipletHieght; ++j){
        for(int i = 0; i < chipletWidth; ++i){

            file >> buffer;

            // parse the string, keep only substring after ','
            size_t position = buffer.find(',');
            assert(position != std::string::npos);
            pinType = buffer.substr(position + 1);

            if(pinType != "SIG"){
                Cord cord(i, chipletHieght - j - 1);
                if(m_allBumpTypes.find(pinType) == m_allBumpTypes.end()){
                    m_allBumpTypes.insert(pinType);
                }

                m_bumpMap[cord] = pinType;
            }
        }
    }

    file.close();
    return true;
}

std::string BumpMap::getName() const {
    return this->m_name;
}

Rectangle BumpMap::getFootprint() const {
    return this->m_footprint;
}

int BumpMap::getWidth() const {
    return rec::getWidth(m_footprint);
}

int BumpMap::getHeight() const {
    return rec::getHeight(m_footprint);
}

const std::unordered_set<bumpType> &BumpMap::getBumpTypes() const {
    return this->m_allBumpTypes;
}

const std::map<Cord, std::string> &BumpMap::getBumpMap() const{
    return this->m_bumpMap;
}
