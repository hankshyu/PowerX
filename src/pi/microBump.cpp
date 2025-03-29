//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/27/2025 09:47:32
//  Module Name:        uBump.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A data structure of storing micro Bump on interposers
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//
/////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:
#include <cassert>
#include <unordered_map>
#include <string>
#include <ostream>
#include <fstream>

// 2. Boost Library:


// 3. Texo Library:
#include "rectangle.hpp"
#include "technology.hpp"
#include "ballOut.hpp"
#include "pinMap.hpp"
#include "microBump.hpp"

MicroBump::MicroBump(): PinMap("", 0, 0), m_interposerSizeRectangle(Rectangle(0, 0, 0, 0)) {

}

MicroBump::MicroBump(const std::string &fileName) {
    std::ifstream file(fileName);
    assert(file.is_open());

    bool readInterposer = false;
    bool readMicrobump = false;
    bool processIncludes = false;

    std::string buffer;
    std::string lineBuffer;
    while(std::getline(file, lineBuffer)){
        if (lineBuffer.empty()|| lineBuffer.find("#") == 0) continue; // whole line comment
        
        size_t comment_pos = lineBuffer.find_first_of("#");
        if (comment_pos != std::string::npos) {
                lineBuffer = lineBuffer.substr(0, comment_pos);  // Remove everything after "#"
        }

        // process line: std::cout << lineBuffer << std::endl;
        std::vector<std::string> splitLine;
        std::istringstream stream(lineBuffer);
        std::string wordBuffer;
    
        while (stream >> wordBuffer) { // Extract words by spaces and newlines
            splitLine.push_back(wordBuffer);
        }

        if(splitLine[0] == "INTERPOSER"){
            this->m_name = splitLine[1];
            this->m_pinMapWidth = std::stoi(splitLine[2]);
            this->m_pinMapHeight = std::stoi(splitLine[3]);
            
            m_interposerSizeRectangle = Rectangle(0, 0,((m_pinMapWidth > 1)? m_pinMapWidth-1 : 0), ((m_pinMapHeight > 1)? m_pinMapHeight-1 : 0));
            
            readInterposer = true;
        }

        if(!readInterposer) continue;
        
        if(splitLine[0] == "MICROBUMP_START"){
            readMicrobump = true;
            continue;
        }
        if(!readMicrobump) continue;
        
        if(splitLine[0] == "MICROBUMP_END"){
            file.close();
            break;
        }

        if(splitLine[0] == "include"){
            processIncludes = false;
            splitLine[1].erase(std::remove(splitLine[1].begin(), splitLine[1].end(), '"'), splitLine[1].end());
            BallOut *newBallOut = new BallOut(splitLine[1]);

            bool repeatedBallOut = false;
            for(const BallOut *const &bo : this->m_allBallouts[0]){
                if(bo->getName() == newBallOut->getName()){
                    std::cout << "[PowerX:PinParser] Warning: Repeated ballout included: " << bo->getName() << std::endl;
                    repeatedBallOut = true;
                }
            }
            if(!repeatedBallOut){
                m_allBallouts[0].push_back(newBallOut);
                allSignalTypes.insert(newBallOut->allSignalTypes.begin(), newBallOut->allSignalTypes.end());
            }
            
        }else if(splitLine[0] == "CHIPLET"){

            if(!processIncludes){
                for(const SignalType &st : this->allSignalTypes){
                    std::unordered_map<SignalType, std::unordered_set<Cord>>::const_iterator cit = signalTypeToAllCords.find(st);
                    if(cit == signalTypeToAllCords.end()) signalTypeToAllCords[st] = {}; // initialize
                }
            }

            BallOut *prototype = nullptr;
            for(BallOut *&bo : m_allBallouts[0]){
                if(bo->getName() == splitLine[1]){
                    prototype = bo;
                    break;
                } 
            }

            if(prototype == nullptr){
                std::cout << "[PowerX:PinParser] Error: Unknown chiplet " << lineBuffer << std::endl;
                abort();
            }

            BallOutRotation rotation = convertToBallOutRotation(splitLine[3]);
            if((rotation == BallOutRotation::EMPTY) || (rotation == BallOutRotation::UNKNOWN)){
                std::cout << "[PowerX:PinParser] Error: Unknown Rotation " << lineBuffer << std::endl;
                abort();
            }
            if(rotation != BallOutRotation::R0){
                prototype = new BallOut(*prototype, rotation);
                m_allBallouts[rotation].push_back(prototype);
            }

            splitLine[4].erase(std::remove(splitLine[4].begin(), splitLine[4].end(), '('), splitLine[4].end());
            splitLine[4].erase(std::remove(splitLine[4].begin(), splitLine[4].end(), ','), splitLine[4].end());
            len_t xDiff = std::stoi(splitLine[4]);

            splitLine[5].erase(std::remove(splitLine[5].begin(), splitLine[5].end(), ')'), splitLine[5].end());
            len_t yDiff = std::stoi(splitLine[5]);

            len_t instRectWidth = (prototype->getBallOutWidth() > 1)? prototype->getBallOutWidth() - 1 : 0;
            len_t instRectHeight = (prototype->getBallOutHeight() > 1) ? prototype->getBallOutHeight() - 1 : 0;
            Rectangle instanceRect(xDiff, yDiff, xDiff + instRectWidth, yDiff + instRectHeight);
            assert(rec::isContained(this->m_interposerSizeRectangle, instanceRect));

            instanceToRectangleMap[splitLine[2]] = instanceRect;
            instanceToBallOutMap[splitLine[2]] = prototype;
            instanceToRotationMap[splitLine[2]] = rotation;

            for(int j = 0; j < prototype->getBallOutHeight(); ++j){
                for(int i = 0; i < prototype->getBallOutWidth(); ++i){
                    Cord transformedCord(xDiff + i, yDiff + j);
                    SignalType transformedCordType = prototype->ballOutArray[j][i];

                    cordToSignalTypeMap[transformedCord] = transformedCordType;
                    signalTypeToAllCords[transformedCordType].insert(transformedCord);
                }
            }

        }else{
            std::cout << "[PowerX:PinParser] Unmatch string: " << lineBuffer << std::endl;
        }
    }
}

MicroBump::~MicroBump(){

    for(int i = 0; i < BALLOUT_ROTATION_COUNT; ++i){
        for(int j = 0; j < m_allBallouts[i].size(); ++j){
            delete m_allBallouts[i][j];
        }
    }
    
}