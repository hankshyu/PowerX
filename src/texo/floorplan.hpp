//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/03/2025 15:10:04
//  Module Name:        floorplan.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A class that provides the vision of a canvas with rectilinear
//                      entities. Provides easy to manipulate functions in order to 
//                      conduct operations on rectilinears
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//  02/21/2025:         Change to suggested way to register as as a model of point concept
//                      in boost library. Full implementation of documented members.
//
//  02/22/2025:         Add conversion operator from Cord to FCord
//
//
//  02/28/2025:         Switch back to typedef implementation
//////////////////////////////////////////////////////////////////////////////////

#ifndef __FLOORPLAN_H__
#define __FLOORPLAN_H__

// Dependencies
// 1. C++ STL:
#include <string>
#include <unordered_map>

// 2. Boost Library:

// 3. Texo Library:
#include "tile.hpp"
#include "lineTile.hpp"
#include "rectangle.hpp"

#include "rectilinear.hpp"
#include "cornerStitching.hpp"

#include "doughnutPolygon.hpp"
#include "doughnutPolygonSet.hpp"

class Floorplan{
private:

    // counts the number of rectilinears
    int mIDCounter;

    Rectangle mChipContour;

    int mAllRectilinearCount;
    int mSoftRectilinearCount;
    int mPreplacedRectilinearCount;
    int mPinRectilinearCount;
    
    // function that places a rectilinear into the floorplan system. It automatically resolves overlaps by splittng and divide existing tiles
    Rectilinear *placeRectilinear(std::string name, rectilinearType type, Rectangle placement, area_t legalArea);

public:
    CornerStitching *cs;

    // allRectilinears is softRectilinears + preplacedRectilinears
    std::vector<Rectilinear *> allRectilinears;
    
    std::vector<Rectilinear *> softRectilinears;
    std::vector<Rectilinear *> preplacedRectilinears;

    // pinRectilinears should only be used for HPWL calculation
    std::vector<Rectilinear *> pinRectilinears;
    
    std::unordered_map<Tile *, Rectilinear *> blockTilePayload;
    std::unordered_map<Tile *, std::vector<Rectilinear *>> overlapTilePayload;


    Floorplan();
    Floorplan(const Floorplan &other);
    ~Floorplan();

    Rectangle getChipContour() const;
    int getAllRectilinearCount() const;
    int getSoftRectilinearCount() const;
    int getPreplacedRectilinearCount() const;
    int getPinRectilinearCount() const;


    // insert a tleType::BLOCK tile at tilePosition into cornerStitching & rectilinear (*rt) system,
    // record rt as it's payload into floorplan system (into blockTilePayload) and return new tile's pointer
    // User himself should make sure the block position has no other occupants, else error would emerge
    Tile *addBlockTile(const Rectangle &tilePosition, Rectilinear *rt);

    // insert a tleType::OVERLAP tile at tilePosition into cornerStitching & rectilinear (•rt) system,
    // record payload as it's payload into floorplan system (into overlapTilePayload) and return new tile's pointer
    // User himself should make sure the block position has no other occupants, else error would emerge
    Tile *addOverlapTile(const Rectangle &tilePosition, const std::vector<Rectilinear*> &payload);

    // remove tile data payload at floorplan system, the rectilienar that records it and lastly remove from cornerStitching,
    // only tile.type == tileType::BLOCK or tileType::OVERLAP is accepted
    void deleteTile(Tile *tile);

    // log newRect as it's overlap tiles and update Rectilinear structure & floorplan paylaod, upgrade tile to tile::OVERLAP if necessary 
    void increaseTileOverlap(Tile *tile, Rectilinear *newRect);

    // remove removeRect as tile's overlap and update Rectilinear structure & floorplan payload, make tile tile::BLOCK if necessary
    void decreaseTileOverlap(Tile *tile, Rectilinear *removeRect);

    // collect all blocks within a rectilinear, reDice them into Rectangles. (potentially reduce Tile count)
    void reshapeRectilinear(Rectilinear *rt);

    // grow the shape toGrow to the Rectilinear
    // User himself should make sure the block position has no other occupants, else error would emerge

    void growRectilinear(const DoughnutPolygonSet &toGrow, Rectilinear *rect);
    
    // take the shape toShrink off the Rectilinear
    void shrinkRectilinear(const DoughnutPolygonSet &toShrink, Rectilinear *rect);

    // Pass in the victim tile pointer through origTop, it will split the tile into two pieces:
    // 1. origTop represents the top portion of the split, with height (origTop.height - newDownHeight)
    // 2. newDown represents the lower portion of the split, with height newDownHeight, is the return value
    Tile *divideTileHorizontally(Tile *origTop, len_t newDownHeight);

    // Pass in the victim tile pointer through origRight, it will stplit the tile into two pieces:
    // 1. origRight represents the right portion of the split, with width (origRight.width - newLeftWidth)
    // 2. newLeft represents the left portion of the split, with width newLeftWidth, is the return value
    Tile *divideTileVertically(Tile *origRight, len_t newLeftWidth);
    
    double calculateOverlapRatio() const;

    // write Floorplan class for presenting software (renderFloorplan.py)
    void visualiseFloorplan(const std::string &outputFileName) const;
};

#endif // __FLOORPLAN_H__


