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
#include <ostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

// 2. Boost Library:

// 3. Texo Library:
#include "cord.hpp"
#include "ballOut.hpp"

std::ostream& operator<<(std::ostream& os, BallOutRotation bor) {
    return os << to_string(bor);
}


inline BallOutRotation convertToBallOutRotation(const std::string& str) {
    std::string input = str;
    std::transform(input.begin(), input.end(), input.begin(), [](unsigned char c){ return std::toupper(c); });

    // Remove optional prefix
    const std::string prefix1 = "ROTATION::";
    const std::string prefix2 = "ROTATION_";
    const std::string prefix3 = "BALLOUT_ROTATION::";
    const std::string prefix4 = "BALLOUT_ROTATION_";

    if(input.rfind(prefix1, 0) == 0){
        input = input.substr(prefix1.length());
    }else if(input.rfind(prefix2, 0) == 0){
        input = input.substr(prefix2.length());
    }else if(input.rfind(prefix3, 0) == 0){
        input = input.substr(prefix3.length());
    }else if(input.rfind(prefix4, 0) == 0){
        input = input.substr(prefix4.length());
    }

    static const std::unordered_map<std::string, BallOutRotation> strToRotationMap = {
        {"EMPTY", BallOutRotation::EMPTY},
        {"R0", BallOutRotation::R0}, {"0", BallOutRotation::R0},
        {"R90", BallOutRotation::R90}, {"90", BallOutRotation::R90},
        {"R180", BallOutRotation::R180}, {"180", BallOutRotation::R180},
        {"R270", BallOutRotation::R270}, {"270", BallOutRotation::R270},
        {"UNKNOWN", BallOutRotation::UNKNOWN}
    };

    std::unordered_map<std::string, BallOutRotation>::const_iterator cit = strToRotationMap.find(input);
    return (cit != strToRotationMap.end()) ? cit->second : BallOutRotation::UNKNOWN;  // fallback default
}

BallOut::BallOut(): m_name(""), m_ballOutWidth(0), m_ballOutHeight(0), m_rotation(BallOutRotation::EMPTY) {

}

BallOut::BallOut(const std::string &filePath): m_rotation(BallOutRotation::R0) {
    
    std::ifstream file(filePath);
    assert(file.is_open());
    
    std::string buffer;

    file >> buffer >> this->m_name >> this->m_ballOutWidth >> this->m_ballOutHeight;
    ballOutArray.resize(this->m_ballOutHeight, std::vector<SignalType>(this->m_ballOutWidth, SignalType::EMPTY));
        
    for(int j = 0; j < this->m_ballOutHeight; ++j){
        for(int i = 0; i < this->m_ballOutWidth; ++i){

            file >> buffer;

            // parse the string, keep only substring after ','
            size_t position = buffer.find(',');
            assert(position != std::string::npos);
            
            Cord pinLocation = CSVCellToCord(buffer.substr(0, position));

            #ifndef NDEBUG
                if(pinLocation != Cord(i, j)){
                    std::cout << "[PowerX:BallOutParser] Error: Discontinuous CSV Cell position value: " << buffer.substr(0, position)  << std::endl;
                    abort();

                }
            #endif


            SignalType signaltp = convertToSignalType(buffer.substr(position + 1));

            #ifndef NDEBUG
                if((signaltp == SignalType::EMPTY) || ((signaltp == SignalType::UNKNOWN))){
                    std::cout << "[PowerX:BallOutParser] Error: Unknown Signal Type: " << buffer.substr(position + 1)  << std::endl;
                    abort();
                }
            #endif

            pinLocation.y(this->m_ballOutHeight - pinLocation.y() - 1);
            ballOutArray[pinLocation.x()][pinLocation.y()] = signaltp;
            // Cord(i, this->m_ballOutHeight - j - 1)
            std::unordered_set<SignalType>::const_iterator cit = allSignalTypes.find(signaltp);
            
            if(cit == allSignalTypes.end()){
                allSignalTypes.insert(signaltp);
                SignalTypeToAllCords[signaltp] = {pinLocation};
            }else{
                SignalTypeToAllCords[signaltp].insert(pinLocation);
            }
        }
    }

    file.close();
}

BallOut::BallOut(const BallOut &ref, enum BallOutRotation rotation) :m_name(ref.m_name), m_rotation(rotation), allSignalTypes(ref.allSignalTypes) {

    for(const SignalType &st : this->allSignalTypes){
        this->SignalTypeToAllCords[st] = {};
    }

    switch (rotation) {
        case BallOutRotation::R90:{
            this->m_ballOutWidth = ref.m_ballOutHeight;
            this->m_ballOutHeight = ref.m_ballOutWidth;
            this->ballOutArray.resize(this->m_ballOutHeight, std::vector<SignalType>(this->m_ballOutWidth, SignalType::EMPTY));

            for(int j = 0; j < this->m_ballOutHeight; ++j){
                for(int i = 0; i < this->m_ballOutWidth; ++i){
                    SignalType sid = ref.ballOutArray[this->m_ballOutHeight - j - 1][i];
                    this->ballOutArray[i][j] = sid;
                    this->SignalTypeToAllCords[sid].insert(Cord(i, j));
                }
            }

            break;
        }

        case BallOutRotation::R180:{
            this->m_ballOutWidth = ref.m_ballOutWidth;
            this->m_ballOutHeight = ref.m_ballOutHeight;
            this->ballOutArray.resize(this->m_ballOutHeight, std::vector<SignalType>(this->m_ballOutWidth, SignalType::EMPTY));

            for(int j = 0; j < this->m_ballOutHeight; ++j){
                for(int i = 0; i < this->m_ballOutWidth; ++i){
                    SignalType sid = ref.ballOutArray[i][this->m_ballOutHeight - j - 1];
                    this->ballOutArray[i][j] = sid;
                    this->SignalTypeToAllCords[sid].insert(Cord(i, j));
                }
            }
            break;

        }

        case BallOutRotation::R270:{
            this->m_ballOutWidth = ref.m_ballOutHeight;
            this->m_ballOutHeight = ref.m_ballOutWidth;
            this->ballOutArray.resize(this->m_ballOutHeight, std::vector<SignalType>(this->m_ballOutWidth, SignalType::EMPTY));

            for(int j = 0; j < this->m_ballOutHeight; ++j){
                for(int i = 0; i < this->m_ballOutWidth; ++i){
                    SignalType sid = ref.ballOutArray[j][this->m_ballOutWidth - i - 1];
                    this->ballOutArray[i][j] = sid;
                    this->SignalTypeToAllCords[sid].insert(Cord(i, j));
                }
            }
            break;

        }

        default:{ // case BallOutRotation::R0:
            this->m_ballOutWidth = ref.m_ballOutWidth;
            this->m_ballOutHeight = ref.m_ballOutHeight;
            this->ballOutArray.resize(this->m_ballOutHeight, std::vector<SignalType>(this->m_ballOutWidth, SignalType::EMPTY));

            for(int j = 0; j < this->m_ballOutHeight; ++j){
                for(int i = 0; i < this->m_ballOutWidth; ++i){
                    SignalType sid = ref.ballOutArray[i][j];
                    this->ballOutArray[i][j] = sid;
                    this->SignalTypeToAllCords[sid].insert(Cord(i, j));
                }
            }
            break;

        }
    }
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
