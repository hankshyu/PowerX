//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/27/2025 09:38:16
//  Module Name:        objectArray.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A data structure that describes an array of pins, will be
//                      use to hold cross-layer pin array, micro-Bumps and C4 Bumps
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//  03/28/2025          Remove 2D grid data structure
//  03/29/2025          Remove Signal Type ID counting mechanics, use SignalType
//  05/02/2025          Resort to Array storage, move unordered mappings to child classes
//
/////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:
#include <cassert>
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <vector>
#include <unordered_set>
// 2. Boost Library:

// 3. Texo Library:
#include "cord.hpp"
#include "objectArray.hpp"

ObjectArray::ObjectArray(): m_width(0), m_height(0) {

}

ObjectArray::ObjectArray(int width, int height): m_width(width), m_height(height), canvas(height, std::vector<SignalType>(width, SignalType::EMPTY)) {

}

void ObjectArray::readBlockages(const std::string &fileName){
    std::ifstream file(fileName);
    assert(file.is_open());

    // lambda for checking if a string is a valid integer
    auto isInteger = [](const std::string& s) -> bool {
        try {
            size_t pos;
            std::stoi(s, &pos);
            return pos == s.size();
        } catch (...) {
            return false;
        }
    };

    // Lambda to trim leading and trailing whitespace
    auto trimWhiteSpace = [](const std::string& s) -> std::string {
        size_t start = 0;
        while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) {
            ++start;
        }

        size_t end = s.size();
        while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
            --end;
        }

        return s.substr(start, end - start);
    };

    bool readPreplace = false;
    bool readFirstSP = false;
    SignalType processingSP;

    std::string lineBuffer;
    std::unordered_set<Cord> allPreplaceCords;
    std::unordered_set<Cord> preplaceCord;
    
    const std::string signalPrefix = "SIGNAL:";
    std::regex singlePattern(R"(^\s*Cord\(\s*([^\s,]+)\s*,\s*([^\s\)]+)\s*\)\s*$)");
    std::regex doublePattern(R"(^\s*Cord\(\s*([^\s,]+)\s*,\s*([^\s\)]+)\s*\)\s*to\s*Cord\(\s*([^\s,]+)\s*,\s*([^\s\)]+)\s*\)\s*$)");
    std::smatch match;

    while(std::getline(file, lineBuffer)){
        if (lineBuffer.empty()|| lineBuffer[0] == '#') continue; // whole line comment
        
        size_t comment_pos = lineBuffer.find_first_of("#");
        if (comment_pos != std::string::npos) {
                lineBuffer = lineBuffer.substr(0, comment_pos);  // Remove everything after "#"
        }

        if(lineBuffer == "BEGIN_PREPLACE"){
            readPreplace = true; 
            continue;
        } 
        if(!readPreplace) continue;
        if(lineBuffer == "END_PREPLACE"){
            this->preplacedCords[processingSP] = std::vector<Cord>(preplaceCord.begin(), preplaceCord.end());
            break;
        }
        
        std::size_t pos = lineBuffer.find(signalPrefix);
        if (pos != std::string::npos) {

            if(readFirstSP){
                // write latched data
                this->preplacedCords[processingSP] = std::vector<Cord>(preplaceCord.begin(), preplaceCord.end());
                preplaceCord.clear();
            }else{
                readFirstSP = true;
            }

            std::string rest = lineBuffer.substr(pos + signalPrefix.length());
            std::string signalTypeStr = trimWhiteSpace(rest);
            processingSP = convertToSignalType(signalTypeStr);
            
            if(processingSP == SignalType::UNKNOWN){
                std::cout << "[PowerX:ObjectArray] Error: Unknown preplace SignalType " << signalTypeStr << std::endl;
                exit(4);
            }

            continue;
        }



        if(std::regex_match(lineBuffer, match, singlePattern)){ // Cord(x, y)
            std::string sx = match[1];
            std::string sy = match[2];

            if (!isInteger(sx) || !isInteger(sy)) {
                std::cout << "[PowerX:ObjectArray] Error: Coordinates must be integers: " << lineBuffer << std::endl;
                exit(4);
            }

            int x = std::stoi(sx);
            int y = std::stoi(sy);
            if (x < 0 || x >= m_width) {
                std::cout << "[PowerX:ObjectArray] Error: X Coordinates must be within range: [0, " << m_width-1 << "]: " << lineBuffer << std::endl;
                exit(4);

            }
            if(y < 0 || y >= m_height){
                std::cout << "[PowerX:ObjectArray] Error: Y Coordinates must be within range: [0, " << m_height-1 << "]: " << lineBuffer << std::endl;
                exit(4);
            }
            Cord newCord(x, y);
            if(allPreplaceCords.count(newCord) == 0){
                allPreplaceCords.insert(newCord);
                preplaceCord.insert(newCord);
            }


        }else if(std::regex_match(lineBuffer, match, doublePattern)){ // Cord(x1, y1) to Cord(x2, y2)
            std::string sx1 = match[1];
            std::string sy1 = match[2];
            std::string sx2 = match[3];
            std::string sy2 = match[4];

            if (!isInteger(sx1) || !isInteger(sy1) || !isInteger(sx2) || !isInteger(sy2)) {
                std::cout << "[PowerX:ObjectArray] Error: Coordinates must be integers: " << lineBuffer << std::endl;
                exit(4);
            }
            int x1 = std::stoi(sx1);
            int y1 = std::stoi(sy1);
            int x2 = std::stoi(sx2);
            int y2 = std::stoi(sy2);

            if(x1 < 0 || x1 >= m_width || x2 < 0 || x2 >= m_width){
                std::cout << "[PowerX:ObjectArray] Error: X Coordinates must be within range: [0, " << m_width-1 << "]: " << lineBuffer << std::endl;
                exit(4); 
            }
            if(y1 < 0 || y1 >= m_height || y2 < 0 || y2 >= m_height){
                std::cout << "[PowerX:ObjectArray] Error: Y Coordinates must be within range: [0, " << m_height-1 << "]: " << lineBuffer << std::endl;
                exit(4);
            }
            if(x1 == x2){ // vertical line
                if(y1 > y2) std::swap(y1, y2);
                for(int i = y1; i <= y2; ++i){
                    Cord newCord(x1, i);
                    if(allPreplaceCords.count(newCord) == 0){
                        allPreplaceCords.insert(newCord);
                        preplaceCord.insert(newCord);
                    }
                }
            }else if(y1 == y2){
                if(x1 > x2) std::swap(x1, x2);
                for(int i = x1; i <= x2; ++i){
                    Cord newCord(i, y1);
                    if(allPreplaceCords.count(newCord) == 0){
                        allPreplaceCords.insert(newCord);
                        preplaceCord.insert(newCord);
                    }
                }
            }else{
                std::cout << "[PowerX:ObjectArray] In Line mode, Cord(x1, y1) to Cord(x2, y2), only horizontal or vertical line accepted" << lineBuffer << std::endl;
                exit(4);
            }

        }else{
            std::cout << "[PowerX:ObjectArray] Error: Blockage format unrecognized: " << lineBuffer << std::endl;
            exit(4);
        }
    }


}

void ObjectArray::markPreplacedToCanvas(){
    for(std::unordered_map<SignalType, std::vector<Cord>>::const_iterator cit = preplacedCords.begin(); cit != preplacedCords.end(); ++cit){
        SignalType st = cit->first;
        for(const Cord &c : cit->second){
            setCanvas(c, st);
        }
    }
}



