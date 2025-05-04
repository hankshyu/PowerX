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
#include "microBump.hpp"

MicroBump::MicroBump(): ObjectArray(0, 0), m_interposerSizeRectangle(Rectangle(0, 0, 0, 0)) {

}

MicroBump::MicroBump(const std::string &fileName) {
    
    std::ifstream file(fileName);
    assert(file.is_open());

    std::string lineBuffer;

    bool readMicrobump = false;
    bool readTechnology = false;
    bool finishTechnologyParsing = false;

    len_t gridWidth = -1, gridHeight = -1;
    len_t pinWidth = -1, pinHeight = -1;
    len_t metalLayers = -1;

    while(std::getline(file, lineBuffer)){

        if (lineBuffer.empty()|| lineBuffer[0] == '#') continue; // whole line comment

        
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

        // parse TECHNOLOGY part
        if(!finishTechnologyParsing){
            if(!readTechnology){
                if(lineBuffer == "TECHNOLOGY_BEGIN") readTechnology = true;
                continue;
            }

            if(lineBuffer == "TECHNOLOGY_END"){
                assert(gridWidth != -1);
                assert(gridHeight != -1);
                assert(pinWidth == (gridWidth + 1));
                assert(pinHeight == (gridHeight + 1));
                assert(metalLayers >= 2);

                this->m_width = pinWidth;
                this->m_height = pinHeight;
                m_interposerSizeRectangle = Rectangle(0, 0,((pinWidth > 1)? pinWidth-1 : 0), ((pinHeight > 1)? pinHeight-1 : 0));


                this->canvas = std::vector<std::vector<SignalType>>(this->m_height, std::vector<SignalType>(this->m_width, SignalType::EMPTY));

                finishTechnologyParsing = true;
                continue;
            }

            
            if(splitLine[0] == "GRID_WIDTH") gridWidth = std::stoi(splitLine[2]);
            else if(splitLine[0] == "GRID_HEIGHT") gridHeight = std::stoi(splitLine[2]);
            else if(splitLine[0] == "PIN_WIDTH") pinWidth = std::stoi(splitLine[2]);
            else if(splitLine[0] == "PIN_HEIGHT") pinHeight = std::stoi(splitLine[2]);
            else if(splitLine[0] == "LAYERS") metalLayers = std::stoi(splitLine[2]);
            else{
                std::cout << "[PowerX:MicroBumpParser] Error: Unrecognizted Technology details: " << lineBuffer << std::endl;
                exit(4);
            }

            continue;
        }

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

            BallOut *prototype = nullptr;
            for(BallOut *&bo : m_allBallouts[0]){
                if(bo->getName() == splitLine[1]){
                    prototype = bo;
                    break;
                } 
            }

            if(prototype == nullptr){
                std::cout << "[PowerX:PinParser] Error: Unknown chiplet " << lineBuffer << std::endl;
                exit(4);
            }

            BallOutRotation rotation = convertToBallOutRotation(splitLine[3]);
            if((rotation == BallOutRotation::EMPTY) || (rotation == BallOutRotation::UNKNOWN)){
                std::cout << "[PowerX:PinParser] Error: Unknown Rotation " << lineBuffer << std::endl;
                exit(4);
            }else if(rotation != BallOutRotation::R0){
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

            allSignalTypes.insert(prototype->allSignalTypes.begin(), prototype->allSignalTypes.end());
            for(const SignalType &st : prototype->allSignalTypes){
                if(signalTypeToAllCords.count(st) == 0) signalTypeToAllCords[st] = {};
                if(signalTypeToInstances.count(st) == 0) signalTypeToInstances[st] = {};
                if(preplacedCords.count(st) == 0) this->preplacedCords[st] = {};
            }

            instanceToRectangleMap[splitLine[2]] = instanceRect;
            instanceToBallOutMap[splitLine[2]] = prototype;
            instanceToRotationMap[splitLine[2]] = rotation;

            for(int j = 0; j < prototype->getBallOutHeight(); ++j){
                for(int i = 0; i < prototype->getBallOutWidth(); ++i){
                    Cord transformedCord(xDiff + i, yDiff + j);
                    SignalType transformedCordType = prototype->ballOutArray[j][i];

                    setCanvas(transformedCord, transformedCordType);
                    this->preplacedCords[transformedCordType].push_back(transformedCord);

                    signalTypeToAllCords[transformedCordType].insert(transformedCord);
                    signalTypeToInstances[transformedCordType].insert(splitLine[2]);
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