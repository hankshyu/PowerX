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
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

// 2. Boost Library:


// 3. Texo Library:
#include "pinout.hpp"
#include "cord.hpp"
#include "rectangle.hpp"

Pinout::Pinout()
    :m_name(""), m_footprint(Rectangle(0, 0, 0, 0)), m_relativePositon(Cord(-1, -1)) {
}

Pinout::Pinout(const len_t footprintWidth, const len_t footprintHeight)
    :m_name(""), m_footprint(Rectangle(0, 0, footprintWidth, footprintHeight)), m_relativePositon(Cord(-1, -1)) {

}

Pinout::Pinout(const Rectangle &footprint)
    :m_name(""), m_footprint(footprint), m_relativePositon(Cord(-1, -1)) {

}

Pinout::Pinout(const Rectangle &footprint, const Cord &relativePosition)
    :m_name(""), m_footprint(footprint), m_relativePositon(relativePosition) {

}

void Pinout::setName(const std::string &name){
    this->m_name = name;
}

void Pinout::setRelativePosition(const Cord &relativePosition){
    this->m_relativePositon = relativePosition;
}

std::string Pinout::getName() const {
    return this->m_name;
}

Rectangle Pinout::getFootprint() const {
    return this->m_footprint;
}

Cord Pinout::getRelativePosition() const {
    return this->m_relativePositon;
}

bool Pinout::readFromPinoutFile(const std::string &pinoutFile){
    std::ifstream file(pinoutFile);
    if(!file.is_open()) return false;
    
    std::string buffer;
    len_t chipletWidth, chipletHieght;

    file >> buffer >> this->m_name >> chipletWidth >> chipletHieght;
    
    std::cout << buffer << std::endl;
    std::cout << getName() << std::endl;
    std::cout << chipletWidth << std::endl;
    std::cout << chipletHieght << std::endl;

    m_footprint = Rectangle(0, 0, chipletWidth - 1, chipletHieght -1);
    
    std::string pinType;
    for(int i = 0; i < chipletHieght; ++i){
        for(int j = 0; j < chipletWidth; ++j){

            file >> buffer;

            // parse the string, keep only substring after ','
            size_t position = buffer.find(',');
            assert(position != std::string::npos);
            pinType = buffer.substr(position + 1);

            if(pinType != "SIG"){
                Cord cord(i, j);
                if(m_allTypes.find(pinType) == m_allTypes.end()){
                    m_allTypes.insert(pinType);
                    m_typeToCords[pinType] = {cord};
                    m_cordToType[cord] = pinType;
                }else{
                    m_typeToCords[pinType].insert(cord);
                    m_cordToType[cord] = pinType; 
                }
            }

        }
    }

    file >> buffer;
    file.close();
    return true;
}

void Pinout::insertPin(const Cord &cord, const std::string &cordType){
    
    assert(m_cordToType.find(cord) == m_cordToType.end());
    
    this->m_cordToType[cord] = cordType;
    if(m_allTypes.find(cordType) != m_allTypes.end()){
        // such type already exist
        this->m_typeToCords[cordType].insert(cord);
    }else{
        // new type
        this->m_allTypes.insert(cordType);
    }
}

std::string Pinout::getPinType(const Cord &c) const {
    std::unordered_map<Cord, std::string>::const_iterator it = m_cordToType.find(c);
    return (it == m_cordToType.end())? "" : it->second;
}

bool Pinout::getAllPinsofType(const std::string & pinType, std::unordered_set<Cord> &pinsOfType) const {
    std::unordered_map<std::string, std::unordered_set<Cord>>::const_iterator it = m_typeToCords.find(pinType);
    
    if(it == m_typeToCords.end()) return false;
    else{
        pinsOfType.insert(it->second.begin(), it->second.end());
        return true;
    }
}

bool Pinout::exportPinOut(std::unordered_map<std::string, std::vector<Cord>> &allPinouts) const {
    std::unordered_map<std::string, std::unordered_set<Cord>>::const_iterator it;

    for(it = m_typeToCords.begin(); it != m_typeToCords.end(); ++it){
        allPinouts[it->first] = std::vector<Cord>(it->second.begin(), it->second.end());
    }
}

void Pinout::visualizePinOut(std::ostream &os) const{
    
    os << m_name << " " << (rec::getWidth(m_footprint) + 1) << " " << (rec::getHeight(m_footprint) + 1) << "\n";
    std::map<Cord, std::string> sortedMap(m_cordToType.begin(), m_cordToType.end());
    for (const std::pair<const Cord, std::string>& entry : sortedMap) {
        os << entry.first << " " << entry.second << '\n';
    }
}
