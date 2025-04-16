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
#include "ballOut.hpp"
#include "microBump.hpp"
#include "c4Bump.hpp"
#include "aStarBaseline.hpp"
#include "voronoiPDNGen.hpp"

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


bool visualiseMicroBump(const MicroBump &microBump, const Technology &tch, const std::string &filePath){
    
    std::ofstream ofs(filePath, std::ios::out);
    
    assert(ofs.is_open());
    if(!ofs.is_open()) return false;
    ofs << "MICROBUMP VISUALISATION" << std::endl;

    len_t pitch = tch.getMicrobumpPitch();
    len_t pinRadius = tch.getMicrobumpRadius();

    len_t pinOutWidth = microBump.m_pinMapWidth;
    len_t pinOutHeight = microBump.m_pinMapHeight;

    ofs << microBump.m_name << " " << pinOutWidth << " " << pinOutHeight << std::endl;
    ofs << pinOutWidth * pitch << " " <<  pinOutHeight * pitch << std::endl;
    
    ofs << "CHIPLETS" << " " << microBump.instanceToRectangleMap.size() << std::endl;

    for(std::unordered_map<std::string, Rectangle>::const_iterator cit = microBump.instanceToRectangleMap.begin(); cit!= microBump.instanceToRectangleMap.end(); ++cit){
        Rectangle chipletBB = cit->second;
        ofs << cit->first << " " << microBump.instanceToBallOutMap.at(cit->first)->getName() << " ";
        ofs << pitch*rec::getXL(chipletBB) << " " << pitch*rec::getYL(chipletBB) << " ";
        ofs << pitch*(rec::getWidth(chipletBB) + 1)<< " " << pitch*(rec::getHeight(chipletBB) + 1) << std::endl;
    }


    ofs << "PINS" << " " << pinOutWidth * pinOutHeight << std::endl;
    std::unordered_map<Cord, SignalType>::const_iterator it;
    for(int j = 0; j < pinOutHeight; ++j){
        for(int i = 0; i < pinOutWidth; ++i){
            it = microBump.cordToSignalTypeMap.find(Cord(i, j));
            len_t centreX = pitch/2 + i*pitch;
            len_t centreY = pitch/2 + j*pitch;
            
            if(it == microBump.cordToSignalTypeMap.end()){
                ofs << centreX << " " << centreY << " " << pinRadius << " " << "EMPTY" << std::endl;
            }else{
                ofs << centreX << " " << centreY << " " << pinRadius << " " << it->second << std::endl;
            }
        }
    }

    ofs.close();
    return true;
}

bool visualiseC4Bump(const C4Bump &c4, const Technology &tch, const std::string &filePath){
    
    std::ofstream ofs(filePath, std::ios::out);
    
    assert(ofs.is_open());
    if(!ofs.is_open()) return false;
    ofs << "C4 VISUALISATION" << std::endl;

    len_t pitch = tch.getMicrobumpPitch();
    len_t pinRadius = tch.getMicrobumpRadius();

    len_t pinOutWidth = c4.m_pinMapWidth;
    len_t pinOutHeight = c4.m_pinMapHeight;


    ofs << c4.m_name << " " << pinOutWidth << " " << pinOutHeight << std::endl;
    ofs << pinOutWidth * pitch << " " <<  pinOutHeight * pitch << std::endl;
    

    ofs << "PINS" << " " << pinOutWidth * pinOutHeight << std::endl;
    std::unordered_map<Cord, SignalType>::const_iterator it;
    for(int j = 0; j < pinOutHeight; ++j){
        for(int i = 0; i < pinOutWidth; ++i){
            it = c4.cordToSignalTypeMap.find(Cord(i, j));
            len_t centreX = pitch/2 + i*pitch;
            len_t centreY = pitch/2 + j*pitch;
            
            if(it == c4.cordToSignalTypeMap.end()){
                ofs << centreX << " " << centreY << " " << pinRadius << " " << "EMPTY" << std::endl;
            }else{
                ofs << centreX << " " << centreY << " " << pinRadius << " " << it->second << std::endl;
            }
        }
    }

    ofs.close();
    return true;
}

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

bool visualisePGM5(const PowerGrid &pg, const std::string &filePath, bool overlayOverlaps, bool overlayM5uBump, bool overlayM7C4){
    
    std::ofstream ofs(filePath, std::ios::out);

    assert(ofs.is_open());
    if(!ofs.is_open()) return false;

    ofs << "PGM5 VISUALISATION " << pg.canvasWidth << " " << pg.canvasHeight << " ";
    ofs << "OVERLAP " << ((overlayOverlaps)? "1" : "0") << " ";
    ofs << "M5_UBUMP " << ((overlayM5uBump)? "1" : "0") << " ";
    ofs << "M7_C4 " << ((overlayM7C4)? "1" : "0") << std::endl;

    for(int j = pg.canvasWidth - 1; j >= 0; --j){
        for(int i = 0; i < pg.canvasWidth; ++i){
            int baseColor = int(toIdx(pg.canvasM5[j][i]));

            if(overlayOverlaps && (pg.canvasM5[j][i] == pg.canvasM7[j][i])) baseColor += 100;
            ofs << baseColor << " ";
        }
        ofs << std::endl;
    }

    if(overlayM5uBump){
        // leave out signal and Ground bumps
        int displaySignalType = pg.uBump.signalTypeToAllCords.size();

        std::unordered_map<SignalType, std::unordered_set<Cord>>::const_iterator cit;
        cit = pg.uBump.signalTypeToAllCords.find(SignalType::SIGNAL);
        if(cit != pg.uBump.signalTypeToAllCords.end()) displaySignalType--;
        cit = pg.uBump.signalTypeToAllCords.find(SignalType::GROUND);
        if(cit != pg.uBump.signalTypeToAllCords.end()) displaySignalType--;
    
        
        ofs << "M5_UBUMP SIGNAL_TYPES " << displaySignalType << std::endl;
        
        for(cit = pg.uBump.signalTypeToAllCords.begin(); cit != pg.uBump.signalTypeToAllCords.end(); ++cit){
            if((cit->first == SignalType::SIGNAL) || (cit->first == SignalType::GROUND)) continue;
            ofs << (cit->first) << " " << cit->second.size() << std::endl;
            for(const Cord &c : cit->second){
                ofs << c << " ";
            }
            ofs << std::endl;
        }
    }else{
        ofs << "M5_UBUMP SIGNAL_TYPES " << 0 << std::endl;
    }

    if(overlayM7C4){
        // leave out signal and ground bumps
        int displaySignalType = pg.c4.signalTypeToAllClusters.size();

        std::unordered_map<SignalType, std::unordered_set<Cord>>::const_iterator cit;
        cit = pg.c4.signalTypeToAllCords.find(SignalType::SIGNAL);
        if(cit != pg.c4.signalTypeToAllCords.end()) displaySignalType--;
        cit = pg.c4.signalTypeToAllCords.find(SignalType::GROUND);
        if(cit != pg.c4.signalTypeToAllCords.end()) displaySignalType--;
        ofs << "M7_C4BUMP SIGNAL_TYPES " << displaySignalType << std::endl;
        
        for(cit = pg.c4.signalTypeToAllCords.begin(); cit != pg.c4.signalTypeToAllCords.end(); ++cit){
            if((cit->first == SignalType::SIGNAL) || (cit->first == SignalType::GROUND)) continue;
            ofs << (cit->first) << " " << cit->second.size() << std::endl;
            for(const Cord &c : cit->second){
                ofs << c << " ";
            }
            ofs << std::endl;
        }
    }else{
        ofs << "M7_C4BUMP SIGNAL_TYPES " << 0 << std::endl;
    }

    ofs.close();
    return true;
}

bool visualisePGM7(const PowerGrid &pg, const std::string &filePath, bool overlayOverlaps, bool overlayM5uBump, bool overlayM7C4){
      
    std::ofstream ofs(filePath, std::ios::out);

    assert(ofs.is_open());
    if(!ofs.is_open()) return false;

    ofs << "PGM7 VISUALISATION " << pg.canvasWidth << " " << pg.canvasHeight << " ";
    ofs << "OVERLAP " << ((overlayOverlaps)? "1" : "0") << " ";
    ofs << "M5_UBUMP " << ((overlayM5uBump)? "1" : "0") << " ";
    ofs << "M7_C4 " << ((overlayM7C4)? "1" : "0") << std::endl;

    for(int j = pg.canvasWidth - 1; j >= 0; --j){
        for(int i = 0; i < pg.canvasWidth; ++i){
            int baseColor = int(toIdx(pg.canvasM7[j][i]));

            if(overlayOverlaps && (pg.canvasM5[j][i] == pg.canvasM7[j][i])) baseColor += 100;
            ofs << baseColor << " ";
        }
        ofs << std::endl;
    }

    if(overlayM5uBump){
        // leave out signal and Ground bumps
        int displaySignalType = pg.uBump.signalTypeToAllCords.size();

        std::unordered_map<SignalType, std::unordered_set<Cord>>::const_iterator cit;
        cit = pg.uBump.signalTypeToAllCords.find(SignalType::SIGNAL);
        if(cit != pg.uBump.signalTypeToAllCords.end()) displaySignalType--;
        cit = pg.uBump.signalTypeToAllCords.find(SignalType::GROUND);
        if(cit != pg.uBump.signalTypeToAllCords.end()) displaySignalType--;
    
        
        ofs << "M5_UBUMP SIGNAL_TYPES " << displaySignalType << std::endl;
        
        for(cit = pg.uBump.signalTypeToAllCords.begin(); cit != pg.uBump.signalTypeToAllCords.end(); ++cit){
            if((cit->first == SignalType::SIGNAL) || (cit->first == SignalType::GROUND)) continue;
            ofs << (cit->first) << " " << cit->second.size() << std::endl;
            for(const Cord &c : cit->second){
                ofs << c << " ";
            }
            ofs << std::endl;
        }
    }else{
        ofs << "M5_UBUMP SIGNAL_TYPES " << 0 << std::endl;
    }

    if(overlayM7C4){
        // leave out signal and ground bumps
        int displaySignalType = pg.c4.signalTypeToAllClusters.size();

        std::unordered_map<SignalType, std::unordered_set<Cord>>::const_iterator cit;
        cit = pg.c4.signalTypeToAllCords.find(SignalType::SIGNAL);
        if(cit != pg.c4.signalTypeToAllCords.end()) displaySignalType--;
        cit = pg.c4.signalTypeToAllCords.find(SignalType::GROUND);
        if(cit != pg.c4.signalTypeToAllCords.end()) displaySignalType--;
        ofs << "M7_C4BUMP SIGNAL_TYPES " << displaySignalType << std::endl;
        
        for(cit = pg.c4.signalTypeToAllCords.begin(); cit != pg.c4.signalTypeToAllCords.end(); ++cit){
            if((cit->first == SignalType::SIGNAL) || (cit->first == SignalType::GROUND)) continue;
            ofs << (cit->first) << " " << cit->second.size() << std::endl;
            for(const Cord &c : cit->second){
                ofs << c << " ";
            }
            ofs << std::endl;
        }
    }else{
        ofs << "M7_C4BUMP SIGNAL_TYPES " << 0 << std::endl;
    }

    ofs.close();
    return true; 
}

bool visualisePGOverlap(const PowerGrid &pg, const std::string &filePath, bool overlayM5uBump, bool overlayM7C4){
          
    std::ofstream ofs(filePath, std::ios::out);

    assert(ofs.is_open());
    if(!ofs.is_open()) return false;

    ofs << "PGOVERLAP VISUALISATION " << pg.canvasWidth << " " << pg.canvasHeight << " ";
    ofs << "OVERLAP 1 ";
    ofs << "M5_UBUMP " << ((overlayM5uBump)? "1" : "0") << " ";
    ofs << "M7_C4 " << ((overlayM7C4)? "1" : "0") << std::endl;

    for(int j = pg.canvasWidth - 1; j >= 0; --j){
        for(int i = 0; i < pg.canvasWidth; ++i){
            if((pg.canvasM5[j][i] == pg.canvasM7[j][i])){
                ofs << int(toIdx(pg.canvasM7[j][i])) << " ";
            }else{
                ofs << int(toIdx(SignalType::EMPTY)) << " ";
            }
        }
        ofs << std::endl;
    }

    if(overlayM5uBump){
        // leave out signal and Ground bumps
        int displaySignalType = pg.uBump.signalTypeToAllCords.size();

        std::unordered_map<SignalType, std::unordered_set<Cord>>::const_iterator cit;
        cit = pg.uBump.signalTypeToAllCords.find(SignalType::SIGNAL);
        if(cit != pg.uBump.signalTypeToAllCords.end()) displaySignalType--;
        cit = pg.uBump.signalTypeToAllCords.find(SignalType::GROUND);
        if(cit != pg.uBump.signalTypeToAllCords.end()) displaySignalType--;
    
        
        ofs << "M5_UBUMP SIGNAL_TYPES " << displaySignalType << std::endl;
        
        for(cit = pg.uBump.signalTypeToAllCords.begin(); cit != pg.uBump.signalTypeToAllCords.end(); ++cit){
            if((cit->first == SignalType::SIGNAL) || (cit->first == SignalType::GROUND)) continue;
            ofs << (cit->first) << " " << cit->second.size() << std::endl;
            for(const Cord &c : cit->second){
                ofs << c << " ";
            }
            ofs << std::endl;
        }
    }else{
        ofs << "M5_UBUMP SIGNAL_TYPES " << 0 << std::endl;
    }

    if(overlayM7C4){
        // leave out signal and ground bumps
        int displaySignalType = pg.c4.signalTypeToAllClusters.size();

        std::unordered_map<SignalType, std::unordered_set<Cord>>::const_iterator cit;
        cit = pg.c4.signalTypeToAllCords.find(SignalType::SIGNAL);
        if(cit != pg.c4.signalTypeToAllCords.end()) displaySignalType--;
        cit = pg.c4.signalTypeToAllCords.find(SignalType::GROUND);
        if(cit != pg.c4.signalTypeToAllCords.end()) displaySignalType--;
        ofs << "M7_C4BUMP SIGNAL_TYPES " << displaySignalType << std::endl;
        
        for(cit = pg.c4.signalTypeToAllCords.begin(); cit != pg.c4.signalTypeToAllCords.end(); ++cit){
            if((cit->first == SignalType::SIGNAL) || (cit->first == SignalType::GROUND)) continue;
            ofs << (cit->first) << " " << cit->second.size() << std::endl;
            for(const Cord &c : cit->second){
                ofs << c << " ";
            }
            ofs << std::endl;
        }
    }else{
        ofs << "M7_C4BUMP SIGNAL_TYPES " << 0 << std::endl;
    }

    ofs.close();
    return true; 
}

bool visualiseM5VoronoiPoints(const VoronoiPDNGen &vpg, const std::string &filePath){

    std::ofstream ofs(filePath, std::ios::out);

    assert(ofs.is_open());
    if(!ofs.is_open()) return false;

    std::unordered_set<Cord> knownCords;

    ofs << "M5 VORONOI VISUALISATION " << vpg.nodeWidth << " " << vpg.nodeHeight << std::endl;
    ofs << "SIGNALS" << " " << vpg.m5Segments.size() << std::endl;

    for(std::unordered_map<SignalType, std::vector<OrderedSegment>>::const_iterator cit = vpg.m5Segments.begin(); cit != vpg.m5Segments.end(); ++cit){
        SignalType st = cit->first;
        ofs << st << std::endl;
        ofs << "SEGMENTS " << cit->second.size() << std::endl;
        for(const OrderedSegment &cos : cit->second){
            Cord c1(cos.getLow());
            Cord c2(cos.getHigh());
            knownCords.insert(c1);
            knownCords.insert(c2);
            ofs << c1 << " " << c2 << std::endl;
        }
        std::unordered_set<Cord> unseenCords;
        for(const Cord &c : vpg.m5Points.at(st)){
            if(knownCords.count(c) == 0) unseenCords.insert(c);
        }
        ofs << "POINTS " << unseenCords.size() << std::endl;
        for(const Cord &c : unseenCords){
            ofs << c << std::endl;
        }
        
    }

    ofs.close();
    return true; 

}

bool visualiseM7VoronoiPoints(const VoronoiPDNGen &vpg, const std::string &filePath){
    return false;
}
