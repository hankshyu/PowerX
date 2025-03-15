//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/09/2025 23:22:24
//  Module Name:        pinout.cpp
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

// Dependencies
// 1. C++ STL:
#include <fstream>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <sstream>

// 2. Boost Library:


// 3. Texo Library:
#include "pinout.hpp"
#include "cord.hpp"
#include "rectangle.hpp"

Pinout::Pinout()
    :m_name(""), m_footprint(Rectangle(0, 0, 0, 0)) {
}

Pinout::Pinout(const len_t footprintWidth, const len_t footprintHeight)
    :m_name(""), m_footprint(Rectangle(0, 0, footprintWidth, footprintHeight)) {

}

Pinout::Pinout(const Rectangle &footprint)
    :m_name(""), m_footprint(footprint) {

}

Pinout::Pinout(const std::string &fileName){
    std::ifstream file(fileName);
    assert(file.is_open());

    std::string buffer;
    std::string lineBuffer;
    while(std::getline(file, lineBuffer)){
        if (lineBuffer.empty()|| lineBuffer.find("#") == 0) continue; // whole line comment
        
        size_t comment_pos = lineBuffer.find_first_of("#");
        if (comment_pos != std::string::npos) {
                lineBuffer = lineBuffer.substr(0, comment_pos);  // Remove everything after "#"
        }

        // process line
        // std::cout << lineBuffer << std::endl;
        
        std::vector<std::string> splitLine;
        std::istringstream stream(lineBuffer);
        std::string wordBuffer;
    
        while (stream >> wordBuffer) { // Extract words by spaces and newlines
            splitLine.push_back(wordBuffer);
        }

        if(splitLine[0] == "include"){
            splitLine[1].erase(std::remove(splitLine[1].begin(), splitLine[1].end(), '"'), splitLine[1].end());
            BumpMap bm(splitLine[1]);
            
            if(m_allChipletTypes.find(bm.getName()) == m_allChipletTypes.end()){
                m_allChipletTypes[bm.getName()] = bm;
            }else{
                std::cout << "[PowerX:PinParser] Warning: Repeated ballout: " << bm.getName() << std::endl;
            }
            const std::unordered_set<bumpType> & chipletBumpTypes = bm.getBumpTypes();
            for(const bumpType &bt : chipletBumpTypes){
                if(m_allPinTypes.find(bt) == m_allPinTypes.end()){
                    m_allPinTypes.insert(bt);
                    m_typeToCords[bt] = {};
                }
            }

        }else if(splitLine[0] == "INTERPOSER"){
            m_name = splitLine[1];
            m_footprint = Rectangle(0, 0, std::stoi(splitLine[2]), std::stoi(splitLine[3]));

        }else if(splitLine[0] == "CHIPLET"){
            if(m_allChipletTypes.find(splitLine[1]) == m_allChipletTypes.end()){
                std::cout << "[PowerX:PinParser] Unknown chiplet " << lineBuffer << std::endl;
            }

            BumpMap &bm = m_allChipletTypes[splitLine[1]];
            m_instanceToType[splitLine[2]] = bm.getName();
            
            splitLine[3].erase(std::remove(splitLine[3].begin(), splitLine[3].end(), '('), splitLine[3].end());
            splitLine[3].erase(std::remove(splitLine[3].begin(), splitLine[3].end(), ','), splitLine[3].end());

            splitLine[4].erase(std::remove(splitLine[4].begin(), splitLine[4].end(), ')'), splitLine[4].end());
            len_t xDiff = std::stoi(splitLine[3]);
            len_t yDiff = std::stoi(splitLine[4]);
            
            m_instanceToBoundingBox[splitLine[2]] = Rectangle(xDiff, yDiff, xDiff + bm.getWidth(), yDiff + bm.getHeight());

            const std::map<Cord, bumpType>& bumpMappings = bm.getBumpMap();
            for(std::map<Cord, bumpType>::const_iterator it = bumpMappings.begin(); it != bumpMappings.end(); it++){
                Cord originalCord = it->first;
                Cord transformedCord = Cord(originalCord.x() + xDiff, originalCord.y() + yDiff);
                m_cordToType[transformedCord] = it->second;
                m_typeToCords[it->second].insert(transformedCord);
            }

        }else{
            std::cout << "[PowerX:PinParser] Unmatch string: " << lineBuffer << std::endl;
        }
    }
}

std::string Pinout::getName() const {
    return this->m_name;
}

Rectangle Pinout::getFootprint() const {
    return this->m_footprint;
}

chipletType Pinout::getInstanceType(const std::string &chipletName) const {
    std::map<std::string, chipletType>::const_iterator it = m_instanceToType.find(chipletName);
    return (it == m_instanceToType.end())? "" : it->second;
}

bumpType Pinout::getPinType(const Cord &c) const {
    std::unordered_map<Cord, bumpType>::const_iterator it = m_cordToType.find(c);
    return (it == m_cordToType.end())? "" : it->second;
}

bool Pinout::getAllPinsofType(const bumpType & pinType, std::set<Cord> &locations) const{
    std::unordered_map<bumpType, std::set<Cord>>::const_iterator it = m_typeToCords.find(pinType);
    if(it == m_typeToCords.end()) return false;
    else{
        locations.insert(it->second.begin(), it->second.end());
        return true;
    }
}

