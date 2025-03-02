//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/02/2025 22:29:22
//  Module Name:        lineTile.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A combination of Line(horizontal or vertical) and Tile data
//                      structure. The line should be within/on the side of the tile
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:           
//
//////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:
#include <ostream>

// 2. Boost Library:
#include "boost/functional/hash.hpp"

// 3. Texo Library:
#include "lineTile.hpp"
#include "line.hpp"
#include "tile.hpp"

std::ostream &operator << (std::ostream &os, const lineTileDirection &ltd){
    switch (ltd)
    {
    case lineTileDirection::LEFT:
        os << "lineTileDirection::LEFT";
        break;
    case lineTileDirection::RIGHT:
        os << "lineTileDirection::RIGHT";
        break;
    case lineTileDirection::DOWN:
        os << "lineTileDirection::DOWN";
        break;
    case lineTileDirection::UP:
        os << "lineTileDirection::UP";
        break;
    case lineTileDirection::CENTRE:
        os << "lineTileDirection::CENTRE";
        break;
    default:
        break;
    }
   
    return os;
}

LineTile::LineTile()
    :mLine(Line()), mTile(nullptr), mDirection(lineTileDirection::CENTRE){
}

LineTile::LineTile(const Line &line, Tile *tile){

    assert(tile != nullptr);

    if(line.getOrient() == eOrientation2D::HORIZONTAL){
        
        // check if the line is well fit with the tile
        assert(tile->getXHigh() >= line.getHigh().x());
        assert(tile->getXLow() <= line.getLow().x());
        
        len_t lineY = line.getLow().y();
        len_t tileYHigh = tile->getYHigh();
        len_t tileYLow = tile->getYLow();
        
        assert(lineY <= tileYHigh);
        assert(lineY >= tileYLow);

        if(lineY == tileYHigh) mDirection = lineTileDirection::DOWN;
        else if(lineY == tileYLow) mDirection = lineTileDirection::UP;
        else mDirection = lineTileDirection::CENTRE;

        mLine = line;
        mTile = tile;

    }else{ //orientation2D::VERTICAL

        // check if the lie is wel fit with the tile
        assert(tile->getYHigh() >= line.getHigh().y());
        assert(tile->getYLow() <= line.getLow().y());

        len_t lineX = line.getLow().x();
        len_t tileXHigh = tile->getXHigh();
        len_t tileXLow = tile->getXLow();
        
        assert(lineX <= tileXHigh);
        assert(lineX >= tileXLow);

        if(lineX == tileXHigh) mDirection = lineTileDirection::LEFT;
        else if(lineX == tileXLow) mDirection = lineTileDirection::RIGHT;
        else mDirection = lineTileDirection::CENTRE;

        mLine = line;
        mTile = tile;
    }
}

bool LineTile::operator == (const LineTile &comp) const {
    return (mLine == comp.mLine) && (mTile == comp.mTile);

}

Line LineTile::getLine() const {
    return this->mLine;
}

Tile *LineTile::getTile() const {
    return this->mTile;
}

lineTileDirection LineTile::getDirection() const {
    return this->mDirection;
}

std::ostream &operator<<(std::ostream &os, const LineTile &tile) {
    return os << "LT[" << tile.getLine() << " " << *(tile.getTile()) << " " << tile.getDirection() << "]";
}

size_t std::hash<LineTile>::operator()(const LineTile &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.getLine());
    boost::hash_combine(seed, key.getTile());
    return seed;
}

size_t boost::hash<LineTile>::operator()(const LineTile &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.getLine());
    boost::hash_combine(seed, key.getTile());
    return seed;
}
