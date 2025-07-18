//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/29/2025 23:10:24
//  Module Name:        c4Bump.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A data structure of storing C4 Bump under interposers.
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//
/////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:
#include <cassert>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <iostream>

// 2. Boost Library:

// 3. Texo Library:
#include "c4Bump.hpp"
#include "signalType.hpp"
#include "ballOut.hpp"
#include "objectArray.hpp"

C4PinCluster::C4PinCluster(): representation(Cord(-1, -1)), clusterSignalType(SignalType::EMPTY) {

}

C4PinCluster::C4PinCluster(const Cord &rep, const SignalType &sigType): representation(rep), clusterSignalType(sigType) {

}

C4Bump::C4Bump(): ObjectArray(0, 0), m_c4BallOut(nullptr),
    m_clusterPinCountWidth(-1), m_clusterPinCountHeight(-1), m_clusterPitchWidth(-1), m_clusterPitchHeight(-1),
    m_clusterCountWidth(-1), m_clusterCountHeight(-1), m_leftBorder(-1), m_rightBorder(-1), m_upBorder(-1), m_downBorder(-1) {

}

C4Bump::C4Bump(const std::string &fileName){
    std::ifstream file(fileName);
    assert(file.is_open());

    std::string lineBuffer;
    
    bool readC4 = false;
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

                this->canvas = std::vector<std::vector<SignalType>>(this->m_height, std::vector<SignalType>(this->m_width, SignalType::EMPTY));
                this->preplacedCords.clear();

                finishTechnologyParsing = true;
                continue;
            }

            
            if(splitLine[0] == "GRID_WIDTH") gridWidth = std::stoi(splitLine[2]);
            else if(splitLine[0] == "GRID_HEIGHT") gridHeight = std::stoi(splitLine[2]);
            else if(splitLine[0] == "PIN_WIDTH") pinWidth = std::stoi(splitLine[2]);
            else if(splitLine[0] == "PIN_HEIGHT") pinHeight = std::stoi(splitLine[2]);
            else if(splitLine[0] == "LAYERS") metalLayers = std::stoi(splitLine[2]);
            else{
                std::cout << "[PowerX:BalloutParser] Error: Unrecognizted Technology details: " << lineBuffer << std::endl;
                exit(4);
            }


            continue;
        }



        // variables collected during parsing for later processing
        BallOutRotation c4Rotation = BallOutRotation::EMPTY;
        
        if(splitLine[0] == "C4_START"){
            readC4 = true;
            continue;
        }
        if(!readC4) continue;
        
        if(splitLine[0] == "C4_END"){
            // process collected information

            if(m_c4BallOut == nullptr){
                std::cout << "[PowerX:BalloutParser] Error: BallOut file missing" << std::endl;
                abort();
            }

            // check if the basic attributes are set properly
            if(m_width <= 0){
                std::cout << "[PowerX:BalloutParser] Error: Basic attribute m_width not set or value(" << m_width << ") invalid" << std::endl;
            }
            if(m_height <= 0){
                std::cout << "[PowerX:BalloutParser] Error: Basic attribute m_height not set or value(" << m_height << ") invalid" << std::endl;
            }
            if(m_clusterPinCountWidth <= 0){
                std::cout << "[PowerX:BalloutParser] Error: Basic attribute m_ballWidth not set or value(" << m_clusterPinCountWidth << ") invalid" << std::endl;
            }   
            if(m_clusterPinCountHeight <= 0){
                std::cout << "[PowerX:BalloutParser] Error: Basic attribute m_clusterPinCountHeight not set or value(" << m_clusterPinCountHeight << ") invalid" << std::endl;
            }
            if((m_clusterPitchWidth <= 0) || (m_clusterPitchWidth < m_clusterPinCountWidth)){
                if(m_clusterPitchWidth <= 0){
                    std::cout << "[PowerX:BalloutParser] Error: Basic attribute m_clusterPitchWidth not set or value(" << m_clusterPitchWidth << ") invalid" << std::endl;
                }else{
                    std::cout << "[PowerX:BalloutParser] Error: Basic attribute m_clusterPitchWidth(" << m_clusterPitchWidth << ") ";
                    std::cout << "cannot be smaller than m_clusterPinCountWidth(" << m_clusterPinCountWidth << ")" << std::endl;
                }
            }
            if((m_clusterPitchHeight <= 0) || (m_clusterPitchHeight < m_clusterPinCountHeight)){
                if(m_clusterPitchHeight <= 0){
                    std::cout << "[PowerX:BalloutParser] Error: Basic attribute m_clusterPitchHeight not set or value(" << m_clusterPitchHeight << ") invalid" << std::endl;
                }else{
                    std::cout << "[PowerX:BalloutParser] Error: Basic attribute m_clusterPitchHeight(" << m_clusterPitchHeight << ") ";
                    std::cout << "cannot be smaller than m_clusterPinCountHeight(" << m_clusterPinCountHeight << ")" << std::endl;
                }
            }
            if(m_clusterCountWidth <= 0){
                std::cout << "[PowerX:BalloutParser] Error: Basic attribute m_clusterCountWidth not set or value(" << m_clusterCountWidth << ") invalid" << std::endl;
            }
            if(m_clusterCountHeight <= 0){
                std::cout << "[PowerX:BalloutParser] Error: Basic attribute m_ballCountHeight not set or value(" << m_clusterCountHeight << ") invalid" << std::endl;
            }
            if(m_leftBorder < 0){
                std::cout << "[PowerX:BalloutParser] Error: Basic attribute m_leftBorder not set or value(" << m_leftBorder << ") invalid" << std::endl;
            }
            if(m_rightBorder < 0){
                std::cout << "[PowerX:BalloutParser] Error: Basic attribute m_rightBorder not set or value(" << m_rightBorder << ") invalid" << std::endl;
            }
            if(m_upBorder < 0){
                std::cout << "[PowerX:BalloutParser] Error: Basic attribute m_upBorder not set or value(" << m_upBorder << ") invalid" << std::endl;
            }
            if(m_downBorder < 0){
                std::cout << "[PowerX:BalloutParser] Error: Basic attribute m_downBorder not set or value(" << m_downBorder << ") invalid" << std::endl;
            }

            // check if the basic attribute values set are consistant 
            int verifyCountWidth;
            if(m_clusterCountWidth != 1) verifyCountWidth = m_leftBorder + m_rightBorder + m_clusterPinCountWidth + m_clusterPitchWidth*(m_clusterCountWidth-1);
            else verifyCountWidth = m_leftBorder + m_rightBorder + m_clusterPinCountWidth;

            if(verifyCountWidth != m_width){
                std::cout << "[PowerX:BalloutParser] Error: Basic attribute mismatch m_width(" << m_width << ")";
                std::cout << ", Caluclated Width = " << verifyCountWidth <<  ")" << std::endl;
            }


            int verifyCountHeight;
            if(m_clusterPitchHeight != 1) verifyCountHeight = m_downBorder + m_upBorder + m_clusterPinCountHeight + m_clusterPitchHeight*(m_clusterCountHeight-1);
            else verifyCountHeight = m_downBorder + m_upBorder + m_clusterPinCountHeight;

            if(verifyCountHeight != m_height){
                std::cout << "[PowerX:BalloutParser] Error: Basic attribute mismatch m_height(" << m_height << ")";
                std::cout << ", Caluclated Height = " << verifyCountHeight <<  ")" << std::endl;
            }

            // variables are verified to be consistant

            if((c4Rotation == BallOutRotation::EMPTY) || (c4Rotation == BallOutRotation::UNKNOWN)) c4Rotation = BallOutRotation::R0;
            if(c4Rotation != BallOutRotation::R0){
                BallOut *tmp = m_c4BallOut;
                m_c4BallOut = new BallOut(*tmp, c4Rotation);
                assert(tmp != nullptr);
                delete tmp;
            }

            for(const SignalType &st : m_c4BallOut->allSignalTypes){
                this->allSignalTypes.insert(st);
                this->signalTypeToAllCords[st] = {};
                this->signalTypeToAllClusters[st] = {};
                this->preplacedCords[st] = {};
            }


            const int LLToRepresentationWidth = m_clusterPinCountWidth/2 + 1;
            const int LLToRepresentationHeight = m_clusterPinCountHeight/2 + 1;
            
            const int LLXInitValue = m_leftBorder;
            int LLX = LLXInitValue;
            int LLY = m_downBorder;
            
            for (int j = 0; j < m_clusterCountHeight; ++j) {
                for (int i = 0; i < m_clusterCountWidth; ++i) {
                    
                    SignalType st = m_c4BallOut->ballOutArray[j][i];

                    C4PinCluster *cluster = new C4PinCluster(Cord(LLX + LLToRepresentationWidth, LLY + LLToRepresentationHeight), st);
                    this->allClusters.push_back(cluster);
                    this->signalTypeToAllClusters[st].insert(cluster);

                    for(int n = 0; n < m_clusterPinCountHeight; ++n){
                        for(int m = 0; m < m_clusterPinCountWidth; ++m){
                            Cord pin(LLX + m, LLY + n);

                            cluster->pins.insert(pin);

                            this->signalTypeToAllCords[st].insert(pin);

                            this->cordToClusterMap[pin] = cluster;

                            this->preplacedCords[st].push_back(pin);
                            setCanvas(pin, st);
                        }
                    }
                    
                    LLX += m_clusterPitchWidth;
                }
                LLX = LLXInitValue;
                LLY += m_clusterPitchHeight;
            }

            file.close();
            break;
        }
        std::string initialWord = splitLine[0];
        std::transform(initialWord.begin(), initialWord.end(), initialWord.begin(), ::toupper);
        
        if(initialWord == "C4_WIDTH"){
            this->m_clusterPinCountWidth = stoi(splitLine[2]);
        }else if(initialWord == "C4_HEIGHT"){
            this->m_clusterPinCountHeight = stoi(splitLine[2]);
        }else if(initialWord == "C4_PITCH_WIDTH"){
            this->m_clusterPitchWidth = stoi(splitLine[2]);
        }else if(initialWord == "C4_PITCH_HEIGHT"){
            this->m_clusterPitchHeight = stoi(splitLine[2]);
        }else if(initialWord == "C4_COUNT_WIDTH"){
            this->m_clusterCountWidth = stoi(splitLine[2]);
        }else if(initialWord == "C4_COUNT_HEIGHT"){
            this->m_clusterCountHeight = stoi(splitLine[2]);
        }else if(initialWord == "C4_LEFT_BORDER"){
            this->m_leftBorder = stoi(splitLine[2]);
        }else if(initialWord == "C4_RIGHT_BORDER"){
            this->m_rightBorder = stoi(splitLine[2]);
        }else if(initialWord == "C4_DOWN_BORDER"){
            this->m_downBorder = stoi(splitLine[2]);
        }else if(initialWord == "C4_UP_BORDER"){
            this->m_upBorder = stoi(splitLine[2]);
        }else if(splitLine[0] == "include"){
            splitLine[1].erase(std::remove(splitLine[1].begin(), splitLine[1].end(), '"'), splitLine[1].end());
            this->m_c4BallOut = new BallOut(splitLine[1]);

        }else if(splitLine[0] == "ROTATION"){
            c4Rotation = convertToBallOutRotation(splitLine[2]);
            if((c4Rotation == BallOutRotation::EMPTY) || (c4Rotation == BallOutRotation::UNKNOWN)){
                std::cout << "[PowerX:BalloutParser] Warning: Unrecognizted rotation type: " << lineBuffer << " Default(R0) is used instead" << std::endl;
            }

        }else{
            std::cout << "[PowerX:BalloutParser] Warning: Unmatch string: " << lineBuffer << std::endl;
        }
    }
    assert(readTechnology);
    assert(finishTechnologyParsing);
}

C4Bump::~C4Bump(){
    if(m_c4BallOut != nullptr) delete m_c4BallOut;

    for(C4PinCluster *&cluster : allClusters){
        delete cluster;
    }
}
