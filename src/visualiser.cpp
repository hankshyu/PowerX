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
#include <unordered_set>

// 2. Boost Library:
#include "boost/geometry.hpp"
#include "boost/geometry/geometries/point_xy.hpp"
#include "boost/geometry/geometries/polygon.hpp"

// 3. Texo Library:
#include "visualiser.hpp"
#include "cord.hpp"

#include "cornerStitching.hpp"
#include "floorplan.hpp"
#include "technology.hpp"
#include "ballOut.hpp"
#include "objectArray.hpp"
#include "microBump.hpp"


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

bool visualisePinArray(const std::vector<std::vector<SignalType>> &pinArr, const Technology &tch, const std::string &filePath){
    std::ofstream ofs(filePath, std::ios::out);

    assert(ofs.is_open());
    if(!ofs.is_open()) return false;

    len_t pitch = tch.getMicrobumpPitch();
    len_t pinRadius = tch.getMicrobumpRadius();

    int pinWidth = pinArr[0].size();
    int pinHeight = pinArr.size();

    int gridWidth = pinWidth -1;
    int gridHeight = pinHeight - 1;

    ofs << "PIN VISUALISATION" << std::endl;
    ofs << pitch << " " << pinRadius << " " << gridWidth << " " << gridHeight << " " << pinWidth << " " << pinHeight << std::endl;

    for(int j = 0; j < pinHeight; ++j){
        for(int i = 0; i < pinWidth; ++i){
            ofs << i << " " << j << " " << pinArr[j][i] << std::endl; 
        }
    }

    ofs.close();
    return true;
}

bool visualiseGridArray(const std::vector<std::vector<SignalType>> &gridArr, const Technology &tch, const std::string &filePath){
    std::ofstream ofs(filePath, std::ios::out);

    assert(ofs.is_open());
    if(!ofs.is_open()) return false;

    len_t pitch = tch.getMicrobumpPitch();
    len_t pinRadius = tch.getMicrobumpRadius();

    int gridWidth = gridArr[0].size();
    int gridHeight = gridArr.size();

    int pinWidth = gridWidth + 1;
    int pinHeight = gridHeight + 1;

    ofs << "GRID VISUALISATION" << std::endl;
    ofs << pitch << " " << pinRadius << " " << gridWidth << " " << gridHeight << " " << pinWidth << " " << pinHeight << std::endl;

    for(int j = 0; j < gridHeight; ++j){
        for(int i = 0; i < gridWidth; ++i){
            ofs << i << " " << j << " " << gridArr[j][i] << std::endl;
        }
    }

    ofs.close();
    return true;
}

bool visualiseGridArrayWithPin(const std::vector<std::vector<SignalType>> &gridArr, const std::vector<std::vector<SignalType>> &pinArr, const Technology &tch, const std::string &filePath){
    std::ofstream ofs(filePath, std::ios::out);

    assert(ofs.is_open());
    if(!ofs.is_open()) return false;

    int gridWidth = gridArr[0].size();
    int gridHeight = gridArr.size();

    int pinWidth = pinArr[0].size();
    int pinHeight = pinArr.size();

    assert(gridWidth == (pinWidth-1));
    assert(gridHeight == (pinHeight-1));

    len_t pitch = tch.getMicrobumpPitch();
    len_t pinRadius = tch.getMicrobumpRadius();

    ofs << "GRID_PIN VISUALISATION" << std::endl;
    ofs << pitch << " " << pinRadius << " " << gridWidth << " " << gridHeight << " " << pinWidth << " " << pinHeight << std::endl;
    
    for(int j = 0; j < gridHeight; ++j){
        for(int i = 0; i < gridWidth; ++i){
            ofs << i << " " << j << " " << gridArr[j][i] << std::endl;
        }
    }

    for(int j = 0; j < pinHeight; ++j){
        for(int i = 0; i < pinWidth; ++i){
            ofs << i << " " << j << " " << pinArr[j][i] << std::endl; 
        }
    }

    ofs.close();
    return true;
}

bool visualiseGridArrayWithPins(const std::vector<std::vector<SignalType>> &gridArr,const std::vector<std::vector<SignalType>> &upPinArr, 
    const std::vector<std::vector<SignalType>> &downPinArr, const Technology &tch, const std::string &filePath){
        
    std::ofstream ofs(filePath, std::ios::out);

    assert(ofs.is_open());
    if(!ofs.is_open()) return false;

    int gridWidth = gridArr[0].size();
    int gridHeight = gridArr.size();

    int upPinWidth = upPinArr[0].size();
    int upPinHeight = upPinArr.size();

    int downPinWidth = downPinArr[0].size();
    int downPinHeight = downPinArr.size();

    assert(gridWidth == (upPinWidth-1));
    assert(gridHeight == (upPinHeight-1));

    assert(upPinWidth == downPinWidth);
    assert(upPinHeight == downPinHeight);

    len_t pitch = tch.getMicrobumpPitch();
    len_t pinRadius = tch.getMicrobumpRadius();

    ofs << "PIN_GRID_PIN VISUALISATION" << std::endl;
    ofs << pitch << " " << pinRadius << " " << gridWidth << " " << gridHeight << " " << upPinWidth << " " << upPinHeight << std::endl;

    for(int j = 0; j < gridHeight; ++j){
        for(int i = 0; i < gridWidth; ++i){
            ofs << i << " " << j << " " << gridArr[j][i] << std::endl;
        }
    }

    for(int j = 0; j < upPinHeight; ++j){
        for(int i = 0; i < upPinWidth; ++i){
            ofs << i << " " << j << " " << upPinArr[j][i] << std::endl; 
        }
    }

    for(int j = 0; j < downPinHeight; ++j){
        for(int i = 0; i < downPinWidth; ++i){
            ofs << i << " " << j << " " << downPinArr[j][i] << std::endl; 
        }
    }


    ofs.close();
    return true;
}

bool visualiseMicroBump(const MicroBump &microBump, const Technology &tch, const std::string &filePath){
    std::ofstream ofs(filePath, std::ios::out);

    assert(ofs.is_open());
    if(!ofs.is_open()) return false;

    len_t pitch = tch.getMicrobumpPitch();
    len_t pinRadius = tch.getMicrobumpRadius();

    int pinWidth = microBump.canvas[0].size();
    int pinHeight = microBump.canvas.size();

    int gridWidth = pinWidth -1;
    int gridHeight = pinHeight - 1;

    ofs << "MICROBUMP VISUALISATION" << std::endl;
    ofs << pitch << " " << pinRadius << " " << gridWidth << " " << gridHeight << " " << pinWidth << " " << pinHeight << std::endl;

    for(int j = 0; j < pinHeight; ++j){
        for(int i = 0; i < pinWidth; ++i){
            ofs << i << " " << j << " " << microBump.canvas[j][i] << std::endl; 
        }
    }
    
    ofs << "CHIPLETS" << " " << microBump.instanceToRectangleMap.size() << std::endl;
    for(std::unordered_map<std::string, Rectangle>::const_iterator cit = microBump.instanceToRectangleMap.begin(); cit!= microBump.instanceToRectangleMap.end(); ++cit){
        Rectangle chipletBB = cit->second;
        ofs << cit->first << " " << microBump.instanceToBallOutMap.at(cit->first)->getName() << " ";
        ofs << rec::getXL(chipletBB) << " " << rec::getYL(chipletBB) << " ";
        ofs << rec::getWidth(chipletBB) + 1 << " " << rec::getHeight(chipletBB) + 1 << std::endl;
    }


    ofs.close();
    return true;
}

bool visualisePointsSegments(const VoronoiPDNGen &vpg, const std::unordered_map<SignalType, std::vector<Cord>> &points, const std::unordered_map<SignalType, std::vector<OrderedSegment>> &segments, const std::string &filePath){

    std::ofstream ofs(filePath, std::ios::out);

    assert(ofs.is_open());
    if(!ofs.is_open()) return false;

    std::unordered_set<SignalType> allSignals;
    std::unordered_set<Cord> knownCords;

    ofs << "VORONOI_POINTS_SEGMENTS VISUALISATION " << vpg.getPinWidth() << " " << vpg.getPinHeight() << std::endl;
    // calculate all appear signals in points and segments
    for(std::unordered_map<SignalType, std::vector<Cord>>::const_iterator cit = points.begin(); cit != points.end(); ++cit){
        allSignals.insert(cit->first);
    }
    for(std::unordered_map<SignalType, std::vector<OrderedSegment>>::const_iterator cit = segments.begin(); cit != segments.end(); ++cit){
        allSignals.insert(cit->first);
    }
    
    
    ofs << "SIGNALS" << " " << allSignals.size() << std::endl;
    for(SignalType st : allSignals){
        ofs << st << std::endl;
        if(segments.count(st) != 0){
            ofs << "SEGMENTS " << segments.at(st).size() << std::endl;
            for(const OrderedSegment &cos : segments.at(st)){
                Cord c1(cos.getLow());
                Cord c2(cos.getHigh());
                knownCords.insert(c1);
                knownCords.insert(c2);
                ofs << c1 << " " << c2 << std::endl;
            }
        }else{
            ofs << "SEGMENTS " << 0 << std::endl;
        }

        if(points.count(st) != 0){
            std::unordered_set<Cord> unseenCords;
            for(const Cord &c : points.at(st)){
                if(knownCords.count(c) == 0) unseenCords.insert(c);
            }
            ofs << "POINTS " << unseenCords.size() << std::endl;
            for(const Cord &c : unseenCords){
                ofs << c << std::endl;
            }
            knownCords.insert(unseenCords.begin(), unseenCords.end());

        }else{
            ofs << "POINTS " << 0 << std::endl;
        }
    }

    ofs.close();
    return true;
}

bool visualiseVoronoiGraph(const VoronoiPDNGen &vpg, const std::unordered_map<SignalType, std::vector<Cord>> &points, const std::unordered_map<Cord, std::vector<FCord>> &cells, const std::string &filePath){

    std::ofstream ofs(filePath, std::ios::out);

    assert(ofs.is_open());
    if(!ofs.is_open()) return false;

    ofs << "VORONOI_GRAPH VISUALISATION " << vpg.getPinWidth() << " " << vpg.getPinHeight() << std::endl;
    ofs << "SIGNALS" << " " << points.size() << std::endl;

    for(std::unordered_map<SignalType, std::vector<Cord>>::const_iterator cit = points.begin(); cit != points.end(); ++cit){
        SignalType st = cit->first;
        ofs << st << std::endl;
        ofs << "POINTS " << cit->second.size() << std::endl;
        for(const Cord &c : cit->second){
            ofs << c << std::endl;
            std::unordered_map<Cord, std::vector<FCord>>::const_iterator fcit = cells.find(c);
            if(fcit == cells.end()) ofs << 0 << std::endl;
            else{
                for(const FCord &fc : fcit->second){
                    ofs << fc << " ";
                }
                ofs << std::endl;
            }
        }
        
    }

    ofs.close();
    return true; 
}

/*
bool visualiseM5VoronoiGraph(const VoronoiPDNGen &vpg, const std::string &filePath){

    std::ofstream ofs(filePath, std::ios::out);

    assert(ofs.is_open());
    if(!ofs.is_open()) return false;

    ofs << "M5 VORONOI_GRAPH VISUALISATION " << vpg.nodeWidth << " " << vpg.nodeHeight << std::endl;
    ofs << "SIGNALS" << " " << vpg.m5Segments.size() << std::endl;

    for(std::unordered_map<SignalType, std::vector<Cord>>::const_iterator cit = vpg.m5Points.begin(); cit != vpg.m5Points.end(); ++cit){
        SignalType st = cit->first;
        ofs << st << std::endl;
        ofs << "POINTS " << cit->second.size() << std::endl;
        for(const Cord &c : cit->second){
            ofs << c << std::endl;
            std::unordered_map<Cord, std::vector<FCord>>::const_iterator fcit = vpg.m5VoronoiCells.find(c);
            if(fcit == vpg.m5VoronoiCells.end()) ofs << 0 << std::endl;
            else{
                for(const FCord &fc : fcit->second){
                    ofs << fc << " ";
                }
                ofs << std::endl;
            }
        }
        
    }

    ofs.close();
    return true; 
}
*/

/*
bool visualiseM7VoronoiGraph(const VoronoiPDNGen &vpg, const std::string &filePath){

    std::ofstream ofs(filePath, std::ios::out);

    assert(ofs.is_open());
    if(!ofs.is_open()) return false;

    ofs << "M5 VORONOI_GRAPH VISUALISATION " << vpg.nodeWidth << " " << vpg.nodeHeight << std::endl;
    ofs << "SIGNALS" << " " << vpg.m7Segments.size() << std::endl;

    for(std::unordered_map<SignalType, std::vector<Cord>>::const_iterator cit = vpg.m7Points.begin(); cit != vpg.m7Points.end(); ++cit){
        SignalType st = cit->first;
        ofs << st << std::endl;
        ofs << "POINTS " << cit->second.size() << std::endl;
        for(const Cord &c : cit->second){
            ofs << c << std::endl;
            std::unordered_map<Cord, std::vector<FCord>>::const_iterator fcit = vpg.m7VoronoiCells.find(c);
            if(fcit == vpg.m7VoronoiCells.end()) ofs << 0 << std::endl;
            else{
                for(const FCord &fc : fcit->second){
                    ofs << fc << " ";
                }
                ofs << std::endl;
            }
        }
        
    }

    ofs.close();
    return true; 
}
*/

/*
bool visualiseM5VoronoiPolygons(const VoronoiPDNGen &vpg, const std::string &filePath){

    std::ofstream ofs(filePath, std::ios::out);

    assert(ofs.is_open());
    if(!ofs.is_open()) return false;

    using FPGMPoint = boost::geometry::model::d2::point_xy<flen_t>;
    using FPGMPolygon = boost::geometry::model::polygon<FPGMPoint>;
    using FPGMMultiPolygon = boost::geometry::model::multi_polygon<FPGMPolygon>;

    ofs << "M5 VORONOI_POLYGON VISUALISATION " << vpg.nodeWidth << " " << vpg.nodeHeight << std::endl;
    ofs << "SIGNALS" << " " << vpg.m5MultiPolygons.size() << std::endl;

    for(std::unordered_map<SignalType, FPGMMultiPolygon>::const_iterator cit = vpg.m5MultiPolygons.begin(); cit != vpg.m5MultiPolygons.end(); ++cit){
        SignalType st = cit->first;
        FPGMMultiPolygon mp = cit->second;
        ofs << st  << " " << mp.size() << " PIECES " << std::endl;

        for (size_t i = 0; i < mp.size(); ++i) {
            const FPGMPolygon& poly = mp[i];
    
            // Print exterior ring
            const auto& exterior = poly.outer();
            ofs << "PIECE " << i << " POINTS " << exterior.size() << std::endl;

            for (const auto& pt : exterior) {
                ofs << "f(" << boost::geometry::get<0>(pt) << ", " << boost::geometry::get<1>(pt) << ")" << std::endl;
            }
    
            // Print interior rings (holes)
            const auto& holes = poly.inners();
            ofs << "PIECE " << i << " HOLES " << holes.size() << std::endl;
            for (size_t h = 0; h < holes.size(); ++h) {
                ofs << "HOLE " << h << " POINTS " << holes[h].size() << std::endl;
                for (const auto& pt : holes[h]) {
                    ofs << "f(" << boost::geometry::get<0>(pt) << ", " << boost::geometry::get<1>(pt) << ")" << std::endl;
                }
            }
        } 
        
    }

    ofs.close();
    return true; 
}
*/

/*
bool visualiseM7VoronoiPolygons(const VoronoiPDNGen &vpg, const std::string &filePath){

    std::ofstream ofs(filePath, std::ios::out);

    assert(ofs.is_open());
    if(!ofs.is_open()) return false;

    using FPGMPoint = boost::geometry::model::d2::point_xy<flen_t>;
    using FPGMPolygon = boost::geometry::model::polygon<FPGMPoint>;
    using FPGMMultiPolygon = boost::geometry::model::multi_polygon<FPGMPolygon>;

    ofs << "M7 VORONOI_POLYGON VISUALISATION " << vpg.nodeWidth << " " << vpg.nodeHeight << std::endl;
    ofs << "SIGNALS" << " " << vpg.m7MultiPolygons.size() << std::endl;

    for(std::unordered_map<SignalType, FPGMMultiPolygon>::const_iterator cit = vpg.m7MultiPolygons.begin(); cit != vpg.m7MultiPolygons.end(); ++cit){
        SignalType st = cit->first;
        FPGMMultiPolygon mp = cit->second;
        ofs << st << " " << mp.size() << " PIECES " << std::endl;

        for (size_t i = 0; i < mp.size(); ++i) {
            const FPGMPolygon& poly = mp[i];
    
            // Print exterior ring
            const auto& exterior = poly.outer();
            ofs << "PIECE " << i << " POINTS " << exterior.size() << std::endl;

            for (const auto& pt : exterior) {
                ofs << "f(" << boost::geometry::get<0>(pt) << ", " << boost::geometry::get<1>(pt) << ")" << std::endl;
            }
    
            // Print interior rings (holes)
            const auto& holes = poly.inners();
            ofs << "PIECE " << i << " HOLES " << holes.size() << std::endl;
            for (size_t h = 0; h < holes.size(); ++h) {
                ofs << "HOLE " << h << " POINTS " << holes[h].size() << std::endl;
                for (const auto& pt : holes[h]) {
                    ofs << "f(" << boost::geometry::get<0>(pt) << ", " << boost::geometry::get<1>(pt) << ")" << std::endl;
                }
            }
        } 
        
    }

    ofs.close();
    return true; 
}
*/