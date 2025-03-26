//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/11/2025 22:19:09
//  Module Name:        ballOut.cpp
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

// Dependencies
// 1. C++ STL:
#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cctype>
#include <climits>


// 2. Boost Library:

// 3. Texo Library:
#include "cord.hpp"
#include "ballOut.hpp"

BallOut::BallOut(): m_name(""), m_ballOutWidth(0), m_ballOutHeight(0), m_ballTypeIdCounter(0) {

}

BallOut::BallOut(const std::string &filePath): m_ballTypeIdCounter(0) {
    
    std::ifstream file(filePath);
    assert(file.is_open());
    
    std::string buffer;

    file >> buffer >> this->m_name >> this->m_ballOutWidth >> this->m_ballOutHeight;
    ballOutArray.resize(this->m_ballOutHeight, std::vector<ballTypeId>(this->m_ballOutWidth, UCHAR_MAX));
        
    for(int j = 0; j < this->m_ballOutHeight; ++j){
        for(int i = 0; i < this->m_ballOutWidth; ++i){

            file >> buffer;

            // parse the string, keep only substring after ','
            size_t position = buffer.find(',');
            assert(position != std::string::npos);
            
            Cord pinLocation = CSVCellToCord(buffer.substr(0, position));
            if(pinLocation != Cord(i, j)){
                std::cout << "[PowerX:BallOutParser] Error: Discontinuous CSV Cell position value: " << buffer.substr(0, position)  << std::endl;
                std::cout << pinLocation << std::endl;
                assert(pinLocation == Cord(i, j));

            }
            
            ballType balltp = buffer.substr(position + 1);
            pinLocation.y(this->m_ballOutHeight - pinLocation.y() - 1);
            // Cord(i, this->m_ballOutHeight - j - 1)
            std::unordered_map<ballType, ballTypeId>::const_iterator cit = ballTypeToIdMap.find(balltp);
            
            if(cit == ballTypeToIdMap.end()){
                ballOutArray[pinLocation.x()][pinLocation.y()] = m_ballTypeIdCounter;
                ballTypeToIdMap[balltp] = m_ballTypeIdCounter;
                IdToBallTypeMap[m_ballTypeIdCounter] = balltp;
                IdToAllCords[m_ballTypeIdCounter] = {pinLocation};
                m_ballTypeIdCounter++;
            }else{
                ballTypeId id = cit->second;
                ballOutArray[pinLocation.x()][pinLocation.y()] = id;

                IdToAllCords[id].insert(pinLocation);
            }
        }
    }

    file.close();
}

Cord CSVCellToCord(const std::string &CSVCell){
    
    int yValue = 0;
    int xValue = 0;

    int splitPosition = 0;
    for( ; splitPosition < CSVCell.length(); ++splitPosition){
        char c = CSVCell[splitPosition];
        if(std::islower(c)){
            yValue *= 26;
            yValue += (c - 'a' + 1);
        }else if(std::isupper(c)){
            yValue *= 26;
            yValue += (c - 'A' + 1);
        }else if(std::isdigit(c)){
            xValue = stoi(CSVCell.substr(splitPosition));
            
            return Cord(xValue - 1, yValue - 1);
        }else{
            break;
        }
    }

    std::cout << "[PowerX:BallOutParser] Error: Unknown CSV Cell position value: " << CSVCell << "" << std::endl;
    return Cord(-1, -1);

}

std::vector<ballType> BallOut::getAllBallTypes() const {
    std::vector<ballType> allBallTypes;
    for(std::unordered_map<ballType, ballTypeId>::const_iterator cit = ballTypeToIdMap.begin(); cit != ballTypeToIdMap.end(); ++cit){
        allBallTypes.push_back(cit->first);
    }
    return allBallTypes;
}