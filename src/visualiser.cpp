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

#include "cornerStitching.hpp"
#include "tile.hpp"


bool visualiseBallOut(const BallOut &ballOut, const Technology &tch, const std::string &filePath){

    std::ofstream ofs(filePath, std::ios::out);

    assert(ofs.is_open());
    if(!ofs.is_open()) return false;
    ofs << "BUMPMAP VISUALISATION" << std::endl;

    len_t pitch = tch.getMicrobumpPitch();
    len_t pinRadius = tch.getMicrobumpRadius();

    len_t ballOutWidth = ballOut.m_ballOutWidth;
    len_t ballOutHeight = ballOut.m_ballOutHeight;
    
    ofs << ballOut.m_name << " " << ballOutWidth << " " << ballOutHeight << std::endl;
    ofs << ballOutWidth * pitch << " " <<  ballOutHeight * pitch << std::endl;

    ofs << "PINS" << " " << ballOutWidth * ballOutHeight << std::endl;
    for(int j = 0; j < ballOutHeight; ++j){
        for(int i = 0; i < ballOutWidth; ++i){
            len_t centreX = pitch/2 + i*pitch;
            len_t centreY = pitch/2 + j*pitch;
            
            ofs << centreX << " " << centreY << " " << pinRadius << " " << ballOut.ballOutArray[j][i] << std::endl;
        }
    }

    ofs.close();
    return true;
}


bool visualiseUBump(const UBump &uBump, const Technology &tch, const std::string &filePath){
    
    std::ofstream ofs(filePath, std::ios::out);
    
    assert(ofs.is_open());
    if(!ofs.is_open()) return false;
    ofs << "UBUMP VISUALISATION" << std::endl;

    len_t pitch = tch.getMicrobumpPitch();
    len_t pinRadius = tch.getMicrobumpRadius();

    len_t pinOutWidth = uBump.m_pinMapWidth;
    len_t pinOutHeight = uBump.m_pinMapHeight;

    ofs << uBump.m_name << " " << pinOutWidth << " " << pinOutHeight << std::endl;
    ofs << pinOutWidth * pitch << " " <<  pinOutHeight * pitch << std::endl;
    
    ofs << "CHIPLETS" << " " << uBump.instanceToRectangleMap.size() << std::endl;

    for(std::unordered_map<std::string, Rectangle>::const_iterator cit = uBump.instanceToRectangleMap.begin(); cit!= uBump.instanceToRectangleMap.end(); ++cit){
        Rectangle chipletBB = cit->second;
        ofs << cit->first << " " << uBump.instanceToBallOutMap.at(cit->first)->getName() << " ";
        ofs << pitch*rec::getXL(chipletBB) << " " << pitch*rec::getYL(chipletBB) << " ";
        ofs << pitch*(rec::getWidth(chipletBB) + 1)<< " " << pitch*(rec::getHeight(chipletBB) + 1) << std::endl;
    }


    ofs << "PINS" << " " << pinOutWidth * pinOutHeight << std::endl;
    std::unordered_map<Cord, SignalType>::const_iterator it;
    for(int j = 0; j < pinOutHeight; ++j){
        for(int i = 0; i < pinOutWidth; ++i){
            it = uBump.cordToSignalTypeMap.find(Cord(i, j));
            len_t centreX = pitch/2 + i*pitch;
            len_t centreY = pitch/2 + j*pitch;
            
            if(it == uBump.cordToSignalTypeMap.end()){
                ofs << centreX << " " << centreY << " " << pinRadius << " " << "EMPTY" << std::endl;
            }else{
                ofs << centreX << " " << centreY << " " << pinRadius << " " << it->second << std::endl;
            }
        }
    }

    ofs.close();
    return true;
}

/*

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
*/
bool visualiseCornerStitching(const CornerStitching &cs, const std::string &filePath){
    
    std::ofstream ofs(filePath, std::ios::out);

    assert(ofs.is_open());
    if(!ofs.is_open()) return false;

	std::unordered_set<Tile *> allTiles;
	cs.collectAllTiles(allTiles);

	// write out the total tile numbers
	ofs << allTiles.size() << std::endl;
	// write the chip contour 
	ofs << cs.mCanvasWidth << " " << cs.mCanvasHeight << std::endl;
	// Then start to write info for each file
	for(Tile *const &tile : allTiles){
		unsigned long long tileHash;
		ofs << *tile << std::endl;

		Tile *rtTile = tile->rt;
		ofs << "rt: ";
		if(rtTile == nullptr){
			ofs << "nullptr" << std::endl; 
		}else{
			ofs << *rtTile << std::endl;
		}

		Tile *trTile = tile->tr;
		ofs << "tr: ";
		if(trTile == nullptr){
			ofs << "nullptr" << std::endl; 
		}else{
			ofs << *trTile << std::endl;
		}

		Tile *blTile = tile->bl;
		ofs << "bl: ";
		if(blTile == nullptr){
			ofs << "nullptr" << std::endl; 
		}else{
			ofs << *blTile << std::endl;
		}

		Tile *lbTile = tile->lb;
		ofs << "lb: ";
		if(lbTile == nullptr){
			ofs << "nullptr" << std::endl; 
		}else{
			ofs << *lbTile << std::endl;
		}
	}
	ofs.close();
    return true;
}

bool visualiseFloorplan(const Floorplan &fp, const std::string &filePath){

    std::ofstream ofs(filePath, std::ios::out);

    assert(ofs.is_open());
    if(!ofs.is_open()) return false;

    ofs << "CHIP " << rec::getWidth(fp.mChipContour) << " " << rec::getHeight(fp.mChipContour) << std::endl;
    
    

    ofs.close();
    return true;
}