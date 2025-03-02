//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/02/2025 14:56:44
//  Module Name:        rectilinear.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A representation of rectilinear shape, a set of tiles
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//  03/02/2025:         Remove exceptions, change to assert
//  03/02/2025:         Change namespace dp and dps set function to correct(new) ones
//  03/02/2025:         revise hash function, add boost library hashing
//////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:
#include <ostream>
#include <string>
#include <unordered_set>
#include <climits>
#include <cassert>

// 2. Boost Library:
#include "boost/polygon/polygon.hpp"
#include "boost/functional/hash.hpp"

// 3. Texo Library:
#include "rectilinear.hpp"
// #include "cSException.hpp"
#include "doughnutPolygon.hpp"
#include "doughnutPolygonSet.hpp"



std::ostream &operator << (std::ostream &os, const rectilinearType &t){
    switch (t)
    {
    case rectilinearType::EMPTY:
        os << "rectilinearType::EMPTY"; 
        break;
    case rectilinearType::SOFT:
        os << "rectilinearType::SOFT"; 
        break;
    case rectilinearType::PREPLACED:
        os << "rectilinearType::PREPLACED"; 
        break;
    case rectilinearType::PIN:
        os << "rectilinearType::PIN";
        break;
    default:
        break;
    }

    return os;

}

std::ostream &operator << (std::ostream &os, const rectilinearIllegalType &t){
    switch (t)
    {
    case rectilinearIllegalType::LEGAL:
        os << "rectilinearIllegalType::LEGAL";
        break;
    case rectilinearIllegalType::PREPLACE_FAIL:
        os << "rectilinearIllegalType::PREPLACE_FAIL";
        break;

    case rectilinearIllegalType::OVERLAP:
        os << "rectilinearIllegalType::OVERLAP";
        break;
    case rectilinearIllegalType::HOLE:
        os << "rectilinearIllegalType::HOLE";
        break;
    case rectilinearIllegalType::TWO_SHAPE:
        os << "rectilinearIllegalType::TWO_SHAPE";
        break;
    case rectilinearIllegalType::INNER_WIDTH:
        os << "rectilinearIllegalType::INNER_WIDTH";
        break;
    
    case rectilinearIllegalType::AREA:
        os << "rectilinearIllegalType::AREA";
        break;
    case rectilinearIllegalType::ASPECT_RATIO:
        os << "rectilinearIllegalType::ASPECT_RATIO";
        break;
    case rectilinearIllegalType::UTILIZATION:
        os << "rectilinearIllegalType::UTILIZATION";
        break;

    default:
        break;
    }

    return os;

}

Rectilinear::Rectilinear()
    : mId(-1), mName(""), mType(rectilinearType::EMPTY), mGlobalPlacement(Rectangle(0, 0, 0, 0)),
    mLegalArea(0), mAspectRatioMin(0), mAspectRatioMax(std::numeric_limits<double>::max()), mUtilizationMin(0){
}

Rectilinear::Rectilinear(int id, std::string name, rectilinearType type, Rectangle initPlacement,
                        area_t legalArea, double aspectRatioMin, double aspectRatioMax, double utilizationMin)
    : mId(id), mName(name), mType(type), mGlobalPlacement(initPlacement),
    mLegalArea(legalArea), mAspectRatioMin(aspectRatioMin), mAspectRatioMax(aspectRatioMax), mUtilizationMin(utilizationMin){
}

Rectilinear::Rectilinear(const Rectilinear &other)
    : mId(other.mId), mName(other.mName), mType(other.mType), mGlobalPlacement(other.mGlobalPlacement),
    mLegalArea(other.mLegalArea), mAspectRatioMin(other.mAspectRatioMin), mAspectRatioMax(other.mAspectRatioMax), mUtilizationMin(other.mUtilizationMin){
        this->blockTiles = std::unordered_set<Tile *>(other.blockTiles.begin(), other.blockTiles.end());
        this->overlapTiles = std::unordered_set<Tile *>(other.overlapTiles.begin(), other.overlapTiles.end());
}

Rectilinear &Rectilinear::operator = (const Rectilinear &other) {
    if(this == &other) return (*this);

    this->mId = other.mId;
    this->mName = other.mName;
    this->mType = other.mType;
    this->mGlobalPlacement = other.mGlobalPlacement;
    this->mLegalArea = other.mLegalArea;
    this->mAspectRatioMin = other.mAspectRatioMin;
    this->mAspectRatioMax = other.mAspectRatioMax;
    this->mUtilizationMin = other.mUtilizationMin;

    this->blockTiles = std::unordered_set<Tile *>(other.blockTiles);
    this->overlapTiles = std::unordered_set<Tile*>(other.overlapTiles);

    return (*this);
}

bool Rectilinear::operator == (const Rectilinear &comp) const {
    bool sameId = (this->mId == comp.mId);
    bool sameName = (this->mName == comp.mName);
    bool sameType = (this->mType == comp.mType);
    bool sameLegalArea = (this->mLegalArea == comp.mLegalArea);
    bool sameGlobalPlacement = (this->mGlobalPlacement == comp.mGlobalPlacement);

    bool sameBlockTiles = (this->blockTiles == comp.blockTiles);
    bool sameOverlapTiles = (this->overlapTiles == comp.overlapTiles);

    return (sameId && sameName && sameType && sameLegalArea && sameGlobalPlacement && sameBlockTiles && sameOverlapTiles);
}

int Rectilinear::getId() const {
    return this->mId;
}
std::string Rectilinear::getName() const {
    return this->mName;
}
rectilinearType Rectilinear::getType() const {
    return this->mType;
}
Rectangle Rectilinear::getGlboalPlacement() const {
    return this->mGlobalPlacement;
}
area_t Rectilinear::getLegalArea() const {
    return this->mLegalArea;
}
double Rectilinear::getAspectRatioMin() const {
    return this->mAspectRatioMin;
}
double Rectilinear::getAspectRatioMax() const {
    return this->mAspectRatioMax;
}
double Rectilinear::getUtilizationMin() const {
    return this->mUtilizationMin;
}

void Rectilinear::setId(int id){
    this->mId = id;
}
void Rectilinear::setName(std::string name){
    this->mName = name;
}
void Rectilinear::setType(rectilinearType type){
    this->mType = type;
}
void Rectilinear::setGlobalPlacement(const Rectangle &globalPlacement){
    this->mGlobalPlacement = globalPlacement;
}
void Rectilinear::setLegalArea(area_t legalArea){
    this->mLegalArea = legalArea;
}
void Rectilinear::setAspectRatioMin(double aspectRatioMin){
    this->mAspectRatioMin = aspectRatioMin;
}
void Rectilinear::setAspectRatioMax(double aspectRatioMax){
    this->mAspectRatioMax = aspectRatioMax;
}
void Rectilinear::setUtilizationMin(double utilizationMin){
    this->mUtilizationMin = utilizationMin;
}

Rectangle Rectilinear::calculateBoundingBox() const {
    
    // if(this->mType == rectilinearType::PIN) return this->mGlobalPlacement;
    
    // if(blockTiles.empty() && overlapTiles.empty()){
        // modified by ryan
        // don't throw error, output global placement floorplan
    //     return mGlobalPlacement;
    // }

    assert(this->mType != rectilinearType::EMPTY);
    assert(this->mType != rectilinearType::PIN);

    assert(!(blockTiles.empty() && overlapTiles.empty()));


    len_t BBXL, BBYL, BBXH, BBYH;
    Tile *randomTile = (blockTiles.empty())? (*(overlapTiles.begin())) : (*(blockTiles.begin()));

    BBXL = (randomTile)->getXLow();
    BBYL = (randomTile)->getYLow();
    BBXH = (randomTile)->getXHigh();
    BBYH = (randomTile)->getYHigh();

    for(Tile *const &t : blockTiles){
        len_t xl = t->getXLow();
        len_t yl = t->getYLow();
        len_t xh = t->getXHigh();
        len_t yh = t->getYHigh();
        
        if(xl < BBXL) BBXL = xl;
        if(yl < BBYL) BBYL = yl;
        if(xh > BBXH) BBXH = xh;
        if(yh > BBYH) BBYH = yh;
    }
    for(Tile *const &t : overlapTiles){
        len_t xl = t->getXLow();
        len_t yl = t->getYLow();
        len_t xh = t->getXHigh();
        len_t yh = t->getYHigh();
        
        if(xl < BBXL) BBXL = xl;
        if(yl < BBYL) BBYL = yl;
        if(xh > BBXH) BBXH = xh;
        if(yh > BBYH) BBYH = yh;
    }

    return Rectangle(BBXL, BBYL, BBXH, BBYH);
}

area_t Rectilinear::calculateActualArea() const {
    area_t actualArea = 0;
    for(Tile *const &t : blockTiles){
        actualArea += t->getArea();
    }
    for(Tile *const &t : overlapTiles){
        actualArea += t->getArea();
    }
    return actualArea;
}

area_t Rectilinear::calculateResidualArea() const {

    area_t actualArea = 0;
    for(Tile *const &t : blockTiles){
        actualArea += t->getArea();
    }
    for(Tile *const &t : overlapTiles){
        actualArea += t->getArea();
    }
    
    return actualArea - mLegalArea;
}

double Rectilinear::calculateUtilization() const {
    return double(calculateActualArea())/double(rec::getArea(calculateBoundingBox()));
}

bool Rectilinear::isLegalNoOverlap() const {
    using namespace boost::polygon::operators;

    DoughnutPolygonSet dpSet, unionSet;

    for(Tile *const &t : this->blockTiles){
        Rectangle rt = t->getRectangle();
        assign(unionSet, dpSet&rt);
        if(!unionSet.empty()) return false;
        dpSet += rt;
    }
    for(Tile *const &t : this->overlapTiles){
        Rectangle rt = t->getRectangle();
        assign(unionSet, dpSet&rt);
        if(!unionSet.empty()) return false;
        dpSet += rt;
    }

    return true;
}

bool Rectilinear::isLegalEnoughArea() const {
    return (calculateActualArea() >= this->mLegalArea);
}

bool Rectilinear::isLegalAspectRatio() const {
    double aspectRatio = rec::calculateAspectRatio(calculateBoundingBox());
    return (aspectRatio >= mAspectRatioMin) && (aspectRatio <= mAspectRatioMax);
}

bool Rectilinear::isLegalUtilization() const {
    double minLegalArea = rec::getArea(calculateBoundingBox()) * mUtilizationMin;
    return (double(calculateActualArea()) >= minLegalArea);
}

bool Rectilinear::isLegalNoHole() const {
    using namespace boost::polygon::operators;
    DoughnutPolygonSet curRectSet;

    for(Tile *const &t : this->blockTiles){
        curRectSet += t->getRectangle();
    }
    for(Tile *const &t : this->overlapTiles){
        curRectSet += t->getRectangle();
    }

    return dps::isHoleFree(curRectSet);

}

bool Rectilinear::isLegalOneShape() const {
    using namespace boost::polygon::operators;
    DoughnutPolygonSet curRectSet;

    for(Tile *const &t : this->blockTiles){
        curRectSet += t->getRectangle();
    }
    for(Tile *const &t : this->overlapTiles){
        curRectSet += t->getRectangle();
    }

    return dps::isOneShape(curRectSet);
}

bool Rectilinear::isLegalInnerWidth(len_t minInnerWidth) const {
    using namespace boost::polygon::operators;
    DoughnutPolygonSet curRectSet;

    for(Tile *const &t : this->blockTiles){
        curRectSet += t->getRectangle();
    }
    for(Tile *const &t : this->overlapTiles){
        curRectSet += t->getRectangle();
    }

    return (dps::calMinInnerWidth(curRectSet) >= minInnerWidth);
}

bool Rectilinear::isLegal(rectilinearIllegalType &illegalCode, len_t minInnerWidth) const {

    // check if any tiles overlap
    using namespace boost::polygon::operators;
    DoughnutPolygonSet curRectSet, unionSet;

    for(Tile *const &t : this->blockTiles){
        Rectangle rt = t->getRectangle();
        assign(unionSet, curRectSet&rt);
        if(!unionSet.empty()){
            illegalCode = rectilinearIllegalType::OVERLAP;
            return false;
        }
        curRectSet += rt;
    }
    for(Tile *const &t : this->overlapTiles){
        Rectangle rt = t->getRectangle();
        assign(unionSet, curRectSet&rt);
        if(!unionSet.empty()){
            illegalCode = rectilinearIllegalType::OVERLAP;
            return false;
        }
        curRectSet += rt;
    }
    
    // check if rectilinear is in one shape
    if(!dps::isOneShape(curRectSet)){
        illegalCode = rectilinearIllegalType::TWO_SHAPE;
        return false;
    }

    // check if hole exist in rectilinear
    if(!dps::isHoleFree(curRectSet)){
        illegalCode = rectilinearIllegalType::HOLE;
        return false;
    }

    // check minimum inner width
    if(dps::calMinInnerWidth(curRectSet) < minInnerWidth){
        illegalCode = rectilinearIllegalType::INNER_WIDTH;
        return false;
    }

    // check area
    area_t actualArea = calculateActualArea();
    if(actualArea < this->mLegalArea){
        illegalCode = rectilinearIllegalType::AREA;
        return false;
    }

    // check bounding box aspect ratio
    Rectangle boundingBox = calculateBoundingBox();
    double boundingBoxAspectRatio = rec::calculateAspectRatio(boundingBox);
    if((boundingBoxAspectRatio < mAspectRatioMin) || (boundingBoxAspectRatio > mAspectRatioMax)){
        illegalCode = rectilinearIllegalType::ASPECT_RATIO;
        return false;
    }

    // check bounding box utilization
    double minUtilizationArea = double(rec::getArea(boundingBox)) * mUtilizationMin;
    if(double(actualArea) < minUtilizationArea){
        illegalCode = rectilinearIllegalType::UTILIZATION;
        return false;
    }


    // error free
    illegalCode = rectilinearIllegalType::LEGAL;
    return true;

}

void Rectilinear::acquireWinding(std::vector<Cord> &winding, Direction1D wd) const {
    
    // if((blockTiles.empty())&&(overlapTiles.empty())){
    //     throw CSException("RECTILINEAR_02");
    // }
    assert(!(blockTiles.empty() && overlapTiles.empty()));

    using namespace boost::polygon::operators;
    DoughnutPolygonSet curRectSet;

    for(Tile *const &t : this->blockTiles){
        curRectSet += t->getRectangle();
    }
    for(Tile *const &t : this->overlapTiles){
        curRectSet += t->getRectangle();
    }

    assert(dps::isOneShape(curRectSet));
    assert(dps::isHoleFree(curRectSet));

    /* this excpetion would cause strange error! changed to assert*/
    // if(!(dps::oneShape(curRectSet))){
    //     throw CSException("RECTILINEAR_03");
    // }
    // if(!(dps::noHole(curRectSet))){
    //     throw CSException("RECTILINEAR_04");
    // }

    DoughnutPolygon rectilinearShape = curRectSet[0];
    dp::acquireWinding(rectilinearShape, winding, wd);

}

size_t std::hash<Rectilinear>::operator()(const Rectilinear &key) const {

    std::size_t seed = 0;
    boost::hash_combine(seed, key.getId());
    boost::hash_combine(seed, key.getName());
    boost::hash_combine(seed, key.getType());
    boost::hash_combine(seed, key.getGlboalPlacement());
    boost::hash_combine(seed, key.getLegalArea());
    boost::hash_combine(seed, key.getAspectRatioMin());
    boost::hash_combine(seed, key.getAspectRatioMax());
    boost::hash_combine(seed, key.getUtilizationMin());
    
    return seed;
}

size_t boost::hash<Rectilinear>::operator()(const Rectilinear &key) const {

    std::size_t seed = 0;
    boost::hash_combine(seed, key.getId());
    boost::hash_combine(seed, key.getName());
    boost::hash_combine(seed, key.getType());
    boost::hash_combine(seed, key.getGlboalPlacement());
    boost::hash_combine(seed, key.getLegalArea());
    boost::hash_combine(seed, key.getAspectRatioMin());
    boost::hash_combine(seed, key.getAspectRatioMax());
    boost::hash_combine(seed, key.getUtilizationMin());
    
    return seed;
}

std::ostream &operator << (std::ostream &os, const Rectilinear &r){
    os << "ID = " << r.mId << " Name = " << r.mName << " Type = " << r.mType;

    os << " Global Placement = " << r.mGlobalPlacement << std::endl;
    os << "Aspect Ratio: " << r.mAspectRatioMin << " ~ " << r.mAspectRatioMax << ", Utilization Min = " << r.mUtilizationMin << std::endl; 

    os << "BLOCK Tiles (" << r.blockTiles.size() << ")" << std::endl;
    for(Tile *const &t : r.blockTiles){
        os << *t << std::endl;
    }

    os << "OVERLAP Tiles (" << r.overlapTiles.size() << ")";
    for(Tile *const &t : r.overlapTiles){
        os << std::endl << *t; 
    }

    return os;
}

