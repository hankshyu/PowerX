//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/08/2025 20:44:28
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
//
//////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:
#include <assert.h>
#include <fstream>
#include <unordered_map>
#include <map>
#include <set>
#include <algorithm>
#include <utility>

// 2. Boost Library:

// 3. Texo Library:
#include "tile.hpp"
#include "floorplan.hpp"
#include "doughnutPolygon.hpp"
#include "doughnutPolygonSet.hpp"


Rectilinear *Floorplan::placeRectilinear(std::string name, rectilinearType type, Rectangle placement, area_t legalArea, double aspectRatioMin, double aspectRatioMax, double mUtilizationMin){

    assert(rec::isContained(mChipContour, placement));

    // register the Rectilinear container into the floorplan data structure
    Rectilinear *newRect = new Rectilinear(mIDCounter++, name, type, placement, legalArea, aspectRatioMin, aspectRatioMax, mUtilizationMin); 
    switch (type){
    case rectilinearType::SOFT:
        this->allRectilinears.push_back(newRect);
        this->softRectilinears.push_back(newRect);
        break;
    case rectilinearType::PREPLACED:
        this->allRectilinears.push_back(newRect);
        this->preplacedRectilinears.push_back(newRect);
        break;
    case rectilinearType::PIN:
        this->pinRectilinears.push_back(newRect);
        return newRect;
        break;
    default:
        break;
    }

    std::vector<Tile *> lappingTiles;
    cs->enumerateDirectedArea(placement, lappingTiles);
    if(lappingTiles.empty()){
        // placement of the rectilinear is in an area whether no other tiles are present
        addBlockTile(placement, newRect);
    }else{

        using namespace boost::polygon::operators;
        DoughnutPolygonSet insertSet;
        insertSet += placement;

        // process all overlap parts with other tles
        for(int i = 0; i < lappingTiles.size(); ++i){
            Tile *lapTile = lappingTiles[i];
            assert((lapTile->getType() == tileType::BLOCK) || (lapTile->getType() == tileType::OVERLAP));

            DoughnutPolygonSet origTileSet;
            origTileSet += lapTile->getRectangle();

            DoughnutPolygonSet overlapSet;
            boost::polygon::assign(overlapSet, insertSet & origTileSet);

            if(boost::polygon::equivalence(origTileSet, overlapSet)){
                // the insertion of such piece would not cause new fragments
                increaseTileOverlap(lapTile, newRect);
            }else{
                // the insertion has overlap with some existing tiles
                // 1. remove the original whole block 
                // 2. place the intersection part
                // 3. place the rest part, dice into small rectangles if necessary

                tileType lapTileType = lapTile->getType();
                if(lapTileType == tileType::BLOCK){
                    Rectilinear *origPayload = this->blockTilePayload[lapTile];

                    deleteTile(lapTile);

                    // add the intersection part first
                    std::vector<Rectangle> intersectRect;
                    dps::diceIntoRectangles(overlapSet, intersectRect);
                    assert(intersectRect.size() == 1);
                    addOverlapTile(intersectRect[0], std::vector<Rectilinear *>({origPayload, newRect}));

                    // process the remains of the overlapSet
                    origTileSet -= overlapSet;
                    std::vector<Rectangle> restRect;
                    dps::diceIntoRectangles(origTileSet, restRect);
                    for(Rectangle const &rt : restRect){
                        addBlockTile(rt, origPayload);
                    }
                }else{ // lapTileType == tileType::OVERLAP
                    std::vector<Rectilinear *> origPaylaod = this->overlapTilePayload[lapTile];

                    deleteTile(lapTile);

                    // add the intersection part first
                    std::vector<Rectangle> intersectRect;
                    dps::diceIntoRectangles(overlapSet, intersectRect);
                    assert(intersectRect.size() == 1);
                    std::vector<Rectilinear *> newPayload(origPaylaod);
                    newPayload.push_back(newRect);
                    addOverlapTile(intersectRect[0], newPayload);

                    // process the remains of the overlapSet
                    origTileSet -= overlapSet;
                    std::vector<Rectangle> restRect;
                    dps::diceIntoRectangles(origTileSet, restRect);
                    for(Rectangle const &rt : restRect){
                        addOverlapTile(rt, origPaylaod);
                    }

                }
            }

            // removed the processed part from insertSet
            insertSet -= overlapSet;
        }

        std::vector<Rectangle> remainInsertRect;
        dps::diceIntoRectangles(insertSet, remainInsertRect);
        for(const Rectangle &rt : remainInsertRect){
            addBlockTile(rt, newRect);
        }
    }

    return newRect;
}

Floorplan::Floorplan()
    : mIDCounter(0), mChipContour(Rectangle(0, 0, 0, 0)) , 
    mAllRectilinearCount(0), mSoftRectilinearCount(0), mPreplacedRectilinearCount(0), pinRectilinears(0),cs(nullptr) {
}

Floorplan::Floorplan(const Floorplan &other){
    
    // copy basic attributes
    this->mIDCounter = other.mIDCounter;
    this->mChipContour = Rectangle(other.mChipContour);
    this->mAllRectilinearCount = other.mAllRectilinearCount;
    this->mSoftRectilinearCount = other.mSoftRectilinearCount;
    this->mPreplacedRectilinearCount = other.mPreplacedRectilinearCount;
    this->mPinRectilinearCount = other.mPinRectilinearCount;

    this->cs = new CornerStitching(*other.cs);

    // build maps to assist copy
    std::unordered_map<Rectilinear *, Rectilinear*> rectMap;
    std::unordered_map<Tile *, Tile *> tileMap;

    for(const Rectilinear *const &oldRect : other.allRectilinears){
        Rectilinear *nR = new Rectilinear(*oldRect);

        // re-consruct the block tiles pointers using the new CornerStitching System
        nR->blockTiles.clear();
        for(const Tile *const &oldT : oldRect->blockTiles){
            Tile *newT = this->cs->findPoint(oldT->getLowerLeft());
            tileMap[oldT] = newT;
            nR->blockTiles.insert(newT);
        }

        // re-consruct the overlap tiles pointers using the new CornerStitching System
        nR->overlapTiles.clear();
        for(Tile *const &oldT : oldRect->overlapTiles){
            Tile *newT = this->cs->findPoint(oldT->getLowerLeft());
            tileMap[oldT] = newT;
            nR->overlapTiles.insert(newT);
        }

        rectMap[oldRect] = nR;
    }
    // rework pointers for rectilinear vectors to point to new CornerStitching System
    this->allRectilinears.clear();
    this->softRectilinears.clear();
    this->preplacedRectilinears.clear();

    for(Rectilinear *const &oldR : other.allRectilinears){
        Rectilinear *newR = rectMap[oldR];
        this->allRectilinears.push_back(rectMap[oldR]);
        
        // categorize types
        switch (newR->getType()){
        case rectilinearType::SOFT:
            this->softRectilinears.push_back(newR);
            break;
        case rectilinearType::PREPLACED:
            this->preplacedRectilinears.push_back(newR);
            break;
        default:
            break;
        }
    }

    // construct pinRectilinears;
    this->pinRectilinears.clear();
    for(Rectilinear *const &oldR : other.pinRectilinears){
        Rectilinear *newR = new Rectilinear(*oldR);
        this->pinRectilinears.push_back(newR);
        rectMap[oldR] = newR;
    }

    // rebuid Tile payloads section  
    this->blockTilePayload.clear();
    for(std::unordered_map<Tile *, Rectilinear *>::const_iterator it = other.blockTilePayload.begin(); it != other.blockTilePayload.end(); ++it){
        Tile *nT = tileMap[it->first];
        Rectilinear *nR = rectMap[it->second];

        this->blockTilePayload[nT] = nR;
    }

    this->overlapTilePayload.clear();
    for(std::unordered_map<Tile *, std::vector<Rectilinear *>>::const_iterator it = other.overlapTilePayload.begin(); it != other.overlapTilePayload.end(); ++it){
        Tile *nT = tileMap[it->first];
        std::vector<Rectilinear *> nRectVec;
        for(Rectilinear *const &oldR : it->second){
            nRectVec.push_back(rectMap[oldR]);
        }
        
        this->overlapTilePayload[nT] = nRectVec;
    }
    
}

Floorplan::~Floorplan(){
    for(Rectilinear *&rt : this->allRectilinears){
        delete(rt);
    }

    for(Rectilinear *&rt : this->pinRectilinears){
        delete(rt);
    }

    for(Connection *&cn : this->allConnections){
        delete(cn);
    }

    delete(cs);
}

Rectangle Floorplan::getChipContour() const {
    return this->mChipContour;
}

int Floorplan::getAllRectilinearCount() const {
    return this->mAllRectilinearCount;
}

int Floorplan::getSoftRectilinearCount() const {
    return this->mSoftRectilinearCount;
}

int Floorplan::getPreplacedRectilinearCount() const {
    return this->mPreplacedRectilinearCount;
}

int Floorplan::getPinRectilinearCount() const {
    return this->mPinRectilinearCount;
}


Tile *Floorplan::addBlockTile(const Rectangle &tilePosition, Rectilinear *rt){

    assert((rt->getType() == rectilinearType::PREPLACED) || (rt->getType() == rectilinearType::SOFT));

    assert(rec::isContained(this->mChipContour, tilePosition));

    assert(std::find(allRectilinears.begin(), allRectilinears.end(), rt) != allRectilinears.end());

    // use the prototype to insert the tile onto the cornerstitching system, receive the actual pointer as return
    Tile *newTile = cs->insertTile(Tile(tileType::BLOCK, tilePosition););
    // register the pointer to the rectilienar system 
    rt->blockTiles.insert(newTile);
    // connect tile's payload as the rectilinear on the floorplan system 
    this->blockTilePayload[newTile] = rt;

    return newTile;
}

Tile *Floorplan::addOverlapTile(const Rectangle &tilePosition, const std::vector<Rectilinear*> &payload){

    assert(rec::isContained(this->mChipContour, tilePosition));

    for(const Rectilinear *const &rt : payload){
        assert((rt->getType() == rectilinearType::PREPLACED) || (rt->getType() == rectilinearType::SOFT));
        assert(std::find(allRectilinears.begin(), allRectilinears.end(), rt) != allRectilinears.end());
    }

    // use the prototype to insert the tile onto the cornerstitching system, receive the actual pointer as return
    Tile *newTile = cs->insertTile(Tile(tileType::OVERLAP, tilePosition));
    
    // register the pointer to all Rectilinears
    for(Rectilinear *const rt : payload){
        rt->overlapTiles.insert(newTile);
    }

    // connect tile's payload 
    this->overlapTilePayload[newTile] = std::vector<Rectilinear *>(payload);

    return newTile;
}

void Floorplan::deleteTile(Tile *tile){
    tileType toDeleteType = tile->getType();
    assert((toDeleteType == tileType::BLOCK) || (toDeleteType == tileType::OVERLAP));
    
    // clean-up the payload information stored inside the floorplan system
    if(toDeleteType == tileType::BLOCK){
        std::unordered_map<Tile *, Rectilinear *>::iterator blockIt= this->blockTilePayload.find(tile);
        
        assert(blockIt != this->blockTilePayload.end);

        // erase the tile from the rectilinear structure
        Rectilinear *rt = blockIt->second;
        rt->blockTiles.erase(tile);
        // erase the payload from the floorplan structure
        this->blockTilePayload.erase(tile);
    }else{
        std::unordered_map<Tile *, std::vector<Rectilinear *>>::iterator overlapIt = this->overlapTilePayload.find(tile);
        assert(overlapIt != this->overlapTilePayload.end());

        // erase the tiles from the rectiliear structures
        for(Rectilinear *const rt : overlapIt->second){
            rt->overlapTiles.erase(tile);
        }

        // erase the payload from the floorplan structure
        this->overlapTilePayload.erase(tile);
    }

    // remove the tile from the cornerStitching structure
    cs->removeTile(tile);
}

void Floorplan::increaseTileOverlap(Tile *tile, Rectilinear *newRect){

    tileType increaseTileType = tile->getType();
    assert((increaseTileType == tileType::BLOCK) || (increaseTileType == tileType::OVERLAP));

    if(increaseTileType == tileType::BLOCK){
        // check if the tile is present in the floorplan structure
        std::unordered_map<Tile *, Rectilinear *>::iterator blockIt = this->blockTilePayload.find(tile);
        
        assert(blockIt != this->blockTilePayload.end());
        assert(blockIt != newRect);


        Rectilinear *oldRect = blockIt->second;
        // erase record at floorplan system and rectilinear system
        this->blockTilePayload.erase(tile);
        oldRect->blockTiles.erase(tile);

        // change the tile's type attribute 
        tile->setType(tileType::OVERLAP);

        // refill correct information for floorplan system and rectilinear
        this->overlapTilePayload[tile] = {oldRect, newRect};
        oldRect->overlapTiles.insert(tile);
        newRect->overlapTiles.insert(tile);

    }else{ // increaseTileType == tileType::OVERLAP
        std::unordered_map<Tile *, std::vector<Rectilinear *>>::iterator overlapIt = this->overlapTilePayload.find(tile);

        assert(overlapIt != this->overlapTilePayload.end());
        assert(std::find(overlapIt->second.begin(), overlapIt->second.end(), newRect) == overlapIt->second.end());

        
        // update newRect's rectilienar structure and register newRect as tile's payload at floorplan system
        newRect->overlapTiles.insert(tile);
        this->overlapTilePayload[tile].push_back(newRect);

    }
}

void Floorplan::decreaseTileOverlap(Tile *tile, Rectilinear *removeRect){

    assert(tile->getType() == tileType::OVERLAP);
    assert(this->overlapTilePayload.find(tile) != this->overlapTilePayload.end());


    std::vector<Rectilinear *> *oldPayload = &(this->overlapTilePayload[tile]);
    assert(std::find(oldPayload->begin(), oldPayload->end(), removeRect) != oldPayload->end());


    int oldPayloadSize = oldPayload->size();
    assert(oldPayload >= 2);
    if(oldPayloadSize == 2){
        // ready to change tile's type to tileType::BLOCK
        Rectilinear *solePayload = (((*oldPayload)[0]) == removeRect)? ((*oldPayload)[1]) : ((*oldPayload)[0]);
        // remove tile payload from floorplan structure
        this->overlapTilePayload.erase(tile);

        // remove from rectilinear structure
        removeRect->overlapTiles.erase(tile);
        solePayload->overlapTiles.erase(tile);

        // change type of the tile
        tile->setType(tileType::BLOCK);

        solePayload->blockTiles.insert(tile);
        this->blockTilePayload[tile] = solePayload;
    }else{  // oldPayloadSize > 2
        // the tile has 2 or more rectilinear after removal, keep type as tileType::OVERLAP

        std::vector<Rectilinear *> *toChange = &(this->overlapTilePayload[tile]);
        // apply erase-remove idiom to delete removeRect from the payload from floorplan 
        toChange->erase(std::remove(toChange->begin(), toChange->end(), removeRect));

        removeRect->overlapTiles.erase(tile);

    }
}

void Floorplan::reshapeRectilinear(Rectilinear *rt){
    if((rt->blockTiles.empty()) && (rt->overlapTiles.empty())) return;
    using namespace boost::polygon::operators;
    DoughnutPolygonSet reshapePart;
    std::unordered_map<Rectangle, Tile*> rectanglesToDelete;

    for(Tile *const &blockTile : rt->blockTiles){
        Rectangle blockRectangle = blockTile->getRectangle();
        rectanglesToDelete[blockRectangle] = blockTile;
        reshapePart += blockRectangle;
    }
    
    std::vector<Rectangle> diceResult;
    dps::diceIntoRectangles(reshapePart, diceResult);
    std::unordered_set<Rectangle> rectanglesToAdd;

    // extract those rectangles that should be deleted and those that should stay
    for(Rectangle const &diceRect : diceResult){
        if(rectanglesToDelete.find(diceRect) != rectanglesToDelete.end()){
            // old Rectilinear already exist such tile
            rectanglesToDelete.erase(diceRect); 
        }else{
            // new tile for the rectilinear
            rectanglesToAdd.insert(diceRect);
        }
    }

    for(std::unordered_map<Rectangle, Tile*>::iterator it = rectanglesToDelete.begin(); it != rectanglesToDelete.end(); ++it){
        deleteTile(it->second);
    }

    for(Rectangle const &toAddRect : rectanglesToAdd){
        addBlockTile(toAddRect, rt);
    }

}

void Floorplan::growRectilinear(const DoughnutPolygonSet &toGrow, Rectilinear *rect){
    using namespace boost::polygon::operators;
    DoughnutPolygonSet growPart;
    std::unordered_map<Rectangle, Tile*> rectanglesToDelete;

    // the original part of the Rectilinear
    for(Tile *const &blockTile :rect->blockTiles){
        Rectangle blockRectangle = blockTile->getRectangle();
        rectanglesToDelete[blockRectangle] = blockTile;
        growPart += blockRectangle;
    }
    
    // the added part
    for(DoughnutPolygon const &toGrowDP : toGrow){
        growPart += toGrowDP;
    }
    

    std::vector<Rectangle> diceResult;
    dps::diceIntoRectangles(growPart, diceResult);
    std::unordered_set<Rectangle> rectanglesToAdd;

    // extract those rectangles that should be deleted and toss those should stay
    for(Rectangle const &diceRect : diceResult){
        if(rectanglesToDelete.find(diceRect) != rectanglesToDelete.end()){
            // old Rectilinear already exist such tile
            rectanglesToDelete.erase(diceRect); 
        }else{
            // new tile for the rectilinear
            rectanglesToAdd.insert(diceRect);
        }
    }

    for(std::unordered_map<Rectangle, Tile*>::iterator it = rectanglesToDelete.begin(); it != rectanglesToDelete.end(); ++it){
        deleteTile(it->second);
    }

    for(Rectangle const &toAddRect : rectanglesToAdd){
        addBlockTile(toAddRect, rect);
    }

}

void Floorplan::shrinkRectilinear(const DoughnutPolygonSet &toShrink, Rectilinear *rect){
    using namespace boost::polygon::operators;
    DoughnutPolygonSet growPart;
    std::unordered_map<Rectangle, Tile*> rectanglesToDelete;

    // the original part of the Rectilinear
    for(Tile *const &blockTile :rect->blockTiles){
        Rectangle blockRectangle = blockTile->getRectangle();
        rectanglesToDelete[blockRectangle] = blockTile;
        growPart += blockRectangle;
    }
    
    // the removing part
    for(DoughnutPolygon const &toShrinkDP : toShrink){
        growPart -= toShrinkDP;
    }
    

    std::vector<Rectangle> diceResult;
    dps::diceIntoRectangles(growPart, diceResult);
    std::unordered_set<Rectangle> rectanglesToAdd;

    // extract those rectangles that should be deleted and tose that should stay
    for(Rectangle const &diceRect : diceResult){
        if(rectanglesToDelete.find(diceRect) != rectanglesToDelete.end()){
            // old Rectilinear already exist such tile
            rectanglesToDelete.erase(diceRect); 
        }else{
            // new tile for the rectilinear
            rectanglesToAdd.insert(diceRect);
        }
    }

    for(std::unordered_map<Rectangle, Tile*>::iterator it = rectanglesToDelete.begin(); it != rectanglesToDelete.end(); ++it){
        deleteTile(it->second);
    }

    for(Rectangle const &toAddRect : rectanglesToAdd){
        addBlockTile(toAddRect, rect);
    }

}

Tile *Floorplan::divideTileHorizontally(Tile *origTop, len_t newDownHeight){
    
    assert((origTop->getType() == tileType::BLOCK) || ((origTop->getType() == tileType::OVERLAP)));  
    
    switch (origTop->getType()){
    case tileType::BLOCK:{
        Rectilinear *origTopBelongRect = this->blockTilePayload[origTop];
        Tile *newDown = cs->cutTileHorizontally(origTop, newDownHeight);
        origTopBelongRect->blockTiles.insert(newDown);
        this->blockTilePayload[newDown] = origTopBelongRect;
        return newDown;
        break;
    }
    case tileType::OVERLAP:{
        std::vector<Rectilinear *> origTopContainedRect(this->overlapTilePayload[origTop]);
        Tile *newDown = cs->cutTileHorizontally(origTop, newDownHeight);
        for(Rectilinear *const &rect : origTopContainedRect){
            rect->overlapTiles.insert(newDown);
        }
        this->overlapTilePayload[newDown] = origTopContainedRect;
        return newDown;
        break;
    }
    default:
        return nullptr;
    }
}

Tile *Floorplan::divideTileVertically(Tile *origRight, len_t newLeftWidth){
    assert((origTop->getType() == tileType::BLOCK) || ((origTop->getType() == tileType::OVERLAP)));

    switch (origRight->getType()){
    case tileType::BLOCK:{
        Rectilinear *origTopBelongRect = this->blockTilePayload[origRight];
        Tile *newDown = cs->cutTileVertically(origRight, newLeftWidth);
        origTopBelongRect->blockTiles.insert(newDown);
        this->blockTilePayload[newDown] = origTopBelongRect;
        return newDown;
        break;
    }
    case tileType::OVERLAP:{
        std::vector<Rectilinear *> origTopContainedRect(this->overlapTilePayload[origRight]);
        Tile *newDown = cs->cutTileVertically(origRight, newLeftWidth);
        for(Rectilinear *const &rect : origTopContainedRect){
            rect->overlapTiles.insert(newDown);
        }
        this->overlapTilePayload[newDown] = origTopContainedRect;
        return newDown;
        break;
    }
    default:
        return nullptr;
    }

}

double Floorplan::calculateOverlapRatio() const {
    area_t sumLegalArea = 0;
    area_t sumOverlapArea = 0;

    for(const Rectilinear *const &rt : this->allRectilinears){
        sumLegalArea += rt->getLegalArea();
    }

    for(std::unordered_map<Tile *, std::vector<Rectilinear *>>::const_iterator it = overlapTilePayload.begin(); it != overlapTilePayload.end(); ++it){
        sumOverlapArea += (it->first->getArea() * it->second.size());
    }

    return double(sumOverlapArea) / double(sumLegalArea);
}

void Floorplan::visualiseFloorplan(const std::string &outputFileName) const {
    using namespace boost::polygon::operators;
	std::ofstream ofs;
	ofs.open(outputFileName, std::fstream::out);
    assert(ofs.is_open());
	// if(!ofs.is_open()){
    //     throw(CSException("CORNERSTITCHING_21"));
    // } 

    ofs << "CHIP " << rec::getWidth(mChipContour) << " " << rec::getHeight(mChipContour) << std::endl;

    ofs << "SOFTBLOCK " << softRectilinears.size() << std::endl;
    for(Rectilinear *const &rect : softRectilinears){
        
        ofs << rect->getName() << " " << rect->calculateActualArea() - rect->getLegalArea() << std::endl;
        Rectangle boundingBox = rect->calculateBoundingBox();
        Cord bbCords [4];
        bbCords[0] = rec::getLL(boundingBox);
        bbCords[1] = rec::getUL(boundingBox);
        bbCords[2] = rec::getUR(boundingBox);
        bbCords[3] = rec::getLR(boundingBox);
        for(int i = 0; i < 4; ++i){
            ofs << bbCords[i].x() << " " << bbCords[i].y() << std::endl;
        }

        double BBCentreX, BBCentreY;
        rec::calculateCentre(boundingBox, BBCentreX, BBCentreY); 
        Cord optCentre = calculateOptimalCentre(rect);

        ofs << int(BBCentreX + 0.5) << " " << int(BBCentreY + 0.5) << " " << optCentre.x() << " " << optCentre.y() << std::endl;

        DoughnutPolygonSet dps;

        // if(!rect->overlapTiles.empty()){
        //     throw(CSException("CORNERSTITCHING_22"));
        // }

        for(Tile *const &t : rect->blockTiles){
            dps += t->getRectangle();
        }

        for(Tile *const &t : rect->overlapTiles){
            dps += t->getRectangle();
        }

        // if(dps.size() != 1){
        //     throw(CSException("CORNERSTITCHING_23"));
        // }
        DoughnutPolygon dp = dps[0];

        boost::polygon::direction_1d direction = boost::polygon::winding(dp);
        ofs << dp.size() << std::endl;  
        
        if(direction == boost::polygon::direction_1d_enum::CLOCKWISE){
            for(auto it = dp.begin(); it != dp.end(); ++it){
                ofs << (*it).x() << " " << (*it).y() << std::endl;
            }
        }else{
            std::vector<Cord> buffer;
            for(auto it = dp.begin(); it != dp.end(); ++it){
                buffer.push_back(*it);
            }
            for(std::vector<Cord>::reverse_iterator it = buffer.rbegin(); it != buffer.rend(); ++it){
                ofs << (*it).x() << " " << (*it).y() << std::endl;
            }
        }
    }

    ofs << "PREPLACEDBLOCK " << preplacedRectilinears.size() << std::endl;
    for(Rectilinear *const &rect : preplacedRectilinears){
        DoughnutPolygonSet dps;
        // if(!rect->overlapTiles.empty()){
        //     throw(CSException("CORNERSTITCHING_22"));
        // }
        for(Tile *const &t : rect->blockTiles){
            dps += t->getRectangle();
        }

        for(Tile *const &t : rect->overlapTiles){
            dps += t->getRectangle();
        }

        // if(dps.size() != 1){
        //     throw(CSException("CORNERSTITCHING_23"));
        // }
        DoughnutPolygon dp = dps[0];

        boost::polygon::direction_1d direction = boost::polygon::winding(dp);
        ofs << rect->getName() << std::endl;
        ofs << dp.size() << std::endl;  
        
        if(direction == boost::polygon::direction_1d_enum::CLOCKWISE){
            for(auto it = dp.begin(); it != dp.end(); ++it){
                ofs << (*it).x() << " " <<  (*it).y() << std::endl;
            }
        }else{
            std::vector<Cord> buffer;
            for(auto it = dp.begin(); it != dp.end(); ++it){
                buffer.push_back(*it);
            }
            for(std::vector<Cord>::reverse_iterator it = buffer.rbegin(); it != buffer.rend(); ++it){
                ofs << (*it).x() << " " << (*it).y() << std::endl;
            }
        }
    }

    ofs << "PIN " << pinRectilinears.size() << std::endl;
    for(Rectilinear *const &rect : pinRectilinears){
        Rectangle bb = rect->calculateBoundingBox();
        Cord pinLoc = rec::getLL(bb);
        ofs << rect->getName() << " " << pinLoc.x() << " " << pinLoc.y() << std::endl;
    }

    ofs << "CONNECTION " << allConnections.size() << std::endl;
    for(Connection *const &conn : allConnections){
        ofs << conn->vertices.size() << " ";
        for(Rectilinear *const &rect : conn->vertices){
            ofs << rect->getName() << " ";
        }
        ofs << conn->weight << std::endl; 
    }

    ofs.close();
}

