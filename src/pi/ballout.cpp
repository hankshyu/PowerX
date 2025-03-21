//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/21/2025 16:46:04
//  Module Name:        ballout.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A data structure for C4 bumpball storage and lookups
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//
//////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:
#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cassert>
#include <algorithm>
// 2. Boost Library:


// 3. Texo Library:
#include "ballout.hpp"
#include "bumpMap.hpp"
#include "cord.hpp"

Ballout::Ballout(): m_name(""), m_pinCountWidth(-1), m_pinCountHeight(-1), 
    m_ballWidth(-1), m_ballHeight(-1), m_ballCountWidth(-1), m_ballCountHeight(-1),
    m_ballPitchWidth(-1), m_ballPitchHeight(-1),
    m_leftBorder(-1), m_rightBorder(-1), m_upBorder(-1), m_downBorder(-1) {

}


Ballout::Ballout(const std::string &fileName): m_name(""), m_pinCountWidth(-1), m_pinCountHeight(-1), 
m_ballWidth(-1), m_ballHeight(-1), m_ballCountWidth(-1), m_ballCountHeight(-1),
m_ballPitchWidth(-1), m_ballPitchHeight(-1),
m_leftBorder(-1), m_rightBorder(-1), m_upBorder(-1), m_downBorder(-1) {

    std::ifstream file(fileName);
    assert(file.is_open());

    bool readInterposer = false;
    bool readC4 = false;

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
            this->m_pinCountWidth = std::stoi(splitLine[2]);
            this->m_pinCountHeight = std::stoi(splitLine[3]);
            readInterposer = true;
        }

        if(!readInterposer) continue;
        
        if(splitLine[0] == "C4_START"){
            readC4 = true;
            continue;
        }
        if(!readC4) continue;
        
        if(splitLine[0] == "C4_END"){
            file.close();
            break;
        }
        std::string initialWord;
        std::transform(splitLine[0].begin(), splitLine[0].end(), initialWord.begin(), ::toupper);

        if(initialWord == "C4_WIDTH"){
            this->m_ballWidth = stoi(splitLine[2]);
        }else if(initialWord == "C4_HEIGHT"){
            this->m_ballHeight = stoi(splitLine[2]);
        }else if(initialWord == "C4_PITCH_WIDTH"){
            this->m_ballPitchWidth = stoi(splitLine[2]);
        }else if(initialWord == "C4_PITCH_HEIGHT"){
            this->m_ballPitchHeight = stoi(splitLine[2]);
        }else if(initialWord == "C4_COUNT_WIDTH"){
            this->m_ballCountWidth = stoi(splitLine[2]);
        }else if(initialWord == "C4_COUNT_HEIGHT"){
            this->m_ballCountHeight = stoi(splitLine[2]);
        }else if(initialWord == "C4_LEFT_BORDER"){
            this->m_leftBorder = stoi(splitLine[2]);
        }else if(initialWord == "C4_RIGHT_BORDER"){
            this>m_rightBorder = stoi(splitLine[2]);
        }else if(initialWord == "C4_DOWN_BORDER"){
            this->m_downBorder = stoi(splitLine[2]);
        }else if(initialWord == "C4_UP_BORDER"){
            this->m_upBorder = stoi(splitLine[2]);
        }else if(splitLine[0] == "include"){

            // check if the basic attributes are set properly
            if(m_pinCountWidth <= 0){
                std::cout << "[PowerX:BalloutParser] Error: Basic attribute m_pinCountWidth not set or value(" << m_pinCountWidth << ") invalid" << std::endl;
            }
            if(m_pinCountHeight <= 0){
                std::cout << "[PowerX:BalloutParser] Error: Basic attribute m_pinCountHeight not set or value(" << m_pinCountHeight << ") invalid" << std::endl;
            }
            if(m_ballWidth <= 0){
                std::cout << "[PowerX:BalloutParser] Error: Basic attribute m_ballWidth not set or value(" << m_ballWidth << ") invalid" << std::endl;
            }   
            if(m_ballHeight <= 0){
                std::cout << "[PowerX:BalloutParser] Error: Basic attribute m_ballHeight not set or value(" << m_ballHeight << ") invalid" << std::endl;
            }
            if((m_ballPitchWidth <= 0) || (m_ballPitchWidth < m_ballWidth)){
                if(m_ballPitchWidth <= 0){
                    std::cout << "[PowerX:BalloutParser] Error: Basic attribute m_ballPitchWidth not set or value(" << m_ballPitchWidth << ") invalid" << std::endl;
                }else{
                    std::cout << "[PowerX:BalloutParser] Error: Basic attribute m_ballPitchWidth(" << m_ballPitchWidth << ") ";
                    std::cout << "cannot be smaller than m_ballWidth(" << m_ballWidth << ")" << std::endl;
                }
            }
            if((m_ballPitchHeight <= 0) || (m_ballPitchHeight < m_ballHeight)){
                if(m_ballPitchHeight <= 0){
                    std::cout << "[PowerX:BalloutParser] Error: Basic attribute m_ballPitchHeight not set or value(" << m_ballPitchHeight << ") invalid" << std::endl;
                }else{
                    std::cout << "[PowerX:BalloutParser] Error: Basic attribute m_ballPitchHeight(" << m_ballPitchHeight << ") ";
                    std::cout << "cannot be smaller than m_ballHeight(" << m_ballHeight << ")" << std::endl;
                }
            }
            if(m_ballCountWidth <= 0){
                std::cout << "[PowerX:BalloutParser] Error: Basic attribute m_ballCountWidth not set or value(" << m_ballCountWidth << ") invalid" << std::endl;
            }
            if(m_ballCountHeight <= 0){
                std::cout << "[PowerX:BalloutParser] Error: Basic attribute m_ballCountHeight not set or value(" << m_ballCountHeight << ") invalid" << std::endl;
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
            if(m_ballCountWidth != 1) verifyCountWidth = m_leftBorder + m_rightBorder + m_ballWidth + m_ballPitchWidth*(m_ballCountWidth-1);
            else verifyCountWidth = m_leftBorder + m_rightBorder + m_ballWidth;

            if(verifyCountWidth != m_pinCountWidth){
                std::cout << "[PowerX:BalloutParser] Error: Basic attribute mismatch m_pinCountWidth(" << m_pinCountWidth << ")";
                std::cout << ", Caluclated Width = " << verifyCountWidth <<  ")" << std::endl;
            }


            int verifyCountHeight;
            if(m_ballCountHeight != 1) verifyCountHeight = m_downBorder + m_upBorder + m_ballHeight + m_ballPitchHeight*(m_ballCountHeight-1);
            else verifyCountHeight = m_downBorder + m_upBorder + m_ballHeight;

            if(verifyCountHeight != m_pinCountHeight){
                std::cout << "[PowerX:BalloutParser] Error: Basic attribute mismatch m_pinCountHeight(" << m_pinCountHeight << ")";
                std::cout << ", Caluclated Height = " << verifyCountHeight <<  ")" << std::endl;
            }

            // pass verification! read pinout files and construct data structure

        
            splitLine[1].erase(std::remove(splitLine[1].begin(), splitLine[1].end(), '"'), splitLine[1].end());
            BumpMap bm(splitLine[1]);
            
            assert(bm.getBumpCountWidth == this->m_ballCountWidth);
            assert(bm.getBumpCountHeight == this->m_ballCountHeight);
            
            const std::map<Cord, bumpType> &bumpMap = bm.getBumpMap();

            m_ballMap.resize(m_ballCountHeight, std::vector<Cluster>(m_ballCountWidth));


            const int LLToRepresentationWidth = m_ballWidth/2 + 1;
            const int LLToRepresentationHeight = m_ballHeight/2 + 1;
            
            const int LLXInitValue = m_leftBorder + 1;
            int LLX = LLXInitValue;
            int LLY = m_downBorder + 1;
            
            for (int j = 0; j < m_ballCountHeight; ++j) {
                for (int i = 0; i < m_ballCountWidth; ++i) {
                    Cluster &c = m_ballMap[i][j];

                    ballType ballType = bumpMap.at(Cord(i, j));
                    m_ballTypeToClusters[ballType].insert(c);

                    c.ballMapX = i;
                    c.ballmapY = j;
                    c.clusterType = ballType;
                    c.lowerLeftBall = Cord(LLX, LLY);
                    c.representation = Cord(LLX + LLToRepresentationWidth, LLY + LLToRepresentationHeight);
                    c.ballouts.clear();
                    for(int n = 0; n < m_ballHeight; ++n){
                        for(int m = 0; m < m_ballWidth; ++m){
                            Cord pin(LLX + m, LLY + n);
                            c.ballouts.insert(pin);
                            m_cordToBallType[pin] = ballType;
                            m_cordToCluster[pin] = c;
                        }
                    }
                    
                    LLX += m_ballPitchWidth;
                }
                LLX = LLXInitValue;
                LLY += m_ballPitchHeight;
            }


        }else{
            std::cout << "[PowerX:BalloutParser] Unmatch string: " << lineBuffer << std::endl;
        }
    }

}