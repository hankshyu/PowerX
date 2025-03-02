//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/02/2025 14:55:45
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
//
//////////////////////////////////////////////////////////////////////////////////

#ifndef __RECTILINEAR_H__
#define __RECTILINEAR_H__

// Dependencies
// 1. C++ STL:
#include <ostream>
#include <string>
#include <unordered_set>

// 2. Boost Library:
#include "boost/polygon/polygon.hpp"
#include "boost/functional/hash.hpp"

// 3. Texo Library:

#include "rectangle.hpp"
#include "isotropy.hpp"
#include "tile.hpp"

enum class rectilinearType{
    EMPTY, SOFT, PREPLACED, PIN
};

std::ostream &operator << (std::ostream &os, const rectilinearType &t);

enum class rectilinearIllegalType{
    LEGAL, PREPLACE_FAIL,
    OVERLAP, HOLE, TWO_SHAPE, INNER_WIDTH, 
    AREA, ASPECT_RATIO, UTILIZATION, 

};

std::ostream &operator << (std::ostream &os, const rectilinearIllegalType &t);

class Rectilinear{
private:
    int mId;
    std::string mName;
    rectilinearType mType;
    
    Rectangle mGlobalPlacement;
    area_t mLegalArea;
    double mAspectRatioMin;
    double mAspectRatioMax;
    double mUtilizationMin;

public: 

    std::unordered_set<Tile *> blockTiles;
    std::unordered_set<Tile *> overlapTiles;

    Rectilinear();
    Rectilinear(int id, std::string name, rectilinearType type, Rectangle globalPlacement,
                area_t legalArea, double aspectRatioMin, double aspectRatioMax, double utilizationMin);
    Rectilinear(const Rectilinear &other);

    Rectilinear &operator = (const Rectilinear &other);
    bool operator == (const Rectilinear &other) const;

    int getId() const;
    std::string getName() const;
    rectilinearType getType() const;
    Rectangle getGlboalPlacement() const;
    area_t getLegalArea() const;
    double getAspectRatioMin() const;
    double getAspectRatioMax() const;
    double getUtilizationMin() const;


    void setId(int id);
    void setName(std::string name);
    void setType(rectilinearType type);
    void setGlobalPlacement(const Rectangle &globalPlacement);
    void setLegalArea(area_t legalArea);
    void setAspectRatioMin(double aspectRatioMin);
    void setAspectRatioMax(double aspectRatioMax);
    void setUtilizationMin(double utilizationMin);
    

    Rectangle calculateBoundingBox() const;
    area_t calculateActualArea() const;
    area_t calculateResidualArea() const;
    double calculateUtilization() const;

    // check if rectilinear contains enough area (larger than mLegalArea), return false if violated
    bool isLegalEnoughArea() const;

    // check if the aspect ratio of rectilinear is within mAspectRatioMin ~ mAspectRatioMax, return false if violated
    bool isLegalAspectRatio() const;

    // check if the utilization (acutal area within the bounding box) is greater than mUtilizationMin, return false if violated
    bool isLegalUtilization() const;

    // check if any of the tiles overlap each other, return false if violated
    bool isLegalNoOverlap() const;

    // check if rectilinear contains internal hole (become doughnut shape), return false if violated
    bool isLegalNoHole() const;

    // check if rectilinear is disconnected, return false if violated
    bool isLegalOneShape() const;
    
    // check if violate minimum inner width, rectilinear inner width value should >= input minInnerWidth
    bool isLegalInnerWidth(len_t minInnerWidth) const;

    // use all legal checking methods, if any violated, return false an error code passed through illegalCode
    bool isLegal(rectilinearIllegalType &illegalCode, len_t minInnerWidth) const;

    // acquire the windings of rectiinear, may choose winding direction, points pass through vector "winding"
    void acquireWinding(std::vector<Cord> &winding,  Direction1D wd = eDirection1D::CLOCKWISE) const;
    
    friend std::ostream &operator << (std::ostream &os, const Rectilinear &recti);

};

// Rectilinear class hash function implementations
namespace std{
    template<>
    struct hash<Rectilinear>{
        size_t operator()(const Rectilinear &key) const;
    };
} // namespace std

namespace boost {
    template<>
    struct hash<Rectilinear>{
        size_t operator()(const Rectilinear &key) const;
    };

} // namespace boost


#endif // __RECTILINEAR_H__