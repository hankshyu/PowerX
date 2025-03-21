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
#include <algorithm>
#include <unordered_map>
#include <unordered_set>


// 2. Boost Library:

// 3. Texo Library:
#include "units.hpp"
#include "cord.hpp"
#include "rectangle.hpp"
#include "bumpMap.hpp"

BumpMap::BumpMap(): m_name(""), bumpCountWidth(0), bumpCountHeight(0) {

}

BumpMap::BumpMap(const std::string &filePath){
    std::ifstream file(filePath);
    assert(file.is_open());
    
    std::string buffer;

    file >> buffer >> this->m_name >> this->bumpCountWidth >> this->bumpCountHeight;
        
    std::string pinType;
    for(int j = 0; j < this->bumpCountHeight; ++j){
        for(int i = 0; i < this->bumpCountWidth; ++i){

            file >> buffer;

            // parse the string, keep only substring after ','
            size_t position = buffer.find(',');
            assert(position != std::string::npos);
            pinType = buffer.substr(position + 1);
            std::transform(pinType.begin(), pinType.end(), pinType.begin(), ::toupper);
            if((pinType != "SIG") || (pinType != "SIGNAL")){
                Cord cord(i, this->bumpCountHeight - j - 1);
                if(m_allBumpTypes.find(pinType) == m_allBumpTypes.end()){
                    m_allBumpTypes.insert(pinType);
                    m_TypeToCords[pinType] = {cord};
                }else{
                    m_TypeToCords[pinType].insert(cord);
                }

                m_bumpMap[cord] = pinType;
            }
        }
    }

    file.close();
}

