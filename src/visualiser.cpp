//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/14/2025 19:19:08
//  Module Name:        visualiser.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        All classes that writes out to a certain file is presented
//                      Use render program to render the output file for visualization purposes
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//
/////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:
#include <iostream>
#include <fstream>

// 2. Boost Library:

// 3. Texo Library:
#include "visualiser.hpp"
#include "cord.hpp"


bool visualiseBumpMap(const BumpMap &bumpMap, const Technology &tch, const std::string &filePath){

    std::ofstream ofs(filePath, std::ios::out);

    assert(ofs.is_open());
    if(!ofs.is_open()) return false;
    ofs << "BUMPMAP VISUALISATION" << std::endl;

    len_t pitch = tch.getMicrobumpPitch();
    len_t pinRadius = tch.getMicrobumpRadius();

    len_t pinOutWidth = bumpMap.getBumpCountWidth();
    len_t pinOutHeight = bumpMap.getBumpCountHeight();
    
    ofs << bumpMap.m_name << " " << pinOutWidth << " " << pinOutHeight << std::endl;
    ofs << pinOutWidth * pitch << " " <<  pinOutHeight * pitch << std::endl;

    ofs << "PINS" << " " << pinOutWidth * pinOutHeight << std::endl;
    std::map<Cord, bumpType>::const_iterator it;
    for(int j = 0; j < pinOutHeight; ++j){
        for(int i = 0; i < pinOutWidth; ++i){
            it = bumpMap.m_bumpMap.find(Cord(i, j));
            len_t centreX = pitch/2 + i*pitch;
            len_t centreY = pitch/2 + j*pitch;
            
            if(it == bumpMap.m_bumpMap.end()){
                ofs << centreX << " " << centreY << " " << pinRadius << " " << "SIG" << std::endl;
            }else{
                ofs << centreX << " " << centreY << " " << pinRadius << " " << it->second << std::endl;
            }
        }
    }

    ofs.close();
    return true;
}

bool visualisePinout(const Pinout &pinout, const Technology &tch, const std::string &filePath){
    
    std::ofstream ofs(filePath, std::ios::out);
    
    assert(ofs.is_open());
    if(!ofs.is_open()) return false;
    ofs << "PINOUT VISUALISATION" << std::endl;

    len_t pitch = tch.getMicrobumpPitch();
    len_t pinRadius = tch.getMicrobumpRadius();

    len_t pinOutWidth = pinout.m_pinCountWidth;
    len_t pinOutHeight = pinout.m_pinCountHeight;


    ofs << pinout.m_name << " " << pinOutWidth << " " << pinOutHeight << std::endl;
    ofs << pinOutWidth * pitch << " " <<  pinOutHeight * pitch << std::endl;
    ofs << "CHIPLETS" << " " << pinout.m_instanceToType.size() << std::endl;
    
    for(std::map<std::string, chipletType>::const_iterator it = pinout.m_instanceToType.begin(); it != pinout.m_instanceToType.end(); ++it){
        Cord instanceLL = pinout.m_instanceToLL.at(it->first);
        len_t llx = instanceLL.x() * pitch;
        len_t lly = instanceLL.y() * pitch;
        len_t width = pinOutWidth * pitch;
        len_t height = pinOutHeight * pitch;
        
        ofs << it->first << " " << it->second << " " << llx << " " << lly << " " << width << " " << height << std::endl;
    }

    ofs << "PINS" << " " << pinOutWidth * pinOutHeight << std::endl;
    std::unordered_map<Cord, bumpType>::const_iterator it;
    for(int j = 0; j < pinOutHeight; ++j){
        for(int i = 0; i < pinOutWidth; ++i){
            it = pinout.m_cordToType.find(Cord(i, j));
            len_t centreX = pitch/2 + i*pitch;
            len_t centreY = pitch/2 + j*pitch;
            
            if(it == pinout.m_cordToType.end()){
                ofs << centreX << " " << centreY << " " << pinRadius << " " << "SIG" << std::endl;
            }else{
                ofs << centreX << " " << centreY << " " << pinRadius << " " << it->second << std::endl;
            }
        }
    }

    ofs.close();
    return true;
}


bool visualiseBallout(const Ballout &ballout, const Technology &tch, const std::string &filePath){
    
    std::ofstream ofs(filePath, std::ios::out);
    
    assert(ofs.is_open());
    if(!ofs.is_open()) return false;
    ofs << "BALLOUT VISUALISATION" << std::endl;

    len_t pitch = tch.getMicrobumpPitch();
    len_t pinRadius = tch.getMicrobumpRadius();

    len_t pinOutWidth = ballout.m_pinCountWidth;
    len_t pinOutHeight = ballout.m_pinCountHeight;


    ofs << ballout.m_name << " " << pinOutWidth << " " << pinOutHeight << std::endl;
    ofs << pinOutWidth * pitch << " " <<  pinOutHeight * pitch << std::endl;
    


    ofs << "PINS" << " " << pinOutWidth * pinOutHeight << std::endl;
    std::unordered_map<Cord, ballType>::const_iterator it;
    for(int j = 0; j < pinOutHeight; ++j){
        for(int i = 0; i < pinOutWidth; ++i){
            it = ballout.m_cordToBallType.find(Cord(i, j));
            len_t centreX = pitch/2 + i*pitch;
            len_t centreY = pitch/2 + j*pitch;
            
            if(it == ballout.m_cordToBallType.end()){
                ofs << centreX << " " << centreY << " " << pinRadius << " " << "SIG" << std::endl;
            }else{
                ofs << centreX << " " << centreY << " " << pinRadius << " " << it->second << std::endl;
            }
        }
    }

    ofs.close();
    return true;
}
